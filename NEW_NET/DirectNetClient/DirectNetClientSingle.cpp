#include "stdafx.h"

// private functions
unsigned int __stdcall s_io_handler_thread_procedure(void*);

VOID CALLBACK StaticSendCompletionRoutine(DWORD, DWORD dwNumBytes, LPWSAOVERLAPPED pOverlapped, DWORD);
VOID CALLBACK StaticRecvCompletionRoutine(DWORD, DWORD dwNumBytes, LPWSAOVERLAPPED pOverlapped, DWORD);

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

class cDirectNetClient : public iDirectNetClient 
{
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

    void IoHandlerThreadProcedure();
    void SendApc(PCHAR pcBuffer, DWORD dwLength);
    void DisconnectApc();

private:
    cDirectNetClient();
    ~cDirectNetClient();

    bool m_bIsInitialized;

    iDirectNetCryption* m_pDirectNetCryption;

    iDirectNetClientEventHandler* m_pDirectNetClientEventHandler;

    SOCKET m_sClient;

    UINT32 m_dwServerAddr;
    DNPORT m_dnServerPort;

    enum CONNECTION_STATE 
    {
        CONNECTION_NOTCONNECTED, CONNECTION_INPROGRESS, CONNECTION_ESTABLISHED
    } m_eConnectionState;

    UINT16 m_u16OutgoingSequence;
    UINT16 m_u16IncomingSequence;

    HANDLE m_hIoHandlerThread;
};

class cMyOverlapped : public WSAOVERLAPPED 
{
public:
    cDirectNetClient *m_pDirectNetClient;
    WSABUF m_WsaBuf; // this buf must delete!
    DWORD tempsize;

    cMyOverlapped(cDirectNetClient *handle, WSABUF buf) :
        m_pDirectNetClient(handle),
        m_WsaBuf(buf)
    {
        tempsize = 0;
        ZeroMemory(this, sizeof(WSAOVERLAPPED));
    }
};

class cRecvOperation : public cMyOverlapped 
{
public:
    RECEIVING_STATE m_State;

    cRecvOperation(cDirectNetClient *handle, WSABUF buf, RECEIVING_STATE state) : 
        cMyOverlapped(handle, buf),
        m_State(state)
    {
    }

    ~cRecvOperation()
    {
        if (m_WsaBuf.buf != NULL) 
            delete m_WsaBuf.buf;
    }
};

class cSendOperation : public cMyOverlapped 
{
public:
    cSendOperation(cDirectNetClient *handle, WSABUF buf) : 
        cMyOverlapped(handle, buf)
    {
    }

    ~cSendOperation()
    {
        if (m_WsaBuf.buf != NULL) 
            delete m_WsaBuf.buf;
    }
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
}

cDirectNetClient::~cDirectNetClient()
{
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

unsigned int __stdcall s_io_handler_thread_procedure(void* pv_this)
{
    reinterpret_cast<cDirectNetClient*>(pv_this)->IoHandlerThreadProcedure();

    return 0U;
}

void cDirectNetClient::IoHandlerThreadProcedure()
{
    //try 
    {
        m_sClient = socket(AF_INET, SOCK_STREAM, 0);

        if (INVALID_SOCKET == m_sClient) 
        {
            DEBUG_ONLY( _dout << "socket creation failed" << _endl );
            throw DNR_E_FAIL;
        }

        DEBUG_ONLY( _dout << "client socket (" << HEX(m_sClient) << ") created" << _endl );

        sockaddr_in sain;
        ZeroMemory( &sain, sizeof(sain) );
        sain.sin_family = AF_INET;
        sain.sin_addr.s_addr = m_dwServerAddr;
        sain.sin_port = htons(m_dnServerPort);

        if ( SOCKET_ERROR == connect( m_sClient, (sockaddr*)&sain, sizeof(sain) ) ) 
        {
            DEBUG_ONLY( _dout << "connection failed" << _endl );
            throw DNR_E_FAIL;
        }
    }

    //catch (DNRESULT dnr) 
    //{
    //    closesocket(m_sClient);
    //    m_eConnectionState = CONNECTION_NOTCONNECTED;
    //    //try
    //    {
    //        m_pDirectNetClientEventHandler->OnConnectComplete(dnr);
    //    }
    //    catch (...)
    //    {
    //        DEBUG_ONLY( _dout << "m_pDirectNetClientEventHandler->OnConnectComplete Exception" << _endl );
    //    }

    //    return;
    //}

    DEBUG_ONLY( _dout << "connection established" << _endl );

    m_eConnectionState = CONNECTION_ESTABLISHED;
    m_pDirectNetClientEventHandler->OnConnectComplete(DNR_S_OK);

    /*
    WSABUF buf;
    buf.buf = (char*)new DNPHDR;
    buf.len = sizeof(DNPHDR);
    cRecvOperation *pRecvOperation = new cRecvOperation(this, buf, RECEIVING_HEAD);

    DWORD dwNumBytes = 0, dwFlags = 0;
    int ir = WSARecv(m_sClient, &buf, 1, &dwNumBytes, &dwFlags, pRecvOperation, StaticRecvCompletionRoutine);
    if ( SOCKET_ERROR == ir && WSA_IO_PENDING != WSAGetLastError() ) 
    {
        DEBUG_ONLY( _dout << "start io th fail" << _endl );

        RecvCompletionHandler(0, pRecvOperation);
        return;
    }

    DEBUG_ONLY( _dout << "io end" << _endl );

    while ( WAIT_IO_COMPLETION == SleepEx(INFINITE, TRUE) );
    */

    char iobuf[10240];

    WSABUF buf;
    buf.buf = iobuf;
    buf.len = sizeof(DNPHDR);
    cRecvOperation RecvOperation(this, buf, RECEIVING_HEAD);

    while (m_bIsInitialized)
    {
        int len = recv(m_sClient, &RecvOperation.m_WsaBuf.buf[RecvOperation.tempsize], 
            RecvOperation.m_WsaBuf.len - RecvOperation.tempsize, 0);

        DEBUG_ONLY( _dout << "recv size " << len << _endl );

        RecvOperation.tempsize += len;

        if (len == SOCKET_ERROR)
        {
            DEBUG_ONLY( _dout << "recv thread err end " << WSAGetLastError() << _endl );
            RecvCompletionHandler(0, &RecvOperation);
            break;
        }
        else if (RecvOperation.tempsize > RecvOperation.m_WsaBuf.len)
        {
            DEBUG_ONLY( _dout << "recv thread err size" << _endl );
            RecvCompletionHandler(0, &RecvOperation);
            break;
        }
        else if (RecvOperation.tempsize < RecvOperation.m_WsaBuf.len)
        {
            DEBUG_ONLY( _dout << "recv non - size " << RecvOperation.tempsize << " / " << RecvOperation.m_WsaBuf.len << _endl );
            continue;
        }

        DEBUG_ONLY( _dout << "recv full package" << _endl );

        //try
        {
            RecvCompletionHandler(RecvOperation.tempsize, &RecvOperation);
        }
        //catch (...)
        //{
        //    DEBUG_ONLY( _dout << "ex in RecvCompletionHandler" << _endl );
        //}

        RecvOperation.tempsize = 0;
    }

    RecvOperation.m_WsaBuf.buf = NULL;
}

DNRESULT cDirectNetClient::Send(PVOID pvBuffer, DWORD dwLength)
{
    if (CONNECTION_ESTABLISHED != m_eConnectionState)
        return DNR_E_NOT_CONNECTED;

    if ( pvBuffer == NULL || dwLength == 0 || dwLength >= (1 << 16) )
        return DNR_E_INVALID_PARAMETER;

    DEBUG_ONLY( _dout << "cDirectNetClient::Send(" << pvBuffer << ", " << DEC(dwLength) << ")" << _endl );

    /*
    SEND_PARAM_STRUCT* pSendParamStruct = new SEND_PARAM_STRUCT;
    pSendParamStruct->pDirectNetClient = this;
    pSendParamStruct->pcBuffer = new char[dwLength];
    memcpy(pSendParamStruct->pcBuffer, pvBuffer, dwLength);
    pSendParamStruct->dwLength = dwLength;
    QueueUserAPC( StaticSendApc, m_hIoHandlerThread, (ULONG_PTR)pSendParamStruct );
    */

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

    int len = send(m_sClient, (const char *)pHeader, sizeof(CRYPTION), 0);
    if (len == SOCKET_ERROR)
    {
        DEBUG_ONLY( _dout << "end thread err 1 end " << WSAGetLastError() << _endl );
        return DNR_E_INVALID_PARAMETER;
    }

    delete pHeader;

    len = send(m_sClient, (const char *)pvBuffer, dwLength, 0);
    if (len == SOCKET_ERROR)
    {
        DEBUG_ONLY( _dout << "end thread err 2 end " << WSAGetLastError() << _endl );
        return DNR_E_INVALID_PARAMETER;
    }

    return DNR_S_PENDING;
}

void cDirectNetClient::SendApc(PCHAR pcBuffer, DWORD dwLength)
{
    DEBUG_ONLY( _dout << "cDirectNetClient::SendApc(): step 1" << _endl );

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

    DEBUG_ONLY( _dout << "cDirectNetClient::SendApc(): step 2" << _endl );

    WSABUF buf;
    buf.buf = (char*)pHeader;
    buf.len = sizeof(CRYPTION);
    cSendOperation *pSendOperation = new cSendOperation(this, buf);

    DWORD dwNumBytes = 0;
    int ir = WSASend(m_sClient, &buf, 1, &dwNumBytes, 0, pSendOperation, StaticSendCompletionRoutine);
    if ( SOCKET_ERROR == ir && WSA_IO_PENDING != WSAGetLastError() ) 
    {
        DEBUG_ONLY( _dout << "WSASend Head returns: " << ir << _endl );
        SendCompletionHandler(0, pSendOperation);
        return;
    }

    buf.buf = pcBuffer;
    buf.len = dwLength;
    pSendOperation = new cSendOperation(this, buf);

    dwNumBytes = 0;
    ir = WSASend(m_sClient, &buf, 1, &dwNumBytes, 0, pSendOperation, StaticSendCompletionRoutine);
    if ( SOCKET_ERROR == ir && WSA_IO_PENDING != WSAGetLastError() ) 
    {
        DEBUG_ONLY( _dout << "WSASend Body returns: " << ir << _endl );
        SendCompletionHandler(0, pSendOperation);
        return;
    }

    DEBUG_ONLY( _dout << "cDirectNetClient::SendApc(): done" << _endl );
}

void cDirectNetClient::RecvCompletionHandler(DWORD dwNumBytesTransferred, cRecvOperation* pRecvOperation)
{
    // disconnected!!!
    if (0 == dwNumBytesTransferred) 
    {
        DEBUG_ONLY( _dout << "connection is broken" << _endl );

        m_eConnectionState = CONNECTION_NOTCONNECTED;

        //try
        {
            m_pDirectNetClientEventHandler->OnTerminateSession();
        }
        //catch (...)
        //{
        //    DEBUG_ONLY( _dout << "m_pDirectNetClientEventHandler->OnTerminateSession Exception" << _endl );
        //}

        return;
    }

    // recved data not enough one package!!!
    if (dwNumBytesTransferred != pRecvOperation->m_WsaBuf.len) 
    {
        DEBUG_ONLY( _dout << "errrrrrrrrrrrr" << _endl );
        return;
    }

    // recv a full header package
    if (pRecvOperation->m_State == RECEIVING_HEAD)
    {
        DEBUG_ONLY( _dout << "recv header" << _endl );

        if (sizeof(DNPHDR) != pRecvOperation->m_WsaBuf.len)
        {
            assert( sizeof(DNPHDR) == pRecvOperation->m_WsaBuf.len );

            DEBUG_ONLY( _dout << "sizeof(DNPHDR) != pRecvOperation->m_WsaBuf.len" << _endl );
            return;
        }

        DNPHDR dnphdr = *(DNPHDR*)pRecvOperation->m_WsaBuf.buf;

        pRecvOperation->m_State = RECEIVING_BODY;
        pRecvOperation->m_WsaBuf.len = dnphdr.paclen;

        m_u16IncomingSequence = dnphdr.seqnum;

        return;
    }

    if (pRecvOperation->m_State == RECEIVING_BODY)
    {
        DEBUG_ONLY( _dout << "recv body" << _endl );

        m_pDirectNetClientEventHandler->OnReceivePacket(pRecvOperation->m_WsaBuf.buf, pRecvOperation->m_WsaBuf.len,
            m_u16IncomingSequence);

        pRecvOperation->m_State = RECEIVING_HEAD;
        pRecvOperation->m_WsaBuf.len = sizeof(DNPHDR);

        return;
    }

    return;
}

void cDirectNetClient::SendCompletionHandler(DWORD dwNumBytesTransferred, cSendOperation* pSendOperation)
{
	DEBUG_ONLY( _dout << "send completed: " << dwNumBytesTransferred << _endl );

    delete pSendOperation;
}

void cDirectNetClient::DisconnectApc()
{
    shutdown(m_sClient, SD_SEND);
    closesocket(m_sClient);

    m_eConnectionState = CONNECTION_NOTCONNECTED;
}

DNRESULT cDirectNetClient::Disconnect()
{
    //DISCONNECT_PARAM_STRUCT* pDisconnectParamStruct = new DISCONNECT_PARAM_STRUCT;
    //pDisconnectParamStruct->pDirectNetClient = this;
    //QueueUserAPC( StaticDisconnectApc, m_hIoHandlerThread, (ULONG_PTR)pDisconnectParamStruct );
    //DISCONNECT_PARAM_STRUCT* pDisconnectParamStruct = (DISCONNECT_PARAM_STRUCT*)pParamStruct;

    DisconnectApc();

    return DNR_S_OK;
}

DNRESULT cDirectNetClient::Close()
{
    if (!m_bIsInitialized)
        return DNR_E_NOT_INITIALIZED;

    Disconnect();

    DEBUG_ONLY( _dout << "client socket (" << HEX(m_sClient) << ") closed" << _endl );

    // QueueUserAPC(StaticExitApc, m_hIoHandlerThread, 0);
    m_bIsInitialized = false;

    WaitForSingleObject(m_hIoHandlerThread, INFINITE);
    CloseHandle(m_hIoHandlerThread);

    DEBUG_ONLY( _dout << "io handler thread (" << m_hIoHandlerThread << ") exited" << _endl );

    return DNR_S_OK;
}

VOID CALLBACK StaticSendCompletionRoutine(DWORD, DWORD dwNumBytes, LPWSAOVERLAPPED pOverlapped, DWORD)
{
    DEBUG_ONLY( _dout << "cDirectNetClient::StaticSendCompletionRoutine" << _endl );

    static_cast<cSendOperation*>(pOverlapped)->m_pDirectNetClient->SendCompletionHandler
        ( dwNumBytes, static_cast<cSendOperation*>(pOverlapped) );
}

VOID CALLBACK StaticRecvCompletionRoutine(DWORD, DWORD dwNumBytes, LPWSAOVERLAPPED pOverlapped, DWORD)
{
    DEBUG_ONLY( _dout << "cDirectNetClient::StaticRecvCompletionRoutine" << _endl );

    //try
    {
        static_cast<cRecvOperation*>(pOverlapped)->m_pDirectNetClient->RecvCompletionHandler
            ( dwNumBytes, static_cast<cRecvOperation*>(pOverlapped) );
    }
    //catch (...)
    //{
    //    DEBUG_ONLY( _dout << "cDirectNetClient::StaticRecvCompletionRoutine Exception" << _endl );
    //}
}

