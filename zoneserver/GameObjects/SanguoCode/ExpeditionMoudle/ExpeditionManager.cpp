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
		rfalse("Զ�����ݹ�������߷������ĸ�Զ����ص���ϢΪNULL");
		return;
	}

	CBaseDataManager& baseDataMgr = m_ptrDataMgr->GetBaseDataMgr(); //��ȡ�������ݹ�����
	const SQExpeditionRequest *pQuestMsg = static_cast<const SQExpeditionRequest *>(pMsg);
	if (pQuestMsg == nullptr)
	{
		rfalse("Զ����Ϣת������");
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
		rfalse("Զ���յ�δ֪���͵���Ϣ");
		break;
	}
}

/// <summary>
/// ս��ʤ���Ľ���
/// </summary>
void CExpeditionManager::_BattleResultResolves()
{
	if (!m_ptrDataMgr->GetPermissionState())
		return;

	///����е���û�������޷����㣬��Ϊ���������ݰ��Ķ�ʧ����ʱ��������ж�
	//for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
	//{
	//	if (m_enemy[i].m_bDeadInExpedition != 0)
	//	{
	//		return;
	//	}
	//}
	
	CBaseDataManager& baseDataManager = m_ptrDataMgr->GetBaseDataMgr();

	///����Զ����ս���������
	CMissionUpdate* ptrMissionUpdator = baseDataManager.GetMissionUpdator();
	if (ptrMissionUpdator != nullptr)
		ptrMissionUpdator->UpdateDungeionMission(InstanceType::Expedition, 1);

	///�鿴�Ƿ�ȫ����û
	if (m_ptrDataMgr->CheckCompletelyAnnihilated())
	{
		rfalse("Զ���ؿ���ս�佫ȫ����û���������");
		return;
	}

	DWORD curInstanceID = 0;
	if (m_ptrDataMgr->GetCurExpeditionInstanceID(curInstanceID))
	{
		///��ȡ��ǰԶ���ؿ�����ϸ������Ϣ
		m_ptrCurLevelInstanceInfor = CConfigManager::getSingleton()->GetSpecifyExpeditionInstanceConfig(baseDataManager.GetMasterLevel(), curInstanceID);
		if (m_ptrCurLevelInstanceInfor == nullptr)
		{
			rfalse("Զ���ؿ�%d����ϸ���û�ȡ����", curInstanceID);
			return;
		}
	}
	else
		return;

	///����Զ���ؿ����ܵ���ս����
	_UpdateExpeditionTimes();
	///��Զ�������ļ��л�ȡ�ؿ�������Ʒ��������Ȩ��
	int greenItemRight = m_ptrCurLevelInstanceInfor->greenItemWeight + m_ptrCurLevelInstanceInfor->whiteItemWeight;
	int blueItemRight = greenItemRight + m_ptrCurLevelInstanceInfor->blueItemWeight;
	int purpleItemRight = blueItemRight + m_ptrCurLevelInstanceInfor->purpleItemWeight;
	int normalHeroRight = purpleItemRight + m_ptrCurLevelInstanceInfor->normalHeroWeight;
	int superiorHeroRight = normalHeroRight + m_ptrCurLevelInstanceInfor->superiorHeroWeight;
	int rareHeroRight = superiorHeroRight + m_ptrCurLevelInstanceInfor->rareHeroWeight;
	int curItemInterval = CRandom::RandRange(1, rareHeroRight);
	int itemIndex = 0;
	int itemcount = 0;

	///���濪ʼ���ݵ�����Ʒ������Ӣ�۵�Ȩ�������㵱ǰӦ�õ���ʲô����
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

///��ȡ��һ�����˵�����
void CExpeditionManager::_GetNextEnmyInfor()
{
	if (m_ptrDataMgr->GetConquredState())
		return;

	CBaseDataManager& baseDataManager = m_ptrDataMgr->GetBaseDataMgr();

	///��ȡ��һ��Զ���ؿ���ID
	int nextExpeditionInstanceID = -1;
	DWORD latestExpeditionInstanceID = 0;
	if (m_ptrDataMgr->GetCurExpeditionInstanceID(latestExpeditionInstanceID))
	{
		nextExpeditionInstanceID = CConfigManager::getSingleton()->GetNextExpeditionInstanceID(latestExpeditionInstanceID);
		if (nextExpeditionInstanceID != -1)
		{
			///�����ݹ������������µ�Զ���ؿ�ID����ʼƥ�䵱ǰ��ҵĵ���
			m_ptrDataMgr->SetCurExpeditionInstanceID(nextExpeditionInstanceID);
			///��ȡ��ǰԶ���ؿ�����ϸ������Ϣ
			m_ptrCurLevelInstanceInfor = CConfigManager::getSingleton()->GetSpecifyExpeditionInstanceConfig(baseDataManager.GetMasterLevel(), nextExpeditionInstanceID);
			if (m_ptrCurLevelInstanceInfor == nullptr)
			{
				rfalse("Զ���ؿ�%d����ϸ���û�ȡ����", nextExpeditionInstanceID);
				return;
			}
			m_ptrDataMgr->SetEnemyMarchingState(true);
			_MatchEnemy();
		}
	}
}

///��ȡ����
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
		///�����һ���Ѿ�û���ˣ����ʾԶ���Ѿ�ͨ����
		nextExpeditionInstanceID = CConfigManager::getSingleton()->GetNextExpeditionInstanceID(latestExpeditionInstanceID);
		if (nextExpeditionInstanceID == -1)
		{
			g_StoreMessage(baseDataManager.GetDNID(), &m_msgExpeditionConqured, sizeof(SAConqured));
			m_ptrDataMgr->SetConquredState(true);
		}
	}
	else
	{
		rfalse("Զ����ȡ��ǰ�ؿ�ID����");
		return;
	}

	///��ȡ��ǰԶ���ؿ�����ϸ������Ϣ
	m_ptrCurLevelInstanceInfor = CConfigManager::getSingleton()->GetSpecifyExpeditionInstanceConfig(baseDataManager.GetMasterLevel(), latestExpeditionInstanceID);
	if (m_ptrCurLevelInstanceInfor == nullptr)
	{
		rfalse("Զ���ؿ�%d����ϸ���û�ȡ����", latestExpeditionInstanceID);
		return;
	}

	///��ʼ���㣬��Ǯ���Ӿ����
	m_ptrDataMgr->CacheCurProceedsRiseState();
	baseDataManager.AddGoods_SG(GoodsType::money, 0, m_ptrDataMgr->GetCurExpectedMoneyProceeds(), GoodsWay::passCustoms);
	baseDataManager.AddGoods_SG(GoodsType::exploit, 0, m_ptrDataMgr->GetCurExpectedExploitProceeds(), GoodsWay::passCustoms);

	DWORD obtainedItemID = 0;
	if (!m_ptrDataMgr->GetCurItemObtained(obtainedItemID))
		return;

	if (m_ptrDataMgr->GetHeroRewardState())
	{
		///����������佫����Ѿ�ӵ����ת���ɶ�Ӧ�����Ľ�����Ƭ
		if (baseDataManager.ExistHero(obtainedItemID))
		{
			const HeroStarConfig* heroStarConfig = CConfigManager::getSingleton()->GetHeroStarAttr(obtainedItemID);
			if (heroStarConfig == nullptr)
			{
				rfalse("�Ҳ���IDΪ %d ��Ӣ�۵Ķ�Ӧ���Ǽ�����", obtainedItemID);
				return;
			}
			const HeroConfig* heroConfig = CConfigManager::getSingleton()->GetHeroConfig(obtainedItemID);
			if (heroConfig == nullptr)
			{
				rfalse("�޷��ҵ�IDΪ%d��Ӣ�۵�����", obtainedItemID);
				return;
			}

			///����ȫ�������������Ϣ�������Ӧ�Ľ�������
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

				///��������Ӷ�Ӧ���佫��Ƭ
				baseDataManager.AddGoods_SG(GoodsType::item, findResult->second.Item, num, GoodsWay::passCustoms);
			}
			else
			{
				rfalse("Զ�����ս�����ʱ���ȡ��Ӣ�������ļ�������Ǽ�����");
			}
		}
		else
		{
			baseDataManager.AddGoods_SG(GoodsType::hero, obtainedItemID, GoodsWay::passCustoms);
		}
	}
	else
		///��������Ӷ�Ӧ����Ʒ
		baseDataManager.AddGoods_SG(GoodsType::item, obtainedItemID, 1, GoodsWay::passCustoms);

	m_ptrDataMgr->SetRewardsUnclaimedState(false);

	if (!m_ptrDataMgr->GetConquredState())
		_GetNextEnmyInfor();
}

///��ȡ����ؿ�������ͨ��֤
void CExpeditionManager::_CheckEnterBattlePermission(const SMessage *pMsg)
{
	CBaseDataManager& baseDataMgr = m_ptrDataMgr->GetBaseDataMgr(); //��ȡ�������ݹ�����
	m_ptrDataMgr->SetPermissionGotState(false);
	m_msgPermissionInfor.checkResult = CheckResult::Failed;

	SQExpeditionPermissionRequest const* requestMsg = static_cast<const SQExpeditionPermissionRequest*>(pMsg);
	if (requestMsg == nullptr)
	{
		rfalse("SQExpeditionPermissionRequestΪnull");
		g_StoreMessage(baseDataMgr.GetDNID(), &m_msgPermissionInfor, sizeof(SAPermissionInfor));
		return;
	}
	
	///����ƥ��Ӣ�ۻ���Զ���Ѿ�ͨ�ص�ʱ���ܽ��볡��
	if (m_ptrDataMgr->GetConquredState() || m_ptrDataMgr->GetEnemyMarchingState())
	{
		g_StoreMessage(baseDataMgr.GetDNID(), &m_msgPermissionInfor, sizeof(SAPermissionInfor));
		return;
	}

	///��������ȼ�δ�ﵽ�ܽ���Զ����Ҫ��Ҳ������볡����GM��û���佫�ĵȼ�����
	if (!baseDataMgr.GetGMFlag() && CConfigManager::getSingleton()->GetGameFeatureActivationConfig().expeditionDungeonLevelLimit > baseDataMgr.GetMasterLevel())
	{
		g_StoreMessage(baseDataMgr.GetDNID(), &m_msgPermissionInfor, sizeof(SAPermissionInfor));
		return;
	}

	///�ж�ѡ�е��佫�Ƿ�Ϊ0
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

	///�ж�ѡ�е��佫�Ƿ��Ѿ���Զ���ؿ�����������
	if (m_ptrDataMgr->CheckCompletelyAnnihilated())
	{
		rfalse("Զ���ؿ���ս�佫ȫ����û�������븱��");
		g_StoreMessage(baseDataMgr.GetDNID(), &m_msgPermissionInfor, sizeof(SAPermissionInfor));
		return;
	}

	///�ж�ѡ�е��佫�ĵȼ��Ƿ�����Ҫ��
	int heroID;
	for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
	{
		heroID = requestMsg->selectedHero[i];
		if (heroID == 0)
			continue;

		const SHeroData * const heroData = baseDataMgr.GetHero(heroID);
		if (heroData == nullptr || heroData->m_dwHeroID == 0)
			continue;

		///GM��û���佫�ĵȼ�����
		if (!baseDataMgr.GetGMFlag() && heroData->m_dwLevel < globalConfig.CrusadeHeroLevelLimit)///���ѡ����佫�ĵȼ������������Ĳ�����븱��
		{
			g_StoreMessage(baseDataMgr.GetDNID(), &m_msgPermissionInfor, sizeof(SAPermissionInfor));
			return;
		}
	}

	m_msgPermissionInfor.checkResult = CheckResult::Pass;
	m_ptrDataMgr->SetPermissionGotState(true);
	g_StoreMessage(baseDataMgr.GetDNID(), &m_msgPermissionInfor, sizeof(SAPermissionInfor));
	//wk 20150706 ���������־
	DWORD latestExpeditionInstanceID = 0;
	m_ptrDataMgr->GetCurExpeditionInstanceID(latestExpeditionInstanceID);
	g_Script.CallFunc("db_gm_setoperation", baseDataMgr.GetSID(), 3, 1, latestExpeditionInstanceID);
}

///ƥ�����
void CExpeditionManager::_MatchEnemy()
{
	CBaseDataManager& baseDataMgr = m_ptrDataMgr->GetBaseDataMgr(); //��ȡ�������ݹ�����
	m_excellentHeroInfor = baseDataMgr.GetExcellentHeroInfor();
	if (m_excellentHeroInfor == nullptr)
	{
		rfalse("Զ���������Ӣ����Ϣ��ȡ����");
		return;
	}
	///��ȡ��ǰ���ŵĸ����͵�Ӣ���б�
	const vector<int32_t>& strengthHeroList = CConfigManager::getSingleton()->GetStrengthHeroList();
	const vector<int32_t>& manaHeroList = CConfigManager::getSingleton()->GetManaHeroList();
	const vector<int32_t>& agilityHeroList = CConfigManager::getSingleton()->GetAgilityHeroList();
	int avaliableStrengthHeroNum = strengthHeroList.size();
	int avaliableManaHeroNum = manaHeroList.size();
	int avaliableAgilityHeroNum = agilityHeroList.size();

	///�������5��Ӣ�۵����ͣ�����˵����Ӣ�ۣ�,NOTE:�˴�g_iOneTypeEnmeyNumLimit�ϸ�С�ڵ���3
	int strengthHeroNum = CRandom::RandRange(1, min(g_iOneTypeEnmeyNumLimit, avaliableStrengthHeroNum));
	int manaHeroNum = 0;
	if (g_iExpeditionCharacterLimit - strengthHeroNum > g_iOneTypeEnmeyNumLimit)
		manaHeroNum = CRandom::RandRange(1, min(g_iOneTypeEnmeyNumLimit, avaliableManaHeroNum));
	else
		manaHeroNum = CRandom::RandRange(1, (g_iExpeditionCharacterLimit - strengthHeroNum - 1));///����Ҫ������Ӣ����һ��
	int agilityHeroNum = g_iExpeditionCharacterLimit - strengthHeroNum - manaHeroNum;

	////////////////////////////////////////////////
	///���濪ʼ���ѿ��ŵ�Ӣ��ڣ���ѡ��ĳһ�����͵�Ӣ��
	int listIndex = 0;
	///��strengthHeroList�ֳ�strengthHeroNum������
	int judgeStep = strengthHeroNum != 0 ? avaliableStrengthHeroNum / strengthHeroNum : 0;
	///���ɵ�������Ӣ��
	for (int index = 0; index < strengthHeroNum; ++index)
	{
		///��ǰ�������ʼλ�ã�����������������ڵ�λ��
		if (index < strengthHeroNum - 1)
			listIndex = index * judgeStep + CRandom::RandRange(0, judgeStep - 1);
		else
			listIndex = index * judgeStep + CRandom::RandRange(0, avaliableStrengthHeroNum - index * judgeStep - 1);
		_GenerateEnemy(m_excellentHeroInfor, strengthHeroList[listIndex], index);
	}
	///���ɵ�������Ӣ��
	///��manaHeroList�ֳ�manaHeroNum������
	judgeStep = manaHeroNum != 0 ? avaliableManaHeroNum / manaHeroNum : 0;
	for (int index = 0; index < manaHeroNum; ++index)
	{
		///��ǰ�������ʼλ�ã�����������������ڵ�λ��
		if (index < manaHeroNum - 1)
			listIndex = index * judgeStep + CRandom::RandRange(0, judgeStep - 1);
		else
			listIndex = index * judgeStep + CRandom::RandRange(0, avaliableManaHeroNum - index * judgeStep - 1);

		_GenerateEnemy(m_excellentHeroInfor, manaHeroList[listIndex], index + strengthHeroNum);
	}
	///���ɵ�������Ӣ��
	///��agilityHeroList�ֳ�agilityHeroNum������
	judgeStep = agilityHeroNum != 0 ? avaliableAgilityHeroNum / agilityHeroNum : 0;
	for (int index = 0; index < agilityHeroNum; ++index)
	{
		///��ǰ�������ʼλ�ã�����������������ڵ�λ��
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

	///���ͻ��˷���ƥ�䵽�ĵ�����Ϣ
	g_StoreMessage(baseDataMgr.GetDNID(), &m_msgHostileEnmeyInfor, sizeof(SAHostileEnemyInfor));

	m_ptrDataMgr->SetEnemyMarchingState(false);
}

///���ɵط�������
void CExpeditionManager::_GenerateEnemy(const SHeroData* refHeroInfo, int enemyID, int enemySrialNumber)
{
	CBaseDataManager& baseDataMgr = m_ptrDataMgr->GetBaseDataMgr(); //��ȡ�������ݹ�����

	SHeroData* enemyData = m_ptrDataMgr->GetCurHostileCharacterInfor();

	if (enemyData == nullptr)
	{
		rfalse("Զ���ĵ�����ϢΪ��");
		return;
	}

	enemyData[enemySrialNumber].m_dwHeroID = enemyID;
	///���ɵз���ҵ��Ǽ���Ʒ����Ϣ
	const HeroConfig* initialHeroAttrConfig = nullptr;
	initialHeroAttrConfig = CConfigManager::getSingleton()->GetHeroConfig(enemyID);
	if (initialHeroAttrConfig == nullptr)
	{
		rfalse("Ӣ��%d��Ӧ�������޷��ҵ�", enemyID);
		return;
	}

	int refHeroMaxStarLevel = CConfigManager::getSingleton()->GetHeroMaxStarLevel(refHeroInfo->m_dwHeroID);
	int refHeroMaxRankLevel = CConfigManager::getSingleton()->GetHeroMaxRankGrade(refHeroInfo->m_dwHeroID);
	if (refHeroMaxStarLevel == -1 || refHeroMaxRankLevel == -1)
	{
		rfalse("Ӣ��%d��Ӧ������Ǽ��޷��ҵ�", refHeroInfo->m_dwHeroID);
		return;
	}

	///���ο��佫���Ǽ�������1�����
	enemyData[enemySrialNumber].m_dwStarLevel = min(max(CRandom::RandRange(-1, 0) + refHeroInfo->m_dwStarLevel, initialHeroAttrConfig->Star), refHeroMaxStarLevel);
	///���ο��佫��Ʒ�׵�����һ�����
	enemyData[enemySrialNumber].m_dwRankLevel = min(max(CRandom::RandRange(-1, 0) + refHeroInfo->m_dwRankLevel, initialHeroAttrConfig->Rank), refHeroMaxRankLevel);

	///������ɵз��佫�ĵȼ� ������Ӣ�۵�ǰ�ȼ� - 7 + �ؿ�����
	int enemyLevel = 0;
	enemyLevel = refHeroInfo->m_dwLevel - 7;
	enemyLevel = int(enemyLevel + (m_ptrCurLevelInstanceInfor->levelID - m_iFirstExpeditionLevelID + 1));
	enemyLevel += CRandom::RandRange(-2, 2);///�ڵȼ������¡�2����
	///GM�ŵĵȼ���������
	if (baseDataMgr.GetGMFlag())
		enemyLevel = max(enemyLevel, 1);
	else
		enemyLevel = max(enemyLevel, CConfigManager::getSingleton()->globalConfig.CrusadeHeroLevelLimit);

	enemyLevel = min(enemyLevel, CConfigManager::getSingleton()->GetHeroMaxLevel());
	///�������ɵз���ҵļ�����Ϣ
	////////////////////////////////////////////////////////////////////////////////////////////////
	if (initialHeroAttrConfig->Skill1 > 0)
	{
		enemyData[enemySrialNumber].m_SkillInfoList[0].m_dwSkillID = initialHeroAttrConfig->Skill1;
		if (refHeroInfo->m_SkillInfoList[0].m_dwSkillLevel == 1)
		{
			enemyData[enemySrialNumber].m_SkillInfoList[0].m_dwSkillLevel = 1;
		}
		else
			enemyData[enemySrialNumber].m_SkillInfoList[0].m_dwSkillLevel = min(refHeroInfo->m_SkillInfoList[0].m_dwSkillLevel + CRandom::RandRange(-5, 5), enemyLevel);///��ģ���佫���ܵȼ��ġ�5���,���ܵȼ�����Ϊ�佫�ȼ�
	}
	if (initialHeroAttrConfig->Skill2 > 0)
	{
		enemyData[enemySrialNumber].m_SkillInfoList[1].m_dwSkillID = initialHeroAttrConfig->Skill2;
		if (refHeroInfo->m_SkillInfoList[1].m_dwSkillLevel == 1)
			enemyData[enemySrialNumber].m_SkillInfoList[1].m_dwSkillLevel = 1;
		else
			enemyData[enemySrialNumber].m_SkillInfoList[1].m_dwSkillLevel = min(refHeroInfo->m_SkillInfoList[1].m_dwSkillLevel + CRandom::RandRange(-5, 5), enemyLevel);///��ģ���佫���ܵȼ��ġ�5���,���ܵȼ�����Ϊ�佫�ȼ�
	}
	if (initialHeroAttrConfig->Skill3 > 0)
	{
		enemyData[enemySrialNumber].m_SkillInfoList[2].m_dwSkillID = initialHeroAttrConfig->Skill3;
		if (refHeroInfo->m_SkillInfoList[2].m_dwSkillLevel > 1)
			enemyData[enemySrialNumber].m_SkillInfoList[2].m_dwSkillLevel = min(refHeroInfo->m_SkillInfoList[2].m_dwSkillLevel + CRandom::RandRange(-5, 5), max(1, enemyLevel - 20));///��ģ���佫���ܵȼ��ġ�5���,���ܵȼ�����Ϊ�佫�ȼ� - 20
		else
			enemyData[enemySrialNumber].m_SkillInfoList[2].m_dwSkillLevel = 1;
	}
	if (initialHeroAttrConfig->Skill4 > 0)
	{
		enemyData[enemySrialNumber].m_SkillInfoList[3].m_dwSkillID = initialHeroAttrConfig->Skill4;
		if (refHeroInfo->m_SkillInfoList[3].m_dwSkillLevel > 1)
			enemyData[enemySrialNumber].m_SkillInfoList[3].m_dwSkillLevel = min(refHeroInfo->m_SkillInfoList[3].m_dwSkillLevel + CRandom::RandRange(-5, 5), max(1, enemyLevel - 40));///��ģ���佫���ܵȼ��ġ�5���,���ܵȼ�����Ϊ�佫�ȼ� - 40
		else
			enemyData[enemySrialNumber].m_SkillInfoList[3].m_dwSkillLevel = 1;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////

	enemyData[enemySrialNumber].m_dwLevel = enemyLevel;
	///���ɹ����Ѫ����ħ��ֵ�ȵ�
	enemyData[enemySrialNumber].m_bDeadInExpedition = false;
	enemyData[enemySrialNumber].m_fExpeditionHealthScale = 1.0f;///Ѫ��һ��ʼ����Ѫ
	enemyData[enemySrialNumber].m_fExpeditionManaScale = CConfigManager::getSingleton()->globalConfig.CrusadeInitMana / 1000.0f;
}

///�����δ������˳�����Ϸ
void CExpeditionManager::_ExitBattle()
{
	m_ptrDataMgr->SetPermissionGotState(false);
}

///��ʱ�ɿͻ��˷��������Ƿ񼤻�Զ������
void CExpeditionManager::_ActiveExpedition()
{
	CBaseDataManager& baseDataMgr = m_ptrDataMgr->GetBaseDataMgr(); //��ȡ�������ݹ�����

	///GM��û���佫�ĵȼ�����,����Ļ��Ͳ�����Ʊ��
	if (baseDataMgr.GetGMFlag() || (!baseDataMgr.GetGMFlag() && CConfigManager::getSingleton()->GetGameFeatureActivationConfig().expeditionDungeonLevelLimit <= baseDataMgr.GetMasterLevel()))
	{
		if (!m_ptrDataMgr->GetExpeditionActiveState())
		{
			///��Ʊ�Ļ�������һЩ���ݣ����罱���Ƿ���ȡ��״̬
			m_ptrDataMgr->SetConquredState(false);
			m_ptrDataMgr->SetRewardsUnclaimedState(false);
			baseDataMgr.ResetExpedtiionInfor();
			m_ptrDataMgr->ResetEnemyInfor();

			if (m_iFirstExpeditionLevelID != -1)
				m_ptrDataMgr->SetCurExpeditionInstanceID(m_iFirstExpeditionLevelID);
			///��ȡ��ǰԶ���ؿ�����ϸ������Ϣ
			m_ptrCurLevelInstanceInfor = CConfigManager::getSingleton()->GetSpecifyExpeditionInstanceConfig(baseDataMgr.GetMasterLevel(), m_iFirstExpeditionLevelID);
			if (m_ptrCurLevelInstanceInfor == nullptr)
			{
				rfalse("Զ���ؿ�%d����ϸ���û�ȡ����", m_iFirstExpeditionLevelID);
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
	CBaseDataManager& baseDataMgr = m_ptrDataMgr->GetBaseDataMgr(); //��ȡ�������ݹ�����

	///GM��û���佫�ĵȼ�����
	if (!baseDataMgr.GetGMFlag() && CConfigManager::getSingleton()->GetGameFeatureActivationConfig().expeditionDungeonLevelLimit > baseDataMgr.GetMasterLevel())
		return;

	DWORD ticketNum = 0;
	///�ȿ�����ǰ��û��Ʊ�����������ô�����
	if (m_ptrDataMgr->GetCurTicketsNum(ticketNum) && ticketNum >= 1)
	{
		///��Ʊ�Ļ�������һЩ���ݣ����罱���Ƿ���ȡ��״̬
		m_ptrDataMgr->SetConquredState(false);
		m_ptrDataMgr->SetRewardsUnclaimedState(false);
		baseDataMgr.ResetExpedtiionInfor();
		m_ptrDataMgr->ResetEnemyInfor();

		///�޸����ݣ�ȥ��һ��Ʊ������Զ���ؿ��Ĺؿ�ID��Ϊ��һ��
		m_ptrDataMgr->CostATicket();
		if (m_iFirstExpeditionLevelID != -1)
			m_ptrDataMgr->SetCurExpeditionInstanceID(m_iFirstExpeditionLevelID);
		///��ȡ��ǰԶ���ؿ�����ϸ������Ϣ
		m_ptrCurLevelInstanceInfor = CConfigManager::getSingleton()->GetSpecifyExpeditionInstanceConfig(baseDataMgr.GetMasterLevel(), m_iFirstExpeditionLevelID);
		if (m_ptrCurLevelInstanceInfor == nullptr)
		{
			rfalse("Զ���ؿ�%d����ϸ���û�ȡ����", m_iFirstExpeditionLevelID);
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
		rfalse("Զ��ͬ����Ҳ�ս�佫Ѫ������");
		return;
	}

	CBaseDataManager& baseDataManager = m_ptrDataMgr->GetBaseDataMgr();
	const SHeroData* heroData = nullptr;
	///�ȼ��ͬ�����佫�Ƿ����¼���佫һ�£���һ�¼�¼���佫ȫ���ж�Ϊ���������ͷ��Դ�ʩ��
	for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
	{
		if (updateMsg->selectedHeroMap[i] == 0)
			continue;

		if (!m_ptrDataMgr->IsHeroSelected(updateMsg->selectedHeroMap[i]))
		{
			rfalse("Զ�������Ӣ��ID����Ҫͨ��֤��ʱ�򴫽�����ID��ͬ������������");
			///todo �������ɼ�¼
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
		rfalse("Զ��ͬ���з���ս�佫Ѫ������");
		return;
	}

	CBaseDataManager& baseDataManager = m_ptrDataMgr->GetBaseDataMgr();
	const SHeroData* heroData = nullptr;
	///�ȼ��ͬ�����佫�Ƿ����¼���佫һ�£����ͷ��Դ�ʩ��
	for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
	{
		if (updateMsg->machedEnemyMap[i] == 0)
			continue;

		if (!m_ptrDataMgr->IsEnemyExist(updateMsg->machedEnemyMap[i]))
		{
			rfalse("Զ������ĵ��˵�Ӣ��ID��ϵͳ��ID��ͬ������������");
			///todo �������ɼ�¼
		}
		else
		{
			m_ptrDataMgr->SetEnemyInfor(updateMsg->machedEnemyMap[i], updateMsg->enemyHealthScale[i], updateMsg->enemyManaScale[i]);
		}
	}
}

void CExpeditionManager::_UpdateExpeditionTimes()
{
	CBaseDataManager& baseDataMgr = m_ptrDataMgr->GetBaseDataMgr(); //��ȡ�������ݹ�����

	int playerSID = baseDataMgr.GetSID();
	BOOL execResult = FALSE;
	int retValue = 0;
	lite::Variant ret;//��lua��ȡ���ķ���ֵ
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

	///���¹���ն����
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
