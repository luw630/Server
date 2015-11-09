#pragma once
#include <memory>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include "stdint.h"
#include "..\PUB\Singleton.h"
#include "SanguoConfigTypeDef.h"
#include "RobotNameConfig.h"

class CTabTableReader;
enum InstanceType;
enum MissionType;

class CConfigManager : public CSingleton<CConfigManager>
{
public:
	CGlobalConfig globalConfig;
	CConfigManager();
	~CConfigManager();
	int m_ServerStartTime; //临时用于开服时间的存储，想明白之后再改动  add by lpd
	/*
	static CConfigManager& GetInstance()
	{
		static CConfigManager instance;
		return instance;
	}
	*/

	///@brief 获取游戏玩法功能开放等级限制配置表信息
	const GameFeatureActivationConfig& GetGameFeatureActivationConfig();
	///@brief 获取指定远征关卡的配置信息
	///@param masterLevel 当前的君主等级，用来获得某一个确切的等级区间内，远征关卡的配置信息
	///@param instanceID 远征关卡的ID
	const ExpeditionInstanceInfor* GetSpecifyExpeditionInstanceConfig(int32_t masterLevel, int32_t instanceID);
	///@brief 获取指定远征关卡的下一个远征关卡的ID
	///@return 如果失败则返回-1
	int32_t GetNextExpeditionInstanceID(int32_t instanceID);
	///@brief 获取第一个远征关卡的ID
	///@return 如果失败返回-1
	int32_t GetFirstExpeditionInstanceID();
	///@brief 获取指定的英雄指定的等级所需的经验值
	///@param level[out] 获取传入的经验值对应的等级
	///@param exp[in\out] 传入的某一个经验值，函数内部判断完了后会将其截断为玩家所能达到的最大的经验值并返回
	///@param levelLimitted[out] 是否经验等级冲顶了
	void GetHeroExpAndLevel(OUT DWORD& level, OUT DWORD& exp, OUT bool& levelLimitted);
	///@brief 获取英雄能达到的最高的等级
	///@return  返回当前英雄能达到的最高的等级,失败返回0
	int32_t GetHeroMaxLevel();
	///@brief 获取英雄的对应等级的所需的经验值
	///@param level 英雄的等级
	///@return 成功返回对应的值，失败返回-1
	int GetHeroNeededExp(DWORD level);
	///@brief 获取普通副本、精英副本、将神传说副本、秘境寻宝副本的对应章节的配置信息
	const ChapterConfig* GetChapterConfig(DWORD chapterID);
	///@brief 获取普通副本、精英副本指定的章节的战斗评分相关的奖励配置
	const ChapterBattleScoreRewardsConfig* GetChapterBattleScoreRewardsConfig(DWORD chapterID);
	///@brief 获取指定副本类型的第一个副本的ID
	///@return 成功返回对应的值，失败返回-1
	int GetFirstDuplicateID(InstanceType type);
	///@brief 获取指定副本类型的最后一个副本的ID
	///@return 成功返回对应的值，失败返回-1
	int GetEndDuplicateID(InstanceType type);
	///@brief 获取指定副本类型的所有副本的ID
	const set<int>* GetDuplicateListByType(InstanceType type);
	///@brief 获取指定副本类型的第一个关卡ID
	///@return 成功返回对应的值，失败返回-1
	int GetFirstTollgateID(InstanceType type);
	///@brief 获取指定副本类型的最后一个关卡ID
	///@return 成功返回对应的值，失败返回-1
	int GetEndTollgateID(InstanceType type);
	///@brief 获取普通副本、精英副本、将神传说副本、秘境寻宝副本的对应关卡的配置信息
	///@param levelID 普通副本、精英副本、将神传说副本、秘境寻宝副本对应的关卡ID
	const BattleLevelConfig* GetBattleLevelConfig(DWORD levelID);
	///@brief 获取普通副本、精英副本、将神传说副本、秘境寻宝副本的对应关卡的第一次掉落配置信息
	///@param levelID 普通副本、精英副本、将神传说副本、秘境寻宝副本对应的关卡ID
	const BattleFirstDropConfig* GetBattleFirstDropConfig(DWORD levelID);
	///@brief 获取普通副本、精英副本、将神传说副本、秘境寻宝副本的对应关卡的掉落配置信息
	const BattleDropConfig * GetBattleDropConfig(DWORD levelID);
	///@brief 获取君主等级对应的等级信息
	const MasterLevelInfor* GetMasterLevelInfor(DWORD masterLevel);
	///@brief 用于判断当前的经验值是否能升一级
	///@param exp 当前的君主经验值，如果超过了当前开放的等级的最大的经验值，会截断为当前所能达到的最大的经验值
	///@param level 当前的君主等级，判断完成之后可能会改成会升到的等级，会截断为当前所能达到的最大的经验值
	void GetMasterExpAndLevel(OUT DWORD& exp,OUT DWORD& level);
	///@brief 获取对应装备的精练的配置
	const ForgingConfig* GetForgingConfig(int equipID);
	///@brief 用于判断某一个装备的经验值是否能升级或者升到某一级
	///@param equipID[in] 要操作的装备的ID
	///@param exp[out] 已经有的经验值，判断后溢出的部分会截掉输出
	///@param starLevel[out] 当前装备的星级，如果exp达到了升级的条件，则会更新成会升级后的星级
	///@return 如果装备不存在或者exp为0返回false
	bool GetForgingExpAndStarLevel(int equipID, OUT DWORD& exp, OUT DWORD& starLevel);
	///@brief 获取指定英雄的配置文件
	const HeroConfig * GetHeroConfig(int32_t heroID);
	///@brief 获取当前可用的所有的英雄及其配置
	const map<int32_t, HeroConfig>& GetHeroConfigMap(){ return m_HeorConfigMap; }
	///@brief 获取当前开发的力量型英雄的列表
	const vector<int32_t>& GetStrengthHeroList() { return m_vecAvaliableStrengthHeroList; }
	///@brief 获取当前开放的智力型英雄的列表
	const vector<int32_t>& GetManaHeroList() { return m_vecAvaliableManaHeroList; }
	///@brief 获取当前开放的敏捷型英雄的列表
	const vector<int32_t>& GetAgilityHeroList() { return m_vecAvaliableAgilityHeroList; }
	const  EquipmentConfig* GetEquipment(int32_t equipmentID);
	const HeroRankConfig* GetHeroRankConfig(int32_t heroID);
	///@breif 获取君主等级对应的训练的的配置文件
	const HeroTrainingCfg* GetHeroTrainingCfg(int masterLevel);
	///@brief 获取君主等级对应的挂机寻宝的配置信息
	const HeroHungingHuntingCfg* GetHeroHungingHuntingCfg(int masterLevel);
	///@brief 获取指定英雄的最大的品阶等级
	///@return 成功放回对应的值，失败放回-1
	int GetHeroMaxRankGrade(int32_t heroID);
	const HeroStarConfig* GetHeroStarAttr(int32_t heroID);
	///@brief 获取指定的怪物的基础属性信息
	///@return 成功放回所需的值，失败返回null
	const MonsterConfig* GetMonsterAttrData(int32_t monsterID);
	///@brief 获取怪物的基础数据
	///@param 指定的要获取的怪物的数据
	///@param[out] 怪物的基础数据的存储空间
	///return 成功返回true，失败返回false
	bool GetMonsterBaseData(int32_t monsterID, OUT MonsterData& outputData);
	///@brief 获取指定英雄的能升到的最大的星级
	///@return 成功放回对应的值，失败放回-1
	int GetHeroMaxStarLevel(int32_t heroID);
	const ItemConfig* GetItemConfig(int32_t itemID);
	///@brief 获取指定的任务的配置文件信息
	const MissionConfig* GetMissionConfig(int missionID) const;
	///@brief 获取指定任务类型的任务列表
	const set<int>* GetMissionListByType(MissionType missionType) const;
	///@brief 获取任务活跃度奖励的配置信息
	const MissionActivenessReswardsConfig* GetMissionActivenessRewardsConfig(int activenessTrigger) const;
	///@brief 判断任务的活跃度是否达到顶值
	bool IsMissionActivenessRestricted(int activeness);
	///@brief 获取购买金钱次数对应的配置文件
	///@param boughtCount已经购买的次数
	const ExchangeGoldConfig* GetExchangeGoldConfig(int boughtCount) const;
	///@brief 根据兑换消耗来获取消耗相同的兑换金币操作信息
	///@param curCost 要查询的消耗值
	///@param configList[out] 拥有相同宝石消耗值的兑换金币操作信息
	///@return 如果要查询的消耗有对应的配置，则返回true，否则返回false
	bool GetExchangeGoldConfigListByCost(DWORD curCost, OUT set<int>& configList) const;
	///@breif 获取任务ID列表
	void GetMissionIDList(OUT vector<int>& IDlist) const;
	///@brief 获取指定成就ID对应的终生成就配置信息
	const AchievementConfig* GetAchievementConfig(int32_t achievementID) const;
	///@brief 获取随机生成的可达成的成就的对应的成就配置信息
	const RandomAchievementConfig* GetRandomAchievementConfig(int32_t achievementID) const;
	///@brief 获取指定的君主等级对应的开放的成就ID列表
	///@param level 对应的君主等级
	///@param IDList 用于返回的已经开放的成就ID
	bool GetAccessibleAchievementIDS(DWORD level, OUT unordered_map<int32_t, int32_t>& IDList);
	///@brief 获取武将进阶成就ID列表
	const map<int32_t, vector<int32_t>>& GetHeroRankUpgradeAchievementIDList()const;
	///@brief 获取指定的成就ID在其组内的下一个成就ID
	///@param acheivementID[in] 指定的成就的ID
	///@param nextAchievementID[out] 指定的成就ID在其组内的下一个成就ID，如果指定的成就ID对应的成就是该组最后一个成就，则返回出-1
	///@return 成功返回true，失败返回false
	bool GetNextAchievementID(int32_t acheivementID, OUT int32_t& nextAchievementID) const;
	///@brief 获取成就ID列表
	void GetAchievementIDList(OUT vector<int32_t>& IDList);
	///@brief 获取某一组别的成就的默认的最新的未完成的成就的ＩＤ
	///@return 如果成功返回相应的值，如果失败则返回-1
	int GetAchivementGroupInitID(int32_t group);
	///@brief 判断某一个成就ID是否存在于某一个组别的成就中
	///@param group 成就点组别
	///@param tollgateID 要判断的成就的ID
	///@return 成功返回对应的成就ID,失败返回-1
	int IsTollgateIDExistInSpecificAchievementGroup(int32_t group, int32_t tollgateID);
	///@brief 获取某一个成就所属于的类别
	///@return 成功返回所需的值，否则返回-1
	int GetAchievementGroupType(int32_t achievementID);
	const vector<int32_t>& GetMeleeHeroIDList() { return m_MeleeHeroIDList; }
	const vector<int32_t>& GetRangeHeroIDList() { return m_RangeHeroIDList; }
	void GetEquipIDList(vector<int32_t>& equipVec);
	void GetVarietyShopCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig);
	void GetArenaShopCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig);
	void GetExpeditionShopCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig);
	void GetMiracleMerchantCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig);
	void GetGemMerchantCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig);
	void GetLegionShopCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig);
	void GetSoulExchangeCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig);
	void GetWarOfLeagueShopCommodityConfigs(DWORD level, vector<CommodityConfig*>& vecConfig);
	const CheckInConfig*   GetCheckInAwardsConfig(int index);
	bool GetArenaRewardConfig(int rank, ArenaRewardConfig& rewardConfig);
	const vector< ArenaRobotLevelInfo> & GetArenaRobotLevelInfo();
	///@brief 获取机器人名字配置
	RobotNameConfig &GetRobotNameConfig();

	const CheckInday7Config*   GetCheckInday7AwardsConfig(int index);
	/*@brief 获取引导配置
	@param [in] type 引导类型(索引)
	*/
	const NewbieGuideConfig* GetGuideConfig(int type);
	/*@brief 匹配关卡ID获取引导类型*/
	int GetGuideType(int tollgateID);
	/**@brief 获取武将缘分配置容器*/
	unordered_map<int, vector<FateConfig>>& GetFateConfigMap() { return m_FateConfigMap; }
	///@brief 获取武将技能配置
	///@param 技能ID
	const HeroSkillConfig* GetHeroSkillConfig(int32_t skillID);
	///@brief 获取buff配置
	///@param [in] buffID 指定获取的buffID
	const BuffConfig* GetBuffConfig(int32_t buffID);
	///@brief 获取绝技抵抗比例
	///@param [in] resistanceRateValue 绝技抵抗值
	float GetResistanceRate(int resistanceRateValue);
private:
	
	bool InitGolalConfig();
	///@brief 初始化游戏中的玩法功能的开放配置文件
	bool InitGameFeatureActivationConfig();
	bool InitHeroConfig();
	bool InitEquipConfig();
	///@brief 初始化宝石购买金钱的配置表信息
	bool InitExchangeGoldConfig();
	///@breif 初始化普通副本、精英副本、将神传说副本、秘境寻宝副本的怪物配置表
	bool InitMonsterConfig();
	///@brief 读取普通副本、精英副本、将神传说副本、秘境寻宝副本的章节配置表
	bool InitChapterConfig();
	///@brief 初始化普通副本、精英副本的战斗评分对应的奖励信息
	bool InitChapterBattleScoreRewardsConfig();
	///@brief 读取普通副本、精英副本、将神传说副本、秘境寻宝副本的关卡配置表
	bool InitBattleLevelConfig();
	///@brief 读取普通副本、精英副本、将神传说副本、秘境寻宝副本的第一次关卡掉落配置表
	bool InitBattleFristDropConfig();
	///@brief 读取普通副本、精英副本、将神传说副本、秘境寻宝副本的关卡掉落配置表
	bool InitBattleDropConfig();
	///@brief 读取装备精练配置
	bool InitForgingConfig();
	///@brief 初始化英雄某一个等级所需的经验的配置信息
	bool InitHeroExpConfig();
	bool InitHeroRankConfig();
	bool InitHeroStarConfig();
	///@brief 初始化任务的配置信息
	bool InitMissionConfig();
	///@brief 初始化任务的活跃度奖励配置
	bool InitMissionActivenessRewardsConfig();
	bool InitAchievementConfig();
	bool InitRandomAchievementConfig();
	bool InitMasterLevelInfor();
	///@brief 初始化训练英雄的配置文件
	bool InitHeroTrainingCfg();
	///@brief 初始化英雄挂机寻宝的配置文件
	bool InitHeroHuntingHuntingCfg();

	/**@brief 初始化物品配置	*/
	bool _InitItemConfig();
	bool InitExpeditionConfig();
	/**@brief 初始化商店配置 各商店配置结果相同 故使用一个接口*/
	bool _InitShopCommodityConfig(const string& fileName, vector<CommodityConfig>& vecShopCommodity);
	///@brief  初始化机器人名字配置表
	bool _InitRobotNameConfig();
	bool InitCheckinAwardsConfig(); //签到奖励
	bool InitArenaRewardConfigList(); //初始化竞技场奖励配置表
	
	///@brief 初始化竞技场机器人等级信息
	bool _InitArenaRobotLevelInfo();
	bool InitCheckinDay7AwardsConfig(); //7日签到奖励
	///@brief 初始化新手引导信息配置
	bool _InitGuideConfig();
	///@brief 初始化缘分配置
	bool _InitFateConfig();
	///@brief 初始化武将技能配置
	bool _InitHeroSkillConfig();
	///@brief 初始化buff配置
	bool _InitBuffConfig();
	///@brief 初始化法抗转换配置
	bool _InitResistanceRateCalculateConfig();

	///@breif 模块化地读取副本战斗评分奖励的配置
	void _ReadChapterBattleScoreRewardsConfigModule(CTabTableReader& fileReader, string baseAttrTag, unordered_map<int, vector<GoodsInfoSG>>& rewardsInfor);
private:

	int m_iHeroTrainingCfgNums;
	int m_iHeroHungingHuntingNums;
	const string m_SplitSign = "|";
	GameFeatureActivationConfig m_gameFeatureActivationConfig;
	map<int32_t, HeroConfig> m_HeorConfigMap;
	unordered_map<int32_t, MonsterConfig> m_MonsterConfigs; ///将神传说、秘境寻宝、普通副本、精英副本的怪物主要配置信息
	vector<int32_t> m_vecAvaliableStrengthHeroList;	///可用的力量英雄列表
	vector<int32_t> m_vecAvaliableManaHeroList;		///可用的智力英雄列表
	vector<int32_t> m_vecAvaliableAgilityHeroList;	///可用的敏捷英雄列表
	HeroTrainingCfg* m_arrayHeroTrainingCfg;///英雄训练的配置文件
	HeroHungingHuntingCfg* m_arrayHeroHungingHuntingCfg;
	map<int32_t, ItemConfig> m_ItemConfigMap;
	map<int32_t, EquipmentConfig> m_EquipmentMap;
	map<int32_t, int32_t> m_HeroExpConfig;///<英雄的某一等级所需要的经验配置,NOTE:key值为经验值，value值为对应的等级
	vector<int32_t> m_HeroLevelNeededExp;///<英雄某一等级对应的所需要的经验值
	map<int32_t, ChapterConfig> m_ChapterConfigList;
	unordered_map<int32_t, ChapterBattleScoreRewardsConfig> m_ChapterBattleScoreRewardsConfig; ///<章节副本的战斗评分对应的奖励
	unordered_map<int32_t, ExchangeGoldConfig> m_ExchangeGoldConfigList;	///<宝石换金钱的配置表
	unordered_multimap<int32_t, int32_t> m_ExchangeGoldDiamondCostList;		///<宝石换金钱的宝石消耗信息表，key值为宝石的消耗，value值为对应的兑换次数
	unordered_map<int32_t, ForgingConfig> m_ForgingConfigList;
	unordered_map<int32_t, MissionConfig> m_MissionConfigList;
	map<int32_t, MissionActivenessReswardsConfig> m_MissionActivenessRewardsConfig; ///<任务活跃度对应的奖励配置信息，key值为任务活跃度到达可领取状态的条件值
	unordered_map<MissionType, set<int>> m_MissionTypeList;
	map<int32_t, AchievementConfig> m_AchievementConfigList;	///<玩家终生成就的配置信息
	unordered_map<int32_t, RandomAchievementConfig> m_RandomAchievementConfigList;	///<随机生成的可达成成就的配置信息
	map<int32_t, unordered_set<int32_t>> m_RandomAchievementAccessibleLevel;///<随机生成的可达成的成就的等级开放信息，key值为开放的等级，value值为该开放等级所开放的成就
	unordered_map<int32_t, set<int32_t>> m_AchievementGroupIDList;///<每种成就类型所对应的全部的成就的ID，key值为成就的类型，value值为该组成就所包含的全部的成就ID
	unordered_map<int32_t, map<int32_t, int32_t>> m_DuplicateAchievementTollgateIDList;///<每种副本成就所对应的全部的副本关卡ID的表，key值为副本成就的类型， value值的key值为该组成就所包含的全部的关卡ID，value值为对应的成就ID 
	map<int32_t, vector<int32_t>> m_HeroRankUpgradeAchievementIDList;///<英雄进阶的成就ID列表,NOTE:Key值为英雄对应的品阶，value值为对应的成就ID
	unordered_map<int32_t, HeroStarConfig> m_HeroStarConfigMap;
	unordered_map<int32_t, HeroRankConfig> m_HeroRankConfigMap;
	unordered_map<int32_t, set<int32_t>> m_DuplicateTypeIDList;///<某一副本类型的副本ID的列表，key值为副本类型，value值为对应的副本列表
	unordered_map<int32_t, BattleLevelConfig> m_BattleLevelConfigList;///<普通副本、精英副本、将神传说副本、秘境寻宝副本的关卡配置表
	unordered_map<int32_t, BattleFirstDropConfig> m_BattleFirstDropConfigList;
	unordered_map<int32_t, BattleDropConfig> m_BattleDropConfigList; ///<普通副本、精英副本、将神传说副本、秘境寻宝副本的关卡掉落配置表
	vector<MasterLevelInfor> m_MasterLevelInforList; ///<君主等级信息表
	map<int32_t, int32_t> m_MasterLevelUpExpTable;///<君主升级所需经验表，key值为某一级所需的经验，value值为对应的等级
	map<int32_t, map<int32_t, ExpeditionInstanceInfor>> m_ExpeditionInstanceInfor;///<远征的配置信息，key值为远征的关卡ID，value值的map的key值为最小的开放等级,value值的map的value值为对应的远征配置
	vector<CommodityConfig> m_vecVarietyShopCommodity;///<杂货铺商品配置信息
	vector<CommodityConfig> m_vecArenaShopCommodity;///<竞技场商店商品配置信息
	vector<CommodityConfig> m_vecExpeditionShopCommodity;///<远征商店商品配置信息
	vector<CommodityConfig> m_vecMiracleMerchantCommodity;///<奇缘商店商品配置信息
	vector<CommodityConfig> m_vecGemMerchantCommodity;///<珍宝商店商品配置信息
	vector<CommodityConfig> m_vecLegionShopCommodity;///<军团商店商品配置信息
	vector<CommodityConfig> m_vecSoulExchangeCommodity;///<将魂兑换商店的配置信息
	vector<CommodityConfig> m_vecWarOfLeagueShopCommodity;///<国战商店的配置信息
	vector<int32_t> m_MeleeHeroIDList; //近战武将ID表
	vector<int32_t> m_RangeHeroIDList;//远程武将ID表
	RobotNameConfig m_RobotNameConfig;
	map<int32_t, CheckInConfig>  m_CheckinAwardsConfigMap;//30天签到奖励表
	map<int,ArenaRewardConfig> m_ArenaRewardConfigList; //竞技场奖励配置
	vector< ArenaRobotLevelInfo> m_ArenaRobotLevelInfo; //竞技场及机器等级信息
	map<int32_t, CheckInday7Config>  m_CheckInday7AwardsConfigMap;//7天签到奖励表
	unordered_map<int32_t, NewbieGuideConfig> m_GuideConfigList;	///引导信息配置
	unordered_map<int32_t, vector<FateConfig>> m_FateConfigMap;	///缘分配置
	unordered_map<int32_t, HeroSkillConfig> m_HeroSkillConfigMap;	///< 武将技能配置
	unordered_map<int32_t, BuffConfig> m_BuffConfigMap;	///< buff配置
	map<int32_t, float> m_ResistanceRateCalculateConfigMap;	///< 法抗转换配置
};

