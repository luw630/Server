#pragma once
#include "BaseObject.h"

#include "pub/ConstValue.h"

#include "networkmodule/playertypedef.h"
#include "pub/smcache.h"

class CPlayer;

class CPlayerManager : public CObjectManager
{
private:
	IObject *CreateObject(const __int32 IID);

protected:
	BOOL DispatchTimeMessage(DNID dnidClient, struct STimeBaseMsg *pMsg, CPlayer *pPlayer);
	BOOL DispatchMoveMessage(DNID dnidClient, struct SMoveBaseMsg *pMsg, CPlayer *pPlayer);
    BOOL DispatchFightMessage(DNID dnidClient, struct SFightBaseMsg *pMsg, CPlayer *pPlayer);
    BOOL DispatchScriptMessage(DNID dnidClient, struct SScriptBaseMsg *pMsg, CPlayer *pPlayer);
    BOOL DispatchItemMessage(DNID dnidClient, struct SItemBaseMsg *pMsg, CPlayer *pPlayer);
    BOOL DispatchUpgradeMessage(DNID dnidClient, struct SUpgradeMsg *pMsg, CPlayer *pPlayer);
	BOOL DispatchNameCardMessage(DNID dnidClient, struct SNameCardBaseMsg *pMsg, CPlayer *pPlayer);
	BOOL DispatchSkillMessage(DNID dnidClient, struct SSkillBaseMsg *pMsg, CPlayer *pPlayer);
public:
    CPlayerManager();
	
	//add by ly 2014/4/14 增加玩家的一些相关附加功能（如：背包格子倒计时时间操作、16分钟恢复玩家1点的体力等）
	void PlayerAttachAction();

    bool Run(); 

    LPCSTR GetRelationString();

    void Reconnect();

    void RescueCachePlayers();
    std::map<DWORD, DWORD> m_PlayerSIDMap;

protected:
    std::map<DNID, DWORD> m_PlayerContextMap;
    
	std::map<dwt::stringkey<char[CONST_USERNAME]>, DWORD> m_PlayerNameMap;
    std::map<std::string, DWORD> m_PlayerAccountMap;

	friend class CNetServerModule;

    friend LPIObject GetPlayerByDnid(DNID dnidClient);
    friend int TraversalPlayers(TRAVERSALFUNC TraversalFunc, LPARAM param);
    friend BOOL RemovePlayerByDnid(DNID dnidClient);
    friend BOOL BindPlayerByDnid(DNID dnidClient, LPIObject pObj);
    // add
    friend LPIObject GetPlayerByAccount2(LPCSTR szAccount);
    friend void SetPlayerCountFlag(LPCSTR szAccount,BOOL bFlag);

    friend LPIObject GetPlayerByName(LPCSTR szName);
    friend LPIObject GetPlayerByAccount(LPCSTR szAccount);

    friend BOOL BindNameRelation(LPCSTR szName, LPIObject pObj);
    friend BOOL RemoveNameRelation(LPCSTR szName);
    friend BOOL BindAccountRelation(LPCSTR szAccount, LPIObject pObj);
    friend BOOL RemoveAccountRelation(LPCSTR szAccount);

    friend LPIObject GetPlayerBySID(DWORD dwStaticID);
    friend BOOL BindSIDRelation(DWORD dwStaticID, LPIObject pObj);
    friend BOOL RemoveSIDRelation(DWORD dwStaticID);

	friend bool SetNameToMap(LPCSTR newName, LPCSTR Name);
public:
    //CNetmsgPackager<char[0xf000]> m_GlobalBroadcastMsgPack;

    ObjectCache< SFixProperty, 500 > *static_playercache;
	//Cmemory_object *m_pmemoryobject;
public:
    void KickHangupPlayerAll();
	void KickPlayerAll();		// 踢出当前服务器所有玩家
	void KickOnePlayer();		// 踢出特定的玩家

public:
	void OnEveryDayManagerRun();
};


//// distributed simple player struct
//#include "orbframework/orbframework.h"
//#include "NetworkModule/Playertypedef.h"
//
//class CDistributedSimplePlayerManager
//{
//public:
//    void UpdateSimplePlayer(LPCSTR szName, class CPlayer *pPlayer);
//    BOOL LocateSimplePlayer(LPCSTR szName, SSimplePlayer &SimplePlayer);
//    DWORD GetCurrentTatolPlayerNumber();
//
//public:
//    CDistributedSimplePlayerManager();
//    ~CDistributedSimplePlayerManager();
//
//public:
//    tForceReadMostlyOrbUser<SSimplePlayer> m_OrbUser;
//};
//
//void UpdateDistributedSimplePlayer(LPCSTR szName, CPlayer *pPlayer);
//BOOL LocateDistributedSimplePlayer(LPCSTR szName, SSimplePlayer &SimplePlayer);
//DWORD GetCurrentTatolPlayerNumber();
