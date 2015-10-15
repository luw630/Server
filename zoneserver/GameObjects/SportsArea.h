#pragma once

#include "dynamicregion.h"
#include "roommanager.h"

const __int32 IID_SPORTSAREA = 0x1a70a54d;

class CSportsArea : public CDynamicRegion
{
public:
    IObject *VDC(const __int32 IID) { return (IID == IID_SPORTSAREA) ? this : CDynamicRegion::VDC(IID); }

public:
    CSportsArea(void);
    ~CSportsArea(void);
    
    int AddObject(LPIObject pChild);
	int DelObject(LPIObject pChild);

    void SetLoopTime(DWORD dwTime);
    void SetMaster(DWORD dwID) { dwRoomMaster = dwID; }
    void SetTemporaryItems(WORD wArea);
    WORD GetItemValidTime(WORD wIndex);
    void InserRefItem(WORD wID, WORD wTileX, WORD wTileY);
    void Bulletin(LPCSTR lpInfo);

    BYTE    m_byPKType;
protected:
    void  OnRun(void);
    void  OnEnd(void);
    BOOL PutBack(DWORD dwID);
    void  PutTeam(CTSet<DWORD>  &team, BYTE byType = 0);
    void  FillMemberPosInfo(SMPOS *pPosInfo, WORD wTeamID, DWORD dwPlayerGID );
    void  FillTeamPosInfo(SMPOS *pPosInfo);
    void  FillPosMsg(struct SSAMemberPosMsg *pMsg, CTSet<DWORD>  &team);
    void  SendMemberPos(void);


    CTSet<DWORD> _listOne;
    CTSet<DWORD> _listTwo;
    std::map<WORD, WORD> _tlistItems;  // temporary items

    struct REFITEM {
        WORD wID;
        WORD wTileX;
        WORD wTileY;
        __int64 nCountTime;
    };

    BOOL  RefItems(REFITEM &item);
    std::list<REFITEM> _refItemList;
    enum { S_START, S_END };
    DWORD dwLoopTime;
    DWORD dwStartTime;
    DWORD dwRoomMaster;
    DWORD dwState;
    DWORD dwRefInfoTime;

};
