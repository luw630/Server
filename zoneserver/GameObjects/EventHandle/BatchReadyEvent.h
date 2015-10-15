#pragma once
#include "../EventTimer.h"

#pragma region TEST_READY_EVENT_LIST
typedef std::list<Event::ReadyEvent*> ReadyEventLIST;
class BatchReadyEvent
{
	//[将事件列表设置成一个列表]
public:	
	Event::ReadyEvent *SetReadyEvent(Event::ReadyEvent * pEvent, DWORD time); // 设置一个事件
	void OnCloseReadyEvent(Event::ReadyEvent * pEvent); // 关闭一个事件	
	BOOL BreakOperationReadyEvent(DWORD iBreakType); // 中断事件类型
	BOOL IsContainsReadyEvent(const char *pEventName);// 判断是否包含某个名字的事件
protected:
	ReadyEventLIST m_listReadyEvent; // 事件列表
};

#pragma endregion TEST_READY_EVENT_LIST