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

// ����Ϊ���⴦����¼�������б��������õĶ������
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

// ս��ʹ�ö���
class CFightObject : public CScriptObject
{
	friend class CPlayerManager;

public:
	DWORD m_dwLastEnemyID;    // ���һ�������Լ��Ķ��ֵ�ID

protected:
	int OnCreate(_W64 long pParameter);
	void OnRun();

	BOOL EndPrevAction();   // ��һ����������ʱ�Ļص�
	BOOL SetCurAction();    // ���ݱ����������õ�ǰ�Ķ���
	BOOL DoCurAction();     // ִ�е�ǰ�Ķ���

public:
	CFightObject(void);
	~CFightObject(void);

public:
	void *m_AttriRefence[XA_MAX_EXP];		// �ڲ��������ã��ڼ������л��������¾��������Ч��
	bool m_FightPropertyStatus[XA_MAX_EXP];	// �����3ս�����Ը��»���
	bool m_IsFightUpdated;					// �Ƿ��и���
	
	// ��������
	DWORD		m_MaxHp;				// �������
	DWORD		m_MaxMp;				// �������
	DWORD		m_MaxTp;				// �������
	DWORD		m_CurMp;				// ��ǰ����
	DWORD		m_CurHp;				// ��ǰ����
	DWORD		m_CurTp;				// ��ǰ����
	WORD		m_GongJi;				// ����
	WORD		m_FangYu;				// ����
	WORD		m_BaoJi;				// ����
	WORD		m_Hit;					//����
	WORD		m_ShanBi;				// ����
	WORD		m_Level;				// �ȼ�
	WORD		m_SorbDamageValue;		// �˺�������ֵ
	WORD		m_SorbDamagePercent;	// �˺����հٷֱ�
	float		m_AtkSpeed;				// ԭʼ�����ٶ�
	WORD		m_CurAtkSpeed;			// ��ǰ�����ٶ�ahj
	WORD		m_HateValue;			// ���ӳ��ֵ

//����ҹ���� ����
	WORD m_newAddproperty[SEquipDataEx::EEA_MAXTP - SEquipDataEx::EEA_UNCRIT];
// 	WORD m_uncrit; //����
// 	WORD m_wreck;//�ƻ�
// 	WORD m_unwreck;	//����
// 	WORD m_puncture;	//����
// 	WORD m_unpuncture;	//����
// 	WORD m_iceatk;	//����
// 	WORD m_icedef;	//����
// 	WORD m_fireatk;	//����
// 	WORD m_firedef;		//��
// 	WORD m_darkatk;		//����
// 	WORD m_darkdef;		//����
// 	WORD m_holyatk;		//��ʥ��
// 	WORD m_holydef;		//��ʥ��
// 	WORD m_hppercent;	//Ѫ���ٷֱȼӳ�
// 	WORD m_mppercent;	//�����ٷֱȼӳ�
// 	WORD m_atkpercent;	//�˺��ٷֱȼӳ�
// 	WORD m_defpercent;	//�����ٷֱȼӳ�
// 	WORD m_critpercent;	//�����ٷֱȼӳ�
// 	WORD m_weakpercent;		//�ƻ��ٷֱȼӳ�
// 	WORD m_hitpercent;		//���аٷֱȼӳ�


	// ����ӵ����� an
	WORD		m_ReduceDamage;			// �˺�����
	WORD		m_AbsDamage;			// �����˺�
	WORD		m_NonFangyu;			// ���ӷ���
	WORD		m_MultiBaoJi;			// ��������
	WORD		m_ExtraExpPercent;		// ���⾭��ӳ�

	WORD		m_IceDamage;			// ���˺�
	WORD		m_IceDefence;			// ������
	WORD		m_FireDamage;			// ���˺�
	WORD		m_FireDefence;			// ����
	WORD		m_XuanDamage;			// ���˺�
	WORD		m_XuanDefence;			// ������
	WORD		m_PoisonDamage;			// ���˺�
	WORD		m_PoisonDefence;		// ������

	// Buff���
	BuffManager	m_buffMgr;									// Buff������
	INT32 m_BuffModifyPot[BAP_ATTRI_MAX-BAP_MHP];			// ��ֵ��ʽ�ı������
	INT32 m_BuffModifyPercent[BAP_ATTRI_MAX-BAP_MHP];		// �԰ٷֱȷ�ʽ�ı������
	//INT32 m_ResistValue[BAP_RESIST_MAX-BAP_RESIST_CHP];	// ���Ե�Ӱ��
	DWORD m_fightState;										// ս��״̬��ѣ��etc...��

	// ��ȴ������
	CDManager	m_CDMgr;			

	// �����ȥ���ӵ���׷���ҵ��ӵ�
	std::set<DWORD> m_FiredToOther;
	std::set<DWORD> m_FiredToMe;

	// ս��״̬���ж�
	typedef std::map<DWORD, DWORD> AtkerList;

	bool		m_IsInFight;		// �Ƿ���ս��״̬��
	AtkerList	m_Attacker;			// �����ߵ��б�

	struct SkillModifyDest			//����༼�ܣ��п�����Ҫ�ͻ��˷��������Ժ��Ŀ���
	{
		bool			m_bModifyDest;
		float			m_fDestPosX;
		float			m_fDestPosY;
	};

	// ���ﻺ���˼��ܹ���������µı������ߵ�������Ϣ
	struct SkillAttackDataStruct : SQuestSkill_C2S_MsgBody
	{
		// �������ã�СС�Ż�
		SSkillBaseData				m_SkillData;	// ��ǰ���ܵĻ�������
		std::list<CFightObject *>	m_TargetList;	// ������Ŀ���б���

		bool			m_enableNextSkill;			// �Ƿ���������һ�����漼��
		DWORD			m_SkillAttackStartTime;		// ���ܵ����忪ʼʱ��
		DWORD			m_SkillPhaseStartTime;		// ����ÿһ���׶εĿ�ʼʱ��
		DWORD			m_SkillAttackSubState;		// ����״̬����״̬
		DWORD			m_consumeHP;				// HP����
		DWORD			m_consumeMP;				// MP����
		DWORD			m_consumeTP;				// TP����

		// �����׶�����ʱ��
		BYTE			m_Phase;					// ��ǰ�׶�

		DWORD			m_PrepareTime;				// ǰ��׼���׶�
		bool			m_PrepareOver;				// ǰ��׼������

		DWORD			m_SingTime;					// ��������ʱ��
		bool			m_SingOver;					// �����Ƿ����

		DWORD			m_BeforeAttack;				// ����ǰ��׶�
		bool			m_BeforeAtkOver;			// ����ǰ�����

		DWORD			m_StartAttack;				// ��һ������˺���ʱ��
		BYTE			m_WholeAtkNumber;			// �ܹ�Ҫ��������
		BYTE			m_CurAtkNumber;				// ��ǰ���ڵڼ����˺���
		bool			m_bIsProcessDamage;			// �Ƿ����������˺�

		DWORD			m_AfterAttack;				// ������׶�
		bool			m_AfterAtkOver;				// ���������

		bool			m_bSkillAtkOver;			// ���弼���Ƿ����

		WORD			m_bHitSpot[12];				// ����㣨���Զ�̬�����ģ�
		WORD			m_CalcDamg[12];				// �Ƿ�����˺�

		SkillModifyDest m_SkillModifyDest;
		D3DXVECTOR2 m_SkillAttackPos;	//���ܹ����㣬����λ�Ƽ����б���ͻ��˷��͹�������������
	} m_AtkContext;

//	DWORD		m_EnemyInEye;		// �Ƿ���Ŀ��

	SQuestSkill_C2S_MsgBody m_backUpQusetSkillMsg;
	SQuestSkill_C2S_MsgBody m_nextQuestSkillMsg;
	SQuestSkill_C2S_MsgBody m_LuaQuestSkillMsg;
	std::vector<SQuestSkill_C2S_MsgBody> m_vcacheQuestSkill; //����ļ���
	std::list<CFightObject *> m_ClientdestList;//�ͻ��˴�������Ⱥ��Ŀ���б�
	std::vector<QuestSkill*>m_ClientQuestskill;//�ͻ�������Ⱥ������
	std::vector<QuestSkill*>m_vQuestskill;//���������б�
private:
	std::list<CFightListener *> m_listenerList;

public:
	// �˺�����
	struct SDamage
	{
		SDamage() : wDamage(0), wDamageExtra(0), wIgnoreDefDamage(0), wReboundDamage(0), isCritical(false), mHitState(0)
		{
			//memset(wDamageExtra, 0, sizeof(WORD) * EXTRA_MAX);
		}

		WORD wDamage;							// ��ͨ�˺�
		WORD wDamageExtra;						// �����˺�
		WORD wIgnoreDefDamage;					// �Ʒ��˺�
		WORD wReboundDamage;					// �����˺�
		bool isCritical;						// �Ƿ񱩻�
		BYTE	mHitState;			// ��������(0���У�1δ���У�2�޵�)
	};

	///����б�
	struct SHateList
	{
		DWORD m_nAtkTime;///������ʱ��
		WORD	 m_nHateValue;///���ֵ
		SHateList():m_nAtkTime(0),m_nHateValue(0){}
	};

	// ����б���ж�
	typedef std::map<DWORD, SHateList> HateplayerList;
	HateplayerList hatelist;


	void Kill(CFightObject *pEnemy);

	// ����/������Ϣ
	void SendAddPlayerExp(DWORD dwExpVal, BYTE byChangeType, LPCSTR how = NULL);
	void SendAddFightPetExp(DWORD dwExpVal, BYTE byChangeType, LPCSTR how = NULL);
	void SendAddPlayerLevel(WORD byLevelVal, BYTE byEffectIndex, LPCSTR info = NULL);
	void SendFightExtraState(DWORD iExtraState);
	void SendEffectiveMsg(struct SASetEffectMsg &msg);
	BOOL SetAttackMsg(SFightBaseMsg *pMsg);

	// ֱ�ӻָ����е�ǰ����������������
	void RestoreFullHPDirectly() { m_CurHp = m_MaxHp; m_FightPropertyStatus[XA_CUR_HP] = true; }
	void RestoreFullMPDirectly() { m_CurMp = m_MaxMp; m_FightPropertyStatus[XA_CUR_MP] = true; }
	void RestoreFullTPDirectly() { m_CurTp = m_MaxTp; m_FightPropertyStatus[XA_CUR_TP] = true; }

	// �޸ĵ�ǰ�������������������ڲ���������ϴ�������ʵ���޸���
	int ModifyCurrentHP(int mod, int type, CFightObject *pReason, bool curInterrupt = true);
	int ModifyCurrentMP(int mod, int type, bool curInterrupt = true);
	int ModifyCurrentTP(int mod, int type, bool curInterrupt = true);

	int GetBuffModifyValue(WORD BuffType,int val);

	virtual bool CheckAddBuff(const SBuffBaseData *pBuff) { return false; };
	virtual void UpdateAllProperties() {}		// ��������
	virtual void SendPropertiesUpdate();		// ���跢�͸��º������

	bool ChangeFightState(DWORD type, bool marked, DWORD pusherID, float fdestX = 0.0f, float fdestY = 0.0f,DWORD dskillIndex = 0);
	bool CheckFightState(DWORD type);
	
     
	// �����ͷ����         
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
	/* ���������˺����  add by an                                                                      */
	/************************************************************************/
	void GetExtraDamage(CFightObject *pDest);
	void GetNunberFromString(string s, WORD ExtraDamageArr[]);

	bool AddToHatelist(CFightObject *pFighter);				// ��Ӳ����³���б�ʱ��
	void RemoveFromHatelistOnTime();						// �Ƴ���ҳ���б�
	void RemoveHatelistFromID(DWORD gid);
	void RemoveAllHatelist();
	bool IsInHatelist(DWORD gid);							// �Ƿ��ڳ���б���
	BYTE IsPlayerOrFightPet(CFightObject *pFighter);		// �Ƿ�����һ������� 1���2 ����
	CPlayer* GetFightObjToPlayer(CFightObject *pFighter);	// ��һ��ս������ת��Ϊ��ң�ֻ�е�ս����������һ��������ǲ���Ч
	//bool CheckPkRule(DWORD gid);							// ���PKģʽ���ж��Ƿ��ڳ���б���

	void CheckStopBullet(WORD type);						// ���ʧЧ�ӵ�
	void DamageObject_Single_Bullet(Bullet *pBullet);		// �������ӵ���
	void DamageObject_Single();								// ���������̣�Զ�̣�
	void DamageObject_AttackMore(bool self);				// Ⱥ��������Զ�̣�
	void	MoveObiect_AttackMore(CFightObject *pDest,BYTE bSkillPropertyType, float fmoveX, float fmoveY);
	void DamageObject_AttackMoreForMonster(bool self);				// Ⱥ��������Զ�̣�
	bool GetDestList(std::list<CFightObject *> *destList,bool self);

	void GetAtkObjectByList(check_list<LPIObject> *objectList, std::list<CFightObject *> *destList, INT32 *pMonsterNumber, WORD atkType, INT32 MaxAttack, bool bischeckAttack = true);

	// ������ɵ��˺�����Ϣ
	INT32 SendDamageMessageBySkillAttack_Normal(CFightObject* pDest,SDamage& damage);
	INT32 InitWoundMsg_Normal(SAWoundObjectMsgEx* woundMsg);

	// ���صķ��͹����˺�ֵ�ӿڣ����Ը��ݲ�ͬ�ġ��˺����͡����;������Ϣ
	INT32 SendDamageMessageBySkillAttack_Normal(CFightObject* pDest, SDamage& damage, BYTE DamageType);

	INT32 InitWoundTiny(SWoundTiny *wound, SDamage& damage, INT32 hitState, float DefenserX, float DefenserY);
	void SendAttackMissMsg(INT32 state);
	

	void ProcessHitPot();							// ��������
	void IntoFightState(CFightObject *pDefencer);	// �л�����ս��״̬
	virtual void SwitchFightState(bool IsIn){}		// ����/�뿪ս��״̬�ص�
	
	bool CheckAttackByClient(const SSkillBaseData *pData, SQuestSkill_C2S_MsgBody *questMsg);
	void StartWindMove(SFightBaseMsg *pMsg);

	void OnRecvCheckMove(SQWinMoveCheckMsg *pMsg);
	void SetMovePos(float fx,float fy);//�ƶ�������
	virtual float GetWalkSpeed();

	// ��״̬�Ĵ�����
	void SkillAttackStart(const SSkillHitSpot *pHitPot);// ��ʼ
	void SkillAttackDelay();	// ����
	void SkillAttackEnd();		// ����
	void UpdateSkillAttack();	// ����
	bool ContinueSkillAttack();	// ����

	virtual void AtkFinished() {}

	virtual INT32 GetCurrentSkillLevel();
	virtual INT32 GetSkillIDBySkillIndex(INT32 index);

	virtual bool SendMove2TargetForAttackMsg(INT32 skillDistance,INT32 skillIndex,CFightObject* pLife);

	void SendSwitch2SkillAttackStateMsg();							// ������Ϣ�ÿͻ����л������ܹ���״̬
	void SendSwitch2PrepareSkillAttackStateMsg(int preTime = 0);	// ������Ϣ�ÿͻ����л�������׼��״̬
	
	BOOL ActivaSkill(WORD wSkillID,LPCSTR info);
	short ActiveTempSkill(WORD wSkillID, BYTE Level = 1);
	void CoolingTempSkill(WORD index);
	bool IsSkillExisted(CPlayer* pPlayer,WORD wSkillID);
	WORD IsTelegryExisted(CPlayer *pPlayer, WORD wID);
	INT32 GetFreeSkillSlot(CPlayer* pPlayer,INT32 startSlot);
	INT32 GetSkillSlot(CPlayer* pPlayer,WORD wSkillID);
	INT32 GetFreeTelergySlot(CPlayer* pPlayer,INT32 startSlot);

	virtual bool ConsumeHP(int consume);	// ��������
	virtual bool ConsumeMP(int consume);	// ��������
	virtual bool ConsumeTP(int consume);	// ��������
	virtual SCC_RESULT CheckConsume(INT32 skillIndex, int& consumeHP, int& consumeMP);
	
	void CheckDelBuffSpot(DWORD delFlag);

	void SetListener(CFightListener* pListener);
	void RemoveListener(CFightListener* pListener);
	void RemoveAllListener();
	float GetSkillAttkFactor(SSkillBaseData *skill);
	BOOL IfAvoidAttack(WORD MyShanbi, WORD OtherShanBi);	// �Ƿ�������
	float ConsumeBaoJi(WORD MyBaoJi, WORD OtherBaoJi);		// �����ʵļ���
	float ConsumeShanBi(WORD MyShanBi, WORD OtherShanBi);	// �����ʼ���
	float DeltaLevelRate(WORD highLevel, WORD lowLevel);	// �ȼ������
	float ConsumeFinalDamageValue(CFightObject *pAtta, CFightObject *pDest);	// �����˺�����
	BOOL ConsumeFinalDamageValue(CFightObject *pAtta, CFightObject *pDest,SDamage &sdamage);	// �����˺�����

	BOOL ActiveSecondarySkill(CFightObject* pAttack, CFightObject* pDest, WORD DamageValue);		// ����������������

	virtual const char *getname();///�õ�����

	//-------���ͼ���ѧϰ���------//
	BOOL _fpActivaSkill(WORD wSkillID,BYTE index,LPCSTR info,BYTE type = SQfpSkillUpdate::SSU_LEARNED);//����ѧϰ����
	BOOL _fpActivaSkill(WORD wSkillID,BYTE bskillLevel,BYTE index,LPCSTR info,BYTE type = SQfpSkillUpdate::SSU_LEARNED);//����ѧϰ����

	BOOL _fpActivaSkillByItem(DWORD ditemID,BYTE index,LPCSTR info,BYTE type = SQfpSkillUpdate::SSU_LEARNED);//����ѧϰ����

	INT32 _fpGetFreeSkillSlot(BYTE index);	//�ռ��ܿ�
	bool	_fpGetSkillEnableLearn(WORD wSkillID,BYTE index);	///�����Ƿ���ѧϰ
	WORD	m_IsCrilial;	//�Ƿ��������
	//bool	m_IsExtraDamage;

	//������������ܴ���
	void SetQusetSkillMsg(SQuestSkill_C2S_MsgBody *pmsg);
	bool GetcontinueSkill(SQuestSkill_C2S_MsgBody *pmsg);
	int GetContinueSkillTarget(SQuestSkill_C2S_MsgBody &pmsg);//��ȡ��һ��Ŀ��
	BOOL verifySkillMsg(SQuestSkill_C2S_MsgBody *questMsg);//��֤������Ϣ����ȷ��
	void AddQuestToCache(SQuestSkill_C2S_MsgBody *questMsg);//���뼼����Ϣ������
	bool GetCacheSkill(SQuestSkill_C2S_MsgBody &pmsg);//��ȡ����ļ���
	void LuaSetCacheSkill(SQuestSkill_C2S_MsgBody *pmsg);//��lua�����û���ļ���
	void SetDamage(const SDamage *pDamage);
	WORD GetSkillDamageRate(WORD skillIndexm,WORD wlevel);//��ȡ���ܹ����˺�����

	//ͬ���ƶ�����
	void MoveInArea(CFightObject* pfightobj, float moveposX, float moveposY, DNID dnidExcept = -1);
	void PutIntoRegion(LPIObject pP, WORD wRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID,  DNID dnidExcept);


	//׷�����
	void AddToFollow(CFightObject *pfight);
	void RemoveFollow(CFightObject *pfight);
	void ClearFollow();//����ʱ���

	//add by ly 2014/4/4 ��ȡ��Ҷ����ս����
	DWORD GetPlayerFightPower();
	
public:
	SDamage	m_sdamage; //�����˺�����
	//std::vector<CFightObject*> m_followObject;//�ҵ�׷�����б�
	std::map<DWORD, CFightObject*>m_followObject;//�ҵ�׷�����б�
	CFightObject* m_pFightFollow;//��ǰ����׷����Ŀ��
protected:
	void NotifyExchange();

public:
		///��ȡ��ǰ��̬����ID��Ϊ0����ͨ����
	virtual 	DWORD GetDynamicRegionID();
	virtual 	DWORD GetSceneRegionID(); //�ؿ�ID
};