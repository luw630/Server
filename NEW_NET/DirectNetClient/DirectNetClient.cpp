#include "stdafx.h"

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

    // try
    {
        va_list header;
        va_start(header, szFormat);
        _vsnprintf(buffer, 1024, szFormat, header);
        va_end(header);

        buffer[1024] = 0;

        std::ofstream stream("client_dinfo.txt", std::ios_base::app);
        stream << GetStringTime() << buffer;
    }
    //catch (...)
    //{
    //}
}
//
//void DumpMemory(LPVOID pDumpEntry, INT iOffset, UINT uiSize, LPCSTR szFilename)
//{
//    try
//    {
//        HANDLE hFile = CreateFile(szFilename == NULL ? "DumpMemory.Bin" : szFilename, 
//            GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, NULL, NULL);
//
//		if (hFile == INVALID_HANDLE_VALUE)
//			return;
//
//        DWORD dwSize = 0;
//        SetFilePointer(hFile, 0, 0, FILE_END);
//        WriteFile(hFile, "{[(Head)", sizeof("{[(Head)")-1, &dwSize, 0);
//
//        LPBYTE pEntry = ((LPBYTE)pDumpEntry) + iOffset;
//        if (!IsBadReadPtr(pEntry, uiSize))
//        {
//            WriteFile(hFile, pEntry, uiSize, &dwSize, 0);
//        }
//        else
//        {
//            WriteFile(hFile, &pEntry, 4, &dwSize, 0);
//            WriteFile(hFile, "IsBadReadPtr", sizeof("IsBadReadPtr")-1, &dwSize, 0);
//        }
//
//        WriteFile(hFile, "(Tail)]}", sizeof("(Tail)]}")-1, &dwSize, 0);
//
//		CloseHandle(hFile);
//    }
//    catch (...)
//    {
//    }
//}
//
//void Dump74FC1834()
//{
//    try
//    {
//        DWORD _W64 * pE = (LPDWORD)0x74FC1834;
//        DumpMemory(pE, -32, 64, "0x74FC1834");
//        DumpMemory((LPVOID)*pE, -256, 2048, "0x74FC1834_Ptr");
//    }
//    catch (...)
//    {
//    }
//}

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

enum RECEIVING_STATE { RECEIVING_HEAD, RECEIVING_BODY };

struct CMMN_PARAM_STRUCT {
    class cDirectNetClient* pDirectNetClient;
};

struct SEND_PARAM_STRUCT : public CMMN_PARAM_STRUCT {
    PCHAR pcBuffer;
    DWORD dwLength;
};

#define DISCONNECT_PARAM_STRUCT CMMN_PARAM_STRUCT

class cDirectNetClient : public iDirectNetClient {
    friend class iDirectNetClient;

public:
    DNRESULT Initialize(iDirectNetCryption* pDirectNetCryption,
        iDirectNetClientEventHandler* pDirectNetClientEventHandler);
    DNRESULT Connect(LPCSTR szServerIpAddress, DNPORT dnPort);
    DNRESULT Send(PVOID pvBuffer, DWORD dwLength);
    DNRESULT Disconnect();
    DNRESULT Close();

    void RecvCompletionHandler(DWORD dwNumBytesTransferred, class cRecvOperation* pRecvOperation);
    void SendCompletionHandler(DWORD dwNumBytesTransferred, class cSendOperation* pSendOperation);

private:
    cDirectNetClient();
    ~cDirectNetClient();

    bool m_bIsInitialized;

    iDirectNetCryption* m_pDirectNetCryption;

    iDirectNetClientEventHandler* m_pDirectNetClientEventHandler;

    SOCKET m_sClient;

    UINT32 m_dwServerAddr;
    DNPORT m_dnServerPort;

    enum CONNECTION_STATE {
        CONNECTION_NOTCONNECTED, CONNECTION_INPROGRESS, CONNECTION_ESTABLISHED
    } m_eConnectionState;

    UINT16 m_u16OutgoingSequence;
    UINT16 m_u16IncomingSequence;

    HANDLE m_hIoHandlerThread;
    void IoHandlerThreadProcedure();
    static unsigned int __stdcall s_io_handler_thread_procedure(void*);

    void IoCompletionHandler(DWORD dwNumBytesTransferred, class cPerIoOperation* pPerIoOperation);
    static VOID CALLBACK StaticCompletionRoutine(DWORD, DWORD dwNumBytes, LPOVERLAPPED pOverlapped, DWORD);

    void SendApc(PCHAR pcBuffer, DWORD dwLength);
    static VOID CALLBACK StaticSendApc(ULONG_PTR);

    static VOID CALLBACK StaticDisconnectApc(ULONG_PTR);
    void DisconnectApc();

    CRITICAL_SECTION m_csLock;

    HANDLE m_hApcEvent;
};

class cPerIoOperation : public OVERLAPPED {
public:
    cDirectNetClient* pDirectNetClient;

    WSABUF wsab[2];

    cPerIoOperation()
    {
        OVERLAPPED* ovlp = this;
        ZeroMemory( ovlp, sizeof(OVERLAPPED) );
    }

    virtual void CompletionHandler(DWORD dwNumBytesTransferred) = 0;

    virtual void FreeBuffer() = 0;

    void Release()
    {
        delete this;
    }

protected:
    virtual ~cPerIoOperation() {}
};

class cRecvOperation : public cPerIoOperation {
public:
    RECEIVING_STATE state;

    void CompletionHandler(DWORD dwNumBytesTransferred)
    {
        pDirectNetClient->RecvCompletionHandler(dwNumBytesTransferred, this);
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
        pDirectNetClient->SendCompletionHandler(dwNumBytesTransferred, this);
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
    WSADATA WSAData;
    WSAStartup( MAKEWORD(2, 2), &WSAData );

    m_bIsInitialized = false;

    m_pDirectNetCryption = NULL;

    m_pDirectNetClientEventHandler = NULL;

    m_sClient = INVALID_SOCKET;

    m_u16OutgoingSequence = 0;
    m_u16IncomingSequence = 0;

    m_dwServerAddr = 0;
    m_dnServerPort = 0;

    m_eConnectionState = CONNECTION_NOTCONNECTED;

    m_hIoHandlerThread = NULL;

    InitializeCriticalSection(&m_csLock);

    m_hApcEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // manual-reset, non-signaled
}

cDirectNetClient::~cDirectNetClient()
{
    CloseHandle(m_hApcEvent);

    DeleteCriticalSection(&m_csLock);

    WSACleanup();
}

DNRESULT cDirectNetClient::Initialize(iDirectNetCryption* pDirectCryption, iDirectNetClientEventHandler* pDirectEventHandler)
{
    if (m_bIsInitialized)
        return DNR_S_OK;

    DEBUG_ONLY( _dout << "cDirectNetClient::Initialize()" << _endl );

    m_pDirectNetCryption = pDirectCryption;

    m_pDirectNetClientEventHandler = pDirectEventHandler;

    m_bIsInitialized = true;

    return DNR_S_OK;
}

DNRESULT cDirectNetClient::Connect(LPCSTR szServerIpAddress, DNPORT dnPort)
{
    if (!m_bIsInitialized)
        return DNR_E_NOT_INITIALIZED;

    if (CONNECTION_NOTCONNECTED != m_eConnectionState)
        return DNR_E_CONNECT_AGAIN;

    DEBUG_ONLY( _dout << "connecting to server " << szServerIpAddress << ":" << DEC(dnPort) << _endl );

    m_eConnectionState = CONNECTION_INPROGRESS;

    m_dwServerAddr = inet_addr(szServerIpAddress);
    m_dnServerPort = dnPort; // host byte order

    m_hIoHandlerThread = (HANDLE)_beginthreadex(NULL, 0, s_io_handler_thread_procedure, this, 0, NULL);

    DEBUG_ONLY( _dout << "io handler thread (" << m_hIoHandlerThread << ") created" << _endl );

    return DNR_S_PENDING;
}

unsigned int cDirectNetClient::s_io_handler_thread_procedure(void* pv_this)
{
    reinterpret_cast<cDirectNetClient*>(pv_this)->IoHandlerThreadProcedure();

    return 0U;
}

void cDirectNetClient::IoHandlerThreadProcedure()
{
    try 
    {
        m_sClient = socket(AF_INET, SOCK_STREAM, 0);
        if (INVALID_SOCKET == m_sClient) {
            DEBUG_ONLY( _dout << "socket creation failed" << _endl );

            throw DNR_E_FAIL;
        }

        DEBUG_ONLY( _dout << "client socket (" << HEX(m_sClient) << ") created" << _endl );

        sockaddr_in sain;
        ZeroMemory( &sain, sizeof(sain) );
        sain.sin_family = AF_INET;
        sain.sin_addr.s_addr = m_dwServerAddr;
        sain.sin_port = htons(m_dnServerPort);
        if ( SOCKET_ERROR == connect( m_sClient, (sockaddr*)&sain, sizeof(sain) ) ) {
            DEBUG_ONLY( _dout << "connection failed" << _endl );

            throw DNR_E_FAIL;
        }
    }

    catch (DNRESULT dnr) {
        closesocket(m_sClient);
        m_eConnectionState = CONNECTION_NOTCONNECTED;
        m_pDirectNetClientEventHandler->OnConnectComplete(dnr);
        return;
    }

    DEBUG_ONLY( _dout << "connection established" << _endl );

    EnterCriticalSection(&m_csLock);
    m_eConnectionState = CONNECTION_ESTABLISHED;
    LeaveCriticalSection(&m_csLock);
    m_pDirectNetClientEventHandler->OnConnectComplete(DNR_S_OK);

    cRecvOperation* pRecvOperation = new cRecvOperation;
    pRecvOperation->pDirectNetClient = this;
    pRecvOperation->wsab[0].buf = (char*)new DNPHDR;
    pRecvOperation->wsab[0].len = sizeof(DNPHDR);
    pRecvOperation->wsab[1].buf = pRecvOperation->wsab[0].buf;
    pRecvOperation->wsab[1].len = pRecvOperation->wsab[0].len;
    pRecvOperation->state = RECEIVING_HEAD;

    //DWORD dwNumBytes = 0, dwFlags = 0;

    int ir = SOCKET_ERROR;

//    try
    {
        ir = WSARecv(m_sClient, &pRecvOperation->wsab[1], 1, &s_dwTransferredBytes, &s_dwFlags, pRecvOperation, StaticCompletionRoutine);
    }
    //catch (...)
    //{
    //    Trace("exception in WSARecv [GetLastError = %d] Client [line = %d]\n", GetLastError(), __LINE__);
    //    return;
    //}

    int le = WSAGetLastError();
    if ( SOCKET_ERROR == ir && WSA_IO_PENDING != le ) 
    {
        Trace("Net Break !!! le = %d line = %d\r\n", le, __LINE__);

        IoCompletionHandler(0, pRecvOperation);
        return;
    }

    // continuously put the thread to the alertable wait state; the thread will exit
    // when the apc event has been signaled
    while (true) {
        DWORD dw = WaitForSingleObjectEx(m_hApcEvent, INFINITE, TRUE);
        if (WAIT_IO_COMPLETION == dw)
            continue;
        else if (WAIT_OBJECT_0 == dw)
            break;
        else if (WAIT_FAILED == dw)
            break;
    }

    closesocket(m_sClient);
    m_eConnectionState = CONNECTION_NOTCONNECTED;
}

DNRESULT cDirectNetClient::Send(PVOID pvBuffer, DWORD dwLength)
{
    if ( pvBuffer == NULL || dwLength == 0 || dwLength >= (1 << 16) )
        return DNR_E_INVALID_PARAMETER;

    EnterCriticalSection(&m_csLock);

    if (CONNECTION_ESTABLISHED != m_eConnectionState) {
        LeaveCriticalSection(&m_csLock);
        return DNR_E_NOT_CONNECTED;
    }

    DEBUG_ONLY( _dout << "cDirectNetClient::Send(" << pvBuffer << ", " << DEC(dwLength) << ")" << _endl );

    SEND_PARAM_STRUCT* pSendParamStruct = new SEND_PARAM_STRUCT;
    pSendParamStruct->pDirectNetClient = this;
    pSendParamStruct->pcBuffer = new char[dwLength];
    memcpy(pSendParamStruct->pcBuffer, pvBuffer, dwLength);
    pSendParamStruct->dwLength = dwLength;
    QueueUserAPC( StaticSendApc, m_hIoHandlerThread, (ULONG_PTR)pSendParamStruct );

    LeaveCriticalSection(&m_csLock);

    return DNR_S_PENDING;
}

VOID cDirectNetClient::StaticSendApc(ULONG_PTR pParamStruct)
{
    DEBUG_ONLY( _dout << "cDirectNetClient::StaticSendApc()" << _endl );

    SEND_PARAM_STRUCT* pSendParamStruct = (SEND_PARAM_STRUCT*)pParamStruct;

    pSendParamStruct->pDirectNetClient->SendApc(pSendParamStruct->pcBuffer, pSendParamStruct->dwLength);

    delete pSendParamStruct;
}

void cDirectNetClient::SendApc(PCHAR pcBuffer, DWORD dwLength)
{
    // build the encrypted header
    CRYPTION* pHeader = new CRYPTION;
    pHeader->dnphdr.seqnum = m_u16OutgoingSequence++;
    pHeader->dnphdr.paclen = (UINT16)dwLength;
    UINT64 u64; QueryPerformanceCounter( (LARGE_INTEGER*)&u64 );
    UINT32 u32 = GetTickCount();
    pHeader->signature = ( (u64 << 32) | u32 );
    pHeader->crc32 = m_pDirectNetCryption->CRC32_compute
        ( (char*)pHeader + sizeof(pHeader->crc32), sizeof(CRYPTION) - sizeof(pHeader->crc32) );
    m_pDirectNetCryption->DES_encrypt( pHeader, sizeof(CRYPTION) );

    cSendOperation* pSendOperation = new cSendOperation;
    pSendOperation->pDirectNetClient = this;
    pSendOperation->wsab[0].buf = (char*)pHeader;
    pSendOperation->wsab[0].len = sizeof(CRYPTION);
    pSendOperation->wsab[1].buf = pcBuffer;
    pSendOperation->wsab[1].len = dwLength;

    //DWORD dwNumBytes = 0;
    int ir = SOCKET_ERROR;
//    try
    {
        ir = WSASend(m_sClient, pSendOperation->wsab, 2, &s_dwTransferredBytes, 0, pSendOperation, StaticCompletionRoutine);
    }
    //catch (...)
    //{
    //    Trace("exception in WSASend [GetLastError = %d] Client [line = %d]\n", GetLastError(), __LINE__);
    //    return;
    //}

    int le = WSAGetLastError();
    if ( SOCKET_ERROR == ir && WSA_IO_PENDING != le )
    {
        Trace("Net Break !!! le = %d line = %d\r\n", le, __LINE__);

        IoCompletionHandler(0, pSendOperation);
    }
}

VOID cDirectNetClient::StaticCompletionRoutine(DWORD, DWORD dwNumBytes, LPOVERLAPPED pOverlapped, DWORD)
{
    static_cast<cPerIoOperation*>(pOverlapped)->pDirectNetClient->IoCompletionHandler
        ( dwNumBytes, static_cast<cPerIoOperation*>(pOverlapped) );
}

void cDirectNetClient::IoCompletionHandler(DWORD dwNumBytesTransferred, cPerIoOperation* pPerIoOperation)
{
    pPerIoOperation->CompletionHandler(dwNumBytesTransferred);
}

void cDirectNetClient::RecvCompletionHandler(DWORD dwNumBytesTransferred, cRecvOperation* pRecvOperation)
{
    if (0 == dwNumBytesTransferred) {
        DEBUG_ONLY( _dout << "connection is broken" << _endl );

        EnterCriticalSection(&m_csLock);
        m_eConnectionState = CONNECTION_NOTCONNECTED;
        LeaveCriticalSection(&m_csLock);

        m_pDirectNetClientEventHandler->OnTerminateSession();

        pRecvOperation->FreeBuffer();
        pRecvOperation->Release();

        SetEvent(m_hApcEvent);

        return;
    }

    if (dwNumBytesTransferred < pRecvOperation->wsab[1].len) {
        pRecvOperation->wsab[1].buf += dwNumBytesTransferred;
        pRecvOperation->wsab[1].len -= dwNumBytesTransferred;

        //DWORD dwNumBytes = 0, dwFlags = 0;

        int ir = SOCKET_ERROR;

//        try
        {
            ir = WSARecv(m_sClient, &pRecvOperation->wsab[1], 1, &s_dwTransferredBytes, &s_dwFlags, pRecvOperation, StaticCompletionRoutine);
        }
        //catch (...)
        //{
        //    Trace("exception in WSARecv [GetLastError = %d] Client [line = %d]\n", GetLastError(), __LINE__);
        //    return;
        //}

        int le = WSAGetLastError();
        if ( SOCKET_ERROR == ir && WSA_IO_PENDING != le )
        {
            Trace("Net Break !!! le = %d line = %d\r\n", le, __LINE__);

            IoCompletionHandler(0, pRecvOperation);
        }

        return;
    }

    switch (pRecvOperation->state) {
        case RECEIVING_HEAD:
            {
                assert( sizeof(DNPHDR) == pRecvOperation->wsab[0].len );
                DNPHDR dnphdr = *(DNPHDR*)pRecvOperation->wsab[0].buf;
                pRecvOperation->FreeBuffer();
                pRecvOperation->state = RECEIVING_BODY;
                pRecvOperation->wsab[0].len = dnphdr.paclen;
                m_u16IncomingSequence = dnphdr.seqnum;
            }
            break;

        case RECEIVING_BODY:
            {
                if (pRecvOperation->wsab[0].len == 8)
                {
                    if (((LPDWORD)(pRecvOperation->wsab[0].buf))[0] == *(LPDWORD)"pong")
                    {
                        sockaddr_in addr;

                        addr.sin_port = 0;
                        addr.sin_addr.S_un.S_addr = 0;

                        int len = sizeof(addr);
                        if (getpeername(m_sClient, (sockaddr *)&addr, &len) == SOCKET_ERROR)
                        {
                            Trace("getsockname fail ! le = %d", WSAGetLastError());
                        }
                        else
                        {
                            Trace("recv pong [IP = %s:%d][time = 0x%08x]\r\n", 
                                inet_ntoa(addr.sin_addr), addr.sin_port, 
                                ((LPDWORD)(pRecvOperation->wsab[0].buf))[1]);
                        }
                    }
                }

                m_pDirectNetClientEventHandler->OnReceivePacket(pRecvOperation->wsab[0].buf, pRecvOperation->wsab[0].len,
                    m_u16IncomingSequence);

                pRecvOperation->FreeBuffer();
                pRecvOperation->state = RECEIVING_HEAD;
                pRecvOperation->wsab[0].len = sizeof(DNPHDR);
            }
            break;
    }

    pRecvOperation->wsab[0].buf = new char[pRecvOperation->wsab[0].len];
    pRecvOperation->wsab[1].buf = pRecvOperation->wsab[0].buf;
    pRecvOperation->wsab[1].len = pRecvOperation->wsab[0].len;

    //DWORD dwNumBytes = 0, dwFlags = 0;

    int ir = SOCKET_ERROR;

//    try
    {
        ir = WSARecv(m_sClient, &pRecvOperation->wsab[1], 1, &s_dwTransferredBytes , &s_dwFlags, pRecvOperation, StaticCompletionRoutine);
    }
    //catch (...)
    //{
    //    Trace("exception in WSARecv [GetLastError = %d] Client [line = %d]\n", GetLastError(), __LINE__);
    //    return;
    //}

    int le = WSAGetLastError();
    if ( SOCKET_ERROR == ir && WSA_IO_PENDING != le )
    {
        Trace("Net Break !!! le = %d line = %d\r\n", le, __LINE__);

        IoCompletionHandler(0, pRecvOperation);
    }

    return;
}

void cDirectNetClient::SendCompletionHandler(DWORD dwNumBytesTransferred, cSendOperation* pSendOperation)
{
    pSendOperation->FreeBuffer();
    pSendOperation->Release();
}

VOID cDirectNetClient::StaticDisconnectApc(ULONG_PTR pParamStruct)
{
    DEBUG_ONLY( _dout << "cDirectNetClient::StaticDisconnectApc()" << _endl );

    DISCONNECT_PARAM_STRUCT* pDisconnectParamStruct = (DISCONNECT_PARAM_STRUCT*)pParamStruct;

    pDisconnectParamStruct->pDirectNetClient->DisconnectApc();

    delete pDisconnectParamStruct;
}

void cDirectNetClient::DisconnectApc()
{
    shutdown(m_sClient, SD_SEND);
    closesocket(m_sClient);
}

DNRESULT cDirectNetClient::Disconnect()
{
    EnterCriticalSection(&m_csLock);
    m_eConnectionState = CONNECTION_NOTCONNECTED;
    DISCONNECT_PARAM_STRUCT* pDisconnectParamStruct = new DISCONNECT_PARAM_STRUCT;
    pDisconnectParamStruct->pDirectNetClient = this;
    QueueUserAPC( StaticDisconnectApc, m_hIoHandlerThread, (ULONG_PTR)pDisconnectParamStruct );
    LeaveCriticalSection(&m_csLock);

    return DNR_S_OK;
}

DNRESULT cDirectNetClient::Close()
{
    if (!m_bIsInitialized)
        return DNR_E_NOT_INITIALIZED;

    Disconnect();

    DEBUG_ONLY( _dout << "client socket (" << HEX(m_sClient) << ") closed" << _endl );

    SetEvent(m_hApcEvent);

    WaitForSingleObject(m_hIoHandlerThread, INFINITE);
    CloseHandle(m_hIoHandlerThread);

    DEBUG_ONLY( _dout << "io handler thread (" << m_hIoHandlerThread << ") exited" << _endl );

    m_bIsInitialized = false;

    return DNR_S_OK;
}

