#pragma once

#include "NetWorkModule/NetProviderModule/NetProviderModule.h"
#include "NetWorkModule/playertypedef.h"
#include <string>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// 玩家相关数据
struct SPlayer
{
    SPlayer()
    {
        memset(this, 0, sizeof(SPlayer));
    }

	DWORD	dwGlobalID;				// 本次执行中唯一的全局ID
    DWORD	dwRegionServerKey;		// 当前所在[场景服务器]的关键字
    BYTE	byIsLostLink;			// 是否断线标志
    BYTE	byIsRebinding;			// 是否在连接重定向中

    std::string account;			// 该玩家的账号（关键字）
    std::string password;			// 密码
    std::string ip;					// IP
	std::string idkey;				// 身份证信息

    SFixProperty	Data;			// 玩家数据
	SFixData fixData; //三国玩家数据
	SPlayerTempData TempData;		// 

    DWORD dwNextCheckExtra;			// 下次额外扣点时间
    DWORD dwNextCheckTime;			// 下一次检测账号的时间
    DWORD dwRebindSegTime;			// 开始重定向的时间，超时后将其剔除
    DWORD dwLoginSegment;			// 玩家在本次游戏中的登入时间（用于防沉迷系统）
    DWORD limitedState;				// 是否纳入防沉迷系统
    DWORD online;
    DWORD offline;
    QWORD puid;
};

// 最初联接在登陆服务器上的玩家联接数据
struct SLinkContextInfo
{
    std::string account;
    std::string password;
    std::string userpass;
	std::string idkey;

    int gmlevel;
    BOOL doUnbind;
    DWORD limitedState;         // 是否纳入防沉迷系统
    DWORD online;				// 
    DWORD offline;				// 
    QWORD puid;
};

struct SCplayerConfig
{
	std::string straccount;
	std::string strconfig;
};

class CPlayerManager
{
public:
    BOOL Host( LPCSTR port );

public:
    void Execution();
    void Destroy();
    void DisplayInfo();

    // 当某个游戏区域服务器down掉之后，需要将其上的数据保存且退出！
    void OnServerShutdown( DWORD serverId );
    LPCSTR GetAccountByDnid( DNID dnid );
    LPCSTR GetUserPassByDnid( DNID dnid );
    BOOL RegisterContext( DNID dnid, LPCSTR account, LPCSTR password, LPCSTR userpass, DWORD limitedState, int gmlevel, LPCSTR idkey, DWORD online, DWORD offline, QWORD puid );
    BOOL ActivateLinkContextWithRebind( DNID dnid );
    BOOL GetDnidContext( DNID dnid, LPCSTR &account, LPCSTR &password, LPCSTR &userpass );
    BOOL UpdatePlayerData( struct SARefreshPlayerMsg *pMsg );
	BOOL DoubleUpdatePlayerData( struct SDoubleSavePlayerMsg *pMsg );
    BOOL DoCheckRebind( DNID dnid, struct SQCheckRebindMsg *pMsg );
	BOOL GetPlayerData(LPCSTR account, DWORD gid, struct SFixData *data, struct SPlayerTempData *tdata);
    BOOL ClearPlayerInCache( LPCSTR account );
    BOOL AccountExist( LPCSTR account );
    BOOL SetPlayerServerKey( LPCSTR account, DWORD ID );

public:
    BOOL RebindAccount( DNID dnidClient, LPCSTR account, DWORD limitedState, LPCSTR idkey, DWORD online, DWORD offline );
	BOOL AssignPlayer(DNID dnidClient, SFixData *data);

public:
    // 处理客户端来的消息
    bool Entry( DNID dnidClient, LINKSTATUS enumStatus, void *pMsg, size_t size ); 

    BOOL OnPlayerLogin  ( DNID dnidClient, struct SQLoginMsg *pMsg     );
    BOOL SelectCharacter( DNID dnidClient, struct SQSelPlayerMsg *pMsg );
    BOOL CreateCharacter( DNID dnidClient, struct SQCrePlayerMsg *pMsg );
    BOOL DelCharacter   ( DNID dnidClient, struct SQDelPlayerMsg *pMsg );

	BOOL OnCheckWhiteList(DNID dnidClient, LPCSTR	account, LPCSTR	password);
	BOOL OnCheckWhiteList(DNID dnidClient, LPCSTR	streamData);
	BOOL  OnCheckAccountServer(DNID dnidClient, LPCSTR	account, LPCSTR	password,BOOL  bCheckWhiteList);
	BOOL  OnCheckAccountServer(DNID dnidClient, BOOL  bCheckWhiteList, struct SACheckWhiteList    *pMsg);

    BOOL PlayerLogout   ( LPCSTR szAccount, LPCSTR szPassword, SPlayer *player );
    BOOL PlayerHangup   ( LPCSTR szAccount, LPCSTR szPassword, BYTE pot, SPlayer *player );
	BOOL SavePlayerData(LPCSTR account, SFixData &PlayerData, int storeFlag);
	
	BOOL SavePlayerConfig (struct SQSavePlayerConfigInfo *pMsg   );
	BOOL SavePlayerConfig (LPCSTR szAccount, LPCSTR szconfig  );
	BOOL SendPlayerConfig (LPCSTR szAccount);	//发送玩家配置
	BOOL CleanPlayerConfig (LPCSTR szAccount);	//清除保存的玩家配置
    void SaveAllPlayer();

public:
    // for GM module
    template < class _InIt, class _Pr > 
    inline _InIt find_if( _InIt _First, _InIt _Last, _Pr _Pred )
    {
	    for ( ; _First != _Last; ++_First )
		    if ( _Pred(_First) )
			    break;
	    return (_First);
	}

    template < typename operation >
    SPlayer *FindPlayer( operation op )
    {
        std::map< std::string, SPlayer >::iterator it = 
            CPlayerManager::find_if( playerCache.begin(), playerCache.end(), op );
        if ( it == playerCache.end() )
            return NULL;

        return &it->second;
    }

    inline SPlayer *FindPlayer( LPCSTR account )
    {
        std::map< std::string, SPlayer >::iterator it = playerCache.find( account );
        if ( it == playerCache.end() )
            return NULL;

        return &it->second;
    }

public:
    CPlayerManager();

public:
    // 管理玩家联接的服务端
    CNetProviderModule server;          

private:
    // 玩家[用户]在联接服务器后，临时记录的一系列用于登陆操作的数据
    std::map< DNID, SLinkContextInfo > tempLinkContext;

    // 玩家[用户]在成功选择角色[进入场景]后，在登陆服务器上保存的一个备份，区域服务器主动定时请求更新备份
    std::map< std::string, SPlayer > playerCache;

	std::vector<SCplayerConfig> playercon;


public:
    // 是否限制登陆
    BOOL m_bCanLogin;
	int m_onlineNumber;

    int  GetPlayerCacheCount() { return ( int )tempLinkContext.size() + m_onlineNumber; }
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


