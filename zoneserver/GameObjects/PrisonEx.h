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
    
    // 添加囚犯
    void AddPrisoner( DWORD sid, WORD freeTiem );

    // 施放囚犯
    void DelPrisoner( DWORD sid );

private:
     DWORD runCheckTiem;
protected:
	void OnRun(void);

public:

    // 囚犯列表，first:SID second:出狱时间
    // 考虑到可能多个监狱，这里可共用一个名单
    std::map< DWORD, WORD > prisonerMap;
};
