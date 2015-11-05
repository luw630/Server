#include "stdafx.h"

#include "mmsystem.h"
#pragma comment (lib, "winmm.lib")

#ifdef _DEBUG
#include <crtdbg.h>
#undef _AFX_NO_DEBUG_CRT
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif _DEBUG
typedef const LPBYTE LPCBYTE;

// 根据实际研究，WSASend&WSARecv的lpNumberOfBytesSent是一个在操作结束时会更新的数据，所以，这个数据必须全程有效。。。
static DWORD s_dwTransferredBytes = 0;
static DWORD s_dwFlags = 0;

LPCSTR GetStringTime()
{
    static char buffer[1024];

    SYSTEMTIME s;
    GetLocalTime(&s);

    sprintf(buffer, "%d-%d-%d %d:%d:%d", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);

    return buffer;
}

void Trace(LPCSTR szFormat, ...)
{
    char buffer[1025];

    try
    {
        va_list header;
        va_start(header, szFormat);
        _vsnprintf(buffer, 1024, szFormat, header);
        va_end(header);

        buffer[1024] = 0;

        std::ofstream stream("client_dinfo.txt", std::ios_base::app);
        stream << GetStringTime() << " " << buffer;
    }
    catch (...)
    {
    }
}

////////////
class cAutoSizeMemory
{
    #define ALLOC_STEP_IN_BIT (10)

public:
    cAutoSizeMemory()
    {
        m_pEntry = NULL;
        m_dwSize = 0;
    }

    ~cAutoSizeMemory()
    {
    }

    char * Alloc(DWORD dwSize)
    {
        if (dwSize > m_dwSize)
        {
            Release();

            m_dwSize = ((dwSize >> ALLOC_STEP_IN_BIT) + 1) << ALLOC_STEP_IN_BIT;
            m_pEntry = new char [m_dwSize];
        }

        return m_pEntry;
    }

    void Release()
    {
        if (m_pEntry != NULL)
        {
            delete m_pEntry;
            m_pEntry = NULL;
            m_dwSize = 0;
        }
    }

    operator char * ()
    {
        return m_pEntry;
    }

    DWORD m_dwSize;
    char *m_pEntry;
};
////////////

// the DirectNetProtocol header
struct DNPHDR {
    UINT16 seqnum;
    UINT16 paclen;
};

// the header that is passed one-way from client to server
struct CRYPTION {
    UINT32 crc32;
    DNPHDR dnphdr;
    UINT64 signature;
};

struct OVERLAPPEDPLUS
{
    OVERLAPPED ol;
    SOCKET s;
    int OpCode;
    WSABUF wbuf;
    DWORD dwTransferredBytes;
    DWORD dwTotalBytes;
    DWORD dwBytes;
    DWORD dwFlags;
    DWORD dwSequence;
    LPVOID pvContext;
    DNID dnidClient;

    cAutoSizeMemory pOperationBuffer;

    class cDirectNetClient *pDirectNetClient;
};

#define OP_READ_HEAD        0
#define OP_READ_BODY        1
#define OP_WRITE            2
#define OP_ACCEPT           3

#define OF_CLOSECONNECTION  (0x00000001)

class cDirectNetClient : 
    public iDirectNetClient 
{
public:
    DNRESULT Initialize(iDirectNetCryption* pDirectNetCryption,
                        iDirectNetClientEventHandler* pDirectNetClientEventHandler);

    DNRESULT Connect(LPCSTR szServerIpAddress, DNPORT dnPort);
    DNRESULT Send(PVOID pvBuffer, DWORD dwLength);
    DNRESULT Disconnect();
    DNRESULT Close();

private:
    void RecvThreadProcedure();
    static unsigned int __stdcall s_recv_thread_procedure(void*);

    void SendThreadProcedure();
    static unsigned int __stdcall s_send_thread_procedure(void*);

    void RecvCompletionRoutine(DWORD cbTransferred, OVERLAPPEDPLUS *pOverlapPlus);
    static void __stdcall s_recv_completion_routine(DWORD dwError, 
        DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);

    void SendCompletionRoutine(DWORD cbTransferred, OVERLAPPEDPLUS *pOverlapPlus);
    static void __stdcall s_send_completion_routine(DWORD dwError, 
        DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);

    // void SendApc(ULONG_PTR OVERLAPPEDPLUS *pOverlapPlus);
    static void __stdcall s_send_apc(ULONG_PTR pParamStruct);

    void ErrorHandle(LPCSTR Info, BOOL le = FALSE, int eid = -1);

    void BreakLink();

    BOOL Send();
    BOOL Push(LPVOID pvEntry, WORD wSize);

public:
    cDirectNetClient();
    ~cDirectNetClient();

private:
    SOCKET m_Socket;

    WORD m_wPort;

    BOOL m_bCloseConnection;

    DWORD m_dwIP;

    BOOL m_bUseAutoPackage;

    iDirectNetClientEventHandler *m_pDirectNetClientEventHandler;

    iDirectNetCryption *m_pDirectNetCryption;

    HANDLE m_hSendThread;

    HANDLE m_hThreadExitEvent;
    HANDLE m_hBreakRecvEvent;
    HANDLE m_hBreakSendEvent;

    DWORD m_dwOutSequence;

    CRITICAL_SECTION m_SBLock;

    DWORD m_dwWorkThreadId;
    DWORD m_dwCurBufferSize;
    char m_pSendBuffer[0xffff];
};

unsigned int cDirectNetClient::s_recv_thread_procedure(void* pv_this)
{
    reinterpret_cast<cDirectNetClient*>(pv_this)->RecvThreadProcedure();

    return 0U;
}

unsigned int cDirectNetClient::s_send_thread_procedure(void* pv_this)
{
    reinterpret_cast<cDirectNetClient*>(pv_this)->SendThreadProcedure();

    return 0U;
}

void cDirectNetClient::s_recv_completion_routine(DWORD dwError, 
    DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
    if (lpOverlapped == NULL)
    {
        Trace("null lpOverlapped!!!\r\n");
        return;
    }

    OVERLAPPEDPLUS *pOverlapPlus = CONTAINING_RECORD(lpOverlapped, OVERLAPPEDPLUS, ol);

    if (dwError != 0)
    {
        if (dwError != 995)
        {
            Trace("recv fail dwError = %d\r\n", dwError);
        }

        pOverlapPlus->pDirectNetClient->BreakLink();

        pOverlapPlus->pOperationBuffer.Release();

        delete pOverlapPlus;

        return;
    }

    pOverlapPlus->pDirectNetClient->RecvCompletionRoutine(cbTransferred, pOverlapPlus);
}

void cDirectNetClient::s_send_apc(ULONG_PTR pParamStruct)
{
    OVERLAPPEDPLUS *pOverlapPlus = (OVERLAPPEDPLUS *)pParamStruct;

    if (WSASend(pOverlapPlus->s, &pOverlapPlus->wbuf, 1, &s_dwTransferredBytes, 
        0, &pOverlapPlus->ol, s_send_completion_routine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            pOverlapPlus->pOperationBuffer.Release();

            delete pOverlapPlus;
        }
    }
}

void cDirectNetClient::s_send_completion_routine(DWORD dwError, 
    DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
    if (lpOverlapped == NULL)
    {
        Trace("null lpOverlapped!!!\r\n");
        return;
    }

    OVERLAPPEDPLUS *pOverlapPlus = CONTAINING_RECORD(lpOverlapped, OVERLAPPEDPLUS, ol);

    if (dwError != 0)
    {
        Trace("recv fail dwError = %d\r\n", dwError);

        pOverlapPlus->pOperationBuffer.Release();

        delete pOverlapPlus;

        return;
    }

    pOverlapPlus->pDirectNetClient->SendCompletionRoutine(cbTransferred, pOverlapPlus);
}

iDirectNetClient* iDirectNetClient::CreateInstance()
{
#ifdef _DEBUG
    // Get the current bits
    int tmp = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

    // Clear the upper 16 bits and OR in the desired freqency
    tmp = (tmp & 0x0000FFFF) | _CRTDBG_LEAK_CHECK_DF;

    // Set the new bits
    _CrtSetDbgFlag(tmp);
#endif

    return new cDirectNetClient;
}

void iDirectNetClient::Release()
{
    delete this;
}

cDirectNetClient::cDirectNetClient()
{
    m_Socket = INVALID_SOCKET;
    m_wPort = 0;
    m_dwIP = 0;
    m_pDirectNetClientEventHandler = NULL;
    m_pDirectNetCryption = NULL;
    m_hThreadExitEvent = NULL;
    m_hBreakRecvEvent = NULL;
    m_hBreakSendEvent = NULL;

    m_hSendThread = NULL;
    m_dwOutSequence = 0;
    m_bCloseConnection = FALSE;
    m_bUseAutoPackage = FALSE;
    m_dwCurBufferSize = 0;
    m_dwWorkThreadId = 0;

    WSADATA wsad;
    if (WSAStartup( MAKEWORD(2, 0), &wsad ) != 0)
    {
        ErrorHandle("当前系统并不支持WinSock2.0，相应的网络操作都无法完成！", TRUE);
    }

    InitializeCriticalSection(&m_SBLock);

    m_hThreadExitEvent = CreateEvent(NULL, 0, 0, NULL);
    m_hBreakRecvEvent = CreateEvent(NULL, 0, 0, NULL);
    m_hBreakSendEvent = CreateEvent(NULL, 0, 0, NULL);
}

cDirectNetClient::~cDirectNetClient()
{
    CloseHandle(m_hBreakRecvEvent);
    CloseHandle(m_hBreakSendEvent);
    CloseHandle(m_hThreadExitEvent);

    DeleteCriticalSection(&m_SBLock);

    WSACleanup();
}

void cDirectNetClient::ErrorHandle(LPCSTR info, BOOL le, int eid)
{
    if (le)
    {
        Trace("%s - [%d]\r\n", info, WSAGetLastError());
    }
    else if (eid != -1)
    {
        Trace("%s - [%d]\r\n", info, eid);
    }
    else
    {
        Trace("%s\r\n", info);
    }
}

DNRESULT cDirectNetClient::Initialize(iDirectNetCryption* pDirectCryption, iDirectNetClientEventHandler* pDirectEventHandler)
{
    if (m_pDirectNetClientEventHandler != NULL)
    {
        ErrorHandle("本对象已经初始化过了！");
        return DNR_E_FAIL;
    }

    if (pDirectEventHandler == NULL)
    {
        ErrorHandle("错误的事件响应句柄！");
        return DNR_E_FAIL;
    }

    m_pDirectNetCryption = pDirectCryption;
    m_pDirectNetClientEventHandler = pDirectEventHandler;

    return DNR_S_OK;
}

DNRESULT cDirectNetClient::Connect(LPCSTR szServerIpAddress, DNPORT dnPort)
{
    if (m_pDirectNetClientEventHandler == NULL)
    {
        ErrorHandle("本对象还没有初始化！");
        return DNR_E_NOT_INITIALIZED;
    }

    if (m_dwIP != 0)
    {
        ErrorHandle("本对象已经处于连接状态中！");
        return DNR_E_CONNECT_AGAIN;
    }

    m_dwIP = inet_addr(szServerIpAddress);
    m_wPort = dnPort;
    m_bCloseConnection = FALSE;
    m_dwCurBufferSize = 0;
    m_dwWorkThreadId = 0;
    ResetEvent(m_hThreadExitEvent);

    _beginthreadex(NULL, 0, s_recv_thread_procedure, this, 0, NULL);

    return DNR_S_PENDING;
}

DNRESULT cDirectNetClient::Close()
{
    Disconnect();

    return DNR_S_OK;
}

void cDirectNetClient::RecvThreadProcedure()
{
    m_Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = m_dwIP;
    addr.sin_port = htons(m_wPort);

    bool bConnectSuccess = (connect(m_Socket, (sockaddr *)&addr, sizeof(addr)) != SOCKET_ERROR);

    if (m_pDirectNetClientEventHandler)
    {
        m_pDirectNetClientEventHandler->OnConnectComplete(bConnectSuccess ? DNR_S_OK : DNR_E_FAIL);
    }

    if (!bConnectSuccess)
        return;

    m_dwWorkThreadId = GetCurrentThreadId();

    OVERLAPPEDPLUS *newolp = new OVERLAPPEDPLUS;
    if (newolp == NULL)
    {
        ErrorHandle("无法分配新的内存！");
    }

    memset(newolp, 0, sizeof(OVERLAPPEDPLUS));

    newolp->s = m_Socket;
    newolp->OpCode = OP_READ_HEAD;
    newolp->wbuf.len = newolp->dwTotalBytes = sizeof(DNPHDR);
    newolp->wbuf.buf = newolp->pOperationBuffer.Alloc(sizeof(DNPHDR));
    newolp->pDirectNetClient = this;

    if (newolp->pOperationBuffer == NULL)
    {
        delete newolp;

        ErrorHandle("无法分配新的内存！");
    }

    if (WSARecv(newolp->s, &newolp->wbuf, 1, &s_dwTransferredBytes, 
        &s_dwFlags, &newolp->ol, s_recv_completion_routine) == SOCKET_ERROR)
    {
        int le = WSAGetLastError();
        if (le != WSA_IO_PENDING)
        {
            return;
        }
    }

    m_hSendThread = (HANDLE)_beginthreadex(NULL, 0, s_send_thread_procedure, this, 0, NULL);

    // continuously put the thread to the alertable wait state; the thread will exit
    // when the apc event has been signaled
    SOCKET s = m_Socket;

    while (WaitForSingleObjectEx(m_hBreakRecvEvent, INFINITE, TRUE) == WAIT_IO_COMPLETION) ;

    shutdown(s, SD_BOTH);
    closesocket(s);

    SleepEx(10, TRUE);

    SetEvent(m_hBreakSendEvent);
}

void cDirectNetClient::SendThreadProcedure()
{
    BOOL bRun = TRUE;
    while (bRun) 
    {
        DWORD dw = WaitForSingleObjectEx(m_hBreakSendEvent, 100, TRUE);

        switch (dw)
        {
        case WAIT_IO_COMPLETION:
            continue;

        case WAIT_TIMEOUT:
            if (m_bUseAutoPackage)
                Send();
            break;

        case WAIT_OBJECT_0:
        case WAIT_FAILED:
            bRun = FALSE;
            break;
        }
    }

    SleepEx(10, TRUE);

    SetEvent(m_hThreadExitEvent);
}

void cDirectNetClient::RecvCompletionRoutine(DWORD cbTransferred, OVERLAPPEDPLUS *pOverlapPlus)
{
    if (m_bCloseConnection)
    {
        pOverlapPlus->pOperationBuffer.Release();

        delete pOverlapPlus;

        return;
    }

    if (cbTransferred == 0)
    {
        BreakLink();

        pOverlapPlus->pOperationBuffer.Release();

        delete pOverlapPlus;

        return;
    }

    pOverlapPlus->dwTransferredBytes += cbTransferred;

    if (pOverlapPlus->dwTransferredBytes < pOverlapPlus->dwTotalBytes)
    {
        memset(&pOverlapPlus->ol, 0, sizeof(OVERLAPPED));

        pOverlapPlus->wbuf.buf = &pOverlapPlus->pOperationBuffer[pOverlapPlus->dwTransferredBytes];
        pOverlapPlus->wbuf.len = pOverlapPlus->dwTotalBytes - pOverlapPlus->dwTransferredBytes;
    }
    else
    {
        memset(&pOverlapPlus->ol, 0, sizeof(OVERLAPPED));

        if (pOverlapPlus->OpCode == OP_READ_HEAD)
        {
            DNPHDR dnphdr = *(DNPHDR *)(char *)pOverlapPlus->pOperationBuffer;

            pOverlapPlus->OpCode = OP_READ_BODY;
            pOverlapPlus->wbuf.len = pOverlapPlus->dwTotalBytes = dnphdr.paclen;
        }
        else
        {
            if (m_pDirectNetClientEventHandler)
            {
                m_pDirectNetClientEventHandler->OnReceivePacket(pOverlapPlus->pOperationBuffer, 
                    pOverlapPlus->dwTotalBytes, pOverlapPlus->dwSequence++);
            }

            pOverlapPlus->OpCode = OP_READ_HEAD;
            pOverlapPlus->wbuf.len = pOverlapPlus->dwTotalBytes = sizeof(DNPHDR);
        }

        pOverlapPlus->dwTransferredBytes = 0;
        pOverlapPlus->wbuf.buf = pOverlapPlus->pOperationBuffer.Alloc(pOverlapPlus->dwTotalBytes);

        if (pOverlapPlus->pOperationBuffer == NULL)
        {
            BreakLink();

            ErrorHandle("OP_READ_HEAD::new pOverlapPlus->pOperationBuffer 空指针");

            delete pOverlapPlus;

            return;
        }
    }

    if (WSARecv(pOverlapPlus->s, &pOverlapPlus->wbuf, 1, &s_dwTransferredBytes, 
        &s_dwFlags, &pOverlapPlus->ol, s_recv_completion_routine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            BreakLink();

            pOverlapPlus->pOperationBuffer.Release();

            delete pOverlapPlus;

            return;
        }
    }
}

void cDirectNetClient::SendCompletionRoutine(DWORD cbTransferred, OVERLAPPEDPLUS *pOverlapPlus)
{
    if (cbTransferred == 0)
    {
        pOverlapPlus->pOperationBuffer.Release();

        delete pOverlapPlus;

        return;
    }

    pOverlapPlus->dwTransferredBytes += cbTransferred;

    if (pOverlapPlus->dwTransferredBytes < pOverlapPlus->dwTotalBytes)
    {
        memset(&pOverlapPlus->ol, 0, sizeof(OVERLAPPED));

        pOverlapPlus->wbuf.buf = &pOverlapPlus->pOperationBuffer[pOverlapPlus->dwTransferredBytes];
        pOverlapPlus->wbuf.len = pOverlapPlus->dwTotalBytes - pOverlapPlus->dwTransferredBytes;

        if (WSASend(pOverlapPlus->s, &pOverlapPlus->wbuf, 1, &s_dwTransferredBytes, 
            0, &pOverlapPlus->ol, s_send_completion_routine) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                pOverlapPlus->pOperationBuffer.Release();

                delete pOverlapPlus;

                return;
            }
        }
    }
    else
    {
        if (pOverlapPlus->dwFlags & OF_CLOSECONNECTION)
        {
            BreakLink();
        }

        pOverlapPlus->pOperationBuffer.Release();

        delete pOverlapPlus;
    }
}

DNRESULT cDirectNetClient::Send(PVOID pvBuffer, DWORD dwLength)
{
    // set option
    if (dwLength == 0xf0f0f0f0)
    {
        LPDWORD pParameter = (LPDWORD)pvBuffer;
        switch (pParameter[0])
        {
        case 1:
            m_bUseAutoPackage = (pParameter[1] == 1);
            break;

        default:
            return DNR_E_FAIL;
        }

        return DNR_S_OK;
    }

    if (pvBuffer == NULL || dwLength > 0xf000)
        return DNR_E_FAIL;

    if (m_Socket == INVALID_SOCKET)
        return DNR_E_FAIL;

    if (!Push(pvBuffer, (WORD)dwLength))
        return DNR_E_FAIL;

    if (!m_bUseAutoPackage)
    {
        return Send() ? DNR_S_OK : DNR_E_FAIL;
    }

    return DNR_S_OK;
}

DNRESULT cDirectNetClient::Disconnect()
{
    if (m_Socket == INVALID_SOCKET)
        return DNR_S_OK;

    m_bCloseConnection = TRUE;
    Send("Byebye!", 8);

    WaitForSingleObject(m_hThreadExitEvent, INFINITE);

    return DNR_S_OK;
}

void cDirectNetClient::BreakLink()
{
    if (m_Socket == INVALID_SOCKET)
        return;

    m_Socket = INVALID_SOCKET;

    m_dwIP = 0;
    m_wPort = 0;
    m_bCloseConnection = FALSE;

    SetEvent(m_hBreakRecvEvent);

    if (m_pDirectNetClientEventHandler)
    {
        m_pDirectNetClientEventHandler->OnTerminateSession();
    }
}

BOOL cDirectNetClient::Send()
{
    OVERLAPPEDPLUS *newolp = new OVERLAPPEDPLUS;
    if (newolp == NULL)
    {
        ErrorHandle("无法分配新的内存！");

        return DNR_E_FAIL;
    }

    memset(newolp, 0, sizeof(OVERLAPPEDPLUS));

    newolp->s = m_Socket;
    newolp->OpCode = OP_WRITE;
    newolp->pDirectNetClient = this;

    BOOL ret = FALSE;

    // 这里开始就需要使用临界数据了
    EnterCriticalSection(&m_SBLock);

    if (m_dwCurBufferSize != 0)
    {
        newolp->dwFlags = m_bCloseConnection ? OF_CLOSECONNECTION : 0;

        newolp->wbuf.len = newolp->dwTotalBytes = m_dwCurBufferSize;
        newolp->wbuf.buf = newolp->pOperationBuffer.m_pEntry = 
            new char[newolp->pOperationBuffer.m_dwSize = (((newolp->dwTotalBytes >> 3) + 1) << 3)];

        if (newolp->pOperationBuffer != NULL)
        {
            memcpy(newolp->pOperationBuffer, m_pSendBuffer, m_dwCurBufferSize);

            m_dwCurBufferSize = 0;

            ret = TRUE;
        }
        else
        {
            delete newolp;

            ErrorHandle("无法分配新的内存！");
        }
    }

    LeaveCriticalSection(&m_SBLock);

    if (ret)
    {
        if (GetCurrentThreadId() == m_dwWorkThreadId)
        {
            if (WSASend(newolp->s, &newolp->wbuf, 1, &s_dwTransferredBytes, 
                0, &newolp->ol, s_send_completion_routine) == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSA_IO_PENDING)
                {
                    newolp->pOperationBuffer.Release();

                    delete newolp;

                    ret = FALSE;
                }
            }
        }
        else
        {
            QueueUserAPC(s_send_apc, m_hSendThread, (ULONG_PTR)newolp);
        }
    }

    return ret;
}

BOOL cDirectNetClient::Push(LPVOID pvEntry, WORD wSize)
{
    if (pvEntry == NULL || wSize == 0 || wSize > 0xf000)
        return FALSE;

    if (m_Socket == INVALID_SOCKET)
        return FALSE;

    BOOL ret = FALSE;

    EnterCriticalSection(&m_SBLock);

    if ((m_dwCurBufferSize + wSize) < 0xf000)
    {
        CRYPTION *pHeader = (CRYPTION *)&m_pSendBuffer[m_dwCurBufferSize];

        pHeader->dnphdr.seqnum = (UINT16)m_dwOutSequence++;
        pHeader->dnphdr.paclen = (UINT16)wSize;

        UINT64 u64; QueryPerformanceCounter((LARGE_INTEGER *)&u64);
        UINT32 u32 = timeGetTime();

        pHeader->signature = ((u64 << 32) | u32);

        if (m_pDirectNetCryption)
        {
            pHeader->crc32 = m_pDirectNetCryption->CRC32_compute( 
                (char*)pHeader + sizeof(pHeader->crc32), sizeof(CRYPTION) - sizeof(pHeader->crc32));

            m_pDirectNetCryption->DES_encrypt(pHeader, sizeof(CRYPTION));
        }

        m_dwCurBufferSize += sizeof(CRYPTION);

        memcpy(&m_pSendBuffer[m_dwCurBufferSize], pvEntry, wSize);

        m_dwCurBufferSize += wSize;

        ret = TRUE;
    }

    LeaveCriticalSection(&m_SBLock);

    return ret;
}
