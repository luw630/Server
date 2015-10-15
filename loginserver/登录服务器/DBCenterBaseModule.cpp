//#######################
//# xo checked at 09-24 #
//#######################
#include "StdAfx.h"
#include <boost/bind.hpp>

#include "dbcenterbasemodule.h"
#include "liteserializer/lite.h"

#include "networkmodule/refreshmsgs.h"
#include "networkmodule/accountmsgs.h"
#include "pub/rpcop.h"

extern BOOL TraceInfo( LPCSTR szFileName, LPCSTR szFormat, ... );

DBCenterBaseModule::DBCenterBaseModule( DWORD dbcId ) : dbcId( dbcId )
{
}

DBCenterBaseModule::~DBCenterBaseModule()
{
}

void DBCenterBaseModule::Destroy()
{
    client.Disconnect();
}

bool DBCenterBaseModule::Connect( LPCSTR ip, LPCSTR port )
{
    if ( client.IsConnected() )
        return false;

    currentIP = ip;
    currentPort = port;

    if ( currentIP.empty() || currentPort.empty() )
        return false;

    if ( !client.Connect( ip, port ) )
        return false;

    SQRegisterLogsrvMsg msg;
    msg.IP = 0;
    msg.ID = Globals::GetServerId();

    return client.SendMessage( &msg, sizeof(msg) ) == TRUE;
}

static DWORD prevConnectTime = timeGetTime();
int DBCenterBaseModule::Execution()
{
	client.Execution(boost::bind(&DBCenterBaseModule::Entry, this, _1, _2));

    // ��������
    if ( abs( ( int )( timeGetTime() - prevConnectTime ) ) > 1000*60 )
    {
        prevConnectTime = timeGetTime();

        if ( !client.IsConnected() )
        {
            if ( Connect( currentIP.c_str(), currentPort.c_str() ) )
            {
                rfalse( 2, 1, "���ӵ��������ݿ�ɹ�");
                rfalse( 1, 1, "���ӵ��������ݿ�ɹ�");
            }
            else
            {
                //rfalse( 2, 1, "���������ݿ⽨������ʧ��");
                //rfalse( 1, 1, "���������ݿ⽨������ʧ��");
            }
        }
    }

    return 1;
}

bool DBCenterBaseModule::Entry( LPCVOID data, size_t size )
{
    if ( data == NULL )
    {
        rfalse( 2, 1, "���������ݿ�����ӶϿ�EX");
        rfalse( 1, 1, "���������ݿ�����ӶϿ�EX");

        // �趨����ʱ�䣬��5�������
        prevConnectTime = ( timeGetTime() - 1000*60 ) + 5000;
        return false;
    }

    const SMessage *pMsg = ( SMessage* )data;
    const SServerCtrlMsg *pCtrlMsg = (SServerCtrlMsg *)pMsg;

    if ( ( pMsg->_protocol != SMessage::EPRO_SERVER_CONTROL ) ||
        ( ( ( const SServerCtrlMsg * )pMsg )->_protocol != SServerCtrlMsg::EPRO_GAMESERVER_REQUEST_RPCOP ) )
    {
        // ����ϢΪ������������ת����Ϣ
        if ( pCtrlMsg->_protocol == SServerCtrlMsg::EPRO_SCRIPT_TRANSFER )
            Globals::SendToAllGameServer( pMsg, size );
        return true;
    }

    // �����������ݿ��������Ϣ������Ҫ���ص�ԭʼ����������ϵģ����߸���û����Ϣ���أ�
	SAGameServerRPCOPMsg *tempMsg = ( SAGameServerRPCOPMsg * )data;
	if ( tempMsg->dnidClient == INVALID_DNID )
        return true;

    try
    {
        lite::Serialreader slr( tempMsg->streamData );

        if ( ( int )slr() == ( int )SMessage::EPRO_SYSTEM_MESSAGE )
            if ( ( int )slr() == 7 ) //SSysBaseMsg::EPRO_LOGIN_NOCHECKACCOUNT
            {
                if ( !Globals::IsValidPlayerDnid( tempMsg->dnidClient ) )
                {
                    // ����Ѿ���Ч����ô���β����Ѿ����ܼ�����ȥ��
                    return false;
                }

                LPCSTR account = slr();
                LPCSTR upassword = slr();
                DWORD gmlevel = ( DWORD )( int ) slr();
                if ( Globals::RebindAccount( tempMsg->dnidClient, account, 0, 0, 0, 0 ) )
                {
                    rfalse(2, 1, "check account [%s] and rebind", account );
                    return true;
                }

                // �������Ӹ���Context���˺š����룩
                if ( !Globals::SetPlayerLinkContext( 
                    tempMsg->dnidClient, account, "", upassword, 0, gmlevel, 0, 0, 0, 0 ) )
                {
                    rfalse(2, 1, "check account [%s] SetPlayerLinkContext fail", account );
                    return true;
                }

                rfalse(2, 1, "[nocheckaccount]check account [%s] ok", account  );
                extern BOOL QueryPlayerList( LPCSTR szAccount, LPCSTR szPassword, DNID dnidClient );

                // ��֤�ɹ�����Ҫ���������ݿ������������ҽ�ɫ�б�
                QueryPlayerList( account, "", tempMsg->dnidClient );
                return TRUE;
            }
    }
    catch( lite::Xcpt & )
    {
    }

    tempMsg->srcDatabase = (BYTE)(2 + dbcId);
	Globals::SendToGameServer( tempMsg->dnidClient, const_cast< void* >( data ), size );

    return true;
}
