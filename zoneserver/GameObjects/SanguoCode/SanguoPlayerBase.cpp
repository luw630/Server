#include "..\stdafx.h"
#include "SanguoPlayerBase.h"
#include"HeroMoudle\HeroDataManager.h"
#include "BaseDataManager.h"
#include "ExtendedDataManager.h"
#include "ExpeditionMoudle\ExpeditionDataManager.h"
#include "ExpeditionMoudle\ExpeditionManager.h"
#include "BlessMoudle\BlessDataMgr.h"
#include "BlessMoudle\BlessMoudle.h"
#include "ExpeditionMoudle\ExpeditionManager.h"
#include "Common\ConfigManager.h"
#include "StorageMoudle\StorageManager.h"
#include "HeroMoudle\HeroUpgradeMoudle.h"
#include "HeroMoudle\HeroExtendsGameplayModel.h"
#include "MailMoudle\MailMoudle.h"
#include "MissionMoudle\CMissionUpdate.h"
#include "MissionMoudle\MissionDataManager.h"
#include "MissionMoudle\MissionManager.h"
#include "MallMoudle\VarietyShopManager.h"
#include "MallMoudle\ArenaShopManager.h"
#include "MallMoudle\ExpeditionShopManager.h"
#include "MallMoudle\MiracleMerchantManager.h"
#include "MallMoudle\SoulExchangeManager.h"
#include "MallMoudle\GemMerchantManager.h"
#include "MallMoudle\LegionShopManager.h"
#include "MallMoudle\WarOfLeagueShopManager.h"
#include "MallMoudle\MallsMoudle.h"
#include "ExchangeMoudle\ExchangeGoldProcess.h"
#include "ExchangeMoudle\ExchangeEndurance.h"
#include "ExchangeMoudle\DiamondExchangeModule.h"
#include "AchievementMoudle\AchievementManager.h"
#include "AchievementMoudle\AchievementDataManager.h"
#include "AchievementMoudle\RandomAchieveManager.h"
#include "AchievementMoudle\RandomAchieveDataManager.h"
#include "AchievementMoudle\RandomAchieveUpdate.h"
#include "DuplicateMoudle\DuplicateDataManager.h"
#include "DuplicateMoudle\DuplicateBaseManager.h"
#include "DuplicateMoudle\EliteDuplicateManager.h"
#include "DuplicateMoudle\StoryDulicateManager.h"
#include "DuplicateMoudle\LeagueOfLegendDuplicateMananger.h"
#include "DuplicateMoudle\TreasureHuntingDuplicateManager.h"
#include "DessertMoudle\DessertSystem.h"
#include "ForgingMoudle\ForgingManager.h"
#include "ArenaMoudel\ArenaMoudel.h"
#include "ArenaMoudel\ArenaDataMgr.h"
#include "ScriptManager.h"
#include "SkillUpgradeMoudle\SkillUpgradeManager.h"
#include "SkillUpgradeMoudle\SkillUpgradeMoudle.h"
#include "LuckyTimeMoudle\ChaseThiefDataManager.h"
#include "LuckyTimeMoudle\ChaseThiefTollgateManager.h"
#include "LuckyTimeMoudle\ChaseThiefMoudle.h"
#include"Common\PubTool.h"
#include "../���������/Recharge.h"
#include "../���������/GameObjects/FactionManager.h"
extern SIZE_T GetMemoryInfo();

CSanguoPlayer::CSanguoPlayer()
	:m_pPlayerData(nullptr),
	cPlayer(nullptr),
	m_ClientIndex(0),
	m_GMFlag(false)
{
	m_DataMgrList.clear();
	m_MallMgrList.clear();
	m_duplicateMgrs.clear();
}

CSanguoPlayer::~CSanguoPlayer()
{
	Release();
}

int CSanguoPlayer::OnCreate(_W64 long pParameter)
{
	rfalse(2, 1, "CSanguoPlayer::OnCreate");
	return 1;
}

void CSanguoPlayer::OnRecvSanguoPlayerRequest(SSGPlayerMsg* pMsg)
{
	switch (pMsg->_protocol)
	{
	case SSGPlayerMsg::EPRO_SANGUO_BLESS://��
		BlessMoudle::getSingleton()->DispatchBlessMsg((CBlessDataMgr*)(m_DataMgrList[Bless_Mgr].get()), (SBlessMsg*)pMsg);
		break;
	case SSGPlayerMsg::EPRO_SANGUO_HEROUPGRADE: //�佫����
		CHeroUpgradeMoudle::getSingleton()->DispatchHeroUpgradeMsg((SBaseHeroUpgrade*)pMsg, m_upBaseDataMgr.get());
		break;
	case SSGPlayerMsg::EPRO_EXPEDITION_MESSAGE: //Զ����Ϣ
		CExpeditionManager::getSingleton()->DispatchExpeditionMsg(pMsg, m_ExpeditionDataMgr.get());
		break;
	case SSGPlayerMsg::EPRO_MISSION_MESSAGE:
		if (m_MissionMgr != nullptr)
			m_MissionMgr->RecvMsg(pMsg);
		break;
	case SSGPlayerMsg::EPRO_ACHIEVEMENT_MESSAGE:
		m_AchievementMgr->RecvMsg(pMsg);
		break;
	case  SSGPlayerMsg::EPRO_SANGUO_FORGING:
		CForgingManager::getSingleton()->OnRecvMsg(pMsg, m_upBaseDataMgr.get());
		break;
	case SSGPlayerMsg::EPRO_SANGUO_DUPLICATE:
		{
			const SDuplicateTypeMsg * ptrMsg = static_cast<const SDuplicateTypeMsg*>(pMsg);
			if (ptrMsg == nullptr)
				break;

			auto findResult = m_duplicateMgrs.find((InstanceType)ptrMsg->duplicateType);
			if (findResult != m_duplicateMgrs.end())
			{
				findResult->second->OnRecvMsg(ptrMsg);
			}
		}
		break;
	case SSGPlayerMsg::EPRO_SANGUO_DIAMONDEXCHANGE:
		if (m_DiamondExchangeModule != nullptr)
			m_DiamondExchangeModule->RecvMsg(pMsg);
		break;
	case SSGPlayerMsg::EPRO_SANGUO_MALL:
		{
			SMallMsgInfo* pMallMsg = static_cast<SMallMsgInfo*>(pMsg);
			MallType type = (MallType)pMallMsg->m_MallType;
			auto iter = m_MallMgrList.find(type);
			if (iter == m_MallMgrList.end())
				return;
			CMallsMoudle::getSingleton()->DispatchMallMsg(iter->second.get(), pMallMsg);
		}
		break;
	case  SSGPlayerMsg::EPRO_SANGUO_CHECKIN:
		{
			SQCHECKIN_Quest* pckMsg = static_cast<SQCHECKIN_Quest*>(pMsg);
			g_Script.SetCondition(0, cPlayer, 0);
			LuaFunctor(g_Script, "checkin_quest")[m_pPlayerData->m_dwStaticID][pckMsg->itype]();
			g_Script.CleanCondition();
		}
		break;
	case SSGPlayerMsg::EPRO_SANGUO_SKILLUPGRADE:
		CSkillUpgradeMoudle::getSingleton()->DispatchSkillUpgradeEvent((SSkillUpgrade*)pMsg, m_SkillUpgradeMgr.get());
		break;
	case SSGPlayerMsg::EPRO_SANGUO_RANDOMAVHIEVEMNT_MESSAGE:
		if (m_RandomAchievementMgr != nullptr)
			m_RandomAchievementMgr->RecvMsg(pMsg);
		break;
	case SSGPlayerMsg::EPRO_SANGUO_DESSERT_MESSAGE:
		if (m_DessertSystem != nullptr)
			m_DessertSystem->RecvMsg(pMsg);
		break;
	case SSGPlayerMsg::EPRO_SANGUO_CHASETHIEF:
	{
		SChaseThiefMsg* pctMsg = static_cast<SChaseThiefMsg*>(pMsg);
		if (nullptr == pctMsg)
			return;
		CChaseThiefMoudle::getSingleton()->OnRecvive(m_ChaseThiefDataMgr.get(), m_ChaseThiefTollgateMgr.get(), pctMsg);
	}
		break;
	case SSGPlayerMsg::EPRO_SANGUO_CHANGENAME:
	{
		CSanguoPlayer::ChangeGameName((SQChangePlayerName*)pMsg);
	}
		break;
	case SSGPlayerMsg::EPRO_SANGUO_ACTIVITY: //����
	{
		SQGetActReward* pGetRwardMsg = static_cast<SQGetActReward*>(pMsg);
		g_Script.SetCondition(0, cPlayer, 0);
		LuaFunctor(g_Script, "act_get_reward")[m_pPlayerData->m_dwStaticID][pGetRwardMsg->actType][pGetRwardMsg->reawrdIndex]();
		g_Script.CleanCondition();
	}
		break;
	case SSGPlayerMsg::EPRO_SANGUO_NEWGUIDE:
	{
		m_upBaseDataMgr->OnRecvGuideOperation(pMsg);
	}
		break;
	case SSGPlayerMsg::EPRO_SANGUO_RECHARGE: //��ֵ
	{
		CRecharge::getSingleton()->RecvRechargeMsg(cPlayer, pMsg);
	}
		break;
	case SSGPlayerMsg::EPRO_SANGUO_HEROEXTENDSGAMEPLAY:
		if (m_HeroExtendsGameplayMgr != nullptr)
			m_HeroExtendsGameplayMgr->OnRecvMsg(pMsg);
		break;
	case SSGPlayerMsg::EPRO_SANGUO_fIRSTRECHARGE:
	{
		g_Script.SetCondition(0, cPlayer, 0);
		LuaFunctor(g_Script, "f_recharge_award")[m_pPlayerData->m_dwStaticID]();
		g_Script.CleanCondition();
	}
	break;
	case SSGPlayerMsg::EPRO_SANGUO_WeekReMoney:
	{
		SQWeekRemoney *msg = static_cast<SQWeekRemoney*>(pMsg);
		if (msg->day == 0){
			g_Script.SetCondition(0, cPlayer, 0);
			LuaFunctor(g_Script, "BuyReMoneyAct")[m_pPlayerData->m_dwStaticID]();
			g_Script.CleanCondition();
		}
		else
		{	
			g_Script.SetCondition(0, cPlayer, 0);
			LuaFunctor(g_Script, "OnTriggerWeekTreasure")[m_pPlayerData->m_dwStaticID][msg->day]();
			g_Script.CleanCondition();
		}

	}
	break;
	case SSGPlayerMsg::EPRO_SANGUO_War://��ս
	{
		SWarMsg *msg = static_cast<SWarMsg*>(pMsg);
		switch (msg->_protocol)
		{
		case SWarMsg::war_join:
			{
				SQWarJoin *warMsg = static_cast<SQWarJoin*>(msg);
				g_Script.SetCondition(0, cPlayer, 0);
				LuaFunctor(g_Script, "war_join")[m_pPlayerData->m_dwStaticID][warMsg->dwHeroId[0]][warMsg->dwHeroId[1]][warMsg->dwHeroId[2]][warMsg->dwHeroId[3]][warMsg->dwHeroId[4]]();
				g_Script.CleanCondition();
			}
			break;
		case SWarMsg::war_teamList:
			{
				SQwar_teamList *warMsg = static_cast<SQwar_teamList*>(msg);
				g_Script.SetCondition(0, cPlayer, 0);
				LuaFunctor(g_Script, "war_get_teamList")[warMsg->teamNum][warMsg->beginNum]();
				g_Script.CleanCondition();
			}
			break;
		case SWarMsg::war_fight_res:
			{
				SQwar_fight_res *warMsg = static_cast<SQwar_fight_res*>(msg);
				g_Script.SetCondition(0, cPlayer, 0);
				//LuaFunctor(g_Script, "war_fight_res")[m_pPlayerData->m_dwStaticID][warMsg->res]();
				if (g_Script.PrepareFunction("war_fight_res"))
				{
					g_Script.PushParameter(m_pPlayerData->m_dwStaticID);
					g_Script.PushParameter(warMsg->res);
					std::list<DWORD> tempTable;
					for (int i = 0; i < 3 * MAX_War_HERONUM;++i)
					{
						tempTable.push_back(warMsg->heroInfo[i]);
					}
					g_Script.PushDWORDArray(tempTable);
					g_Script.Execute();
				}
				g_Script.CleanCondition();
			}
			break;
		case SWarMsg::war_exit:
			{
				SQwar_exit *warMsg = static_cast<SQwar_exit*>(msg);
				g_Script.SetCondition(0, cPlayer, 0);
				LuaFunctor(g_Script, "war_exit")[m_pPlayerData->m_dwStaticID]();
				g_Script.CleanCondition();
			}
			break;
		case SWarMsg::war_isjoin:
		{
			
			g_Script.SetCondition(0, cPlayer, 0);
			LuaFunctor(g_Script, "war_isJoin")[m_pPlayerData->m_dwStaticID][1]();
			g_Script.CleanCondition();
		}
		break;
		case SWarMsg::war_rank:
		{
			g_Script.SetCondition(0, cPlayer, 0);
			LuaFunctor(g_Script, "war_getrank")[m_pPlayerData->m_dwStaticID]();
			g_Script.CleanCondition();
		}
		break;
		default:
			break;
		}

	}
	break;
	case SSGPlayerMsg::EPRO_SANGUO_Rank:
	{
		SRankMsg *msg = static_cast<SRankMsg*>(pMsg);
		if (msg->_protocol == SRankMsg::rank_get){
			SQRank *rankMsg = static_cast<SQRank*>(msg);
			g_Script.SetCondition(0, cPlayer, 0);
			LuaFunctor(g_Script, "rank_getList")[rankMsg->itype]();
			g_Script.CleanCondition();
		}
	}
	break;
	case SSGPlayerMsg::EPRO_SANGUO_Excheange://�һ��
	{
		SExcheangeMsg *msg = static_cast<SExcheangeMsg*>(pMsg);
		if (msg->_protocol == SExcheangeMsg::getExchange){
			SQGetExcheange *ExcheangeMsg = static_cast<SQGetExcheange*>(msg);
			g_Script.SetCondition(0, cPlayer, 0);
			LuaFunctor(g_Script, "exchangeACT_getAwards")[m_pPlayerData->m_dwStaticID][ExcheangeMsg->itype][ExcheangeMsg->index][ExcheangeMsg->num]();
			g_Script.CleanCondition();
		}
	}
	break;
	default:
		break;
	}
}


void CSanguoPlayer::ChangeGameName(SQChangePlayerName* pMsg)
{
	CBaseDataManager* baseDataMgr = CSanguoPlayer::GetBaseDataMgr();

	int decmoney = CConfigManager::getSingleton()->globalConfig.ChangeNameConsumption;

	SAChangeName msg;

	lite::Variant ret;//��lua��ȡ��������
	LuaFunctor(g_Script, "GetChangeNameCount")[baseDataMgr->GetSID()](&ret);
	int count = (int)ret;

	//��ȡ����޸ĵĴ���
	int num = CConfigManager::getSingleton()->globalConfig.ChangeNameConsumptionFreeTime;

	//���Ǹ����޸� 

	if (count > num)
	{
		if (baseDataMgr->GetDiamond() < decmoney)
		{
			//��Ҳ��� ��������  �򷵻ط���˴�����Ϣ
			return;
		}
	}

	dwt::stringkey<char[32]> str = pMsg->name;
	char name[32];

	memcpy(name, pMsg->name, sizeof(pMsg->name));
	//�ж������Ƿ����
	if (baseDataMgr->CheckGameName(name, baseDataMgr->GetSID()) == 0)
	{
		//�����в����ûط�
		msg.ChangeNameCount = count;
		msg.falg = false;
		g_StoreMessage(baseDataMgr->GetDNID(), &msg, sizeof(msg));
	}
	
}

void CSanguoPlayer::Db_Back_ChangeGameName(char* name, DWORD SID, CBaseDataManager* baseDataMgr)
{
	
	SAChangeName msg;

	lite::Variant ret;//��lua��ȡ��������
	LuaFunctor(g_Script, "GetChangeNameCount")[SID](&ret);
	int count = (int)ret;


	//��ȡ�����������
	int decmoney = CConfigManager::getSingleton()->globalConfig.ChangeNameConsumption;

	//��ʯ����
	if (decmoney < 0)
		return;

	//��ȡ����޸ĵĴ���
	int num = CConfigManager::getSingleton()->globalConfig.ChangeNameConsumptionFreeTime;

	//���Ǹ����޸� 
	if (count > num)
	{
		if (baseDataMgr->GetDiamond() < decmoney)
		{
			//��Ҳ��� ��������  �򷵻ط���˴�����Ϣ

			return;
		}

		else
		{
			//Ϊ��������µ�����
			std:: string outname = baseDataMgr->GetName();//.c_str();

			baseDataMgr->SetGameName(name);
			CArenaMoudel::getSingleton()->ChangeNametoArena(baseDataMgr->GetSID(), name);

			//�����Ա������Ϣ
			//CFactionManager::getSingleton()->UpdateMemberMsg(cPlayer, outname.data());
			
			//�۳����� 
			baseDataMgr->DecGoods_SG(GoodsType::diamond, 0, decmoney,GoodsWay::changename);

		}
	}
	else
	{
		//��������޸�
		std::string outname = baseDataMgr->GetName();//.c_str();
		//Ϊ��������µ�����
		baseDataMgr->SetGameName(name);
		//�����Ա������Ϣ
		//CFactionManager::getSingleton()->UpdateMemberMsg(cPlayer, outname.data());
	}


	//�����������
	lite::Variant ret1;//��lua�����������
	LuaFunctor(g_Script, "ChangeNameCountAdd")[SID](&ret1);
	int result = (int)ret1;
	if (result != 0)
	{
		msg.falg = true;
	}
	else
	{
		msg.falg = false;
	}
	msg.ChangeNameCount = result;

	int a= g_StoreMessage(baseDataMgr->GetDNID(), &msg, sizeof(msg));


}

void CSanguoPlayer::SetCplayerPtr(CPlayer*  _cPlayer)
{
	cPlayer = _cPlayer;
}
void CSanguoPlayer::SetSanguoPlayerData(SFixData *fixData)
{
	m_pPlayerData = fixData; //���ݸ�ֵ
	m_upBaseDataMgr.reset(new CBaseDataManager(m_ClientIndex, m_pPlayerData, cPlayer));
	m_upBaseDataMgr->Init(m_GMFlag);
	if (true == m_pPlayerData->m_bNewPlayer)
		m_pPlayerData->m_PlayerIconID = 108; //�������ң����ͷ������Ϊ108���ͷ��


	//��ʼ���ӻ���//
	unique_ptr<CMallManager> varietyShopMgr(new CVarietyShopManager(*m_upBaseDataMgr));
	varietyShopMgr->InitDataMgr(&m_pPlayerData->m_VarietyShopData);
	m_MallMgrList.insert(make_pair(MallType::VarietyShop, move(varietyShopMgr)));

	//��ʼ�������̵�//
	unique_ptr<CMallManager> soulExchangeShopMgr(new CSoulExchangeManager(*m_upBaseDataMgr));
	soulExchangeShopMgr->InitDataMgr(&m_pPlayerData->m_SoulExchangeData);
	m_MallMgrList.insert(make_pair(MallType::SoulExchange, move(soulExchangeShopMgr)));

	//��ʼ���������̵�//
	unique_ptr<CMallManager> arenaShopMgr(new CArenaShopManager(*m_upBaseDataMgr));
	arenaShopMgr->InitDataMgr(&m_pPlayerData->m_ArenaShopData);
	m_MallMgrList.insert(make_pair(MallType::ArenaShop, move(arenaShopMgr)));


	//��ʼ��Զ���̵�//
	unique_ptr<CMallManager> expeditionShopMgr(new CExpeditionShopManager(*m_upBaseDataMgr));
	expeditionShopMgr->InitDataMgr(&m_pPlayerData->m_ExpeditionShopData);
	m_MallMgrList.insert(make_pair(MallType::ExpeditionShop, move(expeditionShopMgr)));


	//��ʼ����Ե�̵�//
	unique_ptr<CMallManager> miracleMerchantMgr(new MiracleMerchantManager(*m_upBaseDataMgr));
	miracleMerchantMgr->InitDataMgr(&m_pPlayerData->m_MiracleMerchantData);
	m_MallMgrList.insert(make_pair(MallType::MiracleMerchant, move(miracleMerchantMgr)));

	//��ʼ���䱦����//
	unique_ptr<CMallManager> gemMerchantMgr(new CGemMerchantManager(*m_upBaseDataMgr));
	gemMerchantMgr->InitDataMgr(&m_pPlayerData->m_GemMerchantData);
	m_MallMgrList.insert(make_pair(MallType::GemMerchant, move(gemMerchantMgr)));

	//��ʼ�������̵�//
	unique_ptr<CMallManager> legionShopMgr(new CLegionShopManager(*m_upBaseDataMgr));
	legionShopMgr->InitDataMgr(&m_pPlayerData->m_LegionShopData);
	m_MallMgrList.insert(make_pair(MallType::LegionShop, move(legionShopMgr)));

	//��ʼ����ս�̵�//
	unique_ptr<CMallManager> warOfLeagueShopMgr(new CWarOfLeagueShopManager(*m_upBaseDataMgr));
	warOfLeagueShopMgr->InitDataMgr(&m_pPlayerData->m_WarOfLeagueShopData);
	m_MallMgrList.insert(make_pair(MallType::WarOfLeagueShop, move(warOfLeagueShopMgr)));

	BlessMoudle *blessMoudle= BlessMoudle::getSingleton();
	//���������ݹ��� �Լ���ʼ�������ݹ���
	unique_ptr<CBlessDataMgr> newBlessMgr(new CBlessDataMgr(*m_upBaseDataMgr));
	newBlessMgr->InitDataMgr(&m_pPlayerData->m_BlessData);

	///���³�ʼ������ͱ��������е����������һ����ֻ����CBaseDataManagerһ���ֽӿڣ�����������Ƿ�CBaseDataManager�Ľӿڷ��뵽�����������У�Ȼ��ϵͳ�������߼�ģ��ֱ�������Щ������ @Jonson
	//////////////////////////////////////////////////////////////////////////
	m_DessertSystem.reset(new CDessertSystem(*m_upBaseDataMgr));
	///��ʼ��Զ�������ݹ���
	m_ExpeditionDataMgr.reset(new CExpeditionDataManager(*m_upBaseDataMgr));
	m_ExpeditionDataMgr->InitDataMgr(&m_pPlayerData->m_ExpeditionData);
	///��ʼ����������ݹ���
	m_MissionDataMgr.reset(new CMissionDataManager(*m_upBaseDataMgr));
	m_MissionDataMgr->InitDataMgr(nullptr);
	m_MissionMgr.reset(new CMissionManager(*((CExtendedDataManager*)m_MissionDataMgr.get())));
	///��ʼ���ɾ͵����ݹ���
	m_AchievementDataMgr.reset(new CAchievementDataManager(*m_upBaseDataMgr));
	m_AchievementDataMgr->InitDataMgr(&m_pPlayerData->m_AchievementData);
	///��ʼ���ɾ͵��߼�����
	m_AchievementMgr.reset(new CAchievementManager(*((CExtendedDataManager*)m_AchievementDataMgr.get())));
	m_RandomAchievementDataMgr.reset(new CRandomAchieveDataManager(*m_upBaseDataMgr));
	m_RandomAchievementDataMgr->InitDataMgr(m_pPlayerData);
	m_RandomAchievementMgr.reset(new CRandomAchieveManager( *((CExtendedDataManager*)m_RandomAchievementDataMgr.get())) );
	///����������ݵĳɾ���ص��߼�
	///��ʱ������ª�������ݹ������������߼�����߼���TODO���ع�@Jonson
	//////////////////////////////////////////////////////////////////////////
	m_upBaseDataMgr->SetAchievementUpdator(m_AchievementMgr.get());
	m_upBaseDataMgr->SetRandomAchievementUpdator(m_RandomAchievementMgr.get());
	m_upBaseDataMgr->SetMissionUpdator(m_MissionMgr.get());
	//////////////////////////////////////////////////////////////////////////
	///��ʼ���ؾ�Ѱ��������˵����ͨ��������Ӣ���������ݹ���
	m_DuplicateDataMgr.reset(new CDuplicateDataManager(*m_upBaseDataMgr));
	m_DuplicateDataMgr->InitDataMgr(m_pPlayerData);
	shared_ptr<CStoryDulicateManager> m_storyDuplicateMgr = make_shared<CStoryDulicateManager>(*((CExtendedDataManager*)m_DuplicateDataMgr.get()));
	m_storyDuplicateMgr->InitMgr();
	m_storyDuplicateMgr->ActiveTo(m_DuplicateDataMgr->GetLatestNormalTollgateID());
	m_duplicateMgrs.insert(make_pair(InstanceType::Story, m_storyDuplicateMgr));
	shared_ptr<CEliteDuplicateManager> m_eliteDuplicateMgr = make_shared<CEliteDuplicateManager>(*((CExtendedDataManager*)m_DuplicateDataMgr.get()));
	m_eliteDuplicateMgr->InitMgr();
	m_eliteDuplicateMgr->ActiveTo(m_DuplicateDataMgr->GetLatestEliteTollgateID());
	m_duplicateMgrs.insert(make_pair(InstanceType::StoryElite, m_eliteDuplicateMgr));
	shared_ptr<CDuplicateBaseManager> m_LOLDuplicateMgr = make_shared<CLeagueOfLegendDuplicateMananger>(*((CExtendedDataManager*)m_DuplicateDataMgr.get()));
	m_LOLDuplicateMgr->InitMgr();
	m_duplicateMgrs.insert(make_pair(InstanceType::LeagueOfLegends, m_LOLDuplicateMgr));
	shared_ptr<CDuplicateBaseManager> m_TreasureHuntingDuplicateMgr = make_shared<CTreasureHuntingDuplicateManager>(*((CExtendedDataManager*)m_DuplicateDataMgr.get()));
	m_TreasureHuntingDuplicateMgr->InitMgr();
	m_duplicateMgrs.insert(make_pair(InstanceType::TreasureHunting, m_TreasureHuntingDuplicateMgr));
	///��ʼ����ʯ�һ����������ģ��
	m_DiamondExchangeModule.reset(new CDiamondExchangeModule());
	///��ʼ����ʯ����Ǯ���߼����ݹ���
	std::shared_ptr<CDiamondExchangeProcess> m_ExchangeGoldProcess = make_shared<CExchangeGoldProcess>(*m_upBaseDataMgr);
	m_ExchangeGoldProcess->Init(&m_pPlayerData->m_dwGoldExhcangedCount);
	m_DiamondExchangeModule->AddProcesser(SDiamondExchangeMsg::SANGUO_EXCHANGEGOLD, m_ExchangeGoldProcess);
	///��ʼ����ʯ�һ��������߼����ݹ���
	std::shared_ptr<CDiamondExchangeProcess> m_ExchangeEnduranceProcess = make_shared<CExchangeEndurance>(*m_upBaseDataMgr);
	m_ExchangeEnduranceProcess->Init(&m_pPlayerData->m_dwEnduranceExchangeCount);
	m_DiamondExchangeModule->AddProcesser(SDiamondExchangeMsg::SANGUO_EXCHANGEENDURANCE, m_ExchangeEnduranceProcess);
	//////////////////////////////////////////////////////////////////////////

	m_HeroExtendsGameplayMgr.reset(new CHeroExtendsGameplayModel(*m_upBaseDataMgr));
	//��������ʼ��
	//CArenaMoudel::getSingleton()->SynLoginArenaData();

	//�����������ݳ�ʼ��
	m_SkillUpgradeMgr.reset(new CSkillUpgradeManager(*m_upBaseDataMgr));
	m_SkillUpgradeMgr->InitDataMgr(&m_pPlayerData->m_SkillPointData);

	//׷���������ݳ�ʼ��
	m_ChaseThiefDataMgr.reset(new CChaseThiefDataManager(*m_upBaseDataMgr));
	m_ChaseThiefDataMgr->InitDataMgr(&m_pPlayerData->m_ChaseThiefData);
	/// �󶨼���׷��������������ͨ�ؿ������¼�
	m_storyDuplicateMgr->BindClearingEvent(m_ChaseThiefDataMgr->GetActivateFunc());
	/// �󶨿����䱦���˹�������ͨ�ؿ������¼�
	m_storyDuplicateMgr->BindClearingEvent(m_MallMgrList[MallType::GemMerchant]->GetOpenFunc());
	/// �󶨼���׷��������������Ӣ�ؿ������¼�
	m_eliteDuplicateMgr->BindClearingEvent(m_ChaseThiefDataMgr->GetActivateFunc());
	/// �󶨿�����Ե���˹�������Ӣ�ؿ������¼�
	m_eliteDuplicateMgr->BindClearingEvent(m_MallMgrList[MallType::MiracleMerchant]->GetOpenFunc());

	//׷�������ؿ���ʼ��
	m_ChaseThiefTollgateMgr.reset(new CChaseThiefTollgateManager());
	m_ChaseThiefTollgateMgr->Init();

	m_DataMgrList.push_back(move(newBlessMgr));
	if (true == m_GMFlag && true == m_pPlayerData->m_bNewPlayer)
		InitGMData(); //��ʼ��GM����
	
	//���;������ϴ���սʱ��
	CArenaMoudel::getSingleton()->SendLastChallengTime(m_upBaseDataMgr->GetDNID(), m_upBaseDataMgr->GetSID());


}

void CSanguoPlayer::FristLogin()
{
	//��Ҫ���ݵ�¼ʱ�� �����ݽ���ˢ�²������߼�
	// 5��ˢ������
	INT64 now;
	_time64(&now);
	tm targetTime;
	_localtime64_s(&targetTime, &now);
	targetTime.tm_hour = 5;
	targetTime.tm_min = 0;
	targetTime.tm_sec = 0;
	if (RefreshJudgement::JudgeCrossed(m_upBaseDataMgr->GetLogoutTime(), targetTime))
	{
		//���ýű�ˢ�»ص�
		if (g_Script.PrepareFunction("OnLoginRefresh"))
		{
			g_Script.PushParameter(m_upBaseDataMgr->GetSID());
			g_Script.PushParameter(5);
			g_Script.Execute();
		}
	}
}

bool CSanguoPlayer::Release()
{
	m_pPlayerData = nullptr;
	cPlayer = nullptr;
	return true;
}

void CSanguoPlayer::InitGMData()
{
	rfalse("����gm�˺�");

	std::map<int32_t, HeroConfig>  heorConfigMap = CConfigManager::getSingleton()->GetHeroConfigMap();
	for (auto heroConfig : heorConfigMap)
		m_upBaseDataMgr->AddHero(heroConfig.first);
	m_upBaseDataMgr->PlusDiamond(10000000);
	m_upBaseDataMgr->PlusMoney(10000000);
	m_upBaseDataMgr->PlusHonor(10000000);
	m_upBaseDataMgr->PlusExploit(10000000);
	m_upBaseDataMgr->PlusPrestige(10000000);
	
	vector<int32_t> equipIDList;
	CConfigManager::getSingleton()->GetEquipIDList(equipIDList);
	CStorageManager& storageMgr = m_upBaseDataMgr->GetStorageManager();
	for each (int32_t var in equipIDList)
		storageMgr.AddItem(var, 99);

	//��Ӧ�Դ���, GM����⹦��
	if (!storageMgr.ItemExistOrEnough(20002))///��֥
		storageMgr.AddItem(20002, 1000);
	if (!storageMgr.ItemExistOrEnough(20010))///ɨ����
		storageMgr.AddItem(20010, 1000);
	if (!storageMgr.ItemExistOrEnough(20011))///����ҩ��
		storageMgr.AddItem(20011, 1000);
	if (!storageMgr.ItemExistOrEnough(20012))///���ر���
		storageMgr.AddItem(20012, 1000);
	if (!storageMgr.ItemExistOrEnough(20013))///��װ����
		storageMgr.AddItem(20013, 1000);
	if (!storageMgr.ItemExistOrEnough(20014))///��װ����
		storageMgr.AddItem(20014, 1000);
	if (!storageMgr.ItemExistOrEnough(20015))///��װ����
		storageMgr.AddItem(20015, 1000);
	if (!storageMgr.ItemExistOrEnough(20016))///��԰���屦��
		storageMgr.AddItem(20016, 1000);


	//GMֱ�ӽ���������
	CArenaMoudel::getSingleton()->DebutArena(m_upBaseDataMgr->GetDNID(), m_upBaseDataMgr.get());
	
	///GM��û����������
	if (m_pPlayerData != nullptr)
		m_pPlayerData->m_dwStoryPlayed = 1;
}

void CSanguoPlayer::OnRecvStorageOperation(SSGPlayerMsg* pMsg)
{
	m_upBaseDataMgr->OnRecvStorageOperation(pMsg);
}
CBaseDataManager* CSanguoPlayer::GetSanguoBaseData()
{
	return m_upBaseDataMgr.get();
}

void CSanguoPlayer::OnRun()
{

}

CBaseDataManager* CSanguoPlayer::GetBaseDataMgr()
{
	return m_upBaseDataMgr.get();
}

CExtendedDataManager* CSanguoPlayer::GetDataMgr(DataMgrType type)
{
	return m_DataMgrList[type].get();
}

bool CSanguoPlayer::SanguoLogout()
{
	CArenaMoudel::getSingleton()->SetPlayerManagedArenaData(m_upBaseDataMgr.get());//��ҵǳ� ���¾������й�����
	if (m_RandomAchievementDataMgr != nullptr)
		m_RandomAchievementDataMgr->LogoutProcess();
	m_bValid = false;
	return true;
}

void CSanguoPlayer::FiveOclockRefreshEvent()
{
	///��ˢ�º����ͻ���ͬ��ˢ����Ϣ
	if (m_ExpeditionDataMgr != nullptr)
		m_ExpeditionDataMgr->RefreshTicket(1);

	if (m_MissionDataMgr != nullptr)
	{
		m_MissionDataMgr->ResetMissionData(1);
		m_MissionDataMgr->LunchMissionVisible();
	}

	if (m_RandomAchievementDataMgr != nullptr)
	{
		m_RandomAchievementDataMgr->RefreshData(1);
	}

	if (m_DuplicateDataMgr != nullptr)
		m_DuplicateDataMgr->RefreshData(1);

	if (m_DiamondExchangeModule != nullptr)
		m_DiamondExchangeModule->RefreshData();

	//if (m_upBaseDataMgr != nullptr)
	//	m_upBaseDataMgr->ResetDataForNewDay();

	if (m_DessertSystem != nullptr)
		m_DessertSystem->ResetData();

	///���濪ʼ�����˵�������߼�
	//////////////////////////////////////////////////////////////////////////
	///GM��Ĭ�ϸ���ȫ����
	if (m_GMFlag == false)
	{
		auto findResut = m_duplicateMgrs.find(InstanceType::LeagueOfLegends);
		if (findResut == m_duplicateMgrs.end())
			return;

		shared_ptr<CLeagueOfLegendDuplicateMananger> m_LOLDuplicateMgr = dynamic_pointer_cast<CLeagueOfLegendDuplicateMananger>(findResut->second);
		if (m_LOLDuplicateMgr == nullptr)
			return;

		m_LOLDuplicateMgr->JudgeDuplicateAcitiveState();
	}
	//////////////////////////////////////////////////////////////////////////
	/// �����̵�ˢ�´���
	for (auto &iter : m_MallMgrList)
	{
		iter.second->ResetRefreshedCount();
	}
}

void CSanguoPlayer::TwelveOclockRefreshEvent()
{
	///��ˢ�º����ͻ���ͬ��ˢ����Ϣ
	if (m_MissionDataMgr != nullptr)
	{
		m_MissionDataMgr->LunchMissionAccessible();
	}
}

void CSanguoPlayer::FourteenOclockRefreshEvent()
{
	///��ˢ�º����ͻ���ͬ��ˢ����Ϣ
	if (m_MissionDataMgr != nullptr)
	{
		m_MissionDataMgr->DinnerMissionVisible();
	}
}

void CSanguoPlayer::EighteenOclockRefreshEvent()
{
	///��ˢ�º����ͻ���ͬ��ˢ����Ϣ
	if (m_MissionDataMgr != nullptr)
	{
		m_MissionDataMgr->DinnerMissionAccessible();
	}
}

void CSanguoPlayer::TwentyOclockRefreshEvent()
{
	///��ˢ�º����ͻ���ͬ��ˢ����Ϣ
	if (m_MissionDataMgr != nullptr)
	{
		m_MissionDataMgr->SupperMissionVisible();
	}
}

void CSanguoPlayer::TwentyOneOclockRefreshEvent()
{
	///��ˢ�º����ͻ���ͬ��ˢ����Ϣ
	if (m_MissionDataMgr != nullptr)
	{
		m_MissionDataMgr->SupperMissionAccessible();
	}
}

void CSanguoPlayer::TwentyFourOclockRefreshEvent()
{
	///��ˢ�º����ͻ���ͬ��ˢ����Ϣ
	if (m_MissionDataMgr != nullptr)
	{
		m_MissionDataMgr->SupperMissionDisable();
	}
	if (m_upBaseDataMgr != nullptr) //��ѽ�Ǯ����������
	{
		((CBlessDataMgr*)m_DataMgrList[Bless_Mgr].get())->SetMoneyFreeBlessCount(5);
	}

}

void CSanguoPlayer::VarietyShopRefreshEvent()
{
	auto iter = m_MallMgrList.find(MallType::VarietyShop);
	if (iter == m_MallMgrList.end())
		return;

	CMallsMoudle::getSingleton()->SendAutoRefreshMsgToClient(iter->second.get());
}

void CSanguoPlayer::ArenaShopRefreshEvent()
{
	auto iter = m_MallMgrList.find(MallType::ArenaShop);
	if (iter == m_MallMgrList.end())
		return;

	CMallsMoudle::getSingleton()->SendAutoRefreshMsgToClient(iter->second.get());
}

void CSanguoPlayer::ExpeditionShopRefreshEvent()
{
	auto iter = m_MallMgrList.find(MallType::ExpeditionShop);
	if (iter == m_MallMgrList.end())
		return;

	CMallsMoudle::getSingleton()->SendAutoRefreshMsgToClient(iter->second.get());
}

void CSanguoPlayer::LegionShopRefreshEvent()
{
	auto iter = m_MallMgrList.find(MallType::LegionShop);
	if (iter == m_MallMgrList.end())
		return;

	CMallsMoudle::getSingleton()->SendAutoRefreshMsgToClient(iter->second.get());
}

void CSanguoPlayer::MiracleMerchantRefreshEvent()
{
	auto iter = m_MallMgrList.find(MallType::MiracleMerchant);
	if (iter == m_MallMgrList.end())
		return;

	CMallsMoudle::getSingleton()->SendAutoRefreshMsgToClient(iter->second.get());
}

void CSanguoPlayer::GemMerchantRefreshEvent()
{
	auto iter = m_MallMgrList.find(MallType::GemMerchant);
	if (iter == m_MallMgrList.end())
		return;

	CMallsMoudle::getSingleton()->SendAutoRefreshMsgToClient(iter->second.get());
}

void CSanguoPlayer::SoulPointExchangeShopRefreshEvent()
{
	auto iter = m_MallMgrList.find(MallType::SoulExchange);
	if (iter == m_MallMgrList.end())
		return;

	CMallsMoudle::getSingleton()->SendAutoRefreshMsgToClient(iter->second.get());
}

void CSanguoPlayer::WarOfLeagueShopRefreshEvent()
{
	auto iter = m_MallMgrList.find(MallType::WarOfLeagueShop);
	if (iter == m_MallMgrList.end())
		return;

	CMallsMoudle::getSingleton()->SendAutoRefreshMsgToClient(iter->second.get());
}

void CSanguoPlayer::UpdateDataForVIP(int vipLevel)
{
	m_SkillUpgradeMgr->SetSkillPointForVIP(vipLevel);
	auto iter = m_MallMgrList.find(MallType::GemMerchant);
	if (iter != m_MallMgrList.end())
	{
		iter->second->VipLevelChange(vipLevel);
	}

	iter = m_MallMgrList.find(MallType::MiracleMerchant);
	if (iter != m_MallMgrList.end())
	{
		iter->second->VipLevelChange(vipLevel);
	}

	///����VIP�ȼ���ص�����
	if (m_upBaseDataMgr != nullptr)
	{
		CMissionUpdate * missionUpdator = m_upBaseDataMgr->GetMissionUpdator();
		if (missionUpdator != nullptr)
			missionUpdator->UpdateVipMission(vipLevel);
	}
}

void CSanguoPlayer::InitDataOfLogin()
{
	if (nullptr != m_SkillUpgradeMgr)
		m_SkillUpgradeMgr->InitSkillPointData();

	///��ʼ��С��������lua�˵�����
	if (m_DessertSystem != nullptr)
		m_DessertSystem->InitLuaData();
	///��ʼ�������һЩ����
	if (m_MissionDataMgr != nullptr)
		m_MissionDataMgr->OnLuaInitialed();

	/// ˢ�¾����´�ˢ��ʱ��
	for (auto& iter : m_MallMgrList)
	{
		iter.second->SetRefreshInfo(false);
	}

	auto iter = m_MallMgrList.find(MallType::GemMerchant);
	if (iter != m_MallMgrList.end())
	{
		iter->second->LoginProcess();
		iter->second->IsOpened();
	}

	iter = m_MallMgrList.find(MallType::MiracleMerchant);
	if (iter != m_MallMgrList.end())
	{
		iter->second->LoginProcess();
		iter->second->IsOpened();
	}


}

const CDuplicateBaseManager* CSanguoPlayer::GetDuplicatemanager(InstanceType type)
{
	auto findResult = m_duplicateMgrs.find(type);
	if (findResult == m_duplicateMgrs.end())
		return nullptr;

	return findResult->second.get();
}

CDessertSystem* CSanguoPlayer::GetDessertSystem()
{
	return m_DessertSystem.get();
}

void CSanguoPlayer::AddSkillPoint(int num)
{
	if (m_SkillUpgradeMgr != nullptr)
		m_SkillUpgradeMgr->AddSkillPoint(num);
}

