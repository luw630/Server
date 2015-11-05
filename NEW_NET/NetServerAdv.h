//---------------------------------------------------------------------------
//  Engine (c) 1999-2002 by Dreamwork
//
// File:	CNetServerAdv.h
// Date:	2002.07.10, 2003.04.23
// Code:	qiuxin, luolin
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef	ContextNetServerEx_H
#define	ContextNetServerEx_H
//---------------------------------------------------------------------------

#include "NetServerEx.h"

#include <list>

class CLinkDataAdv : 
    public CLinkData 
{
public:
    LPVOID GetContext();
    BOOL SetContext(LPVOID pContext);

public:
    CLinkDataAdv();
    CLinkDataAdv(DNID dnidClient, LPVOID pContext);
    ~CLinkDataAdv();

private:
    LPVOID      m_pContext;             // 外部附加的数据

public:
    DWORD       checkTimes;
    DNID        macAddress;

    DWORD       _minuteChkTicks;
    DWORD       _secondChkTicks;
    DWORD       _minuteMsgCount;    // 用于限制客户端，一段时间内不能发送超过一定数量的网络消息请求，一旦超过预订请求，则做无效连接处理
    DWORD       _secondMsgCount;    // 特殊处理，除了对象请求可以在1分钟收取2~3k外，其他消息都只能10秒内收取20～30次
};

/*
class CNetServerPacketAdv : 
    public CNetServerPacket
{
public:
    LPVOID GetContext();

public:
    void operator = (const CNetServerPacketAdv &Packet);

public:
    CNetServerPacketAdv();
    CNetServerPacketAdv(const CNetServerPacketAdv &Packet);
    CNetServerPacketAdv(DNID dnidClient, LINKSTATUS enumStatus, LPVOID pvBuffer, WORD wLength, LPVOID pContext);
    ~CNetServerPacketAdv();

private:
    LPVOID m_pContext;    // 外部附加的数据
};
*/

struct SNetServerPacketAdv : 
    public SNetServerPacket
{
    LPVOID m_pContext;    // 外部附加的数据
};

//---------------------------------------------------------------------------

// 没有考虑让这个类继续作派生，所以去掉了所有的 "virtual"

class CNetServerAdv : 
    public iDirectNetServerEventHandler // declared inside "DirectNetServer.h"
{
public:
    // create the DreamNetServer object, initialize it, and host the session
	BOOL Create(const char *szPortName); 

    // invoke IDreamNetServer::Close() internally
	BOOL Close(); 

    // invoke IDreamNetServer::SendTo internally
	BOOL SendMessage(DNID dnidClient, const void *pvBuf, WORD wSize); 

    // invoke IDreamNetServer::DeletePlayer internally
	BOOL DelOneClient(DNID dnidClient); 

    // get a packet from packet-queue
    // BOOL GetMessage(CNetServerPacketAdv &Packet); 
    BOOL GetMessage(SNetServerPacketAdv **ppPacket); 

    // invoke IDreamNetServer::DeletePlayer internally
    BOOL GetDnidAddr(DNID dnidClient, sockaddr_in *pAddr, size_t size);

    // extra send without encode
    BOOL SendDirect(DNID dnidClient, LPVOID pPackage, WORD wSize);

public:
    // check the dnidClient is Valid
    BOOL IsValidDnid(DNID dnidClient);

    // return the current client number
	DWORD GetClientNum();
	DWORD GetClientNum_Dll();

    // extra operation ...
    LINKSTATUS GetLinkStatus(DNID dnidClient);
    BOOL SetLinkStatus(DNID dnidClient, LINKSTATUS enumStatus);

    LPVOID GetDefaultContext();
    BOOL SetDefaultContext(LPVOID pContext);

    LPVOID GetLinkContext(DNID dnidClient);
    BOOL SetLinkContext(DNID dnidClient, LPVOID pContext);

    BOOL SetLinkForceTimeOutTime(DNID dnidClient, DWORD dwTime);
    BOOL ClearLinkForceTimeOutTime(DNID dnidClient);

    void CheckAlive();

    DWORD GetPacketNumber();

    BOOL SetOption(LPDWORD pParam);

    // 取得流量并且清零
    DWORD GetInSizeAndClear(void);
    DWORD GetOutSizeAndClear(void);
    void  SetMacLimit( int num );

    DNID GetLinkMac( DNID dnid );
    int GetLinkByMac( DNID *dnids, DWORD size, DNID mac );
    int GetLinkByFlags( DNID *dnids, DWORD size, DWORD flags );

private:
    // iDirectEventHandler implementation
    void OnCreatePlayer(DNID dnidClient, PVOID *ppvClientContext);
    void OnReceivePacket(DNID dnidClient, PVOID pvClientContext, PVOID pvBuffer, DWORD dwLength, DWORD dwSequence);
    void OnDeletePlayer(DNID dnidClient, PVOID pvClientContext);

private:
    BOOL CheckIdleTime(DNID dnidClient);
    CLinkDataAdv *NewLinkData(DNID dnidClient);
    BOOL PushPacket(DNID dnidClient, LINKSTATUS enumStatus, LPVOID pvBuffer, WORD wLength, LPVOID pContext);

public:
    // contruct and destruct
    CNetServerAdv(BOOL bUseVerify = FALSE);
    ~CNetServerAdv(void); 

private:
    const BOOL m_bUseVerify;

    LPVOID m_pDefaultContext;

    iDirectNetServer *m_pDirectNetServer;

	DWORD m_dwPrevCheckTime;	                    // 上次做CheckAlive的时间

    CRITICAL_SECTION m_ClientMapLock;               // 连接数据映射表互斥锁

    CRITICAL_SECTION m_PacketQueueLock;             // 封包队列互斥锁

    std::map<DNID, CLinkDataAdv> m_ClientMap;       // 连接数据映射表

    // std::queue<CNetServerPacketAdv> m_PacketQueue;  // 封包队列
    std::queue<SNetServerPacketAdv *> m_PacketQueue;  // 封包队列
    //std::list<SNetServerPacketAdv *> m_PacketQueue;  // 封包队列

    DWORD m_dwInSize;
    DWORD m_dwOutSize;
    DWORD m_dwInNumber;
    DWORD m_dwOutNumber;

    // MAC地址映射表，主要是为了解决小号和一机多开的问题（因为是使用ping来处理，所以是一个及时性较高的处理方式）
    std::map<DNID, std::pair<DNID,DNID> > m_MacMap;
    DWORD m_MacLimit; // 一机多开的限制，只能为0、1、2，0表示无限制，1为只能开1个，2为只能开2个。
};

//---------------------------------------------------------------------------
#endif
