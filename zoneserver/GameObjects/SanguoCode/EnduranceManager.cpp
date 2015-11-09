#include "stdafx.h"
#include "EnduranceManager.h"
#include "../../NETWORKMODULE/SanguoPlayer.h"
#include "TimerEvent_SG\TimerEvent_SG.h"
#include "Common\ConfigManager.h"
#include "Common\PubTool.h"
#include "ScriptManager.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include <time.h>

CEnduranceManager::CEnduranceManager()
	: m_pEnduranceData(nullptr)
	, m_iRecoverSeconds(0)
	, m_iRecoverPoint(0)
	, m_iEnduranceLimit(0)
	, m_iExEnduranceLimit(0)
	, m_bGMMark(0)
	//, m_bInitFinished(false)
{
}


CEnduranceManager::~CEnduranceManager()
{
	m_pEnduranceData == nullptr;
}

void CEnduranceManager::Init(SEnduranceData* pData, bool bGMMark)
{
	if (nullptr == pData/* || m_bInitFinished*/)
		return;
	m_pEnduranceData = pData;
	m_bGMMark = bGMMark;
	//m_bInitFinished = true;
}

void CEnduranceManager::InitEnduranceData(bool newPlayer, const int32_t playerLevel, const DWORD sid)
{
	if (nullptr == m_pEnduranceData)
		return;

	lite::Variant ret;
	BOOL result = FALSE;
	if (g_Script.PrepareFunction("SI_vip_getDetail"))
	{
		g_Script.PushParameter(sid);
		g_Script.PushParameter(VipLevelFactor::VF_EnduranceLimit);
		result = g_Script.Execute(&ret);
	}

	if (!result || ret.dataType == LUA_TNIL)
	{
		rfalse("SetEnduranceForVIP Failed");
	}

	try
	{
		m_iExEnduranceLimit = static_cast<int>(ret);
	}
	catch (lite::Xcpt &e)
	{
		rfalse(2, 1, e.GetErrInfo());
	}

	m_iEnduranceLimit = CConfigManager::getSingleton()->globalConfig.PhysicalLimit;
	m_iRecoverSeconds = CConfigManager::getSingleton()->globalConfig.PhysicalRecoverTime;
	if (newPlayer)
	{
		_time64(&m_tLastCheckRecoverTime);
		ConverTool::ConvertInt64ToBytes(m_tLastCheckRecoverTime, m_pEnduranceData->m_latestCheckTime);
		PlusEndurance(m_iEnduranceLimit, playerLevel, m_bGMMark);
	}
	else
	{
		m_tLastCheckRecoverTime = *((__time64_t*)m_pEnduranceData->m_latestCheckTime);
		Check(playerLevel);
	}
}

int CEnduranceManager::Recover()
{
	if (nullptr == m_pEnduranceData)
		return 0;
	_time64(&m_tCurTime);
	int gapSeconds = (int)difftime(m_tCurTime, m_tLastCheckRecoverTime);
	gapSeconds += m_pEnduranceData->m_dwRemainingSeconds;
	m_iRecoverPoint = gapSeconds / m_iRecoverSeconds;
	m_pEnduranceData->m_dwRemainingSeconds = gapSeconds % m_iRecoverSeconds;
	m_tLastCheckRecoverTime = m_tCurTime;
	ConverTool::ConvertInt64ToBytes(m_tLastCheckRecoverTime, m_pEnduranceData->m_latestCheckTime);
	return m_iRecoverPoint;
}

void CEnduranceManager::PlusEndurance(const int32_t value, const int32_t playerLevel, bool globalLimit/* = true*/)
{
	if (nullptr == m_pEnduranceData)
		return;
	if (value + m_pEnduranceData->m_dwEndurance < 0)
	{
		m_pEnduranceData->m_dwEndurance = 0;
		return;
	}

	m_pEnduranceData->m_dwEndurance += value;
	int32_t enduranceLimit = GetEnduranceLimit(playerLevel, globalLimit);
	if (0 == enduranceLimit)
		return;
	m_pEnduranceData->m_dwEndurance = min(m_pEnduranceData->m_dwEndurance, enduranceLimit);
	if (m_pEnduranceData->m_dwEndurance >= GetEnduranceLimit(playerLevel, m_bGMMark))
		m_pEnduranceData->m_dwRemainingSeconds = 0;
}

bool CEnduranceManager::Check(const int32_t playerLevel, int32_t compareValue/* = 0*/)
{
	Recover();
	if (m_pEnduranceData->m_dwEndurance < GetEnduranceLimit(playerLevel, m_bGMMark))
		PlusEndurance(m_iRecoverPoint, playerLevel, m_bGMMark);
	else
		m_pEnduranceData->m_dwRemainingSeconds = 0;

	return m_pEnduranceData->m_dwEndurance >= compareValue ? true : false;
}

DWORD CEnduranceManager::GetEndurance() const
{
	return nullptr == m_pEnduranceData ? 0 : m_pEnduranceData->m_dwEndurance;
}

int32_t CEnduranceManager::GetEnduranceLimit(const int32_t playerLevel, bool globalLimit) const
{
	if (globalLimit)
		return m_iEnduranceLimit + m_iExEnduranceLimit;

	auto levelInfo = CConfigManager::getSingleton()->GetMasterLevelInfor(playerLevel);
	if (nullptr == levelInfo)
		return 0;
	return levelInfo->CharactorEnduranceLimit + m_iExEnduranceLimit;
}

void CEnduranceManager::UpdateEnduranceForVIP(const DWORD sid, const int32_t playerLevel)
{
	if (nullptr == m_pEnduranceData)
		return;

	Check(playerLevel);

	lite::Variant ret;
	BOOL result = FALSE;
	if (g_Script.PrepareFunction("SI_vip_getDetail"))
	{
		g_Script.PushParameter(sid);
		g_Script.PushParameter(VipLevelFactor::VF_EnduranceLimit);
		result = g_Script.Execute(&ret);
	}

	if (!result || ret.dataType == LUA_TNIL)
	{
		rfalse("SetEnduranceForVIP Failed");
	}

	try
	{
		m_iExEnduranceLimit = static_cast<int>(ret);
	}
	catch (lite::Xcpt &e)
	{
		rfalse(2, 1, e.GetErrInfo());
	}
}