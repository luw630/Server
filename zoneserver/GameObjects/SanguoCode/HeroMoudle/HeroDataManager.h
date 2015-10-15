#pragma once
#include"..\Common\ConfigManager.h"
#include <memory>
#include <functional>

using namespace std;
enum HeroType;
struct SHeroData;
struct SHeroEquipment;

class CHeroDataManager
{
public:
	CHeroDataManager(SHeroData *pHeroArray);
	~CHeroDataManager();
	bool Init();
	bool Release();
	virtual bool AddHero(const int32_t heroID);
	///@brief 当防守阵容变了的时候
	virtual void OnArenaDefenseTeamChanged();

	bool ExistHero(const int32_t heorID) const;
	///@brief 设置竞技场防守阵容ID
	void SetArenaDefenseTeam(DWORD arenaDefenseTeamID[5]);
	///@brief 获取竞技场防守阵容ID
	const DWORD* GetArenaDefenseTeam() const { return m_pDefenseTeamID; }
	///@brief 重置玩家武将在远征副本中的血量、魔法值等信息
	void ResetExpedtiionInfor();
	///@brief 设置玩家武将在远征副本中的一些血量比值、魔法比值
	void SetExpeditionInfor(int32_t heroID, float healthScale, float manaScale);
	const SHeroData* GetExcellentHeroInfor();
	const SHeroData* GetHero(const int32_t heorID) const;
	///@brief 获得英雄的总数
	int GetHeroNum() const;
	///@brief 获取所有英雄的星级对应的总和
	int GetHeroStarSum() const;
	int GetHeroLevel(int heroID) const;
	///@brief 获取英雄某一个装备的信息
	const SHeroEquipment* GetHeroEquipment(int32_t heroID, int32_t equipID)const;
	///@brief 获取某一英雄的所有装备信息
	const SHeroEquipment * const GetHeroEquipments(int32_t heroID)const;
	///@brief 某一个装备是否被指定的英雄所装备
	bool IsHeroEquipmentAttached(int32_t heroID, int32_t equipID) const;
	///@brief 增加某一个英雄的某一件装备的锻造经验
	///@param starLevel 如果有升级,则把提升后的星级返回出来，如果没有升级的情况出现，则返回0
	///@return 成功返回增加的经验值，失败返回-1
	int IncreaseHeroEquipmentForgingExp(int32_t heroID, int32_t equipID, int32_t exp, OUT int32_t& starLevel);
	
	///@brief 获取竞技场防守阵容武将信息
	void GetArenaDefenseTeamInfo(SHeroData* pHeroDataList);
	
	///@brief 获取最强的5个武将
	void GetTop5Hero(SHeroData* pHeroDataList);

	virtual bool EquipHero(const int32_t heroID, const int32_t euquipID, const int32_t equipType);
	///@breif 给武将穿装备完了之后的操作，现在暂时主要用于当一次性穿多件装备的时候，服务器计算英雄属性的提升的优化
	void EquipHeroEndProcess(int32_t heroID);
	virtual bool HeroStarLevelRise(const int32_t heroID, const DWORD curStarLevel);
	void IncreaseHeroRank(int32_t heroID);
	bool CanRiseRank(const int32_t heroID) const;
	bool CanRiseStarLevel(const int32_t heroID) const;
	virtual bool UpgradeHeroSkill(const int32_t heroID, const int32_t skillID, int32_t skillLevel);
	bool MultiUpgradeHeroSkill(const int32_t heroID, const int32_t skillID, int32_t destLevel);
	bool MultiUpgradeHeroSkill(const int32_t heroID, DWORD skillIDs[], DWORD destLevels[], DWORD validNum);
	///@brief 获取英雄技能等级
	DWORD GetHeroSkillLevel(const int32_t heroID, const int32_t skillID) const;
	///@brief 获得所有英雄ID
	void GetAllHeroID(vector<int32_t>& vecID);
	///@brief 获取战斗力
	int GetCombatPower();
	///@brief 获取武将去除缘分属性后的属性
	bool GetHeroAttributeWithoutFateAttribute(int32_t heroID, OUT CharacterAttrData& attr) const;
	///@brief 更新与指定武将缘分相关的武将属性
	///@param [in] heroID 指定武将ID
	void UpdateRelatedHeroesAttribute(int32_t heroID);
	///@brief 能否计算缘分属性
	virtual bool CanCalculateFateAttribute();
	///@brief 更新所有武将的缘分属性
	void UpdateFateAttributeOfAllHeroes();
	///@brief 获取武将属性
	///param [in] heroID 指定武将ID
	const SHeroAttr* GetHeroAttr(int32_t heroID) const;
	///@brief 获取武将非零的缘分属性属性值
	///@param [in] heroID 指定获取的武将ID
	///@param [out] attrArray 待填充的属性数组
	///@param [in] arraySize 待填充的数组大小
	bool GetHeroFateAttribute(int32_t heroID, int attrArray[], const int arraySize) const;
protected:
	struct ExcellentHeroInfo
	{
		int32_t HeroID;
		int32_t CombatPower;
	};

	///计算了战斗力之后会做的事
	virtual void ProcessAfterCombatPowerCalculated(int heroID, int combatPower);
	///@brief 删除指定英雄的所有装备信息
	void ClearHeroEquipment(int heroID);
	///@brief 增加指定英雄的经验值，不支持增加 负数的经验值
	///@param heroID 要操作的英雄的ID
	///@param value[in] 要增加的经验值的数量
	///@param level[in] 提升的等级
	bool PlusHeroExp(int heroID, int value, int level);

	bool m_bDataInitialed;
	int32_t m_HeroNum;
	SHeroData *m_pHeroArray;
	DWORD *m_pDefenseTeamID;
	map<int32_t, int32_t, greater<int32_t>> m_ExcellentHeroInfor;///<key值为战斗力，value值为英雄ID,降序排列,存了最多五位英雄的战斗力
	unordered_map<int32_t, SHeroData* > m_heroList;
	unordered_map<int32_t, std::shared_ptr<SHeroAttr>> m_heroAttrlist;///英雄的属性值集合

private:
	SHeroData * _GetHero(const int32_t hero);
};


