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
	int OnCreate(_W64 long pParameter); // ���ﴫ���ֻ��һ��ID�ţ����ݾ����ID������Ӧ�ĳ���

public:
    CFightArena(void);
    ~CFightArena(void);
};
