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

		// �ɼ�����
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

		// �Զ��һ�����


		// ˢ���������������
		QueryRefreshGameServer();

		// ���˺ŷ������ύ������״̬��Ϣ
		SUpdateServerState msg;
		msg.ServerID = Globals::GetServerId();
		msg.PlayerNumber = (m_dwCurPlayerNumber & 0xffff) | (autoFightPlayers << 16);
		msg.ServerState = ss << 16;
		msg.ServerState |= data.dwTotalPlayers - data.dwTotalHPlayers;
		msg.TotalMac = msg.PlayerNumber;
		memset(msg.streamData, 0, MAX_ONLINEEXTEND);
		Globals::SendToAccountServer(&msg, sizeof(msg), reinterpret_cast<LPCSTR>(-1));

		// ���²�ѯ�õķ�����״̬��Ϣ
		// extern size_t max_player_limit;
		UpdateServerStatus(ss, m_dwCurPlayerNumber, m_wQueuePlayerCount, (int)max_player_limit);

		// �������������������Ŷ�
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
	//             // ע������쳣ʱ���ص����ݣ�����
	//             [OP_BEGIN_ERRHANDLE] ( (int)RPC_ERROR ) ( spname ) [OP_END_ERRHANDLE]
	// 
	//             // ׼���洢����
	//             [OP_PREPARE_STOREDPROC] (spname)
	// 
	//                 // �趨���ò���
	//                 [OP_BEGIN_PARAMS]
	//                 (1) ( abs_account )
	//                     (2) ( -( pMsg->dwEQVPoint * pMsg->nNumber ) )
	//                     (3) ( (int)pMsg->consumable )
	//                     (4) ( (int)pMsg->nIndex )
	//                     (5) ( (int)pMsg->nLevel )
	//                     (6) ( (int)Globals::GetServerId() )
	//                     [OP_END_PARAMS]
	// 
	//                     // ���ô洢����
	//                     [OP_CALL_STOREDPROC]
	// 
	//                     // ��ʼ���������ݺ�
	//                     [OP_INIT_RETBOX]    (1024)
	// 
	//                         // ���ݺеĵ�һ��ֵ
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
	//                         // ��֮ǰ�Ĳ���ֵװ��
	// 
	//                         // 0 Ϊ��׼�洢���̷���ֵ
	//                         [OP_BOX_PARAM]      (0)
	// 
	//                         //[OP_BOX_PARAM]      ("@RETURN_VALUE")
	// 
	//                         // �������ݺ�
	//                         [OP_RETURN_BOX]
	// 
	//                         // ����
	//                         [OP_RPC_END];
	// 
	//                         DWORD nullSize = (DWORD)slmaker.EndEdition();
	//                         Globals::SendToAccountServer( &msg, sizeof( msg ) - nullSize, prefix );
	//     }
	//     catch ( lite::Xcpt &e )
	//     {
	//         TraceInfo( "rpcerr.log", "[%s]׼��RPC�������ִ���%s", Globals::GetStringTime(), e.GetErrInfo() );
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
	//             // ע������쳣ʱ���ص����ݣ�����
	//             [OP_BEGIN_ERRHANDLE] ( (int)RPC_ERROR ) (spname) [OP_END_ERRHANDLE]
	// 
	//             // ׼���洢����
	//             [OP_PREPARE_STOREDPROC] (spname)
	// 
	//                 // �趨���ò���
	//                 [OP_BEGIN_PARAMS]
	//                 (1) ( abs_account )
	//                     [OP_END_PARAMS]
	// 
	//                     // ���ô洢����
	//                     [OP_CALL_STOREDPROC]
	// 
	//                     // ��ʼ���������ݺ�
	//                     [OP_INIT_RETBOX]    (1024)
	// 
	//                         // ���ݺеĵ�һ��ֵ
	//                         [OP_BOX_VARIANT]    ( (int)SMessage::EPRO_POINTMODIFY_MESSAGE )
	//                         [OP_BOX_VARIANT]    ( (int)SPointModifyMsg::EPRO_EQUIVALENT_BASE )
	//                         [OP_BOX_VARIANT]    ( (int)SEquivalentModifyMsg::EPRO_GET_EQUIVALENT )
	//                         [OP_BOX_VARIANT]    ( (int)pMsg->type )
	//                         [OP_BOX_VARIANT]    ( account )
	//                         [OP_BOX_VARIANT]    ( dnid )
	// 
	//                         // ��֮ǰ�Ĳ���ֵװ��
	// 
	//                         // 0 Ϊ��׼�洢���̷���ֵ
	//                         [OP_BOX_PARAM]      (0)
	// 
	//                         //[OP_BOX_PARAM]      ("@RETURN_VALUE")
	// 
	//                         // �������ݺ�
	//                         [OP_RETURN_BOX]
	// 
	//                         // ����
	//                         [OP_RPC_END];
	// 
	//                         DWORD nullSize = (DWORD)slmaker.EndEdition();
	//                         Globals::SendToAccountServer( &msg, sizeof( msg ) - nullSize, prefix );
	//     }
	//     catch ( lite::Xcpt &e )
	//     {
	//         TraceInfo( "rpcerr.log", "[%s]׼��RPC�������ִ���%s", Globals::GetStringTime(), e.GetErrInfo() );
	//         rfalse( 2, 1, e.GetErrInfo() );
	//     }
}

static void ProcessPointModifyMsg(DNID dnid, SPointModifyMsg *pMsg)
{
	switch (pMsg->_protocol)
	{
		// ͨ������
	case SPointModifyMsg::EPRO_MONEYPOINT_BASE:
	{
		// ��ʱ����[ֻ���շѷ����õ�]
		//SMoneyPointModifyMsg *pMPMsg = (SMoneyPointModifyMsg *)pMsg;
		//if(pMPMsg->_protocol == SMoneyPointModifyMsg::EPRO_REFRESH_MONEYPOINT)
		//	NotifyMoneyPointMsg( (SQRefreshMoneyPointMsg *)pMsg);
		//else if(pMPMsg->_protocol == SMoneyPointModifyMsg::EPRO_NOTIFY_MONEYPOINT)
		//	NotifyMoneyPointMsgCo((SQNotifyMoneyPointMsg *)pMsg);
	}
		break;

		// Ԫ������
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
			// ����������Ԫ���ù��ܣ�������������
		}
	}
		break;

		// �Խ�
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
		// ˵�������ӶϿ�

		// ���ﴦ���ڲ��������Ķ���
		std::map<DNID, SServer>::iterator it = gameServers.find(dnidClient);
		if (it == gameServers.end())
			// ���������Ӻ���Ϸ�������޹أ��Ͳ��ù���
			return true;

		// ������Ҫ���÷������ϵ���������˳���Ϸ[������]
		// GetApp().m_PlayerManager.OnServerShutdown( it->second.key );

		// �Ƴ��÷����������������
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
		// ������LinkStatus_Waiting_Login�ȴ���֤��Ϣ��״̬�У�ֻ�ܽ�����֤��Ϣ
		// ���ж��Ƿ��ǵ�½��֤������Ϣ
		if (pMsg->_protocol != SMessage::EPRO_SYSTEM_MESSAGE)
		{
			server.DelOneClient(dnidClient);
			return true;
		}

		// Ȼ���ж��Ƿ�Ϊ��֤��Ϣ
		if (((SSysBaseMsg*)pMsg)->_protocol != SSysBaseMsg::EPRO_LOGIN)
		{
			server.DelOneClient(dnidClient);
			return true;
		}

		// ��ʼ��֤����״̬���ƽ�һ��
		server.SetLinkStatus(dnidClient, LinkStatus_Login);

		// ���ﴦ��������ĵ�½
		OnServerLogin(dnidClient, (SQLoginMsg *)pMsg);
		break;

	case LinkStatus_Login:
		// �����ڵȴ���֤��ʱ�򣬸����Ӳ��������κ���Ϣ

		// �����յ���Ϣ�ͶϿ�������
		server.DelOneClient(dnidClient);
		break;

	case LinkStatus_Connected:
		// ��������Ϣֻ������֤�Ժ��״̬��ʹ�ã�������Ϊ�Ƿ�(���ӶϿ�)
		if (pMsg->_protocol == SMessage::EPRO_TONG_MESSAGE)
		{
			// ��������򷢹���Ҫ��ɾ�����ɵģ�ֻ���Բ�����������ֱ����ô��
			// ����Ϊ�����������޷������Ч����������,������Ҫ�ڵ�½�������һ�β�ѯ
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
				// ���ڵ�½��������˵�������������ǲ���Ҫ��ʹ�õ�½��������������ˢ���Լ�
				// ��������ֻ�������Ϣ��Ӧ��
				RefreshRegionServer((SARefreshRegionServerMsg*)pRf, dnidClient);
				break;

			case SRefreshBaseMsg::EPRO_REFRESH_PLAYER:
				// ���ڵ�½��������˵�������������ǲ���Ҫ��ʹ�õ�½��������������ˢ���Լ�
				// ��������ֻ�������Ϣ��Ӧ��
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
			case SRefreshBaseMsg::EPRO_REFRESH_PLAYERLOGINCOMPLTE: //��ҵ�¼ˢ��,��¼�ɹ�ˢ������
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
			// ����Ӧ��ֻ���յ��ض�����֤��Ϣ�ģ�Ҳ�ͷ�����ڲ����������ض��������
			if (Globals::DoCheckRebindMsg(dnidClient, (SQCheckRebindMsg*)pMsg))
			{
				// �����֤ͨ������Ҫָʾ��һ���Ĳ���
				// Ŀǰ�Ѿ���DoCheckRebindMsg�д������
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
			// ������ʱֻ���з���������ʱ�������Ϣ��
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
			//�������� ֱ��ת������Ϣ����ѹ����ָ� 
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

			// ���ˣ����û�������һ��
			if (((_SDataBaseMsg*)pMsg)->_protocol == _SDataBaseMsg::EPRO_DOUBLE_SAVE_PLAYER)
			{
				SDoubleSavePlayerMsg *dsp = (SDoubleSavePlayerMsg*)pMsg;
				if (!Globals::DoubleUpdatePlayerData(dsp))
				{
					// ͬ�����³����˴��󣬸Ͻ���顣����
					rfalse(2, 1, "�ڽ��׺�ͬ�����³����˴��󣬸Ͻ���顣����");
					break;
				}

				dsp->serverId = Globals::GetServerId();

				rfalse(3, 1, "[%s] double savelog : [%s:%d] [%s:%d]", Globals::GetStringTime(),
					dsp->account[0], dsp->gid[0], dsp->account[1], dsp->gid[1]);

				// DBSave��Ϣ�� �ڷ���DBServerǰ������ѹ�����ݣ�
				extern BOOL TryEncoding_NilBuffer(const void *src_data, size_t src_size, void *&dst_data, size_t &dst_size);

				// �����ض����°汾�ˣ�ֱ�ӷ���ѹ������
				LPVOID outData = NULL;
				size_t outSize = 0;
				size_t validSize = dsp->gid[1] ? sizeof(SFixProperty)* 2 : sizeof(SFixProperty);
				BOOL result = TryEncoding_NilBuffer(dsp->data, validSize, outData, outSize);
				if (result && outSize < validSize - 256)
				{
					// ѹ���ɹ���ֱ����ԭ�������ݻ�����ֱ���޸��������ݺ����ݴ�С��
					LPDWORD dataBuf = (LPDWORD)&dsp->data;
					*dataBuf = (WORD)outSize;
					memcpy(&dataBuf[1], outData, outSize);

					// ���¼��㷢�ʹ�С
					size_t segment = (size_t)&(((SDoubleSavePlayerMsg*)0)->data[0]);
					size = segment + 4 + outSize;
				}
				else
				{
					rfalse(4, 1, (result) ? "DBSAVE ����ѹ�����С�������䣡" : "DBSAVE ����ѹ������");
				}
			}

			// ֱ��ת����Ϣ�����ݿ��������Ŀǰ���漰���ľ�ֻ�н�ɫ����������Ϣ
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
		// GM���߸ý��յ���Ϣ
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
		// rfalse(2, 1, "�쳣������յ����쳣���ӵ���Ϣ[i=%d][t=%d]", nClientIndex, g_GetLogStatus(nClientIndex));
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

	// �������ݿ�Ⱥ���������Ϣ������ͬ�����ݣ���������ĵ���ID�ظ�
	extern int clusterMax, clusterId;
	msg.wGMLevel = MAKEWORD(clusterMax, clusterId);

	if (HIBYTE(pLog->wVersion) != SERVER_VERSION)
	{
		// �汾��ͬ
		msg.dwRetCode = SALoginMsg::ERC_INVALID_VERSION;
	}

	// ��Ҫ�ӵ�½�˺��н����������������ͣ����һ��16����Ч����
	// ͷ�������֡���½|����|����|����|��չ����ʶ���������ͣ�Ȼ�����_���������Ϊ���ֱ��
	char number[32];
	DWORD ID = 0;

	memcpy(number, account + 5, 4);
	number[4] = 0;
	if (dwt::strcmp("����", account, 4) == 0)
	{
		ID = 0x10000000 | (0x0fffffff & atoi(number));
	}
	else
	{
		server.DelOneClient(dnidClient);
		return rfalse(1, 1, "�Ƿ��ķ���������!!!");
	}

	// ����½������������ʱ�䷵�ظ����������������ָ��������־��Ŀ¼
	dwt::strcpy(msg.szDescription, timeSegment, 60);

	// ������������ͨ��������֤����
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
		// �÷�����δע���
		// ������Ҫ����ע�����
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

		// �÷������Ѿ�ע���
		if (s.idControl != 0)
		{
			// ����÷����������ӻ�����
			// ��Ϊ��Ӧ���滻���е�Ŀ�����������
			// ��ô��Ϊ������������Ч��
			server.DelOneClient(dnidClient);

			// �����������һ����Ч��Ϣ���Բ���Ŀ�����Ч�ԣ�
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

			return rfalse(2, 1, "Ŀ������������Ѿ�����!!!");
		}

		// �����µķ���������
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
	// ˢ���������������
	if (pRf == NULL)
		return false;

	// ������Ϣ����Ҫ�Ǵ��������ϵĹ̶���������Ӻ�ɾ��
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

	// �ɼ�����
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
		// ��ע��,ֱ�Ӹ���(����˵�ظ��Ļ�)
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
	//�����������ȡ�������
	//�������°���ɣ����������Ӧ�ÿ��������������,�����¼��������������������
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
			// �ȼ����������
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

	// ������û���ҵ�Ŀ�곡����������Ժ���Կ��ǰ�Ŀ�����ת�Ƶ�һ���յ����ⳡ��
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
		// ���serverKey��Ϊ0����ʾ�ý�ɫ��Ϊĳ��ԭ�򣬻������ڷ�������
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

		// ֪ͨ��ҿͻ��˽��г���ת��
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

	// ����ҷ��볡��ʧ��
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
	rfalse(2, 1, "�������б� %d ����ע�������", gameServers.size());

	DWORD players = 0;
	int playerst[4] = { 0, 0, 0, 0 };
	int count = 0;
	for (std::map<DNID, SServer>::iterator it = gameServers.begin();
		it != gameServers.end(); it++)
	{
		SServer *pS = &it->second;

		rfalse(2, 0, "\t���[%d] ����/���ֵ[%d/%d] %s %s",
			count++, pS->wTotalPlayers, pS->wMaxPlayers,
			pS->account.c_str(), (pS->idControl != 0) ? "����" : "���Ӷ�ʧ");

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

	//rfalse(2, 0, "��ǰ������/�ײ������� �� %d/%d", m_PServer.GetClientNum(), m_PServer.GetClientNum_Dll());

	// �Զ��һ�����
	WORD autoFightPlayers = (WORD)players - (playerst[0] + playerst[1] + playerst[2] + playerst[3]);

	rfalse(2, 0, "�쳣/����/����/�һ�/�Զ���%d/%d/%d/%d/%d", playerst[0],
		playerst[1], playerst[2], playerst[3], autoFightPlayers);

	rfalse(2, 0, "��ǰ�Ŷ���������%d/%d/%d/%d", g_dConcurrencyPlayerNum, m_wQueuePlayerCount, playerloginlist.size(), g_dConcurrencyAllPlayerNum);

	rfalse(2, 0, "��ǰ������/��ʷ������� �� %d/%d\r\n"
		"������������ �� %d\r\n"
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

	//rfalse(2, 0, "��ǰ������/�ײ������� �� %d/%d", m_PServer.GetClientNum(), m_PServer.GetClientNum_Dll());

	// �Զ��һ�����
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
