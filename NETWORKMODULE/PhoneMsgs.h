#pragma once
/*
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 电话相关
//=============================================================================================
DECLARE_MSG_MAP(SBasePhoneMsg,SMessage,SMessage::EPRO_PHONE_MESSAGE)
EPRO_PHONE_CALL,		//呼叫 pc2pc
EPRO_PHONE_ACCEPT,		//应答  因为涉及2个客户端的双向发送  2个消息无法做到 额外加一个。。。。]
EPRO_PHONE_CALL_PC2PC
END_MSG_MAP()
//=============================================================================================

//=============================================================================================
DECLARE_MSG(SPhoneCallMsg, SBasePhoneMsg, SBasePhoneMsg::EPRO_PHONE_CALL)

struct SQPhoneCallMsg : public SPhoneCallMsg	//客户端发出请求消息
{
	
	SQPhoneCallMsg()
	{
		memset(m_szCallerName,0,sizeof(m_szCallerName));
		memset(m_szAccepterName,0,sizeof(m_szAccepterName));
	}
	char	m_szCallerName[CONST_USERNAME];
	char	m_szAccepterName[CONST_USERNAME];
	BYTE   m_nDirectionFlag; //呼叫的类型,如果PC2Phone 允许好友不在线,PC2PC的话好友是必须在线的
	enum {EN_Unknown,EN_PCtoPC,EN_PCtoConfCaller,EN_PCtoConfCallee,EN_PCtoPhone,EN_PCtoIVR,EN_PhonetoPC,EN_PhonetoConfCaller,EN_PhonetoConfCallee,EN_LOGIN,EN_LOGOUT};
};

struct SAPhoneCallMsg : public SPhoneCallMsg	//这个是服务器返回给请求端的消息
{
	SAPhoneCallMsg()
	{
		memset(m_szCallerName,0,sizeof(m_szCallerName));
		memset(m_szAccepterName,0,sizeof(m_szAccepterName));
		memset(m_szAccepterID,0,sizeof(m_szAccepterID));
		memset(m_szCallerID,0,sizeof(m_szCallerID));
		memset(m_szAccepterPhoneNumber,0,sizeof(m_szAccepterPhoneNumber));
	    memset(m_szAccepterPwd,0,sizeof(m_szAccepterPwd));
		memset(m_szCallerIDPwd,0,sizeof(m_szCallerIDPwd));

	}
	char	m_szCallerName[CONST_USERNAME];
	char	m_szAccepterName[CONST_USERNAME];
	char	m_szCallerID[50];
	char	m_szAccepterID[50];
	char    m_szAccepterPhoneNumber[50];
    char    m_szCallerIDPwd[20];
	char    m_szAccepterPwd[20];
	int		m_iRet;
    enum { R_INVALID_ACCOUNT,R_COMPLETE,R_ERROR,R_NOTFRIEND,R_NOONLINE,R_NOTBINDACC}; 
	
};
//=============================================================================================

//=============================================================================================

DECLARE_MSG(SPhoneAcceptMsg, SBasePhoneMsg, SBasePhoneMsg::EPRO_PHONE_ACCEPT)

struct SAPhoneAcceptMsg :public SPhoneAcceptMsg	//服务器发给客户端说明可以发起语音呼叫
{
	char	m_szCallerName[CONST_USERNAME];
	char	m_szAccepterName[CONST_USERNAME];
	BYTE m_iRet;
	enum
	{
		RET_CALLING,		//对方正在通话中
		RET_ERROR ,//初始化失败
		RET_COMPLETE, //成功,可以发起语音通话 
		RET_REJECT //对方拒绝通话
	};
};
DECLARE_MSG(SPhoneCallMsg_PC2PC, SBasePhoneMsg, SBasePhoneMsg::EPRO_PHONE_CALL_PC2PC)

struct SQPhoneCallMsg_PC2PC : public SPhoneCallMsg_PC2PC	//客户端发出请求消息C
{
	char	m_szCallerName[CONST_USERNAME];
	char    m_szCallerID[50];
	char	m_szAccepterName[CONST_USERNAME];
	char	m_szAccepterID[50];
	char    m_szAccepterPwd[20];
	SQPhoneCallMsg_PC2PC()
	{
		memset(m_szCallerName,0,sizeof(m_szCallerName));
		memset(m_szAccepterName,0,sizeof(m_szAccepterName));
		memset(m_szAccepterID,0,sizeof(m_szAccepterID));
		memset(m_szAccepterPwd,0,sizeof(m_szAccepterPwd));
		memset(m_szCallerID,0, sizeof(m_szCallerID));

	}
};

*/