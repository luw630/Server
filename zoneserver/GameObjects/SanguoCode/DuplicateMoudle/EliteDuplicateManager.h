// ----- CEliteDuplicateManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   �ȶ������ľ�Ӣ����������
// --------------------------------------------------------------------
//   --  �������ĳ�ʼ���Լ���֤�߼��������߼���
//---------------------------------------------------------------------   
#pragma once
#include <memory>
#include "StoryDulicateManager.h"
using namespace std;

struct SAActivateDuplicate;

class CEliteDuplicateManager : public CStoryDulicateManager
{
public:
	CEliteDuplicateManager(CExtendedDataManager& dataMgr);
	virtual ~CEliteDuplicateManager();

protected:
	///@brief ���컯�ĳ�ʼ���߼�
	virtual void SpecificInitialization();
	///@brief ����ͻ��˷�������Ϣ
	virtual void DispatchMsg(const SDuplicateMsg* pMsg) final;
	///@brief ���ͻ���������սĳһ���ؿ���ʱ�����������һЩ����
	virtual void OnAskToEnterTollgate();
	///@brief ������صĻ�ڸ�������ʱҪ���Ĳ���
	virtual void ActivityClearingProgress();
	///@brief �������������ҿ�����ս
	void Activate();
	///@brief "����ĳһ���ؿ�����ս����"���ܵ�Ԫ�����ѻ�ȡ
	///@return �ɹ��Żض�Ӧ��ֵ��ʧ�ܷ���-1
	virtual int GetCostOfResetSpecifyTollgateChallengedTimes(int tollgateID);

private:
};

