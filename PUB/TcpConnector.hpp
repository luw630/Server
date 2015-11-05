#pragma once

#include <winsock.h>
#include "threadex.h"
#include "boost/bind.hpp"

#define BEGIN_OF_NAMESPACE_UGE_TCP      namespace UGE { namespace tcp {
#define END_OF_NAMESPACE_UGE_TCP        } }

BEGIN_OF_NAMESPACE_UGE_TCP

template < typename _Handler >
class TcpConnector
{
public:
    class SendProcedure : public UGE::ThreadWaiter
	{
	public:
		SendProcedure( SOCKET tcp, _Handler &handler ) : tcp( tcp ), handler( handler ) {}

		int operator () ()
		{
			static __declspec( thread ) char buffer[1024*1024];

            while ( !QuitFlag() )
            {
                int size = handler.MakeSendStream( buffer, sizeof( buffer ) );

                assert( ( size <= ( int )sizeof( buffer ) ) && "��Ҫ���͵Ļ����С�������ô�С�������Ѿ�Խ�������" );

                if ( size < 0 ) 
                    break;  // ���С��0�����ʾhandlerȷ����û�к���������Ҫ���ͣ����������������ӣ�

                if ( ( size > 0 ) && ( send( tcp, buffer, size, 0 ) != size ) )
                    break;  // ����������Ϊ��������ʧ�ܶ����µĴ���Ҳ����������ӣ�
            }

            // �������߳̽���ʱ���ر����ӣ�
            return shutdown( tcp, 2 ), 1;
		}

    public:
        typedef int result_type;

	private:
		SOCKET   tcp;
		_Handler &handler;
	};

public:
	TcpConnector( _Handler &handler ) : handler( handler ) {}

	int operator () ()
	{
        // ׼����������
        WSADATA wsad;
        if ( WSAStartup( MAKEWORD( 1, 0 ), &wsad ) != 0 )
            return handler.OnConnectComplete( false ), -1;

        SOCKET tcp = socket( AF_INET, SOCK_STREAM, 0 );
        if ( tcp == INVALID_SOCKET )
            return handler.OnConnectComplete( false ), WSACleanup(), -1;

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr( handler.get_IP() );
        if ( addr.sin_addr.s_addr == 0xffffffff )
        {
            if ( hostent *remoteHost = gethostbyname( handler.get_IP() ) )
                if ( remoteHost->h_length == 4 )
                    addr.sin_addr.s_addr = *( LPDWORD )*remoteHost->h_addr_list;
        }

        addr.sin_port = htons( handler.get_Port() );

        if ( connect( tcp, ( sockaddr* )&addr, sizeof( addr ) ) == SOCKET_ERROR )
            return handler.OnConnectComplete( false ), closesocket( tcp ), WSACleanup(), -1;

        handler.OnConnectComplete( true );

        // ���������߳�
        SendProcedure sendProcedure( tcp, handler );
        HANDLE sendThread = sendProcedure.BeginThread( sendProcedure );

        // ��ʼѭ����ȡ����������
		static __declspec( thread ) char buffer[1024*1024];
		int position = 0;

		while ( true )
		{
			// ֱ�ӻ�ȡһ���㹻���buffer
			// ������:ֻҪ�յ�һ��������tcp����,���ܽ������Ƿ�ԶС�������С,���ᵼ��recv����!!!
			int size = recv( tcp, buffer + position, sizeof( buffer ) - position, 0 );
			if ( size <= 0 )
			{
				// �¼��ص�
                handler.OnDisconnected( size == 0 ? 0 : WSAGetLastError(), sendThread );
				break;
			}

			// ����ǰ�Ѿ���ȡ�����ݲ������´�����buffer��position�Ĺ�ϵ!
			position = handler.OnStreamValidate( buffer, position += size );
		}

		// �������߳�ֹͣ��
		sendProcedure.Quit();

        // �������ӣ��˳�
		return closesocket( tcp ), WSACleanup(), 1;
	}

public:
    typedef int result_type;

private:
    _Handler handler;
};

END_OF_NAMESPACE_UGE_TCP
