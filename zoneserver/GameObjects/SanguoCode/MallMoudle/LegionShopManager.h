// ----- LegionShopManager.h -----
//	---	Author : LiuWeiWei ---
//  --- Date : 03/16/2015 ---
//  --- Desc : 军团商店管理类 ---

#pragma once
#include "MallManager.h"

class CLegionShopManager :
	public CMallManager
{
public:
	CLegionShopManager(CBaseDataManager& baseDataManager);
	virtual ~CLegionShopManager();
	virtual MallType GetType() const;
	virtual bool InitDataMgr(void * pData);
	virtual bool GetCommodityRechargeTypeAddCost(UINT index, OUT SSanguoCurrencyType& currencyType, OUT DWORD& cost) const;
	virtual GoodsWay RechargeWay() const;
	virtual void GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs);
};

