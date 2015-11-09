#include "..\stdafx.h"
#include "..\CDManager.h"
#include "BaseDataManager.h"
#include "HeroMoudle\HeroAttributeCalculator.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "StorageMoudle\StorageManager.h";
#include "AchievementMoudle\AchieveUpdate.h"
#include "AchievementMoudle\RandomAchieveUpdate.h"
#include "MissionMoudle\CMissionUpdate.h"
#include "Common\ConfigManager.h";
#include "Common\PubTool.h"
#include "EnduranceManager.h"
#include"Common\ConfigManager.h"
#include <time.h>
#include "Networkmodule\DataMsgs.h"
#include "playermanager.h"
#include "player.h"
#include "GuideMoudle\GuideManager.h"
#include "CMystring.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"

int CBaseDataManager::CheckGameName(char* newname,DWORD SID)
{
	//检查在线缓存

	CPlayer *pDestPlayer = (CPlayer *)GetPlayerByName(newname)->DynamicCast(IID_PLAYER);
	if (pDestPlayer != NULL)
	{
		//TalkToDnid(dnidClient, "当前在线玩家中，已经有人叫这个名字了！！！");
		return 0;
	}

	//检查数据库
	lite::Variant ret;//调用存储过程
	LuaFunctor(g_Script, "db_CheckName")[newname][SID](&ret);


	return 1;
}
CBaseDataManager::CBaseDataManager(DNID clientIndex, SFixData* pPlayerData,CPlayer * _cplayer)
	:CHeroDataManager(pPlayerData->m_HeroList),
	m_pPlayerData(pPlayerData),
	cPlayer(_cplayer),
	m_ClientIndex(clientIndex),
	m_CurItemNum(0),
	m_bTempGMFlag(false)
{
	m_upCdMgr.reset(new CDManager());
	m_upStorageMgr.reset(new CStorageManager(m_pPlayerData->m_dwStaticID));
	m_upEnduranceMgr.reset(new CEnduranceManager());
	m_upGuideMgr.reset(new CGuideManager());
	m_ptrAcievementUpdate = nullptr;
	m_ptrRandomAchievementUpdate = nullptr;
	m_ptrMissionUpdate = nullptr;

	//登录时间
	_time64(&m_LoginTime);
	ConverTool::ConvertInt64ToBytes(m_LoginTime, m_pPlayerData->m_TimeData.m_LoginTime);

	//上次下线时间
	__time64_t *p64 = (__time64_t *)m_pPlayerData->m_TimeData.m_LogoutTime;
	m_LogoutTime = *(p64);
	m_pDefenseTeamID = m_pPlayerData->m_ArenaDefenseTeam;
}


CBaseDataManager::~CBaseDataManager()
{
	Release();
}

void CBaseDataManager::Init(bool bGMFlag)
{
	m_bTempGMFlag = bGMFlag;
	/// 初始化武将
	__super::Init();
	m_upStorageMgr->InitItemData(m_pPlayerData->m_ItemList);
	m_upEnduranceMgr->Init(&m_pPlayerData->m_EnduranceData, bGMFlag);
	m_upGuideMgr->InitGuideData(&m_pPlayerData->m_NewbieGuideData);
	if (!m_bTempGMFlag && !m_upGuideMgr->BeTriggered(GuideType::CollectEquipment))
	{
		m_funcJudgeUpgradeRank = std::bind(&CBaseDataManager::JudgeHeroUpgradeRank, this, std::placeholders::_1);
		m_upStorageMgr->BindAddItemFunction(m_funcJudgeUpgradeRank);
	}
	m_bDataInitialed = true;
}

void CBaseDataManager::InitDataOfLogin()
{
	m_upEnduranceMgr->InitEnduranceData(m_pPlayerData->m_bNewPlayer, GetPlayerLevel(), GetSID());
}

void CBaseDataManager::SetAchievementUpdator(CAchieveUpdate* pAchievement)
{
	m_ptrAcievementUpdate = pAchievement;
}

void CBaseDataManager::SetRandomAchievementUpdator(CRandomAchieveUpdate* updator)
{
	m_ptrRandomAchievementUpdate = updator;
}

void CBaseDataManager::SetMissionUpdator(CMissionUpdate* updator)
{
	m_ptrMissionUpdate = updator;
}


DWORD CBaseDataManager::GetGoods(int32_t iType, int32_t id /* = 0 */)
{
	if (!_checkSFixData())
		return 0;

	switch (iType)
	{
	case GoodsType::money:
		return m_pPlayerData->m_dwMoney;
	case GoodsType::diamond:
		return m_pPlayerData->m_dwDiamond;
	case GoodsType::item:
		return m_upStorageMgr->GetItemNum(id);
	case GoodsType::hero:
		if (ExistHero(id))
			return 1;
		break;
	case GoodsType::Exp:
		return  m_pPlayerData->m_dwExp;
	case GoodsType::honor:
		return m_pPlayerData->m_dwHonor;
	case  GoodsType::exploit:
		return m_pPlayerData->m_dwExploit;
	case GoodsType::endurance:
		return m_upEnduranceMgr->GetEndurance();
	case GoodsType::level:
		return m_pPlayerData->m_dwLevel;
	case GoodsType::Prestige:
		return m_pPlayerData->m_dwPrestige;
	case GoodsType::blessPoints:
		return m_pPlayerData->m_dwBlessPoints;
	case GoodsType::soulPoints:
		return m_pPlayerData->m_dwSoulPoints;
	case GoodsType::token:
		return m_pPlayerData->m_dwToken;
	default:
		break;
	}

	return 0;
}

//******wk 20150114 玩家道具物品操作统一接口:增加,检查,扣除*****begin************************
/*
iType为类型:
1 铜币:id无用 num=数量
2 钻石:  id无用  num=数量
3 道具: id=道具Id  num=数量
4 武将:  id=武将id  num=无用
5 经验: id无用  num=数量
6 荣誉: id无用 num = 数量

money = 1, //铜钱
diamond=2,  //钻石
item=3, //道具
hero=4, //英雄
Exp=5,//经验
honor=6,//荣誉
exploit=7,//军功
endurance=8,//体力
level=9, ///君主等级
vipExp=10,//vip经验
Prestige=11,//声望，国战产出的声望
blessPoints = 12,//祈福产出的祈福积分
soulPoints = 13,//分解将魂产出的魂点
token = 14, //军团产出的军令
*/
//增加单个单据数据等
DWORD CBaseDataManager::AddGoods_SG(const int32_t iType, const int32_t id/* =0 */, int32_t num /* = 0 */, int32_t detailType /*= 0*/, bool IsSynToClient /* = true */)
{
	if (num<=0)
	{
		return 0;
	}
	DWORD ret = 0;
	bool modifyEndurance = false;
	switch (iType)
	{
	case GoodsType::money:
		ret = PlusMoney(num);

		//g_Script.SetCondition(0, cPlayer, 0); 
		//LuaFunctor(g_Script, "rank_update")[m_pPlayerData->m_dwStaticID][RankType_SG::PlayerMoney][ret]();
		////g_Script.CleanCondition();//不清理对象,不然后续使用c++函数时会没有对象

		break;
	case GoodsType::diamond:
		ret = PlusDiamond(num);
		break;
	case GoodsType::item:
		if (m_upStorageMgr->AddItem(id, num))
			ret = num;
		break;
	case GoodsType::hero:
		if (AddHero(id))
			ret = 1;
		break;
	case GoodsType::Exp:
		PlusExp(num);
		ret = m_pPlayerData->m_dwExp;
		break;
	case GoodsType::honor:
		PlusHonor(num);
		ret = GetHonor();
		break;
	case  GoodsType::exploit:
		PlusExploit(num);
		ret = m_pPlayerData->m_dwExploit;
		break;
	case GoodsType::endurance:
		modifyEndurance = true;
		m_upEnduranceMgr->Check(GetPlayerLevel());
		m_upEnduranceMgr->PlusEndurance(num, GetPlayerLevel());
		ret = m_upEnduranceMgr->GetEndurance();
		break;
	case GoodsType::level:
		ret = m_pPlayerData->m_dwLevel;
		break;
	case GoodsType::vipExp:
		//ret = m_pPlayerData->m_dwLevel;
	{
		lite::Variant nowVipexp;//从lua获取返回值
		//g_Script.SetCondition(0, cPlayer, 0); 
		LuaFunctor(g_Script, "SI_vip_addexp")[m_pPlayerData->m_dwStaticID][num](&nowVipexp);
		//g_Script.CleanCondition();//此处脚本通过m_pPlayerData->m_dwStaticID来定位玩家,可不设置,不然清理后后续使用c++函数时会没有对象
		ret = (int)nowVipexp;
	}
		
		break;
	case GoodsType::Prestige:
		ret = PlusPrestige(num);
		break;
	case GoodsType::blessPoints:
		ret = ModifyBlessPoints(num);
		break;
	case GoodsType::soulPoints:
		ret = ModifySoulPoints(num);
		break;
	case GoodsType::token:
		{
			//ret = ModifyToken(num);
			if (g_Script.PrepareFunction("ChangePlayerFactionMoney"))
			{
				g_Script.PushParameter(GetSID());
				g_Script.PushParameter(num);
				g_Script.Execute();
			}

			lite::lua_variant ret1;
			BOOL result = FALSE;
			if (g_Script.PrepareFunction("GetPlayerFactionMoney"))
			{
				g_Script.PushParameter(GetSID());
				result = g_Script.Execute(&ret1);
			}
			else
			{
				return 0;
			}

			if (!result || ret1.dataType == LUA_TNIL)
			{
				rfalse("GetPlayerFactionMoney Failed");
				return 0;
			}

			try
			{
				ret = (int)(ret1);
			}
			catch (lite::Xcpt &e)
			{
				rfalse(2, 1, e.GetErrInfo());
				return 0;
			}
		}
		break;
	default:
		break;
	}

	if (IsSynToClient)
	{
		SAAddGoodsSYNMsg rMsg; //同步至前台,铜钱等为现在总数
		rMsg.iType = iType;
		rMsg.itemGroupNum = 1;
		rMsg.bIDIndex = false;
		int unusedNum = 0;
		///如果不为0，这代表该Goods是以ID为索引的，比如英雄ID、物品ID
		if (id != 0)
		{
			unusedNum = MAX_ITEM_NUM_SYN_TO_CLIENT - 2;
			rMsg.bIDIndex = true;
			rMsg.itemList[0] = id;
			rMsg.itemList[1] = ret;
		}
		else///如果为0，则代表是金钱、体力什么的同一类只有一个个体的Goods
		{
			if (modifyEndurance)
			{
				rMsg.bIDIndex = true;
				unusedNum = MAX_ITEM_NUM_SYN_TO_CLIENT - 2;
				rMsg.itemList[0] = ret;
				rMsg.itemList[1] = m_pPlayerData->m_EnduranceData.m_dwRemainingSeconds;
			}
			else
			{
				unusedNum = MAX_ITEM_NUM_SYN_TO_CLIENT - 1;
				rMsg.itemList[0] = ret;
			}
		}
		///将没用的数组空间去掉，不发送到服务器
		g_StoreMessage(m_ClientIndex, &rMsg, sizeof(SAAddGoodsSYNMsg) - unusedNum * sizeof(DWORD));
	}

	//日志
	bool logMark = false;
	if (iType == GoodsType::diamond || iType == GoodsType::honor || iType == GoodsType::Exp || iType == GoodsType::vipExp)
	{
		logMark = true;
		if (iType == GoodsType::diamond && num >= 0)
		{
			INT result;
			if (g_Script.PrepareFunction("updateTotalDiamond"))
			{
				g_Script.PushParameter(GetSID());
				g_Script.PushParameter(num);
				result = g_Script.Execute();

			}
			if (result == FALSE)
			{
				rfalse("累计获取元宝出错%d", num);
				return 0;
			}
		}
	}

	else if (iType == GoodsType::money && num >  CConfigManager::getSingleton()->globalConfig.LogMoneyNum)
		logMark = true;
	else if (iType == GoodsType::item )
	{
		if (num>= CConfigManager::getSingleton()->globalConfig.LogItemNum)
			logMark = true;
		else if (
					(id >= CConfigManager::getSingleton()->globalConfig.LogItemIdBengin1 
							&& id <= CConfigManager::getSingleton()->globalConfig.LogItemIdEnd1)
						|| (id >= CConfigManager::getSingleton()->globalConfig.LogItemIdBengin2
							&& id <= CConfigManager::getSingleton()->globalConfig.LogItemIdEnd2)
						|| (id >= CConfigManager::getSingleton()->globalConfig.LogItemIdBengin3
							&& id <= CConfigManager::getSingleton()->globalConfig.LogItemIdEnd3))
			logMark = true;
	}
		
	if (logMark)
	{

		SQPointLogMsg msg;
		msg.diff_value = num;
		msg.finale_value = ret;
		msg.uid = GetSID();
		msg.itype = iType;
		msg.detailType = detailType;
		msg.pInfo = m_pPlayerData->m_dwLevel;
		msg.otherInfo = id;
		_snprintf(msg.account, sizeof msg.account, GetName().c_str());
		SendToLoginServer(&msg, sizeof(msg));
	}
	if (detailType==0)
	{
		rfalse("AddGoods_SG 来源 detailType 未填写,请注意修正!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	}
	return ret;
}
//批量增加道具等
bool CBaseDataManager::AddGoods_SG(int32_t iType, int IDNum, int32_t numList[], const int32_t IDList[] /* = nullptr */, int32_t detailType)
{
	if (numList == nullptr || IDNum == 0)
	{
		rfalse("批量添加物品的数量动态数组为Null或者要加的数量为0");
		return false;
	}

	try
	{
		for (int i = 0; i < IDNum; ++i)
		{
			///此处单个添加不往客户端发消息
			if (IDList == nullptr)
				numList[i] = AddGoods_SG(iType, 0, numList[i], detailType, false);
			else
				numList[i] = AddGoods_SG(iType, IDList[i], numList[i], detailType, false);
		}

		SAAddGoodsSYNMsg rMsg; //同步至前台,铜钱等为现在总数
		rMsg.iType = iType;
		int unusedNum = 0;
		int curUnsedNum = 0;
		rMsg.bIDIndex = false;
		switch (iType)
		{
		case GoodsType::endurance:
			rMsg.bIDIndex = true;
			curUnsedNum = 1;
			unusedNum = MAX_ITEM_NUM_SYN_TO_CLIENT - curUnsedNum;
			rMsg.itemList[0] = numList[IDNum - 1];
			rMsg.itemList[1] = m_pPlayerData->m_EnduranceData.m_dwRemainingSeconds;
			break;
		case GoodsType::diamond:
		case GoodsType::Exp:
		case GoodsType::exploit:
		case GoodsType::honor:
		case GoodsType::level:
		case GoodsType::money:
		case GoodsType::blessPoints:
		case GoodsType::soulPoints:
		case GoodsType::token:
			curUnsedNum = 1;
			unusedNum = MAX_ITEM_NUM_SYN_TO_CLIENT - curUnsedNum;
			///只记录最后一次的总的数据
			memcpy_s(rMsg.itemList, sizeof(DWORD), numList + IDNum - 1, sizeof(DWORD));
			break;
		case GoodsType::hero:
		case GoodsType::item:
			///物品类型个数不能超过最大的同步个数
			curUnsedNum = min(MAX_ITEM_NUM_SYN_TO_CLIENT / 2, IDNum);
			if (IDList != nullptr)
			{
				unusedNum = MAX_ITEM_NUM_SYN_TO_CLIENT - 2 * curUnsedNum;
				rMsg.bIDIndex = true;
				memcpy_s(rMsg.itemList, sizeof(DWORD) * curUnsedNum, IDList, sizeof(DWORD) * curUnsedNum);
				///偏移IDNum位，继续存物品对应的个数
				memcpy_s(rMsg.itemList + curUnsedNum, sizeof(DWORD) * curUnsedNum, numList, sizeof(DWORD) * curUnsedNum);
			}
			break;
		}
		rMsg.itemGroupNum = curUnsedNum;

		///将没用的数组空间去掉，不发送到服务器
		g_StoreMessage(m_ClientIndex, &rMsg, sizeof(SAAddGoodsSYNMsg) - unusedNum * sizeof(DWORD));
	}
	catch (const std::exception& oor)
	{
		rfalse("批量添加Goods抛出异常");
	}
}

//扣物品,类型参照AddGoods
DWORD  CBaseDataManager::DecGoods_SG(const int32_t iType, const int32_t id /* = 0 */, int32_t num /* = 0 */, int32_t detailType /*= 0*/, bool IsSynToClient /* = true */)
{
	DWORD ret = 0;
	bool modifyEndurance = false;
	if (num<0 || id<0)
	{
		return false;
	}
	switch (iType)
	{
	case GoodsType::money:
		ret = PlusMoney(-num);
		break;
	case GoodsType::diamond:
		ret = PlusDiamond(-num);
		break;
	case GoodsType::item:
		if (m_upStorageMgr->TryToRemoveItem(id, num))
		{
			///发正数给客户端，客户端再调用RemoveItem逻辑
			ret = num;
		}
		else
			return false;
		break;
	case GoodsType::hero:

		break;
	case GoodsType::Exp:
		m_pPlayerData->m_dwExp -= num;
		ret = m_pPlayerData->m_dwExp;
	case GoodsType::endurance:
		modifyEndurance = true;
		m_upEnduranceMgr->Check(GetPlayerLevel());
		m_upEnduranceMgr->PlusEndurance(-num, GetPlayerLevel());
		ret = m_upEnduranceMgr->GetEndurance();
		break;
	case GoodsType::Prestige:
		ret = PlusPrestige(-num);
		break;
	case GoodsType::honor:
		ret = PlusHonor(-num);
		break;
	case GoodsType::exploit:
		ret = PlusExploit(-num);
		break;
	case GoodsType::blessPoints:
		ret = ModifyBlessPoints(-num);
		break;
	case GoodsType::soulPoints:
		ret = ModifySoulPoints(-num);
		break;
	case GoodsType::token:
		{
			//ret = ModifyToken(-num);
			if (g_Script.PrepareFunction("ChangePlayerFactionMoney"))
			{
				g_Script.PushParameter(GetSID());
				g_Script.PushParameter(-num);
				g_Script.Execute();
			}

			lite::lua_variant ret1;
			BOOL result = FALSE;
			if (g_Script.PrepareFunction("GetPlayerFactionMoney"))
			{
				g_Script.PushParameter(GetSID());
				result = g_Script.Execute(&ret1);
			}
			else
			{
				return 0;
			}

			if (!result || ret1.dataType == LUA_TNIL)
			{
				rfalse("GetPlayerFactionMoney Failed");
				return 0;
			}

			try
			{
				ret = (int)(ret1);
			}
			catch (lite::Xcpt &e)
			{
				rfalse(2, 1, e.GetErrInfo());
				return 0;
			}
		}
		break;
	default:
		break;
	}

	lua_settop(g_Script.ls, 0);  //这里有可能是来自lua的调用，所以栈可能不为空，这里先清空
	if (g_Script.PrepareFunction("OnPlayerDecGoods_SG"))
	{
		g_Script.PushParameter(GetSID());
		g_Script.PushParameter(iType);
		g_Script.PushParameter(id);
		g_Script.PushParameter(num);
		g_Script.Execute();
	}


	if (IsSynToClient)
	{
		SADecGoodsSYNMsg rMsg; //同步至前台,铜钱等为现在总数
		rMsg.iType = iType;
		rMsg.itemGroupNum = 1;
		rMsg.bIDIndex = false;
		int unusedNum = 0;
		///如果不为0，这代表该Goods是以ID为索引的，比如英雄ID、物品ID
		if (id != 0)
		{
			unusedNum = MAX_ITEM_NUM_SYN_TO_CLIENT - 2;
			rMsg.bIDIndex = true;
			rMsg.itemList[0] = id;
			rMsg.itemList[1] = ret;
		}
		else///如果为0，则代表是金钱、体力什么的同一类只有一个个体的Goods
		{
			if (modifyEndurance)
			{
				rMsg.bIDIndex = true;
				unusedNum = MAX_ITEM_NUM_SYN_TO_CLIENT - 2;
				rMsg.itemList[0] = ret;
				rMsg.itemList[1] = m_pPlayerData->m_EnduranceData.m_dwRemainingSeconds;
			}
			else
			{
				unusedNum = MAX_ITEM_NUM_SYN_TO_CLIENT - 1;
				rMsg.itemList[0] = ret;
			}
		}

		///将没用的数组空间去掉，不发送到服务器
		g_StoreMessage(m_ClientIndex, &rMsg, sizeof(SADecGoodsSYNMsg) - unusedNum * sizeof(DWORD));
	}
	//日志
	bool logMark = false;
	if (iType == GoodsType::diamond || iType == GoodsType::honor || iType == GoodsType::Exp || iType == GoodsType::vipExp)
		logMark = true;
	else if (iType == GoodsType::money && num > CConfigManager::getSingleton()->globalConfig.LogMoneyNum)
		logMark = true;
	else if (iType == GoodsType::item)
	{
		if (num >= CConfigManager::getSingleton()->globalConfig.LogItemNum)
			logMark = true;
		else if (
			(id >= CConfigManager::getSingleton()->globalConfig.LogItemIdBengin1
			&& id <= CConfigManager::getSingleton()->globalConfig.LogItemIdEnd1)
			|| (id >= CConfigManager::getSingleton()->globalConfig.LogItemIdBengin2
			&& id <= CConfigManager::getSingleton()->globalConfig.LogItemIdEnd2)
			|| (id >= CConfigManager::getSingleton()->globalConfig.LogItemIdBengin3
			&& id <= CConfigManager::getSingleton()->globalConfig.LogItemIdEnd3))
			logMark = true;
	}

	if (logMark)
	{
		SQPointLogMsg msg;
		msg.diff_value = -num;
		msg.finale_value = ret;
		msg.uid = GetSID();
		msg.itype = iType;
		msg.detailType = detailType;
		msg.pInfo = m_pPlayerData->m_dwLevel;
		msg.otherInfo = id;
		_snprintf(msg.account, sizeof msg.account, GetName().c_str());
		SendToLoginServer(&msg, sizeof(msg));
	}
	if (detailType == 0)
	{
		rfalse("DecGoods_SG 去向 detailType 未填写,请注意修正!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	}
	return ret;
}

bool CBaseDataManager::DecGoods_SG(const int32_t iType, int IDNum, int32_t numList[], const int32_t IDList[] /*= nullptr*/, int32_t detailType)
{
	if (numList == nullptr || IDNum == 0)
	{
		rfalse("批量删除物品的数量动态数组为Null或者要加的数量为0");
		return false;
	}

	try
	{
		for (int i = 0; i < IDNum; ++i)
		{
			///此处单个添加不往客户端发消息
			if (IDList == nullptr)
				numList[i] = DecGoods_SG(iType, 0, numList[i], detailType, false);
			else
				numList[i] = DecGoods_SG(iType, IDList[i], numList[i], detailType, false);
		}

		SADecGoodsSYNMsg rMsg; //同步至前台,铜钱等为现在总数
		rMsg.iType = iType;
		int unusedNum = 0;
		int curUnsedNum = 0;
		rMsg.bIDIndex = false;
		switch (iType)
		{
		case GoodsType::endurance:
			rMsg.bIDIndex = true;
			curUnsedNum = 1;
			unusedNum = MAX_ITEM_NUM_SYN_TO_CLIENT - curUnsedNum;
			rMsg.itemList[0] = numList[IDNum - 1];
			rMsg.itemList[1] = m_pPlayerData->m_EnduranceData.m_dwRemainingSeconds;
			break;
		case GoodsType::diamond:
		case GoodsType::Exp:
		case GoodsType::exploit:
		case GoodsType::honor:
		case GoodsType::level:
		case GoodsType::money:
		case GoodsType::blessPoints:
		case GoodsType::soulPoints:
		case GoodsType::token:
			curUnsedNum = 1;
			unusedNum = MAX_ITEM_NUM_SYN_TO_CLIENT - curUnsedNum;
			///只记录最后一次的总的数据
			memcpy_s(rMsg.itemList, sizeof(DWORD), numList + IDNum - 1, sizeof(DWORD));
			break;
		case GoodsType::hero:
		case GoodsType::item:
			///物品类型个数不能超过最大的同步个数
			curUnsedNum = min(MAX_ITEM_NUM_SYN_TO_CLIENT / 2, IDNum);
			if (IDList != nullptr)
			{
				unusedNum = MAX_ITEM_NUM_SYN_TO_CLIENT - 2 * curUnsedNum;
				rMsg.bIDIndex = true;
				memcpy_s(rMsg.itemList, sizeof(DWORD) * curUnsedNum, IDList, sizeof(DWORD) * curUnsedNum);
				///偏移IDNum位，继续存物品对应的个数
				memcpy_s(rMsg.itemList + curUnsedNum, sizeof(DWORD) * curUnsedNum, numList, sizeof(DWORD) * curUnsedNum);
			}
			break;
		}
		rMsg.itemGroupNum = IDNum;

		///将没用的数组空间去掉，不发送到服务器
		g_StoreMessage(m_ClientIndex, &rMsg, sizeof(SADecGoodsSYNMsg) - unusedNum * sizeof(DWORD));
	}
	catch (const std::exception& oor)
	{
		rfalse("批量删除物品抛出异常");
	}
}

//检查物品,类型参照AddGoods
bool  CBaseDataManager::CheckGoods_SG(const int32_t iType, const int32_t id, int32_t num) 
{
	if (num < 0)
		return false;

	switch (iType)
	{
	case GoodsType::money:
		if (m_pPlayerData->m_dwMoney < num)
		{
			return false;
		}
		break;
	case GoodsType::diamond:
		if (m_pPlayerData->m_dwDiamond < num)
		{
			return false;
		}
		break;
	case GoodsType::item:
		if (!m_upStorageMgr->ItemExistOrEnough(id, num))
		{
			return false;
		}
		break;
	case GoodsType::hero:
		if (!ExistHero(id))
		{
			return false;
		}
		break;
	case GoodsType::Exp:
		if (m_pPlayerData->m_dwDiamond < num)
		{
			return false;
		}
		break;
	case  GoodsType::endurance:
		{
			if (!m_upEnduranceMgr->Check(GetPlayerLevel(), num))
			{
				SAAddGoodsSYNMsg rMsg;
				rMsg.iType = GoodsType::endurance;
				rMsg.itemGroupNum = 1;
				rMsg.bIDIndex = true;
				int unusedNum = MAX_ITEM_NUM_SYN_TO_CLIENT - 2;
				rMsg.itemList[0] = m_pPlayerData->m_EnduranceData.m_dwEndurance;
				rMsg.itemList[1] = m_pPlayerData->m_EnduranceData.m_dwRemainingSeconds;
				///将没用的数组空间去掉，不发送到服务器
				g_StoreMessage(GetSID(), &rMsg, sizeof(SAAddGoodsSYNMsg) - unusedNum * sizeof(DWORD));
				return false;
			}
		}
		break;
	default:
	{
		return false;
	}
		break;
	}
	return true;
}

//******wk 20150114 玩家道具物品操作统一接口:增加,检查,扣除*****end************************


int32_t CBaseDataManager::GetMoney() const
{
	return m_pPlayerData->m_dwMoney;
}

int32_t CBaseDataManager::GetEndurance() const
{
	if (m_pPlayerData == nullptr)
		return 0;

	return m_pPlayerData->m_EnduranceData.m_dwEndurance;
}

int32_t CBaseDataManager::GetDiamond() const
{
	return m_pPlayerData->m_dwDiamond;
}

uint32_t CBaseDataManager::GetMaxRecharge() const
{
	return m_pPlayerData->m_dwMaxRecharge;
}

int32_t CBaseDataManager::GetPrestige() const
{
	return nullptr == m_pPlayerData ? 0 : m_pPlayerData->m_dwPrestige;
}

DWORD  CBaseDataManager::PlusMoney(const int32_t value)
{
	if (!_checkSFixData())
		return 0;

	if (value + (int32_t)m_pPlayerData->m_dwMoney < 0)
	{
		m_pPlayerData->m_dwMoney = 0;
		return 0;
	}
	m_pPlayerData->m_dwMoney += value;
	return m_pPlayerData->m_dwMoney;
}

DWORD CBaseDataManager::PlusDiamond(const int32_t value)
{
	if (!_checkSFixData())
		return 0;

	if (value + (int32_t)m_pPlayerData->m_dwDiamond < 0)
	{
		m_pPlayerData->m_dwDiamond = 0;
		return 0;
	}
	m_pPlayerData->m_dwDiamond += value;
	return m_pPlayerData->m_dwDiamond;
}

DWORD CBaseDataManager::AddMaxRechage(const uint32_t value)
{
	if (m_pPlayerData->m_dwMaxRecharge == 0)
	{
		if (g_Script.PrepareFunction("FirstRecharge"))
		{
			g_Script.PushParameter(GetSID());
			g_Script.PushParameter(value);
			g_Script.Execute();
		}
	}
	m_pPlayerData->m_dwMaxRecharge += value;
	return m_pPlayerData->m_dwMaxRecharge;
}

//void CBaseDataManager::PlusEdurance(int32_t value, bool globalLimmitted /* = true */)
//{
//	if (value + m_pPlayerData->m_EnduranceData.m_dwEndurance < 0)
//	{
//		m_pPlayerData->m_EnduranceData.m_dwEndurance = 0;
//		return;
//	}
//
//	m_pPlayerData->m_EnduranceData.m_dwEndurance += value;
//
//	///不能超过等级所限制的体力值
//	if (!globalLimmitted)
//	{
//		const MasterLevelInfor* levelInfor = CConfigManager::getSingleton()->GetMasterLevelInfor(m_pPlayerData->m_dwLevel);
//		if (levelInfor == nullptr)
//			return;
//
//		m_pPlayerData->m_EnduranceData.m_dwEndurance = min(m_pPlayerData->m_EnduranceData.m_dwEndurance, levelInfor->CharactorEnduranceLimit);
//	}
//	else
//	{
//		m_pPlayerData->m_EnduranceData.m_dwEndurance = min(m_pPlayerData->m_EnduranceData.m_dwEndurance, CConfigManager::getSingleton()->globalConfig.PhysicalLimit);
//	}
//}

///很蠢的办法 目前这样吧 以后要丢弃这个地方 有空再整   消耗很高
DWORD CBaseDataManager::AddItem(const int32_t itemID, const int32_t itemCount)
{
	m_upStorageMgr->AddItem(itemID, itemCount);
	return itemID;

	int32_t ItemPos = 0; //存储位置
	bool bAvaliblePos = false ;//找到了合适位置
	SSanguoItem* item = nullptr;
	for (int itemIndex = 0; itemIndex < MAX_ITEM_NUM; itemIndex++)
	{
		item = &m_pPlayerData->m_ItemList[itemIndex];
		if (bAvaliblePos == false && item->m_dwItemID == 0)
		{
			bAvaliblePos = true;
			ItemPos = itemIndex;
		}

		if (item->m_dwItemID == itemID)
			ItemPos = itemIndex;
	}
	assert(item);
	item = &m_pPlayerData->m_ItemList[ItemPos];
	item->m_dwItemID = itemID;
	item->m_dwCount = itemCount;
	return itemID;
}

bool CBaseDataManager::Release()
{
	bool result = true;
	m_ptrAcievementUpdate = nullptr;
	m_ptrRandomAchievementUpdate = nullptr;
	m_ptrMissionUpdate = nullptr;
	if (m_pPlayerData != nullptr)
		m_pPlayerData = nullptr;
	return result;
}

bool CBaseDataManager::_checkSFixData()
{
	if (m_pPlayerData == nullptr)
	{
		rfalse("CBaseDataManager中的m_pPlayerData数据为空");
		return false;
	}

	return true;
}

int32_t CBaseDataManager::GetMasterLevel()
{
	if (!_checkSFixData())
		return 0;

	return m_pPlayerData->m_dwLevel;
}
DWORD CBaseDataManager::GetSID() const { return m_pPlayerData->m_dwStaticID; }

std::string CBaseDataManager::GetName(){ return m_pPlayerData->m_Name; }

int CBaseDataManager::SetGameName(char * newname)
{
	SetNameToMap(newname, m_pPlayerData->m_Name);
	LuaFunctor(g_Script, "OnPlayerChangeName")[m_pPlayerData->m_dwStaticID][m_pPlayerData->m_Name][newname]();

	ZeroMemory(m_pPlayerData->m_Name, sizeof(m_pPlayerData->m_Name));
	memccpy(m_pPlayerData->m_Name, newname, 0, strlen(newname));
	return 1;
}

bool CBaseDataManager::EquipHero(const int32_t heroID, const int32_t euquipID, const int32_t equipType, const int32_t equipLevelLimit)
{
	auto heroData = GetHero(heroID);
	if (nullptr == heroData)
	{
		TalkToDnid(m_ClientIndex, CMyString::GetInstance().GetFormatString("HERO_NOT_FOUND"));//找不到武将
		return false;
	}

	if (equipLevelLimit > heroData->m_dwLevel)
	{
		TalkToDnid(m_ClientIndex, CMyString::GetInstance().GetFormatString("REQUIRE_HIGHER_LEVEL"));//"武将等级不够"
		return false;
	}

	auto rankConfig = CConfigManager::getSingleton()->GetHeroRankConfig(heroID);
	auto rankData = rankConfig->GetRankData(heroData->m_dwRankLevel + 1);
	///进阶到顶级了，不予再次穿装
	if (nullptr == rankConfig || nullptr == rankData)
	{
		TalkToDnid(m_ClientIndex, CMyString::GetInstance().GetFormatString("EQUIPMENT_NOT_FOUND"));//找不到装备
		return false;
	}

	if (!m_upStorageMgr->AttachEquipment(euquipID))
	{
		TalkToDnid(m_ClientIndex, CMyString::GetInstance().GetFormatString("EQUIPMENT_NOT_FOUND"));//找不到装备
		return false;
	}
		
	if (__super::EquipHero(heroID, euquipID, equipType) == false)
	{
		TalkToDnid(m_ClientIndex, CMyString::GetInstance().GetFormatString("ITEM_ALREADY_EQUIPPED")); // 物品已装备
		return false;
	}
		
	return true;
}

void CBaseDataManager::ResetDataForNewDay()
{
	if (_checkSFixData() == false)
		return;
	///<将格商店的刷新购买次数置为0
	m_pPlayerData->m_VarietyShopData.m_dwRefreshedCount = 0;
	m_pPlayerData->m_ArenaShopData.m_dwRefreshedCount = 0;
	m_pPlayerData->m_ExpeditionShopData.m_dwRefreshedCount = 0;
	m_pPlayerData->m_MiracleMerchantData.mallNormalData.m_dwRefreshedCount = 0;
	m_pPlayerData->m_GemMerchantData.mallNormalData.m_dwRefreshedCount = 0;
	m_pPlayerData->m_LegionShopData.m_dwRefreshedCount = 0;
	m_pPlayerData->m_SkillPointData.m_dwBuySkillPointTime = 0;
}

void CBaseDataManager::OnRecvStorageOperation(SSGPlayerMsg* pMsg)
{
	SStorageBaseMsg* baseMsg = static_cast<SStorageBaseMsg*>(pMsg);
	if (baseMsg == nullptr)
		return;

	switch (baseMsg->_protocol)
	{
	case SStorageBaseMsg::SStorage_SellItem:
	{
		SRequstSellItem* sellMsg = static_cast<SRequstSellItem*>(baseMsg);
		if (sellMsg == nullptr)
			break;

		SSellItemResultMsg resultMsg;
		if (m_upStorageMgr->SellItem(sellMsg->m_dwItemID, sellMsg->m_dwProcessCount, resultMsg.m_CurrencyType, resultMsg.m_dwSellCount, resultMsg.m_dwProperty))
		{
			///加钱
			resultMsg.m_dwProperty = PlusMoney(resultMsg.m_dwProperty);
			resultMsg.m_dwItemID = sellMsg->m_dwItemID;
			g_StoreMessage(GetDNID(), &resultMsg, sizeof(SSellItemResultMsg));
		}
	}
		break;
	case SStorageBaseMsg::REQUEST_USECONSUMABLE:
	{
		SStorageItemProcess* useMsg = static_cast<SStorageItemProcess*>(baseMsg);
		if (useMsg == nullptr)
			break;

		///判断是否使用失败，如果失败了的话回复客户端说明失败
		if (!m_upStorageMgr->UseConsumable(useMsg->m_dwItemID, useMsg->m_dwProcessCount))
		{
			SUseConsumableAnswer answerMsg;
			///告诉客户端使用消耗品失败
			answerMsg.itemNum = 0;
			g_StoreMessage(GetDNID(), &answerMsg, sizeof(SUseConsumableAnswer) - sizeof(DWORD) * MAX_ITEM_NUM_GAINED_FROM_CHEST);
		}
	}
	break;
	case SStorageBaseMsg::REQUEST_SYNTHETIZE:
	{
		SQSynthetizeItem* synthetizeMsg = static_cast<SQSynthetizeItem*>(baseMsg);
		if (synthetizeMsg == nullptr)
			break;

		SASynthetizeItemInfor replyInforMsg;
		replyInforMsg.bsucceed = false;
		const ItemConfig* itemConfig = CConfigManager::getSingleton()->GetItemConfig(synthetizeMsg->debrisID);
		if (itemConfig == nullptr)
		{
			g_StoreMessage(GetDNID(), &replyInforMsg, sizeof(SASynthetizeItemInfor));
			break;
		}

		int synthetizeCost = itemConfig->SellPrice * itemConfig->ComposeNum * synthetizeMsg->synthetizeTimes;
		///检查合成装备的钱、碎片是否够用
		if (!CheckGoods_SG(GoodsType::money, 0, synthetizeCost) || !CheckGoods_SG(GoodsType::item, synthetizeMsg->debrisID, itemConfig->ComposeNum * synthetizeMsg->synthetizeTimes))
		{
			g_StoreMessage(GetDNID(), &replyInforMsg, sizeof(SASynthetizeItemInfor));
			break;
		}
		else
		{
			DecGoods_SG(GoodsType::item, synthetizeMsg->debrisID, itemConfig->ComposeNum * synthetizeMsg->synthetizeTimes, GoodsWay::itemUse);
			DecGoods_SG(GoodsType::money, 0, synthetizeCost, GoodsWay::itemUse);
			AddGoods_SG(GoodsType::item, itemConfig->ComposeItemID, synthetizeMsg->synthetizeTimes, GoodsWay::itemUse);
			replyInforMsg.bsucceed = true;
			g_StoreMessage(GetDNID(), &replyInforMsg, sizeof(SASynthetizeItemInfor));
		}
	}
		break;
	case SStorageBaseMsg::REQUEST_RESOLVEITEM:
	{
		SRequestResolveItemMsg* resolveMsg = static_cast<SRequestResolveItemMsg*>(baseMsg);
		if (resolveMsg == nullptr)
			break;

		SAnswerResolveItemMsg replyInforMsg;
		replyInforMsg.bSuccess = false;
		int soulPointsProceeds = 0;
		if (m_upStorageMgr->GetResolvedPoints(resolveMsg->itemArrayNum, resolveMsg->resolveItemArray, soulPointsProceeds))
		{
			replyInforMsg.bSuccess = true;
			DecGoods_SG(GoodsType::item, resolveMsg->itemArrayNum, resolveMsg->resolveItemArray + resolveMsg->itemArrayNum, resolveMsg->resolveItemArray, GoodsWay::itemUse);
			AddGoods_SG(GoodsType::soulPoints, 0, soulPointsProceeds, GoodsWay::itemUse);
		}

		g_StoreMessage(GetDNID(), &replyInforMsg, sizeof(SAnswerResolveItemMsg));
	}
		break;
	default:
		break;
	}
}

void CBaseDataManager::OnRecvGuideOperation(SSGPlayerMsg *pMsg)
{
	SNewbieGuideMsg* pGuideMsg = static_cast<SNewbieGuideMsg*>(pMsg);
	if (nullptr == pGuideMsg)
		return;

	switch (pGuideMsg->_protocol)
	{
	case SNewbieGuideMsg::Guide_SetInfo:
		{
			SQSetNewbieGuideInfo *pInfo = static_cast<SQSetNewbieGuideInfo*>(pGuideMsg);
			if (nullptr == pInfo)
				return;
			m_upGuideMgr->SetGuideData(pInfo->index, pInfo->info);
		}
		break;
	case SNewbieGuideMsg::Guide_ChangeScene:
		{
			SChangeScene msg;
			g_StoreMessage(GetDNID(), &msg, sizeof(SChangeScene));
		}
		break;
	default:
		break;
	}
}

void CBaseDataManager::ProcessOperationOfGuide(FunctionMoudleType functionType, int rewardID)
{
	if (m_upGuideMgr->ProcessOperation(functionType, rewardID))
	{
		SAGoNextStep msg;
		g_StoreMessage(GetDNID(), &msg, sizeof(SAGoNextStep));
	}
}

DWORD CBaseDataManager::GetPlayerLevel() const
{
	return m_pPlayerData->m_dwLevel;
}

int32_t CBaseDataManager::GetHonor() const
{
	return m_pPlayerData->m_dwHonor;
}

int CBaseDataManager::PlusExploit(int32_t value)
{
	if (!_checkSFixData())
		return 0;

	if (value + (int32_t)m_pPlayerData->m_dwExploit < 0)
	{
		m_pPlayerData->m_dwExploit = 0;
		return 0;
	}

	m_pPlayerData->m_dwExploit += value;
	return m_pPlayerData->m_dwExploit;
}

int CBaseDataManager::PlusHonor(int32_t value)
{
	if (!_checkSFixData())
		return 0;

	if (value + (int32_t)m_pPlayerData->m_dwHonor < 0)
	{
		m_pPlayerData->m_dwHonor = 0;
		return 0;
	}
	m_pPlayerData->m_dwHonor += value;
	return m_pPlayerData->m_dwHonor;
}

int CBaseDataManager::PlusPrestige(int32_t value)
{
	if (!_checkSFixData())
		return 0;

	if (value + (int32_t)m_pPlayerData->m_dwPrestige < 0)
	{
		m_pPlayerData->m_dwPrestige = 0;
		return 0;
	}
	m_pPlayerData->m_dwPrestige += value;
	return m_pPlayerData->m_dwPrestige;
}

int CBaseDataManager::ModifyBlessPoints(int value)
{
	if (!_checkSFixData())
		return 0;

	if (value + (int32_t)m_pPlayerData->m_dwBlessPoints < 0)
	{
		m_pPlayerData->m_dwBlessPoints = 0;
		return 0;
	}
	m_pPlayerData->m_dwBlessPoints += value;
	return m_pPlayerData->m_dwBlessPoints;
}

int CBaseDataManager::ModifySoulPoints(int value)
{
	if (!_checkSFixData())
		return 0;

	if (value + (int32_t)m_pPlayerData->m_dwSoulPoints < 0)
	{
		m_pPlayerData->m_dwSoulPoints = 0;
		return 0;
	}
	m_pPlayerData->m_dwSoulPoints += value;
	return m_pPlayerData->m_dwSoulPoints;
}

int CBaseDataManager::ModifyToken(int value)
{
	if (!_checkSFixData())
		return 0;

	if (value + (int32_t)m_pPlayerData->m_dwToken < 0)
	{
		m_pPlayerData->m_dwToken = 0;
		return 0;
	}
	m_pPlayerData->m_dwToken += value;
	return m_pPlayerData->m_dwToken;
}

void CBaseDataManager::PlusExp(int32_t value)
{
	if (value < 0)
		return;

	DWORD targetExp = m_pPlayerData->m_dwExp + value;
	DWORD tempLevel = m_pPlayerData->m_dwLevel;
	///获取对应的的经验等级信息，如果经验值有超出最大经验值，截断
	CConfigManager::getSingleton()->GetMasterExpAndLevel(targetExp, tempLevel);

	m_pPlayerData->m_dwExp = targetExp;
	if (m_pPlayerData->m_dwLevel != tempLevel)
	{
		int enduranceIncreasement = 0;

		///更新君主等级相关的成就
		//////////////////////////////////////////////////////////////////////////
		if (m_ptrAcievementUpdate != nullptr)
			m_ptrAcievementUpdate->UpdateMasterLevelAchieve(tempLevel, tempLevel - m_pPlayerData->m_dwLevel);
		if (m_ptrRandomAchievementUpdate != nullptr)
			m_ptrRandomAchievementUpdate->UpdateMasterLevelAchieve(tempLevel, tempLevel - m_pPlayerData->m_dwLevel);
		//////////////////////////////////////////////////////////////////////////
		///判断是否解锁了缘分系统,解锁需要计算所有武将的缘分属性
		int fateOpenLevel = CConfigManager::getSingleton()->GetGameFeatureActivationConfig().Relationship;
		bool calcFateAttribute = m_bTempGMFlag ? false : (m_pPlayerData->m_dwLevel < fateOpenLevel && tempLevel >= fateOpenLevel);
		//////////////////////////////////////////////////////////////////////////

		while (m_pPlayerData->m_dwLevel < tempLevel)
		{
			const MasterLevelInfor* levelInfor = CConfigManager::getSingleton()->GetMasterLevelInfor(m_pPlayerData->m_dwLevel);
			if (levelInfor != nullptr)
			{
				///升级后会有一定的体力值奖励
				enduranceIncreasement += levelInfor->CharactorActionRecover;
			}
			++m_pPlayerData->m_dwLevel;
		}

		///升级后会有一定的体力值奖励
		AddGoods_SG(GoodsType::endurance, 0, enduranceIncreasement,GoodsWay::lvup);
		///上面判断需要计算武将缘分属性 则进行计算
		if (calcFateAttribute)
			UpdateFateAttributeOfAllHeroes();

		///往客户端同步君主等级信息
		AddGoods_SG(GoodsType::level, 0, tempLevel, GoodsWay::lvup);
		//向脚本中广播玩家升级信息
		g_Script.SetPlayer(cPlayer);
		LuaFunctor(g_Script, "OnPlayerLevelUp")[m_pPlayerData->m_dwStaticID][m_pPlayerData->m_dwLevel]();
	}
}

int32_t CBaseDataManager::GetExploit() const
{
	return m_pPlayerData->m_dwExploit;
}

const STimeData& CBaseDataManager::GetTimeData() const
{
	return m_pPlayerData->m_TimeData;
}

bool CBaseDataManager::IsNewPlayer()
{
	return m_pPlayerData->m_bNewPlayer == 0 ? false : true;
}

bool CBaseDataManager::AddHero(const int32_t heroID)
{
	bool bAddSuccess = false;
	if (ExistHero(heroID)) //如果该武将已经存在 
	{
		bAddSuccess = true;
		int heroSoul = heroID + 90000;
		const HeroConfig* heroConfig = CConfigManager::Instance()->GetHeroConfig(heroID);
		if (heroConfig == nullptr)
			return false;
		//根据星级获取将魂个数
		int heroPiceces = 0;
		CGlobalConfig globalConfig = CConfigManager::Instance()->globalConfig;
		switch (heroConfig->Star)
		{
		case 1:
			heroPiceces = globalConfig.OneStarHeroTransform;
			break;
		case 2:
			heroPiceces = globalConfig.TwoStarHeroTransform;
			break;
		case 3:
			heroPiceces = globalConfig.ThreeStarHeroTransform;
			break;
		default:
			break;
		}

		m_upStorageMgr->AddItem(heroSoul, heroPiceces);
	}
	else
	{
		bAddSuccess = __super::AddHero(heroID);

		if (bAddSuccess)
		{
			if (m_ptrAcievementUpdate != nullptr)
				m_ptrAcievementUpdate->UpdateHeroCollectionAchieve();

			if (m_ptrRandomAchievementUpdate != nullptr)
				m_ptrRandomAchievementUpdate->UpdateHeroCollectionAchieve();

			///更新武将总数榜
			g_Script.SetCondition(nullptr, cPlayer, nullptr);
			if (g_Script.PrepareFunction("rank_update"))
			{
				g_Script.PushParameter(m_pPlayerData->m_dwStaticID);
				g_Script.PushParameter(RankType_SG::HeroNum);
				g_Script.PushParameter(m_HeroNum);
				g_Script.Execute();
			}
			///更新英雄收集活动
			if (g_Script.PrepareFunction("ActOnHeroNumInc"))
			{
				g_Script.PushParameter(m_pPlayerData->m_dwStaticID);
				g_Script.PushParameter(m_HeroNum);
				g_Script.Execute();
			}
			if (g_Script.PrepareFunction("rank_update"))
			{
				g_Script.PushParameter(m_pPlayerData->m_dwStaticID);
				g_Script.PushParameter(RankType_SG::HeroAllStar);
				g_Script.PushParameter(GetHeroStarSum());
				g_Script.Execute();
			}
		}
	}

	return bAddSuccess;
}

bool CBaseDataManager::HeroRankRise(const int32_t heroID, const DWORD curRank)
{
	///先判断是否可以升阶
	if (!CanRiseRank(heroID))
		return false;

	const SHeroData* pHeroData = GetHero(heroID);
	if (pHeroData == nullptr)
		return false;

	if (pHeroData->m_dwRankLevel != curRank)
	{
		rfalse("客户端武将品阶数据出错 ID : %d", heroID);
		return false;
	}
	
	///获取指定英雄的所有装备
	const SHeroEquipment* const pHeroEquipments = GetHeroEquipments(heroID);
	if (pHeroEquipments == nullptr)
		return false;

	const EquipmentConfig * equipConfig = nullptr;
	///判断英雄的所有装备，看有没有锻造过的，有的话，进阶成功后返回对应个数的玉石出来
	for (int i = 0; i < EQUIP_MAX; ++i)
	{
		if (pHeroEquipments[i].m_dwUpgradeExp <= 0)
			continue;

		CGlobalConfig& globalConfig = CConfigManager::getSingleton()->globalConfig;
		equipConfig = CConfigManager::getSingleton()->GetEquipment(pHeroEquipments[i].m_dwEquipID);
		if (equipConfig == nullptr)
			continue;;
		int curForgeExp = 0;
		switch (equipConfig->Quality)
		{
		case 2:
			curForgeExp = pHeroEquipments[i].m_dwUpgradeExp * globalConfig.GreenEquipForRestore;
			break;
		case 3:
			curForgeExp = pHeroEquipments[i].m_dwUpgradeExp * globalConfig.BluenEquipForRestore;
			break;
		case 4:
			curForgeExp = pHeroEquipments[i].m_dwUpgradeExp * globalConfig.PurpleEquipForRestore;
			break;
		}

		int restoreItemId = globalConfig.ForgeExpRestoreItem;
		const ItemConfig * itemConfig = CConfigManager::getSingleton()->GetItemConfig(restoreItemId);
		if (itemConfig == nullptr)
			continue;

		///返还一定数量的能增加装备精练经验值的道具
		if (itemConfig->RefiningExp != 0)
			AddGoods_SG(GoodsType::item, restoreItemId, curForgeExp / itemConfig->RefiningExp, GoodsWay::forgeMall);
	}

	///提升武将的品阶并移除武将现有的所有的装备
	IncreaseHeroRank(heroID);
	ClearHeroEquipment(heroID);

	auto attrFindResult = m_heroAttrlist.find(heroID);
	if (attrFindResult != m_heroAttrlist.end() && attrFindResult->second != nullptr)
	{
		///先移除旧属性的作用
		attrFindResult->second->curAttr -= attrFindResult->second->RankAttribute;
		attrFindResult->second->curAttr -= attrFindResult->second->EquipmentAttribute;
		attrFindResult->second->EquipmentAttribute.Reset();
		/// 计算等级加成属性
		CHeroAttributeCalculator::getSingleton()->CalculateHeroRankAttribute(heroID, pHeroData->m_dwRankLevel, attrFindResult->second->RankAttribute);
		/// 合计属性
		attrFindResult->second->curAttr += attrFindResult->second->RankAttribute;
		/// 计算战斗力
		int combatPower = CHeroAttributeCalculator::getSingleton()->CalculateCombatPower(pHeroData->m_SkillInfoList, attrFindResult->second.get());
		if (combatPower != 0)
			ProcessAfterCombatPowerCalculated(heroID, combatPower);
		return true;
	}
	else
	{
		rfalse("找不到英雄%d实时的属性值", heroID);
		return false;
	}

	return true;
}

bool CBaseDataManager::HeroStarLevelRise(const int32_t heroID, const DWORD curStarLevel)
{
	if (CanRiseStarLevel(heroID) == false)
		return false;

	const HeroStarConfig* config = CConfigManager::getSingleton()->GetHeroStarAttr(heroID);
	if (config == nullptr)
		return false;

	auto nextStarLevelInfo = config->StarData.find(curStarLevel + 1);
	auto curStarLevelInfo = config->StarData.find(curStarLevel);
	if (nextStarLevelInfo == config->StarData.end() || curStarLevelInfo == config->StarData.end())
		return false;

	int32_t num = nextStarLevelInfo->second.Number - curStarLevelInfo->second.Number;
	if (CheckGoods_SG(GoodsType::item, nextStarLevelInfo->second.Item, num) == false)
		return false;

	DecGoods_SG(GoodsType::item, nextStarLevelInfo->second.Item, num, GoodsWay::itemUse);

	if (__super::HeroStarLevelRise(heroID, curStarLevel) == false)
		return false;

	///更新总星榜
	g_Script.SetCondition(nullptr, cPlayer, nullptr);
	if (g_Script.PrepareFunction("rank_update"))
	{
		g_Script.PushParameter(m_pPlayerData->m_dwStaticID);
		g_Script.PushParameter(RankType_SG::HeroAllStar);
		g_Script.PushParameter(GetHeroStarSum());
		g_Script.Execute();
	}

	return true;
}

bool CBaseDataManager::PlusHeroExp(int heroID, OUT int& value, OUT int& level, OUT bool& levelLimitted)
{
	if (m_pPlayerData == nullptr)
		return false;

	const SHeroData* heroData = GetHero(heroID);
	if (heroData == nullptr)
		return false;

	DWORD targetExp = heroData->m_Exp + value;
	DWORD tempLevel = heroData->m_dwLevel;
	const MasterLevelInfor* masterLevelInfor = CConfigManager::getSingleton()->GetMasterLevelInfor(m_pPlayerData->m_dwLevel);
	if (masterLevelInfor == nullptr)
	{
		rfalse("无法找到当前君主等级对应的君主等级信息，故现在无法增加武将经验");
		return false;
	}

	///如果已经达到当前的君主等级限制的最大等级，则不予继续增加英雄的经验
	if (masterLevelInfor->CharactorLevelLimit <= tempLevel)
		return false;

	int curExpLimit = CConfigManager::getSingleton()->GetHeroNeededExp(masterLevelInfor->CharactorLevelLimit);
	targetExp = min(curExpLimit, targetExp);

	///获取对应的的经验等级信息，如果经验值有超出最大经验值，截断
	CConfigManager::getSingleton()->GetHeroExpAndLevel(tempLevel, targetExp, levelLimitted);

	value = targetExp;
	level = tempLevel;

	bool success = __super::PlusHeroExp(heroID, value, level);
	///更新英雄等级提升的活动
	if (success && g_Script.PrepareFunction("OnHeroLevelUp"))
	{
		g_Script.PushParameter(m_pPlayerData->m_dwStaticID);
		g_Script.PushParameter(heroID);
		g_Script.PushParameter(level);
		g_Script.Execute();
	}
	return success;
}

CAchieveUpdate* CBaseDataManager::GetLifeTimeAchievementUpdator()
{
	return m_ptrAcievementUpdate;
}

CRandomAchieveUpdate* CBaseDataManager::GetRandomAchievementUpdator()
{
	return m_ptrRandomAchievementUpdate;
}

CMissionUpdate* CBaseDataManager::GetMissionUpdator()
{
	return m_ptrMissionUpdate;
}

void CBaseDataManager::JudgeHeroUpgradeRank(int itemID)
{
	///如果添加的物品不是装备
	if (CConfigManager::getSingleton()->GetEquipment(itemID) == nullptr)
		return;

	vector<int32_t> vecHeroID;
	GetAllHeroID(vecHeroID);
	const SHeroData* heroInfo = nullptr;
	const HeroRankConfig* rankConfig = nullptr;
	const HeroRankData* rankData = nullptr;
	const EquipmentConfig* equipConfig = nullptr;

	int havedCount;
	for (auto heroID : vecHeroID)
	{
		havedCount = 0;
		heroInfo = GetHero(heroID);
		rankConfig = CConfigManager::getSingleton()->GetHeroRankConfig(heroID);
		if (nullptr == rankConfig)
			continue;
		rankData = rankConfig->GetRankData(heroInfo->m_dwRankLevel + 1);
		if (nullptr == rankData)
			continue;

		for (auto equipID : rankData->m_EquipIDs)
		{
			equipConfig = m_upStorageMgr->ItemExistOrEnough(equipID) ? CConfigManager::getSingleton()->GetEquipment(equipID) : nullptr;

			if (nullptr != equipConfig && equipConfig->levelLimit <= heroInfo->m_dwLevel)
				havedCount++;
			else if (IsHeroEquipmentAttached(heroID, equipID))
				havedCount++;
			else
				break;
		}

		if (havedCount == rankData->m_EquipIDs.size())
		{
			if (m_upGuideMgr->TriggerCollectEquipment())
				m_upStorageMgr->RemoveAddItemFunction();
			return;
		}
	}
}

bool CBaseDataManager::CanCalculateFateAttribute()
{
	return GetPlayerLevel() >= CConfigManager::getSingleton()->GetGameFeatureActivationConfig().Relationship || m_bTempGMFlag;
}

DWORD CBaseDataManager::GetIconID()
{
	return m_pPlayerData->m_PlayerIconID;
}

void CBaseDataManager::ProcessAfterCombatPowerCalculated(int heroID, int combatPower)
{
	__super::ProcessAfterCombatPowerCalculated(heroID, combatPower);

	const SHeroData* tempData = GetHero(heroID);
	if (tempData == nullptr)
		return;

	if (m_bDataInitialed)
	{
		///更新名将榜
		g_Script.SetCondition(nullptr, cPlayer, nullptr);
		if (g_Script.PrepareFunction("rank_update"))
		{
			g_Script.PushParameter(m_pPlayerData->m_dwStaticID);
			g_Script.PushParameter(RankType_SG::MaxHero);
			g_Script.PushParameter(combatPower);
			g_Script.PushParameter(heroID);
			g_Script.PushParameter(tempData->m_dwStarLevel);
			g_Script.PushParameter(tempData->m_dwRankLevel);
			g_Script.PushParameter(tempData->m_dwLevel);
			g_Script.Execute();
		}

		///下面开始更新阵容榜的战斗力
		if (m_pDefenseTeamID == nullptr)
			return;

		for (int i = 0; i < 5; ++i)
		{
			if (m_pDefenseTeamID[i] == heroID)
			{
				OnArenaDefenseTeamChanged();
				break;
			}
		}
	}
}

void CBaseDataManager::OnArenaDefenseTeamChanged()
{
	if (m_pDefenseTeamID == nullptr)
		return;

	int combatPowerSum = 0;
	try
	{
		for (int i = 0; i < 5; ++i)
		{
			const SHeroAttr* attr = GetHeroAttr(m_pDefenseTeamID[i]);
			if (attr == nullptr)
				continue;

			combatPowerSum += attr->CombatPower;
		}
	}
	catch (const std::exception& oor)
	{
		rfalse(oor.what());
	}

	///更新阵容榜
	g_Script.SetCondition(nullptr, cPlayer, nullptr);
	if (g_Script.PrepareFunction("rank_update"))
	{
		g_Script.PushParameter(m_pPlayerData->m_dwStaticID);
		g_Script.PushParameter(RankType_SG::BattleTeam);
		g_Script.PushParameter(combatPowerSum);
		g_Script.Execute();
	}
}

void CBaseDataManager::UpdateDateForVIP(int vipLevel)
{
	if (nullptr != m_ptrMissionUpdate)
		m_ptrMissionUpdate->UpdateVipMission(vipLevel);

	if (nullptr != m_upEnduranceMgr)
	{
		m_upEnduranceMgr->UpdateEnduranceForVIP(GetSID(), GetPlayerLevel());
		SAAddGoodsSYNMsg rMsg;
		rMsg.iType = GoodsType::endurance;
		rMsg.itemGroupNum = 1;
		rMsg.bIDIndex = true;
		int unusedNum = MAX_ITEM_NUM_SYN_TO_CLIENT - 2;
		rMsg.itemList[0] = m_pPlayerData->m_EnduranceData.m_dwEndurance;
		rMsg.itemList[1] = m_pPlayerData->m_EnduranceData.m_dwRemainingSeconds;
		///将没用的数组空间去掉，不发送到服务器
		g_StoreMessage(GetSID(), &rMsg, sizeof(SAAddGoodsSYNMsg) - unusedNum * sizeof(DWORD));
	}
}
