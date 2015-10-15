#include "stdafx.h"
#include "DuplicateAstrict.h"
#include "..\BaseDataManager.h"
#include "NETWORKMODULE\SanguoPlayer.h"
#include "..\Common\SanguoConfigTypeDef.h"


CDuplicateAstrict::CDuplicateAstrict()
{
	m_iOneDayChanllengeCount = 0;   //ÿ�տ���ս�Ĵ��� <0��ʾ������//
	m_iChanllengedTimes = 0;   //�ѱ���ս�Ĵ���//
	m_fCurCD = 0.0f;         //�´ο���ս��CD,��ʱ����//
}


CDuplicateAstrict::~CDuplicateAstrict()
{
}

void CDuplicateAstrict::Init(const ChapterConfig* config)
{
	if (config == nullptr)
	{
		rfalse("������ս����ģ��Initʱ����������ChapterConfigΪ��");
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
