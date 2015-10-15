#include "stdafx.h"
#include "BuffOutDefense.h"
#include "ProtectedDataTable.h"
#include "FightObject.h"

// CBuffOutDefense::CBuffOutDefense(CFightObject* pFighter)
// 	:CBuffProtectedBase( pFighter, EPST_OUT_PROTECTED, "少林提升外防" ), m_nAddODValue( 0 )
// {
// }
// 
// void CBuffOutDefense::OnBegin()
// {
// 	m_nAddODValue = GetODValue();
// 	m_pPlayer->m_nFPAddFactor[ CFightObject::EAF_OUT_DEFENSE] += m_nAddODValue;
// }
// 
// void CBuffOutDefense::OnEnd()
// {
// 	m_pPlayer->m_nFPAddFactor[ CFightObject::EAF_OUT_DEFENSE] -= m_nAddODValue;
// }
// 
// int CBuffOutDefense::GetODValue()
// {
// 	const SSkill& skillData = GetSkillData();
// 	const CProtectedDataTable::ProtectedData* proData = CProtectedDataTable::GetData( skillData.wTypeID );
// 	return ProtectedFormula::GetOutDefense( proData, skillData.byLevel );
// }
