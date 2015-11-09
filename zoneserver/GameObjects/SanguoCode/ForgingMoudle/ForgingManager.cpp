#include "stdafx.h"
#include "ForgingManager.h"
#include "..\BaseDataManager.h"
#include"..\Common\ConfigManager.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "..\StorageMoudle\StorageManager.h"
#include "../AchievementMoudle/RandomAchieveUpdate.h"
#include "..\MissionMoudle\CMissionUpdate.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "Player.h"
#include "ScriptManager.h"
extern LPIObject GetPlayerBySID(DWORD dwStaticID);


CForgingManager::CForgingManager()
{
	m_ptrBaseDataManager = nullptr;
}

CForgingManager::~CForgingManager()
{
	m_ptrBaseDataManager = nullptr;
}

void CForgingManager::OnRecvMsg(const SMessage *pMsg, CBaseDataManager *pBaseDataMgr)
{
	m_ptrBaseDataManager = pBaseDataMgr;
	if (m_ptrBaseDataManager == nullptr)
	{
		rfalse("锻造数据管理类CBaseDataManager为空");
		return;
	}

	static SAForgingPermission permissionMsg;
	permissionMsg.permitted = false;
	const SQAskToForging* ptrMsg = static_cast<const SQAskToForging*>(pMsg);
	if (ptrMsg == nullptr)
	{
		rfalse("锻造转换消息失败");
		///不予锻造
		g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
		return;
	}

	///如果锻造的英雄不存在或者锻造的装备不存在，则返回
	if (!m_ptrBaseDataManager->IsHeroEquipmentAttached(ptrMsg->heroID, ptrMsg->forgingEquipID))
	{
		///不予锻造
		g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
		rfalse("锻造选择的英雄不存在或者锻造的装备不存在");
		return;
	}

	switch (ptrMsg->_protocol)
	{
	case SQAskToForging::REQUEST_DIAMOND_FORGING:
		AskToDiamondForging(ptrMsg, permissionMsg);
		break;
	case SQAskToForging::REQUEST_MONEY_FORGING:
		AskToMoneyForging(ptrMsg, permissionMsg);
		break;
	default:
		break;
	}
}

void CForgingManager::AskToMoneyForging(const SQAskToForging* pMsg, SAForgingPermission& permissionMsg)
{
	const SQAskToMoneyForging* ptrMsg = static_cast<const SQAskToMoneyForging*>(pMsg);
	if (ptrMsg == nullptr)
		return;

	try
	{
		int curMaterialForgingExp = 0;
		const EquipmentConfig* equipConfig = nullptr;
		const ItemConfig* itemConfig = nullptr;
		for (int i = 0; i < ptrMsg->materialTypeNum; ++i)
		{
			///判断选择的材料是否存在或者足够
			if (!m_ptrBaseDataManager->GetStorageManager().ItemExistOrEnough(ptrMsg->materialInfor[i], ptrMsg->materialInfor[i + ptrMsg->materialTypeNum]))
			{
				///不予锻造
				g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
				rfalse("锻造选择的材料不存在");
				return;
			}

			///计算材料会产生的经验值
			equipConfig = CConfigManager::getSingleton()->GetEquipment(ptrMsg->materialInfor[i]);
			if (equipConfig != nullptr)
				curMaterialForgingExp += equipConfig->refiningExp * ptrMsg->materialInfor[i + ptrMsg->materialTypeNum];
			else
			{
				itemConfig = CConfigManager::getSingleton()->GetItemConfig(ptrMsg->materialInfor[i]);
				if (itemConfig != nullptr)
				{
					curMaterialForgingExp += itemConfig->RefiningExp * ptrMsg->materialInfor[i + ptrMsg->materialTypeNum];
				}
			}
		}

		int equipStarLevel = 0;
		///判断能否给对应的装备升级
		int increasedExp = m_ptrBaseDataManager->IncreaseHeroEquipmentForgingExp(ptrMsg->heroID, ptrMsg->forgingEquipID, curMaterialForgingExp, equipStarLevel);
		if (increasedExp != -1)
		{
			int moneyCost = increasedExp * CConfigManager::getSingleton()->globalConfig.ForgeMoneyCostScale;///暂时比例写死为120

			if (m_ptrBaseDataManager->CheckGoods_SG(GoodsType::money,0, moneyCost))
			{
				///减掉对应的消耗
				m_ptrBaseDataManager->DecGoods_SG(GoodsType::money, 0, moneyCost, GoodsWay::forgeMall);
				permissionMsg._protocol = SForgingMsg::ANSWER_MONEY_FORGING_PERMITTED;
			}
			else
			{
				///客户端不予锻造
				g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
				return;
			}

			///更新任务成就的相关数据
			CRandomAchieveUpdate*  m_ptrRandomAchievementUpdator = m_ptrBaseDataManager->GetRandomAchievementUpdator();
			CMissionUpdate* ptrMissionUpdator = m_ptrBaseDataManager->GetMissionUpdator();
			if (m_ptrRandomAchievementUpdator != nullptr)
				m_ptrRandomAchievementUpdator->UpdateForgingAchieve();
			if (ptrMissionUpdator != nullptr)
				ptrMissionUpdator->UpdateForgingMission(1);

			permissionMsg.permitted = true;
			permissionMsg.heroID = pMsg->heroID;
			permissionMsg.forgedEquipID = pMsg->forgingEquipID;
			permissionMsg.curEquipStarLevel = equipStarLevel;
			///客户端可以进行锻造的操作了
			g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));

			///可以锻造的话移出背包对应的材料
			m_ptrBaseDataManager->DecGoods_SG(GoodsType::item, ptrMsg->materialTypeNum, (int*)ptrMsg->materialInfor + ptrMsg->materialTypeNum, ptrMsg->materialInfor, GoodsWay::forgeMall);

			return;
		}
		else
		{
			///不予锻造
			g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
			rfalse("锻造无法进行");
			return;
		}
	}
	catch (const std::exception& oor)
	{
		rfalse("锻造消息有异常，退出");
		///不予锻造
		g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
		return;
	}
}

void CForgingManager::AskToDiamondForging(const SQAskToForging *pMsg, SAForgingPermission& permissionMsg)
{
	if (pMsg == nullptr)
		return;

	const ForgingConfig* forgingConfig = CConfigManager::getSingleton()->GetForgingConfig(pMsg->forgingEquipID);
	if (forgingConfig == nullptr)
		return;

	///暂时将VIP等级的获取方法放到这里,因为VIP的等级“随时”都会变，所以“实时”获取
	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_ptrBaseDataManager->GetSID())->DynamicCast(IID_PLAYER);
	int forgingAvaliableState = 0;
	if (pPlayer != nullptr)
	{
		g_Script.SetCondition(0, pPlayer, 0);
		lite::Variant ret;//从lua获取购买技能次数
		LuaFunctor(g_Script, "SI_vip_getDetail")[g_Script.m_pPlayer->GetSID()][VipLevelFactor::VF_OneKey_Num](&ret);
		forgingAvaliableState = (int)(ret);
		g_Script.CleanCondition();
	}
	else
		rfalse("获取不到CPlayer的指针");

	///判断当前的VIP等级是否开放了一键元宝锻造
	if (forgingAvaliableState <= 0)
	{
		///客户端不予锻造
		g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
	}

	///元宝祈福直接到顶
	auto findResult = --forgingConfig->levelExp.end();
	int equipStarLevel = 0;
	int increasedExp = m_ptrBaseDataManager->IncreaseHeroEquipmentForgingExp(pMsg->heroID, pMsg->forgingEquipID, findResult->first, equipStarLevel);
	if (increasedExp != -1)
	{
		int diamondCost = increasedExp * CConfigManager::getSingleton()->globalConfig.ForgeDiamondCostScale;///暂时写死元宝消耗为乘以3
		if (m_ptrBaseDataManager->CheckGoods_SG(GoodsType::diamond,0, diamondCost))
		{
			///减掉对应的消耗
			m_ptrBaseDataManager->DecGoods_SG(GoodsType::diamond,0, diamondCost,GoodsWay::forgeMall);
			permissionMsg._protocol = SForgingMsg::ANSWER_DIAMOND_FORGING_PERMITTED;
		}
		else
		{
			///客户端不予锻造
			g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
			return;
		}

		///更新任务成就的相关数据
		CRandomAchieveUpdate*  m_ptrRandomAchievementUpdator = m_ptrBaseDataManager->GetRandomAchievementUpdator();
		CMissionUpdate* ptrMissionUpdator = m_ptrBaseDataManager->GetMissionUpdator();
		if (m_ptrRandomAchievementUpdator != nullptr)
			m_ptrRandomAchievementUpdator->UpdateForgingAchieve();
		if (ptrMissionUpdator != nullptr)
			ptrMissionUpdator->UpdateForgingMission(1);

		///更新回复消息
		permissionMsg.permitted = true;
		permissionMsg.heroID = pMsg->heroID;
		permissionMsg.forgedEquipID = pMsg->forgingEquipID;
		permissionMsg.curEquipStarLevel = equipStarLevel;

		///客户端可以进行锻造的操作了
		g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
	}
	else
	{
		///不予锻造
		g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
		rfalse("锻造无法进行");
		return;
	}
}
