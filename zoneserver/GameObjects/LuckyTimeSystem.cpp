#include "stdafx.h"
#include <time.h>
#include "player.h"
#include "LuckyTimeSystem.h"
#include "SanguoCode/TimerEvent_SG/TimerEvent_SG.h"
#include "SanguoCode/Common/ConfigManager.h"
#include "GameWorld.h" ///NOTE:暂时黑夜系统和GameWorld互相相关联，因为GameWorld是继承于CObjectManager的，此处黑夜系统可以仅与CObjectManager关联

CLuckyTimeSystem::CLuckyTimeSystem()
{
	///服务器一开服就是白天
	m_bIsLuckyTime = false;
	m_LuckyTimePoint = 0;
	m_funcLuckyTimeStart = std::bind(&CLuckyTimeSystem::LuckyTimeStart, this, 0);
	m_funcLuckyTimeEnd = std::bind(&CLuckyTimeSystem::LuckyTimeEnd, this, 0);
	m_ptrLuckyTimeStartEventHandler = nullptr;
	m_ptrLuckyTimeEndEventHandler = nullptr;
	///全局配置规定的时间过后，就开始黑夜降临
	m_ptrLuckyTimeStartEventHandler = TimerEvent_SG::SetCallback(m_funcLuckyTimeStart, CConfigManager::getSingleton()->globalConfig.LuckyTimeGap);
	m_LuckyTimePoint = time(NULL);
}


CLuckyTimeSystem::~CLuckyTimeSystem()
{
	if (m_ptrLuckyTimeStartEventHandler != nullptr)
	{
		m_ptrLuckyTimeStartEventHandler->Interrupt();
		m_ptrLuckyTimeStartEventHandler = nullptr;
	}

	if (m_ptrLuckyTimeEndEventHandler != nullptr)
	{
		m_ptrLuckyTimeEndEventHandler->Interrupt();
		m_ptrLuckyTimeEndEventHandler = nullptr;
	}
}

bool CLuckyTimeSystem::IsLuckyTime() const
{
	return m_bIsLuckyTime;
}

int CLuckyTimeSystem::LuckyTimeRemaining() const
{
	time_t curTime = time(NULL);
	if (m_bIsLuckyTime)
	{
		return max(CConfigManager::getSingleton()->globalConfig.LuckyTimePersistentTime - curTime + m_LuckyTimePoint, 0);
	}
	else
	{
		return max(CConfigManager::getSingleton()->globalConfig.LuckyTimeGap - curTime + m_LuckyTimePoint, 0);
	}
}

void CLuckyTimeSystem::LuckyTimeStart(int fieldHolder)
{
	m_bIsLuckyTime = true;
	///黑夜持续时间过了之后就开始白天
	m_ptrLuckyTimeEndEventHandler = TimerEvent_SG::SetCallback(m_funcLuckyTimeEnd, CConfigManager::getSingleton()->globalConfig.LuckyTimePersistentTime);
	m_LuckyTimePoint = time(NULL);
}

void CLuckyTimeSystem::LuckyTimeEnd(int fieldHolder)
{
	m_bIsLuckyTime = false;
	///全局配置规定的白天时间过后，就开始黑夜降临
	m_ptrLuckyTimeStartEventHandler = TimerEvent_SG::SetCallback(m_funcLuckyTimeStart, CConfigManager::getSingleton()->globalConfig.LuckyTimeGap);
	m_LuckyTimePoint = time(NULL);
}
