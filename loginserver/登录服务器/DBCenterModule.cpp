//#######################
//# xo checked at 09-24 #
//#######################
#include "StdAfx.h"
#include <boost/bind.hpp>

#include "dbcentermodule.h"
#include "liteserializer/lite.h"

#include "networkmodule/refreshmsgs.h"
#include "networkmodule/accountmsgs.h"
#include "pub/rpcop.h"

//#include "networkmodule/logmsgs.h"
//#include "networkmodule/datamsgs.h"
//#include "networkmodule/dbopmsgs.h"
//#include "networkmodule/cardpointmodifymsgs.h"

extern BOOL TraceInfo( LPCSTR szFileName, LPCSTR szFormat, ... );

DBCenterModule::DBCenterModule(void)
{
}

DBCenterModule::~DBCenterModule(void)
{
}

void DBCenterModule::Destroy()
{
}

bool DBCenterModule::Connect( LPCSTR ip, LPCSTR port )
{
    if ( client.IsConnected() )
        return false;

    currentIP = ip;
    currentPort = port;

    if ( !client.Connect( ip, port ) )
        return false;

    SQRegisterLogsrvMsg msg;
    msg.IP = 0;
    msg.ID = Globals::GetServerId();

    return client.SendMessage( &msg, sizeof(msg) ) == TRUE; 
}

static DWORD prevConnectTime = timeGetTime();
int DBCenterModule::Execution()
{
	client.Execution(boost::bind(&DBCenterModule:: Entry, this, _1, _2));

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
				/*rfalse(2, 1, "���������ݿ⽨������ʧ��~");
				rfalse(1, 1, "���������ݿ⽨������ʧ��~");*/
            }
        }
    }

    return 1;
}

bool DBCenterModule::Entry( LPCVOID data, size_t size )
{
    if ( data == NULL )
    {
        rfalse( 2, 1, "���������ݿ�����ӶϿ�");
        rfalse( 1, 1, "���������ݿ�����ӶϿ�");

        // �趨����ʱ�䣬��5�������
        prevConnectTime = ( timeGetTime() - 1000*60 ) + 5000;
        return false;
    }

    typedef const SMessage * LPCMESSAGE;

    LPCMESSAGE pMsg = ( LPCMESSAGE )data;

    if ( ( pMsg->_protocol == SMessage::EPRO_SERVER_CONTROL ) && 
        ( ( ( const SServerCtrlMsg * )pMsg )->_protocol == SServerCtrlMsg::EPRO_GAMESERVER_REQUEST_RPCOP ) )
    {
	    SAGameServerRPCOPMsg *tempMsg = ( SAGameServerRPCOPMsg * )data;

	    if ( tempMsg->dnidClient == INVALID_DNID )
            ProcessInterop( tempMsg->streamData );
        else
        {
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

            tempMsg->srcDatabase = 2;
		    Globals::SendToGameServer( tempMsg->dnidClient, const_cast< void* >( data ), size );
        }
    }

    return true;
}

void DBCenterModule::ProcessInterop( void *data )
{
}

void SendServerLogToDBC( LPCSTR str, int mType, int sb1, int sb2 = 0, int sb3 = 0, int sb4 = 0 )
{
	SQGameServerRPCOPMsg msg;
    msg.dnidClient = INVALID_DNID;

	lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );

	try
	{
            // ׼���洢���̣����㣩
        slm [OP_PREPARE_STOREDPROC] ( "dbo.SaveServerLog" ) 

		    // �趨���ò���
		    [OP_BEGIN_PARAMS]
		    (1) ( str )
		    (2) ( mType )
		    (3) ( sb1 );

        if ( sb2 != 0 )
        {
		    slm ( 4 ) ( sb2 );

            if ( sb3 != 0 )
            {
		        slm ( 5 ) ( sb3 );

                if ( sb4 != 0 )
                {
		            slm ( 6 ) ( sb4 );
                }
            }
        }

        slm [OP_END_PARAMS]

		    // ���ô洢����
		    [OP_CALL_STOREDPROC]

            [OP_RPC_END];

        // ȫ��׼���ú��ٷ�����Ϣ����½�������������ɵ�½��������ת���˺ŷ�����
        msg.dstDatabase = 2;
        Globals::SendToDBCenter( &msg, sizeof( msg ) - slm.EndEdition() );
    }
    catch ( lite::Xcpt & )
	{
        // ֻ��������жϵĴ���
	}
}
