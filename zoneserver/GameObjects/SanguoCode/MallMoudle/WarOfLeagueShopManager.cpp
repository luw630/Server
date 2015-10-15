#include "stdafx.h"
#include "WarOfLeagueShopManager.h"
#include "../Common/ConfigManager.h"
#include "NETWORKMODULE/SanguoPlayer.h"
#include "../BaseDataManager.h"
#include "MallsMoudle.h"

CWarOfLeagueShopManager::CWarOfLeagueShopManager(CBaseDataManager& baseDataManager)
	: CMallManager(baseDataManager)
{
}


CWarOfLeagueShopManager::~CWarOfLeagueShopManager()
{
}

MallType CWarOfLeagueShopManager::GetType() const
{
	return MallType::WarOfLeagueShop;
}

bool CWarOfLeagueShopManager::InitDataMgr(void* pData)
{
	if (nullptr == pData)
		return false;
	m_pMallData = static_cast<SSanguoMallData*>(pData);
	m_pVecRefreshClock = &CConfigManager::getSingleton()->globalConfig.vecWarOfLeagueShopRefreshClock;
	m_dwLimitLevel = 0; //CConfigManager::getSingleton()->GetGameFeatureActivationConfig().teamSiegeLevelLimit;
	if (JudgementRefresh(m_pBaseDataMgr.GetTimeData()))
		CMallsMoudle::getSingleton()->RandomCommoditys(this);
	SetRefreshInfo(false);
	return true;
}

bool CWarOfLeagueShopManager::GetCommodityRechargeTypeAddCost(UINT index, OUT SSanguoCurrencyType& currencyType, OUT DWORD& cost) const
{
	auto pCommodityData = GetCommodityData(index);
	if (nullptr == pCommodityData)
		return false;

	auto itemConfig = CConfigManager::getSingleton()->GetItemConfig(pCommodityData->m_dwID);
	if (nullptr != itemConfig)
	{
		currencyType = SSanguoCurrencyType::Currency_Prestige;
		cost = pCommodityData->m_dwCount * itemConfig->BuyPrestigePrice;
		return true;
	}

	auto equipConfig = CConfigManager::getSingleton()->GetEquipment(pCommodityData->m_dwID);
	if (nullptr != equipConfig)
	{
		currencyType = SSanguoCurrencyType::Currency_Prestige;
		cost = pCommodityData->m_dwCount * equipConfig->buyPrestigePrice;
		return true;
	}

	return false;
}

GoodsWay CWarOfLeagueShopManager::RechargeWay() const
{
	return GoodsWay::warOfLeague;
}

void CWarOfLeagueShopManager::GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs)
{
	CConfigManager::getSingleton()->GetWarOfLeagueShopCommodityConfigs(m_pBaseDataMgr.GetPlayerLevel(), vecConfigs);
}