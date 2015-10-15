#include "StdAfx.h"
#include "time.h"
#include "PlayerDataCache.h"
#include "LITESERIALIZER/Lite.h"

#include "NetworkModule/DataMsgs.h"
#include "NetDBModule/NetDBModule.h"
#include "pub/Thread.h"

extern BOOL SendMessage( DNID dnidClient, LPCVOID lpMsg, size_t wSize );

const int HANGUPSAVETIME = 1000 * 60 * 60; // 1个小时保存一次挂机数据
const int ONLINESAVETIME = 1000 * 60 * 24; // 在线数据24分钟保存一次 退出时数据直接保存

typedef UGE::CriticalLock::Barrier4ReadWrite Barrier;

void LogMsg( LPCSTR filename, LPCSTR szLogMessage, int ex = -1 )
{
    try
    {
        SYSTEMTIME s;
        char szLogTime[128];

        GetLocalTime(&s);
        sprintf(szLogTime, "[%d-%d-%d %d:%d:%d][%d]", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond, ex );

        HANDLE hFile = ::CreateFile( filename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, NULL, NULL );
        if ( hFile == INVALID_HANDLE_VALUE )
            return;

        DWORD sizeA = (DWORD)strlen( szLogTime );
        DWORD sizeB = (DWORD)strlen( szLogMessage );

        SetFilePointer( hFile, 0, 0, FILE_END );
        WriteFile( hFile, szLogTime, sizeA, &sizeA, 0 );
        WriteFile( hFile, szLogMessage, sizeB, &sizeB, 0 );
        WriteFile( hFile, "\r\n", 2, &sizeA, 0 );

        CloseHandle( hFile);
    }
    catch ( ... )
    {
    }
}

#define LogXcptMsg( str ) LogMsg( "debugXcpt.txt", str )

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void PlayerDataCache::GetCharacterList( Query &query, DNID dnid, SQGetCharacterListMsg *pMsg )
{
    SAGetCharacterListMsg msg;

    msg.qwSerialNo = pMsg->qwSerialNo;
    msg.dnidClient = pMsg->dnidClient;

    try
    {
        lite::Serialreader ls( pMsg->streamData );
        LPCSTR account = ls();
        
        lite::Serializer lsm( msg.streamData, sizeof( msg.streamData ) );
        lsm( account );

        // 直接获取角色列表，并为该帐号加锁，如果该帐号已经被其他服务器锁定，则获取将会失败！
        msg.byResult = GetCharacterList( query, pMsg->dwServerId, account, msg.CharListData );
        
        SendMessage( dnid, &msg, sizeof( msg ) - lsm.EndEdition() );
    }
    catch( lite::Xcpt &e )
    {
        LogXcptMsg( e.GetErrInfo() );
    }
}

void PlayerDataCache::DeleteCharacter( Query &query, DNID dnid, SQDeleteCharacterMsg *pMsg )
{
    SADeleteCharacterMsg msg;

    msg.qwSerialNo = pMsg->qwSerialNo;
    msg.dnidClient = pMsg->dnidClient;

    try
    {
        lite::Serialreader ls( pMsg->streamData );
        LPCSTR account = ls();

        msg.dwStaticID = pMsg->dwStaticID;

        // 删除角色，也直接处理，如果已经被其他服务器锁定，则操作将失败
        // 如果目标地点没有角色，操作也将失败
        // 如果目标角色的帐号归属和当前帐号不一致，操作也将失败
        msg.byResult = DeleteCharacter( query, pMsg->dwServerId, account, pMsg->dwStaticID, msg.szName );

        SendMessage( dnid, &msg, sizeof(msg) );
    }
    catch( lite::Xcpt &e )
    {
        LogXcptMsg( e.GetErrInfo() );
    }
}

void PlayerDataCache::CreateCharacter( Query &query, DNID dnid, SQCreatePlayerMsg *pMsg )
{
    SACreatePlayerMsg msg;

    msg.qwSerialNo = pMsg->qwSerialNo;
    msg.dnidClient = pMsg->dnidClient;

    try
    {
        lite::Serialreader ls( pMsg->streamData );
        LPCSTR account = ls();

        msg.m_byIndex = pMsg->m_byIndex;

        // 创建角色，也直接处理，如果已经被其他服务器锁定，则操作将失败
        // 如果目标地点已经存在角色，操作也将失败
        // 如果角色名称重复，操作也将失败
        msg.byResult = CreateCharacter( query, pMsg->dwServerId, account, pMsg->m_byIndex, pMsg->PlayerData );

        msg.PlayerData = pMsg->PlayerData;
        SendMessage( dnid, &msg, sizeof(msg) );
    }
    catch( lite::Xcpt &e )
    {
        LogXcptMsg( e.GetErrInfo() );
    }
}

void PlayerDataCache::GetCharacter( Query &query, DNID dnid, SQGetCharacterMsg *pMsg )
{
    SAGetCharacterMsg msg;

    msg.qwSerialNo = pMsg->qwSerialNo;
    msg.dnidClient = pMsg->dnidClient;

    try
    {
        lite::Serialreader ls( pMsg->streamData );
        LPCSTR account = ls();

        lite::Serializer lsm( msg.streamData, sizeof( msg.streamData ) );
        lsm( account );

        // 获取角色数据，如果没有目标角色，操作将失败！
        msg.byResult = GetCharacter( query, pMsg->dwServerId, account, pMsg->byIndex, msg.PlayerData );

        SendMessage( dnid, &msg, sizeof(msg) - lsm.EndEdition() );
    }
    catch( lite::Xcpt &e )
    {
        LogXcptMsg( e.GetErrInfo() );
    }
}

void PlayerDataCache::SaveCharacter( Query &query, DNID dnid, SQSaveCharacterMsg *pMsg )
{
    SASaveCharacterMsg msg;

    msg.qwSerialNo = pMsg->qwSerialNo;
    msg.dnidClient = pMsg->dnidClient;

    try
    {
        lite::Serialreader ls( pMsg->streamData );
        LPCSTR account = ls();

        lite::Serializer lsm( msg.streamData, sizeof( msg.streamData ) );
        lsm( account );

        // 。。。保存，保存时根据状态，确认究竟是只是保存，还是保存退出（并销毁缓存对象同时解锁）
        msg.byResult = SaveCharacter( query, pMsg->dwServerId, account, pMsg->PlayerData );

        SendMessage( dnid, &msg, sizeof(msg) - lsm.EndEdition() );
    }
    catch( lite::Xcpt &e )
    {
        LogXcptMsg( e.GetErrInfo() );
    }
}

////----------------------LocateAndLock----------------------
//意图: 通过账号来锁定DataCache的操作
//参数说明：
// bNeedCreate 是否需要在容器对像里找不到指定账号的时候创建数据
//返回值说明：返回 
//-----------------------------------------------------------
BOOL PlayerDataCache::LocateAndLock( LPCSTR account, AccountData &*ptr, 
    UGE::CriticalLock::Barrier4ReadWrite &barrier, bool bNeedCreate )
{
    // 默认传入的 ptr 是一个空的智能指针！
    assert( !ptr );

__retry:
    LocateObject( account, ptr );

    if ( !ptr )
    {
        // 如果不需要创建，并且也没有找到这个对象，则失败返回！
        if ( !bNeedCreate )
            return FALSE;

        // 新建指定账号对象！不能覆盖已有的数据，如果出现覆盖就说明其他地方已经成功创建了ptr对象数据库
        static AccountData ssad;
        if ( !InsertObject( ssad, account, false ) )
            return FALSE;

        // 重来
        goto __retry;
    }

    // 为获取的对象加锁，并测试删除状态
    barrier.Assign( ptr );
    if ( ptr->delState == AccountData::DT_DELETED )
    {
        barrier.Assign( NULL );
        ptr = NULL;
        return FALSE;
    }

    // 如果成功获取并通过，则取消这个对象的删除状态！
    ptr->delState = AccountData::DT_NULL;

    return TRUE;
}

// 获取角色的列表数据，如果数据在Cache中不存在，则会从数据库中获取
int PlayerDataCache::GetCharacterList( Query &query, DWORD serverId, LPCSTR account, SCharListData data[3] )
{
    // 获取角色列表，并不会直接操作到缓存（特指新建该帐号对应的缓存对象）
    memset( data, 0, sizeof( SCharListData ) * 3 );

    // 直接到数据库中获取角色列表，并尝试加锁，如果加锁失败，则说明该帐号已经被其他服务器锁定！
    int ret = reinterpret_cast< CNetDBModule * >( &query )->GetPlayerCharList( serverId, account, data );

    // 在获取前未被任何服务器锁定，所以角色列表获取成功！
    if ( ret == 0 ) 
        return SAGetCharacterListMsg::RET_SUCCESS;

    else if ( ret != serverId ) // 意外的情况，该帐号正在被当前服务器锁定！
    {
        // 到缓存中尝试查找，如果没找到，说明可能是当前的缓存服务器挂了。。。那就可以正常反回了
        if ( !LocateTest( account ) ) 
            return SAGetCharacterListMsg::RET_SUCCESS;

        // 如果找到，说明确实被锁定，可能是游戏逻辑层崩溃导致缓存被遗留！
        // 返回被自己锁定的消息，并且让登陆服务器再一次确认是否角色重复，并根据状况请求解除锁定！
        return SAGetCharacterListMsg::RET_LOCKEDSELF; 
    }

    // 这里是被其他服务器锁定了，所以，其他后续的操作就不该继续进行了
    return SAGetCharacterListMsg::RET_LOCKEDOTHER;
}

int PlayerDataCache::DeleteCharacter( Query &query, DWORD serverId, LPCSTR account, DWORD staticId, char name[11] )
{
    // 删除角色时，该帐号数据一定不该存在于缓存中！因为在之前仅仅是获取角色列表，并没有操作缓存的动作！
    if ( LocateTest( account ) ) 
        return FALSE;

    // 直接到数据库中获取角色列表，并尝试加锁，如果加锁失败，则说明该帐号已经被其他服务器锁定！
    int ret = reinterpret_cast< CNetDBModule * >( &query )->DeletePlayer( serverId, account, staticId );

    // 在获取前未被任何服务器锁定，所以角色列表获取成功！
    if ( ret == 0 ) 
        return SAGetCharacterListMsg::RET_SUCCESS;

    else if ( ret != serverId ) // 意外的情况，该帐号正在被当前服务器锁定！
    {
        // 到缓存中尝试查找，如果没找到，说明可能是当前的缓存服务器挂了。。。那就可以正常反回了
        if ( !LocateTest( account ) ) 
            return SAGetCharacterListMsg::RET_SUCCESS;

        // 如果找到，说明确实被锁定，可能是游戏逻辑层崩溃导致缓存被遗留！
        // 返回被自己锁定的消息，并且让登陆服务器再一次确认是否角色重复，并根据状况请求解除锁定！
        return SAGetCharacterListMsg::RET_LOCKEDSELF; 
    }

    // 这里是被其他服务器锁定了，所以，其他后续的操作就不该继续进行了
    return SAGetCharacterListMsg::RET_LOCKEDOTHER;

    // 遍历后未找到指定对象失败返回！
    return FALSE;
}

int PlayerDataCache::CreateCharacter( Query &query, DWORD serverId, 
    LPCSTR account, int index, SCreateFixProperty &data )
{
    index -= 1;

    if ( ( index < 0 ) || ( index > 2 ) )
        return FALSE;

    boost::shared_ptr< AccountData > accdata;
    Barrier barrier;

    if ( !LocateAndLock( account, accdata, barrier ) )
    {
        // 如果锁定失败，或者找不到指定账号数据，则直接返回！
        return FALSE;
    }

    if ( ( accdata->account.empty() ) ||
        ( accdata->server_id != serverId ) ||
        ( accdata->players[index].static_id != 0 ) )
    {
        // 还未初始化，失败返回
        // 发起请求的服务器和当前玩家请求的服务器不同
        // 指定位置上已经存在角色，无法覆盖创建
        return FALSE;
    }

    // 向数据库请求创建！操作成功后填充数据！
    int ret = reinterpret_cast< CNetDBModule * >( &query )->CreatePlayerData( account, index + 1, data );
    if ( ret == 1 )
    {
        accdata->players[index].static_id = data.m_dwStaticID;
        accdata->players[index].base.m_byBRON = data.m_byBRON;
        accdata->players[index].base.m_bySchool = data.m_bySchool;
        accdata->players[index].base.m_bySex = data.m_bySex;
        accdata->players[index].base.m_byLevel = 1;
        memcpy( accdata->players[index].base.m_szName, data.m_szName, sizeof( data.m_szName ) );
    }

    // 创建角色失败！
    return ret;    
}

int PlayerDataCache::SaveCharacter( Query &query, DWORD serverId, LPCSTR account,const SFixProperty &data )
{
    // 暂时不考虑由网络消息来触发保存事件！
    char str[1024] = "";
    boost::shared_ptr< AccountData > accdata;
    Barrier barrier;

    //这种情况只会是DBServer异常关闭造成了登录服务器上有玩家数据，DBServer的Cache中没有数据
    if ( !LocateAndLock( account, accdata, barrier ) )
    {     
        // 现作修改！让重启动后的数据库，也可以接受已经存在的账号服务器数据重联！
        SCharListData tmpdata[3];
        int ret = GetCharacterList( query, serverId, account, tmpdata );

        sprintf( str, "serverId = %d, account = %s [%s]", serverId, account, ret ? "fail" : "succ" );
        LogMsg( "rebind.txt", str );

        if ( ret != SAGetCharacterListMsg::RET_SUCCESS )
            return false;

        // 再次尝试获取，如果还是失败则放弃保存……
        if ( !LocateAndLock( account, accdata, barrier ) )
            return false;
    }

    if ( serverId == accdata->server_id )
    {
        for ( int i=0; i<3; i++ )
        {
            if ( accdata->players[i].static_id == data.m_dwStaticID )
			{
				accdata->players[i].base = data;
                accdata->players[i].package = data;
                accdata->players[i].tasks = data;
                accdata->players[i].relation = data;
                // GM相关数据不参与保存！

                memcpy( accdata->depot[0], &(SFixStorage1&)data, sizeof( accdata->depot[0] ) );
                memcpy( accdata->depot[1], &(SFixStorage2&)data, sizeof( accdata->depot[1] ) );
                memcpy( accdata->depot[2], &(SFixStorage3&)data, sizeof( accdata->depot[2] ) );

                // 将该账号仓库数据标记为已修改，用来作为在将来某个时刻保存到数据库中的判断依据
                accdata->warehousechanged = true;
                accdata->warehouseloaded = true;

                // 用来作为在将来某个时刻保存到数据库中的判断依据
                accdata->byOperaterType[i] = data.m_byStoreFlag;
                switch ( accdata->byOperaterType[i] )
                {
                case SFixProperty::ST_HANGUP :
                    if ( ( int )( timeGetTime() - accdata->dwLastSaveTime ) > HANGUPSAVETIME )
                        PushIntoSaveQueue( accdata, FALSE );
                    break;

                case SFixProperty::ST_LOGIN:
                    if ( ( int )( timeGetTime() - accdata->dwLastSaveTime ) > ONLINESAVETIME )
                        PushIntoSaveQueue( accdata, FALSE );
                    break;

                case SFixProperty::ST_LOGOUT:
                    PushIntoSaveQueue( accdata );
                    break;

                default:
                    sprintf( str, "default save except account = %s, gid = %d", account, data.m_dwStaticID );
                    LogMsg( "error.txt", str, __LINE__ );
                }

                return true;
            }
        }

        // 错误！未找到指定存储的角色！！！
        sprintf( str, "find failure account = %s, gid = %d", account, data.m_dwStaticID );
        LogMsg( "error.txt", str, __LINE__ );
    }
    else
    {
        // 错误！严重错误！！！！！！！！！！！！！！！！！！
        // 数据库收到不同[多个]游戏服务器过来的储存请求！！！
        // 会造成复制！！！！！！！！！！！！！！！！！！！！
        sprintf( str, "sid = %d nsid = %d, account = %s", accdata->server_id, serverId, account );
        LogMsg( "error.txt", str, __LINE__ );
    }

    return false;
}

int PlayerDataCache::GetCharacter( Query &query, DWORD serverId, LPCSTR account, int index, SFixProperty &data )
{
    index -= 1;

    if ( ( index < 0 ) || ( index > 2 ) )
        return false;

    boost::shared_ptr< AccountData > accdata;
    Barrier barrier;

    if ( !LocateAndLock( account, accdata, barrier ) )
    {
        // 如果锁定失败，或者找不到指定账号数据，则直接返回！
        return false;
    }

    if ( ( accdata->account.empty() ) || 
        ( accdata->server_id != serverId ) || 
        ( accdata->players[index].static_id == 0 ) )
    {
        // 账号还未初始化，失败返回
        // 该账号还存在[活跃]于某个服务器上(且不是当前挂机的服务器)，所以不能再次获取角色数据[登陆]
        // 该账号不存在指定角色！
        return false;
    }

    // 如果仓库未获取，则先向数据库请求获取
    if ( !accdata->warehouseloaded )
    {
        if ( !reinterpret_cast< CNetDBModule * >( &query )->
            GetWareHouseData( account, accdata->depot ) )
        {
            // 获取仓库数据失败！
            return false;
        }

        accdata->warehouseloaded = true;
    }

    if ( accdata->players[index].base.m_dwBaseHP == 0 )
    {
        // 该角色角色还未获取，需即时取回数据
        if ( !reinterpret_cast< CNetDBModule * >( &query )->
            GetCharacterData( account, index + 1, 
            accdata->players[index].base, 
            accdata->players[index].package, 
            accdata->players[index].tasks, 
            accdata->players[index].relation,
			serverId ) )
        {
            // 获取角色数据失败！
            accdata->players[index].base.m_dwBaseHP = 0;
            return false;
        }

        // 强制将GM等级置为0
        accdata->players[index].gm.m_wGMLevel = 0;

        // 从玩家获取角色开始计时!
        {
            AccountTimeSet temp;
            if ( !limitManager.LocateObject( account, temp ) )
            {
                // 初始化新帐号对应的计时数据!
                temp.onlineTime = 1;
                temp.offlineTime = 0;
                temp.offlineSegment = ( DWORD )time( NULL );
            }

            // 将在线时长传递给游戏世界服务器
            accdata->players[index].base.m_pTroopsSkills[0].dwProficiency = temp.onlineTime;
            accdata->players[index].base.m_pTroopsSkills[1].dwProficiency = temp.offlineTime;
            accdata->players[index].base.m_pTroopsSkills[2].dwProficiency = temp.offlineSegment;
        }
    }

    // 拷贝缓存的数据到返回的数据区
    data.m_dwStaticID       = accdata->players[index].static_id;

    (SFixBaseData&)data     = accdata->players[index].base;
    (SFixPackage&)data      = accdata->players[index].package;
    (SPlayerTasks&)data     = accdata->players[index].tasks;
    (SPlayerRelation&)data  = accdata->players[index].relation;
    (SPlayerGM&)data        = accdata->players[index].gm;
    data.m_byStoreFlag      = 0;

    memcpy( &(SFixStorage1&)data, accdata->depot[0], sizeof( accdata->depot[0] ) );
    memcpy( &(SFixStorage2&)data, accdata->depot[1], sizeof( accdata->depot[1] ) );
    memcpy( &(SFixStorage3&)data, accdata->depot[2], sizeof( accdata->depot[2] ) );

    return true;
}

void PlayerDataCache::SendMail( Query &query, DNID dnid, SQMailSendMsg *pMsg )
{
    SAMailSendMsg msg;
    int iID;

    msg.m_iStaticID = pMsg->m_iStaticID;
    //
    if ( reinterpret_cast< CNetDBModule * >( &query )->SendMail( pMsg, iID ) )
    {
        msg.m_iRet = SAMailSendMsg::RET_SUCCESS;

        SANewMailMsg newmsg;
        newmsg.m_iStaticID = iID;

        SendMessage( dnid, &newmsg, sizeof( newmsg ) );
    }
    else
    {
        msg.m_iRet = SAMailSendMsg::RET_FAIL;
    }

    SendMessage( dnid, &msg, sizeof( msg ) );
}

void PlayerDataCache::RecvMail( Query &query, DNID dnid, SQMailRecvMsg *pMsg )
{
    SAMailRecvMsg msg;
    msg.m_iIndex = pMsg->m_iIndex;
    msg.m_iStaticID = pMsg->m_iStaticID;

    if ( reinterpret_cast< CNetDBModule * >( &query )->RecvMail( &msg, pMsg->m_iStaticID ) )
    {
        msg.m_iRet = SAMailRecvMsg::RET_SUCCESS;
    }
    else
    {
        msg.m_iRet = SAMailRecvMsg::RET_FAIL;
    }

    SendMessage( dnid, &msg, sizeof( msg ) );
}

void PlayerDataCache::DeleteMail( Query &query, DNID dnid, SQMailDeleteMsg *pMsg )
{
    SAMailDeleteMsg msg;

    msg.m_iStaticID = pMsg->m_iStaticID;
    msg.m_iNumber = pMsg->m_iNumber;

    if ( reinterpret_cast< CNetDBModule * >( &query )->DeleteMail( pMsg ) )
    {
        msg.m_iRet = SAMailDeleteMsg::RET_SUCCESS;
    }
    else
    {
        msg.m_iRet = SAMailDeleteMsg::RET_FAIL;
    }

    SendMessage( dnid, &msg, sizeof( msg ) );
}

void PlayerDataCache::NewMail(Query &query,DNID dnid,struct SQNewMailMsg *pMsg)
{
    if ( reinterpret_cast< CNetDBModule* >( &query )->NewMail( pMsg ) )
    {
        SANewMailMsg msg;
        msg.m_iStaticID = pMsg->m_iStaticID;

        SendMessage( dnid, &msg, sizeof( msg ) );
    }
}
