#pragma once
#include <new_net/netclientex.h>

// pEntry ������ int __thiscall Entry(void *pMsg); ���͵ĺ��������Ҳ������麯��
// ���� pEntry �� int __callback Entry(void *pMsg); ���͵ĺ���

//ģ��˵����
//���� NetClient��һ��ģ�飬��CNetModule�̳ж�����������client�˵�����
//Initialize ��Ϊ��ʼ������
class CNetConsumerModule : 
    public CNetClientEx
{
public:
    // ���̿��ƺ���
    template < typename op >
    int Execution( op _entry )
    {
        SNetClientPacket *pPacket = NULL;

        extern DWORD dwNetRunLimit;
        DWORD segTime = timeGetTime() + dwNetRunLimit;

        while ( (int)( segTime - timeGetTime() ) > 0 )
        {
            if ( GetMessage( &pPacket ) == FALSE )
                break;

            bool ret = _entry( pPacket->m_pvBuffer, pPacket->m_wLength );
 
            if ( pPacket->m_pvBuffer && ret )
                delete pPacket->m_pvBuffer;

            delete pPacket;
        }

        return 1;
    }

    int Destroy(void);

    // ��������
    CNetConsumerModule(BOOL bUseVerify = FALSE);
    ~CNetConsumerModule(void);
};
