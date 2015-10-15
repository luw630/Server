#include "Stdafx.h"
#include "NpcSaleMgr.h"

CNpcSaleService::~CNpcSaleService()
{
	for (NpcGoodInfo::iterator it = m_NpcGoodInfo.begin(); it != m_NpcGoodInfo.end(); ++it)
	{
		NpcGoods *temp = it->second;
		delete temp;
	}

	m_NpcGoodInfo.clear();
}

void CNpcSaleService::ReSetInfo()
{
	// 不需要释放内存，为了在热更新的时候，提高速度
	for (NpcGoodInfo::iterator it = m_NpcGoodInfo.begin(); it != m_NpcGoodInfo.end(); ++it)
	{
		NpcGoods *temp = it->second;
		memset(temp, 0, sizeof(NpcGoods));
	}
}

NpcGoods *CNpcSaleService::GetSaleInfo(WORD npcIndexID)
{
	if (0 == npcIndexID)
		return 0;

	NpcGoodInfo::iterator result = m_NpcGoodInfo.find(npcIndexID);

	if (result != m_NpcGoodInfo.end())
		return result->second;
	else
		return 0;
}

void CNpcSaleService::LoadItemInfo(WORD npcIndexID, DWORD itemID, WORD itemCount)
{
	if (0 == npcIndexID || 0 == itemID || 0 == itemCount)
		return;

	NpcGoods *goodInfo = GetSaleInfo(npcIndexID);
	if (goodInfo)
	{
		// 满了
		if (NpcGoods::ItemCount == goodInfo->m_GoodCount)
			return;
	}
	else
	{
		goodInfo = new NpcGoods;
	}

	goodInfo->m_Goods[goodInfo->m_GoodCount*2]	 = itemID;
	goodInfo->m_Goods[goodInfo->m_GoodCount*2+1] = itemCount;
	goodInfo->m_GoodCount++;

	m_NpcGoodInfo[npcIndexID] = goodInfo;
}
void CFollowShopService::LoadItemInfo(WORD Index,DWORD ItemID)
{
	CFollowShopService::FollowInfoMap::iterator iter =  m_FollowShopInfo.find(Index);
	if (iter != m_FollowShopInfo.end())
	{
		iter->second.m_Itemlist[iter->second.m_Num++] = ItemID;
	}
	else
	{
		FollowShopStruct temp;
		temp.m_Itemlist[temp.m_Num++] = ItemID;
		m_FollowShopInfo.insert(std::make_pair(Index,temp) );
	}

}
void CFollowShopService::PrintfInfo()
{
	CFollowShopService::FollowInfoMap::iterator iter = m_FollowShopInfo.begin();
	while (iter != m_FollowShopInfo.end())
	{
		long key = iter->first;
		for (int i = 0; i < iter->second.m_Num; ++i)
		{
			long k = iter->second.m_Itemlist[i];
			int ik = 0;
		}
		++iter;
	}
}

bool CFollowShopService::GetFollowShopList(WORD Index,DWORD *itemList,WORD &Num,char name[8][10],WORD & nameNum)
{
	CFollowShopService::FollowInfoMap::iterator iter = m_FollowShopInfo.find(Index);
	if (iter != m_FollowShopInfo.end())
	{
		Num = iter->second.m_Num;
		nameNum = m_nameNum;
		memcpy(itemList,&iter->second.m_Itemlist,sizeof(DWORD) * 120);
		memcpy(name,m_name,sizeof(m_name));
		return true;
	}
	return false;
}
bool CFollowShopService::IsExitFollowItemID(WORD Index,DWORD ItemID)
{
	CFollowShopService::FollowInfoMap::iterator iter = m_FollowShopInfo.find(Index);
	if (iter != m_FollowShopInfo.end())
	{
		for (int i = 0; i < iter->second.m_Num; ++i)
		{
			if ( iter->second.m_Itemlist[i] == ItemID )
			{
				return true;
			}
		}
	}
	return false;
}
void CFollowShopService::ClearFollowShop()
{
	if (m_FollowShopInfo.size() != 0)
	{
		m_FollowShopInfo.clear();
		memset(&m_name[0][0], 0 ,sizeof(m_name));
		m_nameNum = 0;
	}
}
void CFollowShopService::LoadFollowMenuInfo(WORD Index,std::string Menuename)
{
	memcpy(&m_name[m_nameNum++],Menuename.c_str(),10);
}

FollowShopStruct * CFollowShopService::FindFollowStruct(long Index)
{
	CFollowShopService::FollowInfoMap::iterator iter =  m_FollowShopInfo.find(Index);
	if (iter != m_FollowShopInfo.end())
	{
		return &iter->second;
	}
	return 0;
}
void CProtectInfo::LoadProtectInfo(WORD Index,DWORD ItemID,BYTE MoneyType,long Money)
{
	ProtectInfoStruct temp;
	temp.ItemID = ItemID;
	temp.MoneyType = MoneyType;
	temp.Money = Money;
	m_ProtectInfoMap[Index] = temp; 
}
ProtectInfoStruct * CProtectInfo::FindProtectInfoStruct(long Index)
{
	CProtectInfo::ProtectInfoMap::iterator iter = m_ProtectInfoMap.find(Index);
	if (iter != m_ProtectInfoMap.end())
	{
		return &iter->second;
	}
	return 0;
}
