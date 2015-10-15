#include "stdafx.h"
#include "Duplicate.h"
#include "Tollgate.h"
#include "DuplicateAstrict.h"
#include "DuplicateDataManager.h"
#include "..\BaseDataManager.h"
#include "..\Common\ConfigManager.h"
#include "NETWORKMODULE\SanguoPlayer.h"
#include "Networkmodule\SanguoPlayerMsg.h"

CDuplicate::CDuplicate()
{
	m_bInitFlag = false;
	m_bActivete = false;
	m_iCurDuplciateID = 0;
	m_iNextDuplicateID = 0;
	m_iDefaultActivateTollgateID = 0;
	m_ptrCurSelectedTollgate = nullptr;
	m_ptrDuplicateAstrict.reset(nullptr);
	m_ptrChapterConfig = nullptr;
	m_ptrDataManager = nullptr;
	m_ptrDropedItemMsg = make_shared<SATollgateDropedItem>();
	m_ptrDuplicateDataManager = nullptr;
	m_curInstanceType = InstanceType::Story;
	m_mapTollgateContainer.clear();
}


CDuplicate::~CDuplicate()
{
}

void CDuplicate::Init(CDuplicateDataManager* pDuplicateDataManage, int chapterID)
{
	m_bInitFlag = false;
	if (pDuplicateDataManage == nullptr)
	{
		rfalse("����%d��ʼ��ʱ��������������Ϊ��");
		return;
	}

	m_ptrDuplicateDataManager = pDuplicateDataManage;
	m_ptrDataManager = &m_ptrDuplicateDataManager->GetBaseDataMgr();

	///��ȡ��������Ӧ��������Ϣ
	m_ptrChapterConfig = CConfigManager::getSingleton()->GetChapterConfig(chapterID);
	if (m_ptrChapterConfig == nullptr)
	{
		rfalse("�½�%d�Ҳ���������Ϣ", chapterID);
		return;
	}

	m_iCurDuplciateID = m_ptrChapterConfig->chapterID;
	m_ptrDuplicateAstrict.reset(new CDuplicateAstrict());
	m_ptrDuplicateAstrict->Init(m_ptrChapterConfig);
	m_curInstanceType = (InstanceType)m_ptrChapterConfig->chapterType;
	shared_ptr<CTollgate> lastTollgate = nullptr;
	///��������ʼ�������������йؿ�
	for (auto iter : m_ptrChapterConfig->battleIdList)
	{
		///��ʼ�������Ĺؿ�
		shared_ptr<CTollgate> tollgate = make_shared<CTollgate>();
		tollgate->Init(m_ptrDuplicateDataManager, CConfigManager::getSingleton()->GetBattleLevelConfig(iter), m_curInstanceType);

		if (m_iDefaultActivateTollgateID == 0)
			m_iDefaultActivateTollgateID = iter;

		///���ؿ�����������
		if (lastTollgate != nullptr)
			lastTollgate->SetNextTollgateID(iter);

		lastTollgate = tollgate;
		auto findResult = m_mapTollgateContainer.find(iter);
		if (findResult != m_mapTollgateContainer.end())
		{
			findResult->second = tollgate;
			rfalse("����%d����ͬ�Ĺؿ�����");
		}
		else
			m_mapTollgateContainer.insert(make_pair(iter, tollgate));
	}
	m_bInitFlag = true;
}

void CDuplicate::Update()
{
	if (!m_bInitFlag || m_ptrDuplicateAstrict == nullptr)
		return;
	
	m_ptrDuplicateAstrict->UpdateCD();

	for (auto& iter : m_mapTollgateContainer)
		if (iter.second != nullptr)
			iter.second->Update();
}

void CDuplicate::ActionWhileEnterBattle(int challengeTime /* = 1 */, bool bMopUpOperation /* = false */)
{
	///Ŀǰ��m_ptrCurSelectedTollgate����Check�ɹ���������ΪCheck��ֵ������սĳһ���ؿ�֮ǰ����ҪCheck�ɹ�֮�������ս�����Դ˴����ӿͻ��˷���Ҫ������DuplicateID��TollgateID����֤
	if (m_ptrCurSelectedTollgate == nullptr || m_ptrDuplicateAstrict == nullptr || m_ptrDataManager == nullptr || m_ptrDropedItemMsg == nullptr)
		return;

	m_ptrCurSelectedTollgate->ActionWhileEnterBattle(challengeTime);

	///���������Ʒ��Ϣ��ȡ����
	const unordered_map<int, int>* commonDropOutItemIdList = m_ptrCurSelectedTollgate->GetCommonDropOutItemList();
	const unordered_map<int, int>* bossDropItemIdList = m_ptrCurSelectedTollgate->GetBossDropItemList();
	if (commonDropOutItemIdList == nullptr || bossDropItemIdList == nullptr) return;
	///��ʼ��������Ϣ
	m_ptrDropedItemMsg->duplicateID = m_iCurDuplciateID;
	m_ptrDropedItemMsg->tollgateID = m_ptrCurSelectedTollgate->GetTollgateID();
	m_ptrDropedItemMsg->bMopUpOperation = bMopUpOperation;
	m_ptrDropedItemMsg->duplicateType = m_curInstanceType;
	m_ptrDropedItemMsg->commonItemNum = commonDropOutItemIdList->size();
	m_ptrDropedItemMsg->commonItemNum = min(m_ptrDropedItemMsg->commonItemNum, MAX_ITEM_NUM_SYN_TO_CLIENT / 2);
	m_ptrDropedItemMsg->bossItemNum = bossDropItemIdList->size();
	int unusedSpace = 0;
	///��ʼ��boss������Ʒ�Ķ�̬����
	int bossItemIndex = 0;
	try
	{
		for (auto const& iter : *bossDropItemIdList)
		{
			m_ptrDropedItemMsg->arrayBossItemID[bossItemIndex] = iter.first;
			m_ptrDropedItemMsg->arrayBossItemCount[bossItemIndex] = iter.second;
			++bossItemIndex;
		}
	}
	catch (const std::exception& oor)
	{
		rfalse("��ʼ���ؿ��е�boss������Ϣ����");
	}
	
	///��ʼ����ͨ������Ʒ�Ķ�̬����
	int commonItemIndex = 0;
	try
	{
		for (auto const& iter : *commonDropOutItemIdList)
		{
			m_ptrDropedItemMsg->arrayCommonItemList[commonItemIndex] = iter.first;
			m_ptrDropedItemMsg->arrayCommonItemList[commonItemIndex + m_ptrDropedItemMsg->commonItemNum] = iter.second;
			++commonItemIndex;
		}
	}
	catch (const std::exception& oor)
	{
		rfalse("��ʼ���ؿ��е���ͨ������Ϣ����");
	}

	///���ͻ��˷��͵�����Ϣ,�ض���Ч����
	g_StoreMessage(m_ptrDataManager->GetDNID(), m_ptrDropedItemMsg.get(), sizeof(SATollgateDropedItem) - (MAX_ITEM_NUM_SYN_TO_CLIENT - commonItemIndex * 2) * sizeof(int));
	m_ptrDuplicateAstrict->Action();
}

void CDuplicate::EarningClearing(int duplicateID, int tollgateID, int combatGrade, int challengeTimes /* = 1 */)
{
	if (m_ptrCurSelectedTollgate == nullptr || m_ptrDuplicateAstrict == nullptr || m_ptrDuplicateDataManager == nullptr)
		return;

	if (m_ptrCurSelectedTollgate->GetTollgateID() != tollgateID)
	{
		rfalse("����%d�������ؿ�%d����������Ϣ�������Ĺؿ�ID��׼����ս�Ĺؿ�ID��һ��", duplicateID, tollgateID);
		return;
	}

	///���йؿ������߼�
	m_ptrCurSelectedTollgate->EarningClearing(challengeTimes);
	m_ptrDuplicateAstrict->Resolves(challengeTimes);
	m_ptrCurSelectedTollgate->SetBattleGrade(combatGrade);
	m_ptrDuplicateDataManager->UpdateTollgateData(m_curInstanceType, duplicateID, tollgateID, combatGrade, challengeTimes);

	///������һ���ؿ�
	auto findResult = m_mapTollgateContainer.find(m_ptrCurSelectedTollgate->GetNextTollgateID());
	if (findResult != m_mapTollgateContainer.end() && findResult->second != nullptr)
		findResult->second->Activate();
}

void CDuplicate::RefreshTicket()
{
	if (m_ptrDuplicateAstrict == nullptr || !m_bInitFlag)
		return;

	m_ptrDuplicateAstrict->ReSetCurTicketNum(m_ptrChapterConfig->ticketNum);
	for (auto itor : m_mapTollgateContainer)
		if (itor.second != nullptr)
			itor.second->RefreshTicket();
}

void CDuplicate::ResetSpecifyTollgateTicket(int tolllgateID)
{
	auto findResult = m_mapTollgateContainer.find(tolllgateID);
	if (findResult == m_mapTollgateContainer.end() || findResult->second == nullptr)
		return;

	findResult->second->RefreshTicket();
}

CheckResult CDuplicate::Check(int tollgateID, int challengeTimes /*= 1*/, bool mopUpOperation /*= false*/)
{
	CheckResult result = CheckResult::Failed;
	if (m_ptrDuplicateAstrict == nullptr || m_ptrDataManager == nullptr)
		return result;

	///��鱾���������ƣ�������ս���������ƣ��������������������ͨ�����;�Ӣ��������û�и�����ս����������
	result = m_ptrDuplicateAstrict->Check(*m_ptrDataManager, challengeTimes);
	if (result != CheckResult::Pass) return result;

	///��ѰtollgateID�Ƿ��Ǳ������Ĺؿ�
	auto findResult = m_mapTollgateContainer.find(tollgateID);
	if (findResult != m_mapTollgateContainer.end())
	{
		shared_ptr<CTollgate> oldTollgate = m_ptrCurSelectedTollgate;
		m_ptrCurSelectedTollgate = findResult->second;

		if (m_ptrCurSelectedTollgate == nullptr) return result;

		if (m_ptrCurSelectedTollgate->GetActiveState())
		{
			///����˵���ؾ�Ѱ��û��ɨ���ĸ���
			if (mopUpOperation && (m_curInstanceType == InstanceType::LeagueOfLegends || m_curInstanceType == InstanceType::TreasureHunting))
			{
				result = CheckResult::Failed;
				return result;
			}

			result = m_ptrCurSelectedTollgate->Check(challengeTimes, mopUpOperation);
			if (result != CheckResult::Pass)
			{
				m_ptrCurSelectedTollgate = oldTollgate;
			}
		}
		else
			result = CheckResult::Failed;
	}
	else
		result = CheckResult::Failed;

	return result;
}

void CDuplicate::Activate()
{
	m_bActivete = true;
	if (m_mapTollgateContainer.find(m_iDefaultActivateTollgateID) != m_mapTollgateContainer.end())
		m_mapTollgateContainer[m_iDefaultActivateTollgateID]->Activate();

	///���������еĹؿ�����ʼ���ιؿ�����
	for (auto iter : m_mapTollgateContainer)
	{
		iter.second->ArrangeDataWhileActivated();
	}

	if (m_ptrDuplicateDataManager == nullptr || m_ptrDuplicateAstrict == nullptr)
		return;

	switch (m_curInstanceType)
	{
	case InstanceType::Story:
		m_ptrDuplicateDataManager->UpdateLatestNormalTollgateID(m_iDefaultActivateTollgateID);
		break;
	case InstanceType::StoryElite:
		m_ptrDuplicateDataManager->UpdateLatestEliteTollgateID(m_iDefaultActivateTollgateID);
		break;
	case InstanceType::TreasureHunting:
	case InstanceType::LeagueOfLegends:
		///��Ϊ���¸�������Ӣ�����ĸ�����ս���������޵ģ���Ӣ�����ֹؿ���ս�������ƣ������Կ����ڴ���Ϊ0
		int challegedTime = 0;
		///�˽ӿ�ֻ�᷵�س��ؾ�Ѱ��������˵�ĸ����Ķ�Ӧ����ս����
		challegedTime = m_ptrDuplicateDataManager->GetDuplicateChallengedTimes(m_curInstanceType, m_iCurDuplciateID);
		m_ptrDuplicateAstrict->SetCurChallengeNum(challegedTime);
		break;
	}
}

void CDuplicate::Deactivate()
{
	m_bActivete = false;
	for (auto iter : m_mapTollgateContainer)
	{
		iter.second->Deactivate();
	}
}

void CDuplicate::ActiveAllTollgate()
{
	Activate();
	for (auto iter : m_mapTollgateContainer)
	{
		iter.second->Activate();
	}
}

void CDuplicate::ActivateTo(int tollgateID)
{
	auto findResult = m_mapTollgateContainer.find(tollgateID);
	if (findResult == m_mapTollgateContainer.end()) return;

	Activate();
	for (auto iter : m_mapTollgateContainer)
	{
		iter.second->Conquer();
		if (iter.first == tollgateID)
			break;
	}
}

bool CDuplicate::IsTollgateExist(int tollgateID)
{
	auto findResult = m_mapTollgateContainer.find(tollgateID);
	if (findResult == m_mapTollgateContainer.end())
		return false;

	return true;
}

void CDuplicate::SetNextDuplciateID(int duplicateID)
{
	m_iNextDuplicateID = duplicateID;
}

bool CDuplicate::GetCompletedState()
{
	///�����ǰ�ؿ������һ���ؿ�û��ͨ������ô����������û��ͨ��
	auto iter = m_mapTollgateContainer.end();
	--iter;
	if (iter->second == nullptr || iter->second->GetCompletedState() == false)
		return false;

	return true;
}

bool CDuplicate::GetActiveState()
{
	return m_bActivete;
}

InstanceType CDuplicate::GetCurInstanceType()
{
	return m_curInstanceType;
}

int CDuplicate::GetDuplicateID()
{
	return m_iCurDuplciateID;
}

int CDuplicate::GetNextDuplicateID()
{
	return m_iNextDuplicateID;
}

int CDuplicate::GetCurTollgateID()
{
	if (m_ptrCurSelectedTollgate == nullptr)
		return -1;

	return m_ptrCurSelectedTollgate->GetTollgateID();
}

void  CDuplicate::ReSendCaculatedDropItem()
{
	if (m_ptrDropedItemMsg == nullptr || m_ptrDataManager == nullptr)
		return;

	///���ͻ��˷��͵�����Ϣ,�ض���Ч����
	g_StoreMessage(m_ptrDataManager->GetDNID(), m_ptrDropedItemMsg.get(), sizeof(SATollgateDropedItem) - (MAX_ITEM_NUM_SYN_TO_CLIENT - m_ptrDropedItemMsg->commonItemNum * 2) * sizeof(int));
}
