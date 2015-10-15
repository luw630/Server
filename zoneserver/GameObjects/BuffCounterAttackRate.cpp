#include "stdafx.h"

#include "ProtectedDataTable.h"
#include "FightObject.h"
#include "BuffCounterAttackRate.h"

// CBuffCounterAttackRate::CBuffCounterAttackRate(CFightObject* pFighter)
// 	:CBuffProtectedBase(pFighter,EPST_COUNTER_ATTACK_RATE, "·´µ¯ÉËº¦±ÈÀý"), m_nCounterAttackRate(0)
// {
// }
// 
// void CBuffCounterAttackRate::OnBegin()
// {
// 	m_nCounterAttackRate = GetCAValue();
// 	m_pPlayer->m_nFPAddFactor[CFightObject::EAF_COUNTER_ATTACK_RATE] += m_nCounterAttackRate;
// }
// 
// void CBuffCounterAttackRate::OnEnd()
// {
// 	m_pPlayer->m_nFPAddFactor[CFightObject::EAF_COUNTER_ATTACK_RATE] -= m_nCounterAttackRate;
// 	m_nCounterAttackRate = 0;
// }
// 
// DWORD CBuffCounterAttackRate::GetUpLevelProficiency()
// {
// 	const SSkill& skillData = GetSkillData();
// 	const CProtectedDataTable::ProtectedData* proData = CProtectedDataTable::GetData( skillData.wTypeID );
// 	return ProtectedFormula::GetMaxProficiencyCUBE(proData, skillData.byLevel);
// 	
// }
// 
// int CBuffCounterAttackRate::GetCAValue()
// {
// 	const SSkill& skillData = GetSkillData();
// 	const CProtectedDataTable::ProtectedData* proData = CProtectedDataTable::GetData( skillData.wTypeID );
// 	return ProtectedFormula::GetCounterAttackRate( proData, skillData.byLevel );
// }