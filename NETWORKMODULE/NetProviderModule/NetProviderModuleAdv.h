#pragma once
#include <new_net/netserveradv.h>

typedef unsigned __int64 QWORD;

// Context��ʹ�ù���
// Context���ϲ��Dnid���ӵĶ���ָ�����ݣ������ڿ��ٵĶ�λDnid��Ӧ��ʵ�����
// ������Ҫע����ǣ�Context�����ɺ��ͷŹ����Լ���ʱ�ܹ���ȫʹ��������Ϣ���ص�Context

// ��3����ȫʹ�ûش���Context��
// ��ΪDnid����Context�ɹ���ʼ�����Dnid���ӵ�ContextΪֹ����Dnid��ȡ��������Ϣ�б�Ȼ���������õ�Context
// һ�������ֻҪ�����ϲ㲻�����ͷ�Context�Ϳ��԰�ȫʹ����
// ����ϲ������ͷ�Context����ô�������Ƿ��������Dnid���ӵ�Context�����п��ܵ��·�����Ч
// ��Ϊ�����ʱ������������Dnid���ӵ�Context�����������ʱ��֮ǰ������ײ���Ȼ�п��������µĴ������ڵ�Context�����ݰ�
// ���������Dnid���ӵ�Context֮�󣬼�������������Ϣ����ʱ���Context�Ѿ����ͷŵ��ˣ������������ǲ���ȫ�ģ�
// ������Ҫ�������һ��Context�����ʱ��ֻ�����Dnid���ӵ�Context�����յ���Dnid�ĶϿ���Ϣʱ�����ͷ�Context����

// ��2��Context�����&�ͷţ�
// ����3�������������������޷������ͷ��Ա����ӵ�Context���󣡣���
// ���԰�ȫ���ͷŷ�ʽΪ���յ���Dnid�ĶϿ���Ϣʱ���ͷ�Context���󣬵�Ȼ����Ҳ���Լ����ı�����Context
// ͬʱ��һ�������������������˳���ʱ�򣬿����޷���ȫ�Ĵ������е�������Ϣ
// ����Ҳ������Dnid�ĶϿ���Ϣ�����Կ��ܵ���Context�����޷����ͷ�
// Ϊ�˴���������⣬���е�Context�������ϲ���һ�������б����˳�ʱ�����ͷ������µĶ���

// ��1��Context������&���ӣ�
// ������ԱȽ���������ɣ����ۺ�ʱ�����ԣ�������Ҫע����ǣ�
// һ��Ҫ������Context��Dnidʱʧ�ܵ��������Ϊ��ʱ���Dnid�Ѿ���Ч���޷��ɵײ�����Context���ͷ�
// ͬʱ��Ҫע�����ϵģ�3����2�����㣬��Ҫ�����ͷ�Context������Context�б�

// ���˵���������������
// A����Ϊһ��Dnid���Ӷ��Context�������ǲ��еģ����Ҿ�����Ҫ��������
// һ������Ϊֻ�ܱ���һ��Context�������µ�Context���ƻ��ˣ�3����2�����򣡣���
// ��һ������Ϊ�������ʱ��ǰ��Context��������Ч�����ܻᵼ���޷��жϵ�ʱ������
// B����Ϊ���Dnid����ͬһ��Context�������ǿ��ԣ����Ǳ����Լ�ͳ�Ƹ��Ӵ����ļ�������
// ȷ���ڴ�������Dnid�ĶϿ���Ϣ֮ǰ�������ͷŵ���Context������

// pEntry ������ int __thiscall Entry(unsigned __int64 qwDnid, LINKSTATUS enumStatus, void *pMsg, void *pContext); ���͵ĺ���
// ���� pEntry �� int __callback Entry(unsigned __int64 qwDnid, LINKSTATUS enumStatus, void *pMsg, void *pContext); ���͵ĺ���

//ģ��˵����
//���� NetServer��һ��ģ�飬��CNetModule�̳ж�����������server�˵�����
//Initialize ��Ϊ��ʼ������
class CNetProviderModuleAdv : 
    public CNetServerAdv
{
public:
    // ���̿��ƺ���
    template < typename op >
    int Execution( op _entry )
    {
        SNetServerPacketAdv *pPacket = NULL;

        extern DWORD dwNetRunLimit;
        DWORD segTime = timeGetTime() + dwNetRunLimit;

        int count = 0;

        while ( (int)( segTime - timeGetTime() ) > 0 )
        {
            if ( GetMessage( &pPacket ) == FALSE )
                break;

            bool ret = _entry( pPacket->m_dnidClient, 
                pPacket->m_enumStatus, pPacket->m_pvBuffer, pPacket->m_wLength, pPacket->m_pContext );

            count ++;
 
            if ( pPacket->m_pvBuffer && ret )
                delete pPacket->m_pvBuffer;

            delete pPacket;
        }

        return count;
    }

    int     Destroy(void);

    // ��������
    CNetProviderModuleAdv(BOOL bUseVerify = FALSE);
    ~CNetProviderModuleAdv(void);
}; 

