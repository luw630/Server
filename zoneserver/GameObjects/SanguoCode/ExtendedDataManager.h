#pragma once
class CBaseDataManager;
//@brief 这里数据管理是个抽象类，派生于此类的管理类，管理的数据区别并依赖与BaseDataManager所管理的数据
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

