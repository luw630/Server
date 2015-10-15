#include "stdafx.h"
#include "MissionManager.h"
#include "..\Common\PubTool.h"
#include "..\Common\ConfigManager.h"
#include "MissionDataManager.h"
#include "..\BaseDataManager.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "NETWORKMODULE\SanguoPlayer.h"


CMissionManager::CMissionManager(CExtendedDataManager& dataMgr)
{
	m_ptrDataMgr = dynamic_cast<CMissionDataManager*>(&dataMgr);
}


CMissionManager::~CMissionManager()
{
	m_ptrDataMgr = nullptr;
}

void CMissionManager::RecvMsg(const SMessage *pMsg)
{
	if (m_ptrDataMgr == nullptr)
	{
		rfalse("任务管理数据为空");
		return;
	}

	const SMissionMsg* ptrMsg = static_cast<const SMissionMsg*>(pMsg);
	if (ptrMsg == nullptr)
	{
		rfalse("任务消息转换出错");
		return;
	}

	switch (ptrMsg->_protocol)
	{
	case SMissionMsg::REQUEST_ACCOMPLISH_MISSION:
		AskToAccomplishMission(ptrMsg);
		break;
	}
}

void CMissionManager::AskToAccomplishMission(const SMissionMsg* msg)
{
	const SQMissionAccomplished * ptrMsg = static_cast<const SQMissionAccomplished*>(msg);
	if (ptrMsg == nullptr)
	{
		rfalse("请求增加任务完成次数的消息为空");
		return;
	}

	m_ptrDataMgr->SetMissionAccompulished(ptrMsg->missionID);
}

void CMissionManager::UpdateDungeionMission(InstanceType type, int completeTimes)
{
	switch (type)
	{
	case InstanceType::Arena:
		m_ptrDataMgr->SetMissionCompleteTimes(MissionType::MT_ArenaMission, completeTimes);
		break;
	case InstanceType::Expedition:
		m_ptrDataMgr->SetMissionCompleteTimes(MissionType::MT_Expedition, completeTimes);
		break;
	case InstanceType::Story:
		m_ptrDataMgr->SetMissionCompleteTimes(MissionType::MT_Duplicate, completeTimes);
		break;
	case InstanceType::StoryElite:
		m_ptrDataMgr->SetMissionCompleteTimes(MissionType::MT_EliteDuplicate, completeTimes);
		///通关精英故事副本的次数也算在普通副本通关次数里面
		m_ptrDataMgr->SetMissionCompleteTimes(MissionType::MT_Duplicate, completeTimes);
		break;
	case InstanceType::TreasureHunting:
		m_ptrDataMgr->SetMissionCompleteTimes(MissionType::MT_TreasureHunting, completeTimes);
		break;
	case InstanceType::LeagueOfLegends:
		m_ptrDataMgr->SetMissionCompleteTimes(MissionType::MT_LeagueOfLegends, completeTimes);
		break;
	}
}

void CMissionManager::UpdateBlessingMission(int blessTimes)
{
	m_ptrDataMgr->SetMissionCompleteTimes(MissionType::MT_Blessing, blessTimes);
}

void CMissionManager::UpdateForgingMission(int forgingTimes)
{
	m_ptrDataMgr->SetMissionCompleteTimes(MissionType::MT_Forging, forgingTimes);
}

void CMissionManager::UpdateSkillUpgradeMission(int upgradeTimes)
{
	m_ptrDataMgr->SetMissionCompleteTimes(MissionType::MT_SkillUpgrade, upgradeTimes);
}

void CMissionManager::UpdateVipMission(int vipLevel)
{
	m_ptrDataMgr->SetMissionCompleteTimes(MissionType::MT_VIP, vipLevel);
}
