#include "stdafx.h"
#include "MissionDataManager.h"
#include "..\BaseDataManager.h"
#include "..\Common\PubTool.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "../networkmodule/playertypedef.h"
#include "..\Common\ConfigManager.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "Player.h"
#include "ScriptManager.h"
#include <vector>
extern LPIObject GetPlayerBySID(DWORD dwStaticID);
extern SIZE_T GetMemoryInfo();

///��ʱ���������ײ͵���ȡʱ�䲻�������ļ��ж�ȡ���ڴ˶���
#define LunchShowTimeFloor	18000
#define LunchRewardTimeFloor	43200
#define DinnerShowTimeFloor	50400
#define DinnerRewardTimeFloor	64800
#define SupperShowTimeFloor	72000
#define SupperRewardTimeFloor	75600
#define SupperShowTimeCeiling	86399

CMissionDataManager::CMissionDataManager(CBaseDataManager& baseDataMgr)
	:CExtendedDataManager(baseDataMgr)
{
	m_ptrMissionAccomplishedMsg = make_shared<SAMissionAccomplished>();
	m_ptrUpdateMissionStateMsg = make_shared<SAUpdateMissionState>();
	m_bInitResetMission = false;
}


CMissionDataManager::~CMissionDataManager()
{
	m_operationOnMissionAccomplished = nullptr;
}

bool CMissionDataManager::InitDataMgr(void * pData)
{
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
		m_bInitResetMission = true;
	}

	return true;
}

void CMissionDataManager::OnLuaInitialed()
{
	if (m_bInitResetMission)
	{
		m_bInitResetMission = false;
		ResetMissionData(0);
	}

	///�ж�������������Ƿ������ʾ��������ȡ���߼�
	EnduranceMissionInitJudgement();

	///����VIP��ص�����
	BOOL execResult = FALSE;
	int retValue = 0;
	lite::Variant ret1;
	if (g_Script.PrepareFunction("SI_vip_getlv"))
	{
		g_Script.PushParameter(m_pBaseDataMgr.GetSID());
		execResult = g_Script.Execute(&ret1);
	}
	else
		return;

	if (!execResult || ret1.dataType == LUA_TNIL)
	{
		rfalse(2, 1, "ConsumeFinalDamageValue Faile");
		return;
	}

	try
	{
		retValue = (int)(ret1);
	}
	catch (lite::Xcpt &e)
	{
		rfalse(2, 1, e.GetErrInfo());
		return;
	}

	SetMissionCompleteTimes(MissionType::MT_VIP, retValue);
}

void CMissionDataManager::SetMissionAccomplishedCallBack(FunctionPtr& fun)
{
	m_operationOnMissionAccomplished = fun;
}

bool CMissionDataManager::ReleaseDataMgr()
{
	return true;
}

void CMissionDataManager::EnduranceMissionInitJudgement()
{
	bool m_bLunchAccessible = false;
	bool m_bDinnerAccessible = false;
	bool m_bSupperAccessible = false;

	///���濪ʼ�����ǰʱ�䵽����0���ʱ���
	tm curTime;
	__time64_t now = _time64(nullptr);
	_localtime64_s(&curTime, &now);
	__time64_t todayOClock, tommorowOClock;
	curTime.tm_hour = 0;
	curTime.tm_min = 0;
	curTime.tm_sec = 0;
	todayOClock = _mktime64(&curTime);
	curTime.tm_mday += 1;
	tommorowOClock = _mktime64(&curTime);

	if (g_Script.PrepareFunction("CloseTimeEnduranceMission"))
	{
		g_Script.PushParameter(m_pBaseDataMgr.GetSID());
		g_Script.Execute();
	}

	///���濪ʼ�ж�ĳЩ������������Ƿ������ʾ������ȡ
	int timeElasped = now - todayOClock;
	if (timeElasped >= SupperShowTimeFloor && timeElasped < SupperShowTimeCeiling)///ҹ�������������ʾ
	{
		if (timeElasped >= SupperRewardTimeFloor)///ҹ�������������ȡ
		{
			m_bSupperAccessible = true;
		}

		EnduranceMissionProcess(MissionType::MT_Supper, true, m_bSupperAccessible, false, false);
	}
	else if (timeElasped >= DinnerShowTimeFloor)///��������������ʾ
	{
		if (timeElasped >= DinnerRewardTimeFloor)///��������������ȡ
		{
			m_bDinnerAccessible = true;
		}

		EnduranceMissionProcess(MissionType::MT_Dinner, true, m_bDinnerAccessible, false, false);
	}
	else if (timeElasped >= LunchShowTimeFloor)///��������������ʾ
	{
		if (timeElasped >= LunchRewardTimeFloor)///��������������ȡ
		{
			m_bLunchAccessible = true;
		}

		EnduranceMissionProcess(MissionType::MT_Lunch, true, m_bLunchAccessible, false, false);
	}
}

void CMissionDataManager::SetMissionCompleteTimes(MissionType type, DWORD times, bool synMsg /* = true */)
{
	const set<int>* missionIDList = CConfigManager::getSingleton()->GetMissionListByType(type);
	if (missionIDList == nullptr)
		return;

	for (auto iter : *missionIDList)
	{
		SetMissionCompleteTimes(iter, times, synMsg);
	}
}

void CMissionDataManager::SetMissionCompleteTimes(int missionID, DWORD times, bool synMsg /*= true*/)
{
	const MissionConfig * config = CConfigManager::getSingleton()->GetMissionConfig(missionID);
	if (config == nullptr)
	{
		rfalse("����%d��Ӧ�������ļ��Ҳ���", missionID);
		return;
	}

	if (config->missionAvaliableLevel > m_pBaseDataMgr.GetMasterLevel())
	{
		return;
	}

	BOOL execResult = FALSE;
	int retValue = 0;
	lite::Variant ret;//��lua��ȡ���ķ���ֵ
	times = min(times, config->needCompleteTime);
	if (g_Script.PrepareFunction("SetMissionCompletedTimes"))
	{
		g_Script.PushParameter(m_pBaseDataMgr.GetSID());
		g_Script.PushParameter(missionID);
		g_Script.PushParameter(times);
		g_Script.PushParameter(config->needCompleteTime);
		execResult = g_Script.Execute(&ret);
	}
	else
		return;

	if (!execResult || ret.dataType == LUA_TNIL)
	{
		rfalse(2, 1, "ConsumeFinalDamageValue Faile");
		return;
	}

	try
	{
		retValue = (int)(ret);
	}
	catch (lite::Xcpt &e)
	{
		rfalse(2, 1, e.GetErrInfo());
		return;
	}

	if (retValue > 0 && synMsg)
	{
		///���¿ͻ��˵���������������Ϣ,��Ϊ�ܡ����һ�Ρ��ģ��������visibleΪtrue�ģ����Դ˴�ֱ������Ϊtrue
		m_ptrUpdateMissionStateMsg->missionID = missionID;
		m_ptrUpdateMissionStateMsg->visible = true;
		m_ptrUpdateMissionStateMsg->completeTimes = retValue;
		g_StoreMessage(m_pBaseDataMgr.GetDNID(), m_ptrUpdateMissionStateMsg.get(), sizeof(SAUpdateMissionState));
	}
}

void CMissionDataManager::SetMissionAccompulished(DWORD missionID)
{
	m_ptrMissionAccomplishedMsg->bAccomplished = false;
	m_ptrMissionAccomplishedMsg->missionID = missionID;
	const MissionConfig * config = CConfigManager::getSingleton()->GetMissionConfig(missionID);
	if (config == nullptr)
	{
		rfalse("����%d��Ӧ�������ļ��Ҳ���", missionID);
		g_StoreMessage(m_pBaseDataMgr.GetDNID(), m_ptrMissionAccomplishedMsg.get(), sizeof(SAMissionAccomplished));
		return;
	}

	if (config->missionAvaliableLevel > m_pBaseDataMgr.GetMasterLevel())
	{
		rfalse("����������%d�Ŀ��ŵȼ�Ҫ��", missionID);
		g_StoreMessage(m_pBaseDataMgr.GetDNID(), m_ptrMissionAccomplishedMsg.get(), sizeof(SAMissionAccomplished));
		return;
	}

	int retValue = 0;
	BOOL execResult = FALSE;
	int completedTimes = config->needCompleteTime;
	lite::Variant ret1;//��lua��ȡ���ķ���ֵ
	///������񲻡��ɼ��������ɡ���ȡ���������Ѿ���������������״̬������ȡ��Ӧ�Ľ���
	if (g_Script.PrepareFunction("SetMissionAccompulished"))
	{
		g_Script.PushParameter(m_pBaseDataMgr.GetSID());
		g_Script.PushParameter(missionID);
		g_Script.PushParameter(completedTimes);
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

	///����������ɴ���û�дﵽҪ��Ҳ�����������״̬����ȡ��Ӧ�Ľ���
	if (retValue == 0)
	{
		lite::Variant ret2;//��lua��ȡ���ķ���ֵ
		if (g_Script.PrepareFunction("GetMissionData"))
		{
			g_Script.PushParameter(m_pBaseDataMgr.GetSID());
			g_Script.PushParameter(missionID);
			g_Script.PushParameter(MissionDataType::accomplished);
			execResult = g_Script.Execute(&ret2);
		}
		retValue = 0;
		if (execResult && ret2.dataType != LUA_TNIL)
		{
			try
			{
				retValue = (int)ret2;
			}
			catch (lite::Xcpt& e)
			{
				retValue = 0;
				rfalse(2, 1, e.GetErrInfo());
			}
		}

		m_ptrMissionAccomplishedMsg->bAccomplished = (retValue != 0);
		m_ptrMissionAccomplishedMsg->missionID = missionID;
		g_StoreMessage(m_pBaseDataMgr.GetDNID(), m_ptrMissionAccomplishedMsg.get(), sizeof(SAMissionAccomplished));
		return;
	}

	///������ɺ�Ļص�����
	if (m_operationOnMissionAccomplished._Empty() == false)
		m_operationOnMissionAccomplished(missionID);

	int itemNum = 0;
	m_ptrMissionAccomplishedMsg->bAccomplished = true;
	m_ptrMissionAccomplishedMsg->missionID = missionID;
	lite::Variant ret2;//��lua��ȡ���ķ���ֵ
	switch (config->rewardsType)
	{
	case MissionRewardsType::MR_DIAMOND:///����ֵ�ҹ��Ľ�����ʯ������
		if (config->missionType == MT_VIP || config->missionType == MT_MonthCard)
		{
			m_pBaseDataMgr.AddGoods_SG(GoodsType::diamond, 0, config->rewardsItemAmount, GoodsWay::task);
		}
		break;
	case MissionRewardsType::MR_LUNCH:/// �в͡���͡�ҹ���Ƚ�������������
	case MissionRewardsType::MR_DINNER:
	case MissionRewardsType::MR_SUPPER:
		m_pBaseDataMgr.AddGoods_SG(GoodsType::endurance, 0, config->rewardsItemAmount, GoodsWay::task);
		break;
	case  MissionRewardsType::MR_ENDURANCE:
		m_pBaseDataMgr.AddGoods_SG(GoodsType::endurance, 0, config->rewardsItemAmount, GoodsWay::task);
		break;
	case MissionRewardsType::MR_SWEEPTICKET:///��VIP�ҹ�������ɨ���������
		///��ʱ��VIP�ȼ��Ļ�ȡ�����ŵ�����,��ΪVIP�ĵȼ�����ʱ������䣬���ԡ�ʵʱ����ȡ
		if (g_Script.PrepareFunction("SI_vip_getDetail"))
		{
			g_Script.PushParameter(m_pBaseDataMgr.GetSID());
			g_Script.PushParameter(VipLevelFactor::VF_GiveATKTicket_Num);
			execResult = g_Script.Execute(&ret2);
		}

		itemNum = -1;
		if (execResult && ret2.dataType != LUA_TNIL)
		{
			try
			{
				itemNum = (int)ret2;
			}
			catch (lite::Xcpt& e)
			{
				itemNum = -1;
				rfalse(2, 1, e.GetErrInfo());
			}
		}

		if (itemNum > 0)
		{
			if (config->rewardsItemID != 0)
				m_pBaseDataMgr.AddGoods_SG(GoodsType::item, config->rewardsItemID, itemNum, GoodsWay::task);
		}
		break;
	case MissionRewardsType::MR_ITEM: ///��������Ʒ
		if (config->rewardsItemID != 0 && config->rewardsItemAmount != 0)
			m_pBaseDataMgr.AddGoods_SG(GoodsType::item, config->rewardsItemID, config->rewardsItemAmount, GoodsWay::task);
		break;
	}

	///�������������Ʒ�������ͻ���ͬ��
	if (config->rewardsExp != 0)
		m_pBaseDataMgr.AddGoods_SG(GoodsType::Exp, 0, config->rewardsExp, GoodsWay::task);
	if (config->rewardsGold != 0)
		m_pBaseDataMgr.AddGoods_SG(GoodsType::money, 0, config->rewardsGold, GoodsWay::task);

	g_StoreMessage(m_pBaseDataMgr.GetDNID(), m_ptrMissionAccomplishedMsg.get(), sizeof(SAMissionAccomplished));
}

void CMissionDataManager::ResetMissionData(int sendMsgFlag /* = 0 */)
{
	if (g_Script.PrepareFunction("ResetMission"))
	{
		g_Script.PushParameter(m_pBaseDataMgr.GetSID());
		g_Script.Execute();
	}

	if (sendMsgFlag != 0)
	{
		SARefreshMissionData refreshMsg;
		g_StoreMessage(m_pBaseDataMgr.GetDNID(), &refreshMsg, sizeof(SARefreshMissionData));
	}
}

void CMissionDataManager::EnduranceMissionProcess(MissionType type, bool visible, bool accessible, bool conflitProcess, bool sendMsg /* = true */)
{
	const set<int>* missionIDList = CConfigManager::getSingleton()->GetMissionListByType(type);
	if (missionIDList == nullptr)
		return;

	if (visible && conflitProcess)
	{
		///��ȡ��һ��������������б�
		const set<int>* conflictMissionIDList = nullptr;
		switch (type)
		{
		case MissionType::MT_Dinner:
			conflictMissionIDList = CConfigManager::getSingleton()->GetMissionListByType(MissionType::MT_Lunch);
			break;
		case MissionType::MT_Supper:
			conflictMissionIDList = CConfigManager::getSingleton()->GetMissionListByType(MissionType::MT_Dinner);
			break;
		}

		///����һ���������������Ϊ���ɼ�
		if (conflictMissionIDList != nullptr)
		{
			for (auto iter : *conflictMissionIDList)
			{
				///���ε�������
				if (g_Script.PrepareFunction("ShieldingMission"))
				{
					g_Script.PushParameter(m_pBaseDataMgr.GetSID());
					g_Script.PushParameter(iter);
					g_Script.Execute();
				}

				if (sendMsg)
				{
					m_ptrUpdateMissionStateMsg->missionID = iter;
					m_ptrUpdateMissionStateMsg->visible = false;
					m_ptrUpdateMissionStateMsg->completeTimes = 0;
					g_StoreMessage(m_pBaseDataMgr.GetDNID(), m_ptrUpdateMissionStateMsg.get(), sizeof(SAUpdateMissionState));
				}
			}
		}
	}
	
	for (auto iter : *missionIDList)
	{
		if (visible)
		{
			///���ø�����Ŀɼ�״̬
			if (g_Script.PrepareFunction("SetMissionData"))
			{
				g_Script.PushParameter(m_pBaseDataMgr.GetSID());
				g_Script.PushParameter(iter);
				g_Script.PushParameter(MissionDataType::visible);
				g_Script.PushParameter(visible ? 1 : 0);
				g_Script.Execute();
			}

			///��ʱ���������������״̬�ڴ�����
			if (accessible)
			{
				SetMissionCompleteTimes(iter, 1, sendMsg);
				return;
			}
		}
		else if (!visible && !accessible)
		{
			///���ε�������
			if (g_Script.PrepareFunction("ShieldingMission"))
			{
				g_Script.PushParameter(m_pBaseDataMgr.GetSID());
				g_Script.PushParameter(iter);
				g_Script.Execute();
			}
		}

		if (sendMsg)
		{
			m_ptrUpdateMissionStateMsg->missionID = iter;
			m_ptrUpdateMissionStateMsg->visible = visible;
			m_ptrUpdateMissionStateMsg->completeTimes = 0;
			g_StoreMessage(m_pBaseDataMgr.GetDNID(), m_ptrUpdateMissionStateMsg.get(), sizeof(SAUpdateMissionState));
		}
	}
}

void CMissionDataManager::LunchMissionVisible()
{
	///��ͬ������������ǻ����
	EnduranceMissionProcess(MissionType::MT_Lunch, true, false, true);
}

void CMissionDataManager::DinnerMissionVisible()
{
	///��ͬ������������ǻ����
	EnduranceMissionProcess(MissionType::MT_Dinner, true, false, true);
}

void CMissionDataManager::SupperMissionVisible()
{
	///��ͬ������������ǻ����
	EnduranceMissionProcess(MissionType::MT_Supper, true, false, true);
}

void CMissionDataManager::LunchMissionAccessible()
{
	///��ͬ������������ǻ����
	EnduranceMissionProcess(MissionType::MT_Lunch, true, true, true);
}

void CMissionDataManager::DinnerMissionAccessible()
{
	///��ͬ������������ǻ����
	EnduranceMissionProcess(MissionType::MT_Dinner, true, true, true);
}

void CMissionDataManager::SupperMissionAccessible()
{
	///��ͬ������������ǻ����
	EnduranceMissionProcess(MissionType::MT_Supper, true, true, true);
}

void CMissionDataManager::SupperMissionDisable()
{
	///���е����񶼹رյ�
	EnduranceMissionProcess(MissionType::MT_Supper, false, false, true);
}