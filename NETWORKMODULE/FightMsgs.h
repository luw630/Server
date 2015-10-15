#pragma once

#include "NetModule.h"
#include "attacktypedef.h"
#include "../pub/ConstValue.h"
#include "PlayerTypedef.h"
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ս�������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SFightBaseMsg, SMessage, SMessage::EPRO_FIGHT_MESSAGE)
//{{AFX
EPRO_FIGHT_OBJECT,		// ���������ϵĶ���
EPRO_WOUND_OBJECT,      // �����ϵĶ��󱻹���
EPRO_FIGHT_POSITION,	// ���������ϵ�ĳ���ط�
EPRO_SET_CURTELERGY,    // ���õ�ǰʹ�õ��ķ�
EPRO_SET_CURSKILL,		// ���õ�ǰʹ�õ��书
EPRO_SET_EXTRASTATE,    // ���ø���״̬(��Ѩ��ѣ��)
EPRO_RET_VENATIONSTATE, // ���ؾ���״̬
EPRO_CURE_VENATION,     // �������ƾ���
EPRO_CUREOK_VENATION,   // ȷ�Ͼ�������
EPRO_DELETE_TELERGY,    // ɾ��װ�����ķ�
EPRO_KILLED_COUNT,      // ��ʾɱ�����
EPRO_PASSVENA_EFFECT,   // ��ѨЧ���㲥

EPRO_ONGOAT_MONSTERCOUNT,// �����ּ���
EPRO_REFRESH_ONUP,       // ���ߺ��������ݸ���
EPRO_WOUND_OBJECTEX,     // �����ϵĶ��󱻹���
EPRO_UPDATE_XINMOVALUE,  // ������ħֵ
EPRO_UPDATE_CUREQUIPDURANCE,// �����;�

EPRO_UPDATE_DECDURTIME,  // ���²����;�ʱ�� 

EPRO_SWITCH_TO_SKILL_ATTACK_STATE,	// ֪ͨ�ͻ����л���Զ�̼��ܹ���״̬��
EPRO_SWITCH_TO_PREPARE_SKILL_ATTACK_STATE,
EPRO_CANCEL_SKILL_ATTACK_PROCESS_BAR,
EPRO_NOTIFY_START_COLD_TIMER,
EPRO_MOUNTSKILL_COLDTIMER,
EPRO_MOVE_TO_TARGET_FOR_ATTACK,
EPRO_SHOW_ENEMY_INFO,
EPRO_QUEST_SKILL,
EPRO_SHOW_SKILL_HINT_INFO,
EPRO_OPEN_SKILL_PROCESS_BAR,

EPRO_QUEST_SKILL_FP,

EPRO_LEARN_SKILL,

EPRO_UPDATE_SKILL_BOX,

EPRO_FORCE_QUEST_ENEMY_INFO,

EPRO_SELECT_TARGET,
EPRO_TELERGY_STATE,		//add by xj �����ķ�״̬
EPRO_SETBUFFICON_CHANGE,		//buff�ı�
EPRO_SETCOLLECT,				//��ʼ�ɼ�
EPRO_CANCELCOLLECT,				//ȡ���ɼ�
EPRO_UPDATEBUFF_INFO,			//��ʱ����Ŀ������buff

//add by xj
EPRO_PRACTICE_REQUEST,			// ����չ�����
EPRO_PRACTICE_ANSWER,			// ������
EPRO_PRACTICE_ITEM_UPDATE,		// ���߸���(��Ϊ�һ����ĵ��ߺͰ����ĵ���ʵ�����Ѿ��ֿ������Բ�����ͬһ��Ϣ)

EPRO_PRACTICE_UPDATEBOX,		// ���������Ϣ

EPRO_PRACTICE_RESULT,			// �����Ľ��
EPRO_PRACTICE_STAR_TYPE,		// �㲥����������
EPRO_NOTIFY_FIGHTSTATE,			// ����/�뿪ս��״̬ʱ����
EPRO_WIND_MOVE_CHECK,			// ���˿ͻ�����֤��Ϣ
EPRO_WIND_MOVE,					// ˲����Ϣ
EPRO_SKILL_CANCEL,				// ���ܴ����Ϣ
EPRO_MONSTER_WRCHANGE,			// �������/���л�

EPRO_UPDATE_EQUIPWEAR_IN_BAG,	// ���±�����װ�����;�
EPRO_JIU_ZI_SHA,				// ����ɱ����������


EPRO_PKVALUE_UPDATE,									// ���ɱ��ֵ����
EPRO_PLAYERNAMECOLOR_CHANGE,				// ���������ɫ�ı�

EPRO_MONSTER_BOSSUPDATE,				// BOSS�������

EPRO_PlayerTemplateSkill,				// ���ģ�弼��
EPRO_QUEST_MULTIPLESKILL,			//������Ŀ��ļ���
// EPRO_ACTIVITY_UPDATE,				// ���Ϣ����
// EPRO_NEWS_UPDATE,				// �������Ϣ����
//}}AFX

//add by ly 2014/3/17
EPRO_XINYANG_SHENGXING,				//��������������Ϣ

END_MSG_MAP()
//---------------------------------------------------------------------------------------------

DECLARE_MSG(SMonsterWRChange, SFightBaseMsg, SFightBaseMsg::EPRO_MONSTER_WRCHANGE)
struct SAMonsterWRChange : public SMonsterWRChange
{
	enum
	{
		SMWR_RUN,
		SMWR_WALK,
	};

	BYTE	bStatus;		// ��ǰ״̬
	DWORD	dwGlobal;		// ����ID
};

DECLARE_MSG(SSkillCancel, SFightBaseMsg, SFightBaseMsg::EPRO_SKILL_CANCEL)
struct SASkillCancel : public SSkillCancel
{
	enum
	{
		SSC_PREPARE,		// ׼��
		SSC_SING,			// ����
		SSC_BEFORE,			// ׼������ǰ�¼�
		SSC_ATTACK,			// ����
		SSC_AFTER,			// ������Ϻ��¼�
	};

	DWORD	dwGlobal;		// Ŀ��
	BYTE	bPhase;			// ���ĸ��׶α����
	BYTE	bSkillIndex;	// ��������
};

DECLARE_MSG(SWindMoveCheckMsg, SFightBaseMsg, SFightBaseMsg::EPRO_WIND_MOVE_CHECK)
struct SAWindMoveCheckMsg : public SWindMoveCheckMsg
{
	DWORD dtagertgid;//����Ŀ��GID
	float	fSouceX;   //�������
	float	fSouceY;
	float fdir;     //���˷���
	float fDestX;   //����Ŀ���
	float fDestY;
};

struct SQWinMoveCheckMsg : public SWindMoveCheckMsg
{
	DWORD dtagertgid;//����Ŀ��GID
	float	fDestX;
	float	fDestY;
};

DECLARE_MSG(SWindMoveMsg, SFightBaseMsg, SFightBaseMsg::EPRO_WIND_MOVE)
struct SAWindMoveMsg : public SWindMoveMsg
{
	enum
	{
		SAM_PULL,		// ����
		SAM_RUSH,		// ���
	};

	BYTE	bType;
	DWORD	dwSelf;
	DWORD	dwTarget;
	float	fDestX;
	float	fDestY;
	WORD	bSkillIndex;	// ��������
};


// ֪ͨ����Ƿ������ս��״̬���뿪��ս��״̬
DECLARE_MSG(SFightStateMsg, SFightBaseMsg, SFightBaseMsg::EPRO_NOTIFY_FIGHTSTATE)
struct SAFightStateMsg : public SFightStateMsg
{
	DWORD	dwGlobalID;		// Ŀ��
	bool	IsInFight;		// �Ƿ���ս��״̬�У�true->�ǣ�false->��
};

//=============================================================================================
// �ͻ������󹥻������ϵ�ĳ�������Լ�������ͬ�⹥���Ļ�Ӧ
DECLARE_MSG(SFightObjectMsg, SFightBaseMsg, SFightBaseMsg::EPRO_FIGHT_OBJECT)
struct SQAFightObjectMsg : public SFightObjectMsg
{
	DWORD dwDestGlobalID;	// ����Ŀ���ID
	SAttack	sa;				// �����Ĳ���
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// ������Ҫ��ͻ�����Ӧһ���������Ķ���
DECLARE_MSG(SWoundObjectMsg, SFightBaseMsg, SFightBaseMsg::EPRO_WOUND_OBJECT)
struct SAWoundObjectMsg : public SWoundObjectMsg
{
	DWORD   dwGlobalID;	    // ����������ID
	SWound	sw;				// ������������Ӧ����
};

DECLARE_MSG(SWoundObjectMsgEx, SFightBaseMsg, SFightBaseMsg::EPRO_WOUND_OBJECTEX)
struct SAWoundObjectMsgEx : public SWoundObjectMsgEx
{
	enum
	{
		MAX_ENEMY_NUMBER = 50,
	};

    BYTE	mWoundObjectNumber;		// ���˵�Ŀ������...����50��
    WORD	mSkillIndex;			// ��ʽ��ţ��������ң����ֵΪ��ʽ��ţ�����Ϊ����Ч����
	DWORD	mAttackerGID;			// ��������ID, ��������SAWoundObjectMsg
	DWORD	mMaxHP;					// ���Ѫ��
	DWORD	mCurHP;					// ��ǰѪ��
// 	float	mAttackerWorldPosX;		// �����ߵĵ��������꣨����ĳЩ������˵�ǹ����㣩
// 	float	mAttackerWorldPosY;

	// �㹻���ͬ���ռ䣬�����Ա���50������Ŀ������
	char streamData[sizeof(SWoundTiny) * MAX_ENEMY_NUMBER + 8 + 4];
};

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ��Χ���������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SFightPositionMsg, SFightBaseMsg, SFightBaseMsg::EPRO_FIGHT_POSITION)
//{{AFX
EPRO_ATTACK,		// ����
EPRO_WOUND,			// ������
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================
// �ͻ����������깥�� �Լ� ������Ҫ��ͻ�����Ӧ���깥��
DECLARE_MSG(SPosAttackMsg, SFightPositionMsg, SFightPositionMsg::EPRO_ATTACK)
struct SQAPosAttackMsg : public SPosAttackMsg
{
	SAttack	sa;				// �����Ĳ���
	WORD wPosX;				// ����Ŀ���λ��
	WORD wPosY;				// ...
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
#if 0 // ����Ϣ����--yuntao.liu
// ������Ҫ��ͻ�����Ӧĳ������
DECLARE_MSG(SPosWoundMsg, SFightPositionMsg, SFightPositionMsg::EPRO_WOUND)
struct SAPosWoundMsg : public SPosWoundMsg
{
	SWound	sw;				// ������������Ӧ����
};
#endif
//---------------------------------------------------------------------------------------------

// �ͻ���ѡ��ǰ�ķ�
DECLARE_MSG(SSetCurTelergyMsg, SFightBaseMsg, SFightBaseMsg::EPRO_SET_CURTELERGY)
struct SQSetCurTelergyMsg : public SSetCurTelergyMsg
{
	BYTE    byCurTelergy;   // Ҫѡ��ʹ�õ��ķ�
};

//////////////////////////////////////////////////////////////////////////
//add by xj
DECLARE_MSG(SSetTelergyStateMsg, SFightBaseMsg, SFightBaseMsg::EPRO_TELERGY_STATE)
struct SQSetTelergyStateMsg : public SSetTelergyStateMsg
{
	BYTE    byTelergy;   // �����ķ�״̬
	//STelergy::STATETEL	byState;
};



//---------------------------------------------------------------------------------------------

//=============================================================================================
// �ͻ���ѡ��ǰ�书
DECLARE_MSG(SSetCurSelSkillMsg, SFightBaseMsg, SFightBaseMsg::EPRO_SET_CURSKILL)
struct SQSetCurSelSkillMsg : public SSetCurSelSkillMsg
{
	WORD    curSelSkill;   // Ҫѡ��ʹ�õ��书
};

enum FIGHT_STATE
{
	FS_NONE			= 0,				// ��	
	FS_DIZZY		= 0x00000001,		// ѣ��
	FS_LIMIT_SKILL	= 0x00000002,		// ����
	FS_HYPNOTISM	= 0x00000004,		// ����
	FS_DINGSHENG	= 0x00000008,		// ����
	FS_WUDI = 0x00000010,		// �޵�
	FS_ADDBUFF = 0x00000020,		// �޷������κ�BUFF
	FS_JITUI = 0x00000040,		// ������
};

// ����ս������״̬
DECLARE_MSG(SSetExtraStateyMsg, SFightBaseMsg, SFightBaseMsg::EPRO_SET_EXTRASTATE)
struct SASetExtraStateyMsg : public SSetExtraStateyMsg
{
	DWORD dwGlobalID;	        // Ŀ���ID
	DWORD dwFightExtraState;    // �����ø��ӵ�״̬
	DWORD	dwpushID;			//�ͷ���
	float  fdestX;			//Ŀ��X
	float  fdestY;			//Ŀ��Y
	DWORD  dskillid;	//����Index
};

//---------------------------------------------------------------------------------------------

//=============================================================================================
// ���ؾ���״̬
DECLARE_MSG(SVenationStateMsg, SFightBaseMsg, SFightBaseMsg::EPRO_RET_VENATIONSTATE)
struct SAVenationStateMsg : public SVenationStateMsg
{
    DWORD   dwGlobalID;	    // ��Ҫ���¾���״̬��ID
	BYTE    byWhoVenation;  // ���־���
    BYTE    byState;        // ״̬��0=���
    BYTE    byIsUpdateMsg;  // ��Ϸ�����еĸ�����Ϣ�������жϽ����Ƿ���ʾ��ʾ
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// �������ƾ���
DECLARE_MSG(SCureVenationMsg, SFightBaseMsg, SFightBaseMsg::EPRO_CURE_VENATION)
struct SQCureVenationMsg : public SCureVenationMsg
{
	DWORD   dwGlobalID;	    // �����Ʒ���ID
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// ȷ�Ͼ�������
DECLARE_MSG(SCureVenationOKMsg, SFightBaseMsg, SFightBaseMsg::EPRO_CUREOK_VENATION)
struct SACureVenationOKMsg : public SCureVenationOKMsg
{
	DWORD   dwGlobalID;	    // ���Ʒ���ID
    DWORD   dwDestGID;	    // �����Ʒ���ID
	BYTE    byDir;          // ���ƶ����ķ���
};
//---------------------------------------------------------------------------------------------

// ɾ��װ�����ķ�
DECLARE_MSG(SDeleteTelergyMsg, SFightBaseMsg, SFightBaseMsg::EPRO_DELETE_TELERGY)
struct SQDeleteTelergyMsg : public SDeleteTelergyMsg
{
	BYTE    byTelergyNum;          // �ķ�������λ��
	char	szUserpass[CONST_USERPASS];
};

struct SADeleteTelergyMsg : public SDeleteTelergyMsg
{
    BYTE    byResult; // 1 �ɹ� 0ʧ�ܣ��������벻��ȷ
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// ��ʾɱ�����
DECLARE_MSG(SKilledCountMsg, SFightBaseMsg, SFightBaseMsg::EPRO_KILLED_COUNT)
struct SAKilledCountMsg : public SKilledCountMsg
{
	BYTE    byKilledCount;          // ɱ��������
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// ��ѨЧ���㲥
DECLARE_MSG(SPassvenaEffMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PASSVENA_EFFECT)
struct SAPassvenaEffMsg : public SPassvenaEffMsg
{
	DWORD   dwGlobalID;	            // ��Ѩ��ID
	BYTE    byPassvenaType;         // ɱ��������
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// �����ּ���
DECLARE_MSG(SOnGoatMonsterCountMsg, SFightBaseMsg, SFightBaseMsg::EPRO_ONGOAT_MONSTERCOUNT)
struct SAOnGoatMonsterCountMsg : public SOnGoatMonsterCountMsg
{
	DWORD   dwGlobalID;	            // ������ҵ�ID
	WORD    wOnGoatMonsterCount;    // ��ǰ����
    WORD    wMutateTime;            // ����ʱ�䣨=0��Ч��
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// ���ߺ��������ݸ���
DECLARE_MSG(SRefreshOnUpMsg, SFightBaseMsg, SFightBaseMsg::EPRO_REFRESH_ONUP)
struct SARefreshOnUpMsg : public SRefreshOnUpMsg
{
	DWORD   dwGlobalID;	            // ������ҵ�ID
    WORD    m_wMonsterCountOnGoat;  // ��װ����ɱ������ļ���
    BYTE    m_byKilledCount;        // ɱ�������
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// ������ħֵ
DECLARE_MSG(SUpdateXMValueMsg, SFightBaseMsg, SFightBaseMsg::EPRO_UPDATE_XINMOVALUE)
struct SAUpdateXMValueMsg : public SUpdateXMValueMsg
{
    WORD m_wXinMoValue;         // ��ħֵ
};
//-----

// �����;�
DECLARE_MSG(SUpdateCurEqDuranceMsg, SFightBaseMsg, SFightBaseMsg::EPRO_UPDATE_CUREQUIPDURANCE)
struct SAUpdateCurEqDuranceMsg : public SUpdateCurEqDuranceMsg
{
    DWORD   wEqIndex;
    BYTE    byPos;
    WORD    wCurEqDuranceCur;
	WORD	wMaxEqDuranceCur;
};

//=============================================================================================
// ���²����;�ʱ��
DECLARE_MSG(SUpdateDecDurTimeMsg, SFightBaseMsg, SFightBaseMsg::EPRO_UPDATE_DECDURTIME)
struct SAUpdateDecDurTimeMsg : public SUpdateDecDurTimeMsg
{
    DWORD    dwDecDurTimeEnd;      // �־õ���ʱ��
};

// �����书��������
DECLARE_MSG(SQuestSkill_C2S, SFightBaseMsg, SFightBaseMsg::EPRO_QUEST_SKILL)
struct SQuestSkill_C2S_MsgBody : public SQuestSkill_C2S
{
	DWORD	mAttackerGID;			// �����ߵ�ID
	DWORD	mDefenderGID;			// �������ߵ�ID
	float	mDefenderWorldPosX;		// �������ߵ��������꣨����ĳЩ������˵�ǹ����㣩
	float	mDefenderWorldPosY;
	BYTE	dwSkillIndex;			// ��������
};

struct QuestSkill
{
	DWORD	mDefenderGID;			// �������ߵ�ID
	float	mDefenderWorldPosX;		// �������ߵ��������꣨����ĳЩ������˵�ǹ����㣩
	float	mDefenderWorldPosY;
	WORD  bDefenderState;//Ŀ��״̬  0 �� 1 ����״̬
};

// �����书��������
DECLARE_MSG(SQuestSkillMultiple_C2S, SFightBaseMsg, SFightBaseMsg::EPRO_QUEST_MULTIPLESKILL)
struct SQuestSkillMultiple_C2S_MsgBody : public SQuestSkillMultiple_C2S
{
	BYTE		dwSkillIndex;			// ��������
	BYTE		bTargetNum;		//����
	DWORD	mAttackerGID;			// �����ߵ�ID
	float	mAttackerWorldPosX;		// �����ߵĵ��������꣨����ĳЩ������˵�ǹ����㣩
	float	mAttackerWorldPosY;
	BYTE		buffer[1024];
};

//#pragma message ("SUpdateSkillBox_C2S_MsgBody ���ݽṹ�����Ӵ󣬲��ʺ����紫��")
DECLARE_MSG(SUpdateSkillBox_C2S, SFightBaseMsg, SFightBaseMsg::EPRO_UPDATE_SKILL_BOX)
struct SUpdateSkillBox_C2S_MsgBody : public SUpdateSkillBox_C2S
{
	INT32 playerGID;
//	SHORTCUT stBox[ 6 ][ 9 ]; 
};

DECLARE_MSG(SForceQuestEnemyInfo_C2S, SFightBaseMsg, SFightBaseMsg::EPRO_FORCE_QUEST_ENEMY_INFO)
struct SForceQuestEnemyInfo_C2S_MsgBody : public SForceQuestEnemyInfo_C2S
{
};

//----------------------------------------------------------------------------------------------------------	
 DECLARE_MSG(SOpenSkillProcessBar_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_OPEN_SKILL_PROCESS_BAR)
 struct SOpenSkillProcessBar_S2C_MsgBody : public SOpenSkillProcessBar_S2C
 {
 		INT32 mAttackGID;  //�����ߵ�gid
 		INT32 mCastTime;
 }; 

DECLARE_MSG(SSwitch2SkillAttackState_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_SWITCH_TO_SKILL_ATTACK_STATE)
struct SSwitch2SkillAttackState_S2C_MsgBody : public SSwitch2SkillAttackState_S2C
{
	BYTE   btargetnum;
	INT32	mAttackGID;		// �����ߵ�GID
	INT32	m_CurSkillID;
	float mAttackerWorldPosX;//�����ߵ�����
	float mAttackerWorldPosY;
	BYTE		buffer[1024];  //����
	
// 	INT32	mDefenderGID;	// ��������GID
// 	float	mDefenderWorldPosX;		// �������ߵ��������꣨����ĳЩ������˵�ǹ����㣩
// 	float	mDefenderWorldPosY;
};


DECLARE_MSG(SSwitch2PrepareSkillAttackState_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_SWITCH_TO_PREPARE_SKILL_ATTACK_STATE)
struct SSwitch2PrepareSkillAttackState_S2C_MsgBody : public SSwitch2PrepareSkillAttackState_S2C
{
	INT32 mAttackGID;		// �����ߵ�GID
	INT32 mDefenderGID;		// ��������GID
	INT32 mCastTime;		// ����ʱ��
	INT32 m_CurSkillID;		// ��ǰ����ID
	float fsingDir;//�ͷ���������ʱ�ķ���
};


DECLARE_MSG(SCancelSkillAttackProcessBar_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_CANCEL_SKILL_ATTACK_PROCESS_BAR)
struct SCancelSkillAttackProcessBar_S2C_MsgBody : public SCancelSkillAttackProcessBar_S2C
{
	INT32 mAttackGID;  //�����ߵ�gid
};

//----------------------------------------------------------------------------------------------------------	
DECLARE_MSG(SMove2TargetForAttack_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_MOVE_TO_TARGET_FOR_ATTACK)
struct SMove2TargetForAttack_S2C_MsgBody : public SMove2TargetForAttack_S2C
{
	INT32 mAttackGID;		// �����ߵ�ID
	INT32 mDefenderGID;		// �������ߵ�ID
	INT32 distance;			// ��������
};

//----------------------------------------------------------------------------------------------------------	
DECLARE_MSG(SShowEnemyInfo_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_SHOW_ENEMY_INFO)
struct SShowEnemyInfo_S2C_MsgBody : public SShowEnemyInfo_S2C
{
	INT32 mPlayerGID;  
	INT32 mEnemyGID;
};

DECLARE_MSG(SShowSkillHintInfo_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_SHOW_SKILL_HINT_INFO)
struct SShowSkillHintInfo_S2C_MsgBody : public SShowSkillHintInfo_S2C
{
	enum
	{
		invalid_skill_index = 0,
		invalid_weapon,
		invalid_common_CD_time,
		invalid_gropu_CD_time,
		invalid_skill_CD_time,
		invalid_consume_hp,
		invalid_consume_mp,
		invalid_consume_sp,
		invalid_state,
		invalid_object,
		object_is_dead,
		invalid_skill_distance,
		send_too_quick,
		move_failed,
		info_max,
	};
	INT32	what;
	INT32	skillID;
};

DECLARE_MSG(SNotifyStartColdTimer_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_NOTIFY_START_COLD_TIMER)
struct SNotifyStartColdTimer_S2C_MsgBody : public SNotifyStartColdTimer_S2C
{
	DWORD	CDType;
	DWORD	coldeTime;
};

//���＼����ȴʱ��
DECLARE_MSG(SMountSkillColdTimer, SFightBaseMsg, SFightBaseMsg::EPRO_MOUNTSKILL_COLDTIMER)
struct SAMountSkillColdTimer : public SMountSkillColdTimer
{
	BYTE  SkillIndex;  
	BYTE  MountIndex;
	INT32 ColdeTime;
};

enum
{
	SKILL = 1,
	FLY,
	PROTECTED,
	TELERGY,
	SPECIAL,
	MOUNT_SCRIPT_SKILL,		// ����ű�����
};

DECLARE_MSG(SLearnSkillMsg_C2S, SFightBaseMsg, SFightBaseMsg::EPRO_LEARN_SKILL)
struct SQLearnSkillMsg : public SLearnSkillMsg_C2S
{
	BYTE  byType;
	INT32 dwSkillIndex;
};

DECLARE_MSG(SSkillFPMsg_C2S, SFightBaseMsg, SFightBaseMsg::EPRO_QUEST_SKILL_FP)
struct SQSkillFPMsg : public SSkillFPMsg_C2S
{
	BYTE  byType;			// FLY or PROTECTED
	INT32 dwSkillIndex;
};

// ���ѡ��һ��Ŀ��
DECLARE_MSG(SSelectTarget, SFightBaseMsg, SFightBaseMsg::EPRO_SELECT_TARGET)
struct SQSelectTarget : public SSelectTarget
{
	DWORD dwTargetGID;		// Ŀ���ȫ��ID
};

struct SASelectTarget : public SSelectTarget
{
	DWORD dwTargetGID;		// Ŀ���ȫ��ID
	DWORD dwMaxHp;			// ���Ѫ
	DWORD dwCurHp;			// ��ǰѪ
	DWORD dwMaxMp;			// �����
	DWORD dwCurMp;			// ��ǰ��
	WORD  wLevel;			// �ȼ�
};

//add by xj 
DECLARE_MSG(SSSetBuffIcon, SFightBaseMsg, SFightBaseMsg::EPRO_SETBUFFICON_CHANGE)
struct SASetBuffIcon : public SSSetBuffIcon
{
	DWORD	dwTargetGID;		// Ŀ���ȫ��ID
	DWORD	dwBuffID;			//buffID
	WORD	buffType;			// BUFF����
	DWORD	ticks;				// BUFF����ʱ�䣨�����ֵ�����ͻ���û�м�¼��BUFF����Ϊ����������Ϊ���£������ֵ��ͬʱ�ͻ����и�BUFF����Ϊ���٣�
	BYTE 	iProcessTime;			//buff ��ʼ���Ѿ����ŵ�ʱ��(�ٷֱ�)
};

DECLARE_MSG(SSSetBuffInfo, SFightBaseMsg, SFightBaseMsg::EPRO_UPDATEBUFF_INFO)
struct SQSetBuffInfo : public SSSetBuffInfo
{
	DWORD	dwTargetGID;	//����ĵ����Ŀ��ID������buff��Ϣ			
};
struct BUFFINFO
{	
	BUFFINFO() 
		: dwBuffID( 0), bProcessTime( 0), ticks(0)
	{}

	DWORD	dwBuffID;			//buffID
	BYTE	bProcessTime;		//��ǰ����ʱʱ��ٷֱ�
	DWORD	ticks;				// BUFF����ʱ��
};
struct SASetBuffInfo : public SSSetBuffInfo
{
	enum
	{
		MAX_BUFF_NUM = 32,
	};
	DWORD	dwTargetGID;	 //����ĵ����Ŀ��ID������buff��Ϣ
	BOOL	bIsTeamMsg;		//�Ƿ������ͬ������Ϣ
	BUFFINFO stbuffinfo[MAX_BUFF_NUM]; //Ŀ��buff�б�Ĭ��������32..
};

DECLARE_MSG(SSetCollect, SFightBaseMsg, SFightBaseMsg::EPRO_SETCOLLECT)
struct SASetCollectionMsg : public SSetCollect
{
	DWORD	dwGID;
	DWORD	dwNpcId;
	DWORD	dwTimeRemain;
	WORD	x;
	WORD	y;
};

DECLARE_MSG(SCancelCollect, SFightBaseMsg, SFightBaseMsg::EPRO_CANCELCOLLECT)
struct SACancelCollectMsg : public SCancelCollect
{
	DWORD	dwGID;
};
////////////////////////////////////////////////////////////////////////// add by xj
//����չ���������
DECLARE_MSG(SCPracRequesttMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PRACTICE_REQUEST)
struct SAPracRequesttMsg : public SCPracRequesttMsg
{
	DWORD	dwGID;			//gid
	bool	bPractice;		//�Ƿ�ȡ������
	BYTE    byProLevel;		//����һ������ĵȼ���ͼ(1,2,3)�ٶȲ�һ��
	SAPracRequesttMsg()
	{
		byProLevel	=	1;
	}
//	stPractice stPracValue[6];		//���ӽṹ
};
//������
DECLARE_MSG(SCPracAnswertMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PRACTICE_ANSWER)
struct SAPracAnswertMsg : public SCPracAnswertMsg
{
	DWORD	dwGID;		
	bool	bSuccess;	//�Ƿ�����ɹ�
};
//���¹һ�������
DECLARE_MSG(SCPracItemUpdateMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PRACTICE_ITEM_UPDATE)
struct SAPracItemUpdateMsg : public SCPracItemUpdateMsg
{
	DWORD	dwGID;		
	WORD	wWhichNum;	//��һ������
	WORD	wNum;		//ʣ�����
	SAPracItemUpdateMsg()
	{
		wWhichNum = 0;
		wNum		=	0;
	}
};
// ��stBoxһ����£������Ϣû������
 DECLARE_MSG(SUpdatePracticeBox_C2S, SFightBaseMsg, SFightBaseMsg::EPRO_PRACTICE_UPDATEBOX)
 struct SUpdatePracticeBox_C2S_MsgBody : public SUpdatePracticeBox_C2S
 {
 	INT32 playerGID;
 //	stPractice stPracBox[ 6 ]; 
 };
//�õ����������Ľ��
DECLARE_MSG(SCPracResultMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PRACTICE_RESULT)
struct SAPracResultMsg : public SCPracResultMsg
{
	WORD	wType;				//�书����
	WORD	wSkillID;			//�书ID
	DWORD	wResultValue;		//������������
};
//�㲥״̬
DECLARE_MSG(SCPracStarIDMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PRACTICE_STAR_TYPE)
struct SAPracStarIDMsg : public SCPracStarIDMsg
{
	DWORD	dwPlayID;			//id
	WORD	wSkillType;			//����
	bool	bIsStar;			//true ��ʼ����  false ȡ������
};

// ���±�����װ�����;�
DECLARE_MSG(SUpdateEquipWearInBagMsg, SFightBaseMsg, SFightBaseMsg::EPRO_UPDATE_EQUIPWEAR_IN_BAG)
struct SAUpdateEquipWearInBagMsg : public SUpdateEquipWearInBagMsg
{
	DWORD   wEqIndex;
	BYTE    byPos;
	WORD    wCurEqDuranceCur;
	WORD	wMaxEqDuranceCur;
};

// ����ɱ��֪ͨ��Ϣ
DECLARE_MSG(SSNineWordMsg, SFightBaseMsg, SFightBaseMsg::EPRO_JIU_ZI_SHA)
struct SANineWordMsg : public SSNineWordMsg
{
	int		num;	// ����ɱ��1 - 9�����Ÿ��ơ���ֵΪ10��ʱ�򣬿ͻ��˾���ɱ��ơ���0��ʱ���������ɱ�����ʾ��
	DWORD	gid;	// ��10�������ʱ�򣬷��͹����ߵ�GID
};

// ���ɱ��ֵ����
DECLARE_MSG(SPkvalueChangeMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PKVALUE_UPDATE)
struct SAPkvalueChangeMsg : public SPkvalueChangeMsg
{
	WORD nPkvalue;	//ɱ��ֵ	
};

// ���������ɫ�ı�
DECLARE_MSG(SPNameColorChangeMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PLAYERNAMECOLOR_CHANGE)
struct SAPNameColorChangeMsg : public SPNameColorChangeMsg
{
	enum
	{
		COLOR_NORMAL,			//������ɫ,
		COLOR_GREEN,            //��ɫ
		COLOR_BLUE,             //��ɫ
		COLOR_VIOLET,           //��ɫ
		COLOR_ORANGE,           //��ɫ
		COLOR_YELLOW,			//��ɫ
		COLOR_LIGHTRED,		//ǳ��ɫ
		COLOR_RED,					//��ɫ
	};
	BYTE bNameColor;	//��ɫ
	DWORD m_gid;			//���gid
};
#define MAX_BOSSUPDATE_SIZE 10240
// BOSS�������
DECLARE_MSG(SBossDeadUpdatemsg, SFightBaseMsg, SFightBaseMsg::EPRO_MONSTER_BOSSUPDATE)
struct SQBossDeadUpdatemsg : public SBossDeadUpdatemsg
{
	enum
	{
		UPDATE_BOSSMSG,					//Boss����
		UPDATE_ACTIVITYMSG,			//�ճ��
		UPDATE_NEWSMSG,			//�ճ��
	};
	BYTE		bType;
	DWORD nBossState;	//״ֵ̬	
};

// BOSS�������
struct SABossDeadUpdatemsg : public SBossDeadUpdatemsg
{
	enum
	{
		UPDATE_BOSSMSG,					//Boss����
		UPDATE_ACTIVITYMSG,			//�ճ��
		UPDATE_NEWSMSG,			//�ճ��
	};
	BYTE		bType;
	DWORD nBossState;	//״ֵ̬
	BYTE Buff[MAX_BOSSUPDATE_SIZE]; //Boss����
};

struct TemplateSkillInfo
{
	BYTE SkillIndex;
	long SkillID;
};
DECLARE_MSG(SPlayerTemplateSkillmsg, SFightBaseMsg, SFightBaseMsg::EPRO_PlayerTemplateSkill)
struct SAPlayerTemplateSkillmsg : public SPlayerTemplateSkillmsg
{
	BYTE Flag; //0,���ؼ��� 1ȥ����Щ����
	BYTE m_num; //���ܸ���
	TemplateSkillInfo temp[10];
	SAPlayerTemplateSkillmsg():Flag(0),m_num(0)
	{
		memset(&temp[0],0,sizeof(TemplateSkillInfo) * 10);
	}
}; 

//add by ly 2014/3/17	����������������Ӧ����Ϣ
DECLARE_MSG(SXinYangRiseStarmsg, SFightBaseMsg, SFightBaseMsg::EPRO_XINYANG_SHENGXING)
struct SQXinYangRiseStarmsg : public SXinYangRiseStarmsg
{
	BYTE m_XinXiuID; //��ʼ����ID
	BYTE MoneyType;	//����Ǯ������
	BYTE m_num; //���Ǵ��� ���Ϊ12��
};

struct SAXinYangRiseStarmsg : public SXinYangRiseStarmsg
{
	BYTE m_SuccessNum;	//���ǳɹ����� m_SuccessNum = -1ʱ����ʾǮ������0��ʾһ�ζ�û�����ǳɹ�
	DWORD m_ConsumeMoney;	//����Ǯ������
	DWORD m_ConsumeSP;		//��������������
};


