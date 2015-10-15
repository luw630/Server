#include "stdafx.h"
#include "FightObject.h"
#include "FlyDataTable.h"
#include "BuffJump.h"

// CBuffJump::CBuffJump(CFightObject* pFighter)
// 	:CBuffFlyBase(pFighter, EFST_JUMP, "少林跳跃")
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
// 	//瞬发技能，特殊处理，返回一
// 	return 1;
// }
// 
// void CBuffJump::OnLevelUp()
// {
// 	//修改player最大跳跃个数 GetMaxJumpTile()
// 	m_pPlayer->m_byJumpMaxTile = GetMaxJumpTile();
// 
// 	SendPropertyChangedMsg();
// }
