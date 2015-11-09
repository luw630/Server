#include "stdafx.h"
#include "MissionManager.h"
#include "..\Common\PubTool.h"
#include "..\Common\ConfigManager.h"
#include "MissionDataManager.h"
#include "MissionActiveness.h"
#include "..\BaseDataManager.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "NETWORKMODULE\SanguoPlayer.h"


CMissionManager::CMissionManager(CExtendedDataManager& missionDataMgr, CExtendedDataManager& missionActivenessMgr)
{
	m_ptrMissionDataMgr = dynamic_cast<CMissionDataManager*>(&missionDataMgr);
	m_ptrMissionActivenessMgr = dynamic_cast<CMissionActiveness*>(&missionActivenessMgr);
}


CMissionManager::~CMissionManager()
{
	m_ptrMissionDataMgr = nullptr;
	m_ptrMissionActivenessMgr = nullptr;
}

void CMissionManager::RecvMsg(const SMessage *pMsg)
{
	if (m_ptrMissionDataMgr == nullptr || m_ptrMissionActivenessMgr == nullptr)
	{
		rfalse("�����������Ϊ��");
		return;
	}

	const SMissionMsg* ptrMsg = static_cast<const SMissionMsg*>(pMsg);
	if (ptrMsg == nullptr)
	{
		rfalse("������Ϣת������");
		return;
	}

	switch (ptrMsg->_protocol)
	{
	case SMissionMsg::REQUEST_ACCOMPLISH_MISSION:
		AskToAccomplishMission(ptrMsg);
		break;
	case SMissionMsg::REQUEST_ACTIVENESS_REWARDS:
		AskToClaimMissionActivenessRewards(ptrMsg);
		break;
	}
}

void CMissionManager::AskToAccomplishMission(const SMissionMsg* msg)
{
	const SQMissionAccomplished * ptrMsg = static_cast<const SQMissionAccomplished*>(msg);
	if (ptrMsg == nullptr)
	{
		rfalse("��������������ɴ�������ϢΪ��");
		return;
	}

	m_ptrMissionDataMgr->SetMissionAccompulished(ptrMsg->missionID);
}

void CMissionManager::AskToClaimMissionActivenessRewards(const SMissionMsg* msg)
{
	const SQActivenessRewards* ptrMsg = static_cast<const SQActivenessRewards*>(msg);
	if (ptrMsg == nullptr)
	{
		rfalse("������ȡ�����Ծ�Ƚ�������ϢΪ��");
		return;
	}

	///��ȡ�����Ծ�Ƚ������߼�����
	m_ptrMissionActivenessMgr->RequestClaimRewards(ptrMsg->activenessReswardsLevel);
}

void CMissionManager::UpdateDungeionMission(InstanceType type, int completeTimes)
{
	switch (type)
	{
	case InstanceType::Arena:
		m_ptrMissionDataMgr->SetMissionCompleteTimes(MissionType::MT_ArenaMission, completeTimes);
		break;
	case InstanceType::Expedition:
		m_ptrMissionDataMgr->SetMissionCompleteTimes(MissionType::MT_Expedition, completeTimes);
		break;
	case InstanceType::Story:
		m_ptrMissionDataMgr->SetMissionCompleteTimes(MissionType::MT_Duplicate, completeTimes);
		break;
	case InstanceType::StoryElite:
		m_ptrMissionDataMgr->SetMissionCompleteTimes(MissionType::MT_EliteDuplicate, completeTimes);
		///ͨ�ؾ�Ӣ���¸����Ĵ���Ҳ������ͨ����ͨ�ش�������
		m_ptrMissionDataMgr->SetMissionCompleteTimes(MissionType::MT_Duplicate, completeTimes);
		break;
	case InstanceType::TreasureHunting:
		m_ptrMissionDataMgr->SetMissionCompleteTimes(MissionType::MT_TreasureHunting, completeTimes);
		break;
	case InstanceType::LeagueOfLegends:
		m_ptrMissionDataMgr->SetMissionCompleteTimes(MissionType::MT_LeagueOfLegends, completeTimes);
		break;
	}
}

void CMissionManager::UpdateBlessingMission(int blessTimes)
{
	m_ptrMissionDataMgr->SetMissionCompleteTimes(MissionType::MT_Blessing, blessTimes);
}

void CMissionManager::UpdateForgingMission(int forgingTimes)
{
	m_ptrMissionDataMgr->SetMissionCompleteTimes(MissionType::MT_Forging, forgingTimes);
}

void CMissionManager::UpdateSkillUpgradeMission(int upgradeTimes)
{
	m_ptrMissionDataMgr->SetMissionCompleteTimes(MissionType::MT_SkillUpgrade, upgradeTimes);
}

void CMissionManager::UpdateVipMission(int vipLevel)
{
	m_ptrMissionDataMgr->SetMissionCompleteTimes(MissionType::MT_VIP, vipLevel);
}
