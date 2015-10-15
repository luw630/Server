#include "StdAfx.h"
#include "ServerManager.h"
#include "PlayerManager.h"
#include "DMainApp.h"
#include <boost/bind.hpp>
#include "NetWorkModule/logmsgs.h"
#include "NetWorkModule/refreshmsgs.h"
#include "NetWorkModule/CardPointModifyMsgs.h"
#include "pub/rpcop.h"
#include "NetWorkModule/accountmsgs.h"
#include "NetWorkModule/tongmsgs.h"
#include "networkmodule/FactionBBSMsgs.h"
#include <LITESERIALIZER/lite.h>
#include <pub/TraceInfo.h>
#include "Networkmodule/NetCollecterModule/CollectDataDef.h"
#include "networkmodule/BuildingTypedef.h"
#include "networkmodule/UnionMsgs.h"
#include "networkmodule/SectionMsgs.h"

extern size_t max_player_limit;
dwt::stringkey< char[256] > timeSegment;

extern std::map<std::string, DWORD> playerloginlist;
extern DWORD  g_dConcurrencyPlayerNum;
extern DWORD g_dConcurrencyDbPlayerNum;
extern DWORD g_dConcurrencyAllPlayerNum;
namespace Globals
{
	void SendUpdateTimeLimit(SQUpdateTimeLimit *pMsg);
}
static CNetProviderModule* currentServer = NULL;
CServerManager::CServerManager()
{
	m_wQueuePlayerCount = 0;
	SYSTEMTIME s;
	GetLocalTime(&s);
	sprintf(timeSegment, "[%d-%d-%d]", s.wYear, s.wMonth, s.wDay);
}

void CServerManager::Destroy()
{
}

BOOL CServerManager::Host(LPCSTR port)
{
	m_dwCurPlayerNumber = 0;
	max_players = 0;
	return server.Create(port);
}

void CServerManager::Execution()
{
	server.Execution(boost::bind(&CServerManager::Entry, this, _1, _2, _3, _4));

	static DWORD nextTime = timeGetTime();

	WORD autoFightPlayers = 0;
	if ((int)timeGetTime() - (int)nextTime > 20000)
	{
		nextTime = timeGetTime();

		// 采集数据
		SCollect_LoginServerData &data = *Globals::GetCollectData();
		memset(&data, 0, sizeof(data));

		Globals::eServerStatus ss = gameServers.empty() ? Globals::SS_PROBLEM : Globals::SS_NORMAL;

		for (std::map<DNID, SServer>::iterator it = gameServers.begin();
			it != gameServers.end(); it++)
		{
			SServer *pServer = &it->second;

			data.dwAvePing += pServer->dwAvePing;
			data.i64TotalSpendMoney += pServer->i64TotalSpendMoney;
			data.dwTotalHPlayers += pServer->wPlayers[3];
			data.dwInFlowPS += pServer->dwInFlowPS;
			data.dwOutFlowPS += pServer->dwOutFlowPS;
			data.dwCreatedSpecialItem += pServer->dwCreatedSpecItem;
			data.dwUseSpecialItem += pServer->dwUseSpecItem;

			data.dwTotalPlayers += pServer->wTotalPlayers;
			autoFightPlayers = (WORD)(m_dwCurPlayerNumber - (pServer->wPlayers[0] + pServer->wPlayers[1] + pServer->wPlayers[2] + pServer->wPlayers[3]));
			if (pServer->idControl == 0)
				ss = Globals::SS_PROBLEM;
		}

		m_dwCurPlayerNumber = data.dwTotalPlayers;

		if (max_players < m_dwCurPlayerNumber)
			max_players = m_dwCurPlayerNumber;

		data.dwMaxPlayers = max_players;
		data.dwAutoPlayer = autoFightPlayers;

		// 自动挂机人数


		// 刷新区域服务器数据
		QueryRefreshGameServer();

		// 向账号服务器提交服务器状态信息
		SUpdateServerState msg;
		msg.ServerID = Globals::GetServerId();
		msg.PlayerNumber = (m_dwCurPlayerNumber & 0xffff) | (autoFightPlayers << 16);
		msg.ServerState = ss << 16;
		msg.ServerState |= data.dwTotalPlayers - data.dwTotalHPlayers;
		msg.TotalMac = msg.PlayerNumber;
		memset(msg.streamData, 0, MAX_ONLINEEXTEND);
		Globals::SendToAccountServer(&msg, sizeof(msg), reinterpret_cast<LPCSTR>(-1));

		// 更新查询用的服务器状态信息
		// extern size_t max_player_limit;
		UpdateServerStatus(ss, m_dwCurPlayerNumber, m_wQueuePlayerCount, (int)max_player_limit);

		// 重设在线人数，用于排队
		GetApp()->m_PlayerManager.m_onlineNumber = data.dwTotalPlayers - data.dwTotalHPlayers;
	}
}

enum RPC_TYPE
{
	RPC_ERROR,
	RPC_TEST,
};

static void DecEQVPointMsg(DNID dnid, SQDecEQVPointMsg *pMsg)
{
	//     LPCSTR account = NULL;
	//     try
	//     {
	//         lite::Serialreader slm( pMsg->streamData );
	//         account = slm();
	//     }
	//     catch ( lite::Xcpt & )
	//     {
	//         return;
	//     }
	// 
	//     LPCSTR spname = NULL;
	//     if ( pMsg->type == EEQ_XYDPOINT )
	//     {
	// 		if ( pMsg->dwEQVPoint <= 0 )
	// 			return;
	// 
	//         spname = "AccountManager.sp_SetXYDPointNewXYD";
	//     }
	// 	else if ( pMsg->type == EEQ_GIFTCOUPON_NEWXYD )
	//     {
	//         spname = "AccountManager.sp_SetGiftCoupoNewXYD";
	//     }
	//     else
	//     {
	//         return;
	//     }
	// 
	//     LPCSTR prefix = NULL;
	//     LPCSTR abs_account = Globals::SplitAccount( account, prefix );
	// 
	//     try
	//     {
	//         SQGameServerRPCOPMsg msg;
	//         msg.dstDatabase = SQGameServerRPCOPMsg::ACCOUNT_DATABASE;
	//         msg.dnidClient = INVALID_DNID;
	//         lite::Serializer slmaker( msg.streamData, sizeof( msg.streamData ) );
	// 
	//         slmaker
	// 
	//             // 注册出现异常时返回的数据（集）
	//             [OP_BEGIN_ERRHANDLE] ( (int)RPC_ERROR ) ( spname ) [OP_END_ERRHANDLE]
	// 
	//             // 准备存储过程
	//             [OP_PREPARE_STOREDPROC] (spname)
	// 
	//                 // 设定调用参数
	//                 [OP_BEGIN_PARAMS]
	//                 (1) ( abs_account )
	//                     (2) ( -( pMsg->dwEQVPoint * pMsg->nNumber ) )
	//                     (3) ( (int)pMsg->consumable )
	//                     (4) ( (int)pMsg->nIndex )
	//                     (5) ( (int)pMsg->nLevel )
	//                     (6) ( (int)Globals::GetServerId() )
	//                     [OP_END_PARAMS]
	// 
	//                     // 调用存储过程
	//                     [OP_CALL_STOREDPROC]
	// 
	//                     // 初始化返回数据盒
	//                     [OP_INIT_RETBOX]    (1024)
	// 
	//                         // 数据盒的第一个值
	//                         [OP_BOX_VARIANT]    ( (int)SMessage::EPRO_POINTMODIFY_MESSAGE )
	//                         [OP_BOX_VARIANT]    ( (int)SPointModifyMsg::EPRO_EQUIVALENT_BASE )
	//                         [OP_BOX_VARIANT]    ( (int)SEquivalentModifyMsg::EPRO_DEC_EQUIVALENT )
	//                         [OP_BOX_VARIANT]    ( (int)pMsg->type )
	//                         [OP_BOX_VARIANT]    ( account )
	//                         [OP_BOX_VARIANT]    ( (int)pMsg->consumable )
	//                         [OP_BOX_VARIANT]    ( pMsg->nIndex )
	//                         [OP_BOX_VARIANT]	( pMsg->nNumber )
	//                         [OP_BOX_VARIANT]	( pMsg->dwEQVPoint * pMsg->nNumber )
	//                         [OP_BOX_VARIANT]	( dnid )
	//                         // 将之前的参数值装箱
	// 
	//                         // 0 为标准存储过程返回值
	//                         [OP_BOX_PARAM]      (0)
	// 
	//                         //[OP_BOX_PARAM]      ("@RETURN_VALUE")
	// 
	//                         // 返回数据盒
	//                         [OP_RETURN_BOX]
	// 
	//                         // 结束
	//                         [OP_RPC_END];
	// 
	//                         DWORD nullSize = (DWORD)slmaker.EndEdition();
	//                         Globals::SendToAccountServer( &msg, sizeof( msg ) - nullSize, prefix );
	//     }
	//     catch ( lite::Xcpt &e )
	//     {
	//         TraceInfo( "rpcerr.log", "[%s]准备RPC参数出现错误：%s", Globals::GetStringTime(), e.GetErrInfo() );
	//         rfalse( 2, 1, e.GetErrInfo() );
	//     }
}

static void GetEQVPointMsg(DNID dnid, SQGetEQVPointMsg *pMsg)
{
	//     LPCSTR account = NULL;
	//     try
	//     {
	//         lite::Serialreader slm( pMsg->streamData );
	//         account = slm();
	//     }
	//     catch ( lite::Xcpt & )
	//     {
	//         return;
	//     }
	// 
	//     LPCSTR spname = NULL;
	//     if ( pMsg->type == EEQ_XYDPOINT )
	//     {
	//         spname = "AccountManager.sp_GetXYDPointNewXYD";
	//     }
	// 	else if ( pMsg->type == EEQ_GIFTCOUPON_NEWXYD )
	//     {
	//         spname = "AccountManager.sp_GetGiftCouponNewXYD";
	//     }
	//     else
	//     {
	//         return;
	//     }
	// 
	//     LPCSTR prefix = NULL;
	//     LPCSTR abs_account = Globals::SplitAccount( account, prefix );
	// 
	//     try
	//     {
	//         SQGameServerRPCOPMsg msg;
	//         msg.dstDatabase = SQGameServerRPCOPMsg::ACCOUNT_DATABASE;
	//         msg.dnidClient = INVALID_DNID;
	//         lite::Serializer slmaker( msg.streamData, sizeof( msg.streamData ) );
	// 
	//         slmaker
	// 
	//             // 注册出现异常时返回的数据（集）
	//             [OP_BEGIN_ERRHANDLE] ( (int)RPC_ERROR ) (spname) [OP_END_ERRHANDLE]
	// 
	//             // 准备存储过程
	//             [OP_PREPARE_STOREDPROC] (spname)
	// 
	//                 // 设定调用参数
	//                 [OP_BEGIN_PARAMS]
	//                 (1) ( abs_account )
	//                     [OP_END_PARAMS]
	// 
	//                     // 调用存储过程
	//                     [OP_CALL_STOREDPROC]
	// 
	//                     // 初始化返回数据盒
	//                     [OP_INIT_RETBOX]    (1024)
	// 
	//                         // 数据盒的第一个值
	//                         [OP_BOX_VARIANT]    ( (int)SMessage::EPRO_POINTMODIFY_MESSAGE )
	//                         [OP_BOX_VARIANT]    ( (int)SPointModifyMsg::EPRO_EQUIVALENT_BASE )
	//                         [OP_BOX_VARIANT]    ( (int)SEquivalentModifyMsg::EPRO_GET_EQUIVALENT )
	//                         [OP_BOX_VARIANT]    ( (int)pMsg->type )
	//                         [OP_BOX_VARIANT]    ( account )
	//                         [OP_BOX_VARIANT]    ( dnid )
	// 
	//                         // 将之前的参数值装箱
	// 
	//                         // 0 为标准存储过程返回值
	//                         [OP_BOX_PARAM]      (0)
	// 
	//                         //[OP_BOX_PARAM]      ("@RETURN_VALUE")
	// 
	//                         // 返回数据盒
	//                         [OP_RETURN_BOX]
	// 
	//                         // 结束
	//                         [OP_RPC_END];
	// 
	//                         DWORD nullSize = (DWORD)slmaker.EndEdition();
	//                         Globals::SendToAccountServer( &msg, sizeof( msg ) - nullSize, prefix );
	//     }
	//     catch ( lite::Xcpt &e )
	//     {
	//         TraceInfo( "rpcerr.log", "[%s]准备RPC参数出现错误：%s", Globals::GetStringTime(), e.GetErrInfo() );
	//         rfalse( 2, 1, e.GetErrInfo() );
	//     }
}

static void ProcessPointModifyMsg(DNID dnid, SPointModifyMsg *pMsg)
{
	switch (pMsg->_protocol)
	{
		// 通宝积分
	case SPointModifyMsg::EPRO_MONEYPOINT_BASE:
	{
		// 暂时屏蔽[只有收费服才用到]
		//SMoneyPointModifyMsg *pMPMsg = (SMoneyPointModifyMsg *)pMsg;
		//if(pMPMsg->_protocol == SMoneyPointModifyMsg::EPRO_REFRESH_MONEYPOINT)
		//	NotifyMoneyPointMsg( (SQRefreshMoneyPointMsg *)pMsg);
		//else if(pMPMsg->_protocol == SMoneyPointModifyMsg::EPRO_NOTIFY_MONEYPOINT)
		//	NotifyMoneyPointMsgCo((SQNotifyMoneyPointMsg *)pMsg);
	}
		break;

		// 元宝消费
	case SPointModifyMsg::EPRO_EQUIVALENT_BASE:
	{
		SEquivalentModifyMsg *pEQVMsg = (SEquivalentModifyMsg*)pMsg;

		if (pEQVMsg->_protocol == SEquivalentModifyMsg::EPRO_DEC_EQUIVALENT)
		{
			DecEQVPointMsg(dnid, (SQDecEQVPointMsg*)pEQVMsg);
		}
		else if (pEQVMsg->_protocol == SEquivalentModifyMsg::EPRO_GET_EQUIVALENT)
		{
			GetEQVPointMsg(dnid, (SQGetEQVPointMsg*)pEQVMsg);
		}
		else if (pEQVMsg->_protocol == SEquivalentModifyMsg::EPRO_INC_EQUIVALENT)
		{
			// 不存在增加元宝得功能，避免数据灾难
		}
	}
		break;

		// 对奖
	case SPointModifyMsg::EPRO_ENCOURAGEEMENT:
	{
		//NotifyEncouragementMsg((SQEncouragement * )pMsg);
	}
		break;
	};
}

bool CServerManager::Entry(DNID dnidClient, LINKSTATUS enumStatus, void *data, size_t size)
{
	SMessage *pMsg = (SMessage *)data;
	if (pMsg == NULL)
	{
		// 说明该连接断开

		// 这里处理内部服务器的断线
		std::map<DNID, SServer>::iterator it = gameServers.find(dnidClient);
		if (it == gameServers.end())
			// 如果这个联接和游戏服务器无关，就不用管他
			return true;

		// 否则需要将该服务器上的所有玩家退出游戏[并保存]
		// GetApp().m_PlayerManager.OnServerShutdown( it->second.key );

		// 移除该服务器联接相关数据
		gameServers.erase(dnidClient);

		return true;
	}
	if (pMsg->_protocol == SMessage::ERPO_SECTION_MESSAGE)
	{
		SMessage* srcMsg = SectionMessageManager::getInstance().recvMessageWithSection((SSectionMsg*)pMsg);
		if (srcMsg)
		{
			bool ret = Entry(dnidClient, enumStatus, (void*)srcMsg, ((SSectionMsg*)pMsg)->byCount*SSectionMsg::MAX_SIZE);
			SectionMessageManager::getInstance().popMessage(srcMsg);
			return ret;
		}
	}
	switch (enumStatus)
	{
	case LinkStatus_Waiting_Login:
		// 当处于LinkStatus_Waiting_Login等待认证消息的状态中，只能接受认证消息
		// 先判断是否是登陆认证级的消息
		if (pMsg->_protocol != SMessage::EPRO_SYSTEM_MESSAGE)
		{
			server.DelOneClient(dnidClient);
			return true;
		}

		// 然后判断是否为认证消息
		if (((SSysBaseMsg*)pMsg)->_protocol != SSysBaseMsg::EPRO_LOGIN)
		{
			server.DelOneClient(dnidClient);
			return true;
		}

		// 开始认证，将状态字移近一步
		server.SetLinkStatus(dnidClient, LinkStatus_Login);

		// 这里处理服务器的登陆
		OnServerLogin(dnidClient, (SQLoginMsg *)pMsg);
		break;

	case LinkStatus_Login:
		// 当处于等待认证的时候，该连接不允许发送任何消息

		// 所以收到消息就断开该连接
		server.DelOneClient(dnidClient);
		break;

	case LinkStatus_Connected:
		// 其它的消息只能在认证以后的状态下使用，否则视为非法(连接断开)
		if (pMsg->_protocol == SMessage::EPRO_TONG_MESSAGE)
		{
			// 这个是区域发过来要求删除帮派的，只所以不在区域里面直接这么做
			// 是因为在区域里面无法获的有效的区域数量,所以需要在登陆里面进行一次查询
			STongBaseMsg *pTb = (STongBaseMsg*)pMsg;
			if (pTb->_protocol == STongBaseMsg::EPRO_FACTIONBBS_MESSAGE)
			{
				SFactionBBSMsg * pFBMsg = (SFactionBBSMsg*)pMsg;
				if (pFBMsg->_protocol == SFactionBBSMsg::EPRO_FACTIONBBS_GET_TEXT)
				{
					SQGetFacBBSTextMsg *pGetFacBBSTextMsg = (SQGetFacBBSTextMsg*)pFBMsg;
					Globals::SendToDBServer(pGetFacBBSTextMsg, sizeof(SQGetFacBBSTextMsg));
				}
				else if (pFBMsg->_protocol == SFactionBBSMsg::EPRO_FACTIONBBS_GET)
				{
					SQGetFacBBSMsg *pGetFacBBSMsg = (SQGetFacBBSMsg*)pFBMsg;
					pGetFacBBSMsg->nServerID = Globals::GetServerId();
					Globals::SendToDBServer(pGetFacBBSMsg, sizeof(SQGetFacBBSMsg));
				}
				else if (pFBMsg->_protocol == SFactionBBSMsg::EPRO_FACTIONBBS_SAVE)
				{
					SQSaveFacBBSMsg *pSaveFacBBSMsg = (SQSaveFacBBSMsg*)pFBMsg;
					pSaveFacBBSMsg->nServerID = Globals::GetServerId();
					Globals::SendToDBServer(pSaveFacBBSMsg, sizeof(SQSaveFacBBSMsg)-pSaveFacBBSMsg->nFreeSize);
				}
				else if (pFBMsg->_protocol == SFactionBBSMsg::EPRO_FACTIONBBS_DELETE)
				{
					SQDelFacBBSMsg *pDelFacBBSMsg = (SQDelFacBBSMsg*)pFBMsg;
					Globals::SendToDBServer(pDelFacBBSMsg, sizeof(SQDelFacBBSMsg));
				}
				else if (pFBMsg->_protocol == SFactionBBSMsg::EPRO_FACTIONBBS_SET_TOP)
				{
					SQSetFacBBSTopMsg *pSetFacBBSMsg = (SQSetFacBBSTopMsg*)pFBMsg;
					Globals::SendToDBServer(pSetFacBBSMsg, sizeof(SQSetFacBBSTopMsg));
				}
			}
		}
		else if (pMsg->_protocol == SMessage::EPRO_REFRESH_MESSAGE)
		{
			SRefreshBaseMsg *pRf = (SRefreshBaseMsg*)pMsg;
			switch (pRf->_protocol)
			{
			case SRefreshBaseMsg::EPRO_REFRESH_REGION_SERVER:
				// 对于登陆服务器来说，场景服务器是不会要求使用登陆服务器的数据来刷新自己
				// 所以这里只会存在消息的应答
				RefreshRegionServer((SARefreshRegionServerMsg*)pRf, dnidClient);
				break;

			case SRefreshBaseMsg::EPRO_REFRESH_PLAYER:
				// 对于登陆服务器来说，场景服务器是不会要求使用登陆服务器的数据来刷新自己
				// 所以这里只会存在消息的应答
				Globals::UpdatePlayerData((SARefreshPlayerMsg*)pRf);
				break;
			case SRefreshBaseMsg::EPRO_REFRESH_RankList:
			{
				SQRefreshRanklist* pMessage = (SQRefreshRanklist *)pRf;
				if (pMessage)
				{
					extern DNID GetServerDnidByName(LPCTSTR);

					DNID serverID = GetServerDnidByName(pMessage->name);

					SQDBGETRankList pMsg;
					pMsg.level = pMessage->level;
					pMsg.serverID = serverID;
					//pMsg.serverID = pMessage->serverID;

					Globals::SendToDBServer(&pMsg, sizeof(SQDBGETRankList));
				}
			}
				break;
			case SRefreshBaseMsg::EPRO_REFRESH_PLAYERLOGINCOMPLTE: //玩家登录刷新,登录成功刷新数据
			{
				if (g_dConcurrencyPlayerNum > 0)
				{
					g_dConcurrencyPlayerNum--;
				}
				SQPlayerLoginComplte *pLoginComplte = (SQPlayerLoginComplte*)pRf;
				if (pLoginComplte)
				{
					std::map<std::string, DWORD>::iterator iter = playerloginlist.find(pLoginComplte->account);
					if (iter != playerloginlist.end())
					{
						DWORD logtime = GetTickCount() - iter->second;
						rfalse(2, 1, "LoginTime [%s] :%d", pLoginComplte->account, logtime);
						iter = playerloginlist.erase(iter);
						
					}
				}

			}
				break;
			}
		}
		else if (pMsg->_protocol == SMessage::EPRO_REBIND_MESSAGE)
		{
			// 这里应该只会收到重定向认证消息的（也就否决了内部服务器的重定向操作）
			if (Globals::DoCheckRebindMsg(dnidClient, (SQCheckRebindMsg*)pMsg))
			{
				// 如果认证通过，需要指示下一步的操作
				// 目前已经在DoCheckRebindMsg中处理完成
			}
		}
		else if (pMsg->_protocol == SMessage::EPRO_SERVER_CONTROL)
		{
			SServerCtrlMsg *pScm = (SServerCtrlMsg *)pMsg;
			switch (pScm->_protocol)
			{
			case SServerCtrlMsg::EPRO_PLAYER_DATA:
				SendPlayerData(dnidClient, (SGetPlayerDataMsg *)pScm);
				break;

			case SServerCtrlMsg::EPRO_RESOLUTION_REGION:
				ResolutionRegion(dnidClient, (SQResolutionDestRegionMsg *)pScm);
				break;

			case SServerCtrlMsg::EPRO_GAMESERVER_REQUEST_RPCOP:
			{
				SQGameServerRPCOPMsg *tempMsg = (SQGameServerRPCOPMsg *)pScm;

				if (tempMsg->dstDatabase == SQGameServerRPCOPMsg::CHARACTER_DATABASE)
				{
					tempMsg->dnidClient = dnidClient;
					Globals::SendToDBServer(data, size);
				}
				else if (tempMsg->dstDatabase == SQGameServerRPCOPMsg::ACCOUNT_DATABASE)
				{
					std::string accountPrefix((LPCSTR)tempMsg->GetExtendBuffer());
					tempMsg->dnidClient = dnidClient;
					Globals::SendToAccountServer(data, size, accountPrefix.empty() ? NULL : accountPrefix.c_str());
				}
				else if (tempMsg->dstDatabase == 2)
				{
					tempMsg->dnidClient = dnidClient;
					Globals::SendToDBCenter(data, size);
				}
				else if (tempMsg->dstDatabase >= 100)
				{
					tempMsg->dnidClient = dnidClient;
					Globals::SendToDBCenterEx(tempMsg->dstDatabase, data, size);
				}

				//try
				//{
				//	lite::Serialreader slr( tempMsg->streamData );

				//	RpcMsg< sizeof( tempMsg->streamData ) > msg;
				//	msg.dnidClient = dnidClient;
				//	msg.size = slr.maxSize();
				//	memcpy( msg.data, tempMsg->streamData, msg.size );

				//	DWORD entity_size = sizeof( msg ) - sizeof( tempMsg->streamData ) + slr.maxSize();

				//	if ( tempMsg->dstDatabase == SQGameServerRPCOPMsg::CHARACTER_DATABASE )
				//	{
				//		Globals::SendToDBServer( &msg, entity_size );
				//	}
				//	else if ( tempMsg->dstDatabase == SQGameServerRPCOPMsg::ACCOUNT_DATABASE )
				//	{
				//		Globals::SendToAccountServer( &msg, entity_size );
				//	}
				//}
				//catch ( ... )
				//{
				//}
			}
				break;
			case SServerCtrlMsg::EPRO_SCRIPT_TRANSFER:
			{
				SAGameScriptTransfreMsg *pScriptTranMsg = (SAGameScriptTransfreMsg *)pMsg;
				Globals::SendToDBCenterEx(pScriptTranMsg->dbcId, pScriptTranMsg, size);
			}
				break;
			}
		}
		else if (pMsg->_protocol == SMessage::EPRO_POINTMODIFY_MESSAGE)
		{
			ProcessPointModifyMsg(dnidClient, (SPointModifyMsg *)pMsg);
		}
		else if (pMsg->_protocol == SMessage::EPRO_GMM_MESSAGE)
		{
			Globals::OnRecvGMMsgFromGameServer(dnidClient, (SGMMMsg*)pMsg, size);
		}
		else if (pMsg->_protocol == SMessage::EPRO_MAIL_MESSAGE)
		{
			Globals::OnRecvMailMsg((SMailBaseMsg*)pMsg);
		}
		else if (pMsg->_protocol == SMessage::EPRO_ACCOUNT_MESSAGE)
		{
			// 这里暂时只会有防沉迷在线时间检测的消息！
			if (((SAccountBaseMsg *)pMsg)->_protocol == SAccountBaseMsg::EPRO_UPDATE_TIMELIMIT)
				Globals::SendUpdateTimeLimit((SQUpdateTimeLimit *)pMsg);
		}
		else if (pMsg->_protocol == SMessage::EPRO_BUILDING_MESSAGE)
		{
			switch (((SBuildingBaseMsg*)pMsg)->_protocol)
			{
			case SBuildingBaseMsg::EPRO_BUILDING_SAVE:
				((SQBuildingSaveMsg*)pMsg)->dwServerID = Globals::GetServerId();
				Globals::SendToDBServer(pMsg, ((SQBuildingSaveMsg*)pMsg)->wBufSize);
				break;

			case SBuildingBaseMsg::EPRO_BUILDING_GET:
				((SQBuildingGetMsg*)pMsg)->dwServerID = Globals::GetServerId();
				((SQBuildingGetMsg*)pMsg)->dwZoneID = dnidClient;
				Globals::SendToDBServer(pMsg, sizeof(SQBuildingGetMsg));
				break;
			}
		}
		else if (pMsg->_protocol == SMessage::EPRO_UNION_MESSAGE)
		{
			switch (((SUnionBaseMsg*)pMsg)->_protocol)
			{
			case SUnionBaseMsg::EPRO_GET_UNIONDATA:
				((SQGetUnionDataMsg*)pMsg)->dwZoneID = dnidClient;
				Globals::SendToDBServer(pMsg, sizeof(SQGetUnionDataMsg));
				break;

			case SUnionBaseMsg::EPRO_SAVE_UNIONDATA:
				Globals::SendToDBServer(pMsg, size);
				break;
			}
		}
		else if (pMsg->_protocol == SMessage::EPRO_DATABASE_MESSAGE)
		{
			//帮派数据 直接转发，消息不用压缩与分割 
			byte msg_protocol = ((_SDataBaseMsg*)pMsg)->_protocol;
			
			if (msg_protocol == _SDataBaseMsg::EPRO_SAVE_FACTION || msg_protocol == _SDataBaseMsg::EPRO_GET_FACTION || msg_protocol == _SDataBaseMsg::EPRO_WRITE_POINT_LOG 
				|| msg_protocol == _SDataBaseMsg::EPRO_DB_Mail_SANGUO || msg_protocol == _SDataBaseMsg::EPRO_DELETE_FACTION)
			{
// 				int num = SectionMessageManager::getInstance().evaluateDevidedAmount(size);
// 				int id = 0;
// 				BOOL ret;
// 				for (int i = 0; i < num; i++)
// 				{
// 					SSectionMsg sMsg;
// 					id = SectionMessageManager::getInstance().devideMessage(i, num, &sMsg, (SMessage*)data, size, id);
// 					ret = Globals::SendToDBServer(&sMsg, sizeof(SSectionMsg));
// 				}
				Globals::SendToDBServer(pMsg, size);
				break;
			}
			else if (msg_protocol == _SDataBaseMsg::EPRO_SAVE_SCRIPTDATA)
			{
				SQScriptData  *pScriptData = (SQScriptData *)pMsg;
				if (pScriptData)
				{
					WORD  datalenth = sizeof(SQScriptData) +  pScriptData->wLenth;
					WORD wcount = datalenth / sizeof(SAScriptData) + 1;
					SAScriptData   *psacriptdata = new SAScriptData[wcount];
					memset(&psacriptdata->wLenth, 0, sizeof(SAScriptData)*(wcount - 1));

					psacriptdata->wLenth = pScriptData->wLenth;
					psacriptdata->pBuffer = (BYTE*)&psacriptdata->wLenth + sizeof(WORD);

					BYTE  *dataaddress = (BYTE*)&pScriptData->wLenth + sizeof(WORD);
					memcpy(psacriptdata->pBuffer, dataaddress, pScriptData->wLenth);

					Globals::SendToAllGameServer(psacriptdata, (sizeof(SAScriptData) * wcount));
					delete[] psacriptdata;
					psacriptdata = nullptr;
		
// 					SAScriptData  scriptdata;
// 					scriptdata.wLenth = pScriptData->wLenth;
// 					scriptdata.pBuffer = new BYTE[scriptdata.wLenth];
// 					memset(scriptdata.pBuffer, 0, scriptdata.wLenth);
// 					memcpy(scriptdata.pBuffer, pScriptData->pBuffer, scriptdata.wLenth);
// 
// 					Globals::SendToAllGameServer(&scriptdata, (sizeof(SAScriptData)+scriptdata.wLenth));
				}

				break;
			}
			else if (msg_protocol == _SDataBaseMsg::EPRO_GET_SCRIPTDATA)
			{
				Globals::SendToDBServer(pMsg, size);
				break;
			}

			// 对了，还该缓存数据一下
			if (((_SDataBaseMsg*)pMsg)->_protocol == _SDataBaseMsg::EPRO_DOUBLE_SAVE_PLAYER)
			{
				SDoubleSavePlayerMsg *dsp = (SDoubleSavePlayerMsg*)pMsg;
				if (!Globals::DoubleUpdatePlayerData(dsp))
				{
					// 同步更新出现了错误，赶紧检查。。。
					rfalse(2, 1, "在交易后同步更新出现了错误，赶紧检查。。。");
					break;
				}

				dsp->serverId = Globals::GetServerId();

				rfalse(3, 1, "[%s] double savelog : [%s:%d] [%s:%d]", Globals::GetStringTime(),
					dsp->account[0], dsp->gid[0], dsp->account[1], dsp->gid[1]);

				// DBSave消息！ 在发往DBServer前，尝试压缩数据！
				extern BOOL TryEncoding_NilBuffer(const void *src_data, size_t src_size, void *&dst_data, size_t &dst_size);

				// 新侠必定是新版本了，直接发送压缩数据
				LPVOID outData = NULL;
				size_t outSize = 0;
				size_t validSize = dsp->gid[1] ? sizeof(SFixProperty)* 2 : sizeof(SFixProperty);
				BOOL result = TryEncoding_NilBuffer(dsp->data, validSize, outData, outSize);
				if (result && outSize < validSize - 256)
				{
					// 压缩成功后，直接在原来的数据基础上直接修改数据内容和数据大小！
					LPDWORD dataBuf = (LPDWORD)&dsp->data;
					*dataBuf = (WORD)outSize;
					memcpy(&dataBuf[1], outData, outSize);

					// 重新计算发送大小
					size_t segment = (size_t)&(((SDoubleSavePlayerMsg*)0)->data[0]);
					size = segment + 4 + outSize;
				}
				else
				{
					rfalse(4, 1, (result) ? "DBSAVE 数据压缩后大小几乎不变！" : "DBSAVE 数据压缩出错！");
				}
			}

			// 直接转发消息到数据库服务器，目前会涉及到的就只有角色立即保存消息
			int num = SectionMessageManager::getInstance().evaluateDevidedAmount(size);
			int id = 0;
			BOOL ret;
			for (int i = 0; i < num; i++)
			{
				SSectionMsg sMsg;
				id = SectionMessageManager::getInstance().devideMessage(i, num, &sMsg, (SMessage*)data, size, id);
				ret = Globals::SendToDBServer(&sMsg, sizeof(SSectionMsg));
			}
			// Globals::SendToDBServer( data, size );
		}
		// GM工具该接收的消息
		else if (pMsg->_protocol == 102)
		{
			try
			{
				SAEXECGMCMD msg;
				lite::Serialreader slr(((LPBYTE)data) + 4);
				DNID dnid = slr();
				lite::Variant var = slr();

				if (var.dataType != lite::Variant::VT_POINTER)
					break;

				if (sizeof(msg.streamData) < var.dataSize)
					break;

				LPCVOID data = var;

				memcpy(msg.streamData, var._pointer, var.dataSize);
				GetApp()->m_NetGMModule.SendGMMessage(dnid, &msg, sizeof(msg)+(sizeof(msg.streamData) - var.dataSize));
			}
			catch (lite::Xcpt &)
			{

			}
		}
		break;
	default:
		;
		// rfalse(2, 1, "异常情况，收到了异常连接的消息[i=%d][t=%d]", nClientIndex, g_GetLogStatus(nClientIndex));
	}

	return true;
}

BOOL CServerManager::OnServerLogin(DNID dnidClient, SQLoginMsg *pLog)
{
	LPCSTR account = NULL;
	LPCSTR password = NULL;

	try
	{
		lite::Serialreader sl(pLog->streamData);
		account = sl();
		password = sl();

		if (strlen(account) < 9)
			return FALSE;
	}
	catch (lite::Xcpt &)
	{
		return FALSE;
	}

	SALoginMsg msg;
	msg.dwRetCode = SALoginMsg::ERC_LOGIN_SUCCESS;

	// 设置数据库群集分组的信息！用于同步数据，避免产生的道具ID重复
	extern int clusterMax, clusterId;
	msg.wGMLevel = MAKEWORD(clusterMax, clusterId);

	if (HIBYTE(pLog->wVersion) != SERVER_VERSION)
	{
		// 版本不同
		msg.dwRetCode = SALoginMsg::ERC_INVALID_VERSION;
	}

	// 需要从登陆账号中解析出服务器的类型，编号一共16个有效数据
	// 头两个汉字“登陆|区域|聊天|网关|扩展”标识服务器类型，然后跟“_”，另外的为数字标号
	char number[32];
	DWORD ID = 0;

	memcpy(number, account + 5, 4);
	number[4] = 0;
	if (dwt::strcmp("区域", account, 4) == 0)
	{
		ID = 0x10000000 | (0x0fffffff & atoi(number));
	}
	else
	{
		server.DelOneClient(dnidClient);
		return rfalse(1, 1, "非法的服务器连接!!!");
	}

	// 将登陆服务器的启动时间返回给区域服务器，用来指定保存日志的目录
	dwt::strcpy(msg.szDescription, timeSegment, 60);

	// 服务器在这里通过连接认证？！
	server.SetLinkStatus(dnidClient, LinkStatus_Connected);

	server.SendMessage(dnidClient, &msg, sizeof(msg));

	std::map<DNID, SServer>::iterator it = gameServers.begin();
	while (it != gameServers.end())
	{
		if (it->second.ID == ID)
			break;

		it++;
	}

	if (it == gameServers.end())
	{
		// 该服务器未注册过
		// 这里需要进行注册审核
		SServer &s = gameServers[dnidClient];
		s.account = account;
		s.idControl = dnidClient;
		s.dwIP = -1;
		s.dwPort = -1;
		s.description = "";
		s.ID = ID;
		s.gid = Globals::GetGuid(false);
		s.wTotalPlayers = 0;
		s.wPlayers[0] = s.wPlayers[1] = s.wPlayers[2] = s.wPlayers[3] = 0;
		s.wMaxPlayers = 0;
	}
	else
	{
		SServer &s = it->second;

		// 该服务器已经注册过
		if (s.idControl != 0)
		{
			// 如果该服务器的连接还存在
			// 因为不应该替换已有的目标服务器连接
			// 那么认为本次连接是无效的
			server.DelOneClient(dnidClient);

			// 向服务器发送一个无效消息，以测试目标的有效性！
			SAResolutionDestRegionMsg msg;
			msg.dnidClient =
				msg.dwGID =
				msg.dwip =
				msg.dwport =
				msg.wMapID =
				msg.wRegionID =
				msg.wX =
				msg.wY = 0;
			server.SendMessage(s.idControl, &msg, sizeof(msg));

			return rfalse(2, 1, "目标服务器连接已经存在!!!");
		}

		// 设置新的服务器连接
		s.idControl = dnidClient;
	}

	return TRUE;
}

BOOL CServerManager::RefreshRegionServerEx(SARefreshRegionServerMsgEx *pRf, DNID dnidClient)
{
	std::map< DNID, SServer >::iterator it = gameServers.find(dnidClient);
	if (it == gameServers.end())
		return FALSE;

	lite::Serialreader slr(pRf->dataStream);
	try
	{
		while (slr.curSize() < slr.maxSize())
		{
			lite::Variant vt = slr();
			if (vt.dataType != vt.VT_POINTER || vt.dataSize != sizeof(SRegion))
				return FALSE;

			SRegion *r = (SRegion*)vt._pointer;
			if (r->MapID == 0)
				it->second.Regions.erase(r->ID);
			else
				it->second.Regions[r->ID] = *r;
		}
	}
	catch (lite::Xcpt &e)
	{
		rfalse(2, 1, e.GetErrInfo());
	}

	return TRUE;
}

BOOL CServerManager::RefreshRegionServer(SARefreshRegionServerMsg *pRf, DNID dnidClient)
{
	// 刷新区域服务器数据
	if (pRf == NULL)
		return false;

	// 特殊消息，主要是处理区域上的固定场景的添加和删减
	if (pRf->dnidClient == 0xff12ff34ff56ff78)
		return RefreshRegionServerEx((SARefreshRegionServerMsgEx*)pRf, dnidClient);

	if (pRf->byRegionNumber > MAX_REGION_NUMBER)
		return false;

	std::map<DNID, SServer>::iterator it = gameServers.begin();
	while (it != gameServers.end())
	{
		if (it->second.ID == pRf->key)
			break;

		it++;
	}

	if (it == gameServers.end())
		return false;

	SServer *pServer = &it->second;
	if (pServer->idControl != pRf->dnidClient)
		return false;

	if (pServer->gid != pRf->gid)
		return false;

	pServer->dwIP = pRf->dwIP;
	pServer->dwPort = pRf->dwPort;

	// 采集数据
	//SCollect_LoginServerData &Data = *Globals::GetCollectData();
	//Data.dwAvePing          -= pServer->dwAvePing;
	//Data.dwAvePing          += pRf->dwAvePing;
	//Data.i64TotalSpendMoney -= pServer->i64TotalSpendMoney;
	//Data.i64TotalSpendMoney += pRf->i64TotalSpendMoney;
	//Data.dwTotalHPlayers    -= pServer->wPlayers[3];
	//Data.dwTotalHPlayers    += pRf->wPlayers[3];
	//Data.dwInFlowPS         -= pServer->dwInFlowPS;
	//Data.dwInFlowPS         += pRf->dwInFlowPS;
	//Data.dwOutFlowPS        -= pServer->dwOutFlowPS;
	//Data.dwOutFlowPS        += pRf->dwOutFlowPS;
	//Data.dwTotalPlayers     =  m_dwCurPlayerNumber;
	//Data.dwMaxPlayers       =  max_players;
	//Data.dwCreatedSpecialItem -= pServer->dwCreatedSpecItem;
	//Data.dwCreatedSpecialItem += pRf->dwCreatedSpecItem;
	//Data.dwUseSpecialItem   -= pServer->dwUseSpecItem;
	//Data.dwUseSpecialItem   += pRf->dwUseSpecItem;

	pServer->dwUseSpecItem = pRf->dwUseSpecItem;
	pServer->dwCreatedSpecItem = pRf->dwCreatedSpecItem;
	pServer->dwInFlowPS = pRf->dwInFlowPS;
	pServer->dwOutFlowPS = pRf->dwOutFlowPS;
	pServer->dwAvePing = pRf->dwAvePing;
	pServer->i64TotalSpendMoney = pRf->i64TotalSpendMoney;
	pServer->wTotalPlayers = pRf->wTotalPlayers;
	pServer->wPlayers[0] = pRf->wPlayers[0];
	pServer->wPlayers[1] = pRf->wPlayers[1];
	pServer->wPlayers[2] = pRf->wPlayers[2];
	pServer->wPlayers[3] = pRf->wPlayers[3];

	if (pServer->wTotalPlayers > pServer->wMaxPlayers)
		pServer->wMaxPlayers = pServer->wTotalPlayers;

	if (pRf->byRegionNumber != 0)
	{
		pServer->bQuestFullData = TRUE;
		//pServer->Regions.clear();
		// 不注销,直接覆盖(假如说重复的话)
		for (int i = 0; i < pRf->byRegionNumber; i++)
			pServer->Regions[pRf->aRegions[i].ID] = pRf->aRegions[i];
	}

	return true;
}

BOOL CServerManager::SendPlayerData(DNID dnidClient, SGetPlayerDataMsg *pScm)
{
	LPCSTR account = NULL;
	SSetPlayerDataMsg msg;
	size_t esize = 0;

	try
	{
		lite::Serialreader sl(pScm->streamData);
		account = sl();

		lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
		slm(account);
		esize = slm.EndEdition();
	}
	catch (lite::Xcpt &)
	{
		return FALSE;
	}

	msg.dnidClient = pScm->dnidClient;
	msg.gid = pScm->gid;
	msg.SServerCtrlMsg::dnidClient = pScm->SServerCtrlMsg::dnidClient;
	msg.SServerCtrlMsg::qwSerialNo = pScm->SServerCtrlMsg::qwSerialNo;

	//rfalse(2, 1, "SGetPlayerDataMsg :[%s]", account);
	if (!Globals::GetPlayerData(account, pScm->gid, &msg.data, &msg.tdata))
		return FALSE;
	//currentServer = &server;
	//SectionMessageManager::getInstance().sendMessageWithSection(dnidClient,0,0,&msg,sizeof( msg ));
	rfalse(2, 1, "SSetPlayerDataMsg :[%s]", account);
	int num = SectionMessageManager::getInstance().evaluateDevidedAmount(sizeof(msg)-esize);
	int id = 0;
	for (int i = 0; i < num; i++)
	{
		SSectionMsg sMsg;
		id = SectionMessageManager::getInstance().devideMessage(i, num, &sMsg, &msg, sizeof(msg), id);
		server.SendMessage(dnidClient, &sMsg, sizeof(SSectionMsg));
	}
	//区域服务器获取玩家数据
	//这里重新绑定完成，区域服务器应该开启请求玩家数据,这里记录并发处理的玩家数量增加
//	if (pPlayer)
	{
		g_dConcurrencyPlayerNum++;
		g_dConcurrencyAllPlayerNum++;
		playerloginlist[account] = GetTickCount();
	}

	//server.SendMessage(dnidClient,&sMsg,sizeof(SSectionMsg));
	//currentServer = &server;
	//SectionMessageManager::getInstance().sendSectionMessage(dnidClient,&sMsg);
	//server.SendMessage( dnidClient, &msg, (WORD)(sizeof( msg ) - esize) );

	return TRUE;
}

void CServerManager::ResolutionRegion(DNID dnidClient, SQResolutionDestRegionMsg *pScm)
{
	assert(pScm != NULL);

	SAResolutionDestRegionMsg msg;
	msg.SServerCtrlMsg::dnidClient = pScm->SServerCtrlMsg::dnidClient;
	msg.SServerCtrlMsg::qwSerialNo = pScm->SServerCtrlMsg::qwSerialNo;

	for (std::map<DNID, SServer>::iterator it = gameServers.begin();
		it != gameServers.end(); it++)
	{
		std::map<WORD, SRegion>::iterator itRegion =
			it->second.Regions.find(pScm->wRegionID);
		if (itRegion == it->second.Regions.end())
			continue;

		if (itRegion->second.NeedLevel > pScm->wLevel)
		{
			// 等级不够的情况
			msg.dwip = msg.dwport = msg.dwGID = 0;
			msg.wRegionID = msg.wMapID = msg.wX = msg.wY = 0;
			msg.dnidClient = pScm->dnidClient;

			server.SendMessage(dnidClient, &msg, sizeof(msg));
			return;
		}

		try
		{
			lite::Serialreader sl(pScm->streamData);
			LPCSTR account = sl();

			if (!Globals::SetPlayerServerKey(account, it->second.ID))
				return;
		}
		catch (...)
		{
			return;
		}

		msg.dwip = it->second.dwIP;
		msg.dwport = it->second.dwPort;
		msg.wMapID = itRegion->second.MapID;
		msg.dnidClient = pScm->dnidClient;
		msg.dwGID = pScm->dwGID;
		msg.wRegionID = pScm->wRegionID;
		msg.wX = pScm->wX;
		msg.wY = pScm->wY;

		server.SendMessage(dnidClient, &msg, sizeof(msg));

		return;
	}

	// 这里是没有找到目标场景的情况，以后可以考虑把目标玩家转移到一个空的特殊场景
	msg.dwip = msg.dwport = msg.dwGID = 0;
	msg.wRegionID = msg.wX = msg.wY = 0;
	msg.dnidClient = pScm->dnidClient;
	msg.wMapID = -1;

	server.SendMessage(dnidClient, &msg, sizeof(msg));
	return;
}

BOOL CServerManager::PutIntoRegion(DNID dnidClient, LPCSTR account, DWORD gid, DWORD regionId, DWORD &serverKey)
{
	for (std::map<DNID, SServer>::iterator it = gameServers.begin(); it != gameServers.end(); it++)
	{
		// 如果serverKey不为0，表示该角色因为某种原因，还存在于服务器上
		if (serverKey != 0 && serverKey != it->second.ID)
			continue;

		std::map<WORD, SRegion>::iterator itRegion = it->second.Regions.find((WORD)regionId);
		if (itRegion == it->second.Regions.end())
		{
			if (serverKey)
				break;
			else
				continue;
		}

		// 通知玩家客户端进行场景转换
		SRebindRegionServerMsg msg;
		msg.ip = it->second.dwIP;
		msg.port = it->second.dwPort;
		msg.gid = gid;

		try
		{
			lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
			slm(account);

			Globals::SendToPlayer(dnidClient, &msg, sizeof(msg)-slm.EndEdition());
		}
		catch (lite::Xcpt &e)
		{
			e.GetErrInfo();
			serverKey = -1;
			return FALSE;
		}

		serverKey = it->second.ID;

		rfalse(2, 1, "put [%s] into region [%d] ok", account, regionId);

		//playerloginlist.push_back(account);
		return TRUE;
	}

	// 将玩家放入场景失败
	return FALSE;
}

BOOL CServerManager::GetServerInfo(DWORD ID, DNID &dnid, DWORD &ip, DWORD &port)
{
	for (std::map<DNID, SServer>::iterator it = gameServers.begin();
		it != gameServers.end(); it++)
	{
		if (ID == it->second.ID)
		{
			dnid = it->second.idControl;
			ip = it->second.dwIP;
			port = it->second.dwPort;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CServerManager::GetRegionInfo(DWORD ID, WORD &mapid, WORD &level)
{
	for (std::map<DNID, SServer>::iterator it = gameServers.begin();
		it != gameServers.end(); it++)
	{
		std::map<WORD, SRegion>::iterator itRegion = it->second.Regions.find((WORD)ID);
		if (itRegion != it->second.Regions.end())
		{
			mapid = itRegion->second.MapID;
			level = itRegion->second.NeedLevel;
			return TRUE;
		}
	}

	return FALSE;
}

void CServerManager::DisplayInfo()
{
	rfalse(2, 1, "服务器列表： %d 个已注册服务器", gameServers.size());

	DWORD players = 0;
	int playerst[4] = { 0, 0, 0, 0 };
	int count = 0;
	for (std::map<DNID, SServer>::iterator it = gameServers.begin();
		it != gameServers.end(); it++)
	{
		SServer *pS = &it->second;

		rfalse(2, 0, "\t编号[%d] 人数/最大值[%d/%d] %s %s",
			count++, pS->wTotalPlayers, pS->wMaxPlayers,
			pS->account.c_str(), (pS->idControl != 0) ? "正常" : "连接丢失");

		players += pS->wTotalPlayers;
		playerst[0] += pS->wPlayers[0];
		playerst[1] += pS->wPlayers[1];
		playerst[2] += pS->wPlayers[2];
		playerst[3] += pS->wPlayers[3];

		std::map<WORD, SRegion>::iterator itRegions = pS->Regions.begin();
		while (itRegions != pS->Regions.end())
		{
			DWORD ID[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			for (int i = 0; (i < 10) && (itRegions != pS->Regions.end()); i++, itRegions++)
			{
				ID[i] = itRegions->second.ID;
			}

			rfalse(2, 0, "\t\t[%d] [%d] [%d] [%d] [%d] [%d] [%d] [%d] [%d] [%d]",
				ID[0], ID[1], ID[2], ID[3], ID[4], ID[5], ID[6], ID[7], ID[8], ID[9]);
		}
	}

	if (max_players < players)
		max_players = players;

	//rfalse(2, 0, "当前连接数/底层连接数 ： %d/%d", m_PServer.GetClientNum(), m_PServer.GetClientNum_Dll());

	// 自动挂机人数
	WORD autoFightPlayers = (WORD)players - (playerst[0] + playerst[1] + playerst[2] + playerst[3]);

	rfalse(2, 0, "异常/正常/掉线/挂机/自动：%d/%d/%d/%d/%d", playerst[0],
		playerst[1], playerst[2], playerst[3], autoFightPlayers);

	rfalse(2, 0, "当前排队中人数：%d/%d/%d/%d", g_dConcurrencyPlayerNum, m_wQueuePlayerCount, playerloginlist.size(), g_dConcurrencyAllPlayerNum);

	rfalse(2, 0, "当前总人数/历史最大人数 ： %d/%d\r\n"
		"允许的最大人数 ： %d\r\n"
		"--------------------- end of server list\r\n",
		players, max_players, max_player_limit);
}

BOOL CServerManager::QueryRefreshGameServer()
{
	SQRefreshRegionServerMsg msg;

	for (std::map<DNID, SServer>::iterator it = gameServers.begin();
		it != gameServers.end(); it++)
	{
		msg.dnidClient = it->first;
		msg.key = it->second.ID;
		msg.gid = it->second.gid;
		msg.bFullQuest = FALSE;
		if (it->second.bQuestFullData == 0)
			msg.bFullQuest = true;

		/* extern SScoreTable s_Scores;
		 msg.Scores = s_Scores;*/

		server.SendMessage(it->first, &msg, sizeof(msg));
	}

	return TRUE;
}

void CServerManager::Broadcast(const void *pMsg, size_t size)
{
	for (std::map<DNID, SServer>::iterator it = gameServers.begin();
		it != gameServers.end(); it++)
	{
		server.SendMessage(it->first, pMsg, static_cast<WORD>(size));
	}
}

void CServerManager::GetPlayerNumber(DWORD &dwMaxCount, DWORD &dwOnlineCount)
{
	dwMaxCount = dwOnlineCount = 0;

	for (std::map<DNID, SServer>::iterator it = gameServers.begin();
		it != gameServers.end(); it++)
	{
		dwMaxCount += it->second.wMaxPlayers;
		dwOnlineCount += it->second.wTotalPlayers;
	}
}

int CServerManager::GetPlayerNumber(BYTE bstate)
{
	DWORD players = 0;
	int playerst[4] = { 0, 0, 0, 0 };
	int count = 0;
	for (std::map<DNID, SServer>::iterator it = gameServers.begin();
		it != gameServers.end(); it++)
	{
		SServer *pS = &it->second;
		players += pS->wTotalPlayers;
		playerst[0] += pS->wPlayers[0];
		playerst[1] += pS->wPlayers[1];
		playerst[2] += pS->wPlayers[2];
		playerst[3] += pS->wPlayers[3];

		std::map<WORD, SRegion>::iterator itRegions = pS->Regions.begin();
		while (itRegions != pS->Regions.end())
		{
			DWORD ID[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			for (int i = 0; (i < 10) && (itRegions != pS->Regions.end()); i++, itRegions++)
			{
				ID[i] = itRegions->second.ID;
			}
		}
	}

	if (max_players < players)
		max_players = players;

	//rfalse(2, 0, "当前连接数/底层连接数 ： %d/%d", m_PServer.GetClientNum(), m_PServer.GetClientNum_Dll());

	// 自动挂机人数
	if (bstate < 4)
	{
		return playerst[bstate];
	}
}

void CServerManager::SendFirst(void *pMsg, size_t size)
{
	for (std::map<DNID, SServer>::iterator it = gameServers.begin();
		it != gameServers.end(); it++)
	{
		server.SendMessage(it->first, pMsg, static_cast<WORD>(size));
		return;
	}
}
SectionMessageManager::SectionMessageManager()
{
	m_id = LOGINSERVER;
}
void SectionMessageManager::sendSectionMessage(DNID dnidClient, SSectionMsg* msg)
{
	currentServer->SendMessage(dnidClient, &msg, sizeof(SSectionMsg));
}
