#pragma once
#include "MallManager.h"
class CArenaShopManager :
	public CMallManager
{
public:
	CArenaShopManager(CBaseDataManager& baseDataMgr);
	virtual ~CArenaShopManager();
	virtual MallType GetType() const;
	virtual bool InitDataMgr(void * pData);
	virtual bool GetCommodityRechargeTypeAddCost(UINT index, OUT SSanguoCurrencyType& currencyType, OUT DWORD& cost) const;
	virtual GoodsWay RechargeWay() const;
	virtual void GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs);
};

