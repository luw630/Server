#include "stdafx.h"
#include "ProtectedDataTable.h"
#include "FightObject.h"
#include "BuffUpHPLimit.h"

// CBuffUpHPLimit::CBuffUpHPLimit(CFightObject* pFighter)
// 	:CBuffProtectedBase(pFighter, EPST_HP_LIMITED, "增加生命上限"), m_nMaxHPValue(0)
// {
// 
// }
// 
// void CBuffUpHPLimit::OnBegin()
// {
// 	m_nMaxHPValue = GetMaxHPUpValue();
// 	m_pPlayer->m_nFPAddFactor[ CFightObject::EAF_MAX_HP] += m_nMaxHPValue;
// }
// 
// void CBuffUpHPLimit::OnEnd()
// {
// 	m_pPlayer->m_nFPAddFactor[ CFightObject::EAF_MAX_HP] -= m_nMaxHPValue;
// 	m_nMaxHPValue =0;
// }
// 
// int CBuffUpHPLimit::GetMaxHPUpValue()
// {
// 	const SSkill& skillData = GetSkillData();
// 	const CProtectedDataTable::ProtectedData* proData = CProtectedDataTable::GetData( skillData.wTypeID );
// 	
// 	return ProtectedFormula::GetUpHPLimit( proData, skillData.byLevel );
// }