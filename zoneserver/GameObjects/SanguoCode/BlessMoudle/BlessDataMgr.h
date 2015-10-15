#pragma once
//#include"..\BaseDataManager.h"
#include"..\ExtendedDataManager.h"
class SBlessData;
class CBaseDataManager;
class CBlessDataMgr : public CExtendedDataManager
{
public:
	CBlessDataMgr(CBaseDataManager& baseDataMgr);
	~CBlessDataMgr();
	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();

	///@brief 是否第一次元宝祈福
	bool IsFirstSingleDiamondBless();
	///@brief 是否第一次祈福
	bool IsFirstDiamondBless();
	///@brief 获取宝石祈福次数
	DWORD GetDiamondBlessCount();
	///@brief 获取免费宝石祈福次数
	DWORD GetDiamondFreeBlessCount();
	///@brief 获取上一次宝石祈福日期
	DWORD GetLastFreeDiamondBlessDate();
	///@brief 第一次祈福完毕
	void DidFirstDiamondBless();
	///@brief 修改第一次元宝祈福
	void CBlessDataMgr::DidFirstSingleDiamondBless();
	///@brief 设置免费宝石祈福次数
	void SetDiamondFreeBlessCount(DWORD count);
	///@brief 设置上一次宝石免费祈福日期
	void SetLastFreeDiamondDate(DWORD date);
	///@brief 设置宝石祈次数
	void ResetDiamondBlessCount();
	///@brief 累加宝石祈福次数
	void AddUpDiamondBlessCount();



	///@brief 是否第一次祈福
	bool IsFirstMonyBless();
	///@brief 获取金钱祈福次数
	DWORD GetMoneyBlessCount();
	///@brief 获取免费金钱祈福次数
	DWORD GetMoneyFreeBlessCount();
	///@brief 获取上一次金钱祈福日期
	DWORD GetLastFreeMoneyBlessDate();
	///@brief 第一次祈福完毕
	void DiaFirstMoneyBless();
	///@brief 设置金钱祈次数
	void ResetMoneyBlessCount();
	///@brief 设置免费金钱祈福次数
	void SetMoneyFreeBlessCount(DWORD count);
	///@brief 设置上一次免费金钱祈福日期
	void SetLastFreeMoneyDate(DWORD date);
	///@累加铜钱祈福次数
	void AddUpMoneyBlessCount();
private:
	SBlessData *m_pBlessData;
};

