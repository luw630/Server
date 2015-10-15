#pragma once
#include "NetModule.h"
#include "../networkmodule/playertypedef.h"


#define MAX_ADDR_HTTP     512  //http请求 最大长度
#define MAX_ARENACHALLENGE_HERONUM 5	//竞技场战斗最大英雄数
#define MAX_War_HERONUM 5	//国战最大英雄数
#define  MAX_WAR_NAME_LIST 20 // 国战参战列表,每次取20个
#define  MAX_WAR_HERO_INFO 27 //16个基础属性数据(1-16)+8个技能数据(17-24)+2个血量怒气(25-26)+1武将id(27)
#define  MAX_Rank_List 50 // 排行榜最大数
#define  MaxExchangeData 10 //兑换活动最大条数

//======================================================================
DECLARE_MSG_MAP(SSGPlayerMsg, SMessage, SMessage::EPRO_SANGUOPLAYER_MESSAGE)
EPRO_SANGUO_ATTR = 0, ///<三国武将属性
EPRO_SANGUO_NEWGUIDE, ///<新手指引数据
EPRO_SANGUO_BLESS, ///<祈福相关
EPRO_SANGUO_DUPLICATE,	///<副本数据
EPRO_SANGUO_FORGING,	///<锻造消息
EPRO_EXPEDITION_MESSAGE,///<远征消息
EPRO_ACHIEVEMENT_MESSAGE,///<成就消息
EPRO_MISSION_MESSAGE,///<任务消息
EPRO_SANGUO_HEROUPGRADE, ///武将升级相关
EPRO_SANGUO_STORAGE,	///<背包数据
EPRO_SANGUO_ADDGOODS_SYN,	///<同步相关,得到东西,主要为addgoods相关,如铜钱元宝等
EPRO_SANGUO_DECGOODS_SYN,	///<同步相关,扣除东西,主要为Decgoods相关,如铜钱元宝等
EPRO_SANGUO_MALL,	///<商城数据
EPRO_SANGUO_CHECKIN,	///<签到
EPRO_SANGUO_DIAMONDEXCHANGE, ///<宝石兑换其他物件的消息
EPRO_SANGUO_SKILLUPGRADE, ///<技能升级
EPRO_SANGUO_SCRIPT, ///<脚本消息相关
EPRO_SANGUO_RANDOMAVHIEVEMNT_MESSAGE, ///<随机生成的可达成成就的相关消息
EPRO_SANGUO_DESSERT_MESSAGE,	///<小型奖励
EPRO_SANGUO_CHASETHIEF,	///<追击盗贼
EPRO_SANGUO_CHANGENAME,	///<变更名称
EPRO_SANGUO_ACTIVITY,///<活动相关
EPRO_SANGUO_RECHARGE,///<充值
EPRO_SANGUO_HEROEXTENDSGAMEPLAY,///武将英雄的扩展玩法，比如挂机得金币，挂机训练
EPRO_SANGUO_fIRSTRECHARGE,//首冲领奖
EPRO_SANGUO_REWARDSTATE,///奖励状态
EPRO_SANGUO_WeekReMoney,//基金
EPRO_SANGUO_War,//国战
EPRO_SANGUO_Rank,//排行榜
EPRO_SANGUO_Excheange,//兑换活动

END_MSG_MAP()





///@brief 元宝兑换其他物件的消息
DECLARE_MSG_MAP(SDiamondExchangeMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_DIAMONDEXCHANGE)
SANGUO_EXCHANGEGOLD = 0, ///<元宝兑换金钱消息
SANGUO_EXCHANGEENDURANCE, ///<元宝兑换体力的消息
END_MSG_MAP()

///@brief 元宝兑换金钱的消息
DECLARE_MSG_MAP(SDiamondExchangeGoldMsg, SDiamondExchangeMsg, SDiamondExchangeMsg::SANGUO_EXCHANGEGOLD)
REQUEST_EXCHANGEGOLD_ONCE = 0,///<申请用宝石兑换一次金钱
REQUEST_EXCHANGEGOLD_CONTINUOUS,///<申请连续使用宝石来兑换金钱
ANSWER_EXCHANGEGOLD_INFOR,///<用宝石兑换金钱的结果信息
ANSWER_REFRESH,			///<刷新可兑换的次数
END_MSG_MAP()

///@brief 返回用宝石兑换金钱的信息
struct SAExchangeGoldInfor : SDiamondExchangeGoldMsg
{
	DWORD diamondCostEachTime;	///<每一次消耗宝石的数量
	DWORD exchangedTimes;		///<兑换的次数
	DWORD exchangedInfors[MAX_ITEM_NUM_SYN_TO_CLIENT];	///<大小为exchangedTimes的2倍，前exchangedTimes位为获得的金币数，后exchangedTimes位为本次的暴击倍数

	SAExchangeGoldInfor()
	{
		diamondCostEachTime = 0;
		exchangedTimes = 0;
		_protocol = SDiamondExchangeGoldMsg::ANSWER_EXCHANGEGOLD_INFOR;
	}
};



///@brief 元宝兑换体力的消息
DECLARE_MSG_MAP(SDiamondExchangeEnduranceMsg, SDiamondExchangeMsg, SDiamondExchangeMsg::SANGUO_EXCHANGEENDURANCE)
REQUEST_EXCHANGEENDURANCE = 0,
ANSWER_EXCHANGEENDURANCE_INFOR,///<用宝石兑换体力的结果信息
ANSWER_REFRESH,			///<刷新可兑换的次数
END_MSG_MAP()

///@brief 元宝兑换体力成功与否的消息
struct SAExchangeEnduranceInfor : SDiamondExchangeEnduranceMsg
{
	bool bSuccess;

	SAExchangeEnduranceInfor()
	{
		bSuccess = false;
		_protocol = SDiamondExchangeEnduranceMsg::ANSWER_EXCHANGEENDURANCE_INFOR;
	}
};

///小型奖励所相关的一些消息
DECLARE_MSG_MAP(SDessertMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_DESSERT_MESSAGE)
REQUEST_DESSERT = 0,
REQUEST_CLAIM_DESSERT,
ANSWER_DESSERT_BAKED_INFOR,
ANSWER_DESSERT_REFRESH,
END_MSG_MAP()

struct SADessertBakedInfor : SDessertMsg
{
	bool bDessertAvaliable;
	byte dessertType;
	int dessertNum;

	SADessertBakedInfor()
	{
		bDessertAvaliable = false;
		dessertType = 0;
		dessertNum = 0;
	}
};

DECLARE_MSG_MAP(SSetAttrMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_ATTR)
EPRO_ATTR_NONE = 0,
//=================================
EPRO_ATTR_STORYPLAYED,/// StoryPlayed
EPRO_ATTR_UPDATEPLAYERICON,///设置玩家头像图标
//EPRO_ATTR_
//EPRO_ATTR_
//EPRO_ATTR_
//EPRO_ATTR_
//EPRO_ATTR_
EPRO_ATTR_MAX, 
END_MSG_MAP()


struct SQSetAttrMsg : public SSetAttrMsg
{
	DWORD m_AttrValue;
	BYTE m_AttrEnum;
	SQSetAttrMsg()
	{
		m_AttrValue = 0;
		m_AttrEnum = 0;
	}
};

//==============仓库信息临时定义

DECLARE_MSG(SStorageMsg, SMessage, SMessage::EPRO_STORAGE_MESSAGE)
struct SQStroageMsg : public SStorageMsg
{
	SSanguoItem m_ItmeInfo;
	SQStroageMsg()
	{
		m_ItmeInfo.m_dwCount = 0;
		m_ItmeInfo.m_dwItemID = 0;
	}
};

DECLARE_MSG_MAP(SBlessMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_BLESS)
MONEY_FREE_BLESS,
MONEY_SINGLE_BLESS,
MONEY_MUTIPLE_BLESS,
DIAMOND_FREE_BLESS,
DIAMOND_SINGLE_BLESS,
DIAMOND_MUTIPLE_BLESS
END_MSG_MAP()

struct SQRequstBless : public SBlessMsg
{

	SQRequstBless()
	{
		SBlessMsg::_protocol = MONEY_FREE_BLESS;
	}

};

struct SASingleRequstBless : public SBlessMsg
{
	DWORD m_dwPrizeID;
	DWORD m_dwProperty;
	SASingleRequstBless(const BYTE blessType)
	{
		SBlessMsg::_protocol = blessType;
		m_dwProperty = 0;
		m_dwPrizeID = 0;
	}

};


struct SARequstMutipleBless : public SBlessMsg
{
	DWORD m_dwPrizeIDs[10];
	DWORD m_dwProperty;
	SARequstMutipleBless(const BYTE blessType)
	{
		SBlessMsg::_protocol = blessType;
		m_dwProperty = 0;
		for (int i = 0; i < 10; i++)
		{
			m_dwPrizeIDs[i] = 0;
		}
	}
};

DECLARE_MSG_MAP(SForgingMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_FORGING)
REQUEST_MONEY_FORGING = 0,
REQUEST_DIAMOND_FORGING,
ANSWER_MONEY_FORGING_PERMITTED,
ANSWER_DIAMOND_FORGING_PERMITTED,
END_MSG_MAP()

struct SQAskToForging : public SForgingMsg
{
	int heroID;///<要锻造装备的英雄的ID
	int forgingEquipID;///<要锻造的装备
	SQAskToForging()
	{
		_protocol = SForgingMsg::REQUEST_MONEY_FORGING;
		forgingEquipID = 0;
	}
};

struct SQAskToMoneyForging : public SQAskToForging
{
	int materialTypeNum;///<有多少种材料
	int materialInfor[];///<前materialTypeNum位为ID，后materialTypeNum位为num
	SQAskToMoneyForging()
	{
		_protocol = SForgingMsg::REQUEST_MONEY_FORGING;
		heroID = 0;
		forgingEquipID = 0;
		materialTypeNum = 1;
	}
};

struct SAForgingPermission : public SForgingMsg
{
	bool permitted;
	int heroID;
	int forgedEquipID;
	int curEquipStarLevel;
	SAForgingPermission()
	{
		permitted = false;
		heroID = 0;
		forgedEquipID = 0;
		curEquipStarLevel = 0;
		_protocol = SForgingMsg::ANSWER_MONEY_FORGING_PERMITTED;
	}
};

DECLARE_MSG_MAP(SMissionMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_MISSION_MESSAGE)
REQUEST_ACCOMPLISH_MISSION,
ANSWER_REFRESH_MISSION_DATA,
ANSWER_MISSION_ACCOMPLISHED,
ANSWER_UPDATE_MISSION_STATE,
END_MSG_MAP()

struct SQMissionAccomplished : public SMissionMsg
{
	int missionID;

	SQMissionAccomplished()
	{
		missionID = 0;
		_protocol = SMissionMsg::REQUEST_ACCOMPLISH_MISSION;
	}
};

struct SARefreshMissionData : public SMissionMsg
{
	SARefreshMissionData()
	{
		_protocol = SMissionMsg::ANSWER_REFRESH_MISSION_DATA;
	}
};

struct SAMissionAccomplished : public SMissionMsg
{
	bool bAccomplished;
	int missionID;
	SAMissionAccomplished()
	{
		missionID = 0;
		bAccomplished = false;
		_protocol = SMissionMsg::ANSWER_MISSION_ACCOMPLISHED;
	}
};

///@brief 刷新按时间计时来决定是否显示的任务的状态
struct SAUpdateMissionState : public SMissionMsg
{
	bool visible;
	int missionID;
	int completeTimes;
	SAUpdateMissionState()
	{
		_protocol = SMissionMsg::ANSWER_UPDATE_MISSION_STATE;
		visible = false;
		completeTimes = 0;
	}
};

DECLARE_MSG_MAP(SAchievementMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_ACHIEVEMENT_MESSAGE)
REQUESET_CLAIM_ACHIEVMENT = 0,
ANSWER_UPDATE_ACHIEVEMENT_STATE,
ANSWER_ACHIEVEMENT_CLAIM_CONFORMED,
END_MSG_MAP()

///@brief 客户端发来消息领取成就
struct SQClaimAchievement : public SAchievementMsg
{
	DWORD achievementID;

	SQClaimAchievement()
	{
		achievementID = 0;
		_protocol = SAchievementMsg::REQUESET_CLAIM_ACHIEVMENT;
	}
};

///@brief 服务器准许是否可以领取某一个成就
struct SAAchievementClaimConformed : public SAchievementMsg
{
	bool bConformed;///<是否允许领取
	bool bAchievementGroupAccomplished;///<该组成就是否全部完成
	byte achievementType;	///<哪组的某个成就可以领取
	DWORD achievementID;	///<哪个成就可以领取
	DWORD newsetAchievementID;
	SAAchievementClaimConformed()
	{
		newsetAchievementID = 0;
		achievementID = 0;
		bAchievementGroupAccomplished = false;
		bConformed = false;
		achievementType = (byte)AchievementType::Battle0;
		_protocol = SAchievementMsg::ANSWER_ACHIEVEMENT_CLAIM_CONFORMED;
	}
};

///@brief 更新某一个成就的完成状态
struct SAUpdateAchievementState : public SAchievementMsg
{
	DWORD achievementID;
	DWORD completedTimes;
	SAUpdateAchievementState()
	{
		achievementID = 0;
		_protocol = SAchievementMsg::ANSWER_UPDATE_ACHIEVEMENT_STATE;
	}
};

DECLARE_MSG_MAP(SRandomAchievementMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_RANDOMAVHIEVEMNT_MESSAGE)
REQUESET_RANDOM_NEW_ACHIEVEMENT = 0,
REQUESET_CLAIM_ACHIEVMENT,
ANSWER_REFRESH_RANDOM_ACHIEVEMENT,
ANSWER_NEW_AHIEVEMENT_GOT,
ANSWER_UPDATE_ACHIEVEMENT_STATE,
ANSWER_ACHIEVEMENT_CLAIM_CONFORMED,
END_MSG_MAP()

///@brief 客户端发来消息领取成就
struct SQClaimRandomAchievement : public SRandomAchievementMsg
{
	DWORD achievementID;

	SQClaimRandomAchievement()
	{
		achievementID = 0;
		_protocol = SRandomAchievementMsg::REQUESET_CLAIM_ACHIEVMENT;
	}
};

///@brief 向客户端发送的的新的随机好的成就
struct SANewRandomAchievementGot : public SRandomAchievementMsg
{
	bool achievementGot;
	DWORD achievementPersistentTimes;
	DWORD achievementID;

	SANewRandomAchievementGot()
	{
		achievementGot = false;
		achievementPersistentTimes = 0;
		_protocol = SRandomAchievementMsg::ANSWER_NEW_AHIEVEMENT_GOT;
	}
};

///@brief 服务器准许是否可以领取某一个成就
struct SARandomAchievementClaimConformed : public SRandomAchievementMsg
{
	bool bConformed;///<是否允许领取
	DWORD achievementID;	///<哪个成就可以领取
	SARandomAchievementClaimConformed()
	{
		achievementID = 0;
		bConformed = false;
		_protocol = SRandomAchievementMsg::ANSWER_ACHIEVEMENT_CLAIM_CONFORMED;
	}
};

///@brief 更新某一个成就的完成状态
struct SAUpdateRandomAchievementState : public SRandomAchievementMsg
{
	DWORD achievementID;
	DWORD completedTimes;
	SAUpdateRandomAchievementState()
	{
		achievementID = 0;
		_protocol = SRandomAchievementMsg::ANSWER_UPDATE_ACHIEVEMENT_STATE;
	}
};

DECLARE_MSG_MAP(SExpeditionMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_EXPEDITION_MESSAGE)
REQUEST_BATTLE_RESULT_RESOLVES = 0,
//REQUEST_NEXT_ENEMY_INFOR,
REQUEST_EXIT_BATTLE,
REQUEST_RECEIVE_REWARDS,
REQUEST_ENTER_BATTLE,
REQUEST_UPDATE_SELECTEDCHARACTER_INFOR,
REQUEST_UPDATE_ENEMYCHARACTER_INFOR,
REQUEST_COST_A_TICKET,
REQUEST_ACTIVE_EXPEDITION,///<暂时由客户端来通知激活远征关卡
ANSWER_BATTLE_RESOLVES_RESULT,
ANSWER_EXPEDITION_CONQURED,
ANSWER_PERMISSION_INFOR,
ANSWER_HOSTILEPLAYER_INFOR,
ANSWER_REFRESH_TICKET,
END_MSG_MAP()

struct SQExpeditionRequest : public SExpeditionMsg
{
	SQExpeditionRequest()
	{
		SQExpeditionRequest::_protocol = REQUEST_BATTLE_RESULT_RESOLVES;
	}
};

struct SQExpeditionPermissionRequest : public SExpeditionMsg
{
	DWORD selectedHero[g_iExpeditionCharacterLimit];

	SQExpeditionPermissionRequest()
	{
		SExpeditionMsg::_protocol = REQUEST_ENTER_BATTLE;
		for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
		{
			selectedHero[i] = 0;
		}
	}
};

struct SQUpdateSelectedHeroData : public SExpeditionMsg
{
	DWORD selectedHeroMap[g_iExpeditionCharacterLimit];///<存储了玩家参战武将的ID
	float heroHealthScale[g_iExpeditionCharacterLimit];
	float heroManaScale[g_iExpeditionCharacterLimit];

	SQUpdateSelectedHeroData()
	{
		SExpeditionMsg::_protocol = REQUEST_UPDATE_SELECTEDCHARACTER_INFOR;
		for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
		{
			selectedHeroMap[i] = 0;
		}
		for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
		{
			heroHealthScale[i] = 0;
		}
		for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
		{
			heroManaScale[i] = 0;
		}
	}
};

struct SQUpdateEnemyData : public SExpeditionMsg
{
	DWORD machedEnemyMap[g_iExpeditionCharacterLimit];///<存储了敌方参战武将的ID
	float enemyHealthScale[g_iExpeditionCharacterLimit];
	float enemyManaScale[g_iExpeditionCharacterLimit];

	SQUpdateEnemyData()
	{
		SExpeditionMsg::_protocol = REQUEST_UPDATE_ENEMYCHARACTER_INFOR;
		for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
		{
			machedEnemyMap[i] = 0;
		}
		for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
		{
			enemyHealthScale[i] = 0;
		}
		for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
		{
			enemyManaScale[i] = 0;
		}
	}
};

struct SABattleResolve : public SExpeditionMsg
{
	bool bIsHero;
	DWORD obtainedItemID;
	SABattleResolve()
	{
		SExpeditionMsg::_protocol = SExpeditionMsg::ANSWER_BATTLE_RESOLVES_RESULT;
		bIsHero = false;
		obtainedItemID = 0;
	}
};

struct SAPermissionInfor : public SExpeditionMsg
{
	BYTE checkResult;

	SAPermissionInfor()
	{
		SExpeditionMsg::_protocol = SExpeditionMsg::ANSWER_PERMISSION_INFOR;
		checkResult = 0;
	}
};

struct SAHostileEnemyInfor : public SExpeditionMsg
{
	float levelFactor;///关卡的难度系数
	DWORD curLevelInstance;
	SHeroData enmeyInfo[g_iExpeditionCharacterLimit];
	SAHostileEnemyInfor()
	{
		levelFactor = 1;
		curLevelInstance = 0;
		SExpeditionMsg::_protocol = SExpeditionMsg::ANSWER_HOSTILEPLAYER_INFOR;
	}
};

struct SAConqured : public SExpeditionMsg
{
	SAConqured()
	{
		SExpeditionMsg::_protocol = SExpeditionMsg::ANSWER_EXPEDITION_CONQURED;
	}
};

struct SAExpeditionRefresh : public SExpeditionMsg
{
	SAExpeditionRefresh()
	{
		SExpeditionMsg::_protocol = SExpeditionMsg::ANSWER_REFRESH_TICKET;
	}
};

///<@brief 萌斗三国的普通副本（又名故事副本）、精英副本、将神传说副本、秘境寻宝副本的消息基类
DECLARE_MSG_MAP(SDuplicateMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_DUPLICATE)
REQUEST_ENTER_SPECIFY_TOLLGATE = 0,
REQUEST_MOP_UP_TOLLGATE,
REQUEST_START_CHALLENGE,
REQUEST_EARNING_CLEAR,
REQUEST_RESET_TOLLGATE,
REREQUEST_TOLLGATE_DROPED_ITEM, ///<重新获取关卡的掉落物品
ANSWER_REFRESH_ALL_TICKET,
ANSWER_ACTIVATE_DUPLICATE,
ANSWER_TOLLGATE_DROPED_ITEM,
ANSWER_RESET_SPECIFY_TOLLGATE,
ANSWER_TOLLGATE_EARNING_CLEARED,
ANSWER_ENTER_TOLLGATE_PERMISSION_INFOR,
END_MSG_MAP()

///@brief 表明是哪一个副本的消息
struct SDuplicateTypeMsg : public SDuplicateMsg
{
	byte duplicateType;

	SDuplicateTypeMsg()
	{
		duplicateType = InstanceType::Story;
	}
};

///@brief 表明具体是哪一个副本中的哪一个关卡
struct SSpecifyTollgateMsg : public SDuplicateTypeMsg 
{
	int duplicateID;
	int tollgateID;

	SSpecifyTollgateMsg()
	{
		duplicateID = 0;
		tollgateID = 0;
	}
};

///@brief 客户端发来的请求进入某一个关卡的消息
struct SQEnterSpecifyTollgate : public SSpecifyTollgateMsg
{
	int selectedHeroNum;
	int monsterCachingNum;
	int requestData[];

	SQEnterSpecifyTollgate()
	{
		_protocol = REQUEST_ENTER_SPECIFY_TOLLGATE;
		selectedHeroNum = 0;
	}
};

///@brief 客户端发来消息请求扫荡某一个关卡
struct SQMopUpTollgate : public SSpecifyTollgateMsg
{
	int sweepTicketID;///<扫荡劵的ID
	int challengeTimes;

	SQMopUpTollgate()
	{
		_protocol = REQUEST_MOP_UP_TOLLGATE;
		challengeTimes = 0;
		sweepTicketID = 0;
	}
};

///@brief 客户端开始挑战对应的副本关卡
struct SQStartToChallengeDuplicate : public SDuplicateTypeMsg
{
	SQStartToChallengeDuplicate()
	{
		_protocol = REQUEST_START_CHALLENGE;
	}
};

///@brief 客户端申请关卡结算
struct SQDuplicateEarningClear : public SSpecifyTollgateMsg
{
	int combatGrade;///<战斗评分

	SQDuplicateEarningClear()
	{
		_protocol = REQUEST_EARNING_CLEAR;
		combatGrade = 0;
	}
};

///@brief 客户端申请重置某一个关卡
struct SQResetTollgate : public SSpecifyTollgateMsg
{
	SQResetTollgate()
	{
		_protocol = REQUEST_RESET_TOLLGATE;
	}
};

struct SAResetSpecifyTollgate : public SSpecifyTollgateMsg
{
	bool bPermited;

	SAResetSpecifyTollgate()
	{
		bPermited = false;
		_protocol = ANSWER_RESET_SPECIFY_TOLLGATE;
	}
};

///@brief 告诉客户端是否可以挑战他指定的关卡
struct SAChallengePermissionInfor : public SDuplicateTypeMsg
{
	bool permitted;
	bool bMopUpOperation;
	int monsterNum;
	MonsterData monsterData[MAX_TOLLGATE_MONSTER_NUM];
	SAChallengePermissionInfor()
	{
		permitted = false;
		bMopUpOperation = false;
		monsterNum = 0;
		memset(monsterData, 0, sizeof(monsterData));
		_protocol = ANSWER_ENTER_TOLLGATE_PERMISSION_INFOR;
	}
};

///@brief 发给客户端来刷新萌斗三国的普通副本（又名故事副本）、精英副本、将神传说副本、秘境寻宝副本的挑战次数等信息
struct SARefreshDuplicateAllTicket : public SDuplicateMsg
{
	SARefreshDuplicateAllTicket()
	{
		_protocol = SDuplicateMsg::ANSWER_REFRESH_ALL_TICKET;
	}
};

///@brief 发给客户端用来激活某一个副本，暂时服务器激活精英副本没有调用此消息，客户端有相应的激活精英副本的逻辑，到时候视情况而定
struct SAActivateDuplicate : public SDuplicateTypeMsg
{
	bool bExclusivity; ///<是否是排他性的，即只有duplicateIDs里面指定的副本才激活
	int duplicateNum;
	int duplicateIDs[MAX_LEAGUEOFLEGEND_DUPILICATE_NUM];

	SAActivateDuplicate()
	{
		bExclusivity = true;
		duplicateNum = 0;
		_protocol = SDuplicateMsg::ANSWER_ACTIVATE_DUPLICATE;
	}
};

///@brief 告诉客户端本次关卡要掉落的物品
struct SATollgateDropedItem : public SSpecifyTollgateMsg
{
	bool bMopUpOperation;
	int commonItemNum;///普通物品有多少种类
	int bossItemNum;///boss掉落的物品有多少种类
	int arrayBossItemID[MAX_BOSS_DROPCOUNT];///<暂时写死boss会掉落的物品数
	int arrayBossItemCount[MAX_BOSS_DROPCOUNT];///<暂时写死boss会掉落的物品数
	int arrayCommonItemList[MAX_ITEM_NUM_SYN_TO_CLIENT];
	SATollgateDropedItem()
		:SSpecifyTollgateMsg()
	{
		commonItemNum = 0;
		bossItemNum = 0;
		bMopUpOperation = false;
		_protocol = SDuplicateMsg::ANSWER_TOLLGATE_DROPED_ITEM;
		memset(arrayBossItemID, 0, sizeof(int) * MAX_BOSS_DROPCOUNT);
		memset(arrayBossItemCount, 0, sizeof(int) * MAX_BOSS_DROPCOUNT);
		memset(arrayCommonItemList, 0, sizeof(int) * MAX_ITEM_NUM_SYN_TO_CLIENT);
	}
};

struct SHeroExpInfor
{
	DWORD m_dwHeroID;///>武将索引
	DWORD m_dwHeroExp;
	DWORD m_dwIncreasedExp;
	DWORD m_dwHeroLevel;
};

///告诉客户端，服务器端已经结算
struct SATollgateEarningClearedInfor : public SSpecifyTollgateMsg
{
	bool bcleared;
	bool bMopUpOperation;
	int heroNum;
	SHeroExpInfor heroExpInfor[MAX_TOLLGATE_HERO_NUM];
	SATollgateEarningClearedInfor()
		:SSpecifyTollgateMsg()
	{
		bcleared = false;
		bMopUpOperation = false;
		heroNum = 0;
		_protocol = SDuplicateMsg::ANSWER_TOLLGATE_EARNING_CLEARED;
	}
};

//======================武将的扩展性新玩法的消息定义
DECLARE_MSG_MAP(SHeroExtendsGamplayMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_HEROEXTENDSGAMEPLAY)
HERO_HUNGING,///英雄挂机寻宝
HERO_TRAINING,///英雄训练
END_MSG_MAP()

/// 玩家武将的扩展性玩法的消息基类
struct HeroHungingHuntingMsg : SHeroExtendsGamplayMsg
{
	HeroHungingHuntingMsg()
	{
		SHeroExtendsGamplayMsg::_protocol = SHeroExtendsGamplayMsg::HERO_HUNGING;
		heroID = 0;
	}

	enum
	{
		REQUEST_START_HUNTING, ///挂机寻宝的申请
		REQUEST_HUNTING_LIQUIDATION,///收益结算的申请
		REQUEST_SURPRISE_PRESENTS,///申请掉宝
		ANSWER_HUNTING_PERMISSION,///是否允许挂机寻宝的消息
		ANSWER_HUNTING_LIQUIDATION_RESULT,///寻宝收益结算是否成功的消息
		ANSWER_SURPRISE_PRESENT,///寻宝过程中的意外奖励的同步
	};

	BYTE _protocol;
	int heroID;
};

///对客户端发来的武将英雄挂机寻宝的一些请求的简单回复
struct AnswerHeroHuntingApplyMsg : HeroHungingHuntingMsg
{
	bool bSucceed;

	explicit AnswerHeroHuntingApplyMsg(BYTE protocolType)
	{
		bSucceed = false;
		HeroHungingHuntingMsg::_protocol = protocolType;
	}
};

struct AnswerSynSurprisePresentsMsg : HeroHungingHuntingMsg
{
	int itemType; ///发给客户端的物品的类型，跟AddGoods相关的物品类型严格一致
	int itemID; ///发给客户端的物品的ID，如果是元宝、金钱等，该ID为0
	int itemNum; ///发给客户端的物品的数量

	AnswerSynSurprisePresentsMsg()
	{
		HeroHungingHuntingMsg::_protocol = HeroHungingHuntingMsg::ANSWER_SURPRISE_PRESENT;
		itemType = 0;
		itemID = 0;
		itemNum = 0;
	}
};

/// 玩家训练相关武将的的相关消息基类
struct HeroTrainingMsg : SHeroExtendsGamplayMsg
{
	HeroTrainingMsg()
	{
		SHeroExtendsGamplayMsg::_protocol = SHeroExtendsGamplayMsg::HERO_TRAINING;
		heroID = 0;
		trainingFlag = 0;
	}

	enum
	{
		REQUEST_START_TRAINING,///申请训练武将
		REQUEST_TRAINING_LIQUIDATION,///申请进行训练收益的结算
		ANSWER_TRAINING_PERMISSION,///申请训练武将
		ANSWER_TRAINING_LIQUIDATION_RESULT,///训练的收益的结算是否成功的消息
	};

	BYTE _protocol;
	int heroID;
	BYTE trainingFlag;
};

//======================武将升级相关协议定义
DECLARE_MSG_MAP(SHeroUpgrade, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_HEROUPGRADE)
HeroUpgrade_AttachEquipment,
HeroUpgrade_RankRise,
HeroUpgrade_StarLevelRise,
HeroUpgrade_ConscribeHero,
HeroUpgrade_UpgradeLevel,
END_MSG_MAP()

struct  SBaseHeroUpgrade : public SHeroUpgrade
{
	DWORD m_HeroID;
	SBaseHeroUpgrade()
	{
		m_HeroID = 0;
	}
};


struct SQHeroAttachEquipment : public SBaseHeroUpgrade
{
	DWORD m_EquipmentNum;
	DWORD m_EquipmentID[];
	SQHeroAttachEquipment()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_AttachEquipment;
		m_EquipmentNum = 0;
	}
}; 


struct SAHeroAttachEquipment : public SBaseHeroUpgrade
{
	BYTE m_SuccessFlag;
	DWORD m_EquipmentNum;
	DWORD m_EquipmentIDs[EQUIP_MAX];
	SAHeroAttachEquipment()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_AttachEquipment;
		m_EquipmentNum = 0;
		m_SuccessFlag = 0;
	}
};

struct SQHeroRankRiseRequest : public SBaseHeroUpgrade
{
	DWORD m_CurRank;
	SQHeroRankRiseRequest()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_RankRise;
		m_CurRank = 0;
	}
};

struct SAHeroRankRiseResult : public SBaseHeroUpgrade
{
	BYTE m_SuccessFlag;
	SAHeroRankRiseResult()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_RankRise;
		m_SuccessFlag = 0;
	}
};

struct SQHeroStarLevelRiseRequest : public SBaseHeroUpgrade
{
	DWORD m_CurStarLevel;
	SQHeroStarLevelRiseRequest()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_StarLevelRise;
		m_CurStarLevel = 0;
	}
};

struct SAHeroStarLevelRiseResult : public SBaseHeroUpgrade
{
	DWORD dwMoney;
	bool bSuccess;
	SAHeroStarLevelRiseResult()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_StarLevelRise;
		dwMoney = 0;
		bSuccess = false;
	}
};

///<客户端招募武将请求//
struct SQConscribeHeroRequest : public SBaseHeroUpgrade
{
	SQConscribeHeroRequest()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_ConscribeHero;
	}
};

///<返回招募武将结果//
struct SAConscribeHeroResult : public SBaseHeroUpgrade
{
	DWORD dwSoulStoneID;
	DWORD dwSoulStoneNum;
	DWORD dwMoney;
	SAConscribeHeroResult()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_ConscribeHero;
	}
};

struct SQHeroUpgradeLevelRequest : public SBaseHeroUpgrade
{
	DWORD dwExpPotionID;
	DWORD dwExpPotionNum;
	SQHeroUpgradeLevelRequest()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_UpgradeLevel;
	}
};

struct SAHeroUpgradeLevelResult : public SBaseHeroUpgrade
{
	DWORD dwLevel;
	DWORD dwExperienceAmount;
	SAHeroUpgradeLevelResult()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_UpgradeLevel;
	}
};

DECLARE_MSG_MAP(SStorageBaseMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_STORAGE)
SStorage_None,
SStorage_SellItem,
REQUEST_USECONSUMABLE,///申请使用消耗品
ANWSER_USECONSUMABLE,///使用消耗品的回复
REQUEST_SYNTHETIZE,
ANWSER_SYNTHETIZEINFOR,
REQUEST_RESOLVEITEM,///申请分解物品
ANSWER_RESOLVEITEM,///分解物品的回复
END_MSG_MAP()

///@brief 客户端发来请求合成装备
struct SQSynthetizeItem : SStorageBaseMsg
{
	DWORD synthetizeTimes;
	DWORD debrisID;

	SQSynthetizeItem()
	{
		synthetizeTimes = 0;
		debrisID = 0;
		_protocol = SStorageBaseMsg::REQUEST_SYNTHETIZE;
	}
};

///@brief 装备合成是否成功的消息
struct SASynthetizeItemInfor : SStorageBaseMsg
{
	bool bsucceed;

	SASynthetizeItemInfor()
	{
		bsucceed = false;
		_protocol = SStorageBaseMsg::ANWSER_SYNTHETIZEINFOR;
	}
};

///@brief 背包物品的处理消息
struct SStorageItemProcess : SStorageBaseMsg
{
	DWORD m_dwItemID;
	DWORD m_dwProcessCount;

	SStorageItemProcess()
	{
		m_dwItemID = 0;
		m_dwProcessCount = 0;
	}
};

struct SUseConsumableAnswer : SStorageBaseMsg
{
	DWORD itemNum;
	DWORD itemList[MAX_ITEM_NUM_GAINED_FROM_CHEST];///存了3部分信息，前itemNum位为物品的类型，中itemNum位为物品的ID，后itemNum位为物品的数量

	SUseConsumableAnswer()
	{
		itemNum = 0;
		memset(itemList, 0, sizeof(DWORD) * MAX_ITEM_NUM_GAINED_FROM_CHEST);
		_protocol = SStorageBaseMsg::ANWSER_USECONSUMABLE;
	}
};

//<@brief 接收客户端请求出售物品消息//
struct SRequstSellItem : SStorageItemProcess
{
	SRequstSellItem()
	{
		SStorageBaseMsg::_protocol = SStorageBaseMsg::SStorage_SellItem;
		m_dwItemID = 0;
		m_dwProcessCount = 0;
	}
};

//<@brief 处理出售物品请求的结果//
struct SSellItemResultMsg : SStorageBaseMsg
{
	BYTE m_CurrencyType;	//<收益币种//
	DWORD m_dwProperty;		//<币种数据//
	DWORD m_dwItemID;		//<物品ID//
	DWORD m_dwSellCount;	//<出售个数//
	SSellItemResultMsg()
	{
		SStorageBaseMsg::_protocol = SStorageBaseMsg::SStorage_SellItem;
		m_CurrencyType = 0;
		m_dwItemID = 0;
		m_dwSellCount = 0;
		m_dwProperty = 0;
	}
};

///@brief 申请分解物品
struct SRequestResolveItemMsg : SStorageBaseMsg
{
	int itemArrayNum;
	int resolveItemArray[];

	SRequestResolveItemMsg()
	{
		SStorageBaseMsg::_protocol = SStorageBaseMsg::REQUEST_RESOLVEITEM;
		itemArrayNum = 0;
	}
};

///@brief 申请分解物品后的回复
struct SAnswerResolveItemMsg : SStorageBaseMsg
{
	bool bSuccess;

	SAnswerResolveItemMsg()
	{
		SStorageBaseMsg::_protocol = SStorageBaseMsg::ANSWER_RESOLVEITEM;
		bSuccess = false;
	}
};

DECLARE_MSG_MAP(SMallBaseMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_MALL)
MALL_BUYCOMMODITY,			///< 购买商品
MALL_REFRESH,				///< 手动刷新商品
MALL_AUTOREFRESH,			///< 自动刷新商品
MALL_OPENMALL,				///< 开放商城
END_MSG_MAP()

struct SMallMsgInfo : SMallBaseMsg
{
	BYTE m_MallType;	///<商店类型/
	SMallMsgInfo()
	{
		m_MallType = 0;
	}
};

struct SBuyCommodityRequest : SMallMsgInfo
{
	DWORD m_dwIndex;	///<商品索引/
	SBuyCommodityRequest()
	{
		SMallBaseMsg::_protocol = SMallBaseMsg::MALL_BUYCOMMODITY;
		m_MallType = 0;
		m_dwIndex = 0;
	}
};

struct SBuyCommodityResult : SMallMsgInfo
{
	BYTE m_CurrencyType;	///<货币类型/
	DWORD m_dwProperty;		///<货币消耗/
	DWORD m_dwIndex;		///<商品索引/
	SBuyCommodityResult()
	{
		SMallBaseMsg::_protocol = SMallBaseMsg::MALL_BUYCOMMODITY;
		m_CurrencyType = 0;
		m_dwProperty = 0;
		m_dwIndex = 0;
	}
};

struct SRefreshCommodityRequest : SMallMsgInfo
{
	SRefreshCommodityRequest()
	{
		SMallBaseMsg::_protocol = SMallBaseMsg::MALL_REFRESH;
	}
};

struct SRefreshCommodityResult : SMallMsgInfo
{
	DWORD m_dwProperty;
	DWORD m_dwNextRefreshTime;
	DWORD m_dwRefreshedCount;
	SSanguoCommodity m_arrCommodity[MALL_COMMODITY_NUM];
	SRefreshCommodityResult()
	{
		SMallBaseMsg::_protocol = SMallBaseMsg::MALL_REFRESH;
		m_dwProperty = 0;
		m_dwNextRefreshTime = 0;
		memset(m_arrCommodity, 0, sizeof(SSanguoCommodity) * MALL_COMMODITY_NUM);
	}
};

struct SAutoRefreshCommodityMsg : SMallMsgInfo
{
	DWORD m_dwNextRefreshTime;
	DWORD m_dwNextRefreshRemainingSeconds;
	SSanguoCommodity m_arrCommodity[MALL_COMMODITY_NUM];
	SAutoRefreshCommodityMsg()
	{
		SMallBaseMsg::_protocol = SMallBaseMsg::MALL_AUTOREFRESH;
		m_dwNextRefreshTime = 0;
		m_dwNextRefreshRemainingSeconds = 0;
		memset(m_arrCommodity, 0, sizeof(SSanguoCommodity) * MALL_COMMODITY_NUM);
	}
};

///开启商城消息
struct SOpenMallMsg : SMallMsgInfo
{
	SOpenMallMsg()
	{
		SMallBaseMsg::_protocol = SMallBaseMsg::MALL_OPENMALL;
	}
};

DECLARE_MSG_MAP(SNewbieGuideMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_NEWGUIDE)
Guide_SetInfo,		///设置引导信息
Guide_ChangeScene,	///战斗引导完成切换场景
Guide_NextStep,		///通知客户端指引下一步
END_MSG_MAP()

struct SQSetNewbieGuideInfo : SNewbieGuideMsg
{
	BYTE index;
	SNewbieGuideDInfo info;
	SQSetNewbieGuideInfo()
	{
		SNewbieGuideMsg::_protocol = SNewbieGuideMsg::Guide_SetInfo;
		index = 0;
	}
};

///@新手战斗完成 切换场景
struct SChangeScene : SNewbieGuideMsg
{
	SChangeScene()
	{
		SNewbieGuideMsg::_protocol = SNewbieGuideMsg::Guide_ChangeScene;
	}
};

///@通知客户端 指引前往下一步
struct SAGoNextStep : SNewbieGuideMsg
{
	SAGoNextStep()
	{
		SNewbieGuideMsg::_protocol = SNewbieGuideMsg::Guide_NextStep;
	}
};

///<同步相关,得到东西,后台推送至前台,主要为addgoods相关,如铜钱元宝等
struct SAAddGoodsSYNMsg : SSGPlayerMsg
{
	bool bIDIndex;		///要同步的物品的是否是以ID来索引的，如果为真，那么itemList中就分成了两部分，如果为假，则itemList就为指定的Goods的数量
	DWORD iType;
	DWORD itemGroupNum;	///要同步的物品的类型的数量
	DWORD itemList[MAX_ITEM_NUM_SYN_TO_CLIENT];///每个物品对应的ID和对应的个数，前itemGroupNum元素为ID，紧接着的itemGroupNum为对应的个数，可为nullptr,最多支持64个物品的同时添加
	SAAddGoodsSYNMsg()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_ADDGOODS_SYN;
		iType = 0;
		itemGroupNum = 0;
	}
};

///<同步相关,扣除东西,后台推送至前台,主要为Decgoods相关,如铜钱元宝等
struct SADecGoodsSYNMsg : SSGPlayerMsg
{
	bool bIDIndex;		///要同步的物品的是否是以ID来索引的，如果为真，那么itemList中就分成了两部分，如果为假，则itemList就为指定的Goods的数量
	DWORD iType;
	DWORD itemGroupNum;	///要同步的物品的类型的数量
	DWORD itemList[128];///每个物品对应的ID和对应的个数，前itemGroupNum元素为ID，紧接着的itemGroupNum为对应的个数，可为nullptr,最多支持64个物品的同时删除
	SADecGoodsSYNMsg()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_DECGOODS_SYN;
		iType = 0;
		itemGroupNum = 0;
	}
};

//=====================武将技能升级(包含技能点的扣除 购买等)==================
DECLARE_MSG_MAP(SSkillUpgrade, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_SKILLUPGRADE)
SkillUpgrade_LearnSkill,
SkillUpgrade_BuyPoint,
SkillUpgrade_CheckPoint,
END_MSG_MAP()

struct  SQUpgradeHeroSkill : public SSkillUpgrade
{
	DWORD dwHeroID;
	//DWORD dwValidNum;
	//DWORD dwSkillID[MAX_SKILLNUM];
	//DWORD dwSkillLevel[MAX_SKILLNUM];
	DWORD dwSkillID;
	DWORD dwSkillLevel;
	SQUpgradeHeroSkill()
	{
		SSkillUpgrade::_protocol = SkillUpgrade_LearnSkill;
		dwHeroID = 0;
		//dwValidNum = 0;
		dwSkillID = 0;
		dwSkillLevel = 0;
	}
};

///@breif 反馈武将技能升级
struct  SAUpgradeHeroSkillResult : public SSkillUpgrade
{
	DWORD dwHeroID;
	DWORD dwValidNum;
	DWORD dwSkillPointCost;
	DWORD dwSkillPoint;
	DWORD dwRemainingTime;
	DWORD dwSkillID[MAX_SKILLNUM];
	DWORD dwSkillLevel[MAX_SKILLNUM];
	DWORD dwMoney;
	bool bResult;
	SAUpgradeHeroSkillResult()
	{
		SSkillUpgrade::_protocol = SkillUpgrade_LearnSkill;
		dwHeroID = 0;
		dwValidNum = 0;
		dwSkillPointCost = 0;
		dwSkillPoint = 0;
		dwRemainingTime = 0;
		memset(dwSkillID, 0, sizeof(DWORD) * MAX_SKILLNUM);
		memset(dwSkillLevel, 0, sizeof(DWORD) * MAX_SKILLNUM);
		dwMoney = 0;
		bResult = false;
	}
};

///<接收购买技能点结构
struct SQBuySkillPoint : public SSkillUpgrade
{
	SQBuySkillPoint()
	{
		SSkillUpgrade::_protocol = SkillUpgrade_BuyPoint;
	}
};

///<反馈技能点购买结构
struct SABuySkillPointResult : public SSkillUpgrade
{
	DWORD dwSkillPoint;
	DWORD dwRemainingTime;
	DWORD dwPurchasedTimes;
	DWORD dwDiamond;
	bool bResult;
	SABuySkillPointResult()
	{
		SSkillUpgrade::_protocol = SkillUpgrade_BuyPoint;
		dwSkillPoint = 0;
		dwRemainingTime = 0;
		dwPurchasedTimes = 0;
		dwDiamond = 0;
		bResult = false;
	}
};

struct SQCheckSkillPoint : public SSkillUpgrade
{
	SQCheckSkillPoint()
	{
		SSkillUpgrade::_protocol = SkillUpgrade_CheckPoint;
	}
};

struct SACheckSkillPoint : public SSkillUpgrade
{
	DWORD dwSkillPoint;
	DWORD dwRemainingTime;
	SACheckSkillPoint()
	{
		SSkillUpgrade::_protocol = SkillUpgrade_CheckPoint;
		dwSkillPoint = 0;
		dwRemainingTime = 0;
	}
};

//签到相关
DECLARE_MSG_MAP(SCHECKINBaseMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_CHECKIN)
CHECKIN_Quest,
CHECKIN_DATASYN,//数据同步
END_MSG_MAP()


struct SQCHECKIN_Quest : SCHECKINBaseMsg
{
	BYTE itype; //1每日免费,2补签
	SQCHECKIN_Quest()
	{
		SCHECKINBaseMsg::_protocol = SCHECKINBaseMsg::CHECKIN_Quest;
	}
};

//数据同步,登陆时签到后使用
struct SACHECKIN_DATASYN : SCHECKINBaseMsg
{
	DWORD beginTime;
	DWORD lastTime;
	DWORD lastCount;
	DWORD addCount;
	/*DWORD turn;
	DWORD days;*/
	SACHECKIN_DATASYN()
	{
		SCHECKINBaseMsg::_protocol = SCHECKINBaseMsg::CHECKIN_DATASYN;
	}
};

//脚本消息
DECLARE_MSG_MAP(SSCRIPTMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_SCRIPT)
DATASYN_FIRST,//首次登陆数据同步
DATASYN_NOTIFICATION,//登陆的时候同步系统推送通知的数据
DATASYN_MISSION,///等路段额时候同步每日任务的数据到客户端
DATASYN_HERO_EXTENDS_DATAS,///<英雄额外的数据，比如武将挂机寻宝的数据，武将训练的数据
END_MSG_MAP()
struct SADATASYN_FIRST : SSCRIPTMsg
{
	DWORD lenth;
	DWORD sdata[MAX_SYN_SCRIPT_LENTH];
	SADATASYN_FIRST()
	{
		SSCRIPTMsg::_protocol = SSCRIPTMsg::DATASYN_FIRST;
	}
};

/// <summary>
/// 通知数据结构。
/// </summary>
struct SNotificationData
{
	/// <summary>
	/// 通知的ID
	/// </summary>
	int notificationID;
	/// <summary>
	/// 通知事件会激活的具体时间，为发过来的秒数（从1970-1-1 00：00：00开始算起）
	/// </summary>
	int alarmTime;
	/// <summary>
	/// 通知标题
	/// </summary>
	char notificationTitle[64];
	/// <summary>
	/// 通知内容
	/// </summary>
	char notificationContents[128];

	SNotificationData()
	{
		alarmTime = 0;
		notificationID = 0;
		memset(notificationTitle, 0, sizeof(notificationTitle));
		memset(notificationContents, 0, sizeof(notificationContents));
	}
};

///同步系统推送通知数据到客户端
struct SDATASYN_NOTIFICATION : SSCRIPTMsg
{
	int length;
	SNotificationData datas[MAX_SYN_NOTIFICATION_LENGTH];

	SDATASYN_NOTIFICATION()
	{
		length = 0;
		SSCRIPTMsg::_protocol = SSCRIPTMsg::DATASYN_NOTIFICATION;
	}
};

///同步任务数据到客户端
struct SDATASYN_MISSION : SSCRIPTMsg
{
	int length;
	int datas[MISSION_NUM];

	SDATASYN_MISSION()
	{
		length = 0;
		SSCRIPTMsg::_protocol = SSCRIPTMsg::DATASYN_MISSION;
	}
};

///同步英雄的扩展性玩法数据，比如英雄的挂机寻宝的数据，训练的数据
struct SDATASYN_HEROEXTENDSGAMEPLAY : SSCRIPTMsg
{
	int length;
	int datas[MAX_HERO_NUM * 2];///前length位为玩家武将扩展玩法的基本信息，后length位为玩家武将开启某个扩展性玩法的时间

	SDATASYN_HEROEXTENDSGAMEPLAY()
	{
		length = 0;
		memset(datas, 0, sizeof(datas));
		SSCRIPTMsg::_protocol = SSCRIPTMsg::DATASYN_HERO_EXTENDS_DATAS;
	}
};

DECLARE_MSG_MAP(SChaseThiefMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_CHASETHIEF)
CHASETHIEF_ACTIVATE,	//激活追击盗贼
CHASETHIEF_PERPATIONCHANLLENGE,	//挑战准备
CHASETHIEF_STARTCHANLLENGE, //开始战斗
CHASETHIEF_REFUSALENTER,	//拒绝进入战斗
CHASETHIEF_CLEARING,	//结算
CHASETHIEF_REWARD,		//战斗奖励
END_MSG_MAP()

struct SThiefActivate : SChaseThiefMsg
{
	DWORD dwRemainingTime;
	SThiefActivate()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_ACTIVATE;
		dwRemainingTime = 0;
	}
};

struct SThiefRequestPerpationChanllenge : SChaseThiefMsg
{
	SThiefRequestPerpationChanllenge()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_PERPATIONCHANLLENGE;
	}
};

struct SThiefAnswerPerpationChanllenge : SChaseThiefMsg
{
	bool permission;
	int tollgateID;
	SThiefAnswerPerpationChanllenge()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_PERPATIONCHANLLENGE;
		permission = false;
		tollgateID = 0;
	}
};

struct SThiefRequestStartChanllenge : SChaseThiefMsg
{
	int heroes[MAX_THIEF_HERO_LIMIT];
	SThiefRequestStartChanllenge()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_STARTCHANLLENGE;
		memset(heroes, 0, sizeof(int) * MAX_THIEF_HERO_LIMIT);
	}
};

struct SThiefAnswerStartChanllenge : SChaseThiefMsg
{
	int commonItemNum;///普通物品有多少种类
	int bossItemNum;///boss掉落的物品有多少种类
	int arrayBossItemID[MAX_BOSS_DROPCOUNT];///<暂时写死boss会掉落的物品数
	int arrayBossItemCount[MAX_BOSS_DROPCOUNT];///<暂时写死boss会掉落的物品数
	int arrayCommonItemList[MAX_ITEM_NUM_SYN_TO_CLIENT];
	SThiefAnswerStartChanllenge()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_STARTCHANLLENGE;
		commonItemNum = 0;
		bossItemNum = 0;
		memset(arrayBossItemID, 0, sizeof(int) * MAX_BOSS_DROPCOUNT);
		memset(arrayBossItemCount, 0, sizeof(int) * MAX_BOSS_DROPCOUNT);
		memset(arrayCommonItemList, 0, sizeof(int) * MAX_ITEM_NUM_SYN_TO_CLIENT);
	}
};

struct SThiefRefusalEnter : SChaseThiefMsg
{
	bool loseEffectiveness;
	SThiefRefusalEnter()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_REFUSALENTER;
		loseEffectiveness = false;
	}
};

struct SThiefRequestClearing : SChaseThiefMsg
{
	bool win;
	SThiefRequestClearing()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_CLEARING;
		win = false;
	}
};

struct SThiefAnswerClearing : SChaseThiefMsg
{
	bool complete;
	SThiefAnswerClearing()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_CLEARING;
		complete = false;
	}
};

struct SThiefFightRewardMsg : SChaseThiefMsg
{
	int rewardItemNum;
	int items[MAX_THIEF_REWARD_NUM];
	SThiefFightRewardMsg()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_REWARD;
		rewardItemNum = 0;
		memset(items, 0, sizeof(int) * MAX_THIEF_REWARD_NUM);
	}
};


///@客户端请求更名消息
struct  SQChangePlayerName : SSGPlayerMsg
{	 
	char name[CONST_USERNAME];
	SQChangePlayerName()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_CHANGENAME;
	}
	
};
///@客户端更名消息回复
struct  SAChangeName : SSGPlayerMsg
{
	SAChangeName()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_CHANGENAME;
	}
	bool falg;
	int ChangeNameCount;

};
///@breif 客户端请求获取活动奖励
struct SQGetActReward : SSGPlayerMsg
{
	int actType;///<活动类型
	int reawrdIndex; ///<奖励索引
	SQGetActReward()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_ACTIVITY;
		actType = 0;
		reawrdIndex = 0;
	}
};


///@breif 客户端反馈获取活动奖励结果
struct SAGetActReward : SSGPlayerMsg
{
	int actType; ///<活动类型
	int reawrdIndex;	 ///奖励索引
	byte result;			///<领取活动成功返回1，失败返回0
	SAGetActReward()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_ACTIVITY;
		result = 0;
		reawrdIndex = 0;
	}
};


///小型奖励所相关的一些消息
DECLARE_MSG_MAP(SRewardState, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_REWARDSTATE)
CHARGESUM,
END_MSG_MAP()

///@breif 领取累计充值活动奖励的数据
struct SARewardState : SRewardState
{
	char rewardState[20]; //'0'为未领取， '1'为已领取
	DWORD chargeSum; //玩家本次活动充值总数
	SARewardState()
	{
		SRewardState::_protocol = SRewardState::CHARGESUM;
		memset(rewardState, 0, sizeof(char) * 20);
		chargeSum = 0;
	}
};

///@brief 元宝兑换其他物件的消息
DECLARE_MSG_MAP(SRecharge, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_RECHARGE)
SANGUO_RECHARGE,
SANGUO_RECHARGE_REQUEST,
SANGUO_RECHARGE_RESET,
SANGUO_MONTHLY_RECHARGE_RESULT,
END_MSG_MAP()


///@breif 客户端发送充值信息
struct SQRecharge : SRecharge
{
	DWORD dwNowPoint;//玩家总余额
	WORD wPlatfrom;//平台 1 微信平台 2 QQ平台
	char addr[MAX_ADDR_HTTP];
	SQRecharge()
	{
		SRecharge::_protocol = SRecharge::SANGUO_RECHARGE;
	}
};


///@breif 服务端反馈充值信息
struct SARecharge : SRecharge
{
	int ret;//充值校验结果:1成功,2失败
	unsigned int nowPoint;//玩家总余额
	SARecharge()
	{
		SRecharge::_protocol = SRecharge::SANGUO_RECHARGE;
	}
};

///@breif 服务端反馈购买月卡
struct SABuyMembership : SRecharge
{
	BYTE byType;//充值校验结果:1成功,-1失败, 0同步
	DWORD memberShipData;//月卡到期时间
	SABuyMembership()
	{
		byType = 0;
		memberShipData = 0;
		SRecharge::_protocol = SRecharge::SANGUO_MONTHLY_RECHARGE_RESULT;
	}
};

struct SQResetRecharge : SRecharge //重置充值
{
	SQResetRecharge()
	{
		SRecharge::_protocol = SRecharge::SANGUO_RECHARGE_RESET;
	}
};

///@breif 充值请求
struct SQRechargeRequst : SRecharge
{
	BYTE byType; //  1 购买游戏币 ，2 购买月卡
	DWORD dwDiamond; //花费元宝数
	SQRechargeRequst()
	{
		byType = -1;
		dwDiamond = 0;
		SRecharge::_protocol = SRecharge::SANGUO_RECHARGE_REQUEST;
	}
	
};

///@breif 充值反馈
struct SARechargeRequst : SRecharge
{
	bool bResult;
	BYTE byType; //  1 购买游戏币 ，2 购买月卡
	DWORD dwDiamond; //花费元宝数
	SARechargeRequst()
	{
		bResult = false;
		byType = -1;
		SRecharge::_protocol = SRecharge::SANGUO_RECHARGE_REQUEST;
		dwDiamond = 0;
	}

};


///@brief  战斗验证物理攻击数据
struct PhysicAttackData
{
	int targetID;
	float damage;
	bool crit;
};

///@brief 战斗验证技能攻击数据
struct SkillAttackData
{
	int targetID;
	int skillID;
	float damage;
	bool crit;
};

///@brief 竞技场参战者战斗验证数据
struct ArenaFighterVerifyData
{
	int heroID;
	float maxHealth;
	int physicAttackDataNum;
	int skillAttackDataNum;
	PhysicAttackData physicAttackData[MAX_ARENACHALLENGE_HERONUM];
	SkillAttackData skillAttackData[MAX_ARENACHALLENGE_HERONUM];
	ArenaFighterVerifyData()
	{
		heroID = 0;
		maxHealth = 0.0f;
		physicAttackDataNum = 0;
		skillAttackDataNum = 0;
		memset(physicAttackData, 0, sizeof(PhysicAttackData) * MAX_ARENACHALLENGE_HERONUM);
		memset(skillAttackData, 0, sizeof(SkillAttackData) * MAX_ARENACHALLENGE_HERONUM);
	}
};

///@brief 竞技场战斗验证数据
struct ArenaBattleVerifyData
{
	int competitorArenaID;
	int heroID;
	int AttackTimes;
	int myFighterNum;
	int competitorFighterNum;
	ArenaFighterVerifyData myFighterVerifyDatas[MAX_ARENACHALLENGE_HERONUM];
	ArenaFighterVerifyData competitorFighterVerifyDatas[MAX_ARENACHALLENGE_HERONUM];
	ArenaBattleVerifyData()
	{
		competitorArenaID = 0;
		myFighterNum = 0;
		competitorFighterNum = 0;
		memset(myFighterVerifyDatas, 0, sizeof(ArenaFighterVerifyData) * MAX_ARENACHALLENGE_HERONUM);
		memset(competitorFighterVerifyDatas, 0, sizeof(ArenaFighterVerifyData) * MAX_ARENACHALLENGE_HERONUM);
	}
};


///领取首冲奖励
struct SQFirstRecharge : SSGPlayerMsg
{
	SQFirstRecharge()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_fIRSTRECHARGE;
	}
};
///回复领取首冲奖励
struct SAFirstRecharge : SSGPlayerMsg
{
	int res;// 1成功,2已领取,3未充值
	SAFirstRecharge()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_fIRSTRECHARGE;
	}
};
///@breif 基金回调信息
struct SAWeekRemoney : SSGPlayerMsg
{
	int state;//活动购买情况 0 未购买
	int day;
	int res[7];

	SAWeekRemoney()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_WeekReMoney;
		state = 0;
		memset(res, 0, sizeof(res));
	}
};
///请求领取基金s
struct SQWeekRemoney : SSGPlayerMsg
{
	int day;
};

///@国战
DECLARE_MSG_MAP(SWarMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_War)
	war_join,			//报名
	war_teamList,//取阵营参战人员列表
	war_fight_start,	//匹配开打
	war_fight_res,		//战斗结果
	war_fight_res_broadcast,		//战斗结果广播
	war_all_end,		//阵营获胜奖励结算
	war_exit,			//玩家退出
	war_isjoin,			//是否报名
	war_rank,			//排行榜
	war_end,			//国战结束
END_MSG_MAP()
//报名
struct SQWarJoin : SWarMsg
{
	DWORD dwHeroId[MAX_War_HERONUM];
	SQWarJoin()
	{
		SQWarJoin::_protocol = SWarMsg::war_join;
		memset(dwHeroId, 0, sizeof(DWORD)*MAX_War_HERONUM);
	}
};
//报名结果
struct SAWarJoin : SWarMsg
{
	DWORD res;//1成功  ,2为已经加入
	DWORD teamNum;//分配到哪个阵营(1,2)
	SAWarJoin()
	{
		SAWarJoin::_protocol = SWarMsg::war_join;
	}
};
//取阵营参战人员列表
struct SQwar_teamList : SWarMsg
{
	DWORD teamNum;//取哪队
	DWORD beginNum;//从这个位置往后取20个
	SQwar_teamList()
	{
		SQwar_teamList::_protocol = SWarMsg::war_teamList;
	}
};
//取阵营参战人员列表
struct SAwar_teamList : SWarMsg
{
	DWORD teamNum;//哪队
	DWORD beginNum;//从这个位置往后取20个
	char name_all[CONST_USERNAME*MAX_WAR_NAME_LIST];
	SAwar_teamList()
	{
		SAwar_teamList::_protocol = SWarMsg::war_teamList;
		memset(name_all, 0, CONST_USERNAME*MAX_WAR_NAME_LIST);
	}
};
//匹配开打
struct SAwar_fight_start : SWarMsg
{
	char name_other[CONST_USERNAME];//对手名字
	DWORD heroInfo_other[MAX_WAR_HERO_INFO*MAX_War_HERONUM];//16个基础属性数据(1-16)+8个技能数据(17-24)+2个血量怒气(25-26)+1武将id(27)
	DWORD heroInfo[MAX_War_HERONUM*3];//自己5个武将的血量,怒气,id
	DWORD score1;//1组积分
	DWORD score2;//2组积分
	DWORD winAll;//对手连胜
	SAwar_fight_start()
	{
		SAwar_fight_start::_protocol = SWarMsg::war_fight_start;
		memset(name_other, 0, CONST_USERNAME);
		memset(heroInfo_other, 0, sizeof(DWORD)*MAX_WAR_HERO_INFO*MAX_War_HERONUM);
		memset(heroInfo, 0, sizeof(DWORD) * 3 * MAX_War_HERONUM);
	}
};

//战斗结果
struct SQwar_fight_res : SWarMsg
{
	DWORD res;//1赢,2输  
	DWORD heroInfo[3 * MAX_War_HERONUM];//自己武将的id,血量,怒气
	SQwar_fight_res()
	{
		SQwar_fight_res::_protocol = SWarMsg::war_fight_res;
		memset(heroInfo, 0, sizeof(DWORD) * 3 * MAX_War_HERONUM);
	}
};
//战斗结果奖励反馈
struct SAwar_fight_res : SWarMsg
{

	DWORD res;//1赢,2输 100 结束了
	DWORD winInfo[6];//连胜信息 --胜利场数,失败场数,当前连胜场数,最大连胜场数,声望奖励,铜钱奖励
	SAwar_fight_res()
	{
		SAwar_fight_res::_protocol = SWarMsg::war_fight_res;
		memset(winInfo, 0, sizeof(DWORD)*4);
	}
};

//战斗结果广播
struct SAwar_fight_res_broadcast : SWarMsg
{
	char name_our[CONST_USERNAME];//己方阵营玩家名字
	char name_other[CONST_USERNAME];//对方阵营玩家名字
	DWORD res;//1赢,2输
	DWORD winNum;//连胜场数

	SAwar_fight_res_broadcast()
	{
		SAwar_fight_res_broadcast::_protocol = SWarMsg::war_fight_res_broadcast;
		memset(name_our, 0, CONST_USERNAME);
		memset(name_other, 0, CONST_USERNAME);
	}
};
//玩家退出
struct SQwar_exit : SWarMsg
{
	SQwar_exit()
	{
		SQwar_exit::_protocol = SWarMsg::war_exit;
	}
};
//玩家退出返回
struct SAwar_exit : SWarMsg
{
	SAwar_exit()
	{
		SAwar_exit::_protocol = SWarMsg::war_exit;
	}
};

//玩家请求是否报名
struct SQwar_isjoin : SWarMsg
{
	SQwar_isjoin()
	{
		SQwar_isjoin::_protocol = SWarMsg::war_isjoin;
	}
};
//玩家退出返回
struct SAwar_isjoin : SWarMsg
{
	DWORD isjoin;//1 已经参加;2 未参加
	DWORD team;//在哪组
	SAwar_isjoin()
	{
		SAwar_isjoin::_protocol = SWarMsg::war_isjoin;
	}
};
//国战结束
struct SAWarEnd : SWarMsg
{
	DWORD teamNum;//赢的阵营
	DWORD score1;//1组分数
	DWORD score2;//2组分数
	SAWarEnd()
	{
		SAWarEnd::_protocol = SWarMsg::war_end;
	}
};


//玩家请求排行榜
struct SQwar_rank : SWarMsg
{
	SQwar_rank()
	{
		SQwar_rank::_protocol = SWarMsg::war_rank;
	}
};

//排行榜更新
struct SAwar_rank : SWarMsg
{
	char name[CONST_USERNAME];
	DWORD win;//连胜数
	DWORD lv;
	DWORD icon;
	DWORD team;

	char name2[CONST_USERNAME];
	DWORD win2;//连胜数
	DWORD lv2;
	DWORD icon2;
	DWORD team2;
	SAwar_rank()
	{
		SAwar_rank::_protocol = SWarMsg::war_rank;
		memset(name, 0, CONST_USERNAME);
		memset(name2, 0, CONST_USERNAME);
	}
};

///@排行榜
DECLARE_MSG_MAP(SRankMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_Rank)
rank_get,			//取排行榜
rank_common,//普通排行版
rank_hero,	//名将排行榜
END_MSG_MAP()
//取排行榜
struct SQRank : SRankMsg
{
	DWORD itype;//排行版类型
	SQRank()
	{
		SQRank::_protocol = SRankMsg::rank_get;
	}
};
///回复取排行榜,普通
struct SARank_common : SRankMsg
{
	DWORD itype;//排行版类型
	RankList_SG list[MAX_Rank_List];
	SARank_common()
	{
		SARank_common::_protocol = SRankMsg::rank_common;
	}
};
///回复取排行榜,名将
struct SARank_hero : SRankMsg
{
	DWORD itype;//排行版类型
	RankList_hero_SG list[MAX_Rank_List];
	SARank_hero()
	{
		SARank_hero::_protocol = SRankMsg::rank_hero;
	}
};


///@兑换活动
DECLARE_MSG_MAP(SExcheangeMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_Excheange)
getExchange,			//取数据,领奖
exchangeBack,//领奖返回
exchangeData,	//返回全部数据
END_MSG_MAP()

///兑换活动 领取
struct SQGetExcheange : SExcheangeMsg
{
	DWORD itype; //1为取兑换数据 ,2兑换道具
	DWORD index;//第几个
	DWORD num;//兑换几次
	SQGetExcheange()
	{
		SQGetExcheange::_protocol = SExcheangeMsg::getExchange;
	}
};
///兑换活动 兑换后返回消息
struct SAexchangeBack : SExcheangeMsg
{
	DWORD res; //操作结果 1成功,2次数不足,3物品不足,4活动未开启
	DWORD index; //第几个子活动
	DWORD value; //操作后的值
	SAexchangeBack()
	{
		SAexchangeBack::_protocol = SExcheangeMsg::exchangeBack;
	}
};
///兑换活动 数据同步
struct SAexchangeData : SExcheangeMsg
{
	DWORD exData[MaxExchangeData];//玩家兑换活动数据
	SAexchangeData()
	{
		SAexchangeData::_protocol = SExcheangeMsg::exchangeData;
		memset(exData, 0, MaxExchangeData*sizeof(DWORD));
	}
};

