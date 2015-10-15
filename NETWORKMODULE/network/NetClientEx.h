//---------------------------------------------------------------------------
//  Engine (c) 1999-2002 by Dreamwork
//
// File:    CNetClientEx.h
// Date:    2002.07.10
// Code:    Qiuxin
// Desc:    Header File
//---------------------------------------------------------------------------
#ifndef NetClientEx_H
#define NetClientEx_H
//---------------------------------------------------------------------------
#include <queue>

#include "DirectNet.h"

/*
class CNetClientPacket
{
public:
    LPVOID GetBufferEntry();
    WORD GetBufferLength();

public:
    void operator = (const CNetClientPacket &Packet);

private:
    void ReleaseBuffer();

public:
    CNetClientPacket();
    CNetClientPacket(const CNetClientPacket &Packet);
    CNetClientPacket(LPVOID pvBuffer, WORD wLength);
    ~CNetClientPacket();

private:
    LPVOID      m_pvBuffer;
    WORD        m_wLength;
};
*/

struct SNetClientPacket
{
    LPVOID  m_pvBuffer;
    WORD    m_wLength;
};

//---------------------------------------------------------------------------

class CNetClientEx : 
    public iDirectNetClientEventHandler // declared inside "DirectNetClient.h"
{
public:
    enum LSCLIENT
    {
        LS_UnConnected,         // ��Ч������״̬
        LS_Connected,           // ���Ӻ��������ʹ�õ�״̬
        LS_Disconnected,        // �����ѶϿ���״̬
    };

public:
    BOOL Connect(const char *szIP, const char *szPortName, BOOL bBlock = TRUE);
    BOOL Disconnect(); 

    BOOL SendMessage(const void *pvBuf, WORD wSize);
    // BOOL GetMessage(CNetClientPacket &Packet);
    BOOL GetMessage(SNetClientPacket **ppPacket);

public:
    BOOL IsConnected();
    void KeepAlive();
    DWORD GetPing();
    DWORD GetPacketNumber();

    BOOL SetOption(LPDWORD pParam);
    BOOL IsWaitingConnect();

private:
    // iDirectEventHandler implementation
    void OnConnectComplete(DNRESULT dnr);
    void OnReceivePacket(PVOID pvBuffer, DWORD dwLength, DWORD dwSequence);
    void OnTerminateSession();

private:
    BOOL PushPacket(LPVOID pvBuffer, WORD wLength);

    BOOL CheckGuardArray();

public:
    CNetClientEx(BOOL bUseVerify = FALSE);
    ~CNetClientEx();

private:
    const BOOL m_bUseVerify;

    iDirectNetClient *m_pDirectNetClient;

    HANDLE m_hConnectEvent;

    LSCLIENT m_enumStatus;                          // �����ӵĵ�ǰ״̬

    DWORD m_dwKey;

    DWORD volatile m_dwPing;                        // pingֵ
    DWORD m_dwPrevPingTime;                         // �ϴ�ping��ʱ��

    CRITICAL_SECTION m_PacketQueueLock;             // ������л�����

    //std::queue<CNetClientPacket> m_PacketQueue;     // �������
    std::queue<SNetClientPacket *> m_PacketQueue;     // �������

    BYTE m_GuardArrayF[64];
    BYTE m_SendBuffer[0xffff];
    BYTE m_GuardArrayB[64];

    BOOL m_bWaitingConnect;
};

//---------------------------------------------------------------------------
#endif
