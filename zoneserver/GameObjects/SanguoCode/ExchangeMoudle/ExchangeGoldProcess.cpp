#include "stdafx.h"
#include "..\BaseDataManager.h"
#include "ExchangeGoldProcess.h"
#include "..\Common\PubTool.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "../networkmodule/playertypedef.h"
#include "..\Common\ConfigManager.h"
#include "..\TimerEvent_SG\TimerEvent_SG.h"
#include "..\..\Random.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "Player.h"
#include "ScriptManager.h"
extern LPIObject GetPlayerBySID(DWORD dwStaticID);

CExchangeGoldProcess::CExchangeGoldProcess(CBaseDataManager& baseDataMgr)
	:m_baseDataMgr(baseDataMgr)
{
	m_ptrExchangedTimes = nullptr;
	m_iCurAvaliableCount = 0;
	m_ptrExchangeGoldInforMsg = make_shared<SAExchangeGoldInfor>();
	m_ptrRefreshExchangeGoldInforMsg = make_shared<SDiamondExchangeGoldMsg>();
	m_ptrRefreshExchangeGoldInforMsg->_protocol = SDiamondExchangeGoldMsg::ANSWER_REFRESH;
}


CExchangeGoldProcess::~CExchangeGoldProcess()
{
}

void CExchangeGoldProcess::Init(void* data)
{
	if (data == nullptr)
		return;

	m_ptrExchangedTimes = static_cast<int*>(data);

	///下面创建刷新点
	__time64_t timeNow = _time64(nullptr);
	__time64_t nextRfreshTime = timeNow;
	tm refreshTm;
	errno_t err = _localtime64_s(&refreshTm, &timeNow);
	if (err != true)
	{
		refreshTm.tm_hour = 5;
		refreshTm.tm_min = 0;
		refreshTm.tm_sec = 0;
		nextRfreshTime = _mktime64(&refreshTm);
	}

	///先判断
	__time64_t tCurTime = m_baseDataMgr.GetLogoutTime();
	if (RefreshJudgement::JudgeCrossed(tCurTime, nextRfreshTime))
	{
		ResetData(0);
	}
}

void CExchangeGoldProcess::RecvMsg(const SDiamondExchangeMsg *pMsg)
{
	const SDiamondExchangeGoldMsg* ptrMsg = static_cast<const SDiamondExchangeGoldMsg*>(pMsg);
	if (ptrMsg == nullptr)
		return;

	///暂时将VIP等级的获取方法放到这里,因为VIP的等级“随时”都会变，所以“实时”获取
	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_baseDataMgr.GetSID())->DynamicCast(IID_PLAYER);
	if (pPlayer != nullptr)
	{
		g_Script.SetCondition(0, pPlayer, 0);
		lite::Variant ret;//从lua获取购买技能次数
		LuaFunctor(g_Script, "SI_vip_getDetail")[g_Script.m_pPlayer->GetSID()][VipLevelFactor::VF_OneToK_Num](&ret);
		m_iCurAvaliableCount = (int)ret;
		g_Script.CleanCondition();
	}
	else
		rfalse("获取不到CPlayer的指针");

	switch (ptrMsg->_protocol)
	{
	case SDiamondExchangeGoldMsg::REQUEST_EXCHANGEGOLD_ONCE:
		RequestExchangeOnce();
		break;
	case SDiamondExchangeGoldMsg::REQUEST_EXCHANGEGOLD_CONTINUOUS:
		RequestExchangeContinuous();
		break;
	}
}

void CExchangeGoldProcess::ResetData(int sendMsg /*= 0*/)
{
	if (m_ptrExchangedTimes != nullptr)
		*m_ptrExchangedTimes = 0;

	if (sendMsg != 0)
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrRefreshExchangeGoldInforMsg.get(), sizeof(SDiamondExchangeGoldMsg));
	}
}

void CExchangeGoldProcess::RequestExchangeOnce()
{
	if (m_ptrExchangedTimes == nullptr)
		return;

	m_ptrExchangeGoldInforMsg->exchangedTimes = 0;
	m_ptrExchangeGoldInforMsg->diamondCostEachTime = 0;

	///如果兑换次数已经达到最大的次数了,不予兑换
	if (*m_ptrExchangedTimes >= m_iCurAvaliableCount)
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - MAX_ITEM_NUM_SYN_TO_CLIENT * sizeof(DWORD));
		return;
	}

	int diamondCost = 0;
	///获取再兑换一次的配置信息
	const ExchangeGoldConfig* config = CConfigManager::getSingleton()->GetExchangeGoldConfig(*m_ptrExchangedTimes + 1);
	if (config == nullptr)
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - MAX_ITEM_NUM_SYN_TO_CLIENT * sizeof(DWORD));
		return;
	}

	diamondCost = config->m_useDiamondCount;
	///判断元宝是否足够,不足够不予兑换
	if (!m_baseDataMgr.CheckGoods_SG(GoodsType::diamond, 0, diamondCost))
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - MAX_ITEM_NUM_SYN_TO_CLIENT * sizeof(DWORD));
		return;
	}

	*m_ptrExchangedTimes += 1;
	m_ptrExchangeGoldInforMsg->diamondCostEachTime = diamondCost;
	m_ptrExchangeGoldInforMsg->exchangedTimes = 1;

	///获取乘以暴击倍数之前的金钱数
	m_ptrExchangeGoldInforMsg->exchangedInfors[0] = config->m_getGoldCount + CRandom::RandRange(config->m_goldRandomMin, config->m_goldRandomMax);

	///获取金钱的暴击倍数
	m_ptrExchangeGoldInforMsg->exchangedInfors[1] = CRandom::RandRange(1, 100) <= config->m_critOdds ? CRandom::RandRange(config->m_critMin, config->m_critMax) : 1;
	m_ptrExchangeGoldInforMsg->exchangedInfors[0] *= m_ptrExchangeGoldInforMsg->exchangedInfors[1];
	m_baseDataMgr.DecGoods_SG(GoodsType::diamond, 0, diamondCost,GoodsWay::goldenTouch);
	m_baseDataMgr.AddGoods_SG(GoodsType::money, 0, m_ptrExchangeGoldInforMsg->exchangedInfors[0], GoodsWay::goldenTouch);
	g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - (MAX_ITEM_NUM_SYN_TO_CLIENT - 2) * sizeof(DWORD));
}

void CExchangeGoldProcess::RequestExchangeContinuous()
{
	if (m_ptrExchangedTimes == nullptr)
		return;

	m_ptrExchangeGoldInforMsg->exchangedTimes = 0;
	m_ptrExchangeGoldInforMsg->diamondCostEachTime = 0;
	///如果兑换次数已经达到最大的次数了,不予兑换
	if (*m_ptrExchangedTimes >= m_iCurAvaliableCount)
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - MAX_ITEM_NUM_SYN_TO_CLIENT);
		return;
	}

	///获取再兑换一次的配置信息
	const ExchangeGoldConfig* config = CConfigManager::getSingleton()->GetExchangeGoldConfig(*m_ptrExchangedTimes + 1);
	if (config == nullptr || config->m_useDiamondCount == 0 || !m_baseDataMgr.CheckGoods_SG(GoodsType::diamond, 0, config->m_useDiamondCount))
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - MAX_ITEM_NUM_SYN_TO_CLIENT * sizeof(DWORD));
		return;
	}

	///获取再兑换一次的元宝消耗相同的兑换操作配置列表，用于实现将宝石消耗相同的兑换操作连续兑换完
	set<int> curAvaliableExchangeTimesList;
	if (!CConfigManager::getSingleton()->GetExchangeGoldConfigListByCost(config->m_useDiamondCount, curAvaliableExchangeTimesList) || curAvaliableExchangeTimesList.size() == 0)
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - MAX_ITEM_NUM_SYN_TO_CLIENT * sizeof(DWORD));
		return;
	}

	///限定当前的操作次数区间
	int diamondCost = 0;
	auto beginItor = curAvaliableExchangeTimesList.lower_bound(*m_ptrExchangedTimes);
	if (beginItor == curAvaliableExchangeTimesList.end())
	{
		rfalse("点金手配置文件出错");
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - MAX_ITEM_NUM_SYN_TO_CLIENT * sizeof(DWORD));
		return;
	}
	else if (*beginItor == *m_ptrExchangedTimes)
		++beginItor; ///<取到*m_ptrExchangedTimes + 1次操作的位置

	///取到指向m_iCurAvaliableCount次操作的位置
	auto endItor = curAvaliableExchangeTimesList.lower_bound(m_iCurAvaliableCount);
	if (endItor == curAvaliableExchangeTimesList.end())
	{
		--endItor;
	}
	else if (*endItor > m_iCurAvaliableCount)
		--endItor;

	diamondCost = m_baseDataMgr.GetDiamond();
	m_ptrExchangeGoldInforMsg->exchangedTimes = min(diamondCost / config->m_useDiamondCount, *endItor - *beginItor + 1);
	m_ptrExchangeGoldInforMsg->exchangedTimes = min(m_ptrExchangeGoldInforMsg->exchangedTimes, MAX_ITEM_NUM_SYN_TO_CLIENT / 2);
	diamondCost = m_ptrExchangeGoldInforMsg->exchangedTimes * config->m_useDiamondCount;

	for (int i = 0; i < m_ptrExchangeGoldInforMsg->exchangedTimes; ++i)
	{
		config = CConfigManager::getSingleton()->GetExchangeGoldConfig(i + *beginItor);
		if (config != nullptr)
		{
			///获取乘以暴击倍数之前的金钱数
			m_ptrExchangeGoldInforMsg->exchangedInfors[i] = config->m_getGoldCount + CRandom::RandRange(config->m_goldRandomMin, config->m_goldRandomMax);

			///获取金钱的暴击倍数
			m_ptrExchangeGoldInforMsg->exchangedInfors[i + m_ptrExchangeGoldInforMsg->exchangedTimes] = CRandom::RandRange(1, 100) <= config->m_critOdds ? CRandom::RandRange(config->m_critMin, config->m_critMax) : 1;
			m_ptrExchangeGoldInforMsg->exchangedInfors[i] *= m_ptrExchangeGoldInforMsg->exchangedInfors[i + m_ptrExchangeGoldInforMsg->exchangedTimes];
		}
		else
		{
			rfalse("点金手的操作次数%d的配置获取不到", i + *beginItor);
			///获取乘以暴击倍数之前的金钱数
			m_ptrExchangeGoldInforMsg->exchangedInfors[i] = m_ptrExchangeGoldInforMsg->exchangedInfors[i - 1];

			///获取金钱的暴击倍数
			m_ptrExchangeGoldInforMsg->exchangedInfors[i + m_ptrExchangeGoldInforMsg->exchangedTimes] = m_ptrExchangeGoldInforMsg->exchangedInfors[i + m_ptrExchangeGoldInforMsg->exchangedTimes - 1];
		}
		
		*m_ptrExchangedTimes += 1;
	}
	m_baseDataMgr.DecGoods_SG(GoodsType::diamond, 0, diamondCost, GoodsWay::goldenTouch);

	g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - (MAX_ITEM_NUM_SYN_TO_CLIENT - m_ptrExchangeGoldInforMsg->exchangedTimes * 2) * sizeof(DWORD));
	m_baseDataMgr.AddGoods_SG(GoodsType::money, m_ptrExchangeGoldInforMsg->exchangedTimes, (int*)m_ptrExchangeGoldInforMsg->exchangedInfors, nullptr, GoodsWay::goldenTouch);
}
