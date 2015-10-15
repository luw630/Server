#pragma once

// ���ܵ�����
// 1 ����
// 2 ��ɽ
// 3 ����
// 4 �Ⱥ�
// 5 ����
// 6 ��������ѧ��
// 7 ����
enum SkillSchool
{
	SS_WUYUE = 1,			// ����
	SS_SHUSHAN,				// ��ɽ
	SS_SHAOLIN,				// ����
	SS_CIHANG,				// �Ⱥ�
	SS_RIYUE,				// ����
	SS_XIAKE,				// ����
	SS_OTHER,				// ����
	SS_MONSTER,				// ����
	SS_MAX,
};

// ����ģʽ
enum SkillAtkScopeType
{
	SAST_SINGLE,			// ���������ڵ�һĿ�꣩				�ӵ�����
	SAST_EMPTY_CIRCLE,		// Բ��Ⱥ��������ԭ�㣩				�ӵ�������
	SAST_EMPTY_SECTOR,		// ����Ⱥ��������ԭ�㣩����			�ӵ�������
	SAST_EMPTY_POINTS,		// ֱ��Ⱥ��������ԭ�㣩				�ӵ�������
	SAST_POINT_CIRCLE,		// Բ��Ⱥ����Ŀ��㣩				�ӵ�����
	SAST_SELF,				// ���Լ��ͷ�						�ӵ�������
	SAST_POINT_SINGLE,		// ��Ŀ����ͷţ�˲�ƣ������ߵȣ�	�ӵ�����
	//SAST_EMPTY_RECT,		// ֱ�߾���Ⱥ��������ԭ�㣩				
};

// ��������
// ���ڼ����������ļ��㹫ʽ��
enum SkillPropertyType
{
	SPT_COMMAN,				// ��ͨ����
	SPT_NEARBY,				// ������
	SPT_LONGDISTANCE,		// Զ�̼���
	SPT_RANGE,				// ��Χ����
	SPT_AUXILIARY_ACTIVE,	// ������������
	SPT_AUXILIARY_PASSIVE,  // �����ֿ�����
	SPT_PASSIVE,			// ��������
	SPT_CHARGE,				// ��漼�ܣ���Ҫ��֤·��
	SPT_REPEL,				// ����Ŀ��
	SPT_MOVE,				// λ�Ƽ��ܣ������ƶ�ͬʱĿ�����
	SPT_SPECIAL,			// ���⼼��(��ʱû���� 2011.08.26 by dj)
};

//����������
enum SkillSubProperty
{
	SSP_CHARGE_AYB = 1,	//���ơ�WOW ��Ӱ����
	SSP_CHARGE_SX ,		//���ơ�WOW ���֡�
	SSP_CHARGE_CF ,		//���ơ�WOW ��桱
	SSP_CHARGE_TY ,		//���ơ�WOW ��Ծ��
	SSP_CHARGE_LR ,		//���ơ�WOW ���ˡ�
};

// �����˺����㹫ʽ��
enum SkillDamageCalcType
{
	SDCT_PHY_NEAR,			// �⹦����
	SDCT_PHY_FAR,			// �⹦Զ��
	SDCT_FP_NEAR,			// �ڹ�����
	SDCT_FP_FAR,			// �ڹ�Զ��
};

// ��������
enum SKillType
{
	SKT_ACTIVE,				// ����
	SKT_PASSITIVE = 2,			// ����
};

// һ�ι����ļ����׶�
enum SKillStep
{
	SKS_PREPARE,			// ׼��
	SKS_SING,				// ����
	SKS_BEFORE,				// ׼������ǰ�¼�
	SKS_ATTACK,				// ����
	SKS_AFTER,				// ������Ϻ��¼�
	SKS_OVER,				// �������
};

// �ķ����ӵĸ�������
enum TelergyType
{
	TET_ATTACK,			// ���ӹ�����
	TET_DEFENCE,		// ���ӷ���
	TET_HP,				// ��������
	TET_MP,				// ��������
	TET_TP,				// ��������
	TET_SHANBI,			// ��������
	TET_BAOJI,			// ���ӱ���
	// ����BUFFER
	TET_SUCKHP_PER,		// ������Ѫ�ٷֱ�
	TET_SUCKMP_PER,		// �����������ٷֱ�
	TET_SUCKTP_PER,		// ���ӿ������ٷֱ�
	TET_SUCKATTK,		// ���ӿ۹�����
	TET_SUCKDEF,		// ���ӿ۷�����

	TET_MAX,
};

const WORD MaxSkillLevel = 100;		// ��߼��ܵȼ�
const WORD MaxXinFaLevel = 100;		// ����ķ��ȼ�

// ���ܴ��������
class SSkillHitSpot
{
public:
	SSkillHitSpot() { memset(this, 0, sizeof(SSkillHitSpot)); }
	
public:
	WORD	m_ID;			// ����ID
	WORD	m_HitCount;		// ��������
	WORD	m_HitPots[12];	// �����ʱ�̱�
	WORD	m_CalcDmg[12];	// ������Ƿ�����˺�
};

// �ķ��Ļ�������
class SXinFaData
{
public:
	SXinFaData() { memset(this, 0, sizeof(SXinFaData)); }

public:
	char	m_Name[31];				// ����
	WORD	m_ID;					// �ķ�ID
	WORD	m_Level;				// �ķ��ȼ�
	DWORD	m_NeedSp;				// ��������
	DWORD	m_NeedMoney;			// �����Ǯ
	WORD    m_AddAttack;            // ���ӹ�����
	WORD    m_AddDefence;           // ���ӷ�����
	WORD    m_AddLife;              // ��������ֵ
	WORD    m_AddInterforce;        // ��������ֵ
	WORD    m_AddPhysiforce;        // ��������ֵ
	WORD    m_AddDodgeforce;        // ���Ӷ���ֵ
	WORD    m_AddCriceforce;        // ���ӱ���ֵ
	WORD    m_AddPctBloodSucker;    // ������Ѫ�ٷֱ�
	WORD    m_AddPctCutInterforce;  // ���ӿ������ٷֱ�
	WORD    m_AddPctCutPhysiforce;  // ���ӿ������ٷֱ�
	WORD    m_AddPctCutAttakNum;	// ���ӿ۹�������
	WORD    m_AddPctCutDefecNum;	// ���ӿ۷���������
};

// ���ܵĻ�������
class SSkillBaseData
{
public:
	SSkillBaseData() { memset(this, 0, sizeof(SSkillBaseData)); }

public:
	char	m_Name[31];				// ����
	WORD	m_ID;					// ���
	WORD	m_Level;				// ���ܵȼ�
	DWORD	m_UpNeedSp;				// ������������
	DWORD	m_UpNeedMoney;			// ���������Ǯ
	BYTE	m_School;				// ����
	BYTE	m_SkillType;			// ��������
	BYTE	m_SkillPropertyType;	// ������������	
	BYTE	m_SkillSubProperty;		// ����������	
	WORD	m_SkillAttack;			// ���ܹ�����
	WORD	m_AtkType;				// ������ʽ
	WORD	m_CDGroup;				// ��ȴ��ID
	DWORD	m_CDTime;				// ������ȴʱ��
	WORD	m_SkillMaxDistance;		// ����������
	WORD	m_SkillMinDistance;		// ������С����
	WORD	m_SkillAtkRadius;		// ���ܹ����뾶
	WORD	m_SkillAtkWidth;		// ���ܹ�����ȣ�����ֱ�����ã�
	WORD	m_AtkAmount;			// ��������
	WORD	m_ConsumeMP;			// MP����
	WORD	m_ConsumeHP;			// HP����
	WORD	m_HitBackDis;			// ���˾���
	WORD	m_HateValue;			// ���ӳ��ֵ
	bool	m_IsNormalSkill;		// �Ƿ�Ϊ��ͨ����
	WORD	m_DamageRate;			// �˺�����
	bool	m_CanUseInDizzy;		// ѣ��ʱ�ɷ�ʹ��
	WORD	m_BulletID;				// �ӵ�ID
	WORD	m_SingTime;				// ����ʱ��

	//add 2014.3.4
	DWORD   m_IconID;				//����ͼ��
	char    m_Description[256];		//��������
	char    m_Detail[256];			//����Ч��
	WORD    m_SkillTypePos;			//���ܴ��λ��
	char    m_FindSkillDes[256];			//���ܳ���
	DWORD   m_SkillBookID;			//������ID
	DWORD   m_ConsumeItemID;		//���ĵ��ߵ�ID
	BYTE	m_OnceNeedItemCount;	//һ����Ҫ���ߵ�����
	BYTE	m_NeedPlayerLevel;	//�������Ҫ��ҵĵȼ�
};
