#include "StdAfx.h"
#include "dynamicregion.h"
#include "ScriptManager.h"

extern CScriptManager g_Script;

DWORD CDynamicRegion::s_DynamicRegionNumber = 0;

CDynamicRegion::CDynamicRegion(void) 
{
}

CDynamicRegion::~CDynamicRegion(void)
{
	rfalse(2, 1, "��̬����%d�����١�����", GetGID());
}

void CDynamicRegion::OnRun(void)
{
	CRegion::OnRun();
// 	if (m_PlayerList.size()>0)
// 	{
// 		if (m_CanMoveOut && (timeGetTime()-m_MoveOutTimeleft >= 1000))
// 		{
// 			m_MoveOutTimeleft=timeGetTime();
// 			check_list<LPIObject>::iterator iter = m_PlayerList.begin();
// 			while (iter != m_PlayerList.end())
// 			{
// 				CPlayer *pPlayer = (CPlayer *)(*iter)->DynamicCast(IID_PLAYER);
// 				if (pPlayer)
// 				{
// 					char str[256];
// 					sprintf(str,"�㻹��%d�뽫�뿪������",m_MoveOutOT/1000-m_MoveOutTime);
// 					TalkToDnid(pPlayer->m_ClientIndex,str);
// 				}
// 				iter++;
// 			}
// 			m_MoveOutTime++;
// 		}
// 
// 	}
// 	if (m_CanMoveOut && (timeGetTime()-m_MoveOutTimeSeg >= m_MoveOutOT))
// 	{
// 		m_CanMoveOut = false;
// 
// 		rfalse("��̬����%d���Ѿ�û�л�һ�ι��׼��ִ�г���ת����...", GetGID());
// 		LuaFunctor(g_Script, "OnDRAllMonsterDead")[GetGID()][m_DynamicIndex]();
// 	}
}

int CDynamicRegion::OnCreate(_W64 long pParameter)
{
    s_DynamicRegionNumber++;

	m_CopySceneGID		= 0;
	m_prevRegID			= 0;
	m_prevX				= 0;
	m_prevY				= 0;
	m_playerID			= 0;
	m_DynamicIndex		= 0;
	m_CanMoveOut		= false;
	m_MoveOutTimeSeg	= 0;
	m_MoveOutOT			= 0;
	m_MoveOutTimeleft	= 0;
	m_MoveOutTime		= 0;
	m_DyRegionEnd		= 0;
    return CRegion::OnCreate(pParameter);
}

void CDynamicRegion::OnClose()
{
    s_DynamicRegionNumber--;

    return CRegion::OnClose();
}

int CDynamicRegion::AddObject(LPIObject pChild)
{
	return CRegion::AddObject(pChild);
}

int CDynamicRegion::DelMonster(DWORD dwGID)
{
	int ret = CRegion::DelMonster(dwGID);
	return ret;
}

int CDynamicRegion::DelObject(LPIObject pChild)
{
	int ret = CRegion::DelObject(pChild);

	if (0 == m_PlayerList.size() && m_DyRegionEnd != 0)
	{
		// ����Ǹ�����̬��������ô���������ͼ��
		if (m_CopySceneGID != 0)
			LuaFunctor(g_Script, "CS_OnCheckTerminate")[m_CopySceneGID]();
		else
			m_bValid = false;
	}

    return ret;
}