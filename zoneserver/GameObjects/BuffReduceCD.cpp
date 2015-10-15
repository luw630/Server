#include "stdafx.h"
#include "FightObject.h"
#include "BuffReduceCD.h"
#include "FlyDataTable.h"

// CBuffReduceCD::CBuffReduceCD(CFightObject* pFighter)
// :CBuffFlyBase(pFighter, EFST_REDUCE_CD, "降公共冷却"), m_nReduceCD(0)
// {
// }
// 
// void CBuffReduceCD::OnBegin()
// {
// 	m_nReduceCD = GetCDRate();// * 公共冷却
// 	m_pPlayer->m_nFPAddFactor[CFightObject::EAF_CD_RATE ] -= m_nReduceCD;		//应为是降公共冷却时间,所以用减法
// 
// 	AddProficiency( 1 );
// }
// 
// void CBuffReduceCD::OnEnd()
// {
// 	m_pPlayer->m_nFPAddFactor[ CFightObject::EAF_CD_RATE ] +=  m_nReduceCD;
// 
// 	m_nReduceCD = 0;
// }
// 
// int CBuffReduceCD::GetCDRate()
// {
// 	const SSkill& skillData = GetSkillData();
// 	const CFlyDataTable::SFlyData* flyData = CFlyDataTable::GetData( skillData.wTypeID );
// 	
// 	return FlyFormula::GetCDRate( flyData, skillData.byLevel );
// }
