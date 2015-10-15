#include "stdafx.h"
#include "ExchangeEndurance.h"
#include "stdafx.h"
#include "..\BaseDataManager.h"
#include "..\Common\PubTool.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "../networkmodule/playertypedef.h"
#include "..\Common\ConfigManager.h"
#include "..\TimerEvent_SG\TimerEvent_SG.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "Player.h"
#include "ScriptManager.h"
extern LPIObject GetPlayerBySID(DWORD dwStaticID);

CExchangeEndurance::CExchangeEndurance(CBaseDataManager& baseDataMgr)
	:m_baseDataMgr(baseDataMgr)
{
	m_ptrExchangedTimes = nullptr;
	m_iCurAvaliableCount = 0;
	m_ptrExchangeInforMsg = make_shared<SAExchangeEnduranceInfor>();
	m_ptrExchangeInforMsg->_protocol = SAExchangeEnduranceInfor::ANSWER_EXCHANGEENDURANCE_INFOR;
	m_ptrRefreshExchangeEnduranceInforMsg = make_shared<SDiamondExchangeEnduranceMsg>();
	m_ptrRefreshExchangeEnduranceInforMsg->_protocol = SDiamondExchangeEnduranceMsg::ANSWER_REFRESH;
}


CExchangeEndurance::~CExchangeEndurance()
{
}

void CExchangeEndurance::Init(void* data)
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

void CExchangeEndurance::RecvMsg(const SDiamondExchangeMsg *pMsg)
{
	const SDiamondExchangeEnduranceMsg* ptrMsg = static_cast<const SDiamondExchangeEnduranceMsg*>(pMsg);
	if (ptrMsg == nullptr)
		return;

	switch (ptrMsg->_protocol)
	{
	case SDiamondExchangeEnduranceMsg::REQUEST_EXCHANGEENDURANCE:
		RequestExchange();
		break;
	}
}

void CExchangeEndurance::ResetData(int sendMsg /*= 0*/)
{
	if (m_ptrExchangedTimes != nullptr)
		*m_ptrExchangedTimes = 0;

	if (sendMsg != 0)
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrRefreshExchangeEnduranceInforMsg.get(), sizeof(SDiamondExchangeEnduranceMsg));
	}
}

void CExchangeEndurance::RequestExchange()
{
	if (m_ptrExchangedTimes == nullptr)
		return;

	m_ptrExchangeInforMsg->bSuccess = false;

	///暂时将VIP等级的获取方法放到这里,因为VIP的等级“随时”都会变，所以“实时”获取
	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_baseDataMgr.GetSID())->DynamicCast(IID_PLAYER);
	int curDiamondCost = 0;
	if (pPlayer != nullptr)
	{
		g_Script.SetCondition(0, pPlayer, 0);
		lite::Variant ret;//从lua获取当前VIP等级所对应的能购买体力的次数
		LuaFunctor(g_Script, "SI_vip_getDetail")[g_Script.m_pPlayer->GetSID()][VipLevelFactor::VF_BuyStrength_Num](&ret);
		m_iCurAvaliableCount = (int)(ret);
		lite::Variant ret1;///从lua获取当前VIP等级和购买体力次数所对应的购买花费
		LuaFunctor(g_Script, "SI_getNeedvalue")[g_Script.m_pPlayer->GetSID()][Expense::Ex_Strength][*m_ptrExchangedTimes + 1](&ret1);
		curDiamondCost = (int)ret1;
		g_Script.CleanCondition();
	}
	else
		rfalse("获取不到CPlayer的指针");

	///如果兑换次数已经达到最大的次数了,不予兑换
	if (*m_ptrExchangedTimes >= m_iCurAvaliableCount)
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeInforMsg.get(), sizeof(SAExchangeEnduranceInfor));
		return;
	}

	///判断当前的元宝值是否可以购买体力
	if (!m_baseDataMgr.CheckGoods_SG(GoodsType::diamond, 0, curDiamondCost))
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeInforMsg.get(), sizeof(SAExchangeEnduranceInfor));
		return;
	}

	*m_ptrExchangedTimes += 1;
	m_ptrExchangeInforMsg->bSuccess = true;
	m_baseDataMgr.DecGoods_SG(GoodsType::diamond, 0, curDiamondCost,GoodsWay::bugEndurance);
	m_baseDataMgr.AddGoods_SG(GoodsType::endurance, 0, CConfigManager::getSingleton()->globalConfig.BuyPhysical, GoodsWay::bugEndurance);
	g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrExchangeInforMsg.get(), sizeof(SAExchangeEnduranceInfor));
}
