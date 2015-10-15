// ----- GemMerchantManager.h -----
//	---	Author : LiuWeiWei ---
//  --- Date : 03/16/2015 ---
//  --- Desc : 珍宝商人管理类 ---

#pragma once
#include "MallManager.h"
#include <memory>
struct SSanguoSpecialMallData;
struct SOpenMallMsg;
struct SRefreshCommodityResult;

class CGemMerchantManager :
	public CMallManager
{
public:
	CGemMerchantManager(CBaseDataManager& baseDataManager);
	virtual ~CGemMerchantManager();
	virtual MallType GetType() const;
	virtual bool InitDataMgr(void * pData);
	virtual GoodsWay RechargeWay() const;
	/// 判断并计算商城是否开放
	virtual bool IsOpened();
	virtual void VipLevelChange(int vipLevel);
	virtual void LoginProcess();
	/// 触发商城开放逻辑
	void OpenProcess();
	virtual void GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs);
private:
	static DWORD m_dwOpenVIPLevel;		///< 商城开启条件之VIP等级
	static DWORD m_dwOpenMasterLevel;	///< 商城开启条件之君主等级
	static float m_fOpenProbability;	///< 商城开启条件之几率
	SSanguoSpecialMallData *m_pSpecialMallData;		///< 商城数据
	__time64_t m_tEffectiveTime;			///< 生效时间
	shared_ptr<SOpenMallMsg> m_pOpenMsg;	///< 商城开启消息
	shared_ptr<SRefreshCommodityResult> m_pRefreshCommodityMsg;	///刷新商品消息
};

