#pragma once

#include "scriptobject.h"
#include "networkmodule/fightmsgs.h"
#include "BuffManager.h"
#include "CooldownMgr.h"
#include "BuffSys.h"
#include "CDManager.h"
#include "networkmodule/UpgradeMsgs.h"
#include "CSkillDefine.h"
#include "UpdatePropertyEx.h"
#include "BulletMgr.h"
#include <set>
#include <d3dx9math.h>

const __int32 IID_FIGHTOBJECT = 0x117c95ba;

enum
{
	SKILL_ATTACK_DELAY,
	SKILL_ATTACK_CONTINUE_ATTACK
};

enum SCC_RESULT
{
	SCCR_OK,
	SCCR_INVALID_SKILL_INDEX,
	SCCR_INVALID_SKILL_TYPE,
	SCCR_HP_NOT_ENOUGH,
	SCCR_MP_NOT_ENOUGH,
	SCCR_SP_NOT_ENOUGH,
};

const int PLAYER_BASIC_ATKSPEED = 100;

extern int g_dir[];

#define GET_RAND_MN(min, max) (min + rand() %(max-min+1))

// 这里为特殊处理，记录本场景中被额外引用的对象个数
extern void UpdateRegionAIRefCount(class CRegion *pRegion, int iValue);

class CMonster;
class CFightObject;

class CFightListener
{
public:
	CFightListener() { __pSubject = 0; }
	virtual ~CFightListener() {}
	virtual void CALLBACK OnExchange(CFightObject* pObj) = 0;
	virtual void CALLBACK SetSubject(CFightObject* pSubject) { __pSubject = pSubject; }
	virtual void CALLBACK OnClose(CFightObject* pObj){}
	CFightObject *GetSubject() { return __pSubject; }

private:
	CFightObject* __pSubject;
};

// 战斗使用对象
class CFightObject : public CScriptObject
{
	friend class CPlayerManager;

public:
	DWORD m_dwLastEnemyID;    // 最后一个攻击自己的对手的ID

protected:
	int OnCreate(_W64 long pParameter);
	void OnRun();

	BOOL EndPrevAction();   // 上一个动作结束时的回调
	BOOL SetCurAction();    // 根据备份数据设置当前的动作
	BOOL DoCurAction();     // 执行当前的动作

public:
	CFightObject(void);
	~CFightObject(void);

public:
	void *m_AttriRefence[XA_MAX_EXP];		// 内部属性引用，在兼容现有机制条件下尽可能提高效率
	bool m_FightPropertyStatus[XA_MAX_EXP];	// 侠义道3战斗属性更新机制
	bool m_IsFightUpdated;					// 是否有更新
	
	// 对象属性
	DWORD		m_MaxHp;				// 最大生命
	DWORD		m_MaxMp;				// 最大内力
	DWORD		m_MaxTp;				// 最大体力
	DWORD		m_CurMp;				// 当前生命
	DWORD		m_CurHp;				// 当前内力
	DWORD		m_CurTp;				// 当前体力
	WORD		m_GongJi;				// 攻击
	WORD		m_FangYu;				// 防御
	WORD		m_BaoJi;				// 暴击
	WORD		m_Hit;					//命中
	WORD		m_ShanBi;				// 闪避
	WORD		m_Level;				// 等级
	WORD		m_SorbDamageValue;		// 伤害吸收数值
	WORD		m_SorbDamagePercent;	// 伤害吸收百分比
	float		m_AtkSpeed;				// 原始攻击速度
	WORD		m_CurAtkSpeed;			// 当前攻击速度ahj
	WORD		m_HateValue;			// 附加仇恨值

//月下夜想曲 新增
	WORD m_newAddproperty[SEquipDataEx::EEA_MAXTP - SEquipDataEx::EEA_UNCRIT];
// 	WORD m_uncrit; //抗暴
// 	WORD m_wreck;//破击
// 	WORD m_unwreck;	//抗破
// 	WORD m_puncture;	//穿刺
// 	WORD m_unpuncture;	//抗穿
// 	WORD m_iceatk;	//冰伤
// 	WORD m_icedef;	//冰抗
// 	WORD m_fireatk;	//火伤
// 	WORD m_firedef;		//火抗
// 	WORD m_darkatk;		//暗伤
// 	WORD m_darkdef;		//暗抗
// 	WORD m_holyatk;		//神圣伤
// 	WORD m_holydef;		//神圣抗
// 	WORD m_hppercent;	//血量百分比加成
// 	WORD m_mppercent;	//精力百分比加成
// 	WORD m_atkpercent;	//伤害百分比加成
// 	WORD m_defpercent;	//防御百分比加成
// 	WORD m_critpercent;	//暴击百分比加成
// 	WORD m_weakpercent;		//破击百分比加成
// 	WORD m_hitpercent;		//命中百分比加成


	// 新添加的属性 an
	WORD		m_ReduceDamage;			// 伤害减免
	WORD		m_AbsDamage;			// 绝对伤害
	WORD		m_NonFangyu;			// 无视防御
	WORD		m_MultiBaoJi;			// 暴击倍数
	WORD		m_ExtraExpPercent;		// 额外经验加成

	WORD		m_IceDamage;			// 冰伤害
	WORD		m_IceDefence;			// 冰抗性
	WORD		m_FireDamage;			// 火伤害
	WORD		m_FireDefence;			// 火抗性
	WORD		m_XuanDamage;			// 玄伤害
	WORD		m_XuanDefence;			// 玄抗性
	WORD		m_PoisonDamage;			// 毒伤害
	WORD		m_PoisonDefence;		// 毒抗性

	// Buff相关
	BuffManager	m_buffMgr;									// Buff管理器
	INT32 m_BuffModifyPot[BAP_ATTRI_MAX-BAP_MHP];			// 以值方式改变的属性
	INT32 m_BuffModifyPercent[BAP_ATTRI_MAX-BAP_MHP];		// 以百分比方式改变的属性
	//INT32 m_ResistValue[BAP_RESIST_MAX-BAP_RESIST_CHP];	// 抗性的影响
	DWORD m_fightState;										// 战斗状态（眩晕etc...）

	// 冷却管理器
	CDManager	m_CDMgr;			

	// 发射出去的子弹和追击我的子弹
	std::set<DWORD> m_FiredToOther;
	std::set<DWORD> m_FiredToMe;

	// 战斗状态的判定
	typedef std::map<DWORD, DWORD> AtkerList;

	bool		m_IsInFight;		// 是否在战斗状态中
	AtkerList	m_Attacker;			// 攻击者的列表

	struct SkillModifyDest			//冲锋类技能，有可能需要客户端返回修正以后的目标点
	{
		bool			m_bModifyDest;
		float			m_fDestPosX;
		float			m_fDestPosY;
	};

	// 这里缓存了技能攻击的情况下的被攻击者的数据信息
	struct SkillAttackDataStruct : SQuestSkill_C2S_MsgBody
	{
		// 快速引用，小小优化
		SSkillBaseData				m_SkillData;	// 当前技能的基本属性
		std::list<CFightObject *>	m_TargetList;	// 攻击的目标列表缓冲

		bool			m_enableNextSkill;			// 是否启动了下一个接替技能
		DWORD			m_SkillAttackStartTime;		// 技能的总体开始时间
		DWORD			m_SkillPhaseStartTime;		// 技能每一个阶段的开始时间
		DWORD			m_SkillAttackSubState;		// 技能状态的子状态
		DWORD			m_consumeHP;				// HP消耗
		DWORD			m_consumeMP;				// MP消耗
		DWORD			m_consumeTP;				// TP消耗

		// 几个阶段所需时间
		BYTE			m_Phase;					// 当前阶段

		DWORD			m_PrepareTime;				// 前期准备阶段
		bool			m_PrepareOver;				// 前期准备结束

		DWORD			m_SingTime;					// 吟唱所需时间
		bool			m_SingOver;					// 吟唱是否完毕

		DWORD			m_BeforeAttack;				// 攻击前奏阶段
		bool			m_BeforeAtkOver;			// 攻击前奏结束

		DWORD			m_StartAttack;				// 上一次造成伤害的时间
		BYTE			m_WholeAtkNumber;			// 总共要攻击几下
		BYTE			m_CurAtkNumber;				// 当前处于第几次伤害中
		bool			m_bIsProcessDamage;			// 是否处理了所有伤害

		DWORD			m_AfterAttack;				// 攻击后阶段
		bool			m_AfterAtkOver;				// 攻击后结束

		bool			m_bSkillAtkOver;			// 整体技能是否完成

		WORD			m_bHitSpot[12];				// 打击点（可以动态修正的）
		WORD			m_CalcDamg[12];				// 是否产生伤害

		SkillModifyDest m_SkillModifyDest;
		D3DXVECTOR2 m_SkillAttackPos;	//技能攻击点，用于位移技能中保存客户端发送过来的自身坐标
	} m_AtkContext;

//	DWORD		m_EnemyInEye;		// 是否有目标

	SQuestSkill_C2S_MsgBody m_backUpQusetSkillMsg;
	SQuestSkill_C2S_MsgBody m_nextQuestSkillMsg;
	SQuestSkill_C2S_MsgBody m_LuaQuestSkillMsg;
	std::vector<SQuestSkill_C2S_MsgBody> m_vcacheQuestSkill; //缓存的技能
	std::list<CFightObject *> m_ClientdestList;//客户端传过来的群攻目标列表
	std::vector<QuestSkill*>m_ClientQuestskill;//客户端请求群攻技能
	std::vector<QuestSkill*>m_vQuestskill;//技能请求列表
private:
	std::list<CFightListener *> m_listenerList;

public:
	// 伤害计算
	struct SDamage
	{
		SDamage() : wDamage(0), wDamageExtra(0), wIgnoreDefDamage(0), wReboundDamage(0), isCritical(false), mHitState(0)
		{
			//memset(wDamageExtra, 0, sizeof(WORD) * EXTRA_MAX);
		}

		WORD wDamage;							// 普通伤害
		WORD wDamageExtra;						// 额外伤害
		WORD wIgnoreDefDamage;					// 破防伤害
		WORD wReboundDamage;					// 反弹伤害
		bool isCritical;						// 是否暴击
		BYTE	mHitState;			// 攻击类型(0命中，1未命中，2无敌)
	};

	///仇恨列表
	struct SHateList
	{
		DWORD m_nAtkTime;///被攻击时间
		WORD	 m_nHateValue;///仇恨值
		SHateList():m_nAtkTime(0),m_nHateValue(0){}
	};

	// 仇恨列表的判定
	typedef std::map<DWORD, SHateList> HateplayerList;
	HateplayerList hatelist;


	void Kill(CFightObject *pEnemy);

	// 发送/接受消息
	void SendAddPlayerExp(DWORD dwExpVal, BYTE byChangeType, LPCSTR how = NULL);
	void SendAddFightPetExp(DWORD dwExpVal, BYTE byChangeType, LPCSTR how = NULL);
	void SendAddPlayerLevel(WORD byLevelVal, BYTE byEffectIndex, LPCSTR info = NULL);
	void SendFightExtraState(DWORD iExtraState);
	void SendEffectiveMsg(struct SASetEffectMsg &msg);
	BOOL SetAttackMsg(SFightBaseMsg *pMsg);

	// 直接恢复所有当前生命，内力，真气
	void RestoreFullHPDirectly() { m_CurHp = m_MaxHp; m_FightPropertyStatus[XA_CUR_HP] = true; }
	void RestoreFullMPDirectly() { m_CurMp = m_MaxMp; m_FightPropertyStatus[XA_CUR_MP] = true; }
	void RestoreFullTPDirectly() { m_CurTp = m_MaxTp; m_FightPropertyStatus[XA_CUR_TP] = true; }

	// 修改当前生命，内力，体力。内部可能做打断处理。返回实际修改量
	int ModifyCurrentHP(int mod, int type, CFightObject *pReason, bool curInterrupt = true);
	int ModifyCurrentMP(int mod, int type, bool curInterrupt = true);
	int ModifyCurrentTP(int mod, int type, bool curInterrupt = true);

	int GetBuffModifyValue(WORD BuffType,int val);

	virtual bool CheckAddBuff(const SBuffBaseData *pBuff) { return false; };
	virtual void UpdateAllProperties() {}		// 更新属性
	virtual void SendPropertiesUpdate();		// 按需发送更新后的属性

	bool ChangeFightState(DWORD type, bool marked, DWORD pusherID, float fdestX = 0.0f, float fdestY = 0.0f,DWORD dskillIndex = 0);
	bool CheckFightState(DWORD type);
	
     
	// 技能释放相关         
	BOOL ProcessQuestSkill(SQuestSkill_C2S_MsgBody *pMsg);//->OK<-//
	BOOL __ProcessQuestSkill(SQuestSkill_C2S_MsgBody *questMsg, bool skipState, SkillModifyDest *skillModifyDest=NULL,bool checkcold = true);//->OK<-//
	BOOL ProcessQuestSkill(SQuestSkillMultiple_C2S_MsgBody *pMsg);//->OK<-//
	void StartGlobalCDTimer(const SSkillBaseData *pSkillData);
	int CheckColdTimeStep0(const SSkillBaseData *pData);
	bool IsCanProcessSkillAttackMsg(const SSkillBaseData *pData, SQuestSkill_C2S_MsgBody* questMsg, bool skipState,bool checkcold = true);
	bool DamageEnemyBySkillAttack(CFightObject* pDest, SDamage& damage);
	bool IsCanDamageByAttackMoreMethod(CFightObject* desObject);
	bool IsHit(CFightObject *att, CFightObject *def);
	void CalculateDamage(CFightObject *pDest, SDamage& damage);
	void CalculateDamagePVP(CFightObject *pAtta, CFightObject *pDest, SDamage& damage);
	void CalculateDamagePVE(CFightObject *pAtta, CFightObject *pDest, SDamage& damage);
	void CalculateDamageEVP(CFightObject *pAtta, CFightObject *pDest, SDamage& damage);
	virtual void OnDamage(INT32 *nDamage, CFightObject *pFighter) = 0;
	virtual void OnAttack(CFightObject *pFighter){}
	virtual void OnDead(CFightObject *PKiller);
	
	/************************************************************************/
	/* 冰火玄毒伤害相关  add by an                                                                      */
	/************************************************************************/
	void GetExtraDamage(CFightObject *pDest);
	void GetNunberFromString(string s, WORD ExtraDamageArr[]);

	bool AddToHatelist(CFightObject *pFighter);				// 添加并更新仇恨列表时间
	void RemoveFromHatelistOnTime();						// 移除玩家仇恨列表
	void RemoveHatelistFromID(DWORD gid);
	void RemoveAllHatelist();
	bool IsInHatelist(DWORD gid);							// 是否在仇恨列表中
	BYTE IsPlayerOrFightPet(CFightObject *pFighter);		// 是否是玩家或者侠客 1玩家2 侠客
	CPlayer* GetFightObjToPlayer(CFightObject *pFighter);	// 把一个战斗对象转换为玩家，只有当战斗对象是玩家或者侠客是才有效
	//bool CheckPkRule(DWORD gid);							// 检测PK模式，判断是否在仇恨列表中

	void CheckStopBullet(WORD type);						// 检查失效子弹
	void DamageObject_Single_Bullet(Bullet *pBullet);		// 单攻（子弹）
	void DamageObject_Single();								// 单攻（近程，远程）
	void DamageObject_AttackMore(bool self);				// 群攻（自身，远程）
	void	MoveObiect_AttackMore(CFightObject *pDest,BYTE bSkillPropertyType, float fmoveX, float fmoveY);
	void DamageObject_AttackMoreForMonster(bool self);				// 群攻（自身，远程）
	bool GetDestList(std::list<CFightObject *> *destList,bool self);

	void GetAtkObjectByList(check_list<LPIObject> *objectList, std::list<CFightObject *> *destList, INT32 *pMonsterNumber, WORD atkType, INT32 MaxAttack, bool bischeckAttack = true);

	// 单攻造成的伤害的消息
	INT32 SendDamageMessageBySkillAttack_Normal(CFightObject* pDest,SDamage& damage);
	INT32 InitWoundMsg_Normal(SAWoundObjectMsgEx* woundMsg);

	// 重载的发送攻击伤害值接口，可以根据不同的【伤害类型】发送具体的消息
	INT32 SendDamageMessageBySkillAttack_Normal(CFightObject* pDest, SDamage& damage, BYTE DamageType);

	INT32 InitWoundTiny(SWoundTiny *wound, SDamage& damage, INT32 hitState, float DefenserX, float DefenserY);
	void SendAttackMissMsg(INT32 state);
	

	void ProcessHitPot();							// 打击点计算
	void IntoFightState(CFightObject *pDefencer);	// 切换进入战斗状态
	virtual void SwitchFightState(bool IsIn){}		// 进入/离开战斗状态回调
	
	bool CheckAttackByClient(const SSkillBaseData *pData, SQuestSkill_C2S_MsgBody *questMsg);
	void StartWindMove(SFightBaseMsg *pMsg);

	void OnRecvCheckMove(SQWinMoveCheckMsg *pMsg);
	void SetMovePos(float fx,float fy);//移动到坐标
	virtual float GetWalkSpeed();

	// 子状态的处理函数
	void SkillAttackStart(const SSkillHitSpot *pHitPot);// 开始
	void SkillAttackDelay();	// 更新
	void SkillAttackEnd();		// 结束
	void UpdateSkillAttack();	// 更新
	bool ContinueSkillAttack();	// 连续

	virtual void AtkFinished() {}

	virtual INT32 GetCurrentSkillLevel();
	virtual INT32 GetSkillIDBySkillIndex(INT32 index);

	virtual bool SendMove2TargetForAttackMsg(INT32 skillDistance,INT32 skillIndex,CFightObject* pLife);

	void SendSwitch2SkillAttackStateMsg();							// 发送消息让客户端切换到技能攻击状态
	void SendSwitch2PrepareSkillAttackStateMsg(int preTime = 0);	// 发送消息让客户端切换到技能准备状态
	
	BOOL ActivaSkill(WORD wSkillID,LPCSTR info);
	short ActiveTempSkill(WORD wSkillID, BYTE Level = 1);
	void CoolingTempSkill(WORD index);
	bool IsSkillExisted(CPlayer* pPlayer,WORD wSkillID);
	WORD IsTelegryExisted(CPlayer *pPlayer, WORD wID);
	INT32 GetFreeSkillSlot(CPlayer* pPlayer,INT32 startSlot);
	INT32 GetSkillSlot(CPlayer* pPlayer,WORD wSkillID);
	INT32 GetFreeTelergySlot(CPlayer* pPlayer,INT32 startSlot);

	virtual bool ConsumeHP(int consume);	// 生命消耗
	virtual bool ConsumeMP(int consume);	// 内力消耗
	virtual bool ConsumeTP(int consume);	// 体力消耗
	virtual SCC_RESULT CheckConsume(INT32 skillIndex, int& consumeHP, int& consumeMP);
	
	void CheckDelBuffSpot(DWORD delFlag);

	void SetListener(CFightListener* pListener);
	void RemoveListener(CFightListener* pListener);
	void RemoveAllListener();
	float GetSkillAttkFactor(SSkillBaseData *skill);
	BOOL IfAvoidAttack(WORD MyShanbi, WORD OtherShanBi);	// 是否闪避了
	float ConsumeBaoJi(WORD MyBaoJi, WORD OtherBaoJi);		// 暴击率的计算
	float ConsumeShanBi(WORD MyShanBi, WORD OtherShanBi);	// 闪避率计算
	float DeltaLevelRate(WORD highLevel, WORD lowLevel);	// 等级差计算
	float ConsumeFinalDamageValue(CFightObject *pAtta, CFightObject *pDest);	// 攻击伤害计算
	BOOL ConsumeFinalDamageValue(CFightObject *pAtta, CFightObject *pDest,SDamage &sdamage);	// 攻击伤害计算

	BOOL ActiveSecondarySkill(CFightObject* pAttack, CFightObject* pDest, WORD DamageValue);		// 触发辅助被动技能

	virtual const char *getname();///得到名称

	//-------侠客技能学习相关------//
	BOOL _fpActivaSkill(WORD wSkillID,BYTE index,LPCSTR info,BYTE type = SQfpSkillUpdate::SSU_LEARNED);//侠客学习技能
	BOOL _fpActivaSkill(WORD wSkillID,BYTE bskillLevel,BYTE index,LPCSTR info,BYTE type = SQfpSkillUpdate::SSU_LEARNED);//侠客学习技能

	BOOL _fpActivaSkillByItem(DWORD ditemID,BYTE index,LPCSTR info,BYTE type = SQfpSkillUpdate::SSU_LEARNED);//侠客学习技能

	INT32 _fpGetFreeSkillSlot(BYTE index);	//空技能孔
	bool	_fpGetSkillEnableLearn(WORD wSkillID,BYTE index);	///技能是否能学习
	WORD	m_IsCrilial;	//是否产生暴击
	//bool	m_IsExtraDamage;

	//特殊的连续技能处理
	void SetQusetSkillMsg(SQuestSkill_C2S_MsgBody *pmsg);
	bool GetcontinueSkill(SQuestSkill_C2S_MsgBody *pmsg);
	int GetContinueSkillTarget(SQuestSkill_C2S_MsgBody &pmsg);//获取下一个目标
	BOOL verifySkillMsg(SQuestSkill_C2S_MsgBody *questMsg);//验证技能消息的正确性
	void AddQuestToCache(SQuestSkill_C2S_MsgBody *questMsg);//加入技能消息到缓存
	bool GetCacheSkill(SQuestSkill_C2S_MsgBody &pmsg);//获取缓存的技能
	void LuaSetCacheSkill(SQuestSkill_C2S_MsgBody *pmsg);//从lua中设置缓存的技能
	void SetDamage(const SDamage *pDamage);
	WORD GetSkillDamageRate(WORD skillIndexm,WORD wlevel);//获取技能攻击伤害比例

	//同步移动问题
	void MoveInArea(CFightObject* pfightobj, float moveposX, float moveposY, DNID dnidExcept = -1);
	void PutIntoRegion(LPIObject pP, WORD wRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID,  DNID dnidExcept);


	//追击相关
	void AddToFollow(CFightObject *pfight);
	void RemoveFollow(CFightObject *pfight);
	void ClearFollow();//死亡时清除

	//add by ly 2014/4/4 获取玩家对象的战斗力
	DWORD GetPlayerFightPower();
	
public:
	SDamage	m_sdamage; //技能伤害数据
	//std::vector<CFightObject*> m_followObject;//我的追击者列表
	std::map<DWORD, CFightObject*>m_followObject;//我的追击者列表
	CFightObject* m_pFightFollow;//当前正在追击的目标
protected:
	void NotifyExchange();

public:
		///获取当前动态场景ID，为0是普通场景
	virtual 	DWORD GetDynamicRegionID();
	virtual 	DWORD GetSceneRegionID(); //关卡ID
};