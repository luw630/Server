#pragma once

#include "NetModule.h"
#include "movetypedef.h"
#include "StallDef.h"

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ����������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SRegionBaseMsg, SMessage, SMessage::EPRO_REGION_MESSAGE)
//{{AFX
EPRO_DEL_OBJECT,		// ɾ�������ϵĶ���
EPRO_DEL_OBJECTLIST,	// ɾ�������ϵĶ������
EPRO_SYN_PLAYER_INFO,	// ͬ�������Ϣ
EPRO_SYN_NPC_INFO,		// ͬ��NPC��Ϣ
EPRO_SYN_MONSTER_INFO,	// ͬ��������Ϣ
EPRO_SET_EFFECT,	    // ������ĳ��������Ҫ���ֵ�Ч��(����Ч��)
EPRO_OBJECT_INFO,	    // �����ȡ������ĳ�����������
EPRO_SET_REGION,	    // ֪ͨ�ͻ������ó���
EPRO_SYN_OBJECT,	    // ֪ͨ�����ϳ�����ĳ������
EPRO_SYN_SALENAME,	    // ��̯��Ϣ 
EPRO_SETEXTRASTATE,     // ���ø���״̬
EPRO_SETPKRULE,			// ����PK����
EPRO_ONMUTATE,          // ���ñ���״̬
EPRO_ONSCAPEGOAT,       // ���ñ���״̬

EPRO_QUERY_SIMPLAYER,   // ��ѯ��ɫ��Ϣ added by yg
EPRO_SET_REGION_TASK,   // �ܵ�ͼ �������
EPRO_SYN_PET_INFO,
EPRO_SYN_EFFECTS,  //ͬ�������Ч
//}}AFX
END_MSG_MAP()

// ֪ͨ�����ϳ�����ĳ������
DECLARE_MSG(SSynObjectMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_OBJECT)
struct SASynObjectMsg : public SSynObjectMsg
{
	DWORD	dwGlobalID;		// �����ȫ��ID���
	DWORD	dwExtra;		// ��չ
};

//=============================================================================================
// �ͻ��������ȡ������ĳ�����������
DECLARE_MSG(SObjectInfoMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_OBJECT_INFO)
struct SQObjectInfoMsg : public SObjectInfoMsg 
{
    enum GETINFO_TYPE {
        GIT_ALL,
        GIT_SALENAME
    };

    WORD  wInfoType;    // �������Ϣ����
	DWORD dwGlobalID;	// �����ȫ��ID���
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ������Ҫ��ͻ���ɾ�������ϵĶ���
DECLARE_MSG(SDelObjectMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_DEL_OBJECT)
struct SADelObjectMsg : public SDelObjectMsg 
{
	DWORD dwGlobalID;	// �����ȫ��ID���
};

DECLARE_MSG(SDelObjectListMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_DEL_OBJECTLIST)
struct SADelObjectListMsg : public SDelObjectListMsg 
{
	WORD  num;
	DWORD dwGlobalIDs[1024];
};

DECLARE_MSG(SSaleNameMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_SALENAME)
struct SASendSaleNameMsg : public SSaleNameMsg
{
    DWORD	dwGlobalID;						// ��������ȫ��Ψһ��ʶ��
    char    szStallName[MAX_STALLNAME];		// ��ҵ�̯λ����
};

// ������Ҫ��ͻ���ͬ����ɫ��Ϣ
DECLARE_MSG(SSynPlayerMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_PLAYER_INFO)
struct SASynPlayerMsg : public SSynPlayerMsg
{
	enum
	{
		SPT_WEAPON = 0,
		SPT_WRIST,
		SPT_SHOSE,
		SPT_CLOTH,
		SPT_MANTLE,
		SPT_PENDANTS1,
		SPT_PENDANTS2,
		SPT_PRECIOUS,
		SPT_HAT,
		SPT_SHIZHUANG_HAT1,			// ʱװͷ��1
		SPT_SHIZHUANG_CLOTH1,		// ʱװ�·�1
		SPT_PAT,				// ����
		SPT_WAIST,				// ���
		SPT_PROTECTION,			// ����
	};

	enum
	{
		COLOR_NORMAL,			//������ɫ,
		COLOR_YELLOW,			//��ɫ
		COLOR_LIGHTRED,		//ǳ��ɫ
		COLOR_RED,					//��ɫ
	};

	enum
	{
		NORMALMODE,		//��ͨģʽ
		FASHIONMODE,		//ʱװģʽ
	};

	BYTE		byCurTitle;		//��ǰ��ҳƺ�
	DWORD		dwGlobalID;				// ���GID
	DWORD		dwShowState;			// ��ұ���״̬
	char		sName[CONST_USERNAME];	// �����
	char		sTranPetName[PETNAMEMAXLEN];	// ���������
	BYTE		bySex:1;				// �Ա�
	BYTE		byBRON:3;				// ����
	BYTE		bySchool:4;				// ����
	BYTE		byFace;					// ����
	BYTE		byHair;					// ͷ��
	DWORD		MountID;				// ͬ����ɫ�����16λ���ID����16λ��˵ȼ���
	WORD		PetID;					// ͬ����ս������
	DWORD		PetGlobalID;			// �����ΨһID
	DWORD		PetTransferID;	//����ĳ���ID
	float		mMoveSpeed;				// �ٶ�
	float		direction;				// ����
	WORD		wAtkSpeed;				// �����ٶ�
	DWORD		wEquipIndex[SPT_PROTECTION ];			// 13�ֿ���ʾ��װ��,����2��ʱװ
	SPath		ssp;					// ͬ��·��
	BYTE			bNameColor;	//������ɫ
	BYTE			bFashionMode;		//ʱװģʽ������ͨģʽ

	int MySize()
	{
		return sizeof(SASynPlayerMsg) - (MAX_TRACK_LENGTH - ssp.m_wayPoints)*sizeof(SWayPoint);
	}
};

/*
//=============================================================================================
// ������Ҫ��ͻ���ͬ����ɫ��Ϣ
DECLARE_MSG(SSynPlayerMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_PLAYER_INFO)
struct SASynPlayerMsg : public SSynPlayerMsg 
{
	DWORD	dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
    BYTE    bySex:1;            // ��ҽ�ɫ�Ա�
    BYTE    byBRON:3;           // �������ʾͼƬ����==1��ʾ��ʦ
    BYTE    bySchool:4;         // �����������
	BYTE	byPKValue;			// PKֵ����������
	short	sXValue;			// ����ֵ
	BYTE    bCittaLevel;        // ����ķ��ȼ�
	char	sName[MAX_NAMELEN];			// ��0��β�������ַ���
	char	sTongName[CONST_USERNAME];		// ������ƣ�ͬʱΪ���ڲ��Ҹð��ɵĹؼ���
	char	sTitle[CONST_USERNAME];			// ����ҵ�ǰ��ʾ�ĳƺ�

	BYTE	isSaleState;	    // �Ƿ��ǰ�̯״̬
    BYTE	isFactionMaster:1;	// �Ƿ��ǰ���ͷ��
    BYTE	byOnlineState:7;    // �������״̬

    DWORD   dwExtraState;       // ��ҵ�һЩ����״̬����̯=0x01��û�ã������ƾ���=0x02�������ƾ���=0x04

	WORD	wMutateID;			// ��ҵ�ǰ����ı��
    WORD    m_wEquipCol[EEC_MAX];       // ����·�����ɫֵ 

    BYTE    m_byEquipColCount[EEC_MAX]; // ��ǰ��ɫ����

	float	mMoveSpeed;

    BYTE    dwVenationState[4];   // ��������״̬��ÿ��BYTE��ʾһ������״̬�����Σ���̫�������ˡ����ˡ����ˡ����ѣ�������������̫����������

    float   fActFrameDec;       // �����ݾ������ٶ�

    WORD    wScriptIcon;        // �ű�Сͼ��
    WORD    wScapegoatID;       // ��ҵ�ǰ����ı�� ��0=û������
    WORD    wGoatEffectID;      // ��ҵ�ǰ�����Ч�����

    DWORD   dwSysTitleID;       // ϵͳ�ƺ�ͼ��ID m_dwSysTitle[1]
    DWORD   dwFactionTitleID;   // ����״̬ͼ��ID 

    char    sSpouseName[MAX_NAMELEN];    // ��ż������

	WORD	wCheckID;           // ����ˢ�µİ汾��
	DWORD   dwConsumePoint;     //���ѻ���
    DWORD   fsicons;            // fight state icons setting
	WORD	wGemEff;			// ��ʯ��Ч 0xff | 0xff ( ��λ���� ��λ��Ч�ȼ� )
	WORD    wTempItem[10];      // ������ϵ���ʱ����

    char masterName[CONST_USERNAME];     // ʦ������
    char prenticeName[CONST_USERNAME];   // ͽ������
    char szUnionName[13];    // ��������

	enum EQUIP_TYPE_SYNC
	{
		ETS_HAT = 0,
		ETS_CLOTH,
		ETS_SHOSE,
		ETS_WRIST,
		ETS_WAIST,
		ETS_MANTLE,
		ETS_WEAPON,

		ETS_MAX,
	};
	WORD wEquipIndex[ETS_MAX]; // ȫ����Ҫ��װ������װ����INDEX
	BYTE bySkillState;		 // ����书����
    WORD wMedicalEthics;     // ҽ��ֵ
    WORD wMaxPerDayME;       // �����ۼ���
	WORD wLevel;			 // ��ҵȼ�
	char sIconName[CONST_USERNAME];		// ����
	//add by xj
	int  iMaxHP;			//ͬ��Ѫ��
	int  iMaxMP;
	int  iMaxSP;
	int  iCurHP;
	int  iCurMP;
	int  iCurSP;
	WORD wPracticeType;		//ͬ������״̬
	bool bIsStar;			//�Ƿ�ʼ����
	//add by dc
	int	 MountID;			// ͬ����ɫ����, this is mount id

	SPath ssp;
};// */
//---------------------------------------------------------------------------------------------

// ͬ�����͵�����
DECLARE_MSG(SSynPetMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_PET_INFO)
struct SASynFightPetMsg : public SSynPetMsg
{
	DWORD	dwGlobalID;	// ID;
	DWORD	dwOwnerID;	// ����ID;
	WORD	ID;			// ����ID
	char	sName[PETNAMEMAXLEN];
	float	mMoveSpeed;	// �ٶ�
	float	direction;	// ����
	SPath	ssp;		// ͬ��·��
};

// ͬ��NPC������
DECLARE_MSG(SSynNpcMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_NPC_INFO)
struct SASynNpcMsg : public SSynNpcMsg 
{
	DWORD	wImageID;			// ��NPC��ͼƬ����
	DWORD	wClickScriptID;		// �ű�ID
	DWORD	dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
	DWORD	dwControlId;		// ����ID
	WORD	wChose;				// �ɷ�ѡ��
	float	byDir;			    // ��NPC�ķ���
	float	mMoveSpeed;			// �ٶ�
	SPath	ssp;				// У���������ݣ�����Ϊ���һ����
};

// ͬ�����������
DECLARE_MSG(SSynMonsterMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_MONSTER_INFO)
struct SASynMonsterMsg : public SSynMonsterMsg 
{
	DWORD	dwGlobalID;			// ����ID
	char	sName[CONST_USERNAME];			// ��0��β�������ַ���
	BYTE	bChangeType;		// ɫ������
	BYTE bMonsterType;//��������  0 1 2 ��ͨ����Ӣ��BOSS
	int		ilevel;				// �ȼ�
	WORD	mImageID;			// ģ��ID
	WORD	monsterID; //�����ñ���ID
	float	mMoveSpeed;			// �ٶ�
	float	direction;			// ����
	float	fScale;				// ģ�ʹ�С
	DWORD dfightState;//ս��״̬
	SPath	ssp;				// ͬ��·��

	/*
	WORD	wTypeID;			// �ù����ͼƬ���ͣ�Ӧ����ͼƬ��Űɣ�
	DWORD	dwGlobalID;			// ��������ȫ��Ψһ��ʶ��  
	DWORD   dwExtraColor;		// ��ǰ�ù������ɫ����

	char	sName[CONST_USERNAME];			// ��0��β�������ַ���
	WORD	wDeadTimes;			// �ù����ͼƬ���ͣ�Ӧ����ͼƬ��Űɣ���
	
    DWORD   fsicons;            // fight state icons setting
    DWORD   dwHalo;             // �⻷ 
	//////////////////////////////////////////////////////////////////////////
	int		iHeadImageNum;		//����ͷ����
	int     ilevel;				//����ȼ�

	DWORD	dwControlId;
	//*/
};
//---------------------------------------------------------------------------------------------

  // ������ת������Ϣ
DECLARE_MSG(SSetRegionMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SET_REGION)
struct SQSetRegionMsg : public SSetRegionMsg 
{
	DWORD	dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
	WORD	wConfigSerialID;	// �������кţ����ڷ�������֤
};

struct SASetRegionMsg : public SSetRegionMsg 
{
	WORD	wMapID;				// Ŀ�곡���ı��
	WORD	wCurRegionID;		// Ŀ�곡���ı��
	float	x;
	float	y;
	float	z;
	WORD	wPkType;		// PK����  0 ����PK 1����PK
};
//---------------------------------------------------------------------------------------------

// ������ĳ��������Ҫ���ֵ�Ч��
DECLARE_MSG(SSetEffectMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SET_EFFECT)
struct SASetEffectMsg : public SSetEffectMsg
{
    enum
    {
        EEFF_LEVELUP,           // �����������
		EEFF_UPDATETELERGY,		// �ķ�������
		EEFF_UPDATESKILL,		// ����������
    };

    BYTE effect;

    union
    {
        struct SLevelUp
        {
            DWORD gid;			// �����Ķ���
        } LevelUp;

		struct UpdateTelergy
		{
			DWORD	dwGID;		// �����Ķ���
			WORD	wID;		// �������ķ�ID
			BYTE	bLevel;		// �ȼ�
		} Telergy;

		struct UpdateSkill
		{
			DWORD	dwGID;		// �����Ķ���
			WORD	wID;		// ���ܵ�ID
			BYTE	bLevel;		// ���ܵĵȼ�
		} Skill;
    } data;
};

// ���ø���״̬
DECLARE_MSG(SSetExtraStateMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SETEXTRASTATE)
struct SASetExtraStateMsg : public SSetExtraStateMsg 
{
    DWORD	dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
    DWORD   dwExtraState;       // ��ҵĸ���״̬
};

//=============================================================================================
// ����PK����
DECLARE_MSG(SPKRule, SRegionBaseMsg, SRegionBaseMsg::EPRO_SETPKRULE)
struct SQPKRule : public SPKRule 
{
	BYTE	byPKRule;			// PK���� 0:��ƽģʽ  1:����ģʽ   2:����ģʽ   3:����ģʽ  4��ȫ��ģʽ
};
struct SAPKRule : public SPKRule 
{
	BYTE	byPKRule;			// PK���� 0:��ƽģʽ  1:����ģʽ   2:����ģʽ   3:����ģʽ  4��ȫ��ģʽ
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// ������ͨ��ORB��ѯ�����Ϣ
DECLARE_MSG(SQueryPlayerMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_QUERY_SIMPLAYER)
struct SQQueryPlayerMsg :
    public SQueryPlayerMsg
{
	char szName[CONST_USERNAME];			// Player Name to query
	BYTE mode;                  // ��ѯģʽ��0=���� 1=������ͨ��ѯ 2=���˴�̽
};

///���ǿ������ѯ�����Ϣ�����
struct SAQueryPlayerMsg :
    public SQueryPlayerMsg
{
    WORD    wServerID;          // ��������������ID
    DNID	  dnidClient;         // ����������ϸ���ҵ�DNID
	char		  szName[CONST_USERNAME];				//����
	BYTE		  bySchool;					//����
	bool		  bSex;						//�Ա�
	WORD	  wLevle;				//�ȼ�
	WORD	wRegion;				//���ڳ���ID
// 	char	szTitle[CONST_USERNAME];					
// 	char	szFactionName[CONST_USERNAME];
// 	WORD	wPKValue;
// 	int		nXValue;
 	
// 	WORD	wXPos;
// 	WORD	wYPos;
// 	BYTE    mode;
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// ���ø���״̬ - ����
DECLARE_MSG(SChangeMutateMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_ONMUTATE)
struct SQChangeMutateMsg : public SChangeMutateMsg 
{
    DWORD	dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
    BYTE    byMutateType;       // �������ͣ�0��������1������2����NPC��
    WORD	wMutateID;          // ������ͼƬID
};
struct SAChangeMutateMsg : public SChangeMutateMsg 
{
    DWORD	dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
    BYTE    byMutateType;       // �������ͣ�0��������1������2����NPC��
    WORD	wMutateID;          // ������ͼƬID
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// ���ø���״̬ - ����
DECLARE_MSG(SSetScapegoatMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_ONSCAPEGOAT)
struct SASetScapegoatMsg : public SSetScapegoatMsg 
{
    DWORD	dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
    WORD	wScapegoatID;       // �����ͼƬID
    WORD	wGoatEffectID;      // �����Ч��ID
};


//ͬ�������Ч���Ժ�������Ч������ʹ��
DECLARE_MSG(SSynEffectsMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_EFFECTS)
struct  SASynEffectsMsg : public SSynEffectsMsg
{
	enum
	{
		EFFECT_ZHENQI,   //������Ч
	};
	WORD  weffectsindex;//��ЧID
	DWORD dwgid;//��Ч���ͷ���  ȫ��GID,
	DWORD dwgidatter;//��Ч�ͷ��� 
	float feffectPosX;//�ͷ��� ����
	float feffectPosY;
};
//---------------------------------------------------------------------------------------------
