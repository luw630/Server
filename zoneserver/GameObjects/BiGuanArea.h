#pragma once
#include "region.h"

const __int32 IID_BIGUANAREA = 0x1982a578; 

class CBiGuanArea :
    public CRegion
{
public:
    IObject *VDC(const __int32 IID) { if ((IID == IID_REGION) || (IID == IID_BIGUANAREA) ) return this; return NULL; }

/*
public:
	int AddObject(LPIObject pChild);
	int DelObject(LPIObject pChild);

protected:
	void OnRun(void);
*/

protected:
	int OnCreate(_W64 long pParameter);

public:
    CBiGuanArea(void);
    ~CBiGuanArea(void);
};

