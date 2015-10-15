#pragma once

#include "FightObject.h"

// �����IID
const __int32 IID_FIGHT_PET = 0x117cb501;
#define MINUTESTIME(x)	(x)*60*1000

enum FightPetEnemyType
{
	FPET_NONE,			// ������
	FPET_PLAYERATK,		// ������ڹ�����
	FPET_ATKPLAYER,		// ������ҵ�
	FPET_ATKME,			// �����ҵ�
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
	void UpdateAllProperties();		// ��������------------------------------
	void SendPropertiesUpdate();	// ���跢�͸��º������-----------------------------
	void UpdateMyProperties();		// ��������������������-------------------------------

	void OnDamage(INT32 *nDamage, CFightObject *pFighter);
	void OnDead(CFightObject *PKiller);

	void AtkFinished();
	virtual INT32 GetCurrentSkillLevel(DWORD dwSkillIndex);
	virtual INT32 GetSkillIDBySkillIndex(INT32 index);
	WORD GetSkillNum();//���ܸ���
	WORD GetRandskillIndex();//�õ�һ������ļ���ID

	virtual bool SendMove2TargetForAttackMsg(INT32 skillDistance,INT32 skillIndex,CFightObject* pLife);
	virtual bool CheckAddBuff(const SBuffBaseData *pBuff);

	bool ConsumeHP(int consume){ return true; }		// ��������
	bool ConsumeMP(int consume);	// ��������
	SCC_RESULT CheckConsume(INT32 skillIndex, int& consumeHP, int& consumeMP);

public:
	inline void SetCurEnemy(DWORD enemyID, BYTE type);
	void SwithcStatus(bool PlayerContrl);

public:
	BOOL DoCurAction();
	void OnChangeState(EActionState newActionID);
	BOOL MoveToPostion(WORD wNewRegionID, float wStartX, float wStartY);//˲������������������;������ʱʹ��
	void OnRecvLevelUp(struct SQLevelChangeMsg *msg);		// ���յ�����������Ϣ---------------
	void SendAddLevel(WORD  Uplevel);			//---------------------------------
	const  char *getname();
	void CheckOnTime();///���ݳ�սʱ������ƣ�Ͷȣ������-----------------
	int GetDamageDecrease(int Damage); //����ƣ�Ͷ�ȡ���˺�����ϵ��----------------------------
	int GetDamageChange(int Damage); //��������ȡ���˺�ϵ��-------------------------------
	void ChangeAttType(BYTE type);//���Ĺ���ģʽ
	void UpdateProperties();//����һЩ���������--------------------------
	void OnCallBackMe();//������ջ�����ʱ

public:
	struct SParameter
	{
		CPlayer *owner;		// ����
		BYTE	index;		// ��������
		float	fX;			// ����
		float	fY;			// 
	};

public:
	typedef std::pair<DWORD, BYTE> TargetCache;

	CPlayer		*m_owner;		// ����
	BYTE		m_index;		// ����
	BYTE		m_FightType;	// ս��ģʽ
	TargetCache m_CurEnemy;		// ��ǰ�ĵ���
	TargetCache	m_TargetCache;	// Ŀ�껺��
	QWORD	m_nMaxExp;	//�������辭��
	QWORD	m_nCurExp;	//��ǰ����
	BYTE			m_nRare;//ϡ�ж�
	WORD		m_nRelation;			// ���ܶ�
	BYTE			m_nTired;				// ƣ�Ͷ�
	BYTE			m_nMood;				// ����
	BYTE			m_bSex;					//�Ա�
	BYTE			m_bAttType;				//����ģʽ

	DWORD	m_nOutFightTime;	//��սʱ��
	DWORD	m_nOutMoodTime;	//��ս��������ʱ��
	DWORD	m_nSearchTargetTime;//��������������Ŀ���ʱ��
	DWORD	m_nChangeSkilltime;//�����л�����ʱ��
	

	static const int MAX_TIRED;			//���ƣ�Ͷ�
	static const int MAX_Mood;			//�������ֵ
	static const int MAX_RELATION;	//������ܶ�
	static const int MAX_IMPEL;		//�����ֵ

	// װ�����Լӳɣ���ֵ�ͼӰٷֱȣ�
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
	
	// �����ǰû��Ŀ��������ȼ����ڵ�ǰĿ�꣬��ô�滻
	if (0 == m_CurEnemy.first || type <= m_CurEnemy.second)
	{
		m_TargetCache.first	 = enemyID;
		m_TargetCache.second = type;
	}

	return;
}