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
		// ��ʾ�������͵�CD��δ�������ȴƵ������ô����
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
			// ���������ȶ���֡�٣���ĳЩ����£��п���CDʱ�䵽�ˣ�����û���ü����¼�����������
			// ����������£���ǰʱ���ȥ����ʱ���п���<=0���������������·���0����ô������˵
			// CD�Ѿ�Over����ô���п���Ҫ�ڴ˼��������ȴƵ������Active��ʱ�򣬱�������Ȼ������ȴ
			// ����Ϸ�߼��������Ӱ�죬����Ч�ʿ�����Ӱ�죬��������ֻ�Ǽ򵥵ķ���1������ȴ��������
			// ��һ֡������ض�OK
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