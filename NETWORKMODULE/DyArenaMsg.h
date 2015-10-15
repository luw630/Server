#pragma once

#include "NetModule.h"
#include "../pub/ConstValue.h"
#include "PlayerTypedef.h"
#include "SanguoPlayerMsg.h"
#define MAX_CHALLENGER	5  //��ս������
#define MAX_CHALLENGEITEM	 5  //������ҽ�����������
#define MAX_CHALLENGERUI 10 //��ǰǰ10����ҽ�������
#define MAX_CONWINITEMNUM 3 //��ʤ������������
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// �����������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SArenaMsg, SMessage, SMessage::EPRO_DYARENA_MESSAGE)
EPRO_DYARENA_INIT,	//���������ѡ����ֽ���
EPRO_DYARENA_REWARD,	//�����������������
EPRO_DYARENA_START,	//���ѡ����ս�ߺ�ʼ���뾺����
EPRO_DYARENA_QUESTREWARD,	//������ȡ����
END_MSG_MAP()

typedef struct PLAYERCHALLENGE    //��Ҷ�������
{
	WORD wRank;//�������
	WORD wModID;//���ģ��ID
	DWORD dfightpower;//ս����
	char  playername[CONST_USERNAME];
	PLAYERCHALLENGE::PLAYERCHALLENGE() :wRank(0), wModID(0), dfightpower(0)
	{
		memset(playername, 0, sizeof(char)*CONST_USERNAME);
	}
}PlayerChallenge;

typedef struct RANKSTAGE  //�����׶Σ���UI����ʾ100-200
{
	WORD wMinRank;
	WORD wMaxRank;
	RANKSTAGE::RANKSTAGE() :wMinRank(0), wMaxRank(0){}
}RankStage;


typedef struct ARENAREWARD   //��������ʤ����
{
	DWORD arenaitem[MAX_CHALLENGEITEM];    //��Ӧ��������
	BYTE  arenaitemnum[MAX_CHALLENGEITEM]; //�������߶�Ӧ����
	RankStage  rankstage; //��ǰ�׶�
	ARENAREWARD::ARENAREWARD()
	{
		memset(arenaitem, 0, sizeof(DWORD)*MAX_CHALLENGEITEM); 
		memset(arenaitemnum, 0, sizeof(BYTE)*MAX_CHALLENGEITEM);
	}
}ArenaReward;



// ������Ҿ���������
DECLARE_MSG(SSArenaInit, SArenaMsg, SArenaMsg::EPRO_DYARENA_INIT)
struct SQSArenaInit : public SSArenaInit
{
};

struct SASArenaInit : public SSArenaInit
{
	WORD wRank;	//��ǰ���� 
	WORD wRankPoint;//����
	WORD wConWin;//������ʤ����
	DWORD dcoldtime;//��ս��ȴʱ��
	PlayerChallenge pchallenge[MAX_CHALLENGER];
	BYTE  bchallengecount;//��ս����
	SASArenaInit::SASArenaInit() :wRank(0), wRankPoint(0), wConWin(0), dcoldtime(0), bchallengecount(0){}
};

// ������Ҿ������������ݽ���
DECLARE_MSG(SSArenaRewardUI, SArenaMsg, SArenaMsg::EPRO_DYARENA_REWARD)
struct SQSArenaRewardUI : public SSArenaRewardUI
{
};

struct SASArenaRewardUI : public SSArenaRewardUI
{
	ArenaReward  arendreward[MAX_CHALLENGERUI];  //ǰ10����ҽ���
	ArenaReward  upstagereward; //��һ�׶ν�����������Լ����ߵĽ��� 
	ArenaReward  currentreward;//��ǰ����Լ���ȡ�Ľ���
	WORD wRankPoint;//����
	DWORD dConWinItem[MAX_CONWINITEMNUM];//��ǰ��ʤ����
	BYTE bConWinItemNum[MAX_CONWINITEMNUM];//��ǰ��ʤ������������
	DWORD drewardcoldtime;//������ȡʣ��ʱ��
	BYTE bGetConWin;     //��ͨ�����Ƿ������ȡ
	BYTE bGetCommonWin; //��ʤ�����Ƿ������ȡ
	SASArenaRewardUI::SASArenaRewardUI() :wRankPoint(0), drewardcoldtime(0)
	{
		memset(dConWinItem, 0, sizeof(DWORD)*MAX_CONWINITEMNUM);
		memset(bConWinItemNum, 0, sizeof(BYTE)*MAX_CONWINITEMNUM);
	}
};

//���ѡ����ս�ߺ�ʼ���뾺����
DECLARE_MSG(SArenaStart, SArenaMsg, SArenaMsg::EPRO_DYARENA_START)
struct SQArenaStart : public SArenaStart
{
	BYTE bIndex;//ѡ��Ķ��ֵ����� 1��2
};


//�ͻ���������ȡ����
DECLARE_MSG(SArenaQuestReward, SArenaMsg, SArenaMsg::EPRO_DYARENA_QUESTREWARD)
struct SQArenaQuestReward : public SArenaQuestReward
{
	BYTE  bRewardtype;// 1 ��ͨ���� 2��ʤ����
};

struct SAArenaQuestReward : public SArenaQuestReward
{
	BYTE bGetConWin;     //��ͨ�����Ƿ������ȡ  0  û�н�����ȡ 1 ������ȡ����δ��ȡ�ɹ� 2 ��ȡ�ɹ�
	BYTE bGetCommonWin; //��ʤ�����Ƿ������ȡ
	DWORD drewardcoldtime;//������ȡʣ��ʱ��
	WORD wRankPoint;//����
};

//==============================������Ϣ

///@brief �����������ļ�������
struct SBaseArenaCompetitor
{
	char m_CompetitorName[32];
	char m_FactionName[32];
	SHeroData m_HeroList[5];
	SHeroFateAttr m_FateAttr[5];
	DWORD m_AernaID;
	DWORD m_TeamLevel;
	DWORD m_ArenaRank; ///@brief ��ǰ����
	DWORD m_BestRank; ///@brief �������
	DWORD m_CompetitorIcon; ///ͷ��ID
	DWORD m_VipLevel; ///@brief vip�ȼ�
};


///@brief �����������������
struct SArenaPlayerReducedData 
{
	char m_CompetitorName[32];
	DWORD m_CompetitorIconID;
	DWORD m_TeamLevel;
	DWORD m_FightingCapacity;
	DWORD m_Rank;
	DWORD m_ArenaID;
};


DECLARE_MSG_MAP(SSGArenaMsg, SMessage, SMessage::EPRO_DYARENA_MESSAGE)
Arena_DelockArena,
Arena_ChallengeBegin,
Arena_ChallengeOver,
Arena_GetCompetitor,
Arena_BirthBestRecord,
Arena_GetTop50th,
Arena_SetDefensiveTeam,
Arena_SynLoginData,
Arena_GetTop50Detail,
Arena_LastChallengTimeSpan,
Arena_SetBuyChallengeNum,  //������ս����
Arena_ResetCountDown,  //���ù���ʱ��
END_MSG_MAP()

DECLARE_MSG(SGetArenaCompetitors, SSGArenaMsg, SSGArenaMsg::Arena_GetCompetitor)
struct SQGetArenaCompetitors : public SGetArenaCompetitors
{
	
};

struct SAGetArenaCompetitors : public SGetArenaCompetitors
{
	DWORD m_curRank;
	SBaseArenaCompetitor m_Competitors[3];
};

DECLARE_MSG(SChallengeBegin, SSGArenaMsg, SSGArenaMsg::Arena_ChallengeBegin)
struct SQChallengeBegin : public SChallengeBegin
{
	DWORD m_ArenaID;
	DWORD m_Rank;
};

struct SAChallengeBegin : public SChallengeBegin
{
	DWORD m_Result;
	DWORD m_CompetitorID;
};

DECLARE_MSG(SChallengeOver, SSGArenaMsg, SSGArenaMsg::Arena_ChallengeOver)
struct SQChallengeOver : public SChallengeOver
{
	DWORD m_BattleResult;
	ArenaBattleVerifyData verifyData;
};

struct SAChallengeOver : public SChallengeOver
{

};

//�¼�¼����
DECLARE_MSG(SBirthBestRecord, SSGArenaMsg, SSGArenaMsg::Arena_BirthBestRecord)
struct SABirthBestRecords : public SBirthBestRecord
{
	DWORD m_OriginalRecordRank; //ԭ��¼
	DWORD m_NewRecordRank; //�¼�¼
	DWORD m_DiamondRewards; //������ʯ
};


DECLARE_MSG(SGetTop50th, SSGArenaMsg, SSGArenaMsg::Arena_GetTop50th)
///@brief ��ȡ����ǰ50��������Ϣ
struct SAGetTop50th : public SGetTop50th
{
	char m_FactionDdata[32];
	SArenaPlayerReducedData m_Top50th[50];
	SAGetTop50th()
	{
		//memset(m_Top50th, 0, sizeof(SAGetTop50th));_
	}
	
};

DECLARE_MSG(SSetDefensiveTeam, SSGArenaMsg, SSGArenaMsg::Arena_SetDefensiveTeam)
struct SQSetDefensiveTeam : public SSetDefensiveTeam
{
	DWORD m_HeroIDList[5];
};


struct SASetDefensiveTeam : public SSetDefensiveTeam
{
	byte m_Result;
};

struct  SAArenaLoginData : public SSGArenaMsg
{
	char m_LastChallengeTime[8];
	DWORD m_ChallengeCount;
	SAArenaLoginData()
	{
		memset(m_LastChallengeTime, 0, sizeof(char) *8);
		m_ChallengeCount = 0;
		SSGArenaMsg::_protocol = SSGArenaMsg::Arena_SynLoginData;
	}
};

DECLARE_MSG(SGetTop50PlayerDetail, SSGArenaMsg, SSGArenaMsg::Arena_GetTop50Detail)
struct  SQGetTop50PlayerDetail : public SGetTop50PlayerDetail
{
	DWORD m_ArenaID;
};

struct  SAGetTop50PlayerDetail : public SGetTop50PlayerDetail
{
	SBaseArenaCompetitor m_BaseArenaData;
};


struct SALastArenaChallengeTimeSpan : public SSGArenaMsg
{
	DWORD m_TimeSpanSeconds;
	SALastArenaChallengeTimeSpan()
	{
		SSGArenaMsg::_protocol = SSGArenaMsg::Arena_LastChallengTimeSpan;
		m_TimeSpanSeconds = 6000;
	}

};

struct BuyChallengeNum : public SSGArenaMsg
{
	
};
struct SArenaBaseMsg : public SSGArenaMsg
{
	SArenaBaseMsg()
	{
		SSGArenaMsg::_protocol = SSGArenaMsg::Arena_SetBuyChallengeNum;
	}
	int BuyChallengeNum;	//�������
	int ChallengeCount;		//ʣ�����
};

//����CDʱ��
struct SAResetCD_Respoens : public SSGArenaMsg
{
	SAResetCD_Respoens()
	{
		SSGArenaMsg::_protocol = SSGArenaMsg::Arena_ResetCountDown;
	}
	int ResetNum;	//���ô���
};