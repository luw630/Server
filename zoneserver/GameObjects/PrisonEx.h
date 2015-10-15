#pragma once
#include "region.h"

const __int32 IID_PRISONEX = 0x1976d663; 

class CPrisonEx :
    public CRegion
{
public:
    IObject *VDC(const __int32 IID) { if ((IID == IID_REGION) || (IID == IID_PRISONEX) ) return this; return NULL; }
public:
    CPrisonEx(void);
    ~CPrisonEx(void);

public:
    int AddObject(LPIObject pChild);
	int DelObject(LPIObject pChild);
    
    // �������
    void AddPrisoner( DWORD sid, WORD freeTiem );

    // ʩ������
    void DelPrisoner( DWORD sid );

private:
     DWORD runCheckTiem;
protected:
	void OnRun(void);

public:

    // �����б�first:SID second:����ʱ��
    // ���ǵ����ܶ������������ɹ���һ������
    std::map< DWORD, WORD > prisonerMap;
};
