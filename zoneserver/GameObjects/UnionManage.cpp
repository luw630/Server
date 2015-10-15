
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
//     // ÿ��Ķ�ʱ���
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
//             // ���SIZE������ô�ͷǷ���
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
//   //      rfalse( 2, 1, "��ȡ�������ݳɹ�,%d��", m_UnionDataMap.size() );
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
//     // ֻ��õ�ǰ�����10%
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
//         // ������д�����-1�ľ�˵����Ҷ��Ѿ����������ˣ���������Ӿ���
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
//             // ͬ�����鵽�ͻ���...
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
/** ����Ƿ��Ѿ�����
	������	playerID: ���ID
	���أ�	TRUE:
			FALSE:
*/
BOOL UnionDataManager::IsUnion( DWORD playerID )
{
	std::map< DWORD, std::string >::iterator it = unionMappedMap.find( playerID );
	if( it != unionMappedMap.end() )
		return TRUE;

	return FALSE;
}

/** �ӽ�ɫID��ɾ��
	������	playerID: ��ɫID
*/
void UnionDataManager::DeletePlayerIDFromPlayerIDMap( DWORD playerID )
{
	std::map< DWORD, std::string >::iterator it = unionMappedMap.find( playerID );
	if( it != unionMappedMap.end() )
		unionMappedMap.erase( it );
}

/** ��ȡ��ɫ���ڵĽ�����
	������	playerID: ��ɫID
*/
std::string UnionDataManager::GetUnionNameByPlayerID( DWORD playerID )
{
	std::string str;
	std::map< DWORD, std::string >::iterator it = unionMappedMap.find( playerID );
	if( it != unionMappedMap.end() )
		str = it->second;

	return str;
}

/** ͨ�����ID��ȡ������Ϣ
	������	playerID: ���ID
	���أ�	NULL: ������
			�ǿգ�������Ϣ
*/
SPlayerUnionData* UnionDataManager::GetUnionDataByPlayerID( DWORD playerID )
{
	std::map< DWORD, std::string >::iterator it = unionMappedMap.find( playerID );
	if( it == unionMappedMap.end() )
		return NULL;

	return GetUnionDataByUnionName( it->second );
}

/** ��ȡ����ĳ�Ա����ͨ�����ID
	������	���� tile
	���أ�	��Ա����
*/
int UnionDataManager::GetUnionMemberNumByPlayerID( DWORD playerID )
{
	std::map< DWORD, std::string >::iterator it = unionMappedMap.find( playerID );
	if( it == unionMappedMap.end() )
		return -1;

	return GetUnionMemberNumByUnionName( it->second );
}
// *********************************************** B ************************************** //
/** �Ƿ��Ѿ����ڽ�������
	������	unionName: �µĽ�����
	���أ�	TRUE:
			FALSE:
*/
BOOL UnionDataManager::IsHaveUnion( std::string unionName )
{
	std::map< std::string, SPlayerUnionData >::iterator it =  m_UnionDataMap.find( unionName );
	if( it != m_UnionDataMap.end() )
		return TRUE;

	return FALSE;
}

/** ͨ����������ȡ������Ϣ
	������	unionName: ������
	���أ�	NULL: ������
			�ǿգ�������Ϣ
*/
SPlayerUnionData* UnionDataManager::GetUnionDataByUnionName( std::string unionName )
{
	std::map< std::string, SPlayerUnionData >::iterator it = m_UnionDataMap.find( unionName );
	if( it == m_UnionDataMap.end() )
		return NULL;

	return &it->second;
}

/** ��ȡ����ĳ�Ա����
	������	���� tile
	���أ�	��Ա����
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
// 		if( it->second.roleid[ i ] > 0 )  // ���ﲻ��Ҫ�ж�������
// 			num ++;
// 	}
// 
// 	return num;
	return -1;
}

/** ɾ������ͨ��������
	������	���� tile
	���أ�	��Ա����
*/
void UnionDataManager::DeleteUnionDataByUnionName( std::string unionName )
{
	std::map< std::string, SPlayerUnionData >::iterator it = m_UnionDataMap.find( unionName );
	if( it != m_UnionDataMap.end() )
	{
		//�ֵ�������������õ�
		g_Script.SetCondition( NULL, NULL, NULL );
		LuaFunctor( g_Script, "UnionSignUpDelete" )[unionName.c_str()]();
		g_Script.CleanCondition();

		m_UnionDataMap.erase( it );

	}
}
// *********************************************** interface ************************************** //
/** �����µĽ���, ���� title �� suffx �Ľ��
	������	���� tile
	���أ�	TRUE:
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

/** ���һ����ɫ��ָ���Ľ��壬��Ÿ����Ⱥ�˳���Զ�����
	������	playerID: ��ɫID
			unionName: ������
	���أ�	TRUE:
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
// 	// ����Ͳ������ظ��ж��ˣ���Ϊǰ���Ѿ����˴���
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
//         TalkToUnionMember( unionName, FormatString( "[%s]��Ϊ�����ǵ��ֵ�!!", pPlayer->GetName() ) );

	return TRUE;
}

/** �ӽ�����ɾ��һ����ɫ������Զ���ǰ��£
	�����������С��2 �Զ���ɢ����֪ͨ������һ�����
	������	playerID����T���ֵ�ID
            ClientID�������ϯ�Ͻ����ֵ�
	���أ�	TRUE:
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
//     const DWORD    UNIONMONEY  = 5000000;  // �˳������Ҫ���ĵĽ��500W
//     const WORD     WINEID      = 40101;    // ����Ʊ��
//     SPackageItem*  pItem       = NULL;
// 
//     CPlayer* pHeader = ( CPlayer* )GetPlayerByDnid( clientID )->DynamicCast( IID_PLAYER );
//     CPlayer* pMember = ( CPlayer* )GetPlayerBySID( playerID )->DynamicCast( IID_PLAYER );
//     if ( pHeader == NULL ) // ����û���ж�pMember������Ϊ��T����ҿ��ܲ�����
//         return FALSE;
// 
//     bool  bIsBoss    = false;
//     DWORD dwStaticID = pHeader->GetSID();
// 
// 
//     //----------------------------------------------------------------------------------------------------------
//     // �ж�����˳������dwStaticIDΪ�����ϯ�Ͻ������ID�� playerIDΪ��T����ң������൱��ʾ����
//     //---------------------------------------------------------------------------------------------------------- 
//     if ( dwStaticID != playerID ) // ��������
//     {
//         if ( dwStaticID != unionData->roleid[ 0 ] ) // ��������ϴ�����T�������ֵܣ����ڿͻ���Ҳ��
//         {                                           // ���жϵģ����������û���޸Ŀͻ���ʱ�������
//             TalkToDnid( clientID, "�㲻���ϴ�, �����߳������ֵܣ�" );
//             return FALSE;
//         }
//         else // T���ֵ�
//         {
//             pItem = pHeader->FindItemByIndex( WINEID );  // �����
//             if ( pItem == NULL )
//             {
//                 TalkToDnid( clientID, "����û�б������ƣ��޷���ϯ�Ͻ���" );
//                 return FALSE;
//             }
// 
//             msg.byType = SAKickUnionMsg::KICK_OTHER;
//         }
//     }
//     else // �ϴ�T���Լ� ���� �ֵ�����
//     {
//         if ( pHeader->m_Property.m_Money < UNIONMONEY ) // ���Ǯ����
//         {
//             TalkToDnid( clientID, "��Ľ�Ҳ��㣬�뱸�������" );
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
//     // ��ȡ��T��ҵ���������T���֮����ֵ�����������ǰ�ƶ�һ��λ��
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
//     num--;   // ��Ա������1
// 
// 	//�ֵ��������
// 
// 	if( delPlayer )//���ߵľͲ��ص�����
// 	{
// 		string delUnionName = GetUnionNameByPlayerID(playerID);
// 		g_Script.SetCondition( NULL, delPlayer, NULL );
// 		LuaFunctor( g_Script, "UnionPlayerLeave" )[delUnionName.c_str()]();
// 		g_Script.CleanCondition();
// 	}
// 
//     //----------------------------------------------------------------------------------------------------------
//     // �����ɢ��ݣ���ɾ�������Ϣ
//     //---------------------------------------------------------------------------------------------------------- 
// 	if( GetUnionMemberNumByPlayerID( playerID ) < 2 )  // �����Ա����С��2 �������
// 	{
//         CPlayer *pp  = ( CPlayer* )GetPlayerBySID( unionData->roleid[ 0 ] )->DynamicCast( IID_PLAYER );
// 		
//         DeletePlayerIDFromPlayerIDMap( unionData->roleid[ 0 ] );
// 
//         if ( pp != NULL )  // ���߲ŷ���
//         {
//             tempMsg.dwPlayerID = unionData->roleid[ 0 ];
//             tempMsg.dwBossID   = unionData->roleid[ 0 ];
//             tempMsg.byType     = SAKickUnionMsg::KICK_OTHER;   // ��ǿ�ƽ�ɢ��
//             g_StoreMessage( pp->m_ClientIndex, &tempMsg, sizeof( SAKickUnionMsg ) );  // ���͸����һ�����
//             
// 	        pp->m_Property.m_szUnionName[0] = 0;
//             pp->SendMyState(); 
// 
//             CalcAmuck( pp ); // �����������ڶ���ɱ��ֵ
//             TalkToDnid( pp->m_ClientIndex, "�����������, ��ݽ�ɢ��" ); // ���͸����һ�����
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
//     // ���͸���T����ң�����ҿ��������˻��߱�T
//     //---------------------------------------------------------------------------------------------------------- 
//     msg.dwPlayerID = playerID;
//     msg.dwBossID   = bIsBoss ? dwStaticID : unionData->roleid[ 0 ];
//  
//     if ( pMember != NULL )  // ����T���ֵ�����ʱ�ŷ���
//     {
//         g_StoreMessage( pMember->m_ClientIndex, &msg, sizeof( SAKickUnionMsg ) );  // ���͸���T�����
// 	    pMember->m_Property.m_szUnionName[0] = 0;
//         pMember->SendMyState();
// 
//         CalcAmuck( pMember );
//     }
// 
// 
//     //----------------------------------------------------------------------------------------------------------
//     // �۳����߻�Ǯ
//     //---------------------------------------------------------------------------------------------------------- 
//     if ( msg.byType == SAKickUnionMsg::KICK_ME ) // ��������ˣ��۵��Լ�500W 
//     {
//     	if ( pMember == NULL )
// 		    return FALSE;
//         if ( pMember->m_Property.m_Money < UNIONMONEY )
//             return FALSE;
// 
// //         SABuy buyMsg; buyMsg.nRet = 1; buyMsg.nMoney = ( pMember->m_Property.m_Money -= UNIONMONEY );
// //         g_StoreMessage( pMember->m_ClientIndex, &buyMsg, sizeof( buyMsg ) );
//     }
//     else  // ���ϴ�T�����õ��ϴ�һƿ�����
//     {
// //         const SItemData *pData = CItem::GetItemData( WINEID );
// //         SItemBase      *pBItem = pItem;
// // 
// //         if ( pData == NULL || pItem == NULL )
// //             return FALSE;
// //         // ������Ҫ�ж���ʲô���͵ĵ��ߣ�����ǿ��ص���ģ���ôֻʹ��һ������
// //         if ( ITEM_IS_OVERLAP( pData->byType ) && ( ( ( SOverlap* )pBItem )->number > 1 ) )
// //         {
// //             ( ( SOverlap * )pBItem )->number -= 1;  // ʹ�ص�����Ʒ��һ��
// //         }
// //         else
// //             pHeader->DelItem( WINEID, "�����ȡ����" );  
//     }
// 
// 
//     //----------------------------------------------------------------------------------------------------------
//     // ������Ϣ��ʣ�µ����
//     //----------------------------------------------------------------------------------------------------------
//     if ( num >= 2 )
//     {
//         static const char index[8][3] = { "һ", "��", "��", "��", "��", "��", "��", "��" };
//         for ( int i = 0; i < num; i++ )
//         {
//             CPlayer* player = ( CPlayer* )GetPlayerBySID( unionData->roleid[ i ] )->DynamicCast( IID_PLAYER );
//             if ( player == NULL )  // �������Ǽ�������������
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
//             UpdateUnionMember( unionData->roleid[ i ], -1 );  //  �������г�Ա
// 
//             // �����������ڶ���ɱ��ֵ
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
//     // �жϴ�����Ƿ��ж��飬����������¼�������е��������ɱ��ֵ
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
        return;  //TalkToDnid( id1, "���������Ϸ�..." );

	if( !CreateUnion( title, suffx ) )
    {
        CPlayer *pPlayer = ( CPlayer* )GetPlayerBySID( id1 )->DynamicCast( IID_PLAYER );
        if ( pPlayer )
            TalkToDnid( pPlayer->m_ClientIndex, "�������Ѵ��ڻ��ѵ��������!" );

		return ;//TalkToDnid( id1, "������Ľ������Դ���..." );
    }

	std::string unionName(title);
	unionName += suffx;

	if( !AddPlayerToUnion( id1, unionName ) ) return;
	if( !AddPlayerToUnion( id2, unionName ) ) return;

	UpdateUnionMember( id1 );
	UpdateUnionMember( id2 );
}

/** �����Լ�����Ϣ�������ֵ�
	������	pMsg��������Ϣ�ṹ
			playerID: Ҫ���µ����ID
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

/** ����ĳĳ�����г�Ա���
	������playerSID: Ҫ���µ����ID
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
//         static const char index[8][3] = { "һ", "��", "��", "��", "��", "��", "��", "��" };
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
/** ����ĳ����Ա��ĳ�����
	������	destID:   ���µ�ĳ������ID
			idx:      ��� idx = -1 �������г�Ա����֮ ����ָ���ĳ�Ա 
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
// 	// ���µ���
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
// 		// ��������ߣ������ľͲ��������
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
//             //  �������break�˵Ļ�����ɾ��ĳ����Ա�󣬴˳�Ա�����ݲ�û�м�ʱ�ӿͻ���Ĩȥ
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
// 			// ��������ߣ������ľͲ��������
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
    rfalse( 2, 1, "���id:%d   ��������:%s", sid, str.c_str() );
}
void UnionDataManager::ShowUnionInfo( LPCSTR key )
{
//     if ( key == NULL )
//         return ;
// 
//     SPlayerUnionData *p = GetUnionDataByUnionName( std::string ( key ) );
//     if ( p == NULL )
//     {
//         rfalse( 2, 1, "δ�ҵ���%s���Ľ�������", key );
//         return ;
//      
//     }
// 
//     for ( int i = 0; i < SPlayerUnionData::NUMBER_MAX; i++ )
//     {
//         // ���ֻ�sidΪ��
//         if ( p->roleid[ i ] == 0 || p->szName[ i ][ 0 ] == NULL )
//             continue;
// 
//         rfalse( 2, 1, "���id:%d,�����:%s ����:%d", p->roleid[ i ], p->szName[ i ], p->exp[ i ] );
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
//         // ֻҪ��һ��δ���Ͳ�����ȡ
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
//     //player->SendAddPlayerExp( unionMaxExp, 0, FALSE, 0, "����");
//     data->exp[index] = -1;
//     SAUpdateExpMsg msg;
//     msg.exp = data->exp[index];
//     msg.memberIdx = index;
//     msg.type = SAUpdateUnionMsg::UPDATE_EXP;
//     SendUnionAllMsg( data, &msg, sizeof( msg )  );
}