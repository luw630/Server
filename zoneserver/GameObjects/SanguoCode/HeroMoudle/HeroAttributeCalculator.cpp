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
	/// �ȼ�>1�Ļ�������ɳ�ֵ��
	int level = heroLevel - 1;
	if (level > 0)
	{
		//���õȼ��ӳ�����
		attr.Reset();

		/// ��ȡ�Ǽ�����
		auto starConfig = CConfigManager::getSingleton()->GetHeroStarAttr(heroID);
		if (starConfig == nullptr)
			return;
		/// ��ȡ��ǰ�Ǽ��ĳɳ�ֵ
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

		/// ���� = �����ɳ�ֵ * ��LV - 1) + ��ʼ����
		attr.Power += growth->PowerGrowth1 * level;
		/// ���� = �����ɳ�ֵ * ��LV - 1) + ��ʼ����
		attr.Intelligence += growth->IntelligenceGrowth * level;
		/// ���� = ���ݳɳ�ֵ * ��LV - 1) + ��ʼ����
		attr.Agility += growth->AgilityGrowth * level;
		/// �������ֵ = �����ɳ�ֵ * 18 *��LV-1��+ ��ʼ�������
		attr.MaxHealth += growth->PowerGrowth1 * 18.0f * level;
		/// ������ = �������Գɳ�ֵ + ���ݳɳ�ֵ * 0.4��*��LV - 1��+ ��ʼ�﹥
		attr.Physic += (MainAttrGrowth + growth->AgilityGrowth * 0.4f) * level;
		/// ����ǿ�� = �����ɳ�ֵ * 1.5 *��LV - 1��+ ��ʼ����ǿ��
		attr.Magic += growth->IntelligenceGrowth * 1.5f * level;
		/// ������ = (�����ɳ� / 2 + ���ݳɳ� / 4��*��LV - 1��+ ��ʼ������
		attr.Armor += (growth->PowerGrowth1 / 2.0f + growth->AgilityGrowth / 4.0f) * level;
		/// �����ֿ� = �����ɳ� / 2 *��LV - 1��+ ��ʼ�����ֿ�
		attr.Resistance += (growth->IntelligenceGrowth / 2.0f) * level;
		/// ������ = ���ݳɳ� / 2.5 *��LV - 1��+ ��ʼ�ﱩ
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
	/// ����װ����������
	for (int i = 0; i < 6; ++i)
	{
		if (equipment[i].m_dwEquipID == 0)
			continue;

		tempAttr.Reset();
		/// ����װ����ǰ����
		CalculateItemEquipmentAttribute(tempAttr, equipment[i]);
		/// �ۼ�װ����ǰ����
		attr += tempAttr;
	}

	/// �������������������ݽ������Լӳ�
	CalculateAttributeEffect(attr, (HeroType)heroOriConfig->Type);
}

void CHeroAttributeCalculator::CalculateHeroRankAttribute(int heroID, int rankGrade, OUT CharacterAttrData& attr)
{
	auto heroOriConfig = CConfigManager::getSingleton()->GetHeroConfig(heroID);
	if (heroOriConfig == nullptr)
		return;

	/// ���Ʒ�״���1�������Ʒ�׼ӳɡ�
	if (rankGrade > 1)
	{
		attr.Reset();
		/// ��ѯ�佫Ʒ������
		auto rankConfig = CConfigManager::getSingleton()->GetHeroRankConfig(heroID);
		if (rankConfig == nullptr)
			return;

		int targetRank = rankGrade + 1;
		targetRank = min(CConfigManager::getSingleton()->GetHeroMaxRankGrade(heroID), targetRank);
		int equipLength = 0;
		int equipID = 0;
		/// ��������Ʒ�ף��ۻ����ԡ�
		for (int rank = 2; rank < targetRank; ++rank)
		{
			/// ��ѯ��ǰƷ������
			auto rankData = rankConfig->GetRankData(rank);

			/// �ۼƽ�������
			attr.Power += rankData->m_Power;
			attr.Intelligence += rankData->m_Intelligence;
			attr.Agility += rankData->m_Agility;

			equipLength = rankData->m_EquipIDs.size();
			/// ������ǰƷ������װ��, �ۼ�װ������
			for (int i = 0; i < equipLength; ++i)
			{
				/// ��ȡװ��ID
				equipID = rankData->m_EquipIDs[i];
				/// ��ȡװ������
				auto equipConfig = CConfigManager::getSingleton()->GetEquipment(equipID);
				/// �ۻ�װ������
				attr += equipConfig->baseAttribute;
			}
		}

		/// �������������������ݽ������Լӳ�
		CalculateAttributeEffect(attr, (HeroType)heroOriConfig->Type);
	}
}

void CHeroAttributeCalculator::CalculateItemEquipmentAttribute(OUT CharacterAttrData& attr, const SHeroEquipment& equipment)
{
	auto config = CConfigManager::getSingleton()->GetEquipment(equipment.m_dwEquipID);
	if (config == nullptr)
		return;

	attr = config->baseAttribute;
	/// ���װ��������
	if (equipment.m_dwStarLevel > 0)
	{
		auto config = CConfigManager::getSingleton()->GetForgingConfig(equipment.m_dwEquipID);
		if (config == nullptr)
			return;

		CharacterAttrData forgingAttr = config->baseAttribute;
		forgingAttr *= (float)equipment.m_dwStarLevel;
		/// �ۼӾ�������
		attr += forgingAttr;
	}
}

void CHeroAttributeCalculator::CalculateAttributeEffect(OUT CharacterAttrData& attribute, HeroType mainAttribute)
{
	/// �������ֵ = �����������ֵ + ���� �� 18
	attribute.MaxHealth += attribute.Power * 18.0f;
	/// ������ = ���������� + ���� �� 0.4
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
	/// ����ǿ�� = ���� �� 1.5
	attribute.Magic += attribute.Intelligence * 1.5f;
	/// ������ = ���� / 2 + ���� / 4
	attribute.Armor += attribute.Power / 2.0f + attribute.Agility / 4.0f;
	/// �����ֿ� = ���� / 2
	attribute.Resistance += attribute.Intelligence / 2.0f;
	/// ������ = ���� / 2.5
	attribute.Crit += attribute.Agility / 2.5f;
}

int CHeroAttributeCalculator::CalculateCombatPower(const SHeroSkillInfo* heroSkillData, SHeroAttr* pHeroAttr)
{
	if (heroSkillData == nullptr || pHeroAttr == nullptr)
		return 0;

	const CharacterAttrData& runtimeAttr = pHeroAttr->curAttr;

	//ս���� = ����*1 + ����*1 + ����*1+ ������*0.5 + �������*2 + �����ظ�*0.17 + �����ظ�*0.0625 + ħ��ǿ��*0.33 + ħ������*0.5
	//+ �������*0.04 + ������*1.3 + ����*4 + ��Ѫ*0.6 + ��͸������*6 + ���Է�������*3.5
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

	//��������ս���� = (ÿ�����ܵȼ� - 1) * 4
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
	/// ����ȼ��ӳ�����
	CalculateHeroLevelAttribute(heroData->m_dwHeroID, heroData->m_dwLevel, heroData->m_dwStarLevel, attrData->LevelAttribute);
	/// ����װ���ӳ�����
	CalculateHeroEquipmentAttribute(heroData->m_dwHeroID, heroData->m_Equipments, attrData->EquipmentAttribute);
	/// ����Ʒ�׼ӳ�����
	CalculateHeroRankAttribute(heroData->m_dwHeroID, heroData->m_dwRankLevel, attrData->RankAttribute);
	/// ת��Ե������
	ParseHeroFateAttribute(fateAttrArray, attrData->FateAttribute);
	/// �ϼ�����
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

	///< ��ȡ�佫���� �����ж��佫����
	auto heroOriConfig = CConfigManager::getSingleton()->GetHeroConfig(heroID);
	if (nullptr == heroOriConfig)
		return;

	vector<FateConfig*> vecfateConfigs;
	///< ��ȡ�佫�Ѽ����Ե������
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
			///< ��ȡԵ���佫�Ƴ�Ե�����Ժ�ĵ�ǰ����
			if (!pHeroDataManager->GetHeroAttributeWithoutFateAttribute(config->vecRelationshipHero[i], finalAttr))
				continue;
			///< Ե����������Ӧ�ӳ��������͵ļӳɰٷֱ�ֵ
			percent = config->vecAttributePercent[i];
			///< ƥ��Ե����������Ӧ�ӳ���������
			switch (config->vecAttributeType[i])
			{
			///< �������ֵ
			case 1:
				attr.MaxHealth += percent * finalAttr.MaxHealth;
				break;
			///< ��������
			case 2:
				attr.Physic += percent * finalAttr.Physic;
				break;
			///< ħ��ǿ��
			case 3:
				attr.Magic += percent * finalAttr.Magic;
				break;
			///< ������
			case 4:
				attr.Armor += percent * finalAttr.Armor;
				break;
			///< ��������
			case 5:
				attr.Resistance += percent * finalAttr.Resistance;
				break;
			///< ����
			case 6:
				attr.Crit += percent * finalAttr.Crit;
				break;
			///< �����ظ�
			case 7:
				attr.HealthRecovery = percent * finalAttr.HealthRecovery;
				break;
			///< �����ظ�
			case 8:
				attr.EnergyRecovery = percent * finalAttr.EnergyRecovery;
				break;
			default:
				break;
			}
		}
	}

	///< �������Լӳ�Ч��
	CalculateAttributeEffect(attr, (HeroType)heroOriConfig->Type);
}

void CHeroAttributeCalculator::ParseHeroFateAttribute(const int(&attrArray)[4], OUT CharacterAttrData& fateAttr)
{
	for (int i = 0; i < 4; ++i)
	{
		switch (attrArray[i] / 1000000)
		{
		case 1:	/// �������ֵ
			fateAttr.MaxHealth = attrArray[i] % 1000000;
			break;
		case 2:	/// ��������
			fateAttr.Physic = attrArray[i] % 1000000;
			break;
		case 3:	/// ħ��ǿ��
			fateAttr.Magic = attrArray[i] % 1000000;
			break;
		case 4: /// ������
			fateAttr.Armor = attrArray[i] % 1000000;
			break;
		case 5:	/// ��������
			fateAttr.Resistance = attrArray[i] % 1000000;
			break;
		case 6:	/// ����
			fateAttr.Crit = attrArray[i] % 1000000;
			break;
		case 7: /// �����ظ�
			fateAttr.HealthRecovery = attrArray[i] % 1000000;
			break;
		case 8:	/// �����ظ�
			fateAttr.EnergyRecovery = attrArray[i] % 1000000;
			break;
		}
	}
}
