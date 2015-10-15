#pragma once

static const int ITEMNAME_MAXLEN		= 21;					// ����������ַ���
static const int MAX_REGION_NUMBER		= 300;					// ��󳡾�����
//20150427 wk  �Ż��������
//static const int CONST_BUYBACK_COUNT	= 40;					// ���16���ع���Ʒ
static const int CONST_BUYBACK_COUNT = 1;					// ���16���ع���Ʒ

// �����3���������ľ���
//wk 20150205 �Ż���Ҫ������
//const DWORD PackageWidth = 6;								// һ��������Ԫ�Ŀ��
//const DWORD PackageHight = 4;								// һ��������Ԫ�ĸ߶�
//const DWORD PackageType = 4;								// ������������Ŀ��Ŀǰ�л�������չ1����չ2������4�֣�
//const DWORD g_PackageMaxItem = 10;							// ��������������

const DWORD PackageWidth	= 1;								// һ��������Ԫ�Ŀ��
const DWORD PackageHight	= 1;								// һ��������Ԫ�ĸ߶�
const DWORD PackageType		= 1;								// ������������Ŀ��Ŀǰ�л�������չ1����չ2������4�֣�
const DWORD PackageAllHight	= PackageHight * PackageType;		// �������ܸ߶�
const DWORD PackageAllCells = PackageAllHight * PackageWidth;	// ���豳����������
// �����3������������
const DWORD g_PackageMaxItem	= 1;							// ��������������
const DWORD g_PackageMoneyIndex = g_PackageMaxItem;				// �����л��ҵ�����

// ���/����װ�����Ͷ���
enum EQUIP_TYPE
{
	EQUIP_T_WEAPON	= 101,		// ����1
	EQUIP_T_HAT,				//  ͷ��װ��
	EQUIP_T_WRIST,				// ����
	EQUIP_T_PAT,				// ����
	EQUIP_T_WAIST,				// ���
	EQUIP_T_SHOSE,				// Ь��
	EQUIP_T_CLOTH,				// �·�
	EQUIP_T_MANTLE,				//  ����1 ����
	EQUIP_T_PENDANTS,			// ����2 ����
	EQUIP_T_PROTECTION,			// ����
	EQUIP_T_RING,				// ��ָ
	EQUIP_T_PRECIOUS,			// ����2
	EQUIP_T_SHIZHUANG_HAT,		//  ����3
	EQUIP_T_SHIZHUANG_CLOTH,	// ʱװ���·��� t����
};

// װ��λ�ö���
enum EQUIP_POSITION
{
	EQUIP_P_WEAPON = 0,			// ����
	EQUIP_P_SHOSE,				// Ь��
	EQUIP_P_PAT,				// ����
	EQUIP_P_WAIST,				// ���
	EQUIP_P_WRIST,				// ����
	EQUIP_P_CLOTH,				// �·�
	EQUIP_P_HAT,				//  ͷ��װ��
	EQUIP_P_MANTLE,				// ����1
	EQUIP_P_PENDANTS1,			// ����1
	EQUIP_P_PENDANTS2,			//  ����2
	EQUIP_P_PROTECTION,			// ����
	EQUIP_P_RING1,				// ��ָ
	EQUIP_P_RING2,				// ��ָ2
	EQUIP_P_PRECIOUS,			// ����2
	EQUIP_P_SHIZHUANG_HAT1,		// ����3
	EQUIP_P_SHIZHUANG_CLOTH1,	// ʱװ�·�

	EQUIP_P_MAX,				// Max
}; 

static int ShowEquipPos[] = 
{
	EQUIP_P_WEAPON,		
	EQUIP_P_WRIST,	
	EQUIP_P_SHOSE,	
	EQUIP_P_PAT,				// ����
	EQUIP_P_CLOTH,	
	EQUIP_P_HAT,
	EQUIP_P_MANTLE,	
	EQUIP_P_PENDANTS1,
	EQUIP_P_PENDANTS2,
	EQUIP_P_PROTECTION,			// ����
	EQUIP_P_PRECIOUS,
	EQUIP_P_SHIZHUANG_HAT1,
	EQUIP_P_SHIZHUANG_CLOTH1
};

// װ�����Զ���
struct SEquipDataEx
{
	//����ҹ����װ�������¶���
	enum EQUIP_EXT_ATTRI
	{
		//�Ѿ�����˳��Ϊ��ҹ

		EEA_MAXHP,					// ��������
		EEA_MAXMP,					// ��������
		EEA_GONGJI,					// ����,�˺�
		EEA_FANGYU,					// ����
		EEA_HIT,							//����
		EEA_SHANBI,					// ����
		EEA_BAOJI,					// ����
		
		//����ҹ���� ����
		
		EEA_UNCRIT, //����
		EEA_WRECK,//�ƻ�
		EEA_UNWRECK,	//����
		EEA_PUNCTURE,	//����
		EEA_UNPUNCTURE,	//����
		EEA_ICEATK,	//����
		EEA_ICEDEF,	//����
		EEA_FIREATK,	//����
		EEA_FIREDEF,		//��
		EEA_DARKATK,		//����
		EEA_DARKDEF,		//����
		EEA_HOLYATK,		//��ʥ��
		EEA_HOLYDEF,		//��ʥ��
		EEA_HPPERCENT,	//Ѫ���ٷֱȼӳ�
		EEA_MPPERCENT,	//�����ٷֱȼӳ�
		EEA_ATKPERCENT,	//�˺��ٷֱȼӳ�
		EEA_DEFPERCENT,	//�����ٷֱȼӳ�
		EEA_CRITPERCENT,	//�����ٷֱȼӳ�
		EEA_WEAKPERCENT,		//�ƻ��ٷֱȼӳ�
		EEA_HITPERCENT,		//���аٷֱȼӳ�

		//����


		
		EEA_MAXTP,					// ��������
		EEA_SPEED,					// �ƶ��ٶ�
		EEA_ATKSPEED,				// �����ٶ�

		// ����ӵ�����
		EEA_REDUCEDAMAGE,			// �˺�����
		EEA_ABSDAMAGE,				// �����˺�
		EEA_NONFANGYU,				// ���ӷ���
		EEA_MULTIBAOJI,				// ��������

		EEA_ICE_DAMAGE,				// ���˺�
		EEA_ICE_DEFENCE,			// ������
		EEA_FIRE_DAMAGE,			// ���˺�
		EEA_FIRE_DEFENCE,			// ����
		EEA_XUAN_DAMAGE,			// ���˺�
		EEA_XUAN_DEFENCE,			// ������
		EEA_POISON_DAMAGE,			// ���˺�
		EEA_POISON_DEFENCE,			// ������

		EEA_MAX,
	};
};

// �������Ͷ���
enum EItemType
{
	ITEM_T_MEDICINE_HP		= 7,	// ����ҩ
	ITEM_T_MEDICINE_MP		= 8,	// ����ҩ
	ITEM_T_MEDICINE_TP		= 9,	// ����ҩ
	ITEM_T_NORMAL			= 10,	// ��ͨ����
	ITEM_T_JEWEL			= 11,	// ��ʯ����
	ITEM_T_CHANGE			= 12,	// �ɱ����
	ITEM_T_METRAIL			= 13,	// ���ϵ���
	ITEM_T_TOOL				= 14,	// ���ߵ���
	ITEM_T_BOOK				= 15,	// �鼮����
	ITEM_T_TASK				= 16,	// �������
	ITEM_T_RECYCLE			= 17,	// ��ʱ���յ���
	ITEM_T_FIGHT_PET		= 18,	// ���͵���
	ITEM_T_PRESENT          = 19,   // �������
	ITEM_T_SUIT_SCROLL		= 20,	// ��װ����
	ITEM_T_SKILLCHIP		= 21,   //���＼������Ƭ
	ITEM_T_PETSKILLBOOK		= 22,	//���＼����
	ITEM_T_EQUIPMENT_BEGIN	= 101,	// װ������ߣ���EQUIP_TYPE��Ӧ��
	ITEM_T_EQUIPMENT_END	= 114,
};

inline bool ITEM_IS_NORMAL(int type)	{ return (type == ITEM_T_NORMAL);	}
inline bool ITEM_IS_JEWELEX(int type)	{ return (type == ITEM_T_JEWEL);	}
inline bool ITEM_IS_CHANGE(int type)	{ return (type == ITEM_T_CHANGE);	}
inline bool ITEM_IS_METRAIL(int type)	{ return (type == ITEM_T_METRAIL);	}
inline bool ITEM_IS_TOOL(int type)		{ return (type == ITEM_T_TOOL);		}
inline bool ITEM_IS_BOOK(int type)		{ return (type == ITEM_T_BOOK);		}
inline bool ITEM_IS_TASK(int type)		{ return (type == ITEM_T_TASK);		}
inline bool ITEM_IS_RECYCLE(int type)	{ return (type == ITEM_T_RECYCLE);	}
inline bool ITEM_IS_FIGHTPET(int type)  { return (type == ITEM_T_FIGHT_PET);}
inline bool ITEM_IS_SUITSCROLL(int type){ return (type == ITEM_T_SUIT_SCROLL); }
inline bool ITEM_IS_SKILLCHIP(int type){ return (type == ITEM_T_SKILLCHIP); }
inline bool ITEM_IS_PETSKILLBOOK(int type){ return (type == ITEM_T_PETSKILLBOOK); }
inline bool ITEM_IS_EQUIPMENT(int type) { return (type >= ITEM_T_EQUIPMENT_BEGIN) && (type <= ITEM_T_EQUIPMENT_END); }

// �ж�װ��λ���Ƿ�Ϊ��Ҫ��3Dģ�͵�λ��
inline bool IsPosNeedToSwitchMesh(BYTE byPos)
{
	if (
		byPos == EQUIP_P_WEAPON		||
		byPos == EQUIP_P_WRIST		||
		byPos == EQUIP_P_SHOSE		||
		byPos == EQUIP_P_CLOTH		||
		byPos == EQUIP_P_MANTLE		||
		byPos == EQUIP_P_PENDANTS1	||
		byPos == EQUIP_P_PENDANTS2	||
		byPos == EQUIP_P_PRECIOUS	||
		byPos == EQUIP_P_HAT	||
		byPos == EQUIP_P_SHIZHUANG_HAT1 ||
		byPos == EQUIP_P_SHIZHUANG_CLOTH1
		)
		return true;
	return false;
}

// �����ƶ�����
enum ItemMoveDirection
{
	IMD_INPACKAGE,		// �ڱ������ƶ�
	IMD_PACK2EQUIP,		// ������װ����
	IMD_EQUIP2PACK,		// װ����������
	IMD_FP_PACK2EQUIP,	// ����������װ����
	IMD_FP_EQUIP2PACK,	// ����װ����������
	IMD_MAX,			// Max
};

// ��������
enum ItemPackageModel
{
	IPM_ITEM,			// ����
	IPM_BUDAI,			// Ǯ������
	IPM_TANMUBOX,		// ̴ľ��
	IPM_YINKOUBOX,		// ������
	IPM_GOLDENBOX,		// �ƽ���
};

// �����е��ߵ�ʰȡ���
enum PackageItemPickType
{
	PIPT_FREE,			// ��������ʰȡ
	PIPT_TAKECHANCE,	// ����ҡ�Ž׶�
	PIPT_LEADER,		// ���ڶӳ�����
};

#pragma pack (push)
#pragma pack (1)

// ������ģ������ã��������ݽṹ
struct SItemBase
{
	enum
	{
		F_NONE			= 0,						// �ޱ��
		F_SYSBINDED		= 0x1,						// �ѱ�ϵͳ��
		F_PLAYERBINDED	= 0x2,						// �Ѿ���Ұ�
    };

	static const int YEAR_SEGMENT     = 2009;		// ʱ�������ʼʱ��Ϊ2009��
	static const int MAX_ACCUMULATOR  = 0x3fffffff;	// 30�����ۼ�ֵ�����ֵΪ0x3fffffff == 1073741823

	inline QWORD uniqueId() const { return uniID; }
	inline void uniqueId(QWORD v) { uniID = v;	  }
	
	// �жϵ�ǰ��Ʒ�Ƿ��ڰ�״̬��
	inline bool IsBindState()
	{	
		return (flags & SItemBase::F_PLAYERBINDED)!=0 || (flags & SItemBase::F_SYSBINDED)!=0;		
	}

	union
	{
		struct
        {
            QWORD   wIndex				: 32;	// ����Ʒ�б��еı�ţ�ͨ�������Բ�����й��ڸõ��ߵ�����
            QWORD   flags				: 8;	// �����ߵĵ������Ա�־
			QWORD	overlap				: 16;	// ��ǰ��������
			QWORD	usetimes			: 8;	// ����ʹ�õĴ����������һ���Եģ���Ϊ1��ʹ�����ɾ����
            QWORD   uniqueTimestamp		: 32;	// [ʱ���      ] 32bit����30��Ϊ��λ���м�¼
            QWORD   uniqueCreateIndex	: 30;	// [30�����ۼ�ֵ] 30bit��ÿ30���ڵ��������Ͽ�����1073741823������
            QWORD   uniqueServerId		: 2;	// [���������  ] 2 bit����¼���ߵĲ���λ�ã��Ա����ڲ�ͬ�������ϵı���ظ�
        } details;

        struct
        {
            DWORD	wIndex;		// ����
			BYTE	flags;		// ��־
			WORD	overlap;	// ��������
			BYTE	usetimes;	// ʹ�ô���
			QWORD	uniID;		// ȫ��Ψһ���
        };
    };
};

// ԭʼ�������ݿռ��С(16+90=106)
// ��������buffer ��С���ĳ�164 (16 + 112 = 128)
struct SRawItemBuffer : public SItemBase
{
	BYTE buffer[120];		// ���ÿռ�
};

// ��ͨ����
struct SNormalItem : public SItemBase
{
	DWORD reserve;			// ������
};

// ��ʯ����
struct SJewelItem : public SItemBase
{
	DWORD reserve;			// ������
};

// �ɱ����
struct SChangeItem : public SItemBase
{
	DWORD dwCreateTime;		// ����ʱ��
};

// ���ϵ���
struct SMetrailItem : public SItemBase
{
	DWORD reserve;			// ������
};

// ���ߵ���
struct SToolItem : public SItemBase
{
	DWORD reserve;			// ������
};

// �鼮����
struct SBookItem : public SItemBase
{
	DWORD reserved;			// ������
};

// �������
struct STaskItem : public SItemBase
{
	DWORD taskID;			// ����������
}; 

// ��ʱ���յ���
struct STimeItem : public SItemBase
{
	DWORD recycleTiem;		// ����ʱ��
};

// ���ͼ���
struct SPetSkill
{
	WORD	wTypeID;			// �ڼ����б��еı��
	BYTE		byLevel : 4;	// ���ܵĵȼ� ���15
	SPetSkill::SPetSkill():wTypeID(0),byLevel(0){}
};

// ���ͻ���������Ϣ
struct SFightPetBaseAttri
{
// 	DWORD			m_BaseMaxHp;		// �����������
// 	DWORD			m_BaseMaxMp;		// �����������
// 	DWORD			m_BaseMaxTp;		// �����������
// 	WORD			m_BaseAtk;			// ��������
// 	WORD			m_BaseDefence;		// ��������
	BYTE			m_BaseCtrAtk;		// ��������
	BYTE			m_BaseEscape;		// �������
	BYTE			m_BaseWuXing;		// ��������
};

// ���ͻ�����Ϣ
struct SFightPetBase	:public SFightPetBaseAttri	
{
	enum { FP_NAME = 11, };
	enum { FP_SKILL_MAX = 12, };

	char		m_fpName[FP_NAME];			// ����
	char		m_owername[FP_NAME];
	DWORD		m_niconID;					// ����ICON
	QWORD		m_fpExp;					// ����
	QWORD		m_fpMaxExp;					// �����
	WORD		m_fpID;						// ID
	WORD		m_nmodelID;					// ģ��ID
	WORD		m_fpRelation;				// ���ܶ�
	WORD		m_nMaxHp;					// ���HP
	WORD		m_nCurHp;					// ��ǰHP
	WORD		m_nMaxMp;					// ���MP
	WORD		m_nCurMp;					// ��ǰMP
	WORD		m_nGongJi;					// ������
	WORD		m_nFangYu;					// ������
	WORD		m_nScore			: 12;	// �ۺ����� ���4095
	BYTE		m_bSex				: 2 ;	// �Ա�
	BYTE		m_fpActivedSkill	: 4 ;	// ��ǰ�Ѿ�����ļ��ܿ�,��󲻳���15��
	BYTE		m_fpLevel;					// �ȼ�
	BYTE		m_fpTired;					// ƣ�Ͷ�
	BYTE		m_fpMood;					// ����
	BYTE		m_fpHpIncRadio;				// Ѫ����ϵ��
	BYTE		m_fpMpIncRadio;				// ������ϵ��
	BYTE		m_fpGongJi;					// ��������ϵ��
	BYTE		m_fpFangYu;					// ��������ϵ��
	BYTE		m_fpBaoJi;					// ��������ϵ��
	BYTE		m_fpShanBi;					// ��������ϵ��
	BYTE		m_nRare;					// ϡ�ж�
	BYTE		m_nBringLevel ;				// Я���ȼ�
	BYTE		m_fpWuXing;					// ����
	SPetSkill	m_fpSkill[FP_SKILL_MAX];	// ���ͼ���
	DWORD		m_fpLearnSkill;				// ���͵�ǰѧϰװ���ļ�������ƷID
};

struct SFightPetItem : public SItemBase
{
	SFightPetBase fpInfo;	// ������Ϣ
};

enum FightPetAttType
{
	FP_ACTIVELY,			// ��������
	FP_PASSIVE,				// ��������
	FP_FOLLOW,				// ����

	FP_MAX,					// Max
};

#define MAX_ATTRI_NUM 2   //װ�����Ը�������
// װ�����ߣ����ܳ���106�ֽڣ�
struct SEquipment : public SItemBase, public SEquipDataEx
{
	//wk 20150205 �Ż���Ҫ������
	//static const int MAX_SLOTS = 6;			// ���׶�
	//static const int MAX_BORN_ATTRI = 3;	// �������Ը�������ֵ
	//static const int MAX_GRADE_ATTRI = 3;	// �������Ը�������ֵ
	//static const int MAX_STAR_NUM = 12;		// ���ǵĸ���
	//static const int MAX_GRADE_NUM = 10;	// ���׵�������
	static const int MAX_SLOTS = 1;			// ���׶�
	static const int MAX_BORN_ATTRI = 1;	// �������Ը�������ֵ
	static const int MAX_GRADE_ATTRI = 1;	// �������Ը�������ֵ
	static const int MAX_STAR_NUM = 1;		// ���ǵĸ���
	static const int MAX_GRADE_NUM = 1;	// ���׵�������

	// ����
	enum
	{
		SLOT_IS_EMPTY = 0,		// û����
		SLOT_IS_INVALID,		// ����ʧ�ܣ�������
		SLOT_IS_VALID,			// ���ǳɹ���ʵ����
	};

	struct ExtraAttri
	{
		BYTE type;				//
		WORD value;				
	};

	// �ף�12���ֽڣ�
	struct JewelSlot
	{
		BYTE	type ;				// ��ʯ����(>0)
		BYTE	value;				// ��ʯ�ȼ�(>0)
		// typeΪ0ʱ��valueΪ0��ʾ�ò���Ч��valueΪ1��ʾ�ò�Ϊ��
		BOOL isEmpty()   const { return type == 0 && value == 1; }
		BOOL isInvalid() const { return type == 0 && value == 0; }
		BOOL isJewel()   const { return type != 0 && value != 0; }
	} slots[MAX_SLOTS];

	struct Attribute 
	{
		WORD currWear;				// ��ǰ�;ö�
		WORD maxWear;				// ��ǰ����;ö�
		BYTE grade;						// װ��Ʒ��
		BYTE		bIntensifyLevel;		//ǿ���ȼ�
		BYTE		bCurrentStar;	  //��ǰ�Ǽ�
		BYTE		bJDingAttriNum;//��ǰ�Ѿ������ļ�����������

		ExtraAttri bornAttri[MAX_ATTRI_NUM];		// ��ʼ��������,װ��ǿ���ı������
		ExtraAttri starAttri[MAX_ATTRI_NUM];		// ���ǻ�������
		ExtraAttri MaxJDingAttri[MAX_SLOTS];	// ����������		
	} attribute;

	struct EquipKeYin     //��ӡ
	{
		BYTE		MaxKeYinLevel[MAX_BORN_ATTRI];				//��Ӧ��ӡ�ȼ�
		ExtraAttri MaxKeYinAttri[MAX_BORN_ATTRI];	// ����ӡ����		
	}equipkeyin;
};

// ���ӵ�����
struct SCellPos 
{ 
	WORD wCellPos;
};

// Tile������
struct STilePos 
{ 
    WORD wTileX;
    WORD wTileY; 
};

// ���ڵ������еĵ���
struct SPackageItem : public SCellPos, public SRawItemBuffer
{
};

// ���ڵ��ϵĵ���
struct SGroundItem : public STilePos, public SRawItemBuffer
{
};

struct SSkill
{
	WORD	wTypeID;	    // �ڼ����б��еı��
	BYTE	byLevel;	    // ���ܵĵȼ�
	DWORD	dwProficiency;	// ���ܵ�������

	SSkill()
	{
		memset(this, 0, sizeof(*this));
	}
};

typedef SSkill SFlyData;		// �Ṧ
typedef SSkill SProtectedData;	// ����

#pragma pack (pop)

// ���ڼ�¼��־�Ľṹ��
struct LogInfo
{
    LogInfo(DWORD rcdtype, LPCSTR description = NULL) : rcdtype(rcdtype), description(description) {}

    DWORD   rcdtype;        // ��¼��� 
    LPCSTR  description;    // ��¼˵��
};