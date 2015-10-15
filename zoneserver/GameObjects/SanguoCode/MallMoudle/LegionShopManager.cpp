#include "stdafx.h"
#include "LegionShopManager.h"
#include "../Common/ConfigManager.h"
#include "NETWORKMODULE/SanguoPlayer.h"
#include "../BaseDataManager.h"
#include "MallsMoudle.h"

CLegionShopManager::CLegionShopManager(CBaseDataManager& baseDataManager)
	: CMallManager(baseDataManager)
{
}


CLegionShopManager::~CLegionShopManager()
{
}

MallType CLegionShopManager::GetType() const
{
	return MallType::LegionShop;
}

bool CLegionShopManager::InitDataMgr(void* pData)
{
	if (nullptr == pData)
		return false;
	m_pMallData = static_cast<SSanguoMallData*>(pData);
	m_pVecRefreshClock = &CConfigManager::getSingleton()->globalConfig.vecLegionShopRefreshClock;
	m_dwLimitLevel = CConfigManager::getSingleton()->GetGameFeatureActivationConfig().teamSiegeLevelLimit;
	if (JudgementRefresh(m_pBaseDataMgr.GetTimeData()))
		CMallsMoudle::getSingleton()->RandomCommoditys(this);
	SetRefreshInfo(false);
	return true;
}

bool CLegionShopManager::GetCommodityRechargeTypeAddCost(UINT index, OUT SSanguoCurrencyType& currencyType, OUT DWORD& cost) const
{
	auto pCommodityData = GetCommodityData(index);
	if (nullptr == pCommodityData)
		return false;

	auto itemConfig = CConfigManager::getSingleton()->GetItemConfig(pCommodityData->m_dwID);
	if (nullptr != itemConfig)
	{
		currencyType = SSanguoCurrencyType::Currency_Token;
		cost = pCommodityData->m_dwCount * itemConfig->BuyTokenPrice;
		return true;
	}

	auto equipConfig = CConfigManager::getSingleton()->GetEquipment(pCommodityData->m_dwID);
	if (nullptr != equipConfig)
	{
		currencyType = SSanguoCurrencyType::Currency_Token;
		cost = pCommodityData->m_dwCount * equipConfig->buyTokenPrice;
		return true;
	}

	return false;
}

GoodsWay CLegionShopManager::RechargeWay() const
{
	return GoodsWay::LegionMall;
}

void CLegionShopManager::GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs)
{
	CConfigManager::getSingleton()->GetLegionShopCommodityConfigs(m_pBaseDataMgr.GetPlayerLevel(), vecConfigs);
}