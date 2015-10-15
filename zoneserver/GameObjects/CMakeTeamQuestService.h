#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>
#include <set>
#include "networkmodule/TeamMsgs.h"

class CMakeTeamQuestService
{
public:
	typedef std::hash_map<DWORD, TRWithIndex> MTQuestList;
	typedef std::hash_map<DWORD, DWORD> HasPosTeamList;

public:
	CMakeTeamQuestService(){}

public:
	static CMakeTeamQuestService& GetInstance()
	{
		static CMakeTeamQuestService instance;
		return instance;
	}

	// 请求列表
	BOOL InsertQuest(TRWithIndex &tq);
	BOOL RemoveQuest(DWORD index);
	BOOL IsQuestExist(DWORD index);
	MTQuestList& GetQuestList();
	TRWithIndex *GetQuestDetail(DWORD index);

	// 队伍列表
	BOOL InsertEmptyTeam(DWORD teamID);
	BOOL RemoveUnJoinedTeam(DWORD teamID);
	BOOL IsEmptyTeamExist(DWORD teamID);
	DWORD GetEmptyTeamUpdateTime(DWORD teamID);
	BOOL UpdateEmptyTeam(DWORD teamID);
	HasPosTeamList& GetEmptyTeamList();

private:
	// 禁止拷贝构造和拷贝赋值
	CMakeTeamQuestService(CMakeTeamQuestService &);
	CMakeTeamQuestService& operator=(CMakeTeamQuestService &);

private:
	MTQuestList		m_MTList;
	HasPosTeamList	m_ETList;

	static DWORD g_TQIndex;	
};

#pragma warning(pop)