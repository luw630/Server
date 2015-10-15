#include "StdAfx.h"
#include "sportsarea.h"
#include "player.h"
#include "item.h"
#include "ScriptManager.h"
#include "Networkmodule\ChatMsgs.h"
#include <time.h>

extern LPIObject GetPlayerByGID(DWORD);
extern BOOL PutPlayerIntoDestRegion(CPlayer *pPlayer, WORD wNewRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID = 0);

CSportsArea::CSportsArea(void)
{
    dwLoopTime = SPORT_TIMES;
    dwStartTime = GetTickCount();
    dwRoomMaster = 0;
    dwState = S_START;
    dwRefInfoTime = timeGetTime();
    m_byPKType = FREE_SIDE;
}

CSportsArea::~CSportsArea(void)
{
}

int CSportsArea::AddObject(LPIObject pChild)
{
    CPlayer *pPlayer = (CPlayer *)pChild->DynamicCast(IID_PLAYER);
    if(pPlayer)
    {
        if(m_byPKType == FREE_FACTION)
        {
            // 
            Bulletin(FormatString("[%s]帮众[%s]进入帮战战场!!!", 
                pPlayer->m_Property.m_szTongName, pPlayer->GetName()));
        }

        switch(pPlayer->GetSportsState()) 
        {
        case 1:
            _listOne.Add(pPlayer->GetGID());
            break;
        case 2:
            _listTwo.Add(pPlayer->GetGID());
            break;

        default:
            {
                DelObject(pChild);
                return 0;
            }
        }
    }
   
    return CDynamicRegion::AddObject(pChild);
}

int CSportsArea::DelObject(LPIObject pChild)
{
    CPlayer *pPlayer = (CPlayer *)pChild->DynamicCast(IID_PLAYER);
    if(pPlayer)
    {
        _listOne.Del(pPlayer->GetGID());
        _listTwo.Del(pPlayer->GetGID());
        if(_listOne.Size() == 0 || _listTwo.Size() == 0)
            dwState = S_END;
    }

    return CDynamicRegion::DelObject(pChild);
}

void CSportsArea::SetLoopTime(DWORD dwTime)
{   
    if(dwTime == 0)
        dwTime = SPORT_TIMES;
    else if(dwTime > 60)
        dwTime = 60;
    
    dwLoopTime = dwTime;
}

BOOL CSportsArea::PutBack(DWORD dwID)
{
    CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwID)->DynamicCast(IID_PLAYER);
    if(pDestPlayer == NULL)
        return FALSE;

    if(m_byPKType == FREE_FACTION)
    {
        return PutPlayerIntoDestRegion(pDestPlayer, pDestPlayer->m_wBackRegionID, 
            pDestPlayer->m_wBackPosX, pDestPlayer->m_wBackPosY);
    }
    else
        return PutPlayerIntoDestRegion(pDestPlayer, m_wReLiveRegionID, (WORD)m_ptReLivePoint.x, (WORD)m_ptReLivePoint.y, 0);

    return FALSE;
}

void CSportsArea::PutTeam(CTSet<DWORD>  &team, BYTE byType )
{
    std::set<DWORD>::iterator it, it_e;
    team.GetItBE(it, it_e);
    
    std::list<DWORD> temp;
    for(; it != it_e; it++)
        temp.push_back(*it);

    std::list<DWORD>::iterator itt = temp.begin();
    while (itt != temp.end())
    {
        PutBack(*itt);
        itt++;
    }
}

void CSportsArea::OnEnd(void)
{
    CSportRoom *pRoom = _theRoomManager->GetSubPtr(dwRoomMaster);
    if(pRoom)
    {
        if(_listOne.Size() == _listTwo.Size())
            pRoom->SetWinner(0);
        else if(_listOne.Size() > _listTwo.Size())
            pRoom->SetWinner(1);
        else if(_listOne.Size() < _listTwo.Size())
            pRoom->SetWinner(2);

        pRoom->OnCancel();
    }

    PutTeam(_listOne);
    PutTeam(_listTwo);

}

void CSportsArea::FillMemberPosInfo(SMPOS *pPosInfo, WORD wTeamID, DWORD dwPlayerGID)
{
//     if(pPosInfo == NULL)
//         return;
// 
//     CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwPlayerGID)->DynamicCast(IID_PLAYER);
//     if(pDestPlayer == NULL)
//         return;
// 
//     WORD wNumber = WORD(_listOne.Size() + _listTwo.Size());
//     if(wNumber > MAX_SMNUMER)
//         wNumber = MAX_SMNUMER;
// 
//     for(WORD n = 0; n < wNumber; n++)
//     {
//         if(pPosInfo[n].wTeamID == 0)
//         {
//             pPosInfo[n].wTeamID = wTeamID;
//             pPosInfo[n].wPosX = pDestPlayer->m_wCurX;
//             pPosInfo[n].wPosY = pDestPlayer->m_wCurY;
//             return;
//         }
//     }
}

void CSportsArea::FillTeamPosInfo(SMPOS *pPosInfo)
{
    if(pPosInfo == NULL)
        return;

    std::set<DWORD>::iterator it, it_e;
  
    _listOne.GetItBE(it, it_e);
    for(; it != it_e; it++ )
    {
        FillMemberPosInfo(pPosInfo, 1, *it);
    }
    _listTwo.GetItBE(it, it_e);
    for(; it != it_e; it++ )
    {
        FillMemberPosInfo(pPosInfo, 2, *it);
    }
}

void CSportsArea::FillPosMsg(SSAMemberPosMsg *pMsg, CTSet<DWORD>  &team)
{
    if(!pMsg)
        return;

    std::set<DWORD>::iterator it, it_e;
  
    team.GetItBE(it, it_e);
    for(; it != it_e; it++ )
    {
        CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(*it)->DynamicCast(IID_PLAYER);
        if(pDestPlayer == NULL)
            continue;

        g_StoreMessage(pDestPlayer->m_ClientIndex, pMsg, pMsg->MySize()); 
    }
 
}

void CSportsArea::SendMemberPos(void)
{
    SSAMemberPosMsg msg;
    ZeroMemory(&msg.stMPos, sizeof(SMPOS)*POS_MAXPLAYERS);
    msg.wNumber = WORD(_listOne.Size() + _listTwo.Size());
    FillTeamPosInfo(&msg.stMPos[0]);
    FillPosMsg(&msg, _listOne);
    FillPosMsg(&msg, _listTwo);
}


BOOL CSportsArea::RefItems(REFITEM &item)
{
//     double dElapsed_time = difftime(time(NULL), (time_t)item.nCountTime);
//     WORD wTime = (int)g_Script.GetGlobalValue("areaconfig_item_reftime");
//     if ( dElapsed_time >= wTime )
//     {
// 
//         CItem::SParameter param;
//         ZeroMemory( &param, sizeof( param ) );
//         param.dwLife = MakeLifeTime( SPORT_TIMES );
//         param.xTile = item.wTileX, param.yTile = item.wTileY;
// 
//         return CItem::GenerateNewGroundItem( this, 0, param, 
//             GenItemParams( item.wID ), LogInfo( -1, "竞技场创建" ) );
//     }

    return FALSE;
}

void CSportsArea::OnRun()
{
  
    if(S_END == dwState)
    {    
        OnEnd();
    }
    else if( ((dwStartTime + (dwLoopTime * 1000 * 60)) <= GetTickCount()) &&
        m_byPKType == FREE_SIDE)
    {
        OnEnd();
    }
    

    if ((int)(timeGetTime() - dwRefInfoTime) > 3000)
    {
        dwRefInfoTime = timeGetTime();
        SendMemberPos();

        if(m_byPKType == FREE_SIDE)
        {
            if(_refItemList.size() > 0)
            {
                REFITEM *pRef = NULL;
                std::list<REFITEM>::iterator it = _refItemList.begin();
                for(; it != _refItemList.end(); it++)
                {
                    pRef = &*it;
                    if(RefItems(*pRef))
                    {
                        _refItemList.erase(it);
                        it = _refItemList.begin();
                    }
                    
                }
            }
        }
    }
    
    CDynamicRegion::Run();
}

void CSportsArea::SetTemporaryItems(WORD wArea)
{
    char szString[65] = {0};
    char szIDStr[65] = {0};
    char szValTimeStr[65] = {0};

    WORD wItemID = 0;
    WORD wItemValTiem = 0;
    sprintf(szString, "areaconfig_item_validtime.%d.1", wArea);
    int num = (int)g_Script.GetTableValue(szString);
    for(int n = 2; n < num + 1;)
    {
        sprintf(szIDStr, "areaconfig_item_validtime.%d.%d.1", wArea, n);
        sprintf(szValTimeStr, "areaconfig_item_validtime.%d.%d.2", wArea, n);
        wItemID = (int)g_Script.GetTableValue(szIDStr);
        wItemValTiem = (int)g_Script.GetTableValue(szValTimeStr);
        _tlistItems[wItemID] = wItemValTiem;
        n++;
    }

}

WORD CSportsArea::GetItemValidTime(WORD wIndex)
{
    WORD wRet = 0;
    std::map<WORD, WORD>::iterator it = _tlistItems.find(wIndex);
    if(it != _tlistItems.end())
    {
        wRet = it->second;
    }
    
    return wRet;
}

void CSportsArea::InserRefItem(WORD wID, WORD wTileX, WORD wTileY)
{
    REFITEM stItem;
    ZeroMemory(&stItem, sizeof(REFITEM));
    stItem.wID = wID;
    stItem.wTileX = wTileX;
    stItem.wTileY = wTileY;
    stItem.nCountTime = time(NULL);

    _refItemList.push_back(stItem);
}

void CSportsArea::Bulletin(LPCSTR lpInfo)
{
    if(lpInfo == NULL)
        return;

    SAChatWisperMsg msg;
    msg.byType = 1;
    dwt::strcpy(msg.szName, "[系统]", sizeof(msg.szName));
    dwt::strcpy(msg.cChatData, lpInfo, sizeof(msg.cChatData));

    Broadcast(&msg, sizeof(SAChatWisperMsg), 1);
}
