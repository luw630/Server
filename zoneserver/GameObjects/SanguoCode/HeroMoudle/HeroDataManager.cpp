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
				rfalse("英雄%d不存在：", heroID);
				return false;
			}

			shared_ptr<SHeroAttr> attrData = make_shared<SHeroAttr>(pHeroConfig->baseAttribute);
			m_heroAttrlist.insert(make_pair(heroID, attrData));
			/// 计算等级加成属性
			CHeroAttributeCalculator::getSingleton()->CalculateHeroLevelAttribute(heroID, m_pHeroArray[heroIndex].m_dwLevel, m_pHeroArray[heroIndex].m_dwStarLevel, attrData->LevelAttribute);
			/// 计算装备加成属性
			CHeroAttributeCalculator::getSingleton()->CalculateHeroEquipmentAttribute(heroID, m_pHeroArray[heroIndex].m_Equipments, attrData->EquipmentAttribute);
			/// 计算品阶加成属性
			CHeroAttributeCalculator::getSingleton()->CalculateHeroRankAttribute(heroID, m_pHeroArray[heroIndex].m_dwRankLevel, attrData->RankAttribute);
			/// 合计属性
			attrData->curAttr += attrData->LevelAttribute;
			attrData->curAttr += attrData->EquipmentAttribute;
			attrData->curAttr += attrData->RankAttribute;
			/// 计算战斗力
			if (!CanCalculateFateAttribute())
			{
				int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(m_pHeroArray[heroIndex].m_SkillInfoList, attrData.get());
				if (combatPower != 0)
					ProcessAfterCombatPowerCalculated(heroID, combatPower);
			}
		}
	}

	/// 更新所有武将的缘分属性
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
	memset(&heroData.m_Equipments, 0, sizeof(SHeroEquipment) *EQUIP_MAX);//清空装备
	
	//技能ID初始化
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
	/// 计算等级加成属性
	CHeroAttributeCalculator::getSingleton()->CalculateHeroLevelAttribute(heroID, heroData.m_dwLevel, heroData.m_dwStarLevel, attrData->LevelAttribute);
	/// 计算装备加成属性
	CHeroAttributeCalculator::getSingleton()->CalculateHeroEquipmentAttribute(heroID, heroData.m_Equipments, attrData->EquipmentAttribute);
	/// 计算品阶加成属性
	CHeroAttributeCalculator::getSingleton()->CalculateHeroRankAttribute(heroID, heroData.m_dwRankLevel, attrData->RankAttribute);
	if (CanCalculateFateAttribute())
	{
		/// 计算缘分属性
		CHeroAttributeCalculator::getSingleton()->CalculateHeroFateAttribute(heroID, attrData->FateAttribute, this);
		attrData->curAttr += attrData->FateAttribute;
	}
	
	/// 合计属性
	attrData->curAttr += attrData->LevelAttribute;
	attrData->curAttr += attrData->EquipmentAttribute;
	attrData->curAttr += attrData->RankAttribute;
	/// 计算战斗力
	int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(heroData.m_SkillInfoList, attrData.get());
	if (combatPower != 0)
		ProcessAfterCombatPowerCalculated(heroID, combatPower);
	/// 更新与武将构成缘分的缘分武将属性
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
		rfalse("找不到英雄ID %d", heroID);
		return false;
	}

	findResult->second->m_Exp = value;
	if (findResult->second->m_dwLevel < level)
	{
		auto attrFindResult = m_heroAttrlist.find(heroID);
		if (attrFindResult == m_heroAttrlist.end() || attrFindResult->second == nullptr)
		{
			rfalse("找不到英雄%d实时的属性值", heroID);
			findResult->second->m_dwLevel = level;
			return false;
		}

		///先移除旧属性的作用
		attrFindResult->second->curAttr -= attrFindResult->second->LevelAttribute;
		findResult->second->m_dwLevel = level;
		/// 计算等级加成属性
		CHeroAttributeCalculator::getSingleton()->CalculateHeroLevelAttribute(heroID, findResult->second->m_dwLevel, findResult->second->m_dwStarLevel, attrFindResult->second->LevelAttribute);
		/// 合计属性
		attrFindResult->second->curAttr += attrFindResult->second->LevelAttribute;
		/// 计算战斗力
		int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(findResult->second->m_SkillInfoList, attrFindResult->second.get());
		if (combatPower != 0)
			ProcessAfterCombatPowerCalculated(heroID, combatPower);
		/// 更新与武将构成缘分的缘分武将属性
		UpdateRelatedHeroesAttribute(heroID);
	}
	return true;
}

void CHeroDataManager::ResetExpedtiionInfor()
{
	for (int i = 0; i < m_HeroNum; ++i)
	{
		///默认满血
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
	if (0 != pHeroData->m_Equipments[equipType].m_dwEquipID) //无法穿装备，因为当前部位已经穿上装备
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
		rfalse("客户端武将星级数据出错 ID : %d", heroID);
		return false;
	}

	auto attrFindResult = m_heroAttrlist.find(heroID);
	if (attrFindResult == m_heroAttrlist.end() || attrFindResult->second == nullptr)
	{
		rfalse("找不到英雄%d实时的属性值", heroID);
		return false;
	}

	///先移除旧属性的作用
	attrFindResult->second->curAttr -= attrFindResult->second->LevelAttribute;
	pHeroData->m_dwStarLevel++;
	/// 计算等级加成属性
	CHeroAttributeCalculator::getSingleton()->CalculateHeroLevelAttribute(heroID, pHeroData->m_dwLevel, pHeroData->m_dwStarLevel, attrFindResult->second->LevelAttribute);
	/// 合计属性
	attrFindResult->second->curAttr += attrFindResult->second->LevelAttribute;
	/// 计算战斗力
	int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(pHeroData->m_SkillInfoList, attrFindResult->second.get());
	if (combatPower != 0)
		ProcessAfterCombatPowerCalculated(heroID, combatPower);
	/// 更新与武将构成缘分的缘分武将属性
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
		if (heroData->m_Equipments[i].m_dwEquipID == 0)	//装备没有穿戴齐全
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
	///如果血量为0，则代表已经死亡
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
	if (pHeroData == nullptr) //无英雄则返回false
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
	if (nullptr == skillInfo) //如果查找不到对应技能 返回false
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
	if (skillInfo->m_dwSkillLevel >= pHeroData->m_dwLevel)	//如果技能等级 >= 武将等级 则返回false
	{
		return false;
	}
	
	skillInfo->m_dwSkillLevel += 1;
	///开始计算英雄的战斗力
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
	///开始计算英雄的战斗力
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

	/// 保存技能索引与技能信息
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

	/// 如果找到的技能个数小于传入技能ID数组有效个数则退出
	if (findNum < validNum)
		return false;

	DWORD index = 0;
	DWORD destLevel;
	/// 检查目标等级是否都有效
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
	/// 设置目标等级
	for (auto &pair : vecskillInfo)
	{
		pair.second->m_dwSkillLevel = destLevels[index++];
	}
	vecskillInfo.clear();
	///开始计算英雄的战斗力
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
		rfalse("找不到装备%d对应的精练配置文件，或者该装备无法精练", equipID);
		return -1;
	}

	///已经锻造成满级了 不能再锻造了
	if (equipData->m_dwStarLevel >= forgingConfig->refiningTime)
		return -1;

	DWORD targetExp = equipData->m_dwUpgradeExp + exp;

	starLevel = equipData->m_dwStarLevel;
	///判断当前的装备的经验值能否加上
	if (!CConfigManager::getSingleton()->GetForgingExpAndStarLevel(equipID, targetExp, equipData->m_dwStarLevel))
	{
		return -1;
	}
	///如果星级没有发生改变，则starLevel默认为0,只有发生了改变，starLevel才会变成对应的星级并返回出去
	if (starLevel == equipData->m_dwStarLevel)
		starLevel = 0;
	else
		starLevel = equipData->m_dwStarLevel;

	///更新装备精练会给英雄带来的属性值的提示
	if (starLevel > 0)
	{
		auto attrFindResult = m_heroAttrlist.find(heroID);
		if (attrFindResult != m_heroAttrlist.end() && attrFindResult->second != nullptr)
		{
			///先移除旧属性的作用
			attrFindResult->second->curAttr -= attrFindResult->second->EquipmentAttribute;
			/// 计算等级加成属性
			CHeroAttributeCalculator::getSingleton()->CalculateHeroEquipmentAttribute(heroID, pHeroData->m_Equipments, attrFindResult->second->EquipmentAttribute);
			/// 合计属性
			attrFindResult->second->curAttr += attrFindResult->second->EquipmentAttribute;
			/// 计算战斗力
			int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(pHeroData->m_SkillInfoList, attrFindResult->second.get());
			if (combatPower != 0)
				ProcessAfterCombatPowerCalculated(heroID, combatPower);
			/// 更新与武将构成缘分的缘分武将属性
			UpdateRelatedHeroesAttribute(heroID);
		}
		else
			rfalse("找不到英雄%d实时的属性值", heroID);
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
		memcpy(&heroDataList[i], GetHero(tempHeroID), sizeof(SHeroData)); //拷贝单个武将
	}
}


void CHeroDataManager::GetTop5Hero(SHeroData* pHeroDataList)
{
	int i = 0;
	for (auto iter : m_ExcellentHeroInfor)
	{
		SHeroData* heroData = _GetHero(iter.second);
		memcpy(&pHeroDataList[i], heroData, sizeof(SHeroData)); //拷贝单个武将
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

	memset(&pHeroData->m_Equipments, 0, sizeof(SHeroEquipment) * EQUIP_MAX);//清空装备
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
	///将“老数据”清理掉
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

	///暂时将战斗力的信息限定在五个英雄中
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
		///先移除旧属性的作用
		attrFindResult->second->curAttr -= attrFindResult->second->EquipmentAttribute;
		/// 计算等级加成属性
		CHeroAttributeCalculator::getSingleton()->CalculateHeroEquipmentAttribute(heroID, pHeroData->m_Equipments, attrFindResult->second->EquipmentAttribute);
		/// 合计属性
		attrFindResult->second->curAttr += attrFindResult->second->EquipmentAttribute;
		/// 计算战斗力
		int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(pHeroData->m_SkillInfoList, attrFindResult->second.get());
		if (combatPower != 0)
			ProcessAfterCombatPowerCalculated(heroID, combatPower);
		/// 更新与武将构成缘分的缘分武将属性
		UpdateRelatedHeroesAttribute(heroID);
		return;
	}
	else
		rfalse("找不到英雄%d实时的属性值", heroID);
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
	/// 如果未解锁缘分功能则退出
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

		/// 计算战斗力
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
	/// 如果未解锁缘分功能则退出
	if (!CanCalculateFateAttribute())
		return;
	for (auto & iter : m_heroAttrlist)
	{
		/// 由于需要更新是在初始化或君主等级升至解锁缘分功能时  故不作curAttr -= FateAttribute处理
		iter.second->curAttr -= iter.second->FateAttribute;
		CHeroAttributeCalculator::getSingleton()->CalculateHeroFateAttribute(iter.first, iter.second->FateAttribute, this);
		iter.second->curAttr += iter.second->FateAttribute;

		/// 计算战斗力
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
	/// 将数组元素置零
	memset(attrArray, 0, sizeof(int) * arraySize);
	auto iter = m_heroAttrlist.find(heroID);
	if (iter == m_heroAttrlist.end())
	{
		return false;
	}

	/// 以下数组元素赋值 假定0 - 999999为保存属性值 类型 = 类型值 * 1000000
	int fillNum = 0;
	const CharacterAttrData& attr = iter->second->FateAttribute;
	/// 最大生命值 缘分加成类型为1
	int tempValue = attr.MaxHealth;
	if (tempValue > 0)
	{
		attrArray[fillNum++] = 1000000 + tempValue;
	}

	if (fillNum >= arraySize)
		return true;

	/// 物理攻击力 缘分加成类型为2
	tempValue = attr.Physic;
	if (tempValue > 0)
	{
		attrArray[fillNum++] = 2000000 + tempValue;
	}

	if (fillNum >= arraySize)
		return true;

	/// 魔法强度 缘分加成类型为3
	tempValue = attr.Magic;
	if (tempValue > 0)
	{
		attrArray[fillNum++] = 3000000 + tempValue;
	}

	if (fillNum >= arraySize)
		return true;

	/// 物理护甲 缘分加成类型为4
	tempValue = attr.Armor;
	if (tempValue > 0)
	{
		attrArray[fillNum++] = 4000000 + tempValue;
	}

	if (fillNum >= arraySize)
		return true;

	/// 法术抗性 缘分加成类型为5
	tempValue = attr.Resistance;
	if (tempValue > 0)
	{
		attrArray[fillNum++] = 5000000 + tempValue;
	}

	if (fillNum >= arraySize)
		return true;

	/// 暴击 缘分加成类型为6
	tempValue = attr.Crit;
	if (tempValue > 0)
	{
		attrArray[fillNum++] = 6000000 + tempValue;
	}

	if (fillNum >= arraySize)
		return true;

	/// 生命回复 缘分加成类型为7
	tempValue = attr.HealthRecovery;
	if (tempValue > 0)
	{
		attrArray[fillNum++] = 7000000 + tempValue;
	}

	if (fillNum >= arraySize)
		return true;

	/// 能量回复 缘分加成类型为8
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
