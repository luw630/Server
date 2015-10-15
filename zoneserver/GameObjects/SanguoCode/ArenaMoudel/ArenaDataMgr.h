#pragma once
#include "Networkmodule\SanguoPlayerMsg.h"
#include"..\ExtendedDataManager.h"
#include "stdint.h"
//class SHeroData;
//class SArenaData;
//class SArenaCompetitor;
//class CBaseDataManager;
class CArenaDataMgr : public CExtendedDataManager
{
public:
	CArenaDataMgr(CBaseDataManager& baseDataMgr);
	~CArenaDataMgr();
	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();

	///@brief 获取防御阵容
	void GetDefenseTeamInfo(SHeroData* pHeroDataList);
	///@brief 更新防御阵容
	void UpdateDefenseTeamInfo();

	///@brief 获取当前对手
	//SArenaCompetitor * GetCompetitor() { return m_pCurrentCompetitor; }
	///@brief 设置当前对手
	//void SetCompetitor(SArenaCompetitor *pCompetitor){ m_pCurrentCompetitor = pCompetitor; }

	const SHeroData * GetDefenseTeamInfo(const int defensiveTeamID[]);
	
	///@brief 获取设置排名
	int GetRank(){ return m_Rank; }
	void SetRank(int rank) { m_Rank = rank; }
	
	///@brief 获取最佳排名
	int GetBestRank();
	///@brief 设置最佳排名
	void SetBestRank(int value);


	///@brief 获取上次挑战时间
	__int64 GetLastChallengeTime();

	///@brief 设置上次挑战时间
	void SetLastChallengeTime(__int64 lastChallengeTime);

private:
	SArenaData *m_pArenaData; //玩家竞技场数据，存在数据库中
	
	//SArenaCompetitor *m_pCurrentCompetitor; //当前选中对手的竞技场数据
	//vector<SArenaData *> m_DefenseTeam;
	SHeroData m_DefenseTeam[5];
	int m_Rank;
};

