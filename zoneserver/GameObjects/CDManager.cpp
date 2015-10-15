#include "Stdafx.h"
#include "CDManager.h"
#include "CoolDownSys.h"

CDManager::~CDManager()
{
	MY_ASSERT(m_cdMap.empty());
}

BOOL CDManager::Active(DWORD type, DWORD remain)
{
	if (0 == type || 0 == remain)
		return FALSE;

	CoolDownBase *cooler = 0;

	CDMap::iterator it = m_cdMap.find(type);
	if (it == m_cdMap.end())
	{
		// 表示此种类型的CD还未加入过冷却频道，那么加入
		cooler = new CoolDownBase(type, remain);
		m_cdMap[type] = cooler;
	}
	else
	{
		cooler = it->second;
		if (false == cooler->m_IsOver)
			return FALSE;
	}

	cooler->m_IsOver = false;
	EventManager::GetInstance().Register(cooler, cooler->m_Margin);

	return TRUE;
}

DWORD CDManager::Check(DWORD type)
{
	if (0 == type)
		return 0;

	CDMap::iterator it = m_cdMap.find(type);

	if (it != m_cdMap.end() &&	it->second	&& (false == it->second->m_IsOver))
	{
		DWORD curTime = timeGetTime();

		if (it->second->m_actTime > curTime)
			return it->second->m_actTime - curTime;
		else
			// 服务器有稳定的帧速，在某些情况下，有可能CD时间到了，但还没来得及被事件管理器处理
			// 在这种情况下，当前时间减去激活时间有可能<=0，如果在这种情况下返回0，那么理论上说
			// CD已经Over，那么就有可能要在此激活这个冷却频道，在Active的时候，被告诉仍然处于冷却
			// 对游戏逻辑不会产生影响，但对效率可能有影响，所以这里只是简单的返回1告诉冷却即将结束
			// 下一帧的请求必定OK
			return 1;
	}
	
	return 0;
}

BOOL CDManager::Deactive(DWORD type)
{
	CDMap::iterator it = m_cdMap.find(type);

	if (it != m_cdMap.end() &&	it->second && (false == it->second->m_IsOver))
	{
		EventManager::GetInstance().Interrupt(it->second);
		return TRUE;
	}

	return FALSE;
}

void CDManager::Free()
{
	for (CDMap::iterator it = m_cdMap.begin(); it != m_cdMap.end(); ++it)
	{
		if (it->second)
		{
			if (false == it->second->m_IsOver)
				EventManager::GetInstance().Interrupt(it->second);
			delete it->second;
			it->second = 0;
		}
	}

	m_cdMap.clear();
}