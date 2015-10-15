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
		rfalse("����Index��%d, GID��%d���״����У���ʼʱ�䣺%d", m_DynamicIndex, m_GID, m_dwRegionStartTime);
		LuaFunctor(g_Script, "EventRegion_Init")[m_DynamicIndex][m_GID]();
	}

	DWORD dwRegionExistTime = timeGetTime() - m_dwRegionStartTime; //�������ڵ�ʱ��
	//rfalse("CEventRegion::OnRun()�������ڵ�ʱ��: %ld",dwRegionExistTime);
	if(m_nEventRegionEnd == 0)	//����������־
	{
		LuaFunctor(g_Script, "EventRegion_Run")[m_DynamicIndex][m_GID][dwRegionExistTime]();
	}
	else
	{
		if(m_dwRegionFinishTime==0) m_dwRegionFinishTime=timeGetTime();
		DWORD dwRegionEndTime = timeGetTime() - m_dwRegionFinishTime; //�Ӹ��������˳���ʼ�����˶���ʱ��

		LuaFunctor(g_Script, "EventRegion_End")[m_DynamicIndex][m_GID][dwRegionExistTime][dwRegionEndTime][m_nEventRegionEnd]();
// 		check_list<LPIObject>::iterator iter = m_PlayerList.begin();
// 		for ( iter; iter != m_PlayerList.end(); ++iter )
// 		{
// 			CPlayer *pPlayer = (CPlayer *)(*iter)->DynamicCast(IID_PLAYER);
// 			TalkToDnid(pPlayer->m_ClientIndex, "����������Ŷ");
// 			rfalse("����������Ŷ %d", m_nEventRegionEnd);
// 		} 

	}

	//��֪��ʲô�ã���ʱע�͵�����������������
	//if (m_DyRegionEnd != 0)
	//{
	//	m_bValid = false;
	//}

}

