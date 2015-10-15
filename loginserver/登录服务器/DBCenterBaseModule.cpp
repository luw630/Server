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

    // 断线重连
    if ( abs( ( int )( timeGetTime() - prevConnectTime ) ) > 1000*60 )
    {
        prevConnectTime = timeGetTime();

        if ( !client.IsConnected() )
        {
            if ( Connect( currentIP.c_str(), currentPort.c_str() ) )
            {
                rfalse( 2, 1, "连接到中央数据库成功");
                rfalse( 1, 1, "连接到中央数据库成功");
            }
            else
            {
                //rfalse( 2, 1, "与中央数据库建立连接失败");
                //rfalse( 1, 1, "与中央数据库建立连接失败");
            }
        }
    }

    return 1;
}

bool DBCenterBaseModule::Entry( LPCVOID data, size_t size )
{
    if ( data == NULL )
    {
        rfalse( 2, 1, "和中央数据库的连接断开EX");
        rfalse( 1, 1, "和中央数据库的连接断开EX");

        // 设定重连时间，在5秒后启动
        prevConnectTime = ( timeGetTime() - 1000*60 ) + 5000;
        return false;
    }

    const SMessage *pMsg = ( SMessage* )data;
    const SServerCtrlMsg *pCtrlMsg = (SServerCtrlMsg *)pMsg;

    if ( ( pMsg->_protocol != SMessage::EPRO_SERVER_CONTROL ) ||
        ( ( ( const SServerCtrlMsg * )pMsg )->_protocol != SServerCtrlMsg::EPRO_GAMESERVER_REQUEST_RPCOP ) )
    {
        // 该消息为其他服务器的转发消息
        if ( pCtrlMsg->_protocol == SServerCtrlMsg::EPRO_SCRIPT_TRANSFER )
            Globals::SendToAllGameServer( pMsg, size );
        return true;
    }

    // 基本中央数据库回来的消息都是需要返回到原始区域服务器上的（或者根本没有消息返回）
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
                    // 如果已经无效，那么本次操作已经不能继续下去了
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

                // 给该连接附加Context（账号、密码）
                if ( !Globals::SetPlayerLinkContext( 
                    tempMsg->dnidClient, account, "", upassword, 0, gmlevel, 0, 0, 0, 0 ) )
                {
                    rfalse(2, 1, "check account [%s] SetPlayerLinkContext fail", account );
                    return true;
                }

                rfalse(2, 1, "[nocheckaccount]check account [%s] ok", account  );
                extern BOOL QueryPlayerList( LPCSTR szAccount, LPCSTR szPassword, DNID dnidClient );

                // 认证成功后，需要继续向数据库服务器请求玩家角色列表
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
