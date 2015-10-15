#include "stdafx.h"
#include "BuffInnerDefense.h"
#include "ProtectedDataTable.h"
#include "FightObject.h"

// CBuffInnerDefense::CBuffInnerDefense(CFightObject* pFighter)
// 	:CBuffProtectedBase(pFighter, EPST_INNER_PROTECTED, "少林提升内防"), m_nAddIDValue( 0 )
// {
// }
// 
// void CBuffInnerDefense::OnBegin()
// {
// 	m_nAddIDValue = GetIDValue();
// 	m_pPlayer->m_nFPAddFactor[ CFightObject::EAF_INNER_DEFENSE] += m_nAddIDValue;    
// }
// 
// void CBuffInnerDefense::OnEnd()
// {
// 	m_pPlayer->m_nFPAddFactor[ CFightObject::EAF_INNER_DEFENSE] -= m_nAddIDValue;
// }
// 
// int CBuffInnerDefense::GetIDValue()
// {
// 	const SSkill& skillData = GetSkillData();
// 	const CProtectedDataTable::ProtectedData* proData = CProtectedDataTable::GetData( skillData.wTypeID );
// 	return  ProtectedFormula::GetInnerDefense( proData, skillData.byLevel );
// 
// }