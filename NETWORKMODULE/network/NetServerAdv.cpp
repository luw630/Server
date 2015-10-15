//---------------------------------------------------------------------------
//  Engine (c) 1999-2000 by Dreamwork
//
// File:	CNetServerAdv.cpp
// Date:	2003.07.08
// Code:	Dreamwork
// Desc:	Simple double linked node class
//---------------------------------------------------------------------------

#include "NetServerAdv.h"

#define USE_CRC
#include "desdec.h"

#pragma comment (lib, "DirectNetServer.lib")

extern BOOL rfalse(char, char, LPCSTR, ...);
extern void TraceMemory(LPVOID pDumpEntry, int iOffset, unsigned int uiSize, LPCSTR szFilename);

//---------------------------------------------------------------------------
#define N_SOCKET_MIN_PORTNUM    (7000)
#define N_SOCKET_MAX_PORTNUM    (7999)
#define N_SOCKET_PORTRANGE      (N_SOCKET_MAX_PORTNUM - N_SOCKET_MIN_PORTNUM)

#define TIMEOUTTIME             (100000)
#define PINGMARGIN              (15000)

cDirectNetDecryption _cryption2;

#define CHECKINFO "this is guard array in class CNetServerExAdv for check overflow"

extern void _set_server_ga_info(char _ga[64]);

// comput hash value, same as g_Hash(char *) in enginelib.lib
static DWORD ComputHashValue(LPCSTR szStr)
{
    BYTE const * pStr = (BYTE const *)szStr;
    unsigned long i = 0, j = 1;
    while (*pStr != 0) i += (*pStr++) * j++;
    return i % 256;
}

//---------------------------------------------------------------------------
// 函数:	CNetServerAdv
// 功能:	构造
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
CNetServerAdv::CNetServerAdv(BOOL bUseVerify) :
    m_bUseVerify(bUseVerify)
{
    InitializeCriticalSection(&m_ClientMapLock);

    InitializeCriticalSection(&m_PacketQueueLock);

    m_pDirectNetServer = iDirectNetServer::CreateInstance();

    m_pDefaultContext = NULL;

    m_dwPrevCheckTime = timeGetTime();

    _set_server_ga_info(CHECKINFO);

    m_dwInSize = 0;
    m_dwOutSize = 0;
    m_dwInNumber = 0;
    m_dwOutNumber = 0;
}

//---------------------------------------------------------------------------
// 函数:	~CNetServerAdv
// 功能:	分构
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
CNetServerAdv::~CNetServerAdv(void)
{
	Close();

    m_pDefaultContext = NULL;

    m_pDirectNetServer->Release();

    DeleteCriticalSection(&m_PacketQueueLock);

    DeleteCriticalSection(&m_ClientMapLock);
}

//---------------------------------------------------------------------------
// 函数:	Create
// 功能:	构造
// 参数:	char* 
// 返回:	void
//---------------------------------------------------------------------------
BOOL CNetServerAdv::Create(const char *szPortName)
{
    if (szPortName == NULL)
        return FALSE;

    // use 'this' as context for the message handler
    DNRESULT dnr = m_pDirectNetServer->Initialize(&_cryption2, this, MAX_CLIENT_NUMBER);

    if (DNR_FAILED(dnr))
        return FALSE;

    // this is needed for extracting the port number from a string
    WORD wPort = static_cast<WORD>( ComputHashValue(szPortName) + N_SOCKET_MIN_PORTNUM );

    dnr = m_pDirectNetServer->Host(wPort);

    // ok, the DreamNetServer is now hosting ...
	
	return !DNR_FAILED(dnr);
}

//---------------------------------------------------------------------------
// 函数:	Close
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
BOOL CNetServerAdv::Close(void)
{
    DNRESULT dnr = m_pDirectNetServer->Close();

    __try 
    {
        EnterCriticalSection(&m_ClientMapLock);

        m_ClientMap.clear();
    }
    __finally 
    {
        LeaveCriticalSection(&m_ClientMapLock);
    }

    __try 
    {
        EnterCriticalSection(&m_PacketQueueLock);

        while (!m_PacketQueue.empty())
        {
            delete m_PacketQueue.front()->m_pvBuffer;
            delete m_PacketQueue.front();

            m_PacketQueue.pop();
        }
    }
    __finally 
    {
        LeaveCriticalSection(&m_PacketQueueLock);
    }

    return DNR_FAILED(dnr);
}

//---------------------------------------------------------------------------
// 函数:	SendMessage
// 功能:	向某一客户端发数据包
// 参数:	int nClientIndex
// 返回:	int
//---------------------------------------------------------------------------
BOOL CNetServerAdv::SendMessage(DNID dnidClient, const void *pvBuf, WORD wSize)
{
    if (dnidClient == 0 || pvBuf == NULL || wSize == 0)
        return FALSE;

    DNRESULT dnr = DNR_E_FAIL;

    __try
    {
        dnr = m_pDirectNetServer->SendTo(dnidClient, const_cast<PVOID>(pvBuf), wSize);

        // 更新统计数据
        m_dwOutSize += wSize + 4; // 后面的4是封包头的数据
        m_dwOutNumber ++;
    }
    __finally
    {
    }

    return !DNR_FAILED(dnr);
}

BOOL CNetServerAdv::SendDirect(DNID dnidClient, LPVOID pPackage, WORD wSize)
{
    if (dnidClient == 0 || pPackage == NULL || wSize == 0)
        return FALSE;

    DNRESULT dnr = DNR_E_FAIL;

    DWORD optopen[2] = {2, 1};
    if (SetOption(optopen))
    {
        dnr = m_pDirectNetServer->SendTo(dnidClient, pPackage, wSize);

        DWORD optclose[2] = {2, 0};
        SetOption(optclose);
    }
    else
    {
        // 旧的directnetserver不支持setoption，所以需要先解码，然后再一个个发送
        // 暂时不允许！！！
        MessageBox(0, "directnetserver.dll 版本太低！！！", 0, 0);
    }

    return !DNR_FAILED(dnr);
}

//---------------------------------------------------------------------------
// 函数:	GetMessage
// 功能:	接收某一客户端的数据包
// 参数:	int nClientIndex
// 返回:	const PACKET &
// 说明:    
//---------------------------------------------------------------------------
BOOL CNetServerAdv::GetMessage(SNetServerPacketAdv **ppPacket)
{
    __try
    {
        EnterCriticalSection(&m_PacketQueueLock);

        if (m_PacketQueue.size() == 0) 
            return FALSE;

        *ppPacket = m_PacketQueue.front();
        m_PacketQueue.pop();
    }
    __finally
    {
        LeaveCriticalSection(&m_PacketQueueLock);
    }

    return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	DelOneClient
// 功能:	断开一个客户端连接
// 参数:	nClientIndex
// 返回:	void
// 2004-1-25 添加互斥锁
//---------------------------------------------------------------------------
BOOL CNetServerAdv::DelOneClient(DNID dnidClient)
{
    if (dnidClient == 0)
        return FALSE;

    __try
    {
        // the rest will be handled inside DN_MSGID_DELETE_PLAYER handler

        // rfalse(2, 1, "因为某种原因删除角色:%x", dnidClient);
        m_pDirectNetServer->DeletePlayer(dnidClient);
    }
    __finally
    {
    }

    return TRUE;
}

//---------------------------------------------------------------------------
void CNetServerAdv::OnCreatePlayer(DNID dnidClient, PVOID *ppvPlayerContext)
{
    if (dnidClient == 0 || ppvPlayerContext == NULL)
        return;

    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        *ppvPlayerContext = NewLinkData(dnidClient);
    }
    __finally
    {
        LeaveCriticalSection(&m_ClientMapLock);
    }
}

//---------------------------------------------------------------------------
void CNetServerAdv::OnReceivePacket(DNID dnidClient, PVOID pvPlayerContext, PVOID pvBuffer, DWORD dwLength, DWORD dwSequence)
{
    if (dnidClient == 0 || pvPlayerContext == NULL || pvBuffer == NULL || dwLength == 0 || dwLength > 0xffff)
        return;

    WORD wLength = static_cast<WORD>(dwLength);

    CLinkDataAdv *pLinkData = static_cast<CLinkDataAdv *>(pvPlayerContext);

    LINKSTATUS enumStatus = LinkStatus_UnConnected;
    LPVOID pContext = NULL;

    if (m_bUseVerify)
    {
        if (wLength < 12)
        {
            // 无效的数据包长度，可能进行了封包修改，直接删除连接！
            DelOneClient(dnidClient);
            return;
        }

        DWORD qwHeader[2] = {0, 0};
        memcpy(qwHeader, pvBuffer, 8);

        _cryption2.DES_decrypt(qwHeader, 8);

        DWORD Key32 = qwHeader[1];
        DWORD Crc32 = qwHeader[0] ^ Key32;

        // 这里操作了m_ClientMap的数据，所以需要加锁！
        __try
        {
            EnterCriticalSection(&m_ClientMapLock);

            // 更新统计数据
            m_dwInSize += dwLength;
            m_dwInNumber ++;

            enumStatus = pLinkData->GetLinkStatus();
            pContext = pLinkData->GetContext();

            // 可能是第一次连接游戏服务器
            if (dwSequence == 0)
            {
                // 记录异或值
                pLinkData->SetXorCipher(Key32);
            }
            // 已经连接过服务器了，通过Key32校验数据
            else if (_get_dval(pLinkData->GetXorCipher()) == Key32)
            {
                // 更新异或值
                pLinkData->SetXorCipher(Key32);
            }
            else
            {
                // 校验失败，可能进行了封包修改，直接删除连接！

                DelOneClient(dnidClient);
                return;
            }
        }
        __finally
        {
            LeaveCriticalSection(&m_ClientMapLock);
        }

        LPDWORD pXorPtr = static_cast<LPDWORD>(pvBuffer) + 2;
        DWORD count = (wLength >> 2) - 2;

        DWORD dwAccKey = Key32;

        while (count > 0)
        {
            *pXorPtr ^= dwAccKey;
            pXorPtr ++;
            count --;
            dwAccKey += 0xcdcd;
        }

        // 校验Crc32
        if (Crc32 != _cryption2.CRC32_compute(static_cast<LPBYTE>(pvBuffer) + 8, wLength - 8))
        {
            // 无效的数据包Crc数据，可能进行了封包修改，直接删除连接！
            DelOneClient(dnidClient);
            return;
        }

        pvBuffer = static_cast<LPBYTE>(pvBuffer) + 8;
        wLength -= 8;
    }
    else
    {
        __try
        {
            EnterCriticalSection(&m_ClientMapLock);

            enumStatus = pLinkData->GetLinkStatus();
            pContext = pLinkData->GetContext();
        }
        __finally
        {
            LeaveCriticalSection(&m_ClientMapLock);
        }
    }

    if (wLength == 8)
    {
        // 及时响应ping消息
        if (static_cast<const LPDWORD>(pvBuffer)[0] == 
            static_cast<const LPDWORD>(static_cast<void *>("ping"))[0])
        {
            __try
            {
                EnterCriticalSection(&m_ClientMapLock);
                pLinkData->SetTimeOutTime(timeGetTime());
            }
            __finally
            {
                LeaveCriticalSection(&m_ClientMapLock);
            }

            DWORD pong[2] = {
                static_cast<const LPDWORD>(static_cast<void *>("pong"))[0],
                static_cast<const LPDWORD>(pvBuffer)[1]
            };
            SendMessage(dnidClient, pong, 8);
            return;
        }
    }

    PushPacket(dnidClient, enumStatus, pvBuffer, wLength, pContext);
}

//---------------------------------------------------------------------------
void CNetServerAdv::OnDeletePlayer(DNID dnidClient, PVOID pvPlayerContext)
{
    if (dnidClient == 0 || pvPlayerContext == NULL)
        return;

    CLinkDataAdv *pLinkData = static_cast<CLinkDataAdv *>(pvPlayerContext);

    LINKSTATUS enumStatus = LinkStatus_UnConnected;
    LPVOID pContext = NULL;

    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        enumStatus = pLinkData->GetLinkStatus();
        pContext = pLinkData->GetContext();

        m_ClientMap.erase(dnidClient);

        if (enumStatus == LinkStatus_Connected)
            enumStatus = LinkStatus_Disconnected;
        else
            enumStatus = LinkStatus_UnConnected;
    }
    __finally
    {
        LeaveCriticalSection(&m_ClientMapLock);
    }

    PushPacket(dnidClient, enumStatus, NULL, 0, pContext);
}

//---------------------------------------------------------------------------
BOOL CNetServerAdv::IsValidDnid(DNID dnidClient)
{
    if (dnidClient == 0)
        return FALSE;

    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        if (m_ClientMap.find(dnidClient) != m_ClientMap.end())
            return TRUE;
    }
    __finally
    {
        LeaveCriticalSection(&m_ClientMapLock);
    }

    return FALSE;
}

BOOL CNetServerAdv::GetDnidAddr(DNID dnidClient, sockaddr_in *pAddr, size_t size)
{
    if (size != sizeof(sockaddr_in))
        return FALSE;

    if (!IsValidDnid(dnidClient))
        return FALSE;

    iDirectNetServer::CONNECTIONINFO cinfo;

    DNRESULT dnr = m_pDirectNetServer->GetPlayerConnectionInfo(dnidClient, &cinfo);

    if (DNR_FAILED(dnr))
        return FALSE;

    pAddr->sin_port = cinfo.port;
    pAddr->sin_addr.S_un.S_addr = cinfo.addr;

    return TRUE;
}

LINKSTATUS CNetServerAdv::GetLinkStatus(DNID dnidClient)
{
    if (dnidClient == 0)
        return LinkStatus_UnConnected;

    LINKSTATUS ret = LinkStatus_UnConnected;

    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        std::map<DNID, CLinkDataAdv>::iterator it = m_ClientMap.find(dnidClient);

        if (it != m_ClientMap.end())
            ret = it->second.GetLinkStatus();
    }
    __finally
    {
        LeaveCriticalSection(&m_ClientMapLock);
    }

    return ret;
}

BOOL CNetServerAdv::SetLinkStatus(DNID dnidClient, LINKSTATUS enumStatus)
{
    if (dnidClient == 0)
        return FALSE;

    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        std::map<DNID, CLinkDataAdv>::iterator it = m_ClientMap.find(dnidClient);

        if (it != m_ClientMap.end())
        {
            it->second.SetLinkStatus(enumStatus);
            return TRUE;
        }
    }
    __finally
    {
        LeaveCriticalSection(&m_ClientMapLock);
    }

    return FALSE;
}

BOOL CNetServerAdv::SetLinkForceTimeOutTime(DNID dnidClient, DWORD dwTime)
{
    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        std::map<DNID, CLinkDataAdv>::iterator it = m_ClientMap.find(dnidClient);

        if (it != m_ClientMap.end())
        {
            it->second.SetForceTimeOutTime(timeGetTime() + dwTime);
            return TRUE;
        }
    }
    __finally
    {
        LeaveCriticalSection(&m_ClientMapLock);
    }

    return FALSE;
}

BOOL CNetServerAdv::ClearLinkForceTimeOutTime(DNID dnidClient)
{
    return SetLinkForceTimeOutTime(dnidClient, 0xffffffff);
}

LPVOID CNetServerAdv::GetDefaultContext()
{
    return m_pDefaultContext;
}

BOOL CNetServerAdv::SetDefaultContext(LPVOID pContext)
{
    m_pDefaultContext = pContext;

    return TRUE;
}

LPVOID CNetServerAdv::GetLinkContext(DNID dnidClient)
{
    if (dnidClient == 0)
        return NULL;

    LPVOID ret = NULL;

    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        std::map<DNID, CLinkDataAdv>::iterator it = m_ClientMap.find(dnidClient);

        if (it != m_ClientMap.end())
            ret = it->second.GetContext();
    }
    __finally
    {
        LeaveCriticalSection(&m_ClientMapLock);
    }

    return ret;
}

BOOL CNetServerAdv::SetLinkContext(DNID dnidClient, LPVOID pContext)
{
    if (dnidClient == 0)
        return NULL;

    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        std::map<DNID, CLinkDataAdv>::iterator it = m_ClientMap.find(dnidClient);

        if (it != m_ClientMap.end())
            it->second.SetContext(pContext);
    }
    __finally
    {
        LeaveCriticalSection(&m_ClientMapLock);
    }

    return TRUE;
}

DWORD CNetServerAdv::GetClientNum()
{
    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        return static_cast<DWORD>(m_ClientMap.size());
    }
    __finally
    {
        LeaveCriticalSection(&m_ClientMapLock);
    }
}

BOOL CNetServerAdv::SetOption(LPDWORD pParam)
{
    return m_pDirectNetServer->GetPlayerConnectionInfo(0xff00ff00ff00ff00, 
        (iDirectNetServer::CONNECTIONINFO *)pParam) == DNR_S_OK;
}

DWORD CNetServerAdv::GetClientNum_Dll()
{
    iDirectNetServer::CONNECTIONINFO cinfo;

    DNRESULT dnr = m_pDirectNetServer->GetPlayerConnectionInfo(-1, &cinfo);

    if (DNR_FAILED(dnr))
        return -1;

    return cinfo.addr;
}

void CNetServerAdv::CheckAlive()
{
    DWORD dwCurrentTime = timeGetTime();

    if ((int)(dwCurrentTime - m_dwPrevCheckTime) < PINGMARGIN/10)
        return;

    // 以这个时间段作为Dump流量的采样单位

    m_dwPrevCheckTime = dwCurrentTime;

    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        std::map<DNID, CLinkDataAdv>::iterator it = m_ClientMap.begin();

        while (it != m_ClientMap.end())
        {
            CLinkDataAdv *pLinkData = &it->second;
            it++;

            if ((int)(dwCurrentTime - pLinkData->GetTimeOutTime()) > TIMEOUTTIME) 
                DelOneClient(pLinkData->GetDnidClient());

            if (pLinkData->GetForceTimeOutTime() != 0)
                if ((int)(dwCurrentTime - pLinkData->GetForceTimeOutTime()) > 0) 
                    DelOneClient(pLinkData->GetDnidClient());
        }
    }
    __finally
    {
        LeaveCriticalSection(&m_ClientMapLock);
    }
}

DWORD CNetServerAdv::GetPacketNumber()
{
    return (DWORD)m_PacketQueue.size();
}

BOOL CNetServerAdv::PushPacket(DNID dnidClient, LINKSTATUS enumStatus, LPVOID pvBuffer, WORD wLength, LPVOID pContext)
{
    __try
    {
        if( wLength>60000 ) return  FALSE;

        EnterCriticalSection(&m_PacketQueueLock);

        SNetServerPacketAdv *pPacket = new SNetServerPacketAdv;

        if (wLength == 0)
        {
            pPacket->m_pvBuffer = NULL;
        }
        else
        {
            pPacket->m_pvBuffer = new char [((DWORD)wLength) + 512];
            memcpy(pPacket->m_pvBuffer, pvBuffer, wLength);
        }

        pPacket->m_wLength = wLength;
        pPacket->m_dnidClient = dnidClient;
        pPacket->m_enumStatus = enumStatus;
        pPacket->m_pContext = pContext;

        static bool cc = false;

        if (cc)
        {
            delete pPacket->m_pvBuffer;
            delete pPacket;

            return TRUE;
        }

        m_PacketQueue.push(pPacket);

        extern DWORD sinsize;
        sinsize += (((DWORD)wLength) + 512);
    }
    __finally
    {
        LeaveCriticalSection(&m_PacketQueueLock);
    }

    return TRUE;
}

CLinkDataAdv *CNetServerAdv::NewLinkData(DNID dnidClient)
{
    m_ClientMap[dnidClient] = CLinkDataAdv(dnidClient, m_pDefaultContext);

#ifdef LOG_IP
    sockaddr_in addr;
    GetDnidAddr(dnidClient, &addr, sizeof(addr));

    void SetIPConnect(DWORD IP);
    SetIPConnect(addr.sin_addr.S_un.S_addr);
#endif

    return &m_ClientMap[dnidClient];
}



DWORD   CNetServerAdv::GetInSizeAndClear(void)
{
    DWORD   dwSize = m_dwInSize;
    m_dwInSize = 0;
    return  dwSize;
}
DWORD   CNetServerAdv::GetOutSizeAndClear(void)
{
    DWORD   dwSize = m_dwOutSize;
    m_dwOutSize = 0;
    return  dwSize;
}
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
//
//  class CLinkDataAdv
//
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

CLinkDataAdv::CLinkDataAdv() : 
    CLinkData()
{
    m_pContext = NULL;
}

CLinkDataAdv::CLinkDataAdv(DNID dnidClient, LPVOID pContext) : 
    CLinkData(dnidClient)
{
    m_pContext = pContext;
}

CLinkDataAdv::~CLinkDataAdv()
{
    m_pContext = NULL;
}

LPVOID CLinkDataAdv::GetContext() 
{ 
    return m_pContext; 
}

BOOL CLinkDataAdv::SetContext(LPVOID pContext) 
{ 
    m_pContext = pContext; 
    return TRUE; 
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
//
//  class CNetServerPacketAdv
//
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/*
CNetServerPacketAdv::CNetServerPacketAdv() : 
    CNetServerPacket()
{
    m_pContext = NULL;
}

CNetServerPacketAdv::CNetServerPacketAdv(const CNetServerPacketAdv &Packet) : 
    CNetServerPacket(Packet)
{
    m_pContext = Packet.m_pContext;
    const_cast<LPVOID>(Packet.m_pContext) = NULL;
}

CNetServerPacketAdv::CNetServerPacketAdv(DNID dnidClient, 
    LINKSTATUS enumStatus, LPVOID pvBuffer, WORD wLength, LPVOID pContext) : 
    CNetServerPacket(dnidClient, enumStatus, pvBuffer, wLength)
{
    m_pContext = pContext;
}

CNetServerPacketAdv::~CNetServerPacketAdv()
{
    m_pContext = NULL;
}

LPVOID CNetServerPacketAdv::GetContext()
{
    return m_pContext;
}

void CNetServerPacketAdv::operator = (const CNetServerPacketAdv &Packet)
{
    CNetServerPacket::operator = (Packet);

    m_pContext = Packet.m_pContext;
    const_cast<LPVOID>(Packet.m_pContext) = NULL;
}

*/