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
	///������һ������,��ʱ��Ӣ�ؿ��ļ��ͬ����Ϣ���ͻ��ˣ��ͻ������Լ��ľ�Ӣ�ؿ�������߼�����
	auto beginPos = m_duplicateContainer.begin();
	if (m_duplicateContainer.size() > 0 && beginPos->second != nullptr)
	{
		beginPos->second->Activate();
		//m_msgActivateDuplciate->duplicateID = beginPos->first;
		///��ʱ������Ϣ���ͻ���,��ʱ�ɿͻ����Լ��ж�
		//g_StoreMessage(m_BaseDataManager.GetDNID(), m_msgActivateDuplciate.get(), sizeof(SAActivateDuplicate));
	}
}

int CEliteDuplicateManager::GetCostOfResetSpecifyTollgateChallengedTimes(int tollgateID)
{
	///��ʱ��VIP�ȼ��Ļ�ȡ�����ŵ�����,��ΪVIP�ĵȼ�����ʱ������䣬���ԡ�ʵʱ����ȡ
	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_BaseDataManager.GetSID())->DynamicCast(IID_PLAYER);
	if (pPlayer != nullptr)
	{
		g_Script.SetCondition(0, pPlayer, 0);
		lite::Variant ret;//��lua��ǰ�����ô�������Ӧ��Ԫ������
		int resetedTimes = m_duplicateDataMgr.GetSpecifyTollgateResetedTimes(m_DuplicateType, tollgateID) + 1;
		LuaFunctor(g_Script, "SI_getNeedvalue")[m_BaseDataManager.GetSID()][Expense::Ex_ExpertPass][resetedTimes](&ret);
		int curDiamondCost = max((int)ret, 0);
		g_Script.CleanCondition();

		return curDiamondCost;
	}
	else
		rfalse("��ȡ����CPlayer��ָ��");

	return -1;
}

void CEliteDuplicateManager::OnAskToEnterTollgate()
{
	m_bMopUpOperation = false;
	if (!m_bDuplicateActivated && m_BaseDataManager.GetMasterLevel() >= CConfigManager::getSingleton()->GetGameFeatureActivationConfig().eliteDungeonLevelLimit)
		Activate();
}
