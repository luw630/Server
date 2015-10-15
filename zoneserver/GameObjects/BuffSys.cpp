#include "stdafx.h"
#include "BuffSys.h"
#include "EventMgr.h"
#include "BuffManager.h"
#include "FightObject.h"
#include "CBuffDefine.h"
#include "CBuffService.h"
#include "ScriptManager.h"
#include "Region.h"
#include "Area.h"
#include <list>
#include "FightObject.h"
#include "CSkillService.h"
void BuffModify::OnActive()
{
	if (m_ActedTimes+1 == m_WillActTimes)
		m_owner->m_buffMgr.ClearStub(this, false);
	else
	{
		m_curStep = m_ActedTimes;
		EventManager::GetInstance().Register(this, m_Margin);
		OnTimer(m_curStep);
	}
}

void BuffModify::OnCancel()
{
	if (m_owner)
	{
		m_owner->m_buffMgr.ClearStub(this, true);
	}
}

void BuffModify::SelfDestory()
{
	if (!m_owner)
	{
		rfalse(4,1,"BuffModify::SelfDestory()");
		return;
	}

	if (m_CanDropBuff)
	{
		m_owner->m_buffMgr.DeleteBuff(m_ID);
		delete this;
	}
}

void BuffModify::SendNotify(bool init)
{
	if (!m_owner)
	{
		rfalse(4,1,"BuffModify::SendNotify()");
		return;
	}
	SUpdateBuff msg;
	msg.dwGlobalID		= m_owner->GetGID();
	msg.dwBuffID		= m_ID;
	msg.dwMargin		= m_Margin;
	msg.dwWillActTimes	= m_WillActTimes;
	msg.m_curStep		= m_curStep;
	msg.bType			= m_Type;
	msg.wReginBuffHeight = m_ReginBuffHeight;
	msg.wReginBuffWidth = m_ReginBuffWidth;
	msg.bReginBuffShape = m_ReginBuffShape;
	msg.fCenterPosX = m_fCenterPosX;
	msg.fCenterPosY = m_fCenterPosY;
	if (m_owner->m_ParentArea)
	{
		m_owner->m_ParentArea->SendAdj(&msg, sizeof(msg), -1);
	}
		
	//m_owner->SendMsg(&msg, sizeof(msg));
}

bool BuffModify::InitBuffData(const SBuffBaseData *pData)
{
	if (!pData)
	{
		rfalse(4,1,"BuffModify::InitBuffData");
		return false;
	}
	m_ID			 = pData->m_ID;
	m_DeleteType	 = pData->m_DeleteType;
	m_SaveType		 = pData->m_SaveType;
	m_ScriptID		 = pData->m_ScriptID;
	m_Margin		 = pData->m_Margin;
	m_WillActTimes	 = pData->m_WillActionTimes;
	m_GroupID		 = pData->m_GroupID;
	m_Weight		 = pData->m_Weight;
	m_InturptRate	 = pData->m_InturptRate;
	m_Type			 = pData->m_Type;
	m_CanBeReplaced	 = pData->m_CanBeReplaced;
	m_curStep		 = 0;
	m_ActedTimes	 = 0;
	m_CanDropBuff	 = false;

	m_PropertyID	 = pData->m_PropertyID;
	m_ActionType	 = pData->m_ActionType;
	memcpy(&m_BuffAction, &pData->m_Action, sizeof(m_BuffAction));
	m_ReginBuffType = pData->m_ReginType;
	m_ReginBuffShape = pData->m_ReginShape;
	m_ReginBuffHeight = pData->m_ReginHeight;
	m_ReginBuffWidth = pData->m_ReginWidth;


	m_fCenterPosX = m_fCenterPosY = 0.0f;
	CreateReginBuff();


	return true;
}

void BuffModify::OnTimer(int step)
{
	if (step < -2)
		return;

	// 因为正常或者被中断而激活，则Buff将被咔嚓^_^
	bool isEnd = ((step == -1 || step == -2) ? true : false);
	m_CanDropBuff = isEnd;

	for (int i = BATS_ADD; i < BATS_MAX; i++)
	{
		bool isTriggred = false;
		WORD TriSpot	= m_BuffAction[i].m_TriggetSpot;
		int	 TriValue = m_BuffAction[i].m_Value;

		if (BATS_MAX == TriSpot)
			continue;

		switch (step)
		{
		case 0:
			if (BATS_ADD == TriSpot || BATS_INTERVAL == TriSpot)
				isTriggred = true;
			break;

		case -1:
			if (BATS_REMOVE == TriSpot || BATS_INTERVAL == TriSpot)
				isTriggred = true;
			break;

		case -2:
			if (BATS_CANCEL == TriSpot)
				isTriggred = true;
			break;

		default:
			if (BATS_INTERVAL == TriSpot)
				isTriggred = true;
			break;
		}

		if (TriValue == 0) isTriggred = false;

		if (isTriggred)
		{
			int modified = 0;
			if (m_Type > 1)
			{
				OnTimerActived(step);
			}
			else
			{
				ModifyProperty(m_PropertyID, m_ActionType, m_BuffAction[i].m_Value, &modified, isEnd);
			}
		}
	}

	// Buff未移除之前，递增激活次数
	if (-1 != step && -2 != step)
		m_ActedTimes++;

	LuaFunctor(g_Script, FormatString("BuffActived%d", m_ID))[step][m_ActedTimes]();

	// 更新自身属性
	if (m_owner)
		m_owner->UpdateAllProperties();

	return;
}

bool BuffModify::ModifyProperty(WORD ProID, WORD Mode, int val, int *ret, bool isEnd)
{
	if (!m_owner)
	{
		rfalse(4,1,"BuffModify::ModifyProperty");
		return false;
	}
	switch (ProID)
	{
	case BAP_NULL:
		break;

	case BAP_CHP:				// 当前生命
		*ret = m_owner->ModifyCurrentHP(val, Mode, m_pusher);
		break;

	case BAP_CMP:				// 当前内力
		*ret = m_owner->ModifyCurrentMP(val, Mode, false);
		break;

	case BAP_CTP:				// 当前体力
		*ret = m_owner->ModifyCurrentTP(val, Mode, false);
		break;

	case BAP_MHP:				// 生命上限
	case BAP_MMP:				// 内力上限
	case BAP_MTP:				// 体力上限
	case BAP_GONGJI:			// 攻击
	case BAP_FANGYU:			// 防御
	case BAP_BAOJI:				// 暴击
	case BAP_SHANBI:			// 闪避
	case BAP_SPEED:				// 移动速度
	case BAP_ATKSPEED:			// 攻击速度
	case BAP_HIT:			// 命中
	case BAP_SORBDAM:			// 伤害吸收值
	case BAP_SORBDAM_PER:		// 伤害吸收百分比
	case BAP_BACK_DAMAGE:		// 伤害反弹值
	case BAP_BACK_DAMAGE_PER:	// 伤害反弹百分比
	case BAP_EXTRA_EXP_PER:		// 额外经验百分比
	case BAP_UNCRIT: //抗暴
	case BAP_WRECK://破击
	case BAP_UNWRECK:	//抗破
	case BAP_PUNCTURE:	//穿刺
	case BAP_UNPUNCTURE:	//抗穿
		{
			if (BAM_POT == Mode)	
				m_owner->m_BuffModifyPot[ProID-1] += val;		// 以值方式改变
			else					
				m_owner->m_BuffModifyPercent[ProID-1] += val;	// 以百分比方式改变
		}
		break;

	case BAP_SP_SUCKHP:			// 吸血
		{
			
		}
		break;

	case BAP_SP_DEDUCTMP:		// 扣内力OK
		{

		}
		break;

	case BAP_SP_DEDUCTTP:		// 扣体力OK
		{

		}
		break;

 	case BAP_DIZZY:					// 昏迷
 	case BAP_HYPNOTISM:				// 催眠
 		m_owner->m_bPlayerAction[CST_CANMOVE]			=
 		m_owner->m_bPlayerAction[CST_CANJUMP]			=
 		m_owner->m_bPlayerAction[CST_CANUSTITEM]		=
 		m_owner->m_bPlayerAction[CST_CANSTALL]			=
		m_owner->m_bPlayerAction[CST_CANATTACK]			= 
		m_owner->m_bPlayerAction[CST_CANSKILLATK]		= isEnd;
 		break;
 
 	case BAP_LIMIT_SKILL:			// 封招
		m_owner->m_bPlayerAction[CST_CANSTALL]			=
 		m_owner->m_bPlayerAction[CST_CANSKILLATK]		= isEnd;
 		break;
 
	case BAP_DINGSHENG:				//定身
		m_owner->m_bPlayerAction[CST_CANMOVE]			=
		m_owner->m_bPlayerAction[CST_CANJUMP]			= 
		m_owner->m_bPlayerAction[CST_CANSTALL]			= isEnd;
		break;

 	case BAP_WUDI:					// 无敌
		m_owner->m_bPlayerAction[CST_WUDI] = isEnd;
		break;

	case BAP_ADDBUFF:					//  无法附加任何BUFF
		m_owner->m_bPlayerAction[CST_ADDBUFF] = isEnd;
		break;

// 	case BAP_RESIST_CHP:			// 抗流血
// 	case BAP_RESIST_SPEED:			// 抗降速
// 	case BAP_RESIST_DIZZY:			// 抗眩晕
// 	case BAP_RESIST_LIMIT_SKILL:	// 抗封招
// 	case BAP_RESIST_HYPNOTISM:		// 抗催眠
// 	case BAP_RESIST_POISION:		// 抗中毒
// 		m_owner->m_ResistValue[ProID-BAP_RESIST_CHP] += val;
// 		m_owner->m_ResistValue[ProID-BAP_RESIST_CHP] = m_owner->m_ResistValue[ProID-BAP_RESIST_CHP] < 0 ? 0 : m_owner->m_ResistValue[ProID-BAP_RESIST_CHP];
// 		m_owner->m_ResistValue[ProID-BAP_RESIST_CHP] = m_owner->m_ResistValue[ProID-BAP_RESIST_CHP] > 10000 ? 10000 : m_owner->m_ResistValue[ProID-BAP_RESIST_CHP];
// 		break;

	default:
		return false;
	}

	if (ProID >= BAP_DIZZY && ProID < BAP_STATE_MAX)
	{
		if (m_pusher)
		{
			m_owner->ChangeFightState(ProID, !isEnd, m_pusher->GetGID());
		}
		else
		{
			m_owner->ChangeFightState(ProID, !isEnd, 0);
		}
	}
		

	return true;
}

// 获取需要的保存buff数据
void BuffModify::GetSaveBuffData(TSaveBuffData *dat_t) const
{	
	if (NULL == dat_t)
		return ;
	dat_t->m_dwBuffID = m_ID;
	dat_t->m_curStep = m_curStep;
	dat_t->m_ActedTimes = m_ActedTimes;
	dat_t->m_CanDropBuff = m_CanDropBuff;
}
void BuffModify::SetSaveBuffData(const TSaveBuffData *dat_t)
{
	m_ID = dat_t->m_dwBuffID;
	m_curStep = dat_t->m_curStep;
	m_ActedTimes = dat_t->m_ActedTimes;
	m_CanDropBuff = dat_t->m_CanDropBuff;
}

bool BuffModify::CreateReginBuff()
{
	if (!m_pusher)
	{
		return false;
	}
	if (m_Type > 1) //区域buff
	{
		if (m_ReginBuffShape == 0 && m_ReginBuffType == 0) //自身为原点圆形
		{
			m_fCenterPosX = m_pusher->m_curX;
			m_fCenterPosY = m_pusher->m_curY;
			return true;
		}
		else if (m_ReginBuffShape == 0 && m_ReginBuffType == 1) //目标点为原点的圆形
		{
			if (m_pusher)
			{
				m_fCenterPosX = m_pusher->m_curX;
				m_fCenterPosY = m_pusher->m_curY;


				INT32 startX, startY, endX, endY;
				startX = (DWORD)(m_fCenterPosX) >> TILE_BITW;
				startY = (DWORD)(m_fCenterPosY) >> TILE_BITH;

				startX = GetCurArea(startX, _AreaW);
				startY = GetCurArea(startY, _AreaH);

				endX = startX + 3;
				endY = startY + 3;
				// 获取攻击名单
				INT32 monsterNumber = 0;
				std::list<CFightObject *> destList;

				for (int x = startX; x < endX; x++)
				{
					for (int y = startY; y < endY; y++)
					{
						CPlayer *pPlayer = (CPlayer*)m_pusher->DynamicCast(IID_PLAYER);
						if (pPlayer)
						{
							check_list<LPIObject> *playerList = m_pusher->m_ParentRegion->GetMonsterListByAreaIndex(x, y);
							CheckAddList(playerList, &destList);
						}
						else
						{
							check_list<LPIObject> *playerList = m_pusher->m_ParentRegion->GetPlayerListByAreaIndex(x, y);
							CheckAddList(playerList, &destList);
						}
					}
				}

				for (std::list<CFightObject *>::iterator it = destList.begin(); it != destList.end(); ++it)
				{
					CFightObject *fightObject = *it;
					D3DXVECTOR2 curPos(m_pusher->m_curX, m_pusher->m_curY);
					D3DXVECTOR2 desPos(fightObject->m_curX, fightObject->m_curY);

					WORD skillid = m_pusher->GetSkillIDBySkillIndex(m_pusher->m_AtkContext.dwSkillIndex);
					const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(skillid);
					if (pData)
					{
						WORD dis = pData->m_SkillMaxDistance + 100 ;
						float distance = D3DXVec2Length(&(curPos - desPos));
						if (dis > distance) //找到距离最近的目标
						{
							m_fCenterPosX = fightObject->m_curX;
							m_fCenterPosY = fightObject->m_curY;
							return true;
						}
						else
						{
							D3DXVECTOR2 desVector = desPos - curPos;
							D3DXVec2Normalize(&desVector, &desVector);
							float fRadian = atan2(desVector.y, desVector.x);
							m_fCenterPosX = curPos.x + dis * cos(fRadian);
							m_fCenterPosY = curPos.y + dis * sin(fRadian);
							return true;
						}
					}
				}

			}
			else   //目标可能不存在，直接使用自身原点
			{
				m_fCenterPosX = m_pusher->m_curX;
				m_fCenterPosY = m_pusher->m_curY;
				return true;
			}
		}
	}
	return false;
}

void BuffModify::OnTimerActived(int step)
{
	if (step < 0 )
	{
		return;
	}
	if (m_Type > 1) //区域buff
	{
		INT32 startX, startY, endX, endY;

		startX = (DWORD)(m_fCenterPosX) >> TILE_BITW;
		startY = (DWORD)(m_fCenterPosY) >> TILE_BITH;

		startX = GetCurArea(startX, _AreaW);
		startY = GetCurArea(startY, _AreaH);

		endX = startX + 3;
		endY = startY + 3;
		// 获取攻击名单
		INT32 monsterNumber = 0;
		std::list<CFightObject *> destList;

		for (int x = startX; x < endX; x++)
		{
			for (int y = startY; y < endY; y++)
			{
				if (m_pusher)
				{
					CPlayer *pPlayer = (CPlayer*)m_pusher->DynamicCast(IID_PLAYER);
					if (pPlayer)
					{
						check_list<LPIObject> *playerList =m_pusher->m_ParentRegion->GetMonsterListByAreaIndex(x, y);
						CheckAddList(playerList, &destList);
					}
					else
					{
						check_list<LPIObject> *playerList = m_pusher->m_ParentRegion->GetPlayerListByAreaIndex(x, y);
						CheckAddList(playerList, &destList);
					}
				}

			}
		}
		for (std::list<CFightObject *>::iterator it = destList.begin(); it != destList.end(); ++it)
		{
			CFightObject *fightObject = *it;
			if (m_PropertyID == BAP_CHP && fightObject) //修改生命
			{

				fightObject->ModifyCurrentHP(m_BuffAction[BATS_INTERVAL].m_Value, m_ActionType, m_pusher);
				CFightObject::SDamage sdamage;
				sdamage.mHitState = 0;
				sdamage.wDamage = abs(m_BuffAction[BATS_INTERVAL].m_Value);
				m_pusher->SendDamageMessageBySkillAttack_Normal(fightObject, sdamage);
				fightObject->UpdateAllProperties();
			}
		}
	}

}

void BuffModify::CheckAddList(check_list<LPIObject> *objectList, std::list<CFightObject *> *destList)
{
	check_list<LPIObject>::iterator iter;
	if (objectList && !objectList->empty())
	{
		for (iter = objectList->begin(); iter != objectList->end(); ++iter)
		{
			LPIObject object = *iter;

			if (!object || !object->isValid())
				continue;

			CFightObject *fightObject = (CFightObject*)((object)->DynamicCast(IID_FIGHTOBJECT));
			if (!fightObject || fightObject->GetGID() == m_pusher->GetGID() || 0 == fightObject->m_CurHp)
				continue;

			// 检查是否满足群攻方式的攻击要求
			if (IsCanDamageByBuff(fightObject))
			{
				destList->push_back(fightObject);
			}
		}
	}
}

bool BuffModify::IsCanDamageByBuff(CFightObject* desObject)
{
	if (!desObject)
		return false;

	// 目标原点圆形群攻
	if (m_ReginBuffShape == 0 )
	{
		// 如果目标点和中心点距离在攻击半径之内，则条件满足
		D3DXVECTOR2 vlenth((m_fCenterPosX - desObject->m_curX), (m_fCenterPosY - desObject->m_curY));
		float distance = D3DXVec2Length(&vlenth);
		if ((WORD)(distance) <= (m_ReginBuffHeight + 2))
			return true;
	}
	return false;
}
