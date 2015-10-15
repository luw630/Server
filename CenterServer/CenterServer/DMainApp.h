#pragma once

//TODO:Tony Modify [2012-2-29]Comment:[中心服务器的简介]
/*
	做一些跨服管理和跨服数据传送
	跨服战
	管理登陆服和游戏服
	是登陆服和游戏服的服务器
	现在是和登陆服，游戏服，客户端建立连接
	玩家登陆游戏的时候，从中心服务器获取所有服务器状态
	
                                                       */
///////////////////////////////////////////////////////////////////

//#include "servermanager.h"
//
//struct SMessagePackage
//{
//	SMessagePackage( DNID dnid, DWORD srvId, struct SMessage *msg ) : dnid( dnid ), srvId( srvId ), msg( msg ) {}
//	SMessagePackage() : dnid( INVALID_DNID ), srvId( 0 ), msg( NULL ) {}
//
//	DNID dnid;
//    DWORD srvId;
//	SMessage *msg;
//};

#include "ServerManager.h"
#include "PlayerManager.h"
#include <unordered_set>
#define MAX_ASSET_SIZE 8192

// enum SERVERFLAG
// {
// 	GAMESERVER_FLAG,
// 	LOGINERVER_FLAG,
// 	DBSERVER_FLAG,
// };
typedef std::unordered_set<std::string> usetstring;
class CDAppMain 
{
public:
    BOOL		InitServer();
    BOOL		LoopServer();
    BOOL		ExitServer();
	BOOL      bCheckWhiteListIn(std::string& account);
	void     LoadWhiteList();
	void		SaveConnectNumString(const char *strConnect);
public:

public:
    CDAppMain(void);
    ~CDAppMain(void);

protected:

public:
    BOOL                m_bQuit;

	ServerManager		m_ServerManager[MAX_MANAGE_SIZE];

	PlayerManager		m_PlayerManager;
	int m_iAssetBundleInfoLength;
	char m_AssetbundleVersionInfo[MAX_ASSET_SIZE];
	usetstring m_uwhitelist;
};

extern LPSTR g_AnsiToUtf8(const char* AnsiStr);
BOOL CreateApp(void);
void ClearApp(void);
CDAppMain &GetApp(void);
