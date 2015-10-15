#include "stdafx.h"
#include "SoulExchangeManager.h"
#include "../Common/ConfigManager.h"
#include "NETWORKMODULE/SanguoPlayer.h"
#include "../BaseDataManager.h"
#include "MallsMoudle.h"

CSoulExchangeManager::CSoulExchangeManager(CBaseDataManager& baseDataMgr)
	: CMallManager(baseDataMgr)
{
}


CSoulExchangeManager::~CSoulExchangeManager()
{
}

MallType CSoulExchangeManager::GetType() const
{
	return MallType::SoulExchange;
}

bool CSoulExchangeManager::InitDataMgr(void * pData)
{
	if (nullptr == pData)
		return false;

	m_pMallData = static_cast<SSanguoMallData*>(pData);
	m_pVecRefreshClock = &CConfigManager::getSingleton()->globalConfig.vecSoulExchangeRefreshClock;
	m_dwLimitLevel = CConfigManager::getSingleton()->GetGameFeatureActivationConfig().soulExchangeLimit;
	if (JudgementRefresh(m_pBaseDataMgr.GetTimeData()))
		CMallsMoudle::getSingleton()->RandomCommoditys(this);
	SetRefreshInfo(false);
	return true;
}

GoodsWay CSoulExchangeManager::RechargeWay() const
{
	return GoodsWay::soulExchange;
}

bool CSoulExchangeManager::GetCommodityRechargeTypeAddCost(UINT index, OUT SSanguoCurrencyType& currencyType, OUT DWORD& cost) const
{
	auto pCommodityData = GetCommodityData(index);
	if (nullptr == pCommodityData)
		return false;

	auto itemConfig = CConfigManager::getSingleton()->GetItemConfig(pCommodityData->m_dwID);
	if (nullptr != itemConfig)
	{
		currencyType = SSanguoCurrencyType::Currency_SoulPoints;
		cost = pCommodityData->m_dwCount * itemConfig->BuySoulPointsCost;
		return true;
	}

	auto equipConfig = CConfigManager::getSingleton()->GetEquipment(pCommodityData->m_dwID);
	if (nullptr != equipConfig)
	{
		currencyType = SSanguoCurrencyType::Currency_SoulPoints;
		cost = pCommodityData->m_dwCount * equipConfig->BuySoulPointsCost;
		return true;
	}

	return false;
}

void CSoulExchangeManager::GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs)
{
	CConfigManager::getSingleton()->GetSoulExchangeCommodityConfigs(m_pBaseDataMgr.GetPlayerLevel(), vecConfigs);
}
