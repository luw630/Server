#include "stdafx.h"
#include <time.h>
#include "ChaseThiefDataManager.h"
#include "../BaseDataManager.h"
#include "../Common/PubTool.h"
#include "../../../NETWORKMODULE/SanguoPlayer.h"
#include "../../../NETWORKMODULE/SanguoPlayerMsg.h"
#include "../Common/ConfigManager.h"
#include "Random.h"
#include "GameWorld.h"
#include "LuckyTimeSystem.h"

DWORD CChaseThiefDataManager::m_dwChaseThiefDurationTime = CConfigManager::getSingleton()->globalConfig.ChaseThiefDurationTime;
DWORD CChaseThiefDataManager::m_dwChaseThiefOpenLevel = CConfigManager::getSingleton()->globalConfig.ChaseThiefOpenLevel;
float CChaseThiefDataManager::m_fChaseThiefActivateRate = CConfigManager::getSingleton()->globalConfig.ChaseThiefActivateRate;

CChaseThiefDataManager::CChaseThiefDataManager(CBaseDataManager& baseDataMgr)
	: CExtendedDataManager(baseDataMgr)
	, m_pChaseThiefData(nullptr)
	, m_tEffectiveTime(0)
	, m_bChaseStarted(false)
	, m_iRewardID(0)
	, m_iRewardNum(0)
{
	m_funcActivate = std::bind(&CChaseThiefDataManager::TakeEffect, this);
	m_vecChasingHeroes.clear();
}


CChaseThiefDataManager::~CChaseThiefDataManager()
{
	m_pChaseThiefData = nullptr;
	m_vecChasingHeroes.clear();
}

bool CChaseThiefDataManager::InitDataMgr(void * pData)
{
	if (nullptr == pData)
		return false;

	m_pChaseThiefData = static_cast<SChaseThiefData*>(pData);
	if (m_pBaseDataMgr.IsNewPlayer())
	{
		if (m_pChaseThiefData != nullptr)
		{
			ZeroMemory(m_pChaseThiefData->activateTime, sizeof(char) * 8);
			m_pChaseThiefData->valid = false;
		}
	}
	else
	{
		if (m_pChaseThiefData != nullptr)
		{
			m_tEffectiveTime = *((__time64_t*)m_pChaseThiefData->activateTime);
			CheckEffectiveness();
		}
	}
	
	return true;
}

bool CChaseThiefDataManager::ReleaseDataMgr()
{
	m_pChaseThiefData = nullptr;
	return true;
}

void CChaseThiefDataManager::TakeEffect()
{
	/// �����������Ч�� ���ٴ�����
	if (CheckEffectiveness())
		return;

	if (!_ChaseThiefDataIsValid())
		return;

	/// �ж��Ƿ��Ǻ�ҹ
	CLuckyTimeSystem* pLuckySys = nullptr; 
	GetGW()->GetLuckySystemPtr(&pLuckySys);
	if (nullptr == pLuckySys)
		return;

	if (false == pLuckySys->IsLuckyTime())
		return;

	/// ���ŵȼ��ж� 
	if (m_pBaseDataMgr.GetPlayerLevel() < m_dwChaseThiefOpenLevel)
		return;

	///���ʼ��
	float random = CRandom::RandRange(1, 100) / 100.0f;
	if (random > m_fChaseThiefActivateRate)
		return;

	_time64(&m_tEffectiveTime);
	ConverTool::ConvertInt64ToBytes(m_tEffectiveTime, m_pChaseThiefData->activateTime);
	m_pChaseThiefData->remainningTime = m_dwChaseThiefDurationTime;
	m_pChaseThiefData->valid = true;
	m_bChaseStarted = false;
	if (nullptr == m_pActivateMsg)
		m_pActivateMsg = make_shared<SThiefActivate>();

	m_pActivateMsg->dwRemainingTime = m_dwChaseThiefDurationTime;
	g_StoreMessage(m_pBaseDataMgr.GetDNID(), m_pActivateMsg.get(), sizeof(SThiefActivate));
}

void CChaseThiefDataManager::LoseEffectiveness()
{
	if (!_ChaseThiefDataIsValid())
		return;

	m_pChaseThiefData->valid = false;
	m_pChaseThiefData->remainningTime = 0;
}

bool CChaseThiefDataManager::CheckEffectiveness()
{
	if (!_ChaseThiefDataIsValid())
		return false;

	if (!m_pChaseThiefData->valid)
		return false;

	__time64_t curtime;
	_time64(&curtime);
	/// ���㵱ǰ���봥��ʱ��ļ��ʱ��
	DWORD intervalTime = (DWORD)difftime(curtime, m_tEffectiveTime);
	/// ����ʣ����Чʱ��
	m_pChaseThiefData->remainningTime = max(m_dwChaseThiefDurationTime - intervalTime, 0);
	/// ������ʱ����ڻ������Чʱ��(�ݶ�1Сʱ),��ʧЧ
	if (intervalTime >= m_dwChaseThiefDurationTime)
	{
		m_pChaseThiefData->valid = false;
		return false;
	}

	return true;
}

bool CChaseThiefDataManager::_ChaseThiefDataIsValid() const
{
	return m_pChaseThiefData ? true : false;
}

void CChaseThiefDataManager::SetChasingHeroes(int heroArr[], int num)
{
	if (nullptr == heroArr)
		return;

	m_vecChasingHeroes.clear();
	try
	{
		for (int i = 0; i < num; ++i)
		{
			if (heroArr[i] == 0)
				break;
			m_vecChasingHeroes.push_back(heroArr[i]);
		}
	}
	catch (...)
	{
		rfalse("����׷���������佫ʱ����");
	}
}

const std::vector<int>& CChaseThiefDataManager::GetChasingHeroes() const
{
	return m_vecChasingHeroes;
}

void CChaseThiefDataManager::RandomReward()
{
	if (m_vecChasingHeroes.empty())
		return;
	
	/// �������� �ȴ������佫�������һλ�佫
	m_iRewardID = m_vecChasingHeroes[CRandom::RandRange(0, m_vecChasingHeroes.size() - 1)];
	/// ����IDΪ�佫ID����90000 �˴�ת���ɽ�������������
	m_iRewardID += 90000;
	/// ��д��������������
	m_iRewardNum = 3;
}

void CChaseThiefDataManager::FillReward(bool win, OUT int& itemID, OUT int& itemNum)
{
	itemID = m_iRewardID;
	itemNum = win ? m_iRewardNum : 1;
}