#pragma once
#include "../EventTimer.h"

#pragma region TEST_READY_EVENT_LIST
typedef std::list<Event::ReadyEvent*> ReadyEventLIST;
class BatchReadyEvent
{
	//[���¼��б����ó�һ���б�]
public:	
	Event::ReadyEvent *SetReadyEvent(Event::ReadyEvent * pEvent, DWORD time); // ����һ���¼�
	void OnCloseReadyEvent(Event::ReadyEvent * pEvent); // �ر�һ���¼�	
	BOOL BreakOperationReadyEvent(DWORD iBreakType); // �ж��¼�����
	BOOL IsContainsReadyEvent(const char *pEventName);// �ж��Ƿ����ĳ�����ֵ��¼�
protected:
	ReadyEventLIST m_listReadyEvent; // �¼��б�
};

#pragma endregion TEST_READY_EVENT_LIST