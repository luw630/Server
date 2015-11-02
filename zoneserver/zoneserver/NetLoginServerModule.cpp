#include "StdAfx.h"
#include "netloginservermodule.h"
#include "DMainApp.h"
#include "GameObjects\Player.h"
#include "GameObjects\region.h"
#include "networkmodule\logmsgs.h"
#include "networkmodule\regionmsgs.h"
#include "networkmodule\factionbbsmsgs.h"
#include "networkmodule\mailmsg.h"
#include "networkmodule\cardpointmodifymsgs.h"
#include "networkmodule\accountmsgs.h"

#include "gameobjects/globalfunctions.h"

#include "gameobjects\scriptmanager.h"
#include "gameobjects\dspcialitemdata.h"
#include "networkmodule\PhoneMsgs.h"
#include "pub\rpcop.h"

#include <boost/bind.hpp>
#include "LITESERIALIZER/lite.h"
#include "networkmodule/ScriptMsgs.h"
#include "pub/traceinfo.h"

#include "gameobjects/gameworld.h"

#include "networkmodule/SectionMsgs.h"
#include "GameObjects/RankList.h"

#include "networkmodule/CenterMsg.h"
#include "GameObjects\SanguoCode\MailMoudle\MailMoudle.h"

extern size_t GetPlayerNumber();
extern LPIObject GetPlayerByGID(DWORD);
extern SIZE_T GetMemoryInfo();

extern void g_SetLogStatus(DNID dnidClient, LINKSTATUS state);

extern DWORD g_CurOnlineCheck[6];

std::map<DWORD, DWORD> msgtime;

CNetLoginServerModule::CNetLoginServerModule(void)
{
}

CNetLoginServerModule::~CNetLoginServerModule(void)
{
}

int CNetLoginServerModule::Execution()
{
	CNetConsumerModule::Execution( boost::bind(&CNetLoginServerModule::OnDispatch, this, _1, _2 ) );
	return 1;
}

BOOL CNetLoginServerModule::SendMsgToLoginSrv( SMessage *pMsg, int iSize )
{
	if (pMsg == NULL) 
		return FALSE;

	return SendMessage(pMsg, iSize);
}

bool CNetLoginServerModule::OnDispatch( void *data, size_t size )
{
	SMessage *pMsg = (SMessage *)data;
	if (pMsg)
	{
		if(pMsg->_protocol == SMessage::ERPO_SECTION_MESSAGE)
		{
			SMessage *srcMsg = SectionMessageManager::getInstance().recvMessageWithSection((SSectionMsg*)pMsg);
			if (srcMsg)
			{
				bool ret = OnDispatch(srcMsg,((SSectionMsg*)pMsg)->byCount*SSectionMsg::MAX_SIZE);
				SectionMessageManager::getInstance().popMessage(srcMsg);
				return ret; 
			}
		}
	}
	
	if (pMsg == NULL)
	{
		// 假设为网闪断！
		// 在这里重复几次连接
		for (int i=0; i<86400; i++)
		{
			// 这里要把自己注册到登陆服务器上去
			Sleep(1000);

			if ( !Connect( GetApp()->szLoginIP, GetApp()->szLoginPORT ) )
			{
				rfalse(2, 1, "无法连接登陆服务器！！！");
				continue;
			}

			// 向服务器注册本机
			SQLoginMsg msg;

			try
			{
				lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
				slm( GetApp()->m_szName )( "123456789" );

				msg.wVersion = MAKEWORD( 0, GetApp()->m_ServerVersion );
				SendMessage( &msg, (WORD)(sizeof(msg) - slm.EndEdition()) );
			}
			catch ( lite::Xcpt & )
			{
				return true;
			}

			return true;
		}

		// if check server disconnected this link, get in here
		GetApp()->m_eLoginStatus = CDAppMain::SS_BREAK;
		return true;
	}

	switch (pMsg->_protocol)
	{
	case SMessage::EPRO_TONG_MESSAGE:
		{
			// 这里处理帮派部分,原理与ORB的群发消息一样
			STongBaseMsg *pTBMsg = (STongBaseMsg *)pMsg;
			if( pTBMsg->_protocol == STongBaseMsg::EPRO_TONG_SEND_SERVER )
				GetGW()->m_FactionManager.ProcessSSMsg( (SQSendServerMsg*)pTBMsg );
			else if( pTBMsg->_protocol == STongBaseMsg::EPRO_FACTIONBBS_MESSAGE )
			{
				SFactionBBSMsg *pFacBBSMsg = (SFactionBBSMsg *)pTBMsg;
				if( pFacBBSMsg->_protocol == SFactionBBSMsg::EPRO_FACTIONBBS_GET )
				{
					SAGetFacBBSMsg *pGetFacBBSMsg = (SAGetFacBBSMsg *)pFacBBSMsg;
					CPlayer *pPlayer = (CPlayer *)GetPlayerByName( pGetFacBBSMsg->szName )->DynamicCast( IID_PLAYER );
					if( pPlayer == NULL )
						break;

					g_StoreMessage( pPlayer->m_ClientIndex, pGetFacBBSMsg, sizeof( SAGetFacBBSMsg ) - sizeof( pGetFacBBSMsg->stFactionBBS[0] ) * ( MAX_BBSLIST - pGetFacBBSMsg->byBBSNum ) );
				}
				else if( pFacBBSMsg->_protocol == SFactionBBSMsg::EPRO_FACTIONBBS_GET_TEXT )
				{
					SAGetFacBBSTextMsg *pGetFacBBSTextMsg = (SAGetFacBBSTextMsg *)pFacBBSMsg;

					lite::Serialreader ls( pGetFacBBSTextMsg->streamData );
					LPCSTR szName = ls();
					CPlayer *pPlayer = (CPlayer *)GetPlayerByName( szName )->DynamicCast( IID_PLAYER );
					if( pPlayer == NULL )
						break;

					g_StoreMessage( pPlayer->m_ClientIndex, pGetFacBBSTextMsg, (DWORD)(sizeof( SAGetFacBBSTextMsg ) - pGetFacBBSTextMsg->nFreeSize) );
				}
			}
		}
		break;
	case SMessage::EPRO_REFRESH_MESSAGE:
		// 这里处理刷新操作
		RecvRefreshMsg((SRefreshBaseMsg*)pMsg);
		break;

	case SMessage::EPRO_SERVER_CONTROL:
		// 这里处理服务器控制消息
		RecvCtrlMsg((SServerCtrlMsg *)pMsg  );
		break;

	case SMessage::EPRO_REBIND_MESSAGE:
		// 这里处理连接重定向的认证操作
		RecvCheckRebind((SACheckRebindMsg *)pMsg);
		break;

	case SMessage::EPRO_SYSTEM_MESSAGE:
		// 处理启动注册后的回应
		if (((SSysBaseMsg *)pMsg)->_protocol == SSysBaseMsg::EPRO_LOGIN)
			RecvLoginMessage((SALoginMsg *)pMsg);
		break;
	case SMessage::EPRO_GMM_MESSAGE:
		{
			if (GetApp() && GetApp()->m_pGameWorld)
			{
				GetApp()->m_pGameWorld->DispatchGMModuleMessage(INVALID_DNID,(SGMMMsg*)pMsg,NULL,TRUE);
			}
		}   
		break;
	case SMessage::EPRO_POINTMODIFY_MESSAGE:
		RecvPointModifyMsg((SPointModifyMsg *)pMsg);
		break;
	case SMessage::EPRO_MAIL_MESSAGE:
		{
			SMailBaseMsg *pMailBaseMsg = (SMailBaseMsg*)pMsg;
			switch (pMailBaseMsg->_protocol)
			{
			case SMailBaseMsg::EPRO_MAIL_SEND:
				{
					SAMailSendMsg *pASendMsg = (SAMailSendMsg*)pMsg;

					CPlayer *pPlayer = NULL;
					pPlayer = (CPlayer *)GetPlayerBySID(pASendMsg->m_iStaticID)->DynamicCast(IID_PLAYER);
					if (pPlayer == NULL)
					{
						return 1;
					}

					g_StoreMessage(pPlayer->m_ClientIndex,pASendMsg,sizeof(SAMailSendMsg));
				}
				break;
			case SMailBaseMsg::EPRO_MAIL_RECV:
				{
					SAMailRecvMsg *pARecvMsg = (SAMailRecvMsg*)pMsg;

					CPlayer *pPlayer = NULL;
					pPlayer = (CPlayer *)GetPlayerBySID(pARecvMsg->m_iStaticID)->DynamicCast(IID_PLAYER);
					if (pPlayer == NULL)
					{
						return 1;
					}

					g_StoreMessage(pPlayer->m_ClientIndex,pARecvMsg,sizeof(SAMailRecvMsg));
				}
				break;
			case SMailBaseMsg::EPRO_MAIL_DELETE:
				{
					SAMailDeleteMsg *pADeleteMsg = (SAMailDeleteMsg*)pMsg;

					CPlayer *pPlayer = NULL;
					pPlayer = (CPlayer *)GetPlayerBySID(pADeleteMsg->m_iStaticID)->DynamicCast(IID_PLAYER);
					if (pPlayer == NULL)
					{
						return 1;
					}

					g_StoreMessage(pPlayer->m_ClientIndex,pADeleteMsg,sizeof(SAMailDeleteMsg));
				}
				break;
			case SMailBaseMsg::EPRO_MAIL_NEWMAIL:
				{
					SANewMailMsg *pANewMailMsg = (SANewMailMsg*)pMsg;

					CPlayer *pPlayer = NULL;
					pPlayer = (CPlayer *)GetPlayerBySID(pANewMailMsg->m_iStaticID)->DynamicCast(IID_PLAYER);
					if (pPlayer == NULL)
					{
						return 1;
					}
					extern void TalkToDnid(DNID dnidClient, LPCSTR info,bool addPrefix = true,bool bPickUp = false, WORD wShowType=1);

					TalkToDnid(pPlayer->m_ClientIndex,"有朋友给你留言了，快去看看吧！");
				}
				break;
			}
		}
	//case SBasePhoneMsg::EPRO_PHONE_MESSAGE:
	//	{
	//		SAPhoneCallMsg *pAMsg =(SAPhoneCallMsg*)pMsg;
	//		extern void SendMessageToPlayer(LPCSTR szName, SMessage *pMsg, WORD wSize);
	//		SendMessageToPlayer(pAMsg->m_szCallerName,pAMsg,sizeof(SAPhoneCallMsg));
	//	}
	//	break;

	case SMessage::EPRO_ACCOUNT_MESSAGE:
		if ( ( ( SAccountBaseMsg* )pMsg )->_protocol == SAccountBaseMsg::EPRO_UPDATE_TIMELIMIT )
		{
			SQUpdateTimeLimit *tempMsg = ( SQUpdateTimeLimit* )pMsg;
			try
			{
				lite::Serialreader slr( tempMsg->streamBuffer );
				LPCSTR account = slr();
				DWORD online = slr();

				LPIObject obj = GetPlayerByAccount( account );
				if ( !obj ) 
					return 1;

				CPlayer *player = ( CPlayer* )obj->DynamicCast( IID_PLAYER );
				player->NotifyTimeLimit( online );
			}
			catch ( lite::Xcpt& )
			{
			}
		}
		break;

	case SMessage::EPRO_BUILDING_MESSAGE:
		// 回传建筑物数据到原请求区域服务器上！
		if ( ( ( SBuildingBaseMsg* )data )->_protocol == SBuildingBaseMsg::EPRO_BUILDING_GET )
        {
            SABuildingGetMsg *pMsg = ( SABuildingGetMsg* )data;
            if ( pMsg->retMsg != SABuildingGetMsg::RET_SUCCESS )
                break;

            if ( CRegion *region = ( CRegion* )FindRegionByID( pMsg->wParentID )->DynamicCast( IID_REGION ) )
                region->LoadBuildings( pMsg );
            else 
                BEGIN_NOTIFY ( "统计返回场景建筑物导致错误问题" )[ GetServerID() ][ pMsg->dwZoneID ][ pMsg->wParentID ] END_NOTIFY
        }
        break;

    case SMessage::EPRO_UNION_MESSAGE:
        {
             switch ( ( (SUnionBaseMsg *)data )->_protocol )
             {
             case SUnionBaseMsg::EPRO_GET_UNIONDATA:
                GetGW()->m_UnionManager.LoadUnionData( ( SAGetUnionDataMsg* )data );
                break;
             }
        }
        break;

    case SMessage::EPRO_DATABASE_MESSAGE:
        {
             switch ( ( (_SDataBaseMsg *)data )->_protocol )
             {
                 case _SDataBaseMsg::EPRO_GET_FACTION:
					// GetGW()->m_FactionManager.LoadFactionList((SAGetFactionMsg *)data);
					 GetGW()->m_FactionManager.LoadFactionDataList((SAGetFactionDataMsg *)data);
                     break;
					 //20150114 wk
				 case _SDataBaseMsg::EPRO_DB_Mail_SANGUO:
					 MailMoudle::getSingleton()->DispatchMailMsg(1, (SMailMsg*)data,1);
					 break;
				 case _SDataBaseMsg::EPRO_DELETE_FACTION:
					 GetGW()->m_FactionManager.DeleteFactionRecall((SADeleteFaction *)data);
					 break;
				 case _SDataBaseMsg::EPRO_GET_SCRIPTDATA:
					 GetGW()->m_FactionManager.GetScriptData((SAGetScriptData *)data);
					 break;
				 case _SDataBaseMsg::EPRO_SAVE_SCRIPTDATA:
					 GetGW()->m_FactionManager.GetScriptData((SAScriptData *)data);
					 break;
             }
        }
        break;
    // GM工具查询特殊控制某一客户端
    case 102:
        {
            extern BOOL RpcControlGMTool( LPVOID stream, DNID clinet );

            DNID *clinet = (DNID*)( ( ( LPBYTE )pMsg ) + 4 );
            LPBYTE data = ( ( LPBYTE )pMsg ) + 12;

            RpcControlGMTool( data, *clinet );
        }
        break;

	// 这里是从数据库经过LOGIN拉过来的排行榜信息
	case SMessage::EPRO_GET_RANKLIST_FROM_DB:
		{
 			SAGetRankListFromDB *msg = (SAGetRankListFromDB *)pMsg;
 			CSingleRank::GetInstance().GetRankData_From_DB(msg->m_NewRankList, msg->m_Num);
		}
		break;
    }

	return true;
}

void CNetLoginServerModule::RecvRefreshMsg(SRefreshBaseMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4,1,"CNetLoginServerModule::RecvRefreshMsg(SRefreshBaseMsg *pMsg)为NULL");
		return;
	}
	switch (pMsg->_protocol)
	{
	case SRefreshBaseMsg::EPRO_REFRESH_REGION_SERVER:
		// 对于场景服务器来说，不会使用外部的数据来刷新自己的基本数据
		// 所以这里接受的只有请求刷新数据的消息
		{
			SARefreshRegionServerMsg msg;
			SQRefreshRegionServerMsg *pRf = (SQRefreshRegionServerMsg *)pMsg;
			if (!pRf)
			{
				return;
			}
			// 填充校验数据
			msg.dnidClient = pRf->dnidClient;
			msg.key = pRf->key;
			msg.gid = pRf->gid;

			// 填充连接数据
			msg.dwIP = GetApp()->m_dwIP;
			msg.dwPort = GetApp()->m_dwPort;
			msg.wTotalPlayers = (WORD)GetPlayerNumber();

#define WORDFLOOR(_fourbyte) (WORD)((_fourbyte > 0xffff) ? (0xffff) : (_fourbyte));

			msg.wPlayers[0] = WORDFLOOR(g_CurOnlineCheck[0]);
			msg.wPlayers[1] = WORDFLOOR(g_CurOnlineCheck[1]);
			msg.wPlayers[2] = WORDFLOOR(g_CurOnlineCheck[2]);
			msg.wPlayers[3] = WORDFLOOR(g_CurOnlineCheck[3]);

            // 自动战斗的玩家究竟算挂机还是在线？
            // 这块不要了，
            // 考虑把 在线人数细分为，在线游戏和在线挂机两种
            // 在线挂机 = 总人数 - ( 异常 + 正常 + 掉线 + 挂机 )
            
            //extern BOOL afLikeHungup;
            //if ( afLikeHungup )
            //    msg.wPlayers[3] += WORDFLOOR(g_CurOnlineCheck[4])
            //else
            //    msg.wPlayers[1] += WORDFLOOR(g_CurOnlineCheck[4])

			{// 写入采集数据
				extern  DWORD   dwAvePing;
				msg.dwAvePing         = dwAvePing;
				msg.i64TotalSpendMoney = GetApp()->GetSpendMoney();
				if( g_pSpecialItemData )
				{
					msg.dwCreatedSpecItem  = g_pSpecialItemData->GetCreatedItem();
					msg.dwUseSpecItem      = g_pSpecialItemData->GetUseedItem();
				}
				else
				{
					msg.dwCreatedSpecItem  = 0;
					msg.dwUseSpecItem      = 0;
				}
				static  DWORD   dwLastTime = timeGetTime();
				DWORD   dwTheTime;
				dwTheTime = timeGetTime();
				if( (int)(dwTheTime - dwLastTime) > 0 )
				{
					msg.dwInFlowPS  = (DWORD)(GetApp()->m_Server.GetInSizeAndClear()  * 1000 / (float)( dwTheTime - dwLastTime ));
					msg.dwOutFlowPS = (DWORD)(GetApp()->m_Server.GetOutSizeAndClear() * 1000 / (float)( dwTheTime - dwLastTime ));
					dwLastTime = dwTheTime;
				}
				else
				{
					msg.dwInFlowPS  = 0;
					msg.dwOutFlowPS = 0;
				}
			}

			// 填充场景具体数据
			FillRegionInfoMsg(msg, pRf->bFullQuest);

			// 返回给登陆服务器
			SendMsgToLoginSrv(&msg, msg.GetMySize());
		}
		break;

	case SRefreshBaseMsg::EPRO_REFRESH_PLAYER:
		{

		}
		// 目前还没有玩家刷新的数据
		break;

        // 这个是准备的刷脚本的消息
    case SRefreshBaseMsg::EPRO_REFRESH_PLAYER + 1:
        {
            SARefreshRegionServerMsgEx msg;
            msg._protocol = SRefreshBaseMsg::EPRO_REFRESH_PLAYER + 1;
            msg.dnidClient = g_Script.Restore( (int)( ( SARefreshRegionServerMsgEx* )pMsg )->dnidClient );
			SendMsgToLoginSrv( &msg, sizeof( msg ) );
        }
        break;
	}
}

void CNetLoginServerModule::RecvPointModifyMsg(SPointModifyMsg *pMsg)
{
	if (!pMsg || !GetApp() || !GetApp()->m_pGameWorld)
	{
		return;
	}
	switch (pMsg->_protocol)
	{
	/*case SPointModifyMsg::EPRO_PREPARE_EXCHANGE:
		GetApp()->m_pGameWorld->RecvPrepareExchangeResult((SAPrepareExchangeMsg *)pMsg);
		break;*/

	case SPointModifyMsg::EPRO_REFRESH_CARDPOINT:
		GetApp()->m_pGameWorld->RefreshPlayerCardPoint((SARefreshCardPointMsg *)pMsg);
		break;

	case SPointModifyMsg::EPRO_MONEYPOINT_BASE:
		GetApp()->m_pGameWorld->RefreshPlayerMoneyPoint( (SARefreshMoneyPointMsg *)pMsg);
		break;

	case SPointModifyMsg::EPRO_EXCHANGEMC_BASE:
		GetApp()->m_pGameWorld->NotifyMCInfo( (SExchangeMCMsg *)pMsg);
		break;

	case SPointModifyMsg::EPRO_EQUIVALENT_BASE:
		GetApp()->m_pGameWorld->NotifyEQVPoint( (SEquivalentModifyMsg *)pMsg);
		break;
	case SPointModifyMsg::EPRO_ENCOURAGEEMENT://抽将
		GetApp()->m_pGameWorld->NotifyEncouragement((SAEncouragement *)pMsg);
		break;
	};
}

void CNetLoginServerModule::RecvCtrlMsg(SServerCtrlMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4,1,"CNetLoginServerModule::RecvCtrlMsg(SServerCtrlMsg *pMsg) NULL");
		return;
	}
	switch (pMsg->_protocol)
	{
	case SServerCtrlMsg::EPRO_PLAYER_DATA:
		{
			SIZE_T smemsize = GetMemoryInfo() / 1024;
			rfalse(2, 1, "SSetPlayerDataMsg  %d", smemsize);

			SSetPlayerDataMsg *pSpd = (SSetPlayerDataMsg *)pMsg;
			
			if (pSpd && pSpd->dnidClient != 0)
			{
				//rfalse(2, 1, "SSetPlayerDataMsg :[%d]", pSpd->gid);
				// 登陆就发送服务器时间
				DWORD tick[2] = { static_cast<const LPDWORD>(static_cast<void *>("tick"))[0], (DWORD)time(NULL) };
				g_StoreMessage(pSpd->dnidClient, tick, 8);
				if (!GetApp()||!GetApp()->m_pGameWorld)
				{
					rfalse(4,1,"GetApp()->m_pGameWorld->SetPlayerData 为 NULL");
					return;
				}
				if (GetApp()->m_pGameWorld->SetPlayerData(pSpd->dnidClient, pSpd->gid, &pSpd->data, &pSpd->tdata))
				{
					
					//这里时基本上一个玩家的数据应该登陆并且从db中获取完成, 通知登录服务器区域服务器登录完成
					SQPlayerLoginComplte sPlayerLoginComplte;

					CPlayer *pPlayer = (CPlayer *)GetPlayerByDnid(pSpd->dnidClient)->DynamicCast(IID_PLAYER);
					if (pPlayer)
					{
						strcpy(sPlayerLoginComplte.account, pPlayer->GetAccount());
						SendMsgToLoginSrv(&sPlayerLoginComplte, sizeof(SQPlayerLoginComplte));

						std::map<DWORD, DWORD>::iterator iter = msgtime.find(pSpd->dnidClient);
						if (iter != msgtime.end())
						{
							DWORD tick = GetTickCount() - iter->second;
							rfalse(2, 1, "处理玩家<%s>登陆耗时%d", pPlayer->GetAccount(),tick);
						}
					}
				}
				else
				{
					rfalse(2, 1, "玩家 [%d] 登录SSetPlayerDataMsg 失败", pSpd->gid);
					g_CutClient(pSpd->dnidClient);  //尝试直接断开处理
				}
				SIZE_T smemsize = GetMemoryInfo() / 1024;
				rfalse(2, 1, "SSetPlayerDataMsgFinish  %d", smemsize);
			}
		}
		break;

	case SServerCtrlMsg::EPRO_RESOLUTION_REGION:
		{
			if (GetApp() && GetApp()->m_pGameWorld)
			{
				GetApp()->m_pGameWorld->RebindPlayerToNewRegionServer((SAResolutionDestRegionMsg *)pMsg);
			}
		}
		break;

	case SServerCtrlMsg::EPRO_GAMESERVER_REQUEST_RPCOP:
        ProcessInterop( ( ( SAGameServerRPCOPMsg * )pMsg )->streamData, ( ( SAGameServerRPCOPMsg * )pMsg )->srcDatabase );
		break;
    case SServerCtrlMsg::EPRO_SCRIPT_TRANSFER:
        {
            extern void RecvTransferMsg( SAGameScriptTransfreMsg *pMsg );
            SAGameScriptTransfreMsg *pScript = ( SAGameScriptTransfreMsg * )pMsg;
            RecvTransferMsg( pScript );

        }
        break;
	}
}

void CNetLoginServerModule::RecvCheckRebind(SACheckRebindMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4,1,"CNetLoginServerModule::RecvCheckRebind 为NULL");
		return;
	}
	LPCSTR account = 0;
	DWORD limitedState = 0, online = 0, offline = 0;  
	QWORD puid = 0;

	try
    {
		lite::Serialreader sl(pMsg->streamData);
		account = sl();
        while (sl.curSize() < sl.maxSize())
        {
            lite::Variant lvt = sl();
            if (lvt.dataType != lvt.VT_RPC_OP)
                break;

            switch (lvt._rpcop)
            {
            case 0:
                limitedState = true;
                online = sl();
                offline = sl();
                break;

            case 1:
                puid = sl();
                break;
            }
        }
    }
	catch (lite::Xcpt &)
    {
        return;
    }

	extern bool g_check;
	if (g_check)
	{
		extern LPCSTR GetStringTime();
		rfalse(1, 1, "check : [%s] 收到CheckRebind ret = %s", GetStringTime(), account);
	}

	if (0 == pMsg->dnidClient)
	{
		rfalse(1, 1, "check : dnidClient == 0!!!");
		return;
	}

	if (0 == pMsg->dwResult)
	{
		rfalse(1, 1, "check : pMsg->dwResult == false!!!");
		g_CutClient(pMsg->dnidClient);
		return;
	}
	else if (1 == pMsg->dwResult)
	{
		// 如果检测成功，保留连接
		g_SetLogStatus(pMsg->dnidClient, LinkStatus_Connected);

		
		// 同时通知客户端可以发数据了
		SACheckRebindMsg msg;
		msg.SMessage::_protocol = 100;
		msg.dwResult			= 2;
		msg.dnidClient			= 0;
		msg.gid					= 0;

		try
		{
			lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
			slm(account);
			///发送失败的话就不发玩家数据过去了,因为现在客户端是根据有没有收到这个消息来判断是否重连上的
			if (g_StoreMessage(pMsg->dnidClient, &msg, (DWORD)(sizeof(msg) - slm.EndEdition())) == 0)
			{
				rfalse("SACheckRebindMsg Send Failed");
				return;
			}
		}
		catch( lite::Xcpt & )
		{
		}
	}
	else
	{
		// 未知校验值
		rfalse(2, 1, "未知的重定向校验值%d！！", pMsg->dwResult);
		g_CutClient(pMsg->dnidClient);
		return;
	}
	if (!GetApp() || !GetApp()->m_pGameWorld)
	{
		rfalse(4,1,"!GetApp() || !GetApp()->m_pGameWorld 为NULL");
		return;
	}
// 	std::map<DWORD, DWORD>::iterator iter = msgtime.find(pMsg->dnidClient);
// 	if (iter == )
// 	{
// 	}

	SIZE_T smemsize = GetMemoryInfo() / 1024;
	rfalse(2, 1, "CreateNewPlayer  %d", smemsize);

	msgtime[pMsg->dnidClient] = GetTickCount();
	// 建立相对应的玩家对象
	int ret = GetApp()->m_pGameWorld->CreateNewPlayer(pMsg->dnidClient, account, pMsg->gid, limitedState, online, offline, puid);
	if (1 == ret)
	{
		//rfalse(2, 1, "SGetPlayerDataMsg :[%d]", pMsg->gid);
		// 同时刷新数据
		SGetPlayerDataMsg msg;
		msg.dnidClient = pMsg->dnidClient;
		msg.gid = pMsg->gid;
		msg.SServerCtrlMsg::dnidClient = pMsg->dnidClient;

		try
		{
			lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
			slm( account );
			SendMsgToLoginSrv( &msg, (int)(sizeof(msg) - slm.EndEdition()) );
		}
		catch( lite::Xcpt & )
		{
		}

		//TODO:Tony Modify [2012-3-8]Comment:[玩家登陆的时候发送给中心服]
		SQLoginPlayer _msg;
		_msg.dnidClient = pMsg->dnidClient;
		try
		{
			lite::Serializer _slm( _msg.streamData , sizeof( _msg.streamData ) );
			_slm( account );
			GetApp()->SendToCenterServer( &_msg , (WORD)( sizeof(_msg) - _slm.EndEdition() ) );
		}
		catch ( lite::Xcpt & )
		{
			__asm nop;
		}

		SIZE_T smemsize = GetMemoryInfo() / 1024;
		rfalse(2, 1, "CreateNewPlayerFinish And SGetPlayerDataMsg  %d", smemsize);

		///////////////////////////////////////////////////////////////////
	}
	else if (2 == ret)
	{
	}
	else
	{
		g_CutClient(pMsg->dnidClient);
	}
}

void CNetLoginServerModule::RecvLoginMessage(SALoginMsg *pMsg)
{
	if (!GetApp())
	{
		rfalse(4,1,"!GetApp() CNetLoginServerModule::RecvLoginMessage(SALoginMsg *pMsg)为NULL");
		return;
	}
	if (!pMsg)
	{
		rfalse(4,1,"!pMsg CNetLoginServerModule::RecvLoginMessage(SALoginMsg *pMsg)为NULL");
		return;
	}
	if (pMsg->dwRetCode == SALoginMsg::ERC_LOGIN_SUCCESS)
	{
		if ( GetApp()->m_eLoginStatus == CDAppMain::SS_LOGIN )
			 GetApp()->m_eLoginStatus = CDAppMain::SS_INIT;

        // 通过wGMLevel来指定数据库群集分组数据，低字表示群集容量，高字表示群集编号
        BYTE clusterMax = LOBYTE(pMsg->wGMLevel);
        if (clusterMax == 0 || clusterMax == 1)
        {
            rfalse(2, 1, "[提示]该服务器并未加入数据库一拖多群集分组！");
        }
        else if ((clusterMax <= 8 ) &&									// 目前设定最多数据库1拖8
				((clusterMax) > HIBYTE(pMsg->wGMLevel)) &&              // 分组ID必定小于最大限额
				((clusterMax & ~(clusterMax - 1)) == clusterMax))		// 设定的数目必定为平方数
        {
            extern DWORD clusterMask, clusterId;

            // 以下该公式是由((v & ~(v-1)) == v)来保证的
            DWORD lsh = ((clusterMax == 2) ? 31 : ((clusterMax == 4) ? 30 : 29));
            clusterMask = ~((clusterMax - 1) << lsh);
            clusterId = HIBYTE(pMsg->wGMLevel) << lsh;

            rfalse(2, 1, "[提示]该服务器加入了数据库一拖多群集分组！[%d/%d][id=%08X|mask=%08X]", 
                HIBYTE(pMsg->wGMLevel), clusterMax, clusterId, clusterMask);

            assert((((~clusterMask) & 0x1fffffff) == 0) && ((clusterId & 0x1fffffff) == 0));
        }
        else
        {
            rfalse(0, 0, "群集分组数据错误，注册本机失败！" );
		    GetApp()->m_eLoginStatus = CDAppMain::SS_BREAK;
            return;
        }

        if ( pMsg->szDescription[0] != 0 )
        {
            // 统一放到LOGS目录下
            CreateDirectory( "LOGS", NULL );

            // 通过登陆服务器来指定日至保存目录
            char temp[256];
            dwt::strcpy( temp, pMsg->szDescription, 64 );

            extern dwt::stringkey< char [256] > szDirname;
            sprintf( szDirname, "LOGS\\%s", temp );

            CreateDirectory( ( LPCSTR )szDirname, NULL );
            strcat( szDirname, "\\" );

		    extern LPCSTR GetStringTime();
            rfalse( 1, 1, "[%s] [%d] connect to loginserver", GetStringTime(), GetServerID() );
			char temp1[100];
			memset(temp1,0,100);
			sprintf(temp1,"%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t","服务器","账号ID","名字","年月日","时分","级别","花费","宝石");
			rfalse(5,1,temp1);
        }
    }
	else
	{
		rfalse(0, 0, "注册本机失败");
		GetApp()->m_eLoginStatus = CDAppMain::SS_BREAK;
	}
}

BOOL SendRefreshPlayerMessage(int storeflag, DWORD gid, LPCSTR acc, SFixData *pData, SPlayerTempData *pTempData)
{
	if (!pData) 
		return FALSE;

	if (!pTempData)
		return FALSE;

	SARefreshPlayerMsg msg;
	msg.storeflag	= storeflag;
	msg.fixData = *pData;
	//msg.data		= *pData;
	//msg.tdata		= *pTempData;

	try
	{
		lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
		slm(acc);
		int num = SectionMessageManager::getInstance().evaluateDevidedAmount(sizeof(msg)-slm.EndEdition());
		int id = 0;
		for (int i = 0;i < num;i++)
		{
			SSectionMsg sMsg;
			id = SectionMessageManager::getInstance().devideMessage(i,num,&sMsg,&msg,sizeof(msg) - slm.EndEdition(),id);
			SendToLoginServer(&sMsg,sizeof(SSectionMsg));
		}
		return TRUE;//SendToLoginServer(&msg, (long)(sizeof(msg) - slm.EndEdition()));
	}
	catch( lite::Xcpt & )
	{
	}

	return FALSE;
}

static int SplitParameters( LPCSTR str, std::vector< std::string > &args )
{
    args.clear();
    if ( ( str == NULL ) || ( str[0] == 0 ) )
        return 0;

    int nLen = dwt::strlen( str, 256 );
    char tempstr[1024];
    memcpy( tempstr, str, nLen + 1 );

    WORD j = 1;
    short nCut[100] = {0};

    for ( int i = 0; i < nLen; i ++ )
    {
        if ( tempstr[i] == ',' )
        {
            tempstr[i] = 0;
            nCut[j++] = i + 1;
        }
    }

    for ( int i = 0; i < j; i ++ )
        args.push_back( tempstr + nCut[i] );

    return (int)args.size();
}

void CNetLoginServerModule::ProcessInterop( void *data, DWORD dbtype )
{
	char bufmsg[1024];
	extern LPCSTR GetStringTime();
	try
	{
		lite::Serialreader sl( data );

		switch ( (int)sl() )
		{
		case 0://RPC_ERROR:
			{
				LPCSTR spname = sl();

                extern dwt::stringkey<char [256]> szDirname;
				TraceInfo( szDirname + "rpcerr.log", "[%s]执行存储过程(%s)出现错误！", GetStringTime(), spname );

				_snprintf( bufmsg, 1023, "RPC ERROR ON EXECUTE STOREDPROCDURE: %s", spname );
				rfalse( 2, 1, bufmsg );
			}
			break;

		case SMessage::EPRO_ACCOUNT_MESSAGE:
            {
                int checkId = sl();
			    if ( checkId == SAccountBaseMsg::EPRO_REFRESH_CARD_POINT )
			    {
					DWORD staticId = sl();
					if ( CPlayer *player = ( CPlayer* )GetPlayerBySID( staticId )->DynamicCast( IID_PLAYER ) )
					{
						DWORD verifier = sl();
						int type  = sl();
						int result  = sl();
						int remaining = sl();
                        player->OnDecreasePointResult( verifier, result, type, remaining );
                    }
                    else
                    {
                        extern dwt::stringkey<char [256]> szDirname;
				        TraceInfo( szDirname + "decpoint.log", "[%s]扣点操作返回后，未发现对应SID的角色存在！[%08X]", GetStringTime(), staticId );
                    }
			    }
                // 这个操作只是用于取用于查询的各项点数
			    else if ( checkId == SAccountBaseMsg::EPRO_REFRESH_MONEY_PIONT )
			    {
				    DWORD staticId = sl();
                    if ( CPlayer *player = ( CPlayer* )GetPlayerBySID( staticId )->DynamicCast( IID_PLAYER ) )
                    {
				        LPCSTR resultString = sl();

                        std::vector< std::string > args; 
                        int argc = SplitParameters( resultString, args );
                        if ( argc > 4 ) argc = 4;

        //                for ( int i = 0; i < argc; i ++ )
		//					player->m_Property.checkPoints[i] = atoi( args[i].c_str() );
                    }
			    }
            }
			break;

		case SMessage::EPRO_SCRIPT_MESSAGE:
			switch ( ( int )sl() )
			{
            case SScriptBaseMsg::EPRO_SCORE_LIST:
                {
				    SAUpdateScoreListMsg msg;
				    if ( sl.maxSize() > sizeof ( msg.streamData )   )
					    return;

				    memcpy( msg.streamData, data, sl.maxSize() );
				    DNID dnid = sl();

				    g_StoreMessage( dnid, &msg, sizeof(msg) - sizeof ( msg.streamData ) + sl.maxSize() );	
                }
                break;

			case 14:/*新定义的消息 SScriptBaseMsg::EPRO_BA_MSG*/
			{
				SARebindLoginServerMsg msg;
				extern std::string sidname;

				LPCSTR ip = sl();
				LPCSTR port = sl();
				DNID dnid = sl();
				LPCSTR account = sl();
				LPCSTR password = sl();
				msg.m_dwGID = sl();
				msg.m_GMLevel = ( DWORD )sl();
				msg.m_dwType =  SARebindLoginServerMsg::REBIND_UNUSUAL;

				if ( ip == NULL || port == NULL || account == NULL || password == NULL )
					return ;

				try
				{
					lite::Serializer slz( msg.streamData, sizeof( msg.streamData ) );
					slz( ip );
					slz( port );
					slz( account );
					slz( password );
					slz( sidname.c_str() );

					g_StoreMessage( dnid, &msg, (DWORD)(sizeof( msg ) - slz.EndEdition()) );
				}
				catch ( lite::Xcpt & )
				{
				}	
			}
			break;

            case SScriptBaseMsg::EPRO_BACK_MSG:
                {
				    DWORD staticId = sl();
                    CPlayer *player = NULL;

                    if ( staticId != 0 )
                        player = ( CPlayer* )GetPlayerBySID( staticId )->DynamicCast( IID_PLAYER );

                    g_Script.SetCondition( NULL, player, NULL );

				    LPCSTR scriptFuncName = sl();
				    g_Script.PrepareFunction( scriptFuncName );
				    int args = sl();
				    for ( int i=0; i<args; i++ )
                    {
                        lite::Variant lvt = sl();
                        if ( lvt.dataType != lite::Variant::VT_RPC_OP && lvt.dataType != lite::Variant::VT_RPC_OP_LITE )
					        g_Script.PushParameter( lvt );

                        // 说明返回的是一个2次序列化后的数据块，并且应该反向序列化回lua_table
                        else if ( lvt._rpcop == OP_TABLE_DATA )
                        {
                            int tableType = sl();
                            lite::Variant block = sl();
                            if ( block.dataType == lite::Variant::VT_POINTER && block.dataSize >= 4 )
                            {
                                try
                                {
                                    lite::Serialreader slr( const_cast< void* >( block._pointer ) );
                                    g_Script.PushVariantTable( slr, tableType );
                                }
	                            catch ( lite::Xcpt & )
	                            {
                                    // 忽略错误？
	                            }
                            }
                        }
                    }

				    g_Script.Execute();
                    g_Script.CleanCondition();
                }
			    break;
            }
            break;
        }
	}
	catch ( lite::Xcpt &e )
	{
        extern dwt::stringkey<char [256]> szDirname;
		TraceInfo( szDirname + "rpcerr.log", "[%s]解析RPC结果出现错误：%s", GetStringTime(), e.GetErrInfo() );
		rfalse( 2, 1, e.GetErrInfo() );
		return;
	}
}
