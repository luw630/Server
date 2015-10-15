#pragma once

#include "NetWorkModule/NetProviderModule/NetProviderModule.h"
#include "NetWorkModule/playertypedef.h"
#include <string>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// ����������
struct SPlayer
{
    SPlayer()
    {
        memset(this, 0, sizeof(SPlayer));
    }

	DWORD	dwGlobalID;				// ����ִ����Ψһ��ȫ��ID
    DWORD	dwRegionServerKey;		// ��ǰ����[����������]�Ĺؼ���
    BYTE	byIsLostLink;			// �Ƿ���߱�־
    BYTE	byIsRebinding;			// �Ƿ��������ض�����

    std::string account;			// ����ҵ��˺ţ��ؼ��֣�
    std::string password;			// ����
    std::string ip;					// IP
	std::string idkey;				// ���֤��Ϣ

    SFixProperty	Data;			// �������
	SFixData fixData; //�����������
	SPlayerTempData TempData;		// 

    DWORD dwNextCheckExtra;			// �´ζ���۵�ʱ��
    DWORD dwNextCheckTime;			// ��һ�μ���˺ŵ�ʱ��
    DWORD dwRebindSegTime;			// ��ʼ�ض����ʱ�䣬��ʱ�����޳�
    DWORD dwLoginSegment;			// ����ڱ�����Ϸ�еĵ���ʱ�䣨���ڷ�����ϵͳ��
    DWORD limitedState;				// �Ƿ����������ϵͳ
    DWORD online;
    DWORD offline;
    QWORD puid;
};

// ��������ڵ�½�������ϵ������������
struct SLinkContextInfo
{
    std::string account;
    std::string password;
    std::string userpass;
	std::string idkey;

    int gmlevel;
    BOOL doUnbind;
    DWORD limitedState;         // �Ƿ����������ϵͳ
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

    // ��ĳ����Ϸ���������down��֮����Ҫ�����ϵ����ݱ������˳���
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
    // ����ͻ���������Ϣ
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
	BOOL SendPlayerConfig (LPCSTR szAccount);	//�����������
	BOOL CleanPlayerConfig (LPCSTR szAccount);	//���������������
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
    // ����������ӵķ����
    CNetProviderModule server;          

private:
    // ���[�û�]�����ӷ���������ʱ��¼��һϵ�����ڵ�½����������
    std::map< DNID, SLinkContextInfo > tempLinkContext;

    // ���[�û�]�ڳɹ�ѡ���ɫ[���볡��]���ڵ�½�������ϱ����һ�����ݣ����������������ʱ������±���
    std::map< std::string, SPlayer > playerCache;

	std::vector<SCplayerConfig> playercon;


public:
    // �Ƿ����Ƶ�½
    BOOL m_bCanLogin;
	int m_onlineNumber;

    int  GetPlayerCacheCount() { return ( int )tempLinkContext.size() + m_onlineNumber; }
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


