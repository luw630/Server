#include "stdafx.h"
#include "ProtectedDataTable.h"
#include "FightObject.h"
#include "BuffReduceHurtRate.h"

// CBuffReduceHurtRate::CBuffReduceHurtRate(CFightObject* pFighter)
// 	:CBuffProtectedBase(pFighter, EPST_HURT_RATE, "¼õÃâÉËº¦±ÈÀý" ), m_nReduceHurtRate( 0 )
// {
// }
// 
// void CBuffReduceHurtRate::OnBegin()
// {
// 	m_nReduceHurtRate = GetReduceHurtRate();
// 	m_pPlayer->m_nFPAddFactor[ CFightObject::EAF_HURT_RATE ] += m_nReduceHurtRate;
// }
// 
// void CBuffReduceHurtRate::OnEnd()
// {
// 	m_pPlayer->m_nFPAddFactor[ CFightObject::EAF_HURT_RATE ] -= m_nReduceHurtRate;
// 	m_nReduceHurtRate = 0;
// }
// 
// int CBuffReduceHurtRate::GetReduceHurtRate()
// {
// 	const SSkill& skillData = GetSkillData();
// 	const CProtectedDataTable::ProtectedData* proData = CProtectedDataTable::GetData( skillData.wTypeID );
// 	
// 	return ProtectedFormula::GetReduceHurtRate( proData, skillData.byLevel );
// }
// 
// DWORD CBuffReduceHurtRate::GetUpLevelProficiency()
// {
// 	const SSkill& skillData = GetSkillData();
// 	const CProtectedDataTable::ProtectedData* proData = CProtectedDataTable::GetData( skillData.wTypeID );
// 	return ProtectedFormula::GetMaxProficiencyCUBE(proData, skillData.byLevel);
// }