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

	///初始判断是否可以重置任务数据
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

	///更新对应的活跃度加成
	*m_dwMissionActiveness += config->activenessGains;
	///如果已达上限值了，则不不必同步到客户端
	if (CConfigManager::getSingleton()->IsMissionActivenessRestricted(*m_dwMissionActiveness))
		return;

	///开始同步活跃度数据到客户端
	SAUpdateMissionActiveness msg;
	msg.activenessData = *m_dwMissionActiveness;
	g_StoreMessage(m_pBaseDataMgr.GetDNID(), &msg, sizeof(SAUpdateMissionActiveness));
}

void CMissionActiveness::RequestClaimRewards(int activenessLevel)
{
	///获取对应的奖励配置信息
	const MissionActivenessReswardsConfig* config = CConfigManager::getSingleton()->GetMissionActivenessRewardsConfig(activenessLevel);
	if (config == nullptr)
	{
		SAActivenessRewards msg;
		g_StoreMessage(m_pBaseDataMgr.GetDNID(), &msg, sizeof(SAActivenessRewards));
		return;
	}

	int retValue = 0;
	BOOL execResult = FALSE;
	lite::Variant ret1;//从lua获取到的返回值
	//跟lua交互，看是否可以领取奖励
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

	///如果领取失败，提示客户端领取失败
	if (retValue == 0)
	{
		SAActivenessRewards msg;
		g_StoreMessage(m_pBaseDataMgr.GetDNID(), &msg, sizeof(SAActivenessRewards));
		return;
	}

	int rewardsSize = config->rewardsInfor.size();
	///添加相应的奖励
	for (int i = 0; i < rewardsSize; ++i)
	{
		m_pBaseDataMgr.AddGoods_SG(config->rewardsInfor[i].itype, config->rewardsInfor[i].id, config->rewardsInfor[i].num, GoodsWay::task);
	}

	///通知客户端领取奖励的情况
	SAActivenessRewards msg;
	msg.bSuccess = true;
	g_StoreMessage(m_pBaseDataMgr.GetDNID(), &msg, sizeof(SAActivenessRewards));
}

void CMissionActiveness::ResetMissionActivenessData(int sendMsgFlag /*= 0*/)
{
	///重置活跃度
	*m_dwMissionActiveness = 0;
	///重置lua端的数据
	////////////////////////////////////////////////////////////////////////////////////////////
	if (g_Script.PrepareFunction("ResetMissionActiveness"))
	{
		g_Script.PushParameter(m_pBaseDataMgr.GetSID());
		g_Script.Execute();
	}
	////////////////////////////////////////////////////////////////////////////////////////////

	///根据需要发送消息到客户端
	if (sendMsgFlag != 0)
	{
		SAUpdateMissionActiveness updateMsg;
		updateMsg.activenessData = 0;
		g_StoreMessage(m_pBaseDataMgr.GetDNID(), &updateMsg, sizeof(SAUpdateMissionActiveness));
	}
}
