#pragma once

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
typedef unsigned __int64 DNID;
#include "networkmodule/playertypedef.h"
//struct SFixProperty;
struct SQCheckRebindMsg;
struct SARefreshPlayerMsg;
struct SAGMCheckAccountMsg;
//struct SPlayerTempData;
struct SGMMMsg;
struct SCollect_LoginServerData;
struct SMailBaseMsg; 
struct SQUpdateTimeLimit;
struct SARefreshFactionMsg;
struct SQBuildingSaveMsg;
struct SABuildingGetMsg;
struct SDoubleSavePlayerMsg;

// 玩家相关数据
namespace Globals
{
    // 各个门派的出生点
    struct SBornPos
    {
        WORD wRegionID;
        WORD wPosX;
        WORD wPosY;
		BYTE byDir;
		WORD wRadius;
    };

    // 服务器状态，用于服务器状态查询
    enum eServerStatus
    {
        SS_NORMAL,  // 服务器正常
        SS_CLOSE,   // 服务器未启动
        SS_REJECT,  // 服务器拒绝登陆
        SS_PROBLEM, // 服务器存在问题
    };

    // 读取和保存配置数据
    BOOL LoadConfigure();
    BOOL SaveConfigure();
    BOOL LoadDbcList( BOOL reset = false );
    void DbcExec();
    void DbcDestroy();
    void DbcInfo();

    void GetDatabaseIPPORT( LPCSTR &ip, LPCSTR &port );
    void GetAccountIPPORT( LPCSTR &ip, LPCSTR &port );
    void GetDBCenterIPPORT( LPCSTR &ip, LPCSTR &port );
	void GetCenterServerIPPORT(LPCSTR &ip, LPCSTR &port);
	void GetServerAccount(LPCSTR &account);
	void GetServerIPPORT(LPCSTR &ip, int &nport);

    void GetGameServerPort( LPCSTR &port );
    void GetPlayerServerPort( LPCSTR &port );
	void GetGMServerPort(LPCSTR &port);
	void GetCollectPort(LPCSTR &port);

    BOOL GetChargeMode();

    BOOL SetPlayerLinkContext( DNID dnid, LPCSTR account, LPCSTR password, LPCSTR userpass, DWORD limitedState, int gmlevel, LPCSTR idkey, DWORD online, DWORD offline, QWORD puid );
    BOOL SetPlayerLinkForceTimeOutTime( DNID dnid, DWORD dwTime );
    BOOL SetPlayerLinkStatus( DNID dnid, int state );
    BOOL ActivateLinkContextWithRebind( DNID dnid );
    BOOL CutPlayerLink( DNID dnid );
    BOOL IsValidPlayerDnid( DNID dnid );

    //BOOL SendToAccountServer( void *msg, size_t size );
    // 新添加参数server,指明需要发送数据到哪个帐号服务器,为NULL时表示发送到默认帐号服务器,为-1时表示向所有帐号服务器广播
    BOOL SendToAccountServer( void *msg, size_t size, LPCSTR server );
    BOOL SendToDBServer( void *msg, size_t size );
    BOOL SendToDBCenter( void *msg, size_t size );
    BOOL SendToDBCenterEx( DWORD dbcId, void *msg, size_t size );
    BOOL SendToGameServer( DNID dnid, LPCVOID msg, size_t size );
    BOOL SendToPlayer( DNID dnid, void *msg, size_t size );
    BOOL Reconnect();

    DWORD GetServerId();
    DWORD GetGuid( bool bIsPlayer );
    SBornPos *GetBornPos( int index );
    BOOL IsValidName( LPCSTR szName );
    BOOL IsValidSqlString( LPCSTR szString, size_t size );
    LPCSTR GetStringTime();

    void LogText( LPCSTR info, LPCSTR name = NULL );
    void TraceMemory( LPCVOID entry, int offset, unsigned int size, LPCSTR filename );
    void UpdateServerStatus( eServerStatus ss, int iPlayerNumber, int iRealPlayerNumber, int iMaxPlayerNumber );
    BOOL PutIntoRegion( DNID dnidClient, LPCSTR account, DWORD gid, DWORD regionId, DWORD &serverKey );

    //void UpdatePlayerScore( SFixProperty *p );
    BOOL UpdatePlayerData( SARefreshPlayerMsg *pMsg );
	BOOL DoubleUpdatePlayerData( SDoubleSavePlayerMsg *pMsg );
    BOOL DoCheckRebindMsg( DNID dnid, SQCheckRebindMsg *pMsg );
	BOOL GetPlayerData(LPCSTR account, DWORD gid, SFixData *data, SPlayerTempData *tdata);
	BOOL AssignPlayer(DNID dnid, SFixData *data);
    BOOL RebindAccount( DNID dnid, LPCSTR account, DWORD limitedState, LPCSTR idkey, DWORD online, DWORD offline );
    BOOL AccountExist( LPCSTR account );
    BOOL GetDnidContext( DNID dnid, LPCSTR &account, LPCSTR &password, LPCSTR &userpass );
    BOOL SetPlayerServerKey( LPCSTR account, DWORD ID );

    BOOL GetServerInfo( DWORD ID, DNID &dnid, DWORD &ip, DWORD &port );
    BOOL GetRegionInfo( DWORD ID, WORD &mapid, WORD &level );

    void RecvGMLoginMsg( SAGMCheckAccountMsg *pMsg );
    void OnRecvGMMsgFromGameServer( DNID dnidClient, SGMMMsg *pGMMMsg, size_t size );
    SCollect_LoginServerData *GetCollectData();
	void OnRecvMailMsg(SMailBaseMsg *pMsg);
	void SendToAllGameServer( const void *msg, size_t size);
    void UpdateTimeLimit( SQUpdateTimeLimit *data, size_t size );
	void ProcessAccountPrefix( std::string &account );

    // 切分帐号，prefix为返回前缀（如果有的话），函数范围值为裸帐号
    LPCSTR SplitAccount( LPCSTR account, LPCSTR &prefix );

	BOOL GetPlayData(DNID dnid);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


