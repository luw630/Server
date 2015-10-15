#pragma once

#include "PlayerManager.h"
#include "ServerManager.h"
#include "NetDBModule.h"
#include "NetAccountCheckModule.h"
#include "NetGMModule.h"
#include "CllctServerModule/LoginDataCollect.h"
#include "DBCenterModule.h"
#include "NetWorkModule/datamsgs.h"
#include "AccountModuleManage.h"
#include "区域服务器/区域服务器/CenterModule.h"
//#include "../../区域服务器/区域服务器/CenterModule.h"
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CDAppMain
{
public:
    BOOL	InitServer();
    BOOL	LoopServer();
    BOOL	ExitServer();

public:
    void DisplayServerInfo();

public:
    CDAppMain(void);
    ~CDAppMain(void);
public:
    // 服务器的退出标志
    BOOL		                m_bQuit;
    std::string                 servername;

    CPlayerManager              m_PlayerManager;
    CServerManager              m_ServerManager;
    //CNetAccountCheckModule      m_AccountClient;
    CNetDBModule                m_DatabaseClient;
    DBCenterModule              m_dbCenterClient;

    CNetGMModule                m_NetGMModule;
    CGMManager					m_GMManager;		// GM管理 Lovelonely

    CLoginDataCollect           m_DataCollect;      // 数据采集对象
	AccountModuleManage			m_AccountModuleManage; //所有的账号管理模块

	CenterModule				m_CenterClient;		//连接中心服的Socket
};

BOOL CreateApp(void);
void ClearApp(void);
CDAppMain *&GetApp(void);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


