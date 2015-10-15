#include "stdafx.h"
#include "ChaseThiefTollgate.h"
#include "..\BaseDataManager.h"
#include "../DuplicateMoudle/TollgateDrop.h"
#include "NETWORKMODULE\SanguoPlayer.h"
#include "../DuplicateMoudle/DuplicateDataManager.h"
#include "../Common/ConfigManager.h"

CChaseThiefTollgate::CChaseThiefTollgate()
	: m_iTollgateID(0)
	, m_iAstrictLevel(0)
{
}


CChaseThiefTollgate::~CChaseThiefTollgate()
{
}

void CChaseThiefTollgate::Init(const BattleLevelConfig* config)
{
	if (nullptr == config)
	{
		rfalse("关卡初始化的时候传进来的关卡配置信息为空");
		return;
	}

	m_iTollgateID = config->ID;
	m_iAstrictLevel = config->LevelLimit;
	m_iGeneralsExp = config->Exp;
	m_iTeamExp = config->Cost;
	//m_iGloryValue = 0;
	//m_iSalaryValue = 0;
	const BattleDropConfig * dropConfig = CConfigManager::getSingleton()->GetBattleDropConfig(config->ID);
	if (dropConfig == nullptr)
		rfalse("don't find drop config with battleID : %d", config->ID);
	else
		m_iMoney = dropConfig->Money;
	m_pTollgateDrop.reset(new CTollgateDrop());
	m_pTollgateDrop->Init(CConfigManager::getSingleton()->GetBattleDropConfig(m_iTollgateID));
}

void CChaseThiefTollgate::Action()
{
	if (nullptr == m_pTollgateDrop)
		return;

	m_listItem.clear();
	m_pTollgateDrop->Reset();

	m_pTollgateDrop->CalculateDropItem(nullptr);

	auto commonDrop = m_pTollgateDrop->GetCommonDropItemList();
	for (auto iter : commonDrop)
	{
		AddEarningItem(iter.first, iter.second);
	}

	auto bossDrop = m_pTollgateDrop->GetBossDropItemList();
	for (auto iter : bossDrop)
	{
		AddEarningItem(iter.first, iter.second);
	}
}

void CChaseThiefTollgate::EarningClear(CBaseDataManager* pBaseDataManager)
{
	if (nullptr == pBaseDataManager)
		return;

	pBaseDataManager->AddGoods_SG(GoodsType::Exp, 0, m_iTeamExp);
	pBaseDataManager->AddGoods_SG(GoodsType::money, 0, m_iMoney);

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
	pBaseDataManager->AddGoods_SG(GoodsType::item, itemCount, numList, idList);
	delete[] idList;
	delete[] numList;
}

const unordered_map<int, int>* CChaseThiefTollgate::GetCommonDropOutItemList()
{
	if (m_pTollgateDrop == nullptr)
		return nullptr;

	return &m_pTollgateDrop->GetCommonDropItemList();
}

const unordered_map<int, int>* CChaseThiefTollgate::GetBossDropItemList()
{
	if (m_pTollgateDrop == nullptr)
		return nullptr;

	return &m_pTollgateDrop->GetBossDropItemList();
}

int CChaseThiefTollgate::AstrictLevel() const
{
	return m_iAstrictLevel;
}

void CChaseThiefTollgate::AddEarningItem(int itemID, int itemNum)
{
	if (itemNum < 0)
		return;

	auto findResult = m_listItem.find(itemID);
	if (findResult == m_listItem.end())
		m_listItem.insert(make_pair(itemID, itemNum));
	else
		findResult->second += itemNum;
}
