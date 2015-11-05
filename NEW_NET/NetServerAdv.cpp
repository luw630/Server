//---------------------------------------------------------------------------
//  Engine (c) 1999-2000 by Dreamwork
//
// File:	CNetServerAdv.cpp
// Date:	2003.07.08
// Code:	Dreamwork
// Desc:	Simple double linked node class
//---------------------------------------------------------------------------

#include "NetServerAdv.h"
#include "time.h"

#define USE_CRC
#include "desdec.h"

#pragma comment (lib, "DirectNetServer.lib")

extern BOOL rfalse(char, char, LPCSTR, ...);
extern void TraceMemory(LPVOID pDumpEntry, int iOffset, unsigned int uiSize, LPCSTR szFilename);

//---------------------------------------------------------------------------
#define N_SOCKET_MIN_PORTNUM    (7000)
#define N_SOCKET_MAX_PORTNUM    (7999)
#define N_SOCKET_PORTRANGE      (N_SOCKET_MAX_PORTNUM - N_SOCKET_MIN_PORTNUM)

#define TIMEOUTTIME             (1800000)//1000*60*30
#define PINGMARGIN              (15000)

cDirectNetDecryption _cryption2;

#define CHECKINFO "this is guard array in class CNetServerExAdv for check overflow"

extern void _set_server_ga_info(char _ga[64]);
BOOL wgLimit = 0;

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
    m_MacLimit = 0;
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

    // 特殊处理，直接将端口调整为数字数据！
    if ( *szPortName == ':' )
        wPort = atoi( szPortName + 1 );

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
    if (0 == dnidClient)
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

            // 统计消息数量，避免同一个链接产生大量的消息请求，导致服务器处理不及而挂掉！！！
            // 先解出前4个字节，用于判断消息类型！
            BYTE temp[4];
            reinterpret_cast< LPDWORD >( temp )[0] = reinterpret_cast< LPDWORD >( pvBuffer )[2];
            reinterpret_cast< LPDWORD >( temp )[0] ^= Key32;

            // 特殊处理，标记头为 0x04ff1023 的消息为外挂上报消息，需要放行到上层！！！
            if ( reinterpret_cast< LPDWORD >( temp )[0] == 0x04ff1023 )
            {
                // 这个特殊消息只能接收一次！
                if ( pLinkData->flags & 2 )
                {
                    DelOneClient(dnidClient);
                    return;
                }

                pLinkData->flags |= 2;
            }
            else
            {
                DWORD *cnt = 0, *chk = 0;
                DWORD max = 0, dur = 0;
                if ( temp[0] == 36 && temp[1] == 5 )
                {
                    cnt = &pLinkData->_minuteMsgCount;
                    chk = &pLinkData->_minuteChkTicks;
                    max = 3000;
                    dur = 60000;
                }
                else
                {
                    cnt = &pLinkData->_secondMsgCount;
                    chk = &pLinkData->_secondChkTicks;
                    max = 1000;
                    dur = 60000;
                }

                if ( *cnt < max )
                    (*cnt)++;
                else
                {
                    // 这里说明在预设时间内消息计数已经超过预算了，需要做断开处理
                    if ( ( int )( timeGetTime() - *chk ) < dur ) 
                    {
                        if ( *cnt < ( max * 10 ) )	// 如果是在接收消息的上限10倍内，只做消息限制处理
                            enumStatus = LinkStatus_UnConnected;
                        else
                            DelOneClient(dnidClient); // 否则做连接断开。。。
                        return;
                    }

                    *cnt = 0;
                    *chk = timeGetTime();
                }

                // 处于该连接状态下的所有接收操作都将被抛弃
                if ( enumStatus == LinkStatus_UnConnected )
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
            // 普通的ping只能在没有mac限制的情况下出现，否则不起作用。
            if ( m_MacLimit != 0 )
                return;

            int ct = 0;
            __try
            {
                EnterCriticalSection(&m_ClientMapLock);
                pLinkData->SetTimeOutTime(timeGetTime());
                pLinkData->CheckPing( static_cast<const LPDWORD>( pvBuffer )[1] );
                ct = pLinkData->checkTimes ++;
            }
            __finally
            {
                LeaveCriticalSection(&m_ClientMapLock);
            }

            if ( ct & 1 )
            {
                DWORD pong[2] = {
                    static_cast<const LPDWORD>(static_cast<void *>("pong"))[0],
                    static_cast<const LPDWORD>(pvBuffer)[1]
                };
                SendMessage(dnidClient, pong, 8);
            }
            else
            {
                DWORD tick[2] = {
                    static_cast<const LPDWORD>(static_cast<void *>("tick"))[0],
                    ( DWORD )time( NULL )
                };
                SendMessage(dnidClient, tick, 8);
            }
            return;
        }
    }

    // 扩展版本的高级ping处理，主要是增加了对MAC地址的检测，用来限制小号
    else if (wLength == 16)
    {
        // 特殊的ping只能在有mac限制的情况下出现，否则不起作用。
        //if ( m_MacLimit == 0 )
        //    return; 这个限制还是不要，以便兼容客户端！

        // 及时响应ping消息
        if (static_cast<const LPDWORD>(pvBuffer)[0] == 
            static_cast<const LPDWORD>(static_cast<void *>("ping"))[0])
        {
            int ct = 0;

            // 完全排除mac地址无效的情况。
            DNID mac = static_cast<const DNID*>(pvBuffer)[1];
            if ( mac == 0 )
                return;

            __try
            {
                BOOL macConfuse = false;
                EnterCriticalSection(&m_ClientMapLock);

                if ( m_MacLimit != 0 )
                {
                    // mac地址冲突检测
                    std::map<DNID, std::pair<DNID,DNID> >::iterator it = m_MacMap.find( mac ); 
                    if ( it == m_MacMap.end() )
                    {
                        // 第一次收到ping时做MAC地址初始化。。。
                        if ( pLinkData->macAddress == 0 ) 
                        {
                            pLinkData->macAddress = mac;
                            std::pair<DNID,DNID> &data = m_MacMap[mac];
                            data.first = dnidClient;
                            data.second = 0;
                        }
                        else
                        {
                            // 这种情况下属于玩家自行修改了mac地址，应该做断开处理！
                            macConfuse = true;
                        }
                    }
                    else
                    {
                        // 这里属于重复收到一个机器的链接信息，根据配置来进行限制（只能连接一个或只能连接2个）
                        if ( pLinkData->macAddress == 0 ) 
                        {
                            // 重复初始化，一机多开。
                            pLinkData->macAddress = mac;
                            if ( it->second.first == 0 )
                                it->second.first = dnidClient;

                            // 允许同机双开的情况下，找第2个连接
                            else if ( it->second.second == 0 && m_MacLimit == 2 ) 
                                it->second.second = dnidClient;

                            // 如果未通过，则属于玩家一机多开超过限制！需要做断开处理
                            else
                            {
                                // 随机将旧的连接停止响应 33% 不处理， 33%断开第一个，33%断开第2个
                                int rdd = rand() % 30000;
                                if ( rdd > 5000 )
                                {
                                    DNID ckdnid = it->second.first;
                                    if ( m_MacLimit == 2 && rdd > 22500 )
                                        ckdnid = it->second.second;

                                    std::map<DNID, CLinkDataAdv>::iterator it = m_ClientMap.find( ckdnid );
                                    if ( it != m_ClientMap.end() && it->second.GetForceTimeOutTime() == 0 )
                                        it->second.SetForceTimeOutTime( timeGetTime() + rdd % 100000 );
                                }
                                macConfuse = true;
                            }
                        }
                        else
                        {
                            // 在线的定时检测。
                            if ( it->second.first != dnidClient && it->second.second != dnidClient )
                            {
                                // 这种情况下属于玩家自行修改了mac地址，应该做断开处理！
                                macConfuse = true;
                            }
                        }
                    }
                }

                // 如果链接的MAC被修改，则对其做限制，最终会导致超时处理
                if ( !macConfuse )
                {
                    pLinkData->SetTimeOutTime(timeGetTime());
                    pLinkData->CheckPing( static_cast<const LPDWORD>( pvBuffer )[1] );
                    ct = pLinkData->checkTimes ++;
                }

                // 标记非法客户端
                if ( ( pLinkData->flags & 1 ) == 0 && ( static_cast<const LPDWORD>(pvBuffer)[1] & 8 ) == 8 )
                    pLinkData->flags |= ( ( static_cast<const LPDWORD>(pvBuffer)[1] & 8 ) == 8 );
            }
            __finally
            {
                LeaveCriticalSection(&m_ClientMapLock);
            }

            if ( ct & 1 )
            {
                DWORD pong[2] = {
                    static_cast<const LPDWORD>(static_cast<void *>("pong"))[0],
                    static_cast<const LPDWORD>(pvBuffer)[1]
                };
                SendMessage(dnidClient, pong, 8);
            }
            else
            {
                DWORD tick[2] = {
                    static_cast<const LPDWORD>(static_cast<void *>("tick"))[0],
                    ( DWORD )time( NULL )
                };
                SendMessage(dnidClient, tick, 8);
            }

            // 非法客户端，需要通知上层逻辑需要作出相应处理
            if ( wgLimit && ( pLinkData->flags & 1 ) )
            {
                char data[10];
                data[0] = ( BYTE )0xef;
                data[1] = 0;
                *( DNID* )( data + 2 ) = pLinkData->macAddress;
                PushPacket( dnidClient, enumStatus, data, 10, pContext );
            }

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

        if ( pLinkData->macAddress != 0 )
        {
            // 清除注册的mac地址
            std::map<DNID, std::pair<DNID,DNID> >::iterator it = m_MacMap.find( pLinkData->macAddress );
            if ( it != m_MacMap.end() )
            {
                // 清楚的前提是，该链接不是外挂，如果是外挂，则不清除dnid，致使该链接出现同机多链接的状态而导致登录无效
                if ( ( wgLimit & 1 ) && ( pLinkData->flags & 1 ) )
                {
                    // 标记该mac地址是存在问题的，需要强制处理！
                    it->second.first |= 0x8000000000000000;
                    it->second.second |= 0x8000000000000000;
                }
                else
                {
                    if ( it->second.first == dnidClient )
                        it->second.first = 0;
                    else if ( it->second.second == dnidClient ) // 这个地方不判断m_MacLimit，主要是为了兼容处理
                        it->second.second = 0;
                }
            }
        }

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
    if (0 == dnidClient)
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

            if ( pLinkData->GetLinkStatus() == LinkStatus_UnConnected )
                continue;

            if ((int)(dwCurrentTime - pLinkData->GetTimeOutTime()) > TIMEOUTTIME) 
                if ( ( rand() % 10000 ) < 2000 )
                    DelOneClient(pLinkData->GetDnidClient());
                else
                    pLinkData->SetLinkStatus( LinkStatus_UnConnected ); // 超时后不做断开，只做屏蔽接收信息！

            if (pLinkData->GetForceTimeOutTime() != 0)
                if ((int)(dwCurrentTime - pLinkData->GetForceTimeOutTime()) > 0) 
                {
                    if ( ( rand() % 10000 ) < 2000 )
                        DelOneClient(pLinkData->GetDnidClient());
                    else
                        pLinkData->SetLinkStatus( LinkStatus_UnConnected ); // 超时后不做断开，只做屏蔽接收信息！
                }
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

DWORD CNetServerAdv::GetInSizeAndClear(void)
{
    DWORD   dwSize = m_dwInSize;
    m_dwInSize = 0;
    return  dwSize;
}

DWORD CNetServerAdv::GetOutSizeAndClear(void)
{
    DWORD   dwSize = m_dwOutSize;
    m_dwOutSize = 0;
    return  dwSize;
}

void  CNetServerAdv::SetMacLimit( int num ) { m_MacLimit = num; } 

int CNetServerAdv::GetLinkByFlags( DNID *dnids, DWORD size, DWORD flags )
{
    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        DWORD idx = 0;
        for ( std::map<DNID, CLinkDataAdv>::iterator it = m_ClientMap.begin();
            it != m_ClientMap.end() && idx < size; it ++ )
        {
            if ( ( it->second.flags & flags ) == flags )
                dnids[idx++] = it->second.GetDnidClient();
        }
        return idx;
    }
    __finally
    {
        LeaveCriticalSection(&m_ClientMapLock);
    }
}

int CNetServerAdv::GetLinkByMac( DNID *dnids, DWORD size, DNID mac )
{
    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        DWORD idx = 0;
        for ( std::map<DNID, CLinkDataAdv>::iterator it = m_ClientMap.begin();
            it != m_ClientMap.end() && idx < size; it ++ )
        {
            if ( it->second.macAddress == mac )
                dnids[idx++] = it->second.GetDnidClient();
        }
        return idx;
    }
    __finally
    {
        LeaveCriticalSection(&m_ClientMapLock);
    }
}

DNID CNetServerAdv::GetLinkMac( DNID dnid )
{
    __try
    {
        EnterCriticalSection(&m_ClientMapLock);

        std::map<DNID, CLinkDataAdv>::iterator it = m_ClientMap.find( dnid );
        if ( it == m_ClientMap.end() )
            return -1;
        return it->second.macAddress;
    }
    __finally
    {
        LeaveCriticalSection(&m_ClientMapLock);
    }
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
    checkTimes = 0;
    macAddress = 0;

    _minuteChkTicks = timeGetTime();
    _secondChkTicks = timeGetTime();
    _minuteMsgCount = 0;
    _secondMsgCount = 0; 
}

CLinkDataAdv::CLinkDataAdv(DNID dnidClient, LPVOID pContext) : 
    CLinkData(dnidClient)
{
    m_pContext = pContext;
    checkTimes = 0;
    macAddress = 0;

    _minuteChkTicks = timeGetTime();
    _secondChkTicks = timeGetTime();
    _minuteMsgCount = 0;
    _secondMsgCount = 0; 
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