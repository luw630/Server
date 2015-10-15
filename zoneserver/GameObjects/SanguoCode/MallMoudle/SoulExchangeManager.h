#pragma once
#include "MallManager.h"
class CSoulExchangeManager :
	public CMallManager
{
public:
	CSoulExchangeManager(CBaseDataManager& baseDataMgr);
	virtual ~CSoulExchangeManager();

	virtual MallType GetType() const;
	virtual bool InitDataMgr(void * pData);
	virtual bool GetCommodityRechargeTypeAddCost(UINT index, OUT SSanguoCurrencyType& currencyType, OUT DWORD& cost) const;
	virtual GoodsWay RechargeWay() const;
	virtual void GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs);
};

