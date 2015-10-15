#include "stdafx.h"
#include "FightObject.h"
#include "BuffReduceCD.h"
#include "FlyDataTable.h"

// CBuffReduceCD::CBuffReduceCD(CFightObject* pFighter)
// :CBuffFlyBase(pFighter, EFST_REDUCE_CD, "��������ȴ"), m_nReduceCD(0)
// {
// }
// 
// void CBuffReduceCD::OnBegin()
// {
// 	m_nReduceCD = GetCDRate();// * ������ȴ
// 	m_pPlayer->m_nFPAddFactor[CFightObject::EAF_CD_RATE ] -= m_nReduceCD;		//ӦΪ�ǽ�������ȴʱ��,�����ü���
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
