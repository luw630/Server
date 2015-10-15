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
	/// 如果还处于有效期 则不再次启动
	if (CheckEffectiveness())
		return;

	if (!_ChaseThiefDataIsValid())
		return;

	/// 判断是否是黑夜
	CLuckyTimeSystem* pLuckySys = nullptr; 
	GetGW()->GetLuckySystemPtr(&pLuckySys);
	if (nullptr == pLuckySys)
		return;

	if (false == pLuckySys->IsLuckyTime())
		return;

	/// 开放等级判定 
	if (m_pBaseDataMgr.GetPlayerLevel() < m_dwChaseThiefOpenLevel)
		return;

	///几率检测
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
	/// 计算当前距离触发时间的间隔时间
	DWORD intervalTime = (DWORD)difftime(curtime, m_tEffectiveTime);
	/// 计算剩余有效时间
	m_pChaseThiefData->remainningTime = max(m_dwChaseThiefDurationTime - intervalTime, 0);
	/// 如果间隔时间大于或等于有效时间(暂定1小时),则失效
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
		rfalse("设置追击盗贼的武将时出错");
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
	
	/// 奖励将魂 先从上阵武将中随机出一位武将
	m_iRewardID = m_vecChasingHeroes[CRandom::RandRange(0, m_vecChasingHeroes.size() - 1)];
	/// 将魂ID为武将ID加上90000 此处转换成将魂先这样处理
	m_iRewardID += 90000;
	/// 暂写死奖励三个将魂
	m_iRewardNum = 3;
}

void CChaseThiefDataManager::FillReward(bool win, OUT int& itemID, OUT int& itemNum)
{
	itemID = m_iRewardID;
	itemNum = win ? m_iRewardNum : 1;
}