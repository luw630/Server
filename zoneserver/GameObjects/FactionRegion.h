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
        WORD mapTemplateId;            // ���ɳ����ĵ�ͼģ����
        WORD factionRegionId;          // ���ɳ���Id����Id������Ψһ�ģ���ֻ��16λ����Ҫ��Ϊ�ڵ�½��������ע���ã�
        WORD parentRegionId;           // ���ɳ����ĸ�����Id
        WORD xEntry, yEntry;           // ���ɳ����ڸ������ϵ���ڵ�����
        std::string name;              // ������
    } property;
};
