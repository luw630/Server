#pragma once
#define MAX_SKILLNUM 4
const int MAX_FRIEND_NUMBER = 100;			//好友

enum EQUIP_ENUM
{
	EQUIP_WEAPON = 0,	 // 武器
	EQUIP_SHOSE,		// 头盔
	EQUIP_PAT,				// 裤子
	EQUIP_WAIST,		// 肩膀
	EQUIP_WRIST,		// 护腕
	EQUIP_CLOTH,		// 衣服

	EQUIP_MAX,				// Max
};

///@brief 武将技能
struct SHeroSkillInfo
{
	DWORD m_dwSkillID;
	DWORD m_dwSkillLevel;
	SHeroSkillInfo()
	{
		m_dwSkillID = 0;
		m_dwSkillLevel = 0;
	}
};

///@brief 武将装备
struct SHeroEquipment
{
	DWORD m_dwEquipID; //装备ID
	DWORD m_dwUpgradeExp; //装备升星经验
	DWORD m_dwStarLevel; //装备星级
	
	SHeroEquipment()
	{
		m_dwEquipID = 0;
		m_dwStarLevel = 0;
		m_dwUpgradeExp = 0;
	}
};

/// <summary>
/// 角色属性数据
/// </summary>
struct CharacterAttrData
{
	/// <summary>
	/// 力量	
	/// </summary>
	float Power;
	/// <summary>
	/// 初始敏捷
	/// </summary>
	float Agility;
	/// <summary>
	/// 智力
	/// </summary>
	float Intelligence;
	/// <summary>
	/// 最大生命值
	/// </summary>
	float MaxHealth;
	/// <summary>
	/// 物理攻击强度
	/// </summary>
	float Physic;
	/// <summary>
	/// 法术攻击强度
	/// </summary>
	float Magic;
	/// <summary>
	/// 物理护甲
	/// </summary>
	float Armor;
	/// <summary>
	/// 法术抗性
	/// </summary>
	float Resistance;
	/// <summary>
	/// 物理暴击
	/// </summary>
	float Crit;
	/// <summary>
	/// 生命回复
	/// </summary>
	float HealthRecovery;
	/// <summary>
	/// 能量回复
	/// </summary>
	float EnergyRecovery;
	/// <summary>
	/// 吸血等级
	/// </summary>
	float BloodLevel;
	/// <summary>
	/// 穿透物理护甲
	/// </summary>
	float OffsetArmor;
	/// <summary>
	/// 忽略法术抗性
	/// </summary>
	float OffsetResistance;
	/// <summary>
	/// 治疗效果提升
	/// </summary>
	float AddCure;
	/// <summary>
	/// 闪避
	/// </summary>
	float Dodge;

	CharacterAttrData()
	{
		Power = 0;
		Agility = 0;
		Intelligence = 0;
		MaxHealth = 0;
		Physic = 0;
		Magic = 0;
		Armor = 0;
		Resistance = 0;
		Crit = 0;
		HealthRecovery = 0;
		EnergyRecovery = 0;
		BloodLevel = 0;
		OffsetArmor = 0;
		OffsetResistance = 0;
		AddCure = 0;
		Dodge = 0;
	}

	CharacterAttrData(const CharacterAttrData& other)
	{
		Power = other.Power;
		Agility = other.Agility;
		Intelligence = other.Intelligence;
		MaxHealth = other.MaxHealth;
		Physic = other.Physic;
		Magic = other.Magic;
		Armor = other.Armor;
		Resistance = other.Resistance;
		Crit = other.Crit;
		HealthRecovery = other.HealthRecovery;
		EnergyRecovery = other.EnergyRecovery;
		BloodLevel = other.BloodLevel;
		OffsetArmor = other.OffsetArmor;
		OffsetResistance = other.OffsetResistance;
		AddCure = other.AddCure;
		Dodge = other.Dodge;
	}

	CharacterAttrData(CharacterAttrData&& other)
	{
		Power = other.Power;
		Agility = other.Agility;
		Intelligence = other.Intelligence;
		MaxHealth = other.MaxHealth;
		Physic = other.Physic;
		Magic = other.Magic;
		Armor = other.Armor;
		Resistance = other.Resistance;
		Crit = other.Crit;
		HealthRecovery = other.HealthRecovery;
		EnergyRecovery = other.EnergyRecovery;
		BloodLevel = other.BloodLevel;
		OffsetArmor = other.OffsetArmor;
		OffsetResistance = other.OffsetResistance;
		AddCure = other.AddCure;
		Dodge = other.Dodge;
	}

	CharacterAttrData& operator = (CharacterAttrData&& other)
	{
		if (this != &other)
		{
			Power = other.Power;
			Agility = other.Agility;
			Intelligence = other.Intelligence;
			MaxHealth = other.MaxHealth;
			Physic = other.Physic;
			Magic = other.Magic;
			Armor = other.Armor;
			Resistance = other.Resistance;
			Crit = other.Crit;
			HealthRecovery = other.HealthRecovery;
			EnergyRecovery = other.EnergyRecovery;
			BloodLevel = other.BloodLevel;
			OffsetArmor = other.OffsetArmor;
			OffsetResistance = other.OffsetResistance;
			AddCure = other.AddCure;
			Dodge = other.Dodge;
		}

		return *this;
	}

	CharacterAttrData& operator + (const CharacterAttrData& other)
	{
		Power += other.Power;
		Agility += other.Agility;
		Intelligence += other.Intelligence;
		MaxHealth += other.MaxHealth;
		Physic += other.Physic;
		Magic += other.Magic;
		Armor += other.Armor;
		Resistance += other.Resistance;
		Crit += other.Crit;
		HealthRecovery += other.HealthRecovery;
		EnergyRecovery += other.EnergyRecovery;
		BloodLevel += other.BloodLevel;
		OffsetArmor += other.OffsetArmor;
		OffsetResistance += other.OffsetResistance;
		AddCure += other.AddCure;
		Dodge += other.Dodge;

		return *this;
	}

	void operator += (const CharacterAttrData& other)
	{
		Power += other.Power;
		Agility += other.Agility;
		Intelligence += other.Intelligence;
		MaxHealth += other.MaxHealth;
		Physic += other.Physic;
		Magic += other.Magic;
		Armor += other.Armor;
		Resistance += other.Resistance;
		Crit += other.Crit;
		HealthRecovery += other.HealthRecovery;
		EnergyRecovery += other.EnergyRecovery;
		BloodLevel += other.BloodLevel;
		OffsetArmor += other.OffsetArmor;
		OffsetResistance += other.OffsetResistance;
		AddCure += other.AddCure;
		Dodge += other.Dodge;
	}

	CharacterAttrData& operator - (const CharacterAttrData& other)
	{
		AddCure -= other.AddCure;
		Agility -= other.Agility;
		Armor -= other.Armor;
		Physic -= other.Physic;
		BloodLevel -= other.BloodLevel;
		Crit -= other.Crit;
		Dodge -= other.Dodge;
		EnergyRecovery -= other.EnergyRecovery;
		HealthRecovery -= other.HealthRecovery;
		Intelligence -= other.Intelligence;
		Magic -= other.Magic;
		MaxHealth -= other.MaxHealth;
		OffsetArmor -= other.OffsetArmor;
		OffsetResistance -= other.OffsetResistance;
		Power -= other.Power;
		Resistance -= other.Resistance;

		return *this;
	}

	void operator -= (const CharacterAttrData& other)
	{
		AddCure -= other.AddCure;
		Agility -= other.Agility;
		Armor -= other.Armor;
		Physic -= other.Physic;
		BloodLevel -= other.BloodLevel;
		Crit -= other.Crit;
		Dodge -= other.Dodge;
		EnergyRecovery -= other.EnergyRecovery;
		HealthRecovery -= other.HealthRecovery;
		Intelligence -= other.Intelligence;
		Magic -= other.Magic;
		MaxHealth -= other.MaxHealth;
		OffsetArmor -= other.OffsetArmor;
		OffsetResistance -= other.OffsetResistance;
		Power -= other.Power;
		Resistance -= other.Resistance;
	}

	CharacterAttrData& operator * (float scale)
	{
		Power *= scale;
		Agility *= scale;
		Intelligence *= scale;
		MaxHealth *= scale;
		Physic *= scale;
		Magic *= scale;
		Armor *= scale;
		Resistance *= scale;
		Crit *= scale;
		HealthRecovery *= scale;
		EnergyRecovery *= scale;
		BloodLevel *= scale;
		OffsetArmor *= scale;
		OffsetResistance *= scale;
		AddCure *= scale;
		Dodge *= scale;

		return *this;
	}

	void operator *= (float scale)
	{
		Power *= scale;
		Agility *= scale;
		Intelligence *= scale;
		MaxHealth *= scale;
		Physic *= scale;
		Magic *= scale;
		Armor *= scale;
		Resistance *= scale;
		Crit *= scale;
		HealthRecovery *= scale;
		EnergyRecovery *= scale;
		BloodLevel *= scale;
		OffsetArmor *= scale;
		OffsetResistance *= scale;
		AddCure *= scale;
		Dodge *= scale;
	}

	void Reset()
	{
		Power = 0;
		Agility = 0;
		Intelligence = 0;
		MaxHealth = 0;
		Physic = 0;
		Magic = 0;
		Armor = 0;
		Resistance = 0;
		Crit = 0;
		HealthRecovery = 0;
		EnergyRecovery = 0;
		BloodLevel = 0;
		OffsetArmor = 0;
		OffsetResistance = 0;
		AddCure = 0;
		Dodge = 0;
	}

	/// <summary>
	/// 验证必要属性值是否为0
	/// </summary>
	void ValidateAttrData()
	{
		Power = max(1, Power);
		Agility = max(1, Agility);
		Intelligence = max(1, Intelligence);
		MaxHealth = max(1, MaxHealth);
	}

	void Difference(const CharacterAttrData& opData, CharacterAttrData& result)
	{
		result.AddCure = AddCure - opData.AddCure;
		result.Agility = Agility - opData.Agility;
		result.Armor = Armor - opData.Armor;
		result.Physic = Physic - opData.Physic;
		result.BloodLevel = BloodLevel - opData.BloodLevel;
		result.Crit = Crit - opData.Crit;
		result.Dodge = Dodge - opData.Dodge;
		result.EnergyRecovery = EnergyRecovery - opData.EnergyRecovery;
		result.HealthRecovery = HealthRecovery - opData.HealthRecovery;
		result.Intelligence = Intelligence - opData.Intelligence;
		result.Magic = Magic - opData.Magic;
		result.MaxHealth = MaxHealth - opData.MaxHealth;
		result.OffsetArmor = OffsetArmor - opData.OffsetArmor;
		result.OffsetResistance = OffsetResistance - opData.OffsetResistance;
		result.Power = Power - opData.Power;
		result.Resistance = Resistance - opData.Resistance;
	}
};

struct MonsterData
{
	int monsterID;
	float attackCD;
	float attackRange;
	float Power;
	float Agility;
	float Intelligence;
	float MaxHealth;
	float Physic;
	float Magic;
	float Armor;
	float Resistance;
	float Crit;
	float HealthRecovery;
	float EnergyRecovery;
	float BloodLevel;
	float OffsetArmor;
	float OffsetResistance;
	float AddCure;
	float Dodge;
};

///@brief 英雄的属性值集合
struct SHeroAttr
{
	int CombatPower;
	CharacterAttrData curAttr;
	CharacterAttrData LevelAttribute;
	CharacterAttrData EquipmentAttribute;
	CharacterAttrData RankAttribute;
	CharacterAttrData FateAttribute;

	SHeroAttr()
	{
		CombatPower = 0;
		curAttr.Reset();
		LevelAttribute.Reset();
		EquipmentAttribute.Reset();
		RankAttribute.Reset();
		FateAttribute.Reset();
	}

	SHeroAttr(const CharacterAttrData& other)
	{
		CombatPower = 0;
		curAttr = other;
		LevelAttribute.Reset();
		EquipmentAttribute.Reset();
		RankAttribute.Reset();
		FateAttribute.Reset();
	}

	SHeroAttr(SHeroAttr&& other)
		:curAttr(std::move(other.curAttr))
		, LevelAttribute(std::move(other.LevelAttribute))
		, EquipmentAttribute(std::move(other.EquipmentAttribute))
		, RankAttribute(std::move(other.RankAttribute))
		, FateAttribute(std::move(other.FateAttribute))
	{
		CombatPower = other.CombatPower;
	}

	SHeroAttr& operator = (SHeroAttr&& other)
	{
		if (this != &other)
		{
			CombatPower = other.CombatPower;
			curAttr = std::move(other.curAttr);
			LevelAttribute = std::move(other.LevelAttribute);
			EquipmentAttribute = std::move(other.EquipmentAttribute);
			RankAttribute = std::move(other.RankAttribute);
			FateAttribute = std::move(other.FateAttribute);
		}

		return *this;
	}
};

///@brief struct 武将数据
struct SHeroData
{
	SHeroEquipment m_Equipments[EQUIP_MAX];///<装备表
	SHeroSkillInfo m_SkillInfoList[MAX_SKILLNUM]; ///<技能表
	BYTE m_bDeadInExpedition;///<在远征副本中已经死亡
	DWORD m_Exp; ///<武将经验
	DWORD m_dwHeroID; ///<武将ID
	DWORD m_dwLevel; ///<等级
	DWORD m_dwRankLevel; ///<武将品阶
	DWORD m_dwStarLevel; ///<武将星级
	float m_fExpeditionHealthScale;///<远征副本下英雄的血量槽比例值值
	float m_fExpeditionManaScale;///<远征副本下英雄的能连槽比例值

	SHeroData()
	{
		m_dwHeroID = 0;
		m_dwLevel = 0;
		m_dwRankLevel = 0;
		m_dwStarLevel = 0;
		m_Exp = 0;
		m_fExpeditionHealthScale = 0;
		m_fExpeditionManaScale = 0;
		m_bDeadInExpedition = true;
	}
};

// 好友，黑名单
struct SPlayerRelates
{
	char		szName[CONST_USERNAME];
	char		sHeadIcon[CONST_USERNAME];
	BYTE		byRelation : 4;		// 关系
	BYTE		wLevle;					// 等级                                                              
};

struct SPlayerRelation
{
	SPlayerRelates m_PlayerFriends[MAX_FRIEND_NUMBER];	// 好友
};

struct SHeroFateAttr
{
	int fateAttr[4];
};