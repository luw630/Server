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
	//20150130 wk 关闭服务器前清理定时器内存
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
	MY_ASSERT(m_EventList.empty());		// 应该已经被清理干净了
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

		// 如果发现了第一个时间还未到的事件，就表示时间到了的事件已全部处理完
		if (curTime < e->m_actTime)
			break;

		it = m_EventList.erase(it);
			
		// 此处激活事件
		e->OnActive();
		e->SelfDestory();

		// 判断事件处理是否已经超时
		if (INFINITE != overTime && ((DWORD)abs((int)(timeGetTime()-curTime)) > overTime))
			break;
	}

	// 如果后续列表中有事件，那么先做一个合并（以后优化）
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

			// 此处中断事件
			e->OnCancel();
			e->SelfDestory();

			return true;
		}
		else
			++startPos;
	}

	rfalse(2, 1, "[警告]：要中断的事件%p不存在！！", e);
	return false;
}