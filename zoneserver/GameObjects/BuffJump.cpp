#include "stdafx.h"
#include "FightObject.h"
#include "FlyDataTable.h"
#include "BuffJump.h"

// CBuffJump::CBuffJump(CFightObject* pFighter)
// 	:CBuffFlyBase(pFighter, EFST_JUMP, "������Ծ")
// {
// }
// 
// void CBuffJump::OnBegin()
// {
// 	//assert(0);
// 	AddProficiency(1);
// }
// 
// int CBuffJump::GetMaxJumpTile()
// {
// 	const SSkill& skillData = GetSkillData();
// 	const CFlyDataTable::SFlyData* flyData = CFlyDataTable::GetData( skillData.wTypeID );
// 	
// 	return FlyFormula::GetMaxJumpTile( flyData, skillData.byLevel );
// }
// 
// int CBuffJump::GetRemainTime()
// {
// 	//˲�����ܣ����⴦������һ
// 	return 1;
// }
// 
// void CBuffJump::OnLevelUp()
// {
// 	//�޸�player�����Ծ���� GetMaxJumpTile()
// 	m_pPlayer->m_byJumpMaxTile = GetMaxJumpTile();
// 
// 	SendPropertyChangedMsg();
// }
