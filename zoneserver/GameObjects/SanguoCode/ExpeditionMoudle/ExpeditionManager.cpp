#include "stdafx.h"
#include "ExpeditionManager.h"
#include "ExpeditionDataManager.h"
#include "..\Common\ConfigManager.h"
#include "..\Common\PubTool.h"
#include "..\BaseDataManager.h"
#include "..\..\Random.h"
#include "../AchievementMoudle/RandomAchieveUpdate.h"
#include "..\MissionMoudle\CMissionUpdate.h"
#include "..\StorageMoudle\StorageManager.h"
#include "ScriptManager.h"
#include "Player.h"

extern LPIObject GetPlayerBySID(DWORD dwStaticID);

CExpeditionManager::CExpeditionManager()
	:globalConfig(CConfigManager::getSingleton()->globalConfig)
{
	m_ptrDataMgr = nullptr;
	m_excellentHeroInfor = nullptr;
	m_ptrCurLevelInstanceInfor = nullptr;
	m_iFirstExpeditionLevelID = CConfigManager::getSingleton()->GetFirstExpeditionInstanceID();
}


CExpeditionManager::~CExpeditionManager()
{
	m_ptrDataMgr = nullptr;
	m_excellentHeroInfor = nullptr;
	m_ptrCurLevelInstanceInfor = nullptr;
}

void CExpeditionManager::DispatchExpeditionMsg(const SMessage *pMsg, CExtendedDataManager *pExpeditionDataMgr)
{
	m_ptrDataMgr = nullptr;
	m_ptrDataMgr = dynamic_cast<CExpeditionDataManager*>(pExpeditionDataMgr);
	if (m_ptrDataMgr == nullptr)
	{
		rfalse("远征数据管理类或者发过来的跟远征相关的消息为NULL");
		return;
	}

	CBaseDataManager& baseDataMgr = m_ptrDataMgr->GetBaseDataMgr(); //获取基础数据管理类
	const SQExpeditionRequest *pQuestMsg = static_cast<const SQExpeditionRequest *>(pMsg);
	if (pQuestMsg == nullptr)
	{
		rfalse("远征消息转换出错");
		return;
	}

	switch (pQuestMsg->_protocol)
	{
	case SExpeditionMsg::REQUEST_BATTLE_RESULT_RESOLVES:
		_BattleResultResolves();
		break;
	case SExpeditionMsg::REQUEST_RECEIVE_REWARDS:
		_RewardsReceived();
		break;
	case SExpeditionMsg::REQUEST_ENTER_BATTLE:
		_CheckEnterBattlePermission(pMsg);
		break;
	case SExpeditionMsg::REQUEST_UPDATE_SELECTEDCHARACTER_INFOR:
		_SynSelectedHeroInfor(pMsg);
		break;
	case SExpeditionMsg::REQUEST_UPDATE_ENEMYCHARACTER_INFOR:
		_SynHostileEnmyInfor(pMsg);
		break;
	case SExpeditionMsg::REQUEST_COST_A_TICKET:
		_AskToCostATicket();
		break;
	case SExpeditionMsg::REQUEST_ACTIVE_EXPEDITION:
		_ActiveExpedition();
		break;
	case SExpeditionMsg::REQUEST_EXIT_BATTLE:
		_ExitBattle();
		break;
	default:
		rfalse("远征收到未知类型的消息");
		break;
	}
}

/// <summary>
/// 战斗胜利的结算
/// </summary>
void CExpeditionManager::_BattleResultResolves()
{
	if (!m_ptrDataMgr->GetPermissionState())
		return;

	///如果有敌人没有死则无法结算，因为可能有数据包的丢失，暂时不做这个判断
	//for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
	//{
	//	if (m_enemy[i].m_bDeadInExpedition != 0)
	//	{
	//		return;
	//	}
	//}
	
	CBaseDataManager& baseDataManager = m_ptrDataMgr->GetBaseDataMgr();

	///更新远征挑战的任务相关
	CMissionUpdate* ptrMissionUpdator = baseDataManager.GetMissionUpdator();
	if (ptrMissionUpdator != nullptr)
		ptrMissionUpdator->UpdateDungeionMission(InstanceType::Expedition, 1);

	///查看是否全军覆没
	if (m_ptrDataMgr->CheckCompletelyAnnihilated())
	{
		rfalse("远征关卡参战武将全军覆没，不予结算");
		return;
	}

	DWORD curInstanceID = 0;
	if (m_ptrDataMgr->GetCurExpeditionInstanceID(curInstanceID))
	{
		///获取当前远征关卡的详细配置信息
		m_ptrCurLevelInstanceInfor = CConfigManager::getSingleton()->GetSpecifyExpeditionInstanceConfig(baseDataManager.GetMasterLevel(), curInstanceID);
		if (m_ptrCurLevelInstanceInfor == nullptr)
		{
			rfalse("远征关卡%d的详细配置获取不到", curInstanceID);
			return;
		}
	}
	else
		return;

	///更新远征关卡的总的挑战次数
	_UpdateExpeditionTimes();
	///从远征配置文件中获取关卡掉落物品的数据与权重
	int greenItemRight = m_ptrCurLevelInstanceInfor->greenItemWeight + m_ptrCurLevelInstanceInfor->whiteItemWeight;
	int blueItemRight = greenItemRight + m_ptrCurLevelInstanceInfor->blueItemWeight;
	int purpleItemRight = blueItemRight + m_ptrCurLevelInstanceInfor->purpleItemWeight;
	int normalHeroRight = purpleItemRight + m_ptrCurLevelInstanceInfor->normalHeroWeight;
	int superiorHeroRight = normalHeroRight + m_ptrCurLevelInstanceInfor->superiorHeroWeight;
	int rareHeroRight = superiorHeroRight + m_ptrCurLevelInstanceInfor->rareHeroWeight;
	int curItemInterval = CRandom::RandRange(1, rareHeroRight);
	int itemIndex = 0;
	int itemcount = 0;

	///下面开始根据掉落物品、掉落英雄的权重来计算当前应该掉落什么东西
	if (curItemInterval <= m_ptrCurLevelInstanceInfor->whiteItemWeight)
	{
		itemcount = m_ptrCurLevelInstanceInfor->whiteItemList.size();
		if (itemcount > 0)
		{
			itemIndex = CRandom::RandRange(0, itemcount - 1);
			m_msgResolveInfo.obtainedItemID = m_ptrCurLevelInstanceInfor->whiteItemList[itemIndex];
			m_msgResolveInfo.bIsHero = false;
		}
	}
	else if (curItemInterval > m_ptrCurLevelInstanceInfor->whiteItemWeight && curItemInterval <= greenItemRight)
	{
		itemcount = m_ptrCurLevelInstanceInfor->greenItemList.size();
		if (itemcount > 0)
		{
			itemIndex = CRandom::RandRange(0, itemcount - 1);
			m_msgResolveInfo.obtainedItemID = m_ptrCurLevelInstanceInfor->greenItemList[itemIndex];
			m_msgResolveInfo.bIsHero = false;
		}
	}
	else if (curItemInterval > greenItemRight && curItemInterval <= blueItemRight)
	{
		itemcount = m_ptrCurLevelInstanceInfor->blueItemList.size();
		if (itemcount > 0)
		{
			itemIndex = CRandom::RandRange(0, itemcount - 1);
			m_msgResolveInfo.obtainedItemID = m_ptrCurLevelInstanceInfor->blueItemList[itemIndex];
			m_msgResolveInfo.bIsHero = false;
		}
	}
	else if (curItemInterval > blueItemRight && curItemInterval <= purpleItemRight)
	{
		itemcount = m_ptrCurLevelInstanceInfor->purpleItemList.size();
		if (itemcount > 0)
		{
			itemIndex = CRandom::RandRange(0, itemcount - 1);
			m_msgResolveInfo.obtainedItemID = m_ptrCurLevelInstanceInfor->purpleItemList[itemIndex];
			m_msgResolveInfo.bIsHero = false;
		}
	}
	else if (curItemInterval > purpleItemRight && curItemInterval <= normalHeroRight)
	{
		itemcount = m_ptrCurLevelInstanceInfor->normalHeroList.size();
		if (itemcount > 0)
		{
			itemIndex = CRandom::RandRange(0, itemcount - 1);
			m_msgResolveInfo.obtainedItemID = m_ptrCurLevelInstanceInfor->normalHeroList[itemIndex];
			m_msgResolveInfo.bIsHero = true;
		}
	}
	else if (curItemInterval > normalHeroRight && curItemInterval <= superiorHeroRight)
	{
		itemcount = m_ptrCurLevelInstanceInfor->superiorHeroList.size();
		if (itemcount > 0)
		{
			itemIndex = CRandom::RandRange(0, itemcount - 1);
			m_msgResolveInfo.obtainedItemID = m_ptrCurLevelInstanceInfor->superiorHeroList[itemIndex];
			m_msgResolveInfo.bIsHero = true;
		}
	}
	else if (curItemInterval > superiorHeroRight && curItemInterval <= rareHeroRight)
	{
		itemcount = m_ptrCurLevelInstanceInfor->rareHeroList.size();
		if (itemcount > 0)
		{
			itemIndex = CRandom::RandRange(0, itemcount - 1);
			m_msgResolveInfo.obtainedItemID = m_ptrCurLevelInstanceInfor->rareHeroList[itemIndex];
			m_msgResolveInfo.bIsHero = true;
		}
	}

	g_StoreMessage(baseDataManager.GetDNID(), &m_msgResolveInfo, sizeof(SABattleResolve));

	m_ptrDataMgr->SetRewardsUnclaimedState(true);
	m_ptrDataMgr->SetCurItemObtained(m_msgResolveInfo.obtainedItemID);
	m_ptrDataMgr->SetHeroRewardState(m_msgResolveInfo.bIsHero);

	m_ptrDataMgr->SetPermissionGotState(false);
}

///获取下一波敌人的数据
void CExpeditionManager::_GetNextEnmyInfor()
{
	if (m_ptrDataMgr->GetConquredState())
		return;

	CBaseDataManager& baseDataManager = m_ptrDataMgr->GetBaseDataMgr();

	///获取下一个远征关卡的ID
	int nextExpeditionInstanceID = -1;
	DWORD latestExpeditionInstanceID = 0;
	if (m_ptrDataMgr->GetCurExpeditionInstanceID(latestExpeditionInstanceID))
	{
		nextExpeditionInstanceID = CConfigManager::getSingleton()->GetNextExpeditionInstanceID(latestExpeditionInstanceID);
		if (nextExpeditionInstanceID != -1)
		{
			///往数据管理器中设置新的远征关卡ID并开始匹配当前玩家的敌人
			m_ptrDataMgr->SetCurExpeditionInstanceID(nextExpeditionInstanceID);
			///获取当前远征关卡的详细配置信息
			m_ptrCurLevelInstanceInfor = CConfigManager::getSingleton()->GetSpecifyExpeditionInstanceConfig(baseDataManager.GetMasterLevel(), nextExpeditionInstanceID);
			if (m_ptrCurLevelInstanceInfor == nullptr)
			{
				rfalse("远征关卡%d的详细配置获取不到", nextExpeditionInstanceID);
				return;
			}
			m_ptrDataMgr->SetEnemyMarchingState(true);
			_MatchEnemy();
		}
	}
}

///领取奖励
void CExpeditionManager::_RewardsReceived()
{
	if (m_ptrDataMgr->GetConquredState() || !m_ptrDataMgr->GetRewardsUnclaimedState())
		return;

	CBaseDataManager& baseDataManager = m_ptrDataMgr->GetBaseDataMgr();
	CStorageManager& storageMgr = baseDataManager.GetStorageManager();

	int nextExpeditionInstanceID = -1;
	DWORD latestExpeditionInstanceID = 0;
	if (m_ptrDataMgr->GetCurExpeditionInstanceID(latestExpeditionInstanceID))
	{
		///如果下一关已经没有了，则表示远征已经通关了
		nextExpeditionInstanceID = CConfigManager::getSingleton()->GetNextExpeditionInstanceID(latestExpeditionInstanceID);
		if (nextExpeditionInstanceID == -1)
		{
			g_StoreMessage(baseDataManager.GetDNID(), &m_msgExpeditionConqured, sizeof(SAConqured));
			m_ptrDataMgr->SetConquredState(true);
		}
	}
	else
	{
		rfalse("远征获取当前关卡ID出错");
		return;
	}

	///获取当前远征关卡的详细配置信息
	m_ptrCurLevelInstanceInfor = CConfigManager::getSingleton()->GetSpecifyExpeditionInstanceConfig(baseDataManager.GetMasterLevel(), latestExpeditionInstanceID);
	if (m_ptrCurLevelInstanceInfor == nullptr)
	{
		rfalse("远征关卡%d的详细配置获取不到", latestExpeditionInstanceID);
		return;
	}

	///开始结算，加钱、加经验等
	m_ptrDataMgr->CacheCurProceedsRiseState();
	baseDataManager.AddGoods_SG(GoodsType::money, 0, m_ptrDataMgr->GetCurExpectedMoneyProceeds(), GoodsWay::passCustoms);
	baseDataManager.AddGoods_SG(GoodsType::exploit, 0, m_ptrDataMgr->GetCurExpectedExploitProceeds(), GoodsWay::passCustoms);

	DWORD obtainedItemID = 0;
	if (!m_ptrDataMgr->GetCurItemObtained(obtainedItemID))
		return;

	if (m_ptrDataMgr->GetHeroRewardState())
	{
		///如果奖励的武将玩家已经拥有则转换成对应数量的将魂碎片
		if (baseDataManager.ExistHero(obtainedItemID))
		{
			const HeroStarConfig* heroStarConfig = CConfigManager::getSingleton()->GetHeroStarAttr(obtainedItemID);
			if (heroStarConfig == nullptr)
			{
				rfalse("找不到ID为 %d 的英雄的对应的星级配置", obtainedItemID);
				return;
			}
			const HeroConfig* heroConfig = CConfigManager::getSingleton()->GetHeroConfig(obtainedItemID);
			if (heroConfig == nullptr)
			{
				rfalse("无法找到ID为%d的英雄的配置", obtainedItemID);
				return;
			}

			///按照全局配置里面的信息来换算对应的将魂数量
			int heroStarLevel = heroConfig->Star;
			auto findResult = heroStarConfig->StarData.find(heroStarLevel);
			if (findResult != heroStarConfig->StarData.end())
			{
				int num = globalConfig.ThreeStarHeroTransform;
				switch (heroStarLevel)
				{
				case 1:
					num = globalConfig.OneStarHeroTransform;
					break;
				case 2:
					num = globalConfig.TwoStarHeroTransform;
					break;
				case 3:
					num = globalConfig.ThreeStarHeroTransform;
					break;
				}

				///往背包添加对应的武将碎片
				baseDataManager.AddGoods_SG(GoodsType::item, findResult->second.Item, num, GoodsWay::passCustoms);
			}
			else
			{
				rfalse("远征接收奖励的时候读取的英雄配置文件里面的星级有误");
			}
		}
		else
		{
			baseDataManager.AddGoods_SG(GoodsType::hero, obtainedItemID, GoodsWay::passCustoms);
		}
	}
	else
		///往背包添加对应的物品
		baseDataManager.AddGoods_SG(GoodsType::item, obtainedItemID, 1, GoodsWay::passCustoms);

	m_ptrDataMgr->SetRewardsUnclaimedState(false);

	if (!m_ptrDataMgr->GetConquredState())
		_GetNextEnmyInfor();
}

///获取进入关卡副本的通行证
void CExpeditionManager::_CheckEnterBattlePermission(const SMessage *pMsg)
{
	CBaseDataManager& baseDataMgr = m_ptrDataMgr->GetBaseDataMgr(); //获取基础数据管理类
	m_ptrDataMgr->SetPermissionGotState(false);
	m_msgPermissionInfor.checkResult = CheckResult::Failed;

	SQExpeditionPermissionRequest const* requestMsg = static_cast<const SQExpeditionPermissionRequest*>(pMsg);
	if (requestMsg == nullptr)
	{
		rfalse("SQExpeditionPermissionRequest为null");
		g_StoreMessage(baseDataMgr.GetDNID(), &m_msgPermissionInfor, sizeof(SAPermissionInfor));
		return;
	}
	
	///正在匹配英雄或者远征已经通关的时候不能进入场景
	if (m_ptrDataMgr->GetConquredState() || m_ptrDataMgr->GetEnemyMarchingState())
	{
		g_StoreMessage(baseDataMgr.GetDNID(), &m_msgPermissionInfor, sizeof(SAPermissionInfor));
		return;
	}

	///如果君主等级未达到能解锁远征的要求也不予进入场景，GM号没有武将的等级限制
	if (!baseDataMgr.GetGMFlag() && CConfigManager::getSingleton()->GetGameFeatureActivationConfig().expeditionDungeonLevelLimit > baseDataMgr.GetMasterLevel())
	{
		g_StoreMessage(baseDataMgr.GetDNID(), &m_msgPermissionInfor, sizeof(SAPermissionInfor));
		return;
	}

	///判断选中的武将是否不为0
	bool selectedHerosExist = false;
	for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
	{
		if (requestMsg->selectedHero[i] != 0)
		{
			selectedHerosExist = true;
			break;
		}
	}

	if (!selectedHerosExist)
	{
		g_StoreMessage(baseDataMgr.GetDNID(), &m_msgPermissionInfor, sizeof(SAPermissionInfor));
		return;
	}
	else
		m_ptrDataMgr->CacheSelectedHero(requestMsg->selectedHero);

	///判断选中的武将是否已经在远征关卡里面死亡了
	if (m_ptrDataMgr->CheckCompletelyAnnihilated())
	{
		rfalse("远征关卡参战武将全军覆没，不进入副本");
		g_StoreMessage(baseDataMgr.GetDNID(), &m_msgPermissionInfor, sizeof(SAPermissionInfor));
		return;
	}

	///判断选中的武将的等级是否满足要求
	int heroID;
	for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
	{
		heroID = requestMsg->selectedHero[i];
		if (heroID == 0)
			continue;

		const SHeroData * const heroData = baseDataMgr.GetHero(heroID);
		if (heroData == nullptr || heroData->m_dwHeroID == 0)
			continue;

		///GM号没有武将的等级限制
		if (!baseDataMgr.GetGMFlag() && heroData->m_dwLevel < globalConfig.CrusadeHeroLevelLimit)///如果选择的武将的等级不满足条件的不予进入副本
		{
			g_StoreMessage(baseDataMgr.GetDNID(), &m_msgPermissionInfor, sizeof(SAPermissionInfor));
			return;
		}
	}

	m_msgPermissionInfor.checkResult = CheckResult::Pass;
	m_ptrDataMgr->SetPermissionGotState(true);
	g_StoreMessage(baseDataMgr.GetDNID(), &m_msgPermissionInfor, sizeof(SAPermissionInfor));
	//wk 20150706 参与次数日志
	DWORD latestExpeditionInstanceID = 0;
	m_ptrDataMgr->GetCurExpeditionInstanceID(latestExpeditionInstanceID);
	g_Script.CallFunc("db_gm_setoperation", baseDataMgr.GetSID(), 3, 1, latestExpeditionInstanceID);
}

///匹配敌人
void CExpeditionManager::_MatchEnemy()
{
	CBaseDataManager& baseDataMgr = m_ptrDataMgr->GetBaseDataMgr(); //获取基础数据管理类
	m_excellentHeroInfor = baseDataMgr.GetExcellentHeroInfor();
	if (m_excellentHeroInfor == nullptr)
	{
		rfalse("远征最优秀的英雄信息获取不到");
		return;
	}
	///获取当前开放的各类型的英雄列表
	const vector<int32_t>& strengthHeroList = CConfigManager::getSingleton()->GetStrengthHeroList();
	const vector<int32_t>& manaHeroList = CConfigManager::getSingleton()->GetManaHeroList();
	const vector<int32_t>& agilityHeroList = CConfigManager::getSingleton()->GetAgilityHeroList();
	int avaliableStrengthHeroNum = strengthHeroList.size();
	int avaliableManaHeroNum = manaHeroList.size();
	int avaliableAgilityHeroNum = agilityHeroList.size();

	///下面随机5个英雄的类型（比如说力量英雄）,NOTE:此处g_iOneTypeEnmeyNumLimit严格小于等于3
	int strengthHeroNum = CRandom::RandRange(1, min(g_iOneTypeEnmeyNumLimit, avaliableStrengthHeroNum));
	int manaHeroNum = 0;
	if (g_iExpeditionCharacterLimit - strengthHeroNum > g_iOneTypeEnmeyNumLimit)
		manaHeroNum = CRandom::RandRange(1, min(g_iOneTypeEnmeyNumLimit, avaliableManaHeroNum));
	else
		manaHeroNum = CRandom::RandRange(1, (g_iExpeditionCharacterLimit - strengthHeroNum - 1));///至少要给敏捷英雄留一个
	int agilityHeroNum = g_iExpeditionCharacterLimit - strengthHeroNum - manaHeroNum;

	////////////////////////////////////////////////
	///下面开始从已开放的英雄冢随机选择某一种类型的英雄
	int listIndex = 0;
	///把strengthHeroList分成strengthHeroNum个区间
	int judgeStep = strengthHeroNum != 0 ? avaliableStrengthHeroNum / strengthHeroNum : 0;
	///生成敌人力量英雄
	for (int index = 0; index < strengthHeroNum; ++index)
	{
		///当前区间的起始位置，加上随机的在区间内的位置
		if (index < strengthHeroNum - 1)
			listIndex = index * judgeStep + CRandom::RandRange(0, judgeStep - 1);
		else
			listIndex = index * judgeStep + CRandom::RandRange(0, avaliableStrengthHeroNum - index * judgeStep - 1);
		_GenerateEnemy(m_excellentHeroInfor, strengthHeroList[listIndex], index);
	}
	///生成敌人智力英雄
	///把manaHeroList分成manaHeroNum个区间
	judgeStep = manaHeroNum != 0 ? avaliableManaHeroNum / manaHeroNum : 0;
	for (int index = 0; index < manaHeroNum; ++index)
	{
		///当前区间的起始位置，加上随机的在区间内的位置
		if (index < manaHeroNum - 1)
			listIndex = index * judgeStep + CRandom::RandRange(0, judgeStep - 1);
		else
			listIndex = index * judgeStep + CRandom::RandRange(0, avaliableManaHeroNum - index * judgeStep - 1);

		_GenerateEnemy(m_excellentHeroInfor, manaHeroList[listIndex], index + strengthHeroNum);
	}
	///生成敌人敏捷英雄
	///把agilityHeroList分成agilityHeroNum个区间
	judgeStep = agilityHeroNum != 0 ? avaliableAgilityHeroNum / agilityHeroNum : 0;
	for (int index = 0; index < agilityHeroNum; ++index)
	{
		///当前区间的起始位置，加上随机的在区间内的位置
		if (index < agilityHeroNum - 1)
			listIndex = index * judgeStep + CRandom::RandRange(0, judgeStep - 1);
		else
			listIndex = index * judgeStep + CRandom::RandRange(0, avaliableAgilityHeroNum - index * judgeStep - 1);

		_GenerateEnemy(m_excellentHeroInfor, agilityHeroList[listIndex], index + strengthHeroNum + manaHeroNum);
	}
	////////////////////////////////////////////////

	SHeroData* enemyData = m_ptrDataMgr->GetCurHostileCharacterInfor();
	m_ptrDataMgr->CacheMarchedEnmey();
	m_msgHostileEnmeyInfor.levelFactor = m_ptrCurLevelInstanceInfor->levelFactor;
	m_msgHostileEnmeyInfor.curLevelInstance = m_ptrCurLevelInstanceInfor->levelID;
	m_ptrDataMgr->SetCurLevelFactor(m_ptrCurLevelInstanceInfor->levelFactor);
	for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
	{
		m_msgHostileEnmeyInfor.enmeyInfo[i] = enemyData[i];
	}

	///往客户端发送匹配到的敌人信息
	g_StoreMessage(baseDataMgr.GetDNID(), &m_msgHostileEnmeyInfor, sizeof(SAHostileEnemyInfor));

	m_ptrDataMgr->SetEnemyMarchingState(false);
}

///生成地方的数据
void CExpeditionManager::_GenerateEnemy(const SHeroData* refHeroInfo, int enemyID, int enemySrialNumber)
{
	CBaseDataManager& baseDataMgr = m_ptrDataMgr->GetBaseDataMgr(); //获取基础数据管理类

	SHeroData* enemyData = m_ptrDataMgr->GetCurHostileCharacterInfor();

	if (enemyData == nullptr)
	{
		rfalse("远征的敌人信息为空");
		return;
	}

	enemyData[enemySrialNumber].m_dwHeroID = enemyID;
	///生成敌方玩家的星级、品阶信息
	const HeroConfig* initialHeroAttrConfig = nullptr;
	initialHeroAttrConfig = CConfigManager::getSingleton()->GetHeroConfig(enemyID);
	if (initialHeroAttrConfig == nullptr)
	{
		rfalse("英雄%d对应的配置无法找到", enemyID);
		return;
	}

	int refHeroMaxStarLevel = CConfigManager::getSingleton()->GetHeroMaxStarLevel(refHeroInfo->m_dwHeroID);
	int refHeroMaxRankLevel = CConfigManager::getSingleton()->GetHeroMaxRankGrade(refHeroInfo->m_dwHeroID);
	if (refHeroMaxStarLevel == -1 || refHeroMaxRankLevel == -1)
	{
		rfalse("英雄%d对应的最大星级无法找到", refHeroInfo->m_dwHeroID);
		return;
	}

	///做参考武将的星级的向下1级随机
	enemyData[enemySrialNumber].m_dwStarLevel = min(max(CRandom::RandRange(-1, 0) + refHeroInfo->m_dwStarLevel, initialHeroAttrConfig->Star), refHeroMaxStarLevel);
	///做参考武将的品阶的向下一级随机
	enemyData[enemySrialNumber].m_dwRankLevel = min(max(CRandom::RandRange(-1, 0) + refHeroInfo->m_dwRankLevel, initialHeroAttrConfig->Rank), refHeroMaxRankLevel);

	///随机生成敌方武将的等级 （最优英雄当前等级 - 7 + 关卡数）
	int enemyLevel = 0;
	enemyLevel = refHeroInfo->m_dwLevel - 7;
	enemyLevel = int(enemyLevel + (m_ptrCurLevelInstanceInfor->levelID - m_iFirstExpeditionLevelID + 1));
	enemyLevel += CRandom::RandRange(-2, 2);///在等级的上下±2浮动
	///GM号的等级不做限制
	if (baseDataMgr.GetGMFlag())
		enemyLevel = max(enemyLevel, 1);
	else
		enemyLevel = max(enemyLevel, CConfigManager::getSingleton()->globalConfig.CrusadeHeroLevelLimit);

	enemyLevel = min(enemyLevel, CConfigManager::getSingleton()->GetHeroMaxLevel());
	///下面生成敌方玩家的技能信息
	////////////////////////////////////////////////////////////////////////////////////////////////
	if (initialHeroAttrConfig->Skill1 > 0)
	{
		enemyData[enemySrialNumber].m_SkillInfoList[0].m_dwSkillID = initialHeroAttrConfig->Skill1;
		if (refHeroInfo->m_SkillInfoList[0].m_dwSkillLevel == 1)
		{
			enemyData[enemySrialNumber].m_SkillInfoList[0].m_dwSkillLevel = 1;
		}
		else
			enemyData[enemySrialNumber].m_SkillInfoList[0].m_dwSkillLevel = min(refHeroInfo->m_SkillInfoList[0].m_dwSkillLevel + CRandom::RandRange(-5, 5), enemyLevel);///做模板武将技能等级的±5随机,技能等级上限为武将等级
	}
	if (initialHeroAttrConfig->Skill2 > 0)
	{
		enemyData[enemySrialNumber].m_SkillInfoList[1].m_dwSkillID = initialHeroAttrConfig->Skill2;
		if (refHeroInfo->m_SkillInfoList[1].m_dwSkillLevel == 1)
			enemyData[enemySrialNumber].m_SkillInfoList[1].m_dwSkillLevel = 1;
		else
			enemyData[enemySrialNumber].m_SkillInfoList[1].m_dwSkillLevel = min(refHeroInfo->m_SkillInfoList[1].m_dwSkillLevel + CRandom::RandRange(-5, 5), enemyLevel);///做模板武将技能等级的±5随机,技能等级上限为武将等级
	}
	if (initialHeroAttrConfig->Skill3 > 0)
	{
		enemyData[enemySrialNumber].m_SkillInfoList[2].m_dwSkillID = initialHeroAttrConfig->Skill3;
		if (refHeroInfo->m_SkillInfoList[2].m_dwSkillLevel > 1)
			enemyData[enemySrialNumber].m_SkillInfoList[2].m_dwSkillLevel = min(refHeroInfo->m_SkillInfoList[2].m_dwSkillLevel + CRandom::RandRange(-5, 5), max(1, enemyLevel - 20));///做模板武将技能等级的±5随机,技能等级上限为武将等级 - 20
		else
			enemyData[enemySrialNumber].m_SkillInfoList[2].m_dwSkillLevel = 1;
	}
	if (initialHeroAttrConfig->Skill4 > 0)
	{
		enemyData[enemySrialNumber].m_SkillInfoList[3].m_dwSkillID = initialHeroAttrConfig->Skill4;
		if (refHeroInfo->m_SkillInfoList[3].m_dwSkillLevel > 1)
			enemyData[enemySrialNumber].m_SkillInfoList[3].m_dwSkillLevel = min(refHeroInfo->m_SkillInfoList[3].m_dwSkillLevel + CRandom::RandRange(-5, 5), max(1, enemyLevel - 40));///做模板武将技能等级的±5随机,技能等级上限为武将等级 - 40
		else
			enemyData[enemySrialNumber].m_SkillInfoList[3].m_dwSkillLevel = 1;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////

	enemyData[enemySrialNumber].m_dwLevel = enemyLevel;
	///生成怪物的血量、魔法值等等
	enemyData[enemySrialNumber].m_bDeadInExpedition = false;
	enemyData[enemySrialNumber].m_fExpeditionHealthScale = 1.0f;///血量一开始就满血
	enemyData[enemySrialNumber].m_fExpeditionManaScale = CConfigManager::getSingleton()->globalConfig.CrusadeInitMana / 1000.0f;
}

///玩家尚未打完就退出了游戏
void CExpeditionManager::_ExitBattle()
{
	m_ptrDataMgr->SetPermissionGotState(false);
}

///暂时由客户端发来决定是否激活远征副本
void CExpeditionManager::_ActiveExpedition()
{
	CBaseDataManager& baseDataMgr = m_ptrDataMgr->GetBaseDataMgr(); //获取基础数据管理类

	///GM号没有武将的等级限制,激活的话就不花费票数
	if (baseDataMgr.GetGMFlag() || (!baseDataMgr.GetGMFlag() && CConfigManager::getSingleton()->GetGameFeatureActivationConfig().expeditionDungeonLevelLimit <= baseDataMgr.GetMasterLevel()))
	{
		if (!m_ptrDataMgr->GetExpeditionActiveState())
		{
			///有票的话就重置一些数据，比如奖励是否领取的状态
			m_ptrDataMgr->SetConquredState(false);
			m_ptrDataMgr->SetRewardsUnclaimedState(false);
			baseDataMgr.ResetExpedtiionInfor();
			m_ptrDataMgr->ResetEnemyInfor();

			if (m_iFirstExpeditionLevelID != -1)
				m_ptrDataMgr->SetCurExpeditionInstanceID(m_iFirstExpeditionLevelID);
			///获取当前远征关卡的详细配置信息
			m_ptrCurLevelInstanceInfor = CConfigManager::getSingleton()->GetSpecifyExpeditionInstanceConfig(baseDataMgr.GetMasterLevel(), m_iFirstExpeditionLevelID);
			if (m_ptrCurLevelInstanceInfor == nullptr)
			{
				rfalse("远征关卡%d的详细配置获取不到", m_iFirstExpeditionLevelID);
				return;
			}
			m_ptrDataMgr->SetEnemyMarchingState(true);
			_MatchEnemy();
			m_ptrDataMgr->SetExpeditionActiveState(true);
		}
	}
}

void CExpeditionManager::_AskToCostATicket()
{
	CBaseDataManager& baseDataMgr = m_ptrDataMgr->GetBaseDataMgr(); //获取基础数据管理类

	///GM号没有武将的等级限制
	if (!baseDataMgr.GetGMFlag() && CConfigManager::getSingleton()->GetGameFeatureActivationConfig().expeditionDungeonLevelLimit > baseDataMgr.GetMasterLevel())
		return;

	DWORD ticketNum = 0;
	///先看看当前有没有票数（副本重置次数）
	if (m_ptrDataMgr->GetCurTicketsNum(ticketNum) && ticketNum >= 1)
	{
		///有票的话就重置一些数据，比如奖励是否领取的状态
		m_ptrDataMgr->SetConquredState(false);
		m_ptrDataMgr->SetRewardsUnclaimedState(false);
		baseDataMgr.ResetExpedtiionInfor();
		m_ptrDataMgr->ResetEnemyInfor();

		///修改数据，去除一张票，并将远征关卡的关卡ID置为第一关
		m_ptrDataMgr->CostATicket();
		if (m_iFirstExpeditionLevelID != -1)
			m_ptrDataMgr->SetCurExpeditionInstanceID(m_iFirstExpeditionLevelID);
		///获取当前远征关卡的详细配置信息
		m_ptrCurLevelInstanceInfor = CConfigManager::getSingleton()->GetSpecifyExpeditionInstanceConfig(baseDataMgr.GetMasterLevel(), m_iFirstExpeditionLevelID);
		if (m_ptrCurLevelInstanceInfor == nullptr)
		{
			rfalse("远征关卡%d的详细配置获取不到", m_iFirstExpeditionLevelID);
			return;
		}
		m_ptrDataMgr->SetEnemyMarchingState(true);
		_MatchEnemy();

	}
}


void CExpeditionManager::_SynSelectedHeroInfor(const SMessage *pMsg)
{
	const SQUpdateSelectedHeroData* updateMsg = static_cast <const SQUpdateSelectedHeroData*>(pMsg);
	if (updateMsg == nullptr)
	{
		rfalse("远征同步玩家参战武将血量出错");
		return;
	}

	CBaseDataManager& baseDataManager = m_ptrDataMgr->GetBaseDataMgr();
	const SHeroData* heroData = nullptr;
	///先检查同步的武将是否跟记录的武将一致，不一致记录的武将全部判定为死亡？（惩罚性措施）
	for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
	{
		if (updateMsg->selectedHeroMap[i] == 0)
			continue;

		if (!m_ptrDataMgr->IsHeroSelected(updateMsg->selectedHeroMap[i]))
		{
			rfalse("远征结算的英雄ID跟索要通行证的时候传进来的ID不同，有作弊嫌疑");
			///todo 作弊嫌疑记录
		}
		else
		{
			baseDataManager.SetExpeditionInfor(updateMsg->selectedHeroMap[i], updateMsg->heroHealthScale[i], updateMsg->heroManaScale[i]);
		}
	}
}

void CExpeditionManager::_SynHostileEnmyInfor(const SMessage *pMsg)
{
	const SQUpdateEnemyData* updateMsg = static_cast <const SQUpdateEnemyData*>(pMsg);
	if (updateMsg == nullptr)
	{
		rfalse("远征同步敌方参战武将血量出错");
		return;
	}

	CBaseDataManager& baseDataManager = m_ptrDataMgr->GetBaseDataMgr();
	const SHeroData* heroData = nullptr;
	///先检查同步的武将是否跟记录的武将一致？（惩罚性措施）
	for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
	{
		if (updateMsg->machedEnemyMap[i] == 0)
			continue;

		if (!m_ptrDataMgr->IsEnemyExist(updateMsg->machedEnemyMap[i]))
		{
			rfalse("远征结算的敌人的英雄ID跟系统的ID不同，有作弊嫌疑");
			///todo 作弊嫌疑记录
		}
		else
		{
			m_ptrDataMgr->SetEnemyInfor(updateMsg->machedEnemyMap[i], updateMsg->enemyHealthScale[i], updateMsg->enemyManaScale[i]);
		}
	}
}

void CExpeditionManager::_UpdateExpeditionTimes()
{
	CBaseDataManager& baseDataMgr = m_ptrDataMgr->GetBaseDataMgr(); //获取基础数据管理类

	int playerSID = baseDataMgr.GetSID();
	BOOL execResult = FALSE;
	int retValue = 0;
	lite::Variant ret;//从lua获取到的返回值
	if (g_Script.PrepareFunction("get_Exdata"))
	{
		g_Script.PushParameter(playerSID);
		g_Script.PushParameter(SG_ExDataType::ExpeditionTimes);
		execResult = g_Script.Execute(&ret);
	}
	else
		return;

	if (!execResult || ret.dataType == LUA_TNIL)
	{
		rfalse(2, 1, "ConsumeFinalDamageValue Faile");
		return;
	}

	try
	{
		retValue = (int)(ret);
	}
	catch (lite::Xcpt &e)
	{
		rfalse(2, 1, e.GetErrInfo());
		return;
	}

	if (g_Script.PrepareFunction("set_Exdata"))
	{
		g_Script.PushParameter(playerSID);
		g_Script.PushParameter(SG_ExDataType::ExpeditionTimes);
		g_Script.PushParameter(retValue + 1);
		g_Script.Execute();
	}

	///更新过关斩将榜
	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(playerSID)->DynamicCast(IID_PLAYER);
	if (pPlayer != nullptr)
	{
		g_Script.SetCondition(nullptr, pPlayer, nullptr);
		if (g_Script.PrepareFunction("rank_update"))
		{
			g_Script.PushParameter(playerSID);
			g_Script.PushParameter(RankType_SG::Overcome);
			g_Script.PushParameter(retValue + 1);
			g_Script.Execute();
		}
	}
}
