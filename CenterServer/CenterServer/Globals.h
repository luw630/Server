//////////////////////////////////////////////////////////////////////
/*
	Created:	2012/03/01
	Filename: 	Globals.h
	Author:		TonyJJ
	Commnet:	全局数据和处理
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

	DNID    m_dDindClient;				//玩家唯一全局ID
	std::string	m_strPlayerName;		//玩家名字
	std::string m_strPlayerAccount;		//玩家账号
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

	std::string m_strServerName;		// 服务器名字
	std::string m_strDescription;		// 服务器说明
	DWORD   m_dwIP;                   // 用于接受外部联接的地址
	DWORD   m_dwPort;                 // 用于接受外部联接的端口
	DNID    m_dDindClient;            //本次执行中唯一的全局ID（网络底层）
	DWORD   m_dwIndex;              //服务器索引
	DWORD   m_dwGid;                    // 本次执行中唯一的全局ID（整个服务器全局）
	WORD    m_wTotalPlayers;          // 在这个服务器上的玩家个数
	DWORD	m_dwNew;					//新服
	DWORD   m_dwActivity;				//活动
	DWORD	m_dwRush;					//抢注

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

//TODO:Tony Modify [2012-3-7]Comment:[服务器状态数据]
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











































