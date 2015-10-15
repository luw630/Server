#include "stdafx.h"
#include "../networkmodule/playertypedef.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "RandomAchieveDataManager.h"
#include "..\Common\PubTool.h"
#include "..\BaseDataManager.h"
#include "..\Common\ConfigManager.h"
#include "..\..\LuckyTimeSystem.h"
#include "..\..\GameWorld.h"

CRandomAchieveDataManager::CRandomAchieveDataManager(CBaseDataManager& baseDataMgr)
	:CExtendedDataManager(baseDataMgr)
{
	m_ptrLuckyTimeFalling = nullptr;
	m_ptrAccomplishedTimes = nullptr;
	m_ptrCurRandomAchievementRemainingTime = nullptr;
	m_ptrAchievementUnitData = nullptr;
}


CRandomAchieveDataManager::~CRandomAchieveDataManager()
{
	ReleaseDataMgr();
}

bool CRandomAchieveDataManager::InitDataMgr(void * pData)
{
	SFixData* playerData = static_cast<SFixData*>(pData);
	if (playerData == nullptr)
		return false;

	m_ptrLuckyTimeFalling = &playerData->m_bLuckyTimeFall;
	m_ptrAccomplishedTimes = &playerData->m_dwRandomAchieveAccomplishedTimes;
	m_ptrCurRandomAchievementRemainingTime = &playerData->m_RandomAchievementRemainingTime;
	m_ptrAchievementUnitData = &playerData->m_RandomAchievementData;

	__time64_t now = _time64(nullptr);
	///先判断
	__time64_t tCurTime = m_pBaseDataMgr.GetLogoutTime();
	__time64_t nextRfreshTime = tCurTime;

	tm curTime;
	_localtime64_s(&curTime, &now);
	curTime.tm_hour = 5;
	curTime.tm_min = 0;
	curTime.tm_sec = 0;
	nextRfreshTime = _mktime64(&curTime);

	if (RefreshJudgement::JudgeCrossed(tCurTime, nextRfreshTime))
	{
		RefreshData();
	}

	CLuckyTimeSystem* ptrLuckyTimeSystem = nullptr;
	GetGW()->GetLuckySystemPtr(&ptrLuckyTimeSystem);
	if (ptrLuckyTimeSystem != nullptr && ptrLuckyTimeSystem->IsLuckyTime())
	{
		///如果当前的时间，离上一次登出的时间差超过了一个黑夜的时间，则在玩家登出的这段时间内，肯定有白天黑夜的交替
		if (now - tCurTime >= CConfigManager::getSingleton()->globalConfig.LuckyTimePersistentTime)
			memset(m_ptrAchievementUnitData, 0, sizeof(SAchivementUnitData));

		SetRemainingTimes(true, ptrLuckyTimeSystem->LuckyTimeRemaining());
	}
	else
	{
		///在白天登陆的话则清空随机任务的数据
		memset(m_ptrAchievementUnitData, 0, sizeof(SAchivementUnitData));
		if (ptrLuckyTimeSystem != nullptr)
		{
			SetRemainingTimes(false, ptrLuckyTimeSystem->LuckyTimeRemaining());
		}
		else
		{
			SetRemainingTimes(false, CConfigManager::getSingleton()->globalConfig.LuckyTimeGap);
		}
	}
}

bool CRandomAchieveDataManager::ReleaseDataMgr()
{
	m_ptrLuckyTimeFalling = nullptr;
	m_ptrAccomplishedTimes = nullptr;
	m_ptrCurRandomAchievementRemainingTime = nullptr;
	m_ptrAchievementUnitData = nullptr;
	return false;
}

const SAchivementUnitData* CRandomAchieveDataManager::SetAchievementCompleteTimes(DWORD achievementID, int times /* = 1 */)
{
	if (m_ptrAchievementUnitData == nullptr || m_ptrAchievementUnitData->achievementID != achievementID)
		return nullptr;

	///如果已经完成了该成就，则不予“再次完成”
	if (m_ptrAchievementUnitData->accompulished)
		return nullptr;

	const RandomAchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetRandomAchievementConfig(achievementID);
	if (achievementConfig == nullptr)
		return nullptr;

	///如果随机生成的成就的开放等级大于当前玩家的君主等级则不予“完成”
	if (achievementConfig->accessibleLevel > m_pBaseDataMgr.GetMasterLevel())
		return nullptr;

	///如果该成就已达成只是没有“领取”，则不予再一次的“完成”
	if (m_ptrAchievementUnitData->completedTimes != 0 && achievementConfig->param2 <= m_ptrAchievementUnitData->completedTimes)
	{
		return nullptr;
	}

	m_ptrAchievementUnitData->completedTimes += times;
	if (m_ptrAchievementUnitData->completedTimes != 0 && m_ptrAchievementUnitData->completedTimes >= achievementConfig->param2)
	{
		if (achievementConfig->param2 != 0)
			m_ptrAchievementUnitData->completedTimes = achievementConfig->param2;
	}

	return m_ptrAchievementUnitData;
}

void CRandomAchieveDataManager::SetAchievementAccompulished(DWORD achievementID)
{
	if (m_ptrAchievementUnitData == nullptr || m_ptrAchievementUnitData->achievementID != achievementID)
		return;

	m_ptrAchievementUnitData->accompulished = true;
}

void CRandomAchieveDataManager::SetNewAchievement(DWORD achievementID)
{
	if (m_ptrAchievementUnitData == nullptr || m_ptrLuckyTimeFalling == nullptr || m_ptrAccomplishedTimes == nullptr)
		return;

	const RandomAchievementConfig* config = CConfigManager::getSingleton()->GetRandomAchievementConfig(achievementID);
	if (config == nullptr)
		return;

	*m_ptrLuckyTimeFalling = 1;
	*m_ptrAccomplishedTimes += 1;
	///先初始化现有的成就数据
	m_ptrAchievementUnitData->achievementID = achievementID;
	m_ptrAchievementUnitData->accompulished = 0;
	m_ptrAchievementUnitData->completedTimes = 0;
	m_ptrAchievementUnitData->groupType = config->achieveType;
}

void CRandomAchieveDataManager::SetRemainingTimes(bool bLuckyTime, DWORD remainingTime)
{
	if (m_ptrLuckyTimeFalling == nullptr || m_ptrCurRandomAchievementRemainingTime == nullptr)
		return;

	*m_ptrLuckyTimeFalling = bLuckyTime ? 1 : 0;
	*m_ptrCurRandomAchievementRemainingTime = remainingTime;
}

const SAchivementUnitData* CRandomAchieveDataManager::GetAchievementUnitData(DWORD achievementID)
{
	if (m_ptrAchievementUnitData == nullptr || m_ptrAchievementUnitData->achievementID != achievementID)
		return nullptr;

	return m_ptrAchievementUnitData;
}

void CRandomAchieveDataManager::RefreshData(int sendMsgFlag /* = 0 */)
{
	///将已经完成的次数置为0
	if (m_ptrAccomplishedTimes != nullptr)
		*m_ptrAccomplishedTimes = 0;

	if (sendMsgFlag != 0)
	{
		SRandomAchievementMsg refreshMsg;
		refreshMsg._protocol = SRandomAchievementMsg::ANSWER_REFRESH_RANDOM_ACHIEVEMENT;
		g_StoreMessage(m_pBaseDataMgr.GetDNID(), &refreshMsg, sizeof(SRandomAchievementMsg));
	}
}

bool CRandomAchieveDataManager::IsAchievementExist(RandomAchievementType type, OUT int& achievementID)
{
	achievementID = 0;
	if (m_ptrAchievementUnitData == nullptr || m_ptrAchievementUnitData->groupType != type)
		return false;

	achievementID = m_ptrAchievementUnitData->achievementID;

	return true;
}

void CRandomAchieveDataManager::LogoutProcess()
{
	CLuckyTimeSystem* ptrLuckyTimeSystem = nullptr;
	GetGW()->GetLuckySystemPtr(&ptrLuckyTimeSystem);
	if ((ptrLuckyTimeSystem == nullptr || !ptrLuckyTimeSystem->IsLuckyTime()) && m_ptrAchievementUnitData != nullptr)
	{
		memset(m_ptrAchievementUnitData, 0, sizeof(SAchivementUnitData));
	}
}
