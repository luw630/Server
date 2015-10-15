// ----- WarOfLeagueShopManager.h -----
//	---	Author : LiuWeiWei ---
//  --- Date : 09/6/2015 ---
//  --- Desc : 国战商店管理类 ---

#pragma once
#include "MallManager.h"
class CWarOfLeagueShopManager :
	public CMallManager
{
public:
	CWarOfLeagueShopManager(CBaseDataManager& baseDataManager);
	virtual ~CWarOfLeagueShopManager();
	virtual MallType GetType() const;
	virtual bool InitDataMgr(void * pData);
	virtual bool GetCommodityRechargeTypeAddCost(UINT index, OUT SSanguoCurrencyType& currencyType, OUT DWORD& cost) const;
	virtual GoodsWay RechargeWay() const;
	virtual void GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs);
};

