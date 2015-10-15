#include"stdafx.h"
#include "GmToolMoudle.h"
#include "playermanager.h"
#include "player.h"
#include "..\BaseDataManager.h"
#include "..\MailMoudle\MailMoudle.h"

DNID gm_dnidClient = 0;
GmToolMoudle::GmToolMoudle()
{

}

GmToolMoudle::~GmToolMoudle()
{
	Release();
}

bool GmToolMoudle::Release()
{
	return true;
}

void GmToolMoudle::DispatchGmToolMsg(DNID dnidClient, SGMTOOLMsg *pMsg)
{
	gm_dnidClient = dnidClient;
	switch (pMsg->_protocol)
	{
	case SGMTOOLMsg::GMq_login: 
		gm_login(dnidClient, (SQgm_login* )pMsg);
		break;
	case SGMTOOLMsg::GMq_getplayerinfo: 
		gm_getplayerinfo(dnidClient, (SQgm_getplayerinfo *)pMsg);
		break;
	case SGMTOOLMsg::GMq_broadcast:
		gm_broadcast(dnidClient, (SQgm_broadcast *)pMsg);
		break;
	case SGMTOOLMsg::GM_registerNum:
		gm_registerNum(dnidClient, (SQgm_registerNum *)pMsg);
		break;
	case SGMTOOLMsg::GM_stayAll:
		gm_stayAll(dnidClient, (SQgm_stayAll *)pMsg);
		break;
	case SGMTOOLMsg::GM_stayOne:
		gm_stayOne(dnidClient, (SQgm_stayOne *)pMsg);
		break;
	case SGMTOOLMsg::GM_online:
		gm_online(dnidClient, (SQgm_online *)pMsg);
		break;
	case SGMTOOLMsg::GM_recharge:
		gm_recharge(dnidClient, (SQgm_recharge *)pMsg);
		break;
	case SGMTOOLMsg::GM_leave:
		gm_leavedata(dnidClient, (SQgm_leave *)pMsg);
		break;
	case SGMTOOLMsg::GM_pointuse:
		gm_pointuse(dnidClient, (SQgm_pointuse *)pMsg);
		break;
	case SGMTOOLMsg::GM_sendmail:
		gm_mail(dnidClient, (SQgm_mail *)pMsg);
		
		break;
	case SGMTOOLMsg::GM_operation:
		gm_operation(dnidClient, (SQgm_operation *)pMsg);

		break;
	default:
		break;
	}
}

void GmToolMoudle::gm_login(DNID dnidClient, SQgm_login* pMsg)
{
	/*if (g_Script.PrepareFunction("LoginGM"))
	{
	g_Script.PushParameter(1);
	g_Script.PushParameter(pMsg->m_GMUserName);
	g_Script.PushParameter(pMsg->m_GMUserPwd);
	g_Script.Execute();
	}
	*/
	//账号验证
	lite::Variant ret;
	LuaFunctor(g_Script, "LoginGM")[pMsg->m_GMUserName][pMsg->m_GMUserPwd](&ret);
	int isGm = (int)ret;


	SAgm_login rMsg;
	rMsg.m_Result = isGm; 
	g_StoreMessage(dnidClient, &rMsg, sizeof(SAgm_login));

	
}

//取玩家信息,必须要在线玩家才能取到
void GmToolMoudle::gm_getplayerinfo(DNID dnidClient, SQgm_getplayerinfo *pMsg)
{
	CPlayer * pPlayer=nullptr;
	SAgm_getplayerinfo rMsg;
	BYTE itype = pMsg->itype;
	char* m_GMUserInfo = pMsg->m_GMUserInfo;
	switch (itype)
	{
	case 1://name
		pPlayer = (CPlayer *)GetPlayerByName(m_GMUserInfo)->DynamicCast(IID_PLAYER);

		break;
	case 2://account
			pPlayer = (CPlayer *)GetPlayerByAccount(m_GMUserInfo)->DynamicCast(IID_PLAYER);
		break;
	case 3://sid
		pPlayer = (CPlayer *)GetPlayerBySID(atoi(m_GMUserInfo))->DynamicCast(IID_PLAYER);
		break;
	default:
		break;
	}
	if (!pPlayer)
	{
		rMsg.m_Result = 2;//不在线
		g_StoreMessage(dnidClient, &rMsg, sizeof(SAgm_getplayerinfo));
		return;
	}
		
	rMsg.m_Result = 1;
	rMsg.pData = pPlayer->m_FixData;
	g_StoreMessage(dnidClient, &rMsg, sizeof(SAgm_getplayerinfo));
}

void GmToolMoudle::gm_broadcast(DNID dnidClient, SQgm_broadcast * pMsg)
{
	dwt::stringkey<char[MAX_CHAT_LEN]> str = pMsg->cChatData;
	SAChatGlobalMsg msg;
	msg.byType = SAChatGlobalMsg::ECT_SYSTEM;
	dwt::strcpy(msg.cChatData, pMsg->cChatData, MAX_CHAT_LEN);

	SAgm_broadcast rMsg;
	rMsg.m_Result = 1;
	g_StoreMessage(dnidClient, &rMsg, sizeof(SAgm_broadcast));

	BroadcastMsg(&msg, msg.GetMySize());
}
void GmToolMoudle::gm_registerNum(DNID dnidClient, SQgm_registerNum *pMsg)
{
	g_Script.CallFunc("db_gm_registerNum", pMsg->beginTime, pMsg->endTime);
}
void GmToolMoudle::gm_stayAll(DNID dnidClient, SQgm_stayAll *pMsg)
{

	g_Script.CallFunc("db_gm_stayAll", pMsg->beginTime);


	/*SAgm_stayAll rMsg;
	int i;
	for (i = 0; i < 11; i++)
	{
	rMsg.result[i] = (double)i / 100;
	}
	g_StoreMessage(dnidClient, &rMsg, sizeof(SAgm_stayAll));*/
}
void GmToolMoudle::gm_stayOne(DNID dnidClient, SQgm_stayOne *pMsg)
{
	g_Script.CallFunc("db_gm_stayOne", pMsg->beginTime, pMsg->day);
	/*
		SAgm_stayOne rMsg;
		rMsg.result[0] = 5;
		rMsg.result[1] = 0.6;
		g_StoreMessage(dnidClient, &rMsg, sizeof(SAgm_stayOne));*/
}
void GmToolMoudle::gm_online(DNID dnidClient, SQgm_online *pMsg)
{
	g_Script.CallFunc("db_gm_online", pMsg->beginTime);
}
void GmToolMoudle::gm_recharge(DNID dnidClient, SQgm_recharge *pMsg)
{
	SAgm_recharge rMsg;
	int i;
	for (i = 0; i < MAX_GM_recharge; i++)
	{
		rMsg.result[i] = i * 3;
	} 
	
	g_StoreMessage(dnidClient, &rMsg, sizeof(SAgm_recharge));
}

void GmToolMoudle::gm_leavedata(DNID dnidClient, SQgm_leave *pMsg)
{
	g_Script.CallFunc("db_gm_getleavedata", pMsg->beginTime, pMsg->leaveday, pMsg->itype, pMsg->num);
}
void GmToolMoudle::gm_pointuse(DNID dnidClient, SQgm_pointuse *pMsg)
{
	if (g_Script.PrepareFunction("db_gm_getpointuse"))
	{
		g_Script.PushParameter(pMsg->optype);
		g_Script.PushParameter(pMsg->begintime);
		g_Script.PushParameter(pMsg->itype);
		g_Script.PushParameter(pMsg->detailType);
		g_Script.PushParameter(pMsg->otherinfo);
		g_Script.PushParameter(pMsg->lv_mini);
		g_Script.PushParameter(pMsg->lv_max);
		g_Script.Execute();
	}
}

void GmToolMoudle::gm_mail(DNID dnidClient, SQgm_mail *pMsg)
{
	if (pMsg->optype==1)//全服
	{
		MailMoudle::getSingleton()->SendMail_SYS_SanGuo(1440, &pMsg->_MailInfo);
	}
	else if (pMsg->optype == 2)//单人
	{
		MailMoudle::getSingleton()->SendMail_SanGuoByName(pMsg->name, 1440, &pMsg->_MailInfo);
	}
}

void GmToolMoudle::gm_operation(DNID dnidClient, SQgm_operation *pMsg)
{
	//g_Script.CallFunc("db_gm_getoperation", pMsg->itype, pMsg->begintime,pMsg->endtime);

	if (g_Script.PrepareFunction("db_gm_getoperation"))
	{
		g_Script.PushParameter(pMsg->itype);
		g_Script.PushParameter(pMsg->begintime);
		g_Script.PushParameter(pMsg->endtime);
		g_Script.Execute();
	}
}
