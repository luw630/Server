#include "StdAfx.h"
#include "DMainApp.h"
#include "NetGMModule.h"
#include <fstream>
#include <pub/constvalue.h>
#include <liteserializer/lite.h>
#include "networkmodule/accountmsgs.h"
#include <boost/bind.hpp>

class if_name
{
public:
    if_name( LPCSTR name ) : name( name ) {}

    bool operator () ( std::map< std::string, SPlayer >::iterator &it )
    {
        return dwt::strcmp( it->second.Data.m_Name, name, CONST_USERNAME ) == 0;
    }

    LPCSTR name;
};

LPCSTR GetAccountbyName( LPCSTR name )
{
    SPlayer *player = GetApp()->m_PlayerManager.FindPlayer( if_name( name ) );
    if ( player == NULL )
        return NULL;

    return player->account.c_str();
}

extern DNID GetServerDnidByName( LPCSTR name )
{
    SPlayer *player = GetApp()->m_PlayerManager.FindPlayer( if_name( name ) );
    if ( player == NULL )
        return INVALID_DNID;

    DNID ret = INVALID_DNID; DWORD ip, port;
    GetApp()->m_ServerManager.GetServerInfo( player->dwRegionServerKey, ret, ip, port );

    return ret;
}

CNetGMModule::CNetGMModule() : m_GMServer(TRUE)
{
	
}

CNetGMModule::~CNetGMModule()
{

}

BOOL CNetGMModule::SetGMLinkContext(DNID dnidClient, LPCSTR szAccount, LPCSTR szPassword,WORD wGMLevel)
{
    SGMContextInfo * pInfo = &m_GMContextMap[dnidClient];

    if (!m_GMServer.SetLinkContext(dnidClient, pInfo))
    {
        m_GMContextMap.erase(dnidClient);
        return FALSE;
    }

    pInfo->account = szAccount;
    pInfo->password = szPassword;

    if(pInfo->m_wGMLevel > 5 || pInfo->m_wGMLevel < 0)
        pInfo->m_wGMLevel = 0;
    else
        pInfo->m_wGMLevel = wGMLevel;

    return TRUE;    
}

BOOL CNetGMModule::RemoveLinkContext(DNID dnidClient)
{
    std::map<DNID, SGMContextInfo >::iterator it = m_GMContextMap.find(dnidClient);

    if (it == m_GMContextMap.end())
        return FALSE;

    m_GMContextMap.erase(it);

    m_GMServer.SetLinkContext(dnidClient, NULL);

    return TRUE;
}

SGMContextInfo &CNetGMModule::GetLinkContext(DNID dnidClient)
{
    static SGMContextInfo nil;
    ZeroMemory(&nil,sizeof(SGMContextInfo));

    std::map<DNID, SGMContextInfo>::iterator it = m_GMContextMap.find(dnidClient);

    if (it == m_GMContextMap.end())
        return nil;

    return it->second;
}

BOOL CNetGMModule::Init(LPCSTR port)
{
    // InitializeServer(m_GMServer,"GMPort",OnDispatchGM,this);
	return m_GMServer.Create(port);
};

int CNetGMModule::Execution()
{
	static DWORD dwOneMinSkipTime = timeGetTime() + 1000 * 60;

	if ( (int)(timeGetTime() - dwOneMinSkipTime) > 0 )
	{
		dwOneMinSkipTime = timeGetTime() + 1000 * 60;

		ProcInfo();

	}

	static DWORD dwFiveSecSkip = timeGetTime() + 1000 * 5;

	if ( (int)(timeGetTime() - dwFiveSecSkip) > 0 )
	{
		dwFiveSecSkip = timeGetTime() + 1000 * 5;

		SendInfo();
	}

	return m_GMServer.Execution(boost::bind(&CNetGMModule::OnDispatchGM, this, _1, _2, _3, _5));
};

int CNetGMModule::SendGMMessage(DNID dnidClient,LPVOID pMsg,WORD wSize)
{
    return m_GMServer.SendMessage(dnidClient,pMsg,wSize);
}
extern __int32 serverid;
int CNetGMModule::OnGMLogin(DNID dnidClient,SGMMMsg * pMsg)
{
    SQGMLoginMsg * pGMMsg = (SQGMLoginMsg *)pMsg;

	//----------------------------------------------------------------------------------------
	//ip地址验证和反馈部分 Add By Lovelonely
    //----------------------------------------------------------------------------------------
    try
    {
        lite::Serialreader sl( pGMMsg->streamData );
        // GM的登陆肯定是默认服务器的登陆，所以不需要考虑前缀问题
        LPCSTR account = sl();
        LPCSTR password = sl();

	    if (!GetApp()->m_GMManager.CheckGMIP(dnidClient,true))
	    {
	        SAGMLoginMsg msg;

            lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
            slm( account );

		    msg.wGMLevel=0;
		    msg.wResult=SAGMLoginMsg::ERC_IP_ERROR;
		
            m_GMServer.SendMessage(dnidClient, &msg, (WORD)(sizeof(msg) - slm.EndEdition()) );
		    m_GMServer.DelOneClient(dnidClient);
	    }
        else
        {
	        //----------------------------------------------------------------------------------------
            SQGMCheckAccountMsg msg;
            msg.iServerID = Globals::GetServerId();
            msg.dnidClient = dnidClient;

            lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
            slm( account )( password );

            sockaddr_in addr;
            ZeroMemory(&addr,sizeof(sockaddr_in));

            if ( m_GMServer.GetDnidAddr( dnidClient, &addr, sizeof(addr) ) )
                dwt::strcpy( msg.szIP, inet_ntoa( addr.sin_addr ), sizeof( msg.szIP ) );

            if ( !Globals::SendToAccountServer( &msg, sizeof(msg) - slm.EndEdition(), NULL ) )
            {
                // 发送失败，可能是账号服务器故障
                // 需要通知客户端失败原因
                // 同时断开连接
                SAGMLoginMsg msg;
                msg.wResult = SAGMLoginMsg::ERC_SEND_CACC_TO_ACCSRV_FAIL;
                m_GMServer.SendMessage(dnidClient, &msg, sizeof(SAGMLoginMsg));
                m_GMServer.DelOneClient(dnidClient);
                return FALSE;
            }
        }
    }
    catch ( ... )
    {
    }

    return 0;
}
BOOL CNetGMModule::GetAddrByDnid(DNID dnidClient, char *szIP , DWORD size)
{
    if (size < 17)
        return FALSE;

    if(IsBadStringPtr(szIP,size))
        return FALSE;

    sockaddr_in addr;
    ZeroMemory(&addr,sizeof(sockaddr_in));

    if(TRUE == m_GMServer.GetDnidAddr(dnidClient, &addr, sizeof(sockaddr_in))) // Get the IP in ULong format
    {
        char * pChar;
        pChar = inet_ntoa(addr.sin_addr); // convert to readable format
        if(pChar == NULL) 
        {
            return FALSE;
        }
        else
        {
            memcpy(szIP,inet_ntoa(addr.sin_addr), 17);// assgin to output value
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}

int CNetGMModule::GMLogout(LPCSTR szAccount,LPCSTR szPassword)
{
    SQGMAccountLogoutMsg msg;
    msg.dnidClient = 0;

    try
    {
        lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
        slm( szAccount ) ( szPassword );

	    //----------------------------------------------------------------------------------------
	    GetApp()->m_GMManager.OnGMOut(szAccount);
	    //----------------------------------------------------------------------------------------

        // 同样，GM的登出也是指定默认服务器！
        return Globals::SendToAccountServer( &msg, sizeof(msg) - slm.EndEdition(), NULL );
    }
    catch ( ... )
    {
    }
return 0;
}

bool CNetGMModule::OnDispatchGM(DNID dnidClient,LINKSTATUS enumStatus, LPVOID data, LPVOID attachment)
{
    SMessage *pMsg = (SMessage *)data;
    SGMContextInfo *pContext = (SGMContextInfo *)attachment;
    if (pMsg == NULL)
    {
        // 说明该连接断开
        if (pContext != NULL)
        {
            GMLogout( pContext->account.c_str(), pContext->password.c_str() ); 
            RemoveLinkContext(dnidClient);
            return 1;
        }
        return 0;
    }
    switch (enumStatus)
    {
    case LinkStatus_Waiting_Login:
        // 当处于LinkStatus_Waiting_Login等待认证消息的状态中，只能接受认证消息！

        // 先判断是否是登陆认证级的消息
        if (pMsg->_protocol != SMessage::EPRO_GMM_MESSAGE)
        {
            m_GMServer.DelOneClient(dnidClient);
            return 0;
        }

        // 然后判断是否为认证消息
        SQGMLoginMsg * pLoginMsg ;
        pLoginMsg = (SQGMLoginMsg *)pMsg;
        if (((SGMMMsg*)pMsg)->_protocol != SGMMMsg::EPRO_GM_LOGIN)
        {
            m_GMServer.DelOneClient(dnidClient);
            return 0;
        }
        // 开始认证，将状态字移近一步
        m_GMServer.SetLinkStatus(dnidClient, LinkStatus_Login);
        // 这里处理玩家的登陆
        //OnPlayerLogin(dnidClient, (SQLoginMsg *)pMsg);
        OnGMLogin(dnidClient,(SQGMLoginMsg *)pMsg);
        return 1;

    case LinkStatus_Login:
        // 当处于等待认证的时候，该连接不允许发送任何消息
        // 所以收到消息就断开该连接
        m_GMServer.DelOneClient(dnidClient);
        return 0;

    case LinkStatus_Connected:
        // 其它的消息只能在认证以后的状态下使用，否则视为非法(连接断开)
        {
            OnGMMessage(dnidClient,(SGMMMsg*)pMsg);
        }
    }
    return 0;
};

int CNetGMModule::Destory()
{
   return m_GMServer.Destroy();
}


BOOL CNetGMModule::CheckGMLevel(DNID dnidClient,int iCheckLevel)
{
    return TRUE;
}

int CNetGMModule::OnChatCheckMessage(SAGMCheckChatGMMsg * pGMMsg)
{
    SGMContextInfo GMContextInfo;
    std::map<DNID, SGMContextInfo>::iterator iter;
    for(iter = m_GMContextMap.begin();iter != m_GMContextMap.end();iter++)
    {
        GMContextInfo = iter->second;
        //if(GMContextInfo.m_wOprParam[0] != 0)
        //{
            SendGMMessage(iter->first,pGMMsg,sizeof(SAGMCheckChatGMMsg));
        //}
    }
    return 1;
}
BOOL CNetGMModule::SetGMLinkContext(DNID dnid,SGMContextInfo &LinkContextInfo)
{
    std::map<DNID, SGMContextInfo>::iterator iter;
    iter = m_GMContextMap.find(dnid);
    if (iter != m_GMContextMap.end())
    {
        iter->second = LinkContextInfo;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
int CNetGMModule::OnGMMessage(DNID dnidClient,SGMMMsg * pGMMsg)
{
    switch(pGMMsg->_protocol)
    {
    case SGMMMsg::EPRO_GM_CHECKCHAT:
        {
            BOOL bSend = FALSE;
            SQGMCheckChatGMMsg * pQGMCheckChatGMMsg;
            pQGMCheckChatGMMsg = (SQGMCheckChatGMMsg *)pGMMsg;
            SQGMCheckChatGMMsg QGMCheckChatGMMsg;
            memcpy(&QGMCheckChatGMMsg,pQGMCheckChatGMMsg,sizeof(SQGMCheckChatGMMsg));
            SGMContextInfo LinkContextInfo = GetLinkContext(dnidClient);
            if(LinkContextInfo.m_wGMLevel >= 3)
            {
                if(pQGMCheckChatGMMsg->byOpr = 0)
                {
                    if(LinkContextInfo.m_wOprParam[0] == 1)
                    {
                        bSend = TRUE;
                        LinkContextInfo.m_wOprParam[0] = 0;
                    }
                }
                else
                {
                    if(LinkContextInfo.m_wOprParam[0] == 0)
                    {
                        bSend = TRUE;
                        LinkContextInfo.m_wOprParam[0] = 1;
                    }
                }
                if (SetGMLinkContext(dnidClient,LinkContextInfo)==FALSE)
                {
                    m_GMContextMap.erase(dnidClient);
                }
            }
            std::map<DWORD,dwt::resource<SServer,DWORD> >::iterator iter;

            if(bSend == TRUE)
            {
                GetApp()->m_ServerManager.Broadcast( &QGMCheckChatGMMsg, sizeof(SQGMCheckChatGMMsg) );
            }
        }
        break;
    case SGMMMsg::EPRO_GM_SENDTALK:
        {
            SQGMTalkMsg * pQGMTalkMsg;
            pQGMTalkMsg = (SQGMTalkMsg *)pGMMsg;
            SQGMTalkMsg QGMTalkMsg;
            memcpy(&QGMTalkMsg,pQGMTalkMsg,sizeof(SQGMTalkMsg));
            std::map< DWORD, dwt::resource<SServer, DWORD> >::iterator iter;
            SGMContextInfo LinkContextInfo = GetLinkContext(dnidClient);
            if(LinkContextInfo.m_wGMLevel >= 2)
            {
                GetApp()->m_ServerManager.Broadcast( &QGMTalkMsg, sizeof(SQGMTalkMsg) );

                SAGMTalkMsg AGMTalkMsg;
                SendGMMessage(dnidClient,&AGMTalkMsg,sizeof(SAGMTalkMsg));
            }
        }
        break;
    case SGMMMsg::EPRO_GM_MOVETOBORN:
        {
            SQMovetoBornMsg * pQMovetoBornMsg;
            pQMovetoBornMsg = (SQMovetoBornMsg *)pGMMsg;
            SQMovetoBornMsg QMovetoBornMsg;
            memcpy(&QMovetoBornMsg,pQMovetoBornMsg,sizeof(SQMovetoBornMsg));


            SGMContextInfo LinkContextInfo = GetLinkContext(dnidClient);
            QMovetoBornMsg.wGMLevel = LinkContextInfo.m_wGMLevel;
            QMovetoBornMsg.dnidClient = dnidClient;
            LPCSTR szAccount = NULL;
            if ( LinkContextInfo.m_wGMLevel >= 1 )
            {
                DNID dnid = GetServerDnidByName( pQMovetoBornMsg->szName );
                if ( !Globals::SendToGameServer( dnid, &QMovetoBornMsg, sizeof(QMovetoBornMsg) ) )
                {
                    SAMovetoBornMsg AMovetoBornMsg;
					memcpy(AMovetoBornMsg.szName, pQMovetoBornMsg->szName, CONST_USERNAME);
                    AMovetoBornMsg.byResult = 0;
                    SendGMMessage(dnidClient,&AMovetoBornMsg,sizeof(SAMovetoBornMsg));
                }
            }
        }
        break;
    case SGMMMsg::EPRO_GM_TALKMASK:
        {
            SQTalkMaskGMMsg *pMsg = (SQTalkMaskGMMsg *)pGMMsg;

            DNID dnid = GetServerDnidByName( pMsg->szName );
            LPCSTR account = GetAccountbyName( pMsg->szName );

            if ( dnid != INVALID_DNID && account != NULL )
            {
                try
                {
			        SQTalkMaskSrvGMMsg msg;

			        msg.dwTime = pMsg->dwTime;
			        msg.dnidClient = dnidClient;
			        dwt::strcpy( msg.szName, pMsg->szName, CONST_USERNAME );

                    lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
                    slm( account );

                    GetApp()->m_ServerManager.Broadcast( &msg, sizeof( msg ) - slm.EndEdition() );
                    //Globals::SendToGameServer( dnid, &msg, sizeof(msg) - slm.EndEdition() );
			    }
                catch ( ... )
                {
                }
            }
            else
            {
				SATalkMaskGMMsg msg;
				msg.wResult=0;
				dwt::strcpy( msg.szName, pMsg->szName, CONST_USERNAME );
				SendGMMessage( dnidClient, &msg, sizeof(msg) );
			}
        }
        break;

    case SGMMMsg::EPRO_GM_TALKUNMASK:
        {
            SQTalkUnMaskGMMsg *pMsg = ( SQTalkUnMaskGMMsg* )pGMMsg;
            
            SGMContextInfo lcInfo = GetLinkContext(dnidClient);
            pMsg->wGMLevel = lcInfo.m_wGMLevel;
            pMsg->dnidClient = dnidClient;

            if ( lcInfo.m_wGMLevel >= 1 )
            {
                GetApp()->m_ServerManager.Broadcast( pMsg, sizeof( SQTalkUnMaskGMMsg ) );

                //DNID dnid = GetServerDnidByName( pQTalkUnMaskGMMsg->szName );
                //if ( dnid != INVALID_DNID )
                //{
                //    Globals::SendToGameServer( dnid, &QTalkUnMaskGMMsg, sizeof(SQTalkUnMaskGMMsg) );
                //}
                //else
                //{
                //    SATalkUnMaskGMMsg ATalkUnMaskGMMsg;
                //    memcpy( ATalkUnMaskGMMsg.szName, pQTalkUnMaskGMMsg->szName, CONST_USERNAME );
                //    ATalkUnMaskGMMsg.wResult = 2;
                //    SendGMMessage( dnidClient, &ATalkUnMaskGMMsg, sizeof(SATalkUnMaskGMMsg) );
                //}
            }
            else
            {
                SATalkUnMaskGMMsg ATalkUnMaskGMMsg;
                memcpy( ATalkUnMaskGMMsg.szName, pMsg->szName, CONST_USERNAME );
                ATalkUnMaskGMMsg.wResult = 2;
                SendGMMessage( dnidClient, &ATalkUnMaskGMMsg, sizeof(SATalkUnMaskGMMsg) );
            }
        }
        break;
    case SGMMMsg::EPRO_GM_CHECKWAREHOUSE:
        {
            SQCheckWareHouseMsg *pQCheckWareHouseMsg = (SQCheckWareHouseMsg *)pGMMsg;

            try
            {
                lite::Serialreader sl( pQCheckWareHouseMsg->streamData );
                LPCSTR account = sl();

                SPlayer *pPlayer = GetApp()->m_PlayerManager.FindPlayer( account );
			    SACheckWareHouseMsg msg;

                lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
                slm( account );

			    if (pPlayer != NULL)
			    {
				    msg.wResult = SACheckWareHouseMsg::ERC_SUCCESS;
				    memcpy( &msg.WarehouseI,&pPlayer->Data.SFixStorage1::m_pStorageGoods,sizeof(SFixStorage1));
//				    memcpy( &msg.WarehouseII,&pPlayer->Data.SFixStorage2::m_pStorageGoods,sizeof(SFixStorage2));
//				    memcpy( &msg.WarehouseIII,&pPlayer->Data.SFixStorage3::m_pStorageGoods,sizeof(SFixStorage3));
                }
			    else
			    {
				    msg.wResult = SACheckWareHouseMsg::ERC_FAIL;
			    }

                SendGMMessage( dnidClient, &msg, (WORD)(sizeof(msg) - slm.EndEdition()) );
			}
            catch ( ... )
            {
            }

            /*SGMContextInfo LinkContextInfo = GetLinkContext(dnidClient);
            if(LinkContextInfo.m_wGMLevel >=1 )
            {
                SQCheckWarehouseMsg QCheckWarehouseMsg;
                memcpy(QCheckWarehouseMsg.szAccount,pQCheckWareHouseMsg->szAccount,17);
                QCheckWarehouseMsg.dnidClient = dnidClient;
                return GetApp()->m_DataServer.SendMsgToDataSrv(&QCheckWarehouseMsg,sizeof(SQCheckWarehouseMsg));
            }*/
        }
        break;
    case SGMMMsg::EPRO_GM_CHECKPLAYER_DB:
        {
            SQCheckPlayerDatabaseMsg * pQCheckPlayerMsg = (SQCheckPlayerDatabaseMsg *)pGMMsg;
			SACheckPlayerDatabaseMsg msg;

            try
            {
                SPlayer *pPlayer = GetApp()->m_PlayerManager.FindPlayer( if_name( pQCheckPlayerMsg->szName ) );
			    if (pPlayer != NULL)
			    {
                    lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
                    slm( pPlayer->account.c_str() );

				    msg.wResult = 1;
				    memcpy( &msg.PlayerData, (SFixBaseData*)&(pPlayer->Data), sizeof(SFixBaseData) );
				    memcpy( &msg.PlayerPackage.m_BaseGoods, &(pPlayer->Data.m_BaseGoods), sizeof(SPackageItem)*64 );

                    dwt::strcpy( msg.szIP, pPlayer->ip.c_str(), sizeof( msg.szIP ) );

                    // ignore nullsize
                    slm.EndEdition();
			    }
			    else
			    {
				    msg.wResult = 0;
			    }

                SendGMMessage( dnidClient, &msg, sizeof(msg) );
            }
            catch ( ... )
            {
            }


            /*SGMContextInfo LinkContextInfo = GetLinkContext(dnidClient);
            if(LinkContextInfo.m_wGMLevel >= 1)
            {
                SQCheckPlayerDBMsg QCheckPlayerDBMsg;
                memcpy(QCheckPlayerDBMsg.szName,pQCheckPlayerMsg->szName,CONST_USERNAME);
                QCheckPlayerDBMsg.dnidClient = dnidClient;
                return GetApp()->m_DataServer.SendMsgToDataSrv(&QCheckPlayerDBMsg,sizeof(SQCheckPlayerDBMsg));
            }*/
        }
        break;
    case SGMMMsg::EPRO_GM_CHANGEPASS:
        {
    //        SQChangePassGMMsg * pQChangePassGMMsg;
    //        pQChangePassGMMsg = (SQChangePassGMMsg *)pGMMsg;
    //        SQChangePasswordMsg QChangePasswordMsg;
    //        SGMContextInfo LinkContextInfo = GetLinkContext(dnidClient);
    //        if(LinkContextInfo.m_wGMLevel >= 3 )
    //        {
				//dwt::strcpy(QChangePasswordMsg.szAccount,pQChangePassGMMsg->szAccount,ACCOUNTSIZE);
    //            dwt::strcpy(QChangePasswordMsg.szNewPassword,pQChangePassGMMsg->szNewPass,CONST_USERNAME);
    //            dwt::strcpy(QChangePasswordMsg.szOldPassword,pQChangePassGMMsg->szOldPass,CONST_USERNAME);
    //            QChangePasswordMsg.dnidClient = dnidClient;
    //            return Globals::SendToAccountServer(&QChangePasswordMsg,sizeof(SQChangePasswordMsg));
    //        }
        }
        break;
    case SGMMMsg::EPRO_GM_BLOCKACCOUNT:
        {
            SQBlockAccountMsg *pMsg =(SQBlockAccountMsg*)pGMMsg;
            SQBAMsg msg;

            SGMContextInfo LinkContextInfo = GetLinkContext(dnidClient);
            if ( LinkContextInfo.m_wGMLevel >=3 )
            {
                try
                {
                    lite::Serialreader sl( pMsg->streamData );
                    lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
					LPCSTR lpAccount = sl();

                    LPCSTR prefix = NULL;
                    LPCSTR abs_account = Globals::SplitAccount( lpAccount, prefix );
					slm( abs_account );

                    msg.dwBlockTime = pMsg->dwBlockDuration;
                    msg.dnidClient = dnidClient;
                    return Globals::SendToAccountServer( &msg, sizeof(msg) - slm.EndEdition(), prefix );
                }
                catch ( ... )
                {
                }
            }
        }
        break;
    //case SGMMMsg::EPRO_GM_GETONLINETIME:
    //    {
    //        SQGetOnlineTimeMsg * pQGetOnlineTimeMsg;
    //        pQGetOnlineTimeMsg = (SQGetOnlineTimeMsg *)pGMMsg;
    //        SQGOTMsg QGetOnlineTimeMsg;
    //        SGMContextInfo LinkContextInfo = GetLinkContext(dnidClient);
    //        if(LinkContextInfo.m_wGMLevel > 1)
    //        {

				//dwt::strcpy(QGetOnlineTimeMsg.szAccount,pQGetOnlineTimeMsg->szAccount,ACCOUNTSIZE);
    //            QGetOnlineTimeMsg.dnidClient = dnidClient;
    //            return GetApp()->m_AccountServer.SendMsgToAccountSrv(&QGetOnlineTimeMsg,sizeof(SQGOTMsg));
    //        }
    //    }
    //    break;
    case SGMMMsg::EPRO_GM_LOGOUT:
        {
            SQGMLogoutMsg *pMsg = (SQGMLogoutMsg *)pGMMsg;
            SQGMAccountLogoutMsg msg;
            SGMContextInfo LinkContextInfo = GetLinkContext(dnidClient);

            try
            {
                // 这个操作是强制对方下线，需要解析服务器
                lite::Serialreader sl( pMsg->streamData );
                lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
                LPCSTR lpAccount = sl();

                LPCSTR prefix = NULL;
                LPCSTR abs_account = Globals::SplitAccount( lpAccount, prefix );
                slm( abs_account );

                msg.dnidClient = dnidClient;
                return Globals::SendToAccountServer( &msg, sizeof(msg) - slm.EndEdition(), prefix );
            }
            catch ( ... )
            {
            }
        }
        break;
  //  case SGMMMsg::EPRO_CHECK_ONLINE:
  //      {
  //          int players = 0;
  //          int max_players = 0;
  //          int count = 0;
  //          SGMContextInfo LinkContextInfo = GetLinkContext(dnidClient);
  //          if ( LinkContextInfo.m_wGMLevel > 1 )
  //          {
  //              SACheckOnlineMsg msg;
  //              GetApp()->m_ServerManager.GetPlayerNumber( msg.dwMaxCount, msg.dwOnlineCount );
  //              g_SendMessage( dnidClient, &msg, sizeof(msg) );
  //          }
  //      }
		//break;
	//-----------------------------------------
	//Add By Lovelonely
	case SGMMMsg::EPRO_GM_MOVETOSAFEPOINT:
		{
			SQMoveToSafePointGMMsg *pMsg = (SQMoveToSafePointGMMsg*)pGMMsg;

			SQMoveToSafePointGMMsg msg;

			memcpy( &msg, pMsg, sizeof(msg) );

			msg.dnidClient = dnidClient;
			
            LPCSTR account = GetAccountbyName( pMsg->szName );
            DNID dnid = GetServerDnidByName( pMsg->szName );

			if ( account != NULL && dnid != INVALID_DNID )
			{
                Globals::SendToGameServer( dnid, &msg, sizeof(msg) );
			}
			else
			{
				SAMoveToSafePointGMMsg msg;

				dwt::strcpy(msg.szName, pMsg->szName, CONST_USERNAME);

				msg.wRet = SAMoveToSafePointGMMsg::RET_FAIL;

				SendGMMessage( dnidClient, &msg, sizeof(msg) );
			}
		}
		break;

	case SGMMMsg::EPRO_GM_GETMASKNAME:
		{
			SQGetMaskNameGMMsg *pMsg = (SQGetMaskNameGMMsg*)pGMMsg;

			if ( pMsg->wPara == SQGetMaskNameGMMsg::PA_TALKMASK )
			{
				SQGetMaskNameGMMsg msg;

				msg.wPara = pMsg->wPara;
				msg.dnidClient = dnidClient;

                GetApp()->m_ServerManager.Broadcast( &msg, sizeof( msg ) );
			}
			if ( pMsg->wPara == SQGetMaskNameGMMsg::PA_GAOL )
			{
				SAGetMaskNameGMMsg msg;

				msg.szName[0] = NULL;
				msg.wPara = SAGetMaskNameGMMsg::PA_BEGIN;
				msg.wResult = SAGetMaskNameGMMsg::RT_GAOL;

				SendGMMessage( dnidClient, &msg, sizeof(msg) );

				std::list<SPlayerName>::iterator it=GetApp()->m_GMManager.m_listGaol.begin();

				while (it != GetApp()->m_GMManager.m_listGaol.end())
				{
					dwt::strcpy(msg.szName, it->szName, CONST_USERNAME);
					msg.wPara=SAGetMaskNameGMMsg::PA_SEND;

					SendGMMessage( dnidClient, &msg, sizeof(msg) );
					it++;
				}

				msg.wPara = SAGetMaskNameGMMsg::PA_END;
				msg.szName[0] = NULL;

				SendGMMessage( dnidClient, &msg, sizeof(msg) );
			}
		}
		break;

	case SGMMMsg::EPRO_GM_GETPING:
		{
			SQGetPingGMMsg *pMsg=(SQGetPingGMMsg*)pGMMsg;

			SQGetPingGMMsg msg;

			msg.dnidClient=dnidClient;
			msg.iPing=0;
            msg.ip32=0;
            msg.mac64=0;
			msg.wPara=pMsg->wPara;

            GetApp()->m_ServerManager.Broadcast( &msg, sizeof( msg ) );
		}
		break;

	case SGMMMsg::EPRO_GM_SAYGLOBAL:
		{
			SQSayGlobalGMMsg* pMsg=(SQSayGlobalGMMsg*)pGMMsg;

			SQSayGlobalGMMsg msg;
			memcpy(&msg,pMsg,sizeof(SQSayGlobalGMMsg));

            GetApp()->m_ServerManager.SendFirst(  &msg, sizeof(msg) );
		}	
		break;

	case SGMMMsg::EPRO_GM_WISPER: 
		{
			SQWisperGMMsg *pMsg=(SQWisperGMMsg*)pGMMsg; //收到GM工具端发过来的GM密语
			
			SAWisperGMMsg msg;

			dwt::strcpy(msg.szGM, pMsg->szGM, CONST_USERNAME);
			dwt::strcpy(msg.szPlayer, pMsg->szPlayer, CONST_USERNAME);
			dwt::strcpy(msg.szSay, pMsg->szSay, 50);	

			CGM *pGM=GetApp()->m_GMManager.FindGM(pMsg->szGM);

			if (pGM == NULL)
                break;

            try
            {
			    msg.wResult = SAWisperGMMsg::RT_ERROR;
                size_t nullsize = 0;

                SPlayer *pPlayer = GetApp()->m_PlayerManager.FindPlayer( if_name( pMsg->szPlayer ) );
			    if ( pPlayer != NULL )
			    { 
                    lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
                    slm( pPlayer->account.c_str() );
                    nullsize = slm.EndEdition();

                    //将玩家的账号和点数附加到数据包作为GM工具端回显是的依据
					//msg.dwConsumePoint = pPlayer->Data.m_dwConsumePoint;
                    DNID dnid = GetServerDnidByName( pMsg->szPlayer );

				    if ( dnid != INVALID_DNID )
				    {
			            msg.wResult=2;
                        Globals::SendToGameServer( dnid, &msg, sizeof(msg) - nullsize );
				    }
			    }

                SendGMMessage( pGM->m_dnidClient, &msg, (WORD)(sizeof(msg) - nullsize) );
            }
            catch ( ... )
            {
            }
	    }
		break;
	case SGMMMsg::EPRO_GM_CLOSE:
		{
			SQCloseGMMsg *pMsg=(SQCloseGMMsg*)pGMMsg;

			SQCloseGMMsg qCloseMsg;

			memcpy(&qCloseMsg,pMsg,sizeof(SQCloseGMMsg));

            DNID dnid = GetServerDnidByName( pMsg->szName );
            if ( dnid != INVALID_DNID )
                Globals::SendToGameServer( dnid, &qCloseMsg,sizeof(SQCloseGMMsg) );
			else
			{
				SACloseGMMsg aCloseMsg;

				dwt::strcpy(aCloseMsg.szName, pMsg->szName, CONST_USERNAME);
				dwt::strcpy(aCloseMsg.szGM, pMsg->szGM, CONST_USERNAME);
				aCloseMsg.wResult=SACloseGMMsg::RT_ERROR;

				SendGMMessage(dnidClient,&aCloseMsg,sizeof(SACloseGMMsg));
			}
		}
		break;
	/*case SGMMMsg::EPRO_GM_CHECKSCORE:
		{
			SACheckScoreGMMsg aCheckScoreMsg;

            extern SScoreTable s_Scores;
			memcpy(&aCheckScoreMsg.s_ScoreTab,&s_Scores,sizeof(SScoreTable));

			SendGMMessage(dnidClient,&aCheckScoreMsg,sizeof(SACheckScoreGMMsg));
		}
		break;*/
	case SGMMMsg::EPRO_GM_CUT:
		{
			SQCutGMMsg qCutMsg;

			memcpy(&qCutMsg,(SQCutGMMsg*)pGMMsg,sizeof(SQCutGMMsg));

			SACutGMMsg aCutMsg;
			dwt::strcpy(aCutMsg.szName, qCutMsg.szName, CONST_USERNAME);

			aCutMsg.wResult=SACutGMMsg::RT_ERROR;

            DNID dnid = GetServerDnidByName( qCutMsg.szName );

            if ( dnid != INVALID_DNID )
            {
                if ( Globals::SendToGameServer( dnid, &qCutMsg,sizeof(qCutMsg) ) )
                {
				    aCutMsg.wResult = SACutGMMsg::RT_SUCCESS;
                }
			}
			
			SendGMMessage( dnidClient, &aCutMsg, sizeof(SACutGMMsg) );
		}
		break;

	case SGMMMsg::EPRO_GM_PUBLICINFO:
		{
			SQPublicInfoGMMsg *pMsg=(SQPublicInfoGMMsg*)pGMMsg;

			SPublicInfo sInfo;
			dwt::strcpy(sInfo.szSay,pMsg->szSay,128);
			sInfo.nTime=pMsg->nTime;
			sInfo.wType=pMsg->wType;
			sInfo.lClr=pMsg->lClr;

			m_InfoTab.AddPInfo(sInfo);

			SAPublicInfoGMMsg aPubInfoMsg;

			aPubInfoMsg.wResult=SAPublicInfoGMMsg::RT_SUCCESS;

			aPubInfoMsg.szSay[0]=NULL;
			
			SendGMMessage(dnidClient,&aPubInfoMsg,sizeof(SAPublicInfoGMMsg));
		}
		break;
	case SGMMMsg::EPRO_GM_CHANGEPUBINFO:
		{
			SQChangePubInfoGMMsg *pMsg=(SQChangePubInfoGMMsg*)pGMMsg;

			if (pMsg->wPara == SQChangePubInfoGMMsg::PA_GET)
			{
				SAChangePubInfoGMMsg aChangeInfoMsg;

				aChangeInfoMsg.wRet=SAChangePubInfoGMMsg::RT_RETURN;
			
				std::list<SPublicInfo>::iterator it=m_InfoTab.m_listInfo.begin();

				int i=0;

				while (it != m_InfoTab.m_listInfo.end())
				{
					dwt::strcpy(aChangeInfoMsg.szInfo[i],it->szSay,MAXPUBLICINFO);

					i++;
					
					it++;
				}

				for (;i < 10; i++)
				{
					aChangeInfoMsg.szInfo[i][0]=NULL;
				}

                SendGMMessage(dnidClient,&aChangeInfoMsg,sizeof(SAChangePubInfoGMMsg)); 			

			}

			if (pMsg->wPara == SQChangePubInfoGMMsg::PA_CHANGE)
			{
				std::list<SPublicInfo>::iterator it=m_InfoTab.m_listInfo.begin();
				
				while (it != m_InfoTab.m_listInfo.end())
				{
					BOOL bDel=TRUE;

					for (int i=0; pMsg->szInfo[i][0] != NULL; i++)
					{
						if (dwt::strcmp(it->szSay,pMsg->szInfo[i],MAXPUBLICINFO) == 0)
						{
							bDel=FALSE;
							break;
						}
					}

					std::list<SPublicInfo>::iterator itTmp=it;
					
					it++;

					if (bDel)
					{
						m_InfoTab.m_listInfo.erase(itTmp);
					}
				}

				SAChangePubInfoGMMsg aChangeInfoMsg;

				aChangeInfoMsg.wRet=SAChangePubInfoGMMsg::RT_SUCCESS;

				SendGMMessage(dnidClient,&aChangeInfoMsg,sizeof(SAChangePubInfoGMMsg));
			}

		}
		break;
	case SGMMMsg::EPRO_GM_CHECKONLINEGM:
		{
			SQCheckOnlineGMGMMsg *pMsg=(SQCheckOnlineGMGMMsg*)pGMMsg;

			if (pMsg->wPara != SQCheckOnlineGMGMMsg::PA_GET)
			{
				return FALSE;
			}

			SQCheckOnlineGMGMMsg msg;

			dwt::strcpy(msg.szName, pMsg->szName, CONST_USERNAME);

			msg.wPara=SQCheckOnlineGMGMMsg::PA_RETURN;

			for (int i=0; i < 10; i++)
			{
				msg.szGM[i][0]=NULL;
			}

			int j;

			std::map<std::string,CGM>::iterator it=GetApp()->m_GMManager.m_GMMap.begin();

			for (j=0; j < 5; j++)
			{
				if (it == GetApp()->m_GMManager.m_GMMap.end())
				{
					break;
				}
				dwt::strcpy(msg.szGM[j], it->second.m_szID, CONST_USERNAME);

				it++;
			}

            /*
			it=GetApp()->m_GMManager.m_OnlineGMMap.begin();

			for (; j < 10; j++)
			{
				if (it == GetApp()->m_GMManager.m_OnlineGMMap.end())
				{
					break;
				}
				dwt::strcpy(msg.szGM[j],it->second.m_szID,CONST_USERNAME);

				it++;
			}
            */
			
			SendGMMessage(dnidClient,&msg,sizeof(SQCheckOnlineGMGMMsg));

		};
		break;

		case SGMMMsg::EPRO_GM_VISIBLE:
		{
			SQVisibleMsg *pVisible = ( SQVisibleMsg * )pGMMsg;
			SAVisibleMsg msg;

			if ( pVisible->szName[0] == 0 )
			{
				msg.byRet = -1;
				dwt::strcpy(msg.szName, 0, CONST_USERNAME);
				SendGMMessage( pVisible->dnidClient,&msg, sizeof( msg ) );
				break;
			}

			CGM *pGM = GetApp()->m_GMManager.FindGM( pVisible->szName );
			if ( pGM == NULL )
			{
				msg.byRet = -1;
				dwt::strcpy(msg.szName, pVisible->szName, CONST_USERNAME);
				SendGMMessage( pVisible->dnidClient,&msg, sizeof( msg ) );
				break;
			}

			pGM->m_bIsVisible = pVisible->byOerate;
			msg.byRet = pGM->m_bIsVisible;
			SendGMMessage( dnidClient,&msg, sizeof( msg ) );
		}
		break;

        case SGMMMsg::EPRO_GM_CMD:
        {
            static char buffer[1036] = {0};
            SQEXECGMCMD *pExecCmd = ( SQEXECGMCMD * )pGMMsg;
            *((LPDWORD)buffer) = 102;
            *( (DNID*)( &buffer[4] ) ) = dnidClient;
            try
            {
                lite::Serialreader slr( pExecCmd->streamData );
                
                int verify = slr();
                LPCSTR account = slr();     // 受控方账号名
                lite::Variant lvt = slr();  // 控制方的指令序列[数据块]
                if ( ( verify != 0xa00200b ) || account == NULL || lvt.dataType == lite::Variant::VT_EMPTY || lvt.dataType == lite::Variant::VT_NULL )
                    break;

                lite::Serializer slm( &buffer[12], 1024 );
                slm( verify )( account )( lvt );
                Globals::SendToAllGameServer(  buffer, ( sizeof(buffer) - slm.EndEdition() + 12 ) );
            }
            catch ( lite::Xcpt & )
            {
            }
        }
        break;
		//-----------------------------------------
	}
    return 1;
}

CDistributedOPR::CDistributedOPR()
{
    m_StoreOpr.clear();
}
CDistributedOPR::~CDistributedOPR()
{

}
BOOL CDistributedOPR::GetBuffer(QWORD qwSerialNo,SGMMMsg *pGMMMsg,WORD &wType )
{
    std::map<QWORD,SSaveInfo*>::iterator iter;
    iter = m_StoreOpr.find(qwSerialNo);
    if(iter != m_StoreOpr.end())
    {
        return FALSE;
    }
    SSaveInfo * pSaveInfo;
    pSaveInfo = iter->second;
    pGMMMsg = pSaveInfo->pGMMMsg;
    wType = pSaveInfo->wType;
    delete pSaveInfo;
    return TRUE;
}

void CDistributedOPR::PushBuffer(SGMMMsg * pGMMMsg,QWORD qwSerialNo,WORD wMessageType)
{
    SSaveInfo * pSaveInfo;
    pSaveInfo = new SSaveInfo;
    pSaveInfo->qwSerialNo = qwSerialNo;
    pSaveInfo->wType = wMessageType;
    pSaveInfo->pGMMMsg = pGMMMsg;
    m_StoreOpr.insert(std::map<QWORD,SSaveInfo*>::value_type(qwSerialNo,pSaveInfo));
}

//-----------------------------------------------------------------------
//Add By Lovelonely
//-----------------------------------------------------------------------
extern void AddInfo(LPCSTR Info);

CGM::CGM()
{	
	m_szID[0]='\0';

	m_bIsVisible=TRUE;

	m_iPing=0;
	m_iNum=0;
	m_iMaxPing=0;
	m_iMinPing=1000;

	m_iTimeSkip=0;
}


CGMManager::CGMManager()
{
	if (!LoadGMIP())
	{
		AddInfo("读取gm工具登陆ip限制列表文件失败!!!\r\n");
	}

	if (!LoadGaolList())
	{
		AddInfo("读取监狱名单文件失败!!!\r\n");
	}

	m_GMMap.clear();
	//m_OnlineGMMap.clear();
}

CGMManager::~CGMManager()
{
	if(!SaveGaolList())
	{
		AddInfo("存储监狱名单失败!!!\r\n");
	}
}
//-----------------------------------------------------------------------
void CGMManager::OnGMLog(LPCSTR szID,DNID dnID)
{
	if (IsBadReadPtr(szID, CONST_USERNAME))
		return;

	CGM gm;

	dwt::strcpy(gm.m_szID, szID, CONST_USERNAME);
	gm.m_dnidClient = dnID;

	m_GMMap[szID] = gm;
}
//-----------------------------------------------------------------------
void CGMManager::OnGMOut(LPCSTR szID)
{
	if (IsBadReadPtr(szID, CONST_USERNAME))
		return;

	std::map<std::string,CGM>::iterator itMap=m_GMMap.find(szID);

	if (itMap != m_GMMap.end())
		m_GMMap.erase(itMap);
}
//-----------------------------------------------------------------------
CGM *CGMManager::FindGM(LPCSTR szID)
{
	if (IsBadReadPtr(szID, CONST_USERNAME))
		return NULL;

    std::map<std::string,CGM>::iterator itMap=m_GMMap.find(szID);

	if (itMap == m_GMMap.end())
		return NULL;

	return &(itMap->second);
}
//-----------------------------------------------------------------------
BOOL CGMManager::LoadGMIP()
{
	
 	m_listGMIP.clear();

	dwt::ifstream in("gmip.ini");
	
    if (!in.is_open())
        return FALSE;
	
	while (!in.eof())
	{
		char szTmp[17];

		in>>szTmp;

		if (dwt::strcmp(szTmp,"",17) == 0)
		{
			continue;
		}

		SGMIP sGMIP;
		dwt::strcpy(sGMIP.m_szIP,szTmp,17);

		m_listGMIP.push_back(sGMIP);
		
	}

	in.close();

	return TRUE;
}
//-----------------------------------------------------------------------
BOOL CGMManager::LoadGaolList()
{
	m_listGaol.clear();

	dwt::ifstream in("playergaol.txt");

	if (!in.is_open())
	{
		return FALSE;
	}

	while (!in.eof())
	{
		char szTmp[CONST_USERNAME];

		in>>szTmp;

		if (dwt::strcmp(szTmp, "", CONST_USERNAME) == 0)
		{
			continue;
		}

		SPlayerName sName;
		dwt::strcpy(sName.szName, szTmp, CONST_USERNAME);

		m_listGaol.push_back(sName);
	}

	in.close();

	return TRUE;
}
//-----------------------------------------------------------------------
BOOL CGMManager::SaveGaolList()
{
	std::ofstream out("playergaol.txt");

	if (!out.is_open())
	{
		return FALSE;
	}

	std::list<SPlayerName>::iterator it=m_listGaol.begin();

	while (it != m_listGaol.end())
	{
		out<<it->szName<<std::endl;

		it++;
	}
	
	out.close();

	return TRUE;
}

//-----------------------------------------------------------------------
//gm的ip检查
//-----------------------------------------------------------------------

BOOL CGMManager::CheckGMIP(DNID dnidClient, BOOL isGM)
{
	char szIP[17];

	sockaddr_in addr;
    ZeroMemory(&addr,sizeof(sockaddr_in));

    BOOL ret = 0;
    if ( isGM )
        ret = GetApp()->m_NetGMModule.m_GMServer.GetDnidAddr(dnidClient, &addr, sizeof(sockaddr_in));
    else
        ret = GetApp()->m_PlayerManager.server.GetDnidAddr(dnidClient, &addr, sizeof(sockaddr_in));
	if(ret) 
    {
        char * pChar;
        pChar = inet_ntoa(addr.sin_addr); 
        if(pChar == NULL) 
        {
            return FALSE;
        }
        else
        {
			dwt::strcpy(szIP,inet_ntoa(addr.sin_addr), 17);
        }
    }
    else
    {
        return FALSE;
    }

	std::list<SGMIP>::iterator it=GetApp()->m_GMManager.m_listGMIP.begin();
	
	while (it !=m_listGMIP.end())
	{

		if (dwt::strcmp(it->m_szIP,szIP,17) == 0)
		{
			return TRUE;
		}

		it++;
	}

	return FALSE;
}

//-----------------------------------------------------------------------
//公告消息的定时处理
//-----------------------------------------------------------------------
void CNetGMModule::ProcInfo()
{
	std::list<SPublicInfo>::iterator it;
	it=m_InfoTab.m_listInfo.begin();

	while (it != m_InfoTab.m_listInfo.end())
	{
		it->nTimeAdd++;

		if (it->nTimeAdd >= it->nTime)
		{
			it->nTimeAdd=0;

			m_buffInfoTab.m_listInfo.push_back(*it);	
		}

		it++;
	}
	
	SARefPlayerNumGMMsg aRefPlayerMsg;

    SCollect_LoginServerData &data = *Globals::GetCollectData();
    aRefPlayerMsg.iPlayerNum = data.dwTotalPlayers - data.dwTotalHPlayers - data.dwAutoPlayer;
	aRefPlayerMsg.iPlayerNotOnline = data.dwTotalHPlayers;
    aRefPlayerMsg.iPlayerAuto = data.dwAutoPlayer;

	std::map<std::string,CGM>::iterator itMap=GetApp()->m_GMManager.m_GMMap.begin();

	while (itMap != GetApp()->m_GMManager.m_GMMap.end())
	{
		SendGMMessage(itMap->second.m_dnidClient,&aRefPlayerMsg,sizeof(SARefPlayerNumGMMsg));
		itMap++;
	}
}

//-----------------------------------------------------------------------
void CNetGMModule::SendInfo()
{
	std::list<SPublicInfo>::iterator it=m_buffInfoTab.m_listInfo.begin();

	if (it == m_buffInfoTab.m_listInfo.end())
	{
		return;
	}

	SAPublicInfoGMMsg aPubInfoMsg;

	aPubInfoMsg.wResult=SAPublicInfoGMMsg::RT_SUCCESS;
	aPubInfoMsg.wType=it->wType;
	dwt::strcpy(aPubInfoMsg.szSay,it->szSay,128);

	std::map<std::string,CGM>::iterator itMap=GetApp()->m_GMManager.m_GMMap.begin();

	while (itMap != GetApp()->m_GMManager.m_GMMap.end())
	{
		SendGMMessage(itMap->second.m_dnidClient,&aPubInfoMsg,sizeof(SAPublicInfoGMMsg));

		itMap++;
	}

	SQPublicInfoGMMsg qPubInfoMsg;

	dwt::strcpy(qPubInfoMsg.szSay,it->szSay,128);
	qPubInfoMsg.lClr=it->lClr;
	qPubInfoMsg.nTime=0;
	qPubInfoMsg.wType=it->wType;

    GetApp()->m_ServerManager.Broadcast( &qPubInfoMsg, sizeof(SQPublicInfoGMMsg) );

	m_buffInfoTab.m_listInfo.erase(it);
}
//-----------------------------------------------------------------------

void CNetGMModule::DoGMLogin( SAGMCheckAccountMsg *pMsg )
{
    SAGMLoginMsg msg;
    if ((pMsg->byResult != SAGMCheckAccountMsg::RET_SUCCESS) &&
        (pMsg->byResult != SAGMCheckAccountMsg::RET_LOGGEDIN))
    {
        switch (pMsg->byResult)
        {
        case SAGMCheckAccountMsg::RET_NOCARDPOINT:
            msg.wResult = SAGMLoginMsg ::ERC_NOTENOUGH_CARDPOINT;
            break;

        case SAGMCheckAccountMsg::RET_INVALID_ACCOUNT:
            msg.wResult = SAGMLoginMsg ::ERC_INVALID_ACCOUNT;
            break;

        case SAGMCheckAccountMsg::RET_INVALID_PASSWORD:
            msg.wResult = SAGMLoginMsg ::ERC_INVALID_PASSWORD;
            break;
        case SAGMCheckAccountMsg::RET_BLOCKED:
            msg.wResult = SAGMLoginMsg ::ERC_BLOCKED;
		
        default:
            msg.wResult = SAGMLoginMsg ::ERC_LOGIN_ERROR;
            break;
        }

        // 非认证成功状态下，应该向原客户端说明情况并断开连接
        SendGMMessage(pMsg->dnidClient, &msg, sizeof(SAGMLoginMsg));
        m_GMServer.DelOneClient(pMsg->dnidClient);
    }
    else
    {
		if (pMsg->wGMLevel < 1)
		{
			msg.wResult = SAGMLoginMsg ::ERC_LOGIN_ERROR;

			SendGMMessage(pMsg->dnidClient, &msg, sizeof(SAGMLoginMsg));
			m_GMServer.DelOneClient(pMsg->dnidClient);

			return;
		}

        m_GMServer.SetLinkStatus(pMsg->dnidClient, LinkStatus_Connected);

        try
        {
            lite::Serialreader sl( pMsg->streamData );
            LPCSTR account = sl();
            LPCSTR password = sl();

            SAGMLoginMsg AGMLoginMsg;
            AGMLoginMsg.wResult = pMsg->byResult;
            AGMLoginMsg.wGMLevel = pMsg->wGMLevel;

            lite::Serializer slm( AGMLoginMsg.streamData, sizeof( AGMLoginMsg.streamData ) );
            slm( account );

            GetApp()->m_GMManager.OnGMLog( account, pMsg->dnidClient );

		    rfalse(1,1,"工具GM列表 添加: %s; GM等级: %d", account, pMsg->wGMLevel);

            SetGMLinkContext(pMsg->dnidClient,account,password,pMsg->wGMLevel);
            SendGMMessage(pMsg->dnidClient, &AGMLoginMsg, (WORD)(sizeof(SAGMLoginMsg) - slm.EndEdition()) );
        }
        catch ( lite::Xcpt & )
        {
        }
    }
}

void CNetGMModule::OnRecvGMMsgFromGameServer( DNID dnidClient, SGMMMsg *pGMMMsg, size_t size )
{
    switch(pGMMMsg->_protocol)
    {
	case SGMMMsg::EPRO_GM_MOVETOSAFEPOINT:
		{
			SAMoveToSafePointGMMsg *pMsg  = (SAMoveToSafePointGMMsg*)pGMMMsg;

			SAMoveToSafePointGMMsg aMoveToSafePointMsg;

			dwt::strcpy(aMoveToSafePointMsg.szName, pMsg->szName, CONST_USERNAME);
			aMoveToSafePointMsg.wRet = pMsg->wRet;

			SendGMMessage(pMsg->dnidClient,&aMoveToSafePointMsg,sizeof(SAMoveToSafePointGMMsg));
		}
		break;
   // case SGMMMsg::EPRO_UPDATE_SB_INFO:
   //     {
   //         SAUpdateSBInfoMsg * pAUpdateSBInfoMsg;
   //         pAUpdateSBInfoMsg = (SAUpdateSBInfoMsg *)pGMMMsg;
			//SAUpdateSBInfoMsg aUpdateMsg;
			//aUpdateMsg.wResult = pAUpdateSBInfoMsg->wResult;
			//dwt::strcpy(aUpdateMsg.szName,pAUpdateSBInfoMsg->szName,CONST_USERNAME);
   //         SendGMMessage(pAUpdateSBInfoMsg->dnidClient,&aUpdateMsg,sizeof(SAUpdateSBInfoMsg));
   //     }
   //     break;
    case SGMMMsg::EPRO_GM_TALKMASK:
        {
            SATalkMaskGMMsg *pATalkMaskMsg;
            pATalkMaskMsg = (SATalkMaskGMMsg *)pGMMMsg;
            SATalkMaskGMMsg ATalkMaskGMMsg;
            memcpy(&ATalkMaskGMMsg,pATalkMaskMsg,sizeof(SATalkMaskGMMsg));
            SendGMMessage(pATalkMaskMsg->dnidClient, &ATalkMaskGMMsg,sizeof(SATalkMaskGMMsg));
        }
        break;
    case SGMMMsg::EPRO_GM_TALKUNMASK:
        {
            SATalkUnMaskGMMsg * pATalkUnMaskMsg;
            pATalkUnMaskMsg = (SATalkUnMaskGMMsg *)pGMMMsg;
            SATalkUnMaskGMMsg ATalkUnMaskGMMsg;
            memcpy(&ATalkUnMaskGMMsg,pATalkUnMaskMsg,sizeof(SATalkUnMaskGMMsg));
            SendGMMessage(pATalkUnMaskMsg->dnidClient, &ATalkUnMaskGMMsg,sizeof(SATalkUnMaskGMMsg));
        }
        break;
    case SGMMMsg::EPRO_GM_MOVETOBORN:
        {
            SAMovetoBornMsg * pAMovetoBornMsg;
            pAMovetoBornMsg = (SAMovetoBornMsg *)pGMMMsg;
            SAMovetoBornMsg AMovetoBornMsg;
            memcpy(&AMovetoBornMsg,pAMovetoBornMsg,sizeof(SAMovetoBornMsg));
            SendGMMessage(pAMovetoBornMsg->dnidClient,&AMovetoBornMsg,sizeof(SAMovetoBornMsg));
        }
        break;
    case SGMMMsg::EPRO_GM_CHECKCHAT:
        {
            SAGMCheckChatGMMsg *pAGMCheckChatGMMsg = (SAGMCheckChatGMMsg *)pGMMMsg;
            SAGMCheckChatGMMsg AGMCheckChatGMMsg;
            memcpy(&AGMCheckChatGMMsg,pAGMCheckChatGMMsg,sizeof(SAGMCheckChatGMMsg));

            for ( std::map<DNID, SGMContextInfo>::iterator iter = m_GMContextMap.begin();
                iter != m_GMContextMap.end(); iter++ )
            {
                SendGMMessage( iter->first, &AGMCheckChatGMMsg, sizeof(SAGMCheckChatGMMsg) );
            }
        }
        break;
	case SGMMMsg::EPRO_GM_SAYGLOBAL:
		{
			SASayGlobalGMMsg saSayGlobalMsg;
			SASayGlobalGMMsg *pMsg=(SASayGlobalGMMsg*)pGMMMsg;
			memcpy(&saSayGlobalMsg,pMsg,sizeof(SASayGlobalGMMsg));

            for ( std::map<DNID, SGMContextInfo>::iterator iter = m_GMContextMap.begin();
                iter != m_GMContextMap.end(); iter++ )
            {
                SendGMMessage(iter->first,&saSayGlobalMsg,sizeof(SASayGlobalGMMsg));
            }
		}
		break;
	case SGMMMsg::EPRO_GM_WISPER:
		{
			SQWisperGMMsg *pMsg=(SQWisperGMMsg*)pGMMMsg; //收到区域发过来的密聊消息

			SAWisperGMMsg aWisperGMMsg;

			dwt::strcpy(aWisperGMMsg.szGM, pMsg->szGM, CONST_USERNAME);
			dwt::strcpy(aWisperGMMsg.szPlayer, pMsg->szPlayer, CONST_USERNAME);
			dwt::strcpy(aWisperGMMsg.szSay,pMsg->szSay,50);
			aWisperGMMsg.dwConsumePoint = pMsg->dwConsumePoint;

            try
            {
                lite::Serialreader sl( pMsg->streamData );
                lite::Serializer slm( aWisperGMMsg.streamData, sizeof( aWisperGMMsg.streamData ) );
                slm( sl() );

			    CGM *pGM=GetApp()->m_GMManager.FindGM(pMsg->szGM);

			    if (pGM == NULL)
			    {
				    aWisperGMMsg.wResult=SAWisperGMMsg::RT_ERROR;
                    Globals::SendToGameServer( dnidClient, &aWisperGMMsg, sizeof(SAWisperGMMsg) - slm.EndEdition() );
					return;
			    }

			    aWisperGMMsg.wResult=1;
                SendGMMessage( pGM->m_dnidClient, &aWisperGMMsg, (WORD)(sizeof(SAWisperGMMsg) - slm.EndEdition()) );
            }
            catch ( lite::Xcpt & )
            {
            }
		}
		break;

	case SGMMMsg::EPRO_GM_CLOSE:
		{
			SACloseGMMsg *pMsg=(SACloseGMMsg*)pGMMMsg;

			SACloseGMMsg aCloseMsg;

			if (pMsg->wResult == SACloseGMMsg::RT_CLOSESUC)
			{
				SPlayerName sName;
				dwt::strcpy(sName.szName, pMsg->szName, CONST_USERNAME);
				std::list<SPlayerName>::iterator it = GetApp()->m_GMManager.m_listGaol.begin();
				for ( ; it != GetApp()->m_GMManager.m_listGaol.end(); it++ )
				{
					if (dwt::strcmp(it->szName, pMsg->szName, CONST_USERNAME) == 0)
						break;
				}
				
				if (it == GetApp()->m_GMManager.m_listGaol.end())
					GetApp()->m_GMManager.m_listGaol.push_back(sName);

			}
			else if(pMsg->wResult == SACloseGMMsg::RT_SETFREESUC)
			{
				std::list<SPlayerName>::iterator it=GetApp()->m_GMManager.m_listGaol.begin();

				for ( std::list<SPlayerName>::iterator it=GetApp()->m_GMManager.m_listGaol.begin();
                    it != GetApp()->m_GMManager.m_listGaol.end(); it++ )
				{
					if (dwt::strcmp(it->szName, pMsg->szName, CONST_USERNAME) == 0)
                    {
						GetApp()->m_GMManager.m_listGaol.erase(it);
						break;
                    }
				}
			}
		
			CGM *pGM=GetApp()->m_GMManager.FindGM(pMsg->szGM);

			if (pGM == NULL)
				break;

			memcpy(&aCloseMsg,pMsg,sizeof(SACloseGMMsg));
			
			SendGMMessage(pGM->m_dnidClient,&aCloseMsg,sizeof(SACloseGMMsg));
		}
		break;
	case SGMMMsg::EPRO_GM_CHECKONLINEGM:
		{
			SQCheckOnlineGMGMMsg *pMsg=(SQCheckOnlineGMGMMsg*)pGMMMsg;

			SQCheckOnlineGMGMMsg msg;

			dwt::strcpy(msg.szName, pMsg->szName, CONST_USERNAME);

			msg.wPara=SQCheckOnlineGMGMMsg::PA_RETURN;

			for (int i=0; i < 10; i++)
			{
				msg.szGM[i][0]=NULL;
			}
			
            // 新的GM需求，存在在线不可视和在线可视GM两种
			int j = 0;
			for ( std::map<std::string,CGM>::iterator it = GetApp()->m_GMManager.m_GMMap.begin(); 
                ( j < 5 ) && ( it != GetApp()->m_GMManager.m_GMMap.end() ); it ++ )
			{
                if ( !it->second.m_bIsVisible )
                    continue;

				dwt::strcpy(msg.szGM[j++], it->second.m_szID, CONST_USERNAME);
			}

            /*
			it=GetApp()->m_GMManager.m_OnlineGMMap.begin();

			for (; j < 10; j++)
			{
				if (it == GetApp()->m_GMManager.m_OnlineGMMap.end())
					break;

                dwt::strcpy(msg.szGM[j],it->second.m_szID,CONST_USERNAME);

				it++;
			}
            */

            DNID dnid = GetServerDnidByName( pMsg->szName );
            if ( dnid != INVALID_DNID )
                Globals::SendToGameServer( dnid, &msg, sizeof(msg) );
		};
		break;
	case SGMMMsg::EPRO_GM_GETPING:
		{
			SQGetPingGMMsg *pMsg=(SQGetPingGMMsg*)pGMMMsg;
			
			SAGetPingGMMsg aGetPingMsg;
            aGetPingMsg.iPing = pMsg->iPing;
            aGetPingMsg.ip32 = pMsg->ip32;
            aGetPingMsg.mac64 = pMsg->mac64;

			SendGMMessage(pMsg->dnidClient,&aGetPingMsg,sizeof(SAGetPingGMMsg));
		}
		break;
	case SGMMMsg::EPRO_GM_GETMASKNAME:
		{
			SAGetMaskNameGMMsg *pGetMaskMsg;
			pGetMaskMsg=(SAGetMaskNameGMMsg*)pGMMMsg;
			
			SAGetMaskNameGMMsg aGetMaskMsg;
			aGetMaskMsg.wPara=pGetMaskMsg->wPara;
			aGetMaskMsg.wResult=pGetMaskMsg->wResult;

            SPlayer *pPlayer = GetApp()->m_PlayerManager.FindPlayer( if_name( pGetMaskMsg->szName ) );
			char szTmp[20];
			if (pPlayer == NULL)
			{
				sprintf(szTmp,"nl:%s",pGetMaskMsg->szName);
			}
			else
			{
				dwt::strcpy(szTmp,pPlayer->Data.m_Name,20);
			}

			dwt::strcpy(aGetMaskMsg.szName,szTmp,sizeof( aGetMaskMsg.szName ) );

			SendGMMessage(pGetMaskMsg->dnidClient,&aGetMaskMsg,sizeof(SAGetMaskNameGMMsg ));		
		}
		break;
    }
}
