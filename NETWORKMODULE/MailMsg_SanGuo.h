#pragma once
#include "../networkmodule/SanguoPlayerMsg.h"
#include "../networkmodule/DataMsgs.h"
#include "../���������/GameObjects/SanguoCode/Common/SanguoConfigTypeDef.h"

const int CONST_MAILTITLE = 256;
const int CONST_MAILCONTENT = 512;
const int CONST_MAIL_MAXITEM_NUM = 5;
const int CONST_MAIL_MAX_NUM = 20;//�ʼ������
enum MailReadState
{
	unread = 0, //δ��
	_read, //�Ѷ�,δɾ��
	getGoods,//��ȡ����
	_delete, //��ɾ��
};


///@wk �ʼ�����
struct MailInfo //�����ʼ�ʱע���ʼ��Ϊ0
{
	long id;//Ψһid,���ݿ�����,������
	int icon;//��ʾͼ��,Ŀǰǰ̨�Լ����,������
	long creatTime;//����ʱ��,�ӿ�������,������
	long readState;//��ȡ��ʾ,���ݿ�����,������
	char senderName[CONST_USERNAME];//������,Ҫ��
	char title[CONST_MAILTITLE];	//�ʼ�����,Ҫ��
	char content[CONST_MAILCONTENT];//�ʼ�����,Ҫ��
	GoodsInfoSG szItemInfo[CONST_MAIL_MAXITEM_NUM];//������Ʒ,�о�ҪҪ��

	MailInfo()
	{
		memset(this, 0, sizeof(MailInfo));
	}
};


//*************************�ʼ�MSG���**bengin******************
DECLARE_MSG_MAP(SMailMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_DB_Mail_SANGUO)
NewMailRecv,
SendMail,
GetMailInfo,
GetMailInfoDBback,
SetMailReadState,
GetMailGoods,
GetMailGoodsDBback,
GetMailGoods_res,
SendMail_sys,//ϵͳ��ȫ���ʼ�,��ȫ������200Ԫ��
NewMail,//�յ����ʼ���ʶ,����ǰ̨

END_MSG_MAP()

struct S2C_NewMail_MSG : public SMailMsg
{
	S2C_NewMail_MSG()
	{
		SMailMsg::_protocol = NewMailRecv;
	}
};
struct S2D_SendMailSYS_MSG : public SMailMsg
{
	DWORD dw_emailtype;
	DWORD dw_serverid;
	DWORD dw_expiretime;
	MailInfo m_MailInfo;

	S2D_SendMailSYS_MSG()
	{
		SMailMsg::_protocol = SendMail_sys;
	}
};
struct S2D_SendMail_MSG : public SMailMsg
{
	DWORD dw_RevId;
	char name[CONST_USERNAME];//�������,dw_RevId>0ʱ����
	DWORD dw_emailtype;
	DWORD dw_serverid;
	DWORD dw_expiretime;
	MailInfo m_MailInfo;

	S2D_SendMail_MSG()
	{
		SMailMsg::_protocol = SendMail;
	}
};
struct SQ_GetMailInfo_MSG : public SMailMsg
{
	DWORD m_dwSid;
	DNID _dnidClient;
	SQ_GetMailInfo_MSG()
	{
		SMailMsg::_protocol = GetMailInfo;
	}
};

struct SA_DBGETMailInfoList : public SMailMsg
{
		// �ʼ����������Ϊ20
	MailInfo m_MailInfo[CONST_MAIL_MAX_NUM];
	DWORD m_dwSid;
	DNID dnidClient;
	SA_DBGETMailInfoList()
	{
		SMailMsg::_protocol = GetMailInfoDBback;
	}
	
};

struct SQ_GetAwards_MSG : public SMailMsg
{
	DWORD m_dwMailId;
	DWORD m_dwSid;
	DNID dnidClient;
	SQ_GetAwards_MSG()
	{
		SMailMsg::_protocol = GetMailGoods;
		m_dwMailId = 0;
		m_dwSid = 0;
	}
};
struct SA_DB_GetAwards_MSG : public SMailMsg
{
	DWORD m_dwSid;
	DNID dnidClient;
	MailInfo m_MailInfo;
	SA_DB_GetAwards_MSG()
	{
		SMailMsg::_protocol = GetMailGoodsDBback;
	}
};

struct SA_GetAwards_MSG : public SMailMsg
{
	DWORD m_dwMailId;
	DWORD m_dwRes;
	DNID dnidClient;
	SA_GetAwards_MSG()
	{
		SMailMsg::_protocol = GetMailGoods_res;
		m_dwMailId = 0;
	}
};
struct SQ_SetMailState_MSG : public SMailMsg
{
	DWORD m_dwMailId;
	DWORD m_dwState;
	SQ_SetMailState_MSG()
	{
		SMailMsg::_protocol = SetMailReadState;
		m_dwMailId = 0;
		m_dwState = 3;
	}
};

struct SA_NewMail_MSG : public SMailMsg
{
	SA_NewMail_MSG()
	{
		SMailMsg::_protocol = NewMail;
	}
};
//*************************�ʼ�MSG���**end******************