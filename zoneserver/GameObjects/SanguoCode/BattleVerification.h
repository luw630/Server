// ------------------------------------
// -------- author : LiuWeiWei --------
// -------- date : 07-01-2015 ---------
// -------- desc : 战斗验证类 ----------
// ------------------------------------
#pragma once
#include "PUB\Singleton.h"
#include <list>
#include <unordered_map>
#include "..\PUB\ConstValue.h"
#include "../NETWORKMODULE/HeroTypeDef.h"
#include <time.h>

using namespace std;

struct ArenaBattleVerifyData;
struct SHeroData;
struct HeroSkillConfig;
struct BuffConfig;
struct CGlobalConfig;
class CBaseDataManager;

///@note key : 武将ID, value : 武将最终属性
typedef unordered_map<int, CharacterAttrData> AttributeMap;

class CBattleVerification :
	public CSingleton<CBattleVerification>
{
public:
	CBattleVerification();
	~CBattleVerification();
	///@brief 竞技场战斗验证
	///@param [in] verifyData 用于验证的战斗数据
	///@param [in] battleStartTime 战斗开始时间
	///@param [in] baseDataManager 用于获取待验证方(战斗发起方)的数据管理类
	bool ArenaBattleVerification(const ArenaBattleVerifyData& verifyData, const __time64_t& battleStartTime, const CBaseDataManager& baseDataManager);
private:
	///@brief 战斗时长验证
	///@param [in] heroID 攻击最多次的武将ID
	///@param [in] attackTimes 上述武将攻击次数
	///@param [in] roundRunTime 每回合跑动时间
	///@param [in] roundNum 战斗回合数
	///@param [in] battleStartTime 战斗开始时间
	bool BattleTimeVerification(int heroID, int attackTimes, float roundRunTime, int roundNum, const __time64_t& battleStartTime, const CBaseDataManager& baseDataManager);
	///@brief 战斗力验证
	///@param [in] ourCombatPowr 我方战斗力
	///@param [in] enemyCombatPower 敌方战斗力
	///@param [in] percent 战斗力比较的比例
	bool CombatPowerVerification(int ourCombatPower, int enemyCombatPower, float percent, const CBaseDataManager& baseDataManager);
private:
	///@brief 计算技能攻击伤害
	///@param [in] pHeroData 施法者英雄数据
	///@param [in] dwSkillID 技能ID
	///@param [in] attackerAttr 施法者属性
	///@param [in] targetAttr 受术者属性
	float _CalculateSkillDamage(const SHeroData* pHeroData, DWORD dwSkillID, const CharacterAttrData& attackerAttr, const CharacterAttrData& targetAttr);
	///@brief 计算物理攻击伤害
	///@param [in] attackerAttr 攻击者属性
	///@param [in] targetAttr 受击者属性
	float _CalculatePhysicAttackDamage(const CharacterAttrData& attackerAttr, const CharacterAttrData& targetAttr);
	///@brief 计算竞技场敌方属性
	bool _CalculateArenaEnemyFighterAttribute(int arenaID);
	///@brief 计算参战者属性
	///@param [in] fightHeroes 待计算的武将ID数组
	///@param [in] heroesNum 武将ID数组元素个数
	///@param [in] baseDataMgr 获取待计算武将信息的数据管理类
	///@param [out] attrMap 保存计算好的属性
	///@param [out] buffAttr 技能增益buff属性
	///@param [out] debuffAttr 技能减益buff属性
	bool _CalculateFighterAttribute(const DWORD* fightHeroes, DWORD heroesNum, const CBaseDataManager& baseDataMgr, OUT AttributeMap& attrMap, OUT CharacterAttrData& buffAttr, OUT CharacterAttrData& debuffAttr);
	///@brief 计算技能buff属性
	///@param [in] pSkillConfig 技能配置
	///@param [in] skillLevel 技能等级
	///@param [out] sharedBuffAttr 共享的增益buff属性
	///@param [out] sharedDebuffAttr 共享的减益buff属性
	///@param [out] selfBuffAttr 仅作用于武将自身的buff属性
	bool _CalculateSkillBuffAttribute(const HeroSkillConfig* pSkillConfig, int skillLevel, OUT CharacterAttrData& sharedBuffAttr, OUT CharacterAttrData& sharedDebuffAttr, OUT CharacterAttrData& selfBuffAttr);
	///@brief 计算属性buff值
	///@param [in] pBuffConfig buff配置
	///@param [in] buffLevel buff等级
	///@param [out] buffAttr 计算好的buff属性
	///@param [out] debuffAttr 计算好的debuff属性
	bool _CalculateAttributeBuff(const BuffConfig* pBuffConfig, int buffLevel, OUT CharacterAttrData& buffAttr, OUT CharacterAttrData& debuffAttr);
private:
	AttributeMap m_mapOurFighterAttr;		///< 我方参战者属性
	CharacterAttrData m_OurBuffAttr;	///< 我方技能的增益buff属性
	CharacterAttrData m_OurDebuffAttr;	///< 我方技能的减益buff属性
	AttributeMap m_mapEnemyFighterAttr;	///< 敌方参战者属性
	CharacterAttrData m_EnemyBuffAttr;		///< 敌方技能的增益buff属性
	CharacterAttrData m_EnemyDebuffAttr;	///< 地方技能的减益buff属性
	__time64_t m_tCurTime;
	CGlobalConfig& m_globalConfig;
};

