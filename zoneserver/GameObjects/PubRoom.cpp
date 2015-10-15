#include "StdAfx.h"
#include "PubRoom.h"
#include "player.h"
#include "Networkmodule\SportsMsgs.h"

// Room
CRoom::CRoom()
{
    _dwMasterID = 0; 
}

CRoom::~CRoom()
{
}

DWORD CRoom::GetMasterID(void) const 
{ 
    return _dwMasterID; 
}

void CRoom::SetMasterID(DWORD dwID)  
{ 
    _dwMasterID = dwID; 
}

void CRoom::SetRoomID(DWORD dwRoomID)
{
    _dwRoomID = dwRoomID;
}

DWORD CRoom::GetRoomID(void) const
{
    return _dwRoomID;
}
// RoomContainer
void CRoomContainer::Attach(CRoom *pSubRoom)
{
    if(pSubRoom) 
    {
        pSubRoom->SetRoomID((DWORD)MapSize() + 1);
        _subroomList[pSubRoom->GetRoomID()] = pSubRoom;
    }
}

void CRoomContainer::Detach(CRoom *pSubRoom)
{
    if(NULL == pSubRoom)
        return;                                                                                                                                                                                                                     

    ROOMMAP::iterator it = _subroomList.find(pSubRoom->GetRoomID());
    if(it != _subroomList.end())
    {
        _subroomList.erase(it);
    }
}

void CRoomContainer::Notify(WORD wType)
{
    ROOMMAP::iterator it; 
    for(it = _subroomList.begin(); it != _subroomList.end(); it++)
    {
        if(wType)
        {
            (it->second)->Release();
        }
        else
        {
            (it->second)->Update(this);
        }
    }
}

CRoom *CRoomContainer::Locate(DWORD dwRoomID)
{
    if(dwRoomID == 0)
        return NULL;

    ROOMMAP::iterator it = _subroomList.find(dwRoomID);
    if(it != _subroomList.end())
    {
        return (it->second);
    }

    return NULL;
}

// RoomContainerCenter
#include "roommanager.h"
#include "gaminghouse.h"
RoomContainerCenter *RoomContainerCenter::_pRoomContainerCenter = NULL;
RoomContainerCenter::RoomContainerCenter()
{
}

RoomContainerCenter::~RoomContainerCenter()
{
    if(_pRoomContainerCenter)
    {
        delete _pRoomContainerCenter;
        _pRoomContainerCenter = NULL;
    }
}

RoomContainerCenter *RoomContainerCenter::Instance(void)
{
    if(_pRoomContainerCenter == NULL)
        _pRoomContainerCenter = new RoomContainerCenter;

    return _pRoomContainerCenter;
}

void RoomContainerCenter::DispatchContainerMsg(SSportsBaseMsg *pMsg, CPlayer *pPlayer)
{
    if(pMsg == NULL)
        return;

    if(pPlayer == NULL)
        return;

    switch(pMsg->_protocol)
    {
    case SSportsBaseMsg::EPRO_GAMINGHOUSE:
        _theGHouseManager->DispatchGamingHouseMsg((SGamingHouseBaseMsg *)pMsg, pPlayer);
        break;

    default:
        _theRoomManager->DispatchSportsMsg(pMsg, pPlayer);
    }

}
