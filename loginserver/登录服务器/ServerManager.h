#pragma once

#include "NetWorkModule/NetProviderModule/NetProviderModule.h"
#include "NetWorkModule/playertypedef.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// 服务器相关数据
struct SServer
{
    std::string account;          // 该集群单元的名称（账号） [MAX_ACCOUNT]
    std::string description;      // 该集群单元的说明         [MAX_USERDESC]

    DWORD   dwIP;                   // 用于接受外部联接的地址
    DWORD   dwPort;                 // 用于接受外部联接的端口

    DNID    idControl;              // 本次执行中唯一的全局ID（网络底层）

    DWORD   ID;                     // 本次执行中的关键字编号（关键字）
    DWORD   gid;                    // 本次执行中唯一的全局ID（整个服务器全局）\
    
    WORD    wTotalPlayers;          // 在这个服务器上的玩家个数
    WORD    wMaxPlayers;            // 在这个服务器上曾经最大的同时在线数
    WORD    wPlayers[4];            // 各个状态的玩家个数

    // 采集到的数据
    __int64 i64TotalSpendMoney;     // 消耗的金钱总额
    DWORD   dwAvePing;              // 平均ping值
    DWORD   dwInFlowPS;             // 每秒流量
    DWORD   dwOutFlowPS;            // 每秒流量
    DWORD   dwCreatedSpecItem;      // 产生的特殊物品数量
    DWORD   dwUseSpecItem;          // 消耗的特殊物品数量
    BOOL    bQuestFullData;         // 是否请求成功过全固定场景数据？

    std::map<WORD, SRegion> Regions;// 场景列表

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

    // 处理内部分服务器来的消息
    bool Entry( DNID dnidClient, LINKSTATUS enumStatus, void *pMsg, size_t size ); 
    BOOL OnServerLogin(DNID dnidClient, struct SQLoginMsg *pLog);
    BOOL RefreshRegionServer(struct SARefreshRegionServerMsg *pRf, DNID dnidClient);             // 根据具体的消息刷新本地数据
    BOOL RefreshRegionServerEx(struct SARefreshRegionServerMsgEx *pRf, DNID dnidClient);             // 根据具体的消息刷新本地数据
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
    CNetProviderModule server;          // 玩家的服务器

private:
    std::map<DNID, SServer> gameServers;    // 场景列表

    DWORD m_dwCurPlayerNumber;
    DWORD max_players;
public:
    WORD m_wQueuePlayerCount;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


