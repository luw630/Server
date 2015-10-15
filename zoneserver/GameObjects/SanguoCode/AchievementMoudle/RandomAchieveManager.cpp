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

		///��������Ĺؿ����ǵ�ǰ����ָ���Ĺؿ����򷵻�
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
		///��ʼ����Ϣ
		m_ptrNewRandomAchievementGotMsg->achievementGot = false;
		m_ptrNewRandomAchievementGotMsg->achievementPersistentTimes = 0;
		m_ptrNewRandomAchievementGotMsg->achievementID = 0;

		///�����ǰ�����Ǻ�ҹ��������������񣬵������ж೤ʱ����ҹ����Ϣ�����ͻ���
		if (!m_ptrLuckyTimeSystem->IsLuckyTime())
		{
			m_ptrNewRandomAchievementGotMsg->achievementPersistentTimes = m_ptrLuckyTimeSystem->LuckyTimeRemaining();
			g_StoreMessage(baseDataManager.GetDNID(), m_ptrNewRandomAchievementGotMsg.get(), sizeof(SANewRandomAchievementGot));
			return;
		}
	}
	
	unordered_map<int32_t, int32_t> accessibleAchievement;
	///��ȡ��ǰ�ȼ����ŵĳɾ��б�
	if (!CConfigManager::getSingleton()->GetAccessibleAchievementIDS(baseDataManager.GetMasterLevel(), accessibleAchievement))
	{
		if (sendMsgFlag)
			g_StoreMessage(baseDataManager.GetDNID(), m_ptrNewRandomAchievementGotMsg.get(), sizeof(SANewRandomAchievementGot));
		return;
	}

	///�����ǰ�Ŀ��ŵĳɾ͸���Ϊ�գ��򷵻�
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

	///��ʼ����µĳɾ�
	auto findResult = accessibleAchievement.find(randomIndex);
	if (findResult != accessibleAchievement.end())
	{
		achievementID = findResult->second;
	}
	else
	{
		rfalse("����ɾͳ���");
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
	///������˺�ҹʱ�䣬�򱾳ɾ��޷���ȡ
	if (m_ptrLuckyTimeSystem == nullptr || !m_ptrLuckyTimeSystem->IsLuckyTime())
	{
		g_StoreMessage(baseDataManager.GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SARandomAchievementClaimConformed));
		return;
	}

	const SAchivementUnitData* achivementData = m_achievementDataMgr.GetAchievementUnitData(ptrMsg->achievementID);
	const RandomAchievementConfig* curAchievementConfig = CConfigManager::getSingleton()->GetRandomAchievementConfig(ptrMsg->achievementID);
	///����ɾ�ID�޷��ҵ����ߴ�ɴ���������Ҫ��������ȡ
	if (curAchievementConfig == nullptr || achivementData == nullptr || achivementData->accompulished || achivementData->completedTimes < curAchievementConfig->param2)
	{
		g_StoreMessage(baseDataManager.GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SARandomAchievementClaimConformed));
		return;
	}

	///��ʼ��ȡ��Ӧ�Ľ���
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
		///����������佫����Ѿ�ӵ����ת���ɶ�Ӧ�����Ľ�����Ƭ
		if (baseDataManager.ExistHero(curAchievementConfig->rewardID))
		{
			const HeroStarConfig* heroStarConfig = CConfigManager::getSingleton()->GetHeroStarAttr(curAchievementConfig->rewardID);
			if (heroStarConfig == nullptr)
			{
				rfalse("�Ҳ���IDΪ %d ��Ӣ�۵Ķ�Ӧ���Ǽ�����", curAchievementConfig->rewardID);
				return;
			}
			const HeroConfig* heroConfig = CConfigManager::getSingleton()->GetHeroConfig(curAchievementConfig->rewardID);
			if (heroConfig == nullptr)
			{
				rfalse("�޷��ҵ�IDΪ%d��Ӣ�۵�����", curAchievementConfig->rewardID);
				return;
			}

			///����ȫ�������������Ϣ�������Ӧ�Ľ�������
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

				///��������Ӷ�Ӧ���佫��Ƭ
				baseDataManager.AddGoods_SG(GoodsType::item, findResult->second.Item, num, GoodsWay::achievement);
			}
			else
			{
				rfalse("��ȡ�ɾ͵�ʱ���ȡ��Ӣ�������ļ�������Ǽ�����");
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
	if (curAchievementData != nullptr)///�����Ѿ��ɹ����øóɾ��������ɴ���+1
	{
		if (curAchievementData->groupType != achievementConfig->achieveType)
		{
			rfalse("�ɾ�����������������");
			return false;
		}

		m_ptrUpdateAchievementStateMsg->achievementID = curAchievementData->achievementID;
		m_ptrUpdateAchievementStateMsg->completedTimes = curAchievementData->completedTimes;
		g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrUpdateAchievementStateMsg.get(), sizeof(SAUpdateRandomAchievementState));

		return true;
	}

	return false;
}