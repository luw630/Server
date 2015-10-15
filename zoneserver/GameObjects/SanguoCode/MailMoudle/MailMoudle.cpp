#include"stdafx.h"
#include "MailMoudle.h"
#include "playermanager.h"
#include "player.h"
#include "..\BaseDataManager.h"
using namespace std;

MailMoudle::MailMoudle()
{

}

MailMoudle::~MailMoudle()
{
	Release();
}

bool MailMoudle::Release()
{
	return true;
}

void MailMoudle::DispatchMailMsg(DNID dnidClient, SMailMsg *pMsg, int sid)
{
	SMailMsg * pMailMsg = (SMailMsg*)pMsg;
	switch (pMsg->_protocol)
	{
	case SMailMsg::SendMail: //邮件信息
		//SendMail_SanGuo(DNID, (S2D_SendMail_MSG*)pMsg);
		break;
	case SMailMsg::GetMailInfo: //取邮件信息
		GetMailInfoFromDB(sid, dnidClient);
		break;
	case SMailMsg::GetMailInfoDBback: 
		DbBack_GetMailInfo((SA_DBGETMailInfoList*)pMsg);
		break;
	case SMailMsg::GetMailGoods:
		GetMailAwards(dnidClient, (SQ_GetAwards_MSG *)pMsg, sid);
		break;
	case SMailMsg::GetMailGoodsDBback:
		DbBack_GetMailAwards((SA_DB_GetAwards_MSG*)pMsg);
		break;
	case SMailMsg::SetMailReadState:
		SetMailState((SQ_SetMailState_MSG *) pMsg ,sid);
		break;
	default:
		break;
	}
}

//发邮件,丢往数据库,系统发给全服  _expiretime为从发送起多少分钟后失效
void MailMoudle::SendMail_SYS_SanGuo(uint32_t _expiretime, MailInfo*_MailInfo)
{
	S2D_SendMailSYS_MSG msg;
	msg.dw_emailtype = 2;//系统邮件
	msg.dw_serverid = GetServerID();
	msg.dw_expiretime = _expiretime;

	memcpy(&msg.m_MailInfo, _MailInfo, sizeof(MailInfo));
	msg.m_MailInfo.creatTime = time(NULL);
	SendToLoginServer(&msg, sizeof(msg));
}

//发邮件,丢往数据库,系统发给个人 recv_pSid 为邮件接收玩家的sid,注意不是dnidClient;_expiretime为从发送起多少分钟后失效
void MailMoudle::SendMail_SanGuo(int recv_pSid, uint32_t _expiretime, MailInfo*_MailInfo)
{

	S2D_SendMail_MSG msg;
	msg.dw_emailtype=2;//系统邮件
	msg.dw_serverid=GetServerID();

	msg.dw_expiretime = _expiretime;
	msg.dw_RevId = recv_pSid;

	

	memcpy(&msg.m_MailInfo, _MailInfo, sizeof(MailInfo));
	msg.m_MailInfo.creatTime = time(NULL);
	SendToLoginServer( &msg, sizeof(msg) );


	CPlayer* pPlayer;
	pPlayer = (CPlayer *)GetPlayerBySID(recv_pSid)->DynamicCast(IID_PLAYER);
		if (!pPlayer)
			return;
	SA_NewMail_MSG msgmail;
	g_StoreMessage(pPlayer->GetSanguoBaseData()->GetDNID(), &msgmail, sizeof(SA_NewMail_MSG));
}
//以玩家名字发邮件
void MailMoudle::SendMail_SanGuoByName(char name[CONST_USERNAME], uint32_t _expiretime, MailInfo*_MailInfo)
{
	CPlayer* pPlayer;
	pPlayer = (CPlayer *)GetPlayerByName(name)->DynamicCast(IID_PLAYER);
	if (!pPlayer)
	{
		S2D_SendMail_MSG msg;
		msg.dw_emailtype = 2;//系统邮件
		msg.dw_serverid = GetServerID();
		msg.dw_expiretime = _expiretime;
		msg.dw_RevId = 0;

		memcpy(msg.name, name, CONST_USERNAME);
		memcpy(&msg.m_MailInfo, _MailInfo, sizeof(MailInfo));
		msg.m_MailInfo.creatTime = time(NULL);
		SendToLoginServer(&msg, sizeof(msg));
	}
	else
	{
		SendMail_SanGuo(pPlayer->m_FixData.m_dwStaticID, _expiretime, _MailInfo);
	}
}
//取邮件列表信息
void MailMoudle::GetMailInfoFromDB(int sid, DNID dnidClient)
{
	SQ_GetMailInfo_MSG msg;
	msg.m_dwSid = sid;
	msg._dnidClient = dnidClient;
	SendToLoginServer(&msg, sizeof(msg));
	CPlayer *player = (CPlayer*)GetPlayerByDnid(dnidClient)->DynamicCast(IID_PLAYER);
}
//DB返回邮件列表信息,发回前台
void MailMoudle::DbBack_GetMailInfo(SA_DBGETMailInfoList* Msg)
{
	
	SA_DBGETMailInfoList rMsg;
	memcpy(&rMsg, Msg, sizeof(SA_DBGETMailInfoList));
	g_StoreMessage(Msg->dnidClient, &rMsg, sizeof(SA_DBGETMailInfoList));
}
//前台取邮件附件,发往数据库验证后返回给东西
void MailMoudle::GetMailAwards(DNID dnidClient, SQ_GetAwards_MSG * PMSG, int sid)
{
	SQ_GetAwards_MSG Msg;
	Msg.m_dwSid = sid;
	Msg.dnidClient = dnidClient;
	Msg.m_dwMailId = PMSG->m_dwMailId;
	SendToLoginServer(&Msg, sizeof(Msg));
}
//数据库验证后返回给东西
void MailMoudle::DbBack_GetMailAwards(SA_DB_GetAwards_MSG* Msg)
{
	SA_GetAwards_MSG rMsg;
	rMsg.m_dwMailId = Msg->m_MailInfo.id;

	int isread = Msg->m_MailInfo.readState;
	if (isread == MailReadState::getGoods || isread == MailReadState::_delete)
	{
		rMsg.m_dwRes = 2;//失败
		return;
	}
	else
	{
		CPlayer *player = (CPlayer*)GetPlayerByDnid(Msg->dnidClient)->DynamicCast(IID_PLAYER);
		if (player==NULL)
		{
			return;
		}
		for (int i = 0; i < CONST_MAIL_MAXITEM_NUM; i++)
		{
			if (Msg->m_MailInfo.szItemInfo[i].itype > 0)
			{
				//给奖励;
				player->GetSanguoBaseData()->AddGoods_SG(Msg->m_MailInfo.szItemInfo[i].itype, Msg->m_MailInfo.szItemInfo[i].id, Msg->m_MailInfo.szItemInfo[i].num,GoodsWay::mail);
			};
		};
		rMsg.m_dwRes = 1;//成功
	};
	
	g_StoreMessage(Msg->dnidClient, &rMsg, sizeof(SA_GetAwards_MSG));
}

void MailMoudle::SetMailState(SQ_SetMailState_MSG * pmsg,int sid)
{
	if (pmsg->m_dwState<MailReadState::_read || pmsg->m_dwState>MailReadState::_delete)
	{
		return;
	}
	SQ_SetMailState_MSG Msg;
	Msg.m_dwMailId = pmsg->m_dwMailId;
	Msg.m_dwState = pmsg->m_dwState;
	SendToLoginServer(&Msg, sizeof(Msg));
}