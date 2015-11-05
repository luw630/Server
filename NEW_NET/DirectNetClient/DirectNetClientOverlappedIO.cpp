#include "stdafx.h"

#include "pub/dwt.h"

#include "mmsystem.h"
#pragma comment (lib, "winmm.lib")

// 根据实际研究，WSASend&WSARecv的lpNumberOfBytesSent是一个在操作结束时会更新的数据，所以，这个数据必须全程有效。。。
//static DWORD s_dwTransferredBytes = 0;
//static DWORD s_dwFlags = 0;

struct XcptBase
{
    virtual LPCSTR GetErrInfo() = 0;

    XcptBase(DWORD dwError) : dwErrCode(dwError) {}

    DWORD dwErrCode;
};

#define DeclareXcpt(XcptName, dwErrCode, szInfo) class XcptName : public XcptBase { \
    public : XcptName() : XcptBase(dwErrCode) {} LPCSTR GetErrInfo() { return szInfo; } };

DeclareXcpt(NullPointer,    0x00000000, "非法使用空指针")
DeclareXcpt(SendFail,       0x00000001, "发送数据失败")
DeclareXcpt(RecvFail,       0x00000002, "接收数据失败")

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

        std::ofstream stream("client_dinfo.txt", std::ios_base::app);
        stream << timebuf << buffer;
    }
    catch (...)
    {
        std::ofstream stream("client_dinfo.txt", std::ios_base::app);
        stream << "err in try \r\n";
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

            m_pEntry = new char [m_dwSize = dwSize];
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
    sOpPackage(int type, DWORD size, LPVOID data = NULL)
    {
        memset(&ol, 0, sizeof(ol));
        dwTransferredBytes = 0;
        dwFlags = 0;
        cbTransferred = 0;

        OpCode = type;
        dwTotalBytes = size;
        pOperationBuffer.RealAlloc(size);

        if (data != NULL)
        {
            memcpy(pOperationBuffer, data, size);
        }
    }

    ~sOpPackage()
    {
        pOperationBuffer.Release();
    }

    void DoRecv(SOCKET s, WSAEVENT wsaEvent)
    {
        memset(&ol, 0, sizeof(WSAOVERLAPPED));
        dwFlags = 0;
        cbTransferred = 0;
        ol.hEvent = wsaEvent;

        WSABUF RecvBuf;
        RecvBuf.buf = &pOperationBuffer[dwTransferredBytes];
        RecvBuf.len = dwTotalBytes - dwTransferredBytes;

        if (WSARecv(s, &RecvBuf, 1, &cbTransferred, 
            &dwFlags, NULL, NULL) == SOCKET_ERROR)
        {
            int le = WSAGetLastError();
            if (le != WSA_IO_PENDING)
            {
                Trace("接收数据失败！ - [%d]\r\n", le);
                throw RecvFail();
            }
        }
    }

    void DoSend(SOCKET s, WSAEVENT wsaEvent)
    {
        memset(&ol, 0, sizeof(WSAOVERLAPPED));
        dwFlags = 0;
        cbTransferred = 0;
        ol.hEvent = wsaEvent;

        WSABUF RecvBuf;
        RecvBuf.buf = &pOperationBuffer[dwTransferredBytes];
        RecvBuf.len = dwTotalBytes - dwTransferredBytes;

        if (WSASend(s, &RecvBuf, 1, &cbTransferred, 
            dwFlags, NULL, NULL) == SOCKET_ERROR)
        {
            int le = WSAGetLastError();
            if (le != WSA_IO_PENDING)
            {
                Trace("发送数据失败！ - [%d]\r\n", le);
                throw SendFail();
            }
        }
    }

public:
    WSAOVERLAPPED ol;

    int OpCode;
    DWORD dwTransferredBytes;
    DWORD dwTotalBytes;

    DWORD dwFlags;
    DWORD cbTransferred;

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
    dwt::mtQueue<sOpPackage *> *m_lpOverQueue;
    WSAEVENT wsaRecvEvent;
    WSAEVENT wsaSendEvent;

    CRITICAL_SECTION m_SBLock;
    DWORD m_dwCurBufferSize;
    char m_pSendBuffer[0xffff];
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

    WSADATA wsad;
    if (WSAStartup( MAKEWORD(2, 2), &wsad ) != 0)
    {
        ErrorHandle("当前系统并不支持WinSock2.2，相应的网络操作都无法完成！", TRUE);
    }

    InitializeCriticalSection(&m_SBLock);

    wsaRecvEvent = WSACreateEvent();
    wsaSendEvent = WSACreateEvent();
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
    {
        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
        return;
    }

    DWORD dwThreadID = 0;
    m_lpOverQueue = new dwt::mtQueue<sOpPackage *>();
    m_lpOverQueue->PushExitEvent(CreateThread(NULL, 0, s_send_thread_procedure, this, 0, &dwThreadID));

    sOpPackage RecvOpPackage(OP_READ_HEAD, sizeof(DNPHDR));

    while (TRUE)
    {
        try
        {
            RecvOpPackage.DoRecv(m_Socket, wsaRecvEvent);
        }
        catch (RecvFail &err)
        {
            Trace("接收线程终止在DoRecv！[%s]", err.GetErrInfo());
            break;
        }

        //if (WSAWaitForMultipleEvents(1, &wsaRecvEvent, FALSE, WSA_INFINITE, FALSE) == WSA_WAIT_FAILED)
        //{
        //    ErrorHandle("接收线程终止在WSAWaitForMultipleEvents！", TRUE, WSAGetLastError());
        //    break;
        //}

        //DWORD cbTransferred = 0;
        //DWORD dwFlags = 0;
        //if (!WSAGetOverlappedResult(m_Socket, &RecvOpPackage.ol, &cbTransferred, TRUE, &dwFlags))
        //{
        //    ErrorHandle("接收线程终止在WSAGetOverlappedResult！", TRUE, WSAGetLastError());
        //    break;
        //}

        DWORD cbTransferred = RecvOpPackage.cbTransferred;

        if (cbTransferred == 0)
        {
            // ErrorHandle("接收线程正常终止！");
            break;
        }

        RecvCompletionRoutine(cbTransferred, &RecvOpPackage);
    }

    delete m_lpOverQueue;
    m_lpOverQueue = NULL;

_EndThread:
    RecvOpPackage.pOperationBuffer.Release();

    if (!WSACloseEvent(wsaRecvEvent))
    {
        ErrorHandle("error in WSACloseEvent！", TRUE, WSAGetLastError());
    }

    if (m_pDirectNetClientEventHandler)
    {
        m_pDirectNetClientEventHandler->OnTerminateSession();
    }

    closesocket(m_Socket);
    m_Socket = INVALID_SOCKET;
}

void cDirectNetClient::SendThreadProcedure()
{
    while (!m_lpOverQueue->isEnd())
    {
        Sleep(1);

        if (m_bUseAutoPackage)
        {
            Send();
            Sleep(99);
        }

        sOpPackage *lpOp = NULL;

        m_lpOverQueue->Lock();
        if (!m_lpOverQueue->empty())
        {
            lpOp = m_lpOverQueue->front();
            m_lpOverQueue->pop();
        }
        m_lpOverQueue->Unlock();

        if (lpOp == NULL)
            continue;

        try
        {
            while (lpOp->dwTransferredBytes < lpOp->dwTotalBytes)
            {
                lpOp->DoSend(m_Socket, wsaSendEvent);

                //if (WSAWaitForMultipleEvents(1, &wsaSendEvent, FALSE, WSA_INFINITE, FALSE) == WSA_WAIT_FAILED)
                //{
                //    ErrorHandle("发送线程终止在WSAWaitForMultipleEvents！", TRUE, WSAGetLastError());
                //    break;
                //}

                //DWORD cbTransferred = 0;
                //DWORD dwFlags = 0;
                //if (!WSAGetOverlappedResult(m_Socket, &lpOp->ol, &cbTransferred, TRUE, &dwFlags))
                //{
                //    ErrorHandle("发送线程终止在WSAGetOverlappedResult！", TRUE, WSAGetLastError());
                //    break;
                //}

                DWORD cbTransferred = lpOp->cbTransferred;

                SendCompletionRoutine(cbTransferred, lpOp);
            }
        }
        catch (SendFail &err)
        {
            Trace("发送线程终止在DoSend！[%s]\r\n", err.GetErrInfo());
        }

        delete lpOp;
    }

    m_lpOverQueue->Lock();
    while (!m_lpOverQueue->empty())
    {
        delete m_lpOverQueue->front();
        m_lpOverQueue->pop();
    }
    m_lpOverQueue->Unlock();

    // ErrorHandle("发送线程正常终止！");
}

void cDirectNetClient::RecvCompletionRoutine(DWORD cbTransferred, sOpPackage *lpOp)
{
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
    if (m_Socket == INVALID_SOCKET)
        return;

    if (cbTransferred == 0)
        return;

    lpOp->dwTransferredBytes += cbTransferred;

    if (lpOp->dwTransferredBytes > lpOp->dwTotalBytes)
    {
        ErrorHandle("invalid sended data size!!!");
        return;
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
    if (m_hWorkThread == NULL)
        return DNR_S_OK;

    BreakLink();

    WaitForSingleObject(m_hWorkThread, INFINITE);
    m_hWorkThread = NULL;

    return DNR_S_OK;
}

void cDirectNetClient::BreakLink()
{
    if (m_hWorkThread == NULL)
        return;

    m_dwIP = 0;
    m_wPort = 0;

    // ErrorHandle("initiative shutdown");

    shutdown(m_Socket, SD_BOTH);

    WSASetEvent(wsaRecvEvent);
    WSASetEvent(wsaSendEvent);
}

BOOL cDirectNetClient::Send()
{
    sOpPackage *lpOp = NULL;

    EnterCriticalSection(&m_SBLock);
    if (m_dwCurBufferSize != 0)
    {
        lpOp = new sOpPackage(OP_WRITE, m_dwCurBufferSize, m_pSendBuffer);
        m_dwCurBufferSize = 0;
    }
    LeaveCriticalSection(&m_SBLock);

    if (lpOp == NULL)
        return FALSE;

    m_lpOverQueue->push(lpOp);

    return TRUE;
}

BOOL cDirectNetClient::Push(LPVOID pvEntry, WORD wSize)
{
    if (pvEntry == NULL || wSize == 0 || wSize > 0xf000)
        return FALSE;

    if (m_hWorkThread == NULL)
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
