#include "stdafx.h"
#include "TollgateDrop.h"
#include "..\Common\SanguoConfigTypeDef.h"
#include "DuplicateDataManager.h"
#include "..\..\Random.h"

CTollgateDrop::CTollgateDrop()
{
	m_iMinNumber = 0;
	m_iMaxNumber = 0;
	m_iWhiteItemWeight = 0;
	m_iGreenItemWeight = 0;
	m_iBlueItemWeight = 0;
	m_iPurpleItemWeight = 0;
	m_iBossItem1 = 0;
	m_iBossItemNumber1 = 0;
	m_iBossItemDropPercent1 = 0;
	m_iBossItem2 = 0;
	m_iBossItemNumber2 = 0;
	m_iBossItemDropPercent2 = 0;
	m_iBossItem3 = 0;
	m_iBossItemNumber3 = 0;
	m_iBossItemDropPercent3 = 0;
	m_whiteItemList = nullptr;
	m_greenItemList = nullptr;
	m_blueItemList = nullptr;
	m_purpleItemList = nullptr;
	m_propItem = nullptr;
	m_propItemNum = nullptr;
	m_commonDropItemIdList.clear();
	m_bossDropItemIdList.clear();
}


CTollgateDrop::~CTollgateDrop()
{
	m_whiteItemList = nullptr;
	m_greenItemList = nullptr;
	m_blueItemList = nullptr;
	m_purpleItemList = nullptr;
}

void CTollgateDrop::Init(const BattleDropConfig* dropConfig)
{
	if (dropConfig == nullptr)
	{
		rfalse("关卡初始化的时候传进来的关卡掉落信息为空");
		return;
	}
	m_iMinNumber = dropConfig->MinNumber;
	m_iMaxNumber = dropConfig->MaxNumber;
	m_iWhiteItemWeight = dropConfig->WhiteItemWeight;
	m_iGreenItemWeight = dropConfig->GreenItemWeight;
	m_iBlueItemWeight = dropConfig->BlueItemWeight;
	m_iPurpleItemWeight = dropConfig->PurpleItemWeight;

	m_iBossItem1 = dropConfig->BossItem1;
	m_iBossItemNumber1 = dropConfig->BossItemNumber1;
	m_iBossItemDropPercent1 = dropConfig->BossItemDropPercent1;

	m_iBossItem2 = dropConfig->BossItem2;
	m_iBossItemNumber2 = dropConfig->BossItemNumber2;
	m_iBossItemDropPercent2 = dropConfig->BossItemDropPercent2;

	m_iBossItem3 = dropConfig->BossItem3;
	m_iBossItemNumber3 = dropConfig->BossItemNumber3;
	m_iBossItemDropPercent3 = dropConfig->BossItemDropPercent3;

	m_whiteItemList = &dropConfig->WhiteItemList;
	m_greenItemList = &dropConfig->GreenItemList;
	m_blueItemList = &dropConfig->BlueItemList;
	m_purpleItemList = &dropConfig->PurpleItemList;

	m_propItem = &dropConfig->PropID;
	m_propItemNum = &dropConfig->PropNumber;
}

void CTollgateDrop::CalculateDropItem(BossItemMissedCounter* countData, bool singleDrop /* = false */, bool bMopUpOperation /* = false */)
{
	///下面开始加上扫荡会固定掉落的物品
	if (bMopUpOperation)
	{
		try
		{
			for (int i = 0; i < m_propItem->size(); ++i)
			{
				auto findResult = m_commonDropItemIdList.find((*m_propItem)[i]);
				if (findResult == m_commonDropItemIdList.end())
					m_commonDropItemIdList.insert(make_pair((*m_propItem)[i], (*m_propItemNum)[i]));
				else
					findResult->second += (*m_propItemNum)[i];
			}
		}
		catch (const std::exception& oor)
		{
			rfalse("扫荡固定掉落物品计算发生错误");
		}
	}

	///下面开始计算BOSS会掉落的物品
	int bossItemDropRandom = CRandom::RandRange(1, 100);
	if (m_iBossItem1 != 0)
	{
		///判断是否掉落BOSS物品1
		if (bossItemDropRandom <= m_iBossItemDropPercent1 || (countData != nullptr && countData->AddItem1RefenceCount()))
		{
			if (countData != nullptr)
				countData->ResetItem1Counter();
			auto findResult = m_bossDropItemIdList.find(m_iBossItem1);
			if (findResult == m_bossDropItemIdList.end())
				m_bossDropItemIdList.insert(make_pair(m_iBossItem1, m_iBossItemNumber1));
			else
				++findResult->second;
		}
	}
	bossItemDropRandom = CRandom::RandRange(1, 100);
	if (m_iBossItem2 != 0)
	{
		if (bossItemDropRandom <= m_iBossItemDropPercent2 || (countData != nullptr && countData->AddItem2RefenceCount()))
		{
			///判断是否掉落BOSS物品2
			if (countData != nullptr)
				countData->ResetItem2Counter();
			auto findResult = m_bossDropItemIdList.find(m_iBossItem2);
			if (findResult == m_bossDropItemIdList.end())
				m_bossDropItemIdList.insert(make_pair(m_iBossItem2, m_iBossItemNumber2));
			else
				++findResult->second;
		}
	}
	bossItemDropRandom = CRandom::RandRange(1, 100);
	if (m_iBossItem3 != 0)
	{
		if (bossItemDropRandom <= m_iBossItemDropPercent3 || (countData != nullptr && countData->AddItem3RefenceCount()))
		{
			///判断是否掉落BOSS物品3
			if (countData != nullptr)
				countData->ResetItem3Counter();
			auto findResult = m_bossDropItemIdList.find(m_iBossItem3);
			if (findResult == m_bossDropItemIdList.end())
				m_bossDropItemIdList.insert(make_pair(m_iBossItem3, m_iBossItemNumber3));
			else
				++findResult->second;
		}
	}

	if (m_whiteItemList == nullptr && m_greenItemList == nullptr && m_blueItemList == nullptr && m_purpleItemList == nullptr)
	{
		return;
	}

	///随机普通物品掉落的个数，范围为[m_iMinNumber， m_iMaxNumber]
	int dropNum = CRandom::RandRange(m_iMinNumber, m_iMaxNumber);
	///如果普通物品的掉落数为0，则不计算普通物品的掉了 直接返回
	if (dropNum <= 0)
		return;

	int whiteItemNum = 0;
	int greenItemNum = 0;
	int blueItemNum = 0;
	int purpleItemNum = 0;
	int randomWhiteIndex = -1;
	int randomGreenIndex = -1;
	int randomBlueIndex = -1;
	int randomPurpleIndex = -1;
	int dropedItemNum = 0;
	while (true)
	{
		if (dropedItemNum >= dropNum)
			break;

		int commonItemRandom = 0;
		int baseWeight = m_iWhiteItemWeight + m_iGreenItemWeight + m_iBlueItemWeight + m_iPurpleItemWeight;
		commonItemRandom = CRandom::RandRange(1, baseWeight);
		///白色物品的掉落随机
		if (m_whiteItemList != nullptr && commonItemRandom <= m_iWhiteItemWeight)
		{
			whiteItemNum = m_whiteItemList->size();
			if (whiteItemNum > 0)
			{
				RandomItem(dropNum, whiteItemNum, m_whiteItemList, randomWhiteIndex, dropedItemNum, singleDrop);
				continue;
			}
		}
		else if (m_greenItemList != nullptr && commonItemRandom <= m_iWhiteItemWeight + m_iGreenItemWeight)///绿色物品的掉落随机
		{
			greenItemNum = m_greenItemList->size();
			if (greenItemNum > 0)
			{
				RandomItem(dropNum, greenItemNum, m_greenItemList, randomGreenIndex, dropedItemNum, singleDrop);
				continue;
			}
		}
		else if (m_blueItemList != nullptr && commonItemRandom <= m_iWhiteItemWeight + m_iGreenItemWeight + m_iBlueItemWeight)///蓝色物品的掉落随机
		{
			blueItemNum = m_blueItemList->size();
			if (blueItemNum > 0)
			{
				RandomItem(dropNum, blueItemNum, m_blueItemList, randomBlueIndex, dropedItemNum, singleDrop);
				continue;
			}
		}
		else if (m_purpleItemList != nullptr && commonItemRandom <= baseWeight)///紫色物品的掉落随机
		{
			purpleItemNum = m_purpleItemList->size();
			if (purpleItemNum > 0)
			{
				RandomItem(dropNum, purpleItemNum, m_purpleItemList, randomPurpleIndex, dropedItemNum, singleDrop);
				continue;
			}
		}

		if (m_iPurpleItemWeight == 0 && m_iBlueItemWeight == 0 && m_iGreenItemWeight == 0 && m_iWhiteItemWeight == 0)
		{
			rfalse("关卡掉落计算失败");
			return;
		}
	}
}

void CTollgateDrop::Reset()
{
	m_commonDropItemIdList.clear();
	m_bossDropItemIdList.clear();
}

void CTollgateDrop::AddBossDropedItem(int itemID, int itemNum)
{
	if (itemNum < 0)
		return;

	auto findResult = m_bossDropItemIdList.find(itemID);
	if (findResult == m_bossDropItemIdList.end())
		m_bossDropItemIdList.insert(make_pair(itemID, itemNum));
	else
		findResult->second += itemNum;
}

const unordered_map<int, int>& CTollgateDrop::GetCommonDropItemList()
{
	return m_commonDropItemIdList;
}

const unordered_map<int, int>& CTollgateDrop::GetBossDropItemList()
{
	return m_bossDropItemIdList;
}

void CTollgateDrop::RandomItem(int dropNum, int itemNum, const vector<int>* itemList, OUT int& stepIndex, OUT int& dropedItemNum, bool singleDrop /* = false */)
{
	if (itemList == nullptr)
		return;

	///该尽量避免同一个物品掉落多次的情况
	if (!singleDrop)
	{
		///首先确认一个基准下标，后续的如果
		if (stepIndex == -1)
			stepIndex = CRandom::RandRange(0, itemNum - 1);
		else
			if (itemNum > 3) ///如果物品个数大于3，则步进为个数的三分之一
				stepIndex += CRandom::RandRange(1, itemNum / 3);
			else///如果物品个数小于等于3，则步进为1
				++stepIndex;

		///越界的回到初始位置
		if (stepIndex >= itemNum)
			stepIndex -= itemNum;

		if (stepIndex < 0)
			stepIndex = 0;

		++dropedItemNum;
		auto findResult = m_commonDropItemIdList.find((*itemList)[stepIndex]);
		if (findResult != m_commonDropItemIdList.end())
			++findResult->second;
		else
		{
			m_commonDropItemIdList.insert(make_pair((*itemList)[stepIndex], 1));
		}
	}
	else///同一个物品可以掉落多次的情况，跟策划商量过，一次性将某一个物品该掉落的数量加进去
	{
		///首先确认一个基准下标，后续的如果
		if (stepIndex == -1)
			stepIndex = CRandom::RandRange(0, itemNum - 1);

		dropedItemNum = dropNum;
		m_commonDropItemIdList.insert(make_pair((*itemList)[stepIndex], dropNum));
	}
}
