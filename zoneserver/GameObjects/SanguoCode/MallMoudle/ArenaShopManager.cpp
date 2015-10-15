#include "stdafx.h"
#include "ArenaShopManager.h"
#include "../Common/ConfigManager.h"
#include "NETWORKMODULE/SanguoPlayer.h"
#include "../BaseDataManager.h"
#include "MallsMoudle.h"

CArenaShopManager::CArenaShopManager(CBaseDataManager& baseDataMgr)
	: CMallManager(baseDataMgr)
{
}


CArenaShopManager::~CArenaShopManager()
{
}

MallType CArenaShopManager::GetType() const
{
	return MallType::ArenaShop;
}

bool CArenaShopManager::InitDataMgr(void * pData)
{
	if (nullptr == pData)
		return false;
	m_pMallData = static_cast<SSanguoMallData*>(pData);
	m_pVecRefreshClock = &CConfigManager::getSingleton()->globalConfig.vecArenaShopRefreshClock;
	m_dwLimitLevel = CConfigManager::getSingleton()->GetGameFeatureActivationConfig().arenaLevelLimit;
	if (JudgementRefresh(m_pBaseDataMgr.GetTimeData()))
		CMallsMoudle::getSingleton()->RandomCommoditys(this);
	SetRefreshInfo(false);
	return true;
}

bool CArenaShopManager::GetCommodityRechargeTypeAddCost(UINT index, OUT SSanguoCurrencyType& currencyType, OUT DWORD& cost) const
{
	auto pCommodityData = GetCommodityData(index);
	if (nullptr == pCommodityData)
		return false;

	auto itemConfig = CConfigManager::getSingleton()->GetItemConfig(pCommodityData->m_dwID);
	if (nullptr != itemConfig)
	{
		currencyType = SSanguoCurrencyType::Currency_Honor;
		cost = pCommodityData->m_dwCount * itemConfig->BuyHonorPrice;
		return true;
	}

	auto equipConfig = CConfigManager::getSingleton()->GetEquipment(pCommodityData->m_dwID);
	if (nullptr != equipConfig)
	{
		currencyType = SSanguoCurrencyType::Currency_Honor;
		cost = pCommodityData->m_dwCount * equipConfig->buyHonorPrice;
		return true;
	}

	return false;
}

GoodsWay CArenaShopManager::RechargeWay() const
{
	return GoodsWay::honorMall;
}

void CArenaShopManager::GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs)
{
	CConfigManager::getSingleton()->GetArenaShopCommodityConfigs(m_pBaseDataMgr.GetPlayerLevel(), vecConfigs);
}
