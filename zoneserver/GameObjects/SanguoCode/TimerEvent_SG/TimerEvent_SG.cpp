#include "Stdafx.h"
#include "TimerEvent_SG.h"
#include "EventMgr.h"
#include <time.h>
#include "player.h"
#include "PlayerManager.h"
#include "..\ArenaMoudel\ArenaMoudel.h"
#include "..\SanguoPlayerBase.h"
#include "..\Common\ConfigManager.h"
#include <vector>
using namespace std;

extern DWORD g_CurOnlineCheck[6];
void TimerEvent_SG::SelfDestory()
{
	//rfalse("定时器清除___type=%d", this->eType);
	delete this;
}
void TimerEvent_SG::OnCancel()
{

}

BOOL CALLBACK FiveOclockEvent(LPIObject &Player, DWORD &number)
{
	CPlayer *pdest = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pdest != nullptr)
	{
		pdest->FiveOclockRefreshEvent();
		//调用脚本刷新回调
		if (g_Script.PrepareFunction("OnLoginRefresh"))
		{
			g_Script.PushParameter(pdest->GetSID());
			g_Script.PushParameter(5); //5点刷新数据
			g_Script.Execute();
		}
	}

	return 1;

}

BOOL CALLBACK TwelveOclockEvent(LPIObject &Player, DWORD &number)
{
	CPlayer *pdest = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pdest != nullptr)
	{
		pdest->TwelveOclockRefreshEvent();
	}
	return 1;
}

BOOL CALLBACK FourteenOclockEvent(LPIObject &Player, DWORD &number)
{
	CPlayer *pdest = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pdest != nullptr)
	{
		pdest->FourteenOclockRefreshEvent();
	}
	return 1;
}

BOOL CALLBACK EighteenOclockEvent(LPIObject &Player, DWORD &number)
{
	CPlayer *pdest = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pdest != nullptr)
	{
		pdest->EighteenOclockRefreshEvent();
	}
	return 1;
}

BOOL CALLBACK TwentyOclockEvent(LPIObject &Player, DWORD &number)
{
	CPlayer *pdest = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pdest != nullptr)
	{
		pdest->TwentyOclockRefreshEvent();
	}
	return 1;
}

BOOL CALLBACK TwentyOneOclockEvent(LPIObject &Player, DWORD &number)
{
	CPlayer *pdest = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pdest != nullptr)
	{
		pdest->TwentyOneOclockRefreshEvent();
	}
	return 1;
}

BOOL CALLBACK TwentyFourOclockEvent(LPIObject &Player, DWORD &number)
{
	CPlayer *pdest = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pdest != nullptr)
	{
		pdest->TwentyFourOclockRefreshEvent();
		//20150505 wk 玩家上线同步相关lua功能数据到前台
		g_Script.SetCondition(0, pdest, 0);
		LuaFunctor(g_Script, "OnPlayerOnline")[pdest->m_FixData.m_dwStaticID]();
		g_Script.CleanCondition();
	}
	return 1;
}

BOOL CALLBACK VarietyShopProcess(LPIObject &Player, DWORD &number)
{
	CPlayer *pdest = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pdest != nullptr)
	{
		pdest->VarietyShopRefreshEvent();
	}
	return 1;
}

BOOL CALLBACK ArenaShopProcess(LPIObject &Player, DWORD &number)
{
	CPlayer *pdest = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pdest != nullptr)
	{
		pdest->ArenaShopRefreshEvent();
	}
	return 1;
}

BOOL CALLBACK ExpeditionShopProcess(LPIObject &Player, DWORD &number)
{
	CPlayer *pdest = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pdest != nullptr)
	{
		pdest->ExpeditionShopRefreshEvent();
	}
	return 1;
}

BOOL CALLBACK LegionShopProcess(LPIObject &Player, DWORD &number)
{
	CPlayer *pdest = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pdest != nullptr)
	{
		pdest->LegionShopRefreshEvent();
	}
	return 1;
}

BOOL CALLBACK MiracleMerchantProcess(LPIObject &Player, DWORD &number)
{
	CPlayer *pdest = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pdest != nullptr)
	{
		pdest->MiracleMerchantRefreshEvent();
	}
	return 1;
}

BOOL CALLBACK GemMerchantProcess(LPIObject &Player, DWORD &number)
{
	CPlayer *pdest = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pdest != nullptr)
	{
		pdest->GemMerchantRefreshEvent();
	}
	return 1;
}

BOOL CALLBACK SoulPointExchangeShopProcess(LPIObject &Player, DWORD &number)
{
	CPlayer *pdest = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pdest != nullptr)
	{
		pdest->SoulPointExchangeShopRefreshEvent();
	}
	return 1;
}

BOOL CALLBACK WarOfLeagueShopProcess(LPIObject &Player, DWORD &number)
{
	CPlayer *pdest = (CPlayer*)Player->DynamicCast(IID_PLAYER);
	if (pdest != nullptr)
	{
		pdest->WarOfLeagueShopRefreshEvent();
	}
	return 1;
}

int TimerEvent_SG::FristReg()
{
	time_t _time = time(NULL);
	tm *_tm = localtime(&_time);
	int next_time;
	int refreshTime;

	//****************************************************************************
	//定时器___启动注册___每日24
	//rfalse("定时器___启动注册___每日24");
	TimerEvent_SG *timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay24;
	_tm->tm_hour = 23;
	_tm->tm_min = 59;
	_tm->tm_sec = 59;
	next_time = mktime(_tm) + 2 - _time;//晚上24点过1秒
	EventManager::GetInstance().Register(timer, next_time*1000); //注册为毫秒
	//****************************************************************************

	//****************************************************************************
	//定时器___启动注册___每日5点
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay5;
	_tm->tm_hour = 5;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	refreshTime = mktime(_tm);
	next_time = refreshTime - _time;
	if (next_time < 0)
	{
		next_time = refreshTime + SecsOfDay - _time;
	}
	EventManager::GetInstance().Register(timer, next_time * 1000); //注册为毫秒
	//****************************************************************************

	//****************************************************************************
	//定时器___启动注册___每日12点
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay12;
	_tm->tm_hour = 12;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	refreshTime = mktime(_tm);
	next_time = refreshTime - _time;
	if (next_time < 0)
	{
		next_time = refreshTime + SecsOfDay - _time;
	}
	EventManager::GetInstance().Register(timer, next_time * 1000); //注册为毫秒
	//****************************************************************************

	//****************************************************************************
	//定时器___启动注册___每日14点
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay14;
	_tm->tm_hour = 14;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	refreshTime = mktime(_tm);
	next_time = refreshTime - _time;
	if (next_time < 0)
	{
		next_time = refreshTime + SecsOfDay - _time;
	}
	EventManager::GetInstance().Register(timer, next_time * 1000); //注册为毫秒
	//****************************************************************************

	//****************************************************************************
	//定时器___启动注册___每日18点
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay18;
	_tm->tm_hour = 18;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	refreshTime = mktime(_tm);
	next_time = refreshTime - _time;
	if (next_time < 0)
	{
		next_time = refreshTime + SecsOfDay - _time;
	}
	EventManager::GetInstance().Register(timer, next_time * 1000); //注册为毫秒
	//****************************************************************************

	//****************************************************************************
	//定时器___启动注册___每日20点
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay20;
	_tm->tm_hour = 20;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	refreshTime = mktime(_tm);
	next_time = refreshTime - _time;
	if (next_time < 0)
	{
		next_time = refreshTime + SecsOfDay - _time;
	}
	EventManager::GetInstance().Register(timer, next_time * 1000); //注册为毫秒
	//****************************************************************************

	//****************************************************************************
	//定时器___启动注册___每日21点
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay21;
	_tm->tm_hour = 21;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	refreshTime = mktime(_tm);
	next_time = refreshTime - _time;
	if (next_time < 0)
	{
		next_time = refreshTime + SecsOfDay - _time;
	}
	EventManager::GetInstance().Register(timer, next_time * 1000); //注册为毫秒
	//****************************************************************************

	//****************************************************************************
	//定时器___启动注册___每小时59分
	//rfalse("定时器___启动注册___每小时59分");
	tm *_curTm = localtime(&_time);
	TimerEvent_SG *timer_hourly = new TimerEvent_SG;
	timer_hourly->eType = EventTypeSG::hourly;
	if (_curTm->tm_min<59)
	{
		_curTm->tm_min = 59;
		_curTm->tm_sec = 0;
		next_time = mktime(_curTm) - _time;
	}
	else
	{
		next_time = 60 * 60 - _curTm->tm_sec;//下个小时的59分00
	}
	//next_time = 10;//测试,每2分钟生成一个
	EventManager::GetInstance().Register(timer_hourly, next_time * 1000); //注册为毫秒
	//****************************************************************************

	//****************************************************************************
	//定时器每10分钟刷新
	TimerEvent_SG* timeEvent_EachTenMins = new TimerEvent_SG();
	timeEvent_EachTenMins->eType = EventTypeSG::EachTenMinutes;
	EventManager::GetInstance().Register(timeEvent_EachTenMins, 10* 60 * 1000); //注册为毫秒
	//****************************************************************************

	TimerEvent_SG* timeEvent_EachMin = new TimerEvent_SG();
	timeEvent_EachMin->eType = EventTypeSG::EachOneMinute;
	EventManager::GetInstance().Register(timeEvent_EachMin, 1 * 60 * 1000); //注册为毫秒

	//****************************************************************************
	//商店刷新事件
	CGlobalConfig& globalConfig = CConfigManager::getSingleton()->globalConfig;
	//****************************************************************************
	//定时器___启动注册___杂货铺刷新
	vector<int32_t>& vecVarietyClock = globalConfig.vecVarietyShopRefreshClock;
	for each (auto var in vecVarietyClock)
	{
		TimerEvent_SG* timer_Variety = new TimerEvent_SG;
		timer_Variety->eType = EventTypeSG::VarietyShopEvent;
		_tm->tm_hour = var;
		_tm->tm_min = 0;
		_tm->tm_sec = 0;
		refreshTime = mktime(_tm);
		next_time = refreshTime - _time;
		if (next_time < 0)
		{
			next_time = refreshTime + SecsOfDay - _time;
		}
		EventManager::GetInstance().Register(timer_Variety, next_time * 1000); //注册为毫秒
	}
	//****************************************************************************

	//****************************************************************************
	//定时器___启动注册___竞技场商店刷新
	vector<int32_t>& vecArenaClock = globalConfig.vecArenaShopRefreshClock;
	for each (auto var in vecArenaClock)
	{
		TimerEvent_SG* timer_Arena = new TimerEvent_SG;
		timer_Arena->eType = EventTypeSG::ArenaShopEvent;
		_tm->tm_hour = var;
		_tm->tm_min = 0;
		_tm->tm_sec = 0;
		refreshTime = mktime(_tm);
		next_time = refreshTime - _time;
		if (next_time < 0)
		{
			next_time = refreshTime + SecsOfDay - _time;
		}
		EventManager::GetInstance().Register(timer_Arena, next_time * 1000); //注册为毫秒
	}
	//****************************************************************************

	//****************************************************************************
	//定时器___启动注册___远征商店刷新
	vector<int32_t>& vecExpeditionClock = globalConfig.vecExpeditionShopRefreshClock;
	for each (auto var in vecExpeditionClock)
	{
		TimerEvent_SG* timer_Expediton = new TimerEvent_SG;
		timer_Expediton->eType = EventTypeSG::ExpeditionShopEvent;
		_tm->tm_hour = var;
		_tm->tm_min = 0;
		_tm->tm_sec = 0;
		refreshTime = mktime(_tm);
		next_time = refreshTime - _time;
		if (next_time < 0)
		{
			next_time = refreshTime + SecsOfDay - _time;
		}
		EventManager::GetInstance().Register(timer_Expediton, next_time * 1000); //注册为毫秒
	}
	//****************************************************************************

	//****************************************************************************
	//定时器___启动注册___军团商店刷新
	vector<int32_t>& vecLegionClock = globalConfig.vecLegionShopRefreshClock;
	for each (auto var in vecLegionClock)
	{
		TimerEvent_SG* timer_Legion = new TimerEvent_SG;
		timer_Legion->eType = EventTypeSG::LegionShopEvent;
		_tm->tm_hour = var;
		_tm->tm_min = 0;
		_tm->tm_sec = 0;
		refreshTime = mktime(_tm);
		next_time = refreshTime - _time;
		if (next_time < 0)
		{
			next_time = refreshTime + SecsOfDay - _time;
		}
		EventManager::GetInstance().Register(timer_Legion, next_time * 1000); //注册为毫秒
	}
	//****************************************************************************

	//****************************************************************************
	//定时器___启动注册___奇缘商人刷新
	vector<int32_t>& vecMiracleClock = globalConfig.vecMiracleMerchantRefreshClock;
	for each (auto var in vecMiracleClock)
	{
		TimerEvent_SG* timer_Miracle = new TimerEvent_SG;
		timer_Miracle->eType = EventTypeSG::MiracleMerchantEvent;
		_tm->tm_hour = var;
		_tm->tm_min = 0;
		_tm->tm_sec = 0;
		refreshTime = mktime(_tm);
		next_time = refreshTime - _time;
		if (next_time < 0)
		{
			next_time = refreshTime + SecsOfDay - _time;
		}
		EventManager::GetInstance().Register(timer_Miracle, next_time * 1000); //注册为毫秒
	}
	//****************************************************************************

	//****************************************************************************
	//定时器___启动注册___珍宝商人刷新
	vector<int32_t>& vecGemClock = globalConfig.vecGemMerchantRefreshClock;
	for each (auto var in vecGemClock)
	{
		TimerEvent_SG* timer_Gem = new TimerEvent_SG;
		timer_Gem->eType = EventTypeSG::GemMerchantEvent;
		_tm->tm_hour = var;
		_tm->tm_min = 0;
		_tm->tm_sec = 0;
		refreshTime = mktime(_tm);
		next_time = refreshTime - _time;
		if (next_time < 0)
		{
			next_time = refreshTime + SecsOfDay - _time;
		}
		EventManager::GetInstance().Register(timer_Gem, next_time * 1000); //注册为毫秒
	}
	//****************************************************************************

	//****************************************************************************
	//定时器___启动注册___将魂商店刷新
	vector<int32_t>& vecSoulClock = globalConfig.vecSoulExchangeRefreshClock;
	for each (auto var in vecSoulClock)
	{
		TimerEvent_SG* timer_Gem = new TimerEvent_SG;
		timer_Gem->eType = EventTypeSG::SoulExchangeShopEvent;
		_tm->tm_hour = var;
		_tm->tm_min = 0;
		_tm->tm_sec = 0;
		refreshTime = mktime(_tm);
		next_time = refreshTime - _time;
		if (next_time < 0)
		{
			next_time = refreshTime + SecsOfDay - _time;
		}
		EventManager::GetInstance().Register(timer_Gem, next_time * 1000); //注册为毫秒
	}
	//****************************************************************************

	//****************************************************************************
	//定时器___启动注册___国战商店刷新
	vector<int32_t>& vecWarOfLeagueClock = globalConfig.vecWarOfLeagueShopRefreshClock;
	for each (auto var in vecWarOfLeagueClock)
	{
		TimerEvent_SG* timer_Gem = new TimerEvent_SG;
		timer_Gem->eType = EventTypeSG::WarOfLeagueShopEvent;
		_tm->tm_hour = var;
		_tm->tm_min = 0;
		_tm->tm_sec = 0;
		refreshTime = mktime(_tm);
		next_time = refreshTime - _time;
		if (next_time < 0)
		{
			next_time = refreshTime + SecsOfDay - _time;
		}
		EventManager::GetInstance().Register(timer_Gem, next_time * 1000); //注册为毫秒
	}
	//****************************************************************************

	return 1;
}

void TimerEvent_SG::OnActive()
{
	//rfalse("定时器激活,type======%d", this->eType);

	switch (this->eType)
	{
	case EventTypeSG::EveryDay5:
		EveryDay_5();
		break;
	case EventTypeSG::EveryDay12:
		EveryDay_12();
		break;
	case EventTypeSG::EveryDay14:
		EveryDay_14();
		break;
	case EventTypeSG::EveryDay18:
		EveryDay_18();
		break;
	case EventTypeSG::EveryDay20:
		EveryDay_20();
		break;
	case EventTypeSG::EveryDay21:
		EveryDay_21();
		break;
	case EventTypeSG::EveryDay24:
		EveryDay_24();
		break;
	case EventTypeSG::hourly:
		EveryHour();
		break;
	case EventTypeSG::VarietyShopEvent:
		VarietyShop();
		break;
	case EventTypeSG::ArenaShopEvent:
		ArenaShop();
		break;
	case EventTypeSG::ExpeditionShopEvent:
		ExpeditionShop();
		break;
	case EventTypeSG::LegionShopEvent:
		LegionShop();
		break;
	case EventTypeSG::MiracleMerchantEvent:
		MiracleMerchant();
		break;
	case EventTypeSG::GemMerchantEvent:
		GemMerchant();
		break;
	case EventTypeSG::SoulExchangeShopEvent:
		SoulExchangeShop();
		break;
	case EventTypeSG::WarOfLeagueShopEvent:
		WarOfLeagueShop();
		break;
	case EventTypeSG::EachTenMinutes:
		ActiveTenMinEvent();
		break;
	case EventTypeSG::EachOneMinute:
		ActiveOneMinEvent();
		break;
	default:
		break;
	}
	if (m_Function._Empty() == false)
		m_Function(0);
}
///激活十分钟时间
int TimerEvent_SG::ActiveTenMinEvent()
{
	TimerEvent_SG *timer = new TimerEvent_SG();
	timer->eType = EventTypeSG::EachTenMinutes;
	EventManager::GetInstance().GetInstance().Register(timer, 10 * 60 * 1000); 

	//竞技场更新前50排名
	//CArenaMoudel::getSingleton()->UpdateTop50th();
	g_Script.CallFunc("SI_EachTenMinutes");
	return 1;
}

int TimerEvent_SG::ActiveOneMinEvent()
{
	TimerEvent_SG *timer = new TimerEvent_SG();
	timer->eType = EventTypeSG::EachOneMinute;
	EventManager::GetInstance().GetInstance().Register(timer, 1 * 60 * 1000);

	//竞技场更新前50排名
	CArenaMoudel::getSingleton()->UpdateTop50th();

	g_Script.CallFunc("SI_EachOneMinute");
	
	return 1;
}

///@brief 注册函数
TimerEvent_SG* TimerEvent_SG::SetCallback(FunctionPtr &func, int timePar)
{
	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::Function;
	timer->m_Function = func;
	EventManager::GetInstance().Register(timer, timePar * 1000); //注册为毫秒
	return timer;
}


int TimerEvent_SG::EveryHour()
{
	
	//******注册下小时事件***********************************************************
	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::hourly;
	EventManager::GetInstance().Register(timer, 60*60 * 1000);//注册为毫秒
	//EventManager::GetInstance().Register(timer, 10 * 1000);//测试,每2分钟生成一个
	//****************************************************************************
	//以下 为每小时需要执行的代码
	//****************************************************************************
	//每小时写入数据库在线人数
	g_Script.CallFunc("db_saveOnlineNum", g_CurOnlineCheck[1]);
	g_Script.CallFunc("SI_EveryHouer");

	//每小时保存数据
	CArenaMoudel::getSingleton()->BackupArenaRankData();
	
	return 1;
}

int TimerEvent_SG::EveryDay_5()
{
	//******注册第二天事件***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay5;
	//为尽量精确,每日都重新算下次24点距离现在的时间
	time_t _time = time(NULL);
	tm *_tm = localtime(&_time);
	_tm->tm_hour = 5;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	_tm->tm_mday += 1;

	EventManager::GetInstance().Register(timer, (mktime(_tm) - _time) * 1000);//注册为毫秒

	///5点每个在线的玩家会做的举动
	TraversalPlayers((TRAVERSALFUNC)FiveOclockEvent, 0);
	return 1;
}

int TimerEvent_SG::EveryDay_12()
{
	//******注册第二天事件***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay12;
	//为尽量精确,每日都重新算下次24点距离现在的时间
	time_t _time = time(NULL);
	tm *_tm = localtime(&_time);
	_tm->tm_hour = 12;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	_tm->tm_mday += 1;

	EventManager::GetInstance().Register(timer, (mktime(_tm) - _time) * 1000);//注册为毫秒

	///12点每个在线的玩家会做的举动
	TraversalPlayers((TRAVERSALFUNC)TwelveOclockEvent, 0);
	return 1;
}

int TimerEvent_SG::EveryDay_14()
{
	//******注册第二天事件***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay14;
	//为尽量精确,每日都重新算下次24点距离现在的时间
	time_t _time = time(NULL);
	tm *_tm = localtime(&_time);
	_tm->tm_hour = 14;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	_tm->tm_mday += 1;

	EventManager::GetInstance().Register(timer, (mktime(_tm) - _time) * 1000);//注册为毫秒

	///14点每个在线的玩家会做的举动
	TraversalPlayers((TRAVERSALFUNC)FourteenOclockEvent, 0);
	return 1;
}

int TimerEvent_SG::EveryDay_18()
{
	//******注册第二天事件***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay18;
	//为尽量精确,每日都重新算下次24点距离现在的时间
	time_t _time = time(NULL);
	tm *_tm = localtime(&_time);
	_tm->tm_hour = 18;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	_tm->tm_mday += 1;

	EventManager::GetInstance().Register(timer, (mktime(_tm) - _time) * 1000);//注册为毫秒

	///18点每个在线的玩家会做的举动
	TraversalPlayers((TRAVERSALFUNC)EighteenOclockEvent, 0);
	return 1;
}

int TimerEvent_SG::EveryDay_20()
{
	//******注册第二天事件***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay20;
	//为尽量精确,每日都重新算下次24点距离现在的时间
	time_t _time = time(NULL);
	tm *_tm = localtime(&_time);
	_tm->tm_hour = 20;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	_tm->tm_mday += 1;

	EventManager::GetInstance().Register(timer, (mktime(_tm) - _time) * 1000);//注册为毫秒

	///20点每个在线的玩家会做的举动
	TraversalPlayers((TRAVERSALFUNC)TwentyOclockEvent, 0);

	LuaFunctor(g_Script, "SI_EveryDay_20")();
	return 1;
}

int TimerEvent_SG::EveryDay_21()
{
	//******注册第二天事件***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay21;
	//为尽量精确,每日都重新算下次24点距离现在的时间
	time_t _time = time(NULL);
	tm *_tm = localtime(&_time);
	_tm->tm_hour = 21;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	_tm->tm_mday += 1;

	EventManager::GetInstance().Register(timer, (mktime(_tm) - _time) * 1000);//注册为毫秒

	///21点每个在线的玩家会做的举动
	TraversalPlayers((TRAVERSALFUNC)TwentyOneOclockEvent, 0);
	return 1;
}

int TimerEvent_SG::EveryDay_24()
{
	//******注册第二天事件***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay24;
	//为尽量精确,每日都重新算下次24点距离现在的时间
	time_t _time = time(NULL);
	tm *_tm = localtime(&_time);
	_tm->tm_hour = 0;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	_tm->tm_mday += 1;
	EventManager::GetInstance().Register(timer, (mktime(_tm) - _time) * 1000);//注册为毫秒

	//****************************************************************************
	//以下 为24时间点需要执行的代码
	//****************************************************************************

	LuaFunctor(g_Script, "SI_EveryDay_24")();

	///24点每个在线的玩家会做的举动
	TraversalPlayers((TRAVERSALFUNC)TwentyFourOclockEvent, 0);
	CArenaMoudel::getSingleton()->ArenaRoutine();
	return 1;
}

int TimerEvent_SG::VarietyShop()
{
	//******注册第二天事件***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::VarietyShopEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///注册为毫秒

	///杂货铺刷新
	TraversalPlayers((TRAVERSALFUNC)VarietyShopProcess, 0);
	return 1;
}

int TimerEvent_SG::ArenaShop()
{
	//******注册第二天事件***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::ArenaShopEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///注册为毫秒

	///竞技场商店刷新
	TraversalPlayers((TRAVERSALFUNC)ArenaShopProcess, 0);
	return 1;
}

int TimerEvent_SG::ExpeditionShop()
{
	//******注册第二天事件***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::ExpeditionShopEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///注册为毫秒

	///远征商店刷新
	TraversalPlayers((TRAVERSALFUNC)ExpeditionShopProcess, 0);
	return 1;
}

int TimerEvent_SG::LegionShop()
{
	//******注册第二天事件***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::LegionShopEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///注册为毫秒

	///军团商店刷新
	TraversalPlayers((TRAVERSALFUNC)LegionShopProcess, 0);
	return 1;
}

int TimerEvent_SG::MiracleMerchant()
{
	//******注册第二天事件***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::MiracleMerchantEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///注册为毫秒

	///奇缘商人刷新
	TraversalPlayers((TRAVERSALFUNC)MiracleMerchantProcess, 0);
	return 1;
}

int TimerEvent_SG::GemMerchant()
{
	//******注册第二天事件***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::GemMerchantEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///注册为毫秒

	///珍宝商人刷新
	TraversalPlayers((TRAVERSALFUNC)GemMerchantProcess, 0);
	return 1;
}

int TimerEvent_SG::SoulExchangeShop()
{
	//******注册第二天事件***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::SoulExchangeShopEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///注册为毫秒

	///将魂商店刷新
	TraversalPlayers((TRAVERSALFUNC)SoulPointExchangeShopProcess, 0);
	return 1;
}

int TimerEvent_SG::WarOfLeagueShop()
{
	//******注册第二天事件***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::WarOfLeagueShopEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///注册为毫秒

	///将魂商店刷新
	TraversalPlayers((TRAVERSALFUNC)WarOfLeagueShopProcess, 0);
	return 1;
}