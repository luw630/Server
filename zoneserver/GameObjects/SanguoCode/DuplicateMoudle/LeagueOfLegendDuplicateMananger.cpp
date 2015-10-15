#include "stdafx.h"
#include "LeagueOfLegendDuplicateMananger.h"
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

CLeagueOfLegendDuplicateMananger::CLeagueOfLegendDuplicateMananger(CExtendedDataManager& dataMgr)
	:CDuplicateBaseManager((CDuplicateDataManager&)dataMgr)
{
	m_DuplicateType = InstanceType::LeagueOfLegends;
}


CLeagueOfLegendDuplicateMananger::~CLeagueOfLegendDuplicateMananger()
{
}

void CLeagueOfLegendDuplicateMananger::InitMgr()
{
	m_bInitFlag = false;
	m_ptrDuplicateIDList = CConfigManager::getSingleton()->GetDuplicateListByType(m_DuplicateType);

	InitProgress();
	///�����GM�ţ�����˵�ĸ���ȫ��
	if (m_BaseDataManager.GetGMFlag())
		ActiveAllDuplicate();

	m_bInitFlag = true;
}

void CLeagueOfLegendDuplicateMananger::DispatchMsg(const SDuplicateMsg* pMsg)
{
	return;
}

void CLeagueOfLegendDuplicateMananger::Update()
{
	if (!m_bInitFlag == false) return;

	///������սCD��ʱû���ã�����Ҫ���£���ֻ�ڴ�����һ���ӿڣ����Ժ�Ҫ��Ҫ���
}

void CLeagueOfLegendDuplicateMananger::JudgeDuplicateAcitiveState()
{
	///NOTE�����������������GM�ţ���Ĭ��ȫ�����������Ҫ�������µ��߼�
	if (m_BaseDataManager.GetGMFlag())
		return;

	///��ʱ��VIP�ȼ��Ļ�ȡ�����ŵ�����,��ΪVIP�ĵȼ�����ʱ������䣬���ԡ�ʵʱ����ȡ
	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_BaseDataManager.GetSID())->DynamicCast(IID_PLAYER);
	if (pPlayer != nullptr)
	{
		g_Script.SetCondition(0, pPlayer, 0);
		LuaFunctor(g_Script, "SI_JudgeLOLActiveState")[m_BaseDataManager.GetSID()]();
		g_Script.CleanCondition();
	}
	else
		rfalse("��ȡ����CPlayer��ָ��");
}

void CLeagueOfLegendDuplicateMananger::GetActiveDuplicateExclusivityState(bool& exclusivityState) const
{
	///GM��ȫ������û�������Ե�
	if (!m_BaseDataManager.GetGMFlag())
		exclusivityState = true;
	else
		exclusivityState = false;
}