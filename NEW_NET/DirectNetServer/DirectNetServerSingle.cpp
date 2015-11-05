#include "stdafx.h"

// 简单版多线程TCP服务端
// accept_thread        -> oncreatepalyer
// receive_thread       -> ondeleteplayer onreceivepacket
// send_thread          -> non-function

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

unsigned int __stdcall s_accept_thread_procedure(void*);
unsigned int __stdcall s_receive_thread_procedure(void*);
unsigned int __stdcall s_send_thread_procedure(void*);

class cPlayerInfo
{
public:
    cPlayerInfo(SOCKET s, sockaddr_in &addr) :
        m_qwDnid(dnid_gen()),
        m_Socket(s),
        m_AddrInfo(addr),
        m_Context(NULL),
        m_u16OutgoingSequence(0),
        m_u16IncomingSequence(0),
        m_State(RECEIVING_HEAD),
        m_pBuffer((char *)new CRYPTION),
        m_dwBufferSize(sizeof(CRYPTION)),
        m_dwTempSize(0)
    {
    }

    ~cPlayerInfo() 
    {
        if (m_Socket != INVALID_SOCKET)
        {
            shutdown(m_Socket, SD_SEND);
            closesocket(m_Socket);
        }

        if (m_pBuffer != NULL)
            delete m_pBuffer;
    }

public:
    RECEIVING_STATE m_State;
    sockaddr_in     m_AddrInfo;

    DNID    m_qwDnid;
    SOCKET  m_Socket;
    PVOID   m_Context;

    UINT16  m_u16OutgoingSequence;
    UINT16  m_u16IncomingSequence;

    char    *m_pBuffer;
    DWORD   m_dwBufferSize;
    DWORD   m_dwTempSize;
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

public:
    unsigned int AcceptThreadProcedure();
    unsigned int ReceiveThreadProcedure();
    unsigned int SendThreadProcedure();

public:
    DNRESULT CreatePlayer(SOCKET s, sockaddr_in &addr);

    DNRESULT ReceiveData(cPlayerInfo *pPlayerInfo);

private:
    cDirectNetServer();
    ~cDirectNetServer();

private:
    bool m_bIsInitialized;

    DWORD m_dwMaxNumPlayers;

    SOCKET m_sServer;

    HANDLE m_hAcceptThread;
    HANDLE m_hReceiveThread;
    HANDLE m_hSendThread;

    iDirectNetCryption* m_pDirectNetCryption;

    iDirectNetServerEventHandler* m_pDirectNetServerEventHandler;

    CRITICAL_SECTION m_csLock;

    std::map<DNID, cPlayerInfo*> m_PlayerInfoMap;
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

    m_bIsInitialized = false;

    m_pDirectNetCryption = NULL;

    m_pDirectNetServerEventHandler = NULL;

    m_dwMaxNumPlayers = 0;

    m_sServer = INVALID_SOCKET;

    m_hAcceptThread = INVALID_HANDLE_VALUE;
    m_hReceiveThread = INVALID_HANDLE_VALUE;
    m_hSendThread = INVALID_HANDLE_VALUE;

    InitializeCriticalSection(&m_csLock);
}

cDirectNetServer::~cDirectNetServer()
{
    DeleteCriticalSection(&m_csLock);

    WSACleanup();
}

DNRESULT cDirectNetServer::Initialize(iDirectNetCryption* pDirectCryption, iDirectNetServerEventHandler* pDirectEventHandler, DWORD dwMaxNumPlayers)
{
    if (m_bIsInitialized)
        return DNR_S_OK;

    DEBUG_ONLY( _dout << "cDirectNetServer::Initialize(" << pDirectEventHandler << ", " << dwMaxNumPlayers << ")" << _endl );

    m_pDirectNetCryption = pDirectCryption;

    m_pDirectNetServerEventHandler = pDirectEventHandler;

    m_dwMaxNumPlayers = dwMaxNumPlayers;

    m_bIsInitialized = true;

    return DNR_S_OK;
}

DNRESULT cDirectNetServer::Host(DNPORT dnPort)
{
    if (!m_bIsInitialized)
        return DNR_E_NOT_INITIALIZED;

    DEBUG_ONLY( _dout << "cDirectNetServer::Host(" << DEC(dnPort) << ")" << _endl );

    DNRESULT dnr = DNR_S_OK;

    try 
    {
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
        if ( SOCKET_ERROR == bind( m_sServer, (sockaddr*)&sain, sizeof(sain) ) ) 
        {
            DEBUG_ONLY( _dout << "bind failed on port: " << DEC(dnPort) << _endl );
            throw DNR_E_FAIL;
        }

        if ( SOCKET_ERROR == listen(m_sServer, SOMAXCONN) )
        {
            int i = WSAGetLastError();
            throw DNR_E_FAIL;
        }

        // create a dedicated accepting thread for this instance,
        // pass the 'this' pointer as the context
        m_hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, s_accept_thread_procedure, this, 0, NULL);
        m_hReceiveThread = (HANDLE)_beginthreadex(NULL, 0, s_receive_thread_procedure, this, 0, NULL);
        m_hSendThread = (HANDLE)_beginthreadex(NULL, 0, s_send_thread_procedure, this, 0, NULL);

        DEBUG_ONLY( _dout << "accept thread (" << m_hAcceptThread << ") created" << _endl );

        m_bIsInitialized = true;
    }
    catch (DNRESULT _dnr) 
    {
        DEBUG_ONLY( _dout << "catched exception in init function" << _endl );

        dnr = _dnr;
        closesocket(m_sServer);
    }

    return dnr;
}

DNRESULT cDirectNetServer::Close()
{
    m_bIsInitialized = false;

    closesocket(m_sServer);

    EnterCriticalSection(&m_csLock);

    std::map<DNID, cPlayerInfo*>::iterator it = m_PlayerInfoMap.begin();

    while (it != m_PlayerInfoMap.end())
    {
        delete it->second;
        it++;
    }

    m_PlayerInfoMap.clear();

    LeaveCriticalSection(&m_csLock);

    return DNR_S_OK;
}

DNRESULT cDirectNetServer::DeletePlayer(DNID dnidPlayer)
{
    if ((dnidPlayer == -1) || (dnidPlayer == 0))
        return DNR_S_OK;

    EnterCriticalSection(&m_csLock);

    std::map<DNID, cPlayerInfo*>::iterator it = m_PlayerInfoMap.find(dnidPlayer);

    if (it != m_PlayerInfoMap.end())
    {
        try
        {
            cPlayerInfo *pPlayerInfo = it->second;

            m_pDirectNetServerEventHandler->OnDeletePlayer(pPlayerInfo->m_qwDnid, pPlayerInfo->m_Context);

            delete it->second;
        }
        catch (...)
        {
            DEBUG_ONLY( _dout << "ex in ondeleteplayer" << _endl );
        }

        m_PlayerInfoMap.erase(it);
    }

    LeaveCriticalSection(&m_csLock);

    return DNR_S_OK;
}

DNRESULT cDirectNetServer::SendTo(DNID dnidPlayer, PVOID pvBuffer, DWORD dwLength)
{
    EnterCriticalSection(&m_csLock);

    DNRESULT ret = DNR_E_FAIL;

    try
    {
        std::map<DNID, cPlayerInfo*>::iterator it = m_PlayerInfoMap.find(dnidPlayer);

        if (it != m_PlayerInfoMap.end())
        {
            cPlayerInfo *pPlayerInfo = it->second;

            /*
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
                int ir = WSASend(pPlayerInfo->s, pSendOperation->wsab, 2, &dwNumBytes, 0, pSendOperation, CompletionRoutine);
                if ( SOCKET_ERROR == ir && WSA_IO_PENDING != WSAGetLastError() ) {
                    DEBUG_ONLY( _dout << "WSASend() failed: " << DEC( WSAGetLastError() ) << _endl );

                    IoCompletionHandler(0, pSendOperation);
                }
            */

            char *pBuffer = new char[sizeof(DNPHDR) + dwLength];

            DNPHDR *pHeader = (DNPHDR *)pBuffer;

            pHeader->seqnum = pPlayerInfo->m_u16OutgoingSequence++;
            pHeader->paclen = (UINT16)dwLength;

            memcpy(pBuffer + sizeof(DNPHDR), pvBuffer, dwLength);

            int len = send(pPlayerInfo->m_Socket, pBuffer, sizeof(DNPHDR) + dwLength, 0);

            if (len == sizeof(DNPHDR) + dwLength)
                ret = DNR_S_OK;

            delete pBuffer;
        }
    }
    catch (...)
    {
        DEBUG_ONLY( _dout << "ex in send" << _endl );
    }

    LeaveCriticalSection(&m_csLock);

    return ret;
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

        *addr = iter->second->m_AddrInfo;
    }
    catch (...)
    {
        DEBUG_ONLY( _dout << "ex in get player info" << _endl );
    }

    LeaveCriticalSection(&m_csLock);

    return DNR_S_OK;
}

DNRESULT cDirectNetServer::CreatePlayer(SOCKET s, sockaddr_in &addr)
{
    if (s == INVALID_SOCKET)
        return DNR_E_FAIL;

    EnterCriticalSection(&m_csLock);

    try
    {
        if (m_PlayerInfoMap.size() >= m_dwMaxNumPlayers)
        {
            LeaveCriticalSection(&m_csLock);

            return DNR_E_FAIL;
        }

        cPlayerInfo *pPlayerInfo = new cPlayerInfo(s, addr);

        m_PlayerInfoMap[pPlayerInfo->m_qwDnid] = pPlayerInfo;

        try
        {
            m_pDirectNetServerEventHandler->OnCreatePlayer(pPlayerInfo->m_qwDnid, &pPlayerInfo->m_Context);
        }
        catch (...)
        {
            DEBUG_ONLY( _dout << "ex in oncreateplayer" << _endl );

            m_PlayerInfoMap.erase(pPlayerInfo->m_qwDnid);

            delete pPlayerInfo;
        }
    }
    catch (...)
    {
        DEBUG_ONLY( _dout << "ex in createplayer" << _endl );
    }

    LeaveCriticalSection(&m_csLock);

    return DNR_S_OK;
}

DNRESULT cDirectNetServer::ReceiveData(cPlayerInfo *pPlayerInfo)
{
    if (pPlayerInfo == NULL)
        return DNR_E_FAIL;

    int len = recv(pPlayerInfo->m_Socket, &pPlayerInfo->m_pBuffer[pPlayerInfo->m_dwTempSize], 
        pPlayerInfo->m_dwBufferSize - pPlayerInfo->m_dwTempSize, 0);

    pPlayerInfo->m_dwTempSize += len;

    if (len == SOCKET_ERROR)
    {
        return DNR_E_FAIL;
    }
    
    if (len == 0)
    {
        return DNR_E_FAIL;
    }
    
    if (pPlayerInfo->m_dwTempSize > pPlayerInfo->m_dwBufferSize)
    {
        return DNR_E_FAIL;
    }
    
    if (pPlayerInfo->m_dwTempSize < pPlayerInfo->m_dwBufferSize)
    {
        return DNR_S_OK;
    }

    // recv a full header package
    if (pPlayerInfo->m_State == RECEIVING_HEAD)
    {
        if (sizeof(CRYPTION) != pPlayerInfo->m_dwBufferSize)
            return DNR_E_FAIL;

        CRYPTION cryption = *(CRYPTION*)pPlayerInfo->m_pBuffer;

        // check the validity of the encrypted header
        m_pDirectNetCryption->DES_decrypt( &cryption, sizeof(cryption) );
        UINT32 crc32 = m_pDirectNetCryption->CRC32_compute
            ( (char*)&cryption + sizeof(cryption.crc32), sizeof(cryption) - sizeof(cryption.crc32) );

        if (crc32 != cryption.crc32) 
            return DNR_E_FAIL;

        if (cryption.dnphdr.paclen > 102400)
            return DNR_E_FAIL;

        pPlayerInfo->m_State = RECEIVING_BODY;
        pPlayerInfo->m_dwBufferSize = cryption.dnphdr.paclen;
        pPlayerInfo->m_u16IncomingSequence = cryption.dnphdr.seqnum;
        pPlayerInfo->m_dwTempSize = 0;

        delete pPlayerInfo->m_pBuffer;
        pPlayerInfo->m_pBuffer = new char [pPlayerInfo->m_dwBufferSize];

        return DNR_S_OK;
    }

    if (pPlayerInfo->m_State == RECEIVING_BODY)
    {
        try
        {
            m_pDirectNetServerEventHandler->OnReceivePacket(pPlayerInfo->m_qwDnid, pPlayerInfo->m_Context, 
                pPlayerInfo->m_pBuffer, pPlayerInfo->m_dwBufferSize, pPlayerInfo->m_u16IncomingSequence);
        }
        catch (int)
        {
            DEBUG_ONLY( _dout << "delete in onrecv" << _endl );

            return DNR_E_FAIL;
        }
        catch (...)
        {
            DEBUG_ONLY( _dout << "ex in onrecv" << _endl );

            return DNR_E_FAIL;
        }

        pPlayerInfo->m_State = RECEIVING_HEAD;
        pPlayerInfo->m_dwBufferSize = sizeof(CRYPTION);
        pPlayerInfo->m_dwTempSize = 0;

        delete pPlayerInfo->m_pBuffer;
        pPlayerInfo->m_pBuffer = new char [pPlayerInfo->m_dwBufferSize];

        return DNR_S_OK;
    }

    return DNR_E_FAIL;
}

unsigned int cDirectNetServer::AcceptThreadProcedure()
{
    sockaddr_in addr;
    int addr_size = 0;
    while (m_bIsInitialized)
    {
        addr_size = sizeof(sockaddr_in);
        ZeroMemory(&addr, addr_size);
        SOCKET s = ::accept(m_sServer, (sockaddr *)&addr, &addr_size);

        CreatePlayer(s, addr);

        Sleep(1);
    }

    return 1;
}

unsigned int cDirectNetServer::ReceiveThreadProcedure()
{
    while (m_bIsInitialized)
    {
        EnterCriticalSection(&m_csLock);

        std::list<DNID> removelist;

        try
        {
            if (m_PlayerInfoMap.size())
            {
                DWORD number = 0;
                fd_set fd_read, fd_write, fd_error;
                cPlayerInfo *fd_array[FD_SETSIZE];

		        FD_ZERO(&fd_read);
		        FD_ZERO(&fd_write);
		        FD_ZERO(&fd_error);
                ZeroMemory(fd_array, sizeof(fd_array));

                std::map<DNID, cPlayerInfo*>::iterator it = m_PlayerInfoMap.begin();
                while (it != m_PlayerInfoMap.end())
                {
                    cPlayerInfo *pPlayerInfo = it->second;

                    if (pPlayerInfo != NULL)
                    {
                        FD_SET(pPlayerInfo->m_Socket, &fd_read);
                        // FD_SET(pPlayerInfo->m_Socket, &fd_write);
                        // FD_SET(pPlayerInfo->m_Socket, &fd_error);
                        fd_array[number++] = pPlayerInfo;
                    }

                    it++;

                    if (number == FD_SETSIZE || it == m_PlayerInfoMap.end())
                    {
                        try
                        {
		                    timeval tv = {0, 1};
		                    // select(NULL, &fd_read, &fd_write, &fd_error, &tv);
		                    select(NULL, &fd_read, NULL, NULL, &tv);

                            for (DWORD i=0; i<number; i++)
                            {
                                if (!FD_ISSET(fd_array[i]->m_Socket, &fd_read))
                                    continue;

                                DNID dnid = 0;
                                try
                                {
                                    dnid = fd_array[i]->m_qwDnid;
                                    if (ReceiveData(fd_array[i]) != DNR_S_OK)
                                        removelist.push_back(dnid);
                                }
                                catch (...)
                                {
                                    removelist.push_back(dnid);

                                    DEBUG_ONLY( _dout << "ex in recv" << _endl );
                                }
                            }
                        }
                        catch (...)
                        {
                            DEBUG_ONLY( _dout << "ex in select" << _endl );
                        }

                        number = 0;
		                FD_ZERO(&fd_read);
		                FD_ZERO(&fd_write);
		                FD_ZERO(&fd_error);
                        ZeroMemory(fd_array, sizeof(fd_array));
                    }
                }
            }
        }
        catch (...)
        {
            DEBUG_ONLY( _dout << "ex in select loop" << _endl );
        }

        LeaveCriticalSection(&m_csLock);

        while (removelist.size())
        {
            DeletePlayer(removelist.front());

            removelist.pop_front();
        }

        Sleep(1);
    }

    return 1;
}

unsigned int cDirectNetServer::SendThreadProcedure()
{
    while (m_bIsInitialized)
    {
        Sleep(1);
    }

    return 1;
}

unsigned int __stdcall s_accept_thread_procedure(void *pHandle)
{
    cDirectNetServer *pDirectNetServer = (cDirectNetServer *)pHandle;
    return pDirectNetServer->AcceptThreadProcedure();
}

unsigned int __stdcall s_receive_thread_procedure(void *pHandle)
{
    cDirectNetServer *pDirectNetServer = (cDirectNetServer *)pHandle;
    return pDirectNetServer->ReceiveThreadProcedure();
}

unsigned int __stdcall s_send_thread_procedure(void *pHandle)
{
    cDirectNetServer *pDirectNetServer = (cDirectNetServer *)pHandle;
    return pDirectNetServer->SendThreadProcedure();
}
