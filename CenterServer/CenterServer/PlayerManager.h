//////////////////////////////////////////////////////////////////////
/*
	Created:	2012/03/01
	Filename: 	PlayerManager.h
	Author:		TonyJJ
	Commnet:	玩家管理器（用于给玩家发送所有服务器的状态，玩家在收到数据后，断开和中心服的连接）
*/
//////////////////////////////////////////////////////////////////////
#pragma once

#ifndef _H_PlayerManager_H_
#define _H_PlayerManager_H_
#include "NetWorkModule/NetProviderModule/NetProviderModule.h"

class PlayerManager
{
public:
	PlayerManager(void);
	~PlayerManager(void);

public:
	//	###################################
	/*
	@brief		创建主机
	@param [LPCSTR]		端口字符串
	*/
	//	###################################
	BOOL Host( LPCSTR port );

	//	###################################
	/*
	@brief		更新
	@param []				
	*/
	//	###################################
	void Execution();

	//	###################################
	/*
	@brief		销毁
	@param []				
	*/
	//	###################################
	void Destroy();

	//	###################################
	/*
	@brief			网络信息更新和处理
	@param [DNID]	游戏服唯一索引
	@param [LINKSTATUS]	连接状态
	@param [void*]	消息指针
	@param [size_t]	消息大小
	*/
	//	###################################
	bool Entry( DNID dnidClient, LINKSTATUS enumStatus, void *data, size_t size ); 

	//	###################################
	/*
	@brief			发送消息给玩家客户单
	@param [DNID]	游戏服唯一索引
	@param [const void*]	消息指针
	@param [size_t]			消息大小
	*/
	//	###################################
	void SendToClient( DNID dnidClient , const void* pMsg, size_t size );


protected:
	CNetProviderModule			m_ServerModule;					//服务器Socket
	DWORD    m_dPlayerConnectNum;
};

#endif
