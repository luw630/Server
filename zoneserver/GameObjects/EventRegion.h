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
	DWORD	m_dwRegionStartTime;	//������ʼ��ʱ��
	DWORD	m_dwRegionFinishTime;		//�����˳�����ʼʱ��

public:
	std::set<DWORD> m_AIMonster;		
	int		m_nEventRegionEnd;		//����������־��=0:���������У�=1:�ɹ�������=2:ʧ�ܽ���

};
///////////////////////////////////////////////////////////////////////////////
