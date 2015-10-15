///////////////////////////////////////////////////////////////////////////////
// 副本/AI相关的函数
///////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "ExtraScriptFunctions.h"
#include "lite_lualibrary.hpp"

#include "pub/wndcusop.h"
#include "pub/traceinfo.h"

#include "gameobjects/region.h"
#include "gameobjects/monster.h"
#include "gameobjects/player.h"
#include "gameobjects/area.h"
#include "gameobjects/npc.h"
#include "gameobjects/item.h"
#include "gameobjects/gameworld.h"
#include "gameobjects/globalfunctions.h"
#include "gameobjects/scriptmanager.h"
#include "gameobjects/factionregion.h"
#include "gameobjects/eventregion.h"
#include "gameobjects/fightarena.h"
#include "gameobjects/prison.h"
#include "gameobjects/prisonex.h"
#include "gameobjects/biguanarea.h"
#include "gameobjects/building.h"

#include "networkmodule/netprovidermodule/netprovidermoduleadv.h"
#include "区域服务器/dmainapp.h"
#include "gameobjects/upgradesys.h"

#include "networkmodule/itemmsgs.h"
#include "networkmodule/scriptmsgs.h"
#include "networkmodule/tongmsgs.h"
#include "networkmodule/regionmsgs.h"
#include "networkmodule/cardpointmodifymsgs.h"
#include "networkmodule/refreshmsgs.h"
#include "networkmodule/accountmsgs.h"
#include "pub/rpcop.h"

#include "gameobjects/CMonsterService.h"

#include <time.h>
#include <math.h>

#include "networkmodule/movemsgs.h"
#include "gameobjects/Random.h"
#include "GameObjects/Pet.h"
///////////////////////////////////////////////////////////////////////////////
extern CScriptManager g_Script;
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// 因为结构才能申明友元，所以无法用名字空间来代替
///////////////////////////////////////////////////////////////////////////////
extern LPIObject FindRegionByGID( DWORD GID );
extern LPIObject FindRegionByID( DWORD ID );

double _result = 0;
bool   _isnul  = false;
#define GET_FIELD_NUMBER_DIRECTLY(_table, _key) ((lua_getfield(L, _table, _key),	\
	(lua_isnil(L, -1)												\
	? (_isnul = true, _result = 0)									\
	: (_isnul = false, _result = lua_tonumber(L, -1))),				\
	lua_pop(L, 1)), _result)

struct ExtraLuaFunctions
{

static CRegion* GetRegionById( DWORD regionId );

static int L_CreateAIMonster( lua_State *L )
{
	///////////////////////////////////////////////////////////////////////////
	if (!lua_istable(L, 1))
		return 0;

	CMonster::SParameter param;
	memset(&param, 0, sizeof(param));

	DWORD regionId		= static_cast<DWORD>(GET_FIELD_NUMBER_DIRECTLY(1, "RegionGID"));
	param.wListID		= static_cast<WORD >(GET_FIELD_NUMBER_DIRECTLY(1, "monsterId"));
	param.wX			= static_cast<WORD >(GET_FIELD_NUMBER_DIRECTLY(1, "x"));
	param.wY			= static_cast<WORD >(GET_FIELD_NUMBER_DIRECTLY(1, "y"));
	param.dwRefreshTime = static_cast<DWORD>(GET_FIELD_NUMBER_DIRECTLY(1, "refreshTime"));
	param.wAIID			= static_cast<WORD >(GET_FIELD_NUMBER_DIRECTLY(1, "AIID"));
	param.wGroupID		= static_cast<WORD >(GET_FIELD_NUMBER_DIRECTLY(1, "GroupID"));
	param.PartolRadius	= static_cast<WORD >(GET_FIELD_NUMBER_DIRECTLY(1, "PartolRadius"));
	CRegion *destRegion = GetRegionById(regionId);
	if (!destRegion)
		return rfalse(2, 1, "CreateAIMonster:找不到指定场景");

	if (lua_istable(L, 2))
	{
		size_t size = lua_objlen(L, 2);
		param.m_Path.m_moveType = EA_RUN;
		param.m_Path.m_wayPoints = size + 1; //多加一个点，后面会用来存放起始点

		for (size_t i=1; i<=size; ++i)
		{
			lua_rawgeti(L, 2, i);
			if( lua_istable(L, -1) )
			{   //0号成员跳过， *32是将格子坐标转化为世界坐标
				param.m_Path.m_path[i].m_X = static_cast<DWORD>(GET_FIELD_NUMBER_DIRECTLY(3, "x")) * 32; 
				param.m_Path.m_path[i].m_Y = static_cast<DWORD>(GET_FIELD_NUMBER_DIRECTLY(3, "y")) * 32;
				lua_pop(L, 1);
			}
		}	
	}
	///////////////////////////////////////////////////////////////////////////
	
	const SMonsterBaseData *pMonsterData = CMonsterService::GetInstance().GetMonsterBaseData(param.wListID);
	if (!pMonsterData)
		return rfalse(2, 1, "L_CreateAIMonster:在基本列表中找不到对应ID的怪物!");

	CMonster *pMonster = destRegion->CreateMonster(&param);
	if (!pMonster)
		return rfalse(2, 1, "CreateAIMonster失败!");

	CEventRegion *lpEventRegion = (CEventRegion*)destRegion->DynamicCast(IID_EVENTREGION);
	if (lpEventRegion)
		lpEventRegion->m_AIMonster.insert(pMonster->GetGID());

	lua_pushnumber(L, pMonster->m_GID);	
	return 1;
}

static int L_SetMonsterPath( lua_State *L )
{
	if (!g_Script.m_pMonster)
	{
		rfalse(4,1,"L_SetMonsterPath");
		return 0;
	}
	CMonster::SParameter param;
	memset(&param, 0, sizeof(param));

	if (lua_istable(L, 1))
	{
		size_t size = lua_objlen(L, 1);
		param.m_Path.m_moveType = EA_RUN;
		param.m_Path.m_wayPoints = size + 1; //多加一个点，后面会用来存放起始点

		for (size_t i=1; i<=size; ++i)
		{
			lua_rawgeti(L, 1, i);
			if( lua_istable(L, -1) )
			{   //0号成员跳过， *32是将格子坐标转化为世界坐标
				param.m_Path.m_path[i].m_X = static_cast<DWORD>(GET_FIELD_NUMBER_DIRECTLY(2, "x")) * 32; 
				param.m_Path.m_path[i].m_Y = static_cast<DWORD>(GET_FIELD_NUMBER_DIRECTLY(2, "y")) * 32;
				lua_pop(L, 1);
			}
		}	
	}

	g_Script.m_pMonster->SetMonsterPath(param);
	return 0;
}

static int L_GetMonsterCurActionID( lua_State *L )
{
	if (!g_Script.m_pMonster)
	{
		rfalse(4,1,"L_GetMonsterCurActionID");
		return 0;
	}
	int nActionID = g_Script.m_pMonster->GetCurActionID();
	lua_pushnumber(L, nActionID);
	return 1;
}
  
static int L_SetMonsterCurActionID( lua_State *L )
{
	int nCurrActionID = lua_tonumber(L, 1);
	CMonster* pCurrMonster = g_Script.m_pMonster;
	if (!pCurrMonster)
	{
		rfalse(4,1,"L_SetMonsterCurActionID");
		return 0;
	}
	pCurrMonster->SetCurActionID((EActionState)nCurrActionID);
	return 0;
}

static int L_GetMonsterCurPos( lua_State *L )
{
	CMonster* pCurrMonster = g_Script.m_pMonster;
	if (!pCurrMonster)
	{
		rfalse(4,1,"L_GetMonsterCurPos");
		return 0;
	}
	WORD x, y;
	pCurrMonster->GetCurPos(x, y);
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	return 2;
}

static int L_GetAttackTargetMonster( lua_State *L )
{
	CMonster* pCurrMonster = g_Script.m_pMonster;
	if (!pCurrMonster)
	{
		rfalse(4,1,"L_GetAttackTargetMonster");
		return 0;
	}
	check_list<LPIObject> *monsterList ;
	monsterList = pCurrMonster->m_ParentRegion->GetMonsterListByAreaIndex(pCurrMonster->m_ParentArea->m_X, pCurrMonster->m_ParentArea->m_Y);
	if (monsterList)
	{
		for (check_list<LPIObject>::iterator it = monsterList->begin(); it != monsterList->end(); ++it)
		{
			LPIObject target = *it;
			CMonster *monster = (CMonster*)target->DynamicCast(IID_MONSTER);
			if (monster && 0 != monster->m_CurHp)
			{
				if(monster->m_AIParameter.wGroupID == pCurrMonster->m_AIParameter.wGroupID) //阵营相同不能攻击
					continue;

				float distance = sqrt(pow((pCurrMonster->m_curX-monster->m_curX),2)+pow((pCurrMonster->m_curY-monster->m_curY),2));
				if ((WORD)distance > 400)
					continue;

				DWORD dwLastEnemyID = monster->GetGID();
				lua_pushnumber(L, dwLastEnemyID);
				return 1;
			}
		}				
	}

	lua_pushnumber(L, 0); //没找到就是0
	return 1;
}

static int L_GetAttackTargetPlayer( lua_State *L )
{
	CMonster* pCurrMonster = g_Script.m_pMonster;
	if (!pCurrMonster)
	{
		rfalse(4,1,"L_GetAttackTargetPlayer");
		return 0;
	}
	check_list<LPIObject> *playerList ;
	//playerList = pCurrMonster->m_ParentRegion->GetPlayerListByAreaIndex(pCurrMonster->m_ParentArea->m_X, pCurrMonster->m_ParentArea->m_Y);
	playerList = &(pCurrMonster->m_ParentArea->m_PlayerList);
	if (playerList)
	{
		for (check_list<LPIObject>::iterator it = playerList->begin(); it != playerList->end(); ++it)
		{
			LPIObject target = *it;
			CPlayer *player = (CPlayer*)target->DynamicCast(IID_PLAYER);
			if (player && 0 != player->m_CurHp)
			{
				//if(player->m_AIParameter.wGroupID == pCurrMonster->m_AIParameter.wGroupID) //阵营相同不能攻击
				//	break;

				float distance = sqrt(pow((pCurrMonster->m_curX-player->m_curX),2)+pow((pCurrMonster->m_curY-player->m_curY),2));
				//rfalse("MonsterID:%d | TargetID:%d | _PartolRadius:%d | Distance:%4.2f ", pCurrMonster->m_GID, player->GetGID(), pCurrMonster->m_Property.m_PartolRadius, distance);
				if ((WORD)distance > pCurrMonster->m_Property.m_PartolRadius)
					continue;

				DWORD dwLastEnemyID = player->GetGID();
				lua_pushnumber(L, dwLastEnemyID);
				return 1;
			}
		}				
	}

	lua_pushnumber(L, 0); //没找到就是0
	return 1;
}

static int L_StartRun( lua_State *L )
{
	CMonster* pCurrMonster = g_Script.m_pMonster;
	if (!pCurrMonster)
	{
		rfalse(4,1,"L_StartRun");
		return 0;
	}
	if (--pCurrMonster->m_ActionFrame <= 0)
	{
		if ((pCurrMonster->m_fightState & FS_DIZZY) || (pCurrMonster->m_fightState & FS_DINGSHENG))
		{
			pCurrMonster->m_ActionFrame = 1;
			return 0;
		}

		if(pCurrMonster->m_AIParameter.m_Path.m_wayPoints<1)
		{
			SQSynPathMsg msg;

			pCurrMonster->MakeRadomPath(pCurrMonster->m_Property.m_ActiveRadius, pCurrMonster->m_Property.m_BirthPosX << TILE_BITH, pCurrMonster->m_Property.m_BirthPosY << TILE_BITH, msg);
			if (!pCurrMonster->SetMovePath(&msg))
				pCurrMonster->m_ActionFrame = CRandom::RandRange(pCurrMonster->m_Property.m_StayTimeMin, pCurrMonster->m_Property.m_StayTimeMax);
		}
		else
		{
			// 站立的时间结束，开始随机移动
			SQSynPathMsg msg;
			pCurrMonster->MakeSynPathMsg(msg);

			// 如果设置路径失败，则继续站立
			if (!pCurrMonster->SetMovePath(&msg))
				pCurrMonster->m_ActionFrame = CRandom::RandRange(pCurrMonster->m_Property.m_StayTimeMin, pCurrMonster->m_Property.m_StayTimeMax);
		}
	}

	return 0;
}

static int L_StartStaticRun( lua_State *L)
{
	CMonster* pCurrMonster = g_Script.m_pMonster;
	if (!pCurrMonster)
	{
		rfalse(4,1,"L_StartRun");
		return 0;
	}
	if (--pCurrMonster->m_ActionFrame <= 0)
	{
		if ((pCurrMonster->m_fightState & FS_DIZZY) || (pCurrMonster->m_fightState & FS_DINGSHENG))
		{
			pCurrMonster->m_ActionFrame = 1;
			return 0;
		}

		// 站立的时间结束，开始随机移动
		SQSynPathMsg msg;

		pCurrMonster->MakeRadomPath(pCurrMonster->m_Property.m_ActiveRadius, pCurrMonster->m_Property.m_BirthPosX << TILE_BITH, pCurrMonster->m_Property.m_BirthPosY << TILE_BITH, msg);
		pCurrMonster->m_nMoveStartTime = GetTickCount64();
		if (msg.m_Path.m_wayPoints>1)
		{
			pCurrMonster->SetMovePos(msg.m_Path.m_path[1].m_X, msg.m_Path.m_path[1].m_Y);
		}
		
		// 如果设置路径失败，则继续站立
// 		if (!pCurrMonster->SetMovePath(&msg))
// 			pCurrMonster->m_ActionFrame = CRandom::RandRange(pCurrMonster->m_Property.m_StayTimeMin, pCurrMonster->m_Property.m_StayTimeMax);



		return 0;
	}
	return 0;
}
static int L_StartAttack( lua_State *L )
{
	CMonster* pCurrMonster = g_Script.m_pMonster;
	DWORD dwLastEnemyID = lua_tonumber(L, 1);
	DWORD SkillID = lua_tonumber(L,2);
	if (!pCurrMonster)
	{
		rfalse(4,1,"L_StartAttack");
		return 0;
	}
	LPIObject object=pCurrMonster->m_ParentRegion->SearchObjectListInAreas(dwLastEnemyID, pCurrMonster->m_ParentArea->m_X, pCurrMonster->m_ParentArea->m_Y);
	if (!object) return 0;

	D3DXVECTOR2 vPos(0.0f,0.0f);
	CPlayer *pDest = (CPlayer *)object->DynamicCast(IID_PLAYER);
	if (pDest)
	{
		vPos.x = pDest->m_curX;
		vPos.y = pDest->m_curY;
	}
	else if (CFightPet *pFightPet = (CFightPet *)object->DynamicCast(IID_FIGHT_PET))
	{
		vPos.x = pFightPet->m_curX;
		vPos.y = pFightPet->m_curY;
	}
	else
	{
		dwLastEnemyID = 0;
	}

	if (dwLastEnemyID > 0)
	{
		SQuestSkill_C2S_MsgBody msg;

		msg.mDefenderGID				= dwLastEnemyID;
		msg.mAttackerGID				= pCurrMonster->GetGID();
		msg.mDefenderWorldPosX = vPos.x;
		msg.mDefenderWorldPosY = vPos.y;

		msg.dwSkillIndex				= SkillID;//pCurrMonster->m_nRandomSkillIndex;

		pCurrMonster->ProcessQuestSkill(&msg);
	}
	lua_pushnumber(L, dwLastEnemyID);
	return 1;
}

static int L_EventRegionFinish( lua_State *L )
{
	if (!g_Script.m_pMonster)
	{
		rfalse(4,1,"L_EventRegionFinish1");
		return 0;
	}
	int nRet = lua_tonumber(L, 1);

	CMonster* pCurrMonster = g_Script.m_pMonster;
	CEventRegion* pEventRegion = (CEventRegion*)pCurrMonster->m_ParentRegion;
	if (!pEventRegion)
	{
		rfalse(4,1,"L_EventRegionFinish2");
		return 0;
	}
	pEventRegion->m_nEventRegionEnd = nRet;
	pEventRegion->m_DyRegionEnd = 1;
	return 0;
}

static int L_SendMsgForEventRegion( lua_State *L )
{
	DWORD dwRegionGID = lua_tonumber(L, 1);
	LPCSTR Txt = static_cast<const char *>(lua_tostring(L, 2));
	if (Txt == NULL) return 0;
	WORD txtShowType = lua_tonumber(L, 3);
	if (txtShowType==NULL) txtShowType=1;	//缺省显示类型为1

	CEventRegion *pRegion = (CEventRegion*)FindRegionByGID(dwRegionGID)->DynamicCast(IID_EVENTREGION);
	if(pRegion==NULL) return 0;

	check_list<LPIObject>::iterator iter = pRegion->m_PlayerList.begin();
	for ( iter; iter != pRegion->m_PlayerList.end(); ++iter )
	{
		if(CPlayer *pPlayer = (CPlayer *)(*iter)->DynamicCast(IID_PLAYER))
			TalkToDnid(pPlayer->m_ClientIndex, Txt, false, false, txtShowType);
	} 

	return 0;
}

// [2012-8-25 +附近聊天]
static int L_SendTalktoNearBy( lua_State *L )
{
	SAChatPublic msg;
	 
	// 说话者的ID
	DWORD dwTalkGID = lua_tonumber(L, 1);
	// 说的内容
	LPCSTR Txt = static_cast<const char *>(lua_tostring(L, 2));
	if (Txt == NULL) return 0;
	// 声音ID
	WORD wTmpSoundID = lua_tonumber(L, 3);
	dwt::strcpy(msg.cCharData, Txt, sizeof(msg.cCharData));

	msg.wSoundId = wTmpSoundID;
	msg.dwGID    = dwTalkGID;

	// 此处是否有通过GID查找对象的方法
	CArea *pTmpParentArea = NULL;
	if (g_Script.m_pMonster && dwTalkGID == g_Script.m_pMonster->GetGID())
	{
		pTmpParentArea = g_Script.m_pMonster->m_ParentArea;
	}
	else if (g_Script.m_pPlayer && dwTalkGID == g_Script.m_pPlayer->GetGID())
	{
		pTmpParentArea = g_Script.m_pPlayer->m_ParentArea;
	}	

	if (pTmpParentArea)
	{
		pTmpParentArea->SendAdj(&msg, msg.GetMySize(), -1);
	}	

	return 0;
}

static int L_SendMsgForPlayer( lua_State *L )
{
	CPlayer *pPlayer = 0;

	DWORD dwPlayerGID = lua_tonumber(L, 1);
	if(dwPlayerGID==0)
	{
		pPlayer = g_Script.m_pPlayer;
	}
	else
	{
		pPlayer = (CPlayer*)GetPlayerByGID(dwPlayerGID)->DynamicCast(IID_PLAYER);
	}
	if(pPlayer==0) return 0;

	LPCSTR Txt = static_cast<const char *>(lua_tostring(L, 2));
	if (Txt == NULL) return 0;
	WORD txtShowType = lua_tonumber(L, 3);
	if (txtShowType==NULL) txtShowType=1;	//缺省显示类型为1

	TalkToDnid(pPlayer->m_ClientIndex, Txt, false, false, txtShowType);

	return 0;
}

static int L_PutPlayerForEventRegion( lua_State *L )
{
	extern BOOL PutPlayerIntoDestRegion(CPlayer *pPlayer, WORD wNewRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID = 0);

	// 获取玩家ID，新场景ID及坐标
	bool	back		= static_cast<bool >(lua_toboolean(L, 1));
	DWORD	curRegID	= static_cast<DWORD>(lua_tonumber (L, 2));
	DWORD	newReID		= static_cast<DWORD>(lua_tonumber (L, 3));
	WORD	toX			= static_cast<WORD >(lua_tonumber (L, 4));
	WORD	toY			= static_cast<WORD >(lua_tonumber (L, 5));

	CEventRegion *pRegion = (CEventRegion*)FindRegionByGID(curRegID)->DynamicCast(IID_EVENTREGION);
	if(pRegion==NULL) return 0;

	//由于PutPlayerIntoDestRegion会操作pRegion->m_PlayerList，所以要先保存出来
	check_list<LPIObject> tempList( pRegion->m_PlayerList );

	check_list<LPIObject>::iterator iter = tempList.begin();
	for ( iter; iter != tempList.end(); ++iter )
	{
		CPlayer *pPlayer = (CPlayer *)(*iter)->DynamicCast(IID_PLAYER);
		if(pPlayer)
		{
			if (back)
			{
				if (pPlayer->GetCurActionID() == EA_DEAD)
				{
					pPlayer->QuestToRelive(0);
					return 1;
				}
				PutPlayerIntoDestRegion(pPlayer, pPlayer->m_PartprevRegID, pPlayer->m_PartprevX, pPlayer->m_PartprevY, 0);
				//PutPlayerIntoDestRegion(pPlayer, pRegion->m_prevRegID, pRegion->m_prevX, pRegion->m_prevY, 0);
			}
			else
				PutPlayerIntoDestRegion(pPlayer, newReID, toX, toY, 0);
		}
	} 

	return 0;
}


static int L_GetLastEnemyID( lua_State *L )
{
	if (!g_Script.m_pMonster)
	{
		rfalse(4,1,"L_GetLastEnemyID");
		return 0;
	}
	lua_pushnumber(L, g_Script.m_pMonster->m_dwLastEnemyID);
	return 1;
}

//这个函数的扩充，只需要添加相应的 switch
static int L_SetMonsterAttr( lua_State *L )
{
	DWORD	regionGID	= static_cast<DWORD>(lua_tonumber (L, 1));
	DWORD	monsterGID	= static_cast<DWORD>(lua_tonumber (L, 2));
	DWORD	attrIndex	= static_cast<DWORD>(lua_tonumber (L, 3));

	CEventRegion *pRegion = (CEventRegion*)FindRegionByGID(regionGID)->DynamicCast(IID_EVENTREGION);
	if(pRegion==NULL) return 0;

	LPIObject pObject =  pRegion->LocateObject(monsterGID);
	CMonster *pMonster = (CMonster*)pObject->DynamicCast(IID_MONSTER);
	if(pMonster==NULL) return 0;

	switch(attrIndex)
	{
	case 1:		//怪物名字
		{
			LPCSTR	txtName		= static_cast<const char *>(lua_tostring(L, 4));
			memcpy(pMonster->m_Property.m_Name, txtName, sizeof(pMonster->m_Property.m_Name));
		}
		break;
	case 2:		//怪物非战斗移动速度
		{
			DWORD	WalkSpeed	= static_cast<DWORD>(lua_tonumber (L, 4));
			pMonster->m_Property.m_WalkSpeed = WalkSpeed;
		}
		break;
	case 3:		//怪物战斗移动速度
		{
			DWORD	FightSpeed	= static_cast<DWORD>(lua_tonumber (L, 4));
			pMonster->m_Property.m_FightSpeed = FightSpeed;
		}
		break;
	default:	
		rfalse("不存在的怪物属性Index：%d", attrIndex); 
		break;
	}		

	return 0;
}

static int L_GetPlayerGIDbyTeam( lua_State *L )
{
	if ( g_Script.m_pPlayer == NULL )
		return 0;
  	if ( g_Script.m_pPlayer->m_dwTeamID == 0 )
  		return 0;
	DWORD	regionGID	= static_cast<DWORD>(lua_tonumber (L, 1));
	DWORD	regionID	= static_cast<DWORD>(lua_tonumber (L, 2));
	extern std::map<DWORD, Team> teamManagerMap;
	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(g_Script.m_pPlayer->m_dwTeamID);
	if (iter == teamManagerMap.end())
		return 0;

	Team &stTeamInfo = iter->second;
	stTeamInfo.ParentReginID = regionGID;
	stTeamInfo.PartregionID	 = regionID;
	for (int i=0; i<5; i++)
	{
		lua_pushnumber(L, stTeamInfo.stTeamPlayer[i].dwGlobal);
	}

	return 5;
}


static int L_CheckTeamDis( lua_State *L )
{
	if ( g_Script.m_pPlayer == NULL )
		return 0;
	if ( g_Script.m_pPlayer->m_dwTeamID == 0 )
		return 0;

	extern std::map<DWORD, Team> teamManagerMap;
	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(g_Script.m_pPlayer->m_dwTeamID);
	if (iter == teamManagerMap.end()) return 0;
	Team &stTeamInfo = iter->second;

	DWORD dwDis	= static_cast<DWORD>(lua_tonumber (L, 1));

	WORD posX, posY, X, Y;
	g_Script.m_pPlayer->GetCurPos(posX, posY);

	CPlayer* pPlayer;
	for (int i=0; i<5; i++)
	{
		pPlayer = stTeamInfo.stTeamPlayer[i].PlayerRef;
		if(g_Script.m_pPlayer->m_ParentRegion == pPlayer->m_ParentRegion)
		{
			pPlayer->GetCurPos(X, Y);
			float dis = sqrt(float(posX * X + posY * Y));
			if(dis > dwDis)
			{
				lua_pushnumber(L, 0);
				lua_pushnumber(L, pPlayer->GetGID());
				return 2;
			}
		}
		else
		{	//不在同一张地图，肯定不符合要求
			lua_pushnumber(L, 0);
			lua_pushnumber(L, pPlayer->GetGID());
			return 2;
		}
	}

	lua_pushnumber(L, 1);
	return 1;
}

static int L_OnMonsterDisplayer( lua_State *L )//服务器想客户端发消息死亡
{
	CMonster* pCurrMonster = g_Script.m_pMonster;
	if (pCurrMonster)
	{
		pCurrMonster->OnDisppear();
	}
	return 1;
}
static int L_OnGivePlayerLiftData( lua_State *L )//服务器副本给奖励
{
	DWORD RegionGID = static_cast<DWORD>(lua_tonumber(L,1));
	DWORD RegionID	= static_cast<DWORD>(lua_tonumber(L,2));
	long  GExp	    = static_cast<long>(lua_tonumber(L,3));
	long  GMoney    = static_cast<long>(lua_tonumber(L,4));
	if (lua_istable(L,5))
	{
		//lua_pushnil(L);
		//while (lua_next(L,5)!=0)
		//{   
		//	long a = static_cast<long>(lua_tonumber(L,-2));//key
		//	if (lua_istable(L,-1))
		//	{
		//		long b = static_cast<DWORD>(GET_FIELD_NUMBER_DIRECTLY(2,"1"));//value
		//		long c = static_cast<long>(GET_FIELD_NUMBER_DIRECTLY(2,"1"));
		//		int i = 1;
		//	}
		//	lua_pop(L,1);
		//}
		//int i = 0;
	}
	return 1;
}

static int L_GetPlayerLevelbyTeam( lua_State *L )
{
	if ( g_Script.m_pPlayer == NULL )
		return 0;
	if ( g_Script.m_pPlayer->m_dwTeamID == 0 )
		return 0;

	extern std::map<DWORD, Team> teamManagerMap;
	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(g_Script.m_pPlayer->m_dwTeamID);
	if (iter == teamManagerMap.end())
		return 0;

	Team stTeamInfo = iter->second;

	for (int i=0; i<5; i++)
	{
		lua_pushnumber(L, stTeamInfo.stTeamPlayer[i].byLevel);
	}

	return 5;
}
static int L_Stand( lua_State* L)
{
	if (!g_Script.m_pMonster)
		return 0;	
	g_Script.m_pMonster->Stand();

	return 1;
}
static int L_GetStaticTargetMonster( lua_State* L)
{
	CMonster* pCurrMonster = g_Script.m_pMonster;
	if (!pCurrMonster)
	{
		rfalse(4,1,"L_GetStaticTargetMonster");
		return 0;
	}
	check_list<LPIObject> *monsterList ;
	monsterList = pCurrMonster->m_ParentRegion->GetMonsterListByAreaIndex(pCurrMonster->m_ParentArea->m_X, pCurrMonster->m_ParentArea->m_Y);
	if (monsterList)
	{
		for (check_list<LPIObject>::iterator it = monsterList->begin(); it != monsterList->end(); ++it)
		{
			LPIObject target = *it;
			CMonster *monster = (CMonster*)target->DynamicCast(IID_MONSTER);
			if (monster && 0 != monster->m_CurHp)
			{
				if(monster->m_Property.m_GroupID == pCurrMonster->m_Property.m_GroupID) //阵营相同不能攻击
					continue;

				float distance = sqrt(pow((pCurrMonster->m_curX-monster->m_curX),2)+pow((pCurrMonster->m_curY-monster->m_curY),2));
				if ((WORD)distance > 400)
					continue;

				DWORD dwLastEnemyID = monster->GetGID();
				lua_pushnumber(L, dwLastEnemyID);
				return 1;
			}
		}
	}
	return 1;
}
static int L_SetAIMonsterAttacker( lua_State* L)
{
	CMonster* pCurrMonster = g_Script.m_pMonster;
	if (!pCurrMonster)
	{
		rfalse(4,1,"L_SetAIMonsterAttacker");
		return 0;
	}
	DWORD distance	= static_cast<DWORD>(lua_tonumber(L,1));		//输入参数周围范围
	DWORD EnemyID	= static_cast<DWORD>(lua_tonumber(L,2));		//目标ID
	BYTE  AttType	= static_cast<BYTE>(lua_tonumber(L,3));			//怪物类型 主动 被动
	check_list<LPIObject> *monsterList ;//param1 范围 param2 找目标 param3 1主动怪 2被动怪 3任意目标
	monsterList = pCurrMonster->m_ParentRegion->GetMonsterListByAreaIndex(pCurrMonster->m_ParentArea->m_X, pCurrMonster->m_ParentArea->m_Y);
	if (monsterList)
	{
		for (check_list<LPIObject>::iterator it = monsterList->begin(); it != monsterList->end(); ++it)
		{
			LPIObject target = *it;
			CMonster *monster = (CMonster*)target->DynamicCast(IID_MONSTER);
			if (monster && 0 != monster->m_CurHp && pCurrMonster->GetGID() != monster->GetGID())
			{
				float distance1 = sqrt(pow((pCurrMonster->m_curX-monster->m_curX),2)+pow((pCurrMonster->m_curY-monster->m_curY),2));
				if ((DWORD)distance1 > distance)
					continue;
				//MAT_ACTIVELY = 0,			// 主动攻击
				//MAT_PASSIVE  = 1,			// 被动攻击
				if (monster->GetCurActionID() == 0 && monster->m_dwLastEnemyID == 0)
				{
					if (monster->m_Property.m_AttType == 0 && AttType == 1)
					{
						monster->m_dwLastEnemyID = EnemyID;
					}
					else if (monster->m_Property.m_AttType == 1 && AttType == 2)
					{
						monster->m_dwLastEnemyID = EnemyID;
					}
					else if (AttType == 3)
					{
						monster->m_dwLastEnemyID = EnemyID;
					}
				}
			}
		}
	}
	return 1;
}
static int L_OnProcessDead( lua_State* L)
{
	CMonster* pCurrMonster = g_Script.m_pMonster;
	if (!pCurrMonster)
	{
		rfalse(4,1,"L_OnProcessDead");
		return 0;
	}
	if (--pCurrMonster->m_ActionFrame <= 0)
		pCurrMonster->SetRenascence(0, 0, pCurrMonster->m_MaxHp, 2);
	return 1;
}
static int L_PartRegionFinish( lua_State* L)
{
	DWORD gid = static_cast<DWORD>(lua_tonumber(L,1));		
	CEventRegion *pRegion = (CEventRegion*)FindRegionByGID(gid)->DynamicCast(IID_EVENTREGION);
	if (pRegion)
	{
		pRegion->m_DyRegionEnd = 1;
	}
	return 1;
}
};
///////////////////////////////////////////////////////////////////////////////
// 名字空间 : 注册函数
///////////////////////////////////////////////////////////////////////////////
namespace ExtraScriptFunctions 
{ 
	void RegisterScriptFunctions_EventRegion()
	{
		#define RegisterFunction( _funcname ) g_Script.RegisterFunction( #_funcname, ExtraLuaFunctions::L_##_funcname );

		RegisterFunction( CreateAIMonster			);
		RegisterFunction( SetMonsterPath			);
		RegisterFunction( GetMonsterCurActionID		);
		RegisterFunction( SetMonsterCurActionID		);
		RegisterFunction( GetMonsterCurPos			);
		RegisterFunction( GetAttackTargetMonster	);
		RegisterFunction( GetAttackTargetPlayer		);
		RegisterFunction( StartRun					);
		RegisterFunction( StartAttack				);
		RegisterFunction( EventRegionFinish			);
		RegisterFunction( SendMsgForEventRegion		);
		RegisterFunction( SendTalktoNearBy		);		
		RegisterFunction( SendMsgForPlayer			);
		RegisterFunction( PutPlayerForEventRegion	);
		RegisterFunction( GetLastEnemyID			);
		RegisterFunction( SetMonsterAttr			);
		RegisterFunction( GetPlayerGIDbyTeam		);
		RegisterFunction( CheckTeamDis				);
		RegisterFunction( OnMonsterDisplayer		);
		RegisterFunction( OnGivePlayerLiftData		);
		RegisterFunction( GetPlayerLevelbyTeam		);
		RegisterFunction( Stand);
		RegisterFunction( GetStaticTargetMonster    );
		RegisterFunction( OnProcessDead				);
		RegisterFunction( StartStaticRun            );
		RegisterFunction( SetAIMonsterAttacker		);
		RegisterFunction( PartRegionFinish			);
		#undef  RegisterFunction
	}
}
///////////////////////////////////////////////////////////////////////////////
