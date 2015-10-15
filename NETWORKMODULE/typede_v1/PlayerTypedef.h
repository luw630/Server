#pragma once
#define GMMODULEON
///////////////////////////////////////////////////////////////////////////////////////////////
// ���ݶ���ԭ��
// ��������Ķ��ǻ�ȫ���õ������ݽṹ
///////////////////////////////////////////////////////////////////////////////////////////////
static const int MAX_FRIEND_NUMBER = 50;
static const int MAX_BLOCK_NUMBER = 30;
static const int MAX_MATE_NUMBER = 10;

static const int MAX_EQUIPTELERGY = 10;        // ����װ�����ķ�����
static const int MAX_EXTRATELERGY = 4;		// �����ķ�

// �౶
const WORD N_MULTIPLES  = 2;  
//static WORD s_wMulHours = 16; //�Ƶ�����������Ԫ��Environment����������


//------------------ ��ͻ��˹������ݽṹ ----------------------

enum    ESHOW_EXTRASTATE                        // ��Ҹ���״̬���
{
    SHOWEXTRASTATE_NONE     = 0x00000000,		// ��״̬
    SHOWEXTRASTATE_SALE     = 0x00000001,		// ��̯����ʱû�õ���
    SHOWEXTRASTATE_CURE     = 0x00000002,		// ����
    SHOWEXTRASTATE_BECURE   = 0x00000004,		// ������
	SHOWEXTRASTATE_STARTZAZEN= 0x00000007,		// ������Ҫ�����״̬
    SHOWEXTRASTATE_SCRIPT   = 0x00000008,       // �ű�״̬
    SHOWEXTRASTATE_SCORES_LEVEL = 0x00000010,   // ����10�����
    SHOWEXTRASTATE_SCORES_MONEY = 0x00000020,   // ����10�󸻺�
    SHOWEXTRASTATE_SCORES_FAMEG = 0x00000040,   // ����10������
    SHOWEXTRASTATE_SCORES_FAMEX = 0x00000080,   // ����10�����
	SHOWEXTRASTATE_ONMOSMUTATE	= 0x00000100,   // MOG����״̬
    SHOWEXTRASTATE_ONNPCMUTATE	= 0x00000200,   // NPC����״̬
    SHOWEXTRASTATE_ONGOATMUTATE	= 0x00000400,   // �������״̬
    SHOWEXTRASTATE_ONESPMUTATE	= 0x00000800,   // �������״̬�����ȣ�
	//SHOWEXTRASTATE_ONGOAT		= 0x00000400    // ����״̬(��ʱ����)
    SHOWEXTRASTATE_SPORT   = 0x0001f000,        // ����״̬
};


// ---  ���� ---------------------------------------------
// -------------------------------------------------------
struct SRoleTask
{
	WORD wTaskID;               // �����   ����ſɸ�NPC�������Ľű���һ��  Ҳ����ר��ָ��������� 
	WORD byGoodsID;             // ���������Ҫ����ƷID�� ��������
    WORD wNextGoods;            // Ԥ����һ������Ҫ����ƷID��
	WORD byNeedNum;             // ���ڿ�����Ϊ��Ҫ��ɵ�������
    WORD byFinished;            // ����ɵ�������
	WORD wForgeGoods[15];       // ����������Ҫ
    DWORD wComplete;              // ������\״ֵ̬
    bool bSave;                 // �Ƿ񱣴�
    char m_szName[11];	        // ��Ҫ�ض�����
	BYTE byTaskType;            // ��������
};

// --- ���NPC������ȴ��ı�־ ��ʵû�й���ֻ�б�־���� 
//     ��byGoodsID = 0 ʱ Ĭ��Ϊ����������� 
typedef struct _RECVITEMINFO {
    WORD wPosX;
    WORD wPosY;
    WORD wItemID;
}SITEMINFO, *LPSITEMINFO;
// -------------------------------------------------------

struct  STelergy
{
    DWORD   dwTelergyVal;       // �ķ�ֵ
    WORD    wTelergyID;        // �ķ�ID
    BYTE    byTelergyLevel;     // �ķ��ȼ�
};

struct SExtraTelergy
{
    STelergy telergy[2];
	BYTE  state[2];         // �������ӵļ���״̬, 
    BYTE  margin;           // �����ǰ������ӽ��������жϣ�������Ҫ���ӷ��Ӽ��������
	DWORD charge;           // ��ֵʱ�䣨�Ե����ƣ�
	DWORD segment;          // ��������ʱ��( �������һ�������Ѿ�����, �ټ���ڶ���ʱ�Ͳ�������ʱ����)
};

struct SFixBaseData // �����ҵĻ�����ֵ
{
	DWORD	m_version;				// �汾�ţ����ڶ�����������չ��
    DWORD	  m_dwConsumePoint; //������ѻ���
    // ���ƣ���Ϊ����Ϸ����ҵ����Ʋ��ᣨ���Բ��ܣ��ظ��������ƶ�Ϊ�ý�ɫ�Ĺؼ��֣����Բ������������г���
    BYTE    m_bySex:1;              // ��ҽ�ɫ�Ա�
    BYTE    m_byBRON:3;             // �������ʾͼƬ����==1��ʾ��ʦ
    BYTE    m_bySchool:4;           // �����������
    //BYTE	m_byBron;			    // ����ҳ��������Ƿ��ʦ����������Ծ��������ʹ�ú��ֽ�ɫͼƬ

    WORD	m_byLevel;			    // ��ҵĵȼ���255������ -->�ָ�ΪWORD, 65535����

    //DWORD   m_dwMaxExp;           // ��ҵĵ�ǰ����ֵ����
    __int64	m_iExp;			        // ��ҵĵ�ǰ����ֵ
    DWORD	m_dwMoney;			    // ��ҵ�ǰ��Я���Ľ�Ǯ
	DWORD	m_dwYuanBao;			// ��ҵ�ǰ��Я����Ԫ��
	DWORD	m_dwZenBao;			    // ��ҵ�ǰ��Я��������

    DWORD	m_dwBaseHP;			    // ����������
    WORD	m_wBaseMP;			    // ��������
    WORD	m_wBaseSP;			    // ��������

    WORD    m_wMaxHPOther;          // �����̶������������ֵ
    WORD    m_wMaxMPOther;          // �����̶������������ֵ
    WORD    m_wMaxSPOther;          // �����̶������������ֵ

    DWORD	m_dwCurHP;			    // ��ǰ������
    WORD	m_wCurMP;			    // ��ǰ����
    WORD	m_wCurSP;			    // ��ǰ����

    WORD	m_wEN;				    // ���� �����ʣ�����̫����
    WORD	m_wST;				    // ���� ��ǿ׳������̫����
    WORD	m_wIN;				    // ���� ���ǻۣ�����������
    WORD	m_wAG;				    // �� ������������������
    WORD	m_wLU;				    // ���� ��������

    BYTE    m_byENState;            // ��̫������״̬
    BYTE    m_bySTState;            // ��̫������״̬
    BYTE    m_byINState;            // ����������״̬
    BYTE    m_byAGState;            // ����������״̬

    BYTE	m_byPoint;			    // ��ҵ�ǰ��û�з������������
    BYTE	m_byAmuck;			    // ɱ��ֵ
    short	m_sXValue;		        // ����ֵ
    BYTE    m_byPKValue;            // pkֵ

    WORD	m_wCurRegionID;		    // ��ǰ���ڵ�ͼ��ID
    WORD	m_wSegX;			    // ��ǰ���ڵ�ͼ��X���꣨Tile��
    WORD	m_wSegY;			    // ...

    char	m_szName[11];		    // m_szName[10] == 0������
    char	m_szTongName[11];	    // ������ƣ�ͬʱΪ���ڲ��Ҹð��ɵĹؼ���
    char	m_szTitle[11];		    // ������Զ��ĳƺ�
    DWORD	m_dwSysTitle[2];	    // ϵͳ����ĳƺţ�64������

    SSkill	m_pSkills[30];		    // Ŀǰ��ѧϰ���书
    SSkill	m_pTroopsSkills[4];	    // Ŀǰ��ѧϰ�����ͼ�

    // �ķ���ص�
    STelergy    m_Telergy[TELERGY_NUM];    // ����ѧϰ64���ķ�
	SExtraTelergy m_ExtraTelergy;	            // ����װ���ķ�	
    BYTE    m_CurTelergy;           // ��ǰ���õ��ķ�
    BYTE    m_byTelergyStep;        // �ķ�������
    //BYTE    m_CurUseAmulet;         // ��ǰʹ�õĻ�����

    SEquipment	m_Equip[16];	        // ���Ŀǰ��װ���ĵ���

    WORD    m_wEquipCol[4];         // ����·�����ɫֵEEC_MAX
    BYTE    m_byEquipColCount[4];   // ��ǰ��ɫ����
    DWORD   m_dwStoreMoney;         // ��Ҳֿ���Ĵ�Ǯ

    // Ѩλ�������
    BYTE    m_byVenapointCount;     // Ѩλ����
    DWORD   m_dwVenapointState[9];  // Ѩλ״̬

    WORD	m_wAddHPSpeed;			    // �����������ٶȸ���ֵ
    WORD	m_wAddMPSpeed;			    // ���������ٶȸ���ֵ
    WORD	m_wAddSPSpeed;			    // ���������ٶȸ���ֵ

    BYTE    m_byNTime; 
    BYTE    m_byLeaveTime; 
    __int32 m_nStartTime[2];

	WORD    m_wScriptID;
	WORD    m_wScriptIcon;
	bool	m_bWarehouseLocked;

    char	m_szMateName[11];		    // ���ޣ�������
    union {
        __int64 m_qwMarryDate;          // ��������
        struct {
            __int32 m_dwMarryDate;      // ��������
            WORD factionId;             // ����ID
            WORD reserve;              
        };
    };
	DWORD	m_dwPowerVal;//ս����

	SItemBase addion_packages[4];	// ��ұ������ֿ⵱ǰװ������չ�������ݣ�
};

struct SFixPackage // ����
{
    SPackageItem    m_pGoods[MAX_ITEM_NUMBER];  // ��ҵĵ���
};

struct SFixStorage1 // �ֿ�
{
    SPackageItem    m_pStorageGoods[MAX_ITEM_NUMBER];  // �ֿ�ĵ���
};

struct SFixStorage2 // �ֿ�
{
    SPackageItem    m_pStorageGoods[MAX_ITEM_NUMBER];  // �ֿ�ĵ���
};

struct SFixStorage3 // �ֿ�
{
    SPackageItem    m_pStorageGoods[MAX_ITEM_NUMBER];  // �ֿ�ĵ���
};

struct SPlayerTask
{
    WORD PlayerTaskID;
    DWORD PlayerTaskStatus;
};

struct SPlayerTasks
{
    SPlayerTask PlayerTask[MAX_TASK_NUMBER];
};

struct SPlayerGM
{
    DWORD  m_wGMLevel;
};
// ����,������,
struct SPlayerRelates
{
    DWORD dwStaticID;
    WORD  wDearValue; // ���ܶ�
    char  szName[11];
    BYTE  byRelation; // 0 ����, 1 ������, 2 ��� ,3 ����
};

struct SPlayerRelation
{
    SPlayerRelates m_PlayerFriends[MAX_FRIEND_NUMBER];
    SPlayerRelates m_PlayerBlocks[MAX_BLOCK_NUMBER];
    SPlayerRelates m_PlayerMates[MAX_MATE_NUMBER];
};

// ������
struct SPlayerVenapointEx
{
    enum
    {
        VENAPOINTEX_SY = 1,
        VENAPOINTEX_SJ,
        VENAPOINTEX_ST,
        VENAPOINTEX_SS,
        VENAPOINTEX_ZY,
        VENAPOINTEX_ZT,
        VENAPOINTEX_ZS,
        VENAPOINTEX_ZJ,
    };
    // Ѩλ�������
    DWORD   m_dwVenapointStateEx[8];  // Ѩλ״̬
};


// ��ת�̽�Ʒ��Ҫ���棬��Ϊ�������˳���ʱ�򣬿���Ҫ���ָ�ʹ�ã�
struct SPlayerDialUnit
{
    union
    {
        struct SDialBase
        {
            BYTE type;
            BYTE data[64];
        }base;

        // ����
        struct SDialUnitItem
        {
            BYTE type;
            SRawItemBuffer itemBuffer;
        }unitItem;

        // ���������羭�飬��
        struct SDialUnitOther
        {
            BYTE type;
            int value;
        }unitOther;
    };
};


struct SFixProperty : // �̶���������ԣ��ᱣ������������
    public SFixBaseData,
    public SFixPackage,
    // public SPlayerComment,
    public SFixStorage1,
    public SFixStorage2,
    public SFixStorage3,
    public SPlayerTasks,
    public SPlayerGM,
    public SPlayerRelation,
    public SPlayerVenapointEx,
    public SPlayerDialUnit
{
    enum SAVESTATE //��ɫ�������ϵ�״̬
	{
	ST_LOGIN =1,//�ոյ���
	ST_LOGOUT,//�˳�
	ST_HANGUP //���߹һ�
	};
    DWORD   m_dwStaticID;                       // ��ҶԷ�������ȺΨһ��̬��ID�������ݿ���������ɡ�
    char    m_szUPassword[CONST_USERPASS];        // ��Ҷ�������
	BYTE    m_byStoreFlag; //��ҵĵ�ǰ״̬ �������ֱ���ʱ����Ϣ

    static const int VERSIONID = 1;
    static DWORD GetVersion() { return ( sizeof( SFixProperty ) << 16 | VERSIONID ); };
};

static const int QUEST_MONEYPOINT_TIME = 30;       // Ԫ������ʱ�������ʱ�䣨30�룩
struct SPlayerTempData // ��ҵ���ʱ���ݣ������棬����ʹ���ڿ糡��������ʱ������ݴ���
{
    DWORD m_dwTeamID;               // ��ӵ�����

    SRoleTask PlayerTask[100];      // ��ʱ�ű�����

    bool m_IsRegionChange;          // �ǳ�ʼ�����ǳ���ת��

    struct {
        DWORD GID;                  // pk����
        DWORD lasttime;             // ʣ�µ�pkʱ��
    } m_PkList[30];                 // PK�����б�

    DWORD m_dwTimeLeaving;          // ��ʱ���ݵĸ���
    DWORD m_dwOneHourLeaving;       // ��ʱ���ݵĸ��£�Сʱ��

    DWORD   m_dwExtraState;         // ��ҵ�һЩ����״̬����̯=0x01��û�ã������ƾ���=0x02�������ƾ���=0x04
	WORD	m_wMutateID;		    // �����ͼ�����
    WORD    m_wScapegoatID;         // �����ͼ����� =0 ������
    WORD    m_wGoatEffectID;        // �����Ч�����

    WORD    m_wMonsterCountOnGoat;  // ��װ����ɱ������ļ���
    WORD    m_wGoatMutateID;        // ��ǰ����������װ���(1-11)
    DWORD   m_TVOnGoatMutate_dwTime;// ��������ʱ��

    DWORD   m_dwSaveTime;           // ����ʱ�䣡

    SAutoUseItemSet m_stAutoUseItemSet[3];    // �Զ���ҩ״̬����

    WORD    m_wCheckID;              // ˢ���ñ�ţ�

    BYTE    m_bQuestFullData;        // �Ƿ���Ҫ��ͻ��˷��ͳ�ʼ������

	WORD	m_wTeamSkillID;			// ���μ�ID
	WORD	m_wTeamMemberCount;		// ��Ա����

    DWORD   checkPoints[4];         // 0-����[�����ں�����չ]�� 1-Ԫ���� 2-������ 3-ͨ��

	//__int64	m_qwMoneyPointTime;		// Ԫ������ʱ���־
};

///////////////////////////////////////////////////////////////////////////////////////////////
// �����������
struct SRegion
{
    char    szName[17];             // ����������
    WORD    ID;                     // �ó����ı��
    WORD    MapID;                  // �ó����ĵ�ͼ���
    WORD    NeedLevel;              // �ó�������ĵȼ�
};

static const int MAX_SCORE_NUMBER = 10;
static const int MAX_SCHOOL_NUMBER = 5;
// ���а��������
struct SScoreTable
{
    enum ScoreType {

        ST_ALEVEL,
        ST_AMONEY,
        ST_RFAME,
        ST_LFAME,
        ST_SLEVEL,
        ST_SXVALUE,
        ST_SMONEY
    };

    struct SScoreRecode
    {
        long dwValue;  // ��ǰֵ
        char Name[11];  // ��ɫ����
    };

    // ��������
    SScoreRecode Level[MAX_SCHOOL_NUMBER][MAX_SCORE_NUMBER];     // �ȼ����а�
    SScoreRecode XValue[MAX_SCHOOL_NUMBER][MAX_SCORE_NUMBER];    // �������а�
    SScoreRecode Money[MAX_SCHOOL_NUMBER][MAX_SCORE_NUMBER];     // ��Ǯ���а�

    // ��������
    SScoreRecode ALevel[MAX_SCORE_NUMBER];
    SScoreRecode AMoney[MAX_SCORE_NUMBER];
    SScoreRecode FAME[2][MAX_SCORE_NUMBER];      // ����������

	// ս��������
    SScoreRecode PowerValue[2][MAX_SCORE_NUMBER];
};

// ׷ɱ��
static const int MAX_KILLINFO = 10;
static const int KILLINFO_BASEMONEY = 10000;

struct SSimplePlayer
{
    // ��������
    DWORD   dwStaticID;         // ����������ݿ��е�Ψһ���
    DWORD   dwGlobalID;         // ����ұ��ε�¼��ȫ�ֹؼ���

    WORD    wServerID;          // ��������������ID
    unsigned __int64 dnidClient;// ����������ϸ���ҵ�DNID

    WORD    wCurRegionID;       // ���ڳ�����ID
    DWORD   dwGMLevel;          // GM�ȼ�

	BYTE	bOnlineState;		// 0:����  1:����  2:�һ� 
};

struct KILLINFO {
	KILLINFO() 
	{
		dwValue = 0;
		szName[0] = 0;

	}

	DWORD dwValue;
	char szName[11];
};



//--------------------------------------------------------------------
//������Ϣ�б�����ݽṹ Add By Lovelonely
//--------------------------------------------------------------------

const int MAXPUBLICINFO=128;
const int MAXPUBLICINFONUM=10;

struct SPublicInfo
{
	SPublicInfo(){nTimeAdd=0;}

	char	szSay[MAXPUBLICINFO];	//����
	long	lClr;					//��ɫ
	int		nTime;					//���ʱ��
	int     nTimeAdd;				//ʱ���ۼ�

	WORD	wType;

	enum
	{
		TP_CHANNEL,					//����Ƶ����ʾ
		TP_ROLL,					//������ʾ
	};
};

class CPublicInfoTab
{
public:

	CPublicInfoTab() {m_listInfo.clear();}

	std::list<SPublicInfo> m_listInfo;
	
	void	AddPInfo(SPublicInfo pInfo){if (m_listInfo.size() >= MAXPUBLICINFONUM)
										m_listInfo.pop_front();
										m_listInfo.push_back(pInfo);}

};

/*
//�̶���������ԣ��ᱣ������������
struct SFixPlayerDataBuf:
    public SFixBaseData,
    public SFixPackage,   
    public SPlayerTasks,
    public SPlayerGM,
    public SPlayerRelation,
    public SPlayerVenapointEx,
    public SPlayerDialUnit
{
    DWORD   m_dwStaticID;  // ��ҶԷ�������ȺΨһ��̬��ID�������ݿ���������ɡ�
};

//�ֿ�����
struct SWareHouses
{
	SFixStorage1 m_WareHouse[3];
	//bool m_blocked;//�ֿ��Ƿ�����
};
*/

//�̶���������ԣ��ᱣ������������
typedef SFixProperty SFixPlayerDataBuf;

//�°汾�����˺Ź�������
struct SWareHouses
{
	//SFixStorage1 m_WareHouse[3];
	//bool m_blocked;//�ֿ��Ƿ�����
};

//--------------------------------------------------------------------

//extern BOOL GenerateNewItem( SRawItemBuffer &itemBuffer, const GenItemParams &params, const LogInfo &log );
//extern BOOL GenerateNewItem( class CPlayer *player, const GenItemParams &params, const LogInfo &log );
