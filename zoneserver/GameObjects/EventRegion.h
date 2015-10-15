#pragma once
#include "dynamicregion.h"

const __int32 IID_EVENTREGION = 0x19eaa992; 
///////////////////////////////////////////////////////////////////////////////
class CEventRegion :  public CDynamicRegion
{
public:
    IObject *VDC(const __int32 IID) { return (IID == IID_EVENTREGION) ? this : CDynamicRegion::VDC(IID); }

public:
    CEventRegion(void);
    ~CEventRegion(void);

protected:
	void OnRun(void);

protected:
	DWORD	m_dwRegionStartTime;	//副本开始的时间
	DWORD	m_dwRegionFinishTime;		//副本退出的起始时间

public:
	std::set<DWORD> m_AIMonster;		
	int		m_nEventRegionEnd;		//副本结束标志，=0:副本运行中，=1:成功结束，=2:失败结束

};
///////////////////////////////////////////////////////////////////////////////
