#pragma once

#include "NetModule.h"
#include "PlayerTypedef.h"

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ��ֵ�仯����������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SUpgradeMsg, SMessage, SMessage::EPRO_UPGRADE_MESSAGE)
//{{AFX
EPRO_RES_CHANGE,				// ��Դ����ǰ���������������������ԣ�������ʱ�仯��
EPRO_EXP_CHANGE,				// ����ֵ�仯
EPRO_MONEY_CHANGE,				// ��Ǯ�仯
EPRO_SET_POINT,					// �������,�ͻ������������
EPRO_BASEPOINT_CHANGE,			// �������Եı仯�����������Ժ�
EPRO_ABILITY_CHANGE,			// �������Եı仯
EPRO_XVALUE_CHANGE,				// ����ֵ�ı仯����Ϊ���������������������Ҷ��ῴ���ģ����Ե�������
EPRO_MAXPROFIC_CHANGE,			// �书���������ֵ�仯
EPRO_TETERGY_CHANGE,			// �ķ����ݵı仯
EPRO_ACTSPEED_CHANGE,			// �����ٶȵĸı�
EPRO_PASS_VENAPOINTASK,			// �����ͨѨ��
EPRO_PASS_VENAPOINTOK,			// ��ͨѨ��ȷ��

EPRO_SET_EXTRATELERGY,			// ������չ�ķ�
EPRO_UPDATE_NPCLIST,			// ����NPC��Ϣ[����ɫ����һ������ʱ�����͵�ǰ����������NPC���ͻ���][�����Զ�Ѱ�˴���]
EPRO_OPEN_EXTRATELERGY,			// ��34��չ�ķ�
EPRO_LOCKRES_CHANGE,			// �������徭���������ҵ����
EPRO_LOCKRES_NAME,				// Ŀ���������

EPRO_UPDATE_USE_POINTS,			// ���¿ͻ���Ŀǰ����ʹ�õĵ���
EPRO_UPDATE_STATE_PANEL_DATA,	// ���¿ͻ��˵�״̬�������� EPRO_UPDATE_ATTACK_DEFENSE_VALUE,		
EPRO_QUEST_ADD_POINTS,			// �ͻ�������ӵ�
EPRO_BUFF_UPDATE,				// ����BUFF��Ϣ���½������¡����٣�

EPRO_BUFF_PRO_SYN_TO_SELF,		// BUFF����ͬ����Ϣ���Լ�
EPRO_BUFF_PRO_SYN_TO_SELECT,	// BUFF����ͬ����Ϣ��ѡ�����Լ���
EPRO_BUFF_PRO_SYN_TO_ALL,		// BUFF����ͬ����Ϣ��������
EPRO_BUFF_EFFECT_SYN_TO_ALL,	// BUFF��Чͬ����Ϣ��������

EPRO_FLY_CHANGE,
EPRO_PROTECTED_CHANGE,

EPRO_FP_BUFF,					// buff�������ͽ��� ֪ͨ�ͻ���
EPRO_TELERGY_CHANGE,			// �ķ��仯

#if 0
EPRO_BUFF_USE_SUCCESS,
#endif

EPRO_FP_ATTRIB_CHANGE,
EPRO_DRUG_INFO_SHOW,

EPRO_BUFFFIGHTOUT_POS,			// BUFFӰ��Ļ�����Ч��Ϣ
EPRO_BUFF_SPLITMOVE,			// ˲��Ч����Ϣ
EPRO_LEVEL_CHANGE,				// �ȼ��仯����Դ�������ֵ�仯���������������������ֵ
EPRO_TELERGY_UPDATE,			// �����ķ�
//add 2014.3.1
EPRO_OPEN_SETSKILL,				//�������ҵļ����������
EPRO_UPDATE_PLAYERSKILL,		//������ҵ�ǰ��ʹ�õļ���

EPRO_SKILL_UPDATE,				// ���¼�����Ϣ
EPRO_SPEED_CHANGED,				// �ٶȸ�����Ϣ
EPRO_ATKSPEED_CHANGED,			// �����ٶȸ�����Ϣ
EPRO_PROPERTY_CHANGED,			// ���Ը���ͳһ֪ͨ
EPRO_PLAYER_DEAD,				// �������
EPRO_PLAYER_RELIVERESULT,				// ��Ҹ���Ľ��
EPRO_REGION_SYN_TIME,			// ����ʱ�����
EPRO_OPEN_VEN,					// ��ʼ��Ѩ


EPRO_FIGHTPET_EXPCHANGE,				// ���;���ֵ�仯
EPRO_FIGHTPET_LEVELCHANGE,				// ���͵ȼ��仯
EPRO_FIGHTPET_PROPERTYCHANGE,			// �������Ա仯
EPRO_FIGHTPET_FIGHTPROPERTYCHANGE,		// ����ս�����Ա仯
EPRO_FIGHTPET_EXTRAPROPERTYCHANGE,		// ���Ͷ������Ա仯
EPRO_FIGHTPET_SKILLUPDATE,				// ���͸��¼�����Ϣ
EPRO_ALL_PROPERTYCHANGED,				// ���Ը���֪ͨ

EPRO_TIZHI_UPDATE,						//�������ݸ���
EPRO_TIZHI_TUPO,						//����ͻ��
EPRO_KYLINARM_UPDATE,					//��������ݸ���

EPRO_KYLINARM_UPGRADE,					//���������
EPRO_RESET_POINT,						// ϴ��
EPRO_XWZ_UPDATE,						// ��Ϊֵ����

EPRO_PLAYERDEAD_ADD,				// �������ʱ�ĸ�����Ϣ�������͸��������������

//add by ly 2014/3/25
//��ҫ��Ϣͨ�������ã��ͻ�����������ݡ�����������lua���л������ݣ��ͻ���û�н���
EPRO_PLAYERGETGLORY,	//��ȡ�����ҫ��Ϣ
EPRO_GETPLAYERGLORY,	//��ȡ��ҫ����
//�ƺ���Ϣ�������󣬿ͻ����Ѿ����������
EPRO_TITLE,	//�ƺ���Ϣ

//add by ly 2014/5/16
EPRO_NOTITY_SKILLUPDATED,	//֪ͨ��ң���ǰ��ҵļ��������Ѿ�����

//add by ly 2014/7/22
EPRO_QUEST_RANDLISTINF,		//�������а�����
EPRO_RANKLIST_GETWARD,	//��ȡ���а��Ӧ�Ľ���
EPRO_RANKLIST_GEAWARDSTATE,		//��ȡ������а�����ȡ״̬

//add by ly 2014/7/23
EPRO_ACTIVENESS_GETINFO,	//��ȡ��Ծ�������Ϣ
EPRO_ACTIVENESS_GETAWARD,	//��ȡ��Ծ�ȵĶ�Ӧ����

//}}AFX
END_MSG_MAP()

//�¼�ö��	��ʶ��ҳ�ҩ�ظ����Ե�ʲôҩ���ظ���ʲô״̬   ������������xj
typedef enum
{
	TYPE_NULLDRUG = 0,
	TYPE_LIFE,
	TYPE_MAGIC,
	TYPE_STRENGTH,
}enDrug_Type;
//�书���� ------xj	//�ʼ�趨ֻ��Ϊ��mainpanel��m_Box��ʾ�����������书Ҳ������������ͱ�ʾ
enum BOX_TYPE
{
	TYPE_ITEM,
	TYPE_COMMONSKILL,
	TYPE_FLYSKILL,
	TYPE_PROSKILL,
	TYPE_TELSKILL,
	TYPE_MOUNTSKILL,
	//�����ͼ���ǰ��
	TYPE_ALL,
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SKylinArmUpdateMsg,SUpgradeMsg,SUpgradeMsg::EPRO_KYLINARM_UPDATE)
struct SAKylinArmUpdateMsg : public SKylinArmUpdateMsg
{
	SPlayerKylinArm sData;
};
struct SQKylinArmUpdateMsg : public SKylinArmUpdateMsg
{

};
DECLARE_MSG(SKylinArmUpgradeMsg,SUpgradeMsg,SUpgradeMsg::EPRO_KYLINARM_UPGRADE)
struct SQKylinArmUpgradeMsg : public SKylinArmUpgradeMsg
{

};
struct SAKylinArmUpgradeMsg : public SKylinArmUpgradeMsg
{
	enum
	{
		SUCCESS,
		FAILED,
		NOTYUANQI,
		NOTLEVEL,
		NOTMONEY,
		NOTSP,
		NOTITEM,
		NOTICE,
	};
	BYTE byState;
};
DECLARE_MSG(STiZhiUpdateMsg,SUpgradeMsg,SUpgradeMsg::EPRO_TIZHI_UPDATE)
struct SATiZhiUpdateMsg : public STiZhiUpdateMsg
{
	SPlayerTiZhi tizhiData;
};
struct SQTiZhiUpdateMsg : public STiZhiUpdateMsg
{
	
};
DECLARE_MSG(STiZhiTupoMsg,SUpgradeMsg,SUpgradeMsg::EPRO_TIZHI_TUPO)
struct SQTiZhiTupoMsg : public STiZhiTupoMsg
{

};
struct SATiZhiTupoMsg : public STiZhiTupoMsg
{
	enum
	{
		SUCCESS,
		NOITEM,
		FAILED,
		NOTLEVEL,
		NOTSP,
		MAXLEVEL
	};
	BYTE byState;
};
DECLARE_MSG(SOpenVenMsg, SUpgradeMsg, SUpgradeMsg::EPRO_OPEN_VEN)
struct SQOpenVenMsg : public SOpenVenMsg
{
	WORD wItemPos;		// ���ϵ�����
	WORD wVenID;		// Ѩλ��ID
	byte bNum;          //���ϵ���Ŀ
};

struct SAOpenVenMsg : public SOpenVenMsg
{
	enum
	{
		SOV_SUCCESS,
		SOV_FAILED,
	};

	BYTE bResult;		// ���
	WORD wVenID;		// ѨλID
};

DECLARE_MSG(SPlayerDeadMsg, SUpgradeMsg, SUpgradeMsg::EPRO_PLAYER_DEAD)
struct SAPlayerDeadMsg : public SPlayerDeadMsg
{
	DWORD dwGID;		// ���ߵ�ID
	DWORD killGID;		//������GID
};

struct SQPlayerDeadMsg : public SPlayerDeadMsg
{
	enum
	{
		PDM_BACK_CITY,					// �سǸ���
		PDM_HERE_FREE,					// ���ԭ�ظ���
		PDM_HERE_PAY,					//�շ�ԭ�ظ���
		PDM_HERE_FULLLIFE,			// ����ԭ�ظ���
	};
	
	BYTE bType;				// ����ѡ��
};
     
DECLARE_MSG(SPlayerDeadResultMsg, SUpgradeMsg, SUpgradeMsg::EPRO_PLAYER_RELIVERESULT)
struct SAPlayerDeadResultMsg :  public SPlayerDeadResultMsg
{
	enum
	{
		RELIVE_SUCCESS, //����ɹ�
		RELIVE_FAIL_TIME, //����ʧ�ܡ�û�дﵽ��Ҫ�ȴ���ʱ��
		RELIVE_FAIL_MONEY, //����ʧ�ܡ���Ǯ����
	};
	BYTE bResult;
};

DECLARE_MSG(SSpeedChanged, SUpgradeMsg, SUpgradeMsg::EPRO_SPEED_CHANGED)
struct SASpeedChanged : public SSpeedChanged
{
	DWORD dwGlobal;		// ID
	float fSpeed;		// ��ǰ�ٶ�
};

DECLARE_MSG(SAtkSpeedChanged, SUpgradeMsg, SUpgradeMsg::EPRO_ATKSPEED_CHANGED)
struct SAAtkSpeedChanged : public SAtkSpeedChanged
{
	DWORD dwGlobal;		// ID
	float wAtkSpeed;	// ��ǰ�����ٶ�
};

DECLARE_MSG(SPropertyChange, SUpgradeMsg, SUpgradeMsg::EPRO_PROPERTY_CHANGED)
struct SAPropertyChange : public SPropertyChange
{
	enum { PROPERTY_BUFFSIZE = 512 };

	WORD	buffSize;	// �����С
	BYTE	PropertyData[PROPERTY_BUFFSIZE];
};

DECLARE_MSG(_SUpdateBuff, SUpgradeMsg, SUpgradeMsg::EPRO_BUFF_UPDATE)
struct SUpdateBuff : public _SUpdateBuff 
{
	DWORD	dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
	DWORD	dwBuffID;			// ID
	DWORD	dwMargin;			// ���
	DWORD	dwWillActTimes;		// Ҫ����Ĵ���
	int		m_curStep;			// ��ǰ�׶Σ�0��-1��-2������ֵ��
	float		wReginBuffHeight;//���򳤶ȣ����ΪԲ�ξ�����Ϊ����뾶��
	float		wReginBuffWidth;//�����ȣ����ΪԲ�ν���ʹ�����������
	float    fCenterPosX;  //Բ������
	float    fCenterPosY;
	BYTE	bType;				// ����/����/����Buff
	BYTE bReginBuffShape;// ������״ 0 Բ�� 1����
};

//=============================================================================================
// NPC�б�
DECLARE_MSG(SUpdateNpcListMsg, SUpgradeMsg, SUpgradeMsg::EPRO_UPDATE_NPCLIST)
struct  SAUpdateNpcListMsg : public SUpdateNpcListMsg
{
    enum
    {
        OPTION_REGION_BLOCK = 0x1,      // ��ʾ���������赲
        OPTION_REGION_GJLIMIT
    };

	struct NpcInfo {
		int npcid;
		DWORD gid;
		WORD x, y;
		BYTE type;
		char name[CONST_USERNAME];
	};
	WORD num;
	WORD option;
	NpcInfo npcArray[1];
};

// ����ʱ�仯����Դ����
DECLARE_MSG(SResChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_RES_CHANGE)
struct SAResChangeMsg : public SResChangeMsg
{
    DWORD	dwGlobalID;         // ��������ȫ��Ψһ��ʶ��
    DWORD	dwCurHP;            // ��ǰ�µ�����ֵ
    WORD	wCurMP;             // ��ǰ�µ�����ֵ
    WORD	wCurSP;             // ��ǰ�µ�����ֵ
};

DECLARE_MSG(SLockResNameMsg, SUpgradeMsg, SUpgradeMsg::EPRO_LOCKRES_NAME )
struct SALockResNameMsg :  public SLockResNameMsg // ��ҵ��Ŀ������֣����ڿ��ͬ��������
{
	char name[CONST_USERNAME];		// ����Ŀ�������
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// ����ʱ�仯�������ҵ���һ�ȡ��Ϣ
DECLARE_MSG(SLockResChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_LOCKRES_CHANGE )
struct SALockResChangeMsg :  public SLockResChangeMsg
{
    DWORD dwGlobalID;      // ��������ȫ��Ψһ��ʶ��
    BYTE byHP;            // ��ǰ�µ�����ֵ�ٷֱ�
    BYTE byMP;            // ��ǰ�µ�����ֵ�ٷֱ�
    BYTE bySP;            // ��ǰ�µ�����ֵ�ٷֱ�
    BYTE byVA;            // ������������������
};

// ��Ǯ�仯
DECLARE_MSG(SMoneyChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_MONEY_CHANGE)
struct SAMoneyChangeMsg : public SMoneyChangeMsg
{
	DWORD	dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
	DWORD   dwBindMoney;		// �󶨻���
	DWORD   dwMoney;			// �ǰ󶨻���
};

// ����ֵ�仯
DECLARE_MSG(SExpChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_EXP_CHANGE)
struct SAExpChangeMsg : public SExpChangeMsg
{
    enum
    {
        EMPTY,
		GM,
		TASKFINISHED,			// �������
        MONSTERDEAD,			// ��������
        TEAMSHARED,				// ��Ӽӳ�
		FRIENDUPDATE,			//�������߽���
		BIGUANJIANGLI,			//�չؽ���
		JINGMAIOPENVENSHARE,	//������Ѩ�������
    };
    DWORD	dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
	UINT64  qwCurGain;			// ���λ�õľ���
    BYTE	byExpChangeType;	// ������ı������
};


// ���;���ֵ�仯
DECLARE_MSG(SFightPetExpChange, SUpgradeMsg, SUpgradeMsg::EPRO_FIGHTPET_EXPCHANGE)
struct SAFightPetExpChange : public SFightPetExpChange
{
	BYTE		m_index;					// ��������
	enum
	{
		EMPTY,
		GM,
		TASKFINISHED,			// �������
		MONSTERDEAD,			// ��������
		TEAMSHARED,				// ��Ӽӳ�
	};
	UINT64  qwCurGain;			// ���λ�õľ���
	BYTE	byExpChangeType;	// ������ı������
};

// ���͵ȼ��仯
DECLARE_MSG(SFightPetLevelChange, SUpgradeMsg, SUpgradeMsg::EPRO_FIGHTPET_LEVELCHANGE)
struct SAFightPetLevelChange : public SFightPetLevelChange
{
	BYTE		m_index;					// ��������
	BYTE		bLevel;	
	UINT64 m_curExp;					//��ǰ����
	UINT64 m_MaxExp;				//�����
};

// �������Ա仯
DECLARE_MSG(SFightPetPropertyChange, SUpgradeMsg, SUpgradeMsg::EPRO_FIGHTPET_PROPERTYCHANGE)
struct SAFightPetPropertyChange : public SFightPetPropertyChange
{
	BYTE			m_index;					// ��������
	WORD			m_MaxHp;					// �������
	WORD			m_MaxMp;					// �������
	WORD			m_CurHp;					// ��ǰ����
	WORD			m_CurMp;					// ��ǰ����
	WORD			m_GongJi;					// ����
	WORD			m_FangYu;					// ����
	BYTE			m_BaoJi;					// ����
	BYTE			m_ShanBi;					// ����
};	

// �������Ա仯
DECLARE_MSG(SfpfightPropertyChange, SUpgradeMsg, SUpgradeMsg::EPRO_FIGHTPET_FIGHTPROPERTYCHANGE)
struct SAfpfightPropertyChange : public SfpfightPropertyChange
{
	enum	FightProperty
	{
		FP_GONGJI	= 0x01,			// ����
		FP_FANGYU,		// ����
		FP_BAOJI,		// ����
		FP_SHANBI,			// ����

		FP_MAX_HP	,		// �������
		FP_MAX_MP	,			// �������
		FP_MAX_TP		,		// �������

		FP_CUR_HP		,		// ��ǰ����
		FP_CUR_MP		,		// ��ǰ����
		FP_CUR_TP		,		// ��ǰ����
		FP_MAXPROPERTY,
	};
	
	BYTE				m_index;						// ��������
	BYTE			m_type;							//����
	WORD			value;			//�ı��ֵ
};	
//// �������磬��EPRO_UPDATE_STATE_PANEL_DATAͬ��
////=============================================================================================
//// �ȼ��仯
//DECLARE_MSG(SLevelChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_LEVEL_CHANGE)
//struct  SALevelChangeMsg    :   public SLevelChangeMsg
//{
//    DWORD   dwGlobalID;         // ��������ȫ��Ψһ��ʶ��
//    WORD    byLevel;            // ��ǰ�µĵȼ�
//    DWORD   dwMaxHP;             // ��ǰ�������ֵ
//    WORD    wMaxMP;             // ��ǰ�������ֵ
//    WORD    wMaxSP;             // ��ǰ�������ֵ
//    WORD    wSparePoint;        // ��ǰʣ�µķ������
//    UINT64   dwCurExp;           // ��ǰ�µľ���ֵ
//    UINT64   dwMaxExp;           // ��ǰ�µľ���ֵ����
//    BYTE    byAmuck;            // ɱ��
//};
////---------------------------------------------------------------------------------------------

// ������� �ȼ��仯���ֶ�������
DECLARE_MSG(SLevelChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_LEVEL_CHANGE)
struct  SQLevelChangeMsg    :   public SLevelChangeMsg
{
	DWORD   dwGlobalID;         // ��������ȫ��Ψһ��ʶ��
};

// ���÷������
DECLARE_MSG(SSetPointMsg, SUpgradeMsg, SUpgradeMsg::EPRO_SET_POINT)
struct SQSetPointMsg : public SSetPointMsg
{
	enum
	{
		SSP_GONGJI,
		SSP_FANGYU,
		SSP_QINGSHEN,
		SSP_JIANSHEN,
	};
	WORD wPoints[4];	// �����츳
};

//add by yuntao.liu
DECLARE_MSG(SUpdateUsePoints_S2C, SUpgradeMsg, SUpgradeMsg::EPRO_UPDATE_USE_POINTS)
struct SUpdateUsePoints_S2C_MsgBody : public SUpdateUsePoints_S2C
{
	WORD mAttackLeftPoints;
	WORD mDefenseLeftPoints;
};

DECLARE_MSG(SUpdateStatePanelData_S2C, SUpgradeMsg, SUpgradeMsg::EPRO_UPDATE_STATE_PANEL_DATA)
struct SUpdateStatePanelData_S2C_MsgBody : public SUpdateStatePanelData_S2C
{
	DWORD		dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
	WORD		m_WaiGong;			// �⹦
	WORD		m_NeiGong;			// �ڹ�
	WORD		m_WuXing;			// ����
	WORD		m_TiZhi;			// ����
	WORD		m_ShenFa;			// ��
	WORD		m_PhyAtk;			// ������
	WORD		m_FpAtk;			// �ڹ�����
	WORD		m_PhyDefence;		// �������
	WORD		m_FpDefence;		// �ڹ�����
	WORD		m_PhyHit;			// ��������
	WORD		m_FpHit;			// �ڹ�����
	WORD		m_PhyEscape;		// ������
	WORD		m_FpEscape;			// �ڹ����
	WORD		m_OutCriticalAtt;	// �⹦����
	WORD		m_InCriticalAtt;	// �ڹ�����
	INT32		m_PhyEscapeFix;		// ����������
	INT32		m_PhyHitFix;		// ������������
	INT32		m_PhyCriHitFix;		// �����ػ�����
	INT32		m_PhyDamageFix;		// �����˺�����
	INT32		m_FpEscapeFix;		// �ڹ��������
	INT32		m_FpHitFix;			// �ڹ���������
	INT32		m_FpCriHitFix;		// �ڹ��ػ�����
	INT32		m_FpDamageFix;		// �ڹ��˺�����
	WORD		byLevel;			// ��ǰ�µĵȼ�
	DWORD		dwMaxHp;			// ���������
	WORD		dwMaxMp;			// �������
	WORD		dwMaxSp;			// �������
	QWORD		maxExp;				// �����
	QWORD		iExp;				// ��ǰ����
};

DECLARE_MSG(SBuffSynPropertyToSelf_S2C, SUpgradeMsg, SUpgradeMsg::EPRO_BUFF_PRO_SYN_TO_SELF)
struct SABuffSynPropertyToSelf_S2C : public SBuffSynPropertyToSelf_S2C
{
    DWORD		dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
    DWORD		dwCurHP;			// ��ǰ�µ�����ֵ
    WORD		wCurMP;				// ��ǰ�µ�����ֵ
    WORD		wCurSP;				// ��ǰ�µ�����ֵ
	float		fSpeed;				// ��ǰ�µ��ٶ�
    bool		bDead;              // �Ƿ�����
};

DECLARE_MSG(SBuffSynPropertyToSelect_S2C, SUpgradeMsg, SUpgradeMsg::EPRO_BUFF_PRO_SYN_TO_SELECT)
struct SABuffSynPropertyToSelect_S2C : public SBuffSynPropertyToSelect_S2C
{
	DWORD       dwGlobalID;         // ��������ȫ��Ψһ��ʶ��
	bool        bDead;              // �Ƿ�����
	DWORD       dwCurHP;            // ��ǰ�µ�����ֵ
	WORD        wCurMP;             // ��ǰ�µ�����ֵ
	WORD        wCurSP;             // ��ǰ�µ�����ֵ
	DWORD       dwMaxHP;	        // ��ǰ�������ֵ
	WORD        wMaxMP;	            // ��ǰ�������ֵ
	WORD        wMaxSP;	            // ��ǰ�������ֵ
	
	// buff �б��Լ�BUFF��ʱ����Ϣ����

};

DECLARE_MSG(SBuffSynPropertyToAll_S2C, SUpgradeMsg, SUpgradeMsg::EPRO_BUFF_PRO_SYN_TO_ALL)
struct SABuffSynPropertyToAll_S2C : public SBuffSynPropertyToAll_S2C
{
	DWORD   dwGlobalID;             // ��������ȫ��Ψһ��ʶ��
	float	fSpeed;					// ��ǰ���µ��ٶ�
	bool    bAction[12];		    // ��������,12����������
	bool    bDead;                  // �Ƿ�����
};

DECLARE_MSG(SBuffSynEffectToAll_S2C, SUpgradeMsg, SUpgradeMsg::EPRO_BUFF_EFFECT_SYN_TO_ALL)
struct  SABuffSynEffectToAll_S2C : public SBuffSynEffectToAll_S2C
{
	DWORD	dwGlobalID;	            // ��������ȫ��Ψһ��ʶ��
	WORD	wBuffId;	            // buff id
	bool	bIsBegin;	            // �Ƿ����
	bool	bIsOnce;                // �Ƿ���һ����BUFF
};

//end buff syn property

DECLARE_MSG(SQuestAddPoint_C2S, SUpgradeMsg, SUpgradeMsg::EPRO_QUEST_ADD_POINTS)
struct SQuestAddPoint_C2S_MsgBody : public SQuestAddPoint_C2S
{
//	WORD mQuestAddPoints[ SPlayerXiaYiShiJie::ATTACK_DEFENSE_MAX ];
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// �������Եı仯�����������Ժ�
DECLARE_MSG(SBasePointChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_BASEPOINT_CHANGE)
struct  SABasePointChangeMsg    :   public SBasePointChangeMsg
{
    DWORD dwGlobalID;        // ��������ȫ��Ψһ��ʶ��
    WORD wCurEN;             // ���� �����ʣ�
    WORD wCurST;             // ���� ��ǿ׳��
    WORD wCurIN;             // ���� ���ǻۣ�
    WORD wCurAG;             // �� ��������
    WORD wCurLU;             // ���� ��������
    BYTE byPoint;            // ʣ��������
    short sXvalue;           // ����ֵ 
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// �������Եı仯
DECLARE_MSG(SAbilityChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_ABILITY_CHANGE)
struct  SAAbilityChangeMsg : public SAbilityChangeMsg
{
    DWORD dwGlobalID;        // ��������ȫ��Ψһ��ʶ��
    WORD wCurDAM;            // �书��������ʼֵ��4λ���ڣ������ɣ�װ�������������й�
    WORD wCurPOW;            // �ڹ���������ʼֵ��4λ����
    WORD wCurDEF;            // ��������ʼֵ��4λ����
    WORD wCurAGI;            // �����ȳ�ʼֵ��4λ����
    BYTE byDocBuff[4];       // ҽ��BUFF���� ������ʾ�� ���� �⹦ �ڹ� ��
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// ����ֵ�ı仯����Ϊ���������������������Ҷ��ῴ���ģ����Ե�������
DECLARE_MSG(SXVauleChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_XVALUE_CHANGE)
struct  SAXVauleChangeMsg : public SXVauleChangeMsg
{
    DWORD dwGlobalID;         // ��������ȫ��Ψһ��ʶ��
    short sXvalue;            // ����ֵ 
    BYTE byPKValue;          // PKֵ 
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// �书���������ֵ�仯
DECLARE_MSG(SMaxProficChange, SUpgradeMsg, SUpgradeMsg::EPRO_MAXPROFIC_CHANGE)
struct  SAMaxProficChange : public SMaxProficChange
{
    DWORD dwGlobalID;         // ��������ȫ��Ψһ��ʶ��
    DWORD dwMaxProfic[MAX_SKILLCOUNT];    // �书���������ֵ 
	DWORD dwCurPorfic[MAX_SKILLCOUNT];	// �书������
};
//---------------------------------------------------------------------------------------------

DECLARE_MSG(SSkillUpdate, SUpgradeMsg, SUpgradeMsg::EPRO_SKILL_UPDATE)
struct SQSkillUpdate : public SSkillUpdate
{
	DWORD	dwGlobalID;		// ��ұ�ʶ
	//����������
	WORD	wID;			// Ҫ�����ܵ�ID
	WORD    wID1;		//Ҫ��������ID1
	WORD	wID2;		//Ҫ��������ID2
	BYTE    UpdateSkillType;	//�����������ͣ�1Ϊ�����������ܣ�2Ϊ����һ�鼼��
};

struct SASkillUpdate : public SSkillUpdate
{
	enum
	{
		SSU_LEARNED = 1,			// ��ѧϰ
		SSU_UPDATE_SHULIANDU,		// �����ȸı�
		SSU_LEVELUP,					// ����
		SSU_LoadSkill,				// ���ؼ���
		SSU_UpLoadSkill,			// ж�ؼ���
	};

	BYTE	byWhat;					// ����ԭ��
	WORD	wPos;					// ����λ��
	SSkill	stSkill;				// ��������
	BYTE byIsRefreshPain;	//�Ƿ�ˢ����壬Ĭ��Ϊ0ʱҪˢ�£�Ϊ1ʱ��ˢ��
};

// �����ķ��ȼ�
DECLARE_MSG(STelergyUpdate, SUpgradeMsg, SUpgradeMsg::EPRO_TELERGY_UPDATE)
struct SQTelergyUpdate : public STelergyUpdate
{
	DWORD	dwGlobalID;		// ��ұ�ʶ
	WORD	wID;			// Ҫ�����ķ���ID
};

struct SATelergyUpdate : public STelergyUpdate
{
	enum
	{
		SAT_SUCCESS = 1,	
		SAT_INOTHERSTATUS,	// ���ڱ�Ļ�������У���̯etc..��
		SAT_FULLLEVEL,		// �ȼ�����
		SAT_LACK_MONEY_SP,	// ��Ǯ/��������

		SAT_LEARNED,		// ѧϰ
		SAT_LEVELUP,		// ����
	};

	WORD			pos;			// �ķ�λ��
	BYTE			bResult;		// ���
	SXYD3Telergy	m_Telergy;
};

DECLARE_MSG(SOpenSetSkillBaseMsg, SUpgradeMsg, SUpgradeMsg::EPRO_OPEN_SETSKILL)
struct SQOpenSetSkillBaseMsg : public SOpenSetSkillBaseMsg	//�������ҵļ����������
{
};

struct SAOpenSetSkillBaseMsg : public SOpenSetSkillBaseMsg
{
	BYTE      m_UpdatePlayerPattern;		//һ��3����̬����ͨ��̬����̬1����̬2��
	DWORD		m_UpdateCurUsedSkill[10];		//��ǰ���ʹ�õļ��ܡ�0-2Ϊ��ͨ��̬����ʹ�õļ��ܣ�3-5Ϊ��̬1�£�6-8Ϊ��̬2�µģ�9Ϊ����ʹ�õļ���
};

DECLARE_MSG(SUpdateSkillBaseMsg, SUpgradeMsg, SUpgradeMsg::EPRO_UPDATE_PLAYERSKILL)
struct SQUpdateSkillBaseMsg : public SUpdateSkillBaseMsg	//���������ҵļ���������Ϣ
{
	BYTE      m_UpdatePlayerPattern;		//һ��3����̬����ͨ��̬����̬1����̬2��
	DWORD		m_UpdateCurUsedSkill[10];		//��ǰ���ʹ�õļ��ܡ�0-2Ϊ��ͨ��̬����ʹ�õļ��ܣ�3-5Ϊ��̬1�£�6-8Ϊ��̬2�µģ�9Ϊ����ʹ�õļ���
};

struct SAUpdateSkillBaseMsg : public SUpdateSkillBaseMsg
{
	BYTE m_byRet;	//��Ӧ���������1����ɹ���0��ʾʧ��
};

//add by ly 2014/5/16
DECLARE_MSG(SNotityUpdateSkillMsg, SUpgradeMsg, SUpgradeMsg::EPRO_NOTITY_SKILLUPDATED)
struct SANotityUpdateSkillMsg : public SNotityUpdateSkillMsg	//֪ͨ��ң���ǰ��ҵļ��������Ѿ�����
{
	BYTE      m_UpdatePlayerPattern;		//һ��3����̬����ͨ��̬����̬1����̬2��
	DWORD		m_UpdateCurUsedSkill[10];		//��ǰ���ʹ�õļ��ܡ�0-2Ϊ��ͨ��̬����ʹ�õļ��ܣ�3-5Ϊ��̬1�£�6-8Ϊ��̬2�µģ�9Ϊ����ʹ�õļ���
};

//=============================================================================================
// �ķ����ݵı仯
DECLARE_MSG(STetergyChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_TETERGY_CHANGE)
struct  SATetergyChangeMsg : public STetergyChangeMsg
{
    DWORD dwGlobalID;         // ��������ȫ��Ψһ��ʶ��
    // STelergy Tetergy[MAX_EQUIPTELERGY];      // �ķ�����
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// �����ٶȵĸı�
DECLARE_MSG(SActSpeedChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_ACTSPEED_CHANGE)
struct  SAActSpeedChangeMsg : public SActSpeedChangeMsg
{
    DWORD dwGlobalID;         // ��������ȫ��Ψһ��ʶ��
    float fActSpeedDec;      // �����ݾ������ٶ�
};
// buff������Ч֪ͨ
DECLARE_MSG(SBuffFightOutMsg, SUpgradeMsg, SUpgradeMsg::EPRO_BUFFFIGHTOUT_POS)
struct  SABuffFightOutMsg : public SBuffFightOutMsg
{
	DWORD dwGlobalID;         // ��������ȫ��Ψһ��ʶ��
	WORD wEndX;				// ������X
	WORD wEndY;				// ������Y
};

// �����ͨѨ��
DECLARE_MSG(SPassVenapointAskMsg, SUpgradeMsg, SUpgradeMsg::EPRO_PASS_VENAPOINTASK)
struct SQPassVenapointAskMsg : public SPassVenapointAskMsg
{
    DWORD	dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
    BYTE	byVenationNum;		// �������
    BYTE	byVenapointNum;		// Ѩλ���
};

// ��ͨѨ��ȷ��
DECLARE_MSG(SPassVenapointOKMsg, SUpgradeMsg, SUpgradeMsg::EPRO_PASS_VENAPOINTOK)
struct SAPassVenapointOKMsg : public SPassVenapointOKMsg
{
    DWORD	dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
    BYTE	byVenationCount;	// ��ʣѨ������
    BYTE	byVenationNum;		// �������
    BYTE	byVenapointNum;		// Ѩλ���
    DWORD	dwUseTelergyVal;	// �����ĵ��ķ�ֵ
};

//�ı䵱ǰ����չ�ķ�EPRO_SET_EXTRATELERGY
DECLARE_MSG(SSetExtraTelergyMsg, SUpgradeMsg, SUpgradeMsg::EPRO_SET_EXTRATELERGY)
struct SQSetExtraTelergyMsg : public SSetExtraTelergyMsg
{
    BYTE state[4];  // �޸ĺ���ķ���״̬
//	WORD wTelergyID[ MAX_EQUIPTELERGY + MAX_EXTRATELERGY ];  // ����޸ĺ���չ������õ��ķ�IDǰ ���4����ʾ��չ��
};

//��������Ӧ���ͻ��˵��µ��ķ����ݣ���һ��ȫ������
struct SASetExtrTelergyMsg: public SSetExtraTelergyMsg
{
//	SExtraTelergy m_ExtraTelergy;                       // ����װ���ķ�
    WORD nTelergy;                             // ָ��װ���ķ������ж�����Ч��Ϊ0xffffʱ��ʾm_Telergyû�����ݣ�
	// STelergy m_Telergy[ TELERGY_NUM ];        // ����ѧϰ���ķ�
};

// �ķ��ĸı�
DECLARE_MSG(SSetChangeTelergyMsg, SUpgradeMsg, SUpgradeMsg::EPRO_TELERGY_CHANGE)
struct SASetChangeTelergyMsg : public SSetChangeTelergyMsg
{
	bool			m_bIsAddTel;	// �Ƿ������µ��ķ���True��ʾ�����ķ�
	BYTE			m_Pos;			// λ��
	SXYD3Telergy	m_Telergy;		// �ķ�

	SASetChangeTelergyMsg()
	{
		m_bIsAddTel = false;		// false��ʾ�ı��ķ�ֵ
	}
};

//---------------------------------------------------------------------------------------------

// ��չ�ķ���������ϵ�λ�� number 1-4 �ķ��ܱ��8-11
/****************
* 3(10)   4(11) *
*               *
*               *
* 2(9)    1(8)  *
*****************/
// �򿪵�ǰ����չ�ķ���EPRO_OPEN_EXTRATELERGY
DECLARE_MSG(SOpenExtraTelergyMsg, SUpgradeMsg, SUpgradeMsg::EPRO_OPEN_EXTRATELERGY)
struct SQOpenExtraTelergyMsg : public SOpenExtraTelergyMsg
{
    BYTE number;       // �򿪵ĵڼ�������
    BYTE state;        // �޸ĺ���ķ���״̬ 0�ر� 1����
};

//��������Ӧ���ͻ��˵�
struct SAOpenExtrTelergyMsg: public SOpenExtraTelergyMsg
{
     BYTE number;      // �򿪵ĵڼ�������
     BYTE state;       // �޸ĺ���ķ���״̬
};


// �Ṧ���ݵı仯
DECLARE_MSG(SFlyChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_FLY_CHANGE)
struct  SAFlyChangeMsg : public SFlyChangeMsg
{
	int	   type;		//��Դ�� EFlySKillTypeö��
	SSkill FlySkill;
};

// �������ݵı仯
DECLARE_MSG(SProtectedChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_PROTECTED_CHANGE)
struct  SAProtectedChangeMsg : public SProtectedChangeMsg
{
	int	   type;		//��Դ�� EProtectedSkillTypeö��
	SSkill ProtectSkill;
};
//�Ṧ������Ӱ�������
DECLARE_MSG(SFPAttribChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_FP_ATTRIB_CHANGE)
struct SAFPAttribChangeMsg : public SFPAttribChangeMsg
{
	DWORD	dwHP;
	DWORD	dwMaxHP;
	WORD	wSP;
	WORD	wMaxSP;
	BYTE	byMaxJumpTile;

	float	iCurSpeed;

	// ��
	WORD wAgile;

	// ��������ֵ
//	WORD mAttackAndDefenseMin[ SPlayerXiaYiShiJie::ATTACK_DEFENSE_MAX];				
//	WORD mAttackAndDefenseMax[ SPlayerXiaYiShiJie::ATTACK_DEFENSE_MAX];
};
//�Ṧ������ı仯����
DECLARE_MSG(SFPBuffChange, SUpgradeMsg, SUpgradeMsg::EPRO_FP_BUFF)
struct  SAFPBuffChange : public SFPBuffChange
{
	int	   type;				//���ͣ������Ṧ�ͻ��壩		
	int    iID;					//��ID��
	bool   bIsStar;			//true��ʾ����,false��ʾ�������
	INT32 coldeTime;
};

#if 0
//�Ṧ�������ʹ�óɹ���ʱ��֪ͨ�ͻ��˲�����Ч
DECLARE_MSG(SBuffUseSuccess_S2C, SUpgradeMsg, SUpgradeMsg::EPRO_BUFF_USE_SUCCESS)
struct  SBuffUseSuccess_S2C_MsgBody : public SBuffUseSuccess_S2C
{
	int	   type;			//���ͣ������Ṧ�ͻ��壩		
	int    iID;				//��ID��
};
#endif

DECLARE_MSG(SDrugInfoShowMsg, SUpgradeMsg, SUpgradeMsg::EPRO_DRUG_INFO_SHOW)
struct  SADrugInfoShowMsg  :   public SDrugInfoShowMsg
{
	DWORD dwGlobalID;         // ��������ȫ��Ψһ��ʶ��
	DWORD dwDeltaValue;
	enDrug_Type  enDrug;				//�Ƿ��ҩ�ظ�
};
//buffЧ��֪ͨ��˲��
DECLARE_MSG(SSBuffSplitMoveMsg, SUpgradeMsg, SUpgradeMsg::EPRO_BUFF_SPLITMOVE)
struct  SABuffSplitMoveMsg : public SSBuffSplitMoveMsg
{
	DWORD dwGlobalID;         // ��������ȫ��Ψһ��ʶ��
	WORD wEndX;				// ������X
	WORD wEndY;				// ������Y
};

//һЩ��������Ա仯
DECLARE_MSG(SFightPetExtraChange, SUpgradeMsg, SUpgradeMsg::EPRO_FIGHTPET_EXTRAPROPERTYCHANGE)
struct  SAFightPetExtraChange : public SFightPetExtraChange
{
	enum
	{
		CHANGE_TIRED,			//ƣ�Ͷȸı�
		CHANGE_RELATION,		//���ܶȸı�
		CHANGE_MOOD,			//����ı�
	};
	BYTE index;						//��������
	BYTE	 changeType;			// �ı�����
	WORD wchangeValue;		//�ı��ֵ
};

//���͸��¼�����Ϣ
DECLARE_MSG(SfpSkillUpdate, SUpgradeMsg, SUpgradeMsg::EPRO_FIGHTPET_SKILLUPDATE)
struct SQfpSkillUpdate : public SfpSkillUpdate
{
	enum
	{
		SSU_LEARNED = 1,								// ��ѧϰ/װ������
		SSU_UPDATE_SHULIANDU,					// �����ȸı�
		SSU_LEVELUP,										// ����
		SSU_LEARNEDBYANOTHER,					// ��������������ѧϰ
	};
	BYTE			byWhat;			// ����ԭ������
	WORD		wID;			// Ҫ��������װ��ѧϰ���ܵ�ID
	BYTE			index;				//Ҫѧϰ���ܵ���������
	BYTE			anotherindex;	//�����ͣ����������ѧϰ���м���
};

struct SAfpSkillUpdate : public SfpSkillUpdate
{
	enum
	{
		SSU_LEARNED = 1,			// ��ѧϰ
		SSU_UPDATE_SHULIANDU,		// �����ȸı�
		SSU_LEVELUP,					// ����
		SSU_EQUIPSKILL,					// װ���ڼ���ѧϰ���ļ�����
		SSU_UNEQUIPSKILL,					// ɾ��װ���ڼ���ѧϰ���ļ�����
	};

	BYTE	byWhat;					// ����ԭ��
	WORD	wPos;					// ����λ��
	BYTE		index;					//��������
	DWORD itemID;				//װ���ڼ���ѧϰ���ļ�����ID
	SPetSkill spSkill;				// ���ͼ�������
};

//���Ա仯��Ϣ
DECLARE_MSG(Sallpropertychange, SUpgradeMsg, SUpgradeMsg::EPRO_ALL_PROPERTYCHANGED)
struct SAallpropertychange : public Sallpropertychange
{
	enum	FightProperty
	{
		PY_GONGJI	= 0x01,			// ����
		PY_FANGYU= 0x02,	// ����
		PY_BAOJI= 0x04,	// ����
		PY_SHANBI= 0x08,			// ����

		PY_MAX_HP= 0x10,		// �������
		PY_MAX_MP= 0x20,			// �������
		PY_MAX_TP		= 0x40,		// �������

		PY_CUR_HP		= 0x80,		// ��ǰ����
		PY_CUR_MP		= 0x100,		// ��ǰ����
		PY_CUR_TP		= 0x200,		// ��ǰ����
		PY_MAXPROPERTY= 0x400,
	};
	enum {BUFFSIZE = 256,};
	enum{PROPERTYNUM = 10,};
	WORD			m_type;							//����
	WORD			messagesize;					//����
	BYTE			valueBuff[BUFFSIZE];			//�ı��ֵ
};

// ���ϴ��
DECLARE_MSG(SSResetPointMsg, SUpgradeMsg, SUpgradeMsg::EPRO_RESET_POINT)
struct SQResetPointMsg : public SSResetPointMsg
{
	enum PointType
	{
		PT_JINGONG,
		PT_FANGYU,
		PT_QINGSHEN,
		PT_JIANSHEN,

		PT_ALL,
	};

// 	bool	bBuy;				// �����Ƿ���
// 	BYTE	type;				// ϴ�����ͣ�PT_ALL��ʾȫ��ϴ�㣬��ʱ��point = 0
	WORD	point[PT_ALL];				// ϴ����
	//WORD	itemCount;			// ��Ҫ�������������߲���ʱ����ʾ��Ҫ�ĵ�����������������˱���Ϊ0��
};

struct SAResetPointMsg : public SSResetPointMsg
{
	enum
	{
		RPM_FAIL,
		RPM_SUCCESS,
	};

	BYTE	result;
};

//��Ϊֵ����
DECLARE_MSG(SXwzUpdateMsg, SUpgradeMsg, SUpgradeMsg::EPRO_XWZ_UPDATE)
struct  SAXwzUpdateMsg : public SXwzUpdateMsg
{
	DWORD xwzValue;         // ��Ϊֵ
};

// �������ʱ�ĸ�����Ϣ�������͸��������������
DECLARE_MSG(SPlayerDeadAddMsg, SUpgradeMsg, SUpgradeMsg::EPRO_PLAYERDEAD_ADD)
struct  SAPlayerDeadAddMsg : public SPlayerDeadAddMsg
{
	WORD wFreeReliveTime;  //�����ڸ���ʱ��
	WORD wReliveCost; //ԭ�ظ����Ǯ
	WORD activityAttkCost;//������������
	WORD activityDefenceCost;//������������
// 	char killname[CONST_USERNAME];
// 	WORD skillid;
// 	struct relivecon
// 	{
// 		WORD wReliveTime;
// 		WORD wReliveMoney;
// 		BYTE		bMoneytype;
// 	};
// 	relivecon wreliveconType[4];
};

//add by ly 2014/3/25

//��ȡ�����ҫ��Ϣ
DECLARE_MSG(SGloryMsg, SUpgradeMsg, SUpgradeMsg::EPRO_PLAYERGETGLORY)
struct  SQGloryMsg : public SGloryMsg
{
};

struct  SAGloryMsg : public SGloryMsg
{
	DWORD m_PlayerGloryDot;		//�����ҫ��
	WORD m_GloryArrSize;	//��ҫ����Ĵ�С
	WORD* m_pGloryInfArr;	//��ҫ���ݺ����״̬����1λΪ���״̬��1��0��������15λΪ��ҫID
	SAGloryMsg(DWORD PlayerGloryDot, WORD GloryArrSize) :m_PlayerGloryDot(PlayerGloryDot), m_GloryArrSize(GloryArrSize)
	{
		if (m_GloryArrSize == 0)
			m_pGloryInfArr = NULL;
		else
			m_pGloryInfArr = new WORD[m_GloryArrSize];
	}

	//������ҫ��Ӧ������ֵ
	void SetGloryData(BYTE FinishFlag, BYTE GloryID, WORD GloryArrIndex)
	{
		WORD GloryData = ((WORD)FinishFlag << 15) + GloryID;
		m_pGloryInfArr[GloryArrIndex] = GloryData;
	}

	~SAGloryMsg()
	{
		if (m_pGloryInfArr != NULL)
		{
			delete[] m_pGloryInfArr;
			m_pGloryInfArr = NULL;
		}
	}
};

//��ȡ��ҫ����
DECLARE_MSG(SGetGloryMsg, SUpgradeMsg, SUpgradeMsg::EPRO_GETPLAYERGLORY)
struct  SQGetGloryMsg : public SGetGloryMsg
{
	WORD m_GloryID;		//��ҫID
};

struct SAGetGloryMsg : public SGetGloryMsg
{
	BYTE m_Ressult;		//��ȡ���������1�ɹ��� 0ʧ�ܣ�
};

//�ƺ���Ϣ
DECLARE_MSG(STitleMsg, SUpgradeMsg, SUpgradeMsg::EPRO_TITLE)
struct  SQTitleMsg : public STitleMsg
{
	enum MsgType
	{
		EXCHANGE,	//����
		USE,	//ʹ��
		UNUSE,	//ȡ��ʹ��
	};
	BYTE m_MsgType;	//��Ϣ����
	WORD m_TitleID;		//�ƺ�ID
};

struct SATitleMsg : public STitleMsg
{
	BYTE m_Ressult;		//���������1�ɹ��� 0ʧ�ܣ�
	BYTE m_TitleSize;	//�Ѿ�ӵ�еĳƺŴ�С
	DWORD m_GloryDot;	//������ҫ��
	BYTE *m_TitleArr;	//�ƺ�����
	SATitleMsg() 
	{ 
		m_TitleArr = NULL; 
		m_TitleSize = 0; 
		m_Ressult = 0; 
	}
	~SATitleMsg()
	{ 
		if (m_TitleArr != NULL)
			delete[] m_TitleArr; 
		m_TitleArr = NULL; 
	}
};

//add by ly 2014/7/22
//������а����ݽṹ
struct PlayerRankList
{
	char m_PlayerName[CONST_USERNAME];		//�������
	BYTE m_PlayerLevel;		//��ҵȼ�
	BYTE m_PlayerSchool;	//�������
	WORD m_PlayerCurRank;	//��ҵ�ǰ����
	DWORD m_PlayerPower;	//���ս����
	DWORD m_PlayerGlod;		//��ҽ����
};

//�������а�����
DECLARE_MSG(SPlayerRankList, SUpgradeMsg, SUpgradeMsg::EPRO_QUEST_RANDLISTINF)
struct SQPlayerRankList : public SPlayerRankList
{
	enum{ PLAYYERPOWER, PLAYERLEVEL, PLAYERGLOD };
	BYTE m_Type;	//�������ͣ�ս�������ȼ����������
};
struct SAPlayerRankList : public SPlayerRankList
{
	BYTE m_Type;	//Ӧ�����ͣ�ս�������ȼ����������
	WORD m_TopNSize;	//�����а���ǰN��������ݴ�С
	WORD m_PlayerTopNSize;	//�������ǰ��N��������ݴ�С����������Լ���
	PlayerRankList *m_pPlayerRankList;
};


//��ȡ���а��Ӧ�Ľ���
DECLARE_MSG(SGetRankAward, SUpgradeMsg, SUpgradeMsg::EPRO_RANKLIST_GETWARD)
struct SQGetRankAward : public SGetRankAward
{
	enum{ PLAYYERPOWER = 1, PLAYERLEVEL, PLAYERGLOD };
	BYTE m_Type;	//�������ͣ�ս�������ȼ����������
};
struct SAGetRankAward : public SGetRankAward
{
	BYTE m_Result;	//�������
};

//��ȡ������а�����ȡ״̬
DECLARE_MSG(SGetRankAwardState, SUpgradeMsg, SUpgradeMsg::EPRO_RANKLIST_GEAWARDSTATE)
struct SQGetRankAwardState : public SGetRankAwardState
{
};
struct SAGetRankAwardState : public SGetRankAwardState
{
	BYTE m_RankState[3];	//����Ϊ��ս�������н�����ȡ״̬����ҵȼ����а�����ȡ״̬����ҽ�����а�����ȡ״̬
};

//add by ly 2014/7/23
struct ActivenessItem
{
	BYTE m_ItmeNum;		//��������
	DWORD m_ItmeID;		//����ID
};

#define ACTIVENESSITEMMAXNUM 4

struct ActivenessAward
{
	BYTE m_AwardLevel;	//�����ȼ�
	BYTE m_NeedActivenessNum;	//��Ҫ��Ծ������
	BYTE m_IsGetAwardFlag;	//ÿ���׶εĽ����Ƿ���ȡ��ʶ0��ʾ������ȡ��1��ʾ������ȡ��2��ʾ����ȡ
	ActivenessItem m_ItemInfo[ACTIVENESSITEMMAXNUM];	//�����ĵ�����
};

struct ActivenessTaskInf
{
	BYTE m_TaskCompleteTimes;	//������ɴ���
	BYTE m_TaskTotalTimes;	//�����ܴ���
	BYTE m_SingleTaskGetNum;	//���λ�õĻ�Ծ������
	BYTE m_TaskIndex;	//��������
};

//��ȡ��Ծ�������Ϣ
DECLARE_MSG(SGetActivenessInfo, SUpgradeMsg, SUpgradeMsg::EPRO_ACTIVENESS_GETINFO)
struct SQGetActivenessInfo : public SGetActivenessInfo
{
};
struct SAGetActivenessInfo : public SGetActivenessInfo
{
	BYTE m_ActivenessValue;	//��Ծ��
	BYTE m_ActivenessAwardNum;	//��Ծ�Ƚ�������
	BYTE m_ActivenessTaskNum;	//��������
	ActivenessTaskInf *m_pTaskInfo;	//������Ϣ
	ActivenessAward *m_pAwardInfo;	//������Ϣ
};

//��ȡ��Ծ�ȵĶ�Ӧ����
DECLARE_MSG(SGetActivenessAward, SUpgradeMsg, SUpgradeMsg::EPRO_ACTIVENESS_GETAWARD)
struct SQGetActivenessAward : public SGetActivenessAward
{
	BYTE m_AwardLevel;	//��ȡ�õȼ��Ľ���
};
struct SAGetActivenessAward : public SGetActivenessAward
{
	BYTE m_Result;	//��ȡ���
};
