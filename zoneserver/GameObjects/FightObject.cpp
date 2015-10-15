#include "StdAfx.h"
#include "Fightobject.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "Pet.h"
#include "region.h"
#include "area.h"
#include "Monster.h"
#include "building.h"
#include "player.h"
#include "UpgradeSys.h"
#include "DFightData.h"
#include "Trigger.h"
#include "GoatMutateData.h"
#include "math.h"
#include "ScriptManager.h"
#include "factionmanager.h"
#include "globalfunctions.h"
#include "unionmanage.h"
#include "gameobjects/Item.h"
#include "Mounts.h"
#include "Npc.h"

#include "networkmodule\upgrademsgs.h"
#include "networkmodule\orbmsgs.h"
#include "networkmodule\regionmsgs.h"
#include "networkmodule\ScriptMsgs.h"
#include "networkmodule\movemsgs.h"
#include "networkmodule\RelationMsgs.h"
#include "networkmodule\tongmsgs.h"
#include "TeamManager.h"
#include "networkmodule/TeamMsgs.h"
#include "Networkmodule/ItemMsgs.h"

#include "gameobjects/gameworld.h"
#include "gameobjects/DSkillStudyTable.h"

#include "BuffJump.h"
#include "BuffUpSpeed.h"
#include "BuffAddAgile.h"
#include "BuffReduceCD.h"
#include "BuffReduceDelay.h"
#include "BuffAbsoluteParry.h"

#include "BuffOutDefense.h"
#include "BuffInnerDefense.h"
#include "BuffCounterAttackRate.h"
#include "BuffReduceAbsoluteHurt.h"
#include "BuffReduceHurtRate.h"
#include "BuffUpHPLimit.h"
#include "BuffSys.h"
#include "BuffBaseData.h"

#include "RoleLevelUpTbl.h"
#include "BulletManager_s.h"
#include "SkillDataTable.h"
#include "DTelergyData.h"

#include "GlobalSetting.h"
#include "CSkillDefine.h"
#include "CSkillService.h"
#include "Random.h"
#include "CPlayerDefine.h"
#include "CPlayerService.h"
#include "CBuffDefine.h"
#include "CFightPetService.h"
#include "DynamicRegion.h"
#include "DynamicScene.h"
#include "StackWalker.h"
extern LPIObject GetPlayerByGID(DWORD);
extern BOOL PutPlayerIntoDestRegion(CPlayer *pPlayer, WORD wNewRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID = 0);
extern  int MAXPLAYERLEVEL;
extern LPIObject GetObjectByGID(DWORD GID);
// 攻击状态（阶段）
enum E_FIGHTSTATE 
{
	EFS_BEGIN,          // 开始攻击
	EFS_HIT,            // 命中目标
	EFS_END,            // 结束攻击 

	EFS_MAX
};

#define  SUPPORT_IN_MOUNT_FIGHT // 支持骑乘上战斗

CFightObject::CFightObject(void)
{
	m_dwLastEnemyID		= 0;
	m_IsCrilial			= 0;		// 是否是暴击初始化
	//m_IsExtraDamage		= false;	// 是否是额外伤害（冰火玄毒）
	//////////////////////////////////////////////////////////////////////////
	m_AtkContext.m_bSkillAtkOver	= true;
	m_AtkContext.m_bIsProcessDamage = false;
	m_AtkContext.m_enableNextSkill	= false;
	m_AtkContext.m_consumeHP		= 0;
	m_AtkContext.m_consumeMP		= 0;
	m_AtkContext.mDefenderGID		= 0;
	m_AtkContext.m_Phase			= 0;

	m_IsInFight			= false;
	//	m_EnemyInEye		= 0;
	m_fightState		= 0;
	m_Level				= 0;
	m_IsFightUpdated	= false;
	m_SorbDamageValue	= 0;
	m_SorbDamagePercent	= 0;
	m_ExtraExpPercent	= 0;
	//	m_AbsDamageValue	= 0;
	m_HateValue=0;///仇恨值
	m_pFightFollow = 0; 
	memset(m_FightPropertyStatus, 0, sizeof(m_FightPropertyStatus));
	memset(m_BuffModifyPot,		  0, sizeof(m_BuffModifyPot));
	memset(m_BuffModifyPercent,   0, sizeof(m_BuffModifyPercent));
	//	memset(m_ResistValue,		  0, sizeof(m_ResistValue));


	m_buffMgr.Init(this);
	m_ReduceDamage	= 0;			// 伤害减免
	m_AbsDamage		= 0;			// 绝对伤害
	m_NonFangyu		= 0;			// 无视防御
	m_MultiBaoJi	= 0;			// 暴击倍数

	m_IceDamage		= 0;			// 冰伤害
	m_IceDefence	= 0;			// 冰抗性
	m_FireDamage	= 0;			// 火伤害
	m_FireDefence	= 0;			// 火抗性
	m_XuanDamage	= 0;			// 玄伤害
	m_XuanDefence	= 0;			// 玄抗性
	m_PoisonDamage	= 0;			// 毒伤害
	m_PoisonDefence	= 0;			// 毒抗性
	// 设置内部引用，用于加速属性的更新
	for (size_t i=0; i<XA_MAX_EXP; i++)
	{
		switch (i)
		{
		case XA_GONGJI:			// 攻击
			m_AttriRefence[i] = &m_GongJi;
			break;
		case XA_FANGYU:			// 防御
			m_AttriRefence[i] = &m_FangYu;
			break;
		case XA_BAOJI:			// 暴击
			m_AttriRefence[i] = &m_BaoJi;
			break;
		case XA_SHANBI:			// 躲避
			m_AttriRefence[i] = &m_ShanBi;
			break;
		case XA_MAX_HP:			// 最大生命
			m_AttriRefence[i] = &m_MaxHp;
			break;
		case XA_MAX_MP:			// 最大内力
			m_AttriRefence[i] = &m_MaxMp;
			break;
		case XA_MAX_TP:			// 最大体力
			m_AttriRefence[i] = &m_MaxTp;
			break;
		case XA_CUR_HP:			// 当前生命
			m_AttriRefence[i] = &m_CurHp;
			break;
		case XA_CUR_MP:			// 当前内力
			m_AttriRefence[i] = &m_CurMp;
			break;
		case XA_CUR_TP:			// 当前体力
			m_AttriRefence[i] = &m_CurTp;
			break;
		case XA_LEVEL:			// 等级
			m_AttriRefence[i] = &m_Level;
			break;
		case XA_SPEED:			// 速度
			m_AttriRefence[i] = 0;	// !!!!速度有单独的更新机制
			break;
		case XA_ATKSPEED:		// 攻击速度
			m_AttriRefence[i] = 0;	// !!!!攻击速度有单独的更新机制
			break;

		case XA_REDUCEDAMAGE:	// 伤害减免
			m_AttriRefence[i] = &m_ReduceDamage;
			break;

		case XA_ABSDAMAGE:		// 绝对伤害
			m_AttriRefence[i] = &m_AbsDamage;
			break;

		case XA_NONFANGYU:		// 无视防御
			m_AttriRefence[i] = &m_NonFangyu;
			break;

		case XA_MULTIBAOJI:		// 暴击倍数
			m_AttriRefence[i] = &m_MultiBaoJi;
			break;

// 		case XA_ICE_DAMAGE:		// 冰伤害
// 			m_AttriRefence[i] = &m_IceDamage;
// 			break;
// 
// 		case XA_ICE_DEFENCE:	// 冰抗性
// 			m_AttriRefence[i] = &m_IceDefence;
// 			break;
// 
// 		case XA_FIRE_DAMAGE:	// 火伤害
// 			m_AttriRefence[i] = &m_FireDamage;
// 			break;
// 
// 		case XA_FIRE_DEFENCE:	// 火抗性
// 			m_AttriRefence[i] = &m_FireDefence;
// 			break;
// 
// 		case XA_XUAN_DAMAGE:	// 玄伤害
// 			m_AttriRefence[i] = &m_XuanDamage;
// 			break;
// 
// 		case XA_XUAN_DEFENCE:	// 玄抗性
// 			m_AttriRefence[i] = &m_XuanDefence;
// 			break;
// 
// 		case XA_POISON_DAMAGE:	// 毒伤害
// 			m_AttriRefence[i] = &m_PoisonDamage;
// 			break;
// 
// 		case XA_POISON_DEFENCE:	// 毒抗性
// 			m_AttriRefence[i] = &m_PoisonDefence;
// 			break;

		default:
			break;
			//MY_ASSERT(0);
		}
	}
}

int CFightObject::OnCreate(_W64 long pParameter)
{
	m_AtkSpeed = PLAYER_BASIC_ATKSPEED;

	return CScriptObject::OnCreate(pParameter);
}

CFightObject::~CFightObject(void)
{
	RemoveAllListener();	// 清除监听队列

	// 强制子弹无效
	CheckStopBullet(1);
	CheckStopBullet(2);

	// 清理CD管理器
	m_CDMgr.Free();

	// 清理Buff管理器
	m_buffMgr.Free();

	// 释放的Buff，释放者要置空
	m_buffMgr.PusherNotAvaliable();
}

void CFightObject::OnRun()
{
	// 战斗状态进入/脱离判断，这里只需要判断是否需要离开战斗状态
	// 进入战斗状态在攻击/被攻击时会自动设置，所以不需要在OnRun中作判断
	if (m_IsInFight)
	{
		// 衰减战斗状态
		DWORD currTick = timeGetTime();
		for (std::map<DWORD, DWORD>::iterator it = m_Attacker.begin(); it != m_Attacker.end();)
		{
			if (currTick - it->second >= 5000)
				it = m_Attacker.erase(it);
			else
				++it;
		}

		// 如果当前攻击列表为空
		if (m_Attacker.empty())
			SwitchFightState(m_IsInFight = false);
	}

	RemoveFromHatelistOnTime();///根据时间移除仇恨

	CActiveObject::OnRun();

	if (g_InDebugMode)
	{
		if (EA_DEAD == GetCurActionID())
		{
			if (m_CurHp >0 )
			{
				CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
				if (pPlayer)
				{
					rfalse(2, 1, "Player CurHP > 0");
				}
				else
				{
					rfalse(2, 1, "CFightObject CurHP > 0");
				}
				m_CurHp = 0;
			}
		}

		if (0 == m_CurHp)
		{
			if (EA_DEAD != GetCurActionID())
			{
				rfalse(2, 1, "EA_DEAD != GetCurActionID()");
				SetCurActionID(EA_DEAD);
			}
		}

// 		if (EA_DEAD == GetCurActionID())
// 			MY_ASSERT(0 == m_CurHp);
// 
// 		if (0 == m_CurHp)
// 			MY_ASSERT(EA_DEAD == GetCurActionID());
	}
}

BOOL CFightObject::EndPrevAction()
{
	CActiveObject::EndPrevAction();

	switch (GetCurActionID())
	{
	case EA_DEAD:
		break;

	default:
		break;
	}

	return TRUE;
}

BOOL CFightObject::SetCurAction()
{
	// 填充后备动作的数据
	switch (GetCurActionID())
	{
	case EA_SKILL_ATTACK:
		break;

	case EA_DEAD:
		break;

	default:
		return CActiveObject::SetCurAction();
	}

	return TRUE;
}

BOOL CFightObject::DoCurAction()
{
	switch (GetCurActionID())
	{
	case EA_SKILL_ATTACK:
		UpdateSkillAttack();
		break;

	case EA_DEAD:
		if (m_CurHp)
		{
			rfalse(4, 1, "Fightobject.cpp - SetCurAction() - m_CurHp");
			return FALSE;
		}
		MY_ASSERT(0 == m_CurHp);

		break;

	default:
		return CActiveObject::DoCurAction();
	}

	return TRUE;
}

void CFightObject::OnDead(CFightObject *PKiller)
{
	RemoveAllListener();	// 清除监听队列

	// 清空攻击者列表（死亡一定脱离，所以直接置为false）
	m_Attacker.clear();
	m_IsInFight = false;

	//ClearFollow();//清除所有追击者
	m_pFightFollow = 0;//我追击的玩家也重置


	CPlayer *player=(CPlayer*)PKiller->DynamicCast(IID_PLAYER);
	CFightPet *pPet = NULL;
	if (player)
	{
		player->RemoveHatelistFromID(m_GID);
	}

	// 清空死亡消失的Buff
	m_buffMgr.CheckDelBuffSpot(BDT_DEAD);

	SAPlayerDeadMsg DeadMsg;
	DeadMsg.dwGID = GetGID();
	DeadMsg.killGID = PKiller ? PKiller->GetGID() : 0;
	if (m_ParentArea)
		m_ParentArea->SendAdj(&DeadMsg, sizeof(DeadMsg), -1);

	SetCurActionID(EA_DEAD);
	SetBackupActionID(EA_DEAD);

	if(!player)
	{
		pPet = (CFightPet *)PKiller->DynamicCast(IID_FIGHT_PET);
		if (pPet)player = pPet->m_owner;
	}

	if(player)
	{
		CPlayer *mplayer = GetFightObjToPlayer(this);
		if(!mplayer)return;					// 不是玩家不计算杀孽值
		if(mplayer->m_wPKValue) return;		// 红名玩家同样不计算杀孽值
		player->m_wPKValue += PLAYER_PKVALUERISE;
		player->SendPlayerPkValue();
		if(player->m_dChangeColorTime == 0xffffffff)player->m_dChangeColorTime = timeGetTime();
		BYTE NameColor = player->GetPlayerNameColor();
		if(player->m_bNameColor != NameColor)
		{
			player->m_bNameColor = NameColor;
			player->SendPlayerNameColor();
		}
	}




}

void CFightObject::Kill(CFightObject *pEnemy)
{
	// 执行被攻击者的OnDead函数
	pEnemy->OnDead(this);

	// 清除被攻击者的相关的变量
	pEnemy->m_dwLastEnemyID = 0;

	// 清除攻击者的相关的变量
	m_dwLastEnemyID = 0;
}

/** 计算经验衰减
【经验衰减规则】

怪物类型为0，1，4的怪物适用以下规则：

人物等级高于怪物等级 10级时，经验衰减20%
20级时，经验衰减50%
50级时，经验衰减90%

人物等级低于怪物等级50级时，经验衰减90%


怪物类型是2和3的怪物适用以下规则：

人物等级高于怪物等级时，无论超过多少级，经验均不衰减
人物等级低于怪物等级50级时，经验衰减90%

怪物类型是2和3的怪物适用以下规则：

人物等级高于怪物等级时，无论超过多少级，经验均不衰减
人物等级低于怪物等级50级时，经验衰减90%
*/

DWORD GetExpAddition( CPlayer *PKiller, int monsterLevel, DWORD exp, DWORD bossFlag )
{
	// 	if ( PKiller && ( bossFlag == 0 || bossFlag == 1 || bossFlag == 4 ) )
	// 	{
	// 		int levelDValue = PKiller->m_Property.m_Level - monsterLevel;
	// 		if( levelDValue > 50 ) return exp / 10;
	// 		else if( levelDValue > 20 ) return exp / 2;
	// 		else if( levelDValue > 10 ) return exp * 4 / 5;
	// 		else if( levelDValue < -50 ) return exp / 10;
	// 	}
	// 
	// 	if ( PKiller && ( bossFlag == 2 || bossFlag == 3 ) )
	// 	{
	// 		int levelDValue = PKiller->m_Property.m_Level - monsterLevel;
	// 		if( levelDValue < -50 ) return exp / 10;
	// 	}
	// 
	// 	return exp;
	return 0;
}

void CFightObject::SendAddPlayerExp(DWORD dwExpVal, BYTE byChangeType, LPCSTR how)
{
	if (/*0 == m_CurHp || */0 == dwExpVal)
		return;

	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;

	// 如果满级，则不加经验了
	if (MAXPLAYERLEVEL == pPlayer->m_Property.m_Level)
		return;

	QWORD remainExp = 0xffffffffffffffff - pPlayer->m_Property.m_Exp;

	pPlayer->m_Property.m_Exp += (dwExpVal > remainExp ? remainExp : dwExpVal);

	// 发送获取经验消息
	SAExpChangeMsg ExpMsg;
	ExpMsg.dwGlobalID       = pPlayer->GetGID();
	ExpMsg.qwCurGain		= (dwExpVal > remainExp ? remainExp : dwExpVal);
	ExpMsg.byExpChangeType  = byChangeType;
	g_StoreMessage(pPlayer->m_ClientIndex, &ExpMsg, sizeof(SAExpChangeMsg));

	// 在此判断是否可以升级
	pPlayer->OnRecvLevelUp(0);
	pPlayer->m_PlayerPropertyStatus[XA_CUR_EXP-XA_MAX_EXP] = true;

	return;
}

void CFightObject::SendPropertiesUpdate()
{
	if (!m_ParentArea)
		return;

	// 更新速度
	if (m_FightPropertyStatus[XA_SPEED])
	{
		SASpeedChanged msg;
		msg.dwGlobal	= GetGID();
		msg.fSpeed		= m_Speed;
		m_ParentArea->SendAdj(&msg, sizeof(SASpeedChanged), -1);
	}

	// 更新攻击速度
	if (m_FightPropertyStatus[XA_ATKSPEED])
	{
		SAAtkSpeedChanged msg;
		msg.dwGlobal	= GetGID();
		msg.wAtkSpeed	= m_AtkSpeed;
		m_ParentArea->SendAdj(&msg, sizeof(SAAtkSpeedChanged), -1);
	}

	// 在这里发送Listener通知（选中自己的），如果死亡，则会通知取消，那么也就不需要传递了
	if (m_CurHp && (m_FightPropertyStatus[XA_MAX_HP] || m_FightPropertyStatus[XA_MAX_MP] || 
		m_FightPropertyStatus[XA_CUR_HP] || m_FightPropertyStatus[XA_CUR_MP] ||
		m_FightPropertyStatus[XA_LEVEL]))
	{
		NotifyExchange();
	}
}

void CFightObject::SendEffectiveMsg(SASetEffectMsg &msg)
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;

	if (pPlayer->m_ParentArea)
		pPlayer->m_ParentArea->SendAdj(&msg, sizeof(msg), -1);

	return;
}

// 增加等级并发送
void CFightObject::SendAddPlayerLevel(WORD byLevelVal, BYTE byEffectIndex, LPCSTR info)
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;

	if (pPlayer->m_Property.m_Level + byLevelVal > MAXPLAYERLEVEL)
		return;

	const SBaseAttribute *baseData = 0;

	// 计算要给玩家多少点自由点
// 	WORD allFreePoint = 0;
	for (size_t i = 1; i <= byLevelVal; i++)
	{
		baseData = CPlayerService::GetInstance().GetPlayerUpdateData(pPlayer->m_Property.m_School, pPlayer->m_Property.m_Level + i);
		if (!baseData)
			return;

		//allFreePoint += baseData->m_FreePoint;
	}

	// 给玩家加点
//	pPlayer->AddPlayerPoint(XA_REMAINPOINT, allFreePoint);

	pPlayer->m_PlayerAttri = baseData;

	pPlayer->UpdateBaseProperties();

	// 获得当前最大经验/精力上限/真气上限
	pPlayer->m_MaxExp = baseData->m_Exp;
	pPlayer->m_MaxJp  = baseData->m_BaseJp;

	pPlayer->m_PlayerPropertyStatus[XA_MAX_EXP-XA_MAX_EXP] =
		pPlayer->m_PlayerPropertyStatus[XA_MAX_JP -XA_MAX_EXP] =
		pPlayer->m_PlayerPropertyStatus[XA_MAX_SP -XA_MAX_EXP] = true;

	// 修正恢复时间和恢复量
	//pPlayer->m_HpRSInterval		= pPlayer->m_PlayerAttri->m_HpRecoverInterval;
	//pPlayer->m_MpRSInterval		= pPlayer->m_PlayerAttri->m_MpRecoverInterval;
	pPlayer->m_TpRSInterval		= pPlayer->m_PlayerAttri->m_TpRecoverInterval;
	pPlayer->m_Hit = pPlayer->m_PlayerAttri->m_Hit;
// 	pPlayer->m_HpRecoverSpeed	= pPlayer->m_PlayerAttri->m_HpRecoverSpeed;
// 	pPlayer->m_MpRecoverSpeed	= pPlayer->m_PlayerAttri->m_MpRecoverSpeed;
//	pPlayer->m_TpRecoverSpeed	= pPlayer->m_PlayerAttri->m_TpRecoverSpeed;

	pPlayer->m_Property.m_Level += byLevelVal;
	m_Level = pPlayer->m_Property.m_Level;
	pPlayer->m_FightPropertyStatus[XA_LEVEL] = true;

	pPlayer->m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT] = pPlayer->m_PlayerAttri->m_uncrit;
	pPlayer->m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT] = pPlayer->m_PlayerAttri->m_wreck;
	pPlayer->m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT] = pPlayer->m_PlayerAttri->m_unwreck;
	pPlayer->m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT] = pPlayer->m_PlayerAttri->m_puncture;
	pPlayer->m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT] = pPlayer->m_PlayerAttri->m_unpuncture;
	

	////update by ly 2014/7/9 当玩家处于变身状态下，不更新玩家的基本战斗属性
	if (pPlayer->m_Property.m_FightPetActived == 0xff)
	{
		UpdateAllProperties();

		RestoreFullHPDirectly();
		RestoreFullMPDirectly();
	}
	//RestoreFullTPDirectly();  更改升级以后不恢复体力

	// 得秀点什么吧~
	SASetEffectMsg msg;
	msg.effect			 = SASetEffectMsg::EEFF_LEVELUP;
	msg.data.LevelUp.gid = GetGID();

	SendEffectiveMsg(msg);
}

// 设置附加状态(点穴、眩晕)
void CFightObject::SendFightExtraState(DWORD iExtraState)
{
	SASetExtraStateyMsg msg;
	msg.dwFightExtraState = iExtraState;
	msg.dwGlobalID = GetGID();

	if (m_ParentArea)
		m_ParentArea->SendAdj(&msg, sizeof(msg), INVALID_DNID);
}

BOOL CFightObject::SetAttackMsg(SFightBaseMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "FightObject.cpp - SetAttackMsg() - !pMsg");
		return FALSE;
	}

	// 通过了等待间隔
	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	CFightPet *FightPet = (CFightPet*)DynamicCast(IID_FIGHT_PET);
	if (!pPlayer&&!FightPet)
	{
		rfalse(4, 1, "FightOjbect.cpp - SetAttackMsg() - !pPlayer");
		return FALSE;
	}

	switch( pMsg->_protocol )
	{
	case SFightBaseMsg::EPRO_FIGHT_OBJECT:
		break;

	case SFightBaseMsg::EPRO_FIGHT_POSITION:
		break;

	case SFightBaseMsg::EPRO_SET_CURTELERGY:
		break;

		// 处理客户端技能请求的消息
	case SFightBaseMsg::EPRO_QUEST_SKILL:
		ProcessQuestSkill((SQuestSkill_C2S_MsgBody*)pMsg);
		break;

	case SFightBaseMsg::EPRO_QUEST_MULTIPLESKILL:
		ProcessQuestSkill((SQuestSkillMultiple_C2S_MsgBody*)pMsg);
		break;
		 
	case SFightBaseMsg::EPRO_QUEST_SKILL_FP:
		break;

	case SFightBaseMsg::EPRO_UPDATE_SKILL_BOX:
		break;

	case SFightBaseMsg::EPRO_PRACTICE_UPDATEBOX:
		break;

	case SFightBaseMsg::EPRO_FORCE_QUEST_ENEMY_INFO:
		{
			if (GetIID() == IID_PLAYER)
			{
				CPlayer* pPlayer = (CPlayer*)this;
				if (!pPlayer)
				{
					rfalse(4, 1, "FightObject.cpp - SetAttackMsg() - !pPlayer - 2");
					return FALSE;
				}

				pPlayer->ProcessForceQusetEnemyInfo();
			}
		}
		break;

	case SFightBaseMsg::EPRO_SELECT_TARGET:
		{
			if (pPlayer)
			{
				SQSelectTarget *pQMsg = (SQSelectTarget*)pMsg;
				if (!pQMsg)
				{
					rfalse(4, 1, "FightOjbect.cpp - SetAttackMsg() - !pQMsg");
					return FALSE;
				}

				if (!m_ParentRegion)
				{
					StackWalker sw;
					sw.ShowCallstack();
					break;
				}


				LPIObject pObject = m_ParentRegion->SearchObjectListInAreas(pQMsg->dwTargetGID, m_ParentArea->m_X, m_ParentArea->m_Y);

				if (pObject)
				{
					pPlayer->StoreTarget((CFightObject*)pObject->DynamicCast(IID_FIGHTOBJECT));

					// 当点击对象是NPC的时候
					CNpc* pNpc = (CNpc*)pObject->DynamicCast(IID_NPC);
					if (pNpc)
					{
						SASelectTarget msg;
						msg.dwTargetGID = pQMsg->dwTargetGID;
						msg.dwMaxHp		= 100;
						msg.dwCurHp		= 100;
						msg.dwMaxMp		= 100;
						msg.dwCurMp		= 100;
						msg.wLevel		= 1;

						g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
					}
				}
			}
		}
		break;

	case SFightBaseMsg::EPRO_UPDATEBUFF_INFO:
		break;

	default:
		return  FALSE;
		break;
	}

	return TRUE;
}

int CFightObject::CheckColdTimeStep0(const SSkillBaseData *pData)
{
	if (!pData)
	{
		rfalse(4, 1, "FightOjbect.cpp - CheckColdTimesSet0() - !pData");
		return 0;
	}

	// 如果是门派的普通攻击，那么不检查公共冷却
	// 	if (pData->m_IsNormalSkill)
	// 		return 0;
	// 	 if (m_CDMgr.Check(1))
	// 	 {
	// 		 rfalse(2,1,"CheckColdTimeStep0 %d",1);
	// 		return 1;  
	// 	 }
	// 	else if (pData->m_CDGroup && m_CDMgr.Check(pData->m_CDGroup))
	// 	{
	// 		 rfalse(2,1,"CheckColdTimeStep0 %d",2);
	// 		return 2;
	// 	 }
	if (m_CDMgr.Check(10000+pData->m_ID))
	{
		// rfalse(2,1,"CheckColdTimeStep0 %d",3);
		return 3;
	}
	return 0;
}

bool CFightObject::SendMove2TargetForAttackMsg(INT32 skillDistance,INT32 skillIndex,CFightObject* pLife)
{
	if (!pLife)
	{
		rfalse(4, 1, "FightOjbect.cpp - SendMove2TargetForAttackMsg() - !pLife");
		return false;
	}

	return true;
}

float GetDirection(float x1, float y1, float x2, float y2)
{
	D3DXVECTOR2 curPos(x1, y1);
	D3DXVECTOR2 desPos(x2, y2);
	D3DXVECTOR2 desVector = desPos - curPos;

	D3DXVec2Normalize(&desVector, &desVector);

	// 计算方向
	return atan2(desVector.y, desVector.x); 
}

float GetDirection(const D3DXVECTOR2 &curPos, const D3DXVECTOR2 &desPos)
{
	D3DXVECTOR2 desVector = desPos - curPos;
	D3DXVec2Normalize(&desVector, &desVector);

	// 计算方向
//	return atan2(desVector.x, desVector.y);
	return atan2(desVector.y, desVector.x);
}

float GetLenth(const float X1, const float Y1, const float X2, const float Y2)
{
	D3DXVECTOR2 v1(X1, Y1);
	D3DXVECTOR2 v2(X2, Y2);
	return D3DXVec2Length(&(v1 - v2));
}


//----------------------------------------------------------------------------------------------------------	
INT32 GetDirection(int x1,int y1,int x2,int y2)
{
	if(x1 == x2 && y1 == y2)
	{
		return 0;
	}

	if(x1 == x2 || y1 == y2)
	{
		if(x1 == x2)
		{
			if(y2 > y1)
			{
				return 4;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			if(x2 > x1)
			{
				return 2;
			}
			else
			{
				return 6;
			}
		}
	}

	if(x2 > x1)
	{
		if(y2 > y1)
		{
			return 3;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		if(y2 > y1)
		{
			return 5;
		}
		else
		{
			return 7;
		}
	}
}

SCC_RESULT CFightObject::CheckConsume(INT32 skillIndex, int& consumeHP, int& consumeMP)
{
	consumeHP = 0;
	consumeMP = 0;

	return SCCR_OK;
}

bool CFightObject::IsCanProcessSkillAttackMsg(const SSkillBaseData *pData, SQuestSkill_C2S_MsgBody *questMsg, bool skipState,bool checkcold)
{
	if (!pData || !questMsg)
	{
		rfalse(4, 1, "FightObject.cpp - IsCanProcessSkillAttackMsg() - !pData || !questMsg");
		return false;
	}

	// 检查冷却
	if (checkcold)
	{
		int cdRet = CheckColdTimeStep0(pData);
		if (cdRet)
		{
			return false;
		}
	}


	// 检查消耗
	int consumeHP = 0;
	int consumeMP = 0;
	int consumeTP = 0;

 	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
 	if (pPlayer && SPT_COMMAN != pData->m_SkillPropertyType && !pData->m_IsNormalSkill)
 	{
 		INT32 skillLevel = -1;
 		INT32 skillIndex = -1;
 
 		for (int i = 0; i < MAX_SKILLCOUNT; ++i)
 		{
 			if (pData->m_ID == pPlayer->m_Property.m_pSkills[i].wTypeID)
 			{
 				skillLevel = pPlayer->m_Property.m_pSkills[i].byLevel;
 				skillIndex = i;
 				break;
 			}
 		}
 
 		if (-1 == skillLevel)
 		{
 			return false;
 		}
 
		if (CheckConsume(skillIndex, consumeHP, consumeMP))
 		{
 			SetCurActionID(EA_STAND);
 			SetBackupActionID(EA_STAND);
 
 			SASynPosMsg	msg;
 			msg.m_GID		= GetGID();
 			msg.m_Action	= EA_STAND;
 			msg.m_X			= m_curX;
 			msg.m_Y			= m_curY;
 			msg.m_Z			= m_curZ;
 
 			g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SASynPositionMsg));
 
 			return false;
 		}
 	}


 	CFightPet *pfightpet = (CFightPet *)DynamicCast(IID_FIGHT_PET);
 	if (pfightpet && SPT_COMMAN != pData->m_SkillPropertyType)
 	{
 		INT32 skillLevel = -1;
 		INT32 skillIndex = -1;

		SNewPetData *pext = &pfightpet->m_owner->m_Property.m_NewPetData[pfightpet->m_index];
		if (pext->m_PetID == 0) return false;
 
 		for (int i = 0; i < PETSKILLMAXNUM; ++i)
 		{
			if (pData->m_ID == pext->m_PetSkill[i].m_PetSkillID)
 			{
				skillLevel = pext->m_PetSkill[i].m_Level;
 				skillIndex = i;
 				break;
 			}
 		}
 
 		if (-1 == skillLevel)
 		{
 			return false;
 		}
 
		if (CheckConsume(skillIndex, consumeHP, consumeMP))
 		{
 			SetCurActionID(EA_STAND);
 			SetBackupActionID(EA_STAND);
 
 			SASynPosMsg	msg;
 			msg.m_GID		= pfightpet->GetGID();
 			msg.m_Action	= EA_STAND;
 			msg.m_X			= m_curX;
 			msg.m_Y			= m_curY;
 			msg.m_Z			= m_curZ;
 
 			g_StoreMessage(pfightpet->m_owner->m_ClientIndex, &msg, sizeof(SASynPositionMsg));
 
 			return false;
 		}
 	}

	// 如果当前有技能在释放，则后续的释放被缓冲
	/*	if (EA_SKILL_ATTACK == GetCurActionID() && (!skipState))*/
	if (EA_SKILL_ATTACK == GetCurActionID())
	{	
		//if (SKS_OVER != m_AtkContext.m_Phase)
		if (!m_AtkContext.m_bSkillAtkOver)     // 如果当前有技能在释放，则后续的释放被缓冲
		{
			AddQuestToCache(questMsg);
			//m_vcacheQuestSkill.push_back(*questMsg);
			m_nextQuestSkillMsg = *questMsg;
			m_AtkContext.m_enableNextSkill = true;
			//rfalse(2,1,"缓冲技能 index = %d",m_nextQuestSkillMsg.dwSkillIndex);
			return false;
		}
	}


	// 当程序走到了这里，那么这个技能请求已经完全满足了，那么初始化技能攻击结构体
	SQuestSkill_C2S_MsgBody *p = &m_AtkContext;
	*p = *questMsg;

	m_AtkContext.m_consumeHP = consumeHP;
	m_AtkContext.m_consumeMP = consumeMP;
	m_AtkContext.m_consumeTP = consumeTP;

	// 对于以下类型的技能，无需验证距离
	if (SAST_EMPTY_CIRCLE == pData->m_AtkType || SAST_EMPTY_SECTOR == pData->m_AtkType || 
		SAST_EMPTY_POINTS == pData->m_AtkType || SAST_SELF == pData->m_AtkType)
		return true;



	CFightObject *pLife = 0;

	// 对于单攻消息，需要验证目标的存在，且如果距离不够，迫使客户端移动
	if (SAST_SINGLE == pData->m_AtkType)
	{
		LPIObject object = m_ParentRegion->SearchObjectListInAreas(questMsg->mDefenderGID, m_ParentArea->m_X,m_ParentArea->m_Y);
		pLife = (CFightObject*)object->DynamicCast(IID_FIGHTOBJECT);
		if (!pLife || 0 == pLife->m_CurHp)
		{
			rfalse(2, 1, "SearchObjectListInAreas NUll %d",questMsg->mDefenderGID);
		}
			//return false;

		CPlayer *player = (CPlayer*)DynamicCast(IID_PLAYER);
		CFightPet *pFightPet = (CFightPet*)DynamicCast(IID_FIGHT_PET);
		if (pLife && (!player|| !pFightPet))
		{

			WORD distance  = sqrt(pow((m_curX-pLife->m_curX),2)+pow((m_curY-pLife->m_curY),2));
			lite::Variant ret;
			LuaFunctor(g_Script, "GetSkillDistance")(&ret);
			int Threshold = ret;
			//WORD Threshold = (EA_RUN == pLife->GetCurActionID()) ? 100 : 40;
			//int Threshold = g_Script.GetGlobalValue("SkillThreshold");
			//int Threshold = 100;
			if (distance > (pData->m_SkillMaxDistance + 100))
			{
				//rfalse("SendMove2TargetForAttackMsg distance = %d", distance);
				SetCurActionID(EA_STAND);
				m_dwLastEnemyID = pLife->GetGID();
				//pLife->AddToFollow(this);//把技能释放者加入到追击列表中
				m_pFightFollow = pLife;
				SendMove2TargetForAttackMsg(pData->m_SkillMaxDistance, m_AtkContext.dwSkillIndex, pLife);
				return false;
			}
		}

	}
	


	// 对于远程群攻，需要验证目的点是否太远，如果太远，则释放失败
	if (SAST_POINT_CIRCLE == pData->m_AtkType || SAST_POINT_SINGLE == pData->m_AtkType)
	{
		float distance = sqrt(pow((m_curX-questMsg->mDefenderWorldPosX),2)+pow((m_curY-questMsg->mDefenderWorldPosY),2));
		//int Threshold = g_Script.GetGlobalValue("SkillThresholdFast");
		if ((WORD)distance > pData->m_SkillMaxDistance + 100)
		{
			SetBackupActionID(EA_STAND);
			if (pPlayer)
			{
				TalkToDnid(pPlayer->m_ClientIndex, "距离过远，技能释放失败！");
			}
			return false;
		}
	}

	// 调整方向
	if (pLife)
	{
		if (SAST_SINGLE == pData->m_AtkType)
			m_Direction = GetDirection(m_curX, m_curY, pLife->m_curX, pLife->m_curY);
		else if (SAST_POINT_CIRCLE == pData->m_AtkType || SAST_POINT_SINGLE == pData->m_AtkType)
			m_Direction = GetDirection(m_curX, m_curY, questMsg->mDefenderWorldPosX, questMsg->mDefenderWorldPosY);
	}
	return true;
}

void CFightObject::StartGlobalCDTimer(const SSkillBaseData* pSkillData)
{
	if (!pSkillData)
	{
		rfalse(4, 1, "FightObject.cpp - StartGlobalCDTimer() - !pSkillData");
		return;
	}

	if (pSkillData->m_CDTime && m_CDMgr.Active(10000 + pSkillData->m_ID, pSkillData->m_CDTime))
	{
		CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
		if (player)
		{
			SNotifyStartColdTimer_S2C_MsgBody msg;
			msg.CDType = 10000 + pSkillData->m_ID;
			msg.coldeTime = pSkillData->m_CDTime;
			g_StoreMessage(player->m_ClientIndex, &msg, sizeof(msg));
		}
	}
	// 	if (pSkillData->m_IsNormalSkill)
	// 		return;

	// 启动公共冷却时间
	// 	if (m_CDMgr.Active(1, 600))
	// 	{
	// 		SNotifyStartColdTimer_S2C_MsgBody msg;
	// 		msg.CDType		= 1;
	// 		msg.coldeTime	= 600;
	// 		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(msg));
	// 	}
	// 
	// 	// 启动技能组冷却时间
	// 	DWORD groupTime = CSkillService::GetInstance().GetGroupCDTime(pSkillData->m_CDGroup);
	// 	if (groupTime && m_CDMgr.Active(pSkillData->m_CDGroup, groupTime))
	// 	{
	// 		SNotifyStartColdTimer_S2C_MsgBody msg;
	// 		msg.CDType		= pSkillData->m_CDGroup;
	// 		msg.coldeTime	= groupTime;
	// 		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(msg));
	// 	}

	// 启动技能单独冷却时间
// 	if (pSkillData->m_CDTime && m_CDMgr.Active(10000 + pSkillData->m_ID, pSkillData->m_CDTime))
// 	{
// 		SNotifyStartColdTimer_S2C_MsgBody msg;
// 		msg.CDType		= 10000 + pSkillData->m_ID;
// 		msg.coldeTime	= pSkillData->m_CDTime;
// 		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(msg));
// 	}

	return;
}

BOOL CFightObject::__ProcessQuestSkill(SQuestSkill_C2S_MsgBody *questMsg, bool skipState, SkillModifyDest *skillModifyDest,bool checkcold)
{
	// skillModifyDest 不用判断，后面都对此的判断，会发送一个失败的消息给Client
	if (!questMsg/* || skillModifyDest*/)
	{
		rfalse(4, 1, "Fightobject.cpp - __ProcessQuestSkill() - !questMsg");
		return FALSE;
	}

	if (!m_CurHp)
	{
		rfalse(4, 1, "Fightobject.cpp - __ProcessQuestSkill() - !m_curhp");
		return FALSE;
	}
	MY_ASSERT(m_CurHp);

	//获得相关数据
	///////////////////////////////////////////////////////////////////////////
	INT32 skillID = GetSkillIDBySkillIndex(questMsg->dwSkillIndex);
	if (-1 == skillID)
		return FALSE;

	const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(skillID);
	if (!pData || SKT_PASSITIVE == pData->m_SkillType)
		return FALSE;

	const SSkillHitSpot *pHitPot = CSkillService::GetInstance().GetSkillHitSpot(skillID);
	if (!pHitPot)
		return FALSE;
	///////////////////////////////////////////////////////////////////////////

	//验证
	///////////////////////////////////////////////////////////////////////////
	if (!m_bPlayerAction[CST_CANATTACK])	//不能攻击
	{
		//rfalse("%d, 你娃遭洗白，不能攻击了！", GetCurrentTime());
		return FALSE;
	}

	if (!pData->m_IsNormalSkill && !m_bPlayerAction[CST_CANSKILLATK])	//封招状态不封普攻
	{
		//rfalse("%d, 你娃遭封招了！", GetCurrentTime());
		return FALSE;
	}

	if (m_fightState & FS_DIZZY)
	{
		//rfalse(2, 1, "已经被昏迷无法攻击");
		return FALSE;
	}

	if (m_fightState & FS_JITUI)
	{
		//rfalse(2, 1, "正在被击退中无法攻击");
		return FALSE;
	}


	// 对于单攻消息，需要验证目标的存在
	if (SAST_SINGLE == pData->m_AtkType)
	{
		LPIObject pObject = m_ParentRegion->SearchObjectListInAreas(questMsg->mDefenderGID, m_ParentArea->m_X, m_ParentArea->m_Y);
		if (!pObject)
		{
			AtkFinished();
			rfalse("target null %d",questMsg->mDefenderGID);
			//return FALSE;
		}

		if (pObject)
		{
			CFightObject *pLife = (CFightObject *)pObject->DynamicCast(IID_FIGHTOBJECT);
			if (!pLife || 0 == pLife->m_CurHp)
			{
				AtkFinished();
				rfalse("%s", !pLife ? "target null " : "target HP == 0");
				return FALSE;
			}
		}
	}

	if (!IsCanProcessSkillAttackMsg(pData, questMsg, skipState,checkcold))
		return FALSE;

	//处理特殊连续技能，如0，1，2
	if (GetcontinueSkill(questMsg))
	{
		if (verifySkillMsg(&m_LuaQuestSkillMsg))  //因为技能已经改变，所以重新验证技能的有效性
		{
			SQuestSkill_C2S_MsgBody *p = &m_AtkContext;
			*p = m_LuaQuestSkillMsg;

			// 			m_AtkContext.m_consumeHP = consumeHP;
			// 			m_AtkContext.m_consumeMP = consumeMP;
			// 			m_AtkContext.m_consumeTP = consumeTP;

			skillID = GetSkillIDBySkillIndex(m_LuaQuestSkillMsg.dwSkillIndex);
			if (-1 == skillID)
				return FALSE;

			pData = CSkillService::GetInstance().GetSkillBaseData(skillID);
			if (!pData || SKT_PASSITIVE == pData->m_SkillType)
				return FALSE;

			pHitPot = CSkillService::GetInstance().GetSkillHitSpot(skillID);
			if (!pHitPot)
				return FALSE;

			//rfalse("切换技能 skillID = %d,startX = %f,startY = %f",skillID,m_LuaQuestSkillMsg.mDefenderWorldPosX,m_LuaQuestSkillMsg.mDefenderWorldPosY);
		}
	}

	//开始攻击
	///////////////////////////////////////////////////////////////////////////
	// 切换技能攻击状态
	SetCurActionID(EA_SKILL_ATTACK);
	SetBackupActionID(EA_SKILL_ATTACK);

	// 保存当前技能属性，便于快速引用
	m_AtkContext.m_SkillData = *pData;
	if(skillModifyDest==NULL)
		m_AtkContext.m_SkillModifyDest.m_bModifyDest = false;
	else
		m_AtkContext.m_SkillModifyDest = *skillModifyDest;

// 	if (SPT_CHARGE == m_AtkContext.m_SkillData.m_SkillPropertyType)
// 	{
// 		rfalse(2,1," quest SSwitch2SkillAttackState_S2C_MsgBody x = %f,y = %f,m_CurSkillID = %d",m_AtkContext.mDefenderWorldPosX,m_AtkContext.mDefenderWorldPosY,m_AtkContext.m_SkillData.m_ID);
// 	}

	SkillAttackStart(pHitPot);
	///////////////////////////////////////////////////////////////////////////

	return TRUE;
}

// 处理请求对某对象使用技能的消息
BOOL CFightObject::ProcessQuestSkill(SQuestSkill_C2S_MsgBody *pMsg)
{
	if (!pMsg || GetGID() == pMsg->mDefenderGID || 0 == m_CurHp || EA_JUMP == GetCurActionID())
		return FALSE;

	if (!m_ParentRegion)
	{
		//StackWalker wp;
		//wp.ShowCallstack();
		if (CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER))
		{
			rfalse(4,1," CFightObject::ProcessQuestSkill 玩家所在地图为NULL,account = %s",pPlayer->GetAccount());
		}
		else if (CMonster *pmonster = (CMonster*)DynamicCast(IID_MONSTER))
		{
			rfalse(4, 1, " CFightObject::ProcessQuestSkill 怪物所在地图为NULL,monsterId = %d",pmonster->m_Property.m_ID);
		}
		else if (CFightPet *pFightPet = (CFightPet*)DynamicCast(IID_FIGHT_PET))
		{
			rfalse(4, 1, " CFightObject::ProcessQuestSkill 宠物所在地图为NULL,account = %s", pFightPet->m_owner->GetAccount());
		}
		else
		{
			rfalse(4, 1, " CFightObject::ProcessQuestSkill 未知所在地图为NULL,gid = %d", GetGID());
		}
		return FALSE;
	}


	//获得相关数据
	///////////////////////////////////////////////////////////////////////////
	//SkillIndex:玩家身上有所学技能的索引
	//这里根据索引找出在“技能配置表”中的技能ID，亦可以验证玩家是否学会了该技能
	INT32 skillID = GetSkillIDBySkillIndex(pMsg->dwSkillIndex);
	if (-1 == skillID)
	{
		//rfalse(3, 1, "ProcessQuestSkill dwSkillIndex = %d", pMsg->dwSkillIndex);
		return FALSE;
	}
		

	//取得“技能配置表”中存放的，该技能的基本数据
	const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(skillID);
	if (!pData || SKT_PASSITIVE == pData->m_SkillType)
		return FALSE;

	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	///////////////////////////////////////////////////////////////////////////
	if (pPlayer)
	{
		//rfalse("ProcessQuestSkill skillID = %d,startX = %f,startY = %f",skillID,pMsg->mDefenderWorldPosX,pMsg->mDefenderWorldPosY);
		if (0 == pPlayer->m_Property.m_Equip[EQUIP_P_WEAPON].wIndex)
		{
			TalkToDnid(pPlayer->m_ClientIndex, "没有装备武器，不能使用技能!");
			return FALSE;
		}
	}



	// 单攻消息，验证目标的有效性
	if (SAST_SINGLE == pData->m_AtkType)
	{
		LPIObject pObject = m_ParentRegion->SearchObjectListInAreas(pMsg->mDefenderGID, m_ParentArea->m_X, m_ParentArea->m_Y);
		CFightObject *pLife = (CFightObject *)pObject->DynamicCast(IID_FIGHTOBJECT);
		if (!pLife || 0 == pLife->m_CurHp)
		{
			rfalse("target not find");
			//return FALSE;
		}

		if (pPlayer&&pLife)
		{
			pPlayer->StoreTarget(pLife);
			//rfalse(2, 1, "玩家攻击目标%d\r\n",pLife->GetGID());
			if (pPlayer->m_pFightPet) pPlayer->m_pFightPet->SetCurEnemy(pMsg->mDefenderGID, FPET_PLAYERATK);
		}
	}

	//PK模式检测没有通过
	if (pPlayer && !pPlayer->CheckPkRuleAttk(pMsg->mDefenderGID,pData->m_AtkType))
	{
		return FALSE;
	}
	///////////////////////////////////////////////////////////////////////////


	m_backUpQusetSkillMsg = *pMsg;

	m_AtkContext.mAttackerGID = pMsg->mAttackerGID;
	m_AtkContext.mDefenderGID = pMsg->mDefenderGID;
	m_AtkContext.mDefenderWorldPosX = pMsg->mDefenderWorldPosX;
	m_AtkContext.mDefenderWorldPosY = pMsg->mDefenderWorldPosY;

	// 	if ()
	// 	{
	// 	}

	//自动进行或取消一些操作
	///////////////////////////////////////////////////////////////////////////
#ifndef SUPPORT_IN_MOUNT_FIGHT // 是否支持骑乘战斗
	//自动下马
	if (pPlayer&&pPlayer->m_RidingStatus!=0)
	{
		pPlayer->DownMounts();
	}
#endif

	//自动取消打坐
	if (pPlayer&&pPlayer->GetCurActionID()==EA_ZAZEN)
	{
		pPlayer->SetZazen(FALSE);
	}

	// 取消当前的延时操作
	if (pPlayer)
	{
		pPlayer->CancelCollect();		// 脱离采集状态
		//pPlayer->BreakOperation();		// 中断各种操作
		pPlayer->m_BatchReadyEvent.BreakOperationReadyEvent(Event::EEventInterrupt_UseSkill);
	}
	///////////////////////////////////////////////////////////////////////////	
	///////////////////////////////////////////////////////////////////////////
	//有些技能可能需要客户端验证	
	// [2012-7-21 16-35 gw: +改变检测的位置，检测得更严密一些]
	// 	if(!CheckAttackByClient(pData, pMsg))
	// 		return false;  放入 lua中缓存
	//////////////////////////////////////////////////////////////////////////
	// 	bool skipState = false;
	//GetcontinueSkill(pMsg);
	// 	if (pPlayer)
	// 	{
	// 		g_Script.SetPlayer(pPlayer);
	// 		g_Script.PrepareFunction("ProcessQuestSkill");
	// 		g_Script.PushParameter(pMsg->mAttackerGID);
	// 		g_Script.PushParameter(pMsg->mDefenderGID);
	// 		g_Script.PushParameter(pMsg->mDefenderWorldPosX);
	// 		g_Script.PushParameter(pMsg->mDefenderWorldPosY);
	// 		g_Script.PushParameter(pMsg->dwSkillIndex);
	// 		g_Script.Execute();
	// 		g_Script.CleanPlayer();
	// 	}
	// 收到后马上处理（参数false表示有机会对技能做缓冲）

	BOOL bRet = FALSE;
	if (pPlayer)
	{
		if (pData && pData->m_SkillType == 3)
		{
			bRet = pPlayer->CheckGoods(pData->m_ConsumeItemID, pData->m_OnceNeedItemCount, TRUE);
			if (!bRet)
			{
				TalkToDnid(pPlayer->m_ClientIndex, "使用该技能所需道具不足");
				return 0;
			}
			pPlayer->DeleteItem(pData->m_ConsumeItemID, pData->m_OnceNeedItemCount);
		}
	}
	//处理技能攻击
	bRet = __ProcessQuestSkill(&m_backUpQusetSkillMsg, false);
	return bRet;
}

BOOL CFightObject::ProcessQuestSkill(SQuestSkillMultiple_C2S_MsgBody *pMsg)
{
	if (!pMsg || 0 == m_CurHp || EA_JUMP == GetCurActionID())
		return FALSE;

	if (!m_ParentRegion)
	{
		//StackWalker wp;
		//wp.ShowCallstack();
		if (CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER))
		{
			rfalse(4, 1, " CFightObject::ProcessQuestSkill 玩家所在地图为NULL,account = %s", pPlayer->GetAccount());
		}
		else if (CMonster *pmonster = (CMonster*)DynamicCast(IID_MONSTER))
		{
			rfalse(4, 1, " CFightObject::ProcessQuestSkill 怪物所在地图为NULL,monsterId = %d", pmonster->m_Property.m_ID);
		}
		else if (CFightPet *pFightPet = (CFightPet*)DynamicCast(IID_FIGHT_PET))
		{
			rfalse(4, 1, " CFightObject::ProcessQuestSkill 宠物所在地图为NULL,account = %s", pFightPet->m_owner->GetAccount());
		}
		else
		{
			rfalse(4, 1, " CFightObject::ProcessQuestSkill 未知所在地图为NULL,gid = %d", GetGID());
		} 
		return FALSE;
	}
	BYTE num = pMsg->bTargetNum;
	BYTE *ptemp = pMsg->buffer;
	m_AtkContext.m_SkillAttackPos.x = pMsg->mAttackerWorldPosX;
	m_AtkContext.m_SkillAttackPos.y = pMsg->mAttackerWorldPosY;
	if (m_AtkContext.m_SkillAttackPos.x > 0.0f || m_AtkContext.m_SkillAttackPos.y > 0.0f)
	{
		D3DXVECTOR2 curPos(m_curX, m_curY);
		D3DXVECTOR2 desPos(pMsg->mAttackerWorldPosX, pMsg->mAttackerWorldPosY);
		D3DXVECTOR2 desVector = desPos - curPos;
		D3DXVec2Normalize(&desVector, &desVector);
		// 计算方向
		m_Direction = atan2(desVector.x, desVector.y);
	}

	if (m_ClientQuestskill.size() > 0)
	{
		std::vector<QuestSkill*>::iterator iter = m_ClientQuestskill.begin();
		while (iter != m_ClientQuestskill.end())
		{
			SAFE_DELETE(*iter);
			iter++;
		}
		m_ClientQuestskill.clear();
	}

	if (num > 0)
	{
		SQuestSkill_C2S_MsgBody questskillmsg;
		for (size_t i = 0; i < num; i++)
		{
			if (ptemp)
			{
				QuestSkill *questskill = new QuestSkill;
				memset(questskill, 0, sizeof(QuestSkill));
				memcpy(questskill, ptemp, sizeof(QuestSkill));
				//rfalse(2, 1, "SQuestSkillMultiple GID = %d , dwSkillIndex = %d", questskill.mDefenderGID, pMsg->dwSkillIndex);
				LPIObject pObject = m_ParentRegion->SearchObjectListInAreas(questskill->mDefenderGID, m_ParentArea->m_X, m_ParentArea->m_Y);
				CFightObject *pLife = (CFightObject *)pObject->DynamicCast(IID_FIGHTOBJECT);
				if (pLife && pLife->m_CurHp > 0)
				{
					questskillmsg.dwSkillIndex = pMsg->dwSkillIndex;
					questskillmsg.mAttackerGID = pMsg->mAttackerGID;
					questskillmsg.mDefenderGID = questskill->mDefenderGID;
					questskillmsg.mDefenderWorldPosX = questskill->mDefenderWorldPosX;
					questskillmsg.mDefenderWorldPosY = questskill->mDefenderWorldPosY;
					//rfalse("ProcessQuestSkill GID = %d,PosX = %f,PosY = %f", questskill->mDefenderGID, questskill->mDefenderWorldPosX, questskill->mDefenderWorldPosY);
					m_ClientQuestskill.push_back(questskill);
				}
				ptemp += sizeof(QuestSkill);
			}
		}
		ProcessQuestSkill(&questskillmsg);
		return TRUE;
	}
	SQuestSkill_C2S_MsgBody questskillmsg;
	questskillmsg.dwSkillIndex = pMsg->dwSkillIndex;
	questskillmsg.mAttackerGID = pMsg->mAttackerGID;
	questskillmsg.mDefenderWorldPosX = pMsg->mAttackerWorldPosX;
	questskillmsg.mDefenderWorldPosY = pMsg->mAttackerWorldPosY;
	ProcessQuestSkill(&questskillmsg);
	return TRUE;
}

bool CFightObject::CheckAttackByClient(const SSkillBaseData *pData, SQuestSkill_C2S_MsgBody *questMsg)
{
	if (!pData || !questMsg)
	{
		rfalse(4, 1, "Fightobject.cpp - CheckAttackByClient() - !pData || !questMsg");
		return false;
	}

	if(SPT_CHARGE==pData->m_SkillPropertyType)
	{
		g_Script.SetFightCondition(this);
		LuaFunctor(g_Script, "CheckAttackByClient")[questMsg->mDefenderGID][pData->m_ID][pData->m_SkillSubProperty]();
		g_Script.CleanFightCondition();

		return false;
	}

	return true;
}

void CFightObject::StartWindMove(SFightBaseMsg *pMsg)
{
	SQWinMoveCheckMsg *pWindMoveMsg = (SQWinMoveCheckMsg*)pMsg;
	if (!pWindMoveMsg)
	{
		rfalse(4, 1, "FightOjbect.cpp - StartWindMove() - !pWindMoveMsg");
		return;
	}


}

void CFightObject::UpdateSkillAttack()
{
	switch (m_AtkContext.m_SkillAttackSubState)    
	{
	case SKILL_ATTACK_DELAY:
		SkillAttackDelay();
		break;

	case SKILL_ATTACK_CONTINUE_ATTACK:
		if (!ContinueSkillAttack())
		{
			SetCurActionID(EA_STAND);
			// 			CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
			// 			if (!pPlayer)
			// 				break;

			//TalkToDnid(pPlayer->m_ClientIndex, "攻击完毕了！！！");
		}
		break;

	default:
		break;
	}
}

void CFightObject::SendSwitch2PrepareSkillAttackStateMsg(int preTime)
{
	SSwitch2PrepareSkillAttackState_S2C_MsgBody msg;
	msg.mAttackGID		= GetGID();
	msg.mDefenderGID	= m_AtkContext.mDefenderGID;
	msg.mCastTime		= preTime;
	msg.m_CurSkillID	= m_AtkContext.m_SkillData.m_ID;
	msg.fsingDir = m_Direction;

	// 发送吟唱消息
	if (m_ParentArea)
		m_ParentArea->SendAdj(&msg, sizeof(msg), -1);
}

void CFightObject::SendSwitch2SkillAttackStateMsg()
{
	MY_ASSERT(EA_SKILL_ATTACK == GetCurActionID());
	if (EA_SKILL_ATTACK != GetCurActionID())return;

	SSwitch2SkillAttackState_S2C_MsgBody msg;
	msg.mAttackGID		= GetGID();
	msg.m_CurSkillID	= m_AtkContext.m_SkillData.m_ID;
	if (SPT_CHARGE == m_AtkContext.m_SkillData.m_SkillPropertyType || SPT_MOVE == m_AtkContext.m_SkillData.m_SkillPropertyType)
	{
		msg.mAttackerWorldPosX = m_AtkContext.m_SkillAttackPos.x;
		msg.mAttackerWorldPosY = m_AtkContext.m_SkillAttackPos.y;
	}
	else 
	{
		msg.mAttackerWorldPosX = m_curX;
		msg.mAttackerWorldPosY = m_curY;
	}

	memset(msg.buffer, 0, 1024);
	BYTE *ptemp = msg.buffer;

	if (m_vQuestskill.size() > 0)
	{
		msg.btargetnum = m_vQuestskill.size();
		std::vector<QuestSkill*>::iterator iter = m_vQuestskill.begin();
		while (iter != m_vQuestskill.end())
		{
			QuestSkill* pqskill = *iter;
			if (pqskill)
			{
				memcpy(ptemp, pqskill, sizeof(QuestSkill));
				ptemp += sizeof(QuestSkill);
			}
			iter++;
		}
	}
	else
	{
		msg.btargetnum = 1;
		QuestSkill qskill;
		qskill.mDefenderGID = m_AtkContext.mDefenderGID;
		qskill.mDefenderWorldPosX = m_AtkContext.mDefenderWorldPosX;
		qskill.mDefenderWorldPosY = m_AtkContext.mDefenderWorldPosY;
		memcpy(msg.buffer, &qskill, sizeof(QuestSkill));
	}

// 	if (CPlayer *player = (CPlayer*)DynamicCast(IID_PLAYER))
// 	{
// 		BYTE num = msg.btargetnum;
// 		for (size_t i = 0; i < num; i++)
// 		{
// 			QuestSkill *qskill=new QuestSkill;
// 			BYTE *pbuffer = msg.buffer;
// 			memcpy(qskill, pbuffer, sizeof(QuestSkill));
// 			pbuffer += sizeof(QuestSkill);
// 			rfalse(2, 1, "SendSwitch2SkillAttackStateMsg MGID = %d ,%4.2f,%4.2f  ", qskill->mDefenderGID,qskill->mDefenderWorldPosX,qskill->mDefenderWorldPosY);
// 		}
// 	}

	// 发送攻击消息
	if (m_ParentArea)
		m_ParentArea->SendAdj(&msg, sizeof(msg), -1);
}

void CFightObject::IntoFightState(CFightObject *pDefencer)
{
	// 被攻击者应该加入攻击者的攻击列表中
	if (pDefencer)
		m_Attacker[pDefencer->GetGID()] = timeGetTime();
	else
		m_Attacker[0xffffffff] = timeGetTime();

	bool oldFightState = m_IsInFight;
	m_IsInFight = true;

	if (!oldFightState)
		SwitchFightState(true);

	if (pDefencer && pDefencer->m_CurHp)
	{
		// 攻击者应该加入被攻击者的攻击列表
		pDefencer->m_Attacker[GetGID()] = timeGetTime();

		bool oldFightState = pDefencer->m_IsInFight;
		pDefencer->m_IsInFight = true;

		if (!oldFightState)
			pDefencer->SwitchFightState(true);
	}
}

void CFightObject::SkillAttackStart(const SSkillHitSpot *pHitPot)
{
	if (!pHitPot)
	{
		rfalse(4, 1, "Fightobject.cpp - SkillAttackStart() - !pHitPot");
		return;
	}

	//m_AtkContext.m_enableNextSkill			= false;
	m_AtkContext.m_SkillAttackStartTime		= timeGetTime();
	m_AtkContext.m_SkillPhaseStartTime		= m_AtkContext.m_SkillAttackStartTime;
	m_AtkContext.m_SkillAttackSubState		= SKILL_ATTACK_DELAY;
	m_AtkContext.m_Phase					= SKS_PREPARE;

	m_AtkContext.m_PrepareTime				= 0;
	m_AtkContext.m_PrepareOver				= false;

	m_AtkContext.m_SingTime					= 0;
	m_AtkContext.m_SingOver					= false;

	m_AtkContext.m_BeforeAttack				= 0;			
	m_AtkContext.m_BeforeAtkOver			= false;	

	memcpy(m_AtkContext.m_bHitSpot, pHitPot->m_HitPots, sizeof(m_AtkContext.m_bHitSpot));
	memcpy(m_AtkContext.m_CalcDamg, pHitPot->m_CalcDmg, sizeof(m_AtkContext.m_CalcDamg));
	m_AtkContext.m_WholeAtkNumber = pHitPot->m_HitCount;

	// 根据攻击速度做比例缩放
	if (m_AtkSpeed != PLAYER_BASIC_ATKSPEED)
	{
		for (size_t i = 0; i < m_AtkContext.m_WholeAtkNumber; i++)
		{
			if (m_AtkSpeed > PLAYER_BASIC_ATKSPEED)
				m_AtkContext.m_bHitSpot[i] -= (m_AtkContext.m_bHitSpot[i] * (m_AtkSpeed-PLAYER_BASIC_ATKSPEED)/100);
			else
				m_AtkContext.m_bHitSpot[i] += (m_AtkContext.m_bHitSpot[i] * (PLAYER_BASIC_ATKSPEED-m_AtkSpeed)/100);
		}

		for (size_t i = 0; i < m_AtkContext.m_WholeAtkNumber; i++)
		{
			rfalse(2, 1, "【%d】 ", m_AtkContext.m_bHitSpot[i]);
		}
	}

	m_AtkContext.m_StartAttack		= 0;
	m_AtkContext.m_CurAtkNumber		= 0;					// 当前已经进行了几次攻击
	m_AtkContext.m_bIsProcessDamage	= false;

	m_AtkContext.m_AfterAttack		= 0;
	m_AtkContext.m_AfterAtkOver		= false;

	m_AtkContext.m_bSkillAtkOver	= false;

	// 产生消耗，不管命中与否都有消耗，群攻和单攻都只消耗一次
	if (m_AtkContext.m_consumeHP) ConsumeHP(m_AtkContext.m_consumeHP);
	if (m_AtkContext.m_consumeMP) ConsumeMP(m_AtkContext.m_consumeMP);

	SkillAttackDelay();
}


// 动作播放时间目前直接定为800
#define  SKILL_ACTION_TIME ( 800 )

// 更新技能信息
void CFightObject::SkillAttackDelay()
{
	DWORD CurrentTime = timeGetTime();

	//add by ly 2014/4/11 过滤掉进入关卡时，m_AtkContext里面的没有技能数据
	if (m_AtkContext.m_SkillData.m_ID == 0)
	{
		
		rfalse(2, 1, "m_AtkContext.m_SkillData.m_ID == 0 ,dwSkillIndex = %d", m_AtkContext.dwSkillIndex);
		return;
	}
	

	if (SKS_PREPARE == m_AtkContext.m_Phase)
	{
		//rfalse(2, 1, "%d SkillStart ",m_AtkContext.m_SkillData.m_ID);
		if (false == m_AtkContext.m_PrepareOver)
		{
			m_AtkContext.m_PrepareOver = true;
			m_AtkContext.m_PrepareTime = 0;
		}

		DWORD e = CurrentTime - m_AtkContext.m_SkillPhaseStartTime;
		e += 5;		// 做一次微调

		if (e >= m_AtkContext.m_PrepareTime)
		{
			m_AtkContext.m_SkillPhaseStartTime += m_AtkContext.m_PrepareTime;

			m_AtkContext.m_Phase = SKS_SING;

			//设置吟唱时间并发送消息
			if(m_AtkContext.m_SkillData.m_SingTime>0)
			{
				//if (m_AtkContext.m_SingTime = m_AtkContext.m_PrepareTime)
				m_AtkContext.m_SingTime = m_AtkContext.m_SkillData.m_SingTime;
				SendSwitch2PrepareSkillAttackStateMsg(m_AtkContext.m_SingTime);				
			}
			
			StartGlobalCDTimer(&m_AtkContext.m_SkillData);
		}
	}

	if (SKS_SING == m_AtkContext.m_Phase)
	{
		// 处于吟唱阶段
		DWORD e = CurrentTime - m_AtkContext.m_SkillPhaseStartTime;
		e += 5;		// 做一次微调
	//	rfalse(2, 1, "%d SKS_SING ", m_AtkContext.m_SkillData.m_ID);
		if (e >= m_AtkContext.m_SingTime)
		{
			m_AtkContext.m_BeforeAttack  = 0;

			// 吟唱完毕
			m_AtkContext.m_SingOver = true;
			m_AtkContext.m_Phase = SKS_BEFORE;

			// 设置下一个阶段开始的时间
			m_AtkContext.m_SkillPhaseStartTime += m_AtkContext.m_SingTime;			
		}
	}

	if (SKS_BEFORE == m_AtkContext.m_Phase)
	{
		m_AtkContext.m_BeforeAtkOver = true;
		//rfalse(2, 1, "%d SKS_BEFORE ", m_AtkContext.m_SkillData.m_ID);
		if (m_vQuestskill.size()>0)
		{
			std::vector<QuestSkill*>::iterator iter = m_vQuestskill.begin();
			while (iter != m_vQuestskill.end())
			{
				SAFE_DELETE(*iter);
				iter++;
			}
			m_vQuestskill.clear();
		}
		// 处于攻击前奏期间
		DWORD e = CurrentTime - m_AtkContext.m_SkillPhaseStartTime;
		//e += 5;		// 做一次微调
		if (e >= m_AtkContext.m_BeforeAttack)
		{
			// 受控位移类技能
			if (SPT_MOVE == m_AtkContext.m_SkillData.m_SkillPropertyType || SPT_REPEL == m_AtkContext.m_SkillData.m_SkillPropertyType)
			{
				if (m_AtkContext.m_SkillData.m_AtkType != SAST_SINGLE && m_AtkContext.m_SkillData.m_AtkType != SAST_SELF)
				{
					if (m_ClientQuestskill.size()>0)
					{
						std::vector<QuestSkill*>::iterator iter = m_ClientQuestskill.begin();
						while (iter != m_ClientQuestskill.end())
						{
							QuestSkill* pqskill = *iter;
							if (pqskill)
							{
								LPIObject pObj = m_ParentRegion->SearchObjectListInAreas(pqskill->mDefenderGID, m_ParentArea->m_X, m_ParentArea->m_Y);
								if (pObj)
								{
									CFightObject *pDest = (CFightObject *)pObj->DynamicCast(IID_FIGHTOBJECT);
									if (pDest)
									{
										if (!(pDest->m_fightState & FS_ADDBUFF) && !(pDest->m_fightState & FS_JITUI))
										{
											pDest->m_CurNode = pDest->m_NodeCount = 0;
											D3DXVECTOR2 curPos(m_curX, m_curY);
											D3DXVECTOR2 desPos(pDest->m_curX, pDest->m_curY);
											D3DXVECTOR2 desVector = desPos - curPos;
											D3DXVec2Normalize(&desVector, &desVector);
											float fRadian = atan2(desVector.y, desVector.x); //当前方向
											D3DXVECTOR2 vMovePos;
											pqskill->mDefenderWorldPosX = pDest->m_curX + m_AtkContext.m_SkillData.m_HitBackDis * cos(fRadian);
											pqskill->mDefenderWorldPosY = pDest->m_curY + m_AtkContext.m_SkillData.m_HitBackDis * sin(fRadian);
											pqskill->bDefenderState = 1;  //击退状态
											pDest->ChangeFightState(BAP_JITUI, true, GetGID(), pqskill->mDefenderWorldPosX, pqskill->mDefenderWorldPosY, m_AtkContext.m_SkillData.m_ID);
										}
										else
										{
											pqskill->mDefenderWorldPosX = pDest->m_curX ;
											pqskill->mDefenderWorldPosY = pDest->m_curY;
											pqskill->bDefenderState = 0;  //
										}
									}
								}
								else
								{
									QuestSkill *qskill = new QuestSkill;
									qskill->mDefenderGID = m_AtkContext.mDefenderGID;
									qskill->mDefenderWorldPosX = m_AtkContext.mDefenderWorldPosX;
									qskill->mDefenderWorldPosY = m_AtkContext.mDefenderWorldPosY;
									qskill->bDefenderState = 0;  //无状态
									m_ClientQuestskill.push_back(qskill);
								}
							}
							iter++;
						}
					}
				}
				else
				{
					if (m_ClientQuestskill.size() > 0)
					{
						QuestSkill* pqskill = m_ClientQuestskill[0];
						if (pqskill)
						{
								LPIObject pObj = m_ParentRegion->SearchObjectListInAreas(pqskill->mDefenderGID, m_ParentArea->m_X, m_ParentArea->m_Y);
								if (pObj)
								{
									CFightObject *pDest = (CFightObject *)pObj->DynamicCast(IID_FIGHTOBJECT);
									if (!(pDest->m_fightState & FS_ADDBUFF) && !(pDest->m_fightState & FS_JITUI))
									{
										pDest->m_CurNode = pDest->m_NodeCount = 0;
										//pDest->SetCurActionID(EA_STAND);

										D3DXVECTOR2 curPos(m_curX, m_curY);
										D3DXVECTOR2 desPos(pDest->m_curX, pDest->m_curY);
										D3DXVECTOR2 desVector = desPos - curPos;
										D3DXVec2Normalize(&desVector, &desVector);
										float fRadian = atan2(desVector.y, desVector.x); //当前方向
										D3DXVECTOR2 vMovePos;  
										pqskill->mDefenderWorldPosX = pDest->m_curX + m_AtkContext.m_SkillData.m_HitBackDis * cos(fRadian);
										pqskill->mDefenderWorldPosY = pDest->m_curY + m_AtkContext.m_SkillData.m_HitBackDis * sin(fRadian);
										pqskill->bDefenderState = 1;  //击退状态
										pDest->ChangeFightState(BAP_JITUI, true, GetGID(), pqskill->mDefenderWorldPosX, pqskill->mDefenderWorldPosY, m_AtkContext.m_SkillData.m_ID);
									}
									else
									{
										pqskill->mDefenderWorldPosX = pDest->m_curX;
										pqskill->mDefenderWorldPosY = pDest->m_curY;
										pqskill->bDefenderState = 0;  //击退状态
									}
								}
						}
					}
					else
					{
						LPIObject pObj = m_ParentRegion->SearchObjectListInAreas(m_AtkContext.mDefenderGID, m_ParentArea->m_X, m_ParentArea->m_Y);
						//if (!pObj || !pObj->isValid())return;
						if (pObj)
						{
							CFightObject *pDest = (CFightObject *)pObj->DynamicCast(IID_FIGHTOBJECT);
							if (!(pDest->m_fightState & FS_ADDBUFF) && !(pDest->m_fightState & FS_JITUI))
							{
								D3DXVECTOR2 curPos(m_curX, m_curY);
								D3DXVECTOR2 desPos(pDest->m_curX, pDest->m_curY);
								D3DXVECTOR2 desVector = desPos - curPos;
								D3DXVec2Normalize(&desVector, &desVector);
								float fRadian = atan2(desVector.y, desVector.x); //当前方向
								D3DXVECTOR2 vMovePos;
								QuestSkill *qskill = new QuestSkill;
								qskill->mDefenderGID = pDest->GetGID();
								qskill->mDefenderWorldPosX = pDest->m_curX + m_AtkContext.m_SkillData.m_HitBackDis * cos(fRadian);
								qskill->mDefenderWorldPosY = pDest->m_curY + m_AtkContext.m_SkillData.m_HitBackDis * sin(fRadian);
								qskill->bDefenderState = 1;  //击退状态
								m_ClientQuestskill.push_back(qskill);
								pDest->ChangeFightState(BAP_JITUI, true, GetGID());
							}
							else
							{
								QuestSkill *qskill = new QuestSkill;
								qskill->mDefenderGID = pDest->GetGID();
								qskill->mDefenderWorldPosX = pDest->m_curX;
								qskill->mDefenderWorldPosY = pDest->m_curY;
								qskill->bDefenderState = 0;  //无状态
								m_ClientQuestskill.push_back(qskill);
							}
						}
						else
						{
							QuestSkill *qskill = new QuestSkill;
							qskill->mDefenderGID = m_AtkContext.mDefenderGID;
							qskill->mDefenderWorldPosX = m_AtkContext.mDefenderWorldPosX;
							qskill->mDefenderWorldPosY = m_AtkContext.mDefenderWorldPosY;
							qskill->bDefenderState = 0;  //无状态
							m_ClientQuestskill.push_back(qskill);
						}
					}
				}
			}

			if (m_ClientQuestskill.size() > 0)
			{
				std::vector<QuestSkill*>::iterator iter = m_ClientQuestskill.begin();
				while (iter != m_ClientQuestskill.end())
				{
					QuestSkill* pqskill = *iter;
					if (pqskill)
					{
						QuestSkill  *pQuestskill = new QuestSkill;
						memcpy(pQuestskill, pqskill, sizeof(QuestSkill));
						m_vQuestskill.push_back(pQuestskill);
					}
					iter++;
				}
			}
			SendSwitch2SkillAttackStateMsg();   
			m_AtkContext.m_SkillPhaseStartTime += m_AtkContext.m_BeforeAttack;
			MY_ASSERT(m_AtkContext.m_WholeAtkNumber);
			m_AtkContext.m_Phase = SKS_ATTACK;
			// 设置下个阶段的时间，下个阶段是开始攻击，所以第一个时间点应该是第一个打击点的时间
			m_AtkContext.m_StartAttack = m_AtkContext.m_bHitSpot[m_AtkContext.m_CurAtkNumber];
		}
	}

	if (SKS_ATTACK == m_AtkContext.m_Phase)
	{
		//rfalse(2, 1, "%d SKS_ATTACK ", m_AtkContext.m_SkillData.m_ID);
		while (true)
		{
			// 处于攻击中
			DWORD e = CurrentTime - m_AtkContext.m_SkillPhaseStartTime;
			e += 5;		// 做一次微调

			if (e >= m_AtkContext.m_StartAttack)
			{
				// 设置下一个阶段开始的时间
				m_AtkContext.m_SkillPhaseStartTime += m_AtkContext.m_StartAttack;

				m_AtkContext.m_CurAtkNumber++;
				if (m_AtkContext.m_WholeAtkNumber == m_AtkContext.m_CurAtkNumber)
				{
					ProcessHitPot();				// 对目标造成伤害/附加Buff

					// 切换到下个阶段
					m_AtkContext.m_bIsProcessDamage = true;
					m_AtkContext.m_Phase = SKS_AFTER;
					break;
				}
				else
				{
					ProcessHitPot();				// 对目标造成伤害/附加Buff
					m_AtkContext.m_StartAttack = m_AtkContext.m_bHitSpot[m_AtkContext.m_CurAtkNumber];
				}
			}
			else
				break;
		}
	}	

	if (SKS_AFTER == m_AtkContext.m_Phase)
	{
		if (false == m_AtkContext.m_AfterAtkOver)
		{
			m_AtkContext.m_AfterAtkOver = true;
			m_AtkContext.m_AfterAttack  = 0;
		}

		// 处于攻击收尾
		DWORD e = CurrentTime - m_AtkContext.m_SkillPhaseStartTime;
		e += 5;		// 做一次微调

		if (e >= m_AtkContext.m_AfterAttack)
		{
			m_AtkContext.m_Phase = SKS_OVER;

			// 设置下一个阶段开始的时间
			m_AtkContext.m_SkillPhaseStartTime += m_AtkContext.m_AfterAttack;
		}
	}

	if (SKS_OVER == m_AtkContext.m_Phase)
	{
		SkillAttackEnd();
		
	}
}

void CFightObject::ProcessHitPot()
{
	m_AtkContext.m_TargetList.clear();

	switch (m_AtkContext.m_SkillData.m_AtkType)
	{
	case SAST_SINGLE:			// 单攻
		DamageObject_Single();
		break;

	case SAST_EMPTY_CIRCLE:		// 圆形群攻（自身原点）
	case SAST_EMPTY_SECTOR:		// 扇形群攻（自身原点）
	case SAST_EMPTY_POINTS:		// 直线群攻（自身原点）
		DamageObject_AttackMore(true);
		break;

	case SAST_POINT_CIRCLE:		// 圆形群攻（目标点）
		DamageObject_AttackMore(false);
		break;

	case SAST_SELF:				// 对自己释放
		break;

	case SAST_POINT_SINGLE:		// 对目标点释放（瞬移，陷阱，产生各种效果。。。）
		break;
	}

	// 普通攻击应该略过，因为没有特殊的效果
	//if (!m_AtkContext.m_SkillData.m_IsNormalSkill)
	{
		INT32 SkillLevel = GetCurrentSkillLevel();

		g_Script.SetFightCondition(this);
		if (g_Script.PrepareFunction(FormatString("InAttack%d", m_AtkContext.m_SkillData.m_ID)))
		{
			g_Script.PushParameter(m_AtkContext.m_CurAtkNumber);
			g_Script.PushParameter(SkillLevel);
			g_Script.Execute();
		}
		g_Script.CleanFightCondition();
		
// 		LuaFunctor(g_Script, FormatString("InAttack%d", m_AtkContext.m_SkillData.m_ID))[m_AtkContext.m_CurAtkNumber][SkillLevel]();
// 		g_Script.CleanFightCondition();


	}

	return;
}

void CFightObject::SkillAttackEnd()
{
	m_AtkContext.m_TargetList.clear();

	m_AtkContext.m_bSkillAtkOver		= true;
	m_AtkContext.m_SkillAttackSubState	= SKILL_ATTACK_CONTINUE_ATTACK;
	//SetCurActionID(EA_STAND);
	// 	CPlayer *player = (CPlayer*)DynamicCast(IID_PLAYER);
	// 	if (player)
	// 	{
	// 		g_Script.SetPlayer(player);
	// 		g_Script.PrepareFunction("SkillAttackEnd");
	// 		g_Script.PushParameter(GetGID());
	// 		g_Script.Execute();
	// 		g_Script.CleanPlayer();
	// 	}
}

void CFightObject::SendAttackMissMsg(INT32 state)
{
	LPIObject pObj = m_ParentRegion->SearchObjectListInAreas(m_AtkContext.mDefenderGID, m_ParentArea->m_X, m_ParentArea->m_Y);

	if (!pObj || !pObj->isValid())
		return;

	CFightObject *pDest = (CFightObject *)pObj->DynamicCast(IID_FIGHTOBJECT);
	if (!pDest) 
		return;

	SAWoundObjectMsgEx woundMsg;
	InitWoundMsg_Normal(&woundMsg);

	SWoundTiny *wound = (SWoundTiny *)woundMsg.streamData;
	SDamage damage;
	//pDest->InitWoundTiny(wound, damage, state);

	WORD size = offsetof(SAWoundObjectMsgEx, streamData) + sizeof(SWoundTiny) * woundMsg.mWoundObjectNumber;
	Synchro(&woundMsg, size);
}

bool CFightObject::ContinueSkillAttack()
{
	while(1)
	{
		if (GetContinueSkillTarget(m_LuaQuestSkillMsg) == 1)  //有效目标
		{
			return ProcessQuestSkill(&m_LuaQuestSkillMsg);
			break;
		}
		else if (GetContinueSkillTarget(m_LuaQuestSkillMsg) == 0) //无效目标
		{
			//m_vcacheQuestSkill.erase(m_vcacheQuestSkill.begin());			//继续查找目标
		}
		else													//无目标
		{
			break;
		}
	}
	return false;
	//return __ProcessQuestSkill(&m_backUpQusetSkillMsg, true);
}

bool CFightObject::DamageEnemyBySkillAttack(CFightObject* pDest, SDamage& damage)
{
	if (!pDest)
	{
		rfalse(4, 1, "Fightobject.cpp - DamageEnemyBySkillAttack() - !pDest");
		return false;
	}

	CalculateDamage(pDest, damage);
	return true;
}

void CFightObject::CalculateDamage(CFightObject *pDest, SDamage& damage)
{
	if (!pDest)
	{
		rfalse(4, 1, "Fightobject.cpp - CalculateDamage() - !pDest");
		return;
	}

	if (pDest->DynamicCast(IID_MONSTER))
	{
		CalculateDamagePVE(this, pDest, damage);
	}
	else
	{
		if (DynamicCast(IID_MONSTER))
			CalculateDamageEVP(this, pDest, damage);
		else
			CalculateDamagePVP(this, pDest, damage);
	}
}

int CFightObject::ModifyCurrentHP(int mod, int type, CFightObject *pReason, bool curInterrupt)
{
	if (m_CurHp <= 0 || 0 == mod)
		return 0;

	int oldHP = (int)m_CurHp;

	int currentHP = 0;

	if (BAM_POT == type)
	{
		currentHP = oldHP + mod;
	}
	else if (BAM_PRECENT == type)
	{
		currentHP = oldHP + (int)((float)oldHP*(float)(mod / 100.0));
	}

	int modified = 0;
	if (currentHP > (int)m_MaxHp)
	{
		m_CurHp	 = m_MaxHp;
		modified = (int)m_MaxHp - oldHP;
	}
	else if (currentHP <= 0)
	{
		m_CurHp  = 0;
		modified = oldHP;

		if (pReason)
			pReason->Kill(this);
		else
			OnDead(0);
	}
	else
	{
		m_CurHp  = currentHP;
		modified = currentHP - oldHP;
	}

	if (0 != modified)
		m_FightPropertyStatus[XA_CUR_HP] = true;

	return modified;
}

int CFightObject::ModifyCurrentMP(int mod, int type, bool curInterrupt)
{
	if (m_CurHp <= 0 || 0 == mod)
		return 0;

	int oldMP = (int)m_CurMp;

	int currentMP = 0;

	if (BAM_POT == type)
	{
		currentMP = oldMP + mod;
	}
	else if (BAM_PRECENT == type)
	{
		currentMP = oldMP + (int)((float)oldMP*(float)(mod / 100.0));
	}

	int modified = 0;
	if (currentMP > (int)m_MaxMp)
	{
		m_CurMp = m_MaxMp;
		modified = (int)m_MaxMp - oldMP;
	}
	else if (currentMP <= 0)
	{
		m_CurMp = 0;
		modified = oldMP;
	}
	else
	{
		m_CurMp = currentMP;
		modified = currentMP - oldMP;
	}

	if (0 != modified)
		m_FightPropertyStatus[XA_CUR_MP] = true;
	 
	return modified;
}

int CFightObject::ModifyCurrentTP(int mod, int type, bool curInterrupt)
{
	if (m_CurHp <= 0 || 0 == mod)
		return 0;

	int oldTP = (int)m_CurTp;

	int currentTP = 0;

	if (BAM_POT == type)
	{
		currentTP = oldTP + mod;
	}
	else if (BAM_PRECENT == type)
	{
		currentTP = oldTP + (int)((float)oldTP*(float)(mod / 100.0));
	}

	int modified = 0;
	if (currentTP > (int)m_MaxTp)
	{
		m_CurTp = m_MaxTp;
		modified = (int)m_MaxTp - oldTP;
	}
	else if (currentTP <= 0)
	{
		m_CurTp = 0;
		modified = oldTP;
	}
	else
	{
		m_CurTp = currentTP;
		modified = currentTP - oldTP;
	}

	if (0 != modified)
		m_FightPropertyStatus[XA_CUR_TP] = true;

	return modified;
}

int CFightObject::GetBuffModifyValue(WORD BuffType, int val)
{
	MY_ASSERT(BuffType >= BAP_MHP && BuffType < BAP_ATTRI_MAX);
	return m_BuffModifyPot[BuffType-1] + (int)(val * (float)(m_BuffModifyPercent[BuffType-1]/100.0f));
}

bool CFightObject::ChangeFightState(DWORD type, bool marked, DWORD pusherID, float fdestX, float fdestY,  DWORD dskillIndex)
{
	if (0 != type)
	{
		MY_ASSERT(type >= BAP_DIZZY && type < BAP_STATE_MAX);

		DWORD base = type - BAP_DIZZY;
		DWORD mask = 0x00000001 << base;

// 		if (type == BAP_JITUI)
// 		{
// 			rfalse(2, 1, "%s", marked ? "击退" : "击退状态解除");
// 		}
// 		else if (type == BAP_DIZZY)
// 		{
// 			rfalse(2, 1, "%s", marked ? "昏迷" : "昏迷状态解除");
// 		}
		
		if (marked)
			m_fightState |= mask;
		else
			m_fightState &= ~mask;
	}

	// 在玩家没有死亡的情况下，需要同步位置
	if ((type == BAP_DIZZY || type == BAP_HYPNOTISM) && 0 != m_CurHp)
	{
	//	Stand(EA_DIZZY);
// 		if (GetCurActionID() != EA_SKILL_ATTACK)
// 		{
// 			Stand();
// 		}
	}
	else if ((type == BAP_DINGSHENG) && (GetCurActionID() == EA_RUN || GetCurActionID() == EA_WALK))
	{ // [2012-5-18 16-25 gw: +如果定身状态下，而且是在移动的时候，我们需要先设置成站立状态]
	//	Stand(EA_DIZZY);
	}

	SASetExtraStateyMsg msg;
	msg.dwFightExtraState	= m_fightState;
	msg.dwGlobalID			= GetGID();
	msg.dwpushID = pusherID;
	msg.fdestX = fdestX;
	msg.fdestY = fdestY;
	msg.dskillid = dskillIndex;

	if (m_ParentArea)
		m_ParentArea->SendAdj(&msg, sizeof(msg), INVALID_DNID);

	return true;
}

bool CFightObject::CheckFightState(DWORD type)
{
// 	MY_ASSERT(type && (0 == (type & (type-1))));
// 	MY_ASSERT(type >= FS_DIZZY && type <= FS_JITUI);

	return m_fightState & type;
}

void CFightObject::GetExtraDamage(CFightObject *pDest)
{
	  
	return;
}

// 从lua得到string中读出元素值,存入数组中返回
void CFightObject::GetNunberFromString(string s, WORD ExtraDamageArr[])
{
	return;
}

void CFightObject::CalculateDamagePVE(CFightObject *pAtta, CFightObject *pDest, SDamage& damage)
{
	if (!pAtta || !pDest)
	{
		rfalse(4, 1, "Fightobject.cpp - CalculateDamagePVE() - !pAtta || !pDest");
		return;
	}

	// 计算伤害
	if (ConsumeFinalDamageValue(pAtta, pDest, damage))
	{
		int FinallyDamage = damage.wDamage;
		///侠客根据疲劳度减少伤害输出
		CFightPet *fpet=(CFightPet*)this->DynamicCast(IID_FIGHT_PET);
		if (fpet)
		{	
			int nDamage = FinallyDamage - fpet->GetDamageDecrease(FinallyDamage);
			FinallyDamage = nDamage>0?nDamage:FinallyDamage;
		}
		pDest->OnDamage(&FinallyDamage, this);
	}

	return;
}

void CFightObject::CalculateDamageEVP(CFightObject *pAtta, CFightObject *pDest, SDamage& damage)
{
	if (!pAtta || !pDest)
	{
		rfalse(4, 1, "Fightobject.cpp - CalculateDamageEVP() - !pAtta || !pDest");
		return;
	}

	// 计算伤害
	if (ConsumeFinalDamageValue(pAtta, pDest, damage))
	{
		int FinallyDamage = damage.wDamage;
		pDest->OnDamage(&FinallyDamage, this);
	}
	return;
}

void CFightObject::CalculateDamagePVP(CFightObject *pAtta, CFightObject *pDest, SDamage& damage)
{
	if (!pAtta || !pDest)
	{
		rfalse(4, 1, "Fightobject.cpp - CalculateDamagePVP() - !pAtta || !pDest");
		return;
	}

	///被攻击时检查一次PK规则是否产生伤害
	CPlayer *attackfight = (CPlayer *)pAtta->DynamicCast(IID_PLAYER);
	if (attackfight)
	{
		if (!attackfight->CheckPkRuleAttk(GetGID()))return;
	}

	// 计算伤害
	if (ConsumeFinalDamageValue(pAtta, pDest, damage))
	{
		int FinallyDamage = damage.wDamage;

		///侠客根据疲劳度减少伤害输出
		CFightPet *fpet=(CFightPet*)this->DynamicCast(IID_FIGHT_PET);
		if (fpet)
		{	
			int nDamage = FinallyDamage - fpet->GetDamageDecrease(FinallyDamage);
			FinallyDamage = nDamage>0?nDamage:FinallyDamage;
		}
		pDest->OnDamage(&FinallyDamage, this);
	}
	return;
}

INT32 CFightObject::InitWoundMsg_Normal(SAWoundObjectMsgEx* woundMsg)
{
	if (!woundMsg)
	{
		rfalse(4, 1, "Fightobject.cpp - InitWoundMsg_Mormal() - !woundMsg");
		return 0;
	}

	woundMsg->mWoundObjectNumber	= 1;
	woundMsg->mAttackerGID			= GetGID();
	//woundMsg->mSkillIndex			= (WORD)m_AtkContext.dwSkillIndex;
	woundMsg->mSkillIndex = (WORD)GetSkillIDBySkillIndex(m_AtkContext.dwSkillIndex);
	woundMsg->mMaxHP				= m_MaxHp;
	woundMsg->mCurHP				= m_CurHp;

	return 1;
}

INT32 CFightObject::InitWoundTiny(SWoundTiny *wound, SDamage& damage, INT32 hitState, float DefenserX, float DefenserY)
{
	if (!wound)
	{
		rfalse(4, 1, "Fightobject.cpp - InitWounedTiny() - !wound");
		return 0;
	}

	memset(wound, 0, sizeof(SWoundTiny));

	wound->mDefenserGID		= GetGID();
	wound->mDamage			= damage.wDamage;
	//memcpy(wound->mDamageExtra, damage.wDamageExtra, sizeof(WORD) * EXTRA_MAX);
// 	wound->mDamageExtra		= damage.wDamageExtra;
// 	wound->mIgnoreDefDamage = damage.wIgnoreDefDamage;
// 	wound->mRebound			= damage.wReboundDamage;
// 	wound->isCritical		= damage.isCritical;
	wound->mDefenserX = DefenserX;
	wound->mDefenserY = DefenserY;
	wound->mHitState		= hitState;
	wound->mMaxHp			= m_MaxHp;
	wound->mCurHp			= m_CurHp;

	return 1;
}

// 技能攻击伤害、普通以及麒麟臂伤害
INT32 CFightObject::SendDamageMessageBySkillAttack_Normal(CFightObject* pDest, SDamage& damage)
{
	if (!pDest)
	{
		rfalse(4, 1, "Fightobject.cpp - SendDamageMessageBySkillAttack_Normal() - !pDest");
		return 0;
	}

	SAWoundObjectMsgEx woundMsg;
	InitWoundMsg_Normal(&woundMsg);

	SWoundTiny *wound = (SWoundTiny *)woundMsg.streamData;
	if (m_vQuestskill.size()>0)
	{
		QuestSkill *pqskill = m_vQuestskill[0];   //单攻默认第一个目标
		if (pqskill)
		{
			MoveObiect_AttackMore(pDest, m_AtkContext.m_SkillData.m_SkillPropertyType, pqskill->mDefenderWorldPosX,pqskill->mDefenderWorldPosY);
		}
	}
	pDest->InitWoundTiny(wound, damage, damage.mHitState,pDest->m_curX,pDest->m_curY);

// 	if (SPT_CHARGE == m_AtkContext.m_SkillData.m_SkillPropertyType) //冲锋类技能,设置自身坐标点
// 	{
// 		wound->mDefenserX = m_AtkContext.m_SkillAttackPos.x;
// 		wound->mDefenserY = m_AtkContext.m_SkillAttackPos.y;
// 		MoveInArea(this, m_AtkContext.m_SkillAttackPos.x, m_AtkContext.m_SkillAttackPos.y);
// 	}
// 	else if (SPT_REPEL == m_AtkContext.m_SkillData.m_SkillPropertyType) //击退目标
// 	{
// 		wound->mDefenserX = m_AtkContext.mDefenderWorldPosX;
// 		wound->mDefenserY = m_AtkContext.mDefenderWorldPosY;
// 		DNID dnidExcept = -1;
// 		CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
// 		if (pPlayer)
// 		{
// 			dnidExcept = pPlayer->m_ClientIndex;
// 		}
// 		MoveInArea(pDest, m_AtkContext.mDefenderWorldPosX, m_AtkContext.mDefenderWorldPosY, dnidExcept);
// 	}
// 	else if (SPT_MOVE == m_AtkContext.m_SkillData.m_SkillPropertyType) //位移技能
// 	{
// 		MoveInArea(this, m_AtkContext.m_SkillAttackPos.x, m_AtkContext.m_SkillAttackPos.y);
// 		wound->mDefenserX = m_AtkContext.mDefenderWorldPosX;
// 		wound->mDefenserY = m_AtkContext.mDefenderWorldPosY;
// 		DNID dnidExcept = -1;
// 		CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
// 		if (pPlayer)
// 		{
// 			dnidExcept = pPlayer->m_ClientIndex;
// 		}
// 		MoveInArea(pDest, m_AtkContext.mDefenderWorldPosX, m_AtkContext.mDefenderWorldPosY, dnidExcept);
// 	}
	
	WORD size = offsetof(SAWoundObjectMsgEx, streamData) + sizeof(SWoundTiny) * woundMsg.mWoundObjectNumber;
	Synchro(&woundMsg,size);

	return 1;
}

// 重写的发送伤害值的消息，可以根据不同的枚举发送不同的消息 add by An
INT32 CFightObject::SendDamageMessageBySkillAttack_Normal(CFightObject* pDest, SDamage& damage, BYTE DamageType)
{
	 	if (!pDest)
	 	{
	 		rfalse(2, 1, "Fightobject.cpp - SendDamageMessageBySkillAttack_Normal(.., .., DamageType) - !pDest");
	 		return 0;
	 	}

	SAWoundObjectMsgEx woundMsg;
	InitWoundMsg_Normal(&woundMsg);

	SWoundTiny *wound = (SWoundTiny *)woundMsg.streamData;

	// 判断是否是麒麟臂伤害
	CPlayer* pPlayer = (CPlayer*)this->DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return 0;

	if (pPlayer)
	{
		rfalse("SendDamageMessageBySkillAttack_Normal %d",woundMsg.mSkillIndex);
	}

	// 重载的接口只是修改了这一条语句，增加的DamageType类型从函数外部直接传入，无需再函数内部赋值 add by An
	//pDest->InitWoundTiny(wound, damage, DamageType);

	WORD size = offsetof(SAWoundObjectMsgEx, streamData) + sizeof(SWoundTiny) * woundMsg.mWoundObjectNumber;
	Synchro(&woundMsg,size);

	return 1;
}

INT32 CFightObject::GetCurrentSkillLevel()
{
	return -1;
}

INT32 CFightObject::GetSkillIDBySkillIndex(INT32 index)
{
	return -1;
}

void CFightObject::CheckDelBuffSpot(DWORD delFlag)
{
	m_buffMgr.CheckDelBuffSpot(delFlag);
}

bool CFightObject::IsHit(CFightObject *att, CFightObject *def)
{
	if (!att || !def)
	{
		rfalse(4, 1, "Fightobject.cpp - IsHit() - !att || !def");
		return false;
	}
	int Flag = 0;
	lite::Variant ret;
	LuaFunctor(g_Script,"IsHit")[att->m_ShanBi][def->m_ShanBi](&ret);
	if (lite::Variant::VT_INTEGER == ret.dataType)
	{
		Flag = ret;
		if (Flag == 0)
		{

			return FALSE;
		}
		else
		{
			return true;
		}
	}
	return true;
	//float ShanBiRate = ConsumeShanBi(att->m_ShanBi, def->m_ShanBi);
	//if (CRandom::RandRange(1, 100) < (int)(ShanBiRate * 100))
	//{
	//	rfalse("你娃NB，没打着你~");//产生闪避
	//	return false;
	//}
	//return true;
	//	return true;
	// 	CMonster *pMonster = (CMonster *)def->DynamicCast(IID_MONSTER);
	// 	if (pMonster && pMonster->m_BackProtection)
	// 	{
	// 		rfalse(2, 1, "怪物处于保护状态，无法击中！！！");
	// 		return false;
	// 	}
	// 
	// 	float Atkhit = 0;			// 攻击方的命中
	// 	float DefEsc = 0;			// 防守方的躲避
	// 
	// 	// 物理攻击
	// 	if (m_AtkContext.m_SkillData.m_DamageCalcType == SDCT_PHY_NEAR || m_AtkContext.m_SkillData.m_DamageCalcType == SDCT_PHY_FAR)
	// 	{
	// 		if (0 == def->m_PhyEscape)
	// 			return true;
	// 
	// 		if (0 == m_PhyHit || 0 == m_PhyAtk)
	// 			return false;
	// 
	// 		Atkhit = m_PhyHit + (m_PhyHitFix/10000);
	// 		DefEsc = def->m_PhyEscape + (def->m_PhyEscapeFix/10000); 
	// 	}
	// 	else
	// 	{
	// 		if (0 == def->m_FpEscape)
	// 			return true;
	// 		
	// 		if (0 == m_FpHit || 0 == m_FpAtk)
	// 			return false;
	// 
	// 		Atkhit = m_FpHit + (m_FpHitFix/10000);
	// 		DefEsc = def->m_FpEscape + (def->m_FpEscapeFix/10000); 
	// 	}
	// 
	// 	WORD HitRate = 0;
	// 	WORD HitRand = 0;
	// 
	// 	if (Atkhit >= DefEsc)
	// 	{
	// 		HitRate = (WORD)((70 + 0.21 * pow(1.01, att->m_Level - def->m_Level + 3) * (Atkhit-DefEsc) / DefEsc*100.0));
	// 
	// 		if (HitRate >= 100)
	// 			HitRand = CRandom::RandRange(70, 98);
	// 		else if (HitRate > 30)
	// 			HitRand = HitRate;
	// 		else
	// 		{
	// 			MY_ASSERT(0 && "伤害随机小于30%");
	// 		}
	// 	}
	// 	else
	// 	{
	// 		HitRand = 30;
	// 	}
	// 
	// 	// 获取随机数
	// 	WORD tempRate = CRandom::RandRange(1, 100);
	// 
	// 	if (tempRate <= HitRand)
	// 		return true;
	// 	else
	// 		return false;
}

void CFightObject::DamageObject_Single_Bullet(Bullet *pBullet)
{
	if (!pBullet)
	{
		rfalse(4, 1, "fightobject.cpp - DamageObject_Single_Bullet() - !pBullet");
		return;
	}

	if (!pBullet->m_Target || !pBullet)		// 目标可能已经不再咯
		return;

	if (!pBullet->m_Sender)
	{
		rfalse(4, 1, "Fightobject.cpp - DamageObject_Single_Bullet() - !pBullet->m_sender");
		return;
	}

	if (this != pBullet->m_Sender)
	{
		rfalse(4, 1, "fightobject.cpp - DamageObject_Single_Bullet() - this != pBullet->m_Sender");
		return;
	}

	MY_ASSERT(pBullet->m_Sender && this == pBullet->m_Sender);

	CFightObject *pDest = pBullet->m_Target;

	if (!pDest)
	{
		rfalse(4, 1, "Fightobject.cpp - DamageOjbect_single_bullet() - !pDest");
		return;
	}

	pDest->CheckDelBuffSpot(BDT_ATTACKED);

	SDamage	damage;

	DamageEnemyBySkillAttack(pDest, damage);		// 计算伤害值

	SendDamageMessageBySkillAttack_Normal(pDest, damage);	// 发送战斗伤害信息

	pDest->CheckDelBuffSpot(BDT_DAMAGED);

	return;
}

void CFightObject::DamageObject_Single()
{
	// 首先得到被攻击的对象
	LPIObject pObj = m_ParentRegion->SearchObjectListInAreas(m_AtkContext.mDefenderGID, m_ParentArea->m_X, m_ParentArea->m_Y);
// 	if (!pObj || !pObj->isValid())
// 		return;

	if (pObj)
	{
		CFightObject *pDest = (CFightObject *)pObj->DynamicCast(IID_FIGHTOBJECT);
		if (!pDest || 0 == pDest->m_CurHp)
			return;

		// 计算伤害
		if (1 == m_AtkContext.m_CalcDamg[m_AtkContext.m_CurAtkNumber-1])
		{
			pDest->CheckDelBuffSpot(BDT_ATTACKED);

			SDamage	damage;
			DamageEnemyBySkillAttack(pDest, damage);	// 计算伤害值
			SendDamageMessageBySkillAttack_Normal(pDest, damage);	// 发送战斗消息
			pDest->CheckDelBuffSpot(BDT_DAMAGED);
		}
		// 如果有子弹，则释放子弹
		if (2 == m_AtkContext.m_CalcDamg[m_AtkContext.m_CurAtkNumber-1])
			BulletManager::GetInstance().FireBullet(m_AtkContext.m_SkillData.m_BulletID, this, pDest, m_AtkContext.m_SkillData.m_AtkType);

		m_AtkContext.m_TargetList.push_back(pDest);

		IntoFightState(pDest);
	}
	else  //没有目标仅仅进行位移
	{
		MoveObiect_AttackMore(0, m_AtkContext.m_SkillData.m_SkillPropertyType, m_AtkContext.mDefenderWorldPosX, m_AtkContext.mDefenderWorldPosY);
		//rfalse(2, 1, "MoveObiect_AttackMore %4.2f,%4.2f", m_AtkContext.mDefenderWorldPosX, m_AtkContext.mDefenderWorldPosY);
	}

	return;
}

void CFightObject::CheckStopBullet(WORD type)
{
	if (1 == type)			// 将自己发射的子弹强制失效
	{
		for (std::set<DWORD>::iterator it = m_FiredToOther.begin(); it != m_FiredToOther.end(); ++it)
			BulletManager::GetInstance().StopBullet(*it);

		m_FiredToOther.clear();
	}

	if (2 == type)			// 将对我发射的子弹强制失效
	{
		for (std::set<DWORD>::iterator it = m_FiredToMe.begin(); it != m_FiredToMe.end(); ++it)
			BulletManager::GetInstance().StopBullet(*it);

		m_FiredToMe.clear();
	}

	return;
}

static bool AlongLineByStartPos(INT32 startX,int startY,int checkX,int checkY,int dir,int len)
{
	for (int j = 0;j <= len;++j)
	{
		// 		UINT32 disX = abs(desObjectTileX - m_wCurX);
		// 		UINT32 disY = abs(desObjectTileY - m_wCurY);

		INT32 stepX = g_dir[ dir * 2 + 0 ];
		INT32 stepY = g_dir[ dir * 2 + 1 ];

		stepX *= j;
		stepY *= j;

		INT32 tempX = startX + stepX;
		INT32 tempY = startY + stepY;
		if (checkX == tempX && checkY == tempY)
		{
			return true;
		}
	}

	return false;
}

bool CFightObject::IsCanDamageByAttackMoreMethod(CFightObject *desObject)
{
	if (!desObject)
		return false;

	//// 如果闪避了，则直接返回
	//if (desObject->IfAvoidAttack(m_ShanBi, desObject->m_ShanBi))
	//	return false;

	CPlayer *player=(CPlayer*)DynamicCast(IID_PLAYER);
	if (player&&!player->CheckPkRuleAttk(desObject->GetGID()))
	{
		return false;
	}

	// 自身圆形群攻
	if (SAST_EMPTY_CIRCLE == m_AtkContext.m_SkillData.m_AtkType)
	{
		// 如果目标和玩家的距离在攻击半径之内，则条件满足
		float distance = sqrt(pow((m_curX-desObject->m_curX),2)+pow((m_curY-desObject->m_curY),2));
		if ((WORD)(distance) <= (m_AtkContext.m_SkillData.m_SkillAtkRadius+100))
			return true;
	}

	// 远程圆形群攻
	if (SAST_POINT_CIRCLE == m_AtkContext.m_SkillData.m_AtkType)
	{
		// 如果目标和攻击点距离在攻击半径之内，则条件满足
		float distance = sqrt(pow((m_AtkContext.mDefenderWorldPosX-desObject->m_curX),2)+pow((m_AtkContext.mDefenderWorldPosY-desObject->m_curY),2));
		return ((WORD)(distance) <= (m_AtkContext.m_SkillData.m_SkillAtkRadius+2));
	}

	// 自身扇形群攻 矩形
	if (SAST_EMPTY_SECTOR == m_AtkContext.m_SkillData.m_AtkType)
	{
	
		D3DXVECTOR2 vtargetPos(desObject->m_curX, desObject->m_curY);
		D3DXVECTOR2 vsrcPos(m_curX, m_curY);

		float fDistance = D3DXVec2Length(&(vtargetPos - vsrcPos));
		float fDisAngle = GetDirection(vsrcPos, vtargetPos);

		//rfalse(2, 1, "怪物攻击方向  dir = %f PlayerPos = %f,%f, 怪物与玩家方向 %f ", D3DXToDegree(m_Direction), desObject->m_curX, desObject->m_curY, D3DXToDegree(fDisAngle));
		if ((WORD)(fDistance) <= m_AtkContext.m_SkillData.m_SkillAtkRadius+100)
		{
			float fView = (m_AtkContext.m_SkillData.m_SkillAtkWidth / 180.0f) *D3DX_PI;
		//	rfalse("攻击范围 %f,%f", D3DXToDegree(m_Direction - fView * 0.5f), D3DXToDegree(m_Direction + fView * 0.5f) );
			if ((m_Direction - fView * 0.5f) < fDisAngle && (m_Direction + fView * 0.5f) > fDisAngle)
			{
				return true;
			}
				
		}
	}

	// 自身直线群攻
	if (SAST_EMPTY_POINTS == m_AtkContext.m_SkillData.m_AtkType)
	{
		float fDisX = desObject->m_curX - m_curX;
		float fDisY = desObject->m_curY - m_curY;

		float fDistance = sqrt(fDisX * fDisX + fDisY * fDisY);	
		float fTmpAngle = atan2(fDisY, fDisX) + 3.14159f / 2 - m_Direction;
		  
		float fTmpWidth  = fDistance * cosf(fTmpAngle);
		float fTmpHeight = fDistance * sinf(fTmpAngle);

		if (fTmpWidth > -1 * m_AtkContext.m_SkillData.m_SkillMaxDistance * 0.5f && fTmpWidth < m_AtkContext.m_SkillData.m_SkillMaxDistance * 0.5f &&
			fTmpHeight > 0 && fTmpHeight < m_AtkContext.m_SkillData.m_SkillAtkRadius)
			return true;

		return false;
	}

	return false;

	/*
	// 得到怪物的tile坐标
	INT32 desObjectTileX = desObject->m_curTileX;
	INT32 desObjectTileY = desObject->m_curTileY;
	desObjectTileY = desObjectTileY / 2;


	INT32 curTileX = m_wCurX;
	INT32 curTileY = m_wCurY / 2;

	// 得到当前的技能ID
	INT32 skillID = GetCurrentSkillID();

	// 得到攻击范围
	INT32 attackRange = CSkillDataTable::GetSkillData(skillID)->data[ CSkillDataTable::RangeType ];

	CSkillDataTable::SkillDataExtra* skillExtra = CSkillDataTable::GetSkillExtraData(attackRange - 1);
	if (!skillExtra)
	return false;


	INT32 lineWidth = skillExtra->data[ CSkillDataTable::Line_Width ];
	lineWidth = lineWidth - 1;

	// loop 8种线和2个圆的攻击方式
	for (int i = 0;i < CSkillDataTable::Circle_Radius_1;++i)
	{
	// 使用空心圆的攻击方式
	if(i > CSkillDataTable::Direct_7)
	{
	if (skillExtra->data[ i ] || skillExtra->data[ i + 1 ])
	{
	UINT32 disX = abs(desObjectTileX - curTileX);
	UINT32 disY = abs(desObjectTileY - curTileY);

	// 如果X或则y在2个半径之间，那么说明条件满足
	if (
	( disX >= (UINT32)skillExtra->data[ i ] && disX <= (UINT32)skillExtra->data[ i + 1] )
	&&
	( disY >= (UINT32)skillExtra->data[ i ] && disY <= (UINT32)skillExtra->data[ i + 1 ])
	)
	{
	return true;
	}
	}
	}
	// 使用线的攻击方式
	else
	{
	if (skillExtra->data[ i ] != 0)
	{
	// 逐步沿着线加step，判断怪物的tile坐标是否在这个线上，如果在那么条件满足
	INT32 curDir =  m_AtkContext.m_curDirection + i;
	curDir = curDir & 7; 


	// 根据直线的宽度，扩展相邻的2个方向
	static int offset[] = {-1,1};
	for (int x = 0;x < 2;x ++)
	{
	INT32 checkDir = curDir + offset[ x ];//(x - 1);
	checkDir = (checkDir + 8) & 7;
	for(int width = 1;width <= lineWidth;width ++)
	{
	INT32 newStartX = curTileX + g_dir[ checkDir * 2 + 0 ] * width;
	INT32 newStartY = curTileY + g_dir[ checkDir * 2 + 1 ] * width;

	bool ret = AlongLineByStartPos(newStartX,newStartY,desObjectTileX,desObjectTileY,curDir,skillExtra->data[ i ]);
	if (ret)
	{
	return true;
	}
	}
	}

	// middle 
	bool ret = AlongLineByStartPos(curTileX,curTileY,desObjectTileX,desObjectTileY,curDir,skillExtra->data[ i ]);
	if (ret)
	{
	return true;
	}
	}
	}

	}
	//*/
}

void CFightObject::GetAtkObjectByList(check_list<LPIObject> *objectList, std::list<CFightObject *> *destList, INT32 *pMonsterNumber, WORD atkType, INT32 MaxAttack, bool bischeckAttack)
{
	check_list<LPIObject>::iterator iter;
	INT32 monsterNumber = *pMonsterNumber;

	// 目前最多只能群体攻50个怪，也不能超过技能所要求的攻击数
	if (monsterNumber >= MaxAttack || monsterNumber >= SAWoundObjectMsgEx::MAX_ENEMY_NUMBER)
		return;

	if (objectList && !objectList->empty())
	{
		for (iter = objectList->begin(); iter != objectList->end(); ++iter)
		{
			LPIObject object = *iter;

			if (!object || !object->isValid())
				continue;

			CFightObject *fightObject = (CFightObject*)((object)->DynamicCast(IID_FIGHTOBJECT));
			if (!fightObject || fightObject->GetGID() == GetGID() || 0 == fightObject->m_CurHp)
				continue;

			// 检查是否满足群攻方式的攻击要求
			if (bischeckAttack)
			{
				if (IsCanDamageByAttackMoreMethod(fightObject))
				{
					destList->push_back(fightObject);
				}
			}
			else
			{
				destList->push_back(fightObject);
			}

			if (++monsterNumber >= MaxAttack)
				break;
		}
	}

	*pMonsterNumber = monsterNumber;
}

void CFightObject::DamageObject_AttackMore(bool self)
{
	INT32 startX, startY, endX, endY;

	//rfalse("DamageObject_AttackMore x = %f,y = %f", m_curX, m_curY);
	if (self)		// 以自身为原点，不可能有子弹
	{
		startX = m_ParentArea->m_X-1;
		startY = m_ParentArea->m_Y-1;
	}
	else			// 目标点为原点，可能释放子弹
	{
		WORD curAreaX = GetCurArea((DWORD)(m_AtkContext.mDefenderWorldPosX) >> TILE_BITW, _AreaW);
		WORD curAreaY = GetCurArea((DWORD)(m_AtkContext.mDefenderWorldPosY) >> TILE_BITH, _AreaH);
		startX = curAreaX-1;
		startY = curAreaY-1;
	}

	endX = startX+3;
	endY = startY+3;

	CMonster *pMonster = (CMonster*)DynamicCast(IID_MONSTER);
	if (pMonster)
	{
		DamageObject_AttackMoreForMonster(self);
		return;
	}

	// 获取攻击名单
	INT32 monsterNumber = 0;
	std::list<CFightObject *> Find_destList;
	std::list<QuestSkill *> destList;
	for (int x = startX; x < endX; x++)
	{
		for (int y = startY; y < endY; y++)
		{
			check_list<LPIObject> *monstList  = m_ParentRegion->GetMonsterListByAreaIndex(x,y);
			GetAtkObjectByList(monstList, &Find_destList, &monsterNumber, m_AtkContext.m_SkillData.m_AtkType, m_AtkContext.m_SkillData.m_AtkAmount);
		}
	}

	if (m_AtkContext.m_CurAtkNumber > 1)
	{
		if (Find_destList.size() > 0)
		{
			std::vector<QuestSkill*>::iterator iter = m_vQuestskill.begin();
			std::list<CFightObject *>::iterator iterdest = Find_destList.begin();
			bool bisquest = false;
			for (iterdest = Find_destList.begin(); iterdest != Find_destList.end(); iterdest++)
			{
				if (m_vQuestskill.size() > 0)
				{
					for (iter = m_vQuestskill.begin(); iter != m_vQuestskill.end(); iter++)
					{
						QuestSkill* pqskill = *iter;
						if (pqskill)
						{
							LPIObject pObj = m_ParentRegion->SearchObjectListInAreas(pqskill->mDefenderGID, m_ParentArea->m_X, m_ParentArea->m_Y);
							if (pObj&&pObj->isValid())
							{
								CFightObject *pDest = (CFightObject *)pObj->DynamicCast(IID_FIGHTOBJECT);
								if (pDest&&pDest->m_CurHp > 0)
								{
									if (pDest->GetGID() == (*iterdest)->GetGID())
									{
										bisquest = true;
										break;
									}
								}
							}
						}
					}
				}
				if (!bisquest)
				{
					QuestSkill *pqskill = new QuestSkill;
					memset(pqskill, 0, sizeof(QuestSkill));
					pqskill->mDefenderGID = (*iterdest)->GetGID();
					pqskill->mDefenderWorldPosX = (*iterdest)->m_curX;
					pqskill->mDefenderWorldPosY = (*iterdest)->m_curY;
					m_vQuestskill.push_back(pqskill);
				}
			}
		}
	}


		if (m_AtkContext.m_CalcDamg[m_AtkContext.m_CurAtkNumber - 1])
		{
			SAWoundObjectMsgEx woundMsg;
			woundMsg.mAttackerGID = GetGID();
			//woundMsg.mSkillIndex		= (WORD)m_AtkContext.dwSkillIndex;
			woundMsg.mSkillIndex = GetSkillIDBySkillIndex(m_AtkContext.dwSkillIndex);
			woundMsg.mMaxHP = m_MaxHp;
			woundMsg.mCurHP = m_CurHp;

			size_t monsterIndex = 0;

			bool battack = false;
			std::vector<QuestSkill*>::iterator iter = m_vQuestskill.begin();
			while (iter != m_vQuestskill.end())
			{
				QuestSkill* pqskill = *iter;
				if (pqskill)
				{
					LPIObject pObj = m_ParentRegion->SearchObjectListInAreas(pqskill->mDefenderGID, m_ParentArea->m_X, m_ParentArea->m_Y);
					if (pObj&&pObj->isValid())
					{
						CFightObject *pDest = (CFightObject *)pObj->DynamicCast(IID_FIGHTOBJECT);
						if (pDest && pDest->m_CurHp > 0)
						{
							D3DXVECTOR2 vDest(pDest->m_curX, pDest->m_curY);
							D3DXVECTOR2 vSelf(m_curX, m_curY);
							float lenth = D3DXVec2Length(&(vDest - vSelf));
							if (m_AtkContext.m_SkillData.m_SkillAtkRadius + 300 > lenth) //更改技能攻击修正距离为300，因为在位移技能的同时可能坐标误差会增大
							{
								MoveObiect_AttackMore(pDest, m_AtkContext.m_SkillData.m_SkillPropertyType, pqskill->mDefenderWorldPosX, pqskill->mDefenderWorldPosY);
								// 检查在被攻击时要强制中断的Buff
								pDest->CheckDelBuffSpot(BDT_ATTACKED);
								SDamage damage;
								memset(&damage, 0, sizeof(SDamage));
								SWoundTiny *wound = (((SWoundTiny *)&woundMsg.streamData)) + monsterIndex;

								vDest.x = pDest->m_curX;
								vDest.y = pDest->m_curY;
								vSelf.x = m_curX;
								vSelf.y = m_curY;
								lenth = D3DXVec2Length(&(vDest - vSelf));      //使用了位移技能以后可能坐标已经改变这里重新计算一次
								if (m_AtkContext.m_SkillData.m_SkillAtkRadius + 300 > lenth) //更改技能攻击修正距离为300，因为在位移技能的同时可能坐标误差会增大
								{
									// 这里是技能攻击伤害计算
									DamageEnemyBySkillAttack(pDest, damage);
									// 检查在受伤害时要强制中断的Buff
									pDest->CheckDelBuffSpot(BDT_DAMAGED);
									pDest->InitWoundTiny(wound, damage, damage.mHitState, pqskill->mDefenderWorldPosX, pqskill->mDefenderWorldPosY);
									monsterIndex++;
								}
						
								m_AtkContext.m_TargetList.push_back(pDest);
								IntoFightState(pDest);
								battack = true;
							}
						}
					}
				}

				iter++;
			}

			if (!battack) //更新坐标
			{
				if (SPT_CHARGE == m_AtkContext.m_SkillData.m_SkillPropertyType) //冲锋类技能,设置自身坐标点
				{
					//MoveInArea(this, m_AtkContext.mDefenderWorldPosX, m_AtkContext.mDefenderWorldPosY);
					SASynPosMsg	msg;
					msg.m_GID = this->GetGID();
					msg.m_Action = EA_STAND;
					msg.m_X = m_AtkContext.mDefenderWorldPosX;
					msg.m_Y = m_AtkContext.mDefenderWorldPosY;
					msg.m_Z = m_curZ;

					CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
					if (pArea)
					{
						pArea->SendAdj(&msg, sizeof(SASynPosMsg), -1);
					}
				}
			}

			//MY_ASSERT(monsterIndex == monsterNumber);
			woundMsg.mWoundObjectNumber = monsterIndex;
			WORD size = offsetof(SAWoundObjectMsgEx, streamData) + sizeof(SWoundTiny)* woundMsg.mWoundObjectNumber;
			if (monsterIndex)
				Synchro(&woundMsg, size);
		}

		if (!self && 2 == m_AtkContext.m_CalcDamg[m_AtkContext.m_CurAtkNumber - 1])
			BulletManager::GetInstance().FireBullet(m_AtkContext.m_SkillData.m_BulletID, this, 0, m_AtkContext.m_SkillData.m_AtkType);

		m_AtkContext.m_TargetList.sort();
		m_AtkContext.m_TargetList.unique();
	return;
}

WORD CFightObject::IsTelegryExisted(CPlayer *pPlayer, WORD wID)
{
	if (0 == wID)
		return -1;

	for (int i = 0; i < MAX_TELEGRYNUM; i++)
	{
		if (pPlayer->m_Property.m_Xyd3Telergy[i].m_TelergyID == wID)
		{
			MY_ASSERT(pPlayer->m_Property.m_Xyd3Telergy[i].m_TelergyLevel > 0 && pPlayer->m_Property.m_Xyd3Telergy[i].m_TelergyLevel <= MaxXinFaLevel);
			return i;
		}
	}

	return -1;
}

INT32 CFightObject::GetFreeTelergySlot(CPlayer* pPlayer, INT32 startSlot)
{
	for (int i = startSlot; i < MAX_TELEGRYNUM; i++)
	{
		if (0 == pPlayer->m_Property.m_Xyd3Telergy[i].m_TelergyID && 0 == pPlayer->m_Property.m_Xyd3Telergy[i].m_TelergyLevel)
			return i;
	}

	return -1;
}

bool CFightObject::IsSkillExisted(CPlayer *pPlayer, WORD wSkillID)
{
	for (int i = 0; i < MAX_SKILLCOUNT; i++)
	{
		if (pPlayer->m_Property.m_pSkills[i].wTypeID == wSkillID && pPlayer->m_Property.m_pSkills[i].byLevel > 0)
			return true;
	}

	return false;
}

INT32 CFightObject::GetFreeSkillSlot(CPlayer* pPlayer, INT32 startSlot)
{
	for (int i = startSlot; i < MAX_SKILLCOUNT; i++)
	{
		if (0 == pPlayer->m_Property.m_pSkills[i].wTypeID && 0 == pPlayer->m_Property.m_pSkills[i].byLevel)
			return i;
	}

	return -1;
}

BOOL CFightObject::ActivaSkill(WORD wSkillID, LPCSTR info)
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(wSkillID);
	if (!pData)
		return FALSE;

	if (IsSkillExisted(pPlayer, wSkillID))
		return TalkToDnid(pPlayer->m_ClientIndex, "您已经学会了该技能!"), FALSE;

	INT32 freeSlot = GetFreeSkillSlot(pPlayer, 0);	
	if (-1 == freeSlot)
		return FALSE;

	//SSkill TempSkill;
	//TempSkill.wTypeID = wSkillID;
	//TempSkill.byLevel = 1;
	//TempSkill.dwProficiency = 0;
	//INT32 i = freeSlot - 1;
	//for (; i >= 0; i--)
	//{
	//	if (wSkillID < pPlayer->m_Property.m_pSkills[i].wTypeID)
	//		memcpy(&pPlayer->m_Property.m_pSkills[i + 1], &pPlayer->m_Property.m_pSkills[i], sizeof(SSkill));
	//	else
	//		break;
	//}
	//memcpy(&pPlayer->m_Property.m_pSkills[i + 1], &TempSkill, sizeof(SSkill));

	pPlayer->m_Property.m_pSkills[freeSlot].wTypeID			= wSkillID;
	pPlayer->m_Property.m_pSkills[freeSlot].byLevel			= 1;
	pPlayer->m_Property.m_pSkills[freeSlot].dwProficiency	= 0;

	pPlayer->SendUpdataSkill(freeSlot);

	// 如果激活了被动技能，则附加被动Buff
	if (SKT_PASSITIVE == pData->m_SkillType)
		LuaFunctor(g_Script, FormatString("AttachPasstiveSkillBuff%d", pData->m_ID))[1]();

	return TRUE;
}

short CFightObject::ActiveTempSkill(WORD wSkillID, BYTE Level)
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return -1;

	const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(wSkillID);
	if (!pData)
		return -1;

	INT32 freeSlot = GetFreeSkillSlot(pPlayer, 0);	
	if (-1 == freeSlot)
		return -1;

	pPlayer->m_Property.m_pSkills[freeSlot].wTypeID			= wSkillID;
	pPlayer->m_Property.m_pSkills[freeSlot].byLevel			= Level;
	pPlayer->m_Property.m_pSkills[freeSlot].dwProficiency	= 0;
	{
		//发送消息
		//pPlayer->SendTemplateSkill(freeSlot,1);
		TemplateSkillInfo temp;
		temp.SkillIndex = freeSlot;
		temp.SkillID = wSkillID;
		pPlayer->m_TemplateSkillInfo.push_back(temp);
	}
	pPlayer->SendUpdataSkill(freeSlot);
	return freeSlot;
}

void CFightObject::CoolingTempSkill(WORD index)
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;

	if (pPlayer->m_TemplateSkillInfo.size() <= 0)
	{
		return;
	}

	SAPlayerTemplateSkillmsg msg;
	msg.Flag = 1;
	msg.m_num = pPlayer->m_TemplateSkillInfo.size();

	for (int i = 0; i < pPlayer->m_TemplateSkillInfo.size();++i)
	{
		BYTE index = pPlayer->m_TemplateSkillInfo[i].SkillIndex;
		long skillid = pPlayer->m_TemplateSkillInfo[i].SkillID;
		pPlayer->m_Property.m_pSkills[index].wTypeID			= 0;
		pPlayer->m_Property.m_pSkills[index].byLevel			= 0;
		pPlayer->m_Property.m_pSkills[index].dwProficiency		= 0;
		msg.temp[i] = pPlayer->m_TemplateSkillInfo[i];
		pPlayer->SendUpdataSkill(index);
	}

	//g_StoreMessage(pPlayer->m_ClientIndex,&msg,sizeof(SAPlayerTemplateSkillmsg));

	if (pPlayer->m_TemplateSkillInfo.size() > 0)
	{
		pPlayer->m_TemplateSkillInfo.clear();
	}

}

INT32 CFightObject::GetSkillSlot(CPlayer* pPlayer, WORD wSkillID)
{
	if (!pPlayer)
	{
		rfalse(4, 1, "Fightobject.cpp - GetSkillSlots() - !pPlayer");
		return -1;
	}

	for (int i = 0; i < MAX_SKILLCOUNT; i++)
	{
		if (wSkillID == pPlayer->m_Property.m_pSkills[i].wTypeID && pPlayer->m_Property.m_pSkills[i].byLevel > 0)
			return i;
	}

	return -1;
}

bool CFightObject::ConsumeHP(int consume)
{
	return true;
}

bool CFightObject::ConsumeMP(int consume)
{
	return true;
}

bool CFightObject::ConsumeTP(int consume)
{
	return true;
}

// 通知改变状态
void CFightObject::NotifyExchange( )
{
	std::list<CFightListener *>::iterator i = m_listenerList.begin();

	for (i; i != m_listenerList.end(); i++)
	{
		if (0 == *i)
		{
			rfalse(4, 1, "fightobject.cpp - NotifyExchange() - *i == 0");
			continue;
		}

		(*i)->OnExchange(this);
	}
}

int g_dir[  ] = {
	0,-1,
	1,-1,
	1,0,
	1,1,
	0,1,
	-1,1,
	-1,0,
	-1,-1
};

void CFightObject::SetListener(CFightListener *pListener)
{
	if (pListener)
	{ 
		pListener->SetSubject(this); 
		m_listenerList.push_back(pListener);
	} 
}

void CFightObject::RemoveListener(CFightListener *pListener)
{
	if (!pListener)
	{
		rfalse(4, 1, "Fightobject.cpp - RemoveListenner() - !pListhener");
		return;
	}

	std::list<CFightListener *>::iterator i = std::find(m_listenerList.begin(), m_listenerList.end(), pListener);
	if (i!= m_listenerList.end())
	{
		delete *i;
		m_listenerList.erase(i);
	}
}

void CFightObject::RemoveAllListener()
{
	for (std::list<CFightListener *>::iterator it = m_listenerList.begin(); it != m_listenerList.end(); ++it)
	{
		if (0 == *it)
		{
			rfalse(4, 1, "fightobject.cpp - NotifyExchange() - *it == 0");
			continue;
		}

		(*it)->OnClose(this);
		delete *it;
	}

	m_listenerList.clear();
}

void CFightObject::SendAddFightPetExp( DWORD dwExpVal, BYTE byChangeType, LPCSTR how /*= NULL*/ )
{
	if (0 == m_CurHp || 0 == dwExpVal)
		return;

	CFightPet *pFightpet = (CFightPet *)DynamicCast(IID_FIGHT_PET);
	if (!pFightpet)
		return;

	// 如果满级，则不加经验了
	if (MAXPLAYERLEVEL == pFightpet->m_Level)
		return;

	QWORD remainExp = 0xffffffffffffffff - pFightpet->m_nCurExp;

	pFightpet->m_nCurExp += (dwExpVal > remainExp ? remainExp : dwExpVal);

	//QWORD needExp =CFightPetService::GetInstance().GetFightPetLevelExp(pFightpet->m_Level);
	QWORD needExp = CPlayerService::GetInstance().GetPlayerUpdateExp(pFightpet->m_Level)/2; //修改为玩家相应等级经验值一半

	bool blevelup = true;

	//如果侠客的等级比主人高5级，侠客不会升级。
	if (pFightpet->m_Level - pFightpet->m_owner->m_Level > 4)
	{
		//最高累计升级上限值的2倍
		pFightpet->m_nCurExp= pFightpet->m_nCurExp  > needExp *2 ? needExp *2 : pFightpet->m_nCurExp ;
		blevelup = false;
	}

	//发送侠客升级以及经验消息
	SAFightPetLevelChange levelMsg;

	levelMsg.bLevel	=	pFightpet->m_Level;
	levelMsg.m_index=pFightpet->m_index;
	levelMsg.m_curExp=pFightpet->m_nCurExp;
	levelMsg.m_MaxExp=pFightpet->m_nMaxExp;
	g_StoreMessage(pFightpet->m_owner->m_ClientIndex, &levelMsg, sizeof(SAFightPetLevelChange));

	// 发送侠客获取经验消息
	// 	SAFightPetExpChange ExpMsg;
	// 	ExpMsg.m_index=pFightpet->m_index;
	// 	ExpMsg.qwCurGain		= (dwExpVal > remainExp ? remainExp : dwExpVal);
	// 	ExpMsg.byExpChangeType  = byChangeType;
	// 	g_StoreMessage(pFightpet->m_owner->m_ClientIndex, &ExpMsg, sizeof(SAFightPetExpChange));

	if (blevelup)
	{
		// 在此判断是否可以升级
		pFightpet->OnRecvLevelUp(0);
	}

	//pPlayer->m_PlayerPropertyStatus[XA_CUR_EXP-XA_MAX_EXP] = true;

	return;
}

float CFightObject::GetSkillAttkFactor(SSkillBaseData *skill)
{
	if (!skill)
	{
		rfalse(4, 1, "Fightobject.cpp - GetSkillAttkFactor() - !skill");
		return 1;
	}

	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
	{
		return 1;
	}

	INT32 skillLevel = pPlayer->GetCurrentSkillLevel(m_AtkContext.dwSkillIndex);

	if (-1 == skillLevel || 0 == skillLevel)
	{
		return 1;
	}
	//// 攻击系数
	float tempFactor = 0.0f;
	{
		double value = 0;
		g_Script.CallFunc("SkillAddAttackCalculate",skillLevel,
			m_AtkContext.m_SkillData.m_SkillPropertyType,m_AtkContext.m_SkillData.m_ID);
		if (g_Script.GetRet(0).GetType() != CScriptValue::VT_NIL)
		{
			tempFactor = (float)g_Script.GetRet(0);
			return tempFactor;
		}
		else
		{
			return 1;
		}
		//lite::Variant ret;
		//LuaFunctor(g_Script,"SkillAddAttackCalculate")[skillLevel][m_AtkContext.m_SkillData.m_SkillPropertyType][m_AtkContext.m_SkillData.m_ID](&ret);
		/*if (lite::Variant::VT_DOUBLE == ret.dataType)
		{
		value = ret;
		tempFactor = value;

		}
		else
		{
		return 1;
		}*/

	}
	//// 攻击系数
	//float tempFactor = 0.0f;
	//switch (m_AtkContext.m_SkillData.m_SkillPropertyType)
	//{
	//case SPT_NEARBY:		// 近身攻击
	//	tempFactor = skillLevel * 0.01f;
	//	//TalkToDnid(pPlayer->m_ClientIndex, FormatString("近身攻击：当前的等级 %d,技能系数 %f", skillLevel, tempFactor));
	//	break;

	//case SPT_LONGDISTANCE:	// 远程攻击
	//	tempFactor = skillLevel * 0.01f * 0.85f;
	//	//TalkToDnid(pPlayer->m_ClientIndex, FormatString("远程攻击：当前的等级 %d,技能系数 %f", skillLevel, tempFactor));
	//	break;

	//case SPT_RANGE:			// 范围攻击
	//	tempFactor = skillLevel * 0.01f * 0.7f;
	//	//TalkToDnid(pPlayer->m_ClientIndex, FormatString("范围攻击：当前的等级 %d,技能系数 %f", skillLevel, tempFactor));
	//	break;
	//default:
	//	return 1;
	//}

	return tempFactor + 1;
}

bool CFightObject::AddToHatelist( CFightObject *pFighter )
{
	if (pFighter)
	{
		CPlayer *player=(CPlayer*)pFighter->DynamicCast(IID_PLAYER);
		if(!player)return false;

		std::map<DWORD,SHateList>::iterator ithate=hatelist.find(pFighter->GetGID());
		if (ithate!=hatelist.end())
		{
			ithate->second.m_nAtkTime=timeGetTime();
			ithate->second.m_nHateValue++;
			return true;
		}	
		hatelist[pFighter->GetGID()].m_nAtkTime=timeGetTime();
		hatelist[pFighter->GetGID()].m_nHateValue++;
		return true;
	}

	return false;
}

void CFightObject::RemoveFromHatelistOnTime()
{
	if (hatelist.size()==0)return;

	std::map<DWORD,SHateList>::iterator ithate=hatelist.begin();
	while (ithate!=hatelist.end())
	{
		DWORD timevalue=timeGetTime()-ithate->second.m_nAtkTime;
		if (timevalue > 30 * 1000)
		{
			hatelist.erase(ithate);
			return;
		}

		ithate++;
	}

}

void CFightObject::RemoveAllHatelist()
{
	if (hatelist.size()==0)return;
	hatelist.clear();
}

void CFightObject::RemoveHatelistFromID( DWORD gid )
{
	if (hatelist.size()==0||gid==0)return;
	std::map<DWORD,SHateList>::iterator ithate=hatelist.find(gid);
	if (ithate!=hatelist.end()){
		hatelist.erase(ithate);
	}
}

bool CFightObject::IsInHatelist( DWORD gid )
{
	if (hatelist.size()==0||gid==0)
		return false;

	std::map<DWORD,SHateList>::iterator ithate=hatelist.find(gid);
	return ithate!=hatelist.end();
}

const char* CFightObject::getname()
{
	if (GetIID()==IID_PLAYER)
	{
		CPlayer *player=(CPlayer*)DynamicCast(IID_PLAYER);
		if (player)
		{
			return player->GetName();
		}
	}
	else if(GetIID()==IID_FIGHT_PET)
	{
		CFightPet *pet=(CFightPet*)DynamicCast(IID_FIGHT_PET);
		if (pet)
		{
			return pet->getname();
		}
	}
	else if(GetIID()==IID_MONSTER)
	{
		CMonster *monster=(CMonster*)DynamicCast(IID_MONSTER);
		if (monster)
		{
			return monster->m_Property.m_Name;
		}
	}

	return "";
}

BOOL CFightObject::IfAvoidAttack(WORD MyShanbi, WORD OtherShanBi)
{
	float ShanBiRate = ConsumeShanBi(MyShanbi, OtherShanBi);
	if (CRandom::RandRange(1, 100) < (int)(ShanBiRate * 100))
	{
		SendAttackMissMsg(SWoundTiny::Not_Hit);
		rfalse("你娃NB，没打着你~");
		return TRUE;
	}

	return FALSE;
}

float CFightObject::ConsumeBaoJi(WORD MyBaoJi, WORD OtherBaoJi)
{
	// 计算暴击率
	float MyCriticalRB		= 0.0f;
	float OtherCriticalRB	= 0.0f;
	float CriticalRB		= 0.0f;

	MyCriticalRB	= MyBaoJi / 10000;
	OtherCriticalRB	= OtherBaoJi / 10000;

	if (MyCriticalRB > OtherCriticalRB)
	{
		CriticalRB = MyCriticalRB - OtherCriticalRB;
	}
	else if (MyCriticalRB < OtherCriticalRB)
	{
		if (MyCriticalRB < OtherCriticalRB * 0.5f)
		{
			CriticalRB = 0.0f;
		}
		else
			CriticalRB = (1 - (OtherCriticalRB - MyCriticalRB) / MyCriticalRB) * (OtherCriticalRB - MyCriticalRB);
	}
	else
		CriticalRB = MyCriticalRB + 0.01f;

	return CriticalRB;
}

float CFightObject::ConsumeShanBi(WORD MyShanBi, WORD OtherShanBi)
{
	// 计算闪避绿
	float MyShanBiRate		= 0.0f;
	float OtherShanBiRate	= 0.0f;
	float ShanBiRate		= 0.0f;

	MyShanBiRate	= (float)MyShanBi / 12000;
	OtherShanBiRate	= (float)OtherShanBi / 12000;

	if (MyShanBiRate > OtherShanBiRate)
	{
		ShanBiRate = MyShanBiRate - OtherShanBiRate;
	}
	else if (MyShanBiRate < OtherShanBiRate)
	{
		if (MyShanBiRate < OtherShanBiRate * 0.5f)
		{
			ShanBiRate = 0.0f;
		}
		else
			ShanBiRate = (1 - (OtherShanBiRate - MyShanBiRate) / MyShanBiRate) * (OtherShanBiRate - MyShanBiRate);
	}
	else
		ShanBiRate = MyShanBiRate + 0.01f;

	return ShanBiRate;
}

// 等级差计算
float CFightObject::DeltaLevelRate(WORD highLevel, WORD lowLevel)
{
	MY_ASSERT(highLevel >= lowLevel);

	float levelRate = 0.0f;

	if (highLevel - lowLevel < 10 && highLevel > lowLevel)
	{
		levelRate = (11 - (highLevel - lowLevel)) * 0.01f;
	}

	return levelRate;
}

// 攻击伤害公式接口
float CFightObject::ConsumeFinalDamageValue(CFightObject *pAtta, CFightObject *pDest)
{
	if (!pAtta || !pDest)
	{
		rfalse(4, 1, "Fightobject.cpp - ConsumeFinalDamageValue() - !pAtta || !pDest");

		return 0.0f;
	}
	
	lite::lua_variant vreslut;
	BOOL execResult =FALSE;
	if (g_Script.PrepareFunction("ConsumeDamage"))
	{
		g_Script.PushParameter(pAtta->GetGID());
		g_Script.PushParameter(pDest->GetGID());
		g_Script.PushParameter(GetSkillIDBySkillIndex(m_AtkContext.dwSkillIndex));
		execResult = g_Script.Execute(&vreslut);
	}
	if (!execResult)
	{
		rfalse(2,1,"ConsumeFinalDamageValue Faile");
		return 0;
	}

	if (vreslut.dataType == LUA_TNIL )
	{
		rfalse(2,1,"ConsumeFinalDamageValue Faile");
		return 0;
	}
	int damage =0;
	try
	{
		damage = vreslut;
	}
	catch ( lite::Xcpt &e )
	{
		rfalse(2,1,e.GetErrInfo());
		return 0;
	}
	return damage;
}

BOOL CFightObject::ConsumeFinalDamageValue(CFightObject *pAtta, CFightObject *pDest, SDamage &sdamage)
{
	if (!pAtta || !pDest)
	{
		rfalse(4, 1, "Fightobject.cpp - ConsumeFinalDamageValue() - !pAtta || !pDest");

		return 0.0f;
	}

	if (g_Script.GetDamageValue("ConsumeDamage", pAtta->GetGID(), pDest->GetGID(), GetSkillIDBySkillIndex(m_AtkContext.dwSkillIndex), sdamage))
	{
		return TRUE;
	}
	return FALSE;
// 	lite::lua_variant vreslut;
// 	BOOL execResult = FALSE;
// 	if (g_Script.PrepareFunction("ConsumeDamage"))
// 	{
// 		g_Script.PushParameter(pAtta->GetGID());
// 		g_Script.PushParameter(pDest->GetGID());
// 		g_Script.PushParameter(GetSkillIDBySkillIndex(m_AtkContext.dwSkillIndex));
// 		execResult = g_Script.Execute(&vreslut);
// 	}
// 	if (!execResult)
// 	{
// 		rfalse(2, 1, "ConsumeFinalDamageValue Faile");
// 		return 0;
// 	}
}

bool CFightObject::_fpGetSkillEnableLearn( WORD wSkillID,BYTE index )
{
	CPlayer *player = (CPlayer*)this->DynamicCast(IID_PLAYER);
	if(!player)
		return false;

	const SSkillBaseData *pData =CSkillService::GetInstance().GetSkillBaseData(wSkillID);
	if (!pData)
		return false;

	SFightPetExt *pet = player->_fpGetFightPetByIndex(index);

	return pet ? pData->m_School == SS_XIAKE : false;
}

INT32 CFightObject::_fpGetFreeSkillSlot( BYTE index )
{
	CPlayer *player = (CPlayer*)this->DynamicCast(IID_PLAYER);
	if(!player)
		return false;

	SFightPetExt *pet = player->_fpGetFightPetByIndex(index);
	if(!pet)
		return false;

	for (int i = 0; i < pet->m_fpActivedSkill; i++)
	{
		if (pet->m_fpSkill[i].wTypeID == 0 && pet->m_fpSkill[i] .byLevel == 0)
			return i;
	}

	return -1;
}

BOOL CFightObject::_fpActivaSkill( WORD wSkillID,BYTE bskillLevel,BYTE index,LPCSTR info,BYTE type /*= SQfpSkillUpdate::SSU_LEARNED*/ )
{
	CPlayer *player = (CPlayer*)this->DynamicCast(IID_PLAYER);
	if(!player)
		return false;

	SFightPetExt *pet = player->_fpGetFightPetByIndex(index);
	if(!pet)
		return false;

	if(!_fpGetSkillEnableLearn(wSkillID,index))
		return FALSE;

	//得到技能孔
	int SkillSlot = 0;
	bool isskillupdata = false;
	if (bskillLevel>1)
	{
		//判断是否已经学过技能
		SkillSlot= player->_fpGetSkillSlot(wSkillID,index);
		if(SkillSlot<0)
		{
			TalkToDnid(player->m_ClientIndex,"你还没有学会上一个技能哈！");
			return FALSE;
		}

		//得到当前已经学习的技能等级
		int currentskilllevel = pet->m_fpSkill[SkillSlot].byLevel;
		if (bskillLevel<currentskilllevel)
		{
			TalkToDnid(player->m_ClientIndex,"你已经学会了技能！");
			return FALSE;
		}

		if (bskillLevel>currentskilllevel+1)
		{
			TalkToDnid(player->m_ClientIndex,"你必须先学会上一级技能！");
			return FALSE;
		}
		isskillupdata = true;
	}
	else SkillSlot = _fpGetFreeSkillSlot(index);//没有空的技能孔已经学满
	if(SkillSlot<0)
	{
		TalkToDnid(player->m_ClientIndex,"侠客技能栏已满，不能学习新的技能！");
		return FALSE;
	}

	int randlearn =CRandom::RandRange(0,100);
	///如果是通过技能书学习的用悟性来判断
	if (type == SQfpSkillUpdate::SSU_LEARNED)
	{
		//取随机值和悟性看能否学会技能,现在测试改为100%学会
		// 		if(pet->m_fpWuXing<randlearn)
		// 		{
		// 			TalkToDnid(player->m_ClientIndex,"没能学会技能");
		// 			return FALSE;
		// 		}
	}
	else if(type == SQfpSkillUpdate::SSU_LEARNEDBYANOTHER)
	{//否则用通过表判断，侠客学个技能还分多个情况，麻烦 
		int chance = CSkillService::GetInstance()._fpGetSkillupgradeChance(bskillLevel);
		if(chance < randlearn)
		{
			TalkToDnid(player->m_ClientIndex,"没能学会技能");
			return FALSE;
		}
	}

	//保存学习的技能
	pet->m_fpSkill[SkillSlot].wTypeID =wSkillID;
	pet->m_fpSkill[SkillSlot].byLevel =bskillLevel;

	///发给客户端
	SAfpSkillUpdate sfpskillmsg;
	sfpskillmsg.byWhat = isskillupdata ? SAfpSkillUpdate::SSU_LEVELUP :  SAfpSkillUpdate::SSU_LEARNED;
	sfpskillmsg.wPos = SkillSlot;
	sfpskillmsg.index = index;
	sfpskillmsg.spSkill = pet->m_fpSkill[SkillSlot];
	g_StoreMessage(player->m_ClientIndex,&sfpskillmsg,sizeof(SAfpSkillUpdate));
	return TRUE;
}

BOOL CFightObject::_fpActivaSkillByItem( DWORD ditemID,BYTE index,LPCSTR info,BYTE type /*= SQfpSkillUpdate::SSU_LEARNED*/ )
{
	CPlayer *player = (CPlayer*)this->DynamicCast(IID_PLAYER);
	if(!player)
		return false;

	SFightPetExt *pet = player->_fpGetFightPetByIndex(index);
	if(!pet)
		return false;

	//判断当前技能等级是学习还是升级技能
	int skillid = player->lua_fpGetItemToSkillID(ditemID);
	int blevel = player->lua_fpGetItemToSkillLevel(ditemID);
	if(skillid == 0 )return FALSE;
	if(!_fpGetSkillEnableLearn(skillid,index))return FALSE;

	return _fpActivaSkill(skillid,blevel,index,"学习技能！",type);
}

BYTE CFightObject::IsPlayerOrFightPet( CFightObject *pFighter )
{
	MY_ASSERT(pFighter);
	CPlayer *player = (CPlayer*)pFighter->DynamicCast(IID_PLAYER);
	if(player)
		return 1;

	CFightPet *pPet = (CFightPet*)pFighter->DynamicCast(IID_FIGHT_PET);
	return pPet ? 2 : 0;
}

CPlayer* CFightObject::GetFightObjToPlayer( CFightObject *pFighter )
{
	CPlayer *player = (CPlayer*)pFighter->DynamicCast(IID_PLAYER);
	if(player)
		return player;
	CFightPet *pPet = (CFightPet*)pFighter->DynamicCast(IID_FIGHT_PET);
	return pPet ? pPet->m_owner : NULL;
}

// 触发辅助主动被动技能
BOOL CFightObject::ActiveSecondarySkill(CFightObject* pAttack, CFightObject* pDest, WORD DamageValue)
{
	if (!pAttack)
	{
		rfalse(4, 1, "Fightobject.cpp - ActiveSecondarySkill() - !pAttack");
		return FALSE;
	}

	if (!pDest)
	{
		rfalse(4, 1, "Fightobject.cpp - ActiveSecondarySkill() - !pDest");
		return FALSE;
	}

	CPlayer *pPlayer = (CPlayer*)pAttack->DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	// 攻击者是否触发此技能的概率
	DWORD InitiativeRate = CRandom::RandRange(1, 10000);

	// 得到技能触发的基础概率
	INT32 skillID = pPlayer->GetSkillIDBySkillIndex(m_AtkContext.dwSkillIndex);
	INT32 skillLevel = pPlayer->GetCurrentSkillLevel(m_AtkContext.dwSkillIndex);
	DWORD rate = skillLevel * 50;

	// 得到心法提升概率,同时得到被攻击者对应的技能ID
	DWORD destSkillID = 0;

	switch (skillID)
	{
	case 6000:
		rate += pPlayer->m_TelergyActiveRate[0];
		destSkillID = 6001;
		break;

	case 6002:
		rate += pPlayer->m_TelergyActiveRate[1];
		destSkillID = 6003;
		break;

	case 6004:
		rate += pPlayer->m_TelergyActiveRate[2];
		destSkillID = 6005;
		break;

	case 6006:
		rate += pPlayer->m_TelergyActiveRate[3];
		destSkillID = 6007;
		break;

	case 6008:
		rate += pPlayer->m_TelergyActiveRate[4];
		destSkillID = 6009;
		break;

	default:
		return FALSE;
	}

	if (0 == destSkillID)
	{
		rfalse(4, 1, "Fightobject.cpp - ActiveSecondarySkill() - 0 == destSkillID");
		return FALSE;
	}
	MY_ASSERT(destSkillID != 0);

	// 无法触发[辅助主动技能]，直接退出
	//if (InitiativeRate > rate)
	//	return FALSE;

	// 计算被攻击者的反抗概率
	DWORD tempRate = CRandom::RandRange(1, 10000);
	DWORD PassiveRate = 0;

	CPlayer* pDestPlayer = (CPlayer*)pDest->DynamicCast(IID_PLAYER);

	// 被攻击者是玩家的情况
	if (pDestPlayer)
	{
		for (size_t i = 0; i < MAX_SKILLCOUNT; ++i)
		{
			// 当反抗技能在被攻击者的技能列表中的时候,计算反抗概率
			if (destSkillID == pDestPlayer->m_Property.m_pSkills[i].wTypeID)
			{
				PassiveRate = pDestPlayer->m_Property.m_pSkills[i].byLevel * 50;
				// 反抗成功，直接退出，没有效果
				if (tempRate <= PassiveRate)
					return FALSE;
				else
					break;
			}

			if (!pDestPlayer->m_Property.m_pSkills[i].wTypeID)
				break;
		}
	}
	else	// 被攻击者是怪物的情况
	{

	}

	switch (skillID)
	{
	case 6000:// 扣伤害，加生命
		pPlayer->ModifyCurrentHP(DamageValue * 0.001, 0, this);
		break;

	case 6002:// 扣内力
		pDest->ModifyCurrentMP(pDest->m_MaxMp * 0.001, 0);
		break;

	case 6004:// 扣体力
		pDest->ModifyCurrentTP(pDest->m_MaxTp * 0.001, 0);
		break;

	case 6006:// 扣攻击 -白手入刃
		pDest->m_GongJi -= pDest->m_GongJi * 0.001;
		break;

	case 6008:// 扣防御 -脱泡卸甲
		pDest->m_FangYu -= pDest->m_FangYu * 0.001;
		break;

	default:
		return FALSE;
	}
	//// 调用技能效果脚本
	//g_Script.SetCondition(0, pPlayer, 0);
	//g_Script.SetFightCondition(pDest);
	//LuaFunctor(g_Script, FormatString("SecondarySkillAttack%d", m_AtkContext.dwSkillIndex))[m_AtkContext.dwSkillIndex, DamageValue]();
	//g_Script.CleanFightCondition();
	//g_Script.CleanCondition();
	return TRUE;
}

DWORD CFightObject::GetDynamicRegionID()
{
	CDynamicRegion *dyregion=(CDynamicRegion*)m_ParentRegion->DynamicCast(IID_DYNAMICREGION);
	return (dyregion)?dyregion->GetGID():0;
}

void CFightObject::SetQusetSkillMsg( SQuestSkill_C2S_MsgBody *pmsg )
{
	if (pmsg)
	{
		//m_backUpQusetSkillMsg = *pmsg;
		m_LuaQuestSkillMsg = *pmsg;
		//rfalse("SetQusetSkillMsg index = %d",m_backUpQusetSkillMsg.dwSkillIndex);
	}
}

bool CFightObject::GetcontinueSkill( SQuestSkill_C2S_MsgBody *pMsg )
{
	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		g_Script.SetPlayer(pPlayer);
		if (g_Script.GetQuestSkill("ProcessQuestSkill", pMsg->mAttackerGID, pMsg->mDefenderGID, pMsg->mDefenderWorldPosX, pMsg->mDefenderWorldPosY, pMsg->dwSkillIndex,m_LuaQuestSkillMsg))
		{
			g_Script.CleanPlayer();
			return true;
		}
		g_Script.CleanPlayer();
	}
	return false;
}

int CFightObject::GetContinueSkillTarget( SQuestSkill_C2S_MsgBody &pmsg )
{
	if (GetCacheSkill(pmsg))
	{
		if (m_LuaQuestSkillMsg.mAttackerGID > 0)
		{
			DWORD gid = m_LuaQuestSkillMsg.mDefenderGID;     //验证目标有效性
			LPIObject pObject = m_ParentRegion->SearchObjectListInAreas(gid, m_ParentArea->m_X, m_ParentArea->m_Y);
			CFightObject *pLife = (CFightObject *)pObject->DynamicCast(IID_FIGHTOBJECT);
			if (!pLife || 0 == pLife->m_CurHp)
			{
				//rfalse(2,1,"cache target not find");
				return 0;
			}
			return 1;
		}
	}
	return -1;
	// 	if (m_vcacheQuestSkill.size() >0)
	// 	{
	// 		DWORD gid = m_vcacheQuestSkill[0].mDefenderGID;     //验证目标有效性
	// 		LPIObject pObject = m_ParentRegion->SearchObjectListInAreas(gid, m_ParentArea->m_X, m_ParentArea->m_Y);
	// 		CFightObject *pLife = (CFightObject *)pObject->DynamicCast(IID_FIGHTOBJECT);
	// 		if (!pLife || 0 == pLife->m_CurHp)
	// 		{
	// 			rfalse("cache target not find");
	// 			return 0;
	// 		}
	// 		pmsg = m_vcacheQuestSkill[0];
	// 		return 1;
	// 	}
	// 	return -1;
}

BOOL CFightObject::verifySkillMsg( SQuestSkill_C2S_MsgBody *questMsg )
{
	if (!questMsg/* || skillModifyDest*/)
	{
		rfalse(4, 1, "Fightobject.cpp - __ProcessQuestSkill() - !questMsg");
		return FALSE;
	}

	if (!m_CurHp)
	{
		rfalse(4, 1, "Fightobject.cpp - __ProcessQuestSkill() - !m_curhp");
		return FALSE;
	}
	MY_ASSERT(m_CurHp);

	//获得相关数据 验证技能的有效性
	///////////////////////////////////////////////////////////////////////////
	INT32 skillID = GetSkillIDBySkillIndex(questMsg->dwSkillIndex);
	if (-1 == skillID)
		return FALSE;

	const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(skillID);
	if (!pData || SKT_PASSITIVE == pData->m_SkillType)
		return FALSE;

	const SSkillHitSpot *pHitPot = CSkillService::GetInstance().GetSkillHitSpot(skillID);
	if (!pHitPot)
		return FALSE;


	///////////////////////////////////////////////////////////////////////////
	return TRUE;
}

void CFightObject::AddQuestToCache( SQuestSkill_C2S_MsgBody *questMsg )
{
	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		g_Script.SetPlayer(pPlayer);
		if (g_Script.PrepareFunction("AddPlayerCacheSkill"))
		{
			g_Script.PushParameter(questMsg->mAttackerGID);
			g_Script.PushParameter(questMsg->mDefenderGID);
			g_Script.PushParameter(questMsg->mDefenderWorldPosX);
			g_Script.PushParameter(questMsg->mDefenderWorldPosY);
			g_Script.PushParameter(questMsg->dwSkillIndex);
			g_Script.Execute();
		}
		g_Script.CleanPlayer();
		return;
	}
}

bool CFightObject::GetCacheSkill( SQuestSkill_C2S_MsgBody &pmsg )
{
	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		g_Script.SetPlayer(pPlayer);
		
		if (g_Script.PrepareFunction("GetPlayerCacheSkill"))
		{
			g_Script.PushParameter(pPlayer->GetGID());
			g_Script.Execute();
		}
		g_Script.CleanPlayer();
		return true;
	}
	return false;
}

void CFightObject::LuaSetCacheSkill( SQuestSkill_C2S_MsgBody *pmsg )
{
	if (pmsg)
	{
		m_LuaQuestSkillMsg = *pmsg;
	}
} 

void CFightObject::SetDamage(const SDamage *pDamage)
{
	if (pDamage)
	{
		memcpy(&m_sdamage, pDamage, sizeof(SDamage));
	}
}

void CFightObject::MoveInArea(CFightObject* pfightobj, float moveposX, float moveposY, DNID dnidExcept)
{
	if (!pfightobj || pfightobj->m_CurHp == 0)
	{
		return;
	}
	WORD oriX = (WORD)(moveposX) >> TILE_BITW;
	WORD oriY = (WORD)(moveposY) >> TILE_BITH;

	WORD wStartX = oriX;
	WORD wStartY = oriY;

	CPlayer *pPlayer = (CPlayer*)pfightobj->DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		GetGW()->PutPlayerIntoRegion(pPlayer->self.lock(), pPlayer->m_ParentRegion->m_wRegionID, wStartX, wStartY, pPlayer->GetDynamicRegionID(),false);
		return;
	}
	PutIntoRegion(pfightobj->self.lock(), pfightobj->m_ParentRegion->m_wRegionID, wStartX, wStartY, pfightobj->GetDynamicRegionID(), dnidExcept);
	return;

// 	DWORD dygid = pfightobj->GetDynamicRegionID();
// 	if (dygid > 0)
// 	{
// 		if (pfightobj->m_ParentArea)
// 		{
// 			SADelObjectMsg msg;
// 			msg.dwGlobalID = pfightobj->GetGID();
// 			pfightobj->m_ParentArea->SendAdj(&msg, sizeof(msg), -1);
// 		}
// 		//在这里做事情就可以了
// 		pfightobj->m_curTileX = wStartX;
// 		pfightobj->m_curTileY = wStartY;
// 		pfightobj->m_curX = float(wStartX << TILE_BITW);
// 		pfightobj->m_curY = float(wStartY << TILE_BITH);
// 
// 		if (0 == pfightobj->m_ParentRegion->Move2Area(pfightobj->self.lock(), wStartX, wStartY, true))
// 			 rfalse(2, 1, "PutPlayerToRegion: Move2Area Fail.");
// 
// 
// 		return;
// 	}
// 	
// 	oriX = GetCurArea(oriX, _AreaW);
// 	oriY = GetCurArea(oriY, _AreaH);
// 
// 	CArea *pDestArea = (CArea*)pfightobj->m_ParentRegion->GetArea(oriX, oriY)->DynamicCast(IID_AREA);
// 
// 	bool bDelObject = true;
// 	if (pfightobj->m_ParentArea&&pDestArea&&pfightobj->m_ParentArea == pDestArea){
// 		bDelObject = false;
// 	}
// 
// 	if (bDelObject)
// 	{
// 		if (pfightobj->m_ParentArea)
// 		{
// 			SADelObjectMsg msg;
// 			msg.dwGlobalID = pfightobj->GetGID();
// 			pfightobj->m_ParentArea->SendAdj(&msg, sizeof(msg), -1);
// 		}
// 	}
// 
// 
// 	pfightobj->m_curTileX = wStartX;
// 	pfightobj->m_curTileY = wStartY;
// 	pfightobj->m_curX = float(wStartX << TILE_BITW);
// 	pfightobj->m_curY = float(wStartY << TILE_BITH);
// 
// 
// 	if (bDelObject){
// 		rfalse(2, 1, "MoveArea");
// 		if (pfightobj->m_ParentRegion->Move2Area(pfightobj->self.lock(), wStartX, wStartY, true) == 0)
// 		{
// 			rfalse(2, 1, "MoveInArea: AddObject Fail.");
// 			return;
// 		}
// 	}

}

void CFightObject::PutIntoRegion(LPIObject pP, WORD wRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID,  DNID dnidExcept)
{
	CFightObject* pfightobj = (CFightObject*)pP->DynamicCast(IID_FIGHTOBJECT);
	if (!pfightobj || pfightobj->m_CurHp == 0)
		return;

	// 动态地图的处理 （dwRegionGID != 0的就是动态场景）
	// 说明需要以GID作为索引编号（做为动态地图，动态场景无法以地图ID为索引，因为有多个）
	if (0 != dwRegionGID)
	{
		//动态地图的瞬移
		if (pfightobj->m_ParentRegion && pfightobj->m_ParentRegion->GetGID() == dwRegionGID)
		{
			WORD oriX = wStartX;
			WORD oriY = wStartY;

			oriX = GetCurArea(oriX, _AreaW);
			oriY = GetCurArea(oriY, _AreaH);

			CArea *pDestArea = (CArea*)pfightobj->m_ParentRegion->GetArea(oriX, oriY)->DynamicCast(IID_AREA);
			bool bDelObject = true;
			if (pfightobj->m_ParentArea&&pDestArea&&pfightobj->m_ParentArea == pDestArea){
				bDelObject = false;
			}

			if (pfightobj->m_ParentArea&&bDelObject)
			{
				SADelObjectMsg msg;
				msg.dwGlobalID = pfightobj->GetGID();
				pfightobj->m_ParentArea->SendAdj(&msg, sizeof(msg), dnidExcept);
			}
			//在这里做事情就可以了
			pfightobj->m_curTileX = wStartX;
			pfightobj->m_curTileY = wStartY;
			pfightobj->m_curX = float(wStartX << TILE_BITW);
			pfightobj->m_curY = float(wStartY << TILE_BITH);
		
			if (0 == pfightobj->m_ParentRegion->Move2Area(pP, wStartX, wStartY, true))
			{
				rfalse(2, 1, "PutPlayerToRegion: Move2Area Fail.");
				return;
			}

			pfightobj->Stand();
			return;
		}
	}

	//普通场景
	if (pfightobj->m_ParentRegion)
	{
		// 普通场景的瞬移
		if (!pfightobj->m_ParentRegion->DynamicCast(IID_DYNAMICREGION) && pfightobj->m_ParentRegion->m_wRegionID == wRegionID)
		{
			WORD oriX = wStartX;
			WORD oriY = wStartY;

			oriX = GetCurArea(oriX, _AreaW);
			oriY = GetCurArea(oriY, _AreaH);

			CArea *pDestArea = (CArea*)pfightobj->m_ParentRegion->GetArea(oriX, oriY)->DynamicCast(IID_AREA);
			bool bDelObject = true;
			if (pfightobj->m_ParentArea&&pDestArea&&pfightobj->m_ParentArea == pDestArea){
				bDelObject = false;
			}

			if (bDelObject)
			{
				SADelObjectMsg msg;
				msg.dwGlobalID = pfightobj->GetGID();
				pfightobj->m_ParentArea->SendAdj(&msg, sizeof(msg), dnidExcept);
			}


			pfightobj->m_curTileX = wStartX;
			pfightobj->m_curTileY = wStartY;
			pfightobj->m_curX = float(wStartX << TILE_BITW);
			pfightobj->m_curY = float(wStartY << TILE_BITH);
	

			if (bDelObject){
				if (pfightobj->m_ParentRegion->Move2Area(pP, wStartX, wStartY, true) == 0)
				{
					rfalse(2, 1, "PutPlayerToRegion: AddObject Fail.");
					return;
				}
					
			}
			pfightobj->Stand();
			return ;
		}
	}
}

void CFightObject::MoveObiect_AttackMore(CFightObject *pDest,BYTE bSkillPropertyType, float fmoveX, float fmoveY)
{
// 	if (pDest&&bSkillPropertyType == SPT_MOVE)
// 	{
// 		rfalse(2, 1, "AttackMore MonsterPosX = %4.2f,MonsterPosY = %4.2f", pDest->m_curX, pDest->m_curY);
// 	}

	if (SPT_CHARGE == bSkillPropertyType) //冲锋类技能,设置自身坐标点
	{
		if (m_AtkContext.m_SkillAttackPos.x > 0.0f || m_AtkContext.m_SkillAttackPos.y > 0.0f)
		{
			//MoveInArea(this, m_AtkContext.m_SkillAttackPos.x, m_AtkContext.m_SkillAttackPos.y);
			SASynPosMsg	msg;
			msg.m_GID = this->GetGID();
			msg.m_Action = EA_STAND;
			msg.m_X = m_AtkContext.mDefenderWorldPosX;
			msg.m_Y = m_AtkContext.mDefenderWorldPosY;
			msg.m_Z = m_curZ;

			CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
			if (pArea)
			{
				pArea->SendAdj(&msg, sizeof(SASynPosMsg), -1);
			}
		}
	}
	else if (SPT_REPEL == bSkillPropertyType) //击退目标
	{
		if (pDest)
		{
			if (!(pDest->m_fightState & FS_ADDBUFF))
			{
				DNID dnidExcept = -1;
				CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
				if (pPlayer)
				{
					dnidExcept = pPlayer->m_ClientIndex;
				}
				//MoveInArea(pDest, fmoveX, fmoveY, dnidExcept);
				SASynPosMsg	msg;
				msg.m_GID = this->GetGID();
				msg.m_Action = EA_STAND;
				msg.m_X = fmoveX;
				msg.m_Y = fmoveY;
				msg.m_Z = m_curZ;

				CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
				if (pArea)
				{
					pArea->SendAdj(&msg, sizeof(SASynPosMsg), dnidExcept);
				}
			}
			pDest->ChangeFightState(BAP_JITUI, false, GetGID(), pDest->m_curX, pDest->m_curY, m_AtkContext.m_SkillData.m_ID);  //恢复目标的状态
		}	
	}
	else if (SPT_MOVE == bSkillPropertyType) //位移技能
	{
		if (m_AtkContext.m_SkillAttackPos.x > 0.0f || m_AtkContext.m_SkillAttackPos.y > 0.0f)
		{
			//MoveInArea(this, m_AtkContext.m_SkillAttackPos.x, m_AtkContext.m_SkillAttackPos.y);

			SASynPosMsg	msg;
			msg.m_GID = this->GetGID();
			msg.m_Action = EA_STAND;
			msg.m_X = m_curX;
			msg.m_Y = m_curY;
			msg.m_Z = m_curZ;

			CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
			if (pArea)
			{
				pArea->SendAdj(&msg, sizeof(SASynPosMsg), -1);
			}
		}
		DNID dnidExcept = -1;
		CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
		if (pPlayer)
		{
			dnidExcept = pPlayer->m_ClientIndex;
		}
		if (pDest)
		{
			if (!(pDest->m_fightState & FS_ADDBUFF))
			{
				//MoveInArea(pDest, fmoveX, fmoveY, dnidExcept);
				SASynPosMsg	msg;
				msg.m_GID = this->GetGID();
				msg.m_Action = EA_STAND;
				msg.m_X = fmoveX;
				msg.m_Y = fmoveY;
				msg.m_Z = m_curZ;

				CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
				if (pArea)
				{
					pArea->SendAdj(&msg, sizeof(SASynPosMsg), dnidExcept);
				}
			}
			pDest->ChangeFightState(BAP_JITUI, false, GetGID(), pDest->m_curX, pDest->m_curY, m_AtkContext.m_SkillData.m_ID);  //恢复目标的状态
		}
	}
// 	if (pDest&&bSkillPropertyType == SPT_MOVE)
// 	{
// 		rfalse(2, 1, "End AttackMore MonsterPosX = %4.2f,MonsterPosY = %4.2f", pDest->m_curX, pDest->m_curY);
// 	}
}

DWORD CFightObject::GetSceneRegionID()
{
	if (m_ParentRegion)
	{
		CDynamicScene *dyregion = (CDynamicScene*)m_ParentRegion->DynamicCast(IID_DYNAMICSCENE);
		if (dyregion)
		{
			return dyregion->GetGID();
		}
		return m_ParentRegion->GetGID();
	}
	return 0;
}

void CFightObject::AddToFollow(CFightObject *pfight)
{
	if (!pfight)return;
	std::map<DWORD, CFightObject*>::iterator iter = m_followObject.find(pfight->GetGID());
	if (iter == m_followObject.end())
	{
		m_followObject[pfight->GetGID()] = pfight;
	}
}

void CFightObject::RemoveFollow(CFightObject *pfight)
{
	if (!pfight)return;
	std::map<DWORD, CFightObject*>::iterator iter = m_followObject.find(pfight->GetGID());
	if (iter != m_followObject.end())
	{
		m_followObject.erase(iter);
	}
}

void CFightObject::ClearFollow()
{
	m_followObject.clear();
}

WORD CFightObject::GetSkillDamageRate(WORD skillIndex, WORD wlevel)
{
	 const SSkillBaseData *pskilldata = CSkillService::GetInstance().GetSkillBaseData(skillIndex, wlevel);
	 if (pskilldata)
	 {
		 return pskilldata->m_DamageRate;
	 }
	 return 0;
}

void CFightObject::DamageObject_AttackMoreForMonster(bool self)
{
	INT32 startX, startY, endX, endY;

	//rfalse("DamageObject_AttackMore x = %f,y = %f", m_curX, m_curY);
	if (self)		// 以自身为原点，不可能有子弹
	{
		startX = m_ParentArea->m_X - 1;
		startY = m_ParentArea->m_Y - 1;
	}
	else			// 目标点为原点，可能释放子弹
	{
		WORD curAreaX = GetCurArea((DWORD)(m_AtkContext.mDefenderWorldPosX) >> TILE_BITW, _AreaW);
		WORD curAreaY = GetCurArea((DWORD)(m_AtkContext.mDefenderWorldPosY) >> TILE_BITH, _AreaH);
		startX = curAreaX - 1;
		startY = curAreaY - 1;
	}

	endX = startX + 3;
	endY = startY + 3;
	// 获取攻击名单
	INT32 monsterNumber = 0;
	std::list<CFightObject *> destList;

// 	for (int x = startX; x < endX; x++)
// 	{
// 		for (int y = startY; y < endY; y++)
// 		{
// 			//check_list<LPIObject> *monstList = m_ParentRegion->GetMonsterListByAreaIndex(x, y);
// 			//GetAtkObjectByList(monstList, &destList, &monsterNumber, m_AtkContext.m_SkillData.m_AtkType, m_AtkContext.m_SkillData.m_AtkAmount);
// 
// 			check_list<LPIObject> *playerList = m_ParentRegion->GetPlayerListByAreaIndex(x, y);
// 			GetAtkObjectByList(playerList, &destList, &monsterNumber, m_AtkContext.m_SkillData.m_AtkType, m_AtkContext.m_SkillData.m_AtkAmount);
// 		}
// 	}
	GetAtkObjectByList(&m_ParentArea->m_PlayerList, &destList, &monsterNumber, m_AtkContext.m_SkillData.m_AtkType, m_AtkContext.m_SkillData.m_AtkAmount);

	if (m_AtkContext.m_CalcDamg[m_AtkContext.m_CurAtkNumber - 1])
	{
		SAWoundObjectMsgEx woundMsg;
		woundMsg.mAttackerGID = GetGID();
		//woundMsg.mSkillIndex		= (WORD)m_AtkContext.dwSkillIndex;
		woundMsg.mSkillIndex = GetSkillIDBySkillIndex(m_AtkContext.dwSkillIndex);
		woundMsg.mMaxHP = m_MaxHp;
		woundMsg.mCurHP = m_CurHp;


		size_t monsterIndex = 0;

		for (std::list<CFightObject *>::iterator it = destList.begin(); it != destList.end(); ++it)
		{
		 	CFightObject *fightObject = *it;
		 
		 	// 检查在被攻击时要强制中断的Buff
		 	fightObject->CheckDelBuffSpot(BDT_ATTACKED);
		 
		 	SDamage damage;
		 	SWoundTiny *wound = (((SWoundTiny *)&woundMsg.streamData)) + monsterIndex;
		 	//if (IsHit(this, fightObject))
		 	{

		 		// 这里是技能攻击伤害计算
		 		DamageEnemyBySkillAttack(fightObject, damage);
				fightObject->InitWoundTiny(wound, damage, damage.mHitState, fightObject->m_curX, fightObject->m_curY);
		 
		 		// 检查在受伤害时要强制中断的Buff
		 		fightObject->CheckDelBuffSpot(BDT_DAMAGED);
		 	}
		 
		 
		 	monsterIndex++;
		}

		//MY_ASSERT(monsterIndex == monsterNumber);
		woundMsg.mWoundObjectNumber = monsterIndex;
		WORD size = offsetof(SAWoundObjectMsgEx, streamData) + sizeof(SWoundTiny)* woundMsg.mWoundObjectNumber;
		if (monsterNumber)
			Synchro(&woundMsg, size);
	}

	if (!self && 2 == m_AtkContext.m_CalcDamg[m_AtkContext.m_CurAtkNumber - 1])
		BulletManager::GetInstance().FireBullet(m_AtkContext.m_SkillData.m_BulletID, this, 0, m_AtkContext.m_SkillData.m_AtkType);

	for (std::list<CFightObject *>::iterator it = destList.begin(); it != destList.end(); ++it)
	{
		CFightObject *fightObject = *it;
		m_AtkContext.m_TargetList.push_back(fightObject);
		IntoFightState(fightObject);
	}

}

bool CFightObject::GetDestList(std::list<CFightObject *> *destList, bool self)
{
	INT32 startX, startY, endX, endY;

	//rfalse("DamageObject_AttackMore x = %f,y = %f", m_curX, m_curY);
	if (self)		// 以自身为原点，不可能有子弹
	{
		startX = m_ParentArea->m_X - 1;
		startY = m_ParentArea->m_Y - 1;
	}
	else			// 目标点为原点，可能释放子弹
	{
		WORD curAreaX = GetCurArea((DWORD)(m_AtkContext.mDefenderWorldPosX) >> TILE_BITW, _AreaW);
		WORD curAreaY = GetCurArea((DWORD)(m_AtkContext.mDefenderWorldPosY) >> TILE_BITH, _AreaH);
		startX = curAreaX - 1;
		startY = curAreaY - 1;
	}

	endX = startX + 3;
	endY = startY + 3;
	// 获取攻击名单
	INT32 monsterNumber = 0;
	//std::list<CFightObject *> destList;

	for (int x = startX; x < endX; x++)
	{
		for (int y = startY; y < endY; y++)
		{
			//check_list<LPIObject> *monstList = m_ParentRegion->GetMonsterListByAreaIndex(x, y);
			//GetAtkObjectByList(monstList, &destList, &monsterNumber, m_AtkContext.m_SkillData.m_AtkType, m_AtkContext.m_SkillData.m_AtkAmount);

			check_list<LPIObject> *playerList = m_ParentRegion->GetPlayerListByAreaIndex(x, y);
			GetAtkObjectByList(playerList, destList, &monsterNumber, m_AtkContext.m_SkillData.m_AtkType, m_AtkContext.m_SkillData.m_AtkAmount);
		}
	}
	return true;
}

//add by ly 2014/4/4 获取玩家对象的战斗力
DWORD CFightObject::GetPlayerFightPower()
{
	int num = lua_gettop(g_Script.ls);
	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	int rs = 0;
	if (NULL != pPlayer)
	{
		lua_State *L = g_Script.ls;
		lua_createtable(L, 0, 0);

		lua_pushstring(L, "MaxHp");
		lua_pushnumber(L, pPlayer->m_MaxHp);
		lua_settable(L, -3);

		lua_pushstring(L, "GongJi");
		lua_pushnumber(L, pPlayer->m_GongJi);
		lua_settable(L, -3);

		lua_pushstring(L, "FangYu");
		lua_pushnumber(L, pPlayer->m_FangYu);
		lua_settable(L, -3);

		lua_pushstring(L, "Hit");
		lua_pushnumber(L, pPlayer->m_Hit);
		lua_settable(L, -3);

		lua_pushstring(L, "ShanBi");
		lua_pushnumber(L, pPlayer->m_ShanBi);
		lua_settable(L, -3);

		lua_pushstring(L, "BaoJi");
		lua_pushnumber(L, pPlayer->m_BaoJi);
		lua_settable(L, -3);

		lua_pushstring(L, "Uncrit");
		lua_pushnumber(L, pPlayer->m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT]);
		lua_settable(L, -3);

		lua_pushstring(L, "Wreck");
		lua_pushnumber(L, pPlayer->m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT]);
		lua_settable(L, -3);

		lua_pushstring(L, "Unwreck");
		lua_pushnumber(L, pPlayer->m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT]);
		lua_settable(L, -3);

		lua_pushstring(L, "Puncture");
		lua_pushnumber(L, pPlayer->m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT]);
		lua_settable(L, -3);

		lua_pushstring(L, "Unpuncture");
		lua_pushnumber(L, pPlayer->m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT]);
		lua_settable(L, -3);

		lua_getglobal(L, "Getfightpower");
		lua_pushvalue(L, -2);
		if (lua_pcall(L, 1, 1, 0) != 0)
			return 0;
		rs = lua_tonumber(L, -1);
		lua_pop(L, 2);
	}
	return rs;
}

void CFightObject::OnRecvCheckMove(SQWinMoveCheckMsg *pMsg)
{
	if (pMsg)
	{
		CMonster *pmonster = (CMonster*)GetObjectByGID(pMsg->dtagertgid)->DynamicCast(IID_MONSTER);
		if (pmonster)
		{
			float flenth = GetLenth(pmonster->m_curX, pmonster->m_curY, pMsg->fDestX, pMsg->fDestY);
			if (flenth < 5000)
			{
				//rfalse("OnRecvCheckMove %4.2f,%4.2f",pMsg->fDestX,pMsg->fDestY);
				CFightObject *pfight = (CFightObject*)pmonster->DynamicCast(IID_FIGHTOBJECT);
				if (pfight)
				{
					//MoveInArea(pfight, pMsg->fDestX, pMsg->fDestY);
					SASynPosMsg	msg;
					msg.m_GID = this->GetGID();
					msg.m_Action = EA_STAND;
					msg.m_X = pMsg->fDestX;
					msg.m_Y = pMsg->fDestY;
					msg.m_Z = m_curZ;

					CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
					if (pArea)
					{
						pArea->SendAdj(&msg, sizeof(SASynPosMsg), -1);
					}
				}	
			}
			else
			{
				rfalse(2, 1, "OnRecvCheckMove To Lenth = %f PosX = %4.2f,PosY = %4.2f",flenth,pMsg->fDestX,pMsg->fDestY);
			}
		}
	}
}

void CFightObject::SetMovePos(float fx, float fy)
{
	SQSynPathMsg WayMsg;

	// 首先计算距离
	D3DXVECTOR2 ver((m_curX - fx), (m_curY - fy));
	float distance = D3DXVec2Length(&ver);  //目标点与自己的距离
	//float distance = sqrt(pow((m_curX - pLife->m_curX), 2) + pow((m_curY - pLife->m_curY), 2));
	if (distance < 100)
	{
		return;
	}

	if (m_fightState & FS_DIZZY || m_fightState & FS_JITUI)
	{
		rfalse(2, 1, "定身状态下无法移动");
		return ;
	}

	float b = GetWalkSpeed() / 2;  //200 毫秒每帧

	// 按照一定的步进单位逐步逼近（测试100）
	D3DXVECTOR2 curPos(m_curX, m_curY);
	D3DXVECTOR2 desPos(fx, fy);

	D3DXVECTOR2 desVector = desPos - curPos;
	D3DXVec2Normalize(&desVector, &desVector);

	float fRadian = atan2(desVector.y, desVector.x);

	D3DXVECTOR2 vMovePos;
	vMovePos.x = m_curX + b * cos(fRadian);
	vMovePos.y = m_curY + b * sin(fRadian);
	float newlen = D3DXVec2Length(&(vMovePos - curPos)); //每次移动的距离

	WORD wnode = 1;
	while (1)
	{
		//distance = sqrt(pow((pLife->m_curX - DestPosX), 2) + pow((pLife->m_curY - DestPosY), 2));
		WayMsg.m_Path.m_path[wnode].m_X = vMovePos.x;
		WayMsg.m_Path.m_path[wnode].m_Y = vMovePos.y;

		//distance = sqrt(pow((m_BX - DestPosX), 2) + pow((m_BY - DestPosY), 2));
		distance = D3DXVec2Length(&(desPos - vMovePos));
		newlen = D3DXVec2Length(&(vMovePos - curPos));
		if (newlen >= distance) //移动到超过目标点时
		{
			WayMsg.m_Path.m_path[wnode].m_X = desPos.x;   //最后1个点就是目标点了
			WayMsg.m_Path.m_path[wnode].m_Y = desPos.y;
			//rfalse(2, 1, "CreateRadomPoint id = %d , x = %f,y = %f ", GetGID(),desPos.x, desPos.y);
			break;
		}

		wnode++;
		curPos = vMovePos;

		vMovePos.x = vMovePos.x + b * cos(fRadian);
		vMovePos.y = vMovePos.y + b * sin(fRadian);

		if (wnode + 1 >= MAX_TRACK_LENGTH)
		{
			ver.x = fx - m_curX;
			ver.y = fy - m_curY;
			distance = D3DXVec2Length(&ver);
			rfalse("wnode to long Lenth = %f", distance);
			break;
		}
	}
	//rfalse("SendMove2TargetForAttackMsg x = %f,y = %f,destX = %f,destY = %f", m_curX,m_curY,DestPosX,DestPosY);

	WayMsg.m_GID = GetGID();
	WayMsg.m_move2What = SQSynWayTrackMsg::move_2_attack;
	WayMsg.m_Path.m_moveType = EA_RUN;
	WayMsg.m_Path.m_wayPoints = wnode + 1;
	WayMsg.m_Path.m_path[0].m_X = m_curX;
	WayMsg.m_Path.m_path[0].m_Y = m_curY;

	// 如果路径成功生成，则移动过去攻击
	m_IsMove2Attack = SQSynPathMsg::normal;
	SetMovePath(&WayMsg);
	return;
}

float CFightObject::GetWalkSpeed()
{
	return 0.0f;
}
