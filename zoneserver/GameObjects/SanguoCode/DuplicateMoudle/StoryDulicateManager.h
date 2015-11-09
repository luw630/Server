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
#include <memory>

typedef std::function<void()> EmptyFunction;
class CDuplicateExtraRewards;

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
	///@brief ���컯�ĳ�ʼ���߼�
	virtual void SpecificInitialization();
	///@brief ����ͻ��˷�������Ϣ
	virtual void DispatchMsg(const SDuplicateMsg* pMsg);
	///@brief ���ͻ���������սĳһ���ؿ���ʱ�����������һЩ����
	virtual void OnAskToEnterTollgate();
	///@brief ���ͻ���׼������ս������ˢ�ֵ�ʱ��Ҫ����Ӧ�ؿ��ĵ�����Ϣ�������ͻ���
	virtual void OnStartToChallenge();
	///@brief ����Ƿ��ܽ���ɨ������
	virtual void CheckMopUpOperationPermission(const SDuplicateMsg *pMsg) final;
	///@brief ���������һЩ�������
	virtual void EarningClearingExtraProgress() final;
	///@brief ������صĻ�ڸ�������ʱҪ���Ĳ���
	virtual void ActivityClearingProgress();

	bool m_bMopUpOperation;		///<�Ƿ�Ϊɨ������
	bool m_bDuplicateActivated;	///<�����Ƿ��Ѿ�������
	int  m_iMopUpTicketID;
	std::vector<EmptyFunction> m_funcClearing;	///<����ص��¼�
	unique_ptr<CDuplicateExtraRewards> m_ptrDuplicateExtraRewards; ///<�ؿ�����Ľ�������
};

