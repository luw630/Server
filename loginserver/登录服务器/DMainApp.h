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
#include "���������/���������/CenterModule.h"
//#include "../../���������/���������/CenterModule.h"
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
    // ���������˳���־
    BOOL		                m_bQuit;
    std::string                 servername;

    CPlayerManager              m_PlayerManager;
    CServerManager              m_ServerManager;
    //CNetAccountCheckModule      m_AccountClient;
    CNetDBModule                m_DatabaseClient;
    DBCenterModule              m_dbCenterClient;

    CNetGMModule                m_NetGMModule;
    CGMManager					m_GMManager;		// GM���� Lovelonely

    CLoginDataCollect           m_DataCollect;      // ���ݲɼ�����
	AccountModuleManage			m_AccountModuleManage; //���е��˺Ź���ģ��

	CenterModule				m_CenterClient;		//�������ķ���Socket
};

BOOL CreateApp(void);
void ClearApp(void);
CDAppMain *&GetApp(void);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


