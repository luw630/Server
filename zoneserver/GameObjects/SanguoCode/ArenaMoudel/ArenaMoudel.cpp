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
		in.read((char*)&maxArenaRank, sizeof(int)); //��ȡ�������������
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
	//����ǰ50����Ұ�
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
	case SSGArenaMsg::Arena_SetBuyChallengeNum://������������ս����
	{
		_BuyChallengeNum(dnidClient, pSArenaMsg, pPlayer);
		break;
	}
	case SSGArenaMsg::Arena_ResetCountDown://���� ������������ս��������ȴʱ��
	{
		_ResetCountDown(dnidClient, pSArenaMsg, pPlayer);
		break;
	}
	default:
		break;
	}
}

//���þ�������սʱ��
void CArenaMoudel::_ResetCountDown(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer)
{

	if (dnidClient == NULL || pSArenaMsg == NULL || pPlayer == NULL)
		return;

	CBaseDataManager* pBaseMgr = pPlayer->GetBaseDataMgr(); //ͨ������

	if (pBaseMgr == NULL)
		return;
	//��ȡ����������
	int rank = m_ArenaIDRankMapping.at(ARENA_ID(pBaseMgr->GetSID()));

	SArenaCompetitor*  pSArenaData = &m_ArenaManagedData.at(rank);

	int ResetNum = pSArenaData->m_ResetNum;

	//���������Ϣ     �߻�˵���޴����� 
	lite::Variant ret;//��lua��ȡ����ֵ
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "SI_vip_getDetail")[pBaseMgr->GetSID()][VipLevelFactor::VF_ReSetArenaCD_Num](&ret);
	g_Script.CleanCondition();
	int Maxnum = (int)ret;

	lite::Variant ret1;//��lua��ȡ����ֵ
	LuaFunctor(g_Script, "SI_getNeedvalue")[pBaseMgr->GetSID()][Expense::Ex_ReSetCD][ResetNum + 1](&ret1);
	g_Script.CleanCondition();
	int Money = (int)ret1;


	//�����һ�ε���սʱ��
	__int64 now;
	_time64(&now);
	int timeSpan = now - pSArenaData->m_LastChallengeTime;
	if (timeSpan > 600)
	{
		return;
	}
	pSArenaData->m_LastChallengeTime = _time64(&now) - 600;

	//�۳���ʯ����
	pBaseMgr->DecGoods_SG(GoodsType::diamond, 0, Money,GoodsWay::practiceForce);
	pSArenaData->m_ResetNum++;

	SAResetCD_Respoens Msg;
	Msg.ResetNum = pSArenaData->m_ResetNum;


	g_StoreMessage(dnidClient, &Msg, sizeof(Msg));
}


//������������ս����
void CArenaMoudel::_BuyChallengeNum(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer)
{

	if (dnidClient == NULL || pSArenaMsg == NULL || pPlayer == NULL)
		return;

	CBaseDataManager* pBaseMgr = pPlayer->GetBaseDataMgr(); //ͨ������

	if (pBaseMgr == NULL)
		return;
	//��ȡ����������
	int rank = m_ArenaIDRankMapping.at(ARENA_ID(pBaseMgr->GetSID()));

	SArenaCompetitor*  pSArenaData = &m_ArenaManagedData.at(rank);


	lite::Variant ret1;//��lua��ȡ����ֵ
	LuaFunctor(g_Script, "SI_getNeedvalue")[pBaseMgr->GetSID()][Expense::Ex_ArenaTicket][pSArenaData->m_BuyedNum + 1](&ret1);
	g_Script.CleanCondition();
	if (ret1.dataType == LUA_TNIL)
	{
		rfalse("��ȡ������VIP�����������");
		return;
	}
		
	int cost = (int)ret1;
	if (pBaseMgr->GetDiamond() < cost) //���Ԫ�����������򷵻�
		return;

	//���������Ϣ
	lite::Variant ret;//��lua��ȡ����ֵ
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "SI_vip_getDetail")[pBaseMgr->GetSID()][VipLevelFactor::VF_BuyArenaTicket_Num](&ret);
	g_Script.CleanCondition();
	int Maxnum = (int)ret;  //��ȡ��ǰ�ȼ�����������
	if (ret.dataType == LUA_TNIL)
	{
		rfalse("��ȡ��������ǰVIP�ȼ���������������");
		return;
	}
	if (pSArenaData->m_BuyedNum >= Maxnum) //������������
	{
		return;
	}
	else
	{
		//�۳���ʯ����
		pBaseMgr->DecGoods_SG(GoodsType::diamond, 0, cost, GoodsWay::practiceForce);
		pSArenaData->m_ChallengeCount = 5;	//��ս��������
		++pSArenaData->m_BuyedNum;//���¹������
	}

	//�����һ�ε���սʱ��
	__int64 now;
	_time64(&now);
	int timeSpan = now - pSArenaData->m_LastChallengeTime;
	pSArenaData->m_LastChallengeTime = _time64(&now) - 600;

	SArenaBaseMsg pSbMsg;
	pSbMsg.BuyChallengeNum = pSArenaData->m_BuyedNum;
	pSbMsg.ChallengeCount = pSArenaData->m_ChallengeCount;
	//������Ϣ���ͻ���
	g_StoreMessage(dnidClient, &pSbMsg, sizeof(pSbMsg));

	//wk 20150706 ���������־
	g_Script.CallFunc("db_gm_setoperation", pBaseMgr->GetSID(), 4, 2, pSArenaData->m_BuyedNum);

}

void CArenaMoudel::_OnRecvChallengeOver(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer)
{
	SQChallengeOver * pOverMsg = (SQChallengeOver *)pSArenaMsg;

	CBaseDataManager* pBaseMgr = pPlayer->GetBaseDataMgr(); //ͨ������
	//��������
	SArenaCompetitor* pCompetitor = nullptr;//����
	try
	{
		int rank = m_ArenaIDRankMapping.at(ARENA_ID(pBaseMgr->GetSID()));
		SArenaCompetitor *pMyselt = &m_ArenaManagedData.at(rank);
		pCompetitor = _GetManagedArenaData(pMyselt->m_curOpponentID);
		if (nullptr == pCompetitor)
		{
			TalkToDnid(dnidClient, "�����������ֲ�����");
			return;
		}
		UnlockCompetitor(pCompetitor->m_AernaID); //�������
		pMyselt->m_curOpponentID = -1;
		// ע����ʱ���¼�
		pMyselt->m_EventPtr->Interrupt();
		pMyselt->m_EventPtr = nullptr;
		if (pOverMsg->m_BattleResult == 1) //ս��ʤ��,����ֻ�������
		{
			if (!CBattleVerification::getSingleton()->ArenaBattleVerification(pOverMsg->verifyData, pMyselt->m_LastChallengeTime, *pBaseMgr))
			{
				LuaFunctor(g_Script, "set_CreditDataValue")[pBaseMgr->GetSID()][2][1]();
				TalkToDnid(dnidClient, "��⵽����");
				return;
			}
			//��սʤ�� ˢ����սʱ��
			pMyselt->m_LastChallengeTime -= 600;
			int myRank = m_ArenaIDRankMapping.at(ARENA_ID(pBaseMgr->GetSID()));
			int oppoRank = m_ArenaIDRankMapping.at(pCompetitor->m_AernaID); //�������ڵ�����
			if (oppoRank >= myRank) //����Է�����С���ҵ�ǰ�������򲻴���
			{
				SAChallengeOver oMsg;
				g_StoreMessage(dnidClient, &oMsg, sizeof(oMsg));
				return;
			}
			SArenaCompetitor tempCompetitor = m_ArenaManagedData.at(oppoRank); //��ȡ������Ϣ
			m_ArenaManagedData.at(myRank).m_ArenaRank = oppoRank; //��������
			m_ArenaManagedData.at(oppoRank) = m_ArenaManagedData.at(myRank); //��������
			tempCompetitor.m_ArenaRank = myRank; //������������
			m_ArenaManagedData.at(myRank) = tempCompetitor; //������������


			//ID����ӳ������
			m_ArenaIDRankMapping.at(ARENA_ID(pBaseMgr->GetSID())) = oppoRank;
			int rank = m_ArenaIDRankMapping.at(ARENA_ID(pBaseMgr->GetSID()));
			m_ArenaIDRankMapping.at(tempCompetitor.m_AernaID) = myRank;
			rank = m_ArenaIDRankMapping.at(tempCompetitor.m_AernaID);

			//�����������
			pMyselt = &m_ArenaManagedData.at(oppoRank);
			if (pMyselt->m_BestRank > oppoRank)
			{

				//�����õݹ���㽱��Ԫ���������ݹ��ȱ�㣺1.���޵ݹ����ջ�����2.�ɶ��Բ�
				std::function<int(int, int)> BestArenaRank = [&BestArenaRank](int curRank, int newRank)->int
				{

					ArenaRewardConfig rewarConfig;
					CConfigManager::getSingleton()->GetArenaRewardConfig(curRank, rewarConfig);
					int result = 0;
					if (rewarConfig.rankUp <= newRank || curRank == 0 || newRank == 0) //�ݹ�����߼�
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
				int rewardDiamond = BestArenaRank(myRank, oppoRank); //��ȡ��ʯ��������

				pMyselt->m_BestRank = oppoRank;
				SABirthBestRecords msg;
				msg.m_DiamondRewards = rewardDiamond; //��ʱ���ڽ����㷨
				msg.m_NewRecordRank = oppoRank;
				msg.m_OriginalRecordRank = myRank;
				g_StoreMessage(dnidClient, &msg, sizeof(msg));

				//������������
				int bRank = myRank - oppoRank ;
				//���ͽ����ʼ�
				MailInfo mailInfo;
				memset(&mailInfo, 0, sizeof(mailInfo));

				lite::Variant ret;//��lua��ȡvip�ȼ�
				LuaFunctor(g_Script, "SI_GetStr")[3](&ret);
				LPCSTR Txt = (LPCSTR)ret;
				if (Txt == nullptr)
					rfalse("�����������ʼ���ȡVIP�ȼ�Ϊ��4");
				strcpy_s(mailInfo.senderName, Txt);

				lite::Variant ret1;//��lua��ȡvip�ȼ�
				LuaFunctor(g_Script, "SI_GetStr")[1](&ret1);
				LPCSTR Txt1 = (LPCSTR)ret1;
				if (Txt1 == nullptr)
					rfalse("�����������ʼ���ȡVIP�ȼ�Ϊ��5");
				strcpy_s(mailInfo.title, Txt1);

				lite::Variant ret2;//��lua��ȡvip�ȼ�
				LuaFunctor(g_Script, "SI_GetStr")[2][oppoRank][bRank](&ret2);
				LPCSTR Txt2 = (LPCSTR)ret2;
				if (Txt2 == nullptr)
					rfalse("�����������ʼ���ȡVIP�ȼ�Ϊ��6");
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
		rfalse("Out of Range error: %s, ��������ս�����������SID��%d", oor.what(), pBaseMgr->GetSID());
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
			rfalse("��������ʼ��ս �޷��ҵ�����");
			TalkToDnid(dnidClient, "�������޷��ҵ�����");
			result = false;
			g_StoreMessage(dnidClient, &msg, sizeof(msg));
			return;
		}

		if (result == true && m_ArenaIDRankMapping.at(challengeMsg->m_ArenaID) != challengeMsg->m_Rank)
		{
			TalkToDnid(dnidClient, "�������������仯");
			result = false;
			g_StoreMessage(dnidClient, &msg, sizeof(msg));
			return;
		}

		int compRank = m_ArenaIDRankMapping.at(challengeMsg->m_ArenaID);
		if (result == true && compRank < 1 && compRank >m_ArenaIDRankMapping.size())
		{
			TalkToDnid(dnidClient, "��������������Χ");
			result = false;
			g_StoreMessage(dnidClient, &msg, sizeof(msg));
			return;
		}

		SArenaCompetitor*	tempCompetitor = &m_ArenaManagedData.at(compRank); //TODO����������߼�BUG����Ϊ���ֵ����ݿ����Ѿ������ı䣬�����ѡ�ж���ʱ�������Ѿ���һ������Ҫ�Ż�
		if (result == true && true == tempCompetitor->m_IsLocked)
		{
			TalkToDnid(dnidClient, "������ս����");
			result = false;
			g_StoreMessage(dnidClient, &msg, sizeof(msg));
			return;
		}

		CBaseDataManager* ptrBaseDataMgr = pPlayer->GetBaseDataMgr();
		int sid = ptrBaseDataMgr->GetSID();
		SArenaCompetitor* pMySelf = _GetManagedArenaData(ARENA_ID(sid)); //��ȡ���������
		__int64 now;
		_time64(&now);
		int timeSpan = now - pMySelf->m_LastChallengeTime;
		if (result == true && timeSpan < 600)
		{
			string tip = "���ھ�����CD��, ���ϴ���ս�Ѿ����ˣ�";
			TalkToDnid(dnidClient, tip.c_str());
			result = false;
		}

		if (result == true && pMySelf->m_ChallengeCount <= 0)
		{
			TalkToDnid(dnidClient, "��ս����������");
			result = false;
			pMySelf->m_ChallengeCount = 0;
		}

		if (result == true)
		{
			///���¾�������ս�������������
			CMissionUpdate* ptrMissionUpdator = ptrBaseDataMgr->GetMissionUpdator();
			if (ptrMissionUpdator != nullptr)
				ptrMissionUpdator->UpdateDungeionMission(InstanceType::Arena, 1);

			//��һ����սʱ�����
			pMySelf->m_LastChallengeTime = now;
			//ս������
			FunctionPtr challengeTimeOver = std::bind(&CArenaMoudel::_Timeout, this, sid);
			pMySelf->m_EventPtr = TimerEvent_SG::SetCallback(challengeTimeOver, 150); //��¼��ʱ���¼�,2min30s�󴥷�
			pMySelf->m_curOpponentID = tempCompetitor->m_AernaID; //���ö���ָ��
			tempCompetitor->m_IsLocked = true;
			rfalse("%d��������ArenaID:%d", sid, tempCompetitor->m_AernaID);
			pMySelf->m_ChallengeCount -= 1;
			msg.m_Result = 1;

			//wk 20150706 ���������־
			g_Script.CallFunc("db_gm_setoperation", sid, 4, 1, pMySelf->m_ArenaRank);
		}

		g_StoreMessage(dnidClient, &msg, sizeof(msg));
	}
	catch (const std::out_of_range& oor)
	{
		rfalse("Out of Range error: %s, ��������ս��ҳ������SID��");
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

//���÷����佫����ȡ��������佫���ݣ������й������еķ�������ID��
void CArenaMoudel::_OnSetDefensiveTeam(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer)
{
	SQSetDefensiveTeam *pDMsg = (SQSetDefensiveTeam *)pSArenaMsg;
	


	CBaseDataManager * baseDataMgr = pPlayer->GetBaseDataMgr();
	SASetDefensiveTeam msg;
	for (int i = 0; i < 5; i++) //���������÷�������
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

	baseDataMgr->SetArenaDefenseTeam(pDMsg->m_HeroIDList); //���ﲻ�ø����й������е��佫��Ϣ����Ϊ�������ʱ��������ȡ���������佫����
	msg.m_Result = true;
	g_StoreMessage(dnidClient, &msg, sizeof(msg));
}

//�������й�ʽ�����3������
//No.1:�������x*0.8 - x*��0.001-0.08��
//No.2:������� x*0.9 - x*��0.001-0.08��
//No.3:������� x*0.98 - x*��0.001-0.08��
void CArenaMoudel::_OnRandomReplacement(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer) //����滻����
{
	int curRank;

	static float posIndexList[] = { 0.8f, 0.9f, 0.98f };
	static int topRank[] = { 1, 2, 3, 4 };
	CBaseDataManager* baseDataMgr = pPlayer->GetBaseDataMgr();
	try
	{
		if (m_ArenaIDRankMapping.find(ARENA_ID(baseDataMgr->GetSID())) == m_ArenaIDRankMapping.end())
		{
			if (DebutArena(dnidClient, baseDataMgr) == false) //�������������ʧ��  �򷵻�
				return;
		}
		curRank = m_ArenaIDRankMapping.at(ARENA_ID(baseDataMgr->GetSID())); //��ȡ��ҵ�ǰ����
		SAGetArenaCompetitors msg;
		SArenaCompetitor  *pManagedArena;
		if (curRank <= 4) //�����ǰ3��
		{
			int index = 0; //������Ϣ�±�
			for (int i = 0; i < 4; i++)
			{
				if (curRank == topRank[i])
					continue;
				CPlayer * comp = (CPlayer*)GetPlayerBySID(m_ArenaIDRankMapping.at(topRank[i]) - MAX_ROBOT_NUM)->DynamicCast(IID_PLAYER);
				if (comp == nullptr) //��������������
					msg.m_Competitors[index] = m_ArenaManagedData.at(topRank[i]);
				else//���������
					_GetOnlineArenaData(comp, msg.m_Competitors[index]);
				++index;
			}
			msg.m_curRank = curRank; //������ֵ
			g_StoreMessage(dnidClient, &msg, sizeof(msg));
			return; //ǰ3���������ֱ���˳�
		}

		float randomFactor;
		int tempRank = -1;
		for (int i = 0; i < 3; i++)//3������
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
				rfalse("���������IDΪ%d���ֳ�����", tempRank);
				return;
			}

			//��ҵ�SID = ArenaID - 10000;

			CPlayer * comp = (CPlayer*)GetPlayerBySID(curArenaID - MAX_ROBOT_NUM)->DynamicCast(IID_PLAYER);
			if (comp == nullptr) //��������������
				msg.m_Competitors[i] = m_ArenaManagedData.at(tempRank);
			else//���������
				_GetOnlineArenaData(comp, msg.m_Competitors[i]);
			string name(msg.m_Competitors[i].m_CompetitorName);
			memset(&msg.m_Competitors[i].m_CompetitorName[name.length()], 0, 32 - name.length());
		}
		msg.m_curRank = curRank; //������ֵ
		g_StoreMessage(dnidClient, &msg, sizeof(msg));
	}
	catch (const std::out_of_range& oor)
	{
		rfalse("Out of Range error: %s, ������������ֳ������SID��%d", oor.what(), baseDataMgr->GetSID());
	}

}

///��������������ҵȼ�>10������ؿ�
bool CArenaMoudel::DebutArena(DNID dnidClient, CBaseDataManager * pDataMgr)
{
	if (pDataMgr->GetMasterLevel() < 10 && pDataMgr->GetGMFlag() == false)
	{
		TalkToDnid(dnidClient, "��ǰ�ȼ��޷����뾺����");
		return false;
	}
	DWORD SID = pDataMgr->GetSID();
	SArenaCompetitor newCompetitor;
	newCompetitor.m_AernaID = ARENA_ID(SID); //��ȡ������ΨһID��
	//���ֵı���
	string playerName = "wrong name";
	int pos = pDataMgr->GetName().length() - 4;
	if (pos > 0)
		playerName = pDataMgr->GetName().substr(0, pos); //TODO:��ʱ��ô�����������

	strcpy_s(newCompetitor.m_CompetitorName, playerName.c_str()); //����

	pDataMgr->GetTop5Hero(newCompetitor.m_HeroList); //��ȡ�������� 
	DWORD defenseTeam[5];
	memset(defenseTeam, 0, sizeof(int) * 5);
	for (int i = 0; i < 5; i++) //�������������ݵĻ�ȡ 
		defenseTeam[i] = newCompetitor.m_HeroList[i].m_dwHeroID;
	pDataMgr->SetArenaDefenseTeam(defenseTeam);
	/// ����Ե������
	for (int i = 0; i < 5; ++i)
	{
		pDataMgr->GetHeroFateAttribute(newCompetitor.m_HeroList[i].m_dwHeroID, newCompetitor.m_FateAttr[i].fateAttr, 4);
	}
	newCompetitor.m_TeamLevel = pDataMgr->GetMasterLevel(); //��ȡ�ȼ�
	newCompetitor.m_CompetitorIcon = pDataMgr->GetIconID(); //��ȡͷ��ID

	newCompetitor.m_ArenaRank = m_ArenaManagedData.size() + 1; //��ǰ�ľ���������
	newCompetitor.m_BestRank = newCompetitor.m_ArenaRank;//�������
	newCompetitor.m_IsLocked = false;
	_time64(&newCompetitor.m_LastChallengeTime);
	newCompetitor.m_LastChallengeTime -= 6000;
	m_ArenaManagedData[newCompetitor.m_ArenaRank] = newCompetitor; //���а��������
	m_ArenaIDRankMapping[newCompetitor.m_AernaID] = newCompetitor.m_ArenaRank; //����ȫ��ID��������ӳ��
	
	//newCompetitor.m_ResetNum = 0;
	//��ʼ�����ô���
	newCompetitor.m_BuyedNum = 0;

	CPlayer * pPlayer = (CPlayer *)GetPlayerBySID(SID)->DynamicCast(IID_PLAYER);
	if (pPlayer == nullptr)
		return false;
	memset(newCompetitor.m_FactionName, 0, sizeof(char) * 32);
	lite::Variant ret;//��lua��ȡ����ֵ
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "SI_vip_getlv")[pDataMgr->GetSID()](&ret);
	g_Script.CleanCondition();
	if (ret.dataType == LUA_TNIL)
		return false;
	newCompetitor.m_VipLevel = (int)ret; //��ȡVIP

	lite::Variant ret2;//��lua��ȡ����ֵ
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "GetPlayerFactionName")[pDataMgr->GetSID()](&ret2);
	g_Script.CleanCondition();
	LPCSTR Txt = (LPCSTR)ret2;
	if (Txt == nullptr)
	{
		rfalse("������������ȡ������Ϊ��");
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
			rfalse("SIDΪ%d�����û�о���������", pBaseDataMgr->GetSID());
			return;
		}

		SAArenaLoginData msg;
		msg.m_ChallengeCount = pCompetitor->m_ChallengeCount; //��ȡ��ս����
		ConverTool::ConvertInt64ToBytes(pCompetitor->m_LastChallengeTime, msg.m_LastChallengeTime); //��ȡ��սʱ��
		g_StoreMessage(dnidClient, &msg, sizeof(msg));
	}
	catch (const std::out_of_range& oor)
	{
		rfalse("Out of Range error: %s ��������¼����ͬ���������SID��%d", oor.what(), pBaseDataMgr->GetSID());
	}


}

SArenaCompetitor* CArenaMoudel::UnlockCompetitor(const int arenaID)
{
	
	if (m_ArenaIDRankMapping.find(arenaID) == m_ArenaIDRankMapping.end()) //��ѯ��������
	{
		rfalse("%d����������,��ѯ��������", arenaID);
		return nullptr;
	}

	int rank = m_ArenaIDRankMapping.at(arenaID);
	if (m_ArenaManagedData.find(rank) == m_ArenaManagedData.end()) //����������Χ
	{
		rfalse(",%d����������,����������Χ", arenaID);
		return nullptr;
	}	

	SArenaCompetitor* tempCompetitor = &m_ArenaManagedData.at(rank); //��ȡ�����ߵ�ָ��
	tempCompetitor->m_IsLocked = false; //�޸��������
	rfalse("%d�������������", arenaID);
	return tempCompetitor;
}

void CArenaMoudel::_GenerateRank()
{
	vector< ArenaRobotLevelInfo> levelInfoList = CConfigManager::getSingleton()->GetArenaRobotLevelInfo(); //��ȡ�����˵Ǽ���Ϣ�б� 
	for (auto levelInfo : levelInfoList) //ѭ������ÿ�������εĵȼ���Ϣ����ʼ�������˾�������Ϣ 
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
	//���ID 2����ս 3��Զ��
	newRobot.m_HeroList[0].m_dwHeroID = meleeIDList[CRandom::RandRange(0, meleeIDList.size() - 1)];
	do //��ô����Ϊ�˲������ظ��佫,��Ȼ���ѿ� ���޶�������
	{
		newRobot.m_HeroList[1].m_dwHeroID = meleeIDList[CRandom::RandRange(0, meleeIDList.size() - 1)];
	} while (newRobot.m_HeroList[1].m_dwHeroID == newRobot.m_HeroList[0].m_dwHeroID);

	//Զ���佫
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
		rfalse("��������ȡ�佫����ʧ��%d:", newRobot.m_HeroList[0].m_dwHeroID);
		return;
	}
	strcpy_s(newRobot.m_CompetitorName, m_RobotNameConfig.RandomName().c_str()); //�������
	newRobot.m_CompetitorIcon = heroConfig->HeroId;
	//memcpy(newRobot.m_CompetitorIcon, );
	//TODO �������ֵ
	//�ȼ�	//����//�Ǽ�//���ܵȼ�
	for (int i = 0; i < 5; i++)
	{
		const HeroConfig *config = CConfigManager::getSingleton()->GetHeroConfig(newRobot.m_HeroList[i].m_dwHeroID);
		int randomHeroLevel = heroLevel;
		if (randomHeroLevel > 1) //���佫�ȼ����������
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
	out.write((char*)&m_lastRoutineTime, sizeof(__int64)); //�����ϴη��ž����������Լ����þ��������ݵ�ʱ��
	int size = m_ArenaManagedData.size();
	out.write((char*)&size, sizeof(int)); //���澺��������
	SBaseArenaCompetitor tempData;
	for (auto iter : m_ArenaManagedData) //���澺��������
	{
		out.write((char*)&iter.first, sizeof(int));
		tempData = iter.second;
		out.write((char*)&tempData, sizeof(SBaseArenaCompetitor));
	}
	out.close();
	
	_time64(&endTime);
	rfalse("���ݾ���������,���ݺ�ʱ��%d��", endTime - startTime);
}
///�������ʱ���ô˽ӿڣ����������ݸ��µ��й�������
bool CArenaMoudel::SetPlayerManagedArenaData(CBaseDataManager* pBaseDataMgr)
{
	//��ȡ��ҵľ������й����ݣ������������
	int ArenaID = ARENA_ID(pBaseDataMgr->GetSID());
	int rank = GetArenaRank(ArenaID);
	if (rank == -1)
		return false;
	SArenaCompetitor& competitor = m_ArenaManagedData.at(rank);

	competitor.m_AernaID = ArenaID; //������ID
	competitor.m_ArenaRank = rank;
	pBaseDataMgr->GetArenaDefenseTeamInfo(competitor.m_HeroList);
	for (int i = 0; i < 5; ++i)
	{
		pBaseDataMgr->GetHeroFateAttribute(competitor.m_HeroList[i].m_dwHeroID, competitor.m_FateAttr[i].fateAttr, 4);
	}
	competitor.m_TeamLevel = pBaseDataMgr->GetMasterLevel();
	if (competitor.m_curOpponentID != -1)//��������������
	{
		rfalse("���%d����", pBaseDataMgr->GetSID());
		UnlockCompetitor(competitor.m_curOpponentID); 
	}
	competitor.m_curOpponentID = -1;
	//��������
	strcpy_s(competitor.m_CompetitorName,pBaseDataMgr->GetName().c_str());
	
	CPlayer * pPlayer = (CPlayer *)GetPlayerBySID(pBaseDataMgr->GetSID())->DynamicCast(IID_PLAYER);
	if (pPlayer == nullptr)
		return false;
	memset(competitor.m_FactionName, 0, sizeof(char) * 32);
	lite::Variant ret;//��lua��ȡ����ֵ
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "SI_vip_getlv")[pBaseDataMgr->GetSID()](&ret);
	g_Script.CleanCondition();
	if (ret.dataType == LUA_TNIL)
		return false;
	competitor.m_VipLevel = (int)ret; //��ȡVIP

	lite::Variant ret2;//��lua��ȡ����ֵ
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "GetPlayerFactionName")[pBaseDataMgr->GetSID()](&ret2);
	g_Script.CleanCondition();
	LPCSTR Txt = (LPCSTR)ret2;
	if (Txt == nullptr)
	{
		rfalse("���߾�������ȡ������Ϊ��");
		return  false;
	}
	strcpy_s(competitor.m_FactionName, Txt);
	return true;

}


///@brief ��ȡ������Ҿ���������
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
	lite::Variant ret;//��lua��ȡ����ֵ
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "SI_vip_getlv")[baseDataMgr->GetSID()](&ret);
	g_Script.CleanCondition();
	competitor.m_VipLevel = (int)ret; //��ȡVIP

	lite::Variant ret2;//��lua��ȡ����ֵ
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "GetPlayerFactionName")[baseDataMgr->GetSID()](&ret2);
	g_Script.CleanCondition();
	LPCSTR Txt = (LPCSTR)ret2;
	if (Txt == nullptr)
		rfalse("�����������ʼ���ȡVIP�ȼ�Ϊ��1");
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
		//rfalse("��ѯ%d��ҵľ���������ʧ��", arenaID);
		return -1;
	}
	return m_ArenaIDRankMapping.at(arenaID);
}


///@brief ����ǰ50���а�
bool CArenaMoudel::UpdateTop50th()
{
	if (m_ArenaIDRankMapping.size() < 50)
	{
		rfalse("���������а�δ��ʼ��");
		return false;
	}
	rfalse("���������п�ʼ����");
	map<int, SArenaCompetitor>* pManagedData = &m_ArenaManagedData;
	SArenaPlayerReducedData* Top50th = m_pTop50th;
	FunctionPtr &UpdtateTop50thFucn = m_UpdtateTop50thFucn;
	int count = 0;

	for (auto &iter : m_ArenaManagedData)
	{
		SArenaPlayerReducedData  &tempReducedData = Top50th[count];
		 SBaseArenaCompetitor & tempComp = iter.second;
		CPlayer* playerPtr = (CPlayer*)GetPlayerBySID(GetSIDFromArenaID(tempComp.m_AernaID))->DynamicCast(IID_PLAYER);
		if (playerPtr != nullptr)//�����������ң��������������ݸ��¾�����������Ϣ
			_GetOnlineArenaData(playerPtr, tempComp);

		memset(&tempReducedData.m_CompetitorName, 0, sizeof(char) * 32);
		strcpy(tempReducedData.m_CompetitorName, tempComp.m_CompetitorName);
		tempReducedData.m_CompetitorIconID = tempComp.m_CompetitorIcon;
		tempReducedData.m_TeamLevel = tempComp.m_TeamLevel; //�ȼ���ֵ
		tempReducedData.m_ArenaID = tempComp.m_AernaID; //������ID
		tempReducedData.m_Rank = tempComp.m_ArenaRank; //����������
		//�򻯰�ս��������
		tempReducedData.m_FightingCapacity = 0;
		for (int i = 0; i < 5; i++)
		{
			tempReducedData.m_FightingCapacity += CHeroAttributeCalculator::getSingleton()->CalculateSpecifyHeroComabtPower(&tempComp.m_HeroList[i], tempComp.m_FateAttr[i].fateAttr);
		}
		++count;
		if (count >= 50)
			break;
	}
	rfalse("����������TOP50�б�");
	return true;
}

//ʱ���������ȡ��ս�ߵĹ������ݣ���������ս��� �� 
void CArenaMoudel::_Timeout(DWORD playerSID)
{
	SArenaCompetitor * managedData = _GetManagedArenaData(ARENA_ID(playerSID));
	if (nullptr == managedData)
		return;

	if (UnlockCompetitor(managedData->m_curOpponentID) == nullptr) //��������
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
		if (ptargetPlayer == nullptr) //��������������
		{
			if (_GetManagedArenaData(detailMsg->m_ArenaID) != nullptr)
				msg.m_BaseArenaData = *(SBaseArenaCompetitor*)_GetManagedArenaData(detailMsg->m_ArenaID);
		}
		else//���������
		{
			_GetOnlineArenaData(ptargetPlayer, msg.m_BaseArenaData);
		}
		//TODO ����ʧ��
		g_StoreMessage(dnidClient, &msg, sizeof(msg));
	}
	catch (const std::out_of_range& oor)
	{
		rfalse("Out of Range error: %s, ��ȡ���а������Ϣ�������SID��%d", oor.what(), pPlayer->GetSID());
	}

}

//�˺������Ƶݹ飬ֻ��������ִ�����첽ִ�У����Ժ������ᵼ��ջ�����
void CArenaMoudel::ArenaRoutine()
{
	//���þ�������ս������lambda
	rfalse("\n\n������ģ���ճ�����===================");
	//��ʱ��
	INT64 now, twelveOclock;
	_time64(&now);
	rfalse("����ʱ��%d", now);
	rfalse("�ϴ�ִ���ճ�ʱ��", m_lastRoutineTime);
	rfalse("ʱ���%d", now - m_lastRoutineTime);
	//�ж��Ƿ��ڽ����Ѿ�ִ�й��������ճ�����
	tm lastRoutineDay;
	tm curRoutineDay;
	errno_t tempLastErr = _localtime64_s(&lastRoutineDay, &m_lastRoutineTime);
	errno_t tempCurtErr = _localtime64_s(&curRoutineDay, &now);
	if (tempLastErr || tempCurtErr) //������
		return;
	int result = CompareTimeOfYear(curRoutineDay, lastRoutineDay); //�ж��ϴ��ճ�ʱ�������ʱ���Ƿ�Ϊͬһ�죬������򷵻�
	if (result < 1)
		return;
	m_lastRoutineTime = now + 10; //�����ճ�ʱ���¼


	map<int, SArenaCompetitor> * pArenaManagedData = &m_ArenaManagedData;
	FunctionPtr ResetChallengeCount = [pArenaManagedData](int a)	
	{
		rfalse("������ģ��������ս����");
		//for_each(pArenaManagedData->begin(), pArenaManagedData->end(), [&](pair<int, SArenaCompetitor> arenaPair){ arenaPair.second.m_ChallengeCount = 5; });
		for (auto &Iter : *pArenaManagedData)
		{
			Iter.second.m_ChallengeCount = 5;

			//������ģ�����ù�����������ô���
			Iter.second.m_BuyedNum = 0;
			Iter.second.m_ResetNum = 0;
		}
	};

	//TODO �ʼ�����`
	map<int, int> *pIDandRankMapping = &m_ArenaIDRankMapping;
	FunctionPtr SetArenaRewardEmail = [pIDandRankMapping](int a)
	{
		///5������ һ������Ҫ�滻��ֵ  ����Ҫ�滻�� ��0ռλ
		/*lite::Variant ret2;
		LuaFunctor(g_Script, "Com_GetTipString")[2]["a"]["b"]["c"][0][0](&ret2);
		string Value = (string)ret2;*/

		MailInfo mailInfo;
		int timeRecord = timeGetTime();
		memset(&mailInfo, 0, sizeof(mailInfo));

		lite::Variant ret;//��lua��ȡvip�ȼ�
		LuaFunctor(g_Script, "SI_GetStr")[3](&ret);
		LPCSTR Txt = (LPCSTR)ret;
		if (Txt == nullptr)
			rfalse("�����������ʼ���ȡVIP�ȼ�Ϊ��1");
		
		strcpy_s(mailInfo.senderName, Txt);

		lite::Variant ret1;//
		LuaFunctor(g_Script, "SI_GetStr")[4](&ret1);
		LPCSTR Txt1 = (LPCSTR)ret1;
		if (Txt == nullptr)
			rfalse("�����������ʼ���ȡVIP�ȼ�Ϊ��2");
		strcpy_s(mailInfo.title, Txt1);

		for_each(pIDandRankMapping->begin(), pIDandRankMapping->end(), [&mailInfo](pair<int, int> arenaPair)  //ѭ��������ң������ʼ�����
		{
			ArenaRewardConfig configIter;
			if (arenaPair.first <= MAX_ROBOT_NUM)
				return;
			if (CConfigManager::getSingleton()->GetArenaRewardConfig(arenaPair.second, configIter) == false)
				return;
			if (configIter.Gold > 0)
			{
				mailInfo.szItemInfo[0].itype = GoodsType::money;//ͬ�����
				mailInfo.szItemInfo[0].num = configIter.Gold;
			}

			if (configIter.Gem > 0)
			{
				mailInfo.szItemInfo[1].itype = GoodsType::diamond; //ͬ����ʯ
				mailInfo.szItemInfo[1].num = configIter.Gem;
			}

			if (configIter.Honor > 0)
			{
				mailInfo.szItemInfo[2].itype = GoodsType::honor; //ͬ����ʯ
				mailInfo.szItemInfo[2].num = configIter.Honor;
			}

			if (configIter.MedicineNumber > 0)
			{
				mailInfo.szItemInfo[3].itype = GoodsType::item; //ͬ������ҩ��
				mailInfo.szItemInfo[3].id = configIter.Medicine;
				mailInfo.szItemInfo[3].num = configIter.MedicineNumber;
			}

			if (configIter.RefinedMaterialNumber > 0)
			{
				mailInfo.szItemInfo[4].itype = GoodsType::item; //ͬ������
				mailInfo.szItemInfo[4].id = configIter.RefinedMaterial;
				mailInfo.szItemInfo[4].num = configIter.RefinedMaterialNumber;
			}


			lite::Variant ret2;//��lua��ȡvip�ȼ�
			LuaFunctor(g_Script, "SI_GetStr")[5][arenaPair.second](&ret2);
			LPCSTR Txt2 = (LPCSTR)ret2;
			if (Txt2 == nullptr)
				rfalse("�����������ʼ���ȡVIP�ȼ�Ϊ��3");
			strcpy_s(mailInfo.content, Txt2);
			MailMoudle::getSingleton()->SendMail_SanGuo(arenaPair.first - MAX_ROBOT_NUM, 1440/*1440�����Ժ�ʧЧ*/, &mailInfo);
		});
		timeRecord = timeGetTime() - timeRecord;
		rfalse("�������ַ�������ʱ %d", timeRecord);
	};

	TimerEvent_SG::SetCallback(ResetChallengeCount, 2); //lambda���붨ʱ�� 2S�󴥷�
	TimerEvent_SG::SetCallback(SetArenaRewardEmail, 4); //lambda���붨ʱ�� 4S�󴥷�

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
