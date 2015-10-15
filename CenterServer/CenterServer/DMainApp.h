#pragma once

//TODO:Tony Modify [2012-2-29]Comment:[���ķ������ļ��]
/*
	��һЩ�������Ϳ�����ݴ���
	���ս
	�����½������Ϸ��
	�ǵ�½������Ϸ���ķ�����
	�����Ǻ͵�½������Ϸ�����ͻ��˽�������
	��ҵ�½��Ϸ��ʱ�򣬴����ķ�������ȡ���з�����״̬
	
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
