#pragma once
#include <new_net/netserverex.h>

typedef unsigned __int64 QWORD;

// pEntry ������ int __thiscall Entry(unsigned __int64 qwDnid, LINKSTATUS enumStatus, void *pMsg); ���͵ĺ���
// ���� pEntry �� int __callback Entry(unsigned __int64 qwDnid, LINKSTATUS enumStatus, void *pMsg); ���͵ĺ���

//ģ��˵����
//���� NetServer��һ��ģ�飬��CNetModule�̳ж�����������server�˵�����
//Initialize ��Ϊ��ʼ������
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

    // ��������
    CNetProviderModule(BOOL bUseVerify = FALSE);
    ~CNetProviderModule(void);
}; 

