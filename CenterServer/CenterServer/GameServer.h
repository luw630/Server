//////////////////////////////////////////////////////////////////////
/*
	Created:	2012/03/01
	Filename: 	GameServer.h
	Author:		TonyJJ
	Commnet:	��Ϸ�����
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
	@brief			�õ���Ϸ����Dnid
	@param []	
	*/
	//	###################################
	inline DNID GetServerDNID()
	{
		return m_ServerData.m_dDindClient;
	}

	//	###################################
	/*
	@brief			�õ���Ϸ��������
	@param []	
	*/
	//	###################################
	inline const std::string& GetServerName()
	{
		return m_ServerData.m_strServerName;
	}

	//	###################################
	/*
	@brief			�õ���Ϸ��������
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
	@brief			����������
	@param [PlayerData]	
	*/
	//	###################################
	bool AddPlayer( PlayerData& data );

	//	###################################
	/*
	@brief			ͨ��DNIDɾ�����
	@param [DNID]	
	*/
	//	###################################
	bool RemovePlayerByDNID( DNID id );

	//	###################################
	/*
	@brief			ͨ������ɾ�����
	@param []	
	*/
	//	###################################
	bool RemovePlayerByName( std::string& name );

	//	###################################
	/*
	@brief			ͨ���˺�ɾ�����
	@param []	
	*/
	//	###################################
	bool RemovePlayerByAccount( std::string& account );

	//	###################################
	/*
	@brief			ͨ��DNID�������
	@param []	
	*/
	//	###################################
	PlayerDnidMap::iterator FindPlayerByDNID( DNID id );
	bool FindPlayerByDNID( DNID id , PlayerData** data );

	//	###################################
	/*
	@brief			ͨ��������ֲ������
	@param []	
	*/
	//	###################################
	//PlayerStringMap::iterator FindPlayerByName( std::string& name );
	bool FindPlayerByName( std::string& name , PlayerData** data );

	//	###################################
	/*
	@brief			ͨ������˺Ų�ѯ���
	@param []	
	*/
	//	###################################
	PlayerStringMap::iterator FindPlayerByAccount( std::string& account );
	bool FindPlayerByAccount( std::string& account , PlayerData** data );



protected:
	//	###################################
	/*
	@brief			ɾ���������(�ڲ�����)
	@param [PlayerData]	
	*/
	//	###################################
	void RemovePlayer( PlayerData& data );

	//	###################################
	/*
	@brief			����������
	@param []	
	*/
	//	###################################
	void ClearAllPlayer();


	void CheckWhiteList();

protected:
	ServerData					m_ServerData;			//��Ϸ������
	PlayerDnidMap				m_PlayerDnidmap;		//���map
	//PlayerStringMap				m_PlayerNameMap;
	PlayerStringMap				m_PlayerAccountMap;
	
};


#endif