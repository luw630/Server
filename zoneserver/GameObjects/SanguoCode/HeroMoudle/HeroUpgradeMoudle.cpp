#include "stdafx.h"
#include "HeroUpgradeMoudle.h"
#include "..\BaseDataManager.h"
#include "..\Common\ConfigManager.h"
#include "..\StorageMoudle\StorageManager.h"
#include "..\AchievementMoudle\AchieveUpdate.h"
#include "..\AchievementMoudle\RandomAchieveUpdate.h"
#include "..\GuideMoudle\GuideManager.h"
#include"CMystring.h"
CHeroUpgradeMoudle::CHeroUpgradeMoudle()
	:globalConfig(CConfigManager::getSingleton()->globalConfig)
{
	m_ptrAcievementUpdate = nullptr;
	m_ptrRandomAchievementUpdator = nullptr;
}


CHeroUpgradeMoudle::~CHeroUpgradeMoudle()
{
	m_ptrAcievementUpdate = nullptr;
	m_ptrRandomAchievementUpdator = nullptr;
}


void CHeroUpgradeMoudle::DispatchHeroUpgradeMsg(SBaseHeroUpgrade *pMsg, CBaseDataManager* pBaseDataMgr)
{
	if (pMsg == nullptr || pBaseDataMgr == nullptr)
		return;

	m_ptrAcievementUpdate = pBaseDataMgr->GetLifeTimeAchievementUpdator();
	m_ptrRandomAchievementUpdator = pBaseDataMgr->GetRandomAchievementUpdator();
	switch (pMsg->_protocol)
	{
	case SBaseHeroUpgrade::HeroUpgrade_AttachEquipment:
	{
		_EquipHero(pMsg, pBaseDataMgr);
		break;
	}
	case SBaseHeroUpgrade::HeroUpgrade_RankRise:
		_HeroRankRise(pMsg, pBaseDataMgr);
		break;
	case SBaseHeroUpgrade::HeroUpgrade_StarLevelRise:
		_HeroStarLevelRise(pMsg, pBaseDataMgr);
		break;
	case SBaseHeroUpgrade::HeroUpgrade_ConscribeHero:
		_ConscribeHero(pMsg, pBaseDataMgr);
		break;
	case SBaseHeroUpgrade::HeroUpgrade_UpgradeLevel:
		_HeroUpgradeLevel(pMsg, pBaseDataMgr);
		break;
	default:
		break;
	}
}

bool CHeroUpgradeMoudle::_EquipHero(SBaseHeroUpgrade *pMsg, CBaseDataManager* pBaseDataMgr)
{ 
	SQHeroAttachEquipment *equipMsg = static_cast<SQHeroAttachEquipment*>(pMsg);
	if (equipMsg == nullptr || equipMsg->m_EquipmentNum <= 0)
		return false;

	SAHeroAttachEquipment msg;
	msg.m_HeroID = equipMsg->m_HeroID;
	msg.m_SuccessFlag = 0;
	msg.m_EquipmentNum = 0;
	bool bEuipSuccess = true;

	try
	{
		for (int i = 0; i < equipMsg->m_EquipmentNum; ++i)
		{
			const EquipmentConfig* equipmentConfig = CConfigManager::getSingleton()->GetEquipment(equipMsg->m_EquipmentID[i]);
			if (equipmentConfig == nullptr)
			{
				rfalse("服务器查找不到该装备");
				bEuipSuccess = false;
				break;
			}

			if (pBaseDataMgr->EquipHero(equipMsg->m_HeroID, equipmentConfig->m_EquipmentID, equipmentConfig->m_EquipmentType - 1, equipmentConfig->levelLimit) == false)
			{
				rfalse("武将穿装备不成功");
				bEuipSuccess = false;
				break;
			}

			msg.m_EquipmentIDs[i] = equipmentConfig->m_EquipmentID;
		}
	}
	catch (const std::exception& oor)
	{
		rfalse("穿戴装备的时候报错:%s", oor.what());
		msg.m_SuccessFlag = 0;
		msg.m_EquipmentNum = 0;
		g_StoreMessage(pBaseDataMgr->GetDNID(), &msg, sizeof(SAHeroAttachEquipment) - (EQUIP_MAX - msg.m_EquipmentNum) * sizeof(DWORD));
		return false;
	}

	msg.m_SuccessFlag = bEuipSuccess ? 1 : 0;
	if (!msg.m_SuccessFlag)
	{
		msg.m_EquipmentNum = 0;
	}
	else
	{
		msg.m_EquipmentNum = equipMsg->m_EquipmentNum;
	}

	pBaseDataMgr->EquipHeroEndProcess(equipMsg->m_HeroID);
	g_StoreMessage(pBaseDataMgr->GetDNID(), &msg, sizeof(SAHeroAttachEquipment) - (EQUIP_MAX - msg.m_EquipmentNum) * sizeof(DWORD));

	if (msg.m_SuccessFlag)
	{
		/// 引导判断  通知客户端前往下一步指引
		pBaseDataMgr->ProcessOperationOfGuide(FunctionMoudleType::Function_HeroUpgrade);
	}

	return true;
}

bool CHeroUpgradeMoudle::_HeroRankRise(SBaseHeroUpgrade *pMsg, CBaseDataManager* pBaseDataMgr)
{
	SQHeroRankRiseRequest *rankMsg = static_cast<SQHeroRankRiseRequest*>(pMsg);
	if (!pBaseDataMgr->HeroRankRise(rankMsg->m_HeroID, rankMsg->m_CurRank))
		return false;

	///更新武将进阶方面的成就
	if (m_ptrAcievementUpdate != nullptr)
		m_ptrAcievementUpdate-> UpdateHeroRankRiseAchieve(rankMsg->m_CurRank + 1);
	if (m_ptrRandomAchievementUpdator != nullptr)
		m_ptrRandomAchievementUpdator->UpdateHeroRankRiseAchieve(rankMsg->m_CurRank + 1);

	SAHeroRankRiseResult msg;
	msg.m_HeroID = rankMsg->m_HeroID;
	msg.m_SuccessFlag = true;
	g_StoreMessage(pBaseDataMgr->GetDNID(), &msg, sizeof(SAHeroRankRiseResult));
	///引导进阶判断  通知客户端前往下一步指引
	pBaseDataMgr->ProcessOperationOfGuide(FunctionMoudleType::Function_HeroUpgrade);

	return true;
}

bool CHeroUpgradeMoudle::_HeroStarLevelRise(SBaseHeroUpgrade *pMsg, CBaseDataManager* pBaseDataMgr)
{
	SQHeroStarLevelRiseRequest *starMsg = static_cast<SQHeroStarLevelRiseRequest*>(pMsg);
	DWORD curIndex = min(3, starMsg->m_CurStarLevel - 1);
	int32_t cost = globalConfig.vecHeroEvolutionCost[curIndex];
	if (pBaseDataMgr->GetMoney() < cost)
	{
		TalkToDnid(pBaseDataMgr->GetDNID(), CMyString::GetInstance().GetFormatString("OUT_OF_MONEY"));//金钱不足
		return false;
	}
	if (!pBaseDataMgr->HeroStarLevelRise(starMsg->m_HeroID, starMsg->m_CurStarLevel))
		return false;
	pBaseDataMgr->PlusMoney(-cost);
	SAHeroStarLevelRiseResult msg;
	msg.m_HeroID = starMsg->m_HeroID;
	msg.dwMoney = pBaseDataMgr->GetMoney();
	msg.bSuccess = true;
	g_StoreMessage(pBaseDataMgr->GetDNID(), &msg, sizeof(SAHeroStarLevelRiseResult));

	return true;
}

bool CHeroUpgradeMoudle::_ConscribeHero(SBaseHeroUpgrade* pMsg, CBaseDataManager* pBaseDataMgr)
{
	SQConscribeHeroRequest *pConMsg = static_cast<SQConscribeHeroRequest*>(pMsg);
	const HeroConfig* attrConfig = CConfigManager::getSingleton()->GetHeroConfig(pConMsg->m_HeroID);
	if (attrConfig == nullptr)
		return false;

	///暂时只能召唤1~3星的武将
	int32_t conscribeCost = globalConfig.vecHeroSummonCost[attrConfig->Star - 1];
	if (pBaseDataMgr->GetMoney() < conscribeCost)
	{
		TalkToDnid(pBaseDataMgr->GetDNID(), CMyString::GetInstance().GetFormatString("OUT_OF_MONEY")); //"没有足够的金钱用于招募该武将"
		return false;
	}
	const HeroStarConfig* starConfig = CConfigManager::getSingleton()->GetHeroStarAttr(pConMsg->m_HeroID);
	auto findResult = starConfig->StarData.find(attrConfig->Star);
	if (findResult == starConfig->StarData.end())
		return false;

	if (pBaseDataMgr->GetStorageManager().TryToRemoveItem(findResult->second.Item, findResult->second.Number) == false)
	{
		TalkToDnid(pBaseDataMgr->GetDNID(), CMyString::GetInstance().GetFormatString("NOT_ENOUGH_HERO_POINT"));//"没有足够的将魂用于招募该武将"
		return false;
	}
	pBaseDataMgr->PlusMoney(-conscribeCost);
	if (pBaseDataMgr->AddHero(attrConfig->HeroId) == false)
		return false;

	SAConscribeHeroResult msg;
	msg.m_HeroID = pConMsg->m_HeroID;
	msg.dwSoulStoneID = findResult->second.Item;
	msg.dwSoulStoneNum = findResult->second.Number;
	msg.dwMoney = pBaseDataMgr->GetMoney();
	g_StoreMessage(pBaseDataMgr->GetDNID(), &msg, sizeof(SAConscribeHeroResult));
	return true;
}

bool CHeroUpgradeMoudle::_HeroUpgradeLevel(SBaseHeroUpgrade* pMsg, CBaseDataManager* pBaseDataMgr)
{
	SQHeroUpgradeLevelRequest* pLevelMsg = static_cast<SQHeroUpgradeLevelRequest*>(pMsg);
	if (pLevelMsg == nullptr)
		return false;

	int32_t exp = 0;
	int level = 0;
	bool levelLimitted = false;
	int expPotionNum = pLevelMsg->dwExpPotionNum;
	if (pBaseDataMgr->GetStorageManager().GetExpPotionAbility(pLevelMsg->dwExpPotionID, expPotionNum, exp) == false)
	{
		rfalse("使用经验药水失败 物品ID : %d", pLevelMsg->dwExpPotionID);
		return false;
	}

	pBaseDataMgr->DecGoods_SG(GoodsType::item, pLevelMsg->dwExpPotionID, expPotionNum, GoodsWay::itemUse);

	if (pBaseDataMgr->PlusHeroExp(pLevelMsg->m_HeroID, exp, level, levelLimitted) == false)
	{
		rfalse("增加英雄经验值失败 英雄ID: %d", pLevelMsg->m_HeroID);
		return false;
	}
	SAHeroUpgradeLevelResult resultMsg;
	resultMsg.m_HeroID = pLevelMsg->m_HeroID;
	resultMsg.dwLevel = level;
	resultMsg.dwExperienceAmount = exp;

	g_StoreMessage(pBaseDataMgr->GetDNID(), &resultMsg, sizeof(SAHeroUpgradeLevelResult));
}
