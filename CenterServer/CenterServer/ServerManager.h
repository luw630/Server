//////////////////////////////////////////////////////////////////////
/*
	Created:	2012/03/01
	Filename: 	ServerManager.h
	Author:		TonyJJ
	Commnet:	游戏服管理器
*/
//////////////////////////////////////////////////////////////////////


#pragma once

#ifndef _H_ServerManager_H_
#define _H_ServerManager_H_

#include "NetWorkModule/NetProviderModule/NetProviderModule.h"
#include "GameServer.h"
#include <map>

#define MAX_MANAGE_SIZE 3
enum SERVERFLAG
{
	GAMESERVER_FLAG,
	LOGINERVER_FLAG,
	DBSERVER_FLAG,
};

typedef std::map< DNID , GameServer >			GameServerMap;

class ServerManager
{
public:
	ServerManager(void);
	~ServerManager(void);

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
	@brief			游戏服注册
	@param []	
	*/
	//	###################################
	BOOL LoginGameServer( DNID dnidClient , struct SQRegisterServer* pMsg );
	BOOL RegisterLoginServer(DNID dnidClient, struct SQRegisterLoginServer* pMsg);
	BOOL RegisterDBServer(DNID dnidClient, struct SQRegisterDBServer* pMsg);
	//	###################################
	/*
	@brief			游戏服注销
	@param []	
	*/
	//	###################################
	void LogoutGameServer( DNID dnidClinet ,BYTE bflag = GAMESERVER_FLAG);

	int OnServerconnectedFail(const char* strServerAlam);
public:
	//	###################################
	/*
	@brief			给指定的游戏服发送消息
	@param [DNID]	游戏服唯一索引
	@param [const void*]	消息指针
	@param [size_t]			消息大小
	*/
	//	###################################
	void BroadcastToServer( DNID id , const void* pMsg, size_t size );

	//	###################################
	/*
	@brief			给所有的游戏服发送消息
	@param [const void*]	消息指针
	@param [size_t]			消息大小
	*/
	//	###################################
	void BroadcastAllServer( const void* pMsg, size_t size );

	//	###################################
	/*
	@brief			给除了指定的服务器以外的所有游戏服发送消息
	@param [DNID]	游戏服唯一索引
	@param [const void*]	消息指针
	@param [size_t]			消息大小
	*/
	//	###################################
	void BroadcastButServer( DNID id , const void* pMsg , size_t size );

	//	###################################
	/*
	@brief			添加游戏服
	@param []	
	*/
	//	###################################
	bool AddGameServer( GameServer& server ,BYTE bserverflag);

	//	###################################
	/*
	@brief			移除一个游戏服
	@param []	
	*/
	//	###################################
	bool RemoveGameServerByDnid(  DNID dnidClient,BYTE bflag );

	//	###################################
	/*
	@brief			移除所有游戏服
	@param []	
	*/
	//	###################################
	void RemoveAllGameServer();

	//	###################################
	/*
	@brief			通过DNID查找游戏服
	@param [DNID]	游戏服唯一索引
	@param [GameServer*]	游戏服数据
	*/
	//	###################################
	GameServerMap::iterator FindGameServerByDNID( DNID id );
	bool FindGameServerByDNID( DNID id , GameServer** server );

	//	###################################
	/*
	@brief			通过游戏服编号查找游戏服
	@param [DWORD]	游戏服编号
	@param [GameServer*]	游戏服数据
	*/
	//	###################################
	GameServerMap::iterator FindGameServerByIndex( DWORD id );
	bool FindGameServerByIndex( DWORD id , GameServer** server );


	//	###################################
	/*
	@brief			通过游戏服名字查找游戏服
	@param [std::string&]	游戏服名字
	@param [GameServer**]	游戏服数据
	*/
	//	###################################
	GameServerMap::iterator FindGameServerByName( std::string& gameServerStr );
	bool FindGameServerByName( std::string& gameServerStr , GameServer** server );

	//	###################################
	/*
	@brief			得到服务器数据包
	@param [const char*]	文件名
	*/
	//	###################################
	bool GetGameServerList( struct SARequestServerState* pMsg );

public:

protected:
	CNetProviderModule			m_ServerModule;					//服务器Socket
	GameServerMap				m_GameServerMap[MAX_MANAGE_SIZE];				//服务器列表
	DWORD		m_dZonsServerPlayerNum; //已经连接到区域服的玩家
};


#endif





































