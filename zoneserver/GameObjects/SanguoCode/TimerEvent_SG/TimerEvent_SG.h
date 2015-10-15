// ----- TimerEvent_SG.h -----
//
//   --  Author: wk
//   --  Date:   15/01/23
//   --  Desc:   萌将三国 定时事件类头文件
// --------------------------------------------------------------------
// 主要功能为注册定时事件,如每天24点刷新数据(竞技场发奖,在线玩家数据重置(c++数据以及lua数据))
//---------------------------------------------------------------------  
#pragma once
#include "..\..\EventBase.h"
#include <functional>
#include <map>
typedef std::function<void(int)> FunctionPtr;





enum EventTypeSG
{
	none,
	EveryDay5 = 1,	///每天5点事件
	EveryDay12,		///每天12点事件
	EveryDay14,		///每天14点事件
	EveryDay18,		///每天18点事件
	EveryDay20,		///每天20点事件
	EveryDay21,		///每天21点事件
	EveryDay24,		///每天24点事件
	pRefresh,//在线玩家刷新数据,11:59:50
	Function, // 函数绑定
	hourly, // 每小时
	EachTenMinutes, //每10分钟
	EachOneMinute, //每1分钟
	VarietyShopEvent,		///杂货铺事件
	ArenaShopEvent,			///竞技场商店事件
	ExpeditionShopEvent,		///远征商店事件
	LegionShopEvent,				///军团商店事件
	MiracleMerchantEvent,	///奇缘商人事件
	GemMerchantEvent,		///珍宝商人事件
	SoulExchangeShopEvent,	///将魂兑换商店事件
	WarOfLeagueShopEvent,	///国战商店事件
};

class TimerEvent_SG : public EventBase
{
public:
	static int FristReg();
	static TimerEvent_SG* SetCallback(FunctionPtr &func, int timePar);

private:								
	virtual void SelfDestory();		// 自销毁方法
	virtual void OnCancel();	// 事件被中断
	virtual void OnActive();	// 事件被激活

	int EveryDay_5();
	int EveryDay_12();
	int EveryDay_14();
	int EveryDay_18();
	int EveryDay_20();
	int EveryDay_21();
	int EveryDay_24();
	int EveryHour();
	int ActiveTenMinEvent();//激活十分钟时间
	int ActiveOneMinEvent();//每分钟激活一次
	///@brief 杂货铺
	int VarietyShop();
	///@brief 竞技场商店
	int ArenaShop();
	///@brief 远征商店
	int ExpeditionShop();
	///@brief 军团商店
	int LegionShop();
	///@brief 奇缘商人
	int MiracleMerchant();
	///@brief 珍宝商人
	int GemMerchant();
	///@brief 将魂商店
	int SoulExchangeShop();
	///@brief 国战商店
	int WarOfLeagueShop();
private:
	int eType;
	int playerSid; //玩家的静态ID，该ID针对所有服务器群唯一
	FunctionPtr m_Function;
};

