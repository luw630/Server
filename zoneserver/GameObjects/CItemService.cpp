#include "Stdafx.h"
#include <utility>
#include "CItemDefine.h"
#include "CItemService.h"
#include "ScriptManager.h"
#include "Player.h"

#pragma warning(push)
#pragma warning(disable:4996)

CItemService::CItemService() {}

CItemService::~CItemService() { Clear(); }

void CItemService::Clear()
{
	// 清除道具配置信息
	for (ItemBaseData::iterator it = m_baseData.begin(); it != m_baseData.end(); ++it)
	{
		SItemBaseData *temp = it->second;
		delete temp;
	}

	// 清除道具属性加成配置信息
	for (ItemFactorData::iterator it = m_factorData.begin(); it != m_factorData.end(); ++it)
	{
		SItemFactorData *temp = it->second;
		delete temp;
	}

	// 清除装备随机附加属性
	for (EquipExtraAttriTable::iterator iter = m_EATable.begin(); iter != m_EATable.end(); ++iter)
	{
		SEquipExtraAttriTable *temp = iter->second;
		delete temp;
	}

	// 清除装备升阶信息
	for (ItemUpdateGradeTable::iterator iter = m_gradeTalbe.begin(); iter != m_gradeTalbe.end(); ++iter)
	{
		SItemUpdateGradeInfo *temp = iter->second;
		delete temp;
	}

	// 装备升阶精炼增加属性配置表
	for (ItemUpgradeAttribute::iterator iter = m_upgradeAttri.begin(); iter != m_upgradeAttri.end(); ++iter)
	{
		SItemUpgradeAttribute *temp = iter->second;
		delete temp;
	}

	// 清除装备升品质
	for (ItemUpdateQuality::iterator iter = m_updateQuality.begin(); iter != m_updateQuality.end(); ++iter)
	{
		SItemUpdateQuality *temp = iter->second;
		delete	temp;
	}

	// 清除装备升级信息
	for (EquipLevelUp::iterator iter = m_EquipLevelUp.begin(); iter != m_EquipLevelUp.end(); ++iter)
	{
		SEquipLevelUp *temp = iter->second;
		delete	temp;
	}

	// 清除装备升星
	for (EquipStar::iterator iter = m_EquipStar.begin(); iter != m_EquipStar.end(); ++iter)
	{
		SEquipStar *temp = iter->second;
		delete temp;
	}

	// 清除最大随机属性值配置表
	for (MaxExtraAttri::iterator iter = m_MaxAttri.begin(); iter != m_MaxAttri.end(); ++iter)
	{
		SMaxExtraAttri *temp = iter->second;
		delete temp;
	}

	// 清除宝石属性值
	for (JewelAttribute::iterator iter = m_JewAttri.begin(); iter != m_JewAttri.end(); ++iter)
	{
		SJewelAttribute *temp = iter->second;
		delete temp;
	}

	// 清除回元丹信息
	for (HuiYuanDanInfo::iterator iter = m_HYDInfo.begin(); iter != m_HYDInfo.end(); ++iter)
	{
		SHuiYuanDanInfo *temp = iter->second;
		delete temp;
	}
	// 清除装备分解信息
	for (SEquipDecompositionInfo::iterator iter = m_SEquipDecompositionInfo.begin(); iter != m_SEquipDecompositionInfo.end(); ++iter)
	{
		SEquipDecomposition *temp = iter->second;
		delete temp;
	}
	// 清除装备熔炼信息
	for (SEquipSmeltingInfo::iterator iter = m_SEquipSmeltingInfo.begin(); iter != m_SEquipSmeltingInfo.end(); ++iter)
	{
		SEquipSmelting *temp = iter->second;
		delete temp;
	}
	// 清楚装备极品预览附加属性信息
	for (SEquipBestPreviewExtraAttriInfo::iterator iter = m_SEquipBestPreviewExtraAttri.begin(); iter != m_SEquipBestPreviewExtraAttri.end(); ++iter)
	{
		SEquipBestPreviewExtraAttri *temp = iter->second;
		delete temp;
	}
	// 清楚灵附套装属性信息
	for (SEquipSpiritAttachBodyAttrInfo::iterator iter = m_SEquipSpiritAttachBodyAttri.begin(); iter != m_SEquipSpiritAttachBodyAttri.end(); ++iter)
	{
		SEquipSpiritAttachBodyAttr *temp = iter->second;
		delete temp;
	}

	m_SEquipSpiritAttachBodyAttri.clear();
	m_SEquipBestPreviewExtraAttri.clear();
	m_SEquipSmeltingInfo.clear();
	m_SEquipDecompositionInfo.clear();
	m_HYDInfo.clear();
	m_JewAttri.clear();
	m_MaxAttri.clear();
	m_EquipStar.clear();
	m_EquipLevelUp.clear();
	m_upgradeAttri.clear();
	m_baseData.clear();
	m_factorData.clear();
	m_groupCD.clear();
	m_EATable.clear();
	m_gradeTalbe.clear();

	m_updateQuality.clear();
}

bool CItemService::ReLoad()
{
	Clear();
	Init();

	return true;
}

bool CItemService::Init()
{
	dwt::ifstream ItemStream("Data\\道具\\道具表.txt");
	if (!ItemStream.is_open())
	{
		return false;
	}

	dwt::ifstream GroupSteam("Data\\道具\\道具冷却组.txt");
	if (!GroupSteam.is_open())
	{
		ItemStream.close();
		return false;
	}

	dwt::ifstream FactStream("Data\\道具\\初始属性表.txt");
	if (!FactStream.is_open())
	{
		ItemStream.close();
		GroupSteam.close();
		return false;
	}

	dwt::ifstream ItemGradeStream("Data\\道具\\装备升阶配置表.txt");
	if (!ItemGradeStream.is_open())
	{
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
	}

	dwt::ifstream ItemUpgradeAttriStream("Data\\道具\\装备升阶精练增加属性.txt");
	if (!ItemUpgradeAttriStream.is_open())
	{
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
		ItemGradeStream.close();
	}

	dwt::ifstream ItemUpdateQualityStream("Data\\道具\\装备品质配置表.txt");
	if (!ItemUpdateQualityStream.is_open())
	{
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
		ItemGradeStream.close();
		ItemUpgradeAttriStream.close();
	}

	dwt::ifstream EquipLevelUpStream("Data\\道具\\装备升级配置表.txt");
	if (!EquipLevelUpStream.is_open())
	{
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
		ItemGradeStream.close();
		ItemUpgradeAttriStream.close();
		ItemUpdateQualityStream.close();
	}

	dwt::ifstream EquipStarStream("Data\\道具\\装备升星配置表.txt");
	if (!EquipStarStream.is_open())
	{
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
		ItemGradeStream.close();
		ItemUpgradeAttriStream.close();
		ItemUpdateQualityStream.close();
		EquipLevelUpStream.close();
	}

	dwt::ifstream MaxAttriStream("Data\\道具\\装备最大附加属性配置表.txt");
	if (!MaxAttriStream.is_open())
	{
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
		ItemGradeStream.close();
		ItemUpgradeAttriStream.close();
		ItemUpdateQualityStream.close();
		EquipLevelUpStream.close();
		EquipStarStream.close();
	}

	dwt::ifstream JwlAttriStream("Data\\道具\\宝石属性配置表.txt");
	if (!JwlAttriStream.is_open())
	{
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
		ItemGradeStream.close();
		ItemUpgradeAttriStream.close();
		ItemUpdateQualityStream.close();
		EquipLevelUpStream.close();
		EquipStarStream.close();
		MaxAttriStream.close();
	}

	dwt::ifstream HYDStream("Data\\道具\\回神丹增加真气表.txt");
	if (!HYDStream.is_open())
	{
		JwlAttriStream.close();
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
		ItemGradeStream.close();
		ItemUpgradeAttriStream.close();
		ItemUpdateQualityStream.close();
		EquipLevelUpStream.close();
		EquipStarStream.close();
		MaxAttriStream.close();
	}
	dwt::ifstream HoleStream("Data\\道具\\打孔花费.txt");
	if (!HoleStream.is_open())
	{
		HYDStream.close();
		JwlAttriStream.close();
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
		ItemGradeStream.close();
		ItemUpgradeAttriStream.close();
		ItemUpdateQualityStream.close();
		EquipLevelUpStream.close();
		EquipStarStream.close();
		MaxAttriStream.close();
	}
	dwt::ifstream GemStream("Data\\道具\\镶嵌花费.txt");
	if (!GemStream.is_open())
	{
		HoleStream.close();
		HYDStream.close();
		JwlAttriStream.close();
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
		ItemGradeStream.close();
		ItemUpgradeAttriStream.close();
		ItemUpdateQualityStream.close();
		EquipLevelUpStream.close();
		EquipStarStream.close();
		MaxAttriStream.close();
	}
	dwt::ifstream TakeOffStream("Data\\道具\\摘除花费.txt");
	if (!TakeOffStream.is_open())
	{
		GemStream.close();
		HoleStream.close();
		HYDStream.close();
		JwlAttriStream.close();
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
		ItemGradeStream.close();
		ItemUpgradeAttriStream.close();
		ItemUpdateQualityStream.close();
		EquipLevelUpStream.close();
		EquipStarStream.close();
		MaxAttriStream.close();
	}
	dwt::ifstream DefineResetStream("Data\\道具\\鉴定重置几率.txt");
	if (!DefineResetStream.is_open())
	{
		TakeOffStream.close();
		GemStream.close();
		HoleStream.close();
		HYDStream.close();
		JwlAttriStream.close();
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
		ItemGradeStream.close();
		ItemUpgradeAttriStream.close();
		ItemUpdateQualityStream.close();
		EquipLevelUpStream.close();
		EquipStarStream.close();
		MaxAttriStream.close();
	}

	dwt::ifstream EquipmentDecompositionStream("Data\\道具\\装备分解.txt");
	if (!EquipmentDecompositionStream.is_open())
	{
		DefineResetStream.close();
		TakeOffStream.close();
		GemStream.close();
		HoleStream.close();
		HYDStream.close();
		JwlAttriStream.close();
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
		ItemGradeStream.close();
		ItemUpgradeAttriStream.close();
		ItemUpdateQualityStream.close();
		EquipLevelUpStream.close();
		EquipStarStream.close();
		MaxAttriStream.close();
	}
	
	dwt::ifstream EquipSmeltingStream("Data\\道具\\装备熔炼.txt");
	if (!EquipSmeltingStream.is_open())
	{
		EquipmentDecompositionStream.close();
		DefineResetStream.close();
		TakeOffStream.close();
		GemStream.close();
		HoleStream.close();
		HYDStream.close();
		JwlAttriStream.close();
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
		ItemGradeStream.close();
		ItemUpgradeAttriStream.close();
		ItemUpdateQualityStream.close();
		EquipLevelUpStream.close();
		EquipStarStream.close();
		MaxAttriStream.close();
	}

	dwt::ifstream EquipBestPreviewStream("Data\\道具\\极品预览装备附加属性.txt");
	if (!EquipBestPreviewStream.is_open())
	{
		EquipSmeltingStream.close();
		EquipmentDecompositionStream.close();
		DefineResetStream.close();
		TakeOffStream.close();
		GemStream.close();
		HoleStream.close();
		HYDStream.close();
		JwlAttriStream.close();
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
		ItemGradeStream.close();
		ItemUpgradeAttriStream.close();
		ItemUpdateQualityStream.close();
		EquipLevelUpStream.close();
		EquipStarStream.close();
		MaxAttriStream.close();
	}

	dwt::ifstream EquipSpiritAttachBodyStream("Data\\道具\\装备灵附.txt");
	if (!EquipSpiritAttachBodyStream.is_open())
	{
		EquipSpiritAttachBodyStream.close();
		EquipSmeltingStream.close();
		EquipmentDecompositionStream.close();
		DefineResetStream.close();
		TakeOffStream.close();
		GemStream.close();
		HoleStream.close();
		HYDStream.close();
		JwlAttriStream.close();
		ItemStream.close();
		GroupSteam.close();
		FactStream.close();
		ItemGradeStream.close();
		ItemUpgradeAttriStream.close();
		ItemUpdateQualityStream.close();
		EquipLevelUpStream.close();
		EquipStarStream.close();
		MaxAttriStream.close();		
	}

	// 开始读取配置了~~~
	char buffer[2048];

	SItemUpdateQuality *pQuality = 0;
	while (!ItemUpdateQualityStream.eof())
	{
		ItemUpdateQualityStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pQuality = new SItemUpdateQuality;

		DWORD temp = 0;
		pQuality->bColor		=	((linebuf >> temp), temp); temp = 0;
		pQuality->m_Type		=	((linebuf >> temp), temp); temp = 0;
		pQuality->dwNeedMoney	=	((linebuf >> temp), temp); temp = 0;
		for (size_t i = 0; i < IC_MAX - 1; ++i)
		{
			pQuality->rate[i]	=	((linebuf >> temp), temp); temp = 0;
		}
		pQuality->bStoneNum		=	((linebuf >> temp), temp); temp = 0;
		pQuality->NeedID		=	((linebuf >> temp), temp); temp = 0;
		pQuality->m_DestroyRate =   ((linebuf >> temp), temp); temp = 0;
		m_updateQuality.insert(make_pair(pQuality->bColor, pQuality));
	}

	SItemBaseData *pData = 0;
	while (!ItemStream.eof())
	{
		ItemStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pData = new SItemBaseData;

		linebuf >> pData->m_Name;
		
		DWORD temp = 0;
		pData->m_ID					= ((linebuf >> temp), temp); temp = 0;
		pData->m_ScriptID			= ((linebuf >> temp), temp); temp = 0;
		pData->m_StepNum			= ((linebuf >> temp), temp); temp = 0;
		pData->m_Type				= ((linebuf >> temp), temp); temp = 0;
		pData->m_ModelIDMan			= ((linebuf >> temp), temp); temp = 0;
		pData->m_ModelIDWoman		= ((linebuf >> temp), temp); temp = 0;
		pData->m_MaxWear			= ((linebuf >> temp), temp); temp = 0;
		pData->m_WearChange			= ((linebuf >> temp), temp); temp = 0;
		pData->m_WearReduceCount	= ((linebuf >> temp), temp); temp = 0;
		pData->m_Overlay			= ((linebuf >> temp), temp); temp = 0;
		pData->m_BuyMoneyType		= ((linebuf >> temp), temp); temp = 0;
		pData->m_BuyPrice			= ((linebuf >> temp), temp); temp = 0;
		pData->m_SellMoneyType		= ((linebuf >> temp), temp); temp = 0;
		pData->m_SellPrice			= ((linebuf >> temp), temp); temp = 0;
		pData->m_School				= ((linebuf >> temp), temp); temp = 0;		
		pData->m_Level				= ((linebuf >> temp), temp); temp = 0;
		pData->m_Sex				= ((linebuf >> temp), temp); temp = 0;
		pData->m_CanQuality			= ((linebuf >> temp), temp); temp = 0;//升品质(锻造)
		pData->m_CanGrade			= ((linebuf >> temp), temp); temp = 0;//可否升级
		pData->m_CanRefine			= ((linebuf >> temp), temp); temp = 0;//鉴定
		pData->m_CanRefresh			= ((linebuf >> temp), temp); temp = 0;//刷新
		pData->m_CanStar			= ((linebuf >> temp), temp); temp = 0;
		pData->m_CanUpdate			= ((linebuf >> temp), temp); temp = 0;
		pData->m_CanMakeHole		= ((linebuf >> temp), temp); temp = 0;
		pData->m_CanTakeOff			= ((linebuf >> temp), temp); temp = 0;
		pData->m_CanUnStar			= ((linebuf >> temp), temp); temp = 0;
		pData->m_CanDrop			= ((linebuf >> temp), temp); temp = 0;
		pData->m_CanSale			= ((linebuf >> temp), temp); temp = 0;
		pData->m_CanTran			= ((linebuf >> temp), temp); temp = 0;
		pData->m_CanFix				= ((linebuf >> temp), temp); temp = 0;
		pData->m_CanStroage			= ((linebuf >> temp), temp); temp = 0;	
		pData->m_DieDropRate		= ((linebuf >> temp), temp); temp = 0;	
		pData->m_CanUse				= ((linebuf >> temp), temp); temp = 0;	
		pData->m_CanJianDing		= ((linebuf >> temp), temp); temp = 0;
		pData->m_LocSrvCoolingTime	= ((linebuf >> temp), temp); temp = 0;
		pData->m_LocSrvCDGroupID	= ((linebuf >> temp), temp); temp = 0;
		pData->m_LocSrvPreTime		= ((linebuf >> temp), temp); temp = 0;
		pData->m_Color				= ((linebuf >> temp), temp); temp = 0;
		pData->m_TaskID				= ((linebuf >> temp), temp); temp = 0;
		pData->m_TaskNeed			= ((linebuf >> temp), temp); temp = 0;
		linebuf >> temp;
		linebuf >> temp;
		pData->m_GemType			= ((linebuf >> temp), temp); temp = 0;
		pData->m_BindType			= ((linebuf >> temp), temp); temp = 0; //绑定类型
		// [2012-9-1 15-27 gw: +todo..装备可否分解]
		pData->m_CanEquipDescomposition = true;//((linebuf >> temp), temp); temp = 0;
		pData->m_CanEquipSmelting = true; //((linebuf >> temp), temp);temp = 0;
		std::pair<ItemBaseData::iterator, bool> result;
		result = m_baseData.insert(make_pair(pData->m_ID, pData));
		if (pData->m_ID == 0)
		{
			rfalse("【Data\\道具\\道具表.txt】发现了一个等于0道具号！%d", pData->m_ID);
			delete pData;
		}

		if (false == result.second)
		{
			rfalse("【Data\\道具\\道具表.txt】发现了一个重复的道具号！%d", pData->m_ID);
			delete pData;
		}

		if (0 == pData->m_Overlay)
		{
			rfalse("【Data\\道具\\道具表.txt】道具%d叠加上限为0~", pData->m_ID);
			delete pData;
		}

		if (0 == pData->m_ID)
		{
			rfalse("【Data\\道具\\道具表.txt】发现了一个道具ID为0！！！！");
			delete pData;
		}
	}

	while (!GroupSteam.eof())
	{
		GroupSteam.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));

		DWORD GroupID;
		DWORD CDTime;

		linebuf >> GroupID >> CDTime;

		MY_ASSERT(GroupID >= 2000 && GroupID <= 2999 && CDTime);

		m_groupCD[GroupID] = CDTime;
	}

	while (!FactStream.eof())
	{
		FactStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;
		
		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		
		DWORD itemID = 0;
		linebuf >> itemID;

// 		int vValue, pValue;
// 		for (size_t i = 0; i < SEquipDataEx::EEA_MAX; i++)
// 		{
// 			linebuf >> vValue >> pValue;
// 			SetFactorValue(itemID, i, vValue, false);
// 			SetFactorValue(itemID, i, pValue, true);
// 		}
		int vType, vValue;
		for (size_t i = 0; i < SEquipDataEx::EEA_MAX; i++)
		{
			linebuf >> vType >> vValue;
			SetFactorValue(itemID, vType, vValue, false);
		}
	}

	// 装备升阶配置表的读取
	SItemUpdateGradeInfo *pUpgradeData;
	while (!ItemGradeStream.eof())
	{
		ItemGradeStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
		{
			continue;
		}

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));

		pUpgradeData = new SItemUpdateGradeInfo;
		DWORD temp = 0;

		pUpgradeData->grade							= ((linebuf >> temp), temp); temp = 0;
		pUpgradeData->baseSuccessRate				= ((linebuf >> temp), temp); temp = 0;
		pUpgradeData->baseFailedReduceGradeRate		= ((linebuf >> temp), temp); temp = 0;
		pUpgradeData->baseFailedReduceGradeRate		= ((linebuf >> temp), temp); temp = 0;
		pUpgradeData->byMoneyType					= ((linebuf >> temp), temp); temp = 0;
		pUpgradeData->needMoney						= ((linebuf >> temp), temp); temp = 0;
		pUpgradeData->dwNeedItemID					= ((linebuf >> temp), temp); temp = 0;
		pUpgradeData->byNeedItemNum					= ((linebuf >> temp), temp); temp = 0;		

		pUpgradeData->dwLuckyCharmID				= ((linebuf >> temp), temp); temp = 0;		
		pUpgradeData->byLuckyCharmNum				= ((linebuf >> temp), temp); temp = 0;		
		pUpgradeData->wLuckyCharmAddSuccessRate		= ((linebuf >> temp), temp); temp = 0;		
		
	
		m_gradeTalbe.insert(make_pair(pUpgradeData->grade, pUpgradeData));
	}

	// 装备升阶升星附加属性配置表
	SItemUpgradeAttribute *pUpgradeAttri;
	while (!ItemUpgradeAttriStream.eof())
	{
		ItemUpgradeAttriStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
		{
			continue;
		}

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));

		pUpgradeAttri = new SItemUpgradeAttribute;
		DWORD temp = 0;

		pUpgradeAttri->wID				= ((linebuf >> temp), temp); temp = 0;
		pUpgradeAttri->wType			= ((linebuf >> temp), temp); temp = 0;
		pUpgradeAttri->bLevel			= ((linebuf >> temp), temp); temp = 0;
		pUpgradeAttri->bColor			= ((linebuf >> temp), temp); temp = 0;
	
		// 读取升阶的附加属性信息
		for (size_t i = 0; i < SItemUpgradeAttribute::msc_iStarAttriDataNum; ++i)
		{
			pUpgradeAttri->wAttri[i]	= ((linebuf >> temp), temp); temp = 0;
		}

		// 读取升星的附加属性信息
		for (size_t j = 0; j < SEquipment::MAX_STAR_NUM; ++j)
		{
			pUpgradeAttri->wStarAttri[j] = ((linebuf >> temp), temp); temp = 0;
		}

		// 获取装备精炼洗练的一些相关需求数据
        // 附加属性的基本数据		
		for (size_t i = 0; i < SEquipDataEx::EEA_MAX; ++i)
		{
			for (size_t j = 0; j < SEquipExtraAttriRequire::EExtraData_Max; ++j)
			{
				pUpgradeAttri->equipRefineRequire.iaExtraData[i][j] = ((linebuf >> temp), temp); temp = 0;
			}
		}		
		// 洗练出附加属性条数的概率
		for (size_t i = 0; i < SEquipExtraAttriRequire::msc_iXiLianExtraAttriMax; ++i)
		{
			pUpgradeAttri->equipRefineRequire.waExtraAttriRate[i] = ((linebuf >> temp), temp); temp = 0;	
		}
		// 洗练的需求配置，注意添加值在iaExtraData里
		pUpgradeAttri->equipRefineRequire.ResetItemID = ((linebuf >> temp), temp); temp = 0;		
		pUpgradeAttri->equipRefineRequire.ResetNeedNum = ((linebuf >> temp), temp); temp = 0;		
		pUpgradeAttri->equipRefineRequire.ResetNeedMoney = ((linebuf >> temp), temp); temp = 0;	
		pUpgradeAttri->equipRefineRequire.ExternLockedItemID = ((linebuf >> temp), temp); temp = 0;
		pUpgradeAttri->equipRefineRequire.ExternLockeItemNum = ((linebuf >> temp), temp); temp = 0;
		// 精炼的需求配置，注意添加值在iaExtraData里
		pUpgradeAttri->equipRefineRequire.RefreshItemID = ((linebuf >> temp), temp); temp = 0;	
		pUpgradeAttri->equipRefineRequire.RefreshNeedNum = ((linebuf >> temp), temp); temp = 0;	
		pUpgradeAttri->equipRefineRequire.RefreshNeedMoney = ((linebuf >> temp), temp); temp = 0;	

		m_upgradeAttri.insert(make_pair(pUpgradeAttri->wID, pUpgradeAttri));
	}

	// 装备升级配置表的读取
	SEquipLevelUp *pEquipLevelUp;
	while(!EquipLevelUpStream.eof())
	{
		EquipLevelUpStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;
		
		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));

		pEquipLevelUp = new SEquipLevelUp;
		DWORD temp = 0;

		pEquipLevelUp->level			= ((linebuf >> temp), temp); temp = 0;
		pEquipLevelUp->StoneNum			= ((linebuf >> temp), temp); temp = 0;
		pEquipLevelUp->SuccessRate		= ((linebuf >> temp), temp); temp = 0;
		pEquipLevelUp->NeedMoney		= ((linebuf >> temp), temp); temp = 0;
		pEquipLevelUp->m_Type			= ((linebuf >> temp), temp); temp = 0;
		pEquipLevelUp->NeedID			= ((linebuf >> temp), temp); temp = 0;
		pEquipLevelUp->m_DestroyRate    = ((linebuf >> temp), temp); temp = 0; 
		m_EquipLevelUp.insert(make_pair(pEquipLevelUp->level, pEquipLevelUp));
	}

	// 装备升星配置表
	SEquipStar *pEquipStar;
	while(!EquipStarStream.eof())
	{
		EquipStarStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));

		pEquipStar = new SEquipStar;
		DWORD temp = 0;

		pEquipStar->num		= ((linebuf >> temp), temp); temp = 0;
		pEquipStar->Rate	= ((linebuf >> temp), temp); temp = 0;
		pEquipStar->StarMoneyType = ((linebuf >> temp), temp); temp = 0;
		pEquipStar->money	= ((linebuf >> temp), temp); temp = 0;
		pEquipStar->StarNeedID = ((linebuf >> temp), temp); temp = 0;
		pEquipStar->StarNum	= ((linebuf >> temp), temp); temp = 0;		
		pEquipStar->ReMoveStarMoneyType = ((linebuf >> temp), temp); temp = 0;
		pEquipStar->ReMovemoney = ((linebuf >> temp), temp); temp = 0;
		pEquipStar->ReMoveStarNeedID = ((linebuf >> temp), temp); temp = 0;
		pEquipStar->RemoveStarNum = ((linebuf >> temp), temp); temp = 0;
		pEquipStar->bNeedReduceStarFailed = ((linebuf >> temp), temp); temp = 0;
		pEquipStar->dwProtectionCharacterID = ((linebuf >> temp), temp); temp = 0;
		pEquipStar->byProtectionCharacterNum = ((linebuf >> temp), temp); temp = 0;
		pEquipStar->dwLuckyCharmID = ((linebuf >> temp), temp); temp = 0;
		pEquipStar->byLuckyCharmNum = ((linebuf >> temp), temp); temp = 0;
		pEquipStar->wLuckyCharmAddSuccessRate = ((linebuf >> temp), temp); temp = 0;
 		m_EquipStar.insert(make_pair(pEquipStar->num, pEquipStar));
	}
	
	// 最大属性读取
	SMaxExtraAttri *pMaxExtraAttri;
	while (!MaxAttriStream.eof())
	{
		MaxAttriStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));

		pMaxExtraAttri = new SMaxExtraAttri;
		DWORD temp = 0;

		pMaxExtraAttri->index	= ((linebuf >> temp), temp); temp = 0;
		pMaxExtraAttri->level	= ((linebuf >> temp), temp); temp = 0;
		pMaxExtraAttri->color	= ((linebuf >> temp), temp); temp = 0;
		pMaxExtraAttri->grade	= ((linebuf >> temp), temp); temp = 0;

		for (size_t i = 0; i < SEquipment::EEA_MAX; ++i)
		{
			pMaxExtraAttri->ExtraData[i] = ((linebuf >> temp), temp); temp = 0;
		}

		m_MaxAttri.insert(make_pair(pMaxExtraAttri->index, pMaxExtraAttri));
	}

	// 宝石属性读取
	SJewelAttribute *pJwlAttri;
	while (!JwlAttriStream.eof())
	{
		JwlAttriStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));

		pJwlAttri = new SJewelAttribute;
		DWORD temp = 0;

		pJwlAttri->index	= ((linebuf >> temp), temp); temp = 0;
		//pJwlAttri->type		= ((linebuf >> temp), temp); temp = 0;

		for (size_t i = 0; i < SEquipDataEx::EEA_MAX; ++i)
		{
			pJwlAttri->ExtraData[i] = ((linebuf >> temp), temp); temp = 0;
		}

		m_JewAttri.insert(make_pair(pJwlAttri->index, pJwlAttri));
	}

	// 回元丹信息
	SHuiYuanDanInfo *pHYDInfo;
	while (!HYDStream.eof())
	{
		HYDStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));

		pHYDInfo = new SHuiYuanDanInfo;
		DWORD temp = 0;
	
		pHYDInfo->wLevel = ((linebuf >> temp), temp); temp = 0;
		pHYDInfo->wSPValue = ((linebuf >> temp), temp); temp = 0;
		pHYDInfo->wSuccessRate = ((linebuf >> temp), temp); temp = 0;
		pHYDInfo->wLianzhiRate = ((linebuf >> temp), temp); temp = 0;
		pHYDInfo->wXMTJRate = ((linebuf >> temp), temp); temp = 0;

		m_HYDInfo.insert(make_pair(pHYDInfo->wLevel, pHYDInfo));
	}
	while (!HoleStream.eof())
	{
		HoleStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;
		long temp = 0;
		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		SMakeHoleInfo SMK;	
		SMK.Index			=	((linebuf >> temp), temp); temp = 0;			//打孔编号ID	
		SMK.HoleNeedID		=	((linebuf >> temp), temp); temp = 0;		//打孔需要的材料ID	
		SMK.HoleNum			=	((linebuf >> temp), temp); temp = 0;		//消耗材料数量	
		SMK.HoleMoneyType	=	((linebuf >> temp), temp); temp = 0;	//打孔钱类型
		SMK.HoleMoney		=	((linebuf >> temp), temp); temp = 0;		//打孔钱数量
		SMK.wMakeSuccessRate =	((linebuf >> temp), temp); temp = 0;
		SMK.dwLuckyCharmID =	((linebuf >> temp), temp); temp = 0; // 升星可用幸运符ID
		SMK.byLuckyCharmNum =	((linebuf >> temp), temp); temp = 0; // 可用幸运符扣除数量
		SMK.wLuckyCharmAddSuccessRate =	((linebuf >> temp), temp); temp = 0; //幸运符增加的成功率
		SMK.dwProtectionCharacterID =	((linebuf >> temp), temp); temp = 0; // 升星可用保护符ID
		SMK.byProtectionCharacterNum =	((linebuf >> temp), temp); temp = 0; // 可用保护符扣除数量
		SMK.wReduceHoleRate = ((linebuf >> temp), temp); temp = 0; // 失败后降孔的概率
		
		m_MakeHoleInfo.insert(make_pair(SMK.Index,SMK));
	}
	while (!GemStream.eof())
	{
		GemStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;
		long temp = 0;
		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		SInsertGem SIG;
		SIG.Index			=	((linebuf >> temp), temp); temp = 0;
		SIG.GemMoneyType	=	((linebuf >> temp), temp); temp = 0;
		SIG.GemMoney		=	((linebuf >> temp), temp); temp = 0;
		m_SInsertGemInfo.insert(make_pair(SIG.Index,SIG));
	}
	while (!TakeOffStream.eof())
	{
		TakeOffStream.getline(buffer,sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;
		long temp = 0;
		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		STakeOffGem STO;
		STO.Index					=	((linebuf >> temp), temp); temp = 0;							
		STO.TakeOffGemMoneyType		=	((linebuf >> temp), temp); temp = 0;	
		STO.TakeOffGemMoney			=	((linebuf >> temp), temp); temp = 0;		
		STO.TakeOffGemItemID		=	((linebuf >> temp), temp); temp = 0;		
		STO.TakeOffGemNum			=	((linebuf >> temp), temp); temp = 0;		
		STO.TakeOffGemHoleRale		=	((linebuf >> temp), temp); temp = 0;
		m_STakeOffGemInfo.insert(make_pair(STO.Index,STO));
	}
	while (!DefineResetStream.eof()) 
	{
		DefineResetStream.getline(buffer,sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;
		long temp = 0;
		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		SDefineReset STO;
		STO.m_Color					=	((linebuf >> temp), temp); temp = 0;
		long tek = 0;
		for (int i = 0; i < 20; ++i)
		{
			STO.m_Rate[i] = ((linebuf >> temp), temp); temp = 0;
			tek  += STO.m_Rate[i];
		}
		if (tek != 100)
		{
			rfalse(2,1,"鉴定重置几率.txt != 100  %C",STO.m_Color);
			MY_ASSERT(0);
		}
		m_SDefineResetInfo.insert(make_pair(STO.m_Color,STO));
	}
	SEquipDecomposition *pSED = NULL;
	while (!EquipmentDecompositionStream.eof())
	{
		EquipmentDecompositionStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;
		long temp = 0;
		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pSED = new SEquipDecomposition();
		if (!pSED)
		{
			continue;
		}

		pSED->dwIndex = ((linebuf >> temp), temp); temp = 0;
		pSED->byColor = ((linebuf >> temp), temp); temp = 0;
		pSED->byStarNum = ((linebuf >> temp), temp); temp = 0;
		pSED->byMoneyType = ((linebuf >> temp), temp); temp = 0;
		pSED->lNeedMoney = ((linebuf >> temp), temp); temp = 0;
		pSED->dwGenNormalItemID = ((linebuf >> temp), temp); temp = 0;
		pSED->byGenNormalItemNum = ((linebuf >> temp), temp); temp = 0;
		pSED->wGenNormalItemRate = ((linebuf >> temp), temp); temp = 0;
		pSED->dwGenSpecialItemID = ((linebuf >> temp), temp); temp = 0;
		pSED->byGenSpecialItemNum = ((linebuf >> temp), temp); temp = 0;
		pSED->wGenSpecialItemRate = ((linebuf >> temp), temp); temp = 0;
		m_SEquipDecompositionInfo.insert(make_pair(pSED->dwIndex, pSED));
	}

	SEquipSmelting *pSES = NULL;
	while (!EquipSmeltingStream.eof())
	{
		EquipSmeltingStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;
		long temp = 0; 
		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pSES = new SEquipSmelting();
		if (!pSES)
		{
			continue;
		}
		pSES->byColor = ((linebuf >> temp), temp); temp = 0;
		pSES->byMoneyType = ((linebuf >> temp), temp); temp = 0;
		pSES->lNeedMoney = ((linebuf >> temp), temp); temp = 0;
		pSES->dwGenNormalItemID = ((linebuf >> temp), temp); temp = 0;
		pSES->byGenNormalItemNum = ((linebuf >> temp), temp); temp = 0;
		pSES->byGenNormalItemColor = ((linebuf >> temp), temp); temp = 0;
		pSES->byAttriFactor = ((linebuf >> temp), temp); temp = 0;
		pSES->byStarNum = ((linebuf >> temp), temp); temp = 0;
		m_SEquipSmeltingInfo.insert(make_pair(pSES->byColor, pSES));
	}
	SEquipBestPreviewExtraAttri *pSBP = NULL;
	while (!EquipBestPreviewStream.eof())
	{
		EquipBestPreviewStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;
		long temp = 0; 
		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pSBP = new SEquipBestPreviewExtraAttri();
		if (!pSBP)
		{
			continue;
		}
		pSBP->byEquipType = ((linebuf >> temp), temp); temp = 0;
		pSBP->byExtraAttriType = ((linebuf >> temp), temp); temp = 0;
		m_SEquipBestPreviewExtraAttri.insert(make_pair(pSBP->byEquipType, pSBP));
	}
	SEquipSpiritAttachBodyAttr *pSSABA = NULL;
	while (!EquipSpiritAttachBodyStream.eof())
	{
		EquipSpiritAttachBodyStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;
		long temp = 0;
		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pSSABA = new SEquipSpiritAttachBodyAttr();
		if (!pSSABA)
		{
			continue;
		}
		pSSABA->wSuitEquipID = ((linebuf >> temp), temp); temp = 0;
		pSSABA->dwNeedItemID = ((linebuf >> temp), temp); temp = 0;
		pSSABA->byNeedItemNum = ((linebuf >> temp), temp); temp = 0;
		pSSABA->byMoneyType = ((linebuf >> temp), temp); temp = 0;
		pSSABA->lNeedMoney = ((linebuf >> temp), temp); temp = 0;
		pSSABA->wLevelLimit = ((linebuf >> temp), temp); temp = 0;
		m_SEquipSpiritAttachBodyAttri.insert(make_pair(pSSABA->wSuitEquipID, pSSABA));
	}

	EquipSpiritAttachBodyStream.close();
	EquipBestPreviewStream.close();
	EquipSmeltingStream.close();
	EquipmentDecompositionStream.close();
	DefineResetStream.close();
	TakeOffStream.close();
	GemStream.close();
	HYDStream.close();
	EquipStarStream.close();
	ItemUpgradeAttriStream.close();
	ItemStream.close();
	GroupSteam.close();
	FactStream.close();
	ItemGradeStream.close();
	EquipLevelUpStream.close();
	ItemUpdateQualityStream.close();
	MaxAttriStream.close();
	JwlAttriStream.close();
	HoleStream.close();
	// 随机附加属性表初始化
	LoadExtraAttri();

	return true;
}

BOOL CItemService::SendItemInfo(DNID dnidClient)
{
	if (0 == m_HYDInfo.size())
		return FALSE;

	SAHuiShenDanListInfoMsg msg;
	memset(msg.buffer, 0, sizeof(msg.buffer));

	BYTE *pBuf = msg.buffer;

	// 首先压入数据条数
	WORD list_size = (WORD)m_HYDInfo.size();
	memcpy(pBuf, &list_size, sizeof(WORD));
	pBuf += sizeof(WORD);

	// 将回元丹信息结构由指针类型转换为数据类型
	for (size_t index = 0; index != m_HYDInfo.size(); ++index)
	{
		HuiYuanDanInfo::iterator iter = m_HYDInfo.find(index);
		if (iter == m_HYDInfo.end())
			continue;

		memcpy(pBuf, &iter->second->wLevel, sizeof(WORD));
		pBuf += sizeof(WORD);

		memcpy(pBuf, &iter->second->wSPValue, sizeof(WORD));
		pBuf += sizeof(WORD);

		memcpy(pBuf, &iter->second->wSuccessRate, sizeof(WORD));
		pBuf += sizeof(WORD);

		memcpy(pBuf, &iter->second->wLianzhiRate, sizeof(WORD));
		pBuf += sizeof(WORD);

		memcpy(pBuf, &iter->second->wXMTJRate, sizeof(WORD));
		pBuf += sizeof(WORD);
	}

	g_StoreMessage(dnidClient, &msg, sizeof(WORD) * 5 * m_HYDInfo.size() + sizeof(WORD));

	rfalse("The size of HUISHENDAN_INFO is: %d ", sizeof(WORD) * 5 * m_HYDInfo.size() + sizeof(WORD));

	return TRUE;
}

bool CItemService::LoadExtraAttri()
{
	dwt::ifstream infile("Data\\掉落\\道具附加属性表.txt");
	if (!infile.is_open())
	{
		rfalse("无法打开道具附加属性表！");
		infile.close();
		return false;
	}

	char buffer[2048];
	SEquipExtraAttriTable *EquipExtraAttri = 0;

	while (!infile.eof())
	{
		infile.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		EquipExtraAttri = new SEquipExtraAttriTable;

		linebuf >> EquipExtraAttri->EquipId;
		linebuf >> EquipExtraAttri->MinNum;
		linebuf >> EquipExtraAttri->MaxNum;

		for (int i = 0; i < SEquipDataEx::EEA_MAX; i++)
		{
			for (int j = 0; j < 3; j++)
				linebuf >> EquipExtraAttri->ExtraData[i][j];
		}
		long temp = 0;
		EquipExtraAttri->ResetItemID = ((linebuf >> temp), temp); temp = 0;		//需要的重置道具ID	
		EquipExtraAttri->ResetNeedNum = ((linebuf >> temp), temp); temp = 0;		//重置需要的数量
		EquipExtraAttri->ResetMoneyType = ((linebuf >> temp), temp); temp = 0;		//需要的钱类型	
		EquipExtraAttri->ResetNeedMoney = ((linebuf >> temp), temp); temp = 0;		//重置需要的钱	
		EquipExtraAttri->RefreshItemID = ((linebuf >> temp), temp); temp = 0;		//需要的刷新道具ID	
		EquipExtraAttri->RefreshNeedNum= ((linebuf >> temp), temp); temp = 0;		//刷新需要的数量		
		EquipExtraAttri->RefreshMoneyType = ((linebuf >> temp), temp); temp = 0;		//刷新的钱类型	
		EquipExtraAttri->RefreshNeedMoney = ((linebuf >> temp), temp); temp = 0;		//刷新需要的钱
		EquipExtraAttri->ExternLockedItemID = ((linebuf >> temp), temp); temp = 0;		// 可用锁定石ID
		EquipExtraAttri->ExternLockeItemNum = ((linebuf >> temp), temp); temp = 0;		// 锁定一条属性需要锁定石的数量

		std::pair<EquipExtraAttriTable::iterator, bool> result;
		result = m_EATable.insert(make_pair(EquipExtraAttri->EquipId, EquipExtraAttri));

		if (false == result.second)
		{
			rfalse("【Data\\掉落\\道具附加属性表.txt】发现了一个重复的道具 %d！", EquipExtraAttri->EquipId);
			delete EquipExtraAttri;
		}
	}
	infile.close();

	return true;
}

const SItemBaseData *CItemService::GetItemBaseData(DWORD index) const
{
	ItemBaseData::const_iterator item = m_baseData.find(index);
	if (item != m_baseData.end())
		return item->second;
	else
		return 0;
}

const SItemFactorData *CItemService::GetItemFactorData(DWORD index) const
{
	ItemFactorData::const_iterator factor = m_factorData.find(index);
	if (factor != m_factorData.end())
		return factor->second;
	else
		return 0;
}

DWORD CItemService::GetItemGroupCDTime(DWORD group) const
{
	if (group < 2000 || group > 2999)
		return 0;

	ItemGroupCD::const_iterator it = m_groupCD.find(group);
	if (it != m_groupCD.end())
		return it->second;
	else
		return 0;
}

void CItemService::SetFactorValue(DWORD item, WORD attri, int value, bool IsPre)
{
	if (attri >= SEquipDataEx::EEA_MAX || 0 == value)
		return;

	SItemFactorData *fd = 0;

	ItemFactorData::iterator factor = m_factorData.find(item);
	if (factor == m_factorData.end())
	{
		fd = new SItemFactorData;
		m_factorData[item] = fd;
	}
	else
		fd = m_factorData[item];

	IsPre ? (fd->m_PrecentFactor[attri]) : (fd->m_ValueFactor[attri]) = value;
}

// const SEquipExtraAttriTable *CItemService::GetExtraAttriTable(DWORD index) const
// {
// 	EquipExtraAttriTable::const_iterator item = m_EATable.find(index);
// 	if (item != m_EATable.end())
// 	{
// 		return item->second;
// 	}
// 	else
// 	{
// 		return 0;
// 	}
// }

const SItemUpdateGradeInfo *CItemService::GetUpgradeInfo(BYTE grade) const
{
	ItemUpdateGradeTable::const_iterator itemInfo = m_gradeTalbe.find(grade);
	
	if (itemInfo != m_gradeTalbe.end())
	{
		return itemInfo->second;
	}
	else
	{
		return 0;
	}
}

// index 的计算： 装备类型 和 等级 和颜色有关系
const SItemUpgradeAttribute *CItemService::GetUpgradeAttribute(DWORD index) const
{
	ItemUpgradeAttribute::const_iterator itemAttri = m_upgradeAttri.find(index);

	if (itemAttri != m_upgradeAttri.end())
	{
		return itemAttri->second;
	}
	else
		return 0;
}

const SItemUpgradeAttribute *CItemService::GetUpgradeAttribute(const struct SItemBaseData *pItemData) const
{
	if (!pItemData)
	{
		return 0;
	}
	// index计算方式：装备类型*100000 + 装备等级 * 100 + 装备颜色
	DWORD dwIndex = pItemData->m_Type * 100000 + pItemData->m_Level * 100 + pItemData->m_Color;	
	return GetUpgradeAttribute(dwIndex);
}

const SItemUpdateQuality *CItemService::GetUpdateQuality(BYTE color) const			// 装备升品质
{
	ItemUpdateQuality::const_iterator updateQuality = m_updateQuality.find(color);

	if (updateQuality != m_updateQuality.end())
	{
		return updateQuality->second;
	}
	else
	{
		return 0;
	}
}

const SEquipLevelUp	*CItemService::GetLevelUpData(BYTE level) const			// 装备升级
{
	EquipLevelUp::const_iterator iter = m_EquipLevelUp.find(level);

	if (iter != m_EquipLevelUp.end())
	{
		return iter->second;
	}
	else
	{
		return 0;
	}
}

const SEquipStar *CItemService::GetEquipStarData(BYTE num) const			// 装备升星
{
	EquipStar::const_iterator iter = m_EquipStar.find(num);

	if (iter != m_EquipStar.end())
	{
		return iter->second;
	}
	else
	{
		return 0;
	}
}

// 装备随机属性最大值读取
const SMaxExtraAttri *CItemService::GetMaxExtraAttri(DWORD index) const
{
	MaxExtraAttri::const_iterator iter = m_MaxAttri.find(index);

	if (iter != m_MaxAttri.end())
		return iter->second;
	else
		return 0;
}

// 获取升阶的附加属性上限的读取
const SMaxExtraAttri *CItemService::GetMaxExtraAttri(const struct SItemBaseData *pItemData, byte byCurGrade) const
{
	if (!pItemData)
	{
		return 0;
	}
	// 装备最大附加属性值index计算方式：装备等级 * 1000 + 装备品质 * 100 + 装备阶数
	DWORD index = pItemData->m_Level * 1000 + pItemData->m_Color * 100 + byCurGrade;
	return GetMaxExtraAttri(index);
}

// 得到宝石属性
const SJewelAttribute *CItemService::GetJewelAttribute(DWORD index) const
{
	JewelAttribute::const_iterator iter = m_JewAttri.find(index);

	if (iter != m_JewAttri.end())
		return iter->second;
	else
		return 0;
}

// 回元丹信息
const SHuiYuanDanInfo *CItemService::GetHuiYuanDanInfo(DWORD index) const
{
	HuiYuanDanInfo::const_iterator iter = m_HYDInfo.find(index);

	if (iter != m_HYDInfo.end())
		return iter->second;
	else
		return 0;
}
const SMakeHoleInfo *CItemService::GetMakeHoleInfo(DWORD index) const
{
	MakeHoleInfo::const_iterator iter = m_MakeHoleInfo.find(index);
	if (iter != m_MakeHoleInfo.end())
	{
		return &iter->second;
	}
	return 0;
}
const SInsertGem *CItemService::GetInsertGemInfo(DWORD index) const
{
	SInsertGemInfo::const_iterator iter = m_SInsertGemInfo.find(index);
	if (iter != m_SInsertGemInfo.end())
	{
		return &iter->second;
	}
	return 0;
}
const STakeOffGem *CItemService::GetSTakeOffGemInfo(DWORD index) const
{
	STakeOffGemInfo::const_iterator iter = m_STakeOffGemInfo.find(index);
	if (iter != m_STakeOffGemInfo.end())
	{
		return &iter->second;
	}
	return 0;
}
long CItemService::GetRandomNum(BYTE Color,long RandomNum)
{
	if (RandomNum > 100)
	{
		RandomNum = 100;
	}
	SDefineResetInfo::const_iterator iter = m_SDefineResetInfo.find(Color);
	if (iter != m_SDefineResetInfo.end())
	{
		int iLen = sizeof(iter->second.m_Rate) / sizeof(iter->second.m_Rate[0]);
		short temp = 0;
		for (int i = 0; i < iLen; ++i) 
		{
			short vl = iter->second.m_Rate[i];
			temp += vl;
			if (RandomNum <= temp)
			{
				return i+1;
			}
		}
	}
	return 0;
}

// 获取装备的分解属性
const SEquipDecomposition *CItemService::GetSEquipDecompositionInfo(const struct SItemBaseData *pItemData, byte byStarNum) const
{
	if (!pItemData)
	{
		return 0;
	}

	DWORD index = pItemData->m_Color * 100 + byStarNum;
	SEquipDecompositionInfo::const_iterator iter = m_SEquipDecompositionInfo.find(index);
	if (iter != m_SEquipDecompositionInfo.end())
	{
		return iter->second;
	}	
	return 0;
}

// 获取装备的熔炼信息
const SEquipSmelting *CItemService::GetSEquipSmeltingInfo(byte byColor) const
{
	SEquipSmeltingInfo::const_iterator iter = m_SEquipSmeltingInfo.find(byColor);
	if (iter != m_SEquipSmeltingInfo.end())
	{
		return iter->second;
	}
	return 0;
}

// 获取装备极品预览附加属性信息
const SEquipBestPreviewExtraAttri *CItemService::GetSEquipBestPreviewExtraAttri(byte byEquipType) const
{
	SEquipBestPreviewExtraAttriInfo::const_iterator citer = m_SEquipBestPreviewExtraAttri.find(byEquipType);
	if (citer != m_SEquipBestPreviewExtraAttri.end())
	{
		return citer->second;
	}
	return 0;
}

// 获取灵附套装信息
const SEquipSpiritAttachBodyAttr *CItemService::GetSEquipSpiritAttachBodyAttri(WORD wSuitEquipID) const
{
	SEquipSpiritAttachBodyAttrInfo::const_iterator citer = m_SEquipSpiritAttachBodyAttri.find(wSuitEquipID);
	if (citer != m_SEquipSpiritAttachBodyAttri.end())
	{
		return citer->second;
	}
	return 0;
}

// 通过卷轴ID查找对应的套装ID
WORD CItemService::GetSuitEquipIDbyScrollID(DWORD dwScrollID) const
{
	WORD	wSuitEquipID = 0;
	for (SEquipSpiritAttachBodyAttrInfo::const_iterator citer = m_SEquipSpiritAttachBodyAttri.begin(); citer != m_SEquipSpiritAttachBodyAttri.end(); ++citer)
	{
		SEquipSpiritAttachBodyAttr *pTmp = citer->second;
		if (NULL == pTmp) continue;
		if (pTmp->dwNeedItemID == dwScrollID)
		{
			wSuitEquipID = citer->first;
			break;
		}		
	}
	return wSuitEquipID;
}

bool CItemService::GetFactorData( DWORD item, WORD &attr,WORD &nvalue,WORD nIndex )
{
	WORD currentin = 0;
	ItemFactorData::iterator factor = m_factorData.find(item);
	if (factor!= m_factorData.end())
	{
		SItemFactorData *fd = factor->second;
		for (int i = 0;i < SEquipDataEx::EEA_MAX;i++)
		{
			if (fd&&fd->m_ValueFactor[i] > 0)
			{
				if (currentin == nIndex)
				{
					attr = i;
					nvalue = fd->m_ValueFactor[i];
					return true;
				}
				currentin++;
			}
		}
	}
	return false;
}

#pragma warning(pop)