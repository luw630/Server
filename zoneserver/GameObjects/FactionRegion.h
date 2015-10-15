#pragma once
#include "region.h"
#include "npc.h"

const __int32 IID_FACTIONREGION = 0x1a0cee57; 

class CFactionRegion :
    public CRegion
{
public:
    IObject *VDC(const __int32 IID) { return (IID == IID_FACTIONREGION) ? this : CRegion::VDC(IID); }

public:
    int OnCreate(_W64 long pParameter);
	int AddObject(LPIObject pChild);
    int DelObject(LPIObject pChild);

	/*virtual*/ CNpc        *CreateNpc( PVOID pParameter, DWORD controlId = 0 );
	/*virtual*/ CBuilding   *CreateBuilding( PVOID pParameter, CPlayer *pPlayer, BOOL isGenerate = FALSE, DWORD controlId = 0 );

	/*virtual*/ int			DelNpc( DWORD dwGID );
    /*virtual*/ int			DelBuilding( DWORD dwGID ); 

public:
    CFactionRegion(void);
    ~CFactionRegion(void);

public:
    struct CreateParameter
    {
        WORD mapTemplateId;            // 帮派场景的地图模版编号
        WORD factionRegionId;          // 帮派场景Id（该Id是世界唯一的，且只有16位，主要做为在登陆服务器上注册用）
        WORD parentRegionId;           // 帮派场景的父场景Id
        WORD xEntry, yEntry;           // 帮派场景在父场景上的入口点坐标
        std::string name;              // 帮派名
    } property;
};
