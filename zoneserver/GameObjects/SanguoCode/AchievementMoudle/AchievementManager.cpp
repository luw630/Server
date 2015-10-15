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
		rfalse("成就接收到的消息为空");
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
			rfalse("客户端发过来的领取成就的消息转换出错");
		break;
	}
}

void CAchievementManager::UpdateDuplicateAchieve(int curConquredTollgateID, int completedTimes /* = 1 */)
{
	///暂时前5个划为副本相关的成就
	for (int i = 0; i <= AchievementType::Battle4; ++i)
	{
		int updateAchievmentID = -1;
		updateAchievmentID = CConfigManager::getSingleton()->IsTollgateIDExistInSpecificAchievementGroup(i, curConquredTollgateID);
		if (updateAchievmentID == -1)
			continue;

		const SAchivementGroupData* groupData = m_achievementDataMgr.GetAchievementGroupData((AchievementType)i);
		if (groupData == nullptr)
			continue;

		////该组所有成就都完成的话就不做进一步的判断了
		if (groupData->accompulished)
			continue;

		int curAchievementUnaccomplishedID = m_achievementDataMgr.GetAchievemntGroupCurUnaccomplishedID((AchievementType)i);
		if (curAchievementUnaccomplishedID == -1)
			continue;

		///NOTE:此逻辑完全建立在策划配置的配置文件中的成就对应ID是一级一级递增的
		const AchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetAchievementConfig(updateAchievmentID);
		if (achievementConfig == nullptr || achievementConfig->param1 != curConquredTollgateID)
			continue;

		bool bAchievementAccomplished = false;
		const SAchivementUnitData* curAchievmentUnityData = AchievementCompleteProgress(achievementConfig, bAchievementAccomplished, completedTimes);
		///判断成就是否“达成”,达成后将该组别的成就的最新的待完成的成就ID往后移一位
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

	////该组所有成就都完成的话就不做进一步的判断了
	if (groupData->accompulished)
	{
		return;
	}

	int curAchievementUnaccomplishedID = m_achievementDataMgr.GetAchievemntGroupCurUnaccomplishedID(AchievementType::Collect);
	if (curAchievementUnaccomplishedID == -1)
	{
		return;
	}

	///NOTE:此逻辑完全建立在策划配置的配置文件中的成就对应ID是一级一级递增的
	const AchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetAchievementConfig(curAchievementUnaccomplishedID);
	if (achievementConfig == nullptr)
	{
		return;
	}

	bool bAchievementAccomplished = false;
	const SAchivementUnitData* curAchievmentUnityData = AchievementCompleteProgress(achievementConfig, bAchievementAccomplished);
	///判断成就是否“达成”,达成后将该组别的成就的最新的待完成的成就ID往后移一位
	if (bAchievementAccomplished)
		ActiveNextAchievement(curAchievmentUnityData);
}

void CAchievementManager:: UpdateHeroRankRiseAchieve(int curHeroRank)
{
	const SAchivementGroupData* groupData = m_achievementDataMgr.GetAchievementGroupData(AchievementType::Advanced);
	if (groupData == nullptr)
		return;

	////该组所有成就都完成的话就不做进一步的判断了
	if (groupData->accompulished)
		return;

	///英雄收集成就跟已有的成就不一样，可以“跳级”完成，所以处理逻辑也不一样
	int updateAchievementID = -1;
	const map<int32_t, vector<int32_t>>& heroRankUpgradeAchievementIDList = CConfigManager::getSingleton()->GetHeroRankUpgradeAchievementIDList();
	if (heroRankUpgradeAchievementIDList.size() == 0)
		return;

	///判断当前的品阶是否能达成任何成就
	auto findResult = heroRankUpgradeAchievementIDList.lower_bound(curHeroRank);
	if (findResult == heroRankUpgradeAchievementIDList.end())
	{
		--findResult;
	}
	else
	{
		if (findResult->first > curHeroRank)
		{
			///如果查出的品阶要大于当前品阶，则判断其前一品阶
			if (findResult != heroRankUpgradeAchievementIDList.begin())
				--findResult;
			else///当前的品阶还不能达成任何成就任务
				return;
		}
	}

	///获得要更新成就完成次数的那个成就ID, NOTE：严重依赖于策划配置文档中的成就完成顺序
	const SAchivementUnitData* achievementUnitData = nullptr;
	const AchievementConfig* achievementConfig = nullptr;
	for (auto itor : findResult->second)
	{
		/////为了防范策划配置文件中配的成就ID不是递增的情况, 先完成ID较小的成就
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

		///如果品阶为curHeroRank这个成就完成了，则转到该品阶的下一个成就
		if (achievementUnitData->accompulished || (achievementUnitData->completedTimes != 0 && achievementUnitData->completedTimes >= achievementConfig->param2))
		{
			achievementUnitData = nullptr;
			achievementConfig = nullptr;
			continue;
		}

		/// NOTE：严重依赖于策划配置文档中的成就完成顺序
		updateAchievementID = itor;
		break;
	}

	///如果当前品阶所对应的所有相关成就都完成了，则不继续往下走了
	if (updateAchievementID == -1)
		return;

	int curAchievementUnaccomplishedID = m_achievementDataMgr.GetAchievemntGroupCurUnaccomplishedID(AchievementType::Advanced);
	if (curAchievementUnaccomplishedID == -1)
		return;

	bool bAchievementAccomplished = false;
	const SAchivementUnitData* curAchievmentUnityData = AchievementCompleteProgress(achievementConfig, bAchievementAccomplished);
	///判断成就是否“达成”,达成后将该组别的成就的最新的待完成的成就ID往后移一位
	if (bAchievementAccomplished && curAchievementUnaccomplishedID == updateAchievementID)
		ActiveNextAchievement(curAchievmentUnityData);
}

void CAchievementManager::UpdateMasterLevelAchieve(int curMasterLevel, int levelUpgradeAmount)
{
	const SAchivementGroupData* groupData = m_achievementDataMgr.GetAchievementGroupData(AchievementType::Level);
	if (groupData == nullptr)
		return;

	////该组所有成就都完成的话就不做进一步的判断了
	if (groupData->accompulished)
		return;


	///处理跳级升级的情况
	int curLevelStep = 0;
	int curAchievementUnaccomplishedID = -1;
	int lastAchievementUnaccomplishedID = -1;
	while (curLevelStep < levelUpgradeAmount)
	{
		///获得
		curAchievementUnaccomplishedID = m_achievementDataMgr.GetAchievemntGroupCurUnaccomplishedID(AchievementType::Level);
		if (curAchievementUnaccomplishedID == -1)
		{
			rfalse("更新君主等级成就出错，无法获取当前待“达成”的成就");
			return;
		}

		///判断是否当前的组别的成就全部“达成”了，“达成”后才能领取
		if (lastAchievementUnaccomplishedID != -1 && lastAchievementUnaccomplishedID == curAchievementUnaccomplishedID)
			return;

		///NOTE:此逻辑完全建立在策划配置的配置文件中的成就对应ID是一级一级递增的
		const AchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetAchievementConfig(curAchievementUnaccomplishedID);
		if (achievementConfig == nullptr)
		{
			rfalse("更新君主等级成就出错，无法获得成就%d对应的配置信息", curAchievementUnaccomplishedID);
			return;
		}

		///判断当前的君主等级是否可以达成该成就
		if (achievementConfig->param1 > curMasterLevel)
			return;

		bool bAchievementAccomplished = false;
		const SAchivementUnitData* curAchievmentUnityData = AchievementCompleteProgress(achievementConfig, bAchievementAccomplished);
		///判断成就是否“达成”,达成后将该组别的成就的最新的待完成的成就ID往后移一位
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
	///如果转换失败 则不予领取
	if (ptrClaimMsg == nullptr)
	{
		g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));
		return;
	}

	///获取要领取的成就的所属的组别
	byte curAchievementType = (byte)CConfigManager::getSingleton()->GetAchievementGroupType(ptrClaimMsg->achievementID);
	if (curAchievementType == -1)
		g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));

	///获取要领取的成就的相关基础数据
	m_ptrAchievementClaimConformedMsg->achievementType = curAchievementType;
	const SAchivementGroupData* groupData = m_achievementDataMgr.GetAchievementGroupData((AchievementType)curAchievementType);
	const SAchivementUnitData* achievementUnitData = m_achievementDataMgr.GetAchievementUnitData(ptrClaimMsg->achievementID);
	const AchievementConfig* curAchievementConfig = CConfigManager::getSingleton()->GetAchievementConfig(ptrClaimMsg->achievementID);
	if (groupData == nullptr || achievementUnitData == nullptr || curAchievementConfig == nullptr)
	{
		g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));
		return;
	}

	////该组所有成就都完成的话就不做进一步的判断了
	if (groupData->accompulished)
	{
		g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));
		return;
	}
	
	///成就已经领取的话不予再一次领取
	if (achievementUnitData->accompulished)
	{
		if (groupData->newestAchivementID == achievementUnitData->achievementID)
		{
			ActiveNextAchievement(achievementUnitData);
			rfalse("成就更新最新的待完成成就ID失败，最新的待完成成就ID应该为%d", groupData->newestAchivementID);
		}
		g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));
		return;
	}

	///成就未达到可以领取的条件
	if (achievementUnitData->completedTimes == 0 || achievementUnitData->completedTimes < curAchievementConfig->param2)
	{
		g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));
		return;
	}

	///设置成就的领取状态
	m_ptrAchievementClaimConformedMsg->achievementID = ptrClaimMsg->achievementID;
	m_ptrAchievementClaimConformedMsg->bConformed = true;
	m_achievementDataMgr.SetAchievementAccompulished(ptrClaimMsg->achievementID);
	///判断是否该组成就全部完成了
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

	///开始领取相应的奖励
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
		///如果奖励的武将玩家已经拥有则转换成对应数量的将魂碎片
		if (baseDataManager.ExistHero(curAchievementConfig->rewardID))
		{
			const HeroStarConfig* heroStarConfig = CConfigManager::getSingleton()->GetHeroStarAttr(curAchievementConfig->rewardID);
			if (heroStarConfig == nullptr)
			{
				rfalse("找不到ID为 %d 的英雄的对应的星级配置", curAchievementConfig->rewardID);
				m_ptrAchievementClaimConformedMsg->bConformed = false;
				m_ptrAchievementClaimConformedMsg->bAchievementGroupAccomplished = false;
				g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));
				return;
			}
			const HeroConfig* heroConfig = CConfigManager::getSingleton()->GetHeroConfig(curAchievementConfig->rewardID);
			if (heroConfig == nullptr)
			{
				rfalse("无法找到ID为%d的英雄的配置", curAchievementConfig->rewardID);
				m_ptrAchievementClaimConformedMsg->bConformed = false;
				m_ptrAchievementClaimConformedMsg->bAchievementGroupAccomplished = false;
				g_StoreMessage(m_achievementDataMgr.GetBaseDataMgr().GetDNID(), m_ptrAchievementClaimConformedMsg.get(), sizeof(SAAchievementClaimConformed));
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
				///引导判定 以通知客户端前往下一步指引
				baseDataManager.ProcessOperationOfGuide(FunctionMoudleType::Function_Achievement, curAchievementConfig->rewardID);
			}
			else
			{
				rfalse("领取成就的时候读取的英雄配置文件里面的星级有误");
			}
		}
		else
		{
			baseDataManager.AddGoods_SG(GoodsType::hero, curAchievementConfig->rewardID, curAchievementConfig->rewardAmount, GoodsWay::achievement);
			///引导判定 以通知客户端前往下一步指引
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
	if (curAchievementData != nullptr)///代表已经成功设置该成就任务的完成次数完成
	{
		if (curAchievementData->groupType != achievementConfig->achieveType)
		{
			rfalse("成就所属组别的配置有误");
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
			///如果为-1，则代表当前成就是该组成就中的最后一个成就
			if (nextAchievemntID != -1)
			{
				endAchievementID = nextAchievemntID;
				nextAchievementUnitData = m_achievementDataMgr.GetAchievementUnitData(nextAchievemntID);
				nextAchievementConfig = CConfigManager::getSingleton()->GetAchievementConfig(nextAchievemntID);
				if (nextAchievementUnitData == nullptr || nextAchievementConfig == nullptr)
				{
					rfalse("当前成就的下一个成就%d的相关数据不存在", nextAchievemntID);
					return;
				}

				///由于武将进阶成就有“跳级”达成成就的情况，故在此判断下，获取到的“下一个”成就是否已经完成，如果已经完成则继续获取下一个
				if (nextAchievementUnitData->accompulished || (nextAchievementUnitData->completedTimes != 0 && nextAchievementUnitData->completedTimes >= nextAchievementConfig->param2))
				{
					judgeAchievementID = nextAchievemntID;
					continue;
				}

				m_achievementDataMgr.SetAchievementGroupCurUnaccomplishedID((AchievementType)curAchievementData->groupType, nextAchievemntID);///“激活”下一个可以完成的成就
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
