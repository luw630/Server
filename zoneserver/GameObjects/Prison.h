#pragma once
#include "region.h"

const __int32 IID_PRISON = 0x1976d662; 

class CPrison :
    public CRegion
{
public:
    IObject *VDC(const __int32 IID) { if ((IID == IID_REGION) || (IID == IID_PRISON) ) return this; return NULL; }

public:
	int AddObject(LPIObject pChild);
	int DelObject(LPIObject pChild);

protected:
	void OnRun(void);

public:
    CPrison(void);
    ~CPrison(void);
};
