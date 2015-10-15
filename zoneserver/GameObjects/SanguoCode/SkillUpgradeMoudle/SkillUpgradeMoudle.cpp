#include "stdafx.h"
#include "SkillUpgradeMoudle.h"
#include "SkillUpgradeManager.h"
#include "../Common/ConfigManager.h"
#include "../BaseDataManager.h"
#include "../AchievementMoudle/RandomAchieveUpdate.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "Player.h"
#include "ScriptManager.h"
extern LPIObject GetPlayerBySID(DWORD dwStaticID);

CSkillUpgradeMoudle::CSkillUpgradeMoudle()
	: m_GlobalConfig(CConfigManager::getSingleton()->globalConfig)
{
	m_ptrRandomAchievementUpdator = nullptr;
}


CSkillUpgradeMoudle::~CSkillUpgradeMoudle()
{
	m_ptrRandomAchievementUpdator = nullptr;
}

void CSkillUpgradeMoudle::DispatchSkillUpgradeEvent(SSkillUpgrade* pMsg, CSkillUpgradeManager* pSkillUpgradeMgr)
{
	if (pSkillUpgradeMgr == nullptr)
		return;

	switch (pMsg->_protocol)
	{
	case SSkillUpgrade::SkillUpgrade_LearnSkill:
		m_ptrRandomAchievementUpdator = pSkillUpgradeMgr->GetBaseDataMgr().GetRandomAchievementUpdator();
		_SkillUpgradeProcess(static_cast<SQUpgradeHeroSkill*>(pMsg), pSkillUpgradeMgr);
		break;
	case SSkillUpgrade::SkillUpgrade_BuyPoint:
		_BuySkillPointProcess(pSkillUpgradeMgr);
		break;
	case SSkillUpgrade::SkillUpgrade_CheckPoint:
		{
			pSkillUpgradeMgr->CanLearnSkill();
			SACheckSkillPoint cMsg;
			cMsg.dwSkillPoint = pSkillUpgradeMgr->RemainingSkillPoint();
			cMsg.dwRemainingTime = pSkillUpgradeMgr->RecoverRemainingTime();
			g_StoreMessage(pSkillUpgradeMgr->GetBaseDataMgr().GetDNID(), &cMsg, sizeof(SACheckSkillPoint));
		}
		break;
	default:
		break;
	}
}

void CSkillUpgradeMoudle::_SkillUpgradeProcess(SQUpgradeHeroSkill* pMsg, CSkillUpgradeManager* pSkillUpgradeMgr)
{
	DWORD heroID = pMsg->dwHeroID;
	SAUpgradeHeroSkillResult resultMsg;
	resultMsg.dwHeroID = heroID;

	/*if (pSkillUpgradeMgr->CanLearnSkill())
	{
		///升级技能需要消耗的金钱
		int32_t cost = 0;
		///升级技能需要的技能点
		DWORD needSkillPoint = 0;
		DWORD skillLevel = 0;
		DWORD skillID = 0;
		DWORD levelDistance = 0;
		DWORD destLevel = 0;
		for (int i = 0, num = pMsg->dwValidNum; i < num; ++i)
		{
			skillID = pMsg->dwSkillID[i];
			skillLevel = baseDataMgr.GetHeroSkillLevel(heroID, skillID);
			if (skillLevel == 0)
				break;
			destLevel = pMsg->dwSkillLevel[i];
			levelDistance = destLevel - skillLevel;
			cost = ((skillLevel + destLevel - 1) * levelDistance / 2) * m_GlobalConfig.SkillCostMoney;
			if (baseDataMgr.GetMoney() < cost)
				break;
			if (pSkillUpgradeMgr->MultiUpgradeSkill(heroID, skillID, destLevel) == false)
				continue;
			baseDataMgr.PlusMoney(-cost);
			resultMsg.dwSkillPointCost += levelDistance;
		}
	}*/

	//resultMsg.bResult = pSkillUpgradeMgr->MultiUpgradeSkill(heroID, pMsg->dwSkillID, pMsg->dwSkillLevel, pMsg->dwValidNum, resultMsg.dwSkillPointCost);
	resultMsg.bResult = pSkillUpgradeMgr->MultiUpgradeSkill(heroID, pMsg->dwSkillID, pMsg->dwSkillLevel, resultMsg.dwSkillPointCost);

	CBaseDataManager& baseDataMgr = pSkillUpgradeMgr->GetBaseDataMgr();

	auto heroData = baseDataMgr.GetHero(heroID);
	if (nullptr == heroData)
	{
		CHAR text[128];
		sprintf_s(text, "武将ID(%d)有误!", heroID);
		TalkToDnid(baseDataMgr.GetDNID(), text);
		return;
	}
	for (int i = 0; i < MAX_SKILLNUM; ++i)
	{
		resultMsg.dwSkillID[i] = heroData->m_SkillInfoList[i].m_dwSkillID;
		resultMsg.dwSkillLevel[i] = heroData->m_SkillInfoList[i].m_dwSkillLevel;
	}
	
	resultMsg.dwValidNum = MAX_SKILLNUM;
	resultMsg.dwMoney = baseDataMgr.GetMoney();
	resultMsg.dwSkillPoint = pSkillUpgradeMgr->RemainingSkillPoint();
	resultMsg.dwRemainingTime = pSkillUpgradeMgr->RecoverRemainingTime();

	///技能升级所对应的成就更新
	if (m_ptrRandomAchievementUpdator != nullptr && resultMsg.bResult)
		m_ptrRandomAchievementUpdator->UpdateSkillUpgradeAchieve(resultMsg.dwSkillPointCost);

	g_StoreMessage(baseDataMgr.GetDNID(), &resultMsg, sizeof(SAUpgradeHeroSkillResult));
}

void CSkillUpgradeMoudle::_BuySkillPointProcess(CSkillUpgradeManager* pSkillUpgradeMgr)
{
	CBaseDataManager& baseDataMgr = pSkillUpgradeMgr->GetBaseDataMgr();
	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(baseDataMgr.GetSID())->DynamicCast(IID_PLAYER);
	int buyTimes = 0;
	if (pPlayer != nullptr)
	{
		g_Script.SetCondition(0, pPlayer, 0);
		lite::Variant ret1;//从lua获取购买技能次数
		LuaFunctor(g_Script, "SI_vip_getDetail")[g_Script.m_pPlayer->GetSID()][VipLevelFactor::VF_ButSkill_Num](&ret1);
		buyTimes = (int)(ret1);
		g_Script.CleanCondition();
	}
	else
		rfalse("获取不到CPlayer的指针");
	
	if (buyTimes == 0)
		return;
	///消耗的计算
	int32_t cost(m_GlobalConfig.BuySkillPointCost);
	if (baseDataMgr.GetDiamond() < cost)
		return;
	if (pSkillUpgradeMgr->BuySkillPoint() == false)
		return;

	baseDataMgr.DecGoods_SG(GoodsType::diamond, 0, cost, GoodsWay::buyskill);

	SABuySkillPointResult resultMsg;
	resultMsg.dwSkillPoint = pSkillUpgradeMgr->RemainingSkillPoint();
	resultMsg.dwRemainingTime = pSkillUpgradeMgr->RecoverRemainingTime();
	resultMsg.dwPurchasedTimes = pSkillUpgradeMgr->PurchasedSkillPointTimes();
	resultMsg.dwDiamond = baseDataMgr.GetDiamond();
	resultMsg.bResult = true;

	g_StoreMessage(baseDataMgr.GetDNID(), &resultMsg, sizeof(SABuySkillPointResult));
}
