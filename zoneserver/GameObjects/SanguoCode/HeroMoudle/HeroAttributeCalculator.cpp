#include "stdafx.h"
#include "..\NETWORKMODULE\SanguoPlayer.h"
#include "HeroAttributeCalculator.h"
#include "HeroDataManager.h"
#include "HeroFateInterrogator.h"
#include <memory>

CHeroAttributeCalculator::CHeroAttributeCalculator()
{
}


CHeroAttributeCalculator::~CHeroAttributeCalculator()
{
}

void CHeroAttributeCalculator::CalculateHeroLevelAttribute(int heroID, int heroLevel, int heroStarLevel, OUT CharacterAttrData& attr)
{
	/// 等级>1的话，计算成长值。
	int level = heroLevel - 1;
	if (level > 0)
	{
		//重置等级加成数据
		attr.Reset();

		/// 获取星级配置
		auto starConfig = CConfigManager::getSingleton()->GetHeroStarAttr(heroID);
		if (starConfig == nullptr)
			return;
		/// 获取当前星级的成长值
		heroStarLevel = min(CConfigManager::getSingleton()->GetHeroMaxStarLevel(heroID), heroStarLevel);
		auto growth = starConfig->GetHeroStarAttrGrowth(heroStarLevel);
		if (growth == nullptr)
			return;
		auto heroOriConfig = CConfigManager::getSingleton()->GetHeroConfig(heroID);
		if (heroOriConfig == nullptr)
			return;

		float MainAttrGrowth = 0;
		switch (heroOriConfig->Type)
		{
		case HeroType::Power:
			MainAttrGrowth = growth->PowerGrowth1;
			break;
		case HeroType::Intelligence:
			MainAttrGrowth = growth->IntelligenceGrowth;
			break;
		case HeroType::Agility:
			MainAttrGrowth = growth->AgilityGrowth;
			break;
		}

		/// 力量 = 力量成长值 * （LV - 1) + 初始力量
		attr.Power += growth->PowerGrowth1 * level;
		/// 智力 = 智力成长值 * （LV - 1) + 初始智力
		attr.Intelligence += growth->IntelligenceGrowth * level;
		/// 敏捷 = 敏捷成长值 * （LV - 1) + 初始敏捷
		attr.Agility += growth->AgilityGrowth * level;
		/// 最大生命值 = 力量成长值 * 18 *（LV-1）+ 初始最大生命
		attr.MaxHealth += growth->PowerGrowth1 * 18.0f * level;
		/// 物理攻击 = （主属性成长值 + 敏捷成长值 * 0.4）*（LV - 1）+ 初始物攻
		attr.Physic += (MainAttrGrowth + growth->AgilityGrowth * 0.4f) * level;
		/// 绝技强度 = 智力成长值 * 1.5 *（LV - 1）+ 初始绝技强度
		attr.Magic += growth->IntelligenceGrowth * 1.5f * level;
		/// 物理护甲 = (力量成长 / 2 + 敏捷成长 / 4）*（LV - 1）+ 初始物理护甲
		attr.Armor += (growth->PowerGrowth1 / 2.0f + growth->AgilityGrowth / 4.0f) * level;
		/// 绝技抵抗 = 智力成长 / 2 *（LV - 1）+ 初始绝技抵抗
		attr.Resistance += (growth->IntelligenceGrowth / 2.0f) * level;
		/// 物理暴击 = 敏捷成长 / 2.5 *（LV - 1）+ 初始物暴
		attr.Crit += (growth->AgilityGrowth / 2.5f) * level;
	}
}

void CHeroAttributeCalculator::CalculateHeroEquipmentAttribute(int heroID, const SHeroEquipment* equipment, OUT CharacterAttrData& attr)
{
	auto heroOriConfig = CConfigManager::getSingleton()->GetHeroConfig(heroID);
	if (heroOriConfig == nullptr || equipment == nullptr)
		return;

	attr.Reset();
	CharacterAttrData tempAttr;
	/// 遍历装备计算属性
	for (int i = 0; i < 6; ++i)
	{
		if (equipment[i].m_dwEquipID == 0)
			continue;

		tempAttr.Reset();
		/// 计算装备当前属性
		CalculateItemEquipmentAttribute(tempAttr, equipment[i]);
		/// 累加装备当前属性
		attr += tempAttr;
	}

	/// 根据力量、智力和敏捷进行属性加成
	CalculateAttributeEffect(attr, (HeroType)heroOriConfig->Type);
}

void CHeroAttributeCalculator::CalculateHeroRankAttribute(int heroID, int rankGrade, OUT CharacterAttrData& attr)
{
	auto heroOriConfig = CConfigManager::getSingleton()->GetHeroConfig(heroID);
	if (heroOriConfig == nullptr)
		return;

	/// 如果品阶大于1，则计算品阶加成。
	if (rankGrade > 1)
	{
		attr.Reset();
		/// 查询武将品阶配置
		auto rankConfig = CConfigManager::getSingleton()->GetHeroRankConfig(heroID);
		if (rankConfig == nullptr)
			return;

		int targetRank = rankGrade + 1;
		targetRank = min(CConfigManager::getSingleton()->GetHeroMaxRankGrade(heroID), targetRank);
		int equipLength = 0;
		int equipID = 0;
		/// 遍历所有品阶，累积属性。
		for (int rank = 2; rank < targetRank; ++rank)
		{
			/// 查询当前品阶配置
			auto rankData = rankConfig->GetRankData(rank);

			/// 累计进阶属性
			attr.Power += rankData->m_Power;
			attr.Intelligence += rankData->m_Intelligence;
			attr.Agility += rankData->m_Agility;

			equipLength = rankData->m_EquipIDs.size();
			/// 遍历当前品阶所有装备, 累计装备属性
			for (int i = 0; i < equipLength; ++i)
			{
				/// 获取装备ID
				equipID = rankData->m_EquipIDs[i];
				/// 读取装备属性
				auto equipConfig = CConfigManager::getSingleton()->GetEquipment(equipID);
				/// 累积装备属性
				attr += equipConfig->baseAttribute;
			}
		}

		/// 根据力量、智力和敏捷进行属性加成
		CalculateAttributeEffect(attr, (HeroType)heroOriConfig->Type);
	}
}

void CHeroAttributeCalculator::CalculateItemEquipmentAttribute(OUT CharacterAttrData& attr, const SHeroEquipment& equipment)
{
	auto config = CConfigManager::getSingleton()->GetEquipment(equipment.m_dwEquipID);
	if (config == nullptr)
		return;

	attr = config->baseAttribute;
	/// 如果装备精炼过
	if (equipment.m_dwStarLevel > 0)
	{
		auto config = CConfigManager::getSingleton()->GetForgingConfig(equipment.m_dwEquipID);
		if (config == nullptr)
			return;

		CharacterAttrData forgingAttr = config->baseAttribute;
		forgingAttr *= (float)equipment.m_dwStarLevel;
		/// 累加精炼属性
		attr += forgingAttr;
	}
}

void CHeroAttributeCalculator::CalculateAttributeEffect(OUT CharacterAttrData& attribute, HeroType mainAttribute)
{
	/// 最大生命值 = 基础最大生命值 + 力量 × 18
	attribute.MaxHealth += attribute.Power * 18.0f;
	/// 物理攻击 = 主属性类型 + 敏捷 × 0.4
	switch (mainAttribute)
	{
	case HeroType::Power:
		attribute.Physic += attribute.Power;
		break;
	case HeroType::Intelligence:
		attribute.Physic += attribute.Intelligence;
		break;
	case HeroType::Agility:
		attribute.Physic += attribute.Agility;
		break;
	}
	attribute.Physic += attribute.Agility * 0.4f;
	/// 绝技强度 = 智力 × 1.5
	attribute.Magic += attribute.Intelligence * 1.5f;
	/// 物理护甲 = 力量 / 2 + 敏捷 / 4
	attribute.Armor += attribute.Power / 2.0f + attribute.Agility / 4.0f;
	/// 绝技抵抗 = 智力 / 2
	attribute.Resistance += attribute.Intelligence / 2.0f;
	/// 物理暴击 = 敏捷 / 2.5
	attribute.Crit += attribute.Agility / 2.5f;
}

int CHeroAttributeCalculator::CalculateCombatPower(const SHeroSkillInfo* heroSkillData, SHeroAttr* pHeroAttr)
{
	if (heroSkillData == nullptr || pHeroAttr == nullptr)
		return 0;

	const CharacterAttrData& runtimeAttr = pHeroAttr->curAttr;

	//战斗力 = 力量*1 + 智力*1 + 敏捷*1+ 物理攻击*0.5 + 物理防御*2 + 能量回复*0.17 + 生命回复*0.0625 + 魔法强度*0.33 + 魔法抗性*0.5
	//+ 最大生命*0.04 + 物理暴击*1.3 + 闪避*4 + 吸血*0.6 + 穿透物理护甲*6 + 忽略法术抗性*3.5
	pHeroAttr->CombatPower = (int)(runtimeAttr.Power * 1.0f +
		runtimeAttr.Intelligence * 1.0f +
		runtimeAttr.Agility * 1.0f +
		runtimeAttr.Physic * 0.5f +
		runtimeAttr.Armor * 2.0f +
		runtimeAttr.EnergyRecovery * 0.17f +
		runtimeAttr.HealthRecovery * 0.0625f +
		runtimeAttr.Magic * 0.33f +
		runtimeAttr.Resistance * 0.5f +
		runtimeAttr.MaxHealth * 0.04f +
		runtimeAttr.Crit * 1.3f +
		runtimeAttr.Dodge * 4.0f +
		runtimeAttr.BloodLevel * 0.6f +
		runtimeAttr.OffsetArmor * 6.0f +
		runtimeAttr.OffsetResistance * 3.5f);

	//技能所加战斗力 = (每个技能等级 - 1) * 4
	for (int i = 0; i < MAX_SKILLNUM; ++i)
	{
		pHeroAttr->CombatPower += (heroSkillData[i].m_dwSkillLevel - 1) * 4;
	}

	return pHeroAttr->CombatPower;
}

int CHeroAttributeCalculator::CalculateSpecifyHeroComabtPower(const SHeroData* heroData, const int(&fateAttrArray)[4])
{
	if (heroData == nullptr)
		return 0;
	auto pHeroConfig = CConfigManager::getSingleton()->GetHeroConfig(heroData->m_dwHeroID);
	if (nullptr == pHeroConfig)
		return 0;

	shared_ptr<SHeroAttr> attrData = make_shared<SHeroAttr>(pHeroConfig->baseAttribute);
	/// 计算等级加成属性
	CalculateHeroLevelAttribute(heroData->m_dwHeroID, heroData->m_dwLevel, heroData->m_dwStarLevel, attrData->LevelAttribute);
	/// 计算装备加成属性
	CalculateHeroEquipmentAttribute(heroData->m_dwHeroID, heroData->m_Equipments, attrData->EquipmentAttribute);
	/// 计算品阶加成属性
	CalculateHeroRankAttribute(heroData->m_dwHeroID, heroData->m_dwRankLevel, attrData->RankAttribute);
	/// 转换缘分属性
	ParseHeroFateAttribute(fateAttrArray, attrData->FateAttribute);
	/// 合计属性
	attrData->curAttr += attrData->LevelAttribute;
	attrData->curAttr += attrData->EquipmentAttribute;
	attrData->curAttr += attrData->RankAttribute;
	attrData->curAttr += attrData->FateAttribute;

	return CalculateCombatPower(heroData->m_SkillInfoList, attrData.get());
}

void CHeroAttributeCalculator::CalculateHeroFateAttribute(int heroID, OUT CharacterAttrData& attr, const CHeroDataManager* pHeroDataManager)
{
	if (nullptr == pHeroDataManager)
		return;

	///< 获取武将配置 用以判断武将类型
	auto heroOriConfig = CConfigManager::getSingleton()->GetHeroConfig(heroID);
	if (nullptr == heroOriConfig)
		return;

	vector<FateConfig*> vecfateConfigs;
	///< 获取武将已激活的缘分配置
	CHeroFateInterrogator::getSingleton()->GetActivatedFateConfigs(heroID, pHeroDataManager, vecfateConfigs);
	if (vecfateConfigs.empty())
		return;

	attr.Reset();
	CharacterAttrData finalAttr;
	float percent;
	for (auto & config : vecfateConfigs)
	{
		for (int i = 0, count = config->vecRelationshipHero.size(); i < count; ++i)
		{
			///< 获取缘分武将移除缘分属性后的当前属性
			if (!pHeroDataManager->GetHeroAttributeWithoutFateAttribute(config->vecRelationshipHero[i], finalAttr))
				continue;
			///< 缘分配置中相应加成属性类型的加成百分比值
			percent = config->vecAttributePercent[i];
			///< 匹配缘分配置中相应加成属性类型
			switch (config->vecAttributeType[i])
			{
			///< 最大生命值
			case 1:
				attr.MaxHealth += percent * finalAttr.MaxHealth;
				break;
			///< 物理攻击力
			case 2:
				attr.Physic += percent * finalAttr.Physic;
				break;
			///< 魔法强度
			case 3:
				attr.Magic += percent * finalAttr.Magic;
				break;
			///< 物理护甲
			case 4:
				attr.Armor += percent * finalAttr.Armor;
				break;
			///< 法术抗性
			case 5:
				attr.Resistance += percent * finalAttr.Resistance;
				break;
			///< 暴击
			case 6:
				attr.Crit += percent * finalAttr.Crit;
				break;
			///< 生命回复
			case 7:
				attr.HealthRecovery = percent * finalAttr.HealthRecovery;
				break;
			///< 能量回复
			case 8:
				attr.EnergyRecovery = percent * finalAttr.EnergyRecovery;
				break;
			default:
				break;
			}
		}
	}

	///< 计算属性加成效果
	CalculateAttributeEffect(attr, (HeroType)heroOriConfig->Type);
}

void CHeroAttributeCalculator::ParseHeroFateAttribute(const int(&attrArray)[4], OUT CharacterAttrData& fateAttr)
{
	for (int i = 0; i < 4; ++i)
	{
		switch (attrArray[i] / 1000000)
		{
		case 1:	/// 最大生命值
			fateAttr.MaxHealth = attrArray[i] % 1000000;
			break;
		case 2:	/// 物理攻击力
			fateAttr.Physic = attrArray[i] % 1000000;
			break;
		case 3:	/// 魔法强度
			fateAttr.Magic = attrArray[i] % 1000000;
			break;
		case 4: /// 物理护甲
			fateAttr.Armor = attrArray[i] % 1000000;
			break;
		case 5:	/// 法术抗性
			fateAttr.Resistance = attrArray[i] % 1000000;
			break;
		case 6:	/// 暴击
			fateAttr.Crit = attrArray[i] % 1000000;
			break;
		case 7: /// 生命回复
			fateAttr.HealthRecovery = attrArray[i] % 1000000;
			break;
		case 8:	/// 能量回复
			fateAttr.EnergyRecovery = attrArray[i] % 1000000;
			break;
		}
	}
}
