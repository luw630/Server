#include "stdafx.h"
#include <fstream>
#include "ArenaMoudel.h"
#include "..\Common\ConfigManager.h"
#include "..\Common\PubTool.h"
#include "..\MissionMoudle\CMissionUpdate.h"
#include "Random.h"
#include "player.h"
#include "..\BaseDataManager.h"
#include "ArenaDataMgr.h"
#include"PlayerManager.h"
#include "..\TimerEvent_SG\TimerEvent_SG.h"
#include "..\MailMoudle\MailMoudle.h"
#include"..\HeroMoudle\HeroAttributeCalculator.h"
#include "lua.h"
#include "../BattleVerification.h"

extern LPIObject GetPlayerBySID(DWORD dwStaticID);
const int ARENA_ID(int sid) { return MAX_ROBOT_NUM + sid; }
const int GetSIDFromArenaID(int arenaID){ return arenaID - MAX_ROBOT_NUM; };
CArenaMoudel::CArenaMoudel()
	:globalconfig(CConfigManager::getSingleton()->globalConfig)
	, m_RobotNameConfig(CConfigManager::getSingleton()->GetRobotNameConfig())
	, m_lastRoutineTime(0)
{
	m_RobotID = 0;
	m_pTop50th = nullptr;
	m_defensiveTeam;
	memset(m_defensiveTeam, 0, sizeof(int) * 5);

	m_pTop50th = new SArenaPlayerReducedData[50];
	memset(m_pTop50th, 0, sizeof(SArenaPlayerReducedData) * 50);

}


CArenaMoudel::~CArenaMoudel()
{
	//Release();
}

bool CArenaMoudel::InitArenaData()
{
	m_ArenaIDRankMapping.clear();
	m_ArenaManagedData.clear();

	ifstream in("dbTables\\SanguoArenaRankList.txt", ios::binary);
	if (in.is_open() == true)
	{
		in.read((char*)&m_lastRoutineTime, sizeof(__int64));
		SBaseArenaCompetitor tempBaseComp;
		int tempInt, maxArenaRank;
		in.read((char*)&maxArenaRank, sizeof(int)); //读取竞技场最多排名
		for (int i = 0; i < maxArenaRank; i++)
		{
			in.read((char*)&tempInt, sizeof(int));
			in.read((char*)&tempBaseComp, sizeof(SBaseArenaCompetitor));

			m_ArenaManagedData.insert(make_pair(tempInt, SArenaCompetitor(tempBaseComp)));
			m_ArenaIDRankMapping.insert(make_pair(tempBaseComp.m_AernaID, tempInt));
		}
	}
	else
	{
		_GenerateRank();
		BackupArenaRankData();
	}
	in.close();
	//更新前50名玩家榜单
	if (UpdateTop50th() == false)
		return false;

	ArenaRoutine();
	return true;
}

void CArenaMoudel::OnDispatchArenaMsg(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer)
{
	switch (pSArenaMsg->_protocol)
	{
	case SSGArenaMsg::Arena_ChallengeBegin:
	{
		_OnRecvChallengeBegin(dnidClient, pSArenaMsg, pPlayer);
		break;
	}
	case SSGArenaMsg::Arena_DelockArena:
	{
		DebutArena(dnidClient, pPlayer->GetBaseDataMgr());
		break;
	}
	case SSGArenaMsg::Arena_GetCompetitor:
	{
		_OnRandomReplacement(dnidClient, pSArenaMsg, pPlayer);
		break;
	}
	case SSGArenaMsg::Arena_ChallengeOver:
	{
		_OnRecvChallengeOver(dnidClient, pSArenaMsg, pPlayer);
		break;
	}
	case SSGArenaMsg::Arena_GetTop50th:
	{
		_OnGetTop50thCompetitors(dnidClient, pSArenaMsg, pPlayer);
		break;
	}
	case SSGArenaMsg::Arena_SetDefensiveTeam:
	{
		_OnSetDefensiveTeam(dnidClient, pSArenaMsg, pPlayer);
		break;
	}
	case SSGArenaMsg::Arena_GetTop50Detail:
	{
		_GetTop50CompetitorDetail(dnidClient, pSArenaMsg, pPlayer);
		break;
	}
	case SSGArenaMsg::Arena_SetBuyChallengeNum://竞技场购买挑战次数
	{
		_BuyChallengeNum(dnidClient, pSArenaMsg, pPlayer);
		break;
	}
	case SSGArenaMsg::Arena_ResetCountDown://重置 竞技场购买挑战次数的冷却时间
	{
		_ResetCountDown(dnidClient, pSArenaMsg, pPlayer);
		break;
	}
	default:
		break;
	}
}

//重置竞技场挑战时间
void CArenaMoudel::_ResetCountDown(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer)
{

	if (dnidClient == NULL || pSArenaMsg == NULL || pPlayer == NULL)
		return;

	CBaseDataManager* pBaseMgr = pPlayer->GetBaseDataMgr(); //通用数据

	if (pBaseMgr == NULL)
		return;
	//获取竞技场数据
	int rank = m_ArenaIDRankMapping.at(ARENA_ID(pBaseMgr->GetSID()));

	SArenaCompetitor*  pSArenaData = &m_ArenaManagedData.at(rank);

	int ResetNum = pSArenaData->m_ResetNum;

	//变更次数信息     策划说无限次重置 
	lite::Variant ret;//从lua获取返回值
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "SI_vip_getDetail")[pBaseMgr->GetSID()][VipLevelFactor::VF_ReSetArenaCD_Num](&ret);
	g_Script.CleanCondition();
	int Maxnum = (int)ret;

	lite::Variant ret1;//从lua获取返回值
	LuaFunctor(g_Script, "SI_getNeedvalue")[pBaseMgr->GetSID()][Expense::Ex_ReSetCD][ResetNum + 1](&ret1);
	g_Script.CleanCondition();
	int Money = (int)ret1;


	//变更上一次的挑战时间
	__int64 now;
	_time64(&now);
	int timeSpan = now - pSArenaData->m_LastChallengeTime;
	if (timeSpan > 600)
	{
		return;
	}
	pSArenaData->m_LastChallengeTime = _time64(&now) - 600;

	//扣除宝石费用
	pBaseMgr->DecGoods_SG(GoodsType::diamond, 0, Money,GoodsWay::practiceForce);
	pSArenaData->m_ResetNum++;

	SAResetCD_Respoens Msg;
	Msg.ResetNum = pSArenaData->m_ResetNum;


	g_StoreMessage(dnidClient, &Msg, sizeof(Msg));
}


//竞技场购买挑战次数
void CArenaMoudel::_BuyChallengeNum(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer)
{

	if (dnidClient == NULL || pSArenaMsg == NULL || pPlayer == NULL)
		return;

	CBaseDataManager* pBaseMgr = pPlayer->GetBaseDataMgr(); //通用数据

	if (pBaseMgr == NULL)
		return;
	//获取竞技场数据
	int rank = m_ArenaIDRankMapping.at(ARENA_ID(pBaseMgr->GetSID()));

	SArenaCompetitor*  pSArenaData = &m_ArenaManagedData.at(rank);


	lite::Variant ret1;//从lua获取返回值
	LuaFunctor(g_Script, "SI_getNeedvalue")[pBaseMgr->GetSID()][Expense::Ex_ArenaTicket][pSArenaData->m_BuyedNum + 1](&ret1);
	g_Script.CleanCondition();
	if (ret1.dataType == LUA_TNIL)
	{
		rfalse("获取竞技场VIP购买次数错误");
		return;
	}
		
	int cost = (int)ret1;
	if (pBaseMgr->GetDiamond() < cost) //如果元宝数不够，则返回
		return;

	//变更次数信息
	lite::Variant ret;//从lua获取返回值
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "SI_vip_getDetail")[pBaseMgr->GetSID()][VipLevelFactor::VF_BuyArenaTicket_Num](&ret);
	g_Script.CleanCondition();
	int Maxnum = (int)ret;  //获取当前等级的最大购买次数
	if (ret.dataType == LUA_TNIL)
	{
		rfalse("获取竞技场当前VIP等级的最大购买次数错误");
		return;
	}
	if (pSArenaData->m_BuyedNum >= Maxnum) //超过最大购买次数
	{
		return;
	}
	else
	{
		//扣除宝石费用
		pBaseMgr->DecGoods_SG(GoodsType::diamond, 0, cost, GoodsWay::practiceForce);
		pSArenaData->m_ChallengeCount = 5;	//挑战次数重置
		++pSArenaData->m_BuyedNum;//更新购买次数
	}

	//变更上一次的挑战时间
	__int64 now;
	_time64(&now);
	int timeSpan = now - pSArenaData->m_LastChallengeTime;
	pSArenaData->m_LastChallengeTime = _time64(&now) - 600;

	SArenaBaseMsg pSbMsg;
	pSbMsg.BuyChallengeNum = pSArenaData->m_BuyedNum;
	pSbMsg.ChallengeCount = pSArenaData->m_ChallengeCount;
	//返回消息给客户端
	g_StoreMessage(dnidClient, &pSbMsg, sizeof(pSbMsg));

	//wk 20150706 购买次数日志
	g_Script.CallFunc("db_gm_setoperation", pBaseMgr->GetSID(), 4, 2, pSArenaData->m_BuyedNum);

}

void CArenaMoudel::_OnRecvChallengeOver(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer)
{
	SQChallengeOver * pOverMsg = (SQChallengeOver *)pSArenaMsg;

	CBaseDataManager* pBaseMgr = pPlayer->GetBaseDataMgr(); //通用数据
	//解锁对手
	SArenaCompetitor* pCompetitor = nullptr;//对手
	try
	{
		int rank = m_ArenaIDRankMapping.at(ARENA_ID(pBaseMgr->GetSID()));
		SArenaCompetitor *pMyselt = &m_ArenaManagedData.at(rank);
		pCompetitor = _GetManagedArenaData(pMyselt->m_curOpponentID);
		if (nullptr == pCompetitor)
		{
			TalkToDnid(dnidClient, "比赛结束对手不存在");
			return;
		}
		UnlockCompetitor(pCompetitor->m_AernaID); //解锁玩家
		pMyselt->m_curOpponentID = -1;
		// 注销定时器事件
		pMyselt->m_EventPtr->Interrupt();
		pMyselt->m_EventPtr = nullptr;
		if (pOverMsg->m_BattleResult == 1) //战斗胜利,与对手互换排名
		{
			if (!CBattleVerification::getSingleton()->ArenaBattleVerification(pOverMsg->verifyData, pMyselt->m_LastChallengeTime, *pBaseMgr))
			{
				LuaFunctor(g_Script, "set_CreditDataValue")[pBaseMgr->GetSID()][2][1]();
				TalkToDnid(dnidClient, "检测到作弊");
				return;
			}
			//挑战胜利 刷新挑战时间
			pMyselt->m_LastChallengeTime -= 600;
			int myRank = m_ArenaIDRankMapping.at(ARENA_ID(pBaseMgr->GetSID()));
			int oppoRank = m_ArenaIDRankMapping.at(pCompetitor->m_AernaID); //对手现在的排名
			if (oppoRank >= myRank) //如果对方排名小于我当前排名，则不处理
			{
				SAChallengeOver oMsg;
				g_StoreMessage(dnidClient, &oMsg, sizeof(oMsg));
				return;
			}
			SArenaCompetitor tempCompetitor = m_ArenaManagedData.at(oppoRank); //获取对手信息
			m_ArenaManagedData.at(myRank).m_ArenaRank = oppoRank; //排名更新
			m_ArenaManagedData.at(oppoRank) = m_ArenaManagedData.at(myRank); //排名互换
			tempCompetitor.m_ArenaRank = myRank; //对手排名更新
			m_ArenaManagedData.at(myRank) = tempCompetitor; //对手排名互换


			//ID排名映射表跟新
			m_ArenaIDRankMapping.at(ARENA_ID(pBaseMgr->GetSID())) = oppoRank;
			int rank = m_ArenaIDRankMapping.at(ARENA_ID(pBaseMgr->GetSID()));
			m_ArenaIDRankMapping.at(tempCompetitor.m_AernaID) = myRank;
			rank = m_ArenaIDRankMapping.at(tempCompetitor.m_AernaID);

			//最佳排名处理
			pMyselt = &m_ArenaManagedData.at(oppoRank);
			if (pMyselt->m_BestRank > oppoRank)
			{

				//这里用递归计算奖励元宝数量，递归的缺点：1.无限递归造成栈溢出，2.可读性差
				std::function<int(int, int)> BestArenaRank = [&BestArenaRank](int curRank, int newRank)->int
				{

					ArenaRewardConfig rewarConfig;
					CConfigManager::getSingleton()->GetArenaRewardConfig(curRank, rewarConfig);
					int result = 0;
					if (rewarConfig.rankUp <= newRank || curRank == 0 || newRank == 0) //递归结束逻辑
					{
						result = (curRank - newRank + 1) * rewarConfig.BestRecord;
						return result;
					}
					else
					{
						result = (curRank - rewarConfig.rankUp + 1) * rewarConfig.BestRecord;
						curRank = rewarConfig.rankUp - 1;
						return BestArenaRank(curRank, newRank) + result;
					}
				};
				int rewardDiamond = BestArenaRank(myRank, oppoRank); //获取宝石奖励数量

				pMyselt->m_BestRank = oppoRank;
				SABirthBestRecords msg;
				msg.m_DiamondRewards = rewardDiamond; //临时用于奖励算法
				msg.m_NewRecordRank = oppoRank;
				msg.m_OriginalRecordRank = myRank;
				g_StoreMessage(dnidClient, &msg, sizeof(msg));

				//计算排名提升
				int bRank = myRank - oppoRank ;
				//发送奖励邮件
				MailInfo mailInfo;
				memset(&mailInfo, 0, sizeof(mailInfo));

				lite::Variant ret;//从lua获取vip等级
				LuaFunctor(g_Script, "SI_GetStr")[3](&ret);
				LPCSTR Txt = (LPCSTR)ret;
				if (Txt == nullptr)
					rfalse("竞技场发送邮件获取VIP等级为空4");
				strcpy_s(mailInfo.senderName, Txt);

				lite::Variant ret1;//从lua获取vip等级
				LuaFunctor(g_Script, "SI_GetStr")[1](&ret1);
				LPCSTR Txt1 = (LPCSTR)ret1;
				if (Txt1 == nullptr)
					rfalse("竞技场发送邮件获取VIP等级为空5");
				strcpy_s(mailInfo.title, Txt1);

				lite::Variant ret2;//从lua获取vip等级
				LuaFunctor(g_Script, "SI_GetStr")[2][oppoRank][bRank](&ret2);
				LPCSTR Txt2 = (LPCSTR)ret2;
				if (Txt2 == nullptr)
					rfalse("竞技场发送邮件获取VIP等级为空6");
				strcpy_s(mailInfo.content, Txt2);

				if (rewardDiamond > 0)
				{
					mailInfo.szItemInfo[0].itype = GoodsType::diamond;
					mailInfo.szItemInfo[0].num = rewardDiamond;
				}

				MailMoudle::getSingleton()->SendMail_SanGuo(pMyselt->m_AernaID - MAX_ROBOT_NUM, 1440, &mailInfo);
			}
		}
		SAChallengeOver oMsg;
		g_StoreMessage(dnidClient, &oMsg, sizeof(oMsg));
	}
	catch (const std::out_of_range& oor)
	{
		rfalse("Out of Range error: %s, 竞技场挑战结束出错，玩家SID：%d", oor.what(), pBaseMgr->GetSID());
	}
}

void CArenaMoudel::_OnRecvChallengeBegin(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer)
{

	try{
		bool result = true;
		SQChallengeBegin  * challengeMsg = (SQChallengeBegin*)pSArenaMsg;
		SAChallengeBegin msg;
		msg.m_Result = -1;
		if (result == true && m_ArenaIDRankMapping.find(challengeMsg->m_ArenaID) == m_ArenaIDRankMapping.end()) //
		{
			rfalse("竞技场开始挑战 无法找到对手");
			TalkToDnid(dnidClient, "竞技场无法找到对手");
			result = false;
			g_StoreMessage(dnidClient, &msg, sizeof(msg));
			return;
		}

		if (result == true && m_ArenaIDRankMapping.at(challengeMsg->m_ArenaID) != challengeMsg->m_Rank)
		{
			TalkToDnid(dnidClient, "对手排名发生变化");
			result = false;
			g_StoreMessage(dnidClient, &msg, sizeof(msg));
			return;
		}

		int compRank = m_ArenaIDRankMapping.at(challengeMsg->m_ArenaID);
		if (result == true && compRank < 1 && compRank >m_ArenaIDRankMapping.size())
		{
			TalkToDnid(dnidClient, "对手排名超出范围");
			result = false;
			g_StoreMessage(dnidClient, &msg, sizeof(msg));
			return;
		}

		SArenaCompetitor*	tempCompetitor = &m_ArenaManagedData.at(compRank); //TODO：这里存在逻辑BUG，因为对手的数据可能已经发生改变，跟玩家选中对手时的数据已经不一样，需要优化
		if (result == true && true == tempCompetitor->m_IsLocked)
		{
			TalkToDnid(dnidClient, "对手在战斗中");
			result = false;
			g_StoreMessage(dnidClient, &msg, sizeof(msg));
			return;
		}

		CBaseDataManager* ptrBaseDataMgr = pPlayer->GetBaseDataMgr();
		int sid = ptrBaseDataMgr->GetSID();
		SArenaCompetitor* pMySelf = _GetManagedArenaData(ARENA_ID(sid)); //获取本身的数据
		__int64 now;
		_time64(&now);
		int timeSpan = now - pMySelf->m_LastChallengeTime;
		if (result == true && timeSpan < 600)
		{
			string tip = "处于竞技场CD中, 距上次挑战已经过了：";
			TalkToDnid(dnidClient, tip.c_str());
			result = false;
		}

		if (result == true && pMySelf->m_ChallengeCount <= 0)
		{
			TalkToDnid(dnidClient, "挑战次数已用完");
			result = false;
			pMySelf->m_ChallengeCount = 0;
		}

		if (result == true)
		{
			///更新竞技场挑战的任务相关数据
			CMissionUpdate* ptrMissionUpdator = ptrBaseDataMgr->GetMissionUpdator();
			if (ptrMissionUpdator != nullptr)
				ptrMissionUpdator->UpdateDungeionMission(InstanceType::Arena, 1);

			//上一次挑战时间更新
			pMySelf->m_LastChallengeTime = now;
			//战斗结算
			FunctionPtr challengeTimeOver = std::bind(&CArenaMoudel::_Timeout, this, sid);
			pMySelf->m_EventPtr = TimerEvent_SG::SetCallback(challengeTimeOver, 150); //记录定时器事件,2min30s后触发
			pMySelf->m_curOpponentID = tempCompetitor->m_AernaID; //设置对手指针
			tempCompetitor->m_IsLocked = true;
			rfalse("%d锁定对手ArenaID:%d", sid, tempCompetitor->m_AernaID);
			pMySelf->m_ChallengeCount -= 1;
			msg.m_Result = 1;

			//wk 20150706 参与次数日志
			g_Script.CallFunc("db_gm_setoperation", sid, 4, 1, pMySelf->m_ArenaRank);
		}

		g_StoreMessage(dnidClient, &msg, sizeof(msg));
	}
	catch (const std::out_of_range& oor)
	{
		rfalse("Out of Range error: %s, 竞技场挑战玩家出错，玩家SID：");
	}

}

void CArenaMoudel::_OnGetTop50thCompetitors(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer)
{
	SAGetTop50th msg;
	memset(msg.m_FactionDdata, 0, sizeof(char) * 32);
	strcpy_s(msg.m_FactionDdata, m_ArenaManagedData.begin()->second.m_FactionName);
	memset(msg.m_Top50th, 0, sizeof(SArenaPlayerReducedData));
	memcpy(msg.m_Top50th, m_pTop50th, 50 * sizeof(SArenaPlayerReducedData));
	g_StoreMessage(dnidClient, &msg, sizeof(SAGetTop50th));
}

//设置防守武将，获取在线玩家武将数据，更新托管数据中的防守阵容ID表
void CArenaMoudel::_OnSetDefensiveTeam(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer)
{
	SQSetDefensiveTeam *pDMsg = (SQSetDefensiveTeam *)pSArenaMsg;
	


	CBaseDataManager * baseDataMgr = pPlayer->GetBaseDataMgr();
	SASetDefensiveTeam msg;
	for (int i = 0; i < 5; i++) //不允许不设置防守阵容
	{
		if (baseDataMgr->ExistHero(pDMsg->m_HeroIDList[i]))
		{
			break;
		}
		if (i = 4)
		{
			msg.m_Result = false;
			g_StoreMessage(dnidClient, &msg, sizeof(msg));
			return;
		}
	}

	baseDataMgr->SetArenaDefenseTeam(pDMsg->m_HeroIDList); //这里不用更新托管数据中的武将信息，因为玩家在线时竞技场获取的是在线武将数据
	msg.m_Result = true;
	g_StoreMessage(dnidClient, &msg, sizeof(msg));
}

//根据下列公式随机出3个对手
//No.1:随机对手x*0.8 - x*（0.001-0.08）
//No.2:随机对手 x*0.9 - x*（0.001-0.08）
//No.3:随机对手 x*0.98 - x*（0.001-0.08）
void CArenaMoudel::_OnRandomReplacement(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer) //随机替换对手
{
	int curRank;

	static float posIndexList[] = { 0.8f, 0.9f, 0.98f };
	static int topRank[] = { 1, 2, 3, 4 };
	CBaseDataManager* baseDataMgr = pPlayer->GetBaseDataMgr();
	try
	{
		if (m_ArenaIDRankMapping.find(ARENA_ID(baseDataMgr->GetSID())) == m_ArenaIDRankMapping.end())
		{
			if (DebutArena(dnidClient, baseDataMgr) == false) //如果解锁竞技场失败  则返回
				return;
		}
		curRank = m_ArenaIDRankMapping.at(ARENA_ID(baseDataMgr->GetSID())); //获取玩家当前排名
		SAGetArenaCompetitors msg;
		SArenaCompetitor  *pManagedArena;
		if (curRank <= 4) //如果是前3名
		{
			int index = 0; //对手消息下标
			for (int i = 0; i < 4; i++)
			{
				if (curRank == topRank[i])
					continue;
				CPlayer * comp = (CPlayer*)GetPlayerBySID(m_ArenaIDRankMapping.at(topRank[i]) - MAX_ROBOT_NUM)->DynamicCast(IID_PLAYER);
				if (comp == nullptr) //如果不是在线玩家
					msg.m_Competitors[index] = m_ArenaManagedData.at(topRank[i]);
				else//是在线玩家
					_GetOnlineArenaData(comp, msg.m_Competitors[index]);
				++index;
			}
			msg.m_curRank = curRank; //排名赋值
			g_StoreMessage(dnidClient, &msg, sizeof(msg));
			return; //前3名处理完毕直接退出
		}

		float randomFactor;
		int tempRank = -1;
		for (int i = 0; i < 3; i++)//3个对手
		{
			randomFactor = (CRandom::RandRange(10, 300) / 10000.0f);
			tempRank = floor((curRank * posIndexList[i] - curRank * randomFactor));
			if (m_ArenaManagedData.find(tempRank) == m_ArenaManagedData.end())
			{
				continue;
			}
			int curArenaID = m_ArenaManagedData[tempRank].m_AernaID;
			if (m_ArenaIDRankMapping.find( m_ArenaManagedData[tempRank].m_AernaID ) == m_ArenaIDRankMapping.end())
			{
				rfalse("随机竞技场ID为%d对手出错了", tempRank);
				return;
			}

			//玩家的SID = ArenaID - 10000;

			CPlayer * comp = (CPlayer*)GetPlayerBySID(curArenaID - MAX_ROBOT_NUM)->DynamicCast(IID_PLAYER);
			if (comp == nullptr) //如果不是在线玩家
				msg.m_Competitors[i] = m_ArenaManagedData.at(tempRank);
			else//是在线玩家
				_GetOnlineArenaData(comp, msg.m_Competitors[i]);
			string name(msg.m_Competitors[i].m_CompetitorName);
			memset(&msg.m_Competitors[i].m_CompetitorName[name.length()], 0, 32 - name.length());
		}
		msg.m_curRank = curRank; //排名赋值
		g_StoreMessage(dnidClient, &msg, sizeof(msg));
	}
	catch (const std::out_of_range& oor)
	{
		rfalse("Out of Range error: %s, 竞技场随机对手出错，玩家SID：%d", oor.what(), baseDataMgr->GetSID());
	}

}

///解锁竞技场，玩家等级>10则解锁关卡
bool CArenaMoudel::DebutArena(DNID dnidClient, CBaseDataManager * pDataMgr)
{
	if (pDataMgr->GetMasterLevel() < 10 && pDataMgr->GetGMFlag() == false)
	{
		TalkToDnid(dnidClient, "当前等级无法进入竞技场");
		return false;
	}
	DWORD SID = pDataMgr->GetSID();
	SArenaCompetitor newCompetitor;
	newCompetitor.m_AernaID = ARENA_ID(SID); //获取竞技场唯一ID，
	//名字的保存
	string playerName = "wrong name";
	int pos = pDataMgr->GetName().length() - 4;
	if (pos > 0)
		playerName = pDataMgr->GetName().substr(0, pos); //TODO:暂时这么处理玩家名字

	strcpy_s(newCompetitor.m_CompetitorName, playerName.c_str()); //名字

	pDataMgr->GetTop5Hero(newCompetitor.m_HeroList); //获取防守阵容 
	DWORD defenseTeam[5];
	memset(defenseTeam, 0, sizeof(int) * 5);
	for (int i = 0; i < 5; i++) //竞技场防守阵容的获取 
		defenseTeam[i] = newCompetitor.m_HeroList[i].m_dwHeroID;
	pDataMgr->SetArenaDefenseTeam(defenseTeam);
	/// 设置缘分属性
	for (int i = 0; i < 5; ++i)
	{
		pDataMgr->GetHeroFateAttribute(newCompetitor.m_HeroList[i].m_dwHeroID, newCompetitor.m_FateAttr[i].fateAttr, 4);
	}
	newCompetitor.m_TeamLevel = pDataMgr->GetMasterLevel(); //获取等级
	newCompetitor.m_CompetitorIcon = pDataMgr->GetIconID(); //获取头像ID

	newCompetitor.m_ArenaRank = m_ArenaManagedData.size() + 1; //当前的竞技场名次
	newCompetitor.m_BestRank = newCompetitor.m_ArenaRank;//最佳名次
	newCompetitor.m_IsLocked = false;
	_time64(&newCompetitor.m_LastChallengeTime);
	newCompetitor.m_LastChallengeTime -= 6000;
	m_ArenaManagedData[newCompetitor.m_ArenaRank] = newCompetitor; //排行榜加入新人
	m_ArenaIDRankMapping[newCompetitor.m_AernaID] = newCompetitor.m_ArenaRank; //加入全局ID和排名的映射
	
	//newCompetitor.m_ResetNum = 0;
	//初始化重置次数
	newCompetitor.m_BuyedNum = 0;

	CPlayer * pPlayer = (CPlayer *)GetPlayerBySID(SID)->DynamicCast(IID_PLAYER);
	if (pPlayer == nullptr)
		return false;
	memset(newCompetitor.m_FactionName, 0, sizeof(char) * 32);
	lite::Variant ret;//从lua获取返回值
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "SI_vip_getlv")[pDataMgr->GetSID()](&ret);
	g_Script.CleanCondition();
	if (ret.dataType == LUA_TNIL)
		return false;
	newCompetitor.m_VipLevel = (int)ret; //获取VIP

	lite::Variant ret2;//从lua获取返回值
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "GetPlayerFactionName")[pDataMgr->GetSID()](&ret2);
	g_Script.CleanCondition();
	LPCSTR Txt = (LPCSTR)ret2;
	if (Txt == nullptr)
	{
		rfalse("解锁竞技场获取军团名为空");
		return false;
	}
	strcpy_s(newCompetitor.m_FactionName, Txt);
	return true;
}

void CArenaMoudel::SynLoginArenaData(DNID dnidClient, CBaseDataManager *pBaseDataMgr)
{
	try
	{
		SArenaCompetitor* pCompetitor = _GetManagedArenaData(ARENA_ID(pBaseDataMgr->GetSID()));
		if (nullptr == pCompetitor)
		{
			rfalse("SID为%d的玩家没有竞技场数据", pBaseDataMgr->GetSID());
			return;
		}

		SAArenaLoginData msg;
		msg.m_ChallengeCount = pCompetitor->m_ChallengeCount; //获取挑战次数
		ConverTool::ConvertInt64ToBytes(pCompetitor->m_LastChallengeTime, msg.m_LastChallengeTime); //获取挑战时间
		g_StoreMessage(dnidClient, &msg, sizeof(msg));
	}
	catch (const std::out_of_range& oor)
	{
		rfalse("Out of Range error: %s 竞技场登录数据同步出错，玩家SID：%d", oor.what(), pBaseDataMgr->GetSID());
	}


}

SArenaCompetitor* CArenaMoudel::UnlockCompetitor(const int arenaID)
{
	
	if (m_ArenaIDRankMapping.find(arenaID) == m_ArenaIDRankMapping.end()) //查询不到此人
	{
		rfalse("%d竞技场解锁,查询不到此人", arenaID);
		return nullptr;
	}

	int rank = m_ArenaIDRankMapping.at(arenaID);
	if (m_ArenaManagedData.find(rank) == m_ArenaManagedData.end()) //超出排名范围
	{
		rfalse(",%d竞技场解锁,超出排名范围", arenaID);
		return nullptr;
	}	

	SArenaCompetitor* tempCompetitor = &m_ArenaManagedData.at(rank); //获取参赛者的指针
	tempCompetitor->m_IsLocked = false; //修改锁定标记
	rfalse("%d竞技场解锁完毕", arenaID);
	return tempCompetitor;
}

void CArenaMoudel::_GenerateRank()
{
	vector< ArenaRobotLevelInfo> levelInfoList = CConfigManager::getSingleton()->GetArenaRobotLevelInfo(); //获取机器人登记信息列表 
	for (auto levelInfo : levelInfoList) //循环遍历每个排名段的等级信息，初始化机器人竞技场信息 
	{
		for (int i = levelInfo.uper; i < levelInfo.lower + 1; i++)
		{
			SArenaCompetitor newCompetitor;
			memset(&newCompetitor, 0, sizeof(SArenaCompetitor));
			_RandomRobotInfo(levelInfo.level, levelInfo.heroLevel, levelInfo.rankLevel, levelInfo.starLevel, levelInfo.skillLevel, newCompetitor, i);
		}
	}
	/*
	SArenaCompetitor newCompetitor;
	memset(&newCompetitor, 0, sizeof(newCompetitor));
	for (int i = 1; i < 11; i++)
	{
	_RandomRobotInfo(22, 3, 3, 22, newCompetitor, i);
	}
	for (int i = 11; i < 31; i++)
	{
	_RandomRobotInfo(21, 3, 3, 20, newCompetitor, i);

	}

	for (int i = 31; i < 101; i++)
	{
	_RandomRobotInfo(20, 3, 2, 16, newCompetitor, i);

	}

	for (int i = 101; i < 201; i++)
	{
	_RandomRobotInfo(19, 3, 2, 14, newCompetitor, i);

	}

	for (int i = 201; i < 401; i++)
	{
	_RandomRobotInfo(18, 2, 2, 12, newCompetitor, i);
	}

	for (int i = 401; i < 601; i++)
	{
	_RandomRobotInfo(17, 2, 2, 10, newCompetitor, i);
	}

	for (int i = 601; i < 801; i++)
	{
	_RandomRobotInfo(16, 2, 2, 9, newCompetitor, i);
	}

	for (int i = 801; i < 1001; i++)
	{
	_RandomRobotInfo(15, 2, 2, 8, newCompetitor, i);
	}

	for (int i = 1001; i < 2001; i++)
	{
	_RandomRobotInfo(14, 2, 2, 7, newCompetitor, i);
	}

	for (int i = 2001; i < 4001; i++)
	{
	_RandomRobotInfo(13, 1, 1, 6, newCompetitor, i);
	}

	for (int i = 4001; i < 6001; i++)
	{
	_RandomRobotInfo(12, 1, 1, 5, newCompetitor, i);
	}

	for (int i = 6001; i < 8001; i++)
	{
	_RandomRobotInfo(11, 1, 1, 4, newCompetitor, i);
	}

	for (int i = 8001; i < 10001; i++)
	{
	_RandomRobotInfo(10, 1, 1, 3, newCompetitor, i);
	}
	*/

}


void CArenaMoudel::_RandomRobotInfo(int level, int heroLevel, int rank, int star, int skillLevel, SArenaCompetitor& newRobot, int arenaRank)
{
	newRobot.m_AernaID = ++m_RobotID;

	newRobot.m_TeamLevel = level;
	const vector<int> rangeIDList = CConfigManager::getSingleton()->GetRangeHeroIDList();
	const vector<int> meleeIDList = CConfigManager::getSingleton()->GetMeleeHeroIDList();
	//随机ID 2个近战 3个远程
	newRobot.m_HeroList[0].m_dwHeroID = meleeIDList[CRandom::RandRange(0, meleeIDList.size() - 1)];
	do //这么做是为了不出现重复武将,虽然很难看 但无额外消耗
	{
		newRobot.m_HeroList[1].m_dwHeroID = meleeIDList[CRandom::RandRange(0, meleeIDList.size() - 1)];
	} while (newRobot.m_HeroList[1].m_dwHeroID == newRobot.m_HeroList[0].m_dwHeroID);

	//远程武将
	newRobot.m_HeroList[2].m_dwHeroID = rangeIDList[CRandom::RandRange(0, rangeIDList.size() - 1)];
	do
	{
		newRobot.m_HeroList[3].m_dwHeroID = rangeIDList[CRandom::RandRange(0, rangeIDList.size() - 1)];

	} while (newRobot.m_HeroList[2].m_dwHeroID == newRobot.m_HeroList[3].m_dwHeroID);

	do
	{
		newRobot.m_HeroList[4].m_dwHeroID = rangeIDList[CRandom::RandRange(0, rangeIDList.size() - 1)];
	} while (newRobot.m_HeroList[4].m_dwHeroID == newRobot.m_HeroList[3].m_dwHeroID || newRobot.m_HeroList[4].m_dwHeroID == newRobot.m_HeroList[2].m_dwHeroID);

	const HeroConfig* heroConfig = CConfigManager::getSingleton()->GetHeroConfig(newRobot.m_HeroList[0].m_dwHeroID);
	if (heroConfig == nullptr)
	{
		rfalse("竞技场获取武将数据失败%d:", newRobot.m_HeroList[0].m_dwHeroID);
		return;
	}
	strcpy_s(newRobot.m_CompetitorName, m_RobotNameConfig.RandomName().c_str()); //随机名字
	newRobot.m_CompetitorIcon = heroConfig->HeroId;
	//memcpy(newRobot.m_CompetitorIcon, );
	//TODO 做的随机值
	//等级	//进阶//星级//技能等级
	for (int i = 0; i < 5; i++)
	{
		const HeroConfig *config = CConfigManager::getSingleton()->GetHeroConfig(newRobot.m_HeroList[i].m_dwHeroID);
		int randomHeroLevel = heroLevel;
		if (randomHeroLevel > 1) //对武将等级做随机处理
		{
			randomHeroLevel += CRandom::RandRange(-1, 1);
		}
		newRobot.m_HeroList[i].m_dwLevel = randomHeroLevel;
		newRobot.m_HeroList[i].m_dwRankLevel = rank;
		const HeroConfig* curConfig = CConfigManager::getSingleton()->GetHeroConfig(newRobot.m_HeroList[i].m_dwHeroID);
		if (curConfig->Star > star)
			star = curConfig->Star;
		newRobot.m_HeroList[i].m_dwStarLevel = star;

		newRobot.m_HeroList[i].m_SkillInfoList[0].m_dwSkillLevel = skillLevel;
		newRobot.m_HeroList[i].m_SkillInfoList[0].m_dwSkillID = config->Skill1;

		newRobot.m_HeroList[i].m_SkillInfoList[1].m_dwSkillLevel = skillLevel;
		newRobot.m_HeroList[i].m_SkillInfoList[1].m_dwSkillID = config->Skill2;

		newRobot.m_HeroList[i].m_SkillInfoList[2].m_dwSkillLevel = skillLevel;
		newRobot.m_HeroList[i].m_SkillInfoList[2].m_dwSkillID = config->Skill3;

		newRobot.m_HeroList[i].m_SkillInfoList[3].m_dwSkillLevel = skillLevel;
		newRobot.m_HeroList[i].m_SkillInfoList[3].m_dwSkillID = config->Skill4;


	}
	newRobot.m_ArenaRank = arenaRank;
	m_ArenaManagedData[arenaRank] = newRobot;
	m_ArenaIDRankMapping[newRobot.m_AernaID] = arenaRank;
}

bool CArenaMoudel::Release()
{
	BackupArenaRankData();
	if (m_pTop50th != nullptr)
	{
		delete[] m_pTop50th;
		m_pTop50th = nullptr;
	}

	return true;
}


void CArenaMoudel::BackupArenaRankData()
{
	int64_t startTime;
	int64_t endTime;
	_time64(&startTime);
	ofstream out("dbTables\\SanguoArenaRankList.txt", ios::binary);
	out.write((char*)&m_lastRoutineTime, sizeof(__int64)); //保存上次发放竞技场奖励以及重置竞技场数据的时间
	int size = m_ArenaManagedData.size();
	out.write((char*)&size, sizeof(int)); //保存竞技场人数
	SBaseArenaCompetitor tempData;
	for (auto iter : m_ArenaManagedData) //保存竞技场数据
	{
		out.write((char*)&iter.first, sizeof(int));
		tempData = iter.second;
		out.write((char*)&tempData, sizeof(SBaseArenaCompetitor));
	}
	out.close();
	
	_time64(&endTime);
	rfalse("备份竞技场数据,备份耗时：%d秒", endTime - startTime);
}
///玩家下线时调用此接口，把在线数据更新到托管数据中
bool CArenaMoudel::SetPlayerManagedArenaData(CBaseDataManager* pBaseDataMgr)
{
	//获取玩家的竞技场托管数据，更新玩家数据
	int ArenaID = ARENA_ID(pBaseDataMgr->GetSID());
	int rank = GetArenaRank(ArenaID);
	if (rank == -1)
		return false;
	SArenaCompetitor& competitor = m_ArenaManagedData.at(rank);

	competitor.m_AernaID = ArenaID; //竞技场ID
	competitor.m_ArenaRank = rank;
	pBaseDataMgr->GetArenaDefenseTeamInfo(competitor.m_HeroList);
	for (int i = 0; i < 5; ++i)
	{
		pBaseDataMgr->GetHeroFateAttribute(competitor.m_HeroList[i].m_dwHeroID, competitor.m_FateAttr[i].fateAttr, 4);
	}
	competitor.m_TeamLevel = pBaseDataMgr->GetMasterLevel();
	if (competitor.m_curOpponentID != -1)//解锁竞技场对手
	{
		rfalse("玩家%d下线", pBaseDataMgr->GetSID());
		UnlockCompetitor(competitor.m_curOpponentID); 
	}
	competitor.m_curOpponentID = -1;
	//更新名字
	strcpy_s(competitor.m_CompetitorName,pBaseDataMgr->GetName().c_str());
	
	CPlayer * pPlayer = (CPlayer *)GetPlayerBySID(pBaseDataMgr->GetSID())->DynamicCast(IID_PLAYER);
	if (pPlayer == nullptr)
		return false;
	memset(competitor.m_FactionName, 0, sizeof(char) * 32);
	lite::Variant ret;//从lua获取返回值
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "SI_vip_getlv")[pBaseDataMgr->GetSID()](&ret);
	g_Script.CleanCondition();
	if (ret.dataType == LUA_TNIL)
		return false;
	competitor.m_VipLevel = (int)ret; //获取VIP

	lite::Variant ret2;//从lua获取返回值
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "GetPlayerFactionName")[pBaseDataMgr->GetSID()](&ret2);
	g_Script.CleanCondition();
	LPCSTR Txt = (LPCSTR)ret2;
	if (Txt == nullptr)
	{
		rfalse("离线竞技场获取军团名为空");
		return  false;
	}
	strcpy_s(competitor.m_FactionName, Txt);
	return true;

}


///@brief 获取在线玩家竞技场数据
void CArenaMoudel::_GetOnlineArenaData(CPlayer *pPlayer, SBaseArenaCompetitor & competitor)
{
	CBaseDataManager *baseDataMgr = pPlayer->GetBaseDataMgr();
	competitor.m_AernaID = ARENA_ID(baseDataMgr->GetSID());
	competitor.m_ArenaRank = m_ArenaIDRankMapping.at(competitor.m_AernaID);
	competitor.m_CompetitorIcon = baseDataMgr->GetIconID();
	strcpy_s(competitor.m_CompetitorName, baseDataMgr->GetName().c_str());
	competitor.m_TeamLevel = baseDataMgr->GetMasterLevel();
	baseDataMgr->GetArenaDefenseTeamInfo(competitor.m_HeroList);

	memset(competitor.m_FactionName, 0, sizeof(char) * 32);
	lite::Variant ret;//从lua获取返回值
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "SI_vip_getlv")[baseDataMgr->GetSID()](&ret);
	g_Script.CleanCondition();
	competitor.m_VipLevel = (int)ret; //获取VIP

	lite::Variant ret2;//从lua获取返回值
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "GetPlayerFactionName")[baseDataMgr->GetSID()](&ret2);
	g_Script.CleanCondition();
	LPCSTR Txt = (LPCSTR)ret2;
	if (Txt == nullptr)
		rfalse("竞技场发送邮件获取VIP等级为空1");
	strcpy_s(competitor.m_FactionName, Txt);
	
	
	for (int i = 0; i < 5; ++i)
	{
		baseDataMgr->GetHeroFateAttribute(competitor.m_HeroList[i].m_dwHeroID, competitor.m_FateAttr[i].fateAttr, 4);
	}
}

SArenaCompetitor*  CArenaMoudel::_GetManagedArenaData(int arenaID)
{
	try
	{
		int rank = m_ArenaIDRankMapping.at(arenaID);
		return &m_ArenaManagedData.at(rank);
	}
	catch (const std::out_of_range& oor)
	{
		//rfalse("Out of Range error: %s", oor.what());
	}
	return nullptr;
}

int CArenaMoudel::GetArenaRank(const int arenaID)
{
	if (m_ArenaIDRankMapping.find(arenaID) == m_ArenaIDRankMapping.end())
	{
		//rfalse("查询%d玩家的竞技场排名失败", arenaID);
		return -1;
	}
	return m_ArenaIDRankMapping.at(arenaID);
}


///@brief 更新前50排行榜
bool CArenaMoudel::UpdateTop50th()
{
	if (m_ArenaIDRankMapping.size() < 50)
	{
		rfalse("竞技场排行榜未初始化");
		return false;
	}
	rfalse("竞技场排行开始更新");
	map<int, SArenaCompetitor>* pManagedData = &m_ArenaManagedData;
	SArenaPlayerReducedData* Top50th = m_pTop50th;
	FunctionPtr &UpdtateTop50thFucn = m_UpdtateTop50thFucn;
	int count = 0;

	for (auto &iter : m_ArenaManagedData)
	{
		SArenaPlayerReducedData  &tempReducedData = Top50th[count];
		 SBaseArenaCompetitor & tempComp = iter.second;
		CPlayer* playerPtr = (CPlayer*)GetPlayerBySID(GetSIDFromArenaID(tempComp.m_AernaID))->DynamicCast(IID_PLAYER);
		if (playerPtr != nullptr)//如果是在线玩家，则用其在线数据更新竞技场排名信息
			_GetOnlineArenaData(playerPtr, tempComp);

		memset(&tempReducedData.m_CompetitorName, 0, sizeof(char) * 32);
		strcpy(tempReducedData.m_CompetitorName, tempComp.m_CompetitorName);
		tempReducedData.m_CompetitorIconID = tempComp.m_CompetitorIcon;
		tempReducedData.m_TeamLevel = tempComp.m_TeamLevel; //等级赋值
		tempReducedData.m_ArenaID = tempComp.m_AernaID; //竞技场ID
		tempReducedData.m_Rank = tempComp.m_ArenaRank; //竞技场排名
		//简化版战斗力计算
		tempReducedData.m_FightingCapacity = 0;
		for (int i = 0; i < 5; i++)
		{
			tempReducedData.m_FightingCapacity += CHeroAttributeCalculator::getSingleton()->CalculateSpecifyHeroComabtPower(&tempComp.m_HeroList[i], tempComp.m_FateAttr[i].fateAttr);
		}
		++count;
		if (count >= 50)
			break;
	}
	rfalse("竞技场更新TOP50列表");
	return true;
}

//时间结束，获取挑战者的过关数据，解锁被挑战玩家 ， 
void CArenaMoudel::_Timeout(DWORD playerSID)
{
	SArenaCompetitor * managedData = _GetManagedArenaData(ARENA_ID(playerSID));
	if (nullptr == managedData)
		return;

	if (UnlockCompetitor(managedData->m_curOpponentID) == nullptr) //解锁对手
		return;
	managedData->m_curOpponentID = -1;
	managedData->m_EventPtr = nullptr;
}

void CArenaMoudel::_GetTop50CompetitorDetail(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer)
{
	try
	{
		SQGetTop50PlayerDetail * detailMsg = (SQGetTop50PlayerDetail*)pSArenaMsg;
		CPlayer * ptargetPlayer = (CPlayer*)GetPlayerBySID(detailMsg->m_ArenaID - MAX_ROBOT_NUM)->DynamicCast(IID_PLAYER);
		SArenaCompetitor* pComp = nullptr;
		SAGetTop50PlayerDetail msg;
		if (ptargetPlayer == nullptr) //如果不是在线玩家
		{
			if (_GetManagedArenaData(detailMsg->m_ArenaID) != nullptr)
				msg.m_BaseArenaData = *(SBaseArenaCompetitor*)_GetManagedArenaData(detailMsg->m_ArenaID);
		}
		else//是在线玩家
		{
			_GetOnlineArenaData(ptargetPlayer, msg.m_BaseArenaData);
		}
		//TODO 返回失败
		g_StoreMessage(dnidClient, &msg, sizeof(msg));
	}
	catch (const std::out_of_range& oor)
	{
		rfalse("Out of Range error: %s, 获取排行榜玩家信息出错，玩家SID：%d", oor.what(), pPlayer->GetSID());
	}

}

//此函数类似递归，只不过函数执行是异步执行，所以函数不会导致栈溢出，
void CArenaMoudel::ArenaRoutine()
{
	//重置竞技场挑战次数的lambda
	rfalse("\n\n竞技场模块日常历程===================");
	//定时器
	INT64 now, twelveOclock;
	_time64(&now);
	rfalse("现在时间%d", now);
	rfalse("上次执行日常时间", m_lastRoutineTime);
	rfalse("时间差%d", now - m_lastRoutineTime);
	//判断是否在今日已经执行过竞技场日常操作
	tm lastRoutineDay;
	tm curRoutineDay;
	errno_t tempLastErr = _localtime64_s(&lastRoutineDay, &m_lastRoutineTime);
	errno_t tempCurtErr = _localtime64_s(&curRoutineDay, &now);
	if (tempLastErr || tempCurtErr) //报错返回
		return;
	int result = CompareTimeOfYear(curRoutineDay, lastRoutineDay); //判断上次日常时间和现在时间是否为同一天，如果是则返回
	if (result < 1)
		return;
	m_lastRoutineTime = now + 10; //更新日常时间记录


	map<int, SArenaCompetitor> * pArenaManagedData = &m_ArenaManagedData;
	FunctionPtr ResetChallengeCount = [pArenaManagedData](int a)	
	{
		rfalse("竞技场模块重置挑战次数");
		//for_each(pArenaManagedData->begin(), pArenaManagedData->end(), [&](pair<int, SArenaCompetitor> arenaPair){ arenaPair.second.m_ChallengeCount = 5; });
		for (auto &Iter : *pArenaManagedData)
		{
			Iter.second.m_ChallengeCount = 5;

			//竞技场模块重置购买次数和重置次数
			Iter.second.m_BuyedNum = 0;
			Iter.second.m_ResetNum = 0;
		}
	};

	//TODO 邮件发送`
	map<int, int> *pIDandRankMapping = &m_ArenaIDRankMapping;
	FunctionPtr SetArenaRewardEmail = [pIDandRankMapping](int a)
	{
		///5个参数 一次填入要替换的值  不需要替换的 用0占位
		/*lite::Variant ret2;
		LuaFunctor(g_Script, "Com_GetTipString")[2]["a"]["b"]["c"][0][0](&ret2);
		string Value = (string)ret2;*/

		MailInfo mailInfo;
		int timeRecord = timeGetTime();
		memset(&mailInfo, 0, sizeof(mailInfo));

		lite::Variant ret;//从lua获取vip等级
		LuaFunctor(g_Script, "SI_GetStr")[3](&ret);
		LPCSTR Txt = (LPCSTR)ret;
		if (Txt == nullptr)
			rfalse("竞技场发送邮件获取VIP等级为空1");
		
		strcpy_s(mailInfo.senderName, Txt);

		lite::Variant ret1;//
		LuaFunctor(g_Script, "SI_GetStr")[4](&ret1);
		LPCSTR Txt1 = (LPCSTR)ret1;
		if (Txt == nullptr)
			rfalse("竞技场发送邮件获取VIP等级为空2");
		strcpy_s(mailInfo.title, Txt1);

		for_each(pIDandRankMapping->begin(), pIDandRankMapping->end(), [&mailInfo](pair<int, int> arenaPair)  //循环遍历玩家，发送邮件奖励
		{
			ArenaRewardConfig configIter;
			if (arenaPair.first <= MAX_ROBOT_NUM)
				return;
			if (CConfigManager::getSingleton()->GetArenaRewardConfig(arenaPair.second, configIter) == false)
				return;
			if (configIter.Gold > 0)
			{
				mailInfo.szItemInfo[0].itype = GoodsType::money;//同步金币
				mailInfo.szItemInfo[0].num = configIter.Gold;
			}

			if (configIter.Gem > 0)
			{
				mailInfo.szItemInfo[1].itype = GoodsType::diamond; //同步宝石
				mailInfo.szItemInfo[1].num = configIter.Gem;
			}

			if (configIter.Honor > 0)
			{
				mailInfo.szItemInfo[2].itype = GoodsType::honor; //同步宝石
				mailInfo.szItemInfo[2].num = configIter.Honor;
			}

			if (configIter.MedicineNumber > 0)
			{
				mailInfo.szItemInfo[3].itype = GoodsType::item; //同步经验药材
				mailInfo.szItemInfo[3].id = configIter.Medicine;
				mailInfo.szItemInfo[3].num = configIter.MedicineNumber;
			}

			if (configIter.RefinedMaterialNumber > 0)
			{
				mailInfo.szItemInfo[4].itype = GoodsType::item; //同步材料
				mailInfo.szItemInfo[4].id = configIter.RefinedMaterial;
				mailInfo.szItemInfo[4].num = configIter.RefinedMaterialNumber;
			}


			lite::Variant ret2;//从lua获取vip等级
			LuaFunctor(g_Script, "SI_GetStr")[5][arenaPair.second](&ret2);
			LPCSTR Txt2 = (LPCSTR)ret2;
			if (Txt2 == nullptr)
				rfalse("竞技场发送邮件获取VIP等级为空3");
			strcpy_s(mailInfo.content, Txt2);
			MailMoudle::getSingleton()->SendMail_SanGuo(arenaPair.first - MAX_ROBOT_NUM, 1440/*1440分钟以后失效*/, &mailInfo);
		});
		timeRecord = timeGetTime() - timeRecord;
		rfalse("竞技场分发奖励用时 %d", timeRecord);
	};

	TimerEvent_SG::SetCallback(ResetChallengeCount, 2); //lambda传入定时器 2S后触发
	TimerEvent_SG::SetCallback(SetArenaRewardEmail, 4); //lambda传入定时器 4S后触发

}


void CArenaMoudel::SendLastChallengTime(DNID dnidClient, int sid)
{

	SALastArenaChallengeTimeSpan msg;
	if (GetLastChallengTimespan(sid, msg.m_TimeSpanSeconds) == false)
		return;
	g_StoreMessage(dnidClient, &msg, sizeof(msg));

}

bool CArenaMoudel::GetLastChallengTimespan(const int sid, DWORD &timespanOut)
{
	SArenaCompetitor* pComp = _GetManagedArenaData(ARENA_ID(sid));
	if (pComp == nullptr)
	{
		return false;
	}
	__int64 now;
	_time64(&now);
	timespanOut = now - pComp->m_LastChallengeTime;
	return true;
}

bool CArenaMoudel::GetBuyChallengeCount(const int sid, DWORD &BuyNum)
{

	SArenaCompetitor* pComp = _GetManagedArenaData(ARENA_ID(sid));
	if (pComp == nullptr)
	{
		return false;
	}


	BuyNum = pComp->m_BuyedNum;
	return true;
}
bool CArenaMoudel::GetReSetCountDownCount(const int sid, DWORD &RestNum)
{
	SArenaCompetitor* pComp = _GetManagedArenaData(ARENA_ID(sid));
	if (pComp == nullptr)
	{
		return false;
	}

	RestNum = pComp->m_ResetNum;
	return true;
}

bool CArenaMoudel::GetArenaChallengCount(const int sid, DWORD &countOut)
{
	SArenaCompetitor* pComp = _GetManagedArenaData(ARENA_ID(sid));
	if (pComp == nullptr)
	{
		return false;
	}
	countOut = pComp->m_ChallengeCount;
	return true;
}
void CArenaMoudel::ChangeNametoArena(const int sid, char* name)
{
	SArenaCompetitor* pComp = _GetManagedArenaData(ARENA_ID(sid));
	if (pComp == NULL)
		return;
	memcpy(pComp->m_CompetitorName, name, sizeof(*name));
}
DWORD CArenaMoudel::GetBestRank(const int sid)
{
	SArenaCompetitor* pComp = _GetManagedArenaData(ARENA_ID(sid));
	if (pComp == nullptr)
	{
		return  m_ArenaIDRankMapping.size()+1;
	}
	return pComp->m_BestRank;
}

const SBaseArenaCompetitor* CArenaMoudel::GetManagedArenaData(int arenaID)
{
	auto rankIter = m_ArenaIDRankMapping.find(arenaID);
	if (rankIter == m_ArenaIDRankMapping.end())
		return nullptr;
	auto dataIter = m_ArenaManagedData.find(rankIter->second);
	return dataIter == m_ArenaManagedData.end() ? nullptr : &dataIter->second;
}
