#pragma once
#include "BaseObject.h"

class CRegionManager :
	public CObjectManager
{
private:
	IObject *CreateObject(const __int32 IID);

public:
    bool Run();     // 主要用于定时保存场景上的建筑物数据!
    bool Destroy(); // 主要用于程序结束时保存场景上的建筑物数据!

public:
    CRegionManager();

    friend LPIObject FindRegionByID(DWORD ID);
    friend BOOL FillRegionInfoMsg(struct SARefreshRegionServerMsg &msg, bool bFullRefresh);

    void SaveBuildings();
};
