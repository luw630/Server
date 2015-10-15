// ------------------------------------
// -------- author : LiuWeiWei --------
// -------- date : 07-01-2015 ---------
// -------- desc : ս����֤�� ----------
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

///@note key : �佫ID, value : �佫��������
typedef unordered_map<int, CharacterAttrData> AttributeMap;

class CBattleVerification :
	public CSingleton<CBattleVerification>
{
public:
	CBattleVerification();
	~CBattleVerification();
	///@brief ������ս����֤
	///@param [in] verifyData ������֤��ս������
	///@param [in] battleStartTime ս����ʼʱ��
	///@param [in] baseDataManager ���ڻ�ȡ����֤��(ս������)�����ݹ�����
	bool ArenaBattleVerification(const ArenaBattleVerifyData& verifyData, const __time64_t& battleStartTime, const CBaseDataManager& baseDataManager);
private:
	///@brief ս��ʱ����֤
	///@param [in] heroID �������ε��佫ID
	///@param [in] attackTimes �����佫��������
	///@param [in] roundRunTime ÿ�غ��ܶ�ʱ��
	///@param [in] roundNum ս���غ���
	///@param [in] battleStartTime ս����ʼʱ��
	bool BattleTimeVerification(int heroID, int attackTimes, float roundRunTime, int roundNum, const __time64_t& battleStartTime, const CBaseDataManager& baseDataManager);
	///@brief ս������֤
	///@param [in] ourCombatPowr �ҷ�ս����
	///@param [in] enemyCombatPower �з�ս����
	///@param [in] percent ս�����Ƚϵı���
	bool CombatPowerVerification(int ourCombatPower, int enemyCombatPower, float percent, const CBaseDataManager& baseDataManager);
private:
	///@brief ���㼼�ܹ����˺�
	///@param [in] pHeroData ʩ����Ӣ������
	///@param [in] dwSkillID ����ID
	///@param [in] attackerAttr ʩ��������
	///@param [in] targetAttr ����������
	float _CalculateSkillDamage(const SHeroData* pHeroData, DWORD dwSkillID, const CharacterAttrData& attackerAttr, const CharacterAttrData& targetAttr);
	///@brief �����������˺�
	///@param [in] attackerAttr ����������
	///@param [in] targetAttr �ܻ�������
	float _CalculatePhysicAttackDamage(const CharacterAttrData& attackerAttr, const CharacterAttrData& targetAttr);
	///@brief ���㾺�����з�����
	bool _CalculateArenaEnemyFighterAttribute(int arenaID);
	///@brief �����ս������
	///@param [in] fightHeroes ��������佫ID����
	///@param [in] heroesNum �佫ID����Ԫ�ظ���
	///@param [in] baseDataMgr ��ȡ�������佫��Ϣ�����ݹ�����
	///@param [out] attrMap �������õ�����
	///@param [out] buffAttr ��������buff����
	///@param [out] debuffAttr ���ܼ���buff����
	bool _CalculateFighterAttribute(const DWORD* fightHeroes, DWORD heroesNum, const CBaseDataManager& baseDataMgr, OUT AttributeMap& attrMap, OUT CharacterAttrData& buffAttr, OUT CharacterAttrData& debuffAttr);
	///@brief ���㼼��buff����
	///@param [in] pSkillConfig ��������
	///@param [in] skillLevel ���ܵȼ�
	///@param [out] sharedBuffAttr ���������buff����
	///@param [out] sharedDebuffAttr ����ļ���buff����
	///@param [out] selfBuffAttr ���������佫�����buff����
	bool _CalculateSkillBuffAttribute(const HeroSkillConfig* pSkillConfig, int skillLevel, OUT CharacterAttrData& sharedBuffAttr, OUT CharacterAttrData& sharedDebuffAttr, OUT CharacterAttrData& selfBuffAttr);
	///@brief ��������buffֵ
	///@param [in] pBuffConfig buff����
	///@param [in] buffLevel buff�ȼ�
	///@param [out] buffAttr ����õ�buff����
	///@param [out] debuffAttr ����õ�debuff����
	bool _CalculateAttributeBuff(const BuffConfig* pBuffConfig, int buffLevel, OUT CharacterAttrData& buffAttr, OUT CharacterAttrData& debuffAttr);
private:
	AttributeMap m_mapOurFighterAttr;		///< �ҷ���ս������
	CharacterAttrData m_OurBuffAttr;	///< �ҷ����ܵ�����buff����
	CharacterAttrData m_OurDebuffAttr;	///< �ҷ����ܵļ���buff����
	AttributeMap m_mapEnemyFighterAttr;	///< �з���ս������
	CharacterAttrData m_EnemyBuffAttr;		///< �з����ܵ�����buff����
	CharacterAttrData m_EnemyDebuffAttr;	///< �ط����ܵļ���buff����
	__time64_t m_tCurTime;
	CGlobalConfig& m_globalConfig;
};

