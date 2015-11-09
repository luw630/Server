#include "..\..\stdafx.h"
#include "ConfigManager.h"
#include "TabReader.h"
#include "../../../../NETWORKMODULE/SanguoPlayer.h"
#include "..\..\ServerProjects\NETWORKMODULE\SanguoPlayerMsg.h"
#include "..\..\ServerProjects\NETWORKMODULE\MailMsg_SanGuo.h"
#include "Random.h"

CConfigManager::CConfigManager()
{
	m_iHeroTrainingCfgNums = 0;
	m_iHeroHungingHuntingNums = 0;
	m_arrayHeroTrainingCfg = nullptr;
	m_arrayHeroHungingHuntingCfg = nullptr;
	m_ExpeditionInstanceInfor.clear();
	m_HeorConfigMap.clear();
	InitGolalConfig();
	InitGameFeatureActivationConfig();
	InitHeroConfig();
	InitHeroStarConfig();
	InitExpeditionConfig();
	InitEquipConfig();
	InitExchangeGoldConfig();
	_InitItemConfig();
	InitForgingConfig();
	InitMonsterConfig();
	InitChapterConfig();
	//InitChapterBattleScoreRewardsConfig();
	InitBattleLevelConfig();
	InitBattleFristDropConfig();
	InitBattleDropConfig();
	InitMasterLevelInfor();
	InitMissionConfig();
	InitMissionActivenessRewardsConfig();
	InitHeroExpConfig();
	InitHeroRankConfig();
	InitHeroTrainingCfg();
	InitHeroHuntingHuntingCfg();
	InitAchievementConfig();
	InitRandomAchievementConfig();
	_InitShopCommodityConfig("ServerConfig\\ZaHuoPu.txt", m_vecVarietyShopCommodity);
	_InitShopCommodityConfig("ServerConfig\\JingJiChangShangDian.txt", m_vecArenaShopCommodity);
	_InitShopCommodityConfig("ServerConfig\\GuoGuanZhanJiangShangDian.txt", m_vecExpeditionShopCommodity);
	_InitShopCommodityConfig("ServerConfig\\JunTuanShangDian.txt", m_vecLegionShopCommodity);
	_InitShopCommodityConfig("ServerConfig\\QiYuanShangRen.txt", m_vecMiracleMerchantCommodity);
	_InitShopCommodityConfig("ServerConfig\\ZhenBaoShangRen.txt", m_vecGemMerchantCommodity);
	_InitShopCommodityConfig("ServerConfig\\JiangHunShangDian.txt", m_vecSoulExchangeCommodity);
	_InitShopCommodityConfig("ServerConfig\\ZhenYingZhanShangDian.txt", m_vecWarOfLeagueShopCommodity);
	_InitRobotNameConfig();
	InitCheckinAwardsConfig();
	InitArenaRewardConfigList();
	_InitArenaRobotLevelInfo();
	InitCheckinDay7AwardsConfig();
	_InitGuideConfig();
	_InitFateConfig();
	_InitHeroSkillConfig();
	_InitBuffConfig();
	_InitResistanceRateCalculateConfig();
}


CConfigManager::~CConfigManager()
{
	delete[] m_arrayHeroTrainingCfg;
	m_arrayHeroTrainingCfg = nullptr;

	delete[] m_arrayHeroHungingHuntingCfg;
	m_arrayHeroHungingHuntingCfg = nullptr;
}

bool CConfigManager::InitExpeditionConfig()
{
	string fileName = "ServerConfig\\YuanZheng.txt";
	CTabTableReader fileReader(fileName);
	if (fileReader.OpenTabTable(fileName) == false)
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	while (fileReader.ReadLine())
	{
		ExpeditionInstanceInfor instanceConfig;
		instanceConfig.levelID = fileReader.GetIntValue("ID");
		instanceConfig.minLevel = fileReader.GetIntValue("MinLevel");
		instanceConfig.maxLevel = fileReader.GetIntValue("MaxLevel");
		instanceConfig.levelFactor = fileReader.GetFloatValue("Hardlevel");
		instanceConfig.moneyProceeds = fileReader.GetIntValue("Money");
		instanceConfig.exploitProceeds = fileReader.GetIntValue("Exploit");
		instanceConfig.whiteItemWeight = fileReader.GetIntValue("WhiteItemWeight");
		instanceConfig.greenItemWeight = fileReader.GetIntValue("GreenItemWeight");
		instanceConfig.blueItemWeight = fileReader.GetIntValue("BlueItemWeight");
		instanceConfig.purpleItemWeight = fileReader.GetIntValue("PurpleItemWeight");

		string ItemStr = fileReader.GetStrValue("WhiteItem");
		instanceConfig.whiteItemList.clear();
		CTabTableReader::StringSplit(ItemStr, "|", instanceConfig.whiteItemList);

		ItemStr = fileReader.GetStrValue("GreenItem");
		instanceConfig.greenItemList.clear();
		CTabTableReader::StringSplit(ItemStr, "|", instanceConfig.greenItemList);

		ItemStr = fileReader.GetStrValue("BlueItem");
		instanceConfig.blueItemList.clear();
		CTabTableReader::StringSplit(ItemStr, "|", instanceConfig.blueItemList);

		ItemStr = fileReader.GetStrValue("PurpleItem");
		instanceConfig.purpleItemList.clear();
		CTabTableReader::StringSplit(ItemStr, "|", instanceConfig.purpleItemList);

		instanceConfig.normalHeroWeight = fileReader.GetIntValue("StageWeight1");
		instanceConfig.superiorHeroWeight = fileReader.GetIntValue("StageWeight2");
		instanceConfig.rareHeroWeight = fileReader.GetIntValue("StageWeight3");

		ItemStr = fileReader.GetStrValue("StageId1");
		instanceConfig.normalHeroList.clear();
		CTabTableReader::StringSplit(ItemStr, "|", instanceConfig.normalHeroList);

		ItemStr = fileReader.GetStrValue("StageId2");
		instanceConfig.superiorHeroList.clear();
		CTabTableReader::StringSplit(ItemStr, "|", instanceConfig.superiorHeroList);

		ItemStr = fileReader.GetStrValue("StageId3");
		instanceConfig.rareHeroList.clear();
		CTabTableReader::StringSplit(ItemStr, "|", instanceConfig.rareHeroList);
		
		auto findResult = m_ExpeditionInstanceInfor.find(instanceConfig.levelID);
		if (findResult == m_ExpeditionInstanceInfor.end())
		{
			map<int32_t, ExpeditionInstanceInfor> expeditionTempInfor;
			int levelID = instanceConfig.levelID;
			expeditionTempInfor.insert(make_pair(instanceConfig.minLevel, move(instanceConfig)));
			m_ExpeditionInstanceInfor.insert(make_pair(levelID, move(expeditionTempInfor)));
		}
		else
		{
			auto subFindResult = findResult->second.find(instanceConfig.minLevel);
			if (subFindResult != findResult->second.end())
			{
				rfalse("远征的关卡%d配置的君主等级的最小值有重复值", instanceConfig.levelID);
			}
			else
			{
				findResult->second.insert(make_pair(instanceConfig.minLevel, move(instanceConfig)));
			}
		}
	}

	return true;
}

bool CConfigManager::InitGolalConfig()
{
	string fileName = "ServerConfig\\QuanJuPeiZhi.txt";
	CTabTableReader fileReader(fileName);
	globalConfig.PhysicalLimit = fileReader.GetSecondRowIntValue("PhysicalLimit");
	globalConfig.PhysicalRecoverTime = fileReader.GetSecondRowIntValue("PhysicalRecoverTime");
	globalConfig.BuyPhysical = fileReader.GetSecondRowIntValue("BuyPhysical");
	globalConfig.SkillCostMoney = fileReader.GetSecondRowIntValue("SkillCostMoney");
	globalConfig.LotteryOnceMoney = fileReader.GetSecondRowIntValue("LotteryOnceMoney");
	globalConfig.LotteryTenTimesMoney = fileReader.GetSecondRowIntValue("LotteryTenTimesMoney");
	globalConfig.LotteryOnceGold = fileReader.GetSecondRowIntValue("LotteryOnceGold");
	globalConfig.LotteryTenTimesGold = fileReader.GetSecondRowIntValue("LotteryTenTimesGold");
	globalConfig.NotVipBuyPhysicalTimes = fileReader.GetSecondRowIntValue("NotVipBuyPhysicalTimes");
	globalConfig.BlessFreeTime = fileReader.GetSecondRowIntValue("BlessFreeTime");
	globalConfig.GetHeroForCashParam1 = fileReader.GetSecondRowFloatValue("GetHeroForCashParam1");
	globalConfig.GetHeroForCashParam2 = fileReader.GetSecondRowFloatValue("GetHeroForCashParam2");
	globalConfig.LearnSkillLevel = fileReader.GetSecondRowIntValue("LearnSkill");
	globalConfig.SkillPointRecoverTime = fileReader.GetSecondRowIntValue("SkillPointRecoverTime");
	globalConfig.SkillPointLimit = fileReader.GetSecondRowIntValue("SkillPointLimit");
	globalConfig.BattleLimitedTime = fileReader.GetSecondRowIntValue("BattleLimitedTime");
	globalConfig.CrusadeHeroLevelLimit = fileReader.GetSecondRowIntValue("CrusadeHeroLevelLimit");
	globalConfig.BlessFreeTimeForMoney = fileReader.GetSecondRowIntValue("BlessFreeTimeForMoney");
	globalConfig.FristBlessHero = fileReader.GetSecondRowIntValue("FristBlessHero");
	globalConfig.SecondBlessHero = fileReader.GetSecondRowIntValue("SecondBlessHero");
	globalConfig.HeroPiecesPriceRatio = fileReader.GetSecondRowIntValue("HeroPiecesPriceRatio");
	globalConfig.DiamondSweepCost = fileReader.GetSecondRowIntValue("DiamondSweep");
	//globalConfig.HeroPiecesPriceRatio = fileReader.GetSecondRowIntValue("HeroPiecesPriceRatio");
	globalConfig.FirstTimeGetHeroForMoney = fileReader.GetSecondRowIntValue("FirstTimeGetHeroForMoney");
	globalConfig.FirstTimeGetHeroForCash = fileReader.GetSecondRowIntValue("FirstTimeGetHeroForCash");
	globalConfig.OneStarHeroTransform = fileReader.GetSecondRowIntValue("OneStarHeroTransform");
	globalConfig.TwoStarHeroTransform = fileReader.GetSecondRowIntValue("TwoStarHeroTransform");
	globalConfig.ThreeStarHeroTransform = fileReader.GetSecondRowIntValue("ThreeStarHeroTransform");
	globalConfig.BuySkillPointCost = fileReader.GetSecondRowIntValue("BuySkills");
	globalConfig.GreenEquipForRestore = fileReader.GetSecondRowFloatValue("GreenEquipForRestore");
	globalConfig.BluenEquipForRestore = fileReader.GetSecondRowFloatValue("BluenEquipForRestore");
	globalConfig.PurpleEquipForRestore = fileReader.GetSecondRowFloatValue("PurpleEquipForRestore");
	globalConfig.ForgeExpRestoreItem = fileReader.GetSecondRowIntValue("ForgeExpRestoreItem");
	globalConfig.ForgeMoneyCostScale = fileReader.GetSecondRowIntValue("ForgeMoneyCostScale");
	globalConfig.ForgeDiamondCostScale = fileReader.GetSecondRowIntValue("ForgeDiamondCostScale");
	globalConfig.LuckyTimeGap = fileReader.GetSecondRowIntValue("LuckyTimeGap");
	globalConfig.LuckyTimePersistentTime = fileReader.GetSecondRowIntValue("LuckyTimePersistentTime");
	globalConfig.MaxRandomAchievementAccomplishTimes = fileReader.GetSecondRowIntValue("MaxRandomAchievementAccomplishTimes");
	globalConfig.ChaseThiefDurationTime = fileReader.GetSecondRowIntValue("ChaseThiefDurationTime");
	globalConfig.ChangeNameConsumption = fileReader.GetSecondRowIntValue("ChangeNameConsumption");//获取改名所需元宝
	globalConfig.CrusadeInitMana = fileReader.GetSecondRowIntValue("CrusadeMana");
	globalConfig.ChangeNameConsumptionFreeTime = fileReader.GetSecondRowIntValue("ChangeNameConsumptionFreeTime");//免费改名的次数
	globalConfig.ChaseThiefOpenLevel = fileReader.GetSecondRowIntValue("ChaseThiefOpenLevel");
	globalConfig.ChaseThiefActivateRate = fileReader.GetSecondRowFloatValue("ChaseThiefRate");

	globalConfig.PlayerDonateMaxNum = fileReader.GetSecondRowFloatValue("PlayerDonateMaxNum");//军团每日捐献上限
	globalConfig.GemAndPrestigeRatio = fileReader.GetSecondRowFloatValue("GemAndPrestigeRatio");//军团宝石和声望转化值
	globalConfig.MoenyAndPrestigeRatio = fileReader.GetSecondRowFloatValue("MoenyAndPrestigeRatio");//军团金币和声望转化值
	globalConfig.CreateFactionNeedMoney = fileReader.GetSecondRowFloatValue("CreateFactionNeedMoney");//军团创建所需金币
	globalConfig.FactionDayGetMaxExp = fileReader.GetSecondRowFloatValue("FactionDayGetMaxExp");//军团每日经验上限
	globalConfig.HeroTrainingCountLimit = fileReader.GetSecondRowFloatValue("HeroTrainingCountLimit");//武将训练人数上限
	globalConfig.HeroTrainingNeedTime = fileReader.GetSecondRowFloatValue("HeroTrainingNeedTime");;//武将训练所需时间(秒)
	globalConfig.HeroTrainingUnitTime = fileReader.GetSecondRowFloatValue("HeroTrainingUnitTime");;//武将训练经验值计算的单位时间
	globalConfig.HeroHuntingCountLimit = fileReader.GetSecondRowFloatValue("HeroHuntingCountLimit");;//武将寻宝人数上限
	globalConfig.HeroHuntingLevelLimit = fileReader.GetSecondRowFloatValue("HeroHuntingLevelLimit");;//武将寻宝等级限制（最低等级限制）
	globalConfig.HeroHuntingTime = fileReader.GetSecondRowFloatValue("HeroHuntingTime");;//武将寻宝时间
	globalConfig.HeroHuntingGoldClearingUnitTime = fileReader.GetSecondRowFloatValue("HeroHuntingGoldClearingUnitTime");;//武将寻宝金币收益计算时间单位
	globalConfig.HeroHuntingSurpriseUnitTime = fileReader.GetSecondRowFloatValue("HeroHuntingSurpriseUnitTime");;//武将寻宝掉宝间隔时间单位

	string tempStr = fileReader.GetSecondRowStrValue("StoreRefresh");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecVarietyShopRefreshCost);
	tempStr = fileReader.GetSecondRowStrValue("StoreRefreshTime");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecVarietyShopRefreshClock);

	tempStr = fileReader.GetSecondRowStrValue("SoulExchangeRefreshCost");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecSoulExchangeRefreshCost);
	tempStr = fileReader.GetSecondRowStrValue("SoulExchangeStoreRefreshTime");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecSoulExchangeRefreshClock);

	tempStr = fileReader.GetSecondRowStrValue("ArenaStoreRefresh");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecArenaShopRefreshCost);
	tempStr = fileReader.GetSecondRowStrValue("ArenaStoreRefreshTime");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecArenaShopRefreshClock);

	tempStr = fileReader.GetSecondRowStrValue("ExpeditionStoreRefresh");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecExpeditionShopRefreshCost);
	tempStr = fileReader.GetSecondRowStrValue("ExpeditionStoreRefreshTime");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecExpeditionShopRefreshClock);

	tempStr = fileReader.GetSecondRowStrValue("LegionStoreRefresh");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecLegionShopRefreshCost);
	tempStr = fileReader.GetSecondRowStrValue("LegionStoreRefreshTime");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecLegionShopRefreshClock);

	tempStr = fileReader.GetSecondRowStrValue("TreasureStoreRefresh");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecMiracleMerchantRefreshCost);
	tempStr = fileReader.GetSecondRowStrValue("TreasureStoreRefreshTime");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecMiracleMerchantRefreshClock);

	tempStr = fileReader.GetSecondRowStrValue("GemStoreRefresh");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecGemMerchantRefreshCost);
	tempStr = fileReader.GetSecondRowStrValue("GemStoreRefreshTime");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecGemMerchantRefreshClock);

	tempStr = fileReader.GetSecondRowStrValue("CampExchangeRefreshCost");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecWarOfLeagueShopRefreshCost);
	tempStr = fileReader.GetSecondRowStrValue("CampExchangeStoreRefreshTime");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecWarOfLeagueShopRefreshClock);

	tempStr = fileReader.GetSecondRowStrValue("HeroSummonCost");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecHeroSummonCost);
	tempStr = fileReader.GetSecondRowStrValue("HeroEvolutionCost");
	CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), globalConfig.vecHeroEvolutionCost);

	globalConfig.miracleMerchantOpenProbability = fileReader.GetSecondRowFloatValue("TreasureStoreOpen");
	globalConfig.MiracleMerchantVIPLevelLimit = fileReader.GetSecondRowIntValue("TreasureStoreRandomVIPLevelLimit");
	globalConfig.MiracleMerchantMasterLevelLimit = fileReader.GetSecondRowIntValue("TreasureStoreOpenMasterLevelLimit");
	globalConfig.gemMerchantOpenProbability = fileReader.GetSecondRowFloatValue("GemStoreOpen");
	globalConfig.GemMerchantVIPLevelLimit = fileReader.GetSecondRowIntValue("GemStoreRandomVIPLevelLimit");
	globalConfig.GemMerchantMasterLevelLimit = fileReader.GetSecondRowIntValue("GemStoreOpenMasterLevelLimit");
	globalConfig.LogMoneyNum = fileReader.GetSecondRowIntValue("LogMoneyNum");
	globalConfig.LogItemNum = fileReader.GetSecondRowIntValue("LogItemNum");
	globalConfig.LogItemIdBengin1 = fileReader.GetSecondRowIntValue("LogItemIdBengin1");
	globalConfig.LogItemIdEnd1 = fileReader.GetSecondRowIntValue("LogItemIdEnd1");
	globalConfig.LogItemIdBengin2 = fileReader.GetSecondRowIntValue("LogItemIdBengin2");
	globalConfig.LogItemIdEnd2 = fileReader.GetSecondRowIntValue("LogItemIdEnd2");
	globalConfig.LogItemIdBengin3 = fileReader.GetSecondRowIntValue("LogItemIdBengin3");
	globalConfig.LogItemIdEnd3 = fileReader.GetSecondRowIntValue("LogItemIdEnd3");
	globalConfig.MovingTime = fileReader.GetSecondRowFloatValue("MovingTime");
	globalConfig.FightingCapacityFaultTolerant = fileReader.GetSecondRowFloatValue("FightingCapacityFaultTolerant");
	globalConfig.AttackingFaultTolerant = fileReader.GetSecondRowFloatValue("AttackFaultTolerant");
	globalConfig.GethitFaultTolerant = fileReader.GetSecondRowFloatValue("GethitFaultTolerant");
	globalConfig.BlessHeroLimit = fileReader.GetSecondRowIntValue("BlessHeroLimit");
	globalConfig.TreasureStoreOpenCost = fileReader.GetSecondRowIntValue("TreasureStoreOpenCost");
	globalConfig.GemStoreOpenCost = fileReader.GetSecondRowIntValue("GemStoreOpenCost");
	return true;
}

bool CConfigManager::InitGameFeatureActivationConfig()
{
	string fileName = "ServerConfig\\GongNengKaiFang.txt";
	CTabTableReader fileReader;
	if (fileReader.OpenTabTable(fileName) == false)
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	m_gameFeatureActivationConfig.shoppingLevelLimit = fileReader.GetSecondRowIntValue("Mall");
	m_gameFeatureActivationConfig.eliteDungeonLevelLimit = fileReader.GetSecondRowIntValue("EliteDuplicate");
	m_gameFeatureActivationConfig.midasLevelLimit = fileReader.GetSecondRowIntValue("Money");
	m_gameFeatureActivationConfig.treasureHuntingDungeonLevelLimit = fileReader.GetSecondRowIntValue("TreasureHunting");
	m_gameFeatureActivationConfig.arenaLevelLimit = fileReader.GetSecondRowIntValue("Arena");
	m_gameFeatureActivationConfig.leagueOfLegendPalaceLevelLimit = fileReader.GetSecondRowIntValue("Legend");
	m_gameFeatureActivationConfig.expeditionDungeonLevelLimit = fileReader.GetSecondRowIntValue("Expedition");
	m_gameFeatureActivationConfig.forgeShopLevelLimit = fileReader.GetSecondRowIntValue("Forging");
	m_gameFeatureActivationConfig.teamSiegeLevelLimit = fileReader.GetSecondRowIntValue("ArmyGroup");
	m_gameFeatureActivationConfig.reinforcementsLevelLimit = fileReader.GetSecondRowIntValue("Reinforcements");
	m_gameFeatureActivationConfig.mopUpTollgateLevelLimit = fileReader.GetSecondRowIntValue("Sweep");
	m_gameFeatureActivationConfig.MoneyHand = fileReader.GetSecondRowIntValue("Money");
	m_gameFeatureActivationConfig.Relationship = fileReader.GetSecondRowIntValue("Friend");
	m_gameFeatureActivationConfig.HeroTrainingLimit = fileReader.GetSecondRowIntValue("HeroTraining");
	m_gameFeatureActivationConfig.HeroHungingHuntingLimit = fileReader.GetSecondRowIntValue("HeroHunting");
	m_gameFeatureActivationConfig.soulExchangeLimit = fileReader.GetSecondRowIntValue("SoulExchange");

	return true;
}

const GameFeatureActivationConfig& CConfigManager::GetGameFeatureActivationConfig()
{
	return m_gameFeatureActivationConfig;
}

bool CConfigManager::InitHeroConfig()
{

	////初始化武将配置表
	m_HeorConfigMap.clear();
	m_vecAvaliableStrengthHeroList.clear();
	m_vecAvaliableAgilityHeroList.clear();
	m_vecAvaliableManaHeroList.clear();
	string fileName = "ServerConfig\\YingXiongShuXing.txt";
	CTabTableReader fileReader;
	if (fileReader.OpenTabTable(fileName) == false)
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}
	m_RangeHeroIDList.clear();
	m_MeleeHeroIDList.clear();
	int tempInt = -1;
	while (fileReader.ReadLine())
	{
		HeroConfig newConfig;
		newConfig.HeroId = fileReader.GetIntValue("ID");
		newConfig.Skill1 = fileReader.GetIntValue("Skill1");
		newConfig.Skill2 = fileReader.GetIntValue("Skill2");
		newConfig.Skill3 = fileReader.GetIntValue("Skill3");
		newConfig.Skill4 = fileReader.GetIntValue("Skill4");

		newConfig.baseAttribute.Power = fileReader.GetFloatValue("Power");
		newConfig.baseAttribute.Intelligence = fileReader.GetFloatValue("Intelligence");
		newConfig.baseAttribute.Agility = fileReader.GetFloatValue("Agility");
		newConfig.baseAttribute.MaxHealth = fileReader.GetFloatValue("MaxHealth");
		newConfig.baseAttribute.Physic = fileReader.GetFloatValue("Physic");
		newConfig.baseAttribute.Magic = fileReader.GetFloatValue("Magic");
		newConfig.baseAttribute.Armor = fileReader.GetFloatValue("Armor");
		newConfig.baseAttribute.Resistance = fileReader.GetFloatValue("Resistance");
		newConfig.baseAttribute.Crit = fileReader.GetFloatValue("Crit");
		newConfig.AttackCD = fileReader.GetFloatValue("AttackCD");

		//初始化远程和近战的武将ID列表
		tempInt = fileReader.GetIntValue("AttackRange");
		if (tempInt > 5) //攻击范围大于5就是远程
		{
			m_RangeHeroIDList.push_back(newConfig.HeroId);
		}
		else
		{
			m_MeleeHeroIDList.push_back(newConfig.HeroId);
		}

		newConfig.Rank = fileReader.GetIntValue("Rank");
		newConfig.Star = fileReader.GetIntValue("Star");
		newConfig.Type = fileReader.GetIntValue("Type");
		newConfig.URL = fileReader.GetStrValue("URL");

		switch (newConfig.Type)
		{
		case HeroType::Power:
			m_vecAvaliableStrengthHeroList.push_back(newConfig.HeroId);
			break;
		case HeroType::Intelligence:
			m_vecAvaliableManaHeroList.push_back(newConfig.HeroId);
			break;
		case HeroType::Agility:
			m_vecAvaliableAgilityHeroList.push_back(newConfig.HeroId);
			break;
		}

		if (m_HeorConfigMap.find(newConfig.HeroId) == m_HeorConfigMap.end())
			m_HeorConfigMap.insert(make_pair(newConfig.HeroId, std::move(newConfig)));
		else
			rfalse("文件%s 有重复ID", fileName.c_str());
	}
	return true;
}

bool CConfigManager::InitExchangeGoldConfig()
{
	string fileName = "ServerConfig\\DianJinShou.txt";
	CTabTableReader fileReader;
	if (fileReader.OpenTabTable(fileName) == false)
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	m_ExchangeGoldConfigList.clear();
	m_ExchangeGoldDiamondCostList.clear();
	while (fileReader.ReadLine())
	{
		ExchangeGoldConfig config;
		config.m_count = fileReader.GetIntValue("Count");
		config.m_useDiamondCount = fileReader.GetIntValue("Cash");
		config.m_getGoldCount = fileReader.GetIntValue("Gold");
		config.m_goldRandomMin = fileReader.GetIntValue("MinRandom");
		config.m_goldRandomMax = fileReader.GetIntValue("MaxRandom");
		config.m_critMin = fileReader.GetIntValue("MinCrit");
		config.m_critMax = fileReader.GetIntValue("MaxCrit");
		config.m_critOdds = fileReader.GetIntValue("CritOdd");

		m_ExchangeGoldDiamondCostList.insert(make_pair(config.m_useDiamondCount, config.m_count));

		if (m_ExchangeGoldConfigList.find(config.m_count) == m_ExchangeGoldConfigList.end())
			m_ExchangeGoldConfigList.insert(make_pair(config.m_count, move(config)));
		else
			rfalse("文件%s 有重复ID", fileName.c_str());
	}

	return true;
}

bool CConfigManager::InitEquipConfig()
{
	int32_t equipID = -1;; 
	string fileName = "ServerConfig\\ZhuangBei.txt";
	CTabTableReader fileReader;
	if (fileReader.OpenTabTable(fileName) == false)
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	while (fileReader.ReadLine())
	{
		EquipmentConfig config;
		config.m_EquipmentID = fileReader.GetIntValue("ID");
		config.m_EquipmentType = fileReader.GetIntValue("Type");
		config.Quality = fileReader.GetIntValue("Color");
		config.refiningExp = fileReader.GetIntValue("RefiningEXP");
		config.levelLimit = fileReader.GetIntValue("LevelLimit");
		config.sellPrice = fileReader.GetIntValue("Price");
		config.buyGoldPrice = fileReader.GetIntValue("Gold");
		config.buyDiamondPrice = fileReader.GetIntValue("Cash");
		config.BuySoulPointsCost = fileReader.GetIntValue("SoulPointsCost");
		config.ResolveSoulPoints = fileReader.GetIntValue("ResolveSoulPoints");
		config.buyHonorPrice = fileReader.GetIntValue("Honor");
		config.buyExploitPrice = fileReader.GetIntValue("Exploit");
		config.buyPrestigePrice = fileReader.GetIntValue("Prestige");
		config.buyTokenPrice = fileReader.GetIntValue("Munition");
		config.pieceID = fileReader.GetIntValue("EquipPieceID");

		config.baseAttribute.AddCure = fileReader.GetFloatValue("AddCure");
		config.baseAttribute.Agility = fileReader.GetIntValue("Agility");
		config.baseAttribute.Armor = fileReader.GetIntValue("Armor");
		config.baseAttribute.Physic = fileReader.GetIntValue("Physic");
		config.baseAttribute.BloodLevel = fileReader.GetIntValue("BloodLevel");
		config.baseAttribute.Crit = fileReader.GetIntValue("Crit");
		config.baseAttribute.Dodge = fileReader.GetIntValue("Dodge");
		config.baseAttribute.EnergyRecovery = fileReader.GetIntValue("EnergyRecovery");
		config.baseAttribute.HealthRecovery = fileReader.GetIntValue("HealthRecovery");
		config.baseAttribute.Intelligence = fileReader.GetIntValue("Intelligence");
		config.baseAttribute.Magic = fileReader.GetIntValue("Magic");
		config.baseAttribute.MaxHealth = fileReader.GetIntValue("MaxHealth");
		config.baseAttribute.OffsetArmor = fileReader.GetIntValue("OffsetArmor");
		config.baseAttribute.OffsetResistance = fileReader.GetIntValue("OffsetResistance");
		config.baseAttribute.Power = fileReader.GetIntValue("Power");
		config.baseAttribute.Resistance = fileReader.GetIntValue("Resistance");

		if (m_EquipmentMap.find(config.m_EquipmentID) == m_EquipmentMap.end())
			m_EquipmentMap.insert(make_pair(config.m_EquipmentID, std::move(config)));
		else
			rfalse("文件%s 有重复ID", fileName.c_str());
	}
	return true;
}

const  EquipmentConfig* CConfigManager::GetEquipment(int32_t equipmentID)
{
	return m_EquipmentMap.find(equipmentID) == m_EquipmentMap.end() ? nullptr : &m_EquipmentMap[equipmentID];
}

bool CConfigManager::InitHeroRankConfig()
{
	m_HeroRankConfigMap.clear();
	string fileName = "ServerConfig\\YingXiongJinJie.txt";
	CTabTableReader fileReader;
	if (fileReader.OpenTabTable(fileName) == false)
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	while (fileReader.ReadLine())
	{
		//===============读取武将进阶配置数据
		HeroRankData newRankData;
		int heroID = fileReader.GetIntValue("ID"); //读取武将ID
		int heroRank = fileReader.GetIntValue("Rank");
		newRankData.m_Agility = fileReader.GetFloatValue("AddAgility");
		newRankData.m_Intelligence = fileReader.GetFloatValue("AddIntelligence");
		newRankData.m_Power = fileReader.GetFloatValue("AddPower");

		newRankData.m_EquipIDs.push_back(fileReader.GetFloatValue("Equip1"));
		newRankData.m_EquipIDs.push_back(fileReader.GetFloatValue("Equip2"));
		newRankData.m_EquipIDs.push_back(fileReader.GetFloatValue("Equip3"));
		newRankData.m_EquipIDs.push_back(fileReader.GetFloatValue("Equip4"));
		newRankData.m_EquipIDs.push_back(fileReader.GetFloatValue("Equip5"));
		newRankData.m_EquipIDs.push_back(fileReader.GetFloatValue("Equip6"));
		//================获取武将进阶配置结构体
		auto findRestul = m_HeroRankConfigMap.find(heroID);
		if (findRestul == m_HeroRankConfigMap.end()) //如果该武将ID为新的，则容器加入新元素
		{
			m_HeroRankConfigMap.emplace(heroID, HeroRankConfig(heroRank, std::move(newRankData))); //新配武将配置
		}
		else
			findRestul->second.m_RankData.insert(make_pair(heroRank, move(newRankData)));
	}
	return true;
}

const HeroRankConfig* CConfigManager::GetHeroRankConfig(int32_t heroID)
{
	return m_HeroRankConfigMap.find(heroID) == m_HeroRankConfigMap.end() ?  nullptr : &m_HeroRankConfigMap[heroID];
}

int CConfigManager::GetHeroMaxRankGrade(int32_t heroID)
{
	auto findResult = m_HeroRankConfigMap.find(heroID);
	if (findResult == m_HeroRankConfigMap.end())
		return -1;

	if (findResult->second.m_RankData.size() == 0)
		return -1;

	auto endItor = findResult->second.m_RankData.end();
	--endItor;
	return endItor->first;
}

const HeroConfig* CConfigManager::GetHeroConfig(int32_t heroID)
{
	return m_HeorConfigMap.find(heroID) == m_HeorConfigMap.end() ?  nullptr :&m_HeorConfigMap[heroID];
}

const ItemConfig* CConfigManager::GetItemConfig(int32_t itemID)
{
	return m_ItemConfigMap.find(itemID) == m_ItemConfigMap.end() ? nullptr : &m_ItemConfigMap[itemID];
}

bool CConfigManager::_InitItemConfig()
{
	string fileName = "ServerConfig\\DaoJu.txt";
	CTabTableReader fileReader;
	if (fileReader.OpenTabTable(fileName) == false)
	{
		rfalse("can not find file %s ！！！！", fileName.c_str());
		return false;
	}

	while (fileReader.ReadLine())
	{
		ItemConfig config;
		config.ItemID = fileReader.GetIntValue("ID");
		config.Ability = fileReader.GetIntValue("Ability");
		config.ComposeItemID = fileReader.GetIntValue("ItemCompose");
		config.ComposeNum = fileReader.GetIntValue("ComposeNum");
		config.Type = fileReader.GetIntValue("Type");
		config.SellPrice = fileReader.GetIntValue("Price");
		config.BuyGoldPrice = fileReader.GetIntValue("Gold");
		config.ResolveSoulPoints = fileReader.GetIntValue("ResolveSoulPoints");
		config.BuySoulPointsCost = fileReader.GetIntValue("SoulPointsCost");
		config.BuyDiamondPrice = fileReader.GetIntValue("Cash");
		config.BuyHonorPrice = fileReader.GetIntValue("Honor");
		config.BuyExploitPrice = fileReader.GetIntValue("Exploit");
		config.BuyPrestigePrice = fileReader.GetIntValue("Prestige");
		config.BuyTokenPrice = fileReader.GetIntValue("Munition");
		config.RefiningExp = fileReader.GetIntValue("RefiningExp");
		config.Quality = fileReader.GetIntValue("Color");

		if (m_ItemConfigMap.find(config.ItemID) == m_ItemConfigMap.end())
			m_ItemConfigMap.insert(make_pair(config.ItemID, config));
		else
			rfalse("文件%s 有重复ID", fileName.c_str());
	}
	return true;
}

void CConfigManager::GetEquipIDList(vector<int32_t>& equipVec)
{
	equipVec.reserve(m_EquipmentMap.size());
	for each (auto var in m_EquipmentMap)
	{
		equipVec.push_back(var.first);
	}
}
const ExpeditionInstanceInfor* CConfigManager::GetSpecifyExpeditionInstanceConfig(int32_t masterLevel, int32_t instanceID)
{
	auto findResult = m_ExpeditionInstanceInfor.find(instanceID);
	if (findResult == m_ExpeditionInstanceInfor.end())
		return nullptr;

	if (findResult->second.size() <= 0)
		return nullptr;

	///找寻关卡ID对应的一系列的配置信息中君主开放等级大于等于masterLevel的第一个位置
	auto subFindResult = findResult->second.lower_bound(masterLevel);
	if (subFindResult == findResult->second.end())
	{
		--subFindResult;
		///如果传进来的君主等级比我配置文件里面配置的最大的等级还要大，则默认无法找到对应的关卡配置
		if (subFindResult->second.maxLevel < masterLevel)
			return nullptr;

		return &subFindResult->second;
	}
	else
	{
		///查看是否是第一个元素
		if (subFindResult == findResult->second.begin())
		{
			///如果传进来的君主等级比配置文件里面的配置的最小的等级还要小，则默认无法找到对应的关卡配置
			if (subFindResult->second.minLevel > masterLevel)
			{
				return nullptr;
			}

			return &subFindResult->second;
		}
		else
		{
			///如果当前配置的君主等级大于masterLevel，则前移一位
			if (subFindResult->second.minLevel > masterLevel)
				--subFindResult;

			if (subFindResult->second.minLevel <= masterLevel && subFindResult->second.maxLevel >= masterLevel)
				return &subFindResult->second;
		}
	}

	return nullptr;
}

int32_t CConfigManager::GetNextExpeditionInstanceID(int32_t instanceID)
{
	auto itor = m_ExpeditionInstanceInfor.find(instanceID);
	if (itor == m_ExpeditionInstanceInfor.end())
	{
		rfalse("远征关卡%d的详细配置获取不到", instanceID);
		return -1;
	}

	++itor;
	if (itor == m_ExpeditionInstanceInfor.end())
		return -1;

	return itor->first;
}

bool CConfigManager::InitHeroStarConfig()
{
	string fileName = "ServerConfig\\YingXiongShengXing.txt";
	CTabTableReader fileReader;
	if (fileReader.OpenTabTable(fileName) == false)
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	while (fileReader.ReadLine())
	{
		AttrGrowth tempAttrGrow;
		tempAttrGrow.AgilityGrowth = fileReader.GetFloatValue("AgilityGrowth");
		tempAttrGrow.IntelligenceGrowth = fileReader.GetFloatValue("IntelligenceGrowth");
		tempAttrGrow.PowerGrowth1 = fileReader.GetFloatValue("PowerGrowth");

		tempAttrGrow.Item = fileReader.GetIntValue("Item");
		tempAttrGrow.Number = fileReader.GetIntValue("Number");

		int curHeroId = fileReader.GetIntValue("ID");
		int starLevel = fileReader.GetIntValue("Star");

		if (m_HeroStarConfigMap.find(curHeroId) != m_HeroStarConfigMap.end())
		{
			if (!m_HeroStarConfigMap[curHeroId].AddStarAttrData(starLevel, move(tempAttrGrow)))
				rfalse("YingXiongShengXing配置中的英雄%d 有重复的星级配置", curHeroId);
		}
		else
		{
			m_HeroStarConfigMap.emplace(curHeroId, HeroStarConfig(curHeroId, starLevel, move(tempAttrGrow)));
		}
	}

	return true;
}

const HeroStarConfig* CConfigManager::GetHeroStarAttr(int32_t heroID)
{
	if (m_HeroStarConfigMap.find(heroID) == m_HeroStarConfigMap.end())
		return nullptr;

	return &m_HeroStarConfigMap[heroID];
}

int CConfigManager::GetHeroMaxStarLevel(int32_t heroID)
{
	auto findResult = m_HeroStarConfigMap.find(heroID);
	if (findResult == m_HeroStarConfigMap.end())
		return -1;

	return findResult->second.GetMaxStarLevel();
}

int32_t CConfigManager::GetFirstExpeditionInstanceID()
{
	if (m_ExpeditionInstanceInfor.size() == 0)
		return -1;

	return m_ExpeditionInstanceInfor.begin()->first;
}

bool CConfigManager::_InitShopCommodityConfig(const string& fileName, vector<CommodityConfig>& vecShopCommodity)
{
	CTabTableReader fileReader;
	if (fileReader.OpenTabTable(fileName) == false)
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	vecShopCommodity.reserve(fileReader.GetDataLineCount());
	string tempStr;
	while (fileReader.ReadLine())
	{
		CommodityConfig config;
		config.minLimitLevel = fileReader.GetIntValue("MinLevel");
		config.maxLimitLevel = fileReader.GetIntValue("MaxLevel");
		config.articleShelf = fileReader.GetIntValue("Index");
		config.minNumber = fileReader.GetIntValue("MinQuantity");
		config.maxNumber = fileReader.GetIntValue("MaxQuantity");

		tempStr = fileReader.GetStrValue("Item1");
		CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), config.vecItem1);
		config.percent1 = fileReader.GetIntValue("Weight1");

		tempStr = fileReader.GetStrValue("Item2");
		CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), config.vecItem2);
		config.percent2 = fileReader.GetIntValue("Weight2");

		tempStr = fileReader.GetStrValue("Item3");
		CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), config.vecItem3);
		config.percent3 = fileReader.GetIntValue("Weight3");

		tempStr = fileReader.GetStrValue("Item4");
		CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), config.vecItem4);
		config.percent4 = fileReader.GetIntValue("Weight4");

		tempStr = fileReader.GetStrValue("Item5");
		CTabTableReader::StringSplit(tempStr, m_SplitSign.data(), config.vecItem5);
		config.percent5 = fileReader.GetIntValue("Weight5");

		vecShopCommodity.push_back(move(config));
	}

	return true;
}

void CConfigManager::GetVarietyShopCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig)
{
	for (auto& config : m_vecVarietyShopCommodity)
	{
		if (config.InLevelRange(level))
			vecConfig.push_back(&config);
	}
}

void CConfigManager::GetArenaShopCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig)
{
	for (auto& config : m_vecArenaShopCommodity)
	{
		if (config.InLevelRange(level))
			vecConfig.push_back(&config);
	}
}

void CConfigManager::GetExpeditionShopCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig)
{
	for (auto& config : m_vecExpeditionShopCommodity)
	{
		if (config.InLevelRange(level))
			vecConfig.push_back(&config);
	}
}

void CConfigManager::GetMiracleMerchantCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig)
{
	for (auto& config : m_vecMiracleMerchantCommodity)
	{
		if (config.InLevelRange(level))
			vecConfig.push_back(&config);
	}
}

void CConfigManager::GetGemMerchantCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig)
{
	for (auto& config : m_vecGemMerchantCommodity)
	{
		if (config.InLevelRange(level))
			vecConfig.push_back(&config);
	}
}

void CConfigManager::GetLegionShopCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig)
{
	for (auto& config : m_vecLegionShopCommodity)
	{
		if (config.InLevelRange(level))
			vecConfig.push_back(&config);
	}
}

void CConfigManager::GetSoulExchangeCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig)
{
	for (auto& config : m_vecSoulExchangeCommodity)
	{
		if (config.InLevelRange(level))
			vecConfig.push_back(&config);
	}
}

void CConfigManager::GetWarOfLeagueShopCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig)
{
	for (auto& config : m_vecWarOfLeagueShopCommodity)
	{
		if (config.InLevelRange(level))
			vecConfig.push_back(&config);
	}
}

bool CConfigManager::InitMissionConfig()
{
	string fileName = "ServerConfig\\RenWu.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	m_MissionConfigList.clear();
	m_MissionTypeList.clear();
	while (filereader.ReadLine())
	{
		MissionConfig config;
		config.id = filereader.GetIntValue("ID");
		config.missionType = (MissionType)filereader.GetIntValue("Type");
		config.missionAvaliableLevel = filereader.GetIntValue("AvaliableLevel");
		config.needCompleteTime = filereader.GetIntValue("RequirementsNumber");
		config.rewardsExp = filereader.GetIntValue("Exp");
		config.rewardsGold = filereader.GetIntValue("Gold");
		config.rewardsItemID = filereader.GetIntValue("AwardItem");
		config.rewardsItemAmount = filereader.GetIntValue("Number");
		config.rewardsType = (MissionRewardsType)filereader.GetIntValue("RewardsType");
		config.activenessGains = filereader.GetIntValue("Vitality");

		auto findResult = m_MissionTypeList.find(config.missionType);
		if (findResult != m_MissionTypeList.end())
		{
			findResult->second.insert(config.id);
		}
		else
		{
			set<int> missionIDList;
			missionIDList.insert(config.id);
			m_MissionTypeList.insert(make_pair(config.missionType, std::move(missionIDList)));
		}

		if (m_MissionConfigList.find(config.id) == m_MissionConfigList.end())
			m_MissionConfigList.insert(make_pair(config.id, move(config)));
		else
			rfalse("文件%s 有重复ID", fileName.c_str());
	}

	return true;
}


bool CConfigManager::InitMissionActivenessRewardsConfig()
{
	///其实是任务活跃度奖励
	string fileName = "ServerConfig\\RenWuHuoYueDu.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	///开始读取配置文件中的信息到内存
	m_MissionActivenessRewardsConfig.clear();
	while (filereader.ReadLine())
	{
		MissionActivenessReswardsConfig config;
		vector<int> types;
		vector<int> IDs;
		vector<int> nums;
		config.activenessTiggerCondition = filereader.GetIntValue("Condition");
		string ItemStr = filereader.GetStrValue("RewardType");
		CTabTableReader::StringSplit(ItemStr, "|", types);
		ItemStr = filereader.GetStrValue("RewardID");
		CTabTableReader::StringSplit(ItemStr, "|", IDs);
		ItemStr = filereader.GetStrValue("RewardNum");
		CTabTableReader::StringSplit(ItemStr, "|", nums);

		if (types.size() != IDs.size() || types.size() != nums.size())
		{
			rfalse("读取RenWuHuoYueDu出错，配置的奖励个数出错");
			continue;
		}

		for (int i = 0; i < types.size(); ++i)
		{
			GoodsInfoSG info;
			info.itype = types[i];
			info.id = IDs[i];
			info.num = nums[i];
			config.rewardsInfor.push_back(std::move(info));
		}

		if (m_MissionActivenessRewardsConfig.find(config.activenessTiggerCondition) == m_MissionActivenessRewardsConfig.end())
			m_MissionActivenessRewardsConfig.insert(make_pair(config.activenessTiggerCondition, move(config)));
		else
			rfalse("文件%s 有重复ID", fileName.c_str());
	}

	return true;
}


const MissionConfig* CConfigManager::GetMissionConfig(int missionID) const
{
	auto findResult = m_MissionConfigList.find(missionID);
	if (findResult != m_MissionConfigList.end())
		return &findResult->second;

	return nullptr;
}

const set<int>* CConfigManager::GetMissionListByType(MissionType missionType) const
{
	auto findResult = m_MissionTypeList.find(missionType);
	if (findResult != m_MissionTypeList.end())
		return &findResult->second;

	return nullptr;
}

const MissionActivenessReswardsConfig* CConfigManager::GetMissionActivenessRewardsConfig(int activenessTrigger) const
{
	auto findResult = m_MissionActivenessRewardsConfig.find(activenessTrigger);
	if (findResult != m_MissionActivenessRewardsConfig.end())
		return &findResult->second;

	return nullptr;
}

bool CConfigManager::IsMissionActivenessRestricted(int activeness)
{
	auto findResult = m_MissionActivenessRewardsConfig.lower_bound(activeness);
	if (findResult == m_MissionActivenessRewardsConfig.end())
		return true;

	return false;
}

const ExchangeGoldConfig* CConfigManager::GetExchangeGoldConfig(int boughtCount) const
{
	auto findResult = m_ExchangeGoldConfigList.find(boughtCount);
	if (findResult == m_ExchangeGoldConfigList.end())
		return nullptr;

	return &(findResult->second);
}

bool CConfigManager::GetExchangeGoldConfigListByCost(DWORD curCost, OUT set<int>& configList) const
{
	auto findResult = m_ExchangeGoldDiamondCostList.equal_range(curCost);
	if (findResult.first == findResult.second)
		return false;

	for (auto iter = findResult.first; iter != findResult.second; ++iter)
	{
		configList.insert(iter->second);
	}

	return true;
}

void CConfigManager::GetMissionIDList(OUT vector<int>& IDlist) const
{
	IDlist.reserve(m_MissionConfigList.size());
	for (auto & itor : m_MissionConfigList)
	{
		IDlist.push_back(itor.first);
	}
}

const AchievementConfig* CConfigManager::GetAchievementConfig(int32_t achievementID)const
{
	auto findResult = m_AchievementConfigList.find(achievementID);
	if (findResult != m_AchievementConfigList.end())
		return &findResult->second;

	return nullptr;
}

const map<int32_t, vector<int32_t>>& CConfigManager::GetHeroRankUpgradeAchievementIDList() const
{
	return m_HeroRankUpgradeAchievementIDList;
}

bool CConfigManager::GetNextAchievementID(int32_t acheivementID, OUT int32_t& nextAchievementID) const
{
	auto findResult = m_AchievementConfigList.find(acheivementID);
	if (findResult == m_AchievementConfigList.end())
		return false;

	nextAchievementID = -1;
	AchievementType achievementType = (AchievementType)findResult->second.achieveType;
	++findResult;

	///此时也代表成功获取到了下一个成就ID，只不过成就ID为-1
	if (findResult == m_AchievementConfigList.end())
		return true;

	///此时也代表成功获取到了下一个成就ID，只不过成就ID为-1
	if (findResult->second.achieveType != achievementType)
		return true;

	nextAchievementID = findResult->first;

	return true;
}

bool CConfigManager::InitAchievementConfig()
{
	string fileName = "ServerConfig\\ChengJiu.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	m_AchievementConfigList.clear();
	m_AchievementGroupIDList.clear();
	m_DuplicateAchievementTollgateIDList.clear();
	m_HeroRankUpgradeAchievementIDList.clear();
	while (filereader.ReadLine())
	{
		AchievementConfig tempAchievementConfig;

		tempAchievementConfig.id = filereader.GetIntValue("ID");
		tempAchievementConfig.achieveType = filereader.GetIntValue("Type");
		tempAchievementConfig.param1 = filereader.GetIntValue("Value1");
		tempAchievementConfig.param2 = filereader.GetIntValue("Value2");
		tempAchievementConfig.rewardsType = filereader.GetIntValue("AwardType");
		tempAchievementConfig.rewardID = filereader.GetIntValue("AwardID");
		tempAchievementConfig.rewardAmount = filereader.GetIntValue("AwardNum");

		///记录武将进阶成就的品阶对应的成就ID列表,严格按照策划配置的文档是有序的来的
		if (tempAchievementConfig.achieveType == AchievementType::Advanced)
		{
			auto findResult = m_HeroRankUpgradeAchievementIDList.find(tempAchievementConfig.param1);
			if (findResult != m_HeroRankUpgradeAchievementIDList.end())
			{
				findResult->second.push_back(tempAchievementConfig.id);
			}
			else
			{
				vector<int32_t> newIDList;
				newIDList.push_back(tempAchievementConfig.id);
				m_HeroRankUpgradeAchievementIDList.insert(make_pair(tempAchievementConfig.param1, move(newIDList)));
			}
		}
		else if (tempAchievementConfig.achieveType <= AchievementType::Battle4) ///<暂时这样危险的区别副本的相关成就
		{
			auto findResult = m_DuplicateAchievementTollgateIDList.find(tempAchievementConfig.achieveType);
			if (findResult != m_DuplicateAchievementTollgateIDList.end())
			{
				findResult->second.insert(make_pair(tempAchievementConfig.param1, tempAchievementConfig.id));
			}
			else
			{
				map<int32_t, int32_t> newIDList;
				newIDList.insert(make_pair(tempAchievementConfig.param1, tempAchievementConfig.id));
				m_DuplicateAchievementTollgateIDList.insert(make_pair(tempAchievementConfig.achieveType, move(newIDList)));
			}
		}

		auto findResult = m_AchievementGroupIDList.find(tempAchievementConfig.achieveType);
		if (findResult == m_AchievementGroupIDList.end())
		{
			set<int32_t> newIDList;
			newIDList.insert(tempAchievementConfig.id);
			m_AchievementGroupIDList.insert(make_pair(tempAchievementConfig.achieveType, move(newIDList)));
		}
		else
		{
			findResult->second.insert(tempAchievementConfig.id);
		}

		if (m_AchievementConfigList.find(tempAchievementConfig.id) == m_AchievementConfigList.end())
			m_AchievementConfigList.insert(make_pair(tempAchievementConfig.id, move(tempAchievementConfig)));
		else
			rfalse("文件%s 有重复ID", fileName.c_str());
	}

	return true;
}

bool CConfigManager::InitRandomAchievementConfig()
{
	string fileName = "ServerConfig\\SuiJiChengJiu.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	m_RandomAchievementConfigList.clear();
	m_RandomAchievementAccessibleLevel.clear();
	while (filereader.ReadLine())
	{
		RandomAchievementConfig tempAchievementConfig;

		tempAchievementConfig.id = filereader.GetIntValue("ID");
		tempAchievementConfig.accessibleLevel = filereader.GetIntValue("AccessibleLevel");
		tempAchievementConfig.achieveType = filereader.GetIntValue("Type");
		tempAchievementConfig.param1 = filereader.GetIntValue("Value1");
		tempAchievementConfig.param2 = filereader.GetIntValue("Value2");
		tempAchievementConfig.rewardsType = filereader.GetIntValue("AwardType");
		tempAchievementConfig.rewardID = filereader.GetIntValue("AwardID");
		tempAchievementConfig.rewardAmount = filereader.GetIntValue("AwardNum");

		auto findResult = m_RandomAchievementAccessibleLevel.find(tempAchievementConfig.accessibleLevel);
		if (findResult == m_RandomAchievementAccessibleLevel.end())
		{
			unordered_set<int32_t> newIDList;
			newIDList.insert(tempAchievementConfig.id);
			m_RandomAchievementAccessibleLevel.insert(make_pair(tempAchievementConfig.accessibleLevel, move(newIDList)));
		}
		else
			findResult->second.insert(tempAchievementConfig.id);

		if (m_RandomAchievementConfigList.find(tempAchievementConfig.id) == m_RandomAchievementConfigList.end())
			m_RandomAchievementConfigList.insert(make_pair(tempAchievementConfig.id, move(tempAchievementConfig)));
		else
			rfalse("文件%s 有重复ID %d", fileName.c_str(), tempAchievementConfig.id);
	}

	return true;
}

void CConfigManager::GetAchievementIDList(OUT vector<int32_t>& IDList)
{
	IDList.reserve(m_AchievementConfigList.size());
	for (auto & itor : m_AchievementConfigList)
	{
		IDList.push_back(itor.first);
	}
}

int CConfigManager::GetAchivementGroupInitID(int32_t group)
{
	auto findResult = m_AchievementGroupIDList.find(group);
	if (findResult != m_AchievementGroupIDList.end())
		return *(findResult->second.begin());

	return -1;
}

int CConfigManager::IsTollgateIDExistInSpecificAchievementGroup(int32_t group, int32_t tollgateID)
{
	auto findResult = m_DuplicateAchievementTollgateIDList.find(group);
	if (findResult == m_DuplicateAchievementTollgateIDList.end())
		return -1;

	auto findResults = findResult->second.find(tollgateID);
	if (findResults == findResult->second.end())
		return -1;

	return findResults->second;
}

int CConfigManager::GetAchievementGroupType(int32_t achievementID)
{
	auto findResult = m_AchievementConfigList.find(achievementID);
	if (findResult != m_AchievementConfigList.end())
	{
		return findResult->second.achieveType;
	}

	return -1;
}

const RandomAchievementConfig* CConfigManager::GetRandomAchievementConfig(int32_t achievementID) const
{
	auto findResult = m_RandomAchievementConfigList.find(achievementID);
	if (findResult != m_RandomAchievementConfigList.end())
		return &findResult->second;

	return nullptr;
}

bool CConfigManager::GetAccessibleAchievementIDS(DWORD level, OUT unordered_map<int32_t, int32_t>& IDList)
{
	auto findResult = m_RandomAchievementAccessibleLevel.lower_bound(level);
	if (findResult == m_RandomAchievementAccessibleLevel.end())
	{
		///全部成就都开放
		int index = 0;
		for (auto iter : m_RandomAchievementAccessibleLevel)
		{
			for (auto subIter : iter.second)
			{
				IDList.insert(make_pair(index, subIter));
				++index;
			}
		}
	}
	else
	{
		if (findResult->first == level)
		{
			///迭代器往后移一位
			++findResult;
		}
		else if (findResult->first > level && findResult == m_RandomAchievementAccessibleLevel.begin())
			return false;///代表当前的等级完全没有开放任何随机生成的可达成的成就
		
		int index = 0;
		for (auto iter = m_RandomAchievementAccessibleLevel.begin(); iter != findResult; ++iter)
		{
			for (auto subIter : iter->second)
			{
				IDList.insert(make_pair(index, subIter));
				++index;
			}
		}
	}

	return true;
}

bool CConfigManager::InitMonsterConfig()
{
	string fileName = "ServerConfig\\GuaiWu.txt";
	CTabTableReader fileReader;
	if (!fileReader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	m_MonsterConfigs.clear();
	while (fileReader.ReadLine())
	{
		MonsterConfig monsterConfig;

		monsterConfig.monsterID = fileReader.GetIntValue("ID");
		monsterConfig.baseAttribute.AddCure = fileReader.GetFloatValue("AddCure");
		monsterConfig.baseAttribute.Agility = fileReader.GetFloatValue("Agility");
		monsterConfig.baseAttribute.Armor = fileReader.GetFloatValue("Armor");
		monsterConfig.baseAttribute.Physic = fileReader.GetFloatValue("Physic");
		monsterConfig.baseAttribute.BloodLevel = fileReader.GetFloatValue("BloodLevel");
		monsterConfig.baseAttribute.Crit = fileReader.GetFloatValue("Crit");
		monsterConfig.baseAttribute.Dodge = fileReader.GetFloatValue("Dodge");
		monsterConfig.baseAttribute.EnergyRecovery = fileReader.GetFloatValue("EnergyRecovery");
		monsterConfig.baseAttribute.HealthRecovery = fileReader.GetFloatValue("HealthRecovery");
		monsterConfig.baseAttribute.Intelligence = fileReader.GetFloatValue("Intelligence");
		monsterConfig.baseAttribute.Magic = fileReader.GetFloatValue("Magic");
		monsterConfig.baseAttribute.MaxHealth = fileReader.GetFloatValue("MaxHealth");
		monsterConfig.baseAttribute.OffsetArmor = fileReader.GetFloatValue("OffsetArmor");
		monsterConfig.baseAttribute.OffsetResistance = fileReader.GetFloatValue("OffsetResistance");
		monsterConfig.baseAttribute.Power = fileReader.GetFloatValue("Power");
		monsterConfig.baseAttribute.Resistance = fileReader.GetFloatValue("Resistance");
		monsterConfig.attackRange = fileReader.GetFloatValue("AttackRange");
		monsterConfig.attackCD = fileReader.GetFloatValue("AttackCD");

		if (m_MonsterConfigs.find(monsterConfig.monsterID) == m_MonsterConfigs.end())
		{
			m_MonsterConfigs.insert(make_pair(monsterConfig.monsterID, std::move(monsterConfig)));
		}
		else
			rfalse("文件%s 有重复ID", fileName.c_str());
	}
	return true;
}

const MonsterConfig* CConfigManager::GetMonsterAttrData(int32_t monsterID)
{
	auto findResult = m_MonsterConfigs.find(monsterID);
	if (findResult != m_MonsterConfigs.end())
	{
		return &(findResult->second);
	}

	return nullptr;
}

bool CConfigManager::GetMonsterBaseData(int32_t monsterID, OUT MonsterData& outputData)
{
	auto findResult = m_MonsterConfigs.find(monsterID);
	if (findResult == m_MonsterConfigs.end())
		return false;

	///开始序列化怪物的基本属性
	outputData.monsterID = monsterID;
	outputData.attackCD = findResult->second.attackCD;
	outputData.attackRange = findResult->second.attackRange;
	outputData.AddCure = findResult->second.baseAttribute.AddCure;
	outputData.Agility = findResult->second.baseAttribute.Agility;
	outputData.Armor = findResult->second.baseAttribute.Armor;
	outputData.BloodLevel = findResult->second.baseAttribute.BloodLevel;
	outputData.Crit = findResult->second.baseAttribute.Crit;
	outputData.Dodge = findResult->second.baseAttribute.Dodge;
	outputData.EnergyRecovery = findResult->second.baseAttribute.EnergyRecovery;
	outputData.HealthRecovery = findResult->second.baseAttribute.HealthRecovery;
	outputData.Intelligence = findResult->second.baseAttribute.Intelligence;
	outputData.Magic = findResult->second.baseAttribute.Magic;
	outputData.MaxHealth = findResult->second.baseAttribute.MaxHealth;
	outputData.OffsetArmor = findResult->second.baseAttribute.OffsetArmor;
	outputData.OffsetResistance = findResult->second.baseAttribute.OffsetResistance;
	outputData.Physic = findResult->second.baseAttribute.Physic;
	outputData.Power = findResult->second.baseAttribute.Power;
	outputData.Resistance = findResult->second.baseAttribute.Resistance;

	return true;
}

bool CConfigManager::InitChapterConfig()
{
	string fileName = "ServerConfig\\ZhangJie.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	m_ChapterConfigList.clear();
	m_DuplicateTypeIDList.clear();
	while (filereader.ReadLine())
	{
		ChapterConfig tempChapter;

		tempChapter.chapterID = filereader.GetIntValue("ID");
		tempChapter.ticketNum = filereader.GetIntValue("Tickets");
		tempChapter.chapterType = filereader.GetIntValue("Type");

		string ItemStr = filereader.GetStrValue("Battle");
		tempChapter.battleIdList.clear();
		CTabTableReader::StringSplit(ItemStr, "|", tempChapter.battleIdList);

		auto findResult = m_DuplicateTypeIDList.find(tempChapter.chapterType);
		if (findResult == m_DuplicateTypeIDList.end())
		{
			set<int> newDuplicateList;
			newDuplicateList.insert(tempChapter.chapterID);
			m_DuplicateTypeIDList.insert(make_pair(tempChapter.chapterType, move(newDuplicateList)));
		}
		else
			findResult->second.insert(tempChapter.chapterID);

		if (m_ChapterConfigList.find(tempChapter.chapterID) == m_ChapterConfigList.end())
			m_ChapterConfigList.insert(make_pair(tempChapter.chapterID, move(tempChapter)));
		else
			rfalse("文件%s 有重复ID", fileName.c_str());
	}

	return true;
}

bool CConfigManager::InitChapterBattleScoreRewardsConfig()
{
	string fileName = "ServerConfig\\ZhangJieJiangLi.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	///开始读取配置文件
	m_ChapterBattleScoreRewardsConfig.clear();
	while (filereader.ReadLine())
	{
		ChapterBattleScoreRewardsConfig config;

		config.chapterID = filereader.GetIntValue("chapterID");
		_ReadChapterBattleScoreRewardsConfigModule(filereader, "StageOne", config.rewardsInfor);
		_ReadChapterBattleScoreRewardsConfigModule(filereader, "StageTwo", config.rewardsInfor);
		_ReadChapterBattleScoreRewardsConfigModule(filereader, "StageThree", config.rewardsInfor);

		auto findReults = m_ChapterBattleScoreRewardsConfig.find(config.chapterID);
		if (findReults == m_ChapterBattleScoreRewardsConfig.end())
		{
			m_ChapterBattleScoreRewardsConfig.insert(make_pair(config.chapterID, std::move(config)));
		}
		else
			rfalse("文件%s 有重复ID", fileName.c_str());
	}

	return true;
}

void CConfigManager::_ReadChapterBattleScoreRewardsConfigModule(CTabTableReader& fileReader, string baseAttrTag, unordered_map<int, vector<GoodsInfoSG>>& rewardsInfor)
{
	vector<int> types;
	vector<int> IDs;
	vector<int> nums;
	int iStage = fileReader.GetIntValue(baseAttrTag);
	string RewardTypeAttrName = "RewardsType";
	string RewardIDAttrName = "RewardsID";
	string RewardNumAttrName = "RewardsNum";

	string ItemStr = fileReader.GetStrValue(RewardTypeAttrName.insert(0, baseAttrTag.c_str()));
	CTabTableReader::StringSplit(ItemStr, "|", types);
	ItemStr = fileReader.GetStrValue(RewardIDAttrName.insert(0, baseAttrTag.c_str()));
	CTabTableReader::StringSplit(ItemStr, "|", IDs);
	ItemStr = fileReader.GetStrValue(RewardNumAttrName.insert(0, baseAttrTag.c_str()));
	CTabTableReader::StringSplit(ItemStr, "|", nums);

	if (types.size() != IDs.size() || types.size() != nums.size())
	{
		rfalse("读取ZhangJieJiangLi出错，配置的奖励个数出错");
		return;
	}

	vector<GoodsInfoSG> goodsInfors;
	for (int i = 0; i < types.size(); ++i)
	{
		GoodsInfoSG info;
		info.itype = types[i];
		info.id = IDs[i];
		info.num = nums[i];
		goodsInfors.push_back(std::move(info));
	}

	auto findResult = rewardsInfor.find(iStage);
	if (findResult == rewardsInfor.end())
		rewardsInfor.insert(make_pair(iStage, std::move(goodsInfors)));
	else
		rfalse("ZhangJieJiangLi表配置的星级个数有重复");
}

const ChapterConfig* CConfigManager::GetChapterConfig(DWORD chapterID)
{
	auto findResult = m_ChapterConfigList.find(chapterID);
	if (findResult != m_ChapterConfigList.end())
		return &findResult->second;

	return nullptr;
}

const ChapterBattleScoreRewardsConfig* CConfigManager::GetChapterBattleScoreRewardsConfig(DWORD chapterID)
{
	auto findResult = m_ChapterBattleScoreRewardsConfig.find(chapterID);
	if (findResult != m_ChapterBattleScoreRewardsConfig.end())
		return &findResult->second;

	return nullptr;
}

int CConfigManager::GetFirstDuplicateID(InstanceType type)
{
	auto findResult = m_DuplicateTypeIDList.find(type);
	if (findResult == m_DuplicateTypeIDList.end())
	{
		rfalse("获取副本类型%d的第一个副本ID出错", type);
		return -1;
	}

	return *findResult->second.begin();
}

int CConfigManager::GetEndDuplicateID(InstanceType type)
{
	auto findResult = m_DuplicateTypeIDList.find(type);
	if (findResult == m_DuplicateTypeIDList.end())
	{
		rfalse("获取副本类型%d的第一个副本ID出错", type);
		return -1;
	}

	return *(--findResult->second.end());
}

const set<int>* CConfigManager::GetDuplicateListByType(InstanceType type)
{
	auto findResult = m_DuplicateTypeIDList.find(type);
	if (findResult == m_DuplicateTypeIDList.end())
	{
		rfalse("获取副本类型%d的所有副本ID出错", type);
		return nullptr;
	}

	return &findResult->second;
}

int CConfigManager::GetFirstTollgateID(InstanceType type)
{
	int firstDuplicateID = GetFirstDuplicateID(type);
	if (firstDuplicateID == -1)
		return -1;

	auto findResult = m_ChapterConfigList.find(firstDuplicateID);
	if (findResult == m_ChapterConfigList.end())
	{
		rfalse("获取副本类型%d的第一个关卡ID出错", type);
		return -1;
	}

	///返回本副本的第一个关卡的ID名
	if (findResult->second.battleIdList.size() > 0)
		return *findResult->second.battleIdList.begin();

	rfalse("获取副本类型%d的第一个关卡ID出错", type);
	return -1;
}

int CConfigManager::GetEndTollgateID(InstanceType type)
{
	int endDuplicateID = GetEndDuplicateID(type);
	if (endDuplicateID == -1)
		return -1;

	auto findResult = m_ChapterConfigList.find(endDuplicateID);
	if (findResult == m_ChapterConfigList.end())
	{
		rfalse("获取副本类型%d的第一个关卡ID出错", type);
		return -1;
	}

	///返回本副本的第一个关卡的ID名
	if (findResult->second.battleIdList.size() > 0)
		return *(--findResult->second.battleIdList.end());

	rfalse("获取副本类型%d的第一个关卡ID出错", type);
	return -1;
}

bool CConfigManager::InitBattleLevelConfig()
{
	string fileName = "ServerConfig\\GuanQia.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	m_BattleLevelConfigList.clear();
	while (filereader.ReadLine())
	{
		BattleLevelConfig tempBattleConfig;

		tempBattleConfig.ID = filereader.GetIntValue("ID");
		tempBattleConfig.LevelLimit = filereader.GetIntValue("LevelLimit");
		tempBattleConfig.Exp = filereader.GetIntValue("Exp");
		tempBattleConfig.TicketNum = filereader.GetIntValue("Tickets");
		tempBattleConfig.Cost = filereader.GetIntValue("Cost");

		char monsterID[64];
		int tempIntVal = 0;
		for (int i = 0; i < 14; ++i)
		{
			sprintf_s(monsterID, "monster%d", i + 1);
			string info = filereader.GetStrValue(monsterID);
			if (info.empty() || info.compare("") == 0)
				continue;

			tempIntVal = atoi(info.c_str());
			tempBattleConfig.LevelMonsters.insert(tempIntVal);
		}

		string str = filereader.GetStrValue("monster15");
		if (str.find("*") != str.npos)
		{
			string::size_type position = str.find_first_of("*");
			if (position != str.npos)
			{
				tempIntVal = atoi(str.substr(0, position).c_str());
				tempBattleConfig.LevelMonsters.insert(tempIntVal);
			}
			else
				rfalse("关卡 %d BOSS ID获取不到", tempBattleConfig.ID);
		}
		else
		{
			tempIntVal = atoi(str.c_str());
			tempBattleConfig.LevelMonsters.insert(tempIntVal);
		}

		if (m_BattleLevelConfigList.find(tempBattleConfig.ID) == m_BattleLevelConfigList.end())
			m_BattleLevelConfigList.insert(make_pair(tempBattleConfig.ID, move(tempBattleConfig)));
		else
			rfalse("文件%s 有重复ID", fileName.c_str());
	}

	return true;
}

const BattleLevelConfig* CConfigManager::GetBattleLevelConfig(DWORD levelID)
{
	auto findResult = m_BattleLevelConfigList.find(levelID);
	if (findResult != m_BattleLevelConfigList.end())
		return &findResult->second;

	return nullptr;
}

bool CConfigManager::InitBattleFristDropConfig()
{
	string fileName = "ServerConfig\\DiYiCiGuanQiaDiaoLuo.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	m_BattleFirstDropConfigList.clear();
	while (filereader.ReadLine())
	{
		BattleFirstDropConfig drop;

		drop.ID = filereader.GetIntValue("ID");
		drop.Money = filereader.GetIntValue("Money");
		drop.BossItem1 = filereader.GetIntValue("Item1");
		drop.DropNum1 = filereader.GetIntValue("Number1");
		drop.BossItem2 = filereader.GetIntValue("Item2");
		drop.DropNum2 = filereader.GetIntValue("Number2");

		if (drop.ID != -1)
		{
			if (m_BattleFirstDropConfigList.find(drop.ID) == m_BattleFirstDropConfigList.end())
				m_BattleFirstDropConfigList.insert(make_pair(drop.ID, move(drop)));
			else
				rfalse("文件%s 有重复ID", fileName.c_str());
		}
	}

	return true;
}

const BattleFirstDropConfig* CConfigManager::GetBattleFirstDropConfig(DWORD levelID)
{
	auto findResult = m_BattleFirstDropConfigList.find(levelID);
	if (findResult != m_BattleFirstDropConfigList.end())
		return &findResult->second;

	return nullptr;
}

bool CConfigManager::InitBattleDropConfig()
{
	string fileName = "ServerConfig\\GuanQiaDiaoLuo.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	m_BattleDropConfigList.clear();
	while (filereader.ReadLine())
	{
		BattleDropConfig drop;

		drop.ID = filereader.GetIntValue("ID");
		drop.Money = filereader.GetIntValue("Money");
		drop.MinNumber = filereader.GetIntValue("MinNumber");
		drop.MaxNumber = filereader.GetIntValue("MaxNumber");
		drop.WhiteItemWeight = filereader.GetIntValue("WhiteItemWeight");
		drop.GreenItemWeight = filereader.GetIntValue("GreenItemWeight");
		drop.BlueItemWeight = filereader.GetIntValue("BlueItemWeight");
		drop.PurpleItemWeight = filereader.GetIntValue("PurpleItemWeight");

		string ItemStr = filereader.GetStrValue("WhiteItem");
		drop.WhiteItemList.clear();
		CTabTableReader::StringSplit(ItemStr, "|", drop.WhiteItemList);

		ItemStr = filereader.GetStrValue("GreenItem");
		drop.GreenItemList.clear();
		CTabTableReader::StringSplit(ItemStr, "|", drop.GreenItemList);

		ItemStr = filereader.GetStrValue("BlueItem");
		drop.BlueItemList.clear();
		CTabTableReader::StringSplit(ItemStr, "|", drop.BlueItemList);

		ItemStr = filereader.GetStrValue("PurpleItem");
		drop.PurpleItemList.clear();
		CTabTableReader::StringSplit(ItemStr, "|", drop.PurpleItemList);

		ItemStr = filereader.GetStrValue("PropID");
		drop.PropID.clear();
		CTabTableReader::StringSplit(ItemStr, "|", drop.PropID);

		ItemStr = filereader.GetStrValue("PropNumber");
		drop.PropNumber.clear();
		CTabTableReader::StringSplit(ItemStr, "|", drop.PropNumber);

		drop.BossItem1 = filereader.GetIntValue("Item1");
		drop.BossItemNumber1 = filereader.GetIntValue("Number1");
		drop.BossItemDropPercent1 = filereader.GetIntValue("Percent1");

		drop.BossItem2 = filereader.GetIntValue("Item2");
		drop.BossItemNumber2 = filereader.GetIntValue("Number2");
		drop.BossItemDropPercent2 = filereader.GetIntValue("Percent2");

		drop.BossItem3 = filereader.GetIntValue("Item3");
		drop.BossItemNumber3 = filereader.GetIntValue("Number3");
		drop.BossItemDropPercent3 = filereader.GetIntValue("Percent3");

		if (m_BattleDropConfigList.find(drop.ID) == m_BattleDropConfigList.end())
			m_BattleDropConfigList.insert(make_pair(drop.ID, move(drop)));
		else
			rfalse("文件%s 有重复ID", fileName.c_str());
	}

	return true;
}

const BattleDropConfig * CConfigManager::GetBattleDropConfig(DWORD levelID)
{
	auto findResult = m_BattleDropConfigList.find(levelID);
	if (findResult != m_BattleDropConfigList.end())
		return &findResult->second;

	return nullptr;
}

bool CConfigManager::InitMasterLevelInfor()
{
	string fileName = "ServerConfig\\ZhuJueDengJi.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	m_MasterLevelInforList.clear();
	m_MasterLevelInforList.reserve(filereader.GetDataLineCount());
	m_MasterLevelUpExpTable.clear();
	DWORD curLevel = 0;
	while (filereader.ReadLine())
	{
		MasterLevelInfor infor;
		curLevel = filereader.GetIntValue("Level");
		infor.LevelNeededExp = filereader.GetIntValue("Exp");
		infor.CharactorLevelLimit = filereader.GetIntValue("RoleMaxLevel");
		infor.CharactorEnduranceLimit = filereader.GetIntValue("ActionLimited");
		infor.CharactorActionRecover = filereader.GetIntValue("ActionRecover");

		if (m_MasterLevelUpExpTable.find(infor.LevelNeededExp) == m_MasterLevelUpExpTable.end())
			m_MasterLevelUpExpTable.insert(make_pair(infor.LevelNeededExp, curLevel));
		else
			rfalse("经验配置文件%s 有重复的经验值配置", fileName.c_str());

		m_MasterLevelInforList.push_back(move(infor));
	}

	return true;
}

const MasterLevelInfor* CConfigManager::GetMasterLevelInfor(DWORD masterLevel)
{
	if (masterLevel > m_MasterLevelInforList.size())
		return nullptr;

	return &m_MasterLevelInforList[masterLevel - 1];
}

bool CConfigManager::InitHeroTrainingCfg()
{
	string fileName = "ServerConfig\\WuJiangChanChuXunLian.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	int curLevel = 0;
	float bonusScale = 0;
	m_iHeroTrainingCfgNums = filereader.GetDataLineCount();
	if (m_arrayHeroTrainingCfg != nullptr)
	{
		delete[] m_arrayHeroTrainingCfg;
		m_arrayHeroTrainingCfg = nullptr;
	}
	m_arrayHeroTrainingCfg = new HeroTrainingCfg[m_iHeroTrainingCfgNums];
	while (filereader.ReadLine())
	{
		bonusScale = 0;
		curLevel = filereader.GetIntValue("Level") - 1;
		if (curLevel < 0 || curLevel >= m_iHeroTrainingCfgNums)
			continue;

		m_arrayHeroTrainingCfg[curLevel].masterLevel = curLevel + 1;
		m_arrayHeroTrainingCfg[curLevel].costOfSpeedUp = filereader.GetFloatValue("EndConsume");
		m_arrayHeroTrainingCfg[curLevel].expProceeds = filereader.GetFloatValue("Experience");
		m_arrayHeroTrainingCfg[curLevel].gainsAndCost.clear();
		HeroTrainingSpeedUpInfor speedUpInfor;
		speedUpInfor.trainingType = HeroTrainingType::Normal;
		speedUpInfor.bonusScale = filereader.GetFloatValue("PlusNormal");
		speedUpInfor.consumeType = GoodsType::money;
		speedUpInfor.payment = filereader.GetIntValue("NormalConsume");
		if (m_arrayHeroTrainingCfg[curLevel].gainsAndCost.find(speedUpInfor.trainingType) == m_arrayHeroTrainingCfg[curLevel].gainsAndCost.end())
		{
			m_arrayHeroTrainingCfg[curLevel].gainsAndCost.insert(make_pair(speedUpInfor.trainingType, speedUpInfor));
		}

		speedUpInfor.bonusScale = filereader.GetFloatValue("PlusEnhance");
		speedUpInfor.trainingType = HeroTrainingType::Fast;
		speedUpInfor.consumeType = GoodsType::money;
		speedUpInfor.payment = filereader.GetIntValue("EnhanceConsume");
		if (m_arrayHeroTrainingCfg[curLevel].gainsAndCost.find(speedUpInfor.trainingType) == m_arrayHeroTrainingCfg[curLevel].gainsAndCost.end())
		{
			m_arrayHeroTrainingCfg[curLevel].gainsAndCost.insert(make_pair(speedUpInfor.trainingType, speedUpInfor));
		}

		speedUpInfor.bonusScale = filereader.GetFloatValue("PlusLimit");
		speedUpInfor.trainingType = HeroTrainingType::Extreme;
		speedUpInfor.consumeType = GoodsType::diamond;
		speedUpInfor.payment = filereader.GetIntValue("LimitConsume");
		if (m_arrayHeroTrainingCfg[curLevel].gainsAndCost.find(speedUpInfor.trainingType) == m_arrayHeroTrainingCfg[curLevel].gainsAndCost.end())
		{
			m_arrayHeroTrainingCfg[curLevel].gainsAndCost.insert(make_pair(speedUpInfor.trainingType, speedUpInfor));
		}
	}
	return true;
}

const HeroTrainingCfg* CConfigManager::GetHeroTrainingCfg(int masterLevel)
{
	if (m_arrayHeroTrainingCfg == nullptr || masterLevel < 1 || masterLevel > m_iHeroTrainingCfgNums)
		return nullptr;

	return &m_arrayHeroTrainingCfg[masterLevel - 1];
}


bool CConfigManager::InitHeroHuntingHuntingCfg()
{
	string fileName = "ServerConfig\\WuJiangChanChuXunBao.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	int curLevel = 0;
	m_iHeroHungingHuntingNums = filereader.GetDataLineCount();
	if (m_arrayHeroHungingHuntingCfg != nullptr)
	{
		delete[] m_arrayHeroHungingHuntingCfg;
		m_arrayHeroHungingHuntingCfg = nullptr;
	}

	m_arrayHeroHungingHuntingCfg = new HeroHungingHuntingCfg[m_iHeroHungingHuntingNums];
	while (filereader.ReadLine())
	{
		curLevel = filereader.GetIntValue("Level") - 1;
		if (curLevel < 0 || curLevel >= m_iHeroTrainingCfgNums)
			continue;

		m_arrayHeroHungingHuntingCfg[curLevel].masterLevel = curLevel + 1;
		m_arrayHeroHungingHuntingCfg[curLevel].fixedProceeds = filereader.GetFloatValue("Efficiency");
		m_arrayHeroHungingHuntingCfg[curLevel].fLuckyOdds = filereader.GetFloatValue("Probability");
		m_arrayHeroHungingHuntingCfg[curLevel].goldProceeds = filereader.GetIntValue("Money");
		m_arrayHeroHungingHuntingCfg[curLevel].goldWeight = filereader.GetFloatValue("MoneyWeight");
		m_arrayHeroHungingHuntingCfg[curLevel].diamondProceeds = filereader.GetIntValue("Gold");
		m_arrayHeroHungingHuntingCfg[curLevel].diamondWeight = filereader.GetFloatValue("GoldWeight");
		m_arrayHeroHungingHuntingCfg[curLevel].propsWeight = filereader.GetFloatValue("PropsWeight");
		m_arrayHeroHungingHuntingCfg[curLevel].equipWeight = filereader.GetFloatValue("EquipmentWeight");
		m_arrayHeroHungingHuntingCfg[curLevel].debrisWeight = filereader.GetFloatValue("DebrisWeight");
		m_arrayHeroHungingHuntingCfg[curLevel].totalWeight = m_arrayHeroHungingHuntingCfg[curLevel].diamondWeight + m_arrayHeroHungingHuntingCfg[curLevel].goldWeight
			+ m_arrayHeroHungingHuntingCfg[curLevel].propsWeight + m_arrayHeroHungingHuntingCfg[curLevel].equipWeight + m_arrayHeroHungingHuntingCfg[curLevel].debrisWeight;
		string ItemStr = filereader.GetStrValue("Props");
		m_arrayHeroHungingHuntingCfg[curLevel].extraPropsProceeds.clear();
		CTabTableReader::StringSplit(ItemStr, "|", m_arrayHeroHungingHuntingCfg[curLevel].extraPropsProceeds);

		ItemStr = filereader.GetStrValue("Equipment");
		m_arrayHeroHungingHuntingCfg[curLevel].extraEqupProceeds.clear();
		CTabTableReader::StringSplit(ItemStr, "|", m_arrayHeroHungingHuntingCfg[curLevel].extraEqupProceeds);

		ItemStr = filereader.GetStrValue("Debris");
		m_arrayHeroHungingHuntingCfg[curLevel].extraDebrisProceeds.clear();
		CTabTableReader::StringSplit(ItemStr, "|", m_arrayHeroHungingHuntingCfg[curLevel].extraDebrisProceeds);
	}

	return true;
}


const HeroHungingHuntingCfg* CConfigManager::GetHeroHungingHuntingCfg(int masterLevel)
{
	if (m_arrayHeroHungingHuntingCfg == nullptr || masterLevel < 1 || masterLevel > m_iHeroHungingHuntingNums)
		return nullptr;

	return &m_arrayHeroHungingHuntingCfg[masterLevel - 1];
}


void CConfigManager::GetMasterExpAndLevel(OUT DWORD& exp, OUT DWORD& level)
{
	///从配置文件中找寻大于等于exp的第一个满足条件的configedExp（配置好的经验值）
	auto findResult = m_MasterLevelUpExpTable.lower_bound(exp);
	///代表满级了
	if (findResult == m_MasterLevelUpExpTable.end())
	{
		///直接取最后一个，经验值最大的那个
		if (m_MasterLevelUpExpTable.size() > 0)
		{
			auto iterEnd = m_MasterLevelUpExpTable.end();
			--iterEnd;
			exp = iterEnd->first;
			level = iterEnd->second;
		}
		else
		{
			rfalse("君主经验配置数为0");
			return;
		}
	}
	else
	{
		if (findResult->first > exp) ///经验值够进一级或者第一级都进不了的情况
		{
			if (findResult != m_MasterLevelUpExpTable.begin())
				level = (--findResult)->second;
		}
		else ///经验值刚好够进级的情况
		{
			level = findResult->second;
		}
	}
}

bool CConfigManager::InitForgingConfig()
{
	string fileName = "ServerConfig\\ZhuangBeiJingLian.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	m_ForgingConfigList.clear();
	while (filereader.ReadLine())
	{
		ForgingConfig tempForgingConfig;

		tempForgingConfig.id = filereader.GetIntValue("ID");
		tempForgingConfig.refiningTime = filereader.GetIntValue("RefiningTime");
		int levelExp = filereader.GetIntValue("Level1Exp");
		
		tempForgingConfig.baseAttribute.Power = filereader.GetIntValue("Power");
		tempForgingConfig.baseAttribute.Agility = filereader.GetIntValue("Agility");
		tempForgingConfig.baseAttribute.Intelligence = filereader.GetIntValue("Intelligence");
		tempForgingConfig.baseAttribute.MaxHealth = filereader.GetIntValue("MaxHealth");
		tempForgingConfig.baseAttribute.Physic = filereader.GetIntValue("Physic");
		tempForgingConfig.baseAttribute.Magic = filereader.GetIntValue("Magic");
		tempForgingConfig.baseAttribute.Armor = filereader.GetIntValue("Armor");
		tempForgingConfig.baseAttribute.Resistance = filereader.GetIntValue("Resistance");
		tempForgingConfig.baseAttribute.Crit = filereader.GetIntValue("Crit");
		tempForgingConfig.baseAttribute.HealthRecovery = filereader.GetIntValue("HealthRecovery");
		tempForgingConfig.baseAttribute.EnergyRecovery = filereader.GetIntValue("EnergyRecovery");
		tempForgingConfig.baseAttribute.BloodLevel = filereader.GetIntValue("BloodLevel");
		tempForgingConfig.baseAttribute.OffsetArmor = filereader.GetIntValue("OffsetArmor");
		tempForgingConfig.baseAttribute.OffsetResistance = filereader.GetIntValue("OffsetResistance");
		tempForgingConfig.baseAttribute.AddCure = filereader.GetFloatValue("AddCure");
		tempForgingConfig.baseAttribute.Dodge = filereader.GetIntValue("Dodge");

		int tempLevelExp = 0;
		///一下是将升到某一星级的所需的所有经验值都加起来
		if (levelExp > 0)
		{
			tempForgingConfig.levelExp.insert(make_pair(levelExp, 1));

			tempLevelExp = filereader.GetIntValue("Level2Exp");
			if (tempLevelExp > 0)
			{
				levelExp += tempLevelExp;
				tempForgingConfig.levelExp.insert(make_pair(levelExp, 2));

				tempLevelExp = filereader.GetIntValue("Level3Exp");
				if (tempLevelExp > 0)
				{
					levelExp += tempLevelExp;
					tempForgingConfig.levelExp.insert(make_pair(levelExp, 3));

					tempLevelExp = filereader.GetIntValue("Level4Exp");
					if (tempLevelExp > 0)
					{
						levelExp += tempLevelExp;
						tempForgingConfig.levelExp.insert(make_pair(levelExp, 4));

						tempLevelExp = filereader.GetIntValue("Level5Exp");
						if (tempLevelExp > 0)
						{
							levelExp += tempLevelExp;
							tempForgingConfig.levelExp.insert(make_pair(levelExp, 5));
						}
					}
				}
			}
		}

		if (m_ForgingConfigList.find(tempForgingConfig.id) == m_ForgingConfigList.end())
			m_ForgingConfigList.insert(make_pair(tempForgingConfig.id, move(tempForgingConfig)));
		else
			rfalse("文件%s 有重复ID", fileName.c_str());
	}

	return true;
}

const ForgingConfig* CConfigManager::GetForgingConfig(int equipID)
{
	auto findResult = m_ForgingConfigList.find(equipID);
	if (findResult != m_ForgingConfigList.end())
		return &findResult->second;

	return nullptr;
}

bool CConfigManager::GetForgingExpAndStarLevel(int equipID, OUT DWORD& exp, OUT DWORD& starLevel)
{
	if (exp == 0)
		return false;

	auto findEquipResult = m_ForgingConfigList.find(equipID);
	if (findEquipResult == m_ForgingConfigList.end())
		return false;

	///寻找该装备配置的等级对应的经验值中大于等于exp的第一个经验值
	auto findResult = findEquipResult->second.levelExp.lower_bound(exp);
	if (findResult == findEquipResult->second.levelExp.end())///满级的情况
	{
		if (findEquipResult->second.levelExp.size() > 0)
		{
			starLevel = findEquipResult->second.refiningTime;
			auto iterEnd = findEquipResult->second.levelExp.end();
			--iterEnd;
			exp = iterEnd->first;
		}
		else
		{
			rfalse("装备%d的锻造经验为0", equipID);
			return false;
		}
	}
	else
	{
		if (findResult->first > exp) ///经验值够进一级或者第一级都进不了的情况
		{
			if (findResult != findEquipResult->second.levelExp.begin())
				starLevel = (--findResult)->second;
		}
		else ///经验值刚好够进级的情况
		{
			starLevel = findResult->second;
		}
	}

	return true;
}

bool CConfigManager::_InitRobotNameConfig()
{
	string fileName = "ServerConfig\\JiQiRenMingZi.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
	}

	while (filereader.ReadLine())
	{
		m_RobotNameConfig.AddPrefixName(filereader.GetStrValue("PrefixName"));
		m_RobotNameConfig.AddSufffixName(filereader.GetStrValue("SuffixName"));
		m_RobotNameConfig.AddThirdName(filereader.GetStrValue("ThirdName"));
		
	}
	return true;
}

RobotNameConfig &CConfigManager::GetRobotNameConfig()
{
	return m_RobotNameConfig;
}

bool CConfigManager::InitHeroExpConfig()
{
	string fileName = "ServerConfig\\YingXiongDengJi.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	int curLevel;
	int curExp;
	m_HeroExpConfig.clear();
	m_HeroLevelNeededExp.clear();
	m_HeroLevelNeededExp.reserve(filereader.GetDataLineCount());
	while (filereader.ReadLine())
	{
		curLevel = filereader.GetIntValue("Level");
		curExp = filereader.GetIntValue("HeroExp");

		if (m_HeroExpConfig.find(curExp) == m_HeroExpConfig.end())
			m_HeroExpConfig.insert(make_pair(curExp, curLevel));
		else
			rfalse("英雄等级文件%s 有重复的经验值", fileName.c_str());

		m_HeroLevelNeededExp.push_back(curExp);
	}
}

void CConfigManager::GetHeroExpAndLevel(OUT DWORD& level, OUT DWORD& exp, OUT bool& levelLimitted)
{
	levelLimitted = false;
	///从配置文件中找寻大于等于exp的第一个满足条件的configedExp（配置好的经验值）
	auto findResult = m_HeroExpConfig.lower_bound(exp);
	///代表满级了
	if (findResult == m_HeroExpConfig.end())
	{
		///直接取最后一个，经验值最大的那个
		if (m_HeroExpConfig.size() > 0)
		{
			levelLimitted = true;
			auto iterEnd = m_HeroExpConfig.end();
			--iterEnd;
			exp = iterEnd->first;
			level = iterEnd->second;
		}
		else
		{
			rfalse("武将经验配置数为0");
			return;
		}
	}
	else
	{
		if (findResult->first > exp) ///经验值够进一级或者第一级都进不了的情况
		{
			if (findResult != m_HeroExpConfig.begin())
				level = (--findResult)->second;
		}
		else ///经验值刚好够进级的情况
		{
			level = findResult->second;
		}
	}
}

int32_t CConfigManager::GetHeroMaxLevel()
{
	auto endItor = m_HeroExpConfig.end();
	if (endItor == m_HeroExpConfig.begin())
		return 0;
	--endItor;

	return endItor->second;
}

int CConfigManager::GetHeroNeededExp(DWORD level)
{
	if (level > m_HeroLevelNeededExp.size())
		return -1;

	return m_HeroLevelNeededExp[level - 1];
}

//加载签到奖励配置
bool CConfigManager::InitCheckinAwardsConfig()
{
	string fileName = "ServerConfig\\QianDao.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}
	m_CheckinAwardsConfigMap.clear();
	while (filereader.ReadLine())
	{

		CheckInConfig config;
		config.times = filereader.GetIntValue("Time");
		config.doubleNeedVip = filereader.GetIntValue("Double");
		config.Goods[0].itype = filereader.GetIntValue("RewardType1");
		config.Goods[0].id = filereader.GetIntValue("RewardID1");
		config.Goods[0].num = filereader.GetIntValue("RewardNumber1");
		config.Goods[1].itype = filereader.GetIntValue("RewardType2");
		config.Goods[1].id = filereader.GetIntValue("RewardID2");
		config.Goods[1].num = filereader.GetIntValue("RewardNumber2");

		if (m_CheckinAwardsConfigMap.find(config.times) == m_CheckinAwardsConfigMap.end())
			m_CheckinAwardsConfigMap.insert(make_pair(config.times, config));
		else
			rfalse("配置文件%s存在相同的物品ID%d", fileName.c_str(), config.times);
	}
	return true;
}

const CheckInConfig*   CConfigManager::GetCheckInAwardsConfig(int index)
{
	auto findResult = m_CheckinAwardsConfigMap.find(index);
	if (findResult != m_CheckinAwardsConfigMap.end())
		return &findResult->second;

	return nullptr;
}
//加载7日签到奖励配置
bool CConfigManager::InitCheckinDay7AwardsConfig()
{
	string fileName = "ServerConfig\\qiandaoday7.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}
	m_CheckInday7AwardsConfigMap.clear();
	while (filereader.ReadLine())
	{

		CheckInday7Config config;
		config.SignInDay = filereader.GetIntValue("SignInDay");
		config.Goods[0].itype = filereader.GetIntValue("FirstType");
		config.Goods[0].id = filereader.GetIntValue("FirstID");
		config.Goods[0].num = filereader.GetIntValue("FirstNum");
		config.Goods[1].itype = filereader.GetIntValue("SignInType");
		config.Goods[1].id = filereader.GetIntValue("SignInID");
		config.Goods[1].num = filereader.GetIntValue("SignInNum");

		if (m_CheckInday7AwardsConfigMap.find(config.SignInDay) == m_CheckInday7AwardsConfigMap.end())
			m_CheckInday7AwardsConfigMap.insert(make_pair(config.SignInDay, config));
		else
			rfalse("配置文件%s存在相同的物品ID%d", fileName.c_str(), config.SignInDay);
	}
	return true;
}
const CheckInday7Config*   CConfigManager::GetCheckInday7AwardsConfig(int index)
{
	auto findResult = m_CheckInday7AwardsConfigMap.find(index);
	if (findResult != m_CheckInday7AwardsConfigMap.end())
		return &findResult->second;

	return nullptr;
}
bool CConfigManager::InitArenaRewardConfigList()
{
	string fileName = "ServerConfig\\JingJiChangJiangLi.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	m_ArenaRewardConfigList.clear();
	while (filereader.ReadLine())
	{
		ArenaRewardConfig config ;
		config.Gem = filereader.GetIntValue("Gem");
		config.Gold = filereader.GetIntValue("Gold");
		config.Honor = filereader.GetIntValue("Honor");
		config.Medicine = filereader.GetIntValue("Medicine");
		config.rankFloor = filereader.GetIntValue("rankFloor");
		config.rankUp = filereader.GetIntValue("rankUp");
		config.RefinedMaterial = filereader.GetIntValue("RefinedMaterial");
		config.RefinedMaterialNumber = filereader.GetIntValue("RefinedMaterialNumber");
		config.MedicineNumber = filereader.GetIntValue("MedicineNumber");
		config.BestRecord = filereader.GetFloatValue("BestRecord");
		//m_ArenaRewardConfigList.insert(make_pair(config.rankUp, config));
		m_ArenaRewardConfigList.insert(make_pair(config.rankFloor, config));

	} 
	return true;

}
// 第一个满足rank <= lower，则返回对应值
bool CConfigManager::GetArenaRewardConfig(int rank, ArenaRewardConfig& rewardConfig)
{
	if (m_ArenaRewardConfigList.lower_bound(rank) != m_ArenaRewardConfigList.end())
	{
		rewardConfig = m_ArenaRewardConfigList.lower_bound(rank)->second;
		return true;
	}
	return false;
}

bool CConfigManager::_InitArenaRobotLevelInfo()
{
	string fileName = "ServerConfig\\JingJiChangDengji.txt";
	CTabTableReader filereader;
	if (!filereader.OpenTabTable(fileName))
	{
		rfalse("can not find file %s！！！！", fileName.c_str());
		return false;
	}

	m_ArenaRobotLevelInfo.clear();
	while (filereader.ReadLine())
	{
		ArenaRobotLevelInfo levelInfo;
		levelInfo.uper = filereader.GetIntValue("Uper");
		levelInfo.lower = filereader.GetIntValue("Lower");
		levelInfo.heroLevel = filereader.GetIntValue("HeroLevel");
		levelInfo.rankLevel = filereader.GetIntValue("HeroRankLevel");
		levelInfo.starLevel = filereader.GetIntValue("HeroStarLevel");
		levelInfo.skillLevel = filereader.GetIntValue("HeroSkillLevel");
		levelInfo.level = filereader.GetIntValue("Level");
		m_ArenaRobotLevelInfo.push_back(levelInfo);
	}
	return true;
}

const vector< ArenaRobotLevelInfo> & CConfigManager::GetArenaRobotLevelInfo()
{
	return m_ArenaRobotLevelInfo;
}

bool CConfigManager::_InitGuideConfig()
{
	string fileName = "ServerConfig\\XinShouYinDao.txt";
	CTabTableReader fileReader;
	if (!fileReader.OpenTabTable(fileName))
	{
		rfalse("can not find %s ! ! ! !", fileName.c_str());
		return false;
	}

	m_GuideConfigList.clear();
	while (fileReader.ReadLine())
	{
		NewbieGuideConfig guideConfig;
		guideConfig.guideType = fileReader.GetIntValue("GuideType");
		guideConfig.startIndex = fileReader.GetIntValue("StartIndex");
		guideConfig.nextTollgateID = fileReader.GetIntValue("TollgateID");
		guideConfig.rewardID = fileReader.GetIntValue("RewardID");
		string tempStr = fileReader.GetStrValue("OperateIndex");
		guideConfig.vecOperateIndex.clear();
		CTabTableReader::StringSplit(tempStr, m_SplitSign.c_str(), guideConfig.vecOperateIndex);

		if (m_GuideConfigList.find(guideConfig.guideType) == m_GuideConfigList.end())
			m_GuideConfigList.insert(make_pair(guideConfig.guideType, guideConfig));
		else
			rfalse("配置文件%s 存在相同键值元素%d ! ! ! !", fileName.c_str(), guideConfig.guideType);
	}

	return true;
}

const NewbieGuideConfig* CConfigManager::GetGuideConfig(int type)
{
	auto findIter = m_GuideConfigList.find(type);

	return findIter != m_GuideConfigList.end() ? &findIter->second : nullptr;
}

int CConfigManager::GetGuideType(int tollgateID)
{
	for (auto &iter : m_GuideConfigList)
	{
		if (iter.second.nextTollgateID == tollgateID)
			return iter.first;
	}

	return -1;
}

bool CConfigManager::_InitFateConfig()
{
	string fileName = "ServerConfig\\YuanFen.txt";
	CTabTableReader fileReader;
	if (!fileReader.OpenTabTable(fileName))
	{
		rfalse("cant not find %s", fileName.c_str());
		return false;
	}

	m_FateConfigMap.clear();
	while (fileReader.ReadLine())
	{
		FateConfig config;
		config.heroID = fileReader.GetIntValue("HeroID");
		string tempStr = fileReader.GetStrValue("RelationshipHero");
		CTabTableReader::StringSplit(tempStr, m_SplitSign.c_str(), config.vecRelationshipHero);
		tempStr = fileReader.GetStrValue("Style");
		CTabTableReader::StringSplit(tempStr, m_SplitSign.c_str(), config.vecAttributeType);
		tempStr = fileReader.GetStrValue("AddPercent");
		CTabTableReader::StringSplit(tempStr, m_SplitSign.c_str(), config.vecAttributePercent);
		if (m_FateConfigMap.find(config.heroID) != m_FateConfigMap.end())
		{
			m_FateConfigMap[config.heroID].push_back(config);
		}
		else
		{
			vector<FateConfig> vecConfig;
			vecConfig.push_back(config);
			m_FateConfigMap.insert(make_pair(config.heroID, move(vecConfig)));
		}
	}

	return true;
}

bool CConfigManager::_InitHeroSkillConfig()
{
	string fileName = "ServerConfig\\YingXiongJiNeng.txt";
	CTabTableReader fileReader;
	if (!fileReader.OpenTabTable(fileName))
	{
		rfalse("cant not find %s", fileName.c_str());
		return false;
	}

	m_HeroSkillConfigMap.clear();

	while (fileReader.ReadLine())
	{
		HeroSkillConfig skillConfig;
		skillConfig.id = fileReader.GetIntValue("ID");
		skillConfig.type = fileReader.GetIntValue("SkillType");
		skillConfig.effective = fileReader.GetIntValue("Effective");
		skillConfig.factor = fileReader.GetFloatValue("Factor");
		skillConfig.damage = fileReader.GetFloatValue("Damage");
		skillConfig.addDamage = fileReader.GetFloatValue("AddDamage");
		skillConfig.extraDamage = fileReader.GetFloatValue("ExtraDamage");
		skillConfig.buffID1 = fileReader.GetIntValue("BuffID1");
		skillConfig.buffID2 = fileReader.GetIntValue("BuffID2");
		skillConfig.selfBuffID = fileReader.GetIntValue("SelfBuff");

		if (m_HeroSkillConfigMap.find(skillConfig.id) == m_HeroSkillConfigMap.end())
			m_HeroSkillConfigMap.insert(make_pair(skillConfig.id, skillConfig));
		else
			rfalse("存在相同的技能ID:%s", skillConfig.id);
	}

	return true;
}

const HeroSkillConfig* CConfigManager::GetHeroSkillConfig(int32_t skillID)
{
	auto iter = m_HeroSkillConfigMap.find(skillID);

	return iter == m_HeroSkillConfigMap.end() ? nullptr : &iter->second;
}

bool CConfigManager::_InitBuffConfig()
{
	string fileName = "ServerConfig\\BuffConfig.txt";
	CTabTableReader fileReader;
	if (!fileReader.OpenTabTable(fileName))
	{
		rfalse("cant not find %s", fileName.c_str());
		return false;
	}

	m_BuffConfigMap.clear();
	while (fileReader.ReadLine())
	{
		BuffConfig config;
		config.id = fileReader.GetIntValue("ID");
		config.name = fileReader.GetStrValue("Name");
		config.style = fileReader.GetIntValue("Style");
		config.initiaValue = fileReader.GetFloatValue("InitialValue");
		config.addValue = fileReader.GetFloatValue("AddValue");
		config.EffectiveStyle = fileReader.GetIntValue("EffectiveStyle");
		if (m_BuffConfigMap.find(config.id) == m_BuffConfigMap.end())
			m_BuffConfigMap.insert(make_pair(config.id, config));
		else
			rfalse("存在相同的buffID:%d", config.id);
	}

	return true;
}

const BuffConfig* CConfigManager::GetBuffConfig(int32_t buffID)
{
	auto iter = m_BuffConfigMap.find(buffID);
	return iter == m_BuffConfigMap.end() ? nullptr : &iter->second;
}

bool CConfigManager::_InitResistanceRateCalculateConfig()
{
	string fileName = "ServerConfig\\FaKangZhuanHuan.txt";
	CTabTableReader fileReader;
	if (!fileReader.OpenTabTable(fileName))
	{
		rfalse("can not find %s", fileName.c_str());
		return false;
	}

	m_ResistanceRateCalculateConfigMap.clear();
	while (fileReader.ReadLine())
	{
		m_ResistanceRateCalculateConfigMap.insert(make_pair(fileReader.GetIntValue("Resistance"), fileReader.GetFloatValue("ResistanceRate")));
	}

	return true;
}

float CConfigManager::GetResistanceRate(int32_t resistanceRateValue)
{
	auto iter = m_ResistanceRateCalculateConfigMap.lower_bound(resistanceRateValue);
	if (iter == m_ResistanceRateCalculateConfigMap.end())
	{
		rfalse("没有对应绝技抵抗值%d", resistanceRateValue);
		return 0.0f;
	}

	return iter->second;
}

