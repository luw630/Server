#include "StdAfx.h"
#include ".\gameserver.h"

GameServer::GameServer(const ServerData& data)
{
	m_ServerData = data;
}

GameServer::~GameServer(void)
{
	ClearAllPlayer();
}

bool GameServer::AddPlayer( PlayerData& data )
{
	PlayerDnidMap::_Pairib b1 = m_PlayerDnidmap.insert( PlayerDnidMap::value_type( data.m_dDindClient , data ) );
	//PlayerStringMap::_Pairib b2 = m_PlayerNameMap.insert( PlayerStringMap::value_type( data.m_strPlayerName , data ) );
	PlayerStringMap::_Pairib b3 = m_PlayerAccountMap.insert( PlayerStringMap::value_type( data.m_strPlayerAccount , data ) );
	if ( b1.second && b3.second )
	{
		++m_ServerData.m_wTotalPlayers;
		return true;
	}
	else
	{
		if ( b1.second )
		{
			m_PlayerDnidmap.erase( data.m_dDindClient );
		}

		//if ( b2.second )
		//{
		//	m_PlayerNameMap.erase( data.m_strPlayerName );
		//}

		if ( b3.second )
		{
			m_PlayerAccountMap.erase( data.m_strPlayerAccount );
		}

	}
	return false;
}

void GameServer::RemovePlayer( PlayerData& data )
{
	DNID id = data.m_dDindClient;
	std::string account = data.m_strPlayerAccount;
	m_PlayerDnidmap.erase( id );
	//m_PlayerNameMap.erase( data.m_strPlayerName );
	m_PlayerAccountMap.erase( account );
	--m_ServerData.m_wTotalPlayers;
}

void GameServer::ClearAllPlayer()
{
	m_PlayerAccountMap.clear();
	m_PlayerDnidmap.clear();
	//m_PlayerNameMap.clear();
}

void GameServer::CheckWhiteList()
{
	
}

bool GameServer::RemovePlayerByDNID(DNID id)
{
	PlayerDnidMap::iterator it = m_PlayerDnidmap.find( id );
	if ( it != m_PlayerDnidmap.end() )
	{
		RemovePlayer( it->second );
		return true;
	}
	return false;
}

bool GameServer::RemovePlayerByAccount( std::string& account )
{
	PlayerStringMap::iterator it = m_PlayerAccountMap.find( account );
	if ( it != m_PlayerAccountMap.end() )
	{
		RemovePlayer( it->second );
		return true;
	}
	return false;
}

bool GameServer::RemovePlayerByName( std::string& name )
{
	//PlayerStringMap::iterator it = m_PlayerNameMap.find( name );
	//if ( it != m_PlayerNameMap.end() )
	//{
	//	RemovePlayer( it->second );
	//	return true;
	//}
	
	return false;
}

PlayerDnidMap::iterator GameServer::FindPlayerByDNID( DNID id )
{
	PlayerDnidMap::iterator it = m_PlayerDnidmap.find( id );
	if ( it != m_PlayerDnidmap.end() )
	{
		return it;
	}
	//return NULL;
}

bool GameServer::FindPlayerByDNID( DNID id , PlayerData** data )
{
	PlayerDnidMap::iterator it = m_PlayerDnidmap.find( id );
	if ( it != m_PlayerDnidmap.end() )
	{
		*data = &(it->second);
		return true;
	}
	return false;
}

PlayerStringMap::iterator GameServer::FindPlayerByAccount( std::string& account )
{
	PlayerStringMap::iterator it = m_PlayerAccountMap.find( account );
	if ( it != m_PlayerAccountMap.end() )
	{
		return it;
	}
	//return NULL;
}

bool GameServer::FindPlayerByAccount( std::string& account , PlayerData** data )
{
	PlayerStringMap::iterator it = m_PlayerAccountMap.find( account );
	if ( it != m_PlayerAccountMap.end() )
	{
		*data = &( it->second );
		return true;
	}
	return false;
}

//PlayerStringMap::iterator GameServer::FindPlayerByName( std::string& name )
//{
//	PlayerStringMap::iterator it = m_PlayerNameMap.find( name );
//	if ( it != m_PlayerNameMap.end() )
//	{
//		return it;
//	}
//	//return NULL;
//}

bool GameServer::FindPlayerByName( std::string& name , PlayerData** data )
{
	//PlayerStringMap::iterator it = m_PlayerNameMap.find( name );
	//if ( it != m_PlayerNameMap.end() )
	//{
	//	*data = &( it->second );
	//	return true;
	//}
	return false;
}

























