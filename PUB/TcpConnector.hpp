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

                assert( ( size <= ( int )sizeof( buffer ) ) && "需要发送的缓存大小超过配置大小，可能已经越界溢出！" );

                if ( size < 0 ) 
                    break;  // 如果小于0，则表示handler确定再没有后续数据需要发送，可以主动结束连接！

                if ( ( size > 0 ) && ( send( tcp, buffer, size, 0 ) != size ) )
                    break;  // 到这里是因为发送数据失败而导致的错误，也必须结束连接！
            }

            // 当发送线程结束时，关闭连接！
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
        // 准备网络连接
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

        // 启动发送线程
        SendProcedure sendProcedure( tcp, handler );
        HANDLE sendThread = sendProcedure.BeginThread( sendProcedure );

        // 开始循环获取网络数据流
		static __declspec( thread ) char buffer[1024*1024];
		int position = 0;

		while ( true )
		{
			// 直接获取一个足够大的buffer
			// 依据是:只要收到一定数据量tcp数据,不管接收量是否远小于请求大小,都会导致recv返回!!!
			int size = recv( tcp, buffer + position, sizeof( buffer ) - position, 0 );
			if ( size <= 0 )
			{
				// 事件回调
                handler.OnDisconnected( size == 0 ? 0 : WSAGetLastError(), sendThread );
				break;
			}

			// 处理当前已经获取的数据并且重新处理缓存buffer和position的关系!
			position = handler.OnStreamValidate( buffer, position += size );
		}

		// 将发送线程停止！
		sendProcedure.Quit();

        // 销毁连接，退出
		return closesocket( tcp ), WSACleanup(), 1;
	}

public:
    typedef int result_type;

private:
    _Handler handler;
};

END_OF_NAMESPACE_UGE_TCP
