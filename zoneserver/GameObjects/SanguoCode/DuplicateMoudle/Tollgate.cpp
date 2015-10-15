#include "stdafx.h"
#include "Tollgate.h"
#include "TollgateDrop.h"
#include "..\BaseDataManager.h"
#include "DuplicateDataManager.h"
#include "..\Common\ConfigManager.h"
#include "NETWORKMODULE\SanguoPlayer.h"
#include "ScriptManager.h"
CTollgate::CTollgate()
{
	m_iTollgateId = 0;
	m_bCompleted = false;
	m_bActivated = false;
	m_bMopUpOperation = false;
	m_bFirstChallenge = true;
	m_iGeneralsExp = 0;
	m_iTeamExp = 0;
	m_iMoney = 0;
	m_iNextTollgateID = -1;
	m_fCurCD = 0;
	m_iTeamMinimumLevel = 0;
	m_iChanllengedTimes = 0;
	m_iChallengeStaminaCost = 0;
	m_iOneDayChanllengeCount = 0;
	m_curInstanceType = InstanceType::Story;
	m_iCurBattleGrade = 0;
	m_ptrConfig = nullptr;
	m_ptrDuplicateDataManager = nullptr;
	m_ptrDataManager = nullptr;
	m_mapFirstDropItems.clear();
	m_listItem.clear();
	m_ptrDropOut.reset(nullptr);
}


CTollgate::~CTollgate()
{
}

void CTollgate::Init(CDuplicateDataManager* dataManager, const BattleLevelConfig* config, InstanceType type)
{
	if (config == nullptr)
	{
		rfalse("�ؿ���ʼ����ʱ�򴫽����Ĺؿ�������ϢΪ��");
		return;
	}

	if (dataManager == nullptr)
	{
		rfalse("���������ݹ�����Ϊ��");
		return;
	}

	m_curInstanceType = type;
	m_ptrConfig = config;
	m_iTollgateId = config->ID;
	m_ptrDuplicateDataManager = dataManager;
	m_ptrDataManager = &m_ptrDuplicateDataManager->GetBaseDataMgr();
	if (m_ptrDataManager == nullptr)
		rfalse("�ؿ�%d��ʼ����ʱ�򴫽��������ݹ�����Ϊ��", m_iTollgateId);
	m_iGeneralsExp = config->Exp;
	m_iChallengeStaminaCost = m_iTeamExp = config->Cost;
	m_iOneDayChanllengeCount = config->TicketNum;
	m_iTeamMinimumLevel = config->LevelLimit;
	m_fCurCD = 0;
	m_iChanllengedTimes = 0;
	//m_iGloryValue = 0;
	//m_iSalaryValue = 0;
	const BattleDropConfig * dropConfig = CConfigManager::getSingleton()->GetBattleDropConfig(config->ID);
	if (dropConfig == nullptr)
		rfalse("don't find drop config with battleID : %d", config->ID);
	else
		m_iMoney = dropConfig->Money;

	m_ptrDropOut.reset(new CTollgateDrop());
	m_ptrDropOut->Init(CConfigManager::getSingleton()->GetBattleDropConfig(m_iTollgateId));
}

void CTollgate::Update()
{
	m_fCurCD = 0;///<��ʱ����
}

void CTollgate::ActionWhileEnterBattle(int challengeTime /* = 1 */)
{
	if (m_ptrDataManager == nullptr)
		return;
	int32_t GoodsWayType = 0;
	switch (m_curInstanceType)
	{
	case Story:
		GoodsWayType = GoodsWay::duplicate;
		break;
	case StoryElite:
		GoodsWayType = GoodsWay::duplicate;
		break;
	case Expedition:
		GoodsWayType = GoodsWay::duplicate;
		break;
	case Arena:
		GoodsWayType = GoodsWay::practiceForce;
		break;
	case LeagueOfLegends://����˵
	{
		//wk 20150706 ���������־
		g_Script.CallFunc("db_gm_setoperation", m_ptrDataManager->GetSID(), 2, 1, 1);
		GoodsWayType = GoodsWay::heroLegend;
	}
		
		break;
	case TreasureHunting://�ؾ�Ѱ��
	{
		//wk 20150706 ���������־
		g_Script.CallFunc("db_gm_setoperation", m_ptrDataManager->GetSID(), 1, 1, 1);
		GoodsWayType = GoodsWay::findCowry;
	}
		
		break;
	case TeamSige:
		GoodsWayType = GoodsWay::fanctionWar;
		break;
	case ChaseThief:
		GoodsWayType = GoodsWay::mainCityActive;
		break;
	default:
		break;
	}
	///�����ɨ��ģʽ����۳�ȫ������ս���ģ�������ǣ����ȿ۳�����޶ȵ���ս����
	if (m_bMopUpOperation)
		m_ptrDataManager->DecGoods_SG(GoodsType::endurance, 0, m_iChallengeStaminaCost * challengeTime, GoodsWayType);
	else
		m_ptrDataManager->DecGoods_SG(GoodsType::endurance, 0, 1 * challengeTime, GoodsWayType); ///<��͵���ս����������ʱֻ��1��

	///�ؿ����Ƽ�ʱ
	m_fCurCD = 0;///<��ʱ����

	CalculateDropOutItem(challengeTime, m_bMopUpOperation); ///�������
}

void CTollgate::EarningClearing(int challengeTime /* = 1 */)
{
	if (m_ptrDataManager == nullptr || m_ptrDuplicateDataManager == nullptr)
		return;
	int32_t GoodsWayType = 0;
	switch (m_curInstanceType)
	{
	case Story:
		GoodsWayType = GoodsWay::duplicate;
		break;
	case StoryElite:
		GoodsWayType = GoodsWay::duplicate;
		break;
	case Expedition:
		GoodsWayType = GoodsWay::duplicate;
		break;
	case Arena:
		GoodsWayType = GoodsWay::practiceForce;
		break;
	case LeagueOfLegends:
		GoodsWayType = GoodsWay::heroLegend;
		break;
	case TreasureHunting:
		GoodsWayType = GoodsWay::findCowry;
		break;
	case TeamSige:
		GoodsWayType = GoodsWay::fanctionWar;
		break;
	case ChaseThief:
		GoodsWayType = GoodsWay::mainCityActive;
		break;
	default:
		break;
	}
	///��ս�ɹ��Ű�ȫ�����ս���Ŀ���
	if (!m_bMopUpOperation)
		m_ptrDataManager->DecGoods_SG(GoodsType::endurance, 0, (m_iChallengeStaminaCost - 1) * challengeTime, GoodsWayType); ///<��͵���ս���������Ѿ�����1�㣬���Դ˴�-1

	m_iChanllengedTimes += challengeTime;

	///���濪ʼ����������ȫ�����뱳��
	if (!m_bMopUpOperation && challengeTime > 1)
		rfalse("�ؿ������У�����ɨ������������ս��������1�������Ǻ������õ�����");
	m_ptrDataManager->AddGoods_SG(GoodsType::Exp, 0, m_iTeamExp * challengeTime, GoodsWayType);
	m_ptrDataManager->AddGoods_SG(GoodsType::money, 0, m_iMoney * challengeTime, GoodsWayType);

	int itemCount = m_listItem.size();
	int *idList = new int[itemCount];
	int *numList = new int[itemCount];
	int itemIdex = 0;
	for (auto itor : m_listItem)
	{
		idList[itemIdex] = itor.first;
		numList[itemIdex] = itor.second;
		++itemIdex;
	}
	m_ptrDataManager->AddGoods_SG(GoodsType::item, itemCount, numList, idList, GoodsWayType);
	delete[] idList;
	delete[] numList;

	///��ɨ��ģʽ������佫���飬ɨ��ģʽ����Թ̶����侭�������Ϊ����
	if (!m_bMopUpOperation && m_iGeneralsExp != 0)
	{
		m_ptrDuplicateDataManager->SetHeroExpIncreasement(m_iGeneralsExp);
	}
	m_bCompleted = true;
	m_bFirstChallenge = false;
}

void CTollgate::RefreshTicket()
{
	if (m_ptrConfig == nullptr)
	{
		rfalse("�ؿ�%dˢ��Ʊ��ʧ��", m_iTollgateId);
		return;
	}

	m_iOneDayChanllengeCount = m_ptrConfig->TicketNum;
	m_iChanllengedTimes = 0;
}

CheckResult CTollgate::Check(int challengeTime /*= 1*/, bool mopUpOperation /*= false*/)
{
	if (m_ptrDataManager == nullptr)
		return CheckResult::Failed;

	///ɨ������������ؿ���δͨ�أ�����ͨ�ط������ﵽ3���ǣ��Ͳ���ɨ��
	if (mopUpOperation && m_iCurBattleGrade < 3)
	{
		return CheckResult::Failed;
	}

	///������ս�������Ƿ����������ս�˹ؿ�
	m_bMopUpOperation = false;
	if (m_iOneDayChanllengeCount == 0 || (m_iOneDayChanllengeCount > 0 && challengeTime > (m_iOneDayChanllengeCount - m_iChanllengedTimes)))
		return CheckResult::NoEnoughTimes;
	if (m_fCurCD > 0.0f)
		return CheckResult::NoEnoughCD;
	if (m_ptrDataManager->GetPlayerLevel() < m_iTeamMinimumLevel)
		return CheckResult::NoEnoughTeamLevel;

	///�����������Ǯ���ܷ����������ս�˹ؿ�
	if (!m_ptrDataManager->CheckGoods_SG(GoodsType::endurance, 0, m_iChallengeStaminaCost * challengeTime))
		return CheckResult::NoEnoughStamina;

	m_bMopUpOperation = mopUpOperation;

	return CheckResult::Pass;
}

void CTollgate::Conquer()
{
	m_bCompleted = true;
	m_bActivated = true;
}

void CTollgate::Activate()
{
	m_bActivated = true;
	switch (m_curInstanceType)
	{
	case InstanceType::Story:
		if (m_ptrDuplicateDataManager != nullptr)
			m_ptrDuplicateDataManager->UpdateLatestNormalTollgateID(m_iTollgateId);
		break;
	case InstanceType::StoryElite:
		if (m_ptrDuplicateDataManager != nullptr)
			m_ptrDuplicateDataManager->UpdateLatestEliteTollgateID(m_iTollgateId);
		break;
	}
}

void CTollgate::Deactivate()
{
	m_bActivated = false;
}

void CTollgate::ArrangeDataWhileActivated()
{
	if (m_ptrDuplicateDataManager == nullptr)
	{
		rfalse("�ؿ������ʱ���ȡ�������ʧ��");
		return;
	}

	switch (m_curInstanceType)
	{
	case InstanceType::Story:///��ʱֻ����ͨ�����ؿ����������¸����ؿ������е�һ�εĵ�����Ϣ
		m_ptrDuplicateDataManager->GetStroyTollgateData(m_iTollgateId, m_bFirstChallenge, m_iCurBattleGrade, m_iChanllengedTimes);
		if (m_bFirstChallenge)
		{
			const BattleFirstDropConfig* dropConfig = CConfigManager::getSingleton()->GetBattleFirstDropConfig(m_iTollgateId);
			if (dropConfig == nullptr)
				break;

			if (dropConfig->DropNum1 != 0)
			{
				m_mapFirstDropItems.insert(make_pair(dropConfig->BossItem1, dropConfig->DropNum1));
			}
			if (dropConfig->DropNum2 != 0)
			{
				m_mapFirstDropItems.insert(make_pair(dropConfig->BossItem2, dropConfig->DropNum2));
			}
		}
		break;
	default:
		m_bActivated = true;
		m_ptrDuplicateDataManager->GetTollgateData(m_curInstanceType, m_iTollgateId, m_iCurBattleGrade, m_iChanllengedTimes);
		break;
	}
}

void CTollgate::SetNextTollgateID(int tollgateID)
{
	m_iNextTollgateID = tollgateID;
}

void CTollgate::SetBattleGrade(int grade)
{
	if (grade > m_iCurBattleGrade)
		m_iCurBattleGrade = min(grade, 3);
}

bool CTollgate::GetCompletedState()
{
	return m_bCompleted;
}

bool CTollgate::GetActiveState()
{
	return m_bActivated;
}

int CTollgate::GetTollgateID()
{
	return m_iTollgateId;
}

int CTollgate::GetNextTollgateID()
{
	return m_iNextTollgateID;
}

const unordered_map<int, int>* CTollgate::GetCommonDropOutItemList()
{
	if (m_ptrDropOut == nullptr)
		return nullptr;

	return &m_ptrDropOut->GetCommonDropItemList();
}

const unordered_map<int, int>* CTollgate::GetBossDropItemList()
{
	if (m_ptrDropOut == nullptr)
		return nullptr;

	return &m_ptrDropOut->GetBossDropItemList();
}

void CTollgate::Reset()
{
	if (m_ptrDropOut == nullptr)
		return;

	m_ptrDropOut->Reset();
	m_listItem.clear();
}

void CTollgate::CalculateDropOutItem(int challengeTime /* = 1 */, bool bMopUpOperation /* = false */)
{
	if (m_ptrDropOut == nullptr)
	{
		rfalse("�ؿ�%d��Ӧ����Ʒ����ģ����߽���ģ��Ϊ��", m_iTollgateId);
		return;
	}

	///�������ɨ��ģʽ����ս��������1������������
	if (!m_bMopUpOperation && challengeTime > 1)
	{
		rfalse("����ɨ����������ս��������1������������");
		return;
	}

	///���������Ʒǰ�Ȱ���һ�εĵ�����Ϣ���
	Reset();

	if (m_bFirstChallenge && m_curInstanceType == InstanceType::Story)
	{
		for (auto iter : m_mapFirstDropItems)
		{
			for (int i = 0; i < iter.second; ++i)
			{
				m_ptrDropOut->AddBossDropedItem(iter.first, iter.second);
				AddEarningItem(iter.first, iter.second);
			}
		}
	}
	else
	{
		m_ptrDropOut->Reset();
		BossItemMissedCounter* data = nullptr;
		///��־�Ƿ������һ����Ʒ�󣬾�һ���Ե���ָ����
		bool singleDrop = false;
		switch (m_curInstanceType)
		{
		case InstanceType::Story:
			data = m_ptrDuplicateDataManager->GetMissedBossItemDataInStoryDup();
			break;
		case InstanceType::StoryElite:
			data = m_ptrDuplicateDataManager->GetMissedBossItemDataInEliteDup();
			break;
		case InstanceType::LeagueOfLegends:
			singleDrop = true;
			break;
		case InstanceType::TreasureHunting:
			singleDrop = true;
			break;
		}
		for (int i = 0; i < challengeTime; ++i)
		{
			///����ǽ���˵���ؾ�Ѱ�������Ļ����Ϳ���һ����Ʒ��ֱ�ӡ������õĳ����ĸ���
			m_ptrDropOut->CalculateDropItem(data, (m_curInstanceType == InstanceType::LeagueOfLegends || m_curInstanceType == InstanceType::TreasureHunting) ? true : false, bMopUpOperation);
		}

		const unordered_map<int, int>& commonDropOutList = m_ptrDropOut->GetCommonDropItemList();
		for (auto iter : commonDropOutList)
		{
			AddEarningItem(iter.first, iter.second);
		}

		const unordered_map<int, int>& bossDropOutList = m_ptrDropOut->GetBossDropItemList();
		for (auto iter : bossDropOutList)
		{
			AddEarningItem(iter.first, iter.second);
		}
	}
}

void CTollgate::AddEarningItem(int itemID, int itemNum)
{
	if (itemNum < 0)
		return;

	auto findResult = m_listItem.find(itemID);
	if (findResult == m_listItem.end())
		m_listItem.insert(make_pair(itemID, itemNum));
	else
		findResult->second += itemNum;
}