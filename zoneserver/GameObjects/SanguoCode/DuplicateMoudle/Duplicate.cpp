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
		rfalse("副本%d初始化时，副本管理数据为空");
		return;
	}

	m_ptrDuplicateDataManager = pDuplicateDataManage;
	m_ptrDataManager = &m_ptrDuplicateDataManager->GetBaseDataMgr();

	///获取本副本对应的配置信息
	m_ptrChapterConfig = CConfigManager::getSingleton()->GetChapterConfig(chapterID);
	if (m_ptrChapterConfig == nullptr)
	{
		rfalse("章节%d找不到配置信息", chapterID);
		return;
	}

	m_iCurDuplciateID = m_ptrChapterConfig->chapterID;
	m_ptrDuplicateAstrict.reset(new CDuplicateAstrict());
	m_ptrDuplicateAstrict->Init(m_ptrChapterConfig);
	m_curInstanceType = (InstanceType)m_ptrChapterConfig->chapterType;
	shared_ptr<CTollgate> lastTollgate = nullptr;
	///接下来初始化本副本的所有关卡
	for (auto iter : m_ptrChapterConfig->battleIdList)
	{
		///初始化单个的关卡
		shared_ptr<CTollgate> tollgate = make_shared<CTollgate>();
		tollgate->Init(m_ptrDuplicateDataManager, CConfigManager::getSingleton()->GetBattleLevelConfig(iter), m_curInstanceType);

		if (m_iDefaultActivateTollgateID == 0)
			m_iDefaultActivateTollgateID = iter;

		///将关卡都串联起来
		if (lastTollgate != nullptr)
			lastTollgate->SetNextTollgateID(iter);

		lastTollgate = tollgate;
		auto findResult = m_mapTollgateContainer.find(iter);
		if (findResult != m_mapTollgateContainer.end())
		{
			findResult->second = tollgate;
			rfalse("副本%d有相同的关卡配置");
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
	///目前的m_ptrCurSelectedTollgate会在Check成功过后设置为Check的值，而挑战某一个关卡之前必须要Check成功之后才能挑战，所以此处不从客户端发来要操作的DuplicateID和TollgateID做验证
	if (m_ptrCurSelectedTollgate == nullptr || m_ptrDuplicateAstrict == nullptr || m_ptrDataManager == nullptr || m_ptrDropedItemMsg == nullptr)
		return;

	m_ptrCurSelectedTollgate->ActionWhileEnterBattle(challengeTime);

	///将掉落的物品信息获取出来
	const unordered_map<int, int>* commonDropOutItemIdList = m_ptrCurSelectedTollgate->GetCommonDropOutItemList();
	const unordered_map<int, int>* bossDropItemIdList = m_ptrCurSelectedTollgate->GetBossDropItemList();
	if (commonDropOutItemIdList == nullptr || bossDropItemIdList == nullptr) return;
	///初始化掉落信息
	m_ptrDropedItemMsg->duplicateID = m_iCurDuplciateID;
	m_ptrDropedItemMsg->tollgateID = m_ptrCurSelectedTollgate->GetTollgateID();
	m_ptrDropedItemMsg->bMopUpOperation = bMopUpOperation;
	m_ptrDropedItemMsg->duplicateType = m_curInstanceType;
	m_ptrDropedItemMsg->commonItemNum = commonDropOutItemIdList->size();
	m_ptrDropedItemMsg->commonItemNum = min(m_ptrDropedItemMsg->commonItemNum, MAX_ITEM_NUM_SYN_TO_CLIENT / 2);
	m_ptrDropedItemMsg->bossItemNum = bossDropItemIdList->size();
	int unusedSpace = 0;
	///初始化boss掉落物品的动态数组
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
		rfalse("初始化关卡中的boss掉落消息出错");
	}
	
	///初始化普通掉落物品的动态数组
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
		rfalse("初始化关卡中的普通掉落消息出错");
	}

	///往客户端发送掉落消息,截断无效数据
	g_StoreMessage(m_ptrDataManager->GetDNID(), m_ptrDropedItemMsg.get(), sizeof(SATollgateDropedItem) - (MAX_ITEM_NUM_SYN_TO_CLIENT - commonItemIndex * 2) * sizeof(int));
	m_ptrDuplicateAstrict->Action();
}

void CDuplicate::EarningClearing(int duplicateID, int tollgateID, int combatGrade, int challengeTimes /* = 1 */)
{
	if (m_ptrCurSelectedTollgate == nullptr || m_ptrDuplicateAstrict == nullptr || m_ptrDuplicateDataManager == nullptr)
		return;

	if (m_ptrCurSelectedTollgate->GetTollgateID() != tollgateID)
	{
		rfalse("副本%d请求结算关卡%d出错，请求消息发过来的关卡ID跟准许挑战的关卡ID不一致", duplicateID, tollgateID);
		return;
	}

	///进行关卡结算逻辑
	m_ptrCurSelectedTollgate->EarningClearing(challengeTimes);
	m_ptrDuplicateAstrict->Resolves(challengeTimes);
	m_ptrCurSelectedTollgate->SetBattleGrade(combatGrade);
	m_ptrDuplicateDataManager->UpdateTollgateData(m_curInstanceType, duplicateID, tollgateID, combatGrade, challengeTimes);

	///激活下一个关卡
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

	///检查本副本的限制，比如挑战次数的限制，如果本副本的类型是普通副本和精英副本，则没有副本挑战次数的限制
	result = m_ptrDuplicateAstrict->Check(*m_ptrDataManager, challengeTimes);
	if (result != CheckResult::Pass) return result;

	///找寻tollgateID是否是本副本的关卡
	auto findResult = m_mapTollgateContainer.find(tollgateID);
	if (findResult != m_mapTollgateContainer.end())
	{
		shared_ptr<CTollgate> oldTollgate = m_ptrCurSelectedTollgate;
		m_ptrCurSelectedTollgate = findResult->second;

		if (m_ptrCurSelectedTollgate == nullptr) return result;

		if (m_ptrCurSelectedTollgate->GetActiveState())
		{
			///将神传说和秘境寻宝没有扫荡的概念
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

	///本副本所有的关卡都初始化次关卡数据
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
		///因为故事副本，精英副本的副本挑战次数是无限的（精英副本又关卡挑战次数限制），所以可以在此设为0
		int challegedTime = 0;
		///此接口只会返回出秘境寻宝、将神传说的副本的对应的挑战次数
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
	///如果当前关卡的最后一个关卡没有通过，那么整个副本就没有通过
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

	///往客户端发送掉落消息,截断无效数据
	g_StoreMessage(m_ptrDataManager->GetDNID(), m_ptrDropedItemMsg.get(), sizeof(SATollgateDropedItem) - (MAX_ITEM_NUM_SYN_TO_CLIENT - m_ptrDropedItemMsg->commonItemNum * 2) * sizeof(int));
}
