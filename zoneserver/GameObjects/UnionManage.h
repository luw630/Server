#pragma once

#include "networkmodule\playertypedef.h"
#include "networkmodule\UnionMsgs.h"

class UnionDataManager
{
public:
    // 结义数据
    std::map< std::string, SPlayerUnionData > m_UnionDataMap;
    std::map< DWORD, std::string > unionMappedMap;

    bool	m_bIsRefused;
	std::string  m_errorInfo;
    UnionDataManager();

public:
    BOOL LoadUnionData( SAGetUnionDataMsg *pMsg );
    BOOL SaveUnionData();

public:
	BOOL IsUnion( DWORD playerID );
	void DeletePlayerIDFromPlayerIDMap( DWORD playerID );
	std::string GetUnionNameByPlayerID( DWORD playerID );
	SPlayerUnionData* GetUnionDataByPlayerID( DWORD playerID );
	int GetUnionMemberNumByPlayerID( DWORD playerID );

	BOOL IsHaveUnion( std::string unionName );
	SPlayerUnionData* GetUnionDataByUnionName( std::string unionName );
	int GetUnionMemberNumByUnionName( std::string unionName );
	void DeleteUnionDataByUnionName( std::string unionName );

	/// Interface
	BOOL CreateUnion( LPCSTR title, LPCSTR suffx );
	BOOL AddPlayerToUnion( DWORD playerID, std::string unionName );
	BOOL DeletePlayerToUnion( DWORD playerID, DNID clientID = 0 );

    void AddPlayerExp( CPlayer *p, DWORD addVal );
    void ReceiveExp( DWORD sid );                   // 经验领取

    void SendUnionAllMsg( SPlayerUnionData *p, void *pMsg, size_t size );
    void TalkToUnionMember( std::string key, LPCSTR info );

    void Run();
public:

    // 外部查询接口
    void ShowPlayerTitle( DWORD sid );
    void ShowUnionInfo( LPCSTR key );

public:
	void CreateUnionAddPlayer( LPCSTR title, LPCSTR suffx, DWORD id1, DWORD id2 );

public:
	void UnionOnline( struct SUnionOnlineMsg *pMsg, DWORD playerID );
	void UnionOnline( CPlayer *pPlayer );
	void UpdateUnionMember( DWORD destID, int idx = -1 );
    void CalcAmuck( CPlayer* pPlayer );
	
};