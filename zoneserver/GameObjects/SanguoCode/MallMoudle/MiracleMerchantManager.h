// ----- MiracleMerchantManager.h -----
//	---	Author : LiuWeiWei ---
//  --- Date : 03/16/2015 ---
//  --- Desc : ��Ե���˹����� ---

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
	static DWORD m_dwOpenVIPLevel;		///< �̳ǿ�������֮VIP�ȼ�
	static DWORD m_dwOpenMasterLevel;	///< �̳ǿ�������֮�����ȼ�
	static float m_fOpenProbability;	///< �̳ǿ�������֮����
	SSanguoSpecialMallData *m_pSpecialMallData;
	__time64_t m_tEffectiveTime;			///< ��Чʱ��
	shared_ptr<SOpenMallMsg> m_pOpenMsg;	///< �̳ǿ�����Ϣ
	shared_ptr<SRefreshCommodityResult> m_pRefreshCommodityMsg;	///ˢ����Ʒ��Ϣ
};

