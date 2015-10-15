
// ***************************************************************
//  NetDBModule   version:  1.0   ��  date: 07/18/2006
//  -------------------------------------------------------------
//  Purpose: ����DB�յ�Login����Ϣ��ķ�����Ϣ
//  History:
//  1��2006-07-18 �޸�����ϵͳ
//  -------------------------------------------------------------
//  Copyright (C) 2006 - All Rights Reserved
// ***************************************************************
	
#include "StdAfx.h"
#include "netdbmodule.h"
#include "networkmodule/logmsgs.h"
#include "networkmodule/dataMsgs.h"
#include "networkmodule/accountMsgs.h"
#include "boost/bind.hpp"
#include "new_net\netserverex.h"
#include "liteserializer/lite.h"
#include "networkmodule/MailMsg.h"
#include "networkmodule/FactionBBSMsgs.h"
//#include "networkmodule/DbopMsgs.h"
#include "networkmodule/RefreshMsgs.h"
#include "pub/traceinfo.h"
#include "DMainApp.h"
#include "networkmodule/buildingtypedef.h"
#include "networkmodule/unionmsgs.h"
#include "networkmodule/SectionMsgs.h"
#include "networkmodule/ScriptMsgs.h"

extern BOOL QueryPlayerList( LPCSTR szAccount, LPCSTR szPassword, DNID dnidClient );
extern DWORD g_dConcurrencyDbPlayerNum;

CNetDBModule::CNetDBModule(void)
{
}

CNetDBModule::~CNetDBModule(void)
{
}

void CNetDBModule::Destroy()
{
}

bool CNetDBModule::Connect( LPCSTR ip, LPCSTR port )
{
	if ( !client.Connect( ip, port ) )
        return false;

	if ( 0 == dbServerIP.length() ) 
  	    dbServerIP = ip, dbServerPort = port;

    SQRegisterLogsrvMsg msg;
    extern int clusterMax, clusterId;
    msg.IP = MAKELONG( clusterMax, clusterId );
    msg.ID = Globals::GetServerId();

    return Globals::SendToDBServer( &msg, sizeof(msg) ) == TRUE;
}

int CNetDBModule::Execution()
{
	client.Execution(boost::bind(&CNetDBModule::Entry, this, _1, _2));

    // ����
    // 3��������һ��
    static DWORD prev = timeGetTime();
    if ((int)(timeGetTime()-prev) > 1000*20)
    {
        prev = timeGetTime();

        if ( !client.IsConnected() )
        {
            if ( !Connect( dbServerIP.c_str(), dbServerPort.c_str() ) )
            {
                rfalse(2, 1, "���ݿ�����������ϰ�������-____-bb");
                rfalse(1, 1, "���ݿ�����������ϰ�������-____-bb");
            }
            else
            {
                rfalse(2, 1, "���ݿ�������������Ͽ�^^");
                rfalse(1, 1, "���ݿ�������������Ͽ�^^");
            }
        }
    }

    return 1;
}

bool CNetDBModule::Entry( LPCVOID data, size_t size )
{
    if (data == NULL)
        // �����⵽�������رգ���ô�ͻ���뵽����
        return 0;

    if ( ((SMessage *)data )->_protocol == SMessage::EPRO_DATABASE_MESSAGE )
    {
        switch ( ((_SDataBaseMsg *)data )->_protocol ) 
        {
        case _SDataBaseMsg::EPRO_GM_UPDATEPLAYER:
            RecvUpdatePlayerMsg ( (SAUpdatePlayerMsg *)data );
            break;
        case _SDataBaseMsg::EPRO_GET_CHARACTER_LIST:
            RecvGetCharacterListMsg( (SAGetCharacterListMsg*)data );
            break;
        case _SDataBaseMsg::EPRO_CREATE_CHARACTER:
            RecvCreateCharacterMsg( (SACreatePlayerMsg *)data );
            break;
        case _SDataBaseMsg::EPRO_DELETE_CHARACTER:
            RecvDelCharacterMsg( (SADeleteCharacterMsg*)data );
            break;
        case _SDataBaseMsg::EPRO_GET_CHARACTER:
            RecvGetCharacterMsg( (SAGetCharacterMsg*)data );
            break;
        case _SDataBaseMsg::EPRO_GM_CHECKPLAYER:
            RecvCheckCharacterMsg( (SACheckPlayerDBMsg*)data );
            break;
        case _SDataBaseMsg::EPRO_GM_CHECKWAREHOUSE:
            RecvCheckWarehouseMsg( (SACheckWarehouseMsg*)data );
            break;
        case _SDataBaseMsg::EPRO_GET_ACCOUNTBYNAME:
            RecvGetAccountByNameMsg( (SAGetAccountByName *)data );
            break;
		case _SDataBaseMsg::EPRO_GET_FACTION:
		case _SDataBaseMsg::EPRO_DELETE_FACTION:
		case  _SDataBaseMsg::EPRO_GET_SCRIPTDATA:
			//RecvGetFactionMsg( (SAGetFactionMsg *)data);
            Globals::SendToAllGameServer( data, size ); // ���ɶ�ȡ��Ϣת�������������
			break;
			//wk 20150114 �����ʼ�ϵͳdbBack
		case _SDataBaseMsg::EPRO_DB_Mail_SANGUO:
			//RecvGetFactionMsg( (SAGetFactionMsg *)data);
			Globals::SendToAllGameServer(data, size); // �����ʼ�ϵͳdbBackת�������������
			break;
			//wk 20150114 �����ʼ�ϵͳdbBack
// 		case _SDataBaseMsg::EPRO_SAVEPLAYER_CONFIG://�������PC����
// 			RecvSavePlayerConfigMsg((SQSavePlayerConfig *)data);	
// 			break;

		// �����������DB�����а���Ϣ
		case _SDataBaseMsg::EPRO_DBGET_RankList:
			RecvGetRankListMsg((SADBGETRankList *)data);
			break;
        }
    }
	else if (((SMessage *)data )->_protocol == SMessage::ERPO_SECTION_MESSAGE)
	{
		SMessage* srcMsg = SectionMessageManager::getInstance().recvMessageWithSection((SSectionMsg*)data);
		if (srcMsg)
		{
			Entry((LPCVOID)srcMsg,((SSectionMsg*)data)->byCount*SSectionMsg::MAX_SIZE);
			SectionMessageManager::getInstance().popMessage(srcMsg);
		}
	}
    else
    {
		if ( ( ( ( SMessage * )data )->_protocol == SMessage::EPRO_SERVER_CONTROL ) && 
			( ( ( SServerCtrlMsg * )data )->_protocol == SServerCtrlMsg::EPRO_GAMESERVER_REQUEST_RPCOP ) )
		{
			SAGameServerRPCOPMsg *tempMsg = ( SAGameServerRPCOPMsg * )data;
            tempMsg->srcDatabase = SAGameServerRPCOPMsg::CHARACTER_DATABASE;
			if ( tempMsg->dnidClient != INVALID_DNID )
				// ������RPCOP��һ���������������������������ֱ�ӻش���
				Globals::SendToGameServer( tempMsg->dnidClient, const_cast< void* >( data ), size );
			//else
			//	 // Ŀǰû�е�½������ֱ�Ӳ������ݿ��RPCOP������û�лش�����
			//	 ProcessInterop( ( (SDbopMsg *)pMsg )->data, ( (SDbopMsg *)pMsg )->size );
		}
		else if(((SMessage *)data )->_protocol == SMessage::EPRO_TONG_MESSAGE)
		{
			STongBaseMsg *pFacMsg = (STongBaseMsg*)data;
			if( pFacMsg->_protocol == STongBaseMsg::EPRO_FACTIONBBS_MESSAGE )
			{
				SFactionBBSMsg *pFacBBSMsg = (SFactionBBSMsg*)pFacMsg;
				switch(	pFacBBSMsg->_protocol )
				{
				case SFactionBBSMsg::EPRO_FACTIONBBS_GET_TEXT:
					{
						SAGetFacBBSTextMsg *pGetBBSTextMsg = (SAGetFacBBSTextMsg*)pFacBBSMsg;
						Globals::SendToAllGameServer( pGetBBSTextMsg, sizeof( SAGetFacBBSTextMsg ) - pGetBBSTextMsg->nFreeSize );
					}
					break;
				case SFactionBBSMsg::EPRO_FACTIONBBS_GET:
					{
						SAGetFacBBSMsg *pGetBBSMsg = (SAGetFacBBSMsg*)pFacBBSMsg;
						Globals::SendToAllGameServer( pGetBBSMsg, sizeof( SAGetFacBBSMsg ) - sizeof( pGetBBSMsg->stFactionBBS[0] ) * ( MAX_BBSLIST - pGetBBSMsg->byBBSNum ) );
					}
					break;
				}
			}
		}
        else if(((SMessage *)data )->_protocol == SMessage::EPRO_MAIL_MESSAGE)
        {
            SMailBaseMsg *pMailBaseMsg = (SMailBaseMsg*)data;
            switch (pMailBaseMsg->_protocol)
            {
            case SMailBaseMsg::EPRO_MAIL_SEND:
                {
                    SAMailSendMsg *pASendMsg = (SAMailSendMsg*)data;
                    Globals::SendToAllGameServer(pASendMsg,sizeof(SAMailSendMsg));
                }
                break;
            case SMailBaseMsg::EPRO_MAIL_RECV:
                {
                    SAMailRecvMsg *pARecvMsg = (SAMailRecvMsg*)data;
                    Globals::SendToAllGameServer(pARecvMsg,sizeof(SAMailRecvMsg));
                }
                break;
            case SMailBaseMsg::EPRO_MAIL_DELETE:
                {
                    SAMailDeleteMsg *pADeleteMsg = (SAMailDeleteMsg*)data;
                    Globals::SendToAllGameServer(pADeleteMsg,sizeof(SAMailDeleteMsg));
                }
                break;
            case SMailBaseMsg::EPRO_MAIL_NEWMAIL:
                {
                    SANewMailMsg *pANewMail = (SANewMailMsg*)data;
                    Globals::SendToAllGameServer( pANewMail, sizeof(SANewMailMsg) );
                }
                break;
            }
        }
        else if ( ( ( SMessage* )data )->_protocol == SMessage::EPRO_UNION_MESSAGE )
        {
            if ( ( ( SUnionBaseMsg* )data )->_protocol == SUnionBaseMsg::EPRO_GET_UNIONDATA )
                Globals::SendToGameServer( ( ( SAGetUnionDataMsg* )data )->dwZoneID,
				data, ( ( SAGetUnionDataMsg* )data )->dwSendSize + ( sizeof( SAGetUnionDataMsg ) - sizeof( ( ( SAGetUnionDataMsg* )data )->streamData ) ) );
        }
        else
		{
			if ( ( ( SMessage* )data )->_protocol == SMessage::EPRO_BUILDING_MESSAGE )
			{
				// �ش����������ݵ�ԭ��������������ϣ�
				if ( ( ( SBuildingBaseMsg* )data )->_protocol == SBuildingBaseMsg::EPRO_BUILDING_GET )
                    Globals::SendToGameServer( ( ( SABuildingGetMsg* )data )->dwZoneID, 
                    data, ( ( SABuildingGetMsg* )data )->WBufSize );
			}
		}
    }

    return true;
}

void CNetDBModule::RecvCreateCharacterMsg(SACreatePlayerMsg *pMsg)
{
    SACrePlayerMsg msg;
    msg.byResult = pMsg->byResult;
    msg.byIndex  = pMsg->m_byIndex;

	memcpy(msg.PlayerData.m_szName, pMsg->PlayerData.m_szName, sizeof(pMsg->PlayerData.m_szName));
	//memcpy(msg.PlayerData.m_w3DEquipID, pMsg->PlayerData.m_w3DEquipID, sizeof(msg.PlayerData.m_w3DEquipID));
	msg.PlayerData.m_bySex		= pMsg->PlayerData.m_bySex;
	msg.PlayerData.m_bySchool	= pMsg->PlayerData.m_bySchool;
	msg.PlayerData.m_byBRON		= pMsg->PlayerData.m_byBRON;
	msg.PlayerData.m_FaceID		= pMsg->PlayerData.m_FaceID;
	msg.PlayerData.m_HairID		= pMsg->PlayerData.m_HairID;
	msg.PlayerData.m_dwStaticID	= pMsg->PlayerData.m_dwStaticID;

    Globals::SendToPlayer(pMsg->dnidClient, &msg, sizeof(msg));
}

void CNetDBModule::RecvDelCharacterMsg( SADeleteCharacterMsg *pMsg )
{
    SADelPlayerMsg msg;
    msg.byResult	= pMsg->byResult;
    msg.dwStaticID	= pMsg->dwStaticID;
    Globals::SendToPlayer(pMsg->dnidClient, &msg, sizeof(msg));
}

LPSTR g_AnsiToUtf8(const char* AnsiStr)
{
	if (AnsiStr == NULL)
		return NULL;
	wchar_t* pBuf = NULL;
	int WriteByte = 0;
	WriteByte = ::MultiByteToWideChar(CP_ACP, 0, AnsiStr, -1, NULL, 0);
	pBuf = new wchar_t[WriteByte + 1];
	memset(pBuf, 0, (WriteByte + 1)*sizeof(wchar_t));
	::MultiByteToWideChar(CP_ACP, 0, AnsiStr, -1, (LPWSTR)pBuf, WriteByte + 1);

	char* Utf8Str = NULL;
	int Utf8StrByte = 0;
	Utf8StrByte = ::WideCharToMultiByte(CP_UTF8, 0, pBuf, -1, NULL, 0, 0, 0);
	Utf8Str = new char[Utf8StrByte + 1];
	::WideCharToMultiByte(CP_UTF8, 0, pBuf, WriteByte + 1, Utf8Str, Utf8StrByte + 1, 0, 0);
	if (pBuf != NULL)
		delete[] pBuf;
	return Utf8Str;
}

void CNetDBModule::RecvGetCharacterListMsg(SAGetCharacterListMsg *pMsg)
{
    if (!Globals::IsValidPlayerDnid(pMsg->dnidClient))
        return;

	// �����ȡ��Ϣ��Ӧ��ʧ�ܣ������ǵ����ˣ����ǵ��ߵ���������Ѿ��ж���
	// �������ﲻ��Ҫ��ȥ������Ĵ���������ʧ���ˣ�ֱ��return����
	LPCSTR account = NULL, password = NULL, userpass = NULL;
    if (!Globals::GetDnidContext(pMsg->dnidClient, account, password, userpass))
        return;

	if (pMsg->byResult == SAGetCharacterListMsg::RET_LOCKEDSELF)
    {
		// ˵����ǰ����п��ܻ������ڵ�ǰ��������
		if (Globals::AccountExist(account))
        {
			// ���°󶨸ý�ɫ�����ʧ�ܣ�ֻ��˵����Ϸ�ڲ��߼�����������
            Globals::RebindAccount(pMsg->dnidClient, account, 0, 0, 0, 0);
        }
        else
        {
			// �������Ϊ�ý�ɫ�Ѿ�����ǰ�����������������Ҳ��������ɫ����ô�����������ȡ��serverId����
            QueryPlayerList(account, NULL, 0);

            // ������ٴε�½
			SALoginMsg msg;
            msg.dwRetCode = SALoginMsg::ERC_GETLIST_FAIL;
            Globals::SendToPlayer(pMsg->dnidClient, &msg, sizeof(msg));
            Globals::CutPlayerLink(pMsg->dnidClient);
        }
		return;
    }
	else if (pMsg->byResult != SAGetCharacterListMsg::RET_SUCCESS)
    {
		// ��Ϊ��ȡ��ɫ�б�ʧ�ܣ���˵���ڲ��������⣨���ݿ����ʧ�ܣ���ɫ��Ӧ��serverId������������������
		// ��������״���¶Ͽ����Ӳ���Ҫ��������������������ע���������context
        Globals::SetPlayerLinkContext(pMsg->dnidClient, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0);

        SALoginMsg msg;
        msg.dwRetCode = ((pMsg->byResult == SAGetCharacterListMsg::RET_LOCKEDOTHER) ? SALoginMsg::ERC_LOCKEDON_SERVERID : SALoginMsg::ERC_GETLIST_FAIL);
        Globals::SendToPlayer(pMsg->dnidClient, &msg, sizeof(msg));
        Globals::CutPlayerLink(pMsg->dnidClient);
        return;
    }
	/*
	// ��ͻ��˷��ؽ�ɫ�б�
    SAChListMsg msg;
    memcpy(msg.CharListData, pMsg->CharListData, sizeof(msg.CharListData));
	//for (int i = 0; i < 5; i++)
	//{
	//	char TempName[CONST_USERNAME] = { 0 };
	//	memcpy_s(TempName, CONST_USERNAME, msg.CharListData[i].m_szName, CONST_USERNAME);
	//	memset(msg.CharListData[i].m_szName, 0, CONST_USERNAME);
	//	char* Utf8Name = g_AnsiToUtf8(TempName);
	//	memcpy_s(msg.CharListData[i].m_szName, CONST_USERNAME, Utf8Name, strlen(Utf8Name));
	//	if (Utf8Name != NULL)
	//		delete[] Utf8Name;
	//}
    Globals::SendToPlayer(pMsg->dnidClient, &msg, sizeof(msg));
	*/

    // ���ϱ�ǣ������ӶϿ�ʱ��Ҫȡ�����ݿ��
	Globals::ActivateLinkContextWithRebind(pMsg->dnidClient);

    // �趨����������ڵ�½������ͣ��3���ӣ����ڸ�Ϊͣ��10����
    Globals::SetPlayerLinkForceTimeOutTime(pMsg->dnidClient, 1000 * 60 * 10);

    // ���������Ժ���Ϊ�������Ѿ�ͨ����֤���ı�����״̬���������������
    Globals::SetPlayerLinkStatus(pMsg->dnidClient, LinkStatus_Connected);

	Globals::GetPlayData(pMsg->dnidClient);
}

void CNetDBModule::RecvGetCharacterMsg(SAGetCharacterMsg *pMsg)
{
	if (!Globals::IsValidPlayerDnid(pMsg->dnidClient))
        return;

	SASelPlayerMsg msg;
	
	if (pMsg->byResult != 1 || pMsg->PlayerData.GetVersion() != pMsg->PlayerData.m_version)
    {
		rfalse(2, 1, "��ȡ��ҵ�����ʧ�ܣ�����ʧ�ܸ��ͻ��ˣ�����");

		// ����û���ж��������ӣ���ҿ��Բ��ϳ��ԣ�����3���ӵĳ�ʱ��
		msg.dwRetCode = SASelPlayerMsg::ERC_GETCHDATA_FAIL;
		Globals::SendToPlayer(pMsg->dnidClient, &msg, sizeof(msg));
        return;
    }

	//���͵�DB��¼��ҵ�����
	 LPCSTR account = NULL;
	lite::Serialreader sl(pMsg->streamData);
	account = sl();
	if (GetApp()->m_PlayerManager.SendPlayerConfig(account)){
		GetApp()->m_PlayerManager.CleanPlayerConfig(account);
	}
	 
	// �����ȡ���Ľ�ɫ�����ִ������⣨û�����֣�Ϊ�ո񣬻�Ƿ��ַ��������������
	// �����3���˴��жϹ��ڼ򵥣����Ժ�����
// 	if ((BYTE)pMsg->PlayerData.m_Name[0] <= 32)
// 	{
// 		TraceInfo("special.log", "%s �������ݿ��ȡ��ɫ����ʱ��������û�����ƵĽ�ɫ��[sid=%d]", Globals::GetStringTime(), pMsg->PlayerData.m_dwStaticID);
// 	}

	// ���ý�ɫע�ᵽ�����У�ͬʱ����ɫ���볡��
	if (!Globals::AssignPlayer(pMsg->dnidClient, &pMsg->PlayerData))
	{
		msg.dwRetCode = SASelPlayerMsg::ERC_PUTTOREGION_FAIL;
		Globals::SendToPlayer(pMsg->dnidClient, &msg, sizeof(msg));
		return;
	}
// 	if (g_dConcurrencyDbPlayerNum > 0)
// 	{
// 		g_dConcurrencyDbPlayerNum--;
// 	}

}

void CNetDBModule::RecvUpdatePlayerMsg (SAUpdatePlayerMsg * pMsg)
{
    //SAUpdateSBInfoMsg AUpdateSBInfoMsg;

    //dwt::strcpy(AUpdateSBInfoMsg.szName,pMsg->szName,CONST_USERNAME);
    //AUpdateSBInfoMsg.wResult = pMsg->byResult;

    //GetApp()->m_NetGMModule.SendGMMessage(pMsg->dnidClient,&AUpdateSBInfoMsg,sizeof(SAUpdateSBInfoMsg));
}

void CNetDBModule::RecvCheckCharacterMsg  (SACheckPlayerDBMsg *pMsg)
{
    //SACheckPlayerDatabaseMsg ACheckPlayerDatabaseMsg;
    //ACheckPlayerDatabaseMsg.wResult = pMsg->wResult ;
    //ACheckPlayerDatabaseMsg.dwStaticID = pMsg->dwStaticID;
    //dwt::strcpy(ACheckPlayerDatabaseMsg.szAccount,pMsg->szAccount,ACCOUNTSIZE);
    //memcpy(&(ACheckPlayerDatabaseMsg.PlayerData),&(pMsg->PlayerData),sizeof(SFixBaseData));
    //memcpy(&(ACheckPlayerDatabaseMsg.PlayerPackage),&(pMsg->PlayerPackage),sizeof(SFixPackage));

    //if(pMsg->wResult == 1)
    //    GetApp()->GetIPbyName(ACheckPlayerDatabaseMsg.PlayerData.m_szName,ACheckPlayerDatabaseMsg.szIP);

    //GetApp()->m_NetGMModule.SendGMMessage(pMsg->dnidClient,&ACheckPlayerDatabaseMsg,sizeof(SACheckPlayerDatabaseMsg));
}

void CNetDBModule::RecvCheckWarehouseMsg (SACheckWarehouseMsg *pMsg)
{
    //SACheckWareHouseMsg ACheckWareHouseMsg;
    //ACheckWareHouseMsg.wResult = pMsg->wResult;
    //dwt::strcpy(ACheckWareHouseMsg.szAccount,pMsg->szAccount,ACCOUNTSIZE);

    //memcpy(&ACheckWareHouseMsg.WarehouseI,&pMsg->WarehouseI,sizeof(SFixStorage1));
    //memcpy(&ACheckWareHouseMsg.WarehouseII,&pMsg->WarehouseII,sizeof(SFixStorage2));
    //memcpy(&ACheckWareHouseMsg.WarehouseIII,&pMsg->WarehouseIII,sizeof(SFixStorage3));

    //GetApp()->m_NetGMModule.SendGMMessage(pMsg->dnidClient,&ACheckWareHouseMsg,sizeof(SACheckWareHouseMsg));
}

// �����DBServer��Ӧ��GetAccountByName����Ϣ
void CNetDBModule::RecvGetAccountByNameMsg(SAGetAccountByName *pMsg)
{
	//GetApp()->NotifyGetAccountByName(pMsg);
}

// void CNetDBModule::RecvSavePlayerConfigMsg( struct SQSavePlayerConfig *pMsg )
// {
// 	SQLSavePlayerConfig psavemsg;
// 	psavemsg.serverId = Globals::GetServerId();
// 	dwt::strcpy(psavemsg.streamData,pMsg->streamData,MAX_ACCOUNT);
// 	dwt::strcpy(psavemsg.playerconfig,pMsg->playerconfig,1024);
// 
// 	Globals::SendToDBServer(&psavemsg,sizeof(SQLSavePlayerConfig));
// }

// ��������DBServer�����а���Ϣ
void CNetDBModule::RecvGetRankListMsg(SADBGETRankList *pMsg)
{
	SAGetRankListFromDB msg;

	msg.m_Num = pMsg->m_Num;
	memcpy(msg.m_NewRankList, pMsg->m_NewRankList, sizeof(NewRankList) * ALL_RANK_NUM);	
	
 	int num = SectionMessageManager::getInstance().evaluateDevidedAmount(sizeof(msg));
 	int id = 0;
 	for (int i = 0;i < num;i++)
 	{
 		SSectionMsg sMsg;
 		id = SectionMessageManager::getInstance().devideMessage(i, num, &sMsg, &msg, sizeof(msg), id);
 
		GetApp()->m_ServerManager.server.SendMessage(pMsg->serverID,&sMsg,sizeof(SSectionMsg));
 	}

	//GetApp()->m_ServerManager.server.SendMessage(pMsg->serverID, &msg, sizeof(msg));

	return;
}