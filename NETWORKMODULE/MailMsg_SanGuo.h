#pragma once
#include "../networkmodule/SanguoPlayerMsg.h"
#include "../networkmodule/DataMsgs.h"
#include "../区域服务器/GameObjects/SanguoCode/Common/SanguoConfigTypeDef.h"

const int CONST_MAILTITLE = 256;
const int CONST_MAILCONTENT = 512;
const int CONST_MAIL_MAXITEM_NUM = 5;
const int CONST_MAIL_MAX_NUM = 20;//邮件最大数
enum MailReadState
{
	unread = 0, //未读
	_read, //已读,未删除
	getGoods,//已取附件
	_delete, //已删除
};


///@wk 邮件构成
struct MailInfo //发送邮件时注意初始化为0
{
	long id;//唯一id,数据库生成,不用填
	int icon;//显示图标,目前前台自己检测,不用填
	long creatTime;//发送时间,接口已生成,不用填
	long readState;//读取标示,数据库生成,不用填
	char senderName[CONST_USERNAME];//发送人,要填
	char title[CONST_MAILTITLE];	//邮件标题,要填
	char content[CONST_MAILCONTENT];//邮件内容,要填
	GoodsInfoSG szItemInfo[CONST_MAIL_MAXITEM_NUM];//奖励物品,有就要要填

	MailInfo()
	{
		memset(this, 0, sizeof(MailInfo));
	}
};


//*************************邮件MSG相关**bengin******************
DECLARE_MSG_MAP(SMailMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_DB_Mail_SANGUO)
NewMailRecv,
SendMail,
GetMailInfo,
GetMailInfoDBback,
SetMailReadState,
GetMailGoods,
GetMailGoodsDBback,
GetMailGoods_res,
SendMail_sys,//系统发全服邮件,如全服奖励200元宝
NewMail,//收到新邮件标识,发给前台

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
	char name[CONST_USERNAME];//玩家名字,dw_RevId>0时无用
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
		// 邮件数量，最大为20
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
//*************************邮件MSG相关**end******************