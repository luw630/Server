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
		string	sName;		//�书����
		int		nId;		//�书ID
		int		nType;		//�书���� 1���� 2�Ṧ 3���� 4�ķ� 5�����书
		//��������
		int		nRequireLevel;	// �ȼ�
		DWORD	nMoneySpend;	// ���Ľ�Ǯ
		int		nRequireTaskId[3];	//��Ҫ�������ID
		pair<int, int>	pairRequireSkill[4];	//��Ҫѧϰ���� ���ܵȼ�
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