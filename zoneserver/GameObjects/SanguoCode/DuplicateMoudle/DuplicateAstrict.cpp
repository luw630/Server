#include "stdafx.h"
#include "DuplicateAstrict.h"
#include "..\BaseDataManager.h"
#include "NETWORKMODULE\SanguoPlayer.h"
#include "..\Common\SanguoConfigTypeDef.h"


CDuplicateAstrict::CDuplicateAstrict()
{
	m_iOneDayChanllengeCount = 0;   //每日可挑战的次数 <0表示无限制//
	m_iChanllengedTimes = 0;   //已被挑战的次数//
	m_fCurCD = 0.0f;         //下次可挑战的CD,暂时无用//
}


CDuplicateAstrict::~CDuplicateAstrict()
{
}

void CDuplicateAstrict::Init(const ChapterConfig* config)
{
	if (config == nullptr)
	{
		rfalse("副本挑战限制模块Init时，传进来的ChapterConfig为空");
		return;
	}

	m_iOneDayChanllengeCount = config->ticketNum;
}

void CDuplicateAstrict::UpdateCD()
{
	m_fCurCD = 0;
}

void CDuplicateAstrict::Resolves(int clearTime /*= 1*/)
{
	m_iChanllengedTimes += clearTime;
}

void CDuplicateAstrict::Action()
{
	m_fCurCD = 0;
}

CheckResult CDuplicateAstrict::Check(const CBaseDataManager& dataManger, int clearTime /*= 1*/)
{
	if (m_iOneDayChanllengeCount == 0 || (m_iOneDayChanllengeCount > 0 && clearTime > m_iOneDayChanllengeCount - m_iChanllengedTimes))
		return CheckResult::NoEnoughTimes;

	if (m_fCurCD > 0.0f)
		return CheckResult::NoEnoughCD;

	return CheckResult::Pass;
}

int CDuplicateAstrict::GetCurChallengeNum()
{
	return m_iChanllengedTimes;
}

void CDuplicateAstrict::SetCurChallengeNum(int num)
{
	if (num < 0)
		return;

	m_iChanllengedTimes = num;
}

void CDuplicateAstrict::ReSetCurTicketNum(int num)
{
	m_iChanllengedTimes = 0;

	if (num < 0)
		return;

	m_iOneDayChanllengeCount = num;
}
