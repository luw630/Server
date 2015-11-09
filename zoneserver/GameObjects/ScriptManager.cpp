// ----- ScriptManager.h -----
//
//   --  Author: RenYi
//   --  Date:   06/13/03
//   --  Desc:   脚本管理类实现文件
// --------------------------------------------------------------------

#include "StdAfx.h"
#include "ScriptManager.h"
#include "FactionManager.h"
#include "Monster.h"
#include "Player.h"
#include "Area.h"
#include "Npc.h"
#include "Item.h"
#include "GameWorld.h"
#include "GlobalFunctions.h"
#include "Networkmodule/logMsgs.h"
#include "Networkmodule/ItemMsgs.h"
#include "Networkmodule/TeamMsgs.h"
#include "Networkmodule/ScriptMsgs.h"
#include "Networkmodule/TongMsgs.h"
#include "Networkmodule/FactionBBSMsgs.h"
#include "Networkmodule/RegionMsgs.h"
#include "Networkmodule/CardPointModifyMsgs.h"
#include "Networkmodule/refreshMsgs.h"
#include "Networkmodule/accountMsgs.h"
#include "Networkmodule/teamMsgs.h"
#include "NETWORKMODULE/DailyMsgs.h"
#include "pub/rpcop.h"
#include "region.h"
#include <time.h>
#include <stack>
#include "Count.h"
#include "sfuncmanager.h"
#include "eventregion.h"
#include "DSpcialItemData.h"
#include "../区域服务器/Environment.h"
#include "pub/wndcusop.h"
#include "pub/traceinfo.h"
#include "Networkmodule/MailMsg.h"
#include "CItemDefine.h"
#include "CItemService.h"
#include "CBuffDefine.h"
#include "CBuffService.h"
#include "extrascriptfunctions/extrascriptfunctions.h"
#include "extrascriptfunctions/lite_lualibrary.hpp"
#include "ItemUser.h"
#include "Mounts.h"
#include "NpcSaleMgr.h"
#include "CItemDropService.h"
#include "CSkillDefine.h"
#include "CSkillService.h"
#include "Pet.h"
#include "networkmodule/netprovidermodule/netprovidermoduleadv.h"
#include "../区域服务器/dmainapp.h"
#include "Random.h"
#include "CMystring.h"
#include "SceneMap.h"
#include "DynamicScene.h"
#include "RegionManager.h"
#include "PlayerData.h"
#include "CPlayerService.h"
#include "CJingMaiService.h"
#include "CGloryAndTitleServer.h"
#include "DynamicArena.h"
#include "DynamicDailyArea.h"
#include "NETWORKMODULE/ShopMsg.h"
#include "NETWORKMODULE/VipMsg.h"
#include "sanguocode/scriptmoudle/scriptmoudle.h"
#include "Networkmodule/SectionMsgs.h"
#include "../../NETWORKMODULE/MailMsg_SanGuo.h"
#include "SanguoCode/MailMoudle/MailMoudle.h"
extern "C" {
#include "lauxlib.h"
}

extern LPIObject FindRegionByID(DWORD ID);
extern void BroadCastAnnounce(LPCSTR info, int iSize);
extern BOOL PutPlayerIntoDestRegion(CPlayer *pPlayer, WORD wNewRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID = 0);
extern LPCSTR GetStringParameter(LPCSTR string, int maxsize, int index);
extern LPIObject GetPlayerByGID(DWORD);
extern void g_AddSpendMoney( __int64 iSpendMoney );
extern BOOL SetFactionMemberJoinTime(CPlayer *pPlayer,DWORD dwNum );
extern DWORD TryInsertClusterID( DWORD );
extern BYTE GetZoneID();
extern BOOL GetTeamInfo( const DWORD dwHandle, Team& stTeamInfo );
//extern BOOL GetFactionHeaderInfo( WORD factionId, pFactionData[i]::SFaction &factionHeader );
extern float GetDirection(float x1, float y1, float x2, float y2);
extern void BroadcastTalkToMsg(const char*pMsg, DWORD wSize);
extern void MonsterTalkInfo(CMonster *pmonster,LPCSTR info);
extern DWORD CreateDynamicScene(const __int32 DR_IID, WORD RegionIndex, int RegionID, int, int, int, DWORD,WORD);
extern DWORD CreateDynamicArena(const __int32 DR_IID, WORD RegionIndex, int RegionID, int, int, int, DWORD, WORD);
extern DWORD CreateDynamicDailyArea(const __int32 DR_IID, WORD RegionIndex, int RegionID, int, int, int, DWORD);
extern LPIObject FindRegionByGID(DWORD GID);
extern LPIObject GetObjectByGID(DWORD GID);
extern LPCSTR GetIPString(DNID dnid, WORD *port);
extern int CALLBACK GetAllPlayerObjectEx(LPIObject& pObject, std::vector< const CPlayer* >* pPlayerVec);
//extern void GetAddAttribute( SEquipment::Attribute::Unit *unit, WORD id, int maxNum, WORD coff[ 57 ] = NULL, BYTE level = 0, BYTE itemType = 0 );
//extern CRegionManager *s_pUniqueThis ; 

CScriptManager g_Script;

extern SScoreTable s_Scores;
extern DWORD rcdMoneyRank;

bool CScriptManager::IfSendTheMultiPageMsg = false;
//#define FILESIZE  0x800000   // 1MB
const DWORD FILESIZE =1024 * 1024 * 10;

DWORD  g_dServerID = 0;


extern double _result ;
extern bool   _isnul  ;
const char    *_strresult = 0;
#define GET_FIELD_NUMBER_DIRECTLY(_table, _key) ((lua_getfield(L, _table, _key),	\
	(lua_isnil(L, -1)												\
	? (_isnul = true, _result = 0)									\
	: (_isnul = false, _result = lua_tonumber(L, -1))),				\
	lua_pop(L, 1)), _result)

#define GET_FIELD_STRING_DIRECTLY(_table, _key) ((lua_getfield(L, _table, _key),	\
	(lua_isnil(L, -1)												\
	? (_isnul = true, _strresult = 0)									\
	: (_isnul = false, _strresult = lua_tostring(L, -1))),				\
	lua_pop(L, 1)), _strresult)

LPCSTR GetNpcScriptInfo( LPCSTR info )
{
	//     LPCSTR sinfo = NULL;
	//     lua_Debug ar;
	//     memset( &ar, 0, sizeof( ar ) );
	//     if ( int ret1 = lua_getstack( g_Script.ls, 1, &ar ) )
	//     {
	//         if ( int ret2 = lua_getinfo( g_Script.ls, "Sl", &ar ) )
	//             sinfo = FormatString( "%s(%d)", ar.short_src, ar.currentline );
	//     }
	// 
	//     static char tempbuffer[1024];
	//     sprintf( tempbuffer, "%s[npc=%s][script=%s]", info, g_Script.m_pNpc ? g_Script.m_pNpc->m_Property.m_Name : "null", sinfo ? sinfo : "null" );
	// 
	//     return tempbuffer;
	return 0;
}

void CScriptManager::SetFightCondition(class CFightObject *pAttacker)
{
	m_pAttacker = pAttacker;
}

void CScriptManager::CleanFightCondition()
{
	m_pAttacker = 0;
}

void CScriptManager::SetCondition( CMonster *pMonster, CPlayer *pPlayer, CNpc *pNpc, CBuilding *pBuilding )
{
	m_pPlayer = pPlayer;
	m_pMonster = pMonster;
	m_pNpc = pNpc;
	m_pBuilding = pBuilding;
}

void CScriptManager::CleanCondition()
{
	m_pMonster = NULL;
	m_pPlayer = NULL;
	m_pNpc = NULL;
	m_pBuilding = NULL;
	m_pUseItem = NULL;
}

void CScriptManager::PushCondition( TempCondition &tmp )
{
	tmp.m_pOldMonster  = m_pMonster;
	tmp.m_pOldPlayer   = m_pPlayer;
	tmp.m_pOldNpc      = m_pNpc;
	tmp.m_pOldBuilding = m_pBuilding; 
}

void CScriptManager::PopCondition( TempCondition &tmp )
{
	m_pMonster   = tmp.m_pOldMonster;
	m_pPlayer    = tmp.m_pOldPlayer;
	m_pNpc       = tmp.m_pOldNpc;
	m_pBuilding  = tmp.m_pOldBuilding;
}

void CScriptManager::PushFightObjectCondition( CScriptManager::TempFightObjectCondition &tmp )
{
	tmp.m_OldpAttacker  = m_pAttacker;
}

void CScriptManager::PopFightObjectCondition(CScriptManager::TempFightObjectCondition &tmp)
{
	m_pAttacker = tmp.m_OldpAttacker;
}

void CScriptManager::GetCondition( CScriptManager::TempCondition &tmp )
{
	tmp.m_pOldMonster	= m_pMonster;
	tmp.m_pOldPlayer	= m_pPlayer;
	tmp.m_pOldNpc		= m_pNpc;
	tmp.m_pOldBuilding	= m_pBuilding;
}

BOOL CScriptManager::TriggerEvent(const char *name, int nEvent)
{
	g_Script.SetGlobalValue( CScriptValue(nEvent), "EventID");
	g_Script.CallFunc(name);

	return TRUE;
}

BOOL CScriptManager::TriggerEvent(const stEvent *pTE)
{
	/*
	switch(pTE->TE_Type)
	{
	case stEvent::TE_MONSTER_DEAD:
	break;
	case stEvent::TE_MONSTER_REFRESH:
	TriggerEvent("Monster_Refresh ", g_Script.m_pMonster->m_Property.m_wRefreshScriptID );
	break;
	case stEvent::TE_MONSTER_AI:
	TriggerEvent("Monster_AI ", g_Script.m_pMonster->m_Property.m_wAIScriptID );
	break;
	}
	//*/

	return TRUE;
}

void CScriptManager::SetScriptPath(const char *rootpath)
{
	g_Script.SetPath(rootpath);
	g_Script.SetGlobalValue( CScriptValue( rootpath ), "filepath");
}
// execute lua file or compile lua file
int CScriptManager::L_compilefile(lua_State *L)
{
	//const char *szFileName = static_cast<const char*>(lua_tostring(L, 1));
	//if(!szFileName) return 0;
	//int nRet = lua_compilefile(L, szFileName);
	//lua_pushnumber(L, nRet);
	//return 1;

	return 0;
}
//---- out debug message -------------------------------------------------------------
int CScriptManager::L_rfalse(lua_State *L)
{
	LPCSTR msg = static_cast<const char *>(lua_tostring(L, 1));
	if (msg == NULL)
		msg = "(null)";

	rfalse(2, 1, msg);
	return 0;
}
//---- end ---------------------------------------------------------------------------
int CScriptManager::L_activate(lua_State *L)
{
	/*
	if (g_Script.m_pMonster)
	{
	SMonsterData *pData = CMonster::GetMonsterData(g_Script.m_pMonster->m_Property.id);
	if (pData != NULL) 
	{
	g_Script.m_pMonster->m_CurHp = pData->hp;
	g_Script.m_pMonster->m_wDAM = pData->outAtkBase;
	return 1;
	}
	}//*/

	return 0;
}

int CScriptManager::L_deactivate(lua_State *L)
{

	return 0;
}

int CScriptManager::L_getAttribute(lua_State *L)
{
	//     WORD id = static_cast<WORD>(lua_tonumber(L, 1));
	// 
	//     SMonsterData *pData = CMonster::GetMonsterData(id);
	//     if (pData == NULL) 
	//         return FALSE;
	// 
	//     lua_pushnumber(L, pData->hp);

	return 0;
}

int CScriptManager::L_setAttribute(lua_State *L)              // 怪物重生
{
	return 0;
}

static DWORD itemAccumulator = 0;
static DWORD prevTimestamp = 0;
static time_t timeSegment = 0;

static BOOL timeAccumulatorInit()
{
	tm timeMaker; 
	ZeroMemory(&timeMaker, sizeof(tm)); 
	timeMaker.tm_year = SItemBase::YEAR_SEGMENT - 1900;
	timeSegment = mktime(&timeMaker);
	return TRUE;
}

static BOOL timeAccumulatorInited = timeAccumulatorInit();

BOOL GenerateNewUniqueId(SItemBase &item)
{
	// 获取一个32bit的时间戳，30秒一个点，这样32bit可以保存上千年的数据
	DWORD timestamp32bit = (DWORD)((time(NULL) - timeSegment)/30);

	// 为了兼容数据库1拖多而导致的ID冲突，需要尝试在这32bit中插入分组编号
	item.details.uniqueTimestamp = TryInsertClusterID(timestamp32bit);

	// 已经过了一个时段，累加计数器需要清空，重新计算
	if (item.details.uniqueTimestamp > prevTimestamp)
		prevTimestamp = item.details.uniqueTimestamp, itemAccumulator = 0;

	// 累加器越界，不能继续创建下去了，等下一个30秒时段
	if (itemAccumulator >= SItemBase::MAX_ACCUMULATOR) 
		return FALSE;

	// 目前只有单服务器，所以ID默认设置为1
	item.details.uniqueServerId = 1;		
	item.details.uniqueCreateIndex = itemAccumulator++;
	return TRUE;
}

struct IRcd { DWORD type, count; };
static std::map< DWORD, IRcd > itemRcdMap;
int InitRcdMap()
{
	std::ifstream stream( "data\\item\\itemrcd.txt" );
	if ( !stream.is_open() )
		return -1;

	char lineBuffer[1024];
	while ( !stream.eof() )
	{
		stream.getline( lineBuffer, sizeof( lineBuffer ) );
		if ( *lineBuffer == 0 || *( LPWORD )lineBuffer == '//' || *( LPWORD )lineBuffer == '--' )
			continue;

		std::strstream lineStream( lineBuffer, ( int )strlen( lineBuffer ), ios_base::in );

		int id, type;
		lineStream >> id >> type;
		if ( ( type & 0x8000 ) == 0 )
		{
			// 如果没有打特殊标记的话，则需要判断是否已经存在该数据，不然会导致统计的数据被重置。
			std::map< DWORD, IRcd >::iterator it = itemRcdMap.find( id );
			if ( it != itemRcdMap.end() )
				continue;
		}

		IRcd &rcd = itemRcdMap[id];
		rcd.type = type;
		rcd.count = 0;
	}

	return ( int )itemRcdMap.size();
}

DWORD GetCheckItemFlag( WORD itemId )
{
	std::map< DWORD, IRcd >::iterator it = itemRcdMap.find( itemId );
	if ( it == itemRcdMap.end() )
		return 0;

	return it->second.type;
}

int CScriptManager::L_ClearPlayerCSItem(lua_State *L)
{
	return 0;
}

//update by ly 2014/4/12 更新激活背包格子时，只能按顺序激活。保持和原来的接口参数一致。(之前的函数为激活指定索引位置的格子)
int CScriptManager::L_ActivePackage(lua_State *L)
{	
	if (!g_Script.m_pPlayer)
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	//WORD wStartPos = (WORD)lua_tointeger(L, 1); // 起始位置
	//WORD wEndPos = (WORD)lua_tointeger(L, 2); // 结束位置
	BOOL bToActive = (BOOL)lua_tointeger(L, 3); // 是否激活

	int wFirstParam = (int)lua_tointeger(L, 1);
	int wSecondParam = (int)lua_tointeger(L, 2);
	int wActiveCellNum = wSecondParam - wFirstParam;
	if (wActiveCellNum <= 0 )	//执行失败
	{
		lua_pushboolean(L, 0);
		return 1;
	}
	WORD wStartPos = 0;
	WORD wEndPos = 0;
	if (bToActive)
	{
		wStartPos = BaseActivedPackage + g_Script.m_pPlayer->m_Property.m_wExtGoodsActivedNum; // 起始位置
		wEndPos = BaseActivedPackage + g_Script.m_pPlayer->m_Property.m_wExtGoodsActivedNum + wActiveCellNum; // 结束位置
	}
	else
	{
		wStartPos = BaseActivedPackage + g_Script.m_pPlayer->m_Property.m_wExtGoodsActivedNum - wActiveCellNum; // 起始位置
		wEndPos = BaseActivedPackage + g_Script.m_pPlayer->m_Property.m_wExtGoodsActivedNum; // 结束位置
	}

	//update by ly 2014/4/10 现在的玩家最多只能有60个格子
	/*if (PackageAllCells < wEndPos || wStartPos >= wEndPos) 	*/
	if (BaseEnd < wEndPos || wStartPos > wEndPos)
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	if (bToActive)
	{
		for (WORD i = wStartPos; i<wEndPos; ++i)
		{
			g_Script.m_pPlayer->ValidationPos(i);
			g_Script.m_pPlayer->m_PackageItems[i].m_State |= SItemWithFlag::IWF_ACTIVED;
			g_Script.m_pPlayer->ValidationPos(i);
		}
		g_Script.m_pPlayer->m_Property.m_wExtGoodsActivedNum += wActiveCellNum/*(wEndPos-wStartPos)*/;
	}
	else 
	{
		//不能锁定玩家默认拥有的12格子
		if (g_Script.m_pPlayer->m_Property.m_wExtGoodsActivedNum - wActiveCellNum < 0)	
		{
			TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "不能锁定玩家默认拥有的12个格子");
			lua_pushboolean(L, 0);
			return 1;
		}
		for (WORD i = wStartPos; i<wEndPos; ++i)
		{
			g_Script.m_pPlayer->ValidationPos(i);
			g_Script.m_pPlayer->m_PackageItems[i].m_State &= (~SItemWithFlag::IWF_ACTIVED);
			g_Script.m_pPlayer->ValidationPos(i);
		}
		g_Script.m_pPlayer->m_Property.m_wExtGoodsActivedNum -= wActiveCellNum/*(wEndPos-wStartPos)*/;
	}

	lua_pushboolean(L, 1);
	return 1;
}

int CScriptManager::L_SendActivePackage(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 1;

	SAActiveExtPackage resultMsg;
	resultMsg.wResult = (WORD)lua_tointeger(L, 1);
	//resultMsg.wExtendActivedCount = (WORD)lua_tointeger(L, 2);
	resultMsg.wExtendActivedCount = g_Script.m_pPlayer->m_Property.m_wExtGoodsActivedNum;
	//resultMsg.wResult = SAActiveExtPackage::EAEP_SUCCESS;
	resultMsg.dwRemainYuanBao = g_Script.m_pPlayer->m_Property.m_YuanBao;
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &resultMsg, sizeof(SAActiveExtPackage));

	return 1;
}


int CScriptManager::L_activetelegry(lua_State* L)
{
	if (!g_Script.m_pPlayer)
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	WORD nTelegryID = (WORD)lua_tointeger(L, 1);
	if (0 == nTelegryID)
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	if (g_Script.m_pPlayer->ActivaTelergy(nTelegryID, "学习心法！"))
	{
		lua_pushboolean(L, 1);
		return 1;	
	}
	return 1;	
}

int CScriptManager::L_activeskill(lua_State* L)
{

	CPlayer *pPlayer = g_Script.m_pPlayer;
	if (lua_isnumber(L, 2))
	{
		int PlayerGid = lua_tonumber(L, 2);
		if (PlayerGid != 0)
			pPlayer = (CPlayer*)GetPlayerByGID(PlayerGid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	WORD nSkillID = (WORD)lua_tointeger(L, 1);
	if (0 == nSkillID)
	{
		lua_pushboolean(L, 0);
		return 1;
	}
	if (pPlayer->ActivaSkill(nSkillID, "学习技能！"))
	{
		lua_pushboolean(L, 1);
	}
	return 1;
}

int CScriptManager::L_activeTempskill(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	WORD nSkillID = static_cast<WORD>(lua_tonumber(L,1));

	short nIndex = g_Script.m_pPlayer->ActiveTempSkill(nSkillID);
	lua_pushnumber(L,nIndex);

	return 1;
}

int CScriptManager::L_CoolingTempSkill(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	WORD Index = static_cast<WORD>(lua_tonumber(L,1));
	g_Script.m_pPlayer->CoolingTempSkill(Index);
	return 1;
}

int CScriptManager::L_giveExp(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	DWORD exp = static_cast<DWORD>(lua_tonumber(L, 1));

	g_Script.m_pPlayer->SendAddPlayerExp(exp, SAExpChangeMsg::TASKFINISHED, "ScriptGive");

	return 1;
}

int CScriptManager::L_givePet(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD petID = static_cast<WORD>(lua_tonumber(L, 1));	// ID

	if (0 == petID)
		return 0;

	g_Script.m_pPlayer->CreatePets(petID, 1);

	return 1;
}

int CScriptManager::L_giveFightPet(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD fpID = static_cast<WORD>(lua_tonumber(L, 1));	// ID

	if (0 == fpID)
		return 0;

	bool result = g_Script.m_pPlayer->CreateFightPet(fpID);
	lua_pushnumber(L, result ? 1 : 0);

	return 1;
}

int CScriptManager::L_giveMount(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD mountID = static_cast<WORD>(lua_tonumber(L, 1));	// ID

	if (0 == mountID)
		return 0;

	bool result = g_Script.m_pPlayer->CreateMounts(mountID, 1);
	lua_pushnumber(L, result ? 1 : 0);

	return 1;
}

int CScriptManager::L_giveMoney(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	DWORD MType = static_cast<DWORD>(lua_tonumber(L, 1));
	DWORD MNum  = static_cast<DWORD>(lua_tonumber(L, 2));

	if (0 == MType || 0 == MNum)
		return 0;

	g_Script.m_pPlayer->AddPlayerMoney(MType, MNum);

	return 0;
}

int CScriptManager::L_giveGoods(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	DWORD index = static_cast<DWORD>(lua_tonumber(L, 1));	// ID
	DWORD num   = static_cast<DWORD>(lua_tonumber(L, 2));	// Num

	if (0 == index || 0 == num)
		return 0;

	std::list<SAddItemInfo> itemList;
	itemList.push_back(SAddItemInfo(index, num));

	if (!itemList.empty()&&g_Script.m_pPlayer->CanAddItems(itemList))
	{
		g_Script.m_pPlayer->StartAddItems(itemList);
		lua_tonumber(L,1);
		return 1;
	}
	lua_tonumber(L,0);
	return 0;
}

int CScriptManager::L_giveSp(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	int spMount = static_cast<int>(lua_tonumber(L, 1));
	if (0 == spMount)
		return 0;

	g_Script.m_pPlayer->AddPlayerSp(spMount);

	return 1;
}

int CScriptManager::L_reitem(lua_State *L)
{
	return 0;
}

// 点击Npc后，服务器的回应消息
int CScriptManager::L_setmessage(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	SAClickNpcMsg msg;

	msg.wScriptID = static_cast<DWORD>(lua_tonumber(L, 1));

	if (0 == msg.wScriptID)
		return 0;
	if (g_Script.m_pPlayer->m_ClickTaskFlag == 1)
	{
		bool Flag = false;
		for (CPlayer::TASKINFO::iterator task = g_Script.m_pPlayer->m_TaskInfo.begin(); task != g_Script.m_pPlayer->m_TaskInfo.end(); ++task)
		{
			if ( (*task).second != TS_ACCEPTED)			// 存入任务状态
			{
				Flag = true;
				break;
			}
		}
		if (Flag == false)
		{
			return 0;
		}
	}

	ZeroMemory(&msg.wMenuIndex,sizeof(msg.wMenuIndex));
	ZeroMemory(&msg.wTaskData, sizeof(msg.wTaskData));

	g_Script.m_pPlayer->temporaryVerifier = msg.verifier = timeGetTime();	// 验证信息

	msg.wTaskNum = g_Script.m_pPlayer->m_TaskInfo.size();

	int index = 0;
	for (CPlayer::TASKINFO::iterator task = g_Script.m_pPlayer->m_TaskInfo.begin(); task != g_Script.m_pPlayer->m_TaskInfo.end(); ++task)
	{
		if (64 == index)
			break;

		WORD taskid = (*task).first;
		WORD state = g_Script.m_pPlayer->GetTaskStatus(taskid);

		// 		msg.wTaskData[index++] = taskid;			
		// 		msg.wTaskData[index++] = state;			
		msg.wTaskData[index++] = task->first;				
		msg.wTaskData[index++] = task->second;		
	}


	lite::Variant ret;
	int FlagPage = 0;
	int menuIndex = 0;
	LuaFunctor(g_Script,"GetUIPageType")[msg.wScriptID](&ret);
	if (lite::Variant::VT_INTEGER == ret.dataType)
	{
		FlagPage = ret;
		if (FlagPage == 0)//不分页
		{
			if (g_Script.m_pPlayer->m_MenuInfo.size())
			{
				msg.wMenuIndex[menuIndex++] = *g_Script.m_pPlayer->m_MenuInfo.begin();	

			}
		}
		else//分页
		{
			for (CPlayer::MENUINFO::iterator menu = g_Script.m_pPlayer->m_MenuInfo.begin(); menu != g_Script.m_pPlayer->m_MenuInfo.end(); ++menu)
			{
				if (20 == menuIndex)
					break;

				msg.wMenuIndex[menuIndex++] = *menu;
			}
		}
	}
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(msg)-(64-index)*sizeof(WORD));

	IfSendTheMultiPageMsg = true;

	return 0;
}

// 点击选项 服务器的回应消息
int CScriptManager::L_setmessageA(lua_State *L) 
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	return 0;
}

// ----- 还原买卖物品栏为空，这主要为重载脚本方便着想 ------------------------------
// 这里做了扩展修改 增加了一个脚本参数的传入 为要为了区分是玩家还是NPC的买卖物品栏
int CScriptManager::L_zero(lua_State *L)
{
	return 0;
}

int CScriptManager::L_playcg(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD CgID = static_cast<WORD>(lua_tonumber(L ,1));
	if (0 == CgID)
		return 0;

	SAPlayCG msg;
	msg.wCgID = CgID;

	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(SAPlayCG));

	return 0;
}

int CScriptManager::L_setmessageB(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD saleIndex = static_cast<WORD>(lua_tonumber(L ,1));
	WORD byIndex   = static_cast<WORD>(lua_tonumber(L, 2));
	if (0 == byIndex)
		return 0;

	SAChoseMenuMsg msg;
	memset(msg.m_Goods, 0, sizeof(msg.m_Goods));
	msg.m_Index = byIndex;
	msg.m_count = 0;

	// NPC买卖
	if (SAChoseMenuMsg::CM_BUYPANEL == byIndex)
	{
		CNpc *buyNpc = g_Script.m_pPlayer->GetCurrentNpc();
		if (!buyNpc)
			return 0;

		NpcGoods* saleInfo = CNpcSaleService::GetInstance().GetSaleInfo(saleIndex);
		if (!saleInfo)
			return 0;

		// 分页消息的发送
		if (IfSendTheMultiPageMsg)
		{
			SAGetMenuName _msg;

			_msg.wScriptID = buyNpc->m_Property.m_wClickScriptID;
			g_Script.m_pPlayer->temporaryVerifier = timeGetTime();	// 验证信息
			_msg.verifier = g_Script.m_pPlayer->temporaryVerifier;

			DWORD pageIndex = 0;
			for (CPlayer::MENUINFO::iterator menu = g_Script.m_pPlayer->m_MenuInfo.begin(); menu != g_Script.m_pPlayer->m_MenuInfo.end(); ++menu)
			{
				if (SAGetMenuName::MI_INDEX == pageIndex)
					break;

				_msg.wMenuIndex[pageIndex++] = *menu;
			}

			_msg.wIndex = pageIndex;
			g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &_msg, sizeof(SAGetMenuName));

			IfSendTheMultiPageMsg = false;
		}

		memcpy(msg.m_Goods, saleInfo->m_Goods, sizeof(saleInfo->m_Goods));
		msg.m_count = saleInfo->m_GoodCount;
		g_Script.m_pPlayer->m_CurShopID = saleIndex;
	}

	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(SAChoseMenuMsg) - ((NpcGoods::ItemCount - msg.m_count) * sizeof(DWORD) * 2));

	// 清除菜单栏选项 ->ahj
	g_Script.m_pPlayer->m_MenuInfo.clear();

	return 0;
}

int CScriptManager::L_setmessageC(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	SAOutSchoolMsg msg;
	msg.dwGlobalID = g_Script.m_pPlayer->GetGID();
	msg.byBRON = 1;   
	msg.bySchool = g_Script.m_pPlayer->m_Property.m_School;
	msg.bySex = g_Script.m_pPlayer->m_Property.m_Sex;

	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(SAOutSchoolMsg));

	return 0;
}

int CScriptManager::L_setmessageR(lua_State *L)
{
	WORD wRType = static_cast<WORD>(lua_tonumber(L,1));
	SABackMsg msg;
	msg.byType = (BYTE)wRType;

	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(SABackMsg));
	return 0;
}
// -- end -----------------------------------------------------------------------------------
int CScriptManager::L_setlistsize(lua_State *L)
{
	int nMListSize = static_cast<int>(lua_tonumber(L, 1));
	int nNListSize = static_cast<int>(lua_tonumber(L, 2));
	//...

	// 已经不考虑脚本初始化这两个数据了，直接程序初始化为65535
	//g_Script.SetMListSize(nMListSize);
	//g_Script.SetNListSize(nNListSize);

	return 0;
}

int CScriptManager::L_delobject(lua_State *L)
{
	WORD  wRegionID = static_cast<WORD>(lua_tonumber(L, 1));
	WORD  wObject = static_cast<WORD>(lua_tonumber(L, 2));
	WORD  wID = static_cast<WORD>(lua_tonumber(L, 3));
	DWORD dwRegionGID = static_cast<DWORD>(lua_tonumber(L, 4));

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

	if (pRegion == NULL)
		return 0;

	switch(wObject) {
	case OT_MONSTER:
		pRegion->DelMonster(g_Script.GetMonsterID(wID));
		g_Script.SetMonsterID(wID, 0);
		break;

	case OT_NPC:
		{
			pRegion->DelNpc(g_Script.GetNpcID(wID));
			g_Script.SetNpcID(wID, 0);
		}
		break;

	default:
		return 0;
	}

	return 0;
}


int CScriptManager::L_setnpcgoodscount(lua_State *L)
{
	CNpcSaleService::GetInstance().ReSetInfo();
	return 0;
}

int CScriptManager::L_loaditem(lua_State *L)
{
	WORD  NpcID		= static_cast<WORD >(lua_tonumber(L, 1));
	DWORD GoodID	= static_cast<DWORD>(lua_tonumber(L, 2));
	WORD  GoodCount	= static_cast<WORD >(lua_tonumber(L, 3));

	if (0 == NpcID || 0 == GoodID || 0 == GoodCount)
		return 0;

	CNpcSaleService::GetInstance().LoadItemInfo(NpcID, GoodID, GoodCount);

	return 0;
}


int CScriptManager::L_checkgoods(lua_State *L)
{
	DWORD index		= static_cast<DWORD>(lua_tonumber(L, 1));		// 道具编号
	DWORD number	= static_cast<DWORD>(lua_tonumber(L, 2));		// 检测数量
	BOOL  checkOnly = static_cast<BOOL >(lua_tonumber(L, 3));		// 只是检测还是同时收取
	DWORD dwSID		= static_cast<DWORD>(lua_tonumber(L, 4));

	CPlayer *pPlayer = 0;

	if (0 == dwSID)
		pPlayer = g_Script.m_pPlayer;
	else
		pPlayer = (CPlayer *)GetPlayerBySID(dwSID)->DynamicCast(IID_PLAYER);

	if (!pPlayer)
		return 0;

	lua_pushnumber(L, pPlayer->CheckGoods(index, number, checkOnly));
	return 1;
}

int CScriptManager::L_getitemnum(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		int nparamnum = lua_gettop(L);
		if (nparamnum < 2)
		{
			return 0;
		}
		DWORD pygid = static_cast<DWORD>(lua_tonumber(L, 1));
		CPlayer *pPlayer = (CPlayer*)GetPlayerByGID(pygid)->DynamicCast(IID_PLAYER);
		if (!pPlayer)
		{
			return 0;
		}
		DWORD index = static_cast<DWORD>(lua_tonumber(L, 2));     // 道具编号
		lua_pushnumber(L, pPlayer->GetItemNum(index, XYD_FT_WHATEVER));
		return 1;
	}

	DWORD index = static_cast<DWORD>(lua_tonumber(L, 1));     // 道具编号

	lua_pushnumber(L, g_Script.m_pPlayer->GetItemNum(index, XYD_FT_WHATEVER));

	return 1;
}


int CScriptManager::L_StartCollection(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	DWORD npcid	= (DWORD)lua_tointeger(L, 1);
	DWORD time	= (DWORD)lua_tointeger(L, 2);
	int x		= (int)lua_tointeger(L, 3);
	int y		= (int)lua_tointeger(L, 4);

	g_Script.m_pPlayer->StartCollect(npcid, time, x, y, lua_tostring(L, 5));
	return 0;
}

int CScriptManager::L_MakeNpcDisappear(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	DWORD time = (DWORD)lua_tointeger(L, 1);
	if (0 == time)
		return 0;

	CNpc *npc = g_Script.m_pPlayer->GetCurrentNpc();
	if (npc)
	{
		npc->MakeDisappear(time);
	}

	return 0;
}


//add by ly 2014/4/4 
int CScriptManager::L_GetPlayerSID(lua_State *L)	//通过玩家名字获取玩家的SID
{
	DWORD dwSID = 0;
	const char *pPlayerName = lua_tostring(L, 1);
	CPlayer *pPlayer = (CPlayer*)GetPlayerByName(pPlayerName)->DynamicCast(IID_PLAYER);
	if (pPlayer == NULL)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "没有该姓名的玩家");
		return 0;
	}	dwSID = pPlayer->GetSID();
	lua_pushnumber(L, dwSID);
	return 1;
}

// 得到玩家的数据
int CScriptManager::L_getplayerGID(lua_State *L)
{
	DWORD dwSID = (DWORD)lua_tonumber(L,1);
	if( dwSID==0 )
	{
		if (g_Script.m_pPlayer)
		{
			lua_pushnumber(L,g_Script.m_pPlayer->GetGID());
			return 1;
		}
	}
	CPlayer *pPlayer = 0;
	pPlayer = (CPlayer *)GetPlayerBySID(dwSID)->DynamicCast(IID_PLAYER);
	if( pPlayer==0 )return 0;

	lua_pushnumber(L,pPlayer->GetGID());
	return 1;
}

// 获得玩家的数据
int CScriptManager::L_getplayerdata(lua_State *L)
{
	DWORD dwGID = static_cast<DWORD>(lua_tonumber(L, 1));
	CPlayer* pPlayer;
	if(dwGID==0)
	{
		pPlayer = g_Script.m_pPlayer;
	}
	else
	{
		pPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer) return 0;

	int index = static_cast<int>(lua_tonumber(L, 2));
	switch (index)
	{
	case 0:		//当前攻击值
		lua_pushnumber(L, pPlayer->m_GongJi);
		break;
	case 1:		//当前防御值
		lua_pushnumber(L, pPlayer->m_FangYu);
		break;
	case 2:		//暴击
		lua_pushnumber(L, pPlayer->m_BaoJi);
		break;
	case 10:		// 等级
		//lua_pushnumber(L, pPlayer->m_Property.m_Level);
		lua_pushnumber(L, pPlayer->m_FixData.m_dwLevel);
		break;

	case 100:		// 性别
		lua_pushnumber(L, pPlayer->m_Property.m_Sex);
		break;
	case 4:		//最大生命值
		lua_pushnumber(L, pPlayer->m_MaxHp);
		break;

	case 5:		// 帮派名字
		lua_pushstring(L, pPlayer->m_Property.m_szTongName);
		break;
	case 6:		//侠义值
		lua_pushnumber(L, pPlayer->m_Property.m_XYValue);
		break;
	case 7:		//杀孽值
		lua_pushnumber(L, pPlayer->m_wPKValue);
		break;
	case 8:		//精力值
		lua_pushnumber(L, pPlayer->m_Property.m_CurJp);
		break;
	case 9:		//经验值
		lua_pushnumber(L, pPlayer->m_Property.m_Exp);
		break;
		case 21:		//真气值
			lua_pushnumber(L, pPlayer->m_Property.m_CurSp);
		break;
	case 11:		//当前移动速度值
		lua_pushnumber(L, pPlayer->m_OriSpeed);
		break;
	case 12:		//当前攻击速度
		lua_pushnumber(L, pPlayer->m_AtkSpeed);
		break;
	case 13:		//当前轻功点数
		lua_pushnumber(L, pPlayer->m_Property.m_QingShen);
		break;
	case 14:		//当前强健点数
		lua_pushnumber(L, pPlayer->m_Property.m_JianShen);
		break;
	case 15:		// 配偶名字
		//lua_pushnumber(L, pPlayer->m_Property.m_Sex);
		break;
	case 16:		//最大内力值
		lua_pushnumber(L, pPlayer->m_MaxMp);
		break;
	case 17:		//最大体力值
		lua_pushnumber(L, pPlayer->m_MaxTp);
		break;
	case 18:		//当前生命值
		lua_pushnumber(L, pPlayer->m_CurHp);
		break;
	case 19:		//当前内力值
		lua_pushnumber(L, pPlayer->m_CurMp);
		break;
	case 20:		//当前体力值
		lua_pushnumber(L, pPlayer->m_CurTp);
		break;
	case 23:		// 门派
		lua_pushnumber(L, pPlayer->m_Property.m_School);
		break;
	case 3:		//当前闪避值
		lua_pushnumber(L, pPlayer->m_ShanBi);
		break;
	case 25:		//当前防御点数
		lua_pushnumber(L, pPlayer->m_Property.m_FangYu);
		break;
	case 26:		//当前进攻点数
		lua_pushnumber(L, pPlayer->m_Property.m_JingGong);
		break;
	case 27:		//当前可用点数值
		lua_pushnumber(L, pPlayer->m_Property.m_RemainPoint);
		break;
	case 28:		//玩家的角色名
		lua_pushstring(L, pPlayer->GetName());
		break;
	case 29:		//玩家的账号名称
		lua_pushstring(L, pPlayer->GetAccount());
		break;
	case 30:		//玩家当前不绑定金币数量
		lua_pushnumber(L, pPlayer->m_Property.m_Money);//金币
		break;
	case 31:		//玩家当前绑定金币数量
		lua_pushnumber(L, pPlayer->m_Property.m_BindMoney);//银两
		break;
	case 32:		//玩家当前元宝数量
		lua_pushnumber(L, pPlayer->m_Property.m_YuanBao);
		break;
	case 33:		//玩家伤害减免
		lua_pushnumber(L, pPlayer->m_ReduceDamage);
		break;
	case 34:		//玩家绝对伤害
		lua_pushnumber(L, pPlayer->m_AbsDamage);
		break;
	case 35:		//玩家无视防御
		lua_pushnumber(L, pPlayer->m_NonFangyu);
		break;
	case 36:		//玩家麒麟臂等级
		lua_pushnumber(L, pPlayer->m_Property.byKylinArmLevel);
		break;
	case 99:		//祈福标识
		lua_pushnumber(L,pPlayer->m_Property.m_BlessFlag);
		break;
	case 37:        //全局编号
		lua_pushnumber(L, pPlayer->GetGID());
		break;
	case 38:	//sid
		lua_pushnumber(L, pPlayer->GetSID());
		break;
	case 39:  //pos
		{
			WORD wx;
			WORD wy;
			pPlayer->GetCurPos(wx,wy);
			lua_createtable(L,0,0);
			lua_pushnumber(L,wx);
			lua_rawseti(L,-2,1);
			lua_pushnumber(L,wy);
			lua_rawseti(L,-2,2);
			//	lua_settable(L,-3);
			// 			lua_rawset(L,1);
			// 			lua_pushnumber(L,wy);
			// 			lua_rawset(L,2);
		}
		break;
	case 40:  //状态动作
		lua_pushnumber(L,pPlayer->GetCurActionID());
		break;
	case 41:  //float pos
	{
		lua_createtable(L, 0, 0);
		lua_pushnumber(L, pPlayer->m_curX);
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, pPlayer->m_curY);
		lua_rawseti(L, -2, 2);
		//	lua_settable(L,-3);
		// 			lua_rawset(L,1);
		// 			lua_pushnumber(L,wy);
		// 			lua_rawset(L,2);
	}
		break;
	case 42: //玩家变身状态
	{
		DWORD state = pPlayer->m_Property.m_ShowState;
		lua_pushnumber(L, state);
	}
		break;
	case 60://生成副本GID
		lua_pushnumber(L,pPlayer->m_PartDyRegionGID);
		break;
	case 61://此副本的RID
		lua_pushnumber(L,pPlayer->m_PartDyRegionID);
		break;
	case 62://此场景的ID
		if (pPlayer->m_ParentRegion)
		{
			lua_pushnumber(L, pPlayer->m_ParentRegion->m_wMapID);
		}
		else 
		{
			return 0;
		}
		break;
	case 63://获取此场景的GID
		if (pPlayer->m_ParentRegion)
		{
			lua_pushnumber(L, pPlayer->m_ParentRegion->GetGID());
		}
		else
		{
			return 0;
		}
		break;
	case 101: //VIP等级
		lua_pushnumber(L, pPlayer->m_Property.m_bVipLevel);
		break;
	case 102:	//玩家战斗力
		lua_pushnumber(L, pPlayer->GetPlayerFightPower());
		break;
	default:
		return 0;
	}

	return 1;

	/*
	int wIndex = static_cast<int>(lua_tonumber(L, 1));
	int  wData = 0;
	short lData = 0L;
	DWORD dwData = 0;
	DNID dData = 0;

	LPCSTR str = " ";


	switch(wIndex)
	{
	case 0:
	luaEx_pushint63(L, g_Script.m_pPlayer->m_Property.puid); // 账号唯一ID，但可能导致大跨平台的重复！
	break;
	case 1:
	wData = g_Script.m_pPlayer->m_Property.m_Level;   // 等级
	lua_pushnumber(L, wData);
	break;
	case 2:
	wData = g_Script.m_pPlayer->m_Property.m_School;  // 门派
	lua_pushnumber(L, wData);	
	break;
	case 3:
	str = g_Script.m_pPlayer->m_Property.m_Name ;     //名称
	lua_pushstring(L, str);
	break;
	case 4:
	//wData = g_Script.m_pPlayer->m_Property.m_Exp;    // 经验
	lua_pushnumber(L, (DWORD)g_Script.m_pPlayer->m_Property.m_Exp);
	break;
	case 5:
	wData = g_Script.m_pPlayer->m_Property.m_wLU;      // 气运
	lua_pushnumber(L, wData);
	break;
	case 6:
	wData = g_Script.m_pPlayer->m_Property.m_wST;      // 体魄
	lua_pushnumber(L, wData);
	break;
	case 7:
	wData = g_Script.m_pPlayer->m_wAgile;     // 身法
	lua_pushnumber(L, wData);
	break;
	case 8:
	wData = g_Script.m_pPlayer->m_Property.m_wEN ;     // 活力
	lua_pushnumber(L, wData);
	break;		
	case 9:
	wData = g_Script.m_pPlayer->m_Property.m_wIN;     // 悟性
	lua_pushnumber(L, wData);
	break;	
	case 10:
	lData = g_Script.m_pPlayer->m_Property.m_byPKValue; // 杀孽
	lua_pushnumber(L, -lData);
	break;
	case 11:
	wData = g_Script.m_pPlayer->m_Property.m_Sex;     // 性别
	lua_pushnumber(L, wData);
	break;
	case 12:
	wData = g_Script.m_pPlayer->m_dwTeamID;             // 组队编号
	lua_pushnumber(L, wData);
	break;
	case 13:
	wData = (int)g_Script.m_pPlayer->IsTeamLeader();    // 是否队长
	lua_pushnumber(L, wData);
	break;
	case 14:
	lData = g_Script.m_pPlayer->m_Property.m_sXValue;  // 名望
	lua_pushnumber(L, lData);
	break;
	case 15:
	str = g_Script.m_pPlayer->GetAccount();             //账号
	lua_pushstring(L, str);
	break;
	case 16:                                                //全局编号
	lua_pushnumber(L, g_Script.m_pPlayer->GetGID());
	break;
	case 17:                                                //全局编号
	lua_pushnumber(L, g_Script.m_pPlayer->m_Property.m_dwStaticID);
	break;
	case 18:											   //消费点数
	lua_pushnumber(L, g_Script.m_pPlayer->m_Property.m_dwConsumePoint /100);
	break;
	case 19:											   //当前的IP-String
	extern LPCSTR GetIPString( DNID dnid, WORD *port = NULL );
	{
	LPCSTR ip = GetIPString( g_Script.m_pPlayer->m_ClientIndex );
	if ( ip == NULL )
	return 0;
	lua_pushstring( L, ip );
	}
	break;
	case 20:											   //是否存在2级密码？
	if ( g_Script.m_pPlayer->m_Property.m_UPassword[0] == 0 )
	return 0;

	g_Script.m_pPlayer->m_Property.m_UPassword[CONST_USERPASS-1] = 0;
	lua_pushstring( L, g_Script.m_pPlayer->m_Property.m_UPassword );
	break;
	case 21:
	str = g_Script.m_pPlayer->GetNotPrefixAccount();             //账号不带前綴
	lua_pushstring(L, str);
	break;
	case 22:
	wData = g_Script.m_pPlayer->m_Property.m_byVenapointCount; 
	lua_pushnumber(L, wData);
	break;
	case 23:
	lua_pushnumber( L, g_Script.m_pPlayer->m_Property.factionId );
	break;
	case 24:
	lua_pushnumber( L, g_Script.m_pPlayer->m_Property.m_ExtraTelergy.charge );
	lua_pushnumber( L, g_Script.m_pPlayer->m_Property.m_ExtraTelergy.state[0] );
	lua_pushnumber( L, g_Script.m_pPlayer->m_Property.m_ExtraTelergy.state[1] );
	lua_pushnumber( L, g_Script.m_pPlayer->m_Property.m_ExtraTelergy.state[2] );
	lua_pushnumber( L, g_Script.m_pPlayer->m_Property.m_ExtraTelergy.state[3] );
	return 5;
	break;
	case 25:													//生命
	lua_pushnumber( L, g_Script.m_pPlayer->m_CurHp );
	break;
	case 26:													//生命上限
	lua_pushnumber( L, g_Script.m_pPlayer->GetMaxHP() );
	break;
	case 27:													// 积分
	lua_pushnumber( L, g_Script.m_pPlayer->m_Property.m_dwAmassCent );
	break;
	case 28:
	lua_pushnumber( L, g_Script.m_pPlayer->m_tempItem.size() );
	break;
	case 29:													// 武功境界
	lua_pushnumber( L, g_Script.m_pPlayer->m_Property.bySkillState );
	break;
	case 30: // 获取平台前缀，这里需要判断是取1级前缀还是2级前缀
	lua_pushstring( L, g_Script.m_pPlayer->GetAccountPrefix( ( int )lua_tonumber( L, 2 ) ) );
	break;
	case 31:													//GM等级
	dwData = g_Script.m_pPlayer->m_Property.m_GMLevel;
	lua_pushnumber( L, dwData);
	break;
	case 32:				//Mac地址
	dData = g_Script.m_pPlayer->GetMac();
	luaEx_pushint63( L, dData);
	break;
	case 33:    // 心魔
	lua_pushnumber( L, g_Script.m_pPlayer->m_Property.wXinMoVaule );
	break;
	case 34:    // 结义名
	lua_pushstring(L, (GetGW()->m_UnionManager.GetUnionNameByPlayerID(g_Script.m_pPlayer->GetSID())).c_str());
	break;
	case 35: // added by xhy 组队人数
	wData = g_Script.m_pPlayer->m_wTeamMemberCount;
	lua_pushnumber(L, wData);
	break;
	case 36: // 武功层数
	if(lua_gettop(L)==2 && lua_isnumber(L, 2) ) {
	wData = g_Script.m_pPlayer->GetSkillLevel( (WORD)lua_tonumber(L, 2) -1);
	lua_pushnumber(L, wData);
	} else {
	lua_pushnumber(L, 0);
	}
	break;
	case 37:													// 心法等级
	if(lua_gettop(L)==2 && lua_isnumber(L, 2) ) {
	wData = g_Script.m_pPlayer->GetTelergyLevel((WORD)lua_tonumber(L, 2));
	lua_pushnumber(L, wData);
	} else {
	lua_pushnumber(L, 0);
	}
	break;
	case 38: //BUFF
	if ( 2==lua_gettop(L) && lua_isstring(L,2) )
	{
	string sBuffName = lua_tostring(L, 2);
	if( g_Script.m_pPlayer->buffContainer.GetBuffByName(sBuffName) )
	lua_pushboolean(L, 1);
	else
	lua_pushboolean(L, 0);
	} else
	return 0;
	break;
	case 39: //MP
	lua_pushnumber( L, g_Script.m_pPlayer->m_wMP );
	break;
	case 40: //SP
	lua_pushnumber( L, g_Script.m_pPlayer->m_wSP );
	break;
	case 41:			//MP上限 内力上限
	lua_pushnumber( L, g_Script.m_pPlayer->m_wMaxMP );
	break;
	case 42:			//SP上限 真气上限
	lua_pushnumber( L, g_Script.m_pPlayer->m_wMaxSP );
	break;
	case 43:			//外功基础
	lua_pushnumber( L, g_Script.m_pPlayer->m_ADBaseValueMax[SPlayerXiaYiShiJie::OUT_ATTACK_INDEX] );
	break;
	case 44:			//内功基础
	lua_pushnumber( L, g_Script.m_pPlayer->m_ADBaseValueMax[SPlayerXiaYiShiJie::INNER_ATTACK_INDEX] );
	break;
	case 45:			//外防基础
	lua_pushnumber( L, g_Script.m_pPlayer->m_ADBaseValueMax[SPlayerXiaYiShiJie::OUT_DEFENSE_INDEX] );
	break;
	case 46:			//内防基础
	lua_pushnumber( L, g_Script.m_pPlayer->m_ADBaseValueMax[SPlayerXiaYiShiJie::INNER_DEFENSE_INDEX]  );
	break;
	case 47:			//杀气
	lua_pushnumber( L, g_Script.m_pPlayer->GetAmuck());
	break;
	case 48:			//Money
	lua_pushnumber(L, g_Script.m_pPlayer->m_Property.m_Money);
	break;
	default:
	return 0;
	}//*/

	return 1;
}

//add by ly 2014/4/9
int CScriptManager::L_SetPlayerProperty(lua_State *L)	//设置玩家相关数据
{
	DWORD dwGID = static_cast<DWORD>(lua_tonumber(L, 1));
	CPlayer* pPlayer;
	if (dwGID == 0)
	{
		pPlayer = g_Script.m_pPlayer;
	}
	else
	{
		pPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer) return 0;

	int index = static_cast<int>(lua_tonumber(L, 2));
	switch (index)
	{			
	case 1:		//增加体力
		pPlayer->m_CurTp += lua_tonumber(L, 3);
		pPlayer->m_Property.m_CurTp += lua_tonumber(L, 3); 
		if (pPlayer->m_Property.m_CurTp > pPlayer->m_MaxTp)
			pPlayer->m_Property.m_CurTp = pPlayer->m_MaxTp;
		if (pPlayer->m_CurTp > pPlayer->m_MaxTp)
			pPlayer->m_CurTp = pPlayer->m_MaxTp;
		pPlayer->m_FightPropertyStatus[XA_CUR_TP] = true;
		break;
	case 2:		//增加生命
		pPlayer->m_CurHp += lua_tonumber(L, 3);
		pPlayer->m_Property.m_CurHp += lua_tonumber(L, 3);
		if (pPlayer->m_Property.m_CurHp > pPlayer->m_MaxHp)
			pPlayer->m_Property.m_CurHp = pPlayer->m_MaxHp;
		if (pPlayer->m_CurHp > pPlayer->m_MaxHp)
			pPlayer->m_CurHp = pPlayer->m_MaxHp;
		pPlayer->m_FightPropertyStatus[XA_CUR_HP] = true;
		break;
	case 3:	// 当前真气
		pPlayer->m_Property.m_CurSp += lua_tonumber(L, 3);
		pPlayer->m_PlayerPropertyStatus[XA_CUR_SP - XA_MAX_EXP] = true;
		break;
	case 4:		//增加内力
		pPlayer->m_CurMp += lua_tonumber(L, 3);
		pPlayer->m_Property.m_CurMp += lua_tonumber(L, 3);
		if (pPlayer->m_Property.m_CurMp > pPlayer->m_MaxMp)
			pPlayer->m_Property.m_CurMp = pPlayer->m_MaxMp;
		if (pPlayer->m_CurMp > pPlayer->m_MaxMp)
			pPlayer->m_CurMp = pPlayer->m_MaxMp;
		pPlayer->m_FightPropertyStatus[XA_CUR_MP] = true;
		break;
	case 5:	// 当前精力
		pPlayer->m_Property.m_CurJp += lua_tonumber(L, 3);
		pPlayer->m_PlayerPropertyStatus[XA_CUR_JP - XA_MAX_EXP] = true;
		break;
	case 6:	// 进攻
		pPlayer->m_Property.m_JingGong += lua_tonumber(L, 3);
		if (pPlayer->m_Property.m_FightPetActived == 0xff || pPlayer->m_ParentRegion->DynamicCast(IID_DYNAMICREGION) == NULL)	//当玩家不在变身状态下
		{
			pPlayer->UpdateBaseProperties();	//更新基础攻击
			pPlayer->UpdateAllProperties();
		}
		break;
	case 7:	// 防御
		pPlayer->m_Property.m_FangYu += lua_tonumber(L, 3);
		if (pPlayer->m_Property.m_FightPetActived == 0xff || pPlayer->m_ParentRegion->DynamicCast(IID_DYNAMICREGION) == NULL)	//当玩家不在变身状态下
		{
			pPlayer->UpdateBaseProperties();	//更新基础最大体力和基础防御
			pPlayer->UpdateAllProperties();
		}
		break;
	case 8:	// 轻身
		pPlayer->m_Property.m_QingShen += lua_tonumber(L, 3);
		pPlayer->UpdateBaseProperties();	//更新基础暴击和基础躲避
		pPlayer->UpdateAllProperties();
		break;
	case 9:	// 健身
		pPlayer->m_Property.m_JianShen += lua_tonumber(L, 3);
		if (pPlayer->m_Property.m_FightPetActived == 0xff || pPlayer->m_ParentRegion->DynamicCast(IID_DYNAMICREGION) == NULL)	//当玩家不在变身状态下
		{
			pPlayer->UpdateBaseProperties();	//更新基础最大体力和基础防御
			pPlayer->UpdateAllProperties();
		}
		break;
	case 10:	// 玩家当前经验
		pPlayer->m_Property.m_Exp += lua_tonumber(L, 3);
		pPlayer->m_PlayerPropertyStatus[XA_CUR_EXP - XA_MAX_EXP] = true;
		break;
	case 11:	// 玩家剩余点数
		pPlayer->m_Property.m_RemainPoint += lua_tonumber(L, 3);
		pPlayer->m_PlayerPropertyStatus[XA_REMAINPOINT - XA_MAX_EXP] = true;
		break;
	case 12:	//设置玩家VIP等级
	{
					BYTE bLevel = static_cast<BYTE>(lua_tonumber(L, 3));			//等级
					g_Script.m_pPlayer->_L_SetPlayerVipLevel(bLevel);
	}
		break;
	case 13:	//设置玩家等级
	{
					BYTE bLevel = static_cast<BYTE>(lua_tonumber(L, 3));			//等级
					g_Script.m_pPlayer->OnSetPlayerLevel(bLevel);
	}
		break;
	case 14:	
		break;
	case 15:	
		break;
	case 16:	//金币
	{
					long value = static_cast<long>(lua_tonumber(L, 3));
					g_Script.m_pPlayer->ChangeMoney(1, value);
	}
		break;
	case 17:	//游戏币
	{
					long value = static_cast<long>(lua_tonumber(L, 3));
					g_Script.m_pPlayer->ChangeMoney(2, value);
	}
		break;
	case 18:	// 赠宝（不可交易）XYD3--	   //礼券
		pPlayer->m_Property.m_ZengBao += lua_tonumber(L, 3);
		pPlayer->m_PlayerPropertyStatus[XA_ZENGBAO - XA_MAX_EXP] = true;
		break;
	case 19:	// 元宝（现实货币，可交易，收税）
		pPlayer->m_Property.m_YuanBao += lua_tonumber(L, 3);
		pPlayer->m_PlayerPropertyStatus[XA_YUANBAO - XA_MAX_EXP] = true;
		break;
	default:
		break;
	}
	return 0;
}

int CScriptManager::L_SetPlayerObject(lua_State *L)		//设置执行脚本函数的玩家对象（参数为：玩家全局唯一GID，为0时表示当前玩家)
{
	DWORD dwGID = static_cast<DWORD>(lua_tonumber(L, 1));
	CPlayer* pPlayer;
	if (dwGID != 0)
	{
		g_Script.m_pPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
	}
	lua_pushnumber(L, 1);	//表示执行成功
	return 1;
}
int CScriptManager::L_getmonsterdata(lua_State *L)
{

	if (g_Script.m_pMonster == NULL)
		return 0;

	WORD wIndex = static_cast<WORD>(lua_tonumber(L, 1));
	int  wData = 0;
	short lData = 0L;

	LPCSTR str = " ";
	switch(wIndex)
	{
	case 1:
		wData = g_Script.m_pMonster->m_Property.m_level;   // 等级
		lua_pushnumber(L, wData);
		break;
	case 2:
		wData = g_Script.m_pMonster->m_Property.m_ID;      // 编号
		lua_pushnumber(L, wData);	
		break;
	case 3:
		str = g_Script.m_pMonster->m_Property.m_Name;      //名称
		lua_pushstring(L, str);
		break;
	case 4:
		wData = (int)g_Script.m_pMonster->m_Property.m_Exp;   // 经验
		lua_pushnumber(L, wData);
		break;
	case 5:
		wData = g_Script.m_pMonster->m_Property.m_LevelType; // 怪物BOSS类型
		lua_pushnumber(L, wData);
		break;
	case 6:
		lua_pushnumber( L, g_Script.m_pMonster->m_CurHp ); // 当前血量
		break;
	case 7:
		lua_pushnumber( L, g_Script.m_pMonster->m_Property.m_MaxHP ); // 最大血量
		break;
	case 8:
		lua_pushnumber( L, g_Script.m_pMonster->m_Property.m_AttType);	//类型ID
		break;
	case 9:
		lua_pushnumber(L, g_Script.m_pMonster->m_dwLastEnemyID);//谁攻击了我
		break;
	case 10:
		lua_pushnumber(L,g_Script.m_pMonster->m_AIParameter.wGroupID);//怪物的组别的ID
		break;
	case 11:
		lua_pushnumber(L,g_Script.m_pMonster->m_Property.m_PartolRadius);//监控半径
		break;
	case 12:
		lua_pushnumber(L,g_Script.m_pMonster->m_Property.m_TailLength);//追击距离
		break;
	case 13:
		lua_pushnumber(L,g_Script.m_pMonster->m_NodeCount);//路点总个数
		break;
	case 14:
		lua_pushnumber(L,g_Script.m_pMonster->m_CurNode);//当前路点个数
		break;
	case 15:
		lua_pushnumber(L,g_Script.m_pMonster->m_Property.m_MoveType);
		break;
	case 16:
		lua_pushnumber(L,g_Script.m_pMonster->m_IsMove2Attack);//移动类型
		break;
	case 18:
		{
			float x = 0;
			if (g_Script.m_pMonster->m_Property.m_MaxHP == 0)
			{
				x = 1000;
			} 
			else
			{
				x = g_Script.m_pMonster->m_Property.m_MaxHP;
			}
			float value = g_Script.m_pMonster->m_CurHp /  x;
			lua_pushnumber(L,value);//当前生命百分比
		}
		break;
	case 19:
		{
			lua_pushnumber(L,g_Script.m_pMonster->m_BackProtection);
		}
		break;
	case 20:
	{
		lua_pushnumber(L, g_Script.m_pMonster->m_nBirthTime);
	}
	break;
	case 21:  //坐标
	{
		lua_createtable(L, 0, 0);
		lua_pushnumber(L, g_Script.m_pMonster->m_curX);
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, g_Script.m_pMonster->m_curY);
		lua_rawseti(L, -2, 2);
	}
	break;
	case 22: //ai id
	{
		lua_pushnumber(L, g_Script.m_pMonster->m_Property.m_StaticAI);
	}
		break;
	case 23:  //坐标，用于怪物出生时的坐标
	{
				  lua_createtable(L, 0, 0);
				  lua_pushnumber(L, g_Script.m_pMonster->m_Property.m_BirthPosX);
				  lua_rawseti(L, -2, 1);
				  lua_pushnumber(L, g_Script.m_pMonster->m_Property.m_BirthPosY);
				  lua_rawseti(L, -2, 2);
	}
		break;
	default:
		return 0;
	}
	return 1;
}

// ----- the task need number subtract the had finished -----------
int CScriptManager::L_subtasknum(lua_State *L)
{
	//     if (g_Script.m_pPlayer == NULL)
	//         return 0;
	// 
	//     WORD wTaskID = static_cast<WORD>(lua_tonumber(L, 1));
	// 
	//     SRoleTask *task = g_Script.m_pPlayer->FindTask(wTaskID);
	//     if(task == NULL) 
	//         return 0;
	// 
	//     WORD wHave = task->byNeedNum;
	//     WORD wFinish = task->byFinished;
	//     //if(wFinish > wHave) 
	//     //{ 
	//     //    task->bComplete = true;
	//     ///*    return 0;*/
	//     //}
	//     int loopnum = wHave - wFinish;
	//     lua_pushnumber(L, loopnum);
	// 
	//     return 1;
	return 0;
}

// ----- get the npc's data ---------------------------------------
int CScriptManager::L_getnpcdata(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	if(g_Script.m_pNpc == NULL) 
		return 0;
	// 
	WORD wIndex = static_cast<WORD>(lua_tonumber(L, 1));
	WORD wData = 0;
	switch(wIndex)
	{
	case 1:
		wData = g_Script.m_pNpc->m_Property.m_wClickScriptID;   // 可能是任务号 现在是靠点击号来判定
		break;
	case 2:
		wData = g_Script.m_pNpc->m_Property.m_wSchool;
		break;

	case 3:
		{
			lua_pushnumber(L, g_Script.m_pNpc->GetGID());
			return 1;

		}break;
	}
	lua_pushnumber(L, wData);

	return 1;
}

// ----- get the task's data --------------------------------------
int CScriptManager::L_gettaskdata(lua_State *L)
{	
	return 0;
}

// ----- get goods's data ------------------------------------------
int CScriptManager::L_getgoodsdata(lua_State *L)
{
	return 0;
}

// ----- get the equipment'data to forge ---------------------------
int CScriptManager::L_getforgedata(lua_State *L)
{
	return 5;
}

struct ExtraLuaFunctions
{
	static CRegion* GetRegionById( DWORD regionId );

	static int override_setplayerdata( lua_State *L )
	{
		return 0;
	}
};

// ----- set goods's data ------------------------------------------
int CScriptManager::L_setplayerdata(lua_State *L)
{  
	if ( g_Script.m_pPlayer == NULL )
		return 0;
	short type = static_cast<short>(lua_tonumber(L,1));
	switch (type)
	{
	case 1:
		{
			short Index = static_cast<short>(lua_tonumber(L,2));
			long value = static_cast<long>(lua_tonumber(L,3));
			g_Script.m_pPlayer->m_TemplateMap.insert(std::make_pair(Index,value));
		}
		break;
	case 2:
		{
			short Index = static_cast<short>(lua_tonumber(L,2));
			long value = static_cast<long>(lua_tonumber(L,3));
			if (Index == 1)
			{
				g_Script.m_pPlayer->m_CountTA += value;
			}
		}
	case 3:
		{
			short Index = static_cast<short>(lua_tonumber(L,2));
			long value = static_cast<long>(lua_tonumber(L,3));
			g_Script.m_pPlayer->m_PartDyRegionID = Index;
			g_Script.m_pPlayer->m_PartDyRegionGID = value;
		}
		break;
	}

	return 1;
}

int CScriptManager::L_setplayericon(lua_State *L)
{  
	//     if (g_Script.m_pPlayer == NULL)
	//         return 0;
	// 
	//     int iIcon = static_cast<int>(lua_tonumber(L, 1));
	//     g_Script.m_pPlayer->m_Property.m_dwSysTitle[1] = iIcon;
	//     g_Script.m_pPlayer->SendMyState();

	return 0;
}

// ----- transform the number to integer ---------------------------------
int CScriptManager::L_rint(lua_State *L)
{
	int num = static_cast<int>(lua_tonumber(L, 1));
	lua_pushnumber(L, num);

	return 1;
}

int CScriptManager::L_putplayerfromdynamic(lua_State *L)
{
	// 获取玩家ID，新场景ID及坐标
	bool	back		= static_cast<bool >(lua_toboolean(L, 1));
	DWORD	curRegID	= static_cast<DWORD>(lua_tonumber (L, 2));
	DWORD	newReID		= static_cast<DWORD>(lua_tonumber (L, 3));
	WORD	toX			= static_cast<WORD >(lua_tonumber (L, 4));
	WORD	toY			= static_cast<WORD >(lua_tonumber (L, 5));

	extern LPIObject FindRegionByGID(DWORD GID);
	LPIObject temp = FindRegionByGID(curRegID);

	if (CDynamicRegion *dynamicRegion = (CDynamicRegion *)temp->DynamicCast(IID_DYNAMICREGION))
	{
		DWORD playerID = dynamicRegion->GetPlayerID();
		if (CPlayer *pPlayer = g_Script.m_pPlayer/*(CPlayer *)GetPlayerBySID(playerID)->DynamicCast(IID_PLAYER)*/)
		{
			if (back)
				PutPlayerIntoDestRegion(pPlayer, dynamicRegion->m_prevRegID, dynamicRegion->m_prevX, dynamicRegion->m_prevY, 0);
			else
				PutPlayerIntoDestRegion(pPlayer, newReID, toX, toY, 0);
		}
		return 1;
	}

	return 0;
}

int CScriptManager::L_putplayerto(lua_State *L)
{
	WORD  wRegion		= static_cast<WORD >(lua_tonumber(L, 1));
	WORD  wToX			= static_cast<WORD >(lua_tonumber(L, 2));
	WORD  wToY			= static_cast<WORD >(lua_tonumber(L, 3));
	DWORD dwRegionGID	= static_cast<DWORD>(lua_tonumber(L, 4));		// 扩展事件地图GID
	DWORD dwPlayerGID	= static_cast<DWORD>(lua_tonumber(L, 5));		// 玩家的GID,如果这个值为0，那么使用g_Script.m_pPlayer

	CPlayer *pPlayer = 0;
	if(dwPlayerGID)		
		pPlayer = (CPlayer*)GetPlayerByGID(dwPlayerGID)->DynamicCast(IID_PLAYER);
	if(!pPlayer)		
		pPlayer = g_Script.m_pPlayer;
	if(!pPlayer)		return 0;

	if (dwRegionGID)
	{
		extern LPIObject FindRegionByGID(DWORD GID);

		if (CDynamicRegion *dynamicRegion = (CDynamicRegion *)FindRegionByGID(dwRegionGID)->DynamicCast(IID_DYNAMICREGION))
		{
			// 获得当前玩家所在的场景ID和坐标并加以保存
			if (pPlayer->m_DynamicRegionState == 1)
			{
				if (pPlayer->m_dwTeamID != 0 && wRegion == 0)//不是队长但是是队员 并且第二个参数为零
				{
					PutPlayerIntoDestRegion(pPlayer, wRegion, wToX, wToY, dwRegionGID);//进副本才用这个
				} 
				else
				{
					PutPlayerIntoDestRegion(pPlayer, pPlayer->m_PartprevRegID, pPlayer->m_PartprevX, pPlayer->m_PartprevY, 0);//出副本就不用
				}
				return 0;
			}
		}
	}

	PutPlayerIntoDestRegion(pPlayer, wRegion, wToX, wToY, dwRegionGID);

	return 0;
}

int CScriptManager::L_putplayerback(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	if (g_Script.m_pPlayer->m_ParentRegion == NULL)
		return 0;

	// 容错，避免玩家不能回到有效的场景！（主要是解决在奇遇部分常常出不去！）
	if ( g_Script.m_pPlayer->m_ParentRegion->m_wReLiveRegionID == 0 ||
		g_Script.m_pPlayer->m_ParentRegion->m_wReLiveRegionID > 1000 )
	{
		g_Script.m_pPlayer->m_ParentRegion->m_wReLiveRegionID = 1;
		g_Script.m_pPlayer->m_ParentRegion->m_ptReLivePoint.x = 200; 
		g_Script.m_pPlayer->m_ParentRegion->m_ptReLivePoint.y = 200; 
	}

	PutPlayerIntoDestRegion(g_Script.m_pPlayer, 
		g_Script.m_pPlayer->m_ParentRegion->m_wReLiveRegionID, 
		(WORD)g_Script.m_pPlayer->m_ParentRegion->m_ptReLivePoint.x, 
		(WORD)g_Script.m_pPlayer->m_ParentRegion->m_ptReLivePoint.y, 
		0);

	return 0;
}

int CScriptManager::L_isFullNum(lua_State *L)
{
	/*	lua_pushnumber( L, g_Script.m_pPlayer->CheckAddItem() );*/
	return 1;
}

int CScriptManager::L_isfull(lua_State *L)
{
	/*	lua_pushnumber( L, g_Script.m_pPlayer->CheckAddItem() > 0 );*/
	return 1;
}

int CScriptManager::L_fillcopyitem(lua_State *L)
{
	return 0;
}

int CScriptManager::L_settipmsg(lua_State *L)
{
	const char* str = lua_tostring(L, 1);
	if(str && g_Script.m_pPlayer)
	{
		//			g_StoreMessage( pPlayer->m_ClientIndex, &msg, sizeof(SABackMsg) );
		SAChatTipsMsg msg;
		msg.wShowType = SAChatTipsMsg::TIPS_SHOWTYPE_SYSTEM ;
		dwt::strcpy(msg.cChatData, str, sizeof(msg.cChatData));
		g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, msg.GetMySize());
		return 1;
	}

	return 0;
}

int CScriptManager::L_teachgest(lua_State *L)
{
	return 0;
}

int CScriptManager::L_teachteamgest(lua_State *L)
{
	return 0;
}

int CScriptManager::L_setgestlevel(lua_State *L)
{
	return 0;
}

int CScriptManager::L_requarefactionname(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

//	SARequareFactionNameMsg msg;
	//msg.dwCheckID = g_Script.m_pPlayer->GetGID();

	//g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(msg));

	return 0;
}

int CScriptManager::L_createfaction(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	const char *szFactionName = static_cast<const char*>(lua_tostring(L, 1));
	if (szFactionName == NULL ) 
		return 0;

	DWORD factionId = static_cast< DWORD >( lua_tonumber( L, 2 ) );

	/*if( !GetGW()->m_FactionManager.CreateFaction( szFactionName, g_Script.m_pPlayer, factionId ) )
		return 0;*/

	// 广播数据
	g_Script.m_pPlayer->SendMyState();

	lua_pushnumber(L, 1);

	return 1;
}

int CScriptManager::L_leavefaction( lua_State *L )
{
	//     if ( lua_isstring( L, 1 ) )
	//     {
	//         // 这个是属于脚本主动删除帮派!
	//         return 0;
	//     }
	//     else
	//     {
	//         if ( g_Script.m_pPlayer == NULL )
	//             return 0;
	// 
	//         dwt::stringkey<char [CONST_USERNAME]> FactionName = g_Script.m_pPlayer->m_Property.m_szTongName;
	//         if (g_Script.m_pPlayer->m_Property.m_szTongName[0] == 0) 
	// 	    {
	// 		    TalkToDnid( g_Script.m_pPlayer->m_ClientIndex, "你好像没有加入过帮派吧。。。" );
	//             return 0;
	// 	    }
	// 
	// 	    if ( g_Script.m_pPlayer->m_stFacRight.Title != 8 )
	// 	    {
	// 		    if ( !GetGW()->m_FactionManager.DelMember( FactionName, g_Script.m_pPlayer->GetName(), TRUE ) )
	// 			    return 0;
	// 
	// 		    //g_Script.m_pPlayer->m_Property.m_szTongName[0] = 0;
	// 		    //g_Script.m_pPlayer->SendMyState();
	// 
	// 		TalkToDnid( g_Script.m_pPlayer->m_ClientIndex, "你已经正式离开了之前所在的帮派！" );
	// 	}
	// 	else
	//     {
	//         if( !GetGW()->m_FactionManager.DeleteFaction( FactionName ) )
	//             return 0;
	//         // 广播数据
	// 		g_Script.m_pPlayer->m_Property.m_dwSysTitle[1] = 0;
	//         g_Script.m_pPlayer->SendMyState();
	// 
	//             TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "阁下已经把你的帮派解散了！");
	//         }
	// 
	//         lua_pushnumber(L, 1);
	// 
	//         return 1;
	//     }
	return 0;
}

int CScriptManager::L_getgestlevel(lua_State *L)
{
	return 0;
}

int CScriptManager::L_paytheplayer(lua_State *L)
{
	return 0;
}

int CScriptManager::L_settaskstring(lua_State *L)
{
	//     if (g_Script.m_pPlayer == NULL)
	//         return 0;
	// 
	//     WORD wTaskID = static_cast<WORD>(lua_tonumber(L, 1));
	//     LPCSTR  szStr = static_cast<const char*>(lua_tostring(L, 2));
	// 
	//     SRoleTask *task = g_Script.m_pPlayer->FindTask(wTaskID);   
	//     if(task == NULL) 
	//         return 0;
	// 
	//     dwt::strcpy(task->m_szName, szStr, CONST_USERNAME);

	return 0;
}

int CScriptManager::L_getcurtelergy(lua_State *L)
{
	return 0;
}

int CScriptManager::L_setcurtelergyval(lua_State *L)
{

	return 0;
}

int CScriptManager::L_checkmoreitem(lua_State *L)
{
	return 0;
}

int CScriptManager::L_gettelergy(lua_State *L)
{
	return 0;
}

int CScriptManager::L_gettelergydata(lua_State *L)
{
	return 0;
}

int CScriptManager::L_getitemname(lua_State *L)
{
	DWORD wIndex = static_cast<DWORD>(lua_tonumber(L, 1));

	const SItemBaseData* pData=CItemService::GetInstance().GetItemBaseData(wIndex);
	if(!pData)return 0;

	lua_pushstring(L ,pData->m_Name);	 
	return 1;

}
int CScriptManager::L_getMountFoodAddExp(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	WORD itemid = static_cast<WORD>(lua_tonumber(L, 1));
	int v = INSTANCE(Mounts::CMountTemplates)->GetInExp(itemid);
	lua_pushnumber(L ,v);
	return 1;
}

int CScriptManager::L_QueryEncouragement(lua_State *L)
{
	return 0;
}


int CScriptManager::L_gettaskitemname(lua_State *L)
{

	return 0;
}

int CScriptManager::L_lostitems(lua_State *L)
{
	return 0;
}

int CScriptManager::L_getscores(lua_State *L)
{
	return 0;
}

int CScriptManager::L_setregionpktype(lua_State *L)
{
	DWORD dwRegionID = static_cast<DWORD>(lua_tonumber(L, 1));
	DWORD dwPKType = static_cast<DWORD>(lua_tonumber(L, 2));

	CRegion *pRegion = ExtraLuaFunctions::GetRegionById( dwRegionID );
	//CRegion *pRegion = (CRegion *)FindRegionByID(wRegionID)->DynamicCast(IID_REGION);
	if ( pRegion == NULL ) 
		return 0;

	pRegion->m_dwPKAvailable = dwPKType;

	return 0;
}

int CScriptManager::L_broadcast(lua_State *L)
{

	LPCSTR szStr = static_cast<const char*>(lua_tostring(L, 1));

	BroadCastAnnounce(szStr, 256);

	return 0;
}

int GetCurTime(TimeType type)
{
	time_t	Seconds;											
	struct tm	*Date = NULL;

	Seconds = time(NULL);															
	Date = localtime(&Seconds);	

	int nData = 0;

	switch(type) {
	case GTT_YEAR:
		nData = Date->tm_year + 1900;
		break;
	case GTT_MONTH:
		nData = Date->tm_mon + 1;
		break;
	case GTT_MDAY:
		nData = Date->tm_mday;
		break;
	case GTT_WDAY:
		nData = Date->tm_wday;
		break;
	case GTT_HOURS:
		nData = Date->tm_hour;
		break;
	case GTT_MINUTES:
		nData = Date->tm_min;
		break;
	case GTT_SECONDS:
		nData = Date->tm_sec;
		break;
	case GTT_YDAY:
		nData = Date->tm_yday;
		break;

	default:
		return 0;
	}

	return nData;

}

int CScriptManager::L_getcurtimeinmin(lua_State *L)
{
	BOOL useSecond = static_cast< BOOL >( lua_tonumber( L, 1 ) );

	if ( useSecond )
		lua_pushnumber( L, ( DWORD )time( NULL ) );
	else
		lua_pushnumber( L, ( DWORD )time( NULL ) / 60 );

	return 1;
}

int CScriptManager::L_getcurtime(lua_State *L)
{
	int nData = 0;

	WORD wWhat = static_cast<WORD>(lua_tonumber(L, 1));
	nData = GetCurTime((TimeType)wWhat);

	lua_pushnumber(L, nData);

	return 1;
}

int CScriptManager::L_getcurtimeA(lua_State *L)
{
	LPCSTR string = GetStringTime();
	lua_pushstring(L, string);

	return 1;
}

int CScriptManager::L_talktoplayer(lua_State *L)
{
	if ( lua_type( L, 1 ) == LUA_TSTRING )
	{
		extern void TalkToPlayer( LPCSTR, LPCSTR );
		LPCSTR lpszName = static_cast< LPCSTR >( lua_tostring( L, 1 ) );
		LPCSTR lpszInfo = static_cast< LPCSTR >( lua_tostring( L, 2 ) );

		TalkToPlayer( lpszName, lpszInfo );
	}
	else
	{
		DWORD dwType = static_cast<DWORD>( lua_tonumber( L, 1 ) );
		LPCSTR lpszInfo = static_cast< LPCSTR >( lua_tostring( L, 2 ) );

		int size = 0;
		SPackageItem *data = ( SPackageItem* )luaEx_touserdata( L, 3, &size );
		if ( data && size != sizeof( SPackageItem ) )
			data = NULL;

		void TalkToAll( LPCSTR info, SRawItemBuffer *item, LPCSTR name = NULL );

		if (0 == dwType)
			TalkToAll( lpszInfo, data );
		else if (1 == dwType && g_Script.m_pAttacker)
		{
			CPlayer *player = (CPlayer *)g_Script.m_pAttacker->DynamicCast(IID_PLAYER);
			if (player)
				TalkToDnid(player->m_ClientIndex, lpszInfo);
		}
		else if (2 == dwType && g_Script.m_pPlayer)
			TalkToDnid( g_Script.m_pPlayer->m_ClientIndex, lpszInfo );
	}

	return 0;
}

int  CScriptManager::L_delitembypos(lua_State *L)
{
	//     if (g_Script.m_pPlayer == NULL)
	//         return 0;
	// 
	//     WORD wType = static_cast<WORD>(lua_tonumber(L, 1));
	//     WORD wX = static_cast<WORD>(lua_tonumber(L, 2));
	//     WORD wY = static_cast<WORD>(lua_tonumber(L, 3));
	// 
	//     SPackageItem *pItem = NULL;
	//     SRoleTask *pTask = NULL;
	// 
	//     switch(wType) {
	//         case 1: 
	//             {
	//                 pTask = g_Script.m_pPlayer->FindTask(4);
	//                 if(pTask == NULL) {
	//                     lua_pushnumber(L, 0);
	//                     return 1;
	//             }
	//             wX = pTask->wNextGoods;
	//             wY = pTask->byFinished;
	// 
	//             }break;
	// 
	//         case 2: 
	//             {
	//                 pTask = g_Script.m_pPlayer->FindTask(5);
	//                 if(pTask == NULL) {
	//                     lua_pushnumber(L, 0);
	//                     return 1;
	//             }
	// 
	//             wX = pTask->wForgeGoods[0];
	//             wY = pTask->wForgeGoods[1];
	// 
	//             }break;
	// 
	//         default: 
	//             break;
	//                  
	//     }
	// 
	//     pItem = g_Script.m_pPlayer->FindItemByPos( (BYTE)wY, (BYTE)wY );
	//     if(pItem == NULL) {
	//         lua_pushnumber(L, 0);
	//         return 1;
	//     }
	// 
	//     if ( g_Script.m_pPlayer->DelItem( *pItem, GetNpcScriptInfo( "脚本删除DelItemByPos" ) ) )
	//         lua_pushnumber(L, 1);
	//     else
	//         lua_pushnumber(L, 0);
	// 
	//     return 1;
	return 0;
}

int CScriptManager::L_GetKillInfo(lua_State *L)
{
	//extern CKillInfoManager g_KillInfoManager;
	//SASendKillInfoMsg msg;
	//ZeroMemory(msg.stKillInfo, sizeof(KILLINFO)*MAX_KILLINFO);
	//g_KillInfoManager.GetKillListAllInfo(&msg);
	//  
	//g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(SASendKillInfoMsg));

	return 0;
}

int CScriptManager::L_setplayerscriptstate(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	WORD wState = static_cast<WORD>(lua_tonumber(L, 1));
	WORD wIcon = static_cast<WORD>(lua_tonumber(L, 2));

	g_Script.m_pPlayer->SetScriptState(wState, wIcon);
	g_Script.m_pPlayer->SendExtraState();

	return 0;
}

int CScriptManager::L_getplayerscriptstate(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	lua_pushnumber(L, g_Script.m_pPlayer->GetScriptState());
	return 1;
}

int CScriptManager::L_setplayer(lua_State *L)
{
	DWORD dwID = static_cast<DWORD>(lua_tonumber(L, 1));
	WORD wOperate = static_cast<WORD>(lua_tonumber(L, 2));
	WORD wOperateEx = static_cast<WORD>(lua_tonumber(L, 3));

	CPlayer *pPlayer = NULL;
	if(wOperateEx == 0)
	{
		pPlayer = (CPlayer *)GetPlayerByGID(dwID)->DynamicCast(IID_PLAYER);
	}
	else
	{
		pPlayer = (CPlayer *)GetPlayerBySID(dwID)->DynamicCast(IID_PLAYER);
	}

	if(pPlayer == NULL) {
		lua_pushnumber(L, 0);
		return 1;
	}

	if(wOperate != 0) 
		g_Script.m_pPlayer = pPlayer;

	lua_pushnumber(L, 1);
	return 1;
}

int CScriptManager::L_setmultipletime( lua_State *L )
{
	if ( g_Script.m_pPlayer == NULL )
		return 0;

	BYTE byTime = static_cast<BYTE>( lua_tonumber( L, 1 ) );

	if ( g_Script.m_pPlayer->GetLeaveTime() < byTime )
		return 0;

	g_Script.m_pPlayer->SetMultipleTime( byTime );
	g_Script.m_pPlayer->SetMulStartTime( ( DWORD )time( NULL ) );
	g_Script.m_pPlayer->SetLeaveTime( g_Script.m_pPlayer->GetLeaveTime() - byTime );
	g_Script.m_pPlayer->AddCurWeekUseMulTime( byTime );

	return 0;
}

int CScriptManager::L_getmultipletimeinfo( lua_State *L )
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	g_Script.m_pPlayer->CheckMulTime();

	BYTE byCurMulTime		= g_Script.m_pPlayer->GetMultipleTime();
	BYTE byCurWeekUseTime	= g_Script.m_pPlayer->GetCurWeekUseTime();
	BYTE byLeaveTime		= ( BYTE )g_Script.m_pPlayer->GetLeaveTime();

	lua_pushnumber( L, byCurMulTime );
	lua_pushnumber( L, byCurWeekUseTime );
	lua_pushnumber( L, byLeaveTime );

	return 3;
}

int CScriptManager::L_setnmultiple(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	BYTE fMultiple = static_cast<BYTE>(lua_tonumber(L, 1));
	g_Script.m_pPlayer->SetNMultiple(fMultiple);

	return 0;
}

int CScriptManager::L_getmultipletime(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	BYTE byTime = g_Script.m_pPlayer->GetMultipleTime();
	lua_pushnumber(L, byTime);

	return 1;
}

int CScriptManager::L_getmulleavetime(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	g_Script.m_pPlayer->CheckMulTime();

	BYTE byTime = ( BYTE )g_Script.m_pPlayer->GetLeaveTime();
	lua_pushnumber(L, byTime);

	return 1;
}

int CScriptManager::L_setlonemanvalue(lua_State *L)
{
	//     if (g_Script.m_pPlayer == NULL)
	//         return 0; 
	// 
	//     float fValue = static_cast<float>(lua_tonumber(L, 1));
	//     g_Script.m_pPlayer->SetLoneManValue(fValue);

	return 0;
}

int CScriptManager::L_checkweekformul(lua_State *L)
{
	//if (g_Script.m_pPlayer == NULL)
	//    return 0;

	//g_Script.m_pPlayer->CheckMulTime();
	//WORD wNextWeek = g_Script.m_pPlayer->GetMulStartTime() == 0 ? 1 : 0;
	//lua_pushnumber(L, wNextWeek);

	return 0;
}

// 取得队长的SID
int L_GetTeamLeaderSID(lua_State* L)
{
	return 0;
}

extern void DelMember( const char* szName, CPlayer *pPlayer );
extern void DelMember( DWORD,DWORD );
int CScriptManager::L_DeleteTeamMember( lua_State* L )
{
	return 0;
}

// 获的一个队伍的数据
int CScriptManager::L_GetTeamInfo( lua_State *L )
{
	return 0;

	//  	if ( g_Script.m_pPlayer == NULL )
	//  		return 0;
	//  
	//  	if ( g_Script.m_pPlayer->m_dwTeamID == 0 )
	//  		return 0;
	// 	int prevStack = lua_gettop( L );
	//     
	// 	Team stTeamInfo;
	// 	if ( !GetTeamInfo( g_Script.m_pPlayer->m_dwTeamID, stTeamInfo ) )
	//         return 0;
	// 
	// 	if ( stTeamInfo.byMemberNum == 0 )
	// 		return 0;
	// 
	// 	lua_createtable( L, stTeamInfo.byMemberNum, 0 );
	// 	if ( !lua_istable( L, -1 ) )
	// 		return 0;
	// 
	// 	// 为每个队员创建一个子表,默认队长为第一个,其他为队员
	// 	for ( int n = 0; n < stTeamInfo.byMemberNum; n++ )
	// 	{
	// 		lua_pushnumber( L, n + 1 );
	//         lua_createtable( L, 0, 10 );
	// 		if ( !lua_istable( L, -1 ) )
	// 	        return lua_settop( L, prevStack ), 0;
	// 			
	// 		// 名字
	// 		lua_pushstring( L, "name" );
	// 		lua_pushstring( L, stTeamInfo.stTeamPlayer[n].szName );
	// 		lua_settable( L, -3 );
	// 
	// 		// 等级
	// 		WORD level = stTeamInfo.stTeamPlayer[n].byLevel;
	// 		lua_pushstring( L, "level" );
	// 		lua_pushnumber( L, level );
	// 		lua_settable( L, -3 );
	// 
	// 		// 帮派
	// 		lua_pushstring( L, "tongName" );
	// 		lua_pushstring( L, stTeamInfo.stTeamPlayer[n].szTongName );
	// 		lua_settable( L, -3 );
	// 
	// 		// 侠义值
	// 		lua_pushstring( L, "xvalue" );
	// 		lua_pushnumber( L, stTeamInfo.stTeamPlayer[n].sXValue );
	// 		lua_settable( L, -3 );
	// 
	// 		// 当前HP
	// 		lua_pushstring( L, "curHP" );
	//         lua_pushnumber( L, stTeamInfo.stTeamPlayer[n].byCurHPPercent );
	// 		lua_settable( L, -3 );
	// 
	// 		// 最大HP
	// 		lua_pushstring( L, "maxHP" );
	// 		lua_pushnumber( L, stTeamInfo.stTeamPlayer[n].wMaxHP );
	// 		lua_settable( L, -3 );
	// 
	// 		// 坐标X
	// 		lua_pushstring( L, "x" );
	// 		lua_pushnumber( L, stTeamInfo.stTeamPlayer[n].wX );
	// 		lua_settable( L, -3 );
	// 
	// 		// 坐标Y
	// 		lua_pushstring( L, "y" );
	// 		lua_pushnumber( L, stTeamInfo.stTeamPlayer[n].wY );
	// 		lua_settable( L, -3 );
	// 
	// 		// 场景编号
	// 		lua_pushstring( L, "regionId" );
	// 		lua_pushnumber( L, stTeamInfo.stTeamPlayer[n].wRegionID );
	// 		lua_settable( L, -3 );
	// 
	// 		// 头像
	// 		lua_pushstring( L, "head" );
	// 		lua_pushnumber( L, stTeamInfo.stTeamPlayer[n].byHead );
	// 		lua_settable( L, -3 );
	// 
	// 		// SID，用于查找目标
	// 		lua_pushstring( L, "staticId" );
	//         lua_pushnumber( L, stTeamInfo.stTeamPlayer[n].staticId );
	// 		lua_settable( L, -3 );
	// 
	// 		// serverId，用于查找目标
	// 		lua_pushstring( L, "serverId" );
	//         lua_pushnumber( L, stTeamInfo.stTeamPlayer[n].wServer );
	// 		lua_settable( L, -3 );
	// 
	// 		lua_settable( L, -3 );
	// 	}
	// 
	// 	assert( prevStack + 1 == lua_gettop( L ) );
	// 	
	return 1;
}

int CScriptManager::L_ViewFactionMsg( lua_State *L )
{
	if ( g_Script.m_pPlayer == NULL )
		return 0;

	if ( g_Script.m_pPlayer->m_Property.m_szTongName[0] == 0 )
		return 0;

	SQQusetGetMsg msg;
	g_StoreMessage( g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof( msg ) );
	return 0;
}

int CScriptManager::L_OperFacRes( lua_State *L )
{
	/*
	if ( g_Script.m_pPlayer == NULL || g_Script.m_pPlayer->m_Property.m_szTongName[0] == 0 )
	{?
	lua_pushnumber(L, FALSE);
	return 1;
	}






	DWORD dwOperType = static_cast<DWORD>( lua_tonumber( L, 1 ) );
	int nNum = static_cast<int>( lua_tonumber( L, 2 ) );

	pFactionData[i]::SFaction faction;
	faction.factionId = 0;
	BOOL bResult = FALSE;

	if ( lua_type( L, 3 ) == LUA_TSTRING )
	{
	LPCSTR factionname = lua_tostring( L, 3 );
	if ( factionname )
	bResult = GetGW()->m_FactionManager.OperFacRes( factionname, dwOperType, nNum );
	}
	else if ( lua_type( L, 3 ) == LUA_TNUMBER )
	{
	DWORD factionId = ( DWORD )lua_tonumber( L, 3 );
	if ( factionId != 0 && factionId < 0xffff )
	{
	if( GetFactionHeaderInfo( ( WORD )factionId, faction ) )
	bResult = GetGW()->m_FactionManager.OperFacRes( faction.szFactionName, dwOperType, nNum );
	}
	}

	lua_pushnumber( L, bResult );
	return 1;
	*/

	lua_pushnumber( L, 1 );
	return 1;
}

int CScriptManager::L_SetPlayerAmassCent( lua_State *L )
{
	// 	DWORD _value = static_cast<DWORD>(lua_tonumber(L, 1));
	// 	if( _value >= 0 && _value < 0xffffffff )
	// 		g_Script.m_pPlayer->m_Property.m_dwAmassCent = _value;

	return 0;
}

int CScriptManager::L_AddPlayerAmassCent( lua_State *L )
{
	// 	int value = static_cast<int>(lua_tonumber(L, 1));
	// 	if( value < 65535 && value > -65535 )    
	// 	{
	// 		if( ( g_Script.m_pPlayer->m_Property.m_dwAmassCent < (0xffffffff - abs(value)) ) &&
	// 			  ( (int)(g_Script.m_pPlayer->m_Property.m_dwAmassCent) >= (int)(-1 * value) ) )
	// 			g_Script.m_pPlayer->m_Property.m_dwAmassCent += value;  
	// 	}

	return 0;
} 

/** 获取配偶所在的场景数据
返回：
-1 没有配偶
-2 配偶不在线
-3 配偶的设置被清除。
*/
int CScriptManager::L_GetMarrowInfo( lua_State *L )
{
	// 	if( g_Script.m_pPlayer == NULL )
	// 		return 0;
	// 
	// 	// 数据类型 暂时还未用，留者以后用
	// 	// int value = static_cast<int>(lua_tonumber(L, 1));
	// 
	// 	// 没有配偶
	// 	if( g_Script.m_pPlayer->m_Property.m_szMateName[0] == 0 )
	// 		return lua_pushnumber( L, -1 ), 1; 
	// 
	// 	// 配偶不在线
	//     LPIObject GetPlayerByName( LPCSTR szName );
	// 	CPlayer *marPlay = (CPlayer *)(GetPlayerByName( g_Script.m_pPlayer->m_Property.m_szMateName )->DynamicCast( IID_PLAYER ) );
	//     if ( marPlay == NULL )
	//         return lua_pushnumber( L, -2 ), 1;
	// 
	//     // 对方的配偶不是你
	//     if ( marPlay->m_Property.m_szMateName[0] == 0 || 
	//         dwt::strcmp( marPlay->m_Property.m_szMateName, g_Script.m_pPlayer->GetName(), CONST_USERNAME ) != 0 )
	//     {
	//         g_Script.m_pPlayer->m_Property.m_szMateName[0] = 0;
	//         g_Script.m_pPlayer->m_Property.m_qwMarryDate = 0;
	//         g_Script.m_pPlayer->SendMyState();
	//         return lua_pushnumber( L, -3 ), 1;
	//     }
	// 
	// 	WORD sceneID = marPlay->m_ParentRegion->m_wRegionID;
	// 	WORD posx = marPlay->m_wCurX; 
	// 	WORD posy = marPlay->m_wCurY;
	// 
	// 	// 名字
	// 	lua_pushnumber( L, sceneID );
	// 	lua_pushnumber( L, posx );
	// 	lua_pushnumber( L, posy ); 
	// 
	// 	return 3;
	return 0;
}  

/** 获取场景的一些信息 ...
*/
int CScriptManager::L_GetRegionInfo( lua_State *L )
{
	if ( g_Script.m_pPlayer == NULL || g_Script.m_pPlayer->m_ParentArea == NULL )
		return 0;

	int pranmaType = ( int )lua_tonumber( L, 1 );
	switch( pranmaType )
	{
		/*配偶所在的场景类型*/
	case 1:
		{
			// 没有配偶
			// 			if( strcmp( g_Script.m_pPlayer->m_Property.m_szMateName, "" ) == 0 )
			// 				return lua_pushnumber( L, -1 ), 1; 
			// 
			// 			// 配偶不在线
			// 			LPIObject GetPlayerByName( LPCSTR szName );
			// 			CPlayer *marPlay = (CPlayer *)(GetPlayerByName( g_Script.m_pPlayer->m_Property.m_szMateName )->DynamicCast( IID_PLAYER ) );
			// 			if (  marPlay == NULL )
			// 				return lua_pushnumber( L, -2 ), 1;
			// 
			// 			return lua_pushnumber( L, marPlay->m_ParentRegion->m_MapProperty ), 1;

			return 0;
		}
		break;
		/*扩展 ..... */
	default:
		return lua_pushnumber( L, -1 ), 1;
	}
}

int CScriptManager::L_GetJoinTime( lua_State *L )
{
	if ( g_Script.m_pPlayer == NULL )
		return 0;

	if ( g_Script.m_pPlayer->m_Property.m_szTongName[0] == 0 || g_Script.m_pPlayer->m_stFacRight.dwJoinTime == 0 )
		return 0;

	tm *local_time = localtime( (time_t*)&g_Script.m_pPlayer->m_stFacRight.dwJoinTime );
	if( local_time == NULL )
		return 0;

	int prevStack = lua_gettop( L );

	lua_createtable( L, 0, 8 );
	if ( !lua_istable( L, -1 ) )
		return 0;

	// 年
	lua_pushstring( L, "year" );
	lua_pushnumber( L, local_time->tm_year + 1900 );
	lua_settable( L, -3 );

	lua_pushstring( L, "mon" );
	lua_pushnumber( L, local_time->tm_mon + 1 );
	lua_settable( L, -3 );

	lua_pushstring( L, "mday" );
	lua_pushnumber( L, local_time->tm_mday );
	lua_settable( L, -3 );

	lua_pushstring( L, "wday" );
	lua_pushnumber( L, local_time->tm_wday );
	lua_settable( L, -3 );

	lua_pushstring( L, "hour" );
	lua_pushnumber( L, local_time->tm_hour );
	lua_settable( L, -3 );

	lua_pushstring( L, "min" );
	lua_pushnumber( L, local_time->tm_min );
	lua_settable( L, -3 );

	lua_pushstring( L, "sec" );
	lua_pushnumber( L, local_time->tm_sec );
	lua_settable( L, -3 );

	lua_pushstring( L, "yday" );
	lua_pushnumber( L, local_time->tm_yday );
	lua_settable( L, -3 );

	assert( prevStack + 1 == lua_gettop( L ) );

	return 1;
}

/*
int CScriptManager::L_GetFactionRes( lua_State *L )
{
if ( g_Script.m_pPlayer == NULL )
return 0;

if ( g_Script.m_pPlayer->m_Property.m_szTongName[0] == 0 )
return 0;

extern BOOL GetFactionInfo( const char* szFactionName, pFactionData[i]& stFactionInfo );
//SimFactionData::SimFactionInfo *pFaction = GetGW()->m_FactionManager.GetFacResBuffer( g_Script.m_pPlayer->m_Property.m_szTongName );
//if( pFaction == NULL )
//	return 0;

pFactionData[i] stFaction;
if ( !GetFactionInfo( g_Script.m_pPlayer->m_Property.m_szTongName, stFaction ) )
return 0;

int prevStack = lua_gettop( L );

lua_createtable( L, 0, 11 );
if ( !lua_istable( L, -1 ) )
return 0;

lua_pushstring( L, "creatorname" );
lua_pushstring( L, stFaction.stFaction.szCreatorName );
lua_settable( L, -3 );

lua_pushstring( L, "memo" );
lua_pushstring( L, stFaction.stFaction.szFactionMemo );
lua_settable( L, -3 );

lua_pushstring( L, "membernum" );
lua_pushnumber( L, stFaction.stFaction.byMemberNum );
lua_settable( L, -3 );

lua_pushstring( L, "level" );
lua_pushnumber( L, stFaction.stFaction.byFactionLevel );
lua_settable( L, -3 );

lua_pushstring( L, "active" );
lua_pushnumber( L, stFaction.stFaction.dwFactionProsperity );
lua_settable( L, -3 );

lua_pushstring( L, "money" );
lua_pushnumber( L, stFaction.stFaction.iMoney );
lua_settable( L, -3 );

lua_pushstring( L, "wood" );
lua_pushnumber( L, stFaction.stFaction.dwWood );
lua_settable( L, -3 );

lua_pushstring( L, "stone" );
lua_pushnumber( L, stFaction.stFaction.dwStone );
lua_settable( L, -3 );

lua_pushstring( L, "mine" );
lua_pushnumber( L, stFaction.stFaction.dwMine );
lua_settable( L, -3 );

lua_pushstring( L, "paper" );
lua_pushnumber( L, stFaction.stFaction.dwPaper );
lua_settable( L, -3 );

lua_pushstring( L, "jade" );
lua_pushnumber( L, stFaction.stFaction.dwJade );
lua_settable( L, -3 );

assert( prevStack + 1 == lua_gettop( L ) );

return 1;
}
*/

int CScriptManager::L_GetMemberInfo( lua_State *L )
{
	if ( g_Script.m_pPlayer == NULL )
		return 0;

	if ( g_Script.m_pPlayer->m_Property.m_szTongName[0] == 0 )
		return 0;

	int prevStack = lua_gettop( L );

	lua_createtable( L, 0, 17 );
	if ( !lua_istable( L, -1 ) )
		return 0;

	// 等级
	lua_pushstring( L, "Level" );
	lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.Level );
	lua_settable( L, -3 );

	// 职务
	lua_pushstring( L, "Title" );
	lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.Title );
	lua_settable( L, -3 );

	// 所属门派与性别
	lua_pushstring( L, "School" );
	lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.School );
	lua_settable( L, -3 );

	// 玩家的阵营
	lua_pushstring( L, "Camp" );
	lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.Camp );
	lua_settable( L, -3 );

	// 任命权
	lua_pushstring( L, "Appoint" );
	lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.Appoint );
	lua_settable( L, -3 );

	// 接纳权
	lua_pushstring( L, "Accept" );
	lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.Accept );
	lua_settable( L, -3 );

	// 授权
	lua_pushstring( L, "Authorize" );
	lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.Authorize );
	lua_settable( L, -3 );

	// 解骋权
	lua_pushstring( L, "Dismiss" );
	lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.Dismiss );
	lua_settable( L, -3 );

	//// 给帮内成员定义特别称号的权力
	//lua_pushstring( L, "EspecialTitle" );
	//lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.EspecialTitle );
	//lua_settable( L, -3 );

	// 留主版权力
	lua_pushstring( L, "Editioner" );
	lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.Editioner );
	lua_settable( L, -3 );

	//// 辞职权
	//lua_pushstring( L, "Resignation" );
	//lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.Resignation );
	//lua_settable( L, -3 );

	// 驱逐权
	lua_pushstring( L, "CastOut" );
	lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.CastOut );
	lua_settable( L, -3 );

	// 退帮
	//lua_pushstring( L, "QuitFaction" );
	//lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.QuitFaction );
	//lua_settable( L, -3 );

	// 贡献
	lua_pushstring( L, "Donation" );
	lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.iDonation );
	lua_settable( L, -3 );

	// 个人成就
	lua_pushstring( L, "Accomplishment" );
	lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.dwAccomplishment );
	lua_settable( L, -3 );

	// 周薪
	lua_pushstring( L, "WeeklySalary" );
	lua_pushnumber( L, g_Script.m_pPlayer->m_stFacRight.dwWeeklySalary );
	lua_settable( L, -3 );

	assert( prevStack + 1 == lua_gettop( L ) );

	return 1;
}

int CScriptManager::L_PlayerAddBuff(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD id = static_cast<WORD>(lua_tonumber(L, 1));
	if (0 == id)
		return 0;

	lua_pushnumber(L,g_Script.m_pPlayer->m_buffMgr.AddBuff(id, g_Script.m_pPlayer));
	return 1;
}

//下马
int CScriptManager::L_Dismount(lua_State *L)
{
	if ( g_Script.m_pPlayer == NULL ) return 0;
	g_Script.m_pPlayer->DownMounts();
	return 0;
}
//增加坐骑属性
int CScriptManager::L_AddMountsProperty(lua_State *L)
{
	if ( g_Script.m_pPlayer == NULL ) return 0;

	int  index = static_cast<int>(lua_tonumber(L, 1));
	int  type = static_cast<int>(lua_tonumber(L, 2));	//属性类型
	int  v = static_cast<int>(lua_tonumber(L, 3));	//属性值

	if( AddMountsProperty(index,type,v) )
		lua_pushnumber(L,1);
	else
		lua_pushnumber(L,0);

	return 1;
}

// 设置角色属性:HP,MP,SP(增量) z
int CScriptManager::L_AddPlayerProperty(lua_State *L)
{
	return 0;
}

int CScriptManager::L_DelFightBuffType(lua_State *L)
{
	if (!g_Script.m_pAttacker)
		return 0;

	MY_ASSERT(EA_SKILL_ATTACK == g_Script.m_pAttacker->GetCurActionID());
	MY_ASSERT(false == g_Script.m_pAttacker->m_AtkContext.m_bSkillAtkOver);

	WORD toWho	= static_cast<WORD>(lua_tonumber(L, 1));
	WORD prop	= static_cast<WORD>(lua_tonumber(L, 2));

	if (0 == toWho || 0 == prop)
		return 0;

	std::list<CFightObject *> &targetList = g_Script.m_pAttacker->m_AtkContext.m_TargetList;

	switch (toWho)
	{
	case 1:
		g_Script.m_pAttacker->m_buffMgr.CheckDelBuffType(prop);
		break;

	case 2:
		if (CPlayer *pPlayer = (CPlayer *)g_Script.m_pAttacker->DynamicCast(IID_PLAYER))
		{
			if (pPlayer->m_dwTeamID)
			{
				extern std::map<DWORD, Team> teamManagerMap;

				std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pPlayer->m_dwTeamID);
				if (iter != teamManagerMap.end())
				{
					MY_ASSERT(iter->second.byMemberNum >= 2 && iter->second.byMemberNum <= 5);

					for (size_t n = 0; n < iter->second.byMemberNum; ++n)
					{
						DWORD gID = iter->second.stTeamPlayer[n].dwGlobal;
						MY_ASSERT(gID);

						CPlayer *pMember = (CPlayer *)GetPlayerByGID(gID)->DynamicCast(IID_PLAYER);
						if (pMember && pMember->m_CurHp)
							pMember->m_buffMgr.CheckDelBuffType(prop);
					}
				}
			}
		}
		break;

	case 3:
		{
			CFightObject *pDefencer = 0;

			switch (g_Script.m_pAttacker->m_AtkContext.m_SkillData.m_AtkType)
			{
			case SAST_SINGLE:			// 单攻
				{
					if (targetList.empty())
						break;

					pDefencer = *targetList.begin();
					pDefencer->m_buffMgr.CheckDelBuffType(prop);
				}
				break;

			case SAST_EMPTY_CIRCLE:		// 圆形群攻（自身原点）
			case SAST_EMPTY_SECTOR:		// 扇形群攻（自身原点）
			case SAST_EMPTY_POINTS:		// 直线群攻（自身原点）
				{
					for (std::list<CFightObject *>::iterator it = targetList.begin(); it != targetList.end(); ++it)
					{
						pDefencer = *it;
						MY_ASSERT(pDefencer);
						pDefencer->m_buffMgr.CheckDelBuffType(prop);
					}
				}
				break;

			case SAST_SELF:
				MY_ASSERT(0);
				break;
			}
		}
		break;
	}

	return 0;
}

int CScriptManager::L_AddFightBuff(lua_State *L)
{
	if (!g_Script.m_pAttacker)
		return 0;

// 	if (g_Script.m_pAttacker->GetCurActionID() != EA_SKILL_ATTACK)
// 	{
// 		return 0;
// 	}

	if (true == g_Script.m_pAttacker->m_AtkContext.m_bSkillAtkOver)
	{
		return 0;
	}

// 	MY_ASSERT(EA_SKILL_ATTACK == g_Script.m_pAttacker->GetCurActionID());
// 	MY_ASSERT(false == g_Script.m_pAttacker->m_AtkContext.m_bSkillAtkOver);

	WORD toWho	= static_cast<WORD >(lua_tonumber(L, 1));
	DWORD id	= static_cast<DWORD>(lua_tonumber(L, 2));

	if (0 == toWho || 0 == id)
		return 0;

	std::list<CFightObject *> &targetList = g_Script.m_pAttacker->m_AtkContext.m_TargetList;

	switch (toWho)
	{
	case 1:				// 给自己加Buff
		g_Script.m_pAttacker->m_buffMgr.AddBuff(id, g_Script.m_pAttacker);
		break;

	case 2:				// 给队友加Buff
		if (CPlayer *pPlayer = (CPlayer *)g_Script.m_pAttacker->DynamicCast(IID_PLAYER))
		{
			if (pPlayer->m_dwTeamID)
			{
				extern std::map<DWORD, Team> teamManagerMap;

				std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pPlayer->m_dwTeamID);
				if (iter != teamManagerMap.end())
				{
					MY_ASSERT(iter->second.byMemberNum >= 2 && iter->second.byMemberNum <= 5);

					for (size_t n = 0; n < iter->second.byMemberNum; ++n)
					{
						DWORD gID = iter->second.stTeamPlayer[n].dwGlobal;
						MY_ASSERT(gID);

						CPlayer *pMember = (CPlayer *)GetPlayerByGID(gID)->DynamicCast(IID_PLAYER);
						if (pMember && pMember->m_CurHp)
							pMember->m_buffMgr.AddBuff(id, g_Script.m_pAttacker);
					}
				}
			}
		}
		break;

	case 3:				// 给目标加Buff
		{
			CFightObject *pDefencer = 0;

			switch (g_Script.m_pAttacker->m_AtkContext.m_SkillData.m_AtkType)
			{
			case SAST_SINGLE:			// 单攻
				{
					if (targetList.empty())
						break;

					pDefencer = *targetList.begin();
					pDefencer->m_buffMgr.AddBuff(id, g_Script.m_pAttacker);
				}
				break;

			case SAST_EMPTY_CIRCLE:		// 圆形群攻（自身原点）
			case SAST_EMPTY_SECTOR:		// 扇形群攻（自身原点）
			case SAST_EMPTY_POINTS:		// 直线群攻（自身原点）
			case SAST_POINT_CIRCLE:		// 有目标的群攻（选择目标点）
				{
					for (std::list<CFightObject *>::iterator it = targetList.begin(); it != targetList.end(); ++it)
					{
						pDefencer = *it;
						MY_ASSERT(pDefencer);
						pDefencer->m_buffMgr.AddBuff(id, g_Script.m_pAttacker);
					}
				}
				break;

			case SAST_SELF:
				MY_ASSERT(0);
				break;
			}
		}
		break;
	}

	return 0;
}

int CScriptManager::GetWindMove(DWORD Type, DWORD DefencerID, WORD skillid, float& DestPosX, float& DestPosY)
{
	float fDefaultDis = 180.0f;

	if (!g_Script.m_pAttacker) return 0;
	if (0 == Type) return 0;

	switch (Type)
	{
	case 1:	// 指定目标ID冲锋(冲到目标背后)
	case 3:	// 指定目标ID冲锋(冲到目标前面)
		{
			if(DefencerID==0)return 0;
			if(skillid==0)return 0;

			CRegion *regin=g_Script.m_pAttacker->m_ParentRegion;
			MY_ASSERT(regin);

			LPIObject object =regin->SearchObjectListInAreas(DefencerID, g_Script.m_pAttacker->m_ParentArea->m_X,g_Script.m_pAttacker->m_ParentArea->m_Y);
			CFightObject *pDefencer=(CFightObject*)object->DynamicCast(IID_FIGHTOBJECT);
			if(!pDefencer)
			{
				rfalse("请求技能找不到目标？？？");
				return 0;
			}

			float DisPosX = pDefencer->m_curX - g_Script.m_pAttacker->m_curX;
			float DisPosY = pDefencer->m_curY - g_Script.m_pAttacker->m_curY;

			float fRadian = atan2(DisPosX, DisPosY);

			if(1==Type)
			{
				DestPosX = pDefencer->m_curX + fDefaultDis * sinf(fRadian);
				DestPosY = pDefencer->m_curY + fDefaultDis * cosf(fRadian);
			}
			if(3==Type)
			{
				DestPosX = pDefencer->m_curX - fDefaultDis * sinf(fRadian);
				DestPosY = pDefencer->m_curY - fDefaultDis * cosf(fRadian);
			}
		}
		break;
	case 2:		// 当前方向冲锋 (闪现)
		{
			const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(skillid);
			WORD wMaxDis = pData->m_SkillMaxDistance;
			DestPosX = g_Script.m_pAttacker->m_curX + wMaxDis * sin(g_Script.m_pAttacker->m_Direction);
			DestPosY = g_Script.m_pAttacker->m_curY + wMaxDis * cos(g_Script.m_pAttacker->m_Direction);
		}
		break;
	case 4:		// 指定目标坐标冲锋
		{
			if(skillid==0)return 0;

			// 目的点坐标
			DestPosX =	g_Script.m_pAttacker->m_AtkContext.mDefenderWorldPosX;
			DestPosY =	g_Script.m_pAttacker->m_AtkContext.mDefenderWorldPosY;
		}
		break;
	case 5:		//拉人,距离200是经验值
		{	
			DestPosX = g_Script.m_pAttacker->m_curX + 200 * sin(g_Script.m_pAttacker->m_Direction);
			DestPosY = g_Script.m_pAttacker->m_curY + 200 * cos(g_Script.m_pAttacker->m_Direction);
		}
		break;
	}

	return 0;
}

int CScriptManager::L_CheckWindMove(lua_State *L)
{
	if (!g_Script.m_pAttacker) return 0;
	return 0;
}

int CScriptManager::L_StartWindMove(lua_State *L)
{
	if (!g_Script.m_pAttacker) return 0;

	MY_ASSERT(EA_SKILL_ATTACK == g_Script.m_pAttacker->GetCurActionID());
	MY_ASSERT(false == g_Script.m_pAttacker->m_AtkContext.m_bSkillAtkOver);

	DWORD Type = static_cast<DWORD>(lua_tonumber(L, 1));
	if (0 == Type) return 0;
	DWORD DefencerID=static_cast<DWORD>(lua_tonumber(L, 2));
	WORD skillid=static_cast<WORD>(lua_tonumber(L, 3));

	float DestPosX, DestPosY ;
	GetWindMove(Type, DefencerID, skillid, DestPosX, DestPosY);



	SAWindMoveMsg msg;
	switch (Type)
	{
	case SSP_CHARGE_SX :
		{
			CFightObject::SkillModifyDest pSkillModifyDest =  g_Script.m_pAttacker->m_AtkContext.m_SkillModifyDest;
			if(pSkillModifyDest.m_bModifyDest)
			{
				const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(skillid);
				WORD wMaxDis = pData->m_SkillMaxDistance;
				wMaxDis += 100;
				//服务器再验证一次
				if( fabs(pSkillModifyDest.m_fDestPosX-DestPosX)>wMaxDis || fabs(pSkillModifyDest.m_fDestPosY-DestPosY)>wMaxDis ) 
				{
					rfalse("闪现技能，客户端传回的坐标不正确，客户端坐标：（X=%.3f，Y=%.3f），服务器坐标：（X=%.3f，Y=%.3f）", 
						pSkillModifyDest.m_fDestPosX, pSkillModifyDest.m_fDestPosY, DestPosX, DestPosY);
					return 0;
				}
				DestPosX = pSkillModifyDest.m_fDestPosX;
				DestPosY = pSkillModifyDest.m_fDestPosY;
			}

			msg.bType		= SAWindMoveMsg::SAM_RUSH;
			msg.dwSelf		= g_Script.m_pAttacker->GetGID();
			msg.dwTarget	= -1;
			msg.fDestX		= DestPosX;
			msg.fDestY		= DestPosY;
			msg.bSkillIndex = skillid;
		}
		break;
	case SSP_CHARGE_AYB :
	case SSP_CHARGE_CF :
		{
			if(DefencerID==0)return 0;
			if(skillid==0)return 0;

			CRegion *regin=g_Script.m_pAttacker->m_ParentRegion;
			MY_ASSERT(regin);

			LPIObject object =regin->SearchObjectListInAreas(DefencerID, g_Script.m_pAttacker->m_ParentArea->m_X,g_Script.m_pAttacker->m_ParentArea->m_Y);
			CFightObject *pDefencer=(CFightObject*)object->DynamicCast(IID_FIGHTOBJECT);
			if(!pDefencer)
			{
				rfalse("请求技能找不到目标？？？");
				return 0;
			}

			CFightObject::SkillModifyDest pSkillModifyDest =  g_Script.m_pAttacker->m_AtkContext.m_SkillModifyDest;
			if(pSkillModifyDest.m_bModifyDest)
			{
				//服务器再验证一次
				if( fabs(pSkillModifyDest.m_fDestPosX-DestPosX)>100 || fabs(pSkillModifyDest.m_fDestPosY-DestPosY)>100 ) 
				{
					rfalse("冲锋类技能：客户端传回来的坐标不正确！");
					return 0;
				}
				DestPosX = pSkillModifyDest.m_fDestPosX;
				DestPosY = pSkillModifyDest.m_fDestPosY;
			}

			msg.bType		= SAWindMoveMsg::SAM_RUSH;
			msg.dwSelf		= g_Script.m_pAttacker->GetGID();
			msg.dwTarget	= pDefencer->GetGID();
			msg.fDestX		= DestPosX;
			msg.fDestY		= DestPosY;
			msg.bSkillIndex = skillid;
		}
		break;
	case SSP_CHARGE_TY :
		{
			if(skillid==0)return 0;

			msg.bType		= SAWindMoveMsg::SAM_RUSH;
			msg.dwSelf		= g_Script.m_pAttacker->GetGID();
			msg.dwTarget	= -1;
			msg.fDestX		= DestPosX;
			msg.fDestY		= DestPosY;
			msg.bSkillIndex=skillid;
		}
		break;
	case SSP_CHARGE_LR :
		{
			if(DefencerID==0)return 0;
			if(skillid==0)return 0;

			CRegion *regin=g_Script.m_pAttacker->m_ParentRegion;
			MY_ASSERT(regin);

			LPIObject object =regin->SearchObjectListInAreas(DefencerID, g_Script.m_pAttacker->m_ParentArea->m_X,g_Script.m_pAttacker->m_ParentArea->m_Y);
			CFightObject *pDefencer=(CFightObject*)object->DynamicCast(IID_FIGHTOBJECT);
			if(!pDefencer)
			{
				rfalse("请求技能找不到目标？？？");
				return 0;
			}

			msg.bType		= SAWindMoveMsg::SAM_PULL;
			msg.dwSelf		= g_Script.m_pAttacker->GetGID();
			msg.dwTarget	= pDefencer->GetGID();
			msg.fDestX		= DestPosX;
			msg.fDestY		= DestPosY;
			msg.bSkillIndex = skillid;

			pDefencer->m_curX = DestPosX;
			pDefencer->m_curY = DestPosY;
			pDefencer->SetBackupActionID(EA_STAND);

			if (pDefencer->m_ParentArea)
				pDefencer->m_ParentArea->SendAdj(&msg, sizeof(msg), -1);
		}
		break;
	}

	if(SSP_CHARGE_LR != Type)
	{
		g_Script.m_pAttacker->m_curX = DestPosX;
		g_Script.m_pAttacker->m_curY = DestPosY;

		if (g_Script.m_pAttacker->m_ParentArea)
			g_Script.m_pAttacker->m_ParentArea->SendAdj(&msg, sizeof(msg), -1);
	}

	return 0;
}

int CScriptManager::L_SendAttackMsg(lua_State *L)
{
	g_Script.m_pAttacker->SendSwitch2SkillAttackStateMsg();
	return 0;
}


int CScriptManager::L_OpenWareHouse(lua_State *L)
{
	WORD type = static_cast<WORD>(lua_tonumber(L, 1));
	if (0 == type || type > 3)
		return 0;

	rfalse("要打开类型为%d的仓库", type);

	if (!g_Script.m_pPlayer)
		return 0;

	SQOpenItemWareHouseMsg msg;
	msg.byDirectly = true;
	memset(msg.szUserpass, 0, sizeof(msg.szUserpass));

	g_Script.m_pPlayer->OnRecvOpenItemWareHouseMsg(&msg);

	return 0;
}

int CScriptManager::L_LockPlayer(lua_State* L)
{
	DWORD dwTime = static_cast<DWORD>( lua_tonumber(L,1) );
	DWORD dwSID = static_cast<DWORD>( lua_tonumber(L,2) );

	CPlayer *pPlayer = 0;
	if( dwSID==0 )	pPlayer = g_Script.m_pPlayer;
	else			pPlayer = (CPlayer *)GetPlayerBySID(dwSID)->DynamicCast(IID_PLAYER);
	if( pPlayer==0 )return 0;

	pPlayer->mLockEvent.Setup( dwTime );

	return 1;
}
int CScriptManager::L_UnLockPlayer(lua_State* L)
{
	DWORD dwSID = static_cast<DWORD>( lua_tonumber(L,1) );

	CPlayer *pPlayer = 0;
	if( dwSID==0 )	pPlayer = g_Script.m_pPlayer;
	else			pPlayer = (CPlayer *)GetPlayerBySID(dwSID)->DynamicCast(IID_PLAYER);
	if( pPlayer==0 )return 0;

	pPlayer->mLockEvent.Destroy( );

	return 1;
}

int CScriptManager::L_NotifyCopySceneTaskReady(lua_State* L)
{
	return 0;
}

int CScriptManager::L_GetDynamicScenePlayerNumber(lua_State* L)
{
	int dynamicSceneGID = static_cast<int>(lua_tonumber(L, 1));
	CRegion* pRegion = ExtraLuaFunctions::GetRegionById(dynamicSceneGID);
	if (pRegion == NULL)
		return 0;

	lua_pushnumber(L, (lua_Number)pRegion->m_PlayerList.size());
	return 1;
}

int CScriptManager::L_DestroyDynamicScene(lua_State* L)
{
	int dynamicSceneGID = static_cast<int>(lua_tonumber(L, 1));
	CRegion* pRegion = ExtraLuaFunctions::GetRegionById(dynamicSceneGID);
	if (pRegion == NULL)
		return 0;

	pRegion->isValid() = false;
	return 0;
}
int CScriptManager::L_GetCurCopyScenesGID(lua_State *L)
{
	DWORD dwPlayerSID = static_cast<DWORD>(lua_tonumber(L, 1));
	CPlayer* pPlayer=0;
	if( dwPlayerSID == 0 )
		pPlayer = g_Script.m_pPlayer;
	else
		pPlayer = (CPlayer *)GetPlayerBySID(dwPlayerSID)->DynamicCast(IID_PLAYER);
	if (pPlayer == NULL)
		return 0;

	DWORD wRegionID = 0;
	if ( pPlayer->m_ParentRegion )
	{
		CDynamicRegion* pRegion = (CDynamicRegion*)pPlayer->m_ParentRegion->DynamicCast( IID_DYNAMICREGION );
		if ( pRegion )			
			wRegionID = pRegion->GetCopySceneGID();
	}
	lua_pushnumber(L, wRegionID);

	return 1;
}
int CScriptManager::L_BackFromDynamicScene(lua_State* L)
{
	DWORD dwPlayerSID = static_cast<DWORD>(lua_tonumber(L, 1));
	CPlayer* pPlayer=0;
	if( dwPlayerSID == 0 )
		pPlayer = g_Script.m_pPlayer;
	else
		pPlayer = (CPlayer *)GetPlayerBySID(dwPlayerSID)->DynamicCast(IID_PLAYER);
	if (pPlayer == NULL)
		return 0;

	CRegion* pRegion = pPlayer->m_ParentRegion;

	// 容错，避免玩家不能回到有效的场景！（主要是解决在奇遇部分常常出不去！）
	if ( pRegion->m_wReLiveRegionID == 0 || pRegion->m_wReLiveRegionID > 1000 )
	{
		pRegion->m_wReLiveRegionID = 1;
		pRegion->m_ptReLivePoint.x = 200; 
		pRegion->m_ptReLivePoint.y = 200; 
	}
	WORD regionid, x, y;
	if(lua_gettop(L)==4)
	{
		regionid = (WORD)lua_tonumber(L,2);
		x = (WORD)lua_tonumber(L,3);
		y = (WORD)lua_tonumber(L,4);
	}
	else
	{
		regionid = (WORD)pRegion->m_wReLiveRegionID;
		x = (WORD)pRegion->m_ptReLivePoint.x;
		y = (WORD)pRegion->m_ptReLivePoint.y; 
	}
	PutPlayerIntoDestRegion(pPlayer, regionid,x,y,0);

	return 0;
}

int CScriptManager::L_SetPlyaerCopySceneState(lua_State* L)
{
	int copySceneState = (int)lua_tonumber( L, 1 );
	DWORD playerSID = (DWORD)lua_tointeger( L, 2 );
	CPlayer* pPlayer = g_Script.m_pPlayer;
	if (playerSID != 0)
		pPlayer = (CPlayer*)GetPlayerBySID(playerSID)->DynamicCast(IID_PLAYER);
	if(pPlayer)
		pPlayer->SetCopySceneState( (CPlayer::COPYSCENE_STATE)copySceneState);
	return 1;
}

int CScriptManager::L_CheckAddGoods(lua_State* L)
{
	// 	int nLen = (int)lua_objlen(L, -1);
	// 	std::map<int,int> goodInfo;
	// 	for ( int i = 1; i < nLen; i ++ )
	// 	{
	// 		int id = 0;
	// 		int num = 0;
	// 		lua_rawgeti( L, 1, 1 );
	// 		id = (int)lua_tointeger(L, -1);
	// 		lua_pop( L, 1 );
	// 		
	// 		lua_rawgeti( L, 1, 2 );
	// 		num = (int)lua_tointeger(L, -1);	
	// 		lua_pop( L, 1 );
	// 
	// 		if ( id != 0)
	// 			goodInfo[id] = num;
	// 	}
	// 
	// 	CPlayer* pPlayer = g_Script.m_pPlayer;
	// 	WORD wBlankNum = pPlayer->GetBlankPackNum();		//背包空位数
	// 
	// 	std::map<int,int>::iterator it = goodInfo.begin();
	// 	for ( it; it != goodInfo.end(); it++)
	// 	{
	// 		const SItemData *itemData = CItem::GetItemData( it->first );
	// 
	// 		if (NULL == itemData)
	// 		{
	// 			rfalse("所需的道具[%d]无法找到!!", it->first);
	// 			lua_pushboolean( L, false);
	// 			return 1;
	// 		}
	// 
	// 		// 叠加数为1或者是装备的时候，占一个格子
	// 		if ( itemData->wOverlay == 1
	// 			|| ( itemData->byType >= ITEM_T_EQUIPMENT_BEGIN && itemData->byType <= ITEM_T_EQUIPMENT_END ))
	// 		{
	// 			wBlankNum = wBlankNum - it->second;
	// 		}
	// 		// 叠加数不为1，并且不是装备类
	// 		else
	// 		{
	// 			int nMaxLord = itemData->wOverlay;	//这种物品一格的最大堆叠数
	// 			if ( nMaxLord == 0)
	// 				nMaxLord = 9999;
	// 			int nNum = pPlayer->GetItemNum( it->first, true);	//取得当前该物品的个数
	// 			if ( nNum > nMaxLord)
	// 				nNum = nNum % nMaxLord;
	// 			wBlankNum = wBlankNum - ( nNum + it->second)/nMaxLord;
	// 		}
	// 		if ( wBlankNum < 0)
	// 		{
	// 			lua_pushboolean( L, false);
	// 			return 1;
	// 		}
	// 	}
	// 	lua_pushboolean( L, true);
	// 	return 1;
	return 0;
}

BOOL CScriptManager::RegisterFunc()
{
	RegisterFunction("DoFile", L_DoFile);

	RegisterFunction("rfalse", L_rfalse);
	RegisterFunction("RFalse", L_RFalse); 
	RegisterFunction("GetStartServerTime", L_GetStartServerTime);
	RegisterFunction("GiveExp", L_giveExp); //增加玩家经验
	RegisterFunction("fpGiveExp", L_fpgiveExp); //增加玩家经验
	RegisterFunction("GiveGoods", L_giveGoods);
	RegisterFunction("GiveMoney", L_giveMoney);
	RegisterFunction("GiveSp", L_giveSp);
	RegisterFunction("GiveMount", L_giveMount);
	RegisterFunction("GivePet", L_givePet);
	RegisterFunction("GiveFightPet", L_giveFightPet);

	RegisterFunction("ClearPlayerCSItem", L_ClearPlayerCSItem);
	//RegisterFunction("MonsterAI", L_monsterai);
	RegisterFunction("FollowLoadItem",L_FollowLoadItem);
	RegisterFunction("LoadFollowMenuInfo",L_LoadFollowMenuInfo);
	RegisterFunction("ClearFollowShop",L_ClearFollowShop);
	RegisterFunction("LoadItem", L_loaditem);
	RegisterFunction("SetNpcGoodCount", L_setnpcgoodscount);
	//RegisterFunction("CreateMonster", L_createmonster);
	//RegisterFunction("RemoveCurrObj", L_removecurrobj);
	RegisterFunction("DelObject", L_delobject);
	RegisterFunction("GetAttribute", L_getAttribute);
	RegisterFunction("SetAttribute", L_setAttribute);
	RegisterFunction("Activate", L_activate);
	RegisterFunction("SetMessage", L_setmessage);
	//RegisterFunction("AddBlood", L_addblood);
	RegisterFunction("SetMessageA", L_setmessageA);
	//RegisterFunction("AddTask", L_addtask);
	//RegisterFunction("BuyTask", L_buytask);
	//RegisterFunction("CheckTask", L_checktask);
	//RegisterFunction("GetNeedBlank", L_GetNeedBlank);
	RegisterFunction("CheckGoods", L_checkgoods);
	//RegisterFunction("CheckMoney", L_checkmoney);
	RegisterFunction("GetItemNum", L_getitemnum);
	//RegisterFunction("DelTask", L_deltask);
	RegisterFunction("GetPlayerData", L_getplayerdata);
	RegisterFunction("NewSetPlayerProperty", L_SetPlayerProperty);
	RegisterFunction("SetPlayerObject", L_SetPlayerObject);
	RegisterFunction("GetPlayerSID", L_GetPlayerSID);
	RegisterFunction("GetPlayerGID", L_getplayerGID);
	RegisterFunction("SetPlayerData", L_setplayerdata);
	RegisterFunction("GetMonsterData", L_getmonsterdata);
	RegisterFunction("SetMonsterData",L_setmonsterdata);
	RegisterFunction("GetNpcData", L_getnpcdata);
	RegisterFunction("SetMessageB", L_setmessageB);
	RegisterFunction("PlayCG", L_playcg);
	RegisterFunction("Sub_TaskNum", L_subtasknum);
	RegisterFunction("GetTaskData", L_gettaskdata);
	RegisterFunction("GetForgeData", L_getforgedata);
	RegisterFunction("GetGoodsData", L_getgoodsdata);
	RegisterFunction("SetMessageC", L_setmessageC);
	RegisterFunction("rint", L_rint);
	RegisterFunction("PutPlayerTo", L_putplayerto);
	RegisterFunction("PutPlayerFromDynamic", L_putplayerfromdynamic); 
	RegisterFunction("PutPlayerBack", L_putplayerback);
	RegisterFunction("ReItem", L_reitem);
	RegisterFunction("isFull", L_isfull);
	RegisterFunction("isFullNum", L_isFullNum);
	RegisterFunction("SetMessageR", L_setmessageR);
	RegisterFunction("ActiveSkill", L_activeskill);
	RegisterFunction("ActiveTempSkill",L_activeTempskill);
	RegisterFunction("CoolingTempSkill",L_CoolingTempSkill);
	RegisterFunction("SendTemplateSkill",L_SendTemplateSkill);
	RegisterFunction("GetEventID",L_GetEventID);
	RegisterFunction("ActiveTelegry", L_activetelegry);
	RegisterFunction("ActivePackage", L_ActivePackage);
	RegisterFunction("SendActivePackage", L_SendActivePackage);

	RegisterFunction("Zero", L_zero);
	RegisterFunction("TeachGest", L_teachgest);
	RegisterFunction("PayThePlayer", L_paytheplayer);
	RegisterFunction("SetTaskString", L_settaskstring);
	RegisterFunction("CheckMoreItem",L_checkmoreitem);
	RegisterFunction("GetTelergy", L_gettelergy);
	RegisterFunction("GetTaskItemName", L_gettaskitemname);
	RegisterFunction("GetItemName", L_getitemname);
	RegisterFunction("GetMountFoodAddExp", L_getMountFoodAddExp);
	RegisterFunction("LostItems", L_lostitems);
	RegisterFunction("FillCopyItem",  L_fillcopyitem);
	RegisterFunction("GetScores", L_getscores);
	RegisterFunction("SetRegionPKType", L_setregionpktype);    
	RegisterFunction("BroadCast", L_broadcast);  
	RegisterFunction("SetObjectListSize", L_setlistsize);
	//RegisterFunction("CreateNpc", L_createnpc);
	RegisterFunction("GetCurTime", L_getcurtime);
	RegisterFunction("GetCurTimeInMin", L_getcurtimeinmin);
	RegisterFunction("GetCurTimeA", L_getcurtimeA);
	RegisterFunction("CompileFile", L_compilefile);
	RegisterFunction("GetServerTime", L_GetServerTime);
	RegisterFunction("GetGestLevel", L_getgestlevel);
	RegisterFunction("SetGestLevel", L_setgestlevel);
	RegisterFunction("DelItemByPos", L_delitembypos);
	RegisterFunction("RequareFactionName", L_requarefactionname);
	RegisterFunction("CreateFaction", L_createfaction);
	RegisterFunction("LeaveFaction", L_leavefaction);
	RegisterFunction("GetKillInfo", L_GetKillInfo);
	RegisterFunction("TeachTeamGest", L_teachteamgest);
	RegisterFunction("SetPlayerScriptState", L_setplayerscriptstate);
	RegisterFunction("GetPlayerScriptState", L_getplayerscriptstate);
	RegisterFunction("SetPlayer", L_setplayer);
	RegisterFunction("SetPlayerIcon", L_setplayericon);

	RegisterFunction("SetMultipleTime", L_setmultipletime);    
	RegisterFunction("SetNMultiple", L_setnmultiple);       
	RegisterFunction("GetMultipleTime", L_getmultipletime);    
	RegisterFunction("GetMulLeaveTime", L_getmulleavetime);    
	RegisterFunction("SetLoneManValue", L_setlonemanvalue);  
	RegisterFunction("CheckWeekForMul", L_checkweekformul);
	RegisterFunction("GetCurTelergy", L_getcurtelergy);       
	RegisterFunction("SetCurTelergyVal", L_setcurtelergyval); 
	RegisterFunction("TalkToPlayer", L_talktoplayer);
	RegisterFunction("GetTelergyData", L_gettelergydata);

	RegisterFunction("CreateRegion", L_CreateRegion);  
	RegisterFunction("CheckCardID",L_QueryEncouragement);
	RegisterFunction("GetMultipleTimeInfo", L_getmultipletimeinfo);
	RegisterFunction("GetTeamInfo", L_GetTeamInfo);
	RegisterFunction("GetTeamLeaderSID", L_GetTeamLeaderSID);
	RegisterFunction("DeleteTeamMember", L_DeleteTeamMember);

	RegisterFunction( "ViewFactionMsg", L_ViewFactionMsg );
	RegisterFunction( "GetMemberInfo", L_GetMemberInfo );
	RegisterFunction( "GetJoinTime", L_GetJoinTime );

	// RegisterFunction( "GetFactionRes", L_GetFactionRes );
	RegisterFunction( "OperFacRes", L_OperFacRes );
	RegisterFunction( "SetPlayerAmassCent", L_SetPlayerAmassCent );
	RegisterFunction( "AddPlayerAmassCent", L_AddPlayerAmassCent );
	RegisterFunction( "GetMarrowInfo", L_GetMarrowInfo );
	RegisterFunction( "GetRegionInfo", L_GetRegionInfo );
	RegisterFunction( "SendBoardCastMsg", L_SendBoardCastMsg );
	RegisterFunction( "AddPlayerProperty", L_AddPlayerProperty );
	RegisterFunction( "AddMountsProperty", L_AddMountsProperty );	

	RegisterFunction( "StartCollect", L_StartCollection );
	RegisterFunction( "MakeNpcDisappear", L_MakeNpcDisappear);
	RegisterFunction("SetTips", L_settipmsg);
	RegisterFunction("Dismount", L_Dismount);

	// 副本相关接口
	RegisterFunction("LockPlayer", L_LockPlayer);
	RegisterFunction("UnLockPlayer", L_UnLockPlayer);
	RegisterFunction("GetDynamicScenePlayerNumber", L_GetDynamicScenePlayerNumber);
	RegisterFunction("DestroyReginDynamicScene", L_DestroyDynamicScene);
	RegisterFunction("BackFromDynamicScene", L_BackFromDynamicScene);
	RegisterFunction("SetPlyaerCopySceneState", L_SetPlyaerCopySceneState);
	RegisterFunction("GetCurCopyScenesGID", L_GetCurCopyScenesGID);
	RegisterFunction("CheckAddGoods", L_CheckAddGoods);

	RegisterFunction( "MonsterAddHpEvent", L_MonsterAddHpEvent);
	RegisterFunction( "MonsterAddBuffEvent", L_MonsterAddBuffEvent);

	// 侠三仓库系统
	RegisterFunction("OpenWareHouse", L_OpenWareHouse);

	RegisterFunction("AddFightBuff", L_AddFightBuff);
	RegisterFunction("DelFightBuffType", L_DelFightBuffType);
	RegisterFunction("PlayerAddBuff", L_PlayerAddBuff);
	RegisterFunction("CheckWindMove", L_CheckWindMove);
	RegisterFunction("StartWindMove", L_StartWindMove);
	RegisterFunction("SendAttackMsg", L_SendAttackMsg);	

	RegisterFunction("loaddropgoods", L_loaddropgoods);
	RegisterFunction("loaddropmonsterid", L_loaddropmonsterid);
	RegisterFunction("loaddroptime", L_loaddroptime);
	RegisterFunction("loaddropgoodsnum", L_loaddropgoodsnum);

	///怪物直接掉落物品
	RegisterFunction("monsterdropitem", L_monsterdropitem);
	RegisterFunction("getattackername",L_getattackername);
	//怪物附近喊话
	RegisterFunction("MonsterTalk",L_monstertalk);
	//侠义道三队伍相关
	RegisterFunction("GetPlayerTeam",L_getplayerteam);
	RegisterFunction("GetPlayerTeamName",L_getplayerteamname);
	//侠义道三新增
	RegisterFunction("Setplayerproperty", L_setplayerproperty);
	RegisterFunction("creategift", L_creategift);
	RegisterFunction("usegift", L_usegift);
	//2014.2.22 Add
	RegisterFunction("AddGiftJudgePackage", L_AddGiftJudgePackage);

	RegisterFunction("QuestUpdateBoss", L_QuestUpdateBoss);
	RegisterFunction("setplayerpoint",L_setplayerpoint);
	RegisterFunction("getplayerSkillproperty",L_getplayerSkillproperty); //获取技能等级相关
	RegisterFunction("setactivityNotice",L_setactivityNotice);//设置日常活动
	RegisterFunction("GetLiveMonster",L_GetLiveMonster);//得到指定场景怪物数量

	//设置玩家好友度L_GetRegionCount
	RegisterFunction("SetPlayerDegree",L_setplayerDegree);
	//侠客学习技能
	RegisterFunction("fpActiveSkill", L_fpactiveskill);
	RegisterFunction("FightPetAddBuff", L_FightPetAddBuff);//侠客加血蓝buff
	//体质系统
	RegisterFunction("GrowTiZhi",L_tizhiGrow);
	RegisterFunction("UpgradeTiZhi",L_tizhiUpgrade);
	//麒麟臂
	RegisterFunction("ActiveKylinArm",L_kylinArmActive);
	RegisterFunction("AddYuanqi",L_addYuanqi);

	// 心法升级消耗 和 附加属性计算公式
	RegisterFunction("ConsumeTelergy",L_ConsumeTelergy);
	RegisterFunction("ConsumeTelergyProperty", L_ConsumeTelergyProperty);

	//祈福
	RegisterFunction("ProcessBlessInfo",L_ProcessBlessInfo);
	RegisterFunction("FillBlessData",L_FillBlessData);
	RegisterFunction("OnBlessResult",L_OnBlessResult);
	RegisterFunction("OnPlayerBlessTime",L_OnPlayerBlessTime);
	RegisterFunction("GetPlayerMoney",L_GetPlayerMoney);
	RegisterFunction("ChangeMoney",L_ChangeMoney);
	RegisterFunction("OnErrorStopBless",L_OnErrorStopBless);
	RegisterFunction("IsHaveSpaceCell",L_IsHaveSpaceCell);
	RegisterFunction("OnProcessNotice",L_OnProcessNotice);
	//新手在线礼包相关
	/*	RegisterFunction("StopTime",L_StopTime);
	RegisterFunction("StartTime",L_StartTime);
	RegisterFunction("CleanTime",L_CleanTime);
	RegisterFunction("IsStopTime",L_IsStopTime);*/	
	RegisterFunction("ResetOnlineGiftState",L_ResetOnlineGiftState);	
	RegisterFunction("GetOnlineGiftState",L_GetOnlineGiftState);
	RegisterFunction("UpdateOnlineGiftState",L_UpdateOnlineGiftState);
	RegisterFunction("GetOnlineBeginTime",L_GetOnlineBeginTime);	
	RegisterFunction("GetNowTime",L_GetNowTime);	

	//倒计时奖励相关
	RegisterFunction("GetCountDownGiftState",L_GetCountDownGiftState);
	RegisterFunction("ResetCountDownGiftState",L_ResetCountDownGiftState);	
	RegisterFunction("UpdateCountDownGiftState",L_UpdateCountDownGiftState);
	RegisterFunction("GetCountDownBeginTime",L_GetCountDownGiftBeginTime);
	RegisterFunction("ResetCountDownGiftTime",L_ResetCountDownGiftTime);


	RegisterFunction("SendBufferToClinet",L_SendBufferToClinet);	
	RegisterFunction("SendItemList",L_SendItemList);	
	RegisterFunction("SendGiftSucceed",L_SendGiftSucceed);	
	RegisterFunction("GetPlayerTime",L_GetPlayerTime);
	RegisterFunction("SetPlayerVipLevel",L_SetPlayerVipLevel);	

	RegisterFunction("SaveToPlayerData",L_SaveToPlayerData);	
	RegisterFunction("LoadFromPlayerData",L_LoadFromPlayerData);	
	RegisterFunction("AddPlayerRoseRecod",L_AddPlayerRoseRecod);
	RegisterFunction("LimitForHuiShenDan", L_HuiShenDanRecall);
	RegisterFunction("giveItemList",L_giveItemList);
	RegisterFunction("CheckPlayerMoney",L_CheckPlayerMoney);

	// 全装备属性加成
	RegisterFunction("ModifyPlayerAttributes", L_ModifyPlayerAttributes);

	RegisterFunction("LoadPlayerModifValue",L_LoadPlayerModifValue);
	RegisterFunction("UnLoadPlayerModifValue",L_UnLoadPlayerModifValue);
	RegisterFunction("OnPlayerClearBuff",L_PlayerClearBuff);
	RegisterFunction("OnBackFightPet",L_OnBackFightPet);
	RegisterFunction("LuaRandRange",L_LuaRandRange);
	RegisterFunction("SetCrilial",L_SetCrilial);
	RegisterFunction("LoadProtectInfo",L_LoadProtectInfo);

	RegisterFunction("OnPartEnterNum",L_OnPartEnterNum);
	RegisterFunction("OnTeamPartEnterNum",L_OnTeamPartEnterNum);
	RegisterFunction("IsPersonalEnterPart",L_IsPersonalEnterPart);
	RegisterFunction("IsTeamEnterPart",L_IsTeamEnterPart);
	RegisterFunction("IsNonTeamPartEnter",L_IsNonTeamPartEnter);
	RegisterFunction("SetGmakeParm",L_SetGmakeParm);
	RegisterFunction("GetGmakeParm",L_GetGmakeParm);

	RegisterFunction("SetJingMaiShareExpTimes",L_SetJingMaiShareExpTimes);



	RegisterFunction("logtable",L_logtable);
	RegisterFunction("LoadPlayerData",L_LoadPlayerData);
	RegisterFunction("SavePlayerData",L_SavePlayerData);
	RegisterFunction("SendPlayerReward",L_SendPlayerReward);
	RegisterFunction("GetActivePackageNum",L_GetActivePackageNum);
	RegisterFunction("SavePlayerTable",L_SavePlayerTable);
	RegisterFunction("LoadPlayerTable",L_LoadPlayerTable);
	RegisterFunction("PlayerProcessQuestSkill", L_ProcessQuestSkill);
	
	RegisterFunction("CI_GetStartTime", L_GetServerStartTime);//获取服务器启动时间
	LuaRegisterFunc();

	ExtraScriptFunctions::RegisterScriptFunctions();

	CSOtherFunc::CreateShadow();
	CSPlayerFunc::CreateShadow();
	CSNPCFunc::CreateShadow();
	CSMonsterFunc::CreateShadow();
	//20150122 wk 三国独立脚本模块
	CSSanGuoFunc::CreateShadow();
	// 日常时间管理器
	RegisterFunction("ReadEveryDayManagerTime",L_ReadEveryDayManagerTime);
	RegisterFunction("WriteEveryDayManagerTime",L_WriteEveryDayManagerTime);

	rfalse(2, 1, "Register Lua functions  OK!");
	return TRUE;
}

BOOL CScriptManager::DoFunc(const char *szFuncName, DWORD wID, DWORD dwChoseID)
{
	char szFileName[64] = {0};

	sprintf(szFileName, "%s%d", szFuncName, wID);

	if (dwChoseID != 0)
	{
		if (CallFunc(szFileName, dwChoseID) == false)
			return FALSE;
	}
	else
	{
		if (CallFunc(szFileName) == false)
			return FALSE;
	}

	return TRUE;
}

int CScriptManager::L_DoFile(lua_State *L)
{
	const char *szFileName = static_cast<const char*>(lua_tostring(L, 1));
	if(!szFileName) 
		return 0;


	int nRet = g_Script.DoFile(szFileName);

	lua_pushnumber(L, nRet);

	return 1;
}

int CScriptManager::L_SendBoardCastMsg( lua_State *L )
{
	char str[MAX_PATH]={};
	dwt::strcpy(str, static_cast<const char*>(lua_tostring(L, 1)), MAX_PATH);
	if(!str)return 0;
	BroadcastTalkToMsg(str,0);		//暂时这样使用,因为下面的系统广播客户端暂时未完全处理
	//BroadcastMsg( &chatmsg, sizeof(chatmsg));
	return 1;
}

BOOL CScriptManager::Restore( int flag,const char* name )
{  
	if ( flag )
	{
		CallFunc( "OnScriptClosing" );
		Reset();
		RegisterFunc();
		CallFunc( "OnScriptReseted" );
	}

	load_errs = 0;
	if (name)
	{
		DoFile(name);
		return load_errs;
	}
	DoFile("Init.lua");
	return load_errs;
}

void CScriptManager::SetLoop(void)
{
	// 执行脚本事件!
	void RunLuaEvent();
	RunLuaEvent();

	WORD wTime = (int)GetGlobalValue("LOOP_TIME");

	if (wTime == 0)
		wTime = 1;

// 	DWORD n = (timeGetTime() - m_dwLoopTime);
// 	rfalse("SetLoop = %l",n);
	if ((DWORD)(timeGetTime() - m_dwLoopTime) > 1000*wTime)
	{
		CleanCondition();
		CallFunc("ActOnRun");
		m_dwLoopTime = timeGetTime();
	}
}

struct SLuaEvent
{
	static const int EVENT_BITSET = 8;
	static const int EVENT_TICKMAX = ( 1 << EVENT_BITSET );
	static const int EVENT_TICKMASK = EVENT_TICKMAX - 1;
	static const int EVENT_HIGHMASK = ~EVENT_TICKMASK;

	DWORD tick;                             // 高位表示遍历序号，低位为当前数组下标 
	DWORD margin;                           // 高位表示遍历序号，触发间隔
	DWORD uid;                              // 高位表示遍历序号，触发间隔
	std::string func;                       // 回调函数名
	std::list< lite::lua_variant > args;    // 参数列表
};

static std::map< DWORD, QWORD > uidmap;
static std::list< SLuaEvent > events[ SLuaEvent::EVENT_TICKMAX ];   // 最短频率为1秒，65535可以支持18个小时
static std::list< SLuaEvent >::iterator runIter;                    // 循环触发中的迭代器，主要用于限制 PushLuaEvent 和 ClrEvent 清除自己
static DWORD prevTick = ( DWORD )time( NULL );

// 添加事件，返回一个id，一方面用于表示事件添加成功，另一方面用于事件的销毁！
QWORD PushLuaEvent( lua_State *L )
{
	int t1 = lua_type( L, 1 );
	int t2 = lua_type( L, 2 );
	int t3 = lua_type( L, 3 );

	if ( t1 != LUA_TNUMBER || t3 != LUA_TSTRING )
		return -4;

	DWORD margin = ( DWORD )lua_tonumber( L, 1 );
	LPCSTR func = lua_tostring( L, 3 );

	// 不能将时间设置得太大，超过1天。。。
	if ( margin == 0 || margin > 60*60*24 )
		return -3;

	DWORD tick = margin + ( DWORD )time( NULL );

	std::list< SLuaEvent > &l = events[ tick & SLuaEvent::EVENT_TICKMASK ];
	l.push_back( SLuaEvent() );
	SLuaEvent &e = l.back();
	e.tick = tick;
	e.margin = margin;
	e.uid = 0;
	e.func = func;

	QWORD kid = ( _w64 DWORD )&l | ( ( QWORD )( ( _w64 DWORD )&e & 0x7fffffff ) << 32 );

	// 如果存在唯一ID，则需要尝试将上一个事件清除，并重设自己新的kid
	if ( t2 == LUA_TNUMBER )
	{
		DWORD uid = ( DWORD )lua_tonumber( L, 2 );
		if ( uid == 0 )
			return -4;

		int ClrEvent( DWORD uid, QWORD *pidk );
		int ick = ClrEvent( uid, NULL );
		if ( ick < 0 )
			return ick;

		uidmap[ uid ] = kid;
		e.uid = uid;
	}

	// 附加参数
	int ck = 1;
	int top = lua_gettop( L );
	try
	{
		for ( int i = 4; i <= top; i ++ )
		{
			e.args.push_back( lite::lua_variant() );
			ck = reinterpret_cast< lite::lua_variant & >( e.args.back() ).get( L, i );
			if ( ck == 0 )
				break;
		}
	}
	catch ( lite::Xcpt& )
	{
		ck = 0;
	}

	if ( ck == 0 )
	{
		l.pop_back();
		return -2;
	}

	return kid;
}

// 清除事件，注意不能在处理当前事件时删除自己！
int ClrEvent( DWORD uid, QWORD *pidk )
{
	QWORD idk = pidk ? *pidk : 0;
	if ( pidk == NULL )
	{
		std::map< DWORD, QWORD >::iterator it = uidmap.find( uid );
		if ( it != uidmap.end() )
			idk = it->second;
	}

	if ( idk != 0 )
	{
		std::list< SLuaEvent > *pl = ( std::list< SLuaEvent >* )( idk & 0xffffffff );
		SLuaEvent *pe = ( SLuaEvent* )( idk >> 32 );
		if ( pl >= &events[0] && pl <= &events[SLuaEvent::EVENT_TICKMASK] )
		{
			for ( std::list< SLuaEvent >::iterator it = pl->begin(); it != pl->end(); it ++ )
			{
				if ( pe == &*it && ( pe->tick & SLuaEvent::EVENT_TICKMASK ) == ( pl - events ) )
				{
					if ( it == runIter )
						return -1;

					if ( pe->uid )
						uidmap.erase( pe->uid );

					pl->erase( it );
					return 1;
				}
			}
		}
	}

	return 0;
}

// 执行事件
void RunLuaEvent()
{
	DWORD curTick = ( DWORD )time( NULL );
	if ( prevTick >= curTick )
		return;

	//DWORD checkMask = prevTick & SLuaEvent::EVENT_TICKMASK;
	for ( DWORD i = prevTick; i != curTick; i ++ )
	{
		std::list< SLuaEvent > &l = events[ i & SLuaEvent::EVENT_TICKMASK ];
		if ( l.empty() )
			continue;

		for ( runIter = l.begin(); runIter != l.end(); )
		{
			if ( runIter->tick != i )
				runIter ++;
			else
			{
				int again = -1;
				try
				{
					g_Script.PrepareFunction( runIter->func.c_str() ); 
					for ( std::list< lite::lua_variant >::iterator k = runIter->args.begin(); k != runIter->args.end(); k ++ )
						g_Script.PushParameter( *k );
					g_Script.CleanCondition();
					lite::Variant ck;
					g_Script.Execute( &ck );
					if ( ck.dataType == lite::Variant::VT_INTEGER )
						again = ck._integer;
				}
				catch ( lite::Xcpt& )
				{
					g_Script.PrepareBreak();
				}
				g_Script.CleanCondition();

				// 是否继续执行
				BOOL remove = false;
				if ( again >= 0 )
				{
					if ( again == 0 )
						again = runIter->margin;

					if ( again > 0 && again <= 60*60*24 )
					{
						std::list< SLuaEvent > &ll = events[ ( runIter->tick + again ) & SLuaEvent::EVENT_TICKMASK ];
						ll.push_back( SLuaEvent() );
						SLuaEvent &e = ll.back();
						e.tick = runIter->tick + again;
						e.margin = again;
						e.uid = runIter->uid;
						e.func.swap( runIter->func );
						e.args.swap( runIter->args );
						remove = true;

						if ( runIter->uid )
						{
							QWORD kid = ( _w64 DWORD )&ll | ( ( QWORD )( ( _w64 DWORD )&e & 0x7fffffff ) << 32 );
							uidmap[ runIter->uid ] = kid;
						}
					}
				}

				if ( !remove && runIter->uid )
					uidmap.erase( runIter->uid );

				runIter = l.erase( runIter );
			}
		}
	}

	prevTick = curTick;
}

LPCSTR GetMemCheckString()
{
	return GetNpcScriptInfo( "mem crush" );
}

int CScriptManager::L_GetServerTime(lua_State *L)
{
	//20150127 wk 修改lua时间与服务器时间一致
	//time_t serverTime = time(0)+60;
	//lua_pushnumber( L,serverTime ); // 服务器时间跑快60秒

	time_t serverTime = time(0);
	lua_pushnumber(L, serverTime); 
	return 1;
}
bool  CScriptManager::AddMountsProperty(int mountIndex, int porperty, int v, int max/*=-1*/)
{
	if ( g_Script.m_pPlayer == NULL ) return false;
	int  index = mountIndex;
	if( index<0 )
		index = g_Script.m_pPlayer->GetEquipMount();

	SPlayerMounts::Mounts* pM = g_Script.m_pPlayer->GetMounts(index);
	if( !pM ) return false;

	int  type = porperty;
	switch (type)
	{
	case 1:
		{
			//if( pM->Hunger >= 100.0f ) return false;
			// 			pM->Hunger += v;
			// 			if(pM->Hunger>100.0f)
			// 				pM->Hunger=100.0f;
			// 			else if(pM->Hunger<=0)
			// 			{
			// 				pM->Hunger = 0.0f;
			// 				if( g_Script.m_pPlayer->GetEquipMount()==index )
			// 					g_Script.m_pPlayer->UnEquipMount();
			// 			}
			// 			SASetMountHunger shunger;
			// 			shunger.showTips = 1;
			// 			shunger.index = index;
			// 			shunger.hunger = (int)( pM->Hunger + 0.5);
			// 			g_StoreMessage( g_Script.m_pPlayer->m_ClientIndex, &shunger, sizeof(SASetMountHunger));
		}
		break;
	case 2:
		{			
			// 			if( v<=0 ) return false;
			// 			if( max<0 ) max = g_Script.m_pPlayer->m_byLevel;
			// 			int rt =  INSTANCE(Mounts::CMountTemplates)->AddMountExp(pM,v,max);
			// 
			// 			if( rt==1 )// 升级了
			// 				g_Script.m_pPlayer->SendStatePanelDataMsg();
			// 
			// 			SASetMountExp expMsg; 
			// 			expMsg.index = index;
			// 			expMsg.exp = pM->Exp;
			// 			expMsg.level = pM->Level;
			// 
			// 			g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex,&expMsg,sizeof(expMsg));
			// 			if( rt==-1 )	//不能升级获得经验是，不扣除物品
			// 				return false;
		}
		break;
	default:
		return false;
	}
	return true;
}

int CScriptManager::L_MonsterAddHpEvent( lua_State* L)
{
	if( g_Script.m_pMonster && 2==lua_gettop(L) ) {
		float rate = (float)lua_tonumber(L, 1);
		string func = lua_tostring(L,2);
		g_Script.m_pMonster->scriptEvent.hpTable.push_back( CMonsterScriptEvent::HPEvent(rate, func) );
	}

	return 0;
}

int CScriptManager::L_MonsterAddBuffEvent( lua_State* L)
{
	if( g_Script.m_pMonster && 2==lua_gettop(L) ) {
		DWORD id = (DWORD)lua_tointeger(L, 1);
		string func = lua_tostring(L,2);
		g_Script.m_pMonster->scriptEvent.buffTable.push_back( CMonsterScriptEvent::BuffEvent(id, func) );
	}
	return 0;
}

int CScriptManager::L_loaddropgoods( lua_State *L )
{
	DWORD dropTable = static_cast<DWORD>( lua_tonumber(L,1) );
	DWORD dropgoodsid = static_cast<DWORD>( lua_tonumber(L,2) );

	if (dropTable<1)return 0;
	CItemDropService::GetInstance().LoadDropSpecial(dropTable,dropgoodsid,DROP_GOODID); 
	return 0;
}

int CScriptManager::L_loaddropgoodsnum( lua_State *L )
{
	DWORD dropTable = static_cast<DWORD>( lua_tonumber(L,1) );
	DWORD goodsnum = static_cast<DWORD>( lua_tonumber(L,2) );

	if (dropTable<1)return 0;
	CItemDropService::GetInstance().LoadDropSpecial(dropTable,goodsnum,DROP_GOODCOUNT); 
	return 0;
}

int CScriptManager::L_loaddroptime( lua_State *L )
{
	DWORD dropTable = static_cast<DWORD>( lua_tonumber(L,1) );
	DWORD droptime = static_cast<DWORD>( lua_tonumber(L,2) );

	if (dropTable<1)return 0;
	CItemDropService::GetInstance().LoadDropSpecial(dropTable,droptime,DROP_TIME); 
	return 0;
}

int CScriptManager::L_loaddropmonsterid( lua_State *L )
{
	DWORD dropTable = static_cast<DWORD>( lua_tonumber(L,1) );
	DWORD monsterid = static_cast<DWORD>( lua_tonumber(L,2) );

	if (dropTable<1)return 0;
	CItemDropService::GetInstance().LoadDropSpecial(dropTable,monsterid,DROP_MONSTERID); 
	return 0;
}

void CScriptManager::SetFightDefencer( class CFightObject *pDefencer )
{
	m_pDefencer	=	pDefencer;
}

void CScriptManager::CleanFightDefencer()
{
	m_pDefencer	= 0;
}

int CScriptManager::L_monsterdropitem( lua_State *L )
{
	if(!g_Script.m_pMonster)return 0;

	DWORD itemid		= static_cast<DWORD>( lua_tonumber(L,1) );
	DWORD itemnum = static_cast<DWORD>( lua_tonumber(L,2) );
	if (itemid == 0 || itemnum == 0)
	{
		rfalse(4,1,"CScriptManager::L_monsterdropitem( lua_State *L )");
	}
	return g_Script.m_pMonster->GetDropItem(itemid,itemnum);
}

int CScriptManager::L_getattackername( lua_State *L )
{
	if(!g_Script.m_pAttacker)return 0;
	lua_pushstring(L,g_Script.m_pAttacker->getname());
	return 1;
}

int CScriptManager::L_setplayerpoint( lua_State *L )
{
	if(!g_Script.m_pPlayer)return 0;
	DWORD pointtype = static_cast<DWORD>( lua_tonumber(L,1) );
	DWORD pointvalue = static_cast<DWORD>( lua_tonumber(L,2) );
	g_Script.m_pPlayer->AddPlayerPoint(pointtype,pointvalue);
	return 1;
}

int CScriptManager::L_monstertalk( lua_State *L )
{
	if (lua_isnumber(L,1)&&lua_isstring(L,2))
	{
		DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
		CMonster *pmonster = (CMonster*)GetObjectByGID(gid)->DynamicCast(IID_MONSTER);
		if (pmonster)
		{
			LPCSTR szStr = static_cast<const char*>(lua_tostring(L, 2));
			MonsterTalkInfo(pmonster, szStr);
		}
	}
	return 1;
}

int CScriptManager::L_getplayerteam( lua_State *L )
{
	if(!g_Script.m_pPlayer)return -1;
	int index = static_cast<int>(lua_tonumber(L, 1));
	if (index == 0)return -1;

	switch (index)
	{
	case 1:		// 是否有组队
		lua_pushnumber(L, g_Script.m_pPlayer->m_dwTeamID);
		break;
	case 2:		// 是否是队长
		lua_pushnumber(L, g_Script.m_pPlayer->m_bIsTeamLeader);
		break;
	case 3:		// 队伍人数
		lua_pushnumber(L, g_Script.m_pPlayer->m_wTeamMemberCount);
		break;
	case 4:
		lua_pushnumber(L,g_Script.m_pPlayer->GetTeamLeaderRegionGID());
		break;
	case 5:
		lua_pushnumber(L,g_Script.m_pPlayer->GetTeamCurPartRegionGID());
		break;
	default:
		return 0;
	}
	return 1;
}

int CScriptManager::L_getplayerteamname( lua_State *L )
{
	if(!g_Script.m_pPlayer)return -1;
	int index = static_cast<int>(lua_tonumber(L, 1));
	if (index == 0)return -1;
	index--;
	lua_pushstring(L, g_Script.m_pPlayer->GetTeamLeaderName(index));
	return 1;
}

int CScriptManager::L_setplayerdatanew( lua_State *L )
{
	if(!g_Script.m_pPlayer)return -1;
	int index = static_cast<int>(lua_tonumber(L, 1));
	int data =	static_cast<int>(lua_tonumber(L, 2));
	return 1;
}

int CScriptManager::L_setplayerDegree( lua_State *L )
{
	if(!g_Script.m_pPlayer)return 0;
	LPCSTR destname=lua_tostring(L, 1);
	int degree = static_cast<int>(lua_tonumber(L, 2));
	CPlayer *pdest = (CPlayer*)GetPlayerByName(destname)->DynamicCast(IID_PLAYER); 
	if (!pdest)return 0;
	g_Script.m_pPlayer->ChangeDegree(g_Script.m_pPlayer->m_ClientIndex,destname,degree);
	pdest->ChangeDegree(pdest->m_ClientIndex,g_Script.m_pPlayer->GetName(),degree);
	return 1;
}

int CScriptManager::L_fpactiveskill( lua_State* L )
{
	if(!g_Script.m_pPlayer)return -1;
	WORD nSkillID = (WORD)lua_tointeger(L, 1);
	if (0 == nSkillID)return -1;
	WORD index = (WORD)lua_tointeger(L, 2);
	//if (g_Script.m_pPlayer->_fpActivaSkill(nSkillID, index,"学习技能！"))return 1;
	return 0;
}

int CScriptManager::L_tizhiGrow( lua_State* L )
{
	if (!g_Script.m_pPlayer) 
	{
		lua_pushboolean(L,false);
		return 1;
	}
	DWORD itemid = lua_tointeger(L,1);
	if (!itemid)
	{
		lua_pushboolean(L,false);
		return 1;
	}
	lua_pushboolean(L,g_Script.m_pPlayer->tizhiGrow(itemid));
	return 1;
}

int CScriptManager::L_tizhiUpgrade( lua_State* L )
{
	if (!g_Script.m_pPlayer)
	{
		lua_pushboolean(L,false);
		return 1;
	}
	DWORD itemid = lua_tointeger(L,1);
	if (!itemid)
	{
		lua_pushboolean(L,g_Script.m_pPlayer->tizhiUpgrade(itemid));
		return 1;
	}
	return 1;
}

int CScriptManager::L_kylinArmActive( lua_State* L )
{
	if (!g_Script.m_pPlayer)
	{
		lua_pushboolean(L,false);
		return 1;
	}
	lua_pushboolean(L,g_Script.m_pPlayer->activeKylinArm());
	return 1;
}

int CScriptManager::L_addYuanqi( lua_State* L )
{
	if (!g_Script.m_pPlayer)
	{
		lua_pushboolean(L,false);
		return 1;
	}
	WORD yuanqi = lua_tointeger(L,1);
	g_Script.m_pPlayer->addPlayerYuanqi(yuanqi);
	lua_pushboolean(L,true);
	return 1;
}

int CScriptManager::L_fpgiveExp( lua_State *L )
{
	if (!g_Script.m_pPlayer)
		return 0;

	DWORD exp = static_cast<DWORD>(lua_tonumber(L, 1));
	if (!g_Script.m_pPlayer->m_pFightPet)return 0;

	g_Script.m_pPlayer->m_pFightPet->SendAddFightPetExp(exp, SAExpChangeMsg::TASKFINISHED, "ScriptGive");

	return 1;
}

int CScriptManager::L_FightPetAddBuff( lua_State *L )
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD id = static_cast<WORD>(lua_tonumber(L, 1));
	if (0 == id)
		return 0;

	if (!g_Script.m_pPlayer->m_pFightPet)return 0;
	g_Script.m_pPlayer->m_pFightPet->m_buffMgr.AddBuff(id, g_Script.m_pPlayer->m_pFightPet);

	return 1;
}

int CScriptManager::L_setplayerproperty( lua_State *L )
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD type = static_cast<WORD>(lua_tonumber(L, 1));

	int setvalue = static_cast<int>(lua_tonumber(L, 2));

	switch (type)
	{
	case 7:
		g_Script.m_pPlayer->OnChangePKValue(setvalue);
		break;
	case 20:
		g_Script.m_pPlayer->m_Property.m_CurTp = setvalue;
		if (g_Script.m_pPlayer->m_Property.m_CurTp > g_Script.m_pPlayer->m_MaxTp)
		{
			g_Script.m_pPlayer->m_Property.m_CurTp = g_Script.m_pPlayer->m_MaxTp;
		}
		g_Script.m_pPlayer->m_FightPropertyStatus[XA_CUR_TP] = true;
		//g_Script.m_pPlayer->ModifyCurrentTP(setvalue, BAM_POT);
		break;
	default:
		break;
	}
	return 1;
}

// 技能、心法升级消耗
int CScriptManager::L_ConsumeTelergy(lua_State *L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	float needSp	= static_cast<float>(lua_tonumber(L, 1));	// 消耗真气
	float needMoney = static_cast<float>(lua_tonumber(L, 2));	// 消耗金钱
	BYTE  type		= static_cast<BYTE>(lua_tonumber(L, 3));	// 心法1， 技能2

	// 扣除升级所需要的金钱
	if (!g_Script.m_pPlayer->CheckPlayerMoney(2, (DWORD)needMoney, true))
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "升级技能所需【金钱】不足!");
		return 0;
	}
	if (!g_Script.m_pPlayer->CheckPlayerSp((DWORD)needSp, true))
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "升级技能所需【信仰之力】不足!");
		return 0;
	}
	//消耗钱和真气
	g_Script.m_pPlayer->ChangeMoney(2, -(long)needMoney);
	g_Script.m_pPlayer->AddPlayerSp(-(DWORD)needSp);

	g_Script.m_pPlayer->UpdateAllProperties();

	(type == 1) ? (g_Script.m_pPlayer->m_updateTelergy = true) : (MY_ASSERT(type == 2), g_Script.m_pPlayer->m_updateSkill = true);

	return 1;
}

// 心法的提升属性
int CScriptManager::L_ConsumeTelergyProperty(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	BYTE type	= static_cast<BYTE>(lua_tonumber(L, 1));
	WORD value	= static_cast<WORD>(lua_tonumber(L, 2));
	WORD rate	= static_cast<WORD>(lua_tonumber(L, 3));

	if (type < 0)
	{
		return 0;
	}
	else if (type >= 7)
	{
		if (rate <= 0)
		{
			return 0;
		}
		MY_ASSERT(rate > 0);

		// 主动技能触发的概率
		if (g_Script.m_pPlayer->m_bFirstLogIn)
		{
			g_Script.m_pPlayer->m_TelergyActiveRate[type - TET_SUCKHP_PER] += rate * g_Script.m_pPlayer->m_bTempLevel;
		}
		else
			g_Script.m_pPlayer->m_TelergyActiveRate[type - TET_SUCKHP_PER] += rate;
	}
	else
	{
		// 附加属性值
		if (g_Script.m_pPlayer->m_bFirstLogIn)
		{
			g_Script.m_pPlayer->m_TelergyFactor[type] += value * g_Script.m_pPlayer->m_bTempLevel;
		}
		else
			g_Script.m_pPlayer->m_TelergyFactor[type] += value;
	}

	//g_Script.m_pPlayer->UpdateAllProperties();

	return 1;
}

// int CScriptManager::L_creategift( lua_State *L )
// {
// //	if (!g_Script.m_pPlayer)return 0;
// 
// 	DWORD giftid = static_cast<DWORD>(lua_tonumber(L, 1)); //礼包ID
// 	if(giftid == 0)return 0;
// 	WORD num	= 0;
// 	std::list<SAddItemInfo> itemList;
// 	SRawItemBuffer *itemBuffer = g_Script.m_pItemBuffer;
// 	if(!itemBuffer) return 0;
// 	if (lua_istable(L,2))
// 	{
// 		WORD index = 0;
// 		char strIndex[20]={0};
// 		while(lua_gettop(L))
// 		{	
// 			//index++;
// 			itoa(++index,strIndex,10);
// 			DWORD ItemId		= static_cast<DWORD>(GET_FIELD_NUMBER_DIRECTLY(2, strIndex));
// 
// 			//index++;
// 			itoa(++index,strIndex,10);
// 			WORD  Itemnum 	= static_cast<WORD >(GET_FIELD_NUMBER_DIRECTLY(2, strIndex));
// 			
// 		}
// 	
// 		while(lua_gettop(L))
// 		{
// 			switch (lua_type( L, lua_gettop( L ) )) 
// 			{
// 			case LUA_TTABLE:
// 				{
// 					lua_pushnil(L);
// 					int num = -2;
// 					DWORD itemid = 0xffffffff;
// 					WORD itemnum = 0xffff;
// 					 while (lua_next(L, num) != 0)
// 					 {
// 						 /* 'key' (索引-2) 和 'value' (索引-1) */  
// 						 const char* key = "unknown";  
// 						 const char* value;  
// 						 if(lua_type(L, -2) == LUA_TSTRING)  
// 						 {  
// 							 key = lua_tostring(L, -2);  
// 							 value = lua_tostring(L, -1);  
// 						 }  
// 						 else if(lua_type(L, -2) == LUA_TNUMBER)  
// 						 {  
// 							 // 因为lua_tostring会更改栈上的元素，  
// 							 // 所以不能直接在key上进行lua_tostring  
// 							 // 因此，复制一个key，压入栈顶，进行lua_tostring  
// 							 lua_pushvalue(L, -2);  
// 							 key = lua_tostring(L, -1);  
// 							 lua_pop(L, 1);  
// 							 value = lua_tostring(L, -1);  
// 						 }  
// 						 else  
// 						 {  
// 							 value = lua_tostring(L, -1);  
// 						 }  
// 						if (atoi(key)%2 != 0){
// 							itemid = atol(value);
// 						}
// 						else{
// 							itemnum = atol(value);
// 						}
// 						
// 						if (itemid != 0xffffffff && itemnum != 0xffff)
// 						{
// 							itemList.push_back(SAddItemInfo(itemid, itemnum));
// 							itemid = 0xffffffff;
// 							itemnum = 0xffff;
// 						}
// 						 /* 移除 'value' ；保留 'key' 做下一次迭代 */  
// 						 lua_pop(L, 1);
// 					 }
// 				}
// 			}
// 				 lua_pop( L, 1 );
// 		}		
// 	}
// 	
// 	num = itemList.size();
// 	if (num == 0)
// 	{
// 		AddInfo("需要添加到礼包中的物品一个都没有");
// 		return 0;
// 	}
// 	// 给玩家添加一个礼包道具
// 
// 	const SItemBaseData *itemdata = CItemService::GetInstance().GetItemBaseData(giftid);
// 	if(!itemdata)
// 	{
// 		AddInfo(FormatString("没有找到礼包道具%d,生成道具失败",giftid));
// 		return 0;
// 	}
// 
// // 	WORD pos = g_Script.m_pPlayer->FindBlankPos(XYD_PT_BASE);
// // 	if (0xffff == pos)
// // 	{
// // 		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex,"背包已满，无法获取");
// // 		return 0;
// // 	}
// 
// 	BYTE *Buffer = itemBuffer->buffer;
//  	memcpy(Buffer,&num,sizeof(WORD));
//  	Buffer += sizeof(WORD);
// 
// 	std::list<SAddItemInfo>::iterator iter = itemList.begin();
// 	while(iter != itemList.end())
// 	{
// 		SAddItemInfo iteminfo = *iter;
// 		memcpy(Buffer,&iteminfo.m_Index,sizeof(DWORD));
// 		Buffer += sizeof(DWORD);
// 
// 		memcpy(Buffer,&iteminfo.m_Count,sizeof(WORD));
// 		Buffer += sizeof(WORD);
// 		
// 		iter++;
// 	}
// 	//lua_pushlightuserdata(L,itemBuffer);
// 	//g_Script.m_pPlayer->AddExistingItem(*itemBuffer, pos, true);
// 	return 1;
//    
// }

int CScriptManager::L_usegift( lua_State *L )
{
	if (!g_Script.m_pUseItem || !g_Script.m_pPlayer)return 0;
	DWORD itemid	= static_cast<DWORD>(lua_tonumber(L, 1));
	MY_ASSERT(itemid == g_Script.m_pUseItem->wIndex);

	// 	const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(itemid);
	// 	if (!pData)return 0;
	// 
	// 	if (!g_Script.m_pPlayer->CanUseIt(g_Script.m_pPlayer,g_Script.m_pUseItem,pData))return 0;


	std::list<SAddItemInfo> itemList;
	DWORD additem = 0;
	WORD  addnum = 0 , count = 0;
	BYTE *bagBuffer = g_Script.m_pUseItem->buffer;
	memcpy(&count,bagBuffer,sizeof(WORD));
	bagBuffer += sizeof(WORD);

	if (count == 0 || count > 20)
	{
		AddInfo("礼包中物品数量不正确");
		return 0;
	}
	while(count)
	{
		memcpy(&additem,bagBuffer,sizeof(DWORD));
		bagBuffer += sizeof(DWORD);

		memcpy(&addnum,bagBuffer,sizeof(WORD));
		bagBuffer += sizeof(WORD);

		itemList.push_back(SAddItemInfo(additem,addnum));
		count --;
	}
	if (g_Script.m_pPlayer->CanAddItems(itemList))
	{
		g_Script.m_pPlayer->StartAddItems(itemList);
		return 1;
	}
	else
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "背包已满！无法获取道具！");
		return 0;
	}

	return 0;
}


int CScriptManager::L_AddGiftJudgePackage(lua_State *L)		//添加元宝礼包道具时，判断是否背包足够
{
	if (/*!g_Script.m_pUseItem || */!g_Script.m_pPlayer)
	{
		lua_pushnumber(L, 0);
		return 1;
	}
	int iYuanBaoGiftCount = lua_tonumber(L, -1);
	int iIsHaveNewGiftID = lua_tonumber(L, -2);
	int CurClickGiftCellPos = lua_tonumber(L, -3);
	//没有添加道具,恢复当前的元宝礼包
	SAddItemInfo ReplyItemPackage;
	if (iYuanBaoGiftCount == 0)	//没有道具可以更新到背包中
	{
		lua_pushnumber(L, 1);
		return 1;
	}
	std::list<SAddItemInfo> itemList;
	DWORD additem = 0;
	WORD  addnum = 0;
	//获取表中的装备信息
	for (int k = 1; k <= iYuanBaoGiftCount; k++)
	{
		lua_getglobal(g_Script.ls, "GetYuanBaoGiftInfo");
		lua_pushnumber(g_Script.ls, k);
		lua_pcall(g_Script.ls, 1, 2, 0);
		additem = lua_tonumber(g_Script.ls, -1);
		addnum = lua_tonumber(g_Script.ls, -2);
		itemList.push_back(SAddItemInfo(additem, addnum));
		lua_pop(g_Script.ls, 3);
	}
	//是否有新礼包
	if (iIsHaveNewGiftID != 0)	//有新礼包，把新礼包添加到待添加的装备列表中
	{
		additem = iIsHaveNewGiftID;
		addnum = 1;
      	itemList.push_back(SAddItemInfo(additem, addnum));
	}

	//先删除背包中当前礼包，判断是否有足够的格子
	SPackageItem* pPackageItem = g_Script.m_pPlayer->FindItemByPos(CurClickGiftCellPos, XYD_FT_ONLYUNLOCK);
	if (pPackageItem == NULL)
	{
		lua_pushnumber(L, 0);
		return 1;
	}
	//缓存道具的ID和数量
	ReplyItemPackage.m_Index = pPackageItem->wIndex;
	ReplyItemPackage.m_Count = pPackageItem->overlap;
	if (ReplyItemPackage.m_Count == 1)
		g_Script.m_pPlayer->DelItem(*pPackageItem);
	else
		pPackageItem->overlap--;

	//判断是否有足够的格子
	if (g_Script.m_pPlayer->CanAddItems(itemList))		//判断能否添加道具
	{
		g_Script.m_pPlayer->StartAddItems(itemList);	//添加道具
		lua_pushnumber(L, 1);
		return 1;
	}
	//没有添加道具,恢复当前的元宝礼包
	if (ReplyItemPackage.m_Count == 1)
		g_Script.m_pPlayer->AddItem(ReplyItemPackage);
	else
		pPackageItem->overlap++;
	lua_pushnumber(L, 0);
	return 1;
}

void CScriptManager::SetInUseItemPackage( SPackageItem *pitem )
{
	m_pUseItem = pitem ;
}

void CScriptManager::CleanItemPackage()
{
	m_pUseItem = NULL;
}

void CScriptManager::SetRawItemBuffer( SRawItemBuffer *ItemBuffer )
{
	m_pItemBuffer = ItemBuffer;
}

void CScriptManager::CleanRawItemBuffer()
{
	m_pItemBuffer = NULL;
}
int CScriptManager::L_FollowLoadItem(lua_State *L)
{
	WORD Index = static_cast<WORD>(lua_tonumber(L,1));
	DWORD ItemID = static_cast<DWORD>(lua_tonumber(L,2));
	CFollowShopService::GetInstance().LoadItemInfo(Index,ItemID);
	return 1;
}
int CScriptManager::L_LoadFollowMenuInfo(lua_State *L)
{
	WORD Index = static_cast<WORD>(lua_tonumber(L,1));
	LPCSTR ItemID = static_cast<LPCSTR>(lua_tostring(L,2));
	CFollowShopService::GetInstance().LoadFollowMenuInfo(Index,ItemID);
	return 1;
}
int CScriptManager::L_ClearFollowShop(lua_State *L)
{
	CFollowShopService::GetInstance().ClearFollowShop();
	return 1;
}
int CScriptManager::L_ProcessBlessInfo(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	if (GetApp() && GetApp()->m_pGameWorld)
	{
		GetApp()->m_pGameWorld->ProcessBlessInfo(g_Script.m_pPlayer->m_ClientIndex,g_Script.m_pPlayer->m_Property.m_BlessFlag);
	}
	return 1;
}
int CScriptManager::L_QuestUpdateBoss( lua_State *L )
{
	if (!g_Script.m_pPlayer)return 0;

	DWORD nstate = static_cast<WORD>(lua_tonumber(L,1));
	SQBossDeadUpdatemsg DeadUpdatemsg;
	DeadUpdatemsg.nBossState = nstate;
	g_Script.m_pPlayer->OnRecvQuestUpdateBossMsg(&DeadUpdatemsg);
	return 1;
}

void CScriptManager::SetPlayer( class CPlayer *pPlayer )
{
	g_Script.m_pPlayer = pPlayer;
}

void CScriptManager::CleanPlayer()
{
	g_Script.m_pPlayer  = NULL;
}

int CScriptManager::L_UpdateCountDownGiftState( lua_State* L )
{
	if (!g_Script.m_pPlayer)return 0;	
	g_Script.m_pPlayer->_L_UpdateCountDownGiftState();
	return 1;
}

int CScriptManager::L_UpdateOnlineGiftState(lua_State* L)
{
	if(!g_Script.m_pPlayer) return 0;
	int index = static_cast<WORD>(lua_tonumber(L,1));
	BYTE state = static_cast<BYTE>(lua_tonumber(L,2));
	g_Script.m_pPlayer->_L_UpdateOnlineGiftState(index, state);
	return 1;
}

int CScriptManager::L_GetCountDownGiftState( lua_State* L )
{
	if (!g_Script.m_pPlayer)return 0;
	BYTE state = g_Script.m_pPlayer->_L_GetCountDownGiftState();
	lua_pushnumber(L,state);
	return 1;
}

int CScriptManager::L_GetOnlineGiftState( lua_State* L )
{
	if (!g_Script.m_pPlayer)return 0;
	WORD index = static_cast<WORD>(lua_tonumber(L,1));
	BYTE state = g_Script.m_pPlayer->_L_GetOnlineGiftState(index);
	lua_pushnumber(L,state);
	return 1;
}

int CScriptManager::L_GetOnlineBeginTime( lua_State* L )
{
	if (!g_Script.m_pPlayer)return 0;
	INT64 LoginTime = g_Script.m_pPlayer->_L_GetOnlineBeginTime();
	lua_pushnumber(L,LoginTime);
	return 1;
}

int CScriptManager::L_SendBufferToClinet( lua_State* L )
{
	if (!g_Script.m_pPlayer)return 0;
	WORD SendType = static_cast<WORD>(lua_tonumber(L,1));
	if (SendType == SABufferPackMsg::PACK_ROSERECORD)return 0;
	return L_SendItemList(L);
}

int CScriptManager::L_SendItemList( lua_State* L )
{
	if (!g_Script.m_pPlayer)return 0;
	WORD SendType = static_cast<WORD>(lua_tonumber(L,1));
	WORD ItemNum = static_cast<WORD>(lua_tonumber(L,2));

	WORD	  cangive = static_cast<WORD>(lua_tonumber(L,4));//能否领取
	DWORD needTime = static_cast<DWORD>(lua_tonumber(L,5));//领取礼包的条件
	DWORD remainTime = static_cast<DWORD>(lua_tonumber(L,6));//领取礼包的剩余条件

	std::list<SAddItemInfo> itemList;

	if (lua_istable(L,3))
	{
		while(lua_gettop(L))
		{
			switch (lua_type( L, lua_gettop( L ) )) 
			{
			case LUA_TTABLE:
				{
					lua_pushnil(L);
					int num = -2;
					DWORD itemid = 0xffffffff;
					WORD itemnum = 0xffff;
					while (lua_next(L, num) != 0)
					{
						/* 'key' (索引-2) 和 'value' (索引-1) */  
						const char* key = "unknown";  
						const char* value;  
						if(lua_type(L, -2) == LUA_TSTRING)  
						{  
							key = lua_tostring(L, -2);  
							value = lua_tostring(L, -1);  
						}  
						else if(lua_type(L, -2) == LUA_TNUMBER)  
						{  
							// 因为lua_tostring会更改栈上的元素，  
							// 所以不能直接在key上进行lua_tostring  
							// 因此，复制一个key，压入栈顶，进行lua_tostring  
							lua_pushvalue(L, -2);  
							key = lua_tostring(L, -1);  
							lua_pop(L, 1);  
							value = lua_tostring(L, -1);  
						}  
						else  
						{  
							value = lua_tostring(L, -1);  
						}  
						if (atoi(key)%2 != 0){
							itemid = atol(value);
						}
						else{
							itemnum = atol(value);
						}

						if (itemid != 0xffffffff && itemnum != 0xffff)
						{
							itemList.push_back(SAddItemInfo(itemid, itemnum));
							itemid = 0xffffffff;
							itemnum = 0xffff;
						}
						/* 移除 'value' ；保留 'key' 做下一次迭代 */  
						lua_pop(L, 1);
					}
				}
			}
			lua_pop( L, 1 );
		}		
	}
	if (ItemNum != itemList.size())
	{
		AddInfo("需要发送的物品数量不正确");
		return 0;
	}

	SABufferPackMsg BufferPackMsg;
	BufferPackMsg.PackType = SendType;
	BufferPackMsg.Packnum = ItemNum;

	BYTE		*pBuff = BufferPackMsg.PackBuff;

	memcpy(pBuff,&cangive,sizeof(WORD));
	pBuff +=sizeof(WORD);

	memcpy(pBuff,&needTime,sizeof(DWORD));
	pBuff +=sizeof(DWORD);

	memcpy(pBuff,&remainTime,sizeof(DWORD));
	pBuff +=sizeof(DWORD);

	std::list<SAddItemInfo>::iterator iter = itemList.begin();
	while(iter != itemList.end())
	{
		SAddItemInfo iteminfo = *iter;
		memcpy(pBuff,&iteminfo.m_Index,sizeof(DWORD));
		pBuff += sizeof(DWORD);

		memcpy(pBuff,&iteminfo.m_Count,sizeof(WORD));
		pBuff += sizeof(WORD);

		iter++;
	}

	int buffsize = (int)(pBuff - BufferPackMsg.PackBuff);
	if (buffsize > MAX_PACK_BUFFER){
		AddInfo(FormatString("需要发送的数据包太大了,当前支持的最大数据包大小%d字节",MAX_PACK_BUFFER));
		return 0;
	}

	int msgsize = sizeof(SABufferPackMsg) - (MAX_PACK_BUFFER - buffsize);
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex,&BufferPackMsg,msgsize);
	return 1;
}

int CScriptManager::L_SaveToPlayerData( lua_State* L )
{
	if (!g_Script.m_pPlayer)return 0;
	WORD SavePos = static_cast<WORD>(lua_tonumber(L,1));
	int	 SaveValue = static_cast<int>(lua_tonumber(L,2));
	if (SavePos * sizeof(int) >= LUABUFFERSIZE)
	{
		AddInfo("存储位置已满");
		return 0;
	}
	BYTE *pBuffer = g_Script.m_pPlayer->m_Property.m_bluaBuffer;
	pBuffer += SavePos * sizeof(int);
	memcpy(pBuffer,&SaveValue,sizeof(int));
	return 1;
}

int CScriptManager::L_LoadFromPlayerData( lua_State* L )
{
	if (!g_Script.m_pPlayer)return 0;
	WORD LoadPos = static_cast<WORD>(lua_tonumber(L,1));
	if (LoadPos * sizeof(int) >= LUABUFFERSIZE)
	{
		AddInfo("读取位置出错");
		return 0;
	}
	int loadValue = 0;
	BYTE *pBuffer = g_Script.m_pPlayer->m_Property.m_bluaBuffer;
	pBuffer += LoadPos * sizeof(int);
	memcpy(&loadValue,pBuffer,sizeof(int));
	lua_pushnumber(L,loadValue);
	return 1;
}


int CScriptManager::L_LoadPlayerModifValue(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	if (g_Script.m_pPlayer->m_TemplateMap.size() != 0)
	{
		g_Script.m_pPlayer->UpdataPlayerCounterpartProperties();
	}
	return 1;
}
int CScriptManager::L_UnLoadPlayerModifValue(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	if (g_Script.m_pPlayer->m_TemplateMap.size() != 0)
	{
		g_Script.m_pPlayer->m_TemplateMap.clear();
		g_Script.m_pPlayer->m_buffMgr.Free();
		g_Script.m_pPlayer->UpdateAllProperties();
	}
	return 1;
}
int CScriptManager::L_PlayerClearBuff(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	g_Script.m_pPlayer->m_buffMgr.Free();
	return 1;
}
int CScriptManager::L_OnBackFightPet(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	g_Script.m_pPlayer->CallBackFightPet(false);
	return 1;
}
int CScriptManager::L_OnCallOutFightPet(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	return 1;
}
int CScriptManager::L_FillBlessData(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	long ItemID = static_cast<long>(lua_tonumber(L,1));
	g_Script.m_pPlayer->m_Blessvec.push_back(ItemID);
	return 1;
}
int CScriptManager::L_OnBlessResult(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	long Flag = static_cast<long>(lua_tonumber(L,1));
	long Result = static_cast<long>(lua_tonumber(L,2));
	long FLAGX = static_cast<long>(lua_tonumber(L,3));
	BYTE QuickFlag = static_cast<BYTE>(lua_tonumber(L,4));
	g_Script.m_pPlayer->m_Blessvec.push_back(Result);
	//std::random_shuffle(g_Script.m_pPlayer->m_Blessvec.begin(),g_Script.m_pPlayer->m_Blessvec.end());
	if (g_Script.m_pPlayer->m_Blessvec.size() != 12)
	{
		rfalse("没有等于12");
		return 1;
	}
	SABlessItemReQuestMsg pMsg;
	pMsg.Flag = FLAGX;
	pMsg.QuickFlag = QuickFlag;
	pMsg.m_ItemID = 11;
	for (int i = 0; i < g_Script.m_pPlayer->m_Blessvec.size(); ++i)
	{
		SPackageItem Item;
		if (i == 11)
		{
			//存入玩家数据
			g_Script.m_pPlayer->CreateNewItemInfo(g_Script.m_pPlayer->m_Blessvec[i],2,Item,FLAGX);
		}
		else
		{
			g_Script.m_pPlayer->CreateNewItemInfo(g_Script.m_pPlayer->m_Blessvec[i],1,Item,FLAGX);
		}
		pMsg.m_ItemIDVec[i] = Item;
	}
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex,&pMsg,sizeof(SABlessItemReQuestMsg));
	g_Script.m_pPlayer->m_Blessvec.clear();
	g_Script.m_pPlayer->m_Property.m_HightiTEM = 0;
	if (Flag == 1)
	{
		g_Script.m_pPlayer->m_Property.m_HightiTEM = 1;
		//const SItemBaseData * psi = CItemService::GetInstance().GetItemBaseData(Result);
		//if (psi)
		//{
		//	long IDd = psi->m_ID;
		//	char chr[20];
		//	memset(chr,0,20);
		//	itoa(IDd,chr,10);
		//	std::string IDName = chr;
		//	GetApp()->m_pGameWorld->AddGlobalBlessInfo(g_Script.m_pPlayer->m_Property.m_Name,IDName);
		//	SABlessAllServerNoticeMsg pMsgAll;
		//	std::string name = g_Script.m_pPlayer->m_Property.m_Name;
		//	memcpy(pMsgAll.m_Notice.Name,name.c_str(),name.size());
		//	std::string Bname = IDName;
		//	memcpy(pMsgAll.m_Notice.ItemIDName,Bname.c_str(),Bname.size());
		//	//g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex,&pMsgAll,sizeof(SABlessAllServerNoticeMsg));
		//	extern void BroadcastAllBlessMsg(SMessage *pMsg);
		//	BroadcastAllBlessMsg(&pMsgAll);
		//	
		//}
		//全副通告
	}
	return 1;
}
int CScriptManager::L_OnPlayerBlessTime(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	g_Script.m_pPlayer->OnPlayerBlessTime();
	return 1;
}
int CScriptManager::L_GetPlayerMoney(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	long type  = static_cast<long>(lua_tonumber(L,1));
	lua_pushnumber(L,g_Script.m_pPlayer->GetPlayerMoney(type));
	return 1;
}
int CScriptManager::L_ChangeMoney(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	long type  = static_cast<long>(lua_tonumber(L,1));
	long value = static_cast<long>(lua_tonumber(L,2));
	g_Script.m_pPlayer->ChangeMoney(type,value);
	return 1;
}
int CScriptManager::L_OnErrorStopBless(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	BYTE type  = static_cast<BYTE>(lua_tonumber(L,1));
	SABlessStopBless pMsg;
	pMsg.BlessStoptype = type;
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex,&pMsg,sizeof(SABlessStopBless));
	return 1;
}
int CScriptManager::L_IsHaveSpaceCell(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	lua_pushnumber(L,g_Script.m_pPlayer->IsHaveSpaceCell());
	return 1;
}
int CScriptManager::L_AddPlayerRoseRecod( lua_State* L )
{
	if (!g_Script.m_pPlayer)return 0;

	return 1;
}
int CScriptManager::L_GetNowTime( lua_State* L )
{
	if (!g_Script.m_pPlayer)return 0;
	INT64 NowTime = g_Script.m_pPlayer->_L_GetNowTime();
	lua_pushnumber(L,NowTime);
	return 1;
}

int CScriptManager::L_giveItemList( lua_State* L )
{
	std::list<SAddItemInfo> itemList;
	if (lua_istable(L,1))
	{
		while(lua_gettop(L))
		{
			switch (lua_type( L, lua_gettop( L ) )) 
			{
			case LUA_TTABLE:
				{
					lua_pushnil(L);
					int num = -2;
					DWORD itemid = 0xffffffff;
					WORD itemnum = 0xffff;
					while (lua_next(L, num) != 0)
					{
						/* 'key' (索引-2) 和 'value' (索引-1) */  
						const char* key = "unknown";  
						const char* value;  
						if(lua_type(L, -2) == LUA_TSTRING)  
						{  
							key = lua_tostring(L, -2);  
							value = lua_tostring(L, -1);  
						}  
						else if(lua_type(L, -2) == LUA_TNUMBER)  
						{  
							// 因为lua_tostring会更改栈上的元素，  
							// 所以不能直接在key上进行lua_tostring  
							// 因此，复制一个key，压入栈顶，进行lua_tostring  
							lua_pushvalue(L, -2);  
							key = lua_tostring(L, -1);  
							lua_pop(L, 1);  
							value = lua_tostring(L, -1);  
						}  
						else  
						{  
							value = lua_tostring(L, -1);  
						}  
						if (atoi(key)%2 != 0){
							itemid = atol(value);
						}
						else{
							itemnum = atol(value);
						}

						if (itemid != 0xffffffff && itemnum != 0xffff)
						{
							itemList.push_back(SAddItemInfo(itemid, itemnum));
							itemid = 0xffffffff;
							itemnum = 0xffff;
						}
						/* 移除 'value' ；保留 'key' 做下一次迭代 */  
						lua_pop(L, 1);
					}
				}
			}
			lua_pop( L, 1 );
		}		
	}
	if (!itemList.empty()&&g_Script.m_pPlayer->CanAddItems(itemList))
	{
		g_Script.m_pPlayer->StartAddItems(itemList);
		lua_pushnumber(L,1);
		return 1;
	}
	lua_pushnumber(L,0);
	return 1;
}

int CScriptManager::L_HuiShenDanRecall(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD MaxUseTimes = static_cast<WORD>(lua_tonumber(L, 1));			// 每天使用的回神丹最大使用次数
	WORD MaxUpdateTimes = static_cast<WORD>(lua_tonumber(L, 2));		// 每颗回神丹最大升级次数

	MY_ASSERT(MaxUseTimes >= 0 && MaxUseTimes >= 0);

	g_Script.m_pPlayer->m_MaxUpdateTimes = MaxUpdateTimes;
	g_Script.m_pPlayer->m_MaxUseTimes = MaxUseTimes;

	// 回调函数
	g_Script.m_pPlayer->BeginUpdateItem();

	return 1;
}

int CScriptManager::L_ModifyPlayerAttributes(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	// [2012-9-20 13-03 gw: +表数据读取]
	if (!lua_istable(L,1))	
		return 0;	
	BYTE type = 0;
	WORD value = 0;
	while(lua_gettop(L))
	{
		if (LUA_TTABLE == lua_type( L, lua_gettop( L ) )) 
		{
			int num = lua_gettop(L);								
			lua_pushnil(L);			
			while (lua_next(L, num) != 0)
			{							
				if(lua_type(L, -2) == LUA_TNUMBER)  
				{  
					// 获取有效数据
					int key = lua_tonumber(L, -2);  
					int tmp = lua_tonumber(L, -1);  
					if (0 != (key & 0x1))
					{
						type = (BYTE)tmp; // 读取类型
					}
					else 
					{
						value = (WORD)tmp; // 读取value
						if (type != 0xff && value != 0)
						{
							MY_ASSERT(type >= SEquipDataEx::EEA_MAXHP && type < SEquipDataEx::EEA_MAX && value > 0);	
							if (type >= 0 && type < SEquipDataEx::EEA_MAX && value > 0)
							{
								g_Script.m_pPlayer->m_EquipFactorValue[type] += value;			
							}			
						}						
						type = 0xff;
					}						
				}  
				/* 移除 'value' ；保留 'key' 做下一次迭代 */  
				lua_pop(L, 1);
			}
		}
		lua_pop( L, 1 );
	}			

	return 1;
}

int CScriptManager::L_OnProcessNotice(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	long Result = g_Script.m_pPlayer->m_Property.m_BlessSPackageitem.wIndex;
	const SItemBaseData * psi = CItemService::GetInstance().GetItemBaseData(Result);
	if (psi)
	{
		long IDd = psi->m_ID;
		char chr[20];
		memset(chr,0,20);
		itoa(IDd,chr,10);
		std::string IDName = chr;
		GetApp()->m_pGameWorld->AddGlobalBlessInfo(g_Script.m_pPlayer->m_Property.m_Name,IDName);
		SABlessAllServerNoticeMsg pMsgAll;
		std::string name = g_Script.m_pPlayer->m_Property.m_Name;
		memcpy(pMsgAll.m_Notice.Name,name.c_str(),name.size());
		std::string Bname = IDName;
		pMsgAll.m_Notice.Item.wIndex = IDd;
		//memcpy(pMsgAll.m_Notice.ItemIDName,Bname.c_str(),Bname.size());
		//g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex,&pMsgAll,sizeof(SABlessAllServerNoticeMsg));
		extern void BroadcastAllBlessMsg(SABlessAllServerNoticeMsg *pMsgAll);
		BroadcastAllBlessMsg(&pMsgAll);

	}
	//全副通告
	return 1;
}

int CScriptManager::L_getplayerSkillproperty( lua_State *L )
{
	if (!g_Script.m_pPlayer)
		return 0;

	WORD skillIndex = static_cast<WORD>(lua_tonumber(L, 1));			// 技能索引
	BYTE skillType = static_cast<BYTE>(lua_tonumber(L, 2));				// 技能ID还是技能等级,0获取技能ID，1技能等级
	if (skillIndex<MAX_SKILLCOUNT&& skillType<2)
	{
		WORD skilldata = 0;
		if (skillType == 0){
			skilldata = g_Script.m_pPlayer->m_Property.m_pSkills[skillIndex].byLevel;
		}
		else{
			skilldata = g_Script.m_pPlayer->m_Property.m_pSkills[skillIndex].wTypeID;
		}
		lua_pushnumber(L,skilldata);
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendGiftSucceed( lua_State* L )
{
	if (!g_Script.m_pPlayer)
		return 0;
	BYTE bType = static_cast<BYTE>(lua_tonumber(L, 1));			//礼包类型
	BYTE bResult = static_cast<BYTE>(lua_tonumber(L, 2));//是否成功
	BYTE index = static_cast<BYTE>(lua_tonumber(L, 3));
	g_Script.m_pPlayer->SentClinetGiftSucceed(bType,bResult,index);
	return 1;
}
int CScriptManager::L_SendTemplateSkill(lua_State* L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	g_Script.m_pPlayer->SendTemplateSkill(0,0);
	return 1;
}
int CScriptManager::L_GetEventID(lua_State* L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (g_Script.m_pPlayer->m_ParentRegion == 0)
	{
		return 0;
	}
	CDynamicRegion *pRegionD = (CDynamicRegion *)g_Script.m_pPlayer->m_ParentRegion->DynamicCast(IID_DYNAMICREGION);
	if (pRegionD)
	{
		lua_pushnumber(L,pRegionD->GetGID());
	}
	else
	{
		lua_pushnumber(L,0);
	}
	return 1;
}
int CScriptManager::L_RFalse(lua_State *L)
{
	LPCSTR msg = static_cast<const char *>(lua_tostring(L, 1));
	if ( msg == NULL )
		msg = "(null)";

	rfalse(4, 1, msg);
	return 1;
}


int CScriptManager::L_GetStartServerTime(lua_State *L)
{
	//服务器启动时间
	lua_pushnumber(L, g_StartUpZoneServerTime);
	return 1;
}

int CScriptManager::L_setmonsterdata(lua_State *L)
{
	CMonster *Pmonster = 0;
	WORD wIndex = 0;
	DWORD value = 0;
	WORD wparamNum = lua_gettop(L);
	if (wparamNum == 3)
	{
		DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
		 wIndex = static_cast<WORD>(lua_tonumber(L, 2));
		 value = static_cast<DWORD>(lua_tonumber(L, 3));
		Pmonster = (CMonster*)GetObjectByGID(gid)->DynamicCast(IID_MONSTER);
	}
	else
	{
		Pmonster = g_Script.m_pMonster;
		wIndex = static_cast<WORD>(lua_tonumber(L, 1));
		value = static_cast<DWORD>(lua_tonumber(L, 2));
	}

	if (!Pmonster)
	{
		return 0;
	}
	switch (wIndex)
	{
	case 9:
		Pmonster->m_dwLastEnemyID = value;
		break;
	case 17:
		{
		Pmonster->SetCurActionID((EActionState)value);
		if (value == 7)
		{
			Pmonster->m_CurHp = 0;
		}
		}
		break;
	case 6:
		Pmonster->m_CurHp = value;
		break;
	case 19:
		Pmonster->m_BackProtection = value;
		break;
	default:
		break;
	}
	return 1;
}
int CScriptManager::L_LuaRandRange(lua_State* L)
{
	WORD lowerV = static_cast<WORD>(lua_tonumber(L, 1));
	WORD upperV = static_cast<WORD>(lua_tonumber(L, 2));
	int Result = CRandom::RandRange(lowerV, upperV);
	lua_pushnumber(L,Result);
	return 1;
}
void CScriptManager::SetMonster( class CMonster *pmonster )
{
	g_Script.m_pMonster = pmonster;
}

void CScriptManager::CleanMonster()
{
	g_Script.m_pMonster = 0;
}

int CScriptManager::L_SetCrilial(lua_State* L)
{
	if (!g_Script.m_pAttacker)
	{
		return 0;
	}
	WORD value = static_cast<WORD>(lua_tonumber(L, 1));
	g_Script.m_pAttacker->m_IsCrilial = value;
	return 1;
}

int CScriptManager::L_setactivityNotice( lua_State *L )
{
	if (!lua_istable(L, 1))return 0;

	ActivityNotice actotice;

	actotice.ActivityReward[0] =  static_cast<DWORD>(lua_tonumber(L, 2));
	actotice.ActivityReward[1] =  static_cast<DWORD>(lua_tonumber(L, 3));

	LPCSTR ActivityName =  static_cast<LPCSTR>(GET_FIELD_STRING_DIRECTLY(1, "ActivityName"));
	LPCSTR Activitydescribe	=  static_cast<LPCSTR>(GET_FIELD_STRING_DIRECTLY(1, "Activitydescribe"));
	LPCSTR ActivityNpcName  =  static_cast<LPCSTR>(GET_FIELD_STRING_DIRECTLY(1, "ActivityNpcName"));

	dwt::strcpy(actotice.ActivityName,ActivityName,10);
	dwt::strcpy(actotice.Activitydescribe,Activitydescribe,50);
	dwt::strcpy(actotice.ActivityNpcName,ActivityNpcName,10);

	actotice.ActivityDate	= static_cast<DWORD>(GET_FIELD_NUMBER_DIRECTLY(1, "ActivityDate"));
	actotice.ActivityType =  static_cast<BYTE>(GET_FIELD_NUMBER_DIRECTLY(1, "ActivityType"));

	actotice.ActivityStartTime =  static_cast<WORD>(GET_FIELD_NUMBER_DIRECTLY(1, "ActivityStartTime"));
	actotice.ActivityEntryRegin =  static_cast<WORD>(GET_FIELD_NUMBER_DIRECTLY(1, "ActivityEntryRegin"));
	actotice.ActivityEntryCoordX =  static_cast<WORD>(GET_FIELD_NUMBER_DIRECTLY(1, "ActivityEntryCoordX"));
	actotice.ActivityEntryCoordY =  static_cast<WORD>(GET_FIELD_NUMBER_DIRECTLY(1, "ActivityEntryCoordY"));


	GetGW()->SetActivityNotice(actotice);
	return 1;
}
int CScriptManager::L_LoadProtectInfo(lua_State * L)
{
	WORD	Index = static_cast<WORD>(lua_tonumber(L, 1));
	DWORD	ItemID = static_cast<DWORD>(lua_tonumber(L, 2));
	BYTE	MoneyType = static_cast<BYTE>(lua_tonumber(L, 3));
	long	Money = static_cast<long>(lua_tonumber(L, 4));
	CProtectInfo::GetInstance().LoadProtectInfo(Index, ItemID, MoneyType, Money);
	return 1;
}

int CScriptManager::L_CheckPlayerMoney( lua_State* L )
{
	if (!g_Script.m_pPlayer)return 0;

	BYTE bType = static_cast<BYTE>(lua_tonumber(L, 1));			//金钱类型
	DWORD wnum = static_cast<DWORD>(lua_tonumber(L, 2));			//金钱数量
	BYTE bOnlyCheck = static_cast<BYTE>(lua_tonumber(L, 3));			//是否扣除
	BYTE bResult = g_Script.m_pPlayer->CheckPlayerMoney(bType,wnum,bOnlyCheck);
	lua_pushnumber(L,bResult);
	return 1;
}
//获取玩家相关时间，0--上次登录时间，1--上次离线时间，2--本次登录时间
int CScriptManager::L_GetPlayerTime( lua_State* L )
{
	BYTE bType = static_cast<BYTE>(lua_tonumber(L, 1));			//时间类型
	if (bType == 0){
		if (g_Script.m_pPlayer)
		{
			lua_pushnumber(L,g_Script.m_pPlayer->_L_GetLastLoginTime());
		}
		return 1;
	}
	else if (bType == 1){
		if (g_Script.m_pPlayer)
		{
			lua_pushnumber(L,g_Script.m_pPlayer->_L_GetLastLeaveTime());
		}
		return 1;
	}
	else if (bType == 2){
		if (g_Script.m_pPlayer)
		{
			lua_pushnumber(L,g_Script.m_pPlayer->_L_GetCurrentLoginTime());
		}
		return 1;
	}
	else if (bType == 3){
		INT64 nowtime= 0;
		_time64(&nowtime);
		lua_pushnumber(L,nowtime);
		return 1;
	}
	return 0;
}

int CScriptManager::L_SetPlayerVipLevel( lua_State* L )
{
	if (!g_Script.m_pPlayer)return 0;
	BYTE bLevel = static_cast<BYTE>(lua_tonumber(L, 1));			//等级
	g_Script.m_pPlayer->_L_SetPlayerVipLevel(bLevel);
	return 1;
}


int CScriptManager::L_ResetCountDownGiftState( lua_State* L )
{
	if (!g_Script.m_pPlayer)return 0;	
	g_Script.m_pPlayer->_L_ResetCountDownGiftState();
	return 1;
}

int CScriptManager::L_ResetOnlineGiftState(lua_State* L)
{
	if(!g_Script.m_pPlayer)return 0;
	g_Script.m_pPlayer->_L_ResetOnlineGiftState();
	return 1;
}

int CScriptManager::L_ResetCountDownGiftTime(lua_State* L)
{
	if (!g_Script.m_pPlayer) return 0;
	g_Script.m_pPlayer->_L_ResetCountDownGiftTime();
	return 1;
}

int CScriptManager::L_GetCountDownGiftBeginTime(lua_State* L)
{
	if (!g_Script.m_pPlayer)return 0;
	INT64 countDownBeginTime = g_Script.m_pPlayer->_L_GetCountDownGiftBeginTime();
	lua_pushnumber(L,countDownBeginTime);
	return 1;
}

int CScriptManager::L_OnPartEnterNum(lua_State * L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	DWORD RegionID = static_cast<DWORD>(lua_tonumber(L,1));
	BYTE  numLimit = static_cast<BYTE>(lua_tonumber(L,2));
	lua_pushnumber(L,g_Script.m_pPlayer->OnPartEnterNum(RegionID,numLimit));
	return 1;
}

int CScriptManager::L_OnTeamPartEnterNum(lua_State * L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	DWORD RegionID = static_cast<DWORD>(lua_tonumber(L,1));
	BYTE  numLimit = static_cast<BYTE>(lua_tonumber(L,2));
	lua_pushnumber(L,g_Script.m_pPlayer->OnTeamPartEnterNum(RegionID,numLimit));
	return 1;
}

int CScriptManager::L_IsPersonalEnterPart(lua_State * L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	DWORD RegionID = static_cast<DWORD>(lua_tonumber(L,1));
	lua_pushnumber(L,g_Script.m_pPlayer->IsPersonalEnterPart(RegionID));
	return 1;
}

int CScriptManager::L_IsTeamEnterPart(lua_State * L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	DWORD RegionID = static_cast<DWORD>(lua_tonumber(L,1));
	lua_pushnumber(L,g_Script.m_pPlayer->IsTeamEnterPart(RegionID));
	return 1;
}
int CScriptManager::L_IsNonTeamPartEnter(lua_State * L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	lua_pushnumber(L,g_Script.m_pPlayer->IsNonTeamPartEnter(g_Script.m_pPlayer->m_PartDyRegionGID));
	return 1;
}


int CScriptManager::L_GetLiveMonster( lua_State *L )
{
	DWORD	Index = static_cast<DWORD>(lua_tonumber(L, 1));
	CDynamicScene *regin = (CDynamicScene*)FindRegionByGID(Index)->DynamicCast(IID_DYNAMICSCENE);
	if (regin){
		lua_pushnumber(L,regin->GetLiveMonster(0));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SetGmakeParm(lua_State * L)
{
	if (!g_Script.m_pMonster)
	{
		return 0;
	}
	BYTE index = static_cast<BYTE>(lua_tonumber(L,1));
	BYTE value = static_cast<BYTE>(lua_tonumber(L,2));
	switch (index)
	{
	case 1:
		g_Script.m_pMonster->m_gmakeparm1 = value;
		break;
	case 2:
		g_Script.m_pMonster->m_gmakeparm2 = value;
		break;
	case 3:
		g_Script.m_pMonster->m_gmakeparm3 = value;
		break;
	case 4:
		g_Script.m_pMonster->m_gmakeparm4 = value;
		break;
	default:
		break;

	}
	return 1;
}

int CScriptManager::L_GetGmakeParm(lua_State * L)
{
	if (!g_Script.m_pMonster)
	{
		return 0;
	}
	BYTE index = static_cast<BYTE>(lua_tonumber(L,1));
	switch (index)
	{
	case 1:
		lua_pushnumber(L,g_Script.m_pMonster->m_gmakeparm1);
		break;
	case 2:
		lua_pushnumber(L,g_Script.m_pMonster->m_gmakeparm2);
		break;
	case 3:
		lua_pushnumber(L,g_Script.m_pMonster->m_gmakeparm3);
		break;
	case 4:
		lua_pushnumber(L,g_Script.m_pMonster->m_gmakeparm4);
		break;
	default:
		break;
	}
	return 1;
}

int CScriptManager::L_SetJingMaiShareExpTimes(lua_State * L)
{
	if (!g_Script.m_pPlayer)
		return 0;
	BYTE byShareExpTimes = static_cast<BYTE>(lua_tonumber(L,1));
	g_Script.m_pPlayer->m_Property.SetJingMaiShareExpTimes(byShareExpTimes);

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int CScriptManager::OnEveryDayManagerRun()
{
	lite::Variant ret;
	g_Script.SetCondition( NULL, NULL, NULL );
	LuaFunctor( g_Script, "EveryDayManagerRun" )(&ret);
	g_Script.CleanCondition();

	//返回重置序号：当前那个日常可以重置了
	return (int)ret;
}

int CScriptManager::L_ReadEveryDayManagerTime(lua_State *L)
{
	lua_pushnumber(L, g_Script.m_EveryDayManagerTime);
	return 1;
}

int CScriptManager::L_WriteEveryDayManagerTime(lua_State *L)
{
	g_Script.m_EveryDayManagerTime = static_cast<INT64>(lua_tonumber(L,1));
	return 1;
}


int CScriptManager::L_MystringFormat( lua_State * L )
{
	int luatype = lua_type(L,1);
	if (luatype != LUA_TSTRING)
	{
		return 0;
	}
	const char *formatstr = lua_tostring(L,1);
	if (!CMyString::GetInstance().GetFormatString(formatstr))
	{
		lua_pushnumber(L,0);
		rfalse(1,1,"%s 字符串没有找到",formatstr);
		return 1;
	}
	std::vector<lite::Variant> vlvariant;
	int paramlen = lua_gettop(L);
	if (paramlen > 1)
	{
		int nIndex= 2;
		while(nIndex <= paramlen)
		{
			int type = lua_type(L,nIndex);
			if (lua_type(L, nIndex) == LUA_TSTRING)
			{
				const char *param = lua_tostring(L,nIndex);
				vlvariant.push_back(lite::Variant(param));
			}
			else if(lua_type(L, nIndex) == LUA_TNUMBER)
			{
				int nparam = lua_tonumber(L,nIndex);
				vlvariant.push_back(lite::Variant(nparam));
			}
			nIndex++;
		}
	}
	std::string sformatstring;
	if ( CMyString::GetInstance().MyStringFormat(formatstr,vlvariant,sformatstring))
	{
		//AddInfo(sformatstring.c_str());
		lua_pushstring(L,sformatstring.c_str());
		return 1;
	}
	return 0;
}

int CScriptManager::L_logtable( lua_State * L )
{
	if (lua_istable(L,1))
	{
		std::vector<lite::Variant> vlvariant;
		lua_gettable(L,1);
		int ntablelen = lua_gettop(L);
		for(int i = 1;i<=ntablelen;i++)
		{
			if (lua_isnumber(L,i))
			{
				int num = lua_tonumber(L,i);
				vlvariant.push_back(lite::Variant(num));
			}
			else if (lua_isstring(L,i))
			{
				const char* str = lua_tostring(L,i);
				vlvariant.push_back(lite::Variant(str));
			}
			else
			{
				const char* str = lua_typename(L,i);
				AddInfo(str);
			}
		}
	}

	return 0;
}

int CScriptManager::L_SavePlayerData( lua_State* L )
{
	if (!g_Script.m_pPlayer)return 0;
	int nlen = lua_gettop(L);
	if (nlen != 2)return 0;

	if (!lua_isstring(L,1))
	{
		rfalse(2,1,"L_SavePlayerData NO Name");
		return 0;
	}
	if (!lua_istable(L,2))
	{
		rfalse(2,1,"L_SavePlayerData NO Table");
		return 0;
	}
	const char* tablename = static_cast<const char *>(lua_tostring(L,1));
	//WORD	tablesize = lua_tonumber(L,3);
	std::vector<lite::Variant> vlvariant;
	std::vector<lite::Variant > keyvariant;


	if (GetTableData(L,keyvariant,vlvariant) == 0)
	{
		return 0;
	}
	// 	lua_pushnil(L);
	// 	while(lua_next(L,-2) != 0)
	// 	{
	// 		const char* key ="" ;
	// 		const char* strparam ="";
	// 
	// 		int nkey = 0;
	// 		int nparam = 0;
	// 
	// 		if (lua_type(L, -2) == LUA_TSTRING)
	// 		{
	// 			key = lua_tostring(L,-2);
	// 			keyvariant.push_back(lite::Variant(key));
	// 		}
	// 		else if(lua_type(L, -2) == LUA_TNUMBER)
	// 		{
	// 			nkey = lua_tonumber(L,-2);
	// 			keyvariant.push_back(lite::Variant(nkey));
	// 		}
	// 		else
	// 		{
	// 			rfalse(2,1,"key %s",lua_typename(L,-2));
	// 		}
	// 
	// 		if (lua_type(L, -1) == LUA_TSTRING)
	// 		{
	// 			strparam = lua_tostring(L,-1);
	// 			vlvariant.push_back(lite::Variant(strparam));
	// 		}
	// 		else if(lua_type(L, -1) == LUA_TNUMBER)
	// 		{
	// 			nparam = lua_tonumber(L,-1);
	// 			vlvariant.push_back(lite::Variant(nparam));
	// 		}
	// 		else
	// 		{
	// 			rfalse(2,1,"param %s",lua_typename(L,-1));
	// 		}
	// 		lua_pop( L, 1 );
	// 	}
	int nsaveindex = findDataPos(tablename,true);
	if (nsaveindex >= 0)
	{
		stLuaData sluadata;
		strcpy(sluadata.cstrDataKey,tablename);
		BYTE *pBuffer = g_Script.m_pPlayer->m_Property.m_bluaBuffer;
		int nAllBufferLen = (int)*pBuffer;


		pBuffer += nsaveindex;
		BYTE	*pDate = pBuffer;
		pDate += sizeof(stLuaData);
		int i =0;
		//MY_ASSERT(vlvariant.size() == keyvariant.size());
		int nvsize = vlvariant.size();
		nvsize = keyvariant.size();
		for(int i = 0;i < vlvariant.size();i++)
		{
			if (keyvariant[i].dataType == lite::Variant::VT_STRING)		//保存key
			{
				std::string strkey = keyvariant[i];
				memcpy(pDate,strkey.c_str(),strkey.length());
				pDate += strkey.length();

				*pDate = '\0';
				pDate++;
			}
			else if (keyvariant[i].dataType == lite::Variant::VT_I32_24)
			{
				char chrkey[10]={};
				int nkey = (int)keyvariant[i];
				itoa(nkey,chrkey,10);

				std::string strkey(chrkey);
				memcpy(pDate,strkey.c_str(),strkey.length());
				pDate += strkey.length();
				*pDate = '\0';
				pDate++;
			}

			if (vlvariant[i].dataType != lite::Variant::VT_I32_24 )		//保存value
			{
				rfalse("value type != number");
				return -1;
			}
			int ndata =vlvariant[i];
			memcpy(pDate,&ndata,sizeof(int));
			pDate += sizeof(int);
		}
		sluadata.wDataLen = int(pDate - pBuffer) - sizeof(stLuaData);
		memcpy(pBuffer,&sluadata,sizeof(stLuaData));
		UpdateLenth();
		lua_pushnumber(L,1);
		return 1;
	}
	return 0;
}

int CScriptManager::findDataPos(const char* tablename,bool bissave )//是保存数据还是读取数据
{
	if (!g_Script.m_pPlayer)return -1;
	if(!tablename)return -1;
	BYTE *pBuffer = g_Script.m_pPlayer->m_Property.m_bluaBuffer;
	BYTE *pBufStart = pBuffer;

	int nPlayerDLen = (int)*pBuffer;
	if (nPlayerDLen == 0)return bissave ? sizeof(int) : -1;
	pBuffer += sizeof(int);

	int nAlldatalen =  sizeof(int);
	int ndatalen = 0;
	BYTE	*pSaveDate = 0;
	stLuaData sluadata;
	while(1)
	{
		memset(&sluadata,0,sizeof(stLuaData));
		memcpy(&sluadata,pBuffer,sizeof(stLuaData));
		if (sluadata.wDataLen > 0)
		{
			if (dwt::strcmp(tablename,sluadata.cstrDataKey,30)== 0)
			{
				if (!bissave)
				{
					return int(pBuffer -pBufStart);		//如果是读取数据直接返回指针偏移位置
				}
				pSaveDate = pBuffer;		//查找到的原来数据指针
				break;
			}
			ndatalen = sluadata.wDataLen;
			pBuffer +=sizeof(stLuaData) + ndatalen;				//存储为一个struct + 实际存储长度
			nAlldatalen += sizeof(stLuaData) + ndatalen;
		}
		else
		{
			return bissave ? int(pBuffer -pBufStart) : -1;			//如果是保存数据，没有查找到相关的table名字，返回一个可以存储的指针偏移位置
		}

		if (nAlldatalen >LUABUFFERSIZE)
		{
			rfalse(2,1,"data to over");
			return -2;
		}
	}
	if (pSaveDate&&bissave)	//因为存储可能会在大小上进行变化 ,所以直接把数据提前
	{

		int nsrelen = nAlldatalen;
		ndatalen = sluadata.wDataLen;
		pBuffer +=sizeof(stLuaData) + ndatalen;		
		nAlldatalen += sizeof(stLuaData) + ndatalen;		//原始数据的大小位置


		int ncopylen = (int)*pBufStart;		//所有数据的大小
		//int ncopylen = getDataLenth();			//所有数据的大小
		if (ncopylen >= nAlldatalen)			//所有数据的大小必定大于原始位置数据大小
		{
			BYTE *pnewdata = new BYTE[ncopylen];	//把后面的数据复制衣服
			memset(pnewdata,0,ncopylen);

			ncopylen -= nAlldatalen;		//取得后面的数据的长度
			memcpy(pnewdata,pBuffer,ncopylen);

			memset(pSaveDate,0,(ncopylen +sizeof(stLuaData) + ndatalen));	//先把原始位置数据后面全部清空
			memcpy(pSaveDate,pnewdata,ncopylen );		//把后面的数据拷贝到前面

			pSaveDate += ncopylen;

			UpdateLenth();//更新数据头数据长度
			pBuffer = g_Script.m_pPlayer->m_Property.m_bluaBuffer;
			return int(pSaveDate - pBuffer);	//返回指针偏移位置
		}
	}
	return -1;
}

int CScriptManager::L_LoadPlayerData( lua_State* L )
{
	// 	if (lua_isstring(L,1))
	// 	{
	// 		const char* straccount = static_cast<const char *>(lua_tostring(L,1));
	// 		CPlayer *dst = ( CPlayer* )GetPlayerByAccount( straccount )->DynamicCast( IID_PLAYER ); 
	// 		g_Script.m_pPlayer = dst;
	// 	}
	if (!g_Script.m_pPlayer)return 0;
	if (!lua_isstring(L,1))
	{
		rfalse(2,1,"no table name");
		return 0;
	}
	const char* tablename = static_cast<const char *>(lua_tostring(L,1));
	int nsaveindex = findDataPos(tablename,false);
	if (nsaveindex >= 0)
	{
		BYTE *pBuffer = g_Script.m_pPlayer->m_Property.m_bluaBuffer;
		pBuffer += nsaveindex;
		BYTE *pDate = pBuffer;
		pDate += sizeof(stLuaData);
		stLuaData sluadata;
		memcpy(&sluadata,pBuffer,sizeof(stLuaData));
		if (sluadata.wDataLen == 0)
		{
			rfalse(2,1,"没有找到数据");
			return -1;
		}
		char strkey[20]={};
		lua_createtable(L,sluadata.wDataLen,0);
		while(1)
		{
			BYTE *pfind = (BYTE*)memchr(pDate,'\0',sluadata.wDataLen);
			if (!pfind)
			{
				rfalse(2,1,"key 没有找到");
				return -1;
			}
			int nresult = pfind -  pDate + 1;
			dwt::strcpy(strkey,(char*)pDate,nresult);
			lua_pushstring(L,strkey);
			pDate += nresult;

			int ndata =0;
			memcpy(&ndata,pDate,sizeof(int));
			lua_pushnumber(L,ndata);
			pDate += sizeof(int);

			lua_settable( L, -3 );
			int ncurent = (int)(pDate - pBuffer) - sizeof(stLuaData);
			if (ncurent >= sluadata.wDataLen)
			{
				return 1;
				break;
			}
		}
	}
	return 0;
}

int CScriptManager::getDataLenth()
{
	if (!g_Script.m_pPlayer)return -1;
	BYTE *pBuffer = g_Script.m_pPlayer->m_Property.m_bluaBuffer;
	int nAlldatalen = 0;
	int ndatalen = 0;
	stLuaData sluadata;
	while(1)
	{
		memset(&sluadata,0,sizeof(stLuaData));
		memcpy(&sluadata,pBuffer,sizeof(stLuaData));
		if (sluadata.wDataLen > 0)
		{
			ndatalen = sluadata.wDataLen;
			pBuffer +=sizeof(stLuaData) + ndatalen;				
			nAlldatalen += sizeof(stLuaData) + ndatalen;
		}
		else
		{
			return 	nAlldatalen;
		}

		if (nAlldatalen >LUABUFFERSIZE)
		{
			rfalse(2,1,"data to over");
			return -2;
		}
	}
	return -1;
}

int CScriptManager::UpdateLenth()
{
	if (!g_Script.m_pPlayer)return -1;
	BYTE *pBuffer = g_Script.m_pPlayer->m_Property.m_bluaBuffer;
	int npallLen = (int)*pBuffer;
	pBuffer += sizeof(int);

	int nAlldatalen =  sizeof(int);
	int ndatalen = 0;
	stLuaData sluadata;
	while(1)
	{
		memset(&sluadata,0,sizeof(stLuaData));
		memcpy(&sluadata,pBuffer,sizeof(stLuaData));
		if (sluadata.wDataLen > 0)
		{
			ndatalen = sluadata.wDataLen;
			pBuffer +=sizeof(stLuaData) + ndatalen;				
			nAlldatalen += sizeof(stLuaData) + ndatalen;
		}
		else
		{
			pBuffer = g_Script.m_pPlayer->m_Property.m_bluaBuffer;
			memcpy(pBuffer,&nAlldatalen,sizeof(int));
			return 	nAlldatalen;
		}

		if (nAlldatalen >LUABUFFERSIZE)
		{
			rfalse(2,1,"data to over");
			return -2;
		}
	}
	return -1;
}

int CScriptManager::L_SendPlayerReward( lua_State* L )
{
	if (!g_Script.m_pPlayer)
	{
		rfalse(2,1,"Player null");
		return 0;
	}
	if(!lua_isnumber(L,1) || !lua_istable(L,2))
	{
		rfalse(1,1,"SendPlayerReward 参数错误");
		return 0;
	}
	BYTE type = (BYTE)lua_tonumber(L,1);
	std::vector<lite::Variant> vlvariant;
	std::vector<lite::Variant > keyvariant;
	if (GetTableData(L,keyvariant,vlvariant))
	{
		SASendReward sendreward;
		sendreward.bRewardtype = type;
		sendreward.wRewardNum = vlvariant.size();
		sendreward.nRewardData = 0;
		for(SIZE_T i = 0;i<vlvariant.size();i++)
		{
			BYTE bState = (int)vlvariant[i] - 1;
			sendreward.nRewardData = MakeRewardData(sendreward.nRewardData,bState,i);
		}
		int msgsize = sizeof(SASendReward);
		g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex,&sendreward,msgsize);
		return 1;
	}
	return 0;
}

int CScriptManager::GetTableData(lua_State* L,std::vector<lite::Variant > &keyvariant,std::vector<lite::Variant> &vlvariant )
{
	int nlen = lua_gettop(L);
	lua_pushnil(L);
	while(lua_next(L,-2) != 0)
	{
		const char* key ="" ;
		const char* strparam ="";

		int nkey = 0;
		int nparam = 0;

		if (lua_type(L, -2) == LUA_TSTRING)
		{
			key = lua_tostring(L,-2);
			keyvariant.push_back(lite::Variant(key));
		}
		else if(lua_type(L, -2) == LUA_TNUMBER)
		{
			nkey = lua_tonumber(L,-2);
			keyvariant.push_back(lite::Variant(nkey));
		}
		else
		{
			rfalse(2,1,"key %s",lua_typename(L,-2));
			return 0;
		}

		if (lua_type(L, -1) == LUA_TSTRING)
		{
			strparam = lua_tostring(L,-1);
			vlvariant.push_back(lite::Variant(strparam));
		}
		else if(lua_type(L, -1) == LUA_TNUMBER)
		{
			nparam = lua_tonumber(L,-1);
			vlvariant.push_back(lite::Variant(nparam));
		}
		else if (lua_type(L, -1) == LUA_TTABLE)
		{
			GetTableData(L,keyvariant,vlvariant);
		}
		else
		{
			rfalse(2,1,"value %s",lua_typename(L,-1));
			return 0;
		}
		lua_pop( L, 1 );
	}
	return 1;
}

int CScriptManager::MakeRewardData( int nReward,BYTE bState,BYTE blocation )
{
	int nrewardstate = bState << (blocation * 2); //每次移动2
	return nReward | nrewardstate;
}

int CScriptManager::L_GetActivePackageNum( lua_State *L )
{
	if (!g_Script.m_pPlayer)
	{
		lua_pushboolean(L, 0);
		return 1;
	}
	lua_pushnumber(L,g_Script.m_pPlayer->m_Property.m_wExtGoodsActivedNum);
	return 1;
}

int CScriptManager::L_SavePlayerTable( lua_State* L )
{
	//if (!g_Script.m_pPlayer)return 0;
	CPlayer *player = g_Script.m_pPlayer;
	if (!player)
	{
		DWORD pygid =  static_cast<DWORD>(lua_tonumber(L,3));
		player = (CPlayer*)GetPlayerByGID(pygid)->DynamicCast(IID_PLAYER);
		if (!player)
		{
			player = (CPlayer*)GetPlayerBySID(pygid)->DynamicCast(IID_PLAYER);
		}
	}
	if (!player)
	{
		return 0;
	}
	// 	int nlen = lua_gettop(L);
	// 	if (nlen != 2)return 0;

	if (!lua_isstring(L,1))
	{
		rfalse(2,1,"L_SavePlayerData NO Name");
		return 0;
	}
	if (!lua_istable(L,2))
	{
		rfalse(2,1,"L_SavePlayerData NO Table");
		return 0;
	}
	const char* tablename = static_cast<const char *>(lua_tostring(L, 1));
	if (tablename == NULL)
	{
		rfalse(2, 1, "talbename is null, and PlayerSid = %ld", player->GetSID());
		return 0;
	}
	BYTE *luabuffer = new BYTE[20480];
	//lua_pushnil(L);
	if ( lua_istable( L, 2 ) )
	{
		lua_pushvalue( L, 1 );
		//lua_rawget( L, -2 ); // key删除，被域结果替代

		int cur1 = luaEx_serialize( L, 2, luabuffer, 20480 );
		if ( cur1 < 0 )return 0;
		if (!player->SavePlayerTable(tablename,luabuffer,cur1))
		{
			if (luabuffer != NULL)
				delete[]luabuffer;
			return 0;
		}
	}
	if (luabuffer != NULL)
		delete[]luabuffer;
	//20150126 wk 增加第4个参数,1代表下线保存数据,且将dbMgr[sid]清空;dbMgr元方法为安全不能直接在lua层置空
	// 是否在更新后，清除lua数据
	if (lua_isnumber(L, 4))
	{
		BOOL doClean = (lua_tonumber(L, 4) == 1);

		lua_getglobal(L, "dbMgr");

		if (lua_istable(L, -1))
		{
			// 获取指定数据表
			lua_pushvalue(L, 3); //sid入栈
			lua_rawget(L, -2); // key删除，被域结果替代,取得dbMgr[sid]
			BOOL isTable = lua_istable(L, -1);
			lua_pop(L, 1); //弹出key对应的值,即dbMgr[sid],到dbMgr这一层
			if (doClean && isTable)//有dbMgr[sid]且清除数据
			{
				lua_pushvalue(L, 3);//sid入栈,key取得dbMgr[sid]
				lua_pushnil(L);//value入栈nil
				lua_rawset(L, -3);//gettable ,压到了-3位置 --dbMgr[sid]=nil
			}
		}
		lua_pop(L, 1);//弹出key对应的值,即dbMgr
	}

	return 1;
}

int CScriptManager::L_LoadPlayerTable( lua_State* L )
{
	CPlayer *player = g_Script.m_pPlayer;
	if (!player)
	{
		DWORD pygid =  static_cast<DWORD>(lua_tonumber(L,2));
		player = (CPlayer*)GetPlayerByGID(pygid)->DynamicCast(IID_PLAYER);
		if (!player)
		{
			player = (CPlayer*)GetPlayerBySID(pygid)->DynamicCast(IID_PLAYER);
		}
	}

	if (!player)
	{
		return 0;
	}

	if (!lua_isstring(L,1))
	{
		rfalse(2,1,"no table name");
		return 0;
	}

	const char* tablename = static_cast<const char *>(lua_tostring(L,1));
	if (tablename == NULL)
	{
		rfalse(2, 1, "talbename is null, and PlayerSid = %ld", player->GetSID());
		return 0;
	}
	int ndatasize = 0;
	BYTE *pdata=(BYTE*)player->LoadPlayerTable(tablename,ndatasize);
	if (pdata && ndatasize > 0)
	{
		lua_createtable(L,0,0);
		if ( !lua_istable( L, -1 ) )
		{
			return 0;
		}
		int ck = luaEx_unserialize( L,pdata ,ndatasize);
		if (ck <= 0)
		{
			lua_pushnil(L);
			return 0;
		}
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendSceneMsg( lua_State *L )
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	int paramcount = lua_gettop(L);
	if (paramcount != 3)
	{
		return 0;
	}
	SASceneInit  sceneinit;
	memset(sceneinit.bStarData,0,MAX_STAR_SIZE);
	BYTE *ptemp = sceneinit.bStarData;

	if (lua_isnumber(L,1))
	{
		sceneinit.wCurrentSceneMap = lua_tonumber(L,1);
	}

	if (lua_isnumber(L,2))
	{
		sceneinit.wCurrentScene = lua_tonumber(L,2);
	}

	if (sceneinit.wCurrentScene == 0 || sceneinit.wCurrentSceneMap == 0)
	{
		rfalse(2,1,"关卡或场景索引为0");
		return 0;
	}

	if (lua_istable(L,3))
	{
		int nsize =  luaL_getn(L, 3);
		int *tempstar = new int[nsize];
		for(int i = 0;i<nsize;i++)
		{
			if (GetTableArrayNumber(L,3,i+1,tempstar[i]))
			{
				memcpy(ptemp,&tempstar[i],sizeof(int));
				ptemp += sizeof(WORD);

				if (ptemp - sceneinit.bStarData > MAX_STAR_SIZE)
				{
					rfalse(2,1,"stardata to large");
					break;
				}
			}
		}
		SAFE_DELETE_ARRAY(tempstar);
	}
	/*int meglen = sizeof(SASceneInit) - MAX_STAR_SIZE + (ptemp - sceneinit.bStarData);*/
	int meglen = sizeof(SASceneInit);
	CSceneMap::GetInstance().SendSceneMsg(g_Script.m_pPlayer,&sceneinit,meglen);
	return 1;
}

int CScriptManager::L_SendSelectSceneMsg( lua_State *L )
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	SASceneSelect saselect;
	memset(saselect.bitemreward,0,MAX_IREWARD_SIZE);
	memset(saselect.bequip,0,MAX_IREWARD_SIZE);
	BYTE *pitemdata = saselect.bitemreward;
	BYTE *pequipdata = saselect.bequip;
	if (lua_istable(L,1))
	{
		GetTableNumber(L,1,"scenemapIndex",saselect.wscenemap);
		GetTableNumber(L,1,"wsceneIndex",saselect.wsceneIndex);
		GetTableNumber(L,1,"bsdtime",saselect.bsdtime);
		GetTableNumber(L,1,"bmaxsdtime",saselect.bmaxsdtime);
		GetTableNumber(L,1,"bcommontime",saselect.bcommontime);
		GetTableNumber(L,1,"bmaxcommontime",saselect.bmaxcommontime);
		GetTableNumber(L,1,"SDTime",saselect.SDTime);
		GetTableNumber(L,1,"bsplimit",saselect.bsplimit);
		GetTableNumber(L,1,"dexpreward",saselect.dexpreward);
		GetTableNumber(L,1,"dmoneyreward",saselect.dmoneyreward);
// 		std::string strname;
// 		GetTableStringValue(L, 1, "bestplayer", strname);
// 		strcpy(saselect.bestplayername, strname.c_str());
		memset(saselect.bestplayername, 0, sizeof(CONST_USERNAME));
		GetTableStringValue(L, 1, "bestplayer", saselect.bestplayername);
		GetTableNumber(L, 1, "besttime", saselect.wbesttime);
	}
	if (lua_istable(L,2))
	{
		int nsize =  luaL_getn(L, 2);
		int *itemreward = new int[nsize];
		for(int i = 0;i<nsize;i++)
		{
			if (GetTableArrayNumber(L,2,i+1,itemreward[i]))
			{
				memcpy(pitemdata,&itemreward[i],sizeof(int));
				pitemdata += sizeof(int);

				if (pitemdata - saselect.bitemreward > MAX_IREWARD_SIZE)
				{
					rfalse(2,1,"itemreward to large");
					break;
				}
			}
		}
		SAFE_DELETE_ARRAY(itemreward);
	}
	if (lua_istable(L,3))
	{
		int nsize =  luaL_getn(L, 3);
		int *itemreward = new int[nsize];
		for(int i = 0;i<nsize;i++)
		{
			if (GetTableArrayNumber(L,3,i+1,itemreward[i]))
			{
				memcpy(pequipdata,&itemreward[i],sizeof(int));
				pequipdata += sizeof(int);

				if (pequipdata - saselect.bequip > MAX_IREWARD_SIZE)
				{
					rfalse(2,1,"itemreward to large");
					break;
				}
			}
		}
		SAFE_DELETE_ARRAY(itemreward);
	}
	int msglen = sizeof(SASceneSelect);
	//int msglen = sizeof(SASceneSelect) - MAX_IREWARD_SIZE +(pequipdata - saselect.bequip);
// 	BYTE *ptemp = (BYTE*)&saselect;
// 	int n = (int)(saselect.bitemreward - ptemp);
// 	n = (int)(saselect.bequip - ptemp);
// 	int msglen = sizeof(SASceneSelect);
	CSceneMap::GetInstance().SendSelectSceneMsg(g_Script.m_pPlayer,&saselect,msglen);
	return 1;
}

int CScriptManager::GetTableNumber( lua_State *L,int nIndex,const char* key,int &nvalue )
{
	lua_getfield(L,nIndex,key);
	if (lua_isnumber(L,-1))
	{
		nvalue =static_cast<int>(lua_tonumber(L,-1));
		lua_pop(L, 1);
		return 1;
	}
	lua_pop(L,1);
	rfalse(2,1,FormatString("GetTableNumber Fail key = %s",key));
	return 0;
}

int CScriptManager::GetTableNumber( lua_State *L,int nIndex,const char* key,float &nvalue )
{
	lua_getfield(L,nIndex,key);
	if (lua_isnumber(L,-1))
	{
		nvalue =static_cast<float>(lua_tonumber(L,-1));
		lua_pop(L, 1);
		return 1;
	}
	lua_pop(L,1);
	rfalse(2,1,FormatString("GetTableNumber Fail key = %s",key));
	return 0;
}

int CScriptManager::GetTableNumber( lua_State *L,int nIndex,const char* key,DWORD &nvalue )
{
	lua_getfield(L,nIndex,key);
	if (lua_isnumber(L,-1))
	{
		nvalue =static_cast<DWORD>(lua_tonumber(L,-1));
		lua_pop(L, 1);
		return 1;
	}
	lua_pop(L,1);
	rfalse(2,1,FormatString("GetTableNumber Fail key = %s",key));
	return 0;
}

int CScriptManager::GetTableNumber(lua_State *L, int nIndex, const char* key, WORD &nvalue)
{
	lua_getfield(L, nIndex, key);
	if (lua_isnumber(L, -1))
	{
		nvalue = static_cast<WORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		return 1;
	}
	lua_pop(L, 1);
	rfalse(2, 1, FormatString("GetTableNumber Fail key = %s", key));
	return 0;
}

int CScriptManager::GetTableNumber(lua_State *L, int nIndex, const char* key, BYTE &nvalue)
{
	lua_getfield(L, nIndex, key);
	if (lua_isnumber(L, -1))
	{
		nvalue = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		return 1;
	}
	lua_pop(L, 1);
	rfalse(2, 1, FormatString("GetTableNumber Fail key = %s", key));
	return 0;
}

int CScriptManager::GetTableNumber(lua_State *L, int nIndex, const char* key, QWORD &nvalue)
{
	lua_getfield(L, nIndex, key);
	if (lua_isnumber(L, -1))
	{
		nvalue = static_cast<QWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		return 1;
	}
	lua_pop(L, 1);
	rfalse(2, 1, FormatString("GetTableNumber Fail key = %s", key));
	return 0;
}

int CScriptManager::GetTableNumber(lua_State *L, int nIndex, const char* key, double &nvalue)
{
	lua_getfield(L, nIndex, key);
	if (lua_isnumber(L, -1))
	{
		nvalue = static_cast<double>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		return 1;
	}
	lua_pop(L, 1);
	rfalse(2, 1, FormatString("GetTableNumber Fail key = %s", key));
	return 0;
}

int CScriptManager::GetTableArrayNumber(lua_State *L, int nIndex, int narrayIndex, int &nvalue)
{
	lua_rawgeti(L,nIndex,narrayIndex);
	if (lua_isnumber(L,-1))
	{
		nvalue = static_cast<int>(lua_tonumber(L,-1));
		lua_pop(L,1);
		return 1;
	}
	lua_pop(L,1);
	rfalse(2,1,FormatString("GetTableArrayNumber Fail tableIndex = %d,arrayIndex = %d",nIndex,narrayIndex));
	return 0;
}

int CScriptManager::GetTableArrayNumber(lua_State *L, int nIndex, int narrayIndex, BYTE &nvalue)
{
	lua_rawgeti(L, nIndex, narrayIndex);
	if (lua_isnumber(L, -1))
	{
		nvalue = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		return 1;
	}
	lua_pop(L, 1);
	rfalse(2, 1, FormatString("GetTableArrayNumber Fail tableIndex = %d,arrayIndex = %d", nIndex, narrayIndex));
	return 0;
}

int CScriptManager::GetTableArrayNumber(lua_State *L, int nIndex, int narrayIndex, float &nvalue)
{
	lua_rawgeti(L, nIndex, narrayIndex);
	if (lua_isnumber(L, -1))
	{
		nvalue = static_cast<float>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		return 1;
	}
	lua_pop(L, 1);
	rfalse(2, 1, FormatString("GetTableArrayNumber Fail tableIndex = %d,arrayIndex = %d", nIndex, narrayIndex));
	return 0;
}

int CScriptManager::GetTableArrayNumber(lua_State *L, int nIndex, int narrayIndex, DWORD &nvalue)
{
	lua_rawgeti(L, nIndex, narrayIndex);
	if (lua_isnumber(L, -1))
	{
		nvalue = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		return 1;
	}
	lua_pop(L, 1);
	rfalse(2, 1, FormatString("GetTableArrayNumber Fail tableIndex = %d,arrayIndex = %d", nIndex, narrayIndex));
	return 0;
}

int CScriptManager::GetTableArrayNumber(lua_State *L, int nIndex, int narrayIndex, WORD &nvalue)
{
	lua_rawgeti(L, nIndex, narrayIndex);
	if (lua_isnumber(L, -1))
	{
		nvalue = static_cast<WORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		return 1;
	}
	lua_pop(L, 1);
	rfalse(2, 1, FormatString("GetTableArrayNumber Fail tableIndex = %d,arrayIndex = %d", nIndex, narrayIndex));
	return 0;
}

int CScriptManager::L_SendEnterSceneMsg( lua_State *L )
{
	WORD  paramIndex = 1;
	CPlayer *pPlayer = g_Script.m_pPlayer;
	if (!pPlayer)
	{
		if (lua_isnumber(L, paramIndex))
		{
			DWORD pysid =  static_cast<DWORD>(lua_tonumber(L,1));
			pPlayer = (CPlayer*)GetPlayerBySID(pysid)->DynamicCast(IID_PLAYER);
			if (!pPlayer)
			{
				lua_pushnumber(L,-1);
				return 0;
			}
			paramIndex++;
		}
		return 0;
	}
	int nmsgsize = lua_gettop(L);
	SASceneEnter saentermsg;
	if (lua_isnumber(L, paramIndex))
	{
		saentermsg.bresult = lua_tonumber(L, paramIndex);
		paramIndex++;
	}
	saentermsg.dSDTime = 0;
	saentermsg.wSDcost = 0;
	if (nmsgsize > 1)
	{
		if (lua_isnumber(L, paramIndex) && lua_isnumber(L, paramIndex + 1))
		{
			saentermsg.dSDTime = static_cast<DWORD>(lua_tonumber(L, paramIndex));
			saentermsg.wSDcost = static_cast<WORD>(lua_tonumber(L, paramIndex+1));
		}
	}
	CSceneMap::GetInstance().SendEnterSceneMsg(pPlayer,&saentermsg);
	return 1;
}

int CScriptManager::L_CreateScene( lua_State* L )
{

	WORD	RegionIndex		= static_cast<WORD>	( lua_tonumber( L, 1 ) );//20000 lua编号
	int		RegionID        = static_cast<int>  ( lua_tonumber( L, 2 ) );
	BYTE    regintype = static_cast<BYTE>  (lua_tonumber(L, 3));
	WORD  pktype = static_cast<WORD>  (lua_tonumber(L, 4)); //新增PK模式
	if (regintype == 0) //创建关卡
	{
		DWORD dwGID = CreateDynamicScene(IID_DYNAMICSCENE, RegionIndex, RegionID, 0, 0, 0, 0, pktype);
		lua_pushnumber(L, dwGID);
	}
	else if (regintype == 1)//竞技场
	{
		DWORD dwGID = CreateDynamicArena(IID_DYNAMICARENA, RegionIndex, RegionID, 0, 0, 0, 0, pktype);
		lua_pushnumber(L, dwGID);
	}
	else if (regintype == 2)//日常活动
	{
		DWORD dwGID = CreateDynamicDailyArea(IID_DYNAMICDAILYAREA, RegionIndex, RegionID, 0, 0, 0, 0);
		lua_pushnumber(L, dwGID);
	}

	return 1;
}

int CScriptManager::L_GetObjectType( lua_State *L )
{
	if (!lua_isnumber(L,1))
	{
		return 0;	
	}
	int gid = lua_tonumber(L,1);
	BYTE objtype = gid >> 28;
	lua_pushnumber(L,objtype);
	return 1;
}

int CScriptManager::L_GetTableLenth( lua_State *L )
{
	if (!lua_istable(L,1))
	{
		return 0;
	}
	int nsize = luaL_getn(L,1);
	lua_pushnumber(L,nsize);
	return 1;
}

int CScriptManager::L_GetDynamicRegionID(lua_State *L)
{
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
	CFightObject *pfight = (CFightObject*)GetObjectByGID(gid)->DynamicCast(IID_FIGHTOBJECT);
	if (pfight)
	{
		DWORD dygid = pfight->GetDynamicRegionID();
		lua_pushnumber(L, dygid);
		return 1;
	}
	return 0;
}

int CScriptManager::L_DestroyScene( lua_State *L )
{
	if (!lua_isnumber(L,1))
	{
		return 0;
	}
	DWORD dwRegionGID = static_cast<DWORD>(lua_tonumber(L,1));
	CDynamicRegion* pregin =  (CDynamicRegion *)FindRegionByGID(dwRegionGID)->DynamicCast(IID_DYNAMICREGION);
	if (pregin)
	{
		pregin->isValid() = false;
		lua_pushnumber(L,1);
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendRewardMsg( lua_State *L )
{
	CPlayer *player = g_Script.m_pPlayer;
	if (!player)
	{
		DWORD pysid =  static_cast<DWORD>(lua_tonumber(L,3));
		player = (CPlayer*)GetPlayerBySID(pysid)->DynamicCast(IID_PLAYER);
		if (!player)
		{
			return 0;
		}
	}

	SAWinrewardInfo sawininfo;
	memset(sawininfo.bitemreward,0,MAX_IREWARD_SIZE);
	BYTE *pequipdata = sawininfo.bitemreward;
	if (lua_istable(L,1))
	{
		GetTableNumber(L,1,"bwintype",sawininfo.bwintype);
		GetTableNumber(L,1,"bwinstar",sawininfo.bwinstar);
		GetTableNumber(L,1,"wfinishtime",sawininfo.wfinishtime);
		GetTableNumber(L,1,"wkillcount",sawininfo.wkillcount);
		GetTableNumber(L,1,"wmaxhit",sawininfo.wmaxhit);
		GetTableNumber(L,1,"playerexp",sawininfo.dexp[0]);
		GetTableNumber(L,1,"petexp",sawininfo.dexp[1]);
		GetTableNumber(L,1,"mountexp",sawininfo.dexp[2]);
		GetTableNumber(L, 1, "dmoney", sawininfo.dmoney);
		GetTableNumber(L,1,"dmoneyextract_1",sawininfo.dmoneyextract[0]);
		GetTableNumber(L,1,"dmoneyextract_2",sawininfo.dmoneyextract[1]);
		GetTableNumber(L,1,"dmoneyextract_3",sawininfo.dmoneyextract[2]);
		GetTableNumber(L,1,"dgoldextract_1",sawininfo.dgoldextract[0]);
		GetTableNumber(L,1,"dgoldextract_2",sawininfo.dgoldextract[1]);
		GetTableNumber(L,1,"dgoldextract_3",sawininfo.dgoldextract[2]);
		GetTableNumber(L, 1, "bspecialscene", sawininfo.bspecialscene);
	}

	if (lua_istable(L,2))
	{
		int nsize =  luaL_getn(L, 2);
		int *itemreward = new int[nsize];
		for(int i = 0;i<nsize;i++)
		{
			if (GetTableArrayNumber(L,2,i+1,itemreward[i]))
			{
				memcpy(pequipdata,&itemreward[i],sizeof(int));
				pequipdata += sizeof(int);

				if (pequipdata - sawininfo.bitemreward > MAX_IREWARD_SIZE)
				{
					rfalse(2,1,"itemreward to large");
					break;
				}
			}
		}
		SAFE_DELETE_ARRAY(itemreward);
	}
	//int msglen = sizeof(SAWinrewardInfo) - MAX_IREWARD_SIZE +(pequipdata - sawininfo.bitemreward);
	int msglen = sizeof(SAWinrewardInfo);
	CSceneMap::GetInstance().SendRewardMsg(player,&sawininfo,msglen);
	return 1;
}

int CScriptManager::L_GetBlankCell( lua_State *L )
{
	CPlayer *player = g_Script.m_pPlayer;
	if (!player)
	{
		DWORD pygid =  static_cast<DWORD>(lua_tonumber(L,3));
		player = (CPlayer*)GetPlayerByGID(pygid)->DynamicCast(IID_PLAYER);
		if (!player)
		{
			return 0;
		}
	}
	WORD backcell = player->GetBlankCell(XYD_PT_BASE);
	lua_pushnumber(L,backcell);
	return 1;
}

int CScriptManager::L_cleanplayer( lua_State *L )
{
	if (g_Script.m_pPlayer)
	{
		g_Script.m_pPlayer = NULL;
	}
	return 1;
}

int CScriptManager::L_SendWinRewardMsg( lua_State *L )
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	SAPickCards winreward;
	memset(winreward.bitemreward,0,MAX_IREWARD_SIZE);
	BYTE *ptemp = winreward.bitemreward;
	if (lua_isnumber(L,1) && lua_isnumber(L,2))
	{
		winreward.bextractindex = static_cast<BYTE>(lua_tonumber(L,1));
		winreward.bextracttype = static_cast<BYTE>(lua_tonumber(L,2));
	}
	if (lua_istable(L,3))
	{
		int nsize =  luaL_getn(L, 3);
		int *itemreward = new int[nsize];
		for(int i = 0;i<nsize;i++)
		{
			if (GetTableArrayNumber(L,3,i+1,itemreward[i]))
			{
				memcpy(ptemp,&itemreward[i],sizeof(int));
				ptemp += sizeof(int);

				if (ptemp - winreward.bitemreward > MAX_IREWARD_SIZE)
				{
					rfalse(2,1,"itemreward to large");
					break;
				}
			}
		}
		SAFE_DELETE_ARRAY(itemreward);
	}
	WORD wmsglenth = sizeof(SAPickCards) - MAX_IREWARD_SIZE + (ptemp - winreward.bitemreward);
	CSceneMap::GetInstance().SendWinRewardMsg(g_Script.m_pPlayer,&winreward,wmsglenth);
	return 1;
}

BOOL CALLBACK CallBackCheckSceneRegion( LPIObject &region, LPARAM parm )
{
	CSceneMap *pScenemap = ( CSceneMap* )region->DynamicCast( IID_DYNAMICSCENE );

	if ( pScenemap )
	{
		parm ++;
	}
	return TRUE;
}

int CScriptManager::L_GetSceneCount( lua_State *L )
{
	return 1;
}

int CScriptManager::L_SetQusetSkillMsg( lua_State *L )
{
	if (!lua_istable(L,1))
	{
		return 0;
	}
	SQuestSkill_C2S_MsgBody questskillmsg;
	GetTableNumber(L,1,"AttackerGID",questskillmsg.mAttackerGID);
	CPlayer *pPlayer = (CPlayer*)GetPlayerByGID(questskillmsg.mAttackerGID)->DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		GetTableNumber(L, 1, "DefenderGID", questskillmsg.mDefenderGID);
		GetTableNumber(L, 1, "DefenderWorldPosX", questskillmsg.mDefenderWorldPosX);
		GetTableNumber(L, 1, "DefenderWorldPosY", questskillmsg.mDefenderWorldPosY);
		GetTableNumber(L, 1, "dwSkillIndex", questskillmsg.dwSkillIndex);
		pPlayer->SetQusetSkillMsg(&questskillmsg);
	}
	return 1;
}

int CScriptManager::L_TimeGetTime( lua_State *L )
{
	INT64 nowtime= timeGetTime();
	lua_pushnumber(L,nowtime);
	return 1;
}

int CScriptManager::L_ProcessQuestSkill( lua_State *L )
{
	if (!lua_istable(L,1))return 0;
	//if (!g_Script.m_pPlayer)return 0;

	int nsize = luaL_getn(L,1);
	SQuestSkillMultiple_C2S_MsgBody questMultipleskillmsg;
	GetTableNumber(L, 1, "AttackerGID", questMultipleskillmsg.mAttackerGID);
	CPlayer *pPlayer = (CPlayer*)GetPlayerByGID(questMultipleskillmsg.mAttackerGID)->DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		GetTableNumber(L, 1, "dwSkillIndex", questMultipleskillmsg.dwSkillIndex);
		QuestSkill luaquestskill;
		GetTableNumber(L, 1, "DefenderGID", luaquestskill.mDefenderGID);
		GetTableNumber(L, 1, "DefenderWorldPosX", luaquestskill.mDefenderWorldPosX);
		GetTableNumber(L, 1, "DefenderWorldPosY", luaquestskill.mDefenderWorldPosY);
		questMultipleskillmsg.bTargetNum = 1;
		questMultipleskillmsg.mAttackerWorldPosX = pPlayer->m_curX;
		questMultipleskillmsg.mAttackerWorldPosY = pPlayer->m_curY;
		memset(questMultipleskillmsg.buffer, 0, sizeof(questMultipleskillmsg.buffer));
		memcpy(questMultipleskillmsg.buffer, &luaquestskill, sizeof(QuestSkill));
		pPlayer->ProcessQuestSkill(&questMultipleskillmsg);
	}
	return 1;
}

int CScriptManager::L_GetSkillIDBySkillIndex( lua_State *L )
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	WORD index = static_cast<WORD>(lua_tonumber(L,1));
	int skillindex = g_Script.m_pPlayer->GetSkillIDBySkillIndex(index);
	lua_pushnumber(L,skillindex);
	return 1;
}

int CScriptManager::L_GetPlayerInScene( lua_State *L )
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	CDynamicScene *pscene = (CDynamicScene*)g_Script.m_pPlayer->m_ParentRegion->DynamicCast(IID_DYNAMICSCENE);
	if (pscene)
	{
		lua_pushnumber(L,1);
		return 1;
	}
	lua_pushnumber(L,0);
	return 0;
}

int CScriptManager::L_SendPassFailMsg( lua_State *L )
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L,1)&&lua_isnumber(L,2))
	{
		SAPassFail sfail;
		sfail.wfinishtime = static_cast<WORD>(lua_tonumber(L, 1));
		sfail.wmaxhit = static_cast<WORD>(lua_tonumber(L, 2));
		WORD wmsglenth = sizeof(SAPassFail) ;
		CSceneMap::GetInstance().SendPassFailMsg(g_Script.m_pPlayer,&sfail,wmsglenth);
		return 1;
	}
	return 0;
}

int CScriptManager::L_PlayerRelive( lua_State *L )
{
	if (lua_isnumber(L,1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L,1));
		CPlayer *pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
		if (!pPlayer)
		{
			return 0;
		}
		if (lua_istable(L,2))
		{
			WORD regionID = 0;
			WORD wX = 0 ;
			WORD wY = 0 ;
			WORD type = 0 ;
			GetTableNumber(L,2,"regionID",regionID);
			GetTableNumber(L,2,"wX",wX);
			GetTableNumber(L,2,"wY",wY);
			GetTableNumber(L,2,"type",type);
			pPlayer->TimeToRelive(regionID,wX,wY,type);
			return 1;
		}
	}
	else if (lua_istable(L,1))
	{
		if (!g_Script.m_pPlayer)
		{
			return 0;
		}
		WORD regionID = 0;
		WORD wX = 0 ;
		WORD wY = 0 ;
		WORD type = 0 ;
		GetTableNumber(L,1,"regionID",regionID);
		GetTableNumber(L,1,"wX",wX);
		GetTableNumber(L,1,"wY",wY);
		GetTableNumber(L,1,"type",type);
		g_Script.m_pPlayer->TimeToRelive(regionID,wX,wY,type);
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendSDMsg( lua_State *L )
{
	CPlayer *pPlayer = g_Script.m_pPlayer;
	if (!pPlayer)
	{
		if (lua_isnumber(L,4))
		{
			DWORD pysid =  static_cast<DWORD>(lua_tonumber(L,4));
			pPlayer = (CPlayer*)GetPlayerBySID(pysid)->DynamicCast(IID_PLAYER);
			if (!pPlayer)
			{
				lua_pushnumber(L,-1);
				return 0;
			}
		}
	}
	if (!pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L,1)&&lua_isnumber(L,2)&&lua_isnumber(L,3))
	{
		SAStopSD sstopsd;
		sstopsd.wSceneMap = lua_tonumber(L,1);
		sstopsd.wsceneIndex =  lua_tonumber(L,2);
		sstopsd.bresult=  lua_tonumber(L,3);
		WORD wmsglenth = sizeof(SAStopSD) ;
		CSceneMap::GetInstance().SendSDMsg(pPlayer,&sstopsd,wmsglenth);
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendSDReward( lua_State *L )
{
	CPlayer *pPlayer = g_Script.m_pPlayer;
	if (!pPlayer)
	{
		if (lua_isnumber(L,2))
		{
			DWORD pysid =  static_cast<DWORD>(lua_tonumber(L,2));
			pPlayer = (CPlayer*)GetPlayerBySID(pysid)->DynamicCast(IID_PLAYER);
			if (!pPlayer)
			{
				return 0;
			}
		}
		return 0;
	}
	if (lua_isnumber(L,1))
	{
		SASDReward sreward;
		sreward.bresult = lua_tonumber(L,1);
		WORD wmsglenth = sizeof(SASDReward) ;
		CSceneMap::GetInstance().SendSDReward(pPlayer,&sreward,wmsglenth);
		return 1;
	}
	return 0;
}

int CScriptManager::L_Removeobj( lua_State *L )
{
	if (!lua_isnumber(L,1))
	{
		return 0;
	}
	DWORD dwRegionGID = static_cast<DWORD>(lua_tonumber(L,1));
	DWORD dmonstergid = static_cast<DWORD>(lua_tonumber(L,2));
	CDynamicRegion* pregin =  (CDynamicRegion *)FindRegionByGID(dwRegionGID)->DynamicCast(IID_DYNAMICREGION);
	if (pregin)
	{
		pregin->DelMonster(dmonstergid);
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendPlayerDeadAddMsg( lua_State *L )
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	int nsize = lua_gettop(L);
	if (nsize != 4)return 0;
	SAPlayerDeadAddMsg sdeadmsg;
	if (lua_isnumber(L,1)&&lua_isnumber(L,2)&&lua_isnumber(L,3)&&lua_isnumber(L,4))
	{
		sdeadmsg.wFreeReliveTime = lua_tonumber(L,1);
		sdeadmsg.wReliveCost = lua_tonumber(L,2);
		sdeadmsg.activityAttkCost = lua_tonumber(L,3);
		sdeadmsg.activityDefenceCost = lua_tonumber(L,4);
		g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex,&sdeadmsg,sizeof(SAPlayerDeadAddMsg));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendReliveResult( lua_State *L )
{
	if (lua_isnumber(L,1)&&lua_isnumber(L,2))
	{
		DWORD sid =static_cast<DWORD>(lua_tonumber(L,1));
		BYTE bResult = lua_tonumber(L,2);
		CPlayer *pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
		if (pPlayer && bResult <= SAPlayerDeadResultMsg::RELIVE_FAIL_MONEY)
		{
			SAPlayerDeadResultMsg sResultMsg;
			sResultMsg.bResult = bResult;
			g_StoreMessage(pPlayer->m_ClientIndex,&sResultMsg,sizeof(SAPlayerDeadResultMsg));
			return 1;
		}
	}
	return 0;
}

int CScriptManager::L_SetCacheSkill( lua_State *L )
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnil(L,1))
	{
		SQuestSkill_C2S_MsgBody questskillmsg;
		memset(&questskillmsg,0,sizeof(SQuestSkill_C2S_MsgBody));
		g_Script.m_pPlayer->LuaSetCacheSkill(&questskillmsg);
		return 1;
	}

	if (!lua_istable(L,1))
	{
		return 0;
	}
	SQuestSkill_C2S_MsgBody questskillmsg;
	GetTableNumber(L,1,"AttackerGID",questskillmsg.mAttackerGID);
	if (g_Script.m_pPlayer->GetGID() != questskillmsg.mAttackerGID)
	{
		rfalse(2,1,"no same player");
		return 0;
	}
	GetTableNumber(L,1,"DefenderGID",questskillmsg.mDefenderGID);
	GetTableNumber(L,1,"DefenderWorldPosX",questskillmsg.mDefenderWorldPosX);
	GetTableNumber(L,1,"DefenderWorldPosY",questskillmsg.mDefenderWorldPosY);
	GetTableNumber(L,1,"dwSkillIndex",questskillmsg.dwSkillIndex);
	g_Script.m_pPlayer->LuaSetCacheSkill(&questskillmsg);
	return 1;
}

int CScriptManager::L_SendSceneListMsg( lua_State *L )
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	SASceneList scenelist;
	memset(scenelist.bhighlight,0,MAX_LIST_SIZE);
	if (lua_isnumber(L,1)&&lua_isnumber(L,2))
	{
		scenelist.wSceneMap = static_cast<WORD>(lua_tonumber(L,1));
		scenelist.wCurrentScene = static_cast<WORD>(lua_tonumber(L,2));
		if (lua_istable(L,3))
		{
			int nsize =  luaL_getn(L, 3);
			int *nbhighlight = (int*)scenelist.bhighlight;
			for (int i = 0;i<nsize;i++)
			{
				GetTableArrayNumber(L,3,i+1,*nbhighlight);
				nbhighlight += sizeof(int);
			}
		}
		WORD wmsglenth = sizeof(SASceneList);
		CSceneMap::GetInstance().SendSceneListMsg(g_Script.m_pPlayer,&scenelist,wmsglenth);
		return 1;
	}
	return 0;
}

int CScriptManager::L_GetObjectData( lua_State *L )
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		return 0;
	}
	DWORD gid = static_cast<DWORD>(lua_tonumber(L,1));
	int index = static_cast<int>(lua_tonumber(L, 2));
	LPIObject pobject = GetObjectByGID(gid);
	CFightObject *pfightobj = (CFightObject*)pobject->DynamicCast(IID_FIGHTOBJECT);
	if (!pfightobj)
	{
		return 0;
	}
	CPlayer *pPlayer = (CPlayer*)pfightobj->DynamicCast(IID_PLAYER);
	switch (index)
	{
	case 0:		//当前攻击值
		lua_pushnumber(L, pfightobj->m_GongJi);
		break;
	case 1:		//当前防御值
		lua_pushnumber(L, pfightobj->m_FangYu);
		break;
	case 2:		//暴击
		lua_pushnumber(L, pfightobj->m_BaoJi);
		break;
	case 10:		// 等级
		//update by ly 2014/7/21 当玩家在变身状态下并且玩家在战斗场景中，获取变身宠物的等级；（战斗伤害计算需要根据对象的等级）
		if (pPlayer != NULL && pPlayer->m_ParentRegion->DynamicCast(IID_DYNAMICREGION) != NULL && pPlayer->m_Property.m_FightPetActived != 0xff)
		{
			SNewPetData &pPet = pPlayer->m_Property.m_NewPetData[pPlayer->m_Property.m_TransPetIndex];
			if (0 == pPet.m_PetID)
				lua_pushnumber(L, 0);
			lua_pushnumber(L, pPet.m_PetLevel);
		}
		else
			lua_pushnumber(L, pfightobj->m_Level);
		break;

	case 100:		// 性别
		//lua_pushnumber(L, pPlayer->m_Property.m_Sex);
		break;
	case 4:		//最大生命值
		lua_pushnumber(L, pfightobj->m_MaxHp);
		break;

	case 5:		// 帮派名字
		//lua_pushstring(L, pPlayer->m_Property.m_szTongName);
		break;
	case 6:		//侠义值
		//lua_pushnumber(L, pPlayer->m_Property.m_XYValue);
		break;
	case 7:		//杀孽值
		//lua_pushnumber(L, pPlayer->m_wPKValue);
		break;
	case 8:		//精力值
		//lua_pushnumber(L, pPlayer->m_Property.m_CurJp);
		break;
	case 9:		//经验值
		//lua_pushnumber(L, pPlayer->m_Property.m_Exp);
		break;
	case 21:		//真气值
		if (pPlayer)
		{
			lua_pushnumber(L, pPlayer->m_Property.m_CurSp);
		}
		break;
	case 11:		//当前移动速度值
		lua_pushnumber(L, pfightobj->m_OriSpeed);
		break;
	case 12:		//当前攻击速度
		lua_pushnumber(L, pfightobj->m_AtkSpeed);
		break;
	case 13:		//当前轻功点数
		//lua_pushnumber(L, pPlayer->m_Property.m_QingShen);
		break;
	case 14:		//当前强健点数
	//	lua_pushnumber(L, pPlayer->m_Property.m_JianShen);
		break;
	case 15:		// 配偶名字
		//lua_pushnumber(L, pPlayer->m_Property.m_Sex);
		break;
	case 16:		//最大内力值
		lua_pushnumber(L, pfightobj->m_MaxMp);
		break;
	case 17:		//最大体力值
		lua_pushnumber(L, pfightobj->m_MaxTp);
		break;
	case 18:		//当前生命值
		lua_pushnumber(L, pfightobj->m_CurHp);
		break;
	case 19:		//当前内力值
		lua_pushnumber(L, pfightobj->m_CurMp);
		break;
	case 20:		//当前体力值
		lua_pushnumber(L, pfightobj->m_CurTp);
		break;
	case 23:		// 怪物ID
	{
		CMonster *pmonster = (CMonster*)pfightobj->DynamicCast(IID_MONSTER);
		if (pmonster)
		{
			lua_pushnumber(L, pmonster->m_Property.m_ID);
		}
	}
		break;
	case 3:		//当前闪避值
	{
		lua_pushnumber(L, pfightobj->m_ShanBi);
	}
		break;
	case 25:		//当前防御点数
		break;
	case 26:		//当前进攻点数
		break;
	case 27:		//是否无敌
	{
		bool state = pfightobj->m_bPlayerAction[CST_WUDI];
		lua_pushnumber(L, state);
	}
		break;
	}
	return 1;
}

int CScriptManager::L_GetEquipment( lua_State *L )
{
	CPlayer *pPlayer = g_Script.m_pPlayer;
	if (!pPlayer)
	{
		if (lua_isnumber(L,3))
		{
			DWORD gid = static_cast<DWORD>(lua_tonumber(L,3));
			pPlayer = (CPlayer*)GetPlayerByGID(gid)->DynamicCast(IID_PLAYER);
		}
	}
	if (!pPlayer)
	{
		return 0;
	}
	BYTE btype = lua_tonumber(L,1);
	BYTE bIndex = lua_tonumber(L,2);
	SEquipment pequip;
	memset(&pequip,0,sizeof(SEquipment));
	if (pPlayer->GetEquipment(btype,bIndex,pequip))
	{
		lua_createtable(L,0,0);
		SetTableNumber(L,"wIndex",pequip.wIndex);
		SetTableNumber(L,"currWear",pequip.attribute.currWear );
		SetTableNumber(L,"maxWear",pequip.attribute.maxWear );
		SetTableNumber(L,"grade",pequip.attribute.grade );
		SetTableNumber(L,"bIntensifyLevel",pequip.attribute.bIntensifyLevel );
		SetTableNumber(L,"bCurrentStar",pequip.attribute.bCurrentStar);
		
		SetTableNumber(L,"bornAttriType",pequip.attribute.bornAttri[0].type);
		SetTableNumber(L,"bornAttriValue",pequip.attribute.bornAttri[0].value);
		SetTableNumber(L,"bornAttriType2",pequip.attribute.bornAttri[1].type);
		SetTableNumber(L,"bornAttriValue2",pequip.attribute.bornAttri[1].value);

		SetTableNumber(L,"starAttriType",pequip.attribute.starAttri[0].type);
		SetTableNumber(L,"starAttriValue",pequip.attribute.starAttri[0].value);
		SetTableNumber(L,"starAttriType2",pequip.attribute.starAttri[1].type);
		SetTableNumber(L,"starAttriValue2",pequip.attribute.starAttri[1].value);

		SetTableNumber(L, "MaxJDingAttriType", pequip.attribute.MaxJDingAttri[0].type);
		SetTableNumber(L, "MaxJDingAttriValue", pequip.attribute.MaxJDingAttri[0].value);
		SetTableNumber(L, "MaxJDingAttriType2", pequip.attribute.MaxJDingAttri[1].type);
		SetTableNumber(L, "MaxJDingAttriValue2", pequip.attribute.MaxJDingAttri[1].value);
		SetTableNumber(L, "MaxJDingAttriType3", pequip.attribute.MaxJDingAttri[2].type);
		SetTableNumber(L, "MaxJDingAttriValue3", pequip.attribute.MaxJDingAttri[2].value);
		SetTableNumber(L, "MaxJDingAttriType4", pequip.attribute.MaxJDingAttri[3].type);
		SetTableNumber(L, "MaxJDingAttriValue4", pequip.attribute.MaxJDingAttri[3].value);
		SetTableNumber(L, "MaxJDingAttriType5", pequip.attribute.MaxJDingAttri[4].type);
		SetTableNumber(L, "MaxJDingAttriValue5", pequip.attribute.MaxJDingAttri[4].value);
		SetTableNumber(L, "MaxJDingAttriType6", pequip.attribute.MaxJDingAttri[5].type);
		SetTableNumber(L, "MaxJDingAttriValue6", pequip.attribute.MaxJDingAttri[5].value);
		SetTableNumber(L, "bJDingAttriNum", pequip.attribute.bJDingAttriNum);

		SetTableNumber(L, "MaxKeYinLevel", pequip.equipkeyin.MaxKeYinLevel[0]);
		SetTableNumber(L, "MaxKeYinAttriType", pequip.equipkeyin.MaxKeYinAttri[0].type);
		SetTableNumber(L, "MaxKeYinAttriValue", pequip.equipkeyin.MaxKeYinAttri[0].value);
		SetTableNumber(L, "MaxKeYinLevel2", pequip.equipkeyin.MaxKeYinLevel[1]);
		SetTableNumber(L, "MaxKeYinAttriType2", pequip.equipkeyin.MaxKeYinAttri[1].type);
		SetTableNumber(L, "MaxKeYinAttriValue2", pequip.equipkeyin.MaxKeYinAttri[1].value);
		SetTableNumber(L, "MaxKeYinLevel3", pequip.equipkeyin.MaxKeYinLevel[2]);
		SetTableNumber(L, "MaxKeYinAttriType3", pequip.equipkeyin.MaxKeYinAttri[2].type);
		SetTableNumber(L, "MaxKeYinAttriValue3", pequip.equipkeyin.MaxKeYinAttri[2].value);


		return 1;
	}
	lua_pushnil(L);
	return 1;
}

int CScriptManager::SetTableNumber( lua_State *L,const char *key,int nvalue )
{
	if (!lua_istable(L,-1))
	{
		return 0;
	}
	lua_pushstring( L, key); 
	lua_pushnumber( L, nvalue); 
	lua_settable( L, -3 );
	return 1;
}

int CScriptManager::SetTableArrayNumber( lua_State *L,int nIndex,int nvalue )
{
	if (!lua_istable(L,-1))
	{
		return 0;
	}
	lua_pushnumber( L, nvalue );
	lua_rawseti( L, -2, nIndex);
	return 1;
}

int CScriptManager::L_SendIntensifyInfo( lua_State *L )
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_istable(L,1))
	{
		SAIntensifyInfo sinfo;
		GetTableNumber(L,1,"moneyconst",sinfo.dmoneycost);
		GetTableNumber(L,1,"playercoldtime",sinfo.dcoldtime);
		GetTableNumber(L, 1, "allcoldtime", sinfo.dallcoldtime);
		GetTableNumber(L, 1, "successrate", sinfo.bIntensifysuccessrate);
		GetTableNumber(L,1,"upgradrate",sinfo.bupgradrate);
		GetTableNumber(L,1,"nextupgradrate",sinfo.bnextupgradrate);
		GetTableNumber(L, 1, "attriValue", sinfo.wNextLevelattriValue);
		GetTableNumber(L, 1, "bCanIntensify", sinfo.bCanIntensify);
		g_Script.m_pPlayer->SendIntensifyInfo(&sinfo);
		return 1;
	}
	return 0;
}

int CScriptManager::L_SetEquipment( lua_State *L )
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	BYTE btype = lua_tonumber(L,1);
	BYTE bIndex = lua_tonumber(L,2);
	if (lua_istable(L,3))
	{
		SEquipment pequip;
		memset(&pequip, 0, sizeof(SEquipment));
		if (g_Script.m_pPlayer->GetEquipment(btype, bIndex, pequip))
		{
			GetTableNumber(L, 3, "wIndex", pequip.wIndex);
			GetTableNumber(L, 3, "currWear", pequip.attribute.currWear);
			GetTableNumber(L, 3, "maxWear", pequip.attribute.maxWear);
			GetTableNumber(L, 3, "grade", pequip.attribute.grade);
			GetTableNumber(L, 3, "bIntensifyLevel", pequip.attribute.bIntensifyLevel);
			GetTableNumber(L, 3, "bCurrentStar", pequip.attribute.bCurrentStar);

			GetTableNumber(L, 3, "bornAttriType", pequip.attribute.bornAttri[0].type);
			GetTableNumber(L, 3, "bornAttriValue", pequip.attribute.bornAttri[0].value);
			GetTableNumber(L, 3, "bornAttriType2", pequip.attribute.bornAttri[1].type);
			GetTableNumber(L, 3, "bornAttriValue2", pequip.attribute.bornAttri[1].value);

			GetTableNumber(L, 3, "starAttriType", pequip.attribute.starAttri[0].type);
			GetTableNumber(L, 3, "starAttriValue", pequip.attribute.starAttri[0].value);
			GetTableNumber(L, 3, "starAttriType2", pequip.attribute.starAttri[1].type);
			GetTableNumber(L, 3, "starAttriValue2", pequip.attribute.starAttri[1].value);


			GetTableNumber(L,3,"MaxJDingAttriType", pequip.attribute.MaxJDingAttri[0].type);
			GetTableNumber(L,3, "MaxJDingAttriValue", pequip.attribute.MaxJDingAttri[0].value);
			GetTableNumber(L, 3, "MaxJDingAttriType2", pequip.attribute.MaxJDingAttri[1].type);
			GetTableNumber(L, 3, "MaxJDingAttriValue2", pequip.attribute.MaxJDingAttri[1].value);
			GetTableNumber(L, 3, "MaxJDingAttriType3", pequip.attribute.MaxJDingAttri[2].type);
			GetTableNumber(L, 3, "MaxJDingAttriValue3", pequip.attribute.MaxJDingAttri[2].value);
			GetTableNumber(L, 3, "MaxJDingAttriType4", pequip.attribute.MaxJDingAttri[3].type);
			GetTableNumber(L, 3, "MaxJDingAttriValue4", pequip.attribute.MaxJDingAttri[3].value);
			GetTableNumber(L, 3, "MaxJDingAttriType5", pequip.attribute.MaxJDingAttri[4].type);
			GetTableNumber(L, 3, "MaxJDingAttriValue5", pequip.attribute.MaxJDingAttri[4].value);
			GetTableNumber(L, 3, "MaxJDingAttriType6", pequip.attribute.MaxJDingAttri[5].type);
			GetTableNumber(L, 3, "MaxJDingAttriValue6", pequip.attribute.MaxJDingAttri[5].value);
			GetTableNumber(L, 3, "bJDingAttriNum", pequip.attribute.bJDingAttriNum);

			GetTableNumber(L, 3, "MaxKeYinLevel", pequip.equipkeyin.MaxKeYinLevel[0]);
			GetTableNumber(L, 3, "MaxKeYinAttriType", pequip.equipkeyin.MaxKeYinAttri[0].type);
			GetTableNumber(L, 3, "MaxKeYinAttriValue", pequip.equipkeyin.MaxKeYinAttri[0].value);
			GetTableNumber(L, 3, "MaxKeYinLevel2", pequip.equipkeyin.MaxKeYinLevel[1]);
			GetTableNumber(L, 3, "MaxKeYinAttriType2", pequip.equipkeyin.MaxKeYinAttri[1].type);
			GetTableNumber(L, 3, "MaxKeYinAttriValue2", pequip.equipkeyin.MaxKeYinAttri[1].value);
			GetTableNumber(L, 3, "MaxKeYinLevel3", pequip.equipkeyin.MaxKeYinLevel[2]);
			GetTableNumber(L, 3, "MaxKeYinAttriType3", pequip.equipkeyin.MaxKeYinAttri[2].type);
			GetTableNumber(L, 3, "MaxKeYinAttriValue3", pequip.equipkeyin.MaxKeYinAttri[2].value);

			g_Script.m_pPlayer->SetEquipment(btype, bIndex, &pequip);
		}
		
		return 1;
	}
	return 0;
}

int CScriptManager::L_IsLockedEquipColumnCell( lua_State *L )
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L,1))
	{
		WORD wEquipPos= static_cast<DWORD>(lua_tonumber(L,1));
		int nislock = g_Script.m_pPlayer->IsLockedEquipColumnCell(wEquipPos);
		lua_pushnumber(L,nislock);
		return 1;
	}
	return 0;
}

int CScriptManager::L_LockEquipColumnCell( lua_State *L )
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L,1)&&lua_isnumber(L,2)&&lua_isnumber(L,3))
	{
		WORD wEquipPos= static_cast<DWORD>(lua_tonumber(L,1));
		int lock= static_cast<int>(lua_tonumber(L,2));
		int sync = static_cast<int>(lua_tonumber(L,3));
		int nlock = g_Script.m_pPlayer->LockEquipColumnCell(wEquipPos,lock,sync);
		lua_pushnumber(L,nlock);
		return 1;
	}
	return 0;
}

int CScriptManager::L_GetObjectFightData( lua_State *L )
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		return 0;
	}
	DWORD gid = static_cast<DWORD>(lua_tonumber(L,1));
	int index = static_cast<int>(lua_tonumber(L, 2));
	LPIObject pobject = GetObjectByGID(gid);
	CFightObject *pfightobj = (CFightObject*)pobject->DynamicCast(IID_FIGHTOBJECT);
	if (!pfightobj)
	{
		return 0;
	}
	switch (index)
	{
	case  0:  //等级
		lua_pushnumber(L,pfightobj->m_Level);
		break;
	case 1: //生命
		lua_pushnumber(L,pfightobj->m_CurHp);
		break;
	case 2: //攻击
		lua_pushnumber(L,pfightobj->m_GongJi);
		break;
	case 3://防御
		lua_pushnumber(L,pfightobj->m_FangYu);
		break;
	case 4: //命中
		lua_pushnumber(L,pfightobj->m_Hit);
		break;
	case 5: //闪避
		lua_pushnumber(L,pfightobj->m_ShanBi);
		break;
	case 6: //暴击
		lua_pushnumber(L,pfightobj->m_BaoJi);
		break;
	default:
		break;
	}
	if (index > 6 && index < 12) //新增加属性
	{
		int nvalue = pfightobj->m_newAddproperty[index -  SEquipDataEx::EEA_UNCRIT];
		lua_pushnumber(L,nvalue);
	}
	return 1;
}

int CScriptManager::L_SendIntensifyResult( lua_State *L )
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L,1))
	{
		SABeginEquipLevelUpMsg pmsg;
		pmsg.result = lua_tonumber(L,1);
		g_Script.m_pPlayer->SendIntensifyResult(&pmsg);
		return 1;
	}
	return 0;
}
//extern struct SDamage sdamage;
int CScriptManager::L_SendObjectDamage(lua_State *L)
{
	if (!lua_isnumber(L,1)|| !lua_isnumber(L,2))
	{
		return 0;
	}
	DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
	LPIObject pobject = GetObjectByGID(gid);
	CFightObject *pfightobj = (CFightObject*)pobject->DynamicCast(IID_FIGHTOBJECT);
	if (!pfightobj)
	{
		return 0;
	}

	DWORD giddest = static_cast<DWORD>(lua_tonumber(L, 2));
	LPIObject pobjectDest = GetObjectByGID(giddest);
	CFightObject *pfightobjDest = (CFightObject*)pobjectDest->DynamicCast(IID_FIGHTOBJECT);
	if (!pfightobjDest)
	{
		return 0;
	}


	if (lua_istable(L,3))
	{
		CFightObject::SDamage sdamge;
		GetTableNumber(L, 3, "wDamage", sdamge.wDamage);
		GetTableNumber(L, 3, "wDamageExtra", sdamge.wDamageExtra);
		GetTableNumber(L, 3, "wIgnoreDefDamage", sdamge.wIgnoreDefDamage);
		GetTableNumber(L, 3, "wReboundDamage", sdamge.wReboundDamage);
		GetTableNumber(L, 3, "mHitState", sdamge.mHitState);
		GetTableNumber(L, 3, "isCritical", (BYTE&)sdamge.isCritical);
		pfightobj->SendDamageMessageBySkillAttack_Normal(pfightobjDest, sdamge);
		return 1;
	}
	return 0;
}

int CScriptManager::L_SetObjectDamage(lua_State *L)
{
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
	LPIObject pobject = GetObjectByGID(gid);
	CFightObject *pfightobj = (CFightObject*)pobject->DynamicCast(IID_FIGHTOBJECT);
	if (!pfightobj)
	{
		return 0;
	}
	if (lua_istable(L, 2))
	{
		CFightObject::SDamage sdamge;
		GetTableNumber(L, 2, "wDamage", sdamge.wDamage);
		GetTableNumber(L, 2, "wDamageExtra", sdamge.wDamageExtra);
		GetTableNumber(L, 2, "wIgnoreDefDamage", sdamge.wIgnoreDefDamage);
		GetTableNumber(L, 2, "wReboundDamage", sdamge.wReboundDamage);
		GetTableNumber(L, 2, "mHitState", sdamge.mHitState);
		GetTableNumber(L, 2, "isCritical",(BYTE&)sdamge.isCritical);
		pfightobj->SetDamage(&sdamge);
		return 1;
	}
	return 0;
}

int CScriptManager::L_GetPlayerIP(lua_State *L)
{
	CPlayer *pPlayer = 0;
	if (g_Script.m_pPlayer)
	{
		pPlayer = g_Script.m_pPlayer;
	}
	else
	{
		if (lua_isstring(L, 1))
		{
			const char* straccount = static_cast<const char *>(lua_tostring(L, 1));
			pPlayer = (CPlayer*)GetPlayerByAccount(straccount)->DynamicCast(IID_PLAYER);
		}
	}

	if (!pPlayer)
	{
		return 0;
	}
	WORD nport;
	LPCSTR ipstring = GetIPString(pPlayer->m_ClientIndex, &nport);
	if (ipstring)
	{
		lua_pushstring(L, ipstring);
		return 1;
	}
	return 0;
}

void CScriptManager::LuaRegisterFunc()
{
#define LuaRegisterFunction( _funcname ) RegisterFunction( #_funcname, CScriptManager::L_##_funcname );
	LuaRegisterFunction(SendSceneMsg);
	LuaRegisterFunction(SendSelectSceneMsg);
	LuaRegisterFunction(SendEnterSceneMsg);
	LuaRegisterFunction(CreateScene);
	LuaRegisterFunction(GetTableLenth);
	LuaRegisterFunction(GetObjectType);
	LuaRegisterFunction(DestroyScene);
	LuaRegisterFunction(DestroyDailyArea); 
	LuaRegisterFunction(ResponseEntryDailyArea); 
	LuaRegisterFunction(SendDailyAwardToPlayer); 
	LuaRegisterFunction(GetPlayerInDailyArea); 
	LuaRegisterFunction(SendNextBatchMonsterInf); 
	LuaRegisterFunction(SendDailyOpenFlag);
	LuaRegisterFunction(BroadCastDailyOpenFlag); 
	LuaRegisterFunction(SendJuLongShanAward); 
	LuaRegisterFunction(SendTarotBaseData);
	LuaRegisterFunction(SendAddTurnOnTarotData);
	LuaRegisterFunction(SendPlayerGetTarotAwardRes);
	LuaRegisterFunction(SendResetTurnOnTarotData);
	LuaRegisterFunction(SendCurDayOnlineData);
	LuaRegisterFunction(SendCurDayOnlineLong);
	LuaRegisterFunction(SendCurDayOnlineGetAwardRes);
	LuaRegisterFunction(NotifyCurDayOnlineReset);
	LuaRegisterFunction(NotifyCurDayOnlineAddNewAward); 
	LuaRegisterFunction(GetPlayerAddUpLoginTime);
	LuaRegisterFunction(SendPlayerRFBInf);
	LuaRegisterFunction(SendGetRFBAwardRes);
	LuaRegisterFunction(NotifyRFBCanGetNewAward);
	LuaRegisterFunction(SendPlayerLRInf);
	LuaRegisterFunction(SendGetLRAwardRes);
	LuaRegisterFunction(NotifyLRCanGetNewAward);
	LuaRegisterFunction(BroadCastLRResidueTimes);
	LuaRegisterFunction(SendFWInf);
	LuaRegisterFunction(SendPlayerFWInf);
	LuaRegisterFunction(SendFWSelectGoodsInf);
	LuaRegisterFunction(SendFWGEtAwardRes);
	LuaRegisterFunction(BroadCastFWUptRecordInf); 
	LuaRegisterFunction(SynRechargeTatolGlod);
	LuaRegisterFunction(SendPlayerAddUpLogintime);
	LuaRegisterFunction(SendPlayerAddUpLoginInf);
	LuaRegisterFunction(NotifyAddUpLoginAddNewAward);
	LuaRegisterFunction(SendAddUpLoginGetAwardRes);
	LuaRegisterFunction(SendSignInAwardInfo);
	LuaRegisterFunction(SendGetSignInAwardResult); 
	LuaRegisterFunction(GetPlayerSchool);
	LuaRegisterFunction(SendShopListInfo); 
	LuaRegisterFunction(SendShopGoodsListInfo);
	LuaRegisterFunction(NotityGoodsUpdated);
	LuaRegisterFunction(SendBuyGoodsRes);  
	LuaRegisterFunction(GetGoodsIndexByTriIndex);
	LuaRegisterFunction(GetTriIndexByGoodsIndex); 
	LuaRegisterFunction(SendShopCountDownRes); 
	LuaRegisterFunction(SendGoodsInfo);
	LuaRegisterFunction(FirstRegCallSetPlayerSkill); 
	LuaRegisterFunction(SetCurVipFactor);
	LuaRegisterFunction(SendVipStateInf);
	LuaRegisterFunction(SendGiftInfo);
	LuaRegisterFunction(SendGetOrButGiftRes); 
	LuaRegisterFunction(AddSomeGoods); 
	LuaRegisterFunction(GetPlayerPetNum);
	LuaRegisterFunction(AddNewPet);
	LuaRegisterFunction(SendCardPetInf); 
	LuaRegisterFunction(SendPetCardInf); 
	LuaRegisterFunction(ExceSwallowPetOpt); 
	LuaRegisterFunction(PetBreachOpt);
	LuaRegisterFunction(PetRenameOpt);
	LuaRegisterFunction(SendPetMergerInf);
	LuaRegisterFunction(SendPetBreachInf);
	LuaRegisterFunction(SendPetRenameInf); 
	LuaRegisterFunction(GetPetIDAndStar);  
	LuaRegisterFunction(PetStudySkillByBook); 
	LuaRegisterFunction(ActivePetSkill);
	LuaRegisterFunction(GetFightObjActionID);
	LuaRegisterFunction(PetSkillCfg); 
	LuaRegisterFunction(CalAndSendPetExp); 
	LuaRegisterFunction(JudgePetDurableIsZero);
	LuaRegisterFunction(SendResumeDurableNeedGold);
	LuaRegisterFunction(SendGoldResumeDurableRes); 
	LuaRegisterFunction(UptPetDurable);
	LuaRegisterFunction(NotifyPetDurableUpt);
	LuaRegisterFunction(NotifyMsgUtf8ToAnsi); 
	LuaRegisterFunction(GetFirstPayFlag);
	LuaRegisterFunction(SetFirstPayFlag); 
	LuaRegisterFunction(GetEveryDayPayFlag); 
	LuaRegisterFunction(SetEveryDayPayFlag);
	LuaRegisterFunction(SendEveryDayPayAwardState);
	LuaRegisterFunction(SendGetEDPAwardRes);
	LuaRegisterFunction(SendRewardMsg);
	LuaRegisterFunction(GetBlankCell); 
	LuaRegisterFunction(SendRandListData); 
	LuaRegisterFunction(SendRandAwardFlag); 
	LuaRegisterFunction(SendGetRandAwardRes); 
	LuaRegisterFunction(SendActivenessInfo);
	LuaRegisterFunction(SendGetActivenessAwardRes);
	LuaRegisterFunction(cleanplayer);
	LuaRegisterFunction(SendWinRewardMsg);
	LuaRegisterFunction(GetSceneCount);
	LuaRegisterFunction(SetQusetSkillMsg);
	LuaRegisterFunction(TimeGetTime);
	//LuaRegisterFunction(ProcessQuestSkill);
	LuaRegisterFunction(GetSkillIDBySkillIndex);
	LuaRegisterFunction(MystringFormat);
	LuaRegisterFunction(GetPlayerInScene);
	LuaRegisterFunction(SendPassFailMsg);
	LuaRegisterFunction(PlayerRelive);
	LuaRegisterFunction(SendSDMsg);
	LuaRegisterFunction(SendSDReward);
	LuaRegisterFunction(Removeobj);
	LuaRegisterFunction(SendPlayerDeadAddMsg);
	LuaRegisterFunction(SendReliveResult);
	LuaRegisterFunction(MystringFormat);
	LuaRegisterFunction(SetCacheSkill);
	LuaRegisterFunction(SendSceneListMsg);
	LuaRegisterFunction(GetObjectData);
	LuaRegisterFunction(GetEquipment);
	LuaRegisterFunction(SendIntensifyInfo);
	LuaRegisterFunction(SetEquipment);
	LuaRegisterFunction(IsLockedEquipColumnCell);
	LuaRegisterFunction(LockEquipColumnCell);
	LuaRegisterFunction(SendIntensifyResult);
	LuaRegisterFunction(SendKeYinResult);	
	LuaRegisterFunction(GetObjectFightData);
	LuaRegisterFunction(SendObjectDamage);
	LuaRegisterFunction(SetObjectDamage);
	LuaRegisterFunction(GetPlayerIP);
	LuaRegisterFunction(SendSceneFinishMsg);
	LuaRegisterFunction(SendQualityInfo);
	LuaRegisterFunction(DeleteItem);
	LuaRegisterFunction(DelEquipment);
	LuaRegisterFunction(AddItem);
	LuaRegisterFunction(RecvUseItem);
	LuaRegisterFunction(SendQualityResult);
	LuaRegisterFunction(GetItemBaseData);
	LuaRegisterFunction(GetIsNewPlayer);
	LuaRegisterFunction(SetIsNewPlayer);
	LuaRegisterFunction(KillAllMonster);
	LuaRegisterFunction(SendCleanColdResult);
	LuaRegisterFunction(SendRiseStarInfo);
	LuaRegisterFunction(SendRiseStarResult);
	LuaRegisterFunction(ChangeObjectData);
	LuaRegisterFunction(SetObjectData);
	LuaRegisterFunction(SendEquiptKeYinInfo);
	LuaRegisterFunction(SendEquipJDingResult);
	LuaRegisterFunction(SendEquipJDingInfo);
	LuaRegisterFunction(SendEquipJDingColor);
	LuaRegisterFunction(SendKeYinChangeData);
	LuaRegisterFunction(SetTaskFinish);
	LuaRegisterFunction(ReloadPlayerData);
	LuaRegisterFunction(InitPlayerData);
	LuaRegisterFunction(SendSuitcondition);
	LuaRegisterFunction(SendDebrisinfo);
	LuaRegisterFunction(SendDebrisAward);
	LuaRegisterFunction(SetTransformersLevel);
	LuaRegisterFunction(GetTransformersLevel);
	LuaRegisterFunction(OnSendTransformersInfoMsg);
	LuaRegisterFunction(SendTransformersSwitch);
	LuaRegisterFunction(SendMyState);
	LuaRegisterFunction(InitPlayerProperty);
	LuaRegisterFunction(GetCurrentSkillLevel);
	LuaRegisterFunction(SetCurrentSkillLevel);
	LuaRegisterFunction(GetSkillLevel);
	LuaRegisterFunction(SetSkillLevel);
	LuaRegisterFunction(GetSkillProficiency);
	LuaRegisterFunction(AddSkillProficiency);
	LuaRegisterFunction(SendTransformersSkillInfo);
	LuaRegisterFunction(CheckFightState);
	LuaRegisterFunction(LoadTable);
	LuaRegisterFunction(SaveTable);
	LuaRegisterFunction(TaskKillMonster);
	LuaRegisterFunction(AddPlayerXinYangProperty);
	LuaRegisterFunction(SendBatchesMonster);
	LuaRegisterFunction(Syneffects);
	LuaRegisterFunction(CopyFromPlayer);
	LuaRegisterFunction(CopyPlayerToTable);
	LuaRegisterFunction(GetObjectReginType);
	LuaRegisterFunction(GetSkillDamageRate);
	LuaRegisterFunction(SendArenaInitMsg);
	LuaRegisterFunction(SetStopTracing);
	LuaRegisterFunction(SendSceneSDListMsg);	//add by ly 2014/3/25
	LuaRegisterFunction(KickPlayer);
	LuaRegisterFunction(InitNewPlayerGloryInfo);
	LuaRegisterFunction(SendClientGloryInfo);
	LuaRegisterFunction(QstGetGloryAward); 
	LuaRegisterFunction(QstHandleTitleMsg); 
	LuaRegisterFunction(SetKillAllMonster);
	LuaRegisterFunction(SendArenaRewardMsg);
	LuaRegisterFunction(SendArenaQuestMsg);
	LuaRegisterFunction(getmonsterdatanew);
	LuaRegisterFunction(GetBuffData);
	LuaRegisterFunction(MoveToPos);
	LuaRegisterFunction(GetDynamicRegionID);
	LuaRegisterFunction(GetPetOwer);
	LuaRegisterFunction(CheckObject);
	LuaRegisterFunction(GetSceneRegionID);


	LuaRegisterFunction(AddBuff);
	LuaRegisterFunction(SetPlayerGMLevel);
	LuaRegisterFunction(BaseRelive);
	LuaRegisterFunction(CreateMonsterRegion);
	//LuaRegisterFunction(SetMapCollision);
	LuaRegisterFunction(GetOnlinePlayerNum);
	LuaRegisterFunction(SendMessageToAllPlayer);
	LuaRegisterFunction(PutPlayerToDyncRegion); 
	LuaRegisterFunction(SendSysCall); 
	LuaRegisterFunction(SendGmExceResult);

	LuaRegisterFunction(ResetDailyByTime);
	LuaRegisterFunction(ReturnDailyListInfo);

	LuaRegisterFunction(SendAddFriendToList);
	LuaRegisterFunction(SendFriendFailMsg)
	LuaRegisterFunction(GetPlayerSidByName);
	LuaRegisterFunction(SendFriendList);
	LuaRegisterFunction(GetAllPlayer);
	LuaRegisterFunction(GetAllPlayerSid);
	LuaRegisterFunction(SendTreasureResult);
	LuaRegisterFunction(SavePlayerToDB);
	LuaRegisterFunction(GetServerID);

	//军团
	LuaRegisterFunction(SendCreateFactionResult);
	LuaRegisterFunction(SendJoinResult);
	LuaRegisterFunction(SendManagerFaction);
	LuaRegisterFunction(SendFactioninfo);
	LuaRegisterFunction(SendFactionList);
	LuaRegisterFunction(SDeleteFaction);
	LuaRegisterFunction(SendSynFaction);
	LuaRegisterFunction(SendFactionSceneList);
	LuaRegisterFunction(SendSceneInfo);
	LuaRegisterFunction(SendChallengeSceneMsg);
	LuaRegisterFunction(SaveAllFactionData);
	LuaRegisterFunction(SendInitPrayer);
	LuaRegisterFunction(SendPrayerResult);
	LuaRegisterFunction(SendShopItemList);
	LuaRegisterFunction(SendShopBuyResult);
	LuaRegisterFunction(SendMailToPlayer);
	LuaRegisterFunction(SendFactionSceneRank);
	LuaRegisterFunction(SendFactionSalaryData);
	LuaRegisterFunction(SendGetSalaryResult);
	LuaRegisterFunction(SendActivityCodeResult);
	LuaRegisterFunction(SendScriptData);
	LuaRegisterFunction(SendFactionNotice);
	LuaRegisterFunction(SendModifyNoticeResult);
	LuaRegisterFunction(SendFactionLog);
	LuaRegisterFunction(SendFcEmailResult);
	LuaRegisterFunction(SendEquiptList);
	LuaRegisterFunction(SendRqEquiptResult);
	LuaRegisterFunction(SendRequestStatus);
	LuaRegisterFunction(SendCanceledResult);

	//LuaRegisterFunction(SendCreateFactionResult);
#undef  LuaRegisterFunction
}

int CScriptManager::L_SendSceneFinishMsg(lua_State *L)
{
	CPlayer *pPlayer = 0;
	if (lua_isnumber(L,1))
	{
		DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerByGID(gid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	SASceneFinish safinish;
	CSceneMap::GetInstance().SendSceneFinishMsg(pPlayer, &safinish, sizeof(SASceneFinish));
	return 1;
}

int CScriptManager::L_SendQualityInfo(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	SAEquipQualityInfo sinfo;
	if (lua_istable(L, 1)&&lua_istable(L,2))
	{
		GetTableNumber(L, 1, "exchangeitem", sinfo.dnextqualityitem);
		GetTableNumber(L, 1, "needmoney", sinfo.dmoneycost);
		GetTableNumber(L, 1, "Success", sinfo.bQualitysuccessrate);
		GetTableNumber(L, 1, "needLevel", sinfo.wneedLevel);
		
		int nsize = luaL_getn(L, 2);
		int *bmaterialBuffer = (int*)sinfo.bmaterial;
		memset(bmaterialBuffer, 0, 512);
		for (int i = 0; i < nsize; i++)
		{
			GetTableArrayNumber(L, 2, i + 1, *bmaterialBuffer);
			bmaterialBuffer++;
		}
		//WORD wmsgsize = sizeof(SAEquipQualityInfo)-sizeof(sinfo.bmaterial) + (bmaterialBuffer - (int*)sinfo.bmaterial);
		WORD wmsgsize = sizeof(SAEquipQualityInfo);
		g_Script.m_pPlayer->SendQualityInfo(&sinfo,wmsgsize);
		return 1;
	}
	return 0;
}

int CScriptManager::L_DeleteItem(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L,1)&&lua_isnumber(L,2))
	{
		DWORD itemid = static_cast<DWORD>(lua_tonumber(L, 1));
		WORD num = static_cast<WORD>(lua_tonumber(L, 2));
		if (itemid > 0 && num > 0)
		{
			BOOL isdel = g_Script.m_pPlayer->DeleteItem(itemid, num);
			if (isdel)
			{
				lua_pushnumber(L, 1);
			}
			else
			{
				lua_pushnumber(L, 0);
			}
			return 1;
		}
	}
	return 0;
}

int CScriptManager::L_DelEquipment(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L, 1))
	{
		BYTE bpos = static_cast<BYTE>(lua_tonumber(L, 1));
		BOOL isdel = g_Script.m_pPlayer->DelEquipment(bpos);
		if (isdel)
		{
			lua_pushnumber(L, 1);
		}
		else
		{
			lua_pushnumber(L, 0);
		}
		return 1;
	}
	return 0;
}

int CScriptManager::L_AddItem(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L, 1))
	{
		DWORD itemid = static_cast<DWORD>(lua_tonumber(L, 1));
		SAddItemInfo iteninfo(itemid, 1);
		int npos = g_Script.m_pPlayer->AddItem(iteninfo);
		lua_pushnumber(L, npos);
		return 1;
	}
	lua_pushnumber(L, -1);
	return 0;
}

int CScriptManager::L_RecvUseItem(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	SQUseItemMsg susemsg;
	if (lua_isnumber(L,1))
	{
		WORD bpos = static_cast<WORD>(lua_tonumber(L, 1));
		susemsg.wCellPos = bpos;
		g_Script.m_pPlayer->RecvUseItem(&susemsg);
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendQualityResult(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
	{
		SAStartQuality aquality;
		aquality.result = static_cast<WORD>(lua_tonumber(L, 1));
		aquality.index = static_cast<DWORD>(lua_tonumber(L, 2));
		g_Script.m_pPlayer->SendQualityResult(&aquality);
		return 1;
	}
	return 0;
}

int CScriptManager::L_GetItemBaseData(lua_State *L)
{
	if (lua_isnumber(L, 1))
	{
		DWORD itemid = static_cast<DWORD>(lua_tonumber(L, 1));
		const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(itemid);
		if (pData)
		{
			lua_createtable(L, 0, 0);
			SetTableNumber(L, "m_Overlay", pData->m_Overlay);
			SetTableNumber(L, "m_Level", pData->m_Level);
			SetTableNumber(L, "m_MaxWear", pData->m_MaxWear);
			SetTableNumber(L, "m_Type", pData->m_Type);
			SetTableNumber(L, "m_School", pData->m_School);
			SetTableNumber(L, "m_Sex", pData->m_Sex);
			return 1;
		}
	}
	lua_pushnil(L);
	return 0;
}

int CScriptManager::L_GetIsNewPlayer(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (g_Script.m_pPlayer->m_Property.m_bStopTime == 0)
	{
		lua_pushnumber(L, 1);
	}
	else
	{
		lua_pushnumber(L, 0);
	}
	return 1;
}

int CScriptManager::L_SetIsNewPlayer(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L,1))
	{
		BYTE bnew = static_cast<BYTE>(lua_tonumber(L, 1));
		g_Script.m_pPlayer->m_Property.m_bStopTime = bnew;
	}
	return 1;
}

int CScriptManager::L_KillAllMonster(lua_State *L)
{
	if (lua_isnumber(L,1))
	{
		DWORD reginid = static_cast<DWORD>(lua_tonumber(L, 1));
		DWORD pgid = static_cast<DWORD>(lua_tonumber(L, 2));
		CPlayer*	pPlayer = (CPlayer*)GetPlayerByGID(pgid)->DynamicCast(IID_PLAYER);
		CRegion *regin = (CRegion*)FindRegionByID(reginid)->DynamicCast(IID_REGION);
		if (regin)
		{
			regin->KillMonster(pPlayer);
			return 1;
		}
		CDynamicRegion* pregin = (CDynamicRegion *)FindRegionByGID(reginid)->DynamicCast(IID_DYNAMICREGION);
		if (pregin)
		{
			pregin->KillMonster(pPlayer);
			return 1;
		}

		CDynamicDailyArea* pDailyArea = (CDynamicDailyArea *)FindRegionByGID(reginid)->DynamicCast(IID_DYNAMICDAILYAREA);
		if (pDailyArea)
		{
			pDailyArea->KillMonster(pPlayer);
			return 1;
		}
	}
	return 0;
}

int CScriptManager::L_SendCleanColdResult(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L,1))
	{
		SACleanColdtime  sacleantime;
		sacleantime.bresult = static_cast<BYTE>(lua_tonumber(L, 1));
		g_Script.m_pPlayer->SendCleanColdTimeResult(&sacleantime);
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendRiseStarInfo(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_istable(L,1))
	{
		SARiseStarInfo  sriseinfo;
		GetTableNumber(L, 1, "AddPropertyID", sriseinfo.starpropertyid);
		GetTableNumber(L, 1, "successrate", sriseinfo.bsuccessrate);
		GetTableNumber(L, 1, "needmaterialnum", sriseinfo.dmaterialnum);
		GetTableNumber(L, 1, "RefineStarValue", sriseinfo.starvalue);
		GetTableNumber(L, 1, "moneyconst", sriseinfo.dmoney);
		GetTableNumber(L, 1, "needmaterialid", sriseinfo.dmaterialid);
		g_Script.m_pPlayer->SendRiseStarInfo(&sriseinfo, sizeof(SARiseStarInfo));
		lua_pushnumber(L, 1);
		return 1;
	}
	lua_pushnumber(L, 0);
	return 1;
}

int CScriptManager::L_SendRiseStarResult(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L,1))
	{
		SABeginRefineStar saRefineStar;
		saRefineStar.result = static_cast<BYTE>(lua_tonumber(L, 1));
		g_Script.m_pPlayer->SendMsgToPlayer(&saRefineStar, sizeof(SABeginRefineStar));
		lua_pushnumber(L, 1);
		return 1;
	}
	lua_pushnumber(L, 0);
	return 1;
}

int CScriptManager::L_ChangeObjectData(lua_State *L)
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
	{
		return 0;
	}
	DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
	int index = static_cast<int>(lua_tonumber(L, 2));
	int nvalue = static_cast<int>(lua_tonumber(L, 3));
	LPIObject pobject = GetObjectByGID(gid);
	CFightObject *pfightobj = (CFightObject*)pobject->DynamicCast(IID_FIGHTOBJECT);
	if (!pfightobj)
	{
		return 0;
	}
	CPlayer *pPlayer = (CPlayer*)pfightobj->DynamicCast(IID_PLAYER);
	switch (index)
	{
	case 0:		//当前攻击值
		pfightobj->m_GongJi += nvalue;
		if (pPlayer)
		{
			pPlayer->m_Property.m_JingGong += nvalue;
			//add by ly 2014/5/6 同步玩家战斗力数据
			pPlayer->m_PlayerFightPower = pPlayer->GetPlayerFightPower();	//获取玩家战斗力
			pPlayer->m_PlayerPropertyStatus[XA_FIGHTPOWER - XA_MAX_EXP] = true;
		}
		pfightobj->m_FightPropertyStatus[XA_GONGJI] = true;	
		break;
	case 1:		//当前防御值
		pfightobj->m_FangYu += nvalue;
		if (pPlayer)
		{
			pPlayer->m_Property.m_FangYu += nvalue;
			//add by ly 2014/5/6 同步玩家战斗力数据
			pPlayer->m_PlayerFightPower = pPlayer->GetPlayerFightPower();	//获取玩家战斗力
			pPlayer->m_PlayerPropertyStatus[XA_FIGHTPOWER - XA_MAX_EXP] = true;
		}
		pfightobj->m_FightPropertyStatus[XA_FANGYU] = true;
		break;
	case 2:		//暴击
		pfightobj->m_BaoJi += nvalue;
		pfightobj->m_FightPropertyStatus[XA_BAOJI] = true;
		if (pPlayer)
		{
			//add by ly 2014/5/6 同步玩家战斗力数据
			pPlayer->m_PlayerFightPower = pPlayer->GetPlayerFightPower();	//获取玩家战斗力
			pPlayer->m_PlayerPropertyStatus[XA_FIGHTPOWER - XA_MAX_EXP] = true;
		}
		break;
	case 10:		// 等级
		lua_pushnumber(L, pfightobj->m_Level);
		break;

	case 100:		// 性别
		//lua_pushnumber(L, pPlayer->m_Property.m_Sex);
		break;
	case 4:		//最大生命值
		if (pPlayer)
		{
			DWORD MaxHp = pPlayer->m_BaseMaxHp;
			if (MaxHp + nvalue > 0)
			{
				pPlayer->m_BaseMaxHp += nvalue;
				pfightobj->m_FightPropertyStatus[XA_MAX_HP] = true;
				if (pPlayer)
				{
					//add by ly 2014/5/6 同步玩家战斗力数据
					pPlayer->m_PlayerFightPower = pPlayer->GetPlayerFightPower();	//获取玩家战斗力
					pPlayer->m_PlayerPropertyStatus[XA_FIGHTPOWER - XA_MAX_EXP] = true;
				}
			}
		}
		//pfightobj->m_MaxHp += nvalue;
		
		break;
	case 5:		// 帮派名字
		//lua_pushstring(L, pPlayer->m_Property.m_szTongName);
		break;
	case 6:		//侠义值
		//lua_pushnumber(L, pPlayer->m_Property.m_XYValue);
		break;
	case 7:		//杀孽值
		//lua_pushnumber(L, pPlayer->m_wPKValue);
		break;
	case 8:		//精力值
		//lua_pushnumber(L, pPlayer->m_Property.m_CurJp);
		break;
	case 9:		//经验值
		//lua_pushnumber(L, pPlayer->m_Property.m_Exp);
		break;
		//case 10:		//真气值
		//	lua_pushnumber(L, pPlayer->m_Property.m_CurSp);
		//	break;
	case 11:		//当前移动速度值
		lua_pushnumber(L, pfightobj->m_OriSpeed);
		break;
	case 12:		//当前攻击速度
		lua_pushnumber(L, pfightobj->m_AtkSpeed);
		break;
	case 13:		//当前轻功点数
		//lua_pushnumber(L, pPlayer->m_Property.m_QingShen);
		break;
	case 14:		//当前强健点数
		//	lua_pushnumber(L, pPlayer->m_Property.m_JianShen);
		break;
	case 15:		// 配偶名字
		//lua_pushnumber(L, pPlayer->m_Property.m_Sex);
		break;
	case 16:		//最大内力值
		lua_pushnumber(L, pfightobj->m_MaxMp);
		break;
	case 17:		//最大体力值
		pfightobj->m_MaxTp += nvalue;
		pfightobj->m_FightPropertyStatus[XA_MAX_TP] = true;	
		if (pPlayer)
		{
			//add by ly 2014/5/6 同步玩家战斗力数据
			pPlayer->m_PlayerFightPower = pPlayer->GetPlayerFightPower();	//获取玩家战斗力
			pPlayer->m_PlayerPropertyStatus[XA_FIGHTPOWER - XA_MAX_EXP] = true;
		}
		break;
	case 18:		//当前生命值
		//if (pfightobj->m_CurHp + nvalue > 0)
		//{
			//pfightobj->m_CurHp += nvalue;
		//}
		//pfightobj->m_FightPropertyStatus[XA_CUR_HP] = true;

		pfightobj->ModifyCurrentHP(nvalue, 0, 0);
		break;
	case 19:		//当前内力值
		lua_pushnumber(L, pfightobj->m_CurMp);
		break;
	case 20:		//当前体力值
		pfightobj->ModifyCurrentTP(nvalue, 0, 0);

		break;
	case 23:		// 怪物ID
		break;
	case 3:		//当前闪避值
	pfightobj->m_ShanBi += nvalue;
	pfightobj->m_FightPropertyStatus[XA_SHANBI] = true;
	if (pPlayer)
	{
		//add by ly 2014/5/6 同步玩家战斗力数据
		pPlayer->m_PlayerFightPower = pPlayer->GetPlayerFightPower();	//获取玩家战斗力
		pPlayer->m_PlayerPropertyStatus[XA_FIGHTPOWER - XA_MAX_EXP] = true;
	}
	break;
	case 25:		//当前防御点数
		break;
	case 26:		//当前进攻点数
		break;
	}
	return 1;
}

int CScriptManager::L_SendEquiptKeYinInfo(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_istable(L, 1))
	{
		SAKeYinInfo sekeyininfo;
		GetTableNumber(L, 1, "successrate", sekeyininfo.bsuccessrate);
		GetTableNumber(L, 1, "needmaterialnum", sekeyininfo.dmaterialnum);
		GetTableNumber(L, 1, "needmaterial", sekeyininfo.dmaterialid);
		GetTableNumber(L, 1, "moneyconst", sekeyininfo.dmoney);
		g_Script.m_pPlayer->SendEquiptKeYinInfo(&sekeyininfo, sizeof(SAKeYinInfo));
		lua_pushnumber(L, 1);
		return 1;
	}
	lua_pushnumber(L, 0);
	return 1;
}

int CScriptManager::L_SendEquipJDingResult(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_istable(L, 1))
	{
		SAEquipJDing sequipjding;
		GetTableNumber(L, 1, "bPropertyIndex", sequipjding.bPropertyIndex);
		GetTableNumber(L, 1, "bPropertyID", sequipjding.bPropertyID);
		GetTableNumber(L, 1, "wvalue", sequipjding.wvalue);
		GetTableNumber(L, 1, "color", sequipjding.bColor);
		g_Script.m_pPlayer->SendEquipJDingResult(&sequipjding, sizeof(SAEquipJDing));
		lua_pushnumber(L, 1);
		return 1;
	}
	lua_pushnumber(L, 0);
	return 1;
}

int CScriptManager::L_SendEquipJDingInfo(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_istable(L, 1))
	{
		SAEquipJDingInfo sequipjdinginfo;
		GetTableNumber(L, 1, "dmaterialnum", sequipjdinginfo.dmaterialnum);
		GetTableNumber(L, 1, "dlockmaterialnum", sequipjdinginfo.dlockmaterialnum);
		GetTableNumber(L, 1, "dmaterialid", sequipjdinginfo.dmaterialid);
		GetTableNumber(L, 1, "dlockmaterialid", sequipjdinginfo.dlockmaterialid);
		g_Script.m_pPlayer->SendEquipJDingInfo(&sequipjdinginfo, sizeof(SAEquipJDingInfo));
		lua_pushnumber(L, 1);
		return 1;
	}
	lua_pushnumber(L, 0);
	return 1;
}

int CScriptManager::L_SendEquipJDingColor(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	SAEquipJDingColor eqjdingcolor;
	if (lua_isnumber(L,1) && lua_isnumber(L,2))
	{
		eqjdingcolor.bType = static_cast<BYTE>(lua_tonumber(L, 1));
		eqjdingcolor.bIndex = static_cast<BYTE>(lua_tonumber(L, 2));
		if (lua_istable(L, 3))
		{
			for (size_t i = 0; i < COLORNUM; i++)
			{
				GetTableArrayNumber(L,3, i + 1, eqjdingcolor.bcolorIndex[i]);
			}
			g_Script.m_pPlayer->SendEquipJDingColor(&eqjdingcolor, sizeof(SAEquipJDingColor));
		}
	}
	return 1;
}

int CScriptManager::L_SetTaskFinish(lua_State *L)
{
	CPlayer *pPlayer = g_Script.m_pPlayer;
	WORD taskid = 0;
	if (!pPlayer)
	{
		if (lua_isnumber(L,1))
		{
			DWORD pysid = static_cast<DWORD>(lua_tonumber(L, 1));
			pPlayer = (CPlayer*)GetPlayerBySID(pysid)->DynamicCast(IID_PLAYER);
			if (!pPlayer)
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
		taskid = static_cast<DWORD>(lua_tonumber(L, 2));
	}
	else
	{
		taskid = static_cast<DWORD>(lua_tonumber(L, 1));
	}

	if (pPlayer)
	{
		if (CRoleTask *task = pPlayer->m_TaskManager.GetRoleTask(taskid))
		{
			for (CRoleTask::TELIST::iterator it = task->m_flagList.begin(); it != task->m_flagList.end(); ++it)
			{
				if ( it->m_Complete == 0)
				{
					it->m_Complete = 1;

					// 发送更新旗标的消息
					pPlayer->SendUpdateTaskFlagMsg(*it, true);

					lua_pushnumber(L, 1);	//判断该操作是否执行，返回执行结果
					break;
				}
			}
		}
	}
	return 1;
}

int CScriptManager::L_SendKeYinChangeData(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	SAKeYinChange skeyinchange;
	if (lua_istable(L, 1))
	{
		
		int nsize = luaL_getn(L, 1);
		if (nsize > 3)
		{
			return 0;
		}
		for (int i = 0; i < nsize; i++)
		{
			GetTableArrayNumber(L, 1, i + 1, skeyinchange.bPropertyIndex[i]);
		}
	}
	if (lua_istable(L,2))
	{
		GetTableNumber(L, 2, "moneyconst", skeyinchange.dmoney);
		GetTableNumber(L, 2, "itemchange", skeyinchange.dmaterialid);
		GetTableNumber(L, 2, "itemchangenum", skeyinchange.dmaterialnum);
		g_Script.m_pPlayer->SendKeYinChangeData(&skeyinchange);
	}
	return 1;
}

int CScriptManager::L_InitPlayerData(lua_State *L)
{
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_istable(L, 3))
	{
		WORD wjob = static_cast<WORD>(lua_tonumber(L, 1));
		WORD wlevel = static_cast<WORD>(lua_tonumber(L, 2));
		SBaseAttribute  splayerattri;
		GetTableNumber(L, 3, "m_BaseHp", splayerattri.m_BaseHp);
		GetTableNumber(L, 3, "m_BaseJp", splayerattri.m_BaseJp);
		GetTableNumber(L, 3, "m_GongJi", splayerattri.m_GongJi);
		GetTableNumber(L, 3, "m_FangYu", splayerattri.m_FangYu);
		GetTableNumber(L, 3, "m_Hit", splayerattri.m_Hit);
		GetTableNumber(L, 3, "m_ShanBi", splayerattri.m_ShanBi);
		GetTableNumber(L, 3, "m_BaoJi", splayerattri.m_BaoJi);
		GetTableNumber(L, 3, "m_uncrit", splayerattri.m_uncrit);
		GetTableNumber(L, 3, "m_wreck", splayerattri.m_wreck);
		GetTableNumber(L, 3, "m_unwreck", splayerattri.m_unwreck);
		GetTableNumber(L, 3, "m_puncture", splayerattri.m_puncture);
		GetTableNumber(L, 3, "m_unpuncture", splayerattri.m_unpuncture);
		GetTableNumber(L, 3, "m_BaseTp", splayerattri.m_BaseTp);
		GetTableNumber(L, 3, "m_TpRecover", splayerattri.m_TpRecover);
		GetTableNumber(L, 3, "m_TpRecoverInterval", splayerattri.m_TpRecoverInterval);
		GetTableNumber(L, 3, "m_Exp", splayerattri.m_Exp);
		CPlayerService::GetInstance().Init(wjob, wlevel, &splayerattri);
	}
	return 1;
}

int CScriptManager::L_ReloadPlayerData(lua_State *L)
{
	CPlayerService::GetInstance().ReloadPlayerData();
	return 1;
}

int CScriptManager::L_SendSuitcondition(lua_State *L)
{ 
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	SASuitcondition	Suitcondition;
	if (lua_isnumber(L, 1) && lua_istable(L, 2) && lua_istable(L, 3) && lua_istable(L, 4) && lua_isnumber(L, 5))
	{
		Suitcondition.btype = static_cast<BYTE>( lua_tonumber(L, 1));
		memset(Suitcondition.benough, 0, sizeof(BYTE)*EQUIP_P_MAX);
		memset(Suitcondition.nenoughproperty, 0, sizeof(int)*3);
		memset(Suitcondition.ncurrentproperty, 0, sizeof(int)* 3);
		memset(Suitcondition.wepropertyindex, 0, sizeof(WORD)* 3);
		memset(Suitcondition.wcepropertyindex, 0, sizeof(WORD)* 3);

		int nsize = luaL_getn(L, 2);
		if (nsize > EQUIP_P_MAX)
		{
			return 0;
		}
		for (size_t i = 0; i < nsize; i++)
		{
			GetTableArrayNumber(L, 2, i + 1, Suitcondition.benough[i]);
		}
		
		 nsize = luaL_getn(L, 3);
		if (nsize > 6)
		{
			return 0;
		}
		for (size_t i = 0; i < nsize; i++)
		{
			if (i % 2 == 0)  //当前属性ID
			{
				GetTableArrayNumber(L,3, i + 1, Suitcondition.wcepropertyindex[i/2]);
			}
			else  //当前属性值
			{
				GetTableArrayNumber(L, 3, i + 1, Suitcondition.ncurrentproperty[(i-1)/2]);
			}
		}

		nsize = luaL_getn(L, 4);
		if (nsize > 6)
		{
			return 0;
		}
		for (size_t i = 0; i < nsize; i++)
		{
			if (i % 2 == 0)  //满足条件增加的属性ID
			{
				GetTableArrayNumber(L,4, i + 1, Suitcondition.wepropertyindex[i /2]);
			}
			else  //满足条件增加的属性值
			{
				GetTableArrayNumber(L, 4, i + 1, Suitcondition.nenoughproperty[(i-1)/2]);
			}
		}
		Suitcondition.wcurrentlevel = static_cast<WORD>(lua_tonumber(L, 5));
		WORD msgsize = sizeof(SASuitcondition);
		g_Script.m_pPlayer->SendSuitcondition(&Suitcondition, msgsize);
	}
	return 1;
}

int CScriptManager::L_SendDebrisinfo(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	SADebrisinfo pSceneMsg;
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
	{
		pSceneMsg.bbadgeindex = static_cast<BYTE>(lua_tonumber(L, 1));
		pSceneMsg.bDebrisnum = static_cast<BYTE>(lua_tonumber(L, 2));
		pSceneMsg.bDebrisAllnum = static_cast<BYTE>(lua_tonumber(L, 3));
		WORD wmsglenth = sizeof(SADebrisinfo);
		CSceneMap::GetInstance().SendDebrisinfo(g_Script.m_pPlayer, &pSceneMsg, wmsglenth);
	}
	return 1;
}

int CScriptManager::L_SendDebrisAward(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	SADebrisAward pSceneMsg;
	WORD wmsglenth = sizeof(SADebrisAward);
	CSceneMap::GetInstance().SendDebrisAward(g_Script.m_pPlayer, &pSceneMsg, wmsglenth);
	return 1;
}

int CScriptManager::L_GetTransformersLevel(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L, 1))
	{
		BYTE bTransformerIndex = static_cast<BYTE>(lua_tonumber(L, 1));
		//BYTE level = g_Script.m_pPlayer->GetTransformersLevel(bTransformerIndex);
		lua_pushnumber(L, level);
		return 1;
	}
	return 0;
}

int CScriptManager::L_SetTransformersLevel(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L, 1) && lua_isnumber(L,2))
	{
		BYTE bTransformerIndex = static_cast<BYTE>(lua_tonumber(L, 1));
		BYTE bTransformerlevel = static_cast<BYTE>(lua_tonumber(L, 2));
//		g_Script.m_pPlayer->SetTransformersLevel(bTransformerIndex, bTransformerlevel);
		return 1;
	}
	return 0;
}

int CScriptManager::L_OnSendTransformersInfoMsg(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_istable(L,1))
	{
		SATransformersInfo sTransformersInfo;
		GetTableNumber(L, 1, "needmaterial", sTransformersInfo.dmaterialIndex);
		GetTableNumber(L, 1, "needmaterialnum", sTransformersInfo.dmaterialnum);
		GetTableNumber(L, 1, "HpValue", sTransformersInfo.dcurrentHP);
		GetTableNumber(L, 1, "money", sTransformersInfo.dmoney);
		GetTableNumber(L, 1, "HpnextValue", sTransformersInfo.dnextHP);
//		g_Script.m_pPlayer->OnSendTransformersInfoMsg(&sTransformersInfo, sizeof(SATransformersInfo));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendTransformersSwitch(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L,1))
	{
		SATransformersSwitch TransformersSwitch;
		TransformersSwitch.bresult = static_cast<BYTE>(lua_tonumber(L, 1));
//		g_Script.m_pPlayer->SendTransformersSwitch(&TransformersSwitch, sizeof(SATransformersSwitch));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendMyState(lua_State *L)
{
	if (lua_isnumber(L,1))
	{
		DWORD pygid = static_cast<DWORD>(lua_tonumber(L, 1));
		CPlayer *pPlayer = (CPlayer*)GetPlayerByGID(pygid)->DynamicCast(IID_PLAYER);
		if (!pPlayer)
		{
			return 0;
		}
		pPlayer->SendMyState();
	}
	return 1;
}

int CScriptManager::L_InitPlayerProperty(lua_State *L)
{
	if (lua_isnumber(L, 1))
	{
		DWORD pygid = static_cast<DWORD>(lua_tonumber(L, 1));
		CPlayer *pPlayer = (CPlayer*)GetPlayerByGID(pygid)->DynamicCast(IID_PLAYER);
		if (!pPlayer)
		{
			return 0;
		}
		pPlayer->InitEquipmentData();
		pPlayer->UpdateAllProperties();
	}
	return 1;
}

int CScriptManager::L_SetObjectData(lua_State *L)
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
	{
		return 0;
	}
	DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
	int index = static_cast<int>(lua_tonumber(L, 2));
	int nvalue = static_cast<int>(lua_tonumber(L, 3));
	if (nvalue < 0)
	{
		return 0;
	}

	LPIObject pobject = GetObjectByGID(gid);
	CFightObject *pfightobj = (CFightObject*)pobject->DynamicCast(IID_FIGHTOBJECT);
	if (!pfightobj)
	{
		return 0;
	}
	CPlayer *pPlayer = (CPlayer*)pfightobj->DynamicCast(IID_PLAYER);
	switch (index)
	{
	case 0:		//当前攻击值
		pfightobj->m_GongJi = nvalue;
		pfightobj->m_FightPropertyStatus[XA_GONGJI] = true;
		if (pPlayer)
		{
			//add by ly 2014/5/6 同步玩家战斗力数据
			pPlayer->m_PlayerFightPower = pPlayer->GetPlayerFightPower();	//获取玩家战斗力
			pPlayer->m_PlayerPropertyStatus[XA_FIGHTPOWER - XA_MAX_EXP] = true;
		}
		break;
	case 1:		//当前防御值
		pfightobj->m_FangYu = nvalue;
		pfightobj->m_FightPropertyStatus[XA_FANGYU] = true;
		if (pPlayer)
		{
			//add by ly 2014/5/6 同步玩家战斗力数据
			pPlayer->m_PlayerFightPower = pPlayer->GetPlayerFightPower();	//获取玩家战斗力
			pPlayer->m_PlayerPropertyStatus[XA_FIGHTPOWER - XA_MAX_EXP] = true;
		}
		break;
	case 2:		//暴击
		pfightobj->m_BaoJi = nvalue;
		pfightobj->m_FightPropertyStatus[XA_BAOJI] = true;
		if (pPlayer)
		{
			//add by ly 2014/5/6 同步玩家战斗力数据
			pPlayer->m_PlayerFightPower = pPlayer->GetPlayerFightPower();	//获取玩家战斗力
			pPlayer->m_PlayerPropertyStatus[XA_FIGHTPOWER - XA_MAX_EXP] = true;
		}
		break;
	case 10:		// 等级
		lua_pushnumber(L, pfightobj->m_Level);
		break;

	case 100:		// 性别
		//lua_pushnumber(L, pPlayer->m_Property.m_Sex);
		break;
	case 4:		//最大生命值
// 		if (pPlayer)
// 		{
// 			DWORD MaxHp = pPlayer->m_BaseMaxHp;
// 			if (MaxHp + nvalue > 0)
// 			{
// 				pPlayer->m_BaseMaxHp += nvalue;
// 				pfightobj->m_FightPropertyStatus[XA_MAX_HP] = true;
// 			}
// 		}
		pfightobj->m_MaxHp = nvalue;
		pfightobj->m_FightPropertyStatus[XA_MAX_HP] = true;
		if (pPlayer)
		{
			//add by ly 2014/5/6 同步玩家战斗力数据
			pPlayer->m_PlayerFightPower = pPlayer->GetPlayerFightPower();	//获取玩家战斗力
			pPlayer->m_PlayerPropertyStatus[XA_FIGHTPOWER - XA_MAX_EXP] = true;
		}
		break;
	case 5:		// 帮派名字
		//lua_pushstring(L, pPlayer->m_Property.m_szTongName);
		break;
	case 6:		//侠义值
		//lua_pushnumber(L, pPlayer->m_Property.m_XYValue);
		break;
	case 7:		//杀孽值
		//lua_pushnumber(L, pPlayer->m_wPKValue);
		break;
	case 8:		//精力值
		//lua_pushnumber(L, pPlayer->m_Property.m_CurJp);
		break;
	case 9:		//经验值
		//lua_pushnumber(L, pPlayer->m_Property.m_Exp);
		break;
	case 21:		//真气值
		if (pPlayer)
		{
			pPlayer->m_Property.m_CurSp = nvalue;
			if (pPlayer->m_Property.m_CurSp > pPlayer->m_MaxSp)
			{
				pPlayer->m_Property.m_CurSp = pPlayer->m_MaxSp;
			}
			pPlayer->m_PlayerPropertyStatus[XA_CUR_SP - XA_MAX_EXP] = true;

		}
	break;
	case 11:		//当前移动速度值
		lua_pushnumber(L, pfightobj->m_OriSpeed);
		break;
	case 12:		//当前攻击速度
		lua_pushnumber(L, pfightobj->m_AtkSpeed);
		break;
	case 13:		//当前轻功点数
		//lua_pushnumber(L, pPlayer->m_Property.m_QingShen);
		break;
	case 14:		//当前强健点数
		//	lua_pushnumber(L, pPlayer->m_Property.m_JianShen);
		break;
	case 15:		// 配偶名字
		//lua_pushnumber(L, pPlayer->m_Property.m_Sex);
		break;
	case 16:		//最大内力值
		//lua_pushnumber(L, pfightobj->m_MaxMp);
		pfightobj->m_MaxMp = nvalue;
		pfightobj->m_FightPropertyStatus[XA_MAX_MP] = true;
		break;
	case 17:		//最大体力值
		pfightobj->m_MaxTp = nvalue;
		pfightobj->m_FightPropertyStatus[XA_MAX_TP] = true;
		if (pPlayer)
		{
			//add by ly 2014/5/6 同步玩家战斗力数据
			pPlayer->m_PlayerFightPower = pPlayer->GetPlayerFightPower();	//获取玩家战斗力
			pPlayer->m_PlayerPropertyStatus[XA_FIGHTPOWER - XA_MAX_EXP] = true;
		}
		break;
	case 18:		//当前生命值
		pfightobj->m_CurHp = nvalue;
		pfightobj->m_FightPropertyStatus[XA_CUR_HP] = true;
		break;
	case 19:		//当前内力值
		//lua_pushnumber(L, pfightobj->m_CurMp);
		pfightobj->m_CurMp = nvalue;
		if (pfightobj->m_CurMp > pfightobj->m_MaxMp)
			pfightobj->m_CurMp = pfightobj->m_MaxMp;
		pfightobj->m_FightPropertyStatus[XA_CUR_MP] = true;
		break;
	case 20:		//当前体力值
		pfightobj->m_CurTp = nvalue;
		if (pfightobj->m_CurTp > pfightobj->m_MaxTp)
		{
			pfightobj->m_CurTp = pfightobj->m_MaxTp;
		}
		pfightobj->m_FightPropertyStatus[XA_CUR_TP] = true;
		break;
	case 23:		// 怪物ID
		break;
	case 3:		//当前闪避值
		pfightobj->m_ShanBi = nvalue;
		pfightobj->m_FightPropertyStatus[XA_SHANBI] = true;
		if (pPlayer)
		{
			//add by ly 2014/5/6 同步玩家战斗力数据
			pPlayer->m_PlayerFightPower = pPlayer->GetPlayerFightPower();	//获取玩家战斗力
			pPlayer->m_PlayerPropertyStatus[XA_FIGHTPOWER - XA_MAX_EXP] = true;
		}
		break;
	case 25:		//当前防御点数
		break;
	case 26:		//当前进攻点数
		break;
	}
	return 1;
}

int CScriptManager::L_GetCurrentSkillLevel(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L,1))
	{
		DWORD skillindex = static_cast<DWORD>(lua_tonumber(L, 1));
		INT32 skilllevel = g_Script.m_pPlayer->GetCurrentSkillLevel(skillindex);
		lua_pushnumber(L, skilllevel);
	}
	return 1;
}

int CScriptManager::L_SetCurrentSkillLevel(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L, 1)&&lua_isnumber(L,2))
	{
		DWORD skillindex = static_cast<DWORD>(lua_tonumber(L, 1));
		BYTE skilllevel = static_cast<BYTE>(lua_tonumber(L, 2));
		INT32 isset = g_Script.m_pPlayer->SetCurrentSkillLevel(skillindex,skilllevel);
		lua_pushnumber(L, isset);
	}
	return 1;
}

int CScriptManager::L_GetSkillProficiency(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L, 1))
	{
		DWORD skillindex = static_cast<DWORD>(lua_tonumber(L, 1));
		DWORD SkillProficiency = g_Script.m_pPlayer->GetSkillProficiency(skillindex);
		lua_pushnumber(L, SkillProficiency);
	}
	return 1;
}

int CScriptManager::L_AddSkillProficiency(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L, 1)&&lua_isnumber(L,2))
	{
		DWORD skillindex = static_cast<DWORD>(lua_tonumber(L, 1));
		DWORD SkillProficiency = static_cast<DWORD>(lua_tonumber(L,2));
		int isset = (int)g_Script.m_pPlayer->AddSkillProficiency(skillindex, SkillProficiency);
		lua_pushnumber(L, isset);
	}
	return 1;
}

int CScriptManager::L_SendTransformersSkillInfo(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_istable(L,1))
	{
		SATransformersSkillInfo sformersSkillInfo;
		GetTableNumber(L, 1, "levelUpSkillExp", sformersSkillInfo.levelUpSkillExp);
		GetTableNumber(L, 1, "currentSkillExp", sformersSkillInfo.currentSkillExp);
		GetTableNumber(L, 1, "CurrentTriggerate", sformersSkillInfo.CurrentTriggerate);
		GetTableNumber(L, 1, "LevelUpTriggerate", sformersSkillInfo.LevelUpTriggerate);
		GetTableNumber(L, 1, "skillCurrentdamage", sformersSkillInfo.skillCurrentdamage);
		GetTableNumber(L, 1, "skillLevelUpdamage", sformersSkillInfo.skillLevelUpdamage);
		GetTableNumber(L, 1, "SkillIndex", sformersSkillInfo.wSkillIndex);
		GetTableNumber(L, 1, "silveritem", sformersSkillInfo.silveritem);
		GetTableNumber(L, 1, "goditem", sformersSkillInfo.goditem);
		GetTableNumber(L, 1, "CurrentLevel", sformersSkillInfo.bCurrentLevel);
		WORD msgsize = sizeof(SATransformersSkillInfo);
//		g_Script.m_pPlayer->SendTransformersSkillInfo(&sformersSkillInfo, msgsize);
	}
	return 1;
}

int CScriptManager::L_GetSkillLevel(lua_State *L)
{
	WORD topnum = lua_gettop(L);
	if (topnum == 2)
	{
		DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
		DWORD skillindex = static_cast<DWORD>(lua_tonumber(L, 2));
		CPlayer *Player = (CPlayer*)GetObjectByGID(gid)->DynamicCast(IID_PLAYER);
		if (Player)
		{
			INT32 skilllevel = Player->GetSkillLevel(skillindex);
			lua_pushnumber(L, skilllevel);
			return 1;
		}
	}
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L, 1))
	{
		DWORD skillindex = static_cast<DWORD>(lua_tonumber(L, 1));
		INT32 skilllevel = g_Script.m_pPlayer->GetSkillLevel(skillindex);
		lua_pushnumber(L, skilllevel);
	}
	return 1;
}

int CScriptManager::L_SetSkillLevel(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
	{
		DWORD skillindex = static_cast<DWORD>(lua_tonumber(L, 1));
		BYTE skilllevel = static_cast<BYTE>(lua_tonumber(L, 2));
		INT32 isset = g_Script.m_pPlayer->SetSkillLevel(skillindex, skilllevel);
		lua_pushnumber(L, isset);
	}
	return 1;
}

int CScriptManager::L_GetSceneRegionID(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	DWORD gid = g_Script.m_pPlayer->GetSceneRegionID();
	lua_pushnumber(L, gid);
	return 1;
}
//#define FILESIZE  0x800000   // 1MB
int CScriptManager::L_LoadTable(lua_State *L)
{
	if (lua_isstring(L,1))
	{
		const char* tablename = static_cast<const char *>(lua_tostring(L, 1));
		FILE *file = fopen(tablename, "rb");
		if (file)
		{
			//BYTE Buff[FILESIZE];
			fseek(file, 0, SEEK_END);
			long fsize = ftell(file);
			fseek(file, 0, SEEK_SET);
			if (fsize == 0)
			{
				fsize = FILESIZE;
			}
			BYTE  *Buff = new BYTE[fsize];
			memset(Buff, 0, fsize * sizeof(BYTE));
			size_t  lfsize = fread(Buff, sizeof(BYTE), fsize, file);

			lua_createtable(L, 0, 0);
			if (!lua_istable(L, -1))
			{
				SAFE_DELETE_ARRAY(Buff);
				return 0;
			}
			
			int ck = luaEx_unserialize(L, Buff, lfsize);
			if (ck <= 0)
			{
				fclose(file);
				SAFE_DELETE_ARRAY(Buff);
				//lua_pushnumber(L, 0);
				lua_pushnil(L);
				return 1;
			}
			fclose(file);
			SAFE_DELETE_ARRAY(Buff);
			return 1;
		}
	}
	//lua_pushnumber(L, 0);
	lua_pushnil(L);
	return 1;
}

int CScriptManager::L_SaveTable(lua_State *L)
{
	if (lua_isstring(L, 1) && lua_istable(L,2))
	{
		const char* tablename = static_cast<const char *>(lua_tostring(L, 1));
		FILE *file = fopen(tablename, "wb");
		if (file)
		{
			BYTE *luabuffer = new BYTE[FILESIZE];
			memset(luabuffer, 0, FILESIZE * sizeof(BYTE));
			lua_pushvalue(L, 1);
			int cur1 = luaEx_serialize(L, 2, luabuffer, FILESIZE);
			if (cur1 < 0)
			{
				SAFE_DELETE_ARRAY(luabuffer);
				fclose(file);
				lua_pushnumber(L, 0);
				return 1;	
			}
			fwrite(luabuffer, sizeof(BYTE), cur1, file);
			fclose(file);
			lua_pushnumber(L, 1);
			SAFE_DELETE_ARRAY(luabuffer);
			return 1;
		}
	}
	return 1;
}

//add by ly 2014/3/18
int CScriptManager::L_AddPlayerXinYangProperty(lua_State *L)	//增加玩家的属性
{
	if (!g_Script.m_pPlayer)
		return 0;
	BYTE StartUpgradeStar = lua_tonumber(L, 1);
	BYTE UpgradeStarNum = lua_tonumber(L, 2);
	BYTE MoneyType = lua_tonumber(L, 3);
	BYTE byProbability = lua_tonumber(L, 4);
	WORD ConsumeMoney = 0;
	WORD ConsumeSp = 0;

	SAXinYangRiseStarmsg XinYangData;
	XinYangData.m_SuccessNum = 0;


	if (g_Script.m_pPlayer->m_Property.m_JingMai[StartUpgradeStar / 12] < ((StartUpgradeStar % 12) - 1))//表示不能够越级升星
	{
		XinYangData.m_SuccessNum = -2;	
		XinYangData.m_ConsumeMoney = 0; 
		XinYangData.m_ConsumeSP = 0;
		g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &XinYangData, sizeof(SAXinYangRiseStarmsg));
		return 0;
	}

	for (int i = StartUpgradeStar; i < StartUpgradeStar + UpgradeStarNum; i++)	//计算消耗的钱和真气
	{
		const SXinYangBaseData *lpXinYangBaseData = CXinYangService::GetInstance().GetXingXiuDataByID(i);
		if (lpXinYangBaseData == NULL)	//没有该星宿信息
			continue;
		if (MoneyType == 1)		//金币
			ConsumeMoney += lpXinYangBaseData->m_ConsumeJinBi;
		else	//银两
		{
			ConsumeMoney += lpXinYangBaseData->m_ConsumeYinLiang;
			ConsumeSp += lpXinYangBaseData->m_ConsumeSp;
		}
	}
	//判断钱是否足够
	BOOL bFlag = g_Script.m_pPlayer->IsMoneyEnough(MoneyType, ConsumeMoney);
	if (!bFlag)
	{
		if (MoneyType == 1)
			XinYangData.m_SuccessNum = -1;	//表示金币不足
		else
			XinYangData.m_SuccessNum = -4;	//表示银币不足
		XinYangData.m_ConsumeMoney = 0;
		XinYangData.m_ConsumeSP = 0;
		g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &XinYangData, sizeof(SAXinYangRiseStarmsg));
		return 0;
	}

	//判断真气（信仰之力）是否足够
	if (g_Script.m_pPlayer->m_Property.m_CurSp < ConsumeSp)
	{
		XinYangData.m_SuccessNum = -3;	//表示真气不足
		XinYangData.m_ConsumeMoney = 0;
		XinYangData.m_ConsumeSP = 0;
		g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &XinYangData, sizeof(SAXinYangRiseStarmsg));
		return 0;
	}

	WORD RealityConsumeMoney = ConsumeMoney;	//实际消耗钱。消耗金币时时固定价格，不过你要升星几次。
	WORD RealityConsumeSP = 0;		//实际消耗真气
	if (MoneyType == 2)		//银两
	{
		RealityConsumeMoney = 0;
		RealityConsumeSP = 0;
	}
	BYTE CurZhuShenIndex = (StartUpgradeStar - 1) / 12;

	static int SetOnceRandFlag = 0;
	if (SetOnceRandFlag != 1)	//只设置一次随机数生成器
	{
		srand(time(NULL));
		SetOnceRandFlag = 1;	
	}
	for (int i = StartUpgradeStar; i < StartUpgradeStar + UpgradeStarNum; i++)	//升级每一颗星宿
	{
		if (CurZhuShenIndex < (i - 1) / 12)	//已经升满该主神的星宿，不能再升星
			break;

		const SXinYangBaseData *lpXinYangBaseData = CXinYangService::GetInstance().GetXingXiuDataByID(i);
		if (lpXinYangBaseData == NULL)	//没有改星宿信息
			continue;

		if (MoneyType == 2)		//计算实际消耗的银两
		{
			RealityConsumeMoney += lpXinYangBaseData->m_ConsumeYinLiang;
			RealityConsumeSP += lpXinYangBaseData->m_ConsumeSp;		//计算实际消耗的真气
		}
		int iRandValue = 0;
		if (MoneyType == 2)		//有概率的情况
		{
			iRandValue = rand() % 100 + 1;		//随机值
		}
		if (lpXinYangBaseData->m_SuccessRate < iRandValue)	//随机增加属性失败
		{
			i--;
			UpgradeStarNum--;
			continue;
		}
		XinYangData.m_SuccessNum += 1;	//概率性成功
		g_Script.m_pPlayer->m_Property.m_JingMai[(i - 1) / 12] = (i - 1) % 12 + 1;	//玩家升级该星宿
		//这里先不激活，必须等到挑战信仰主神成功后才能激活该主神相应的技能
		//if (i % 12 == 0)	//该主神的12个星宿升级完毕，激活对应的技能
		//	g_Script.m_pPlayer->ActivaSkill(lpXinYangBaseData->m_CanHaveSkillID, "12星宿升级完毕，激活技能");
	}

	g_Script.m_pPlayer->ChangeMoney(MoneyType, -RealityConsumeMoney);	//消耗实际花费的钱
	if (MoneyType == 2)
	{
		g_Script.m_pPlayer->m_Property.m_CurSp -= RealityConsumeSP;		//消耗实际使用的真气
		g_Script.m_pPlayer->m_PlayerPropertyStatus[XA_CUR_SP - XA_MAX_EXP] = true;
	}

	//增加玩家相关属性
	g_Script.m_pPlayer->InitJingMaiData();
	g_Script.m_pPlayer->UpdateAllProperties();
	XinYangData.m_ConsumeMoney = RealityConsumeMoney;
	XinYangData.m_ConsumeSP = RealityConsumeSP;
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &XinYangData, sizeof(SAXinYangRiseStarmsg));
	if (XinYangData.m_SuccessNum > 0) 
	{
		g_Script.CallFunc("OnPlayerXinYangUpdate", XinYangData.m_SuccessNum);
	}
	return 0;
}

//荣耀相关 add by ly 2014/3/25
//初始化新手玩家的荣耀信息
int CScriptManager::L_InitNewPlayerGloryInfo(lua_State *L)
{
	if (!lua_istable(L, 1))
	{
		lua_createtable(L, 0, 0);
		if (!lua_istable(L, -1))
			return 0;
	}
	
	//设置表中的元素,保存的是荣耀的ID和完成状态
	int GloryInfArrSize = 0;
	WORD *pGloryInfArr = CGloryAndTitleServer::GetInstance().ReturnAllGloryID(&GloryInfArrSize);
	for (int i = 1; i <= GloryInfArrSize; i++)
	{
		lua_pushnumber(L, pGloryInfArr[i - 1]);
		lua_rawseti(L, -2, i);
	}
	delete[] pGloryInfArr;
	pGloryInfArr = NULL;
	return 1;
}

//向客户端返回荣耀信息
int CScriptManager::L_SendClientGloryInfo(lua_State *L)
{
	if (!lua_istable(L, 1))
	{
		return 0;
	}
	//获取荣耀点
	lua_getfield(L, -1, "PlayerGloryDot");
	int GloryDot = lua_tonumber(L, -1);
	lua_pop(L, 1);
	
	//获取荣耀ID和对应的完成状态数组
	lua_getfield(L, -1, "GloryInfArr");
	int GloryInfArrSize = luaL_getn(L, -1);
	SAGloryMsg GloryMsg(GloryDot, GloryInfArrSize);
	for (int i = 1; i <= GloryInfArrSize; i++)
	{
		lua_rawgeti(L, -1, i);
		GloryMsg.m_pGloryInfArr[i - 1] = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	g_Script.m_pPlayer->SendSAGloryMsg(&GloryMsg);
	return 0;
}

//请求获得荣耀奖励
int CScriptManager::L_QstGetGloryAward(lua_State *L)
{
	SAGetGloryMsg GetGloryMsg;
	if (!lua_istable(L, -1))		//领取奖励失败
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "领取奖励失败");
		return 1;
	}
	WORD GloryID = lua_tonumber(L, -2);
	const SGloryBaseData *lpGloryBaseData = CGloryAndTitleServer::GetInstance().GetGloryBaseDataByID(GloryID);	//获取该荣耀ID的基本数据
	if (lpGloryBaseData == NULL)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "没有要领取的荣耀信息");
		return 1;
	}

	//判断条件是否满足，条件比较多，占时不处理
	if (!g_Script.m_pPlayer->JudgeCanGetGloryAward(lpGloryBaseData))	//条件不满足领取
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "条件不满足领取");
		return 1;
	}
	//计算奖励
	g_Script.m_pPlayer->ChangeMoney(2, lpGloryBaseData->m_AwardFactor[GloryAddType::GLORY_AYINLIANG]);
	g_Script.m_pPlayer->ChangeMoney(1, lpGloryBaseData->m_AwardFactor[GloryAddType::GLORY_AJINBI]);
	lua_getfield(L, -1, "PlayerGloryDot");
	DWORD OldGloryDot = lua_tonumber(L, -1);
	lua_pop(L, 1);
	DWORD NewGloryDot = OldGloryDot + lpGloryBaseData->m_AwardFactor[GloryAddType::GLORY_AGLORYDOT];
	lua_pushnumber(L, NewGloryDot);
	lua_setfield(L, -2, "PlayerGloryDot");	//更新奖励点

	//更新对应荣耀ID的完成状态
	lua_getfield(L, -1, "GloryInfArr");
	int TableSize = luaL_getn(L, -1);
	for (int i = 1; i <= TableSize; i++)
	{
		lua_rawgeti(L, -1, i);
		WORD OldTempValue = lua_tonumber(L, -1);
		lua_pop(L, 1);
		if (OldTempValue == GloryID)	//更新荣耀数据
		{
			WORD SetValue = (0x1 << 15) + GloryID;
			lua_pushnumber(L, SetValue);
			lua_rawseti(L, -2, i);
		}
	}
	lua_pop(L, 1);	//弹出荣耀完成状态表
	//返回成功消息
	GetGloryMsg.m_Ressult = 1;
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &GetGloryMsg, sizeof(SAGetGloryMsg));
	return 1;	//返回更新的荣耀数据
}

//请求处理称号消息

void SendTitleSuccessMsg(SATitleMsg *lpAnsTitleMsg)
{
	
	int SendDataSize = lpAnsTitleMsg->m_TitleSize * sizeof(BYTE)+sizeof(SATitleMsg)-sizeof(void*);
	BYTE *Buf = new BYTE[SendDataSize];
	memset(Buf, 0, SendDataSize);
	memcpy(Buf, lpAnsTitleMsg, sizeof(SATitleMsg)-sizeof(void*));
	memcpy(Buf + sizeof(SATitleMsg)-sizeof(void*), lpAnsTitleMsg->m_TitleArr, lpAnsTitleMsg->m_TitleSize * sizeof(BYTE));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, Buf, SendDataSize);
}

int CScriptManager::L_QstHandleTitleMsg(lua_State *L)
{
	SATitleMsg AnsTitleMsg;
	if (!lua_istable(L, -1))
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "操作失败");
		return 1;
	}
	BYTE MsgType = lua_tonumber(L, -3);
	BYTE TitleID = lua_tonumber(L, -2);
	const STitleBaseData *lpTitleBaseData = CGloryAndTitleServer::GetInstance().GetTitleBaseDataByID(TitleID);
	if (lpTitleBaseData == NULL)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "没有该称号ID");
		return 1;
	}

	lua_getfield(L, -1, "PlayerGloryDot");
	int CurPlayerGloryDot = lua_tonumber(L, -1);
	switch (MsgType)
	{
	case SQTitleMsg::EXCHANGE:		//兑换称号操作
	{
										lua_pop(L, 1);
										SPackageItem *lpCurTitleItem = g_Script.m_pPlayer->FindItemByItemID(lpTitleBaseData->m_ConsumeItemID);
										int ItemNum = g_Script.m_pPlayer->GetItemNum(lpTitleBaseData->m_ConsumeItemID, XYD_FT_ONLYUNLOCK);
										if (lpCurTitleItem == NULL && lpTitleBaseData->m_ConsumeFactor[TitleConsumeType::TITLE_CGLORYITEMNUM] != 0)		//背包中道具数量为0
										{
											TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "背包中没有需要的荣耀道具！");
											return 1;
										}
										//判断荣耀点和荣耀道具数量是否足够
										if (CurPlayerGloryDot < lpTitleBaseData->m_ConsumeFactor[TitleConsumeType::TITLE_CGLORYDOT] ||
											ItemNum < lpTitleBaseData->m_ConsumeFactor[TitleConsumeType::TITLE_CGLORYITEMNUM])
										{
											TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "荣耀点或荣耀道具数量不够");
											return 1;
										}

										//消耗荣耀点
										CurPlayerGloryDot -= lpTitleBaseData->m_ConsumeFactor[TitleConsumeType::TITLE_CGLORYDOT];
										//更新荣耀点
										lua_pushnumber(L, CurPlayerGloryDot);
										lua_setfield(L, -2, "PlayerGloryDot");

										////消耗荣耀道具数量计算
										//DWORD GloryItemID = lpCurTitleItem->wIndex;
										//WORD GloryItemCount = lpCurTitleItem->overlap;
										//GloryItemCount -= lpTitleBaseData->m_ConsumeFactor[TitleConsumeType::TITLE_CGLORYITEMNUM];
										////先删除该道具
										//g_Script.m_pPlayer->DelItem(*lpCurTitleItem);
										////更新背包中的荣耀道具
										//if (GloryItemCount > 0)
										//	g_Script.m_pPlayer->RecvAddItem(GloryItemID, GloryItemCount);
										//消耗荣耀道具
										g_Script.m_pPlayer->DeleteItem(lpTitleBaseData->m_ConsumeItemID, lpTitleBaseData->m_ConsumeFactor[TitleConsumeType::TITLE_CGLORYITEMNUM]);

										int TitleSize = CONST_USERNAME * MAX_TITLE;
										//更新玩家所拥有的称号
										int i = 0;
										for (; i < TitleSize; i++)		//从第1个位置开始保存所拥有的的称号。每个字节的第一位为1，表示玩家当前使用中的称号；为0表示玩家没有使用称号
										{
											if (((BYTE)((*(g_Script.m_pPlayer->m_Property.m_Title[i / CONST_USERNAME] + i % CONST_USERNAME)) << 1) >> 1) == TitleID)
											{
												TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "当前已经拥有了该称号");
												return 1;
											}
											if ((BYTE)(*(g_Script.m_pPlayer->m_Property.m_Title[i / CONST_USERNAME] + i % CONST_USERNAME)) == 0)		//增加玩家所拥有的称号
											{
												*(g_Script.m_pPlayer->m_Property.m_Title[i / CONST_USERNAME] + i % CONST_USERNAME) = TitleID;
												break;
											}
										}

										//兑换成功
										AnsTitleMsg.m_Ressult = 1;
										AnsTitleMsg.m_TitleSize = i + 1; 
										AnsTitleMsg.m_GloryDot = CurPlayerGloryDot;
										AnsTitleMsg.m_TitleArr = new BYTE[AnsTitleMsg.m_TitleSize];
										memcpy(AnsTitleMsg.m_TitleArr, g_Script.m_pPlayer->m_Property.m_Title, AnsTitleMsg.m_TitleSize);
										SendTitleSuccessMsg(&AnsTitleMsg);
										return 1;
	}
		break;
	case SQTitleMsg::USE:		//使用称号操作
	{
									BYTE byState = 0;	//元素的状态
									BYTE byTitleID = 0;	//元素的ID
									int iCurTitleIDIndex = -1, iUsedTitleID = -1;	//iCurTitleIDIndex表示当前要使用的ID；iUsedTitleID表示已经在使用的ID，-1表示当前没有使用中的称号ID
									int TitleSize = CONST_USERNAME * MAX_TITLE;
									for (int i = 0; i < TitleSize; i++)		//从第1个位置查找称号。
									{
										byState = (BYTE)(*(g_Script.m_pPlayer->m_Property.m_Title[i / CONST_USERNAME] + i % CONST_USERNAME)) >> 7;	//获取元素的状态
										BYTE byTempTitleID = ((BYTE)((*(g_Script.m_pPlayer->m_Property.m_Title[i / CONST_USERNAME] + i % CONST_USERNAME)) << 1)) >> 1;
										if (byState == 1)
										{
											byTitleID = byTempTitleID;
											iUsedTitleID = i;
										}
										if (byTempTitleID == TitleID)
										{
											iCurTitleIDIndex = i;
										}
									}
									if (iCurTitleIDIndex == -1)
									{
										TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "玩家没有获得该称号");
										return 1;
									}

									//把使用中的称号变为未使用中，把TitleID变为使用中的称号
									if (iUsedTitleID != -1)
										*(g_Script.m_pPlayer->m_Property.m_Title[iUsedTitleID / CONST_USERNAME] + iUsedTitleID % CONST_USERNAME) = byTitleID;	//使已经使用中的称号更改为未使用
									*(g_Script.m_pPlayer->m_Property.m_Title[iCurTitleIDIndex / CONST_USERNAME] + iCurTitleIDIndex % CONST_USERNAME) = TitleID + 0x80; //0x80 等于 1000 0000

									//增加玩家附加属性
									g_Script.m_pPlayer->InitTitleData();
									g_Script.m_pPlayer->UpdateAllProperties();

									int  CurTitleSize = 0;
									for (; CurTitleSize < TitleSize; CurTitleSize++)
									if ((BYTE)(*(g_Script.m_pPlayer->m_Property.m_Title[CurTitleSize / CONST_USERNAME] + CurTitleSize % CONST_USERNAME)) == 0)
										break;
									//使用称号成功
									AnsTitleMsg.m_Ressult = 1;
									AnsTitleMsg.m_TitleSize = CurTitleSize;
									AnsTitleMsg.m_GloryDot = CurPlayerGloryDot;
									AnsTitleMsg.m_TitleArr = new BYTE[AnsTitleMsg.m_TitleSize];
									memcpy(AnsTitleMsg.m_TitleArr, g_Script.m_pPlayer->m_Property.m_Title, AnsTitleMsg.m_TitleSize);
									SendTitleSuccessMsg(&AnsTitleMsg);
									g_Script.m_pPlayer->SendMyState();
									return 1;
	}
		break;
	case SQTitleMsg::UNUSE:		//取消使用称号操作
	{
									BYTE byState = 0;	//元素的状态
									BYTE byTitleID = 0;	//元素的ID
									int iUsedTitleID = -1;	//iUsedTitleID表示已经在使用的ID，-1表示当前没有使用中的称号ID
									int TitleSize = CONST_USERNAME * MAX_TITLE;
									for (int i = 0; i < TitleSize; i++)		//从第1个位置查找称号。
									{
										byState = (BYTE)(*(g_Script.m_pPlayer->m_Property.m_Title[i / CONST_USERNAME] + i % CONST_USERNAME)) >> 7;	//获取元素的状态
										if (byState == 1)
										{
											byTitleID = ((BYTE)((*(g_Script.m_pPlayer->m_Property.m_Title[i / CONST_USERNAME] + i % CONST_USERNAME)) << 1)) >> 1;
											iUsedTitleID = i;
											break;
										}
									}
									if (iUsedTitleID == -1)
									{
										TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "玩家该称号没有在使用中");
										return 1;
									}
									//取消使用称号，把称号数组中的第一元素的最高位设置为0
									*(g_Script.m_pPlayer->m_Property.m_Title[iUsedTitleID / CONST_USERNAME] + iUsedTitleID % CONST_USERNAME) = byTitleID;

									//修改玩家附加属性
									g_Script.m_pPlayer->InitTitleData();
									g_Script.m_pPlayer->UpdateAllProperties();

									int TitleAllSize = CONST_USERNAME * MAX_TITLE;
									int  CurTitleSize = 0;
									for (; CurTitleSize < TitleAllSize; CurTitleSize++)
									if ((BYTE)(*(g_Script.m_pPlayer->m_Property.m_Title[CurTitleSize / CONST_USERNAME] + CurTitleSize % CONST_USERNAME)) == 0)
										break;
									//取消使用称号成功
									AnsTitleMsg.m_Ressult = 1;
									AnsTitleMsg.m_TitleSize = CurTitleSize;
									AnsTitleMsg.m_GloryDot = CurPlayerGloryDot;
									AnsTitleMsg.m_TitleArr = new BYTE[AnsTitleMsg.m_TitleSize];
									memcpy(AnsTitleMsg.m_TitleArr, g_Script.m_pPlayer->m_Property.m_Title, AnsTitleMsg.m_TitleSize);
									SendTitleSuccessMsg(&AnsTitleMsg);
									g_Script.m_pPlayer->SendMyState();
									return 1;
	}
		break;
	default:
		break;
	}
	return 1;
}

int CScriptManager::L_SetKillAllMonster(lua_State *L)		//设置玩家在关卡中杀死全部已经刷新的怪物（用于测试用）
{
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
	{
		DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
		BYTE SetValue = static_cast<BYTE>(lua_tonumber(L, 2));
		CPlayer *pPlayer = NULL;
		if (gid != 0)
			pPlayer = (CPlayer*)GetObjectByGID(gid)->DynamicCast(IID_PLAYER);
		else
			pPlayer = g_Script.m_pPlayer;
		if (SetValue != 0)
			pPlayer->m_IsUseKillMonster = 1;
		else
			pPlayer->m_IsUseKillMonster = 0;
		lua_pushnumber(L, 1);
		return 1;
	}
	return 0;
}

int CScriptManager::GetTableStringValue(lua_State *L, int nIndex, const char* key, std::string &str)
{
	lua_getfield(L, nIndex, key);
	if (lua_isstring(L, -1))
	{
		LPCSTR strvalue = static_cast<const char*>(lua_tostring(L, -1));
		str = strvalue;
		return 1;
	}
	lua_pop(L, 1);
	rfalse(2, 1, FormatString("GetTableNumber Fail key = %s", key));
	return 0;
}

int CScriptManager::GetTableStringValue(lua_State *L, int nIndex, const char* key,  char strvalue[CONST_USERNAME])
{
	lua_getfield(L, nIndex, key);
	if (lua_isstring(L, -1))
	{
		LPCSTR pstrvalue = static_cast<const char*>(lua_tostring(L, -1));
		//strcpy(*strvalue, pstrvalue);
		strcpy(strvalue, pstrvalue);
		lua_pop(L, 1);
		return 1;
	}
	lua_pop(L, 1);
	rfalse(2, 1, FormatString("GetTableNumber Fail key = %s", key));
	return 0;
}
int CScriptManager::GetTableArrayString(lua_State *L, int nIndex, int narrayIndex, char strvalue[CONST_USERNAME])
{
	lua_rawgeti(L, nIndex, narrayIndex);
	if (lua_isstring(L, -1))
	{
		LPCSTR pstrvalue = static_cast<const char*>(lua_tostring(L, -1));
		//strcpy(*strvalue, pstrvalue);
		strcpy(strvalue, pstrvalue);
		lua_pop(L, 1);
		return 1;
	}
	lua_pop(L, 1);
	rfalse(2, 1, FormatString("GetTableArrayString Fail tableIndex = %d,arrayIndex = %d", nIndex, narrayIndex));
	return 0;
}

int CScriptManager::L_TaskKillMonster(lua_State *L)
{
	if (lua_isnumber(L,1)&&lua_isnumber(L,2))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		WORD pmonsterid = static_cast<WORD>(lua_tonumber(L, 2));
		LPIObject pobject = GetPlayerBySID(sid);
		CPlayer *pPlayer = (CPlayer *)pobject->DynamicCast(IID_PLAYER);
		if (pPlayer)
		{
			CPlayer::TASKKILL::iterator it = pPlayer->m_KillTask.find(pmonsterid);
			//CPlayer::TASKKILL::iterator it = pCurrPlayer->m_KillTask.find(this->m_Property.m_KillTaskID);
			if (it != pPlayer->m_KillTask.end())
			{
				for (std::list<DWORD>::iterator ti = (it->second).begin(); ti != (it->second).end(); ++ti)
				{
					DWORD flagIndex = *ti;

					if (CRoleTask *task = pPlayer->m_TaskManager.GetRoleTask((flagIndex & 0xffff0000) >> 16))
					{
						if (STaskFlag *flag = task->GetFlag(flagIndex))
						{
							if (flag->m_Complete)			// 如果已经完成了，不做任何处理
								continue;

							BYTE oldComp = flag->m_Complete;

							if (++flag->m_TaskDetail.Kill.CurKillNum == flag->m_TaskDetail.Kill.KillNum)
								flag->m_Complete = 1;

							// 发送旗标更新消息
							pPlayer->SendUpdateTaskFlagMsg(*flag, oldComp != flag->m_Complete);
						}
						else
						{
							rfalse("检查任务杀怪信息，竟然找不到任务旗标！");
						}
					}
					else
					{
						rfalse("检查任务杀怪信息，竟然找不到任务！");
					}
				}
			}
		}
	}
	return 1;
}

int CScriptManager::L_SendBatchesMonster(lua_State *L)
{
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3)  )
	{
		DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
		CPlayer *pPlayer = (CPlayer*)GetObjectByGID(gid)->DynamicCast(IID_PLAYER);
		if (pPlayer)
		{
			SABatchesMonster sbatch;
			sbatch.bIndex = static_cast<BYTE>(lua_tonumber(L, 2));
			sbatch.bstate = static_cast<BYTE>(lua_tonumber(L,3));
			CSceneMap::GetInstance().SendBatchesMonster(pPlayer, &sbatch, sizeof(SABatchesMonster));
		}
	}  
	return 1;
}

int CScriptManager::L_Syneffects(lua_State *L)
{
	if (lua_isnumber(L,1)&&lua_isnumber(L,2))
	{
		DWORD dwgid = static_cast<DWORD>(lua_tonumber(L,1));
		WORD deffectid = static_cast<WORD>(lua_tonumber(L, 2));
		
		return 1;
	}
	return 0;
}

int CScriptManager::L_CopyFromPlayer(lua_State *L)
{
	if (!g_Script.m_pMonster)
	{
		return 0;
	}
	if (lua_istable(L,1))
	{
		GetTableNumber(L, 1, "MaxHp", g_Script.m_pMonster->m_MaxHp);
		g_Script.m_pMonster->m_CurHp = g_Script.m_pMonster->m_MaxHp;
		GetTableNumber(L, 1, "GongJi", g_Script.m_pMonster->m_GongJi);
		GetTableNumber(L, 1, "FangYu", g_Script.m_pMonster->m_FangYu);
		GetTableNumber(L, 1, "BaoJi", g_Script.m_pMonster->m_BaoJi);
		GetTableNumber(L, 1, "ShanBi", g_Script.m_pMonster->m_ShanBi);
		GetTableNumber(L, 1, "Uncrit", g_Script.m_pMonster->m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT]);
		GetTableNumber(L, 1, "Wreck", g_Script.m_pMonster->m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT]);
		GetTableNumber(L, 1, "Unwreck", g_Script.m_pMonster->m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT]);
		GetTableNumber(L, 1, "Puncture", g_Script.m_pMonster->m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT]);
		GetTableNumber(L, 1, "Unpuncture", g_Script.m_pMonster->m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT]);
		GetTableNumber(L, 1, "Hit", g_Script.m_pMonster->m_Hit);
		GetTableNumber(L, 1, "ShowState", g_Script.m_pMonster->m_ShowState);
		GetTableNumber(L, 1, "Level", g_Script.m_pMonster->m_Level);
		GetTableNumber(L, 1, "ModId", g_Script.m_pMonster->m_dModID);
		GetTableStringValue(L, 1, "MyName", g_Script.m_pMonster->m_Name);
		lua_settop(L, 2);
		lua_pushnumber(L, 1);
		return 1;
	}
	return 0;
}

int CScriptManager::L_CopyPlayerToTable(lua_State *L)
{
	CPlayer *pPlayer = 0;
	if (!pPlayer)
	{
		if (lua_isnumber(L,1)&&lua_istable(L,2))
		{
			DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
			pPlayer = (CPlayer*)GetObjectByGID(gid)->DynamicCast(IID_PLAYER);

			if (!pPlayer)
			{
				return 0;
			}

			if (lua_istable(L,2))
			{
				SetTableNumber(L, "PlayerSid", pPlayer->GetSID());
				SetTableNumber(L, "MaxHp", pPlayer->m_MaxHp);
				SetTableNumber(L, "GongJi", pPlayer->m_GongJi);
				SetTableNumber(L, "FangYu", pPlayer->m_FangYu);
				SetTableNumber(L, "BaoJi", pPlayer->m_BaoJi);
				SetTableNumber(L, "ShanBi", pPlayer->m_ShanBi);
				SetTableNumber(L, "Uncrit", pPlayer->m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT]);
				SetTableNumber(L, "Wreck", pPlayer->m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT]);
				SetTableNumber(L, "Unwreck", pPlayer->m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT]);
				SetTableNumber(L, "Puncture", pPlayer->m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT]);
				SetTableNumber(L, "Unpuncture", pPlayer->m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT]);
				SetTableNumber(L, "Hit", pPlayer->m_Hit);
				SetTableNumber(L, "ShowState", pPlayer->m_Property.m_ShowState);
				SetTableNumber(L, "Level", pPlayer->m_Level);
				SetTableNumber(L, "ModID", pPlayer->m_Property.m_School); //使用门派来区别模型ID
				SetTableStringValue(L, "MyName", pPlayer->m_Property.m_Name);
// 				int ntop = lua_gettop(L);
// 				lua_settop(L, ntop+1);
				lua_pushnumber(L, 1);
				return 1;
			}

		}
		return 0;
	}
	//lua_createtable(L, 0, 0);

	return 0;
}

int CScriptManager::SetTableStringValue(lua_State *L, const char *key, char strvalue[CONST_USERNAME])
{
	if (!lua_istable(L, -1))
	{
		return 0;
	}
	lua_pushstring(L, key);
	lua_pushstring(L, strvalue);
	lua_settable(L, -3);
	return 1;
}

int CScriptManager::L_GetObjectReginType(lua_State *L)
{
	if (lua_isnumber(L,1))
	{
		DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
		CFightObject *pfight = (CFightObject*)GetObjectByGID(gid)->DynamicCast(IID_FIGHTOBJECT);
		if (pfight)
		{
			CDynamicScene *pscene = (CDynamicScene*)pfight->m_ParentRegion->DynamicCast(IID_DYNAMICSCENE);
			if (pscene)
			{
				lua_pushnumber(L, 3);
				return 1;
			}

			CDynamicArena *parena = (CDynamicArena*)pfight->m_ParentRegion->DynamicCast(IID_DYNAMICARENA);
			if (parena)
			{
				lua_pushnumber(L, 4);
				return 1;
			}

			CDynamicDailyArea *pDailyArea = (CDynamicDailyArea*)pfight->m_ParentRegion->DynamicCast(IID_DYNAMICDAILYAREA);
			if (pDailyArea)
			{
				lua_pushnumber(L, 5);
				return 1;
			}

			CDynamicRegion *pdyregin = (CDynamicRegion*)pfight->m_ParentRegion->DynamicCast(IID_DYNAMICREGION);
			if (pdyregin)
			{
				lua_pushnumber(L, 2);
				return 1;
			}
			lua_pushnumber(L, 1);
			return 1;
		}
	}
	return 1;
}

int CScriptManager::L_GetSkillDamageRate(lua_State *L)
{
	if (lua_isnumber(L,1)&&lua_isnumber(L,2))
	{
		WORD skillindex = static_cast<WORD>(lua_tonumber(L, 1));
		WORD skilllevel = static_cast<WORD>(lua_tonumber(L, 2));
		const SSkillBaseData *pskilldata = CSkillService::GetInstance().GetSkillBaseData(skillindex, skilllevel);
		if (pskilldata)
		{
			lua_pushnumber(L, pskilldata->m_DamageRate);
			return 1;
		}
	}
	return 0;
}


//add by ly 2014/4/10  服务器GM相关操作
int CScriptManager::L_SetPlayerGMLevel(lua_State *L)	//设置玩家GM等级，只有等级达到才有控制权限
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
		if (gid != 0)
			pPlayer = (CPlayer*)GetObjectByGID(gid)->DynamicCast(IID_PLAYER);
		else
			pPlayer = g_Script.m_pPlayer;
	}
	if (lua_isnumber(L, 2))
	{
		WORD SetValue = static_cast<WORD>(lua_tonumber(L, 2));
		pPlayer->m_Property.m_GMLevel = SetValue;
		lua_pushnumber(L, 1);
		return 1;
	}
	return 0;
}
int CScriptManager::L_BaseRelive(lua_State *L)	//原地复活,里面对L_PlayerRelive实现了封装
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
		if (gid != 0)
			pPlayer = (CPlayer*)GetObjectByGID(gid)->DynamicCast(IID_PLAYER);
		else
			pPlayer = g_Script.m_pPlayer;
	}
	if (pPlayer && lua_isnumber(L, 2))
	{
		BYTE TypeValue = static_cast<BYTE>(lua_tonumber(L, 2));
		pPlayer->QuestToRelive(TypeValue);
		lua_pushnumber(L, 1);
		return 1;
	}
	return 0;
}
int CScriptManager::L_CreateMonsterRegion(lua_State *L)		//地图刷怪 玩家GID mapid x y monsterid num
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
		if (gid != 0)
			pPlayer = (CPlayer*)GetObjectByGID(gid)->DynamicCast(IID_PLAYER);
		else
			pPlayer = g_Script.m_pPlayer;
	}

	DWORD reginid = static_cast<DWORD>(lua_tonumber(L, 2));
	if (pPlayer)
	{
		CRegion *regin = NULL;
		if (reginid == 0)
		{
			reginid = pPlayer->GetPlayerReginID();
		}
		regin = (CRegion*)FindRegionByID(reginid)->DynamicCast(IID_REGION);

		if (regin)
		{
			CMonster::SParameter param;
			if (reginid == 0)
			{
				param.wX = pPlayer->m_curTileX;
				param.wY = pPlayer->m_curTileY;
			}
			else
			{
				param.wX = static_cast<WORD>(lua_tonumber(L, 3));
				param.wY = static_cast<DWORD>(lua_tonumber(L, 4));
			}
			param.dwRefreshTime = -1;
			param.wListID = static_cast<WORD>(lua_tonumber(L, 5));
			DWORD montern = static_cast<DWORD>(lua_tonumber(L, 6));
			while (montern--){
				regin->CreateMonster(&param);
			}
			lua_pushnumber(L, 1);
			return 1;
		}
	}

	return 0;
}


extern size_t GetPlayerNumber();
int CScriptManager::L_GetOnlinePlayerNum(lua_State *L)	//获取当前服务器人数
{
	size_t num = GetPlayerNumber();
	char playernum[100];
	sprintf(playernum, "当前服务器人数%d人", num);
	lua_pushstring(L, playernum);
	return 1;
}


int CScriptManager::L_SendSysCall(lua_State *L)		//发送系统公告
{
	if (!lua_isnumber(L, 1) || !lua_isstring(L, 2) || !lua_isnumber(L, 3))
		return 0;
	int ShowNum = lua_tonumber(L, 1);
	const char* pWillSendMsg = lua_tostring(L, 2);
	BYTE Type = static_cast<BYTE>(lua_tonumber(L, 3));
	SChatToAllMsg ChatToAllMsg;
	ChatToAllMsg.wSoundId = 0;
	ChatToAllMsg.wShowNum = ShowNum;
	memset(ChatToAllMsg.cChat, 0, sizeof(ChatToAllMsg.cChat));
	if (Type == 0)	//字符串的编码格式为utf8
	{
		//char* AnsiChat = g_Utf8ToAnsi(pWillSendMsg);

		ChatToAllMsg.wChatLen = strlen(pWillSendMsg);
		memcpy_s(ChatToAllMsg.cChat, sizeof(ChatToAllMsg.cChat), (LPCSTR)pWillSendMsg, ChatToAllMsg.wChatLen);
		//if (AnsiChat != NULL)
		//	delete[] AnsiChat;

	}
	else if (Type == 1)	//字符串的编码格式为ansi
	{
		char* Utf8Str = g_AnsiToUtf8(pWillSendMsg);
		ChatToAllMsg.wChatLen = strlen(Utf8Str);
		memset(ChatToAllMsg.cChat, 0, sizeof(ChatToAllMsg.cChat));
		memcpy_s(ChatToAllMsg.cChat, sizeof(ChatToAllMsg.cChat), (LPCSTR)Utf8Str, ChatToAllMsg.wChatLen);
		if (Utf8Str != NULL)
			delete[] Utf8Str;
	}
	BroadcastMsg(&ChatToAllMsg, sizeof(SChatToAllMsg));
	lua_pushnumber(L, 1);
	return 1;
}


int CScriptManager::L_SendGmExceResult(lua_State *L)	//发送GM命令执行结果
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1) || !lua_isstring(L, 2) || !lua_isstring(L, 3))
		return 0;
	const char* AnsiName = lua_tostring(L, 2);
	const char* AnsiSendMsg = lua_tostring(L, 3);
	//char* UTF8Name = g_Utf8ToAnsi(AnsiName);
	//char* UTF8SendMsg = g_Utf8ToAnsi(AnsiSendMsg);


	SAChatGlobalMsg msg;
	msg.byType = static_cast<BYTE>(lua_tonumber(L, 1));
	int k = strlen(AnsiName);
	strcpy(msg.cTalkerName, AnsiName);
	memset(msg.cChatData, 0, sizeof(msg.cChatData));
	memcpy_s(msg.cChatData, sizeof(msg.cChatData), (LPCSTR)AnsiSendMsg, strlen(AnsiSendMsg));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, msg.GetMySize());
	//if (UTF8Name != NULL)
	//	delete[] UTF8Name;
	//if (UTF8SendMsg != NULL)
	//	delete[] UTF8SendMsg;
	return 1;
}

extern void TalkToAllObj(LPCSTR info);
int CScriptManager::L_SendMessageToAllPlayer(lua_State *L)	//发送消息给所有玩家
{
	if (lua_isstring(L, 1))
	{
		TalkToAllObj(lua_tostring(L, 1));
		lua_pushnumber(L, 1);
		return 1;
	}
	return 0;
}

int CScriptManager::L_PutPlayerToDyncRegion(lua_State *L)	//创建动态地图并且将玩家放入
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
		if (gid != 0)
			pPlayer = (CPlayer*)GetObjectByGID(gid)->DynamicCast(IID_PLAYER);
		else
			pPlayer = g_Script.m_pPlayer;
	}
	if (pPlayer)
	{
		int regionid = lua_tonumber(L, 2);
		int x = lua_tonumber(L, 3);
		int y = lua_tonumber(L, 4);

		g_Script.CallFunc("PutPlayerToDynamicRegion", regionid, x, y, pPlayer->GetGID());
	}
	return 0;
}

int CScriptManager::L_SendArenaInitMsg(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	int nindex = lua_gettop(L);
	SASArenaInit  sarenainit;
	if (lua_istable(L,1))
	{
		GetTableNumber(L, 1, "wRank", sarenainit.wRank);
		GetTableNumber(L, 1, "wRankPoint", sarenainit.wRankPoint);
		GetTableNumber(L, 1, "wConWin", sarenainit.wConWin);
		GetTableNumber(L, 1, "dcoldtime", sarenainit.dcoldtime);
		GetTableNumber(L, 1, "bchallengecount", sarenainit.bchallengecount);
	}

	//size_t challneger = lua_gettop(L);
	int param = -2;
	int i = 0;
	if (lua_istable(L,2))
	{
		int nindex = lua_gettop(L);
		size_t nlenth = luaL_getn(L, 2);
		for (int i = 0; i < nlenth; i++)
		{
			lua_rawgeti(L, 2, i + 1);
			if (lua_istable(L, -1))
			{
				if (GetTableNumber(L, -1, "wRank", sarenainit.pchallenge[i].wRank) )
				{
					lua_pop(L, 1);
				}
				if (GetTableNumber(L, -1, "wModID", sarenainit.pchallenge[i].wModID))
				{
					lua_pop(L, 1);
				}
				if (GetTableNumber(L, -1, "dfightpower", sarenainit.pchallenge[i].dfightpower))
				{
					lua_pop(L, 1);
				}
				if (GetTableStringValue(L, -1, "playername", sarenainit.pchallenge[i].playername))
				{
					lua_pop(L, 1);
				}
			}
		}

	}


	CArenaMap::GetInstance().SendArenaMsg(g_Script.m_pPlayer, &sarenainit, sizeof(sarenainit));
	return 1;
}



int CScriptManager::L_SetStopTracing(lua_State *L)
{
	if (lua_isnumber(L,1))
	{
		DWORD gid =static_cast<DWORD>(lua_tonumber(L, 1));
		CMonster *pMonster = (CMonster*)GetObjectByGID(gid)->DynamicCast(IID_MONSTER);
		if (pMonster)
		{
			pMonster->SetStopTracing(EA_RUN);
			return 1;
		}
	}
	return 0;
}

//add by ly 2014/4/16
int CScriptManager::L_ResetDailyByTime(lua_State *L)	//玩家重置已经完成的日常活动列表信息（参数依次为：玩家GID，日常重置时间；已经完成的日常数据（重置时间满足后重置该表）；）
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_istable(L, 3))
		return 0;
	CPlayer *pPlayer = NULL;
	int PlayerGid = lua_tonumber(L, 1);
	if (PlayerGid == 0)
		pPlayer = g_Script.m_pPlayer;
	else
		pPlayer = (CPlayer*)GetPlayerByGID(PlayerGid)->DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return 1;
	time_t ResetTime = static_cast<time_t>(lua_tonumber(L, 2));		//每天什么时候重置日常活动信息（24 * 60 * 60）

	lua_getfield(L, 3, "PlayerPreQuestTime");
	time_t PreReqDailyTime = static_cast<time_t>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	time_t dwCurTime;
	time(&dwCurTime);
	time_t CurT = dwCurTime - ResetTime;	//标准到24点重置
	if (CurT < 0)
		CurT = 0;
	tm * pCurT = localtime(&CurT);
	tm CurTime, PreTime;
	memcpy(&CurTime, pCurT, sizeof(tm));
	time_t PreT = PreReqDailyTime - ResetTime;	//标准到24点重置
	if (PreT < 0)
		PreT = 0;
	tm *pPreT = localtime(&PreT);
	memcpy(&PreTime, pPreT, sizeof(tm));
	if (CurTime.tm_year > PreTime.tm_year || CurTime.tm_yday > PreTime.tm_yday) //满足重置时间，重置玩家日常活动列表信息
	{
		for (int i = 1; i <= luaL_getn(L, 3); i++)
		{
			lua_rawgeti(L, 3, i);

			lua_pushnumber(L, 0);
			lua_setfield(L, -2, "FinishCount");

			lua_pushnumber(L, 0);
			lua_setfield(L, -2, "FinishState"); 

			lua_pushnumber(L, 0);
			lua_setfield(L, -2, "SpendTimes");
			lua_pop(L, 1);
		}
	}
	//返回完成日常信息，主要由于有定时重置功能
	return 1;
}

int CScriptManager::L_ReturnDailyListInfo(lua_State *L)	//获取玩家日常活动列表信息（参数依次为：玩家GID，已经完成的日常数据；日常列表信息）
{
	if (!lua_isnumber(L, 1) || !lua_istable(L, 2) /*|| !lua_istable(L, 3)*/)
		return 0;
	CPlayer *pPlayer = NULL;
	int PlayerGid = lua_tonumber(L, 1);
	if (PlayerGid == 0)
		pPlayer = g_Script.m_pPlayer;
	else
		pPlayer = (CPlayer*)GetPlayerByGID(PlayerGid)->DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return 0;
	SAInitDailyMsg RetInitDailyMsg;
	RetInitDailyMsg.m_DailyArrSize = luaL_getn(L, 2);
	RetInitDailyMsg.m_pDailyDetail = new unsigned short[RetInitDailyMsg.m_DailyArrSize];
	RetInitDailyMsg.m_pDailyTotalTimes = new BYTE[RetInitDailyMsg.m_DailyArrSize];
	for (int i = 0; i < RetInitDailyMsg.m_DailyArrSize; i++)
	{
		lua_rawgeti(L, 2, i + 1);

		BYTE State = 0, ResideCount = 0, DailyID = 0;
		lua_getfield(L, -1, "DailyID");
		BYTE iDailyID = lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, -1, "LimitTimes");
		BYTE CanUseFreeCount = lua_tonumber(L, -1);
		lua_pop(L, 1);

		ResideCount = CanUseFreeCount;
		DailyID = iDailyID;

		lua_getfield(L, -1, "FinishCount");
		BYTE FinishCount = lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, -1, "FinishState");
		BYTE FinishState = lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, -1, "SpendTimes");	//金币购买的次数
		BYTE GlodSpendTimes = lua_tonumber(L, -1);
		lua_pop(L, 1);
		ResideCount += GlodSpendTimes;
		ResideCount -= FinishCount;
		State = FinishState;

		////更加玩家完成日常情况，更新日常列表信息
		//for (int j = 0; j < RetInitDailyMsg.m_DailyArrSize; j++)
		//{
		//	lua_rawgeti(L, 2, j + 1);

		//	lua_getfield(L, -1, "DailyID");
		//	BYTE jDailyID = lua_tonumber(L, -1);
		//	lua_pop(L, 1);

		//	lua_getfield(L, -1, "FinishCount");
		//	BYTE FinishCount = lua_tonumber(L, -1);
		//	lua_pop(L, 1);		

		//	lua_getfield(L, -1, "FinishState");
		//	BYTE FinishState = lua_tonumber(L, -1);
		//	lua_pop(L, 1);

		//	lua_getfield(L, -1, "SpendTimes");	//金币购买的次数
		//	BYTE GlodSpendTimes = lua_tonumber(L, -1);
		//	lua_pop(L, 1);

		//	if (iDailyID == jDailyID)
		//	{
		//		ResideCount += GlodSpendTimes;
		//		ResideCount -= FinishCount;
		//		State = FinishState;
		//		break;
		//	}
		//	lua_pop(L, 1);
		//}
		RetInitDailyMsg.m_pDailyTotalTimes[i] = CanUseFreeCount;
		RetInitDailyMsg.m_pDailyDetail[i] = (unsigned short)(State << 15) + (unsigned short)(ResideCount << 8) + DailyID;
		lua_pop(L, 1);
	}
	pPlayer->SendDailyMsgToPlayer(&RetInitDailyMsg);
	delete[] RetInitDailyMsg.m_pDailyDetail;
	RetInitDailyMsg.m_pDailyDetail = NULL;
	//返回成功
	lua_pushnumber(L, 1);
	return 1;
}


int CScriptManager::L_DestroyDailyArea(lua_State *L)	//销毁日常活动场景
{
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	DWORD dwRegionGID = static_cast<DWORD>(lua_tonumber(L, 1));
	CDynamicDailyArea* pDailyArea = (CDynamicDailyArea *)FindRegionByGID(dwRegionGID)->DynamicCast(IID_DYNAMICDAILYAREA);
	if (pDailyArea)
	{
		pDailyArea->isValid() = false;
		lua_pushnumber(L, 1);
		return 1;
	}
	return 0;
}

int CScriptManager::L_ResponseEntryDailyArea(lua_State *L)	//进入日常活动后的响应操作
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
	{
		return 0;
	}	
	CPlayer *pPlayer = NULL;
	int PlayerGid = lua_tonumber(L, 1);
	if (PlayerGid == 0)
		pPlayer = g_Script.m_pPlayer;
	else
		pPlayer = (CPlayer*)GetPlayerByGID(PlayerGid)->DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return 0;
	BYTE OptRet = static_cast<BYTE>(lua_tonumber(L, 2));
	WORD NextSpendGlod = static_cast<WORD>(lua_tonumber(L, 3));
	SAEntryDailyMsg ResponseEntry;
	ResponseEntry.m_Result = OptRet;
	ResponseEntry.m_NextSpendGlord = NextSpendGlod;
	g_StoreMessage(pPlayer->m_ClientIndex, &ResponseEntry, sizeof(SAEntryDailyMsg));
	return 0;
} 

int CScriptManager::L_SendDailyAwardToPlayer(lua_State *L)	//发送日常活动的奖励给玩家
{
	CPlayer *pPlayer = g_Script.m_pPlayer;
	if (lua_isnumber(L, 2))
	{
		int PlayerGid = lua_tonumber(L, 2);
		if (PlayerGid != 0)
			pPlayer = (CPlayer*)GetPlayerByGID(PlayerGid)->DynamicCast(IID_PLAYER);
	}
	if (pPlayer == NULL)
		return 0;
	if (!lua_istable(L, 1))
		return 0;
	int TableSize = luaL_getn(L, 1);
	SADailyAwardMsg DailyAwardData;
	for (int i = 0; i < TableSize; i++)
	{
		lua_rawgeti(L, 1, i + 1);
		DailyAwardData.m_AwardValue[i] = static_cast<DWORD>(lua_tonumber(L, -1));
	}
	g_StoreMessage(pPlayer->m_ClientIndex, &DailyAwardData, sizeof(SADailyAwardMsg));

	return 0;
}


int CScriptManager::L_GetPlayerInDailyArea(lua_State *L)	//获取玩家是否在日常活动场景中
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	CDynamicDailyArea *pscene = (CDynamicDailyArea *)g_Script.m_pPlayer->m_ParentRegion->DynamicCast(IID_DYNAMICDAILYAREA);
	if (pscene)
	{
		lua_pushnumber(L, 1);
		return 1;
	}
	lua_pushnumber(L, 0);
	return 0;
}

int CScriptManager::L_SendNextBatchMonsterInf(lua_State *L)	//通知玩家当前波怪物死亡，下一波为第几波怪和下一波怪还有多久刷新
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
	{
		return 0;
	}
	CPlayer *pPlayer = NULL;
	int PlayerGid = lua_tonumber(L, 1);
	if (PlayerGid == 0)
		pPlayer = g_Script.m_pPlayer;
	else
		pPlayer = (CPlayer*)GetPlayerByGID(PlayerGid)->DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return 0;
	int m, n;
	m = lua_tonumber(L, 2);
	n = lua_tonumber(L, 3);
	SANextMonsterRefreshMsg NextRefreshMonster;
	NextRefreshMonster.m_CurBatchMonster = lua_tonumber(L, 2);
	NextRefreshMonster.m_NextBatchRefreshTime = lua_tonumber(L, 3);
	NextRefreshMonster.m_CurDailyID = lua_tonumber(L, 4);
	g_StoreMessage(pPlayer->m_ClientIndex, &NextRefreshMonster, sizeof(SANextMonsterRefreshMsg));
	return 0;
}


int CScriptManager::L_SendDailyOpenFlag(lua_State *L)	//发送活动开放标志
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
	{
		return 0;
	}
	BYTE OpenFlag = static_cast<BYTE>(lua_tonumber(L, 1));
	BYTE DailyType = static_cast<BYTE>(lua_tonumber(L, 2));
	SADailyOpenFlagMsg Msg;
	Msg.m_Flag = OpenFlag;
	Msg.m_DailyType = DailyType;
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SADailyOpenFlagMsg));
	return 1;
}

int CScriptManager::L_BroadCastDailyOpenFlag(lua_State *L)	//广播活动开放标志
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
	{
		return 0;
	}
	BYTE OpenFlag = static_cast<BYTE>(lua_tonumber(L, 1));
	BYTE DailyType = static_cast<BYTE>(lua_tonumber(L, 2));
	SADailyOpenFlagMsg Msg;
	Msg.m_Flag = OpenFlag;
	Msg.m_DailyType = DailyType;

	BroadcastMsg(&Msg, sizeof(SADailyOpenFlagMsg));
	return 1;
}

int CScriptManager::L_SendJuLongShanAward(lua_State *L)		//发送巨龙山探险奖励信息
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isstring(L, 4))
	{
		return 0;
	}
	BYTE AwardType = static_cast<BYTE>(lua_tonumber(L, 1));
	DWORD AwardItmeID = static_cast<DWORD>(lua_tonumber(L, 2));
	DWORD AwardGoodsNum = static_cast<DWORD>(lua_tonumber(L, 3));
	const char* pPlotDialogueInf = lua_tostring(L, 4);
	SAJuLongShanRiskMsg Msg;
	Msg.m_AwardType = AwardType;
	Msg.m_AwardItemID = AwardItmeID;
	Msg.m_AwardNum = AwardGoodsNum;
	memset(Msg.m_PlotDialogueInf, 0, PLOTDIALOGUEMAXSIZE);
	strcpy(Msg.m_PlotDialogueInf, pPlotDialogueInf);
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SAJuLongShanRiskMsg));
	return 0;
}


int CScriptManager::L_SendTarotBaseData(lua_State *L)	//发送玩家塔罗牌基本数据信息
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	//第一个表为塔罗牌基本数据表，第二个表为已经翻开的塔罗牌数据表
	if (!lua_isnumber(L, 1) || !lua_istable(L, 2) || !lua_istable(L, 3))
	{
		return 0;
	}
	int TarotBaseDataSize = luaL_getn(L, 2);
	int TurnOnTarotDataSize = luaL_getn(L, 3);
	if (TarotBaseDataSize > TAROTMAXNUM || TurnOnTarotDataSize > MAXTURNONTAROTNUM)
		return 0;
	SAInitTarotDataMsg Msg;
	Msg.m_CanPlayTurnOffCard = static_cast<BYTE>(lua_tonumber(L, 1));
	memset(Msg.m_TarotBaseData, 0, sizeof(TarotBaseData)* TAROTMAXNUM);
	memset(Msg.m_TurnOnTarotData, 0, sizeof(TurnOnTarotData)* MAXTURNONTAROTNUM);
	for (int i = 1; i <= TarotBaseDataSize; i++)
	{
		lua_rawgeti(L, 2, i);
		if (!lua_istable(L, -1))
			return 0;
		lua_rawgeti(L, -1, 1);
		Msg.m_TarotBaseData[i - 1].m_TarotID = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 2);
		memset(Msg.m_TarotBaseData[i - 1].m_TarotName, 0, TAROTMAXNAMELEN);
		strcpy(Msg.m_TarotBaseData[i - 1].m_TarotName, lua_tostring(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 3);
		Msg.m_TarotBaseData[i - 1].m_TarotAwardData.m_ItemID = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 4);
		Msg.m_TarotBaseData[i - 1].m_TarotAwardData.m_ItemNUm = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pop(L, 1);
	}

	for (int i = 1; i <= TurnOnTarotDataSize; i++)
	{
		lua_rawgeti(L, 3, i);
		if (!lua_istable(L, -1))
			return 0;
		lua_rawgeti(L, -1, 1);
		Msg.m_TurnOnTarotData[i - 1].m_TarotID = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 2);
		Msg.m_TurnOnTarotData[i - 1].m_TarotPos = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pop(L, 1);
	}

	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SAInitTarotDataMsg));
	return 0;
}

int CScriptManager::L_SendAddTurnOnTarotData(lua_State *L)	//发送玩家翻开的塔罗牌数据信息
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
	{
		return 0;
	}
	SATurnOnOneTarotCardMsg Msg;
	Msg.m_InsertPos = static_cast<BYTE>(lua_tonumber(L, 1)) - 1;	//lua中的数组从1开始
	Msg.m_TurnOnTarotData.m_TarotID = static_cast<BYTE>(lua_tonumber(L, 2));
	Msg.m_TurnOnTarotData.m_TarotPos = static_cast<BYTE>(lua_tonumber(L, 3));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SATurnOnOneTarotCardMsg));
	return 0;
}

int CScriptManager::L_SendPlayerGetTarotAwardRes(lua_State *L)	//发送玩家领取塔罗牌奖励的操作结果消息
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	SAGetTarotAwardMsg Msg;
	Msg.m_Ret = static_cast<BYTE>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SAGetTarotAwardMsg));
	return 0;
}

int CScriptManager::L_SendResetTurnOnTarotData(lua_State *L)	//发送重置玩家已经翻开的塔罗牌数据信息
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	SAResetPlayerTarotMsg Msg;
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SAResetPlayerTarotMsg));
	return 0;
}

int CScriptManager::L_SendCurDayOnlineData(lua_State *L)	//发送玩家当天在线奖励活动的基本数据信息
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1) || !lua_istable(L, 2))
	{
		return 0;
	}
	SACurDayOnlineInitMsg Msg;
	Msg.m_CurDayOnlineAwardNum = static_cast<BYTE>(lua_tonumber(L, 1));
	Msg.m_lpCurDayOnlineData = new CurDayOnlineData[Msg.m_CurDayOnlineAwardNum];
	memset(Msg.m_lpCurDayOnlineData, 0, sizeof(CurDayOnlineData)* Msg.m_CurDayOnlineAwardNum);
	for (int i = 1; i <= Msg.m_CurDayOnlineAwardNum; i++)
	{
		lua_rawgeti(L, 2, i);
		if (!lua_istable(L, -1))
			return 0;
		lua_rawgeti(L, -1, 1);	//玩家当天在线级别ID
		Msg.m_lpCurDayOnlineData[i - 1].m_CurDayOnlineLevelID = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		lua_rawgeti(L, -1, 2);	//奖励状态（0标识不能领取，1标识可以领取，2标识已经领取）
		Msg.m_lpCurDayOnlineData[i - 1].m_AwardFlag = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		lua_rawgeti(L, -1, 3);	//需要在线时长时间
		Msg.m_lpCurDayOnlineData[i - 1].m_NeedTime = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		lua_rawgeti(L, -1, 4);	//奖励的道具ID
		Msg.m_lpCurDayOnlineData[i - 1].m_AwardItemID = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		lua_rawgeti(L, -1, 5);	//奖励数量（为道具（除经验、真气、信仰之力、银币之外的道具）为道具的数量，否则为实际的奖励值）
		Msg.m_lpCurDayOnlineData[i - 1].m_AwardItemNum = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pop(L, 1);
	}

	int AllDataSize = sizeof(SACurDayOnlineInitMsg)-sizeof(void*)+sizeof(CurDayOnlineData)* Msg.m_CurDayOnlineAwardNum;		//总数据大小
	int HeadDataSize = sizeof(SACurDayOnlineInitMsg)-sizeof(void*);		//数据头数据大小
	int ValidDataSize = sizeof(CurDayOnlineData)* Msg.m_CurDayOnlineAwardNum;	//实际数据大小

	BYTE *lpSendBuf = new BYTE[AllDataSize];
	memcpy(lpSendBuf, &Msg, HeadDataSize);
	memcpy(lpSendBuf + HeadDataSize, Msg.m_lpCurDayOnlineData, ValidDataSize);
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, lpSendBuf, AllDataSize);
	if (lpSendBuf != NULL)
		delete[] lpSendBuf;
	if (Msg.m_lpCurDayOnlineData != NULL)
		delete[] Msg.m_lpCurDayOnlineData;
	return 0;
}

int CScriptManager::L_SendCurDayOnlineLong(lua_State *L)	//发送玩家当天在线奖励活动的在线时长
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	SACurDayOnlineLongMsg Msg;
	Msg.m_CurDayOnlineTime = static_cast<DWORD>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SACurDayOnlineLongMsg));
	return 0;
}

int CScriptManager::L_SendCurDayOnlineGetAwardRes(lua_State *L)		//发送玩家领取奖励结果
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	SACurDayOnlineGetAwardMsg Msg;
	Msg.m_Ret = static_cast<BYTE>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SACurDayOnlineGetAwardMsg));
	return 0;
}

int CScriptManager::L_NotifyCurDayOnlineReset(lua_State *L)		//通知玩家每日在线时长活动重置
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	SACurDayOnlineResetMsg Msg;
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SACurDayOnlineResetMsg));
	return 0;
}

int CScriptManager::L_NotifyCurDayOnlineAddNewAward(lua_State *L)	//玩家达到一个新的可以领取的奖励
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	SACurDayOnlineAddAwardMsg Msg;
	Msg.m_OnlineLevelID = static_cast<BYTE>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SACurDayOnlineAddAwardMsg));
	return 0;
}

int CScriptManager::L_GetPlayerAddUpLoginTime(lua_State *L)		//获取玩家累计登陆时间
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	DWORD PlayerAddUpLoginTime = g_Script.m_pPlayer->m_Property.m_OnlineTime + time(NULL) - g_Script.m_pPlayer->m_dwLoginTime;
	lua_pushnumber(L, PlayerAddUpLoginTime);
	return 1;
}

int CScriptManager::L_SendPlayerAddUpLogintime(lua_State *L)	//发送玩家累计登陆时间
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	SAAddUpLoginDayMsg Msg;
	Msg.m_AddUpLoginDay = static_cast<WORD>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SAAddUpLoginDayMsg));
	return 0;
}

int CScriptManager::L_SendPlayerAddUpLoginInf(lua_State *L)		//发送玩家累计登陆基本信息
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}

	//表为累计登陆基本数据表
	if (!lua_isnumber(L, 1) || !lua_istable(L, 2))
	{
		return 0;
	}
	SAAddUpLoginInitMsg Msg;
	Msg.m_AddUpLevelNum = static_cast<BYTE>(lua_tonumber(L, 1));
	Msg.m_lpAddUpLoginBaseData = new AddUpLoginBaseData[Msg.m_AddUpLevelNum];
	memset(Msg.m_lpAddUpLoginBaseData, 0, sizeof(AddUpLoginBaseData)* Msg.m_AddUpLevelNum);
	for (int i = 1; i <= Msg.m_AddUpLevelNum; i++)
	{
		lua_rawgeti(L, 2, i);
		if (!lua_istable(L, -1))
			return 0;
		lua_rawgeti(L, -1, 1);	//玩家累计登陆级别ID
		Msg.m_lpAddUpLoginBaseData[i - 1].m_AddUpLoginLevelID = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		lua_rawgeti(L, -1, 3);	//奖励状态（0标识不能领取，1标识可以领取，2标识已经领取）
		Msg.m_lpAddUpLoginBaseData[i - 1].m_AwardFlag = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		lua_rawgeti(L, -1, 2);	//需要累计登陆天数
		Msg.m_lpAddUpLoginBaseData[i - 1].m_NeedDay = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 4);	//累计登陆奖励的道具信息
		if (!lua_istable(L, -1))
			return 0;
		int ItemTablesize = luaL_getn(L, -1);
		if (ItemTablesize > 2 * (ADDUPLOGINMAXAWARDNUM) || ItemTablesize % 2 == 1)
			return 0;
		for (int j = 0; j < ItemTablesize; j++)
		{
			lua_rawgeti(L, -1, j + 1);	//累计登陆奖励数量（为道具（除经验、真气、信仰之力、银币之外的道具）为道具的数量，否则为实际的奖励值）
			if (j % 2 == 0)
				Msg.m_lpAddUpLoginBaseData[i - 1].m_AwardItem[j/2] = static_cast<DWORD>(lua_tonumber(L, -1));
			else
				Msg.m_lpAddUpLoginBaseData[i - 1].m_AwardItemNum[j / 2] = static_cast<BYTE>(lua_tonumber(L, -1));
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		lua_pop(L, 1);
	}

	int AllDataSize = sizeof(SAAddUpLoginInitMsg)-sizeof(void*)+sizeof(AddUpLoginBaseData)* Msg.m_AddUpLevelNum;		//总数据大小
	int HeadDataSize = sizeof(SAAddUpLoginInitMsg)-sizeof(void*);		//数据头数据大小
	int ValidDataSize = sizeof(AddUpLoginBaseData)* Msg.m_AddUpLevelNum;	//实际数据大小

	BYTE *lpSendBuf = new BYTE[AllDataSize];
	memcpy(lpSendBuf, &Msg, HeadDataSize);
	memcpy(lpSendBuf + HeadDataSize, Msg.m_lpAddUpLoginBaseData, ValidDataSize);
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, lpSendBuf, AllDataSize);
	if (lpSendBuf != NULL)
		delete[] lpSendBuf;
	if (Msg.m_lpAddUpLoginBaseData != NULL)
		delete[] Msg.m_lpAddUpLoginBaseData;
	return 0;
}

int CScriptManager::L_NotifyAddUpLoginAddNewAward(lua_State *L)	//玩家达到了新的可以领取的累计登陆奖励
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	SAAddUpLoginAddNewAwardMsg Msg;
	Msg.m_AddUpLoginLevelID = static_cast<BYTE>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SAAddUpLoginAddNewAwardMsg));
	return 0;
}

int CScriptManager::L_SendAddUpLoginGetAwardRes(lua_State *L)	//发送玩家累计登陆领取奖励的结果
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	SAAddUpLoginGetAwardMsg Msg;
	Msg.m_Ret = static_cast<BYTE>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SAAddUpLoginGetAwardMsg));
	return 0;
}

int CScriptManager::L_SendPlayerRFBInf(lua_State *L)	//发送玩家充值回馈金币信息
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}

	//表为充值回馈基本数据表
	if (!lua_isnumber(L, 1) || !lua_istable(L, 2))
	{
		return 0;
	}
	SARFBInitMsg Msg;
	Msg.m_AwardAllNum = static_cast<BYTE>(lua_tonumber(L, 1));
	Msg.m_lpRFBAwardInf = new RFBAwardInf[Msg.m_AwardAllNum];
	memset(Msg.m_lpRFBAwardInf, 0, sizeof(RFBAwardInf)* Msg.m_AwardAllNum);
	for (int i = 1; i <= Msg.m_AwardAllNum; i++)
	{
		lua_rawgeti(L, 2, i);
		if (!lua_istable(L, -1))
			return 0;
		lua_rawgeti(L, -1, 1);	//玩家充值回馈奖励ID
		Msg.m_lpRFBAwardInf[i - 1].m_AwardID = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		lua_rawgeti(L, -1, 2);	//需要玩家充值总金额
		Msg.m_lpRFBAwardInf[i - 1].m_NeedRechargeGlodNum = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		lua_rawgeti(L, -1, 3);	//奖励状态（0标识不能领取，1标识可以领取，2标识已经领取）
		Msg.m_lpRFBAwardInf[i - 1].m_IsCanGetAwardFlag = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 4);	//充值回馈奖励的道具信息
		if (!lua_istable(L, -1))
			return 0;
		int ItemTablesize = luaL_getn(L, -1);
		if (ItemTablesize > 2 * (ADDUPLOGINMAXAWARDNUM) || ItemTablesize % 2 == 1)
			return 0;
		for (int j = 0; j < ItemTablesize; j++)
		{
			lua_rawgeti(L, -1, j + 1);	//充值回馈奖励数量（为道具（除经验、真气、信仰之力、银币之外的道具）为道具的数量，否则为实际的奖励值）
			if (j % 2 == 0)
				Msg.m_lpRFBAwardInf[i - 1].m_AwardItem[j / 2] = static_cast<DWORD>(lua_tonumber(L, -1));
			else
				Msg.m_lpRFBAwardInf[i - 1].m_AwardItemNum[j / 2] = static_cast<BYTE>(lua_tonumber(L, -1));
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		lua_pop(L, 1);
	}

	int AllDataSize = sizeof(SARFBInitMsg)-sizeof(void*)+sizeof(RFBAwardInf)* Msg.m_AwardAllNum;		//总数据大小
	int HeadDataSize = sizeof(SARFBInitMsg)-sizeof(void*);		//数据头数据大小
	int ValidDataSize = sizeof(RFBAwardInf)* Msg.m_AwardAllNum;	//实际数据大小

	BYTE *lpSendBuf = new BYTE[AllDataSize];
	memcpy(lpSendBuf, &Msg, HeadDataSize);
	memcpy(lpSendBuf + HeadDataSize, Msg.m_lpRFBAwardInf, ValidDataSize);
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, lpSendBuf, AllDataSize);
	if (lpSendBuf != NULL)
		delete[] lpSendBuf;
	if (Msg.m_lpRFBAwardInf != NULL)
		delete[] Msg.m_lpRFBAwardInf;
	return 0;
}

int CScriptManager::L_SendGetRFBAwardRes(lua_State *L)	//发送玩家领取充值回馈奖励结果
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	SARFBGetAwardMsg Msg;
	Msg.m_Ret = static_cast<BYTE>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SARFBGetAwardMsg));
	return 0;
}

int CScriptManager::L_NotifyRFBCanGetNewAward(lua_State *L)	//通知玩家，充值回馈有新的奖励可以领取
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	SARFBAddNewAwardMsg Msg;
	Msg.m_AwardID = static_cast<BYTE>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SARFBAddNewAwardMsg));
	return 0;
}

int CScriptManager::L_SendPlayerLRInf(lua_State *L)		//发送玩家等级竞赛数据信息
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	//表为玩家等级竞赛基本数据表
	if (!lua_isnumber(L, 1) || !lua_istable(L, 2))
	{
		return 0;
	}
	SALRInitMsg Msg;
	Msg.m_LevelRaceNum = static_cast<BYTE>(lua_tonumber(L, 1));
	Msg.m_lpPlayerLevelRaceBaseData = new PlayerLevelRaceBaseData[Msg.m_LevelRaceNum];
	memset(Msg.m_lpPlayerLevelRaceBaseData, 0, sizeof(PlayerLevelRaceBaseData)* Msg.m_LevelRaceNum);
	for (int i = 1; i <= Msg.m_LevelRaceNum; i++)
	{
		lua_rawgeti(L, 2, i);
		if (!lua_istable(L, -1))
			return 0;
		lua_rawgeti(L, -1, 1);	//玩家等级精神数据ID
		Msg.m_lpPlayerLevelRaceBaseData[i - 1].m_DataID = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		lua_rawgeti(L, -1, 2);	//需要玩家等级要求
		Msg.m_lpPlayerLevelRaceBaseData[i - 1].m_NeedLevel = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		lua_rawgeti(L, -1, 3);	//最多人数
		Msg.m_lpPlayerLevelRaceBaseData[i - 1].m_MaxPeopleNum = static_cast<WORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		lua_rawgeti(L, -1, 5);	//奖励状态（0标识不能领取，1标识可以领取，2标识已经领取）
		Msg.m_lpPlayerLevelRaceBaseData[i - 1].m_IsCanGetAwardFlag = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		lua_rawgeti(L, -1, 6);	//剩余人数
		Msg.m_lpPlayerLevelRaceBaseData[i - 1].m_ResiduePeopleNum = static_cast<WORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 4);	//等级竞赛奖励的道具信息
		if (!lua_istable(L, -1))
			return 0;
		int ItemTablesize = luaL_getn(L, -1);
		if (ItemTablesize > 2 * (LEVELRACEMAXAWARDNUM) || ItemTablesize % 2 == 1)
			return 0;
		for (int j = 0; j < ItemTablesize; j++)
		{
			lua_rawgeti(L, -1, j + 1);	//等级竞赛奖励数量（为道具（除经验、真气、信仰之力、银币之外的道具）为道具的数量，否则为实际的奖励值）
			if (j % 2 == 0)
				Msg.m_lpPlayerLevelRaceBaseData[i - 1].m_AwardItem[j / 2] = static_cast<DWORD>(lua_tonumber(L, -1));
			else
				Msg.m_lpPlayerLevelRaceBaseData[i - 1].m_AwardItemNum[j / 2] = static_cast<BYTE>(lua_tonumber(L, -1));
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		lua_pop(L, 1);
	}

	int AllDataSize = sizeof(SALRInitMsg)-sizeof(void*)+sizeof(PlayerLevelRaceBaseData)* Msg.m_LevelRaceNum;		//总数据大小
	int HeadDataSize = sizeof(SALRInitMsg)-sizeof(void*);		//数据头数据大小
	int ValidDataSize = sizeof(PlayerLevelRaceBaseData)* Msg.m_LevelRaceNum;	//实际数据大小

	BYTE *lpSendBuf = new BYTE[AllDataSize];
	memcpy(lpSendBuf, &Msg, HeadDataSize);
	memcpy(lpSendBuf + HeadDataSize, Msg.m_lpPlayerLevelRaceBaseData, ValidDataSize);
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, lpSendBuf, AllDataSize);
	if (lpSendBuf != NULL)
		delete[] lpSendBuf;
	if (Msg.m_lpPlayerLevelRaceBaseData != NULL)
		delete[] Msg.m_lpPlayerLevelRaceBaseData;
	return 0;
}

int CScriptManager::L_SendGetLRAwardRes(lua_State *L)	//发送玩家领取等级竞赛奖励结果
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	SALRGetAwardMsg Msg;
	Msg.m_Ret = static_cast<BYTE>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SALRGetAwardMsg));
	return 0;
}

int CScriptManager::L_NotifyLRCanGetNewAward(lua_State *L)	//通知玩家可以获得等级竞赛新的奖励
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	SALRNotifyAddNewAwardMsg Msg;
	Msg.m_DataID = static_cast<BYTE>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SALRNotifyAddNewAwardMsg));
	return 0;
}

int CScriptManager::L_BroadCastLRResidueTimes(lua_State *L)	//广播等级竞赛剩余次数
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
	{
		return 0;
	}
	SALRBroadCastResidueTimesMsg Msg;
	Msg.m_DataID = static_cast<BYTE>(lua_tonumber(L, 1));
	Msg.m_LRResidueTimes = static_cast<WORD>(lua_tonumber(L, 2));
	BroadcastMsg(&Msg, sizeof(SALRBroadCastResidueTimesMsg));
	return 0;
}

//解析幸运摩天轮记录玩家的数据信息
void AnalyzeFWRecordInf(lua_State *L, int iStackPos, FWRecordPlayerInf* lpResData)
{
	if (!lua_istable(L, iStackPos) || lpResData == NULL)
	{
		return;
	}
	int Tablesize = luaL_getn(L, iStackPos);
	for (int i = 1; i <= Tablesize; i++)
	{
		lua_rawgeti(L, iStackPos, i);
		if (!lua_istable(L, -1))
			return;
		memset(lpResData[i- 1].m_PlayerName, 0, CONST_USERNAME);

		lua_rawgeti(L, -1, 1);
		strcpy(lpResData[i - 1].m_PlayerName, lua_tostring(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 2);
		lpResData[i - 1].m_AwardItemID = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pop(L, 1);
	}
}

//解析幸运摩天轮基本数据信息
void AnalyzeFWBaseInf(lua_State *L, int iStackPos, FWBaseInf* lpResData)
{
	if (!lua_istable(L, iStackPos) || lpResData == NULL)
	{
		return;
	}
	int Tablesize = luaL_getn(L, iStackPos);
	for (int i = 1; i <= Tablesize; i++)
	{
		lua_rawgeti(L, iStackPos, i);
		if (!lua_istable(L, -1))
			return;

		lua_rawgeti(L, -1, 1);
		lpResData[i - 1].m_SerialID = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 2);
		lpResData[i - 1].m_AwardItemID = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 3);
		lpResData[i - 1].m_AwardItmeNum = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pop(L, 1);
	}
}

int CScriptManager::L_SendFWInf(lua_State *L)	//发送幸运摩天轮数据信息
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_istable(L, 1) || !lua_istable(L, 2))
	{
		return 0;
	}
	SAFWLoginInitMsg Msg;//解析幸运摩天轮基本数据信息
	memset(Msg.m_FWBaseInf, 0, sizeof(Msg.m_FWRecordPlayerInf));
	AnalyzeFWBaseInf(L, 1, Msg.m_FWBaseInf);

	//解析幸运摩天轮记录玩家的数据信息
	memset(Msg.m_FWRecordPlayerInf, 0, sizeof(Msg.m_FWRecordPlayerInf));
	AnalyzeFWRecordInf(L, 2, Msg.m_FWRecordPlayerInf);

	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SAFWLoginInitMsg));
	return 0;
}

int CScriptManager::L_SendPlayerFWInf(lua_State *L)	//发送玩家幸运摩天轮倒计时数据信息
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
	{
		return 0;
	}
	SAFWOpenResidueTimeMsg Msg;
	Msg.m_CanUseTimes = static_cast<WORD>(lua_tonumber(L, 1));
	Msg.m_OpenResidueTime = static_cast<DWORD>(lua_tonumber(L, 2));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SAFWOpenResidueTimeMsg));
	return 0;
}

int CScriptManager::L_SendFWSelectGoodsInf(lua_State *L)	//发送选中的物品序号信息
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	SAFWStartGameMsg Msg;
	Msg.m_SerialID = static_cast<BYTE>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SAFWStartGameMsg));
	return 0;
}

int CScriptManager::L_SendFWGEtAwardRes(lua_State *L)	//发送选中中物品领取操作结果
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	SAFWGetAwardMsg Msg;
	Msg.m_Ret = static_cast<BYTE>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SAFWGetAwardMsg));
	return 0;
}

int CScriptManager::L_BroadCastFWUptRecordInf(lua_State *L)	//广播幸运摩天轮的记录信息已经变更
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_istable(L, 1))
	{
		return 0;
	}
	SAFWRecordUptMsg Msg;
	memset(Msg.m_NewFWRecordPlayerInf, 0, sizeof(Msg.m_NewFWRecordPlayerInf));
	//解析幸运摩天轮记录玩家的数据信息
	AnalyzeFWRecordInf(L, 1, Msg.m_NewFWRecordPlayerInf);
	BroadcastMsg(&Msg, sizeof(SAFWRecordUptMsg));
	return 0;
}

int CScriptManager::L_SynRechargeTatolGlod(lua_State *L)	//同步玩家总的充值金币数量
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	SASynRechargeTatolGlodMsg Msg;
	Msg.m_TatolRechargeNum = static_cast<DWORD>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Msg, sizeof(SASynRechargeTatolGlodMsg));
	return 0;
}


int CScriptManager::L_SendSceneSDListMsg(lua_State *L)
{
	CPlayer *pPlayer = 0;
	if (g_Script.m_pPlayer)
	{
		pPlayer = g_Script.m_pPlayer;
	}
	else
	{
		if (lua_isnumber(L,7))
		{
			DWORD gid = static_cast<DWORD>(lua_tonumber(L, 7));
			pPlayer = (CPlayer*)GetObjectByGID(gid)->DynamicCast(IID_PLAYER);
		}
	}
	  
	if (!pPlayer)
	{
		return 0;
	}
	SASceneRewardList pSceneMsg;
	WORD wmsglenth = sizeof(SASceneRewardList);
	memset(pSceneMsg.bitemreward, 0, MAX_IREWARD_SIZE);
	BYTE *ptemp = pSceneMsg.bitemreward;
	
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4) && lua_isnumber(L, 5))
	{
		pSceneMsg.wSceneMap =static_cast<WORD>( lua_tonumber(L, 1));
		pSceneMsg.wsceneIndex = static_cast<WORD>(lua_tonumber(L, 2));
		pSceneMsg.dexp = static_cast<DWORD>(lua_tonumber(L, 3));
		pSceneMsg.dmoney = static_cast<DWORD>(lua_tonumber(L, 4));
		pSceneMsg.wbuymoey = static_cast<WORD>(lua_tonumber(L, 5));

		if (lua_istable(L,6))
		{
			int nsize = luaL_getn(L, 6);
			int *tempstar = new int[nsize];
			for (int i = 0; i<nsize; i++)
			{
				if (GetTableArrayNumber(L, 6, i + 1, tempstar[i]))
				{
					memcpy(ptemp, &tempstar[i], sizeof(int));
					ptemp += sizeof(int);

					if (ptemp - pSceneMsg.bitemreward > MAX_IREWARD_SIZE)
					{
						rfalse(2, 1, "SceneRewardList to large");
						break;
					}
				}
			}
			SAFE_DELETE_ARRAY(tempstar);
		}
		CSceneMap::GetInstance().SendEPSceneMsg(pPlayer, &pSceneMsg, wmsglenth);
	}
	return 1;
}

int CScriptManager::L_KickPlayer(lua_State *L)
{
	CPlayer *Pplayer = g_Script.m_pPlayer;
	if (!Pplayer)
	{
		if (lua_isstring(L,1))
		{
			LPCSTR pstrvalue = static_cast<const char*>(lua_tostring(L, 1));
			Pplayer = (CPlayer*)GetPlayerByName(pstrvalue)->DynamicCast(IID_PLAYER);
		}
	}
	Pplayer->Logout(true);
	return 1;
}



int CScriptManager::L_SendArenaRewardMsg(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_istable(L, 1) && lua_istable(L, 2) && lua_istable(L, 3) && lua_istable(L, 4))
	{
		//table 1
		SASArenaRewardUI sArenaReward;
		GetTableNumber(L, 1, "drewardcoldtime", sArenaReward.drewardcoldtime);
		GetTableNumber(L, 1, "wRankPoint", sArenaReward.wRankPoint);
		GetTableNumber(L, 1, "GetConWin", sArenaReward.bGetConWin);
		GetTableNumber(L, 1, "GetCommonWin", sArenaReward.bGetCommonWin);

		lua_getfield(L, 1, "dConWinItem");
		if (lua_istable(L, -1))
		{
			size_t tlenth = luaL_getn(L, -1);
			for (size_t i = 0; i < tlenth; i++)
			{
				GetTableArrayNumber(L, -1, i + 1, sArenaReward.dConWinItem[i]);
			}
		}
		lua_getfield(L, 1, "bConWinItemNum");
		if (lua_istable(L, -1))
		{
			size_t tlenth = luaL_getn(L, -1);
			for (size_t i = 0; i < tlenth; i++)
			{
				GetTableArrayNumber(L, -1, i + 1, sArenaReward.bConWinItemNum[i]);
			}
		}

		//table 2
		size_t tlenth = luaL_getn(L, 2);
		for (size_t i = 0; i < tlenth; i++)
		{
			lua_rawgeti(L, 2, i + 1);
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "rankstage");
				if (lua_istable(L,-1))
				{
					if (GetTableNumber(L, -1, "wMinRank", sArenaReward.arendreward[i].rankstage.wMinRank))
					{
						lua_pop(L, 1);
					}
					if (GetTableNumber(L, -1, "wMaxRank", sArenaReward.arendreward[i].rankstage.wMaxRank))
					{
						lua_pop(L, 1);
					}
					lua_pop(L, 1);
				}
				lua_getfield(L, -1, "arenaitem");
				if (lua_istable(L, -1))
				{
					size_t tslenth = luaL_getn(L, -1);
					for (size_t j = 0; j < tslenth;j++)
					{
						GetTableArrayNumber(L, -1, j + 1, sArenaReward.arendreward[i].arenaitem[j]);
					}
					lua_pop(L, 1);
				}
				lua_getfield(L, -1, "arenaitemnum");
				if (lua_istable(L, -1))
				{
					size_t tslenth = luaL_getn(L, -1);
					for (size_t j = 0; j < tslenth; j++)
					{
						GetTableArrayNumber(L, -1, j + 1, sArenaReward.arendreward[i].arenaitemnum[j]);
					}
					lua_pop(L, 1);
				}
			}
		}
		lua_pop(L, 1);

		//table 3	
		lua_getfield(L,3, "rankstage");
		if (lua_istable(L, -1))
		{
			if (GetTableNumber(L, -1, "wMinRank", sArenaReward.upstagereward.rankstage.wMinRank))
			{
				lua_pop(L, 1);
			}
			if (GetTableNumber(L, -1, "wMaxRank", sArenaReward.upstagereward.rankstage.wMaxRank))
			{
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		}
		lua_getfield(L, 3, "arenaitem");
		if (lua_istable(L, -1))
		{
			size_t tslenth = luaL_getn(L, -1);
			for (size_t j = 0; j < tslenth; j++)
			{
				GetTableArrayNumber(L, -1, j + 1, sArenaReward.upstagereward.arenaitem[j]);
			}
			lua_pop(L, 1);
		}
		lua_getfield(L, 3, "arenaitemnum");
		if (lua_istable(L, -1))
		{
			size_t tslenth = luaL_getn(L, -1);
			for (size_t j = 0; j < tslenth; j++)
			{
				GetTableArrayNumber(L, -1, j + 1, sArenaReward.upstagereward.arenaitemnum[j]);
			}
			lua_pop(L, 1);
		}
		lua_pop(L, 1);


		//table 4
		lua_getfield(L, 4, "rankstage");
		if (lua_istable(L, -1))
		{
			if (GetTableNumber(L, -1, "wMinRank", sArenaReward.currentreward.rankstage.wMinRank))
			{
				lua_pop(L, 1);
			}
			if (GetTableNumber(L, -1, "wMaxRank", sArenaReward.currentreward.rankstage.wMaxRank))
			{
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		}
		lua_getfield(L, 4, "arenaitem");
		if (lua_istable(L, -1))
		{
			size_t tslenth = luaL_getn(L, -1);
			for (size_t j = 0; j < tslenth; j++)
			{
				GetTableArrayNumber(L, -1, j + 1, sArenaReward.currentreward.arenaitem[j]);
			}
			lua_pop(L, 1);
		}
		lua_getfield(L, 4, "arenaitemnum");
		if (lua_istable(L, -1))
		{
			size_t tslenth = luaL_getn(L, -1);
			for (size_t j = 0; j < tslenth; j++)
			{
				GetTableArrayNumber(L, -1, j + 1, sArenaReward.currentreward.arenaitemnum[j]);
			}
			lua_pop(L, 1);
		}
		CArenaMap::GetInstance().SendArenaMsg(g_Script.m_pPlayer, &sArenaReward, sizeof(SASArenaRewardUI));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendArenaQuestMsg(lua_State *L)
{
	if (!g_Script.m_pPlayer)
	{
		return 0;
	}
	if (lua_istable(L,1))
	{
		SAArenaQuestReward  squestreward;
		GetTableNumber(L, 1, "GetConWin", squestreward.bGetConWin);
		GetTableNumber(L, 1, "GetCommonWin", squestreward.bGetCommonWin);
		GetTableNumber(L, 1, "drewardcoldtime", squestreward.drewardcoldtime);
		GetTableNumber(L, 1, "wRankPoint", squestreward.wRankPoint);
		CArenaMap::GetInstance().SendArenaMsg(g_Script.m_pPlayer, &squestreward, sizeof(SAArenaQuestReward));
		return 1;
	}
	return 0;
}

int CScriptManager::L_getmonsterdatanew(lua_State *L)
{
	if (lua_isnumber(L,1)&&lua_isnumber(L,2))
	{
		DWORD gid = static_cast<DWORD>(lua_tonumber(L,1));
		CMonster *pmonster = (CMonster*)GetObjectByGID(gid)->DynamicCast(IID_MONSTER);
		if (pmonster)
		{
			CMonster *tempmonster = g_Script.m_pMonster;
			g_Script.m_pMonster = pmonster;
			WORD wIndex = static_cast<WORD>(lua_tonumber(L, 2));
			lua_pop(L, 2);
			lua_pushnumber(L, wIndex);
			L_getmonsterdata(L);
			g_Script.m_pMonster = tempmonster;
			return 1;
		}
	}
	return 0;
}

//add by ly 2014/4/28
int CScriptManager::L_SendSignInAwardInfo(lua_State *L)	//发送每日签到奖励信息
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		return 0;
	if (g_Script.m_pPlayer == NULL)
		return 0;
	SAEveryDaySignInAwardMsg SignInAwardInf;
	SignInAwardInf.m_EveryDaySignInIndex = static_cast<BYTE>(lua_tonumber(L, 1));
	SignInAwardInf.m_AwardState = static_cast<BYTE>(lua_tonumber(L, 2));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &SignInAwardInf, sizeof(SAEveryDaySignInAwardMsg));
	return 1;
}
int CScriptManager::L_SendGetSignInAwardResult(lua_State *L)	//发送领取每日签到奖励的结果
{
	if (!lua_isnumber(L, 1))
		return 0;
	if (g_Script.m_pPlayer == NULL)
		return 0;
	SAGetSignInAwardMsg GetSignInAwardInf;
	GetSignInAwardInf.m_OptResult = static_cast<BYTE>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &GetSignInAwardInf, sizeof(SAGetSignInAwardMsg));
	return 1;
}

int CScriptManager::GetQuestSkill(const char* name, DWORD mAttackerGID, DWORD mDefenderGID, float mDefenderWorldPosX, float mDefenderWorldPosY, BYTE dwSkillIndex, SQuestSkill_C2S_MsgBody &sqQuestSkillMsg)
{
	CPlayer *pPlayer = (CPlayer*)GetPlayerByGID(mAttackerGID)->DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		if (lua_gettop(ls) == 0)
		{
			lua_settop(ls, 0);
			lua_getglobal(ls, name);
			if (lua_isnil(ls, -1))
			{
				//rfalse(2,1,FormatString("PrepareFunction %s not find \r\n",funcname));
				lua_pop(ls, 1);
				return 0;
			}
			lua_pushnumber(ls,mAttackerGID);
			lua_pushnumber(ls, mDefenderGID);
			lua_pushnumber(ls, mDefenderWorldPosX);
			lua_pushnumber(ls, mDefenderWorldPosY);
			lua_pushnumber(ls, dwSkillIndex);
			if (lua_pcall(ls, 5, 1, 0) != 0)
			{
				char err[1024];
				sprintf(err, "%s\r\n", lua_tostring(ls, -1));
				rfalse(2, 1, err);
				lua_settop(ls, 0);
				return 0;
			}
			if (lua_gettop(ls) == 1)
			{
				if (lua_istable(ls, 1))
				{
					GetTableNumber(ls, 1, "AttackerGID", sqQuestSkillMsg.mAttackerGID);
					GetTableNumber(ls, 1, "DefenderGID", sqQuestSkillMsg.mDefenderGID);
					GetTableNumber(ls, 1, "DefenderWorldPosX", sqQuestSkillMsg.mDefenderWorldPosX);
					GetTableNumber(ls, 1, "DefenderWorldPosY", sqQuestSkillMsg.mDefenderWorldPosY);
					GetTableNumber(ls, 1, "dwSkillIndex", sqQuestSkillMsg.dwSkillIndex);
				}
			}
			lua_settop(ls, 0);
		}
	}
	return 1;
}

int CScriptManager::L_GetBuffData(lua_State *L)
{
	if (lua_isnumber(L,1))
	{
		DWORD Buffid = static_cast<DWORD>(lua_tonumber(L, 1));
		const SBuffBaseData*  pBuffdata = CBuffService::GetInstance().GetBuffBaseData(Buffid);
		if (pBuffdata)
		{
			lua_createtable(L, 0, 0);
			SetTableNumber(L, "m_ID", pBuffdata->m_ID);
			SetTableNumber(L, "m_DeleteType", pBuffdata->m_DeleteType);
			SetTableNumber(L, "m_SaveType", pBuffdata->m_SaveType);
			SetTableNumber(L, "m_Margin", pBuffdata->m_Margin);
			SetTableNumber(L, "m_WillActionTimes", pBuffdata->m_WillActionTimes);
			SetTableNumber(L, "m_GroupID", pBuffdata->m_GroupID);
			SetTableNumber(L, "m_Weight", pBuffdata->m_Weight);
			SetTableNumber(L, "m_InturptRate", pBuffdata->m_InturptRate);
			SetTableNumber(L, "m_PropertyID", pBuffdata->m_PropertyID);
			SetTableNumber(L, "m_ActionType", pBuffdata->m_ActionType);
			SetTableNumber(L, "m_Type", pBuffdata->m_Type);
			SetTableNumber(L, "m_CanBeReplaced", pBuffdata->m_CanBeReplaced);
			SetTableNumber(L, "m_ReginType", pBuffdata->m_ReginType);
			SetTableNumber(L, "m_ReginShape", pBuffdata->m_ReginShape);
			SetTableNumber(L, "m_ReginHeight", pBuffdata->m_ReginHeight);
			SetTableNumber(L, "m_ReginWidth", pBuffdata->m_ReginWidth);
			return 1;
		}
	}
	return 0;
}

//add by ly 2014/4/30
int CScriptManager::L_GetPlayerSchool(lua_State *L)		//获取玩家角色类型(3为吸血鬼；2为狼人；1为人类)
{
	if (g_Script.m_pPlayer == NULL)
	{
		lua_pushnumber(L, -1);
		return 0;
	}
	int PlayerSchool = g_Script.m_pPlayer->m_Property.m_School;
	lua_pushnumber(L, PlayerSchool);
	return 1;
}


int CScriptManager::GetDamageValue(const char* name, DWORD mAttackerGID, DWORD mDefenderGID, WORD dskillID, struct CFightObject::SDamage &sdamage)
{
	CFightObject *pfight = (CFightObject*)GetObjectByGID(mAttackerGID)->DynamicCast(IID_FIGHTOBJECT);
	if (pfight)
	{
		sdamage.isCritical = false;
		if (lua_gettop(ls) == 0)
		{
			lua_settop(ls, 0);
			lua_getglobal(ls, name);
			if (lua_isnil(ls, -1))
			{
				//rfalse(2,1,FormatString("PrepareFunction %s not find \r\n",funcname));
				lua_pop(ls, 1);
				return 0;
			}
			lua_pushnumber(ls, mAttackerGID);
			lua_pushnumber(ls, mDefenderGID);
			lua_pushnumber(ls, dskillID);
			if (lua_pcall(ls, 3, 1, 0) != 0)
			{
				char err[1024];
				sprintf(err, "%s\r\n", lua_tostring(ls, -1));
				rfalse(2, 1, err);
				lua_settop(ls, 0);
				return 0;
			}
			if (lua_gettop(ls) == 1)
			{
				if (lua_istable(ls, 1))
				{
					GetTableNumber(ls, 1, "wDamage", sdamage.wDamage);
					GetTableNumber(ls, 1, "wDamageExtra", sdamage.wDamageExtra);
					GetTableNumber(ls, 1, "wIgnoreDefDamage", sdamage.wIgnoreDefDamage);
					GetTableNumber(ls, 1, "wReboundDamage", sdamage.wReboundDamage);
					GetTableNumber(ls, 1, "mHitState", sdamage.mHitState);
				}
			}
			lua_settop(ls, 0);
		}
	}
	return 1;
}

//add by ly 2014/5/8 商城相关
int CScriptManager::L_SendShopListInfo(lua_State *L)		//发送商城信息
{
	if (g_Script.m_pPlayer == NULL)
	{
		lua_pushnumber(L, -1);
		return 0;
	}
	if (!lua_istable(L, -1))
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "操作失败！");
		return 0;
	}
	SAShopListMsg ShopList;
	int GoodsTableSize = luaL_getn(L, -1);
	ShopList.m_ShopNum = GoodsTableSize;
	if (GoodsTableSize == 0)
		ShopList.m_lpShopData = NULL;
	else
		ShopList.m_lpShopData = new ShopData[GoodsTableSize];
	for (int i = 1; i <= GoodsTableSize; i++)	//lua中索引从1开始
	{
		lua_rawgeti(L, -1, i);
		if (!lua_istable(L, -1))
		{
			TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "配置错误！");
			delete[] ShopList.m_lpShopData;
			ShopList.m_lpShopData = NULL;
			return 0;
		}

		//填充商城数据
		ShopList.m_lpShopData[i - 1].m_ShopType = static_cast<BYTE>(i);

		lua_rawgeti(L, -1, 1);
		ShopList.m_lpShopData[i - 1].m_RefreshFlag = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 2);
		ShopList.m_lpShopData[i - 1].m_RefreshAllTime = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 3);
		ShopList.m_lpShopData[i - 1].m_RefreshType = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 4);
		ShopList.m_lpShopData[i - 1].m_UpdateFlag = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pop(L, 1);
	}

	//发送商品数据
	g_Script.m_pPlayer->SendShopListInfo(&ShopList);

	delete[] ShopList.m_lpShopData;
	ShopList.m_lpShopData = NULL;
	return 0;
}

int CScriptManager::L_SendShopGoodsListInfo(lua_State *L)		//发送商城中商品信息
{
	if (g_Script.m_pPlayer == NULL)
	{
		lua_pushnumber(L, -1);
		return 0;
	}
	if (!lua_istable(L, -1))
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "操作失败！");
		return 0;
	}
	SAShopGoodsListMsg GoodsList;
	int GoodsTableSize = luaL_getn(L, -1);
	GoodsList.m_GoodsNum = GoodsTableSize;
	if (GoodsTableSize == 0)
		GoodsList.m_lpGoodsData = NULL;
	else
		GoodsList.m_lpGoodsData = new GoodsData[GoodsTableSize];
	for (int i = 1; i <= GoodsTableSize; i++)	//lua中索引从1开始
	{
		lua_rawgeti(L, -1, i);
		if (!lua_istable(L, -1))
		{
			TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "配置错误！");
			delete[] GoodsList.m_lpGoodsData;
			GoodsList.m_lpGoodsData = NULL;
			return 0;
		}

		//填充商品数据

		lua_rawgeti(L, -1, 1);
		GoodsList.m_lpGoodsData[i - 1].m_GoodsType = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 2);
		GoodsList.m_lpGoodsData[i - 1].m_GoodsID = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 3);
		GoodsList.m_lpGoodsData[i - 1].m_OriginalJinBiCost = static_cast<WORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 4);
		GoodsList.m_lpGoodsData[i - 1].m_CurrentJinBiCost = static_cast<WORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 5);
		GoodsList.m_lpGoodsData[i - 1].m_OriginalYinBiCost = static_cast<WORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 6);
		GoodsList.m_lpGoodsData[i - 1].m_CurrentYinBiCost = static_cast<WORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 7);
		GoodsList.m_lpGoodsData[i - 1].m_Count = static_cast<WORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 8);
		GoodsList.m_lpGoodsData[i - 1].m_GoodsIndex = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		lua_rawgeti(L, -1, 9);
		GoodsList.m_lpGoodsData[i - 1].m_ShopType = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pop(L, 1);
	}

	//发送商品数据
	g_Script.m_pPlayer->SendGoodsListInfo(&GoodsList);

	delete[] GoodsList.m_lpGoodsData;
	GoodsList.m_lpGoodsData = NULL;
	return 0;
}

int CScriptManager::L_NotityGoodsUpdated(lua_State *L)		//通知商品更新处理
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
	{
		return 0;
	}

	SANotifySpecialGoodsUpdateMsg UpdateGoodsData;

	//填充商品数据
	UpdateGoodsData.m_ShopType = static_cast<BYTE>(lua_tonumber(L, 1));
	UpdateGoodsData.m_UpdateFlag = static_cast<BYTE>(lua_tonumber(L, 2));
	UpdateGoodsData.m_UpdateRefreshAllTime = static_cast<DWORD>(lua_tonumber(L, 3));

	BroadcastMsg(&UpdateGoodsData, sizeof(SANotifySpecialGoodsUpdateMsg));

	return 0;
}

int CScriptManager::L_SendBuyGoodsRes(lua_State *L)		//处理购买商品操作
{
	if (g_Script.m_pPlayer == NULL)
	{
		lua_pushnumber(L, -1);
		return 0;
	}
	if (!lua_isnumber(L, -1))
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "操作失败！");
		return 0;
	}

	SABuyGoodsMsg AnswerBuyGoods;
	AnswerBuyGoods.m_BuyResult = static_cast<BYTE>(lua_tonumber(L, -1));
	g_Script.m_pPlayer->SendBuyGoodsResult(&AnswerBuyGoods);

}

int CScriptManager::L_GetGoodsIndexByTriIndex(lua_State *L)	//通过商城类型，商品索引1和商品索引2计算商品的唯一索引
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))	//依次为商城类型，商品索引1和商品索引2
	{
		lua_pushnumber(L, 0);
		return 1;
	}
	BYTE ShopIndex = static_cast<BYTE>(lua_tonumber(L, 1));
	BYTE GoodsIndex1 = static_cast<BYTE>(lua_tonumber(L, 2));
	WORD GoodsIndex2 = static_cast<WORD>(lua_tonumber(L, 3));
	DWORD GoodsUniqueIndex = 0;
	GoodsUniqueIndex = (DWORD)(ShopIndex << 24) + (DWORD)(GoodsIndex1 << 16) + GoodsIndex2;
	lua_pushnumber(L, GoodsUniqueIndex);
	return 1;
}

int CScriptManager::L_GetTriIndexByGoodsIndex(lua_State *L)		//通过商品唯一索引获取商城类型，商品索引1和商品索引2
{
	if (!lua_isnumber(L, 1))
	{
		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);
		return 3;
	}
	BYTE ShopIndex = 0;
	BYTE GoodsIndex1 = 0;
	WORD GoodsIndex2 = 0;
	DWORD GoodsUniqueIndex = static_cast<DWORD>(lua_tonumber(L, 1));
	ShopIndex = (BYTE)(GoodsUniqueIndex >> 24);
	GoodsIndex1 = (BYTE)(GoodsUniqueIndex >> 16);
	GoodsIndex2 = (WORD)GoodsUniqueIndex;
	lua_pushnumber(L, ShopIndex);
	lua_pushnumber(L, GoodsIndex1);
	lua_pushnumber(L, GoodsIndex2);
	return 3;
}

int CScriptManager::L_SendShopCountDownRes(lua_State *L)	//发送商城刷新倒计时时间
{
	if (g_Script.m_pPlayer == NULL)
	{
		lua_pushnumber(L, -1);
		return 0;
	}
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "操作失败！");
		return 0;
	}
	SAGetShopCountDownMsg ShopCountDownMsg;
	ShopCountDownMsg.m_ShopType = static_cast<BYTE>(lua_tonumber(L, 1));
	ShopCountDownMsg.m_CountDownTime = static_cast<DWORD>(lua_tonumber(L, 2));
	g_Script.m_pPlayer->SendShopCountDown(&ShopCountDownMsg);
	return 0;
}

int CScriptManager::L_SendGoodsInfo(lua_State *L)	//发送一条商品信息
{
	if (g_Script.m_pPlayer == NULL)
	{
		lua_pushnumber(L, -1);
		return 0;
	}
	if (!lua_istable(L, -1))
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "操作失败！");
		return 0;
	}
	GoodsData m_GoodsData;
	lua_rawgeti(L, -1, 1);
	m_GoodsData.m_GoodsType = static_cast<BYTE>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_rawgeti(L, -1, 2);
	m_GoodsData.m_GoodsID = static_cast<DWORD>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_rawgeti(L, -1, 3);
	m_GoodsData.m_OriginalJinBiCost = static_cast<WORD>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_rawgeti(L, -1, 4);
	m_GoodsData.m_CurrentJinBiCost = static_cast<WORD>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_rawgeti(L, -1, 5);
	m_GoodsData.m_OriginalYinBiCost = static_cast<WORD>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_rawgeti(L, -1, 6);
	m_GoodsData.m_CurrentYinBiCost = static_cast<WORD>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_rawgeti(L, -1, 7);
	m_GoodsData.m_Count = static_cast<WORD>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_rawgeti(L, -1, 8);
	m_GoodsData.m_GoodsIndex = static_cast<DWORD>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_rawgeti(L, -1, 9);
	m_GoodsData.m_ShopType = static_cast<BYTE>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	if (lua_isnumber(L, -2))	//当有第二个参数时，表示活动中点击NPC，通过NPC消息游戏币购买商品
	{
		if (!lua_isnumber(L, -3))
			return 0;
		BYTE DefaultNum = static_cast<BYTE>(lua_tonumber(L, -3));
		SAClickNpcGetSpGoodsInfMsg GoodsInfo;		//填充商品数据
		GoodsInfo.m_DefaultNum = DefaultNum;
		memcpy(&GoodsInfo.m_GoodsData, &m_GoodsData, sizeof(GoodsData));
		g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &GoodsInfo, sizeof(SAClickNpcGetSpGoodsInfMsg));
	}
	else
	{
		SAFindGoodsMsg GoodsInfo;		//填充商品数据
		memcpy(&GoodsInfo.m_GoodsData, &m_GoodsData, sizeof(GoodsData));
		g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &GoodsInfo, sizeof(SAFindGoodsMsg));
	}

	return 0;
}

//add by ly 2014/5/16 玩家第一次注册后调用；用于配置技能到玩家的技能配置面板中
int CScriptManager::L_FirstRegCallSetPlayerSkill(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		return 0;
	DWORD SkillID = static_cast<DWORD>(lua_tonumber(L, 1));		//普通形态技能
	DWORD SkillID1 = static_cast<DWORD>(lua_tonumber(L, 2));	//变身1形态技能
	DWORD SkillID2 = static_cast<DWORD>(lua_tonumber(L, 3));	//变身2形态技能
	SANotityUpdateSkillMsg SetData;
	memset(SetData.m_UpdateCurUsedSkill, 0, sizeof(DWORD)* 10);
	SetData.m_UpdateCurUsedSkill[0] = SkillID;
	SetData.m_UpdateCurUsedSkill[3] = SkillID1;
	SetData.m_UpdateCurUsedSkill[6] = SkillID2;
	memcpy(g_Script.m_pPlayer->m_Property.m_CurUsedSkill, SetData.m_UpdateCurUsedSkill, 10 * sizeof(DWORD));		//更新当前所使用的技能
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &SetData, sizeof(SANotityUpdateSkillMsg));
	return 0;
}

//add by ly 2014/5/16
//VIP相关
int CScriptManager::L_SetCurVipFactor(lua_State *L)		//设置当前VIP等级附加属性
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		return 0;
	memset(g_Script.m_pPlayer->m_VIPFactor, 0, sizeof(WORD)* VFT_MAX);
	g_Script.m_pPlayer->m_VIPFactor[VFT_HP] = static_cast<WORD>(lua_tonumber(L, 1));
	g_Script.m_pPlayer->m_VIPFactor[VFT_GONGJI] = static_cast<WORD>(lua_tonumber(L, 2));
	g_Script.m_pPlayer->m_VIPFactor[VFT_FANGYU] = static_cast<WORD>(lua_tonumber(L, 3));
	return 1;
}

int CScriptManager::L_SendVipStateInf(lua_State *L)		//发送玩家VIP的状态信息
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_istable(L, 1))	
		return 0;
	int TableSize = luaL_getn(L, 1);
	if (TableSize != 10)//vip等级最大为10
		return 0;
	SAInitVipInfMsg InitVipInfo;
	for (int i = 0; i < TableSize; i++)
	{
		lua_rawgeti(L, 1, i + 1);
		InitVipInfo.m_VipGetGiftState[i] = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
	}
	g_Script.m_pPlayer->SendVipStateInfOpt(&InitVipInfo);
	return 0;
}

int CScriptManager::L_SendGiftInfo(lua_State *L)	//发送礼包数据信息
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_istable(L, 1))
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "操作失败！");
		return 0;
	}
	SAGetVipGiftInfMsg GetVipGiftInf;
	int DataSize = luaL_getn(L, 1);
	GetVipGiftInf.m_ItemSize = DataSize / 2;
	GetVipGiftInf.m_pItemID = new DWORD[GetVipGiftInf.m_ItemSize];
	GetVipGiftInf.m_pItemIDNum = new BYTE[GetVipGiftInf.m_ItemSize];
	for (int i = 0; i < DataSize; i++)
	{
		lua_rawgeti(L, 1, i + 1);
		if (i % 2 == 0)
			GetVipGiftInf.m_pItemID[i / 2] = static_cast<DWORD>(lua_tonumber(L, -1));
		else
			GetVipGiftInf.m_pItemIDNum[i / 2] = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
	}
	g_Script.m_pPlayer->SendGiftInfoOpt(&GetVipGiftInf);
	delete[] GetVipGiftInf.m_pItemID;
	delete[] GetVipGiftInf.m_pItemIDNum;
	GetVipGiftInf.m_pItemID = NULL;
	GetVipGiftInf.m_pItemIDNum = NULL;
	return 0;
}

int CScriptManager::L_SendGetOrButGiftRes(lua_State *L)		//发送领取或获取VIP礼包结果信息
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1))
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "操作失败！");
		return 0;
	}
	SAGetVipGiftMsg AnswerGetVipGiftOpt;
	AnswerGetVipGiftOpt.m_Result = static_cast<BYTE>(lua_tonumber(L, 1));
	g_Script.m_pPlayer->SendGetOrBuyGiftResOpt(&AnswerGetVipGiftOpt);
	return 0;
}

int CScriptManager::L_AddSomeGoods(lua_State *L)	//添加一系列道具到背包中，如果背包不足则全部都不能添加
{
	if (g_Script.m_pPlayer == NULL)
	{
		lua_pushnumber(L, 0);
		return 1;
	}
	if (!lua_istable(L, 1))
	{
		lua_pushnumber(L, 0);
		return 1;
	}
	std::list<SAddItemInfo> itemList;
	DWORD additem = 0;
	WORD  addnum = 0;
	//获取表中的装备信息
	for (int k = 1; k <= luaL_getn(L, 1); k++)
	{
		lua_rawgeti(L, 1, k);
		if (k % 2 == 1)
			additem = static_cast<DWORD>(lua_tonumber(L, -1));
		else
		{
			addnum = static_cast<WORD>(lua_tonumber(L, -1));
			itemList.push_back(SAddItemInfo(additem, addnum));
		}
		lua_pop(L, 1);
	}

	//判断是否有足够的格子
	if (g_Script.m_pPlayer->CanAddItems(itemList))		//判断能否添加道具
	{
		g_Script.m_pPlayer->StartAddItems(itemList);	//添加道具
		lua_pushnumber(L, 1);
		return 1;
	}
	//添加失败
	lua_pushnumber(L, 0);
	return 1;
}


int CScriptManager::L_AddBuff(lua_State *L)
{
	if (lua_isnumber(L,1)&&lua_isnumber(L,2))
	{
		DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
		WORD buffid = static_cast<WORD>(lua_tonumber(L, 2));
		CFightObject *pfight = (CFightObject*)GetObjectByGID(gid)->DynamicCast(IID_FIGHTOBJECT);
		if (pfight)
		{
			lua_pushnumber(L, pfight->m_buffMgr.AddBuff(buffid, pfight));
			return 1;
		}
	}
	return 0;
}

int CScriptManager::L_SendKeYinResult(lua_State *L)
{
	if (g_Script.m_pPlayer&&lua_isnumber(L,1))
	{
		SABeginKeYin pBeginKeYinInfo;
		pBeginKeYinInfo.bResult = static_cast<BYTE>(lua_tonumber(L, 1));
		g_Script.m_pPlayer->SendKeYinResult(&pBeginKeYinInfo, sizeof(SABeginKeYin));
		return 1;
	}
	return 0;
}

int CScriptManager::GetEquiptFightPower(const char* name, DWORD dEquiptUserGID, BYTE bType, BYTE bIndex, DWORD &dEquiptFightPower)
{
	CFightObject *pfight = (CFightObject*)GetObjectByGID(dEquiptUserGID)->DynamicCast(IID_FIGHTOBJECT);
	if (pfight)
	{
		dEquiptFightPower = 0;
		if (lua_gettop(ls) == 0)
		{
			lua_settop(ls, 0);
			lua_getglobal(ls, name);
			if (lua_isnil(ls, -1))
			{
				//rfalse(2,1,FormatString("PrepareFunction %s not find \r\n",funcname));
				lua_pop(ls, 1);
				return 0;
			}
			lua_pushnumber(ls, bType);
			lua_pushnumber(ls, bIndex);
			if (lua_pcall(ls, 2, 1, 0) != 0)
			{
				char err[1024];
				sprintf(err, "%s\r\n", lua_tostring(ls, -1));
				rfalse(2, 1, err);
				lua_settop(ls, 0);
				return 0;
			}
			if (lua_gettop(ls) == 1)
			{
				if (lua_isnumber(ls,1))
				{
					dEquiptFightPower = static_cast<DWORD>(lua_tonumber(ls, 1));
				}
			}
			lua_settop(ls, 0);
			return 1;
		}
	}
	return 0;
}

int CScriptManager::L_MoveToPos(lua_State *L)
{
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L,3))
	{
		DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
		float fx = static_cast<float>(lua_tonumber(L,2));
		float fy = static_cast<float>(lua_tonumber(L, 3));
		CFightObject *pfight = (CFightObject*)GetObjectByGID(gid)->DynamicCast(IID_FIGHTOBJECT);
		if (pfight)
		{
			pfight->SetMovePos(fx, fy);
		}
		return 1;
	}
	return 0;
}

int CScriptManager::L_CheckFightState(lua_State *L)
{
	if (lua_isnumber(L,1)&&lua_isnumber(L,2))
	{
		DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
		DWORD state = static_cast<DWORD>(lua_tonumber(L,2));
		CFightObject *pfight = (CFightObject*)GetObjectByGID(gid)->DynamicCast(IID_FIGHTOBJECT);
		if (pfight)
		{
			bool bFightState = pfight->CheckFightState(state);
			lua_pushnumber(L, bFightState);
			return 1;
		}
	}
	return 0;
}



//add by ly 2014/5/23 宠物相关消息
int CScriptManager::L_GetPlayerPetNum(lua_State *L)	//获取玩家已经拥有了的宠物数量
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	lua_pushnumber(L, g_Script.m_pPlayer->m_Property.m_CurPetNum);
	return 1;
}

int CScriptManager::L_AddNewPet(lua_State *L)	//添加宠物
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	//参数依次为：宠物基本数据、宠物战斗数据、宠物技能组、宠物星级、宠物对应星级下的最大等级、创建宠物类型（1为抽取创建、2为合成创建、3为使用道具创建）
	if (!lua_istable(L, 1) || !lua_istable(L, 2) || !lua_istable(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5) || !lua_isnumber(L, 6))
		return 0;
	BYTE PetStar = static_cast<BYTE>(lua_tonumber(L, 4));
	BYTE PetCurStarMaxLevel = static_cast<BYTE>(lua_tonumber(L, 5));
	BYTE OptType = static_cast<BYTE>(lua_tonumber(L, 6));

	BYTE PetLevel = 1;
	if (lua_isnumber(L, 7))
		PetLevel = static_cast<BYTE>(lua_tonumber(L, 7));
	SNewPetData CreatePetData;
	//当前宠物的经验为0
	CreatePetData.m_CurPetExp = 0;
	CreatePetData.m_PetLevel = PetLevel;
	CreatePetData.m_PetStarAndMaxLevel = (PetStar << 8) + PetCurStarMaxLevel;
	memset(CreatePetData.m_CurUseSkill, 0xff, sizeof(CreatePetData.m_CurUseSkill));
	memset(CreatePetData.m_PetSkill, 0, sizeof(CreatePetData.m_PetSkill));

	//宠物ID
	lua_rawgeti(L, 1, 1);
	CreatePetData.m_PetID = static_cast<DWORD>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	//宠物名字
	lua_rawgeti(L, 1, 7);
	//char *AnsiPetName = g_Utf8ToAnsi(lua_tostring(L, -1));
	memset(CreatePetData.m_PetName, 0, PETNAMEMAXLEN);
	strcpy(CreatePetData.m_PetName, /*AnsiPetName*/lua_tostring(L, -1));
	//::MultiByteToWideChar(CP_UTF8, 0, lua_tostring(L, -1), -1, (LPWSTR)CreatePetData.m_PetName,15);
	//strcpy(CreatePetData.m_PetName, lua_tostring(L, -1));
	//if (AnsiPetName != NULL)
	//	delete[] AnsiPetName;
	lua_pop(L, 1);

	//宠物当前血量
	lua_rawgeti(L, 2, 5);
	CreatePetData.m_CurPetHp = static_cast<DWORD>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	//当前精力
	lua_rawgeti(L, 2, 6);
	CreatePetData.m_CurPetMp = static_cast<DWORD>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	//当前魂力
	lua_rawgeti(L, 2, 4);
	CreatePetData.m_CurPetDurable = static_cast<DWORD>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	//当前可以拥有技能
	int SkillSize = luaL_getn(L, 3) - 1;	//获取可以拥有的技能数量
	for (int i = 0; i < SkillSize; i++)
	{
		lua_rawgeti(L, 3, i + 2);
		CreatePetData.m_PetSkill[i].m_PetSkillID = static_cast<DWORD>(lua_tonumber(L, -1));
		CreatePetData.m_PetSkill[i].m_Level = 1;
		lua_pop(L, 1);
	}
	
	if (OptType == 1)	//卡牌抽取宠物
	{
		SAExtractPetMsg ExtractPetRes;
		memcpy(&ExtractPetRes.m_ExtractPet, &CreatePetData, sizeof(SNewPetData));
		g_Script.m_pPlayer->SendExtractPetRes(&ExtractPetRes);
	}
	else if (OptType == 2)	//碎片合成宠物
	{
		SAChipComposPetMsg ChipComposePetRes;
		memcpy(&ChipComposePetRes.m_ClipComposePet, &CreatePetData, sizeof(SNewPetData));
		g_Script.m_pPlayer->SendClipComposePetRes(&ChipComposePetRes);
	}
	else if (OptType == 3)	//道具兑换
	{
		SAUsePetItem UseItemGetPet;
		memcpy(&UseItemGetPet.m_GetPet, &CreatePetData, sizeof(SNewPetData));
		g_Script.m_pPlayer->SendUseItemGetPet(&UseItemGetPet);
	}
	else if (OptType == 4)	//金币直接购买
	{
		SAGlodDirectBuyPetMsg BuyPet;
		memcpy(&BuyPet.m_BuyPet, &CreatePetData, sizeof(SNewPetData));
		g_Script.m_pPlayer->SendBuyPet(&BuyPet);
	}

	g_Script.m_pPlayer->AddPet(&CreatePetData);
	g_Script.m_pPlayer->m_Property.m_CurPetNum++;
	return 0;
}

int CScriptManager::L_SendCardPetInf(lua_State *L)	//发送卡牌宠物列表信息
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_istable(L, 1))
		return 0;
	int CardPetNum = luaL_getn(L, 1);
	if (CardPetNum == 0)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "卡牌中没有宠物数据");
		return 0;
	}
	SAGetCardPetInf PetCardInf;
	PetCardInf.m_PetCount = CardPetNum;
	PetCardInf.m_pPetID = new DWORD[CardPetNum];
	for (int i = 0; i < CardPetNum; i++)
	{
		lua_rawgeti(L, 1, i + 1);
		PetCardInf.m_pPetID[i] = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);
	}
	g_Script.m_pPlayer->SendCardPetInfo(&PetCardInf);
	delete[] PetCardInf.m_pPetID;
	return 0;
}

int CScriptManager::L_SendPetCardInf(lua_State *L)	//发送宠物卡牌信息
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_istable(L, 1))
		return 0;
	int CardNum = luaL_getn(L, 1);
	if (CardNum == 0)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "没有卡牌数据");
		return 0;
	}
	SAGetPetCardInf PetCardInf;
	PetCardInf.m_CardCount = CardNum;
	PetCardInf.m_pCardCanExtractTimes = new BYTE[CardNum];
	PetCardInf.m_pUsePrice = new DWORD[CardNum];
	PetCardInf.m_pCardCountDown = new DWORD[CardNum];
	memset(PetCardInf.m_pCardCountDown, 0, sizeof(DWORD) * CardNum);
	for (int i = 0; i < CardNum; i++)
	{
		lua_rawgeti(L, 1, i + 1);
		lua_getfield(L, -1, "ExtracteTimes");
		PetCardInf.m_pCardCanExtractTimes[i] = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_getfield(L, -1, "ConsumeJinBi");
		PetCardInf.m_pUsePrice[i] = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pop(L, 1);

		lua_rawgeti(L, 2, i + 1);
		PetCardInf.m_pCardCountDown[i] = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);
	}

	g_Script.m_pPlayer->SendPetCardInfo(&PetCardInf);
	delete[] PetCardInf.m_pCardCanExtractTimes;
	delete[] PetCardInf.m_pCardCountDown;
	delete[] PetCardInf.m_pUsePrice;
	return 0;
}


int CScriptManager::L_ExceSwallowPetOpt(lua_State *L)	//吞噬宠物
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1) || !lua_istable(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5))
		return 0;
	BYTE SrcPetIndex = static_cast<BYTE>(lua_tonumber(L, 1));
	int GrowExpSwitchScale = static_cast<int>(lua_tonumber(L, 3));
	int BringExpSwitchScale = static_cast<int>(lua_tonumber(L, 4));
	int AllExpAttachScale = static_cast<int>(lua_tonumber(L, 5));
	LPSNewPetData pSrcPetData = g_Script.m_pPlayer->GetPetByIndex(SrcPetIndex);
	if (pSrcPetData == NULL)
		return 0;

	BYTE DestPetIDNum = static_cast<BYTE>(luaL_getn(L, 2));
	for (BYTE i = 0; i < DestPetIDNum; i++)
	{
		lua_rawgeti(L, 2, i + 1);
		BYTE DestPetIndex = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		if (SrcPetIndex == DestPetIndex)
			continue;
		LPSNewPetData pDestPetData = g_Script.m_pPlayer->GetPetByIndex(DestPetIndex);
		//吞噬
		if (pDestPetData != NULL)
		{
			DWORD PetIndex = g_Script.m_pPlayer->_L_GetLuaValue("GetPetDataIndex", pDestPetData->m_PetID, pDestPetData->m_PetLevel);
			if (PetIndex == 0xffffffff)
				continue;

			DWORD PetFightDataIndex = g_Script.m_pPlayer->_L_GetLuaValue("GetPetBaseProperties", PetIndex, 4);
			if (PetFightDataIndex == 0xffffffff)
				PetFightDataIndex = 0;

			DWORD DestPetBringExp = g_Script.m_pPlayer->_L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 3);
			if (DestPetBringExp == 0xffffffff)
				DestPetBringExp = 0;
			//增加宠物的经验
			pSrcPetData->m_CurPetExp += pDestPetData->m_CurPetExp * ((float)GrowExpSwitchScale / 100) + DestPetBringExp * (((float)BringExpSwitchScale) / 100);

			//删除
			g_Script.m_pPlayer->DeletePet(DestPetIndex);
		}
	}

	pSrcPetData->m_CurPetExp = ((float)(pSrcPetData->m_CurPetExp * (100 + AllExpAttachScale))) / 100;

	//计算宠物的等级和经验值
	BYTE CurPetMaxLevel = (BYTE)pSrcPetData->m_PetStarAndMaxLevel;
	for (BYTE i = CurPetMaxLevel; i >= pSrcPetData->m_PetLevel; i--)
	{
		//获取等级i的最大成长经验
		DWORD PetIndex = g_Script.m_pPlayer->_L_GetLuaValue("GetPetDataIndex", pSrcPetData->m_PetID, i);
		if (PetIndex == 0xffffffff)
			continue;
		//等级为i的最大成长经验
		DWORD iPetMaxExp = g_Script.m_pPlayer->_L_GetLuaValue("GetPetBaseProperties", PetIndex, 6);
		//if (iPetMaxExp == 0xffffffff)
		//	iPetMaxExp = 0;
		DWORD PetFightDataIndex = g_Script.m_pPlayer->_L_GetLuaValue("GetPetBaseProperties", PetIndex, 4);
		if (PetFightDataIndex == 0xffffffff)
			PetFightDataIndex = 0;

		DWORD CurLevelPetHp = g_Script.m_pPlayer->_L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 5);
		if (CurLevelPetHp == 0xffffffff)
			CurLevelPetHp = 0;

		//宠物已经达到当前星级的最大等级
		if (i == CurPetMaxLevel && iPetMaxExp <= pSrcPetData->m_CurPetExp)
		{
			pSrcPetData->m_CurPetExp = iPetMaxExp;
			pSrcPetData->m_PetLevel = CurPetMaxLevel;
			pSrcPetData->m_CurPetHp = CurLevelPetHp;
			break;
		}

		//宠物的没有升等级
		if (i == pSrcPetData->m_PetLevel && iPetMaxExp > pSrcPetData->m_CurPetExp)
			break;

		if (iPetMaxExp <= pSrcPetData->m_CurPetExp)
		{
			//获取等级i + 1的最大血量
			DWORD mPetIndex = g_Script.m_pPlayer->_L_GetLuaValue("GetPetDataIndex", pSrcPetData->m_PetID, i + 1);
			if (mPetIndex == 0xffffffff)
				continue;
			DWORD mPetFightDataIndex = g_Script.m_pPlayer->_L_GetLuaValue("GetPetBaseProperties", mPetIndex, 4);
			if (mPetFightDataIndex == 0xffffffff)
				mPetFightDataIndex = 0;

			DWORD mCurLevelPetHp = g_Script.m_pPlayer->_L_GetLuaValue("GetPetFightBaseProperties", mPetFightDataIndex, 5);
			if (mCurLevelPetHp == 0xffffffff)
				mCurLevelPetHp = 0;
			pSrcPetData->m_PetLevel = i + 1;
			pSrcPetData->m_CurPetHp = mCurLevelPetHp;
			break;
		}
	}
	return 0;
}
int CScriptManager::L_PetBreachOpt(lua_State *L)	//宠物突破（宠物升星）
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1) || !lua_istable(L, 2))
		return 0;
	BYTE PetIndex = static_cast<BYTE>(lua_tonumber(L, 1));
	LPSNewPetData pPetData = g_Script.m_pPlayer->GetPetByIndex(PetIndex);
	if (pPetData == NULL)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "没有该宠物ID");
		return 0;
	}

	int PetStarNum = luaL_getn(L, 2);
	WORD PetStarAndMaxLevel = pPetData->m_PetStarAndMaxLevel;
	BYTE PetStar = (BYTE)(PetStarAndMaxLevel >> 8);

	//突破
	PetStar++;
	BYTE PetMaxLevel = 0;

	//获取该星级的最大等级
	for (int i = 1; i <= PetStarNum; i++)
	{
		if (i == PetStar)
		{
			lua_rawgeti(L, 2, i);
			PetMaxLevel = static_cast<BYTE>(lua_tonumber(L, -1));
			lua_pop(L, 1);
			break;
		}
	}
	if (PetMaxLevel == 0)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "没有该宠物星级数据，操作失败");
		return 0;
	}
	PetStarAndMaxLevel = (PetStar << 8) + PetMaxLevel;
	pPetData->m_PetStarAndMaxLevel = PetStarAndMaxLevel;
	return 1;
}

int CScriptManager::L_PetRenameOpt(lua_State *L)	//更改宠物名字
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1) || !lua_isstring(L, 2))
		return 0;

	BYTE PetIndex = static_cast<BYTE>(lua_tonumber(L, 1));
	LPSNewPetData pPetData = g_Script.m_pPlayer->GetPetByIndex(PetIndex);
	if (pPetData == NULL)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "没有该宠物");
		return 0;
	}

	const char* PetNewName = lua_tostring(L, 2);
	memset(pPetData->m_PetName, 0, PETNAMEMAXLEN);
	strcpy(pPetData->m_PetName, PetNewName);
	//同步更名信息
	if (PetIndex == g_Script.m_pPlayer->m_Property.m_FightPetActived)		//同步变身玩家信息
		g_Script.m_pPlayer->SendMyState();
	if (PetIndex == g_Script.m_pPlayer->m_Property.m_FollowPetIndex)	//同步跟随宠物信息
	{
		if (g_Script.m_pPlayer->m_pFightPet)
			if (g_Script.m_pPlayer->m_pFightPet->m_ParentArea)
				g_Script.m_pPlayer->m_pFightPet->m_ParentArea->SendAdj(g_Script.m_pPlayer->m_pFightPet->GetStateMsg(), sizeof(SASynFightPetMsg), 0);
	}
	return 1;
}

int CScriptManager::L_SendPetMergerInf(lua_State *L)	//发送宠物吞噬结果消息
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	if (!lua_isnumber(L, 1))
		return 0;

	BYTE PetIndex = static_cast<BYTE>(lua_tonumber(L, 1));
	LPSNewPetData pPetData = g_Script.m_pPlayer->GetPetByIndex(PetIndex);
	if (pPetData == NULL)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "没有该宠物");
		return 0;
	}
	SAMergerPetMsg PetUpgradeData;
	PetUpgradeData.m_PetLevel = pPetData->m_PetLevel;
	PetUpgradeData.m_PetHp = pPetData->m_CurPetHp;
	PetUpgradeData.m_PetExp = pPetData->m_CurPetExp;
	PetUpgradeData.m_PetMp = pPetData->m_CurPetMp;
	PetUpgradeData.m_PetDurable = pPetData->m_CurPetDurable;
	g_Script.m_pPlayer->SendPetMergerRes(&PetUpgradeData);
}

int CScriptManager::L_SendPetBreachInf(lua_State *L)	//发送宠物突破消息
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	if (!lua_isnumber(L, 1))
		return 0;

	BYTE PetIndex = static_cast<BYTE>(lua_tonumber(L, 1));
	LPSNewPetData pPetData = g_Script.m_pPlayer->GetPetByIndex(PetIndex);
	if (pPetData == NULL)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "没有该宠物");
		return 0;
	}
	SAPetBreachStarMsg PetUpgradeData;
	PetUpgradeData.m_PetStarAndMaxLevel = pPetData->m_PetStarAndMaxLevel;
	g_Script.m_pPlayer->SendPetBreachRes(&PetUpgradeData);
}

int CScriptManager::L_SendPetRenameInf(lua_State *L)	//发送宠物重命名消息
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		return 0;

	BYTE PetIndex = static_cast<BYTE>(lua_tonumber(L, 1));
	LPSNewPetData pPetData = g_Script.m_pPlayer->GetPetByIndex(PetIndex);
	if (pPetData == NULL)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "没有该宠物");
		return 0;
	}
	BYTE Res = static_cast<BYTE>(lua_tonumber(L, 2));
	SARenamePetMsg NotifyPlayerRes;
	NotifyPlayerRes.m_Result = Res;
	g_Script.m_pPlayer->SendPetRenameRes(&NotifyPlayerRes);
}

int CScriptManager::L_GetPetIDAndStar(lua_State *L)		//获取宠物的ID和宠物的星级通过玩家宠物的索引
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	if (!lua_isnumber(L, 1))
		return 0;

	BYTE PetIndex = static_cast<BYTE>(lua_tonumber(L, 1));
	LPSNewPetData pPetData = g_Script.m_pPlayer->GetPetByIndex(PetIndex);
	if (pPetData == NULL)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "没有该宠物");
		return 0;
	}
	BYTE PetStar = (BYTE)(pPetData->m_PetStarAndMaxLevel >> 8);
	lua_pushnumber(L, pPetData->m_PetID);
	lua_pushnumber(L, PetStar);
	return 2;
}

int CScriptManager::L_PetSkillCfg(lua_State *L)		//设置宠物当前可以使用的技能
{
	if (g_Script.m_pPlayer == NULL)
		return 0;

	if (!lua_isnumber(L, 1) || !lua_istable(L, 2))
		return 0;
	BYTE PetIndex = static_cast<BYTE>(lua_tonumber(L, 1));
	LPSNewPetData pPetData = g_Script.m_pPlayer->GetPetByIndex(PetIndex);
	if (pPetData == NULL)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "没有该宠物");
		return 0;
	}
	int SkillNum = luaL_getn(L, 2);
	BYTE PetCanUsedSkill[PETMAXUSESKILLNUM];
	for (int i = 1; i <= SkillNum; i++)
	{
		lua_rawgeti(L, 2, i);
		PetCanUsedSkill[i - 1] = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
	}
	memcpy(pPetData->m_CurUseSkill, PetCanUsedSkill, sizeof(BYTE)* PETMAXUSESKILLNUM);
	SAPetSkillCfgInf PetSkillReponse;
	PetSkillReponse.m_Res = 1;
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &PetSkillReponse, sizeof(SAPetSkillCfgInf));
	return 0;
}




//add by ly 2014/6/5 获取战斗对象的状态动作
int CScriptManager::L_GetFightObjActionID(lua_State *L)
{
	DWORD dwGID = static_cast<DWORD>(lua_tonumber(L, 1));
	CPlayer* pPlayer = NULL;
	CFightPet* pFightPet = NULL;
	if (dwGID == 0)
	{
		return 0;
	}

	CFightObject *pobject = (CFightObject *)GetObjectByGID(dwGID)->DynamicCast(IID_FIGHTOBJECT);
	if (pobject)
	{
		lua_pushnumber(L, pobject->GetCurActionID());
		return 1;
	}
	BYTE objtype = dwGID >> 28;
	if (objtype == 4)  //宠物
	{
		lua_pushnumber(L, EA_DEAD);
		return 1;
	}
	return 0;
}

int CScriptManager::L_GetPetOwer(lua_State *L)
{
	if (lua_isnumber(L,1))
	{
		DWORD gid = lua_tonumber(L, 1);
		CFightPet *pet = (CFightPet*)GetObjectByGID(gid)->DynamicCast(IID_FIGHT_PET);
		if (pet&&pet->m_owner)
		{
			lua_pushnumber(L, pet->m_owner->GetGID());
			return 1;
		}
	}
	return 0;
}

int CScriptManager::L_CalAndSendPetExp(lua_State *L)	//计算宠物的相关经验信息，并发送给玩家
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_istable(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		return 0;
	int GrowExpSwitchScale = static_cast<int>(lua_tonumber(L, 2));
	int BringExpSwitchScale = static_cast<int>(lua_tonumber(L, 3));
	int AllExpAttachScale = static_cast<int>(lua_tonumber(L, 4));
	
	BYTE DestPetIDNum = static_cast<BYTE>(luaL_getn(L, 1));
	DWORD AddPetExp = 0;	//吞噬宠物可以获得的经验值
	for (BYTE i = 0; i < DestPetIDNum; i++)
	{
		lua_rawgeti(L, 1, i + 1);
		BYTE DestPetIndex = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		LPSNewPetData pDestPetData = g_Script.m_pPlayer->GetPetByIndex(DestPetIndex);
		//计算吞噬后可以获得的经验值
		if (pDestPetData != NULL)
		{
			DWORD PetIndex = g_Script.m_pPlayer->_L_GetLuaValue("GetPetDataIndex", pDestPetData->m_PetID, pDestPetData->m_PetLevel);
			if (PetIndex == 0xffffffff)
				return 0;

			DWORD PetFightDataIndex = g_Script.m_pPlayer->_L_GetLuaValue("GetPetBaseProperties", PetIndex, 4);
			if (PetFightDataIndex == 0xffffffff)
				PetFightDataIndex = 0;

			DWORD DestPetBringExp = g_Script.m_pPlayer->_L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 3);
			if (DestPetBringExp == 0xffffffff)
				DestPetBringExp = 0;
			//增加宠物的经验
			AddPetExp += pDestPetData->m_CurPetExp * ((float)GrowExpSwitchScale / 100) + DestPetBringExp * (((float)BringExpSwitchScale) / 100);
		}
	}
	AddPetExp = ((float)(AddPetExp * (100 + AllExpAttachScale))) / 100;

	SAPetExpInfMsg PetAddExpObj;
	PetAddExpObj.m_PetAddExp = AddPetExp;
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &PetAddExpObj, sizeof(SAPetExpInfMsg));
	return 0;
}

int CScriptManager::L_JudgePetDurableIsZero(lua_State *L)	//判断宠物魂力是否为0
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1))
		return 0;
	BYTE PetIndex = static_cast<BYTE>(lua_tonumber(L, 1));
	LPSNewPetData pPetData = g_Script.m_pPlayer->GetPetByIndex(PetIndex);
	if (pPetData == NULL)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "没有该宠物");
		return 0;
	}
	if (pPetData->m_CurPetDurable == 0)
	{
		lua_pushnumber(L, 1);
		return 1;
	}
	else
		return 0;
}

int CScriptManager::L_SendResumeDurableNeedGold(lua_State *L)	//发送恢复宠物魂力需要的金币
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1))
		return 0;
	DWORD GoldNum = static_cast<DWORD>(lua_tonumber(L, 1));
	SAResumeNdGoldMsg Res;
	Res.m_GoldNum = GoldNum;
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Res, sizeof(SAResumeNdGoldMsg));
	return 0;
}

int CScriptManager::L_SendGoldResumeDurableRes(lua_State *L)	//发送金币恢复宠物魂力的结果
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1))
		return 0;
	BYTE Result = static_cast<BYTE>(lua_tonumber(L, 1));
	SAResumePetDurableMsg Res;
	Res.m_Result = Result;
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Res, sizeof(SAResumePetDurableMsg));
	return 0;
}


int CScriptManager::L_UptPetDurable(lua_State *L)	//更新宠物魂力
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		return 0;
	BYTE PetIndex = static_cast<BYTE>(lua_tonumber(L, 1));
	LPSNewPetData pPetData = g_Script.m_pPlayer->GetPetByIndex(PetIndex);
	if (pPetData == NULL)
	{
		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "没有该宠物");
		return 0;
	}
	DWORD NewPetDurable = static_cast<DWORD>(lua_tonumber(L, 2));
	pPetData->m_CurPetDurable = NewPetDurable;
	return 0;
}

int CScriptManager::L_NotifyPetDurableUpt(lua_State *L)		//通知宠物魂力已经更新
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1))
		return 0;
	BYTE PetIndex = static_cast<BYTE>(lua_tonumber(L, 1));
	g_Script.m_pPlayer->SynPetDurable(PetIndex);
	return 0;
}

int CScriptManager::L_PetStudySkillByBook(lua_State *L)		//宠物通过技能书学习技能
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		return 0;
	BYTE PetIndex = static_cast<BYTE>(lua_tonumber(L, 1));
	DWORD PetSkillBookID = static_cast<DWORD>(lua_tonumber(L, 2));
	const WORD SkillID = CSkillService::GetInstance().GetSkillIdByBookId(PetSkillBookID);
	BOOL bRet = g_Script.m_pPlayer->PetActiveSkill(PetIndex, SkillID);
	if (!bRet)
		return 0;
	lua_pushnumber(L, 1);
	return 1;
}


int CScriptManager::L_ActivePetSkill(lua_State *L)		//激活宠物技能
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		return 0;
	BYTE PetIndex = static_cast<BYTE>(lua_tonumber(L, 1));
	DWORD PetSkillID = static_cast<DWORD>(lua_tonumber(L, 2));
	BOOL bRet = g_Script.m_pPlayer->PetActiveSkill(PetIndex, PetSkillID);
	if (!bRet)
		return 0;
	lua_pushnumber(L, 1);
	return 1;
}


//add by ly 2014/6/25 通知玩家消息（从lua的UTF_8编码转换为Ansi码，并发送给玩家）
int CScriptManager::L_NotifyMsgUtf8ToAnsi(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	LPCSTR lpszUTF8Info = static_cast< LPCSTR >(lua_tostring(L, 1)); 
	char* AnsiStr = g_Utf8ToAnsi(lpszUTF8Info);
	TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, AnsiStr);
	if (AnsiStr != NULL)
		delete[] AnsiStr;

	//SAChatTipsMsg msg;
	//msg.bHasPrefix = true;
	//if (msg.bHasPrefix)
	//	dwt::strcpy(msg.cPrefix, "[系统]", sizeof(msg.cPrefix));
	//msg.wShowType = 1;
	//memcpy(msg.cChatData, pBuf, (WriteByte + 1)*sizeof(wchar_t));
	//g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(SAChatTipsMsg)-500 + (WriteByte + 1)*sizeof(wchar_t));
	return 0;
}


//add by ly 2014/6/27活动相关
int CScriptManager::L_GetFirstPayFlag(lua_State *L)		//获取首次充值状态
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	lua_pushnumber(L, g_Script.m_pPlayer->m_Property.m_PlayerPattern);
	return 1;
}

int CScriptManager::L_SetFirstPayFlag(lua_State *L)		//设置首次充值状态
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, -1))
		return 0;
	g_Script.m_pPlayer->m_Property.m_PlayerPattern = static_cast<BYTE>(lua_tonumber(L, -1));

	SAUptFirstPayFlagMsg NotifyMsg;
	NotifyMsg.m_NewValue = static_cast<BYTE>(lua_tonumber(L, -1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &NotifyMsg, sizeof(SAUptFirstPayFlagMsg));
	return 0;
}

int CScriptManager::L_GetEveryDayPayFlag(lua_State *L)	//获取每日充值的状态
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		return 0;
	DWORD AllEveryDayFlag = static_cast<DWORD>(lua_tonumber(L, 1));
	BYTE EveryDayAwardIndex = static_cast<BYTE>(lua_tonumber(L, 2));
	BYTE MovePos = EveryDayAwardIndex * 2;
	DWORD Value = 3 << MovePos;
	BYTE State = (AllEveryDayFlag & Value) >> MovePos;
	lua_pushnumber(L, State);
	return 1;
}

int CScriptManager::L_SetEveryDayPayFlag(lua_State *L)	//设置每日充值的状态
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		return 0;
	DWORD AllEveryDayFlag = static_cast<DWORD>(lua_tonumber(L, 1));
	BYTE EveryDayAwardIndex = static_cast<BYTE>(lua_tonumber(L, 2));
	BYTE StateValue = static_cast<BYTE>(lua_tonumber(L, 3));
	BYTE MovePos = EveryDayAwardIndex * 2;
	AllEveryDayFlag = AllEveryDayFlag & (~((DWORD)3 << MovePos)) | ((DWORD)StateValue << MovePos);
	lua_pushnumber(L, AllEveryDayFlag);
	return 1;
}


int CScriptManager::L_SendEveryDayPayAwardState(lua_State *L)	//发送每日充值奖励状态
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_istable(L, 1))
		return 0;
	int TableSize = luaL_getn(L, 1);
	if (TableSize > EVERYDAYAWARDNUM)
		TableSize = EVERYDAYAWARDNUM;
	SAGetEveryDayAwardStateMsg Res;
	memset(&Res.m_State, 0, EVERYDAYAWARDNUM);
	for (int i = 1; i <= TableSize; i++)
	{
		lua_rawgeti(L, 1, i);
		Res.m_State[i - 1] = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
	}
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Res, sizeof(SAGetEveryDayAwardStateMsg));
}

int CScriptManager::L_SendGetEDPAwardRes(lua_State *L)		//发送领取每日充值奖励结果
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1))
		return 0;
	SAEveryDayPayAwardMsg Res;
	Res.m_Result = static_cast<BYTE>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Res, sizeof(SAEveryDayPayAwardMsg));
}

int CScriptManager::L_CheckObject(lua_State *L)
{
	DWORD gid = static_cast<DWORD>(lua_tonumber(L, 1));
	LPIObject pobject = GetObjectByGID(gid);
	if (pobject)
	{
		lua_pushnumber(L, 1);
	}
	else
	{
		lua_pushnumber(L, 0);
	}
	return 1;
}

int CScriptManager::GetWorldTalkLimit(CPlayer *pPlayer, const char* funcname, WORD &wresult)
{
	if (pPlayer)
	{
		g_Script.SetPlayer(pPlayer);
		if (lua_gettop(ls) == 0)
		{
			lua_settop(ls, 0);
			lua_getglobal(ls, funcname);
			if (lua_isnil(ls, -1))
			{
				//rfalse(2,1,FormatString("PrepareFunction %s not find \r\n",funcname));
				lua_pop(ls, 1);
				return 0;
			}
			if (lua_pcall(ls, 0, 1, 0) != 0)
			{
				char err[1024];
				sprintf(err, "%s\r\n", lua_tostring(ls, -1));
				rfalse(2, 1, err);
				lua_settop(ls, 0);
				return 0;
			}
			if (lua_gettop(ls) == 1)
			{
				if (lua_isnumber(ls, 1))
				{
					wresult = static_cast<WORD>(lua_tonumber(ls, 1));
				}
			}
			lua_settop(ls, 0);
			g_Script.CleanPlayer();
			return 1;
		}
		
	}
	return 0;
}

int CScriptManager::GetMonsterVipExp(CPlayer *pPlayer, const char* funcname, DWORD dmonsterexp, DWORD &dmonsteraddexp)
{
	if (pPlayer)
	{
		g_Script.SetPlayer(pPlayer);
		if (lua_gettop(ls) == 0)
		{
			lua_settop(ls, 0);
			lua_getglobal(ls, funcname);
			if (lua_isnil(ls, -1))
			{
				//rfalse(2,1,FormatString("PrepareFunction %s not find \r\n",funcname));
				lua_pop(ls, 1);
				return 0;
			}
			lua_pushnumber(ls, dmonsterexp);
			if (lua_pcall(ls, 1, 1, 0) != 0)
			{
				char err[1024];
				sprintf(err, "%s\r\n", lua_tostring(ls, -1));
				rfalse(2, 1, err);
				lua_settop(ls, 0);
				return 0;
			}
			if (lua_gettop(ls) == 1)
			{
				if (lua_isnumber(ls, 1))
				{
					dmonsteraddexp = static_cast<DWORD>(lua_tonumber(ls, 1));
				}
			}
			lua_settop(ls, 0);
			g_Script.CleanPlayer();
			return 1;
		}

	}
	return 0;
}

int CScriptManager::GetItemUseLimit(CPlayer *pPlayer, const char* funcname, DWORD ditemindex, WORD &buse)
{
	if (pPlayer)
	{
		g_Script.SetPlayer(pPlayer);
		if (lua_gettop(ls) == 0)
		{
			lua_settop(ls, 0);
			lua_getglobal(ls, funcname);
			if (lua_isnil(ls, -1))
			{
				//rfalse(2,1,FormatString("PrepareFunction %s not find \r\n",funcname));
				lua_pop(ls, 1);
				return 0;
			}
			lua_pushnumber(ls, ditemindex);
			if (lua_pcall(ls, 1, 1, 0) != 0)
			{
				char err[1024];
				sprintf(err, "%s\r\n", lua_tostring(ls, -1));
				rfalse(2, 1, err);
				lua_settop(ls, 0);
				return 0;
			}
			if (lua_gettop(ls) == 1)
			{
				if (lua_isnumber(ls, 1))
				{
					buse = static_cast<WORD>(lua_tonumber(ls, 1));
				}
			}
			lua_settop(ls, 0);
			g_Script.CleanPlayer();
			return 1;
		}

	}
	return 0;
}

int CScriptManager::LoadAllFactioninfo(SaveFactionData_New *FactionData)
{
	return 1;
}

//add by ly 2014/7/22 排行榜相关
void AnalyzeRankData(lua_State *L, PlayerRankList &SaveData)
{
	if (!lua_istable(L, -1))
		return ;

	lua_getfield(L, -1, "PlayerName");
	memset(SaveData.m_PlayerName, 0, CONST_USERNAME);
	strcpy(SaveData.m_PlayerName, lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "PlayerSchool");
	SaveData.m_PlayerSchool = static_cast<BYTE>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "PlayerPower");
	SaveData.m_PlayerPower = static_cast<DWORD>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "PlayerLevel");
	SaveData.m_PlayerLevel = static_cast<BYTE>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "PlayerGlod");
	SaveData.m_PlayerGlod = static_cast<DWORD>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "PlayerCurRank");
	SaveData.m_PlayerCurRank = static_cast<WORD>(lua_tonumber(L, -1));
	lua_pop(L, 1);
}

int CScriptManager::L_SendRandListData(lua_State *L)	//发送排行榜数据
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_istable(L, 1) || !lua_istable(L, 2) || !lua_isnumber(L, 3))
		return 0;
	WORD TopNDataSize = luaL_getn(L, 1);
	WORD PlayerTopNDataSize = luaL_getn(L, 2);
	BYTE Type = lua_tonumber(L, 3);
	SAPlayerRankList ResponseData;
	ResponseData.m_Type = Type;
	ResponseData.m_TopNSize = TopNDataSize;
	ResponseData.m_PlayerTopNSize = PlayerTopNDataSize;
	ResponseData.m_pPlayerRankList = new PlayerRankList[TopNDataSize + PlayerTopNDataSize];
	int i = 0;
	for (; i < TopNDataSize; i++)
	{
		lua_rawgeti(L, 1, i + 1);

		AnalyzeRankData(L, ResponseData.m_pPlayerRankList[i]);

		lua_pop(L, 1);
	}
	for (int j = 0; j < PlayerTopNDataSize; j++)
	{
		lua_rawgeti(L, 2, j + 1);

		AnalyzeRankData(L, ResponseData.m_pPlayerRankList[i]);
		
		lua_pop(L, 1);
		i++;
	}
	g_Script.m_pPlayer->SendRandListData(&ResponseData);

	if (ResponseData.m_pPlayerRankList != NULL)
		delete[] ResponseData.m_pPlayerRankList;
	return 0;
}

int CScriptManager::L_SendRandAwardFlag(lua_State *L)	//发送玩家排行榜领奖状态
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_istable(L, 1))
		return 0;
	SAGetRankAwardState RetureState;
	BYTE TableSize = luaL_getn(L, 1);
	for (int i = 1; i <= TableSize; i++)
	{
		lua_rawgeti(L, 1, i);
		RetureState.m_RankState[i - 1] = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);
	}
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &RetureState, sizeof(SAGetRankAwardState));
	return 0;
}

int CScriptManager::L_SendGetRandAwardRes(lua_State *L)	//发送领取排行奖励操作结果
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1))
		return 0;
	SAGetRankAward Res;
	Res.m_Result = static_cast<BYTE>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Res, sizeof(SAGetRankAward));
	return 0;
}

//add by ly 2014/7/24 活跃度相关

//解析活跃度任务信息列表
void AnalyzeActivenessTask(lua_State *L, ActivenessTaskInf* pTaskInf, BYTE TaskInfSize)
{
	if (pTaskInf == NULL)
		return;
	for (int i = 1; i <= TaskInfSize; i++)
	{
		lua_rawgeti(L, -1, i);
		if (!lua_istable(L, -1))
			return;
		lua_getfield(L, -1, "TaskCompleteTimes");
		pTaskInf[i - 1].m_TaskCompleteTimes = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_getfield(L, -1, "TaskTotalTimes");
		pTaskInf[i - 1].m_TaskTotalTimes = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_getfield(L, -1, "SingleGetActiveness");
		pTaskInf[i - 1].m_SingleTaskGetNum = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_getfield(L, -1, "TaskIndex");
		pTaskInf[i - 1].m_TaskIndex = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pop(L, 1);
	}
}

//解析活跃度奖励道具信息列表
void AnalyzeActivenessItem(lua_State *L, ActivenessAward *pAwardInf, BYTE AwardInfSize)
{
	if (pAwardInf == NULL)
		return;
	for (int i = 1; i <= AwardInfSize; i++)
	{
		lua_rawgeti(L, -1, i);
		if (!lua_istable(L, -1))
			return;
		lua_getfield(L, -1, "AwardLevel");
		pAwardInf[i - 1].m_AwardLevel = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_getfield(L, -1, "NeedActiveness");
		pAwardInf[i - 1].m_NeedActivenessNum = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_getfield(L, -1, "Awardflag");
		pAwardInf[i - 1].m_IsGetAwardFlag = static_cast<BYTE>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_getfield(L, -1, "AwardItemInf");
		if (!lua_istable(L, -1))
			return;
		int ItemArrNum = luaL_getn(L, -1);
		if (ItemArrNum > ACTIVENESSITEMMAXNUM)
			ItemArrNum = ACTIVENESSITEMMAXNUM;
		for (int j = 1; j <= ItemArrNum; j++)
		{
			lua_rawgeti(L, -1, j);
			if (!lua_istable(L, -1))
				return;

			lua_rawgeti(L, -1, 2);
			pAwardInf[i - 1].m_ItemInfo[j - 1].m_ItmeID = static_cast<DWORD>(lua_tonumber(L, -1));
			lua_pop(L, 1);

			lua_rawgeti(L, -1, 3);
			pAwardInf[i - 1].m_ItemInfo[j - 1].m_ItmeNum = static_cast<BYTE>(lua_tonumber(L, -1));
			lua_pop(L, 1);

			lua_pop(L, 1);
		}

		lua_pop(L, 2);
	}
}

int CScriptManager::L_SendActivenessInfo(lua_State *L)	//发送活跃度详细信息
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_istable(L, 1))
		return 0;
	SAGetActivenessInfo ActivenessInf;
	lua_getfield(L, 1, "CurActivenessNum");
	ActivenessInf.m_ActivenessValue = static_cast<BYTE>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, 1, "TaskInf");
	if (!lua_istable(L, -1))
		return 0;
	int TaskInfSize = luaL_getn(L, -1);
	ActivenessInf.m_ActivenessTaskNum = TaskInfSize;
	ActivenessInf.m_pTaskInfo = new ActivenessTaskInf[TaskInfSize];
	AnalyzeActivenessTask(L, ActivenessInf.m_pTaskInfo, TaskInfSize);
	lua_pop(L, 1);

	lua_getfield(L, 1, "AwardInf");
	if (!lua_istable(L, -1))
		return 0;
	int AwardInfSize = luaL_getn(L, -1);
	ActivenessInf.m_ActivenessAwardNum = AwardInfSize;
	ActivenessInf.m_pAwardInfo = new ActivenessAward[AwardInfSize];
	memset(ActivenessInf.m_pAwardInfo, 0, sizeof(ActivenessAward)* AwardInfSize);
	AnalyzeActivenessItem(L, ActivenessInf.m_pAwardInfo, AwardInfSize);
	lua_pop(L, 1);

	g_Script.m_pPlayer->SendActivenessInf(&ActivenessInf);

	if (ActivenessInf.m_pTaskInfo != NULL)
		delete[] ActivenessInf.m_pTaskInfo;
	if (ActivenessInf.m_pAwardInfo != NULL)
		delete[] ActivenessInf.m_pAwardInfo;
}

int CScriptManager::L_SendGetActivenessAwardRes(lua_State *L)	//发送领取活跃度奖励操作结果
{
	if (g_Script.m_pPlayer == NULL)
		return 0;
	if (!lua_isnumber(L, 1))
		return 0;
	SAGetActivenessAward Res;
	Res.m_Result = static_cast<BYTE>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &Res, sizeof(SAGetActivenessAward));
	return 0;
}

int CScriptManager::L_GetServerStartTime(lua_State *L)
{
	lua_pushnumber(L, GetApp()->m_ServerStartTime);
	return 1;
}

int CScriptManager::L_SendAddFriendToList(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		return 0;
	}
	DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
	CPlayer *pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		if (!lua_istable(L,2))
		{
			return 0;
		}

		SAAddFriendMsg msg;

		GetTableArrayString(L, 2, 1, msg.m_friendData.bFriendName);
		GetTableArrayNumber(L, 2, 2, msg.m_friendData.wIconIndex);
		GetTableArrayNumber(L, 2, 3, msg.m_friendData.wLevel);
		GetTableArrayNumber(L, 2, 4, msg.m_friendData.byType);
		GetTableArrayNumber(L, 2, 5, msg.m_friendData.wFightPower);
		GetTableArrayNumber(L, 2, 6, msg.m_friendData.bonlinestate);
		GetTableArrayNumber(L, 2, 7, msg.m_friendData.bySendFlag);
		GetTableArrayNumber(L, 2, 8, msg.m_friendData.byReceiveFlag); 
		GetTableArrayNumber(L, 2, 9, msg.m_friendData.dwSid);

		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAAddFriendMsg));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendFriendFailMsg(lua_State * L)
{
	DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
	CPlayer *pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	if (pPlayer != nullptr)
	{
		if (!lua_isnumber(L, 2))
		{
			return 0;
		}
		SAFriendFailMsg msg;
		msg.byType = static_cast<DWORD>(lua_tonumber(L, 2));
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAFriendFailMsg));
		return 1;
	}
	return 0;
}

int CScriptManager::L_GetPlayerSidByName(lua_State *L)
{
	if (!lua_isstring(L,1))
	{
		lua_pushnumber(L, -1);
		return 0;
	}
	const char* pname = static_cast<const char*>(lua_tostring(L, 1));
	CPlayer *pPlayer = (CPlayer*)GetPlayerByName(pname)->DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		lua_pushnumber(L, pPlayer->GetSID());
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendFriendList(lua_State *L)
{
	if (!lua_isnumber(L, 1))
	{
		return 0;
	}
	DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
	CPlayer *pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		SRefreshFriendsMsg srefrechmsg;
		srefrechmsg.wFriendCount = 0;
		memset(srefrechmsg.fdata, 0, sizeof(FriendData)*MAX_PEOPLE_NUMBER);

		if (!lua_isnumber(L, 2))
			return 0;
		srefrechmsg.wSendCount = static_cast<DWORD>(lua_tonumber(L, 2));
		if (!lua_isnumber(L, 3))
			return 0;
		srefrechmsg.wReceiveCount = static_cast<DWORD>(lua_tonumber(L, 3));
		if (!lua_isnumber(L, 4))
			return 0;
		srefrechmsg.wSendLimit = static_cast<DWORD>(lua_tonumber(L, 4));
		if (!lua_isnumber(L, 5))
			return 0;
		srefrechmsg.wReceiveLimit = static_cast<DWORD>(lua_tonumber(L, 5));
		if (!lua_isnumber(L, 6))
			return 0;
		srefrechmsg.wStaminaNum = static_cast<DWORD>(lua_tonumber(L, 6));

		if (lua_istable(L,7))
		{
			/*
			int nsize = luaL_getn(L,2);
			SRefreshFriendsMsg srefrechmsg;
			srefrechmsg.wFriendCount = nsize;
			memset(srefrechmsg.fdata, 0, sizeof(FriendData)*MAX_PEOPLE_NUMBER);
			
			for (int i = 0; i < nsize; i++)
			{
				lua_rawgeti(L, 2, i + 1);
				if (lua_istable(L,-1))
				{
					GetTableArrayString(L, -1,1, srefrechmsg.fdata[i].bFriendName);
					GetTableArrayNumber(L, -1, 2, srefrechmsg.fdata[i].wIconIndex);
					GetTableArrayNumber(L, -1, 3, srefrechmsg.fdata[i].wLevel);
					GetTableArrayNumber(L, -1, 4, srefrechmsg.fdata[i].byType);
					GetTableArrayNumber(L, -1, 5, srefrechmsg.fdata[i].wFightPower);
					GetTableArrayNumber(L, -1, 6, srefrechmsg.fdata[i].bonlinestate);
					GetTableArrayNumber(L, -1, 7, srefrechmsg.fdata[i].bySendFlag);
					GetTableArrayNumber(L, -1, 8, srefrechmsg.fdata[i].byReceiveFlag);
					GetTableArrayNumber(L, -1, 9, srefrechmsg.fdata[i].dwSid);
				}
				lua_pop(L, 1);

			}
			*/
			int t_idx = 0;
			int it_idx = 0;
			int key1, key2;
			int count = 0;
			t_idx = lua_gettop(L);
			lua_pushnil(L);// nil 入栈作为初始 key 

			while (lua_next(L, t_idx))//一层
			{
				key1 = lua_tonumber(L, -2);//key1
				if (key1 == 0)
				{
					lua_pop(L, 1);
					continue;
				}
				if (lua_istable(L, -1))
				{
					it_idx = lua_gettop(L);
					lua_pushnil(L);
					while (lua_next(L, it_idx))//2层
					{
						key2 = lua_tonumber(L, -2);//key2
						if (key2 == 0)
						{
							lua_pop(L, 1);
							continue;
						}
							
						switch (key2)
						{
						case 1:
							memcpy(srefrechmsg.fdata[count].bFriendName, lua_tostring(L, -1), CONST_USERNAME);
							//LPCSTR pstrvalue = static_cast<const char*>(lua_tostring(L, -1));
							//strcpy(srefrechmsg.fdata[count].bFriendName, pstrvalue);
							break;
						case 2:
							srefrechmsg.fdata[count].wIconIndex = lua_tonumber(L, -1);//value
							break;
						case 3:
							srefrechmsg.fdata[count].wLevel = lua_tonumber(L, -1);//value
							break;
						case 4:
							srefrechmsg.fdata[count].byType = lua_tonumber(L, -1);//value
							break;
						case 5:
							srefrechmsg.fdata[count].wFightPower = lua_tonumber(L, -1);//value
							break;
						case 6:
							srefrechmsg.fdata[count].bonlinestate = lua_tonumber(L, -1);//value
							break;
						case 7:
							srefrechmsg.fdata[count].bySendFlag = lua_tonumber(L, -1);//value
							break;
						case 8:
							srefrechmsg.fdata[count].byReceiveFlag = lua_tonumber(L, -1);//value
							break;
						case 9:
							srefrechmsg.fdata[count].dwSid = lua_tonumber(L, -1);//value
							break;
						default:
							break;
						}//end switch
						lua_pop(L, 1);
						
					}//end while
				}//end if
				++count;
				lua_pop(L, 1);
			}
			srefrechmsg.wFriendCount = count;
			g_StoreMessage(pPlayer->m_ClientIndex, &srefrechmsg, sizeof(SRefreshFriendsMsg));

			return 1;
		}
	}
	return 0; 
}

int CScriptManager::L_GetAllPlayer(lua_State *L)
{
	std::vector< const CPlayer* > PlayerList;
	TraversalPlayers((TRAVERSALFUNC)GetAllPlayerObjectEx, (LPARAM)&PlayerList);
	if (PlayerList.empty())
	{
		lua_pushnumber(L, 0);
		return 1;
	}
	int nIndex = 1;
	lua_createtable(L, 0, 0);
	for (std::vector<const CPlayer*>::iterator iter = PlayerList.begin(); iter != PlayerList.end(); iter++)
	{
		const CPlayer* constPlayer = *iter;
		if (constPlayer)
		{
			CPlayer* pPlayer = (CPlayer*)constPlayer;
			SetTableArrayNumber(L, nIndex, pPlayer->GetGID());
			nIndex++;
		}
	}
	return 1;
}


int CScriptManager::L_GetAllPlayerSid(lua_State *L)
{
	std::vector< const CPlayer* > PlayerList;
	TraversalPlayers((TRAVERSALFUNC)GetAllPlayerObjectEx, (LPARAM)&PlayerList);
	if (PlayerList.empty())
	{
		lua_pushnumber(L, 0);
		return 1;
	}
	int nIndex = 1;
	lua_createtable(L, 0, 0);
	for (std::vector<const CPlayer*>::iterator iter = PlayerList.begin(); iter != PlayerList.end(); iter++)
	{
		const CPlayer* constPlayer = *iter;
		if (constPlayer)
		{
			CPlayer* pPlayer = (CPlayer*)constPlayer;
			SetTableArrayNumber(L, nIndex, pPlayer->GetSID());
			nIndex++;
		}
	}
	return 1;
}



int CScriptManager::L_SendTreasureResult(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L,1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
	{
		SATreasureMsg streasuremsg;
		streasuremsg.bTreasureIndex = static_cast<BYTE>(lua_tonumber(L, 2));
		streasuremsg.bResule = static_cast<BYTE>(lua_tonumber(L, 3));
		streasuremsg.wTreasureNum = static_cast<WORD>(lua_tonumber(L, 4));
		g_StoreMessage(pPlayer->m_ClientIndex, &streasuremsg, sizeof(SATreasureMsg));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SavePlayerToDB(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L,1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	pPlayer->SendData(SARefreshPlayerMsg::BACKUP_AND_SAVE);
	return 1;
}


int CScriptManager::L_GetServerID(lua_State *L)
{
	lua_pushnumber(L, g_dServerID);
	return 1;
}

int CScriptManager::L_SendCreateFactionResult(lua_State *L)
{
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		CPlayer *pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
		SACreateFaction sCreateFaction;
		sCreateFaction.bresult = static_cast<BYTE>(lua_tonumber(L, 2));
		g_StoreMessage(pPlayer->m_ClientIndex, &sCreateFaction, sizeof(SACreateFaction));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendFactioninfo(lua_State *L)
{
	CPlayer *pPlayer = nullptr;
	if (lua_isnumber(L, 1) )
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	if (lua_istable(L, 2))
	{
		SAFactioninfo  sfinfo;
		GetTableStringValue(L, 2, "FactionName", sfinfo.szFactionName);
		GetTableStringValue(L, 2, "CreatePlayerName", sfinfo.szCreatorName);
		GetTableNumber(L, 2, "FactionIcon", sfinfo.FactionIcon);
		GetTableNumber(L, 2, "FactionLevel", sfinfo.byFactionLevel);
		GetTableNumber(L, 2, "MemberNum", sfinfo.CurMemberNum);
		GetTableNumber(L, 2, "FactionID", sfinfo.FactionID);
		GetTableNumber(L, 2, "ActiveValue", sfinfo.dActiveValue);
		GetTableNumber(L, 2, "Exp", sfinfo.dfactionexp);
		GetTableNumber(L, 2, "Title", sfinfo.Title);
		if (lua_istable(L, 3))
		{
			size_t nlenth = luaL_getn(L, 3);
			size_t	istart = 0;
			size_t   isend = nlenth > MAX_FACTION_NUMBER ? MAX_FACTION_NUMBER : nlenth;
			sfinfo.CurMemberNum = isend;
			sfinfo.Title = 0;
			size_t	i = 0;
			while (true)
			{
				for (istart; istart < isend; istart++)
				{
					lua_rawgeti(L, 3, istart + 1);
					if (lua_istable(L, -1))
					{
						GetTableStringValue(L, -1, "PlayerName", sfinfo.m_factionmember[i].strMemberName);
						GetTableNumber(L, -1, "PlayerLevle", sfinfo.m_factionmember[i].wLevel);
						GetTableNumber(L, -1, "wFightPower", sfinfo.m_factionmember[i].wFightPower);
						GetTableNumber(L, -1, "wHeadIcon", sfinfo.m_factionmember[i].wPlayerIcon);
						GetTableNumber(L, -1, "isonline", sfinfo.m_factionmember[i].bisonline);
						GetTableNumber(L, -1, "Title", sfinfo.m_factionmember[i].Title);
						GetTableNumber(L, -1, "PlayerActiveValue", sfinfo.m_factionmember[i].wWeekActiveValue);
					}
					lua_pop(L, 1);
					i ++ ;
				}
				g_StoreMessage(pPlayer->m_ClientIndex, &sfinfo, sizeof(SAFactioninfo));
				if (nlenth > isend)
				{
					i = 0;
					istart = isend;
					WORD sendcount = (nlenth - isend) > MAX_FACTION_NUMBER ? MAX_FACTION_NUMBER : nlenth - isend;
					isend += sendcount;
					sfinfo.CurMemberNum = sendcount;
					sfinfo.Title = 1;
				}
				else
				{
					break;
				}
			}
		}
	}
	return 1;
}

int CScriptManager::L_SendFactionList(lua_State *L)
{
	CPlayer *pPlayer = nullptr;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	if (lua_istable(L, 2))
	{
		SAFactionList sFactionList;
		memset(sFactionList.m_factionlist, 0, MAX_FACTION_NUMBER * sizeof(FactionInfoList));

		size_t nlenth = luaL_getn(L, 2);
		for (int i = 0; i < nlenth; i++)
		{
			lua_rawgeti(L, 2, i + 1);
			if (lua_istable(L, -1))
			{
				GetTableStringValue(L, -1, "FactionName", sFactionList.m_factionlist[i].szFactionName);
				GetTableStringValue(L, -1, "CreatePlayerName", sFactionList.m_factionlist[i].szCreatorName);
				GetTableNumber(L, -1, "FactionLevel", sFactionList.m_factionlist[i].byFactionLevel);
				GetTableNumber(L, -1, "MemberNum", sFactionList.m_factionlist[i].CurMemberNum);
				GetTableNumber(L, -1, "FactionIcon", sFactionList.m_factionlist[i].FactionIcon);
				GetTableNumber(L, -1, "FactionID", sFactionList.m_factionlist[i].FactionID);
			}
			lua_pop(L, 1);
			if (i + 1 >= MAX_FACTION_NUMBER )
			{
				rfalse(2, 1, "SendFactionList to long");
				break;
			}
		}
		g_StoreMessage(pPlayer->m_ClientIndex, &sFactionList, sizeof(SAFactionList));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendJoinResult(lua_State *L)
{
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		CPlayer *pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
		SAJoinFaction sJoinFaction;
		sJoinFaction.bresult = static_cast<BYTE>(lua_tonumber(L, 2));
		g_StoreMessage(pPlayer->m_ClientIndex, &sJoinFaction, sizeof(SAJoinFaction));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendManagerFaction(lua_State *L)
{
	CPlayer *pPlayer = nullptr;
	BYTE  bManagerType = 0;
	SAManagerFaction sManagfaction;
	BYTE  *bbuffer = sManagfaction.Buffer;
	memset(bbuffer, 0, MAX_JoinListCount * sizeof(FactionMember));
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		sManagfaction.bresult = static_cast<BYTE>(lua_tonumber(L, 2));
		bManagerType = static_cast<BYTE>(lua_tonumber(L, 3));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	if (bManagerType == 0)
	{
		if (sManagfaction.bresult)
		{
			if (lua_istable(L, 4))
			{
				size_t nlenth = luaL_getn(L, 4);
				sManagfaction.bMembernum = nlenth;
				for (int i = 0; i < nlenth; i++)
				{
					lua_rawgeti(L, 4, i + 1);
					if (lua_istable(L, -1))
					{
						FactionMember fmember;
						GetTableStringValue(L, -1, "PlayerName", fmember.strMemberName);
						GetTableNumber(L, -1, "PlayerLevle", fmember.wLevel);
						GetTableNumber(L, -1, "wFightPower", fmember.wFightPower);
						GetTableNumber(L, -1, "wHeadIcon", fmember.wPlayerIcon);
						GetTableNumber(L, -1, "Title", fmember.Title);
						memcpy(bbuffer, &fmember, sizeof(FactionMember));
						bbuffer += sizeof(FactionMember);
					}
					lua_pop(L, 1);
					if (bbuffer - sManagfaction.Buffer > MAX_JoinListCount * sizeof(FactionMember))
					{
						rfalse(2, 1, "SendManagerFaction Member To long");
						break;
					}
				}
			}
		}
	}
	else if (bManagerType == 5) //查看已经申请的军团
	{
		if (sManagfaction.bresult)
		{
			if (lua_istable(L, 4))
			{
				size_t nlenth = luaL_getn(L, 4);
				sManagfaction.bMembernum = nlenth;
				for (int i = 0; i < nlenth; i++)
				{
					lua_rawgeti(L, 4, i + 1);
					if (lua_istable(L, -1))
					{
						DWORD  dfactionid = 0;
						GetTableArrayNumber(L, -1,1, dfactionid);
						memcpy(bbuffer, &dfactionid, sizeof(DWORD));
						bbuffer += sizeof(DWORD);

						GetTableArrayNumber(L, -1,  2, dfactionid);
						memcpy(bbuffer, &dfactionid, sizeof(DWORD));
						bbuffer += sizeof(DWORD);
						
					}
					lua_pop(L, 1);

					if (bbuffer - sManagfaction.Buffer > MAX_JoinListCount * sizeof(FactionMember))
					{
						rfalse(2, 1, "SendManagerFaction Member To long");
						break;
					}
				}
			}
		}
	}
	g_StoreMessage(pPlayer->m_ClientIndex, &sManagfaction, sizeof(SAManagerFaction));

	return 0;
}

int CScriptManager::L_SDeleteFaction(lua_State *L)
{
	if (lua_isnumber(L,1) && lua_isnumber(L,2))
	{
		SQDeleteFaction  sqdeletefaction;
		sqdeletefaction.nServerId = static_cast<DWORD>(lua_tonumber(L, 1));
		sqdeletefaction.dFactionID= static_cast<DWORD>(lua_tonumber(L, 2));
		SendToLoginServer(&sqdeletefaction, sizeof(SQDeleteFaction));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendSynFaction(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	SASynFaction  ssynfaction;
	if (lua_isnumber(L, 1) && lua_istable(L, 2))
	{   
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
		if (!pPlayer)
		{
			return 0;
		}

		GetTableNumber(L, 2, "FactionID", ssynfaction.dFactionID);
		GetTableNumber(L, 2, "JoinID", ssynfaction.dJoinID);
		GetTableNumber(L, 2, "JoinState", ssynfaction.bJoinState); 
		GetTableNumber(L, 2, "FactionMoney", ssynfaction.dFactionMoney);
		g_StoreMessage(pPlayer->m_ClientIndex, &ssynfaction, sizeof(SASynFaction));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendFactionSceneList(lua_State *L)
{
	CPlayer *pPlayer = nullptr;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	SAFactionSceneData  sFactionSceneData;
	memset(sFactionSceneData.m_factionscene, 0, sizeof(factionscene)*MAX_SENDFACTIONS);
	if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_istable(L, 4))
	{
		sFactionSceneData.wactivevalue = static_cast<DWORD>(lua_tonumber(L,2));
		sFactionSceneData.wturnonnum = static_cast<DWORD>(lua_tonumber(L, 3));
		size_t nlenth = luaL_getn(L, 4);
		for (int i = 0; i < nlenth; i++)
		{
			if (i + 1 > MAX_SENDFACTIONS)
			{
				rfalse(2, 1, "MAX FACTIONSCENE");
				break;
			}
			lua_rawgeti(L, 4, i + 1);
			if (lua_istable(L, -1))
			{
				GetTableNumber(L, -1, "sceneID", sFactionSceneData.m_factionscene[i].dsceneID);
				GetTableNumber(L, -1, "stauts", sFactionSceneData.m_factionscene[i].bscenestatus);
				GetTableNumber(L, -1, "scenemapcount", sFactionSceneData.m_factionscene[i].wTurnOnNum);
			}
			lua_pop(L, 1);
		}
		g_StoreMessage(pPlayer->m_ClientIndex, &sFactionSceneData, sizeof(SAFactionSceneData));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendSceneInfo(lua_State *L)
{
	CPlayer *pPlayer = nullptr;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L, 2) && lua_isnumber(L, 3) &&lua_istable(L,4))
	{
		SAFactionSceneMapData sFactionMapData;
		memset(sFactionMapData.m_factionscenemap, 0, sizeof(factionscenemap)*MAX_SENDFACTIONS);
		sFactionMapData.bChallengeCount = static_cast<BYTE>(lua_tonumber(L, 2));
		sFactionMapData.dsceneID = static_cast<DWORD>(lua_tonumber(L, 3));
		size_t nlenth = luaL_getn(L, 4);
		for (int i = 0; i < nlenth; i++)
		{
			if (i + 1 > MAX_SENDFACTIONS)
			{
				rfalse(2, 1, "MAX FACTIONSCENE");
				break;
			}
			lua_rawgeti(L, 4, i + 1);
			if (lua_istable(L, -1))
			{
				GetTableNumber(L, -1, "scenemapindex", sFactionMapData.m_factionscenemap[i].bscenemapIndex);
				GetTableNumber(L, -1, "stauts", sFactionMapData.m_factionscenemap[i].bscenestatus);
				GetTableNumber(L, -1, "BossAllHP", sFactionMapData.m_factionscenemap[i].dmonsterHP);
				GetTableNumber(L, -1, "BossHP", sFactionMapData.m_factionscenemap[i].dmonstercurrentHP);
			}
			lua_pop(L, 1);
		}
		g_StoreMessage(pPlayer->m_ClientIndex, &sFactionMapData, sizeof(SAFactionSceneMapData));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendChallengeSceneMsg(lua_State *L)
{
	CPlayer *pPlayer = nullptr;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L, 2) && lua_isnumber(L, 3))
	{
		SAChallengeSceneMsg  sChallengeSceneMsg;
		sChallengeSceneMsg.bresult = static_cast<BYTE>(lua_tonumber(L, 2));
		memset(&sChallengeSceneMsg.monsterdata, 0, sizeof(MonsterData));

		DWORD monsterID = static_cast<DWORD>(lua_tonumber(L, 3));
		if (!CConfigManager::getSingleton()->GetMonsterBaseData(monsterID, sChallengeSceneMsg.monsterdata))
		{
			rfalse(2, 1, "GetMonsterBaseData Faile");
			return 0;
		}
		g_StoreMessage(pPlayer->m_ClientIndex, &sChallengeSceneMsg, sizeof(SAChallengeSceneMsg));
		return 1;
	}
	return 1;
}

int CScriptManager::L_SendMailToPlayer(lua_State *L)
{
	if (!lua_istable(L,1)|| !lua_istable(L,2) )
	{
		return 0;
	}

	BYTE stringtype = 0;
	if (lua_isnumber(L, 3))
	{
		stringtype = static_cast<BYTE>(lua_tonumber(L, 3));
	}

	DWORD  recvsid = 0;     //接收SID
	char  strname[CONST_USERNAME] = {};
	DWORD expiretime = 0;   //有效时间
	MailInfo mailInfo;
	memset(&mailInfo, 0, sizeof(mailInfo));
	GetTableStringValue(L, 1, "senderName", mailInfo.senderName);
	GetTableStringValue(L, 1, "title", mailInfo.title);
	GetTableStringValue(L, 1, "content", mailInfo.content);
	GetTableStringValue(L, 1, "recvname", strname);
	GetTableNumber(L, 1, "expiretime", expiretime);

	if (stringtype == 2) //ansi to utf8
	{
		char* Utf8Str = g_AnsiToUtf8(mailInfo.senderName);
		size_t  uChatLen = strlen(Utf8Str);
		memset(mailInfo.senderName, 0, CONST_USERNAME);
		memcpy_s(mailInfo.senderName, CONST_USERNAME, (LPCSTR)Utf8Str, uChatLen);

		Utf8Str = g_AnsiToUtf8(mailInfo.title);
		uChatLen = strlen(Utf8Str);
		memset(mailInfo.title, 0, CONST_MAILTITLE);
		memcpy_s(mailInfo.title, CONST_MAILTITLE, (LPCSTR)Utf8Str, uChatLen);

		Utf8Str = g_AnsiToUtf8(mailInfo.content);
		uChatLen = strlen(Utf8Str);
		memset(mailInfo.content, 0, CONST_MAILCONTENT);
		memcpy_s(mailInfo.content, CONST_MAILCONTENT, (LPCSTR)Utf8Str, uChatLen);

		if (Utf8Str != NULL)
			delete[] Utf8Str;

	}

	size_t nlenth = luaL_getn(L,2);
	for (int i = 0; i < nlenth; i++)
	{
		if (i + 1 > CONST_MAIL_MAXITEM_NUM)
		{
			rfalse(2, 1, "MAX CONST_MAIL_MAXITEM_NUM");
			break;
		}
		lua_rawgeti(L, 2, i + 1);
		if (lua_istable(L, -1))
		{
			GetTableNumber(L, -1, "itemid", mailInfo.szItemInfo[i].id);
			GetTableNumber(L, -1, "itype", mailInfo.szItemInfo[i].itype);
			GetTableNumber(L, -1, "num", mailInfo.szItemInfo[i].num);
		}
		lua_pop(L, 1);
	}

	if (std::strlen(strname) > 0)
	{
		MailMoudle::getSingleton()->SendMail_SanGuoByName(strname, expiretime, &mailInfo);
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendFactionSceneRank(lua_State *L)
{
	CPlayer *pPlayer = nullptr;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	SASceneHurtRank  sscenerank;
	memset(sscenerank.m_factionrank, 0, sizeof(factionscenerank)*MAX_FACTION_NUMBER);
	if (lua_istable(L,2))
	{
		size_t nlenth = luaL_getn(L, 2);
		sscenerank.wRankNum = nlenth;
		for (int i = 0; i < nlenth; i++)
		{
			lua_rawgeti(L, 2, i + 1);
			if (lua_istable(L, -1))
			{
				GetTableStringValue(L, -1, "MemberName", sscenerank.m_factionrank[i].playername);
				GetTableNumber(L, -1, "MemberHurt", sscenerank.m_factionrank[i].dHurt);
				GetTableNumber(L, -1, "RewardNum", sscenerank.m_factionrank[i].drewardnum);
			}
			lua_pop(L, 1);

			if (i + 1 >= MAX_FACTION_NUMBER)
			{
				rfalse(2, 1, "SendInitPrayer Rank List to long");
				break;
			}
		}

		if (lua_istable(L, 3))
		{
			GetTableNumber(L, 3, "Rank", sscenerank.wPlayerRank);
			GetTableNumber(L, 3, "RewardNum", sscenerank.dRewardNum);
			GetTableNumber(L, 3, "NextRank", sscenerank.bNextRange);
			GetTableNumber(L, 3, "HurtValue", sscenerank.dselfhurt);
		}

		g_StoreMessage(pPlayer->m_ClientIndex, &sscenerank, sizeof(SASceneHurtRank));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendFactionSalaryData(lua_State *L)
{
	CPlayer *pPlayer = nullptr;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}  
	SAInitFactionSalary   sfactionsalary;
	memset(sfactionsalary.m_fsalarydata, 0, sizeof(factionsalarydata) * 2);
	if (lua_istable(L,2))
	{
		size_t nlenth = luaL_getn(L, 2);
		for (int i = 0; i < nlenth; i++)
		{
			lua_rawgeti(L, 2, i + 1);
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "itemlist");
				if (lua_istable(L,-1))
				{
					size_t nitemlenth = luaL_getn(L, -1);
					for (int j = 0; j < nitemlenth; j+=3)
					{
						if (j + 1 > MAX_SENDFACTIONS * 3)
						{
							rfalse(2, 1, "send to long");
							break;
						}
						GetTableArrayNumber(L, -1, j + 1, sfactionsalary.m_fsalarydata[i].sitemdata[j].bitemtype);
						GetTableArrayNumber(L, -1, j + 2, sfactionsalary.m_fsalarydata[i].sitemdata[j].ditemIndex);
						GetTableArrayNumber(L, -1, j + 3, sfactionsalary.m_fsalarydata[i].sitemdata[j].ditemnum);
					}
				}
				lua_pop(L, 1);
				GetTableNumber(L, -1, "ActivityValue", sfactionsalary.m_fsalarydata[i].dActivityValue);
				GetTableNumber(L, -1, "FactionExp", sfactionsalary.m_fsalarydata[i].dExp);
				GetTableNumber(L, -1, "CostGold", sfactionsalary.m_fsalarydata[i].wcostgold);
			}
			lua_pop(L, 1);
		}

		if (lua_isnumber(L,3))
		{
			sfactionsalary.bIsGot = static_cast<BYTE>(lua_tonumber(L, 3));
		}
		g_StoreMessage(pPlayer->m_ClientIndex, &sfactionsalary, sizeof(SAInitFactionSalary));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendGetSalaryResult(lua_State *L)
{
	CPlayer *pPlayer = nullptr;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	SAGetFactionSalary   sgetfactionsalary;
	if (lua_isnumber(L, 2))
	{
		sgetfactionsalary.bresult = static_cast<BYTE>(lua_tonumber(L, 2));
		g_StoreMessage(pPlayer->m_ClientIndex, &sgetfactionsalary, sizeof(SAGetFactionSalary));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SaveAllFactionData(lua_State *L)
{
	SQSaveFactionDataMsg  sSaveFactionMsg;
	sSaveFactionMsg.nServerID = g_dServerID;
	WORD  nFactions = 0;
	tm timeMaker;
	sSaveFactionMsg.nTime = mktime(&timeMaker);
	sSaveFactionMsg.bSaveMode = TRUE;
	BYTE  *pfactiondatastream = (BYTE*)sSaveFactionMsg.streamFaction;
	memset(pfactiondatastream, 0, sizeof(sSaveFactionMsg.streamFaction));
	if (lua_istable(L, 1))
	{
		size_t  toplenth = luaL_getn(L, 1);
		if (toplenth == 0)
		{
			rfalse(2, 1, "L_SaveAllFactioninfo ERROR");
			return 0;
		}
		size_t	istart = 0;
		size_t   isend = toplenth > MAX_SENDFACTIONDATA ? MAX_SENDFACTIONDATA : toplenth;
		bool bsendover = false;
		while (true)
		{
			SaveFactionData_Lua FactionData;
			for (istart; istart < isend; istart++)
			{
				lua_rawgeti(L, 1, istart + 1);
				if (lua_istable(L, -1))
				{
					memset(&FactionData, 0, sizeof(SaveFactionData_Lua));
					size_t  nlenth = luaL_getn(L, -1);
					lua_rawgeti(L, -1, 1);
					if (lua_istable(L, -1))
					{
						GetTableStringValue(L, -1, "FactionName", FactionData.szFactionName);
						GetTableNumber(L, -1, "FactionID", FactionData.FactionID);
					}
					lua_pop(L, 1);

					//lua_getfield(L, -1, "RequestList"); 
					lua_rawgeti(L, -1, 2);
					if (lua_istable(L, -1))
					{
						int cur1 = luaEx_serialize(L, -1, FactionData.bFactionData, FACTIONDATA_LENTH);
						if (cur1 < 0)
						{
							rfalse(2, 1, " Faction Data size was to long  %d ",cur1);
							break;
						}
						FactionData.dfactionlenth = cur1;
					}
					lua_pop(L, 1);

				}
				memcpy(pfactiondatastream, &FactionData, sizeof(SaveFactionData_Lua));
				pfactiondatastream += sizeof(SaveFactionData_Lua);
				nFactions++;
			}
			sSaveFactionMsg.nFactions = nFactions;
			long sendsize = sizeof(SQSaveFactionDataMsg);
			SendToLoginServer(&sSaveFactionMsg, sendsize);
			//SectionMessageManager::getInstance().sendSectionMessageTologin(&sSaveFactionMsg, sendsize);
			if (toplenth > isend)
			{
				istart = isend;
				WORD sendcount = (toplenth - isend) > MAX_SENDFACTIONDATA ? MAX_SENDFACTIONDATA : toplenth - isend;
				isend += sendcount;

				nFactions = 0;
				pfactiondatastream = (BYTE*)sSaveFactionMsg.streamFaction;
				memset(pfactiondatastream, 0, sizeof(sSaveFactionMsg.streamFaction));
			}
			else
			{
				break;
			}
		}
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendFactionNotice(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	SAFactionNotice  sfactionnotice;
	memset(sfactionnotice.strnotice, 0, MAX_NOTICE_NUMBER);
	if (lua_isstring(L, 2))
	{
		const char* strnotice = static_cast<const char *>(lua_tostring(L, 2));
		strcpy(sfactionnotice.strnotice, strnotice);
		g_StoreMessage(pPlayer->m_ClientIndex, &sfactionnotice, sizeof(SAFactionNotice));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendModifyNoticeResult(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	SAFModifyNotice sfnotice;
	if (lua_isnumber(L, 2))
	{
		sfnotice.bresult = static_cast<BYTE>(lua_tonumber(L, 2));
		g_StoreMessage(pPlayer->m_ClientIndex, &sfnotice, sizeof(SAFModifyNotice));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendFactionLog(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	SAFactionOperateLog  sOperateLog;
	if (lua_istable(L, 2))
	{
		memset(sOperateLog.factionlog, 0, sizeof(operatelog)*MAX_NOTICE_NUMBER);
		size_t nlenth = luaL_getn(L, 2);
		sOperateLog.blognum = nlenth;
		for (int i = 0; i < nlenth; i++)
		{
			lua_rawgeti(L, 2, i + 1);
			if (lua_istable(L, -1))
			{
				GetTableArrayNumber(L, -1, 1, sOperateLog.factionlog[i].btype);
				GetTableArrayString(L, -1, 2, sOperateLog.factionlog[i].strName);
				GetTableArrayNumber(L, -1, 3, sOperateLog.factionlog[i].dParamID);
			}
			lua_pop(L, 1);
		}
		g_StoreMessage(pPlayer->m_ClientIndex, &sOperateLog, sizeof(SAFactionOperateLog));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendFcEmailResult(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	SAFcEmailToAll  sfemailall;
	if (lua_isnumber(L, 2))
	{
		sfemailall.bresult = static_cast<BYTE>(lua_tonumber(L, 2));
		g_StoreMessage(pPlayer->m_ClientIndex, &sfemailall, sizeof(SAFcEmailToAll));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendEquiptList(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	SAShowEquipt   sshowequipt;
	memset(sshowequipt.Buffer, 0, MAX_QUESTEQUIPTBUFFER);
	BYTE   *pBuffer = sshowequipt.Buffer;
	if (lua_isnumber(L, 2) && lua_istable(L,3))
	{
		sshowequipt.dSelectedEquipt = static_cast<DWORD>(lua_tonumber(L, 2));
		sshowequipt.blevelnum = luaL_getn(L, 3);
		for (int i = 0; i < sshowequipt.blevelnum; i++)
		{
			lua_rawgeti(L, 3, i + 1);
			if (lua_istable(L, -1))
			{
				DWORD  dequiptid = 0;
				size_t nlenth = luaL_getn(L, -1);
				if (pBuffer - sshowequipt.Buffer  >= MAX_QUESTEQUIPTBUFFER)
				{
					rfalse(2, 1, "too long for equiptbuffer");
					break;
				}		
				memcpy(pBuffer, &nlenth, sizeof(size_t));
				pBuffer += sizeof(DWORD);

				for (int j = 0; j < nlenth; j++)
				{
					GetTableArrayNumber(L, -1, j + 1, dequiptid);
					memcpy(pBuffer, &dequiptid, sizeof(DWORD));
					pBuffer += sizeof(DWORD);
				}
			}
			lua_pop(L, 1);
		}
		g_StoreMessage(pPlayer->m_ClientIndex, &sshowequipt, sizeof(SAShowEquipt));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendRqEquiptResult(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	size_t nlenth = luaL_getn(L, 1);
	SARequestEquipt  sRequestEquipt;
	sRequestEquipt.dParam = 0;
	if (lua_isnumber(L, 2))
	{
		sRequestEquipt.bresult = static_cast<BYTE>(lua_tonumber(L, 2));
		if (nlenth > 2 && lua_isnumber(L,3))
		{
			sRequestEquipt.dParam = static_cast<DWORD>(lua_tonumber(L, 3));
		}
		g_StoreMessage(pPlayer->m_ClientIndex, &sRequestEquipt, sizeof(SARequestEquipt));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendRequestStatus(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	SARequestStatus   sRequestStatus;
	memset(sRequestStatus.mquestlist, 0, sizeof(requestlist)*MAX_EQUIPTLISTNUM);
	if (lua_isnumber(L, 2) && lua_istable(L, 3))
	{
		sRequestStatus.wEquiptNum = static_cast<WORD>(lua_tonumber(L, 2));
		size_t nlenth = luaL_getn(L, 3);
		for (int i = 0; i < nlenth; i++)
		{
			lua_rawgeti(L, 3, i + 1);
			if (lua_istable(L, -1))
			{
				GetTableArrayNumber(L, -1, 1, sRequestStatus.mquestlist[i].wRank);
				GetTableArrayNumber(L, -1, 2, sRequestStatus.mquestlist[i].wIconIndex);
				GetTableArrayNumber(L, -1, 3, sRequestStatus.mquestlist[i].wLevle);
				GetTableArrayString(L, -1, 4, sRequestStatus.mquestlist[i].strname);
			}
			lua_pop(L, 1);
		}
		g_StoreMessage(pPlayer->m_ClientIndex, &sRequestStatus, sizeof(SARequestStatus));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendCanceledResult(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L, 2))
	{
		SACanceledQuest  scanceledquest;
		scanceledquest.bresult = static_cast<BYTE>(lua_tonumber(L, 2));
		g_StoreMessage(pPlayer->m_ClientIndex, &scanceledquest, sizeof(SACanceledQuest));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendInitPrayer(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	SAInitPrayerMsg  sinitprayermsg;
	memset(sinitprayermsg.pPlayerPoint, 0, sizeof(PrayerPoint) * RANK_LIST_MAX);
	if (lua_istable(L,2))
	{
		size_t nlenth  = luaL_getn(L, 2);
		for (int i = 0; i < nlenth; i++)
		{
			lua_rawgeti(L, 2, i+1);
			if (lua_istable(L, -1))
			{
				GetTableStringValue(L, -1, "PlayerName", sinitprayermsg.pPlayerPoint[i].playername);
				GetTableNumber(L, -1, "point", sinitprayermsg.pPlayerPoint[i].wPoint);
			}
			lua_pop(L, 1);

			if (i + 1 >= RANK_LIST_MAX)
			{
				rfalse(2, 1, "SendInitPrayer Rank List to long");
				break;
			}
		}

		if (lua_istable(L, 3))
		{
			GetTableNumber(L, 3, "wCurrentPoint", sinitprayermsg.wCurrentPoint);
			GetTableNumber(L, 3, "wHistoryPoint", sinitprayermsg.wHistoryPoint);
			GetTableNumber(L, 3, "wRemainDay", sinitprayermsg.wRemainDay);
			GetTableNumber(L, 3, "dfreetime", sinitprayermsg.dfreetime);
			GetTableNumber(L, 3, "dclosetime", sinitprayermsg.dclosetime);
			GetTableNumber(L, 3, "bShopOpened", sinitprayermsg.bShopOpened);
			g_StoreMessage(pPlayer->m_ClientIndex, &sinitprayermsg, sizeof(SAInitPrayerMsg));
			return 1;
		}
	}
	return 0;
}

int CScriptManager::L_SendPrayerResult(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	SAPrayerMsg  pPrayerMsg;
	memset(pPrayerMsg.bBuffer, 0, bufflen);
	BYTE *pPrayerBuff = pPrayerMsg.bBuffer;
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPrayerMsg.bresult = static_cast<BYTE>(lua_tonumber(L, 2));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}

	BYTE  btype, bnum = 0;
	DWORD ditemindex = 0;
	if (lua_istable(L, 3))
	{
		size_t nlenth = luaL_getn(L, 3);
		pPrayerMsg.wItemNum = nlenth;
		for (int i = 0; i < nlenth; i++)
		{
			if (pPrayerBuff - pPrayerMsg.bBuffer >= bufflen)
			{
				rfalse(2, 1, "SendPrayerResult  buff  lenth to long");
				break;
			}

			lua_rawgeti(L, 3, i + 1);
			if (lua_istable(L, -1))
			{
				GetTableNumber(L, -1, "itype", btype);
				memcpy(pPrayerBuff, &btype, sizeof(BYTE));
				pPrayerBuff += sizeof(BYTE);
				GetTableNumber(L, -1, "inum", bnum);
				memcpy(pPrayerBuff, &bnum, sizeof(BYTE));
				pPrayerBuff += sizeof(BYTE);
				GetTableNumber(L, -1, "iindex", ditemindex);
				memcpy(pPrayerBuff, &ditemindex, sizeof(DWORD));
				pPrayerBuff += sizeof(DWORD);
			}
			lua_pop(L, 1);
		}
	}
	g_StoreMessage(pPlayer->m_ClientIndex, &pPrayerMsg, sizeof(SAPrayerMsg));
	return 1;
}

int CScriptManager::L_SendShopItemList(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	SAShopRefresh   sshoprefresh;
	memset(sshoprefresh.m_PrayerItem, 0, sizeof(PrayerShopItem) * MAX_PRAYER_SHOPITEM);
	if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
	{
		sshoprefresh.dRefreshTime = static_cast<DWORD>(lua_tonumber(L, 2));
		sshoprefresh.bmoneyType = static_cast<BYTE>(lua_tonumber(L, 3));
		sshoprefresh.bRefreshPoint = static_cast<WORD>(lua_tonumber(L, 4));

		if (lua_istable(L,5))
		{
			size_t nlenth = luaL_getn(L, 5);
			for (int i = 0; i < nlenth; i++)
			{
				if (i + 1 > MAX_PRAYER_SHOPITEM)
				{
					rfalse(2, 1, "L_SendShopItemList To long");
					break;
				}
				lua_rawgeti(L, 5, i + 1);
				if (lua_istable(L, -1))
				{
					GetTableArrayNumber(L, -1,  1, sshoprefresh.m_PrayerItem[i].ditemIndex);
					GetTableArrayNumber(L, -1,  2, sshoprefresh.m_PrayerItem[i].bnum);
					GetTableArrayNumber(L, -1, 3, sshoprefresh.m_PrayerItem[i].ditemprice);
				}
				lua_pop(L, 1);
			}
			g_StoreMessage(pPlayer->m_ClientIndex, &sshoprefresh, sizeof(SAShopRefresh));
		}

	}
	return 0;
}

int CScriptManager::L_SendShopBuyResult(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	SAShopBusiness  sashopresult;
	if (lua_isnumber(L, 2))
	{
		sashopresult.bresult = static_cast<BYTE>(lua_tonumber(L,2));
		g_StoreMessage(pPlayer->m_ClientIndex, &sashopresult, sizeof(SAShopBusiness));
	}
	return 0;
}

int CScriptManager::L_SendActivityCodeResult(lua_State *L)
{
	CPlayer *pPlayer = NULL;
	if (lua_isnumber(L, 1))
	{
		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
		pPlayer = (CPlayer*)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer)
	{
		return 0;
	}
	if (lua_isnumber(L, 2) && lua_istable(L, 3))
	{
		SAGiftcodeActivity  sgiftcode;
		memset(sgiftcode.bBuffer, 0, bufflen);
		BYTE  *pgiftcode = sgiftcode.bBuffer;

		size_t nlenth = luaL_getn(L, 3);
		sgiftcode.wItemNum = nlenth;
		sgiftcode.bresult = static_cast<BYTE>(lua_tonumber(L, 2));
		BYTE  btype = 0;
		DWORD dnum = 0;
		DWORD ditemindex = 0;
		for (int i = 0; i < nlenth; i++)
		{
			if (pgiftcode - sgiftcode.bBuffer >= bufflen)
			{
				rfalse(2, 1, "SendActivityCodeResult  buff  lenth to long");
				break;
			}

			lua_rawgeti(L, 3, i + 1);
			if (lua_istable(L, -1))
			{
				GetTableNumber(L, -1, "itype", btype);
				memcpy(pgiftcode, &btype, sizeof(BYTE));
				pgiftcode += sizeof(BYTE);
				GetTableNumber(L, -1, "inum", dnum);
				memcpy(pgiftcode, &dnum, sizeof(DWORD));
				pgiftcode += sizeof(DWORD);
				GetTableNumber(L, -1, "iindex", ditemindex);
				memcpy(pgiftcode, &ditemindex, sizeof(DWORD));
				pgiftcode += sizeof(DWORD);
			}
			lua_pop(L, 1);
		}
		g_StoreMessage(pPlayer->m_ClientIndex, &sgiftcode, sizeof(SAGiftcodeActivity));
		return 1;
	}
	return 0;
}

int CScriptManager::L_SendScriptData(lua_State *L)
{
	
	if (lua_istable(L,1))
	{
		BYTE *luabuffer = new BYTE[102400];
		memset(luabuffer, 0, 102400);
		int cur1 = luaEx_serialize(L, 1, luabuffer, 102400);
		if (cur1 < 0)return 0;

		WORD  datalenth = sizeof(SQScriptData) + cur1;

		WORD wcount = datalenth / sizeof(SQScriptData) + 1;
		SQScriptData   *pscriptdata = new SQScriptData[wcount];
		memset(&pscriptdata->wLenth, 0, sizeof(SQScriptData)*(wcount - 1));

		pscriptdata->wLenth = cur1;
		pscriptdata->pBuffer = (BYTE*)&pscriptdata->wLenth + sizeof(WORD);
		memcpy(pscriptdata->pBuffer, luabuffer, cur1);
		SendToLoginServer(pscriptdata, sizeof(SQScriptData)*wcount);
		//SectionMessageManager::getInstance().sendSectionMessageTologin(pscriptdata, sizeof(SQScriptData)*wcount);

		SAFE_DELETE_ARRAY(pscriptdata);
		pscriptdata = nullptr;
		return 1;   

// 		SQScriptData   sqscriptdata;
// 		sqscriptdata.wLenth = cur1;
// 		sqscriptdata.pBuffer = (BYTE*)(&sqscriptdata + sizeof(SQScriptData));
// 
// 
// 		memset(sqscriptdata.pBuffer, 0, cur1);
// 		memcpy(sqscriptdata.pBuffer, luabuffer, cur1);
// 		SendToLoginServer(&sqscriptdata, datalenth);
// 		return 1;
// 		int copylenth = (int)&sqscriptdata.wLenth - (int)&sqscriptdata;
// 		if (copylenth >0)
// 		{
// 			memcpy(pscriptdata, &sqscriptdata, copylenth);
// 			pscriptdata->wLenth = cur1;
// 			pscriptdata->pBuffer = (BYTE*)&pscriptdata->wLenth + sizeof(WORD);
// 			memcpy(pscriptdata->pBuffer, luabuffer, cur1);
// 			SendToLoginServer(pscriptdata, datalenth);
// 			return 1;
// 		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
