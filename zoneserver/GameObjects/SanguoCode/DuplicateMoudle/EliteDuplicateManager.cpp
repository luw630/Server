#include "stdafx.h"
#include "EliteDuplicateManager.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "DuplicateBaseManager.h"
#include "DuplicateDataManager.h"
#include "..\BaseDataManager.h"
#include "..\Common\ConfigManager.h"
#include "Duplicate.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "Player.h"
#include "ScriptManager.h"
extern LPIObject GetPlayerBySID(DWORD dwStaticID);


CEliteDuplicateManager::CEliteDuplicateManager(CExtendedDataManager& dataMgr)
	:CStoryDulicateManager((CDuplicateDataManager&)dataMgr)
{
	m_DuplicateType = InstanceType::StoryElite;
	if (m_msgActivateDuplciate)
		m_msgActivateDuplciate->duplicateType = m_DuplicateType;
}


CEliteDuplicateManager::~CEliteDuplicateManager()
{
}

void CEliteDuplicateManager::DispatchMsg(const SDuplicateMsg* pMsg)
{
	if (pMsg == nullptr)
		return;

	switch (pMsg->_protocol)
	{
	case SDuplicateMsg::REQUEST_RESET_TOLLGATE:
		ResetSpecifyTollgateChallegedTimes(pMsg);
		break;
	case  SDuplicateMsg::REQUEST_MOP_UP_TOLLGATE:
		CheckMopUpOperationPermission(pMsg);
		break;
	}
}

void CEliteDuplicateManager::InitMgr()
{
	m_bInitFlag = false;
	m_ptrDuplicateIDList = CConfigManager::getSingleton()->GetDuplicateListByType(m_DuplicateType);

	InitProgress();

	m_bInitFlag = true;
}

void CEliteDuplicateManager::Activate()
{
	m_bDuplicateActivated = true;
	///开启第一个副本,暂时精英关卡的激活不同步消息到客户端，客户端有自己的精英关卡激活的逻辑处理
	auto beginPos = m_duplicateContainer.begin();
	if (m_duplicateContainer.size() > 0 && beginPos->second != nullptr)
	{
		beginPos->second->Activate();
		//m_msgActivateDuplciate->duplicateID = beginPos->first;
		///暂时不发消息到客户端,暂时由客户端自己判断
		//g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgActivateDuplciate.get(), sizeof(SAActivateDuplicate));
	}
}

int CEliteDuplicateManager::GetCostOfResetSpecifyTollgateChallengedTimes(int tollgateID)
{
	///暂时将VIP等级的获取方法放到这里,因为VIP的等级“随时”都会变，所以“实时”获取
	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_BaseDataManager.GetSID())->DynamicCast(IID_PLAYER);
	if (pPlayer != nullptr)
	{
		g_Script.SetCondition(0, pPlayer, 0);
		lite::Variant ret;//从lua当前的重置次数所对应的元宝花费
		int resetedTimes = m_duplicateDataMgr.GetSpecifyTollgateResetedTimes(m_DuplicateType, tollgateID) + 1;
		LuaFunctor(g_Script, "SI_getNeedvalue")[m_BaseDataManager.GetSID()][Expense::Ex_ExpertPass][resetedTimes](&ret);
		int curDiamondCost = max((int)ret, 0);
		g_Script.CleanCondition();

		return curDiamondCost;
	}
	else
		rfalse("获取不到CPlayer的指针");

	return -1;
}

void CEliteDuplicateManager::OnAskToEnterTollgate()
{
	m_bMopUpOperation = false;
	if (!m_bDuplicateActivated && m_BaseDataManager.GetMasterLevel() >= CConfigManager::getSingleton()->GetGameFeatureActivationConfig().eliteDungeonLevelLimit)
		Activate();
}
