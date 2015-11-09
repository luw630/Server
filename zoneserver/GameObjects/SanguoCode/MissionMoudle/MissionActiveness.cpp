#include "stdafx.h"
#include "MissionActiveness.h"
#include "..\BaseDataManager.h"
#include "..\Common\PubTool.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "../networkmodule/playertypedef.h"
#include "..\Common\ConfigManager.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "Player.h"
#include "ScriptManager.h"


CMissionActiveness::CMissionActiveness(CBaseDataManager& baseDataMgr)
	:CExtendedDataManager(baseDataMgr)
{
	m_dwMissionActiveness = nullptr;
	m_bInitResetMissionActiveness = false;
}

CMissionActiveness::~CMissionActiveness()
{
	m_dwMissionActiveness = nullptr;
}

bool CMissionActiveness::ReleaseDataMgr()
{
	m_dwMissionActiveness = nullptr;
	return true;
}

bool CMissionActiveness::InitDataMgr(void * pData)
{
	if (pData != nullptr)
		m_dwMissionActiveness = static_cast<DWORD*>(pData);

	///��ʼ�ж��Ƿ����������������
	__time64_t tCurTime = m_pBaseDataMgr.GetLogoutTime();
	__time64_t nextRefreshTime = tCurTime;

	__time64_t now = _time64(nullptr);
	tm curTime;
	_localtime64_s(&curTime, &now);
	curTime.tm_hour = 5;
	curTime.tm_min = 0;
	curTime.tm_sec = 0;
	nextRefreshTime = _mktime64(&curTime);

	if (RefreshJudgement::JudgeCrossed(tCurTime, nextRefreshTime))
	{
		m_bInitResetMissionActiveness = true;
	}

	return true;
}

void CMissionActiveness::OnLuaInitialed()
{
	if (m_bInitResetMissionActiveness)
	{
		m_bInitResetMissionActiveness = false;
		ResetMissionActivenessData(0);
	}
}

void CMissionActiveness::OnMissionAccomplished(int missionID)
{
	if (m_dwMissionActiveness == nullptr)
		return;
	
	const MissionConfig* config = CConfigManager::getSingleton()->GetMissionConfig(missionID);
	if (config == nullptr)
		return;

	///���¶�Ӧ�Ļ�Ծ�ȼӳ�
	*m_dwMissionActiveness += config->activenessGains;
	///����Ѵ�����ֵ�ˣ��򲻲���ͬ�����ͻ���
	if (CConfigManager::getSingleton()->IsMissionActivenessRestricted(*m_dwMissionActiveness))
		return;

	///��ʼͬ����Ծ�����ݵ��ͻ���
	SAUpdateMissionActiveness msg;
	msg.activenessData = *m_dwMissionActiveness;
	g_StoreMessage(m_pBaseDataMgr.GetDNID(), &msg, sizeof(SAUpdateMissionActiveness));
}

void CMissionActiveness::RequestClaimRewards(int activenessLevel)
{
	///��ȡ��Ӧ�Ľ���������Ϣ
	const MissionActivenessReswardsConfig* config = CConfigManager::getSingleton()->GetMissionActivenessRewardsConfig(activenessLevel);
	if (config == nullptr)
	{
		SAActivenessRewards msg;
		g_StoreMessage(m_pBaseDataMgr.GetDNID(), &msg, sizeof(SAActivenessRewards));
		return;
	}

	int retValue = 0;
	BOOL execResult = FALSE;
	lite::Variant ret1;//��lua��ȡ���ķ���ֵ
	//��lua���������Ƿ������ȡ����
	if (g_Script.PrepareFunction("ClaimMissionActivenessRewards"))
	{
		g_Script.PushParameter(m_pBaseDataMgr.GetSID());
		g_Script.PushParameter(activenessLevel);
		execResult = g_Script.Execute(&ret1);
	}

	retValue = 0;
	if (execResult && ret1.dataType != LUA_TNIL)
	{
		try
		{
			retValue = (int)ret1;
		}
		catch (lite::Xcpt& e)
		{
			retValue = 0;
			rfalse(2, 1, e.GetErrInfo());
		}
	}

	///�����ȡʧ�ܣ���ʾ�ͻ�����ȡʧ��
	if (retValue == 0)
	{
		SAActivenessRewards msg;
		g_StoreMessage(m_pBaseDataMgr.GetDNID(), &msg, sizeof(SAActivenessRewards));
		return;
	}

	int rewardsSize = config->rewardsInfor.size();
	///�����Ӧ�Ľ���
	for (int i = 0; i < rewardsSize; ++i)
	{
		m_pBaseDataMgr.AddGoods_SG(config->rewardsInfor[i].itype, config->rewardsInfor[i].id, config->rewardsInfor[i].num, GoodsWay::task);
	}

	///֪ͨ�ͻ�����ȡ���������
	SAActivenessRewards msg;
	msg.bSuccess = true;
	g_StoreMessage(m_pBaseDataMgr.GetDNID(), &msg, sizeof(SAActivenessRewards));
}

void CMissionActiveness::ResetMissionActivenessData(int sendMsgFlag /*= 0*/)
{
	///���û�Ծ��
	*m_dwMissionActiveness = 0;
	///����lua�˵�����
	////////////////////////////////////////////////////////////////////////////////////////////
	if (g_Script.PrepareFunction("ResetMissionActiveness"))
	{
		g_Script.PushParameter(m_pBaseDataMgr.GetSID());
		g_Script.Execute();
	}
	////////////////////////////////////////////////////////////////////////////////////////////

	///������Ҫ������Ϣ���ͻ���
	if (sendMsgFlag != 0)
	{
		SAUpdateMissionActiveness updateMsg;
		updateMsg.activenessData = 0;
		g_StoreMessage(m_pBaseDataMgr.GetDNID(), &updateMsg, sizeof(SAUpdateMissionActiveness));
	}
}
