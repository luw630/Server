#include "stdafx.h"
#include "DessertSystem.h"
#include "time.h"
#include "..\Common\PubTool.h"
#include "..\BaseDataManager.h"
#include "..\Common\ConfigManager.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "Player.h"
#include "ScriptManager.h"
extern LPIObject GetPlayerBySID(DWORD dwStaticID);

CDessertSystem::CDessertSystem(CBaseDataManager& dataMgr)
	:m_BaseDataManager(dataMgr)
{
	m_pPlayer = nullptr;
	m_bJudgementFuncRegister = false;
	m_iDessertBakedCount = 0;
	m_ptrDessertBakedInforMsg = make_shared<SADessertBakedInfor>();
	m_ptrDessertBakedInforMsg->_protocol = SDessertMsg::ANSWER_DESSERT_BAKED_INFOR;
	m_ptrDessertRefreshMsg = make_shared<SDessertMsg>();
	m_ptrDessertRefreshMsg->_protocol = SDessertMsg::ANSWER_DESSERT_REFRESH;
}


CDessertSystem::~CDessertSystem()
{
}

void CDessertSystem::RecvMsg(const SMessage *pMsg)
{
	const SDessertMsg* ptrMsg = static_cast<const SDessertMsg*>(pMsg);
	if (ptrMsg == nullptr)
		return;

	switch (ptrMsg->_protocol)
	{
	case SDessertMsg::REQUEST_DESSERT:
		RequestBakeDessert(ptrMsg);
		break;
	case SDessertMsg::REQUEST_CLAIM_DESSERT:
		RequestClaimDessert(ptrMsg);
		break;
	}
}

void CDessertSystem::RequestBakeDessert(const SDessertMsg* pMsg)
{
	if (pMsg == nullptr)
		return;

	m_ptrDessertBakedInforMsg->bDessertAvaliable = false;

	if (m_pPlayer == nullptr)
		m_pPlayer = (CPlayer *)GetPlayerBySID(m_BaseDataManager.GetSID())->DynamicCast(IID_PLAYER);
	if (m_pPlayer != nullptr)
	{
		g_Script.SetCondition(0, m_pPlayer, 0);
		///��lua���ж��Ƿ������ȡ����
		LuaFunctor(g_Script, "SI_DispachDessert")[m_BaseDataManager.GetSID()]();
		g_Script.CleanCondition();
	}
	else
		rfalse("��ȡ����CPlayer��ָ��");
}

void CDessertSystem::RequestClaimDessert(const SDessertMsg* pMsg)
{
	if (pMsg == nullptr || m_ptrDessertBakedInforMsg == nullptr || !m_ptrDessertBakedInforMsg->bDessertAvaliable)
		return;

	m_ptrDessertBakedInforMsg->bDessertAvaliable = false;

	if (m_pPlayer == nullptr)
		m_pPlayer = (CPlayer *)GetPlayerBySID(m_BaseDataManager.GetSID())->DynamicCast(IID_PLAYER);
	if (m_pPlayer != nullptr)
	{
		g_Script.SetCondition(0, m_pPlayer, 0);
		///��lua����ȡ����
		LuaFunctor(g_Script, "SI_ClaimDessert")[m_BaseDataManager.GetSID()]();
		g_Script.CleanCondition();
	}
	else
		rfalse("��ȡ����CPlayer��ָ��");
}

void CDessertSystem::ResetData()
{
	///��lua�еĽ��յ�С�����ȡ������Ϊ0
	if (m_pPlayer == nullptr)
		m_pPlayer = (CPlayer *)GetPlayerBySID(m_BaseDataManager.GetSID())->DynamicCast(IID_PLAYER);
	if (m_pPlayer != nullptr)
	{
		LuaFunctor(g_Script, "SI_ResetDesserBakedCount")[m_BaseDataManager.GetSID()]();
	}
	else
		rfalse("��ȡ����CPlayer��ָ��");
}

void CDessertSystem::InitLuaData()
{
	__time64_t now = _time64(nullptr);
	__time64_t tCurTime = m_BaseDataManager.GetLogoutTime();
	__time64_t nextRfreshTime = tCurTime;

	tm curTime;
	_localtime64_s(&curTime, &now);
	curTime.tm_hour = 5;
	curTime.tm_min = 0;
	curTime.tm_sec = 0;
	nextRfreshTime = _mktime64(&curTime);

	///�жϵǳ�ʱ�䵽��½ʱ������ʱ�����Ƿ��Ѿ�ˢ������ص���ȡ����
	if (RefreshJudgement::JudgeCrossed(tCurTime, nextRfreshTime))
	{
		ResetData();
	}
}

void CDessertSystem::DispachDessert(bool bAvaliable, DessertType type, int dessertNum)
{
	m_ptrDessertBakedInforMsg->bDessertAvaliable = bAvaliable;
	m_ptrDessertBakedInforMsg->dessertType = type;
	m_ptrDessertBakedInforMsg->dessertNum = dessertNum;

	g_StoreMessage(m_BaseDataManager.GetDNID(), m_ptrDessertBakedInforMsg.get(), sizeof(SADessertBakedInfor));
}
