//#######################
//# xo checked at 09-24 #
//#######################
#include "NetProviderModuleAdv.h"

#include <crtdbg.h>

#define USE_CRC
#include "new_net/desdec.h"

#define DOTHROW

extern BOOL rfalse(char lTraceType, char bUseEndl, LPCSTR szFormat, ...);

CNetProviderModuleAdv::CNetProviderModuleAdv(BOOL bUseVerify) :
    CNetServerAdv(bUseVerify)
{

}

CNetProviderModuleAdv::~CNetProviderModuleAdv()
{

}

int CNetProviderModuleAdv::Destroy()
{
    Close();
    return 1;
}

int CNetProviderModuleAdv::Execution()
{
    SNetServerPacketAdv *pPacket = NULL;

    extern DWORD dwNetRunLimit;
    DWORD segTime = timeGetTime() + dwNetRunLimit;

    extern unsigned __int64 CycleSeg();
    extern unsigned __int64 CycleEnd();
    extern unsigned __int64 GetCpuSpeed();

    extern LPCSTR GetStringTime();
    extern void TraceMemory(LPVOID, int, unsigned int, LPCSTR);

    while ( (int)(segTime - timeGetTime()) > 0 )
    {
        if (GetMessage(&pPacket) == FALSE)
            break;

        DNID dnidClient = pPacket->m_dnidClient;
        LINKSTATUS enumStatus = pPacket->m_enumStatus;
        LPVOID pContext = pPacket->m_pContext;

        CycleSeg();

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
                    push    pContext
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
                    push    pContext
                    push    pMsg
                    push    enumStatus
                    lea     ecx, dnidClient
                    push    dword ptr [ecx + 4]
                    push    dword ptr [ecx]
                    mov     edx, pOnDispatch
                    mov     ecx, pThis
                    call    edx
                }

                extern DWORD sinsized;
                sinsized += (((DWORD)pPacket->m_wLength) + 512);
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

        double lt = CycleEnd() / (double)GetCpuSpeed();

        if (lt > 0.2)
        {
            rfalse(2, 1, "[lt = %f]", static_cast<float>(lt));
        }

        if (lt > 0.5)
        {
            if (pPacket->m_pvBuffer)
            {
                #define GetByte(index) ((len > index) ? buf[index] : 0)

                int len = pPacket->m_wLength;
                LPBYTE buf = (LPBYTE)pPacket->m_pvBuffer;

                rfalse(1, 1, "run time limit [%s] [lt = %f] [len = %d] "
                    "[info = <%02x %02x %02x %02x %02x %02x %02x %02x>]", 
                    GetStringTime(), (float)lt, len, GetByte(0), GetByte(1), GetByte(2), 
                    GetByte(3), GetByte(4), GetByte(5), GetByte(6), GetByte(7));

                //TraceMemory(buf, 0, len, "RunTimeLimit.Mem");
            }
            else
            {
                rfalse(1, 1, "run time limit [%s] [lt = %f] net break", GetStringTime(), (float)lt);
            }
        }

        if (pPacket->m_pvBuffer)
            delete pPacket->m_pvBuffer;
        delete pPacket;
    }

    CheckAlive();

    return 1;
}

bool CNetProviderModuleAdv::__init(char* szPortName, ...)
{
    va_list cp;

    va_start (cp, szPortName);
    m_pOnDispatch = va_arg (cp, void*);
    m_pThis = va_arg (cp, void*);
    va_end (cp);

    return Create(szPortName) == TRUE;
}
