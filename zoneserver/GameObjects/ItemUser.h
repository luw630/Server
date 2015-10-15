#pragma once

#include "fightobject.h"
#include "randomsys.h"
#include "Networkmodule\ItemTypedef.h"
#include "EventManager.h"
#include "EventTimer.h"
#include "SingleItem.h"
const __int32 IID_ITEMUSER = 0x117ca49d;

class CNpc;

// 侠义道3背包CELL结构
struct SItemWithFlag
{
	enum
	{
		IWF_ACTIVED		= 0x0001,		// 此格子是否被激活
		IWF_LOCKED		= 0x0002,		// 此格子是否被锁定
		IWF_ITEMIN		= 0x0004,		// 此格子是否有道具
	};

	WORD			m_State;			// 格子状态
	SPackageItem	m_Item;				// 道具内容
};

// 已穿装备的锁定状态
struct SEquipedWithFlag
{
	WORD m_State;	// 已穿装备的锁定状态，使用SItemWithFlag中的枚举状态
};

// 侠义道3背包类型
enum XYD3PackageType
{
	XYD_PT_BASE = 1,		// 基本+扩展1+扩展2+扩展n...
	//XYD_PT_TASK,			// 任务

	XYD_PT_MAX,				// Max
};

// 侠义道3查找道具分类
enum XYD3FindItemType
{
	XYD_FT_ONLYLOCK = 1,	// 只找锁定
	XYD_FT_ONLYUNLOCK,		// 只找非锁定
	XYD_FT_WHATEVER,		// 随便
};

// 侠义道3货币使用规则
enum XYD3MoneyUseRule
{
	XYD_UM_ONLYUNBIND = 1,	// XYD3--->银两  非绑定 人民币
	XYD_UM_ONLYBIND,		// XYD3--->银币  绑定 游戏币
	//XYD_UM_ONLYUNBIND,		

	XYD_UM_ZENGBAO,			// 赠宝 = XYD3--->礼券
	XYD_UM_YUANBAO,			// 元宝 = XYD3--->元宝

	XYD_UM_BINDFIRST,		// 绑定优先使用/增加非绑定溢出可加入到绑定中
};

// 分界限
const WORD BaseStart	= 0;
const WORD BaseEnd		= 72/*4 * (PackageWidth * PackageHight)*/;
const WORD BaseActivedPackage = 11/* PackageWidth * PackageHight*/; //初始被激活的背包11个，就是第一次进游戏就送的背包格子数

// 道具添加参数
struct SAddItemInfo
{
	SAddItemInfo() : m_Index(0), m_Count(0) {}
	SAddItemInfo(DWORD index, WORD count) : m_Index(index), m_Count(count) {}

	DWORD	m_Index;
	WORD	m_Count;
};

class CItemUser : public CFightObject
{ 
public:
    BOOL AutoUseItem(WORD wItemID, DWORD useType = 0);
    BOOL AutoUseItem(SPackageItem *pItem, DWORD useType = 0);
	BOOL CheckItemChange();

public:
	static BYTE GetSItemBaseBindState(BYTE eCurBindType, BYTE byItemDataBindType); // [11-12-2012 17:16 gw]+获取物品绑定状态 	
	static BOOL GenerateNewItem(SRawItemBuffer &itemBuffer, const SAddItemInfo &params,BYTE index = 0);
	// 随机添加装备的随机属性
	static BOOL AddRandomAttri(SEquipment* equip);	
	static BOOL ResetRandomExtraAttriEntry(SEquipment* equip, const vector<int> &vecLockedIndex);// [2012-8-23 18-06 gw: +重新设置随机属性的条目，对于已经锁定的条目和数据 不处理]
	static BOOL OnUpdataGrade(SEquipment * pEquip);
	virtual BOOL _UseItem(SPackageItem *pItem,DWORD useType=0);
	BOOL SetPreTime(SPackageItem *pItem, DWORD in_dwLocSrvPreTime = 0, DWORD in_dwLocSrvBreakOdds = 0);
	//BOOL BreakOperation();
	CNpc *GetCurrentNpc(void);
	BOOL BuyItem(DWORD index, WORD count);
	WORD GetBlankCell(BYTE type);
	WORD GetRemainOverlap(DWORD index);
	WORD GetNeedBlank(const struct SItemBaseData *pData, const SAddItemInfo &Info);
	BOOL CanAddItems(std::list<SAddItemInfo> &itemList, WORD reuseBaseBlank = 0, WORD reuseTaskBlank = 0);
	WORD TryPossibleMerge(DWORD index, WORD count, WORD maxOverlap);
	BOOL AddExistingItem(const SItemBase &item, WORD pos, BOOL synchro);
	BOOL FillItemPos(WORD pos, BYTE type, bool mark);	
	SPackageItem *FindItemByPos(WORD pos, WORD LookType);
	//add by ly 2014/3/25
	SPackageItem *FindItemByItemID(DWORD ItemID, WORD LookType = XYD_FT_WHATEVER);	//检索相应的道具ID是否在背包中,如果在返回对应的信息
	void SendItemSynMsg(SPackageItem *pItem, WORD LookType = XYD_FT_ONLYUNLOCK);
	BOOL DelItem(const SPackageItem &pItem, LPCSTR info = NULL, bool autoUnLock = false);
	WORD FindBlankPos(BYTE type);
	BOOL StartAddItems(std::list<SAddItemInfo> &itemList);
	int AddItem(SAddItemInfo &siteminfo); //增加单个道具，返回位置
	void SetPlayerInitItems(CPlayer *pPlayer, bool OnlyInit);
	BOOL RecvAddItem(DWORD wIndex, WORD iNum, WORD level = 1);
	BOOL IsItemPosValid(WORD wCellPos, BOOL canBeLock = false, BOOL canBeEmpty = false);
	BOOL CanEquipIt(CPlayer *pPlayer, SPackageItem *pIB, const struct SItemBaseData *pData);
	BOOL CanEquipItFightPet(CPlayer *pPlayer, SPackageItem *pIB, const struct SItemBaseData *pData,BYTE index);//能否为侠客装备
	BOOL CanUseIt(CPlayer *pPlayer, SPackageItem *pIB, const struct SItemBaseData *pData);
	BOOL _EquipItem(SPackageItem *pItem);

	BOOL _UseBookUpSkill(SPackageItem *pItem);		//使用技能书升级技能
	BOOL _UseFightPetItem(SPackageItem *pItem,BYTE index);		//使用已经生成的侠客道具
	BOOL _EquipFightPetItem(SPackageItem *pItem,BYTE index);				// 装备侠客道具
	BOOL PickOneItem(SRawItemBuffer &goundItem);							// 拾取一件地面的物品
	BOOL PickSingleItem(CSingleItem &SingleItem);							// 拾取一件地面的物品
	BOOL DelEquipment(BYTE equipID);			                            // 删除一个装备
	BOOL _fpDelEquipment(BYTE equipID);			                            // 在侠客上删除一个装备
	BOOL InPackage(WORD wCellPos1, WORD wCellPos2);				// 背包内移动
	BOOL ToPackage(BYTE byPos,BOOL onlydel = FALSE);									// 拖动卸装,是否仅删除
	BOOL _fpToPackage(BYTE byPos,BYTE index);								// 侠客卸载装备
	BOOL ToEquip(WORD wCellPos);										// 装备
	BOOL _fpToEquip(WORD wCellPos,BYTE index);							// 侠客装备
	BOOL SynItemFlag(SPackageItem *item);									// 同步道具的属性
	BOOL LockBatchCells(std::vector<WORD> &vecCell, bool lock, bool sync = true);// +Func: 锁定或者解锁一批格子
	BOOL LockItemCell(WORD wCellPos, bool lock, bool sync = true);		// 锁定/解锁一个格子
	BOOL CheckGoods(DWORD index, DWORD number, BOOL checkOnly);				// 仅仅检查没有被锁定的道具！！
	int InsertItem(DWORD index, WORD willCreate, const SItemBaseData *pData, SRawItemBuffer &item, bool genNew);
	void ChangeOverlap(SPackageItem *pItem, WORD delta, bool Add, bool Sync = true);
	DWORD GetItemNum(DWORD wIndex, WORD GetType); //取得道具数量
	DWORD GetAllItemNum(WORD GetType= XYD_FT_ONLYUNLOCK); //取得背包内所有道具数量,重复的道具算1个
	DWORD GetRadomItem(WORD GetType = XYD_FT_ONLYUNLOCK); //取得背包内的一个有效的道具
	int GetRadomItemPos(WORD GetType = XYD_FT_ONLYUNLOCK); //取得背包内的一个有效的道具
	bool ValidationPos(WORD pos);
	BOOL LockItemCellTwo(WORD wCellPos, bool lock, bool sync  = true );
	//添加祈福道具新接口
	long IsHaveSpaceCell();
	long CreateNewItemInfo(long index,long Flag,SPackageItem &Item,long FLAGX);
	void InSertBlessItem(const SItemBase &item, WORD pos);
	void OnAllNotice();
	// 侠义道三任务检测
	void UpdateItemRelatedTaskFlag(class CPlayer *pPlayer,	DWORD index);
	void UpdateItemTaskFlag(class CPlayer *pPlayer,			DWORD index);
	void UpdateUseItemTaskFlag(class CPlayer *pPlayer,		DWORD index);
	void UpdateGiveItemTaskFlag(class CPlayer *pPlayer,		DWORD index);
	void UpdateGetItemTaskFlag(class CPlayer *pPlayer,		DWORD index);
	void UpdateMapTaskFlag();

	// 消息处理函数
	BOOL RecvUnEquipItem(struct SQUnEquipItemMsg *pMsg);					// 客户端要求卸装
	BOOL RecvUnfpEquipItem(struct SQfpUnEquipItemMsg *pMsg);				// 客户端要求卸下侠客装备
	BOOL RecvPickupItem(struct SQPickupItemMsg* pMsg);						// 客户端要求从地上捡起一个道具
	BOOL OnPickupItem(CSingleItem *pItem);	
	BOOL RecvDropItem(struct SQDropItemMsg* pMsg);							// 客户端要求从道具栏扔掉一个道具
	BOOL RecvUseItem(struct SQUseItemMsg* pMsg);							// 客户端要求使用一个道具
	BOOL RecvSpecialUseItem(struct SQSpecialUseItemMsg* pMsg);				// 客户端要求装备侠客道具
	BOOL RecvMoveItem(struct SQMoveItemMsg* pMsg);							// 客户端要求在物品栏中移动道具
	BOOL RecvBuyItem(struct SBuy* pMsg);									// 道具交易
	BOOL RecvActiveExtPackage(struct SQActiveExtPackage *msg);				// 收到激活背包的消息
	BOOL RecvSplitItem(struct QuerySplitItemMsg *pMsg);						// 分开道具
	BOOL RecvOpenBuyBackList();												// 打开回购列表
	BOOL RecvQuestFollowItem(struct SQQuestFollowItemMsg *pMsg);			// 随身商店
	BOOL RecvBuyFollowItem(struct SQBUYFollowItemMsg *pMsg);				// 购买随身道具
	BOOL RecvBlessExcellenceData(struct SQBlessExcellenceDataMsg *pMsg);
	BOOL RecvBlessBlessReQuest(struct SQBlessItemReQuestMsg *pMsg);
	BOOL RecvBlessQuestAddItems(struct SQBlessQuestAddItems* pMsg);
	BOOL RecvChannelCallSellInfoQuest(struct SQChannelCallSellInfo* pMsg);
	// 装备修复
	BOOL RecvFixOneItem(struct SQEquipFixOne *pMsg);
	BOOL RecvFixAllItem(struct SQEquipFixAll *pMsg);

	// 装备修复的内部使用函数
	SEquipment* GetEquipByPos(BYTE type, BYTE pos);

	BOOL StartPickSingleOne(CPlayer *pPlayer, class CItem *pItem, WORD index);

	// CD处理
	void StartItemCDTimer(const SItemBaseData *pItemData);
	int CheckItemCDTime(const SItemBaseData *pItemData);

	BOOL RecvForgeItem(struct SQStartForge *pMsg);							// 收到装备打造的消息
	BOOL RecvDragMetrail(struct SQDragMetrial *pMsg);						// 收到拖入材料消息
	BOOL RecvEndForgeItem(struct SQEndForge *pMsg);							// 收到取消打造消息
	BOOL RecvMakeHole(struct SQMakeHole *pMsg);								// 收到打孔消息
	BOOL RecvInLayJewel(struct SQInLayJewel *pMsg);							// 收到镶嵌消息
	BOOL RecvTakeoffJewel(struct SQTakeOffJewel *pMsg);						// 收到取宝石消息
	BOOL StartMakeHole();													// 开始打孔

	// ===================装备强化===================ahj	
	// 获取一个位置 分布情况,可能是包裹，可能是装备栏
	enum TItemPosType
	{
		EPackage_PosType = 0,  // 包裹
		EEquipColumn_PosType, // 装备栏
		EError_PosType, // 错误的位置
	};
	static const WORD sciEquipColumnBaseLine = 1000; // 装备栏的位置分割线
	static TItemPosType GetItemPosType(WORD wEquipPos);
	SEquipment * GetEquipmentbyItem(WORD wEquipPos, WORD LookType = XYD_FT_WHATEVER); // 通过位置获取装备(可能是从包裹中获取，也可能是从装备栏里面获取)	
	void CheckAndUpdateEquipColumnAttribyPos(CPlayer *pPlayer, WORD wEquipPos); // 通过通用位置检测并且更新装备栏的属性	

	WORD GetEquipColumnPos(WORD wEquipPos, bool bAddEquipColumnBaseLine); // 获取包裹栏的位置，注意是否需要添加装备栏基准线		
	BOOL IsLockedEquipColumnCell(WORD wEquipPos); // 检测装备栏是否被锁定了		
	BOOL LockEquipColumnCell(WORD wEquipPos, bool lock, bool sync);// 锁定或解锁定装备栏的格子
	// 精炼
	BOOL RecvDragStrengthenEquip(struct SQAddEquip* pMsg);					// 拖入装备
	BOOL RecvDragStrengthenMetrial(struct SQAddMetrial *pMsg);				// 拖入材料

	BOOL RecvBeginRefineGrade(struct SQBeginRefineGrade *pMsg);				// 升阶
	BOOL RecvBeginRefineStar(struct SQBeginRefineStar *pMsg);				// 升星
	BOOL RecvBeginReMoveStar(struct SQBeginReMoveStar *pMsg);				// 洗星	
	BOOL RecvBeginUpgradeQuality(struct SQStartQuality *pMsg);				// 升品质
	

	BOOL RecvBeginDecomposition(struct SQBeginIdentifyMsg* pMsg);			// 装备分解
	BOOL RecvBeginShengJie(struct SQBeginIdentifyMsg* pMsg);				// 装备升阶（提升附加属性的上限）
	BOOL RecvBeginJingLian(struct SQBeginIdentifyMsg *pMsg);				// 装备精炼（提升装备的附加属性值）			
	BOOL RecvBeginPolished(struct SQBeginIdentifyMsg *pMsg);				// 装备洗练（生成附加属性条目，随机没有锁定的附加属性值）
	BOOL RecvBeginSmelting(struct SQBeginIdentifyMsg* pMsg);				// 装备熔炼	
	BOOL RecvBeginSpiritAttachBody(struct SQBeginIdentifyMsg* pMsg);		// 装备灵附
		
	BOOL RecvIdentifyExchange(struct SQIdentifyExchangeMsg *pMsg);// [2012-8-24 10-11 gw: +收到洗练后的交换消息]
	
	BOOL RecvEndRefine(struct SQEndRefineMsg *pMsg);						// 结束强化

	// 用于升阶内部操作	
	BOOL GetUpgradeResult(SEquipment* pE, BYTE &result,bool bHaveLuckyCharmItem);

	// 取消洗点
	BOOL RecvEndResetPoint(struct SQEndResetPoint* pMsg);

	// 道具升级（目前只有有回元丹）
	BOOL RecvDragUpdateItem(struct SQDragUpdateItemMsg *pMsg);
	BOOL RecvDragUpdateMetrial(struct SQDragUpdateMetrialMsg *pMsg);
	BOOL RecvBeginUpdateItem(struct SQBeginUpdateItem *pMsg);
	BOOL RecvEndUpdateItem(struct SQEndUpdateItem *pMsg);
	BOOL BeginUpdateItem();		// 脚本回调函数

	// 装备增加套装属性
	BOOL RecvGetSuitAttributes(struct SQSuitAttributeMsg *pMsg);
	BOOL SendNeedMETRIALInfo(CPlayer *pPlayer,BYTE  bType);//发送需要强化的道具信息
	long OnGetNeedMetrial(CPlayer* pPlayer,DWORD NeedNum,std::vector<SPackageItem*> & vec,long ItemID, WORD LookType = XYD_FT_ONLYUNLOCK);//从背包中得到合适的道具集合
	long OnProcessNeedMetrial(CPlayer* pPlayer,std::vector<SPackageItem*> vec,long NeedNum,std::string str,bool& bHaveBindMetrial);
	void OnthSysNotice(WORD index);
public://（未做处理。。。）
	BOOL RecvRequireItemInfoMsg(struct SQRequireItemInfoMsg *pMsg);			// 客户要求获取装备信息
    BOOL RecvQueryUpdateItemData( struct QueryUpdateItemDataMsg *pMsg );	// 用户请求尝试获取某个道具的配置数据
	BOOL ToForge(BYTE byX, BYTE byY);
	BOOL ToFuse(BYTE byX, BYTE byY);
	void SendUseIntEffMsg(WORD type, DWORD time, WORD in_wKindOfCD);		// 暂时使用以前的方式传送消息

public:
	bool GetEquipment(BYTE btype,BYTE bIndex,SEquipment &equip); //获取一件装备数据 主要使用与lua
	bool SetEquipment(BYTE btype,BYTE bIndex,const SEquipment *sequip);//设置一件装备数据 主要使用与lua


	void OnGetIntensifyInfo(struct SQIntensifyInfo *pintensifyinfo);
	void SendIntensifyInfo(const struct SAIntensifyInfo *pIntensifyInfo);

	void OnGetQualityInfo(const struct SQEquipQualityInfo *pQualityInfo);
	void SendQualityInfo(const struct SAEquipQualityInfo *pQualityInfo,WORD msgsize);

	BOOL RecvBeginUpgradeLevel(struct SQBeginEquipLevelUpMsg *pMsg);		// 装备升级
	BOOL SendIntensifyResult(struct SABeginEquipLevelUpMsg *pMsg); //装备强化结果

	BOOL SendQualityResult(struct SAStartQuality *pMsg);//装备升阶结果
	BOOL OnQuestEquipSwitch(struct SQEquipSwitch* pMsg);//客户端请求切换武器

	BOOL DeleteItem(DWORD ItemID,WORD num);//删除道具
	void OnRecvSaleItem(struct SQItemSale *pMsg); //出售道具

	void OnRecvCleanColdTime(struct SQCleanColdtime *pMsg);
	void SendCleanColdTimeResult(struct SACleanColdtime *pMsg);

	void OnGetRiseStarInfo(const struct SQRiseStarInfo *pRiseStarInfo);  //请求升星描述信息
	void SendRiseStarInfo(const struct SARiseStarInfo *pRiseStarInfo, WORD msgsize); //发送升星描述信息
	void SendRefineStarResult(const struct SABeginRefineStar *pRefineStar,WORD msgsize);//发送升星结果
	void SendMsgToPlayer(const  LPVOID strarresule, WORD msgsize);

	void OnGetEquiptKeYinInfo(const struct SQKeYinInfo *pKeYinInfo); //请求刻印描述
	void SendEquiptKeYinInfo(const struct SAKeYinInfo *pKeYinInfo, WORD msgsize); //发送刻印描述
	void OnBeginKeYin(const  struct SQBeginKeYin *pBeginKeYinInfo); //请求开始刻印
	void SendKeYinResult(const  struct SABeginKeYin *pBeginKeYinInfo, WORD msgsize); //返回刻印结果
	void OnBeginKeYinChange(const struct SQKeYinChange *pBeginKeYinChange);//刻印转换
	void OnGetKeYinChangeInfo(const struct SQKeYinChangeInfo *pKeYinChangeInfo);//刻印转换信息
	void SendKeYinChangeData(const struct SAKeYinChange *pKeYinChange);//发送准备转换的刻印属性
	void OnBeginEquipJDing(const struct SQEquipJDing* pEquipJDing);//装备鉴定
	void SendEquipJDingResult(const struct SAEquipJDing *pEquipJDing,WORD msgsize);//装备鉴定结果
	void OnGetEquipJDingInfo(const struct SQEquipJDingInfo* pEquipJDingInfo);//请求装备鉴定数据
	void SendEquipJDingInfo(const struct SAEquipJDingInfo *pEquipJDingInfo, WORD msgsize);//发送装备鉴定数据
	void OnGetEquipJDingColor(const struct SQEquipJDingColor *pEquipJDingColor);//请求颜色信息
	void SendEquipJDingColor(const struct SAEquipJDingColor *pEquipJDingColor, WORD msgsize);//发送鉴定条颜色
	void OnGetSuitcondition(const struct SQSuitcondition *pSuitcondition);//请求套装条件
	void SendSuitcondition(const struct SASuitcondition *pSuitcondition,WORD msgsize);//发送套装条件
	void OnGetEquipPower(const struct SQEquipPower *pEquipPower);//获取装备战斗力
	void OnBuyPlayerTp(const struct SQBuyPlayerTp* pPlayerTp); //玩家购买体力

	
	//2014.2.20 add
	void OnReturnQiDaoMsg(struct SQQiDaoInfo *pMsg);	//返回祈祷所有信息
	void OnAddQiDaoAndReturn(struct SQAddQiDaoInfo *pMsg);	//接收客户端领取祈祷消息做相应的处理并返回结果
	//2014.2.21 add
	void OnReturnYuanBaoMsg(struct SQGiftPacketMsg *pMsg);		//返回元宝礼包中的装备信息
	void OnBueYuanBaoGift(struct SQBuyGiftPacketMsg *pMsg);		//购买元宝礼包中的装备

	//add 2014.3.5 ly
	void OnOpenSetSkillPlan(struct SQOpenSetSkillBaseMsg *pMsg);		//请求打开技能设置面板配置技能
	void OnUdtPlayerSkillAndPatern(struct SQUpdateSkillBaseMsg *pMsg);		//更新玩家当前所使用的技能和当前的形态

	//add by ly 2014/4/14
	void OnReturnCellCountDown(const struct SQCellCountDown *pMsg);	//请求背包格子倒计时时间


	void OnUpgradeSkill(struct SQUpgradeSkillBaseMsg *pMsg);		//升级技能
public:
    CItemUser(void);
    ~CItemUser(void);

	BOOL UpdateTimeRecycleItem();

public:
	///////////////////////////////////////////////////////////////////////////////////
	// 侠义道3道具系统（优化了性能和存储方式！）
	typedef std::map<DWORD, DWORD> ItemState;
	
	SItemWithFlag	m_PackageItems[PackageAllCells];	// 道具列表
	ItemState		m_ItemsState;						// 道具存在状态
	SEquipedWithFlag m_EquipedColumnFlag[EQUIP_P_MAX];		// 记录玩家装备栏的格子锁定状态

	DWORD			m_CommNpcGlobalID;					// 当前的NpcID
	WORD			m_CurShopID;						// 当前的店铺ID
	DWORD			ArrangeIntervalTime;				// 道具整理时间间隔
	SPackageItem	m_EmptyItem4Debug;					// 用于ASSERT验证用
	///////////////////////////////////////////////////////////////////////////////////
	std::vector<long> m_Blessvec;
public:
	// 等待购买消息结果回来，之后才允许再次购买
	BOOL m_bWaitBuyResult;

public:
	// 一组强化私有函数
	BOOL Equip_Is_Tyler(DWORD wIndex);				// 装备是否是新手装备
	BOOL RemoveRefineMetrial();						// 精炼到最大等级时，清除原材料

	void SendUpdateEquipInfo(SEquipment* equip);	// 背包显示信息更新.用于装备精炼
	BOOL SendNextLevelEquip(BYTE type);				// 得到要显示的强化后的装备信息	
	BOOL SendRefineEquipTheBestPreview(BYTE byType); // [锻造功能里发送极品预览]
	
	BOOL AddRefineAttribute(SEquipment* pEquip);	// 添加升阶和升星后的附加属性
	BOOL AddMaxExtraAttri(SEquipment* pEquip, BOOL bFillRefineAttri = FALSE);		// 添加升品和升级后的最大附加属性
	long AddStrengthenEquip(DWORD ItemId = 0);						// 升级升品质添加新装备

	static WORD calculateRefineStarSuccessRate(byte bStarNum, bool bHaveLuckyCharmItem=false); // 计算升星（强化）的成功率
	static BOOL IsReduceStarbyCalculateRefineStarFailed(byte byStarNum); // 判断强化失败之后是否减星
	
};
