#pragma once
#include <d3dx9math.h>
#include "fightobject.h"
#include "CMonsterDefine.h"
#include "NetWorkModule/TeamMsgs.h"

// ���＼�������ļ�
#define FILENAME_MSKILL "Data\\MonSkillData.def"     

// �����IID
const __int32 IID_MONSTER = 0x112100a4;

// �����书����(����<255)
const int MAX_MSKILLCOUNT  = 20;      

// ������������书����
#define  MONSTERSKILLMAX 6

// �����������ֿ�BUFF������
#define  MONSTER_REMOVE_BUFF_NUM 8

/************************************************************************/
/*      ��������  װ����������������									*/
/************************************************************************/

//Ŀǰ��Ƶ����ͬʱ�������Ը���
#define Max_Att_Num			11	

//ÿ�����������ܳ��ֵ�ֵ����	
#define Max_Att_Value_Num	100		

//װ����Ϊ8�ף���������ÿ����������������Ե��������
#define Max_Att_Num_Rule	8

struct SEquipAttTypeOddsData
{
	char name[20];		//������
	WORD	OddsValue;	//����ֵ����
};
struct SEquipAttValueOddsData
{
	char     name[20];		//������
	DWORD	OddsValue[Max_Att_Value_Num];	//����ֵ����
};
struct SEquipAttNumRuleData
{
	WORD	levle;	//װ���ȼ�
	WORD	number; //��������
};

// ����ITEM�ṹ�����յ���ITEM�źͼ���
struct SItemOddsTabledata  
{
	SItemOddsTabledata()
	{
		itemId = 0;
		wOdds  = 0;
	}
	WORD    itemId;  //Item id
	WORD    wOdds;	 //Item ���伸��
};

//����ITEM ��һ�������ļ���
typedef std::vector< SItemOddsTabledata > ITEMDROPTABLE;	

//��ѯ��ṹ�������˵�����ļ���ID���Լ�����ITEM ��
struct SItemDropTableData
{
	ITEMDROPTABLE itemIdTable;	
	WORD	itemId;
};

typedef std::vector< SItemDropTableData > ITEMDROPIDTABLE;
extern ITEMDROPIDTABLE				g_vItemDropIdTable;		// ������Ʒid��,���е�������ݽṹ
extern ITEMDROPIDTABLE				g_vTaskDropIdTable;		// ���������ƷID��
extern WORD							*g_wAttNumOdds;			// װ��������Եĸ���������ʣ��� 10000��		
extern SEquipAttTypeOddsData		*g_sAttTypeOdds;		// װ�����ÿ�����Գ��ּ��ʣ�����ʣ��� 10000��
extern SEquipAttValueOddsData  		*g_sAttValueOdds;		// װ��ÿ������ȡÿ��ֵ�ļ���(����ʣ��� 10000)
extern SEquipAttNumRuleData			*g_sAttNumRule;			// װ������������������
// end--ab_yue

/*
���	
����	
ͼ����	
ͷ��ͼ����	
�ȼ�	
�Ƿ���BOSS	
����ֵ	
�⹦����	
�ڹ�����	
�������	
�ڷ�����	
ʹ���⹦��������
ʹ���ڹ���������
ʹ��ƽ��ֵ����
���⹥���������ӷ�����	
�ƿ��ְ��м���	
���������(���ӹ���)	
������ж�׼���	
�书����	
�书ʹ�ü���	
��	
�������[�����ٶ�]	
����ʱ��	
������Χ	
�����⹦ɫ�ּ���	
�����ڹ�ɫ�ּ���	
�������ɫ�ֱ���	
�����ڷ�ɫ�ּ���	
������Ǯɫ�֡�����	
Я������ֵ	
Я����Ǯ	
Я������ֵ	
����1	
����1����	
����2	
����2����	
����3	
����3����	
����4	
����4����	
����5	
����5����	
��BUFF1���	
��BUFF1����
��BUFF2���
��BUFF2����
��BUFF3���
��BUFF3����	
��BUFF4���
��BUFF4����
��BUFF5���
��BUFF5����	
��BUFF6���
��BUFF6����	
��BUFF7���	
��BUFF7����	
��BUFF8���
��BUFF8����
�ű����	
���������	
Я����Ʒ�б�1	
��Ʒ�������1	
Я����Ʒ�б�n	
��Ʒ�������n
*/
// struct SMonsterData
// {
//     // �µĹ����
//     // ��� ���� ͼ���� ͷ��ͼ���� �ȼ� �Ƿ���BOSS 
// 	char name[20];
//     int id, image, iHeadImage,level, boss;
//     // ����ֵ �⹦���� �ڹ����� ������� �ڷ����� 
//     int hp, outAtkBase, innerAtkBase, outDefBase, innerDefBase;
// 	// ʹ���⹦�������ʡ�ʹ���ڹ��������ʡ�ʹ��ƽ��ֵ����
// 	int outAtkChance, innerAtkChance, avgAtkChance;
//     // ���⹥���� �ƿ��ְ��м��� ��������� ������ж�׼��� �书���� �书ʹ�ü��� �� �������[�����ٶ�] ����ʱ�� ������Χ 
//     int specAtk, bWeaponOdds, specPro, bArmourOdds,skillScale, skillRate, agility, hitSpeed, hitTick, hitArea;
//     // ������
//     int ChangeRate;
//     // Я������ֵ Я����Ǯ Я������ֵ ���ܱ�ţ����ܼ��ʣ���BUFF��ţ���BUFF���� �������ű����
//     int exp, money, xydvalue,skillInfo[5][2],moveBuffInfo[8][2],scriptId;
// 	// ��������� ������Ʒ�����Я����Ʒ�б�n ��Ʒ�������n��
// 	int dropMaxNum, dropTaskTable,dropTable[30][2];
// 	int extraScriptId;
// };

struct SMonsterSkill
{
	WORD type;			// ��������1.��Ѩ(ʱ��/2/20ÿ��) 2.��ѣ(ʱ��/10/20ÿ��) 3.�ж�(ʱ��/1/20ÿ��) 4.����(ʱ��/100/20ÿ��) 5.�����������ܳ���ҩ(ʱ��/1/20ÿ��) 6.���������ܳ���ҩ(ʱ��/1/20ÿ��) 7.����
                        //         8.�Ƽ�(ʱ��/10/20ÿ��) 9.����(ʱ��/10/20ÿ��) ...(δ����)10.��Ѫ 11.�������  12.���˽��� 13.Ⱥ���˺�ƽ��
    WORD  rating;		// ��1/10000ΪС����λ,������ָ���
    WORD  vitalsodds;	// �����������ֵ�ᱻ����ķ��ֿ�
    WORD  effectTime;	// ����ʱ��(���ֵҲ�������й�)
    DWORD effectId;		// �ͻ��˲��ŵ�����Ч��ͼ�����
    DWORD waiting;		// �ӷ��������еĵȴ�ʱ��

    // ��չ    
    char  szName[16];         // ����[δ��]
    int   dwSkillTimes;       // ʹ�õĴ���
    WORD  wRedDefRate;        // ���ͷ���������[�ű���ѡ]                      
    WORD  wRedAgiRate;        // ���������ȱ���[�ű���ѡ]                           
    WORD  wToxicRedHPRate;    // �ж�����������[�ű���ѡ]              
    WORD  wDemToHPRate;       // �����˺�ת��������[�ű���ѡ]
    DWORD dwDamageArea;       // Ⱥ�˵Ĺ�����Χ
    DWORD dwDamageTotal;      // ���˺�
};

class CMonsterScriptEvent
{
public:
	struct MEventBase
	{
		MEventBase(const string& f): func(f) {}
		string	func;
	};

	//Ѫ���仯�¼���
	struct HPEvent:public MEventBase
	{
		float	hpRate;

		HPEvent(float rate, const string& func) : hpRate(rate), MEventBase(func) {}
		bool operator<(const HPEvent& e) { return hpRate<e.hpRate; }
	};
	typedef std::vector<HPEvent> HPEventTable;

    //��������ʱ���¼���
	struct BeAttackEvent:public MEventBase
	{
		DWORD	dwAttackTime;

		BeAttackEvent(DWORD time, const string& f): dwAttackTime(time),MEventBase(f){}
		bool operator<(const BeAttackEvent& e) { return dwAttackTime<e.dwAttackTime; }
	};	
	typedef std::vector<BeAttackEvent> BeAttackTable;


	//����Buff�¼���
	struct BuffEvent:public MEventBase
	{
		DWORD	dwBuffId;

		BuffEvent(DWORD id, const string& f) : dwBuffId(id),MEventBase(f){}
	};
	typedef std::vector<BuffEvent> BuffTable;
    
	void Sort()
	{
		std::sort( hpTable.begin(), hpTable.end() );
		std::sort( baTable.begin(), baTable.end() );
	}

	HPEventTable	hpTable;
	BeAttackTable	baTable;
	BuffTable		buffTable;
};

class CMonster : public CFightObject
{
public:
	typedef CFightObject Parent;

	static int LoadMonsterData();                           // ������������
	static void ClearMonsterData();                         // ������������
//    static SMonsterData *GetMonsterData(int index);         // ������������

    // ����
    static void LoadMonsterSkillData();                             // �������ļ�������
    static SMonsterSkill *GetMonsterSkillData(int SkillType);      // ��ȡ����ļ������� SkillType [1, MAX_MSKILLCOUNT]

	//��������ȫ�ֵ��ߡ�װ�����Ա�
	static int LoadEquipDropAttributeTable();

	////����������Ը������ʱ�
	static int LoadEquipDropAttNum();	

	//��������������ͼ��ʱ�
	static int LoadEquipDropAttType();	

	//����װ��ÿ������ȡÿ��ֵ�ļ���
	static int LoadEquipDropAttVale();	

	//����װ������������������
	static int LoadEquipDropAttRule();	

	//��������������ߵ����
	static int LoadItemDropTable();				

	//������������������������Ʒ��
	static int LoadTaskDropTable();
	//end

    // -----------------------------------
    // �̳��麯�� 
public:
    IObject *VDC(const __int32 IID) { if ((IID == IID_ACTIVEOBJECT) || (IID == IID_FIGHTOBJECT) ) return this; return NULL; }

public:
	void OnClose();
	void OnDisppear();
protected:
	int OnCreate(_W64 long pParameter);

public:
	struct SASynMonsterMsg *GetStateMsg();

  //ɨ��һ����Χ�ڵĵ�ͼ��ȡ�ù���Ŀ����б�
    int Attack();
    int WatchArea();

    BOOL CheckAction(int eCA);        // ��⶯��ִ�е�����

    void SetRenascence(DWORD dwLast, DWORD dwCur, DWORD dwHP, WORD wTime); 
    // ����������ش���
    BOOL SendDropItemOnDead( BOOL bDoubleDrop, class CPlayer *pActPlayer );

	//��������
	BOOL SendDropItemOnDeadXYSJ( BOOL bDoubleDrop, class CPlayer *pActPlayer );
	//end

    bool IsBoss(void);              // �ж��Ƿ���Boss

	// �������������
    void OnDead( CFightObject *PKiller );
    void _OnDead( CFightObject *PKiller );

    BOOL AttackTarget(void);
    BOOL CheckTarget(void);

	void ChangeFightSpeed(bool inFight);

	//BUFF���ʱ�����߼�����
	virtual bool CheckAddBuff(const SBuffBaseData *pBuff);

	//���¹��������
	virtual void UpdateAllProperties();

	bool StopTracing();
	bool SetStopTracing(EActionState action=EA_RUN);///ǿ��ֹͣ׷��Ŀ��
	bool MoveToBirth();///�ڷ��س����㱻�赲�������ǿ��˲��
public:

	int		m_ActionFrame;		// ������ʱ�䣬���ڹ����AI
	DWORD	m_BaseSpeed;		// ����Ļ����ٶȣ���ս��/��ս���²�ͬ��
	void Thinking();				// ˼��������ѡ����һ�����ж�

	struct SParameter
	{
		WORD	wListID;			// ����ID
		DWORD	dwRefreshTime;		// ˢ�¼��
		WORD	wX;					// ���꣨�������꣩
		WORD	wY;					
		WORD	wAIID;				// ����AI��ID, =0��ʾ��ʹ��AI
		WORD	wGroupID;			// ��ӪID
		WORD	reginID;			//��ͼID
		WORD	PartolRadius;
		SPath	m_Path;				//Ϊ�����趨��·���㣬=0��ʾû������
		DWORD wCreatePlayerID;	//������������GID����һ���У�ֻ�е�����ű��в���
		DWORD dRankIndex;//�������������︴�Ƶ����sid
		SParameter()
		{
			wListID  = 0;			// ����ID
			dwRefreshTime = 0;		// ˢ�¼��
			wX = 0;					// ���꣨�������꣩
			wY = 0;					
			wAIID = 0;				// ����AI��ID, =0��ʾ��ʹ��AI
			wGroupID = 0;			// ��ӪID
			reginID = 0;
			wCreatePlayerID = 0;
			PartolRadius = 0;
			dRankIndex = 0;
		}
	};

	/*
	struct SParameter
	{
		WORD	wRefreshStyle;		// ˢ�·�ʽ ��չ 1��ʾ������ 2��ʾ����
		WORD	wListID;			// �ù���ı�ţ������б��еı�ţ�
		DWORD	dwRefreshTime;		// ˢ��ʱ��
		WORD	wRefreshScriptID;	// �����ű���ţ����ù��ﱻˢ�£�����֮��ʱ������

		WORD	wX;					// ��ǰ������
		WORD	wY;					// ������
		WORD	wMoveArea;			// ����������ߵĻ����������Ƶķ�Χ
		WORD	wAIType;			// �����AI����
		WORD	wAIScriptID;		// AI�ű���ţ�����ű��Ĵ���Ҫ��AI���������趨���ˣ�
		WORD	wDeadScriptID;		// �����ű���ţ��������ɱ����ʱ�򴥷���
		WORD	wExtraScriptId;		// ��չ�ű�ID
        SMonsterData *extraData;

		char	szName[CONST_USERNAME];

        DWORD   dwBRTeamNumber;      // ����ˢ������
        WORD	wCenterX;			 // ���ĵ�����(���������wX��Ҫ������m_wCenterX��Ӧ��������ˢ��ʱ��Ҫ�õ�)
        WORD	wCenterY;			 // ������
        DWORD   dwBRNumber;          // ����ˢ������
        WORD    wBRArea;             // ����ˢ�·�Χ 
	}; //*/

public:
	/*
	struct SRefreshData				// ˢ����ص�����
	{
		WORD m_wRefreshStyle;		// ���ַ�ʽ 
		WORD m_wListID;				// ������
		DWORD m_dwRefreshTime;		// ����ʱ�� 
		WORD m_wSrcX;				// �������� 
		WORD m_wSrcY;				//	...
		WORD m_wRefreshScriptID;	// �����ű���ţ����ù��ﱻˢ�£�����֮��ʱ������

        DWORD m_dwBRTeamNumber;     // ����ˢ������
		float m_BirthPosX;			// ����������
		float m_BirthPosY;			

        WORD  m_wCenterX;			// ���ĵ�����
        WORD  m_wCenterY;			// 
        DWORD m_dwBRNumber;         // ����ˢ������
        WORD  m_wBRArea;            // ����ˢ�·�Χ  
	}; //*/

	struct SRefreshData
	{
		WORD m_BirthPosX;
		WORD m_BirthPosY;
	};

	struct SMonsterProperty : public SRefreshData, public SMonsterBaseData
	{
		DWORD controlId;        // ���Ʊ��,���ڽű�����
	};

	struct SMonsterAIProperty
	{
		DWORD   startUpdateTime;	// ��һ�θ���ʱ�䣨�������ɵ�ʱ�䣩
		DWORD	preUpdateTime;		// ��һ�θ���ʱ��
	};

	/*
	struct SMonsterProperty : public SRefreshData, public SMonsterBaseData
	{
		WORD m_wMoveArea;		// �ƶ���Χ
		WORD m_wAIType;			// AI����
		WORD m_wAIScriptID;		// AI�ű����
		WORD m_wDeadScriptID;	// �����ű����

		DWORD m_dwCurHP;		// ��ǰ��HP
		DWORD m_dwExtraColor;	// ������ɫ
		
		//BYTE m_byGoodState;		// ��ǰ����״̬
		//BYTE m_byBadState;		// ��ǰ����״̬
        //WORD m_wDamageType;       // �˺�����(����ָ�����Ͷ�ָ��Ŀ����ɶ����˺�,��:�����ֶ����˺� )
        struct SDamageType
        {
            BYTE sex: 2;            // �Ա�1Ů2��
            BYTE school:4;          // ����
            BYTE reserve : 2;
        };

        SDamageType mageType;

        WORD m_wDamageValue;       // (1-65535%)�ٷֱ�

        char szMaskName[22];    // �޶���
        DWORD sideId;           // �޶���ӪID��Ŀǰ�ƻ�ͨ��szMaskName��ֱ���ҵ���Ӫ��ţ�

        static const IS_NAME_LIMITED    = 0x00000008;       // �����޶�������[�ܵ�����ʱ]
        static const IS_TONG_VALIDATE   = 0x00000004;       // �����޶�/��ɫ�޶�
        static const IS_MASK_ATTACK     = 0x00000002;       // ���ܹ���/ֻ�ܹ���
        static const IS_ACT_MASK_ATTACK = 0x00000010;       // ���ܹ���/ֻ�ܹ���
        static const IS_TEAM_VALIDATE   = 0x00000001;       // ��Ӳ��ܹ���
        static const IS_SCHOOL_VALIDATE = 0x00002000;       // ָ�����ɲ��ܹ���

        static const IS_RE_LIMIT_SKILL  = 0x00000020;       // �ֿ�����
        static const IS_LOS_ATTACK      = 0x00000040;       // ����N�����¹���
        static const IS_RE_DAM_DAM      = 0x00000080;       // �ֿ�X%�⹦�˺�
        static const IS_RE_POW_DAM      = 0x00000100;       // �ֿ�X%�ڹ��˺�
        static const IS_EXC_DAM_DAM     = 0x00000200;       // �ܵ��⹦�˺���X%�ӳ�       
        static const IS_EXC_POW_DAM     = 0x00000400;       // �ܵ��ڹ��˺���X%�ӳ�
        static const IS_TOXIC_VALIDATE  = 0x00000800;       // �嶾�澭�ܹ���/���ܹ���
        static const IS_RE_DIZZY        = 0x00001000;       // �ֿ���������

        DWORD controlId;        // ���Ʊ��,���ڽű�����

        // ��Ŀ�깥���趨
        struct {
            DWORD rating;   // ��1/10000ΪС����λ,������ָ���
            DWORD damage;   // ��1/10000ΪС����λ,���㸡������(��ԭ�����˺��Ͻ��и���)
            DWORD effect;   // �ͻ��˲��ŵ�����Ч��ͼ�����
            DWORD area;     // Ⱥ�˵Ĺ�����Χ
            DWORD hit;      // Ⱥ�˵����и���
            DWORD waiting;  // �ӷ��������еĵȴ�ʱ��
        } multiAttack;

        // �������ӹ���AI�����ԵĽű���������
        struct {
			std::string stateChanged;
            std::string hpLimited;
        } scriptAi;

        struct {
            int dodge;     // ����ľ��Զ����
            int hit;       // ����ľ���������
            int damage;    // ����ľ����˺��� ( ��ֵ�Ƚ�����,Ϊ����ʱΪ�̶��˺�ֵ,Ϊ����ʱΪ�̶��˺��׷ֱ� )
            int defence;   // ����ľ��Է�����
        } absoluteSetting;

        DWORD curSegment;
    }; //*/

    // ������� 
	std::list<SMonsterSkill> skillList;			// �������ʹ�õļ���
	SMonsterSkill skill;						// ����׼��ʹ�õļ���

	SMonsterProperty	m_Property;		
	
	SParameter			m_AIParameter;
	SMonsterAIProperty	m_AIProperty;

	DWORD	m_nlastdropTime;		// ��һ�ε���ʱ��
	// �����ƶ�������
	struct		SWayTrack { BYTE dir:3; BYTE len:5; };

	BYTE		m_bySpeedFrame;					// ����ҵ��ƶ��ٶȣ�һ��������ѭ��FrameΪ��λ	
    BYTE        m_byMutateState;                // ��ǰ�Ƿ������ͻ�䣨 0��ʾδͻ�䣬1~6��ʾͻ������ͣ�

    DWORD m_dwEndAttackTime; // û���ܵ�����һ��ʱ���ֹͣ׷��

    DWORD m_dwTelesportTime;                    // ����˲��ʱ��
    static const int TELESPORT_TICK = 2000;         // 2��

public:
    // �������
    struct SHitData
    {
        SHitData();

        DWORD dwGID;            // �ö����ID
        DWORD dwTeamID;         // �ö����TeamID
        DWORD dwHitTimes;       // �ö���Ĺ�������
        DWORD dwDamage;         // �ö������ɵ��˺�
        DWORD dwTimeMargin;     // �ö��󹥻������е����ʱ����
        DWORD dwLastHitTime;    // �ö������һ�ι�����ʱ�䣨��������������㣬�����ж��������Ƿ�����˹������
        DWORD dwFristHitTime;   // ��һ�ι���ʱ��

        class CPlayer *pPlayer;       // ������ʱ��
    };

    std::map<DWORD, SHitData> m_HitMap;

    void SetHit(DWORD GID, DWORD TeamID, DWORD Damage);
    void UpdateHitMap();
	void KillMonsterTask(CPlayer *pKiller);
	void KillMonsterTaskWithSingle(CPlayer *pCurrPlayer);
	void HandleExpWithTeam(CPlayer *pKiller);
	void HandleGetSP(CPlayer *pKiller);
	void HandleExpWithObj(CFightObject *pKiller);

	// ����������ʱ�򱻵���
	void OnDamage(INT32 *nDamage, CFightObject *pFighter);
	void OnRebound(int rebound, CFightObject* pFighter);
	void OnRun();

	BOOL EndPrevAction();   // ��һ����������ʱ�Ļص�
	BOOL DoCurAction();

	bool ProcessDeadWaitTime();
	bool EndPrevActionByStandState();
	bool EndPrevActionByDeadState();
	bool EndPrevActionByWoundState();
	bool EndPrevActionByStandState_DoAIByID3Or4();
	bool EndPrevActionByStandState_DoAIByID2();
	bool EndPrevActionByStandState_DoAIByID1();
	bool DoRenascenceScript();
	bool DeDeadEvent();
	bool ProcessEndAttack();

	// ���跢�͸��º������
	virtual void SendPropertiesUpdate();

	virtual void SwitchFightState(bool IsIn);	// ����/�뿪ս��״̬�ص�

	// ���������Ʒ����
	BOOL GetDropItem(CPlayer *Killer);
	// ���������Ʒ����
	BOOL GetDropItem(DWORD itemID,INT32 itemNum=1);
	/************************************************************************/
	/* �µļ��ܹ�������--yuntao.liu											*/
	/************************************************************************/

	// ������ʹ�ü��ܹ�����ʱ�򣬾����Զ����ô�����ƶ���Ϣ
	virtual bool SendMove2TargetForAttackMsg(INT32 skillDistance,INT32 skillIndex,CFightObject* pLife);

	// ����һ��·��
	bool CreatePath(SQSynPathMsg& WayMsg, CFightObject *pLife);

	// ����һ��·��
	int CreatePath(SQSynWayTrackMsg& WayMsg,CFightObject* pLife);

	// �����漴��׷��Ŀ�꣬��Ŀ����Χ����
	int CreateRadomPoint(CFightObject* pLife, D3DXVECTOR2 &vtargetPos, WORD mindistance, WORD maxdistance);

	virtual INT32 GetCurrentSkillLevel(DWORD dwSkillIndex);

	// ��������ļ��������õ�����ID
	virtual INT32 GetSkillIDBySkillIndex(INT32 index);

	// ״̬�л���ʱ��Ļص�����
	virtual void OnChangeState(EActionState newActionID);

	// ���ƶ���ɺ�ص�����
	virtual void OnRunEnd();

	// Ŀǰ�������ѧϰ5�����ܣ���һ����ͨ���ܣ��ܹ�6������
	SSkill m_pSkills[ MONSTERSKILLMAX ];   //modify by ab_yue
	// end--yuntao.liu

	int OnCreateEnd();//������ɣ��ڰѹ�����볡�����һ�λص�

	void OnRecvPath(struct SQSynPathMonsterMsg *pMsg);
	float GetWalkSpeed();
private :
    BOOL  m_bStopToStand;
    WORD  m_wIdleTime;
	WORD m_wCuruseskill;//��ǰʹ�õļ���
	//------------------------------------------------
	// �������
private:
	DWORD m_CopySceneGID;

public:
	void SetCopySceneGID(DWORD dwGID) { m_CopySceneGID = dwGID; }

public:
    // 4 monster dead event
    BOOL m_bDeadEvent;

    DWORD m_dwDeadDelayScript;
    DWORD m_dwKillerPlayerGID;

    WORD  m_wDeadTimes;

    // ����ai��أ��Ƿ���Ҫ����ѡ��һ������Ŀ�ꣿ
    DWORD targetSegTime;
    DWORD targetDamage;

    // ͳ������
    static DWORD monsterDeadMoney;

    BYTE  bySelectType;				// ��ʾ���Ȼ����ų���1.���� 2.�ų�  
    BYTE  byPriorityExceptType;		// ���Ȼ��ų�ѡ��Ŀ�����ͣ� 1,��ɫ  2,����  3,�Ա�  4,����  5,����
    DWORD dwPriorityTarget;			// ����ѡ��Ŀ��

	DWORD m_SearchTargetTime;		// ����Ŀ��ʱ��
	bool  m_BackProtection;			// ���ڷ��ر���
	///���ڹ���AIʱ�����
	DWORD m_nAttackTime;			///��������ʼʱ���¼
	DWORD GetRandomEnemyID();
	DWORD	m_nRandomSkillIndex;//�漴����ID
	ULONGLONG	m_nBirthTime;
	DWORD  m_nCollisionTime;
	ULONGLONG m_nMoveStartTime;//��ʼ�ƶ�ʱ��
	DWORD  m_dCreatePlayerID;	//����Ĵ�����
	DWORD  m_dRankIndex;	//���������︴���������
	DWORD  m_ShowState;  //��Ӧ���m_ShowState
	char   m_Name[CONST_USERNAME];//����
	WORD m_dModID;//ģ��ID
	D3DXVECTOR2 *m_vdesPos;
	
	long   m_StaticpreUpdateTime;//��̬����Ԥǰʱ��
	BYTE m_gmakeparm1;
	BYTE m_gmakeparm2;
	BYTE m_gmakeparm3;
	BYTE m_gmakeparm4;
public:
	CMonsterScriptEvent scriptEvent;
	//���¹���״̬
	void UpdateMyData(WORD reginID,WORD wHour,WORD wMinute,char *killmeName,WORD mystate);
	//void ChangeProperty();
private:
	void InitScriptEvent();
	void TriggerHpScriptEvent(int nDamage, CFightObject *pFighter);

private:
	CPlayer*	m_PlayerInDis[MAX_TEAM_MEMBER];	
	DWORD		m_dwPlayerInDisCount;			//��Ч��Χ����ҵ�����
	void HandlePlayerInDis(CPlayer *pKiller);   //������������ң���Щ�ھ������/���������Ч��Χ��

	void SetNineWordsCondition(CPlayer* player);

	BossDeadData m_BossDeadData;
///////////////////////////////////////////////////////////////////////////////////////////////////
//AI�������
///////////////////////////////////////////////////////////////////////////////////////////////////
public:
	void MakeSynPathMsg(SQSynPathMsg &msg);

	void AIThinking();
	void ProcessScript();
	void SetMonsterPath(CMonster::SParameter param);

///////////////////////////////////////////////////////////////////////////////////////////////////

};