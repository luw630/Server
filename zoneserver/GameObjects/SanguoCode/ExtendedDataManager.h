#pragma once
class CBaseDataManager;
//@brief �������ݹ����Ǹ������࣬�����ڴ���Ĺ����࣬�������������������BaseDataManager�����������
class CExtendedDataManager
{
public:
	CExtendedDataManager(CBaseDataManager& BaseDataMgr)
		:m_pBaseDataMgr(BaseDataMgr)
	{
		;
	}
	~CExtendedDataManager(){ ; }
	virtual bool InitDataMgr(void * pData) = 0;
	virtual bool ReleaseDataMgr() = 0;
	CBaseDataManager &GetBaseDataMgr(){ return m_pBaseDataMgr;}
protected:
	CBaseDataManager  &m_pBaseDataMgr;
};

