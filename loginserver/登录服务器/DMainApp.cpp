#include "StdAfx.h"
#include "DMainApp.h"
#include "networkmodule\CenterMsg.h"
#include "NetWorkModule/ChatTypedef.h"
#include "liteserializer/lite.h"
#include <time.h>

CDAppMain *&GetApp(void)
{
    static class CDAppMain *ptheApp = NULL;
    return ptheApp;
}

BOOL CreateApp(void)
{
    if ( GetApp() == NULL )
    {
        return ( GetApp() = new CDAppMain ) != NULL;
    }

    return NULL;
}

void ClearApp(void)
{
    if ( GetApp() )
        delete GetApp();

    GetApp() = NULL;
}

CDAppMain::CDAppMain(void)
{
    ::CoInitialize(0);
}

CDAppMain::~CDAppMain(void)
{
}

BOOL CDAppMain::InitServer()
{
    if ( !Globals::LoadConfigure() )
    {
        return rfalse( 0, 0, "LoadConfigure failure" );
    }

    //搜寻帐号服务器和场景服务器，并试图建立连接
    m_bQuit = FALSE;

	LPCSTR ip = NULL, port = NULL, account = NULL;

    Globals::GetDatabaseIPPORT( ip, port );
    if ( !m_DatabaseClient.Connect( ip, port ) )
        return rfalse(0, 0, "无法连接数据库服务器！");

    //Globals::GetAccountIPPORT( ip, port );
    //if ( !m_AccountClient.Connect( ip, port ) )
   //     return rfalse(0, 0, "无法连接帐号服务器！");
	if ( !m_AccountModuleManage.RegisterALLACC() )
			return rfalse(0, 0, "无法连接帐号服务器！");

    Globals::GetDBCenterIPPORT( ip, port );
    if ( !m_dbCenterClient.Connect( ip, port ) )
        /*return */rfalse( 2, 0, "无法连接中央数据库服务器[%s:%s]！", ip, port );
	
	Globals::GetCenterServerIPPORT(ip, port);
	if (!m_CenterClient.Connect(ip, port))
	{
		return rfalse(2, 1, "无法连接中心服!!!!!");
	}

    // 读取中央数据库列表数据失败时，也可能是正常的情况，所以不需要处理返回值
    Globals::LoadDbcList();

    Globals::GetGameServerPort( port );
    if ( !m_ServerManager.Host( port ) )
        return rfalse(0, 0, "无法启动服务器（S）端口监听！");

    Globals::GetPlayerServerPort( port );
    if ( !m_PlayerManager.Host( port ) )
        return rfalse(0, 0, "无法启动服务器（P）端口监听！");

	/*if (!m_NetGMModule.Init())
		return rfalse(0, 0, "无法启动服务器（GM）端口监听！");*/
	
	Globals::GetGMServerPort(port);
	if (!m_NetGMModule.Init(port))
		return rfalse(0, 0, "无法启动服务器（GM）端口监听！");

	Globals::GetCollectPort(port);
	if (!m_DataCollect.InitializeNet((char *)port))
        return rfalse(0, 0, "无法启动采集服务端口监听！");

    char vermsg[40];
    _snprintf( vermsg, 20, "版本号：Build%04d", SERVER_VERSION );
    rfalse(2, 1, vermsg);

	sprintf(vermsg,"玩家数据版本号：Build 0x%x",SFixPlayerDataBuf::GetVersion() );
	rfalse(2, 1, vermsg);

	int nport = 0;
	Globals::GetServerAccount(account);
	Globals::GetServerIPPORT(ip, nport);

	SQRegisterLoginServer _msg;
	_msg.dwIP = inet_addr(ip);
	_msg.dwPort = nport;
	_msg.index = 0;
	_msg._dNew = 0;
	_msg._dActivity = 0;
	_msg._dRush = 0;
	try
	{
		lite::Serializer slm(_msg.streamservername, sizeof(_msg.streamservername));
		slm(account);
		size_t _esize = slm.EndEdition();
		m_CenterClient.SendToCenterServer(&_msg, static_cast<WORD>(sizeof(_msg) - _esize));
	}
	catch (lite::Xcpt &)
	{
		return FALSE;
	}
	rfalse(2, 1, "Registering To CenterServer");
    rfalse(2, 1, "登陆服务器[%s]启动啦！", Globals::GetChargeMode() ? "收费" : "免费" );

	return TRUE;
}

//---------------------------------------------------------------------------
BOOL CDAppMain::LoopServer()
{
    Sleep(1);

    // 处理时间间隔中会导致的操作
    // OnRunTime();

	// 这个是检查帮派数据
	//m_FactionManager.CheckFactionData();

	m_DatabaseClient.Execution();
    //m_AccountClient.Execution();
    m_dbCenterClient.Execution();
    m_ServerManager.Execution();
    m_PlayerManager.Execution();
    m_NetGMModule.Execution();
    m_DataCollect.UpdateNetMsg();
	//
	m_AccountModuleManage.Execution();
	m_CenterClient.Execution();
    Globals::DbcExec();

    return TRUE;
}

//---------------------------------------------------------------------------
BOOL CDAppMain::ExitServer()
{
	//m_FactionManager.SendSaveFactionMsg( TRUE );
	m_CenterClient.OnLogout();
	m_CenterClient.Destroy();
    m_PlayerManager.Destroy();
    m_ServerManager.Destroy();
    m_DatabaseClient.Destroy();
    //m_AccountClient.Destroy();
    m_dbCenterClient.Destroy();
    m_NetGMModule.Destory();
    m_DataCollect.DestroyNet();

    // 保存所有配置数据
    Globals::SaveConfigure();

    // 清除数据中心连接
    Globals::DbcDestroy();

    return TRUE;
}

void CDAppMain::DisplayServerInfo()
{
    rfalse(2, 1, "数据库服务[%s]\r\n账号服务器[%s]\r\n中央数据库服务器[%s]", 
        m_DatabaseClient.client.IsConnected()?"正常":"没联上", 
        //m_AccountClient.client.IsConnected()?"正常":"没联上",
		m_AccountModuleManage.GetStatus().c_str(), 
        m_dbCenterClient.client.IsConnected()?"正常":"没联上" );

    m_PlayerManager.DisplayInfo();
    m_ServerManager.DisplayInfo();

    Globals::DbcInfo();
}
/********************************************************************************************/

//void CDAppMain::CheckCardPoint()
//{
//    std::map< dwt::stringkey<char[ACCOUNTSIZE]>, dwt::resource<SPlayer, 
//        dwt::stringkey<char[ACCOUNTSIZE]> > >::iterator it = m_PlayerList._depository.begin();
//
//    DWORD dwCurTime = timeGetTime();
//
//    for (;it != m_PlayerList._depository.end();it++)
//    {
//        SPlayer *pP = &it->second._data;
//
//
//        // 如果当前这个玩家一直处于连接重定向的状态中，说明他没有进入到游戏场景中去
//        // 一定时间后将他踢出 // 考虑到安全问题，比如：如果将这个角色踢出了，但是因为其它BUG，
//        // 这个角色还在游戏中的，那么他后续的数据将无法保存。。。同样会出现复制。。。
//        // 所以不能踢出，而是做一次由登陆服务器发起的保存！
//        if (pP->byIsRebinding && pP->dwRebindSegTime != 0)
//        {
//            // 如果1分钟都还没有连上，那就做一次保存
//            //if ((int)dwCurTime - (int)pP->dwRebindSegTime >= 60000)
//            if ((int)(dwCurTime - pP->dwRebindSegTime) >= 60000)
//            {
//                if (SavePlayerData(pP->Data, pP->key))
//                    // 成功保存后才能清除dwRebindSegTime
//                    pP->dwRebindSegTime = 0;
//            }
//            continue;
//        }
//
//        if ( (0 != m_bTickStatus) && (pP->dwNextCheckTime != 0) && (pP->dwRegionServerKey != 0))
//        {
//            //if (((int)dwCurTime) - ((int)pP->dwNextCheckTime) >= 0)
//            if ( (int)(dwCurTime - pP->dwNextCheckTime) >= 0)
//            {
//                // 离线挂机4小时扣一次点
//                // pP->dwNextCheckTime = pP->byIsLostLink ? dwCurTime + 1000 * 60 * 60 * 4 : dwCurTime + 1000 * 60 * 10;
//
//                SQRefreshCardMsg msg;
//
//                msg.iCardPoint = 0;
//
//                if(pP->byIsLostLink && pP->dwRebindSegTime == 0)
//                {
//                    if((int)(dwCurTime - pP->dwNextCheckExtra) >=0)
//                    {
//                        // 目前用m_wScriptID来判断有没有随身功能
//                        if(pP->Data.m_wScriptID !=0 )
//                            msg.iCardPoint++;
//
//                        if(pP->TempData.m_dwExtraState & SHOWEXTRASTATE_SALE)
//                            msg.iCardPoint++;
//
//                        pP->dwNextCheckExtra = dwCurTime + REFRESHTIME_OFFLINEEXTRA;
//                    }
//                }
//
//                pP->dwNextCheckTime = pP->byIsLostLink ? dwCurTime + REFRESHTIME_OFFLINE : dwCurTime + REFRESHTIME_ONLINE;
//
//                // 通知账号服务器刷新数据
//
//                dwt::strcpy(msg.szAccount, pP->key, ACCOUNTSIZE);
//                msg.byRefreshType = SQRefreshCardMsg::QUEST_REFRESH;
//                msg.dwActionCode = 0;
//
//                m_AccountServer.SendMessage(&msg, sizeof(msg));
//            }
//        }
//    }
//}

char const * GetMemCheckString() {
    return "this is login server";
}

void AppQuit() {
    GetApp()->m_bQuit = true;
}
