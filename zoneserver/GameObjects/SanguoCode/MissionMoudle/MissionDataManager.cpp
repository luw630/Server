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

///暂时各个体力套餐的领取时间不从配置文件中读取，在此定义
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

	///判断体力礼包任务是否可以显示，可以领取的逻辑
	EnduranceMissionInitJudgement();

	///更新VIP相关的任务
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

	///下面开始求出当前时间到今天0点的时间差
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

	///下面开始判断某些体力礼包任务是否可以显示或者领取
	int timeElasped = now - todayOClock;
	if (timeElasped >= SupperShowTimeFloor && timeElasped < SupperShowTimeCeiling)///夜宵体力任务可显示
	{
		if (timeElasped >= SupperRewardTimeFloor)///夜宵体力任务可领取
		{
			m_bSupperAccessible = true;
		}

		EnduranceMissionProcess(MissionType::MT_Supper, true, m_bSupperAccessible, false, false);
	}
	else if (timeElasped >= DinnerShowTimeFloor)///晚餐体力任务可显示
	{
		if (timeElasped >= DinnerRewardTimeFloor)///晚餐体力任务可领取
		{
			m_bDinnerAccessible = true;
		}

		EnduranceMissionProcess(MissionType::MT_Dinner, true, m_bDinnerAccessible, false, false);
	}
	else if (timeElasped >= LunchShowTimeFloor)///午餐体力任务可显示
	{
		if (timeElasped >= LunchRewardTimeFloor)///午餐体力任务可领取
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
		rfalse("任务%d对应的配置文件找不到", missionID);
		return;
	}

	if (config->missionAvaliableLevel > m_pBaseDataMgr.GetMasterLevel())
	{
		return;
	}

	BOOL execResult = FALSE;
	int retValue = 0;
	lite::Variant ret;//从lua获取到的返回值
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
		///更新客户端的任务的完成数据信息,因为能“完成一次”的，必须的是visible为true的，所以此处直接设置为true
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
		rfalse("任务%d对应的配置文件找不到", missionID);
		g_StoreMessage(m_pBaseDataMgr.GetDNID(), m_ptrMissionAccomplishedMsg.get(), sizeof(SAMissionAccomplished));
		return;
	}

	if (config->missionAvaliableLevel > m_pBaseDataMgr.GetMasterLevel())
	{
		rfalse("不满足任务%d的开放等级要求", missionID);
		g_StoreMessage(m_pBaseDataMgr.GetDNID(), m_ptrMissionAccomplishedMsg.get(), sizeof(SAMissionAccomplished));
		return;
	}

	int retValue = 0;
	BOOL execResult = FALSE;
	int completedTimes = config->needCompleteTime;
	lite::Variant ret1;//从lua获取到的返回值
	///如果任务不“可见”，不可“领取”，任务已经完成则不予设置完成状态，并领取相应的奖励
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

	///如果任务的完成次数没有达到要求也不予设置完成状态和领取相应的奖励
	if (retValue == 0)
	{
		lite::Variant ret2;//从lua获取到的返回值
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

	///任务完成后的回调操作
	if (m_operationOnMissionAccomplished._Empty() == false)
		m_operationOnMissionAccomplished(missionID);

	int itemNum = 0;
	m_ptrMissionAccomplishedMsg->bAccomplished = true;
	m_ptrMissionAccomplishedMsg->missionID = missionID;
	lite::Variant ret2;//从lua获取到的返回值
	switch (config->rewardsType)
	{
	case MissionRewardsType::MR_DIAMOND:///跟充值挂钩的奖励宝石的任务
		if (config->missionType == MT_VIP || config->missionType == MT_MonthCard)
		{
			m_pBaseDataMgr.AddGoods_SG(GoodsType::diamond, 0, config->rewardsItemAmount, GoodsWay::task);
		}
		break;
	case MissionRewardsType::MR_LUNCH:/// 中餐、晚餐、夜宵等奖励体力的任务
	case MissionRewardsType::MR_DINNER:
	case MissionRewardsType::MR_SUPPER:
		m_pBaseDataMgr.AddGoods_SG(GoodsType::endurance, 0, config->rewardsItemAmount, GoodsWay::task);
		break;
	case  MissionRewardsType::MR_ENDURANCE:
		m_pBaseDataMgr.AddGoods_SG(GoodsType::endurance, 0, config->rewardsItemAmount, GoodsWay::task);
		break;
	case MissionRewardsType::MR_SWEEPTICKET:///跟VIP挂钩的赠送扫荡卷的任务
		///暂时将VIP等级的获取方法放到这里,因为VIP的等级“随时”都会变，所以“实时”获取
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
	case MissionRewardsType::MR_ITEM: ///奖励了物品
		if (config->rewardsItemID != 0 && config->rewardsItemAmount != 0)
			m_pBaseDataMgr.AddGoods_SG(GoodsType::item, config->rewardsItemID, config->rewardsItemAmount, GoodsWay::task);
		break;
	}

	///添加任务奖励的物品，并往客户端同步
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
		///获取上一个体力礼包任务列表
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

		///将上一个体力礼包任务置为不可见
		if (conflictMissionIDList != nullptr)
		{
			for (auto iter : *conflictMissionIDList)
			{
				///屏蔽掉该任务
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
			///设置该任务的可见状态
			if (g_Script.PrepareFunction("SetMissionData"))
			{
				g_Script.PushParameter(m_pBaseDataMgr.GetSID());
				g_Script.PushParameter(iter);
				g_Script.PushParameter(MissionDataType::visible);
				g_Script.PushParameter(visible ? 1 : 0);
				g_Script.Execute();
			}

			///暂时体力礼包任务的完成状态在此设置
			if (accessible)
			{
				SetMissionCompleteTimes(iter, 1, sendMsg);
				return;
			}
		}
		else if (!visible && !accessible)
		{
			///屏蔽掉该任务
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
	///不同体力礼包任务是互斥的
	EnduranceMissionProcess(MissionType::MT_Lunch, true, false, true);
}

void CMissionDataManager::DinnerMissionVisible()
{
	///不同体力礼包任务是互斥的
	EnduranceMissionProcess(MissionType::MT_Dinner, true, false, true);
}

void CMissionDataManager::SupperMissionVisible()
{
	///不同体力礼包任务是互斥的
	EnduranceMissionProcess(MissionType::MT_Supper, true, false, true);
}

void CMissionDataManager::LunchMissionAccessible()
{
	///不同体力礼包任务是互斥的
	EnduranceMissionProcess(MissionType::MT_Lunch, true, true, true);
}

void CMissionDataManager::DinnerMissionAccessible()
{
	///不同体力礼包任务是互斥的
	EnduranceMissionProcess(MissionType::MT_Dinner, true, true, true);
}

void CMissionDataManager::SupperMissionAccessible()
{
	///不同体力礼包任务是互斥的
	EnduranceMissionProcess(MissionType::MT_Supper, true, true, true);
}

void CMissionDataManager::SupperMissionDisable()
{
	///所有的任务都关闭掉
	EnduranceMissionProcess(MissionType::MT_Supper, false, false, true);
}