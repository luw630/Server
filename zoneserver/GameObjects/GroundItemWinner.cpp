#include "Stdafx.h"
#include "GroundItemWinner.h"
#include "Player.h"
#include "Item.h"
#include "Random.h"
#include "networkmodule/ItemMsgs.h"

#pragma warning(push)
#pragma warning(disable:4996)

DWORD CGroundItemWinner::counter = 0;

BOOL CGroundItemWinner::AddDummyChance(std::vector<CPlayer *> &playerList, CItem *pItem, BYTE index)
{
	MY_ASSERT(pItem);
	MY_ASSERT(index < pItem->m_ItemsInPack && pItem->m_GroundItem[index].wIndex);
	MY_ASSERT(playerList.size() >= 1 && playerList.size() <= 4);

	++counter;

	// 发送Dummy摇号，仅用于显示
	SAGroundItemTakeChance msg;
	msg.m_Type	= SAGroundItemTakeChance::SGITC_NOUSE;
	msg.m_Index	= counter;
	msg.m_Item	= pItem->m_GroundItem[index];

	for (std::vector<CPlayer *>::iterator it = playerList.begin(); it != playerList.end(); ++it)
		g_StoreMessage((*it)->m_ClientIndex, &msg, sizeof(msg));

	return TRUE;
}

BOOL CGroundItemWinner::AddDummyChance( std::vector<CPlayer *> &playerList, CSingleItem *pItem )
{
	if(!pItem)return FALSE;
	MY_ASSERT(playerList.size() >= 1 && playerList.size() <= 4);

	++counter;

	// 发送Dummy摇号，仅用于显示
	SAGroundItemTakeChance msg;
	msg.m_Type	= SAGroundItemTakeChance::SGITC_NOUSE;
	msg.m_Index	= counter;
	msg.m_Item	= pItem->m_GroundItem;

	for (std::vector<CPlayer *>::iterator it = playerList.begin(); it != playerList.end(); ++it)
		g_StoreMessage((*it)->m_ClientIndex, &msg, sizeof(msg));

	return TRUE;
}

BOOL CGroundItemWinner::AddChanceItem(std::vector<CPlayer *> &playerList, CItem *pItem, BYTE index)
{
	MY_ASSERT(pItem);
	MY_ASSERT(index < pItem->m_ItemsInPack && pItem->m_GroundItem[index].wIndex);
	MY_ASSERT(playerList.size() >= 1 && playerList.size() <= 5);
	MY_ASSERT(m_ChanceItem.end() == m_ChanceItem.find(counter+1));

	ChangeItem *pNewChange = new ChangeItem;
	pNewChange->m_pItem			= pItem;
	pNewChange->m_Index			= index;
	pNewChange->m_InitPeople	= playerList.size();
	pNewChange->m_StartTime		= timeGetTime();

	++counter;

	m_ChanceItem.insert(make_pair(counter, pNewChange));

	// 通知玩家，并做记录
	SAGroundItemTakeChance msg;
	msg.m_Type	= SAGroundItemTakeChance::SGITC_USE;
	msg.m_Index	= counter;
	msg.m_Item	= pItem->m_GroundItem[index];

	size_t playerIndex = 0;
	for (std::vector<CPlayer *>::iterator it = playerList.begin(); it != playerList.end(); ++it, ++playerIndex)
	{
		MY_ASSERT((*it)->m_OwnedChangeItems.end() == (*it)->m_OwnedChangeItems.find(counter));

		pNewChange->m_Player[playerIndex] = *it;
		(*it)->m_OwnedChangeItems.insert(make_pair(counter, playerIndex));
				
		g_StoreMessage((*it)->m_ClientIndex, &msg, sizeof(msg));
	}

	// 包裹本身也要记录
	MY_ASSERT(pItem->m_ChanceItems.end() == pItem->m_ChanceItems.find(counter));
	pItem->m_ChanceItems.insert(counter);

	return TRUE;
}

BOOL CGroundItemWinner::AddChanceItem( std::vector<CPlayer *> &playerList, CSingleItem *pItem )
{
	if (!pItem)return FALSE;
	
	MY_ASSERT(playerList.size() >= 1 && playerList.size() <= 5);
	MY_ASSERT(m_ChanceItem.end() == m_ChanceItem.find(counter+1));

	ChangeItem *pNewChange = new ChangeItem;
	pNewChange->m_pSingleItem			= pItem;
	pNewChange->m_InitPeople	= playerList.size();
	pNewChange->m_StartTime		= timeGetTime();

	++counter;

	m_ChanceItem.insert(make_pair(counter, pNewChange));

	// 通知玩家，并做记录
	SAGroundItemTakeChance msg;
	msg.m_Type	= SAGroundItemTakeChance::SGITC_USE;
	msg.m_Index	= counter;
	msg.m_Item	= pItem->m_GroundItem;

	size_t playerIndex = 0;
	for (std::vector<CPlayer *>::iterator it = playerList.begin(); it != playerList.end(); ++it, ++playerIndex)
	{
		MY_ASSERT((*it)->m_OwnedChangeItems.end() == (*it)->m_OwnedChangeItems.find(counter));

		pNewChange->m_Player[playerIndex] = *it;
		(*it)->m_OwnedChangeItems.insert(make_pair(counter, playerIndex));

		g_StoreMessage((*it)->m_ClientIndex, &msg, sizeof(msg));
	}

	// 包裹本身也要记录
	MY_ASSERT(pItem->m_ChanceItems.end() == pItem->m_ChanceItems.find(counter));
	pItem->m_ChanceItems.insert(counter);

	return TRUE;
}

BOOL CGroundItemWinner::GroundItemGone(CItem *pItem)
{
//  	for (std::set<DWORD>::iterator it = pItem->m_ChanceItems.begin(); it != pItem->m_ChanceItems.end(); ++it)
//  	{
//  		DWORD index = *it;
//  		ChanceItems::iterator ti = m_ChanceItem.find(index);
//  		MY_ASSERT(m_ChanceItem.end() != ti);
//  
//  		ChangeItem *pChance = ti->second;
//  		MY_ASSERT(pChance->m_pItem == pItem);
//  
//  		// 通知玩家
//  		SAGroundItemChanceBack backMsg;
//  		backMsg.m_Choice = SAGroundItemChanceBack::SAGICB_ITEM_GONE;
//  		backMsg.m_Result = 0;
//  		backMsg.m_Index	 = index;
//  
//  		for (size_t i = 0; i < CI_MAXPLAYER; i++)
//  		{
//  			if (!pChance->m_Player[i])
//  				continue;
//  
//  			MY_ASSERT(pChance->m_Player[i]->m_OwnedChangeItems.end() != pChance->m_Player[i]->m_OwnedChangeItems.find(index));
//  			pChance->m_Player[i]->m_OwnedChangeItems.erase(index);
//  			
//  			if (0 == pChance->m_MaxPoint[i])
//  				g_StoreMessage(pChance->m_Player[i]->m_ClientIndex, &backMsg, sizeof(backMsg));
//  		}
//  
//  		m_ChanceItem.erase(index);
//  		delete pChance;
//  	}

	return TRUE;
}

BOOL CGroundItemWinner::GroundItemGone( CSingleItem *pItem )
{
	for (std::set<DWORD>::iterator it = pItem->m_ChanceItems.begin(); it != pItem->m_ChanceItems.end(); ++it)
	{
		DWORD index = *it;
		ChanceItems::iterator ti = m_ChanceItem.find(index);
		MY_ASSERT(m_ChanceItem.end() != ti);

		ChangeItem *pChance = ti->second;
		MY_ASSERT(pChance->m_pSingleItem == pItem);

		// 通知玩家
		SAGroundItemChanceBack backMsg;
		backMsg.m_Choice = SAGroundItemChanceBack::SAGICB_ITEM_GONE;
		backMsg.m_Result = 0;
		backMsg.m_Index	 = index;

		for (size_t i = 0; i < CI_MAXPLAYER; i++)
		{
			if (!pChance->m_Player[i])
				continue;

			MY_ASSERT(pChance->m_Player[i]->m_OwnedChangeItems.end() != pChance->m_Player[i]->m_OwnedChangeItems.find(index));
			pChance->m_Player[i]->m_OwnedChangeItems.erase(index);

			if (0 == pChance->m_MaxPoint[i])
				g_StoreMessage(pChance->m_Player[i]->m_ClientIndex, &backMsg, sizeof(backMsg));
		}

		m_ChanceItem.erase(index);
		delete pChance;
	}

	return TRUE;
}

BOOL CGroundItemWinner::Run()
{
	DWORD currTime = timeGetTime();

// 	ChanceItems copy(m_ChanceItem);
// 	for (ChanceItems::iterator it = copy.begin(); it != copy.end(); ++it)
// 	{
// 		ChangeItem *pChance = it->second;
// 		if (currTime - pChance->m_StartTime > 60000)
// 		{
// 			ChoiceTimeOut(it->first, pChance);
// 			delete pChance;
// 		}
// 	
	ChanceItems copy(m_ChanceItem);
	for (ChanceItems::iterator it = copy.begin(); it != copy.end(); ++it)
	{
		ChangeItem *pChance = it->second;
		if (pChance && currTime - pChance->m_StartTime > 60000&&pChance->m_pSingleItem)
		{
			ChoiceTimeOut(it->first, pChance);
			delete pChance;
		}
	}

	return TRUE;
}

BOOL CGroundItemWinner::ChoiceTimeOut(DWORD index, ChangeItem *pChance)
{
	if (!pChance || !pChance->m_pSingleItem)return FALSE;	
	if (pChance->m_pSingleItem->m_ChanceItems.size() == 0)return FALSE;
	if (pChance->m_pSingleItem->m_ChanceItems.end() == pChance->m_pSingleItem->m_ChanceItems.find(index))return FALSE;
	
//	MY_ASSERT(pChance->m_pSingleItem->m_ChanceItems.end() != pChance->m_pSingleItem->m_ChanceItems.find(index));

	// 移除玩家的映射
	SAGroundItemChanceBack backMsg;
	backMsg.m_Choice = SAGroundItemChanceBack::SAGICB_TIMEOUT;
	backMsg.m_Result = 0;
	backMsg.m_Index	 = index;

	for (size_t i = 0; i < CI_MAXPLAYER; i++)
	{
		if (!pChance->m_Player[i])
			continue;
		
		if (0 == pChance->m_MaxPoint[i])
		{
			pChance->m_MaxPoint[i] = 0xffff;
			g_StoreMessage(pChance->m_Player[i]->m_ClientIndex, &backMsg, sizeof(backMsg));
		}
	}

	FinishTakeChance(pChance, index);

	return TRUE;
}

BOOL CGroundItemWinner::PlayerLogOut(CPlayer *player, DWORD index, WORD playerIndex)	// 玩家下线了
{
	ChanceItems::iterator it = m_ChanceItem.find(index);
	MY_ASSERT(m_ChanceItem.end() != it);

	ChangeItem *pChance = it->second;
	MY_ASSERT(playerIndex < CI_MAXPLAYER && player == pChance->m_Player[playerIndex]);
	MY_ASSERT(pChance->m_pSingleItem->m_ChanceItems.end() != pChance->m_pSingleItem->m_ChanceItems.find(index));

	rfalse(2, 1, "玩家%s下线了，弃权了~~~", player->GetName());
	pChance->m_Player[playerIndex] = 0;

	if (0 == --pChance->m_InitPeople)
	{
		FinishTakeChance(pChance, index);
		delete pChance;
	}

	return TRUE;
}

BOOL CGroundItemWinner::StartTakeChance(CPlayer *player, DWORD index, WORD playerIndex, bool IsGiveup)
{
	ChanceItems::iterator it = m_ChanceItem.find(index);
	MY_ASSERT(m_ChanceItem.end() != it);

	ChangeItem *pChance = it->second;
	if (playerIndex >= CI_MAXPLAYER || !pChance->m_Player[playerIndex])		// 来了个打酱油的
		return FALSE;
	
	if (pChance->m_MaxPoint[playerIndex])		// 曾经放弃了或者已经投掷过
		return FALSE;

	MY_ASSERT(player == pChance->m_Player[playerIndex]);
	MY_ASSERT(pChance->m_pSingleItem->m_ChanceItems.end() != pChance->m_pSingleItem->m_ChanceItems.find(index));

	if (!IsGiveup)
	{
		pChance->m_MaxPoint[playerIndex] = CRandom::RandRange(1, 100);
		rfalse(2, 1, "玩家%s参与的摇号，点数为%d", player->GetName(), pChance->m_MaxPoint[playerIndex]);
	}
	else
	{
		pChance->m_MaxPoint[playerIndex] = 0xffff;
		rfalse(2, 1, "玩家%s放弃了摇号", player->GetName());
	}

	SAGroundItemChanceBack backMsg;
	backMsg.m_Choice = IsGiveup ? SAGroundItemChanceBack::SAGICB_GIVEUP : SAGroundItemChanceBack::SAGICB_GO;
	backMsg.m_Result = pChance->m_MaxPoint[playerIndex];
	backMsg.m_Index	 = index;
	g_StoreMessage(player->m_ClientIndex, &backMsg, sizeof(backMsg));
	
	// 所有人都完成了参与
	if (0 == --pChance->m_InitPeople)
	{
		FinishTakeChance(pChance, index);
		delete pChance;
	}

	return TRUE;
}

BOOL CGroundItemWinner::FinishTakeChance(ChangeItem *pChance, DWORD index)
{
	WORD	MaxPoint = 0;
	CPlayer *Owner	 = 0;

	// 处理完成，修正映射
	pChance->m_pSingleItem->m_ChanceItems.erase(index);
	m_ChanceItem.erase(index);

	for (size_t i = 0; i < CI_MAXPLAYER; i++)
	{
		if (!pChance->m_Player[i])
			continue;

		if (0xffff != pChance->m_MaxPoint[i] && pChance->m_MaxPoint[i] > MaxPoint)
		{
			MaxPoint = pChance->m_MaxPoint[i];
			Owner = pChance->m_Player[i];
		}
		MY_ASSERT(pChance->m_Player[i]->m_OwnedChangeItems.end() != pChance->m_Player[i]->m_OwnedChangeItems.find(index));
		pChance->m_Player[i]->m_OwnedChangeItems.erase(index);
	}
	if (!Owner || !Owner->PickSingleItem(*pChance->m_pSingleItem))/*Owner, pChance->m_pItem, pChance->m_pItem->m_IndexMask[pChance->m_Index]))*/
	{
		pChance->m_pSingleItem->m_PickStatus = PIPT_FREE;
	}
// 	if (!Owner || !Owner->StartPickSingleOne(Owner, pChance->m_pItem, pChance->m_pItem->m_IndexMask[pChance->m_Index]))
// 	{
// 		rfalse(2, 1, "无人中标，设置为自由，队伍内自由拾取！！");
// 		pChance->m_pItem->m_PickStatus[pChance->m_pItem->m_IndexMask[pChance->m_Index]] = PIPT_FREE;
// 	}

	return TRUE;
}

#pragma warning(pop)