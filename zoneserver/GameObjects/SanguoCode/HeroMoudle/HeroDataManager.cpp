#include "stdafx.h"
#include "HeroDataManager.h"
#include"..\Common\ConfigManager.h"
#include "HeroAttributeCalculator.h"
#include "..\NETWORKMODULE\SanguoPlayer.h"
#include "HeroFateInterrogator.h"
#include "ScriptManager.h"

CHeroDataManager::CHeroDataManager(SHeroData *pHeroArray)
	:m_HeroNum(0),
	m_pHeroArray(pHeroArray),
	m_bDataInitialed(false)
{
	//Init();
}

CHeroDataManager::~CHeroDataManager()
{
	Release();
}

bool CHeroDataManager::Init()
{
	if (m_bDataInitialed)
		return false;

	DWORD heroID = 0;
	for (int32_t heroIndex = 0; heroIndex < MAX_HERO_NUM; heroIndex++)
	{

		heroID = m_pHeroArray[heroIndex].m_dwHeroID;
		if (heroID == 0)
		{
			m_HeroNum = heroIndex;
			break;
		}
		else
		{
			m_heroList.insert(make_pair(heroID, &m_pHeroArray[heroIndex]));
			auto pHeroConfig = CConfigManager::getSingleton()->GetHeroConfig(heroID);
			if (nullptr == pHeroConfig)
			{
				rfalse("Ӣ��%d�����ڣ�", heroID);
				return false;
			}

			shared_ptr<SHeroAttr> attrData = make_shared<SHeroAttr>(pHeroConfig->baseAttribute);
			m_heroAttrlist.insert(make_pair(heroID, attrData));
			/// ����ȼ��ӳ�����
			CHeroAttributeCalculator::getSingleton()->CalculateHeroLevelAttribute(heroID, m_pHeroArray[heroIndex].m_dwLevel, m_pHeroArray[heroIndex].m_dwStarLevel, attrData->LevelAttribute);
			/// ����װ���ӳ�����
			CHeroAttributeCalculator::getSingleton()->CalculateHeroEquipmentAttribute(heroID, m_pHeroArray[heroIndex].m_Equipments, attrData->EquipmentAttribute);
			/// ����Ʒ�׼ӳ�����
			CHeroAttributeCalculator::getSingleton()->CalculateHeroRankAttribute(heroID, m_pHeroArray[heroIndex].m_dwRankLevel, attrData->RankAttribute);
			/// �ϼ�����
			attrData->curAttr += attrData->LevelAttribute;
			attrData->curAttr += attrData->EquipmentAttribute;
			attrData->curAttr += attrData->RankAttribute;
			/// ����ս����
			if (!CanCalculateFateAttribute())
			{
				int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(m_pHeroArray[heroIndex].m_SkillInfoList, attrData.get());
				if (combatPower != 0)
					ProcessAfterCombatPowerCalculated(heroID, combatPower);
			}
		}
	}

	/// ���������佫��Ե������
	UpdateFateAttributeOfAllHeroes();

	return true;
}

bool CHeroDataManager::Release()
{
	if (m_pHeroArray !=nullptr)
		m_pHeroArray = nullptr;

	return true;
}



bool  CHeroDataManager::AddHero(const int32_t heroID)
{
	if (m_heroList.size() >= MAX_HERO_NUM)
		return false;

	auto pHeroConfig = CConfigManager::getSingleton()->GetHeroConfig(heroID);
	if (ExistHero(heroID) || nullptr == pHeroConfig)
		return false;

	SHeroData& heroData = m_pHeroArray[m_HeroNum];
	if (heroData.m_dwHeroID != 0)
		return false;

	heroData.m_dwHeroID = heroID;
	heroData.m_dwLevel = 1;
	heroData.m_dwRankLevel = pHeroConfig->Rank;
	heroData.m_dwStarLevel = pHeroConfig->Star;
	heroData.m_Exp = 0;
	memset(&heroData.m_Equipments, 0, sizeof(SHeroEquipment) *EQUIP_MAX);//���װ��
	
	//����ID��ʼ��
	heroData.m_SkillInfoList[0].m_dwSkillID = pHeroConfig->Skill1;
	heroData.m_SkillInfoList[1].m_dwSkillID = pHeroConfig->Skill2;
	heroData.m_SkillInfoList[2].m_dwSkillID = pHeroConfig->Skill3;
	heroData.m_SkillInfoList[3].m_dwSkillID = pHeroConfig->Skill4;
	
	heroData.m_SkillInfoList[0].m_dwSkillLevel = 1;
	heroData.m_SkillInfoList[1].m_dwSkillLevel = 1;
	heroData.m_SkillInfoList[2].m_dwSkillLevel = 1;
	heroData.m_SkillInfoList[3].m_dwSkillLevel = 1;
	m_heroList.insert(make_pair(heroID, &heroData));
	shared_ptr<SHeroAttr> attrData = make_shared<SHeroAttr>(pHeroConfig->baseAttribute);
	m_heroAttrlist.insert(make_pair(heroID, attrData));
	/// ����ȼ��ӳ�����
	CHeroAttributeCalculator::getSingleton()->CalculateHeroLevelAttribute(heroID, heroData.m_dwLevel, heroData.m_dwStarLevel, attrData->LevelAttribute);
	/// ����װ���ӳ�����
	CHeroAttributeCalculator::getSingleton()->CalculateHeroEquipmentAttribute(heroID, heroData.m_Equipments, attrData->EquipmentAttribute);
	/// ����Ʒ�׼ӳ�����
	CHeroAttributeCalculator::getSingleton()->CalculateHeroRankAttribute(heroID, heroData.m_dwRankLevel, attrData->RankAttribute);
	if (CanCalculateFateAttribute())
	{
		/// ����Ե������
		CHeroAttributeCalculator::getSingleton()->CalculateHeroFateAttribute(heroID, attrData->FateAttribute, this);
		attrData->curAttr += attrData->FateAttribute;
	}
	
	/// �ϼ�����
	attrData->curAttr += attrData->LevelAttribute;
	attrData->curAttr += attrData->EquipmentAttribute;
	attrData->curAttr += attrData->RankAttribute;
	/// ����ս����
	int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(heroData.m_SkillInfoList, attrData.get());
	if (combatPower != 0)
		ProcessAfterCombatPowerCalculated(heroID, combatPower);
	/// �������佫����Ե�ֵ�Ե���佫����
	UpdateRelatedHeroesAttribute(heroID);

	++m_HeroNum;
	return true;
}

bool CHeroDataManager::ExistHero(const int32_t heorID) const
{
	if (m_heroList.find(heorID) != m_heroList.end())
		return true;

	return false;
}


const SHeroData* CHeroDataManager::GetHero(const int32_t heorID)const
{
	auto findResult = m_heroList.find(heorID);
	if (findResult != m_heroList.end())
	{
		return findResult->second;
	}

	return nullptr;
}

bool CHeroDataManager::PlusHeroExp(int heroID, int value, int level)
{
	if (value < 0)
		return false;

	auto findResult = m_heroList.find(heroID);
	if (findResult == m_heroList.end() || findResult->second == nullptr)
	{
		rfalse("�Ҳ���Ӣ��ID %d", heroID);
		return false;
	}

	findResult->second->m_Exp = value;
	if (findResult->second->m_dwLevel < level)
	{
		auto attrFindResult = m_heroAttrlist.find(heroID);
		if (attrFindResult == m_heroAttrlist.end() || attrFindResult->second == nullptr)
		{
			rfalse("�Ҳ���Ӣ��%dʵʱ������ֵ", heroID);
			findResult->second->m_dwLevel = level;
			return false;
		}

		///���Ƴ������Ե�����
		attrFindResult->second->curAttr -= attrFindResult->second->LevelAttribute;
		findResult->second->m_dwLevel = level;
		/// ����ȼ��ӳ�����
		CHeroAttributeCalculator::getSingleton()->CalculateHeroLevelAttribute(heroID, findResult->second->m_dwLevel, findResult->second->m_dwStarLevel, attrFindResult->second->LevelAttribute);
		/// �ϼ�����
		attrFindResult->second->curAttr += attrFindResult->second->LevelAttribute;
		/// ����ս����
		int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(findResult->second->m_SkillInfoList, attrFindResult->second.get());
		if (combatPower != 0)
			ProcessAfterCombatPowerCalculated(heroID, combatPower);
		/// �������佫����Ե�ֵ�Ե���佫����
		UpdateRelatedHeroesAttribute(heroID);
	}
	return true;
}

void CHeroDataManager::ResetExpedtiionInfor()
{
	for (int i = 0; i < m_HeroNum; ++i)
	{
		///Ĭ����Ѫ
		m_pHeroArray[i].m_fExpeditionHealthScale = 1.0f;
		m_pHeroArray[i].m_fExpeditionManaScale = 0;
		m_pHeroArray[i].m_bDeadInExpedition = false;
	}
}

SHeroData* CHeroDataManager::_GetHero(const int32_t heroID) 
{
	if (ExistHero(heroID))
	{
		return m_heroList[heroID];
	}

	return nullptr;
}

bool CHeroDataManager::EquipHero(const int32_t heroID, const int32_t euquipID, const int32_t equipType)
{
	SHeroData * pHeroData = _GetHero(heroID);
	if (nullptr == pHeroData)
		return false;
	EquipmentConfig newEquipment;
	if (0 != pHeroData->m_Equipments[equipType].m_dwEquipID) //�޷���װ������Ϊ��ǰ��λ�Ѿ�����װ��
		return false;

	pHeroData->m_Equipments[equipType].m_dwEquipID = euquipID;
	return true;
}

void CHeroDataManager::IncreaseHeroRank(int32_t heroID)
{
	SHeroData * pHeroData = _GetHero(heroID);
	if (nullptr == pHeroData)
		return;

	pHeroData->m_dwRankLevel++;
}

bool CHeroDataManager::HeroStarLevelRise(const int32_t heroID, const DWORD curStarLevel)
{
	SHeroData* pHeroData = _GetHero(heroID);
	if (pHeroData == nullptr)
		return false;
	if (pHeroData->m_dwStarLevel != curStarLevel)
	{
		rfalse("�ͻ����佫�Ǽ����ݳ��� ID : %d", heroID);
		return false;
	}

	auto attrFindResult = m_heroAttrlist.find(heroID);
	if (attrFindResult == m_heroAttrlist.end() || attrFindResult->second == nullptr)
	{
		rfalse("�Ҳ���Ӣ��%dʵʱ������ֵ", heroID);
		return false;
	}

	///���Ƴ������Ե�����
	attrFindResult->second->curAttr -= attrFindResult->second->LevelAttribute;
	pHeroData->m_dwStarLevel++;
	/// ����ȼ��ӳ�����
	CHeroAttributeCalculator::getSingleton()->CalculateHeroLevelAttribute(heroID, pHeroData->m_dwLevel, pHeroData->m_dwStarLevel, attrFindResult->second->LevelAttribute);
	/// �ϼ�����
	attrFindResult->second->curAttr += attrFindResult->second->LevelAttribute;
	/// ����ս����
	int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(pHeroData->m_SkillInfoList, attrFindResult->second.get());
	if (combatPower != 0)
		ProcessAfterCombatPowerCalculated(heroID, combatPower);
	/// �������佫����Ե�ֵ�Ե���佫����
	UpdateRelatedHeroesAttribute(heroID);
	return true;
}

bool CHeroDataManager::CanRiseRank(const int32_t heroID) const
{
	const SHeroData* heroData = GetHero(heroID);
	int heroMaxRank = CConfigManager::getSingleton()->GetHeroMaxRankGrade(heroID);
	bool result = false;
	result = (heroData != nullptr && heroMaxRank != -1) ? heroData->m_dwRankLevel < heroMaxRank : false;
	if (!result)
		return false;

	for (int i = 0; i < EQUIP_MAX; ++i)
	{
		if (heroData->m_Equipments[i].m_dwEquipID == 0)	//װ��û�д�����ȫ
			return false;
	}

	return true;
}

bool CHeroDataManager::CanRiseStarLevel(const int32_t heroID) const
{	
	const SHeroData* heroData = GetHero(heroID);
	int maxStarLevel = CConfigManager::getSingleton()->GetHeroMaxStarLevel(heroID);
	return (heroData != nullptr && maxStarLevel != -1) ? heroData->m_dwStarLevel < maxStarLevel : false;
}

void CHeroDataManager::SetExpeditionInfor(int32_t heroID, float healthScale, float manaScale)
{
	auto findResult = m_heroList.find(heroID);
	if (findResult == m_heroList.end() || findResult->second == nullptr)
		return;
	
	findResult->second->m_fExpeditionHealthScale = max(0, min(healthScale, 1));
	findResult->second->m_fExpeditionManaScale = max(0, min(manaScale, 1));
	///���Ѫ��Ϊ0��������Ѿ�����
	if (healthScale == 0)
	{
		findResult->second->m_bDeadInExpedition = true;
		findResult->second->m_fExpeditionManaScale = 0;
	}
}

const SHeroData* CHeroDataManager::GetExcellentHeroInfor()
{
	if (m_ExcellentHeroInfor.empty())
		return nullptr;

	return GetHero(m_ExcellentHeroInfor.begin()->second);
}

bool CHeroDataManager::UpgradeHeroSkill(const int32_t heroID, const int32_t skillID, int32_t skillLevel)
{
	SHeroData* pHeroData = _GetHero(heroID);
	if (pHeroData == nullptr) //��Ӣ���򷵻�false
		return false;

	auto findResult = m_heroAttrlist.find(heroID);
	if (findResult == m_heroAttrlist.end())
		return false;

	SHeroSkillInfo* skillInfo = nullptr;
	int index = 0;
	for (; index < MAX_SKILLNUM; index++)
	{
		if (pHeroData->m_SkillInfoList[index].m_dwSkillID == skillID)
		{
			skillInfo = &pHeroData->m_SkillInfoList[index];
			break;
		}
	}
	if (nullptr == skillInfo) //������Ҳ�����Ӧ���� ����false
		return false ;

	/*if (index == 3)
	{
		if (pHeroData->m_dwLevel - skillInfo->m_dwSkillLevel <= 40)
			return false;
	}
	else if (index == 2)
	{
		if (pHeroData->m_dwLevel - skillInfo->m_dwSkillLevel <= 20)
			return false;
	}
	else */
	if (skillInfo->m_dwSkillLevel >= pHeroData->m_dwLevel)	//������ܵȼ� >= �佫�ȼ� �򷵻�false
	{
		return false;
	}
	
	skillInfo->m_dwSkillLevel += 1;
	///��ʼ����Ӣ�۵�ս����
	int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(pHeroData->m_SkillInfoList, findResult->second.get());
	if (combatPower != 0)
		ProcessAfterCombatPowerCalculated(heroID, combatPower);
	return true;
}

bool CHeroDataManager::MultiUpgradeHeroSkill(const int32_t heroID, const int32_t skillID, int32_t destLevel)
{
	SHeroData* pHeroData = _GetHero(heroID);
	if (pHeroData == nullptr)
		return false;

	auto findResult = m_heroAttrlist.find(heroID);
	if (findResult == m_heroAttrlist.end())
		return false;

	SHeroSkillInfo* skillInfo = nullptr;
	int index = 0;
	for (; index < MAX_SKILLNUM; index++)
	{
		if (pHeroData->m_SkillInfoList[index].m_dwSkillID == skillID)
		{
			skillInfo = &pHeroData->m_SkillInfoList[index];
			break;
		}
	}

	if (nullptr == skillInfo || destLevel <= skillInfo->m_dwSkillLevel)
		return false;

	/*if (index == 3)
	{
		if (pHeroData->m_dwLevel - destLevel < 40)
			return false;
	}
	else if (index == 2)
	{
		if (pHeroData->m_dwLevel - destLevel < 20)
			return false;
	}
	else */
	if (destLevel > pHeroData->m_dwLevel)
	{
		return false;
	}

	skillInfo->m_dwSkillLevel = destLevel;
	///��ʼ����Ӣ�۵�ս����
	int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(pHeroData->m_SkillInfoList, findResult->second.get());
	if (combatPower != 0)
		ProcessAfterCombatPowerCalculated(heroID, combatPower);
	return true;
}

bool CHeroDataManager::MultiUpgradeHeroSkill(const int32_t heroID, DWORD skillIDs[], DWORD destLevels[], DWORD validNum)
{
	SHeroData* pHeroData = _GetHero(heroID);
	if (nullptr == pHeroData)
		return false;

	auto findResult = m_heroAttrlist.find(heroID);
	if (findResult == m_heroAttrlist.end())
		return false;

	/// ���漼�������뼼����Ϣ
	vector<pair<int, SHeroSkillInfo*>> vecskillInfo;
	DWORD findNum = 0;
	for (int i = 0; i < MAX_SKILLNUM; ++i)
	{
		if (pHeroData->m_SkillInfoList[i].m_dwSkillID == skillIDs[findNum])
		{
			vecskillInfo.push_back(make_pair(i, &pHeroData->m_SkillInfoList[i]));
			if (++findNum == validNum)
				break;
		}
	}

	/// ����ҵ��ļ��ܸ���С�ڴ��뼼��ID������Ч�������˳�
	if (findNum < validNum)
		return false;

	DWORD index = 0;
	DWORD destLevel;
	/// ���Ŀ��ȼ��Ƿ���Ч
	for (auto &pair : vecskillInfo)
	{
		destLevel = destLevels[index++];
		if (destLevel <= pair.second->m_dwSkillLevel)
			return false;
		/*if (pair.first == 3)
		{
			if (pHeroData->m_dwLevel - destLevel < 40)
				return false;
		}
		else if (pair.first == 2)
		{
			if (pHeroData->m_dwLevel - destLevel < 20)
				return false;
		}
		else */
		if (destLevel > pHeroData->m_dwLevel)
		{
			return false;
		}
	}

	index = 0;
	/// ����Ŀ��ȼ�
	for (auto &pair : vecskillInfo)
	{
		pair.second->m_dwSkillLevel = destLevels[index++];
	}
	vecskillInfo.clear();
	///��ʼ����Ӣ�۵�ս����
	int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(pHeroData->m_SkillInfoList, findResult->second.get());
	if (combatPower != 0)
		ProcessAfterCombatPowerCalculated(heroID, combatPower);
	return true;
}

const SHeroEquipment* CHeroDataManager::GetHeroEquipment(int32_t heroID, int32_t equipID) const
{
	const SHeroData* pHeroData = GetHero(heroID);
	if (pHeroData == nullptr)
		return nullptr;

	for (int i = 0; i < EQUIP_MAX; ++i)
	{
		if (pHeroData->m_Equipments[i].m_dwEquipID == equipID)
			return &pHeroData->m_Equipments[i];
	}
}

const SHeroEquipment * const CHeroDataManager::GetHeroEquipments(int32_t heroID) const
{
	const SHeroData* pHeroData = GetHero(heroID);
	if (pHeroData == nullptr)
		return nullptr;

	return pHeroData->m_Equipments;
}

int CHeroDataManager::IncreaseHeroEquipmentForgingExp(int32_t heroID, int32_t equipID, int32_t exp, OUT int32_t& starLevel)
{
	if (exp == 0)
		return -1;

	SHeroData* pHeroData = _GetHero(heroID);
	if (pHeroData == nullptr)
		return -1;

	SHeroEquipment *equipData = nullptr;
	for (int i = 0; i < EQUIP_MAX; ++i)
	{
		if (pHeroData->m_Equipments[i].m_dwEquipID == equipID)
		{
			equipData = &pHeroData->m_Equipments[i];
			break;
		}
	}
	if (equipData == nullptr)
		return -1;

	const ForgingConfig* forgingConfig = CConfigManager::getSingleton()->GetForgingConfig(equipID);
	if (forgingConfig == nullptr || forgingConfig->refiningTime == 0)
	{
		rfalse("�Ҳ���װ��%d��Ӧ�ľ��������ļ������߸�װ���޷�����", equipID);
		return -1;
	}

	///�Ѿ������������ �����ٶ�����
	if (equipData->m_dwStarLevel >= forgingConfig->refiningTime)
		return -1;

	DWORD targetExp = equipData->m_dwUpgradeExp + exp;

	starLevel = equipData->m_dwStarLevel;
	///�жϵ�ǰ��װ���ľ���ֵ�ܷ����
	if (!CConfigManager::getSingleton()->GetForgingExpAndStarLevel(equipID, targetExp, equipData->m_dwStarLevel))
	{
		return -1;
	}
	///����Ǽ�û�з����ı䣬��starLevelĬ��Ϊ0,ֻ�з����˸ı䣬starLevel�Ż��ɶ�Ӧ���Ǽ������س�ȥ
	if (starLevel == equipData->m_dwStarLevel)
		starLevel = 0;
	else
		starLevel = equipData->m_dwStarLevel;

	///����װ���������Ӣ�۴���������ֵ����ʾ
	if (starLevel > 0)
	{
		auto attrFindResult = m_heroAttrlist.find(heroID);
		if (attrFindResult != m_heroAttrlist.end() && attrFindResult->second != nullptr)
		{
			///���Ƴ������Ե�����
			attrFindResult->second->curAttr -= attrFindResult->second->EquipmentAttribute;
			/// ����ȼ��ӳ�����
			CHeroAttributeCalculator::getSingleton()->CalculateHeroEquipmentAttribute(heroID, pHeroData->m_Equipments, attrFindResult->second->EquipmentAttribute);
			/// �ϼ�����
			attrFindResult->second->curAttr += attrFindResult->second->EquipmentAttribute;
			/// ����ս����
			int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(pHeroData->m_SkillInfoList, attrFindResult->second.get());
			if (combatPower != 0)
				ProcessAfterCombatPowerCalculated(heroID, combatPower);
			/// �������佫����Ե�ֵ�Ե���佫����
			UpdateRelatedHeroesAttribute(heroID);
		}
		else
			rfalse("�Ҳ���Ӣ��%dʵʱ������ֵ", heroID);
	}

	int increasedExp = targetExp - equipData->m_dwUpgradeExp;
	equipData->m_dwUpgradeExp = targetExp;

	return increasedExp;
}

bool CHeroDataManager::IsHeroEquipmentAttached(int32_t heroID, int32_t equipID)const
{
	const SHeroData* pHeroData = GetHero(heroID);
	if (pHeroData == nullptr)
		return false;

	for (int i = 0; i < EQUIP_MAX; ++i)
	{
		if (pHeroData->m_Equipments[i].m_dwEquipID == equipID)
		{
			return true;
		}
	}

	return false;
}

DWORD CHeroDataManager::GetHeroSkillLevel(const int32_t heroID, const int32_t skillID) const
{
	auto pHeroData = GetHero(heroID);
	if (pHeroData == nullptr)
		return 0;
	const SHeroSkillInfo* skillInfo = nullptr;
	for (int i = 0; i < MAX_SKILLNUM; ++i)
	{
		skillInfo = &pHeroData->m_SkillInfoList[i];
		if (skillInfo->m_dwSkillID == skillID)
			return skillInfo->m_dwSkillLevel;
	}

	return 0;
}
void CHeroDataManager::GetArenaDefenseTeamInfo(SHeroData* heroDataList)
{
	int tempHeroID;
	for (int i = 0; i < 5; i++)
	{
		tempHeroID = m_pDefenseTeamID[i];
		if (false == ExistHero(tempHeroID))
		{
			memset(&heroDataList[i], 0, sizeof(SHeroData));
			continue;
		}
		memcpy(&heroDataList[i], GetHero(tempHeroID), sizeof(SHeroData)); //���������佫
	}
}


void CHeroDataManager::GetTop5Hero(SHeroData* pHeroDataList)
{
	int i = 0;
	for (auto iter : m_ExcellentHeroInfor)
	{
		SHeroData* heroData = _GetHero(iter.second);
		memcpy(&pHeroDataList[i], heroData, sizeof(SHeroData)); //���������佫
		++i;
		if (i >= 5)
			break;
	}
}

void CHeroDataManager::SetArenaDefenseTeam(DWORD arenaDefenseTeamID[5])
{
	memcpy(m_pDefenseTeamID, arenaDefenseTeamID, 5*sizeof(DWORD));
	OnArenaDefenseTeamChanged();
}

void CHeroDataManager::OnArenaDefenseTeamChanged() {}

void CHeroDataManager::ClearHeroEquipment(int heroID)
{
	SHeroData* pHeroData = _GetHero(heroID);
	if (pHeroData == nullptr)
		return;

	memset(&pHeroData->m_Equipments, 0, sizeof(SHeroEquipment) * EQUIP_MAX);//���װ��
}

void CHeroDataManager::GetAllHeroID(vector<int32_t>& vecID)
{
	for (auto &iter : m_heroList)
	{
		vecID.push_back(iter.first);
	}
}

void CHeroDataManager::ProcessAfterCombatPowerCalculated(int heroID, int combatPower)
{
	///���������ݡ������
	auto itor = m_ExcellentHeroInfor.begin();
	for (; itor != m_ExcellentHeroInfor.end();)
	{
		if (itor->second == heroID)
		{
			m_ExcellentHeroInfor.erase(itor++);
		}
		else
			++itor;
	}

	m_ExcellentHeroInfor.insert(make_pair(combatPower, heroID));

	///��ʱ��ս��������Ϣ�޶������Ӣ����
	int extraCount = (int)m_ExcellentHeroInfor.size() - 5;
	if (extraCount > 0)
	{
		auto ritor = m_ExcellentHeroInfor.end();
		--ritor;
		m_ExcellentHeroInfor.erase(ritor);
	}
}

void CHeroDataManager::EquipHeroEndProcess(int32_t heroID)
{
	SHeroData* pHeroData = _GetHero(heroID);
	if (pHeroData == nullptr)
		return ;

	auto attrFindResult = m_heroAttrlist.find(heroID);
	if (attrFindResult != m_heroAttrlist.end() && attrFindResult->second != nullptr)
	{
		///���Ƴ������Ե�����
		attrFindResult->second->curAttr -= attrFindResult->second->EquipmentAttribute;
		/// ����ȼ��ӳ�����
		CHeroAttributeCalculator::getSingleton()->CalculateHeroEquipmentAttribute(heroID, pHeroData->m_Equipments, attrFindResult->second->EquipmentAttribute);
		/// �ϼ�����
		attrFindResult->second->curAttr += attrFindResult->second->EquipmentAttribute;
		/// ����ս����
		int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(pHeroData->m_SkillInfoList, attrFindResult->second.get());
		if (combatPower != 0)
			ProcessAfterCombatPowerCalculated(heroID, combatPower);
		/// �������佫����Ե�ֵ�Ե���佫����
		UpdateRelatedHeroesAttribute(heroID);
		return;
	}
	else
		rfalse("�Ҳ���Ӣ��%dʵʱ������ֵ", heroID);
}

int CHeroDataManager::GetCombatPower()
{
	int combatPower = 0;
	for (auto it : m_ExcellentHeroInfor)
		combatPower += it.first;
	return combatPower;
}

bool CHeroDataManager::GetHeroAttributeWithoutFateAttribute(int32_t heroID, OUT CharacterAttrData& attr) const
{
	auto attrFindResult = m_heroAttrlist.find(heroID);
	if (attrFindResult == m_heroAttrlist.end() || nullptr == attrFindResult->second)
		return false;

	attrFindResult->second->curAttr.Difference(attrFindResult->second->FateAttribute, attr);

	return true;
}

void CHeroDataManager::UpdateRelatedHeroesAttribute(int32_t heroID)
{
	/// ���δ����Ե�ֹ������˳�
	if (!CanCalculateFateAttribute())
		return;

	auto vecHeroes = CHeroFateInterrogator::getSingleton()->GetRelatedHeroes(heroID);
	if (nullptr == vecHeroes)
		return;

	unordered_map<int32_t, shared_ptr<SHeroAttr>>::iterator iter;
	auto endIter = m_heroAttrlist.end();
	for (auto & id : *vecHeroes)
	{
		iter = m_heroAttrlist.find(id);
		if (iter == endIter)
			continue;
		iter->second->curAttr -= iter->second->FateAttribute;
		CHeroAttributeCalculator::getSingleton()->CalculateHeroFateAttribute(id, iter->second->FateAttribute, this);
		iter->second->curAttr += iter->second->FateAttribute;

		/// ����ս����
		SHeroData* pHeroData = _GetHero(iter->first);
		if (pHeroData == nullptr)
			continue;

		int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(pHeroData->m_SkillInfoList, iter->second.get());
		if (combatPower != 0)
			ProcessAfterCombatPowerCalculated(id, combatPower);
	}
}

bool CHeroDataManager::CanCalculateFateAttribute()
{
	return false;
}

void CHeroDataManager::UpdateFateAttributeOfAllHeroes()
{
	/// ���δ����Ե�ֹ������˳�
	if (!CanCalculateFateAttribute())
		return;
	for (auto & iter : m_heroAttrlist)
	{
		/// ������Ҫ�������ڳ�ʼ��������ȼ���������Ե�ֹ���ʱ  �ʲ���curAttr -= FateAttribute����
		iter.second->curAttr -= iter.second->FateAttribute;
		CHeroAttributeCalculator::getSingleton()->CalculateHeroFateAttribute(iter.first, iter.second->FateAttribute, this);
		iter.second->curAttr += iter.second->FateAttribute;

		/// ����ս����
		SHeroData* pHeroData = _GetHero(iter.first);
		if (pHeroData == nullptr)
			continue;

		int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(pHeroData->m_SkillInfoList, iter.second.get());
		if (combatPower != 0)
			ProcessAfterCombatPowerCalculated(iter.first, combatPower);
	}
}

const SHeroAttr* CHeroDataManager::GetHeroAttr(int32_t heroID) const
{
	auto iter = m_heroAttrlist.find(heroID);

	return iter == m_heroAttrlist.end() ? nullptr : iter->second.get();
}

bool CHeroDataManager::GetHeroFateAttribute(int32_t heroID, int attrArray[], const int arraySize) const
{
	if (nullptr == attrArray || 0 >= arraySize)
		return false;
	/// ������Ԫ������
	memset(attrArray, 0, sizeof(int) * arraySize);
	auto iter = m_heroAttrlist.find(heroID);
	if (iter == m_heroAttrlist.end())
	{
		return false;
	}

	/// ��������Ԫ�ظ�ֵ �ٶ�0 - 999999Ϊ��������ֵ ���� = ����ֵ * 1000000
	int fillNum = 0;
	const CharacterAttrData& attr = iter->second->FateAttribute;
	/// �������ֵ Ե�ּӳ�����Ϊ1
	int tempValue = attr.MaxHealth;
	if (tempValue > 0)
	{
		attrArray[fillNum++] = 1000000 + tempValue;
	}

	if (fillNum >= arraySize)
		return true;

	/// �������� Ե�ּӳ�����Ϊ2
	tempValue = attr.Physic;
	if (tempValue > 0)
	{
		attrArray[fillNum++] = 2000000 + tempValue;
	}

	if (fillNum >= arraySize)
		return true;

	/// ħ��ǿ�� Ե�ּӳ�����Ϊ3
	tempValue = attr.Magic;
	if (tempValue > 0)
	{
		attrArray[fillNum++] = 3000000 + tempValue;
	}

	if (fillNum >= arraySize)
		return true;

	/// ������ Ե�ּӳ�����Ϊ4
	tempValue = attr.Armor;
	if (tempValue > 0)
	{
		attrArray[fillNum++] = 4000000 + tempValue;
	}

	if (fillNum >= arraySize)
		return true;

	/// �������� Ե�ּӳ�����Ϊ5
	tempValue = attr.Resistance;
	if (tempValue > 0)
	{
		attrArray[fillNum++] = 5000000 + tempValue;
	}

	if (fillNum >= arraySize)
		return true;

	/// ���� Ե�ּӳ�����Ϊ6
	tempValue = attr.Crit;
	if (tempValue > 0)
	{
		attrArray[fillNum++] = 6000000 + tempValue;
	}

	if (fillNum >= arraySize)
		return true;

	/// �����ظ� Ե�ּӳ�����Ϊ7
	tempValue = attr.HealthRecovery;
	if (tempValue > 0)
	{
		attrArray[fillNum++] = 7000000 + tempValue;
	}

	if (fillNum >= arraySize)
		return true;

	/// �����ظ� Ե�ּӳ�����Ϊ8
	tempValue = attr.EnergyRecovery;
	if (tempValue > 0)
	{
		attrArray[fillNum++] = 8000000 + tempValue;
	}

	return true;
}


int CHeroDataManager::GetHeroNum() const
{
	return m_HeroNum;
}

int CHeroDataManager::GetHeroStarSum() const
{
	int sum = 0;
	for (auto itor : m_heroList)
	{
		sum += itor.second->m_dwStarLevel;
	}

	return sum;
}

int CHeroDataManager::GetHeroLevel(int heroID) const
{
	auto findResult = m_heroList.find(heroID);
	if (findResult != m_heroList.end())
	{
		return findResult->second->m_dwLevel;
	}

	return 0;
}
