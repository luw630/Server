#pragma once

static const int ITEMNAME_MAXLEN		= 21;					// 道具名最大字符数
static const int MAX_REGION_NUMBER		= 300;					// 最大场景数量
//20150427 wk  优化玩家数据
//static const int CONST_BUYBACK_COUNT	= 40;					// 最多16个回购物品
static const int CONST_BUYBACK_COUNT = 1;					// 最多16个回购物品

// 侠义道3基本背包的矩阵
//wk 20150205 优化不要的数据
//const DWORD PackageWidth = 6;								// 一个背包单元的宽度
//const DWORD PackageHight = 4;								// 一个背包单元的高度
//const DWORD PackageType = 4;								// 背包的类型数目（目前有基本，扩展1，扩展2，任务4种）
//const DWORD g_PackageMaxItem = 10;							// 包裹中最大道具数

const DWORD PackageWidth	= 1;								// 一个背包单元的宽度
const DWORD PackageHight	= 1;								// 一个背包单元的高度
const DWORD PackageType		= 1;								// 背包的类型数目（目前有基本，扩展1，扩展2，任务4种）
const DWORD PackageAllHight	= PackageHight * PackageType;		// 背包的总高度
const DWORD PackageAllCells = PackageAllHight * PackageWidth;	// 所需背包格子总数
// 侠义道3包裹常量定义
const DWORD g_PackageMaxItem	= 1;							// 包裹中最大道具数
const DWORD g_PackageMoneyIndex = g_PackageMaxItem;				// 包裹中货币的索引

// 玩家/侠客装备类型定义
enum EQUIP_TYPE
{
	EQUIP_T_WEAPON	= 101,		// 武器1
	EQUIP_T_HAT,				//  头部装备
	EQUIP_T_WRIST,				// 护腕
	EQUIP_T_PAT,				// 裤子
	EQUIP_T_WAIST,				// 肩膀
	EQUIP_T_SHOSE,				// 鞋子
	EQUIP_T_CLOTH,				// 衣服
	EQUIP_T_MANTLE,				//  备用1 宝物
	EQUIP_T_PENDANTS,			// 备用2 法器
	EQUIP_T_PROTECTION,			// 项链
	EQUIP_T_RING,				// 戒指
	EQUIP_T_PRECIOUS,			// 武器2
	EQUIP_T_SHIZHUANG_HAT,		//  武器3
	EQUIP_T_SHIZHUANG_CLOTH,	// 时装（衣服） t腰带
};

// 装备位置定义
enum EQUIP_POSITION
{
	EQUIP_P_WEAPON = 0,			// 武器
	EQUIP_P_SHOSE,				// 鞋子
	EQUIP_P_PAT,				// 裤子
	EQUIP_P_WAIST,				// 肩膀
	EQUIP_P_WRIST,				// 护腕
	EQUIP_P_CLOTH,				// 衣服
	EQUIP_P_HAT,				//  头部装备
	EQUIP_P_MANTLE,				// 备用1
	EQUIP_P_PENDANTS1,			// 配饰1
	EQUIP_P_PENDANTS2,			//  备用2
	EQUIP_P_PROTECTION,			// 项链
	EQUIP_P_RING1,				// 戒指
	EQUIP_P_RING2,				// 戒指2
	EQUIP_P_PRECIOUS,			// 武器2
	EQUIP_P_SHIZHUANG_HAT1,		// 武器3
	EQUIP_P_SHIZHUANG_CLOTH1,	// 时装衣服

	EQUIP_P_MAX,				// Max
}; 

static int ShowEquipPos[] = 
{
	EQUIP_P_WEAPON,		
	EQUIP_P_WRIST,	
	EQUIP_P_SHOSE,	
	EQUIP_P_PAT,				// 裤子
	EQUIP_P_CLOTH,	
	EQUIP_P_HAT,
	EQUIP_P_MANTLE,	
	EQUIP_P_PENDANTS1,
	EQUIP_P_PENDANTS2,
	EQUIP_P_PROTECTION,			// 项链
	EQUIP_P_PRECIOUS,
	EQUIP_P_SHIZHUANG_HAT1,
	EQUIP_P_SHIZHUANG_CLOTH1
};

// 装备属性定义
struct SEquipDataEx
{
	//月下夜想曲装备属性新定义
	enum EQUIP_EXT_ATTRI
	{
		//已经调整顺序为月夜

		EEA_MAXHP,					// 生命上限
		EEA_MAXMP,					// 精力上限
		EEA_GONGJI,					// 攻击,伤害
		EEA_FANGYU,					// 防御
		EEA_HIT,							//命中
		EEA_SHANBI,					// 闪避
		EEA_BAOJI,					// 暴击
		
		//月下夜想曲 新增
		
		EEA_UNCRIT, //抗暴
		EEA_WRECK,//破击
		EEA_UNWRECK,	//抗破
		EEA_PUNCTURE,	//穿刺
		EEA_UNPUNCTURE,	//抗穿
		EEA_ICEATK,	//冰伤
		EEA_ICEDEF,	//冰抗
		EEA_FIREATK,	//火伤
		EEA_FIREDEF,		//火抗
		EEA_DARKATK,		//暗伤
		EEA_DARKDEF,		//暗抗
		EEA_HOLYATK,		//神圣伤
		EEA_HOLYDEF,		//神圣抗
		EEA_HPPERCENT,	//血量百分比加成
		EEA_MPPERCENT,	//精力百分比加成
		EEA_ATKPERCENT,	//伤害百分比加成
		EEA_DEFPERCENT,	//防御百分比加成
		EEA_CRITPERCENT,	//暴击百分比加成
		EEA_WEAKPERCENT,		//破击百分比加成
		EEA_HITPERCENT,		//命中百分比加成

		//月下


		
		EEA_MAXTP,					// 体力上限
		EEA_SPEED,					// 移动速度
		EEA_ATKSPEED,				// 攻击速度

		// 新添加的属性
		EEA_REDUCEDAMAGE,			// 伤害减免
		EEA_ABSDAMAGE,				// 绝对伤害
		EEA_NONFANGYU,				// 无视防御
		EEA_MULTIBAOJI,				// 暴击倍数

		EEA_ICE_DAMAGE,				// 冰伤害
		EEA_ICE_DEFENCE,			// 冰抗性
		EEA_FIRE_DAMAGE,			// 火伤害
		EEA_FIRE_DEFENCE,			// 火抗性
		EEA_XUAN_DAMAGE,			// 玄伤害
		EEA_XUAN_DEFENCE,			// 玄抗性
		EEA_POISON_DAMAGE,			// 毒伤害
		EEA_POISON_DEFENCE,			// 毒抗性

		EEA_MAX,
	};
};

// 道具类型定义
enum EItemType
{
	ITEM_T_MEDICINE_HP		= 7,	// 生命药
	ITEM_T_MEDICINE_MP		= 8,	// 内力药
	ITEM_T_MEDICINE_TP		= 9,	// 体力药
	ITEM_T_NORMAL			= 10,	// 普通道具
	ITEM_T_JEWEL			= 11,	// 宝石道具
	ITEM_T_CHANGE			= 12,	// 可变道具
	ITEM_T_METRAIL			= 13,	// 材料道具
	ITEM_T_TOOL				= 14,	// 工具道具
	ITEM_T_BOOK				= 15,	// 书籍道具
	ITEM_T_TASK				= 16,	// 任务道具
	ITEM_T_RECYCLE			= 17,	// 定时回收道具
	ITEM_T_FIGHT_PET		= 18,	// 侠客道具
	ITEM_T_PRESENT          = 19,   // 礼包道具
	ITEM_T_SUIT_SCROLL		= 20,	// 套装卷轴
	ITEM_T_SKILLCHIP		= 21,   //宠物技能书碎片
	ITEM_T_PETSKILLBOOK		= 22,	//宠物技能书
	ITEM_T_EQUIPMENT_BEGIN	= 101,	// 装备类道具（和EQUIP_TYPE对应）
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

// 判断装备位置是否为需要换3D模型的位置
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

// 道具移动方向
enum ItemMoveDirection
{
	IMD_INPACKAGE,		// 在背包内移动
	IMD_PACK2EQUIP,		// 背包到装备栏
	IMD_EQUIP2PACK,		// 装备栏到背包
	IMD_FP_PACK2EQUIP,	// 背包到侠客装备栏
	IMD_FP_EQUIP2PACK,	// 侠客装备栏到背包
	IMD_MAX,			// Max
};

// 包裹类型
enum ItemPackageModel
{
	IPM_ITEM,			// 单物
	IPM_BUDAI,			// 钱，银币
	IPM_TANMUBOX,		// 檀木箱
	IPM_YINKOUBOX,		// 银口箱
	IPM_GOLDENBOX,		// 黄金箱
};

// 包裹中道具的拾取类别
enum PackageItemPickType
{
	PIPT_FREE,			// 可以自由拾取
	PIPT_TAKECHANCE,	// 处于摇号阶段
	PIPT_LEADER,		// 处于队长分配
};

#pragma pack (push)
#pragma pack (1)

// 最基本的（保存用）道具数据结构
struct SItemBase
{
	enum
	{
		F_NONE			= 0,						// 无标记
		F_SYSBINDED		= 0x1,						// 已被系统绑定
		F_PLAYERBINDED	= 0x2,						// 已经玩家绑定
    };

	static const int YEAR_SEGMENT     = 2009;		// 时间戳的起始时间为2009年
	static const int MAX_ACCUMULATOR  = 0x3fffffff;	// 30秒内累加值的最大值为0x3fffffff == 1073741823

	inline QWORD uniqueId() const { return uniID; }
	inline void uniqueId(QWORD v) { uniID = v;	  }
	
	// 判断当前物品是否处于绑定状态。
	inline bool IsBindState()
	{	
		return (flags & SItemBase::F_PLAYERBINDED)!=0 || (flags & SItemBase::F_SYSBINDED)!=0;		
	}

	union
	{
		struct
        {
            QWORD   wIndex				: 32;	// 在物品列表中的编号，通过它可以查出所有关于该道具的数据
            QWORD   flags				: 8;	// 本道具的道具属性标志
			QWORD	overlap				: 16;	// 当前叠加数量
			QWORD	usetimes			: 8;	// 可以使用的次数（如果是一次性的，则为1，使用完便删除）
            QWORD   uniqueTimestamp		: 32;	// [时间戳      ] 32bit，以30秒为单位进行记录
            QWORD   uniqueCreateIndex	: 30;	// [30秒内累加值] 30bit，每30秒内单服务器上可生成1073741823个道具
            QWORD   uniqueServerId		: 2;	// [服务器编号  ] 2 bit，记录道具的产生位置，以避免在不同服务器上的编号重复
        } details;

        struct
        {
            DWORD	wIndex;		// 索引
			BYTE	flags;		// 标志
			WORD	overlap;	// 叠加数量
			BYTE	usetimes;	// 使用次数
			QWORD	uniID;		// 全局唯一编号
        };
    };
};

// 原始道具数据空间大小(16+90=106)
// 现在扩大buffer 大小，改成164 (16 + 112 = 128)
struct SRawItemBuffer : public SItemBase
{
	BYTE buffer[120];		// 公用空间
};

// 普通道具
struct SNormalItem : public SItemBase
{
	DWORD reserve;			// 保留用
};

// 宝石道具
struct SJewelItem : public SItemBase
{
	DWORD reserve;			// 保留用
};

// 可变道具
struct SChangeItem : public SItemBase
{
	DWORD dwCreateTime;		// 产生时间
};

// 材料道具
struct SMetrailItem : public SItemBase
{
	DWORD reserve;			// 保留用
};

// 工具道具
struct SToolItem : public SItemBase
{
	DWORD reserve;			// 保留用
};

// 书籍道具
struct SBookItem : public SItemBase
{
	DWORD reserved;			// 保留用
};

// 任务道具
struct STaskItem : public SItemBase
{
	DWORD taskID;			// 关联的任务
}; 

// 定时回收道具
struct STimeItem : public SItemBase
{
	DWORD recycleTiem;		// 回收时间
};

// 侠客技能
struct SPetSkill
{
	WORD	wTypeID;			// 在技能列表中的编号
	BYTE		byLevel : 4;	// 技能的等级 最高15
	SPetSkill::SPetSkill():wTypeID(0),byLevel(0){}
};

// 侠客基础属性信息
struct SFightPetBaseAttri
{
// 	DWORD			m_BaseMaxHp;		// 基础最大生命
// 	DWORD			m_BaseMaxMp;		// 基础最大内力
// 	DWORD			m_BaseMaxTp;		// 基础最大体力
// 	WORD			m_BaseAtk;			// 基础攻击
// 	WORD			m_BaseDefence;		// 基础防御
	BYTE			m_BaseCtrAtk;		// 基础暴击
	BYTE			m_BaseEscape;		// 基础躲避
	BYTE			m_BaseWuXing;		// 基础悟性
};

// 侠客基本信息
struct SFightPetBase	:public SFightPetBaseAttri	
{
	enum { FP_NAME = 11, };
	enum { FP_SKILL_MAX = 12, };

	char		m_fpName[FP_NAME];			// 名字
	char		m_owername[FP_NAME];
	DWORD		m_niconID;					// 道具ICON
	QWORD		m_fpExp;					// 经验
	QWORD		m_fpMaxExp;					// 最大经验
	WORD		m_fpID;						// ID
	WORD		m_nmodelID;					// 模型ID
	WORD		m_fpRelation;				// 亲密度
	WORD		m_nMaxHp;					// 最大HP
	WORD		m_nCurHp;					// 当前HP
	WORD		m_nMaxMp;					// 最大MP
	WORD		m_nCurMp;					// 当前MP
	WORD		m_nGongJi;					// 攻击力
	WORD		m_nFangYu;					// 防御力
	WORD		m_nScore			: 12;	// 综合评分 最大4095
	BYTE		m_bSex				: 2 ;	// 性别
	BYTE		m_fpActivedSkill	: 4 ;	// 当前已经激活的技能孔,最大不超过15个
	BYTE		m_fpLevel;					// 等级
	BYTE		m_fpTired;					// 疲劳度
	BYTE		m_fpMood;					// 心情
	BYTE		m_fpHpIncRadio;				// 血增加系数
	BYTE		m_fpMpIncRadio;				// 蓝增加系数
	BYTE		m_fpGongJi;					// 攻击增加系数
	BYTE		m_fpFangYu;					// 防御增加系数
	BYTE		m_fpBaoJi;					// 暴击增加系数
	BYTE		m_fpShanBi;					// 闪避增加系数
	BYTE		m_nRare;					// 稀有度
	BYTE		m_nBringLevel ;				// 携带等级
	BYTE		m_fpWuXing;					// 悟性
	SPetSkill	m_fpSkill[FP_SKILL_MAX];	// 侠客技能
	DWORD		m_fpLearnSkill;				// 侠客当前学习装备的技能书物品ID
};

struct SFightPetItem : public SItemBase
{
	SFightPetBase fpInfo;	// 侠客信息
};

enum FightPetAttType
{
	FP_ACTIVELY,			// 主动攻击
	FP_PASSIVE,				// 被动攻击
	FP_FOLLOW,				// 跟随

	FP_MAX,					// Max
};

#define MAX_ATTRI_NUM 2   //装备属性附加条数
// 装备道具（不能超过106字节）
struct SEquipment : public SItemBase, public SEquipDataEx
{
	//wk 20150205 优化不要的数据
	//static const int MAX_SLOTS = 6;			// 最大孔洞
	//static const int MAX_BORN_ATTRI = 3;	// 附加属性个数上限值
	//static const int MAX_GRADE_ATTRI = 3;	// 阶数属性个数上限值
	//static const int MAX_STAR_NUM = 12;		// 升星的个数
	//static const int MAX_GRADE_NUM = 10;	// 升阶的最大个数
	static const int MAX_SLOTS = 1;			// 最大孔洞
	static const int MAX_BORN_ATTRI = 1;	// 附加属性个数上限值
	static const int MAX_GRADE_ATTRI = 1;	// 阶数属性个数上限值
	static const int MAX_STAR_NUM = 1;		// 升星的个数
	static const int MAX_GRADE_NUM = 1;	// 升阶的最大个数

	// 升星
	enum
	{
		SLOT_IS_EMPTY = 0,		// 没有星
		SLOT_IS_INVALID,		// 升星失败，空心星
		SLOT_IS_VALID,			// 升星成功，实心星
	};

	struct ExtraAttri
	{
		BYTE type;				//
		WORD value;				
	};

	// 孔（12个字节）
	struct JewelSlot
	{
		BYTE	type ;				// 玉石类型(>0)
		BYTE	value;				// 玉石等级(>0)
		// type为0时，value为0表示该槽无效，value为1表示该槽为空
		BOOL isEmpty()   const { return type == 0 && value == 1; }
		BOOL isInvalid() const { return type == 0 && value == 0; }
		BOOL isJewel()   const { return type != 0 && value != 0; }
	} slots[MAX_SLOTS];

	struct Attribute 
	{
		WORD currWear;				// 当前耐久度
		WORD maxWear;				// 当前最大耐久度
		BYTE grade;						// 装备品质
		BYTE		bIntensifyLevel;		//强化等级
		BYTE		bCurrentStar;	  //当前星级
		BYTE		bJDingAttriNum;//当前已经开启的鉴定属性条数

		ExtraAttri bornAttri[MAX_ATTRI_NUM];		// 初始附加属性,装备强化改变的属性
		ExtraAttri starAttri[MAX_ATTRI_NUM];		// 升星基础属性
		ExtraAttri MaxJDingAttri[MAX_SLOTS];	// 最大鉴定属性		
	} attribute;

	struct EquipKeYin     //刻印
	{
		BYTE		MaxKeYinLevel[MAX_BORN_ATTRI];				//对应刻印等级
		ExtraAttri MaxKeYinAttri[MAX_BORN_ATTRI];	// 最大刻印属性		
	}equipkeyin;
};

// 格子的坐标
struct SCellPos 
{ 
	WORD wCellPos;
};

// Tile的坐标
struct STilePos 
{ 
    WORD wTileX;
    WORD wTileY; 
};

// 放在道具栏中的道具
struct SPackageItem : public SCellPos, public SRawItemBuffer
{
};

// 放在地上的道具
struct SGroundItem : public STilePos, public SRawItemBuffer
{
};

struct SSkill
{
	WORD	wTypeID;	    // 在技能列表中的编号
	BYTE	byLevel;	    // 技能的等级
	DWORD	dwProficiency;	// 技能的熟练度

	SSkill()
	{
		memset(this, 0, sizeof(*this));
	}
};

typedef SSkill SFlyData;		// 轻功
typedef SSkill SProtectedData;	// 护体

#pragma pack (pop)

// 用于记录日志的结构体
struct LogInfo
{
    LogInfo(DWORD rcdtype, LPCSTR description = NULL) : rcdtype(rcdtype), description(description) {}

    DWORD   rcdtype;        // 记录编号 
    LPCSTR  description;    // 记录说明
};