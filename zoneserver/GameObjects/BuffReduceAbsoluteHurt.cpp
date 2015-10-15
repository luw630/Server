#include "stdafx.h"
#include "ProtectedDataTable.h"
#include "BuffReduceAbsoluteHurt.h"
#include "FightObject.h"

// CBuffAbsoluteHurt::CBuffAbsoluteHurt(CFightObject* pFighter)
// 	:CBuffProtectedBase(pFighter, EPST_ABSOLUTE_HURT, "¼õÃâ¾ø¶ÔÉËº¦" ), m_nAbsolutHurtReduceValue( 0 )
// {
// }
// 
// void CBuffAbsoluteHurt::OnBegin()
// {
// 	m_nAbsolutHurtReduceValue = GetRAHValue();
// 	m_pPlayer->m_nFPAddFactor[ CFightObject::EAF_ABSOLUTE_HURT] += m_nAbsolutHurtReduceValue;
// }
// 
// void CBuffAbsoluteHurt::OnEnd()
// {
// 	m_pPlayer->m_nFPAddFactor[ CFightObject::EAF_ABSOLUTE_HURT] -= m_nAbsolutHurtReduceValue;
//     m_nAbsolutHurtReduceValue = 0;
// }
// 
// DWORD CBuffAbsoluteHurt::GetUpLevelProficiency()
// {
// 	const SSkill& skillData = GetSkillData();
// 	const CProtectedDataTable::ProtectedData* proData = CProtectedDataTable::GetData( skillData.wTypeID );
// 	return ProtectedFormula::GetMaxProficiencyCUBE(proData, skillData.byLevel);
// }
// 
// int CBuffAbsoluteHurt::GetRAHValue()
// {
// 	const SSkill& skillData = GetSkillData();
// 	const CProtectedDataTable::ProtectedData* proData = CProtectedDataTable::GetData( skillData.wTypeID );
// 
// 	return ProtectedFormula::GetReduceAbsoluteHurt( proData, skillData.byLevel );
// }