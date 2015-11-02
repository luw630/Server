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
		// ����Ϊ�����ϣ�
		// �������ظ���������
		for (int i=0; i<86400; i++)
		{
			// ����Ҫ���Լ�ע�ᵽ��½��������ȥ
			Sleep(1000);

			if ( !Connect( GetApp()->szLoginIP, GetApp()->szLoginPORT ) )
			{
				rfalse(2, 1, "�޷����ӵ�½������������");
				continue;
			}

			// �������ע�᱾��
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
			// ���ﴦ����ɲ���,ԭ����ORB��Ⱥ����Ϣһ��
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
		// ���ﴦ��ˢ�²���
		RecvRefreshMsg((SRefreshBaseMsg*)pMsg);
		break;

	case SMessage::EPRO_SERVER_CONTROL:
		// ���ﴦ�������������Ϣ
		RecvCtrlMsg((SServerCtrlMsg *)pMsg  );
		break;

	case SMessage::EPRO_REBIND_MESSAGE:
		// ���ﴦ�������ض������֤����
		RecvCheckRebind((SACheckRebindMsg *)pMsg);
		break;

	case SMessage::EPRO_SYSTEM_MESSAGE:
		// ��������ע���Ļ�Ӧ
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

					TalkToDnid(pPlayer->m_ClientIndex,"�����Ѹ��������ˣ���ȥ�����ɣ�");
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
		// �ش����������ݵ�ԭ��������������ϣ�
		if ( ( ( SBuildingBaseMsg* )data )->_protocol == SBuildingBaseMsg::EPRO_BUILDING_GET )
        {
            SABuildingGetMsg *pMsg = ( SABuildingGetMsg* )data;
            if ( pMsg->retMsg != SABuildingGetMsg::RET_SUCCESS )
                break;

            if ( CRegion *region = ( CRegion* )FindRegionByID( pMsg->wParentID )->DynamicCast( IID_REGION ) )
                region->LoadBuildings( pMsg );
            else 
                BEGIN_NOTIFY ( "ͳ�Ʒ��س��������ﵼ�´�������" )[ GetServerID() ][ pMsg->dwZoneID ][ pMsg->wParentID ] END_NOTIFY
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
    // GM���߲�ѯ�������ĳһ�ͻ���
    case 102:
        {
            extern BOOL RpcControlGMTool( LPVOID stream, DNID clinet );

            DNID *clinet = (DNID*)( ( ( LPBYTE )pMsg ) + 4 );
            LPBYTE data = ( ( LPBYTE )pMsg ) + 12;

            RpcControlGMTool( data, *clinet );
        }
        break;

	// �����Ǵ����ݿ⾭��LOGIN�����������а���Ϣ
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
		rfalse(4,1,"CNetLoginServerModule::RecvRefreshMsg(SRefreshBaseMsg *pMsg)ΪNULL");
		return;
	}
	switch (pMsg->_protocol)
	{
	case SRefreshBaseMsg::EPRO_REFRESH_REGION_SERVER:
		// ���ڳ�����������˵������ʹ���ⲿ��������ˢ���Լ��Ļ�������
		// ����������ܵ�ֻ������ˢ�����ݵ���Ϣ
		{
			SARefreshRegionServerMsg msg;
			SQRefreshRegionServerMsg *pRf = (SQRefreshRegionServerMsg *)pMsg;
			if (!pRf)
			{
				return;
			}
			// ���У������
			msg.dnidClient = pRf->dnidClient;
			msg.key = pRf->key;
			msg.gid = pRf->gid;

			// �����������
			msg.dwIP = GetApp()->m_dwIP;
			msg.dwPort = GetApp()->m_dwPort;
			msg.wTotalPlayers = (WORD)GetPlayerNumber();

#define WORDFLOOR(_fourbyte) (WORD)((_fourbyte > 0xffff) ? (0xffff) : (_fourbyte));

			msg.wPlayers[0] = WORDFLOOR(g_CurOnlineCheck[0]);
			msg.wPlayers[1] = WORDFLOOR(g_CurOnlineCheck[1]);
			msg.wPlayers[2] = WORDFLOOR(g_CurOnlineCheck[2]);
			msg.wPlayers[3] = WORDFLOOR(g_CurOnlineCheck[3]);

            // �Զ�ս������Ҿ�����һ��������ߣ�
            // ��鲻Ҫ�ˣ�
            // ���ǰ� ��������ϸ��Ϊ��������Ϸ�����߹һ�����
            // ���߹һ� = ������ - ( �쳣 + ���� + ���� + �һ� )
            
            //extern BOOL afLikeHungup;
            //if ( afLikeHungup )
            //    msg.wPlayers[3] += WORDFLOOR(g_CurOnlineCheck[4])
            //else
            //    msg.wPlayers[1] += WORDFLOOR(g_CurOnlineCheck[4])

			{// д��ɼ�����
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

			// ��䳡����������
			FillRegionInfoMsg(msg, pRf->bFullQuest);

			// ���ظ���½������
			SendMsgToLoginSrv(&msg, msg.GetMySize());
		}
		break;

	case SRefreshBaseMsg::EPRO_REFRESH_PLAYER:
		{

		}
		// Ŀǰ��û�����ˢ�µ�����
		break;

        // �����׼����ˢ�ű�����Ϣ
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
	case SPointModifyMsg::EPRO_ENCOURAGEEMENT://�齫
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
				// ��½�ͷ��ͷ�����ʱ��
				DWORD tick[2] = { static_cast<const LPDWORD>(static_cast<void *>("tick"))[0], (DWORD)time(NULL) };
				g_StoreMessage(pSpd->dnidClient, tick, 8);
				if (!GetApp()||!GetApp()->m_pGameWorld)
				{
					rfalse(4,1,"GetApp()->m_pGameWorld->SetPlayerData Ϊ NULL");
					return;
				}
				if (GetApp()->m_pGameWorld->SetPlayerData(pSpd->dnidClient, pSpd->gid, &pSpd->data, &pSpd->tdata))
				{
					
					//����ʱ������һ����ҵ�����Ӧ�õ�½���Ҵ�db�л�ȡ���, ֪ͨ��¼�����������������¼���
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
							rfalse(2, 1, "�������<%s>��½��ʱ%d", pPlayer->GetAccount(),tick);
						}
					}
				}
				else
				{
					rfalse(2, 1, "��� [%d] ��¼SSetPlayerDataMsg ʧ��", pSpd->gid);
					g_CutClient(pSpd->dnidClient);  //����ֱ�ӶϿ�����
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
		rfalse(4,1,"CNetLoginServerModule::RecvCheckRebind ΪNULL");
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
		rfalse(1, 1, "check : [%s] �յ�CheckRebind ret = %s", GetStringTime(), account);
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
		// ������ɹ�����������
		g_SetLogStatus(pMsg->dnidClient, LinkStatus_Connected);

		
		// ͬʱ֪ͨ�ͻ��˿��Է�������
		SACheckRebindMsg msg;
		msg.SMessage::_protocol = 100;
		msg.dwResult			= 2;
		msg.dnidClient			= 0;
		msg.gid					= 0;

		try
		{
			lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
			slm(account);
			///����ʧ�ܵĻ��Ͳ���������ݹ�ȥ��,��Ϊ���ڿͻ����Ǹ�����û���յ������Ϣ���ж��Ƿ������ϵ�
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
		// δ֪У��ֵ
		rfalse(2, 1, "δ֪���ض���У��ֵ%d����", pMsg->dwResult);
		g_CutClient(pMsg->dnidClient);
		return;
	}
	if (!GetApp() || !GetApp()->m_pGameWorld)
	{
		rfalse(4,1,"!GetApp() || !GetApp()->m_pGameWorld ΪNULL");
		return;
	}
// 	std::map<DWORD, DWORD>::iterator iter = msgtime.find(pMsg->dnidClient);
// 	if (iter == )
// 	{
// 	}

	SIZE_T smemsize = GetMemoryInfo() / 1024;
	rfalse(2, 1, "CreateNewPlayer  %d", smemsize);

	msgtime[pMsg->dnidClient] = GetTickCount();
	// �������Ӧ����Ҷ���
	int ret = GetApp()->m_pGameWorld->CreateNewPlayer(pMsg->dnidClient, account, pMsg->gid, limitedState, online, offline, puid);
	if (1 == ret)
	{
		//rfalse(2, 1, "SGetPlayerDataMsg :[%d]", pMsg->gid);
		// ͬʱˢ������
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

		//TODO:Tony Modify [2012-3-8]Comment:[��ҵ�½��ʱ���͸����ķ�]
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
		rfalse(4,1,"!GetApp() CNetLoginServerModule::RecvLoginMessage(SALoginMsg *pMsg)ΪNULL");
		return;
	}
	if (!pMsg)
	{
		rfalse(4,1,"!pMsg CNetLoginServerModule::RecvLoginMessage(SALoginMsg *pMsg)ΪNULL");
		return;
	}
	if (pMsg->dwRetCode == SALoginMsg::ERC_LOGIN_SUCCESS)
	{
		if ( GetApp()->m_eLoginStatus == CDAppMain::SS_LOGIN )
			 GetApp()->m_eLoginStatus = CDAppMain::SS_INIT;

        // ͨ��wGMLevel��ָ�����ݿ�Ⱥ���������ݣ����ֱ�ʾȺ�����������ֱ�ʾȺ�����
        BYTE clusterMax = LOBYTE(pMsg->wGMLevel);
        if (clusterMax == 0 || clusterMax == 1)
        {
            rfalse(2, 1, "[��ʾ]�÷�������δ�������ݿ�һ�϶�Ⱥ�����飡");
        }
        else if ((clusterMax <= 8 ) &&									// Ŀǰ�趨������ݿ�1��8
				((clusterMax) > HIBYTE(pMsg->wGMLevel)) &&              // ����ID�ض�С������޶�
				((clusterMax & ~(clusterMax - 1)) == clusterMax))		// �趨����Ŀ�ض�Ϊƽ����
        {
            extern DWORD clusterMask, clusterId;

            // ���¸ù�ʽ����((v & ~(v-1)) == v)����֤��
            DWORD lsh = ((clusterMax == 2) ? 31 : ((clusterMax == 4) ? 30 : 29));
            clusterMask = ~((clusterMax - 1) << lsh);
            clusterId = HIBYTE(pMsg->wGMLevel) << lsh;

            rfalse(2, 1, "[��ʾ]�÷��������������ݿ�һ�϶�Ⱥ�����飡[%d/%d][id=%08X|mask=%08X]", 
                HIBYTE(pMsg->wGMLevel), clusterMax, clusterId, clusterMask);

            assert((((~clusterMask) & 0x1fffffff) == 0) && ((clusterId & 0x1fffffff) == 0));
        }
        else
        {
            rfalse(0, 0, "Ⱥ���������ݴ���ע�᱾��ʧ�ܣ�" );
		    GetApp()->m_eLoginStatus = CDAppMain::SS_BREAK;
            return;
        }

        if ( pMsg->szDescription[0] != 0 )
        {
            // ͳһ�ŵ�LOGSĿ¼��
            CreateDirectory( "LOGS", NULL );

            // ͨ����½��������ָ����������Ŀ¼
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
			sprintf(temp1,"%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t","������","�˺�ID","����","������","ʱ��","����","����","��ʯ");
			rfalse(5,1,temp1);
        }
    }
	else
	{
		rfalse(0, 0, "ע�᱾��ʧ��");
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
				TraceInfo( szDirname + "rpcerr.log", "[%s]ִ�д洢����(%s)���ִ���", GetStringTime(), spname );

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
				        TraceInfo( szDirname + "decpoint.log", "[%s]�۵�������غ�δ���ֶ�ӦSID�Ľ�ɫ���ڣ�[%08X]", GetStringTime(), staticId );
                    }
			    }
                // �������ֻ������ȡ���ڲ�ѯ�ĸ������
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

			case 14:/*�¶������Ϣ SScriptBaseMsg::EPRO_BA_MSG*/
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

                        // ˵�����ص���һ��2�����л�������ݿ飬����Ӧ�÷������л���lua_table
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
                                    // ���Դ���
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
		TraceInfo( szDirname + "rpcerr.log", "[%s]����RPC������ִ���%s", GetStringTime(), e.GetErrInfo() );
		rfalse( 2, 1, e.GetErrInfo() );
		return;
	}
}
