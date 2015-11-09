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
	int m_ServerStartTime; //��ʱ���ڿ���ʱ��Ĵ洢��������֮���ٸĶ�  add by lpd
	/*
	static CConfigManager& GetInstance()
	{
		static CConfigManager instance;
		return instance;
	}
	*/

	///@brief ��ȡ��Ϸ�淨���ܿ��ŵȼ��������ñ���Ϣ
	const GameFeatureActivationConfig& GetGameFeatureActivationConfig();
	///@brief ��ȡָ��Զ���ؿ���������Ϣ
	///@param masterLevel ��ǰ�ľ����ȼ����������ĳһ��ȷ�еĵȼ������ڣ�Զ���ؿ���������Ϣ
	///@param instanceID Զ���ؿ���ID
	const ExpeditionInstanceInfor* GetSpecifyExpeditionInstanceConfig(int32_t masterLevel, int32_t instanceID);
	///@brief ��ȡָ��Զ���ؿ�����һ��Զ���ؿ���ID
	///@return ���ʧ���򷵻�-1
	int32_t GetNextExpeditionInstanceID(int32_t instanceID);
	///@brief ��ȡ��һ��Զ���ؿ���ID
	///@return ���ʧ�ܷ���-1
	int32_t GetFirstExpeditionInstanceID();
	///@brief ��ȡָ����Ӣ��ָ���ĵȼ�����ľ���ֵ
	///@param level[out] ��ȡ����ľ���ֵ��Ӧ�ĵȼ�
	///@param exp[in\out] �����ĳһ������ֵ�������ڲ��ж����˺�Ὣ��ض�Ϊ������ܴﵽ�����ľ���ֵ������
	///@param levelLimitted[out] �Ƿ���ȼ��嶥��
	void GetHeroExpAndLevel(OUT DWORD& level, OUT DWORD& exp, OUT bool& levelLimitted);
	///@brief ��ȡӢ���ܴﵽ����ߵĵȼ�
	///@return  ���ص�ǰӢ���ܴﵽ����ߵĵȼ�,ʧ�ܷ���0
	int32_t GetHeroMaxLevel();
	///@brief ��ȡӢ�۵Ķ�Ӧ�ȼ�������ľ���ֵ
	///@param level Ӣ�۵ĵȼ�
	///@return �ɹ����ض�Ӧ��ֵ��ʧ�ܷ���-1
	int GetHeroNeededExp(DWORD level);
	///@brief ��ȡ��ͨ��������Ӣ����������˵�������ؾ�Ѱ�������Ķ�Ӧ�½ڵ�������Ϣ
	const ChapterConfig* GetChapterConfig(DWORD chapterID);
	///@brief ��ȡ��ͨ��������Ӣ����ָ�����½ڵ�ս��������صĽ�������
	const ChapterBattleScoreRewardsConfig* GetChapterBattleScoreRewardsConfig(DWORD chapterID);
	///@brief ��ȡָ���������͵ĵ�һ��������ID
	///@return �ɹ����ض�Ӧ��ֵ��ʧ�ܷ���-1
	int GetFirstDuplicateID(InstanceType type);
	///@brief ��ȡָ���������͵����һ��������ID
	///@return �ɹ����ض�Ӧ��ֵ��ʧ�ܷ���-1
	int GetEndDuplicateID(InstanceType type);
	///@brief ��ȡָ���������͵����и�����ID
	const set<int>* GetDuplicateListByType(InstanceType type);
	///@brief ��ȡָ���������͵ĵ�һ���ؿ�ID
	///@return �ɹ����ض�Ӧ��ֵ��ʧ�ܷ���-1
	int GetFirstTollgateID(InstanceType type);
	///@brief ��ȡָ���������͵����һ���ؿ�ID
	///@return �ɹ����ض�Ӧ��ֵ��ʧ�ܷ���-1
	int GetEndTollgateID(InstanceType type);
	///@brief ��ȡ��ͨ��������Ӣ����������˵�������ؾ�Ѱ�������Ķ�Ӧ�ؿ���������Ϣ
	///@param levelID ��ͨ��������Ӣ����������˵�������ؾ�Ѱ��������Ӧ�Ĺؿ�ID
	const BattleLevelConfig* GetBattleLevelConfig(DWORD levelID);
	///@brief ��ȡ��ͨ��������Ӣ����������˵�������ؾ�Ѱ�������Ķ�Ӧ�ؿ��ĵ�һ�ε���������Ϣ
	///@param levelID ��ͨ��������Ӣ����������˵�������ؾ�Ѱ��������Ӧ�Ĺؿ�ID
	const BattleFirstDropConfig* GetBattleFirstDropConfig(DWORD levelID);
	///@brief ��ȡ��ͨ��������Ӣ����������˵�������ؾ�Ѱ�������Ķ�Ӧ�ؿ��ĵ���������Ϣ
	const BattleDropConfig * GetBattleDropConfig(DWORD levelID);
	///@brief ��ȡ�����ȼ���Ӧ�ĵȼ���Ϣ
	const MasterLevelInfor* GetMasterLevelInfor(DWORD masterLevel);
	///@brief �����жϵ�ǰ�ľ���ֵ�Ƿ�����һ��
	///@param exp ��ǰ�ľ�������ֵ����������˵�ǰ���ŵĵȼ������ľ���ֵ����ض�Ϊ��ǰ���ܴﵽ�����ľ���ֵ
	///@param level ��ǰ�ľ����ȼ����ж����֮����ܻ�ĳɻ������ĵȼ�����ض�Ϊ��ǰ���ܴﵽ�����ľ���ֵ
	void GetMasterExpAndLevel(OUT DWORD& exp,OUT DWORD& level);
	///@brief ��ȡ��Ӧװ���ľ���������
	const ForgingConfig* GetForgingConfig(int equipID);
	///@brief �����ж�ĳһ��װ���ľ���ֵ�Ƿ���������������ĳһ��
	///@param equipID[in] Ҫ������װ����ID
	///@param exp[out] �Ѿ��еľ���ֵ���жϺ�����Ĳ��ֻ�ص����
	///@param starLevel[out] ��ǰװ�����Ǽ������exp�ﵽ�������������������³ɻ���������Ǽ�
	///@return ���װ�������ڻ���expΪ0����false
	bool GetForgingExpAndStarLevel(int equipID, OUT DWORD& exp, OUT DWORD& starLevel);
	///@brief ��ȡָ��Ӣ�۵������ļ�
	const HeroConfig * GetHeroConfig(int32_t heroID);
	///@brief ��ȡ��ǰ���õ����е�Ӣ�ۼ�������
	const map<int32_t, HeroConfig>& GetHeroConfigMap(){ return m_HeorConfigMap; }
	///@brief ��ȡ��ǰ������������Ӣ�۵��б�
	const vector<int32_t>& GetStrengthHeroList() { return m_vecAvaliableStrengthHeroList; }
	///@brief ��ȡ��ǰ���ŵ�������Ӣ�۵��б�
	const vector<int32_t>& GetManaHeroList() { return m_vecAvaliableManaHeroList; }
	///@brief ��ȡ��ǰ���ŵ�������Ӣ�۵��б�
	const vector<int32_t>& GetAgilityHeroList() { return m_vecAvaliableAgilityHeroList; }
	const  EquipmentConfig* GetEquipment(int32_t equipmentID);
	const HeroRankConfig* GetHeroRankConfig(int32_t heroID);
	///@breif ��ȡ�����ȼ���Ӧ��ѵ���ĵ������ļ�
	const HeroTrainingCfg* GetHeroTrainingCfg(int masterLevel);
	///@brief ��ȡ�����ȼ���Ӧ�Ĺһ�Ѱ����������Ϣ
	const HeroHungingHuntingCfg* GetHeroHungingHuntingCfg(int masterLevel);
	///@brief ��ȡָ��Ӣ�۵�����Ʒ�׵ȼ�
	///@return �ɹ��Żض�Ӧ��ֵ��ʧ�ܷŻ�-1
	int GetHeroMaxRankGrade(int32_t heroID);
	const HeroStarConfig* GetHeroStarAttr(int32_t heroID);
	///@brief ��ȡָ���Ĺ���Ļ���������Ϣ
	///@return �ɹ��Ż������ֵ��ʧ�ܷ���null
	const MonsterConfig* GetMonsterAttrData(int32_t monsterID);
	///@brief ��ȡ����Ļ�������
	///@param ָ����Ҫ��ȡ�Ĺ��������
	///@param[out] ����Ļ������ݵĴ洢�ռ�
	///return �ɹ�����true��ʧ�ܷ���false
	bool GetMonsterBaseData(int32_t monsterID, OUT MonsterData& outputData);
	///@brief ��ȡָ��Ӣ�۵��������������Ǽ�
	///@return �ɹ��Żض�Ӧ��ֵ��ʧ�ܷŻ�-1
	int GetHeroMaxStarLevel(int32_t heroID);
	const ItemConfig* GetItemConfig(int32_t itemID);
	///@brief ��ȡָ��������������ļ���Ϣ
	const MissionConfig* GetMissionConfig(int missionID) const;
	///@brief ��ȡָ���������͵������б�
	const set<int>* GetMissionListByType(MissionType missionType) const;
	///@brief ��ȡ�����Ծ�Ƚ�����������Ϣ
	const MissionActivenessReswardsConfig* GetMissionActivenessRewardsConfig(int activenessTrigger) const;
	///@brief �ж�����Ļ�Ծ���Ƿ�ﵽ��ֵ
	bool IsMissionActivenessRestricted(int activeness);
	///@brief ��ȡ�����Ǯ������Ӧ�������ļ�
	///@param boughtCount�Ѿ�����Ĵ���
	const ExchangeGoldConfig* GetExchangeGoldConfig(int boughtCount) const;
	///@brief ���ݶһ���������ȡ������ͬ�Ķһ���Ҳ�����Ϣ
	///@param curCost Ҫ��ѯ������ֵ
	///@param configList[out] ӵ����ͬ��ʯ����ֵ�Ķһ���Ҳ�����Ϣ
	///@return ���Ҫ��ѯ�������ж�Ӧ�����ã��򷵻�true�����򷵻�false
	bool GetExchangeGoldConfigListByCost(DWORD curCost, OUT set<int>& configList) const;
	///@breif ��ȡ����ID�б�
	void GetMissionIDList(OUT vector<int>& IDlist) const;
	///@brief ��ȡָ���ɾ�ID��Ӧ�������ɾ�������Ϣ
	const AchievementConfig* GetAchievementConfig(int32_t achievementID) const;
	///@brief ��ȡ������ɵĿɴ�ɵĳɾ͵Ķ�Ӧ�ĳɾ�������Ϣ
	const RandomAchievementConfig* GetRandomAchievementConfig(int32_t achievementID) const;
	///@brief ��ȡָ���ľ����ȼ���Ӧ�Ŀ��ŵĳɾ�ID�б�
	///@param level ��Ӧ�ľ����ȼ�
	///@param IDList ���ڷ��ص��Ѿ����ŵĳɾ�ID
	bool GetAccessibleAchievementIDS(DWORD level, OUT unordered_map<int32_t, int32_t>& IDList);
	///@brief ��ȡ�佫���׳ɾ�ID�б�
	const map<int32_t, vector<int32_t>>& GetHeroRankUpgradeAchievementIDList()const;
	///@brief ��ȡָ���ĳɾ�ID�������ڵ���һ���ɾ�ID
	///@param acheivementID[in] ָ���ĳɾ͵�ID
	///@param nextAchievementID[out] ָ���ĳɾ�ID�������ڵ���һ���ɾ�ID�����ָ���ĳɾ�ID��Ӧ�ĳɾ��Ǹ������һ���ɾͣ��򷵻س�-1
	///@return �ɹ�����true��ʧ�ܷ���false
	bool GetNextAchievementID(int32_t acheivementID, OUT int32_t& nextAchievementID) const;
	///@brief ��ȡ�ɾ�ID�б�
	void GetAchievementIDList(OUT vector<int32_t>& IDList);
	///@brief ��ȡĳһ���ĳɾ͵�Ĭ�ϵ����µ�δ��ɵĳɾ͵ģɣ�
	///@return ����ɹ�������Ӧ��ֵ�����ʧ���򷵻�-1
	int GetAchivementGroupInitID(int32_t group);
	///@brief �ж�ĳһ���ɾ�ID�Ƿ������ĳһ�����ĳɾ���
	///@param group �ɾ͵����
	///@param tollgateID Ҫ�жϵĳɾ͵�ID
	///@return �ɹ����ض�Ӧ�ĳɾ�ID,ʧ�ܷ���-1
	int IsTollgateIDExistInSpecificAchievementGroup(int32_t group, int32_t tollgateID);
	///@brief ��ȡĳһ���ɾ������ڵ����
	///@return �ɹ����������ֵ�����򷵻�-1
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
	///@brief ��ȡ��������������
	RobotNameConfig &GetRobotNameConfig();

	const CheckInday7Config*   GetCheckInday7AwardsConfig(int index);
	/*@brief ��ȡ��������
	@param [in] type ��������(����)
	*/
	const NewbieGuideConfig* GetGuideConfig(int type);
	/*@brief ƥ��ؿ�ID��ȡ��������*/
	int GetGuideType(int tollgateID);
	/**@brief ��ȡ�佫Ե����������*/
	unordered_map<int, vector<FateConfig>>& GetFateConfigMap() { return m_FateConfigMap; }
	///@brief ��ȡ�佫��������
	///@param ����ID
	const HeroSkillConfig* GetHeroSkillConfig(int32_t skillID);
	///@brief ��ȡbuff����
	///@param [in] buffID ָ����ȡ��buffID
	const BuffConfig* GetBuffConfig(int32_t buffID);
	///@brief ��ȡ�����ֿ�����
	///@param [in] resistanceRateValue �����ֿ�ֵ
	float GetResistanceRate(int resistanceRateValue);
private:
	
	bool InitGolalConfig();
	///@brief ��ʼ����Ϸ�е��淨���ܵĿ��������ļ�
	bool InitGameFeatureActivationConfig();
	bool InitHeroConfig();
	bool InitEquipConfig();
	///@brief ��ʼ����ʯ�����Ǯ�����ñ���Ϣ
	bool InitExchangeGoldConfig();
	///@breif ��ʼ����ͨ��������Ӣ����������˵�������ؾ�Ѱ�������Ĺ������ñ�
	bool InitMonsterConfig();
	///@brief ��ȡ��ͨ��������Ӣ����������˵�������ؾ�Ѱ���������½����ñ�
	bool InitChapterConfig();
	///@brief ��ʼ����ͨ��������Ӣ������ս�����ֶ�Ӧ�Ľ�����Ϣ
	bool InitChapterBattleScoreRewardsConfig();
	///@brief ��ȡ��ͨ��������Ӣ����������˵�������ؾ�Ѱ�������Ĺؿ����ñ�
	bool InitBattleLevelConfig();
	///@brief ��ȡ��ͨ��������Ӣ����������˵�������ؾ�Ѱ�������ĵ�һ�ιؿ��������ñ�
	bool InitBattleFristDropConfig();
	///@brief ��ȡ��ͨ��������Ӣ����������˵�������ؾ�Ѱ�������Ĺؿ��������ñ�
	bool InitBattleDropConfig();
	///@brief ��ȡװ����������
	bool InitForgingConfig();
	///@brief ��ʼ��Ӣ��ĳһ���ȼ�����ľ����������Ϣ
	bool InitHeroExpConfig();
	bool InitHeroRankConfig();
	bool InitHeroStarConfig();
	///@brief ��ʼ�������������Ϣ
	bool InitMissionConfig();
	///@brief ��ʼ������Ļ�Ծ�Ƚ�������
	bool InitMissionActivenessRewardsConfig();
	bool InitAchievementConfig();
	bool InitRandomAchievementConfig();
	bool InitMasterLevelInfor();
	///@brief ��ʼ��ѵ��Ӣ�۵������ļ�
	bool InitHeroTrainingCfg();
	///@brief ��ʼ��Ӣ�۹һ�Ѱ���������ļ�
	bool InitHeroHuntingHuntingCfg();

	/**@brief ��ʼ����Ʒ����	*/
	bool _InitItemConfig();
	bool InitExpeditionConfig();
	/**@brief ��ʼ���̵����� ���̵����ý����ͬ ��ʹ��һ���ӿ�*/
	bool _InitShopCommodityConfig(const string& fileName, vector<CommodityConfig>& vecShopCommodity);
	///@brief  ��ʼ���������������ñ�
	bool _InitRobotNameConfig();
	bool InitCheckinAwardsConfig(); //ǩ������
	bool InitArenaRewardConfigList(); //��ʼ���������������ñ�
	
	///@brief ��ʼ�������������˵ȼ���Ϣ
	bool _InitArenaRobotLevelInfo();
	bool InitCheckinDay7AwardsConfig(); //7��ǩ������
	///@brief ��ʼ������������Ϣ����
	bool _InitGuideConfig();
	///@brief ��ʼ��Ե������
	bool _InitFateConfig();
	///@brief ��ʼ���佫��������
	bool _InitHeroSkillConfig();
	///@brief ��ʼ��buff����
	bool _InitBuffConfig();
	///@brief ��ʼ������ת������
	bool _InitResistanceRateCalculateConfig();

	///@breif ģ�黯�ض�ȡ����ս�����ֽ���������
	void _ReadChapterBattleScoreRewardsConfigModule(CTabTableReader& fileReader, string baseAttrTag, unordered_map<int, vector<GoodsInfoSG>>& rewardsInfor);
private:

	int m_iHeroTrainingCfgNums;
	int m_iHeroHungingHuntingNums;
	const string m_SplitSign = "|";
	GameFeatureActivationConfig m_gameFeatureActivationConfig;
	map<int32_t, HeroConfig> m_HeorConfigMap;
	unordered_map<int32_t, MonsterConfig> m_MonsterConfigs; ///����˵���ؾ�Ѱ������ͨ��������Ӣ�����Ĺ�����Ҫ������Ϣ
	vector<int32_t> m_vecAvaliableStrengthHeroList;	///���õ�����Ӣ���б�
	vector<int32_t> m_vecAvaliableManaHeroList;		///���õ�����Ӣ���б�
	vector<int32_t> m_vecAvaliableAgilityHeroList;	///���õ�����Ӣ���б�
	HeroTrainingCfg* m_arrayHeroTrainingCfg;///Ӣ��ѵ���������ļ�
	HeroHungingHuntingCfg* m_arrayHeroHungingHuntingCfg;
	map<int32_t, ItemConfig> m_ItemConfigMap;
	map<int32_t, EquipmentConfig> m_EquipmentMap;
	map<int32_t, int32_t> m_HeroExpConfig;///<Ӣ�۵�ĳһ�ȼ�����Ҫ�ľ�������,NOTE:keyֵΪ����ֵ��valueֵΪ��Ӧ�ĵȼ�
	vector<int32_t> m_HeroLevelNeededExp;///<Ӣ��ĳһ�ȼ���Ӧ������Ҫ�ľ���ֵ
	map<int32_t, ChapterConfig> m_ChapterConfigList;
	unordered_map<int32_t, ChapterBattleScoreRewardsConfig> m_ChapterBattleScoreRewardsConfig; ///<�½ڸ�����ս�����ֶ�Ӧ�Ľ���
	unordered_map<int32_t, ExchangeGoldConfig> m_ExchangeGoldConfigList;	///<��ʯ����Ǯ�����ñ�
	unordered_multimap<int32_t, int32_t> m_ExchangeGoldDiamondCostList;		///<��ʯ����Ǯ�ı�ʯ������Ϣ��keyֵΪ��ʯ�����ģ�valueֵΪ��Ӧ�Ķһ�����
	unordered_map<int32_t, ForgingConfig> m_ForgingConfigList;
	unordered_map<int32_t, MissionConfig> m_MissionConfigList;
	map<int32_t, MissionActivenessReswardsConfig> m_MissionActivenessRewardsConfig; ///<�����Ծ�ȶ�Ӧ�Ľ���������Ϣ��keyֵΪ�����Ծ�ȵ������ȡ״̬������ֵ
	unordered_map<MissionType, set<int>> m_MissionTypeList;
	map<int32_t, AchievementConfig> m_AchievementConfigList;	///<��������ɾ͵�������Ϣ
	unordered_map<int32_t, RandomAchievementConfig> m_RandomAchievementConfigList;	///<������ɵĿɴ�ɳɾ͵�������Ϣ
	map<int32_t, unordered_set<int32_t>> m_RandomAchievementAccessibleLevel;///<������ɵĿɴ�ɵĳɾ͵ĵȼ�������Ϣ��keyֵΪ���ŵĵȼ���valueֵΪ�ÿ��ŵȼ������ŵĳɾ�
	unordered_map<int32_t, set<int32_t>> m_AchievementGroupIDList;///<ÿ�ֳɾ���������Ӧ��ȫ���ĳɾ͵�ID��keyֵΪ�ɾ͵����ͣ�valueֵΪ����ɾ���������ȫ���ĳɾ�ID
	unordered_map<int32_t, map<int32_t, int32_t>> m_DuplicateAchievementTollgateIDList;///<ÿ�ָ����ɾ�����Ӧ��ȫ���ĸ����ؿ�ID�ı�keyֵΪ�����ɾ͵����ͣ� valueֵ��keyֵΪ����ɾ���������ȫ���Ĺؿ�ID��valueֵΪ��Ӧ�ĳɾ�ID 
	map<int32_t, vector<int32_t>> m_HeroRankUpgradeAchievementIDList;///<Ӣ�۽��׵ĳɾ�ID�б�,NOTE:KeyֵΪӢ�۶�Ӧ��Ʒ�ף�valueֵΪ��Ӧ�ĳɾ�ID
	unordered_map<int32_t, HeroStarConfig> m_HeroStarConfigMap;
	unordered_map<int32_t, HeroRankConfig> m_HeroRankConfigMap;
	unordered_map<int32_t, set<int32_t>> m_DuplicateTypeIDList;///<ĳһ�������͵ĸ���ID���б�keyֵΪ�������ͣ�valueֵΪ��Ӧ�ĸ����б�
	unordered_map<int32_t, BattleLevelConfig> m_BattleLevelConfigList;///<��ͨ��������Ӣ����������˵�������ؾ�Ѱ�������Ĺؿ����ñ�
	unordered_map<int32_t, BattleFirstDropConfig> m_BattleFirstDropConfigList;
	unordered_map<int32_t, BattleDropConfig> m_BattleDropConfigList; ///<��ͨ��������Ӣ����������˵�������ؾ�Ѱ�������Ĺؿ��������ñ�
	vector<MasterLevelInfor> m_MasterLevelInforList; ///<�����ȼ���Ϣ��
	map<int32_t, int32_t> m_MasterLevelUpExpTable;///<�����������辭���keyֵΪĳһ������ľ��飬valueֵΪ��Ӧ�ĵȼ�
	map<int32_t, map<int32_t, ExpeditionInstanceInfor>> m_ExpeditionInstanceInfor;///<Զ����������Ϣ��keyֵΪԶ���Ĺؿ�ID��valueֵ��map��keyֵΪ��С�Ŀ��ŵȼ�,valueֵ��map��valueֵΪ��Ӧ��Զ������
	vector<CommodityConfig> m_vecVarietyShopCommodity;///<�ӻ�����Ʒ������Ϣ
	vector<CommodityConfig> m_vecArenaShopCommodity;///<�������̵���Ʒ������Ϣ
	vector<CommodityConfig> m_vecExpeditionShopCommodity;///<Զ���̵���Ʒ������Ϣ
	vector<CommodityConfig> m_vecMiracleMerchantCommodity;///<��Ե�̵���Ʒ������Ϣ
	vector<CommodityConfig> m_vecGemMerchantCommodity;///<�䱦�̵���Ʒ������Ϣ
	vector<CommodityConfig> m_vecLegionShopCommodity;///<�����̵���Ʒ������Ϣ
	vector<CommodityConfig> m_vecSoulExchangeCommodity;///<����һ��̵��������Ϣ
	vector<CommodityConfig> m_vecWarOfLeagueShopCommodity;///<��ս�̵��������Ϣ
	vector<int32_t> m_MeleeHeroIDList; //��ս�佫ID��
	vector<int32_t> m_RangeHeroIDList;//Զ���佫ID��
	RobotNameConfig m_RobotNameConfig;
	map<int32_t, CheckInConfig>  m_CheckinAwardsConfigMap;//30��ǩ��������
	map<int,ArenaRewardConfig> m_ArenaRewardConfigList; //��������������
	vector< ArenaRobotLevelInfo> m_ArenaRobotLevelInfo; //�������������ȼ���Ϣ
	map<int32_t, CheckInday7Config>  m_CheckInday7AwardsConfigMap;//7��ǩ��������
	unordered_map<int32_t, NewbieGuideConfig> m_GuideConfigList;	///������Ϣ����
	unordered_map<int32_t, vector<FateConfig>> m_FateConfigMap;	///Ե������
	unordered_map<int32_t, HeroSkillConfig> m_HeroSkillConfigMap;	///< �佫��������
	unordered_map<int32_t, BuffConfig> m_BuffConfigMap;	///< buff����
	map<int32_t, float> m_ResistanceRateCalculateConfigMap;	///< ����ת������
};

