#include "stdafx.h"
#include "MallsMoudle.h"
#include "../BaseDataManager.h"
#include "MallManager.h"
#include "../StorageMoudle/StorageManager.h"
#include "Random.h"
#include "Player.h"
#include "ScriptManager.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"

extern LPIObject GetPlayerBySID(DWORD dwStaticID);

CMallsMoudle::CMallsMoudle() : m_GlobalConfig(CConfigManager::getSingleton()->globalConfig)
{
	m_vecHasCommodity.reserve(MALL_COMMODITY_NUM);
	m_vecUsableItem.reserve(24);
	m_vecUnusableItemsIndex.reserve(5);
}


CMallsMoudle::~CMallsMoudle()
{

}

void CMallsMoudle::DispatchMallMsg(CMallManager* pMallMgr, SMallMsgInfo* pMsg)
{
	if (nullptr == pMallMgr || nullptr == pMsg)
		return;
	switch (pMsg->_protocol)
	{
	case SMallBaseMsg::MALL_BUYCOMMODITY:
		{
			SBuyCommodityRequest* buyMsg = static_cast<SBuyCommodityRequest*>(pMsg);
			if (nullptr == buyMsg)
				return;
			_ProcessBuyCommodityMsg(pMallMgr, buyMsg);
		}
		break;
	case SMallBaseMsg::MALL_REFRESH:
		{
			SRefreshCommodityRequest* refreshMsg = static_cast<SRefreshCommodityRequest*>(pMsg);
			if (nullptr == refreshMsg)
				return;
			_ProcessRefreshCommodityMsg(pMallMgr, refreshMsg);
		}
		break;
	default:
		break;
	}
}

void CMallsMoudle::_ProcessBuyCommodityMsg(CMallManager* pMallMgr, SBuyCommodityRequest* pMsg)
{
	CBaseDataManager& pBaseDataMgr = pMallMgr->GetBaseDataMgr();
	/// 判断商城是否开放  GM帐号默认开放
	if (!pMallMgr->IsOpened() && !pBaseDataMgr.GetGMFlag())
		return;
	SSanguoCurrencyType currency;
	DWORD cost;
	/// 判断商品是否能够购买
	if (!pMallMgr->CanBuyCommodity(pMsg->m_dwIndex, currency, cost))
		return;


	int32_t GoodsWayType = pMallMgr->RechargeWay();
	/// 购买商品并添加到背包
	pMallMgr->BuyCommodity(pMsg->m_dwIndex);
	auto commodity = pMallMgr->GetCommodityData(pMsg->m_dwIndex);
	pBaseDataMgr.AddGoods_SG(GoodsType::item, commodity->m_dwID, commodity->m_dwCount, GoodsWayType);

	SBuyCommodityResult resultMsg;
	resultMsg.m_CurrencyType = currency;
	resultMsg.m_dwIndex = pMsg->m_dwIndex;
	resultMsg.m_MallType = pMallMgr->GetType();
	/// 扣除购买商品消耗
	switch (currency)
	{
	case SSanguoCurrencyType::Currency_Money:
		resultMsg.m_dwProperty = pBaseDataMgr.DecGoods_SG(GoodsType::money, 0, cost, GoodsWayType);
		break;
	case SSanguoCurrencyType::Currency_Diamond:
		resultMsg.m_dwProperty = pBaseDataMgr.DecGoods_SG(GoodsType::diamond, 0, cost, GoodsWayType);
		break;
	case SSanguoCurrencyType::Currency_Honor:
		resultMsg.m_dwProperty = pBaseDataMgr.DecGoods_SG(GoodsType::honor, 0, cost, GoodsWayType);
		break;
	case SSanguoCurrencyType::Currency_Exploit:
		resultMsg.m_dwProperty = pBaseDataMgr.DecGoods_SG(GoodsType::exploit, 0, cost, GoodsWayType);
		break;
	case SSanguoCurrencyType::Currency_Token:
		{
			CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(pBaseDataMgr.GetSID())->DynamicCast(IID_PLAYER);
			if (nullptr != pPlayer)
			{
				g_Script.SetCondition(0, pPlayer, 0);
				if (g_Script.PrepareFunction("ChangePlayerFactionMoney"))
				{
					g_Script.PushParameter(pPlayer->GetSID());
					g_Script.PushParameter(-(int)cost);
					g_Script.Execute();
				}
				g_Script.CleanCondition();
			}

			lite::lua_variant ret;
			BOOL result = FALSE;
			if (g_Script.PrepareFunction("GetPlayerFactionMoney"))
			{
				g_Script.PushParameter(pBaseDataMgr.GetSID());
				result = g_Script.Execute(&ret);
			}
			else
			{
				return;
			}

			if (!result || ret.dataType == LUA_TNIL)
			{
				rfalse("GetPlayerFactionMoney Failed");
				return;
			}

			int token = 0;
			try
			{
				token = (int)(ret);
			}
			catch (lite::Xcpt &e)
			{
				rfalse(2, 1, e.GetErrInfo());
				return;
			}
			resultMsg.m_dwProperty = token;
		}
		break;
	case SSanguoCurrencyType::Currency_SoulPoints:
		resultMsg.m_dwProperty = pBaseDataMgr.DecGoods_SG(GoodsType::soulPoints, 0, cost, GoodsWayType);
		break;
	case SSanguoCurrencyType::Currency_Prestige:
		resultMsg.m_dwProperty = pBaseDataMgr.DecGoods_SG(GoodsType::Prestige, 0, cost, GoodsWayType);
		break;
	default:
		break;
	}

	g_StoreMessage(pBaseDataMgr.GetDNID(), &resultMsg, sizeof(SBuyCommodityResult));
}

void CMallsMoudle::_ProcessRefreshCommodityMsg(CMallManager* pMallMgr, SRefreshCommodityRequest* pMsg)
{
	CBaseDataManager& pBaseDataMgr = pMallMgr->GetBaseDataMgr();
	/// 非GM帐号 判断开放条件
	if (!pMallMgr->IsOpened() && !pBaseDataMgr.GetGMFlag())
		return;
	/// 获取商城已手动刷新的次数
	DWORD refreshedCount = pMallMgr->GetRefreshedCount();
	/// 获取商城手动刷新消耗
	int32_t refreshCost = _GetRefreshCost(pMallMgr->GetType(), refreshedCount);
	if (_CanRefresh(pMallMgr, refreshCost) == false)
		return;
	if (!RandomCommoditys(pMallMgr))
		return;
	
	SRefreshCommodityResult resultMsg;
	switch (pMallMgr->GetType())
	{
	case VarietyShop:
	case MiracleMerchant:
	case GemMerchant:
		resultMsg.m_dwProperty = pBaseDataMgr.PlusDiamond(-refreshCost);
		break;
	case ArenaShop:
		resultMsg.m_dwProperty = pBaseDataMgr.PlusHonor(-refreshCost);
		break;
	case  ExpeditionShop:
		resultMsg.m_dwProperty = pBaseDataMgr.PlusExploit(-refreshCost);
		break;
	case LegionShop:
		{
			CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(pBaseDataMgr.GetSID())->DynamicCast(IID_PLAYER);
			if (nullptr != pPlayer)
			{
				g_Script.SetCondition(0, pPlayer, 0);
				if (g_Script.PrepareFunction("ChangePlayerFactionMoney"))
				{
					g_Script.PushParameter(pPlayer->GetSID());
					g_Script.PushParameter(-refreshCost);
					g_Script.Execute();
				}
				//g_Script.CleanCondition();
			}

			lite::lua_variant ret;
			BOOL result = FALSE;
			if (g_Script.PrepareFunction("GetPlayerFactionMoney"))
			{
				g_Script.PushParameter(pBaseDataMgr.GetSID());
				result = g_Script.Execute(&ret);
			}
			else
			{
				return;
			}

			if (!result || ret.dataType == LUA_TNIL)
			{
				rfalse("GetPlayerFactionMoney Failed");
				return;
			}

			int token = 0;
			try
			{
				token = (int)(ret);
			}
			catch (lite::Xcpt &e)
			{
				rfalse(2, 1, e.GetErrInfo());
				return;
			}
			resultMsg.m_dwProperty = token;
		}
		break;
	case SoulExchange:
		resultMsg.m_dwProperty =  pBaseDataMgr.ModifySoulPoints(-refreshCost);
		break;
	case WarOfLeagueShop:
		resultMsg.m_dwProperty = pBaseDataMgr.PlusPrestige(-refreshCost);
		break;
	default:
		break;
	}

	/// 增加商店手动刷新次数
	pMallMgr->AddRefreshCount();
	resultMsg.m_MallType = pMsg->m_MallType;
	resultMsg.m_dwNextRefreshTime = pMallMgr->GetNextRefreshTime();
	resultMsg.m_dwRefreshedCount = pMallMgr->GetRefreshedCount();
	///设置货币属性 TODO
	for (int i = 0; i < MALL_COMMODITY_NUM; ++i)
	{
		auto tempCommodity = pMallMgr->GetCommodityData(i);
		resultMsg.m_arrCommodity[i].m_dwID = tempCommodity->m_dwID;
		resultMsg.m_arrCommodity[i].m_dwCount = tempCommodity->m_dwCount;
		resultMsg.m_arrCommodity[i].m_bSoldOut = tempCommodity->m_bSoldOut;
	}

	g_StoreMessage(pBaseDataMgr.GetDNID(), &resultMsg, sizeof(SRefreshCommodityResult));
}

void CMallsMoudle::SendAutoRefreshMsgToClient(CMallManager* pMallMgr)
{
	if (nullptr == pMallMgr)
		return;
	CBaseDataManager& pBaseDataMgr = pMallMgr->GetBaseDataMgr();
	if (pMallMgr->AutoRefreshCommoditys() == false)
		return;

	SAutoRefreshCommodityMsg autoMsg;
	autoMsg.m_MallType = pMallMgr->GetType();
	autoMsg.m_dwNextRefreshTime = pMallMgr->GetNextRefreshTime();
	autoMsg.m_dwNextRefreshRemainingSeconds = pMallMgr->GetNextRefreshRemainingSeconds();
	for (int i = 0; i < MALL_COMMODITY_NUM; ++i)
	{
		auto tempCommodity = pMallMgr->GetCommodityData(i);
		if (nullptr == tempCommodity)
			return;
		autoMsg.m_arrCommodity[i].m_dwID = tempCommodity->m_dwID;
		autoMsg.m_arrCommodity[i].m_dwCount = tempCommodity->m_dwCount;
		autoMsg.m_arrCommodity[i].m_bSoldOut = tempCommodity->m_bSoldOut;
	}

	g_StoreMessage(pBaseDataMgr.GetDNID(), &autoMsg, sizeof(SAutoRefreshCommodityMsg));
}

int32_t CMallsMoudle::_GetRefreshCost(MallType type, DWORD dwRefreshedCount) const
{
	switch (type)
	{
	case VarietyShop:
		return dwRefreshedCount >= m_GlobalConfig.vecVarietyShopRefreshCost.size() ? m_GlobalConfig.vecVarietyShopRefreshCost.back() : m_GlobalConfig.vecVarietyShopRefreshCost[dwRefreshedCount];
	case ArenaShop:
		return dwRefreshedCount >= m_GlobalConfig.vecArenaShopRefreshCost.size() ? m_GlobalConfig.vecArenaShopRefreshCost.back() : m_GlobalConfig.vecArenaShopRefreshCost[dwRefreshedCount];
	case ExpeditionShop:
		return dwRefreshedCount >= m_GlobalConfig.vecExpeditionShopRefreshCost.size() ? m_GlobalConfig.vecExpeditionShopRefreshCost.back() : m_GlobalConfig.vecExpeditionShopRefreshCost[dwRefreshedCount];
	case MiracleMerchant:
		return dwRefreshedCount >= m_GlobalConfig.vecMiracleMerchantRefreshCost.size() ? m_GlobalConfig.vecMiracleMerchantRefreshCost.back() : m_GlobalConfig.vecMiracleMerchantRefreshCost[dwRefreshedCount];
	case GemMerchant:
		return dwRefreshedCount >= m_GlobalConfig.vecGemMerchantRefreshCost.size() ? m_GlobalConfig.vecGemMerchantRefreshCost.back() : m_GlobalConfig.vecGemMerchantRefreshCost[dwRefreshedCount];
	case LegionShop:
		return dwRefreshedCount >= m_GlobalConfig.vecLegionShopRefreshCost.size() ? m_GlobalConfig.vecLegionShopRefreshCost.back() : m_GlobalConfig.vecLegionShopRefreshCost[dwRefreshedCount];
	case SoulExchange:
		return dwRefreshedCount >= m_GlobalConfig.vecSoulExchangeRefreshCost.size() ? m_GlobalConfig.vecSoulExchangeRefreshCost.back() : m_GlobalConfig.vecSoulExchangeRefreshCost[dwRefreshedCount];
	case WarOfLeagueShop:
		return dwRefreshedCount >= m_GlobalConfig.vecWarOfLeagueShopRefreshCost.size() ? m_GlobalConfig.vecSoulExchangeRefreshCost.back() : m_GlobalConfig.vecWarOfLeagueShopRefreshCost[dwRefreshedCount];
	default:
		break;
	}
	return 0;
}

bool CMallsMoudle::_CanRefresh(CMallManager* pMallMgr, DWORD refreshCost) const
{
	switch (pMallMgr->GetType())
	{
	case VarietyShop:
		return pMallMgr->GetBaseDataMgr().GetDiamond() >= refreshCost;
	case ArenaShop:
		return pMallMgr->GetBaseDataMgr().GetHonor() >= refreshCost;
	case ExpeditionShop:
		return pMallMgr->GetBaseDataMgr().GetExploit() >= refreshCost;
	case MiracleMerchant:
		return pMallMgr->GetBaseDataMgr().GetDiamond() >= refreshCost;
	case GemMerchant:
		return pMallMgr->GetBaseDataMgr().GetDiamond() >= refreshCost;
	case LegionShop:
		{
			lite::lua_variant ret;
			BOOL result = FALSE;
			if (g_Script.PrepareFunction("GetPlayerFactionMoney"))
			{
				g_Script.PushParameter(pMallMgr->GetBaseDataMgr().GetSID());
				result = g_Script.Execute(&ret);
			}
			else
			{
				return false;
			}

			if (!result || ret.dataType == LUA_TNIL)
			{
				rfalse("GetPlayerFactionMoney Failed");
				return false;
			}

			int token = 0;
			try
			{
				token = (int)(ret);
			}
			catch (lite::Xcpt &e)
			{
				rfalse(2, 1, e.GetErrInfo());
				return false;
			}

			return token >= refreshCost;
		}
		break;
	case SoulExchange:
		return pMallMgr->GetBaseDataMgr().GetGoods(GoodsType::soulPoints) >= refreshCost;
	case WarOfLeagueShop:
		return pMallMgr->GetBaseDataMgr().GetPrestige() >= refreshCost;
	default:
		break;
	}
	return false;
}

bool CMallsMoudle::RandomCommoditys(CMallManager* pMallMgr)
{
	if (nullptr == pMallMgr)
		return false;

	m_vecTempCommodityConfig.clear();
	/// 获取配置表
	pMallMgr->GetCommodityConfigs(m_vecTempCommodityConfig);

	if (m_vecTempCommodityConfig.empty())
		return false;

	auto pCommodityDatas = pMallMgr->GetCommodityDatas();
	if (nullptr == pCommodityDatas)
		return false;

	m_vecHasCommodity.clear();
	DWORD dwID, dwCount, dwIndex = 0;
	for each (auto& config in m_vecTempCommodityConfig)
	{
		m_vecUnusableItemsIndex.clear();
		_GetCommodityData(config, dwID, dwCount, m_vecUnusableItemsIndex);
		pCommodityDatas[dwIndex++].SetData(dwID, dwCount, false);
		m_vecHasCommodity.push_back(dwID);
	}
	return true;
}

void CMallsMoudle::_GetCommodityData(const CommodityConfig* config, DWORD& dwID, DWORD& dwCount, vector<int32_t>& unusableItemIndex)
{
	int32_t unusableIndex = 0;
	int32_t maxWeight = 0;
	int32_t tempRandVal = 0;
	const vector<int32_t>* finalVec = nullptr;

	//判断此物品列表是否可用(为空则直接加入不可用列表,包含的物品ID全是已随机到的物品ID也加入不可用列表)
	if (find(unusableItemIndex.begin(), unusableItemIndex.end(), 1) == unusableItemIndex.end())
	{
		if (!config->vecItem1.empty())
		{
			maxWeight = CRandom::RandRange(1, config->percent1);
			finalVec = &(config->vecItem1);
			unusableIndex = 1;
		}
		else
			unusableItemIndex.push_back(1);
	}

	if (find(unusableItemIndex.begin(), unusableItemIndex.end(), 2) == unusableItemIndex.end())
	{
		if (!config->vecItem2.empty())
		{
			tempRandVal = CRandom::RandRange(1, config->percent2);
			if (tempRandVal >= maxWeight)
			{
				maxWeight = tempRandVal;
				finalVec = &(config->vecItem2);
				unusableIndex = 2;
			}
		}
		else
			unusableItemIndex.push_back(2);
	}

	if (find(unusableItemIndex.begin(), unusableItemIndex.end(), 3) == unusableItemIndex.end())
	{
		if (!config->vecItem3.empty())
		{
			tempRandVal = CRandom::RandRange(1, config->percent3);
			if (tempRandVal >= maxWeight)
			{
				maxWeight = tempRandVal;
				finalVec = &(config->vecItem3);
				unusableIndex = 3;
			}
		}
		else
			unusableItemIndex.push_back(3);
	}

	if (find(unusableItemIndex.begin(), unusableItemIndex.end(), 4) == unusableItemIndex.end())
	{
		if (!config->vecItem4.empty())
		{
			tempRandVal = CRandom::RandRange(1, config->percent4);
			if (tempRandVal >= maxWeight)
			{
				maxWeight = tempRandVal;
				finalVec = &(config->vecItem4);
				unusableIndex = 4;
			}
		}
		else
			unusableItemIndex.push_back(4);
	}

	if (find(unusableItemIndex.begin(), unusableItemIndex.end(), 5) == unusableItemIndex.end())
	{
		if (!config->vecItem5.empty())
		{
			tempRandVal = CRandom::RandRange(1, config->percent5);
			if (tempRandVal >= maxWeight)
			{
				finalVec = &(config->vecItem5);
				unusableIndex = 5;
			}
		}
		else
			unusableItemIndex.push_back(5);
	}

	if (nullptr == finalVec)
	{
		rfalse("商品配置表为空或其它异常");
		dwID = 0;
		dwCount = 0;
		return;
	}

	if (m_vecHasCommodity.empty())
	{
		dwID = (*finalVec)[CRandom::RandRange(0, finalVec->size() - 1)];
		dwCount = CRandom::RandRange(config->minNumber, config->maxNumber);
		return;
	}

	auto begin = m_vecHasCommodity.begin();
	auto end = m_vecHasCommodity.end();
	int32_t id;
	m_vecUsableItem.clear();
	for (int i = 0, count = finalVec->size(); i < count; ++i)
	{
		id = (*finalVec)[i];
		if (find(begin, end, id) != end)
			continue;
		m_vecUsableItem.push_back(id);
	}

	if (m_vecUsableItem.empty())
	{
		unusableItemIndex.push_back(unusableIndex);
		_GetCommodityData(config, dwID, dwCount, unusableItemIndex);
	}

	dwID = m_vecUsableItem[CRandom::RandRange(0, m_vecUsableItem.size() - 1)];
	dwCount = CRandom::RandRange(config->minNumber, config->maxNumber);
}

void CMallsMoudle::_GetCommodityData(const CommodityConfig* config, DWORD& dwID, DWORD& dwCount)
{
	dwCount = CRandom::RandRange(config->minNumber, config->maxNumber);

	int32_t maxWeight = 0;
	int32_t tempRandVal = 0;
	const vector<int32_t>* finalVec;
	if (!config->vecItem1.empty())
	{
		maxWeight = CRandom::RandRange(1, config->percent1);
		finalVec = &(config->vecItem1);
	}
	if (!config->vecItem2.empty())
	{
		tempRandVal = CRandom::RandRange(1, config->percent2);
		if (tempRandVal >= maxWeight)
		{
			maxWeight = tempRandVal;
			finalVec = &(config->vecItem2);
		}
	}
	if (!config->vecItem3.empty())
	{
		tempRandVal = CRandom::RandRange(1, config->percent3);
		if (tempRandVal >= maxWeight)
		{
			maxWeight = tempRandVal;
			finalVec = &(config->vecItem3);
		}
	}
	if (!config->vecItem4.empty())
	{
		tempRandVal = CRandom::RandRange(1, config->percent4);
		if (tempRandVal >= maxWeight)
		{
			maxWeight = tempRandVal;
			finalVec = &(config->vecItem4);
		}
	}
	if (!config->vecItem5.empty())
	{
		tempRandVal = CRandom::RandRange(1, config->percent5);
		if (tempRandVal >= maxWeight)
			finalVec = &(config->vecItem5);
	}

	if (m_vecHasCommodity.empty())
	{
		dwID = (*finalVec)[CRandom::RandRange(0, finalVec->size() - 1)];
		return;
	}

	auto begin = m_vecHasCommodity.begin();
	auto end = m_vecHasCommodity.end();
	int32_t id;
	m_vecUsableItem.clear();
	for (int i = 0, count = finalVec->size(); i < count; ++i)
	{
		id = (*finalVec)[i];
		if (find(begin, end, id) != end)
			continue;
		m_vecUsableItem.push_back(id);
	}

	dwID = m_vecUsableItem[CRandom::RandRange(0, m_vecUsableItem.size() - 1)];
}
