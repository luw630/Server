#include "stdafx.h"
#include "StorageManager.h"
#include "../../../../NETWORKMODULE/SanguoPlayer.h"
#include "../Common/ConfigManager.h"
#include "../BaseDataManager.h"
#include "Player.h"
#include "ScriptManager.h"
extern LPIObject GetPlayerBySID(DWORD dwStaticID);

CStorageManager::CStorageManager(int playerSID)
	:m_pItemDatas(nullptr),
	m_iCurPlayerSID(playerSID)
{
}


CStorageManager::~CStorageManager()
{
	m_mapItemIndex.clear();
	m_listFreeIndex.clear();
	m_pItemDatas = nullptr;
}

bool CStorageManager::InitItemData(SSanguoItem* pData)
{
	m_pItemDatas = pData;
	_Init();
	return true;
}

bool CStorageManager::AddItem(const int32_t itemID, const int32_t num)
{
	if (m_pItemDatas == nullptr)
	{
		rfalse("未指定三国物品数据");
		return false;
	}

	//去已有物品中查找此物品ID对应的物品数据//
	SSanguoItem* tempItem = m_mapItemIndex.find(itemID) != m_mapItemIndex.end() ? &m_pItemDatas[m_mapItemIndex[itemID]] : nullptr;
	if (tempItem == nullptr)
	{
		if (m_listFreeIndex.empty())
		{
			rfalse("背包物品格已满");
			return false;
		}
		
		//从未被使用的物品数组中获取索引//
		int32_t freeIndex = *(m_listFreeIndex.begin());
		tempItem = &m_pItemDatas[freeIndex];
		m_listFreeIndex.pop_front();
		m_mapItemIndex.insert(make_pair(itemID, freeIndex));
	}

	tempItem->m_dwItemID = itemID;
	tempItem->m_dwCount += num;
	if (m_funcAddItem._Empty() == false)
		m_funcAddItem(itemID);

	return true;
}

int32_t CStorageManager::_RemoveItem(const int32_t itemID, const int32_t num)
{
	if (m_pItemDatas == nullptr)
	{
		rfalse("未指定三国物品数据");
		return 0;
	}
	auto iter = m_mapItemIndex.find(itemID);
	if (iter == m_mapItemIndex.end())
	{
		rfalse("未找到相应物品%d", itemID);
		return 0;
	}

	SSanguoItem* tempItem = &m_pItemDatas[iter->second];
	if (tempItem == nullptr)
		return 0;

	int realNum = min(tempItem->m_dwCount, num);
	tempItem->m_dwCount -= realNum;
	if (tempItem->m_dwCount == 0)
	{
		tempItem->m_dwItemID = 0;	//此物品移除完全则将物品ID设置为0//
		m_listFreeIndex.push_back(iter->second);
		m_mapItemIndex.erase(iter);
	}

	return realNum;
}

bool CStorageManager::SellItem(const int32_t itemID, const int32_t num, BYTE& currencyType, DWORD& dwSellCount, DWORD& dwProperty)
{
	int32_t sellNum = _RemoveItem(itemID, num);
	if (sellNum == 0)
	{
		currencyType = 0;
		dwSellCount = 0;
		dwProperty = 0;
		return false;
	}

	int32_t price = 0;
	auto itemConfig = CConfigManager::getSingleton()->GetItemConfig(itemID);
	if (nullptr == itemConfig)
	{
		auto equipConfig = CConfigManager::getSingleton()->GetEquipment(itemID);
		price = (nullptr == equipConfig) ? 0 : equipConfig->sellPrice;
	}
	else
	{
		price = itemConfig->SellPrice;
	}

	dwSellCount = sellNum;
	currencyType = Currency_Money;

	dwProperty = price * sellNum;
	return true;
}

bool CStorageManager::GetResolvedPoints(int32_t itemArrayNum, const int32_t itemIDNumList[], OUT int& resolvedSoulPoints)
{
	if (itemArrayNum <= 0 || itemIDNumList == nullptr)
		return false;

	///限制一次性会分解的量
	itemArrayNum = min(MAX_ITEM_NUM_SYN_TO_CLIENT, itemArrayNum);

	try
	{
		resolvedSoulPoints = 0;
		int tempResolveSoulPoints = 0;
		for (int i = 0; i < itemArrayNum; ++i)
		{
			if (!ItemExistOrEnough(itemIDNumList[i], itemIDNumList[i + itemArrayNum]))
				return false;

			const ItemConfig* config = CConfigManager::getSingleton()->GetItemConfig(itemIDNumList[i]);
			if (config == nullptr)
			{
				const EquipmentConfig* equipConfig = CConfigManager::getSingleton()->GetEquipment(itemIDNumList[i]);
				if (equipConfig == nullptr)
					return false;
				else
					tempResolveSoulPoints = equipConfig->ResolveSoulPoints;
			}
			else
				tempResolveSoulPoints = config->ResolveSoulPoints;

			resolvedSoulPoints += tempResolveSoulPoints * itemIDNumList[i + itemArrayNum];
		}
	}
	catch (const std::exception& oor)
	{
		rfalse(oor.what());
	}

	return true;
}

bool CStorageManager::GetExpPotionAbility(const int32_t itemID, OUT int32_t& num, OUT int32_t& exp)
{
	exp = 0;
	const ItemConfig* config = CConfigManager::getSingleton()->GetItemConfig(itemID);
	if (ParseItemType(config) != Item_ExpPotion)
		return false;

	int32_t useNum = min(GetItemNum(itemID), num);
	if (useNum == 0)
		return false;

	num = useNum;
	exp = config->Ability * useNum;
	return true;
}

bool CStorageManager::AttachEquipment(const int32_t equipID)
{
	return _RemoveItem(equipID);
}

bool CStorageManager::CheckSweepTicket(int32_t itemID, int32_t num /*= 1*/)
{
	if (!ItemExistOrEnough(itemID, num))
		return false;

	const ItemConfig* config = CConfigManager::getSingleton()->GetItemConfig(itemID);
	if (ParseItemType(config) != Item_SweepTicket)
		return false;

	return true;
}

bool CStorageManager::TryToRemoveItem(const int32_t itemID, const int32_t num)
{
	if (!ItemExistOrEnough(itemID, num))
		return false;

	_RemoveItem(itemID, num);
	return true;
}

bool CStorageManager::UseConsumable(int32_t itemID, int32_t num /*= 1*/)
{
	if (!ItemExistOrEnough(itemID, num))
		return false;

	const ItemConfig* config = CConfigManager::getSingleton()->GetItemConfig(itemID);
	if (config == nullptr)
		return false;

	CPlayer * ptrPlayer = (CPlayer *)GetPlayerBySID(m_iCurPlayerSID)->DynamicCast(IID_PLAYER);
	if (ptrPlayer != nullptr)
	{
		g_Script.SetCondition(0, ptrPlayer, 0);
		///在lua端来实现使用消耗品的具体逻辑
		LuaFunctor(g_Script, "SI_UseComsumable")[itemID][num][config->Ability]();
		g_Script.CleanCondition();
	}
	else
		rfalse("获取不到CPlayer的指针");

	return true;
}

SSanguoItemType CStorageManager::ParseItemType(const ItemConfig* config)
{
	if (config == nullptr)
		return Item_None;

	switch (config->Type)
	{
	case 1:
		return Item_ExpPotion;
	case 2:
		return Item_SellingGoods;
	case 3:
		return Item_SoulStone;
	case 4:
		return Item_Debris;
	case 5:
		return Item_TreasureChest;
	case 6:
		return Item_SweepTicket;
	case 7:
		return Item_EndurancePotion;
	case 8:
		return Item_EnchantingMaterial;
	default:
		return Item_None;
	}
}

void CStorageManager::_Init()
{
	if (m_pItemDatas == nullptr)
	{
		rfalse("未指定三国物品数据");
		return;
	}
	m_listFreeIndex.clear();
	m_mapItemIndex.clear();
	SSanguoItem* tempItem = nullptr;
	for (int i = 0; i < MAX_ITEM_NUM; ++i)
	{
		tempItem = &m_pItemDatas[i];
		if (tempItem->m_dwItemID == 0)
			m_listFreeIndex.push_back(i);
		else
			m_mapItemIndex.insert(make_pair(tempItem->m_dwItemID, i));
	}
}

bool CStorageManager::ItemExistOrEnough(int32_t itemID, int32_t num /*= 1*/)
{
	if (m_pItemDatas == nullptr)
		return false;

	auto findResult = m_mapItemIndex.find(itemID);
	if (findResult == m_mapItemIndex.end())
		return false;

	if (m_pItemDatas[findResult->second].m_dwCount < num)
		return false;

	return true;
}

int CStorageManager::GetItemNum(int32_t itemID)
{
	if (m_pItemDatas == nullptr)
	{
		rfalse("未指定三国物品数据");
		return 0;
	}
	auto iter = m_mapItemIndex.find(itemID);
	if (iter == m_mapItemIndex.end())
	{
		rfalse("未找到相应物品%d", itemID);
		return 0;
	}

	SSanguoItem* tempItem = &m_pItemDatas[iter->second];
	if (tempItem == nullptr)
		return 0;

	return tempItem->m_dwCount;
}

void CStorageManager::BindAddItemFunction(std::function<void(int)>& func)
{
	m_funcAddItem = func;
}
