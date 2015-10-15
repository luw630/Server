#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>

struct SItemBaseData;
class SItemFactorData;

struct SItemUpdateGradeInfo;
struct SItemUpgradeAttribute;
struct SItemUpdateQuality;	// 装备升品质
struct SEquipLevelUp;		// 装备升级
struct SEquipStar;			// 装备升星
struct SMaxExtraAttri;		// 附加属性最大值
struct SJewelAttribute;		// 宝石属性
struct SHuiYuanDanInfo;		// 回元丹增加属性配置表
struct SMakeHoleInfo;		// 打孔花费
struct SInsertGem;			// 镶嵌花费
struct STakeOffGem;			// 封洞花费
struct SDefineReset;
struct SEquipDecomposition; // 装备分解
struct SEquipSmelting;		// 装备熔炼
struct SEquipBestPreviewExtraAttri; // 装备极品预览附加属性
struct SEquipSpiritAttachBodyAttr; // 灵附属性
// 提供对道具相关数据的访问
// 1. 道具基本属性表
// 2. 道具加成属性表
// 3. 道具组CD表

struct SEquipExtraAttriTable
{
	DWORD EquipId;
	WORD  MinNum;
	WORD  MaxNum;
	int	  ExtraData[SEquipDataEx::EEA_MAX][3];
	DWORD ResetItemID;		//需要的重置道具ID
	BYTE  ResetNeedNum;		//重置需要的数量
	BYTE  ResetMoneyType;	//需要的钱类型	
	long  ResetNeedMoney;	//重置需要的钱	
	DWORD RefreshItemID;	//需要的刷新道具ID	
	BYTE  RefreshNeedNum;	//刷新需要的数量
	BYTE  RefreshMoneyType;	//刷新的钱类型	
	long  RefreshNeedMoney;	//刷新需要的钱	
	DWORD ExternLockedItemID; //可用锁定石ID
	byte  ExternLockeItemNum; // 锁定一条属性需要锁定石的数量
};



class CItemService
{	
	typedef std::hash_map<DWORD, SEquipExtraAttriTable* > EquipExtraAttriTable;
	typedef std::hash_map<DWORD, SItemBaseData*  > ItemBaseData;
	typedef std::hash_map<DWORD, SItemFactorData*> ItemFactorData;
	typedef std::hash_map<DWORD, DWORD> ItemGroupCD;
	typedef std::hash_map<BYTE, SItemUpdateGradeInfo*> ItemUpdateGradeTable;
	typedef std::hash_map<DWORD, SItemUpgradeAttribute*> ItemUpgradeAttribute;
	typedef std::hash_map<BYTE, SItemUpdateQuality*> ItemUpdateQuality;	// 升品质
	typedef	std::hash_map<BYTE,	SEquipLevelUp*>	EquipLevelUp;			// 升级
	typedef std::hash_map<BYTE, SEquipStar*> EquipStar;					// 装备升星
	typedef std::hash_map<DWORD, SMaxExtraAttri*> MaxExtraAttri;		// 装备最大随机属性值
	typedef std::hash_map<DWORD, SJewelAttribute*> JewelAttribute;		// 宝石属性值
	typedef	std::hash_map<DWORD, SHuiYuanDanInfo*> HuiYuanDanInfo;		// 回元丹属性配置表
	typedef std::hash_map<DWORD,  SMakeHoleInfo>	   MakeHoleInfo;	
	typedef std::hash_map<DWORD, SInsertGem>		   SInsertGemInfo;
	typedef std::hash_map<DWORD, STakeOffGem>		   STakeOffGemInfo;
	typedef std::hash_map<BYTE,  SDefineReset>		   SDefineResetInfo;
	typedef std::hash_map<DWORD,  SEquipDecomposition*>	   SEquipDecompositionInfo;
	typedef std::hash_map<BYTE, SEquipSmelting*>	SEquipSmeltingInfo;
	typedef std::hash_map<BYTE, SEquipBestPreviewExtraAttri*> SEquipBestPreviewExtraAttriInfo;
	typedef std::hash_map<WORD, SEquipSpiritAttachBodyAttr*> SEquipSpiritAttachBodyAttrInfo;

private:
	CItemService();

public:
	static CItemService& GetInstance()
	{
		static CItemService instance;
		return instance;
	}

	bool Init();
	bool ReLoad();
	void Clear();

	~CItemService();

	DWORD GetItemGroupCDTime(DWORD group) const;

	const SItemBaseData *GetItemBaseData(DWORD index) const;
	const SItemFactorData *GetItemFactorData(DWORD index) const;

	//const SEquipExtraAttriTable *GetExtraAttriTable(DWORD index) const;		// 装备附加属性
	const SItemUpdateGradeInfo *GetUpgradeInfo(BYTE grade) const;			// 装备升阶信息
	const SItemUpgradeAttribute *GetUpgradeAttribute(const struct SItemBaseData *pItemData) const; // 装备升阶附加属性
	const SItemUpgradeAttribute *GetUpgradeAttribute(DWORD ) const;			// 装备升阶附加属性
	const SItemUpdateQuality *GetUpdateQuality(BYTE color) const;			// 装备升品质
	const SEquipLevelUp	*GetLevelUpData(BYTE level)	const;					// 装备升级
	const SEquipStar *GetEquipStarData(BYTE num) const;						// 装备升星
	const SMaxExtraAttri *GetMaxExtraAttri(DWORD index) const;				// 装备最大随机属性配置表	
	const SMaxExtraAttri *GetMaxExtraAttri(const struct SItemBaseData *pItemData, byte byCurGrade) const; // 获取升阶的附加属性上限的读取
	const SJewelAttribute *GetJewelAttribute(DWORD index) const;			// 宝石的附加属性
	const SHuiYuanDanInfo *GetHuiYuanDanInfo(DWORD index) const;			// 回元丹增加真气表
	const SMakeHoleInfo *GetMakeHoleInfo(DWORD index) const;
	const SInsertGem *GetInsertGemInfo(DWORD index) const;
	const STakeOffGem *GetSTakeOffGemInfo(DWORD index) const;	
	const SEquipDecomposition *GetSEquipDecompositionInfo(const struct SItemBaseData *pItemData, byte byStarNum) const; // 获取装备的分解属性
	const SEquipSmelting *GetSEquipSmeltingInfo(byte byColor) const; // 获取装备熔炼配置属性	
	const SEquipBestPreviewExtraAttri *GetSEquipBestPreviewExtraAttri(byte byEquipType) const; // 获取装备极品预览附加属性信息

	WORD GetSuitEquipIDbyScrollID(DWORD dwScrollID) const; // 通过卷轴ID查找对应的套装ID
	const SEquipSpiritAttachBodyAttr *GetSEquipSpiritAttachBodyAttri(WORD wSuitEquipID) const;// 获取灵附套装信息

	bool LoadExtraAttri();
	long GetRandomNum(BYTE Color,long RandomNum);
	// 发送信息结构到客户端
	BOOL SendItemInfo(DNID dnidClient);

	bool GetFactorData(DWORD item, WORD &attr,WORD &nvalue,WORD nIndex);
private:
	// 配置道具属性加成
	void SetFactorValue(DWORD item, WORD attri, int value, bool IsPre);
	
	// 禁止拷贝构造和拷贝赋值
	CItemService(CItemService &);
	CItemService& operator=(CItemService &);

private:
	ItemBaseData	m_baseData;			// 道具的基本属性
	ItemFactorData	m_factorData;		// 道具的加成属性
	ItemGroupCD		m_groupCD;			// 组CD
	EquipExtraAttriTable	m_EATable;
	ItemUpdateGradeTable	m_gradeTalbe;		// 装备升阶信息
	ItemUpgradeAttribute	m_upgradeAttri;		// 装备升阶的附加属性
	ItemUpdateQuality		m_updateQuality;	// 装备升品质
	EquipLevelUp			m_EquipLevelUp;		// 装备升级
	EquipStar				m_EquipStar;		// 装备升星

	MaxExtraAttri			m_MaxAttri;			// 最大随机属性
	JewelAttribute			m_JewAttri;			// 宝石属性
	HuiYuanDanInfo			m_HYDInfo;			// 回元丹信息
	MakeHoleInfo			m_MakeHoleInfo;
	SInsertGemInfo			m_SInsertGemInfo;
	STakeOffGemInfo         m_STakeOffGemInfo;
	SDefineResetInfo        m_SDefineResetInfo; // 
	SEquipDecompositionInfo m_SEquipDecompositionInfo; // 装备分解配置
	SEquipSmeltingInfo		m_SEquipSmeltingInfo; // 装备熔炼配置
	SEquipBestPreviewExtraAttriInfo m_SEquipBestPreviewExtraAttri; // 装备极品预览附加属性配置
	SEquipSpiritAttachBodyAttrInfo	m_SEquipSpiritAttachBodyAttri; // 灵附相关的处理
};

#pragma warning(pop)