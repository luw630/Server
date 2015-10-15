#include "stdafx.h"
#include "ChaseThiefTollgateManager.h"
#include "ChaseThiefTollgate.h"
#include "../BaseDataManager.h"
#include "../Common/ConfigManager.h"
#include "../../../NETWORKMODULE/SanguoPlayer.h"
#include "../../../NETWORKMODULE/SanguoPlayerMsg.h"

CChaseThiefTollgateManager::CChaseThiefTollgateManager()
{
	//m_pMsgTollgateClearingAnswer.reset(new SATollgateEarningClearedInfor);
	//m_pMsgTollgateClearingAnswer->duplicateType = InstanceType::ChaseThief;
	m_pDropOutMsg = make_shared<SThiefAnswerStartChanllenge>();
}


CChaseThiefTollgateManager::~CChaseThiefTollgateManager()
{
	m_mapTollgateContainer.clear();
}

void CChaseThiefTollgateManager::Init()
{
	auto chapterIDSet = CConfigManager::getSingleton()->GetDuplicateListByType(InstanceType::ChaseThief);
	if (nullptr == chapterIDSet)
		return;

	for (auto iter : *chapterIDSet)
	{
		auto chapterConfig = CConfigManager::getSingleton()->GetChapterConfig(iter);
		if (nullptr == chapterConfig)
			return;
		for (auto id : chapterConfig->battleIdList)
		{
			shared_ptr<CChaseThiefTollgate> tollgate = make_shared<CChaseThiefTollgate>();
			tollgate->Init(CConfigManager::getSingleton()->GetBattleLevelConfig(id));

			auto findIter = m_mapTollgateContainer.find(id);
			if (findIter != m_mapTollgateContainer.end())
			{
				findIter->second = tollgate;
				rfalse("副本%d有相同的关卡配置", id);
			}
			else
				m_mapTollgateContainer.insert(make_pair(id, tollgate));
		}
	}
}

bool CChaseThiefTollgateManager::PerpationChanllenge(int playerLevel)
{
	m_pCurTollgate = nullptr;

	for (auto pair : m_mapTollgateContainer)
	{
		if (playerLevel < pair.second->AstrictLevel())
		{
			break;
		}

		m_pCurTollgate = pair.second;
	}

	if (nullptr == m_pCurTollgate)
		return false;

	return true;
}

bool CChaseThiefTollgateManager::Action(CBaseDataManager* pBaseDataManager)
{
	if (nullptr == m_pCurTollgate)
		return false;

	m_pCurTollgate->Action();
	///将掉落的物品信息获取出来
	const unordered_map<int, int>* commonDropOutItemIdList = m_pCurTollgate->GetCommonDropOutItemList();
	const unordered_map<int, int>* bossDropItemIdList = m_pCurTollgate->GetBossDropItemList();
	if (commonDropOutItemIdList == nullptr || bossDropItemIdList == nullptr)
		return false;
	///初始化掉落信息
	m_pDropOutMsg->commonItemNum = commonDropOutItemIdList->size();
	m_pDropOutMsg->commonItemNum = min(m_pDropOutMsg->commonItemNum, MAX_ITEM_NUM_SYN_TO_CLIENT / 2);
	m_pDropOutMsg->bossItemNum = bossDropItemIdList->size();
	int unusedSpace = 0;
	///初始化boss掉落物品的动态数组
	int bossItemIndex = 0;
	try
	{
		for (auto const& iter : *bossDropItemIdList)
		{
			m_pDropOutMsg->arrayBossItemID[bossItemIndex] = iter.first;
			m_pDropOutMsg->arrayBossItemCount[bossItemIndex] = iter.second;
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
			m_pDropOutMsg->arrayCommonItemList[commonItemIndex] = iter.first;
			m_pDropOutMsg->arrayCommonItemList[commonItemIndex + m_pDropOutMsg->commonItemNum] = iter.second;
			++commonItemIndex;
		}
	}
	catch (const std::exception& oor)
	{
		rfalse("初始化关卡中的普通掉落消息出错");
	}

	///往客户端发送掉落消息,截断无效数据
	g_StoreMessage(pBaseDataManager->GetDNID(), m_pDropOutMsg.get(), sizeof(SThiefAnswerStartChanllenge) - (MAX_ITEM_NUM_SYN_TO_CLIENT - commonItemIndex * 2) * sizeof(int));
	return true;
}

void CChaseThiefTollgateManager::EarningClear(CBaseDataManager* pBaseDataManager)
{
	if (nullptr == m_pCurTollgate || nullptr == pBaseDataManager)
		return;

	/// 团队经验 金钱 物品的添加
	m_pCurTollgate->EarningClear(pBaseDataManager);
	/// 添加武将经验
	/*int generalsExp = m_pCurTollgate->GeneralsExp();
	int heroNum = 0;
	int level = 0;
	bool levelLimitted = false;
	int heroBeforLevel = 0;
	int exp = 0;

	for (auto const& itor : vecHero)
	{
		auto heroData = pBaseDataManager->GetHero(itor);
		if (heroData == nullptr)
			continue;

		exp = generalsExp;
		heroBeforLevel = heroData->m_dwLevel;
		///判断是否加经验成功，成功的话，再判断是否加之前武将是否满级了
		if (pBaseDataManager->PlusHeroExp(itor, exp, level, levelLimitted))
		{
			///满级的话就不用再同步到客户端了
			if (levelLimitted && heroBeforLevel == level)
				continue;

			m_pMsgTollgateClearingAnswer->heroExpInfor[heroNum].m_dwHeroID = itor;
			m_pMsgTollgateClearingAnswer->heroExpInfor[heroNum].m_dwHeroLevel = level;
			m_pMsgTollgateClearingAnswer->heroExpInfor[heroNum].m_dwHeroExp = exp;
			++heroNum;
		}
	}
	m_pMsgTollgateClearingAnswer->heroNum = heroNum;
	m_pMsgTollgateClearingAnswer->bcleared = true;
	g_StoreMessage(pBaseDataManager->GetDNID(), m_pMsgTollgateClearingAnswer.get(), sizeof(SATollgateEarningClearedInfor) - sizeof(SHeroExpInfor) * (MAX_TOLLGATE_HERO_NUM - heroNum));*/
}

int CChaseThiefTollgateManager::GetCurTollgateID() const
{
	return m_pCurTollgate != nullptr ? m_pCurTollgate->TollgateID() : 0;
}
