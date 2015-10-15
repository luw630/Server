#include "Stdafx.h"
#include "DSkillStudyTable.h"

using std::stringstream;

CSkillStudyTable::TSKillConditionMap CSkillStudyTable::m_mSkillConditions;

bool CSkillStudyTable::LoadFromFile( LPSTR pszFileName )
{
	PakFile pkFile;
	if ( pkFile.Open( pszFileName ) )
	{
		DWORD size = pkFile.Size();
		char *p = new char[ size ];
		pkFile.Read( p, size );
		pkFile.Close();

		char szBuffer[1024];
		strstream FileStream( p, size, ios_base::in );
		
		//开始符
		while( !FileStream.eof() ) {
			FileStream.getline( szBuffer, 1024);
			if ( 0==strncmp( szBuffer, "·", 2 ) )
				break;
		}
		if ( FileStream.eof() )
			return false;

		while( !FileStream.eof() ) {
			FileStream.getline( szBuffer, 1024);
			if ( 0==szBuffer[0] )
				continue;

			SSkillCondition sSkillDesc;
			strstream ss( szBuffer, (std::streamsize)strlen(szBuffer) );

			ss >> sSkillDesc.sName
				>> sSkillDesc.nId >> sSkillDesc.nType >> sSkillDesc.nRequireLevel >> sSkillDesc.nMoneySpend
				>> sSkillDesc.nRequireTaskId[0] >> sSkillDesc.nRequireTaskId[1] >> sSkillDesc.nRequireTaskId[2]
				>> sSkillDesc.pairRequireSkill[0].first >> sSkillDesc.pairRequireSkill[0].second
				>> sSkillDesc.pairRequireSkill[1].first >> sSkillDesc.pairRequireSkill[1].second
				>> sSkillDesc.pairRequireSkill[2].first >> sSkillDesc.pairRequireSkill[2].second
				>> sSkillDesc.pairRequireSkill[3].first >> sSkillDesc.pairRequireSkill[3].second
				>> sSkillDesc.nWeapon >> sSkillDesc.nActionId 
				>> sSkillDesc.effect.name >> sSkillDesc.effect.height
				>> sSkillDesc.strSoundFileName;
			--sSkillDesc.nId;
			m_mSkillConditions[GET_SKILL_KEY(sSkillDesc.nType, sSkillDesc.nId) ] = sSkillDesc;
		}

		delete[] p;
		return true;
	}// else
	//	rfalse( 2, 1, "无法打开Data\\SkillStudy.txt 配置文件");
		
	return false;
}

const CSkillStudyTable::SSkillCondition* CSkillStudyTable::GetSKillConditionById( int nSkillID )
{
	TSKillConditionMap::const_iterator citr = m_mSkillConditions.find( nSkillID );
	if ( citr != m_mSkillConditions.end())
		return &citr->second;
	else
		return 0;
}

