#include "stdafx.h"

#include "Mswsock.h"
#pragma comment (lib, "Mswsock.lib")

#include "mmsystem.h"
#pragma comment (lib, "winmm.lib")

#include "..\..\OrbFramework\ObjectService2.h"
#include <list>

#ifdef _DEBUG
#include <crtdbg.h>
#undef _AFX_NO_DEBUG_CRT
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif _DEBUG
typedef const LPBYTE LPCBYTE;
#define SEND_MARGIN 100

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

        std::ofstream stream("server_dinfo.txt", std::ios_base::app);
        stream << GetStringTime() << buffer;
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
        Release();
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
            delete [] m_pEntry;
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
    DWORD dwFlags;
    DWORD dwSequence;
    LPVOID pvContext;
    DNID dnidClient;

    DWORD dwRecvBytes;
    DWORD dwRecvTimes;

    cAutoSizeMemory pOperationBuffer;
};

#define OP_READ_HEAD        0
#define OP_READ_BODY        1
#define OP_WRITE            2
#define OP_ACCEPT           3

#define OF_CLOSECONNECTION  (0x00000001)

struct SClientLink
{
    SClientLink()
    {
        m_Socket = INVALID_SOCKET;
        m_Context = NULL;
        m_dwOutSequence = 0;
        memset(&m_addr, 0, sizeof(m_addr));

        m_pSendBuffer = NULL;
        m_dwCurBufferSize = 0;
        m_bCloseConnection = FALSE;
        m_bSending = FALSE;
    }

    ~SClientLink()
    {
        if (m_Socket != INVALID_SOCKET)
        {
            shutdown(m_Socket, SD_BOTH);
            closesocket(m_Socket);
        }

        m_Context = NULL;
        m_dwOutSequence = 0;

        if (m_pSendBuffer != NULL)
        {
            delete m_pSendBuffer;
            m_pSendBuffer = NULL;
            m_dwCurBufferSize = 0;
        }

        while (m_SendQueue.size())
        {
            OVERLAPPEDPLUS *pOverlapPlus = m_SendQueue.front();
            m_SendQueue.pop_front();

            delete pOverlapPlus;
        }
    }

    BOOL Push(LPVOID pvEntry, WORD wSize, BOOL bUsePushDirect)
    {
        if (pvEntry == NULL || wSize == 0)
            return FALSE;

        if (m_pSendBuffer == NULL)
        {
            m_pSendBuffer = new char[0xffff];
            if (m_pSendBuffer == NULL)
            {
                Trace("64KB内存分配失败！！！");
                return FALSE;
            }
        }

        if (m_dwCurBufferSize + wSize > 0xf000)
            return FALSE;

        if (!bUsePushDirect)
        {
            DNPHDR *pTemp = (DNPHDR *)&m_pSendBuffer[m_dwCurBufferSize];
            pTemp->seqnum = (WORD)m_dwOutSequence++;
            pTemp->paclen = (UINT16)wSize;
            m_dwCurBufferSize += sizeof(DNPHDR);
        }

        memcpy(&m_pSendBuffer[m_dwCurBufferSize], pvEntry, wSize);
        m_dwCurBufferSize += wSize;

        return TRUE;
    }

private :
    void PushIntoQueue()
    {
        if (m_dwCurBufferSize == 0) 
            return;

        if (m_pSendBuffer == NULL)
            return;

        if (m_SendQueue.size() != 0)
        {
            OVERLAPPEDPLUS *pOverlapPlus = NULL;
            std::list<OVERLAPPEDPLUS *>::iterator it = m_SendQueue.end();
            while (it != m_SendQueue.begin())
            {
                it--;

                if ((*it)->dwTotalBytes == 0)
                    pOverlapPlus = *it;
                else if ((*it)->dwTotalBytes + m_dwCurBufferSize > 0xf000)
                    break;
                else
                {
                    pOverlapPlus = *it;
                    break;
                }
            }
            
            if (pOverlapPlus != NULL)
            {
                if (pOverlapPlus->dwTotalBytes + m_dwCurBufferSize > 0xf000)
                    _asm int 3;

                memcpy(&pOverlapPlus->pOperationBuffer[pOverlapPlus->dwTotalBytes], 
                    m_pSendBuffer, m_dwCurBufferSize);

                pOverlapPlus->dwTotalBytes += m_dwCurBufferSize;

                m_dwCurBufferSize = 0;

                return;
            }
        }

        OVERLAPPEDPLUS *newolp = new OVERLAPPEDPLUS;
        if (newolp == NULL)
            return;

        memset(newolp, 0, sizeof(OVERLAPPEDPLUS));

        newolp->s = m_Socket;
        newolp->dnidClient = m_Dnid;
        newolp->OpCode = OP_WRITE;
        newolp->dwTotalBytes = m_dwCurBufferSize;
        newolp->pOperationBuffer.Alloc(0xffff);

        newolp->dwFlags = m_bCloseConnection ? OF_CLOSECONNECTION : 0;

        memcpy(newolp->pOperationBuffer, m_pSendBuffer, m_dwCurBufferSize);

        m_dwCurBufferSize = 0;

        m_SendQueue.push_back(newolp);
    }

public:
    BOOL Send()
    {
        PushIntoQueue();

        if (m_SendQueue.size() == 0)
            return TRUE;

        if (m_SendQueue.size() > 10)
        {
            // 这也就是说：需要发送的包已经累积了10个了！
            // 总数局量达到了600K或者要求发送的次数超过了10次！
            // 应该断掉这个连接！
            m_bCloseConnection = TRUE;

            return FALSE;
        }

        if (m_bSending)
            return TRUE;

        m_bSending = TRUE;

        OVERLAPPEDPLUS *newolp = m_SendQueue.front();
        m_SendQueue.pop_front();

        memset(&newolp->ol, 0, sizeof(OVERLAPPED));

        newolp->s = m_Socket;
        newolp->OpCode = OP_WRITE;
        newolp->dnidClient = m_Dnid;
        newolp->wbuf.len = newolp->dwTotalBytes;
        newolp->wbuf.buf = newolp->pOperationBuffer;

        if (WSASend(newolp->s, &newolp->wbuf, 1, &s_dwTransferredBytes, 0, &newolp->ol, NULL) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                m_bCloseConnection = TRUE;

                delete newolp;

                return FALSE;
            }
        }

        return TRUE;
    }

    DNID m_Dnid;
    SOCKET m_Socket;
    LPVOID m_Context;
    DWORD m_dwOutSequence;
    sockaddr_in m_addr;

    // 用于检测发送数据超标的数据
    // ---- WSASend使用异步发送，为了保证发送的数据在整个发送过程中有效
    // 它将发送的数据锁定到了非分页内存中，如果同时多次使用WSASend ----
    // 那么会有很多非分页内存被占用！这将会导致标准系统无法使用非分页内存。。。
    // 同样的，WSARecv也会出现这个问题！
    // 不过每一个连接的WSARecv是通过WSARecv自己激活的，所以应该不会出现太大的问题
    // 但是也需要检测接收包的大小是否过大才行！
    // 这里修正WSASend的方法也可以使用方法吧，也就是自己激活自己！
    std::list<OVERLAPPEDPLUS *> m_SendQueue;    // 累积的发送次数！一般来说10次1秒
    BOOL m_bSending;                            // 当前有数据正在发送中！

    BOOL m_bCloseConnection;
    DWORD m_dwCurBufferSize;
    char *m_pSendBuffer;
};

typedef tLockedPtr<SClientLink> LPClientLink;

class cLinkManager :
    public tObjectService<DNID, SClientLink>
{
    virtual DWORD TraversalCallback(const DNID &dnid, 
                                    tObjectService<DNID, SClientLink>::LPOBJECT &pLink, 
                                    WPARAM wParam, 
                                    LPARAM lParam) 
    {
        if (pLink == NULL)
            return 0;

        if (pLink->Send() == FALSE)
            return 0;// TCR_REMOVE;

        return 0;
    }
};

static DNID dnid_gen()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    DNID dnid = *(DNID*)&li;
    return dnid;
}

class cDirectNetServer : 
    public iDirectNetServer 
{
public:
    DNRESULT Initialize(iDirectNetCryption* pDirectNetCryption,
                        iDirectNetServerEventHandler* pDirectNetServerEventHandler, 
                        DWORD dwMaxNumPlayers);

    DNRESULT Host(DNPORT dnPort);
    DNRESULT SendTo(DNID dnidPlayer, PVOID pvBuffer, DWORD dwLength);
    DNRESULT DeletePlayer(DNID dnidPlayer);
    DNRESULT Close();

    DNRESULT GetPlayerConnectionInfo(DNID dnidPlayer, CONNECTIONINFO* pConnectionInfo);

private:
    // 完成端口工作线程
    void IocpWorkerThreadProcedure();
    static unsigned int __stdcall s_iocp_worker_thread_procedure(void*);

    // 完成端口连接线程
    void IocpAcceptThreadProcedure();
    static unsigned int __stdcall s_iocp_accept_thread_procedure(void*);

    void IocpSendThreadProcedure();
    static unsigned int __stdcall s_iocp_send_thread_procedure(void*);

    void ErrorHandle(LPCSTR Info, BOOL le = FALSE, int eid = -1);

    void DeleteLink(DNID dnidClient);

public:
    cDirectNetServer();
    ~cDirectNetServer();

private:
    WORD m_wMaxClient;

    WORD m_wPort;

    int m_MaxAcceptSocket;

    SOCKET m_sServer;

    BOOL m_bAcceptThreadQuit;
    BOOL m_bSendThreadQuit;

    BOOL m_bUseAutoPackage;
    BOOL m_bUsePushDirect;

    DWORD m_dwWorkThreadNumber;

    iDirectNetServerEventHandler *m_pDirectNetServerEventHandler;

    iDirectNetCryption *m_pDirectNetCryption;

    LPFN_ACCEPTEX lpAcceptEx;

    LPFN_GETACCEPTEXSOCKADDRS lpGetAcceptExSockaddrs;

    HANDLE m_hIoCompletionPort;

    HANDLE m_hThreadExitEvent;

    HANDLE m_hActiveAcceptEvent;

    cLinkManager m_LinkManager;
};

unsigned int cDirectNetServer::s_iocp_worker_thread_procedure(void* pv_this)
{
    reinterpret_cast<cDirectNetServer*>(pv_this)->IocpWorkerThreadProcedure();

    return 0U;
}

unsigned int cDirectNetServer::s_iocp_accept_thread_procedure(void* pv_this)
{
    reinterpret_cast<cDirectNetServer*>(pv_this)->IocpAcceptThreadProcedure();

    return 0U;
}

unsigned int cDirectNetServer::s_iocp_send_thread_procedure(void* pv_this)
{
    reinterpret_cast<cDirectNetServer*>(pv_this)->IocpSendThreadProcedure();

    return 0U;
}

iDirectNetServer* iDirectNetServer::CreateInstance()
{
#ifdef _DEBUG
    // Get the current bits
    int tmp = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

    // Clear the upper 16 bits and OR in the desired freqency
    tmp = (tmp & 0x0000FFFF) | _CRTDBG_LEAK_CHECK_DF;

    // Set the new bits
    _CrtSetDbgFlag(tmp);
#endif

    return new cDirectNetServer;
}

void iDirectNetServer::Release()
{
    delete this;
}

cDirectNetServer::cDirectNetServer()
{
    m_bAcceptThreadQuit = FALSE;
    m_bSendThreadQuit = FALSE;
    m_bUseAutoPackage = FALSE;
    m_bUsePushDirect = FALSE;

    m_wMaxClient = 0;
    m_wPort = 0;
    m_MaxAcceptSocket = 10;
    m_sServer = INVALID_SOCKET;
    m_dwWorkThreadNumber = 0;
    m_pDirectNetServerEventHandler = NULL;
    m_hIoCompletionPort = NULL;
    m_hThreadExitEvent = INVALID_HANDLE_VALUE;
    m_hActiveAcceptEvent = INVALID_HANDLE_VALUE;

    lpAcceptEx = NULL;
    lpGetAcceptExSockaddrs = NULL;

    WSADATA wsad;
    if (WSAStartup( MAKEWORD(2, 0), &wsad ) != 0)
    {
        ErrorHandle("当前系统并不支持WinSock2.0，相应的网络操作都无法完成！", TRUE);
    }

    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    //m_dwWorkThreadNumber = SystemInfo.dwNumberOfProcessors + 1; // the old mode is x2
    m_dwWorkThreadNumber = SystemInfo.dwNumberOfProcessors * 2 + 1;

    m_hThreadExitEvent = CreateEvent(NULL, 0, 0, NULL);
    m_hActiveAcceptEvent = CreateEvent(NULL, 0, 0, NULL);
}

cDirectNetServer::~cDirectNetServer()
{
    CloseHandle(m_hThreadExitEvent);
    CloseHandle(m_hActiveAcceptEvent);

    WSACleanup();
}

void cDirectNetServer::ErrorHandle(LPCSTR info, BOOL le, int eid)
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

DNRESULT cDirectNetServer::Initialize(iDirectNetCryption* pDirectCryption, 
                                      iDirectNetServerEventHandler* pDirectEventHandler, 
                                      DWORD dwMaxNumPlayers)
{
    if (m_pDirectNetServerEventHandler != NULL)
    {
        ErrorHandle("本对象已经初始化过了！");
        return DNR_E_FAIL;
    }

    m_hIoCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (m_hIoCompletionPort == NULL) 
    {
        ErrorHandle("创建完成端口失败！", TRUE);
        return DNR_E_FAIL;
    }

    if (pDirectEventHandler == NULL ||
        dwMaxNumPlayers == 0)
    {
        ErrorHandle("错误的事件响应句柄或者错误的最大连接数！");
        return DNR_E_INVALID_PARAMETER;
    }

    m_pDirectNetCryption = pDirectCryption;
    m_pDirectNetServerEventHandler = pDirectEventHandler;
    m_wMaxClient = (dwMaxNumPlayers > 30000) ? 30000 : (WORD)dwMaxNumPlayers;

    m_MaxAcceptSocket = 10;

    return DNR_S_OK;
}

DNRESULT cDirectNetServer::Close()
{
    if (m_hIoCompletionPort == NULL)
        return DNR_S_OK;

    if (m_sServer != INVALID_SOCKET)
    {
        m_MaxAcceptSocket = -100;
        SOCKET s = m_sServer;
        m_sServer = INVALID_SOCKET;
        closesocket(s);

        ResetEvent(m_hThreadExitEvent);
        m_bAcceptThreadQuit = TRUE;
        SetEvent(m_hActiveAcceptEvent);
        WaitForSingleObject(m_hThreadExitEvent, INFINITE);

        // send thread exit
        ResetEvent(m_hThreadExitEvent);
        m_bSendThreadQuit = TRUE;
        m_LinkManager.Clear();
        WaitForSingleObject(m_hThreadExitEvent, INFINITE);

        for (DWORD i = 0; i < m_dwWorkThreadNumber; i++)
        {
            ResetEvent(m_hThreadExitEvent);

            // 发出退出标记
            PostQueuedCompletionStatus(m_hIoCompletionPort, NULL, 0, 0);

            WaitForSingleObject(m_hThreadExitEvent, INFINITE);
        }

        ULONG_PTR key = 0;
        DWORD dwNumBytes = 0;
        OVERLAPPED* pOverlapped = NULL;
        OVERLAPPEDPLUS* pOverlapPlus = NULL;

        while (GetQueuedCompletionStatus(m_hIoCompletionPort, &dwNumBytes, &key, &pOverlapped, 0))
        {
            if (pOverlapPlus == NULL)
                continue;

            pOverlapPlus = CONTAINING_RECORD(pOverlapped, OVERLAPPEDPLUS, ol);

            delete pOverlapPlus;
        }
    }

    if (m_hIoCompletionPort != NULL ||
        m_pDirectNetServerEventHandler != NULL ||
        m_wMaxClient != 0)
    {
        m_wMaxClient = 0;
        m_pDirectNetServerEventHandler = NULL;
        m_pDirectNetCryption = NULL;

        CloseHandle(m_hIoCompletionPort);
        m_hIoCompletionPort = NULL;
    }

    return DNR_S_OK;
}

DNRESULT cDirectNetServer::Host(DNPORT dnPort)
{
    if (m_sServer != INVALID_SOCKET)
        return DNR_E_FAIL;

    if (m_hIoCompletionPort == NULL ||
        m_pDirectNetServerEventHandler == NULL ||
        m_wMaxClient == 0)
        return DNR_E_NOT_INITIALIZED;

	DWORD dwResult = 0;

    GUID GUIDAcceptEx = WSAID_ACCEPTEX;
    GUID GUIDGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

    m_wPort = dnPort;

    // create the server socket, bind it to a fixed port, and put it to listen state
    m_sServer = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == m_sServer)
        return DNR_E_FAIL;

    sockaddr_in sain;
    ZeroMemory(&sain, sizeof(sain));
    sain.sin_family = AF_INET;
    sain.sin_addr.s_addr = INADDR_ANY;
    sain.sin_port = htons(dnPort);
    if (SOCKET_ERROR == bind(m_sServer, (sockaddr*)&sain, sizeof(sain)))
        goto __host_fail;

    if (SOCKET_ERROR == listen(m_sServer, SOMAXCONN))
        goto __host_fail;

    if (CreateIoCompletionPort((HANDLE)m_sServer, m_hIoCompletionPort, 0, 0) == NULL)
        goto __host_fail;

	if (WSAIoctl(m_sServer, SIO_GET_EXTENSION_FUNCTION_POINTER, 
        &GUIDAcceptEx, sizeof(GUIDAcceptEx), &lpAcceptEx, 
        sizeof(lpAcceptEx), &dwResult, NULL, NULL) == SOCKET_ERROR)
	{
		ErrorHandle("获取AcceptEx函数地址失败！", TRUE);
        goto __host_fail;
	}

	if (WSAIoctl(m_sServer, SIO_GET_EXTENSION_FUNCTION_POINTER, 
        &GUIDGetAcceptExSockaddrs, sizeof(GUIDGetAcceptExSockaddrs), &lpGetAcceptExSockaddrs, 
        sizeof(lpGetAcceptExSockaddrs), &dwResult, NULL, NULL) == SOCKET_ERROR)
	{
		ErrorHandle("获取GetAcceptExSockaddrs函数地址失败！", TRUE);
        goto __host_fail;
	}

    for (DWORD i = 0; i < m_dwWorkThreadNumber; i++)
    {
        _beginthreadex(NULL, 0, s_iocp_worker_thread_procedure, this, 0, NULL);
    }

    _beginthreadex(NULL, 0, s_iocp_accept_thread_procedure, this, 0, NULL);
    _beginthreadex(NULL, 0, s_iocp_send_thread_procedure, this, 0, NULL);

    return DNR_S_OK;

__host_fail:
    closesocket(m_sServer);
    m_sServer = INVALID_SOCKET;
    return DNR_E_FAIL;
}

void cDirectNetServer::IocpAcceptThreadProcedure()
{
    if (m_sServer == INVALID_SOCKET)
        return;

    while (true)
    {
        if (m_bAcceptThreadQuit)
        {
            // accept thread exit
            break;
        }

        if (m_MaxAcceptSocket < 0)
        {
            m_MaxAcceptSocket++;
            WaitForSingleObject(m_hActiveAcceptEvent, INFINITE);

            continue;
        }

        SOCKET newsock = socket(AF_INET, SOCK_STREAM, 0);
        if (newsock == INVALID_SOCKET)
        {
            ErrorHandle("无法创建新的套接字！", TRUE);

            break;
        }

        OVERLAPPEDPLUS *newolp = new OVERLAPPEDPLUS;
        if (newolp == NULL)
        {
            ErrorHandle("无法分配新的内存！");

            break;
        }

        memset(newolp, 0, sizeof(OVERLAPPEDPLUS));

        newolp->s = newsock;
        newolp->OpCode = OP_ACCEPT;
        newolp->wbuf.len = newolp->dwTotalBytes = 0;
        newolp->wbuf.buf = newolp->pOperationBuffer.Alloc(128);

        if (newolp->pOperationBuffer == NULL)
        {
            delete newolp;

            ErrorHandle("无法分配新的内存！");

            break;
        }

        if (!lpAcceptEx(m_sServer, newsock, newolp->pOperationBuffer, 0,
            sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &s_dwTransferredBytes, &newolp->ol))
        {
            int eid = WSAGetLastError();
            if (eid != WSA_IO_PENDING)
            {
                delete newolp;

                ErrorHandle("执行AcceptEx失败！", FALSE, eid);

                break;
            }
        }

        if (m_MaxAcceptSocket > 0)
        {
            m_MaxAcceptSocket--;

            if (m_MaxAcceptSocket > 0)
            {
                continue;
            }
        }

        WaitForSingleObject(m_hActiveAcceptEvent, INFINITE);
    }

    // the last accept thread is exiting, signal the event
    SetEvent(m_hThreadExitEvent);
}

void cDirectNetServer::IocpSendThreadProcedure()
{
    while (!m_bSendThreadQuit)
    {
        if (m_bUseAutoPackage)
        {
            // 这个地方存在一个非常大的隐患，很可能造成非分页内存的大量分配！！！
            // 因为这个调用会直接触发Send()，而Send会直接调用WSASend发送缓存的数据！
            // 只要客户端不接收，WSASend发送的数据就会全部累积到非分页内存中！
            // 所以，我们需要修改Send()函数
            m_LinkManager.TraversalObjects(0, 0);
        }

        Sleep(SEND_MARGIN);
    }

    // the last send thread is exiting, signal the event
    SetEvent(m_hThreadExitEvent);
}

void cDirectNetServer::IocpWorkerThreadProcedure()
{
    ULONG_PTR key = 0;
    DWORD dwNumBytes = 0;
    OVERLAPPED* pOverlapped = NULL;
    OVERLAPPEDPLUS* pOverlapPlus = NULL;

    while (true) 
    {
        BOOL ret = GetQueuedCompletionStatus(m_hIoCompletionPort, &dwNumBytes, &key, &pOverlapped, INFINITE);
        if (ret == 0 && pOverlapped == NULL)
        {
            ErrorHandle("GetQueuedCompletionStatus 操作失败", TRUE);
            break; // or continue ???
        }

        if (pOverlapped == NULL)
        {
            break; // normal exit
        }

        pOverlapPlus = CONTAINING_RECORD(pOverlapped, OVERLAPPEDPLUS, ol);

        if (m_sServer == INVALID_SOCKET)
        {
            delete pOverlapPlus;

            continue;
        }

        switch (pOverlapPlus->OpCode)
        {
        case OP_ACCEPT:
            {
                SetEvent(m_hActiveAcceptEvent);

                if (setsockopt(pOverlapPlus->s, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, 
                    (char *)&m_sServer, sizeof(m_sServer)) == SOCKET_ERROR)
                {
                    ErrorHandle("OP_ACCEPT::lpGetAcceptExSockaddrs 操作失败", TRUE);

                    closesocket(pOverlapPlus->s);

                    delete pOverlapPlus;

                    break;
                }

                sockaddr_in addrin;

                sockaddr *p1 = NULL, *p2 = NULL;
                int i1 = 0, i2 = 0;

                lpGetAcceptExSockaddrs(pOverlapPlus->pOperationBuffer, 0, sizeof(sockaddr_in) + 16, 
                    sizeof(sockaddr_in) + 16, &p1, &i1, &p2, &i2);

                if (p1 == NULL || p2 == NULL)
                {
                    ErrorHandle("OP_ACCEPT::lpGetAcceptExSockaddrs 操作失败", TRUE);

                    closesocket(pOverlapPlus->s);

                    delete pOverlapPlus;

                    break;
                }

                addrin = *(sockaddr_in*)p2;

                if (CreateIoCompletionPort((HANDLE)pOverlapPlus->s, m_hIoCompletionPort, (ULONG_PTR)0, 0) == 0)
                {
                    ErrorHandle("OP_ACCEPT::CreateIoCompletionPort 操作失败", TRUE);

                    closesocket(pOverlapPlus->s);

                    delete pOverlapPlus;

                    break;
                }

                memset(&pOverlapPlus->ol, 0, sizeof(OVERLAPPED));

                pOverlapPlus->OpCode = OP_READ_HEAD;
                pOverlapPlus->wbuf.len = pOverlapPlus->dwTotalBytes = sizeof(CRYPTION);
                pOverlapPlus->wbuf.buf = pOverlapPlus->pOperationBuffer.Alloc(pOverlapPlus->dwTotalBytes);
                pOverlapPlus->dnidClient = dnid_gen();

                if (pOverlapPlus->pOperationBuffer == NULL)
                {
                    ErrorHandle("OP_ACCEPT::new pOverlapPlus->pOperationBuffer 空指针");

                    closesocket(pOverlapPlus->s);

                    delete pOverlapPlus;

                    break;
                }

                if (WSARecv(pOverlapPlus->s, &pOverlapPlus->wbuf, 1, &s_dwTransferredBytes, 
                    &s_dwFlags, &pOverlapPlus->ol, NULL) == SOCKET_ERROR)
                {
                    int eid = WSAGetLastError();
                    if (eid != WSA_IO_PENDING)
                    {
                        // ErrorHandle("OP_ACCEPT::WSARecv 操作失败", FALSE, eid);

                        closesocket(pOverlapPlus->s);

                        delete pOverlapPlus;

                        break;
                    }
                }

                if (m_pDirectNetServerEventHandler)
                {
                    m_pDirectNetServerEventHandler->OnCreatePlayer(pOverlapPlus->dnidClient, &pOverlapPlus->pvContext);
                }

                SClientLink cl;
                cl.m_Dnid = pOverlapPlus->dnidClient;
                cl.m_Socket = pOverlapPlus->s;
                cl.m_Context = pOverlapPlus->pvContext;
                cl.m_dwOutSequence = 0;
                cl.m_addr = addrin;

                m_LinkManager.AddObject(pOverlapPlus->dnidClient, cl);

                cl.m_Socket = INVALID_SOCKET; //keep socket can't close in distruct
            }
            break;

        case OP_READ_HEAD:
            {
                if (dwNumBytes == 0)
                {
                    DeleteLink(pOverlapPlus->dnidClient);

                    delete pOverlapPlus;

                    break;
                }

                pOverlapPlus->dwTransferredBytes += dwNumBytes;

                if (pOverlapPlus->dwTransferredBytes < pOverlapPlus->dwTotalBytes)
                {
                    memset(&pOverlapPlus->ol, 0, sizeof(OVERLAPPED));

                    pOverlapPlus->wbuf.buf = &pOverlapPlus->pOperationBuffer[pOverlapPlus->dwTransferredBytes];
                    pOverlapPlus->wbuf.len = pOverlapPlus->dwTotalBytes - pOverlapPlus->dwTransferredBytes;
                }
                else
                {
                    memset(&pOverlapPlus->ol, 0, sizeof(OVERLAPPED));

                    CRYPTION cryption = *(CRYPTION *)(char *)pOverlapPlus->pOperationBuffer;

                    if (m_pDirectNetCryption)
                    {
                        m_pDirectNetCryption->DES_decrypt(&cryption, sizeof(cryption));

                        UINT32 crc32 = m_pDirectNetCryption->CRC32_compute( 
                            (char*)&cryption + sizeof(cryption.crc32), sizeof(cryption) - sizeof(cryption.crc32) );

                        if (crc32 != cryption.crc32) 
                        {
                            ErrorHandle("OP_READ_HEAD::crc32 解密后的CRC32错误");

                            DeleteLink(pOverlapPlus->dnidClient);

                            delete pOverlapPlus;

                            break;
                        }
                    }

                    if (cryption.dnphdr.seqnum != (WORD)pOverlapPlus->dwSequence)
                    {
                        DeleteLink(pOverlapPlus->dnidClient);

                        ErrorHandle("detected package lost !!!");
                    }

                    pOverlapPlus->OpCode = OP_READ_BODY;
                    pOverlapPlus->dwTransferredBytes = 0;
                    pOverlapPlus->wbuf.len = pOverlapPlus->dwTotalBytes = cryption.dnphdr.paclen;
                    // cryption.dnphdr.paclen 是WORD类型，所以他的阻塞对非分页内存不会造成太影响！
                    // 最大程度为 连接上限*64k = (64,000)(1000个连接64MB的消耗)
                    pOverlapPlus->wbuf.buf = pOverlapPlus->pOperationBuffer.Alloc(pOverlapPlus->dwTotalBytes);

                    if (pOverlapPlus->pOperationBuffer == NULL)
                    {
                        ErrorHandle("OP_READ_HEAD::new pOverlapPlus->pOperationBuffer 空指针");

                        DeleteLink(pOverlapPlus->dnidClient);

                        delete pOverlapPlus;

                        break;
                    }
                }

                if (WSARecv(pOverlapPlus->s, &pOverlapPlus->wbuf, 1, &s_dwTransferredBytes, 
                    &s_dwFlags, &pOverlapPlus->ol, NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != WSA_IO_PENDING)
                    {
                        DeleteLink(pOverlapPlus->dnidClient);

                        delete pOverlapPlus;

                        break;
                    }
                }
            }
            break;

        case OP_READ_BODY:
            {
                if (dwNumBytes == 0)
                {
                    DeleteLink(pOverlapPlus->dnidClient);

                    delete pOverlapPlus;

                    break;
                }

                pOverlapPlus->dwTransferredBytes += dwNumBytes;

                if (pOverlapPlus->dwTransferredBytes < pOverlapPlus->dwTotalBytes)
                {
                    memset(&pOverlapPlus->ol, 0, sizeof(OVERLAPPED));

                    pOverlapPlus->wbuf.buf = &pOverlapPlus->pOperationBuffer[pOverlapPlus->dwTransferredBytes];
                    pOverlapPlus->wbuf.len = pOverlapPlus->dwTotalBytes - pOverlapPlus->dwTransferredBytes;
                }
                else
                {
                    if (m_pDirectNetServerEventHandler)
                    {
                        m_pDirectNetServerEventHandler->OnReceivePacket(pOverlapPlus->dnidClient, 
                            pOverlapPlus->pvContext, pOverlapPlus->pOperationBuffer, 
                            pOverlapPlus->dwTotalBytes, pOverlapPlus->dwSequence++);
                    }

                    memset(&pOverlapPlus->ol, 0, sizeof(OVERLAPPED));

                    pOverlapPlus->OpCode = OP_READ_HEAD;
                    pOverlapPlus->dwTransferredBytes = 0;
                    pOverlapPlus->wbuf.len = pOverlapPlus->dwTotalBytes = sizeof(CRYPTION);
                    pOverlapPlus->wbuf.buf = pOverlapPlus->pOperationBuffer.Alloc(pOverlapPlus->dwTotalBytes);

                    if (pOverlapPlus->pOperationBuffer == NULL)
                    {
                        ErrorHandle("OP_READ_BODY::new pOverlapPlus->pOperationBuffer 空指针");

                        DeleteLink(pOverlapPlus->dnidClient);

                        delete pOverlapPlus;

                        break;
                    }
                }

                if (WSARecv(pOverlapPlus->s, &pOverlapPlus->wbuf, 1, &s_dwTransferredBytes, 
                    &s_dwFlags, &pOverlapPlus->ol, NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != WSA_IO_PENDING)
                    {
                        DeleteLink(pOverlapPlus->dnidClient);

                        delete pOverlapPlus;

                        break;
                    }
                }
            }
            break;

        case OP_WRITE:
            {
                // 如果断开，就不管发送状态了，直接断开
                if (pOverlapPlus->dwFlags & OF_CLOSECONNECTION)
                {
                    DeleteLink(pOverlapPlus->dnidClient);
                    delete pOverlapPlus;
                    break;
                }

                pOverlapPlus->dwTransferredBytes += dwNumBytes;

                // 如果还有数据没有发完，则继续发剩下的
                if (pOverlapPlus->dwTransferredBytes < pOverlapPlus->dwTotalBytes)
                {
                    memset(&pOverlapPlus->ol, 0, sizeof(OVERLAPPED));

                    pOverlapPlus->wbuf.buf = &pOverlapPlus->pOperationBuffer[pOverlapPlus->dwTransferredBytes];
                    pOverlapPlus->wbuf.len = pOverlapPlus->dwTotalBytes - pOverlapPlus->dwTransferredBytes;

                    if (WSASend(pOverlapPlus->s, &pOverlapPlus->wbuf, 1, &s_dwTransferredBytes, 
                        0, &pOverlapPlus->ol, NULL) == SOCKET_ERROR)
                    {
                        if (WSAGetLastError() != WSA_IO_PENDING)
                        {
                            delete pOverlapPlus;

                            break;
                        }
                    }
                }
                else
                {
                    // 成功发送完毕！
                    BOOL bClose = false;
                    DNID dnid = pOverlapPlus->dnidClient;
                    if (LPClientLink pcl = m_LinkManager.GetLocateObject(pOverlapPlus->dnidClient))
                    {
                        pOverlapPlus->dwTotalBytes = 0;
                        pOverlapPlus->pOperationBuffer;

                        pcl->m_bSending = FALSE;

                        if (m_bUseAutoPackage)
                            pcl->m_SendQueue.push_back(pOverlapPlus);
                        else
                        {
                            delete pOverlapPlus;
                            pcl->Send();
                        }

                        bClose = pcl->m_bCloseConnection;
                    }
                    else
                    {
                        delete pOverlapPlus;
                    }

                    if (bClose)
                    {
                        DeleteLink(dnid);
                    }

                    break;
                }
            }
            break;
        }
    }

    // the last iocp worker thread is exiting, signal the event
    SetEvent(m_hThreadExitEvent);
}

DNRESULT cDirectNetServer::SendTo(DNID dnidClient, PVOID pvBuffer, DWORD dwLength)
{
    if ( pvBuffer == NULL || dwLength == 0 || dwLength >= 0xf000 )
    {
        return DNR_E_INVALID_PARAMETER;
    }

    if (LPClientLink pcl = m_LinkManager.GetLocateObject(dnidClient))
    {
        if (pcl->m_bCloseConnection)
            return DNR_E_FAIL;

        if (!pcl->Push(pvBuffer, (WORD)dwLength, m_bUsePushDirect))
        {
            if (!pcl->Send())
                goto _SendFail;

            if (!pcl->Push(pvBuffer, (WORD)dwLength, m_bUsePushDirect))
                return DNR_E_FAIL;
        }

        if (!m_bUseAutoPackage)
        {
            if (!pcl->Send())
                goto _SendFail;
        }

        return DNR_S_OK;
    }

    return DNR_E_INVALID_PLAYER;

_SendFail:
    DeleteLink(dnidClient);

    return DNR_E_INVALID_PLAYER;
}

void cDirectNetServer::DeleteLink(DNID dnidClient)
{
    LPVOID pContext = NULL;
    BOOL findit = FALSE;

    if (LPClientLink pcl = m_LinkManager.GetLocateObject(dnidClient))
    {
        pContext = pcl->m_Context;
        findit = TRUE;
    }
    else
    {
        return;
    }

    if (findit)
    {
        m_LinkManager.DelObject(dnidClient);
    }

    if (m_pDirectNetServerEventHandler)
    {
        m_pDirectNetServerEventHandler->OnDeletePlayer(dnidClient, pContext);
    }
}

DNRESULT cDirectNetServer::DeletePlayer(DNID dnidClient)
{
    if (LPClientLink pcl = m_LinkManager.GetLocateObject(dnidClient))
    {
        pcl->m_bCloseConnection = TRUE;

        pcl->Push("Byebye!", 8, FALSE);

        if (!m_bUseAutoPackage)
        {
            pcl->Send();
        }

        return TRUE;
    }

    return FALSE;
}

DNRESULT cDirectNetServer::GetPlayerConnectionInfo(DNID dnidClient, CONNECTIONINFO* pConnectionInfo)
{
    if (pConnectionInfo == NULL)
        return DNR_E_INVALID_PARAMETER;

    if (dnidClient == 0xff00ff00ff00ff00)
    {
        LPDWORD pParameter = (LPDWORD)pConnectionInfo;
        switch (pParameter[0])
        {
        case 1:
            m_bUseAutoPackage = (pParameter[1] == 1);
            break;

        case 2:
            m_bUsePushDirect = (pParameter[1] == 1);
            break;

        default:
            return DNR_E_FAIL;
        }

        return DNR_S_OK;
    }

    if (LPClientLink pcl = m_LinkManager.GetLocateObject(dnidClient))
    {
        pConnectionInfo->addr = pcl->m_addr.sin_addr.s_addr;
	    pConnectionInfo->port = pcl->m_addr.sin_port;

        return DNR_S_OK;
    }

    return DNR_E_INVALID_PLAYER;
}
