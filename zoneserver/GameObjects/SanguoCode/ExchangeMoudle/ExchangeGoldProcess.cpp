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

	///���洴��ˢ�µ�
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

	///���ж�
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

	///��ʱ��VIP�ȼ��Ļ�ȡ�����ŵ�����,��ΪVIP�ĵȼ�����ʱ������䣬���ԡ�ʵʱ����ȡ
	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_baseDataMgr.GetSID())->DynamicCast(IID_PLAYER);
	if (pPlayer != nullptr)
	{
		g_Script.SetCondition(0, pPlayer, 0);
		lite::Variant ret;//��lua��ȡ�����ܴ���
		LuaFunctor(g_Script, "SI_vip_getDetail")[g_Script.m_pPlayer->GetSID()][VipLevelFactor::VF_OneToK_Num](&ret);
		m_iCurAvaliableCount = (int)ret;
		g_Script.CleanCondition();
	}
	else
		rfalse("��ȡ����CPlayer��ָ��");

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

	///����һ������Ѿ��ﵽ���Ĵ�����,����һ�
	if (*m_ptrExchangedTimes >= m_iCurAvaliableCount)
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - MAX_ITEM_NUM_SYN_TO_CLIENT * sizeof(DWORD));
		return;
	}

	int diamondCost = 0;
	///��ȡ�ٶһ�һ�ε�������Ϣ
	const ExchangeGoldConfig* config = CConfigManager::getSingleton()->GetExchangeGoldConfig(*m_ptrExchangedTimes + 1);
	if (config == nullptr)
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - MAX_ITEM_NUM_SYN_TO_CLIENT * sizeof(DWORD));
		return;
	}

	diamondCost = config->m_useDiamondCount;
	///�ж�Ԫ���Ƿ��㹻,���㹻����һ�
	if (!m_baseDataMgr.CheckGoods_SG(GoodsType::diamond, 0, diamondCost))
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - MAX_ITEM_NUM_SYN_TO_CLIENT * sizeof(DWORD));
		return;
	}

	*m_ptrExchangedTimes += 1;
	m_ptrExchangeGoldInforMsg->diamondCostEachTime = diamondCost;
	m_ptrExchangeGoldInforMsg->exchangedTimes = 1;

	///��ȡ���Ա�������֮ǰ�Ľ�Ǯ��
	m_ptrExchangeGoldInforMsg->exchangedInfors[0] = config->m_getGoldCount + CRandom::RandRange(config->m_goldRandomMin, config->m_goldRandomMax);

	///��ȡ��Ǯ�ı�������
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
	///����һ������Ѿ��ﵽ���Ĵ�����,����һ�
	if (*m_ptrExchangedTimes >= m_iCurAvaliableCount)
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - MAX_ITEM_NUM_SYN_TO_CLIENT);
		return;
	}

	///��ȡ�ٶһ�һ�ε�������Ϣ
	const ExchangeGoldConfig* config = CConfigManager::getSingleton()->GetExchangeGoldConfig(*m_ptrExchangedTimes + 1);
	if (config == nullptr || config->m_useDiamondCount == 0 || !m_baseDataMgr.CheckGoods_SG(GoodsType::diamond, 0, config->m_useDiamondCount))
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - MAX_ITEM_NUM_SYN_TO_CLIENT * sizeof(DWORD));
		return;
	}

	///��ȡ�ٶһ�һ�ε�Ԫ��������ͬ�Ķһ����������б�����ʵ�ֽ���ʯ������ͬ�Ķһ����������һ���
	set<int> curAvaliableExchangeTimesList;
	if (!CConfigManager::getSingleton()->GetExchangeGoldConfigListByCost(config->m_useDiamondCount, curAvaliableExchangeTimesList) || curAvaliableExchangeTimesList.size() == 0)
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - MAX_ITEM_NUM_SYN_TO_CLIENT * sizeof(DWORD));
		return;
	}

	///�޶���ǰ�Ĳ�����������
	int diamondCost = 0;
	auto beginItor = curAvaliableExchangeTimesList.lower_bound(*m_ptrExchangedTimes);
	if (beginItor == curAvaliableExchangeTimesList.end())
	{
		rfalse("����������ļ�����");
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - MAX_ITEM_NUM_SYN_TO_CLIENT * sizeof(DWORD));
		return;
	}
	else if (*beginItor == *m_ptrExchangedTimes)
		++beginItor; ///<ȡ��*m_ptrExchangedTimes + 1�β�����λ��

	///ȡ��ָ��m_iCurAvaliableCount�β�����λ��
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
			///��ȡ���Ա�������֮ǰ�Ľ�Ǯ��
			m_ptrExchangeGoldInforMsg->exchangedInfors[i] = config->m_getGoldCount + CRandom::RandRange(config->m_goldRandomMin, config->m_goldRandomMax);

			///��ȡ��Ǯ�ı�������
			m_ptrExchangeGoldInforMsg->exchangedInfors[i + m_ptrExchangeGoldInforMsg->exchangedTimes] = CRandom::RandRange(1, 100) <= config->m_critOdds ? CRandom::RandRange(config->m_critMin, config->m_critMax) : 1;
			m_ptrExchangeGoldInforMsg->exchangedInfors[i] *= m_ptrExchangeGoldInforMsg->exchangedInfors[i + m_ptrExchangeGoldInforMsg->exchangedTimes];
		}
		else
		{
			rfalse("����ֵĲ�������%d�����û�ȡ����", i + *beginItor);
			///��ȡ���Ա�������֮ǰ�Ľ�Ǯ��
			m_ptrExchangeGoldInforMsg->exchangedInfors[i] = m_ptrExchangeGoldInforMsg->exchangedInfors[i - 1];

			///��ȡ��Ǯ�ı�������
			m_ptrExchangeGoldInforMsg->exchangedInfors[i + m_ptrExchangeGoldInforMsg->exchangedTimes] = m_ptrExchangeGoldInforMsg->exchangedInfors[i + m_ptrExchangeGoldInforMsg->exchangedTimes - 1];
		}
		
		*m_ptrExchangedTimes += 1;
	}
	m_baseDataMgr.DecGoods_SG(GoodsType::diamond, 0, diamondCost, GoodsWay::goldenTouch);

	g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeGoldInforMsg.get(), sizeof(SAExchangeGoldInfor) - (MAX_ITEM_NUM_SYN_TO_CLIENT - m_ptrExchangeGoldInforMsg->exchangedTimes * 2) * sizeof(DWORD));
	m_baseDataMgr.AddGoods_SG(GoodsType::money, m_ptrExchangeGoldInforMsg->exchangedTimes, (int*)m_ptrExchangeGoldInforMsg->exchangedInfors, nullptr, GoodsWay::goldenTouch);
}
