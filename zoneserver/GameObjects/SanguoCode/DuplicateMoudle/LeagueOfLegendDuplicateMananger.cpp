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
	///如果是GM号，则将神传说的副本全开
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

	///由于挑战CD暂时没有用，不需要更新，故只在次声明一个接口，看以后要不要添加
}

void CLeagueOfLegendDuplicateMananger::JudgeDuplicateAcitiveState()
{
	///NOTE！！！！！！如果是GM号，则默认全副本激活，不必要进行如下的逻辑
	if (m_BaseDataManager.GetGMFlag())
		return;

	///暂时将VIP等级的获取方法放到这里,因为VIP的等级“随时”都会变，所以“实时”获取
	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_BaseDataManager.GetSID())->DynamicCast(IID_PLAYER);
	if (pPlayer != nullptr)
	{
		g_Script.SetCondition(0, pPlayer, 0);
		LuaFunctor(g_Script, "SI_JudgeLOLActiveState")[m_BaseDataManager.GetSID()]();
		g_Script.CleanCondition();
	}
	else
		rfalse("获取不到CPlayer的指针");
}

void CLeagueOfLegendDuplicateMananger::GetActiveDuplicateExclusivityState(bool& exclusivityState) const
{
	///GM号全开，故没有排他性的
	if (!m_BaseDataManager.GetGMFlag())
		exclusivityState = true;
	else
		exclusivityState = false;
}