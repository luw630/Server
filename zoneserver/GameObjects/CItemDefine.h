#ifndef CITEMDEFINE_H
#define CITEMDEFINE_H

#include "Networkmodule\ItemTypedef.h"

// 道具的门派
enum ItemOrbit
{
	IO_NONE = 0,			// 无门派需求
	IO_WUYUE,				// 五岳
	IO_SHUSHAN,				// 蜀山
	IO_SHAOLIN,				// 少林
	IO_CIHANG,				// 慈航
	IO_RIYUE,				// 日月

	IO_XIAKE,				// 侠客通用
	IO_XA_WUYUE,				// 侠客五岳
	IO_XA_SHUSHAN,				// 侠客蜀山
	IO_XA_SHAOLIN,				// 侠客少林
	IO_XA_CIHANG,				// 侠客慈航
	IO_XA_RIYUE,				// 侠客日月
	
	I0_MAX,					// Max
};

// 道具的性别需求
enum ItemSex
{
	IS_NONE = 0,
	IS_MALE,				// 男
	IS_FEMALE,				// 女
};

// 道具的颜色
enum ItemColor
{
	IC_WHITE = 1,			// 白色
	IC_Green = 2,			// 绿色
	IC_BLUE,				// 蓝色
	IC_PURPLE,				// 紫色
	IC_GOLD,				// 橘色(金色)

	IC_MAX
};

// 耐久改变方式
enum WearChangeType
{
	WCT_NONE		= 0x0,	// 不改变
	WCT_ATTACK		= 0x1,	// 攻击
	WCT_BEATTACED	= 0x2,	// 被攻击
	WCT_DIE			= 0x4,	// 死亡
	WCT_EQUIP		= 0x8,	// 装备
	WCT_PACKED		= 0x10,	// 携带
};

// 买入货币类型
enum BuyItemMoneyType
{
	BIMT_ONLYBIND	= 1,	// 只使用绑定货币	银币
	BIMT_ONLYUNBIND,		// 只使用非绑定货币 银两
	BIMT_BINDFIRST,			// 绑定优先使用，不足再扣非绑定
};

// 装备的最大阶数
enum
{
	MAX_REDUCE_EQUIP_GRADE = 9,
	MAX_EQUIP_GRADE = 10,
};

// 套装附加属性的类型
// 与itemMsg文件中的 装备套装加成枚举 对应
enum SUIT_ATTRI_TYPE
{
	SAT_LEVEL,
	SAT_GRADE,
	SAT_COLOR,
	SAT_JEWEL,
	SAT_STAR,

	SAT_MAX,
};

// [11-12-2012 16:17 gw]+物品绑定类型 
enum ITEM_BIND_TYPE
{
	EITEM_BIND_NONE = 0, // 永不绑定
	EITEM_BIND_GET = 0x01, //获得的时候绑定
	EITEM_BIND_EQUIP=0x02, //装备的时候绑定
};

// 道具的基本属性
struct SItemBaseData
{
public:
	SItemBaseData() { memset(this, 0, sizeof(SItemBaseData)); }

public:
	char	m_Name[ITEMNAME_MAXLEN];	// 名称
	DWORD	m_BuyPrice;					// 买入价格
	DWORD	m_SellPrice;				// 卖出价格
	DWORD	m_LocSrvCoolingTime;		// 冷却时间
	DWORD	m_LocSrvPreTime;			// 吟唱时间
	DWORD	m_ModelIDMan;				// 模型ID（可见装备男）
	DWORD	m_ModelIDWoman;				// 模型ID（可见装备女）
	DWORD	m_ID;						// 唯一ID
	DWORD	m_ScriptID;					// 脚本ID
	DWORD   m_StepNum;                  // 阶数2011.8.9  2013.12.25 更改为品质
	WORD	m_Overlay;					// 最大叠加数
	WORD	m_Level;					// 等级需求
	WORD	m_MaxWear;					// 最大耐久数
	WORD	m_WearReduceCount;			// 耐久改变数
	WORD	m_DieDropRate;				// 死亡掉落概率
	WORD	m_LocSrvCDGroupID;			// 冷却组
	WORD	m_TaskID;					// 关联的任务ID
	WORD	m_TaskNeed;					// 任务需求数量
	BYTE	m_BuyMoneyType;				// 买入货币模式
	BYTE	m_SellMoneyType;			// 卖出货币类型
	BYTE	m_WearChange;				// 耐久改变方式
	BYTE	m_Type;						// 道具类型
	BYTE	m_School;					// 门派需求
	BYTE	m_Sex;						// 性别需求
	BYTE	m_Color;					// 道具颜色
	bool	m_CanUse;					// 可否使用
	bool	m_CanDrop;					// 可否丢弃
	bool	m_CanSale;					// 可否卖店
	bool	m_CanTran;					// 可否交易
	bool	m_CanFix;					// 可否修理
	bool	m_CanStroage;				// 可否存入仓库
	bool	m_CanJianDing;				// 是否可鉴定
	// ===============新属性=====================
	bool	m_CanQuality;				// 可否升品质
	bool	m_CanGrade;					// 可否升阶
	bool	m_CanRefine;				// 可否重置(重置属性条数)
	bool	m_CanRefresh;				// 可否刷新(重置属性数值)
	bool	m_CanStar;					// 可否升星
	bool	m_CanUpdate;				// 可否升级
	bool	m_CanMakeHole;				// 可否打孔
	bool	m_CanTakeOff;				// 可否摘除
	bool	m_CanUnStar;				// 可否洗星
	bool	m_CanEquipDescomposition;	// 可否装备分解
	bool	m_CanEquipSmelting;			// 装备熔炼
	BYTE    m_GemType;					// 宝石类型	

	BYTE	m_BindType;					// [11-12-2012 16:23 gw]+  绑定类型
};

// 道具属性加成
class SItemFactorData
{
public:
	SItemFactorData(){ memset(this, 0, sizeof(SItemFactorData)); }
	
public:
	int		m_ValueFactor[SEquipDataEx::EEA_MAX];
	int		m_PrecentFactor[SEquipDataEx::EEA_MAX];
};

// 装备升级阶数信息
struct SItemUpdateGradeInfo
{
	BYTE	grade; // 阶数
	WORD	baseSuccessRate; // 成功率
	WORD	baseFailedReduceGradeRate; // 失败之后降阶的概率
	WORD	baseFailedDestroyRate; // 失败后销毁的概率
	BYTE    byMoneyType;//钱类型
	DWORD	needMoney;	// 需要金钱数目	

	DWORD   dwNeedItemID; // 需要材料ID
	BYTE	byNeedItemNum; // 需要材料的数量

	DWORD	dwLuckyCharmID; // 可用幸运符ID
	BYTE	byLuckyCharmNum; // 可用幸运符需求数目
	WORD	wLuckyCharmAddSuccessRate; // 幸运符添加成功率
	
};

// [2012-8-29 12-42 gw: +附加属性的需求结构,主要用在洗练，精炼，升阶]  
struct SEquipExtraAttriRequire
{
	static const int msc_iXiLianExtraAttriMax = 6; // 最大洗练出附加属性条数为6条
	// 附加属性
	enum TExtraAttrDataElem
	{	
		EExtraData_LowerLimit = 0,	// 下限
		EExtraData_UpperLimit,		// 上限
		EExtraData_Rate,			// 概率
		EExtraData_JingLianAdd,		// 精炼添加值

		EExtraData_Max,
	};
	int		iaExtraData[SEquipDataEx::EEA_MAX][EExtraData_Max]; // 附加属性的详细信息

	WORD  waExtraAttriRate[msc_iXiLianExtraAttriMax]; // 附加属性产生条数的概率

	DWORD ResetItemID;		//洗练需要的道具ID
	BYTE  ResetNeedNum;		//洗练需要的道具数量
	BYTE  ResetMoneyType;	//洗练需要的钱类型	
	long  ResetNeedMoney;	//洗练需要的钱	

	DWORD RefreshItemID;	//精炼需要的道具ID	
	BYTE  RefreshNeedNum;	//精炼需要的道具数量
	BYTE  RefreshMoneyType;	//精炼的钱类型	
	long  RefreshNeedMoney;	//精炼需要的钱	

	DWORD ExternLockedItemID; //洗练可用锁定石ID
	byte  ExternLockeItemNum; // 洗练锁定一条属性需要锁定石的数量
};

// 装备升阶精练增加属性
struct SItemUpgradeAttribute
{
	// 升阶基础附加属性数据条数
	static const int msc_iStarAttriDataNum = 10;
	DWORD	wID;				// ID，唯一，通过计算而来
	WORD	wType;				// 装备的类型（衣服、护腕、裤子...）
	BYTE	bLevel;				// 装备的等级
	BYTE	bColor;				// 装备的品级
	WORD	wAttri[msc_iStarAttriDataNum];			// 升阶的基础附加属性
	WORD	wStarAttri[SEquipment::MAX_STAR_NUM];		// 升星的基础属性添加值

	struct SEquipExtraAttriRequire equipRefineRequire;	//装备洗练精炼的配置数据
};

// 装备升品质
struct SItemUpdateQuality
{
	BYTE	bColor;
	long	dwNeedMoney;
	BYTE    m_Type;
	WORD	rate[IC_MAX - 1];
	BYTE	bStoneNum;
	DWORD   NeedID;
	long    m_DestroyRate;
};

// 装备升级
struct SEquipLevelUp
{
	BYTE	level;
	BYTE	StoneNum;
	WORD	SuccessRate;
	DWORD	NeedMoney;
	BYTE    m_Type;
	DWORD   NeedID;
	long    m_DestroyRate;
};

// 装备升星
struct SEquipStar
{
	BYTE	num;
	DWORD	Rate;
	BYTE	StarMoneyType;
	long	money;
	DWORD   StarNeedID;
	BYTE    StarNum;
	BYTE	ReMoveStarMoneyType;
	long	ReMovemoney;
	DWORD   ReMoveStarNeedID;
	BYTE    RemoveStarNum;

	byte	bNeedReduceStarFailed; //失败后是否需要减星
	DWORD	dwProtectionCharacterID; // 升星可用保护符ID
	byte	byProtectionCharacterNum; // 可用保护符扣除数量
	DWORD	dwLuckyCharmID; // 升星可用幸运符ID
	byte	byLuckyCharmNum; // 可用幸运符扣除数量
	WORD	wLuckyCharmAddSuccessRate; //幸运符增加的成功率
};

// 装备附加属性最大值
struct SMaxExtraAttri
{
	DWORD	index;
	WORD	level;
	BYTE	color;
	BYTE	grade;

	WORD	ExtraData[SEquipDataEx::EEA_MAX];
};

struct SJewelAttribute
{
	DWORD	index;
	//BYTE	type;
	int		ExtraData[SEquipDataEx::EEA_MAX];
};

// 回元丹增加真气配置表
struct SHuiYuanDanInfo
{
	WORD	wLevel;
	WORD	wSPValue;
	WORD	wSuccessRate;
	WORD	wLianzhiRate;
	WORD	wXMTJRate;
};
struct SMakeHoleInfo
{
	//打孔编号ID	打孔需要的材料ID	消耗材料数量	打孔钱类型	打孔钱数量	打孔失败几率
	DWORD   Index;			//打孔编号ID	
	DWORD	HoleNeedID;		//打孔需要的材料ID	
	DWORD   HoleNum;		//消耗材料数量	
	BYTE    HoleMoneyType;	//打孔钱类型
	long    HoleMoney;		//打孔钱数量
	WORD    wMakeSuccessRate;	//打孔成功几率

	DWORD	dwProtectionCharacterID; // 升星可用保护符ID
	byte	byProtectionCharacterNum; // 可用保护符扣除数量
	DWORD	dwLuckyCharmID; // 升星可用幸运符ID
	byte	byLuckyCharmNum; // 可用幸运符扣除数量
	WORD	wLuckyCharmAddSuccessRate; //幸运符增加的成功率
	WORD	wReduceHoleRate; // 失败后降孔的概率
};
struct SInsertGem
{
	DWORD   Index;			//孔索引ID	
	BYTE    GemMoneyType;	//镶嵌孔用的钱类型	
	long    GemMoney;		//镶嵌孔用的钱
};
struct STakeOffGem
{
	DWORD Index;				//摘除索引ID			
	BYTE  TakeOffGemMoneyType;	//摘除钱类型
	long  TakeOffGemMoney;		//摘除花费
	DWORD TakeOffGemItemID;		//摘除材料ID
	BYTE  TakeOffGemNum;		//摘除材料数量
	long  TakeOffGemHoleRale;	//摘除宝石封洞几率
};
struct SDefineReset
{
	BYTE	m_Color;					// 道具颜色
	short   m_Rate[20];					// 出现几率
};

// 装备分解的配置存储元素
struct SEquipDecomposition
{
	DWORD	dwIndex;			//装备分解索引
	byte	byColor;			// 装备品质
	byte	byStarNum;			// 装备强化星数
	byte	byMoneyType;		// 需要金钱类型
	long	lNeedMoney;			// 需要多少金钱
	DWORD	dwGenNormalItemID;	// 产生普通材料ID
	byte	byGenNormalItemNum; // 产生普通材料数量
	WORD	wGenNormalItemRate; // 产生普通材料的几率	
	DWORD	dwGenSpecialItemID;	// 产生特殊材料ID
	byte	byGenSpecialItemNum; // 产生特殊材料数量
	WORD	wGenSpecialItemRate; // 产生特殊材料的几率

};

// 装备熔炼
struct SEquipSmelting
{
	//DWORD	dwIndex;				// 装备熔炼
	byte	byColor;				// 装备品质
	byte	byMoneyType;			// 需要金钱类型
	long	lNeedMoney;				// 需要多少金钱
	DWORD	dwGenNormalItemID;		// 产生普通材料ID
	byte	byGenNormalItemNum;		// 产生普通材料数量
	byte	byGenNormalItemColor;	// 产生材料的品质
	byte	byAttriFactor;			// 熔炼系数（用时需要/100）
	byte	byStarNum;			// 装备强化星数
};

// 极品预览附加属性配置
struct SEquipBestPreviewExtraAttri
{
	byte	byEquipType;		// 装备类型
	byte	byExtraAttriType;	// 极品附加属性类型
};

// 灵附相关属性配置
struct SEquipSpiritAttachBodyAttr
{
	WORD	wSuitEquipID; // 套装ID
	DWORD	dwNeedItemID; // 卷轴需求ID（元素）
	byte	byNeedItemNum; // 需求卷轴数量
	byte	byMoneyType; // 需求金钱类型
	long	lNeedMoney;	// 需要多少金钱
	WORD	wLevelLimit; // 等级限制（可能需要移动到lua文件比较合适）
};

#endif // CITEMDEFINE_H