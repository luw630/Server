#include "StdAfx.h"
#include "scriptobject.h"
#include "Trigger.h"
#include "Npc.h"
#include "Player.h"
#include "Monster.h"
#include "area.h"
#include "ScriptManager.h"
#include "Item.h"
#include "region.h"
#include "building.h"

#include "GlobalFunctions.h"

#include "NetworkModule/ScriptMsgs.h"
#include "Networkmodule\ItemMsgs.h"
#include "Networkmodule\ChatMsgs.h"
#include "DSpcialItemData.h"
#include "LITESERIALIZER/Lite.h"
#include "pub/rpcop.h"
#include "networkmodule/RefreshMsgs.h"
#include "pub/traceinfo.h"
#include "pub/lite_lualibrary.hpp"


extern BOOL PutPlayerIntoDestRegion(CPlayer *pPlayer, WORD wNewRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID = 0);
extern LPIObject GetPlayerByName(LPCSTR szName);
extern void g_AddSpendMoney( __int64 iSpendMoney );


CScriptObject::CScriptObject(void)
{
}

CScriptObject::~CScriptObject(void)
{
}

DWORD CScriptObject::OnTriggerActivated(const CTrigger *pTrigger)
{
	if (!pTrigger)
		return 0;

	// 触发器触发可能是用脚本操作，也可能直接使用程序代码操作
	switch (pTrigger->m_dwType)
	{
	case CTrigger::TT_CLICK_ON_NPC:
		if (pTrigger->SClickOnNpc.pPlayer && pTrigger->SClickOnNpc.pNpc && pTrigger->SClickOnNpc.pNpc->m_Property.m_wClickScriptID > 0)
		{
			// 脚本的处理
			g_Script.SetCondition(0, pTrigger->SClickOnNpc.pPlayer, pTrigger->SClickOnNpc.pNpc);
			g_Script.CallFunc("ClickNpcCallBack", pTrigger->SClickOnNpc.pNpc->m_Property.m_wClickScriptID);
			g_Script.CleanCondition();
		}
		break;

		/*
		case CTrigger::TT_CHANGE_REGION:
		// 目前只能对玩家进行这个操作，因为玩家和其他的对象不一样，玩家是单独的一个管理链表于场景结构之外
		return PutPlayerIntoDestRegion((CPlayer*)this, pTrigger->SChangeRegion.wNewRegionID, 

		pTrigger->SChangeRegion.wStartX, pTrigger->SChangeRegion.wStartY,pTrigger->SChangeRegion.dwNewRegionGID);
		break;

		case CTrigger::TT_MONSTER_HALF_HP:
		if (pTrigger->SMonsterHalfHP.pMonster)
		if (pTrigger->SMonsterHalfHP.dwScriptID)
		{
		g_Script.SetCondition(pTrigger->SMonsterHalfHP.pMonster, NULL, NULL);
		g_Script.TriggerEvent("Monster_AI", pTrigger->SMonsterHalfHP.dwScriptID);
		g_Script.CleanCondition(); 
		}
		break;

		case CTrigger::TT_MONSTER_NEWLIFE:
		if (pTrigger->SMonsterNewLife.pMonster)
		if (pTrigger->SMonsterNewLife.dwScriptID)
		{
		g_Script.SetCondition(pTrigger->SMonsterNewLife.pMonster, NULL, NULL);
		g_Script.TriggerEvent("Monster_Refresh", pTrigger->SMonsterNewLife.dwScriptID);
		g_Script.CleanCondition(); 
		}
		break;

		case CTrigger::TT_MONSTER_AI:
		if(pTrigger->SMonsterAI.pMonster)
		if(pTrigger->SMonsterAI.pPlayer)
		if(pTrigger->SMonsterAI.dwScriptID)
		{
		g_Script.SetCondition(pTrigger->SMonsterAI.pMonster, pTrigger->SMonsterAI.pPlayer, NULL);
		g_Script.DoFunc("OnClick", (WORD)pTrigger->SMonsterAI.dwScriptID);
		g_Script.CleanCondition(); 
		}
		break;

		case CTrigger::TT_MONSTERDEAD:
		if(pTrigger->SMonsterDead.pMonster)
		if(pTrigger->SMonsterDead.pPlayer)
		if(pTrigger->SMonsterDead.dwScriptID)
		{
		g_Script.SetCondition(pTrigger->SMonsterDead.pMonster, pTrigger->SMonsterDead.pPlayer, NULL);
		g_Script.DoFunc("OnMonsterDead", (WORD)pTrigger->SMonsterDead.dwScriptID);
		g_Script.CleanCondition(); 
		}
		break;

		case CTrigger::TT_MAPTRAP_EVENT:
		if (pTrigger->SMapEvent.dwScriptID != 0)
		{
		CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
		CMonster *pMonster = (CMonster *)DynamicCast(IID_MONSTER);
		CNpc *pNpc = (CNpc *)DynamicCast(IID_NPC);

		g_Script.SetCondition(pMonster, pPlayer, pNpc);
		if ( pTrigger->SMapEvent._variant == 0 )
		g_Script.DoFunc("OnMapEvent", (WORD)pTrigger->SMapEvent.dwScriptID);
		else
		{
		char szFileName[64] = { 0 };
		sprintf(szFileName, "OnMapEvent%d", pTrigger->SMapEvent.dwScriptID);
		g_Script.PrepareFunction( szFileName );
		g_Script.PushParameter( pTrigger->SMapEvent.GetVariant() );
		g_Script.Execute();
		}
		g_Script.CleanCondition(); 
		}
		break;
		case CTrigger::TT_MAPTRAP_BUILDING:
		if ( m_ParentRegion && ( pTrigger->SMapEvent_Building.buildingId != 0 ) )
		{
		CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
		CMonster *pMonster = (CMonster *)DynamicCast(IID_MONSTER);
		CNpc *pNpc = (CNpc *)DynamicCast(IID_NPC);

		std::map< QWORD, LPIObject >::iterator it = 
		m_ParentRegion->m_BuildingMap.find( pTrigger->SMapEvent_Building.buildingId );
		if ( it != m_ParentRegion->m_BuildingMap.end() )
		{
		CBuilding *building = ( CBuilding* )it->second->DynamicCast( IID_BUILDING );
		if ( building == NULL )
		return FALSE;

		g_Script.SetCondition( pMonster, pPlayer, pNpc, building );

		int scriptId = 0;
		if ( building->GetInteger( "trapScriptId", scriptId ) )
		g_Script.DoFunc( "OnMapEvent_Building", (WORD)scriptId );

		// 这里的 scriptId 实际是用来存字符串大小的！
		else if ( LPCSTR trapScript = building->GetString( "trapScript", ( size_t* )&scriptId ) )
		g_Script.DoBuffer( trapScript, scriptId ); 

		else if ( LPCSTR trapSFunc = building->GetString( "trapSFunc" ) )
		g_Script.PrepareFunction( trapSFunc ), g_Script.Execute();

		g_Script.CleanCondition(); 
		}
		}
		break;
		case CTrigger::TT_CLICK_ON_PLAYER:
		if(pTrigger->SClickOnPlayer.pPlayer)
		if(pTrigger->SClickOnPlayer.dwScriptID != 0)
		{
		g_Script.SetCondition(NULL, pTrigger->SClickOnPlayer.pPlayer, NULL);
		g_Script.DoFunc("ClickPlayer", pTrigger->SClickOnPlayer.dwScriptID, pTrigger->SClickOnPlayer.dwPPlayerID );
		g_Script.CleanCondition(); 
		}
		break;
		case CTrigger::TT_PLAYERDEAD:
		if ( pTrigger->SMonsterDead.pPlayer )
		{
		g_Script.SetCondition( NULL, pTrigger->SMonsterDead.pPlayer, NULL );
		g_Script.CallFunc( "OnPlayerDead", pTrigger->SMonsterDead.dwScriptID );
		g_Script.CleanCondition(); 
		}
		break;
		case CTrigger::TT_CLICK_ON_BUILDING:
		if ( pTrigger->SClickOnBuilding.pPlayer && pTrigger->SClickOnBuilding.pBuilding )
		{
		CBuilding *building = pTrigger->SClickOnBuilding.pBuilding;
		CPlayer *player = pTrigger->SClickOnBuilding.pPlayer;

		g_Script.SetCondition( NULL, player, NULL, building );

		int clickId = 0;
		if ( building->GetInteger( "clickId", clickId ) )
		g_Script.DoFunc( "OnClickBuilding", (WORD)clickId );

		else if ( LPCSTR clickScript = building->GetString( "clickScript", ( size_t* )&clickId ) )
		g_Script.DoBuffer( clickScript, clickId ); 

		else if ( LPCSTR clickSFunc = building->GetString( "clickSFunc" ) )
		g_Script.PrepareFunction( clickSFunc ), g_Script.Execute();

		g_Script.CleanCondition(); 
		}
		break;
		case CTrigger::TT_MONSTER_DROP_ITEM:
		if(pTrigger->SMonster_DropItem.pMonster)
		if(pTrigger->SMonster_DropItem.pPlayer)
		if(pTrigger->SMonster_DropItem.dwDropItemID)
		{
		g_Script.SetCondition(pTrigger->SMonster_DropItem.pMonster, pTrigger->SMonster_DropItem.pPlayer, NULL);
		g_Script.DoFunc("OnMonsterDropItem", (WORD)pTrigger->SMonster_DropItem.dwDropItemID);
		g_Script.CleanCondition(); 
		}
		break;
		case CTrigger::TT_USEITEM_ON_NPC:
		if(pTrigger->SUseItemOnNpc.pNpc)
		if(pTrigger->SUseItemOnNpc.pPlayer)
		if(pTrigger->SUseItemOnNpc.dwItemID)
		{
		g_Script.SetCondition(NULL, pTrigger->SUseItemOnNpc.pPlayer, pTrigger->SUseItemOnNpc.pNpc);
		g_Script.DoFunc("OnUseItemOnNPC", pTrigger->SUseItemOnNpc.pNpc->m_Property.m_wClickScriptID,pTrigger->SUseItemOnNpc.dwItemID);
		g_Script.CleanCondition(); 
		}
		break;
		case CTrigger::TT_USEITEM_ON_MONSTER:
		if(pTrigger->SUseItemOnMonster.pMonster)
		if(pTrigger->SUseItemOnMonster.pPlayer)
		if(pTrigger->SUseItemOnMonster.dwItemID)
		{
		g_Script.SetCondition(pTrigger->SUseItemOnMonster.pMonster, pTrigger->SUseItemOnMonster.pPlayer, NULL);
		g_Script.DoFunc("OnCaptureMonster", 30000, pTrigger->SUseItemOnMonster.dwItemID);
		g_Script.CleanCondition(); 
		}
		break;
		//*/
	}
	return 0;
}

void CScriptObject::GetForgeData(int &nGoodsID, int &nDrinkNum, int &nSuccess, DWORD &nMoney)
{
	if (CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER))
	{
		g_Script.SetCondition(NULL, pPlayer, NULL);
		g_Script.CallFunc("R_Success");

		nMoney = (int)g_Script.GetRet(0);
		nSuccess = g_Script.GetRet(1);
		nDrinkNum = g_Script.GetRet(2);
		nGoodsID = g_Script.GetRet(3);

		g_Script.CleanCondition();
	}
}

void CScriptObject::OnClickMenu(SQChoseMenuMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4,1,"CScriptObject::OnClickMenu");
		return;
	}
	if (CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER)) 
	{
		CNpc *curNpc = pPlayer->GetCurrentNpc();
		if (!curNpc)
			return;

		g_Script.SetCondition(0, pPlayer, curNpc);

		if ((pPlayer->temporaryVerifier != pMsg->verifier)) 
		{
			g_Script.CleanCondition();
			return;
		}

		//pPlayer->temporaryVerifier = 0;

		// 获取即将调用的函数名，并做调用准备
		//g_Script.CallFunc(FormatString("ClickMenu%d", curNpc->m_Property.m_wClickScriptID),CScriptValue(pMsg->byChoseID));
		g_Script.PrepareFunction(FormatString("ClickMenu%d", curNpc->m_Property.m_wClickScriptID));
		g_Script.PushParameter(pMsg->byChoseID);
		g_Script.Execute();
		g_Script.CleanCondition();
	}
}

void CScriptObject::OnPlayCG(SQPlayCG *pMsg)
{
	if (!pMsg)
	{
		rfalse(4,1,"CScriptObject::OnPlayCG");
		return;
	}
	if (CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER)) 
	{
		WORD wCgID = pMsg->wCgID;

		g_Script.SetCondition(0, pPlayer, 0);
		g_Script.PrepareFunction(FormatString("PlayCgEnd_%d", wCgID));
		g_Script.Execute();
		g_Script.CleanCondition();
	}
}

void CScriptObject::SendShowTaskMsg(WORD wTaskID, DWORD wState, BYTE byOperate, BYTE taskType )
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if(pPlayer == NULL)
		return;

	SAShowTaskInfoMsg msg;
	msg.wTaskID = wTaskID;
	msg.wTaskState = wState;
	msg.byOperate = byOperate;
	msg.byTaskType = taskType;

	g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAShowTaskInfoMsg));
}

// 炼制、融合相关函数
BOOL CScriptObject::SetForgeData(SPackageItem *pItem)
{
	return FALSE;
}

BOOL CScriptObject::SetFuseData(SPackageItem *pItem)
{
	return TRUE;
}

BOOL CScriptObject::DelItemByFlag()
{
	//     CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	//     if(pPlayer == NULL)
	//         return FALSE;
	// 
	//     SPackageItem *pItem = NULL;
	//     siit it = m_RecvItemList.begin();
	//     for(; it != m_RecvItemList.end(); )
	//     {
	//         pItem = pPlayer->FindItemByPos( (BYTE)it->wPosX, (BYTE)it->wPosY );
	//         if(pItem && (pItem->wIndex == it->wItemID))
	//             pPlayer->DelItem( *pItem, "DelItemByFlag" );
	// 
	//         it++;
	//     }
	// 
	//     ClearRItemInfo();

	return TRUE;
}

BOOL CScriptObject::RestoreItemByFlag(WORD wPosX, WORD wPosY, WORD wID)
{
	//     CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	//     if(pPlayer == NULL)
	//         return FALSE;
	// 
	//     SPackageItem *pItem = pPlayer->FindItemByPos( (BYTE)wPosX, (BYTE)wPosY );
	//     if(pItem)
	//     {
	//         if(wID == pItem->wIndex)
	//         {
	//             SAAddPackageItemMsg msg;
	// 	        msg.stItem = *pItem;
	// 	        g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAAddPackageItemMsg));
	//             
	//             return TRUE;
	//         }
	//     }

	return FALSE;

}

void CScriptObject::RestoreAllItemByFlag(void)
{
	//     siit it = m_RecvItemList.begin();
	//     for(; it != m_RecvItemList.end(); )
	//     {      
	//         RestoreItemByFlag(it->wPosX, it->wPosY, it->wItemID);    
	//         it++;
	//     }
	// 
	//     ClearRItemInfo();
}

bool CScriptObject::CheckSOneItemFlag(WORD wID)
{
	//     siit it, it_e;
	//     GetSIITBegin(it);
	//     GetSIITEnd(it_e);
	//     for(; it != it_e; )
	//     {
	// 	    if (wID == it->wItemID)
	//         {   
	//             return true;
	//         }
	//         it++;
	//     }

	return false;
}

WORD CScriptObject::GetSOneItemNum(WORD wID)
{
	/*
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if(pPlayer == NULL)
	return 0;

	WORD wCount = 0;

	siit it, it_e;
	GetSIITBegin(it);
	GetSIITEnd(it_e);
	SPackageItem *pItem = NULL;

	for(; it != it_e; )
	{
	if (wID == it->wItemID)
	{
	pItem = pPlayer->FindItemByPos( (BYTE)it->wPosX, (BYTE)it->wPosY );
	if(pItem)
	{
	if(pItem->byType == OVERLAP)
	wCount += ((SOverlap *)(SItemBase *)pItem)->byQuantity;
	else
	wCount += 1;
	}
	}
	it++;
	}

	return wCount;
	*/
	return 0;
}

void CScriptObject::RecvSRecvItemInfo(SQSendSItemInfoMsg *pMsg)
{
	//     if(pMsg == NULL)
	//         return;
	// 
	//     if(pMsg->type == SQSendSItemInfoMsg::ST_CHECKITEM)
	//     {
	//         if(m_RecvItemList.size() >= 2)
	//         {
	//             return;
	//         }
	//     }
	// 
	//     SITEMINFO info;
	//     info = pMsg->info;
	//     if(!CheckSOneItemFlag(info))
	//         AddRecvItemInfo(info);
}



void CScriptObject::OnClickForgeMenu(SQForgeGoodsMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4,1,"CScriptObject::OnClickForgeMenu");
		return;
	}
	if (CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER))
	{
		g_Script.SetCondition(NULL, pPlayer, NULL); 
		switch(pMsg->byWhat)
		{
		case SQForgeGoodsMsg::SP_FORGE:          // 炼制
			{
				if(pMsg->wSuccess == 1)
					g_Script.CallFunc("Touch_Forge"); 
				else if(pMsg->wSuccess == 0)
					g_Script.CallFunc("R_Cancel");
			}break;
		case SQForgeGoodsMsg::SP_FUSE:         // 融合
			{
				if(pMsg->wSuccess == 1)
					g_Script.CallFunc("Touch_Fuse");
				else if(pMsg->wSuccess == 0)
					g_Script.CallFunc("R_LostFuse");
			}break;
		case SQForgeGoodsMsg::SP_FAME:        // 名望框 
			{
				g_Script.CallFunc("GetMoneyFromPanel", pMsg->wSuccess);

			}break;
		case SQForgeGoodsMsg::SP_MONEY:
			{
				//   pPlayer->SetRecvMoney(pMsg->wSuccess);

			}break;
		case SQForgeGoodsMsg::SP_SITEMS:
			{
				g_Script.CallFunc("Touch_SItems", pMsg->wSuccess); 

			}break;
		}
		g_Script.CleanCondition();
	}
}

void PrintScriptVariables(LPCSTR name)
{
}

/*
void CScriptObject::LostItems(WORD wID, DWORD dwLife, DWORD ProtectedGID, WORD ProtectedTime, WORD wRange, DWORD dwMoney)
{
if (m_ParentRegion == NULL)
return;

SGroundItem item;
memset(&item, 0, sizeof(item));

int nNum = 0;

CItem::SParameter param;
param.dwProtectedGID = ProtectedGID;
param.ProtectTeamID = 0;
param.dwLife = dwLife;
nNum = rand()%wRange;
param.xTile = (rand()%1 == 0) ? (m_wCurX + nNum): (m_wCurX - nNum);
nNum = rand()%wRange;
param.yTile = (rand()%1 == 0) ? (m_wCurY + nNum): (m_wCurY - nNum);
item.wIndex = wID;
param.pItem = &item;

SetItemTypeData(&item);

if(item.byType == MONEY) {
((SMoney *)(SItemBase *)&item)->dwNumber = dwMoney;    
}

LPDWORD pdwTiles = m_ParentRegion->m_pdwTiles;
WORD	rw = m_ParentRegion->m_RegionW;
WORD	rh = m_ParentRegion->m_RegionH;

if( param.xTile >= rw || param.yTile >= rh )    return; 

if( !(pdwTiles[param.yTile*rw+param.xTile]&TILETYPE_MOVE) ) {
param.yTile = m_wCurY;
param.xTile = m_wCurX;
}

if (m_ParentRegion->Move2Area(m_ParentRegion->GenerateObject(IID_ITEM, 0, (LPARAM)&param), param.xTile, param.yTile, true))
{
//rfalse("Move2Area");
// 记录产生道具
g_pSpecialItemData->OnCreatedItem(wID, 1);
}
}
*/

/*
static BOOL CALLBACK s_net_operation_end_callback_killinfo(const StringKey32 &szName, KILLINFO *pObject, CKillInfoManager *pKillInfoManager)
{
StringKey32 Name = szName;
if (pObject != NULL)
{
pKillInfoManager->m_KillInfoMap.AddObject(Name, *pObject);
}
else
{
pKillInfoManager->m_KillInfoMap.DelObject(Name);
}

return TRUE;
}

CKillInfoManager::CKillInfoManager()
{
m_OrbUser.SetNetOpEndCallback((tForceReadMostlyOrbUser<KILLINFO>::NET_OPEND_CALLBACK)
s_net_operation_end_callback_killinfo, (LPARAM)this);
}

DWORD CKillInfoManager::GetKillListNumber()
{
return m_KillInfoMap.GetSize();
}

static int index = 0;
DWORD CALLBACK GetPkInfoCB(const StringKey32 &szName, KILLINFO *pObject, LPARAM lParam)
{
SASendKillInfoMsg *pMsg = (SASendKillInfoMsg *)lParam;

pMsg->stKillInfo[index ++] = *pObject;

if (index >= sizeof(pMsg->stKillInfo) / sizeof(KILLINFO))
return TCR_BREAK;

return 0;
}

void CKillInfoManager::GetKillListAllInfo(SASendKillInfoMsg *pMsg)
{
index = 0;
m_KillInfoMap.TraversalObjects((WPARAM)GetPkInfoCB, (LPARAM)pMsg);
}

DWORD CALLBACK GetLowerInfoCB(const StringKey32 &szName, KILLINFO *pObject, LPARAM lParam)
{
KILLINFO *pInfo = (KILLINFO *)lParam;

if (pInfo->dwValue > pObject->dwValue)
*pInfo = *pObject;

return 0;
}

KILLINFO CKillInfoManager::GetLowerInfo()
{
KILLINFO Info;
Info.dwValue = 0xffffffff;
Info.szName[0] = 0;

m_KillInfoMap.TraversalObjects((WPARAM)GetLowerInfoCB, (LPARAM)&Info);

return Info;
}

bool CKillInfoManager::AddKillInfo(KILLINFO &stInfo)
{
if (m_KillInfoMap.LocateObject(StringKey32(stInfo.szName)) != NULL)
return false;

if (GetKillListNumber() >= 10) 
{
KILLINFO Info = GetLowerInfo();

if (stInfo.dwValue < Info.dwValue + 1000)
return false;

if (!DelKillInfo(Info.szName))
return false;

TalkToAll(FormatString("悬赏[%d]两银子追杀[%s]的榜文已经无效了", Info.dwValue, Info.szName));
}

if (!m_OrbUser.RequestCreateObject(stInfo.szName, stInfo))
return false;

return true;
}

bool CKillInfoManager::DelKillInfo(const char *name)
{
if (!m_OrbUser.RequestDeleteObject(name))
return false;

return true;
}

tForceReadMostlyOrbUser<KILLINFO>::roo_ptr CKillInfoManager::GetKillInfoInCache(const char *name)
{
return m_OrbUser.RequestReadOnlyObject(name);
}

tForceReadMostlyOrbUser<KILLINFO>::rwo_ptr CKillInfoManager::GetKillInfo(const char *name)
{
return m_OrbUser.RequestReadWriteObject(name);
}

void CScriptObject::RecvKillInfoMessage(SQSendKillInfoMsg *pMsg)
{
CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
if(pPlayer == NULL)
return;

if(pPlayer->m_nMoney < pMsg->dwMoney) 
{
pPlayer->SendErrorMsg(SABackMsg::EXM_MONEY_NOT_ENOUGH);
return;
}

BOOL IsValidName(LPCSTR);
if (!IsValidName(pMsg->szName))
{
pPlayer->SendErrorMsg(SABackMsg::EXM_MONEY_NOT_ENOUGH);
return;
}

//LPIObject pObject = GetPlayerByName(pMsg->szName);

//CPlayer *pDestPlayer = (CPlayer*)pObject->DynamicCast(IID_PLAYER);

//if (pDestPlayer == NULL) {
//    TalkToDnid(pPlayer->m_ClientIndex, "此名称的玩家不在线!");
//    return;
//}

KILLINFO Info;
ZeroMemory(&Info, sizeof(KILLINFO));
Info.dwValue = pMsg->dwMoney;
dwt::strcpy(Info.szName, pMsg->szName, CONST_USERNAME);
if (Info.dwValue < 10000)
{
TalkToDnid(pPlayer->m_ClientIndex, "这年头想找人出头，怎么也要一万两银子吧？！");
}
else if (g_KillInfoManager.AddKillInfo(Info))
{
//SRoleTask task;
//ZeroMemory(&task, sizeof(SRoleTask));
//task.wTaskID = ETT_BYKILLER;
//task.bSave = true;

//pDestPlayer->AddTask(task);
pPlayer->m_nMoney -= pMsg->dwMoney;
g_AddSpendMoney(pMsg->dwMoney);
pPlayer->SendMoneyInfo(pPlayer->m_ClientIndex, pPlayer->m_nMoney);

TalkToAll(FormatString("[%s]悬赏[%d]两银子追杀[%s]", pPlayer->GetName(), pMsg->dwMoney ,pMsg->szName));
// 可能是调用一个脚本函数继续处理 也许是直接发送操作请求
} 
else
{
TalkToDnid(pPlayer->m_ClientIndex, "数量已满，不能加榜，但是如果你出足够多的银子……");
}
}
*/

void CScriptObject::RecvOpenScoreList(struct SQUpdateScoreListMsg *pMsg)
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if ( pPlayer == NULL )
		return;

	if ( pMsg->commandType == SQUpdateScoreListMsg::QUERY_TITLE )
	{
		g_Script.SetCondition( NULL, pPlayer, NULL );
		g_Script.CallFunc("OpenScorewnd"); 
		//	else if(pMsg->wSuccess == 0)
		//		g_Script.CallFunc("R_Cancel");
		g_Script.CleanCondition();

		return;
	}
	else if ( pMsg->commandType == SQUpdateScoreListMsg::UPDATE_APPRAISE )
	{
		g_Script.SetCondition( NULL, pPlayer, NULL );
		LuaFunctor( g_Script, "OnPlayeAppraise" )[ pMsg->param ]();
		g_Script.CleanCondition();
		return ;
	}
	else if ( pMsg->commandType == SQUpdateScoreListMsg::SCORE_AWARD )
	{
		g_Script.SetCondition( NULL, pPlayer, NULL );
		LuaFunctor( g_Script, "OnPlayeScoreAward" )();
		g_Script.CleanCondition();
		return ;
	}

	try
	{
		SQGameServerRPCOPMsg rpcmsg;
		rpcmsg.dstDatabase = SQGameServerRPCOPMsg::CHARACTER_DATABASE;
		int tmppageindex = -1;

		lite::Serializer slmaker( rpcmsg.streamData, sizeof( rpcmsg.streamData) );

		slmaker

			// 注册出现异常时返回的数据（集）
			[OP_BEGIN_ERRHANDLE] ( 0 ) ("p_GetTopList") [OP_END_ERRHANDLE]

		// 准备存储过程
		[OP_PREPARE_STOREDPROC] ("p_GetTopList")

			// 设定调用参数
			[OP_BEGIN_PARAMS]
		(1) ( pMsg->param )
			(2) ( ( int )( pMsg->commandType == SQUpdateScoreListMsg::QUERY_SCORE ? pPlayer->GetSID() : pMsg->page ) )
			(3) ( ( int )pMsg->commandType );

		if ( pMsg->commandType == SQUpdateScoreListMsg::SCORE_END + 1 )
			slmaker (4) ( ( ( LPCSTR )&pMsg->page ) + 4 );

		slmaker

			[OP_END_PARAMS]

		// 调用存储过程
		[OP_CALL_STOREDPROC]

		// 初始化返回数据盒
		[OP_INIT_RETBOX]    (1024)

			// 数据盒的第一个值
			[OP_BOX_VARIANT]    ( (int)SMessage::EPRO_SCRIPT_MESSAGE )
			[OP_BOX_VARIANT]    ( (int)SScriptBaseMsg::EPRO_SCORE_LIST )
			[OP_BOX_VARIANT]    ( pPlayer->m_ClientIndex )		

			//[OP_TRAVERSE_RECORDSET]	( 10)
			//[OP_BOX_FIELD] ( 0 )
			//[OP_BOX_FIELD] ( 1 )
			//[OP_BOX_FIELD] ( 2 )
			//[OP_CHECK_TRAVERSE_LOOP]

			[OP_TABLE_DATA] ( 10 )( '#' )

			// 返回数据盒
			[OP_RETURN_BOX]

		// 结束
		[OP_RPC_END];

		SendToLoginServer( &rpcmsg, ( long )( sizeof( rpcmsg ) - slmaker.EndEdition() ) );
	}
	catch ( lite::Xcpt &e )
	{
		( "rpcerr.log", "[%s]Script准备RPC参数出现错误：%s", GetStringTime(), e.GetErrInfo() );
		rfalse( 2, 1, e.GetErrInfo() );
	}
}

void CScriptObject::RecvCustomWndResult( SACustomWndMsg *pMsg )
{
}

void CScriptObject::RecvLuaCustomMsg( SQALuaCustomMsg *pMsg )
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if ( pPlayer == NULL )
		return;
	//20150305 wk 添加下面代码,注释以前的
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("SI_OnLuaMessage"))
	{
		g_Script.PushParameter(pPlayer->GetSID());
		g_Script.PushParameter(pMsg->streamData);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();

	//if ( pMsg->flags != 0xff ) {
	//	try
	//	{
	//		int top = lua_gettop( g_Script.ls );
	//		if ( g_Script.ls == NULL || top != 0 )
	//			return;

	//		lite::Serialreader slr( pMsg->streamData );

	//		// 获取即将调用的函数名，并做调用准备
	//		g_Script.PrepareFunction( "OnLuaMessage" );
	//		g_Script.PrepareParameters( slr );
	//		g_Script.SetCondition( NULL, pPlayer, NULL );
	//		g_Script.Execute(); 
	//		g_Script.CleanCondition();
	//		return;
	//	}
	//	catch ( lite::Xcpt &e )
	//	{
	//		extern dwt::stringkey<char [256]> szDirname;
	//		TraceInfo( szDirname + "rpcerr.log", "[%s]ScriptTrigger准备RPC参数出现错误：%s", GetStringTime(), e.GetErrInfo() );
	//	}

	//	g_Script.PrepareBreak();
	//} else {

	//	g_Script.SetCondition( NULL, pPlayer, NULL );
	//	lua_State *L = g_Script.ls;
	//	int top = lua_gettop( L );
	//	LuaFunctor luacall( g_Script, "OnLuaMessage" );
	//	luacall[pPlayer->GetSID()];
	//	int ck = luaEx_unserialize( L, ( LPBYTE )pMsg->streamData + 2, *( LPWORD )pMsg->streamData );
	//	if ( ck <= 0 || ck != *( LPWORD )pMsg->streamData )
	//		;
	//	else
	//		luacall();
	//	assert( lua_gettop( L ) == top );
	//	if ( lua_gettop( L ) != top )
	//		lua_settop( L, top );
	//	g_Script.CleanCondition();
	//}
}

void CScriptObject::RecvScriptTrigger( SQScriptTriggerMsg *pMsg )
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if ( pPlayer == NULL )
		return;

	try
	{
		// 获取即将调用的函数名，并做调用准备
		g_Script.PrepareFunction( "ScriptTigger" );

		// 将所有参数放入调用栈
		lite::Serialreader slr( pMsg->streamData );
		while ( slr.curSize() < slr.maxSize() )
		{
			lite::Variant lvt = slr();
			if ( lite::Variant::VT_RPC_OP == lvt.dataType )
			{
				// 说明返回的是一个数组
				if ( lvt._rpcop == 0x80 )
					g_Script.PushVariantArray( slr );
				else
					return;
			}
			else
			{
				g_Script.PushParameter( lvt );
			}
		}

		// 启动调用！
		g_Script.SetCondition( NULL, pPlayer, NULL );
		g_Script.Execute(); 
		g_Script.CleanCondition();
	}
	catch ( lite::Xcpt &e )
	{
		extern dwt::stringkey<char [256]> szDirname;
		TraceInfo( szDirname + "rpcerr.log", "[%s]ScriptTrigger准备RPC参数出现错误：%s", GetStringTime(), e.GetErrInfo() );
	}
}

// 该函数用于响应 L_ATM_DecreasePoint 脚本扣点！
void CScriptObject::OnDecreasePointResult( DWORD verifier, int result, int type, int remaining )
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if ( pPlayer == NULL )
		return;

	// 没有等待处理的扣点操作回调消息
	if ( pPlayer->ATM_temporaryScriptVariant.empty() )
		return;

	// 校验未通过，说明本次操作结果并非上次的请求
	if ( pPlayer->ATM_temporaryVerifier != verifier )
		return;

	pPlayer->ATM_temporaryVerifier = 0;

	// 将对应类型的值刷新，只刷新类型小于4的值！
	//    if ( ( DWORD )type < 4 )
	//      pPlayer->m_Property.checkPoints[ type ] = ( ( remaining > 0 ) ? remaining : 0 );

	try
	{
		// 获取即将调用的函数名，并做调用准备
		std::string funcname = ( LPCSTR )pPlayer->ATM_temporaryScriptVariant.top();
		pPlayer->ATM_temporaryScriptVariant.pop();
		g_Script.PrepareFunction( funcname.c_str() );

		// 将所有存在的参数放入调用栈
		while ( !pPlayer->ATM_temporaryScriptVariant.empty() )
		{
			g_Script.PushParameter( pPlayer->ATM_temporaryScriptVariant.top() );
			pPlayer->ATM_temporaryScriptVariant.pop();
		}

		// 最后压入返回值！
		g_Script.PushParameter( lite::Variant( result ) );

		extern dwt::stringkey<char [256]> szDirname;
		TraceInfo( szDirname + "decpoint.log", "[%s]扣点操作完成返回[%d]！[SID=%08X][剩余[%d-%d]]", 
			GetStringTime(), result, pPlayer->GetSID(), type, remaining );

		// 启动调用！
		g_Script.SetCondition( NULL, pPlayer, NULL );
		g_Script.Execute(); 
		g_Script.CleanCondition();
	}
	catch ( lite::Xcpt &e )
	{
		extern dwt::stringkey<char [256]> szDirname;
		TraceInfo( szDirname + "decpoint.log", "[%s]Script准备RPC参数出现错误：%s", GetStringTime(), e.GetErrInfo() );
	}
}

void CScriptObject::TalkBubble( LPCSTR info )
{
	if ( m_ParentArea == NULL )
		return;

	SAChatPublic msg;
	msg.dwGID = GetGID();
	msg.wSoundId = 0;
	dwt::strcpy( msg.cCharData, info, sizeof( msg.cCharData ) );
	m_ParentArea->SendAdj( &msg, msg.GetMySize(), -1 );
}

// 取得任务加点
int GetTaskPoint( WORD taskId )
{
	switch ( taskId )
	{
	case 681:
	case 691:
		return 1;
	case 683:
	case 693:
	case 777:
	case 779:
	case 781:
	case 783:
	case 785:
		return 2;
	case 685:
	case 695:
		return 3;
	case 687:
	case 697:
		return 4;

	case 121: // 新手帮助 +5
	case 122:
	case 224:
	case 225:
	case 324:
	case 325:
	case 425:
	case 426:
	case 521:
	case 522:
	case 649:
	case 651:
	case 653:
	case 662:
	case 740:
	case 741:
	case 742:
	case 788:
	case 789:
	case 791:
	case 792:
	case 793:
	case 794:
	case 796:
	case 797:
		return 5;
	case 764:
	case 771:
	case 775:
	case 678:
	case 786:
	case 790:
	case 798:
	case 627:
		return 10;
	case 689:
	case 699:
		return 15;
	case 655:
	case 795:
		return 20;
	};
	return 0;
}

int CScriptObject::CheckTaskPoints()
{
	int result = 0;
	//     for ( std::list<SRoleTask>::iterator it = m_TaskList.begin(); it != m_TaskList.end(); it++ )
	// 	{
	// 		SRoleTask *p = &*it;
	// 		if ( !p->bSave)
	//             continue;
	// 
	//         result += GetTaskPoint( p->wTaskID );
	// 	}

	return result;
}
