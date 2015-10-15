#include "stdafx.h"
#include "mmsystem.h"
#include "psapi.h"
#pragma comment ( lib, "psapi" )

#include "new_net/netclientex.cpp"
#include "pub\complierconfigure.h"

//#include "pub\dwt.h"
#include "NetClientModule.h"
#include "NetworkModule\LogMsgs.h"
#include "NetworkModule\MoveMsgs.h"
#include "NetworkModule\ItemMsgs.h"
#include "NetworkModule\FightMsgs.h"
#include "NetworkModule\ScriptMsgs.h"
#include "NetworkModule\ChatMsgs.h"
#include "NetworkModule\RegionMsgs.h"
#include "NetworkModule\UpgradeMsgs.h"
#include "NetworkModule\refreshmsgs.h"
#include "NetworkModule\TeamMsgs.h"
#include "NetworkModule\NameCardMsgs.h"
#include "NetworkModule\GMModuleMsg.h"
#include "NetworkModule\TongMsgs.h"
#include "NetworkModule\RelationMsgs.h"
#include "NetworkModule\SportsMsgs.h"
#include "NetworkModule\cardpointmodifyMsgs.h"
#include "NetworkModule\MailMsg.h"
#include "NetworkModule\UnionMsgs.h"
#include "NetworkModule\MountsMsgs.h"
#include "networkmodule\PhoneMsgs.h"
#include "networkmodule/PlugingameMsg.h"

#include "liteserializer\Lite.h"
#include "xydclient\rfss\remotefilesystem.h"

extern void ResetAllSate();
extern void DeleteAllObjExceptPlayer();
void SetBeLogining(void (*pFuc)(), bool bLogining = true, int nMaxTryTimes = 0, void (*pConectFail)() = NULL);

extern class DPlayer *g_pCurPlayer;

int iTcpPingGameServer = -1;
bool bInWait = false;
sockaddr_in last_addr; 
DWORD last_globalid;
std::string last_account;
bool is_gameing = false;
int try_times = 30;

// ������4�����������޶���MAX_RECONNECT_TIME��һ���ӣ�* MAX_RECONNECT_TIMES��3����ʱ������
// ���ֻ�������Զ�����MAX_RECONNECT_TIMES��3����[�������绷������ĵط���ֻ�ܽ����ų�]
#define MAX_RECONNECT_TIMES 10
#define MAX_RECONNECT_TIME 20000
DWORD dwTotalAvariableTimes = MAX_RECONNECT_TIMES;
DWORD dwPrevConnectTime = 0;

unsigned __int32 GetCycleTime();

CNetClientModule *g_pNetClientModule = NULL;

CNetClientModule::CNetClientModule()
{
    g_pNetClientModule = this;
	m_bIsKuaFu = false;
    m_pMainLink = NULL;
    m_pTempLink = NULL;
}

static DWORD autoupdatetime = timeGetTime() + 1000 * 60 * 5;
DWORD curMsgSize = 0; // ������������չ��Ϣ����״̬��Ϣ��ȷ����ǰ���ڴ������Ϣ��ʵ�����ݴ�С

int CNetClientModule::Execution()
{
    if (m_pMainLink == NULL) 
        return 0;

    m_pMainLink->KeepAlive();
    iTcpPingGameServer = m_pMainLink->GetPing();

    if ( is_gameing && ( autoupdatetime < timeGetTime() ) )
    {
        autoupdatetime = timeGetTime() + 1000 * 60 * 5;

        SQRebindMsg msg;
        memset(msg.streamData, 0, sizeof(msg.streamData));
        msg.gid = iTcpPingGameServer;
        m_pMainLink->SendMessage(&msg, sizeof(SQRebindMsg));
    }

    SNetClientPacket *pPacket = NULL;

    while (m_pMainLink->GetMessage(&pPacket))
    {
        if (pPacket->m_wLength == 0)
        {
            // rfalse(1, 0, "disconnect!!!");
            // link break
            OnDispatch(NULL);

            delete pPacket;

            break;
        }
        else
        {
            //char info[100];
            //sprintf(info, "i_n %d<%02x %02x %02x %02x>\r\n", pPacket->m_wLength, 
            //    ((LPBYTE)pPacket->m_pvBuffer)[0], ((LPBYTE)pPacket->m_pvBuffer)[1], 
            //    ((LPBYTE)pPacket->m_pvBuffer)[2], ((LPBYTE)pPacket->m_pvBuffer)[3]);
            //OutputDebugString(info);

            // �������ͨ��m_pMainLink���жϱ��ε���Ϣ�Ƿ��Ƿ���������֤��Ϣ
            if (m_pTempLink != NULL)
            {
                delete m_pTempLink;
                m_pTempLink = NULL;
            }

            curMsgSize = pPacket->m_wLength;
            OnDispatch( (SMessage *) pPacket->m_pvBuffer );

            delete pPacket->m_pvBuffer;
            delete pPacket;
        }
    }

    return 1;
}

int CNetClientModule::Initialize(void)
{
    if (m_pMainLink != NULL) 
        return 0;

    if (m_pTempLink != NULL)
        delete m_pTempLink;

    m_pMainLink = new CNetClientEx(TRUE);
    m_pTempLink = NULL;

	return 1;
}

int CNetClientModule::Destroy()
{
    if (m_pMainLink) 
        delete m_pMainLink;

    if (m_pTempLink) 
        delete m_pTempLink;

    m_pMainLink = NULL;
    m_pTempLink = NULL;

    return 1;
}

void CNetClientModule::OnConnectComplete()
{
	if (!m_pMainLink->IsConnected())
	{
		//g_pInterfaceManager->SetBeLogining(NULL, true, 30);
		//return;
	}

    DWORD Param[2] = {1, 1};
    m_pMainLink->SetOption(Param);

    // ���ӳɹ�
    // ���·�����ת���ض���֪ͨ����������
    SQRebindMsg msg;

    try
    {
        lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
        slm( last_account.c_str() );
        msg.gid = last_globalid;
        m_pMainLink->SendMessage( &msg, WORD(sizeof(msg) - slm.EndEdition()) );
    }
    catch ( lite::Xcpt & )
    {
    }

    // bInWait = true;

    is_gameing = false;

    // �۵���Ч����������
    dwTotalAvariableTimes --;

    // ��¼��ǰ������ʱ��
    dwPrevConnectTime = timeGetTime();
}
void ConnectComplete()
{
	g_pNetClientModule->OnConnectComplete();
}

void CNetClientModule::OnRedirectorConnect()
{
    m_pMainLink->SendMessage(&msgRebindTmp, sizeof(msgRebindTmp));

    // Ȼ�����ȴ�״̬
    // ����֤�ɹ��������Ϣ֪ͨ����֤��֪ͨ entry:m_pMainLink��
    // �����ڰ汾�������û�гɹ�֪ͨ��Ϣ��ֱ����Ϊ�µ������Ѿ����ã��ɵ�����ֱ�ӱ��ͷţ��������Ӵ�����Ϣ֮ǰ�ȴ���
    // �������������ֶ��ߴ���1�������������ǵĻ�
    //m_pMainLink->Disconnect();

    //CNetClientEx *p = m_pMainLink;
    //m_pMainLink = m_pTempLink;
    //m_pTempLink = p;
}

void RedirectorConnect()
{
	g_pNetClientModule->OnRedirectorConnect();
}

void TryReconnect()
{
	g_pNetClientModule->OnDispatch(NULL);
}

int CNetClientModule::OnDispatch(SMessage *pMsg)
{
    if (m_pMainLink == NULL) 
        return 0;

    if (pMsg == NULL)
    {
        // ���ӶϿ�������
        if (m_pMainLink == NULL) 
            return 1;

        m_pMainLink->Disconnect();

        // �жϵ�ǰ����һ�ε�����ʱ��ü���Ƿ���Ч
        if ( (int)(timeGetTime() - dwPrevConnectTime) > MAX_RECONNECT_TIME )
        {
            // �����Ч�������ܹ��������������
            if (dwTotalAvariableTimes < MAX_RECONNECT_TIMES)
                dwTotalAvariableTimes++;
        }

        // �Զ�������Ȼ�������°���Ϣ
        if ( is_gameing && (dwTotalAvariableTimes > 0)/* && try_times > 0*/)
        {
            //try_times--;

            bInWait = true;
            char szPort[32];
            m_pMainLink->Connect(inet_ntoa(last_addr.sin_addr), 
                itoa(last_addr.sin_port, szPort, 10), FALSE);

            SetBeLogining(ConnectComplete, true, 10);
			return 0;
        }

        void TrackDMessageBox(LPCSTR info);
        TrackDMessageBox("�ͷ��������ӶϿ�������");

        return 1;
    }

    // ͨ����
    if ( pMsg->_protocol == pMsg->EPRO_SYSTEM_MESSAGE )
    {
        extern BOOL	OnRecvSYSTEM_MESSAGE( SSysBaseMsg *pMsg );
        OnRecvSYSTEM_MESSAGE((SSysBaseMsg*)pMsg);
        return 1;
    }
    else if ( pMsg->_protocol == 100 || pMsg->_protocol == pMsg->EPRO_REBIND_MESSAGE )
    // set rebind flag, at this state, main application maybe run in non-loop
    // connect back socket to new server
    // wait it 
    // the KNetClient::Connect is brock operation, so, do this call cause mainapp pause few minutes
    {
        SRebindMsg *pRebindMsg = (SRebindMsg *)pMsg;
        if (pRebindMsg->_protocol == SRebindMsg::EPRO_REBIND_REGION_SERVER)
        {
            // �����ǰ�����ض���״̬�У�ԭ�����ǲ����ٴ��ض����
            if (m_pTempLink != NULL)
                delete m_pTempLink;

            SRebindRegionServerMsg *pRm = (SRebindRegionServerMsg *)pMsg;
            m_pTempLink = new CNetClientEx(TRUE);

            // ��ΪKNetClientʹ��HASH����˿ںţ�Ҳ����˵���������Ķ˿ںű����Ǽ���HASHֵ��ǰ������
            // ���ԣ�����ͳһʹ��������ΪHASH��ԭʼ����
            char szPort[32];
            last_addr.sin_addr.S_un.S_addr = pRm->ip;
            last_addr.sin_port = (u_short)pRm->port;
            last_globalid = pRm->gid;

            try
            {
                lite::Serialreader sl( pRm->streamData );
                LPCSTR account = sl();
                last_account = account;
            }
            catch ( lite::Xcpt & )
            {
            }

            bInWait = true;
            if (m_pTempLink->Connect(inet_ntoa(last_addr.sin_addr), itoa(last_addr.sin_port, szPort, 10), FALSE))
            {
				iTcpPingGameServer = -1;

                DWORD Param[2] = {1, 1};
                m_pTempLink->SetOption(Param);

                m_pMainLink->Disconnect();

				CNetClientEx *p = m_pMainLink;
                m_pMainLink = m_pTempLink;
                m_pTempLink = p;

                // ���ӳɹ�
                // ���·�����ת���ض���֪ͨ����������
                //SQRebindMsg msg;
				//memset(&msgRebindTmp, 0, sizeof(SQRebindMsg));
				SetBeLogining(RedirectorConnect);

                msgRebindTmp.gid = pRm->gid;
                try
                {
                    lite::Serializer slm( msgRebindTmp.streamData, sizeof( msgRebindTmp.streamData ) );
                    slm( last_account.c_str() );
                    slm.EndEdition();
                }
                catch ( lite::Xcpt & )
                {
                }
            }
            else
            {
                // �޷�����Ŀ�������
                // ��ԭ�������ύ������������У�����Ȩ��û��ǲ�Ҫ�ÿͻ��˾�����
                delete m_pTempLink;
                m_pTempLink = NULL;

                void TrackDMessageBox(LPCSTR info);
                TrackDMessageBox("�޷������ض���ķ�����������");

                m_pMainLink->Disconnect();
            }
        }
        else if (pRebindMsg->_protocol == SRebindMsg::EPRO_CHECK_REBIND)
        {
            autoupdatetime = timeGetTime() + 1000 * 60 * 5;

            SACheckRebindMsg *pCheckRebindMsg = (SACheckRebindMsg *)pMsg;
            if (pCheckRebindMsg->dwResult)
            {
                bInWait = false;
                is_gameing = (pCheckRebindMsg->dwResult == 2);
            }
        }
		// �޷�����
		else if ( pRebindMsg->_protocol == _SRebindMsg::EPRO_REBIND_LOGIN_SERVER )
		{
			if ( m_pTempLink != NULL )
				delete m_pTempLink;

			SARebindLoginServerMsg *pRebindMsg = ( SARebindLoginServerMsg * )pMsg;
			m_pTempLink = new CNetClientEx(TRUE);

			LPCSTR ip = NULL;
			LPCSTR port =NULL;
			LPCSTR account = NULL;
			LPCSTR password = NULL;
			LPCSTR sidname = NULL;
            DWORD checkpassword = 0;	// ��ԭ������ȥ DB Center����ļ����

            try
            {
                lite::Serialreader slm( pRebindMsg->streamData, sizeof( pRebindMsg->streamData ) );

                ip			= slm();
                port		= slm();
                account		= slm();

                // ��������
                if ( pRebindMsg->m_dwType == SARebindLoginServerMsg::REBIND_UNUSUAL )
                {
                    password = slm();
                    sidname	= slm();
                }
                else
                {
                    checkpassword = slm();
                    sidname	= slm();
                }
            }
            catch ( lite::Xcpt & )
            {
                return 0;
            }

			sockaddr_in temp_addr;

			temp_addr.sin_addr .S_un.S_addr = last_addr.sin_addr.S_un.S_addr;
			DWORD tempgid = last_globalid;
			
			last_addr.sin_addr.S_un.S_addr = inet_addr( ip );
            //last_addr.sin_port = pRm->port; // �˿�û�м�Ӧ����һ����
            last_globalid = pRebindMsg->m_dwGID;

			bInWait = true;
            if ( m_pTempLink->Connect( ip, port ) )
            {
				// �������
                ResetAllSate();

				DWORD Param[2] = {1, 1};
                m_pTempLink->SetOption( Param );

				// ��ԭ�����������͵ǳ���Ϣ
				SQLogoutMsg	LogoutMsg;
				LogoutMsg.dwGlobalID = pRebindMsg->m_dwGID;
				LogoutMsg.byLogoutState = SQLogoutMsg::EST_LOGOUT_NORMAL; 

				m_pMainLink->SendMessage( &LogoutMsg, sizeof( LogoutMsg ) );
				Sleep( 100 );

                m_pMainLink->Disconnect();

				DeleteAllObjExceptPlayer();

				CNetClientEx *p = m_pMainLink;
                m_pMainLink = m_pTempLink;
                m_pTempLink = p;

                try
                {	
                    // ��ͬ�ĵ�½��������½��ϢҲ��ͬ
                    if ( pRebindMsg->m_dwType == SARebindLoginServerMsg::REBIND_UNUSUAL )
                    {
                        SQLoginMsg msg;
                        msg.wVersion = PLAYER_VERSION;

                        lite::Serializer sl( msg.streamData , sizeof( msg.streamData ) );
                        if ( sidname[0] != 0 )
						{
                            sl( account )( password )( sidname )( pRebindMsg->m_wGMLevel );
							m_bIsKuaFu = true;
						}
						else
						{
                            sl( account )( password );
							m_bIsKuaFu = false;
						}
                        m_pMainLink->SendMessage( &msg, WORD(sizeof( msg ) - sl.EndEdition()) );
                    }
                    else
                    {
                        extern void ExitToPartSelWindow();
                        ExitToPartSelWindow();
                        bInWait = false;
                        SQLoginNoCheckAccountMsg msg;
                        lite::Serializer sl( msg.streamData , sizeof( msg.streamData ) );

                        sl( account )( checkpassword )( sidname );
                        m_pMainLink->SendMessage( &msg, WORD(sizeof( msg ) - sl.EndEdition()) );
                    }
                }
                catch ( lite::Xcpt & )
                {
                }
			}
			else
			{
				// ����û�гɹ��ָ�֮ǰ��״̬ 
				bInWait = false;
				last_addr.sin_addr.S_un.S_addr = temp_addr.sin_addr .S_un.S_addr;
				last_globalid = tempgid ;
			}
		}

        return 1;
    }
    else if ( pMsg->_protocol == pMsg->EPRO_CHAT_MESSAGE )
    {
        extern BOOL OnRecvCHAT_MESSAGE(SChatBaseMsg * pMsg);
        OnRecvCHAT_MESSAGE((SChatBaseMsg *) pMsg);
        return 1;
    }

    // ��ֹ�ͻ�����δ������ɫ������´���������Ϣ��
    if ( g_pCurPlayer == NULL )
    {
        // ���⴦�����δ���·�����ǰ˫��ʱ�����ʾ���⣡
        if ( pMsg->_protocol != pMsg->EPRO_SCRIPT_MESSAGE ||
            reinterpret_cast< SScriptBaseMsg* >( pMsg )->_protocol != SScriptBaseMsg::EPRO_MULTIME_INFO )
            return 1;
    }
                
    switch(pMsg->_protocol)
    {
    //case pMsg->EPRO_SYSTEM_MESSAGE:
    //    extern BOOL	OnRecvSYSTEM_MESSAGE( SSysBaseMsg *pMsg );
    //    OnRecvSYSTEM_MESSAGE((SSysBaseMsg*)pMsg);
    //    break;

    case pMsg->EPRO_REGION_MESSAGE:
        extern BOOL	OnRecvREGION_MESSAGE( SRegionBaseMsg *pMsg );
        OnRecvREGION_MESSAGE((SRegionBaseMsg*)pMsg);
        break;

    case pMsg->EPRO_MOVE_MESSAGE:
        extern BOOL OnRecvMOVE_MESSAGE(SMoveBaseMsg * pMsg);
        OnRecvMOVE_MESSAGE((SMoveBaseMsg *) pMsg);
        break;

    case pMsg->EPRO_FIGHT_MESSAGE:
        extern BOOL OnRecvFIGHT_MESSAGE(SFightBaseMsg *pMsg);
        OnRecvFIGHT_MESSAGE( (SFightBaseMsg *)pMsg );
        break;

	case pMsg->EPRO_ITEM_MESSAGE:
		extern BOOL OnRecvITEM_MESSAGE(SItemBaseMsg *pMsg);
		OnRecvITEM_MESSAGE( (SItemBaseMsg *)pMsg );
		break;

    case pMsg->EPRO_UPGRADE_MESSAGE:
        extern BOOL OnRecvUPGRADE_MESSAGE( SUpgradeMsg *pMsg );
        OnRecvUPGRADE_MESSAGE( (SUpgradeMsg *)pMsg );
        break;

    case pMsg->EPRO_TEAM_MESSAGE:
        extern BOOL OnRecvATEAM_MESSAGE(STeamBaseMsg* pMsg);
        OnRecvATEAM_MESSAGE((STeamBaseMsg*)pMsg);
        break;
////
    //case pMsg->EPRO_NAMECARD_BASE:
    //    extern BOOL OnRecvNAMECARD_MESSAGE(SNameCardBaseMsg* pMsg);
    //    OnRecvNAMECARD_MESSAGE((SNameCardBaseMsg*)pMsg);
    //    break;

	case pMsg->EPRO_SCRIPT_MESSAGE:
		extern BOOL OnRecvSCRIPT_MESSAGE( SScriptBaseMsg *pMsg); 
		OnRecvSCRIPT_MESSAGE((SScriptBaseMsg *)pMsg);
		break;
    case pMsg->EPRO_GMM_MESSAGE:
        extern BOOL OnRecvGMMODULE_MESSAGE (SGMMMsg *pMsg);
        OnRecvGMMODULE_MESSAGE((SGMMMsg*)pMsg);
        break;
    case pMsg->EPRO_TONG_MESSAGE:
        extern BOOL OnRecvTONG_MESSAGE(STongBaseMsg *pMsg);
        OnRecvTONG_MESSAGE((STongBaseMsg*)pMsg);
        break;
    case pMsg->EPRO_RELATION_MESSAGE:
        extern BOOL OnRecvRELATION_MESSAGE(SRelationBaseMsg* pMsg);
        OnRecvRELATION_MESSAGE((SRelationBaseMsg*)pMsg);
        break;
    case pMsg->EPRO_SPORT_MESSAGE:
        extern BOOL OnRecvSPORTS_MESSAGE(SSportsBaseMsg *pMsg );
        OnRecvSPORTS_MESSAGE((SSportsBaseMsg*)pMsg);
        break;
    case pMsg->EPRO_SPORT_MESSAGE + 1: // ������Ϣ���!
        extern BOOL OnRecvBuilding_MESSAGE( SMessage *pMsg );
        OnRecvBuilding_MESSAGE( pMsg );
        break;
        // this is speical message, caller don't care it 

    case pMsg->EPRO_POINTMODIFY_MESSAGE:
        extern BOOL OnRecvPointModify_MESSAGE(SPointModifyMsg *pMsg );
        OnRecvPointModify_MESSAGE((SPointModifyMsg*)pMsg);
        break;

    case 101:
    case 102:
        // remote file system service
        RFSS::Singleton().DoRpc( ( LPDWORD )pMsg + 1, pMsg->_protocol );
        break;

	case pMsg->EPRO_MAIL_MESSAGE:
		{
			extern BOOL OnRecvMail_MESSAGE(SMailBaseMsg *pMsg );
			OnRecvMail_MESSAGE((SMailBaseMsg*)pMsg);
		}
        break;
		//�绰��Ϣ
	//case pMsg->EPRO_PHONE_MESSAGE:
	//	{
	//	 extern void OnRecvPhone_MESSAGE(SBasePhoneMsg *pMsg);
	//	 OnRecvPhone_MESSAGE((SBasePhoneMsg *)pMsg);
	//	}
 //       break;

	case pMsg->EPRO_UNION_MESSAGE:
		{
        extern VOID OnRecvUnion_MESSAGE( SUnionBaseMsg *pMsg );
		 OnRecvUnion_MESSAGE((SUnionBaseMsg *)pMsg);
		}
        break;
    case pMsg->EPRO_PLUGINGAME_MESSAGE:
        extern VOID OnRecvPluginGame_MESSAGE( SPluginGameBaseMsg *pMsg );
        OnRecvPluginGame_MESSAGE((SPluginGameBaseMsg *)pMsg);
        break;
	case pMsg->ERPO_MOUNTS_MESSAGE:
		extern VOID OnRecvMounts_MESSAGE( SMountsMsg *pMsg );
		OnRecvMounts_MESSAGE((SMountsMsg *)pMsg);
		break;
    default:
        break;
    }

    return 1;
}

int CNetClientModule::SendToServer(char * pSendMsg , DWORD nCount)
{
    if (m_pMainLink == NULL) 
        return 0;

    if( IsBadStringPtr(pSendMsg,nCount))
    {
        //ERROR HANDLING 
    }

    if (bInWait)
        return 0;

    //static DWORD prev = timeGetTime() + 3;
    //if (prev > timeGetTime())
    //{
    //    Sleep(prev - timeGetTime());
    //    prev = timeGetTime() + 10;
    //}
    
    //extern DWORD couttimes;
    //couttimes++;

    //extern DWORD coutsize;
    //coutsize += nCount;

    //static DWORD prev = timeGetTime() + 1000;

    //if (prev < timeGetTime())
    //{
    //    prev = timeGetTime() + 1000;

    //    std::ofstream stream("����.TXT", std::ios::app);
    //    stream << "couttimes = " << couttimes << "\tcoutsize = " << coutsize << "\r\n";

    //    couttimes = 0;
    //    coutsize = 0;
    //}

    //char info[100];
    //sprintf(info, "out %d<%02x %02x %02x %02x>\r\n", nCount, 
    //    ((LPBYTE)pSendMsg)[0], ((LPBYTE)pSendMsg)[1], 
    //    ((LPBYTE)pSendMsg)[2], ((LPBYTE)pSendMsg)[3]);
    //OutputDebugString(info);

    //BOOL MakeFuncPatch( LPVOID handle );
    //MakeFuncPatch( m_pMainLink );

    return m_pMainLink->SendMessage(pSendMsg , (WORD)nCount);  
}

int CNetClientModule::ConnectToServer(char *szIP, char *szPORT)
{
    bInWait = false;

    if (m_pMainLink == NULL) 
        return 0;

    if (m_pMainLink->Connect(szIP, szPORT, 0))
    {
        DWORD Param[2] = {1, 1};
        m_pMainLink->SetOption(Param);

        is_gameing = false;

        return TRUE;
    }

    return FALSE;
}

void CNetClientModule::DisconnectFromServer(void)
{
    if (m_pMainLink == NULL) 
        return;

    m_pMainLink->Disconnect();
}

void CNetClientModule::UpdateInOutSize(DWORD ios[2])
{
    if (m_pMainLink == NULL) 
        return;

    ios[0] = 2;
    ios[1] = 0;

    if (!m_pMainLink->SetOption(ios))
        ios[0] = ios[1] = 0;
}

/*
extern BOOL isErrCtrl;
extern DWORD errCode;
std::list< std::pair< DWORD, DWORD > > validList;

void __stdcall sck( void )
{
    extern UINT utMsgType;
    if ( utMsgType == WM_TIMER )
    {
        if ( !isErrCtrl ) {
            isErrCtrl = true;
            errCode = -1;
        }
        return;
    }

    LPDWORD* stack = 0;
    _asm mov stack, ebp;

    if ( !IsBadReadPtr( stack, 8 ) )
    {
        char str[256];
        int stkdps = 100;

        //BOOL ck = 0;
        while ( stack[0] != 0 || stkdps-- <= 0 )
        {
            //if ( isErrCtrl )
            //    return;

            BOOL isErr = true;
            for ( std::list< std::pair< DWORD, DWORD > >::iterator it = validList.begin(); isErr && ( it != validList.end() ); it ++ )
                if ( ( _W64 DWORD )stack[1] > it->first && ( _W64 DWORD )stack[1] < it->second )
                    isErr = false;

            // ���������Ż��������µ���ջ���Ǳ�׼��ebp��ܽṹ���������������û�취�ˡ�����
            if ( IsBadReadPtr( stack[1], 8 ) )
                break;

            if ( isErr )
            {
                isErrCtrl = true;
                errCode = ( _W64 DWORD )stack[1];
            }

            stack = ( LPDWORD* )stack[0];

            if ( stack != 0 && IsBadReadPtr( stack, 8 ) )
            {
                //isErrCtrl = true;
                break;
            }
        }
    }
    //if ( ck ) OutputDebugString( "end -------------------- \r\n" );

    // OutputDebugString( "Push1\r\n" );
    // MessageBox( 0,0,0,0 );
}

__declspec( naked ) int cgg( void )
{
    _asm
    {
        mov eax, [esp + 12]
        xor eax, 0xf0f0f0f0
        jz __endout

        pushad
        call sck
        popad

__endout:
        pop  eax
        push 0xffffffff
        push 0x00391988
        push eax
        ret 
    }
}

static LPCSTR bklist[][2] = { 
    { NULL, 0 },
    { "ntdll.dll", 0 },
    { "kernel32.dll", 0 },
    { "user32.dll", 0 },
    { "shell32.dll", 0 },
    { "ddraw.dll", 0 },
    { "d3d9.dll", 0 },
    { "shdocvw.dll", 0 },
    { "ieframe.dll", 0 },
    { "shlwapi.dll", 0 },
    { "allayer.dll", 0 },
    { "hxw.dll", 0 },
    { "huoxingyu.dll", 0 },
};

void AddWiFz( HMODULE hMod )
{
    MODULEINFO mi;
    GetModuleInformation( GetCurrentProcess(), hMod, &mi, sizeof( mi ) );
    validList.push_back( std::pair< DWORD, DWORD >( ( DWORD )mi.lpBaseOfDll, ( DWORD )mi.lpBaseOfDll + mi.SizeOfImage ) );
}

void AddWiFn( HMODULE hMod, LPCSTR name )
{
    for ( int i=1;i<sizeof(bklist)/sizeof(bklist[0]);i++ ) {
        if ( bklist[i][1] == 0 && stricmp( name, bklist[i][0] ) == 0 ) {
            AddWiFz( hMod );
            bklist[i][1] = "";
        }
    }
}

void AddWiFn( HMODULE hMod, LPCWSTR name )
{
    char ansi[256];
    WideCharToMultiByte( 936, 0, name, -1, ansi, sizeof( ansi ), NULL, NULL );

    for ( int i=1;i<sizeof(bklist)/sizeof(bklist[0]);i++ ) {
        if ( bklist[i][1] == 0 && stricmp( ansi, bklist[i][0] ) == 0 ) {
            AddWiFz( hMod );
            bklist[i][1] = "";
        }
    }
}

BOOL MakeFuncPatch( LPVOID handle )
{
    if ( bklist[0][1] == 0 ) 
    {
        // �ų��б�
        MODULEINFO mi;
        if ( HMODULE hMod = GetModuleHandle( NULL ) ) {
            GetModuleInformation( GetCurrentProcess(), hMod, &mi, sizeof( mi ) );
            validList.push_front( std::pair< DWORD, DWORD >( ( DWORD )mi.lpBaseOfDll, ( DWORD )mi.lpBaseOfDll + mi.SizeOfImage ) );
        }

        for ( int i=1;i<sizeof(bklist)/sizeof(bklist[0]);i++ )
            LoadLibrary( bklist[i][0] );
        bklist[0][1] = "";
    }

    LPDWORD*** h = ( ( LPDWORD*** )handle );
    if ( h && h[2] && h[2][0] && h[2][0][2] )
    {
        handle = ( ( LPDWORD*** )handle )[2][0][2];
        if ( ( ( *( unsigned __int64 * )handle ) & 0xff00000000ffffff ) != 0x640000000068ff6a )
            return false;

        DWORD flag = 0;
        VirtualProtect( handle, 8, PAGE_EXECUTE_READWRITE, &flag );
        *( unsigned __int64 * )handle = ( ( ( unsigned __int64 )cgg ) << 8 ) | 0x64d0ff00000000b8;
        VirtualProtect( handle, 8, flag, &flag );
        return true;
    }
    return false;
}
*/