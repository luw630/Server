#pragma once

using std::string;
using std::map;
using std::vector;

#define GET_SKILL_KEY( type, id )	((type)*100000+ id)
#define GET_SKILL_TYPE( key )		( ((key)/100000) )
#define GET_SKILL_ID(key)			((key)%100000)

class CSkillStudyTable
{
public:
	struct SSkillCondition
	{
		string	sName;		//武功名称
		int		nId;		//武功ID
		int		nType;		//武功类型 1技能 2轻功 3护体 4心法 5独门武功
		//接受条件
		int		nRequireLevel;	// 等级
		DWORD	nMoneySpend;	// 消耗金钱
		int		nRequireTaskId[3];	//需要完成任务ID
		pair<int, int>	pairRequireSkill[4];	//需要学习过的 技能等级
		int		nWeapon;
		int		nActionId;
		struct  Effect
		{
			string name;
			int		height;            
		} effect;
		string	strSoundFileName;
	};

	typedef map<int, SSkillCondition> TSKillConditionMap;

	static bool	LoadFromFile( LPSTR pszFileName );
	static int	GetSkillCounter() { return (int)m_mSkillConditions.size(); }

	static const SSkillCondition* GetSKillConditionById( int nSkillID );

	static const TSKillConditionMap& GetRawMap() { return m_mSkillConditions; }
private:
	static TSKillConditionMap m_mSkillConditions;
};