#pragma once
#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include "stdint.h"
#include "..\PUB\ConstValue.h"
#include "..\NETWORKMODULE\HeroTypeDef.h"
using namespace std;

enum MissionType;

enum HeroTrainingType
{
	/// <summary>常规训练</summary>
	Normal = 1,
	/// <summary>快速训练</summary>
	Fast,
	/// <summary>极端训练</summary>
	Extreme,
};

struct GoodsInfoSG
{
	int itype; //类型 GoodsType 参照函数  AddGoods_SG(const int32_t iType, const int32_t id=0, int32_t num = 0);
	int id; //类型为铜钱钻石等时此参数无用,只用数量就可以了
	int num;
};

///@ 全局配置
struct CGlobalConfig
{
	int32_t PhysicalLimit;///<君主体力上限
	int32_t PhysicalRecoverTime;///每点体力恢复时间(秒)	    
	int32_t BuyPhysical;///每次购买获得的体力值	    
	int32_t SkillCostMoney;///每次技能升级消耗金钱增量   
	int32_t LotteryOnceMoney;///单次铜钱抽奖价格	        
	int32_t LotteryTenTimesMoney;///十次铜钱抽奖价格	        
	int32_t LotteryOnceGold;///单次元宝抽奖价格	
	int32_t LotteryTenTimesGold;///十次元宝抽奖价格	
	int32_t NotVipBuyPhysicalTimes;///非VIP购买体力次数
	int32_t BlessFreeTime;///元宝祈福免费间隔时间(秒)	
	int32_t ChangeNameConsumption;//改名消耗的钻石
	int32_t ChangeNameConsumptionFreeTime;//免费改名的次数
	float	GetHeroForCashParam1;///元宝招将参数1	
	float	GetHeroForCashParam2;///元宝招将参数2
	int32_t LearnSkillLevel;	///触发学习技能引导等级
	int32_t SkillPointRecoverTime;
	int32_t SkillPointLimit;
	int32_t BattleLimitedTime;
	int32_t CrusadeHeroLevelLimit;
	int32_t CrusadeInitMana;
	int32_t BlessFreeTimeForMoney;
	int32_t FristBlessHero;
	int32_t SecondBlessHero;
	int32_t DiamondSweepCost;
	float	HeroPiecesPriceRatio;
	float	GreenEquipForRestore; ///<绿色装备在升阶过程中锻造经验值的返还比例
	float	BluenEquipForRestore;	///<蓝色装备在升阶过程中锻造经验值的返还比例
	float	PurpleEquipForRestore;///<紫色装备在升阶过程中锻造经验值的返还比例
	///<奇缘商人开放几率
	float	miracleMerchantOpenProbability;
	///< 奇缘商人开启条件VIP等级限制
	int32_t MiracleMerchantVIPLevelLimit;
	///< 奇缘商人开启条件君主等级限制
	int32_t MiracleMerchantMasterLevelLimit;
	///< 珍宝商人开放几率
	float	gemMerchantOpenProbability;
	///< 珍宝商人开启条件VIP等级限制
	int32_t GemMerchantVIPLevelLimit;
	///< 珍宝商人开启条件君主等级限制
	int32_t GemMerchantMasterLevelLimit;
	int32_t ForgeExpRestoreItem;	///<装备在升阶过程中锻造经验值会返还的物品ID
	int32_t ForgeMoneyCostScale;	///<锻造物品的时候的金钱消耗的比例值
	int32_t FirstTimeGetHeroForMoney;
	int32_t FirstTimeGetHeroForCash;
	int32_t OneStarHeroTransform;
	int32_t TwoStarHeroTransform;
	int32_t ThreeStarHeroTransform;
	int32_t BuySkillPointCost;
	int32_t LuckyTimeGap;///<幸运时间段（策划所说的黑夜时间）的间隔时间，间隔多久才生效
	int32_t LuckyTimePersistentTime;///<幸运时间段的持续时间
	int32_t MaxRandomAchievementAccomplishTimes;///<随机生成的可达成的成就的最大的可挑战的次数
	int32_t ChaseThiefDurationTime;	///<追击小偷的持续时间
	int32_t ChaseThiefOpenLevel;///<追击盗贼开放等级
	float ChaseThiefActivateRate;///<追击盗贼激活几率

	int	PlayerDonateMaxNum;//军团每日捐献上限
	int	GemAndPrestigeRatio;//军团宝石和声望转化值
	int	MoenyAndPrestigeRatio;//军团金币和声望转化值
	int	CreateFactionNeedMoney;//军团创建所需金币
	int	FactionDayGetMaxExp;//军团每日经验上限 

	int32_t LogMoneyNum;//日志 铜钱起步
	int32_t LogItemNum;//日志 道具数量起步
	int32_t LogItemIdBengin1;//日志 道具id起始值1
	int32_t LogItemIdEnd1;//日志 道具id结束值1
	int32_t LogItemIdBengin2;//日志 道具id起始值2
	int32_t LogItemIdEnd2;//日志 道具id结束值2
	int32_t LogItemIdBengin3;//日志 道具id起始值3
	int32_t LogItemIdEnd3;//日志 道具id结束值3
	float MovingTime;			//验证移动时间
	float FightingCapacityFaultTolerant;	//验证战力容错率
	float AttackingFaultTolerant;	//验证攻击容错率
	float GethitFaultTolerant;		//验证手机容错率

	int HeroTrainingCountLimit;//武将训练人数上限
	int HeroTrainingNeedTime;//武将训练所需时间(秒)
	int HeroTrainingUnitTime;//武将训练经验值计算的单位时间
	int HeroHuntingCountLimit;//武将寻宝人数上限
	int HeroHuntingLevelLimit;//武将寻宝等级限制（最低等级限制）
	int HeroHuntingTime;//武将寻宝时间
	int HeroHuntingGoldClearingUnitTime;//武将寻宝金币收益计算时间单位
	int HeroHuntingSurpriseUnitTime;//武将寻宝掉宝间隔时间单位

	///<杂货铺刷新消耗
	vector<int32_t> vecVarietyShopRefreshCost;
	///<将魂兑换商店刷新消耗
	vector<int32_t> vecSoulExchangeRefreshCost;
	///<竞技场商店刷新消耗
	vector<int32_t> vecArenaShopRefreshCost;
	///<远征商店刷新消耗
	vector<int32_t> vecExpeditionShopRefreshCost;
	///<奇缘商店刷新消耗
	vector<int32_t> vecMiracleMerchantRefreshCost;
	///<珍宝商店刷新消耗
	vector<int32_t> vecGemMerchantRefreshCost;
	///<军团商店刷新消耗
	vector<int32_t> vecLegionShopRefreshCost;
	///<国战商店刷新消耗
	vector<int32_t> vecWarOfLeagueShopRefreshCost;
	///<杂货铺物品刷新时间列表
	vector<int32_t> vecVarietyShopRefreshClock;
	///<将魂兑换商店物品刷新时间列表
	vector<int32_t> vecSoulExchangeRefreshClock;
	///<竞技场商店物品刷新时间列表
	vector<int32_t> vecArenaShopRefreshClock;
	///<远征商店物品刷新时间列表
	vector<int32_t> vecExpeditionShopRefreshClock;
	///<奇缘商店物品刷新时间列表
	vector<int32_t> vecMiracleMerchantRefreshClock;
	///<珍宝商人物品刷新时间列表
	vector<int32_t> vecGemMerchantRefreshClock;
	///<军团商店物品刷新时间列表
	vector<int32_t> vecLegionShopRefreshClock;
	///<国战商店物品刷新时间列表
	vector<int32_t> vecWarOfLeagueShopRefreshClock;
	///<招募武将消耗
	vector<int32_t> vecHeroSummonCost;
	///<武将升星消耗
	vector<int32_t> vecHeroEvolutionCost;
	//std::string GMHeroID;
};

///@brief 游戏玩法功能开放配置
struct GameFeatureActivationConfig
{
	//int skillUpgradeLevelLimit;///<技能升级的开放等级
	int shoppingLevelLimit;///<购买物品的开放等级
	int eliteDungeonLevelLimit;///<精英副本的开放等级
	int midasLevelLimit; ///<点金手等级开放限制
	int treasureHuntingDungeonLevelLimit;///<秘境寻宝副本的开放等级
	int arenaLevelLimit;///<竞技场的开放等级
	int leagueOfLegendPalaceLevelLimit;///<将神传说的开放等级
	int expeditionDungeonLevelLimit;///<远征的开放等级
	int forgeShopLevelLimit;///<铁匠铺的开放等级
	int teamSiegeLevelLimit;///<军团的开放等级
	int reinforcementsLevelLimit;///<援军的开放等级
	int mopUpTollgateLevelLimit;///<关卡扫荡的开放等级
	int MoneyHand;///<点金手
	int Relationship;///< 缘分系统开放等级
	int HeroTrainingLimit;///<英雄武将训练
	int HeroHungingHuntingLimit;///<英雄挂机寻宝
	int soulExchangeLimit;///<将魂兑换
};

///@brief 元宝兑换金钱的配置信息
struct ExchangeGoldConfig{
	/// <summary>
	/// 次数
	/// </summary>
	int m_count;
	/// <summary>
	/// 兑换金币
	/// </summary>
	int m_getGoldCount;
	/// <summary>
	/// 宝石消耗
	/// </summary>
	int m_useDiamondCount;
	/// <summary>
	/// 金币增量随机min
	/// </summary>
	int m_goldRandomMin;
	/// <summary>
	/// 金币增量随机max
	/// </summary>
	int m_goldRandomMax;
	/// <summary>
	/// 暴击min
	/// </summary>
	int m_critMin;
	/// <summary>
	/// 暴击max
	/// </summary>
	int m_critMax;
	/// <summary>
	/// 暴击几率
	/// </summary>
	int m_critOdds;
};

///@brief 小奖励的配置文件
struct DessertConfig
{
	int DessertType;
	int DessertProbability;
	vector<int> DessertNums;///<每一种类型的小奖励所对应的会奖励的个数
	vector<int> DessertWeightPerUnit;///<每一种类型的小奖励的所对应的会奖励的个数的几率

	DessertConfig()
	{
		DessertType = 0;
		DessertProbability = 0;
		DessertNums.clear();
		DessertWeightPerUnit.clear();
	}

	DessertConfig(DessertConfig&& other)
		:DessertNums(std::move(other.DessertNums)),
		DessertWeightPerUnit(std::move(other.DessertWeightPerUnit))
	{
		DessertType = other.DessertType;
		DessertProbability = other.DessertProbability;
	}

	DessertConfig& operator = (DessertConfig&& other)
	{
		if (this != &other)
		{
			DessertType = other.DessertType;
			DessertProbability = other.DessertProbability;
			DessertNums = std::move(other.DessertNums);
			DessertWeightPerUnit = std::move(other.DessertWeightPerUnit);
		}

		return *this;
	}
};

///@brief 道具配置结构//
struct ItemConfig
{
	///< ID //
	int32_t ItemID;
	///< 效果值 //
	int32_t Ability;
	///< 合成物品名 //
	int32_t ComposeItemID;
	///< 合成数量//
	int32_t ComposeNum;
	///< 精炼经验值 //
	int32_t RefiningExp;
	///< 金币购买价格 //
	int32_t BuyGoldPrice;
	///< 钻石购买价格 //
	int32_t BuyDiamondPrice;
	//分解得到的魂点数量
	int32_t ResolveSoulPoints;
	//购买本道具得花费的魂点数量
	int32_t BuySoulPointsCost;
	///< 荣誉购买价格 //
	int32_t BuyHonorPrice;
	///< 军功购买价格 //
	int32_t BuyExploitPrice;
	///< 声望购买价格 //
	int32_t BuyPrestigePrice;
	///< 军需令购买价格
	int32_t BuyTokenPrice;
	///< 出售价格(出售只换取金币) //
	int32_t SellPrice;
	///< 类型(消耗品、将魂石...) //
	BYTE Type;
	///< 品质 (区分白装 绿装 蓝装 紫装)
	BYTE Quality;
};

struct MissionConfig
{
	int id; //任务ID
	MissionType missionType;//任务的类型
	int missionAvaliableLevel;//任务的开放的你急
	int rewardsExp; //奖励经验
	int rewardsGold; //奖励金币
	int rewardsItemID; //奖励物品ID
	int rewardsItemAmount; //奖励物品数量
	int needCompleteTime; //需要完成次数
	int rewardsType;//奖励的类型
};

struct AchievementConfig
{
	int id; //成就ID
	int achieveType;//成就的类型
	int param1; //达成成就所需条件1
	int param2; //达成成就所需条件2
	int rewardsType;//成就奖励的类型
	int rewardID; //奖励物品ID
	int rewardAmount; //奖励物品数量
};

struct RandomAchievementConfig : public AchievementConfig
{
	int accessibleLevel;///<随机生成的可达成的成就的开放等级
};

///普通副本、精英副本、将神传说副本、秘境寻宝副本的章节配置
struct ChapterConfig
{
	int chapterID;        ///<章节ID
	int chapterType;		///<副本类型
	int ticketNum;			///<副本可挑战次数，为-1则不限制挑战次数
	set<int> battleIdList;  ///该章节所包含的关卡ID列表

	ChapterConfig()
	{
		chapterID = 0;
		chapterType = 0;
		ticketNum = 0;
		battleIdList.clear();
	}

	ChapterConfig(ChapterConfig&& other)
	{
		chapterID = other.chapterID;
		chapterType = other.chapterType;
		ticketNum = other.ticketNum;
		battleIdList = std::move(other.battleIdList);
	}

	ChapterConfig& operator = (ChapterConfig&& other)
	{
		if (this != &other)
		{
			chapterID = other.chapterID;
			chapterType = other.chapterType;
			ticketNum = other.ticketNum;
			battleIdList = std::move(other.battleIdList);
		}

		return *this;
	}
};

struct HeroTrainingSpeedUpInfor
{
	int consumeType;///英雄训练消耗物品的类型
	HeroTrainingType trainingType;
	float bonusScale;///英雄训练的训练模式对应的经验值加成
	int payment; ///训练英雄消耗的铜币或者元宝数量

	HeroTrainingSpeedUpInfor()
	{
		bonusScale = 0;
		trainingType = HeroTrainingType::Normal;
		consumeType = 1;
		payment = 0;
	}
};

///英雄训练的配置文件
struct HeroTrainingCfg
{
	int masterLevel;
	float expProceeds;///每一个时间单位会固定收获的金钱
	float costOfSpeedUp;
	unordered_map<HeroTrainingType, HeroTrainingSpeedUpInfor> gainsAndCost;

	HeroTrainingCfg()
	{
		masterLevel = 0;
		expProceeds = 0;
		costOfSpeedUp = 0;
		gainsAndCost.clear();
	}
};

///英雄挂机寻宝的配置信息
struct HeroHungingHuntingCfg
{
	int masterLevel;
	float fixedProceeds;	///每一个时间单位会固定收获的金钱
	float fLuckyOdds;		///会掉落额外物品的几率
	int goldProceeds;		///额外掉落的金钱的数量
	float goldWeight;		///额外掉落会掉金钱的权重
	int diamondProceeds;	///额外掉落的元宝的数量
	float diamondWeight;	
	float propsWeight;
	float equipWeight;
	float debrisWeight;
	float totalWeight;
	vector<int> extraPropsProceeds;///额外会掉落的道具物品
	vector<int> extraEqupProceeds;///额外会掉落的装备
	vector<int> extraDebrisProceeds;///额外会掉落的碎片

	HeroHungingHuntingCfg()
	{
		masterLevel = 0;
		fixedProceeds = 0;
		fLuckyOdds = 0;
		goldProceeds = 0;
		goldWeight = 0;
		diamondProceeds = 0;
		diamondWeight = 0;
		propsWeight = 0;
		equipWeight = 0;
		debrisWeight = 0;
		extraPropsProceeds.clear();
		extraEqupProceeds.clear();
		extraDebrisProceeds.clear();
	}
};

///普通副本、精英副本、将神传说副本、秘境寻宝副本的关卡配置
struct BattleLevelConfig
{
	/// <summary>
	/// 编号
	/// </summary>
	int ID;
	/// <summary>
	/// 限制等级
	/// </summary>
	int LevelLimit;
	/// <summary>
	/// 关卡英雄经验
	/// </summary>
	int Exp;
	/// <summary>
	/// 体力消耗
	/// </summary>
	int Cost;
	/// <summary>
	/// 挑战次数
	/// </summary>
	int TicketNum;
	/// <summary>
	/// 关卡中的怪物
	/// </summary>
	unordered_set < int > LevelMonsters;

	BattleLevelConfig()
	{
		ID = 0;
		LevelLimit = 0;
		Exp = 0;
		Cost = 0;
		TicketNum = 0;
		LevelMonsters.clear();
	}

	BattleLevelConfig(BattleLevelConfig&& other)
		:LevelMonsters(std::move(other.LevelMonsters))
	{
		ID = other.ID;
		LevelLimit = other.LevelLimit;
		Exp = other.Exp;
		Cost = other.Cost;
		TicketNum = other.TicketNum;
	}

	BattleLevelConfig& operator = (BattleLevelConfig&& other)
	{
		ID = other.ID;
		LevelLimit = other.LevelLimit;
		Exp = other.Exp;
		Cost = other.Cost;
		TicketNum = other.TicketNum;
		LevelMonsters = std::move(other.LevelMonsters);
	}
};

///普通副本、精英副本、将神传说副本、秘境寻宝副本的关卡第一次掉落配置
struct BattleFirstDropConfig
{
	int ID;
	int Money;
	int BossItem1;
	int DropNum1;
	int BossItem2;
	int DropNum2;
};

///普通副本、精英副本、将神传说副本、秘境寻宝副本的关卡掉落配置
struct BattleDropConfig
{
	int ID;              ///<关卡ID
	int Money;           ///<关卡金钱
	int MinNumber;       ///<最小掉落数
	int MaxNumber;       ///<最大掉落数
	int WhiteItemWeight;     ///<白色物品掉落权重
	int GreenItemWeight;     ///<绿色物品掉落权重
	int BlueItemWeight;      ///<蓝色物品掉落权重
	int PurpleItemWeight;    ///<紫色物品掉落权重
	int BossItem1;
	int BossItemNumber1;
	int BossItemDropPercent1;
	int BossItem2;
	int BossItemNumber2;
	int BossItemDropPercent2;
	int BossItem3;
	int BossItemNumber3;
	int BossItemDropPercent3;
	vector<int> WhiteItemList;
	vector<int> GreenItemList;
	vector<int> BlueItemList;
	vector<int> PurpleItemList;
	vector<int> PropID; ///<扫荡掉落道具ID
	vector<int> PropNumber; ///<扫荡掉落道具数量

	BattleDropConfig()
	{
		ID = 0;
		Money = 0;
		MinNumber = 0;
		MaxNumber = 0;
		WhiteItemWeight = 0;
		GreenItemWeight = 0;
		BlueItemWeight = 0;
		PurpleItemWeight = 0;
		BossItem1 = 0;
		BossItemNumber1 = 0;
		BossItemDropPercent1 = 0;
		BossItem2 = 0;
		BossItemNumber2 = 0;
		BossItemDropPercent2 = 0;
		BossItem3 = 0;
		BossItemNumber3 = 0;
		BossItemDropPercent3 = 0;
		WhiteItemList.clear();
		GreenItemList.clear();
		BlueItemList.clear();
		PurpleItemList.clear();
		PropID.clear();
		PropNumber.clear();
	}

	BattleDropConfig(BattleDropConfig&& other)
	{
		ID = other.ID;
		Money = other.Money;
		MinNumber = other.MinNumber;
		MaxNumber = other.MaxNumber;
		WhiteItemWeight = other.WhiteItemWeight;
		GreenItemWeight = other.GreenItemWeight;
		BlueItemWeight = other.BlueItemWeight;
		PurpleItemWeight = other.PurpleItemWeight;
		BossItem1 = other.BossItem1;
		BossItemNumber1 = other.BossItemNumber1;
		BossItemDropPercent1 = other.BossItemDropPercent1;
		BossItem2 = other.BossItem2;
		BossItemNumber2 = other.BossItemNumber2;
		BossItemDropPercent2 = other.BossItemDropPercent2;
		BossItem3 = other.BossItem3;
		BossItemNumber3 = other.BossItemNumber3;
		BossItemDropPercent3 = other.BossItemDropPercent3;
		WhiteItemList = std::move(other.WhiteItemList);
		GreenItemList = std::move(other.GreenItemList);
		BlueItemList = std::move(other.BlueItemList);
		PurpleItemList = std::move(other.PurpleItemList);
		PropID = std::move(other.PropID);
		PropNumber = std::move(other.PropNumber);
	}

	BattleDropConfig& operator = (BattleDropConfig&& other)
	{
		if (this != &other)
		{
			ID = other.ID;
			Money = other.Money;
			MinNumber = other.MinNumber;
			MaxNumber = other.MaxNumber;
			WhiteItemWeight = other.WhiteItemWeight;
			GreenItemWeight = other.GreenItemWeight;
			BlueItemWeight = other.BlueItemWeight;
			PurpleItemWeight = other.PurpleItemWeight;
			BossItem1 = other.BossItem1;
			BossItemNumber1 = other.BossItemNumber1;
			BossItemDropPercent1 = other.BossItemDropPercent1;
			BossItem2 = other.BossItem2;
			BossItemNumber2 = other.BossItemNumber2;
			BossItemDropPercent2 = other.BossItemDropPercent2;
			BossItem3 = other.BossItem3;
			BossItemNumber3 = other.BossItemNumber3;
			BossItemDropPercent3 = other.BossItemDropPercent3;
			WhiteItemList = std::move(other.WhiteItemList);
			GreenItemList = std::move(other.GreenItemList);
			BlueItemList = std::move(other.BlueItemList);
			PurpleItemList = std::move(other.PurpleItemList);
			PropID = std::move(other.PropID);
			PropNumber = std::move(other.PropNumber);
		}

		return *this;
	}
};

/// <summary>
/// 君主对应等级的经验、等级信息
/// </summary>
struct MasterLevelInfor
{
	/// <summary>
	/// 相应等级所需要的经验值
	/// </summary>
	int LevelNeededExp;
	/// <summary>
	/// 相应君主等级的武将等级限制
	/// </summary>
	int CharactorLevelLimit;
	/// <summary>
	/// 君主对应等级的体力上限
	/// </summary>
	int CharactorEnduranceLimit;
	/// <summary>
	/// 君主当前等级回复体力值
	/// </summary>
	int CharactorActionRecover;
};

/// <summary>
/// 放在服务端记录当前远征关卡信息的
/// </summary>
struct ExpeditionInstanceInfor
{
	/// <summary>
	/// 关卡ID
	/// </summary>
	int32_t levelID;
	/// <summary>
	/// 最小的开放等级
	/// </summary>
	int32_t minLevel;
	/// <summary>
	/// 最大的开放等级
	/// </summary>
	int32_t maxLevel;
	/// <summary>
	/// 关卡的金钱收入
	/// </summary>
	int32_t moneyProceeds;
	/// <summary>
	/// 关卡的军功收入
	/// </summary>
	int32_t exploitProceeds;
	/// <summary>
	/// 白色物品掉落几率
	/// </summary>
	int32_t whiteItemWeight;
	/// <summary>
	/// 绿色物品掉落几率
	/// </summary>
	int32_t greenItemWeight;
	/// <summary>
	/// 蓝色物品掉落几率
	/// </summary>
	int32_t blueItemWeight;
	/// <summary>
	/// 紫色物品掉落几率
	/// </summary>
	int32_t purpleItemWeight;
	/// <summary>
	/// 普通英雄掉落几率
	/// </summary>
	int32_t normalHeroWeight;
	/// <summary>
	/// 精良英雄掉落几率
	/// </summary>
	int32_t superiorHeroWeight;
	/// <summary>
	/// 稀有英雄掉落几率
	/// </summary>
	int32_t rareHeroWeight;
	/// <summary>
	/// 关卡难度
	/// </summary>
	float levelFactor;
	/// <summary>
	/// 关卡能掉落的普通英雄列表
	/// </summary>
	vector<int> normalHeroList;
	/// <summary>
	/// 关卡会掉落的精良英雄列表
	/// </summary>
	vector<int> superiorHeroList;
	/// <summary>
	/// 关卡会掉落的稀有英雄列表
	/// </summary>
	vector<int> rareHeroList;
	/// <summary>
	/// 关卡会掉落的白色物品列表
	/// </summary>
	vector<int> whiteItemList;
	/// <summary>
	/// 关卡会掉落的绿色物品列表
	/// </summary>
	vector<int> greenItemList;
	/// <summary>
	/// 关卡会掉落的蓝色物品列表
	/// </summary>
	vector<int> blueItemList;
	/// <summary>
	/// 关卡会掉落的紫色物品列表
	/// </summary>
	vector<int> purpleItemList;

	ExpeditionInstanceInfor()
	{
		levelID = 0;
		minLevel = 0;
		maxLevel = 0;
		moneyProceeds = 0;
		exploitProceeds = 0;
		whiteItemWeight = 0;
		greenItemWeight = 0;
		blueItemWeight = 0;
		purpleItemWeight = 0;
		normalHeroWeight = 0;
		superiorHeroWeight = 0;
		rareHeroWeight = 0;
		levelFactor = 0.0f;
		normalHeroList.clear();
		superiorHeroList.clear();
		rareHeroList.clear();
		whiteItemList.clear();
		greenItemList.clear();
		blueItemList.clear();
		purpleItemList.clear();
	}

	ExpeditionInstanceInfor(ExpeditionInstanceInfor&& other)
		:normalHeroList(std::move(other.normalHeroList))
		, superiorHeroList(std::move(other.superiorHeroList))
		, rareHeroList(std::move(other.rareHeroList))
		, whiteItemList(std::move(other.whiteItemList))
		, greenItemList(std::move(other.greenItemList))
		, blueItemList(std::move(other.blueItemList))
		, purpleItemList(std::move(other.purpleItemList))
	{
		levelID = other.levelID;
		minLevel = other.minLevel;
		maxLevel = other.maxLevel;
		moneyProceeds = other.moneyProceeds;
		exploitProceeds = other.exploitProceeds;
		whiteItemWeight = other.whiteItemWeight;
		greenItemWeight = other.greenItemWeight;
		blueItemWeight = other.blueItemWeight;
		purpleItemWeight = other.purpleItemWeight;
		normalHeroWeight = other.normalHeroWeight;
		superiorHeroWeight = other.superiorHeroWeight;
		rareHeroWeight = other.rareHeroWeight;
		levelFactor = other.levelFactor;
	}

	ExpeditionInstanceInfor& operator = (ExpeditionInstanceInfor&& other)
	{
		if (this != &other)
		{
			levelID = other.levelID;
			minLevel = other.minLevel;
			maxLevel = other.maxLevel;
			moneyProceeds = other.moneyProceeds;
			exploitProceeds = other.exploitProceeds;
			whiteItemWeight = other.whiteItemWeight;
			greenItemWeight = other.greenItemWeight;
			blueItemWeight = other.blueItemWeight;
			purpleItemWeight = other.purpleItemWeight;
			normalHeroWeight = other.normalHeroWeight;
			superiorHeroWeight = other.superiorHeroWeight;
			rareHeroWeight = other.rareHeroWeight;
			levelFactor = other.levelFactor;
			normalHeroList = std::move(other.normalHeroList);
			superiorHeroList = std::move(other.superiorHeroList);
			rareHeroList = std::move(other.rareHeroList);
			whiteItemList = std::move(other.whiteItemList);
			greenItemList = std::move(other.greenItemList);
			blueItemList = std::move(other.blueItemList);
			purpleItemList = std::move(other.purpleItemList);
		}
		return *this;
	}
};

///@breif 武将进阶数据
struct HeroRankData
{
	float m_Power; ///力量	增加
	float m_Agility; ///初始敏捷增加		
	float m_Intelligence; ///智力增加	
	std::vector<int> m_EquipIDs;
};

///@breif 武将进阶配置结构
struct HeroRankConfig
{
	map<int, HeroRankData> m_RankData;
public:
	HeroRankConfig()
	{
		m_RankData.clear();
	}

	HeroRankConfig(int rankLevel, HeroRankData&& data)
	{
		m_RankData.clear();
		m_RankData.insert(make_pair(rankLevel, std::move(data)));
	}

	HeroRankConfig(HeroRankConfig&& other)
	{
		m_RankData = std::move(other.m_RankData);
	}

	HeroRankConfig& operator = (HeroRankConfig&& other)
	{
		if (this != &other)
		{
			m_RankData = std::move(other.m_RankData);
		}

		return *this;
	}

	const HeroRankData* GetRankData(int rankLevel) const
	{
		auto iter = m_RankData.find(rankLevel);
		return iter == m_RankData.end() ? nullptr : &iter->second;
	}
};

struct AttrGrowth
{
	float PowerGrowth1;
	float AgilityGrowth;
	float IntelligenceGrowth;
	int Item;
	int Number;
	AttrGrowth()
	{
		PowerGrowth1 = 0;
		AgilityGrowth = 0;
		IntelligenceGrowth = 0;
		Item = 0;
		Number = 0;
	}
};

///@brief 武将配置结构
struct HeroConfig
{
	int32_t HeroId;
	int32_t Skill1;
	int32_t Skill2;
	int32_t Skill3;
	int32_t Skill4;
	int32_t Type;
	int32_t Star;
	int32_t Rank;
	float AttackCD;
	string URL;
	CharacterAttrData baseAttribute;///<基础数据

	HeroConfig()
		:baseAttribute()
	{
		HeroId = 0;
		Skill1 = 0;
		Skill2 = 0;
		Skill3 = 0;
		Skill4 = 0;
		Type = 0;
		Star = 0;
		Rank = 0;
		AttackCD = 0.0f;
		URL = "";
	}

	HeroConfig(HeroConfig&& other)
		:baseAttribute(std::move(other.baseAttribute))
	{
		HeroId = other.HeroId;
		Skill1 = other.Skill1;
		Skill2 = other.Skill2;
		Skill3 = other.Skill3;
		Skill4 = other.Skill4;
		Type = other.Type;
		Star = other.Star;
		Rank = other.Rank;
		AttackCD = other.AttackCD;
		URL = other.URL;
	}

	HeroConfig& operator = (HeroConfig&& other)
	{
		if (this != &other)
		{
			HeroId = other.HeroId;
			Skill1 = other.Skill1;
			Skill2 = other.Skill2;
			Skill3 = other.Skill3;
			Skill4 = other.Skill4;
			Type = other.Type;
			Star = other.Star;
			Rank = other.Rank;
			AttackCD = other.AttackCD;
			URL = other.URL;
			baseAttribute = std::move(other.baseAttribute);
		}

		return *this;
	}
};

struct HeroStarConfig
{
	int HeroID;
	map<int, AttrGrowth> StarData;

public:
	HeroStarConfig()
	{
		HeroID = 0;
		StarData.clear();
	}

	HeroStarConfig(int heroID, int starLevel, AttrGrowth&& attr)
	{
		HeroID = heroID;
		StarData.clear();
		StarData.insert(make_pair(starLevel, std::move(attr)));
	}

	bool AddStarAttrData(int starLevel, AttrGrowth&& attr)
	{
		if (StarData.find(starLevel) != StarData.end())
			return false;

		StarData.insert(make_pair(starLevel, std::move(attr)));
		return true;
	}

	HeroStarConfig(HeroStarConfig&& other)
		:StarData(std::move(other.StarData))
	{
		HeroID = other.HeroID;
	}

	HeroStarConfig& operator = (HeroStarConfig&& other)
	{
		if (this != &other)
		{
			HeroID = other.HeroID;
			StarData = std::move(other.StarData);
		}
		
		return *this;
	}

	///@brief 获取本英雄最大的星级
	///@return 成功返回对应的值，失败返回-1
	int GetMaxStarLevel()
	{
		if (StarData.size() == 0)
			return -1;

		auto endItor = StarData.end();
		--endItor;
		return endItor->first;
	}

	const AttrGrowth* GetHeroStarAttrGrowth(int starLevel) const
	{
		auto findResult = StarData.find(starLevel);
		if (findResult == StarData.end())
			return nullptr;

		return &(findResult->second);
	}
};

struct MonsterConfig
{
	int32_t monsterID;
	float attackCD;
	float attackRange;
	CharacterAttrData baseAttribute;///<基础数据

	MonsterConfig()
		:baseAttribute()
	{
		monsterID = 0;
		attackCD = 0;
		attackRange = 0;
	}

	MonsterConfig(MonsterConfig&& other)
		:baseAttribute(std::move(other.baseAttribute))
	{
		monsterID = other.monsterID;
		attackCD = other.attackCD;
		attackRange = other.attackRange;
	}

	MonsterConfig& operator = (MonsterConfig&& other)
	{
		monsterID = other.monsterID;
		attackCD = other.attackCD;
		attackRange = other.attackRange;
		baseAttribute = std::move(other.baseAttribute);
	}
};

///@breif 装备配置
struct EquipmentConfig
{
	int32_t m_EquipmentID; ///装备ID
	int32_t m_EquipmentType; ///装备部位
	int32_t levelLimit;	///等级限制
	int32_t refiningExp; //精炼经验
	///< 金币购买价格 //
	int32_t buyGoldPrice;
	///< 钻石购买价格 //
	int32_t buyDiamondPrice;
	///< 荣誉购买价格 //
	int32_t buyHonorPrice;
	///< 军功购买价格 //
	int32_t buyExploitPrice;
	//购买本道具得花费的魂点数量
	int32_t BuySoulPointsCost;
	//分解得到的魂点数量
	int32_t ResolveSoulPoints;
	///< 声望购买价格 //
	int32_t buyPrestigePrice;
	///< 军需令购买价格
	int32_t buyTokenPrice;
	///< 出售价格(出售只换取金币) //
	int32_t sellPrice;
	///< 合成装备所需的碎片ID //
	int32_t pieceID;
	int32_t Quality;///装备的成色，比如白色装备、绿色装备、蓝色装备等
	CharacterAttrData baseAttribute;///<基础数据

	EquipmentConfig()
		:baseAttribute()
	{
		m_EquipmentID = 0;
		m_EquipmentType = 0;
		levelLimit = 0;
		refiningExp = 0;
		buyGoldPrice = 0;
		buyDiamondPrice = 0;
		buyHonorPrice = 0;
		buyExploitPrice = 0;
		buyPrestigePrice = 0;
		ResolveSoulPoints = 0;
		BuySoulPointsCost = 0;
		buyTokenPrice = 0;
		sellPrice = 0;
		pieceID = 0;
		Quality = 0;
	}

	EquipmentConfig(EquipmentConfig&& other)
		:baseAttribute(std::move(other.baseAttribute))
	{
		m_EquipmentID = other.m_EquipmentID;
		m_EquipmentType = other.m_EquipmentType;
		levelLimit = other.levelLimit;
		refiningExp = other.refiningExp;
		buyGoldPrice = other.buyGoldPrice;
		buyDiamondPrice = other.buyDiamondPrice;
		buyHonorPrice = other.buyHonorPrice;
		buyExploitPrice = other.buyExploitPrice;
		buyPrestigePrice = other.buyPrestigePrice;
		ResolveSoulPoints = other.ResolveSoulPoints;
		BuySoulPointsCost = other.BuySoulPointsCost;
		buyTokenPrice = other.buyTokenPrice;
		sellPrice = other.sellPrice;
		pieceID = other.pieceID;
		Quality = other.Quality;
	}

	EquipmentConfig& operator = (EquipmentConfig&& other)
	{
		if (this != &other)
		{
			m_EquipmentID = other.m_EquipmentID;
			m_EquipmentType = other.m_EquipmentType;
			levelLimit = other.levelLimit;
			refiningExp = other.refiningExp;
			buyGoldPrice = other.buyGoldPrice;
			buyDiamondPrice = other.buyDiamondPrice;
			buyHonorPrice = other.buyHonorPrice;
			buyExploitPrice = other.buyExploitPrice;
			buyPrestigePrice = other.buyPrestigePrice;
			ResolveSoulPoints = other.ResolveSoulPoints;
			BuySoulPointsCost = other.BuySoulPointsCost;
			buyTokenPrice = other.buyTokenPrice;
			sellPrice = other.sellPrice;
			pieceID = other.pieceID;
			Quality = other.Quality;
			baseAttribute = std::move(other.baseAttribute);
		}

		return *this;
	}
};

///@brief 装备精练配置，暂时没存某一星级的装备的对应的属性加成
struct ForgingConfig
{
	int id;
	int refiningTime;
	std::map<int, int> levelExp;
	CharacterAttrData baseAttribute;///<基础数据

	ForgingConfig()
		:baseAttribute()
	{
		id = 0;
		refiningTime = 0;
		levelExp.clear();
	}

	ForgingConfig(ForgingConfig&& other)
		:baseAttribute(std::move(other.baseAttribute))
		, levelExp(std::move(other.levelExp))
	{
		id = other.id;
		refiningTime = other.refiningTime;
	}

	ForgingConfig& operator = (ForgingConfig&& other)
	{
		if (this != &other)
		{
			id = other.id;
			refiningTime = other.refiningTime;
			levelExp = std::move(other.levelExp);
			baseAttribute = std::move(other.baseAttribute);
		}

		return *this;
	}
};


/**@brief 商城物品配置*/
struct CommodityConfig
{
	int32_t minLimitLevel;
	int32_t maxLimitLevel;
	int32_t articleShelf;    //物品格//
	int32_t minNumber;
	int32_t maxNumber;
	vector<int32_t> vecItem1;
	int percent1;
	vector<int32_t> vecItem2;
	int percent2;
	vector<int32_t> vecItem3;
	int percent3;
	vector<int32_t> vecItem4;
	int percent4;
	vector<int32_t> vecItem5;
	int percent5;

	bool InLevelRange(int32_t level) const
	{
		return (minLimitLevel <= level && level <= maxLimitLevel) ? true : false;
	}
};

///@wk 30天签到奖励
struct CheckInConfig
{
	int32_t times;//签到天数
	int32_t doubleNeedVip;//双倍需要的vip等级
	GoodsInfoSG Goods[2];//2个奖励
};
///@wk 7天签到奖励
struct CheckInday7Config
{
	int32_t SignInDay;//签到天数
	GoodsInfoSG Goods[2];//1为首次签到;2为普通奖励
};
struct ArenaRewardConfig {

	 int rankUp;
	 int rankFloor;
	 int Gold;
	 int  Gem;
	 int RefinedMaterial;
	 int RefinedMaterialNumber;
	 int Medicine;
	 int MedicineNumber;
	 int Honor;
	 float BestRecord;
};

struct ArenaRobotLevelInfo
{
	int uper;
	int lower;
	int heroLevel;
	int rankLevel;
	int starLevel;
	int skillLevel;
	int level;
};

///@brief 新手引导触发信息
struct NewbieGuideConfig
{
	int guideType;				///<引导类型
	int startIndex;				///<开始索引
	int nextTollgateID;			///<领取武将触发的关卡ID
	int rewardID;				///<领取额武将的武将ID
	vector<int> vecOperateIndex;///<与服务器交互的操作步骤
};

///@brief 武将缘分配置结构
struct FateConfig
{
	int heroID;							///<缘分主武将
	vector<int> vecRelationshipHero;	///<缘分武将
	vector<int> vecAttributeType;		///<缘分加成属性类型
	vector<float> vecAttributePercent;	///<缘分加成属性比例
};

///@brief 武将技能配置结构
struct HeroSkillConfig
{
	int id;				///< 技能ID
	int type;			///< 技能类型
	int buffID1;		///< 第一个buffID
	int buffID2;		///< 第二个buffID
	int selfBuffID;		///< 自身加成buffID
	int effective;		///< 技能生效初始等级
	float damage;		///< 技能附加初始伤害
	float addDamage;	///< 技能附加升级伤害
	float extraDamage;	///< 修正值
	float factor;		///< 技能伤害系数
};

///@brief buff配置表
struct BuffConfig
{
	int id;
	int style;			///< 类型(0:状态类 1:其他类)
	float initiaValue;	///< 初始值
	float addValue;		///< 升级加成
	int EffectiveStyle; ///< 作用方式
	string name;		///< 名称
};