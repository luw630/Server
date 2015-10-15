// ----- CRandomAchieveDataManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/04/11
//   --  Desc:   �ȶ�������С�ͽ����Ĺ���ģ��
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

	///��ʼ����������ص�lua������
	void InitLuaData();
	void RecvMsg(const SMessage *pMsg);
	///����ÿ�������ȡ�Ĵ���
	void ResetData();
	///��ʼ���ͻ��˷��Ž���
	///@param bAvaliable  �����Ƿ������ȡ
	///@param type ����������
	///@param dessertNum ����������
	void DispachDessert(bool bAvaliable, DessertType type, int dessertNum);

private:
	void RequestBakeDessert(const SDessertMsg* pMsg);
	void RequestClaimDessert(const SDessertMsg* pMsg);

	bool m_bJudgementFuncRegister;///<�ж��Ƿ������ȡ�����ĺ����Ѿ�ע�ᵽlua��������
	int m_iDessertBakedCount;///<С����ڽ����Ѿ���ȡ�Ĵ���
	CPlayer* m_pPlayer;
	CBaseDataManager& m_BaseDataManager;
	shared_ptr<SADessertBakedInfor> m_ptrDessertBakedInforMsg;
	shared_ptr<SDessertMsg> m_ptrDessertRefreshMsg;
};

