#pragma once
#include "MallManager.h"
class CVarietyShopManager :
	public CMallManager
{
public:
	CVarietyShopManager(CBaseDataManager& baseDataMgr);
	virtual ~CVarietyShopManager();
	virtual MallType GetType() const;
	virtual bool InitDataMgr(void * pData);
	virtual GoodsWay RechargeWay() const;
	virtual void GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs);
};

