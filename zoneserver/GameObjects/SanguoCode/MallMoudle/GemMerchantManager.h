// ----- GemMerchantManager.h -----
//	---	Author : LiuWeiWei ---
//  --- Date : 03/16/2015 ---
//  --- Desc : �䱦���˹����� ---

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
	/// �жϲ������̳��Ƿ񿪷�
	virtual bool IsOpened();
	virtual void VipLevelChange(int vipLevel);
	virtual void LoginProcess();
	/// �����̳ǿ����߼�
	void OpenProcess();
	virtual void GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs);
private:
	static DWORD m_dwOpenVIPLevel;		///< �̳ǿ�������֮VIP�ȼ�
	static DWORD m_dwOpenMasterLevel;	///< �̳ǿ�������֮�����ȼ�
	static float m_fOpenProbability;	///< �̳ǿ�������֮����
	SSanguoSpecialMallData *m_pSpecialMallData;		///< �̳�����
	__time64_t m_tEffectiveTime;			///< ��Чʱ��
	shared_ptr<SOpenMallMsg> m_pOpenMsg;	///< �̳ǿ�����Ϣ
	shared_ptr<SRefreshCommodityResult> m_pRefreshCommodityMsg;	///ˢ����Ʒ��Ϣ
};

