//#######################
//# xo checked at 09-24 #
//#######################
#include "stdafx.h"
#include "NetServerModule.h"

CNetServerModule::CNetServerModule() :
    CNetServerEx(true)
{
    // 这个地方比较诡异………………
    // 当初的网络底层比较不全面，数据采集器发送过来的网络底层包头的数据是加密过的，但是后续的数据又未加密……
    // 所这个地方在最初的初始化时，将 m_bUseVerify 设为真，用以在create时创建加密模块
    // 而在create后，将 m_bUseVerify 设为假，这样在收到数据后就不用解密了！
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
