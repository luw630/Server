#include "StdAfx.h"

#include "playermanager.h"
#include "player.h"
#include "region.h"
#include "NetWorkModule\MoveMsgs.h"
#include "NetWorkModule\UpgradeMsgs.h"
#include "NetWorkModule\ScriptMsgs.h"
#include "Networkmodule\ItemMsgs.h"
#include "Networkmodule\RefreshMsgs.h"
#include "Networkmodule\ChatMsgs.h"
#include "networkmodule\TimeSynMsgs.h"
#include "gaminghouse.h"
#include "ScriptManager.h"
#include "GameWorld.h"

#include "liteserializer\variant.h"

#include "GlobalFunctions.h"
#include "DSkillStudyTable.h"
#include "RoleLevelUpTbl.h"
#include "CPlayerService.h"
#include "FightObject.h"
#include "networkmodule\regionmsgs.h"
#include "RankList.h"
#include "memory_object.h"
#include <time.h>
#include "pet.h"

#define MAX_CHECKLIVETIME		    ( 1000 * 10 * 60)			// 1分钟  //20150527 版本是10分钟,玩家线路出现问题10分钟后就会让其主动下线,后面需要恢复1分钟等待重连
//#define MAX_CHECKLIVETIME		    ( 1000 *5)			// 1分钟  //20150527 版本是10分钟,玩家线路出现问题10分钟后就会让其主动下线,后面需要恢复1分钟等待重连

extern BOOL SetDnidContext(DNID dnidClient, LPVOID pContext);
extern void SendMessageToPlayer(LPCSTR szName, SMessage *pMsg, WORD wSize);
extern LPCSTR _GetStringTime();
extern BOOL TraceInfo(LPCSTR szFileName, LPCSTR szFormat, ...);
extern CGameWorld *&GetGW();
extern LPIObject GetObjectByGID(DWORD GID);
extern int cacheEnabled;
extern int backupNum;

// extern CKillInfoManager g_KillInfoManager;
// CDistributedSimplePlayerManager g_DistributedSimplePlayerManager;

// 唯一的引用对象，因为static的对象是隐含于其它不相关文件的，所以它是唯一的
static CPlayerManager *s_pUniqueThis = NULL; 
extern LINKSTATUS g_GetLogStatus(DNID dnidClient);

Cmemory_object *g_pmemoryobject = NULL;


extern BOOL g_bUsePackageMessage;
extern BYTE commBuffer[ 0xfff ];
extern CScriptManager g_Script;

BOOL  CALLBACK ChecklivePlayer( LPIObject &Player, DWORD &number );
extern int CALLBACK GetAllPlayerObjectEx(LPIObject& pObject, std::vector< const CPlayer* >* pPlayerVec);
// ############################ 新的角色缓存功能部分

struct RebuildSence
{
    BOOL operator () ( LPCSTR key, SFixProperty &player )
    {
        number[0] ++;

        // OutputDebugString( FormatString( "account = %s still alive!", key ) );

        // 恢复角色时,取出GID
        DWORD GID = player.m_dwPowerVal;

        // 先要判断新建目标是否有重复
        LPIObject pObj = s_pUniqueThis->LocateObject( GID );
        assert( !pObj );
        if ( pObj )
            return rfalse( 2, 1, "恢复角色时,发现一个重复的玩家! [ GID = %d ]", GID );

        extern LPIObject GetPlayerByAccount( LPCSTR szAccount );
        pObj = GetPlayerByAccount( key );
        assert( !pObj );
        if ( pObj )
            return rfalse( 2, 1, "恢复角色时,发现一个重复的玩家! [ account = %s ]", key );

	    CPlayer::SParameter Param;
	    Param.dnidClient = 0;
        Param.dwVersion = 1;
        pObj = s_pUniqueThis->GenerateObject( IID_PLAYER, GID, ( LPARAM )&Param );
	    assert( pObj );
        if ( !pObj )
            return rfalse( 2, 1, "恢复角色时,创建玩家对象失败! [ account = %s ]", key );

        CPlayer *pPlayer = ( CPlayer* )pObj->DynamicCast( IID_PLAYER );
        if ( pPlayer == NULL )
            return rfalse( 2, 1, "绝对不可能!!! [ account = %s ]", key );

        pPlayer->SetAccount( key );
        pPlayer->m_ClientIndex = 0;

        // 在查询映射表内保存对象
        BOOL BindAccountRelation( LPCSTR szAccount, LPIObject pObj );
        if ( !BindAccountRelation( key, pObj ) )
            return rfalse( 2, 1, "恢复角色时,为玩家对象绑定帐号失败! [ account = %s ]", key );

        pPlayer->m_Property.m_Name[0] = 0;

        LPIObject GetPlayerByName( LPCSTR szName );
        if ( GetPlayerByName( player.m_Name )->DynamicCast( IID_PLAYER ) != NULL )
            return rfalse( 2, 1, "恢复角色时,发现了一个重复的角色名! [ account = %s, name = %d ]", key, player.m_Name );

        if ( !pPlayer->SetFixProperty( &player ) ) 
            return rfalse( 2, 1, "恢复角色时,设置角色属性失败! [ account = %s, name = %d ]", key, player.m_Name );

        //if (!pPlayer->SetTempData(pTempData)) 
        //    return FALSE;

		pPlayer->LoginChecks();

        LPIObject FindRegionByID( DWORD ID );
        CRegion *pRegion = ( CRegion* )FindRegionByID( pPlayer->m_Property.m_CurRegionID )->DynamicCast( IID_REGION );
		if (pRegion == NULL)
			return 0;
		//20150424 wk 去除打印
           // return rfalse( 2, 1, "恢复角色时,设置角色场景失败! [ account = %s, name = %d, regionId = %d ]", key, player.m_Name, pPlayer->m_Property.m_CurRegionID );

	    if ( pRegion->AddObject( pObj ) == 0 )
            return rfalse( 2, 1, "恢复角色时,将角色放入场景失败! [ account = %s, name = %d, regionId = %d, x = %d, y = %d ]", key, player.m_Name, pPlayer->m_Property.m_CurRegionID, ((DWORD)pPlayer->m_Property.m_X) >> 32, ((DWORD)pPlayer->m_Property.m_Y) >> 32 );

        pPlayer->m_OnlineState = CPlayer::OST_HANGUP;
        pPlayer->m_bInit = true;

        number[1] ++;
        return TRUE;
    }

    int number[2];
};

void RemoveFromCache( LPCSTR account )
{
    // 这个地方不考虑 cacheEnabled 标志的影响，能删就删
    if (s_pUniqueThis && s_pUniqueThis->static_playercache )
        s_pUniqueThis->static_playercache->Erase( account );
}

void InsertIntoCache(LPCSTR account, CPlayer *player)
{
	if (!player || !player->m_bInit)
        return;
	MY_ASSERT(player->m_bInit);
// 	if (g_pmemoryobject)
// 	{
// 		g_pmemoryobject->BackupPlayer(player);
// 	}

//     extern int cacheEnabled;
//     if (cacheEnabled &&	s_pUniqueThis	&& s_pUniqueThis->static_playercache)
//     {
//         SFixProperty &slotData = (*s_pUniqueThis->static_playercache)[player->GetAccount()];
//         player->Backup(slotData);
//     }
}

void CPlayerManager::RescueCachePlayers()
{
	if (!cacheEnabled)return;
	g_pmemoryobject = new Cmemory_object;
	//g_pmemoryobject->CreateShareMemory("playerback", sizeof(CPlayer)* 50);
	g_pmemoryobject->CreateShareMemory("playerback", sizeof(SBackupPlayer)* backupNum);

//     if ( !cacheEnabled )
//         return;
// 
//     try {
//         static_playercache = new ObjectCache< SFixProperty, 500 >( __argv[1] );
//     } catch ( exception &e ) {
//         delete static_playercache;
//         static_playercache = NULL;
//         rfalse( 2, 1, "初始化共享缓存失败 info = %s\r\n", e.what() );
//         return;
//     }
// 
//     if ( static_playercache->isNewly() )
//         return;
// 
//     if ( !cacheEnabled )
//     {
//         rfalse( 2, 1, "########################################################\r\n"
//             "检测到共享缓存在区域启动前已经有效存在，是不是之前区域出错了呢？\r\n"
//             "因为没有开启共享缓存备份功能，所以并未作任何恢复操作！\r\n"
//             "%d 个角色被清除！\r\n"
//             "########################################################\r\n", static_playercache->Size() );
// 
//         // 清空缓存
//         static_playercache->Clear();
//         return;
//     }
//     
//     RebuildSence tcb;
//     tcb.number[1] = tcb.number[0] = 0;
// 
//     static_playercache->Traverse( tcb );
// 
//     rfalse( 2, 1, "########################################################\r\n"
//         "在上一场灾难后[%d/%d]个角色被恢复！\r\n"
//         "########################################################\r\n", tcb.number[1], tcb.number[0] );
}

static BOOL CALLBACK BackupPlayer( LPIObject &Player, DWORD &number )
{
    CPlayer *player = (CPlayer*)Player->DynamicCast( IID_PLAYER );
    if ( ( player == NULL ) || ( !player->m_bInit ) )
        return TRUE;

	if (g_pmemoryobject)
	{
		/*g_pmemoryobject->BackupPlayer(player);*/
//		player->SaveData();
		// 20150117 wk 恢复缓存时传入&player->m_FixData******begin***************************************
		//g_pmemoryobject->BackupPlayer(&player->m_Property,player->GetAccount(),player->GetGID());

		g_pmemoryobject->BackupPlayer(&player->m_FixData, player->GetAccount(), player->GetGID());
		// 20150117 wk 恢复缓存时传入&player->m_FixData******end***************************************
		number ++;
	}
//     SFixProperty &slotData = ( *s_pUniqueThis->static_playercache )[ player->GetAccount() ];
//     player->Backup( slotData );
   
    return TRUE;
}

static BOOL CALLBACK GetPlayer(LPIObject &Player, DWORD &number)
{
	CPlayer *player = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if ((player == NULL) || (!player->m_bInit))
		return TRUE;

}

void BackupPlayerIntoCache()
{
    DWORD number = 0;
    //unsigned __int64 GetCpuCycle();
	unsigned __int64 seg = GetTickCount64();
    if (s_pUniqueThis && s_pUniqueThis->static_playercache )
        s_pUniqueThis->TraversalObjects( ( TRAVERSALFUNC )BackupPlayer, ( LPARAM )&number );
	unsigned __int64 end = GetTickCount64();
    rfalse( 2, 1, "%d个角色被备份，消耗%I64d毫秒！", number, end - seg );
}

// ############################ end of 新的角色缓存功能部分

CPlayerManager::CPlayerManager() :
    CObjectManager(),
    static_playercache( NULL )
{
    extern char ORBCADDR[256];

    s_pUniqueThis = this;

}


//add by ly 2014/4/14 增加玩家的一些相关附加功能（如：背包格子倒计时时间操作、16分钟恢复玩家1点的体力等）
void CPlayerManager::PlayerAttachAction()
{	
	map<DWORD, DWORD>::iterator it = m_PlayerSIDMap.begin();
	for (; it != m_PlayerSIDMap.end(); it++)
	{
		CPlayer * lpPlayer = (CPlayer*)::GetPlayerByGID(it->second)->DynamicCast(IID_PLAYER);
		if (lpPlayer != NULL)
		{
			INT64 TimeNow = 0;
			_time64(&TimeNow);

			int PlayerOnlineTime = (lpPlayer->m_Property.m_OnlineTime + (TimeNow - lpPlayer->m_dwLoginTime));

			g_Script.SetCondition(NULL, lpPlayer, NULL, NULL);
			//处理玩家的背包倒计时相关操作
			//在线时间满足背包格子激活处理
			LuaFunctor(g_Script, FormatString("ActivePackageOpt"))[BaseActivedPackage + lpPlayer->m_Property.m_wExtGoodsActivedNum + 1][PlayerOnlineTime][0][1]();
			g_Script.CleanCondition();

			//玩家每隔16分钟恢复玩家1点的体力（不管玩家是否已经在线或者不在线，时间都会走动）
			//计算离线期间所需要增加的TP值
			int HuiFuTimeInterval = 5 * 60;		//默认16分钟
			lite::Variant ret;
			LuaFunctor(g_Script, "ReturnTpTimeInterval")(&ret);
			if (lite::Variant::VT_INTEGER == ret.dataType)
				HuiFuTimeInterval = ret;

			INT64 PreNotOnlineTime = 0;
			if (lpPlayer->m_Property.m_dLeaveTime64 != 0)
				PreNotOnlineTime = lpPlayer->m_dwLoginTime - lpPlayer->m_Property.m_dLeaveTime64;	//上一次不在线的时间长度
			INT64 NeedAddTime = (lpPlayer->m_Property.m_LeaveTime + lpPlayer->m_Property.m_OnlineTime - PreNotOnlineTime) % HuiFuTimeInterval + PreNotOnlineTime;	//上一次不在线的时间长度 加上 没有计算在16分钟的时间
			DWORD NeedAddTp = NeedAddTime / HuiFuTimeInterval;
			if (lpPlayer->m_AddOnceTpFlag)
			{
				if (lpPlayer->m_Property.m_CurTp + NeedAddTp < lpPlayer->m_MaxTp)
					lpPlayer->m_CurTp = lpPlayer->m_Property.m_CurTp = lpPlayer->m_Property.m_CurTp + NeedAddTp;
				else
					lpPlayer->m_CurTp = lpPlayer->m_Property.m_CurTp = lpPlayer->m_MaxTp;
				lpPlayer->m_AddOnceTpFlag = FALSE;
				lpPlayer->m_FightPropertyStatus[XA_CUR_TP] = true;
			}
			//计算登陆后到现在玩家需要增加的值
			INT64 PlayerGameTime = NeedAddTime % HuiFuTimeInterval + (TimeNow - lpPlayer->m_dwLoginTime);
			if (lpPlayer->m_Property.m_CurTp < lpPlayer->m_MaxTp  && PlayerGameTime % HuiFuTimeInterval == 0 &&	lpPlayer->m_SecondOnceAddTp != PlayerGameTime / HuiFuTimeInterval)
			{
				lpPlayer->m_CurTp++;
				lpPlayer->m_Property.m_CurTp++;
				lpPlayer->m_FightPropertyStatus[XA_CUR_TP] = true;
				lpPlayer->m_SecondOnceAddTp = PlayerGameTime / HuiFuTimeInterval;
			}
		}
	}
}

int g_SendDirect(DNID dnidClient, LPVOID pPackage, WORD wSize);

struct SBroadcastStruct
{
    LPVOID pBuffer;
    WORD wSize;
};

static BOOL CALLBACK BroadcastPackageCallback(LPIObject &Player, SBroadcastStruct *pSBMS)
{
    if (CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER))
        g_SendDirect(pPlayer->m_ClientIndex, pSBMS->pBuffer, pSBMS->wSize);

    return TRUE;
}

static BOOL CALLBACK BroadcastMessageCallback(LPIObject &Player, SBroadcastStruct *pSBMS)
{
    if (CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER))
	{
		g_StoreMessage(pPlayer->m_ClientIndex, pSBMS->pBuffer, pSBMS->wSize);
	}
    return TRUE;
}

static BOOL CALLBACK TalkToAllMessageCallback(LPIObject &Player, char *str)
{
	if (CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER))
		TalkToDnid(pPlayer->m_ClientIndex,str,0);
		//g_StoreMessage(pPlayer->m_ClientIndex, pSBMS->pBuffer, pSBMS->wSize);

	return TRUE;
}

bool CPlayerManager::Run()
{
    if (!this)
        return false;

	// 执行日常时间管理器
	//OnEveryDayManagerRun();

	LPVOID pPackage = 0;
	WORD wSize = 0;

	if (CSingleRank::GetInstance().CanProcessTheRankList())
		CSingleRank::GetInstance().ProcessTheRankList();

 	INT64 curTime;
 	_time64(&curTime);

	if (CSingleRank::GetInstance().CheckTime(curTime))
	{
		// 更新排行榜
		std::vector< const CPlayer* > PlayerList;
		TraversalPlayers((TRAVERSALFUNC)GetAllPlayerObjectEx, (LPARAM)&PlayerList);

		// 非空，请求排行榜数据
		if (!PlayerList.empty())
		{
			CSingleRank::GetInstance().GetRankData_From_ZoneServer(PlayerList);
			CSingleRank::GetInstance().SendToDB_2_GetRankData((*(PlayerList.begin()))->m_Property.m_Name);
			CSingleRank::GetInstance().SetRankCondition(true);
		}
	}
	
    // 定时备份！
	extern int backupMargin, cacheEnabled, backupNum;
	static unsigned __int64 prevTick = GetTickCount64();
//     if ( cacheEnabled && static_playercache && ( abs( ( int )( timeGetTime() - prevTick ) ) > backupMargin ) )
//     {
//         DWORD number = 0;
//         TraversalObjects( ( TRAVERSALFUNC )BackupPlayer, ( LPARAM )&number );
//         prevTick = timeGetTime();
//     }

	if (cacheEnabled && abs((int)(GetTickCount64() - prevTick)) > backupMargin)
	{
		DWORD number = 0;
		unsigned __int64 seg = GetTickCount64();
		TraversalObjects((TRAVERSALFUNC)BackupPlayer, (LPARAM)&number);
		//unsigned __int64 end22 = GetTickCount64();
		if (g_pmemoryobject)
		{
			g_pmemoryobject->Onrefreshmemory();  //刷新数据到内存
		}
		unsigned __int64 end = GetTickCount64();
		//rfalse(2, 1, "备份前遍历玩家完成-----------，消耗%I64d毫秒！", end22 - seg);
		rfalse(2, 1, "%d个角色被备份，消耗%I64d毫秒！", number, end - seg);
		
		prevTick = GetTickCount64();
	}
	
	TraversalObjects((TRAVERSALFUNC)ChecklivePlayer,0);

    return CObjectManager::Run();
}

void CPlayerManager::Reconnect()
{
    //g_KillInfoManager.m_OrbUser.Reconnect2Center();
    //g_DistributedSimplePlayerManager.m_OrbUser.Reconnect2Center();
}

IObject* CPlayerManager::CreateObject(const __int32 IID)
{
	if (IID == IID_PLAYER)
		return new CPlayer();

	rfalse(4, 1, "CPlayerManager::CreateObjec  fail   !!!!!!!!!!!");
    return NULL;
}

BOOL CPlayerManager::DispatchTimeMessage(DNID dnidClient, struct STimeBaseMsg *pMsg, CPlayer *pPlayer)
{
	if (pMsg == NULL || dnidClient == 0 || pPlayer == NULL)
		return FALSE;

	// 进行逻辑时间校正
	switch (pMsg->_protocol)
	{
	case STimeBaseMsg::EPRO_TIMESYN_SYNTIME:
		{
			SASynTimeMsg msg;
			msg.m_serverTime = timeGetTime();
			msg.m_clientTime = ((SQSynTimeMsg *)pMsg)->m_clientTime;
			g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
			break;
		}

	case STimeBaseMsg::EPRO_TIMESYN_CHECKTIME:
		{
			SQCheckTimeMsg *checkMsg = (SQCheckTimeMsg *)pMsg;
			if (!checkMsg)
			{
				rfalse(4,1,"STimeBaseMsg::EPRO_TIMESYN_CHECKTIME");
				return FALSE;
			}
			// 如果客户端和服务器端的逻辑时间的误差很小，那么当数据到达时，
			// 服务器的时间应该是早于客户端的逻辑时间的，但是如果反之，则
			// 说明网络状况发生了变化或者使用了外挂，那么强制重新校对时钟
			if (checkMsg->m_serverTime > timeGetTime() && checkMsg->m_phase != 0)
			{
				SASynTimeMsg msg;
				msg.m_serverTime = timeGetTime();
				msg.m_clientTime = checkMsg->m_clientTime;
				g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
			}
			else
			{
				SACheckTimeMsg msg;
				msg.m_delay = timeGetTime() - checkMsg->m_serverTime;
				g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
			}
		}
		break;

	default:
		return FALSE;
	}
	
	return TRUE;
}

BOOL CPlayerManager::DispatchMoveMessage(DNID dnidClient, struct SMoveBaseMsg *pMsg, CPlayer *pPlayer)
{
	if (!pMsg || 0 == dnidClient || !pPlayer)
		return FALSE;

	if (!pPlayer->m_ParentRegion || !pPlayer->m_ParentArea)
		return FALSE;

	if (0 == pPlayer->m_CurHp)
		return FALSE;

	if (pPlayer->IsInStall())		// 摆摊中
		return FALSE;

	/*
    if (pPlayer->m_bWarehouseOpened || pPlayer->InExchange())
        return FALSE;

    // 因为网络消息到达，停止当前后备的攻击信息
    pPlayer->bMsgComein = false;

    // 做指令数量检测，检测连点器
    pPlayer->clickLimitChecker.Increment();
    if (!pPlayer->clickLimitChecker.isValid())
    {
        return FALSE;
    }//*/

	switch (pMsg->_protocol)
	{
	case SMoveBaseMsg::EPRO_SYN_PATH:
		pPlayer->OnRecvMoveMsg((SQSynPathMsg *)pMsg);
		break;

	case SMoveBaseMsg::EPRO_SYN_JUMP:
		pPlayer->OnRecvJumpMsg((SAQSynJumpMsg *)pMsg);
		break;

	case SMoveBaseMsg::EPRO_SYN_UNITY3DJUMP:
		pPlayer->OnRecvUnity3DJumpMsg((SAQUnity3DJumpMsg *)pMsg);
		break;

	case SMoveBaseMsg::EPRO_SYN_POS:
		pPlayer->OnRecvSynPosMsg((SQSynPosMsg *)pMsg);
		break;

	case SMoveBaseMsg::EPRO_SET_ZAZEN:
        pPlayer->OnRecvZazenMsg((SQSetZazenMsg*)pMsg);
		break;

	case SMoveBaseMsg::EPRO_TASK_MOVE:
		pPlayer->OnRecvTaskMovemsg((SQtaskmove*)pMsg);
		break;

	case SMoveBaseMsg::EPRO_SYN_PATHMONSTER:
	{
		SQSynPathMonsterMsg *pMoveMsg = (SQSynPathMonsterMsg*)pMsg;
		if (pMoveMsg)
		{
			CMonster *pMonster = (CMonster*)GetObjectByGID(pMoveMsg->dwGlobalID)->DynamicCast(IID_MONSTER);
			if (pMonster)
			{
				pMonster->OnRecvPath(pMoveMsg);
			}
		}
	}
		break;

	}

	return TRUE;
}

BOOL CPlayerManager::DispatchFightMessage(DNID dnidClient, struct SFightBaseMsg *pMsg, CPlayer *pPlayer)
{
	if (!pMsg || 0 == dnidClient || !pPlayer) 
		return FALSE;

	if (0 == pPlayer->m_CurHp)
		return FALSE;



	// 做指令数量检测，检测连点器
	pPlayer->clickLimitChecker.Increment();
	if (!pPlayer->clickLimitChecker.isValid())
		return FALSE;

	switch (pMsg->_protocol) 
    {
	case SFightBaseMsg::EPRO_FIGHT_OBJECT:
	case SFightBaseMsg::EPRO_FIGHT_POSITION:
	case SFightBaseMsg::EPRO_QUEST_SKILL:
	case SFightBaseMsg::EPRO_UPDATE_SKILL_BOX:
	case SFightBaseMsg::EPRO_FORCE_QUEST_ENEMY_INFO:
	case SFightBaseMsg::EPRO_QUEST_SKILL_FP:
	case SFightBaseMsg::EPRO_SELECT_TARGET:
	case SFightBaseMsg::EPRO_PRACTICE_UPDATEBOX:
	case SFightBaseMsg::EPRO_QUEST_MULTIPLESKILL:
	{
		SQuestSkillMultiple_C2S_MsgBody *pfightMsg = (SQuestSkillMultiple_C2S_MsgBody*)pMsg;
		if (pfightMsg)
		{
			CFightPet *pfight = (CFightPet*)GetObjectByGID(pfightMsg->mAttackerGID)->DynamicCast(IID_FIGHT_PET);
			if (pfight)
			{
				pfight->SetAttackMsg(pMsg);
				return TRUE;
			}
		}
		pPlayer->SetAttackMsg(pMsg);
	}
		break;
	case SFightBaseMsg::EPRO_WIND_MOVE_CHECK:
	{
		pPlayer->OnRecvCheckMove((SQWinMoveCheckMsg*)pMsg);
	}
		break;

	case SFightBaseMsg::EPRO_CURE_VENATION:
		break;

	case SFightBaseMsg::EPRO_DELETE_TELERGY:
		break;

	case SFightBaseMsg::EPRO_SET_CURTELERGY:
		break;

	case SFightBaseMsg::EPRO_SET_CURSKILL:
        break;

	case SFightBaseMsg::EPRO_LEARN_SKILL:
		break;
	
	case SFightBaseMsg::EPRO_TELERGY_STATE:
		break;

	case SFightBaseMsg::EPRO_UPDATEBUFF_INFO:
		break;

	case SFightBaseMsg::EPRO_PRACTICE_REQUEST:
		break;

	case SFightBaseMsg::EPRO_MONSTER_BOSSUPDATE:
		switch (((SQBossDeadUpdatemsg*)pMsg)->bType)
		{
		case SQBossDeadUpdatemsg::UPDATE_BOSSMSG:
				pPlayer->OnRecvQuestUpdateBossMsg((SQBossDeadUpdatemsg*)pMsg);
			break;
		case SQBossDeadUpdatemsg::UPDATE_ACTIVITYMSG:
				pPlayer->OnRecvQuestActivityNotice((SQBossDeadUpdatemsg*)pMsg);
			break;
		case SQBossDeadUpdatemsg::UPDATE_NEWSMSG:
				break;
		}
		
		break;

		//add by ly 2014/3/17  玩家升星请求消息
	case SFightBaseMsg::EPRO_XINYANG_SHENGXING:
		pPlayer->OnHandlePlayerRiseStar((SQXinYangRiseStarmsg*)pMsg);
		break;

    }

    return TRUE;
}

void CPlayerManager::OnEveryDayManagerRun()
{
	int nIndex = g_Script.OnEveryDayManagerRun();
	if(nIndex == 0) return;

	std::vector<const CPlayer*> PlayerList;
	TraversalPlayers((TRAVERSALFUNC)GetAllPlayerObjectEx, (LPARAM)&PlayerList);
	if(PlayerList.empty()) return;

	std::vector<const CPlayer*>::iterator it;
	for(it=PlayerList.begin(); it!=PlayerList.end(); it++)
	{
		const CPlayer* clpPlayer = *it;
		CPlayer* lpPlayer = (CPlayer*)clpPlayer;
		lpPlayer->OnEveryDayManagerRun(nIndex);
	}
}

bool g_CheckPlayerCanIStudyIt(const CSkillStudyTable::SSkillCondition& rStudyCondition, const CPlayer* pPlayer,int iMaxCount )
{
// 	if ( !pPlayer )
// 		return false;
// 
// 	if ( pPlayer->m_Property.m_Level < rStudyCondition.nRequireLevel)
// 		return false;
// 
// 	if ( pPlayer->m_Property.m_Money < rStudyCondition.nMoneySpend )
// 		return false;
// 
// 	for(int i=0; i<3; ++i)
// 	{
// 		int nTaskId = rStudyCondition.nRequireTaskId[i];
// 		if ( nTaskId>0 )
// 		{
// 			lite::Variant _result;
// 
// 			LuaFunctor( g_Script, "IsTaskCompleted")[pPlayer->GetSID()][nTaskId](&_result );
// 			int ok = (int)_result;
// 			if( ok == 0 )
// 				return false;
// 		}		
// 	}
// 
// 	for ( int i=0; i<4; ++i)
// 	{
// 		int nSkillId = rStudyCondition.pairRequireSkill[i].first;
// 		int nSkillLevel = rStudyCondition.pairRequireSkill[i].second;
// 
// 		if ( nSkillId> 0)
// 		{
// 			bool bFound = false;
// 			for ( int j = 0; j<iMaxCount; ++j)
// 			{
// 				if ( GET_SKILL_TYPE(pPlayer->m_Property.m_pSkills[j].wTypeID )==rStudyCondition.nType 
// 					&& pPlayer->m_Property.m_pSkills[j].wTypeID == nSkillId 
// 					&& pPlayer->m_Property.m_pSkills[j].byLevel>=nSkillLevel )
// 				{
// 					bFound = true;
// 				}
// 			}
// 
// 			if ( !bFound )
// 				return false;
// 		}
// 	}

	return true;
}

BOOL CPlayerManager::DispatchScriptMessage(DNID dnidClient, struct SScriptBaseMsg *pMsg, CPlayer *pPlayer)
{
	if (!pMsg || 0 == dnidClient || !pPlayer) 
        return FALSE;
	//20150305 注释,萌将三国没有地图  ---begin
	//if (!pPlayer->m_ParentRegion || !pPlayer->m_ParentArea)
	//	return FALSE;

    //if (0 == pPlayer->m_CurHp) 
     //   return FALSE;
	//20150305 注释,萌将三国没有地图  ---end

    // 因为网络消息到达，停止当前后备的攻击信息
//    pPlayer->bMsgComein = false;

	switch (pMsg->_protocol)
    {
	case SScriptBaseMsg::EPRO_ROLETASK_INFO:
		// 任务相关
		pPlayer->OnTaskMsg((SRoleTaskBaseMsg*)pMsg);
		break;

    case SScriptBaseMsg::EPRO_CLICK_OBJECT:
		// 客户端点击Npc，服务器做出的处理
        pPlayer->StoreClickMsg((SQClickObjectMsg*)pMsg);
		break;

	case SScriptBaseMsg::EPRO_CLICK_MENU:
		// 服务器根据客户端做出的选择做出的相应处理
        pPlayer->OnClickMenu((SQChoseMenuMsg *)pMsg);
		break;

	case SScriptBaseMsg::EPRO_PLAY_CG:
		pPlayer->OnPlayCG((SQPlayCG *)pMsg);
		break;

// 	case SScriptBaseMsg::EPRO_FORGE_GOODS:
// 		pPlayer->OnClickForgeMenu((SQForgeGoodsMsg *)pMsg);
// 		break;
// 
//     case SScriptBaseMsg::EPRO_SEND_KILLINFO:
// 		pPlayer->RecvKillInfoMessage((SQSendKillInfoMsg *)pMsg);
// 		break;
// 
//     case SScriptBaseMsg::EPRO_SEND_ITEMINFO:
// 		pPlayer->RecvSRecvItemInfo((SQSendSItemInfoMsg *)pMsg);
// 		break;
// 
//     case SScriptBaseMsg::EPRO_SCORE_LIST:
// 		pPlayer->RecvOpenScoreList( (SQUpdateScoreListMsg *) pMsg);
//         break;
// 
//     case SScriptBaseMsg::EPRO_CUSTOM_WND:
// 		pPlayer->RecvCustomWndResult( (SACustomWndMsg *) pMsg);
//         break;
// 
//     case SScriptBaseMsg::EPRO_SCRIPT_TRIGGER:
// 		pPlayer->RecvScriptTrigger( (SQScriptTriggerMsg *) pMsg);
//         break;
// 
     case SScriptBaseMsg::EPRO_LUACUSTOM_MSG:
 		pPlayer->RecvLuaCustomMsg( (SQALuaCustomMsg *) pMsg);
//         break;
//     case SScriptBaseMsg::EPRO_SCORE_LIST_EX:
// 		pPlayer->RecvGetNewXYDScoresInf( (SQUpdateScoreListExMsg *) pMsg );
//         break;
     case SScriptBaseMsg::EPRO_OTHEREQUIPMENTNAME:
         if ( GetGW() && ( (SQOthereQuipmentNameMsg *)pMsg)->szDestName[0] != '\0' )
             GetGW()->LookOtherPlayerEquipment(pPlayer, (CPlayer*)GetPlayerByName( ((SQOthereQuipmentNameMsg *)pMsg)->szDestName )->DynamicCast(IID_PLAYER), false);
         break;

	 case SScriptBaseMsg::EPRO_RANK_GETOTHEREQUIPMENT:
		 // 排行榜查看玩家信息
		 if ( GetGW() && ( (SQRankListEquipMsg *)pMsg)->szDestName[0] != '\0' )
			 GetGW()->LookRankListEquipInfo(pPlayer, (CPlayer*)GetPlayerByName( ((SQRankListEquipMsg*)pMsg)->szDestName )->DynamicCast(IID_PLAYER));
		 break;

	 case SScriptBaseMsg::EPRO_CLIENT_REQUEST_DB:
		 // 客户端请求排行榜数据，服务器收到这个消息，就要从DB拉数据了...
		pPlayer->QuestDB2GetRankList((SQClientQuestRankFromDB *)pMsg);
		 break;
	 case SScriptBaseMsg::EPRO_RANK_LIST:
		 pPlayer->GetRankListInfo((SQRankListMsg*)pMsg);
		 break;
	 case SScriptBaseMsg::EPRO_QUEST_TREASURE:
		 pPlayer->OnTreasureMsg((SQTreasureMsg*)pMsg);
		 break;
	 case SScriptBaseMsg::EPRO_INIT_PRAYER:
		 pPlayer->OnInitPrayerMsg((SQInitPrayerMsg*)pMsg);
		 break;
	 case SScriptBaseMsg::EPRO_START_PRAYER:
		 pPlayer->OnStartPrayerMsg((SQPrayerMsg*)pMsg);
		 break;
	 case SScriptBaseMsg::EPRO_SHOP_REFRESH:
		 pPlayer->OnShopRefresh((SQShopRefresh*)pMsg);
		 break;
	 case SScriptBaseMsg::EPRO_SHOP_BUSINESS:
		 pPlayer->OnShopBusiness((SQShopBusiness*)pMsg);
		 break;
	 case SScriptBaseMsg::EPRO_GIFTCODE_ACTIVITY:
		 pPlayer->OnGetGiftCode((SQGiftcodeActivity*)pMsg);
		 break;
    }

    return TRUE;
}

BOOL CPlayerManager::DispatchItemMessage(DNID dnidClient, struct SItemBaseMsg *pMsg, CPlayer *pPlayer)
{
	if (!pMsg || 0 == dnidClient || !pPlayer || 0 == pPlayer->m_CurHp) 
        return FALSE;

	if (!pPlayer->m_ParentRegion || !pPlayer->m_ParentArea)
		return FALSE;

//     if(_theGHouseManager->Locate(pPlayer->GetSportsRoomID()))
//     {
//         if(pMsg->_protocol == SItemBaseMsg::EPRO_EXCHANGE_MESSAGE)
//             return FALSE;
//     }

    switch (pMsg->_protocol)
    {
    case SItemBaseMsg::EPRO_PICKUP_ITEM:
        pPlayer->RecvPickupItem((SQPickupItemMsg*)pMsg);
        break;

    case SItemBaseMsg::EPRO_DROP_ITEM:
        pPlayer->RecvDropItem((SQDropItemMsg*)pMsg);
        break;

	case SItemBaseMsg::EPRO_UNLOAD_EQUIP:
		pPlayer->RecvUnEquipItem((SQUnEquipItemMsg *)pMsg);
		break;

    case SItemBaseMsg::EPRO_USE_ITEM:
        pPlayer->RecvUseItem((SQUseItemMsg*)pMsg);
        break;

	case SItemBaseMsg::EPRO_MOVE_ITEM:
		pPlayer->RecvMoveItem((SQMoveItemMsg*)pMsg);
		break;
        
	case SItemBaseMsg::EPRO_SPLIT_ITEM:
		pPlayer->RecvSplitItem(( QuerySplitItemMsg* )pMsg);
		break;

	case SItemBaseMsg::EPRO_BUY_MESSAGE:
		pPlayer->RecvBuyItem((SBuy *)pMsg);
		break;

	case SItemBaseMsg::EPRO_FOLLOWITEM_QuestData:
		pPlayer->RecvQuestFollowItem( (SQQuestFollowItemMsg*)pMsg );
		break;

	case SItemBaseMsg::EPRO_BUYITEM_FOLLOW:
		pPlayer->RecvBuyFollowItem( (SQBUYFollowItemMsg*)pMsg);
		break;

	case SItemBaseMsg::EPRO_Bless_ReQuest:
		pPlayer->RecvBlessBlessReQuest(	(SQBlessItemReQuestMsg*)pMsg);
		break;

	case SItemBaseMsg::EPRO_Bless_ExcellenceData:
		pPlayer->RecvBlessExcellenceData((SQBlessExcellenceDataMsg*)pMsg);
		break;

	case SItemBaseMsg::EPRO_Bless_QuestAddItems:
		pPlayer->RecvBlessQuestAddItems((SQBlessQuestAddItems*)pMsg);
		break;

	case SItemBaseMsg::EPRO_ChannelCallSell_INFO:
		pPlayer->RecvChannelCallSellInfoQuest((SQChannelCallSellInfo*)pMsg);
		break;

	case SItemBaseMsg::EPRO_NEATEN:
		pPlayer->ArrangePackage(0);
		break;

	case SItemBaseMsg::EPRO_SALEDITEM_LIST:
		pPlayer->RecvOpenBuyBackList();
		break;

	case SItemBaseMsg::EPRO_CHECKGROUND_ITEM:
		pPlayer->RecvCheckGroundItem((struct SQCheckGroundItemMsg *)pMsg);
		break;

	case SItemBaseMsg::EPRO_CLOSEGROUND_ITEM:
		pPlayer->RecvCloseGroundItem();
		break;

	case SItemBaseMsg::EPRO_GROUNDITEM_CHANCEBACK:
		pPlayer->RecvStartTakeChance((SQGroundItemChanceBack *)pMsg);
		break;

	case SItemBaseMsg::EPRO_BUY_SALEDITEM:
		pPlayer->BuyBackSaledItem((struct SQBuySaledItem *)pMsg);
		break;

	case SItemBaseMsg::EPRO_EXCHANGE_MESSAGE:
        pPlayer->CExchangeGoods::OnDispatchMsg((SExchangeBaseMsg *)pMsg);
		break;

    case SItemBaseMsg::RPRO_WAREHOUSE_BASE:
        pPlayer->CWareHouse::OnDispatchMsg((SWareHouseBaseMsg *)pMsg);
        break;

    case SItemBaseMsg::RPRO_SALEITEM_MESSAGE:
        pPlayer->CStall::DispatchSaleMessage((SSaleItemBaseMsg *)pMsg);
        break;

    case SItemBaseMsg::EPRO_AUTOUSE_ITEM:
        //pPlayer->RecvAutoUseItem((SQAutoUseItemMsg*)pMsg);
        break;

    case SItemBaseMsg::EPRO_AUTOFIGHT:
        //pPlayer->isAutoFight = ( ( SQAutoFightMsg* )pMsg )->flags;
        break;

	case SItemBaseMsg::EPRO_CHANGE_EQUIPCOL:
		//pPlayer->OnRecvEquipColor((SQAEquipColorMsg *)pMsg);
		break;

    case SItemBaseMsg::EPRO_UPDATE_ITEMINFO:
        //pPlayer->RecvQueryUpdateItemData( ( QueryUpdateItemDataMsg* )pMsg );
        break;
	case SItemBaseMsg::EPRO_ACTIVEEXTPACKAGE:
		pPlayer->RecvActiveExtPackage((struct SQActiveExtPackage *)pMsg);
		break;
	case SItemBaseMsg::EPRO_START_FORGE:
		pPlayer->RecvForgeItem((struct SQStartForge *)pMsg);
		break;

	case SItemBaseMsg::EPRO_DRAG_METRIAL:
		pPlayer->RecvDragMetrail((struct SQDragMetrial *)pMsg);
		break;

	case SItemBaseMsg::EPRO_END_FORGE:
		pPlayer->RecvEndForgeItem((struct SQEndForge *)pMsg);
		break;

	case SItemBaseMsg::EPRO_MAKE_HOLE:
		pPlayer->RecvMakeHole((struct SQMakeHole *)pMsg);
		break;

	case SItemBaseMsg::EPRO_INLAY_JEWEL:
		pPlayer->RecvInLayJewel((struct SQInLayJewel *)pMsg);
		break;

	case SItemBaseMsg::EPRO_TAKEOFF_JEWEL:
		pPlayer->RecvTakeoffJewel((struct SQTakeOffJewel *)pMsg);
		break;

    case SItemBaseMsg::EPRO_DIAL:
        {
//             SQDialMsg *pDialMsg = ( SQDialMsg * )pMsg;
//             switch ( pDialMsg->type )
//             {
//             case SQDialMsg::DIALP_START:
//                 {
// #undef new
//                     SADialMsg &msg = *new ( commBuffer )SADialMsg;
//                     size_t size = sizeof( commBuffer );
//                     msg.type = SADialMsg::DIAL_RUN;
//                     try
//                     {
//                         lite::Serializer slm( msg.buffer, size - sizeof( SADialMsg ) );
//                         slm( &pPlayer->m_Property.base, sizeof( pPlayer->m_Property.base ) );
//                         size = ( WORD )size - slm.EndEdition();
//                     }
//                     catch ( lite::Xcpt & )
//                     {
//                     }
// 
//                     pPlayer->SendMsg( &msg, size );
//                 }
//                 break;
//             case SQDialMsg::DIALP_TRY:
//                 {
//                     g_Script.SetCondition( NULL, pPlayer, NULL );
//                     LuaFunctor( g_Script, "OnDialpTry" )[ pDialMsg->byDialType ]();
//                     g_Script.CleanCondition();
//                 }
//                 break;
//             case SQDialMsg::DIALP_RECEIVE:
//                 {
//                     // 开宝箱记录
//                     pPlayer->m_Property.openBoxTimes ++;
// 
//                     g_Script.SetCondition( NULL, pPlayer, NULL );
// 
//                     if ( pPlayer->m_Property.base.type == 1 )
//                     {
//                         SPackageItem item;
//                         memset( &item, 0, sizeof( item ) );
//                         item.byCellX = -1;
//                         item.byCellY = -1;
// 
//                         memcpy( &item.wIndex, &pPlayer->m_Property.unitItem.itemBuffer, sizeof( SRawItemBuffer ) );
//                         LuaFunctor( g_Script, "OnDialpReceive" )[pPlayer->m_Property.base.type][ lite::Variant( &item, sizeof( item ) ) ][ pDialMsg->byDialType ]();
// 
//                         TraceInfo( "LOGS\\开宝箱记录.txt", "%s 账号:%s 角色:%s 道具Id：%d", _GetStringTime(), pPlayer->GetAccount(), pPlayer->GetName(), 
//                             item.wIndex );
//                     }
//                     else if ( pPlayer->m_Property.base.type == 2 )
//                     {
//                         LuaFunctor( g_Script, "OnDialpReceive" )[pPlayer->m_Property.base.type][ pPlayer->m_Property.unitOther.value ]();
//                         TraceInfo( "LOGS\\开宝箱记录.txt", "%s 账号:%s 角色:%s 非道具值：%d", _GetStringTime(), pPlayer->GetAccount(), pPlayer->GetName(),
//                             pPlayer->m_Property.unitOther.value );
//                     }
// 
//                     g_Script.CleanCondition();
// 
//                     memset( &pPlayer->m_Property.base, 0, sizeof( pPlayer->m_Property.base ) );
//                 }
//                 break;
//             }
        }
        break;
	case SItemBaseMsg::EPRO_FIX_ONE_ITEM:
		pPlayer->RecvFixOneItem((struct SQEquipFixOne*)pMsg);
		break;

	case SItemBaseMsg::EPRO_FIX_ALL_ITEM:
		pPlayer->RecvFixAllItem((struct SQEquipFixAll*)pMsg);
		break;

	// ===============装备精炼================
	case SItemBaseMsg::EPRO_ADD_EQUIP:
		pPlayer->RecvDragStrengthenEquip((SQAddEquip*)pMsg);
		break;

	case SItemBaseMsg::EPRO_ADD_METRIAL:
		pPlayer->RecvDragStrengthenMetrial((SQAddMetrial*)pMsg);
		break;

	// 升阶
	case SItemBaseMsg::EPRO_BEGIN_REFINE_GRADE:
		pPlayer->RecvBeginRefineGrade((SQBeginRefineGrade*)pMsg);
		break;

	// 升星
	case SItemBaseMsg::EPRO_BEGIN_REFINE_STAR:
		pPlayer->RecvBeginRefineStar((SQBeginRefineStar*)pMsg);
		break;
	
	case SItemBaseMsg::EPRO_BEGIN_ReMove_STAR:
		//pPlayer->RecvBeginReMoveStar((SQBeginReMoveStar*)pMsg);
		break;

	// 升品质
	case SItemBaseMsg::EPRO_BEGIN_UPGRADE_QUALITY:
		pPlayer->RecvBeginUpgradeQuality((SQStartQuality*)pMsg);
		break;
	// 升级
	case SItemBaseMsg::EPRO_BEGIN_UPGRADE_LEVEL:
		pPlayer->RecvBeginUpgradeLevel((SQBeginEquipLevelUpMsg*)pMsg);
		break;

	case SItemBaseMsg::EPRO_END_STRENGTHEN:
		pPlayer->RecvEndRefine((SQEndRefineMsg*)pMsg);
		break;	
	case SItemBaseMsg::EPRO_SPECIAL_USEITEM:
		pPlayer->RecvSpecialUseItem((SQSpecialUseItemMsg*)pMsg);
		break;
	case SItemBaseMsg::EPRO_UNLOAD_FIGHTPETEQUIP:
			pPlayer->RecvUnfpEquipItem((SQfpUnEquipItemMsg*)pMsg);
			break;
	case SItemBaseMsg::EPRO_FIGHTPET_FIGHTPETTOITEM:
		pPlayer->_fpMakeItem(((SQfpToitemMsg*)pMsg)->index);
		break;

	case SItemBaseMsg::EPRO_BEGIN_RESET_ATTRIBUTE:
		//pPlayer->RecvBeginResetAttribute((SQBeginResetAttribute*) pMsg);
		break;

	case SItemBaseMsg::EPRO_BEGIN_IDENTIFY:
		{
			SQBeginIdentifyMsg *pChildMsg = (SQBeginIdentifyMsg*) pMsg;
			if (!pChildMsg)
			{
				break;
			}
			switch (pChildMsg->type)
			{
			case SQBeginIdentifyMsg::QBIM_XILIAN:
				//pPlayer->RecvBeginPolished((SQBeginIdentifyMsg*) pMsg);
				break;
			case SQBeginIdentifyMsg::QBIM_JINGLIAN:
				//pPlayer->RecvBeginJingLian((SQBeginIdentifyMsg*) pMsg);
				break;
			case SQBeginIdentifyMsg::QBIM_SHENGJIE:
				//pPlayer->RecvBeginShengJie((SQBeginIdentifyMsg*) pMsg);
				break;
			case SQBeginIdentifyMsg::QBIM_FENJIE:
				//pPlayer->RecvBeginDecomposition((SQBeginIdentifyMsg*) pMsg);
				break;
			case SQBeginIdentifyMsg::QBIM_SMELTING:
				//pPlayer->RecvBeginSmelting((SQBeginIdentifyMsg*) pMsg);
				break;
			case SQBeginIdentifyMsg::QBIM_LINGFU:
				//pPlayer->RecvBeginSpiritAttachBody((SQBeginIdentifyMsg*) pMsg);
				break;
			default:
				break;
			}
		}		
		break;	
	case SItemBaseMsg::EPRO_BEGIN_IDENTIFY_EXCHANGE:
		//pPlayer->RecvIdentifyExchange((SQIdentifyExchangeMsg*) pMsg);
		break;

	// 取消洗点
	case SItemBaseMsg::EPRO_END_RESETPOINT:
		pPlayer->RecvEndResetPoint((SQEndResetPoint*) pMsg);
		break;

	case SItemBaseMsg::EPRO__GET_ONLINEGIFT:
		pPlayer->OnRecvGetGiftMsg((SQGetOnlienGiftMsg*)pMsg);
		break;

	case  SItemBaseMsg::EPRO_EQUIP_FASHIONCHANGE:
		pPlayer->OnRecvQuestChangeFashion((SQFashionChangeMsg*)pMsg);
		break;

	// ============道具升级相关======================
	case SItemBaseMsg::EPRO_DRAG_UPDATE_ITEM:
		pPlayer->RecvDragUpdateItem((SQDragUpdateItemMsg*) pMsg);
		break;

	case SItemBaseMsg::EPRO_DRAG_UPDATE_METRIAL:
		pPlayer->RecvDragUpdateMetrial((SQDragUpdateMetrialMsg*) pMsg);
		break;

	case SItemBaseMsg::EPRO_BEGIN_UPDATE_ITEM:
		pPlayer->RecvBeginUpdateItem((SQBeginUpdateItem*) pMsg);
		break;

	case SItemBaseMsg::EPRO_END_UPDATE_ITEM:
		pPlayer->RecvEndUpdateItem((SQEndUpdateItem*) pMsg);
		break;

	case SItemBaseMsg::EPRO_EQUIP_SUIT_ATTRIBUTE:
		pPlayer->RecvGetSuitAttributes((SQSuitAttributeMsg*) pMsg);
		break;
	case SItemBaseMsg::EPRO_INTENSIFY_INFO:
		pPlayer->OnGetIntensifyInfo((SQIntensifyInfo*)pMsg);
		break;
	case SItemBaseMsg::EPRO__EQUIP_SWITCH:
		pPlayer->OnQuestEquipSwitch((SQEquipSwitch*)pMsg);
		break;
	case SItemBaseMsg::EPRO_QUALITY_INFO:
		pPlayer->OnGetQualityInfo((SQEquipQualityInfo*)pMsg);
		break;
	case SItemBaseMsg::EPRO__ITEM_SALE:
		pPlayer->OnRecvSaleItem((SQItemSale*)pMsg);
		break;
	case SItemBaseMsg::EPRO_CLEAN_COLDTIME:
		pPlayer->OnRecvCleanColdTime((SQCleanColdtime*)pMsg);
		break;
	case SItemBaseMsg::EPRO_RISE_INFO:
		pPlayer->OnGetRiseStarInfo((SQRiseStarInfo*)pMsg);
		break;
	case SItemBaseMsg::EPRO_KEYIN_INFO:
		pPlayer->OnGetEquiptKeYinInfo((SQKeYinInfo*)pMsg);
		break;
	case SItemBaseMsg::EPRO_BEGIN_KEYIN:
		pPlayer->OnBeginKeYin((SQBeginKeYin*)pMsg);
		break;
	case SItemBaseMsg::EPRO_BEGIN_KEYINCHANGE:
		pPlayer->OnBeginKeYinChange((SQKeYinChange*)pMsg);
		break;
	case SItemBaseMsg::EPRO_BEGIN_KEYINCHANGEINFO:
		pPlayer->OnGetKeYinChangeInfo((SQKeYinChangeInfo*)pMsg);
		break;
	case SItemBaseMsg::EPRO_BEGIN_JDING:
		pPlayer->OnBeginEquipJDing((SQEquipJDing*)pMsg);
		break;
	case SItemBaseMsg::EPRO_JDING_INFO:
		pPlayer->OnGetEquipJDingInfo((SQEquipJDingInfo*)pMsg);
		break;
	case SItemBaseMsg::EPRO_JDING_COLOR:
		pPlayer->OnGetEquipJDingColor((SQEquipJDingColor*)pMsg);
		break;
	case SItemBaseMsg::EPRO_SUIT_CONDITION:
		pPlayer->OnGetSuitcondition((SQSuitcondition*)pMsg);
		break;
	case SItemBaseMsg::EPRO_EQUIPT_POWER:
		pPlayer->OnGetEquipPower((SQEquipPower*)pMsg);
			break;
		//2014.2.20 add
	case SItemBaseMsg::EPRO_EVERYDAY_ALLQIDAO:
		pPlayer->OnReturnQiDaoMsg((SQQiDaoInfo *)pMsg);
		break;
	case SItemBaseMsg::EPRO_EVERYDAY_ADDQIDAO:
		pPlayer->OnAddQiDaoAndReturn((SQAddQiDaoInfo *)pMsg);
		break;
		//2014.2.21 add
	case SItemBaseMsg::EPRO_GET_YUANBAOGIFTINF:
		pPlayer->OnReturnYuanBaoMsg((SQGiftPacketMsg *)pMsg);
		break;
	case SItemBaseMsg::EPRO_BUY_ALLYUANBAOGIFT:
		pPlayer->OnBueYuanBaoGift((SQBuyGiftPacketMsg *)pMsg);
		break;

	//add by ly 2014/4/14
	case SItemBaseMsg::EPRO_CELL_COUNTDOWN:
		pPlayer->OnReturnCellCountDown((SQCellCountDown *)pMsg);
		break;
	case SItemBaseMsg::EPRO_BUY_PLAYERTP:
		pPlayer->OnBuyPlayerTp((SQBuyPlayerTp*)pMsg);
		break;
		//add by ly 2014/4/10
	//case SItemBaseMsg::EPRO_DELPACKAGE_ITEM:
	//	SQDelPackageItemMsg *lpDelPackageItemMsg = (SQDelPackageItemMsg*)pMsg;
	//	pPlayer->DeleteItem(lpDelPackageItemMsg->ItemID, lpDelPackageItemMsg->DelItemNum);
	//	break;
    } 

    return TRUE;
}

BOOL CPlayerManager::DispatchUpgradeMessage(DNID dnidClient, struct SUpgradeMsg *pMsg, CPlayer *pPlayer)
{
	if (!pMsg || !dnidClient || !pPlayer) 
        return FALSE;

	if (0 == pPlayer->m_CurHp && SUpgradeMsg::EPRO_PLAYER_DEAD != pMsg->_protocol)
		return FALSE;

    switch (pMsg->_protocol)
    {
    case SUpgradeMsg::EPRO_SET_POINT:
		pPlayer->OnRecvSetPoint((SQSetPointMsg *)pMsg);
        break;

	// 洗点消息
	case SUpgradeMsg::EPRO_RESET_POINT:
		//pPlayer->OnRecvResetPoint((SQResetPointMsg*)pMsg);
		pPlayer->OnRecvClearPoint((SQResetPointMsg*)pMsg);
		break;

    case SUpgradeMsg::EPRO_PASS_VENAPOINTASK:
        {
            SQPassVenapointAskMsg *pAskMsg = (SQPassVenapointAskMsg*)pMsg;
            //pPlayer->OnRecvPassVenapointAsk( pAskMsg->byVenationNum, pAskMsg->byVenapointNum );
        }
        break;

	case SUpgradeMsg::EPRO_TELERGY_UPDATE:
		pPlayer->OnRecvUpdateTelegry((SQTelergyUpdate *)pMsg);
		break;

		//add 2014.3.1
	case SUpgradeMsg::EPRO_OPEN_SETSKILL:
		pPlayer->OnOpenSetSkillPlan((SQOpenSetSkillBaseMsg *)pMsg);
		break;
	case SUpgradeMsg::EPRO_UPDATE_PLAYERSKILL:
		pPlayer->OnUdtPlayerSkillAndPatern((SQUpdateSkillBaseMsg *)pMsg);
		break;

	case  SUpgradeMsg::EPRO_SKILL_UPDATE:
		pPlayer->OnRecvUpdateSkill((SQSkillUpdate *)pMsg);
		break;

	case SUpgradeMsg::EPRO_SET_EXTRATELERGY:
        //pPlayer->OnExtraTelergyModify( ( SQSetExtraTelergyMsg* )pMsg );
        break;
 
	case SUpgradeMsg::EPRO_OPEN_EXTRATELERGY:
        //pPlayer->OnOpenExtraTelergy( ( SQOpenExtraTelergyMsg* )pMsg );
        break;

	case SUpgradeMsg::EPRO_QUEST_ADD_POINTS:
		//pPlayer->OnRecvQuestAddPoints((SQuestAddPoint_C2S_MsgBody*)pMsg);	
		break;

	case SUpgradeMsg::EPRO_PLAYER_DEAD:
		pPlayer->QuestToRelive(((SQPlayerDeadMsg *)pMsg)->bType);
		break;

	case SUpgradeMsg::EPRO_OPEN_VEN:
		pPlayer->RecvOpenVenMsg((SQOpenVenMsg *)pMsg);
		break;
	case  SUpgradeMsg::EPRO_FIGHTPET_SKILLUPDATE:
		pPlayer->_fpLearnSkillByfp((SQfpSkillUpdate *)pMsg);
		break;
	case SUpgradeMsg::EPRO_TIZHI_UPDATE:
		pPlayer->OnRecvTiZhiUpdateMsg((SQTiZhiUpdateMsg*)pMsg);
		break;
	case SUpgradeMsg::EPRO_TIZHI_TUPO:
		pPlayer->OnRecvTizhiTupoMsg((SQTiZhiTupoMsg*)pMsg);
		break;
	case SUpgradeMsg::EPRO_KYLINARM_UPGRADE:
		pPlayer->OnRecvKylinArmUpgradeMsg((SQKylinArmUpgradeMsg*)pMsg);
		break;
		//add by ly 2014/3/25	
	case SUpgradeMsg::EPRO_PLAYERGETGLORY:	//请求玩家荣耀信息
		pPlayer->GetPlayerGloryInfo((SQGloryMsg*)pMsg);
		break;
	case SUpgradeMsg::EPRO_GETPLAYERGLORY:	//领取荣耀奖励
		pPlayer->GetGloryAward((SQGetGloryMsg*)pMsg);
		break;
	case SUpgradeMsg::EPRO_TITLE:	//称号信息
		pPlayer->OnHandleTitleMsg((SQTitleMsg*)pMsg);
		break;
	case SUpgradeMsg::EPRO_QUEST_RANDLISTINF:	//请求排行榜信息
		pPlayer->OnHandleRandListMsg((SQPlayerRankList*)pMsg);
		break;
	case SUpgradeMsg::EPRO_RANKLIST_GEAWARDSTATE:	//获取玩家排行榜领奖状态
		pPlayer->OnHandleGetRankAwardState((SQGetRankAwardState*)pMsg);
		break;
	case SUpgradeMsg::EPRO_RANKLIST_GETWARD:	//请求领取奖励
		pPlayer->OnHandleRankAwardOpt((SQGetRankAward*)pMsg);
		break;
	case SUpgradeMsg::EPRO_ACTIVENESS_GETINFO:	//获取玩家活跃度信息
		pPlayer->OnGetActivenessInf((SQGetActivenessInfo *)pMsg);
		break;
	case SUpgradeMsg::EPRO_ACTIVENESS_GETAWARD:	//领取活跃度的对应奖励
		pPlayer->OnGetActivenessAward((SQGetActivenessAward *)pMsg);
		break;
    default:
        break;
    }

    return TRUE;
}

BOOL CPlayerManager::DispatchNameCardMessage(DNID dnidClient, struct SNameCardBaseMsg *pMsg, CPlayer *pPlayer)
{
    if (pMsg == NULL || dnidClient == 0 || pPlayer == NULL) 
        return FALSE;

    // pPlayer->CNameCard::OnDispatchMsg(pMsg);

    return TRUE;
}

BOOL CALLBACK KickHangupPlayer( LPIObject &Player, LPARAM )
{
    if (CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER))
    {
        if ( pPlayer->m_OnlineState != CPlayer::OST_HANGUP )
            return true;

        pPlayer->Logout(true);
    }

    return true;
}

BOOL CALLBACK KickPlayer( LPIObject &Player, LPARAM )
{
	SQLogoutMsg lmsg;
	lmsg.byLogoutState = SQLogoutMsg::EST_LOGOUT_NORMAL;
	if (CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER))
	{
// 		if ( pPlayer->m_OnlineState != CPlayer::OST_NORMAL )
// 			return true;
		lmsg.dwGlobalID = pPlayer->GetGID();
		GetGW()->OnLogout(pPlayer->m_ClientIndex, &lmsg, pPlayer);
		//pPlayer->Logout(true);
	}

	return true;
}

void CPlayerManager::KickHangupPlayerAll()
{
     TraversalPlayers( ( TRAVERSALFUNC )KickHangupPlayer, 0 );
}

LPIObject GetPlayerByGID(DWORD GID)
{
    if (s_pUniqueThis == NULL)
        return LPIObject();

    return s_pUniqueThis->LocateObject(GID);
}

LPIObject GetPlayerByDnid(DNID dnidClient)
{
    if (s_pUniqueThis == NULL)
        return LPIObject();

    if (dnidClient == 0)
        return LPIObject();

    std::map<DNID, DWORD>::iterator it = s_pUniqueThis->m_PlayerContextMap.find(dnidClient);

    if (it == s_pUniqueThis->m_PlayerContextMap.end())
        return LPIObject();

    return GetPlayerByGID(it->second);
}

LPIObject GetPlayerByName(LPCSTR szName)
{
    if (s_pUniqueThis == NULL)
        return LPIObject();

    if (szName == NULL)
        return LPIObject();

	std::map<dwt::stringkey<char[CONST_USERNAME]>, DWORD>::iterator it = s_pUniqueThis->m_PlayerNameMap.find(szName);

    if (it == s_pUniqueThis->m_PlayerNameMap.end())
        return LPIObject();

    return GetPlayerByGID(it->second);
}

LPIObject GetPlayerByAccount(LPCSTR szAccount)
{
    if (s_pUniqueThis == NULL)
        return LPIObject();

    if (szAccount == 0)
        return LPIObject();

    std::map< std::string, DWORD >::iterator it = s_pUniqueThis->m_PlayerAccountMap.find( szAccount );

    if (it == s_pUniqueThis->m_PlayerAccountMap.end())
        return LPIObject();

    return GetPlayerByGID(it->second);
}

BOOL BindNameRelation(LPCSTR szName, LPIObject pObj)
{
    if (s_pUniqueThis == NULL)
        return FALSE;

    if (szName == NULL)
        return FALSE;

    if (!pObj)
        return FALSE;

    if (pObj->DynamicCast(IID_PLAYER) == NULL)
        return FALSE;

    s_pUniqueThis->m_PlayerNameMap[szName] = pObj->GetGID();

    return TRUE;
}

BOOL RemoveNameRelation(LPCSTR szName)
{
    if (s_pUniqueThis == NULL)
        return FALSE;

    if (szName == NULL)
        return FALSE;

	std::map<dwt::stringkey<char[CONST_USERNAME]>, DWORD>::iterator it =
        s_pUniqueThis->m_PlayerNameMap.find(szName);

    if (it == s_pUniqueThis->m_PlayerNameMap.end())
        return FALSE;

    s_pUniqueThis->m_PlayerNameMap.erase(it);

    return TRUE;
}

BOOL BindAccountRelation(LPCSTR szAccount, LPIObject pObj)
{
    if (s_pUniqueThis == NULL)
        return FALSE;

    if (szAccount == NULL)
        return FALSE;

    if (!pObj)
        return FALSE;

    if (pObj->DynamicCast(IID_PLAYER) == NULL)
        return FALSE;

    s_pUniqueThis->m_PlayerAccountMap[szAccount] = pObj->GetGID();

    return TRUE;
}

BOOL RemoveAccountRelation(LPCSTR szAccount)
{
    if (s_pUniqueThis == NULL)
        return FALSE;

    if (szAccount == NULL)
        return FALSE;

    std::map< std::string, DWORD>::iterator it = 
        s_pUniqueThis->m_PlayerAccountMap.find( szAccount );

    if (it == s_pUniqueThis->m_PlayerAccountMap.end())
        return FALSE;

    s_pUniqueThis->m_PlayerAccountMap.erase(it);

    return TRUE;
}

BOOL BindPlayerByDnid(DNID dnidClient, LPIObject pObj)
{
    if (s_pUniqueThis == NULL)
        return FALSE;

    if (dnidClient == 0)
        return FALSE;

    if (!pObj)
        return FALSE;

    if (pObj->DynamicCast(IID_PLAYER) == NULL)
        return FALSE;

    if (!SetDnidContext(dnidClient, pObj.get()))
        return FALSE;

    s_pUniqueThis->m_PlayerContextMap[dnidClient] = pObj->GetGID();

    return TRUE;
}

BOOL RemovePlayerByDnid(DNID dnidClient)
{
    if (s_pUniqueThis == NULL)
        return FALSE;

    if (dnidClient == 0)
        return FALSE;

    std::map<DNID, DWORD>::iterator it = s_pUniqueThis->m_PlayerContextMap.find(dnidClient);

    if (it == s_pUniqueThis->m_PlayerContextMap.end())
        return FALSE;

    s_pUniqueThis->m_PlayerContextMap.erase(it);

    SetDnidContext(dnidClient, NULL);

    return TRUE;
}

int TraversalPlayers(TRAVERSALFUNC TraversalFunc, LPARAM param)
{
    if (s_pUniqueThis == NULL)
        return 0;

    return s_pUniqueThis->TraversalObjects(TraversalFunc, param);
}


LPIObject GetPlayerBySID(DWORD dwStaticID)
{
 if (s_pUniqueThis == NULL)
        return LPIObject();

    if (dwStaticID == 0)
        return LPIObject();

    std::map<DWORD, DWORD>::iterator it = s_pUniqueThis->m_PlayerSIDMap.find(dwStaticID);

    if (it == s_pUniqueThis->m_PlayerSIDMap.end())
        return LPIObject();

    return GetPlayerByGID(it->second);
}

BOOL BindSIDRelation(DWORD dwStaticID, LPIObject pObj)
{
    if (s_pUniqueThis == NULL)
        return FALSE;

    if (dwStaticID == 0)
        return FALSE;

    if (!pObj)
        return FALSE;

    if (pObj->DynamicCast(IID_PLAYER) == NULL)
        return FALSE;

    s_pUniqueThis->m_PlayerSIDMap[dwStaticID] = pObj->GetGID();

    return TRUE;
}

BOOL RemoveSIDRelation(DWORD dwStaticID)
{
    if (s_pUniqueThis == NULL)
        return FALSE;

    if (dwStaticID == NULL)
        return FALSE;

    std::map<DWORD, DWORD>::iterator it = 
        s_pUniqueThis->m_PlayerSIDMap.find(dwStaticID);

    if (it == s_pUniqueThis->m_PlayerSIDMap.end())
        return FALSE;

    s_pUniqueThis->m_PlayerSIDMap.erase(it);

    return TRUE;
}

/*
struct SGetPlayerByName
{
    dwt::stringkey< char [CONST_USERNAME] > szName;
    LPIObject pPlayer;
};

BOOL CALLBACK FindPlayerByName(LPIObject &Player, SGetPlayerByName *pInOut)
{
    CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER);
    if (pPlayer == NULL)
        return TRUE;

    if (dwt::strcmp(pInOut->szName, pPlayer->m_Property.m_Name, CONST_USERNAME) == 0)
    {
        pInOut->pPlayer = pPlayer;
        return FALSE; // false means break traversal
    }

    return TRUE;
}

LPIObject GetPlayerByName(LPCSTR szName)
{
    SGetPlayerByName ret;
    ret.szName = szName;
    TraversalPlayers((TRAVERSALFUNC)FindPlayerByName, (LPARAM)&ret);
    return ret.pPlayer;
}

struct SGetPlayerByNameN
{
    dwt::stringkey< char [CONST_USERNAME] > szName;
    int i;
};

BOOL CALLBACK FindPlayerByNameN(LPIObject &Player, SGetPlayerByNameN *pInOut)
{
    CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER);
    if (pPlayer == NULL)
        return TRUE;

    if (dwt::strcmp(pInOut->szName, pPlayer->m_Property.m_Name, CONST_USERNAME) == 0)
    {
        pInOut->i++;
        return FALSE; // false means break traversal
    }

    return TRUE;
}

int GetPlayerByNameN(LPCSTR szName)
{
    SGetPlayerByNameN ret;
    ret.szName = szName;
    ret.i = 0;
    TraversalPlayers((TRAVERSALFUNC)FindPlayerByNameN, (LPARAM)&ret);
    return ret.i;
}

struct SGetPlayerByAccount
{
    dwt::stringkey< char [17] > szAccount;
    LPIObject pPlayer;
};

BOOL CALLBACK FindPlayerByAccount(LPIObject &Player, SGetPlayerByAccount *pInOut)
{
    CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER);
    if (pPlayer == NULL)
        return TRUE;

    if (dwt::strcmp(pInOut->szAccount, pPlayer->m_szAccount, 17) == 0)
    {
        pInOut->pPlayer = pPlayer;
        return FALSE; // false means break traversal
    }

    return TRUE;
}

LPIObject GetPlayerByAccount(LPCSTR szAccount)
{
    SGetPlayerByAccount ret;
    ret.szAccount = szAccount;
    TraversalPlayers((TRAVERSALFUNC)FindPlayerByAccount, (LPARAM)&ret);
    return ret.pPlayer;
}
*/

size_t GetPlayerNumber()
{
    if (s_pUniqueThis == NULL)
        return 0;

    return s_pUniqueThis->size();
}

BOOL CALLBACK SavePlayerDataAndLogout(LPIObject &Player, LPARAM)
{
    if (CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER))
        pPlayer->Logout(true);

    return true;
}

void SaveAndLogoutAllPlayer()
{
    TraversalPlayers( ( TRAVERSALFUNC )SavePlayerDataAndLogout, 0 );

    if (s_pUniqueThis && s_pUniqueThis->static_playercache) 
    { 
        s_pUniqueThis->static_playercache->TheEnd(); 
        delete s_pUniqueThis->static_playercache;
        s_pUniqueThis->static_playercache = NULL;
    }
	if (g_pmemoryobject)
	{
		g_pmemoryobject->CloseShareMemory();
	}
}

LPIObject GetPlayerByAccount2(LPCSTR szAccount)
{
    return LPIObject();
}

void SetPlayerCountFlag(LPCSTR szAccount, BOOL bFlag)
{
   LPIObject pObj = GetPlayerByAccount(szAccount);
   CPlayer *pPlayer = (CPlayer*)pObj->DynamicCast(IID_PLAYER);
   if(pPlayer == NULL) {
       rfalse(2, 1, "没有此玩家，可能不在线!!!!!");
       return;
   }

    pPlayer->SetCountFlag(bFlag);
}

LPCSTR CPlayerManager::GetRelationString()
{
    return FormatString("源[%d]Dnid[%d]Name[%d]Account[%d]", 
        size(), m_PlayerContextMap.size(), 
        m_PlayerNameMap.size(), m_PlayerAccountMap.size());
}

void CPlayerManager::KickPlayerAll()
{
	TraversalPlayers( ( TRAVERSALFUNC )KickPlayer, 0 );
}

// 踢出特定的玩家
void KickOnePlayer()
{

}

void BroadcastTalkToMsg(const char*pMsg, DWORD wSize)
{
	TraversalPlayers((TRAVERSALFUNC)TalkToAllMessageCallback, (LPARAM)pMsg);

// 	if (s_pUniqueThis == NULL || wSize > 0xffff )
// 		return;
// 	if (!g_bUsePackageMessage)
// 	{
// 		SBroadcastStruct Param;
// 		Param.pBuffer = pMsg;
// 		Param.wSize = (WORD)wSize;
// // 		SAChatGlobalMsg *chatmsg=(SAChatGlobalMsg*)pMsg;
// // 		if(!chatmsg)return;
// 		TraversalPlayers((TRAVERSALFUNC)TalkToAllMessageCallback, (LPARAM)&pMsg);
// 		return;
// 	}
}

void BroadcastMsg(SMessage *pMsg, DWORD wSize)
{
    if (s_pUniqueThis == NULL || wSize > 0xffff )
        return;

    if (!g_bUsePackageMessage)
    {
        SBroadcastStruct Param;
        Param.pBuffer = pMsg;
        Param.wSize = (WORD)wSize;
		SAChatGlobalMsg *chatmsg=(SAChatGlobalMsg*)pMsg;
		if(!chatmsg)return;
        TraversalPlayers((TRAVERSALFUNC)BroadcastMessageCallback, (LPARAM)&Param);
        return;
    }

	/*
    // 如果push失败，说明缓冲区已满，需要先将数据发出！
    if (!s_pUniqueThis->m_GlobalBroadcastMsgPack.PushMsg(pMsg, wSize))
    {
        LPVOID pPackage = NULL;
        WORD wSize = 0;

        // 获取缓冲区数据
        if (s_pUniqueThis->m_GlobalBroadcastMsgPack.GetPackagedMsg(pPackage, wSize))
        {
            if (wSize != 0)
            {
                SBroadcastStruct Param;
                Param.pBuffer = pPackage;
                Param.wSize = wSize;
                TraversalPlayers((TRAVERSALFUNC)BroadcastPackageCallback, (LPARAM)&Param);
            }
        }

        // 再次push
        s_pUniqueThis->m_GlobalBroadcastMsgPack.PushMsg(pMsg, wSize);
    }
	*/
}

//////////////////////////////////////////////////////////////////////////////////////////////
/*

static BOOL CALLBACK s_net_operation_end_callback_simpleplayer(
    const StringKey32 &szName, 
    SSimplePlayer *pObject, 
    CDistributedSimplePlayerManager *pSimplePlayerManager)
{
    if (pObject != NULL)
    {
    }
    else
    {
    }

    return TRUE;
}

CDistributedSimplePlayerManager::CDistributedSimplePlayerManager()
{
    m_OrbUser.SetNetOpEndCallback((tForceReadMostlyOrbUser<SSimplePlayer>::NET_OPEND_CALLBACK)
        s_net_operation_end_callback_simpleplayer, (LPARAM)this);
}

CDistributedSimplePlayerManager::~CDistributedSimplePlayerManager()
{
}

void CDistributedSimplePlayerManager::UpdateSimplePlayer(LPCSTR szName, class CPlayer *pPlayer)
{
    if (szName == NULL)
        return;

    if (pPlayer == NULL)
    {
        m_OrbUser.RequestDeleteObject(szName);
    }
    else
    {
        WORD  GetServerID();

        SSimplePlayer data;
        data.dnidClient = pPlayer->m_ClientIndex;
        data.dwGlobalID = pPlayer->GetGID();
        data.dwGMLevel = pPlayer->m_Property.m_GMLevel;
        data.dwStaticID = pPlayer->m_Property.m_dwStaticID;
        data.wServerID = GetServerID();
		data.bOnlineState = (pPlayer->m_OnlineState == CPlayer::OST_HANGUP) ? 2 : 1;

        if (pPlayer->m_ParentRegion != NULL)
        {
            data.wCurRegionID = pPlayer->m_ParentRegion->m_wRegionID;
        }
        else
        {
            data.wCurRegionID = pPlayer->m_Property.m_CurRegionID;
        }

        if (tForceReadMostlyOrbUser<SSimplePlayer>::rwo_ptr pSimplePlayer = 
            m_OrbUser.RequestReadWriteObject(szName))
        {
            *pSimplePlayer = data;
        }
        else
        {
            m_OrbUser.RequestCreateObject(szName, data);
        }
    }
}

BOOL CDistributedSimplePlayerManager::LocateSimplePlayer(LPCSTR szName, SSimplePlayer &SimplePlayer)
{
    if (tForceReadMostlyOrbUser<SSimplePlayer>::roo_ptr pSimplePlayer = 
        m_OrbUser.RequestReadOnlyObject(szName))
    {
        SimplePlayer = *pSimplePlayer;
        return TRUE;
    }

    return FALSE;
}

DWORD CDistributedSimplePlayerManager::GetCurrentTatolPlayerNumber()
{
    return m_OrbUser.GetSize();
}

void UpdateDistributedSimplePlayer(LPCSTR szName, CPlayer *pPlayer)
{
    g_DistributedSimplePlayerManager.UpdateSimplePlayer(szName, pPlayer);
}

BOOL LocateDistributedSimplePlayer(LPCSTR szName, SSimplePlayer &SimplePlayer)
{
    return g_DistributedSimplePlayerManager.LocateSimplePlayer(szName, SimplePlayer);
}

DWORD GetCurrentTatolPlayerNumber()
{
    return g_DistributedSimplePlayerManager.GetCurrentTatolPlayerNumber();
}
*/

void SendToPlayer(LPCSTR szName, LPVOID pMsg, WORD wSize)
{
}

void TalkToPlayer(LPCSTR szName, LPCSTR info)
{
    SAChatWisperMsg msg;
    msg.byType = 1;
    dwt::strcpy(msg.szName, "[系统]", sizeof(msg.szName));
    dwt::strcpy(msg.cChatData, info, sizeof(msg.cChatData));

    
    CPlayer *pPlayer = reinterpret_cast< CPlayer* >( GetPlayerByName( szName )->DynamicCast( IID_PLAYER ) );
    if ( pPlayer )
        g_StoreMessage(  pPlayer->m_ClientIndex, &msg, sizeof( msg ) );
    //SendMessageToPlayer(szName, &msg, msg.GetMySize());
}

struct SBroadcastSchoolStruct
{
    LPVOID pBuffer;
    WORD   wSize;
    BYTE   bySchool;
};

int CALLBACK SendSchoolMsg( LPIObject &Player, SBroadcastSchoolStruct *pBroadMsgStruct ) 
{
	if (!pBroadMsgStruct)
	{
		rfalse(4,1,"CALLBACK SendSchoolMsg");
		return 1;
	}
    if ( CPlayer* pDest = ( CPlayer* )Player->DynamicCast( IID_PLAYER ) )
    {
        SAChatSchool msg;
        dwt::strcpy( msg.cChatData, ( ( SQChatSchool* )( pBroadMsgStruct->pBuffer ) )->cChatData, MAX_CHAT_LEN );
		dwt::strcpy(msg.cTalkerName, ((SQChatSchool*)(pBroadMsgStruct->pBuffer))->cTalkerName, CONST_USERNAME);

        // 没有帮派且同门派的人才能接收到消息

        if ( (pDest->m_Property.m_School == pBroadMsgStruct->bySchool || pBroadMsgStruct->bySchool == 111) && pDest->m_Property.m_szTongName[ 0 ] == 0 )
            g_StoreMessage( pDest->m_ClientIndex, &msg, pBroadMsgStruct->wSize );
    }
    return 1;
}

static DWORD prev = timeGetTime(); //发送间隔30秒
void BroadcastSchoolChatMsg( CPlayer* pPlayer, SMessage *pMsg, WORD wSize )  // 广播门派消息
{
	/*
    if ( s_pUniqueThis == NULL )
        return;

    if ( pPlayer == NULL )
        return;

    SBroadcastSchoolStruct Param;
    Param.pBuffer  = pMsg;
    Param.wSize    = wSize;
    Param.bySchool = pPlayer->m_Property.m_School;

	if (pPlayer->IsWuLinChief() && pPlayer->m_Property.m_Equip[8].details.wIndex == 40205)//是武林盟主的判断 还要带“武林玺” 就向所有门派发消息
	{	
		Param.bySchool = 111;//所有门派发送判断标志

		if ( abs( ( int )( timeGetTime() - prev ) ) < 1000 * 30)
		{
			TalkToDnid(pPlayer->m_ClientIndex, "武林盟主在佩戴武林玺的时候不能说太快哦！");
			return;
		}
		prev = timeGetTime();
	}
    TraversalPlayers( ( TRAVERSALFUNC )SendSchoolMsg, ( LPARAM )&Param );
	//*/
}

// ==============add by an===============
// 重载版本
int CALLBACK GetAllPlayerObjectEx(LPIObject& pObject, std::vector< const CPlayer* >* pPlayerVec)
{
	if ( CPlayer* pDest = ( CPlayer* )pObject->DynamicCast( IID_PLAYER ) )
		pPlayerVec->push_back( pDest );
	return 1;
}

// int CALLBACK PutAllPlayerIntoMap(LPIObject& pObject, std::map< const CPlayer*> *pPlayerMAP)
// {
// 	if ( CPlayer* pDest = ( CPlayer* )pObject->DynamicCast( IID_PLAYER ) )
// 		pPlayerMAP->insert(std::make_pair(pDest->m_Property.m_Name, pDest));
// 	return 1;
// }
// ===========================================

BOOL  CALLBACK ChecklivePlayer( LPIObject &Player, DWORD &number )
{
	CPlayer *pdest = ( CPlayer* )Player->DynamicCast( IID_PLAYER );
	if (pdest && GetGW())
	{
		if (g_GetLogStatus(pdest->m_ClientIndex)== 0 && pdest->m_bInit)
		{
			if (pdest->m_nchecklivetime == 0xffffffff){
				pdest->m_nchecklivetime = timeGetTime() + MAX_CHECKLIVETIME;
				return 2;
			}
			else{
				if (timeGetTime() < pdest->m_nchecklivetime){
					return 2;
				}
			}
			SYSTEMTIME stime;
			GetLocalTime(&stime);
			SQLogoutMsg lmsg;
			lmsg.byLogoutState = SQLogoutMsg::EST_LOGOUT_NORMAL;
			lmsg.dwGlobalID = pdest->GetGID();
			//AddInfo(FormatString("注意：发现一个掉线玩家 <%s> ，踢下去\n",pdest->getname()));
			rfalse(3, 1, "检测到掉线玩家退出 [%s]",pdest->getname());  
			return GetGW()->OnLogout(pdest->m_ClientIndex,&lmsg,pdest);
		}
	}
	return 1;
}

static BOOL CALLBACK TalkToAllBlessCallback(LPIObject &Player, LPARAM param)
{
	CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		SABlessAllServerNoticeMsg* pMsg = (SABlessAllServerNoticeMsg*)param;
		if (pMsg)
		{
			g_StoreMessage(pPlayer->m_ClientIndex, pMsg, sizeof(SABlessAllServerNoticeMsg));
		}
	}
	return TRUE;
}
void BroadcastAllBlessMsg(SABlessAllServerNoticeMsg *pMsg)
{
	TraversalPlayers((TRAVERSALFUNC)TalkToAllBlessCallback, (LPARAM)pMsg);
}
static BOOL CALLBACK TalkToAllSystemMsg(LPIObject &Player,LPARAM param)
{
	CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		SAChatBroadcastMsg * pMsg = (SAChatBroadcastMsg*)param;
		if (pMsg)
		{
			g_StoreMessage(pPlayer->m_ClientIndex,pMsg,sizeof(SAChatBroadcastMsg));
		}
	}
	return TRUE;
}
void BroadcastSysNoticeMsg(SMessage *pMsg)
{
	TraversalPlayers((TRAVERSALFUNC)TalkToAllSystemMsg,(LPARAM)pMsg);
}
static BOOL CALLBACK TalkToAllBlessOpenCloseCallback(LPIObject &Player, LPARAM param)
{
	CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		SABLESSOpenClose* pMsg = (SABLESSOpenClose*)param;
		if (pMsg)
		{
			pPlayer->m_PlayerOpenClose = pMsg->Flag;
			g_StoreMessage(pPlayer->m_ClientIndex, pMsg, sizeof(SABLESSOpenClose));
		}
	}
	return TRUE;
}
void BroadcastBlessOpenClose(SMessage *pMsg)
{
	TraversalPlayers((TRAVERSALFUNC)TalkToAllBlessOpenCloseCallback,(LPARAM)pMsg);
}

bool SetNameToMap(LPCSTR newName, LPCSTR szName)
{
	if (s_pUniqueThis == NULL)
		return false;

	if (szName == NULL)
		return false;

	std::map<dwt::stringkey<char[CONST_USERNAME]>, DWORD>::iterator it = s_pUniqueThis->m_PlayerNameMap.find(szName);

	if (it == s_pUniqueThis->m_PlayerNameMap.end())
		return false;
	int sid = it->second;
	s_pUniqueThis->m_PlayerNameMap.erase(it);
	s_pUniqueThis->m_PlayerNameMap[newName] = sid;
	return true;

}