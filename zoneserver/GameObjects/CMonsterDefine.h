#ifndef CMONSTERDEFINE_H
#define CMONSTERDEFINE_H
#include "pub\ConstValue.h"

// ���������
// 0	����	�����κ�����
// 1	��ɽ	��������ɽ�����Ѻ���������
// 2	����	���������������Ѻ���������
// 3	����	���������������Ѻ���������
// 4	����	���������������Ѻ���������
// 5	�Ⱥ�	�������Ⱥ������Ѻ���������
enum MonsterOrbit
{
	MO_NONE = 0,			// ����
	MO_SHUSHAN,				// ��ɽ
	MO_RIYUE,				// ����
	MO_SHAOLIN,				// ����
	MO_WUYUE,				// ����
	MO_CIHANG,				// �Ⱥ�

	M0_MAX,					// Max
};

// ���������ģ�ʹ�С
const float MB_SMALL	= 0.5;
const float MB_NORMAL	= 1;
const float MB_LARGE	= 2;

enum MonsterLevelType
{
	MT_NORMAL = 0,			// ��ͨ
	MT_ELITE,				// ��Ӣ
	MT_JUNIOR_BOSS,			// ����BOSS
	MT_MIDDLE_BOSS,			// �м�BOSS
	MT_ADVANCE_BOSS,		// �߼�BOSS

	MT_MAX,					// Max
};

enum MonsterAttType
{
	MAT_ACTIVELY,			// ��������
	MAT_PASSIVE,			// ��������
	MAT_RUBBER,				// ��Ƥ�ˣ��򲻻���

	MAT_MAX,				// Max
};

enum MonsterMoveType
{
	MMT_NOMOVE = 0,			// ���ƶ�
	MMT_AREA,				// ��Χ�ƶ�
	MMT_ROAD,				// ·���ƶ�
};

// ׷��ֹͣ����
enum MonsterAfterCondition
{
	MAC_OVER_PARTOL = 0,	// �������Χ
	MAC_MAX,				

	// ����MAX���ͱ�ʾ��׷����������
	MAC_OVER_DISTANCE,		// �������ĳ��ֵ
};

// ��������
enum MonsterEscapeCondition
{
	MES_NORUN,				// �Ӳ�����
	MES_LOW_HP,				// Ѫ������
};

#define MAX_MONTER_SKILLCOUNT 5 ///���＼���������
// ����Ļ�������
class SMonsterBaseData
{
public:
	SMonsterBaseData()
	{
		memset(this, 0, sizeof(SMonsterBaseData));
	}

public:
	char	m_Name[CONST_USERNAME];				// ���ƣ�name[10] == 0
	BYTE	m_Orbit;				// ����
	BYTE	m_LevelType;			// �������ͣ���ͨ&��Ӣ&BOSS��
	BYTE	m_AttType;				// ��������
	DWORD	m_WalkSpeed;			// ��ս���ƶ��ٶ�
	DWORD	m_FightSpeed;			// ս���ƶ��ٶ�
	DWORD	m_MaxHP;				// ���Ѫ��
	DWORD	m_ReBirthInterval;		// �������ʱ��
	DWORD	m_PartolRadius;			// ��ذ뾶
	DWORD	m_ActiveRadius; 		// ��뾶
	DWORD	m_TailLength;			// ׷������
	DWORD	m_Exp;					// Я������
	DWORD	m_Sp;					// Я������
	WORD	m_ID;					// ����ID
	WORD	m_LookID;				// ���&ͷ����
	WORD	m_BodySize;				// ����ģ�ʹ�С
	WORD	m_level;				// �ȼ�
	WORD	m_AttInterval;			// �������
	WORD	m_MaxMP;				// ����ڹ�
	WORD	m_BloodMute;			// Ѫ�ֱ�����
	WORD	m_AttMute;				// ���ֱ�����
	WORD	m_DefenceMute;			// ���ֱ�����
	WORD	m_KillTaskID;			// ɱ���������
	WORD	m_Direction;			// ���ﳯ��
	WORD	m_EndTailCondition;		// ׷��ֹͣ����
	WORD	m_MoveType;				// �ƶ�����
	WORD	m_SearchTargetInterval;	// ����Ŀ����
	WORD	m_StayTimeMin;			// ����ͣ��ʱ������
	WORD	m_StayTimeMax;			// ����ͣ��ʱ������

	WORD	m_GongJi;				// ����
	WORD	m_FangYu;				// ����
	WORD	m_BaoJi;				// ����
	WORD	m_ShanBi;				// ����
	WORD	m_AtkSpeed;				// �����ٶ�	

	WORD m_SkillID[MAX_MONTER_SKILLCOUNT];	///���＼��ID
	WORD m_StaticAI;						///AI���
	WORD m_GroupID;							///AI���
	WORD m_uncrit;//����
	WORD m_wreck;//�ƻ�
	WORD m_unwreck;//����
	WORD m_puncture;//����
	WORD m_unpuncture;//����
	WORD m_Hit;//����

};

struct BossDeadData	//Boss���͹����������ݽṹ
{
	WORD	m_ID;					// ����ID
	WORD	m_level;				// �ȼ�
	WORD	reginid;					//����ID
	WORD  wX;						//����X	
	WORD  wY;				//����Y
	WORD	wHour;					//����
	WORD	wMinute;				//����
	WORD	wState;					//��ǰ״̬
	char	m_KillMyName[CONST_USERNAME];				// ɱ��������������
	DWORD m_UpdateTime;	//״̬����ʱ��
};

struct ActivityNotice	//�ճ��Ļ���ݽṹ
{
	
	char  ActivityName[10] ;		//�����(10)
	char	Activitydescribe[50] ;			//�����(50)
	char	ActivityNpcName[10] ;			//�Npc����(50)

	BYTE		  ActivityType ;				//�����
	DWORD	ActivityDate ;			//�����
	WORD	 ActivityStartTime ;		//���ʼʱ��
	WORD	ActivityEntryRegin ;		//���ڳ���id
	WORD	ActivityEntryCoordX ;	//��������
	WORD	ActivityEntryCoordY;		//��������

	DWORD	ActivityReward[2];			//���������
	//DWORD m_UpdateTime;	//״̬����ʱ��
};

#endif // CMONSTERDEFINE_H
