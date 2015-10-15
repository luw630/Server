//#######################
//# xo checked at 09-24 #
//#######################
#include "StdAfx.h"
#include "netaccountcheckmodule.h"
#include "networkmodule/logmsgs.h"
#include "networkmodule/datamsgs.h"
#include "networkmodule/accountmsgs.h"
//#include "networkmodule/dbopmsgs.h"
#include "networkmodule/cardpointmodifymsgs.h"
#include "networkmodule/refreshmsgs.h"
#include <boost/bind.hpp>
#include <LITESERIALIZER/lite.h>

#include "playermanager.h"
#include "DMainApp.h"
//玩家登录时在登录服务器保存，在区域服务器进入完成后在删除，以减缓区域服务器的登录压力
//std::vector<std::string> playerloginlist;
std::map<std::string, DWORD> playerloginlist;
DWORD  g_dConcurrencyPlayerNum = 0; //并发登陆的玩家数量
DWORD  g_dConcurrencyAllPlayerNum = 0; //并发登陆的玩家总数量
DWORD  g_dConcurrencyDbPlayerNum = 0; //并发数据库中的玩家数量

extern size_t max_player_limit;
extern BOOL TraceInfo(LPCSTR szFileName, LPCSTR szFormat, ...);
static void g_EQVLog(LPCSTR info)
{
    TraceInfo( "EqvInfo.txt", "[%s]%s", Globals::GetStringTime(), info );
}

BOOL QueryPlayerList( LPCSTR szAccount, LPCSTR szPassword, DNID dnidClient )
{
    SQGetCharacterListMsg msg;

    try
    {
        lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
        slm( szAccount );

        msg.dnidClient = dnidClient;
        msg.dwServerId = Globals::GetServerId();

        if ( Globals::SendToDBServer( &msg, sizeof(msg) - slm.EndEdition() ) )
            return true;

        // 通知账号服务器这一次的登录取消原因，同时断开连接;
        SALoginMsg msg;
        msg.dwRetCode = SALoginMsg::ERC_SEND_GETCL_TO_DATASRV_FAIL;
        Globals::SendToPlayer( dnidClient, &msg, sizeof(msg) );
        Globals::CutPlayerLink( dnidClient );
    }
    catch ( lite::Xcpt & )
    {
    }
    return false;
}

static BOOL PlayerLogin(DNID dnidClient, DWORD checkResult, DWORD gmlevel, LPCSTR account, LPCSTR password, LPCSTR userpass, DWORD limitedState, LPCSTR idkey, DWORD online, DWORD offline, QWORD puid)
{
    // 先通知客户端设置是否使用2级密码的标志，避免重绑定后给直接返回了
    SALoginMsg msg;
    msg.dwRetCode = SALoginMsg::ERC_LOGIN_SUCCESS;
    msg.dwEchoIP = (userpass[0] != 0);		// 有没有2级密码呢？
    //Globals::SendToPlayer(dnidClient, &msg, sizeof(msg) - sizeof(msg.szDescription));
	rfalse("账号%s重定向检测", account);
	if (Globals::RebindAccount(dnidClient, account, limitedState, idkey, online, offline))
    {
        rfalse(2, 1, "check account [%s] %d/%d and rebind", account, checkResult, limitedState);
		msg.dwRetCode = SALoginMsg::ERC_ALREADYLOGIN_AND_LINKVALID;
		Globals::SendToPlayer(dnidClient, &msg, sizeof(msg) - sizeof(msg.szDescription)); //修改登录
        return true;
    }
	Globals::SendToPlayer(dnidClient, &msg, sizeof(msg) - sizeof(msg.szDescription)); //修改登录

    // 给该连接附加Context（账号、密码）
	if (!Globals::SetPlayerLinkContext(dnidClient, account, password, userpass, limitedState, gmlevel, idkey, online, offline, puid))
    {
        rfalse(2, 1, "check account [%s] %d SetPlayerLinkContext fail", account, checkResult);
        return false;
    }

    rfalse(2, 1, "check account [%s] %d/%d ok", account, checkResult, limitedState);

    // 认证成功后，需要继续向数据库服务器请求玩家角色列表
	return QueryPlayerList(account, password, dnidClient);
	//SQSelPlayerMsg selMsg;
	//selMsg.byIndex = 1;
	//return GetApp()->m_PlayerManager.SelectCharacter(dnidClient, &selMsg);
}

CNetAccountCheckModule::CNetAccountCheckModule(void)
{
	dwPrev =  timeGetTime();
}

CNetAccountCheckModule::~CNetAccountCheckModule(void)
{
}

void CNetAccountCheckModule::Destroy()
{
}

bool CNetAccountCheckModule::Connect( LPCSTR ip, LPCSTR port )
{
    if ( !client.Connect( ip, port ) )
        return false;
	if ( accountServerIP.empty() )
		accountServerIP = ip;
	if ( accountServerPort.empty() )
		accountServerPort = port;
    SQRegisterLogsrvMsg msg;
    msg.IP = (200<<24)|(200<<16)|(200<<8)|107;
    msg.ID = Globals::GetServerId();

    return client.SendMessage( &msg, sizeof( msg ) ) == TRUE;
}

int CNetAccountCheckModule::Execution()
{
	client.Execution(boost::bind(&CNetAccountCheckModule::Entry, this, _1, _2));

    // 重连
    // 1分钟重连一次
    if ((int)(timeGetTime()-dwPrev) > 1000*60)
    {
        dwPrev = timeGetTime();

        if ( !client.IsConnected() )
        {
            if ( !Connect( accountServerIP.c_str(), accountServerPort.c_str() ) )
            {
                rfalse(2, 1, "帐号服务器[%s]还是连不上-____-bb", accountServerIP.c_str() );
                rfalse(1, 1, "帐号服务器[%s]还是连不上-____-bb", accountServerIP.c_str() );
            }
            else
            {
                rfalse(2, 1, "帐号服务器[%s]重新连上咯^_^", accountServerIP.c_str() );
                rfalse(1, 1, "帐号服务器[%s]重新连上咯^_^", accountServerIP.c_str() );
            }
        }
    }

    static DWORD tickQueueTime = timeGetTime();
    if ( abs( ( int )timeGetTime() - ( int )tickQueueTime ) > 20000 )
    {
        tickQueueTime = timeGetTime();
   /*     if ( GetApp()->m_PlayerManager.GetPlayerCacheCount() < (int)max_player_limit && loginQueueList.size() > 0 )*/
		if (g_dConcurrencyPlayerNum < (int)max_player_limit && loginQueueList.size() > 0)
        {
            // 取得可以进入玩家名额
			size_t intoPlayerCount = max_player_limit - g_dConcurrencyPlayerNum ;
            size_t ps = loginQueueList.size();
            while ( loginQueueList.size() && ( intoPlayerCount > 0 ) )
            {
                SQueueData &data = loginQueueList.front();

                // 先要确定客户端连接有效
                if ( Globals::IsValidPlayerDnid( data.dnidClient ) )
                {
                    if ( PlayerLogin( data.dnidClient, 0, 0, data.account.c_str(), data.password.c_str(), 
                        data.userpass.c_str(), data.limitedState, data.idkey.c_str(), data.online, data.offline, data.puid ) )
                    {
                        // 登入成功后，处理进入人数
                        intoPlayerCount --;
                    }
                }

                loginQueueList.pop_front();
            }

            GetApp()->m_ServerManager.m_wQueuePlayerCount = (WORD)loginQueueList.size();
            if ( ps != loginQueueList.size() )
                UpdateQueuePlayer();
        }
		//UpdateLoginPlayer();
    }

    return 1;
}

bool CNetAccountCheckModule::Entry( LPCVOID data, size_t size )
{
    const SMessage *pMsg = (const SMessage *)data;

    if ( pMsg == NULL )
    {
        // if check server disconnected this link, get in here
        // 设定重连时间，在5秒后启动
        dwPrev = ( timeGetTime() - 1000*60 ) + 5000;
        return 0;
    }

    if ( pMsg->_protocol != SMessage::EPRO_ACCOUNT_MESSAGE )
    {
        /*if ( pMsg->_protocol == SMessage::EPRO_DBOP_MESSAGE )
        {
            SDbopMsg *tempMsg = ( SDbopMsg * )pMsg;

            // 目前没有区域服务器直接向账号数据库请求RPCOP
            //if ( tempMsg->dnidClient != INVALID_DNID )
            //	Globals::SendToGameServer( tempMsg->dnidClient, data, size );
            //else
            ProcessInterop( tempMsg->data, tempMsg->size );
        }
        else */if ( ( ( ( SMessage * )data )->_protocol == SMessage::EPRO_SERVER_CONTROL ) && 
		    ( ( ( SServerCtrlMsg * )data )->_protocol == SServerCtrlMsg::EPRO_GAMESERVER_REQUEST_RPCOP ) )
	    {
		    SAGameServerRPCOPMsg *tempMsg = ( SAGameServerRPCOPMsg * )data;
            tempMsg->srcDatabase = SAGameServerRPCOPMsg::ACCOUNT_DATABASE;
		    if ( tempMsg->dnidClient != INVALID_DNID )
			    // 过来的RPCOP都一定是区域服务器过来的请求，所以直接回传！
			    Globals::SendToGameServer( tempMsg->dnidClient, const_cast< void* >( data ), size );
		    else
                ProcessInterop( tempMsg->streamData );
	    }

        return true;
    }

    // 以前存在超时处理的模块，意义不大，也没有被应用起来，现在已经完全取消掉了
    switch ( ( (SAccountBaseMsg *)pMsg )->_protocol )
    {
    case SAccountBaseMsg::EPRO_CHECK_ACCOUNT:
        RecvCheckAccountMsg( (SACheckAccountMsg*)pMsg );
		break;

    case SAccountBaseMsg::EPRO_REFRESH_CARD_POINT:
        RecvRefreshAccountMsg( (SARefreshCardMsg*)pMsg );
		break;

    case SAccountBaseMsg::EPRO_REFRESH_MONEY_PIONT:
		RecvRefreshMoneyPointMsg( (SALRefreshMoneyPointMsg *)pMsg );
		break;

    case SAccountBaseMsg::EPRO_QUERY_ENCOURAGEMENT:
		RecvQueryEncouragementMsg( (SAQueryEncouragement *)pMsg );
		break;

    case SAccountBaseMsg::EPRO_CHECK_PLAYEREXIST:
		RecvCheckPlayerExistMsg( (SQCheckPlayerExistBaseMsg*)pMsg );
		break;

    case SAccountBaseMsg::EPRO_CHANGE_PASSWORD:
        RecvChangePasswordMsg( (SAChangePasswordMsg*)pMsg );
        break;

    case SAccountBaseMsg::ERPO_BLOCK_ACCOUNT:
        RecvBlockAccountMsg( (SABAMsg *)pMsg );
        break;

    case SAccountBaseMsg::EPRO_GETONLINETIME:
        RecvGetOnlineTimeMsg( (SAGOTMsg*)pMsg );
        break;

    case SAccountBaseMsg::EPRO_GMCHECK_ACCOUNT:
        Globals::RecvGMLoginMsg( (SAGMCheckAccountMsg*)pMsg );
        break;

	case SAccountBaseMsg::EPRO_UPDATE_TIMELIMIT:
        Globals::UpdateTimeLimit( ( SQUpdateTimeLimit* )data, size );
        break;
	}

    return 1;
}

void CNetAccountCheckModule::RecvCheckAccountMsg(SACheckAccountMsg *pMsg)
{
    // 判断该连接是否已经无效
	if (!Globals::IsValidPlayerDnid(pMsg->dnidClient))
    {
		// 如果已经无效，那么本次操作已经不能继续下去了
		return;
    }

	LPCSTR account	= NULL;
    LPCSTR password = NULL;
    LPCSTR userpass = NULL;
    LPCSTR userdesc = NULL;
    LPCSTR idkey	= NULL;

    // 该帐号是否纳入防沉迷系统内
    DWORD limitedState = 0, online = 0, offline = 0;  
    extern BOOL netLimited;

    try
    {
        lite::Serialreader sl(pMsg->streamData);
        account		= sl();
        password	= sl();
        userpass	= sl();
        userdesc	= sl();
		rfalse("账号验证通过%s", account);
        // 在有效的条件下获取剩余的沉迷时间
        if (netLimited && (sl.curSize() < sl.maxSize()))
		{
            limitedState = true;
			idkey = sl();
			online = sl();
			offline = sl();
		}
    }
    catch (lite::Xcpt &)
    {
        // 解码序列化数据中出现异常，放弃对该消息的继续操作
        return;
    }

	char tempAccount[256];
    if (!accountPrefix.empty()) 
	{
        _snprintf(tempAccount, sizeof(tempAccount) - 1, "%s\x1e%s", accountPrefix.c_str(), account);
        account = tempAccount;
	}

	// 此处做一下修改：
	// 账号检测成功通过不管是RET_SUCCESS还是RET_LOGGEDIN
    // 都直接先做 RebindAccount 将账号重新绑定到可能的已存在角色上
    // 如果成功，则直接进入游戏
    // 如果失败，则请求角色列表
	if (pMsg->byResult == SACheckAccountMsg::RET_SUCCESS || pMsg->byResult == SACheckAccountMsg::RET_LOGGEDIN)
    {
        // 检查是否排队
		if (g_dConcurrencyPlayerNum >= (int)max_player_limit )
		{
            if (!GetApp()->m_GMManager.CheckGMIP(pMsg->dnidClient, false))
            {
                if ( account == NULL || password == NULL )
                    return ;

                // 如果不在游戏中，同时服务器达到人数上限，则该账号不能够登陆
                SPlayer *data = GetApp()->m_PlayerManager.FindPlayer( account );
				if (data == NULL)
                {
                    SQueueData data;
                    data.account = account;
                    data.password = password;
                    data.userpass = ( userpass == NULL ) ? "" : userpass;
                    data.idkey = ( idkey == NULL ) ? "" : idkey;
                    data.online = online;
                    data.offline = offline;
                    data.limitedState = limitedState;
                    data.dnidClient = pMsg->dnidClient;
                    data.puid = pMsg->qwPuid;

                    loginQueueList.push_back( data );
                    GetApp()->m_ServerManager.m_wQueuePlayerCount = (WORD)loginQueueList.size();

                    // 设置状态为登陆等待
                   //  GetApp()->m_PlayerManager.server.SetLinkStatus( pMsg->dnidClient, LinkStatus_QueueWaite );

                    // 通知客户端排队等待
                    SALoginMsg msg;
                    msg.dwRetCode = SALoginMsg::ERetCode::ERC_QUEUEWAIT;
                    msg.dwEchoIP = (DWORD)loginQueueList.size(); // 有没有2级密码呢？
                    Globals::SendToPlayer( pMsg->dnidClient, &msg, sizeof( msg ) - sizeof( msg.szDescription ) );
                    return;
                }
            }
		}
        PlayerLogin(pMsg->dnidClient, pMsg->byResult, pMsg->wGMLevel, account, password, userpass, limitedState, idkey, online, offline, pMsg->qwPuid);
		
    }
    else
    {
        SALoginMsg msg;
		strncpy(msg.szDescription, userdesc, sizeof(msg.szDescription));
        msg.szDescription[sizeof(msg.szDescription) - 1] = 0;

		if (pMsg->byResult < SALoginMsg::ERC_SPLIT)
		{
			switch (pMsg->byResult)
			{
			case SACheckAccountMsg::RET_NOCARDPOINT:
				msg.dwRetCode = SALoginMsg::ERC_NOTENOUGH_CARDPOINT;
				break;

			case SACheckAccountMsg::RET_INVALID_ACCOUNT:
			case SACheckAccountMsg::RET_INVALID_PASSWORD:
				msg.dwRetCode = SALoginMsg::ERC_INVALID_ACCOUNT;
				break;

			case SACheckAccountMsg::RET_BLOCKED:
				msg.dwRetCode = SALoginMsg::ERC_BLOCKED;
				break;

			case SACheckAccountMsg::RET_LIMITED:
				msg.dwRetCode = SALoginMsg::ERC_LIMITED;
				break;

			case SACheckAccountMsg::RET_LOGIN_OTHER_SERVER:
				msg.dwRetCode = SALoginMsg::ERC_LOGIN_ALREADY_LOGIN;
				break;

			default:
				msg.dwRetCode = SALoginMsg::ERC_LOGIN_ERROR;
				break;
			}
		}
		else
			msg.dwRetCode = pMsg->byResult;

        rfalse(2, 1, "login err - %d", pMsg->byResult);

        // 非认证成功状态下，应该向原客户端说明情况并断开连接
        Globals::SendToPlayer(pMsg->dnidClient, &msg, sizeof(msg));
        Globals::CutPlayerLink(pMsg->dnidClient);
        return;
    }

    return;
}

void CNetAccountCheckModule::RecvChangePasswordMsg(SAChangePasswordMsg *pMsg)
{
 //   SAChangePassGMMsg AChangePassGMMsg;
 //   AChangePassGMMsg.wResult = pMsg->wResult;
	//dwt::strcpy(AChangePassGMMsg.szAccount,pMsg->szAccount,ACCOUNTSIZE);
 //   memcpy(AChangePassGMMsg.szNewPass,pMsg->szNewPassword,CONST_USERNAME);
 //   memcpy(AChangePassGMMsg.szOldPass,pMsg->szOldPassword,CONST_USERNAME);

 //   GetApp()->m_NetGMModule.SendGMMessage(pMsg->dnidClient,&AChangePassGMMsg,sizeof(SAChangePassGMMsg));
}

void CNetAccountCheckModule::RecvBlockAccountMsg( SABAMsg *pMsg )
{
    //try
    //{
    //    lite::Serializer sl( pMsg->streamData );
    //    LPCSTR account = sl();

    //    SABlockAccountMsg msg;
    //    msg.wResult = pMsg->wResult;
    //    msg.dwBlockDuration = pMsg->dwBlockTime;

    //    lite::Serializer slm( msg.streamData, sizeof(msg.streamData) );
    //    slm( account );


    //    GetApp()->m_NetGMModule.SendGMMessage( pMsg->dnidClient, &msg, sizeof(msg) );
    //}
    //catch ( lite::Xcpt &e )
    //{
    //}
}

void CNetAccountCheckModule::RecvGetOnlineTimeMsg(SAGOTMsg *pMsg)
{
    //SAGetOnlineTimeMsg AGetOnlineTimeMsg;
    //AGetOnlineTimeMsg.wResult = pMsg->wResult ;
    //dwt::strcpy(AGetOnlineTimeMsg.szAccount,pMsg->szAccount,ACCOUNTSIZE);
    //AGetOnlineTimeMsg.iOnlineTime = pMsg->iOnlineTime;

    //GetApp()->m_NetGMModule.SendGMMessage(pMsg->dnidClient,&AGetOnlineTimeMsg,sizeof(SAGetOnlineTimeMsg));
}


void CNetAccountCheckModule::RecvRefreshAccountMsg(struct SARefreshCardMsg *pMsg)
{
    //if ( pMsg->byRefreshType != SARefreshCardMsg::QUEST_REFRESH )
    //    return;

    //SPlayer *pPlayer = GetApp()->m_PlayerList.GetData(pMsg->szAccount);
    //if (pPlayer == NULL)
    //    return;

    //SServer *pServer = GetApp()->m_ServerList.GetData(pPlayer->dwRegionServerKey);
    //if (pServer == NULL)
    //    return;

    //if (pMsg->byResult == SARefreshCardMsg::RET_POINT_NOTENOUGH)
    //{
    //    SQCutGMMsg msg;
    //    msg.szGM[0] = 0;
    //    dwt::strcpy(msg.szName, pPlayer->Data.m_szName, CONST_USERNAME);
    //    GetApp()->m_SServer.SendMessage(pServer->idControl, &msg, sizeof(SQCutGMMsg));
    //}
    //else
    //{
    //    SARefreshCardPointMsg rcp;
    //    rcp.dwGID = pPlayer->dwGlobalID;
    //    rcp.dwCardPoint = pMsg->dwCardPoint;
    //    // 这里增加ADDHERE
    //    GetApp()->m_SServer.SendMessage(pServer->idControl, &rcp, sizeof(rcp));
    //}
}

void CNetAccountCheckModule::RecvRefreshMoneyPointMsg(SALRefreshMoneyPointMsg *pMsg)
{
    //if(pMsg == NULL)
    //    return;

    //SMoneyPointNotify mpn;
    //ZeroMemory(&mpn, sizeof(SMoneyPointNotify));

    //SPlayer *pPlayer = GetApp()->m_PlayerList.GetData(pMsg->szAccount);
    //if (pPlayer == NULL)
    //    return;

    //SServer *pServer = GetApp()->m_ServerList.GetData(pPlayer->dwRegionServerKey);
    //if (pServer == NULL)
    //    return;

    //// 刷新登陆服务器的元宝积分
    //if(pMsg->dwRet == SALRefreshMoneyPointMsg::R_COMPLETE)
    //{
    //    pPlayer->dwMoneyPoint = pMsg->dwMoneyPoint;
    //    return;
    //}

    //if(GetApp()->GetQuestMoneyPoint(pMsg->szAccount, mpn) == false)
    //{
    //    pMsg->dwRet = SARefreshMoneyPointMsg::R_TIMEOUT;
    //}

    //// 这里要把元宝请求的相关操作情况发回给区域服务器
    //SARefreshMoneyPointMsg msg;
    //msg.dwGID = pPlayer->dwGlobalID;
    //msg.dwMoneyPoint = pMsg->dwMoneyPoint;
    //msg.dwRet = pMsg->dwRet;
    //msg.mpn = mpn;	

    //GetApp()->m_SServer.SendMessage(pServer->idControl, &msg, sizeof(msg));
}

void CNetAccountCheckModule::ProcessInterop( void *data )
{
    char bufmsg[1024];

    try
    {
        lite::Serialreader sl( data );

        switch ( (int)sl() )
        {
        case 0://RPC_ERROR:
            {
                LPCSTR spname = sl();

                TraceInfo( "rpcerr.log", "[%s]执行存储过程(%s)出现错误！", Globals::GetStringTime(), spname );

                _snprintf( bufmsg, 1023, "RPC ERROR ON EXECUTE STOREDPROCDURE: %s", spname );
                rfalse( 2, 1, bufmsg );
            }
            break;

        case SMessage::EPRO_SYSTEM_MESSAGE:
            // 子消息switch
            switch ( (int)sl() )
            {
            case _SDataBaseMsg::EPRO_CREATE_CHARACTER:
                {
                    int ret = sl();
                    lite::Variant lvt = sl();
                    LPCVOID ptr = lvt._pointer;
                    DWORD size = lvt.dataSize;
                    if ( ( lvt.dataType == lite::Variant::VT_POINTER ) && 
                        ( size < sizeof( SQCreatePlayerMsg ) ) && 
                        ( size > offsetof( SQCreatePlayerMsg, streamData ) ) )
                    {
                        SQCreatePlayerMsg *pMsg = ( SQCreatePlayerMsg* )ptr;
                        if ( ret == 0 )
                        {
                            if ( !Globals::SendToDBServer( pMsg, size ) )
                            {
                                SACrePlayerMsg msg;
                                msg.byResult = SACrePlayerMsg::ERC_UNHANDLE;
                                Globals::SendToPlayer( pMsg->dnidClient, &msg, sizeof(msg) );
                            }
                        }
                        else
                        {
                            SACrePlayerMsg msg;
                            msg.byResult = ( ret == 1 ) ? SACrePlayerMsg::ERC_NAMETAKEN : SACrePlayerMsg::ERC_UNHANDLE;
                            Globals::SendToPlayer( pMsg->dnidClient, &msg, sizeof(msg) );
                        }
                    }
                }
                break;
            }
            break;

        case SMessage::EPRO_POINTMODIFY_MESSAGE:

            // 子消息switch
            switch ( (int)sl() )
            {
            case SPointModifyMsg::EPRO_EQUIVALENT_BASE:

                // 子消息switch
                switch ( (int)sl() )
                {
                case SEquivalentModifyMsg::EPRO_DEC_EQUIVALENT:
                    {
//                         int type = sl();
//                         LPCSTR account = sl();
// 			            int consumable = sl();
// 			            int itemIndex = sl();
// 			            int itemNumber = sl();
//                         int decPoint = sl();
//                         DNID dnid = sl();
//                         int ret = sl();
// 
//                         SADecEQVPointMsg msg;
// 
//                         msg.consumable = (CONSUMABLETYPE)consumable;
//                         msg.type = (EQUIVALENT)type;
//                         msg.nIndex = itemIndex;
//                         msg.nNumber = itemNumber;
//                         msg.dwDecPoint = decPoint;
//                         msg.dwEQVPoint = ret;
// 
//                         lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
//                         slm( account );
// 
//                         if ( ret < 0 )
//                         {
//                             // 交易调用失败
//                             // -1可能是账号不存在
//                             // -2可能是插入交易记录失败，做记录，都未扣钱
//                             //  decmsga.dwDecPoint = 0; //失败的话消费点数为零
//                             if ( ret == -1 )
//                             {
//                                 g_EQVLog("交易调用失败，可能是账号不存在！");
//                             }
//                             else if ( ret == -2 )
//                             {
//                                 g_EQVLog("交易调用失败，可能是插入交易记录失败！");
//                             }
//                             else
//                             {
//                                 g_EQVLog("交易调用失败，可能是数据错误！");
//                             }
// 
//                             return;
//                         }
// 
//                         Globals::SendToGameServer( dnid, &msg, sizeof(msg) - slm.EndEdition() );
                    }
                    break;

                case SEquivalentModifyMsg::EPRO_GET_EQUIVALENT:
                    {
//                         int type = sl();
//                         LPCSTR account = sl();
//                         DNID dnid = sl();
//                         int point = sl();
// 
//                         SAGetEQVPointMsg msg;
// 
//                         msg.dwEQVPoint = point;
//                         msg.type = (EQUIVALENT)type;
// 
//                         lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
//                         slm( account );
// 
//                         Globals::SendToGameServer( dnid, &msg, sizeof(msg) - slm.EndEdition() );
                    }
                    break;
                }
                break;
            }
            break;
        }

        // end of the main switch
    }
    catch ( lite::Xcpt &e )
    {
        TraceInfo( "rpcerr.log", "[%s]解析RPC结果出现错误：%s", Globals::GetStringTime(), e.GetErrInfo() );
        rfalse( 2, 1, e.GetErrInfo() );
        return;
    }
}

//void CNetAccountCheckModule::ProcessInterop(LiteInterop::CLiteInterop *lpLiop)
//{
//    LiteInterop::CLiteInterop Liop(lpLiop);
//
//    try
//    {
//        LiteInterop::CLiteVariant lvtOpType = Liop;
//
//		char bufmsg[1024];
//
//		//_snprintf(bufmsg, 1023, "Operator Type: %d", (int)lvtOpType);
//		//rfalse(2, 1, bufmsg);
//
//        switch ((int)lvtOpType)
//        {
//		case RPC_ERROR:
//			{
//				LPCSTR spname = (LiteInterop::CLiteVariant &)Liop;
//
//				TraceInfo("rpcerr.log", "[%s]执行存储过程(%s)出现错误！", GetStringTime(), spname);
//
//				_snprintf(bufmsg, 1023, "RPC ERROR ON EXECUTE STOREDPROCDURE: %s", spname);
//				rfalse(2, 1, bufmsg);
//			}
//			break;
//
//        case SMessage::EPRO_POINTMODIFY_MESSAGE:
//            lvtOpType = Liop;
//			int valOptype = (int)lvtOpType;
//
//			//_snprintf(bufmsg, 1023, "Operator Type 2nd: %d", (int)lvtOpType);
//			//rfalse(2, 1, bufmsg);
//
//            switch ((int)lvtOpType)
//            {-
//            case SPointModifyMsg::EPRO_PREPARE_EXCHANGE:
//                {
//                    int ret = (LiteInterop::CLiteVariant &)Liop;
//                    DWORD dwTime = (LiteInterop::CLiteVariant &)Liop;
//                    DWORD dwSrcGID = (LiteInterop::CLiteVariant &)Liop;
//                    DWORD dwDstGID = (LiteInterop::CLiteVariant &)Liop;
//                    DWORD dwSrcPoint = (LiteInterop::CLiteVariant &)Liop;
//                    DWORD dwDstPoint = (LiteInterop::CLiteVariant &)Liop;
//
//                    GetApp()->ProcessPrepareExchangeResult(ret, dwTime, dwSrcGID, dwDstGID, dwSrcPoint, dwDstPoint);
//                }
//                break;
//			case SPointModifyMsg::EPRO_EQUIVALENT_BASE:
//				{
//					lvtOpType = Liop;
//					int ivalOptype = (int)lvtOpType;
//
//					//_snprintf(bufmsg, 1023, "Operator Type 3rd: %d", (int)ivalOptype);
//					//rfalse(2, 1, bufmsg);
//
//					DWORD etype = (LiteInterop::CLiteVariant &)Liop;
//					LPCSTR account = (LiteInterop::CLiteVariant &)Liop;
//
//					if(ivalOptype == SEquivalentModifyMsg::EPRO_DEC_EQUIVALENT)
//					{
//						DWORD ctype = (LiteInterop::CLiteVariant &)Liop;
//
//						SADecEQVPointMsg decmsga;
//
//						decmsga.consumable = (CONSUMABLETYPE)ctype;
//						decmsga.type = (EQUIVALENT)etype;
//						decmsga.nIndex = (LiteInterop::CLiteVariant &)Liop;
//						decmsga.dwDecPoint = (DWORD)((LiteInterop::CLiteVariant&)Liop); //扣掉的点数
//
//						decmsga.dwEQVPoint = (LiteInterop::CLiteVariant &)Liop;
//
//						if (decmsga.dwEQVPoint < 0)
//						{
//							// 交易调用失败
//							// -1可能是账号不存在
//							// -2可能是插入交易记录失败，做记录，都未扣钱
//                          //  decmsga.dwDecPoint = 0; //失败的话消费点数为零
//							if(decmsga.dwEQVPoint == -1)
//							{
//								g_EQVLog("交易调用失败，可能是账号不存在！");
//							}
//							else if(decmsga.dwEQVPoint == -2)
//							{
//								g_EQVLog("交易调用失败，可能是插入交易记录失败！");
//							}
//							else
//							{
//								g_EQVLog("交易调用失败，可能是数据错误！");
//							}
//
//							return;
//						}
//
//						dwt::strcpy(decmsga.szAccount, account, ACCOUNTSIZE);
//
//						SPlayer *pPlayer = GetApp()->m_PlayerList.GetData(account);
//						if (pPlayer == NULL)
//						{
//							// 玩家未找到，交易失败，但是钱已经扣了，做记录
//							char err[128] = {0};
//
//							_snprintf(err, 127, "交易失败，玩家未找到，但是钱已经扣了，账号为: %s", account);
//							g_EQVLog(err);
//
//							return;
//						}
//						
//						if(decmsga.type == EEQ_XYDPOINT)
//						{
//							pPlayer->dwXYDPoint = decmsga.dwEQVPoint;
//						}
//						else if(decmsga.type == EEQ_GIFTCOUPON)
//						{
//							pPlayer->dwGiftCoupon = decmsga.dwEQVPoint;
//						}
//
//						SServer *pServer = GetApp()->m_ServerList.GetData(pPlayer->dwRegionServerKey);
//						if (pServer == NULL)
//						{
//							// 区域服务器未找到，可能是区域服务器当机或者是orb错误，但是钱已经扣了，做记录
//							char err[128] = {0};
//
//							_snprintf(err, 127, "交易失败，区域未找到，可能是区域错误，但是钱已经扣了，账号为: %s", account);
//							g_EQVLog(err);
//
//							return;
//						}
//
//						GetApp()->m_SServer.SendMessage(pServer->idControl, &decmsga, sizeof(SADecEQVPointMsg));
//					}
//					else if(ivalOptype == SEquivalentModifyMsg::EPRO_GET_EQUIVALENT)
//					{
//						DWORD ret = (LiteInterop::CLiteVariant &)Liop;
//
//						//_snprintf(bufmsg, 1023, "Result: %d, Account: %s", ret, account);
//						//rfalse(2, 1, bufmsg);
//
//						SPlayer *pPlayer = GetApp()->m_PlayerList.GetData(account);
//						if (pPlayer == NULL)
//							return;
//
//						SServer *pServer = GetApp()->m_ServerList.GetData(pPlayer->dwRegionServerKey);
//						if (pServer == NULL)
//							return;
//
//						SAGetEQVPointMsg emsg;
//
//						emsg.dwEQVPoint = ret;
//						emsg.type = (EQUIVALENT)etype;
//						dwt::strcpy(emsg.szAccount, account, ACCOUNTSIZE);
//						
//						if(emsg.type == EEQ_XYDPOINT)
//						{
//							pPlayer->dwXYDPoint = emsg.dwEQVPoint;
//						}
//						else if(emsg.type == EEQ_GIFTCOUPON)
//						{
//							pPlayer->dwGiftCoupon = emsg.dwEQVPoint;
//						}
//
//						GetApp()->m_SServer.SendMessage(pServer->idControl, &emsg, sizeof(SAGetEQVPointMsg));
//					}
//				}
//				break;
//            }
//            break;
//        }
//    }
//    catch (LiteInterop::XcptBase &err)
//    {
//		TraceInfo("rpcerr.log", "[%s]解析RPC结果出现错误：%s", GetStringTime(), err.GetErrInfo());
//		rfalse(2, 1, err.GetErrInfo());
//        return;
//    }
//}

//抽奖消息
void CNetAccountCheckModule::RecvQueryEncouragementMsg(struct SAQueryEncouragement *pMsg)
{
	if ( NULL == pMsg ) 
        return;

	SAEncouragement msg;
	dwt::strcpy( msg.m_szName, pMsg->m_szName, sizeof(msg.m_szName) );
	msg.m_iRet = pMsg->m_iRet;

    Globals::SendToGameServer( pMsg->dnidClient, &msg, sizeof(msg) );

	//std::map< DWORD, dwt::resource<SServer, DWORD> >::iterator it = GetApp()->m_ServerList._depository.begin();
	//while (it != GetApp()->m_ServerList._depository.end())
	//{
	//	SServer *pS = &it->second._data;
	//	GetApp()->m_SServer.SendMessage(pS->idControl, &Amsg, sizeof(Amsg));
	//	it++;
	//}
}

void CNetAccountCheckModule::RecvCheckPlayerExistMsg(SQCheckPlayerExistBaseMsg *pMsg)
{
	//if(IsBadReadPtr(pMsg->szAccount,ACCOUNTSIZE))
	//	return;

	//SACheckPlayerExistBaseMsg msg;

	//dwt::strcpy(msg.szAccount, pMsg->szAccount, sizeof(msg.szAccount));

	//extern __int32 serverid;

	//SPlayer *pPlayer = GetApp()->m_PlayerList.GetData(pMsg->szAccount);
	//if (pPlayer != NULL)
	//{
	//	msg.iDestServerID = pMsg->iDestServerID;
	//	msg.iSrcServerID = serverid;
	//	msg.wExist = 1;
	//}
	//else
	//{
	//	msg.iDestServerID = pMsg->iDestServerID;
	//	msg.iSrcServerID = 0;
	//	msg.wExist = 0;
	//}

	//if(!GetApp()->m_AccountServer.SendMsgToAccountSrv(&msg, sizeof(SACheckPlayerExistBaseMsg)))
	//{
	//	// send error, log error message here
	//}
}

BOOL CNetAccountCheckModule::Reconnect()
{
	if ( client.IsConnected() )
        return TRUE;

	return Connect( accountServerIP.c_str(), accountServerPort.c_str() );		
}

void CNetAccountCheckModule::UpdateQueuePlayer()
{
    std::list< SQueueData >::iterator iter = loginQueueList.begin();
    size_t index = 1;
    for ( iter; iter != loginQueueList.end(); )
    {
		if (!Globals::IsValidPlayerDnid(iter->dnidClient))
		{
			PopLoginPlayer(iter->account);
			iter = loginQueueList.erase( iter );
		}
        else
        {
            SALoginMsg msg;
            msg.dwRetCode = SALoginMsg::ERetCode::ERC_QUEUEWAIT;
            msg.dwEchoIP = (DWORD)index;

            Globals::SendToPlayer( iter->dnidClient, &msg, sizeof( msg ) - sizeof( msg.szDescription ) );
            index ++;
	        iter++;
        }
    }
}

void CNetAccountCheckModule::UpdateLoginPlayer()
{
	int nsyplayernum = GetApp()->m_ServerManager.GetPlayerNumber(0);//获取异常玩家数量
	if (nsyplayernum > 0 )
	{
		if (g_dConcurrencyPlayerNum >= nsyplayernum)
		{
			g_dConcurrencyPlayerNum -= nsyplayernum;
		}
		else
		{
			g_dConcurrencyPlayerNum = 0;
		}
		rfalse(2, 1, "清除异常玩家 %d",nsyplayernum);
	}
	
}

void CNetAccountCheckModule::PopLoginPlayer(std::string strcount)
{
	std::map<std::string, DWORD>::iterator iter = playerloginlist.find(strcount);
	if (iter != playerloginlist.end())
	{
		iter = playerloginlist.erase(iter);
		if (g_dConcurrencyPlayerNum > 0)
		{
			g_dConcurrencyPlayerNum--;
		}
	}
}
