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

#define OP_READ_HEAD        0
#define OP_READ_BODY        1

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

    void ErrorHandle(LPCSTR Info, BOOL le = FALSE, int eid = -1);

    void BreakLink();

    BOOL Recv();
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

    DWORD m_dwOpCode;

    BOOL m_bUseAutoPackage;

    iDirectNetClientEventHandler *m_pDirectNetClientEventHandler;

    iDirectNetCryption *m_pDirectNetCryption;

    HANDLE m_hThreadExitEvent;
    HANDLE m_hBreakRecvEvent;
    HANDLE m_hBreakSendEvent;

    DWORD m_dwOutSequence;
    DWORD m_dwInSequence;

    CRITICAL_SECTION m_SBLock;

    DWORD m_dwWorkThreadId;
    DWORD m_dwCurBufferSize;
    char m_pSendBuffer[0xffff];

    DWORD m_dwCurTransSize;
    DWORD m_dwTotalTransSize;
    char m_pRecvBuffer[0xffff];
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
    m_dwOpCode = OP_READ_HEAD;

    m_pDirectNetClientEventHandler = NULL;
    m_pDirectNetCryption = NULL;

    m_hThreadExitEvent = NULL;
    m_hBreakRecvEvent = NULL;
    m_hBreakSendEvent = NULL;

    m_dwOutSequence = 0;
    m_dwInSequence = 0;
    m_bCloseConnection = FALSE;
    m_bUseAutoPackage = FALSE;
    m_dwCurBufferSize = 0;

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
    m_Socket = socket(AF_INET, SOCK_STREAM, 0);

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

    m_dwOpCode = OP_READ_HEAD;
    m_dwTotalTransSize = sizeof(DNPHDR);
    m_dwCurTransSize = 0;
    m_dwInSequence = 0;

    SOCKET s = m_Socket;

    HANDLE m_hNetNotifyEvent = WSACreateEvent();

    WSAEventSelect(s, m_hNetNotifyEvent, FD_READ);

    WSANETWORKEVENTS event;

    _beginthreadex(NULL, 0, s_send_thread_procedure, this, 0, NULL);

    HANDLE waitEvent[2] = {m_hNetNotifyEvent, m_hBreakSendEvent};

    while (TRUE) 
    {
        DWORD ret = WSAWaitForMultipleEvents(2, waitEvent, FALSE, WSA_INFINITE, FALSE);

        if (ret != (WAIT_OBJECT_0))
        {
            break;
        }

        if (WSAEnumNetworkEvents(s, m_hNetNotifyEvent, &event) != 0)
        {
            ErrorHandle("WSAEnumNetworkEvents fail", TRUE);
            break;
        }

        if (event.lNetworkEvents != FD_READ)
        {
            ErrorHandle("unregistered fd_type!");
            break;
        }

        if (event.iErrorCode[FD_READ_BIT] != 0)
        {
            ErrorHandle("recv fail", FALSE, event.iErrorCode[FD_READ_BIT]);
        }

        Recv();
    }

    shutdown(s, SD_BOTH);
    closesocket(s);

    SetEvent(m_hBreakSendEvent);
}

void cDirectNetClient::SendThreadProcedure()
{
    while (TRUE) 
    {
        if (WaitForSingleObject(m_hBreakSendEvent, 100) == WAIT_TIMEOUT)
        {
            if (m_bUseAutoPackage)
                Send();
        }
        else
        {
            break;
        }
    }

    SetEvent(m_hThreadExitEvent);
}

BOOL cDirectNetClient::Recv()
{
    int cbTransferred = recv(m_Socket, &m_pRecvBuffer[m_dwCurTransSize], 
        m_dwTotalTransSize - m_dwCurTransSize, 0);

    if (cbTransferred == 0)
    {
        BreakLink();

        return FALSE;
    }

    if (cbTransferred == SOCKET_ERROR)
    {
        BreakLink();

        ErrorHandle("recv fail", TRUE);

        return FALSE;
    }

    m_dwCurTransSize += cbTransferred;

    if (m_dwCurTransSize > m_dwTotalTransSize)
    {
        BreakLink();

        ErrorHandle("m_dwCurTransSize > m_dwTotalTransSize !!!");

        return FALSE;
    }

    if (m_dwCurTransSize < m_dwTotalTransSize)
    {
        return TRUE;
    }

    if (m_dwOpCode == OP_READ_HEAD)
    {
        DNPHDR dnphdr = *(DNPHDR *)(char *)m_pRecvBuffer;

        m_dwOpCode = OP_READ_BODY;
        m_dwTotalTransSize = dnphdr.paclen;
        m_dwCurTransSize = 0;

        return TRUE;
    }

    if (m_pDirectNetClientEventHandler)
    {
        m_pDirectNetClientEventHandler->OnReceivePacket(m_pRecvBuffer, 
            m_dwTotalTransSize, m_dwInSequence++);
    }

    m_dwOpCode = OP_READ_HEAD;
    m_dwTotalTransSize = sizeof(DNPHDR);
    m_dwCurTransSize = 0;

    return TRUE;
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
    BOOL ret = FALSE;
    // 这里开始就需要使用临界数据了
    EnterCriticalSection(&m_SBLock);

    if (send(m_Socket, m_pSendBuffer, m_dwCurBufferSize, 0) != SOCKET_ERROR)
    {
        m_dwCurBufferSize = 0;

        ret = TRUE;
    }
    else
    {
        ErrorHandle("send fail", TRUE);
    }

    LeaveCriticalSection(&m_SBLock);

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
