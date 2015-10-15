#include "StdAfx.h"
#include "BatchReadyEvent.h"
#include "../EventMgr.h"

#pragma region TEST_READY_EVENT_LIST
//[将事件列表设置成一个列表]

 // 设置一个事件
Event::ReadyEvent *BatchReadyEvent::SetReadyEvent(Event::ReadyEvent * pEvent, DWORD time)
{
	if (!pEvent)
	{
		rfalse(4, 1, "Itemuser.cpp - SetReadyEvent() - !pEvent");
		return 0;
	}

	// 检测到如果有冲突的事件，需要先中断之前的事件
    for (ReadyEventLIST::iterator iter = m_listReadyEvent.begin(); iter != m_listReadyEvent.end(); ++iter) 
	{
        if (*iter == NULL) continue;		
		
		//if ((*iter).IsEvent(" "))
		//	tod.. BreakOperationReadyEvent();
    }
	
	m_listReadyEvent.push_back(pEvent);	
	EventManager::GetInstance().Register(pEvent, time);

	return pEvent;
}
// 关闭一个事件	
void BatchReadyEvent::OnCloseReadyEvent(Event::ReadyEvent * pEvent)
{		
	if (m_listReadyEvent.size() == 0)
		return ;
    for (ReadyEventLIST::iterator iter = m_listReadyEvent.begin(); iter != m_listReadyEvent.end(); ++iter) 
	{		
		if (*iter == pEvent)
		{
			m_listReadyEvent.erase(iter);
			break;
		}
	}	
}

// 中断事件类型
BOOL BatchReadyEvent::BreakOperationReadyEvent(DWORD dwBreakType)
{
	if (m_listReadyEvent.size() == 0)
		return FALSE;

	size_t countOld = m_listReadyEvent.size();	
	for (ReadyEventLIST::iterator iter = m_listReadyEvent.begin(); iter != m_listReadyEvent.end(); ) 
	{
		if (*iter != NULL)		
		{				
			// 注意这句的关联性有些高
			if ((*iter)->InterruptReadyEvent(dwBreakType)) 
			{	
				//不需要SAFE_DELETE(pEvent)了，因为InterruptReadyEvent 为true的时候已经删除了。
				// 这个函数为true的话调用了OnCloseReadyEvent删除该元素，所以此处iter已经失效了
				//不能用：iter = m_listReadyEvent.erase(iter);//不能用：m_listReadyEvent.erase(iter++);
				size_t coutNew = m_listReadyEvent.size();
				if (coutNew != countOld)
				{
					iter = m_listReadyEvent.begin();
					countOld = coutNew;
				}
				else 
				{
					MY_ASSERT(0);
					SAFE_DELETE(*iter);
					iter = m_listReadyEvent.erase(iter);
				}
				continue;
			}
		}	
		++iter;
	}
	return TRUE;
}	

// 判断是否包含某个名字的事件
BOOL BatchReadyEvent::IsContainsReadyEvent(const char *pEventName)
{
	if (m_listReadyEvent.size() == 0)
		return FALSE;
	for (ReadyEventLIST::iterator iter = m_listReadyEvent.begin(); iter != m_listReadyEvent.end(); ) 
	{
		if (*iter != NULL)		
		{			
			if ((*iter)->IsEvent(pEventName))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

#pragma endregion TEST_READY_EVENT_LIST