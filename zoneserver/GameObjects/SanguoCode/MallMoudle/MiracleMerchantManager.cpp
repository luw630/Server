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
	/// 如果长期开放 返回true
	if (m_pSpecialMallData->bAlwaysOpened)
		return true;

	/// 如果商城未开放 返回false
	if (!m_pSpecialMallData->bTemporaryOpened)
		return false;

	__time64_t curTime;
	_time64(&curTime);
	/// 计算生效至今的间隔时间
	DWORD intervalTime = (DWORD)difftime(curTime, m_tEffectiveTime);
	/// 如果间隔时间超出持续时间 则关闭商城
	if (intervalTime >= 3600)
	{
		m_pSpecialMallData->bTemporaryOpened = false;
		m_pSpecialMallData->dwElapsedTime = 0;
		return false;
	}

	/// 保存商城开启距今的已流逝时间
	m_pSpecialMallData->dwElapsedTime = intervalTime;

	return true;
}

void MiracleMerchantManager::OpenProcess()
{
	/// 如果商城已开启则不再触发开启判定
	if (IsOpened())
		return;

	/// 如果未达到开放等级 则不予开放
	if (m_pBaseDataMgr.GetPlayerLevel() < m_dwOpenMasterLevel)
		return;

	float randomValue = CRandom::RandRange(1, 100) / 100.0f;
	/// 随机几率值 并进行判断
	if (randomValue > m_fOpenProbability)
		return;

	_time64(&m_tEffectiveTime);
	ConverTool::ConvertInt64ToBytes(m_tEffectiveTime, m_pSpecialMallData->activateTime);
	m_pSpecialMallData->bTemporaryOpened = true;
	m_pSpecialMallData->dwElapsedTime = 0;
	if (nullptr == m_pOpenMsg)
	{
		m_pOpenMsg = make_shared<SQAOpenMallMsg>();
		m_pOpenMsg->m_MallType = GetType();
	}
	g_StoreMessage(m_pBaseDataMgr.GetDNID(), m_pOpenMsg.get(), sizeof(SQAOpenMallMsg));

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

	/// 获取玩家VIP等级 如果大于VIP等级限定则开放商店
	if (vipLevel > m_dwOpenVIPLevel)
	{
		///从lua获取奇缘商人开启限制
		lite::Variant ret2;
		BOOL result = FALSE;
		if (g_Script.PrepareFunction("Get_MoudelLimitDetail"))
		{
			g_Script.PushParameter("mall_moudel");
			g_Script.PushParameter("Miracle_OpenLimit");
			result = g_Script.Execute(&ret2);
		}

		if (!result || ret2.dataType == LUA_TNIL)
		{
			rfalse("Get_MoudelLimitDetail failed");
		}

		int limit = 0;
		try
		{
			limit = static_cast<int>(ret2);
		}
		catch (lite::Xcpt &e)
		{
			rfalse(2, 1, e.GetErrInfo());
		}

		if (0 == limit)
			m_pSpecialMallData->bAlwaysOpened = true;
	}
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
		VipLevelChange(vipLevel);
	}
}

void MiracleMerchantManager::GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs)
{
	CConfigManager::getSingleton()->GetMiracleMerchantCommodityConfigs(m_pBaseDataMgr.GetPlayerLevel(), vecConfigs);
}

bool MiracleMerchantManager::OpenMall(bool alwaysOpen)
{
	if (alwaysOpen)
	{
		lite::Variant ret;
		BOOL result = FALSE;
		if (g_Script.PrepareFunction("SI_vip_getlv"))
		{
			g_Script.PushParameter(m_pBaseDataMgr.GetSID());
			result = g_Script.Execute(&ret);
		}

		if (!result || ret.dataType == LUA_TNIL)
		{
			rfalse("SI_vip_getlv failed");
		}

		int vipLevel = 0;
		try
		{
			vipLevel = static_cast<int>(ret);
		}
		catch (lite::Xcpt &e)
		{
			rfalse(2, 1, e.GetErrInfo());
		}

		if (vipLevel <= m_dwOpenVIPLevel)
			return false;

		m_pSpecialMallData->bAlwaysOpened = true;
	}
	else
	{
		_time64(&m_tEffectiveTime);
		ConverTool::ConvertInt64ToBytes(m_tEffectiveTime, m_pSpecialMallData->activateTime);
		m_pSpecialMallData->bTemporaryOpened = true;
		m_pSpecialMallData->dwElapsedTime = 0;
		if (!CMallsMoudle::getSingleton()->RandomCommoditys(this))
			return false;

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

	return true;
}
