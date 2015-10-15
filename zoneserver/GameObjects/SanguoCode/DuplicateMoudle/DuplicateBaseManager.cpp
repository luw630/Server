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
		rfalse("������Ϣת��ʧ��");
		return;
	}

	const SQDuplicateEarningClear* clearMsg = nullptr;

	switch (prtMsg->_protocol)
	{
	case SDuplicateMsg::REQUEST_ENTER_SPECIFY_TOLLGATE:
		///������սָ���ĸ����ؿ�
		AskToEnterTollgate(prtMsg);
		break;
	case SDuplicateMsg::REQUEST_EARNING_CLEAR:
		clearMsg = static_cast<const SQDuplicateEarningClear*>(pMsg);
		if (clearMsg != nullptr)
			EarningClearProcess(clearMsg->duplicateID, clearMsg->tollgateID, clearMsg->combatGrade);
		break;
	case SDuplicateMsg::REQUEST_START_CHALLENGE:
		OnStartToChallenge();
		/// ��¼ս����ʼʱ��(��������������OnStartToChanllenge ���ڴ˴���¼ ���ܴ˴��Ƿ�ͨ��ս������ �ڽ���ǰ��Ӧ�ɴ˺���֪ͨ�л�����)
		_time64(&m_tBattleStartTime);
		break;
	case SDuplicateMsg::REREQUEST_TOLLGATE_DROPED_ITEM:
		ReRequestTollgateDropedItem(prtMsg);
		break;
	default:
		///����Ĳ��컯����Ϣ����ģ��
		DispatchMsg(prtMsg);
		break;
	}
}

void CDuplicateBaseManager::InitProgress()
{
	if (m_ptrDuplicateIDList == nullptr)
	{
		rfalse("�޷���ʼ������,��Ϊ��ȡ������Ӧ�������͵����и���ID");
		return;
	}

	m_duplicateContainer.clear();
	shared_ptr<CDuplicate> lastDup = nullptr;
	for (auto iter : *m_ptrDuplicateIDList)
	{
		///��ʼ��������
		shared_ptr<CDuplicate> duplicate = make_shared<CDuplicate>();
		duplicate->Init(&m_duplicateDataMgr, iter);

		///�����������������Ա����һ���������ܼ�����һ������
		if (lastDup != nullptr)
			lastDup->SetNextDuplciateID(iter);

		lastDup = duplicate;
		///��������ӵ�������
		auto findResult = m_duplicateContainer.find(iter);
		if (findResult != m_duplicateContainer.end())
		{
			findResult->second = duplicate;
			rfalse("��������%d����ͬ����", m_DuplicateType);
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

	///�鿴����ĸ���ID�Ƿ��Ǳ����͸���
	auto findResult = m_duplicateContainer.find(ptrMsg->duplicateID);
	if (findResult != m_duplicateContainer.end())
	{
		shared_ptr<CDuplicate> oldDuplicate = m_ptrCurDuplicate;
		m_ptrCurDuplicate = findResult->second;

		if (m_ptrCurDuplicate == nullptr)
		{
			///������ս
			rfalse("Ҫ��ս�Ĺؿ�δ������߲�����");
			m_ptrCurDuplicate = oldDuplicate;
			g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///��ȥ����Ҫ�Ŀռ� 1024����Ϊ������һ��1024��С������浱ǰ����Ļ�����Ϣ
			return;
		}

		OnAskToEnterTollgate();
		///�鿴��ǰ�ĸ����Ƿ��Ѿ�����
		if (m_ptrCurDuplicate->GetActiveState())
		{
			checkResult = m_ptrCurDuplicate->Check(ptrMsg->tollgateID);
			if (checkResult == CheckResult::Pass)
			{
				m_msgEnterPermissionAnswer->permitted = true;
				///�ȴ��ؿ�����
				m_bEarningCleared = false;
				m_bPermissionGot = true;
				m_iCurChallengeTimes = 1;
				///���������ս�����¼��ǰ����ս�ؿ���Ӣ�ۣ�ɨ��ģʽû��ѡ���Ӣ�ۣ����Բ���¼Ӣ��
				m_duplicateDataMgr.UpdateSelectedHero(ptrMsg->selectedHeroNum, ptrMsg->requestData);
			}
			else
				rfalse("�����͹ؿ�����ؼ�ⲻ������ս");

			const BattleLevelConfig* config = CConfigManager::getSingleton()->GetBattleLevelConfig(ptrMsg->tollgateID);
			if (config == nullptr)
			{
				g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///��ȥ����Ҫ�Ŀռ� 1024����Ϊ������һ��1024��С������浱ǰ����Ļ�����Ϣ
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
							///��ȡ����Ļ�������
							if (!CConfigManager::getSingleton()->GetMonsterBaseData(*findResult, m_msgEnterPermissionAnswer->monsterData[m_msgEnterPermissionAnswer->monsterNum]))
								continue;

							++m_msgEnterPermissionAnswer->monsterNum;
							if (m_msgEnterPermissionAnswer->monsterNum > MAX_TOLLGATE_MONSTER_NUM)
								break;
						}
						else
						{
							rfalse("���Ҳ�������ID�� %d", ptrMsg->requestData[i]);
						}
					}
				}
				catch (std::exception& e)
				{
					rfalse(e.what());
				}

				g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(MonsterData) * (MAX_TOLLGATE_MONSTER_NUM - m_msgEnterPermissionAnswer->monsterNum)); ///��ȥ����Ҫ�Ŀռ� 1024����Ϊ������һ��1024��С������浱ǰ����Ļ�����Ϣ
			}
			return;
		}
		else
		{
			///������ս
			m_ptrCurDuplicate = oldDuplicate;
			g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///��ȥ����Ҫ�Ŀռ� 1024����Ϊ������һ��1024��С������浱ǰ����Ļ�����Ϣ
		}
	}
}

void CDuplicateBaseManager::EarningClearProcess(int duplicateID, int tollgateID, int combatGrade, bool sendMsg /* = true */)
{
	///���û��׼����ؿ���ս���������
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
	
	///�������ս�ؿ��Ѿ������һ�Σ����ٽ��н������
	if (m_bEarningCleared)
	{
		if (sendMsg)
		{
			///���߿ͻ��˽���ɹ�
			m_msgTollgateEarningClearedAnswer->bcleared = true;
			g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgTollgateEarningClearedAnswer.get(), sizeof(SATollgateEarningClearedInfor));
		}
		
		return;
	}

	///������ʱ�򴫹����ĸ���ID�Ƿ����֤�Ƿ������ս��ʱ�򴫹����ĸ���IDһ��
	if (m_ptrCurDuplicate->GetDuplicateID() != duplicateID)
	{
		rfalse("�ؿ������ʱ�򴫹����ĸ���ID��������ܷ���սʱ�������ĸ���ID��һ�£��������");
		///<���߿ͻ��˽���ʧ��
		if (sendMsg)
			g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgTollgateEarningClearedAnswer.get(), sizeof(SATollgateEarningClearedInfor));
		return;
	}

	///���¸�����صĳɾ�
	if (tollgateID != -1 && m_DuplicateType != InstanceType::LeagueOfLegends && m_DuplicateType != InstanceType::TreasureHunting)
	{
		if (m_ptrAcievementUpdate != nullptr)
			m_ptrAcievementUpdate->UpdateDuplicateAchieve(tollgateID, m_iCurChallengeTimes);
		if (m_ptrRandomAchievementUpdator != nullptr)
			m_ptrRandomAchievementUpdator->UpdateDuplicateAchieve(tollgateID, m_iCurChallengeTimes);
	}
	///���¸�����ص�����
	if (m_ptrMissionUpdate != nullptr)
		m_ptrMissionUpdate->UpdateDungeionMission(m_DuplicateType, m_iCurChallengeTimes);

	///��ǰ������ʼ����
	m_ptrCurDuplicate->EarningClearing(duplicateID, tollgateID, combatGrade, m_iCurChallengeTimes);
	///���з������˽���Ķ���Ĳ���,������ḻ
	EarningClearingExtraProgress();
	///���ؿ������ӵľ���ֵ�ӵ���Ӧ�佫���ϣ���ͬ�����ͻ���
	PlusHeroExpBatch();
	m_bEarningCleared = true;

	if (sendMsg)
	{
		///���߿ͻ��˽���ɹ�
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
		///�ж��Ƿ�Ӿ���ɹ����ɹ��Ļ������ж��Ƿ��֮ǰ�佫�Ƿ�������
		if (m_BaseDataManager.PlusHeroExp(itor, exp, level, levelLimitted))
		{
			///�����Ļ��Ͳ�����ͬ�����ͻ�����
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
	///����������ս�������������������
	if (diamondCost != -1 && m_BaseDataManager.CheckGoods_SG(GoodsType::diamond, 0, diamondCost) 
		&& m_duplicateDataMgr.ResetSpecifyTollgateChallegedTimes(m_DuplicateType, ptrMsg->tollgateID))
	{
		///��ʼ������صĸ����ؿ�����ս����
		findResult->second->ResetSpecifyTollgateTicket(ptrMsg->tollgateID);
		///�۳����õĻ���
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

	///�����������ĵ�����Ϣ���ǵ�ǰ�Ĺؿ�����������ģ����践��
	if (ptrMsg->duplicateID != m_ptrCurDuplicate->GetDuplicateID() || ptrMsg->tollgateID != m_ptrCurDuplicate->GetCurTollgateID())
	{
		rfalse("�����»�ȡ�������Ʒ����Ϣ��ָ���ĸ���ID���ؿ�ID����ǰ��ѡ�еĸ������ؿ������ϣ�������");
		return;
	}

	m_ptrCurDuplicate->ReSendCaculatedDropItem();
}

void CDuplicateBaseManager::ActiveSpecifyDuplicate(int duplicateIDs[], int IDNum, bool bSendMsg /*= false*/)const
{
	///NOTE�����������������GM�ţ���Ĭ��ȫ�����������Ҫ�������µ��߼�
	if (duplicateIDs == nullptr || IDNum ==  0 || m_BaseDataManager.GetGMFlag())
		return;

	IDNum = min(IDNum, MAX_LEAGUEOFLEGEND_DUPILICATE_NUM);

	///��ȡ�����ļ���ķ�ʽ���Ƿ�Ϊ�����Եļ��
	bool bExclusivityState = false;
	GetActiveDuplicateExclusivityState(bExclusivityState);

	try
	{
		///����������Եļ����ֻ�����б���ָ���ĸ�������������������
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
		rfalse("����ָ����һϵ�еĸ�������");
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
