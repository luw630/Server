#include "stdafx.h"

#ifdef _DEBUG
#include <crtdbg.h>
#undef _AFX_NO_DEBUG_CRT
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif _DEBUG

// the DirectNetProtocol header that is passed from server to client
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

enum RECEIVING_STATE { RECEIVING_HEAD, RECEIVING_BODY };

static DNID dnid_gen()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    DNID dnid = *(DNID*)&li;
    return dnid;
}

class cReferenceCount {
public:
    cReferenceCount() : m_lReferenceCount(1) {}

    LONG AddRef()
    {
        return InterlockedIncrement(&m_lReferenceCount);
    }

    LONG Release()
    {
        if ( 0 == InterlockedDecrement(&m_lReferenceCount) ) {
            delete this;
            return 0;
        }

        return m_lReferenceCount;
    }

protected:
    virtual ~cReferenceCount() {}

private:
    LONG m_lReferenceCount;
};

class cPlayerInfo : public cReferenceCount {
public:
    DNID dnid;
    SOCKET s;
    sockaddr_in sain;
    PVOID context;

    UINT16 u16OutgoingSequence;
    UINT16 u16IncomingSequence;

    bool is_alive;

protected:
    virtual ~cPlayerInfo() {}
};

struct CMMN_PARAM_STRUCT {
    class cDirectNetServer* pDirectNetServer;
};

struct SEND_PARAM_STRUCT : public CMMN_PARAM_STRUCT {
    cPlayerInfo* pPlayerInfo;
    PCHAR pcBuffer;
    DWORD dwLength;
};

struct RECV_PARAM_STRUCT : public CMMN_PARAM_STRUCT {
    cPlayerInfo* pPlayerInfo;
};

#define DELETE_PLAYER_PARAM_STRUCT RECV_PARAM_STRUCT

struct ON_DELETE_PLAYER_PARAM_STRUCT : public CMMN_PARAM_STRUCT {
    DNID dnid;
    PVOID context;
};

class cDirectNetServer : public iDirectNetServer {
    friend class iDirectNetServer;

public:
    DNRESULT Initialize(iDirectNetCryption* pDirectNetCryption,
        iDirectNetServerEventHandler* pDirectNetServerEventHandler, DWORD dwMaxNumPlayers);
    DNRESULT Host(DNPORT dnPort);
    DNRESULT SendTo(DNID dnidPlayer, PVOID pvBuffer, DWORD dwLength);
    DNRESULT DeletePlayer(DNID dnidPlayer);
    DNRESULT GetPlayerAddrInfo(DNID dnidPlayer, sockaddr_in *addr, size_t size);
    DNRESULT Close();

    void RecvCompletionHandler(DWORD dwNumBytesTransferred, class cRecvOperation* pRecvOperation);
    void SendCompletionHandler(DWORD dwNumBytesTransferred, class cSendOperation* pSendOperation);

private:
    cDirectNetServer();
    ~cDirectNetServer();

    bool m_bIsInitialized;

    iDirectNetCryption* m_pDirectNetCryption;

    iDirectNetServerEventHandler* m_pDirectNetServerEventHandler;

    DWORD m_dwMaxNumPlayers;

    SOCKET m_sServer;

    HANDLE m_hIoCompletionPort;
    DWORD m_dwNumWorkerThreads;

    HANDLE m_hCloseEvent;

    std::map<DNID, cPlayerInfo*> m_PlayerInfoMap;
    CRITICAL_SECTION m_csLock;

    HANDLE m_hAcceptThread;
    void AcceptThreadProcedure();
    static unsigned int __stdcall s_accept_thread_procedure(void*);

    HANDLE m_hApcHandlerThread;
    void ApcHandlerThread();
    static unsigned int __stdcall s_apc_handler_thread_procedure(void*);

    void IocpWorkerThreadProcedure();
    static unsigned int __stdcall s_iocp_worker_thread_procedure(void*);

    void IoCompletionHandler(DWORD dwNumBytesTransferred, class cPerIoOperation* pPerIoOperation);

    // this static apc is used to exit the apc thread
    static VOID CALLBACK StaticExitApc(ULONG_PTR);

    static VOID CALLBACK StaticSendApc(ULONG_PTR);
    static VOID CALLBACK StaticRecvApc(ULONG_PTR);

    void SendApc(cPlayerInfo* pPlayerInfo, PCHAR pvBuffer, DWORD dwLength);
    void RecvApc(cPlayerInfo* pPlayerInfo);

    bool ReleasePlayerInfo(std::map<DNID, cPlayerInfo*>::iterator& iter);
    bool ReleasePlayerInfo(cPlayerInfo* pPlayerInfo);

    static VOID CALLBACK StaticCompletionRoutine(DWORD, DWORD dwNumBytes, LPOVERLAPPED pOverlapped, DWORD);

    static VOID CALLBACK StaticDeletePlayerApc(ULONG_PTR);
    void DeletePlayerApc(cPlayerInfo* pPlayerInfo);

    static VOID CALLBACK StaticOnDeletePlayerApc(ULONG_PTR);
    void OnDeletePlayerApc(DNID dnid, PVOID context);
};

class cPerIoOperation : public cReferenceCount {
public:
    cDirectNetServer* pDirectNetServer;

    cPlayerInfo* pPlayerInfo;

    WSABUF wsab[2];

    cPerIoOperation() {}

    // this method is invoked inside IoCompletionHandler
    virtual void CompletionHandler(DWORD dwNumBytesTransferred) = 0;

    virtual void FreeBuffer() = 0;

protected:
    virtual ~cPerIoOperation() {}
};

class cRecvOperation : public cPerIoOperation {
public:
    RECEIVING_STATE state;

    void CompletionHandler(DWORD dwNumBytesTransferred)
    {
        pDirectNetServer->RecvCompletionHandler(dwNumBytesTransferred, this);
    }

    void FreeBuffer()
    {
        if (wsab[0].buf) {
            delete[] wsab[0].buf;
            wsab[0].buf = 0;
            wsab[0].len = 0;
            wsab[1].buf = 0;
            wsab[1].len = 0;
        }
    }

protected:
    virtual ~cRecvOperation() {}
};

class cSendOperation : public cPerIoOperation {
public:
    void CompletionHandler(DWORD dwNumBytesTransferred)
    {
        pDirectNetServer->SendCompletionHandler(dwNumBytesTransferred, this);
    }

    void FreeBuffer()
    {
        if (wsab[0].buf) {
            delete[] wsab[0].buf;
            wsab[0].buf = 0;
            wsab[0].len = 0;
        }

        if (wsab[1].buf) {
            delete[] wsab[1].buf;
            wsab[1].buf = 0;
            wsab[1].len = 0;
        }
    }

protected:
    virtual ~cSendOperation() {}
};

struct SOverlappedEx : public OVERLAPPED
{
    SOverlappedEx(cPerIoOperation *pIo) : m_pIo(pIo) 
    {
        OVERLAPPED* ovlp = this;
        ZeroMemory( ovlp, sizeof(OVERLAPPED) );

        pIo->AddRef();
    }

    ~SOverlappedEx() 
    { 
        // m_pIo->FreeBuffer(); 
        m_pIo->Release();
        m_pIo = NULL;
    }

    cPerIoOperation *m_pIo;
};

iDirectNetServer* iDirectNetServer::CreateInstance()
{
    return new cDirectNetServer;
}

void iDirectNetServer::Release()
{
    delete this;
}

cDirectNetServer::cDirectNetServer()
{
    // Get current flag
    int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

    // Turn on leak-checking bit
    tmpFlag = _CRTDBG_ALLOC_MEM_DF | 
        _CRTDBG_LEAK_CHECK_DF;
        // _CRTDBG_CHECK_CRT_DF;

    // Set flag to the new value
    _CrtSetDbgFlag( tmpFlag );

    WSADATA wsad;
    WSAStartup( MAKEWORD(2, 2), &wsad );

    m_bIsInitialized = false;

    m_pDirectNetCryption = NULL;

    m_pDirectNetServerEventHandler = NULL;

    m_dwMaxNumPlayers = 0;

    m_sServer = INVALID_SOCKET;

    m_hAcceptThread = NULL;
    m_hApcHandlerThread = NULL;

    m_hIoCompletionPort = NULL;
    m_dwNumWorkerThreads = 0;

    InitializeCriticalSection(&m_csLock);

    m_hCloseEvent = NULL;
}

cDirectNetServer::~cDirectNetServer()
{
    DeleteCriticalSection(&m_csLock);

    WSACleanup();
}

VOID cDirectNetServer::StaticExitApc(ULONG_PTR)
{
    DEBUG_ONLY( _dout << "cDirectNetServer::StaticExitApc()" << _endl );

    _endthreadex(0);
}

VOID cDirectNetServer::StaticSendApc(ULONG_PTR pParamStruct)
{
    DEBUG_ONLY( _dout << "cDirectNetServer::StaticSendApc()" << _endl );

    SEND_PARAM_STRUCT* pSendParamStruct = (SEND_PARAM_STRUCT*)pParamStruct;

    pSendParamStruct->pDirectNetServer->SendApc(pSendParamStruct->pPlayerInfo,
        pSendParamStruct->pcBuffer, pSendParamStruct->dwLength);

    delete pSendParamStruct;
}

VOID cDirectNetServer::StaticRecvApc(ULONG_PTR pParamStruct)
{
    DEBUG_ONLY( _dout << "cDirectNetServer::StaticRecvApc()" << _endl );

    RECV_PARAM_STRUCT* pRecvParamStruct = (RECV_PARAM_STRUCT*)pParamStruct;

    pRecvParamStruct->pDirectNetServer->RecvApc(pRecvParamStruct->pPlayerInfo);

    delete pRecvParamStruct;
}

VOID cDirectNetServer::StaticOnDeletePlayerApc(ULONG_PTR pParamStruct)
{
    DEBUG_ONLY( _dout << "cDirectNetServer::StaticOnDeletePlayerApc()" << _endl );

    ON_DELETE_PLAYER_PARAM_STRUCT* pOnDeletePlayerParamStruct = (ON_DELETE_PLAYER_PARAM_STRUCT*)pParamStruct;

    pOnDeletePlayerParamStruct->pDirectNetServer->OnDeletePlayerApc(pOnDeletePlayerParamStruct->dnid,
        pOnDeletePlayerParamStruct->context);

    delete pOnDeletePlayerParamStruct;
}

void cDirectNetServer::OnDeletePlayerApc(DNID dnid, PVOID context)
{
    try
    {
        m_pDirectNetServerEventHandler->OnDeletePlayer(dnid, context);
    }
    catch (...)
    {
    }
}

void cDirectNetServer::SendApc(cPlayerInfo* pPlayerInfo, PCHAR pcBuffer, DWORD dwLength)
{
    DNPHDR* pHeader = new DNPHDR;
    pHeader->seqnum = pPlayerInfo->u16OutgoingSequence++;
    pHeader->paclen = (UINT16)dwLength;

    cSendOperation* pSendOperation = new cSendOperation;
    pSendOperation->pDirectNetServer = this;
    pSendOperation->pPlayerInfo = pPlayerInfo;
    pSendOperation->wsab[0].buf = (char*)pHeader;
    pSendOperation->wsab[0].len = sizeof(DNPHDR);
    pSendOperation->wsab[1].buf = pcBuffer;
    pSendOperation->wsab[1].len = dwLength;
  
    SOverlappedEx *pOex = new SOverlappedEx(pSendOperation);

    LPWSAOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine = NULL;
    if (NULL == m_hIoCompletionPort)
        CompletionRoutine = StaticCompletionRoutine;
    DWORD dwNumBytes = 0;
    int ir = WSASend(pPlayerInfo->s, pSendOperation->wsab, 2, &dwNumBytes, 0, pOex, CompletionRoutine);
    if ( SOCKET_ERROR == ir && WSA_IO_PENDING != WSAGetLastError() ) {
        DEBUG_ONLY( _dout << "WSASend() failed: " << DEC( WSAGetLastError() ) << _endl );

        IoCompletionHandler(0, pSendOperation);
    }
}

void cDirectNetServer::RecvApc(cPlayerInfo* pPlayerInfo)
{
    cRecvOperation* pRecvOperation = new cRecvOperation;
    pRecvOperation->pDirectNetServer = this;
    pRecvOperation->pPlayerInfo = pPlayerInfo;
    pRecvOperation->wsab[0].buf = (char*)new CRYPTION;
    pRecvOperation->wsab[0].len = sizeof(CRYPTION);
    pRecvOperation->wsab[1].buf = pRecvOperation->wsab[0].buf;
    pRecvOperation->wsab[1].len = pRecvOperation->wsab[0].len;
    pRecvOperation->state = RECEIVING_HEAD;

    SOverlappedEx *pOex = new SOverlappedEx(pRecvOperation);

    LPWSAOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine = NULL;
    if (NULL == m_hIoCompletionPort)
        CompletionRoutine = StaticCompletionRoutine;
    DWORD dwNumBytes = 0, dwFlags = 0;
    pRecvOperation->AddRef(); // XXX
    int ir = WSARecv(pPlayerInfo->s, &pRecvOperation->wsab[1], 1, &dwNumBytes, &dwFlags, pOex, CompletionRoutine);
    if ( SOCKET_ERROR == ir && WSA_IO_PENDING != WSAGetLastError() ) {
        DEBUG_ONLY( _dout << "WSARecv() failed: " << DEC( WSAGetLastError() ) << _endl );

        IoCompletionHandler(0, pRecvOperation);
    }
}

bool cDirectNetServer::ReleasePlayerInfo(std::map<DNID, cPlayerInfo*>::iterator& iter)
{
    SOCKET s = iter->second->s;
    DNID dnid = iter->second->dnid;
    PVOID context = iter->second->context;

    if ( 0 == iter->second->Release() ) {
        m_PlayerInfoMap.erase(iter);

        closesocket(s);

        // we cannot hold any internal lock when we give control to user mode code, so we
        // differ the transition to our apc handler thread
        //m_pDirectNetServerEventHandler->OnDeletePlayer(dnid, context);
        ON_DELETE_PLAYER_PARAM_STRUCT* pOnDeletePlayerParamStruct = new ON_DELETE_PLAYER_PARAM_STRUCT;
        pOnDeletePlayerParamStruct->pDirectNetServer = this; 
        pOnDeletePlayerParamStruct->dnid = dnid;
        pOnDeletePlayerParamStruct->context = context;
        QueueUserAPC( StaticOnDeletePlayerApc, m_hApcHandlerThread, (ULONG_PTR)pOnDeletePlayerParamStruct );

        return true;
    }

    return false;
}

bool cDirectNetServer::ReleasePlayerInfo(cPlayerInfo* pPlayerInfo)
{
    SOCKET s = pPlayerInfo->s;
    DNID dnid = pPlayerInfo->dnid;
    PVOID context = pPlayerInfo->context;

    if ( 0 == pPlayerInfo->Release() ) {
        m_PlayerInfoMap.erase(dnid);

        closesocket(s);

        // we cannot hold any internal lock when we give control to user mode code, so we
        // differ the transition to our apc handler thread
        //m_pDirectNetServerEventHandler->OnDeletePlayer(dnid, context);
        ON_DELETE_PLAYER_PARAM_STRUCT* pOnDeletePlayerParamStruct = new ON_DELETE_PLAYER_PARAM_STRUCT;
        pOnDeletePlayerParamStruct->pDirectNetServer = this; 
        pOnDeletePlayerParamStruct->dnid = dnid;
        pOnDeletePlayerParamStruct->context = context;
        QueueUserAPC( StaticOnDeletePlayerApc, m_hApcHandlerThread, (ULONG_PTR)pOnDeletePlayerParamStruct );

        return true;
    }

    return false;
}

unsigned int cDirectNetServer::s_accept_thread_procedure(void* pv_this)
{
    reinterpret_cast<cDirectNetServer*>(pv_this)->AcceptThreadProcedure();

    return 0U;
}

unsigned int cDirectNetServer::s_apc_handler_thread_procedure(void* pv_this)
{
    reinterpret_cast<cDirectNetServer*>(pv_this)->ApcHandlerThread();

    return 0U;
}

unsigned int cDirectNetServer::s_iocp_worker_thread_procedure(void* pv_this)
{
    reinterpret_cast<cDirectNetServer*>(pv_this)->IocpWorkerThreadProcedure();

    return 0U;
}

void cDirectNetServer::AcceptThreadProcedure()
{
    while (true) {
        EnterCriticalSection(&m_csLock);
        if ( m_PlayerInfoMap.size() >= m_dwMaxNumPlayers ) {
            LeaveCriticalSection(&m_csLock);
            Sleep(0); continue;
        }
        LeaveCriticalSection(&m_csLock);

        SOCKET s = INVALID_SOCKET;
        sockaddr_in sain;
        ZeroMemory( &sain, sizeof(sain) );
        int salen = sizeof(sain);

        s = accept( m_sServer, (sockaddr*)&sain, &salen );
		if (INVALID_SOCKET == s)
			break; // exit indication

        cPlayerInfo* pPlayerInfo = new cPlayerInfo;
        pPlayerInfo->dnid = dnid_gen();
        pPlayerInfo->s = s;
        pPlayerInfo->sain = sain;
        pPlayerInfo->context = NULL;
        pPlayerInfo->u16IncomingSequence = 0;
        pPlayerInfo->u16OutgoingSequence = 0;
        pPlayerInfo->is_alive = true;

        EnterCriticalSection(&m_csLock);
        m_PlayerInfoMap[pPlayerInfo->dnid] = pPlayerInfo;
        LeaveCriticalSection(&m_csLock);

        if (m_hIoCompletionPort) {
            DEBUG_ONLY( _dout << "associating socket " << HEX(s) << " with completion port " << m_hIoCompletionPort << _endl );
            HANDLE iocp = CreateIoCompletionPort( (HANDLE)s, m_hIoCompletionPort, 0, 0 );
            //assert(iocp == m_hIoCompletionPort);
        }

        // notify upper layer application
        pPlayerInfo->AddRef(); // XXX

        DNID dnidPlayer = pPlayerInfo->dnid;

        try
        {
            m_pDirectNetServerEventHandler->OnCreatePlayer(pPlayerInfo->dnid, &pPlayerInfo->context);
        }
        catch (...)
        {
            DeletePlayer(dnidPlayer);
        }

        // parameter marshalling
        RECV_PARAM_STRUCT* pRecvParamStruct = new RECV_PARAM_STRUCT;
        pRecvParamStruct->pDirectNetServer = this;
        pRecvParamStruct->pPlayerInfo = pPlayerInfo;
        QueueUserAPC( StaticRecvApc, m_hApcHandlerThread, (ULONG_PTR)pRecvParamStruct );
    }
}

void cDirectNetServer::ApcHandlerThread()
{
    // continuously put the thread to the alertable wait state
    // the thread will exit when a special APC is queued to it
    while ( WAIT_IO_COMPLETION == SleepEx(INFINITE, TRUE) );
}

void cDirectNetServer::IocpWorkerThreadProcedure()
{
    ULONG_PTR key = 0;
    DWORD dwNumBytes = 0;
    OVERLAPPED* pOverlapped = NULL;

    while (true) {
        BOOL br = GetQueuedCompletionStatus(m_hIoCompletionPort, &dwNumBytes, &key, &pOverlapped, INFINITE);
        if ( (FALSE == br && NULL == pOverlapped) /* completion port error */ ||
            (TRUE == br && 0 == dwNumBytes && KEY_EXIT == key && NULL == pOverlapped) /* exit indication */ )
            break;

        // normal processing goes here ...
        IoCompletionHandler( dwNumBytes, static_cast<SOverlappedEx*>(pOverlapped)->m_pIo );

        delete static_cast<SOverlappedEx*>(pOverlapped);
    }

    if ( 0 == InterlockedDecrement((LONG*)&m_dwNumWorkerThreads) )
        // the last iocp worker thread has exited
        SetEvent(m_hCloseEvent);
}

void cDirectNetServer::StaticCompletionRoutine(DWORD, DWORD dwNumBytes, LPOVERLAPPED pOverlapped, DWORD)
{
    static_cast<SOverlappedEx*>(pOverlapped)->m_pIo->pDirectNetServer->IoCompletionHandler
        ( dwNumBytes, static_cast<SOverlappedEx*>(pOverlapped)->m_pIo );
}

void cDirectNetServer::IoCompletionHandler(DWORD dwNumBytesTransferred, cPerIoOperation* pPerIoOperation)
{
    cPlayerInfo* pPlayerInfo = pPerIoOperation->pPlayerInfo;

    // first we handle io completion stuff by holding the ref count of the player info
    pPerIoOperation->CompletionHandler(dwNumBytesTransferred);

    // then balance the ref count of the player info
    EnterCriticalSection(&m_csLock);

    // balance the reference count of the io operation; send operation will always be released
    LONG lRefCount = pPerIoOperation->Release();
    if ( ReleasePlayerInfo(pPlayerInfo) ) {
        // the player has been destroyed (active deletion possibly); free the allocated
        // recv or send buffer and release itself, if not done
        if (lRefCount) {
            pPerIoOperation->FreeBuffer();
            pPerIoOperation->Release();
        }
    }

    LeaveCriticalSection(&m_csLock);
}

void cDirectNetServer::RecvCompletionHandler(DWORD dwNumBytesTransferred, cRecvOperation* pRecvOperation)
{
    cPlayerInfo* pPlayerInfo = pRecvOperation->pPlayerInfo;

    if (0 == dwNumBytesTransferred) {
        // the connection is broken
        EnterCriticalSection(&m_csLock);
        if (pPlayerInfo->is_alive) {
            pPlayerInfo->is_alive = false; 
            ReleasePlayerInfo(pPlayerInfo);
        }
        LeaveCriticalSection(&m_csLock);

        pRecvOperation->FreeBuffer();
        pRecvOperation->Release();

        return;
    }

    if (dwNumBytesTransferred < pRecvOperation->wsab[1].len) {
        pRecvOperation->wsab[1].buf += dwNumBytesTransferred;
        pRecvOperation->wsab[1].len -= dwNumBytesTransferred;

        pPlayerInfo->AddRef(); // XXX

        SOverlappedEx *pOex = new SOverlappedEx(pRecvOperation);

        LPWSAOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine = NULL;
        if (NULL == m_hIoCompletionPort)
            CompletionRoutine = StaticCompletionRoutine;
        DWORD dwNumBytes = 0, dwFlags = 0;
        pRecvOperation->AddRef(); // XXX
        int ir = WSARecv(pPlayerInfo->s, &pRecvOperation->wsab[1], 1, &dwNumBytes, &dwFlags, pOex, CompletionRoutine);
        if ( SOCKET_ERROR == ir && WSA_IO_PENDING != WSAGetLastError() ) {
            DEBUG_ONLY( _dout << "WSARecv() failed: " << DEC( WSAGetLastError() ) << _endl );

            IoCompletionHandler(0, pRecvOperation);

            return; // XXX: pRecvOperation MUST be released inside IoCompletionHandler
        }

        return;
    }

    switch (pRecvOperation->state) {
        case RECEIVING_HEAD:
            {
                assert( sizeof(CRYPTION) == pRecvOperation->wsab[0].len );
                CRYPTION cryption = *(CRYPTION*)pRecvOperation->wsab[0].buf;

                // check the validity of the encrypted header
                m_pDirectNetCryption->DES_decrypt( &cryption, sizeof(cryption) );
                UINT32 crc32 = m_pDirectNetCryption->CRC32_compute
                    ( (char*)&cryption + sizeof(cryption.crc32), sizeof(cryption) - sizeof(cryption.crc32) );
                if (crc32 != cryption.crc32) {
                    // invalid connection
                    DEBUG_ONLY( _dout << "invalid packet header from player " << HEX(pPlayerInfo->dnid) << _endl );

                    closesocket(pPlayerInfo->s);
                    EnterCriticalSection(&m_csLock);
                    if (pPlayerInfo->is_alive) {
                        pPlayerInfo->is_alive = false;
                        ReleasePlayerInfo(pPlayerInfo);
                    }
                    LeaveCriticalSection(&m_csLock);

                    pRecvOperation->FreeBuffer();
                    pRecvOperation->Release();

                    return;
                }

                pRecvOperation->FreeBuffer();
                pRecvOperation->state = RECEIVING_BODY;
                pRecvOperation->wsab[0].len = cryption.dnphdr.paclen;
                pPlayerInfo->u16IncomingSequence = cryption.dnphdr.seqnum;
            }
            break;

        case RECEIVING_BODY:
            {
                DNID dnidPlayer = pPlayerInfo->dnid;

                try
                {
                    m_pDirectNetServerEventHandler->OnReceivePacket(pPlayerInfo->dnid, pPlayerInfo->context,
                        pRecvOperation->wsab[0].buf, pRecvOperation->wsab[0].len, pPlayerInfo->u16IncomingSequence);
                }
                catch (...)
                {
                    DeletePlayer(dnidPlayer);
                }

                pRecvOperation->FreeBuffer();
                pRecvOperation->state = RECEIVING_HEAD;
                pRecvOperation->wsab[0].len = sizeof(CRYPTION);
            }
            break;
    }

    pRecvOperation->wsab[0].buf = new char[pRecvOperation->wsab[0].len];
    pRecvOperation->wsab[1].buf = pRecvOperation->wsab[0].buf;
    pRecvOperation->wsab[1].len = pRecvOperation->wsab[0].len;

    pPlayerInfo->AddRef(); // XXX

    SOverlappedEx *pOex = new SOverlappedEx(pRecvOperation);

    LPWSAOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine = NULL;
    if (NULL == m_hIoCompletionPort)
        CompletionRoutine = StaticCompletionRoutine;
    DWORD dwNumBytes = 0, dwFlags = 0;
    pRecvOperation->AddRef(); // XXX
    int ir = WSARecv(pPlayerInfo->s, &pRecvOperation->wsab[1], 1, &dwNumBytes, &dwFlags, pOex, CompletionRoutine);
    if ( SOCKET_ERROR == ir && WSA_IO_PENDING != WSAGetLastError() ) {
        DEBUG_ONLY( _dout << "WSARecv() failed: " << DEC( WSAGetLastError() ) << _endl );

        IoCompletionHandler(0, pRecvOperation);

        return; // XXX: pRecvOperation MUST be released inside IoCompletionHandler
    }

    return;
}

void cDirectNetServer::SendCompletionHandler(DWORD dwNumBytesTransferred, cSendOperation* pSendOperation)
{
    DNID dnidPlayer = pSendOperation->pPlayerInfo->dnid;

    if (0 == dwNumBytesTransferred)
        // the connection is broken
        DEBUG_ONLY( _dout << "last send of player (" << HEX(dnidPlayer) << ") failed!" << _endl );

    // release the associated resources
    pSendOperation->FreeBuffer();
    //pSendOperation->Release();

    return;
}

DNRESULT cDirectNetServer::Initialize(iDirectNetCryption* pDirectCryption, iDirectNetServerEventHandler* pDirectEventHandler, DWORD dwMaxNumPlayers)
{
    if (m_bIsInitialized)
        return DNR_S_OK;

    DEBUG_ONLY( _dout << "cDirectNetServer::Initialize(" << pDirectEventHandler << ", " << dwMaxNumPlayers << ")"
        << _endl );

    m_pDirectNetCryption = pDirectCryption;

    m_pDirectNetServerEventHandler = pDirectEventHandler;

    m_dwMaxNumPlayers = dwMaxNumPlayers;

    m_hIoCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (m_hIoCompletionPort) {
        DEBUG_ONLY( _dout << "io completion port (" << m_hIoCompletionPort << ") created" << _endl );

        SYSTEM_INFO SystemInfo;
        GetSystemInfo(&SystemInfo);
        m_dwNumWorkerThreads = SystemInfo.dwNumberOfProcessors * 2;

        DEBUG_ONLY( _dout << "number of worker threads: " << m_dwNumWorkerThreads << _endl );

        for (DWORD i = 0; i < m_dwNumWorkerThreads; ++i)
            CloseHandle( (HANDLE)_beginthreadex(NULL, 0, s_iocp_worker_thread_procedure, this, 0, NULL) );

        m_hCloseEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // manual-reset, non-signaled

        DEBUG_ONLY( _dout << "close event (" << m_hCloseEvent << ") created" << _endl );

    } else
        DEBUG_ONLY( _dout << "io completion port not supported" << _endl );

    m_hApcHandlerThread = (HANDLE)_beginthreadex(NULL, 0, s_apc_handler_thread_procedure, this, 0, NULL);

    DEBUG_ONLY( _dout << "apc handler thread (" << m_hApcHandlerThread << ") created" << _endl );

    m_bIsInitialized = true;

    return DNR_S_OK;
}

DNRESULT cDirectNetServer::Host(DNPORT dnPort)
{
    if (!m_bIsInitialized)
        return DNR_E_NOT_INITIALIZED;

    DEBUG_ONLY( _dout << "cDirectNetServer::Host(" << DEC(dnPort) << ")" << _endl );

    DNRESULT dnr = DNR_S_OK;

    try {
        // create the server socket, bind it to a fixed port, and put it to listen state
        m_sServer = socket(AF_INET, SOCK_STREAM, 0);
        if (INVALID_SOCKET == m_sServer)
            throw DNR_E_FAIL;

        DEBUG_ONLY( _dout << "server socket (" << HEX(m_sServer) << ") created" << _endl );

        sockaddr_in sain;
        ZeroMemory( &sain, sizeof(sain) );
        sain.sin_family = AF_INET;
        sain.sin_addr.s_addr = INADDR_ANY;
        sain.sin_port = htons(dnPort);
        if ( SOCKET_ERROR == bind( m_sServer, (sockaddr*)&sain, sizeof(sain) ) ) {
            DEBUG_ONLY( _dout << "bind failed on port: " << DEC(dnPort) << _endl );
            throw DNR_E_FAIL;
        }

        if ( SOCKET_ERROR == listen(m_sServer, SOMAXCONN) )
            throw DNR_E_FAIL;

        // create a dedicated accepting thread for this instance,
        // pass the 'this' pointer as the context
        m_hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, s_accept_thread_procedure, this, 0, NULL);

        DEBUG_ONLY( _dout << "accept thread (" << m_hAcceptThread << ") created" << _endl );
    }

    catch (DNRESULT _dnr) {
        dnr = _dnr;
        closesocket(m_sServer);
    }

    return dnr;
}

DNRESULT cDirectNetServer::GetPlayerAddrInfo(DNID dnidPlayer, sockaddr_in *addr, size_t size)
{
    if ( addr == NULL || size < sizeof(sockaddr_in) )
        return DNR_E_INVALID_PARAMETER;

    EnterCriticalSection(&m_csLock);

    try
    {
        std::map<DNID, cPlayerInfo*>::iterator iter = m_PlayerInfoMap.find(dnidPlayer);
        if ( iter == m_PlayerInfoMap.end() ) 
        {
            LeaveCriticalSection(&m_csLock);
            return DNR_E_INVALID_PLAYER;
        }

        *addr = iter->second->sain;
    }
    catch (...)
    {
    }

    LeaveCriticalSection(&m_csLock);

    return DNR_S_OK;
}

DNRESULT cDirectNetServer::SendTo(DNID dnidPlayer, PVOID pvBuffer, DWORD dwLength)
{
    if ( pvBuffer == NULL || dwLength == 0 || dwLength >= (1 << 16) )
        return DNR_E_INVALID_PARAMETER;

    DEBUG_ONLY( _dout << "cDirectNetServer::SendTo(" << HEX(dnidPlayer)
        << ", " << pvBuffer << ", " << DEC(dwLength) << ")" << _endl );

    EnterCriticalSection(&m_csLock);

    std::map<DNID, cPlayerInfo*>::iterator iter = m_PlayerInfoMap.find(dnidPlayer);
    if ( iter == m_PlayerInfoMap.end() ) {
        LeaveCriticalSection(&m_csLock);
        return DNR_E_INVALID_PLAYER;
    }

    // parameter marshalling
    SEND_PARAM_STRUCT* pSendParamStruct = new SEND_PARAM_STRUCT;
    pSendParamStruct->pDirectNetServer = this;
    pSendParamStruct->pPlayerInfo = iter->second;
    pSendParamStruct->pPlayerInfo->AddRef(); // XXX
    pSendParamStruct->pcBuffer = new char[dwLength];
    memcpy(pSendParamStruct->pcBuffer, pvBuffer, dwLength);
    pSendParamStruct->dwLength = dwLength;
    QueueUserAPC( StaticSendApc, m_hApcHandlerThread, (ULONG_PTR)pSendParamStruct );

    LeaveCriticalSection(&m_csLock);

    return DNR_S_PENDING;
}

VOID cDirectNetServer::StaticDeletePlayerApc(ULONG_PTR pParamStruct)
{
    DEBUG_ONLY( _dout << "cDirectNetServer::StaticDeletePlayerApc()" << _endl );

    DELETE_PLAYER_PARAM_STRUCT* pDeletePlayerParamStruct = (DELETE_PLAYER_PARAM_STRUCT*)pParamStruct;

    pDeletePlayerParamStruct->pDirectNetServer->DeletePlayerApc(pDeletePlayerParamStruct->pPlayerInfo);

    delete pDeletePlayerParamStruct;
}

void cDirectNetServer::DeletePlayerApc(cPlayerInfo* pPlayerInfo)
{
    shutdown(pPlayerInfo->s, SD_SEND);
    closesocket(pPlayerInfo->s);

    EnterCriticalSection(&m_csLock);
    ReleasePlayerInfo(pPlayerInfo);
    LeaveCriticalSection(&m_csLock);
}

DNRESULT cDirectNetServer::DeletePlayer(DNID dnidPlayer)
{
    DEBUG_ONLY( _dout << "cDirectNetServer::DeletePlayer(" << HEX(dnidPlayer) << ")" << _endl );

    DNRESULT dnr = DNR_S_OK;

    EnterCriticalSection(&m_csLock);

    try {
        std::map<DNID, cPlayerInfo*>::iterator iter = m_PlayerInfoMap.find(dnidPlayer);
        if ( iter == m_PlayerInfoMap.end() )
            throw DNR_E_INVALID_PLAYER;

        if (iter->second->is_alive) {
            iter->second->is_alive = false;

            DELETE_PLAYER_PARAM_STRUCT* pDeletePlayerParamStruct = new DELETE_PLAYER_PARAM_STRUCT;
            pDeletePlayerParamStruct->pDirectNetServer = this;
            pDeletePlayerParamStruct->pPlayerInfo = iter->second;
            QueueUserAPC( StaticDeletePlayerApc, m_hApcHandlerThread, (ULONG_PTR)pDeletePlayerParamStruct );
        }
    }

    catch (DNRESULT _dnr) {
        dnr = _dnr;
    }

    LeaveCriticalSection(&m_csLock);

    DEBUG_ONLY( _dout << "cDirectNetServer::DeletePlayer() returns: " << HEX(dnr) << _endl );

    return dnr;
}

DNRESULT cDirectNetServer::Close()
{
    if (!m_bIsInitialized)
        return DNR_E_NOT_INITIALIZED;

    DEBUG_ONLY( _dout << "cDirectNetServer::Close()" << _endl );

    closesocket(m_sServer);

    DEBUG_ONLY( _dout << "server socket (" << HEX(m_sServer) << ") closed" << _endl );

    WaitForSingleObject(m_hAcceptThread, INFINITE);
    CloseHandle(m_hAcceptThread);

    DEBUG_ONLY( _dout << "accept thread (" << m_hAcceptThread << ") exited" << _endl );

    EnterCriticalSection(&m_csLock);
    std::map<DNID, cPlayerInfo*>::iterator iter, next = m_PlayerInfoMap.begin();
    while ( next != m_PlayerInfoMap.end() ) {
        iter = next++;

        DeletePlayer(iter->first);
    }
    LeaveCriticalSection(&m_csLock);

    DEBUG_ONLY( _dout << "all the connected players have been deleted" << _endl );

    if (m_hIoCompletionPort) {
        DEBUG_ONLY( _dout << "terminating iocp (" << m_hIoCompletionPort << ") bound worker threads" << _endl );

        DWORD dwNumWorkerThreads = m_dwNumWorkerThreads;
        for (DWORD i = 0; i < dwNumWorkerThreads; ++i)
            PostQueuedCompletionStatus(m_hIoCompletionPort, 0, KEY_EXIT, 0);

        WaitForSingleObject(m_hCloseEvent, INFINITE);
        CloseHandle(m_hCloseEvent);

        DEBUG_ONLY( _dout << "close event (" << m_hCloseEvent << ") closed" << _endl );

        CloseHandle(m_hIoCompletionPort);

        DEBUG_ONLY( _dout << "iocp (" << m_hIoCompletionPort << ") closed" << _endl );
    }

    QueueUserAPC(StaticExitApc, m_hApcHandlerThread, 0);

    WaitForSingleObject(m_hApcHandlerThread, INFINITE);
    CloseHandle(m_hApcHandlerThread);

    DEBUG_ONLY( _dout << "apc handler thread (" << m_hApcHandlerThread << ") exited" << _endl );

    m_bIsInitialized = false;

    return DNR_S_OK;
}

