#include "StdAfx.h"
#include "EventRegion.h"

CEventRegion::CEventRegion(void) : CDynamicRegion()
{
	m_dwRegionStartTime = 0;
	m_dwRegionFinishTime = 0;
	m_nEventRegionEnd = 0;
}

CEventRegion::~CEventRegion(void)
{
}

void CEventRegion::OnRun()
{
	CRegion::OnRun();

	if(m_dwRegionStartTime==0) 
	{
		m_dwRegionStartTime=timeGetTime();
		rfalse("副本Index：%d, GID：%d，首次运行！开始时间：%d", m_DynamicIndex, m_GID, m_dwRegionStartTime);
		LuaFunctor(g_Script, "EventRegion_Init")[m_DynamicIndex][m_GID]();
	}

	DWORD dwRegionExistTime = timeGetTime() - m_dwRegionStartTime; //副本存在的时间
	//rfalse("CEventRegion::OnRun()副本存在的时间: %ld",dwRegionExistTime);
	if(m_nEventRegionEnd == 0)	//副本结束标志
	{
		LuaFunctor(g_Script, "EventRegion_Run")[m_DynamicIndex][m_GID][dwRegionExistTime]();
	}
	else
	{
		if(m_dwRegionFinishTime==0) m_dwRegionFinishTime=timeGetTime();
		DWORD dwRegionEndTime = timeGetTime() - m_dwRegionFinishTime; //从副本决定退出开始，过了多少时间

		LuaFunctor(g_Script, "EventRegion_End")[m_DynamicIndex][m_GID][dwRegionExistTime][dwRegionEndTime][m_nEventRegionEnd]();
// 		check_list<LPIObject>::iterator iter = m_PlayerList.begin();
// 		for ( iter; iter != m_PlayerList.end(); ++iter )
// 		{
// 			CPlayer *pPlayer = (CPlayer *)(*iter)->DynamicCast(IID_PLAYER);
// 			TalkToDnid(pPlayer->m_ClientIndex, "副本结束了哦");
// 			rfalse("副本结束了哦 %d", m_nEventRegionEnd);
// 		} 

	}

	//不知道什么用，暂时注释掉。。。。。。。。
	//if (m_DyRegionEnd != 0)
	//{
	//	m_bValid = false;
	//}

}

