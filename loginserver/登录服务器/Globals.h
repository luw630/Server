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

// ����������
namespace Globals
{
    // �������ɵĳ�����
    struct SBornPos
    {
        WORD wRegionID;
        WORD wPosX;
        WORD wPosY;
		BYTE byDir;
		WORD wRadius;
    };

    // ������״̬�����ڷ�����״̬��ѯ
    enum eServerStatus
    {
        SS_NORMAL,  // ����������
        SS_CLOSE,   // ������δ����
        SS_REJECT,  // �������ܾ���½
        SS_PROBLEM, // ��������������
    };

    // ��ȡ�ͱ�����������
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
    // ����Ӳ���server,ָ����Ҫ�������ݵ��ĸ��ʺŷ�����,ΪNULLʱ��ʾ���͵�Ĭ���ʺŷ�����,Ϊ-1ʱ��ʾ�������ʺŷ������㲥
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

    // �з��ʺţ�prefixΪ����ǰ׺������еĻ�����������ΧֵΪ���ʺ�
    LPCSTR SplitAccount( LPCSTR account, LPCSTR &prefix );

	BOOL GetPlayData(DNID dnid);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


