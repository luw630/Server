//////////////////////////////////////////////////////////////////////
/*
	Created:	2012/03/02
	Filename: 	CenterMsg.h
	Author:		TonyJJ
	Commnet:	中心服消息
*/
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _H_CenterMsg_H_
#define _H_CenterMsg_H_

#include "NetModule.h"
#include "..\PUB\ConstValue.h"
DECLARE_MSG_MAP( SCenterBaseMsg , SMessage , SMessage::EPRO_Center_MESSAGE )
EPRO_RequestClientVersionInfo,				//请求客户端版本信息
EPRO_RequestServerState,						//请求服务器状态
EPRO_RegisterServer,							//注册游戏服到中心服
EPRO_LoginPlayer,								//玩家成功登陆到区域服后，注册到中心服
EPRO_LogoutPlayer,								//玩家断开和区域服的连接后，注销中心服的玩家数据
EPRO_RequestServerDetailInfo,					//请求一个服务器的详细信息
EPRO_RegisterLoginServer,						//注册登陆服到中心服   2015/6/11  决定把登陆服务器以及数据库服务器同时注册到中心服监控
EPRO_RegisterDBServer,						//注册登陆服到中心服
EPRO_LogoutServer,							//服务器从中心服中退出
EPRO_CheckWhiteList,							//中心服检查白名单。因为中心服控制所有服务器
END_MSG_MAP()

//客户端启动的时候向中心服请求服务器状态
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

//玩家向中心服请求一个服务器的详细信息
DECLARE_MSG( SRequestServerDetailInfoMsg , SCenterBaseMsg , SCenterBaseMsg::EPRO_RequestServerDetailInfo )
struct SQRequestServeDetailInfoMsg : public SRequestServerDetailInfoMsg 
{
	DWORD	dwIndex; // 服务器索引
};

struct SARequestServeDetailInfoMsg : public SRequestServerDetailInfoMsg 
{
	DWORD	dwIndex; // 服务器索引
	char streamData[512]; // 服务器名
};


//游戏服启动向中心服注册服务器信息
DECLARE_MSG( SRegisterServer , SCenterBaseMsg , SCenterBaseMsg::EPRO_RegisterServer )
struct SQRegisterServer : public SRegisterServer
{
	unsigned long dwIP; // IP
	unsigned long dwPort; // 端口号
	unsigned long index;
	unsigned long _dNew;
	unsigned long _dActivity;
	unsigned long _dRush;
	char streamData[512];	// 账号&密码
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

//注册登陆服务器
DECLARE_MSG(SRegisterLoginServer, SCenterBaseMsg, SCenterBaseMsg::EPRO_RegisterLoginServer)
struct SQRegisterLoginServer : public SRegisterLoginServer
{
	unsigned long dwIP; // IP
	unsigned long dwPort; // 端口号
	unsigned long index;
	unsigned long _dNew;
	unsigned long _dActivity;
	unsigned long _dRush;
	char streamservername[128]; //服务器名称
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


//注册数据库服务器
DECLARE_MSG(SRegisterDBServer, SCenterBaseMsg, SCenterBaseMsg::EPRO_RegisterDBServer)
struct SQRegisterDBServer : public SRegisterDBServer
{
	unsigned long dwIP; // IP
	unsigned long dwPort; // 端口号
	unsigned long index;
	unsigned long _dNew;
	unsigned long _dActivity;
	unsigned long _dRush;
	char streamservername[512];	// 账号&密码
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


DECLARE_MSG(SLogoutServer, SCenterBaseMsg, SCenterBaseMsg::EPRO_LogoutServer) //1个中心服对应目前服务器组
struct SQLogoutServer : public SLogoutServer
{ 
};

DECLARE_MSG(SCheckWhiteList, SCenterBaseMsg, SCenterBaseMsg::EPRO_CheckWhiteList) //登陆服务器向中心服请求
struct SQCheckWhiteList : public SCheckWhiteList
{
	DNID   dnidPlayerClient;
	char streamData[MAX_ACCAPASS];  //账号
	char  strpassword[MAX_ACCAPASS];//密码
};

struct SACheckWhiteList : public SCheckWhiteList
{
	DNID   dnidPlayerClient;
	char streamData[MAX_ACCAPASS];  //账号
	char  strpassword[MAX_ACCAPASS];//密码
	BOOL  bresult;
};



#endif






























