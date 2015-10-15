#include "stdafx.h"
#include "GuideManager.h"
#include "../Common/ConfigManager.h"
#include "../../../NETWORKMODULE/SanguoPlayer.h"

CGuideManager::CGuideManager()
	: m_pGuideData(nullptr)
	, m_eCurGuideType(GuideType::Newbie)
{
}


CGuideManager::~CGuideManager()
{
	m_pGuideData = nullptr;
}

bool CGuideManager::InitGuideData(SNewbieGudieData *pData)
{
	if (nullptr == pData)
		return false;

	m_pGuideData = pData;
	/// 解析当前指引类型 存储时此值 = 当前类型 * 1000 + 当前步骤
	m_eCurGuideType = (GuideType)(m_pGuideData->curGuide / 1000);

	return true;
}

bool CGuideManager::SetGuideData(BYTE type, SNewbieGuideDInfo& info)
{
	if (type >= GuideType::MaxCount)
		return false;

	m_eCurGuideType = (GuideType)type;
	auto data = &m_pGuideData->datas[type];
	data->completed = info.completed;
	data->stage = max(data->stage, info.stage);
	/// 此处处理用于提供数据给外部分析
	m_pGuideData->curGuide = type * 1000 + info.stage;

	return true;
}

bool CGuideManager::IsCompleted(GuideType type) const
{
	if (nullptr == m_pGuideData)
		return true;

	return m_pGuideData->datas[(int)type].completed;
}

bool CGuideManager::BeTriggered(GuideType type) const
{
	if (nullptr == m_pGuideData)
		return true;

	return m_pGuideData->datas[(int)type].stage > 0;
}

bool CGuideManager::TriggerRequireHero(int latestTollgateID)
{
	if (nullptr == m_pGuideData)
		return false;

	int type = CConfigManager::getSingleton()->GetGuideType(latestTollgateID);
	if (0 > type || GuideType::MaxCount <= type)
		return false;

	if (m_pGuideData->datas[type].completed || m_pGuideData->datas[type].stage > 0)
		return false;

	_Trigger((GuideType)type);
}

bool CGuideManager::TriggerCollectEquipment()
{
	if (nullptr == m_pGuideData)
		return false;

	if (m_pGuideData->datas[(int)GuideType::CollectEquipment].stage > 0)
		return false;

	_Trigger(GuideType::CollectEquipment);
	return true;
}

bool CGuideManager::ProcessOperation(FunctionMoudleType functionType, int rewardID)
{
	/// 是否比较奖励ID
	bool compareToRewardID = false;
	switch (functionType)
	{
	case FunctionMoudleType::Function_Bless:
		/// 处于新手指引会引导祈福 否则退出
		if (m_eCurGuideType != GuideType::Newbie)
			return false;
		break;
	case FunctionMoudleType::Function_HeroUpgrade:
		/// 处于新手指引引导穿装备 进阶指引引导穿装备合进阶 不满足则退出
		if (m_eCurGuideType != GuideType::Newbie && m_eCurGuideType != GuideType::CollectEquipment)
			return false;
		break;
	case FunctionMoudleType::Function_Achievement:
		if (m_eCurGuideType != GuideType::AcquireGuanFeng && m_eCurGuideType != GuideType::AcquireChengYu && m_eCurGuideType != GuideType::AcquireXiaoQiao)
			return false;
		compareToRewardID = true;
		break;
	case FunctionMoudleType::Function_Duplicate:
		if (m_eCurGuideType != GuideType::Newbie)
			return false;
		compareToRewardID = true;
		break;
	default:
		return false;
	}

	int type = (int)m_eCurGuideType;
	auto config = CConfigManager::getSingleton()->GetGuideConfig(type);
	/// 如果未找到引导配置 或操作步骤值列表为空 或需要匹配奖励ID却匹配不上 则退出
	if (nullptr == config || config->vecOperateIndex.empty() || (compareToRewardID && rewardID != config->rewardID))
		return false;
	auto info = &m_pGuideData->datas[type];
	for (auto stage : config->vecOperateIndex)
	{
		/// 比较操作步骤是否有效
		if (info->stage < stage)
		{
			info->stage = stage;
			m_pGuideData->curGuide = type * 1000 + stage;
			return true;
		}
	}

	return false;
}

void CGuideManager::_Trigger(GuideType type)
{
	m_eCurGuideType = type;
	auto config = CConfigManager::getSingleton()->GetGuideConfig((int)type);
	if (nullptr == config)
		return;

	auto info = &m_pGuideData->datas[(int)type];
	info->completed = false;
	info->stage = config->startIndex;
	m_pGuideData->curGuide = type * 1000 + info->stage;
}
