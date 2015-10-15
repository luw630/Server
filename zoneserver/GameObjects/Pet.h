#pragma once

#include "FightObject.h"

// 宠物的IID
const __int32 IID_FIGHT_PET = 0x117cb501;
#define MINUTESTIME(x)	(x)*60*1000

enum FightPetEnemyType
{
	FPET_NONE,			// 无类型
	FPET_PLAYERATK,		// 玩家正在攻击的
	FPET_ATKPLAYER,		// 攻击玩家的
	FPET_ATKME,			// 攻击我的
	FPET_MAX,
};

class CFightPet : public CFightObject
{
public:
	IObject *VDC(const __int32 IID)
	{
		if (IID == IID_ACTIVEOBJECT || IID == IID_FIGHTOBJECT)
			return this;

		return 0;
	}

public:
	CFightPet();
	~CFightPet();

	void OnClose();
	void OnRun();
	void OnRunEnd();

protected:
	int OnCreate(_W64 long pParameter);

public:
	struct SASynFightPetMsg *GetStateMsg();

public:
	void UpdateAllProperties();		// 更新属性------------------------------
	void SendPropertiesUpdate();	// 按需发送更新后的属性-----------------------------
	void UpdateMyProperties();		// 更新所有侠客属性属性-------------------------------

	void OnDamage(INT32 *nDamage, CFightObject *pFighter);
	void OnDead(CFightObject *PKiller);

	void AtkFinished();
	virtual INT32 GetCurrentSkillLevel(DWORD dwSkillIndex);
	virtual INT32 GetSkillIDBySkillIndex(INT32 index);
	WORD GetSkillNum();//技能个数
	WORD GetRandskillIndex();//得到一个随机的技能ID

	virtual bool SendMove2TargetForAttackMsg(INT32 skillDistance,INT32 skillIndex,CFightObject* pLife);
	virtual bool CheckAddBuff(const SBuffBaseData *pBuff);

	bool ConsumeHP(int consume){ return true; }		// 生命消耗
	bool ConsumeMP(int consume);	// 内力消耗
	SCC_RESULT CheckConsume(INT32 skillIndex, int& consumeHP, int& consumeMP);

public:
	inline void SetCurEnemy(DWORD enemyID, BYTE type);
	void SwithcStatus(bool PlayerContrl);

public:
	BOOL DoCurAction();
	void OnChangeState(EActionState newActionID);
	BOOL MoveToPostion(WORD wNewRegionID, float wStartX, float wStartY);//瞬移侠客用于玩家于侠客距离过大时使用
	void OnRecvLevelUp(struct SQLevelChangeMsg *msg);		// 接收到侠客升级消息---------------
	void SendAddLevel(WORD  Uplevel);			//---------------------------------
	const  char *getname();
	void CheckOnTime();///根据出战时间增加疲劳度，心情等-----------------
	int GetDamageDecrease(int Damage); //根据疲劳度取得伤害减少系数----------------------------
	int GetDamageChange(int Damage); //根据心情取得伤害系数-------------------------------
	void ChangeAttType(BYTE type);//更改攻击模式
	void UpdateProperties();//更新一些必须得属性--------------------------
	void OnCallBackMe();//当玩家收回侠客时

public:
	struct SParameter
	{
		CPlayer *owner;		// 主人
		BYTE	index;		// 侠客索引
		float	fX;			// 坐标
		float	fY;			// 
	};

public:
	typedef std::pair<DWORD, BYTE> TargetCache;

	CPlayer		*m_owner;		// 主人
	BYTE		m_index;		// 索引
	BYTE		m_FightType;	// 战斗模式
	TargetCache m_CurEnemy;		// 当前的敌人
	TargetCache	m_TargetCache;	// 目标缓冲
	QWORD	m_nMaxExp;	//升级所需经验
	QWORD	m_nCurExp;	//当前经验
	BYTE			m_nRare;//稀有度
	WORD		m_nRelation;			// 亲密度
	BYTE			m_nTired;				// 疲劳度
	BYTE			m_nMood;				// 心情
	BYTE			m_bSex;					//性别
	BYTE			m_bAttType;				//攻击模式

	DWORD	m_nOutFightTime;	//出战时间
	DWORD	m_nOutMoodTime;	//出战计算心情时间
	DWORD	m_nSearchTargetTime;//主动攻击是搜索目标的时间
	DWORD	m_nChangeSkilltime;//侠客切换技能时间
	

	static const int MAX_TIRED;			//最大疲劳度
	static const int MAX_Mood;			//最大心情值
	static const int MAX_RELATION;	//最高亲密度
	static const int MAX_IMPEL;		//最大激励值

	// 装备属性加成（加值和加百分比）
	int m_EquipFactorValue[SEquipDataEx::EEA_MAX];
	WORD	fightproperty[SAallpropertychange::PROPERTYNUM];
// 	WORD	fightpropertytype;
};

void CFightPet::SetCurEnemy(DWORD enemyID, BYTE type)
{
	if(m_bAttType == FP_FOLLOW)return;

	if (type < FPET_PLAYERATK || type > FPET_ATKME)
	{
		rfalse(4,1,"CFightPet::SetCurEnemy");
		return;
	}
	MY_ASSERT(type >= FPET_PLAYERATK && type <= FPET_ATKME);
	
	// 如果当前没有目标或者优先级高于当前目标，那么替换
	if (0 == m_CurEnemy.first || type <= m_CurEnemy.second)
	{
		m_TargetCache.first	 = enemyID;
		m_TargetCache.second = type;
	}

	return;
}