#pragma once

#include "networkmodule\netconsumermodule\netconsumermodule.h"
#define DEFAULT_ACCOUNT_PREFIX "default"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CNetAccountCheckModule
{
public:
    // 这里做基本的套接字操作，数据的接收与发送，相应函数的调用
    int Execution(void);
    void Destroy();

    bool Connect( LPCSTR ip, LPCSTR port );
    bool Entry( LPCVOID data, size_t size );

public:
    void RecvCheckAccountMsg( struct SACheckAccountMsg *pMsg );

    void RecvChangePasswordMsg( struct SAChangePasswordMsg *pMsg );
    void RecvBlockAccountMsg( struct SABAMsg *pMsg );
    void RecvGetOnlineTimeMsg( struct SAGOTMsg *pMsg );

    // void RecvGMLoginMsg( struct SAGMCheckAccountMsg *pMsg );

    void RecvRefreshAccountMsg( struct SARefreshCardMsg *pMsg );
	void RecvRefreshMoneyPointMsg( struct SALRefreshMoneyPointMsg *pMsg );
    void RecvQueryEncouragementMsg( struct SAQueryEncouragement *pMsg );
	void RecvCheckPlayerExistMsg( struct SQCheckPlayerExistBaseMsg *pMsg );

    void ProcessInterop( void *data );
	BOOL Reconnect();

public:
    CNetAccountCheckModule();
	CNetAccountCheckModule( LPCSTR prefix ) : accountPrefix( prefix ){  dwPrev = timeGetTime(); };
    ~CNetAccountCheckModule();

public:
    CNetConsumerModule client;
	std::string accountPrefix;  // 当前帐号模块对应的服务器前缀名，如果该值为empty，则说明是默认的帐号服务器

private:
    std::string accountServerIP;
    std::string accountServerPort;
	DWORD dwPrev;

public:
    // 排队系统这里这个地方会使用所有就放在这里了
    // 这里一个结构大约为500 现在假定排队玩家为 2000 * 500≈1M 那么占用 1M 数据空间
    struct SQueueData
    {
        std::string account;
        std::string password;
        std::string userpass;
        std::string idkey;
        DWORD online;
        DWORD offline;
        DWORD limitedState;
        DNID dnidClient;
        QWORD puid;
    };

    // 登陆队列
    std::list< SQueueData > loginQueueList;

    // 更新排队玩家
    void UpdateQueuePlayer();
	void UpdateLoginPlayer(); //更新在区域服务器中处理的玩家
	void PopLoginPlayer(std::string strcount);
// 	//玩家登录时在登录服务器保存，在区域服务器进入完成后在删除，以减缓区域服务器的登录压力
// 	std::vector<std::string> playerloginlist;
};

