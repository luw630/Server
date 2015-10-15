#include "stdafx.h"
#include "Duplicate.h"
#include "DuplicateBaseManager.h"
#include "DuplicateDataManager.h"
#include "..\BaseDataManager.h"
#include "..\Common\ConfigManager.h"
#include "NETWORKMODULE\SanguoPlayer.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "..\AchievementMoudle\AchieveUpdate.h"
#include "..\AchievementMoudle\RandomAchieveUpdate.h"
#include "..\MissionMoudle\CMissionUpdate.h"

CDuplicateBaseManager::CDuplicateBaseManager(CExtendedDataManager& dataMgr)
	: m_duplicateDataMgr((CDuplicateDataManager&)dataMgr),
	m_BaseDataManager(dataMgr.GetBaseDataMgr())
{
	m_DuplicateType = InstanceType::Story;
	m_bInitFlag = false;
	m_bEarningCleared = false;
	m_iCurChallengeTimes = 0;
	m_ptrCurDuplicate = nullptr;
	m_ptrAcievementUpdate = m_BaseDataManager.GetLifeTimeAchievementUpdator();
	m_ptrMissionUpdate = m_BaseDataManager.GetMissionUpdator();
	m_ptrRandomAchievementUpdator = m_BaseDataManager.GetRandomAchievementUpdator();
	m_ptrDuplicateIDList = nullptr;
	m_bPermissionGot = false;
	m_msgEnterPermissionAnswer.reset(new SAChallengePermissionInfor());
	m_msgResetSpecifyTollgate.reset(new SAResetSpecifyTollgate());
	m_msgActivateDuplciate = make_shared<SAActivateDuplicate>();
	m_msgActivateDuplciate->duplicateType = m_DuplicateType;
	m_msgEnterPermissionAnswer->duplicateType = m_DuplicateType;
	m_msgTollgateEarningClearedAnswer = make_shared<SATollgateEarningClearedInfor>();
	m_msgTollgateEarningClearedAnswer->duplicateType = m_DuplicateType;
	m_duplicateContainer.clear();
}

CDuplicateBaseManager::~CDuplicateBaseManager()
{
	m_duplicateContainer.clear();
	m_ptrAcievementUpdate = nullptr;
	m_ptrMissionUpdate = nullptr;
	m_ptrRandomAchievementUpdator = nullptr;
}

void CDuplicateBaseManager::OnRecvMsg(const SMessage *pMsg)
{
	const SDuplicateMsg * prtMsg = static_cast<const SDuplicateMsg*>(pMsg);
	if (prtMsg == nullptr)
	{
		rfalse("副本消息转换失败");
		return;
	}

	const SQDuplicateEarningClear* clearMsg = nullptr;

	switch (prtMsg->_protocol)
	{
	case SDuplicateMsg::REQUEST_ENTER_SPECIFY_TOLLGATE:
		///请求挑战指定的副本关卡
		AskToEnterTollgate(prtMsg);
		break;
	case SDuplicateMsg::REQUEST_EARNING_CLEAR:
		clearMsg = static_cast<const SQDuplicateEarningClear*>(pMsg);
		if (clearMsg != nullptr)
			EarningClearProcess(clearMsg->duplicateID, clearMsg->tollgateID, clearMsg->combatGrade);
		break;
	case SDuplicateMsg::REQUEST_START_CHALLENGE:
		OnStartToChallenge();
		/// 记录战斗开始时间(由于子类有重载OnStartToChanllenge 故在此处记录 不管此次是否通过战斗申请 在结算前都应由此函数通知切换场景)
		_time64(&m_tBattleStartTime);
		break;
	case SDuplicateMsg::REREQUEST_TOLLGATE_DROPED_ITEM:
		ReRequestTollgateDropedItem(prtMsg);
		break;
	default:
		///子类的差异化的消息处理模块
		DispatchMsg(prtMsg);
		break;
	}
}

void CDuplicateBaseManager::InitProgress()
{
	if (m_ptrDuplicateIDList == nullptr)
	{
		rfalse("无法初始化副本,因为获取不到对应副本类型的所有副本ID");
		return;
	}

	m_duplicateContainer.clear();
	shared_ptr<CDuplicate> lastDup = nullptr;
	for (auto iter : *m_ptrDuplicateIDList)
	{
		///初始化本副本
		shared_ptr<CDuplicate> duplicate = make_shared<CDuplicate>();
		duplicate->Init(&m_duplicateDataMgr, iter);

		///将副本串联起来，以便打完一个副本后能激活下一个副本
		if (lastDup != nullptr)
			lastDup->SetNextDuplciateID(iter);

		lastDup = duplicate;
		///将副本添加到容器中
		auto findResult = m_duplicateContainer.find(iter);
		if (findResult != m_duplicateContainer.end())
		{
			findResult->second = duplicate;
			rfalse("副本类型%d有相同副本", m_DuplicateType);
		}
		else
			m_duplicateContainer.insert(make_pair(iter, duplicate));
	}
}

void CDuplicateBaseManager::AskToEnterTollgate(const SDuplicateMsg *pMsg)
{
	m_iCurChallengeTimes = 0;
	m_bPermissionGot = false;
	const SQEnterSpecifyTollgate * ptrMsg = static_cast<const SQEnterSpecifyTollgate *>(pMsg);
	if (ptrMsg == nullptr || m_msgEnterPermissionAnswer == nullptr)
		return;

	m_msgEnterPermissionAnswer->permitted = false;
	m_msgEnterPermissionAnswer->bMopUpOperation = false;
	m_msgEnterPermissionAnswer->monsterNum = 0;
	bool bPermit = false;
	CheckResult checkResult = CheckResult::Failed;

	///查看请求的副本ID是否是本类型副本
	auto findResult = m_duplicateContainer.find(ptrMsg->duplicateID);
	if (findResult != m_duplicateContainer.end())
	{
		shared_ptr<CDuplicate> oldDuplicate = m_ptrCurDuplicate;
		m_ptrCurDuplicate = findResult->second;

		if (m_ptrCurDuplicate == nullptr)
		{
			///不予挑战
			rfalse("要挑战的关卡未激活或者不存在");
			m_ptrCurDuplicate = oldDuplicate;
			g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///减去不必要的空间 1024，因为后面有一个1024大小的数组存当前怪物的基础信息
			return;
		}

		OnAskToEnterTollgate();
		///查看当前的副本是否已经激活
		if (m_ptrCurDuplicate->GetActiveState())
		{
			checkResult = m_ptrCurDuplicate->Check(ptrMsg->tollgateID);
			if (checkResult == CheckResult::Pass)
			{
				m_msgEnterPermissionAnswer->permitted = true;
				///等待关卡结算
				m_bEarningCleared = false;
				m_bPermissionGot = true;
				m_iCurChallengeTimes = 1;
				///如果允许挑战，则记录当前的挑战关卡的英雄，扫荡模式没有选择的英雄，所以不记录英雄
				m_duplicateDataMgr.UpdateSelectedHero(ptrMsg->selectedHeroNum, ptrMsg->requestData);
			}
			else
				rfalse("副本和关卡的相关检测不允许挑战");

			const BattleLevelConfig* config = CConfigManager::getSingleton()->GetBattleLevelConfig(ptrMsg->tollgateID);
			if (config == nullptr)
			{
				g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///减去不必要的空间 1024，因为后面有一个1024大小的数组存当前怪物的基础信息
			}
			else
			{
				try
				{
					int dataLength = ptrMsg->selectedHeroNum + ptrMsg->monsterCachingNum;
					for (int i = ptrMsg->selectedHeroNum; i < dataLength; ++i)
					{
						auto findResult = config->LevelMonsters.find(ptrMsg->requestData[i]);
						if (findResult != config->LevelMonsters.end())
						{
							///获取怪物的基础属性
							if (!CConfigManager::getSingleton()->GetMonsterBaseData(*findResult, m_msgEnterPermissionAnswer->monsterData[m_msgEnterPermissionAnswer->monsterNum]))
								continue;

							++m_msgEnterPermissionAnswer->monsterNum;
							if (m_msgEnterPermissionAnswer->monsterNum > MAX_TOLLGATE_MONSTER_NUM)
								break;
						}
						else
						{
							rfalse("查找不到怪物ID： %d", ptrMsg->requestData[i]);
						}
					}
				}
				catch (std::exception& e)
				{
					rfalse(e.what());
				}

				g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(MonsterData) * (MAX_TOLLGATE_MONSTER_NUM - m_msgEnterPermissionAnswer->monsterNum)); ///减去不必要的空间 1024，因为后面有一个1024大小的数组存当前怪物的基础信息
			}
			return;
		}
		else
		{
			///不予挑战
			m_ptrCurDuplicate = oldDuplicate;
			g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///减去不必要的空间 1024，因为后面有一个1024大小的数组存当前怪物的基础信息
		}
	}
}

void CDuplicateBaseManager::EarningClearProcess(int duplicateID, int tollgateID, int combatGrade, bool sendMsg /* = true */)
{
	///如果没获准进入关卡挑战，则不予结算
	if (!m_bPermissionGot)
		return;

	if (sendMsg)
	{
		m_msgTollgateEarningClearedAnswer->duplicateID = m_ptrCurDuplicate->GetDuplicateID();
		m_msgTollgateEarningClearedAnswer->tollgateID = m_ptrCurDuplicate->GetCurTollgateID();
		m_msgTollgateEarningClearedAnswer->heroNum = 0;
		m_msgTollgateEarningClearedAnswer->bcleared = false;
		m_msgTollgateEarningClearedAnswer->bMopUpOperation = false;
	}
	
	///如果本挑战关卡已经结算过一次，则不再进行结算操作
	if (m_bEarningCleared)
	{
		if (sendMsg)
		{
			///告诉客户端结算成功
			m_msgTollgateEarningClearedAnswer->bcleared = true;
			g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgTollgateEarningClearedAnswer.get(), sizeof(SATollgateEarningClearedInfor));
		}
		
		return;
	}

	///检测结算时候传过来的副本ID是否跟验证是否可以挑战的时候传过来的副本ID一致
	if (m_ptrCurDuplicate->GetDuplicateID() != duplicateID)
	{
		rfalse("关卡结算的时候传过来的副本ID，跟检测能否挑战时传过来的副本ID不一致，不予结算");
		///<告诉客户端结算失败
		if (sendMsg)
			g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgTollgateEarningClearedAnswer.get(), sizeof(SATollgateEarningClearedInfor));
		return;
	}

	///更新副本相关的成就
	if (tollgateID != -1 && m_DuplicateType != InstanceType::LeagueOfLegends && m_DuplicateType != InstanceType::TreasureHunting)
	{
		if (m_ptrAcievementUpdate != nullptr)
			m_ptrAcievementUpdate->UpdateDuplicateAchieve(tollgateID, m_iCurChallengeTimes);
		if (m_ptrRandomAchievementUpdator != nullptr)
			m_ptrRandomAchievementUpdator->UpdateDuplicateAchieve(tollgateID, m_iCurChallengeTimes);
	}
	///更新副本相关的任务
	if (m_ptrMissionUpdate != nullptr)
		m_ptrMissionUpdate->UpdateDungeionMission(m_DuplicateType, m_iCurChallengeTimes);

	///当前副本开始结算
	m_ptrCurDuplicate->EarningClearing(duplicateID, tollgateID, combatGrade, m_iCurChallengeTimes);
	///进行服务器端结算的额外的操作,由子类丰富
	EarningClearingExtraProgress();
	///将关卡会增加的经验值加到对应武将身上，并同步给客户端
	PlusHeroExpBatch();
	m_bEarningCleared = true;

	if (sendMsg)
	{
		///告诉客户端结算成功
		m_msgTollgateEarningClearedAnswer->bcleared = true;
		g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgTollgateEarningClearedAnswer.get(), sizeof(SATollgateEarningClearedInfor) - sizeof(SHeroExpInfor) * (MAX_TOLLGATE_HERO_NUM - m_msgTollgateEarningClearedAnswer->heroNum));
	}
}

void CDuplicateBaseManager::PlusHeroExpBatch()
{
	int heroNum = 0;
	int level = 0;
	int exp = 0;
	bool levelLimitted = false;
	int heroBeforLevel = 0;
	int heroBeforeExp = 0;
	
	const list<int>& listSelectedHero = m_duplicateDataMgr.GetSelectedHeroList();
	for (auto const& itor : listSelectedHero)
	{
		exp = m_duplicateDataMgr.GetCurHeroExpIncreasement();
		auto heroData = m_BaseDataManager.GetHero(itor);
		if (heroData == nullptr)
			continue;

		heroBeforLevel = heroData->m_dwLevel;
		heroBeforeExp = heroData->m_Exp;
		///判断是否加经验成功，成功的话，再判断是否加之前武将是否满级了
		if (m_BaseDataManager.PlusHeroExp(itor, exp, level, levelLimitted))
		{
			///满级的话就不用再同步到客户端了
			if (levelLimitted && heroBeforLevel == level)
				continue;

			m_msgTollgateEarningClearedAnswer->heroExpInfor[heroNum].m_dwHeroID = itor;
			m_msgTollgateEarningClearedAnswer->heroExpInfor[heroNum].m_dwHeroLevel = level;
			m_msgTollgateEarningClearedAnswer->heroExpInfor[heroNum].m_dwIncreasedExp = max(0, exp - heroBeforeExp);
			m_msgTollgateEarningClearedAnswer->heroExpInfor[heroNum].m_dwHeroExp = exp;
			++heroNum;
		}
	}
	m_msgTollgateEarningClearedAnswer->heroNum = heroNum;
}

void CDuplicateBaseManager::OnStartToChallenge()
{
	if (m_ptrCurDuplicate == nullptr || !m_bPermissionGot)
		return;

	m_ptrCurDuplicate->ActionWhileEnterBattle(m_iCurChallengeTimes);
}

void CDuplicateBaseManager::ActiveAllDuplicate()
{
	if (m_duplicateContainer.size() == 0)
		return;

	for (auto iter : m_duplicateContainer)
	{
		if (iter.second != nullptr)
			iter.second->Activate();
	}
}

void CDuplicateBaseManager::EarningClearingExtraProgress()
{
	return;
}

bool CDuplicateBaseManager::IsDuplicateExist(int duplicateID)
{
	auto findResult = m_duplicateContainer.find(duplicateID);
	if (findResult == m_duplicateContainer.end())
		return false;

	return true;
}

void CDuplicateBaseManager::ResetSpecifyTollgateChallegedTimes(const SDuplicateMsg *pMsg)
{
	const SQResetTollgate* ptrMsg = static_cast<const SQResetTollgate*>(pMsg);
	if (ptrMsg == nullptr)
		return;

	auto findResult = m_duplicateContainer.find(ptrMsg->duplicateID);
	if (findResult == m_duplicateContainer.end() || findResult->second == nullptr)
		return;

	m_msgResetSpecifyTollgate->duplicateType = m_DuplicateType;
	m_msgResetSpecifyTollgate->bPermited = false;
	m_msgResetSpecifyTollgate->duplicateID = ptrMsg->duplicateID;
	m_msgResetSpecifyTollgate->tollgateID = ptrMsg->tollgateID;
	int diamondCost = GetCostOfResetSpecifyTollgateChallengedTimes(ptrMsg->tollgateID);
	///将副本的挑战次数的相关数据重置下
	if (diamondCost != -1 && m_BaseDataManager.CheckGoods_SG(GoodsType::diamond, 0, diamondCost) 
		&& m_duplicateDataMgr.ResetSpecifyTollgateChallegedTimes(m_DuplicateType, ptrMsg->tollgateID))
	{
		///开始重置相关的副本关卡的挑战次数
		findResult->second->ResetSpecifyTollgateTicket(ptrMsg->tollgateID);
		///扣除重置的花费
		m_BaseDataManager.DecGoods_SG(GoodsType::diamond, 0, diamondCost, GoodsWay::duplicate);
		m_msgResetSpecifyTollgate->bPermited = true;
	}

	g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgResetSpecifyTollgate.get(), sizeof(SAResetSpecifyTollgate));
}

int CDuplicateBaseManager::GetCostOfResetSpecifyTollgateChallengedTimes(int tollgateID)
{
	return -1;
}

void CDuplicateBaseManager::ReRequestTollgateDropedItem(const SDuplicateMsg *pMsg)
{
	const SSpecifyTollgateMsg* ptrMsg = static_cast<const SSpecifyTollgateMsg*>(pMsg);
	if (ptrMsg == nullptr)
		return;

	///如果重新请求的掉落信息不是当前的关卡所计算出来的，则不予返回
	if (ptrMsg->duplicateID != m_ptrCurDuplicate->GetDuplicateID() || ptrMsg->tollgateID != m_ptrCurDuplicate->GetCurTollgateID())
	{
		rfalse("想重新获取掉落的物品的消息中指定的副本ID、关卡ID跟当前的选中的副本、关卡不符合，不予获得");
		return;
	}

	m_ptrCurDuplicate->ReSendCaculatedDropItem();
}

void CDuplicateBaseManager::ActiveSpecifyDuplicate(int duplicateIDs[], int IDNum, bool bSendMsg /*= false*/)const
{
	///NOTE！！！！！！如果是GM号，则默认全副本激活，不必要进行如下的逻辑
	if (duplicateIDs == nullptr || IDNum ==  0 || m_BaseDataManager.GetGMFlag())
		return;

	IDNum = min(IDNum, MAX_LEAGUEOFLEGEND_DUPILICATE_NUM);

	///获取副本的激活的方式（是否为排他性的激活）
	bool bExclusivityState = false;
	GetActiveDuplicateExclusivityState(bExclusivityState);

	try
	{
		///如果是排他性的激活，则只激活列表中指定的副本，其他副本不激活
		if (bExclusivityState)
		{
			for (auto iter : m_duplicateContainer)
			{
				iter.second->Deactivate();
			}
		}

		for (int i = 0; i < IDNum; ++i)
		{
			if (m_duplicateContainer.size() == 0)
				return;

			auto findResult = m_duplicateContainer.find(duplicateIDs[i]);
			if (findResult == m_duplicateContainer.end())
				continue;

			findResult->second->Activate();
		}
	}
	catch (const std::exception& e)
	{
		rfalse("激活指定的一系列的副本报错");
		return;
	}

	if (bSendMsg)
	{
		if (m_msgActivateDuplciate == nullptr)
			return;

		m_msgActivateDuplciate->duplicateNum = IDNum;
		m_msgActivateDuplciate->duplicateType = m_DuplicateType;
		m_msgActivateDuplciate->bExclusivity = bExclusivityState;
		memcpy_s(m_msgActivateDuplciate->duplicateIDs, sizeof(int) * IDNum, duplicateIDs, sizeof(int) * IDNum);

		g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgActivateDuplciate.get(), sizeof(SAActivateDuplicate) - (MAX_LEAGUEOFLEGEND_DUPILICATE_NUM - IDNum) * sizeof(int));
	}
}

void CDuplicateBaseManager::GetActiveDuplicateExclusivityState(bool& exclusivityState) const
{
	exclusivityState = false;
}

void CDuplicateBaseManager::OnAskToEnterTollgate()
{
	return;
}

float CDuplicateBaseManager::BattleSpendSeconds() const
{
	__time64_t curTime;
	_time64(&curTime);
	return difftime(curTime, m_tBattleStartTime);
}
