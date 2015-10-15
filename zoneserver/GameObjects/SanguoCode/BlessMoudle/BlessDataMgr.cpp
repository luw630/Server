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
///@brief 是否第一次元宝祈福
bool CBlessDataMgr::IsFirstSingleDiamondBless()
{
	if (m_pBlessData->m_dwIsFirstDiamondBless == 0)
		return true;
	return false;
}

///@brief 是否第一次祈福
bool CBlessDataMgr::IsFirstDiamondBless()
{
	if (m_pBlessData->m_dwIsFirstDiamondBless == 1)
		return true;
	return false;
}
///@brief 获取宝石祈福次数
DWORD CBlessDataMgr::GetDiamondBlessCount() { return m_pBlessData->m_dwDiamondblessCount; }
///@brief 获取免费宝石祈福次数
DWORD CBlessDataMgr::GetDiamondFreeBlessCount() { return m_pBlessData->m_dwDiamondFreeBlessCount; }
///@brief 获取上一次宝石祈福日期
DWORD CBlessDataMgr::GetLastFreeDiamondBlessDate() { return m_pBlessData->m_dwLastDiamondFreeBlessDate; }
///@brief 第一次祈福完毕
void CBlessDataMgr::DidFirstDiamondBless(){ m_pBlessData->m_dwIsFirstDiamondBless = 0; }
///@brief 第一次祈福完毕
void CBlessDataMgr::DidFirstSingleDiamondBless(){ m_pBlessData->m_dwIsFirstDiamondBless = -1; }
///@brief 设置免费宝石祈福次数
void CBlessDataMgr::SetDiamondFreeBlessCount(DWORD count) { m_pBlessData->m_dwDiamondFreeBlessCount = count; }
///@brief 设置上一次免费宝石祈福日期
void CBlessDataMgr::SetLastFreeDiamondDate(DWORD date) { m_pBlessData->m_dwLastDiamondFreeBlessDate = date; }
///@brief 设置宝石祈次数
void CBlessDataMgr::ResetDiamondBlessCount() { m_pBlessData->m_dwDiamondblessCount = 0; }
///@brief 累加宝石祈福次数
void CBlessDataMgr::AddUpDiamondBlessCount() { ++ m_pBlessData->m_dwDiamondblessCount; }
///@brief 是否第一次祈福
bool CBlessDataMgr::IsFirstMonyBless()
{
	if (m_pBlessData->m_dwIsFirstMoneyBless == 0)
		return false;
	return true;
}
///@brief 获取金钱祈福次数
DWORD CBlessDataMgr::GetMoneyBlessCount() { return m_pBlessData->m_dwMoneyBlessCount; }
///@brief 获取免费金钱祈福次数
DWORD CBlessDataMgr::GetMoneyFreeBlessCount() { return m_pBlessData->m_dwMoneyFreeBlessCount; }
///@brief 获取上一次金钱祈福日期
DWORD CBlessDataMgr::GetLastFreeMoneyBlessDate() { return m_pBlessData->m_dwLastMoneyFreeBlessDate; }
///@brief 第一次祈福完毕
void CBlessDataMgr::DiaFirstMoneyBless() { m_pBlessData->m_dwIsFirstMoneyBless = 0; }
///@brief 重置金钱祈次数
void CBlessDataMgr::ResetMoneyBlessCount() { m_pBlessData->m_dwMoneyBlessCount = 0; }
///@brief 设置免费金钱祈福次数
void CBlessDataMgr::SetMoneyFreeBlessCount(DWORD count) { m_pBlessData->m_dwMoneyFreeBlessCount = count; }
///@brief 设置上一次金钱祈福日期
void CBlessDataMgr::SetLastFreeMoneyDate(DWORD date) { m_pBlessData->m_dwLastMoneyFreeBlessDate = date; }
///@brief 累加金钱次数
void CBlessDataMgr::AddUpMoneyBlessCount() { ++ (m_pBlessData->m_dwMoneyBlessCount) ; }