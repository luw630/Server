#include "StdAfx.h"
#include "BatchReadyEvent.h"
#include "../EventMgr.h"

#pragma region TEST_READY_EVENT_LIST
//[���¼��б����ó�һ���б�]

 // ����һ���¼�
Event::ReadyEvent *BatchReadyEvent::SetReadyEvent(Event::ReadyEvent * pEvent, DWORD time)
{
	if (!pEvent)
	{
		rfalse(4, 1, "Itemuser.cpp - SetReadyEvent() - !pEvent");
		return 0;
	}

	// ��⵽����г�ͻ���¼�����Ҫ���ж�֮ǰ���¼�
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
// �ر�һ���¼�	
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

// �ж��¼�����
BOOL BatchReadyEvent::BreakOperationReadyEvent(DWORD dwBreakType)
{
	if (m_listReadyEvent.size() == 0)
		return FALSE;

	size_t countOld = m_listReadyEvent.size();	
	for (ReadyEventLIST::iterator iter = m_listReadyEvent.begin(); iter != m_listReadyEvent.end(); ) 
	{
		if (*iter != NULL)		
		{				
			// ע�����Ĺ�������Щ��
			if ((*iter)->InterruptReadyEvent(dwBreakType)) 
			{	
				//����ҪSAFE_DELETE(pEvent)�ˣ���ΪInterruptReadyEvent Ϊtrue��ʱ���Ѿ�ɾ���ˡ�
				// �������Ϊtrue�Ļ�������OnCloseReadyEventɾ����Ԫ�أ����Դ˴�iter�Ѿ�ʧЧ��
				//�����ã�iter = m_listReadyEvent.erase(iter);//�����ã�m_listReadyEvent.erase(iter++);
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

// �ж��Ƿ����ĳ�����ֵ��¼�
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