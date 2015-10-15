#include "stdafx.h"
#include <time.h>
#include "MiracleMerchantManager.h"
#include "../Common/ConfigManager.h"
#include "NETWORKMODULE/SanguoPlayer.h"
#include "../../../NETWORKMODULE/SanguoPlayerMsg.h"
#include "../BaseDataManager.h"
#include "Random.h"
#include "Player.h"
#include "ScriptManager.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "../Common/ConfigManager.h"
#include "../Common/PubTool.h"
#include "GameWorld.h"
#include "MallsMoudle.h"

extern LPIObject GetPlayerBySID(DWORD dwStaticID);

DWORD MiracleMerchantManager::m_dwOpenVIPLevel = CConfigManager::getSingleton()->globalConfig.MiracleMerchantVIPLevelLimit;
DWORD MiracleMerchantManager::m_dwOpenMasterLevel = CConfigManager::getSingleton()->globalConfig.MiracleMerchantMasterLevelLimit;
float MiracleMerchantManager::m_fOpenProbability = CConfigManager::getSingleton()->globalConfig.miracleMerchantOpenProbability;

MiracleMerchantManager::MiracleMerchantManager(CBaseDataManager& baseDataManager)
	: CMallManager(baseDataManager)
	, m_pSpecialMallData(nullptr)
{
	m_fOpenFunc = std::bind(&MiracleMerchantManager::OpenProcess, this);
}


MiracleMerchantManager::~MiracleMerchantManager()
{
	m_pSpecialMallData = nullptr;
	m_pMallData = nullptr;
}

MallType MiracleMerchantManager::GetType() const
{
	return MallType::MiracleMerchant;
}

bool MiracleMerchantManager::InitDataMgr(void* pData)
{
	if (nullptr == pData)
		return false;
	m_pSpecialMallData = static_cast<SSanguoSpecialMallData*>(pData);
	m_pMallData = &m_pSpecialMallData->mallNormalData;
	m_tEffectiveTime = *((__time64_t*)m_pSpecialMallData->activateTime);
	m_pVecRefreshClock = &CConfigManager::getSingleton()->globalConfig.vecMiracleMerchantRefreshClock;
	if (JudgementRefresh(m_pBaseDataMgr.GetTimeData()))
		CMallsMoudle::getSingleton()->RandomCommoditys(this);
	SetRefreshInfo(false);
	return true;
}

GoodsWay MiracleMerchantManager::RechargeWay() const
{
	return GoodsWay::luckMan;
}

bool MiracleMerchantManager::IsOpened()
{
	/// ������ڿ��� ����true
	if (m_pSpecialMallData->bAlwaysOpened)
		return true;

	/// ����̳�δ���� ����false
	if (!m_pSpecialMallData->bTemporaryOpened)
		return false;

	__time64_t curTime;
	_time64(&curTime);
	/// ������Ч����ļ��ʱ��
	DWORD intervalTime = (DWORD)difftime(curTime, m_tEffectiveTime);
	/// ������ʱ�䳬������ʱ�� ��ر��̳�
	if (intervalTime >= 3600)
	{
		m_pSpecialMallData->bTemporaryOpened = false;
		m_pSpecialMallData->dwElapsedTime = 0;
		return false;
	}

	/// �����̳ǿ�������������ʱ��
	m_pSpecialMallData->dwElapsedTime = intervalTime;

	return true;
}

void MiracleMerchantManager::OpenProcess()
{
	/// ����̳��ѿ������ٴ��������ж�
	if (IsOpened())
		return;

	/// ���δ�ﵽ���ŵȼ� ���迪��
	if (m_pBaseDataMgr.GetPlayerLevel() < m_dwOpenMasterLevel)
		return;

	float randomValue = CRandom::RandRange(1, 100) / 100.0f;
	/// �������ֵ �������ж�
	if (randomValue > m_fOpenProbability)
		return;

	_time64(&m_tEffectiveTime);
	ConverTool::ConvertInt64ToBytes(m_tEffectiveTime, m_pSpecialMallData->activateTime);
	m_pSpecialMallData->bTemporaryOpened = true;
	m_pSpecialMallData->dwElapsedTime = 0;
	if (nullptr == m_pOpenMsg)
	{
		m_pOpenMsg = make_shared<SOpenMallMsg>();
		m_pOpenMsg->m_MallType = GetType();
	}
	g_StoreMessage(m_pBaseDataMgr.GetDNID(), m_pOpenMsg.get(), sizeof(SOpenMallMsg));

	if (!CMallsMoudle::getSingleton()->RandomCommoditys(this))
		return;
	if (nullptr == m_pRefreshCommodityMsg)
	{
		m_pRefreshCommodityMsg = make_shared<SRefreshCommodityResult>();
		m_pRefreshCommodityMsg->m_MallType = GetType();
	}
	m_pRefreshCommodityMsg->m_dwProperty = m_pBaseDataMgr.GetDiamond();
	m_pRefreshCommodityMsg->m_dwNextRefreshTime = GetNextRefreshTime();
	m_pRefreshCommodityMsg->m_dwRefreshedCount = GetRefreshedCount();

	const SSanguoCommodity* tempCommodity = nullptr;
	for (int i = 0; i < MALL_COMMODITY_NUM; ++i)
	{
		tempCommodity = &m_pMallData->m_arrCommodity[i];
		m_pRefreshCommodityMsg->m_arrCommodity[i].SetData(tempCommodity->m_dwID, tempCommodity->m_dwCount, tempCommodity->m_bSoldOut);
	}
	g_StoreMessage(m_pBaseDataMgr.GetDNID(), m_pRefreshCommodityMsg.get(), sizeof(SRefreshCommodityResult));
}

void MiracleMerchantManager::VipLevelChange(int vipLevel)
{
	if (m_pSpecialMallData->bAlwaysOpened)
		return;

	if (vipLevel > m_dwOpenVIPLevel)
		m_pSpecialMallData->bAlwaysOpened = true;
}

void MiracleMerchantManager::LoginProcess()
{
	if (m_pSpecialMallData->bAlwaysOpened)
		return;

	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_pBaseDataMgr.GetSID())->DynamicCast(IID_PLAYER);
	if (nullptr != pPlayer)
	{
		g_Script.SetCondition(0, pPlayer, 0);
		lite::Variant ret1;
		LuaFunctor(g_Script, "SI_vip_getlv")[g_Script.m_pPlayer->GetSID()](&ret1);
		int vipLevel = static_cast<int>(ret1);
		g_Script.CleanCondition();
		/// ��ȡ���VIP�ȼ� �������VIP�ȼ��޶��򿪷��̵�
		if (vipLevel > m_dwOpenVIPLevel)
		{
			m_pSpecialMallData->bAlwaysOpened = true;
		}
	}
}

void MiracleMerchantManager::GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs)
{
	CConfigManager::getSingleton()->GetMiracleMerchantCommodityConfigs(m_pBaseDataMgr.GetPlayerLevel(), vecConfigs);
}
