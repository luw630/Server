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

    //��Ѱ�ʺŷ������ͳ���������������ͼ��������
    m_bQuit = FALSE;

	LPCSTR ip = NULL, port = NULL, account = NULL;

    Globals::GetDatabaseIPPORT( ip, port );
    if ( !m_DatabaseClient.Connect( ip, port ) )
        return rfalse(0, 0, "�޷��������ݿ��������");

    //Globals::GetAccountIPPORT( ip, port );
    //if ( !m_AccountClient.Connect( ip, port ) )
   //     return rfalse(0, 0, "�޷������ʺŷ�������");
	if ( !m_AccountModuleManage.RegisterALLACC() )
			return rfalse(0, 0, "�޷������ʺŷ�������");

    Globals::GetDBCenterIPPORT( ip, port );
    if ( !m_dbCenterClient.Connect( ip, port ) )
        /*return */rfalse( 2, 0, "�޷������������ݿ������[%s:%s]��", ip, port );
	
	Globals::GetCenterServerIPPORT(ip, port);
	if (!m_CenterClient.Connect(ip, port))
	{
		return rfalse(2, 1, "�޷��������ķ�!!!!!");
	}

    // ��ȡ�������ݿ��б�����ʧ��ʱ��Ҳ��������������������Բ���Ҫ������ֵ
    Globals::LoadDbcList();

    Globals::GetGameServerPort( port );
    if ( !m_ServerManager.Host( port ) )
        return rfalse(0, 0, "�޷�������������S���˿ڼ�����");

    Globals::GetPlayerServerPort( port );
    if ( !m_PlayerManager.Host( port ) )
        return rfalse(0, 0, "�޷�������������P���˿ڼ�����");

	/*if (!m_NetGMModule.Init())
		return rfalse(0, 0, "�޷�������������GM���˿ڼ�����");*/
	
	Globals::GetGMServerPort(port);
	if (!m_NetGMModule.Init(port))
		return rfalse(0, 0, "�޷�������������GM���˿ڼ�����");

	Globals::GetCollectPort(port);
	if (!m_DataCollect.InitializeNet((char *)port))
        return rfalse(0, 0, "�޷������ɼ�����˿ڼ�����");

    char vermsg[40];
    _snprintf( vermsg, 20, "�汾�ţ�Build%04d", SERVER_VERSION );
    rfalse(2, 1, vermsg);

	sprintf(vermsg,"������ݰ汾�ţ�Build 0x%x",SFixPlayerDataBuf::GetVersion() );
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
    rfalse(2, 1, "��½������[%s]��������", Globals::GetChargeMode() ? "�շ�" : "���" );

	return TRUE;
}

//---------------------------------------------------------------------------
BOOL CDAppMain::LoopServer()
{
    Sleep(1);

    // ����ʱ�����лᵼ�µĲ���
    // OnRunTime();

	// ����Ǽ���������
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

    // ����������������
    Globals::SaveConfigure();

    // ���������������
    Globals::DbcDestroy();

    return TRUE;
}

void CDAppMain::DisplayServerInfo()
{
    rfalse(2, 1, "���ݿ����[%s]\r\n�˺ŷ�����[%s]\r\n�������ݿ������[%s]", 
        m_DatabaseClient.client.IsConnected()?"����":"û����", 
        //m_AccountClient.client.IsConnected()?"����":"û����",
		m_AccountModuleManage.GetStatus().c_str(), 
        m_dbCenterClient.client.IsConnected()?"����":"û����" );

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
//        // �����ǰ������һֱ���������ض����״̬�У�˵����û�н��뵽��Ϸ������ȥ
//        // һ��ʱ������߳� // ���ǵ���ȫ���⣬���磺����������ɫ�߳��ˣ�������Ϊ����BUG��
//        // �����ɫ������Ϸ�еģ���ô�����������ݽ��޷����档����ͬ������ָ��ơ�����
//        // ���Բ����߳���������һ���ɵ�½����������ı��棡
//        if (pP->byIsRebinding && pP->dwRebindSegTime != 0)
//        {
//            // ���1���Ӷ���û�����ϣ��Ǿ���һ�α���
//            //if ((int)dwCurTime - (int)pP->dwRebindSegTime >= 60000)
//            if ((int)(dwCurTime - pP->dwRebindSegTime) >= 60000)
//            {
//                if (SavePlayerData(pP->Data, pP->key))
//                    // �ɹ������������dwRebindSegTime
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
//                // ���߹һ�4Сʱ��һ�ε�
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
//                        // Ŀǰ��m_wScriptID���ж���û��������
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
//                // ֪ͨ�˺ŷ�����ˢ������
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
