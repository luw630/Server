//---------------------------------------------------------------------------
//  Engine (c) 1999-2002 by Dreamwork
//
// File:	CNetServerEx.h
// Date:	2002.07.10, 2003.04.23
// Code:	qiuxin, luolin
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef	NetServer_H
#define	NetServer_H
//---------------------------------------------------------------------------
#include <map>
#include <queue>

#include "DirectNet.h"

#define  MAX_CLIENT_NUMBER   3000

enum LINKSTATUS
{
	LinkStatus_UnConnected,         // 无效的连接状态
	LinkStatus_Waiting_Login,	    // 连接后等待认证消息的状态
	LinkStatus_Login,	            // 连接后等待认证结果的状态（因为有可能需要提交认证数据到其他地方）
	LinkStatus_Connected,           // 连接后认证通过，可以正常使用的状态
	LinkStatus_Disconnected,        // 连接已断开的状态
};

class CLinkData
{
public:
    LINKSTATUS GetLinkStatus();
    BOOL SetLinkStatus(LINKSTATUS enumStatus);

public:
    DWORD GetXorCipher();
    BOOL SetXorCipher(DWORD dwXorCipher);

public:
    BOOL SetTimeOutTime(DWORD dwNewTime);
    DWORD GetTimeOutTime();

public:
    BOOL SetForceTimeOutTime(DWORD dwNewTime = 0xffffffff);
    DWORD GetForceTimeOutTime();

public:
    void CheckPing( DWORD clientTime );

public:
    DNID GetDnidClient();

public:
    CLinkData();
    CLinkData(DNID dnidClient);
    ~CLinkData();

private:
    DNID        m_dnidClient;           // 和"DirectNetServer"底层匹配的连接编号

private:
    DWORD       m_dwXorCipher;          // 上层加密数据
    LINKSTATUS  m_enumStatus;           // 该连接的当前状态

private:
	DWORD       m_dwTimeOutTime;        // 空闲超时检测数据

private:
	DWORD       m_dwForceTimeOutTime;   // 强制超时检测数据，这个数据用于游戏中，需要强制设定一个连接多久后超时

private:
	DWORD       m_dwSegPing_ClientTime; // 第一次过来的ping中的客户端时间
	DWORD       m_dwSegPing_ServerTime; // 收到第一次ping的服务器时间
};

/*
class CNetServerPacket
{
public:
    DNID GetLinkDnid();
    LINKSTATUS GetLinkStatus();
    LPVOID GetBufferEntry();
    WORD GetBufferLength();

public:
    void operator = (const CNetServerPacket &Packet);

private:
    void ReleaseBuffer();

public:
    CNetServerPacket();
    CNetServerPacket(const CNetServerPacket &Packet);
    CNetServerPacket(DNID dnidClient, LINKSTATUS enumStatus, LPVOID pvBuffer, WORD wLength);
    ~CNetServerPacket();

private:
    DNID        m_dnidClient;
    LINKSTATUS  m_enumStatus;
    LPVOID      m_pvBuffer;
    WORD        m_wLength;
};
*/

struct SNetServerPacket
{
    DNID        m_dnidClient;
    LINKSTATUS  m_enumStatus;
    LPVOID      m_pvBuffer;
    WORD        m_wLength;
};

//---------------------------------------------------------------------------

// 没有考虑让这个类继续作派生，所以去掉了所有的 "virtual"

class CNetServerEx : 
    public iDirectNetServerEventHandler // declared inside "DirectNetServer.h"
{
public:
    // create the DreamNetServer object, initialize it, and host the session
	BOOL Create(const char *szPortName); 
    BOOL Create(const WORD wPort      ); // 没有加密

    // invoke IDreamNetServer::Close() internally
	BOOL Close(); 

    // invoke IDreamNetServer::SendTo internally
	BOOL SendMessage(DNID dnidClient, const void *pvBuf, WORD wSize); 

    // invoke IDreamNetServer::DeletePlayer internally
	BOOL DelOneClient(DNID dnidClient); 

    // get a packet from packet-queue
    // BOOL GetMessage(CNetServerPacket &Packet); 
    BOOL GetMessage(SNetServerPacket **ppPacket); 

    // invoke IDreamNetServer::DeletePlayer internally
    BOOL GetDnidAddr(DNID dnidClient, sockaddr_in *pAddr, size_t size);

public:
    // check the dnidClient is Valid
    BOOL IsValidDnid(DNID dnidClient);

    // return the current client number
	DWORD GetClientNum();

    // extra operation ...
    LINKSTATUS GetLinkStatus(DNID dnidClient);
    BOOL SetLinkStatus(DNID dnidClient, LINKSTATUS enumStatus);

    // extra operation ...
    BOOL SetLinkForceTimeOutTime(DNID dnidClient, DWORD dwTime);
    BOOL ClearLinkForceTimeOutTime(DNID dnidClient);

    void CheckAlive();

    BOOL SetOption(LPDWORD pParam);

private:
    // iDirectEventHandler implementation
    void OnCreatePlayer(DNID dnidClient, PVOID *ppvClientContext);
    void OnReceivePacket(DNID dnidClient, PVOID pvClientContext, PVOID pvBuffer, DWORD dwLength, DWORD dwSequence);
    void OnDeletePlayer(DNID dnidClient, PVOID pvClientContext);

private:
    BOOL CheckIdleTime(DNID dnidClient);
    CLinkData *NewLinkData(DNID dnidClient);
    BOOL PushPacket(DNID dnidClient, LINKSTATUS enumStatus, LPVOID pvBuffer, WORD wLength);

public:
    // contruct and destruct
    CNetServerEx(BOOL bUseVerify = FALSE);
    ~CNetServerEx(void); 

private:
    const BOOL m_bUseVerify;

    iDirectNetServer *m_pDirectNetServer;

	DWORD m_dwPrevCheckTime;	                    // 上次做CheckAlive的时间

    CRITICAL_SECTION m_ClientMapLock;               // 连接数据映射表互斥锁

    CRITICAL_SECTION m_PacketQueueLock;             // 封包队列互斥锁

    std::map<DNID, CLinkData> m_ClientMap;          // 连接数据映射表

    // std::queue<CNetServerPacket> m_PacketQueue;     // 封包队列
    std::queue<SNetServerPacket *> m_PacketQueue;     // 封包队列
};

//---------------------------------------------------------------------------
#endif
