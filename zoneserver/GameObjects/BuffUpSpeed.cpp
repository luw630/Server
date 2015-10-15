#include "stdafx.h"
#include "FightObject.h"
#include "BuffUpSpeed.h"
#include "FlyDataTable.h"

// CBuffUpSpeedModify::CBuffUpSpeedModify(CFightObject* pFighter)
// 	:CBuffFlyBase( pFighter,EFST_UP_SPEED, "少林提速" ), m_nUpSpeedValue( 0 )
// {
// }
// 
// int CBuffUpSpeedModify::GetSpeedUpVale()
// {
// 	const SSkill& skillData = GetSkillData();
// 	const CFlyDataTable::SFlyData* flyData = CFlyDataTable::GetData( skillData.wTypeID );
// 
// 	return FlyFormula::GetUpSpeed( flyData, skillData.byLevel );
// }
// 
// void CBuffUpSpeedModify::OnEnd()
// {
// 	//还原提速数据
// 	m_pPlayer->m_nFPAddFactor[CFightObject::EAF_SPEED ] -= m_nUpSpeedValue;
// 	m_nUpSpeedValue = 0;
// }
// 
// void CBuffUpSpeedModify::OnBegin()
// {
// 	//计算提速值
// 	m_nUpSpeedValue = GetSpeedUpVale();
// 	m_pPlayer->m_nFPAddFactor[CFightObject::EAF_SPEED ] += m_nUpSpeedValue;
// }
