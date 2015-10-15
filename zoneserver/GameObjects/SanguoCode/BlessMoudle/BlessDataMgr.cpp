#include "stdafx.h"
#include "BlessDataMgr.h"
#include "../BaseDataManager.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include"..\Common\PubTool.h"
CBlessDataMgr::CBlessDataMgr(CBaseDataManager& baseDataMgr)
	:CExtendedDataManager(baseDataMgr)
{

}


CBlessDataMgr::~CBlessDataMgr()
{
	
}

bool CBlessDataMgr::InitDataMgr(void * pData)
{
	m_pBlessData = static_cast<SBlessData*> (pData);
	INT64 now;
	_time64(&now);
	tm curTime;
	_localtime64_s(&curTime, &now);
	curTime.tm_hour = 0;
	curTime.tm_min = 0;
	curTime.tm_sec = 0;
	if (RefreshJudgement::JudgeCrossed(m_pBaseDataMgr.GetLogoutTime(), curTime))
	{
		m_pBlessData->m_dwMoneyFreeBlessCount = 5;
	}

	return true;
}

bool CBlessDataMgr::ReleaseDataMgr()
{
	m_pBlessData = nullptr;
	return true;
}
///@brief �Ƿ��һ��Ԫ����
bool CBlessDataMgr::IsFirstSingleDiamondBless()
{
	if (m_pBlessData->m_dwIsFirstDiamondBless == 0)
		return true;
	return false;
}

///@brief �Ƿ��һ����
bool CBlessDataMgr::IsFirstDiamondBless()
{
	if (m_pBlessData->m_dwIsFirstDiamondBless == 1)
		return true;
	return false;
}
///@brief ��ȡ��ʯ������
DWORD CBlessDataMgr::GetDiamondBlessCount() { return m_pBlessData->m_dwDiamondblessCount; }
///@brief ��ȡ��ѱ�ʯ������
DWORD CBlessDataMgr::GetDiamondFreeBlessCount() { return m_pBlessData->m_dwDiamondFreeBlessCount; }
///@brief ��ȡ��һ�α�ʯ������
DWORD CBlessDataMgr::GetLastFreeDiamondBlessDate() { return m_pBlessData->m_dwLastDiamondFreeBlessDate; }
///@brief ��һ�������
void CBlessDataMgr::DidFirstDiamondBless(){ m_pBlessData->m_dwIsFirstDiamondBless = 0; }
///@brief ��һ�������
void CBlessDataMgr::DidFirstSingleDiamondBless(){ m_pBlessData->m_dwIsFirstDiamondBless = -1; }
///@brief ������ѱ�ʯ������
void CBlessDataMgr::SetDiamondFreeBlessCount(DWORD count) { m_pBlessData->m_dwDiamondFreeBlessCount = count; }
///@brief ������һ����ѱ�ʯ������
void CBlessDataMgr::SetLastFreeDiamondDate(DWORD date) { m_pBlessData->m_dwLastDiamondFreeBlessDate = date; }
///@brief ���ñ�ʯ�����
void CBlessDataMgr::ResetDiamondBlessCount() { m_pBlessData->m_dwDiamondblessCount = 0; }
///@brief �ۼӱ�ʯ������
void CBlessDataMgr::AddUpDiamondBlessCount() { ++ m_pBlessData->m_dwDiamondblessCount; }
///@brief �Ƿ��һ����
bool CBlessDataMgr::IsFirstMonyBless()
{
	if (m_pBlessData->m_dwIsFirstMoneyBless == 0)
		return false;
	return true;
}
///@brief ��ȡ��Ǯ������
DWORD CBlessDataMgr::GetMoneyBlessCount() { return m_pBlessData->m_dwMoneyBlessCount; }
///@brief ��ȡ��ѽ�Ǯ������
DWORD CBlessDataMgr::GetMoneyFreeBlessCount() { return m_pBlessData->m_dwMoneyFreeBlessCount; }
///@brief ��ȡ��һ�ν�Ǯ������
DWORD CBlessDataMgr::GetLastFreeMoneyBlessDate() { return m_pBlessData->m_dwLastMoneyFreeBlessDate; }
///@brief ��һ�������
void CBlessDataMgr::DiaFirstMoneyBless() { m_pBlessData->m_dwIsFirstMoneyBless = 0; }
///@brief ���ý�Ǯ�����
void CBlessDataMgr::ResetMoneyBlessCount() { m_pBlessData->m_dwMoneyBlessCount = 0; }
///@brief ������ѽ�Ǯ������
void CBlessDataMgr::SetMoneyFreeBlessCount(DWORD count) { m_pBlessData->m_dwMoneyFreeBlessCount = count; }
///@brief ������һ�ν�Ǯ������
void CBlessDataMgr::SetLastFreeMoneyDate(DWORD date) { m_pBlessData->m_dwLastMoneyFreeBlessDate = date; }
///@brief �ۼӽ�Ǯ����
void CBlessDataMgr::AddUpMoneyBlessCount() { ++ (m_pBlessData->m_dwMoneyBlessCount) ; }