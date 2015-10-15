#pragma once

#include "MailTypedef.h"
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ����
//=============================================================================================
DECLARE_MSG_MAP(SMailBaseMsg,SMessage,SMessage::EPRO_MAIL_MESSAGE)
//{{AFX
EPRO_MAIL_SEND,			// ��������
EPRO_MAIL_RECV,			// ��������
EPRO_MAIL_DELETE,		// ɾ������
EPRO_MAIL_NEWMAIL,		// ������֪ͨ
EPRO_BROADCAST,			// ��㲥
//}}AFX
END_MSG_MAP()
//=============================================================================================

//=============================================================================================
DECLARE_MSG(SMailSendMsg, SMailBaseMsg, SMailBaseMsg::EPRO_MAIL_SEND)

struct SQMailSendMsg : public SMailSendMsg
{
	int m_iStaticID;
    CMail m_mail;
};

struct SAMailSendMsg : public SMailSendMsg
{
	int m_iStaticID;
	enum
	{
		RET_SUCCESS,
		RET_FAIL
	};

	int m_iRet;
};
//=============================================================================================

//=============================================================================================
DECLARE_MSG(SMailRecvMsg, SMailBaseMsg, SMailBaseMsg::EPRO_MAIL_RECV)

struct SQMailRecvMsg : public SMailRecvMsg
{
	int m_iStaticID;
	int m_iIndex;
};

struct SAMailRecvMsg : public SMailRecvMsg
{
	int m_iStaticID;
	enum
	{
		RET_SUCCESS,
		RET_FAIL
	};

	CMail m_mail[10];
	int m_iIndex;
	int m_iRet;
};
//=============================================================================================

//=============================================================================================
DECLARE_MSG(SMailDeleteMsg, SMailBaseMsg, SMailBaseMsg::EPRO_MAIL_DELETE)

struct SQMailDeleteMsg : public SMailDeleteMsg
{
	int m_iStaticID;
	int m_iNumber;
};

struct SAMailDeleteMsg : public SMailDeleteMsg
{
	int m_iStaticID;	
	enum
	{
		RET_SUCCESS,
		RET_FAIL
	};

	int m_iNumber;
	int m_iRet;
};
//=============================================================================================

//=============================================================================================
DECLARE_MSG(SMailNewMailMsg, SMailBaseMsg, SMailBaseMsg::EPRO_MAIL_NEWMAIL)

struct SQNewMailMsg : public SMailNewMailMsg
{
	int m_iStaticID;
};

struct SANewMailMsg : public SMailNewMailMsg
{
	int m_iStaticID;
};

//=============================================================================================
// ��㲥
DECLARE_MSG(SBroadCastMsg, SMailBaseMsg, SMailBaseMsg::EPRO_BROADCAST)
struct SSendBroadCast : public SBroadCastMsg
{
	int sMegID;
};