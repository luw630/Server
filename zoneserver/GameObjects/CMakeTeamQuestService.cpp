#include "Stdafx.h"
#include "CMakeTeamQuestService.h"

DWORD CMakeTeamQuestService::g_TQIndex = 1;

BOOL CMakeTeamQuestService::InsertQuest(TRWithIndex &tq)
{
	tq.dwIndex = g_TQIndex++;

	MTQuestList::iterator hasOne = m_MTList.find(tq.dwIndex);
	MY_ASSERT(hasOne == m_MTList.end());
	
	m_MTList[tq.dwIndex] = tq;

	return TRUE;
}

BOOL CMakeTeamQuestService::RemoveQuest(DWORD index)
{
	if (0 == index)
		return FALSE;

	MTQuestList::iterator hasOne = m_MTList.find(index);
	MY_ASSERT(hasOne != m_MTList.end());

	m_MTList.erase(index);

	return TRUE;
}

BOOL CMakeTeamQuestService::IsQuestExist(DWORD index)
{
	if (0 == index)
		return FALSE;

	MTQuestList::iterator it = m_MTList.find(index);

	return (it != m_MTList.end()) ? TRUE : FALSE;
}

CMakeTeamQuestService::MTQuestList& CMakeTeamQuestService::GetQuestList()
{
	return m_MTList;
}

TRWithIndex* CMakeTeamQuestService::GetQuestDetail(DWORD index)
{
	if (0 == index)
		return 0;

	MTQuestList::iterator it = m_MTList.find(index);
	
	if (it != m_MTList.end())
		return &it->second;
	else
		return 0;
}

BOOL CMakeTeamQuestService::InsertEmptyTeam(DWORD teamID)
{
	if (0 == teamID)
		return FALSE;

	HasPosTeamList::iterator hasOne = m_ETList.find(teamID);
	//MY_ASSERT(hasOne == m_ETList.end());
	
	m_ETList[teamID] = timeGetTime();

	return TRUE;
}

BOOL CMakeTeamQuestService::RemoveUnJoinedTeam(DWORD teamID)
{
	if (0 == teamID)
		return FALSE;

	HasPosTeamList::iterator hasOne = m_ETList.find(teamID);
	MY_ASSERT(hasOne != m_ETList.end());

	m_ETList.erase(teamID);

	return TRUE;
}

BOOL CMakeTeamQuestService::IsEmptyTeamExist(DWORD teamID)
{
	if (0 == teamID)
		return FALSE;

	HasPosTeamList::iterator hasOne = m_ETList.find(teamID);

	return (hasOne != m_ETList.end()) ? TRUE : FALSE;
}

DWORD CMakeTeamQuestService::GetEmptyTeamUpdateTime(DWORD teamID)
{
	if (teamID == 0)
	{
		return 0;
	}
	MY_ASSERT(teamID);

	HasPosTeamList::iterator hasOne = m_ETList.find(teamID);
	MY_ASSERT(hasOne != m_ETList.end());

	return m_ETList[teamID];
}

BOOL CMakeTeamQuestService::UpdateEmptyTeam(DWORD teamID)
{
	if (teamID == 0)
	{
		return false;
	}
	MY_ASSERT(teamID);

	HasPosTeamList::iterator hasOne = m_ETList.find(teamID);
	//MY_ASSERT(hasOne != m_ETList.end());

	m_ETList[teamID] = timeGetTime();

	return TRUE;
}

CMakeTeamQuestService::HasPosTeamList & CMakeTeamQuestService::GetEmptyTeamList()
{
	return m_ETList;
}