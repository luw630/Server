#include "StdAfx.h"
#include "Player.h"
#include "networkmodule/movemsgs.h"
#include "networkmodule/regionmsgs.h"
#include "CFightPetDefine.h"
#include "CFightPetService.h"
#include "Pet.h"
#include "Region.h"
#include "Area.h"
#include "Random.h"
#include "CSkillService.h"
#include "CPlayerService.h"
#include "gameobjects/gameworld.h"
#include "DynamicDailyArea.h"

extern BOOL PutPlayerIntoDestRegion(CPlayer *pPlayer, WORD wNewRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID = 0);


const int CFightPet::MAX_TIRED = 100;
const int CFightPet::MAX_Mood = 100;
const int CFightPet::MAX_RELATION = 50000;
const int CFightPet::MAX_IMPEL = 100;
extern int MAXPLAYERLEVEL;

CFightPet::CFightPet() { m_owner = 0; }

CFightPet::~CFightPet() 
{
	rfalse(2, 1, "销毁了一个侠客%d", GetGID());
}

int CFightPet::OnCreate(_W64 long pParameter)
{
	SParameter *pParam = (SParameter *)pParameter;
	if (!pParam->owner)
		return 0;

	SNewPetData &pPet = pParam->owner->m_Property.m_NewPetData[pParam->index];
	if (0 == pPet.m_PetID)
		return 0;

// 	const SFightPetBaseData *pBaseData = CFightPetService::GetInstance().GetFightPetBaseData(pPet.m_fpID);
// 	if (!pBaseData)
// 		return 0;

	m_owner		= pParam->owner;
	m_index		= pParam->index;
	m_curX		= pParam->fX;
	m_curY		= pParam->fY;
	m_curTileX	= ((DWORD)m_curX) >> TILE_BITW;
	m_curTileY	= ((DWORD)m_curY) >> TILE_BITH;
	
	m_Direction = m_owner->m_Direction;
	m_Speed = PLAYER_BASIC_SPEED;

// 	// 成长属性
// 	const SFightPetRise *priseData = CFightPetService::GetInstance().GetFightPetRise(pBaseData->m_RareMax);
// 	if (!priseData)
// 		return 0;
	
	// 战斗属性
//  	WORD tempLevel  = pPet.m_fpLevel - 1;
//  	m_MaxHp				= pPet.m_BaseMaxHp		    + (tempLevel) * pPet.m_fpHpIncRadio;
//  	m_MaxMp			= pPet.m_BaseMaxMp			+ (tempLevel) * pPet.m_fpMpIncRadio;
// 	m_GongJi				= pPet.m_BaseAtk					+ (tempLevel) * pPet.m_fpGongJi;
// 	m_FangYu			= pPet.m_BaseDefence			+ (tempLevel) * pPet.m_fpFangYu;
	DWORD PetIndex = m_owner->_L_GetLuaValue("GetPetDataIndex", pPet.m_PetID, pPet.m_PetLevel);
	if (PetIndex == 0xffffffff)
		return 0;

	DWORD PetFightDataIndex = m_owner->_L_GetLuaValue("GetPetBaseProperties", PetIndex, 4);
	if (PetFightDataIndex == 0xffffffff)
		PetFightDataIndex = 0;
	DWORD PetCurLevelMaxExp = m_owner->_L_GetLuaValue("GetPetBaseProperties", PetIndex, 6);
	if (PetCurLevelMaxExp == 0xffffffff)
		PetCurLevelMaxExp = 0;

	DWORD BaseGongJi = m_owner->_L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 9);
	if (BaseGongJi == 0xffffffff)
		BaseGongJi = 0;	
	DWORD BaseFangYu = m_owner->_L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 10);
	if (BaseFangYu == 0xffffffff)
		BaseFangYu = 0;
	DWORD BaseBaoJi = m_owner->_L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 11);
	if (BaseBaoJi == 0xffffffff)
		BaseBaoJi = 0;
	DWORD BaseShanBi = m_owner->_L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 18);
	if (BaseShanBi == 0xffffffff)
		BaseShanBi = 0;
	DWORD MaxHp = m_owner->_L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 5);
	if (MaxHp == 0xffffffff)
		MaxHp = 0;
	DWORD MaxMp = m_owner->_L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 6);
	if (MaxMp == 0xffffffff)
		MaxMp = 0;
	DWORD BaseHit = m_owner->_L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 17);
	if (BaseHit == 0xffffffff)
		BaseHit = 0;
	DWORD BaseUncrit = m_owner->_L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 12);
	if (BaseUncrit == 0xffffffff)
		BaseUncrit = 0;
	DWORD BaseWreck = m_owner->_L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 13);
	if (BaseWreck == 0xffffffff)
		BaseWreck = 0;
	DWORD BaseUnWreck = m_owner->_L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 14);
	if (BaseUnWreck == 0xffffffff)
		BaseUnWreck = 0;
	DWORD BasePuncture = m_owner->_L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 15);
	if (BasePuncture == 0xffffffff)
		BasePuncture = 0;
	DWORD BaseUnPuncture = m_owner->_L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 16);
	if (BaseUnPuncture == 0xffffffff)
		BaseUnPuncture = 0;


	m_MaxHp = MaxHp;
	m_MaxMp = MaxMp;
	m_GongJi = BaseGongJi;
	m_FangYu = BaseFangYu;
	m_BaoJi = BaseBaoJi;
	m_ShanBi = BaseShanBi;
	m_Hit = BaseHit;

	m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT] = BaseUncrit;		//抗暴
	m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT] = BaseWreck;	//破击
	m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT] = BaseUnWreck;				//抗破
	m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT] = BasePuncture;			//穿刺
	m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT] = BaseUnPuncture;			//抗穿


	m_CurHp = pPet.m_CurPetHp;
	m_CurMp = pPet.m_CurPetMp;
	m_CurTp				= 0;
	m_Level				= pPet.m_PetLevel;

	m_nMaxExp = PetCurLevelMaxExp;
	m_nCurExp			=pPet.m_CurPetExp;
	m_nRare				=0;

	m_nRelation			=0;
	m_nTired			=0;
	m_nMood				=0;
	m_bSex				=0;

	m_SorbDamageValue	= m_SorbDamagePercent = 0;

	m_CurEnemy.first	= m_CurEnemy.second = 0;
	m_TargetCache.first = m_TargetCache.second = 0;

	m_nOutFightTime		= timeGetTime();	//开始计算出战时间
	m_nOutMoodTime		= timeGetTime(); //开始计算出战心情时间

	m_bAttType = FP_PASSIVE;		//默认被动攻击
	m_nSearchTargetTime = timeGetTime();
	m_nChangeSkilltime = 0xffffffff;

	fightproperty[0] = 1;
	for(size_t i= 1;i<SAallpropertychange::PROPERTYNUM;i++)
	{
		fightproperty[i] = fightproperty[i-1]*2;
	}
// 	fightpropertytype = 0;
	
	return CFightObject::OnCreate(pParameter);
}

void CFightPet::OnDead(CFightObject *PKiller)
{
	//MY_ASSERT(0 == m_CurHp);

	CFightObject::OnDead(PKiller);	
	CDynamicDailyArea *pDailyScene = (CDynamicDailyArea *)m_ParentRegion->DynamicCast(IID_DYNAMICDAILYAREA);
	if (pDailyScene != NULL)
	{
		if (g_Script.PrepareFunction("OnDailyObjectDead"))
		{
			g_Script.PushParameter(GetGID());	//自己gid
			g_Script.PushParameter(GetSceneRegionID()); //所在地图gid
			g_Script.PushParameter(m_ParentRegion->m_wRegionID);//地图regionid
			if (PKiller)
			{
				g_Script.PushParameter(PKiller->GetGID());	//杀死玩家的gid
			}
			g_Script.PushParameter(this->GetGID());	//当前玩家的gid
			g_Script.Execute();
		}
	}
	else
	{
		if (g_Script.PrepareFunction("OnObjectDead"))
		{
			g_Script.PushParameter(GetGID());	//自己gid
			g_Script.PushParameter(GetSceneRegionID()); //所在地图gid
			g_Script.PushParameter(m_ParentRegion->m_wRegionID);//地图regionid
			if (PKiller)
			{
				g_Script.PushParameter(PKiller->GetGID());	//杀死玩家的gid
			}
			g_Script.Execute();
		}
	}
	
	m_CurHp = m_MaxHp;
	m_CurMp	= m_MaxMp;

	m_FightPropertyStatus[XA_CUR_HP] = true;
	m_FightPropertyStatus[XA_CUR_MP] = true;

	//// 死了就回主人那里吧
	//if (m_owner)
	//{
	//	m_owner->CallBackFightPet(false);
	//}
	return;
}

void CFightPet::OnDamage(INT32 *nDamage, CFightObject *pFighter)
{
	if (!pFighter)
	{
		rfalse(4,1,"CFightPet::OnDamage");
		return;
	}
	if (0 == m_CurHp)
		return;

	CPlayer *pAttackerPlayer = (CPlayer *)pFighter->DynamicCast(IID_PLAYER);
	if (pAttackerPlayer)
	{
		// 减少攻击者武器耐久
		if (0 == pAttackerPlayer->m_Property.m_Equip[EQUIP_P_WEAPON].wIndex && 0 != pAttackerPlayer->m_Property.m_Equip[EQUIP_P_WEAPON].attribute.currWear)
		{
			int Rate = CRandom::RandRange(1, 100);
			if (Rate <= 60)
				pAttackerPlayer->UpdateEquipWear(EQUIP_P_WEAPON, 1);
		}
	}

	//判断变身宠物魂力是否为0，当变身宠物被攻击时魂力减1	
	if (m_owner)
	{
		if (m_owner->m_Property.m_NewPetData[m_index].m_CurPetDurable == 0)
		{
			DWORD NeedTatalTime = m_owner->_L_GetLuaValue("GetHunliResumeTime");
			if (NeedTatalTime == 0xffffffff)
				NeedTatalTime = 0;
			INT64 CurTime = _time64(NULL);
			m_owner->m_Property.m_NewPetData[m_index].m_DurableResumeNeedTime = NeedTatalTime + (CurTime - m_owner->m_dwLoginTime);
			m_owner->CallBackFightPet(false);
			return;
		}
		m_owner->m_Property.m_NewPetData[m_index].m_CurPetDurable--;
		SAPetSynDurableMsg SynPetDurable;
		SynPetDurable.m_PetIndex = m_index;
		SynPetDurable.m_CurDurable = m_owner->m_Property.m_NewPetData[m_index].m_CurPetDurable;
		g_StoreMessage(m_owner->m_ClientIndex, &SynPetDurable, sizeof(SAPetSynDurableMsg));
	}

	// 查看伤害吸收
	/*if (m_SorbDamageValue)
		*nDamage -= m_SorbDamageValue;

	if (*nDamage < 0)
		*nDamage = 0;

	if (*nDamage && m_SorbDamagePercent)
		*nDamage -= (*nDamage * 100 / m_SorbDamagePercent);*/

	///根据心情值计算伤害
	*nDamage += GetDamageChange(*nDamage);

	int modifyhp = ModifyCurrentHP(-(*nDamage), 0, pFighter);
	
	if (0 != m_CurHp && pFighter)
		SetCurEnemy(pFighter->GetGID(), FPET_ATKME);
	
	if (modifyhp)
	{
		//SFightPetExt * pext = m_owner->_fpGetFightPetByIndex(m_index);
		SNewPetData *pext = &m_owner->m_Property.m_NewPetData[m_index];
		if (pext)
		{
			pext->m_CurPetHp = m_CurHp;
		}
	}


	return;
}

void CFightPet::UpdateAllProperties()
{
	WORD oldSpeed = m_OriSpeed;

	WORD wSpeed			= PLAYER_BASIC_SPEED;
	m_SorbDamageValue	= 0;
	m_SorbDamagePercent	= 0;

	m_SorbDamageValue	+= GetBuffModifyValue(BAP_SORBDAM,		0);
	m_SorbDamagePercent += GetBuffModifyValue(BAP_SORBDAM_PER,	0);
	wSpeed += GetBuffModifyValue(BAP_SPEED, PLAYER_BASIC_SPEED);
	m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_UNCRIT, m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_WRECK, m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_UNWRECK, m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_PUNCTURE, m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_UNPUNCTURE, m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT]);

	if (oldSpeed != wSpeed)
	{
		m_OriSpeed = wSpeed;
		m_Speed = m_OriSpeed / 1000.0f;
		m_FightPropertyStatus[XA_SPEED] = true;
	}
}

void CFightPet::SendPropertiesUpdate()
{
	CFightObject::SendPropertiesUpdate();

	

	// 通知主人我的HP，MP
	UpdateProperties();

	memset(m_FightPropertyStatus, 0, sizeof(m_FightPropertyStatus));
}

BOOL CFightPet::DoCurAction()
{
	return CFightObject::DoCurAction();
}

void CFightPet::SwithcStatus(bool PlayerContrl)
{
	if (!m_owner)
	{
		rfalse(4,1,"CFightPet::SwithcStatus");
		return;
	}
	SAFightPetStatusChg noticeMsg;
	noticeMsg.dwPetGID		= GetGID();
	noticeMsg.bCurStatus	= PlayerContrl ? SAFightPetStatusChg::SFPS_FOLLOW : SAFightPetStatusChg::SFPS_FIGHT;
	g_StoreMessage(m_owner->m_ClientIndex, &noticeMsg, sizeof(SAFightPetStatusChg));
}

void CFightPet::OnChangeState(EActionState newActionID)
{
	if (EA_SKILL_ATTACK != GetCurActionID() && EA_SKILL_ATTACK == newActionID)
		SwithcStatus(false);
}

void CFightPet::OnRun()
{
	SendPropertiesUpdate();		// 更新属性

	//if(m_bAttType == FP_ACTIVELY && m_TargetCache.first == 0 && m_CurEnemy.first == 0 && m_nSearchTargetTime != 0xffffffff && timeGetTime() - m_nSearchTargetTime > 2000)
	//{
	//	m_nSearchTargetTime = timeGetTime();
	//	// 搜寻一个目标
	//	check_list<LPIObject> *monsterList = m_ParentRegion->GetMonsterListByAreaIndex(m_ParentArea->m_X, m_ParentArea->m_Y);

	//	if (monsterList)
	//	{
	//		for (check_list<LPIObject>::iterator it = monsterList->begin(); it != monsterList->end(); ++it)
	//		{
	//			LPIObject target = *it;
	//			if (target)
	//			{
	//				CMonster *Monster = (CMonster  *)target->DynamicCast(IID_MONSTER);
	//				if (Monster && 0 != Monster->m_CurHp)
	//				{
	//					SetCurEnemy(Monster->GetGID(),FPET_ATKME);
	//					m_nSearchTargetTime = 0xffffffff;
	//					//						rfalse(2, 1, "找到一个没死的目标【%d】，准备发起攻击！", m_dwLastEnemyID);
	//					break;
	//				}
	//			}
	//		}				
	//	}
	//}

	//// 查看当前攻击目标是否已经更换
	//if (m_TargetCache.first && m_TargetCache.first != m_CurEnemy.first)
	//{
	//	if (!m_ParentRegion || !m_owner)
	//	{
	//		rfalse(4,1,"m_TargetCache.first != m_CurEnemy.first_1");
	//		return;
	//	}
	//	MY_ASSERT(m_ParentRegion);
	//	MY_ASSERT(m_owner);
	//	LPIObject object = m_owner->m_ParentRegion->SearchObjectListInAreas(m_TargetCache.first, m_owner->m_ParentArea->m_X, m_owner->m_ParentArea->m_Y);
	//	if (!object)
	//	{
	//		rfalse(4,1,"m_TargetCache.first != m_CurEnemy.first_2");
	//		return;
	//	}
	//	CFightObject *pLife = (CFightObject *)object->DynamicCast(IID_FIGHTOBJECT);
	//	if (pLife && pLife->m_CurHp)
	//	{
	//		SwithcStatus(false);
	//		SQuestSkill_C2S_MsgBody msg;
	//		msg.mDefenderGID		= m_TargetCache.first;
	//		msg.mAttackerGID		= GetGID();
	//		msg.mDefenderWorldPosX	= pLife->m_curX;
	//		msg.mDefenderWorldPosY	= pLife->m_curY;
	//		//选择侠客的技能攻击
	//		msg.dwSkillIndex = GetRandskillIndex();
	//		const SSkillBaseData *data = CSkillService::GetInstance().GetSkillBaseData(GetSkillIDBySkillIndex(msg.dwSkillIndex)) ;
	//		if(!data)return;
	//		WORD ConsumeMP = data->m_ConsumeMP; 
	//		if(m_CurMp<=ConsumeMP)msg.dwSkillIndex = 0;
	//		rfalse(2, 1, "侠客开始切换目标%d\r\n",pLife->GetGID());
	//		ProcessQuestSkill(&msg);
	//		m_CurEnemy = m_TargetCache;
	//		m_nChangeSkilltime = timeGetTime();
	//		return;
	//	}
	//}
	//else if(m_TargetCache.first == m_CurEnemy.first&&m_CurEnemy.first != 0&&m_nChangeSkilltime!=0xffffffff&&timeGetTime()-m_nChangeSkilltime>2000)
	//{
	//	m_backUpQusetSkillMsg.dwSkillIndex = GetRandskillIndex();
	//	const SSkillBaseData *data = CSkillService::GetInstance().GetSkillBaseData(GetSkillIDBySkillIndex(m_backUpQusetSkillMsg.dwSkillIndex)) ;
	//	if(!data)return;
	//	WORD ConsumeMP = data->m_ConsumeMP; 
	//	if(m_CurMp<=ConsumeMP)m_backUpQusetSkillMsg.dwSkillIndex = 0;
	//	//rfalse(2, 1, "侠客开始切换技能\r\n",m_backUpQusetSkillMsg.dwSkillIndex);
	//	__ProcessQuestSkill(&m_backUpQusetSkillMsg,TRUE);
	//	m_nChangeSkilltime = timeGetTime();
	//}
	CheckOnTime();
	CFightObject::OnRun();
}

bool CFightPet::SendMove2TargetForAttackMsg(INT32 skillDistance, INT32 skillIndex, CFightObject *pLife)
{
	SMove2TargetForAttack_S2C_MsgBody msg;
	msg.mAttackGID		= GetGID();
	msg.mDefenderGID	= pLife->GetGID();
	msg.distance		= skillDistance;

	g_StoreMessage(m_owner->m_ClientIndex, &msg, sizeof(SMove2TargetForAttack_S2C_MsgBody));

	return true;

	if (!m_ParentRegion || !pLife)
		return false;

	SQSynPathMsg WayMsg;

	// 按照一定的步进单位逐步逼近
	D3DXVECTOR2 curPos(m_curX, m_curY);
	D3DXVECTOR2 desPos(pLife->m_curX, pLife->m_curY);
	D3DXVECTOR2 desVector = desPos - curPos;
	D3DXVec2Normalize(&desVector, &desVector);

	// 计算距离
	float NeedMove = sqrt(pow((m_curX-pLife->m_curX),2)+pow((m_curY-pLife->m_curY),2));
//	WORD Threshold = (EA_RUN == pLife->GetCurActionID()) ? 100 : 40;
//	NeedMove -= (skillDistance + Threshold);

	D3DXVECTOR2 newPos = curPos + (desVector * NeedMove/2);

	WayMsg.m_GID				= GetGID();
	WayMsg.m_move2What			= SQSynWayTrackMsg::move_2_attack;
	WayMsg.m_Path.m_moveType	= EA_RUN;
	WayMsg.m_Path.m_wayPoints	= 2;
	WayMsg.m_Path.m_path[0].m_X = m_curX;
	WayMsg.m_Path.m_path[0].m_Y = m_curY;
	WayMsg.m_Path.m_path[1].m_X = newPos.x;
	WayMsg.m_Path.m_path[1].m_Y = newPos.y;

	// 如果路径成功生成，则移动过去攻击
	if (SetMovePath(&WayMsg))
	{
		SwithcStatus(false);
	}
	else
	{
		// 如果路径生成（设置）失败，那么表示出了问题，这时可以选择多种方案
		// 如：回到原点、原地不动等，此处暂时不处理，但是函数依然要返回true。
	}

	return true;
}

INT32 CFightPet::GetSkillIDBySkillIndex(INT32 index)
{
	if (!m_owner)
	{
		rfalse(4,1,"CFightPet::GetSkillIDBySkillIndex()_1");
		return -1;
	}
	//SFightPetExt *pet = m_owner->_fpGetFightPetByIndex(m_index);
	SNewPetData *pet = &m_owner->m_Property.m_NewPetData[m_index];
	if (!pet)
	{
		rfalse(4,1,"CFightPet::GetSkillIDBySkillIndex()_2");
		return -1;
	}
	MY_ASSERT(pet);
	return index<10&&index>=0?pet->m_PetSkill[index].m_PetSkillID:-1;
}

INT32 CFightPet::GetCurrentSkillLevel(DWORD dwSkillIndex)
{
	if (!m_owner)
	{
		rfalse(4,1,"CFightPet::GetCurrentSkillLevel()1");
		return 0;
	}
	//SFightPetExt *pet = m_owner->_fpGetFightPetByIndex(m_index);
	SNewPetData *pet = &m_owner->m_Property.m_NewPetData[m_index];
	if (!pet)
	{
		rfalse(4,1,"CFightPet::GetCurrentSkillLevel()2");
		return 0;
	}
	MY_ASSERT(pet);
	for (int i = 0; i < 10; i++)
	{
		if (dwSkillIndex == pet->m_PetSkill[i].m_PetSkillID && pet->m_PetSkill[i].m_Level > 0)
			return pet->m_PetSkill[i].m_Level;
	}
	return 0;
}

void CFightPet::AtkFinished()
{
	SwithcStatus(true);
	m_TargetCache.first = m_TargetCache.second = m_CurEnemy.first = m_CurEnemy.second = 0;
	m_nSearchTargetTime = timeGetTime();
	m_nChangeSkilltime = 0xffffffff;
}

void CFightPet::OnRunEnd()
{
	if (m_IsMove2Attack == SQSynWayTrackMsg::move_2_attack)
	{
	//	rfalse(2, 1, "侠客攻击移动完毕，当前坐标为%f, %f,攻击目标ID:%d", m_curX, m_curY,m_backUpQusetSkillMsg.mDefenderGID);
		m_backUpQusetSkillMsg = m_AtkContext;
		__ProcessQuestSkill(&m_backUpQusetSkillMsg, true);
	}
}

void CFightPet::OnClose()
{
	m_owner = 0;
	return;
}

SASynFightPetMsg *CFightPet::GetStateMsg()
{
	if (!m_owner)
	{
		rfalse(4,1,"CFightPet::GetStateMsg()");
		return 0;
	}
	static SASynFightPetMsg msg;

	msg.dwGlobalID	= GetGID();
	msg.dwOwnerID	= m_owner ? m_owner->GetGID() : 0;
	//msg.ID			= m_owner->m_Property.m_FightPets[m_index].m_fpID;
	msg.ID = m_owner->m_Property.m_NewPetData[m_index].m_PetID;///模型ID
	memset(msg.sName, 0, PETNAMEMAXLEN);
	strcpy(msg.sName, m_owner->m_Property.m_NewPetData[m_index].m_PetName);
	msg.mMoveSpeed	= m_Speed;
	msg.direction	= m_Direction;
	FillSynPos(&msg.ssp);

	rfalse(2, 1, "同步了一个侠客：ID-%d, Owner-%d", msg.dwGlobalID, msg.dwOwnerID);

	return &msg;
}

BOOL CFightPet::MoveToPostion( WORD wNewRegionID, float wStartX, float wStartY )
{
	if (m_ParentRegion && m_ParentArea)
	{
		if(!m_owner)return FALSE;

		m_curX=wStartX;
		m_curY=wStartY;

		m_curTileX	= ((DWORD)m_curX) >> TILE_BITW;
		m_curTileY	= ((DWORD)m_curY) >> TILE_BITH;

		
		SADelObjectMsg msg;
		msg.dwGlobalID = GetGID();
		m_ParentArea->SendAdj(&msg, sizeof(msg), -1);
		m_ParentArea->DelObject(this->self.lock());
		
		SetCurActionID(EA_STAND);
		SwithcStatus(true);

		if (m_ParentRegion->Move2Area(this->self.lock(), m_curTileX, m_curTileY,true) == 0)
			return rfalse(2, 1, "PutPetToRegion: AddObject Fail.");

// 		SAFightPetMoveFinsh smsgfinsh;
// 		smsgfinsh.dwGID=m_owner->GetGID();
// 		//SendMsg(&smsgfinsh,sizeof(SAFightPetMoveFinsh)); 
// 		g_StoreMessage(m_owner->m_ClientIndex,&smsgfinsh,sizeof(SAFightPetMoveFinsh)); 
		return TRUE;
	}
		return rfalse(2, 1, "PutPetToRegion: AddObject Fail.");
}

void CFightPet::OnRecvLevelUp( struct SQLevelChangeMsg *msg )
{
	if (!msg || !m_owner)
	{
		rfalse(4,1,"CFightPet::OnRecvLevelUp");
		return;
	}
	WORD  UpCount = 0;
	//QWORD needExp =CFightPetService::GetInstance().GetFightPetLevelExp(m_Level);
	QWORD needExp = CPlayerService::GetInstance().GetPlayerUpdateExp(m_Level)/2; //修改为玩家相应等级经验值一半

	QWORD CurExp = m_nCurExp;

	while (m_nCurExp >= needExp && (m_Level+UpCount) != MAXPLAYERLEVEL)
	{
		m_nCurExp -= needExp, UpCount++;
		needExp =CPlayerService::GetInstance().GetPlayerUpdateExp(m_Level)/2; //修改为玩家相应等级经验值一半
	}
	//如果可以升级在升级中处理玩家经验更新
	if (0 != UpCount)
	{
		if (m_Level+UpCount - m_owner->m_Level> 4)
		{
			m_nCurExp = CurExp;
			UpCount = m_Level - m_owner->m_Level > 4 ? 0 :  m_owner->m_Level +5 - m_Level;
			needExp =CPlayerService::GetInstance().GetPlayerUpdateExp(m_Level+UpCount)/2;
			m_nCurExp = m_nCurExp>needExp*2?needExp*2:m_nCurExp;
		}
		if(0 != UpCount)SendAddLevel(UpCount);

		if (MAXPLAYERLEVEL == m_Level)
			m_nCurExp = 0;

		return;
	}
	if (!m_owner)
	{
		rfalse(4,1,"CFightPet::OnRecvLevelUp()");
		return;
	}
	MY_ASSERT(m_owner);
	SFightPetExt &pPet = m_owner->m_Property.m_FightPets[m_index];
	if (0 == pPet.m_fpID)
		return;
	///更新侠客数据
	pPet.m_fpExp=m_nCurExp;
	
}

void CFightPet::SendAddLevel( WORD Uplevel )
{
	if (m_Level+Uplevel>MAXPLAYERLEVEL)return;
	if (!m_owner)
	{
		rfalse(4,1,"CFightPet::SendAddLevel()");
		return;
	}
	MY_ASSERT(m_owner);
	m_Level+=Uplevel;
	
	//获取升级以后的最大经验值
	UINT64 needExp =CPlayerService::GetInstance().GetPlayerUpdateExp(m_Level)/2; //修改为玩家相应等级经验值一半
	m_nMaxExp=needExp;

	//发送侠客升级消息
	SAFightPetLevelChange levelMsg;
	levelMsg.bLevel	=	m_Level;
	levelMsg.m_index=m_index;
	levelMsg.m_curExp=m_nCurExp;
	levelMsg.m_MaxExp=m_nMaxExp;
	g_StoreMessage(m_owner->m_ClientIndex, &levelMsg, sizeof(SAFightPetLevelChange));
	

	UpdateMyProperties();
}

const  char * CFightPet::getname()
{	
	if (!m_owner)
	{
		rfalse(4,1,"CFightPet::getname()");
		return "";
	}
	//SFightPetExt &pPet = m_owner->m_Property.m_FightPets[m_index];
	SNewPetData &pPet = m_owner->m_Property.m_NewPetData[m_index];
	return pPet.m_PetName? pPet.m_PetName :"";
}

void CFightPet::CheckOnTime()
{
	if (!m_owner)
	{
		rfalse(4,1,"CFightPet::CheckOnTime()");
		return;
	}
	SFightPetExt &pPet = m_owner->m_Property.m_FightPets[m_index];
	if (0 == pPet.m_fpID)
		return;

	///每10分钟增加1点疲劳度和1点友好度,测试改为1分钟
	if (timeGetTime()-m_nOutFightTime>MINUTESTIME(1))
	{
		m_owner->_fpChangeExtraProperties(SAFightPetExtraChange::CHANGE_TIRED,1,m_index);			//增加疲劳度
		m_owner->_fpChangeExtraProperties(SAFightPetExtraChange::CHANGE_RELATION,1,m_index);	//增加友好度
		m_nTired = pPet.m_fpTired;
		m_nRelation = pPet.m_fpRelation;
		m_nOutFightTime = timeGetTime();
	}
	
	///每30分钟减少心情5点,测试改为1分钟
	if (timeGetTime()-m_nOutMoodTime>MINUTESTIME(1))
	{
		m_owner->_fpChangeExtraProperties(SAFightPetExtraChange::CHANGE_MOOD,-5,m_index);	//改变心情
		m_nMood = pPet.m_fpMood;
		m_nOutMoodTime = timeGetTime();
	}
}

int CFightPet::GetDamageDecrease(int Damage)
{
	if (m_nTired>50&&m_nTired<=80)
	{
		Damage *=  20/100;
	}
	else if (m_nTired>80&&m_nTired<=90)
	{
		Damage *=  40/100;
	}
	else if(m_nTired<=100)
	{
		Damage *=  60/100;
	}

	return Damage;
}

int CFightPet::GetDamageChange( int Damage )
{
	if (m_nMood>0&&m_nMood<20)
	{
		Damage *= 10/100;
	}
	else if (m_nMood>=20&&m_nMood<40)
	{
		Damage *= 5/100;
	}
	else if (m_nMood>60&&m_nMood<80)
	{
		Damage *= 5/100;
		Damage = -Damage;
	}
	else if (m_nMood>=80&&m_nMood<=MAX_Mood)
	{
		Damage *= 10/100;
		Damage = -Damage;
	}
	return Damage;
}


WORD CFightPet::GetSkillNum()
{
	if (!m_owner)
	{
		rfalse(4,1,"CFightPet::GetSkillNum()1");
		return 0;
	}
	//SFightPetExt *pet = m_owner->_fpGetFightPetByIndex(m_index);
	SNewPetData *pet = &m_owner->m_Property.m_NewPetData[m_index];
	if (!pet)
	{
		rfalse(4,1,"CFightPet::GetSkillNum()2");
		return 0;
	}
	MY_ASSERT(pet);
	WORD num = 0;
	for (int i = 0; i < 10; i++)
	{
		//if (pet->m_CurUseSkill[i] == 0)
		//	continue;
		//if (pet->m_PetSkill[pet->m_CurUseSkill[i]].m_PetSkillID && pet->m_PetSkill[pet->m_CurUseSkill[i]].m_Level)

		if (pet->m_PetSkill[i].m_PetSkillID == 0)
			continue;
		if (pet->m_PetSkill[i].m_PetSkillID && pet->m_PetSkill[i].m_Level)
			num++;
	}	
	return num;
}

WORD CFightPet::GetRandskillIndex()
{
	return (WORD)CRandom::RandRange(0,GetSkillNum()-1);
}

void CFightPet::ChangeAttType( BYTE type )
{
	BYTE tempatttype = m_bAttType;
	m_bAttType = type>=FP_ACTIVELY&&type<FP_MAX ? type : m_bAttType;
	if (m_bAttType == FP_FOLLOW&&tempatttype!=FP_FOLLOW)AtkFinished();
	

}

void CFightPet::UpdateMyProperties()
{
	if (!m_owner)
	{
		rfalse(4,1,"CFightPet::UpdateMyProperties()1");
		return;
	}
	SFightPetExt &pPet = m_owner->m_Property.m_FightPets[m_index];
	if (0 == pPet.m_fpID)
		return;

	const SFightPetBaseData *pFPData = CFightPetService::GetInstance().GetFightPetBaseData(pPet.m_fpID);
	if (!pFPData)
	{
		rfalse(4,1,"CFightPet::UpdateMyProperties()2");
		return;
	}
	MY_ASSERT(pFPData);


	WORD BaseMaxHp = pFPData->m_Attri[FPA_MAXHP][0];
	WORD BaseMaxMp	= pFPData->m_Attri[FPA_MAXMP][0];
	WORD BaseGongJi	= pFPData->m_Attri[FPA_GONGJI][0];
	WORD BaseFangYu	= pFPData->m_Attri[FPA_FANGYU][0];

	m_MaxHp=(m_Level-1)*pPet.m_fpHpIncRadio+BaseMaxHp;
	m_MaxMp=(m_Level-1)*pPet.m_fpMpIncRadio+BaseMaxMp;
	m_GongJi=(m_Level-1)*pPet.m_fpGongJi+BaseGongJi;
	m_FangYu=(m_Level-1)*pPet.m_fpFangYu+BaseFangYu;

	m_CurHp=m_MaxHp;
	m_CurMp=m_MaxMp;

	///更新侠客数据
	pPet.m_fpExp=m_nCurExp;
	pPet.m_nMaxHp=m_MaxHp;
	pPet.m_nCurHp=m_CurHp;
	pPet.m_nMaxMp=m_MaxMp;
	pPet.m_nCurMp=m_CurMp;
	pPet.m_nGongJi=m_GongJi;
	pPet.m_nFangYu=m_FangYu;
	pPet.m_fpLevel=m_Level;

	//发送侠客属性变化消息
	SAFightPetPropertyChange PropertyMsg;
	PropertyMsg.m_MaxHp=m_MaxHp;
	PropertyMsg.m_MaxMp=m_MaxMp;
	PropertyMsg.m_GongJi=m_GongJi;
	PropertyMsg.m_FangYu=m_FangYu;
	PropertyMsg.m_BaoJi = m_BaoJi;
	PropertyMsg.m_ShanBi = m_ShanBi;
	PropertyMsg.m_CurHp=m_CurHp;
	PropertyMsg.m_CurMp=m_CurMp;
	PropertyMsg.m_index=m_index;
	g_StoreMessage(m_owner->m_ClientIndex, &PropertyMsg, sizeof(SAFightPetPropertyChange));
}

void CFightPet::UpdateProperties()
{
	for (size_t i=0; i<SAfpfightPropertyChange::FP_MAXPROPERTY-1; i++)
	{
		if (m_FightPropertyStatus[i])
		{
			BYTE type = i+SAfpfightPropertyChange::FP_GONGJI;
			WORD *attData = reinterpret_cast<WORD *>(m_AttriRefence[i]);
			if (m_owner)
			{
				m_owner->_fpUpdateProperties(type,*attData,m_index);
			}
		}
	}
	return;

	if (m_FightPropertyStatus[XA_CUR_MP]){
		m_owner->_fpUpdateProperties(SAfpfightPropertyChange::FP_CUR_MP,m_CurMp,m_index);
	}

	if (m_FightPropertyStatus[XA_CUR_HP]){
		m_owner->_fpUpdateProperties(SAfpfightPropertyChange::FP_CUR_HP,m_CurHp,m_index);
	}

	if (m_FightPropertyStatus[XA_MAX_HP]){
		m_owner->_fpUpdateProperties(SAfpfightPropertyChange::FP_MAX_HP,m_MaxHp,m_index);
	}

	if (m_FightPropertyStatus[XA_MAX_MP]){
		m_owner->_fpUpdateProperties(SAfpfightPropertyChange::FP_MAX_MP,m_MaxMp,m_index);
	}

	if (m_FightPropertyStatus[XA_GONGJI]){
		m_owner->_fpUpdateProperties(SAfpfightPropertyChange::FP_GONGJI,m_GongJi,m_index);
	}

	if (m_FightPropertyStatus[XA_FANGYU]){
		m_owner->_fpUpdateProperties(SAfpfightPropertyChange::FP_FANGYU,m_FangYu,m_index);
	}

	if (m_FightPropertyStatus[XA_BAOJI]){
		m_owner->_fpUpdateProperties(SAfpfightPropertyChange::FP_BAOJI,m_BaoJi,m_index);
	}

	if (m_FightPropertyStatus[XA_SHANBI]){
		m_owner->_fpUpdateProperties(SAfpfightPropertyChange::FP_SHANBI,m_ShanBi,m_index);
	}
	
	return;
	SAallpropertychange apropertychange;
	apropertychange.messagesize = 0;
	apropertychange.m_type = 0;
	BYTE *buff = apropertychange.valueBuff;

	for (size_t i=0; i<SAallpropertychange::PROPERTYNUM; i++)
	{
		if ( m_FightPropertyStatus[i])
		{
			//				rfalse(2, 1, "【%s】 ", AttriName[i]);
			WORD attSize;
			WORD attType = GetAttriType(static_cast<XYD3_ATTRI>(i), attSize);
			if (XAT_MAX == attType)
				continue;

			// 溢出了
			if ((apropertychange.messagesize + attSize ) > SAallpropertychange::BUFFSIZE)
				break;

			apropertychange.m_type |= fightproperty[i];

			BYTE *attData = reinterpret_cast<BYTE *>(m_AttriRefence[i]);
			if (!attData)
				continue;

			memcpy(buff, attData, attSize);
			buff += attSize;

			apropertychange.messagesize += attSize;
		}
	}

	g_StoreMessage(m_owner->m_ClientIndex,&apropertychange,sizeof(SAallpropertychange)-(SAallpropertychange::BUFFSIZE-apropertychange.messagesize));


}

bool CFightPet::ConsumeMP( int consume )
{
	ModifyCurrentMP(-m_AtkContext.m_consumeMP, 0);
	return true;
}

SCC_RESULT CFightPet::CheckConsume( INT32 skillIndex, int& consumeHP, int& consumeMP )
{
	if (skillIndex < 0 || skillIndex >= 10)return SCCR_INVALID_SKILL_INDEX;

	int skillid = GetSkillIDBySkillIndex(skillIndex);

	const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(skillid);
	if (!pData)return SCCR_INVALID_SKILL_INDEX;
	
	consumeMP = pData->m_ConsumeMP;
	if (m_CurMp < consumeMP)return SCCR_MP_NOT_ENOUGH;

	consumeHP = pData->m_ConsumeHP; 
	if(m_CurHp<=consumeHP)return SCCR_HP_NOT_ENOUGH;

	return SCCR_OK;
	
}

bool CFightPet::CheckAddBuff( const SBuffBaseData *pBuff )
{
	return true;
}

void CFightPet::OnCallBackMe()
{
	//if (!m_owner)
	//{
	//	rfalse(4,1,"CFightPet::OnCallBackMe()");
	//	return;
	//}
	////SFightPetExt *pext = m_owner->_fpGetFightPetByIndex(m_index);
	SNewPetData *pext = &m_owner->m_Property.m_NewPetData[m_index];
	if(!pext)
		return;
	pext->m_CurPetHp = m_CurHp;
	pext->m_CurPetMp = m_CurMp;
}


