//////////////////////////////////////////////////////////////////////
/*
	Created:	2012/03/01
	Filename: 	PlayerManager.h
	Author:		TonyJJ
	Commnet:	��ҹ����������ڸ���ҷ������з�������״̬��������յ����ݺ󣬶Ͽ������ķ������ӣ�
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
	@brief			������Ϣ����ҿͻ���
	@param [DNID]	��Ϸ��Ψһ����
	@param [const void*]	��Ϣָ��
	@param [size_t]			��Ϣ��С
	*/
	//	###################################
	void SendToClient( DNID dnidClient , const void* pMsg, size_t size );


protected:
	CNetProviderModule			m_ServerModule;					//������Socket
	DWORD    m_dPlayerConnectNum;
};

#endif
