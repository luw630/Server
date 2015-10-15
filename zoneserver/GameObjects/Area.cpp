#include "StdAfx.h"
#include "region.h"
#include "area.h"
#include "player.h"
#include "npc.h"
#include "monster.h"
#include "item.h"
#include "SingleItem.h"
#include "Pet.h"
#include "building.h"
#include "networkmodule\regionmsgs.h"
#include "networkmodule\itemmsgs.h"

check_list<LPIObject>::iterator CArea::tempIter;
check_list<LPIObject> *CArea::tempList = 0;

#define GETDIR(_dir, _x1, _y1, _x2, _y2)  if (_x2 > _x1) if (_y2 > _y1) _dir = 3; else if (_y2 < _y1) _dir = 1; else _dir = 2; \
	else if (_x2 < _x1) if (_y2 > _y1) _dir = 5; else if (_y2 < _y1) _dir = 7; else _dir = 6; \
	else if (_y2 > _y1) _dir = 4; else if (_y2 < _y1) _dir = 0; else _dir = -1;

extern bool g_newtype;

int CArea::OnCreate(_W64 long pParameter)
{
	// 初始化该地域的基本信息
	SInitParam *pParam = (SInitParam *)pParameter;
	if (!pParam) 
	{
		rfalse("SInitParam *pParam = (SInitParam *)pParameter;");
		return 0;
	}
	
	m_X = pParam->x;
	m_Y = pParam->y;
	m_ParentRegion = pParam->pParentRegion;

	return 1;
}

void CArea::OnClose()
{
	return;
}

void CArea::OnRun(void)
{
	return;
}

int CArea::AddObject(LPIObject pChild)
{
	if (!pChild)
		return 0;

	SASynObjectMsg msg;
	msg.dwGlobalID = pChild->GetGID();
	msg.dwExtra    = 0;

	CArea *lpPrevArea = 0;

	if (CPlayer *pPlayer = (CPlayer *)pChild->DynamicCast(IID_PLAYER))
	{
		if (pPlayer->m_ParentArea )
		{
			lpPrevArea = pPlayer->m_ParentArea;

// 			SADelObjectMsg msgDel;
// 			msgDel.dwGlobalID = pPlayer->GetGID();
// 			pPlayer->m_ParentArea->SendAdjWithDirection(&msgDel, sizeof(SADelObjectMsg), pPlayer->m_ClientIndex, this);	//发给Area内的别人，要求删除自己

		//	if ( pPlayer->m_ParentArea!= this )
			{				
				//1、发给自己，要求删除Area内的所有对象
 				pPlayer->m_ParentArea->SendDelObjectsWithDirection(pPlayer->m_ClientIndex, this);				

				//2、发给区域内其他玩家，要删除m_ClientIndex的对象
				SADelObjectMsg msgDel;
				msgDel.dwGlobalID = pPlayer->GetGID();
				pPlayer->m_ParentArea->SendDelWithDirection(&msgDel, sizeof(msgDel), pPlayer->m_ClientIndex, this);

				pPlayer->m_ParentArea->DelObject(pChild);				
			}
		}

		pPlayer->m_ParentArea   = this;
		pPlayer->m_ParentRegion = m_ParentRegion;

		// 将该玩家放到目标区块中
		m_PlayerList.push_back(pChild); 

		// 向有效区域发送该玩家的同步信息，除了他自己
		SendAdjWithDirection(&msg, sizeof(SASynObjectMsg), pPlayer->m_ClientIndex, lpPrevArea);

		// 将周围的对象同步给它
		SendAdjObjectsWithDirection(pPlayer->m_ClientIndex, pPlayer->m_ClientIndex, lpPrevArea);

		return 1;
	}

	if (CNpc *pNpc = (CNpc *)pChild->DynamicCast(IID_NPC))
	{
        if (pNpc->m_ParentArea)
		{
            lpPrevArea = pNpc->m_ParentArea;
			
			//1、发给区域内其他玩家，要删除m_ClientIndex的对象
			SADelObjectMsg msgDel;
			msgDel.dwGlobalID = pNpc->GetGID();
			pNpc->m_ParentArea->SendDelWithDirection(&msgDel, sizeof(msgDel), INVALID_DNID, this);

            pNpc->m_ParentArea->DelObject(pChild);
		}

		pNpc->m_ParentArea   = this;
		pNpc->m_ParentRegion = m_ParentRegion;

		m_NpcList.push_back(pChild);

        SendAdjWithDirection(&msg, sizeof(SASynObjectMsg), -1, lpPrevArea);

		return 1;
	}

	if (CFightPet *pPet = (CFightPet *)pChild->DynamicCast(IID_FIGHT_PET))
	{
		if (pPet->m_ParentArea)
		{
			lpPrevArea = pPet->m_ParentArea;
			pPet->m_ParentArea->DelObject(pChild);
		}

		pPet->m_ParentArea   = this;
		pPet->m_ParentRegion = m_ParentRegion;

		m_FightPetList.push_back(pChild);

		SendAdjWithDirection(&msg, sizeof(SASynObjectMsg), -1, lpPrevArea);

		return 1;
	}

	if (CMonster *pMonster = (CMonster *)pChild->DynamicCast(IID_MONSTER))
	{
		if (pMonster->m_ParentArea)
		{
            lpPrevArea	= pMonster->m_ParentArea;
			
			//1、发给区域内其他玩家，要删除m_ClientIndex的对象
			SADelObjectMsg msgDel;
			msgDel.dwGlobalID = pMonster->GetGID();
			pMonster->m_ParentArea->SendDelWithDirection(&msgDel, sizeof(msgDel), INVALID_DNID, this);

            pMonster->m_ParentArea->DelObject(pChild);
		}

		pMonster->m_ParentArea   = this;
		pMonster->m_ParentRegion = m_ParentRegion;

		m_MonsterList.push_back(pChild);

        SendAdjWithDirection(&msg, sizeof(SASynObjectMsg), -1, lpPrevArea);

		return 1;
	}

	if (CItem *pItem = (CItem*)pChild->DynamicCast(IID_ITEM))
	{
		if (g_InDebugMode)	// 包裹不应该有前任区域~
		{
			if (pItem->m_pParentArea)
			{
				rfalse(4, 1, "Area.cpp - AddOjbect() - pItem->m_pParentArea");
				return 0;
			}
			MY_ASSERT(!pItem->m_pParentArea);
		}

		pItem->m_pParentArea = this;

		m_ItemList.push_back(pChild);

        SendAdjWithDirection(&msg, sizeof(SASynObjectMsg), -1, lpPrevArea);

        return 1;
    }

	if (CSingleItem *pItem = (CSingleItem*)pChild->DynamicCast(IID_SINGLEITEM))
	{
		if (g_InDebugMode)	// 包裹不应该有前任区域~
		{
			if (pItem->m_pParentArea)
			{
				rfalse(4, 1, "Area.cpp - AddOjbect() - pItem->m_pParentArea");
				return 0;
			}
			MY_ASSERT(!pItem->m_pParentArea);
		}

		pItem->m_pParentArea = this;

		m_singleItemList.push_back(pChild);

		SendAdjWithDirection(&msg, sizeof(SASynObjectMsg), -1, lpPrevArea);

		return 1;
	}

	return 0;
}

int CArea::DelObject(LPIObject pChild)
{
    check_list<LPIObject> *targetList = 0;

	if (CPlayer *pPlayer = (CPlayer*)pChild->DynamicCast(IID_PLAYER))
	{
        if (pPlayer->m_ParentArea != this)
			return 0;

		pPlayer->m_ParentArea = 0;
        targetList = &m_PlayerList;
	}
	else if (CNpc *pNpc = (CNpc*)pChild->DynamicCast(IID_NPC))
	{
		if (pNpc->m_ParentArea != this)
			return 0;

		pNpc->m_ParentArea = 0;
        targetList = &m_NpcList;
	}
	else if (CFightPet *pPet = (CFightPet *)pChild->DynamicCast(IID_FIGHT_PET))
	{
		if (pPet->m_ParentArea != this)
			return 0;

		pPet->m_ParentArea = 0;
		targetList = &m_FightPetList;
	}
	else if (CMonster *pMonster = (CMonster*)pChild->DynamicCast(IID_MONSTER))
	{
		if (pMonster->m_ParentArea != this)
			return 0;
		
		pMonster->m_ParentArea = 0;
        targetList = &m_MonsterList;
	}
	else if (CItem *pItem = (CItem*)pChild->DynamicCast(IID_ITEM))
	{
		if (pItem->m_pParentArea != this)
			return 0;

		pItem->m_pParentArea = 0;
        targetList = &m_ItemList;
	}
	else if (CBuilding *pBuilding = (CBuilding*)pChild->DynamicCast(IID_BUILDING))
	{
		MY_ASSERT(0);
	}
	else if (CSingleItem *pItem = (CSingleItem*)pChild->DynamicCast(IID_SINGLEITEM))
	{
		if (pItem->m_pParentArea != this)
			return 0;

		pItem->m_pParentArea = 0;
		targetList = &m_singleItemList;
	}

	check_list<LPIObject>::iterator dest = std::find(targetList->begin(), targetList->end(), pChild);
	check_list<LPIObject>::iterator after = targetList->erase(dest);

    if (tempList && (tempList == targetList))
        tempIter = after;

    return 0;
}

void CArea::SendMe(struct SMessage *pMsg, DWORD dwSize, DNID dnidExcept)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Area.cpp - SendMe() - !pMsg");
		return;
	}

    if (!this) 
		return;

	if (m_PlayerList.size() == 0)
	{
		//rfalse(4,1,"");
		return;
	}
	check_list<LPIObject>::iterator it = m_PlayerList.begin();
	while (it != m_PlayerList.end())
	{
		CPlayer *pPlayer = (CPlayer*)(*it)->DynamicCast(IID_PLAYER);
		if (pPlayer && dnidExcept != pPlayer->m_ClientIndex)
			g_StoreMessage(pPlayer->m_ClientIndex, pMsg, (WORD)dwSize);

		it++;
	}
}

void CArea::SendAdj(struct SMessage *pMsg, DWORD dwSize, DNID dnidExcept)
{
	SendMe(pMsg, dwSize, dnidExcept);

	if (m_ParentRegion)
	{
		// up
		if (CArea *area = (CArea*)m_ParentRegion->GetArea(m_X, m_Y - 1)->DynamicCast(IID_AREA))
		{
			area->SendMe(pMsg, dwSize, dnidExcept);
		}

		// right up
		if (CArea *area = (CArea*)m_ParentRegion->GetArea(m_X + 1, m_Y - 1)->DynamicCast(IID_AREA))
		{
			area->SendMe(pMsg, dwSize, dnidExcept);
		}
		// right 
		if (CArea *area = (CArea*)m_ParentRegion->GetArea(m_X + 1, m_Y)->DynamicCast(IID_AREA))
		{
			area->SendMe(pMsg, dwSize, dnidExcept);
		}
		// right down
		if (CArea *area = (CArea*)m_ParentRegion->GetArea(m_X + 1, m_Y + 1)->DynamicCast(IID_AREA))
		{
			area->SendMe(pMsg, dwSize, dnidExcept);
		}
		// down
		if (CArea *area = (CArea*)m_ParentRegion->GetArea(m_X, m_Y + 1)->DynamicCast(IID_AREA))
		{
			area->SendMe(pMsg, dwSize, dnidExcept);
		}
		// down left
		if (CArea *area = (CArea*)m_ParentRegion->GetArea(m_X - 1, m_Y + 1)->DynamicCast(IID_AREA))
		{
			area->SendMe(pMsg, dwSize, dnidExcept);
		}
		//  left
		if (CArea *area = (CArea*)m_ParentRegion->GetArea(m_X - 1, m_Y)->DynamicCast(IID_AREA))
		{
			area->SendMe(pMsg, dwSize, dnidExcept);
		}
		// left up
		if (CArea *area = (CArea*)m_ParentRegion->GetArea(m_X - 1, m_Y - 1)->DynamicCast(IID_AREA))
		{
			area->SendMe(pMsg, dwSize, dnidExcept);
		}
	
// 		((CArea*)m_ParentRegion->GetArea(m_X, m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
// 		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
// 		// right
// 		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y).get())->SendMe(pMsg, dwSize, dnidExcept);
// 		// right down
// 		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
// 		// down
// 		((CArea*)m_ParentRegion->GetArea(m_X, m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
// 		// down left
// 		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
// 		// left
// 		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y).get())->SendMe(pMsg, dwSize, dnidExcept);
// 		// left up
// 		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
	}
}

void CArea::SendAdjWithDirection(struct SMessage *pMsg, DWORD dwSize, DNID dnidExcept, CArea *lpPrevArea)
{
	// 如果是新创建的对象或者是从别的场景来的对象，则同步所有的九宫格
	if (!lpPrevArea || lpPrevArea->m_ParentRegion != m_ParentRegion)
	{
		SendAdj(pMsg, dwSize, dnidExcept);
		return;
	}

	int iDirection = -1;

	int x = (int)lpPrevArea->m_X - (int)m_X;
	int y = (int)lpPrevArea->m_Y - (int)m_Y;

	if ((abs(x) <= 1) && (abs(y) <= 1))
	{
		GETDIR(iDirection, lpPrevArea->m_X, lpPrevArea->m_Y, m_X, m_Y);
	}

	SendMe(pMsg, dwSize, dnidExcept);

	switch (iDirection)
	{
	case 0:
		// 向上
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X,   m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
		break;

	case 1:
		// 向右上
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y  ).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
		break;

	case 2:
		// 向右
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y  ).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
		break;

	case 3:
		// 向右下
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y  ).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
		break;

	case 4:
		// 向下
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
		break;

	case 5:
		// 向左下
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y  ).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
		break;

	case 6:
		// 向左
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y  ).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
		break;

	case 7:
		// 向左上
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y  ).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidExcept);
		break;

	default:
		SendAdj(pMsg, dwSize, dnidExcept);
	}
}

void CArea::SendDel(DNID dnidMe)
{
	if(!this)
		return;

	//向自己发消息，要求删除区块内的所有玩家
	SADelObjectListMsg msg;

	DWORD totalSize = m_PlayerList.size() + m_NpcList.size() + m_MonsterList.size() + m_ItemList.size()+m_singleItemList.size() + m_FightPetList.size() + m_BuildingList.size();
	if(totalSize > sizeof(msg.dwGlobalIDs)/sizeof(msg.dwGlobalIDs[0]))
	{
		rfalse("人太多了吧？！需要删除的对象超过数量了！");
		return;
	}

	int i = 0;
	check_list<LPIObject>::iterator iter ;

	for(iter = m_PlayerList.begin(); iter != m_PlayerList.end(); iter++ ) 
	{
		CPlayer *pPlayer = (CPlayer*)(*iter)->DynamicCast(IID_PLAYER);
		if (pPlayer && dnidMe != pPlayer->m_ClientIndex)
		{
			msg.dwGlobalIDs[i++] = (*iter)->GetGID();
		}
	}
	for(iter = m_NpcList.begin(); iter != m_NpcList.end(); iter++ ) msg.dwGlobalIDs[i++] = (*iter)->GetGID();
	for(iter = m_MonsterList.begin(); iter != m_MonsterList.end(); iter++ ) msg.dwGlobalIDs[i++] = (*iter)->GetGID();
	for(iter = m_ItemList.begin(); iter != m_ItemList.end(); iter++ ) msg.dwGlobalIDs[i++] = (*iter)->GetGID();
	for(iter = m_singleItemList.begin(); iter != m_singleItemList.end(); iter++ ) msg.dwGlobalIDs[i++] = (*iter)->GetGID();
	for(iter = m_FightPetList.begin(); iter != m_FightPetList.end(); iter++ ) msg.dwGlobalIDs[i++] = (*iter)->GetGID();
	for(iter = m_BuildingList.begin(); iter != m_BuildingList.end(); iter++ ) msg.dwGlobalIDs[i++] = (*iter)->GetGID();

	if(i<1) return;

	msg.num = i;
	DWORD sendSize = sizeof(msg) - (sizeof(msg.dwGlobalIDs)/sizeof(msg.dwGlobalIDs[0]) - i) * sizeof(msg.dwGlobalIDs[0]);
	
	g_StoreMessage(dnidMe, &msg, sendSize);
}

void CArea::SendDelWithDirection(struct SMessage *pMsg, DWORD dwSize, DNID dnidMe, CArea *lpNewArea)
{
	if (!lpNewArea || lpNewArea->m_ParentRegion != m_ParentRegion)
	{
		return;
	}

	int iDirection = -1;

	int x = (int)lpNewArea->m_X - (int)m_X;
	int y = (int)lpNewArea->m_Y - (int)m_Y;

	if (x == 0 && y == 0)
	{
		return ;
	}

	if ((abs(x) <= 1) && (abs(y) <= 1))
	{
		GETDIR(iDirection, m_X, m_Y, lpNewArea->m_X, lpNewArea->m_Y);
	}

	switch (iDirection)
	{
	case 0:
		// 向上
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X,   m_Y+1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidMe);
		break;

	case 1:
		// 向右上
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X, m_Y+1  ).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidMe);
		break;

	case 2:
		// 向右
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y  ).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidMe);
		break;

	case 3:
		// 向右下
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y  ).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y-1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidMe);
		break;

	case 4:
		// 向下
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y-1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidMe);
		break;

	case 5:
		// 向左下
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y-1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y  ).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidMe);
		break;

	case 6:
		// 向左
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y  ).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidMe);
		break;

	case 7:
		// 向左上
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y  ).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y+1).get())->SendMe(pMsg, dwSize, dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMe(pMsg, dwSize, dnidMe);
		break;

	default:
		SendMe(pMsg, dwSize, dnidMe);
	}
}

void CArea::SendDelObjectsWithDirection(DNID dnidMe, CArea *lpNewArea)
{
	if (!lpNewArea || lpNewArea->m_ParentRegion != m_ParentRegion)
	{
		return;
	}

	int iDirection = -1;

	int x = (int)lpNewArea->m_X - (int)m_X;
	int y = (int)lpNewArea->m_Y - (int)m_Y;

	if (x == 0 && y == 0)
	{
		return ;
	}

	if ((abs(x) <= 1) && (abs(y) <= 1))
	{
		GETDIR(iDirection, m_X, m_Y, lpNewArea->m_X, lpNewArea->m_Y);
	}

	switch (iDirection)
	{
	case 0:
		// 向上
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X,   m_Y+1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendDel(dnidMe);
		break;

	case 1:
		// 向右上
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X, m_Y+1  ).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendDel(dnidMe);
		break;

	case 2:
		// 向右
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y  ).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendDel(dnidMe);
		break;

	case 3:
		// 向右下
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y  ).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y-1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendDel(dnidMe);
		break;

	case 4:
		// 向下
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y-1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendDel(dnidMe);
		break;

	case 5:
		// 向左下
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y-1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y  ).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendDel(dnidMe);
		break;

	case 6:
		// 向左
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y  ).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendDel(dnidMe);
		break;

	case 7:
		// 向左上
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y  ).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y+1).get())->SendDel(dnidMe);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendDel(dnidMe);
		break;

	default:
		SendDel(dnidMe);
	}
}

void CArea::SendMyState(DNID dnidClient, DNID dnidExcept)
{
    if (!this)
        return;

	// 发送玩家列表
	check_list<LPIObject>::iterator it = m_PlayerList.begin();
	while (it != m_PlayerList.end())
	{
		CPlayer *pPlayer = (CPlayer*)(*it)->DynamicCast(IID_PLAYER);
		if (pPlayer && dnidExcept != pPlayer->m_ClientIndex)
			g_StoreMessage(dnidClient, pPlayer->GetStateMsg(), sizeof(SASynPlayerMsg));

		it++;
	}

	// 发送NPC列表
	it = m_NpcList.begin();
	while (it != m_NpcList.end())
	{
		CNpc *pNpc = (CNpc*)(*it)->DynamicCast(IID_NPC);
		if (pNpc)
			g_StoreMessage(dnidClient, pNpc->GetStateMsg(), sizeof(SASynNpcMsg));

		it++;
	}

	// 发送活着的怪物列表
	it = m_MonsterList.begin();
	while (it != m_MonsterList.end())
	{
		CMonster *pMonster = (CMonster*)(*it)->DynamicCast(IID_MONSTER);
		if (pMonster && pMonster->m_CurHp > 0)
			g_StoreMessage(dnidClient, pMonster->GetStateMsg(), sizeof(SASynMonsterMsg));

		it++;
	}

	// 发送包裹列表
	it = m_ItemList.begin();
	while (it != m_ItemList.end())
	{
		CItem *pItem = (CItem*)(*it)->DynamicCast(IID_ITEM);
		if (pItem)
		    g_StoreMessage(dnidClient, pItem->GetStateMsg(), sizeof(SAAddGroundItemMsg));

		it++;
	}

	// 发送包裹列表
	it = m_singleItemList.begin();
	while (it != m_singleItemList.end())
	{
		CSingleItem *pItem = (CSingleItem*)(*it)->DynamicCast(IID_SINGLEITEM);
		if (pItem)
			g_StoreMessage(dnidClient, pItem->GetStateMsg(), sizeof(SAAddGroundItemMsg));

		it++;
	}
}

void CArea::SendAdjState(DNID dnidClient, DNID dnidExcept)
{
	SendMyState(dnidClient, dnidExcept);

	if (m_ParentRegion)
	{
		// up
		((CArea*)m_ParentRegion->GetArea(m_X, m_Y-1).get())->SendMyState(dnidClient, dnidExcept);
		// right up
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMyState(dnidClient, dnidExcept);
		// right
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y).get())->SendMyState(dnidClient, dnidExcept);
		// right down
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMyState(dnidClient, dnidExcept);
		// down
		((CArea*)m_ParentRegion->GetArea(m_X, m_Y+1).get())->SendMyState(dnidClient, dnidExcept);
		// down left
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMyState(dnidClient, dnidExcept);
		// left
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y).get())->SendMyState(dnidClient, dnidExcept);
		// left up
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMyState(dnidClient, dnidExcept);
	}
}

void CArea::SendMyObjects(DNID dnidClient, DNID dnidExcept)
{
	if (!this)
		return;

	SASynObjectMsg msg;
    msg.dwExtra = 0;

	// 发送玩家列表
	check_list<LPIObject>::iterator it = m_PlayerList.begin();
	while (it != m_PlayerList.end())
	{
		CPlayer *pPlayer = (CPlayer*)(*it)->DynamicCast(IID_PLAYER);
		if (pPlayer && dnidExcept != pPlayer->m_ClientIndex)
		{
			msg.dwGlobalID = pPlayer->GetGID();

            if (!g_newtype)
			    g_StoreMessage(dnidClient, &msg, sizeof(SASynObjectMsg));
            else
                g_StoreMessage(dnidClient, pPlayer->GetStateMsg(), sizeof(SASynPlayerMsg));
        }

		it++;
	}

	// 发送NPC列表
	it = m_NpcList.begin();
	while (it != m_NpcList.end())
	{
		CNpc *pNpc = (CNpc*)(*it)->DynamicCast(IID_NPC);
		if (pNpc)
		{
			msg.dwGlobalID = pNpc->GetGID();

            if (!g_newtype)
			    g_StoreMessage(dnidClient, &msg, sizeof(SASynObjectMsg));
            else
                g_StoreMessage(dnidClient, pNpc->GetStateMsg(), sizeof(SASynNpcMsg));
		}

		it++;
	}

	// 发送宠物列表
	it = m_FightPetList.begin();
	while (it != m_FightPetList.end())
	{
		CFightPet *pPet = (CFightPet*)(*it)->DynamicCast(IID_FIGHT_PET);
		if (pPet)
		{
			msg.dwGlobalID = pPet->GetGID();

			if (!g_newtype)
				g_StoreMessage(dnidClient, &msg, sizeof(SASynObjectMsg));
			else
				g_StoreMessage(dnidClient, pPet->GetStateMsg(), sizeof(SASynFightPetMsg));
		}

		it++;
	}

	// 发送Item列表
// 	it = m_ItemList.begin();
// 	while (it != m_ItemList.end())
// 	{
// 		CItem *pItem = (CItem*)(*it)->DynamicCast(IID_ITEM);
// 		if (pItem)
//         {
// 			msg.dwGlobalID = pItem->GetGID();
//             
//             if (!g_newtype)
// 			    g_StoreMessage(dnidClient, &msg, sizeof(SASynObjectMsg));
//             else
//                 g_StoreMessage(dnidClient, pItem->GetStateMsg(), sizeof(SAAddGroundItemMsg));
//         }
// 
// 		it++;
// 	}

	it = m_singleItemList.begin();
	while (it != m_singleItemList.end())
	{
		CSingleItem *pItem = (CSingleItem*)(*it)->DynamicCast(IID_SINGLEITEM);
		if (pItem)
		{
			msg.dwGlobalID = pItem->GetGID();

			if (!g_newtype)
				g_StoreMessage(dnidClient, &msg, sizeof(SASynObjectMsg));
			else
				g_StoreMessage(dnidClient, pItem->GetStateMsg(), sizeof(SAAddGroundItemMsg));
		}

		it++;
	}



	// 发送活着的Monster列表
	it = m_MonsterList.begin();
	while (it != m_MonsterList.end())
	{
		CMonster *pMonster = (CMonster*)(*it)->DynamicCast(IID_MONSTER);
        if (pMonster && pMonster->m_CurHp > 0)
        {
			msg.dwGlobalID = pMonster->GetGID();

            if (!g_newtype)
			    g_StoreMessage(dnidClient, &msg, sizeof(SASynObjectMsg));
            else
                g_StoreMessage(dnidClient, pMonster->GetStateMsg(), sizeof(SASynMonsterMsg));
        }

		it++;
	}
}

void CArea::SendAdjObjects(DNID dnidClient, DNID dnidExcept)
{
	SendMyObjects(dnidClient, dnidExcept);

	if (m_ParentRegion)
	{
		// up
		((CArea*)m_ParentRegion->GetArea(m_X, m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
		// right up
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
		// right
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y).get())->SendMyObjects(dnidClient, dnidExcept);
		// right down
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
		// down
		((CArea*)m_ParentRegion->GetArea(m_X, m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
		// down left
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
		// left
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y).get())->SendMyObjects(dnidClient, dnidExcept);
		// left up
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
	}
}


void CArea::SendAdjObjectsWithDirection(DNID dnidClient, DNID dnidExcept, CArea *lpPrevArea)
{
	// 如果以前的所属区域为空，则表示是新创建的，那么就全部同步
	// 如果来自不同的场景，也需要全部同步
	if (!lpPrevArea || (lpPrevArea->m_ParentRegion != m_ParentRegion))
    {
        SendAdjObjects(dnidClient, dnidExcept);
        return;
    }

	// 如果原区域和目标区域相差为1，则只需要更新局部
	int iDirection = -1;

	int x = abs((int)lpPrevArea->m_X - (int)m_X);
    int y = abs((int)lpPrevArea->m_Y - (int)m_Y);

	if (x == 0 && y == 0)
	{
		return;
	}

	if (abs(x) <= 1 && abs(y) <= 1)
    {
        GETDIR(iDirection, lpPrevArea->m_X, lpPrevArea->m_Y, m_X, m_Y);
    }

    switch (iDirection)
    {
    case 0:
        // 向上
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X,   m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
        break;    

    case 1:                                           
        // 向右上                                     
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y  ).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
        break;     

    case 2:                                           
        // 向右                                       
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y  ).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
        break;     

    case 3:                                           
        // 向右下                                     
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y  ).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
        break;     

    case 4:                                           
        // 向下                                       
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
        break;     

    case 5:                                           
        // 向左下                                     
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y  ).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
        break;       

    case 6:                                           
        // 向左                                       
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y  ).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
        break;      

    case 7:                                           
        // 向左上                                     
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y+1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y  ).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X-1, m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X  , m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
		((CArea*)m_ParentRegion->GetArea(m_X+1, m_Y-1).get())->SendMyObjects(dnidClient, dnidExcept);
        break;

    default:
        SendAdjObjects(dnidClient, dnidExcept);
    }
}
