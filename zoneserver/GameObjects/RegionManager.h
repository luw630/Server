#pragma once
#include "BaseObject.h"

class CRegionManager :
	public CObjectManager
{
private:
	IObject *CreateObject(const __int32 IID);

public:
    bool Run();     // ��Ҫ���ڶ�ʱ���泡���ϵĽ���������!
    bool Destroy(); // ��Ҫ���ڳ������ʱ���泡���ϵĽ���������!

public:
    CRegionManager();

    friend LPIObject FindRegionByID(DWORD ID);
    friend BOOL FillRegionInfoMsg(struct SARefreshRegionServerMsg &msg, bool bFullRefresh);

    void SaveBuildings();
};
