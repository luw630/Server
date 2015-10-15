//#######################
//# xo checked at 09-24 #
//#######################
#include "stdafx.h"
#include "NetServerModule.h"

CNetServerModule::CNetServerModule() :
    CNetServerEx(true)
{
    // ����ط��ȽϹ��졭����������
    // ����������ײ�Ƚϲ�ȫ�棬���ݲɼ������͹���������ײ��ͷ�������Ǽ��ܹ��ģ����Ǻ�����������δ���ܡ���
    // ������ط�������ĳ�ʼ��ʱ���� m_bUseVerify ��Ϊ�棬������createʱ��������ģ��
    // ����create�󣬽� m_bUseVerify ��Ϊ�٣��������յ����ݺ�Ͳ��ý����ˣ�
}

BOOL CNetServerModule::Create( const char *szPortName )
{
    BOOL ret = CNetServerEx::Create( szPortName );

    struct dummy : 
        public iDirectNetServerEventHandler // declared inside "DirectNetServer.h"
    {
        BOOL bUseVerify;
    };

    ( (dummy *)( (CNetServerEx *)this ) )->bUseVerify = false;

    return ret;
}

CNetServerModule::~CNetServerModule()
{

}

int CNetServerModule::Destroy()
{
    Close();
    return 1;
}

int CNetServerModule::Execution()
{
    SNetServerPacket *pPacket = NULL;

    while (GetMessage(&pPacket))
    {
        DNID dnidClient = pPacket->m_dnidClient;
        LINKSTATUS enumStatus = pPacket->m_enumStatus;

        if (pPacket->m_wLength == 0)
        {
            // rfalse(1, 0, "disconnect!!!");
            // link break
#ifdef USE_TRY
            //try
#endif
            {
                void *pOnDispatch = m_pOnDispatch, *pThis = m_pThis;
                __asm 
                {
                    push    0
                    push    enumStatus
                    lea     ecx, dnidClient
                    push    dword ptr [ecx + 4]
                    push    dword ptr [ecx]
                    mov     edx, pOnDispatch
                    mov     ecx, pThis
                    call    edx
                }
            }
#ifdef USE_TRY
            //catch(...)
            //{   
            //    //ERROR HANDLING
            //    DOTHROW;
            //    BOOL rfalse(char, char, LPCSTR, ...);
            //    rfalse(1, 1, "catched N.E. "__FILE__"(%d)", __LINE__);
            //}
#endif
        }
        else
        {
            LPVOID pMsg = pPacket->m_pvBuffer;

#ifdef USE_TRY
            //try
#endif
            {
                void *pOnDispatch = m_pOnDispatch, *pThis = m_pThis;
                __asm 
                {
                    push    pMsg
                    push    enumStatus
                    lea     ecx, dnidClient
                    push    dword ptr [ecx + 4]
                    push    dword ptr [ecx]
                    mov     edx, pOnDispatch
                    mov     ecx, pThis
                    call    edx
                }
            }
#ifdef USE_TRY
            //catch(...)
            //{   
            //    //ERROR HANDLING
            //    DOTHROW;
            //    BOOL rfalse(char, char, LPCSTR, ...);
            //    rfalse(1, 1, "catched N.E. "__FILE__"(%d)", __LINE__);
            //}
#endif
        }

        if (pPacket->m_pvBuffer)
            delete pPacket->m_pvBuffer;
        delete pPacket;
    }

    // CheckAlive();

    return 1;
}

bool CNetServerModule::__init(char* szPortName, ...)
{
    va_list cp;

    va_start (cp, szPortName);
    m_pOnDispatch = va_arg (cp, void*);
    m_pThis = va_arg (cp, void*);
    va_end (cp);

    return Create(szPortName) == TRUE;
}
