#include "stdafx.h"
#include "VarietyShopManager.h"
#include "../Common/ConfigManager.h"
#include "NETWORKMODULE/SanguoPlayer.h"
#include "../BaseDataManager.h"
#include "MallsMoudle.h"

CVarietyShopManager::CVarietyShopManager(CBaseDataManager& baseDataMgr)
	: CMallManager(baseDataMgr)
{
}


CVarietyShopManager::~CVarietyShopManager()
{
}

MallType CVarietyShopManager::GetType() const
{
	return MallType::VarietyShop;
}

bool CVarietyShopManager::InitDataMgr(void * pData)
{
	if (nullptr == pData)
		return false;
	m_pMallData = static_cast<SSanguoMallData*>(pData);
	m_pVecRefreshClock = &CConfigManager::getSingleton()->globalConfig.vecVarietyShopRefreshClock;
	m_dwLimitLevel = CConfigManager::getSingleton()->GetGameFeatureActivationConfig().shoppingLevelLimit;
	if (JudgementRefresh(m_pBaseDataMgr.GetTimeData()))
		CMallsMoudle::getSingleton()->RandomCommoditys(this);
	SetRefreshInfo(false);
	return true;
}

GoodsWay CVarietyShopManager::RechargeWay() const
{
	return GoodsWay::varietyMall;
}

void CVarietyShopManager::GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs)
{
	CConfigManager::getSingleton()->GetVarietyShopCommodityConfigs(m_pBaseDataMgr.GetPlayerLevel(), vecConfigs);
}
