//---------------------------------------------------------------------------
//  Engine (c) 1999-2000 by Dreamwork
//
// File:	CNetClientEx.cpp
// Date:	2003.07.08
// Code:	Dreamwork
// Desc:	Simple double linked node class
//---------------------------------------------------------------------------

#include "NetClientEx.h"

#define USE_CRC
#include "desenc.h"

#pragma comment (lib, "DirectNetClient.lib")

extern BOOL rfalse(char, char, LPCSTR, ...);
extern void TraceMemory(LPVOID pDumpEntry, int iOffset, unsigned int uiSize, LPCSTR szFilename);

//---------------------------------------------------------------------------
#define N_SOCKET_MIN_PORTNUM    (7000)
#define N_SOCKET_MAX_PORTNUM    (7999)
#define N_SOCKET_PORTRANGE      (N_SOCKET_MAX_PORTNUM - N_SOCKET_MIN_PORTNUM)

#define PINGMARGIN              (15000)

typedef struct { BYTE _ga[64]; } GUARD_ARRAY;

static GUARD_ARRAY s_GuardArray;

#define CHECKINFO "this is guard array in class CNetClientEx for check overflow ->"

cDirectNetEncryption _cryption;

// comput hash value, same as g_Hash(char *) in enginelib.lib
static DWORD ComputHashValue(LPCSTR szStr)
{
    BYTE const * pStr = (BYTE const *)szStr;
    unsigned long i = 0, j = 1;
    while (*pStr != 0) i += (*pStr++) * j++;
    return i % 256;
}

//---------------------------------------------------------------------------
// 函数:	CNetClientEx
// 功能:	constructor
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
CNetClientEx::CNetClientEx(BOOL bUseVerify) :
    m_bUseVerify(bUseVerify)
{
    m_pDirectNetClient = NULL;

    m_dwPing = -1;
    
    InitializeCriticalSection(&m_PacketQueueLock);

    m_hConnectEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // non-signaled

    m_enumStatus = LS_UnConnected;

    m_dwPrevPingTime = timeGetTime();

    m_dwKey = 0;

    m_bWaitingConnect = FALSE;

    if (sizeof(CHECKINFO) != sizeof(GUARD_ARRAY))
    {
        rfalse(0, 0, "(INFO SIZE = %d ) != %d", sizeof(CHECKINFO), sizeof(GUARD_ARRAY));
    }

    if (sizeof(m_GuardArrayF) != sizeof(GUARD_ARRAY))
    {
        rfalse(0, 0, "(m_GuardArrayF SIZE = %d ) != %d", sizeof(m_GuardArrayF), sizeof(GUARD_ARRAY));
    }

    if (sizeof(m_GuardArrayB) != sizeof(GUARD_ARRAY))
    {
        rfalse(0, 0, "(m_GuardArrayB SIZE = %d ) != %d", sizeof(m_GuardArrayF), sizeof(GUARD_ARRAY));
    }

    memcpy(&s_GuardArray, CHECKINFO, sizeof(GUARD_ARRAY));

    memcpy(m_GuardArrayF, &s_GuardArray, sizeof(GUARD_ARRAY));
    memcpy(m_GuardArrayB, &s_GuardArray, sizeof(GUARD_ARRAY));
}

//---------------------------------------------------------------------------
// 函数:	~CNetClientEx
// 功能:	destructor
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
CNetClientEx::~CNetClientEx()
{
    Disconnect();

    CloseHandle(m_hConnectEvent);

    DeleteCriticalSection(&m_PacketQueueLock);

    CheckGuardArray();
}

//---------------------------------------------------------------------------
// 函数:	Connect
// 功能:	
// 参数:	char*
// 返回:	bool
//---------------------------------------------------------------------------
BOOL CNetClientEx::Connect(const char *szIP, const char *szPortName, BOOL bBlock)
{
    if (m_bWaitingConnect)
        return FALSE;

    if (m_enumStatus == LS_Disconnected)
    {
        Disconnect();
    }
    else if (m_enumStatus != LS_UnConnected)
    {
        return FALSE;
    }

    m_pDirectNetClient = iDirectNetClient::CreateInstance();

    DNRESULT dnr = m_pDirectNetClient->Initialize(&_cryption, this);
    if (DNR_FAILED(dnr))
    {
        return FALSE;
    }

    // this is needed for extracting the port number from a string
    WORD wPort = static_cast<WORD>( ComputHashValue(szPortName) + N_SOCKET_MIN_PORTNUM );

    dnr = m_pDirectNetClient->Connect(szIP, wPort);
    if (DNR_FAILED(dnr))
    {
        return FALSE;
    }

    if (bBlock)
    {
        // wait until the connection has completed
        WaitForSingleObject(m_hConnectEvent, INFINITE);

        if (m_enumStatus == LS_UnConnected)
        {
            return FALSE;
        }
    }
    else
    {
        m_bWaitingConnect = TRUE;
    }

    return TRUE;
}		   

//---------------------------------------------------------------------------
// 函数:	Disconnect
// 功能:	断开连接
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
BOOL CNetClientEx::Disconnect()
{
    if (m_bWaitingConnect)
        return FALSE;

    if (!IsConnected())
    {
        if (m_pDirectNetClient != NULL)
        {
            m_pDirectNetClient->Release();
            m_pDirectNetClient = NULL;
        }

        return FALSE;
    }

    DNRESULT dnr = m_pDirectNetClient->Close();
    //m_pDirectNetClient->Disconnect(); dnr = 

    __try
    {
        EnterCriticalSection(&m_PacketQueueLock);

        while (m_PacketQueue.size())
            m_PacketQueue.pop();
    }
    __finally
    {
        LeaveCriticalSection(&m_PacketQueueLock);
    }

    m_enumStatus = LS_UnConnected;

    m_dwKey = 0; 

    m_pDirectNetClient->Release();

    m_pDirectNetClient = NULL;

    return DNR_FAILED(dnr);
}

BOOL CNetClientEx::SetOption(LPDWORD pParam)
{
    //if (m_bWaitingConnect)
    //    return FALSE;
    // 因为SetOption只是设置内部状态，所以可以在未连接前使用！

    if (m_pDirectNetClient == NULL)
        return FALSE;

    return m_pDirectNetClient->Send(pParam, 0xf0f0f0f0) == DNR_S_OK;
}

//---------------------------------------------------------------------------
// 函数:	SendMsg
// 功能:	
// 参数:	void*, long
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL CNetClientEx::SendMessage(const void *pvBuf, WORD wSize)
{
    if (m_bWaitingConnect)
        return FALSE;

    if (!IsConnected())
        return FALSE;

	if (pvBuf == NULL || wSize == 0) 
        return FALSE;

    DNRESULT dnr = DNR_E_FAIL;

    if (!CheckGuardArray())
    {
        // assert !!!
    }

    if (m_bUseVerify)
    {
        LPDWORD pDword = (LPDWORD)m_SendBuffer;

        memcpy(m_SendBuffer + 8, pvBuf, wSize);

        // 4字节上对齐
        if (wSize & 0x3)
            wSize = (wSize & 0xfffc) + 4;

        pDword[1] = m_dwKey = ( m_dwKey == 0 ) 
            ? ( timeGetTime() ^ ( ( rand() << 20 ) | ( rand() << 10 ) | rand() ) ) 
            : ( _get_dval(m_dwKey) );

        pDword[0] = _cryption.CRC32_compute(m_SendBuffer + 8, wSize) ^ m_dwKey;

        LPDWORD pXorPtr = (LPDWORD)m_SendBuffer + 2;
        WORD count = (wSize >> 2);

        DWORD dwAccKey = m_dwKey;

        while (count > 0)
        {
            *pXorPtr ^= dwAccKey;
            pXorPtr ++;
            count --;
            dwAccKey += 0xcdcd;
        }

        _cryption.DES_encrypt(m_SendBuffer, 8);

        dnr = m_pDirectNetClient->Send(m_SendBuffer, wSize + 8);
    }
    else
    {
        dnr = m_pDirectNetClient->Send(const_cast<PVOID>(pvBuf), wSize);
    }

    if ( DNR_FAILED(dnr) )
	{
		return FALSE;
	}

    if (!CheckGuardArray())
    {
        // assert !!!
    }

	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	GetMessage
// 功能:	
// 参数:	
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL CNetClientEx::GetMessage(SNetClientPacket **ppPacket)
{
    if (m_bWaitingConnect)
        return FALSE;

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

void CNetClientEx::OnConnectComplete(DNRESULT dnr)
{
    // 只要连接完成之后就需要初始化关键数据
    m_dwKey = 0; 

    if (DNR_FAILED(dnr))
        m_enumStatus = LS_UnConnected;
    else
        m_enumStatus = LS_Connected;

    SetEvent(m_hConnectEvent);

    m_bWaitingConnect = FALSE;
}

void CNetClientEx::OnReceivePacket(PVOID pvBuffer, DWORD dwLength, DWORD dwSequence)
{
    if (pvBuffer == NULL || dwLength == 0 || dwLength > 0xffff)
        return;

    //char info[100];
    //sprintf(info, "i_n %d<%02x %02x %02x %02x>\r\n", dwLength, 
    //    ((LPBYTE)pvBuffer)[0], ((LPBYTE)pvBuffer)[1], 
    //    ((LPBYTE)pvBuffer)[2], ((LPBYTE)pvBuffer)[3]);

    //OutputDebugString(info);

    WORD wLength = static_cast<WORD>(dwLength);

    if (wLength == 8)
    {
        // 及时处理pong消息
        if (static_cast<const LPDWORD>(pvBuffer)[0] == 
            static_cast<const LPDWORD>(static_cast<void *>("pong"))[0])
        {
	        m_dwPing = (timeGetTime() - static_cast<const LPDWORD>(pvBuffer)[1]) / 2;
            char info[32];
            sprintf(info, "%d\r\n", m_dwPing); 
            //OutputDebugString(info);
            return;
        }
    }

    PushPacket(pvBuffer, wLength);
}

void CNetClientEx::OnTerminateSession()
{
    m_enumStatus = LS_Disconnected;

    PushPacket(NULL, 0);
}

BOOL CNetClientEx::IsConnected()
{
    return m_enumStatus == LS_Connected; 
}

DWORD CNetClientEx::GetPing()
{
    return m_dwPing;
}

DWORD CNetClientEx::GetPacketNumber()
{
    return (DWORD)m_PacketQueue.size();
}

BOOL CNetClientEx::IsWaitingConnect() 
{ 
    return m_bWaitingConnect; 
}

void CNetClientEx::KeepAlive()
{
    if (m_bWaitingConnect)
        return;

    if ((int)(timeGetTime() - m_dwPrevPingTime) < PINGMARGIN)
        return;

    m_dwPrevPingTime = timeGetTime();

    DWORD ping[2] = {
        static_cast<const LPDWORD>(static_cast<void *>("ping"))[0],
        timeGetTime()
    };
    SendMessage(ping, 8);
}

BOOL CNetClientEx::PushPacket(LPVOID pvBuffer, WORD wLength)
{
    __try
    {
        EnterCriticalSection(&m_PacketQueueLock);

        SNetClientPacket *pPacket = new SNetClientPacket;

        if (wLength == 0)
        {
            pPacket->m_pvBuffer = NULL;
        }
        else
        {
            pPacket->m_pvBuffer = new char[((DWORD)wLength) + 512];
            memcpy(pPacket->m_pvBuffer, pvBuffer, wLength);
        }

        pPacket->m_wLength = wLength;

        m_PacketQueue.push(pPacket);

        extern DWORD cinsize;
        cinsize += (((DWORD)wLength) + 512);
    }
    __finally
    {
        LeaveCriticalSection(&m_PacketQueueLock);
    }

    return TRUE;
}

BOOL CNetClientEx::CheckGuardArray()
{
    if (memcmp(m_GuardArrayB, &s_GuardArray, sizeof(GUARD_ARRAY)) != 0 ||
        memcmp(m_GuardArrayF, &s_GuardArray, sizeof(GUARD_ARRAY)) != 0)
    {
        TraceMemory(m_GuardArrayF, 0, sizeof(m_SendBuffer) + 
            sizeof(GUARD_ARRAY) * 2, "SendBufferOverflow.mem");

        memcpy(m_GuardArrayF, &s_GuardArray, sizeof(GUARD_ARRAY));
        memcpy(m_GuardArrayB, &s_GuardArray, sizeof(GUARD_ARRAY));

        return FALSE;
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
//
//  class CNetClientPacket
//
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/*
CNetClientPacket::CNetClientPacket()
{
    memset(this, 0, sizeof(CNetClientPacket));
}

CNetClientPacket::CNetClientPacket(const CNetClientPacket &Packet)
{
    memset(this, 0, sizeof(CNetClientPacket));
    *this = Packet;
}

CNetClientPacket::CNetClientPacket(LPVOID pvBuffer, WORD wLength)
{
    m_wLength = wLength;

    m_pvBuffer = new char[m_wLength + sizeof(GUARD_ARRAY) * 2];
    memcpy(m_pvBuffer, &s_GuardArray, sizeof(GUARD_ARRAY));
    memcpy(static_cast<LPBYTE>(m_pvBuffer) + sizeof(GUARD_ARRAY) + m_wLength, &s_GuardArray, sizeof(GUARD_ARRAY));
    memcpy(static_cast<LPBYTE>(m_pvBuffer) + sizeof(GUARD_ARRAY), pvBuffer, m_wLength);
}

CNetClientPacket::~CNetClientPacket()
{
    ReleaseBuffer();

    m_pvBuffer = NULL;
    m_wLength = 0;
}

void CNetClientPacket::operator = (const CNetClientPacket &Packet)
{
    ReleaseBuffer();

    void *pv = const_cast<CNetClientPacket *>(&Packet);

    memcpy(this, pv, sizeof(CNetClientPacket));

    memset(pv, 0, sizeof(CNetClientPacket));
}

LPVOID CNetClientPacket::GetBufferEntry()
{
    return static_cast<LPBYTE>(m_pvBuffer) + sizeof(GUARD_ARRAY);
}

WORD CNetClientPacket::GetBufferLength()
{
    return m_wLength;
}

void CNetClientPacket::ReleaseBuffer()
{
    if (m_pvBuffer == NULL)
        return;

    if (memcmp(m_pvBuffer, &s_GuardArray, sizeof(GUARD_ARRAY)) != 0)
    {
        TraceMemory(m_pvBuffer, 0, m_wLength + sizeof(GUARD_ARRAY) * 2, "NetMessageOverflow.mem");
        return;
    }
    if (memcmp(static_cast<LPBYTE>(m_pvBuffer) + sizeof(GUARD_ARRAY) + m_wLength, 
        &s_GuardArray, sizeof(GUARD_ARRAY)) != 0)
    {
        TraceMemory(m_pvBuffer, 0, m_wLength + sizeof(GUARD_ARRAY) * 2, "NetMessageOverflow.mem");
        return;
    }

    delete m_pvBuffer;
}
*/