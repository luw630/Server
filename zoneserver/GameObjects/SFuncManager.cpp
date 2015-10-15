#include "StdAfx.h"
#include <hash_map>
#include "ScriptManager.h"
#include "region.h"
#include "dynamicregion.h"
#include "trigger.h"
#include "Player.h"
#include "Item.h"
#include "Area.h"
#include "networkmodule\regionmsgs.h"
#include "networkmodule\scriptmsgs.h"
#include "networkmodule\chatmsgs.h"
#include "networkmodule\orbmsgs.h"
#include "networkmodule\DataTransMsgs.h"
#include "common.h"
#include "monster.h"
#include <time.h>
#include "npc.h"
#include "sendmail.h"
#include "gaminghouse.h"
#include "factionmanager.h"
#include "NetworkModule/CardPointModifyMsgs.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "sfuncmanager.h"
extern LPIObject FindRegionByID(DWORD ID);
extern LPIObject GetPlayerByGID(DWORD);
extern LPIObject FindRegionByGID(DWORD GID);
extern BOOL PutPlayerIntoDestRegion(CPlayer *pPlayer, WORD wNewRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID = 0);
extern LPCSTR GetStringParameter(LPCSTR string, int maxsize, int index);
extern WORD GetServerIDFromName(LPCSTR szServerName);
extern LPIObject GetPlayerByName(LPCSTR);

extern int SendToLoginServer(SMessage *data, long size);
extern LPCSTR GetNpcScriptInfo( LPCSTR info );
extern LPIObject GetPlayerBySID(DWORD dwStaticID);
//////////////////////////// 
// spouse
#define SPOUSR_FILENAME "spousemap.spm"
std::map<DWORD, string> g_spouseMap;

bool PushSpouse(DWORD dwID, string name)
{
	if(dwID == 0 || name == "")
		return false;

	g_spouseMap[dwID] = name;
	WriteSpouseToFile();

	return true;
}

bool PopSpouse(DWORD dwID)
{
	if(dwID == 0)
		return false;

	std::map<DWORD, string>::iterator it = g_spouseMap.find(dwID);
	if(it != g_spouseMap.end())
	{
		g_spouseMap.erase(it);
		WriteSpouseToFile();
		return true;
	}

	return false;
}

string GetSpouseName(DWORD dwID)
{
	std::map<DWORD, string>::iterator it = g_spouseMap.find(dwID);
	if(it != g_spouseMap.end())
		return it->second;

	return "";
}

void FillSpouseList()
{
	if(g_spouseMap.size() > 0)
		return;

	ifstream infile(SPOUSR_FILENAME);
	if(!infile.is_open())
		return;

	DWORD dwSID = 0;
	string name = "";
	while(!infile.eof())
	{
		infile >> dwSID >> name;
		if(dwSID != 0 && name != "")
			g_spouseMap[dwSID] = name;
	}

	infile.close();

}

MAKE_LFUNC(L_fillspouselist, FillSpouseList());

void WriteSpouseToFile()
{
	if(g_spouseMap.size() == 0)
		return;

	DeleteFile(SPOUSR_FILENAME);

	ofstream outfile;
	outfile.open(SPOUSR_FILENAME, ios_base::app);
	if(outfile.fail())
		return;

	std::map<DWORD, string>::iterator it = g_spouseMap.begin();
	for(; it != g_spouseMap.end(); it++)
	{
		outfile << (it->first) << ends << (it->second) << endl;
	}

	outfile.close();
}
///////////////////////////
// 怪物相关的脚本函数
int CSMonsterFunc::L_getmonsterlistid(lua_State *L)
{
	//     if(g_Script.m_pMonster == NULL)
	//     {
	//         lua_pushnumber(L, 0);
	//         return 1;
	//     }
	// 
	//     SRoleTask *pTask = g_Script.m_pMonster->FindTask(1);
	//     if(pTask)
	//         lua_pushnumber(L, pTask->byGoodsID);
	//     else
	//         lua_pushnumber(L, 0);
	// 
	//     return 1;
	return 0;  
}

int CSMonsterFunc::L_getdynmonstergid(lua_State *L)
{
	//     if(g_Script.m_pMonster == NULL)
	//     {
	//         lua_pushnumber(L, 0);
	//         return 1;
	//     }
	// 
	//     SRoleTask *pTask = g_Script.m_pMonster->FindTask(1);
	//     if(pTask)
	//         lua_pushnumber(L, pTask->wComplete);
	//     else
	//         lua_pushnumber(L, 0);
	// 
	//     return 1;
	return 0;
}

int CSMonsterFunc::L_delmonster(lua_State *L)
{
	DWORD  dwRegionID = static_cast<DWORD>(lua_tonumber(L, 1));
	DWORD  dwMonsterGID = static_cast<DWORD>(lua_tonumber(L, 2));

	CRegion *pRegion = NULL;

	if(dwRegionID > 0x40000000)
	{
		pRegion = (CRegion *)FindRegionByGID(
			dwRegionID)->DynamicCast(IID_REGION);

	}
	else
	{
		pRegion = (CRegion *)FindRegionByID(
			dwRegionID)->DynamicCast(IID_REGION);
	}

	if (pRegion)
		pRegion->DelMonster(dwMonsterGID);

	return 0;
}

int CSMonsterFunc::L_createmonsterg(lua_State *L)
{
	/*
	DWORD dwRegionGID = static_cast<DWORD>(lua_tonumber(L, 1));

	CRegion *pRegion = NULL;

	if (dwRegionGID > 0x40000000)
	{
	pRegion = (CRegion *)FindRegionByGID(dwRegionGID)->DynamicCast(IID_REGION);
	}
	else
	{
	pRegion = (CRegion *)FindRegionByID(dwRegionGID)->DynamicCast(IID_REGION);
	}

	if (pRegion == NULL)
	return 0;

	CMonster::SParameter param;
	ZeroMemory( &param, sizeof( CMonster::SParameter) );

	param.extraData = NULL;
	LPCSTR szName = static_cast<const char*>(lua_tostring(L, 2));

	dwt::strcpy(param.szName, szName, CONST_USERNAME);

	param.wRefreshStyle = static_cast<WORD>(lua_tonumber(L, 3));
	param.dwRefreshTime = static_cast<DWORD>(lua_tonumber(L, 4));
	param.wRefreshScriptID = static_cast<WORD>(lua_tonumber(L, 5));

	param.wX = static_cast<WORD>(lua_tonumber(L, 6));
	param.wY = static_cast<WORD>(lua_tonumber(L, 7));
	param.wMoveArea = static_cast<WORD>(lua_tonumber(L, 8));
	param.wAIType = static_cast<WORD>(lua_tonumber(L, 9));
	param.wAIScriptID = static_cast<WORD>(lua_tonumber(L, 10));
	param.wDeadScriptID = static_cast<WORD>(lua_tonumber(L, 11));
	param.wListID = static_cast<WORD>(lua_tonumber(L, 12));

	CMonster *pMonster = pMonster = pRegion->CreateMonster(&param);
	if(pMonster)
	{
	SRoleTask task;
	memset(&task,  0, sizeof(SRoleTask));
	task.wTaskID = 1;     // 专用于保存脚本生成的列表编号
	task.wComplete = pMonster->GetGID();

	pMonster->AddTask(task, FALSE); 
	}
	else 
	{
	rfalse(2, 1, "生成怪物失败！！");
	return 0;
	}//*/

	return 0;
}

int CSMonsterFunc::L_getmonstercurpos( lua_State *L )
{
	if ( g_Script.m_pMonster == NULL )
		return 0;

	WORD wX = 0;
	WORD wY = 0;
	DWORD wRegionID = 0;
	BOOL isDynamic = FALSE;
	g_Script.m_pMonster->GetCurPos( wX, wY );
	if ( g_Script.m_pMonster->m_ParentRegion )
	{
		if ( g_Script.m_pMonster->m_ParentRegion->DynamicCast( IID_DYNAMICREGION ) )
		{
			wRegionID = g_Script.m_pMonster->m_ParentRegion->GetGID();
			isDynamic = true;
		}
		else
		{
			wRegionID = g_Script.m_pMonster->m_ParentRegion->m_wRegionID;
		}
	}

	lua_pushnumber(L, wX);
	lua_pushnumber(L, wY);
	lua_pushnumber(L, wRegionID);
	lua_pushnumber(L, isDynamic);

	return 4;
}

void CSMonsterFunc::CreateShadow()
{
	g_Script.RegisterFunction("GetMonsterListID", L_getmonsterlistid);
	g_Script.RegisterFunction("GetDynMonsterGID", L_getdynmonstergid);
	g_Script.RegisterFunction("GetMonsterCurPos", L_getmonstercurpos);
	g_Script.RegisterFunction("DelMonster", L_delmonster);
	g_Script.RegisterFunction("CreateMonsterG", L_createmonsterg);
}
//-------------------------------------------------------------------------------------------
// NPC相关的脚本函数
int CSNPCFunc::L_getnpclistid(lua_State *L)
{
	//     if(g_Script.m_pNpc == NULL)
	//     {
	//         lua_pushnumber(L, 0);
	//         return 1;
	//     }
	// 
	//     SRoleTask *pTask = g_Script.m_pNpc->FindTask(1);
	//     if(pTask)
	//         lua_pushnumber(L, pTask->byGoodsID);
	//     else
	//         lua_pushnumber(L, 0);
	// 
	//     return 1;
	return 0;
}

int CSNPCFunc::L_deltaskfornpc(lua_State *L)
{
	return 0;
}

int CSNPCFunc::L_addtaskfornpc(lua_State *L)
{
	return 0;
}

int CSNPCFunc::L_gettaskdatafromnpc(lua_State *L)
{
	return 0;
}

int CSNPCFunc::L_checktaskfromnpc(lua_State *L)
{
	return 0;
}

int CSNPCFunc::L_getdynnpcgid(lua_State *L)
{
	return 0;
}

int CSNPCFunc::L_delnpc(lua_State *L)
{
	DWORD  dwRegionID = static_cast<DWORD>(lua_tonumber(L, 1));
	DWORD  dwNpcGID = static_cast<DWORD>(lua_tonumber(L, 2));

	CRegion *pRegion = NULL;

	if(dwRegionID > 0x40000000)
	{
		pRegion = (CRegion *)FindRegionByGID(
			dwRegionID)->DynamicCast(IID_REGION);

	}
	else
	{
		pRegion = (CRegion *)FindRegionByID(
			dwRegionID)->DynamicCast(IID_REGION);
	}

	if (pRegion)
		pRegion->DelNpc(dwNpcGID);

	return 0;
}

int CSNPCFunc::L_createnpcg(lua_State *L)
{
	return 0;

}

void CSNPCFunc::CreateShadow()
{
	g_Script.RegisterFunction("GetNpclistID", L_getnpclistid);
	g_Script.RegisterFunction("AddTaskForNpc", L_addtaskfornpc);
	g_Script.RegisterFunction("DelTaskForNpc", L_deltaskfornpc);
	g_Script.RegisterFunction("GetTaskDataFromNpc", L_gettaskdatafromnpc);
	g_Script.RegisterFunction("CheckTaskFromNpc", L_checktaskfromnpc);
	g_Script.RegisterFunction("GetDynNpcGID", L_getdynnpcgid);
	g_Script.RegisterFunction("DelNpc", L_delnpc);
	g_Script.RegisterFunction("CreateNpcG", L_createnpcg);
}
//----------------------------------------------------------------------------------------------
// 玩家相关的脚本函数
int CSPlayerFunc::L_getplayervenationstate(lua_State *L)
{
	return 0;
}

int CSPlayerFunc::L_damagetovenation(lua_State *L)
{
	return 0;
}

int CSPlayerFunc::L_changeskill(lua_State *L)
{
	return 0;
}

int CSPlayerFunc::L_getrecvmoney(lua_State *L)
{

	return 1;
}

int CSPlayerFunc::L_getbackitemsinfo(lua_State *L)
{
	return 0;
}

int CSPlayerFunc::L_checkrecvitem(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
		return 0;

	WORD wID = static_cast<WORD>(lua_tonumber(L, 1));

	WORD wCheck = g_Script.m_pPlayer->CheckSOneItemFlag(wID) == true ? 1 : 0;

	lua_pushnumber(L, wCheck);
	return 1;
}

int CSPlayerFunc::L_getsoneitemnum(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
		return 0;

	WORD wID = static_cast<WORD>(lua_tonumber(L, 1));
	WORD wCount = 0;

	wCount = g_Script.m_pPlayer->GetSOneItemNum(wID);

	lua_pushnumber(L, wCount);
	return 1;
}

int CSPlayerFunc::L_delallrecvitems(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
		return 0;

	g_Script.m_pPlayer->DelItemByFlag();

	return 0;
}

int CSPlayerFunc::L_restoreallrecvitems(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
		return 0;

	g_Script.m_pPlayer->RestoreAllItemByFlag();

	return 0;
}

int CSPlayerFunc::L_clearallsitems(lua_State *L)
{
	return 0;
}

int CSPlayerFunc::L_getregionretpos( lua_State *L )
{
	if ( g_Script.m_pPlayer == NULL )
		return 0;

	WORD wX = 0;
	WORD wY = 0;
	WORD wRegionID = 0;

	if ( g_Script.m_pPlayer->m_ParentRegion )
	{
		wRegionID = g_Script.m_pPlayer->m_ParentRegion->m_wReLiveRegionID;
		wX = ( WORD )g_Script.m_pPlayer->m_ParentRegion->m_ptReLivePoint.x;
		wY = ( WORD )g_Script.m_pPlayer->m_ParentRegion->m_ptReLivePoint.y;
	}

	lua_pushnumber(L, wX);
	lua_pushnumber(L, wY);
	lua_pushnumber(L, wRegionID);

	return 3;
}

int CSPlayerFunc::L_getplayercurpos(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
		return 0;

	WORD wX = 0;
	WORD wY = 0;
	DWORD wRegionID = 0;
	BOOL isDynamic = FALSE;
	g_Script.m_pPlayer->GetCurPos( wX, wY );
	if ( g_Script.m_pPlayer->m_ParentRegion )
	{
		if ( g_Script.m_pPlayer->m_ParentRegion->DynamicCast( IID_DYNAMICREGION ) )
		{
			wRegionID = g_Script.m_pPlayer->m_ParentRegion->GetGID();
			isDynamic = true;
		}
		else
		{
			wRegionID = g_Script.m_pPlayer->m_ParentRegion->m_wRegionID;
		}
	}

	lua_pushnumber(L, wX);
	lua_pushnumber(L, wY);
	lua_pushnumber(L, wRegionID);
	lua_pushnumber(L, isDynamic);

	return 4;
}

int CSPlayerFunc::L_setbuynpc(lua_State *L)
{
	//     if(g_Script.m_pPlayer == NULL)
	//         return 0;
	// 
	//     DWORD dwGID = static_cast<DWORD>(lua_tonumber(L, 1));
	//     g_Script.m_pPlayer->m_CommNpcGlobalID  = dwGID;

	return 0;
}

int CSPlayerFunc::L_ismonstermutate(lua_State *L)
{
	//     if(g_Script.m_pPlayer == NULL)
	//         return 0;
	// 
	//     WORD wIsMutate = (WORD)g_Script.m_pPlayer->IsMonsterMutate();
	//     lua_pushnumber(L, wIsMutate);
	// 
	//     return 1;
	return 0;
}

int CSPlayerFunc::L_setmutate(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
		return 0;

	WORD wType = static_cast<WORD>(lua_tonumber(L, 1));
	WORD wID = static_cast<WORD>(lua_tonumber(L, 2));

	g_Script.m_pPlayer->SetMutate((BYTE)wType, wID);
	if(wType == 0 && wID == 0)
	{
		g_Script.m_pPlayer->SetWalkSpeedCoef( 1.0 );
		g_Script.m_pPlayer->m_wMonsterCountOnGoat = 0;
		g_Script.m_pPlayer->m_dwAutoAddMCountTime = timeGetTime();
		g_Script.m_pPlayer->SendOnGoatMonsterCount( 0, 0 );
	}

	return 0;

}

int CSPlayerFunc::L_getmutateid(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
		return 0;

	lua_pushnumber(L, g_Script.m_pPlayer->GetMutateID());
	return 1;
}

int CSPlayerFunc::L_addmultipletime(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
		return 0;

	WORD wOperate = static_cast<WORD>(lua_tonumber(L, 1));
	int value = static_cast<int>(lua_tonumber(L, 2));

	switch(wOperate) 
	{
	case 1:  // 正在进行的累加
		if ( value > 0 )
			g_Script.m_pPlayer->SetMultipleTime(( BYTE )value + g_Script.m_pPlayer->GetMultipleTime());
		break;
	case 2:  // 自由分配不扣除
		if ( value > 0 )
		{
			g_Script.m_pPlayer->SetMultipleTime(( BYTE )value);
			g_Script.m_pPlayer->SetMulStartTime((DWORD)time(NULL));
		}
		break;
	case 3:  // 剩余上的累加
		if ( value < 0 && g_Script.m_pPlayer->GetLeaveTime() + value <= 0 )
			g_Script.m_pPlayer->SetLeaveTime( 0 );
		else
			g_Script.m_pPlayer->SetLeaveTime( g_Script.m_pPlayer->GetLeaveTime() + value );
		break;
	case 4: // 设置当前周时间
		if ( value < 0 && g_Script.m_pPlayer->GetCurWeekUseTime() + value <= 0 )
			g_Script.m_pPlayer->SetCurWeekUseMulTime( 0 );
		else
			g_Script.m_pPlayer->SetCurWeekUseMulTime( g_Script.m_pPlayer->GetCurWeekUseTime() + value );
		break;
	}

	return 0;
}

int CSPlayerFunc::L_inviteteamtohome(lua_State *L)
{
	return 0;
}

int CSPlayerFunc::L_setspouse(lua_State *L)
{
	//     if(g_Script.m_pPlayer == NULL)
	//         return 0;
	// 
	//     LPCSTR lpszName = static_cast<const char*>(lua_tostring(L, 1)); 
	// 
	//     if(lpszName != NULL)
	//     {
	//         dwt::strcpy(g_Script.m_pPlayer->m_Property.m_szMateName, lpszName, CONST_USERNAME);
	//         g_Script.m_pPlayer->SendMyState();
	//     }

	return 0;
}

int CSPlayerFunc::L_delspouse(lua_State *L)
{
	//     if(g_Script.m_pPlayer == NULL)
	//         return 0;
	// 
	//     ZeroMemory(g_Script.m_pPlayer->m_Property.m_szMateName, sizeof(char)*CONST_USERNAME);
	//     g_Script.m_pPlayer->SendMyState();

	return 0;
}

int CSPlayerFunc::L_getcurmoney(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
	{
		lua_pushnumber(L, -1);
		return 1;
	}

	lua_pushnumber(L, g_Script.m_pPlayer->m_Property.m_Money);
	return 1;
}


int CSPlayerFunc::L_entergaminghouse(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
		return 0;

	DWORD dwMasterGID = static_cast<DWORD>(lua_tonumber(L, 1));
	CPlayer *pPlayer = (CPlayer *)GetPlayerByGID(dwMasterGID)->DynamicCast(IID_PLAYER);
	if(pPlayer == NULL)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex,  "对方玩家不在线!!");
		return 0;
	}

	if(g_Script.m_pPlayer->GetSportsRoomID() > 0)
	{
		g_Script.m_pPlayer->SetSportsRoomID(0);
	}

	if(pPlayer->GetSportsRoomID())
	{

		CGamingHouse *pSubHouse = (CGamingHouse *)_theGHouseManager->Locate(
			pPlayer->GetSportsRoomID());

		if(pSubHouse == NULL)
		{
			pPlayer->SetSportsRoomID(0);
			TalkToDnid(g_Script.m_pPlayer->m_ClientIndex,  "对方房间不存在!!");
			return 0;
		}

		if(pSubHouse->GetMasterID() != pPlayer->GetGID())
		{
			TalkToDnid(g_Script.m_pPlayer->m_ClientIndex,  "对方正在其它的房间!!");
			return 0;
		}

		if(pSubHouse->IsEmptyRoom())
		{
			pPlayer->SetSportsRoomID(0);
			return 0;
		}

		if(pSubHouse->CountPlayer() == GAMING_MAXPLAER)
		{
			TalkToDnid(g_Script.m_pPlayer->m_ClientIndex,  "对方房间里人数已满!!");
			return 0;
		}
		else
		{
			pSubHouse->AddGamingPlayer(g_Script.m_pPlayer);
		}


	}
	else
	{
		CGamingHouse *pSubHouse = NULL;
		pSubHouse = (CGamingHouse *)_theGHouseManager->GetEmptyRoom();
		if(pSubHouse == NULL) 
			pSubHouse = new  CGamingHouse( dwMasterGID );
		else
		{
			pSubHouse->SetMasterID(dwMasterGID);
			pSubHouse->InitNew();
		}

		if( pSubHouse )
		{
			pSubHouse->AddGamingPlayer(pPlayer);
			pSubHouse->AddGamingPlayer(g_Script.m_pPlayer);
		}
	}

	return 0;
}

int CSPlayerFunc::L_getroomid(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
	{
		lua_pushnumber(L, -1);
		return 1;
	}

	lua_pushnumber(L, g_Script.m_pPlayer->GetSportsRoomID());
	return 1;
}

int CSPlayerFunc::L_getfrienddegree(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
	{
		lua_pushnumber(L, -1);
		return 1;
	}

	return 1;
}

int CSPlayerFunc::L_getmarrydegree(lua_State *L)
{
	//     if(g_Script.m_pPlayer == NULL)
	//     {
	//         lua_pushnumber(L, -1);
	//         return 1;
	//     }
	//     
	//     DWORD date = g_Script.m_pPlayer->m_Property.m_dwMarryDate;
	//     if(date > 0)
	//     {
	//         double el_time = difftime(time(NULL), ((time_t)date));
	//         DWORD dwRet = DWORD(el_time/3600);
	//         lua_pushnumber(L, dwRet);
	//     }
	//     else
	//     {
	//         lua_pushnumber(L, 0);
	//     }
	// 
	//     return 1;
	return 0;
}

int CSPlayerFunc::L_setmarrydate(lua_State *L)
{
	//     if(g_Script.m_pPlayer == NULL)
	//     {
	//         return 0;
	//     }
	// 
	//     DWORD dwOGID = static_cast<DWORD>(lua_tonumber(L, 1));
	//     WORD wOperate = static_cast<WORD>(lua_tonumber(L, 2));
	// 
	//     CPlayer *pPlayer = (CPlayer *)GetPlayerByGID(dwOGID)->DynamicCast(IID_PLAYER);
	//     if(pPlayer == NULL)
	//     {
	//         return 0;
	//     }
	// 
	//     time_t tt = 0;
	//     if(wOperate)
	//         tt = time(NULL);
	// 
	//     g_Script.m_pPlayer->m_Property.m_dwMarryDate = (__int32)tt;
	//     pPlayer->m_Property.m_dwMarryDate = (int)tt;

	return 0;
}

int CSPlayerFunc::L_getmarrydate(lua_State *L)
{
	//     if(g_Script.m_pPlayer == NULL)
	//     {
	//         lua_pushnumber(L, 0);
	//         return 1;
	//     }   
	// 
	//     lua_pushnumber(L, g_Script.m_pPlayer->m_Property.m_dwMarryDate);
	//     return 1;
	return 0;
}

int CSPlayerFunc::L_addskillproficiency(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
	{
		return 0;
	}

	WORD wSkill = static_cast<WORD>(lua_tonumber(L, 1));
	int  nNum = static_cast<int>(lua_tonumber(L, 2));

	g_Script.m_pPlayer->AddSkillProficiency(wSkill, nNum);

	return 0;

}

//int CSPlayerFunc::L_addskillproficiencyF(lua_State *L)
//{
//	if(g_Script.m_pPlayer == NULL)
//	{
//		return 0;
//	}
//
//    WORD wSkill = static_cast<WORD>(lua_tonumber(L, 1));
//    int  nNum = static_cast<int>(lua_tonumber(L, 2));
//	WORD wLevel = 0;
//
//    if(FALSE == g_Script.m_pPlayer->AddSkillProficiencyF(wSkill, nNum, wLevel))
//	{
//		lua_pushnumber(L, -1);
//	}
//	else
//	{
//		lua_pushnumber(L, (int)wLevel);
//	}
//
//	return 1;
//}

int CSPlayerFunc::L_getfactionname(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
	{
		lua_pushnumber(L, 0);
		return 1;
	}

	lua_pushstring(L, g_Script.m_pPlayer->m_Property.m_szTongName);
	return 1;   
}

int CSPlayerFunc::L_isfactionmaster(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
	{
		lua_pushnumber(L, 0);
		return 1;
	}

	lua_pushnumber(L, g_Script.m_pPlayer->m_stFacRight.Title == 8 );
	return 1;
}

string s_FactionName = "";
DWORD s_dwFactionTitle = 0;

int CSPlayerFunc::L_setfactionsystitleid(lua_State *L)
{
	LPCSTR lpszName = static_cast<const char*>(lua_tostring(L, 1));
	DWORD dwID = static_cast<DWORD>(lua_tonumber(L, 2));

	// 目前对于加标志的帮派就是天下第一帮的唯一标志
	if(dwID)
	{
		s_dwFactionTitle = dwID;
		s_FactionName = lpszName;
	}

	return 0;
}

int CSPlayerFunc::L_getelapsedtimesFac(lua_State *L)
{
	//     if(g_Script.m_pPlayer == NULL)
	//     {
	//         lua_pushnumber(L, -1);
	//         return 1;
	//     }
	// 
	// 	WORD wTaskID = static_cast<WORD>(lua_tonumber(L, 1));
	// 	SRoleTask *pTask = g_Script.m_pPlayer->FindTask(wTaskID);
	// 	if(pTask == NULL)
	// 	{
	//         lua_pushnumber(L, -1);
	//         return 1;
	// 	}
	// 
	// 	time_t jointime = (time_t)pTask->wComplete;
	// 	double elsptime = difftime(time(0), jointime);
	// 	lua_pushnumber(L, elsptime);
	// 
	// 	return 1;
	return 0;
}

int CSPlayerFunc::L_updateGiftCoupon(lua_State *L)
{/*
 if(g_Script.m_pPlayer == NULL)
 {
 lua_pushnumber(L, -1);
 return 1;
 }
 int add = static_cast<int>(lua_tonumber(L, 1));
 int num = g_Script.m_pPlayer->m_Property.m_dwGiftCoupon;
 num += add;
 if( num < 0 || num > 0xffffffff )
 lua_pushnumber(L, -1);

 g_Script.m_pPlayer->m_Property.m_dwGiftCoupon = num;

 SATreasureShopMsg _msg;
 _msg.type_ = SQTreasureShopMsg::TYPE_MONEY;
 _msg.riches[0] = g_Script.m_pPlayer->m_Property.m_dwXYDPoint;
 _msg.riches[1] = g_Script.m_pPlayer->m_Property.m_Money;
 _msg.riches[2] = g_Script.m_pPlayer->m_Property.m_dwGiftCoupon;

 g_StoreMessage( g_Script.m_pPlayer->m_ClientIndex, &_msg, sizeof(SATreasureShopMsg) );

 lua_pushnumber( L, 1 );

 return 1;*/
	return 0;
}

//------------------------------------------------------------------------
struct SC_STINFO 
{
	enum OTYPE { OT_MESSAGE, OT_MOVETO, OT_ADDSUBT};
	WORD wOperate;

	union 
	{
		struct 
		{
			SMessage *pMsg;
			DWORD dwSize;
			DNID dnidExcept;
		}MESSGE;

		struct 
		{
			DWORD dwRegion;
			DWORD dwToX;
			DWORD dwToY;
		}MOVETO;

		struct 
		{
			WORD wOperate;  // 0: sub  1: add	 
			DWORD dwValue;
		}ADDSUBT;
	}Att;
};
// sc_SendTeamInfo: 发给区块上的同组玩家的消息
void sc_SendTeamInfo(class CArea *pArea, SC_STINFO &stInfo, DWORD dwTeamID)
{
	if( pArea == NULL || dwTeamID == 0 )
		return;

	check_list< LPIObject > _TempList(pArea->m_PlayerList);
	check_list< LPIObject >::iterator it = _TempList.begin();
	while (it != _TempList.end())
	{
		CPlayer *pPlayer = (CPlayer*)(*it)->DynamicCast(IID_PLAYER);
		if (pPlayer != NULL)
		{
			if (dwTeamID == pPlayer->m_dwTeamID)
			{
				switch(stInfo.wOperate)
				{
				case SC_STINFO::OT_MESSAGE :
					{
						g_StoreMessage(pPlayer->m_ClientIndex, stInfo.Att.MESSGE.pMsg, 
							stInfo.Att.MESSGE.dwSize);

					}break;

				case SC_STINFO::OT_MOVETO:
					{
						if(stInfo.Att.MOVETO.dwRegion > 0x40000000)
						{
							PutPlayerIntoDestRegion(pPlayer, 0, (WORD)stInfo.Att.MOVETO.dwToX, 
								(WORD)stInfo.Att.MOVETO.dwToY, stInfo.Att.MOVETO.dwRegion);
						}
						else
						{
							PutPlayerIntoDestRegion(pPlayer, (WORD)stInfo.Att.MOVETO.dwRegion, 
								(WORD)stInfo.Att.MOVETO.dwToX, (WORD)stInfo.Att.MOVETO.dwToY);
						}
					}break;

				case SC_STINFO::OT_ADDSUBT:
					{
					}break;

				default:
					return;
				}
			}
		}
		it++;
	}
}

void sc_SendTeamInfoAroundArea(class CArea *pArea, SC_STINFO &stInfo, DWORD dwTeamID)
{
	sc_SendTeamInfo(pArea, stInfo, dwTeamID);
	if (pArea->m_ParentRegion)
	{
		// up
		sc_SendTeamInfo((CArea*)pArea->m_ParentRegion->GetArea(pArea->m_X, pArea->m_Y-1).get(), stInfo, dwTeamID);
		// right up
		sc_SendTeamInfo((CArea*)pArea->m_ParentRegion->GetArea(pArea->m_X+1, pArea->m_Y-1).get(), stInfo, dwTeamID);
		// right
		sc_SendTeamInfo((CArea*)pArea->m_ParentRegion->GetArea(pArea->m_X+1, pArea->m_Y).get(), stInfo, dwTeamID);
		// right down
		sc_SendTeamInfo((CArea*)pArea->m_ParentRegion->GetArea(pArea->m_X+1, pArea->m_Y+1).get(), stInfo, dwTeamID);
		// down
		sc_SendTeamInfo((CArea*)pArea->m_ParentRegion->GetArea(pArea->m_X, pArea->m_Y+1).get(), stInfo, dwTeamID);
		// down left
		sc_SendTeamInfo((CArea*)pArea->m_ParentRegion->GetArea(pArea->m_X-1, pArea->m_Y+1).get(), stInfo, dwTeamID);
		// left
		sc_SendTeamInfo((CArea*)pArea->m_ParentRegion->GetArea(pArea->m_X-1, pArea->m_Y).get(), stInfo, dwTeamID);
		// left up
		sc_SendTeamInfo((CArea*)pArea->m_ParentRegion->GetArea(pArea->m_X-1, pArea->m_Y-1).get(), stInfo, dwTeamID);

	}

}
//------------------------------------------------------
int CSPlayerFunc::L_sendteaminfotoaround(lua_State *L)
{
	/*
	if(g_Script.m_pPlayer == NULL)
	{
	return 0;
	}

	SAClickObjectMsg msg;
	ZeroMemory(&msg.wParam,sizeof(WORD)*5);
	ZeroMemory(&msg.sMessage, sizeof(char)*STRING_LENTH);

	LPCSTR str = {0};
	msg.wParam[0] = static_cast<DWORD>(lua_tonumber(L, 1));      // 服务器处理的参数ID（其实可以保留在服务器）
	str = static_cast<const char*>(lua_tostring(L, 2));          // 发送的字符串
	msg.wParam[1] = static_cast<DWORD>(lua_tonumber(L, 3));      // 面版是否关闭
	msg.wParam[2] = static_cast<DWORD>(lua_tonumber(L, 4));      // 面板头像编号
	msg.wParam[3] = static_cast<DWORD>(lua_tonumber(L, 5));      // 扩展的类型

	dwt::strcpy(msg.sMessage, str, STRING_LENTH);

	//g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(SAClickObjectMsg));

	WORD wSameArea = static_cast<WORD>(lua_tonumber(L, 6));

	SC_STINFO sc_info;
	ZeroMemory(&sc_info, sizeof(SC_STINFO));
	sc_info.wOperate = SC_STINFO::OT_MESSAGE;
	sc_info.Att.MESSGE.pMsg = &msg;
	sc_info.Att.MESSGE.dwSize = sizeof(SAClickObjectMsg);

	if(wSameArea)
	{
	sc_SendTeamInfoAroundArea(g_Script.m_pPlayer->m_ParentArea, sc_info, g_Script.m_pPlayer->m_dwTeamID);
	}
	else
	{
	sc_SendTeamInfo(g_Script.m_pPlayer->m_ParentArea, sc_info, g_Script.m_pPlayer->m_dwTeamID);
	}
	//*/
	return 0;
}

int CSPlayerFunc::L_putaroundteamto(lua_State *L)
{

	if(g_Script.m_pPlayer == NULL)
	{
		return 0;
	}

	SC_STINFO sc_info;
	ZeroMemory(&sc_info, sizeof(SC_STINFO));
	sc_info.wOperate = SC_STINFO::OT_MOVETO;
	sc_info.Att.MOVETO.dwRegion = static_cast<DWORD>(lua_tonumber(L, 1));
	sc_info.Att.MOVETO.dwToX = static_cast<DWORD>(lua_tonumber(L, 2));
	sc_info.Att.MOVETO.dwToY = static_cast<DWORD>(lua_tonumber(L, 3));
	WORD wSameArea = static_cast<WORD>(lua_tonumber(L, 4));

	if(wSameArea)
	{
		sc_SendTeamInfoAroundArea(g_Script.m_pPlayer->m_ParentArea, sc_info, g_Script.m_pPlayer->m_dwTeamID);
	}
	else
	{
		sc_SendTeamInfo(g_Script.m_pPlayer->m_ParentArea, sc_info, g_Script.m_pPlayer->m_dwTeamID);
	}

	return 0;
}

int CSPlayerFunc::L_addsubteamtask(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
	{
		return 0;
	}

	SC_STINFO sc_info;
	ZeroMemory(&sc_info, sizeof(SC_STINFO));
	sc_info.wOperate = SC_STINFO::OT_ADDSUBT;
	sc_info.Att.ADDSUBT.wOperate = static_cast<WORD>(lua_tonumber(L, 1));
	sc_info.Att.ADDSUBT.dwValue = static_cast<DWORD>(lua_tonumber(L, 2));

	WORD wSameArea = static_cast<WORD>(lua_tonumber(L, 3));

	if(wSameArea)
	{
		sc_SendTeamInfoAroundArea(g_Script.m_pPlayer->m_ParentArea, sc_info, g_Script.m_pPlayer->m_dwTeamID);
	}
	else
	{
		sc_SendTeamInfo(g_Script.m_pPlayer->m_ParentArea, sc_info, g_Script.m_pPlayer->m_dwTeamID);
	}


	return 0;
}

int CSPlayerFunc::L_putallplayerA(lua_State *L)
{
	CRegion *pRegion = NULL;
	DWORD srcRegionId = static_cast<DWORD>( lua_tonumber( L, 4 ) );

	if( srcRegionId == 0 && g_Script.m_pPlayer )
		pRegion = g_Script.m_pPlayer->m_ParentRegion;
	else if ( srcRegionId )
	{
		if ( srcRegionId > 0x40000000)
			pRegion = (CRegion *)FindRegionByGID( srcRegionId )->DynamicCast(IID_REGION);
		else
			pRegion = (CRegion *)FindRegionByID( srcRegionId )->DynamicCast(IID_REGION);
	}

	if ( pRegion == NULL )
		return 0;

	DWORD wNewRegionID = static_cast<DWORD>(lua_tonumber(L, 1));
	WORD wNewX = static_cast<WORD>(lua_tonumber(L, 2));
	WORD wNewY = static_cast<WORD>(lua_tonumber(L, 3));

	// <=46号的场景中,不能使用putallplayer
	if ( pRegion->m_wRegionID <= 46 )
		return 0;

	check_list<LPIObject> templist( pRegion->m_PlayerList );

	check_list<LPIObject>::iterator iter = templist.begin();
	while (iter != templist.end())
	{
		CPlayer *pPlayer = (CPlayer *)(*iter)->DynamicCast(IID_PLAYER);
		if (pPlayer)
		{
			if(wNewRegionID > 0x40000000)
				PutPlayerIntoDestRegion(pPlayer, 0, wNewX, wNewY, wNewRegionID);
			else
				PutPlayerIntoDestRegion(pPlayer, (WORD)wNewRegionID, wNewX, wNewY);

		}

		iter++;
	}

	return 0;
}

int CSPlayerFunc::L_getregionbackpos(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
	{
		lua_pushnumber(L, -1);
		return 1;
	}

	if(g_Script.m_pPlayer->m_ParentRegion)
	{
		lua_pushnumber(L, g_Script.m_pPlayer->m_ParentRegion->m_wReLiveRegionID);
		lua_pushnumber(L, g_Script.m_pPlayer->m_ParentRegion->m_ptReLivePoint.x );
		lua_pushnumber(L, g_Script.m_pPlayer->m_ParentRegion->m_ptReLivePoint.y );
		return 3;
	}

	lua_pushnumber(L, -1);
	return 1;
}

int CSPlayerFunc::L_subpoint(lua_State *L)
{
	return 0;
}

int CSPlayerFunc::L_getpoint(lua_State *L)
{
	// 	if(g_Script.m_pPlayer == NULL)
	//     {
	// 		lua_pushnumber(L, -1);
	//         return 1;
	//     }
	// 	
	// 	WORD wType = static_cast<WORD>(lua_tonumber(L, 1));
	//     int  nNumber = -1;
	// 
	// 	switch (wType)
	// 	{
	// 	case 0://SQSetPointMsg::EBAN_EN:   （手太阳）
	// 		{
	// 			nNumber = g_Script.m_pPlayer->m_Property.m_wEN;
	// 		}
	// 		break;
	// 	case 1://SQSetPointMsg::EBAN_ST:  （足太阴）
	// 		{
	// 			nNumber = g_Script.m_pPlayer->m_Property.m_wST;
	// 		}
	// 		break;
	// 	case 2://SQSetPointMsg::EBAN_IN:  （手少阳）
	// 		{
	// 			nNumber = g_Script.m_pPlayer->m_Property.m_wIN;
	// 		}
	// 		break;
	// 	case 3://SQSetPointMsg::EBAN_AG:  （足少阴）
	// 		{
	// 			//nNumber =  g_Script.m_pPlayer->m_Property.m_wAG;
	// 		}
	// 		break;
	// 	case 4://SQSetPointMsg::EBAN_LU:
	// 		{
	// 			nNumber = g_Script.m_pPlayer->m_Property.m_wLU;
	// 		}
	// 		break;
	// 
	// 	default:
	// 		break;
	// 	}
	// 
	// 	lua_pushnumber(L, nNumber);
	// 	return 1;
	return 0;
}

int CSPlayerFunc::L_getcurpoint(lua_State *L)
{
	return 0;
}

int CSPlayerFunc::L_getmoneypoint(lua_State *L)
{
	// 	if(g_Script.m_pPlayer == NULL)
	//     {
	// 		lua_pushnumber(L, -1);
	//         return 1;
	//     }
	// 
	// 	lua_pushnumber(L, g_Script.m_pPlayer->m_Property.m_dwMoneyPoint );
	// 	return 1;
	return 0;
}

int CSPlayerFunc::L_getxydpoint(lua_State *L)
{
	// 	if(g_Script.m_pPlayer == NULL)
	//     {
	// 		lua_pushnumber(L, -1);
	//         return 1;
	//     }
	// 
	// 	lua_pushnumber(L, g_Script.m_pPlayer->m_Property.m_dwXYDPoint );
	// 
	//     SQGetEQVPointMsg msg;
	//     msg.type = EEQ_XYDPOINT;
	// 
	//     try
	//     {
	//         lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
	//         slm( g_Script.m_pPlayer->GetAccount() );
	//         SendToLoginServer( &msg, long(sizeof(msg) - slm.EndEdition()) );
	//     }
	//     catch ( lite::Xcpt & )
	//     {
	//     }
	// 
	// 	return 1;

	return 0;
}

/** 获取赠宝：第一次点击时是得到缓存中的数据，
要第二次点击才能获取数据库中真正的数据，
但是两次之间的间隔时间不能太短...
*/
int CSPlayerFunc::L_getgiftcoupon(lua_State *L)
{
	// 	if(g_Script.m_pPlayer == NULL)
	//     {
	// 		lua_pushnumber(L, -1);
	//         return 1;
	//     }
	//  
	// 	lua_pushnumber(L, g_Script.m_pPlayer->m_Property.m_dwGiftCoupon );
	// 
	//     SQGetEQVPointMsg msg;
	//     msg.type = EEQ_GIFTCOUPON_NEWXYD;
	// 
	//     try
	//     {
	//         lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
	//         slm( g_Script.m_pPlayer->GetAccount() );
	//         SendToLoginServer( &msg, long(sizeof(msg) - slm.EndEdition()) );
	//     }
	//     catch ( lite::Xcpt & )
	//     {
	//     }
	// 
	// 	return 1;

	return 0;
}

/** 设置赠宝,为了安全起见，一次最多能扣除或者增加 10000个
*/
int CSPlayerFunc::L_setgiftcoupon( lua_State *L )
{
	// 	int value = static_cast<int>( lua_tonumber( L, 1 ) );
	// 	if( value > 10000 || value < -10000 )
	// 		return lua_pushnumber(L, -1), 1;
	// 
	// 	SQDecEQVPointMsg eqvmsg;
	// 	eqvmsg.type = EEQ_GIFTCOUPON_NEWXYD;
	// 	eqvmsg.dwEQVPoint = value;
	// 	eqvmsg.consumable = ECT_BUYITEM;
	// 	eqvmsg.nIndex = 0;
	//     eqvmsg.nNumber = 1;
	// 	eqvmsg.nLevel = g_Script.m_pPlayer->m_byLevel;
	// 
	//     try
	//     {
	//         lite::Serializer slm( eqvmsg.streamData, sizeof( eqvmsg.streamData ) );
	// 		slm( g_Script.m_pPlayer->GetAccount() );
	//         SendToLoginServer( &eqvmsg, ( long )( sizeof(eqvmsg) - slm.EndEdition() ) );
	//     }
	//     catch ( lite::Xcpt & )
	//     {
	//         return FALSE;
	//     }
	// 
	// 	return 1;
	return 0;
}

int CSPlayerFunc::L_notifymoneypoint(lua_State *L)
{
	// 	if(g_Script.m_pPlayer == NULL)
	//     {
	//         return 0;
	//     }
	// 
	// 	DWORD dwMoneyPoint = static_cast<DWORD>(lua_tonumber(L, 1));
	// 	
	// 	if(g_Script.m_pPlayer->m_Property.m_dwMoneyPoint >= dwMoneyPoint)
	// 	{
	// 		//g_Script.m_pPlayer->m_Property.m_dwMoneyPoint -= dwMoneyPoint;
	// 
	// 		SQRefreshMoneyPointMsg msg;
	// 		ZeroMemory(&msg.mpn, sizeof(SMoneyPointNotify));
	// 		
	// 		msg.dwMoneyPoint = g_Script.m_pPlayer->m_Property.m_dwMoneyPoint;
	// 		msg.mpn.dwOprate = static_cast<DWORD>(lua_tonumber(L, 2));
	// 		msg.mpn.dwMoneyPoint = dwMoneyPoint;
	// 
	// 		switch(msg.mpn.dwOprate)
	// 		{
	// 		case SMoneyPointNotify::OPT_EXCHANGEITEM:
	// 			{
	// 				msg.mpn.ATT.ITEMS.ItemID[0] = static_cast<DWORD>(lua_tonumber(L, 3));
	// 				msg.mpn.ATT.ITEMS.ItemID[1] = static_cast<DWORD>(lua_tonumber(L, 4));
	// 				msg.mpn.ATT.ITEMS.ItemID[2] = static_cast<DWORD>(lua_tonumber(L, 5));
	// 				msg.mpn.ATT.ITEMS.ItemID[3] = static_cast<DWORD>(lua_tonumber(L, 6));
	// 				msg.mpn.ATT.ITEMS.ItemID[4] = static_cast<DWORD>(lua_tonumber(L, 7));
	// 
	// 			}
	// 			break;
	// 
	// 		case SMoneyPointNotify::OPT_EXCHANGETELE:
	// 			{
	// 				msg.mpn.ATT.TELE.dwTeleID = static_cast<DWORD>(lua_tonumber(L, 3));
	// 				msg.mpn.ATT.TELE.dwTeleNum = static_cast<DWORD>(lua_tonumber(L, 4));	
	// 			}
	// 			break;
	// 
	//         case SMoneyPointNotify::OPT_EXBOTH:
	//             {
	//                 msg.mpn.ATT.EXBOTH.dwItemID = static_cast<DWORD>(lua_tonumber(L, 3));
	//                 msg.mpn.ATT.EXBOTH.dwTeleID = static_cast<DWORD>(lua_tonumber(L, 4));
	//                 msg.mpn.ATT.EXBOTH.dwTeleNum = static_cast<DWORD>(lua_tonumber(L, 5));
	//             }
	//             break;
	// 
	// 		default:
	// 			return 0;
	// 		}
	// 
	//         try
	//         {
	//             lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
	//             slm( g_Script.m_pPlayer->GetAccount() );
	//             SendToLoginServer(&msg, long(sizeof(msg) - slm.EndEdition()) );
	//         }
	//         catch ( lite::Xcpt & )
	//         {
	//         }
	// 	}
	// 	else
	// 	{
	// 		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, " 账号积分不足！");
	// 	}

	return 0;
}

int CSPlayerFunc::L_settelergyproficM(lua_State *L)
{
	// 	if ( g_Script.m_pPlayer == NULL )
	//     {
	//         return 0;
	//     }
	// 
	//     DWORD dwID = static_cast<DWORD>(lua_tonumber(L, 1));
	//     int iNum = static_cast<int>(lua_tonumber(L, 2));
	// 
	//     BOOL result = 0;
	// 
	//     if ( iNum > 0 )
	//         result = g_Script.m_pPlayer->SetTelergyProficM((WORD)dwID, iNum, GetNpcScriptInfo( "脚本SetTelergyProficM添加" ));
	//     else if ( iNum < 0 )
	//     {
	//         for ( int i=0; i<MAX_EQUIPTELERGY; i++ )
	//         {
	//             STelergy &t = g_Script.m_pPlayer->m_Property.m_Telergy[ i ];
	//             if ( t.wTelergyID == dwID && t.byTelergyLevel != 0 )
	//             {
	//                 result = g_Script.m_pPlayer->ReduceTelergyVal( i, -iNum, 1, GetNpcScriptInfo( "脚本SetTelergyProficM减少" ) );
	//                 break;
	//             }
	//         }
	//     }
	// 
	//     lua_pushnumber( L, result );
	//     return 1;

	return 0;
}

int CSPlayerFunc::L_getmymcinfo(lua_State *L)
{
	return 0;

	/* if(g_Script.m_pPlayer == NULL)
	return 0;

	SQRequestMCInfoMsg msg;
	msg.wType = SQRequestMCInfoMsg::RT_ME;
	_EMCWorkInstance->Entry(&msg, g_Script.m_pPlayer);*/

	//g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(SQRequestMCInfoMsg));

	return 0;
}

void CSPlayerFunc::CreateShadow()
{
	g_Script.RegisterFunction("GetPlayerVenationState", L_getplayervenationstate);
	g_Script.RegisterFunction("DamageToVenation", L_damagetovenation);
	g_Script.RegisterFunction("ChangeSkill", L_changeskill);
	g_Script.RegisterFunction("GetRecvMoney", L_getrecvmoney);
	g_Script.RegisterFunction("GetBackItemsInfo", L_getbackitemsinfo);
	g_Script.RegisterFunction("CheckRecvItem", L_checkrecvitem);
	g_Script.RegisterFunction("GetSOneItemNum", L_getsoneitemnum);
	g_Script.RegisterFunction("getplayercurpos", L_getplayercurpos);
	g_Script.RegisterFunction("GetRegionRetPos", L_getregionretpos);
	g_Script.RegisterFunction("SetBuyNpc", L_setbuynpc);
	g_Script.RegisterFunction("DelAllSItems", L_delallrecvitems);
	g_Script.RegisterFunction("RestoreAllSItems", L_restoreallrecvitems);
	g_Script.RegisterFunction("ClearAllSItems", L_clearallsitems);
	g_Script.RegisterFunction("IsMonsterMutate", L_ismonstermutate);
	g_Script.RegisterFunction("SetMutate", L_setmutate);
	g_Script.RegisterFunction("GetMutateID", L_getmutateid);
	g_Script.RegisterFunction("AddMultipleTime", L_addmultipletime);
	g_Script.RegisterFunction("InviteTeamToHome", L_inviteteamtohome);
	g_Script.RegisterFunction("SetSpouse", L_setspouse);
	g_Script.RegisterFunction("DelSpouse", L_delspouse);
	g_Script.RegisterFunction("EnterGaminghouse", L_entergaminghouse);
	g_Script.RegisterFunction("GetRoomID", L_getroomid);
	g_Script.RegisterFunction("GetFriendDegree", L_getfrienddegree);
	g_Script.RegisterFunction("GetMarryDegree", L_getmarrydegree);
	g_Script.RegisterFunction("SetMarryDate", L_setmarrydate);
	g_Script.RegisterFunction("GetMarryDate", L_getmarrydate);
	g_Script.RegisterFunction("AddSkillProficiency", L_addskillproficiency);
	g_Script.RegisterFunction("ClearPlayerInfo", L_ClearPlayerInfo);
	g_Script.RegisterFunction("SetTaskInfo", L_settaskinfo);
	g_Script.RegisterFunction("SetMenuInfo", L_setmenuinfo);
	g_Script.RegisterFunction("CanAddItems", L_canadditems);
	g_Script.RegisterFunction("GetTaskStatus", L_gettaskstatus);
	g_Script.RegisterFunction("SetTaskData", L_settaskdata);
	g_Script.RegisterFunction("SetTaskFlag", L_settaskflag);
	g_Script.RegisterFunction("SynTaskStatus", L_syntaskstatus);
	g_Script.RegisterFunction("DelRoleTask", L_delroletask);
	g_Script.RegisterFunction("DelTaskMap", L_DelTaskMap);
	g_Script.RegisterFunction("IsTaskFlagComplete", L_istaskflagcomplete);
	g_Script.RegisterFunction("GetRdTasksNum",L_getrdtasksNum);
	g_Script.RegisterFunction("GetCurrentNpcTaskID",L_GetCurrentNpcTaskID);
	g_Script.RegisterFunction("InsertPlayerRdTaskInfo",L_InsertPlayerRdTaskInfo);
	g_Script.RegisterFunction("GetRdNpcNum",L_GetRdNpcNum);
	g_Script.RegisterFunction("OnXKLProcess",L_OnXKLProcess);
	g_Script.RegisterFunction("AddXKLUseNum",L_AddXKLUseNum);
	g_Script.RegisterFunction("GetXKLUseNum",L_GetXKLUseNum);
	g_Script.RegisterFunction("OnHandleCycTask",L_OnHandleCycTask);
	g_Script.RegisterFunction("GetHeadIDByStr",L_GetHeadIDByStr);
	g_Script.RegisterFunction("PrintXKLLOG",L_PrintXKLLOG);
	g_Script.RegisterFunction("CheckRoleTaskS", L_checkroletasks);
	g_Script.RegisterFunction("CheckRoleTaskN", L_checkroletaskn);
	g_Script.RegisterFunction("AddRoleTaskElementS", L_addroletaskelements);
	g_Script.RegisterFunction("AddRoleTaskElementN", L_addroletaskelementn);
	g_Script.RegisterFunction("SetRoleTaskElementS", L_setroletaskelements);
	g_Script.RegisterFunction("SetRoleTaskElementN", L_setroletaskelementn);
	g_Script.RegisterFunction("GetRoleTaskElementS", L_getroletaskelements);
	g_Script.RegisterFunction("GetRoleTaskElementN", L_getroletaskelementn);
	g_Script.RegisterFunction("CleanTask", L_cleantask);
	g_Script.RegisterFunction("GetCurMoney", L_getcurmoney);
	g_Script.RegisterFunction("GetFactionName", L_getfactionname);
	g_Script.RegisterFunction("IsFactionMaster", L_isfactionmaster);
	g_Script.RegisterFunction("SendTeamInfoToAround", L_sendteaminfotoaround);
	g_Script.RegisterFunction("PutAroundTeamTo", L_putaroundteamto);
	g_Script.RegisterFunction("PutAllPlayerA", L_putallplayerA);
	g_Script.RegisterFunction("GetRegionBackPos", L_getregionbackpos);
	g_Script.RegisterFunction("AddSubTeamTask", L_addsubteamtask);
	g_Script.RegisterFunction("SetFactionSysTitleID", L_setfactionsystitleid);
	g_Script.RegisterFunction("SubPoint", L_subpoint);
	g_Script.RegisterFunction("GetPoint", L_getpoint);
	g_Script.RegisterFunction("GetCurPoint", L_getcurpoint);
	g_Script.RegisterFunction("NotifyMoneyPoint", L_notifymoneypoint);
	g_Script.RegisterFunction("GetMoneyPoint", L_getmoneypoint);
	g_Script.RegisterFunction("GetXYDPoint", 	L_getxydpoint);
	g_Script.RegisterFunction("GetGiftCoupon", 	L_getgiftcoupon);
	g_Script.RegisterFunction("SetGiftCoupon", L_setgiftcoupon);
	g_Script.RegisterFunction("SetTelergyProficM", L_settelergyproficM);
	g_Script.RegisterFunction("GetMyMCinfo", L_getmymcinfo);
	g_Script.RegisterFunction("GetElapsedtimesFac", L_getelapsedtimesFac);
	g_Script.RegisterFunction("UpdateGiftCoupon", L_updateGiftCoupon);
	g_Script.RegisterFunction("SetWulinChief", L_setwulinchief);
	g_Script.RegisterFunction("SendRoleTaskInfo", L_SendRoleTaskInfo);
	g_Script.RegisterFunction("OnUpdateTask", L_OnUpdateTask);
	g_Script.RegisterFunction("SetTaskStatus", L_settaskstatus);

	// 侠义道3接口
	g_Script.RegisterFunction("GetPlayerMapInfo", L_GetPlayerMapInfo);
	g_Script.RegisterFunction("OnPlayerCycTask",L_OnPlayerCycTask);
}

int CSPlayerFunc::L_GetPlayerMapInfo(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	lua_pushnumber(L, g_Script.m_pPlayer->m_ParentRegion->m_wMapID);
	lua_pushnumber(L, g_Script.m_pPlayer->m_curTileX);
	lua_pushnumber(L, g_Script.m_pPlayer->m_curTileY);

	return 3;
}

//----------------------------------------------------------------------------------------------
// 其他对象相关的脚本函数
int CSOtherFunc::L_makenewtrap(lua_State *L)
{
	WORD wRegionID = static_cast<WORD>(lua_tonumber(L, 1));

	DWORD dwRegionGID = static_cast<DWORD>(lua_tonumber(L, 3));

	CRegion *pRegion = NULL;

	if (dwRegionGID == 0)
	{
		pRegion = (CRegion *)FindRegionByID(wRegionID)->DynamicCast(IID_REGION);
	}
	else
	{
		LPIObject FindRegionByGID(DWORD GID);
		pRegion = (CRegion *)FindRegionByGID(dwRegionGID)->DynamicCast(IID_REGION);
	}

	if(pRegion == NULL) 
	{
		lua_pushnumber(L ,0);
		return 1;
	}

	WORD wMax = static_cast<WORD>(lua_tonumber(L, 2));
	if(pRegion->MakeNewTrap((BYTE)wMax))
	{
		lua_pushnumber(L ,1);
	}
	else
		lua_pushnumber(L, 0);

	return 1;
}

int CSOtherFunc::L_settrap(lua_State *L)
{
	WORD wRegionID = static_cast<WORD>(lua_tonumber(L, 1));

	DWORD dwRegionGID = static_cast<DWORD>(lua_tonumber(L, 7));

	CRegion *pRegion = NULL;

	if (dwRegionGID == 0)
	{
		pRegion = (CRegion *)FindRegionByID(wRegionID)->DynamicCast(IID_REGION);
	}
	else
	{
		LPIObject FindRegionByGID(DWORD GID);
		pRegion = (CRegion *)FindRegionByGID(dwRegionGID)->DynamicCast(IID_REGION);
	}

	if(pRegion == NULL) 
	{
		lua_pushnumber(L, 0);
		return 1;
	}

	WORD wOrder = static_cast<WORD>(lua_tonumber(L, 2));

	WORD x = 0, y = 0;
	CTrigger Trap;
	Trap.m_dwType = static_cast<DWORD>(lua_tonumber(L, 3));
	if ( Trap.m_dwType == CTrigger::TT_MAPTRAP_EVENT )
	{
		x = Trap.SMapEvent.wCurX = static_cast<WORD>(lua_tonumber(L, 4));
		y = Trap.SMapEvent.wCurY = static_cast<WORD>(lua_tonumber(L, 5));
		Trap.SMapEvent.dwScriptID = static_cast<DWORD>(lua_tonumber(L, 6));
		reinterpret_cast< lite::lua_variant& >( Trap.SMapEvent.GetVariant() ).get( L, 8 );
	}
	else if ( Trap.m_dwType == CTrigger::TT_CHANGE_REGION )
	{
		Trap.SChangeRegion.wNewRegionID = static_cast<WORD>(lua_tonumber(L, 4));
		Trap.SChangeRegion.wStartX = static_cast<WORD>(lua_tonumber(L, 5));
		Trap.SChangeRegion.wStartY = static_cast<WORD>(lua_tonumber(L, 6));
		Trap.SChangeRegion.dwNewRegionGID = static_cast<DWORD>(lua_tonumber(L, 10));
		x = static_cast<WORD>(lua_tonumber(L, 8));
		y = static_cast<WORD>(lua_tonumber(L, 9));
	}
	else if ( Trap.m_dwType != 0 )
		return lua_pushnumber(L, 0), 1;

	if(pRegion->SetTrap(/*(BYTE) 会导致截断*/wOrder, x, y, &Trap))
	{
		lua_pushnumber(L, 1);

		// 避免Trap析构导致已经设置成功的lite::variant失效！
		memset( &Trap, 0, sizeof( Trap ) );
	}
	else
		lua_pushnumber(L, 0);

	return 1;
}

int CSOtherFunc::L_gettrapnum(lua_State *L)
{
	WORD wRegionID = static_cast<WORD>(lua_tonumber(L, 1));

	CRegion *pRegion = (CRegion *)FindRegionByID(wRegionID)->DynamicCast(IID_REGION);
	if(pRegion == NULL) 
	{
		return 0;
	}

	lua_pushnumber(L, pRegion->GetMaxTrap());

	return 1;

}

int CSOtherFunc::L_seteffectmsg(lua_State *L)
{
	return 0;
}

int CSOtherFunc::L_setmail(lua_State *L)
{
	POSTPARAM post;
	ZeroMemory(&post, sizeof(POSTPARAM));

	dwt::strcpy(post.szUserName, static_cast<const char*>(lua_tostring(L, 1)), 32);
	dwt::strcpy(post.szPassWord, static_cast<const char*>(lua_tostring(L, 2)), 32);
	dwt::strcpy(post.szSender, static_cast<const char*>(lua_tostring(L, 3)), 32);
	dwt::strcpy(post.szRecipient, static_cast<const char*>(lua_tostring(L, 4)), 32);
	dwt::strcpy(post.szSubject, static_cast<const char*>(lua_tostring(L, 5)), 32);
	dwt::strcpy(post.szAttachment, static_cast<const char*>(lua_tostring(L, 6)), MAX_PATH);
	dwt::strcpy(post.szSMTPServer, static_cast<const char*>(lua_tostring(L, 7)), 32);
	post.wPort = static_cast<WORD>(lua_tonumber(L, 8));

	if(post.szSMTPServer[0] == '\0')
		dwt::strcpy(post.szSMTPServer, "mail.dreamwork.cn", 32);
	if(post.wPort == 0)
		post.wPort = 25;

	//g_PostParamQueue.push(post);
	PushMailParam(post);

	return 0;
}

int CSOtherFunc::L_putallplayer(lua_State *L)
{

	DWORD wRegionID = static_cast<DWORD>(lua_tonumber(L, 1));
	DWORD wNewRegionID = static_cast<DWORD>(lua_tonumber(L, 2));
	WORD wNewX = static_cast<WORD>(lua_tonumber(L, 3));
	WORD wNewY = static_cast<WORD>(lua_tonumber(L, 4));
	LPCSTR lpszName = static_cast<const char*>(lua_tostring(L, 5));
	bool isExcept = ( lua_toboolean( L, 6 ) == 1 );

	if ( wRegionID < 46 )
		return 0;


	CRegion *pRegion = NULL;
	if(wRegionID > 0x40000000)
	{
		pRegion = (CRegion *)FindRegionByGID(
			wRegionID)->DynamicCast(IID_REGION);

	}
	else
	{
		pRegion = (CRegion *)FindRegionByID(
			wRegionID)->DynamicCast(IID_REGION);
	}

	if(pRegion == NULL)
	{
		rfalse(2, 1, "需要传送所有玩家的场景不存在!");
		return 0;
	}

	// <=46号的场景中,不能使用putallplayer
	if ( pRegion->m_wRegionID <= 46 )
		return 0;

	BOOL bPut = TRUE;
	check_list<LPIObject> templist(pRegion->m_PlayerList);

	check_list<LPIObject>::iterator iter = templist.begin();
	while (iter != templist.end())
	{
		CPlayer *pPlayer = (CPlayer *)(*iter)->DynamicCast(IID_PLAYER);
		if ( pPlayer )
		{
			if ( lpszName )
				if ( dwt::strcmp( pPlayer->m_Property.m_szTongName, lpszName, sizeof( pPlayer->m_Property.m_szTongName ) ) == 0 )
					bPut = isExcept;
				else
					bPut = !isExcept;

			if(bPut)
			{
				if(wNewRegionID > 0x40000000)
					PutPlayerIntoDestRegion(pPlayer, 0, wNewX, wNewY, wNewRegionID);
				else
					PutPlayerIntoDestRegion(pPlayer, (WORD)wNewRegionID, wNewX, wNewY);
			}
		}

		bPut = TRUE;
		iter++;
	}

	return 0;
}

int CSOtherFunc::L_pushspouse(lua_State *L)
{
	DWORD dwID = static_cast<DWORD>(lua_tonumber(L, 1));
	LPCSTR lpszName = static_cast<const char*>(lua_tostring(L, 2));
	string name = "";
	if(lpszName != NULL)
		name = lpszName;

	PushSpouse(dwID, name);
	return 0;
}

int CSOtherFunc::L_setregionexper(lua_State *L)
{
	DWORD dwRegionID = static_cast<DWORD>(lua_tonumber(L, 1));
	WORD  wData = static_cast<WORD>(lua_tonumber(L, 2));

	CRegion *pRegion = NULL;
	if(dwRegionID > 0x40000000)
	{
		pRegion = (CRegion *)FindRegionByGID(
			dwRegionID)->DynamicCast(IID_REGION);

	}
	else
	{
		pRegion = (CRegion *)FindRegionByID(
			dwRegionID)->DynamicCast(IID_REGION);
	}

	if(pRegion == NULL)
	{
		// lua_pushnumber(L, 0);
		return 0;
	}

	pRegion->SetExperMul(wData);

	return 0;

}

int CSOtherFunc::L_getregionexper(lua_State *L)
{
	DWORD dwRegionID = static_cast<DWORD>(lua_tonumber(L, 1));

	CRegion *pRegion = NULL;
	if(dwRegionID > 0x40000000)
	{
		pRegion = (CRegion *)FindRegionByGID(
			dwRegionID)->DynamicCast(IID_REGION);

	}
	else
	{
		pRegion = (CRegion *)FindRegionByID(
			dwRegionID)->DynamicCast(IID_REGION);
	}

	if(pRegion == NULL)
	{
		lua_pushnumber(L, 0);
		return 1;
	}

	lua_pushnumber(L, pRegion->GetExperMul());
	return 1;
}

int CSOtherFunc::L_getlocalserverid(lua_State *L)
{
	WORD wID = GetServerID();
	lua_pushnumber(L, wID);
	return 1;
}

int CSOtherFunc::L_getserverid(lua_State *L)
{
	LPCSTR lpszName = static_cast<const char*>(lua_tostring(L, 1));
	if(lpszName == NULL)
	{
		lua_pushnumber(L, 0);
		return 1;
	}
	char szName[32] = {0}; 
	dwt::strcpy(szName, lpszName, 32);
	WORD wNum = GetServerIDFromName(szName);
	lua_pushnumber(L, wNum);

	return 1;
}

int CSOtherFunc::L_setgaminginfo(lua_State *L)
{
	GAMING_WAITTIME = static_cast<int>(lua_tonumber(L, 1));
	GAMING_RATE = static_cast<int>(lua_tonumber(L, 2));


	if(GAMING_WAITTIME <= 0)
		GAMING_WAITTIME = 20;

	if(GAMING_RATE <= 0)
		GAMING_RATE = 99;

	return 0;
}

int CSOtherFunc::L_tipforbottom(lua_State *L)
{
	SAScriptBulletinMsg msg;
	//msg.byType = SAChatGlobalMsg::ECT_SPECIAL;
	//dwt::strcpy(msg.cTalkerName, static_cast<const char*>(lua_tostring(L, 1)), sizeof(msg.cTalkerName));
	dwt::strcpy(msg.cChatData, static_cast<const char*>(lua_tostring(L, 1)), sizeof(msg.cChatData));

	//SendMessageToAllServer(&msg,msg.GetMySize());

	return 0;
}

int CSOtherFunc::L_sendmessagetoallfactionmember(lua_State *L)
{
	LPCSTR lpszTalker = static_cast<const char*>(lua_tostring(L, 1));
	LPCSTR lpszData = static_cast<const char*>(lua_tostring(L, 2));

	if(lpszTalker && lpszData)
	{

		LPCSTR lpszFaction = static_cast<const char*>(lua_tostring(L, 3));
		DWORD dwRegion = static_cast<DWORD>(lua_tonumber(L, 4));

		if(lpszFaction)
		{
			SAChatFactionMsg msg; 
			dwt::strcpy(msg.cTalkerName, lpszTalker, CONST_USERNAME);
			dwt::strcpy(msg.cChatData, lpszData, MAX_CHAT_LEN);
			msg.wResult = 1;

			CFactionManager::SendFactionAllServer( lpszFaction, SQSendServerMsg::TALK, &msg, msg.GetMySize(), (WORD)dwRegion ); 

		}
	}

	return 0;
}

int CSOtherFunc::L_setregionpkx(lua_State *L)
{
	DWORD dwRegionID = static_cast<DWORD>(lua_tonumber(L, 1));
	DWORD dwPKXValue = static_cast<DWORD>(lua_tonumber(L, 2));

	CRegion *pRegion = NULL;
	if(dwRegionID > 0x40000000)
	{
		pRegion = (CRegion *)FindRegionByGID(
			dwRegionID)->DynamicCast(IID_REGION);

	}
	else
	{
		pRegion = (CRegion *)FindRegionByID(
			dwRegionID)->DynamicCast(IID_REGION);
	}

	if(pRegion == NULL)
	{
		return 0;
	}

	pRegion->m_dwPKX = dwPKXValue;

	return 0;
}

void CSOtherFunc::CreateShadow()
{
	g_Script.RegisterFunction("MakeNewTrap", L_makenewtrap);
	g_Script.RegisterFunction("SetTrap", L_settrap);
	g_Script.RegisterFunction("GetTrapNum", L_gettrapnum);
	g_Script.RegisterFunction("SetEffectMsg", L_seteffectmsg);
	g_Script.RegisterFunction("SetMail", L_setmail);
	g_Script.RegisterFunction("PutAllPlayer", L_putallplayer);
	g_Script.RegisterFunction("PushSpouse", L_pushspouse);
	g_Script.RegisterFunction( "FillSpouseList",L_fillspouselist);
	g_Script.RegisterFunction( "SetRegionExper",L_setregionexper);
	g_Script.RegisterFunction( "GetRegionExper",L_getregionexper);
	g_Script.RegisterFunction( "GetLocalServerID",L_getlocalserverid);
//	g_Script.RegisterFunction( "GetServerID",L_getserverid);
	g_Script.RegisterFunction( "SetGamingInfo",L_setgaminginfo);
	g_Script.RegisterFunction("LoadTaskDesc", L_loadtaskdesc);
	g_Script.RegisterFunction("LoadTaskDescA", L_loadtaskdescA);
	g_Script.RegisterFunction("TipForBottom", L_tipforbottom);
	g_Script.RegisterFunction("SendMessageToAllFactionMember", L_sendmessagetoallfactionmember);
	g_Script.RegisterFunction("SetRegionPKX", L_setregionpkx);
	g_Script.RegisterFunction("SetCommonRRate", L_setcommonrrate);
	g_Script.RegisterFunction("EmptyRRateMap", L_emptyrratemap);
	g_Script.RegisterFunction("SetRegionRate", L_setregionrate);
	g_Script.RegisterFunction("EmptyRMoneyMap", L_emptyrmoneymap);
	g_Script.RegisterFunction("SetCountRegion", L_setcountregion);
	g_Script.RegisterFunction("GetRegionRMoney", L_getregionrmoney);
	g_Script.RegisterFunction("ZeroRMoneyMap", L_zerormoneymap);
}

///// 场景税率相关
std::map<DWORD, float> g_RRateMap;				// 场景税率表
std::map<DWORD, DWORD> g_RTotalMoneyMap;        // 场景税收总表

int CSOtherFunc::L_emptyrratemap(lua_State *L)
{
	DWORD dwRegionID = static_cast<DWORD>(lua_tonumber(L, 1));
	if(dwRegionID)
	{
		std::map<DWORD, float>::iterator it;
		it = g_RRateMap.find(dwRegionID);
		if(it != g_RRateMap.end())
		{
			g_RRateMap.erase(it);
		}
	}
	else
		g_RRateMap.clear();

	return 0;	
}

int CSOtherFunc::L_setregionrate(lua_State *L)
{
	DWORD dwRegionID = static_cast<DWORD>(lua_tonumber(L, 1));
	float fRate = static_cast<float>(lua_tonumber(L, 2));
	if(dwRegionID)
	{
		g_RRateMap[dwRegionID] = fRate;
	}

	return 0;
}

int CSOtherFunc::L_setcountregion(lua_State *L)
{
	DWORD dwRegionID = static_cast<DWORD>(lua_tonumber(L, 1));
	if(dwRegionID)
	{
		g_RTotalMoneyMap[dwRegionID] = 0;
	}

	return 0;
}

int CSOtherFunc::L_emptyrmoneymap(lua_State *L)
{
	DWORD dwRegionID = static_cast<DWORD>(lua_tonumber(L, 1));
	if(dwRegionID)
	{
		std::map<DWORD, DWORD>::iterator it;
		it = g_RTotalMoneyMap.find(dwRegionID);
		if(it != g_RTotalMoneyMap.end())
		{
			g_RTotalMoneyMap.erase(it);
		}
	}
	else
		g_RTotalMoneyMap.clear();

	return 0;
}

int CSOtherFunc::L_setcommonrrate(lua_State *L)
{
	float fRate = static_cast<float>(lua_tonumber(L, 1));
	s_fRegionRate = fRate;

	return 0;
}

int CSOtherFunc::L_getregionrmoney(lua_State *L)
{
	DWORD dwRegionID = static_cast<DWORD>(lua_tonumber(L, 1));
	std::map<DWORD, DWORD>::iterator it =  g_RTotalMoneyMap.find(dwRegionID);
	if(it != g_RTotalMoneyMap.end())
	{
		lua_pushnumber(L, it->second);
		return 1;
	}

	lua_pushnumber(L, -1);
	return 1;
}

int CSOtherFunc::L_zerormoneymap(lua_State *L)
{
	DWORD dwRegionID = static_cast<DWORD>(lua_tonumber(L, 1));
	if(dwRegionID)
	{
		std::map<DWORD, DWORD>::iterator it;
		it = g_RTotalMoneyMap.find(dwRegionID);
		if(it != g_RTotalMoneyMap.end())
		{
			it->second = 0;
		}
	}

	return 0;
}

float FindRgionInRateMap(DWORD dwRegionID)
{
	if(dwRegionID)
	{
		std::map<DWORD, float>::iterator it;
		it = g_RRateMap.find(dwRegionID);
		if(it != g_RRateMap.end())
		{
			return it->second;
		}
	}

	return 0L;
}

void AddMoneyIfINRMap(DWORD dwRegionID, DWORD dwValue)
{
	if(dwRegionID)
	{
		std::map<DWORD, DWORD>::iterator it;
		it = g_RTotalMoneyMap.find(dwRegionID);
		if(it != g_RTotalMoneyMap.end())
		{
			it->second += dwValue;
		}
	}
}

RTRESULT CRoleTask::PushFlag(STaskFlag &flag)
{
	if (m_flagList.size() >= 8)
		return R_FAILED;

	m_flagList.push_back(flag);

	return R_TOK;
}

RTRESULT CRoleTask::DelFlg(STaskFlag *pFlag)
{
	if (pFlag == NULL)
		return R_FAILED;

	TELIST::iterator it = m_flagList.begin();
	for(; it != m_flagList.end();)
	{
		if (memcmp(pFlag, &*it, sizeof(STaskFlag)) == 0)
		{
			m_flagList.erase(it);
			return R_TOK;
		}

		it++;
	}

	return R_FAILED;
}

STaskFlag* CRoleTask::GetFlag(DWORD index)
{
	TELIST::iterator it = m_flagList.begin();

	for (; it != m_flagList.end();)
	{
		if (it->m_Index == index)
			return &*it;

		++it;
	}

	return NULL;
}

/*
BOOL CRoleTask::GetTElementDesc(STaskFlag *pTElement, LPSTR lpszInfo)
{
if(pTElement == NULL || lpszInfo == NULL)
return FALSE;

LPTASKDESC desc = (CRoleTaskDesc::Instance())->GetTaskDesc(pTElement->wType, CRoleTaskDesc::DT_ELEMENT);
if(desc)
{
char szInfo[MAX_PATH] = {0};
char szFormat[MAX_PATH] = {0};

if(desc->Desc1 != "")
{
dwt::strcpy(szInfo, desc->Desc1.c_str(), (unsigned int)desc->Desc1.length());
dwt::_strncat(szInfo, pTElement->szName, sizeof(pTElement->szName));
}

if(desc->Desc2 != "")
{
dwt::_strncat(szInfo, desc->Desc2.c_str(), sizeof(desc->Desc2.length()));
_snprintf(szFormat, sizeof(szFormat), "%d", pTElement->dwValue);
dwt::_strncat(szInfo, szFormat, sizeof(szFormat));
}

if(desc->Desc3 != "")
{
dwt::_strncat(szInfo, desc->Desc3.c_str(), (unsigned int)desc->Desc3.length());
_snprintf(szFormat, sizeof(szFormat), "%d", pTElement->dwComplete);
dwt::_strncat(szInfo, szFormat, sizeof(szFormat));
}

if(desc->Desc4 != "")
{
dwt::_strncat(szInfo, desc->Desc4.c_str(), (unsigned int)desc->Desc4.length());
}

dwt::strcpy(lpszInfo, szInfo, sizeof(szInfo));
return TRUE;
}

return FALSE;
}//*/

void CRoleTaskManager::ClearAllRoleTask()
{
	for (RTLIST::iterator it = m_taskList.begin(); it != m_taskList.end(); ++it)
		it->m_flagList.clear();

	m_taskList.clear();
}

RTRESULT CRoleTaskManager::AddRoleTask(CRoleTask *pTask)
{
	if (m_taskList.size() >= 20)
		return R_TASKFULL;

	if (GetRoleTask(pTask->m_Index))
		return R_TASKIN;

	if (pTask)
	{
		m_taskList.push_back(*pTask);
		return R_TOK;
	}

	return R_FAILED;
}

RTRESULT CRoleTaskManager::DelRoleTask(CRoleTask *pTask)
{
	if (!pTask)
		return R_FAILED;

	RTLIST::iterator it = m_taskList.begin();

	for(; it != m_taskList.end();)
	{
		if (memcmp(pTask, &*it, sizeof(CRoleTask)) == 0)
		{
			m_taskList.erase(it);
			return R_TOK;
		}
		++it;
	}
	return R_FAILED;
}

int CRoleTaskManager::IsTaskFinished(CRoleTask *pTask)
{
	if (!pTask)
		return -1;

	if (pTask->m_flagList.empty())
		return -1;

	for (CRoleTask::TELIST::iterator it = pTask->m_flagList.begin(); it != pTask->m_flagList.end(); ++it)
	{
		if (TT_GIVE == it->m_Type && (0 == (*it).m_Complete || 1 == (*it).m_Complete))
			return 0;

		if (0 == (*it).m_Complete)
			return 0;
	}

	return 1;
}

CRoleTask *CRoleTaskManager::GetRoleTask(WORD Key)
{
	RTLIST::iterator it = m_taskList.begin();

	for(; it != m_taskList.end();)
	{
		if(it->m_Index == Key)
			return &*it;
		++it;
	}

	return 0;
}

std::list<CRoleTask> *CRoleTaskManager::GetRoleTask()
{
	if (m_taskList.size())
		return &m_taskList;

	return 0;
}

void CRoleTaskManager::SendRoleTaskInfo(SRoleTaskBaseMsg *pMsg, long Size,  CPlayer *pPlayer)
{
	if (pMsg == NULL || pPlayer == NULL)
		return;

	g_StoreMessage(pPlayer->m_ClientIndex, pMsg, Size);
}

void CRoleTaskManager::SendAllRoleTaskInfo(CPlayer *pPlayer)
{
	if (!pPlayer)
		return;

	for (RTLIST::iterator &it = m_taskList.begin(); it != m_taskList.end(); ++it)
	{
		SAAddRoleTask msg;
		msg.dwGlobalID		= pPlayer->GetGID();
		msg.wTaskID			= it->m_Index;
		msg.wFlagCount		= it->m_flagList.size();
		msg.m_CreateTime	= it->m_CreateTime;
		size_t FlagPos = 0;
		for (CRoleTask::TELIST::iterator &ti = it->m_flagList.begin(); ti != it->m_flagList.end(); ++ti)
		{
			/*if (it->m_Index == 1)
			{
			(*ti).m_Complete = 1;
			it->m_timelimit = 0xffffffff;
			if ((*ti).m_Type != TT_EMPTY)
			{
			(*ti).m_Type = TT_EMPTY;
			}
			}*/
			msg.w_flags[FlagPos++] = (*ti);
		}

		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAAddRoleTask));
		//g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg) - (8-msg.wFlagCount)*sizeof(STaskFlag));
	}

}

CRoleTaskDesc *CRoleTaskDesc::_pInstance = NULL;
CRoleTaskDesc *CRoleTaskDesc::Instance()
{
	if(_pInstance == NULL)
		_pInstance = new CRoleTaskDesc;

	return _pInstance;
}

CRoleTaskDesc::~CRoleTaskDesc()
{
	if(_pInstance)
	{
		delete _pInstance;
		_pInstance = NULL;
	}
}

void CRoleTaskDesc::PushTaskDesc(WORD wTypeKey, LPTASKDESC lpDesc, WORD wDescType )
{
	if(lpDesc == NULL)
		return;

	switch(wDescType)
	{
	case DT_TASK:
		_taskdescMap[wTypeKey] = *lpDesc;
		break;

	case DT_ELEMENT:
		_descMap[wTypeKey] = *lpDesc;
		break;

	default:
		return;
	}
}

LPTASKDESC CRoleTaskDesc::GetTaskDesc(WORD wTypeKey, WORD wDescType )
{
	//     TASKDESCMAP::iterator it, it_e;
	//     
	//     switch( wDescType )
	//     {
	//     case DT_TASK:
	//         it = _taskdescMap.find(wTypeKey);
	//         it_e = _taskdescMap.end();
	//         break;
	// 
	//     case DT_ELEMENT:
	//         it = _descMap.find(wTypeKey);
	//         it_e = _descMap.end();
	//         break;
	// 
	//     default:
	//         return NULL;
	//     }
	// 
	//     if(it != it_e)
	//         return &it->second;

	return NULL;
}

int CSPlayerFunc::L_settaskinfo(lua_State *L)
{
	WORD taskID = 0;
	WORD taskStatus = 0;
	if (!g_Script.m_pPlayer)
	{
		DWORD psid = static_cast<DWORD>(lua_tonumber(L, 1));
		g_Script.m_pPlayer = (CPlayer*)GetPlayerBySID(psid)->DynamicCast(IID_PLAYER);
		if (!g_Script.m_pPlayer)
		{
			return 0;
		}
		taskID = static_cast<WORD>(lua_tonumber(L, 2));
		taskStatus = static_cast<WORD>(lua_tonumber(L, 3));	// 任务状态
		if (0 == taskID || 0 == taskStatus || TS_COMPLETE == taskStatus)
		{
			rfalse(2, 1, "任务信息不正确！");
			return 0;
		}
		g_Script.m_pPlayer->m_TaskInfo.push_back(make_pair(taskID, taskStatus));
		g_Script.m_pPlayer = 0;
		return 1;
	}
	else
	{
		 taskID		 = static_cast<WORD>(lua_tonumber(L, 1));	// 任务ID
		 taskStatus  = static_cast<WORD>(lua_tonumber(L, 2));	// 任务状态
	}

	if (0 == taskID || 0 == taskStatus || TS_COMPLETE == taskStatus)
	{
		rfalse(2, 1, "任务信息不正确！");
		return 0;
	}

	g_Script.m_pPlayer->m_TaskInfo.push_back(make_pair(taskID, taskStatus));

	return 0;
}

int CSPlayerFunc::L_ClearPlayerInfo(lua_State *L)
{
	g_Script.m_pPlayer->m_TaskInfo.clear();
	g_Script.m_pPlayer->m_MenuInfo.clear();

	return 0;
}

int CSPlayerFunc::L_setmenuinfo(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD menuID = static_cast<WORD>(lua_tonumber(L, 1));

	if (0 == menuID)
		return 0;

	g_Script.m_pPlayer->m_MenuInfo.push_back(menuID);

	return 0;
}

int CSPlayerFunc::L_canadditems(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD  Operation	= static_cast<WORD> (lua_tonumber(L, 1));
	DWORD itemID	= static_cast<DWORD>(lua_tonumber(L, 2));
	WORD  itemNum	= static_cast<WORD> (lua_tonumber(L, 3));

	if (0 == Operation || 0 == itemID || 0 == itemNum)
		return 0;

	if (1 == Operation)
		g_Script.m_pPlayer->m_ItemInfo.push_back(SAddItemInfo(itemID, itemNum));
	else
	{
		if (g_Script.m_pPlayer->m_ItemInfo.empty())
		{
			lua_pushnumber(L, 1);
		}
		else
		{
			BOOL result = g_Script.m_pPlayer->CanAddItems(g_Script.m_pPlayer->m_ItemInfo);
			g_Script.m_pPlayer->m_ItemInfo.clear();
			lua_pushnumber(L, result);
		}

		return 1;
	}
	return 0;
}

int CSPlayerFunc::L_gettaskstatus(lua_State *L)
{
	WORD taskID = 0;
	if (!g_Script.m_pPlayer)
	{
		DWORD psid = static_cast<DWORD>(lua_tonumber(L, 1));
		g_Script.m_pPlayer = (CPlayer*)GetPlayerBySID(psid)->DynamicCast(IID_PLAYER);
		if (!g_Script.m_pPlayer)
		{
			return 0;
		}
		taskID = static_cast<WORD>(lua_tonumber(L, 2));
		lua_pushnumber(L, g_Script.m_pPlayer->GetTaskStatus(taskID));
		g_Script.m_pPlayer = 0;
		return 1;
	}
	else
	{
		taskID = static_cast<WORD>(lua_tonumber(L, 1));
	}

	if (0 == taskID)
		return 0;

	lua_pushnumber(L, g_Script.m_pPlayer->GetTaskStatus(taskID));

	return 1;
}

int CSPlayerFunc::L_settaskdata(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD taskID = static_cast<WORD>(lua_tonumber(L, 1));
	WORD timelimit = static_cast<WORD>(lua_tonumber(L, 2));

	if (0 == taskID)
		return 0;

	if (g_Script.m_pPlayer->m_TaskManager.GetRoleTask(taskID))
		return 0;

	CRoleTask task;
	task.m_Index      = taskID;
	task.m_CreateTime = timeGetTime();
	task.m_timelimit = timelimit ? timelimit : 0xffffffff; 
	g_Script.m_pPlayer->m_TaskManager.AddRoleTask(&task);

	return 0;
}

int CSPlayerFunc::L_settaskflag(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD  taskID	= static_cast<WORD> (lua_tonumber(L, 1));
	DWORD flagIndex = static_cast<DWORD>(lua_tonumber(L, 2));
	DWORD flagType  = static_cast<DWORD>(lua_tonumber(L, 3));
	DWORD goalID    = static_cast<DWORD>(lua_tonumber(L, 4));	// （对于跑地图，为地图ID,新增任务时应该为新任务类型）
	DWORD goalCount = static_cast<DWORD>(lua_tonumber(L, 5));	// （对于跑地图，为Y坐标，新增任务时应该为新任务的次数）
	DWORD goalSubID = 0;

	if (0 == taskID || 0 == flagIndex || 0 == flagType || 0 == goalCount)
		return 0;

	// 对于给/要，为道具ID，对于跑地图，为X坐标
	if (TT_GIVE == flagType || TT_GET == flagType || TT_REGION == flagType)
	{
		goalSubID = static_cast<DWORD>(lua_tonumber(L, 6));

		if (0 == goalSubID)
			return 0;
	}

	CRoleTask *task = g_Script.m_pPlayer->m_TaskManager.GetRoleTask(taskID);
	if (!task)
		return 0;

	STaskFlag flag;
	flag.m_Index	 = ((taskID << 16)&0xffff0000) | (flagIndex &0xffff);
	flag.m_Type		 = (BYTE)flagType;
	flag.m_Complete	 = 0;

	// 更新旗标数据
	switch (flagType)
	{
	case TT_EMPTY:
		{
			flag.m_Complete = 1;
		}
		break;

	case TT_TALK:
		{
			flag.m_TaskDetail.Talk.NpcID	= goalID;
			flag.m_TaskDetail.Talk.Times	= goalCount;
			flag.m_TaskDetail.Talk.CurTimes	= 0;
		}
		break;

	case TT_ITEM:
		{
			flag.m_TaskDetail.Item.ItemID	= goalID;
			flag.m_TaskDetail.Item.ItemNum	= goalCount;

			// 首先判断玩家背包中的物品信息以更新旗标状态
			DWORD number = g_Script.m_pPlayer->GetItemNum(goalID, XYD_FT_WHATEVER);

			if (number >= goalCount)
			{
				flag.m_TaskDetail.Item.CurItemNum = goalCount;
				flag.m_Complete = 1;
			}
			else
			{
				flag.m_TaskDetail.Item.CurItemNum = number;
				flag.m_Complete = 0;
			}

			g_Script.m_pPlayer->m_ItemTask[goalID].push_back(flag.m_Index);
		}
		break;

	case TT_GIVE:
	case TT_GET:
		{
			flag.m_TaskDetail.GiveGet.NpcID		= goalID;
			flag.m_TaskDetail.GiveGet.ItemID	= goalSubID;
			flag.m_TaskDetail.GiveGet.ItemNum	= goalCount;

			DWORD number = g_Script.m_pPlayer->GetItemNum(goalSubID, XYD_FT_WHATEVER);

			if (number >= goalCount)
			{
				flag.m_TaskDetail.GiveGet.CurItemNum = goalCount;
				flag.m_Complete = 1;
			}
			else
			{
				flag.m_TaskDetail.GiveGet.CurItemNum = number;
				flag.m_Complete = 0;
			}

			if (TT_GIVE == flagType)
				g_Script.m_pPlayer->m_GiveItemTask[goalSubID].push_back(flag.m_Index);
			else
				g_Script.m_pPlayer->m_GetItemTask[goalSubID].push_back(flag.m_Index);
		}
		break;

	case TT_KILL:
		flag.m_TaskDetail.Kill.MonsterID		= goalID;
		flag.m_TaskDetail.Kill.KillNum			= goalCount;
		flag.m_TaskDetail.Kill.CurKillNum		= 0;

		g_Script.m_pPlayer->m_KillTask[goalID].push_back(flag.m_Index);
		break;

	case TT_USEITEM:
		flag.m_TaskDetail.UseItem.ItemID		= goalID;
		flag.m_TaskDetail.UseItem.UseTimes		= goalCount;
		flag.m_TaskDetail.UseItem.CurUseTimes	= 0;

		g_Script.m_pPlayer->m_UseItemTask[goalID].push_back(flag.m_Index);
		break;	

	case TT_REGION:
		flag.m_TaskDetail.Region.DestMapID		= goalID;
		flag.m_TaskDetail.Region.DestX			= goalSubID;
		flag.m_TaskDetail.Region.DestY			= goalCount;

		g_Script.m_pPlayer->m_MapTask[goalID].push_back(flag.m_Index);	
		break;
	case TT_SCENE:
		flag.m_TaskDetail.Scene.DestSceneID = goalID;
		break;
	case TT_ADD:
		flag.m_TaskDetail.TaskADD.btype = goalID;
		flag.m_TaskDetail.TaskADD.wCount = 0;
		flag.m_TaskDetail.TaskADD.wAllCount = goalCount;
		if (goalID == flag.m_TaskDetail.TaskADD.TASK_SCENECOMPLETE)
		{
			goalSubID = static_cast<DWORD>(lua_tonumber(L, 6));
			flag.m_TaskDetail.TaskADD.dIndex = goalSubID;
		}
		g_Script.m_pPlayer->m_AddTask[goalID].push_back(flag.m_Index);
		break;
	default:
		break;
	}

	// 将任务旗标加入任务
	task->PushFlag(flag);

	return 0;
}

int CSPlayerFunc::L_syntaskstatus(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD taskID = static_cast<WORD>(lua_tonumber(L, 1));

	if (0 == taskID)
		return 0;

	if (CRoleTask *task = g_Script.m_pPlayer->m_TaskManager.GetRoleTask(taskID))
	{
		SAAddRoleTask msg;
		msg.dwGlobalID   = g_Script.m_pPlayer->GetGID();
		msg.wTaskID	     = taskID;
		msg.m_CreateTime = task->m_CreateTime;
		msg.wFlagCount   = task->SizeFlags();

		DWORD flagindex = 0;
		for (CRoleTask::TELIST::iterator it = task->m_flagList.begin(); it != task->m_flagList.end(); ++it)
		{
			msg.w_flags[flagindex++] = *it;
		}

		g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(SAAddRoleTask));
		//g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(msg) - (8-msg.wFlagCount)*sizeof(STaskFlag));
	}
	else
	{
		rfalse("【SynTaskStatus】任务%d不在玩家身上！", taskID);
	}

	return 0;
}

int CSPlayerFunc::L_DelTaskMap(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD taskID = static_cast<WORD>(lua_tonumber(L, 1));

	if (0 == taskID)
		return 0;

	g_Script.m_pPlayer->DeleteTaskItemMap(taskID);

	return 0;
}

int CSPlayerFunc::L_delroletask(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD taskID   = static_cast<WORD>(lua_tonumber(L, 1));
	WORD giveup	  = static_cast<WORD>(lua_tonumber(L, 2));

	if (CRoleTask *task = g_Script.m_pPlayer->m_TaskManager.GetRoleTask(taskID))
	{
		g_Script.m_pPlayer->m_TaskManager.DelRoleTask(task);
	}
	else
	{
		rfalse("要交的任务%d不在玩家身上", taskID);
		return 0;
	}

	if (!giveup)
	{
		g_Script.m_pPlayer->m_Property.MarkComplete(taskID);

		SAHandInRoleTask msg;
		msg.dwGlobalID  = g_Script.m_pPlayer->GetGID();
		msg.wTaskID     = taskID;
		msg.wResult     = SAHandInRoleTask::HTR_SUCCESS;

		g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(msg));
		long flag = 1;
		lua_pushnumber(L,flag);
		return 1;
	}
	else
	{
		// 表示是放弃任务
		SAGiveUpTask msg;
		msg.dwGlobalID = g_Script.m_pPlayer->GetGID();
		msg.wTaskID	   = taskID;
		msg.wResult    = SAGiveUpTask::QUT_SUCCESS;

		g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}

	return 0;
}

int CSPlayerFunc::L_istaskflagcomplete(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD  taskID	= static_cast<WORD >(lua_tonumber(L, 1));
	WORD  flagType	= static_cast<WORD >(lua_tonumber(L, 2));
	DWORD DestID	= static_cast<DWORD>(lua_tonumber(L, 3));
	DWORD itemID	= 0;   
	WORD  itemNum	= 0; 

	if (taskID == 0 || flagType == 0 || DestID == 0)
		return 0;

	if (flagType == TT_GIVE || flagType == TT_GET)
	{
		itemID  = static_cast<DWORD>(lua_tonumber(L, 4));
		itemNum = static_cast<WORD >(lua_tonumber(L, 5));

		if (itemID == 0 || itemNum == 0)
			return 0;
	}

	if (CRoleTask *task = g_Script.m_pPlayer->m_TaskManager.GetRoleTask(taskID))
	{
		for (CRoleTask::TELIST::iterator it = task->m_flagList.begin(); it != task->m_flagList.end(); ++it)
		{
			if (it->m_Type != flagType)
				continue;

			switch (it->m_Type)
			{
			case TT_GIVE:
			case TT_GET:
				{
					if (it->m_Type == flagType && it->m_TaskDetail.GiveGet.NpcID == DestID && it->m_TaskDetail.GiveGet.ItemID == itemID 
						&& it->m_TaskDetail.GiveGet.ItemNum == itemNum)
					{
						lua_pushnumber(L, it->m_Complete);
						return 1;
					}
				}
				break;

			case TT_TALK:
				{
					if (it->m_Type == flagType && it->m_TaskDetail.Talk.NpcID == DestID)
					{
						lua_pushnumber(L, it->m_Complete);
						return 1;
					}
				}
				break;

			default:
				rfalse(2, 1, "IsTaskFlagComplete：查询了未处理的旗标类型：%d", flagType);
				break;
			}
		}
	}

	return 0;
}

int CSPlayerFunc::L_checkroletasks(lua_State *L)
{
	/*
	if(g_Script.m_pPlayer == NULL)
	{
	lua_pushnumber(L, 0);
	return 1;
	}

	if(g_Script.m_pPlayer->m_TaskManager.GetRoleTask(static_cast<const char*>(lua_tostring(L, 1))))
	lua_pushnumber(L, 1);
	else
	lua_pushnumber(L, 0);

	return 1;
	*/
	return 0;
}

int CSPlayerFunc::L_checkroletaskn(lua_State *L)
{
	if(g_Script.m_pPlayer == NULL)
	{
		lua_pushnumber(L, 0);
		return 1;
	}

	if(g_Script.m_pPlayer->m_TaskManager.GetRoleTask(static_cast<WORD>(lua_tonumber(L, 1))))
		lua_pushnumber(L, 1);
	else
		lua_pushnumber(L, 0);

	return 1;
}

int CSPlayerFunc::L_addroletaskelements(lua_State *L)
{
	/*
	if(g_Script.m_pPlayer == NULL)
	{
	return 0;
	}

	CRoleTask *pTask = g_Script.m_pPlayer->m_TaskManager.GetRoleTask(static_cast<const char*>(lua_tostring(L, 1)));
	if(pTask)
	{
	if(pTask->SizeElement() >= MAX_TASKELEMENT)
	return 0;

	STaskElement element;
	ZeroMemory(&element, sizeof(STaskElement));

	element.wType = static_cast<WORD>(lua_tonumber(L, 2));
	dwt::strcpy(element.szName,  static_cast<const char*>(lua_tostring(L, 3)), 25);
	element.dwIndex = static_cast<DWORD>(lua_tonumber(L, 4));
	element.dwValue = static_cast<DWORD>(lua_tonumber(L, 5));
	element.dwComplete = static_cast<DWORD>(lua_tonumber(L, 6));

	pTask->PushTElement(element);

	// 任务原子消息
	SASendRoleTaskElementMsg emsg;
	ZeroMemory(emsg.szName, sizeof(emsg.szName));
	emsg.wIndex = pTask->wIndex;
	strncpy(emsg.szName, pTask->szName, 25);
	emsg.dwElementIndex = element.dwIndex;
	pTask->GetTElementDesc(&element, emsg.szInfo);
	g_Script.m_pPlayer->m_TaskManager.SendRoleTaskInfo(&emsg, sizeof(SASendRoleTaskElementMsg),g_Script.m_pPlayer);
	}//*/

	return 0;
}

int CSPlayerFunc::L_addroletaskelementn(lua_State *L)
{
	/*
	if(g_Script.m_pPlayer == NULL)
	{
	return 0;
	}

	CRoleTask *pTask = g_Script.m_pPlayer->m_TaskManager.GetRoleTask(static_cast<WORD>(lua_tonumber(L, 1)));
	if(pTask)
	{
	if(pTask->SizeElement() >= MAX_TASKELEMENT)
	return 0;

	STaskElement element;
	ZeroMemory(&element, sizeof(STaskElement));

	element.wType = static_cast<WORD>(lua_tonumber(L, 2));
	dwt::strcpy(element.szName,  static_cast<const char*>(lua_tostring(L, 3)), 25);
	element.dwIndex = static_cast<DWORD>(lua_tonumber(L, 4));
	element.dwValue = static_cast<DWORD>(lua_tonumber(L, 5));
	element.dwComplete = static_cast<DWORD>(lua_tonumber(L, 6));

	pTask->PushTElement(element);

	// 任务原子消息
	SASendRoleTaskElementMsg emsg;
	ZeroMemory(emsg.szName, sizeof(emsg.szName));
	emsg.wIndex = pTask->wIndex;
	strncpy(emsg.szName, pTask->szName, 25);
	emsg.dwElementIndex = element.dwIndex;
	pTask->GetTElementDesc(&element, emsg.szInfo);
	g_Script.m_pPlayer->m_TaskManager.SendRoleTaskInfo(&emsg, sizeof(SASendRoleTaskElementMsg), g_Script.m_pPlayer);
	}
	//*/

	return 0;   
}

int CSOtherFunc::L_loadtaskdesc(lua_State *L)
{
	WORD wType = static_cast<WORD>(lua_tonumber(L, 1));

	TASKDESC desc;
	desc.Desc1 = static_cast<LPCSTR>(lua_tostring(L, 2));
	desc.Desc2 = static_cast<LPCSTR>(lua_tostring(L, 3));
	desc.Desc3 = static_cast<LPCSTR>(lua_tostring(L, 4));
	desc.Desc4 = static_cast<LPCSTR>(lua_tostring(L, 5));

	(CRoleTaskDesc::Instance())->PushTaskDesc(wType, &desc, CRoleTaskDesc::DT_ELEMENT);

	return 0;
}

int CSOtherFunc::L_loadtaskdescA(lua_State *L)
{
	WORD wType = static_cast<WORD>(lua_tonumber(L, 1));

	TASKDESC desc;
	desc.Desc1 = static_cast<LPCSTR>(lua_tostring(L, 2));
	desc.Desc2 = static_cast<LPCSTR>(lua_tostring(L, 3));
	desc.Desc3 = static_cast<LPCSTR>(lua_tostring(L, 4));
	desc.Desc4 = static_cast<LPCSTR>(lua_tostring(L, 5));

	(CRoleTaskDesc::Instance())->PushTaskDesc(wType, &desc, CRoleTaskDesc::DT_TASK);

	return 0;
}

int CSPlayerFunc::L_setroletaskelements(lua_State *L)
{
	/*
	if(g_Script.m_pPlayer == NULL)
	{
	return 0;
	}

	CRoleTask *pTask = g_Script.m_pPlayer->m_TaskManager.GetRoleTask(static_cast<const char*>(lua_tostring(L, 1)));
	if(pTask)
	{
	STaskElement *pElement = pTask->GetTElement(static_cast<const char*>(lua_tostring(L, 2)));
	if(pElement)
	{
	pElement->dwComplete = static_cast<DWORD>(lua_tonumber(L, 3));
	}

	}//*/

	return 0;
}

int CSPlayerFunc::L_setroletaskelementn(lua_State *L)
{
	/*
	if(g_Script.m_pPlayer == NULL)
	{
	return 0;
	}

	CRoleTask *pTask = g_Script.m_pPlayer->m_TaskManager.GetRoleTask(static_cast<WORD>(lua_tonumber(L, 1)));
	if(pTask)
	{
	STaskElement *pElement = pTask->GetTElement(static_cast<DWORD>(lua_tonumber(L, 2)));
	if(pElement)
	{
	pElement->dwComplete = static_cast<DWORD>(lua_tonumber(L, 3));
	}

	}//*/

	return 0;
}

int CSPlayerFunc::L_getroletaskelements(lua_State *L)
{
	/*
	if(g_Script.m_pPlayer == NULL)
	{
	lua_pushnumber(L, -1);
	return 1;
	}

	CRoleTask *pTask = g_Script.m_pPlayer->m_TaskManager.GetRoleTask(static_cast<const char*>(lua_tostring(L, 1)));
	if(pTask)
	{
	STaskElement *pElement = pTask->GetTElement(static_cast<const char*>(lua_tostring(L, 2)));
	if(pElement)
	{
	lua_pushstring(L, pElement->szName);
	lua_pushnumber(L, pElement->dwIndex);
	lua_pushnumber(L, pElement->dwValue);
	lua_pushnumber(L, pElement->dwComplete);
	return 4;
	}

	}

	lua_pushnumber(L, -1);
	return 1;
	//*/

	return 0;
}

int CSPlayerFunc::L_getroletaskelementn(lua_State *L)
{
	return 0;
}

int CSPlayerFunc::L_cleantask(lua_State *L)
{

	return 0;
}

// 用名字和门派判断武林盟主
string s_WulinChiefName = "";
BYTE s_WulinChiefSchool = 0;

int CSPlayerFunc::L_setwulinchief(lua_State *L)
{
	LPCSTR lpszName = static_cast<const char*>(lua_tostring(L, 1));
	BYTE bySchool = static_cast<BYTE>(lua_tonumber(L, 2));

	if(bySchool)
	{
		s_WulinChiefName = lpszName;
		s_WulinChiefSchool = bySchool;     // bySchool1-5  m_Property.m_bySchool0-4 比较的时候注意+1
	}

	return 0;
}

int CSPlayerFunc::L_OnPlayerCycTask(lua_State *L)
{//SPlayerCycTask
	if (!g_Script.m_pPlayer)
	{
		rfalse("L_OnPlayerCycTask: 玩家状态为空!!!");
		return 0;
	}
	WORD  taskID = static_cast<WORD>(lua_tonumber(L,1));//任务索引号 结束任务的索引
	WORD  ttype	= static_cast<WORD >(lua_tonumber(L,2));//任务类型1,2taskType = 1(单个任务)  taskType = 2(多个任务)
	WORD  cyctype	= static_cast<WORD >(lua_tonumber(L,3));//任务时间类型cyctype  = 1(日常任务),	2(跑环(即时)任务),	3(周长任务)
	WORD  HeadID = static_cast<WORD >(lua_tonumber(L,4));
	if (cyctype == 2 && ttype == 1)//即时清数据 只有一个
	{
		g_Script.m_pPlayer->DeleteCycData(taskID,1);
		//g_Script.m_pPlayer->SendCycData(taskID);
		return 1;
	}
	else if (cyctype == 2 && ttype == 2)
	{
		g_Script.m_pPlayer->DeleteCycData(taskID);
		//执行lua
		return 1;
	}
	//g_Script.m_pPlayer->m_Property.m_PlayerCyc;
	std::string CompleteTimeStr;
	g_Script.m_pPlayer->GetSysTimeTick(CompleteTimeStr);
	SPlayerCycTask tcycTask;
	memcpy(tcycTask.m_CompleteTime,CompleteTimeStr.c_str(),CompleteTimeStr.size());
	tcycTask.m_TaskID = taskID;
	tcycTask.m_type = ttype;
	tcycTask.m_cyctype = cyctype;
	for (int i = 0; i < 20; ++i)
	{
		WORD taskid = g_Script.m_pPlayer->m_Property.m_PlayerCyc[i].m_TaskID; 
		if (taskid == 0)
		{
			g_Script.m_pPlayer->m_Property.m_PlayerCyc[i] = tcycTask;
			break;
		}
	}
	lite::Variant ret;
	int Fal = 0;
	LuaFunctor(g_Script,"GetHeadFlagData")[taskID](&ret);
	if (lite::Variant::VT_INTEGER == ret.dataType)
	{
		Fal = ret;
		if (Fal == 1)//说明是有任务链的头任务
		{


		}
		else
		{
			for (long i = 0; i <  g_Script.m_pPlayer->m_Property.m_SPlayerLimtTime.m_RdNum; ++i )
			{
				WORD id = g_Script.m_pPlayer->m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[i].m_TaskID;
				if (id == HeadID)
				{
					g_Script.m_pPlayer->m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[i].m_TaskID = 0;
					break;
				}
			}
		}
	}
	else
	{
		int i = 0;
	}


	return 1;	
}
int CSPlayerFunc::L_getrdtasksNum(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD NpcID = static_cast<WORD>(lua_tonumber(L, 1));
	if (0 == NpcID)
		return 0;

	lua_pushnumber(L, g_Script.m_pPlayer->GetRdTasksNum(NpcID));

	return 1;
}
int CSPlayerFunc::L_GetCurrentNpcTaskID(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD NpcID = static_cast<WORD>(lua_tonumber(L, 1));
	if (0 == NpcID)
		return 0;

	lua_pushnumber(L, g_Script.m_pPlayer->GetCurrentNpcTaskID(NpcID));
	return 1;
}
int CSPlayerFunc::L_InsertPlayerRdTaskInfo(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD NpcID = static_cast<WORD>(lua_tonumber(L, 1));
	long taskID = static_cast<long>(lua_tonumber(L, 2));
	if (0 == NpcID || taskID == 0)
		return 0;
	g_Script.m_pPlayer->InsertPlayerRdTaskInfo(NpcID,taskID);
	return 1;
}
int CSPlayerFunc::L_GetRdNpcNum(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	lua_pushnumber(L, g_Script.m_pPlayer->GetRdNpcNum());
	return 1;
}
int CSPlayerFunc::L_OnXKLProcess(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	lua_pushnumber(L, g_Script.m_pPlayer->OnXKLProcess());
	return 1;
}
int CSPlayerFunc::L_AddXKLUseNum(lua_State * L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	short num = static_cast<short>(lua_tonumber(L, 1));
	g_Script.m_pPlayer->AddXKLUseNum(num);
	std::string currtimestr;
	g_Script.m_pPlayer->GetSysTimeTick(currtimestr);
	memset(g_Script.m_pPlayer->m_Property.GKLtime,0,30);
	memcpy(g_Script.m_pPlayer->m_Property.GKLtime,currtimestr.c_str(),currtimestr.size());
	return 1;
}
int CSPlayerFunc::L_GetXKLUseNum(lua_State * L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	lua_pushnumber(L,g_Script.m_pPlayer->GetXKLUseNum());
	return 1;
}
int CSPlayerFunc::L_OnHandleCycTask(lua_State * L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	WORD Npcid = static_cast<WORD>(lua_tonumber(L,1));
	g_Script.m_pPlayer->OnHandleCycTask(Npcid);
	return 1;
}
int CSPlayerFunc::L_GetHeadIDByStr(lua_State * L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	LPCSTR lpszFaction = static_cast<const char*>(lua_tostring(L, 1));
	if (lpszFaction)
	{
		std::string str  = lpszFaction;
		WORD HeadtaskID = g_Script.m_pPlayer->GetHeadIDByStr(str);
		lua_pushnumber(L,HeadtaskID);
	}
	else
	{
		rfalse("任务尾部被占距");
	}
	return 1;
}
int CSPlayerFunc::L_PrintXKLLOG(lua_State * L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	short num = static_cast<short>(lua_tonumber(L, 1));
	long ItemID = static_cast<long>(lua_tonumber(L, 2));
	g_Script.m_pPlayer->PrintXKLLOG(num,ItemID);
	return 1;
}

int CSPlayerFunc::L_SendRoleTaskInfo(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	SASendRoleTaskInfoMsg roleinfomsg;
	memset(roleinfomsg.wTaskData, 0, sizeof(roleinfomsg.wTaskData));
	roleinfomsg.wTaskNum = 0;
	int index = 0;
	for (CPlayer::TASKINFO::iterator task = g_Script.m_pPlayer->m_TaskInfo.begin(); task != g_Script.m_pPlayer->m_TaskInfo.end(); ++task)
	{
		if (64 == index)
			break;

		WORD taskid = (*task).first;
		WORD state = g_Script.m_pPlayer->GetTaskStatus(taskid);
	
		roleinfomsg.wTaskData[index++] = task->first;
		roleinfomsg.wTaskData[index++] = task->second;
	}
	roleinfomsg.wTaskNum = index/2;
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &roleinfomsg, sizeof(roleinfomsg)-(64 - index)*sizeof(WORD));
	return 1;
}

int CSPlayerFunc::L_OnUpdateTask(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	SQFinishAddTask addtaskmsg;
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2)&&lua_isnumber(L,3))
	{
		addtaskmsg.btype = static_cast<BYTE>( lua_tonumber(L, 1));
		addtaskmsg.wCount = static_cast<WORD>(lua_tonumber(L, 2));
		addtaskmsg.dIndex = static_cast<DWORD>(lua_tonumber(L, 3));
		if (g_Script.m_pPlayer->OnUpdateTask(&addtaskmsg))
		{
			return 1;
		}
		 

	}
	return 0;
}

int CSPlayerFunc::L_settaskstatus(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L,1))
	{
		WORD taskid = static_cast<WORD>(lua_tonumber(L, 1));
		if (taskid > 0)
		{
			g_Script.m_pPlayer->m_TaskManager.ClearAllRoleTask();
			g_Script.m_pPlayer->m_TaskInfo.clear();
			g_Script.m_pPlayer->m_MenuInfo.clear();
			g_Script.m_pPlayer->m_ItemTask.clear();
			g_Script.m_pPlayer->m_KillTask.clear();
			g_Script.m_pPlayer->m_UseItemTask.clear();
			g_Script.m_pPlayer->m_GiveItemTask.clear();
			g_Script.m_pPlayer->m_GetItemTask.clear();
			g_Script.m_pPlayer->m_MapTask.clear();
			g_Script.m_pPlayer->m_AddTask.clear();

			for (int i = taskid; i <= MAX_WHOLE_TASK; i++)
			{
				g_Script.m_pPlayer->m_Property.MarkUnComplete(i);
			}

			for (int i = 1; i < taskid; i++)
			{
				g_Script.m_pPlayer->m_Property.MarkComplete(i);
			}

			SAHandInRoleTask msg;
			msg.dwGlobalID = g_Script.m_pPlayer->GetGID();
			msg.wTaskID = taskid > 0 ? taskid - 1 : taskid;
			msg.wResult = SAHandInRoleTask::HTR_SUCCESS;

			g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(msg));

			lua_pushnumber(L, 1);
			
			g_Script.m_pPlayer->m_TaskManager.SendAllRoleTaskInfo(g_Script.m_pPlayer);
			return 1;
		}
	}
	return 0;
}


