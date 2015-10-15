
#include "stdafx.h"
#include "unionmanage.h"
#include <time.h>
#include "Player.h"
#include "networkmodule/UnionMsgs.h"
#include "TeamManager.h"
#include "networkmodule/TeamMsgs.h"
#include "PlayerManager.h"
#include "Item.h"
#include "ItemUser.h"

static DWORD itemAccumulator = 0;
static DWORD prevTimestamp = 0;
static time_t timeSegment = 0;

extern LPIObject GetPlayerBySID(DWORD dwStaticID);
extern CScriptManager g_Script;

static WORD unionCount = 0;

BOOL unionLoadSuccess = FALSE;
DWORD unionMaxExp = 0;
WORD unionMax = 0;
static BOOL timeAccumulatorInit()
{
    tm timeMaker; 
    ZeroMemory( &timeMaker, sizeof( tm ) ); 
    timeMaker.tm_year = SItemBase::YEAR_SEGMENT - 1900;
    timeSegment = mktime( &timeMaker );
    return TRUE;
}

static const int MAX_EXP = 4000000;

static BOOL timeAccumulatorInited = timeAccumulatorInit();


UnionDataManager::UnionDataManager()
{
	m_bIsRefused = false;
}

void UnionDataManager::Run()
{
//     static DWORD dwSaveTime = timeGetTime();
//     static DWORD dwCheckTime = timeGetTime();
// 
//     // 每天的定时清空
//     if (  ( int )( timeGetTime() - dwCheckTime ) > 1000 * 60 )
//     {
//         dwCheckTime = timeGetTime();
//         time_t currTime = time( NULL );
//         tm *pcurTime = localtime( &currTime );
//         if ( pcurTime == NULL )
//             return ;
//         if ( ( pcurTime->tm_min == 1 ) && pcurTime->tm_hour == 0 )
//         {
//             std::map< std::string, SPlayerUnionData >::iterator iter = m_UnionDataMap.begin();
//             for ( iter; iter != m_UnionDataMap.end(); iter++ )
//             {
//                 for ( int i = 0; i < SPlayerUnionData::NUMBER_MAX; i++ )
//                 {
//                     iter->second.exp[ i ] = 0;
// 
//                     if ( iter->second.roleid[i] > 0 )
//                     {
//                         SAUpdateExpMsg msg;
//                         msg.exp = 0;
//                         msg.memberIdx = i;
//                         msg.type = SAUpdateUnionMsg::UPDATE_EXP;
//                         SendUnionAllMsg( &iter->second, &msg, sizeof( msg )  );
//                     }
//                 }
//             }
//         }
//     }
// 
// 	if ( ( int )( timeGetTime() - dwSaveTime ) > ( 1000 * 60 * 30 ) )
// 	{
//         if ( unionLoadSuccess == FALSE )
//             return ;
// 
//         rfalse( 2, 1, "union save chk %08x %08x", timeGetTime(), dwSaveTime );
// 
//         dwSaveTime = timeGetTime();
//         SaveUnionData();
//     }
}


BOOL UnionDataManager::LoadUnionData( SAGetUnionDataMsg *pMsg )
{
//     LPBYTE streamIter = ( LPBYTE )pMsg->streamData;
//     const DWORD dwUnionMax = pMsg->unionCount;
//     DWORD useDataSize = 0;
// 
//     if ( pMsg->unionCount > 0 )
//     {
//         for ( int i = 0; i < (int)dwUnionMax; i++ )
//         {
//             // 如果SIZE过来那么就非法啦
//             if ( useDataSize > pMsg->dwSendSize || useDataSize > sizeof( pMsg->streamData ) )
//                 break;
// 
//             SPlayerUnionData *temp = ( SPlayerUnionData * )streamIter;
//             std::string str( temp->title );
//             str += temp->suffx;
//             m_UnionDataMap[ str ] = *temp;
// 
//             for ( int j = 0; j < SPlayerUnionBase::NUMBER_MAX; j++ )
//                 unionMappedMap[ temp->roleid[j] ] = str;
// 
//             useDataSize += sizeof( SPlayerUnionData );
//             streamIter += sizeof( SPlayerUnionData );
//         }
//     }
// 
//     if ( pMsg->readEnd == TRUE )
//     {
//         unionLoadSuccess = TRUE;
//   //      rfalse( 2, 1, "读取结义数据成功,%d个", m_UnionDataMap.size() );
//     }

    return TRUE;
}

extern int SendToLoginServer( SMessage *data, long size );

static SQSaveUnionDataMsg  msg;
BOOL UnionDataManager::SaveUnionData()
{
    DWORD unionCount = 0;
    DWORD useSize = 0;

    msg.saveTime = (DWORD)(time( NULL ) - timeSegment);

    LPBYTE iterStream = ( LPBYTE )msg.streamData;

    std::map< std::string, SPlayerUnionData >::iterator iter =  m_UnionDataMap.begin();
    for ( iter; iter != m_UnionDataMap.end(); iter ++ )
    {
        if ( useSize + ( sizeof( SPlayerUnionData )  ) > sizeof( msg.streamData )  )
        {
            msg.unionCount = unionCount;
            msg.dwSendSize = useSize;
            SendToLoginServer( &msg, msg.dwSendSize + sizeof( msg ) - sizeof( msg.streamData ) );
            unionCount = 0;
            useSize = 0;
            iterStream = ( LPBYTE )msg.streamData;
        }

        *( SPlayerUnionBase* )iterStream = ( SPlayerUnionBase )iter->second;
        iterStream += sizeof( SPlayerUnionBase );
        useSize += sizeof( SPlayerUnionBase );
        unionCount ++;
    }

    msg.unionCount = unionCount;
    msg.dwSendSize = useSize;
    SendToLoginServer( &msg, msg.dwSendSize + sizeof( msg ) - sizeof( msg.streamData ) );
    return TRUE;
}


void UnionDataManager::AddPlayerExp( CPlayer *p, DWORD addVal )
{
//     if ( p == NULL || addVal == 0 )
//         return;
// 
//     // 只获得当前经验的10%
//     addVal = addVal * 10 / 100;
// 
//     std::map< DWORD, std::string >::iterator iter = unionMappedMap.find( p->GetSID() );
//     if ( iter == unionMappedMap.end() )
//         return ;
// 
//     std::map< std::string, SPlayerUnionData >::iterator iterData = m_UnionDataMap.find( iter->second.c_str() );
//     if ( iterData == m_UnionDataMap.end() )
//         return ;
// 
//     SPlayerUnionData &data = iterData->second;
//     for ( int i = 0; i < SPlayerUnionData::NUMBER_MAX; i++ )
//     {
//         // 如果其中存在有-1的就说明大家都已经满足条件了，不能在添加经验
//         if ( data.exp[ i ] > unionMaxExp )
//             break;
// 
//         if ( data.roleid[ i ] == p->GetSID() )
//         {
//             DWORD expPrev = data.exp[ i ] / ( unionMaxExp / 100 );
//             if ( data.exp[ i ] + addVal < unionMaxExp )
//                 data.exp[ i ] += addVal;
//             else
//                 data.exp[ i ] = unionMaxExp;
// 
//             if ( unionMaxExp == 0 )
//                 unionMaxExp = 1;
// 
//             DWORD expPro = data.exp[ i ] / ( unionMaxExp / 100 );
//             // 同步经验到客户端...
//             if ( expPrev != expPro )
//             {
//                 SAUpdateExpMsg msg;
//                 msg.exp = data.exp[ i ];
//                 msg.memberIdx = i;
//                 msg.type = SAUpdateUnionMsg::UPDATE_EXP;
//                 SendUnionAllMsg( &data, &msg, sizeof( msg )  );
//             }
//             ;
//         }
//     }
}

void UnionDataManager::TalkToUnionMember( std::string key,  LPCSTR info )
{
//     if ( info == NULL )
//         return; 
// 
//     SPlayerUnionData *p = GetUnionDataByUnionName( key );
//     if ( p == NULL )
//         return ;
// 
//     for ( int i = 0; i < SPlayerUnionData::NUMBER_MAX; i++ )
//     {
//         CPlayer* player = ( CPlayer* )GetPlayerBySID( p->roleid[i] )->DynamicCast( IID_PLAYER );
//         if( player == NULL )
//             continue;
// 
//         TalkToDnid( player->m_ClientIndex, info );
//     }
}

void UnionDataManager::SendUnionAllMsg( SPlayerUnionData *p, void *pMsg, size_t size )
{
//     if ( p == NULL )
//         return ;
// 
//     for ( int i = 0; i < SPlayerUnionData::NUMBER_MAX; i++ )
//     {
//         CPlayer* player = ( CPlayer* )GetPlayerBySID( p->roleid[i] )->DynamicCast( IID_PLAYER );
//         if( player == NULL )
//             continue ;
// 
//         player->SendMsg( pMsg, size );
//     }
}

// *********************************************** A ************************************** //
/** 玩家是否已经结义
	参数：	playerID: 玩家ID
	返回：	TRUE:
			FALSE:
*/
BOOL UnionDataManager::IsUnion( DWORD playerID )
{
	std::map< DWORD, std::string >::iterator it = unionMappedMap.find( playerID );
	if( it != unionMappedMap.end() )
		return TRUE;

	return FALSE;
}

/** 从角色ID中删除
	参数：	playerID: 角色ID
*/
void UnionDataManager::DeletePlayerIDFromPlayerIDMap( DWORD playerID )
{
	std::map< DWORD, std::string >::iterator it = unionMappedMap.find( playerID );
	if( it != unionMappedMap.end() )
		unionMappedMap.erase( it );
}

/** 获取角色所在的结义名
	参数：	playerID: 角色ID
*/
std::string UnionDataManager::GetUnionNameByPlayerID( DWORD playerID )
{
	std::string str;
	std::map< DWORD, std::string >::iterator it = unionMappedMap.find( playerID );
	if( it != unionMappedMap.end() )
		str = it->second;

	return str;
}

/** 通过玩家ID获取结义信息
	参数：	playerID: 玩家ID
	返回：	NULL: 不存在
			非空：结义信息
*/
SPlayerUnionData* UnionDataManager::GetUnionDataByPlayerID( DWORD playerID )
{
	std::map< DWORD, std::string >::iterator it = unionMappedMap.find( playerID );
	if( it == unionMappedMap.end() )
		return NULL;

	return GetUnionDataByUnionName( it->second );
}

/** 获取结义的成员数量通过玩家ID
	参数：	结义 tile
	返回：	成员数量
*/
int UnionDataManager::GetUnionMemberNumByPlayerID( DWORD playerID )
{
	std::map< DWORD, std::string >::iterator it = unionMappedMap.find( playerID );
	if( it == unionMappedMap.end() )
		return -1;

	return GetUnionMemberNumByUnionName( it->second );
}
// *********************************************** B ************************************** //
/** 是否已经存在结义名字
	参数：	unionName: 新的结义名
	返回：	TRUE:
			FALSE:
*/
BOOL UnionDataManager::IsHaveUnion( std::string unionName )
{
	std::map< std::string, SPlayerUnionData >::iterator it =  m_UnionDataMap.find( unionName );
	if( it != m_UnionDataMap.end() )
		return TRUE;

	return FALSE;
}

/** 通过结义名获取结义信息
	参数：	unionName: 结义名
	返回：	NULL: 不存在
			非空：结义信息
*/
SPlayerUnionData* UnionDataManager::GetUnionDataByUnionName( std::string unionName )
{
	std::map< std::string, SPlayerUnionData >::iterator it = m_UnionDataMap.find( unionName );
	if( it == m_UnionDataMap.end() )
		return NULL;

	return &it->second;
}

/** 获取结义的成员数量
	参数：	结义 tile
	返回：	成员数量
*/
int UnionDataManager::GetUnionMemberNumByUnionName( std::string unionName )
{
// 	std::map< std::string, SPlayerUnionData >::iterator it = m_UnionDataMap.find( unionName );
// 	if( it == m_UnionDataMap.end() )
// 		return -1;
// 
// 	int num = 0;
// 	for( int i = 0; i < 8; i ++ )
// 	{
// 		if( it->second.roleid[ i ] > 0 )  // 这里不需要判断名字了
// 			num ++;
// 	}
// 
// 	return num;
	return -1;
}

/** 删除结义通过结义名
	参数：	结义 tile
	返回：	成员数量
*/
void UnionDataManager::DeleteUnionDataByUnionName( std::string unionName )
{
	std::map< std::string, SPlayerUnionData >::iterator it = m_UnionDataMap.find( unionName );
	if( it != m_UnionDataMap.end() )
	{
		//兄弟名活动报名所调用的
		g_Script.SetCondition( NULL, NULL, NULL );
		LuaFunctor( g_Script, "UnionSignUpDelete" )[unionName.c_str()]();
		g_Script.CleanCondition();

		m_UnionDataMap.erase( it );

	}
}
// *********************************************** interface ************************************** //
/** 创建新的结义, 键是 title 与 suffx 的结合
	参数：	结义 tile
	返回：	TRUE:
			FALSE:
*/
BOOL UnionDataManager::CreateUnion( LPCSTR title, LPCSTR suffx )
{
// 	if ( title == NULL || suffx == NULL )
// 		return FALSE;
// 
//     if ( m_UnionDataMap.size() > unionMax  )
//         return FALSE;
// 
// 	std::string strname( title );
// 	strname += suffx;
// 
// 	if( IsHaveUnion( strname ) )
// 		return FALSE;
// 
// 	SPlayerUnionData unionData;
// 
// 	memset( &unionData, 0, sizeof( unionData ) );
// 	dwt::strcpy( unionData.title, title, 9 );
// 	dwt::strcpy( unionData.suffx, suffx, 3 );
// 
// 	unionData.title[8] = 0;
// 	unionData.suffx[2] = 0;
// 
// 	m_UnionDataMap[ strname ] = unionData;

	return TRUE;
}

/** 添加一个角色到指定的结义，编号根据先后顺序自动递增
	参数：	playerID: 角色ID
			unionName: 结义名
	返回：	TRUE:
			FALSE:
*/
BOOL UnionDataManager::AddPlayerToUnion( DWORD playerID, std::string unionName )
{
// 	if( IsUnion( playerID ) )
// 		return FALSE;
// 
// 	int idx = GetUnionMemberNumByUnionName( unionName );
// 	if( idx < 0 || idx >= 8 )
// 		return FALSE;
// 
// 	SPlayerUnionData *unionData = GetUnionDataByUnionName( unionName );
// 	if( !unionData )
// 		return FALSE;
// 
// 	CPlayer* pPlayer = ( CPlayer* )GetPlayerBySID( playerID )->DynamicCast( IID_PLAYER );
// 	if( !pPlayer )
// 		return FALSE;
// 
// 	unionData->roleid[ idx ] = playerID;
// 	dwt::strcpy( unionData->szName[ idx ], pPlayer->m_Property.m_Name, CONST_USERNAME );
// 
// 	// 这里就不再做重复判断了，因为前面已经做了处理
// 	unionMappedMap[ playerID ] = unionName;
// 
// 	CPlayer *addPlayer = (CPlayer *)GetPlayerBySID(playerID)->DynamicCast( IID_PLAYER );
// 	if (addPlayer)
// 	{	
// 		g_Script.SetCondition( NULL, addPlayer, NULL );
// 		LuaFunctor( g_Script, "UnionPlayerJoin" )[unionName.c_str()]();
// 		g_Script.CleanCondition();
// 	}
// 
// 
//     UnionOnline( pPlayer );
// 
//     extern LPCSTR FormatString(LPCSTR szFormat, ...);
//     int count = GetUnionMemberNumByUnionName( unionName );
//     if ( count >= 2 )
//         TalkToUnionMember( unionName, FormatString( "[%s]成为了我们的兄弟!!", pPlayer->GetName() ) );

	return TRUE;
}

/** 从结义中删除一个角色，编号自动向前靠拢
	如果最后的人数小于2 自动解散，并通知最后的那一个玩家
	参数：	playerID：被T的兄弟ID
            ClientID：请求割席断交的兄弟
	返回：	TRUE:
			FALSE:
*/
BOOL UnionDataManager::DeletePlayerToUnion( DWORD playerID, DNID clientID )  
{
// 	if (m_bIsRefused)
// 	{
// 		CPlayer* pPlayer = ( CPlayer* )GetPlayerBySID( playerID )->DynamicCast( IID_PLAYER );
// 		if (pPlayer)
// 			TalkToDnid(pPlayer->m_ClientIndex, m_errorInfo.c_str());
// 		return FALSE;
// 	}
// 	if( !IsUnion( playerID ) )
// 		return FALSE;
// 
// 	SPlayerUnionData *unionData = GetUnionDataByPlayerID( playerID );
// 	if( !unionData )
// 		return FALSE;
// 
//     SAKickUnionMsg msg; 
//     SAKickUnionMsg tempMsg;
//     const DWORD    UNIONMONEY  = 5000000;  // 退出结拜需要消耗的金币500W
//     const WORD     WINEID      = 40101;    // 绝情酒编号
//     SPackageItem*  pItem       = NULL;
// 
//     CPlayer* pHeader = ( CPlayer* )GetPlayerByDnid( clientID )->DynamicCast( IID_PLAYER );
//     CPlayer* pMember = ( CPlayer* )GetPlayerBySID( playerID )->DynamicCast( IID_PLAYER );
//     if ( pHeader == NULL ) // 这里没有判断pMember，是因为被T的玩家可能不在线
//         return FALSE;
// 
//     bool  bIsBoss    = false;
//     DWORD dwStaticID = pHeader->GetSID();
// 
// 
//     //----------------------------------------------------------------------------------------------------------
//     // 判断玩家退出情况，dwStaticID为请求割席断交的玩家ID， playerID为被T的玩家，两者相当表示自退
//     //---------------------------------------------------------------------------------------------------------- 
//     if ( dwStaticID != playerID ) // 踢其他人
//     {
//         if ( dwStaticID != unionData->roleid[ 0 ] ) // 如果不是老大且想T出其他兄弟，由于客户端也做
//         {                                           // 了判断的，这个条件在没有修改客户端时不会进入
//             TalkToDnid( clientID, "你不是老大, 不能踢出其他兄弟！" );
//             return FALSE;
//         }
//         else // T出兄弟
//         {
//             pItem = pHeader->FindItemByIndex( WINEID );  // 绝情酒
//             if ( pItem == NULL )
//             {
//                 TalkToDnid( clientID, "你尚没有备齐绝情酒，无法割席断交！" );
//                 return FALSE;
//             }
// 
//             msg.byType = SAKickUnionMsg::KICK_OTHER;
//         }
//     }
//     else // 老大T出自己 或者 兄弟自退
//     {
//         if ( pHeader->m_Property.m_Money < UNIONMONEY ) // 如果钱不够
//         {
//             TalkToDnid( clientID, "你的金币不足，请备齐后重试" );
//             return FALSE;
//         }
// 
//         msg.byType = SAKickUnionMsg::KICK_ME;
//     }
//    
//     if ( dwStaticID == unionData->roleid[ 0 ] )
//         bIsBoss = true;
// 
// 
//     //----------------------------------------------------------------------------------------------------------
//     // 获取被T玩家的索引，被T玩家之后的兄弟数据依次向前移动一个位置
//     //---------------------------------------------------------------------------------------------------------- 
//     int idx = -1;
//     int num = GetUnionMemberNumByPlayerID( playerID );
// 	CPlayer *delPlayer = NULL;
//     for ( int i = 0; i < num; i++ )
//     {
//         if ( unionData->roleid[ i ] == playerID )
//         {
//             idx = i;
// 			delPlayer = (CPlayer *)GetPlayerByName(unionData->szName[ i ])->DynamicCast( IID_PLAYER );
//             memcpy( msg.szName, unionData->szName[ i ], sizeof( unionData->szName[ i ] ) );
// 	        break;
//         }
//     }
// 
// 	if( idx < 0 || idx > 7 )
// 		return FALSE;
// 
// 	if( idx < 7 )
// 	{
//         memcpy( unionData->szName[ idx ], unionData->szName[ idx + 1 ], sizeof( unionData->szName[ idx + 1 ] ) * ( 7 - idx ) );
// 		memcpy( &unionData->roleid[ idx ], &unionData->roleid[ idx + 1 ], sizeof( unionData->roleid[ idx + 1 ] ) * ( 7 - idx ) );
// 		memcpy( &unionData->exp[ idx ], &unionData->exp[ idx + 1 ], sizeof( unionData->exp[ idx + 1  ] ) * ( 7 - idx ) );
// 	}
// 
// 	memset( unionData->szName[ 7 ], 0, sizeof( unionData->szName[ 7 ] ) );
// 	unionData->roleid[ 7 ] = 0;
// 	unionData->exp[ 7 ]    = 0;
//     num--;   // 成员数量减1
// 
// 	//兄弟名活动调用
// 
// 	if( delPlayer )//下线的就不必调用了
// 	{
// 		string delUnionName = GetUnionNameByPlayerID(playerID);
// 		g_Script.SetCondition( NULL, delPlayer, NULL );
// 		LuaFunctor( g_Script, "UnionPlayerLeave" )[delUnionName.c_str()]();
// 		g_Script.CleanCondition();
// 	}
// 
//     //----------------------------------------------------------------------------------------------------------
//     // 处理解散结拜，并删除结拜信息
//     //---------------------------------------------------------------------------------------------------------- 
// 	if( GetUnionMemberNumByPlayerID( playerID ) < 2 )  // 如果成员数量小于2 解除结义
// 	{
//         CPlayer *pp  = ( CPlayer* )GetPlayerBySID( unionData->roleid[ 0 ] )->DynamicCast( IID_PLAYER );
// 		
//         DeletePlayerIDFromPlayerIDMap( unionData->roleid[ 0 ] );
// 
//         if ( pp != NULL )  // 在线才发送
//         {
//             tempMsg.dwPlayerID = unionData->roleid[ 0 ];
//             tempMsg.dwBossID   = unionData->roleid[ 0 ];
//             tempMsg.byType     = SAKickUnionMsg::KICK_OTHER;   // 被强制解散了
//             g_StoreMessage( pp->m_ClientIndex, &tempMsg, sizeof( SAKickUnionMsg ) );  // 发送给最后一个玩家
//             
// 	        pp->m_Property.m_szUnionName[0] = 0;
//             pp->SendMyState(); 
// 
//             CalcAmuck( pp ); // 计算此玩家所在队伍杀气值
//             TalkToDnid( pp->m_ClientIndex, "结拜人数过少, 结拜解散！" ); // 发送给最后一个玩家
//         }
// 
// 		DeleteUnionDataByUnionName( GetUnionNameByPlayerID( playerID ) );
// 		DeletePlayerIDFromPlayerIDMap( playerID );
// 	}
// 	else
// 		DeletePlayerIDFromPlayerIDMap( playerID );
// 
// 
//     //----------------------------------------------------------------------------------------------------------
//     // 发送给被T的玩家，此玩家可以是自退或者被T
//     //---------------------------------------------------------------------------------------------------------- 
//     msg.dwPlayerID = playerID;
//     msg.dwBossID   = bIsBoss ? dwStaticID : unionData->roleid[ 0 ];
//  
//     if ( pMember != NULL )  // 当被T的兄弟在线时才发送
//     {
//         g_StoreMessage( pMember->m_ClientIndex, &msg, sizeof( SAKickUnionMsg ) );  // 发送给被T的玩家
// 	    pMember->m_Property.m_szUnionName[0] = 0;
//         pMember->SendMyState();
// 
//         CalcAmuck( pMember );
//     }
// 
// 
//     //----------------------------------------------------------------------------------------------------------
//     // 扣除道具或钱
//     //---------------------------------------------------------------------------------------------------------- 
//     if ( msg.byType == SAKickUnionMsg::KICK_ME ) // 如果是自退，扣掉自己500W 
//     {
//     	if ( pMember == NULL )
// 		    return FALSE;
//         if ( pMember->m_Property.m_Money < UNIONMONEY )
//             return FALSE;
// 
// //         SABuy buyMsg; buyMsg.nRet = 1; buyMsg.nMoney = ( pMember->m_Property.m_Money -= UNIONMONEY );
// //         g_StoreMessage( pMember->m_ClientIndex, &buyMsg, sizeof( buyMsg ) );
//     }
//     else  // 被老大T，则用掉老大一瓶绝情酒
//     {
// //         const SItemData *pData = CItem::GetItemData( WINEID );
// //         SItemBase      *pBItem = pItem;
// // 
// //         if ( pData == NULL || pItem == NULL )
// //             return FALSE;
// //         // 这里需要判断是什么类型的道具，如果是可重叠类的，那么只使用一个道具
// //         if ( ITEM_IS_OVERLAP( pData->byType ) && ( ( ( SOverlap* )pBItem )->number > 1 ) )
// //         {
// //             ( ( SOverlap * )pBItem )->number -= 1;  // 使重叠的物品少一个
// //         }
// //         else
// //             pHeader->DelItem( WINEID, "结拜收取道具" );  
//     }
// 
// 
//     //----------------------------------------------------------------------------------------------------------
//     // 发送消息到剩下的玩家
//     //----------------------------------------------------------------------------------------------------------
//     if ( num >= 2 )
//     {
//         static const char index[8][3] = { "一", "二", "三", "四", "五", "六", "七", "八" };
//         for ( int i = 0; i < num; i++ )
//         {
//             CPlayer* player = ( CPlayer* )GetPlayerBySID( unionData->roleid[ i ] )->DynamicCast( IID_PLAYER );
//             if ( player == NULL )  // 不在线是继续更新其他的
//                 continue;
// 
//             g_StoreMessage( player->m_ClientIndex, &msg, sizeof( SAKickUnionMsg ) );
// 
//             memset( player->m_Property.m_szUnionName, 0, sizeof( player->m_Property.m_szUnionName ) );
//             std::string UnionName( unionData->title );
//             UnionName += index[ i ];
//             UnionName += unionData->suffx;
//             dwt::strcpy( player->m_Property.m_szUnionName, UnionName.c_str(), sizeof( player->m_Property.m_szUnionName ) );
// 
//             UpdateUnionMember( unionData->roleid[ i ], -1 );  //  更新所有成员
// 
//             // 计算此玩家所在队伍杀气值
//             CalcAmuck( player );
//         }
//     }
   
	return TRUE;
}

void UnionDataManager::CalcAmuck( CPlayer* pPlayer )
{
//     if ( !pPlayer )
//         return;
// 
//     // 判断此玩家是否有队伍，如果有则重新计算队伍中的天罡北斗阵杀气值
//     if ( pPlayer->m_dwTeamID != 0 )
//     {
//         extern std::map< DWORD, Team > teamManagerMap;
//         std::map< DWORD, Team >::iterator iter = teamManagerMap.find( pPlayer->m_dwTeamID );
//         if ( iter != teamManagerMap.end() )
//         {
//             Team team = iter->second;
//             CalcTeamUnionAmuck( pPlayer, team );
//         }
//     }
}

void UnionDataManager::CreateUnionAddPlayer( LPCSTR title, LPCSTR suffx, DWORD id1, DWORD id2 )
{
    if ( !IsValidName( title ) )
        return;  //TalkToDnid( id1, "结义名不合法..." );

	if( !CreateUnion( title, suffx ) )
    {
        CPlayer *pPlayer = ( CPlayer* )GetPlayerBySID( id1 )->DynamicCast( IID_PLAYER );
        if ( pPlayer )
            TalkToDnid( pPlayer->m_ClientIndex, "结义名已存在或已到最大数量!" );

		return ;//TalkToDnid( id1, "你申请的结义名以存在..." );
    }

	std::string unionName(title);
	unionName += suffx;

	if( !AddPlayerToUnion( id1, unionName ) ) return;
	if( !AddPlayerToUnion( id2, unionName ) ) return;

	UpdateUnionMember( id1 );
	UpdateUnionMember( id2 );
}

/** 更新自己的信息到结义兄弟
	参数：	pMsg：填充的消息结构
			playerID: 要更新的玩家ID
*/
void UnionDataManager::UnionOnline( SUnionOnlineMsg *pMsg, DWORD playerID )
{
// 	if( !pMsg || !IsUnion( playerID ))
// 		return;
// 
// 	SPlayerUnionData *unionData = GetUnionDataByPlayerID( playerID );
// 	if( !unionData )
// 		return;
// 
// 	for( int i = 0; i < 8; i ++ )
// 	{
// 		if( unionData->roleid[ i ] > 0 && unionData->roleid[ i ] != playerID )
// 		{
// 			CPlayer *pPlayer = ( CPlayer* )GetPlayerBySID( unionData->roleid[ i ] )->DynamicCast( IID_PLAYER );
// 			if( pPlayer )
// 				g_StoreMessage( pPlayer->m_ClientIndex, pMsg, sizeof( SUnionOnlineMsg ) );
// 		}
// 		else if( unionData->roleid[ i ] != playerID )
// 			break;
// 	}
}

/** 更新某某到所有成员玩家
	参数：playerSID: 要更新的玩家ID
*/
void UnionDataManager::UnionOnline( CPlayer *pPlayer )
{
//     if ( !pPlayer )
//         return;
// 
//     DWORD SID = pPlayer->GetSID();
// 
// 	if( !IsUnion( SID ) )
// 		return;
// 
// 	SPlayerUnionData *unionData = GetUnionDataByPlayerID( SID );
// 	if( !unionData )
// 		return;
// 
// 	int idx = -1;
// 	for( int i = 0; i < 8; i ++ )
// 		if( unionData->roleid[ i ] == SID )
// 			idx = i;
// 
//     if( idx == - 1 )
//         return;
// 
//     if ( strcmp( pPlayer->m_Property.m_szUnionName, "" ) == 0 )
//     {
//         static const char index[8][3] = { "一", "二", "三", "四", "五", "六", "七", "八" };
//         std::string UnionName( unionData->title );
//         UnionName += index[idx];
//         UnionName += unionData->suffx;
// 
//         dwt::strcpy( pPlayer->m_Property.m_szUnionName, UnionName.c_str(), sizeof( pPlayer->m_Property.m_szUnionName ) );
//     }
// 
//     CalcAmuck( pPlayer );
// 
// 	for( int i = 0; i < 8; i ++ )
// 	{
// 		if( unionData->roleid[ i ] > 0 && unionData->roleid[ i ] != SID )
// 			UpdateUnionMember( unionData->roleid[i], idx );
// 		else if( unionData->roleid[ i ] != SID )
// 			break;
// 	}
}
/** 更新某个成员到某个玩家
	参数：	destID:   更新到某个晚间的ID
			idx:      如果 idx = -1 更新所有成员，反之 更新指定的成员 
*/
void UnionDataManager::UpdateUnionMember( DWORD destID, int idx )
{
// 	if( !IsUnion( destID ) || ( idx != -1 && ( idx < 0 || idx > 7 ) ) )
// 		return;
// 
// 	CPlayer *pplayer = ( CPlayer* )GetPlayerBySID( destID )->DynamicCast( IID_PLAYER );
// 	if( !pplayer )
// 		return;
// 
// 	std::string unionName = GetUnionNameByPlayerID( destID );
// 	SPlayerUnionData *unionData = GetUnionDataByPlayerID( destID );
// 	if( !unionData )
// 		return;
// 
// 	SAUpdateUnionMsg msg;	
// 	msg.type = SAUpdateUnionMsg::UPDATE_MEMBER;
// 	dwt::strcpy( msg.szName, unionName.c_str(), CONST_USERNAME );
// 	msg.change = 0;
// 
// 	int num = 0;
// 	// 更新单个
// 	if( idx != -1 )
// 	{
// 		num = 1;
// 		UnionInfo data; 
// 		memset( &data, 0, sizeof( data ) );
// 		data.rolrid = unionData->roleid[ idx ];
// 		data.exp = unionData->exp[ idx ];
// 		dwt::strcpy( data.szName, unionData->szName[ idx ], CONST_USERNAME );
// 
// 		CPlayer *pPlayer = ( CPlayer* )GetPlayerBySID( unionData->roleid[ idx ] )->DynamicCast( IID_PLAYER );
// 		if ( pPlayer )
// 		{
// 			data.school = pPlayer->m_Property.m_School;
// 			data.camp = pPlayer->m_Property.m_sXValue > 0 ? 1 : ( pPlayer->m_Property.m_sXValue == 0 ? 2 : 3 );
// 			data.job = 0;
// 			dwt::strcpy( data.szFaceionName, pPlayer->m_Property.m_szTongName, CONST_USERNAME );
// 			data.online = 1;
// 		}
// 		// 如果不在线，其它的就不用填充了
// 		else
// 			data.online = 0;
// 
// 		msg.unionInfo[ 0 ] = data;
// 
// 		msg.change |= 1 << ( 7 - idx );
// 
// 	}
// 	else
// 	{
// 		for( int i = 0; i < 8; i ++ )
// 		{
//             //  如果这里break了的话，当删除某个成员后，此成员的数据并没有即时从客户端抹去
// // 			if( unionData->roleid[i] == 0 )  
// // 				break;
// 
// 			UnionInfo data;
// 			memset( &data, 0, sizeof( data ) );
// 			data.rolrid = unionData->roleid[ i ];
// 			data.exp = unionData->exp[ i ];
// 			dwt::strcpy( data.szName, unionData->szName[ i ], CONST_USERNAME );
// 
// 			CPlayer *pPlayer = ( CPlayer* )GetPlayerBySID( unionData->roleid[ i ] )->DynamicCast( IID_PLAYER );
// 			if ( pPlayer )
// 			{
// 				data.school = pPlayer->m_Property.m_School;
// 				data.camp = pPlayer->m_Property.m_sXValue > 0 ? 1 : ( pPlayer->m_Property.m_sXValue == 0 ? 2 : 3 );
// 				data.job = 0;
// 				dwt::strcpy( data.szFaceionName, pPlayer->m_Property.m_szTongName, CONST_USERNAME );
// 				data.online = 1;
// 			}
// 			// 如果不在线，其它的就不用填充了
// 			else
// 				data.online = 0;
// 
// 			msg.unionInfo[ i ] = data;
// 
// 			msg.change |= 1 << ( 7 - i );
// 			num ++;
// 		}
// 	}
// 
// 	g_StoreMessage( pplayer->m_ClientIndex, &msg, sizeof( SAUpdateUnionMsg ) - sizeof( UnionInfo ) * ( 8 - num ) );
//     pplayer->SendMyState();
}


void UnionDataManager::ShowPlayerTitle( DWORD sid )
{
    if ( sid == 0 )
        return;

    std::string str = GetUnionNameByPlayerID( sid );
    rfalse( 2, 1, "玩家id:%d   结义名称:%s", sid, str.c_str() );
}
void UnionDataManager::ShowUnionInfo( LPCSTR key )
{
//     if ( key == NULL )
//         return ;
// 
//     SPlayerUnionData *p = GetUnionDataByUnionName( std::string ( key ) );
//     if ( p == NULL )
//     {
//         rfalse( 2, 1, "未找到【%s】的结义数据", key );
//         return ;
//      
//     }
// 
//     for ( int i = 0; i < SPlayerUnionData::NUMBER_MAX; i++ )
//     {
//         // 名字或sid为空
//         if ( p->roleid[ i ] == 0 || p->szName[ i ][ 0 ] == NULL )
//             continue;
// 
//         rfalse( 2, 1, "玩家id:%d,玩家名:%s 经验:%d", p->roleid[ i ], p->szName[ i ], p->exp[ i ] );
//     }
//     
}

void UnionDataManager::ReceiveExp( DWORD sid )
{
//     CPlayer *player = ( CPlayer* )GetPlayerBySID( sid )->DynamicCast( IID_PLAYER );
//     if ( player == NULL )
//         return ;
// 
//     SPlayerUnionData *data = GetUnionDataByPlayerID( sid );
//     if ( data == NULL )
//         return ;
// 
//     BYTE index = -1;
//     for ( int i = 0; i < SPlayerUnionData::NUMBER_MAX; i++ )
//     {
//         // 只要有一个未满就不能领取
//         if ( ( data->roleid[i] > 0 ) && ( data->exp[i] < unionMaxExp ) )
//             return ;
// 
//         if ( data->roleid[i] && data->roleid[i] == sid )
//             index = i;
//     }
// 
//     if ( index >= 8 )
//         return ;
// 
//     if ( data->exp[index] != unionMaxExp )
//         return ;
// 
//     //player->SendAddPlayerExp( unionMaxExp, 0, FALSE, 0, "结义");
//     data->exp[index] = -1;
//     SAUpdateExpMsg msg;
//     msg.exp = data->exp[index];
//     msg.memberIdx = index;
//     msg.type = SAUpdateUnionMsg::UPDATE_EXP;
//     SendUnionAllMsg( data, &msg, sizeof( msg )  );
}