// ----- CStoryDulicateManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   �ȶ���������ͨ���¸����Ĺ�����
// --------------------------------------------------------------------
//   --  �������ĳ�ʼ���Լ���֤�߼��������߼���
//---------------------------------------------------------------------   
#pragma once
#include <functional>
#include "DuplicateBaseManager.h"
#include <vector>

typedef std::function<void()> EmptyFunction;

class CStoryDulicateManager : public CDuplicateBaseManager
{
public:
	CStoryDulicateManager(CExtendedDataManager& dataMgr);
	virtual ~CStoryDulicateManager();

	virtual void InitMgr();
	virtual void Update();
	///@brief ����������ͨ�������������¸����������µĸ����͹ؿ�
	virtual void ActiveTo(int tollgateID);
	virtual void BindClearingEvent(EmptyFunction& func) final;

protected:
	///@brief ����ͻ��˷�������Ϣ
	virtual void DispatchMsg(const SDuplicateMsg* pMsg);
	///@brief ���ͻ���������սĳһ���ؿ���ʱ�����������һЩ����
	virtual void OnAskToEnterTollgate();
	///@brief ���ͻ���׼������ս������ˢ�ֵ�ʱ��Ҫ����Ӧ�ؿ��ĵ�����Ϣ�������ͻ���
	virtual void OnStartToChallenge();
	///@brief ����Ƿ��ܽ���ɨ������
	virtual void CheckMopUpOperationPermission(const SDuplicateMsg *pMsg) final;
	virtual void EarningClearingExtraProgress();

	bool m_bMopUpOperation;		///<�Ƿ�Ϊɨ������
	bool m_bDuplicateActivated;	///<�����Ƿ��Ѿ�������
	int  m_iMopUpTicketID;
	std::vector<EmptyFunction> m_funcClearing;	///<����ص��¼�
};

