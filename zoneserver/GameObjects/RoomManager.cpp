#include "StdAfx.h"
#include "roommanager.h"
#include "player.h"
#include "sportsarea.h"
#include "prison.h"
#include "Networkmodule\ScriptMsgs.h"
#include "item.h"
#include "ScriptManager.h"
#include "factionmanager.h"

#include <map>
#include <time.h>
extern LPIObject GetPlayerByGID(DWORD);
extern DWORD CreateDynamicRegion(const __int32 DR_IID, WORD RegionIndex, int RegionID, int, int, int, DWORD,WORD);
extern BOOL PutPlayerIntoDestRegion(CPlayer *pPlayer, WORD wNewRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID = 0);

CRoomManager *CRoomManager::_instance = NULL;
CRoomManager::CRoomManager(void)
{
    _Count = 0;
}

CRoomManager::~CRoomManager(void)
{
    if(_instance)
    {
        delete _instance;
        _instance = NULL;
    }
}

void CRoomManager::CheckCount()
{
    if(_Count > CHECK_COUNT)
    {
        double dElapsed_time = 0; 
        std::map<DWORD, CSportRoom>::iterator it = _list.begin();

        for(; it != _list.end(); it++)
        {
            dElapsed_time = difftime(time(NULL), (time_t)it->second.nCreateTime);
            if(dElapsed_time > (3600*24))
            {
                _list.erase(it);
                it = _list.begin();
            }
        }
  
        _Count = 0;
    }
}

void CRoomManager::Attach(CSportRoom* pSubRoom)
{
    if(pSubRoom)
    {
        pSubRoom->nCreateTime = time(NULL);
        _list[pSubRoom->GetMasterID()] = *pSubRoom;
        _Count++;
    }

}

CRoomManager *CRoomManager::Instance(void)
{
    if(_instance == NULL)
        _instance = new CRoomManager;

    return _instance;
}

void CRoomManager::Detach(CSportRoom* pSubRoom)
{
    if(pSubRoom)
    {
        std::map<DWORD, CSportRoom>::iterator it = _list.find(pSubRoom->GetMasterID());
        if(it != _list.end())
        {
            _list.erase(it);
            _Count--;
        }
    }
}

void CRoomManager::Detach(DWORD  masterID)
{

    std::map<DWORD, CSportRoom>::iterator it = _list.find(masterID);
    if(it != _list.end())
    {
        _list.erase(it);
        _Count--;
    }

}

CSportRoom *CRoomManager::GetSubPtr(DWORD dwSubID)
{
    CSportRoom *pRoom = NULL;
    std::map<DWORD, CSportRoom>::iterator it = _list.find(dwSubID);
    if(it != _list.end())
    {
        pRoom = &it->second;
    }

    return pRoom;
}

bool CRoomManager::CheckAskState(DWORD dwScrID, DWORD dwDesID, WORD wAskState)
{
//     CPlayer *pSrcPlayer = (CPlayer *)GetPlayerByGID(dwScrID)->DynamicCast(IID_PLAYER);
//     if(pSrcPlayer == NULL)
//         return false;  
// 
//     if (pSrcPlayer->m_byLevel < 10)				
// 	{
// 		TalkToDnid(pSrcPlayer->m_ClientIndex, "修为在10级以下不能切磋！");
// 		return false;
// 	}
// 
//     CPlayer *pDesPlayer = (CPlayer *)GetPlayerByGID(dwDesID)->DynamicCast(IID_PLAYER);
//     if(pDesPlayer == NULL)
//         return false; 
// 
//     if (pDesPlayer->m_byLevel < 10)				
// 	{
// 		TalkToDnid(pDesPlayer->m_ClientIndex, "修为在10级以下不能切磋！");
// 		return false;
// 	}
// 
//     if(pSrcPlayer->GetSportsRoomID())
//         return false;
// 
//     CSportRoom *pRoom = GetSubPtr(dwScrID);
//     if(pRoom)
//     {
//         if(pSrcPlayer->GetSportsState() == 0)
//         {
//             double dElapsed_time = 0;             
//             dElapsed_time = difftime(time(NULL), (time_t)pRoom->nCreateTime);
//             if(dElapsed_time > (60*SPORT_TIMES/2))
//             {
//                 //long time, restart
//                 pRoom->OnCancel();
//                 return true;
//             }
//             else
//             {
//                 //answering 
//                 
//             }
//         }
//         return false;
//     }
// 
//     if(GetSubPtr(dwDesID) || 
//         pDesPlayer->GetSportsRoomID())
//     {
//         if(wAskState == AS_JOIN)
//             return true;
// 
//         return false;
//     }

    return true;
}

void CRoomManager::RecvJoinSideMsg(SQJoinSideMsg *pMsg)
{
    if(!CheckAskState(pMsg->dwSrcGID, pMsg->dwDesGID, AS_JOIN))
        return;

    CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(pMsg->dwDesGID)->DynamicCast(IID_PLAYER);
    if(pDestPlayer == NULL)
        return;   

    CSportRoom *pRoom = GetSubPtr(pDestPlayer->GetSportsRoomID());
    if(pRoom)
    {
        pRoom->RecvJoinSideMsg(pMsg);
    }

}

// 帮派相关
//--------------------------------------------------------------------------------
bool CRoomManager::CheckFaction(DWORD dwSrcID, bool bCheckMoney)
{
    return false;
}
//--------------------------------------------------------------------------------
void CRoomManager::DispatchSportsMsg(SSportsBaseMsg *pMsg, CPlayer *pPlayer)
{
    if (pPlayer == NULL)
        return;

    switch(pMsg->_protocol)
    {
    case SSportsBaseMsg::EPRO_ASKCREATEROOM:
        {
			//切磋功能关闭 2006-10-27
			return;
       // 监狱里边不能切磋
            if (pPlayer->m_ParentRegion->DynamicCast(IID_PRISON) != NULL)
                return;

            CheckCount();
            SQAskCreateRoomMsg *pCreateMsg = (SQAskCreateRoomMsg *)pMsg;
            if(pCreateMsg->byType == FREE_SIDE || pCreateMsg->byType == FREE_FACTION)
            {
                if(!CheckAskState(pCreateMsg->dwSrcGID, pCreateMsg->dwDesGID))
                    return;

                // 帮战创建检查
                if(FREE_FACTION == pCreateMsg->byType)
                {
                    if(!CheckFaction(pCreateMsg->dwSrcGID, true))
                        return;

                    if(!CheckFaction(pCreateMsg->dwDesGID))
                        return;
                }

                CSportRoom *pNewRoom = new CSportRoom(_theRoomManager, pCreateMsg->dwSrcGID);
                CSportRoom *pRoom = GetSubPtr(pCreateMsg->dwSrcGID);
                if(pRoom)
                {
                    pRoom->SetSideSate(1,pCreateMsg->dwSrcGID,CSportRoom::T_TALK);  
                    pRoom->SetSideSate(2,pCreateMsg->dwDesGID,CSportRoom::T_TALK);
                    pRoom->byRoomType = pCreateMsg->byType;
                    pRoom->SendAskMsg();
                }
                delete pNewRoom;
                pNewRoom = NULL;
            }
        }break;
    case SSportsBaseMsg::EPRO_ANSWERROOM:
        {
            SQAnswerRoomMsg *pAnswerMsg = (SQAnswerRoomMsg *)pMsg;
            CSportRoom *pRoom = GetSubPtr(pAnswerMsg->dwMasterGID);
            if(pRoom)
            {
                pRoom->RecvAnswerMsg(pAnswerMsg);
            }

        }break;
    case SSportsBaseMsg::EPRO_SENDTERM:
        {        
            SIDE_TERM Term = ((SQSendTermMsg *)pMsg)->stTerm;
            CSportRoom *pRoom = GetSubPtr(((SQSendTermMsg *)pMsg)->dwMasterGID); 
            if(pRoom)
            {
                pRoom->RecvTermInfo(&Term, ((SQSendTermMsg *)pMsg)->dwOtherGID);
            }
             
        }break;
    case SSportsBaseMsg::EPRO_JOINSIDE:
        {
            SQJoinSideMsg *pJoinSide = (SQJoinSideMsg *)pMsg;
            RecvJoinSideMsg(pJoinSide);
            
        }break;
    case SSportsBaseMsg::EPRO_KICKOUT:
        {
            CSportRoom *pRoom = GetSubPtr(pPlayer->GetSportsRoomID());
            if(pRoom)
            {
                pRoom->RecvDelMember(pPlayer, ((SQKickoutMsg *)pMsg)->szName);
            }
            
        }break;

    default:
        return;

    }
}

CSportRoom::CSportRoom(CRoomManager *pPubFrame, DWORD dwMasterID)
{
    _pManager = NULL;
    memset(&_roomMaster, 0, sizeof(PSTATE));
    memset(&_roomSubMaster, 0, sizeof(PSTATE));
    memset(&term, 0, sizeof(SIDE_TERM));
    term.bHaveGoods = true;
    dwRegionGID = 0;
    nCreateTime = 0;
    byRoomType = FREE_SIDE;

    if(pPubFrame)
    {
        _pManager = pPubFrame;
        this->SetMasterID(dwMasterID);
        _pManager->Attach(this);
    }

}

CSportRoom::~CSportRoom()
{
   // OnCancel();
}

void CSportRoom::SendPInfo(DWORD dwGID)
{
    CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
    if(pDestPlayer == NULL)
        return;

    SASendTeamMsg msg;
    ZeroMemory(&msg.stTeam, sizeof(SMatchMember)*MAX_SMNUMER);
    std::map<ustring, PINFO>::iterator it  = _MemberInfo.begin();
    for(; it != _MemberInfo.end(); )
    {
        for(int n = 0; n < MAX_SMNUMER; n ++)
        {
            if(msg.stTeam[n].m_szName[0] == '\0')
            {
				strncpy(msg.stTeam[n].m_szName, it->first, CONST_USERNAME);
                msg.wState = SASendTeamMsg::M_ALL;
                msg.stTeam[n].byTeamID =(BYTE) (it->second).wTeamID;
                msg.stTeam[n].m_wLevel = (it->second).wLevel;
                break;
            }
        }

        it++;
    }

    g_StoreMessage(pDestPlayer->m_ClientIndex, &msg, sizeof(SASendTeamMsg));
}

void CSportRoom::SendPInfoMsg(CTSet<DWORD> &set, SASendTeamMsg &msg)
{
    std::set<DWORD>::iterator it, it_e;
    set.GetItBE(it, it_e);
    for(; it != it_e; )
    {
        CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(*it)->DynamicCast(IID_PLAYER);
        if(pDestPlayer)
        {
            g_StoreMessage(pDestPlayer->m_ClientIndex, &msg, sizeof(SASendTeamMsg) - sizeof(SMatchMember)*15);
        }
        it++;
    }

}

void CSportRoom::SendPInfoToAll(DWORD dwGID, WORD wTeam, WORD wState)
{
//     CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
//     if(pDestPlayer == NULL)
//         return;
// 
//     SASendTeamMsg msg;
//     msg.wState = wState;
//     strncpy(msg.stTeam[0].m_szName, pDestPlayer->GetName(), CONST_USERNAME);
//     msg.stTeam[0].byTeamID = (BYTE)wTeam;
//     msg.stTeam[0].m_wLevel = pDestPlayer->m_Property.m_Level;
// 
//     SendPInfoMsg(_listOne, msg);
//     SendPInfoMsg(_listTwo, msg);
}

void CSportRoom::OperatePInfo(DWORD dwGID, WORD byTeam, WORD wOperate)
{
//     CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
//     if(pDestPlayer == NULL)
//         return;
// 
//     if(wOperate == 0)
//     {   
//         std::map<ustring, PINFO>::iterator it  = _MemberInfo.find(pDestPlayer->GetName());
//         if(it != _MemberInfo.end())
//         {
//             _MemberInfo.erase(it);
//             SendPInfoToAll(dwGID, byTeam, SASendTeamMsg::M_DELONE); 
//             SendAnswerMsg(SAAnswerRoomMsg::ST_TERM_CANCEL, dwGID);
//             SetPState(dwGID, 0, 0);
//         }
//     }
//     else if(wOperate == 1)
//     {
//         PINFO info;
//         info.wTeamID = byTeam;
//         info.wLevel = pDestPlayer->m_Property.m_Level;
//         info.dwGID = dwGID;
//         _MemberInfo[pDestPlayer->GetName()] = info;
//         SendPInfo(dwGID);
//         SendPInfoToAll(dwGID, byTeam, SASendTeamMsg::M_ONE);
//     }

}

bool CSportRoom::Add(DWORD wID, BYTE bySide)
{
    switch(bySide)
    {
    case 1:
        {
            if(_listOne.Add(wID))
            {
                OperatePInfo(wID, 1, 1);
                return true;
            }
        }break;

    case 2:
        {
            if(_listTwo.Add(wID))
            {
                OperatePInfo(wID, 2, 1);
                return true;
            }
        }break;

    default:
        break;
    }

    return false;

}

bool CSportRoom::Del(DWORD wID, BYTE bySide)
{
   switch(bySide)
    {
    case 1:
        _listOne.Del(wID);
         OperatePInfo(wID, 1, 0);  
        break;
    case 2:
        _listTwo.Del(wID);
         OperatePInfo(wID, 2, 0);
        break;

    default:
        return false;
    }

    return true;
}

void CSportRoom::SetSideSate(BYTE byWhich, DWORD dwGID, DWORD dwState)
{
    P_STATE state;
    state.dwGID = dwGID;
    state.dwState = dwState;

    switch(byWhich)
    {
    case 1:
        _roomMaster = state;
        _listOne.Add(state.dwGID);
        SetPState(_roomMaster.dwGID, 0, _roomMaster.dwGID);
        break;
    case 2:
        _roomSubMaster = state;
        _listTwo.Add(state.dwGID);
        SetPState(_roomSubMaster.dwGID, 0, _roomMaster.dwGID);
        break;

    default:
        return;
    }
}

DWORD CSportRoom::GetSideSate(BYTE byWhich)
{
    if(byWhich == 1)
        return _roomMaster.dwState;
    else if(byWhich == 2)
        return _roomSubMaster.dwState;
        
    return 0;
}

void CSportRoom::SendAskMsg()
{
    CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(_roomSubMaster.dwGID)->DynamicCast(IID_PLAYER);
    if(pDestPlayer == NULL)
        return;

    CPlayer *pAskPlayer = (CPlayer *)GetPlayerByGID(_roomMaster.dwGID)->DynamicCast(IID_PLAYER);
    if(pAskPlayer == NULL)
        return;

    SAAskCreateRoomMsg msg;
    msg.byType = byRoomType;
    msg.dwSrcGID = GetMasterID();
    msg.dwDesGID = _roomSubMaster.dwGID;
    if(byRoomType == FREE_FACTION)
		strncpy(msg.szName, pAskPlayer->m_Property.m_szTongName, CONST_USERNAME);
    else
		strncpy(msg.szName, pAskPlayer->GetName(), CONST_USERNAME);

    g_StoreMessage(pDestPlayer->m_ClientIndex, &msg, sizeof(SAAskCreateRoomMsg));

}

void CSportRoom::SendAnswerMsg(BYTE byState, DWORD dwGID)
{
    CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
    if(pDestPlayer == NULL)
        return;

    SAAnswerRoomMsg msg;
    msg.dwOtherGID = _roomSubMaster.dwGID;
    msg.dwMasterGID = GetMasterID();
    msg.byState = byState; //SAAnswerRoomMsg::ST_OK;

    g_StoreMessage(pDestPlayer->m_ClientIndex, &msg, sizeof(SAAnswerRoomMsg));

}

bool CSportRoom::CheckTermMoney(DWORD dwGID, DWORD dwMoney, bool bSubtract, bool bAdd, BYTE AddN)
{
//     CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
//     if(pDestPlayer == NULL)
//         return false;
//     
//     if(bSubtract || bAdd)
//     {
//         SABuy msg;
//         if(bSubtract)
//             pDestPlayer->m_Property.m_Money -= dwMoney;
//         if(bAdd)
//             pDestPlayer->m_Property.m_Money += dwMoney*AddN;
// 		msg.nRet = 1;
// 		msg.nMoney = pDestPlayer->m_Property.m_Money;
// 		g_StoreMessage(pDestPlayer->m_ClientIndex , &msg, sizeof(SABuy));
//     }
//     else
//     {
//         if(pDestPlayer->m_Property.m_Money < dwMoney)
//         {
//             pDestPlayer->SendErrorMsg(SABackMsg::EXM_MONEY_NOT_ENOUGH);
//             return false;
//         }
//     }
  
    return true;
}

void CSportRoom::RecvTermInfo(SIDE_TERM *pTerm, DWORD dwSendGID)
{
    if(!pTerm)
        return;

    if(dwSendGID != _roomMaster.dwGID)
        return;

    if(GetSideSate(1) == T_TALK)
    {
        term = *pTerm;
        SendTermToOther();
    }
}

void CSportRoom::SendLeaderMoneyInfo(DWORD dwGID)
{
    CPlayer *pMasterPlayer = (CPlayer *)GetPlayerByGID(_roomMaster.dwGID)->DynamicCast(IID_PLAYER);
    if(pMasterPlayer == NULL)
        return;

    CPlayer *pSubMasterPlayer = (CPlayer *)GetPlayerByGID(_roomSubMaster.dwGID)->DynamicCast(IID_PLAYER);
    if(pSubMasterPlayer == NULL)
        return;

    SALeaderMoneyMsg msg;
    msg.dwMasterMoney = pMasterPlayer->m_Property.m_Money;
    msg.dwSubMasterMoney = pSubMasterPlayer->m_Property.m_Money;

    if(dwGID)
    {
        CPlayer *pPlayer = (CPlayer *)GetPlayerByGID(_roomSubMaster.dwGID)->DynamicCast(IID_PLAYER);
        if(pPlayer == NULL)
            return;    

        g_StoreMessage(pPlayer->m_ClientIndex , &msg, sizeof(SALeaderMoneyMsg));
    }
    else
    {
        g_StoreMessage(pMasterPlayer->m_ClientIndex , &msg, sizeof(SALeaderMoneyMsg));
        g_StoreMessage(pSubMasterPlayer->m_ClientIndex , &msg, sizeof(SALeaderMoneyMsg));
    }
}

// 帮派相关 
//-------------------------------------------------------------------------------------
bool CSportRoom::PutFaction()
{

//     WORD wNum = (int)g_Script.GetTableValue("areaconfig_region_F.1");
//     if (wNum == 0)
//         wNum = 1;
// 
//     wNum = rand()%wNum + 2;
//     term.wPlace = wNum-1;   // 记录下场景标记
//     char szRegion[65] = {0};
//     sprintf(szRegion, "areaconfig_region_F.%d", wNum); 
//     dwRegionGID =  CreateDynamicRegion(IID_SPORTSAREA, (int)g_Script.GetTableValue(szRegion), 
//     1 /*pPlayer->m_ParentRegion->m_wRegionID*/, 
//     187 /*pPlayer->m_wCurX*/,  
//     287/* pPlayer->m_wCurY*/,
// 	0);
// 
//     LPIObject FindRegionByGID(DWORD GID);
//     CSportsArea *pArea = (CSportsArea *)FindRegionByGID(dwRegionGID)->DynamicCast(IID_SPORTSAREA);
//     if(pArea)
//     {
//         pArea->m_byPKType = FREE_FACTION;
//         pArea->SetMaster(GetMasterID());
//         PutFactionMember(_roomMaster.dwGID); 
//         PutFactionMember(_roomSubMaster.dwGID); 
//         return true;
//     }
//     else
//     {
//         if(_pManager)
//             _pManager->Detach(this);
//     }

    return false;
}

void CSportRoom::PutFactionMember(DWORD dwGID)
{
//     CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
//     if(pDestPlayer == NULL)
//         return;
// 
//     CPlayer *pMasterPlayer = (CPlayer *)GetPlayerByGID(_roomMaster.dwGID)->DynamicCast(IID_PLAYER);
//     if(pMasterPlayer == NULL)
//         return;
// 
//     WORD wPosX = 0;
//     WORD wPosY = 0;
//     WORD wTeam = 0;
//     if(strncmp(pDestPlayer->m_Property.m_szTongName, 
//         pMasterPlayer->m_Property.m_szTongName, CONST_USERNAME) == 0)
//     {
//         if(dwGID == _roomMaster.dwGID)
//         {
//             PINFO info;
//             info.wTeamID = 1;
//             info.wLevel = pDestPlayer->m_Property.m_Level;
//             info.dwGID = dwGID;
//             _MemberInfo[pMasterPlayer->m_Property.m_szTongName] = info;
// 
//         }
// 
//         char szPos[65] = {0};
//         sprintf(szPos, "areaconfig_region_F_pos%d1.1", term.wPlace); 
//         
//         WORD wNum = (int)g_Script.GetTableValue(szPos);
//         if (wNum == 0)
//             wNum = 1;
//         wNum = rand()%wNum + 2;
//  
//         sprintf(szPos, "areaconfig_region_F_pos%d1.%d.1", term.wPlace, wNum); 
//         wPosX = (int)g_Script.GetTableValue(szPos);
//         sprintf(szPos, "areaconfig_region_F_pos%d1.%d.2", term.wPlace, wNum); 
//         wPosY = (int)g_Script.GetTableValue(szPos);
//         wTeam = 1;
//         goto _enter;
//     }
//     else
//     {
//         CPlayer *pSubMasterPlayer = (CPlayer *)GetPlayerByGID(_roomSubMaster.dwGID)->DynamicCast(IID_PLAYER);
//         if(pSubMasterPlayer == NULL)
//             return;
// 
//         if(strncmp(pDestPlayer->m_Property.m_szTongName, 
//             pSubMasterPlayer->m_Property.m_szTongName, CONST_USERNAME) == 0)
//         {
//             if(dwGID == _roomSubMaster.dwGID)
//             {
//                 PINFO info;
//                 info.wTeamID = 2;
//                 info.wLevel = pDestPlayer->m_Property.m_Level;
//                 info.dwGID = dwGID;
//                 _MemberInfo[pSubMasterPlayer->m_Property.m_szTongName] = info;
// 
//             }
// 
//             char szPos[65] = {0};
//             sprintf(szPos, "areaconfig_region_F_pos%d2.1", term.wPlace); 
//             
//             WORD wNum = (int)g_Script.GetTableValue(szPos);
//             if (wNum == 0)
//                 wNum = 1;
//             wNum = rand()%wNum + 2;
// 
//             sprintf(szPos, "areaconfig_region_F_pos%d2.%d.1", term.wPlace, wNum); 
//             wPosX = (int)g_Script.GetTableValue(szPos);
//             sprintf(szPos, "areaconfig_region_F_pos%d2.%d.2", term.wPlace, wNum); 
//             wPosY = (int)g_Script.GetTableValue(szPos);
//             wTeam = 2;
//             goto _enter;
//         }
//         else
//             return;
//     }
// 
// _enter:
//     if(pDestPlayer->m_ParentRegion)
//     {
//         pDestPlayer->m_wBackRegionID = pDestPlayer->m_ParentRegion->m_wRegionID;
//         pDestPlayer->m_wBackPosX = pDestPlayer->m_wCurX;
//         pDestPlayer->m_wBackPosY = pDestPlayer->m_wCurY;
//     }
// 
// 
//     SetPState(dwGID, (BYTE)wTeam, _roomMaster.dwGID);
//     //Add(dwGID, wTeam);
//     if(wTeam == 1)
//         _listOne.Add(dwGID);
//     else if(wTeam == 2)
//         _listTwo.Add(dwGID);
//     PutPlayerIntoDestRegion(pDestPlayer, 0, wPosX, wPosY, dwRegionGID);

}

void CSportRoom::SendFPtoMember()
{
    CPlayer *pMasterPlayer = (CPlayer *)GetPlayerByGID(_roomMaster.dwGID)->DynamicCast(IID_PLAYER);
    if(pMasterPlayer == NULL)
        return;
   
    CPlayer *pSubMasterPlayer = (CPlayer *)GetPlayerByGID(_roomSubMaster.dwGID)->DynamicCast(IID_PLAYER);
    if(pSubMasterPlayer == NULL)
        return;
   
    if(pMasterPlayer->m_ParentRegion == NULL)    
        return;
        
    WORD wRegionID = pMasterPlayer->m_wBackRegionID;

    SAFactionInfoMsg msg;
    msg.dwMasterID = _roomMaster.dwGID;
	dwt::strcpy(msg.szAskFactionName, pMasterPlayer->m_Property.m_szTongName, CONST_USERNAME);

    //=SendMessageToAllMember(pSubMasterPlayer->m_Property.m_szTongName, &msg, 
//        sizeof(SAFactionInfoMsg), 0, wRegionID);
    
	dwt::strcpy(msg.szAskFactionName, pSubMasterPlayer->m_Property.m_szTongName, CONST_USERNAME);

    //SendMessageToAllMember(pMasterPlayer->m_Property.m_szTongName, &msg, 
     //   sizeof(SAFactionInfoMsg), 0, wRegionID);
}
//-------------------------------------------------------------------------------------
void CSportRoom::RecvAnswerMsg(SQAnswerRoomMsg *pMsg)
{
    if(!pMsg)
        return;

    if(!_listOne.Locate(pMsg->dwOtherGID))
    if(!_listTwo.Locate(pMsg->dwOtherGID))
        return;

    switch(pMsg->byState)
    {
    case SQAnswerRoomMsg::ST_OK:
        {
            // 是帮战的话直接创建场景进入
            if(byRoomType == FREE_FACTION)
            {
                if(PutFaction())
                {
                    DWORD dwMoney = (int)g_Script.GetGlobalValue("areaconfig_region_F_money");
                    CheckTermMoney(_roomMaster.dwGID, dwMoney, true);
                    SendFPtoMember();
                }
            }
            else
            {
                SendAnswerMsg(SAAnswerRoomMsg::ST_OK, _roomMaster.dwGID);
                SendAnswerMsg(SAAnswerRoomMsg::ST_OK, _roomSubMaster.dwGID);
                OperatePInfo(_roomMaster.dwGID, 1, 1);
                OperatePInfo(_roomSubMaster.dwGID, 2, 1);
                SendLeaderMoneyInfo();
            }
        }
        break;
    case SQAnswerRoomMsg::ST_TERM_OK:
        {
            if( pMsg->dwOtherGID == _roomMaster.dwGID)
            {
                //if(!CheckTermMoney(pMsg->dwMasterGID, term.dwMoney))
                //    return;

                SendAnswerMsg(SAAnswerRoomMsg::ST_TERM_OK, _roomSubMaster.dwGID);
                OnOkTerm(&_roomMaster, 1);
            }
            else if(pMsg->dwOtherGID == _roomSubMaster.dwGID)
            {   
                SendAnswerMsg(SAAnswerRoomMsg::ST_TERM_OK, _roomMaster.dwGID); 
                OnOkTerm(&_roomSubMaster, 2);
            }
        }
        break;
    case SQAnswerRoomMsg::ST_CANCEL:
    case SQAnswerRoomMsg::ST_TERM_CANCEL:
        {
            if(pMsg->dwOtherGID == _roomMaster.dwGID || 
                pMsg->dwOtherGID == _roomSubMaster.dwGID)
            {
                OnCancel();
            }

        }break;
    case SQAnswerRoomMsg::ST_ENTER_OK:
        {
            if(pMsg->dwOtherGID == _roomMaster.dwGID)
            {
                if(_roomMaster.dwState == T_OK)
                {
                    _roomMaster.dwState = T_ENTER;
                    SendAnswerMsg(SAAnswerRoomMsg::ST_ENTER_OK, _roomSubMaster.dwGID);
                }
            }
            else if(pMsg->dwOtherGID == _roomSubMaster.dwGID)
            {
                if(_roomSubMaster.dwState == T_OK)
                {       
                    _roomSubMaster.dwState = T_ENTER;
                    SendAnswerMsg(SAAnswerRoomMsg::ST_ENTER_OK, _roomMaster.dwGID);
                }
            }
            if( _roomMaster.dwState == T_ENTER && _roomSubMaster.dwState == T_ENTER)
            {
                SendTermToOther(_listOne, SST_CLOSETERM);
                SendTermToOther(_listTwo, SST_CLOSETERM);
                OnOkEnter();
            }
                
        }break;
    case SQAnswerRoomMsg::ST_MEMBER_EXIT:
        {
            if(pMsg->dwMasterGID == pMsg->dwOtherGID ||
                pMsg->dwOtherGID == _roomSubMaster.dwGID)
                OnCancel();
            else
            {
                if(_listOne.Locate(pMsg->dwOtherGID))
                    Del(pMsg->dwOtherGID, 1);
                if(_listTwo.Locate(pMsg->dwOtherGID))
                    Del(pMsg->dwOtherGID, 2);
                //SetPState(pMsg->dwOtherGID, 0, 0);
                //SendAnswerMsg(SAAnswerRoomMsg::ST_TERM_CANCEL, pMsg->dwOtherGID);
            }

        }break;

    default:
        return;
    }

}

void CSportRoom::SendTermMsg(DWORD dwIndex)
{
    SASendTermMsg msg;
    msg.stTerm = term;
    msg.dwMasterGID = _roomMaster.dwGID;
 
    CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwIndex)->DynamicCast(IID_PLAYER);
    if(pDestPlayer == NULL)
        return;

    g_StoreMessage(pDestPlayer->m_ClientIndex, &msg, sizeof(SASendTermMsg));  
}

void CSportRoom::SendTermToOther(CTSet<DWORD> &set, BYTE bySSType)
{   
    std::set<DWORD>::iterator it, it_e;
    set.GetItBE(it, it_e);
    for(; it != it_e; )
    {
        switch(bySSType)
        {
        case SST_TERM:
            {
                if(*it != _roomMaster.dwGID)
                    SendTermMsg(*it);
            }break;
        case SST_TEAMNAME:
            break;
        case SST_ZALLSTATE:
            SetPState(*it, 0,0);   
            break;
        case SST_ENTERAREA:
            PutPlayerToSportsArea(*it);
            break;
        case SST_CLOSETERM:
            SendAnswerMsg(SAAnswerRoomMsg::ST_TERM_CANCEL, *it);
            break;

        default:
            break;
        }
        it++;
    }
}

void CSportRoom::SendTermToOther()
{
    SendTermToOther(_listOne, SST_TERM);
    SendTermToOther(_listTwo, SST_TERM);
}

void CSportRoom::SetPState(DWORD dwGID, BYTE byTeamID, DWORD dwMasterID)
{
    CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
    if(pDestPlayer == NULL)
        return;

    DWORD dwTheGID = pDestPlayer->GetGID();
    if( dwTheGID == _roomMaster.dwGID || dwTheGID == _roomSubMaster.dwGID)
    {
        pDestPlayer->SetSportsState(byTeamID, 1);
    }
    else
        pDestPlayer->SetSportsState(byTeamID);

    pDestPlayer->SetSportsRoomID(dwMasterID);
}

void CSportRoom::OnOkTerm(PSTATE *pState, BYTE byTeamID)
{
    if(pState)
    {
        pState->dwState = T_OK;
        SetPState(pState->dwGID, byTeamID, _roomMaster.dwGID);
    }
}

void CSportRoom::OnCancel()
{
    SendTermToOther(_listOne, SST_CLOSETERM);
    SendTermToOther(_listTwo, SST_CLOSETERM);
    SendTermToOther(_listOne, SST_ZALLSTATE);
    SendTermToOther(_listTwo, SST_ZALLSTATE);

    if(_pManager)
        _pManager->Detach(this);
}

void CSportRoom::RecvJoinSideMsg(SQJoinSideMsg *pMsg)
{
    // 判断帮战是否
    if(byRoomType == FREE_FACTION)
    {
        PutFactionMember(pMsg->dwSrcGID);
        return;
    }

    if(pMsg->dwDesGID == _roomMaster.dwGID)
    {
        if(_listOne.Size() < TEAM_NUMBERS)
        {
            if(_roomMaster.dwState = T_OK)
            {
                //_listOne.Add(pMsg->dwSrcGID);
                SendTermMsg(pMsg->dwSrcGID);
                Add(pMsg->dwSrcGID, 1);
                SetPState(pMsg->dwSrcGID, 1, _roomMaster.dwGID);
            }
        }
    }
    else if(pMsg->dwDesGID == _roomSubMaster.dwGID)    
    {
        if(_listTwo.Size() < TEAM_NUMBERS)
        {
            if(_roomSubMaster.dwState = T_OK)
            {
                //_listTwo.Add(pMsg->dwSrcGID);
                SendTermMsg(pMsg->dwSrcGID);
                Add(pMsg->dwSrcGID, 2);   
                SetPState(pMsg->dwSrcGID, 2, _roomMaster.dwGID);
            }
        }
    }

}

void CSportRoom::RecvDelMember(CPlayer *pPlayer, const char *szName)
{
    if(pPlayer == NULL || szName == NULL)
        return;

    DWORD dwGID = pPlayer->GetGID();
    if(dwGID == _roomMaster.dwGID || dwGID == _roomSubMaster.dwGID)
    {
        std::map<ustring, PINFO>::iterator it  = _MemberInfo.find(szName);
        if(it != _MemberInfo.end())
        {
            if(it->second.dwGID == _roomMaster.dwGID ||
                it->second.dwGID == _roomSubMaster.dwGID)
                OnCancel();
            else
                Del(it->second.dwGID, (BYTE)it->second.wTeamID);
        }
    }

}

void CSportRoom::PutPlayerToSportsArea(DWORD dwPlayerGID)
{
    CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwPlayerGID)->DynamicCast(IID_PLAYER);
    if(pDestPlayer == NULL)
        return;

    WORD wPos = term.wPlace + AREA_OFFER;
    char szString1[32] = {0};
    char szString2[32] = {0};
    WORD wTeam = pDestPlayer->GetSportsState(); 
    if(wTeam == 2)
        wTeam = 3;

    sprintf(szString1, "areaconfig_teampos.%d.%d", wPos, wTeam);
    sprintf(szString2, "areaconfig_teampos.%d.%d", wPos, wTeam + 1);

    PutPlayerIntoDestRegion(pDestPlayer, 0, (int)g_Script.GetTableValue(szString1), 
        (int)g_Script.GetTableValue(szString2), dwRegionGID);

}

void CSportRoom::PutSportsItems(CRegion *pRegion)
{
//     WORD wItem = term.wPlace + AREA_OFFER;
//     char szString[32] = {0};
//     char szIDString[32] = {0};
//     char szPosXString[32] = {0};
//     char szPosYString[32] = {0};
// 
//     sprintf(szString, "areaconfig_item.%d.1", wItem);
//     int num = (int)g_Script.GetTableValue(szString);
//     for( int n = 2; n < num + 1; ++ n )
//     {
//         sprintf(szIDString, "areaconfig_item.%d.%d.1", wItem, n);
//         sprintf(szPosXString, "areaconfig_item.%d.%d.2", wItem, n);
//         sprintf(szPosYString, "areaconfig_item.%d.%d.3", wItem, n);
// 
//         CItem::SParameter param;
//         ZeroMemory( &param, sizeof( param ) );
//         param.dwLife = MakeLifeTime( SPORT_TIMES );
//         param.xTile = (int)g_Script.GetTableValue( szPosXString );
//         param.yTile = (int)g_Script.GetTableValue( szPosYString );
// 
//         CItem::GenerateNewGroundItem( pRegion, 0, param, 
//             GenItemParams( (int)g_Script.GetTableValue(szIDString) ), LogInfo( -1, "竞技场创建" ) );
//     }
}

void CSportRoom::OnOkEnter()
{
//     if(!CheckTermMoney(_roomMaster.dwGID, term.dwMoney) ||
//         !CheckTermMoney(_roomSubMaster.dwGID, term.dwMoney))
//     {
//         OnCancel();
//         return;
//     }
// 
//     CheckTermMoney(_roomMaster.dwGID, term.dwMoney, true);
//     CheckTermMoney(_roomSubMaster.dwGID, term.dwMoney, true);
// 
//     CPlayer *pPlayer = (CPlayer *)GetPlayerByGID(_roomMaster.dwGID)->DynamicCast(IID_PLAYER);
//     if(pPlayer == NULL)
//     {
//         OnCancel();
//         return;
//     }
// 
//     if(pPlayer->m_ParentRegion == NULL)
//     {
//         OnCancel();
//         return;
//     }
// 
//     WORD wRegion = term.wPlace + AREA_OFFER;
//     char szString[32] = {0};
//     sprintf(szString, "areaconfig_region.%d", wRegion);
//     dwRegionGID =  CreateDynamicRegion(IID_SPORTSAREA, (int)g_Script.GetTableValue(szString), 
//        1 /*pPlayer->m_ParentRegion->m_wRegionID*/, 
//        187 /*pPlayer->m_wCurX*/,  
//        287/* pPlayer->m_wCurY*/,
// 	   0);
// 
//     LPIObject FindRegionByGID(DWORD GID);
//     CSportsArea *pArea = (CSportsArea *)FindRegionByGID(dwRegionGID)->DynamicCast(IID_SPORTSAREA);
//     if(pArea)
//     {
//         if(term.bHaveGoods)
//         {
//             PutSportsItems(pArea);
//             pArea->SetTemporaryItems(wRegion);
//         }
//         pArea->SetLoopTime(term.wTime);
//         pArea->SetMaster(GetMasterID());
// 
//         SendTermToOther(_listOne, SST_ENTERAREA);
//         SendTermToOther(_listTwo, SST_ENTERAREA);
//     }
//     else
//         OnCancel();
}

bool CSportRoom::CheckTermExper(DWORD dwGID, DWORD dwExper, bool bSubOrAdd)
{
//     CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
//     if(pDestPlayer == NULL)
//         return false;  
// 
//     if(dwExper >= 1000000)
//         dwExper = 1000000;
// 
//     if(bSubOrAdd)
//     {
//         //pDestPlayer->m_iExp += dwExper;    
//         //pDestPlayer->SendAddPlayerExp( dwExper, 0, false, 1, "竞技" );
//     }
//     else
//     {
//         //pDestPlayer->m_iExp -= dwExper;
//         pDestPlayer->SendReducePlayerExp(dwExper);
//     }

    return true;
}

dwt::stringkey<char[CONST_USERNAME]> CSportRoom::GetNeedName(DWORD dwGID)
{
    std::map<ustring, PINFO>::iterator it  = _MemberInfo.begin();
    for(; it != _MemberInfo.end(); )
    {
        if( it->second.dwGID == dwGID)
            return it->first;

        it++;
    }

    return 0;
}

void CSportRoom::Bulletin(BYTE byWin)
{ 
    ustring mastername = GetNeedName(_roomMaster.dwGID);;
    ustring submastername = GetNeedName(_roomSubMaster.dwGID);;

    if(byWin == 0)
        TalkToAll(FormatString("[%s](队)在与[%s](队)切磋中不分胜负！", &submastername, &mastername));
    else if(byWin == 1)
    {
        if(byRoomType == FREE_FACTION)
            TalkToAll(FormatString("[%s]帮在与[%s]帮交战中获胜！！！", 
                &mastername, &submastername));
        else
            TalkToAll(FormatString("[%s](队)在与[%s](队)切磋中获胜！！！", &mastername, &submastername));
    }
    else if(byWin == 2)
    {
        if(byRoomType == FREE_FACTION)
            TalkToAll(FormatString("[%s]帮在与[%s]帮交战中获胜！！！", 
                &submastername, &mastername));
        else
            TalkToAll(FormatString("[%s](队)在与[%s](队)切磋中获胜！！！", &submastername, &mastername));
    }
}

void CSportRoom::SetWinner(BYTE byTeamID)
{
    Bulletin(byTeamID);

    if(byRoomType == FREE_FACTION)
        return;

    if(byTeamID == 0)
    {
        CheckTermMoney(_roomMaster.dwGID, term.dwMoney, false, true, 1);
        CheckTermMoney(_roomSubMaster.dwGID, term.dwMoney, false, true, 1);
    }
    else if(byTeamID == 1)
    {
        if(term.dwMoney != 0)
            CheckTermMoney(_roomMaster.dwGID, term.dwMoney, false, true);
        
        if(term.dwExper != 0)
        {
            //CheckTermExper(_roomMaster.dwGID, term.dwExper, true);
            CheckTermExper(_roomSubMaster.dwGID, term.dwExper, false);
        }
    }
    else if(byTeamID == 2)
    {
        if(term.dwMoney != 0)
            CheckTermMoney(_roomSubMaster.dwGID, term.dwMoney, false, true);
        
        if(term.dwExper != 0)
        {
            CheckTermExper(_roomMaster.dwGID, term.dwExper, false);
            //CheckTermExper(_roomSubMaster.dwGID, term.dwExper, true);
        }
    }

}
