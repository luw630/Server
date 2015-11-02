#include "stdafx.h"
#include "GMServer.h"
#include <stdio.h>
#include <string.h>
#include "gameobjects\player.h"
#include "networkmodule/SectionMsgs.h"


extern LPIObject GetPlayerByName(LPCSTR);
extern LPIObject GetPlayerByGID(DWORD dwStaticID);
extern void AddInfo(LPCSTR Info);


SOCKET ServerSock;
HANDLE ListenEvent;
SFixProperty *g_lpCachePlayerBaseData = NULL;
DWORD g_PlayerID = -1;
int InitGMServer()
{
	WSADATA WsaData;
	WORD Version;
	SOCKADDR_IN ServerAddr;
	Version = MAKEWORD(2, 2);
	if (WSAStartup(Version, &WsaData) != 0)
	{
		//printf("初始化失败：%d\n", WSAGetLastError());
		return 0;
	}
	ServerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSock == INVALID_SOCKET)
	{
		//printf("创建监听套接字失败：%d\n", WSAGetLastError());
		WSACleanup();
		return 0;
	}

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(SERVERPORT);
	ServerAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	if (::bind(ServerSock, (LPSOCKADDR)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR)
	{
		//printf("绑定失败：%d\n", WSAGetLastError());
		WSACleanup();
		return 0;
	}

	if (listen(ServerSock, 100) == SOCKET_ERROR)
	{
		//printf("监听失败：%d\n", WSAGetLastError());
		WSACleanup();
		return 0;
	}

	ListenEvent = WSACreateEvent();
	WSAEventSelect(ServerSock, ListenEvent, FD_ACCEPT | FD_CLOSE);
	InitializeCriticalSection(&g_cs);
	InitializeCriticalSection(&g_DataCs);
	AddInfo("初始化GM服务器成功...\r\n");
	return 1;
}

void LoopGMServer()
{

	HandleDataBuf(OnDispatchMessage);
	int nRet = WaitForSingleObject(ListenEvent, 100);
	if (nRet == WAIT_FAILED)
	{
		//printf("WaitForSingleObject Failed!\n");
		//break;
	}
	else if (nRet == WSA_WAIT_TIMEOUT)	//定时显示连接数
	{
		//printf("当前套接字连接数：%d\n", g_nCurrentConnections);
	}
	else	//有新的连接请求
	{
		ResetEvent(ListenEvent);
		while (true)
		{
			SOCKADDR_IN ClientAddr;
			int AddrLen = sizeof(ClientAddr);
			char sAct[64];
			SOCKET AcceptSock = accept(ServerSock, (LPSOCKADDR)&ClientAddr, &AddrLen);
			if (AcceptSock == SOCKET_ERROR)
			{
				break;
			}
			sprintf(sAct, "IP地址(%s)连接了区域服的GM服务\n", inet_ntoa(ClientAddr.sin_addr));
			AddInfo(sAct);
			LPSOCKET_OBJ pSockObj = CreateSocketObj(AcceptSock);
			pSockObj->m_clientaddr = ClientAddr;
			WSAEventSelect(pSockObj->m_socket, pSockObj->m_event, FD_READ | FD_WRITE | FD_CLOSE);
			AssignToFreeThread(pSockObj);
		}
	}
}

void ReleaseGMServer()
{
	if (g_lpCachePlayerBaseData != NULL)
		delete g_lpCachePlayerBaseData;
	DeleteCriticalSection(&g_DataCs);
	DeleteCriticalSection(&g_cs);
	WSACleanup();
}

void OnDispatchMessage(LPSOCKET_OBJ lpSocketObj, BYTE *lpData, int DataSize)
{
	BaseMessage* pMsg = (BaseMessage*)lpData;
	switch (pMsg->m_FirstFlag)
	{
	case BaseMessage::MSG_PLAYERCFG:
		OnDispatchPlayerCfgMsg(lpSocketObj, pMsg);
		break;
	case BaseMessage::MSG_SYSCALL:
		OnDispatchSyscallMsg(lpSocketObj, pMsg);
		break;
	case BaseMessage::MSG_LOGIN:
		OnDispatchLoginMsg(lpSocketObj, pMsg);
		break;
	default:
		break;
	}
}

//处理玩家配置信息
void OnDispatchPlayerCfgMsg(LPSOCKET_OBJ lpSocketObj, BaseMessage *pMsg)
{
	BasePlayerCfgMsg* PQAMsg = (BasePlayerCfgMsg*)pMsg;
	switch (PQAMsg->m_SecondFlag)
	{
	case BasePlayerCfgMsg::MSG_FINDPLAYER:
		OnHandleFindPlayerMsg(lpSocketObj, (SQPlayerBaseInf*)PQAMsg);
		break;
	case BasePlayerCfgMsg::MSG_SAVEUPTPLAYER:
		OnHandleSavePlayerInf(lpSocketObj, (SQSavePlayerInf*)PQAMsg);
		break;
	case BasePlayerCfgMsg::MSG_TRANSFORMPLAYER:
		OnHandleTransfer(lpSocketObj, (SQTransfer*)PQAMsg);
		break;
	case BasePlayerCfgMsg::MSG_ADDITEM:
		OnHandleAddItem(lpSocketObj, (SQAddItem*)PQAMsg);
		break;
	default:
		break;
	}
}

void GMFindPlayerData(SOCKET s, SAPlayerFindOtherDataMsg *pMsg)
{
	g_lpCachePlayerBaseData = new SFixProperty();
	//memcpy(g_lpCachePlayerBaseData, &pMsg->data, sizeof(SFixProperty));
	//SAPlayerBaseInf AMsg;
	//g_PlayerID = pMsg->dwRoleID;
	//	AMsg.m_PlayerGID = pMsg->dwRoleID;
	//	AMsg.m_School = pMsg->data.m_School;
	//	AMsg.m_Level = pMsg->data.m_Level;
	//	AMsg.m_VIPLevel = pMsg->data.m_bVipLevel;
	//	AMsg.m_YinBi = pMsg->data.m_Money;
	//	AMsg.m_JinBi = pMsg->data.m_BindMoney;
	//
	//	
	//	AMsg.m_GloryDot = pMsg->data.m_PlayerExtProObj.m_ExtGloryDot;
	//	AMsg.m_Sp = pMsg->data.m_CurSp;
	//	AMsg.m_Hp = pMsg->data.m_PlayerExtProObj.m_ExtMaxHp;
	//	AMsg.m_Tp = pMsg->data.m_PlayerExtProObj.m_ExtMaxTp;
	//	AMsg.m_GongJi = pMsg->data.m_PlayerExtProObj.m_ExtGongJi;
	//	AMsg.m_FangYu = pMsg->data.m_PlayerExtProObj.m_ExtFangYu;
	//	AMsg.m_Hit = pMsg->data.m_PlayerExtProObj.m_ExtHit;
	//	AMsg.m_ShanBi = pMsg->data.m_PlayerExtProObj.m_ExtShanBi;
	//	AMsg.m_BaoJi = pMsg->data.m_PlayerExtProObj.m_ExtBaoJi;
	//	AMsg.m_KangBao = pMsg->data.m_PlayerExtProObj.m_ExtKangBao;
	//	AMsg.m_PoJi = pMsg->data.m_PlayerExtProObj.m_ExtPoJi;
	//	AMsg.m_KangPo = pMsg->data.m_PlayerExtProObj.m_ExtKangPo;
	//	AMsg.m_ChuanCi = pMsg->data.m_PlayerExtProObj.m_ExtChuanCi;
	//	AMsg.m_KangChuan = pMsg->data.m_PlayerExtProObj.m_ExtKangChuan;
	//send(s, (char*)&AMsg, sizeof(SAPlayerBaseInf), 0);
}


void OnHandleFindPlayerMsg(LPSOCKET_OBJ lpSocketObj, SQPlayerBaseInf *pMsg)
{
	if (g_lpCachePlayerBaseData == NULL)
	{
		SQPlayerFindOtherDataMsg msg;
		//msg.gid = -1;
		//strcpy(msg.RoleName, pMsg->m_PlayerName);
		//
		//try
		//{
		//	if (GetApp() != NULL)
		//	{
		//		GetApp()->m_GMQuestState = TRUE;
		//		GetApp()->m_GMSocket = lpSocketObj->m_socket;
		//		GetApp()->m_FindPlayerDataCallFunc = GMFindPlayerData;
		//		GetApp()->m_LoginServer.SendMsgToLoginSrv(&msg, (int)(sizeof(msg)));
		//	}
		//}
		//catch (lite::Xcpt &)
		//{
		//}
	}
	else
	{
		SQSavePlayerInf ksg;
		ksg.m_PlayerGID = g_PlayerID;
		ksg.m_JinBi = 1000;
		ksg.m_YinBi = 1000;
		OnHandleSavePlayerInf(lpSocketObj, &ksg);
	}
	//CPlayer *player = (CPlayer*)GetPlayerByName(pMsg->m_PlayerName)->DynamicCast(IID_PLAYER);
	//SAPlayerBaseInf AMsg;
	//if (player == NULL)
	//	AMsg.m_Result = 0;
	//else
	//{
	//	AMsg.m_PlayerGID = player->m_GID;
	//	AMsg.m_School = player->m_Property.m_School;
	//	AMsg.m_Level = player->m_Property.m_Level;
	//	AMsg.m_VIPLevel = player->m_Property.m_bVipLevel;
	//	AMsg.m_YinBi = player->m_Property.m_Money;
	//	AMsg.m_JinBi = player->m_Property.m_BindMoney;

	//	lite::Variant ret;
	//	LuaFunctor(g_Script, "GetPlayerGloryDot")[player->m_GID](&ret);
	//	AMsg.m_GloryDot = (DWORD)((int)ret);
	//	AMsg.m_Sp = player->m_Property.m_CurSp;
	//	AMsg.m_Hp = player->m_MaxHp;
	//	AMsg.m_Tp = player->m_MaxTp;
	//	AMsg.m_GongJi = player->m_GongJi;
	//	AMsg.m_FangYu = player->m_FangYu;
	//	AMsg.m_Hit = player->m_Hit;
	//	AMsg.m_ShanBi = player->m_ShanBi;
	//	AMsg.m_BaoJi = player->m_BaoJi;
	//	AMsg.m_KangBao = player->m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT];
	//	AMsg.m_PoJi = player->m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT];
	//	AMsg.m_KangPo = player->m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT];
	//	AMsg.m_ChuanCi = player->m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT];
	//	AMsg.m_KangChuan = player->m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT];
	//}
	//send(lpSocketObj->m_socket, (char*)&AMsg, sizeof(SAPlayerBaseInf), 0);
}

void GMSavePlayerData(SOCKET s, SAPlayerSaveOtherDataMsg *pData)
{

	SASavePlayerInf AMsg;
	//AMsg.m_Result = pData->byResult;
	send(s, (char*)&AMsg, sizeof(SASavePlayerInf), 0);
}

void OnHandleSavePlayerInf(LPSOCKET_OBJ lpSocketObj, SQSavePlayerInf *pMsg)
{
	SQPlayerSaveOtherDataMsg msg;
	//msg.gid = -1;
	//msg.dwRoleID = pMsg->m_PlayerGID;
	if (g_lpCachePlayerBaseData == NULL)
	{
		SASavePlayerInf AMsg;
		AMsg.m_Result = 0;
		send(lpSocketObj->m_socket, (char*)&AMsg, sizeof(SASavePlayerInf), 0);
	}

	g_lpCachePlayerBaseData->m_bVipLevel = pMsg->m_VIPLevel;
	g_lpCachePlayerBaseData->m_Money = pMsg->m_YinBi;
	g_lpCachePlayerBaseData->m_BindMoney = pMsg->m_JinBi;
	//g_lpCachePlayerBaseData->m_bVipLevel = pMsg->m_GloryDot;
	g_lpCachePlayerBaseData->m_CurSp = pMsg->m_Sp;
	//memcpy(&msg.data, g_lpCachePlayerBaseData, sizeof(SFixProperty));
	//try
	//{
	//	if (GetApp() != NULL)
	//	{
	//		GetApp()->m_GMQuestState = TRUE;
	//		GetApp()->m_GMSocket = lpSocketObj->m_socket;
	//		GetApp()->m_SavePlayerDataCallFunc = GMSavePlayerData;
	//
	//		int num = SectionMessageManager::getInstance().evaluateDevidedAmount(sizeof(msg));
	//		int id = 0;
	//		for (int i = 0; i < num; i++)
	//		{
	//			SSectionMsg sMsg;
	//			id = SectionMessageManager::getInstance().devideMessage(i, num, &sMsg, &msg, sizeof(msg), id);
	//			GetApp()->m_LoginServer.SendMsgToLoginSrv(&sMsg, sizeof(SSectionMsg));
	//		}
	//
	//	}
	//}
	//catch (lite::Xcpt &)
	//{
	//}

}

void OnHandleTransfer(LPSOCKET_OBJ lpSocketObj, SQTransfer *pMsg)
{
	CPlayer *player = (CPlayer*)GetPlayerByGID(pMsg->m_PlayerGID)->DynamicCast(IID_PLAYER);
	SATransfer AMsg;
	if (player == NULL)
	{
		AMsg.m_Result = 0;
		send(lpSocketObj->m_socket, (char*)&AMsg, sizeof(SATransfer), 0);
		return;
	}

	g_Script.SetPlayer(player);
	if (g_Script.PrepareFunction("pptm"))
	{
		g_Script.PushParameter(pMsg->m_MapID);
		g_Script.PushParameter(pMsg->m_XPos);
		g_Script.PushParameter(pMsg->m_YPos);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
	AMsg.m_Result = 1;
	send(lpSocketObj->m_socket, (char*)&AMsg, sizeof(SATransfer), 0);
}

void OnHandleAddItem(LPSOCKET_OBJ lpSocketObj, SQAddItem *pMsg)
{
	CPlayer *player = (CPlayer*)GetPlayerByGID(pMsg->m_PlayerGID)->DynamicCast(IID_PLAYER);
	SAAddItem AMsg;
	if (player == NULL)
	{
		AMsg.m_Result = 0;
		send(lpSocketObj->m_socket, (char*)&AMsg, sizeof(SATransfer), 0);
		return;
	}

	g_Script.SetPlayer(player);
	if (g_Script.PrepareFunction("GiveGoods"))
	{
		g_Script.PushParameter(pMsg->m_ItemID);
		g_Script.PushParameter(pMsg->m_ItemNum);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
	AMsg.m_Result = 1;
	send(lpSocketObj->m_socket, (char*)&AMsg, sizeof(SAAddItem), 0);
}

//系统公告消息解析
void OnDispatchSyscallMsg(LPSOCKET_OBJ lpSocketObj, BaseMessage *pMsg)
{
	BaseSyscallMsg *pQAMsg = (BaseSyscallMsg*)pMsg;
	switch (pQAMsg->m_SecondFlag)
	{
	case BaseSyscallMsg::MSG_GETRECORDSYSCALL:
		HandleRecrodSyscallMsg((void*)lpSocketObj, (SQRecordSyscallMsg *)pQAMsg);
		break;
	case BaseSyscallMsg::MSG_ADDSYSCALL:
		HandleAddSyscall((void*)lpSocketObj, (SQAddSyscallMsg *)pQAMsg);
		break;
	case BaseSyscallMsg::MSG_UPTSYSCALL:
		HandleUptSyscall((void*)lpSocketObj, (SQUptSyscallMsg *)pQAMsg);
		break;
	case BaseSyscallMsg::MSG_DELSYSCALL:
		HandleDelSyscall((void*)lpSocketObj, (SQDelSyscallMsg *)pQAMsg);
		break;
	case BaseSyscallMsg::MSG_GETCONTENTBYID:
		HandleGetContentByID((void*)lpSocketObj, (SQGetContentByIDMsg *)pQAMsg);
		break;
	default:
		break;
	}
}

//登陆GM服务器
void OnDispatchLoginMsg(LPSOCKET_OBJ lpSocketObj, BaseMessage *pMsg)
{
	BaseLoginMsg *pQAMsg = (BaseLoginMsg*)pMsg;
	switch (pQAMsg->m_SecondFlag)
	{
	case BaseLoginMsg::MSG_GMLOGIN:
		HandleLoginMsg((void*)lpSocketObj, (SQLoginGMMsg *)pQAMsg);
		break;
	default:
		break;
	}
}

void HandleLoginMsg(void* pSocketObj, SQLoginGMMsg * pMsg)
{
	unsigned int SocketPoint = (unsigned int)pSocketObj;
	if (g_Script.PrepareFunction("LoginGM"))
	{
		g_Script.PushParameter(SocketPoint);
		g_Script.PushParameter(pMsg->m_GMUserName);
		g_Script.PushParameter(pMsg->m_GMUserPwd);
		g_Script.Execute();
	}
}

void HandleRecrodSyscallMsg(void* pSocketObj, SQRecordSyscallMsg *pMsg)
{
	unsigned int SocketPoint = (unsigned int)pSocketObj;
	if (g_Script.PrepareFunction("GetSyscallMsgRecord"))
	{
		g_Script.PushParameter(SocketPoint);
		g_Script.Execute();
	}
}

void HandleAddSyscall(void* pSocketObj, SQAddSyscallMsg *pMsg)
{
	unsigned int SocketPoint = (unsigned int)pSocketObj;
	if (g_Script.PrepareFunction("AddSyscallMsg"))
	{
		g_Script.PushParameter(SocketPoint);
		g_Script.PushParameter(pMsg->m_Theme);
		g_Script.PushParameter(pMsg->m_Data);
		g_Script.PushParameter(pMsg->m_TimesPerTimes);
		g_Script.PushParameter(pMsg->m_Interval);
		g_Script.PushParameter(pMsg->m_RecordTime);
		g_Script.PushParameter(pMsg->m_StartTime);
		g_Script.PushParameter(pMsg->m_EndTime);
		g_Script.Execute();
	}
}

void HandleUptSyscall(void* pSocketObj, SQUptSyscallMsg *pMsg)
{
	unsigned int SocketPoint = (unsigned int)pSocketObj;
	if (g_Script.PrepareFunction("UptSyscallMsg"))
	{
		g_Script.PushParameter(SocketPoint);
		g_Script.PushParameter(pMsg->m_Theme);
		g_Script.PushParameter(pMsg->m_Data);
		g_Script.PushParameter(pMsg->m_TimesPerTimes);
		g_Script.PushParameter(pMsg->m_Interval);
		g_Script.PushParameter(pMsg->m_SyscallID);
		g_Script.PushParameter(pMsg->m_RecordTime);
		g_Script.PushParameter(pMsg->m_StartTime);
		g_Script.PushParameter(pMsg->m_EndTime);
		g_Script.Execute();
	}
}

void HandleDelSyscall(void* pSocketObj, SQDelSyscallMsg *pMsg)
{
	unsigned int SocketPoint = (unsigned int)pSocketObj;
	if (g_Script.PrepareFunction("DelSyscallMsg"))
	{
		g_Script.PushParameter(SocketPoint);
		g_Script.PushParameter(pMsg->m_SyscallID);
		g_Script.Execute();
	}
}


void HandleGetContentByID(void* pSocketObj, SQGetContentByIDMsg * pMsg)
{
	unsigned int SocketPoint = (unsigned int)pSocketObj;
	if (g_Script.PrepareFunction("GetSyscallContentByID"))
	{
		g_Script.PushParameter(SocketPoint);
		g_Script.PushParameter(pMsg->m_SyscallID);
		g_Script.Execute();
	}
}