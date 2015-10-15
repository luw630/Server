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

	///@brief ��ȡ��������
	void GetDefenseTeamInfo(SHeroData* pHeroDataList);
	///@brief ���·�������
	void UpdateDefenseTeamInfo();

	///@brief ��ȡ��ǰ����
	//SArenaCompetitor * GetCompetitor() { return m_pCurrentCompetitor; }
	///@brief ���õ�ǰ����
	//void SetCompetitor(SArenaCompetitor *pCompetitor){ m_pCurrentCompetitor = pCompetitor; }

	const SHeroData * GetDefenseTeamInfo(const int defensiveTeamID[]);
	
	///@brief ��ȡ��������
	int GetRank(){ return m_Rank; }
	void SetRank(int rank) { m_Rank = rank; }
	
	///@brief ��ȡ�������
	int GetBestRank();
	///@brief �����������
	void SetBestRank(int value);


	///@brief ��ȡ�ϴ���սʱ��
	__int64 GetLastChallengeTime();

	///@brief �����ϴ���սʱ��
	void SetLastChallengeTime(__int64 lastChallengeTime);

private:
	SArenaData *m_pArenaData; //��Ҿ��������ݣ��������ݿ���
	
	//SArenaCompetitor *m_pCurrentCompetitor; //��ǰѡ�ж��ֵľ���������
	//vector<SArenaData *> m_DefenseTeam;
	SHeroData m_DefenseTeam[5];
	int m_Rank;
};

