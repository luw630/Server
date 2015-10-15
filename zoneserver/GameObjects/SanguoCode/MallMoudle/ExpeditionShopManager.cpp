#include "stdafx.h"
#include "ExpeditionShopManager.h"
#include "../Common/ConfigManager.h"
#include "NETWORKMODULE/SanguoPlayer.h"
#include "../BaseDataManager.h"
#include "MallsMoudle.h"

CExpeditionShopManager::CExpeditionShopManager(CBaseDataManager& baseDataMgr)
	: CMallManager(baseDataMgr)
{
}


CExpeditionShopManager::~CExpeditionShopManager()
{
}

MallType CExpeditionShopManager::GetType() const
{
	return MallType::ExpeditionShop;
}

bool CExpeditionShopManager::InitDataMgr(void * pData)
{
	if (nullptr == pData)
		return false;
	m_pMallData = static_cast<SSanguoMallData*>(pData);
	m_pVecRefreshClock = &CConfigManager::getSingleton()->globalConfig.vecExpeditionShopRefreshClock;
	m_dwLimitLevel = CConfigManager::getSingleton()->GetGameFeatureActivationConfig().expeditionDungeonLevelLimit;
	if (JudgementRefresh(m_pBaseDataMgr.GetTimeData()))
		CMallsMoudle::getSingleton()->RandomCommoditys(this);
	SetRefreshInfo(false);
	return true;
}

bool CExpeditionShopManager::GetCommodityRechargeTypeAddCost(UINT index, OUT SSanguoCurrencyType& currencyType, OUT DWORD& cost) const
{
	auto pCommodityData = GetCommodityData(index);
	if (nullptr == pCommodityData)
		return false;

	auto itemConfig = CConfigManager::getSingleton()->GetItemConfig(pCommodityData->m_dwID);
	if (nullptr != itemConfig)
	{
		currencyType = SSanguoCurrencyType::Currency_Exploit;
		cost = pCommodityData->m_dwCount * itemConfig->BuyExploitPrice;
		return true;
	}

	auto equipConfig = CConfigManager::getSingleton()->GetEquipment(pCommodityData->m_dwID);
	if (nullptr != equipConfig)
	{
		currencyType = SSanguoCurrencyType::Currency_Exploit;
		cost = pCommodityData->m_dwCount * equipConfig->buyExploitPrice;
		return true;
	}

	return false;
}
GoodsWay CExpeditionShopManager::RechargeWay() const
{
	return GoodsWay::exploitMall;
}

void CExpeditionShopManager::GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs)
{
	CConfigManager::getSingleton()->GetExpeditionShopCommodityConfigs(m_pBaseDataMgr.GetPlayerLevel(), vecConfigs);
}
