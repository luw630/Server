#include "stdafx.h"
#include "StoryDulicateManager.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "DuplicateBaseManager.h"
#include "DuplicateDataManager.h"
#include "..\BaseDataManager.h"
#include "..\Common\ConfigManager.h"
#include "..\StorageMoudle\StorageManager.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "Player.h"
#include "ScriptManager.h"
#include "Duplicate.h"
extern LPIObject GetPlayerBySID(DWORD dwStaticID);

CStoryDulicateManager::CStoryDulicateManager(CExtendedDataManager& dataMgr)
	:CDuplicateBaseManager((CDuplicateDataManager&)dataMgr)
{
	m_DuplicateType = InstanceType::Story;
	m_bDuplicateActivated = false;
	m_bMopUpOperation = false;
	m_iMopUpTicketID = 0;
}


CStoryDulicateManager::~CStoryDulicateManager()
{
}

void CStoryDulicateManager::InitMgr()
{
	m_bInitFlag = false;
	m_ptrDuplicateIDList = CConfigManager::getSingleton()->GetDuplicateListByType(m_DuplicateType);
	
	InitProgress();

	///��ͨ����Ĭ�ϼ����һ������
	if (m_duplicateContainer.size() > 0)
	{
		auto firstDuplicate = m_duplicateContainer.begin();
		firstDuplicate->second->Activate();
	}
	///���¸���Ĭ�ϻἤ��
	m_bDuplicateActivated = true;
	m_bInitFlag = true;
}

void CStoryDulicateManager::DispatchMsg(const SDuplicateMsg* pMsg)
{
	if (pMsg == nullptr)
		return;

	switch (pMsg->_protocol)
	{
	case SDuplicateMsg::REQUEST_MOP_UP_TOLLGATE:
		CheckMopUpOperationPermission(pMsg);
		break;
	}
}

void CStoryDulicateManager::ActiveTo(int tollgateID)
{
	bool bFinded = false;
	for (auto iter : m_duplicateContainer)
	{
		if (iter.second == nullptr) continue;

		if (iter.second->IsTollgateExist(tollgateID))
		{
			bFinded = true;
			break;
		}
	}

	if (bFinded)
	{
		m_bDuplicateActivated = true;
		for (auto iter : m_duplicateContainer)
		{
			if (iter.second == nullptr) continue;

			if (!iter.second->IsTollgateExist(tollgateID))
			{
				iter.second->ActiveAllTollgate();
			}
			else
			{
				bFinded = true;
				iter.second->ActivateTo(tollgateID);
				break;
			}
		}
	}
}

void CStoryDulicateManager::Update()
{
	if (!m_bInitFlag == false) return;

	///������սCD��ʱû���ã�����Ҫ���£���ֻ�ڴ�����һ���ӿڣ����Ժ�Ҫ��Ҫ���
}

void CStoryDulicateManager::EarningClearingExtraProgress()
{
	m_msgTollgateEarningClearedAnswer->bMopUpOperation = m_bMopUpOperation;
	///�������ɨ��ģʽ���ɹ���ս��һ�������󣬼�����һ������
	if (m_ptrCurDuplicate->GetCompletedState() && !m_bMopUpOperation)
	{
		auto findResult = m_duplicateContainer.find(m_ptrCurDuplicate->GetNextDuplicateID());
		if (findResult != m_duplicateContainer.end() && findResult->second != nullptr)
			findResult->second->Activate();
	}
	else if (m_bMopUpOperation)
	{
		///ɨ����ģʽ
		if (m_iMopUpTicketID != 0)
		{
			m_BaseDataManager.DecGoods_SG(GoodsType::item, m_iMopUpTicketID, m_iCurChallengeTimes, GoodsWay::duplicate);
		}
		else///�ñ�ʯɨ��ģʽ
		{
			const CGlobalConfig& config = CConfigManager::getSingleton()->globalConfig;
			m_BaseDataManager.DecGoods_SG(GoodsType::diamond, 0, m_iCurChallengeTimes * config.DiamondSweepCost, GoodsWay::duplicate);
		}
	}

	m_bMopUpOperation = false;
	for (auto & func : m_funcClearing)
	{
		if (!func._Empty())
			func();
	}
}

void CStoryDulicateManager::CheckMopUpOperationPermission(const SDuplicateMsg *pMsg)
{
	m_bPermissionGot = false;
	m_msgEnterPermissionAnswer->permitted = false;
	m_msgEnterPermissionAnswer->bMopUpOperation = true;
	m_msgEnterPermissionAnswer->monsterNum = 0;
	const SQMopUpTollgate* ptrMsg = static_cast<const SQMopUpTollgate*>(pMsg);
	///�������ID���ڱ����������У�������ս
	if (ptrMsg == nullptr || !IsDuplicateExist(ptrMsg->duplicateID))
	{
		///�ͻ��˲�����ս
		g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///��ȥ����Ҫ�Ŀռ� 1024����Ϊ������һ��1024��С������浱ǰ����Ļ�����Ϣ
		return;
	}

	///ɨ��ʮ��ֻ��VIP����һ���ĵȼ����ܿ���
	if (ptrMsg->challengeTimes >= 10)
	{
		///��ʱ��VIP�ȼ��Ļ�ȡ�����ŵ�����,��ΪVIP�ĵȼ�����ʱ������䣬���ԡ�ʵʱ����ȡ
		CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_BaseDataManager.GetSID())->DynamicCast(IID_PLAYER);
		int MopUpTenTimesAvaliableState = 0;
		if (pPlayer != nullptr)
		{
			g_Script.SetCondition(0, pPlayer, 0);
			lite::Variant ret;//��lua��ȡɨ��ʮ���Ƿ񿪷ŵ�״̬
			LuaFunctor(g_Script, "SI_vip_getDetail")[g_Script.m_pPlayer->GetSID()][VipLevelFactor::VF_ATKTen_Num](&ret);
			MopUpTenTimesAvaliableState = (int)ret;
			g_Script.CleanCondition();
		}
		else
			rfalse("��ȡ����CPlayer��ָ��");

		if (MopUpTenTimesAvaliableState <= 0)
		{
			g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///��ȥ����Ҫ�Ŀռ� 1024����Ϊ������һ��1024��С������浱ǰ����Ļ�����Ϣ
			return;
		}
	}

	///�жϷ�������ɨ������
	if (ptrMsg->sweepTicketID != 0)
	{
		///���ֿ��е�ɨ�����Ƿ��㹻
		if (!m_BaseDataManager.GetStorageManager().CheckSweepTicket(ptrMsg->sweepTicketID, ptrMsg->challengeTimes))
		{
			g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///��ȥ����Ҫ�Ŀռ� 1024����Ϊ������һ��1024��С������浱ǰ����Ļ�����Ϣ
			return;
		}
		///�ͻ��˲�����ս
		m_iMopUpTicketID = ptrMsg->sweepTicketID;
	}
	else ///Ԫ��ɨ��
	{
		m_iMopUpTicketID = 0;
		const CGlobalConfig& config = CConfigManager::getSingleton()->globalConfig;
		if (!m_BaseDataManager.CheckGoods_SG(GoodsType::diamond, 0, ptrMsg->challengeTimes * config.DiamondSweepCost))
		{
			///�ͻ��˲�����ս
			g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///��ȥ����Ҫ�Ŀռ� 1024����Ϊ������һ��1024��С������浱ǰ����Ļ�����Ϣ
			return;
		}
	}

	shared_ptr<CDuplicate> oldDuplicate = m_ptrCurDuplicate;
	m_ptrCurDuplicate = m_duplicateContainer[ptrMsg->duplicateID];

	OnAskToEnterTollgate();
	if (!m_ptrCurDuplicate->GetActiveState())
	{
		///�ͻ��˲�����ս
		g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///��ȥ����Ҫ�Ŀռ� 1024����Ϊ������һ��1024��С������浱ǰ����Ļ�����Ϣ
		m_ptrCurDuplicate = oldDuplicate;
		return;
	}

	///��⵱ǰ�����Ƿ����ɨ��
	CheckResult checkResult = m_ptrCurDuplicate->Check(ptrMsg->tollgateID, ptrMsg->challengeTimes, true);
	if (checkResult == CheckResult::Pass)
	{
		m_bEarningCleared = false;
		m_bPermissionGot = true;
		m_bMopUpOperation = true;
		m_msgEnterPermissionAnswer->permitted = true;
		///���������ս�Ļ������¼Ҫ��ս�Ĵ�����������ؿ�����ͽ����ʱ����
		m_iCurChallengeTimes = ptrMsg->challengeTimes;

		///���������ս�Ļ��������õ�����Ʒ���������Ʒһ����Ū��
		OnStartToChallenge();
		EarningClearProcess(ptrMsg->duplicateID, ptrMsg->tollgateID, 0, false);
	}

	g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///��ȥ����Ҫ�Ŀռ� 1024����Ϊ������һ��1024��С������浱ǰ����Ļ�����Ϣ
}

void CStoryDulicateManager::OnStartToChallenge()
{
	if (m_ptrCurDuplicate == nullptr || !m_bPermissionGot)
		return;

	m_ptrCurDuplicate->ActionWhileEnterBattle(m_iCurChallengeTimes, m_bMopUpOperation);
}

void CStoryDulicateManager::BindClearingEvent(EmptyFunction& func)
{
	m_funcClearing.push_back(func);
}

void CStoryDulicateManager::OnAskToEnterTollgate()
{
	m_bMopUpOperation = false;
}
