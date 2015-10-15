//////////////////////////////////////////////////////////////////////
/*
	Created:	2012/03/01
	Filename: 	ServerManager.h
	Author:		TonyJJ
	Commnet:	��Ϸ��������
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
	@brief		��������
	@param [LPCSTR]		�˿��ַ���
	*/
	//	###################################
	BOOL Host( LPCSTR port );

	//	###################################
	/*
	@brief		����
	@param []				
	*/
	//	###################################
	void Execution();

	//	###################################
	/*
	@brief		����
	@param []				
	*/
	//	###################################
	void Destroy();

	//	###################################
	/*
	@brief			������Ϣ���ºʹ���
	@param [DNID]	��Ϸ��Ψһ����
	@param [LINKSTATUS]	����״̬
	@param [void*]	��Ϣָ��
	@param [size_t]	��Ϣ��С
	*/
	//	###################################
	bool Entry( DNID dnidClient, LINKSTATUS enumStatus, void *data, size_t size ); 

	//	###################################
	/*
	@brief			��Ϸ��ע��
	@param []	
	*/
	//	###################################
	BOOL LoginGameServer( DNID dnidClient , struct SQRegisterServer* pMsg );
	BOOL RegisterLoginServer(DNID dnidClient, struct SQRegisterLoginServer* pMsg);
	BOOL RegisterDBServer(DNID dnidClient, struct SQRegisterDBServer* pMsg);
	//	###################################
	/*
	@brief			��Ϸ��ע��
	@param []	
	*/
	//	###################################
	void LogoutGameServer( DNID dnidClinet ,BYTE bflag = GAMESERVER_FLAG);

	int OnServerconnectedFail(const char* strServerAlam);
public:
	//	###################################
	/*
	@brief			��ָ������Ϸ��������Ϣ
	@param [DNID]	��Ϸ��Ψһ����
	@param [const void*]	��Ϣָ��
	@param [size_t]			��Ϣ��С
	*/
	//	###################################
	void BroadcastToServer( DNID id , const void* pMsg, size_t size );

	//	###################################
	/*
	@brief			�����е���Ϸ��������Ϣ
	@param [const void*]	��Ϣָ��
	@param [size_t]			��Ϣ��С
	*/
	//	###################################
	void BroadcastAllServer( const void* pMsg, size_t size );

	//	###################################
	/*
	@brief			������ָ���ķ����������������Ϸ��������Ϣ
	@param [DNID]	��Ϸ��Ψһ����
	@param [const void*]	��Ϣָ��
	@param [size_t]			��Ϣ��С
	*/
	//	###################################
	void BroadcastButServer( DNID id , const void* pMsg , size_t size );

	//	###################################
	/*
	@brief			�����Ϸ��
	@param []	
	*/
	//	###################################
	bool AddGameServer( GameServer& server ,BYTE bserverflag);

	//	###################################
	/*
	@brief			�Ƴ�һ����Ϸ��
	@param []	
	*/
	//	###################################
	bool RemoveGameServerByDnid(  DNID dnidClient,BYTE bflag );

	//	###################################
	/*
	@brief			�Ƴ�������Ϸ��
	@param []	
	*/
	//	###################################
	void RemoveAllGameServer();

	//	###################################
	/*
	@brief			ͨ��DNID������Ϸ��
	@param [DNID]	��Ϸ��Ψһ����
	@param [GameServer*]	��Ϸ������
	*/
	//	###################################
	GameServerMap::iterator FindGameServerByDNID( DNID id );
	bool FindGameServerByDNID( DNID id , GameServer** server );

	//	###################################
	/*
	@brief			ͨ����Ϸ����Ų�����Ϸ��
	@param [DWORD]	��Ϸ�����
	@param [GameServer*]	��Ϸ������
	*/
	//	###################################
	GameServerMap::iterator FindGameServerByIndex( DWORD id );
	bool FindGameServerByIndex( DWORD id , GameServer** server );


	//	###################################
	/*
	@brief			ͨ����Ϸ�����ֲ�����Ϸ��
	@param [std::string&]	��Ϸ������
	@param [GameServer**]	��Ϸ������
	*/
	//	###################################
	GameServerMap::iterator FindGameServerByName( std::string& gameServerStr );
	bool FindGameServerByName( std::string& gameServerStr , GameServer** server );

	//	###################################
	/*
	@brief			�õ����������ݰ�
	@param [const char*]	�ļ���
	*/
	//	###################################
	bool GetGameServerList( struct SARequestServerState* pMsg );

public:

protected:
	CNetProviderModule			m_ServerModule;					//������Socket
	GameServerMap				m_GameServerMap[MAX_MANAGE_SIZE];				//�������б�
	DWORD		m_dZonsServerPlayerNum; //�Ѿ����ӵ�����������
};


#endif





































