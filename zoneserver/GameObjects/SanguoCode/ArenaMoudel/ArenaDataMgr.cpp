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
	//ArenaDataMgr唯一需要调用ArenaMoudel的接口，1.保留下线时的数据，2.对手如果未释放，则释放对手
	//这里与ArenaMoudel出现相互调用，可以通过监听析构来解耦，优化时在做处理
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


///@brief 获取上次挑战时间
__int64 CArenaDataMgr::GetLastChallengeTime()
{
	__int64 * timePtr = (__int64 *)m_pArenaData->m_LastChallengeTime;
	return *timePtr;
}

///@brief 设置上次挑战时间
void CArenaDataMgr::SetLastChallengeTime(__int64 lastChallengeTime)
{
	ConverTool::ConvertInt64ToBytes(lastChallengeTime, m_pArenaData->m_LastChallengeTime); //设置时间
}