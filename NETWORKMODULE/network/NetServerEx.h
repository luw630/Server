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
	LinkStatus_UnConnected,         // ��Ч������״̬
	LinkStatus_Waiting_Login,	    // ���Ӻ�ȴ���֤��Ϣ��״̬
	LinkStatus_Login,	            // ���Ӻ�ȴ���֤�����״̬����Ϊ�п�����Ҫ�ύ��֤���ݵ������ط���
	LinkStatus_Connected,           // ���Ӻ���֤ͨ������������ʹ�õ�״̬
	LinkStatus_Disconnected,        // �����ѶϿ���״̬
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
    DNID        m_dnidClient;           // ��"DirectNetServer"�ײ�ƥ������ӱ��

private:
    DWORD       m_dwXorCipher;          // �ϲ��������
    LINKSTATUS  m_enumStatus;           // �����ӵĵ�ǰ״̬

private:
	DWORD       m_dwTimeOutTime;        // ���г�ʱ�������

private:
	DWORD       m_dwForceTimeOutTime;   // ǿ�Ƴ�ʱ������ݣ��������������Ϸ�У���Ҫǿ���趨һ�����Ӷ�ú�ʱ

private:
	DWORD       m_dwSegPing_ClientTime; // ��һ�ι�����ping�еĿͻ���ʱ��
	DWORD       m_dwSegPing_ServerTime; // �յ���һ��ping�ķ�����ʱ��
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

// û�п�������������������������ȥ�������е� "virtual"

class CNetServerEx : 
    public iDirectNetServerEventHandler // declared inside "DirectNetServer.h"
{
public:
    // create the DreamNetServer object, initialize it, and host the session
	BOOL Create(const char *szPortName); 
    BOOL Create(const WORD wPort      ); // û�м���

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

	DWORD m_dwPrevCheckTime;	                    // �ϴ���CheckAlive��ʱ��

    CRITICAL_SECTION m_ClientMapLock;               // ��������ӳ�������

    CRITICAL_SECTION m_PacketQueueLock;             // ������л�����

    std::map<DNID, CLinkData> m_ClientMap;          // ��������ӳ���

    // std::queue<CNetServerPacket> m_PacketQueue;     // �������
    std::queue<SNetServerPacket *> m_PacketQueue;     // �������
};

//---------------------------------------------------------------------------
#endif
