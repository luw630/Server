#include "StdAfx.h"
#include "ScriptManager.h"
#include "trigger.h"
#include "Player.h"
#include <time.h>
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "scriptmoudle.h"
#include "Networkmodule/PlayerBaseInf.h"
#include "../DuplicateMoudle/LeagueOfLegendDuplicateMananger.h"
#include "../区域服务器/SocketAndThreadObj.h"
#include "..\BaseDataManager.h"
#include "../HeroMoudle/HeroDataManager.h"
#include "NetWorkModule\GmToolMsgs.h"
#include "../DessertMoudle/DessertSystem.h"
#include "../Common/PubTool.h"
#include "..\MailMoudle\MailMoudle.h"


extern LPIObject GetPlayerBySID(DWORD dwStaticID);
extern DNID gm_dnidClient;

//注册函数到lua
//统一标准,注册往lua都以CI开头,以便lua区分c函数和自身函数
void CSSanGuoFunc::CreateShadow()
{

	g_Script.RegisterFunction("CI_GetPlayerDataSG", L_GetPlayerDataSG);
	g_Script.RegisterFunction("CI_SetPlayerDataSG", L_SetPlayerDataSG);
	g_Script.RegisterFunction("CI_GetGoods_SG", L_GetGoods);
	g_Script.RegisterFunction("CI_AddGoods_SG", L_AddGoods_SG);
	g_Script.RegisterFunction("CI_AddGoodsArray_SG", L_AddGoodsArray_SG);
	g_Script.RegisterFunction("CI_DecGoods_SG", L_DecGoods_SG);
	g_Script.RegisterFunction("CI_CheckGoods_SG", L_CheckGoods_SG);
	g_Script.RegisterFunction("CI_GiveCheckinAwards", L_GiveCheckinAwards);
	g_Script.RegisterFunction("CI_SendLoginGMRes", L_SendLoginGMRes);
	g_Script.RegisterFunction("CI_SynNotificationDataSG", L_SynNotificationDataSG);
	g_Script.RegisterFunction("CI_SynMissionData", L_SynMissionDataSG);
	g_Script.RegisterFunction("CI_UpdateMissionState", L_UpdateMissionStateSG);
	g_Script.RegisterFunction("CI_SendCheckinData", L_SendCheckinData);
	g_Script.RegisterFunction("CI_TipMsg", L_TipMsg);
	g_Script.RegisterFunction("CI_SynPlayerDataSG", L_SynPlayerDataSG);
	g_Script.RegisterFunction("CI_SendMsg_SG", L_SendMsg_SG);
	g_Script.RegisterFunction("CI_ActiveLeagueOfLegendDuplicate", L_ActiveLeagueOfLegend);
	g_Script.RegisterFunction("CI_gm_registerNum", L_gm_registerNum);
	g_Script.RegisterFunction("CI_gm_stayAll", L_gm_stayAll);
	g_Script.RegisterFunction("CI_gm_stayOne", L_gm_stayOne);
	g_Script.RegisterFunction("CI_gm_online", L_gm_online);
	g_Script.RegisterFunction("CI_gm_recharge", L_gm_recharge);
	g_Script.RegisterFunction("CI_gm_leave", L_gm_leave);
	g_Script.RegisterFunction("CI_gm_pointuse", L_gm_pointuse);
	g_Script.RegisterFunction("CI_gm_operation", L_gm_operation);

	g_Script.RegisterFunction("CI_UpdateVIPLevel", L_UpdateVIPLevel);
	g_Script.RegisterFunction("CI_DessertDispachInfor", L_SetDessertDispachInfor);
	g_Script.RegisterFunction("CI_UseComsumable", L_ComsumableUsedInfor);
	g_Script.RegisterFunction("CI_AddSkillPoint", L_AddSkillPoint);
	g_Script.RegisterFunction("CI_SynHeroExtendsData", L_SynHeroExtendsData);

	g_Script.RegisterFunction("CI_ChangeName", L_ChangeName);
	g_Script.RegisterFunction("CI_ActResult", L_SendActResult);
	g_Script.RegisterFunction("CI_SendFirstRechargeRes", L_SendFirstRechargeRes);
	g_Script.RegisterFunction("CI_WeekReMoney", L_WeekReMoney);
	g_Script.RegisterFunction("CI_SendChargeSumResult", L_SendChargeSumResult); 

	g_Script.RegisterFunction("CI_GetHeroInfo", L_GetHeroInfo);
	g_Script.RegisterFunction("CI_WarMsg", L_WarMsg);
	g_Script.RegisterFunction("CI_WarMail", L_WarMail);
	g_Script.RegisterFunction("CI_RankMsg", L_RankMsg);
	
	g_Script.RegisterFunction("CI_SynMembershipToClient", L_SynMembershipToClient);
	g_Script.RegisterFunction("CI_exchangeActRes", L_exchangeActRes);
	g_Script.RegisterFunction("CI_exchangeActSendData", L_exchangeActSendData);
	
}

//取三国玩家数据
int CSSanGuoFunc::L_GetPlayerDataSG(lua_State *L)
{
	DWORD dwSID = static_cast<DWORD>(lua_tonumber(L, 1));
	CPlayer* pPlayer;
	if (dwSID == 0)
	{
		pPlayer = g_Script.m_pPlayer;
	}
	else
	{
		pPlayer = (CPlayer *)GetPlayerBySID(dwSID)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer) return 0;

	int index = static_cast<int>(lua_tonumber(L, 2));
	switch (index)
	{
	case 0:	//sid
		lua_pushnumber(L, pPlayer->GetSID());
		break;
	case 1:	//money
		lua_pushnumber(L, pPlayer->m_FixData.m_dwMoney);
		break;
	case 2:	//diamond
		lua_pushnumber(L, pPlayer->m_FixData.m_dwDiamond);
		break;
	case 3: //player Level
		lua_pushnumber(L, pPlayer->m_FixData.m_dwLevel);
		break;
	case 4: //player Name
		lua_pushstring(L, pPlayer->m_FixData.m_Name);
		break;
	case 7:	//充值总额
		lua_pushnumber(L, pPlayer->m_FixData.m_dwMaxRecharge);
		break;
	case 8: //获取战斗力
	{
		CBaseDataManager* baseDataMgr = pPlayer->GetSanguoBaseData();
		if (baseDataMgr == nullptr)
			return 0;
		lua_pushnumber(L, baseDataMgr->GetCombatPower());
	}
	break;
	case 9://获取iconid
	{
		CBaseDataManager* baseDataMgr = pPlayer->GetSanguoBaseData();
		if (baseDataMgr == nullptr)
			return 0;
		lua_pushnumber(L, baseDataMgr->GetIconID());
	}
	break;
	case 10://获取工会ID
	{
		//lua_pushnumber(L, pPlayer->m_FixData.m_dFactionID[0]); //TODO 为老数据做兼容，正式上线需要修改
		byte faction[4];
		memcpy(&faction, &pPlayer->m_FixData.m_FactionName, 4);
		DWORD* factionID = (DWORD*)faction;
		lua_pushnumber(L, *factionID);
		rfalse("获取军团ID%d", *factionID);
	}
	break;
	case 11://获取工会名
	{
		lua_pushstring(L, pPlayer->m_FixData.m_Name);
	}
	break;
	default:
		break;
	};
	return 1;
}

int CSSanGuoFunc::L_GetGoods(lua_State *L)
{
	CPlayer* pPlayer;
	int sid = static_cast<int>(lua_tonumber(L, 3));
	if (sid > 0)//指定玩家
		pPlayer = (CPlayer *)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	else
		pPlayer = g_Script.m_pPlayer;

	if (pPlayer == NULL)
	{
		rfalse("L_GetGoods---m_pPlayer=null");
		return 0;
	}
	int iType = static_cast<int>(lua_tonumber(L, 1));
	int id = static_cast<int>(lua_tonumber(L, 2));

	auto baseDataManager = pPlayer->GetSanguoBaseData();
	if (baseDataManager == nullptr)
		return 0;

	lua_pushnumber(L, baseDataManager->GetGoods(iType, id));

	return 1;
}

int CSSanGuoFunc::L_AddGoods_SG(lua_State *L)	//增加玩家点数数据
{
	CPlayer* pPlayer;
	int sid = static_cast<int>(lua_tonumber(L, 5));
	if (sid > 0)//指定玩家
		pPlayer = (CPlayer *)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	else
		pPlayer = g_Script.m_pPlayer;

	if (pPlayer == NULL)
	{
		rfalse("L_AddGoods_SG---m_pPlayer=null");
		return 0;
	}
	int iType = static_cast<int>(lua_tonumber(L, 1));
	int id = static_cast<int>(lua_tonumber(L, 2));
	int num = static_cast<int>(lua_tonumber(L, 3));
	int detailType = static_cast<int>(lua_tonumber(L, 4));//功能类型 GoodsWay

	lua_settop(L, 0);
	pPlayer->GetSanguoBaseData()->AddGoods_SG(iType, id, num, detailType);

	return 1;
}

int CSSanGuoFunc::L_AddGoodsArray_SG(lua_State *L)	//增加玩家点数数据
{
	if (g_Script.m_pPlayer == NULL)
	{
		rfalse("L_AddGoodsArray_SG---m_pPlayer=null");
		return 0;
	}
	///先判断后2个参数是否是table
	if (!lua_istable(L, 2) || !lua_istable(L, 3))
	{
		rfalse("批量加物品时lua端传过来的参数不为数组");
		return 0;
	}

	///获取要批量添加的物品的类型
	int iType = static_cast<int>(lua_tonumber(L, 1));
	int idList[MAX_ITEM_NUM_SYN_TO_CLIENT / 2];
	int numList[MAX_ITEM_NUM_SYN_TO_CLIENT / 2];
	int itemNum = 0;

	int stackSize = lua_gettop(L);
	///开始将table的值填入数组中
	lua_pushvalue(L, 2);
	lua_pushnil(L);
	int itemIndex = 0;
	while (0 != lua_next(L, -2))
	{
		idList[itemIndex] = static_cast<int>(lua_tonumber(L, -1));
		lua_remove(L, -1);
		++itemIndex;
	}
	lua_settop(L, stackSize);
	itemNum = itemIndex;

	///开始将table的值填入数组中
	lua_pushvalue(L, 3);
	lua_pushnil(L);
	itemIndex = 0;
	while (0 != lua_next(L, -2))
	{
		numList[itemIndex] = static_cast<int>(lua_tonumber(L, -1));
		lua_remove(L, -1);
		++itemIndex;
	}
	lua_settop(L, stackSize);

	lua_pop(L, 1);
	if (itemNum != itemIndex)
	{
		rfalse("批量添加游戏点数中，lua传过来的数组的大小有误");
		return 0;
	}
	int detailType = static_cast<int>(lua_tonumber(L, 4));//功能类型 GoodsWay
	g_Script.m_pPlayer->GetSanguoBaseData()->AddGoods_SG(iType, itemNum, numList, idList, detailType);
	return 0;
}

int CSSanGuoFunc::L_DecGoods_SG(lua_State *L)	//扣除玩家点数数据
{
	CPlayer* pPlayer;
	int sid = static_cast<int>(lua_tonumber(L, 5));
	if (sid > 0)//指定玩家
		pPlayer = (CPlayer *)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	else
		pPlayer = g_Script.m_pPlayer;

	if (pPlayer == NULL)
	{
		rfalse("L_AddGoods_SG---m_pPlayer=null");
		return 0;
	}

	int iType = static_cast<int>(lua_tonumber(L, 1));
	int id = static_cast<int>(lua_tonumber(L, 2));
	int num = static_cast<int>(lua_tonumber(L, 3));
	int detailType = static_cast<int>(lua_tonumber(L, 4));//功能类型 GoodsWay

	pPlayer->GetSanguoBaseData()->DecGoods_SG(iType, id, num, detailType);
	return 1;
}
int CSSanGuoFunc::L_CheckGoods_SG(lua_State *L)	//检查玩家点数数据
{
	if (g_Script.m_pPlayer == NULL)
	{
		rfalse("L_CheckGoods_SG---m_pPlayer=null");
		return 0;
	}
	int iType = static_cast<int>(lua_tonumber(L, 1));
	int arg1 = static_cast<int>(lua_tonumber(L, 2));
	int arg2 = static_cast<int>(lua_tonumber(L, 3));
	if (g_Script.m_pPlayer->GetSanguoBaseData()->CheckGoods_SG(iType, arg1, arg2))
	{
		lua_pushnumber(L, 1);
	}
	else
	{
		lua_pushnumber(L, -1);
	}
	return 1;
}
//20150121 wk GMtool登陆验证结果发往客户端
int CSSanGuoFunc::L_SendLoginGMRes(lua_State *L)
{

	if (!lua_isnumber(L, 1))
		return 0;

	SALoginGMMsg Res;
	unsigned int sddr = static_cast<unsigned int>(lua_tonumber(L, 1));
	Res.m_Result = static_cast<BYTE>(lua_tonumber(L, 2));
	void* p = (void*)sddr;
	LPSOCKET_OBJ lpSocketObj = (LPSOCKET_OBJ)sddr;

	int ret = send(lpSocketObj->m_socket, (char*)&Res, sizeof(SAPlayerBaseInf), 0);
	int a = WSAGetLastError();
	return 0;
}
int CSSanGuoFunc::L_SendCheckinData(lua_State *L)	//同步签到信息到前台
{
	if (g_Script.m_pPlayer == NULL)
	{
		rfalse("L_SendCheckinData---m_pPlayer=null");
		return 0;
	}
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		return 0;
	SACHECKIN_DATASYN msg;
	msg.beginTime = static_cast<DWORD>(lua_tonumber(L, 1));
	msg.lastTime = static_cast<DWORD>(lua_tonumber(L, 2));
	msg.lastCount = static_cast<DWORD>(lua_tonumber(L, 3));
	msg.addCount = static_cast<DWORD>(lua_tonumber(L, 4));
	/*msg.turn = static_cast<DWORD>(lua_tonumber(L, 5));
	msg.days = static_cast<DWORD>(lua_tonumber(L, 6));*/
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(SACHECKIN_DATASYN));

}

int CSSanGuoFunc::L_GiveCheckinAwards(lua_State *L)	//给签到奖励
{
	if (g_Script.m_pPlayer == NULL)
	{
		rfalse("L_GiveCheckinAwards---m_pPlayer=null");
		return 0;
	}
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		return 0;
	int index = static_cast<DWORD>(lua_tonumber(L, 1));
	int itype = static_cast<DWORD>(lua_tonumber(L, 2));
	if (itype == 1)//30日签到
	{
		const CheckInConfig * _CheckInConfig = CConfigManager::getSingleton()->GetCheckInAwardsConfig(index);
		if (!_CheckInConfig)
			return 0;
		//从lua获取vip等级
		lite::Variant ret;
		LuaFunctor(g_Script, "SI_vip_getlv")[g_Script.m_pPlayer->GetSID()](&ret);
		int playerViplv = (int)ret;

		int doublenum = 1;
		int doubleneed = _CheckInConfig->doubleNeedVip;
		if (doubleneed > 0 && playerViplv >= doubleneed) //有双倍标识且够lv
		{
			doublenum = 2;
		}
		g_Script.m_pPlayer->GetSanguoBaseData()->AddGoods_SG(_CheckInConfig->Goods[0].itype, _CheckInConfig->Goods[0].id, _CheckInConfig->Goods[0].num*doublenum, GoodsWay::checkIn);
		g_Script.m_pPlayer->GetSanguoBaseData()->AddGoods_SG(_CheckInConfig->Goods[1].itype, _CheckInConfig->Goods[1].id, _CheckInConfig->Goods[1].num, GoodsWay::checkIn);

	}
	else if (itype == 2)//7日签到
	{

		int goodsindex = 0;
		if (index>7)//非首次签到
		{
			index = index - 7;
			goodsindex = 1;
		}
		const CheckInday7Config * _CheckInday7Config = CConfigManager::getSingleton()->GetCheckInday7AwardsConfig(index);
		if (!_CheckInday7Config)
			return 0;
		g_Script.m_pPlayer->GetSanguoBaseData()->AddGoods_SG(_CheckInday7Config->Goods[goodsindex].itype, _CheckInday7Config->Goods[goodsindex].id, _CheckInday7Config->Goods[goodsindex].num, GoodsWay::checkIn);
	}
	return 1;
}

int CSSanGuoFunc::L_TipMsg(lua_State *L)	//发提示信息
{
	if (g_Script.m_pPlayer == NULL)
	{
		rfalse("L_TipMsg---m_pPlayer=null");
		return 0;
	}
	LPCSTR szStr = static_cast<const char*>(lua_tostring(L, 1));
	TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, szStr);

	//int a = sizeof(*g_Script.m_pPlayer);
	//int b = sizeof(g_Script.m_pPlayer->m_FixData);
	//int c = sizeof(g_Script.m_pPlayer->m_Property);
	//for (int i = 0; i < 10000; i++)
	//{
	//	rfalse("警告等级[1]进入等待时间累计超时，可能是服务器过载，也可能是服务器内部出错");
	//}
	
	
	//LuaFunctor(g_Script, "war_fight_res")[m_pPlayerData->m_dwStaticID][warMsg->res]();
	if (g_Script.PrepareFunction("war_fight_res"))
	{
		g_Script.PushParameter(3444);
		g_Script.PushParameter(22);

		DWORD _dd[10] = { 1, 2, 3, 4, 5 };
		std::list<DWORD> aa;
		aa.push_back(1);
		aa.push_back(2);
		aa.push_back(3);
		aa.push_back(4);
		g_Script.PushDWORDArray(aa);
		g_Script.Execute();
	}

	return 1;
}

//设置三国玩家数据
int CSSanGuoFunc::L_SetPlayerDataSG(lua_State *L)
{
	DWORD dwSID = static_cast<DWORD>(lua_tonumber(L, 1));
	CPlayer* pPlayer;
	if (dwSID == 0)
	{
		pPlayer = g_Script.m_pPlayer;
	}
	else
	{
		pPlayer = (CPlayer *)GetPlayerBySID(dwSID)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer) return 0;

	int index = static_cast<int>(lua_tonumber(L, 2));
	switch (index)
	{
	case 0:	//sid
		//lua_pushnumber(L, pPlayer->GetSID());
		break;
	case 1:	//money
		//lua_pushnumber(L, pPlayer->m_FixData.m_dwMoney);
		break;
	case 2:	//diamond
		//lua_pushnumber(L, pPlayer->m_FixData.m_dwDiamond);
		break;
	case 3:	//heroskill
	{
		int heroId = static_cast<int>(lua_tonumber(L, 3));
		int skillIndex = static_cast<int>(lua_tonumber(L, 4));
		int skillLv = static_cast<int>(lua_tonumber(L, 5));
		if (skillIndex > MAX_SKILLNUM || skillIndex < 1)
		{
			TalkToDnid(pPlayer->m_ClientIndex, "技能位置错误,要崩溃的,哥!");
			return 1;
		};
		for (int i = 0; i < MAX_HERO_NUM; i++)
		{
			if (heroId == pPlayer->m_FixData.m_HeroList[i].m_dwHeroID)
			{
				pPlayer->m_FixData.m_HeroList[i].m_SkillInfoList[skillIndex - 1].m_dwSkillLevel = skillLv;

				SAUpgradeHeroSkillResult resultMsg;
				resultMsg.dwHeroID = heroId;
				resultMsg.dwSkillID[resultMsg.dwValidNum] = pPlayer->m_FixData.m_HeroList[i].m_SkillInfoList[skillIndex - 1].m_dwSkillID;
				resultMsg.dwSkillLevel[resultMsg.dwValidNum] = skillLv;
				resultMsg.dwValidNum++;
				resultMsg.bResult = true;
				g_StoreMessage(pPlayer->m_ClientIndex, &resultMsg, sizeof(SAUpgradeHeroSkillResult));
				return 1;
			};
		};
		TalkToDnid(pPlayer->m_ClientIndex, "武将id错误,没有此武将!");
		break;
	};
	case 4:	//herolv
	{
		int heroId = static_cast<int>(lua_tonumber(L, 3));
		int heroexp = static_cast<int>(lua_tonumber(L, 4));

		for (int i = 0; i < MAX_HERO_NUM; i++)
		{
			if (heroId == pPlayer->m_FixData.m_HeroList[i].m_dwHeroID)
			{
				//pPlayer->m_FixData.m_HeroList[i].m_dwLevel = herolv;
				//pPlayer->m_FixData.m_HeroList[i].m_Exp = heroexp;


				int level = 0;
				bool levelLimitted = false;
				if (g_Script.m_pPlayer->GetSanguoBaseData()->PlusHeroExp(heroId, heroexp, level, levelLimitted) == false)
				{

					return false;
				}

				TalkToDnid(pPlayer->m_ClientIndex, "操作成功,重新登陆生效!");
				return 1;
			};
		};
		TalkToDnid(pPlayer->m_ClientIndex, "武将id错误,没有此武将!");
		break;
	};

	case 5:	//m_dwStarLevel
	{
		int heroId = static_cast<int>(lua_tonumber(L, 3));
		int StarLevel = static_cast<int>(lua_tonumber(L, 4));

		for (int i = 0; i < MAX_HERO_NUM; i++)
		{
			if (heroId == pPlayer->m_FixData.m_HeroList[i].m_dwHeroID)
			{
				pPlayer->m_FixData.m_HeroList[i].m_dwStarLevel = StarLevel;
				TalkToDnid(pPlayer->m_ClientIndex, "操作成功,重新登陆生效!");
				return 1;
			};
		};
		TalkToDnid(pPlayer->m_ClientIndex, "武将id错误,没有此武将!");
		break;
	};
	case 6:	//m_dwRankLevel
	{
		int heroId = static_cast<int>(lua_tonumber(L, 3));
		int RankLevel = static_cast<int>(lua_tonumber(L, 4));

		for (int i = 0; i < MAX_HERO_NUM; i++)
		{
			if (heroId == pPlayer->m_FixData.m_HeroList[i].m_dwHeroID)
			{
				pPlayer->m_FixData.m_HeroList[i].m_dwRankLevel = RankLevel;
				TalkToDnid(pPlayer->m_ClientIndex, "操作成功,重新登陆生效!");
				return 1;
			};
		};
		TalkToDnid(pPlayer->m_ClientIndex, "武将id错误,没有此武将!");
		break;
	};
	case 7: //FactionID 
	case 10: //FactionID 
	{
		DWORD dfactionid = static_cast<DWORD>(lua_tonumber(L, 3));
		memcpy(&pPlayer->m_FixData.m_FactionName, &dfactionid, 4);  //TODO 为老数据做兼容，正式上线需要修改
		return 1;
	}
	break;
	default:
		break;
	};
	return 1;
}

int CSSanGuoFunc::L_SynPlayerDataSG(lua_State *L)	//玩家首次登陆同步脚本数据
{
	if (g_Script.m_pPlayer == NULL)
	{
		rfalse("L_SynPlayerDataSG---m_pPlayer=null");
		return 0;
	}
	if (!lua_istable(L, 1))
	{
		return 0;
	}
	SADATASYN_FIRST msg;
	size_t nlenth = luaL_getn(L, 1);

	msg.lenth = nlenth;
	for (int i = 0; i < nlenth; i++)
	{
		lua_rawgeti(L, 1, i + 1);
		msg.sdata[i] = static_cast<int>(lua_tonumber(L, -1));
		lua_pop(L, 1);
	}
	int aa = sizeof(SADATASYN_FIRST) - (MAX_SYN_SCRIPT_LENTH - nlenth)*sizeof(DWORD);
	int bb = sizeof(SADATASYN_FIRST);
	int vv = (MAX_SYN_SCRIPT_LENTH - nlenth)*sizeof(DWORD);
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(SADATASYN_FIRST) - (MAX_SYN_SCRIPT_LENTH - nlenth)*sizeof(DWORD));
	return 1;
}

int CSSanGuoFunc::L_SynNotificationDataSG(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
	{
		rfalse("L_SynNotificationDataSG---m_pPlayer=null");
		return 0;
	}
	if (!lua_istable(L, 1))
		return 0;

	size_t nlenth = luaL_getn(L, 1);

	if (nlenth == 0)
		return 0;

	SDATASYN_NOTIFICATION msg;
	msg.length = nlenth;
	for (int i = 0; i < nlenth; ++i)
	{
		///开始获取第i个推送消息
		lua_rawgeti(L, 1, i + 1);
		if (lua_istable(L, -1))
		{
			int subTableLength = lua_objlen(L, -1);
			///严格限制表的长度为4
			if (subTableLength != 4)
				continue;

			///读取推送的ID
			lua_rawgeti(L, -1, 1);
			msg.datas[i].notificationID = static_cast<int>(lua_tonumber(L, -1));

			///读取推送的时间
			lua_rawgeti(L, -2, 2);
			msg.datas[i].alarmTime = static_cast<int>(lua_tonumber(L, -1));///存好触发的时间的秒数

			///读取推送的标题
			lua_rawgeti(L, -3, 3);
			const char* title = lua_tostring(L, -1);
			char* Utf8Str = g_AnsiToUtf8(title);
			strcpy_s(msg.datas[i].notificationTitle, 64, Utf8Str);///暂时写死推送的标题不超过64个字符

			///读取推送的内容
			lua_rawgeti(L, -4, 4);
			const char* content = lua_tostring(L, -1);
			Utf8Str = g_AnsiToUtf8(content);
			strcpy_s(msg.datas[i].notificationContents, 128, Utf8Str);///暂时写死推送的内容不超过128个字符
		}
		lua_pop(L, 1);
	}

	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(SDATASYN_NOTIFICATION) - (MAX_SYN_NOTIFICATION_LENGTH - msg.length)*sizeof(SNotificationData));
	return 1;
}

int CSSanGuoFunc::L_SynMissionDataSG(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
	{
		rfalse("L_SynMissionDataSG---m_pPlayer=null");
		return 0;
	}
	if (!lua_istable(L, 1))
		return 0;

	SDATASYN_MISSION msg;

	///开始将table的值填入数组中
	int stackSize = lua_gettop(L);
	lua_pushnil(L);
	int value = 0;
	while (lua_next(L, stackSize))
	{
		if (msg.length >= MISSION_NUM)
			break;

		value = lua_tointeger(L, -1);
		if (value == 0)
		{
			lua_pop(L, 1);
			continue;
		}

		msg.datas[msg.length] = lua_tointeger(L, -1);
		++msg.length;
		lua_pop(L, 1);
	}

	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(SDATASYN_MISSION) - (MISSION_NUM - msg.length) * sizeof(int));

	return 1;
}

int CSSanGuoFunc::L_SynHeroExtendsData(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
	{
		rfalse("L_SynHeroExtendsData---m_pPlayer=null");
		return 0;
	}
	if (!lua_istable(L, 1))
		return 0;

	SDATASYN_HEROEXTENDSGAMEPLAY msg;
	///开始将table的值填入数组中
	int stackSize = lua_gettop(L);
	int subTableSize = 0;
	lua_pushnil(L);
	int timeDatas[MAX_HERO_NUM];
	while (0 != lua_next(L, stackSize))
	{
		if (msg.length >= MAX_HERO_NUM)
			break;

		if (!lua_istable(L, -1))
		{
			lua_pop(L, stackSize);
			continue;
		}

		subTableSize = luaL_getn(L, -1);
		if (subTableSize == 0)
		{
			lua_pop(L, stackSize);
			continue;
		}
		for (int i = 1; i <= subTableSize; ++i)
		{
			lua_rawgeti(L, -1, i);
			if (i == 1)
				msg.datas[msg.length] = lua_tointeger(L, -1);
			else
				timeDatas[msg.length] = lua_tointeger(L, -1);
			lua_remove(L, -1);
		}

		lua_pop(L, stackSize);
		++msg.length;
	}

	if (msg.length > 0)
	{
		memcpy_s(msg.datas + msg.length, msg.length * sizeof(int), timeDatas, msg.length * sizeof(int));
		g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(SDATASYN_HEROEXTENDSGAMEPLAY) - sizeof(int) * (MAX_HERO_NUM - msg.length) * 2);
	}
	lua_pop(L, 1);

	return 0;
}

int CSSanGuoFunc::L_SendMsg_SG(lua_State *L)	//脚本发送消息给客户端
{

	DWORD dwSID = static_cast<DWORD>(lua_tonumber(L, 1));
	CPlayer* pPlayer;
	if (dwSID == 0)
	{
		pPlayer = g_Script.m_pPlayer;
	}
	else
	{
		pPlayer = (CPlayer *)GetPlayerBySID(dwSID)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer) return 0;

	SQALuaCustomMsg msg;
	ZeroMemory(msg.streamData, sizeof(msg.streamData));
	size_t size = sizeof(msg);
	LPCSTR szStr = static_cast<const char*>(lua_tostring(L, 2));
	//int a = strlen(szStr);
	//int b = sizeof(msg.streamData);
	//int c = sizeof(msg);
	size -= sizeof(msg.streamData) - (strlen(szStr) + 2);
	strcpy_s(msg.streamData, szStr);
	pPlayer->SendMsg(&msg, size);
	lua_pushnumber(L, size);

	return 1;
}
int CSSanGuoFunc::L_gm_registerNum(lua_State *L)	//gm取注册人数
{
	int arg1 = static_cast<int>(lua_tonumber(L, 1));
	SAgm_registerNum rMsg;
	rMsg.registerNum = arg1;
	g_StoreMessage(gm_dnidClient, &rMsg, sizeof(SAgm_registerNum));
	return 1;
}
int CSSanGuoFunc::L_gm_stayAll(lua_State *L)	//gm取30日留存
{

	SAgm_stayAll rMsg;
	int i;
	for (i = 0; i < 11; i++)
	{
		rMsg.result[i] = static_cast<int>(lua_tonumber(L, i + 1));
	}
	g_StoreMessage(gm_dnidClient, &rMsg, sizeof(SAgm_stayAll));
	return 1;
}
int CSSanGuoFunc::L_gm_stayOne(lua_State *L)	//gm取单日留存
{
	int allreg = static_cast<int>(lua_tonumber(L, 1));
	int num = static_cast<int>(lua_tonumber(L, 2));

	SAgm_stayOne rMsg;
	rMsg.result[0] = allreg;
	rMsg.result[1] = num;
	g_StoreMessage(gm_dnidClient, &rMsg, sizeof(SAgm_stayOne));
	return 1;
}
int CSSanGuoFunc::L_gm_online(lua_State *L)	//gm取在线人数
{
	SAgm_online rMsg;
	int ii = 0;
	if (lua_istable(L, 1))
	{
		int t_idx = 0;
		int it_idx = 0;
		int value, key;

		t_idx = lua_gettop(L);
		lua_pushnil(L);// nil 入栈作为初始 key 

		while (lua_next(L, t_idx))//一层
		{
			//c = lua_tonumber(L, -2);//key
			it_idx = lua_gettop(L);
			lua_pushnil(L);
			while (lua_next(L, it_idx))//2层
			{
				value = lua_tonumber(L, -1);//value
				key = lua_tonumber(L, -2);//key
				if (key == 1)//时间
				{
					rMsg.m_time[ii] = value;
				}
				else//次数
				{
					rMsg.m_num[ii] = value;
					ii++;
				}
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
			if (ii > MAX_GM_online - 1)
				break;
		}
	}
	g_StoreMessage(gm_dnidClient, &rMsg, sizeof(SAgm_online));
	return 1;
}
int CSSanGuoFunc::L_gm_recharge(lua_State *L)	//gm取充值信息
{
	return 1;
}

int CSSanGuoFunc::L_gm_leave(lua_State *L)	//gm取流失数据
{
	int itype = static_cast<int>(lua_tonumber(L, 1));
	int num = static_cast<int>(lua_tonumber(L, 2));
	int res = static_cast<int>(lua_tonumber(L, 3));
	SAgm_leave rMsg;
	rMsg.itype = itype;
	rMsg.num = num;
	rMsg.res = res;
	g_StoreMessage(gm_dnidClient, &rMsg, sizeof(SAgm_leave));
	return 1;
}
int CSSanGuoFunc::L_gm_pointuse(lua_State *L)	//gm取消费信息
{
	int allreg = static_cast<int>(lua_tonumber(L, 1));
	SAgm_pointuse rMsg;
	rMsg.res = allreg;
	g_StoreMessage(gm_dnidClient, &rMsg, sizeof(SAgm_pointuse));
	return 1;
}

int CSSanGuoFunc::L_gm_operation(lua_State *L)	//gm取后台功能信息
{
	int itype = static_cast<int>(lua_tonumber(L, 1));
	SAgm_operation rMsg;
	rMsg.itype = itype;

	rMsg.res1 = static_cast<int>(lua_tonumber(L, 2));
	rMsg.res2 = static_cast<int>(lua_tonumber(L, 3));
	rMsg.res3 = static_cast<int>(lua_tonumber(L, 4));
	rMsg.res4 = static_cast<int>(lua_tonumber(L, 5));
	rMsg.res5 = static_cast<int>(lua_tonumber(L, 6));
	rMsg.res6 = static_cast<int>(lua_tonumber(L, 7));

	g_StoreMessage(gm_dnidClient, &rMsg, sizeof(SAgm_operation));
	return 1;
}

int CSSanGuoFunc::L_UpdateVIPLevel(lua_State *L)	//设置玩家VIP等级
{
	DWORD dwSid = static_cast<DWORD>(lua_tointeger(L, 1));
	CPlayer* pPlayer = nullptr;
	if (0 == dwSid)
	{
		pPlayer = g_Script.m_pPlayer;
	}
	else
	{
		pPlayer = (CPlayer *)GetPlayerBySID(dwSid)->DynamicCast(IID_PLAYER);
	}

	if (nullptr == pPlayer)
	{
		rfalse("L_UpdateVIPLevel---pPlayer=null");
		return 0;
	}

	int vipLevel = lua_tointeger(L, 2);
	lua_settop(L, 0);
	pPlayer->UpdateDataForVIP(vipLevel);
	return 0;
}

int CSSanGuoFunc::L_ActiveLeagueOfLegend(lua_State *L)
{
	if (g_Script.m_pPlayer == nullptr)
		return 0;

	const CLeagueOfLegendDuplicateMananger* leagueOfLegendMgr = static_cast<const CLeagueOfLegendDuplicateMananger*>(g_Script.m_pPlayer->GetDuplicatemanager(InstanceType::LeagueOfLegends));
	if (leagueOfLegendMgr == nullptr)
		return 0;

	///先判断第一个参数是否是table
	if (!lua_istable(L, 1))
		return 0;

	///获取Table的长度个数
	int n = lua_objlen(L, 1);
	n = min(n, MAX_LEAGUEOFLEGEND_DUPILICATE_NUM);
	int duplicateIDs[MAX_LEAGUEOFLEGEND_DUPILICATE_NUM];
	///开始将table的值填入数组中
	for (int i = 1; i <= n; ++i)
	{
		lua_rawgeti(L, 1, i);
		duplicateIDs[i - 1] = static_cast<int>(lua_tonumber(L, -1));
	}
	lua_pop(L, 1);

	///激活将神传说副本,并往客户端同步消息
	leagueOfLegendMgr->ActiveSpecifyDuplicate(duplicateIDs, n, true);
	return 0;
}


int CSSanGuoFunc::L_ComsumableUsedInfor(lua_State *L)
{
	if (g_Script.m_pPlayer == nullptr)
		return 0;

	SUseConsumableAnswer answerMsg;

	///先判断所有的3个参数是否是table
	if (lua_istable(L, 1) && lua_istable(L, 2) && lua_istable(L, 3))
	{
		///获取物品类型数组的长度个数
		int n = lua_objlen(L, 1);
		n = min(n, MAX_ITEM_NUM_GAINED_FROM_CHEST / 3);
		answerMsg.itemNum = n;
		///开始将table的值填入数组中
		for (int i = 1; i <= n; ++i)
		{
			lua_rawgeti(L, 1, i);
			answerMsg.itemList[i - 1] = static_cast<int>(lua_tonumber(L, -1));
		}

		///获取物品ID数组的长度个数
		n = lua_objlen(L, 2);
		if (n != answerMsg.itemNum)
		{
			rfalse("开宝箱的时候lua传过来的物品ID数组大小有误");
			return 0;
		}
		///开始将table的值填入数组中
		for (int i = 1; i <= n; ++i)
		{
			lua_rawgeti(L, 2, i);
			answerMsg.itemList[i + n - 1] = static_cast<int>(lua_tonumber(L, -1));
		}

		///获取物品个数数组的长度个数
		n = lua_objlen(L, 3);
		if (n != answerMsg.itemNum)
		{
			rfalse("开宝箱的时候lua传过来的物品的个数数组大小有误");
			return 0;
		}
		///开始将table的值填入数组中
		for (int i = 1; i <= n; ++i)
		{
			lua_rawgeti(L, 3, i);
			answerMsg.itemList[i + 2 * n - 1] = static_cast<int>(lua_tonumber(L, -1));
		}

		lua_pop(L, 1);

		///告诉客户端开宝箱或者使用某些消耗品后得到的可放入仓库的道具
		g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &answerMsg, sizeof(SUseConsumableAnswer) - sizeof(DWORD) * (MAX_ITEM_NUM_GAINED_FROM_CHEST - 3 * n));
	}
	else ///如果不是table，则代表只有一个物品类型
	{
		answerMsg.itemNum = 1;
		answerMsg.itemList[0] = static_cast<int>(lua_tonumber(L, 1));
		///填充技能点药剂的ID
		answerMsg.itemList[1] = static_cast<int>(lua_tonumber(L, 2));
		///填充技能点药剂的使用个数
		answerMsg.itemList[2] = static_cast<int>(lua_tonumber(L, 3));
		///告诉客户端开宝箱或者使用某些消耗品后得到的可放入仓库的道具
		g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &answerMsg, sizeof(SUseConsumableAnswer) - sizeof(DWORD) * (MAX_ITEM_NUM_GAINED_FROM_CHEST - 3));
	}

	return 0;
}

int CSSanGuoFunc::L_SetDessertDispachInfor(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
	{
		rfalse("L_SetDessertDispachInfor---m_pPlayer=null");
		return 0;
	}
	bool bAvaliable = static_cast<bool>(lua_toboolean(L, 1));
	int iType = static_cast<int>(lua_tointeger(L, 2));
	int iNum = static_cast<int>(lua_tointeger(L, 3));
	CDessertSystem* dessertSystem = g_Script.m_pPlayer->GetDessertSystem();
	if (dessertSystem != nullptr)
		dessertSystem->DispachDessert(bAvaliable, (DessertType)iType, iNum);

	return 0;
}


int CSSanGuoFunc::L_ChangeName(lua_State *L)
{

	int iType = static_cast<int>(lua_tointeger(L, 1));

	DWORD sid = static_cast<DWORD>(lua_tointeger(L, 2));

	char name[32];

	LPCSTR szStr = static_cast<const char*>(lua_tostring(L, 3));

	SAChangeName msg;

	CPlayer* pPlayer;

	CBaseDataManager* basedata;

	pPlayer = (CPlayer *)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);


	if (pPlayer != NULL)
	{
		basedata = pPlayer->GetSanguoBaseData();
	}
	else
	{
		return 0;
	}
	if (iType == 0)
	{
		pPlayer->Db_Back_ChangeGameName((char*)szStr, sid, basedata);
	}
	else
	{
		//该名称已存在
		lite::Variant ret;//从lua获取更名次数
		LuaFunctor(g_Script, "GetChangeNameCount")[sid](&ret);
		int count = (int)ret;

		msg.ChangeNameCount = count;
		msg.falg = false;
		g_StoreMessage(basedata->GetDNID(), &msg, sizeof(msg));
	}
	return 0;
}

///反馈客户端获取活动奖励结果
int CSSanGuoFunc::L_SendActResult(lua_State *L)
{
	SAGetActReward msg;
	msg.actType = static_cast<int32_t>(lua_tointeger(L, 1)); //活动类型
	msg.reawrdIndex = static_cast<int32_t>(lua_tointeger(L, 2)); //奖励索引
	msg.result = static_cast<byte>(lua_tointeger(L, 3)); //获取奖励结果。 1为成功，0为失败
	if (g_Script.m_pPlayer == nullptr)
		return 0;
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(msg));
	return 1;
}
///发送累计充值奖励数据
int CSSanGuoFunc::L_SendChargeSumResult(lua_State * L)
{
	SARewardState msg;
	DWORD sid = static_cast<int32_t>(lua_tointeger(L, 1));
	CPlayer * pPlayer = (CPlayer *)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	if (pPlayer == nullptr)
		return 0;
	DWORD chargeNum = static_cast<DWORD>(lua_tointeger(L, 2));
	LPCSTR Txt1 = static_cast<const char*>(lua_tostring(L, 3));
	if (Txt1 == nullptr)
		return 0;
	strcpy_s(msg.rewardState, Txt1);
	msg.chargeSum = chargeNum;
	g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
}
int CSSanGuoFunc::L_UpdateMissionStateSG(lua_State* L)
{
	DWORD dwSID = static_cast<DWORD>(lua_tonumber(L, 1));
	CPlayer* pPlayer;
	if (dwSID == 0)
	{
		pPlayer = g_Script.m_pPlayer;
	}
	else
	{
		pPlayer = (CPlayer *)GetPlayerBySID(dwSID)->DynamicCast(IID_PLAYER);
	}
	if (!pPlayer) return 0;

	SAUpdateMissionState msg;
	msg.missionID = static_cast<int32_t>(lua_tointeger(L, 2));
	msg.visible = static_cast<bool>(lua_toboolean(L, 3));
	msg.completeTimes = static_cast<int32_t>(lua_tointeger(L, 4));
	pPlayer->SendMsg(&msg, sizeof(msg));
}
int CSSanGuoFunc::L_AddSkillPoint(lua_State *L)
{
	if (g_Script.m_pPlayer == nullptr)
		return 0;

	///获取增加的技能点数
	int skillPointNum = static_cast<int>(lua_tonumber(L, 1));
	g_Script.m_pPlayer->AddSkillPoint(skillPointNum);

	return 0;
}
int CSSanGuoFunc::L_SendFirstRechargeRes(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
	{
		rfalse("L_SendFirstRechargeRes---m_pPlayer=null");
		return 0;
	}
	SAFirstRecharge msg;
	msg.res = static_cast<int32_t>(lua_tonumber(L, 1));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(msg));
	return 1;
}

int CSSanGuoFunc::L_WeekReMoney(lua_State *L)
{
	if (g_Script.m_pPlayer == NULL)
	{
		rfalse("L_WeekReMoney---m_pPlayer=null");
		return 0;
	}
	int state = static_cast<int>(lua_tointeger(L, 1));
	int one = static_cast<int>(lua_tointeger(L, 2));
	int two = static_cast<int>(lua_tointeger(L, 3));
	int three = static_cast<int>(lua_tointeger(L, 4));
	int four = static_cast<int>(lua_tointeger(L, 5));
	int five = static_cast<int>(lua_tointeger(L, 6));
	int six = static_cast<int>(lua_tointeger(L, 7));
	int seven = static_cast<int>(lua_tointeger(L, 8));
	int snum = static_cast<int>(lua_tointeger(L, 9));
	int res[7];
	res[0] = one;
	res[1] = two;
	res[2] = three;
	res[3] = four;
	res[4] = five;
	res[5] = six;
	res[6] = seven;

	SAWeekRemoney msg;
	msg.state = state;
	msg.day = snum;
	memcpy(msg.res, res, sizeof(res));
	if (nullptr != g_Script.m_pPlayer)
		g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(msg));
	return 0;
}

int CSSanGuoFunc::L_SendMail(lua_State *L)
{
	MailInfo _MailInfo;
	memset(&_MailInfo, 0, sizeof(_MailInfo));
	strcpy_s(_MailInfo.senderName, "mingzi");
	strcpy_s(_MailInfo.title, "biaoti");
	strcpy_s(_MailInfo.content, "content");

	_MailInfo.szItemInfo[0].itype = 1;
	_MailInfo.szItemInfo[0].id = 1;
	_MailInfo.szItemInfo[0].num = 1;
	MailMoudle::getSingleton()->SendMail_SanGuo(11, 11, &_MailInfo);
	return 1;
}

//取玩家武将信息 //16个属性+8个技能(id,lv)+1个战斗力
int CSSanGuoFunc::L_GetHeroInfo(lua_State *L) 
{
	

	int32_t dwSID = static_cast<int32_t>(lua_tointeger(L, 1));
	CPlayer* pPlayer;
	if (dwSID == 0)//当前对象
	{
		pPlayer = g_Script.m_pPlayer;
	}
	else
	{
		pPlayer = (CPlayer *)GetPlayerBySID(dwSID)->DynamicCast(IID_PLAYER); //取制定玩家
	}
	if (!pPlayer) return 0;

	int32_t heroId = static_cast<int32_t>(lua_tointeger(L, 2));
	const SHeroAttr* pHeroAttr = pPlayer->GetSanguoBaseData()->GetHeroAttr(heroId);
	const SHeroData * pHeroData = pPlayer->GetSanguoBaseData()->GetHero(heroId);
	if (pHeroAttr == nullptr || pHeroData==nullptr)
	{ 
		rfalse("L_GetHeroInfo error ,sid= %d", dwSID);
		return 0;
	}

	lua_createtable(L, 0, 0);
	float array[25] ;//16个属性+8个技能(id,lv)+1个战斗力
	memcpy(array, &pHeroAttr->curAttr.Power, sizeof(float) * 16);//属性16个

	int site = 16;  //技能开始位置
	for (int i = 0; i < 4;i++)
	{
		array[site] = pHeroData->m_SkillInfoList[i].m_dwSkillID;
		array[site+1] = pHeroData->m_SkillInfoList[i].m_dwSkillLevel;
		site += 2;
	}

	array[24] = pHeroAttr->CombatPower;//战力

	for (int n = 0; n < 25; n++)
	{
		lua_pushinteger(L, array[n]); //将数组的数据入栈
		lua_rawseti(L, -2, n + 1); //将刚刚入栈的数据设置为数组的第n+1个数据，同时这个数据会自动从栈里pop
	}

	return 1;
}
//国战和前台消息交互相关
int CSSanGuoFunc::L_WarMsg(lua_State *L)
{
	int32_t dwSID = static_cast<int32_t>(lua_tointeger(L, 1));
	CPlayer* pPlayer;
	if (dwSID == 0)//当前对象
	{
		pPlayer = g_Script.m_pPlayer;
	}
	else
	{
		pPlayer = (CPlayer *)GetPlayerBySID(dwSID)->DynamicCast(IID_PLAYER); //取制定玩家
	}
	if (!pPlayer) return 0;


	int32_t warMsgType = static_cast<int32_t>(lua_tointeger(L, 2));
	switch (warMsgType)
	{
	case SWarMsg::war_join:
	{
		SAWarJoin msg;
		msg.res = static_cast<int32_t>(lua_tointeger(L, 3));
		msg.teamNum = static_cast<int32_t>(lua_tointeger(L, 4));
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}
	break;
	case SWarMsg::war_teamList:
	{
		
		SAwar_teamList msg;
		msg.teamNum = static_cast<int32_t>(lua_tointeger(L, 3));
		msg.beginNum = static_cast<int32_t>(lua_tointeger(L, 4));

		///先判断第一个参数是否是table
		if (!lua_istable(L, 5))
			return 0;

		///获取Table的长度个数
		int n = lua_objlen(L, 5);
		n = min(n, MAX_WAR_NAME_LIST);
		char* namePtr = msg.name_all;
		for (int i = 1; i <= n; ++i)
		{
			lua_rawgeti(L, 5, i);
			memcpy(namePtr, lua_tostring(L, -1), CONST_USERNAME);
			namePtr += CONST_USERNAME;
		}
		lua_pop(L, 1);

		/*char* tt = "dfafafa";

		for (int i = 0; i < MAX_WAR_NAME_LIST;i++)
		{
		memcpy(test, tt, CONST_USERNAME);
		test += CONST_USERNAME;
		}*/
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}
	break;
	case SWarMsg::war_fight_start:
	{
		SAwar_fight_start msg; 
		//char* tt = "xxxx";
		//memcpy(msg.name_other, tt, CONST_USERNAME);

		//自己的武将,血量,怒气,id
		int n = lua_objlen(L, 3);
		n = min(n, MAX_War_HERONUM * 3);
		for (int i = 1; i <= n; ++i)
		{
			lua_rawgeti(L, 3, i);
			msg.heroInfo[i - 1] = static_cast<DWORD>(lua_tonumber(L, -1));
			lua_pop(L, 1);
		}
		

		////16个基础属性数据(1-16)+8个技能数据(17-24)+2个血量怒气(25-26)+1武将id(27)
		n = lua_objlen(L, 4);
		n = min(n, MAX_WAR_HERO_INFO*MAX_War_HERONUM);
		for (int i = 1; i <= n; ++i)
		{
			lua_rawgeti(L, 4, i);
			msg.heroInfo_other[i - 1] = static_cast<DWORD>(lua_tonumber(L, -1));
			lua_pop(L, 1);
		}
		
		msg.score1 = static_cast<int32_t>(lua_tointeger(L, 5));
		msg.score2 = static_cast<int32_t>(lua_tointeger(L, 6));
		memcpy(msg.name_other, lua_tostring(L,7), CONST_USERNAME);
		msg.winAll = static_cast<int32_t>(lua_tointeger(L, 8));
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}
	break;
	case SWarMsg::war_fight_res:
	{
		SAwar_fight_res msg;
		msg.res = static_cast<int32_t>(lua_tointeger(L, 3));
		for (int i = 0; i < 6; i++)
		{
			msg.winInfo[i] = static_cast<int32_t>(lua_tointeger(L, 4+i));
		}
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}
	break; 
	case SWarMsg::war_fight_res_broadcast:
	{
		SAwar_fight_res_broadcast msg;
		memcpy(msg.name_our, lua_tostring(L, 3), CONST_USERNAME);
		memcpy(msg.name_other, lua_tostring(L, 4), CONST_USERNAME);

		msg.res = static_cast<int32_t>(lua_tointeger(L, 5));
		msg.winNum = static_cast<int32_t>(lua_tointeger(L, 6));
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}
	break;
	case SWarMsg::war_exit:
	{
		SAwar_exit msg;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}
	break;
	case SWarMsg::war_isjoin:
	{
		SAwar_isjoin msg;
		msg.isjoin = static_cast<int32_t>(lua_tointeger(L, 3));
		msg.team = static_cast<int32_t>(lua_tointeger(L, 4));
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}
	break;
	case SWarMsg::war_rank:
	{
		SAwar_rank msg;
		memcpy(msg.name, lua_tostring(L, 3), CONST_USERNAME);
		msg.win = static_cast<int32_t>(lua_tointeger(L, 4));
		msg.lv = static_cast<int32_t>(lua_tointeger(L, 5));
		msg.icon = static_cast<int32_t>(lua_tointeger(L, 6));
		msg.team= static_cast<int32_t>(lua_tointeger(L, 7));

		memcpy(msg.name2, lua_tostring(L, 8), CONST_USERNAME);
		msg.win2 = static_cast<int32_t>(lua_tointeger(L, 9));
		msg.lv2 = static_cast<int32_t>(lua_tointeger(L, 10));
		msg.icon2 = static_cast<int32_t>(lua_tointeger(L, 11));
		msg.team2 = static_cast<int32_t>(lua_tointeger(L, 12));

		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}
	break;
	case SWarMsg::war_end:
	{
		SAWarEnd msg;
		msg.teamNum= static_cast<int32_t>(lua_tointeger(L, 3));
		msg.score1 = static_cast<int32_t>(lua_tointeger(L,4));
		msg.score2 = static_cast<int32_t>(lua_tointeger(L, 5));
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}
	break;
	default:
		break;
	}

	return 1;
}

//国战邮件奖励
int CSSanGuoFunc::L_WarMail(lua_State *L)
{
	int sid = static_cast<int32_t>(lua_tointeger(L, 1));
	MailInfo _MailInfo;
	memset(&_MailInfo, 0, sizeof(_MailInfo));

	lite::Variant ret1;//
	LuaFunctor(g_Script, "SI_GetStr")[13](&ret1);
	LPCSTR Txt1 = (LPCSTR)ret1;
	if (Txt1 == nullptr)
		rfalse("L_WarMail error 1");
	strcpy_s(_MailInfo.senderName, Txt1);

	lite::Variant ret2;//
	LuaFunctor(g_Script, "SI_GetStr")[11](&ret2);
	LPCSTR Txt2 = (LPCSTR)ret2;
	if (Txt2 == nullptr)
		rfalse("L_WarMail error 2");
	strcpy_s(_MailInfo.title, Txt2);

	lite::Variant ret3;//
	LuaFunctor(g_Script, "SI_GetStr")[12](&ret3);
	LPCSTR Txt3 = (LPCSTR)ret3;
	if (Txt3 == nullptr)
		rfalse("L_WarMail error 3");
	strcpy_s(_MailInfo.content, Txt3);

	_MailInfo.szItemInfo[0].itype = static_cast<int32_t>(lua_tointeger(L, 2));
	_MailInfo.szItemInfo[0].id = static_cast<int32_t>(lua_tointeger(L,3));
	_MailInfo.szItemInfo[0].num = static_cast<int32_t>(lua_tointeger(L, 4));
	MailMoudle::getSingleton()->SendMail_SanGuo(sid, 60 * 24 * 7, &_MailInfo);
	return 1;
}


//排行榜消息
int CSSanGuoFunc::L_RankMsg(lua_State *L)
{
	CPlayer* pPlayer = g_Script.m_pPlayer;
	if (!pPlayer) return 0;


	int32_t RankMsgType = static_cast<int32_t>(lua_tointeger(L, 1));
	if (RankMsgType==1)
	{
		SARank_hero msg;
		if (lua_istable(L, 2))
		{
			int t_idx = 0;
			int it_idx = 0;
			int key1, key2;
			t_idx = lua_gettop(L);
			lua_pushnil(L);// nil 入栈作为初始 key 

			while (lua_next(L, t_idx))//一层
			{
				key1 = lua_tonumber(L, -2);//key1
				if (key1 >= MAX_Rank_List)
					return 0;
				if (lua_istable(L, -1))
				{
					it_idx = lua_gettop(L);
					lua_pushnil(L);
					while (lua_next(L, it_idx))//2层
					{
						key2 = lua_tonumber(L, -2);//key2
						switch (key2)
						{
						case 1:
							msg.list[key1 - 1].Num = lua_tonumber(L, -1);//value
							break;
						case 2:
							msg.list[key1 - 1].sid = lua_tonumber(L, -1);//value
							break;
						case 3:
							msg.list[key1 - 1].lv = lua_tonumber(L, -1);//value
							break;
						case 4:
							msg.list[key1 - 1].icon = lua_tonumber(L, -1);//value
							break;
						case 5:
							memcpy(msg.list[key1 - 1].name, lua_tostring(L, -1), CONST_USERNAME);
							break;
						case 6:
							memcpy(msg.list[key1 - 1].fname, lua_tostring(L, -1), CONST_USERNAME);
							break;
						case 7:
							msg.list[key1 - 1].heroID = lua_tonumber(L, -1);//value
							break;
						case 8:
							msg.list[key1 - 1].heroStar = lua_tonumber(L, -1);//value
							break;
						case 9:
							msg.list[key1 - 1].heroStep = lua_tonumber(L, -1);//value
							break;
						case 10:
							msg.list[key1 - 1].heroLv = lua_tonumber(L, -1);//value
							break;
						default:
							break;
						}
						lua_pop(L, 1);
					}
				}


				lua_pop(L, 1);
			}
		}
		msg.itype = RankMsgType;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}
	else
	{
		SARank_common msg;
		if (lua_istable(L, 2))
		{
			int t_idx = 0;
			int it_idx = 0;
			int key1, key2;
			t_idx = lua_gettop(L);
			lua_pushnil(L);// nil 入栈作为初始 key 

			while (lua_next(L, t_idx))//一层
			{
				key1 = lua_tonumber(L, -2);//key1
				if (key1 >= MAX_Rank_List)
					return 0;

				it_idx = lua_gettop(L);
				lua_pushnil(L);
				while (lua_next(L, it_idx))//2层
				{
					key2 = lua_tonumber(L, -2);//key2
					switch (key2)
					{
					case 1:
						msg.list[key1 - 1].Num = lua_tonumber(L, -1);//value
						break;
					case 2:
						msg.list[key1 - 1].sid = lua_tonumber(L, -1);//value
						break;
					case 3:
						msg.list[key1 - 1].lv = lua_tonumber(L, -1);//value
						break;
					case 4:
						msg.list[key1 - 1].icon = lua_tonumber(L, -1);//value
						break;
					case 5:
						memcpy(msg.list[key1 - 1].name, lua_tostring(L, -1), CONST_USERNAME);
						break;
					case 6:
						memcpy(msg.list[key1 - 1].fname, lua_tostring(L, -1), CONST_USERNAME);
						break;
					default:
						break;
					}
					lua_pop(L, 1);
				}
				lua_pop(L, 1);
			}
		}
		msg.itype = RankMsgType;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}
	
	return 1;
}

int CSSanGuoFunc::L_SynMembershipToClient(lua_State *L)
{
	//TODO  同步月卡到期时间给客户端
	DWORD sid = static_cast<DWORD>(lua_tointeger(L, 1));
	CPlayer * pPlayer = (CPlayer *)GetPlayerBySID(sid)->DynamicCast(IID_PLAYER);
	if (pPlayer == nullptr)
		return 1;

	DWORD membershipDate = static_cast<DWORD>(lua_tointeger(L, 2));
	SABuyMembership synMsg;
	synMsg.byType = 1;
	synMsg.memberShipData = membershipDate;
	g_StoreMessage(pPlayer->m_ClientIndex, &synMsg, sizeof(synMsg));
	return 1;
}

int CSSanGuoFunc::L_exchangeActRes(lua_State *L)//兑换活动结果
{
	if (g_Script.m_pPlayer == NULL)
	{
		rfalse("L_exchangeActRes---m_pPlayer=null");
		return 0;
	}

	SAexchangeBack  msg;
	msg.res = static_cast<int32_t>(lua_tointeger(L, 1));
	msg.index = static_cast<int32_t>(lua_tointeger(L, 2));
	msg.value = static_cast<int32_t>(lua_tointeger(L, 3));
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(msg));
}
int CSSanGuoFunc::L_exchangeActSendData(lua_State *L)//兑换活动玩家数据同步
{
	if (g_Script.m_pPlayer == NULL)
	{
		rfalse("L_exchangeActSendData---m_pPlayer=null");
		return 0;
	}

	SAexchangeData  msg;
	int n = lua_objlen(L, 1);
	n = min(n, MaxExchangeData);
	for (int i = 1; i <= n; ++i)
	{
		lua_rawgeti(L, 1, i);
		msg.exData[i - 1] = static_cast<DWORD>(lua_tonumber(L, -1));
		lua_pop(L, 1);
	}
	g_StoreMessage(g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof(msg));
}