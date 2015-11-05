//---------------------------------------------------------------------------
//  Engine (c) 1999-2000 by Dreamwork
//
// File:	CNetClientEx.cpp
// Date:	2003.07.08
// Code:	Dreamwork
// Desc:	Simple double linked node class
//---------------------------------------------------------------------------

#include "NetClientEx.h"
#include <nb30.h>

#define USE_CRC
#include "desenc.h"
#include <time.h>


#pragma comment (lib, "DirectNetClient.lib")
#pragma comment( lib, "netapi32" )

//extern BOOL rfalse(char, char, LPCSTR, ...);
//extern void TraceMemory(LPVOID pDumpEntry, int iOffset, unsigned int uiSize, LPCSTR szFilename);

//---------------------------------------------------------------------------
#define N_SOCKET_MIN_PORTNUM    (7000)
#define N_SOCKET_MAX_PORTNUM    (7999)
#define N_SOCKET_PORTRANGE      (N_SOCKET_MAX_PORTNUM - N_SOCKET_MIN_PORTNUM)

#define PINGMARGIN              (15000)

typedef struct { BYTE _ga[64]; } GUARD_ARRAY;

static GUARD_ARRAY s_GuardArray;

#define CHECKINFO "this is guard array in class CNetClientEx for check overflow ->"
//#define DEBUG_NET_MSG
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
    m_dwSvrTime = 0;
    
    InitializeCriticalSection(&m_PacketQueueLock);

    m_hConnectEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // non-signaled

    m_enumStatus = LS_UnConnected;

    m_dwPrevPingTime = timeGetTime();

    m_dwKey = 0;

    m_bWaitingConnect = FALSE;

    if (sizeof(CHECKINFO) != sizeof(GUARD_ARRAY))
    {
        //rfalse(0, 0, "(INFO SIZE = %d ) != %d", sizeof(CHECKINFO), sizeof(GUARD_ARRAY));
    }

    if (sizeof(m_GuardArrayF) != sizeof(GUARD_ARRAY))
    {
        //rfalse(0, 0, "(m_GuardArrayF SIZE = %d ) != %d", sizeof(m_GuardArrayF), sizeof(GUARD_ARRAY));
    }

    if (sizeof(m_GuardArrayB) != sizeof(GUARD_ARRAY))
    {
        //rfalse(0, 0, "(m_GuardArrayB SIZE = %d ) != %d", sizeof(m_GuardArrayF), sizeof(GUARD_ARRAY));
    }

    memcpy(&s_GuardArray, CHECKINFO, sizeof(GUARD_ARRAY));

    memcpy(m_GuardArrayF, &s_GuardArray, sizeof(GUARD_ARRAY));
    memcpy(m_GuardArrayB, &s_GuardArray, sizeof(GUARD_ARRAY));

	m_dwSvrUpdateTime = 0;
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
BOOL CNetClientEx::Connect( const char *szIP, const char *szPortName, BOOL bBlock )
{
    if ( m_bWaitingConnect || szIP == NULL || szPortName == NULL )
        return FALSE;

    if ( m_enumStatus == LS_Connected )
        return FALSE;

    Disconnect();

    m_pDirectNetClient = iDirectNetClient::CreateInstance();

    if ( DNR_FAILED( m_pDirectNetClient->Initialize( ( m_bUseVerify ? &_cryption : NULL ), this ) ) )
        return FALSE;

    // this is needed for extracting the port number from a string
    WORD wPort = static_cast< WORD >( ComputHashValue( szPortName ) + N_SOCKET_MIN_PORTNUM );

    // 特殊处理，直接将端口调整为数字数据！
    if ( *szPortName == ':' )
        wPort = atoi( szPortName + 1 );

    // 发现一个逻辑错误！可能导致连接后无法正确发送数据！详细代码可察看旧的sourcesafe历史数据！
    // Connect中会创建网络底层线程！而该线程在连接成功后会主动调用OnConnectComplete……
    // 而当前函数的后续操作会将m_bWaitingConnect置为TRUE！打算在OnConnectComplete中将其恢复为FALSE……
    // 结果……如果在连接线程中OnConnectComplete被先调用的话，后续的m_bWaitingConnect=TRUE会导致m_bWaitingConnect状态永远无法恢复……
    
    m_bWaitingConnect = TRUE;

    if ( DNR_FAILED( m_pDirectNetClient->Connect( szIP, wPort ) ) )
        return FALSE;

    if ( bBlock )
    {
        // wait until the connection has completed
        WaitForSingleObject( m_hConnectEvent, INFINITE );

        if ( m_enumStatus == LS_UnConnected )
            return FALSE;

        m_bWaitingConnect = FALSE;
    }

    return TRUE;
}		   

BOOL CNetClientEx::Connect( const char *szIP, WORD wPort, BOOL bBlock )
{
    if ( m_bWaitingConnect )
        return FALSE;

    if ( m_enumStatus != LS_UnConnected )
        return FALSE;

    Disconnect();

    m_pDirectNetClient = iDirectNetClient::CreateInstance();

    if ( DNR_FAILED( m_pDirectNetClient->Initialize( ( m_bUseVerify ? &_cryption : NULL ), this ) ) )
        return FALSE;

    m_bWaitingConnect = TRUE;
    if ( DNR_FAILED( m_pDirectNetClient->Connect( szIP, wPort ) ) )
        return FALSE;

    if ( bBlock )
    {
        // wait until the connection has completed
        WaitForSingleObject( m_hConnectEvent, INFINITE );

        if ( m_enumStatus == LS_UnConnected )
            return FALSE;

        m_bWaitingConnect = FALSE;
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
        {
            if ( m_PacketQueue.front() ) {
                if ( m_PacketQueue.front()->m_pvBuffer )
                    delete m_PacketQueue.front()->m_pvBuffer;
                delete m_PacketQueue.front();
            }
            m_PacketQueue.pop();
        }
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
        else if (static_cast<const LPDWORD>(pvBuffer)[0] == 
            static_cast<const LPDWORD>(static_cast<void *>("tick"))[0])
        {
	        m_dwSvrTime = static_cast<const LPDWORD>(pvBuffer)[1];
			m_dwSvrUpdateTime = timeGetTime();
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

BOOL isErrCtrl = 0;
DWORD errCode = 0;
void CNetClientEx::KeepAlive()
{
    if (m_bWaitingConnect)
        return;

    if ((int)(timeGetTime() - m_dwPrevPingTime) < PINGMARGIN)
        return;

    m_dwPrevPingTime = timeGetTime();

    if ( !m_bUseVerify )
    {
        DWORD ping[2] = {
            static_cast<const LPDWORD>(static_cast<void *>("ping"))[0],
            ( timeGetTime() & 0xfffffff7 ) | ( isErrCtrl << 3 )
        };
        SendMessage(ping, 8);
    }
    else
    {
        DWORD ping[4] = {
            static_cast<const LPDWORD>(static_cast<void *>("ping"))[0],
            ( timeGetTime() & 0xfffffff7 ) | ( isErrCtrl << 3 )
        };
        unsigned __int64 GetFirstMacAddr();
        ( ( unsigned __int64* )ping )[1] = GetFirstMacAddr();
        SendMessage(ping, 16);
    }
}
void WriteLog(const char *format, ...)
{
	//MLOCK lk(s_cs);	// 多线程安全

	FILE *hf = NULL;
	char tmp[4096];
	hf = fopen("net_log.txt", "a");
	if(!hf) return;

	vsprintf(tmp, format, (char *)&format+sizeof(format));
	// 自动在日志后面增加时间,便于跟踪调试
	fprintf(hf, tmp);
	time_t current_time;
	time(&current_time);
	fprintf(hf," --%s", ctime(&current_time));

	fclose(hf);

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
		//将消息记录到日志
#ifdef DEBUG_NET_MSG
		if(pvBuffer)
		{
			BYTE* pMsg = (BYTE*)pvBuffer;
			WriteLog("%d %d size:%d", pMsg[0],pMsg[1],wLength);
		}
#endif
        //extern DWORD cinsize;
        //cinsize += (((DWORD)wLength) + 512);
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
        //TraceMemory(m_GuardArrayF, 0, sizeof(m_SendBuffer) + 
        //    sizeof(GUARD_ARRAY) * 2, "SendBufferOverflow.mem");

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


typedef struct _ASTAT_ { 
    ADAPTER_STATUS adapt;
    NAME_BUFFER    NameBuff [30];
} ASTAT, *PASTAT;

int __stdcall GetLanaID( int *lana, int n )
{
    NCB Ncb;
    LANA_ENUM lenum;

    memset( &Ncb, 0, sizeof( Ncb ) );
    Ncb.ncb_command = NCBENUM;
    Ncb.ncb_buffer = ( UCHAR * )&lenum;
    Ncb.ncb_length = sizeof( lenum );
    if ( NRC_GOODRET != Netbios( &Ncb ) )
        return 0;

    for ( int i = 0; i < min( lenum.length, n ); i++ )
        lana[i] = lenum.lana[i];

    return lenum.length;
}

unsigned __int64 __stdcall GetMACAddress( int LANAID )
{
    NCB Ncb;
    ASTAT Adapter;
    unsigned __int64 mac = 0;

    memset( &Ncb, 0, sizeof( Ncb ) );
    Ncb.ncb_command = NCBRESET;
    Ncb.ncb_lana_num = LANAID;

    if ( NRC_GOODRET != Netbios( &Ncb ) )
        return 0;

    memset( &Ncb, 0, sizeof ( Ncb ) );
    Ncb.ncb_command = NCBASTAT;
    Ncb.ncb_lana_num = LANAID;
    Ncb.ncb_callname[0] = '*';

    memset( &Adapter, 0, sizeof( Adapter ) );
    Ncb.ncb_buffer = ( unsigned char * )&Adapter;
    Ncb.ncb_length = sizeof( Adapter );

    if ( NRC_GOODRET != Netbios( &Ncb ) )
        return 0;

    LPBYTE pmac = ( LPBYTE )&mac;

    pmac[0]= Adapter.adapt.adapter_address[0] ^ 0x33;
    pmac[1]= Adapter.adapt.adapter_address[1] ^ 0xAC;
    pmac[2]= Adapter.adapt.adapter_address[2] ^ 0xFD;
    pmac[3]= Adapter.adapt.adapter_address[3] ^ 0xCC;
    pmac[4]= Adapter.adapt.adapter_address[4] ^ 0x19;
    pmac[5]= Adapter.adapt.adapter_address[5] ^ 0x74;

    return mac;
}

unsigned __int64 GetFirstMacAddr()
{
    /*
    struct ASTAT 
    {
        ADAPTER_STATUS adapt;
        NAME_BUFFER    psz_name[30];
    } Adapter;

    NCB ncb;

    memset( &ncb, 0, sizeof( ncb ) );
    ncb.ncb_command  = NCBRESET;
    if ( Netbios( &ncb ) != NRC_GOODRET )
        return 0;

    memset( &ncb, 0, sizeof( ncb ) );
    ncb.ncb_command = NCBASTAT;
    ncb.ncb_callname[0] = '*';

    memset( &Adapter, 0, sizeof( Adapter ) );

    ncb.ncb_buffer = ( unsigned char * )&Adapter;
    ncb.ncb_length = sizeof( Adapter );
    if ( Netbios( &ncb ) != NRC_GOODRET )
        return 0;

    unsigned __int64 result = 0;
    memcpy( &result, Adapter.adapt.adapter_address, 6 );
    return result;
    */

    unsigned __int64 GetMac3Times();
    unsigned __int64 get_first_mac();
    unsigned __int64 mac = GetMac3Times();
    if ( mac == 0x7419CCFDAC33 || mac == 0 ) {
        int lanas[16];
        int nlana = GetLanaID( lanas, 16 );
        if ( nlana != 0 )
            mac = GetMACAddress( lanas[0] );
        if ( mac == 0x7419CCFDAC33 || mac == 0 )
            mac = get_first_mac();
        if ( mac == 0x7419CCFDAC33 || mac == 0 )
            mac = GetMac3Times();
    }
    return mac;
}

#include <iphlpapi.h>
#pragma comment ( lib, "iphlpapi" )

unsigned __int64 GetMac3Times()
{
    static int initWSA = false;
    if ( !initWSA )
    {
        initWSA = true;
        WSADATA w;
        WSAStartup( 0x101, &w );
    }

    //HRESULT hr;
    //IPAddr  ipAddr;
    unsigned __int64 mac;
    ULONG   ulLen;
    char name[100];
    int i = gethostname( name, 100 );
    if ( SOCKET_ERROR == i )
        return 0;

    hostent *p = gethostbyname( name );
    if ( p == 0 || p->h_length == 0 )
        return 0;

    //ipAddr = inet_addr( p->h_addr_list[0] );
    //inet_addr( p->h_addr_list[0] );
    mac = 0;
    ulLen = 6;
    
    if ( NO_ERROR != SendARP( *( LPDWORD )( p->h_addr_list[0] ), 0, (PULONG)&mac, &ulLen ) )
        return 0;

    unsigned __int64 macret = 0;;
    LPBYTE pmac = ( LPBYTE )&mac;
    LPBYTE cmac = ( LPBYTE )&macret;

    cmac[0]= pmac[0] ^ 0x33;
    cmac[1]= pmac[1] ^ 0xAC;
    cmac[2]= pmac[2] ^ 0xFD;
    cmac[3]= pmac[3] ^ 0xCC;
    cmac[4]= pmac[4] ^ 0x19;
    cmac[5]= pmac[5] ^ 0x74;

    return macret;
}

#include   <snmp.h>     
#include   <conio.h>     
#include   <stdio.h>     

typedef   bool(WINAPI   *   pSnmpExtensionInit)   (     
    IN   DWORD   dwTimeZeroReference,     
    OUT   HANDLE   *   hPollForTrapEvent,     
    OUT   AsnObjectIdentifier   *   supportedView);     

typedef   bool(WINAPI   *   pSnmpExtensionTrap)   (     
    OUT   AsnObjectIdentifier   *   enterprise,     
    OUT   AsnInteger   *   genericTrap,     
    OUT   AsnInteger   *   specificTrap,     
    OUT   AsnTimeticks   *   timeStamp,     
    OUT   RFC1157VarBindList   *   variableBindings);     

typedef   bool(WINAPI   *   pSnmpExtensionQuery)   (     
    IN   BYTE   requestType,     
    IN   OUT   RFC1157VarBindList   *   variableBindings,     
    OUT   AsnInteger   *   errorStatus,     
    OUT   AsnInteger   *   errorIndex);     

typedef   bool(WINAPI   *   pSnmpExtensionInitEx)   (     
    OUT   AsnObjectIdentifier   *   supportedView);     

#pragma comment ( lib, "Snmpapi" )
#define printf rfalse

unsigned __int64 get_first_mac()     
{     
    static HINSTANCE   m_hInst = (HINSTANCE)HINSTANCE_ERROR;     
    static pSnmpExtensionInit   m_Init;     
    static pSnmpExtensionInitEx   m_InitEx;     
    static pSnmpExtensionQuery   m_Query;     
    static pSnmpExtensionTrap   m_Trap;
    HANDLE   PollForTrapEvent;     
    AsnObjectIdentifier   SupportedView;     
    UINT   OID_ifEntryType[]  = { 1,   3,   6,   1,   2,   1,   2,   2,   1,   3 };
    UINT   OID_ifEntryNum[]   = { 1,   3,   6,   1,   2,   1,   2,   1 };
    UINT   OID_ipMACEntAddr[] = { 1,   3,   6,   1,   2,   1,   2,   2,   1,   6 };
    AsnObjectIdentifier  MIB_ifMACEntAddr = { sizeof(OID_ipMACEntAddr) / sizeof(UINT),   OID_ipMACEntAddr };     
    AsnObjectIdentifier  MIB_ifEntryType  = { sizeof(OID_ifEntryType)  / sizeof(UINT),   OID_ifEntryType  };     
    AsnObjectIdentifier  MIB_ifEntryNum   = { sizeof(OID_ifEntryNum)   / sizeof(UINT),   OID_ifEntryNum   };     
    RFC1157VarBindList   varBindList;     
    RFC1157VarBind   varBind[2];     
    AsnInteger   errorStatus;     
    AsnInteger   errorIndex;     
    AsnObjectIdentifier   MIB_NULL   =   {0,   0};     
    int   ret;     
    int   dtmp;     
    int   i   =   0,   j   =   0;     
    bool   found   =   false;     
    unsigned __int64 mac = 0;

    /*   载入SNMP   DLL并取得实例句柄   */     
    if ( m_hInst == (HINSTANCE)HINSTANCE_ERROR )
    {
        m_hInst = LoadLibrary("inetmib1.dll");     
        if (m_hInst <= (HINSTANCE)HINSTANCE_ERROR)
            m_hInst = NULL;

        if ( m_hInst == NULL )
            return 0;

        m_Init = (pSnmpExtensionInit)GetProcAddress(m_hInst,   "SnmpExtensionInit");     
        m_InitEx = (pSnmpExtensionInitEx)GetProcAddress(m_hInst, "SnmpExtensionInitEx");     
        m_Query = (pSnmpExtensionQuery)GetProcAddress(m_hInst, "SnmpExtensionQuery");     
        m_Trap = (pSnmpExtensionTrap)GetProcAddress(m_hInst,   "SnmpExtensionTrap");     
    }

    if ( m_hInst == NULL )
        return 0;

    m_Init(GetTickCount(),   &PollForTrapEvent,   &SupportedView);     

    /*   初始化用来接收m_Query查询结果的变量列表   */     
    varBindList.list   =   varBind;     
    varBind[0].name   =   MIB_NULL;     
    varBind[1].name   =   MIB_NULL;     

    /*   在OID中拷贝并查找接口表中的入口数量   */     
    varBindList.len   =   1;   /*   Only   retrieving   one   item   */     
    SNMP_oidcpy(&varBind[0].name,   &MIB_ifEntryNum);     
    ret = m_Query(ASN_RFC1157_GETNEXTREQUEST, &varBindList, &errorStatus, &errorIndex);     
    //printf("#   of   adapters   in   this   system   :   %in", varBind[0].value.asnValue.number);     
    varBindList.len = 2;     

    /*   拷贝OID的ifType－接口类型   */     
    SNMP_oidcpy(&varBind[0].name,   &MIB_ifEntryType);     

    /*   拷贝OID的ifPhysAddress－物理地址   */     
    SNMP_oidcpy(&varBind[1].name,   &MIB_ifMACEntAddr);     

    do     
    {
        /*   提交查询，结果将载入   varBindList。     
        可以预料这个循环调用的次数和系统中的接口卡数量相等   */     
        ret = m_Query( ASN_RFC1157_GETNEXTREQUEST, &varBindList, &errorStatus, &errorIndex );
        if ( !ret ) 
            ret = 1;     
        else     
            /*   确认正确的返回类型   */     
            ret = SNMP_oidncmp( &varBind[0].name, &MIB_ifEntryType, MIB_ifEntryType.idLength );

        if ( !ret ) {     
            j++;     
            dtmp   =   varBind[0].value.asnValue.number;     
            //printf("Interface   #%i   type   :   %in",   j,   dtmp);     

            /*   Type   6   describes   ethernet   interfaces   */     
            if   (dtmp   ==   6)     
            {     
                /*   确认我们已经在此取得地址   */     
                ret = SNMP_oidncmp( &varBind[1].name, &MIB_ifMACEntAddr, MIB_ifMACEntAddr.idLength );
                if ( ( !ret ) && ( varBind[1].value.asnValue.address.stream != NULL ) )     
                {     
                    if ((varBind[1].value.asnValue.address.stream[0]   ==   0x44)     
                        &&   (varBind[1].value.asnValue.address.stream[1]   ==   0x45)     
                        &&   (varBind[1].value.asnValue.address.stream[2]   ==   0x53)     
                        &&   (varBind[1].value.asnValue.address.stream[3]   ==   0x54)     
                        &&   (varBind[1].value.asnValue.address.stream[4]   ==   0x00))     
                    {     
                        /*   忽略所有的拨号网络接口卡   */     
                        //printf("Interface   #%i   is   a   DUN   adaptern",   j);     
                        continue;     
                    }     
                    if ((varBind[1].value.asnValue.address.stream[0]   ==   0x00)     
                        &&   (varBind[1].value.asnValue.address.stream[1]   ==   0x00)     
                        &&   (varBind[1].value.asnValue.address.stream[2]   ==   0x00)     
                        &&   (varBind[1].value.asnValue.address.stream[3]   ==   0x00)     
                        &&   (varBind[1].value.asnValue.address.stream[4]   ==   0x00)     
                        &&   (varBind[1].value.asnValue.address.stream[5]   ==   0x00))     
                    {     
                        /*   忽略由其他的网络接口卡返回的NULL地址   */     
                        //printf("Interface   #%i   is   a   NULL   addressn",   j);     
                        continue;     
                    }     
                    /*
                    sprintf(TempEthernet,   "%02x%02x%02x%02x%02x%02x",     
                        varBind[1].value.asnValue.address.stream[0],     
                        varBind[1].value.asnValue.address.stream[1],     
                        varBind[1].value.asnValue.address.stream[2],     
                        varBind[1].value.asnValue.address.stream[3],     
                        varBind[1].value.asnValue.address.stream[4],     
                        varBind[1].value.asnValue.address.stream[5]);     */
                    //printf("MAC   Address   of   interface   #%i:   %sn",   j, TempEthernet);

                    LPBYTE pmac = ( LPBYTE )&mac;

                    pmac[0]= varBind[1].value.asnValue.address.stream[0] ^ 0x33;
                    pmac[1]= varBind[1].value.asnValue.address.stream[1] ^ 0xAC;
                    pmac[2]= varBind[1].value.asnValue.address.stream[2] ^ 0xFD;
                    pmac[3]= varBind[1].value.asnValue.address.stream[3] ^ 0xCC;
                    pmac[4]= varBind[1].value.asnValue.address.stream[4] ^ 0x19;
                    pmac[5]= varBind[1].value.asnValue.address.stream[5] ^ 0x74;
                    break;
                }     
            }     
        }
    }   while   (!ret);   /*   发生错误终止。   */     

    //FreeLibrary(m_hInst);     
    /*   解除绑定   */     
    SNMP_FreeVarBind(&varBind[0]);     
    SNMP_FreeVarBind(&varBind[1]);   
    return mac;
}     
