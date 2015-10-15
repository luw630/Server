// ----- CRandomAchieveDataManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/04/11
//   --  Desc:   萌斗三国的小型奖励的管理模块
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include <memory>

enum DessertType;
struct SMessage;
struct SDessertMsg;
struct SADessertBakedInfor;
class CBaseDataManager;
class CPlayer;

class CDessertSystem
{
public:
	CDessertSystem(CBaseDataManager& dataMgr);
	~CDessertSystem();

	///初始化跟本类相关的lua端数据
	void InitLuaData();
	void RecvMsg(const SMessage *pMsg);
	///重置每天可以领取的次数
	void ResetData();
	///开始往客户端发放奖励
	///@param bAvaliable  奖励是否可以领取
	///@param type 奖励的类型
	///@param dessertNum 奖励的数量
	void DispachDessert(bool bAvaliable, DessertType type, int dessertNum);

private:
	void RequestBakeDessert(const SDessertMsg* pMsg);
	void RequestClaimDessert(const SDessertMsg* pMsg);

	bool m_bJudgementFuncRegister;///<判断是否可以领取奖励的函数已经注册到lua函数中了
	int m_iDessertBakedCount;///<小额奖励在今天已经领取的次数
	CPlayer* m_pPlayer;
	CBaseDataManager& m_BaseDataManager;
	shared_ptr<SADessertBakedInfor> m_ptrDessertBakedInforMsg;
	shared_ptr<SDessertMsg> m_ptrDessertRefreshMsg;
};

