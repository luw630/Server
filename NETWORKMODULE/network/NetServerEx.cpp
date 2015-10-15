//---------------------------------------------------------------------------
//  Engine (c) 1999-2000 by Dreamwork
//
// File:	CNetServerEx.cpp
// Date:	2003.07.08
// Code:	Dreamwork
// Desc:	Simple double linked node class
//---------------------------------------------------------------------------

#include "NetServerEx.h"

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

typedef struct { BYTE _ga[64]; } GUARD_ARRAY;

static GUARD_ARRAY s_GuardArray;

#define CHECKINFO "this is guard array in class CNetServerEx for check overflow ->"

cDirectNetDecryption _cryption;

void _set_server_ga_info(char _ga[64])
{
    memcpy(&s_GuardArray, _ga, sizeof(GUARD_ARRAY));
}

// comput hash value, same as g_Hash(char *) in enginelib.lib
static DWORD ComputHashValue(LPCSTR szStr)
{
    BYTE const * pStr = (BYTE const *)szStr;
    unsigned long i = 0, j = 1;
    while (*pStr != 0) i += (*pStr++) * j++;
    return i % 256;
}

//---------------------------------------------------------------------------
// 函数:	CNetServerEx
// 功能:	构造
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
CNetServerEx::CNetServerEx(BOOL bUseVerify) :
    m_bUseVerify(bUseVerify)
{
    InitializeCriticalSection(&m_ClientMapLock);

    InitializeCriticalSection(&m_PacketQueueLock);

    m_pDirectNetServer = iDirectNetServer::CreateInstance();

    m_dwPrevCheckTime = timeGetTime();

    _set_server_ga_info(CHECKINFO);
}

//---------------------------------------------------------------------------
// 函数:	~CNetServerEx
// 功能:	分构
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
CNetServerEx::~CNetServerEx(void)
{
	Close();

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
BOOL CNetServerEx::Create(const char *szPortName)
{
    if (szPortName == NULL)
        return FALSE;

    // use 'this' as context for the message handler
    DNRESULT dnr = m_pDirectNetServer->Initialize(&_cryption, this, MAX_CLIENT_NUMBER);

    if (DNR_FAILED(dnr))
        return FALSE;

    // this is needed for extracting the port number from a string
    WORD wPort = static_cast<WORD>( ComputHashValue(szPortName) + N_SOCKET_MIN_PORTNUM );

    dnr = m_pDirectNetServer->Host(wPort);

    // ok, the DreamNetServer is now hosting ...
	
	return !DNR_FAILED(dnr);
}

//---------------------------------------------------------------------------
// 函数:	Create
// 功能:	构造
// 参数:	int
// 返回:	void
//---------------------------------------------------------------------------
BOOL CNetServerEx::Create(const WORD wPort)
{
    // use 'this' as context for the message handler
    DNRESULT dnr = m_pDirectNetServer->Initialize(NULL, this, MAX_CLIENT_NUMBER);

    if (DNR_FAILED(dnr))
        return FALSE;

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
BOOL CNetServerEx::Close(void)
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
            m_PacketQueue.pop();
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
BOOL CNetServerEx::SendMessage(DNID dnidClient, const void *pvBuf, WORD wSize)
{
    if (dnidClient == 0 || pvBuf == NULL || wSize == 0)
        return FALSE;

    DNRESULT dnr = DNR_E_FAIL;

    __try
    {
        dnr = m_pDirectNetServer->SendTo(dnidClient, const_cast<PVOID>(pvBuf), wSize);
    }
    __finally
    {
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
BOOL CNetServerEx::GetMessage(SNetServerPacket **ppPacket)
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
BOOL CNetServerEx::DelOneClient(DNID dnidClient)
{
    if (dnidClient == 0)
        return FALSE;

    __try
    {
        // the rest will be handled inside DN_MSGID_DELETE_PLAYER handler

        m_pDirectNetServer->DeletePlayer(dnidClient);
    }
    __finally
    {
    }

    return TRUE;
}

//---------------------------------------------------------------------------
void CNetServerEx::OnCreatePlayer(DNID dnidClient, PVOID *ppvPlayerContext)
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
void CNetServerEx::OnReceivePacket(DNID dnidClient, PVOID pvPlayerContext, PVOID pvBuffer, DWORD dwLength, DWORD dwSequence)
{
    if (dnidClient == 0 || pvPlayerContext == NULL || pvBuffer == NULL || dwLength == 0 || dwLength > 0xffff)
        return;

    WORD wLength = static_cast<WORD>(dwLength);

    CLinkData *pLinkData = static_cast<CLinkData *>(pvPlayerContext);

    LINKSTATUS enumStatus = LinkStatus_UnConnected;

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

        _cryption.DES_decrypt(qwHeader, 8);

        DWORD Key32 = qwHeader[1];
        DWORD Crc32 = qwHeader[0] ^ Key32;

        // 这里操作了m_ClientMap的数据，所以需要加锁！
        __try
        {
            EnterCriticalSection(&m_ClientMapLock);

            enumStatus = pLinkData->GetLinkStatus();

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
        if (Crc32 != _cryption.CRC32_compute(static_cast<LPBYTE>(pvBuffer) + 8, wLength - 8))
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
        }
        __finally
        {
            LeaveCriticalSection(&m_ClientMapLock);
        }
    }

    if (wLength == 8)
    {
        // 及时响应ping消息
        if (static_cast<const LPDWORD>( pvBuffer )[0] == 
            static_cast<const LPDWORD>( static_cast<void *>( "ping" ) )[0])
        {
            __try
            {
                EnterCriticalSection( &m_ClientMapLock );
                pLinkData->SetTimeOutTime( timeGetTime() );
                pLinkData->CheckPing( static_cast<const LPDWORD>( pvBuffer )[1] );
            }
            __finally
            {
                LeaveCriticalSection( &m_ClientMapLock );
            }

            DWORD pong[2] = {
                static_cast<const LPDWORD>( static_cast<void *>( "pong" ) )[0],
                static_cast<const LPDWORD>( pvBuffer )[1]
            };
            SendMessage( dnidClient, pong, 8 );
            return;
        }
    }

    PushPacket(dnidClient, enumStatus, pvBuffer, wLength);
}

//---------------------------------------------------------------------------
void CNetServerEx::OnDeletePlayer(DNID dnidClient, PVOID pvPlayerContext)
{
    if (dnidClient == 0 || pvPlayerContext == NULL)
        return;

    CLinkData *pLinkData = static_cast<CLinkData *>(pvPlayerContext);

    LINKSTATUS enumStatus = LinkStatus_UnConnected;

    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        enumStatus = pLinkData->GetLinkStatus();

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

    PushPacket(dnidClient, enumStatus, NULL, 0);
}

//---------------------------------------------------------------------------
BOOL CNetServerEx::IsValidDnid(DNID dnidClient)
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

BOOL CNetServerEx::GetDnidAddr(DNID dnidClient, sockaddr_in *pAddr, size_t size)
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

LINKSTATUS CNetServerEx::GetLinkStatus(DNID dnidClient)
{
    if (dnidClient == 0)
        return LinkStatus_UnConnected;

    LINKSTATUS ret = LinkStatus_UnConnected;

    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        std::map<DNID, CLinkData>::iterator it = m_ClientMap.find(dnidClient);

        if (it != m_ClientMap.end())
            ret = it->second.GetLinkStatus();
    }
    __finally
    {
        LeaveCriticalSection(&m_ClientMapLock);
    }

    return ret;
}

BOOL CNetServerEx::SetLinkStatus(DNID dnidClient, LINKSTATUS enumStatus)
{
    if (dnidClient == 0)
        return FALSE;

    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        std::map<DNID, CLinkData>::iterator it = m_ClientMap.find(dnidClient);

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

BOOL CNetServerEx::SetLinkForceTimeOutTime(DNID dnidClient, DWORD dwTime)
{
    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        std::map<DNID, CLinkData>::iterator it = m_ClientMap.find(dnidClient);

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

BOOL CNetServerEx::ClearLinkForceTimeOutTime(DNID dnidClient)
{
    return SetLinkForceTimeOutTime(dnidClient, 0xffffffff);
}

DWORD CNetServerEx::GetClientNum()
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

BOOL CNetServerEx::SetOption(LPDWORD pParam)
{
    return m_pDirectNetServer->GetPlayerConnectionInfo(0xff00ff00ff00ff00, 
        (iDirectNetServer::CONNECTIONINFO *)pParam) == DNR_S_OK;
}

void CNetServerEx::CheckAlive()
{
    DWORD dwCurrentTime = timeGetTime();

    if ((int)(dwCurrentTime - m_dwPrevCheckTime) < PINGMARGIN/10)
        return;

    m_dwPrevCheckTime = dwCurrentTime;

    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        std::map<DNID, CLinkData>::iterator it = m_ClientMap.begin();

        while (it != m_ClientMap.end())
        {
            CLinkData *pLinkData = &it->second;
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

BOOL CNetServerEx::PushPacket(DNID dnidClient, LINKSTATUS enumStatus, LPVOID pvBuffer, WORD wLength)
{
    __try
    {
        EnterCriticalSection(&m_PacketQueueLock);

        SNetServerPacket *pPacket = new SNetServerPacket;

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

        m_PacketQueue.push(pPacket);
    }
    __finally
    {
        LeaveCriticalSection(&m_PacketQueueLock);
    }

    return TRUE;
}

CLinkData *CNetServerEx::NewLinkData(DNID dnidClient)
{
    m_ClientMap[dnidClient] = CLinkData(dnidClient);

    return &m_ClientMap[dnidClient];
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
//
//  class CLinkData
//
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

CLinkData::CLinkData()
{
    m_dnidClient = 0;
    m_dwXorCipher = 0;
	m_dwTimeOutTime = 0;
    m_dwForceTimeOutTime = 0;
    m_enumStatus = LinkStatus_UnConnected;

	m_dwSegPing_ClientTime = 0;
	m_dwSegPing_ServerTime = 0;
}

CLinkData::CLinkData(DNID dnidClient)
{
    m_dnidClient = dnidClient;
    m_dwXorCipher = 0;
	m_dwTimeOutTime = timeGetTime();
    m_dwForceTimeOutTime = 0;
    m_enumStatus = LinkStatus_Waiting_Login;

	m_dwSegPing_ClientTime = 0;
	m_dwSegPing_ServerTime = 0;
}

CLinkData::~CLinkData()
{
    m_dnidClient = 0;
    m_dwXorCipher = 0;
	m_dwTimeOutTime = 0;
    m_dwForceTimeOutTime = 0;
    m_enumStatus = LinkStatus_UnConnected;

    m_dwSegPing_ClientTime = 0;
	m_dwSegPing_ServerTime = 0;
}

LINKSTATUS CLinkData::GetLinkStatus()
{
    return m_enumStatus;
}

BOOL CLinkData::SetLinkStatus(LINKSTATUS enumStatus)
{
    m_enumStatus = enumStatus;

    return TRUE;
}

DWORD CLinkData::GetXorCipher()
{
    return m_dwXorCipher;
}

BOOL CLinkData::SetXorCipher(DWORD dwXorCipher)
{
    m_dwXorCipher = dwXorCipher;

    return TRUE;
}

DWORD CLinkData::GetTimeOutTime()
{
    return m_dwTimeOutTime;
}

BOOL CLinkData::SetTimeOutTime(DWORD dwNewTime)
{
    m_dwTimeOutTime = dwNewTime;

    return TRUE;
}

DWORD CLinkData::GetForceTimeOutTime()
{
    return m_dwForceTimeOutTime;
}

BOOL CLinkData::SetForceTimeOutTime(DWORD dwNewTime)
{
    m_dwForceTimeOutTime = dwNewTime;

    return TRUE;
}

DNID CLinkData::GetDnidClient()
{
    return m_dnidClient;
}

void CLinkData::CheckPing( DWORD clientTime )
{
    if ( m_dwSegPing_ClientTime == 0 && m_dwSegPing_ServerTime == 0 )
    {
	    m_dwSegPing_ClientTime = clientTime;
	    m_dwSegPing_ServerTime = timeGetTime();
    }
    else
    {
        int margin = ( (int)timeGetTime() - (int)m_dwSegPing_ServerTime ) - ( (int)clientTime - (int)m_dwSegPing_ClientTime );
        if ( abs( margin ) > TIMEOUTTIME )
            SetForceTimeOutTime( timeGetTime() ); // disconnect immediately at next checkalive
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
//
//  class CNetServerPacket
//
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/*
CNetServerPacket::CNetServerPacket()
{
    memset(this, 0, sizeof(CNetServerPacket));
}

CNetServerPacket::CNetServerPacket(const CNetServerPacket &Packet)
{
    memset(this, 0, sizeof(CNetServerPacket));
    *this = Packet;
}

CNetServerPacket::CNetServerPacket(DNID dnidClient, LINKSTATUS enumStatus, LPVOID pvBuffer, WORD wLength)
{
    m_dnidClient = dnidClient;
    m_enumStatus = enumStatus;
    m_wLength = wLength;

    m_pvBuffer = new char[m_wLength + sizeof(GUARD_ARRAY) * 2];
    memcpy(m_pvBuffer, &s_GuardArray, sizeof(GUARD_ARRAY));
    memcpy(static_cast<LPBYTE>(m_pvBuffer) + sizeof(GUARD_ARRAY) + m_wLength, &s_GuardArray, sizeof(GUARD_ARRAY));
    memcpy(static_cast<LPBYTE>(m_pvBuffer) + sizeof(GUARD_ARRAY), pvBuffer, m_wLength);
}

CNetServerPacket::~CNetServerPacket()
{
    ReleaseBuffer();

    m_dnidClient = 0;
    m_enumStatus = LinkStatus_UnConnected;
    m_pvBuffer = NULL;
    m_wLength = 0;
}

void CNetServerPacket::operator = (const CNetServerPacket &Packet)
{
    ReleaseBuffer();

    void *pv = const_cast<CNetServerPacket *>(&Packet);

    memcpy(this, pv, sizeof(CNetServerPacket));

    memset(pv, 0, sizeof(CNetServerPacket));
}

DNID CNetServerPacket::GetLinkDnid()
{
    return m_dnidClient;
}

LINKSTATUS CNetServerPacket::GetLinkStatus()
{
    return m_enumStatus;
}

LPVOID CNetServerPacket::GetBufferEntry()
{
    return static_cast<LPBYTE>(m_pvBuffer) + sizeof(GUARD_ARRAY);
}

WORD CNetServerPacket::GetBufferLength()
{
    return m_wLength;
}

void CNetServerPacket::ReleaseBuffer()
{
    if (m_pvBuffer == NULL)
        return;

    if (memcmp(m_pvBuffer, &s_GuardArray, sizeof(GUARD_ARRAY)) != 0)
    {
        TraceMemory(m_pvBuffer, 0, m_wLength + sizeof(GUARD_ARRAY) * 2, "NetMessageOverflowSF.mem");
        return;
    }

    if (memcmp(static_cast<LPBYTE>(m_pvBuffer) + sizeof(GUARD_ARRAY) + m_wLength, 
        &s_GuardArray, sizeof(GUARD_ARRAY)) != 0)
    {
        TraceMemory(m_pvBuffer, 0, m_wLength + sizeof(GUARD_ARRAY) * 2, "NetMessageOverflowSB.mem");
        return;
    }

    delete m_pvBuffer;
}
*/