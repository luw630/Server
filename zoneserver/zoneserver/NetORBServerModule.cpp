#include "StdAfx.h"
#include "netorbservermodule.h"
#include "gameobjects/Player.h"
#include "networkmodule/teammsgs.h"
#include "gameobjects/playermanager.h"
#include "networkmodule/chatmsgs.h"
#include "networkmodule/RelationMsgs.h"
#include "Gameobjects\Gameworld.h"
#include "gameobjects/factionmanager.h"
#include "networkmodule/RegionMsgs.h"

#include <boost/bind.hpp>

extern bool g_check;

#define UNDISABLE_ORB

extern WORD GetServerID();
extern int SendToORBServer(SORBMsg *, long);
extern int g_StoreMessage(DNID dnidClient, LPVOID pMsg, WORD wSize);
extern void SendMessageToPlayer(LPCSTR szName, SMessage *pMsg, WORD wSize);
extern void KeepLive();
extern void BroadcastMsg( SMessage *, WORD );
extern void TalkToPlayer(LPCSTR szName, LPCSTR info);
extern void SendMessageToDestPlayerServer(LPCSTR szName, SMessage *pMsg, WORD wSize);

void AssignTeamMemberInfo(SACSTeamOperation &CSTeamOperation, SATeamOperation *pATeamOperation, int ExcludeID);
DWORD CaculateTeamExp(WORD wExp, WORD *ExpArray, CPlayer **ppPlayer, WORD wMemberNum);
BOOL IsPlayerInRange(CPlayer *pPlayerCore, CPlayer *pPlayer, int Range);
//BOOL IsPlayerInView(CPlayer *pPlayerCore, CPlayer *pPlayer);

CNetORBServerModule::CNetORBServerModule(void)
{
	m_prevCheckTime = (int)timeGetTime() ;
}

CNetORBServerModule::~CNetORBServerModule(void)
{
}

void CNetORBServerModule::Execution()
{
#ifdef DISABLE_ORB
    return ;
#endif

    CNetConsumerModule::Execution( boost::bind( OnDispatch, this, _1, _2 ) );

    if ( (int)timeGetTime() - (int)m_prevCheckTime > (int)AUTOCONNECTORBTIME )
    {
        KeepLive();
        m_prevCheckTime = timeGetTime();
    }
	
	return;
}


BOOL CNetORBServerModule::SendMsgToORBSrv(SORBMsg *pMsg, int iSize)
{
#ifdef DISABLE_ORB
    return FALSE;
#endif

    if (pMsg == NULL) 
        return FALSE;

    return SendMessage(pMsg, iSize);
}

bool CNetORBServerModule::OnDispatch( void *data, size_t size  )
{
#ifdef DISABLE_ORB
    return 1;
#endif

    SMessage *pMsg = (SMessage *)data;

    if (pMsg == NULL)
        // if check server disconnected this link, get in here
        return true;

    if (pMsg->_protocol != SMessage::EPRO_ORB_MESSAGE) 
        // �Ƿ�����Ϣ
        return true;

    SORBMsg *pOm = (SORBMsg*)pMsg;

    if (pOm->dnidClient == 0) 
        return true;

    switch (pOm->_protocol) 
    {
    case SORBMsg::EPRO_CTRL_CHAT:
        {
            GetGW()->m_NetGMModule.GetORBChatMessage(pOm);
            OnDispatchCtrlChatMsg((SORBBaseChatWisperMsg *)pMsg);
        }
        break;
    case SORBMsg::EPRO_CTRL_SIMPLAYER:
        OnDispatchCtrlSimplayerMsg((SCtrlSimPlayer*)pMsg);
        break;

    case SORBMsg::EPRO_CTRL_CHECK:
        OnDispatchCtrlCheckMsg((SCtrlCheckMsg*)pMsg);
        break;

    case SORBMsg::EPRO_CTRL_FACTION:
        OnDispatchCtrlFactionMsg((SCtrlFactionMsg*)pMsg);
        break;

    case SORBMsg::EPRO_CTRL_DATATRANS:
        OnDispatchCtrlTranDataMsg((SQAORBDataTransMsg*)pMsg);
        break;
    }

    return true;
}

void CNetORBServerModule::OnDispatchCtrlTranDataMsg(SQAORBDataTransMsg *pMsg)
{
    switch (pMsg->GetTransType())
    {
	case SQAORBDataTransMsg::TRANSDATATOPLAYER:
        if (CPlayer *pPlayer = (CPlayer *)GetPlayerByName(pMsg->GetTransName())->DynamicCast(IID_PLAYER))
		{
			SMessage* pMsgTmp1 = (SMessage*)(pMsg->GetTransData());
			if ( pMsgTmp1->_protocol == SMessage::EPRO_RELATION_MESSAGE ) // �ô�Ϊ���⴦��
			{
				SRelationBaseMsg* pMsgTmp2 = (SRelationBaseMsg*)pMsgTmp1;
				if ( pMsgTmp2->_protocol == SRelationBaseMsg::EPRO_FRIENDONLINE )
				{
					if ( !pPlayer->FindRelation(((SFriendOnlineMsg*)pMsgTmp2)->cName, 0) ) //�Է��Ƿ����Լ�����
						break;
				}
			}

            g_StoreMessage( pPlayer->m_ClientIndex, (LPVOID)pMsg->GetTransData(), pMsg->GetTransSize() );
		}
        break;

	case SQAORBDataTransMsg::TRANSDATATO_DESTPLAYERSERVER:
        if (CPlayer *pPlayer = (CPlayer *)GetPlayerByName(pMsg->GetTransName())->DynamicCast(IID_PLAYER))
            GetGW()->OnDispathDTM((SMessage*)pMsg->GetTransData(), pMsg->GetTransSize(), pPlayer);
        break;

	case SQAORBDataTransMsg::TRANSDATATO_ALLSERVER:
        if ( *( LPWORD )pMsg->GetTransName() == 1 ) 
            BroadcastMsg( ( SMessage* )pMsg->GetTransData(), pMsg->GetTransSize() );
        else
            GetGW()->OnDispathDTM((SMessage*)pMsg->GetTransData(), pMsg->GetTransSize());
        break;

	case SQAORBDataTransMsg::PLAYERINFO_QUERY:
        if ( dwt::IsBadStringPtr( ( LPCSTR )pMsg->GetTransData(), 11 ) )
            break;

        if ( CPlayer *pDestPlayer = ( CPlayer* )GetPlayerByName( pMsg->GetTransName() )->DynamicCast( IID_PLAYER ) )
        {
            // ȷ���ǲ�ѯ���ѻ��ǳ��˻���ʲô������ʲôʲô��...
            BYTE mode = ( ( LPCSTR )pMsg->GetTransData() )[11];

            // ȷ���£��Է��Ƿ����Լ�����,���ѹ�ϵ�������໥��
			if ( ( mode == 0 ) && !pDestPlayer->FindRelation( ( LPCSTR )pMsg->GetTransData(), 0 ) ) 
            {
                TalkToPlayer( ( LPCSTR )pMsg->GetTransData(), "�㲢���ǲ�ѯĿ��ĺ���!" );
                break;
            }

			SAQueryPlayerMsg msg;
            msg.mode     = mode;
			msg.bySchool = pDestPlayer->m_Property.m_bySchool;
			msg.bSex     = pDestPlayer->m_Property.m_bySex;
			msg.wPKValue = pDestPlayer->m_Property.m_byPKValue;
			msg.nXValue  = pDestPlayer->m_Property.m_sXValue;
			msg.wRegion  = pDestPlayer->m_Property.m_wCurRegionID;
			msg.wXPos    = pDestPlayer->m_Property.m_wSegX;
			msg.wYPos    = pDestPlayer->m_Property.m_wSegY;
            dwt::strcpy( msg.szName, pDestPlayer->m_Property.m_szName, 11 );
			dwt::strcpy( msg.szTitle, pDestPlayer->m_Property.m_szTitle, 11 );
			dwt::strcpy( msg.szFactionName, pDestPlayer->m_Property.m_szTongName, 11 );
            
            // �������ͨ��ѯĿ�����,�򲻸�����ϸ����
            if ( mode == 1 )    
                msg.wRegion = msg.wXPos = msg.wYPos = 0;

            SendMessageToDestPlayerServer( ( LPCSTR )pMsg->GetTransData(), &msg, sizeof( SAQueryPlayerMsg ) );
        }
        else
        {
            // Ӧ����ORBȷ������ڸ÷������ϲŻ��յ������Ϣ�����Բ�������Ҳ���������
            // ������Ϣ��������û�����zgc
            TalkToPlayer( ( LPCSTR )pMsg->GetTransData(), "���ѯ��Ŀ�겻�����ϣ�" );
        }
		break;

    default:
        return;
    }
}

void CNetORBServerModule::OnDispatchCtrlFactionMsg(SCtrlFactionMsg *pMsg)
{
}

void CNetORBServerModule::OnDispatchCtrlCheckMsg(SCtrlCheckMsg * pMsg)
{

    switch(pMsg->_protocol)
    {
    case SCtrlCheckMsg::EPRO_CHECK_NAME:
        {
            SACheckNameMsg * pCheckNameMsg;
            pCheckNameMsg = (SACheckNameMsg *)pMsg;
            //switch(pCheckNameMsg->byType)
            {
            //case 0:
                {
                    SAAddFriendMsg msg;
                    msg.byResult = pCheckNameMsg->byResult ;
                    memcpy(msg.szName,pCheckNameMsg->szName,11);
					msg.byType = pCheckNameMsg->byType;
                    if(msg.byResult == 1)
                    {
                        CPlayer * pPlayer = (CPlayer*)GetPlayerByDnid(pCheckNameMsg->dnidClient)->DynamicCast(IID_PLAYER);
                        if(pPlayer != NULL)
                        {
                            pPlayer->AddRelation(pCheckNameMsg->szName,pCheckNameMsg->byType,FALSE);
                        }
                    }
                    else
                        g_StoreMessage(pCheckNameMsg->dnidClient,&msg,sizeof(SAAddFriendMsg));
                }
            }
        }
    }
    
}
void CNetORBServerModule::OnDispatchCtrlChatMsg(SORBChatBaseMsg * pMsg)
{
    switch(pMsg->_protocol)
    {
    case SORBChatBaseMsg::EPRO_CHAT_GLOBAL:
        {
            OnRecvTalkGlobalMsg((SAORBChatGlobalMsg*)pMsg);
        }
        break;
    case SORBChatBaseMsg::EPRO_CHAT_WISPER:
        {
            CPlayer * pPlayer;
            SAORBChatWisperMsg * pAORBChatWisperMsg;
            pAORBChatWisperMsg = (SAORBChatWisperMsg *)pMsg;
            if (pAORBChatWisperMsg->dnidRecv == 0) 
                break;

            pPlayer = (CPlayer*)GetPlayerByDnid(pAORBChatWisperMsg->dnidRecv)->DynamicCast(IID_PLAYER);
            if (pPlayer == NULL) 
            {
                pPlayer = (CPlayer*)GetPlayerByName(pAORBChatWisperMsg->cNameII)->DynamicCast(IID_PLAYER);

                if (pPlayer == NULL) 
                    break;
            }

            if (pPlayer->m_OnlineState == CPlayer::OST_HANGUP)
            {
                void TalkToPlayer(LPCSTR, LPCSTR);
                TalkToPlayer(pAORBChatWisperMsg->cName, "�Է������ڹһ�״̬");
                break;
            }

            if (pAORBChatWisperMsg->wRecvServerID != GetServerID()) 
                break;

            SAChatWisperMsg AChatWisperMsg;
            dwt::strcpy(AChatWisperMsg.szName, pAORBChatWisperMsg->cName, 11);
            dwt::strcpy(AChatWisperMsg.cChatData, pAORBChatWisperMsg->cChatData, MAX_CHAT_LEN);
            AChatWisperMsg.byType = pAORBChatWisperMsg->byType;

            g_StoreMessage(pAORBChatWisperMsg->dnidRecv, &AChatWisperMsg, sizeof(SAChatWisperMsg));
        }
        break;
    //case SORBChatBaseMsg::EPRO_CHAT_FACTION:
    //    {
    //        SAORBChatFactionMsg * pAChatMsg;
    //        pAChatMsg = (SAORBChatFactionMsg *)pMsg;
    //        CFaction * pFaction;
    //        pFaction = GetGW()->m_FactionManager.GetFactionByFName(pAChatMsg->cFactionName);
    //        if(pFaction != NULL)
    //        {
    //            SAChatFactionMsg msg; 
    //            memcpy(msg.cTalkerName,pAChatMsg->cTalkerName,11);
    //            memcpy(msg.cChatData,pAChatMsg->cChatData,MAX_CHAT_LEN);
    //            msg.wResult = 1;
    //            pFaction->SendMessage(&msg,sizeof(SAChatFactionMsg),INVALID_DNID);
    //        }
    //    }
        break;
    }
}

void CNetORBServerModule::OnDispatchCtrlSimplayerMsg(SCtrlSimPlayer *pMsg)
{
    // Ŀǰorb����û�й��ڻ�����Ҷ���Ĳ�����Ӧ
    switch (pMsg->_protocol)
    {
    case SCtrlSimPlayer::EPRO_DEL_SIMPLAYER:
        break;

    case SCtrlSimPlayer::EPRO_SYN_SIMPLAYER:
        break;

	//case SCtrlSimPlayer::EPRO_QUERY_SIMPLAYER:
	//	CPlayer *pDestPlayer;
	//	pDestPlayer = (CPlayer *)GetPlayerByName(((SQQuerySimPlayerMsg*)pMsg)->szName)->DynamicCast(IID_PLAYER);
	//	if (pDestPlayer == NULL)
	//	{
	//		SAQuerySimPlayerMsg msg;
	//		SendToORBServer(&msg, sizeof(SAQuerySimPlayerMsg));
	//	}

	//	break;
    }
}

//BOOL IsPlayerInView(CPlayer *pPlayerCore, CPlayer *pPlayer)
//{
//    if (pPlayer == NULL) 
//        return FALSE;
//
//    if (pPlayerCore == NULL) 
//        return FALSE;
//
//    if (pPlayerCore->m_Property.m_wCurRegionID != pPlayer->m_Property.m_wCurRegionID)
//        return FALSE;
//
//    if ((abs((int)pPlayer->m_wCurX - (int)pPlayerCore->m_wCurX) <= 6) && 
//        (abs((int)pPlayer->m_wCurY - (int)pPlayerCore->m_wCurY) <= 8))
//        return TRUE;
//
//    return FALSE;
//}

BOOL IsPlayerInRange(CPlayer *pPlayerCore, CPlayer *pPlayer, int Range)
{
    if (pPlayer == NULL) 
        return FALSE;

    if (pPlayerCore == NULL) 
        return FALSE;

    if (pPlayerCore->m_Property.m_wCurRegionID != pPlayer->m_Property.m_wCurRegionID)
        return FALSE;

    if ((abs((int)pPlayer->m_wCurX - (int)pPlayerCore->m_wCurX) <= Range) && 
        (abs((int)pPlayer->m_wCurY - (int)pPlayerCore->m_wCurY) <= Range))
        return TRUE;

    return FALSE;
}

BOOL SendORBDeletePlayerMessage(CPlayer *pPlayer)
{
    SQDelSimPlayerMsg msg;

    pPlayer->m_dwTeamID = 0;

    msg.dwClientGID = pPlayer->GetGID();
    msg.dwStaticID = pPlayer->m_Property.m_dwStaticID;
    msg.dnidClient = pPlayer->m_ClientIndex;
    msg.wServerID = GetServerID();

    return SendToORBServer(&msg, sizeof(SQDelSimPlayerMsg));
}

BOOL SendORBRefreshPlayerMessage(CPlayer *pPlayer)
{
    SQSynSimPlayerMsg msg;

    msg.dwClientGID = pPlayer->GetGID();
    msg.dwStaticID = pPlayer->m_Property.m_dwStaticID;
    msg.wCurRegionID = pPlayer->m_Property.m_wCurRegionID;
    msg.dnidClient = pPlayer->m_ClientIndex;
    msg.wServerID = GetServerID();
    msg.dwGMLevel = pPlayer->m_Property.m_wGMLevel;
    
    memcpy(msg.szName, pPlayer->m_Property.m_szName, 11);

	msg.bOnlineState = (pPlayer->m_OnlineState == CPlayer::OST_HANGUP) ? 2 : 
	((pPlayer->m_OnlineState == CPlayer::OST_NOT_INIT) ? 0 : 1);

    return SendToORBServer(&msg, sizeof(SQSynSimPlayerMsg));
}

void CNetORBServerModule::OnRecvTalkGlobalMsg(SAORBChatGlobalMsg *pMsg)
{
    SAChatGlobalMsg AChatGlobalMsg;
    AChatGlobalMsg.byType = pMsg->byType;
    dwt::strcpy(AChatGlobalMsg.cChatData, pMsg->cChatData, MAX_CHAT_LEN);
    memcpy(AChatGlobalMsg.cTalkerName, pMsg->cTalkerName, 11);

    BroadcastMsg(&AChatGlobalMsg, AChatGlobalMsg.GetMySize());

    //void BroadCastGlobalTalk(DNID dnidClient, SAChatGlobalMsg *pMsg, int iSize);
    //BroadCastGlobalTalk(-1, &AChatGlobalMsg, sizeof(SAChatGlobalMsg));
}
