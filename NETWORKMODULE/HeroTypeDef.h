#pragma once
#define MAX_SKILLNUM 4
const int MAX_FRIEND_NUMBER = 100;			//����

enum EQUIP_ENUM
{
	EQUIP_WEAPON = 0,	 // ����
	EQUIP_SHOSE,		// ͷ��
	EQUIP_PAT,				// ����
	EQUIP_WAIST,		// ���
	EQUIP_WRIST,		// ����
	EQUIP_CLOTH,		// �·�

	EQUIP_MAX,				// Max
};

///@brief �佫����
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

///@brief �佫װ��
struct SHeroEquipment
{
	DWORD m_dwEquipID; //װ��ID
	DWORD m_dwUpgradeExp; //װ�����Ǿ���
	DWORD m_dwStarLevel; //װ���Ǽ�
	
	SHeroEquipment()
	{
		m_dwEquipID = 0;
		m_dwStarLevel = 0;
		m_dwUpgradeExp = 0;
	}
};

/// <summary>
/// ��ɫ��������
/// </summary>
struct CharacterAttrData
{
	/// <summary>
	/// ����	
	/// </summary>
	float Power;
	/// <summary>
	/// ��ʼ����
	/// </summary>
	float Agility;
	/// <summary>
	/// ����
	/// </summary>
	float Intelligence;
	/// <summary>
	/// �������ֵ
	/// </summary>
	float MaxHealth;
	/// <summary>
	/// ������ǿ��
	/// </summary>
	float Physic;
	/// <summary>
	/// ��������ǿ��
	/// </summary>
	float Magic;
	/// <summary>
	/// ������
	/// </summary>
	float Armor;
	/// <summary>
	/// ��������
	/// </summary>
	float Resistance;
	/// <summary>
	/// ������
	/// </summary>
	float Crit;
	/// <summary>
	/// �����ظ�
	/// </summary>
	float HealthRecovery;
	/// <summary>
	/// �����ظ�
	/// </summary>
	float EnergyRecovery;
	/// <summary>
	/// ��Ѫ�ȼ�
	/// </summary>
	float BloodLevel;
	/// <summary>
	/// ��͸������
	/// </summary>
	float OffsetArmor;
	/// <summary>
	/// ���Է�������
	/// </summary>
	float OffsetResistance;
	/// <summary>
	/// ����Ч������
	/// </summary>
	float AddCure;
	/// <summary>
	/// ����
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
	/// ��֤��Ҫ����ֵ�Ƿ�Ϊ0
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

///@brief Ӣ�۵�����ֵ����
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

///@brief struct �佫����
struct SHeroData
{
	SHeroEquipment m_Equipments[EQUIP_MAX];///<װ����
	SHeroSkillInfo m_SkillInfoList[MAX_SKILLNUM]; ///<���ܱ�
	BYTE m_bDeadInExpedition;///<��Զ���������Ѿ�����
	DWORD m_Exp; ///<�佫����
	DWORD m_dwHeroID; ///<�佫ID
	DWORD m_dwLevel; ///<�ȼ�
	DWORD m_dwRankLevel; ///<�佫Ʒ��
	DWORD m_dwStarLevel; ///<�佫�Ǽ�
	float m_fExpeditionHealthScale;///<Զ��������Ӣ�۵�Ѫ���۱���ֵֵ
	float m_fExpeditionManaScale;///<Զ��������Ӣ�۵������۱���ֵ

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

// ���ѣ�������
struct SPlayerRelates
{
	char		szName[CONST_USERNAME];
	char		sHeadIcon[CONST_USERNAME];
	BYTE		byRelation : 4;		// ��ϵ
	BYTE		wLevle;					// �ȼ�                                                              
};

struct SPlayerRelation
{
	SPlayerRelates m_PlayerFriends[MAX_FRIEND_NUMBER];	// ����
};

struct SHeroFateAttr
{
	int fateAttr[4];
};