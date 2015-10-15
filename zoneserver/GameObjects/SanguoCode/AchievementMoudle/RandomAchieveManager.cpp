#include "stdafx.h"
#include "RandomAchieveManager.h"
#include "..\BaseDataManager.h"
#include "..\Common\ConfigManager.h"
#include "RandomAchieveDataManager.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "..\..\LuckyTimeSystem.h"
#include "..\..\GameWorld.h"
#include <vector>

CRandomAchieveManager::CRandomAchieveManager(CExtendedDataManager& dataMgr)
	:m_achievementDataMgr((CRandomAchieveDataManager&)dataMgr)
{
	m_ptrAchievementClaimConformedMsg = make_shared<SARandomAchievementClaimConformed>();
	m_ptrUpdateAchievementStateMsg = make_shared<SAUpdateRandomAchievementState>();
	m_ptrNewRandomAchievementGotMsg = make_shared<SANewRandomAchievementGot>();

	m_ptrLuckyTimeSystem = nullptr;
	GetGW()->GetLuckySystemPtr(&m_ptrLuckyTimeSystem);
}

CRandomAchieveManager::~CRandomAchieveManager()
{
	m_ptrLuckyTimeSystem = nullptr;
}

void CRandomAchieveManager::RecvMsg(const SMessage *pMsg)
{
	const SRandomAchievementMsg* achievementMsg = static_cast<const SRandomAchievementMsg*>(pMsg);
	if (achievementMsg == nullptr)
		return;

	switch (achievementMsg->_protocol)
	{
	case SRandomAchievementMsg::REQUESET_CLAIM_ACHIEVMENT:
		ClaimAchievement(achievementMsg);
		break;
	case SRandomAchievementMsg::REQUESET_RANDOM_NEW_ACHIEVEMENT:
		RandomAchieveTrophy();
		break;
	}
}

void CRandomAchieveManager::UpdateDuplicateAchieve(int curConquredTollgateID, int completedTimes /* = 1 */)
{
	if (m_ptrLuckyTimeSystem == nullptr || !m_ptrLuckyTimeSystem->IsLuckyTime())
		return;

	int achievementID;
	if (m_achievementDataMgr.IsAchievementExist(RandomAchievementType::Random_Battle, achievementID))
	{
		const RandomAchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetRandomAchievementConfig(achievementID);
		if (achievementConfig == nullptr)
			return;

		///如果征服的关卡不是当前任务指定的关卡，则返回
		if (achievementConfig->param1 != curConquredTollgateID)
			return;

		AchievementCompleteProgress(achievementConfig, completedTimes);
	}
}

void CRandomAchieveManager::UpdateHeroCollectionAchieve()
{
	if (m_ptrLuckyTimeSystem == nullptr || !m_ptrLuckyTimeSystem->IsLuckyTime())
		return;

	int achievementID;
	if (m_achievementDataMgr.IsAchievementExist(RandomAchievementType::Random_Collect, achievementID))
	{
		const RandomAchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetRandomAchievementConfig(achievementID);
		if (achievementConfig == nullptr)
			return;

		AchievementCompleteProgress(achievementConfig);
	}
}

void CRandomAchieveManager::UpdateHeroRankRiseAchieve(int curHeroRank)
{
	if (m_ptrLuckyTimeSystem == nullptr || !m_ptrLuckyTimeSystem->IsLuckyTime())
		return;

	int achievementID;
	if (m_achievementDataMgr.IsAchievementExist(RandomAchievementType::Random_Advanced, achievementID))
	{
		const RandomAchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetRandomAchievementConfig(achievementID);
		if (achievementConfig == nullptr)
			return;

		if (achievementConfig->param1 != curHeroRank)
			return;

		AchievementCompleteProgress(achievementConfig);
	}
}

void CRandomAchieveManager::UpdateMasterLevelAchieve(int curMasterLevel, int levelUpgradeAmount)
{
	if (m_ptrLuckyTimeSystem == nullptr || !m_ptrLuckyTimeSystem->IsLuckyTime())
		return;

	int achievementID;
	if (m_achievementDataMgr.IsAchievementExist(RandomAchievementType::Random_Level, achievementID))
	{
		const RandomAchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetRandomAchievementConfig(achievementID);
		if (achievementConfig == nullptr)
			return;

		if (achievementConfig->param1 != curMasterLevel)
			return;

		AchievementCompleteProgress(achievementConfig);
	}
}

void CRandomAchieveManager::UpdateBlessAchieve(int blessTimes /* = 1 */)
{
	if (m_ptrLuckyTimeSystem == nullptr || !m_ptrLuckyTimeSystem->IsLuckyTime())
		return;

	int achievementID;
	if (m_achievementDataMgr.IsAchievementExist(RandomAchievementType::Random_Blessing, achievementID))
	{
		const RandomAchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetRandomAchievementConfig(achievementID);
		if (achievementConfig == nullptr)
			return;

		AchievementCompleteProgress(achievementConfig, blessTimes);
	}
}

void CRandomAchieveManager::UpdateSkillUpgradeAchieve(int upgradeTimes /* = 1 */)
{
	if (m_ptrLuckyTimeSystem == nullptr || !m_ptrLuckyTimeSystem->IsLuckyTime())
		return;

	int achievementID;
	if (m_achievementDataMgr.IsAchievementExist(RandomAchievementType::Random_SkillUpgrade, achievementID))
	{
		const RandomAchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetRandomAchievementConfig(achievementID);
		if (achievementConfig == nullptr)
			return;

		AchievementCompleteProgress(achievementConfig, upgradeTimes);
	}
}

void CRandomAchieveManager::UpdateForgingAchieve()
{
	if (m_ptrLuckyTimeSystem == nullptr || !m_ptrLuckyTimeSystem->IsLuckyTime())
		return;

	int achievementID;
	if (m_achievementDataMgr.IsAchievementExist(RandomAchievementType::Random_Forging, achievementID))
	{
		const RandomAchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetRandomAchievementConfig(achievementID);
		if (achievementConfig == nullptr)
			return;

		AchievementCompleteProgress(achievementConfig);
	}
}

void CRandomAchieveManager::RandomAchieveTrophy(bool sendMsgFlag /* = true */)
{
	if (m_ptrLuckyTimeSystem == nullptr)
		return;

	CBaseDataManager& baseDataManager = m_achievementDataMgr.GetBaseDataMgr();
	if (sendMsgFlag)
	{
		///初始化消息
		m_ptrNewRandomAchievementGotMsg->achievementGot = false;
		m_ptrNewRandomAchievementGotMsg->achievementPersistentTimes = 0;
		m_ptrNewRandomAchievementGotMsg->achievementID = 0;

		///如果当前还不是黑夜，则不生成随机任务，但将还有多长时间后黑夜的信息发到客户端
		if (!m_ptrLuckyTimeSystem->IsLuckyTime())
		{
			m_ptrNewRandomAchievementGotMsg->achievementPersistentTimes = m_ptrLuckyTimeSystem->LuckyTimeRemaining();
			g_StoreMessage(baseDataManager.GetDNID(), m_ptrNewRandomAchievementGotMsg.get(), sizeof(SANewRandomAchievementGot));
			return;
		}
	}
	
	unordered_map<int32_t, int32_t> accessibleAchievement;
	///获取当前等级开放的成就列表
	if (!CConfigManager::getSingleton()->GetAccessibleAchievementIDS(baseDataManager.GetMasterLevel(), accessibleAchievement))
	{
		if (sendMsgFlag)
			g_StoreMessage(baseDataManager.GetDNID(), m_ptrNewRandomAchievementGotMsg.get(), sizeof(SANewRandomAchievementGot));
		return;
	}

	///如果当前的开放的成就个数为空，则返回
	int achievementNum = 1;
	achievementNum = min(accessibleAchievement.size(), achievementNum);
	if (achievementNum == 0)
	{
		if (sendMsgFlag)
			g_StoreMessage(baseDataManager.GetDNID(), m_ptrNewRandomAchievementGotMsg.get(), sizeof(SANewRandomAchievementGot));
		return;
	}

	int randomIndex = 0;
	DWORD achievementID;
	randomIndex = rand() % accessibleAchievement.size();

	///开始随机新的成就
	auto findResult = accessibleAchievement.find(randomIndex);
	if (findResult != accessibleAchievement.end())
	{
		achievementID = findResult->second;
	}
	else
	{
		rfalse("随机成就出错");
		if (sendMsgFlag)
			g_StoreMessage(baseDataManager.GetDNID(), m_ptrNewRandomAchievementGotMsg.get(), sizeof(SANewRandomAchievementGot));
		return;
	}

	m_achievementDataMgr.SetRemainingTimes(true, m_ptrLuckyTimeSystem->LuckyTimeRemaining());
	m_achievementDataMgr.SetNewAchievement(achievementID);

	if (sendMsgFlag)
	{
		m_ptrNewRandomAchievementGotMsg->achievementGot = true;
		m_ptrNewRandomAchievementGotMsg->achievementPersistentTimes = m_ptrLuckyTimeSystem->LuckyTimeRemaining();
		m_ptrNewRandomAchievementGotMsg->achievementID = achievementID;
		g_StoreMessage(baseDataManager.GetDNID(), m_ptrNewRandomAchievementGotMsg.get(), sizeof(SANewRandomAchievementGot));
	}
}

void CRandomAchieveManager::ClaimAchievement(const SRandomAchievementMsg* pMsg)
{
	const SQClaimRandomAchievement* ptrMsg = static_cast<const SQClaimRandomAchievement*>(pMsg);
	if (ptrMsg == nullptr || m_ptrAchievementClaimConformedMsg == nullptr)
		return;

	CBaseDataManager& baseDataManager = m_achievementDataMgr.GetBaseDataMgr();
	m_ptrAchievementClaimConformedMsg->bConformed = false;
	///如果过了黑夜时间，则本成就无法领取
	if (m_ptrLuckyTimeSystem == nullptr || !m_ptrLuckyTimeSystem->IsLuckyTime())
	{
		g_StoreMessage(baseDataManager.GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SARandomAchievementClaimConformed));
		return;
	}

	const SAchivementUnitData* achivementData = m_achievementDataMgr.GetAchievementUnitData(ptrMsg->achievementID);
	const RandomAchievementConfig* curAchievementConfig = CConfigManager::getSingleton()->GetRandomAchievementConfig(ptrMsg->achievementID);
	///如果成就ID无法找到或者达成次数不符合要求，则不予领取
	if (curAchievementConfig == nullptr || achivementData == nullptr || achivementData->accompulished || achivementData->completedTimes < curAchievementConfig->param2)
	{
		g_StoreMessage(baseDataManager.GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SARandomAchievementClaimConformed));
		return;
	}

	///开始领取相应的奖励
	switch (curAchievementConfig->rewardsType)
	{
	case AchievementRewardType::ART_Diamond:
		baseDataManager.AddGoods_SG(GoodsType::diamond, 0, curAchievementConfig->rewardAmount,GoodsWay::achievement);
		break;
	case AchievementRewardType::ART_Equip:
		baseDataManager.AddGoods_SG(GoodsType::item, curAchievementConfig->rewardID, curAchievementConfig->rewardAmount, GoodsWay::achievement);
		break;
	case AchievementRewardType::ART_Gold:
		baseDataManager.AddGoods_SG(GoodsType::money, 0, curAchievementConfig->rewardAmount, GoodsWay::achievement);
		break;
	case AchievementRewardType::ART_Hero:
		///如果奖励的武将玩家已经拥有则转换成对应数量的将魂碎片
		if (baseDataManager.ExistHero(curAchievementConfig->rewardID))
		{
			const HeroStarConfig* heroStarConfig = CConfigManager::getSingleton()->GetHeroStarAttr(curAchievementConfig->rewardID);
			if (heroStarConfig == nullptr)
			{
				rfalse("找不到ID为 %d 的英雄的对应的星级配置", curAchievementConfig->rewardID);
				return;
			}
			const HeroConfig* heroConfig = CConfigManager::getSingleton()->GetHeroConfig(curAchievementConfig->rewardID);
			if (heroConfig == nullptr)
			{
				rfalse("无法找到ID为%d的英雄的配置", curAchievementConfig->rewardID);
				return;
			}

			///按照全局配置里面的信息来换算对应的将魂数量
			int heroStarLevel = heroConfig->Star;
			auto findResult = heroStarConfig->StarData.find(heroStarLevel);
			if (findResult != heroStarConfig->StarData.end())
			{
				int num = CConfigManager::getSingleton()->globalConfig.ThreeStarHeroTransform;
				switch (heroStarLevel)
				{
				case 1:
					num = CConfigManager::getSingleton()->globalConfig.OneStarHeroTransform;
					break;
				case 2:
					num = CConfigManager::getSingleton()->globalConfig.TwoStarHeroTransform;
					break;
				case 3:
					num = CConfigManager::getSingleton()->globalConfig.ThreeStarHeroTransform;
					break;
				}

				///往背包添加对应的武将碎片
				baseDataManager.AddGoods_SG(GoodsType::item, findResult->second.Item, num, GoodsWay::achievement);
			}
			else
			{
				rfalse("领取成就的时候读取的英雄配置文件里面的星级有误");
			}
		}
		else
		{
			baseDataManager.AddGoods_SG(GoodsType::hero, curAchievementConfig->rewardID, curAchievementConfig->rewardAmount, GoodsWay::achievement);
		}
		break;
	case AchievementRewardType::ART_HeroDebris:
		baseDataManager.AddGoods_SG(GoodsType::item, curAchievementConfig->rewardID, curAchievementConfig->rewardAmount, GoodsWay::achievement);
		break;
	case AchievementRewardType::ART_Property:
		baseDataManager.AddGoods_SG(GoodsType::item, curAchievementConfig->rewardID, curAchievementConfig->rewardAmount, GoodsWay::achievement);
		break;
	}

	m_achievementDataMgr.SetAchievementAccompulished(ptrMsg->achievementID);
	m_ptrAchievementClaimConformedMsg->bConformed = true;
	m_ptrAchievementClaimConformedMsg->achievementID = ptrMsg->achievementID;
	g_StoreMessage(baseDataManager.GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SARandomAchievementClaimConformed));
}

bool CRandomAchieveManager::AchievementCompleteProgress(const RandomAchievementConfig* achievementConfig, int times /* = 1 */)
{
	if (achievementConfig == nullptr)
		return false;

	const SAchivementUnitData* curAchievementData = m_achievementDataMgr.SetAchievementCompleteTimes(achievementConfig->id, times);
	if (curAchievementData != nullptr)///代表已经成功设置该成就任务的完成次数+1
	{
		if (curAchievementData->groupType != achievementConfig->achieveType)
		{
			rfalse("成就所属组别的配置有误");
			return false;
		}

		m_ptrUpdateAchievementStateMsg->achievementID = curAchievementData->achievementID;
		m_ptrUpdateAchievementStateMsg->completedTimes = curAchievementData->completedTimes;
		g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrUpdateAchievementStateMsg.get(), sizeof(SAUpdateRandomAchievementState));

		return true;
	}

	return false;
}