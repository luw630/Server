#include "stdafx.h"

#include "pub/dwt.h"

#include "mmsystem.h"
#pragma comment (lib, "winmm.lib")

// 根据实际研究，WSASend&WSARecv的lpNumberOfBytesSent是一个在操作结束时会更新的数据，所以，这个数据必须全程有效。。。
static DWORD s_dwTransferredBytes = 0;
static DWORD s_dwFlags = 0;

static std::ofstream dout("client_dinfo.txt", std::ios_base::trunc);

void Trace(LPCSTR szFormat, ...)
{
    char timebuf[128];
    char buffer[1025];

    SYSTEMTIME s;
    GetLocalTime(&s);
    sprintf(timebuf, "[%d-%d-%d %d:%d:%d]", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);

    try
    {
        va_list header;
        va_start(header, szFormat);
        _vsnprintf(buffer, 1024, szFormat, header);
        va_end(header);

        buffer[1024] = 0;

        //std::ofstream stream("client_dinfo.txt", std::ios_base::app);
        //stream << timebuf << buffer;
        dout << timebuf << buffer;
        dout.flush();
    }
    catch (...)
    {
        //std::ofstream stream("client_dinfo.txt", std::ios_base::app);
        //stream << "err in try \r\n";
        dout << "err in try \r\n";
        dout.flush();
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

    char * RealAlloc(DWORD dwSize)
    {
        if (dwSize > m_dwSize)
        {
            Release();

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

struct sOpPackage
{
    int OpCode;
    DWORD dwTransferredBytes;
    DWORD dwTotalBytes;

    cAutoSizeMemory pOperationBuffer;
};

#define OP_READ_HEAD        0
#define OP_READ_BODY        1
#define OP_WRITE            2

class cDirectNetClient : 
    public iDirectNetClient 
{
public:
    DNRESULT Initialize(iDirectNetCryption *pDirectNetCryption,
                        iDirectNetClientEventHandler *pDirectNetClientEventHandler);

    DNRESULT Connect(LPCSTR szServerIpAddress, DNPORT dnPort);
    DNRESULT Send(PVOID pvBuffer, DWORD dwLength);
    DNRESULT Disconnect();
    DNRESULT Close();

private:
    void MainThreadProcedure();
    static DWORD WINAPI s_main_thread_procedure(void *);

    void SendThreadProcedure();
    static DWORD WINAPI s_send_thread_procedure(void *);

    void RecvCompletionRoutine(DWORD cbTransferred, sOpPackage *lpOp);
    void SendCompletionRoutine(DWORD cbTransferred, sOpPackage *lpOp);

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

    DWORD m_dwIP;
    BOOL m_bUseAutoPackage;

    iDirectNetClientEventHandler *m_pDirectNetClientEventHandler;
    iDirectNetCryption *m_pDirectNetCryption;

    HANDLE m_hWorkThread;
    DWORD m_dwInSequence;
    DWORD m_dwOutSequence;

    // this objectptr managed by s_work_thread_procedure
    dwt::mtQueue<sOpPackage> *m_lpOverQueue;

    CRITICAL_SECTION m_SBLock;
    DWORD m_dwCurBufferSize;
    char m_pSendBuffer[0xffff];

    DWORD dwPrevCheckTime;
    DWORD P_dwInSize;   // Per time
    DWORD P_dwOutSize;
    DWORD F_dwInSize;   // Frame time
    DWORD F_dwOutSize;
    DWORD T_dwInSize;   // Total time
    DWORD T_dwOutSize;
};

DWORD cDirectNetClient::s_main_thread_procedure(void *pv_this)
{
    reinterpret_cast<cDirectNetClient*>(pv_this)->MainThreadProcedure();

    return 0U;
}

DWORD cDirectNetClient::s_send_thread_procedure(void *pv_this)
{
    reinterpret_cast<cDirectNetClient*>(pv_this)->SendThreadProcedure();

    return 0U;
}

iDirectNetClient* iDirectNetClient::CreateInstance()
{
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
    m_hWorkThread = NULL;
    m_dwInSequence = 0;
    m_dwOutSequence = 0;
    m_bUseAutoPackage = FALSE;
    m_dwCurBufferSize = 0;
    m_lpOverQueue = NULL;

    P_dwInSize = 0;   // Per time
    P_dwOutSize = 0;
    F_dwInSize = 0;   // Frame time
    F_dwOutSize = 0;
    T_dwInSize = 0;   // Total time
    T_dwOutSize = 0;

    WSADATA wsad;
    if (WSAStartup( MAKEWORD(2, 2), &wsad ) != 0)
    {
        ErrorHandle("当前系统并不支持WinSock2.2，相应的网络操作都无法完成！", TRUE);
    }

    InitializeCriticalSection(&m_SBLock);
}

cDirectNetClient::~cDirectNetClient()
{
    Disconnect();

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

    if ((m_dwIP != 0) || (m_hWorkThread != NULL) || (m_lpOverQueue != NULL))
    {
        ErrorHandle("本对象已经处于连接状态中！");
        return DNR_E_CONNECT_AGAIN;
    }

    m_dwIP = inet_addr(szServerIpAddress);
    m_wPort = dnPort;
    m_dwCurBufferSize = 0;

    DWORD dwThreadID = 0;
    m_hWorkThread = CreateThread(NULL, 0, s_main_thread_procedure, this, 0, &dwThreadID);

    return DNR_S_PENDING;
}

DNRESULT cDirectNetClient::Close()
{
    Disconnect();

    return DNR_S_OK;
}

void cDirectNetClient::MainThreadProcedure()
{
    m_Socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = m_dwIP;
    addr.sin_port = htons(m_wPort);

    Trace("连接服务器 %08x:%d\r\n", m_dwIP, m_wPort);
    bool bConnectSuccess = (connect(m_Socket, (sockaddr *)&addr, sizeof(addr)) != SOCKET_ERROR);
    Trace("连接%s\r\n", bConnectSuccess?"成功":"失败");

    if (m_pDirectNetClientEventHandler)
    {
        m_pDirectNetClientEventHandler->OnConnectComplete(bConnectSuccess ? DNR_S_OK : DNR_E_FAIL);
    }

    if (!bConnectSuccess)
    {
        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
        return;
    }

    WSAEVENT wsaNetEvent = WSACreateEvent();

    WSAEventSelect(m_Socket, wsaNetEvent, FD_READ|FD_WRITE|FD_CLOSE);

    WSANETWORKEVENTS wsaNetEventData;

    sOpPackage RecvOpPackage;
    RecvOpPackage.OpCode = OP_READ_HEAD;
    RecvOpPackage.dwTransferredBytes = 0;
    RecvOpPackage.dwTotalBytes = sizeof(DNPHDR);
    RecvOpPackage.pOperationBuffer.Alloc(sizeof(DNPHDR));

    SOCKET tempSocket = m_Socket;
    DWORD dwThreadID = 0;

    m_lpOverQueue = new dwt::mtQueue<sOpPackage>();
    m_lpOverQueue->PushExitEvent(CreateThread(NULL, 0, s_send_thread_procedure, this, 0, &dwThreadID));

    while (TRUE)
    {
        if (WSAWaitForMultipleEvents(1, &wsaNetEvent, FALSE, WSA_INFINITE, FALSE) == WSA_WAIT_FAILED)
        {
            ErrorHandle("工作线程终止在WSAWaitForMultipleEvents！", TRUE, WSAGetLastError());
            break;
        }

        if (WSAEnumNetworkEvents(tempSocket, wsaNetEvent, &wsaNetEventData) == SOCKET_ERROR)
        {
            ErrorHandle("工作线程终止在WSAEnumNetworkEvents！", TRUE, WSAGetLastError());
            break;
        }

        if (wsaNetEventData.lNetworkEvents & FD_READ)
        {
            if (wsaNetEventData.iErrorCode[FD_READ_BIT] != 0)
            {
                ErrorHandle("FD_READ have error", TRUE, wsaNetEventData.iErrorCode[FD_READ_BIT]);
            }
            else
            {
                int cbTransferred = recv(tempSocket, 
                    &RecvOpPackage.pOperationBuffer[RecvOpPackage.dwTransferredBytes],
                    RecvOpPackage.dwTotalBytes - RecvOpPackage.dwTransferredBytes, 0);

                Trace("RECV %d\r\n", cbTransferred);

                if (cbTransferred == SOCKET_ERROR)
                {
                    ErrorHandle("获取数据失败！", TRUE, WSAGetLastError());
                }
                else
                {
                    RecvCompletionRoutine((DWORD)cbTransferred, &RecvOpPackage);
                }
            }
        }

        if (wsaNetEventData.lNetworkEvents & FD_WRITE)
        {
            if (wsaNetEventData.iErrorCode[FD_WRITE_BIT] != 0)
            {
                ErrorHandle("FD_WRITE have error", TRUE, wsaNetEventData.iErrorCode[FD_WRITE_BIT]);
            }
            continue;
        }
        
        if (wsaNetEventData.lNetworkEvents & FD_CLOSE)
        {
            while (TRUE)
            {
                int cbTransferred = recv(tempSocket, 
                    &RecvOpPackage.pOperationBuffer[RecvOpPackage.dwTransferredBytes],
                    RecvOpPackage.dwTotalBytes - RecvOpPackage.dwTransferredBytes, 0);

                if ((cbTransferred == SOCKET_ERROR) || (cbTransferred == 0))
                    break;

                RecvCompletionRoutine((DWORD)cbTransferred, &RecvOpPackage);
            }

            ErrorHandle("Network Shutdown", TRUE, wsaNetEventData.iErrorCode[FD_CLOSE_BIT]);
            closesocket(tempSocket);
            tempSocket = INVALID_SOCKET;
            break;
        }
    }

    RecvOpPackage.pOperationBuffer.Release();

    delete m_lpOverQueue;
    m_lpOverQueue = NULL;

    if (!WSACloseEvent(wsaNetEvent))
    {
        ErrorHandle("error in WSACloseEvent！", TRUE, WSAGetLastError());
    }

    if (m_pDirectNetClientEventHandler)
    {
        m_pDirectNetClientEventHandler->OnTerminateSession();
    }
}

void cDirectNetClient::SendThreadProcedure()
{
    dwPrevCheckTime = timeGetTime();

    sOpPackage opp;
    while (!m_lpOverQueue->isEnd())
    {
        if ((int)timeGetTime() - (int)dwPrevCheckTime > 200)
        {
            dwPrevCheckTime = timeGetTime();

            T_dwInSize += (F_dwInSize = P_dwInSize);
            T_dwOutSize += (F_dwOutSize = P_dwOutSize);
            P_dwInSize = 0;
            P_dwOutSize = 0;
        }

        if (m_Socket == INVALID_SOCKET)
            break;

        opp.dwTotalBytes = -1;

        m_lpOverQueue->Lock();
        if (!m_lpOverQueue->empty())
        {
            opp = m_lpOverQueue->front();
            m_lpOverQueue->pop();
        }
        m_lpOverQueue->Unlock();

        if (opp.dwTotalBytes != -1)
        {
            int cbTransferred = send(m_Socket, 
                &opp.pOperationBuffer[opp.dwTransferredBytes], 
                opp.dwTotalBytes - opp.dwTransferredBytes, 0);

            if (cbTransferred == SOCKET_ERROR)
            {
                ErrorHandle("发送数据失败！", TRUE, WSAGetLastError());
            }
            else
            {
                SendCompletionRoutine((DWORD)cbTransferred, &opp);
            }
        }

        if (m_bUseAutoPackage)
        {
            Sleep(100);
            Send();
        }
        else
        {
            Sleep(1);
        }
    }

    m_lpOverQueue->Lock();
    while (!m_lpOverQueue->empty())
    {
        m_lpOverQueue->front().pOperationBuffer.Release();
        m_lpOverQueue->pop();
    }
    m_lpOverQueue->Unlock();
}

void cDirectNetClient::RecvCompletionRoutine(DWORD cbTransferred, sOpPackage *lpOp)
{
    P_dwInSize += cbTransferred;

    if (cbTransferred == 0)
    {
        BreakLink();
        return;
    }

    lpOp->dwTransferredBytes += cbTransferred;

    if (lpOp->dwTransferredBytes > lpOp->dwTotalBytes)
    {
        ErrorHandle("invalid received data size!!!");
        return;
    }

    if (lpOp->dwTransferredBytes < lpOp->dwTotalBytes)
        return;

    Trace("IN %d\r\n", cbTransferred);

    if (lpOp->OpCode == OP_READ_HEAD)
    {
        DNPHDR dnphdr = *(DNPHDR *)(char *)lpOp->pOperationBuffer;

        if (dnphdr.seqnum != (WORD)m_dwInSequence)
        {
            m_dwInSequence = dnphdr.seqnum;
            ErrorHandle("detected package lost !!!");
        }

        lpOp->OpCode = OP_READ_BODY;
        lpOp->dwTotalBytes = dnphdr.paclen;
    }
    else
    {
        if (m_pDirectNetClientEventHandler)
        {
            Trace("onrecv %d\r\n", cbTransferred);

            m_pDirectNetClientEventHandler->OnReceivePacket(lpOp->pOperationBuffer, 
                lpOp->dwTotalBytes, m_dwInSequence++);
        }

        lpOp->OpCode = OP_READ_HEAD;
        lpOp->dwTotalBytes = sizeof(DNPHDR);
    }

    lpOp->dwTransferredBytes = 0;
    lpOp->pOperationBuffer.Alloc(lpOp->dwTotalBytes);

    if (lpOp->pOperationBuffer == NULL)
    {
        BreakLink();
        ErrorHandle("OP_READ_HEAD::new pOverlapPlus->pOperationBuffer 空指针");
        return;
    }
}

void cDirectNetClient::SendCompletionRoutine(DWORD cbTransferred, sOpPackage *lpOp)
{
    P_dwOutSize += cbTransferred;

__Retry:

    if (m_Socket == INVALID_SOCKET)
    {
        lpOp->pOperationBuffer.Release();
        return;
    }

    if (cbTransferred == 0)
    {
        lpOp->pOperationBuffer.Release();
        return;
    }

    lpOp->dwTransferredBytes += cbTransferred;

    if (lpOp->dwTransferredBytes > lpOp->dwTotalBytes)
    {
        ErrorHandle("invalid sended data size!!!");
        lpOp->pOperationBuffer.Release();
        return;
    }

    if (lpOp->dwTransferredBytes < lpOp->dwTotalBytes)
    {
        // 因为发送失败，同时又不能破坏数据序，所以只能接着发送！
        int ret = send(m_Socket, 
            &lpOp->pOperationBuffer[lpOp->dwTransferredBytes], 
            lpOp->dwTotalBytes - lpOp->dwTransferredBytes, 0);

        if (ret == SOCKET_ERROR)
        {
            ErrorHandle("send fail in retry!!!");
            lpOp->pOperationBuffer.Release();
            return;
        }

        cbTransferred = (DWORD)ret;

        goto __Retry;
        //ErrorHandle("invalid received data size!!!");
        //return;
    }

    lpOp->pOperationBuffer.Release();
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

        case 2:
            pParameter[0] = F_dwInSize;
            pParameter[1] = F_dwOutSize;
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
    {
        if (!Send())
        {
            return DNR_E_FAIL;
        }

        if (!Push(pvBuffer, (WORD)dwLength))
        {
            return DNR_E_FAIL;
        }
    }

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

    BreakLink();

    WaitForSingleObject(m_hWorkThread, INFINITE);
    m_hWorkThread = NULL;

    return DNR_S_OK;
}

void cDirectNetClient::BreakLink()
{
    if (m_dwIP == 0)
        return;

    m_dwIP = 0;
    m_wPort = 0;

    ErrorHandle("initiative shutdown");

    shutdown(m_Socket, SD_BOTH);

    m_Socket = INVALID_SOCKET;
}

BOOL cDirectNetClient::Send()
{
    sOpPackage opp;
    BOOL bHaveInfo = FALSE;

    EnterCriticalSection(&m_SBLock);
    if (m_dwCurBufferSize != 0)
    {
        bHaveInfo = TRUE;

        opp.OpCode = OP_WRITE;
        opp.dwTransferredBytes = 0;
        opp.dwTotalBytes = m_dwCurBufferSize;
        opp.pOperationBuffer.Alloc(m_dwCurBufferSize);

        if (opp.pOperationBuffer != NULL)
        {
            memcpy(opp.pOperationBuffer, m_pSendBuffer, m_dwCurBufferSize);
            m_dwCurBufferSize = 0;
        }
    }
    LeaveCriticalSection(&m_SBLock);

    if (!bHaveInfo)
    {
        // 没有数据需要发送！！！
        return FALSE;
    }

    if (opp.pOperationBuffer == NULL)
    {
        ErrorHandle("无法分配新的内存！");
        return FALSE;
    }

    m_lpOverQueue->push(opp);
    return TRUE;
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
