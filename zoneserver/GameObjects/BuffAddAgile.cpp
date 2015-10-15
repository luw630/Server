#include "stdafx.h"
#include "FightObject.h"
#include "BuffAddAgile.h"
#include "FlyDataTable.h"

// CBuffAddAgile::CBuffAddAgile(CFightObject* pFighter)
// 	:CBuffFlyBase(pFighter, EFST_ADD_AGILE, "少林增加身法"), m_nAgileValue(0)
// {
// }
// 
// void CBuffAddAgile::OnBegin()
// {
// 	m_nAgileValue = GetAgileValue();
// 	m_pPlayer->m_nFPAddFactor[CFightObject::EAF_AGILE ] += m_nAgileValue;
// 	
// 	AddProficiency(1);
// }
// 
// void CBuffAddAgile::OnEnd()
// {
// 	m_pPlayer->m_nFPAddFactor[ CFightObject::EAF_AGILE ] -=  m_nAgileValue;
// 
// 	m_nAgileValue = 0;
// }
// 
// int CBuffAddAgile::GetAgileValue()
// {
// 	const SSkill& skillData = GetSkillData();
// 	const CFlyDataTable::SFlyData* flyData = CFlyDataTable::GetData( skillData.wTypeID );
// 	
// 	return FlyFormula::GetAgile( flyData, skillData.byLevel );
// }
