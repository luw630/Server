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
	///�������еĸ����ؿ�����Ϊ�ؾ�Ѱ��������ʱû�м�����߲�����ĸ��һ��ʼ��ҾͿ�����ս���и�����ֻ�ǵȼ���������
	ActiveAllDuplicate();
	m_bInitFlag = true;
}

void CTreasureHuntingDuplicateManager::DispatchMsg(const SDuplicateMsg* pMsg)
{

}

void CTreasureHuntingDuplicateManager::Update()
{
	if (!m_bInitFlag == false) return;

	///������սCD��ʱû���ã�����Ҫ���£���ֻ�ڴ�����һ���ӿڣ����Ժ�Ҫ��Ҫ���
}
