#include "stdafx.h"

#include "d:\Work\pub\dwt.h"

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
        stream << buffer;
    }
    catch (...)
    {
    }
}

dwt::stringkey<char[256]> GetStringTime()
{
    char buffer[1024];

    SYSTEMTIME s;
    GetLocalTime(&s);

    sprintf(buffer, "%d-%d-%d %d:%d:%d", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);

    return buffer;
}

#include <Dbghelp.h>
#pragma comment (lib, "Dbghelp.lib")

void DumpStack(LPCSTR info)
{
    __try
    {
    __eip:

        DWORD _EIP = 0;
        DWORD _ESP = 0;
        DWORD _EBP = 0;

        _asm
        {
            lea ecx, _EIP
            mov eax, __eip
            mov [ecx], eax

            lea ecx, _ESP
            mov [ecx], esp

            lea ecx, _EBP
            mov [ecx], ebp
        }

        STACKFRAME stackFrame;

	    ::ZeroMemory(&stackFrame, sizeof(stackFrame));

	    stackFrame.AddrPC.Offset = _EIP;
	    stackFrame.AddrPC.Mode = AddrModeFlat;

	    stackFrame.AddrStack.Offset = _ESP;
	    stackFrame.AddrStack.Mode = AddrModeFlat;

	    stackFrame.AddrFrame.Offset = _EBP;
	    stackFrame.AddrFrame.Mode = AddrModeFlat;

	    while (TRUE)
	    {
		    if (StackWalk(IMAGE_FILE_MACHINE_I386, ::GetCurrentProcess(), ::GetCurrentThread(),
			    &stackFrame, NULL, NULL, NULL, NULL, NULL) == FALSE)
			    break;

            if (stackFrame.AddrPC.Offset == _EIP)
            {
                if (info != NULL)
                    Trace("%s\r\n", info);
                else
                    Trace("DumpStack\r\n");
            }
            else
            {
                Trace("0x%08x\r\n", stackFrame.AddrPC.Offset);
            }

            _asm nop
	    }
    }
    __finally
    {
        Trace("End of DumpStack\r\n");
    }
}

/*
void DumpMemory(LPVOID pDumpEntry, INT iOffset, UINT uiSize, LPCSTR szFilename)
{
    try
    {
        HANDLE hFile = CreateFile(szFilename == NULL ? "DumpMemory.Bin" : szFilename, 
            GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, NULL, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			return;

        DWORD dwSize = 0;
        SetFilePointer(hFile, 0, 0, FILE_END);
        WriteFile(hFile, "{[(Head)", sizeof("{[(Head)")-1, &dwSize, 0);

        LPBYTE pEntry = ((LPBYTE)pDumpEntry) + iOffset;
        if (!IsBadReadPtr(pEntry, uiSize))
        {
            WriteFile(hFile, pEntry, uiSize, &dwSize, 0);
        }
        else
        {
            WriteFile(hFile, &pEntry, 4, &dwSize, 0);
            WriteFile(hFile, "IsBadReadPtr", sizeof("IsBadReadPtr")-1, &dwSize, 0);
        }

        WriteFile(hFile, "(Tail)]}", sizeof("(Tail)]}")-1, &dwSize, 0);

		CloseHandle(hFile);
    }
    catch (...)
    {
    }
}

void Dump74FC1834()
{
    try
    {
        DWORD _W64 * pE = (LPDWORD)0x74FC1834;
        DumpMemory(pE, -32, 64, "0x74FC1834");
        DumpMemory((LPVOID)*pE, -256, 2048, "0x74FC1834_Ptr");
    }
    catch (...)
    {
    }
}
*/

#define KEY_EXIT 0xDEADBABE
#define EXITFLAG 987654321

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
    DNRESULT Close();

    DNRESULT GetPlayerConnectionInfo(DNID dnidPlayer, CONNECTIONINFO* pConnectionInfo);

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

    HANDLE m_hApcEvent;
    HANDLE m_hIocpEvent;

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

    static VOID CALLBACK StaticSendApc(ULONG_PTR);
    static VOID CALLBACK StaticRecvApc(ULONG_PTR);

    void SendApc(cPlayerInfo* pPlayerInfo, PCHAR pvBuffer, DWORD dwLength);
    void RecvApc(cPlayerInfo* pPlayerInfo);

    bool ReleasePlayerInfo(cPlayerInfo* pPlayerInfo);

    static VOID CALLBACK StaticCompletionRoutine(DWORD, DWORD dwNumBytes, LPOVERLAPPED pOverlapped, DWORD);

    static VOID CALLBACK StaticDeletePlayerApc(ULONG_PTR);
    void DeletePlayerApc(cPlayerInfo* pPlayerInfo);

    static VOID CALLBACK StaticOnDeletePlayerApc(ULONG_PTR);
    void OnDeletePlayerApc(DNID dnid, PVOID context);

    LONG m_lNumIocpPackets;

    WORD wPort;
    DWORD dwCheck;
    BOOL m_bLogForceCut;
};

class cPerIoOperation : public OVERLAPPED, public cReferenceCount {
public:
    cDirectNetServer* pDirectNetServer;

    cPlayerInfo* pPlayerInfo;

    WSABUF wsab[2];

    cPerIoOperation()
    {
        OVERLAPPED* ovlp = this;
        ZeroMemory( ovlp, sizeof(OVERLAPPED) );
    }

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
    WSADATA wsad;
    WSAStartup( MAKEWORD(2, 2), &wsad );

    // Dump74FC1834();

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

    m_hApcEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // manual-reset, non-signaled
    m_hIocpEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // manual-reset, non-signaled

    m_lNumIocpPackets = 0;

	dwCheck = 0;

    m_bLogForceCut = FALSE;
}

cDirectNetServer::~cDirectNetServer()
{
    CloseHandle(m_hApcEvent);
    CloseHandle(m_hIocpEvent);

    DeleteCriticalSection(&m_csLock);

    WSACleanup();
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
    m_pDirectNetServerEventHandler->OnDeletePlayer(dnid, context);
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

    LPWSAOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine = NULL;
    if (NULL == m_hIoCompletionPort)
        CompletionRoutine = StaticCompletionRoutine;

    DWORD dwNumBytes = 0;

    int ir = SOCKET_ERROR;
    
    try
    {
        ir = WSASend(pPlayerInfo->s, pSendOperation->wsab, 2, &dwNumBytes, 0, pSendOperation, CompletionRoutine);
    }
    catch (...)
    {
        Trace("exception in WSASend [GetLastError = %d] Server [line = %d]\n", GetLastError(), __LINE__);
        // Dump74FC1834();

        return;
    }

    if ( SOCKET_ERROR == ir && WSA_IO_PENDING != WSAGetLastError() ) {
        DEBUG_ONLY( _dout << "WSASend() failed: " << DEC( WSAGetLastError() ) << _endl );

        IoCompletionHandler(0, pSendOperation);

        return;
    }

    if (m_hIoCompletionPort)
        InterlockedIncrement(&m_lNumIocpPackets);
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

    LPWSAOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine = NULL;

    if (NULL == m_hIoCompletionPort)
        CompletionRoutine = StaticCompletionRoutine;

    DWORD dwNumBytes = 0, dwFlags = 0;

    pRecvOperation->AddRef(); // XXX

    int ir = SOCKET_ERROR;

    try
    {
        ir = WSARecv(pPlayerInfo->s, &pRecvOperation->wsab[1], 1, &dwNumBytes, &dwFlags, pRecvOperation, CompletionRoutine);
    }
    catch (...)
    {
        Trace("exception in WSARecv [GetLastError = %d] Server [line = %d]\n", GetLastError(), __LINE__);
        // Dump74FC1834();

        return;
    }

    if ( SOCKET_ERROR == ir && WSA_IO_PENDING != WSAGetLastError() ) {
        DEBUG_ONLY( _dout << "WSARecv() failed: " << DEC( WSAGetLastError() ) << _endl );

        IoCompletionHandler(0, pRecvOperation);

        return;
    }

    if (m_hIoCompletionPort)
        InterlockedIncrement(&m_lNumIocpPackets);
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
	dwCheck = -1;
    
    Trace("[0x%x08] ServerStart [s = %d] [port = %d]\n", this, m_sServer, wPort);

    while (true) {
        EnterCriticalSection(&m_csLock);
        if ( m_PlayerInfoMap.size() >= m_dwMaxNumPlayers ) {
            LeaveCriticalSection(&m_csLock);
            Sleep(1); 
			dwCheck = -2;
			continue;
        }
        LeaveCriticalSection(&m_csLock);

        SOCKET s = INVALID_SOCKET;
        sockaddr_in sain;
        ZeroMemory( &sain, sizeof(sain) );
        int salen = sizeof(sain);

        s = accept( m_sServer, (sockaddr*)&sain, &salen );

        if (dwCheck == EXITFLAG)
            break;

        if (INVALID_SOCKET == s)
		{
			dwCheck = -3;

            Trace("[0x%x08] accept fail [GetLastError = %d] [s = %d]\n", this, GetLastError(), m_sServer);

            if (SOCKET_ERROR == closesocket(m_sServer))
            {
                Trace("[0x%x08] closesocket fail in resume [GetLastError = %d] [s = %d]\n", this, GetLastError(), m_sServer);
                break;
            }

            // create the server socket, bind it to a fixed port, and put it to listen state
            m_sServer = socket(AF_INET, SOCK_STREAM, 0);
            if (INVALID_SOCKET == m_sServer)
            {
                Trace("[0x%x08] create socket fail in resume [GetLastError = %d]\n", this, GetLastError());
                break;
            }

            sockaddr_in sain;
            ZeroMemory( &sain, sizeof(sain) );
            sain.sin_family = AF_INET;
            sain.sin_addr.s_addr = INADDR_ANY;
            sain.sin_port = htons(wPort);
            if (SOCKET_ERROR == bind(m_sServer, (sockaddr*)&sain, sizeof(sain))) 
            {
                Trace("[0x%x08] bind socket fail in resume [GetLastError = %d] [port = %d]\n", this, GetLastError(), wPort);
                break;
            }

            if ( SOCKET_ERROR == listen(m_sServer, SOMAXCONN) )
            {
                Trace("[0x%x08] listen socket fail in resume [GetLastError = %d] [port = %d]\n", this, GetLastError(), wPort);
                break;
            }

            Trace("[0x%x08] ServerRestart [s = %d] [port = %d]\n", this, m_sServer, wPort);

            continue;
		}

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
        m_pDirectNetServerEventHandler->OnCreatePlayer(pPlayerInfo->dnid, &pPlayerInfo->context);

        // parameter marshalling
        RECV_PARAM_STRUCT* pRecvParamStruct = new RECV_PARAM_STRUCT;
        pRecvParamStruct->pDirectNetServer = this;
        pRecvParamStruct->pPlayerInfo = pPlayerInfo;
        QueueUserAPC( StaticRecvApc, m_hApcHandlerThread, (ULONG_PTR)pRecvParamStruct );
    }
}

void cDirectNetServer::ApcHandlerThread()
{
    // continuously put the thread to the alertable wait state; the thread will exit
    // when the apc event has been signaled
    while (true) {
        DWORD dw = WaitForSingleObjectEx(m_hApcEvent, INFINITE, TRUE);
        switch (dw) {
            case WAIT_OBJECT_0:
                return;

            case WAIT_IO_COMPLETION:
                continue;

            case WAIT_FAILED:
                // what should we do here?
                return;
        }
    }
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

        InterlockedDecrement(&m_lNumIocpPackets);

        // normal processing goes here ...
        IoCompletionHandler( dwNumBytes, static_cast<cPerIoOperation*>(pOverlapped) );
    }

    if ( 0 == InterlockedDecrement((LONG*)&m_dwNumWorkerThreads) ) {
        // cleanup the completion port
        while (m_lNumIocpPackets > 0) {
            BOOL br = GetQueuedCompletionStatus(m_hIoCompletionPort, &dwNumBytes, &key, &pOverlapped, INFINITE);
            if (FALSE == br && NULL == pOverlapped) // completion port error
                break;

            --m_lNumIocpPackets;

            IoCompletionHandler( dwNumBytes, static_cast<cPerIoOperation*>(pOverlapped) );
        }

        // the last iocp worker thread is exiting, signal the event
        SetEvent(m_hIocpEvent);
    }
}

void cDirectNetServer::StaticCompletionRoutine(DWORD, DWORD dwNumBytes, LPOVERLAPPED pOverlapped, DWORD)
{
    static_cast<cPerIoOperation*>(pOverlapped)->pDirectNetServer->IoCompletionHandler
        ( dwNumBytes, static_cast<cPerIoOperation*>(pOverlapped) );
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
        LPWSAOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine = NULL;
        if (NULL == m_hIoCompletionPort)
            CompletionRoutine = StaticCompletionRoutine;

        DWORD dwNumBytes = 0, dwFlags = 0;

        pRecvOperation->AddRef(); // XXX

        int ir = SOCKET_ERROR;

        try
        {
            ir = WSARecv(pPlayerInfo->s, &pRecvOperation->wsab[1], 1, &dwNumBytes, &dwFlags, pRecvOperation, CompletionRoutine);
        }
        catch (...)
        {
            Trace("exception in WSARecv [GetLastError = %d] Server [line = %d]\n", GetLastError(), __LINE__);
            // Dump74FC1834();

            return;
        }

        if ( SOCKET_ERROR == ir && WSA_IO_PENDING != WSAGetLastError() ) {
            DEBUG_ONLY( _dout << "WSARecv() failed: " << DEC( WSAGetLastError() ) << _endl );

            IoCompletionHandler(0, pRecvOperation);

            return; // XXX: pRecvOperation MUST be released inside IoCompletionHandler
        }

        if (m_hIoCompletionPort)
            InterlockedIncrement(&m_lNumIocpPackets);

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
                m_pDirectNetServerEventHandler->OnReceivePacket(pPlayerInfo->dnid, pPlayerInfo->context,
                    pRecvOperation->wsab[0].buf, pRecvOperation->wsab[0].len, pPlayerInfo->u16IncomingSequence);

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
    LPWSAOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine = NULL;
    if (NULL == m_hIoCompletionPort)
        CompletionRoutine = StaticCompletionRoutine;

    DWORD dwNumBytes = 0, dwFlags = 0;

    pRecvOperation->AddRef(); // XXX

    int ir = SOCKET_ERROR;
    
    try
    {
        ir = WSARecv(pPlayerInfo->s, &pRecvOperation->wsab[1], 1, &dwNumBytes, &dwFlags, pRecvOperation, CompletionRoutine);
    }
    catch (...)
    {
        Trace("exception in WSARecv [GetLastError = %d] Server [line = %d]\n", GetLastError(), __LINE__);
        // Dump74FC1834();

        return;
    }

    if ( SOCKET_ERROR == ir && WSA_IO_PENDING != WSAGetLastError() ) {
        DEBUG_ONLY( _dout << "WSARecv() failed: " << DEC( WSAGetLastError() ) << _endl );

        IoCompletionHandler(0, pRecvOperation);

        return; // XXX: pRecvOperation MUST be released inside IoCompletionHandler
    }

    if (m_hIoCompletionPort)
        InterlockedIncrement(&m_lNumIocpPackets);

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

    if (dnPort == 7116)
        m_bLogForceCut = TRUE;

    wPort = dnPort;

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

DNRESULT cDirectNetServer::SendTo(DNID dnidPlayer, PVOID pvBuffer, DWORD dwLength)
{
    if ( pvBuffer == NULL || dwLength == 0 || dwLength >= (1 << 16) )
        return DNR_E_INVALID_PARAMETER;

    DEBUG_ONLY( _dout << "cDirectNetServer::SendTo(" << HEX(dnidPlayer)
        << ", " << pvBuffer << ", " << DEC(dwLength) << ")" << _endl );

    EnterCriticalSection(&m_csLock);

    std::map<DNID, cPlayerInfo*>::iterator iter = m_PlayerInfoMap.find(dnidPlayer);
    if ( iter == m_PlayerInfoMap.end() || !iter->second->is_alive ) {
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

    std::map<DNID, cPlayerInfo*>::iterator iter = m_PlayerInfoMap.find(dnidPlayer);
    if ( iter == m_PlayerInfoMap.end() )
        dnr = DNR_E_INVALID_PLAYER;
    else if (iter->second->is_alive) {
        iter->second->is_alive = false;

        DELETE_PLAYER_PARAM_STRUCT* pDeletePlayerParamStruct = new DELETE_PLAYER_PARAM_STRUCT;
        pDeletePlayerParamStruct->pDirectNetServer = this;
        pDeletePlayerParamStruct->pPlayerInfo = iter->second;
        QueueUserAPC( StaticDeletePlayerApc, m_hApcHandlerThread, (ULONG_PTR)pDeletePlayerParamStruct );

        if (m_bLogForceCut)
        {
            Trace("[%s]联接主动断开", GetStringTime());
            DumpStack(NULL);
        }
    }

    LeaveCriticalSection(&m_csLock);

    DEBUG_ONLY( _dout << "cDirectNetServer::DeletePlayer() returns: " << HEX(dnr) << _endl );

    return dnr;
}

DNRESULT cDirectNetServer::GetPlayerConnectionInfo(DNID dnidPlayer, CONNECTIONINFO* pConnectionInfo)
{
    DNRESULT dnr = DNR_S_OK;

    EnterCriticalSection(&m_csLock);

	if (dnidPlayer == -1)
	{
		if (dwCheck == -1)
			pConnectionInfo->addr = (UINT32)m_PlayerInfoMap.size();
		else
			pConnectionInfo->addr = dwCheck;

		pConnectionInfo->port = 0;
	}
	else
	{
		std::map<DNID, cPlayerInfo*>::iterator iter = m_PlayerInfoMap.find(dnidPlayer);
		if ( iter == m_PlayerInfoMap.end() )
			dnr = DNR_E_INVALID_PLAYER;
		else {
			pConnectionInfo->addr = iter->second->sain.sin_addr.s_addr;
			pConnectionInfo->port = iter->second->sain.sin_port;
		}
	}

    LeaveCriticalSection(&m_csLock);

    return dnr;
}

DNRESULT cDirectNetServer::Close()
{
    if (!m_bIsInitialized)
        return DNR_E_NOT_INITIALIZED;

    DEBUG_ONLY( _dout << "cDirectNetServer::Close()" << _endl );

    dwCheck = EXITFLAG;
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

        WaitForSingleObject(m_hIocpEvent, INFINITE);

        CloseHandle(m_hIoCompletionPort);

        DEBUG_ONLY( _dout << "iocp (" << m_hIoCompletionPort << ") closed" << _endl );
    }

    Sleep(1); // XXX
    SetEvent(m_hApcEvent);

    WaitForSingleObject(m_hApcHandlerThread, INFINITE);
    CloseHandle(m_hApcHandlerThread);

    DEBUG_ONLY( _dout << "apc handler thread (" << m_hApcHandlerThread << ") exited" << _endl );

    m_bIsInitialized = false;

    return DNR_S_OK;
}

