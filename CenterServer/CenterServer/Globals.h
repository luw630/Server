//////////////////////////////////////////////////////////////////////
/*
	Created:	2012/03/01
	Filename: 	Globals.h
	Author:		TonyJJ
	Commnet:	ȫ�����ݺʹ���
*/
//////////////////////////////////////////////////////////////////////
#pragma once

#ifndef _H_Globals_H_
#define _H_Globals_H_

#include <string>


struct PlayerData 
{

	PlayerData() : m_dDindClient( 0 ),
				   m_strPlayerName( "" ),
				   m_strPlayerAccount( "" )
	{

	}

	DNID    m_dDindClient;				//���Ψһȫ��ID
	std::string	m_strPlayerName;		//�������
	std::string m_strPlayerAccount;		//����˺�
};

struct ServerData
{

	ServerData() : m_strServerName( "" ),
				   m_strDescription( "" ),
				   m_dwIP( 0 ),
				   m_dwPort( 0 ),
				   m_dDindClient( 0 ),
				   m_dwIndex( 0 ),
				   m_dwGid( 0 ),
				   m_wTotalPlayers( 0 ),
				   m_dwNew( 0 ),
				   m_dwActivity( 0 ),
				   m_dwRush( 0 )
	{

	}

	std::string m_strServerName;		// ����������
	std::string m_strDescription;		// ������˵��
	DWORD   m_dwIP;                   // ���ڽ����ⲿ���ӵĵ�ַ
	DWORD   m_dwPort;                 // ���ڽ����ⲿ���ӵĶ˿�
	DNID    m_dDindClient;            //����ִ����Ψһ��ȫ��ID������ײ㣩
	DWORD   m_dwIndex;              //����������
	DWORD   m_dwGid;                    // ����ִ����Ψһ��ȫ��ID������������ȫ�֣�
	WORD    m_wTotalPlayers;          // ������������ϵ���Ҹ���
	DWORD	m_dwNew;					//�·�
	DWORD   m_dwActivity;				//�
	DWORD	m_dwRush;					//��ע

	void operator=( const ServerData& data )
	{
		m_strDescription = data.m_strDescription;
		m_strServerName = data.m_strServerName;
		m_dwGid = data.m_dwGid;
		m_dwIndex = data.m_dwIndex;
		m_dwIP = data.m_dwIP;
		m_dwPort = data.m_dwPort;
		m_dDindClient = data.m_dDindClient;
		m_wTotalPlayers = data.m_wTotalPlayers;
		m_dwNew = data.m_dwNew;
		m_dwActivity = data.m_dwActivity;
		m_dwRush = data.m_dwRush;
	}

};

//TODO:Tony Modify [2012-3-7]Comment:[������״̬����]
// struct ServerState
// {
// 	ServerState() : dwIndex( 0 ),
// 		dwNew( 0 ),
// 		dwActity( 0 ),
// 		dwRush( 0 ),
// 		dwTotalPlayers( 0 )
// 	{
// 
// 	}
// 	DWORD	dwIndex;
// 	DWORD	dwNew;
// 	DWORD	dwActity;
// 	DWORD	dwRush;
// 	DWORD	dwTotalPlayers;
// };
///////////////////////////////////////////////////////////////////






#endif











































