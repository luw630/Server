//////////////////////////////////////////////////////////////////////
/*
	Created:	2012/03/01
	Filename: 	GameServer.h
	Author:		TonyJJ
	Commnet:	游戏服相关
*/
//////////////////////////////////////////////////////////////////////
#pragma once

#ifndef _H_GameServer_H_
#define _H_GameServer_H_

#include "Globals.h"
#include <map>
#include <string>

typedef std::map< DNID , PlayerData >			PlayerDnidMap;
typedef std::map< std::string , PlayerData >	PlayerStringMap;

class GameServer
{
public:
	GameServer( const ServerData& data );
	~GameServer(void);

public:
	//	###################################
	/*
	@brief			得到游戏服的Dnid
	@param []	
	*/
	//	###################################
	inline DNID GetServerDNID()
	{
		return m_ServerData.m_dDindClient;
	}

	//	###################################
	/*
	@brief			得到游戏服的名字
	@param []	
	*/
	//	###################################
	inline const std::string& GetServerName()
	{
		return m_ServerData.m_strServerName;
	}

	//	###################################
	/*
	@brief			得到游戏服的索引
	@param []	
	*/
	//	###################################
	inline DWORD GetServerIndex()
	{
		return m_ServerData.m_dwIndex;
	}

	inline ServerData* GetServerData()
	{
		return &m_ServerData;
	}

	//	###################################
	/*
	@brief			添加玩家数据
	@param [PlayerData]	
	*/
	//	###################################
	bool AddPlayer( PlayerData& data );

	//	###################################
	/*
	@brief			通过DNID删除玩家
	@param [DNID]	
	*/
	//	###################################
	bool RemovePlayerByDNID( DNID id );

	//	###################################
	/*
	@brief			通过名字删除玩家
	@param []	
	*/
	//	###################################
	bool RemovePlayerByName( std::string& name );

	//	###################################
	/*
	@brief			通过账号删除玩家
	@param []	
	*/
	//	###################################
	bool RemovePlayerByAccount( std::string& account );

	//	###################################
	/*
	@brief			通过DNID查找玩家
	@param []	
	*/
	//	###################################
	PlayerDnidMap::iterator FindPlayerByDNID( DNID id );
	bool FindPlayerByDNID( DNID id , PlayerData** data );

	//	###################################
	/*
	@brief			通过玩家名字查找玩家
	@param []	
	*/
	//	###################################
	//PlayerStringMap::iterator FindPlayerByName( std::string& name );
	bool FindPlayerByName( std::string& name , PlayerData** data );

	//	###################################
	/*
	@brief			通过玩家账号查询玩家
	@param []	
	*/
	//	###################################
	PlayerStringMap::iterator FindPlayerByAccount( std::string& account );
	bool FindPlayerByAccount( std::string& account , PlayerData** data );



protected:
	//	###################################
	/*
	@brief			删除玩家数据(内部操作)
	@param [PlayerData]	
	*/
	//	###################################
	void RemovePlayer( PlayerData& data );

	//	###################################
	/*
	@brief			清空玩家数据
	@param []	
	*/
	//	###################################
	void ClearAllPlayer();


	void CheckWhiteList();

protected:
	ServerData					m_ServerData;			//游戏服数据
	PlayerDnidMap				m_PlayerDnidmap;		//玩家map
	//PlayerStringMap				m_PlayerNameMap;
	PlayerStringMap				m_PlayerAccountMap;
	
};


#endif