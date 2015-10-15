#include "stdafx.h"
#include "ArenaDataMgr.h"
#include"..\BaseDataManager.h"
#include "ArenaDataMgr.h"
#include "ArenaMoudel.h"
#include "..\Common\PubTool.h"

CArenaDataMgr::CArenaDataMgr(CBaseDataManager& baseDataMgr)
	:CExtendedDataManager(baseDataMgr)
{

}


CArenaDataMgr::~CArenaDataMgr()
{
}

bool CArenaDataMgr::InitDataMgr(void * pData)
{
	m_pArenaData = static_cast<SArenaData*>(pData);

	return true;
}

bool CArenaDataMgr::ReleaseDataMgr()
{

	//m_pCurrentCompetitor = nullptr;
	//ArenaDataMgrΨһ��Ҫ����ArenaMoudel�Ľӿڣ�1.��������ʱ�����ݣ�2.�������δ�ͷţ����ͷŶ���
	//������ArenaMoudel�����໥���ã�����ͨ����������������Ż�ʱ��������
	//if (nullptr != m_pCurrentCompetitor)
	//	CArenaMoudel::getSingleton()->UnlockCompetitor(m_pCurrentCompetitor->m_AernaID);
	return true;
}

int CArenaDataMgr::GetBestRank()
{
	return m_pArenaData->m_BestRank;
}


void CArenaDataMgr::SetBestRank(int value)
{
	m_pArenaData->m_BestRank = value;
}


///@brief ��ȡ�ϴ���սʱ��
__int64 CArenaDataMgr::GetLastChallengeTime()
{
	__int64 * timePtr = (__int64 *)m_pArenaData->m_LastChallengeTime;
	return *timePtr;
}

///@brief �����ϴ���սʱ��
void CArenaDataMgr::SetLastChallengeTime(__int64 lastChallengeTime)
{
	ConverTool::ConvertInt64ToBytes(lastChallengeTime, m_pArenaData->m_LastChallengeTime); //����ʱ��
}