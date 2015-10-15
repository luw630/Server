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
    LPVOID      m_pContext;             // �ⲿ���ӵ�����
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
    LPVOID m_pContext;    // �ⲿ���ӵ�����
};
*/

struct SNetServerPacketAdv : 
    public SNetServerPacket
{
    LPVOID m_pContext;    // �ⲿ���ӵ�����
};

//---------------------------------------------------------------------------

// û�п�������������������������ȥ�������е� "virtual"

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

    // ȡ��������������
    DWORD   GetInSizeAndClear(void);
    DWORD   GetOutSizeAndClear(void);
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

	DWORD m_dwPrevCheckTime;	                    // �ϴ���CheckAlive��ʱ��

    CRITICAL_SECTION m_ClientMapLock;               // ��������ӳ�������

    CRITICAL_SECTION m_PacketQueueLock;             // ������л�����

    std::map<DNID, CLinkDataAdv> m_ClientMap;       // ��������ӳ���

    // std::queue<CNetServerPacketAdv> m_PacketQueue;  // �������
    std::queue<SNetServerPacketAdv *> m_PacketQueue;  // �������
    //std::list<SNetServerPacketAdv *> m_PacketQueue;  // �������

    DWORD m_dwInSize;
    DWORD m_dwOutSize;
    DWORD m_dwInNumber;
    DWORD m_dwOutNumber;
};

//---------------------------------------------------------------------------
#endif
