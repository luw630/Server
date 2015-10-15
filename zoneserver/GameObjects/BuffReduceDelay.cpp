#include "stdafx.h"
#include "FightObject.h"
#include "FlyDataTable.h"
#include "BuffReduceDelay.h"

// CBuffReduceMyDelay::CBuffReduceMyDelay(CFightObject* pFighter)
// 	:CBuffFlyBase(pFighter, EFST_REDUCE_MY_DELAY, "降个体冷却"), m_nReduceDelayRate(0)
// {
// }
// 
// void CBuffReduceMyDelay::OnBegin()
// {
//     m_nReduceDelayRate =  GetMDRate();
// 	
// 	m_pPlayer->m_nFPAddFactor[CFightObject::EAF_MY_DELAY_RATE] -= m_nReduceDelayRate;  //应为是降个体冷却时间,所以用减法
// 	AddProficiency(1);
// }
// 
// void CBuffReduceMyDelay::OnEnd()
// {
// 	m_pPlayer->m_nFPAddFactor[CFightObject::EAF_MY_DELAY_RATE] += m_nReduceDelayRate;
// }
// 
// int CBuffReduceMyDelay::GetMDRate()
// {
// 	const SSkill& skillData = GetSkillData();
// 	const CFlyDataTable::SFlyData* flyData = CFlyDataTable::GetData( skillData.wTypeID );
// 	
// 	return FlyFormula::GetMyColdRate( flyData, skillData.byLevel );
// }