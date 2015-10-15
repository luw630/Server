#include "stdafx.h"
#include "AchievementManager.h"
#include "AchievementDataManager.h"
#include "NETWORKMODULE\SanguoPlayer.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "..\Common\ConfigManager.h"
#include "..\BaseDataManager.h"
#include "time.h"
#include "../GuideMoudle/GuideManager.h"


CAchievementManager::CAchievementManager(CExtendedDataManager& dataMgr)
	:m_achievementDataMgr((CAchievementDataManager&)dataMgr)
{
	m_ptrAchievementClaimConformedMsg = make_shared<SAAchievementClaimConformed>();
	m_ptrUpdateAchievementStateMsg = make_shared<SAUpdateAchievementState>();
}


CAchievementManager::~CAchievementManager()
{
}

void CAchievementManager::RecvMsg(const SMessage *pMsg)
{
	const SAchievementMsg* ptrMsg = static_cast<const SAchievementMsg*>(pMsg);
	if (ptrMsg == nullptr)
	{
		rfalse("�ɾͽ��յ�����ϢΪ��");
		return;
	}

	const SQClaimAchievement* claimAchievementMsg = nullptr;
	switch (ptrMsg->_protocol)
	{
	case  SAchievementMsg::REQUESET_CLAIM_ACHIEVMENT:
		claimAchievementMsg = static_cast<const SQClaimAchievement*>(pMsg);
		if (claimAchievementMsg != nullptr)
		{
			ClaimAchievement(claimAchievementMsg);
		}
		else
			rfalse("�ͻ��˷���������ȡ�ɾ͵���Ϣת������");
		break;
	}
}

void CAchievementManager::UpdateDuplicateAchieve(int curConquredTollgateID, int completedTimes /* = 1 */)
{
	///��ʱǰ5����Ϊ������صĳɾ�
	for (int i = 0; i <= AchievementType::Battle4; ++i)
	{
		int updateAchievmentID = -1;
		updateAchievmentID = CConfigManager::getSingleton()->IsTollgateIDExistInSpecificAchievementGroup(i, curConquredTollgateID);
		if (updateAchievmentID == -1)
			continue;

		const SAchivementGroupData* groupData = m_achievementDataMgr.GetAchievementGroupData((AchievementType)i);
		if (groupData == nullptr)
			continue;

		////�������гɾͶ���ɵĻ��Ͳ�����һ�����ж���
		if (groupData->accompulished)
			continue;

		int curAchievementUnaccomplishedID = m_achievementDataMgr.GetAchievemntGroupCurUnaccomplishedID((AchievementType)i);
		if (curAchievementUnaccomplishedID == -1)
			continue;

		///NOTE:���߼���ȫ�����ڲ߻����õ������ļ��еĳɾͶ�ӦID��һ��һ��������
		const AchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetAchievementConfig(updateAchievmentID);
		if (achievementConfig == nullptr || achievementConfig->param1 != curConquredTollgateID)
			continue;

		bool bAchievementAccomplished = false;
		const SAchivementUnitData* curAchievmentUnityData = AchievementCompleteProgress(achievementConfig, bAchievementAccomplished, completedTimes);
		///�жϳɾ��Ƿ񡰴�ɡ�,��ɺ󽫸����ĳɾ͵����µĴ���ɵĳɾ�ID������һλ
		if (bAchievementAccomplished && curAchievementUnaccomplishedID == updateAchievmentID)
			ActiveNextAchievement(curAchievmentUnityData);
	}
}

void CAchievementManager::UpdateHeroCollectionAchieve()
{
	const SAchivementGroupData* groupData = m_achievementDataMgr.GetAchievementGroupData(AchievementType::Collect);
	if (groupData == nullptr)
	{
		return;
	}

	////�������гɾͶ���ɵĻ��Ͳ�����һ�����ж���
	if (groupData->accompulished)
	{
		return;
	}

	int curAchievementUnaccomplishedID = m_achievementDataMgr.GetAchievemntGroupCurUnaccomplishedID(AchievementType::Collect);
	if (curAchievementUnaccomplishedID == -1)
	{
		return;
	}

	///NOTE:���߼���ȫ�����ڲ߻����õ������ļ��еĳɾͶ�ӦID��һ��һ��������
	const AchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetAchievementConfig(curAchievementUnaccomplishedID);
	if (achievementConfig == nullptr)
	{
		return;
	}

	bool bAchievementAccomplished = false;
	const SAchivementUnitData* curAchievmentUnityData = AchievementCompleteProgress(achievementConfig, bAchievementAccomplished);
	///�жϳɾ��Ƿ񡰴�ɡ�,��ɺ󽫸����ĳɾ͵����µĴ���ɵĳɾ�ID������һλ
	if (bAchievementAccomplished)
		ActiveNextAchievement(curAchievmentUnityData);
}

void CAchievementManager:: UpdateHeroRankRiseAchieve(int curHeroRank)
{
	const SAchivementGroupData* groupData = m_achievementDataMgr.GetAchievementGroupData(AchievementType::Advanced);
	if (groupData == nullptr)
		return;

	////�������гɾͶ���ɵĻ��Ͳ�����һ�����ж���
	if (groupData->accompulished)
		return;

	///Ӣ���ռ��ɾ͸����еĳɾͲ�һ�������ԡ���������ɣ����Դ����߼�Ҳ��һ��
	int updateAchievementID = -1;
	const map<int32_t, vector<int32_t>>& heroRankUpgradeAchievementIDList = CConfigManager::getSingleton()->GetHeroRankUpgradeAchievementIDList();
	if (heroRankUpgradeAchievementIDList.size() == 0)
		return;

	///�жϵ�ǰ��Ʒ���Ƿ��ܴ���κγɾ�
	auto findResult = heroRankUpgradeAchievementIDList.lower_bound(curHeroRank);
	if (findResult == heroRankUpgradeAchievementIDList.end())
	{
		--findResult;
	}
	else
	{
		if (findResult->first > curHeroRank)
		{
			///��������Ʒ��Ҫ���ڵ�ǰƷ�ף����ж���ǰһƷ��
			if (findResult != heroRankUpgradeAchievementIDList.begin())
				--findResult;
			else///��ǰ��Ʒ�׻����ܴ���κγɾ�����
				return;
		}
	}

	///���Ҫ���³ɾ���ɴ������Ǹ��ɾ�ID, NOTE�����������ڲ߻������ĵ��еĳɾ����˳��
	const SAchivementUnitData* achievementUnitData = nullptr;
	const AchievementConfig* achievementConfig = nullptr;
	for (auto itor : findResult->second)
	{
		/////Ϊ�˷����߻������ļ�����ĳɾ�ID���ǵ��������, �����ID��С�ĳɾ�
		//if (updateAchievementID == -1)
		//	updateAchievementID = itor;
		//else if (itor < updateAchievementID)
		//	updateAchievementID = itor;
		//else if (itor > updateAchievementID)
		//{
		//	continue;
		//}

		achievementUnitData = m_achievementDataMgr.GetAchievementUnitData(itor);
		achievementConfig = CConfigManager::getSingleton()->GetAchievementConfig(itor);
		if (achievementUnitData == nullptr || achievementConfig == nullptr)
			continue;

		///���Ʒ��ΪcurHeroRank����ɾ�����ˣ���ת����Ʒ�׵���һ���ɾ�
		if (achievementUnitData->accompulished || (achievementUnitData->completedTimes != 0 && achievementUnitData->completedTimes >= achievementConfig->param2))
		{
			achievementUnitData = nullptr;
			achievementConfig = nullptr;
			continue;
		}

		/// NOTE�����������ڲ߻������ĵ��еĳɾ����˳��
		updateAchievementID = itor;
		break;
	}

	///�����ǰƷ������Ӧ��������سɾͶ�����ˣ��򲻼�����������
	if (updateAchievementID == -1)
		return;

	int curAchievementUnaccomplishedID = m_achievementDataMgr.GetAchievemntGroupCurUnaccomplishedID(AchievementType::Advanced);
	if (curAchievementUnaccomplishedID == -1)
		return;

	bool bAchievementAccomplished = false;
	const SAchivementUnitData* curAchievmentUnityData = AchievementCompleteProgress(achievementConfig, bAchievementAccomplished);
	///�жϳɾ��Ƿ񡰴�ɡ�,��ɺ󽫸����ĳɾ͵����µĴ���ɵĳɾ�ID������һλ
	if (bAchievementAccomplished && curAchievementUnaccomplishedID == updateAchievementID)
		ActiveNextAchievement(curAchievmentUnityData);
}

void CAchievementManager::UpdateMasterLevelAchieve(int curMasterLevel, int levelUpgradeAmount)
{
	const SAchivementGroupData* groupData = m_achievementDataMgr.GetAchievementGroupData(AchievementType::Level);
	if (groupData == nullptr)
		return;

	////�������гɾͶ���ɵĻ��Ͳ�����һ�����ж���
	if (groupData->accompulished)
		return;


	///�����������������
	int curLevelStep = 0;
	int curAchievementUnaccomplishedID = -1;
	int lastAchievementUnaccomplishedID = -1;
	while (curLevelStep < levelUpgradeAmount)
	{
		///���
		curAchievementUnaccomplishedID = m_achievementDataMgr.GetAchievemntGroupCurUnaccomplishedID(AchievementType::Level);
		if (curAchievementUnaccomplishedID == -1)
		{
			rfalse("���¾����ȼ��ɾͳ����޷���ȡ��ǰ������ɡ��ĳɾ�");
			return;
		}

		///�ж��Ƿ�ǰ�����ĳɾ�ȫ������ɡ��ˣ�����ɡ��������ȡ
		if (lastAchievementUnaccomplishedID != -1 && lastAchievementUnaccomplishedID == curAchievementUnaccomplishedID)
			return;

		///NOTE:���߼���ȫ�����ڲ߻����õ������ļ��еĳɾͶ�ӦID��һ��һ��������
		const AchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetAchievementConfig(curAchievementUnaccomplishedID);
		if (achievementConfig == nullptr)
		{
			rfalse("���¾����ȼ��ɾͳ����޷���óɾ�%d��Ӧ��������Ϣ", curAchievementUnaccomplishedID);
			return;
		}

		///�жϵ�ǰ�ľ����ȼ��Ƿ���Դ�ɸóɾ�
		if (achievementConfig->param1 > curMasterLevel)
			return;

		bool bAchievementAccomplished = false;
		const SAchivementUnitData* curAchievmentUnityData = AchievementCompleteProgress(achievementConfig, bAchievementAccomplished);
		///�жϳɾ��Ƿ񡰴�ɡ�,��ɺ󽫸����ĳɾ͵����µĴ���ɵĳɾ�ID������һλ
		if (bAchievementAccomplished)
			ActiveNextAchievement(curAchievmentUnityData);

		lastAchievementUnaccomplishedID = curAchievementUnaccomplishedID;
		++curLevelStep;
	}
}

void CAchievementManager::ClaimAchievement(const SAchievementMsg* pMsg)
{
	m_ptrAchievementClaimConformedMsg->bConformed = false;
	m_ptrAchievementClaimConformedMsg->bAchievementGroupAccomplished = false;
	const SQClaimAchievement* ptrClaimMsg = static_cast<const SQClaimAchievement*>(pMsg);
	///���ת��ʧ�� ������ȡ
	if (ptrClaimMsg == nullptr)
	{
		g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));
		return;
	}

	///��ȡҪ��ȡ�ĳɾ͵����������
	byte curAchievementType = (byte)CConfigManager::getSingleton()->GetAchievementGroupType(ptrClaimMsg->achievementID);
	if (curAchievementType == -1)
		g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));

	///��ȡҪ��ȡ�ĳɾ͵���ػ�������
	m_ptrAchievementClaimConformedMsg->achievementType = curAchievementType;
	const SAchivementGroupData* groupData = m_achievementDataMgr.GetAchievementGroupData((AchievementType)curAchievementType);
	const SAchivementUnitData* achievementUnitData = m_achievementDataMgr.GetAchievementUnitData(ptrClaimMsg->achievementID);
	const AchievementConfig* curAchievementConfig = CConfigManager::getSingleton()->GetAchievementConfig(ptrClaimMsg->achievementID);
	if (groupData == nullptr || achievementUnitData == nullptr || curAchievementConfig == nullptr)
	{
		g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));
		return;
	}

	////�������гɾͶ���ɵĻ��Ͳ�����һ�����ж���
	if (groupData->accompulished)
	{
		g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));
		return;
	}
	
	///�ɾ��Ѿ���ȡ�Ļ�������һ����ȡ
	if (achievementUnitData->accompulished)
	{
		if (groupData->newestAchivementID == achievementUnitData->achievementID)
		{
			ActiveNextAchievement(achievementUnitData);
			rfalse("�ɾ͸������µĴ���ɳɾ�IDʧ�ܣ����µĴ���ɳɾ�IDӦ��Ϊ%d", groupData->newestAchivementID);
		}
		g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));
		return;
	}

	///�ɾ�δ�ﵽ������ȡ������
	if (achievementUnitData->completedTimes == 0 || achievementUnitData->completedTimes < curAchievementConfig->param2)
	{
		g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));
		return;
	}

	///���óɾ͵���ȡ״̬
	m_ptrAchievementClaimConformedMsg->achievementID = ptrClaimMsg->achievementID;
	m_ptrAchievementClaimConformedMsg->bConformed = true;
	m_achievementDataMgr.SetAchievementAccompulished(ptrClaimMsg->achievementID);
	///�ж��Ƿ����ɾ�ȫ�������
	int nextAchievemntID = 0;
	if (CConfigManager::getSingleton()->GetNextAchievementID(achievementUnitData->achievementID, nextAchievemntID))
	{
		if (nextAchievemntID == -1)
		{
			m_ptrAchievementClaimConformedMsg->bAchievementGroupAccomplished = true;
			m_achievementDataMgr.SetAchievementGroupAccompulished((AchievementType)curAchievementType);
			m_ptrAchievementClaimConformedMsg->newsetAchievementID = achievementUnitData->achievementID;
		}
		else
		{
			m_achievementDataMgr.SetAchievementGroupNewsetID((AchievementType)curAchievementType, nextAchievemntID);
			m_ptrAchievementClaimConformedMsg->newsetAchievementID = nextAchievemntID;
		}
	}

	///��ʼ��ȡ��Ӧ�Ľ���
	CBaseDataManager& baseDataManager = m_achievementDataMgr.GetBaseDataMgr();
	switch (curAchievementConfig->rewardsType)
	{
	case AchievementRewardType::ART_Diamond:
		baseDataManager.AddGoods_SG(GoodsType::diamond, 0, curAchievementConfig->rewardAmount, GoodsWay::achievement);
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
				m_ptrAchievementClaimConformedMsg->bConformed = false;
				m_ptrAchievementClaimConformedMsg->bAchievementGroupAccomplished = false;
				g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));
				return;
			}
			const HeroConfig* heroConfig = CConfigManager::getSingleton()->GetHeroConfig(curAchievementConfig->rewardID);
			if (heroConfig == nullptr)
			{
				rfalse("�޷��ҵ�IDΪ%d��Ӣ�۵�����", curAchievementConfig->rewardID);
				m_ptrAchievementClaimConformedMsg->bConformed = false;
				m_ptrAchievementClaimConformedMsg->bAchievementGroupAccomplished = false;
				g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));
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
				///�����ж� ��֪ͨ�ͻ���ǰ����һ��ָ��
				baseDataManager.ProcessOperationOfGuide(FunctionMoudleType::Function_Achievement, curAchievementConfig->rewardID);
			}
			else
			{
				rfalse("��ȡ�ɾ͵�ʱ���ȡ��Ӣ�������ļ�������Ǽ�����");
			}
		}
		else
		{
			baseDataManager.AddGoods_SG(GoodsType::hero, curAchievementConfig->rewardID, curAchievementConfig->rewardAmount, GoodsWay::achievement);
			///�����ж� ��֪ͨ�ͻ���ǰ����һ��ָ��
			baseDataManager.ProcessOperationOfGuide(FunctionMoudleType::Function_Achievement, curAchievementConfig->rewardID);
		}
		break;
	case AchievementRewardType::ART_HeroDebris:
		baseDataManager.AddGoods_SG(GoodsType::item, curAchievementConfig->rewardID, curAchievementConfig->rewardAmount, GoodsWay::achievement);
		break;
	case AchievementRewardType::ART_Property:
		baseDataManager.AddGoods_SG(GoodsType::item, curAchievementConfig->rewardID, curAchievementConfig->rewardAmount, GoodsWay::achievement);
		break;
	}

	g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));
}

const SAchivementUnitData* CAchievementManager::AchievementCompleteProgress(const AchievementConfig* achievementConfig, OUT bool& achievementAccomplished, int completedTimes /* = 1 */)
{
	if (achievementConfig == nullptr)
		return nullptr;

	achievementAccomplished = false;
	const SAchivementUnitData* curAchievementData = m_achievementDataMgr.SetAchievementCompleteTimes(achievementConfig->id, achievementAccomplished, completedTimes);
	if (curAchievementData != nullptr)///�����Ѿ��ɹ����øóɾ��������ɴ������
	{
		if (curAchievementData->groupType != achievementConfig->achieveType)
		{
			rfalse("�ɾ�����������������");
			return nullptr;
		}

		m_ptrUpdateAchievementStateMsg->achievementID = curAchievementData->achievementID;
		m_ptrUpdateAchievementStateMsg->completedTimes = curAchievementData->completedTimes;
		g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrUpdateAchievementStateMsg.get(), sizeof(SAUpdateAchievementState));
	}

	return curAchievementData;
}

void CAchievementManager::ActiveNextAchievement(const SAchivementUnitData* curAchievementData)
{
	if (curAchievementData == nullptr)
		return;

	int endAchievementID = -1;
	int judgeAchievementID = curAchievementData->achievementID;
	const SAchivementUnitData* nextAchievementUnitData = nullptr;
	const AchievementConfig* nextAchievementConfig = nullptr;
	while (true)
	{
		int nextAchievemntID = 0;
		if (CConfigManager::getSingleton()->GetNextAchievementID(judgeAchievementID, nextAchievemntID))
		{
			///���Ϊ-1�������ǰ�ɾ��Ǹ���ɾ��е����һ���ɾ�
			if (nextAchievemntID != -1)
			{
				endAchievementID = nextAchievemntID;
				nextAchievementUnitData = m_achievementDataMgr.GetAchievementUnitData(nextAchievemntID);
				nextAchievementConfig = CConfigManager::getSingleton()->GetAchievementConfig(nextAchievemntID);
				if (nextAchievementUnitData == nullptr || nextAchievementConfig == nullptr)
				{
					rfalse("��ǰ�ɾ͵���һ���ɾ�%d��������ݲ�����", nextAchievemntID);
					return;
				}

				///�����佫���׳ɾ��С���������ɳɾ͵���������ڴ��ж��£���ȡ���ġ���һ�����ɾ��Ƿ��Ѿ���ɣ�����Ѿ�����������ȡ��һ��
				if (nextAchievementUnitData->accompulished || (nextAchievementUnitData->completedTimes != 0 && nextAchievementUnitData->completedTimes >= nextAchievementConfig->param2))
				{
					judgeAchievementID = nextAchievemntID;
					continue;
				}

				m_achievementDataMgr.SetAchievementGroupCurUnaccomplishedID((AchievementType)curAchievementData->groupType, nextAchievemntID);///�������һ��������ɵĳɾ�
				return;
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
}
