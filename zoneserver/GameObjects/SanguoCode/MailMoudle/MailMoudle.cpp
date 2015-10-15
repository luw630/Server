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
	case SMailMsg::SendMail: //�ʼ���Ϣ
		//SendMail_SanGuo(DNID, (S2D_SendMail_MSG*)pMsg);
		break;
	case SMailMsg::GetMailInfo: //ȡ�ʼ���Ϣ
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

//���ʼ�,�������ݿ�,ϵͳ����ȫ��  _expiretimeΪ�ӷ�������ٷ��Ӻ�ʧЧ
void MailMoudle::SendMail_SYS_SanGuo(uint32_t _expiretime, MailInfo*_MailInfo)
{
	S2D_SendMailSYS_MSG msg;
	msg.dw_emailtype = 2;//ϵͳ�ʼ�
	msg.dw_serverid = GetServerID();
	msg.dw_expiretime = _expiretime;

	memcpy(&msg.m_MailInfo, _MailInfo, sizeof(MailInfo));
	msg.m_MailInfo.creatTime = time(NULL);
	SendToLoginServer(&msg, sizeof(msg));
}

//���ʼ�,�������ݿ�,ϵͳ�������� recv_pSid Ϊ�ʼ�������ҵ�sid,ע�ⲻ��dnidClient;_expiretimeΪ�ӷ�������ٷ��Ӻ�ʧЧ
void MailMoudle::SendMail_SanGuo(int recv_pSid, uint32_t _expiretime, MailInfo*_MailInfo)
{

	S2D_SendMail_MSG msg;
	msg.dw_emailtype=2;//ϵͳ�ʼ�
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
//��������ַ��ʼ�
void MailMoudle::SendMail_SanGuoByName(char name[CONST_USERNAME], uint32_t _expiretime, MailInfo*_MailInfo)
{
	CPlayer* pPlayer;
	pPlayer = (CPlayer *)GetPlayerByName(name)->DynamicCast(IID_PLAYER);
	if (!pPlayer)
	{
		S2D_SendMail_MSG msg;
		msg.dw_emailtype = 2;//ϵͳ�ʼ�
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
//ȡ�ʼ��б���Ϣ
void MailMoudle::GetMailInfoFromDB(int sid, DNID dnidClient)
{
	SQ_GetMailInfo_MSG msg;
	msg.m_dwSid = sid;
	msg._dnidClient = dnidClient;
	SendToLoginServer(&msg, sizeof(msg));
	CPlayer *player = (CPlayer*)GetPlayerByDnid(dnidClient)->DynamicCast(IID_PLAYER);
}
//DB�����ʼ��б���Ϣ,����ǰ̨
void MailMoudle::DbBack_GetMailInfo(SA_DBGETMailInfoList* Msg)
{
	
	SA_DBGETMailInfoList rMsg;
	memcpy(&rMsg, Msg, sizeof(SA_DBGETMailInfoList));
	g_StoreMessage(Msg->dnidClient, &rMsg, sizeof(SA_DBGETMailInfoList));
}
//ǰ̨ȡ�ʼ�����,�������ݿ���֤�󷵻ظ�����
void MailMoudle::GetMailAwards(DNID dnidClient, SQ_GetAwards_MSG * PMSG, int sid)
{
	SQ_GetAwards_MSG Msg;
	Msg.m_dwSid = sid;
	Msg.dnidClient = dnidClient;
	Msg.m_dwMailId = PMSG->m_dwMailId;
	SendToLoginServer(&Msg, sizeof(Msg));
}
//���ݿ���֤�󷵻ظ�����
void MailMoudle::DbBack_GetMailAwards(SA_DB_GetAwards_MSG* Msg)
{
	SA_GetAwards_MSG rMsg;
	rMsg.m_dwMailId = Msg->m_MailInfo.id;

	int isread = Msg->m_MailInfo.readState;
	if (isread == MailReadState::getGoods || isread == MailReadState::_delete)
	{
		rMsg.m_dwRes = 2;//ʧ��
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
				//������;
				player->GetSanguoBaseData()->AddGoods_SG(Msg->m_MailInfo.szItemInfo[i].itype, Msg->m_MailInfo.szItemInfo[i].id, Msg->m_MailInfo.szItemInfo[i].num,GoodsWay::mail);
			};
		};
		rMsg.m_dwRes = 1;//�ɹ�
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