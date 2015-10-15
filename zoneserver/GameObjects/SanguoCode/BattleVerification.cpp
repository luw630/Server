#include "stdafx.h"
#include "BattleVerification.h"
//#include "DuplicateMoudle\DuplicateBaseManager.h"
//#include "DuplicateMoudle\DuplicateDataManager.h"
#include "BaseDataManager.h"
#include "Common\ConfigManager.h"
#include "../NETWORKMODULE/SanguoPlayerMsg.h"
#include "ScriptMoudle\ScriptMoudle.h"
#include "extraScriptFunctions\lite_lualibrary.hpp"
#include "extraScriptFunctions\ExtraScriptFunctions.h"
#include "ScriptManager.h"
#include "HeroMoudle\HeroAttributeCalculator.h"
#include "ArenaMoudel\ArenaMoudel.h"
#include "PlayerManager.h"
#include "Player.h"

extern LPIObject GetPlayerBySID(DWORD dwStaticID);

CBattleVerification::CBattleVerification()
	: m_globalConfig(CConfigManager::getSingleton()->globalConfig)
{
}


CBattleVerification::~CBattleVerification()
{
}

bool CBattleVerification::ArenaBattleVerification(const ArenaBattleVerifyData& verifyData, const __time64_t& battleStartTime, const CBaseDataManager& baseDataManager)
{
	if (baseDataManager.GetGMFlag())
		return true;

	if (!BattleTimeVerification(verifyData.heroID, verifyData.AttackTimes, m_globalConfig.MovingTime, 1, battleStartTime, baseDataManager))
		return false;

	DWORD ourHeroes[MAX_ARENACHALLENGE_HERONUM];
	int ourCombatPower = 0, competiterCombatPower = 0;
	const SHeroAttr* pHeroAttr = nullptr;
	int tempHeroID;
	for (int i = 0; i < verifyData.myFighterNum; ++i)
	{
		tempHeroID = verifyData.myFighterVerifyDatas[i].heroID;
		pHeroAttr = baseDataManager.GetHeroAttr(tempHeroID);
		if (nullptr == pHeroAttr)
			return false;
		ourHeroes[i] = tempHeroID;
		ourCombatPower += pHeroAttr->CombatPower;
	}

	CPlayer* pPlayer = (CPlayer*)GetPlayerBySID(verifyData.competitorArenaID - MAX_ROBOT_NUM)->DynamicCast(IID_PLAYER);
	CBaseDataManager* pCompetitorBaseMgr = nullptr;
	const SBaseArenaCompetitor* pCompetitorData = nullptr;
	if (pPlayer)
	{
		pCompetitorBaseMgr = pPlayer->GetBaseDataMgr();
		if (nullptr == pCompetitorBaseMgr)
			return false;
		for (int i = 0; i < verifyData.competitorFighterNum; ++i)
		{
			pHeroAttr = pCompetitorBaseMgr->GetHeroAttr(verifyData.competitorFighterVerifyDatas[i].heroID);
			if (nullptr == pHeroAttr)
				return false;
			competiterCombatPower += pHeroAttr->CombatPower;
		}
	}
	else
	{
		pCompetitorData = CArenaMoudel::getSingleton()->GetManagedArenaData(verifyData.competitorArenaID);
		if (nullptr == pCompetitorData)
			return false;
		for (int i = 0; i < 5; ++i)
		{
			competiterCombatPower += CHeroAttributeCalculator::getSingleton()->CalculateSpecifyHeroComabtPower(&pCompetitorData->m_HeroList[i], pCompetitorData->m_FateAttr[i].fateAttr);
		}
	}

	/// 验证双方战斗力 如果验证通过 则判定战斗胜利
	if (CombatPowerVerification(ourCombatPower, competiterCombatPower, m_globalConfig.FightingCapacityFaultTolerant, baseDataManager))
		return true;

	/// 计算我方上阵武将属性
	if (!_CalculateFighterAttribute(ourHeroes, verifyData.myFighterNum, baseDataManager, m_mapOurFighterAttr, m_OurBuffAttr, m_OurDebuffAttr))
		return false;
	/// 计算地方上阵武将属性
	if (!_CalculateArenaEnemyFighterAttribute(verifyData.competitorArenaID))
		return false;
	///生命值验证
	AttributeMap::iterator findIter;
	AttributeMap::iterator endIter = m_mapOurFighterAttr.end();
	for (int i = 0; i < verifyData.myFighterNum; ++i)
	{
		findIter = m_mapOurFighterAttr.find(verifyData.myFighterVerifyDatas[i].heroID);
		if (findIter == endIter)
			return false;
		if (verifyData.myFighterVerifyDatas[i].maxHealth >= findIter->second.MaxHealth * 1.1f)
			return false;
	}

	endIter = m_mapEnemyFighterAttr.end();
	for (int i = 0; i < verifyData.competitorFighterNum; ++i)
	{
		findIter = m_mapEnemyFighterAttr.find(verifyData.competitorFighterVerifyDatas[i].heroID);
		if (findIter == endIter)
			return false;
		if (verifyData.competitorFighterVerifyDatas[i].maxHealth >= findIter->second.MaxHealth * 1.1f)
			return false;
	}


	for (auto & iter : m_mapOurFighterAttr)
	{
		iter.second -= m_EnemyDebuffAttr;
	}

	for (auto & iter : m_mapEnemyFighterAttr)
	{
		iter.second -= m_OurDebuffAttr;
	}

	///伤害验证
	float damage;
	const PhysicAttackData* physicData;
	for (int i = 0; i < verifyData.myFighterNum; ++i)
	{
		tempHeroID = verifyData.myFighterVerifyDatas[i].heroID;
		for (int j = 0, count = verifyData.myFighterVerifyDatas[i].physicAttackDataNum; j < count; ++j)
		{
			physicData = &verifyData.myFighterVerifyDatas[i].physicAttackData[j];
			findIter = m_mapEnemyFighterAttr.find(physicData->targetID);
			if (findIter == endIter)
				return false;
			damage = _CalculatePhysicAttackDamage(m_mapOurFighterAttr[tempHeroID], findIter->second);
			damage = physicData->crit ? damage * 2.0f * m_globalConfig.AttackingFaultTolerant : damage * m_globalConfig.AttackingFaultTolerant;
			if (physicData->damage >= damage)
				return false;
		}
	}

	endIter = m_mapOurFighterAttr.end();
	for (int i = 0; i < verifyData.competitorFighterNum; ++i)
	{
		tempHeroID = verifyData.competitorFighterVerifyDatas[i].heroID;
		for (int j = 0, count = verifyData.competitorFighterVerifyDatas[i].physicAttackDataNum; j < count; ++j)
		{
			physicData = &verifyData.competitorFighterVerifyDatas[i].physicAttackData[j];
			findIter = m_mapOurFighterAttr.find(physicData->targetID);
			if (findIter == endIter)
				return false;
			damage = _CalculatePhysicAttackDamage(m_mapEnemyFighterAttr[tempHeroID], findIter->second);
			damage = physicData->crit ? damage * 2.0f * m_globalConfig.GethitFaultTolerant : damage * m_globalConfig.GethitFaultTolerant;
			if (physicData->damage <= damage)
				return false;
		}
	}

	const SkillAttackData* pSkillData;
	const SHeroData* pHeroData;
	/// 验证我方对敌方造成的技能伤害
	endIter = m_mapEnemyFighterAttr.end();
	for (int i = 0; i < verifyData.myFighterNum; ++i)
	{
		tempHeroID = verifyData.myFighterVerifyDatas[i].heroID;
		pHeroData = baseDataManager.GetHero(tempHeroID);
		if (nullptr == pHeroData)
			return false;
		for (int j = 0, count = verifyData.myFighterVerifyDatas[i].skillAttackDataNum; j < count; ++j)
		{
			pSkillData = &verifyData.myFighterVerifyDatas[i].skillAttackData[j];
			findIter = m_mapEnemyFighterAttr.find(pSkillData->targetID);
			if (findIter == endIter)
				return false;
			damage = _CalculateSkillDamage(pHeroData, pSkillData->skillID, m_mapOurFighterAttr[tempHeroID], findIter->second);
			damage = pSkillData->crit ? damage * 2.0f * m_globalConfig.AttackingFaultTolerant : damage * m_globalConfig.AttackingFaultTolerant;
			if (pSkillData->damage >= damage)
				return false;
		}
	}

	/// 获取敌方武将数据
	auto func = [pPlayer, pCompetitorBaseMgr, pCompetitorData](int heroID) -> const SHeroData*
	{
		if (pPlayer)
		{
			return pCompetitorBaseMgr->GetHero(heroID);
		}

		for (int i = 0; i < 5; ++i)
		{
			if (pCompetitorData->m_HeroList[i].m_dwHeroID == heroID)
				return &pCompetitorData->m_HeroList[i];
		}

		return nullptr;
	};

	/// 验证敌方对我方的技能伤害数据
	endIter = m_mapOurFighterAttr.end();
	for (int i = 0; i < verifyData.competitorFighterNum; ++i)
	{
		tempHeroID = verifyData.competitorFighterVerifyDatas[i].heroID;
		pHeroData = func(tempHeroID);
		if (nullptr == pHeroData)
			return false;
		for (int j = 0, count = verifyData.competitorFighterVerifyDatas[i].skillAttackDataNum; j < count; ++j)
		{
			pSkillData = &verifyData.competitorFighterVerifyDatas[i].skillAttackData[j];
			findIter = m_mapOurFighterAttr.find(pSkillData->targetID);
			if (findIter == endIter)
				return false;
			damage = _CalculateSkillDamage(pHeroData, pSkillData->skillID, m_mapEnemyFighterAttr[tempHeroID], findIter->second);
			damage = pSkillData->crit ? damage * 2.0f * m_globalConfig.GethitFaultTolerant : damage * m_globalConfig.GethitFaultTolerant;
			if (pSkillData->damage <= damage)
				return false;
		}
	}

	/// 重置玩家信用值
	LuaFunctor(g_Script, "set_CreditDataValue")[baseDataManager.GetSID()][1][5]();

	return true;
}

bool CBattleVerification::BattleTimeVerification(int heroID, int attackTimes, float roundRunTime, int roundNum, const __time64_t& battleStartTime, const CBaseDataManager& baseDataManager)
{
	_time64(&m_tCurTime);
	float serverSpendTime = difftime(m_tCurTime, battleStartTime);
	auto pHeroConfig = CConfigManager::getSingleton()->GetHeroConfig(heroID);
	if (nullptr == pHeroConfig)
		return false;
	float clientSpendTime = roundRunTime * roundNum + attackTimes * pHeroConfig->AttackCD;
	if (clientSpendTime > serverSpendTime)
	{
		lite::Variant ret;
		LuaFunctor(g_Script, "get_CreditDataValue")[baseDataManager.GetSID()][1](&ret);
		int credit = (int)ret;
		credit -= 1;
		LuaFunctor(g_Script, "set_CreditDataValue")[baseDataManager.GetSID()][1][credit]();

		return credit > 0;
	}

	return true;
}

bool CBattleVerification::CombatPowerVerification(int ourCombatPower, int enemyCombatPower, float percent, const CBaseDataManager& baseDataManager)
{
	if (ourCombatPower >= enemyCombatPower * percent)
		return true;

	lite::Variant ret;
	LuaFunctor(g_Script, "get_CreditDataValue")[baseDataManager.GetSID()][1](&ret);
	int credit = (int)ret;
	credit -= 1;
	LuaFunctor(g_Script, "set_CreditDataValue")[baseDataManager.GetSID()][1][credit]();

	return credit > 0;
}

float CBattleVerification::_CalculateSkillDamage(const SHeroData* pHeroData, DWORD dwSkillID, const CharacterAttrData& attackerAttr, const CharacterAttrData& targetAttr)
{
	if (nullptr == pHeroData)
		return 0.0f;

	DWORD dwSkillLevel = 0;
	for (int i = 0; i < MAX_SKILLNUM; ++i)
	{
		if (dwSkillID == pHeroData->m_SkillInfoList[i].m_dwSkillID)
		{
			dwSkillLevel = pHeroData->m_SkillInfoList[i].m_dwSkillLevel;
			break;
		}
	}

	auto pSkillConfig = CConfigManager::getSingleton()->GetHeroSkillConfig(dwSkillID);
	if (nullptr == pSkillConfig)
		return 0.0f;

	/// 技能效果 = 技能伤害系数 × 英雄法强 + 技能附加初始伤害 + 技能附加升级伤害 × （技能等级 - 1）+ 修正值
	float skillEffect = pSkillConfig->factor * attackerAttr.Magic + pSkillConfig->damage + pSkillConfig->addDamage * (dwSkillLevel - 1) + pSkillConfig->extraDamage;

	float resistance = (targetAttr.Resistance > 0.0f) ? CConfigManager::getSingleton()->GetResistanceRate(targetAttr.Resistance) : 0.0f;
	float fianlResistance = max(0.0f, min(1.0f - (resistance - attackerAttr.OffsetResistance), 1.0f));
	return skillEffect * fianlResistance;
}

float CBattleVerification::_CalculatePhysicAttackDamage(const CharacterAttrData& attackerAttr, const CharacterAttrData& targetAttr)
{
	/// 物理伤害 = 攻击者攻击力 * 攻击者攻击力 /（2 * (受击者物理护甲 - 攻击者物理护甲穿透） + 攻击者攻击力）
	return attackerAttr.Physic * attackerAttr.Physic / (2.0f * max(0.0f, targetAttr.Armor - attackerAttr.OffsetArmor) + attackerAttr.Physic);
}

bool CBattleVerification::_CalculateArenaEnemyFighterAttribute(int arenaID)
{
	CPlayer* pPlayer = (CPlayer*)GetPlayerBySID(arenaID - MAX_ROBOT_NUM)->DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		auto pBaseDataMgr = pPlayer->GetBaseDataMgr();
		if (nullptr == pBaseDataMgr)
			return false;
		auto pDefenseTeam = pBaseDataMgr->GetArenaDefenseTeam();
		if (nullptr == pDefenseTeam)
			return false;

		return _CalculateFighterAttribute(pDefenseTeam, 5, *pBaseDataMgr, m_mapEnemyFighterAttr, m_EnemyBuffAttr, m_EnemyDebuffAttr);
	}
	else
	{
		m_mapEnemyFighterAttr.clear();
		m_EnemyBuffAttr.Reset();
		m_EnemyDebuffAttr.Reset();
		auto pArenaCompiter = CArenaMoudel::getSingleton()->GetManagedArenaData(arenaID);
		if (nullptr == pArenaCompiter)
			return false;
		const SHeroData* pHeroData = nullptr;
		const HeroConfig* pHeroConfig = nullptr;
		DWORD dwHeroID;
		for (int i = 0; i < 5; ++i)
		{
			pHeroData = &pArenaCompiter->m_HeroList[i];
			dwHeroID = pHeroData->m_dwHeroID;
			pHeroConfig = CConfigManager::getSingleton()->GetHeroConfig(dwHeroID);
			if (nullptr == pHeroConfig)
				return false;
			SHeroAttr heroAttr(pHeroConfig->baseAttribute);
			CharacterAttrData selfAttribute;
			/// 计算等级加成属性
			CHeroAttributeCalculator::getSingleton()->CalculateHeroLevelAttribute(dwHeroID, pHeroData->m_dwLevel, pHeroData->m_dwStarLevel, heroAttr.LevelAttribute);
			/// 计算装备加成属性
			CHeroAttributeCalculator::getSingleton()->CalculateHeroEquipmentAttribute(dwHeroID, pHeroData->m_Equipments, heroAttr.EquipmentAttribute);
			/// 计算品阶加成属性
			CHeroAttributeCalculator::getSingleton()->CalculateHeroRankAttribute(dwHeroID, pHeroData->m_dwRankLevel, heroAttr.RankAttribute);
			/// TODO : 计算缘分加成属性

			/// 合计属性
			heroAttr.curAttr += heroAttr.LevelAttribute;
			heroAttr.curAttr += heroAttr.EquipmentAttribute;
			heroAttr.curAttr += heroAttr.RankAttribute;
			heroAttr.curAttr += heroAttr.FateAttribute;

			/// 开放了全部技能
			if (pHeroData->m_dwRankLevel > 6)
			{
				_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[3].m_dwSkillID), pHeroData->m_SkillInfoList[3].m_dwSkillLevel, m_EnemyBuffAttr, m_EnemyDebuffAttr, selfAttribute);
				_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[2].m_dwSkillID), pHeroData->m_SkillInfoList[2].m_dwSkillLevel, m_EnemyBuffAttr, m_EnemyDebuffAttr, selfAttribute);
				_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[1].m_dwSkillID), pHeroData->m_SkillInfoList[1].m_dwSkillLevel, m_EnemyBuffAttr, m_EnemyDebuffAttr, selfAttribute);
				_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[0].m_dwSkillID), pHeroData->m_SkillInfoList[0].m_dwSkillLevel, m_EnemyBuffAttr, m_EnemyDebuffAttr, selfAttribute);
			}
			/// 开放了前三个技能
			else if (pHeroData->m_dwRankLevel > 3)
			{
				_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[2].m_dwSkillID), pHeroData->m_SkillInfoList[2].m_dwSkillLevel, m_EnemyBuffAttr, m_EnemyDebuffAttr, selfAttribute);
				_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[1].m_dwSkillID), pHeroData->m_SkillInfoList[1].m_dwSkillLevel, m_EnemyBuffAttr, m_EnemyDebuffAttr, selfAttribute);
				_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[0].m_dwSkillID), pHeroData->m_SkillInfoList[0].m_dwSkillLevel, m_EnemyBuffAttr, m_EnemyDebuffAttr, selfAttribute);
			}
			/// 开放了前两个技能
			else if (pHeroData->m_dwRankLevel > 1)
			{
				_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[1].m_dwSkillID), pHeroData->m_SkillInfoList[1].m_dwSkillLevel, m_EnemyBuffAttr, m_EnemyDebuffAttr, selfAttribute);
				_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[0].m_dwSkillID), pHeroData->m_SkillInfoList[0].m_dwSkillLevel, m_EnemyBuffAttr, m_EnemyDebuffAttr, selfAttribute);
			}
			/// 开放了第一个技能
			else
			{
				_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[0].m_dwSkillID), pHeroData->m_SkillInfoList[0].m_dwSkillLevel, m_EnemyBuffAttr, m_EnemyDebuffAttr, selfAttribute);
			}

			CHeroAttributeCalculator::getSingleton()->CalculateAttributeEffect(selfAttribute, (HeroType)pHeroConfig->Type);

			selfAttribute += heroAttr.curAttr;
			auto result = m_mapEnemyFighterAttr.insert(make_pair(dwHeroID, selfAttribute));
			if (!result.second)
				return false;
		}

		/// 所有武将属性加上公共属性
		for (auto & iter : m_mapEnemyFighterAttr)
		{
			iter.second += m_EnemyBuffAttr;
		}
	}

	return true;
}

bool CBattleVerification::_CalculateFighterAttribute(const DWORD* fightHeroes, DWORD heroesNum, const CBaseDataManager& baseDataMgr, OUT AttributeMap& attrMap, OUT CharacterAttrData& buffAttr, OUT CharacterAttrData& debuffAttr)
{
	attrMap.clear();
	buffAttr.Reset();
	debuffAttr.Reset();
	const SHeroData* pHeroData = nullptr;
	const SHeroAttr* pHeroAttr = nullptr;
	const HeroConfig* pHeroConfig = nullptr;
	int tempHeroID;
	/// 计算上阵武将技能selfbuff属性以及公共属性
	for (int i = 0; i < heroesNum; ++i)
	{
		tempHeroID = fightHeroes[i];
		if (0 == tempHeroID)
			break;
		pHeroData = baseDataMgr.GetHero(tempHeroID);
		pHeroAttr = baseDataMgr.GetHeroAttr(tempHeroID);

		if (nullptr == pHeroData || nullptr == pHeroAttr)
			return false;

		CharacterAttrData selfAttribute;
		/// 开放了全部技能
		if (pHeroData->m_dwRankLevel > 6)
		{
			_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[3].m_dwSkillID), pHeroData->m_SkillInfoList[3].m_dwSkillLevel, buffAttr, debuffAttr, selfAttribute);
			_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[2].m_dwSkillID), pHeroData->m_SkillInfoList[2].m_dwSkillLevel, buffAttr, debuffAttr, selfAttribute);
			_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[1].m_dwSkillID), pHeroData->m_SkillInfoList[1].m_dwSkillLevel, buffAttr, debuffAttr, selfAttribute);
			_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[0].m_dwSkillID), pHeroData->m_SkillInfoList[0].m_dwSkillLevel, buffAttr, debuffAttr, selfAttribute);
		}
		/// 开放了前三个技能
		else if (pHeroData->m_dwRankLevel > 3)
		{
			_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[2].m_dwSkillID), pHeroData->m_SkillInfoList[2].m_dwSkillLevel, buffAttr, debuffAttr, selfAttribute);
			_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[1].m_dwSkillID), pHeroData->m_SkillInfoList[1].m_dwSkillLevel, buffAttr, debuffAttr, selfAttribute);
			_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[0].m_dwSkillID), pHeroData->m_SkillInfoList[0].m_dwSkillLevel, buffAttr, debuffAttr, selfAttribute);
		}
		/// 开放了前两个技能
		else if (pHeroData->m_dwRankLevel > 1)
		{
			_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[1].m_dwSkillID), pHeroData->m_SkillInfoList[1].m_dwSkillLevel, buffAttr, debuffAttr, selfAttribute);
			_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[0].m_dwSkillID), pHeroData->m_SkillInfoList[0].m_dwSkillLevel, buffAttr, debuffAttr, selfAttribute);
		}
		/// 开放了第一个技能
		else
		{
			_CalculateSkillBuffAttribute(CConfigManager::getSingleton()->GetHeroSkillConfig(pHeroData->m_SkillInfoList[0].m_dwSkillID), pHeroData->m_SkillInfoList[0].m_dwSkillLevel, buffAttr, debuffAttr, selfAttribute);
		}

		pHeroConfig = CConfigManager::getSingleton()->GetHeroConfig(tempHeroID);
		if (pHeroConfig)
			CHeroAttributeCalculator::getSingleton()->CalculateAttributeEffect(selfAttribute, (HeroType)pHeroConfig->Type);

		selfAttribute += pHeroAttr->curAttr;
		auto result = attrMap.insert(make_pair(tempHeroID, selfAttribute));
		if (!result.second)
			return false;
	}

	/// 所有武将属性加上公共属性
	for (auto & iter : attrMap)
	{
		iter.second += buffAttr;
	}

	return true;
}

bool CBattleVerification::_CalculateSkillBuffAttribute(const HeroSkillConfig* pSkillConfig, int skillLevel, CharacterAttrData& sharedBuffAttr, OUT CharacterAttrData& sharedDebuffAttr, OUT CharacterAttrData& selfBuffAttr)
{
	if (nullptr == pSkillConfig)
		return false;

	auto pBuffConfig = CConfigManager::getSingleton()->GetBuffConfig(pSkillConfig->buffID1);
	_CalculateAttributeBuff(pBuffConfig, skillLevel, sharedBuffAttr, sharedDebuffAttr);
	pBuffConfig = CConfigManager::getSingleton()->GetBuffConfig(pSkillConfig->buffID2);
	_CalculateAttributeBuff(pBuffConfig, skillLevel, sharedBuffAttr, sharedDebuffAttr);
	pBuffConfig = CConfigManager::getSingleton()->GetBuffConfig(pSkillConfig->selfBuffID);
	_CalculateAttributeBuff(pBuffConfig, skillLevel, selfBuffAttr, sharedDebuffAttr);
}

bool CBattleVerification::_CalculateAttributeBuff(const BuffConfig* pBuffConfig, int buffLevel, CharacterAttrData& buffAttr, OUT CharacterAttrData& debuffAttr)
{
	if (nullptr == pBuffConfig)
		return false;

	/// pBuffConfig->EffectiveStyle == 0 代表增益buff  否则是减益buff
	CharacterAttrData& finalAttr = pBuffConfig->EffectiveStyle == 0 ? buffAttr : debuffAttr;

	if (pBuffConfig->name == "Power")
	{
		finalAttr.Power = max(finalAttr.Power, pBuffConfig->initiaValue + pBuffConfig->addValue * buffLevel);
	}
	else if (pBuffConfig->name == "Intelligence")
	{
		finalAttr.Intelligence = max(finalAttr.Intelligence, pBuffConfig->initiaValue + pBuffConfig->addValue * buffLevel);
	}
	else if (pBuffConfig->name == "Agility")
	{
		finalAttr.Agility = max(finalAttr.Agility, pBuffConfig->initiaValue + pBuffConfig->addValue * buffLevel);
	}
	else if (pBuffConfig->name == "Physic")
	{
		finalAttr.Physic = max(finalAttr.Physic, pBuffConfig->initiaValue + pBuffConfig->addValue * buffLevel);
	}
	else if (pBuffConfig->name == "Magic")
	{
		finalAttr.Magic = max(finalAttr.Magic, pBuffConfig->initiaValue + pBuffConfig->addValue * buffLevel);
	}
	else if (pBuffConfig->name == "Armor")
	{
		finalAttr.Armor = max(finalAttr.Armor, pBuffConfig->initiaValue + pBuffConfig->addValue * buffLevel);
	}
	else if (pBuffConfig->name == "Resistance")
	{
		finalAttr.Resistance = max(finalAttr.Resistance, pBuffConfig->initiaValue + pBuffConfig->addValue * buffLevel);
	}
	else if (pBuffConfig->name == "Critical")
	{
		finalAttr.Crit = max(finalAttr.Crit, pBuffConfig->initiaValue + pBuffConfig->addValue * buffLevel);
	}
	else if (pBuffConfig->name == "Dodge")
	{
		finalAttr.Dodge = max(finalAttr.Dodge, pBuffConfig->initiaValue + pBuffConfig->addValue * buffLevel);
	}
	else if (pBuffConfig->name == "MaxHealth")
	{
		finalAttr.MaxHealth = max(finalAttr.MaxHealth, pBuffConfig->initiaValue + pBuffConfig->addValue * buffLevel);
	}

	return true;
}
