#pragma once

#include "NetModule.h"
#include "../pub/ConstValue.h"
#include "PlayerTypedef.h"
#include "SanguoPlayerMsg.h"
#define MAX_CHALLENGER	5  //挑战者数量
#define MAX_CHALLENGEITEM	 5  //单个玩家奖励道具数量
#define MAX_CHALLENGERUI 10 //当前前10的玩家奖励数据
#define MAX_CONWINITEMNUM 3 //连胜奖励道具数量
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 竞技场相关消息类
//=============================================================================================
DECLARE_MSG_MAP(SArenaMsg, SMessage, SMessage::EPRO_DYARENA_MESSAGE)
EPRO_DYARENA_INIT,	//点击竞技场选择对手界面
EPRO_DYARENA_REWARD,	//点击竞技场奖励界面
EPRO_DYARENA_START,	//玩家选择挑战者后开始进入竞技场
EPRO_DYARENA_QUESTREWARD,	//请求领取奖励
END_MSG_MAP()

typedef struct PLAYERCHALLENGE    //玩家对手数据
{
	WORD wRank;//玩家排名
	WORD wModID;//玩家模型ID
	DWORD dfightpower;//战斗力
	char  playername[CONST_USERNAME];
	PLAYERCHALLENGE::PLAYERCHALLENGE() :wRank(0), wModID(0), dfightpower(0)
	{
		memset(playername, 0, sizeof(char)*CONST_USERNAME);
	}
}PlayerChallenge;

typedef struct RANKSTAGE  //排名阶段，在UI中显示100-200
{
	WORD wMinRank;
	WORD wMaxRank;
	RANKSTAGE::RANKSTAGE() :wMinRank(0), wMaxRank(0){}
}RankStage;


typedef struct ARENAREWARD   //竞技场获胜奖励
{
	DWORD arenaitem[MAX_CHALLENGEITEM];    //对应奖励道具
	BYTE  arenaitemnum[MAX_CHALLENGEITEM]; //奖励道具对应数量
	RankStage  rankstage; //当前阶段
	ARENAREWARD::ARENAREWARD()
	{
		memset(arenaitem, 0, sizeof(DWORD)*MAX_CHALLENGEITEM); 
		memset(arenaitemnum, 0, sizeof(BYTE)*MAX_CHALLENGEITEM);
	}
}ArenaReward;



// 请求玩家竞技场数据
DECLARE_MSG(SSArenaInit, SArenaMsg, SArenaMsg::EPRO_DYARENA_INIT)
struct SQSArenaInit : public SSArenaInit
{
};

struct SASArenaInit : public SSArenaInit
{
	WORD wRank;	//当前排名 
	WORD wRankPoint;//积分
	WORD wConWin;//连续获胜次数
	DWORD dcoldtime;//挑战冷却时间
	PlayerChallenge pchallenge[MAX_CHALLENGER];
	BYTE  bchallengecount;//挑战次数
	SASArenaInit::SASArenaInit() :wRank(0), wRankPoint(0), wConWin(0), dcoldtime(0), bchallengecount(0){}
};

// 请求玩家竞技场奖励数据界面
DECLARE_MSG(SSArenaRewardUI, SArenaMsg, SArenaMsg::EPRO_DYARENA_REWARD)
struct SQSArenaRewardUI : public SSArenaRewardUI
{
};

struct SASArenaRewardUI : public SSArenaRewardUI
{
	ArenaReward  arendreward[MAX_CHALLENGERUI];  //前10名玩家奖励
	ArenaReward  upstagereward; //上一阶段奖励，比玩家自己更高的奖励 
	ArenaReward  currentreward;//当前玩家自己领取的奖励
	WORD wRankPoint;//积分
	DWORD dConWinItem[MAX_CONWINITEMNUM];//当前连胜奖励
	BYTE bConWinItemNum[MAX_CONWINITEMNUM];//当前连胜奖励道具数量
	DWORD drewardcoldtime;//奖励领取剩余时间
	BYTE bGetConWin;     //普通奖励是否可以领取
	BYTE bGetCommonWin; //连胜奖励是否可以领取
	SASArenaRewardUI::SASArenaRewardUI() :wRankPoint(0), drewardcoldtime(0)
	{
		memset(dConWinItem, 0, sizeof(DWORD)*MAX_CONWINITEMNUM);
		memset(bConWinItemNum, 0, sizeof(BYTE)*MAX_CONWINITEMNUM);
	}
};

//玩家选择挑战者后开始进入竞技场
DECLARE_MSG(SArenaStart, SArenaMsg, SArenaMsg::EPRO_DYARENA_START)
struct SQArenaStart : public SArenaStart
{
	BYTE bIndex;//选择的对手的索引 1，2
};


//客户端请求领取奖励
DECLARE_MSG(SArenaQuestReward, SArenaMsg, SArenaMsg::EPRO_DYARENA_QUESTREWARD)
struct SQArenaQuestReward : public SArenaQuestReward
{
	BYTE  bRewardtype;// 1 普通奖励 2连胜奖励
};

struct SAArenaQuestReward : public SArenaQuestReward
{
	BYTE bGetConWin;     //普通奖励是否可以领取  0  没有奖励领取 1 可以领取但是未领取成功 2 领取成功
	BYTE bGetCommonWin; //连胜奖励是否可以领取
	DWORD drewardcoldtime;//奖励领取剩余时间
	WORD wRankPoint;//积分
};

//==============================三国消息

///@brief 保留到本地文件的数据
struct SBaseArenaCompetitor
{
	char m_CompetitorName[32];
	char m_FactionName[32];
	SHeroData m_HeroList[5];
	SHeroFateAttr m_FateAttr[5];
	DWORD m_AernaID;
	DWORD m_TeamLevel;
	DWORD m_ArenaRank; ///@brief 当前排名
	DWORD m_BestRank; ///@brief 最佳排名
	DWORD m_CompetitorIcon; ///头像ID
	DWORD m_VipLevel; ///@brief vip等级
};


///@brief 竞技场玩家排名数据
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
Arena_SetBuyChallengeNum,  //购买挑战次数
Arena_ResetCountDown,  //重置购买时间
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

//新纪录诞生
DECLARE_MSG(SBirthBestRecord, SSGArenaMsg, SSGArenaMsg::Arena_BirthBestRecord)
struct SABirthBestRecords : public SBirthBestRecord
{
	DWORD m_OriginalRecordRank; //原纪录
	DWORD m_NewRecordRank; //新纪录
	DWORD m_DiamondRewards; //奖励宝石
};


DECLARE_MSG(SGetTop50th, SSGArenaMsg, SSGArenaMsg::Arena_GetTop50th)
///@brief 获取排行前50的排名信息
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
	int BuyChallengeNum;	//购买次数
	int ChallengeCount;		//剩余次数
};

//重置CD时间
struct SAResetCD_Respoens : public SSGArenaMsg
{
	SAResetCD_Respoens()
	{
		SSGArenaMsg::_protocol = SSGArenaMsg::Arena_ResetCountDown;
	}
	int ResetNum;	//重置次数
};