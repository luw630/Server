#pragma once

#include "NetWorkModule/NetProviderModule/NetProviderModule.h"
#include "NetWorkModule/playertypedef.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// �������������
struct SServer
{
    std::string account;          // �ü�Ⱥ��Ԫ�����ƣ��˺ţ� [MAX_ACCOUNT]
    std::string description;      // �ü�Ⱥ��Ԫ��˵��         [MAX_USERDESC]

    DWORD   dwIP;                   // ���ڽ����ⲿ���ӵĵ�ַ
    DWORD   dwPort;                 // ���ڽ����ⲿ���ӵĶ˿�

    DNID    idControl;              // ����ִ����Ψһ��ȫ��ID������ײ㣩

    DWORD   ID;                     // ����ִ���еĹؼ��ֱ�ţ��ؼ��֣�
    DWORD   gid;                    // ����ִ����Ψһ��ȫ��ID������������ȫ�֣�\
    
    WORD    wTotalPlayers;          // ������������ϵ���Ҹ���
    WORD    wMaxPlayers;            // ���������������������ͬʱ������
    WORD    wPlayers[4];            // ����״̬����Ҹ���

    // �ɼ���������
    __int64 i64TotalSpendMoney;     // ���ĵĽ�Ǯ�ܶ�
    DWORD   dwAvePing;              // ƽ��pingֵ
    DWORD   dwInFlowPS;             // ÿ������
    DWORD   dwOutFlowPS;            // ÿ������
    DWORD   dwCreatedSpecItem;      // ������������Ʒ����
    DWORD   dwUseSpecItem;          // ���ĵ�������Ʒ����
    BOOL    bQuestFullData;         // �Ƿ�����ɹ���ȫ�̶��������ݣ�

    std::map<WORD, SRegion> Regions;// �����б�

    SServer(void) : i64TotalSpendMoney(0),dwAvePing(0),dwInFlowPS(0),dwOutFlowPS(0),
                    dwCreatedSpecItem(0), dwUseSpecItem(0), bQuestFullData(0)
    {
    }
};

class CServerManager
{
public:
    BOOL Host( LPCSTR port );

public:
    void Execution();
    void Destroy();
    void DisplayInfo();

    // �����ڲ��ַ�����������Ϣ
    bool Entry( DNID dnidClient, LINKSTATUS enumStatus, void *pMsg, size_t size ); 
    BOOL OnServerLogin(DNID dnidClient, struct SQLoginMsg *pLog);
    BOOL RefreshRegionServer(struct SARefreshRegionServerMsg *pRf, DNID dnidClient);             // ���ݾ������Ϣˢ�±�������
    BOOL RefreshRegionServerEx(struct SARefreshRegionServerMsgEx *pRf, DNID dnidClient);             // ���ݾ������Ϣˢ�±�������
    BOOL SendPlayerData(DNID dnidClient, struct SGetPlayerDataMsg *pScm);
    void ResolutionRegion(DNID dnidClient, struct SQResolutionDestRegionMsg *pScm);

public:
    BOOL PutIntoRegion( DNID dnidClient, LPCSTR account, DWORD gid, DWORD regionId, DWORD &serverKey );
    BOOL GetServerInfo( DWORD ID, DNID &dnid, DWORD &ip, DWORD &port );
    BOOL GetRegionInfo( DWORD ID, WORD &mapid, WORD &level );
    BOOL QueryRefreshGameServer();

public:
    void Broadcast(const void *pMsg, size_t size );
    void SendFirst( void *pMsg, size_t size );
    void GetPlayerNumber( DWORD &dwMaxCount, DWORD &dwOnlineCount );
	DWORD GetServerNumber() { return ( DWORD )gameServers.size(); }
	int GetPlayerNumber(BYTE bstate);
public:
    CServerManager();

public:
    CNetProviderModule server;          // ��ҵķ�����

private:
    std::map<DNID, SServer> gameServers;    // �����б�

    DWORD m_dwCurPlayerNumber;
    DWORD max_players;
public:
    WORD m_wQueuePlayerCount;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


