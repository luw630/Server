#pragma once
#include "../pub/ConstValue.h"
#include "HeroTypeDef.h"
#include <time.h>
#include "ConstValueSG.h"
//#include "PlayerTypedef.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////// 
///三国武将基本数据
///////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_HERO_NUM 100 ///<武将的最大数量
#define MAX_ITEM_NUM 400 ///< 物品最大数量
#define MAX_TOLLEGATE_NUM 160 ///<最大的关卡数
#define MAX_TREASUREFIND_TOLLGATE_NUM 6///<最大的秘境寻宝关卡数
#define MAX_LEAGUEOFLEGEND_TOLLGATE_NUM 18///<最大的将神传说关卡数
#define MAX_LEAGUEOFLEGEND_DUPILICATE_NUM 3///<将神传说的副本数
//#define MONEY_BLESS_INDEX 0 ///<金钱祈福下标
//#define DIAMOND_BLESS_INDEX 1 ///<元宝祈福下标
#define  SWEEPTICKET_ID 20010	///<扫荡券ID
#define  MALL_COMMODITY_NUM 8	///<商城商品数量
#define  MISSION_NUM 15 ///<当前的任务数量
#define  NEWBIEGUIDE_NUM 20	///<当前指引个数
#define  ACHIEVEMENT_NUM 168 ///<当前的成就的数量
#define  ACHIEVEMENT_GROUP_NUM 8///<当前成就类型的数量
#define  MAX_BOSS_DROPCOUNT 3///<boss 掉落的物品种类的最大数
#define	 MAX_ITEM_NUM_GAINED_FROM_CHEST 30 ///<开宝箱最多能获得多少物品
#define  MAX_ITEM_NUM_SYN_TO_CLIENT 128///<客户端更服务器一次性交互的物品的信息的数组的最大大小，物品类别个数的话为64，一半存ID，一半存个数
#define  MAX_TOLLGATE_HERO_NUM 10 ///<关卡中最多有多少个武将可以参战
#define  MAX_TOLLGATE_MONSTER_NUM 16///<普通副本、精英副本、将神传说、秘境寻宝中关卡的怪物的种类的最大数
#define SecsOfDay 86400 ///<定义一天中有多少秒
#define  EXTENDS_STATICDATAS_SIZE 5120 ///<静态的“扩展性”C++端数据，为以后的新功能所需数据占位用
#define  MAX_SYN_SCRIPT_LENTH 256///<同步给客户端的LUA数据
#define  MAX_SYN_NOTIFICATION_LENGTH 16 ///<同步给客户端的通知的数据的最大的长度为16个
#define	 MAX_THIEF_REWARD_NUM 10	///<小偷最大掉落信息数量
#define  MAX_THIEF_HERO_LIMIT 5		///<小偷挑战上阵武将限制
const int g_iExpeditionCharacterLimit = 5;///<远征最多的上场武将数量
const int g_iOneTypeEnmeyNumLimit = 3;///<远征同一属性的敌人的数量限制

///@brief struct 三国玩家的物品结构体，可以考虑压缩成char[]，把结构体大小控制在4 byte，物品ID 8 bits, 物品数量3bits 分隔符2bits
struct SSanguoItem
{
	DWORD m_dwItemID; ///< 物品ID
	DWORD m_dwCount; ///<物品数量
	SSanguoItem()
	{
		m_dwItemID = 0;
		m_dwCount = 0;
	}
};

///@brief struct 祈福数据
struct SBlessData
{
	//DWORD m_dwBlessCount; ///<祈福次数
	//DWORD m_dwBlessFreeCount; ///<免费祈福次数
	//DWORD m_dwLastBlessDate; ///<上次祈福日期
	//DWORD m_dwIsFirstBless; ///<第一次祈福标记
	DWORD m_dwDiamondblessCount; ///元宝抽奖次数
	DWORD m_dwMoneyBlessCount; ///铜钱抽奖次数
	DWORD m_dwDiamondFreeBlessCount = 1;  ///元宝祈福免费次数
	DWORD m_dwMoneyFreeBlessCount = 5; ///铜钱祈福免费次数
	DWORD m_dwLastDiamondFreeBlessDate; ///上一次免费元宝时间
	DWORD m_dwLastMoneyFreeBlessDate; ///上一次免费金钱时间
	DWORD m_dwIsFirstMoneyBless; /// 第一次铜钱祈福标记
	DWORD m_dwIsFirstDiamondBless; /// 第一次元宝祈福标记
	SBlessData()
	{
		m_dwDiamondblessCount = 0; ///元宝抽奖次数
		m_dwMoneyBlessCount = 0; ///铜钱抽奖次数
		m_dwDiamondFreeBlessCount = 1;  ///元宝祈福免费次数
		m_dwMoneyFreeBlessCount = 5; ///铜钱祈福免费次数
		m_dwLastDiamondFreeBlessDate = 0; ///上一次免费元宝时间
		m_dwLastMoneyFreeBlessDate = 0; ///上一次免费金钱时间
		m_dwIsFirstMoneyBless = 0; /// 第一次铜钱祈福标记
		m_dwIsFirstDiamondBless = 0; /// 第一次元宝祈福标记
	};
};

struct SSanguoTollgate
{
	DWORD m_dwStarLevel;
	DWORD m_dwChallengeTime;
	SSanguoTollgate()
	{
		m_dwStarLevel = 0;
		m_dwChallengeTime = 0;
	}
};

struct SSanguoStoryTollgate
{
	bool bFirstTime;
	SSanguoTollgate tollgateData;

	SSanguoStoryTollgate()
	{
		bFirstTime = true;
	}
};

struct SSanguoStoryEliteTollgate
{
	DWORD dwResetedTimes; ///已经重置了多少次
	SSanguoTollgate tollgateData;

	SSanguoStoryEliteTollgate()
	{
		dwResetedTimes = 0;
	}
};

/**@brief 用于商城的商品数据*/
struct SSanguoCommodity
{
	BYTE m_bSoldOut;
	DWORD m_dwID;
	DWORD m_dwCount;
	SSanguoCommodity()
	{
		m_dwID = 0;
		m_dwCount = 0;
		m_bSoldOut = FALSE;
	}

	void SetData(DWORD dwID, DWORD dwCount, BYTE bSoldOut)
	{
		m_dwID = dwID;
		m_dwCount = dwCount;
		m_bSoldOut = bSoldOut;
	}
};

/// <summary>
/// 副本类型
/// </summary>
enum InstanceType
{
	/// <summary>
	/// 剧情副本
	/// </summary>
	Story = 0,
	/// <summary>
	/// 剧情精英副本
	/// </summary>
	StoryElite,
	/// <summary>
	/// 远征副本
	/// </summary>
	Expedition,
	/// <summary>
	/// 竞技场
	/// </summary>
	Arena,
	/// <summary>
	/// 将神传说
	/// </summary>
	LeagueOfLegends,
	/// <summary>
	/// 秘境寻宝
	/// </summary>
	TreasureHunting,
	/// <summary>
	/// 军团战
	/// </summary>
	TeamSige,
	/// <summary>
	/// 追击盗贼
	/// </summary>
	ChaseThief,
	/// <summary>
	/// 国战
	/// </summary>
	WarOfLeague,
};

///@brief 三国货币类型
enum SSanguoCurrencyType
{
	Currency_Money = 1,		//<金币//
	Currency_Diamond = 2,	//<钻石//
	Currency_Honor = 3,		//<荣誉//
	Currency_Exploit = 4,	//<军功//
	Currency_Prestige = 5,	//<声望//
	Currency_SoulPoints = 6,	//<魂点
	Currency_Token = 7,		//<军需令//
};

enum CheckResult     //检查限制的结果//
{
	Failed = 0,                 //失败 不明原因//
	Pass,                   //通过//
	NoEnoughStamina,        //体力不足//
	NoEnoughMoney,          //金钱不足//
	NoEnoughDiamond,        //钻石不足//
	NoEnoughCD,             //CD不足//
	NoEnoughTimes,          //挑战次数不足//
	NoEnoughTeamLevel,      //战队等级不足//
};

//玩家点数类型
enum GoodsType
{
	money = 1, //铜钱
	diamond=2,  //钻石
	item=3, //道具
	hero=4, //英雄
	Exp=5,//经验
	honor=6,//荣誉
	exploit=7,//军功
	endurance=8,//体力
	level=9, ///君主等级
	vipExp=10,//vip经验
	Prestige=11,//声望，国战产出的声望
	blessPoints = 12,//祈福产出的祈福积分
	soulPoints = 13,//分解将魂产出的魂点
	token = 14, //军团产出的军令
};
enum GoodsWay //物品来源以及流向类型,日志使用,参照策划 "产出与消耗.xlsx"
{
	duplicate=1,	//副本
	checkIn =2,		//签到
	exploitMall=3,		//军功商城,远征商店
	honorMall=4,		//荣誉商城,竞技场商店
	findCowry = 5,		//秘境寻宝	
	passCustoms = 6,	//过关斩将	
	heroLegend= 7,		//将神传说	
	practiceForce= 8,		//练武场	(竞技场)
	recharge = 9,		//充值	
	task = 10,			//任务
	achievement = 11,	//	成就	
	active = 12,			//活动	
	mainCityActive = 13,	//主城活动	(追击盗贼)
	varietyMall = 14,	//杂货铺	
	forgeMall = 15,		//铁匠铺	
	bless = 16,			//祈福
	addskill = 17,		//加技能	
	buyskill = 18,		//购买技能点数	
	luckMan = 19,		//奇缘商人	
	gemMan = 20,		//珍宝商人	
	goldenTouch= 21,	//点金手	
	bugEndurance = 22,	//购买体力	
	cdtimeEndurance = 23,	//时间回复(体力)	
	itemUse = 24,		//道具使用或出售
	LegionMall = 25,		//军团商店,用声望买
	mail=26,//邮件获取
	lvup=27,//人物升级
	fanctionWar = 28,//军团战
	giftVipexp = 29,		//赠送vip经验值
	hungingHunting = 30, //武将挂机寻宝
	heroTraining = 31, //武将的训练
	chargeRebate = 32, //充值返利
	changename = 33, //改名
	warOfLeague = 34, ///国战
	soulExchange = 35,//将魂兑换
	monthlyMembership = 36, //月卡
	StaminaGift = 37, //好友赠送体力
	exchangeACT = 38, //兑换活动


};

///三国的一些小型数据的类型，用来从对应的lua数据池中获取该小型数据
enum SG_ExDataType
{
	MultiDiamondBlessCount = 1, //这个是元宝十连祈福次数
	BlessMaxThreeStarHeroCount, //控制十连抽必得三星武将的最大次数
	ChargeClaimedFlag, //首冲领取标识
	TrainedFlag, //首次训练是否完成的标识，为0表示没开始，为1标示已经开始训练，为2标示已经用钱立刻完成训练过一次
	NewbieGoldGainedFlag,///首次进主城是否掉落完金币
	SendStaminaCount, ///赠送体力次数
	GetStaminaCount, ///领取体力次数
	ExpeditionTimes, ///<过关斩将通关关卡次数
};

enum AchievementType
{
	Battle0 = 0,
	Battle1,
	Battle2,
	Battle3,
	Battle4,
	Advanced,
	Level,
	Collect,
};

/// <summary>
/// 随机生成的可达成的成就的类型
/// </summary>
enum RandomAchievementType
{
	/// <summary>
	/// 祈福
	/// </summary>
	Random_Blessing = 1,
	/// <summary>
	/// 锻造
	/// </summary>
	Random_Forging,
	/// <summary>
	/// 通关副本
	/// </summary>
	Random_Battle,
	/// <summary>
	/// 升级技能
	/// </summary>
	Random_SkillUpgrade,
	/// <summary>
	/// 进阶
	/// </summary>
	Random_Advanced,
	Random_Level,
	/// <summary>
	/// 收集武将
	/// </summary>
	Random_Collect,
};

enum AchievementRewardType
{
	ART_Hero = 0,
	ART_HeroDebris,
	ART_Diamond,
	ART_Equip,
	ART_Gold,
	ART_Property,
};

enum MissionRewardsType
{
	MR_DIAMOND, ///<奖励宝石，跟vip有挂钩
	MR_ENDURANCE, ///<奖励体力
	MR_SWEEPTICKET, ///<奖励扫荡令，其实跟MR_ITEM是一个概念，但是由于他的个数跟vip等级有挂钩关系，故在此独立出来
	MR_ITEM, ///<奖励物品
	MR_COMMON, ///<奖励一些通用的东西，比如说金钱、经验等
	MR_LUNCH, ///<午餐奖励
	MR_DINNER,///<晚餐奖励
	MR_SUPPER,///<夜宵奖励
};

///已有的任务类型
enum MissionType
{
	MT_MonthCard,	///<月卡任务
	MT_VIP,		///<扫荡令任务
	MT_Lunch,		///<午餐任务
	MT_Dinner,		///<晚餐任务
	MT_Supper,		///<夜宵任务
	MT_Duplicate,	///<副本任务
	MT_EliteDuplicate,///<精英故事副本任务
	MT_TreasureHunting,///<秘境寻宝副本任务
	MT_LeagueOfLegends,///<将神传说副本任务
	MT_ArenaMission,		///<练武场副本任务
	MT_SkillUpgrade,		///<技能升级任务
	MT_Forging,	///<锻造任务
	MT_Blessing,	///<祈福任务
	MT_Expedition,	///<过关斩将副本任务
};

enum HeroType
{
	Power = 0,
	Intelligence,
	Agility,
};

///@brief 小奖励的类型
enum DessertType
{
	Dessert_Gold = 1,
	DessertDiamond,
};

enum SSanguoItemType
{
	Item_None = -1,
	Item_ExpPotion,				//<经验药水//
	Item_EnchantingMaterial,	//<附魔材料//
	Item_SellingGoods,			//<贩卖品//
	Item_SweepTicket,			//<扫荡券//
	Item_TreasureChest,			///<宝箱
	Item_EndurancePotion,		///<体力药剂
	Item_SoulStone,				//<灵魂石//
	Item_Debris,				//<碎片//
};

///@brief 引导类型
enum GuideType
{
	Newbie,				///<新手流程//
	UpgradeRank,		///<武将进阶//
	SummonHero,			///<召唤英雄//
	LearnSkill,			///<技能加点//
	UnlockArena,		///<开放竞技场//
	CollectEquipment,	///<英雄收集六件装备//
	EliteDuplicate,		///<解锁精英副本//
	UnlockMall,				///<解锁商店//
	UnlockTreasureHunting,	///<解锁秘境寻宝//
	UnlockLegend,				///<解锁将神传说//
	UnlockForging,			///<解锁铁匠铺(装备精炼)//
	UnlockExpedition,			///<解锁过关斩将//
	AcquireGuanFeng,	///<领取关凤//
	AcquireXiaoQiao,	///<领取小乔//
	AcquireChengYu,		///<领取程昱//
	MaxCount = 20,			///<指引类型最大数量//
};

struct STimeData
{
	char m_LoginTime[8]; //上次上线时间
	char m_LogoutTime[8]; //上次下线时间
	char m_OnlineTime[8]; //在线时间
};
struct SExpeditionData
{
	/// <summary>
	/// 成功的征服了所有的远征副本
	/// </summary>
	bool bSuccessToConqureWholeExpedtion;
	/// <summary>
	/// 抽奖抽到英雄
	/// </summary>
	bool bRewardedHero;
	/// <summary>
	/// 是否有奖励没领取
	/// </summary>
	bool bRewardsUnclaimed;
	bool expeditionActived;
	/// <summary>
	/// 当前远征副本的ID
	/// </summary>
	DWORD curExpeditionInstanceID;
	/// <summary>
	/// 当前远征副本的挑战次数
	/// </summary>
	DWORD curChallengeTimes;
	/// <summary>
	/// 当前收获的物品
	/// </summary>
	DWORD curItemObtained;
	/// <summary>
	/// 当前的关卡难度系数
	/// </summary>
	float levelFactor;
	/// <summary>
	/// 当前敌对的武将信息
	/// </summary>
	SHeroData curHostileCharacterInfor[g_iExpeditionCharacterLimit];

	SExpeditionData()
	{
		bSuccessToConqureWholeExpedtion = false;
		bRewardedHero = false;
		bRewardsUnclaimed = false;
		expeditionActived = false;
		curExpeditionInstanceID = 0;
		curChallengeTimes = 0;
		curItemObtained = 0;
		levelFactor = 1;
	}
};

struct SSanguoMallData
{
	/// 商品数据
	SSanguoCommodity m_arrCommodity[MALL_COMMODITY_NUM];
	/// 下次刷新时间点
	DWORD m_dwNextRefreshTime;
	/// 当日手动刷新次数
	DWORD m_dwRefreshedCount;
	/// 上次刷新时间
	char m_LastRefreshTime[8];
	/// 距离下次刷新剩余时间
	DWORD m_dwRefreshRemainingSeconds;
	
	SSanguoMallData()
	{
		memset(m_arrCommodity, 0, sizeof(SSanguoCommodity) * MALL_COMMODITY_NUM);
		m_dwNextRefreshTime = 0;
		m_dwRefreshedCount = 0;
		memset(m_LastRefreshTime, 0, sizeof(char) * 8);
		m_dwRefreshRemainingSeconds = 0;
	}
};

struct SSanguoSpecialMallData
{
	SSanguoMallData mallNormalData;
	/// 激活时间
	char activateTime[8];
	/// 激活后已流逝时间
	DWORD dwElapsedTime;
	/// 是否临时开启
	bool bTemporaryOpened;
	/// 是否一直开放
	bool bAlwaysOpened;

	SSanguoSpecialMallData()
	{
		//memset(&mallNormalData, 0, sizeof(SSanguoMallData));
		memset(activateTime, 0, sizeof(char) * 8);
		dwElapsedTime = 0;
		bTemporaryOpened = false;
		bAlwaysOpened = false;
	}
};

///@brief 单个终生成就的完成等数据
struct SAchivementUnitData
{
	bool accompulished;///<是否完成
	DWORD achievementID;///<成就ID
	DWORD completedTimes;///<完成次数
	DWORD groupType;///<成就属于的类别
};

///@brief 某一类别成就的数据
struct SAchivementGroupData
{
	bool accompulished;///<该组成就是否已经完成
	int GroupType;///<该组的类型
	DWORD newestAchivementID;///<该组成就最新的待完成的成就ID
};

struct SAchievementData
{
	SAchivementUnitData achievementsData[ACHIEVEMENT_NUM];
	SAchivementGroupData groupData[ACHIEVEMENT_GROUP_NUM];
};

struct SArenaData
{
	char m_LastChallengeTime[8];
	DWORD m_BestRank;
	DWORD defensiveTeam[5];
	DWORD attackingTeam[5];
	SArenaData()
	{
		memset(&defensiveTeam, 0, 5 * sizeof(DWORD));
		memset(&attackingTeam, 0, 5 * sizeof(DWORD));
	}
};


struct SNewbieGuideDInfo
{
	bool completed;
	BYTE stage;
	SNewbieGuideDInfo()
	{
		completed = false;
		stage = 0;
	}
};

struct SNewbieGudieData
{
	SNewbieGuideDInfo datas[NEWBIEGUIDE_NUM];
	DWORD curGuide;//新手引导 千位为引导类型,以下为哪一步
	SNewbieGudieData()
	{
		memset(datas, 0, sizeof(SNewbieGuideDInfo) * NEWBIEGUIDE_NUM);
		curGuide = 0;
	}
};

struct SSkillPointData
{
	DWORD m_dwSkillPoint; ///<技能点
	DWORD m_dwBuySkillPointTime;	///<购买技能点次数
	DWORD m_dwRemainingSeconds;	///<计算技能点时的剩余CD
	char m_LastCalculateSkillPointTime[8];	///<上次计算技能点的时间
	bool learnedSkill;	///<学习过技能的标志(用于引导学习技能的判断)
};

struct SEnduranceData
{
	DWORD m_dwEndurance;	///<玩家在线时更新或玩家离线时保存的体力值
	DWORD m_dwRemainingSeconds;	///<玩家离线所剩余的回复秒数
	char m_latestCheckTime[8];	///<最新计算体力的时间
};

struct SChaseThiefData
{
	char  activateTime[8];	///< 追击盗贼激活时间 
	DWORD remainningTime;	///< 剩余有效时间
	bool valid;	///< 是否有效
};

///@brief struct 玩家数据
struct SSanGuoPlayerFixData
{
	SHeroData m_HeroList[MAX_HERO_NUM]; ///<武将列表
	SSanguoItem m_ItemList[MAX_ITEM_NUM]; ///<物品列表
	SBlessData m_BlessData; ///<金钱祈福和元宝祈福数据
	SExpeditionData m_ExpeditionData; ///<远征的数据
	SAchievementData m_AchievementData;///<终生成就数据
	SAchivementUnitData m_RandomAchievementData; ///<随机生成的可达成的成就的数据
	SSanguoStoryTollgate m_TollgateData[MAX_TOLLEGATE_NUM]; ///<普通关卡
	SSanguoStoryEliteTollgate m_EliteTollgateData[MAX_TOLLEGATE_NUM]; ///<精英关卡
	SSanguoTollgate m_TreasureTollgateData[MAX_TREASUREFIND_TOLLGATE_NUM];///<秘境寻宝关卡
	SSanguoTollgate m_LeagueOfLegendTollgateData[MAX_LEAGUEOFLEGEND_TOLLGATE_NUM];///<将神传说关卡
	DWORD m_TreasureHuntingChallengeTimes;
	DWORD m_LeagueOfLegendChallengeTimes[MAX_LEAGUEOFLEGEND_DUPILICATE_NUM];
	STimeData m_TimeData;
	SSanguoMallData m_VarietyShopData;		///<杂货铺数据
	SSanguoMallData m_ArenaShopData;		///<竞技场商店数据
	SSanguoMallData m_ExpeditionShopData;	///<远征商店数据
	SSanguoSpecialMallData m_MiracleMerchantData;	///<奇缘商店数据
	SSanguoSpecialMallData m_GemMerchantData;		///<珍宝商店数据
	SSanguoMallData m_LegionShopData;		///<军团商店数据
	SSanguoMallData m_WarOfLeagueShopData; ///<国战商店数据
	SSanguoMallData m_SoulExchangeData;		///<将魂商店
	SSanguoMallData m_BlessPointsShop;		///<祈福积分商城
	SNewbieGudieData m_NewbieGuideData;	///< 引导数据
	SSkillPointData m_SkillPointData;	///< 技能点数据
	SEnduranceData m_EnduranceData;		///< 体力数据
	SChaseThiefData m_ChaseThiefData;	///< 追击盗贼数据
	DWORD m_ArenaDefenseTeam[5]; ///<竞技场防守阵容
	DWORD m_RandomAchievementRemainingTime;//随机成就结束的剩余时间（秒）
	//SPlayerRelation		m_sPlayerRelation;	//玩家好友数据
	DWORD		m_dwExp;	///< 当前经验
	DWORD		m_version; ///<版本号
	DWORD		m_dwMoney;	///< 金币
	DWORD		m_dwMaxRecharge; ///<总充值数额
	DWORD		m_dwDiamond; ///<钻石
	DWORD		m_dwLevel; ///<等级
	//======================== 下列属性今后要分离开这本结构体中
	DWORD	m_dwExploit; ///<军功,过关斩将获得
	DWORD	m_dwHonor;		///<荣誉，练武场获得
	DWORD	m_dwPrestige;	///<声望，国战获得
	DWORD	m_dwSoulPoints;  ///<魂点，将魂碎片分解获得
	DWORD	m_dwToken;		///<军令，军团获得
	DWORD	m_dwBlessPoints;///<祈福产出的积分
	DWORD m_dwLatestBattleFileID; ///<最新解锁的普通关卡ID(Int)
	DWORD m_dwLatestEliteBattelFileID; ///<最新解锁的精英关卡ID(Int)
	DWORD m_dwSiginCount; ///<累积签到次数(Int)
	DWORD m_dwTodaySiginFlag; ///<每日签到状态(BOOL)
	DWORD m_dwGoldExhcangedCount; ///<宝石兑换金钱后的次数
	DWORD m_dwEnduranceExchangeCount;///<宝石兑换体力后的次数
	DWORD m_dwRandomAchieveAccomplishedTimes;///<随机可达成任务已经刷新的次数
	DWORD m_dwIsFirstGoldBless; ///<第一次金钱祈福标记
	DWORD m_dwIsFirstDiamondBless; ///<第一次元宝祈福标记
	DWORD m_dwIsFirstEnterArena; ///<第一次竞技场标记
	DWORD m_dwStoryPlayed; ///
	DWORD m_dwEnterZoneServerTime; ///zone server的当前时间
	DWORD m_dwZoneID; ///创建账号时的zone id
	DWORD m_PlayerIconID; //玩家头像
	BYTE m_extendsBuffer[EXTENDS_STATICDATAS_SIZE]; ///扩展性的C++端静态数据，占位用
	BYTE m_bNewPlayer; ///<新玩家 
	BYTE m_bLuckyTimeFall; ///<是否处于黑夜时间（幸运时间）
	char m_Name[CONST_USERNAME]; /// 玩家名字
	//DWORD m_dFactionID[8];//玩家工会ID
	char m_FactionName[CONST_USERNAME];//公会id,只用了前4位
	BYTE	m_bluaBuffer[LUABUFFERSIZE];	// 用于在lua 中数据的存储


	SSanGuoPlayerFixData()
	{
		//memset(m_FactionName, 0, sizeof(m_FactionName));
		m_PlayerIconID = 108;
	}
};



///@brief struct 固定的玩家属性，会保存下来的数据	
struct SFixData : SSanGuoPlayerFixData
{ 
	enum SAVESTATE		///< 角色服务器上的状态
	{
		ST_LOGIN = 1,	// 刚刚登入
		ST_LOGOUT,		// 退出
		ST_HANGUP,		// 离线挂机
	};

	DWORD   m_dwStaticID;					///< 玩家对服务器集群唯一静态的ID，由数据库服务器生成
	char    m_UPassword[CONST_USERPASS];///< 玩家二级密码
	BYTE    m_byStoreFlag;					///< 玩家的当前状态，用于区分保存时的信息
	static const int VERSIONID = 5;///<
	//子类结构
	static DWORD GetVersion() { return (sizeof(SFixData) << 16 | VERSIONID); };
};
enum Expense
{
	EX_Zhan0,			//占0索引
	Ex_Strength,		//购买体力消耗宝石数		

	Ex_Skill,			//购买技能点数消耗		
	Ex_ExpertPass,		//重置精英关卡消耗宝石		
	Ex_ArenaTicket,		//购买练武场门票消耗元宝		
	Ex_ReSetCD,		//重置练武场战斗CD消耗宝石
};

enum VipLevelFactor
{
	VF_Zhan0,				//占0索引
	VF_GiveATKTicket_Num,		//赠送扫荡券		
	VF_BuyStrength_Num,			//购买体力次数		
	VF_OneToK_Num,			//一掷千金次数		
	VF_ButSkill_Num,		//购买技能点次数		
	VF_ReSetPass_Num,			//重置精英关卡
	VF_BuyArenaTicket_Num,				//购买练武场门票次数
	VF_Skill_Num,			//技能点数
	VF_ATKTen_Num,			//扫荡十次
	VF_ReSetArenaCD_Num,			//重置练武场战斗CD
	VF_OneKey_Num,			//一键精炼
	VF_ArtfulMan_Num,			//奇缘商人
	VF_KillPass_Num,			//过关斩将次数
	VF_GemMan_Num,			//珍宝商人
	VF_KillPassAdd_Num,			//过关斩将奖励增加
};


