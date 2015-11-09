#include "stdafx.h"
#include "ExpeditionDataManager.h"
#include "..\Common\PubTool.h"
#include "..\BaseDataManager.h"
#include "..\Common\ConfigManager.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "../networkmodule/playertypedef.h"
#include "..\TimerEvent_SG\TimerEvent_SG.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "Player.h"
#include "ScriptManager.h"
extern LPIObject GetPlayerBySID(DWORD dwStaticID);

CExpeditionDataManager::CExpeditionDataManager(CBaseDataManager& baseDataMgr)
	:CExtendedDataManager(baseDataMgr),
	globalConfig(CConfigManager::getSingleton()->globalConfig)
{
	m_bPermissionGot = false;
	m_bEnemyMarching = false;
	m_dwAvaliableHostileCharacterNum = g_iExpeditionCharacterLimit;
	m_ptrData = nullptr;
	m_dwCurMaxAvaliableTickets = 0;
	m_iProceedsRiseState = 0;
	m_selectedHeroID.clear();
}


CExpeditionDataManager::~CExpeditionDataManager()
{
	m_ptrData = nullptr;
	m_bPermissionGot = false;
	m_bEnemyMarching = false;
}

bool CExpeditionDataManager::InitDataMgr(void * pData)
{
	m_ptrData = static_cast<SExpeditionData *>(pData);
	
	if (!_checkExpeditionDataAvaliable())
		return false;

	m_hostileCharacterList.clear();
	for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
	{
		if (m_ptrData->curHostileCharacterInfor[i].m_dwHeroID != 0)
			m_hostileCharacterList.insert(make_pair(m_ptrData->curHostileCharacterInfor[i].m_dwHeroID, &m_ptrData->curHostileCharacterInfor[i]));
	}

	///生成票数刷新点
	__time64_t timeNow = _time64(nullptr);
	__time64_t refreshTime_t = timeNow;
	tm refreshTm;
	errno_t err = _localtime64_s(&refreshTm, &timeNow);
	if (err != true)
	{
		refreshTm.tm_hour = 5;
		refreshTm.tm_min = 0;
		refreshTm.tm_sec = 0;
		refreshTime_t = _mktime64(&refreshTm);
	}

	///先判断上一次登出时间到现在有木有触发刷新事件
	__time64_t tLogoutTime = m_pBaseDataMgr.GetLogoutTime();
	if (RefreshJudgement::JudgeCrossed(tLogoutTime, refreshTime_t))
	{
		RefreshTicket(0);
	}

	return true;
}

void CExpeditionDataManager::RefreshTicket(int sendMsg /* = 0 */)
{
	if (sendMsg != 0)
	{
		SAExpeditionRefresh msgExpeditionTicketRefresh;
		g_StoreMessage(m_pBaseDataMgr.GetDNID(), &msgExpeditionTicketRefresh, sizeof(SAExpeditionRefresh));
	}

	if (!_checkExpeditionDataAvaliable())
		return;

	m_ptrData->curChallengeTimes = 0;
}

bool CExpeditionDataManager::ReleaseDataMgr()
{
	m_ptrData = nullptr;
	return true;
}

void CExpeditionDataManager::SetConquredState(bool state)
{
	if (!_checkExpeditionDataAvaliable())
		return;

	m_ptrData->bSuccessToConqureWholeExpedtion = state;
}

bool CExpeditionDataManager::GetConquredState()
{
	if (!_checkExpeditionDataAvaliable())
		return true;

	return m_ptrData->bSuccessToConqureWholeExpedtion;
}

void CExpeditionDataManager::SetHeroRewardState(bool state)
{
	if (!_checkExpeditionDataAvaliable())
		return;

	m_ptrData->bRewardedHero = state;
}

void CExpeditionDataManager::SetRewardsUnclaimedState(bool state)
{
	if (!_checkExpeditionDataAvaliable())
		return;

	m_ptrData->bRewardsUnclaimed = state;
}

void CExpeditionDataManager::SetExpeditionActiveState(bool state)
{
	if (!_checkExpeditionDataAvaliable())
		return;

	m_ptrData->expeditionActived = state;
}

void CExpeditionDataManager::SetCurExpeditionInstanceID(DWORD ID)
{
	if (!_checkExpeditionDataAvaliable())
		return;

	m_ptrData->curExpeditionInstanceID = ID;
}

void CExpeditionDataManager::SetCurItemObtained(DWORD itemID)
{
	if (!_checkExpeditionDataAvaliable())
		return;

	m_ptrData->curItemObtained = itemID;
}

bool CExpeditionDataManager::GetHeroRewardState()
{
	if (!_checkExpeditionDataAvaliable())
		return false;

	return m_ptrData->bRewardedHero;
}

bool CExpeditionDataManager::GetRewardsUnclaimedState()
{
	if (!_checkExpeditionDataAvaliable())
		return true;

	return m_ptrData->bRewardsUnclaimed;
}

bool CExpeditionDataManager::GetExpeditionActiveState()
{
	if (!_checkExpeditionDataAvaliable())
		return false;

	return m_ptrData->expeditionActived;
}

bool CExpeditionDataManager::GetCurExpeditionInstanceID(OUT DWORD& data)
{
	if (!_checkExpeditionDataAvaliable())
		return false;

	data = m_ptrData->curExpeditionInstanceID;

	return true;
}

bool CExpeditionDataManager::GetCurTicketsNum(OUT DWORD& data)
{
	if (!_checkExpeditionDataAvaliable())
		return false;

	///暂时将VIP等级的获取方法放到这里,因为VIP的等级“随时”都会变，所以“实时”获取
	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_pBaseDataMgr.GetSID())->DynamicCast(IID_PLAYER);
	if (pPlayer != nullptr)
	{
		g_Script.SetCondition(0, pPlayer, 0);
		lite::Variant ret;//从lua获取购买技能次数
		LuaFunctor(g_Script, "SI_vip_getDetail")[m_pBaseDataMgr.GetSID()][VipLevelFactor::VF_KillPass_Num](&ret);
		m_dwCurMaxAvaliableTickets = max((int)ret, 0);
		g_Script.CleanCondition();
	}
	else
		rfalse("获取不到CPlayer的指针");

	if (m_ptrData->curChallengeTimes <= m_dwCurMaxAvaliableTickets)
		data = m_dwCurMaxAvaliableTickets - m_ptrData->curChallengeTimes;
	else
		data = 0;

	return true;
}

bool CExpeditionDataManager::GetCurItemObtained(OUT DWORD& data)
{
	if (!_checkExpeditionDataAvaliable())
		return false;

	data = m_ptrData->curItemObtained;

	return true;
}

SHeroData* CExpeditionDataManager::GetCurHostileCharacterInfor()
{
	if (!_checkExpeditionDataAvaliable())
		return nullptr;

	return m_ptrData->curHostileCharacterInfor;
}

bool CExpeditionDataManager::_checkExpeditionDataAvaliable()
{
	if (m_ptrData == nullptr)
	{
		rfalse("远征数据为空");
		return false;
	}

	return true;
}

void CExpeditionDataManager::CostATicket()
{
	if (!_checkExpeditionDataAvaliable())
		return;

	if (m_ptrData->curChallengeTimes < m_dwCurMaxAvaliableTickets)
	{
		++m_ptrData->curChallengeTimes;
		if (m_ptrData->curChallengeTimes > 1)
		{
			//wk 20150706 购买次数日志
			g_Script.CallFunc("db_gm_setoperation", m_pBaseDataMgr.GetSID(), 3, 2, 1);
		}
	}
}

void CExpeditionDataManager::ResetEnemyInfor()
{
	if (!_checkExpeditionDataAvaliable())
		return;

	m_hostileCharacterList.clear();
	for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
	{
		m_ptrData->curHostileCharacterInfor[i].m_bDeadInExpedition = true;
		m_ptrData->curHostileCharacterInfor[i].m_fExpeditionHealthScale = 0.0f;
		m_ptrData->curHostileCharacterInfor[i].m_fExpeditionManaScale = 0;
	}
}

bool CExpeditionDataManager::IsEnemyExist(DWORD heroID)
{
	if (m_hostileCharacterList.find(heroID) == m_hostileCharacterList.end())
		return false;

	return true;
}

void CExpeditionDataManager::CacheMarchedEnmey()
{
	m_hostileCharacterList.clear();
	for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
	{
		m_hostileCharacterList.insert(make_pair(m_ptrData->curHostileCharacterInfor[i].m_dwHeroID, &m_ptrData->curHostileCharacterInfor[i]));
	}
}

void CExpeditionDataManager::CacheSelectedHero(const DWORD* selectedHero)
{
	if (selectedHero == nullptr)
		return;

	m_selectedHeroID.clear();
	for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
	{
		if (selectedHero[i] != 0 && m_selectedHeroID.find(selectedHero[i]) == m_selectedHeroID.end())
		{
			m_selectedHeroID.insert(selectedHero[i]);
		}
	}
}

void CExpeditionDataManager::SetEnemyInfor(DWORD heroID, float healthScale, float manaScale)
{
	auto findResult = m_hostileCharacterList.find(heroID);
	if (findResult == m_hostileCharacterList.end() || findResult->second == nullptr)
		return;

	findResult->second->m_fExpeditionHealthScale = max(0, min(healthScale, 1));
	findResult->second->m_fExpeditionManaScale = max(0, min(manaScale, 1));
	if (healthScale == 0)
	{
		findResult->second->m_bDeadInExpedition = true;
		findResult->second->m_fExpeditionManaScale = 0;
	}
}

void CExpeditionDataManager::SetCurLevelFactor(float factor)
{
	if (!_checkExpeditionDataAvaliable())
		return;

	m_ptrData->levelFactor = factor;
}

void CExpeditionDataManager::SetPermissionGotState(bool state)
{
	m_bPermissionGot = state;
}

void CExpeditionDataManager::SetEnemyMarchingState(bool state)
{
	m_bEnemyMarching = state;
}

bool CExpeditionDataManager::GetPermissionState()
{
	return m_bPermissionGot;
}

bool CExpeditionDataManager::GetEnemyMarchingState()
{
	return m_bEnemyMarching;
}

bool CExpeditionDataManager::CheckCompletelyAnnihilated()
{
	bool bAnnihilated = true;
	if (m_selectedHeroID.size() == 0)
		bAnnihilated = false;
	const SHeroData* heroData = nullptr;
	for (auto itor : m_selectedHeroID)
	{
		if (itor == 0)
			break;

		heroData = m_pBaseDataMgr.GetHero(itor);
		if (heroData != nullptr)
		{
			if (!heroData->m_bDeadInExpedition)
			{
				bAnnihilated = false;
				break;
			}
		}
		else
		{
			rfalse("远征关卡中，选择的英雄不存在");
			break;
		}
	}

	return bAnnihilated;
}

bool CExpeditionDataManager::IsHeroSelected(DWORD heroID)
{
	if (m_selectedHeroID.find(heroID) != m_selectedHeroID.end())
		return true;

	return false;
}

void CExpeditionDataManager::CacheCurProceedsRiseState()
{
	if (!_checkExpeditionDataAvaliable())
		return;

	m_iProceedsRiseState = 0;
	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_pBaseDataMgr.GetSID())->DynamicCast(IID_PLAYER);
	if (pPlayer != nullptr)
	{
		g_Script.SetCondition(0, pPlayer, 0);
		lite::Variant ret;//从lua获取是否提升远征奖励的标志
		LuaFunctor(g_Script, "SI_vip_getDetail")[g_Script.m_pPlayer->GetSID()][VipLevelFactor::VF_KillPassAdd_Num](&ret);
		m_iProceedsRiseState = (int)ret;
		g_Script.CleanCondition();
	}
	else
		rfalse("获取不到CPlayer的指针");
}

int CExpeditionDataManager::GetCurExpectedMoneyProceeds()
{
	if (!_checkExpeditionDataAvaliable())
		return -1;

	const ExpeditionInstanceInfor* ptrCurLevelInstanceInfor = CConfigManager::getSingleton()->GetSpecifyExpeditionInstanceConfig(m_pBaseDataMgr.GetMasterLevel(), m_ptrData->curExpeditionInstanceID);
	if (ptrCurLevelInstanceInfor == nullptr)
		return -1;

	return ptrCurLevelInstanceInfor->moneyProceeds * m_iProceedsRiseState / 100;
}

int CExpeditionDataManager::GetCurExpectedExploitProceeds()
{
	if (!_checkExpeditionDataAvaliable())
		return -1;

	const ExpeditionInstanceInfor* ptrCurLevelInstanceInfor = CConfigManager::getSingleton()->GetSpecifyExpeditionInstanceConfig(m_pBaseDataMgr.GetMasterLevel(), m_ptrData->curExpeditionInstanceID);
	if (ptrCurLevelInstanceInfor == nullptr)
		return -1;

	return ptrCurLevelInstanceInfor->exploitProceeds * m_iProceedsRiseState / 100;
}