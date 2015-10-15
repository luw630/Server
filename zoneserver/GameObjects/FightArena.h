#pragma once
#include "region.h"

const __int32 IID_FIGHTARENA = 0x188b09c9; 

class CFightArena :
    public CRegion
{
public:
    IObject *VDC(const __int32 IID) { if ((IID == IID_REGION) || (IID == IID_FIGHTARENA) ) return this; return NULL; }

public:
	int AddObject(LPIObject pChild);
	int DelObject(LPIObject pChild);
    void OnClose();

    void OnKill(CFightObject *PKiller, CFightObject *PDead);

protected:
	void OnRun(void);
	int OnCreate(_W64 long pParameter); // 这里传入的只是一个ID号，根据具体的ID载入相应的场景

public:
    CFightArena(void);
    ~CFightArena(void);
};
