#include "Stdafx.h"
#include <utility>
#include "CMonsterDefine.h"
#include "CMonsterService.h"

#pragma warning(push)
#pragma warning(disable:4996)

CMonsterService::CMonsterService()
{
}

CMonsterService::~CMonsterService()
{
	CleanUp();
}

void CMonsterService::CleanUp()
{
	// 清除怪物配置信息
	for (MonsterBaseData::iterator it = m_baseData.begin(); it != m_baseData.end(); ++it)
	{
		SMonsterBaseData *temp = it->second;
		delete temp;
	}

	m_baseData.clear();
}

bool CMonsterService::ReLoad()
{
	CleanUp();
	Init();

	return true;
}

bool CMonsterService::Init()
{
	bool result = true;

	result = result && LoadMonsterBaseData();
	

	return result;
}

bool CMonsterService::LoadMonsterBaseData()
{
	dwt::ifstream stream("Data\\怪物\\怪物配置表.txt");
	if (!stream.is_open())
	{
		rfalse("无法打开怪物配置表！");
		return false;
	}

	char buffer[2048];
	SMonsterBaseData *pData = 0;

	while (!stream.eof())
	{
		stream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pData = new SMonsterBaseData;
		DWORD temp = 0;

		linebuf >> pData->m_ID >> pData->m_Name;
		
		pData->m_LookID				= ((linebuf >> temp), temp); temp = 0;
		pData->m_level				= ((linebuf >> temp), temp); temp = 0;
		pData->m_MaxHP				= ((linebuf >> temp), temp); temp = 0;
		pData->m_MaxMP				= ((linebuf >> temp), temp); temp = 0;
		pData->m_Orbit				= ((linebuf >> temp), temp); temp = 0;
		pData->m_WalkSpeed			= ((linebuf >> temp), temp); temp = 0;
		pData->m_FightSpeed			= ((linebuf >> temp), temp); temp = 0;
		pData->m_ReBirthInterval	= ((linebuf >> temp), temp); temp = 0;
		pData->m_PartolRadius		= ((linebuf >> temp), temp); temp = 0;
		pData->m_BodySize			= ((linebuf >> temp), temp); temp = 0;
		pData->m_ActiveRadius		= ((linebuf >> temp), temp); temp = 0;
		pData->m_TailLength			= ((linebuf >> temp), temp); temp = 0;
		pData->m_GongJi				= ((linebuf >> temp), temp); temp = 0;
		pData->m_FangYu				= ((linebuf >> temp), temp); temp = 0;
		pData->m_ShanBi				= ((linebuf >> temp), temp); temp = 0;
		pData->m_BaoJi				= ((linebuf >> temp), temp); temp = 0;
		pData->m_AtkSpeed			= ((linebuf >> temp), temp); temp = 0;
		pData->m_LevelType			= ((linebuf >> temp), temp); temp = 0;
		pData->m_AttType			= ((linebuf >> temp), temp); temp = 0;
		pData->m_Exp				= ((linebuf >> temp), temp); temp = 0;
		pData->m_Sp					= ((linebuf >> temp), temp); temp = 0;
		pData->m_BloodMute			= ((linebuf >> temp), temp); temp = 0;
		pData->m_AttMute			= ((linebuf >> temp), temp); temp = 0;
		pData->m_DefenceMute		= ((linebuf >> temp), temp); temp = 0;
		pData->m_AttInterval		= ((linebuf >> temp), temp); temp = 0;
		pData->m_KillTaskID			= ((linebuf >> temp), temp); temp = 0;
		pData->m_Direction			= ((linebuf >> temp), temp); temp = 0;
		pData->m_EndTailCondition		= ((linebuf >> temp), temp); temp = 0;
		pData->m_MoveType				= ((linebuf >> temp), temp); temp = 0;
		pData->m_SearchTargetInterval	= ((linebuf >> temp), temp); temp = 0;
		pData->m_StayTimeMin		= ((linebuf >> temp), temp); temp = 0;
		pData->m_StayTimeMax		= ((linebuf >> temp), temp); temp = 0;

		///读取怪物技能ID
		int i=0;
		while(i<MAX_MONTER_SKILLCOUNT)
		{
			pData->m_SkillID[i]		= ((linebuf >> temp), temp); temp = 0;
			i++;
		}
		pData->m_StaticAI			= ((linebuf >> temp), temp); temp = 0;	
		pData->m_GroupID			= ((linebuf >> temp), temp); temp = 0;			
		pData->m_uncrit				= ((linebuf >> temp), temp); temp = 0;
		pData->m_wreck				 = ((linebuf >> temp), temp); temp = 0;
		pData->m_unwreck			= ((linebuf >> temp), temp); temp = 0;
		pData->m_puncture			 = ((linebuf >> temp), temp); temp = 0;
		pData->m_unpuncture		= ((linebuf >> temp), temp); temp = 0;
		pData->m_Hit					= ((linebuf >> temp), temp); temp = 0;

		std::pair<MonsterBaseData::iterator, bool> result;
		result = m_baseData.insert(make_pair(pData->m_ID, pData));

		if (false == result.second)
		{
			rfalse("发现了一个重复的怪物ID号！");
			delete pData;
		}
	}
	
	stream.close();
	return true;
}

const SMonsterBaseData* CMonsterService::GetMonsterBaseData(WORD index) const
{
	MonsterBaseData::const_iterator monster = m_baseData.find(index);
	if (monster != m_baseData.end())
		return monster->second;
	else
		return 0;
}

#pragma warning(pop)