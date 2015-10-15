#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <functional>
#include <algorithm>
#include <map>

#pragma warning (push)
#pragma warning (disable: 4800)
#include <boost/pool/pool_alloc.hpp>
#pragma warning (pop)

#include "EventUtility.h"

class EventBase;

// �¼������������������ڷ�������֡�٣��ṩ�����ܵľ���
class EventManager
{
	typedef std::multimap<DWORD, EventBase*, EventCompare, boost::fast_pool_allocator<DWORD> > EventList;

private:
	EventManager() : m_Locked(false) {}
	EventManager(EventManager &);

	~EventManager();

public:
	static EventManager& GetInstance();

public:
	void Run(DWORD overTime);
	void Register(EventBase *e, DWORD remain);
	bool Interrupt(EventBase *e);

private:	
	EventList	m_EventList;
	EventList	m_BackUpList;
	bool		m_Locked;
};
