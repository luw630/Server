#pragma once
/*
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// �绰���
//=============================================================================================
DECLARE_MSG_MAP(SBasePhoneMsg,SMessage,SMessage::EPRO_PHONE_MESSAGE)
EPRO_PHONE_CALL,		//���� pc2pc
EPRO_PHONE_ACCEPT,		//Ӧ��  ��Ϊ�漰2���ͻ��˵�˫����  2����Ϣ�޷����� �����һ����������]
EPRO_PHONE_CALL_PC2PC
END_MSG_MAP()
//=============================================================================================

//=============================================================================================
DECLARE_MSG(SPhoneCallMsg, SBasePhoneMsg, SBasePhoneMsg::EPRO_PHONE_CALL)

struct SQPhoneCallMsg : public SPhoneCallMsg	//�ͻ��˷���������Ϣ
{
	
	SQPhoneCallMsg()
	{
		memset(m_szCallerName,0,sizeof(m_szCallerName));
		memset(m_szAccepterName,0,sizeof(m_szAccepterName));
	}
	char	m_szCallerName[CONST_USERNAME];
	char	m_szAccepterName[CONST_USERNAME];
	BYTE   m_nDirectionFlag; //���е�����,���PC2Phone ������Ѳ�����,PC2PC�Ļ������Ǳ������ߵ�
	enum {EN_Unknown,EN_PCtoPC,EN_PCtoConfCaller,EN_PCtoConfCallee,EN_PCtoPhone,EN_PCtoIVR,EN_PhonetoPC,EN_PhonetoConfCaller,EN_PhonetoConfCallee,EN_LOGIN,EN_LOGOUT};
};

struct SAPhoneCallMsg : public SPhoneCallMsg	//����Ƿ��������ظ�����˵���Ϣ
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

struct SAPhoneAcceptMsg :public SPhoneAcceptMsg	//�����������ͻ���˵�����Է�����������
{
	char	m_szCallerName[CONST_USERNAME];
	char	m_szAccepterName[CONST_USERNAME];
	BYTE m_iRet;
	enum
	{
		RET_CALLING,		//�Է�����ͨ����
		RET_ERROR ,//��ʼ��ʧ��
		RET_COMPLETE, //�ɹ�,���Է�������ͨ�� 
		RET_REJECT //�Է��ܾ�ͨ��
	};
};
DECLARE_MSG(SPhoneCallMsg_PC2PC, SBasePhoneMsg, SBasePhoneMsg::EPRO_PHONE_CALL_PC2PC)

struct SQPhoneCallMsg_PC2PC : public SPhoneCallMsg_PC2PC	//�ͻ��˷���������ϢC
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