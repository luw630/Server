#include "StdAfx.h"
#include "time.h"
#include "PlayerDataCache.h"
#include "LITESERIALIZER/Lite.h"

#include "NetworkModule/DataMsgs.h"
#include "NetDBModule/NetDBModule.h"
#include "pub/Thread.h"

extern BOOL SendMessage( DNID dnidClient, LPCVOID lpMsg, size_t wSize );

const int HANGUPSAVETIME = 1000 * 60 * 60; // 1��Сʱ����һ�ιһ�����
const int ONLINESAVETIME = 1000 * 60 * 24; // ��������24���ӱ���һ�� �˳�ʱ����ֱ�ӱ���

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

        // ֱ�ӻ�ȡ��ɫ�б���Ϊ���ʺż�����������ʺ��Ѿ����������������������ȡ����ʧ�ܣ�
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

        // ɾ����ɫ��Ҳֱ�Ӵ�������Ѿ��������������������������ʧ��
        // ���Ŀ��ص�û�н�ɫ������Ҳ��ʧ��
        // ���Ŀ���ɫ���ʺŹ����͵�ǰ�ʺŲ�һ�£�����Ҳ��ʧ��
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

        // ������ɫ��Ҳֱ�Ӵ�������Ѿ��������������������������ʧ��
        // ���Ŀ��ص��Ѿ����ڽ�ɫ������Ҳ��ʧ��
        // �����ɫ�����ظ�������Ҳ��ʧ��
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

        // ��ȡ��ɫ���ݣ����û��Ŀ���ɫ��������ʧ�ܣ�
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

        // ���������棬����ʱ����״̬��ȷ�Ͼ�����ֻ�Ǳ��棬���Ǳ����˳��������ٻ������ͬʱ������
        msg.byResult = SaveCharacter( query, pMsg->dwServerId, account, pMsg->PlayerData );

        SendMessage( dnid, &msg, sizeof(msg) - lsm.EndEdition() );
    }
    catch( lite::Xcpt &e )
    {
        LogXcptMsg( e.GetErrInfo() );
    }
}

////----------------------LocateAndLock----------------------
//��ͼ: ͨ���˺�������DataCache�Ĳ���
//����˵����
// bNeedCreate �Ƿ���Ҫ�������������Ҳ���ָ���˺ŵ�ʱ�򴴽�����
//����ֵ˵�������� 
//-----------------------------------------------------------
BOOL PlayerDataCache::LocateAndLock( LPCSTR account, AccountData &*ptr, 
    UGE::CriticalLock::Barrier4ReadWrite &barrier, bool bNeedCreate )
{
    // Ĭ�ϴ���� ptr ��һ���յ�����ָ�룡
    assert( !ptr );

__retry:
    LocateObject( account, ptr );

    if ( !ptr )
    {
        // �������Ҫ����������Ҳû���ҵ����������ʧ�ܷ��أ�
        if ( !bNeedCreate )
            return FALSE;

        // �½�ָ���˺Ŷ��󣡲��ܸ������е����ݣ�������ָ��Ǿ�˵�������ط��Ѿ��ɹ�������ptr�������ݿ�
        static AccountData ssad;
        if ( !InsertObject( ssad, account, false ) )
            return FALSE;

        // ����
        goto __retry;
    }

    // Ϊ��ȡ�Ķ��������������ɾ��״̬
    barrier.Assign( ptr );
    if ( ptr->delState == AccountData::DT_DELETED )
    {
        barrier.Assign( NULL );
        ptr = NULL;
        return FALSE;
    }

    // ����ɹ���ȡ��ͨ������ȡ����������ɾ��״̬��
    ptr->delState = AccountData::DT_NULL;

    return TRUE;
}

// ��ȡ��ɫ���б����ݣ����������Cache�в����ڣ��������ݿ��л�ȡ
int PlayerDataCache::GetCharacterList( Query &query, DWORD serverId, LPCSTR account, SCharListData data[3] )
{
    // ��ȡ��ɫ�б�������ֱ�Ӳ��������棨��ָ�½����ʺŶ�Ӧ�Ļ������
    memset( data, 0, sizeof( SCharListData ) * 3 );

    // ֱ�ӵ����ݿ��л�ȡ��ɫ�б������Լ������������ʧ�ܣ���˵�����ʺ��Ѿ�������������������
    int ret = reinterpret_cast< CNetDBModule * >( &query )->GetPlayerCharList( serverId, account, data );

    // �ڻ�ȡǰδ���κη��������������Խ�ɫ�б��ȡ�ɹ���
    if ( ret == 0 ) 
        return SAGetCharacterListMsg::RET_SUCCESS;

    else if ( ret != serverId ) // �������������ʺ����ڱ���ǰ������������
    {
        // �������г��Բ��ң����û�ҵ���˵�������ǵ�ǰ�Ļ�����������ˡ������ǾͿ�������������
        if ( !LocateTest( account ) ) 
            return SAGetCharacterListMsg::RET_SUCCESS;

        // ����ҵ���˵��ȷʵ����������������Ϸ�߼���������»��汻������
        // ���ر��Լ���������Ϣ�������õ�½��������һ��ȷ���Ƿ��ɫ�ظ���������״��������������
        return SAGetCharacterListMsg::RET_LOCKEDSELF; 
    }

    // �����Ǳ����������������ˣ����ԣ����������Ĳ����Ͳ��ü���������
    return SAGetCharacterListMsg::RET_LOCKEDOTHER;
}

int PlayerDataCache::DeleteCharacter( Query &query, DWORD serverId, LPCSTR account, DWORD staticId, char name[11] )
{
    // ɾ����ɫʱ�����ʺ�����һ�����ô����ڻ����У���Ϊ��֮ǰ�����ǻ�ȡ��ɫ�б���û�в�������Ķ�����
    if ( LocateTest( account ) ) 
        return FALSE;

    // ֱ�ӵ����ݿ��л�ȡ��ɫ�б������Լ������������ʧ�ܣ���˵�����ʺ��Ѿ�������������������
    int ret = reinterpret_cast< CNetDBModule * >( &query )->DeletePlayer( serverId, account, staticId );

    // �ڻ�ȡǰδ���κη��������������Խ�ɫ�б��ȡ�ɹ���
    if ( ret == 0 ) 
        return SAGetCharacterListMsg::RET_SUCCESS;

    else if ( ret != serverId ) // �������������ʺ����ڱ���ǰ������������
    {
        // �������г��Բ��ң����û�ҵ���˵�������ǵ�ǰ�Ļ�����������ˡ������ǾͿ�������������
        if ( !LocateTest( account ) ) 
            return SAGetCharacterListMsg::RET_SUCCESS;

        // ����ҵ���˵��ȷʵ����������������Ϸ�߼���������»��汻������
        // ���ر��Լ���������Ϣ�������õ�½��������һ��ȷ���Ƿ��ɫ�ظ���������״��������������
        return SAGetCharacterListMsg::RET_LOCKEDSELF; 
    }

    // �����Ǳ����������������ˣ����ԣ����������Ĳ����Ͳ��ü���������
    return SAGetCharacterListMsg::RET_LOCKEDOTHER;

    // ������δ�ҵ�ָ������ʧ�ܷ��أ�
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
        // �������ʧ�ܣ������Ҳ���ָ���˺����ݣ���ֱ�ӷ��أ�
        return FALSE;
    }

    if ( ( accdata->account.empty() ) ||
        ( accdata->server_id != serverId ) ||
        ( accdata->players[index].static_id != 0 ) )
    {
        // ��δ��ʼ����ʧ�ܷ���
        // ��������ķ������͵�ǰ�������ķ�������ͬ
        // ָ��λ�����Ѿ����ڽ�ɫ���޷����Ǵ���
        return FALSE;
    }

    // �����ݿ����󴴽��������ɹ���������ݣ�
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

    // ������ɫʧ�ܣ�
    return ret;    
}

int PlayerDataCache::SaveCharacter( Query &query, DWORD serverId, LPCSTR account,const SFixProperty &data )
{
    // ��ʱ��������������Ϣ�����������¼���
    char str[1024] = "";
    boost::shared_ptr< AccountData > accdata;
    Barrier barrier;

    //�������ֻ����DBServer�쳣�ر�����˵�¼����������������ݣ�DBServer��Cache��û������
    if ( !LocateAndLock( account, accdata, barrier ) )
    {     
        // �����޸ģ���������������ݿ⣬Ҳ���Խ����Ѿ����ڵ��˺ŷ���������������
        SCharListData tmpdata[3];
        int ret = GetCharacterList( query, serverId, account, tmpdata );

        sprintf( str, "serverId = %d, account = %s [%s]", serverId, account, ret ? "fail" : "succ" );
        LogMsg( "rebind.txt", str );

        if ( ret != SAGetCharacterListMsg::RET_SUCCESS )
            return false;

        // �ٴγ��Ի�ȡ���������ʧ����������桭��
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
                // GM������ݲ����뱣�棡

                memcpy( accdata->depot[0], &(SFixStorage1&)data, sizeof( accdata->depot[0] ) );
                memcpy( accdata->depot[1], &(SFixStorage2&)data, sizeof( accdata->depot[1] ) );
                memcpy( accdata->depot[2], &(SFixStorage3&)data, sizeof( accdata->depot[2] ) );

                // �����˺Ųֿ����ݱ��Ϊ���޸ģ�������Ϊ�ڽ���ĳ��ʱ�̱��浽���ݿ��е��ж�����
                accdata->warehousechanged = true;
                accdata->warehouseloaded = true;

                // ������Ϊ�ڽ���ĳ��ʱ�̱��浽���ݿ��е��ж�����
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

        // ����δ�ҵ�ָ���洢�Ľ�ɫ������
        sprintf( str, "find failure account = %s, gid = %d", account, data.m_dwStaticID );
        LogMsg( "error.txt", str, __LINE__ );
    }
    else
    {
        // �������ش��󣡣���������������������������������
        // ���ݿ��յ���ͬ[���]��Ϸ�����������Ĵ������󣡣���
        // ����ɸ��ƣ���������������������������������������
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
        // �������ʧ�ܣ������Ҳ���ָ���˺����ݣ���ֱ�ӷ��أ�
        return false;
    }

    if ( ( accdata->account.empty() ) || 
        ( accdata->server_id != serverId ) || 
        ( accdata->players[index].static_id == 0 ) )
    {
        // �˺Ż�δ��ʼ����ʧ�ܷ���
        // ���˺Ż�����[��Ծ]��ĳ����������(�Ҳ��ǵ�ǰ�һ��ķ�����)�����Բ����ٴλ�ȡ��ɫ����[��½]
        // ���˺Ų�����ָ����ɫ��
        return false;
    }

    // ����ֿ�δ��ȡ�����������ݿ������ȡ
    if ( !accdata->warehouseloaded )
    {
        if ( !reinterpret_cast< CNetDBModule * >( &query )->
            GetWareHouseData( account, accdata->depot ) )
        {
            // ��ȡ�ֿ�����ʧ�ܣ�
            return false;
        }

        accdata->warehouseloaded = true;
    }

    if ( accdata->players[index].base.m_dwBaseHP == 0 )
    {
        // �ý�ɫ��ɫ��δ��ȡ���輴ʱȡ������
        if ( !reinterpret_cast< CNetDBModule * >( &query )->
            GetCharacterData( account, index + 1, 
            accdata->players[index].base, 
            accdata->players[index].package, 
            accdata->players[index].tasks, 
            accdata->players[index].relation,
			serverId ) )
        {
            // ��ȡ��ɫ����ʧ�ܣ�
            accdata->players[index].base.m_dwBaseHP = 0;
            return false;
        }

        // ǿ�ƽ�GM�ȼ���Ϊ0
        accdata->players[index].gm.m_wGMLevel = 0;

        // ����һ�ȡ��ɫ��ʼ��ʱ!
        {
            AccountTimeSet temp;
            if ( !limitManager.LocateObject( account, temp ) )
            {
                // ��ʼ�����ʺŶ�Ӧ�ļ�ʱ����!
                temp.onlineTime = 1;
                temp.offlineTime = 0;
                temp.offlineSegment = ( DWORD )time( NULL );
            }

            // ������ʱ�����ݸ���Ϸ���������
            accdata->players[index].base.m_pTroopsSkills[0].dwProficiency = temp.onlineTime;
            accdata->players[index].base.m_pTroopsSkills[1].dwProficiency = temp.offlineTime;
            accdata->players[index].base.m_pTroopsSkills[2].dwProficiency = temp.offlineSegment;
        }
    }

    // ������������ݵ����ص�������
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
