//////////////////////////////////////////////////////////////////////
/*
	Created:	2012/03/02
	Filename: 	CenterMsg.h
	Author:		TonyJJ
	Commnet:	���ķ���Ϣ
*/
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _H_CenterMsg_H_
#define _H_CenterMsg_H_

#include "NetModule.h"
#include "..\PUB\ConstValue.h"
DECLARE_MSG_MAP( SCenterBaseMsg , SMessage , SMessage::EPRO_Center_MESSAGE )
EPRO_RequestClientVersionInfo,				//����ͻ��˰汾��Ϣ
EPRO_RequestServerState,						//���������״̬
EPRO_RegisterServer,							//ע����Ϸ�������ķ�
EPRO_LoginPlayer,								//��ҳɹ���½���������ע�ᵽ���ķ�
EPRO_LogoutPlayer,								//��ҶϿ�������������Ӻ�ע�����ķ����������
EPRO_RequestServerDetailInfo,					//����һ������������ϸ��Ϣ
EPRO_RegisterLoginServer,						//ע���½�������ķ�   2015/6/11  �����ѵ�½�������Լ����ݿ������ͬʱע�ᵽ���ķ����
EPRO_RegisterDBServer,						//ע���½�������ķ�
EPRO_LogoutServer,							//�����������ķ����˳�
EPRO_CheckWhiteList,							//���ķ�������������Ϊ���ķ��������з�����
END_MSG_MAP()

//�ͻ���������ʱ�������ķ����������״̬
DECLARE_MSG( SRequestServerStateMsg , SCenterBaseMsg , SCenterBaseMsg::EPRO_RequestServerState )
struct SQRequestServerState : public SRequestServerStateMsg 
{
	WORD b;
};

struct ServerState
{
	ServerState() : dwIndex( 0 ),
		dwNew( 0 ),
		dwActity( 0 ),
		dwRush( 0 ),
		dwTotalPlayers( 0 )
	{
	}
	DWORD	dwIndex;
	DWORD	dwNew;
	DWORD	dwActity;
	DWORD	dwRush;
	DWORD	dwTotalPlayers;
};
struct SARequestServerState : public SRequestServerStateMsg 
{
	SARequestServerState()
	{
		wSaveCount = 0;
	}
	const static WORD csiMaxSaveCount = 100;
	WORD		wSaveCount;
	ServerState	atServerData[csiMaxSaveCount];
	//BYTE Buff[1024];
	int MySize()
	{
		return sizeof(SARequestServerState) - (csiMaxSaveCount - wSaveCount)*sizeof(ServerState);
	}
};

//��������ķ�����һ������������ϸ��Ϣ
DECLARE_MSG( SRequestServerDetailInfoMsg , SCenterBaseMsg , SCenterBaseMsg::EPRO_RequestServerDetailInfo )
struct SQRequestServeDetailInfoMsg : public SRequestServerDetailInfoMsg 
{
	DWORD	dwIndex; // ����������
};

struct SARequestServeDetailInfoMsg : public SRequestServerDetailInfoMsg 
{
	DWORD	dwIndex; // ����������
	char streamData[512]; // ��������
};


//��Ϸ�����������ķ�ע���������Ϣ
DECLARE_MSG( SRegisterServer , SCenterBaseMsg , SCenterBaseMsg::EPRO_RegisterServer )
struct SQRegisterServer : public SRegisterServer
{
	unsigned long dwIP; // IP
	unsigned long dwPort; // �˿ں�
	unsigned long index;
	unsigned long _dNew;
	unsigned long _dActivity;
	unsigned long _dRush;
	char streamData[512];	// �˺�&����
};

struct SARegisterServer : public SRegisterServer 
{
	enum ERetCode
	{
		ERC_Register_Success,
		ERC_Register_Fail,
	};
	ERetCode code;
};

//ע���½������
DECLARE_MSG(SRegisterLoginServer, SCenterBaseMsg, SCenterBaseMsg::EPRO_RegisterLoginServer)
struct SQRegisterLoginServer : public SRegisterLoginServer
{
	unsigned long dwIP; // IP
	unsigned long dwPort; // �˿ں�
	unsigned long index;
	unsigned long _dNew;
	unsigned long _dActivity;
	unsigned long _dRush;
	char streamservername[128]; //����������
};

struct SARegisterLoginServer : public SRegisterLoginServer
{
	enum ERetCode
	{
		ERC_Register_Success,
		ERC_Register_Fail,
	};
	ERetCode code;
};


//ע�����ݿ������
DECLARE_MSG(SRegisterDBServer, SCenterBaseMsg, SCenterBaseMsg::EPRO_RegisterDBServer)
struct SQRegisterDBServer : public SRegisterDBServer
{
	unsigned long dwIP; // IP
	unsigned long dwPort; // �˿ں�
	unsigned long index;
	unsigned long _dNew;
	unsigned long _dActivity;
	unsigned long _dRush;
	char streamservername[512];	// �˺�&����
};

struct SARegisterDBServer : public SRegisterDBServer
{
	enum ERetCode
	{
		ERC_Register_Success,
		ERC_Register_Fail,
	};
	ERetCode code;
};


DECLARE_MSG( SLoginPlayer , SCenterBaseMsg , SCenterBaseMsg::EPRO_LoginPlayer )
struct SQLoginPlayer : public SLoginPlayer
{
	DNID	dnidClient;
	char streamData[512];
};

DECLARE_MSG( SLogoutPlayer , SCenterBaseMsg , SCenterBaseMsg::EPRO_LogoutPlayer )
struct SQLogoutPlayer : public SLogoutPlayer
{
	DNID dnidClient;
};

DECLARE_MSG(SRequestAssetUpdateInfo, SCenterBaseMsg, SCenterBaseMsg::EPRO_RequestClientVersionInfo)
struct SQRequestAssetUpdateInfo : public SRequestAssetUpdateInfo
{
	int16_t flag;
};

struct  SARequestAssetUpdateInfo : public SRequestAssetUpdateInfo
{
	int streamLength;
	char streamData[8192];
};


DECLARE_MSG(SLogoutServer, SCenterBaseMsg, SCenterBaseMsg::EPRO_LogoutServer) //1�����ķ���ӦĿǰ��������
struct SQLogoutServer : public SLogoutServer
{ 
};

DECLARE_MSG(SCheckWhiteList, SCenterBaseMsg, SCenterBaseMsg::EPRO_CheckWhiteList) //��½�����������ķ�����
struct SQCheckWhiteList : public SCheckWhiteList
{
	DNID   dnidPlayerClient;
	char streamData[MAX_ACCAPASS];  //�˺�
	char  strpassword[MAX_ACCAPASS];//����
};

struct SACheckWhiteList : public SCheckWhiteList
{
	DNID   dnidPlayerClient;
	char streamData[MAX_ACCAPASS];  //�˺�
	char  strpassword[MAX_ACCAPASS];//����
	BOOL  bresult;
};



#endif






























