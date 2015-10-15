#pragma once
#include <new_net/netserverex.h>

typedef unsigned __int64 QWORD;

// pEntry 必须是 int __thiscall Entry(unsigned __int64 qwDnid, LINKSTATUS enumStatus, void *pMsg); 类型的函数
// 或者 pEntry 是 int __callback Entry(unsigned __int64 qwDnid, LINKSTATUS enumStatus, void *pMsg); 类型的函数

//模块说明：
//基于 NetServer的一个模块，由CNetModule继承而来，负责处理server端的数据
//Initialize 作为初始化函数
class CNetProviderModule : 
    public CNetServerEx
{
public:
    template < typename op >
    int Execution( op _entry )
    {
        SNetServerPacket *pPacket = NULL;

        extern DWORD dwNetRunLimit;
        DWORD segTime = timeGetTime() + dwNetRunLimit;

        while ( (int)( segTime - timeGetTime() ) > 0 )
        {
            if ( GetMessage( &pPacket ) == FALSE )
                break;

            bool ret = _entry( pPacket->m_dnidClient, 
                pPacket->m_enumStatus, pPacket->m_pvBuffer, pPacket->m_wLength );
 
            if ( pPacket->m_pvBuffer && ret )
                delete pPacket->m_pvBuffer;

            delete pPacket;
        }

        return 1;
    }

    int Destroy(void);

    // 构造析构
    CNetProviderModule(BOOL bUseVerify = FALSE);
    ~CNetProviderModule(void);
}; 

