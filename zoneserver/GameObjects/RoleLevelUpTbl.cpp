#include "stdafx.h"

#include "pub\dwt.h"
#include "networkmodule\PlayerTypedef.h"
#include "RoleLevelUpTbl.h"

//---------------------------------------------------------
CRoleLevelUpTbl::CRoleLevelUpTbl()
{
	m_dataTbl = NULL;
	LoadRoleLevelUpTbl();
}

//---------------------------------------------------------
CRoleLevelUpTbl::~CRoleLevelUpTbl()
{
	if(m_dataTbl)
	{
		delete[] m_dataTbl;
		m_dataTbl = NULL;
	}
}

//---------------------------------------------------------
CRoleLevelUpTbl* CRoleLevelUpTbl::GetSingleton()
{
	static CRoleLevelUpTbl s;
	return &s;
}

//---------------------------------------------------------
bool CRoleLevelUpTbl::LoadRoleLevelUpTbl()
{
	//m_dataTbl = g_cData;
// 	dwt::ifstream stream( "data\\角色升级表.txt" );
// 	if ( !stream.is_open() )
// 		return false;
// 
// 	char str[ 0xff ];
// 	while(!stream.eof())
// 	{
// 		stream >> str;
// 		if(strcmp(str,"·") == 0)
// 			break;
// 	}
// 	
// 	DWORD nTimes = 0;
// 
// 	stream >> m_totalLevel;
// 	m_dataTbl = new SDataTbl[ m_totalLevel ];
// 
// 	int tempValue = 0;
// 	int useValue = 0;
//  	while( nTimes < m_totalLevel)
//  	{
// 		stream >> tempValue;		//等级
// 		stream >> m_dataTbl[ nTimes ].m_Blood;	
// 		stream >> m_dataTbl[ nTimes ].m_InnerForce;	
// 		stream >> m_dataTbl[ nTimes ].m_VitalForce;
// 		stream >> m_dataTbl[ nTimes ].m_KillForce;
// 		stream >> m_dataTbl[ nTimes ].m_wAttackDefenseBase[ SPlayerXiaYiShiJie::OUT_ATTACK_INDEX ];
// 		stream >> m_dataTbl[ nTimes ].m_wAttackDefenseBase[ SPlayerXiaYiShiJie::INNER_ATTACK_INDEX ];
// 		stream >> m_dataTbl[ nTimes ].m_wAttackDefenseBase[ SPlayerXiaYiShiJie::OUT_DEFENSE_INDEX ];
// 		stream >> m_dataTbl[ nTimes ].m_wAttackDefenseBase[ SPlayerXiaYiShiJie::INNER_DEFENSE_INDEX ];
// 		stream >> m_dataTbl[ nTimes ].m_BodyWork;
// 		stream >> m_dataTbl[ nTimes ].m_VenapointCount;
// 		stream >> m_dataTbl[ nTimes ].m_wAddPoints[ 0 ];
// 		stream >> m_dataTbl[ nTimes ].m_wAddPoints[ 1 ];
// 		stream >> m_dataTbl[ nTimes ].m_MaxExp;
// 
// 		++nTimes;
//  	}
// 
//  	stream.close();

	return true;
}