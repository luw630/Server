#include "stdafx.h"
#include "FightObject.h"
#include "FlyDataTable.h"
#include "BuffAbsoluteParry.h"

// CBuffAbsoluteParryRate::CBuffAbsoluteParryRate(CFightObject* pFighter)
// :CBuffFlyBase( pFighter, EFST_ABSOLUTE_PARRY, "¼Ó¾ø¶Ô¶ãÉÁ"), m_nAbsoluteParryRate(0)
// {
// }
// 
// void CBuffAbsoluteParryRate::OnBegin()
// {
// 	const int apValue = 0; //xxx
// 	m_nAbsoluteParryRate = GetAPRate();
// 	m_pPlayer->m_nFPAddFactor[ CFightObject::EAF_ABSOLUTE_PARRY] += m_nAbsoluteParryRate;
// 	
// 	AddProficiency( 1 );
// }
// 
// void CBuffAbsoluteParryRate::OnEnd()
// {
// 	m_pPlayer->m_nFPAddFactor[ CFightObject::EAF_ABSOLUTE_PARRY] -= m_nAbsoluteParryRate;
// 	m_nAbsoluteParryRate = 0;
// }
// 
// int CBuffAbsoluteParryRate::GetAPRate()
// {
// 	const SSkill& skillData = GetSkillData();
// 	const CFlyDataTable::SFlyData* flyData = CFlyDataTable::GetData( skillData.wTypeID );
// 	
// 	return FlyFormula::GetAbsoluteParryRate( flyData, skillData.byLevel);
// }