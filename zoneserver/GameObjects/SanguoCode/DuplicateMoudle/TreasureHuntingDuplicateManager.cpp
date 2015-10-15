#include "stdafx.h"
#include "TreasureHuntingDuplicateManager.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "DuplicateBaseManager.h"
#include "DuplicateDataManager.h"
#include "..\BaseDataManager.h"
#include "..\Common\ConfigManager.h"
#include "Duplicate.h"

CTreasureHuntingDuplicateManager::CTreasureHuntingDuplicateManager(CExtendedDataManager& dataMgr)
	:CDuplicateBaseManager((CDuplicateDataManager&)dataMgr)
{
	m_DuplicateType = InstanceType::TreasureHunting;
}


CTreasureHuntingDuplicateManager::~CTreasureHuntingDuplicateManager()
{
}

void CTreasureHuntingDuplicateManager::InitMgr()
{
	m_bInitFlag = false;
	m_ptrDuplicateIDList = CConfigManager::getSingleton()->GetDuplicateListByType(m_DuplicateType);

	InitProgress();
	///开启所有的副本关卡，因为秘境寻宝副本暂时没有激活或者不激活的概念，一开始玩家就可以挑战所有副本，只是等级有所限制
	ActiveAllDuplicate();
	m_bInitFlag = true;
}

void CTreasureHuntingDuplicateManager::DispatchMsg(const SDuplicateMsg* pMsg)
{

}

void CTreasureHuntingDuplicateManager::Update()
{
	if (!m_bInitFlag == false) return;

	///由于挑战CD暂时没有用，不需要更新，故只在次声明一个接口，看以后要不要添加
}
