#include "Stdafx.h"
#include "EventMgr.h"
#include "EventBase.h"
#include "CoolDownSys.h"
#include <iostream>

using namespace std;

EventManager& EventManager::GetInstance()
{
	static EventManager instance;
	return instance;
}

EventManager::~EventManager()
{
	//20150130 wk �رշ�����ǰ����ʱ���ڴ�
	for (EventList::iterator it = m_EventList.begin(); it != m_EventList.end();)
	{
		EventBase *e = it->second;

		it = m_EventList.erase(it);
		e->SelfDestory();
	}
	for (EventList::iterator it = m_BackUpList.begin(); it != m_BackUpList.end();)
	{
		EventBase *e = it->second;

		it = m_BackUpList.erase(it);
		e->SelfDestory();
	}
	MY_ASSERT(m_EventList.empty());		// Ӧ���Ѿ�������ɾ���
}

void EventManager::Run(DWORD overTime)
{
	if (0 == overTime)
		return;

	m_Locked = true;

	DWORD curTime = timeGetTime();

	for (EventList::iterator it = m_EventList.begin(); it != m_EventList.end();)
	{
		EventBase *e = it->second;

		// ��������˵�һ��ʱ�仹δ�����¼����ͱ�ʾʱ�䵽�˵��¼���ȫ��������
		if (curTime < e->m_actTime)
			break;

		it = m_EventList.erase(it);
			
		// �˴������¼�
		e->OnActive();
		e->SelfDestory();

		// �ж��¼������Ƿ��Ѿ���ʱ
		if (INFINITE != overTime && ((DWORD)abs((int)(timeGetTime()-curTime)) > overTime))
			break;
	}

	// ��������б������¼�����ô����һ���ϲ����Ժ��Ż���
	if (m_BackUpList.size())
	{
		m_EventList.insert(m_BackUpList.begin(), m_BackUpList.end());
		m_BackUpList.clear();
	}

	m_Locked = false;

	return;
}

void EventManager::Register(EventBase *e, DWORD remain)
{
	MY_ASSERT(e && remain);

	e->m_actTime = timeGetTime() + remain;

	m_Locked ? m_BackUpList.insert(make_pair(e->m_actTime, e)) : m_EventList.insert(make_pair(e->m_actTime, e));

	return;
}

bool EventManager::Interrupt(EventBase *e)
{
	if (!e)
		return false;

	EventList::iterator startPos = m_EventList.lower_bound(e->m_actTime);
	EventList::iterator endPos   = m_EventList.upper_bound(e->m_actTime);

	while (startPos != endPos)
	{
		EventBase *temp = startPos->second;
		if (e == temp)
		{
			m_EventList.erase(startPos);

			// �˴��ж��¼�
			e->OnCancel();
			e->SelfDestory();

			return true;
		}
		else
			++startPos;
	}

	rfalse(2, 1, "[����]��Ҫ�жϵ��¼�%p�����ڣ���", e);
	return false;
}