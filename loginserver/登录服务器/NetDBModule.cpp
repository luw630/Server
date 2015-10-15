
// ***************************************************************
//  NetDBModule   version:  1.0   ·  date: 07/18/2006
//  -------------------------------------------------------------
//  Purpose: 处理DB收到Login的消息后的返回消息
//  History:
//  1、2006-07-18 修改留言系统
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

    // 重连
    // 3分钟重连一次
    static DWORD prev = timeGetTime();
    if ((int)(timeGetTime()-prev) > 1000*20)
    {
        prev = timeGetTime();

        if ( !client.IsConnected() )
        {
            if ( !Connect( dbServerIP.c_str(), dbServerPort.c_str() ) )
            {
                rfalse(2, 1, "数据库服务器连不上啊连不上-____-bb");
                rfalse(1, 1, "数据库服务器连不上啊连不上-____-bb");
            }
            else
            {
                rfalse(2, 1, "数据库服务器重新连上咯^^");
                rfalse(1, 1, "数据库服务器重新连上咯^^");
            }
        }
    }

    return 1;
}

bool CNetDBModule::Entry( LPCVOID data, size_t size )
{
    if (data == NULL)
        // 如果检测到服务器关闭，那么就会进入到这里
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
            Globals::SendToAllGameServer( data, size ); // 帮派读取消息转发到世界服务器
			break;
			//wk 20150114 三国邮件系统dbBack
		case _SDataBaseMsg::EPRO_DB_Mail_SANGUO:
			//RecvGetFactionMsg( (SAGetFactionMsg *)data);
			Globals::SendToAllGameServer(data, size); // 三国邮件系统dbBack转发到世界服务器
			break;
			//wk 20150114 三国邮件系统dbBack
// 		case _SDataBaseMsg::EPRO_SAVEPLAYER_CONFIG://保存玩家PC配置
// 			RecvSavePlayerConfigMsg((SQSavePlayerConfig *)data);	
// 			break;

		// 这里接受来自DB的排行榜信息
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
				// 过来的RPCOP都一定是区域服务器过来的请求，所以直接回传！
				Globals::SendToGameServer( tempMsg->dnidClient, const_cast< void* >( data ), size );
			//else
			//	 // 目前没有登陆服务器直接操作数据库的RPCOP，所以没有回传解析
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
				// 回传建筑物数据到原请求区域服务器上！
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

	// 这里获取信息不应该失败，除非是掉线了，但是掉线的情况上面已经判断了
	// 所以这里不需要再去做过多的处理，如果真的失败了，直接return即可
	LPCSTR account = NULL, password = NULL, userpass = NULL;
    if (!Globals::GetDnidContext(pMsg->dnidClient, account, password, userpass))
        return;

	if (pMsg->byResult == SAGetCharacterListMsg::RET_LOCKEDSELF)
    {
		// 说明当前玩家有可能还存在于当前服务器上
		if (Globals::AccountExist(account))
        {
			// 重新绑定该角色，如果失败，只能说明游戏内部逻辑出现问题了
            Globals::RebindAccount(pMsg->dnidClient, account, 0, 0, 0, 0);
        }
        else
        {
			// 如果被认为该角色已经被当前服务器锁定，但又找不到这个角色，那么向服务器请求取消serverId锁定
            QueryPlayerList(account, NULL, 0);

            // 让玩家再次登陆
			SALoginMsg msg;
            msg.dwRetCode = SALoginMsg::ERC_GETLIST_FAIL;
            Globals::SendToPlayer(pMsg->dnidClient, &msg, sizeof(msg));
            Globals::CutPlayerLink(pMsg->dnidClient);
        }
		return;
    }
	else if (pMsg->byResult != SAGetCharacterListMsg::RET_SUCCESS)
    {
		// 因为获取角色列表失败，则说明内部出现问题（数据库操作失败，角色对应的serverId被其他服务器锁定）
		// 所以这种状况下断开联接不需要向服务器请求解锁，所以注销这个联接context
        Globals::SetPlayerLinkContext(pMsg->dnidClient, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0);

        SALoginMsg msg;
        msg.dwRetCode = ((pMsg->byResult == SAGetCharacterListMsg::RET_LOCKEDOTHER) ? SALoginMsg::ERC_LOCKEDON_SERVERID : SALoginMsg::ERC_GETLIST_FAIL);
        Globals::SendToPlayer(pMsg->dnidClient, &msg, sizeof(msg));
        Globals::CutPlayerLink(pMsg->dnidClient);
        return;
    }
	/*
	// 向客户端返回角色列表
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

    // 打上标记，在连接断开时需要取消数据库绑定
	Globals::ActivateLinkContextWithRebind(pMsg->dnidClient);

    // 设定该连接最多在登陆服务器停留3分钟，现在改为停留10分钟
    Globals::SetPlayerLinkForceTimeOutTime(pMsg->dnidClient, 1000 * 60 * 10);

    // 到了这里以后，认为该连接已经通过认证，改变连接状态已允许接收其数据
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
		rfalse(2, 1, "获取玩家的数据失败，返回失败给客户端！！！");

		// 这里没有切断网络连接，玩家可以不断尝试，但有3分钟的超时！
		msg.dwRetCode = SASelPlayerMsg::ERC_GETCHDATA_FAIL;
		Globals::SendToPlayer(pMsg->dnidClient, &msg, sizeof(msg));
        return;
    }

	//发送到DB记录玩家的配置
	 LPCSTR account = NULL;
	lite::Serialreader sl(pMsg->streamData);
	account = sl();
	if (GetApp()->m_PlayerManager.SendPlayerConfig(account)){
		GetApp()->m_PlayerManager.CleanPlayerConfig(account);
	}
	 
	// 如果获取到的角色的名字存在问题（没有名字，为空格，或非法字符），则给出警告
	// 侠义道3：此处判断过于简单，待以后完善
// 	if ((BYTE)pMsg->PlayerData.m_Name[0] <= 32)
// 	{
// 		TraceInfo("special.log", "%s 在向数据库获取角色数据时，发现了没有名称的角色！[sid=%d]", Globals::GetStringTime(), pMsg->PlayerData.m_dwStaticID);
// 	}

	// 将该角色注册到缓存中，同时将角色放入场景
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

// 处理从DBServer回应的GetAccountByName的消息
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

// 处理来自DBServer的排行榜信息
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