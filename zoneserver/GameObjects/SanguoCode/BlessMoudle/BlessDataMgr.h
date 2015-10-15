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

	///@brief �Ƿ��һ��Ԫ����
	bool IsFirstSingleDiamondBless();
	///@brief �Ƿ��һ����
	bool IsFirstDiamondBless();
	///@brief ��ȡ��ʯ������
	DWORD GetDiamondBlessCount();
	///@brief ��ȡ��ѱ�ʯ������
	DWORD GetDiamondFreeBlessCount();
	///@brief ��ȡ��һ�α�ʯ������
	DWORD GetLastFreeDiamondBlessDate();
	///@brief ��һ�������
	void DidFirstDiamondBless();
	///@brief �޸ĵ�һ��Ԫ����
	void CBlessDataMgr::DidFirstSingleDiamondBless();
	///@brief ������ѱ�ʯ������
	void SetDiamondFreeBlessCount(DWORD count);
	///@brief ������һ�α�ʯ���������
	void SetLastFreeDiamondDate(DWORD date);
	///@brief ���ñ�ʯ�����
	void ResetDiamondBlessCount();
	///@brief �ۼӱ�ʯ������
	void AddUpDiamondBlessCount();



	///@brief �Ƿ��һ����
	bool IsFirstMonyBless();
	///@brief ��ȡ��Ǯ������
	DWORD GetMoneyBlessCount();
	///@brief ��ȡ��ѽ�Ǯ������
	DWORD GetMoneyFreeBlessCount();
	///@brief ��ȡ��һ�ν�Ǯ������
	DWORD GetLastFreeMoneyBlessDate();
	///@brief ��һ�������
	void DiaFirstMoneyBless();
	///@brief ���ý�Ǯ�����
	void ResetMoneyBlessCount();
	///@brief ������ѽ�Ǯ������
	void SetMoneyFreeBlessCount(DWORD count);
	///@brief ������һ����ѽ�Ǯ������
	void SetLastFreeMoneyDate(DWORD date);
	///@�ۼ�ͭǮ������
	void AddUpMoneyBlessCount();
private:
	SBlessData *m_pBlessData;
};

