// ----- MiracleMerchantManager.h -----
//	---	Author : LiuWeiWei ---
//  --- Date : 03/16/2015 ---
//  --- Desc : 奇缘商人管理类 ---

#pragma once
#include "MallManager.h"
#include <memory>
struct SSanguoSpecialMallData;
struct SOpenMallMsg;
struct SRefreshCommodityResult;

class MiracleMerchantManager
	: public CMallManager
{
public:
	MiracleMerchantManager(CBaseDataManager& baseDataManager);
	virtual ~MiracleMerchantManager();
	virtual MallType GetType() const;
	virtual bool InitDataMgr(void * pData);
	virtual GoodsWay RechargeWay() const;
	virtual bool IsOpened();
	virtual void VipLevelChange(int vipLevel);
	virtual void LoginProcess();
	void OpenProcess();
	virtual void GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs);
private:
	static DWORD m_dwOpenVIPLevel;		///< 商城开启条件之VIP等级
	static DWORD m_dwOpenMasterLevel;	///< 商城开启条件之君主等级
	static float m_fOpenProbability;	///< 商城开启条件之几率
	SSanguoSpecialMallData *m_pSpecialMallData;
	__time64_t m_tEffectiveTime;			///< 生效时间
	shared_ptr<SOpenMallMsg> m_pOpenMsg;	///< 商城开启消息
	shared_ptr<SRefreshCommodityResult> m_pRefreshCommodityMsg;	///刷新商品消息
};

