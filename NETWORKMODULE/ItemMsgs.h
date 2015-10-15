#pragma once

#include "NetModule.h"
#include "StallDef.h"
#include "../pub/ConstValue.h"

// 道具相关消息类
DECLARE_MSG_MAP(SItemBaseMsg, SMessage, SMessage::EPRO_ITEM_MESSAGE)
//{{AFX
EPRO_DROP_ITEM,                             // 丢掉物品
EPRO_PICKUP_ITEM,                           // 拾取
EPRO_ADDPACKAGE_ITEM,                       // 物品栏中加物品
EPRO_ADDWAREHOUSE_ITEM,                     // 仓库中的物品
EPRO_ADDGROUND_ITEM,                        // 地面加物品
EPRO_CHECKGROUND_ITEM,						// 查看地面包裹
EPRO_CLOSEGROUND_ITEM,						// 取消查看地面包裹
EPRO_PRE_USE_ITEM,							// 物品预处理
EPRO_USE_ITEM,                              // 使用物品
EPRO_DELPACKAGE_ITEM,                       // 删除物品栏中道具
EPRO_DELGROUND_ITEM,                        // 删除地表物品
EPRO_MOVE_ITEM,                             // 移动物品在物品栏的位置
EPRO_UNLOAD_EQUIP,							// 在装备栏点击右键卸装
EPRO_REQUIRE_ITEMINFO,						// 请求获取一个物品的所有信息
EPRO_EQUIP_ITEM,                            // 装备外表可见的物品（群发）
EPRO_CHANGE_EQUIPCOL,                       // 改变装束颜色
EPRO_EXCHANGE_MESSAGE,                      // 道具交换
EPRO_BUY_MESSAGE,                           // 道具交易
RPRO_EQUIP_INFO,                            // 装备孔的信息
RPRO_WAREHOUSE_BASE,                        // 仓库基类
RPRO_EQUIPMENTINFO,                         // 发送一个人的装备数据
RPRO_SALEITEM_MESSAGE,                      // 出售到具相关
EPRO_AUTOUSE_ITEM,							// 自动吃药
EPRO_CONSUMEPOINT_UPDATE,                   // 消费积分变化，可能是因为玩家有购买行为
EPRO_SPLIT_ITEM,                            // 请求拆分道具
EPRO_UPDATE_ITEMINFO,                       // 更新（获取）某项道具配置信息
EPRO_TREASURE_SHOP,                         // 百宝商城
EPRO_DIAL,                                  // 大转盘操作
EPRO_NEATEN,                                // 整理背包
EPRO_UPDATE_TEMPITEM,                       // 更新玩家身上的临时物品
EPRO_AUTOFIGHT,							    // 自动战斗
EPRO_USEINTERVAL_EFFECT,                    // 播放使用间隔光效
EPRO_PLUGINGAME_MONEY,                      // 游戏大厅金钱改变
EPRO_EQUIP_WEAR_CHANGE,						// 装备耐久度变化
EPRO_UPDATEGROUND_ITEM,						// 更新地面物品
EPRO_ACTIVEEXTPACKAGE,						// 激活背包
EPRO_UPDATEITEM_ATTRIBUTE,					// 更新道具属性
EPRO_SPLITITEM_INPROGRESS,					// 是否拆分道具进行中
EPRO_START_FORGE,							// 拖入装备准备锻造
EPRO_DRAG_METRIAL,							// 拖入材料
EPRO_END_FORGE,								// 完成锻造
EPRO_MAKE_HOLE,								// 打孔
EPRO_INLAY_JEWEL,							// 镶嵌宝石
EPRO_TAKEOFF_JEWEL,							// 取下宝石
EPRO_SALEDITEM_LIST,						// 查看回购列表
EPRO_BUY_SALEDITEM,							// 买一个回购物品
EPRO_LOCKITEM_CELL,							// 锁定一个格子

EPRO_GROUNDITEM_TAKECHANCE,					// 获取道具摇号
EPRO_GROUNDITEM_CHANCEBACK,					// 客户端摇号回馈

EPRO_FIX_ONE_ITEM,							// 单件修理装备
EPRO_FIX_ALL_ITEM,							// 全部修理装备

EPRO_ADD_EQUIP,								// 强化拖入装备
EPRO_ADD_METRIAL,							// 强化拖入材料（原材料、保护材料）

EPRO_BEGIN_REFINE_GRADE,					// 精炼 升阶
EPRO_BEGIN_REFINE_STAR,						// 精炼 升星
EPRO_BEGIN_UPGRADE_LEVEL,					// 装备强化等级
EPRO_BEGIN_UPGRADE_QUALITY,					// 升品质


EPRO_BEGIN_RESET_ATTRIBUTE,					// 装备重置
EPRO_BEGIN_IDENTIFY,						// 装备鉴定
EPRO_BEGIN_IDENTIFY_SYNEQUIP,				// 开始洗练后同步新装备数据
EPRO_BEGIN_IDENTIFY_EXCHANGE,				// 开始洗练之后交换

EPRO_END_STRENGTHEN,						// 结束强化

EPRO_AFTER_UPGRADE_EQUIP,					// 显示强化后的装备
EPRO_EQUIP_INFO_IN_BAG,						// 背包里装备信息


EPRO_SPECIAL_USEITEM,                       // 特殊使用物品
EPRO_FIGHTPET_EQUIPITEM,                    // 侠客装备外表可见的物品（群发）
RPRO_FIGHTPET_EQUIPINFO,                    // 侠客装备孔的信息
EPRO_UNLOAD_FIGHTPETEQUIP,					// 在装备栏点击右键卸载侠客装备
EPRO_FIGHTPET_FIGHTPETTOITEM,				// 侠客转换为相应道具
EPRO_FIGHTPET_ITEMTOFIGHTPET,				// 侠客道具转换侠客
EPRO_RANK_EQUIPMENT_INFO,					// 排行榜中查看其他玩家装备信息

EPRO_RESETPOINT_METRIAL,					// 拖入洗点的材料
EPRO_END_RESETPOINT,						// 取消洗点

EPRO_EQUIP_ATTRICHANGE,						// 装备后属性发生变化

EPRO_DRAG_UPDATE_ITEM,						// 道具强化相关-拖入升级道具
EPRO_DRAG_UPDATE_METRIAL,					// 拖入升级道具的材料
EPRO_BEGIN_UPDATE_ITEM,						// 开始升级道具
EPRO_END_UPDATE_ITEM,						// 取消升级道具

EPRO_HUISHENDAN_LIST_INFO,					// 回神丹信息列表消息

EPRO__GET_ONLINEGIFT,						// 客户端请求获取在线新手礼包
EPRO_PACK_BUFFERPACK,						// 发送物品ID打包信息到客户端	

EPRO_FOLLOWITEM_QuestData,					// 请求随身商城列表数据
EPRO_BUYITEM_FOLLOW,						// 随机道具的买卖
EPRO_Bless_ReQuest,							// 祈福请求开始
EPRO_Bless_ExcellenceData,					// 祈福得到当前所有玩家获得最前面10次的高级道具通告 
EPRO_Bless_AllServerNotice,					// 祈福获得道具全服务通告
EPRO_Bless_StopBless,						// 停止祈福
EPRO_Bless_QuestAddItems,					// 请求添加道具

EPRO_EQUIP_FASHIONCHANGE,					// 时装模式或者普通模式
EPRO_EQUIP_SUIT_ATTRIBUTE,					// 显示套装附加属性是否激活

EPRO_ChannelCallSell_INFO,					// 叫卖信息
EPRO_ChannetCallBuy_INFO,					// 频道叫买

EPRO_STRENGTHEN_NeedMETRIAL,				// 强化需要材料
EPRO_BEGIN_ReMove_STAR,						// 洗心

EPRO__SEND_REWARD,			//发送相关活动礼包等消息

EPRO_INTENSIFY_INFO,				//强化描述
EPRO_QUALITY_INFO,				//升阶描述Quality
EPRO_RISE_INFO,						//升星描述
EPRO_KEYIN_INFO,				//刻印描述 

EPRO__EQUIP_SWITCH,			//武器切换
EPRO__ITEM_SALE,			//道具出售
EPRO_CLEAN_COLDTIME,				//清除强化冷却时间 

EPRO_BEGIN_KEYIN,							//请求升级刻印属性
EPRO_BEGIN_KEYINCHANGE,				//请求刻印属性转换
EPRO_BEGIN_JDING,				//请求装备鉴定
EPRO_JDING_INFO,				//请求装备鉴定描述信息
EPRO_JDING_COLOR,				//同步装备鉴定条颜色信息
EPRO_BEGIN_KEYINCHANGEINFO,				//请求刻印属性转换信息
EPRO_SUIT_CONDITION,				//套装条件
EPRO_SUIT_PROPERTY,				//套装属性
EPRO_EQUIPT_POWER,				//请求装备战斗力

//2014.2.20  Add
EPRO_EVERYDAY_ALLQIDAO,		//每日VIP获得的祈祷消息
EPRO_EVERYDAY_ADDQIDAO,		//添加相关的祈祷消息

EPRO_GET_YUANBAOGIFTINF,	//元宝礼包的信息
EPRO_BUY_ALLYUANBAOGIFT,		//一键购买元宝礼包中的装备

EPRO_BUY_PLAYERTP,		//玩家购买体力
//2014/4/14 add by ly
EPRO_CELL_COUNTDOWN,	//请求格子倒计时消息

//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------

DECLARE_MSG(SGroundItemTakeChance, SItemBaseMsg, SItemBaseMsg::EPRO_GROUNDITEM_TAKECHANCE)
struct SAGroundItemTakeChance : public SGroundItemTakeChance
{
	enum
	{
		SGITC_NOUSE,			// 不可用
		SGITC_USE,				// 可用
	};

	BYTE			m_Type;		// 类型
	DWORD			m_Index;	// 摇号ID
	SRawItemBuffer	m_Item;		// 道具
};

DECLARE_MSG(SGroundItemChanceBack, SItemBaseMsg, SItemBaseMsg::EPRO_GROUNDITEM_CHANCEBACK)
struct SQGroundItemChanceBack : public SGroundItemChanceBack
{
	enum
	{
		SQGICB_GO,				// 开始摇号
		SQGICB_GIVEUP,			// 放弃了
		SQGICB_GIVEUPALL,		// 全部放弃
	};

	BYTE	m_Choice;			// 玩家选择
	DWORD	m_Index;			// 摇号ID
};

struct SAGroundItemChanceBack : public SGroundItemChanceBack
{
	enum
	{
		SAGICB_GO,				// 摇号结果
		SAGICB_GIVEUP,			// 放弃结果
		SAGICB_TIMEOUT,			// 超时
		SAGICB_ITEM_GONE,		// 包裹消失
	};

	BYTE	m_Choice;			// 玩家选择
	WORD	m_Result;			// 结果
	DWORD	m_Index;			// 摇号ID
};

// 侠义道3回购系统
struct SaledItemType
{
	SPackageItem sItem;					// 道具
	DWORD		 dwValue;				// 价格
};

DECLARE_MSG(SSaledItemList, SItemBaseMsg, SItemBaseMsg::EPRO_SALEDITEM_LIST)
struct SQSaledItemList : public SSaledItemList
{
	DWORD dwSerial;
};

struct SASaledItemList : public SSaledItemList
{
	BYTE bCount;						// 个数
	SaledItemType iSaledList[CONST_BUYBACK_COUNT];
};

DECLARE_MSG(SBuySaledItem, SItemBaseMsg, SItemBaseMsg::EPRO_BUY_SALEDITEM)
struct SQBuySaledItem : public SBuySaledItem
{
	BYTE bIndex;			// 要回购的索引，0xff表示全购
};

struct SABuySaledItem : public SBuySaledItem
{
	BYTE bIndex;			// 成功回购道具，0xff表示全购
};

DECLARE_MSG(SLockItemCell, SItemBaseMsg, SItemBaseMsg::EPRO_LOCKITEM_CELL)
struct SALockItemCell : public SLockItemCell
{
	enum
	{
		SLC_PACKAGE,		// 玩家背包
		SLC_EQUIPCOLUMN,	// 装备栏
	};

	BYTE bTyte;
	bool bLock;				// 是否锁定
	WORD wPos;				// 格子位置
};

DECLARE_MSG(SStartForge, SItemBaseMsg, SItemBaseMsg::EPRO_START_FORGE)
struct SQStartForge : public SStartForge
{
	enum
	{
		ASF_NONE,			// Nothing
		ASF_MAKE_HOLE,		// 打孔
		ASF_INLAY_JEWEL,	// 镶嵌宝石
		ASF_TAKEOUT_JEWEL,	// 取宝石
	};

	WORD wCellPos;
	BYTE bType;		// 干嘛？
};
struct SAStartForge : public SStartForge
{
	enum
	{
		SAS_SUCCESS,
		SAS_INLAY_NoGEM,	//不是插入宝石的协议发错了
		SAS_INLAY_Error,	//现在的状态时插入宝石，但是你发成其他的了
		SAS_INLAY_EQUIP,	//打动的装备为空
		SAS_INLAY_XY,		//背包发来的位置没有找到
		SAS_INLAY_XY_LOCK,	//背包发来的位置是锁定的
		SAS_NoFindGEM,		//没有找到背包里的道具ID在脚本里面
		SAS_NoGEMITEM,		//发来的格子坐标里面的道具不是宝石道具
	};
	BYTE Result;		// 干嘛？
};

/// 装备强化相关的外部选择
enum TStrengthenExternChoose
{	
	//EChooseNone = 0x0, //什么都没选
	EStrengthenChoose_LuckyCharm = 0x1,	//选中幸运符
	EStrengthenChoose_ProtectionCharacter= 0x2, //选中保护符
};

DECLARE_MSG(SDragMetrial, SItemBaseMsg, SItemBaseMsg::EPRO_DRAG_METRIAL)
struct SQDragMetrial : public SDragMetrial
{
	const static WORD msc_wMaxLen = 32; // [2012-9-17 11-38 gw: +目前最多包裹格子个数]
	//BYTE bX;		// 坐标
	//BYTE bY;		
	BYTE byPosNum;
	WORD waPos[msc_wMaxLen]; // x * PACKAGEWITH + y
};

// 通知锻造结束
DECLARE_MSG(SEndForge, SItemBaseMsg, SItemBaseMsg::EPRO_END_FORGE)
struct SQEndForge : public SEndForge
{
	enum
	{
		QEF_EQUIP_OFF,
		QEF_METRAIL_OFF,
		QEF_CLOSE,
	};

	BYTE bOps;
};

DECLARE_MSG(STakeOffJewel, SItemBaseMsg, SItemBaseMsg::EPRO_TAKEOFF_JEWEL)
struct SQTakeOffJewel : public STakeOffJewel
{
	BYTE pos;		// 取宝石的位子
	BYTE SelectType;// 0 没有选择 1 成功符 2 保护石
};

struct SATakeOffJewel : public STakeOffJewel
{
	enum
	{
		ATJ_SUCCESS,		// 成功
		ATJ_LACK_MONEY,		// 金钱不足
		ATJ_LACK_SPACE,		// 背包空间不足
		ATJ_LACK_EXP,       // 经验不足
		ATJ_CLOSED,         // 遭封洞了
		ATJ_NoFindHole,		// 没有找到镶嵌好宝石的洞，所以就不存在摘除这会事情
		ATJ_protocol_ERROR,	// 0 没有选择 1 成功符 2 保护石
		ATJ_Metrial_ERROR,	// 材料不足
	};	

	BYTE result;
	BYTE pos;
};

DECLARE_MSG(SInLayJewel, SItemBaseMsg, SItemBaseMsg::EPRO_INLAY_JEWEL)
struct SQInLayJewel : public SInLayJewel
{
};

struct SAInLayJewel : public SInLayJewel
{
	enum
	{
		ALJ_SUCCESS,		// 镶嵌成功
		ALJ_LOCK_MONEY,     // 没有金币
		ALJ_LOCK_TOOL,      // 没有宝石
	};

	BYTE result : 3;
	BYTE index	: 5;
	BYTE type;
	BYTE value;
};

// 打孔
DECLARE_MSG(SMakeHole, SItemBaseMsg, SItemBaseMsg::EPRO_MAKE_HOLE)
struct SQMakeHole : public SMakeHole
{
	//BYTE SelectType;//0 没有选择 1 成功符 2 保护石
	byte byStrengthenExternChoose; // 是否选中幸运符保护符等外部状态，参考TStrengthenExternChoose	
};

// 打孔的结果
struct SAMakeHole : public SMakeHole
{
	enum
	{
		AMH_SUCCESS,		// 打孔成功
		AMH_FAILED,			// 打孔失败
		AMH_LACK_MONEY,		// 货币不足
		AMH_LACK_TOOL,		// 材料不足
		AMH_ALL_OPENED,		// 孔已全开
		AMH_MISMATCH,       // 原材料不匹配
		AMH_PrctectMetrial,	// 保护材料不足
		AMH_FAILED_REDUCEHOLE, // 失败减孔了
	};
	
	BYTE result : 3;
	BYTE index	: 5;
};

DECLARE_MSG(SSplitItemInProgress, SItemBaseMsg, SItemBaseMsg::EPRO_SPLITITEM_INPROGRESS)
struct SASplitItemInProgress : public SSplitItemInProgress
{
	enum
	{
		SIIP_START = 1,
		SIIP_END,
	};

	DWORD	dwGlobalID;
	BYTE	SplitStatus;
};

DECLARE_MSG(SUpdateItemAttribute, SItemBaseMsg, SItemBaseMsg::EPRO_UPDATEITEM_ATTRIBUTE)
struct SAUpdateItemAttribute : public SUpdateItemAttribute
{
	DWORD	dwGlobalID;
	DWORD	wIndex;
	WORD	wCellPos;	//坐标
	BYTE	bFlag;
};

// 激活玩家的扩展背包
DECLARE_MSG(SActiveExtPackage, SItemBaseMsg, SItemBaseMsg::EPRO_ACTIVEEXTPACKAGE)
struct SQActiveExtPackage : public SActiveExtPackage
{
	DWORD	dwGlobalID;						// 玩家的GID
	WORD	wWillActiveNum;							// 要激活的背包格子数量
};

struct SAActiveExtPackage : public SActiveExtPackage
{
	enum
	{
		EAEP_SUCCESS,						// 激活成功
		EAEP_LACK_ITEM_YUANBAO,					// 物品或元宝不足
		EAEP_NUM_ERROR,				// 激活的格子数不对		
	};

	WORD	wResult;						// 激活结局
	WORD	wExtendActivedCount;			// 所有被激活的扩展数量
	DWORD	dwRemainYuanBao;				// 剩余元宝
};

//=============================================================================================
// 更新玩家身上的临时物品
DECLARE_MSG( SUpdateTempItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_UPDATE_TEMPITEM )
struct SAUpdateTempItemMsg : public SUpdateTempItemMsg
{
	DWORD	dwGlobalID;		// 本次运行的全区唯一标识符
	WORD	itemlist[10];  // 一个玩家最多同时带10个临时物品
};
//=============================================================================================
// 整理背包
DECLARE_MSG( SQNeatenMsgBASE, SItemBaseMsg, SItemBaseMsg::EPRO_NEATEN )
struct SQNeatenMsg : public SQNeatenMsgBASE
{
	DWORD	dwGlobalID;		// 玩家ID
};

struct SANeatenMsg : public SQNeatenMsgBASE
{
	BYTE	m_Result;		// 整理背包成功与否（1为成功，0为失败）
};

//=============================================================================================
// 大转盘
DECLARE_MSG(SDialMsgBASE, SItemBaseMsg, SItemBaseMsg::EPRO_DIAL)
struct SQDialMsg : public SDialMsgBASE
{
    enum OP_TYPE
    {
        DIALP_START = 1,
        DIALP_TRY,
        DIALP_RECEIVE,
    };

    BYTE type;
    BYTE byDialType;
};

struct SADialMsg : public SDialMsgBASE
{
    enum DIAL_TYPE   // 普通，黄金，锦囊，元宝.....
    {
        DIALT_NORMAL,       // 普通的
        DIALT_GOLD,         // 黄金
        DIALT_TIPS,         // 锦囊
        DIALT_SECRETTIPS,   // 天机锦囊
        DIALT_YUANBAO,      // 元宝
        DIALT_LUBANSUO,     // 鲁班锁
        DIALT_KONGMINGSUO,  // 孔明锁

        //......
        DIALT_MAX,  
    };

    enum OP_TYPE
    {
        DIAL_SHOW = 1,
        DIAL_SHOWAWARD,
        DIAL_RUN,
        DIAL_COLSE,
    };

    BYTE index;
    BYTE type;
    BYTE buffer[1];
};

// 百宝商城
DECLARE_MSG(STreasureShopMsg, SItemBaseMsg, SItemBaseMsg::EPRO_TREASURE_SHOP)
struct SQTreasureShopMsg : public STreasureShopMsg
{
	enum TYPE
	{
		TYPE_BUY,
		TYPE_MONEY,
		TYPE_OPEN,
		TYPE_UPDATE
	};

	BYTE type_;
    WORD itemID;
	BYTE type[2];
	WORD num;
};

struct SATreasureShopMsg : public STreasureShopMsg
{
	BYTE type_;
	DWORD riches[3];  // 元宝；银两；赠宝 数量
};

// 请求更新百宝内道具
struct SQUpdateItemTreasureShopMsg : public STreasureShopMsg
{
	BYTE type_;
	BYTE chargeItemType; // 付费类型: 1111 商品类型 1111
};

// 更新百宝内道具 内部结构：付费类型-商品类型-{ { 物品ID-现价-原价-数量} ... }
struct SAUpdateItemTreasureShopMsg : public STreasureShopMsg
{
	BYTE type_;
	BYTE chargeItemType;     // 付费类型: 1111 商品类型 1111
	BYTE shows;              // 111111 每一位表示每个专区是否有商品
	WORD itemNum;            // 商品数量
	BYTE buffer[1];
};

// 请求打开百宝阁消息
struct SAOpenTreasureShopMsg : public STreasureShopMsg
{
	BYTE type_;
};

// 道具/骑乘读条消息
DECLARE_MSG(SPreUseItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_PRE_USE_ITEM)
struct SAPreUseItemMsg : public SPreUseItemMsg
{
	enum OperateType
	{
		OT_USE_NORMAL_ITEM,		// 开始使用道具读条
		OT_RIDE,				// 开始骑马读条
		OT_USE_ITEM_CANCEL,		// 取消使用道具读条
		OT_RIDE_CANCEL,			// 取消骑马读条
	};

	DWORD	time;
	BYTE	operateType;
};

// 使用道具
DECLARE_MSG(SUseItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_USE_ITEM)
struct SQUseItemMsg : public SUseItemMsg
{
	WORD wCellPos;
};

struct SAUseItemMsg : public SUseItemMsg
{
    enum EUseGoods 
    { 
        UG_NORMAL = 1 , 
        UG_BIJOU, 
        UG_GESTBOOK, 
		UG_ORDERSKILL,
        UG_ANTIDOTE 
    };

	SPackageItem stUseItem;
	BYTE byWhat;                  //   用这个来传装备变化的数据               
	SSkill stSkill;
};

DECLARE_MSG(SAutoUseItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_AUTOUSE_ITEM)
struct SQAutoUseItemMsg : public SAutoUseItemMsg
{
	// SAutoUseItemSet sSetHP, sSetMP, sSetSP;
    BOOL bSetQXD;    //新加的气血丹设置
    BOOL bSetLZ;     // 灵珠
    BYTE type;
};

DECLARE_MSG(SAutoFightMsg, SItemBaseMsg, SItemBaseMsg::EPRO_AUTOFIGHT)
struct SQAutoFightMsg : public SAutoFightMsg
{
    // 客户端通知服务器自动战斗状态
	DWORD flags;
};

// 丢掉道具
DECLARE_MSG(SDropItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_DROP_ITEM)
struct SQDropItemMsg : public SDropItemMsg
{
	SPackageItem stItem;
};

// 捡到道具
DECLARE_MSG(SPickupItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_PICKUP_ITEM)
struct SQPickupItemMsg : public SPickupItemMsg
{
	DWORD	dwGlobalID;		// 道具GID
	WORD	wIndex;			// 拾取索引，0xffff表示全部拾取，其余表示要拾取的索引
};

// 增加物品栏道具
DECLARE_MSG(SAddPackageItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_ADDPACKAGE_ITEM)
struct SAAddPackageItemMsg : public SAddPackageItemMsg
{
    SPackageItem stItem; 
};

DECLARE_MSG(SAddWarehouseItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_ADDWAREHOUSE_ITEM)
struct SAAddWarehouseItemMsg : public SAddWarehouseItemMsg
{
    SPackageItem stItem;
};


// ----- 暂时使用，为了测试方便 ----------------------------------------
struct SQAddPackageItemMsg : public SAddPackageItemMsg
{
    SPackageItem stItem; 
	BYTE byNum;
};

// 删除物品栏道具
DECLARE_MSG(SDelPackageItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_DELPACKAGE_ITEM)
struct SQDelPackageItemMsg : public SDelPackageItemMsg
{
	WORD DelItemNum;
	DWORD ItemID;
};
struct SADelPackageItemMsg : public SDelPackageItemMsg
{
    SPackageItem stItem;
};

// 装备孔的信息
DECLARE_MSG(SEquipInfoMsg, SItemBaseMsg, SItemBaseMsg::RPRO_EQUIP_INFO)
struct SAEquipInfoMsg : public SEquipInfoMsg
{
	SEquipment	stEquip;
	BYTE		byPos;
};

// 增加地表道具
DECLARE_MSG(SAddGroundItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_ADDGROUND_ITEM)
struct SAAddGroundItemMsg : public SAddGroundItemMsg
{
	DWORD	dwGlobalID;
	DWORD   ItemID;
	WORD		ItemNum;
	WORD	PackageMode;
	WORD	wTileX;
	WORD	wTileY;
};

// 更新地面物品，通知客户端某个索引的物品已拾取
DECLARE_MSG(SUpdateGroundItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_UPDATEGROUND_ITEM)
struct SAUpdateGroundItemMsg : public SUpdateGroundItemMsg
{
	DWORD	dwGlobalID;		// 道具ID
	WORD	wIndex;			// 索引
};

// 查看地面包裹
DECLARE_MSG(SCheckGroundItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_CHECKGROUND_ITEM)
struct SQCheckGroundItemMsg : public SCheckGroundItemMsg
{
	DWORD dwItemGID;		// 包裹ID
};

struct SACheckGroundItemMsg : public SCheckGroundItemMsg
{
	struct GroundItemWS
	{
		SRawItemBuffer item;
		BYTE		   status;
	};

	DWORD			dwItemGID;		// 包裹ID，为0表示不可查看
	DWORD			dwMoney;		// 货币数量
	WORD			wMoneyType;		// 货币类型
	WORD			wItemsInPack;	// 道具个数
	GroundItemWS	m_GroundItem[g_PackageMaxItem];	// 道具数据
	
	WORD MySize()
	{
		return sizeof(SACheckGroundItemMsg) - ((g_PackageMaxItem-wItemsInPack) * sizeof(SRawItemBuffer));
	}
};

// 取消查看地面包裹
DECLARE_MSG(SCloseGroundItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_CLOSEGROUND_ITEM)
struct SQCloseGroundItemMsg : public SCloseGroundItemMsg
{
};

// 移动物品栏道具
DECLARE_MSG(SMoveItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_MOVE_ITEM)
struct SQMoveItemMsg : public SMoveItemMsg
{
	WORD wCellPos1; //启动位置。如果是卸装则表示在对应位置索引
	WORD wCellPos2;
	BYTE byMoveTo;
	BYTE byindex;	//侠客索引
};

struct SAMoveItemMsg : public SMoveItemMsg
{
	WORD	wIndex;
	WORD	wWhat;
	BYTE	byIsChangeWeapon;
};

// 装备一个外表可见道具
DECLARE_MSG(SEquipItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_EQUIP_ITEM)
struct SAEquipItemMsg : public SEquipItemMsg
{
	DWORD	dwGlobalID;
    BYTE	byEquipPos;
	DWORD	wItemIndex;
};

// 在装备栏点击右键卸装
DECLARE_MSG(SUnEquipItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_UNLOAD_EQUIP)
struct SQUnEquipItemMsg : public SUnEquipItemMsg
{
	DWORD	dwGlobalID;
	BYTE	byEquipPos;
};

// 请求获取道具的信息
DECLARE_MSG(SRequirItemInfoMsg, SItemBaseMsg, SItemBaseMsg::EPRO_REQUIRE_ITEMINFO)
struct SQRequireItemInfoMsg : public SRequirItemInfoMsg
{
	enum ItemLocation
	{
		IL_PACKAGE = 1,			// 背包
		IL_EQUIP,				// 装备栏
	};
	DWORD   dwGlobalID;			// 请求的玩家
	DWORD   dwDesID;			// 被请求的玩家
	WORD	wItem;				// 物品ID
	BYTE	wLocation;			// 物品的位置
	BYTE	wX;					// 坐标，（如果在装备栏，则仅wX有效，表示装备孔信息）
	BYTE	wY;					//
};

struct SARequireItemInfoMsg : public SRequirItemInfoMsg
{
	SRawItemBuffer iteminfo;	// 如wIndex为0表示没查到
	// 其他信息以后添加
};

//=============================================================================================
// 更换外装颜色
DECLARE_MSG(SEquipColorMsg, SItemBaseMsg, SItemBaseMsg::EPRO_CHANGE_EQUIPCOL)
struct SQAEquipColorMsg : public SEquipColorMsg 
{
    DWORD	dwGlobalID;			// 本次运行全局唯一标识符
    BYTE    byEquipID;          // 装束ID 0-3 （衣服、内衣、腰带、鞋子）；4-7 清除掉相应的颜色
    WORD    wEquipCol;          // 装束要改的颜色
};
DECLARE_MSG(SQuestFollowItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_FOLLOWITEM_QuestData)
struct SQQuestFollowItemMsg : public SQuestFollowItemMsg
{
	WORD m_Index;				//1(药品类) 2(杂货类) ,3(强化类)
};
struct FollowShopS
{
	WORD	m_Index;				//1(药品类) 2(杂货类) ,3(强化类)
	WORD	m_Num;					//道具列表个数
	DWORD   m_Itemlist[120];			//道具列表
	char    m_name[8][10];
	WORD    m_NameNum;
	FollowShopS():m_Index(0),m_Num(0),m_NameNum(0)
	{
		memset(&m_Itemlist[0],0,sizeof(DWORD) * 120);
		memset(&m_name[0][0],0,sizeof(m_name));
	}
};
struct SAQuestFollowItemMsg : public SQuestFollowItemMsg
{
	FollowShopS m_Follow;
};

DECLARE_MSG(SBUYFollowItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_BUYITEM_FOLLOW)
struct SQBUYFollowItemMsg : public SBUYFollowItemMsg
{
	WORD  m_Index;				//1(药品类) 2(杂货类) ,3(强化类)
	DWORD m_ItemID;				//购买道具的编号(实体ID)
	DWORD m_Num;				//购买道具数量
};
DECLARE_MSG(SBlessItemReQuestMsg, SItemBaseMsg, SItemBaseMsg::EPRO_Bless_ReQuest)
struct SQBlessItemReQuestMsg : public SBlessItemReQuestMsg
{
	//这里需要客户端发消息不需要做任何事情
	BYTE Flag;					//0为免费其他为不免费
	BYTE QuickFlag;				//是否快速祈福
};
struct SABlessItemReQuestMsg : public SBlessItemReQuestMsg
{
	BYTE Flag;					//0为免费其他为不免费
	BYTE QuickFlag;				//是否快速祈福
	DWORD m_ItemID;				//得到的道具序号
	SPackageItem m_ItemIDVec[12];			//最后一个是服务器已经随机出来的ID 就是这次抽奖的结果ID
};
DECLARE_MSG(SBlessExcellenceDataMsg, SItemBaseMsg, SItemBaseMsg::EPRO_Bless_ExcellenceData)
struct SQBlessExcellenceDataMsg : public SBlessExcellenceDataMsg
{
	//这里需要客户端发消息不需要做任何事情 请求数据
};
struct ExcellenceData
{
	char Name[30];
	SPackageItem Item;
	ExcellenceData()
	{
		memset(Name,0,30);
		//memset(ItemIDName,0,30);
	}
};
struct SABlessExcellenceDataMsg : public SBlessExcellenceDataMsg
{
	//申请当前获得优秀道具的列表最大10行
	BYTE m_BlessFlag;						//0 可以使用免费次数 1表示不能使用免费次数
	BYTE m_Num;						//消息个数
	ExcellenceData m_ExData[10];	//消息数组
};

DECLARE_MSG(SBlessAllServerNotice, SItemBaseMsg, SItemBaseMsg::EPRO_Bless_AllServerNotice)
struct SABlessAllServerNoticeMsg : public SBlessAllServerNotice
{
	ExcellenceData m_Notice;
	SABlessAllServerNoticeMsg()
	{
		
	}
};
DECLARE_MSG(SBlessStopBless, SItemBaseMsg, SItemBaseMsg::EPRO_Bless_StopBless)
struct SABlessStopBless : public SBlessStopBless
{
	BYTE BlessStoptype;//祈福中断原因 1 钱不够 2 背包满了 3时序出错
};
DECLARE_MSG(SBlessQuestAddItems, SItemBaseMsg, SItemBaseMsg::EPRO_Bless_QuestAddItems)
struct SQBlessQuestAddItems : public SBlessQuestAddItems
{
	//请求添加道具
};
struct SABlessQuestAddItems : public SBlessQuestAddItems
{
	//请求添加道具
	SPackageItem Item;
};
//EPRO_Bless_ReQuest,							// 祈福请求
//EPRO_Bless_ExcellenceData,					// 祈福得到的需要全局通告的史诗数据
//EPRO_Bless_AllServerNotice					// 祈福获得道具全局通告

DECLARE_MSG_MAP(SSaleItemBaseMsg, SItemBaseMsg, SItemBaseMsg::RPRO_SALEITEM_MESSAGE)
RPRO_MOVETO_ITEM,								// 移动买卖道具
EPRO_ADDSALE_ITEM,								// 添加道具到买卖框
EPRO_DELSALE_ITEM,								// 删除买卖框道具
EPRO_SEESALE_ITEM,								// 察看买卖框
EPRO_BUYSALE_ITEM,								// 交易买卖框的道具
EPRO_CLOSESALE_ITEM,							// 收摊
EPRO_UPDATESALE_ITEM,							// 刷新
EPRO_SEND_SALEINFO,								// 发送摆摊信息
EPRO_CHANGE_PRICE,								// 改价
EPRO_CHANGE_NAME,								// 修改摊位名
EPRO_CHANGE_INFO,								// 修改说明
EPRO_SEND_MESSAGE,								// 发送留言
EPRO_SEND_ALL_MESSAGE,							// 发送所有可能的留言
EPRO_MONEY_IN,									// 本次收益
END_MSG_MAP() 

// 上下架的统一消息，对于下架，dMoney无意义，不传递
DECLARE_MSG(SMovetoSaleBoxMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::RPRO_MOVETO_ITEM)
struct SQMovetoSaleBoxMsg : public SMovetoSaleBoxMsg
{
    enum 
	{
		MP_SALEBOX,
        MP_PACKAGE
    };

    BYTE	byMoveto;		// 移动方向
	WORD	wCellPos;		// 物品的位置，对于MP_PACKAGE，仅用它表示铺面中的位置	
	DWORD	wIndex;			// ID，用于验证
	DWORD	dMoney;			// 物品的单价

	size_t MySize()
	{
		return sizeof(SQMovetoSaleBoxMsg) - (MP_PACKAGE == byMoveto) ? (sizeof(DWORD)) : 0;
	}
};

// 上架的回复
DECLARE_MSG(SAddSaleItemMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_ADDSALE_ITEM)
struct SAAddSaleItemMsg : public SAddSaleItemMsg
{
	BYTE			bPos;		// 位置
	DWORD			dMoney;		// 单价
	SPackageItem	item;		// 道具
};

// 下架的回复
DECLARE_MSG(SDelSaleItemMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_DELSALE_ITEM)
struct SADelSaleItemMsg : public SDelSaleItemMsg
{
	BYTE	bPos;				// 要下架的位置
};

// 改价消息
DECLARE_MSG(SChangeItemPriceMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_CHANGE_PRICE)
struct SQChangeItemPriceMsg : public SChangeItemPriceMsg
{
	BYTE	bPos;				// 要改价的物品位置
	DWORD	newPrice;			// 新的
};

// 改摊位名
DECLARE_MSG(SChangeStallNameMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_CHANGE_NAME)
struct SQChangeStallNameMsg : public SChangeStallNameMsg
{
	char	szStallName[MAX_STALLNAME];		// 摊位名
};

struct SAChangeStallNameMsg : public SChangeStallNameMsg
{
	DWORD	dwGlobalID;						// 唯一ID
	char	szStallName[MAX_STALLNAME];		// 摊位名
};

// 改留言
DECLARE_MSG(SChangeStallInfoMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_CHANGE_INFO)
struct SQChangeStallInfoMsg : public SChangeStallInfoMsg
{
	char	szStallInfo[MAX_SALEINFO];
};

struct SAChangeStallInfoMsg : public SChangeStallInfoMsg
{
	DWORD	dwGlobalID;						// 唯一ID
	char	szStallInfo[MAX_SALEINFO];
};

// 发送留言
DECLARE_MSG(SSendMessageMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_SEND_MESSAGE)
struct SQSendMessageMsg : public SSendMessageMsg
{
	DWORD	dwGlobal;						// 用于区分是玩家自己还是查看着
	char	szMsg[MAX_LOGSIZE];
};

struct SASendMessageMsg : public SSendMessageMsg
{
	StallLogInfo	log;					// 回传，同步
};

// 在玩家点开摆摊面板后，发送所有可能的消息
DECLARE_MSG(SSendAllMessageMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_SEND_ALL_MESSAGE)
struct SASendAllMessageMsg : public SSendAllMessageMsg
{
	BYTE			bNumber;						// 有几条消息
	StallLogInfo	logs[MAX_LOGLINES];				// 消息总数

	size_t MySize()
	{
		return sizeof(SASendAllMessageMsg) - ((MAX_LOGLINES-bNumber)*(sizeof(LogInfo)));
	}
};

// 货品更新恢复，改价/数量改变等
DECLARE_MSG(SUpdateSaleItemMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_UPDATESALE_ITEM)
struct SAUpdateSaleItemMsg : public SUpdateSaleItemMsg
{
    SaleItem   stSaleItem;
};

// 收入
DECLARE_MSG(SMoneyInMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_MONEY_IN)
struct SAMoneyInMsg : public SMoneyInMsg
{
	DWORD dwMoneyIn;
};

// 玩家查看店面
DECLARE_MSG(SSeeSaleItemMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_SEESALE_ITEM)
struct SQSeeSaleItemMsg : public SSeeSaleItemMsg
{
	enum
	{
		SSI_ITEM,
	};

	BYTE			bType;
    SALE_BOTHSIDES	stBothSides;
};

struct SASeeSaleItemMsg : public SSeeSaleItemMsg
{
	SaleSeeItemInfo stItemBaseInfo;

	size_t MySize()
	{
		return sizeof(SASeeSaleItemMsg) - (MAX_SALEITEMNUM-stItemBaseInfo.ItemNumber)*sizeof(SaleItem);
	}
};

// 收摊/关闭面板消息
DECLARE_MSG(SCloseSaleItemMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_CLOSESALE_ITEM)
struct SQCloseSaleItemMsg : public SCloseSaleItemMsg
{
	enum
	{
		CS_SELF,			// 玩家自己收摊
		CS_SELF_NOBACK,		// 不下架
		CS_BUYER,			// 查看着关闭
	};

	BYTE bOperation;
};

struct SACloseSaleItemMsg : public SCloseSaleItemMsg
{
	enum
	{
		CSI_CLOSESEE,		// 关闭查看面板
		CSI_CLOSESTALL,		// 收摊
	};

	DWORD dwGID;			// 摆摊者
	BYTE  bOperation;		// 操作
};

// 开始摆摊
DECLARE_MSG(SSendSaleInfoMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_SEND_SALEINFO)
struct SQSendSaleInfoMsg : public SSendSaleInfoMsg
{
    char szStallName[MAX_STALLNAME];		// 摊位名
	char szStallInfo[MAX_SALEINFO];			// 摊位说明
};

// 给自己的摆摊回馈
struct SASendSaleInfoMsg : public SSendSaleInfoMsg
{
	BYTE InfoLen;
    char szSaleInfo[MAX_STALLNAME];			

    int GetMySize()
    {
		InfoLen = dwt::strlen(szSaleInfo, MAX_STALLNAME-1);
		return sizeof(SASendSaleInfoMsg) - MAX_STALLNAME + InfoLen + 1;
    }
};

DECLARE_MSG(SBuySaleItemMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_BUYSALE_ITEM)
struct SQBuySaleItemMsg : public SBuySaleItemMsg
{
    SALE_BUYITEM stBuyItem;
};

// 道具交易
DECLARE_MSG(SBuyMsg, SItemBaseMsg, SItemBaseMsg::EPRO_BUY_MESSAGE)
struct SBuy : SBuyMsg
{
	WORD	nCount;		// 买卖数量
	BYTE	nPage;		// 物品分类/页面
	WORD	index;		// 物品的索引
	bool	bBuy;		// 买：true 卖：false
	long    Flag;       // 默认0 走NPC 1走随身商店
	SBuy():nCount(0),nPage(0),index(0),bBuy(0),Flag(0)
	{

	}
};

struct SABuy : SBuyMsg
{
	SABuy() : bShowMessage(1) {};
    BYTE	nRet;			// 0：钱不够 1：成功
	bool	bShowMessage;	
};

//=============================================================================================
// 一个交易对象提交了一个交易操作（确认、取消。。。）
DECLARE_MSG(SEquipmentListMsg, SItemBaseMsg, SItemBaseMsg::RPRO_EQUIPMENTINFO)
struct SAEquipmentListMsg : 
    SEquipmentListMsg
{
    //DWORD dwGID; 
    //SEquipment Equip[16];
	//char szTitle[CONST_USERNAME];           // 称号
	//WORD wkillValue;            // 杀孽值 PK值
	//char szFaction[CONST_USERNAME];         // 帮派
	//char szSpouse[CONST_USERNAME];          // 配偶

    //char szName[CONST_USERNAME];            // 姓名	OK
    //short nValue;               // 侠义值
    //DWORD dwExtraState;         // 状态
    //WORD wMedicalEthics;        // 医德值
    //int  nState;                // 站立还是打坐
    //BYTE bySex:1;               // 玩家角色性别
    //BYTE byBRON:3;              // 玩家所显示图片级别，==1表示出师
    //BYTE bySchool:4;            // 玩家所属门派OK
    //WORD wWeaponIndex;
    //WORD wEquipIndex;		    // 因为装备需要显示
    //WORD wMutateID;             // 玩家当前变身的编号
    //WORD wOtherLevel;           // 玩家等级OK

    //// [新扩展的功能]脚本生成的个人信息查看地址！
    ////BYTE newMark;   // 新消息扩展标记！！！
    ////BYTE infoLength;
    ////char infoString[256];
	
	char		szName[CONST_USERNAME];				// 姓名	OK
	WORD		wOtherLevel;			// 玩家等级OK
	SEquipment	Equip[16];

	DWORD		dwGlobal;				// GID
	BYTE		bFaceID;				// 脸模型
	BYTE		bHairID;				// 头发模型
	DWORD		m_3DEquipID[11];			// 3D装备挂件物品号

	float		m_fPositionX;			// 位置x
	float		m_fPositionY;			// 位置y
	float		m_fDirection;			// 方向

	BYTE		bySex:1;			    // 玩家角色性别
	BYTE		byBRON:3;				// 玩家所显示图片级别，==1表示出师
	BYTE		bySchool:4;				// 玩家所属门派OK

	DWORD		m_OtherMaxHP;
	DWORD		m_OtherMaxMP;
	DWORD		m_OtherMaxTP;
	DWORD		m_OtherHP;	
	DWORD		m_OtherMP;
	DWORD		m_OtherTP;
	WORD		m_OtherGongJi;			// 攻击力
	WORD		m_OtherFangYu;			// 防御力
	WORD		m_OtherBaoJi;			// 暴击
	WORD		m_OtherShanBi;			// 闪避
	float		m_OtherAtkSpeed;		// 攻击速度
	float		m_OtherMoveSpeed;		// 移动速度

	WORD		m_OtherGongjiPt;		//攻击点数
	WORD		m_OtherFangyuPt;		//防御点数
	WORD		m_OtherQinggongPt;		//轻功点数
	WORD		m_OtherQiangjianPt;		//强健点数

	QWORD		m_OtherCurExp;			//玩家当前等级的当前经验
	QWORD		m_OtherMaxExp;			//玩家当前等级的最大经验			

	WORD		m_Otherbingshang;		//冰伤
	WORD		m_Otherbingkang;		//冰抗	
	WORD		m_Otherhuoshang;		//火伤
	WORD		m_Otherhuokang;			//火抗
	WORD		m_Otherxuanshang;		//玄伤
	WORD		m_Otherxuankang;		//玄抗
	WORD		m_Otherdushang;			//毒伤
	WORD		m_Otherdukang;			//毒抗

	WORD		m_Othershanghaijianmian;//伤害减免	
	WORD		m_Otherjueduishanghai;	//绝对伤害
	WORD		m_Otherwushifangyu;		//无视防御
	WORD		m_Otherbaojibeishu;		//暴击倍数

	DWORD       m_OtherJP;				//玩家当前精力
	DWORD       m_OtherMaxJP;			//玩家最大精力

	WORD		m_OtherShaLu;			//杀戮值
	WORD		m_OtherXiaYi;			//侠义值
	WORD		m_XwzValue;			//修为值
};

// 仓库相关消息类
DECLARE_MSG_MAP(SWareHouseBaseMsg, SItemBaseMsg, SItemBaseMsg::RPRO_WAREHOUSE_BASE)
//{{AFX
RPRO_OPEN_ITEM_WAREHOUSE,					// 请求打开一个道具仓库
RPRO_CLOSE_WAREHOUSE,                       // 请求关闭一个仓库
RPRO_ITEM_MOVEIN,                           // 将一个道具（或者金钱）从身上移动到仓库
RPRO_ITEM_MOVEOUT,                          // 将一个道具（或者金钱）从仓库移动到身上
RPRO_ITEM_MOVESELF,                         // 将一个道具在仓库中移动
RPRO_ITEM_LOCKIT,							// 锁定
RPRO_ACTIVE_WAREHOUSE,						// 激活
RPRO_ARRANGE_WAREHOUSE,						// 整理仓库
//}}AFX
END_MSG_MAP()

// 客户端开始激活
DECLARE_MSG(SArrangeWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_ARRANGE_WAREHOUSE)
struct SQArrangeWareHouseMsg : SArrangeWareHouseMsg
{
};

// 客户端开始激活
DECLARE_MSG(SActiveWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_ACTIVE_WAREHOUSE)
struct SQActiveWareHouseMsg : SActiveWareHouseMsg
{
	DWORD	dwGlobal;
};

struct SAActiveWareHouseMsg : SActiveWareHouseMsg
{
	enum
	{
		AWH_SUCCESS,
		AWH_LACK_MONEY,
		AWH_ALL_ACTIVED,
	};

	BYTE	bResult;
	BYTE	bCurActiveTimes;
};

// 请求打开一个道具仓库
DECLARE_MSG(SOpenItemWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_OPEN_ITEM_WAREHOUSE)
struct SQOpenItemWareHouseMsg : SOpenItemWareHouseMsg
{
	BYTE	byDirectly;			// 是否直接进入，忽略密码（1表示忽略，2表示输入了密码）
	char	szUserpass[CONST_USERPASS];

	WORD MySize()
	{
		return sizeof(SQOpenItemWareHouseMsg) - (byDirectly ? (sizeof(char) * CONST_USERPASS) : 0);
	}
};

struct SAOpenItemWareHouseMsg : SOpenItemWareHouseMsg
{
	enum ERetCode
    {
        ERC_OK,                 // 成功
        ERC_ALREADY_OPEN,       // 仓库已经被打开过了
        ERC_SERVER_DISCARD,     // 本次操作被服务器否决
		ERC_NEEDPASSWORD,		// 需要打开二级密码输入面板
		ERC_PASSWORD_ERROR,		// 二级密码错误
    };

	BYTE			byRetCode;			// 操作反馈信息
    BYTE			byGoodsNumber;		// 本仓库道具的数量
	BYTE			m_ActiveTimes;		// 激活的次数
	DWORD			dwOperationSerial;	// 本次操作的序列号
    DWORD			nMoneyStoraged;		// 仓库中的金钱
    SPackageItem	GoodsArray[MAX_WAREHOUSE_ITEMNUMBER];
	DWORD			nBindMoney;	// 仓库中的绑定金钱
    WORD MySize()
    {
		if ((byGoodsNumber > MAX_WAREHOUSE_ITEMNUMBER) && (byRetCode == ERC_OK))  // 打开成功才需判断byGoodsNumber
        {
            byGoodsNumber = 0;
            byRetCode	  = ERC_SERVER_DISCARD;
        }

        return sizeof(SAOpenItemWareHouseMsg) - sizeof(SPackageItem)*(MAX_WAREHOUSE_ITEMNUMBER-byGoodsNumber);
    }
};

// 请求关闭一个仓库（通知服务器仓库使用完毕）
DECLARE_MSG(SCloseWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_CLOSE_WAREHOUSE)
struct SQCloseWareHouseMsg : SCloseWareHouseMsg
{
    DWORD	dwOperationSerial;	                    // 本次操作的序列号
};

struct SACloseWareHouseMsg : SCloseWareHouseMsg
{
	enum ERetCode
    {
        ERC_OK,                 // 成功
        ERC_SERVER_DISCARD,     // 本次操作被服务器否决
    };

	BYTE byRetCode;             // 操作反馈信息
};

// 请求锁定/解锁一个仓库
DECLARE_MSG(SLockWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_ITEM_LOCKIT)
struct SQLockWareHouseMsg : SLockWareHouseMsg
{
	char szUserpass[CONST_USERPASS];
	bool bLock;
};

struct SALockWareHouseMsg : SLockWareHouseMsg
{
	bool bLock;
	bool bSuccess;
};

// 将一个道具（或者金钱）从身上移动到仓库
DECLARE_MSG(SMoveItemInWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_ITEM_MOVEIN)
struct SQMoveItemInWareHouseMsg : SMoveItemInWareHouseMsg
{
    SCellPos Pocket;				// 口袋中需要移动的道具的位置
    SCellPos Store;					// 将要移动到仓库中的位置
    DWORD	 wIndex;				// 本次操作道具的编号（用以校验）
    DWORD	 dwOperationSerial;		// 本次操作的序列号
};

struct SAMoveItemInWareHouseMsg : SMoveItemInWareHouseMsg
{
	enum ERetCode
    {
        ERC_OK,						// 成功
        ERC_SET_MONEY,				// 存钱成功
        ERC_CANNOT_PUT,				// 无法将该道具放到目标点
        ERC_SERVER_DISCARD,			// 本次操作被服务器否决（参数错，意外的情况等）
    };

	BYTE	byRetCode;				// 操作反馈信息
    DWORD	nMoneyOfStorage;		// 仓库中非绑定的货币
	DWORD	nBindMoneyOfStorage;	// 仓库中绑定的货币
	SPackageItem Item;				// 仓库中新加的道具
};

// 将一个道具（或者金钱）从仓库移动到身上
DECLARE_MSG(SMoveItemOutWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_ITEM_MOVEOUT)
struct SQMoveItemOutWareHouseMsg : SMoveItemOutWareHouseMsg
{
    SCellPos Store;					// 将要移动的道具在仓库中的位置
    SCellPos Pocket;				// 移动到口袋中的目标位置
    DWORD	 wIndex;				// 本次操作道具的编号（用以校验）
    DWORD	 dwOperationSerial;		// 本次操作的序列号
};

struct SAMoveItemOutWareHouseMsg : SMoveItemOutWareHouseMsg
{
	enum ERetCode
    {
        ERC_OK,						// 成功
        ERC_SET_MONEY,				// 取钱成功
        ERC_CANNOT_PUT,				// 无法将该道具放到目标点
        ERC_SERVER_DISCARD,			// 本次操作被服务器否决（参数错，意外的情况等）
    };

	BYTE	byRetCode;				// 操作反馈信息
	DWORD	nMoneyOfStorage;		// 仓库中非绑定的货币
	DWORD	nBindMoneyOfStorage;	// 仓库中绑定的货币
	SPackageItem Item;				// 仓库中新加的道具
};

// 将一个道具在仓库中移动
DECLARE_MSG(SMoveSelfItemWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_ITEM_MOVESELF)
struct SQMoveSelfItemWareHouseMsg : SMoveSelfItemWareHouseMsg
{
    SCellPos StoreSrc;          // 将要移动的道具在仓库中的位置
    SCellPos StoreDest;         // 将要移动的仓库中的目标位置
    DWORD	 dwOperationSerial;	// 本次操作的序列号
};

// 本操作返回的消息还有另外一个：SAAddPackageItemMsg
struct SAMoveSelfItemWareHouseMsg : SMoveSelfItemWareHouseMsg
{
	enum ERetCode
    {
        ERC_OK,                 // 成功
        ERC_SERVER_DISCARD,     // 本次操作被服务器否决（参数错，意外的情况等）
    };

	BYTE byRetCode;             // 操作反馈信息
    SPackageItem srcItem;		// 源
	SPackageItem destItem;		// 目的
};

// 交换仓库内的两个道具
struct SQExChangeItemWareHouseMsg : SMoveSelfItemWareHouseMsg
{
    SCellPos SrcPos;			// 将要移动的道具在仓库中的位置
    SCellPos DestPos;			// 将要移动的仓库中的目标位置
    DWORD	 dwOperationSerial; // 本次操作的序列号
};

// 玩家间交易相关消息类
//=============================================================================================
DECLARE_MSG_MAP(SExchangeBaseMsg, SItemBaseMsg, SItemBaseMsg::EPRO_EXCHANGE_MESSAGE)
//{{AFX
RPRO_QUEST_EXCHANGE,                        // 请求和某个玩家进行交易（服务器通知客户端，客户端应答服务器）
RPRO_MOVE_EXCHANGE_ITEM,                    // 移动一个道具到交易栏（包括金钱）
RPRO_EXCHANGE_OPERATION,                    // 一个交易对象提交了一个交易操作（确认、取消。。。）
RPRO_CANCEL_QUEST,							// 取消了交易的请求
RPRO_QUEST_REMOTE_EXCHANGE,                 // 请求和某个远程玩家进行基于指定道具的交易【A -> S => S -> B => B -> S 】
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------

// 请求和某个玩家进行交易
DECLARE_MSG(SQuestRemoteExchangeMsg, SExchangeBaseMsg, SExchangeBaseMsg::RPRO_QUEST_REMOTE_EXCHANGE)
struct SQAQuestRemoteExchangeMsg : SQuestRemoteExchangeMsg
{
    DNID dnidClient;        // 校验数据
    DWORD dwSrcGID;         // 交易发起人的GID
    DWORD dwDestGID;        // 交易目标的GID
    bool bAccept;           // 是否同意交易       <<<< 前边这段主要是为了和以前的交易逻辑保持一致，以前有强制转换的。。。

    WORD item;              // 请求交易的原始道具ID[发起端有效]
    char name[CONST_USERNAME];          // 交易发起人请求的的目标玩家名字
	BYTE state;             // 为真：打开交易栏 假：打开交易请求
};

// 请求和某个玩家进行交易
DECLARE_MSG(SQuestExchangeMsg, SExchangeBaseMsg, SExchangeBaseMsg::RPRO_QUEST_EXCHANGE)

// 先是服务器通知客户端
struct SAQuestExchangeMsg : SQuestExchangeMsg
{
	DNID dnidClient;		// 校验数据
    DWORD dwSrcGID;			// 交易发起人的GID
    DWORD dwDestGID;		// 交易目标的GID
	bool  bAccept;			// 为真：打开交易栏 假：打开交易请求
};

// 然后是客户端应答服务器
struct SQQuestExchangeMsg : SQuestExchangeMsg
{
    DNID dnidClient;		// 校验数据
    DWORD dwSrcGID;			// 交易发起人的GID
    DWORD dwDestGID;		// 交易目标的GID
    bool bAccept;			// 是否同意交易
};

// 取消我的请求，老子不想和他交易了
DECLARE_MSG(SCancelQuestExchangeMsg, SExchangeBaseMsg, SExchangeBaseMsg::RPRO_CANCEL_QUEST)
struct SACancelQuestExchangeMsg : SCancelQuestExchangeMsg
{
	enum
	{
		CQE_DEAD,			// 因为翘辫子了
	};
	DWORD dwSrcID;			// 交易发起人
	BYTE  bResult;			// 取消的原因
};

// 移动一个道具到交易栏（包括金钱），暂时没有回传类消息，通过其他的消息来处理的
DECLARE_MSG(SPutItemToExchangeBoxMsg, SExchangeBaseMsg, SExchangeBaseMsg::RPRO_MOVE_EXCHANGE_ITEM)
struct SQPutItemToExchangeBoxMsg : SPutItemToExchangeBoxMsg
{
    DWORD dwSrcGID;			// 交易发起人的GID
    DWORD dwDestGID;		// 交易目标的GID

    SPackageItem Item;		// 放入的物品（可以是金钱）

	WORD  wCellX, wCellY;	// 目标位置
};

struct SAPutItemToExchangeBoxMsg : SPutItemToExchangeBoxMsg
{
    DWORD dwSrcGID;         // 交易发起人的GID
    DWORD dwDestGID;        // 交易目标的GID

    bool  isSrc;            // 是否是发起人执行的移动物品操作

    SPackageItem Item;      // 放入的物品（可以是金钱）

	WORD  wCellX, wCellY;   // 目标位置
};

// 一个交易对象提交了一个交易操作（确认、取消）
DECLARE_MSG(SExchangeOperationMsg, SExchangeBaseMsg, SExchangeBaseMsg::RPRO_EXCHANGE_OPERATION)
struct SQExchangeOperationMsg : SExchangeOperationMsg
{
    DWORD dwSrcGID;     // 交易发起人的GID
    DWORD dwDestGID;    // 交易目标的GID

    enum EOperation
    {
		LOCK,			// 锁定
		UNLOCK,			// 取消锁定     
		COMMIT,			// 提交
		UNCOMMIT,		// 取消提交
        CANCEL,			// 取消
    };

    EOperation operation;
};

struct SAExchangeOperationMsg : SExchangeOperationMsg
{
	DWORD dwSrcGID;     // 交易发起人的GID
    DWORD dwDestGID;    // 交易目标的GID
    bool  isSrc;        // 是否是发起人执行的操作

    enum EOperation
    {
		LOCK,			// 锁定
		UNLOCK,			// 取消锁定
        COMMIT,			// 提交
		UNCOMMIT,		// 取消提交
        CANCEL,			// 取消
        SUCCEED,
    };

    EOperation operation;
};

// 道具交易
DECLARE_MSG(SConsumePointMsg, SItemBaseMsg, SItemBaseMsg::EPRO_CONSUMEPOINT_UPDATE)
struct SAConsumePoint : SConsumePointMsg
{
	SAConsumePoint():dwConsumePoint(0){};
	DWORD dwConsumePoint;
};

// 请求拆分道具
DECLARE_MSG(SplitItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_SPLIT_ITEM)
struct QuerySplitItemMsg : SplitItemMsg
{
    SCellPos	src;
    DWORD		count;
};

//=============================================================================================
// 更新（获取）某项道具配置信息
DECLARE_MSG( UpdateItemDataMsg, SItemBaseMsg, SItemBaseMsg::EPRO_UPDATE_ITEMINFO )
struct QueryUpdateItemDataMsg : UpdateItemDataMsg
{
    // 客户端请求获取指定道具的配置数据，有效的道具ID空间为WORD[0~65535]
    enum ExtraQueryIndex
    {
        RESET = 65536,          // 请求获取需要排除重新更新的所有道具编号（返回一个道具序列，所有序列中的道具配置都将被删除，从而通过后续的请求重新获取）
        RANDMONTABLE = 65537    // 请求获取装备的随机属性列表（适用于道具列表中的extra_attribute）
    };

    DWORD index; 
    DWORD crc32;
};

struct AnswerUpdateItemDataMsg : UpdateItemDataMsg
{
    // 客户端请求获取指定道具的配置数据，有效的道具ID空间为WORD[0~65535]
    enum ExtraQueryIndex
    {
        RESET = 65536,          // 请求获取需要排除重新更新的所有道具编号（返回一个道具序列，所有序列中的道具配置都将被删除，从而通过后续的请求重新获取）
        RANDMONTABLE = 65537    // 请求获取装备的随机属性列表（适用于道具列表中的extra_attribute）
    };

    DWORD index; 

    char streamBuffer[10240];
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 播放使用间隔光效
DECLARE_MSG(SUseIntervalEffectMsg, SItemBaseMsg, SItemBaseMsg::EPRO_USEINTERVAL_EFFECT)
struct SAUseIntervalEffect : SUseIntervalEffectMsg
{
    // 间隔类型
    enum E_USEINTERVAL_TYPE
    {
        EUIT_HP,
        EUIT_MP,
        EUIT_SP,
        EUIT_HYDXG,      // 黑玉断续膏
        EUIT_XQDx,       // 血气丹小
        EUIT_XQDd,       // 血气丹大
        EUIT_XQDt,       // 血气丹特
        EUIT_ROSE,       // 玫瑰
        EUIT_LZ1,        // 灵珠1
        EUIT_LZ2,        // 灵珠2
        EUIT_LZ3,        // 灵珠3
        EUIT_LZ4,        // 灵珠4
        EUIT_LZ5,        // 灵珠5
        EUIT_LZ6,        // 灵珠6
        //............
        EUIT_MAX,
    }; 
	WORD type;
	DWORD time;         // 间隔时间
	WORD wLocSrvKindNumberForCD;		// 冷却时间用的物品类型
};
//////////////////////////////////////////////////////////////////////////
//技能
struct SSkillUseIntervalEffect : SUseIntervalEffectMsg
{
	// 间隔类型
	enum E_USEINTERVAL_TYPE
	{
		EUIT_SKILL1,
		EUIT_SKILL2,
		EUIT_SKILL3,
		EUIT_SKILL4,
		EUIT_SKILL5,
		EUIT_SKILL6,
		EUIT_SKILL7,
		EUIT_SKILL8,
		EUIT_SKILL9,
		EUIT_SKILL10,
		EUIT_SKILL11,
		EUIT_SKILL12,
		EUIT_SKILL13,
		EUIT_SKILL14,
		EUIT_SKILL15,
		EUIT_SKILL16,

		//............
		EUIT_MAX,
	};
	WORD type;          // 间隔类型
	DWORD time;         // 间隔时间
};

//---------------------------------------------------------------------------------------------


//=============================================================================================
// 游戏大厅金钱改变
DECLARE_MSG(_sc_SPlugInGameMoneyMsg, SItemBaseMsg, SItemBaseMsg::EPRO_PLUGINGAME_MONEY)
struct sc_PlugInGameMoneyMsg : public _sc_SPlugInGameMoneyMsg
{
    DWORD dwMoney[3]; // 元宝；赠宝； 银两；数量
};

//=================================================
// 装备耐久度改变
DECLARE_MSG(SEquipWearChange, SItemBaseMsg, SItemBaseMsg::EPRO_EQUIP_WEAR_CHANGE)
struct SAEquipWearChange : public SEquipWearChange
{
	EQUIP_POSITION equipPosition;
	WORD currWear;
};

// 单修一件装备
DECLARE_MSG(SEquipFixOne, SItemBaseMsg, SItemBaseMsg::EPRO_FIX_ONE_ITEM)
struct SQEquipFixOne : public SEquipFixOne
{
	enum EQUIP_FIXED_POS 
	{
		EFP_WEAR,
		EEP_BAG,
	};
	DWORD	index;
	BYTE	type;
	BYTE	pos;
};

// 修理所有装备
DECLARE_MSG(SEquipFixAll, SItemBaseMsg, SItemBaseMsg::EPRO_FIX_ALL_ITEM)
struct SQEquipFixAll : public SEquipFixAll
{
	
};

//	======装备精炼=====

// 拖入装备消息
DECLARE_MSG(SSAddEquip, SItemBaseMsg, SItemBaseMsg::EPRO_ADD_EQUIP)
struct SQAddEquip : public SSAddEquip
{
	enum
	{
		ADE_NONE,			     // 默认状态
		ADE_UPDATE_LEVEL = 1,	 // 装备升级
		ADE_REFINE_GRADE = 2,	 // 装备升阶
		ADE_UPDATE_QUALITY = 3,	 // 装备升品
		ADE_REFINE_STAR = 4,	 // 装备升星
		ADE_ReMove_STAR = 5,	 // 装备洗星		
		ADE_EQUIP_RESET = 6,	 // 装备洗练(重置附加属性条数)
		ADE_EQUIP_REFINE = 7,	 // 装备精炼(重置附加属性数值)		
		ADE_EQUIP_SHENGJIE=8,	 // 装备升阶（提升附加属性的上限）
		ADE_EQUIP_DECOMPOSITION = 9, // 装备分解
		ADE_ADD_HOLE = 10,		 // 装备打孔
		ADE_GEM_INSERT = 11,		 // 装备镶嵌宝石
		ADE_GEM_REMOVE = 12,	 // 装备宝石摘除	
		ADE_EQUIP_SMELTING = 13,	 // 装备熔炼
		ADE_EQUIP_SPIRITATTACHBODY = 14, // 装备灵附		

		ADE_Max,
	};
	static const WORD msc_wMaxLen = 12; // 批处理最大上限
	static const byte msc_byBatchDecomposition_Max = 8; // 批处理分解最大数量
	static const byte msc_byBatchSmelting_Max = 5; // 批处理熔炼最大数量	
	SQAddEquip()
	{	
		bType = 0;
		byPosNum = 0;
		byAssistPosNum = 0;
		memset(&waPos, 0, sizeof(waPos));
	}

	BYTE bType;		// 装备强化类型
	BYTE byPosNum; // 拖装备的数量	
	BYTE byAssistPosNum;  // 辅助位置的数量
	WORD waPos[msc_wMaxLen]; // x * PACKAGEWITH + y	, 先存放脱装备，再存放辅助位置（所以需要满足byPosNum+byAssistPosNum<=msc_wMaxLen）
};
struct SAAddEquip : public SSAddEquip
{
	enum
	{
		ADE_NONE,						// 默认状态
		ADE_Success = 1,
		ADE_EquipLevel_Less = 2,		// 等级不够
		ADE_NoEquip = 3,				// 不是装备类型
		ADE_GETNEXTEquip_Error = 4,		// 得到下一级装备失败
		ADE_SendNEEDITEM_Info_Error = 5,// 发送道具信息失败
		ADE_UPDATE_QUALITY_1 = 6,	// 装备升品质 金色装备不能显示
		ADE_UPDATE_QUALITY_2 = 7,	// 此装备不能升品质
		ADE_YINBING = 8,	// 升级品质的装备(银币)不够
		ADE_YINGLIANG = 9,	// 升级品质的装备(银两)不够
		ADE_UPDATE_NoCanLEVEL,	//此装备不能升等级
		ADE_OnceHaveEquip_Error,	// 已经有装备了，必须卸下才能重新装上
		ADE_REFINE_GRADE_ERROR,	// 装备升阶	级数错误
		ADE_No_Grade,			// 此道具不能升级
		ADE_REFINE_STAR_ERROR,	// 装备升星
		ADE_UPDATE_LEVEL_ERROR,	// 装备升级
		ADE_RESET_Color,		// 不能重置白色装备
		ADE_RFRESH_Color,		// 不能刷新白色装备
		ADE_NORESET,			// 不能鉴定
		ADE_NORFRESH,			// 不能刷新
		ADE_REFINE_STAR_FULL,	// 装备上的星星已经满了
		ADE_REFINE_STAR_Equip,	// 此装备不能升星
		ADE_REFINE_ReSTAR_Empty,// 装备没有升过星,所有不能洗星
		ADE_REFINE_ReSTAR_Equip,// 此装备不能洗星
		ADE_EQUIP_NoMakeHole,	// 此装备不能打孔
		ADE_EQUIP_MakeHoleFUll,	// 此装备孔满了
		ADE_EQUIP_MakeGemNoEmpty,	// 插入宝石的时候没有找到空的位置
		ADE_EQUIP_NoFindGem,		// 此装备没有已经镶嵌了宝石的地方
		ADE_EquipColor_Less,	// 颜色限制
		ADE_EQUIP_NOMakeDECOMPOSITION, // 装备不能分解
		ADE_EQUIP_NOMakeSmelting, // 装备熔炼
	};
	BYTE Flag;
};
// 材料需求的结构
struct MetrialItem
{		
	MetrialItem(DWORD dwParamID = 0, WORD wParamSuccessRate = 10000, BYTE byParamNum = 0, BYTE	byParamMetrialType = 0)
	{
		dwID = dwParamID; wSuccessRate = wParamSuccessRate; byNum = byParamNum; byMetrialType = byParamMetrialType;
	}
	DWORD	dwID; // 材料ID
	WORD	wSuccessRate; // 成功率
	BYTE	byNum; // 材料数量
	BYTE	byMetrialType;	// 材料类型	
};	
DECLARE_MSG(SSNeedMETRIAL, SItemBaseMsg, SItemBaseMsg::EPRO_STRENGTHEN_NeedMETRIAL)
struct SANeedMETRIAL : public SSNeedMETRIAL
{
	SANeedMETRIAL()
	{
		bType = 0;
		MoneyType = 0;
		NeedMoney = 0;
		wSuccessRate = 10000;
		byMetrialNum = 0;
		memset(&aMetrialItem, 0, sizeof(aMetrialItem));
	}
	static const int msc_iMetrialMaxLen = 10;

	enum TMETR 
	{
		EMETR_NEED_COMMON = 0, // 普通材料需求
		EMETR_NEED_PROTECTION, // 保护材料需求
		EMETR_NEED_LUCKYCHARM, // 幸运符需求
		//EMETR_NEED_MONEY,	 // 需要多少钱（此时结构中的dwID-表示钱的数量，byNum-表示）
		EMETR_PRODUCE_COMMON, // 产出普通材料
		EMETR_PRODUCE_SPECIAL, // 产出特殊材料
	};
	
	BYTE  bType;			// 装备强化类型 同(SQAddEquip 协议枚举类型)
	BYTE  MoneyType;		// 1是银币 2 是银两 钱类型
	DWORD NeedMoney;		// 需要的钱
	WORD  wSuccessRate;		// [2012-8-17 +成功概率(没有包含有幸运符时的总成功率)]

	BYTE  byMetrialNum; // 目前的数目 
	MetrialItem	aMetrialItem[msc_iMetrialMaxLen]; // 存储材料需求或者产出
};

// 拖入材料消息
DECLARE_MSG(SSAddMetrial, SItemBaseMsg, SItemBaseMsg::EPRO_ADD_METRIAL)
struct SQAddMetrial : public SSAddMetrial
{
	enum
	{
		ADM_RAW_METRIAL,			// 升阶原材料
		ADM_PROTECT_METRIAL,		// 升阶保护材料
		ADM_RATE_METRIAL,			// 概率材料
	};
	const static WORD msc_wMaxLen = 32; // [2012-9-17 11-38 gw: +目前最多包裹格子个数]

	//BYTE bX;		// 坐标
	//BYTE bY;
	BYTE bType;		// 材料类型	
	BYTE byPosNum; // 当前有效数量

	WORD waPos[msc_wMaxLen]; // 存储位置列表x * PACKAGEWITH + y
};
struct SAAddMetrial : public SSAddMetrial
{
	enum
	{
		ADD_SUCCESS,
		ADD_INLAY_NoGEM,	//不是插入宝石的协议发错了
		ADD_INLAY_Error,	//现在的状态时插入宝石，但是你发成其他的了
		ADD_INLAY_EQUIP,	//打动的装备为空
		ADD_INLAY_XY,		//背包发来的位置没有找到
		ADD_INLAY_XY_LOCK,	//背包发来的位置是锁定的
		ADD_NoFindGEM,		//没有找到背包里的道具ID在脚本里面
		ADD_NoGEMITEM,		//发来的格子坐标里面的道具不是宝石道具
		ADD_GemNoInsert,	//宝石不能插入
	};

	BYTE Result;
	WORD wCellPos; // 坐标
};
// 精炼--开始升阶消息
DECLARE_MSG(SSBeginRefineGrade, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_REFINE_GRADE)
struct SQBeginRefineGrade : public SSBeginRefineGrade
{
	//byte SelectType;// 0 没有选择 1 成功符 2 保护石
	byte byStrengthenExternChoose; // 是否选中幸运符保护符等外部状态，参考TStrengthenExternChoose	
};

struct SABeginRefineGrade : public SSBeginRefineGrade
{
	enum
	{
		BRG_SUCCESS,		// 升阶成功
		BRG_NOMONEY,		// 金钱不足
		BRG_FAIL,			// 升阶失败
		BRG_DESTROY,		// 升阶失败并销毁装备
		BRG_LACK_METRIAL,	// 缺少材料
		BRG_YINBING,		// 缺少银币
		BRG_YINGLIANG,		// 缺少银两
		BRG_GRADE_FULL,		// 升阶到了满级
	};

	BYTE result;
};

// 精炼--开始升星消息
DECLARE_MSG(SSBeginRefineStar, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_REFINE_STAR)
struct SQBeginRefineStar : public SSBeginRefineStar
{
	BYTE bIndex;		//	索引
	BYTE bType;		// 0 为玩家装备槽  1 玩家背包
	BYTE	 bCount;	//强化次数
};

struct SABeginRefineStar : public SSBeginRefineStar
{
	enum
	{
		BRS_SUCCESS,		// 成功
		BRS_NOMONEY,		// 金钱不足
		BRS_FAIL,			// 失败
		BRS_LACK_METRIAL,	// 缺少材料
		BRS_REFINE_STAR_FULL,
	};

	BYTE result;
};
DECLARE_MSG(SSBeginReMoveStar, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_ReMove_STAR)// 洗心
struct SQBeginReMoveStar : public SSBeginReMoveStar
{
	
};
struct SABeginReMoveStar : public SSBeginReMoveStar
{
	enum
	{
		ABS_SUCCESS,		// 成功
		ABS_NOMONEY,		// 金钱不足
		ABS_FAIL,			// 失败
		ABS_LACK_METRIAL,	// 缺少材料
		ABS_REFINE_STAR_Empty,
	};
	BYTE result;
};

// 开始升品质
DECLARE_MSG(SSStartQuality, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_UPGRADE_QUALITY)
struct SQStartQuality : public SSStartQuality
{
	BYTE bIndex;		//	索引
	BYTE bType;		// 0 为玩家装备槽  1 玩家背包
	//byte byStrengthenExternChoose; // 是否选中幸运符保护符等外部状态，参考TStrengthenExternChoose
};

struct SAStartQuality : public SSStartQuality
{
	enum
	{
		SSQ_SUCCESS,		// 升品质成功
		SSQ_NOMONEY,		// 金钱不足
		SSQ_FAIL,			// 升品质失败
		SSQ_LACK_METRIAL,	// 缺少材料
		SSQ_LACK_EUQIT,		// 缺少升级材料
		SSQ_YINBING,		// 银币不够
		SSQ_YINGLIANG,		// 银两不够
		SSQ_EQUITFail,		// 装备毁坏
	};

	BYTE  result;
	DWORD index;            //成功后新物品的ID
};

// 装备强化等级
DECLARE_MSG(SSBeginEquipLevelUpMsg, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_UPGRADE_LEVEL)
struct SQBeginEquipLevelUpMsg : public SSBeginEquipLevelUpMsg
{
	BYTE bIndex;		//	索引
	BYTE bType;		// 0 为玩家装备槽  1 玩家背包
	BYTE	 bCount;	//强化次数
};

struct SABeginEquipLevelUpMsg : public SSBeginEquipLevelUpMsg
{
	enum
	{
		ELU_SUCCESS,		// 升级成功
		ELU_NOMONEY,		// 金钱不足
		ELU_FAIL,			// 升级失败
		ELU_LACK_METRIAL,	// 缺少材料
		ELU_LACK_EQUIP,		// 缺少装备
		ELU_NOUPDTE_Level,	// 不能升级
		ELU_YINBING,		// 银币不够
		ELU_YINGLIANG,		// 银两不够
		ELU_EQUITFail,		// 装备毁坏
		ELU_COLDTIME,	//冷却时间中
	};
	BYTE  result;
	
};

// 装备鉴定
DECLARE_MSG(SIdentifyMsg, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_IDENTIFY)
struct SQBeginIdentifyMsg : public SIdentifyMsg
{
	SQBeginIdentifyMsg()
	{
		for (int i = 0; i < SEquipment::MAX_BORN_ATTRI; ++i)
		{
			baLockedExtAttri[i] = false;
		}
		bPriorityUseBindMoney = false;
	}
	enum
	{
		QBIM_JINGLIAN,		// 装备精练（提升附加属性）
		QBIM_XILIAN,		// 装备洗练（随机重新生成附加属性）
		QBIM_FENJIE,		// 装备分解
		QBIM_SMELTING,		// 装备熔炼
		QBIM_LINGFU,		// 装备灵附		
		QBIM_SHENGJIE,		// 装备升阶（提升附加属性的上限）		
	};

	BYTE type;
	bool baLockedExtAttri[SEquipment::MAX_BORN_ATTRI]; // [2012-8-23 16-16 gw: +记录附加属性的锁定状态] 
	byte bPriorityUseBindMoney; // [2012-8-23 16-39 gw: +是否有限使用绑定货币]
};

struct SABeginIdentifyMsg : public SIdentifyMsg
{
	enum
	{
		BIM_SUCESS,			// 成功鉴定
		BIM_NOMONEY,		// 缺少金钱
		BIM_MISMATCHING,	// 鉴定材料不匹配
		BIM_NOEQUIP,		// 没有鉴定装备
		BIM_FAIL,
		BIM_DefineMetrial_Less,//鉴定材料不足
		BIM_LockedItem_Less, // 锁定石不够
		BIM_JINGLIAN_FULL, // 精炼满了
		BIM_PACKAGE_FULL, // 包裹满了
		BIM_STAR_LESS,	// 星数不够
		BIM_LEVEL_LESS, // 装备等级不够
	};

	BYTE result; // 返回结果
	BYTE type; // 洗练-精炼等类型
};

// 发送给客户端洗练之后的装备信息,这个装备目前是还没有替换
DECLARE_MSG(SIdentifySynEquipMsg, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_IDENTIFY_SYNEQUIP)
struct SAUpdateIdentifyedEquipMsg : public SIdentifySynEquipMsg
{
	SEquipment newExternEquip; // 同步新的装备数据
};

// 洗练后请求交换装备
DECLARE_MSG(SIdentifyExChangeMsg, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_IDENTIFY_EXCHANGE)
struct SQIdentifyExchangeMsg : public SIdentifyExChangeMsg
{	
	BYTE type;
};
struct SAIdentifyExchangeMsg : public SIdentifyExChangeMsg
{	
	enum
	{
		IEM_SUCCESS, // 洗练交换成功
		IEM_FAIL,
	};
	BYTE result;
};

// 装备属性重置
// DECLARE_MSG(SResetAttribute, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_RESET_ATTRIBUTE)
// struct SQBeginResetAttribute : public SResetAttribute
// {
// 
// };
// 
// struct SABeginResetAttribute : public SResetAttribute
// {
// 	enum
// 	{
// 		BRA_SUCESS,
// 		BRA_NOMONEY,
// 		BRA_MISMATICHING,	// 重置材料不匹配
// 
// 		BRA_FAIL,
// 	};
// 
// 	BYTE result;
// };

// 取消强化
DECLARE_MSG(SSEndRefineMsg, SItemBaseMsg, SItemBaseMsg::EPRO_END_STRENGTHEN)
struct SQEndRefineMsg : public SSEndRefineMsg
{
	enum
	{
		ERM_CLOSE,				// 关闭面板
		ERM_EQUIP_OFF,			// 取消装备
		ERM_METRAIL_OFF,		// 取消原始材料
	};

	BYTE bType;
	BYTE bPosX;	// 坐标
	BYTE bPosY; // 坐标
};
struct SAEndRefineMsg : public SSEndRefineMsg
{
	enum
	{
		ERM_SUCCESS, // 关闭或者取消面板取消原材料成功
		ERM_FAIL, // 操作失败
	};

	BYTE bType;
	BYTE bPosX;	// 坐标
	BYTE bPosY; // 坐标
	BYTE bResult; // 返回操作结果
};


// 发送背包里装备的信息
DECLARE_MSG(SSEquipInfoInBagMsg, SItemBaseMsg, SItemBaseMsg::EPRO_EQUIP_INFO_IN_BAG)
struct SAEquipInfoInBagMsg : public SSEquipInfoInBagMsg
{
	SEquipment	equip;
	WORD		wPos; // 背包里的位置	
};

// 显示强化后的装备
DECLARE_MSG(SSAfterUpgradeEquipMsg, SItemBaseMsg, SItemBaseMsg::EPRO_AFTER_UPGRADE_EQUIP)
struct SAAfterUpgradeEquipMsg : public SSAfterUpgradeEquipMsg
{
	enum
	{
		UPDATE_LEVEL,
		UPDATE_GRADE,		//升阶		
		UPDATE_QUALITY,	
		UPDATE_STAR,		// 强化
		UPDATE_ReMove,		//洗星
		EQUIP_RESET,	 // 装备洗练
		EQUIP_REFINE,	 // 装备精炼
		ADD_HOLE,		 // 装备打孔
		GEM_INSERT,		 // 装备镶嵌宝石
		GEM_REMOVE,		 // 装备宝石摘除
	};

	SEquipment	equip;
	BYTE		type;
};



// 使用特殊道具，目前用于处理侠客在装备道具时
DECLARE_MSG(SpecialUseItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_SPECIAL_USEITEM)
struct SQSpecialUseItemMsg : public SpecialUseItemMsg
{
	enum
	{
		USEFOR_XIAKE,			// 对侠客使用
		USEFOR_MOUNT,			// 对坐骑使用
		USEFOR_NPC,				// 对NPC使用
		USEFOR_MONSTER,			// 对怪物使用
		USEFOR_PLAYER,			// 对玩家使用
		USEFOR_NULL,
	};
	BYTE btype;					// 类型
	WORD wCellPos;				// 坐标	
	WORD windex;				// 侠客的索引，以后可以是其他东西
};

// 侠客装备孔的信息
DECLARE_MSG(SFightPetEquipInfoMsg, SItemBaseMsg, SItemBaseMsg::RPRO_FIGHTPET_EQUIPINFO)
struct SAFightPetEquipInfoMsg : public SFightPetEquipInfoMsg
{
	SEquipment	stEquip;
	BYTE		byPos;
	BYTE		index;			// 侠客索引
};

// 侠客装备一个外表可见道具
DECLARE_MSG(SFightPetEquipItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_FIGHTPET_EQUIPITEM)
struct SAFightPetEquipItemMsg : public SFightPetEquipItemMsg
{
	DWORD	dwGlobalID;
	BYTE			byEquipPos;
	DWORD	wItemIndex;
	BYTE		index;			// 侠客索引
};

// 在装备栏点击右键卸载侠客装备
DECLARE_MSG(SfpUnEquipItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_UNLOAD_FIGHTPETEQUIP)
struct SQfpUnEquipItemMsg : public SfpUnEquipItemMsg
{
	DWORD	dwGlobalID;
	BYTE	byEquipPos;
	BYTE		index;			// 侠客的索引
};

// XYD3排行榜中查看其他玩家的装备信息
DECLARE_MSG(SRankEquipInfoMsg, SItemBaseMsg, SItemBaseMsg::EPRO_RANK_EQUIPMENT_INFO)
struct SAankEquipInfoMsg : public SRankEquipInfoMsg
{
	char		szName[CONST_USERNAME];				// 姓名	OK
	WORD		wOtherLevel;			// 玩家等级OK
	SEquipment	Equip[16];

	DWORD		dwGlobal;				// GID
	BYTE		bFaceID;				// 脸模型
	BYTE		bHairID;				// 头发模型
	DWORD		m_3DEquipID[11];			// 3D装备挂件物品号

	float		m_fPositionX;			// 位置x
	float		m_fPositionY;			// 位置y
	float		m_fDirection;			// 方向

	BYTE		bySex:1;			    // 玩家角色性别
	BYTE		byBRON:3;				// 玩家所显示图片级别，==1表示出师
	BYTE		bySchool:4;				// 玩家所属门派OK
};

//侠客转换为相应道具
DECLARE_MSG(SfpToitemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_FIGHTPET_FIGHTPETTOITEM)
struct SQfpToitemMsg : public SfpToitemMsg
{
	BYTE		index;			// 侠客的索引
};

// 取消洗点
DECLARE_MSG(SSEndResetPoint, SItemBaseMsg, SItemBaseMsg::EPRO_END_RESETPOINT)
struct SQEndResetPoint : public SSEndResetPoint
{
};

// 装备后属性发生变化
DECLARE_MSG(SequitattrichangeMsg, SItemBaseMsg, SItemBaseMsg::EPRO_EQUIP_ATTRICHANGE)
struct SAequitattrichangeMsg : public SequitattrichangeMsg
{
	enum 
	{
		EQUIP_GONGJI,	// 攻击
		EQUIP_FANGYU,	// 防御
		EQUIP_BAOJI,		// 暴击
		EQUIP_SHANBI,	// 闪避
	};	
	BYTE		index;				// 改变的类型
	WORD  changevalue;	//改变的值
};

// =================道具强化类型相关==================
DECLARE_MSG(SDragUpdateItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_DRAG_UPDATE_ITEM)
struct SQDragUpdateItemMsg : public SDragUpdateItemMsg
{
	enum UPDATE_TYPE
	{
		DUI_HUISHENDAN,

		DUI_NONE,
	};

	WORD wCellPos; // 坐标
	BYTE bType;		// 道具升级类型
};

DECLARE_MSG(SDragUpdateMetrialMsg, SItemBaseMsg, SItemBaseMsg::EPRO_DRAG_UPDATE_METRIAL)
struct SQDragUpdateMetrialMsg : public SDragUpdateMetrialMsg
{
	enum
	{
		DUM_LIANZHI,
		DUM_XUANJING,
	};

	WORD wCellPos;		// 概率材料坐标
	BYTE bType;		// 道具材料类型
};

DECLARE_MSG(SBeginUpdateItem, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_UPDATE_ITEM)
struct SQBeginUpdateItem : public SBeginUpdateItem
{

};
struct SABeginUpdateItem : public SBeginUpdateItem
{
	enum
	{
		BUI_SUCCESS,
		BUI_FAILED,
		BUI_MAX_TIMES,
	};

	WORD	wCellPos; // 坐标

	BYTE	bType;
	BYTE	bSuccessTimes;
	BYTE	bTotalUpdateTimes;
};

DECLARE_MSG(SEndUpdateItem, SItemBaseMsg, SItemBaseMsg::EPRO_END_UPDATE_ITEM)
struct SQEndUpdateItem : public SEndUpdateItem
{
	enum
	{
		EUI_ITEM_OFF,
		EUI_LJS_OFF,
		EUI_XMTJ_OFF,
	};

	BYTE bType;
};

DECLARE_MSG(SHuiShenDanListInfoMsg, SItemBaseMsg, SItemBaseMsg::EPRO_HUISHENDAN_LIST_INFO)
struct SAHuiShenDanListInfoMsg : public SHuiShenDanListInfoMsg
{
	BYTE buffer[1024];
};

#define MAX_PACK_BUFFER 10240
//发送打包信息到客户端,	根据类型不同，可以是一些物品ID，也可以是其他东西，看具体情况 	
DECLARE_MSG(SBufferPackMsg, SItemBaseMsg, SItemBaseMsg::EPRO_PACK_BUFFERPACK)
struct SABufferPackMsg : public SBufferPackMsg
{
	enum
	{
		PACK_COUNTDOWNGIFT,	//倒计时礼包
		PACK_ONLINEGIFT,	//在线玩家礼包
		PACK_ROSERECORD, //好友的送花记录
		PACK_LEVELGIFT,	//等级奖励礼包
		PACK_LOGINGIFT,	//连续登陆礼包
	};
	BYTE		PackType;			//类型
	WORD Packnum;			//  数量
	BYTE		PackBuff[MAX_PACK_BUFFER];
};

DECLARE_MSG(SGetOnlienGiftMsg, SItemBaseMsg, SItemBaseMsg::EPRO__GET_ONLINEGIFT)
struct SQGetOnlienGiftMsg : public SGetOnlienGiftMsg
{
	enum
	{
		PACK_COUNTDOWNGIFT,	//倒计时礼包
		PACK_ONLINEGIFT,	//在线玩家礼包
		PACK_ROSERECORD, //好友的送花记录
		PACK_LEVELGIFT,	//等级奖励礼包
		PACK_LOGINGIFT,	//连续登陆礼包
	};
	BYTE		PackType;			//类型
	BYTE		index;				//索引
};

struct SAGetOnlienGiftMsg : public SGetOnlienGiftMsg
{
	enum
	{
		SENDGIFT_FAIL,//礼包领取失败
		SENDGIFT_OK,//礼包领取成功
		SENDGIFT_ALREADY,//今天礼包已经领取了或者已经领取了当前等级的
		SENDGIFT_ALLSEND,//所有礼包已经全部领取
	};
	BYTE bResult;
	BYTE	 PackType;			//类型
	BYTE index;		//索引
};


// 时装模式或者普通模式的切换
DECLARE_MSG(SFashionChangeMsg, SItemBaseMsg, SItemBaseMsg::EPRO_EQUIP_FASHIONCHANGE)
struct SQFashionChangeMsg : public SFashionChangeMsg
{
	enum{NORMALMODE,FASHIONMODE};
	BYTE mode;
};

struct SAFashionChangeMsg : public SFashionChangeMsg
{
	enum{NORMALMODE,FASHIONMODE};
	BYTE mode;
	DWORD playergid;
};

// 套装附加属性是否激活
DECLARE_MSG(SSuitAttribute, SItemBaseMsg, SItemBaseMsg::EPRO_EQUIP_SUIT_ATTRIBUTE)
struct SQSuitAttributeMsg : public SSuitAttribute
{

};

struct SASuitAttributeMsg : public SSuitAttribute
{
	// 装备套装加成[同级.同阶.同品质.10级以上宝石.全部镶星]
	// 与CItemDefine文件中的 套装附加属性类型 枚举对应
	enum
	{
		EQUIP_SAME_LEVEL = 1,
		EQUIP_SAME_GRADE,
		EQUIP_SAME_COLOR,
		EQUIP_SAME_JEWEL,
		EQUIP_SAME_STAR,
	};

	// 用16为无符号存储状态，最大能够定义16种属性状态状态
	WORD bSuitAttriState;
};

DECLARE_MSG(SChannelCallSellInfo,SItemBaseMsg,SItemBaseMsg::EPRO_ChannelCallSell_INFO)
struct SQChannelCallSellInfo : public SChannelCallSellInfo
{
	WORD wCellPos;			// 格子坐标
	long AllPrice;			// 总价(银两)
	BYTE NoticeRoll;		// 提示滚动 0 不提示 1 提示
};

struct SAChannelCallSellInfo : public SChannelCallSellInfo
{
	BYTE ErrorCode;			// 1(成功) 2(钱不够扣) 3(格子数不对有可能越界) 4(格子为空) 5(格子已经锁定) 6() 
};

DECLARE_MSG(SChannetCallBuyInfo,SItemBaseMsg,SItemBaseMsg::EPRO_ChannetCallBuy_INFO)
struct SQChannetCallBuyInfo : public SChannetCallBuyInfo
{
	DWORD m_PlayerGID;		// 玩家GID
	WORD  m_Pos;			// 位置
};

struct SAChannetCallBuyInfo : public SChannetCallBuyInfo
{
	BYTE ErrorCode;			// 1(成功) 2(没有找到这个玩家，玩家已经下线) 3(发来的位置不对)
};

DECLARE_MSG(SSendReward,SItemBaseMsg,SItemBaseMsg::EPRO__SEND_REWARD)
struct SQSendReward : public SSendReward
{
	enum RewardType			//奖励类型
	{
		TOTALLOGIN,	//累计登录
		CONTINLOGIN,	//连续登录
		CHENGJIUREWARD,//成就
		LEAVEREWARD,//离线
		BUCHANGREWARD,//补偿
		CHONGZHIREWARD,//充值
	};
	BYTE	 bRewardtype;	//奖励类型
	WORD	wRewardIndex;//请求领取的ID
};

struct SASendReward : public SSendReward
{
	enum RewardType			//奖励类型
	{
		TOTALLOGIN,	//累计登录
		CONTINLOGIN,	//连续登录
		CHENGJIUREWARD,//成就
		LEAVEREWARD,//离线
		BUCHANGREWARD,//补偿
		CHONGZHIREWARD,//充值
	};
	enum GETTYPE
	{
		NOTGET,	//没有领取 已经打到条件但是没有领取
		ISGET,	//已经领取
		NONEGET,//无法领取
	};
	BYTE	 bRewardtype;	//奖励类型
	WORD	wRewardNum;//奖励条数
	int	nRewardData;//奖励数据
};

//在强化前请求强化描述
DECLARE_MSG(SIntensifyInfo,SItemBaseMsg,SItemBaseMsg::EPRO_INTENSIFY_INFO)
struct SQIntensifyInfo : public SIntensifyInfo
{
	BYTE bIndex;		//	索引
	BYTE bType;		// 0 为玩家装备槽  1 玩家背包
};

//服务器回复强化描述信息
struct SAIntensifyInfo : public SIntensifyInfo
{
	WORD		bIntensifysuccessrate; //强化成功率
	WORD		bupgradrate;// 升阶成功率
	WORD		bnextupgradrate;// 下一等级升阶成功率
	WORD		wNextLevelattriValue;//强化至下一等级的属性值
	DWORD dcoldtime;		//已经使用的强化冷却时间
	DWORD dallcoldtime;		//所有强化冷却时间
	DWORD dmoneycost;//单次银币花费
	BYTE       bCanIntensify;			//能否强化 0 否
};

//武器切换
DECLARE_MSG(SEquipSwitch,SItemBaseMsg,SItemBaseMsg::EPRO__EQUIP_SWITCH)
struct SQEquipSwitch : public SEquipSwitch
{
	BYTE bIndex;		//	索引1
	BYTE bIndex2;		//	索引2    索引1的武器将切换到索引2中
};

struct SAEquipSwitch : public SEquipSwitch
{
	enum
	{
		SUCCESS,   
		FAIL,			//失败，格子可能处于锁定状态
	};
	BYTE bresule;
};
//获取提升品质的需求
DECLARE_MSG(SEquipQualityInfo, SItemBaseMsg, SItemBaseMsg::EPRO_QUALITY_INFO)
struct SQEquipQualityInfo : public SEquipQualityInfo
{
	BYTE bIndex;		//	索引
	BYTE bType;		// 0 为玩家装备槽  1 玩家背包
};

//服务器回复品质描述信息
struct SAEquipQualityInfo : public SEquipQualityInfo
{
	DWORD	dnextqualityitem;	//提升品质成功后下一阶的道具
	DWORD	dmoneycost;//单次银币花费
	WORD		bQualitysuccessrate; //提升品质成功率
	WORD      wneedLevel;//需要等级
	BYTE			bmaterial[512];	//材料需要 itemid + num
};

//道具出售
DECLARE_MSG(SItemSale, SItemBaseMsg, SItemBaseMsg::EPRO__ITEM_SALE)
struct SQItemSale : public SItemSale
{
	BYTE bIndex;		//	背包中的索引
};

struct SAItemSale : public SItemSale
{
	enum
	{
		ITEMSALE_OK,			//成功
		ITEMSALE_FAILE,		//失败可能处于锁定状态，或者当前道具不允许出售等待
	};
	BYTE bResult;//出售结果
};

//清除强化冷却时间 
DECLARE_MSG(SCleanColdtime, SItemBaseMsg, SItemBaseMsg::EPRO_CLEAN_COLDTIME)
struct SQCleanColdtime : public SCleanColdtime
{
};

struct SACleanColdtime : public SCleanColdtime
{
	BYTE bresult;
};

//升星描述 
DECLARE_MSG(SRiseStarInfo, SItemBaseMsg, SItemBaseMsg::EPRO_RISE_INFO)
struct SQRiseStarInfo : public SRiseStarInfo
{
	BYTE bIndex;		//	索引
	BYTE bType;		// 0 为玩家装备槽  1 玩家背包
};

struct SARiseStarInfo : public SRiseStarInfo
{
	WORD starpropertyid;//升星后的装备属性ID
	WORD starvalue;//升星后的属性值
	WORD		bsuccessrate; //升星成功率
	WORD dmaterialnum;//材料数量
	DWORD dmaterialid; //材料道具ID
	DWORD dmoney;//需求的银币数量
};

//请求开始升级刻印属性
DECLARE_MSG(SBeginKeYin, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_KEYIN)
struct SQBeginKeYin : public SBeginKeYin
{
	BYTE bIndex;		//	索引
	BYTE bType;		// 0 为玩家装备槽  1 玩家背包
	BYTE bPropertyIndex;	//刻印属性索引
};
struct SABeginKeYin : public SBeginKeYin
{
	enum KEYINRESULT
	{
		KEYIN_SUCCESS,  //刻印成功
		KEYIN_FAILE,  //刻印失败
	};
	BYTE bResult;		//	结果
};


//请求刻印属性描述
DECLARE_MSG(SKeYinInfo, SItemBaseMsg, SItemBaseMsg::EPRO_KEYIN_INFO)
struct SQKeYinInfo : public SKeYinInfo
{
	BYTE bIndex;		//	索引
	BYTE bType;		// 0 为玩家装备槽  1 玩家背包
	BYTE bPropertyIndex;	//刻印属性索引
};

struct SAKeYinInfo : public SKeYinInfo
{
	WORD		bsuccessrate; //刻印成功率
	WORD dmaterialnum;//材料数量
	DWORD dmaterialid; //材料道具ID
	DWORD dmoney;//需求的银币数量
};

////请求刻印属性转换
DECLARE_MSG(SKeYinChange, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_KEYINCHANGE)
struct SQKeYinChange : public SKeYinChange
{
	BYTE bIndex;		//	索引
	BYTE bType;		// 0 为玩家装备槽  1 玩家背包
	BYTE bPropertyIndex;	//准备更改的刻印属性索引
	BYTE bTargetIndex;	//目标刻印属性索引
	BYTE	 bChangeType;//转换类型 0为使用道具转换 1为使用银币转换
};

////请求刻印属性转换信息
DECLARE_MSG(SKeYinChangeInfo, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_KEYINCHANGEINFO)
struct SQKeYinChangeInfo : public SKeYinChangeInfo
{
	BYTE bIndex;		//	索引
	BYTE bType;		// 0 为玩家装备槽  1 玩家背包
	BYTE bPropertyIndex;	//准备更改的刻印属性索引
};
struct SAKeYinChange : public SKeYinChangeInfo
{
	WORD dmaterialnum;//材料数量
	DWORD dmaterialid; //材料道具ID
	DWORD dmoney;//需求的银币数量
	BYTE bPropertyIndex[3];	//准备更改的刻印属性
};


////请求装备鉴定
DECLARE_MSG(SEquipJDing, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_JDING)
struct SQEquipJDing : public SEquipJDing
{
	BYTE bIndex;		//	索引
	BYTE bType;		// 0 为玩家装备槽  1 玩家背包
	BYTE bPropertyIndex;	//准备鉴定的属性索引
	BYTE bIslock;//是否锁定当前的鉴定属性
};

struct SAEquipJDing : public SEquipJDing
{
	BYTE bPropertyIndex;	//准备鉴定的属性索引
	BYTE bPropertyID;//属性ID
	WORD wvalue;//属性值
	BYTE	 bColor; //鉴定后的鉴定条显示的颜色
};

//请求装备鉴定信息
DECLARE_MSG(SEquipJDingInfo, SItemBaseMsg, SItemBaseMsg::EPRO_JDING_INFO)
struct SQEquipJDingInfo : public SEquipJDingInfo
{
	BYTE bIndex;		//	索引
	BYTE bType;		// 0 为玩家装备槽  1 玩家背包
};

struct SAEquipJDingInfo : public SEquipJDingInfo
{
	WORD dmaterialnum;//鉴定材料数量
	WORD dlockmaterialnum;//锁定鉴定材料数量
	DWORD dmaterialid; //材料道具ID
	DWORD dlockmaterialid; //锁定材料道具ID
};
#define COLORNUM 6	//颜色数量
//同步装备鉴定条颜色信息
DECLARE_MSG(SEquipJDingColor, SItemBaseMsg, SItemBaseMsg::EPRO_JDING_COLOR)
struct SQEquipJDingColor : public SEquipJDingColor
{
	BYTE bIndex;		//	索引
	BYTE bType;		// 0 为玩家装备槽  1 玩家背包
};

struct SAEquipJDingColor : public SEquipJDingColor
{
	BYTE bIndex;		//	索引
	BYTE bType;		// 0 为玩家装备槽  1 玩家背包
	BYTE bcolorIndex[COLORNUM];		//	索引
};

///2014.2.20 Add
//每日祈祷请求和应答描述 
DECLARE_MSG(SQiDaoInfo, SItemBaseMsg, SItemBaseMsg::EPRO_EVERYDAY_ALLQIDAO)
struct SQQiDaoInfo : public SQiDaoInfo
{
};

struct SAQiDaoInfo : public SQiDaoInfo
{
	WORD wSilerCoinGetCount; //每次购买银币可获得的银币数
	WORD wSilerCoinPrice; //银币的单价
	WORD wSilerCoinCanNum; //每天可以获取银币的次数
	WORD wSilerCoinUsedNum;	//当天银币已经获得的次数，每天24小时后，该次数重置为0
	WORD wAnimaGetCount;
	WORD wAnimaPrice;
	WORD wAnimaCanNum;
	WORD wAnimaUsedNum;
	WORD wExpGetCount;
	WORD wExpPrice;
	WORD wExpCanNum;
	WORD wExpUsedNum;
	WORD wNextVipCanNum;		//下一次VIP可以获取的总次数
};

//添加祈祷的相关点数（银币、真气和经验）
DECLARE_MSG(SAddQiDaoInfo, SItemBaseMsg, SItemBaseMsg::EPRO_EVERYDAY_ADDQIDAO)
struct SQAddQiDaoInfo : public SAddQiDaoInfo
{
	BYTE bQiDaoType;		//0：为银币  1：为真气  2：为经验
};

struct SAAddQiDaoInfo : public SAddQiDaoInfo
{
	char cOptResult;	////-1: 表示数量不足  0: 表示金币不足  1：表示成功
};

//2014.2.21 Add
//元宝礼包的相关数据结构
DECLARE_MSG(SGiftPacketMsg, SItemBaseMsg, SItemBaseMsg::EPRO_GET_YUANBAOGIFTINF)
struct SQGiftPacketMsg : public SGiftPacketMsg
{
	DWORD dwRqItemPos;	//请求礼包的位置
};

typedef struct _GiftPacketData
{
	unsigned int CurIDItemCount;	//当前道具的数量

	unsigned int ItemID;	//道具的ID
}GiftPacketData, *LPGiftPacketData;

struct SAGiftPacketMsg : public SGiftPacketMsg
{
	BYTE Isbuyed;	//是否已经购买1为购买，0为没有购买

	BYTE TypeCount;	//道具类型的数量

	DWORD IsExtraCell;	//是否有额外的第九个格子,如果有则为新礼包的ID号，为0表示没有第九个格子

	LPGiftPacketData lpGiftPacketData;

	SAGiftPacketMsg(BYTE ucTypeCount)
	{
		TypeCount = ucTypeCount;
		lpGiftPacketData = new GiftPacketData[TypeCount];
	}
	~SAGiftPacketMsg()
	{
		delete[] lpGiftPacketData;
	}
};

//购买元宝礼包的相关数据结构
DECLARE_MSG(SBuyGiftPacketMsg, SItemBaseMsg, SItemBaseMsg::EPRO_BUY_ALLYUANBAOGIFT)
struct SQBuyGiftPacketMsg : public SBuyGiftPacketMsg
{
};

struct SABuyGiftPacketMsg : public SBuyGiftPacketMsg
{
	BYTE bResult;	//0， 金币不够  1， 背包不够   2， 成功
};

//请求套装条件
DECLARE_MSG(SSuitcondition, SItemBaseMsg, SItemBaseMsg::EPRO_SUIT_CONDITION)
struct SQSuitcondition : public SSuitcondition
{
	BYTE btype;//套装类型 1 品质条件 2 星级条件 3刻印条件
};

struct SASuitcondition : public SSuitcondition
{
	BYTE btype;//套装类型 1 品质条件 2 星级条件 3刻印条件
	BYTE benough[EQUIP_P_MAX];//是否到达当前条件
	int nenoughproperty[3];//满足条件增加的属性值
	int ncurrentproperty[3];//当前增加的属性值
	WORD wcurrentlevel;//达到值后获得属性
	WORD  wepropertyindex[3]; //满足条件增加的属性ID
	WORD  wcepropertyindex[3]; //当前条件增加的属性ID
};

DECLARE_MSG(SCellCountDown, SItemBaseMsg, SItemBaseMsg::EPRO_CELL_COUNTDOWN)
struct SQCellCountDown : public SCellCountDown
{
};

struct SACellCountDown : public SCellCountDown
{
	DWORD m_CountDown; //为当前格子的倒计时时间
};

//请求相应装备战斗力
DECLARE_MSG(SEquipPower, SItemBaseMsg, SItemBaseMsg::EPRO_EQUIPT_POWER)
struct SQEquipPower : public SEquipPower
{
	BYTE bIndex;		//	索引
	BYTE bType;		// 0 为玩家装备槽  1 玩家背包
};

struct SAEquipPower : public SEquipPower
{
	DWORD dequippower;
	BYTE bIndex;		//	索引
};

// 玩家购买体力
DECLARE_MSG(SBuyPlayerTp, SItemBaseMsg, SItemBaseMsg::EPRO_BUY_PLAYERTP)
struct SQBuyPlayerTp : public SBuyPlayerTp
{
};
