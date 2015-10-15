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
	//rfalse("��ʱ�����___type=%d", this->eType);
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
		//���ýű�ˢ�»ص�
		if (g_Script.PrepareFunction("OnLoginRefresh"))
		{
			g_Script.PushParameter(pdest->GetSID());
			g_Script.PushParameter(5); //5��ˢ������
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
		//20150505 wk �������ͬ�����lua�������ݵ�ǰ̨
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
	//��ʱ��___����ע��___ÿ��24
	//rfalse("��ʱ��___����ע��___ÿ��24");
	TimerEvent_SG *timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay24;
	_tm->tm_hour = 23;
	_tm->tm_min = 59;
	_tm->tm_sec = 59;
	next_time = mktime(_tm) + 2 - _time;//����24���1��
	EventManager::GetInstance().Register(timer, next_time*1000); //ע��Ϊ����
	//****************************************************************************

	//****************************************************************************
	//��ʱ��___����ע��___ÿ��5��
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
	EventManager::GetInstance().Register(timer, next_time * 1000); //ע��Ϊ����
	//****************************************************************************

	//****************************************************************************
	//��ʱ��___����ע��___ÿ��12��
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
	EventManager::GetInstance().Register(timer, next_time * 1000); //ע��Ϊ����
	//****************************************************************************

	//****************************************************************************
	//��ʱ��___����ע��___ÿ��14��
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
	EventManager::GetInstance().Register(timer, next_time * 1000); //ע��Ϊ����
	//****************************************************************************

	//****************************************************************************
	//��ʱ��___����ע��___ÿ��18��
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
	EventManager::GetInstance().Register(timer, next_time * 1000); //ע��Ϊ����
	//****************************************************************************

	//****************************************************************************
	//��ʱ��___����ע��___ÿ��20��
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
	EventManager::GetInstance().Register(timer, next_time * 1000); //ע��Ϊ����
	//****************************************************************************

	//****************************************************************************
	//��ʱ��___����ע��___ÿ��21��
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
	EventManager::GetInstance().Register(timer, next_time * 1000); //ע��Ϊ����
	//****************************************************************************

	//****************************************************************************
	//��ʱ��___����ע��___ÿСʱ59��
	//rfalse("��ʱ��___����ע��___ÿСʱ59��");
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
		next_time = 60 * 60 - _curTm->tm_sec;//�¸�Сʱ��59��00
	}
	//next_time = 10;//����,ÿ2��������һ��
	EventManager::GetInstance().Register(timer_hourly, next_time * 1000); //ע��Ϊ����
	//****************************************************************************

	//****************************************************************************
	//��ʱ��ÿ10����ˢ��
	TimerEvent_SG* timeEvent_EachTenMins = new TimerEvent_SG();
	timeEvent_EachTenMins->eType = EventTypeSG::EachTenMinutes;
	EventManager::GetInstance().Register(timeEvent_EachTenMins, 10* 60 * 1000); //ע��Ϊ����
	//****************************************************************************

	TimerEvent_SG* timeEvent_EachMin = new TimerEvent_SG();
	timeEvent_EachMin->eType = EventTypeSG::EachOneMinute;
	EventManager::GetInstance().Register(timeEvent_EachMin, 1 * 60 * 1000); //ע��Ϊ����

	//****************************************************************************
	//�̵�ˢ���¼�
	CGlobalConfig& globalConfig = CConfigManager::getSingleton()->globalConfig;
	//****************************************************************************
	//��ʱ��___����ע��___�ӻ���ˢ��
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
		EventManager::GetInstance().Register(timer_Variety, next_time * 1000); //ע��Ϊ����
	}
	//****************************************************************************

	//****************************************************************************
	//��ʱ��___����ע��___�������̵�ˢ��
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
		EventManager::GetInstance().Register(timer_Arena, next_time * 1000); //ע��Ϊ����
	}
	//****************************************************************************

	//****************************************************************************
	//��ʱ��___����ע��___Զ���̵�ˢ��
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
		EventManager::GetInstance().Register(timer_Expediton, next_time * 1000); //ע��Ϊ����
	}
	//****************************************************************************

	//****************************************************************************
	//��ʱ��___����ע��___�����̵�ˢ��
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
		EventManager::GetInstance().Register(timer_Legion, next_time * 1000); //ע��Ϊ����
	}
	//****************************************************************************

	//****************************************************************************
	//��ʱ��___����ע��___��Ե����ˢ��
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
		EventManager::GetInstance().Register(timer_Miracle, next_time * 1000); //ע��Ϊ����
	}
	//****************************************************************************

	//****************************************************************************
	//��ʱ��___����ע��___�䱦����ˢ��
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
		EventManager::GetInstance().Register(timer_Gem, next_time * 1000); //ע��Ϊ����
	}
	//****************************************************************************

	//****************************************************************************
	//��ʱ��___����ע��___�����̵�ˢ��
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
		EventManager::GetInstance().Register(timer_Gem, next_time * 1000); //ע��Ϊ����
	}
	//****************************************************************************

	//****************************************************************************
	//��ʱ��___����ע��___��ս�̵�ˢ��
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
		EventManager::GetInstance().Register(timer_Gem, next_time * 1000); //ע��Ϊ����
	}
	//****************************************************************************

	return 1;
}

void TimerEvent_SG::OnActive()
{
	//rfalse("��ʱ������,type======%d", this->eType);

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
///����ʮ����ʱ��
int TimerEvent_SG::ActiveTenMinEvent()
{
	TimerEvent_SG *timer = new TimerEvent_SG();
	timer->eType = EventTypeSG::EachTenMinutes;
	EventManager::GetInstance().GetInstance().Register(timer, 10 * 60 * 1000); 

	//����������ǰ50����
	//CArenaMoudel::getSingleton()->UpdateTop50th();
	g_Script.CallFunc("SI_EachTenMinutes");
	return 1;
}

int TimerEvent_SG::ActiveOneMinEvent()
{
	TimerEvent_SG *timer = new TimerEvent_SG();
	timer->eType = EventTypeSG::EachOneMinute;
	EventManager::GetInstance().GetInstance().Register(timer, 1 * 60 * 1000);

	//����������ǰ50����
	CArenaMoudel::getSingleton()->UpdateTop50th();

	g_Script.CallFunc("SI_EachOneMinute");
	
	return 1;
}

///@brief ע�ắ��
TimerEvent_SG* TimerEvent_SG::SetCallback(FunctionPtr &func, int timePar)
{
	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::Function;
	timer->m_Function = func;
	EventManager::GetInstance().Register(timer, timePar * 1000); //ע��Ϊ����
	return timer;
}


int TimerEvent_SG::EveryHour()
{
	
	//******ע����Сʱ�¼�***********************************************************
	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::hourly;
	EventManager::GetInstance().Register(timer, 60*60 * 1000);//ע��Ϊ����
	//EventManager::GetInstance().Register(timer, 10 * 1000);//����,ÿ2��������һ��
	//****************************************************************************
	//���� ΪÿСʱ��Ҫִ�еĴ���
	//****************************************************************************
	//ÿСʱд�����ݿ���������
	g_Script.CallFunc("db_saveOnlineNum", g_CurOnlineCheck[1]);
	g_Script.CallFunc("SI_EveryHouer");

	//ÿСʱ��������
	CArenaMoudel::getSingleton()->BackupArenaRankData();
	
	return 1;
}

int TimerEvent_SG::EveryDay_5()
{
	//******ע��ڶ����¼�***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay5;
	//Ϊ������ȷ,ÿ�ն��������´�24��������ڵ�ʱ��
	time_t _time = time(NULL);
	tm *_tm = localtime(&_time);
	_tm->tm_hour = 5;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	_tm->tm_mday += 1;

	EventManager::GetInstance().Register(timer, (mktime(_tm) - _time) * 1000);//ע��Ϊ����

	///5��ÿ�����ߵ���һ����ľٶ�
	TraversalPlayers((TRAVERSALFUNC)FiveOclockEvent, 0);
	return 1;
}

int TimerEvent_SG::EveryDay_12()
{
	//******ע��ڶ����¼�***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay12;
	//Ϊ������ȷ,ÿ�ն��������´�24��������ڵ�ʱ��
	time_t _time = time(NULL);
	tm *_tm = localtime(&_time);
	_tm->tm_hour = 12;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	_tm->tm_mday += 1;

	EventManager::GetInstance().Register(timer, (mktime(_tm) - _time) * 1000);//ע��Ϊ����

	///12��ÿ�����ߵ���һ����ľٶ�
	TraversalPlayers((TRAVERSALFUNC)TwelveOclockEvent, 0);
	return 1;
}

int TimerEvent_SG::EveryDay_14()
{
	//******ע��ڶ����¼�***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay14;
	//Ϊ������ȷ,ÿ�ն��������´�24��������ڵ�ʱ��
	time_t _time = time(NULL);
	tm *_tm = localtime(&_time);
	_tm->tm_hour = 14;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	_tm->tm_mday += 1;

	EventManager::GetInstance().Register(timer, (mktime(_tm) - _time) * 1000);//ע��Ϊ����

	///14��ÿ�����ߵ���һ����ľٶ�
	TraversalPlayers((TRAVERSALFUNC)FourteenOclockEvent, 0);
	return 1;
}

int TimerEvent_SG::EveryDay_18()
{
	//******ע��ڶ����¼�***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay18;
	//Ϊ������ȷ,ÿ�ն��������´�24��������ڵ�ʱ��
	time_t _time = time(NULL);
	tm *_tm = localtime(&_time);
	_tm->tm_hour = 18;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	_tm->tm_mday += 1;

	EventManager::GetInstance().Register(timer, (mktime(_tm) - _time) * 1000);//ע��Ϊ����

	///18��ÿ�����ߵ���һ����ľٶ�
	TraversalPlayers((TRAVERSALFUNC)EighteenOclockEvent, 0);
	return 1;
}

int TimerEvent_SG::EveryDay_20()
{
	//******ע��ڶ����¼�***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay20;
	//Ϊ������ȷ,ÿ�ն��������´�24��������ڵ�ʱ��
	time_t _time = time(NULL);
	tm *_tm = localtime(&_time);
	_tm->tm_hour = 20;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	_tm->tm_mday += 1;

	EventManager::GetInstance().Register(timer, (mktime(_tm) - _time) * 1000);//ע��Ϊ����

	///20��ÿ�����ߵ���һ����ľٶ�
	TraversalPlayers((TRAVERSALFUNC)TwentyOclockEvent, 0);

	LuaFunctor(g_Script, "SI_EveryDay_20")();
	return 1;
}

int TimerEvent_SG::EveryDay_21()
{
	//******ע��ڶ����¼�***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay21;
	//Ϊ������ȷ,ÿ�ն��������´�24��������ڵ�ʱ��
	time_t _time = time(NULL);
	tm *_tm = localtime(&_time);
	_tm->tm_hour = 21;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	_tm->tm_mday += 1;

	EventManager::GetInstance().Register(timer, (mktime(_tm) - _time) * 1000);//ע��Ϊ����

	///21��ÿ�����ߵ���һ����ľٶ�
	TraversalPlayers((TRAVERSALFUNC)TwentyOneOclockEvent, 0);
	return 1;
}

int TimerEvent_SG::EveryDay_24()
{
	//******ע��ڶ����¼�***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::EveryDay24;
	//Ϊ������ȷ,ÿ�ն��������´�24��������ڵ�ʱ��
	time_t _time = time(NULL);
	tm *_tm = localtime(&_time);
	_tm->tm_hour = 0;
	_tm->tm_min = 0;
	_tm->tm_sec = 0;
	_tm->tm_mday += 1;
	EventManager::GetInstance().Register(timer, (mktime(_tm) - _time) * 1000);//ע��Ϊ����

	//****************************************************************************
	//���� Ϊ24ʱ�����Ҫִ�еĴ���
	//****************************************************************************

	LuaFunctor(g_Script, "SI_EveryDay_24")();

	///24��ÿ�����ߵ���һ����ľٶ�
	TraversalPlayers((TRAVERSALFUNC)TwentyFourOclockEvent, 0);
	CArenaMoudel::getSingleton()->ArenaRoutine();
	return 1;
}

int TimerEvent_SG::VarietyShop()
{
	//******ע��ڶ����¼�***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::VarietyShopEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///ע��Ϊ����

	///�ӻ���ˢ��
	TraversalPlayers((TRAVERSALFUNC)VarietyShopProcess, 0);
	return 1;
}

int TimerEvent_SG::ArenaShop()
{
	//******ע��ڶ����¼�***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::ArenaShopEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///ע��Ϊ����

	///�������̵�ˢ��
	TraversalPlayers((TRAVERSALFUNC)ArenaShopProcess, 0);
	return 1;
}

int TimerEvent_SG::ExpeditionShop()
{
	//******ע��ڶ����¼�***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::ExpeditionShopEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///ע��Ϊ����

	///Զ���̵�ˢ��
	TraversalPlayers((TRAVERSALFUNC)ExpeditionShopProcess, 0);
	return 1;
}

int TimerEvent_SG::LegionShop()
{
	//******ע��ڶ����¼�***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::LegionShopEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///ע��Ϊ����

	///�����̵�ˢ��
	TraversalPlayers((TRAVERSALFUNC)LegionShopProcess, 0);
	return 1;
}

int TimerEvent_SG::MiracleMerchant()
{
	//******ע��ڶ����¼�***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::MiracleMerchantEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///ע��Ϊ����

	///��Ե����ˢ��
	TraversalPlayers((TRAVERSALFUNC)MiracleMerchantProcess, 0);
	return 1;
}

int TimerEvent_SG::GemMerchant()
{
	//******ע��ڶ����¼�***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::GemMerchantEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///ע��Ϊ����

	///�䱦����ˢ��
	TraversalPlayers((TRAVERSALFUNC)GemMerchantProcess, 0);
	return 1;
}

int TimerEvent_SG::SoulExchangeShop()
{
	//******ע��ڶ����¼�***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::SoulExchangeShopEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///ע��Ϊ����

	///�����̵�ˢ��
	TraversalPlayers((TRAVERSALFUNC)SoulPointExchangeShopProcess, 0);
	return 1;
}

int TimerEvent_SG::WarOfLeagueShop()
{
	//******ע��ڶ����¼�***********************************************************

	TimerEvent_SG *timer;
	timer = new TimerEvent_SG;
	timer->eType = EventTypeSG::WarOfLeagueShopEvent;
	EventManager::GetInstance().Register(timer, SecsOfDay * 1000);///ע��Ϊ����

	///�����̵�ˢ��
	TraversalPlayers((TRAVERSALFUNC)WarOfLeagueShopProcess, 0);
	return 1;
}