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

	///普通副本默认激活第一个副本
	if (m_duplicateContainer.size() > 0)
	{
		auto firstDuplicate = m_duplicateContainer.begin();
		firstDuplicate->second->Activate();
	}
	///故事副本默认会激活
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

	///由于挑战CD暂时没有用，不需要更新，故只在次声明一个接口，看以后要不要添加
}

void CStoryDulicateManager::EarningClearingExtraProgress()
{
	m_msgTollgateEarningClearedAnswer->bMopUpOperation = m_bMopUpOperation;
	///如果不是扫荡模式，成功挑战完一个副本后，激活下一个副本
	if (m_ptrCurDuplicate->GetCompletedState() && !m_bMopUpOperation)
	{
		auto findResult = m_duplicateContainer.find(m_ptrCurDuplicate->GetNextDuplicateID());
		if (findResult != m_duplicateContainer.end() && findResult->second != nullptr)
			findResult->second->Activate();
	}
	else if (m_bMopUpOperation)
	{
		///扫荡卷模式
		if (m_iMopUpTicketID != 0)
		{
			m_BaseDataManager.DecGoods_SG(GoodsType::item, m_iMopUpTicketID, m_iCurChallengeTimes, GoodsWay::duplicate);
		}
		else///用宝石扫荡模式
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
	///如果副本ID不在本副本类型中，不予挑战
	if (ptrMsg == nullptr || !IsDuplicateExist(ptrMsg->duplicateID))
	{
		///客户端不予挑战
		g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///减去不必要的空间 1024，因为后面有一个1024大小的数组存当前怪物的基础信息
		return;
	}

	///扫荡十次只有VIP到达一定的等级才能开放
	if (ptrMsg->challengeTimes >= 10)
	{
		///暂时将VIP等级的获取方法放到这里,因为VIP的等级“随时”都会变，所以“实时”获取
		CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_BaseDataManager.GetSID())->DynamicCast(IID_PLAYER);
		int MopUpTenTimesAvaliableState = 0;
		if (pPlayer != nullptr)
		{
			g_Script.SetCondition(0, pPlayer, 0);
			lite::Variant ret;//从lua获取扫荡十次是否开放的状态
			LuaFunctor(g_Script, "SI_vip_getDetail")[g_Script.m_pPlayer->GetSID()][VipLevelFactor::VF_ATKTen_Num](&ret);
			MopUpTenTimesAvaliableState = (int)ret;
			g_Script.CleanCondition();
		}
		else
			rfalse("获取不到CPlayer的指针");

		if (MopUpTenTimesAvaliableState <= 0)
		{
			g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///减去不必要的空间 1024，因为后面有一个1024大小的数组存当前怪物的基础信息
			return;
		}
	}

	///判断发过来的扫荡劵的
	if (ptrMsg->sweepTicketID != 0)
	{
		///检测仓库中的扫荡卷是否足够
		if (!m_BaseDataManager.GetStorageManager().CheckSweepTicket(ptrMsg->sweepTicketID, ptrMsg->challengeTimes))
		{
			g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///减去不必要的空间 1024，因为后面有一个1024大小的数组存当前怪物的基础信息
			return;
		}
		///客户端不予挑战
		m_iMopUpTicketID = ptrMsg->sweepTicketID;
	}
	else ///元宝扫荡
	{
		m_iMopUpTicketID = 0;
		const CGlobalConfig& config = CConfigManager::getSingleton()->globalConfig;
		if (!m_BaseDataManager.CheckGoods_SG(GoodsType::diamond, 0, ptrMsg->challengeTimes * config.DiamondSweepCost))
		{
			///客户端不予挑战
			g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///减去不必要的空间 1024，因为后面有一个1024大小的数组存当前怪物的基础信息
			return;
		}
	}

	shared_ptr<CDuplicate> oldDuplicate = m_ptrCurDuplicate;
	m_ptrCurDuplicate = m_duplicateContainer[ptrMsg->duplicateID];

	OnAskToEnterTollgate();
	if (!m_ptrCurDuplicate->GetActiveState())
	{
		///客户端不予挑战
		g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///减去不必要的空间 1024，因为后面有一个1024大小的数组存当前怪物的基础信息
		m_ptrCurDuplicate = oldDuplicate;
		return;
	}

	///检测当前副本是否可以扫荡
	CheckResult checkResult = m_ptrCurDuplicate->Check(ptrMsg->tollgateID, ptrMsg->challengeTimes, true);
	if (checkResult == CheckResult::Pass)
	{
		m_bEarningCleared = false;
		m_bPermissionGot = true;
		m_bMopUpOperation = true;
		m_msgEnterPermissionAnswer->permitted = true;
		///如果允许挑战的话，则记录要挑战的次数，供计算关卡掉落和结算的时候用
		m_iCurChallengeTimes = ptrMsg->challengeTimes;

		///如果允许挑战的话，则计算好掉落物品，结算好物品一条龙弄完
		OnStartToChallenge();
		EarningClearProcess(ptrMsg->duplicateID, ptrMsg->tollgateID, 0, false);
	}

	g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgEnterPermissionAnswer.get(), sizeof(SAChallengePermissionInfor) - sizeof(m_msgEnterPermissionAnswer->monsterData)); ///减去不必要的空间 1024，因为后面有一个1024大小的数组存当前怪物的基础信息
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
