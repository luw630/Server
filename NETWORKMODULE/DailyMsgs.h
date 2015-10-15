#pragma once
#include "NetModule.h"
//玩家日常活动相关消息
//=============================================================================================
DECLARE_MSG_MAP(SDailyMsg, SMessage, SMessage::EPRO_DAILY_MESSAGE)
//{{AFX
	//玩家日常活动消息
EPRO_DAILY_INIT,	//初始化日常活动相关数据
EPRO_DAILY_ENTRY,	//请求进入日常活动场景或领取日常活动奖励
EPRO_DAILY_AWARD,	//返回奖励详细信息
EPRO_DAILY_LEAVE,	//客户端请求离开日常活动场景
EPRO_DAILY_CURBATCHMONSTERDIE,	//通知玩家当前波怪物死亡，下一波还有多久出现，和玩家已经达到多少波怪了

//玩家活动消息
EPRO_FIRSTPAY_UPDATEFLAGE,	//通知玩家首次充值标识改变
EPRO_FIRSTPAY_AWARD,		//玩家首次充值奖励
EPRO_EVERYDAYPAY_AWARD,		//玩家领取每日充值奖励
EPRO_EVERYDAYPAY_GETAWARDSTATE,	//获取每日充值奖励状态

EPRO_DAILY_NPCBUYSP,	//活动中点击NPC，返回购买信仰之力商品的信息
EPRO_DAILY_OPENFLAG,	//返回失乐园活动开放标志(玩家登陆时要发给玩家，和标志重置时要同步给所有在线玩家)

EPRO_DAILY_JULONGSHANRISK,	//玩家请求在巨龙山中探险

EPRO_TAROT_INITTAROTDATA,	//初始化玩家塔罗牌数据
EPRO_TAROT_TURNONONECARD,	//玩家请求翻开一张塔罗牌
EPRO_TAROT_GETAWARD,	//玩家请求塔罗牌奖励
EPRO_TAROT_RESETTAROTDATA,	//重置玩家塔罗牌数据
EPRO_TAROT_UPTPLAYANIMATEFLAG,	//更该塔罗牌播放动画标志

//当天在线时长奖励活动
EPRO_CURDAYONLINE_INIT,	//初始化玩家每日在线时长奖励信息
EPRO_CURDAYONLINE_TIMELONG,		//玩家请求每日在线时长活动玩家当天在线时长
EPRO_CURDAYONLINE_GETAWARD,		//获取对应时间段的奖励
EPRO_CURDAYONLINE_RESET,	//通知玩家活动重置
EPRO_CURDAYONLINE_ADDONEAWARD,		//玩家达到一个新的可以领取的奖励

//累计登陆活动
EPRO_ADDUPLOGIN_INIT,	//初始化累计在线登陆数据
EPRO_ADDUPLOGIN_ADDUPDAY,	//活动玩家当天累计登陆天数
EPRO_ADDUPLOGIN_ADDNEWAWARD,	//玩家达到可以领取新的奖励
EPRO_ADDUPLOGIN_GETAWARD,	//玩家领取累计登陆相应级别ID的奖励

//充值回馈活动
EPRO_RFB_INIT,	//初始化充值回馈数据
EPRO_RFB_GETAWARD,	//领取充值回馈对应的奖励
EPRO_RFB_ADDNEWAWARD,	//通知玩家有新的奖励可以领取

//等级竞赛活动
EPRO_LR_INIT,	//初始化玩家等级竞赛数据
EPRO_LR_GETAWARD,	//获取等级竞赛奖励
EPRO_LR_NOTIFYCANGETNEWAWARD,	//通知玩家可以领取新的奖励
EPRO_LR_NOTIFYUPTRESIDUETIMES,	//通知所有在线玩家剩余次数改变

//幸运摩天轮活动
EPRO_FW_LOGININIT,	//获取幸运摩天轮登陆初始化的数据
EPRO_FW_GETOPENRESIDUETIME,	//获取打开幸运摩天轮的开放剩余时间数据
EPRO_FW_STARTGAME,	//开始幸运摩天轮游戏
EPRO_FW_BROADCASTRECORDUPT,	//广播幸运摩天轮玩家记录信息变更数据
EPRO_FW_GETAWARD,	//获取奖励

EPRO_RECHARGE_SYNTATOLGLOD,		//同步玩家充值金币总数

//}}AFX
END_MSG_MAP()

//初始化日常活动相关数据
DECLARE_MSG(SInitDailyMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_INIT)
struct SQInitDailyMsg : public SInitDailyMsg
{
};

struct SAInitDailyMsg : public SInitDailyMsg
{
	BYTE m_DailyArrSize;
	unsigned short *m_pDailyDetail;		//包括：1、最高位表示日常完成状态；2、高字节后面的7个位表示日常活动的剩余次数；3、低字节表示日常ID
	BYTE *m_pDailyTotalTimes;	//日常活动总的免费次数
};

//进入日常活动场景
DECLARE_MSG(SEntryDailyMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_ENTRY)
struct SQEntryDailyMsg : public SEntryDailyMsg
{
	BYTE m_DailyID;
	BYTE m_IsFreeDaily;		//是否为免费日常活动标识，0为免费日常，1为付费日常
};

struct SAEntryDailyMsg : public SEntryDailyMsg
{
	BYTE m_Result;	// 返回状态  0次数不足进入失败  1进入成功  2进入操作失败   4日常活动结束 5玩家等级足够，不能进入日常活动
	WORD m_NextSpendGlord;	//下一次付费需要的金币数
};

//返回奖励详细信息
DECLARE_MSG(SDailyAwardMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_AWARD)
struct SQDailyAwardMsg : public SDailyAwardMsg
{
	BYTE m_DailyID;
};

struct SADailyAwardMsg : public SDailyAwardMsg
{
	//BYTE m_AwardType;	//奖励类型（0对应领取奖励失败，1对应奖励材料道具，2对应奖励真气，3对应奖励经验，4对应奖励银币，5对应奖励金币）
	enum
	{
		AW_ITEMID,
		AW_ITEMNUM,
		AW_EXTRAITEMID,
		AW_EXTRAITEMNUM,
		AW_SP,
		AW_EXP,
		AW_JINBI,
		AW_YINBI,
	};
	DWORD m_AwardValue[8];	//奖励的值，0对应奖励材料道具ID，1对应奖励材料道具数量, 2对应额外奖励材料道具ID，3对应额外奖励材料道具数量,4对应奖励真气，5对应奖励经验，6对应奖励银币，7对应奖励金币）
};

//客户端请求离开日常活动场景
DECLARE_MSG(SDailyLeaveMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_LEAVE)
struct SQDailyLeaveMsg : public SDailyLeaveMsg
{
};

//通知玩家当前波怪物死亡，下一波还有多久出现，和玩家已经达到多少波怪了
DECLARE_MSG(SNextMonsterRefreshMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_CURBATCHMONSTERDIE)
struct SANextMonsterRefreshMsg : public SNextMonsterRefreshMsg
{
	BYTE m_CurBatchMonster;	//下一波怪为第几波怪物
	WORD m_NextBatchRefreshTime;	//下一波怪还有多久刷新
	WORD m_CurDailyID;	//当前所属日常活动ID
};

//玩家活动消息
//通知玩家首次充值标识改变
DECLARE_MSG(SUptFirstPayFlagMsg, SDailyMsg, SDailyMsg::EPRO_FIRSTPAY_UPDATEFLAGE)
struct SAUptFirstPayFlagMsg : public SUptFirstPayFlagMsg
{
	BYTE m_NewValue;
};

//玩家首次充值奖励
DECLARE_MSG(SFirstPayAwardMsg, SDailyMsg, SDailyMsg::EPRO_FIRSTPAY_AWARD)
struct SQFirstPayAwardMsg : public SFirstPayAwardMsg
{
};
struct SAFirstPayAwardMsg : public SFirstPayAwardMsg
{
	BYTE m_Result;
};


//玩家领取每日充值奖励
DECLARE_MSG(SEveryDayPayAwardMsg, SDailyMsg, SDailyMsg::EPRO_EVERYDAYPAY_AWARD)
struct SQEveryDayPayAwardMsg : public SEveryDayPayAwardMsg
{
	BYTE m_AwardIndex;	//奖励索引从1开始
};
struct SAEveryDayPayAwardMsg : public SEveryDayPayAwardMsg
{
	BYTE m_Result;
};

//获取每日充值奖励状态
#define EVERYDAYAWARDNUM 4

DECLARE_MSG(SGetEveryDayAwardStateMsg, SDailyMsg, SDailyMsg::EPRO_EVERYDAYPAY_GETAWARDSTATE)
struct SQGetEveryDayAwardStateMsg : public SGetEveryDayAwardStateMsg
{
};
struct SAGetEveryDayAwardStateMsg : public SGetEveryDayAwardStateMsg
{
	BYTE m_State[EVERYDAYAWARDNUM];
};


//活动中点击NPC，返回购买信仰之力商品的信息
DECLARE_MSG(SClickNpcGetSpGoodsInfMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_NPCBUYSP)
struct SAClickNpcGetSpGoodsInfMsg : public SClickNpcGetSpGoodsInfMsg
{
	BYTE m_DefaultNum;	//默认一次可以购买的数量
	GoodsData m_GoodsData;	//商品信息
};

//返回活动开放标志(玩家登陆时要发给玩家，和标志重置时要同步给所有在线玩家)
DECLARE_MSG(SDailyOpenFlagMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_OPENFLAG)
struct SADailyOpenFlagMsg : public SDailyOpenFlagMsg
{
	BYTE m_DailyType;
	BYTE m_Flag;
};


//玩家请求在巨龙山中探险
#define PLOTDIALOGUEMAXSIZE 400

DECLARE_MSG(SJuLongShanRiskMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_JULONGSHANRISK)
struct SQJuLongShanRiskMsg : public SJuLongShanRiskMsg
{
};
struct SAJuLongShanRiskMsg : public SJuLongShanRiskMsg
{
	BYTE m_AwardType;		//奖励类型（为1表示奖励道具，为2表示奖励经验，为3奖励银币，为4奖励信仰之力，为5表示不给于任何奖励）
	char m_PlotDialogueInf[PLOTDIALOGUEMAXSIZE];	//探险的剧情对话
	DWORD m_AwardItemID;	//如果奖励类型为1，则为道具ID，否则为0
	DWORD m_AwardNum;		//如果为1类型奖励，则为道具的数量，否则为具体的奖励值数量
};


//塔罗牌最大数量
#define TAROTMAXNUM 12

//塔罗牌名字的最大长度
#define TAROTMAXNAMELEN 32
//塔罗牌最大翻牌数
#define MAXTURNONTAROTNUM 5

//塔罗牌对家奖励
struct TarotAwardData
{
	DWORD m_ItemID;
	BYTE m_ItemNUm;
};

//塔罗牌基本数据信息
struct TarotBaseData
{
	BYTE m_TarotID;
	char m_TarotName[TAROTMAXNAMELEN];
	TarotAwardData m_TarotAwardData;
};

//玩家翻开的一张塔罗牌数据
struct TurnOnTarotData
{
	BYTE m_TarotID;
	BYTE m_TarotPos;
};

//初始化玩家塔罗牌数据

DECLARE_MSG(SInitTarotDataMsg, SDailyMsg, SDailyMsg::EPRO_TAROT_INITTAROTDATA)
struct SAInitTarotDataMsg : public SInitTarotDataMsg
{
	BYTE m_CanPlayTurnOffCard;	//玩家能否播放扣牌动画，为1表示会，为0表示不会
	TarotBaseData m_TarotBaseData[TAROTMAXNUM];  //塔罗牌基本数据
	TurnOnTarotData m_TurnOnTarotData[MAXTURNONTAROTNUM];	//玩家翻开的塔罗牌信息
};

//玩家请求翻开一张塔罗牌
DECLARE_MSG(STurnOnOneTarotCardMsg, SDailyMsg, SDailyMsg::EPRO_TAROT_TURNONONECARD)
struct SQTurnOnOneTarotCardMsg : public STurnOnOneTarotCardMsg
{
	BYTE m_TurnOnTarotPos;	//玩家翻开那个位置的塔罗牌
};
struct SATurnOnOneTarotCardMsg : public STurnOnOneTarotCardMsg
{
	BYTE m_InsertPos;	//新翻开的塔罗牌位置
	TurnOnTarotData m_TurnOnTarotData;	//新翻开的塔罗牌信息
};

//玩家请求塔罗牌奖励
DECLARE_MSG(SGetTarotAwardMsg, SDailyMsg, SDailyMsg::EPRO_TAROT_GETAWARD)
struct SQGetTarotAwardMsg : public SGetTarotAwardMsg
{
};
struct SAGetTarotAwardMsg : public SGetTarotAwardMsg
{
	BYTE m_Ret;
};

//重置玩家塔罗牌数据
DECLARE_MSG(SResetPlayerTarotMsg, SDailyMsg, SDailyMsg::EPRO_TAROT_RESETTAROTDATA)
struct SAResetPlayerTarotMsg : public SResetPlayerTarotMsg
{
};


//更该塔罗牌播放动画标志
DECLARE_MSG(SUptTarotPlayAnimateFlagMsg, SDailyMsg, SDailyMsg::EPRO_TAROT_UPTPLAYANIMATEFLAG)
struct SQUptTarotPlayAnimateFlagMsg : public SUptTarotPlayAnimateFlagMsg
{
};


//当天在线时长奖励活动
//玩家每个级别的信息
struct CurDayOnlineData
{
	BYTE m_CurDayOnlineLevelID;		//玩家当天在线级别ID
	BYTE m_AwardFlag;	//奖励状态（0标识不能领取，1标识可以领取，2标识已经领取）
	DWORD m_NeedTime;	//需要在线时长时间
	DWORD m_AwardItemID;	//奖励的道具ID
	DWORD m_AwardItemNum;	//奖励数量（为道具（除经验、真气、信仰之力、银币之外的道具）为道具的数量，否则为实际的奖励值）

};

//初始化玩家每日在线时长奖励信息
DECLARE_MSG(SCurDayOnlineInitMsg, SDailyMsg, SDailyMsg::EPRO_CURDAYONLINE_INIT)
struct SACurDayOnlineInitMsg : public SCurDayOnlineInitMsg
{
	BYTE m_CurDayOnlineAwardNum;
	CurDayOnlineData* m_lpCurDayOnlineData;
};

//玩家请求每日在线时长活动玩家当天在线时长
DECLARE_MSG(SCurDayOnlineLongMsg, SDailyMsg, SDailyMsg::EPRO_CURDAYONLINE_TIMELONG)
struct SQCurDayOnlineLongMsg : public SCurDayOnlineLongMsg
{
};
struct SACurDayOnlineLongMsg : public SCurDayOnlineLongMsg
{
	DWORD m_CurDayOnlineTime;	//玩家当天在线总时长
};

//获取对应时间段的奖励
DECLARE_MSG(SCurDayOnlineGetAwardMsg, SDailyMsg, SDailyMsg::EPRO_CURDAYONLINE_GETAWARD)
struct SQCurDayOnlineGetAwardMsg : public SCurDayOnlineGetAwardMsg
{
	BYTE m_CurDayOnlineLevelID;		//玩家当天在线级别ID，为0表示领取当前所有可以领取的奖励
};
struct SACurDayOnlineGetAwardMsg : public SCurDayOnlineGetAwardMsg
{
	BYTE m_Ret;
};

//通知玩家活动重置
DECLARE_MSG(SCurDayOnlineResetMsg, SDailyMsg, SDailyMsg::EPRO_CURDAYONLINE_RESET)
struct SACurDayOnlineResetMsg : public SCurDayOnlineResetMsg
{
};

//玩家达到一个新的可以领取的奖励
DECLARE_MSG(SCurDayOnlineAddAwardMsg, SDailyMsg, SDailyMsg::EPRO_CURDAYONLINE_ADDONEAWARD)
struct SACurDayOnlineAddAwardMsg : public SCurDayOnlineAddAwardMsg
{
	BYTE m_OnlineLevelID;		//玩家当天在线级别ID
};

//累计登陆活动
#define ADDUPLOGINMAXAWARDNUM 5
//累计登陆基本数据
struct AddUpLoginBaseData
{
	BYTE m_AddUpLoginLevelID;	//基本ID
	BYTE m_AwardFlag;	//奖励标识
	BYTE m_AwardItemNum[ADDUPLOGINMAXAWARDNUM];	//奖励的道具数量
	WORD m_NeedDay;	//达到的天数
	DWORD m_AwardItem[ADDUPLOGINMAXAWARDNUM];	//奖励的道具ID
};

//初始化累计在线登陆数据
DECLARE_MSG(SAddUpLoginInitMsg, SDailyMsg, SDailyMsg::EPRO_ADDUPLOGIN_INIT)
struct SAAddUpLoginInitMsg : public SAddUpLoginInitMsg
{
	BYTE m_AddUpLevelNum;
	AddUpLoginBaseData* m_lpAddUpLoginBaseData;
};

//活动玩家当天累计登陆天数
DECLARE_MSG(SAddUpLoginDayMsg, SDailyMsg, SDailyMsg::EPRO_ADDUPLOGIN_ADDUPDAY)
struct SQAddUpLoginDayMsg : public SAddUpLoginDayMsg
{
};
struct SAAddUpLoginDayMsg : public SAddUpLoginDayMsg
{
	WORD m_AddUpLoginDay;
};

//玩家达到可以领取新的奖励
DECLARE_MSG(SAddUpLoginAddNewAwardMsg, SDailyMsg, SDailyMsg::EPRO_ADDUPLOGIN_ADDNEWAWARD)
struct SAAddUpLoginAddNewAwardMsg : public SAddUpLoginAddNewAwardMsg
{
	BYTE m_AddUpLoginLevelID;
};

//玩家领取累计登陆相应级别ID的奖励
DECLARE_MSG(SAddUpLoginGetAwardMsg, SDailyMsg, SDailyMsg::EPRO_ADDUPLOGIN_GETAWARD)
struct SQAddUpLoginGetAwardMsg : public SAddUpLoginGetAwardMsg
{
	BYTE m_AddUpLoginLevelID;
};
struct SAAddUpLoginGetAwardMsg : public SAddUpLoginGetAwardMsg
{
	BYTE m_Ret;
};

//充值回馈活动
#define RFBMAXAWARDITEMNUM 6
struct RFBAwardInf
{
	BYTE m_AwardID;	//奖励ID
	BYTE m_IsCanGetAwardFlag;	//是否可以领取奖励标识
	BYTE m_AwardItemNum[RFBMAXAWARDITEMNUM];	//奖励道具对应的数量
	DWORD m_NeedRechargeGlodNum;	//需要充值金币总数
	DWORD m_AwardItem[RFBMAXAWARDITEMNUM];		//奖励道具
};

//初始化充值回馈数据
DECLARE_MSG(SRFBInitMsg, SDailyMsg, SDailyMsg::EPRO_RFB_INIT)
struct SARFBInitMsg : public SRFBInitMsg
{
	BYTE m_AwardAllNum;
	RFBAwardInf* m_lpRFBAwardInf;
};

//领取充值回馈对应的奖励
DECLARE_MSG(SRFBGetAwardMsg, SDailyMsg, SDailyMsg::EPRO_RFB_GETAWARD)
struct SQRFBGetAwardMsg : public SRFBGetAwardMsg
{
	BYTE m_AwardID;
};
struct SARFBGetAwardMsg : public SRFBGetAwardMsg
{
	BYTE m_Ret;
};
//通知玩家有新的奖励可以领取
DECLARE_MSG(SRFBAddNewAwardMsg, SDailyMsg, SDailyMsg::EPRO_RFB_ADDNEWAWARD)
struct SARFBAddNewAwardMsg : public SRFBAddNewAwardMsg
{
	BYTE m_AwardID;
};

//等级竞赛活动
#define LEVELRACEMAXAWARDNUM 4
//玩家等级竞赛基本数据
struct PlayerLevelRaceBaseData
{
	BYTE m_DataID;	//数据ID
	BYTE m_NeedLevel;  //需要达到的等级
	BYTE m_IsCanGetAwardFlag;	//是否可以领取奖励标识（0为不可以领取，1为可以领取，2为已经领取）
	WORD m_MaxPeopleNum;	//最多人数
	WORD m_ResiduePeopleNum;	//剩余人数
	WORD m_AwardItemNum[LEVELRACEMAXAWARDNUM];	//对应奖励道具的数量
	DWORD m_AwardItem[LEVELRACEMAXAWARDNUM];	//奖励的道具组
};

//初始化玩家等级竞赛数据
DECLARE_MSG(SLRInitMsg, SDailyMsg, SDailyMsg::EPRO_LR_INIT)
struct SALRInitMsg : public SLRInitMsg
{
	BYTE m_LevelRaceNum;
	PlayerLevelRaceBaseData* m_lpPlayerLevelRaceBaseData;
};

//获取等级竞赛奖励
DECLARE_MSG(SLRGetAwardMsg, SDailyMsg, SDailyMsg::EPRO_LR_GETAWARD)
struct SQLRGetAwardMsg : public SLRGetAwardMsg
{
	BYTE m_DataID;
};
struct SALRGetAwardMsg : public SLRGetAwardMsg
{
	BYTE m_Ret;
};

//通知玩家可以领取新的奖励
DECLARE_MSG(SLRNotifyAddNewAwardMsg, SDailyMsg, SDailyMsg::EPRO_LR_NOTIFYCANGETNEWAWARD)
struct SALRNotifyAddNewAwardMsg : public SLRNotifyAddNewAwardMsg
{
	BYTE m_DataID;
};

//通知所有在线玩家剩余次数改变
DECLARE_MSG(SLRBroadCastResidueTimesMsg, SDailyMsg, SDailyMsg::EPRO_LR_NOTIFYUPTRESIDUETIMES)
struct SALRBroadCastResidueTimesMsg : public SLRBroadCastResidueTimesMsg
{
	BYTE m_DataID;
	WORD m_LRResidueTimes;
};


//幸运摩天轮活动
#define FWMAXNUM 12 //幸运摩天轮最大数量
#define FWMAXRECORDPLAYERNUM 5 //幸运摩天轮最大记录玩家的数量
//幸运摩天轮基本数据信息
struct FWBaseInf
{
	BYTE m_SerialID;	//幸运摩天轮的序号ID
	BYTE m_AwardItmeNum;	//奖励的道具数量
	DWORD m_AwardItemID;	//奖励的道具
};

//幸运摩天轮记录玩家数据信息
struct FWRecordPlayerInf
{
	char m_PlayerName[CONST_USERNAME];	//记录玩家的姓名
	DWORD m_AwardItemID;	//记录玩家获得的奖励
};

//获取幸运摩天轮登陆初始化的数据
DECLARE_MSG(SFWLoginInitMsg, SDailyMsg, SDailyMsg::EPRO_FW_LOGININIT)
struct SAFWLoginInitMsg : public SFWLoginInitMsg
{
	FWBaseInf m_FWBaseInf[FWMAXNUM];
	FWRecordPlayerInf m_FWRecordPlayerInf[FWMAXRECORDPLAYERNUM];
};

//获取打开幸运摩天轮的开放剩余时间数据
DECLARE_MSG(SFWOpenResidueTimeMsg, SDailyMsg, SDailyMsg::EPRO_FW_GETOPENRESIDUETIME)
struct SQFWOpenResidueTimeMsg : public SFWOpenResidueTimeMsg
{
};
struct SAFWOpenResidueTimeMsg : public SFWOpenResidueTimeMsg
{
	WORD m_CanUseTimes;	//可以使用的次数
	DWORD m_OpenResidueTime;
};

//开始幸运摩天轮游戏
DECLARE_MSG(SFWStartGameMsg, SDailyMsg, SDailyMsg::EPRO_FW_STARTGAME)
struct SQFWStartGameMsg : public SFWStartGameMsg
{
};
struct SAFWStartGameMsg : public SFWStartGameMsg
{
	BYTE m_SerialID;	//幸运摩天轮的序号ID
};

//广播幸运摩天轮玩家记录信息变更数据
DECLARE_MSG(SFWRecordUptMsg, SDailyMsg, SDailyMsg::EPRO_FW_BROADCASTRECORDUPT)
struct SAFWRecordUptMsg : public SFWRecordUptMsg
{
	FWRecordPlayerInf m_NewFWRecordPlayerInf[FWMAXRECORDPLAYERNUM]; //新的玩家记录信息
};

//获取奖励
DECLARE_MSG(SFWGetAwardMsg, SDailyMsg, SDailyMsg::EPRO_FW_GETAWARD)
struct SQFWGetAwardMsg : public SFWGetAwardMsg
{
	BYTE m_SerialID;	//幸运摩天轮的序号ID
};
struct SAFWGetAwardMsg : public SFWGetAwardMsg
{
	BYTE m_Ret;	
};

//同步玩家充值金币总数
DECLARE_MSG(SSynRechargeTatolGlodMsg, SDailyMsg, SDailyMsg::EPRO_RECHARGE_SYNTATOLGLOD)
struct SASynRechargeTatolGlodMsg : public SSynRechargeTatolGlodMsg
{
	DWORD m_TatolRechargeNum;	//总的充值数量
};