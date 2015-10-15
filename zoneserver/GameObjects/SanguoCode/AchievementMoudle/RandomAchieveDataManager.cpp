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
	///���ж�
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
		///�����ǰ��ʱ�䣬����һ�εǳ���ʱ������һ����ҹ��ʱ�䣬������ҵǳ������ʱ���ڣ��϶��а����ҹ�Ľ���
		if (now - tCurTime >= CConfigManager::getSingleton()->globalConfig.LuckyTimePersistentTime)
			memset(m_ptrAchievementUnitData, 0, sizeof(SAchivementUnitData));

		SetRemainingTimes(true, ptrLuckyTimeSystem->LuckyTimeRemaining());
	}
	else
	{
		///�ڰ����½�Ļ������������������
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

	///����Ѿ�����˸óɾͣ����衰�ٴ���ɡ�
	if (m_ptrAchievementUnitData->accompulished)
		return nullptr;

	const RandomAchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetRandomAchievementConfig(achievementID);
	if (achievementConfig == nullptr)
		return nullptr;

	///���������ɵĳɾ͵Ŀ��ŵȼ����ڵ�ǰ��ҵľ����ȼ����衰��ɡ�
	if (achievementConfig->accessibleLevel > m_pBaseDataMgr.GetMasterLevel())
		return nullptr;

	///����óɾ��Ѵ��ֻ��û�С���ȡ����������һ�εġ���ɡ�
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
	///�ȳ�ʼ�����еĳɾ�����
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
	///���Ѿ���ɵĴ�����Ϊ0
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
