#include "StdAfx.h"
#include "gaminghouse.h"
#include "player.h"

extern LPIObject GetPlayerByGID(DWORD);

CGHouseManager *CGHouseManager::_Instance = NULL;
CGHouseManager::CGHouseManager()
{

}

CGHouseManager::~CGHouseManager()
{
    if(_Instance)
    {
        delete _Instance;
        _Instance = NULL;
    }
    Notify(1);
}

CGHouseManager *CGHouseManager::Instance(void)
{
    if(_Instance == NULL)
    {
        _Instance = new CGHouseManager;
    }

    return _Instance;
}

void CGHouseManager::DispatchGamingHouseMsg(SGamingHouseBaseMsg *pMsg, CPlayer *pPlayer)
{
    if(pMsg == NULL || pPlayer == NULL)
        return;

    CGamingHouse *pSubRoom = (CGamingHouse *)Locate(pPlayer->GetSportsRoomID());
    if(NULL == pSubRoom)
        return;

    if(pSubRoom->IsEmptyRoom())
        return;

    switch(pMsg->_protocol)
    {
    case SGamingHouseBaseMsg::EPRO_SHOWWAGERINFO:
        pSubRoom->RecvWagerMessage((SQShowWagerInfoMsg *)pMsg, pPlayer);
        break;

    case SGamingHouseBaseMsg::EPRO_GAMINGCAST:
        pSubRoom->RecvGamingcastMessage((SQShowGamingcastMsg *)pMsg, pPlayer);
        break;
        
    case SGamingHouseBaseMsg::EPRO_GAMINGPLAYERINFO:
        pSubRoom->RecvGamingPlayerRequestMessage((SQGamingPlayerRequestMsg *)pMsg, pPlayer);
        break;

    default:
        return;
    }
}

void CGHouseManager::PushEmptyList(DWORD dwRoomID)
{

    _ghouseList.push_back(dwRoomID);
    
    //CRoomContainer::Detach(pSubRoom);
}

CRoom *CGHouseManager::GetEmptyRoom(void)
{
    if(_ghouseList.size() == 0)
        return NULL;

    CRoom *pRoom = Locate(_ghouseList.front());

	if (!pRoom)
	{
		rfalse(4, 1, "GameingHouse.cpp - GetEmptyRoom() - !pRoom");
		return NULL;
	}

    _ghouseList.pop_front();

    return pRoom;
}

CGamingHouse::CGamingHouse(DWORD dwRoomID)
{
    SetMasterID(dwRoomID);
    GameZero();
    InitNew();
    _theGHouseManager->Attach(this);
}

CGamingHouse::~CGamingHouse(void)
{
    _theGHouseManager->Detach(this);
}

void CGamingHouse::Release(void)
{
    delete this;
}

void CGamingHouse::InitNew()
{
    _wCurPlayerNumber = 0;  
    _GamingState = GMT_CREATE;
    _dwCountTime = GetTickCount();
    //_wCount = 0;
    ZeroMemory(_gamingPlayerList, sizeof(GAMINGPLAYER)*GAMING_MAXPLAER);
}

void CGamingHouse::Update(CRoomContainer *pSubFrame)
{
    GamingLoop();
}

void CGamingHouse::GameZero(void)
{
    _wCurPlayerPos = 0;
    _bNewLoop = TRUE;
    _dwCountTime = 0;
    _wCount = 0;
    _Budget[0] = 0;
    _Budget[1] = 0;
    _winSet.Clear();
    _loseSet.Clear();
}

WORD CGamingHouse::GetMultiple(WORD wPutNumber)
{
    switch(wPutNumber)
    {
    case 1: return 18;
    case 2: return 10;
    case 3: return  8;
    case 4: return  6;
    case 5: return  4;
    case 6: return  2;
    default:
        return 1;
    }
}

void CGamingHouse::BackWager(void)
{
    //if(_loseSet.Size() == 0)
    //    return;

    for(int n = 0; n < GAMING_MAXPLAER; n++)
    {
        CPlayer *pPlayer = (CPlayer *)GetPlayerByGID(_gamingPlayerList[n].dwGID)->DynamicCast(IID_PLAYER);
        if(pPlayer == NULL)
            continue;

        pPlayer->SetSportsRoomID(0);
        if(_gamingPlayerList[n].dwWager == 0 )
            continue;
        
        if(_loseSet.Locate(_gamingPlayerList[n].wPosID))
        {
         
            pPlayer->m_Property.m_Money +=  _gamingPlayerList[n].dwWager;
            //_gamingPlayerList[n].dwWager = 0;
            //_gamingPlayerList[n].dwMoney = pPlayer->m_nMoney;
            //SendOnePlayerInfoToAll(&_gamingPlayerList[n]);

            // 玩家身上的钱数据的刷新
//             SABuy msg;
//             msg.nRet = 1;
//             msg.nMoney = pPlayer->m_Property.m_Money;
//             msg.bShowMessage = false;
//             g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABuy));
        }
    }
}

void CGamingHouse::GameResult(void)
{
    std::set<WORD>::iterator it, it_e;

    // 庄家收赢了的钱
    CPlayer *pPlayer = (CPlayer *)GetPlayerByGID(GetPlayer(0)->dwGID)->DynamicCast(IID_PLAYER);
    if(pPlayer)
    {
         _Budget[0] += pPlayer->m_Property.m_Money;
    }
    else
    {
        return;
    }

    // 庄家支付给赢了自己的玩家的钱
    _winSet.GetItBE(it, it_e);
    for(; it != it_e; it++)
    {
        if(_Budget[0] == 0)
        {
            break;
        }

        CGamingHouse::GAMINGPLAYER *pGPlayer = GetPlayer(*it);
        if(pGPlayer == NULL)
            continue;

        DWORD dwWager = pGPlayer->dwWager;
        if(dwWager)
        {
            CPlayer *pOPlayer = (CPlayer *)GetPlayerByGID(pGPlayer->dwGID)->DynamicCast(IID_PLAYER);
            if(pOPlayer)
            {
                if(_Budget[0] >= dwWager)
                {
                    _Budget[0] -= dwWager;
                    pOPlayer->m_Property.m_Money += dwWager;
                }
                else
                {
                    _Budget[0] = 0;
                    pOPlayer->m_Property.m_Money += _Budget[0];
                }
                //// 刷新数据
                pGPlayer->dwWager = 0;
                pGPlayer->dwMoney = 0;
                //SendOnePlayerInfoToAll(GetPlayer(*it));

                // 玩家身上的钱数据的刷新
//                 SABuy msg;
//                 msg.nRet = 1;
//                 msg.nMoney = pOPlayer->m_Property.m_Money;
//                 msg.bShowMessage = false;
//                 g_StoreMessage(pOPlayer->m_ClientIndex, &msg, sizeof(SABuy));
            }
            else
            {
                --_wCurPlayerNumber;
				ZeroMemory(pGPlayer->szName, CONST_USERNAME);
                SendOnePlayerInfoToAll(pGPlayer, pGPlayer->dnid);
                ZeroMemory(pGPlayer, sizeof(GAMINGPLAYER)); 
            }
        }
    }

    // 随后刷新庄家数据
    pPlayer->m_Property.m_Money = _Budget[0];
    GetPlayer(0)->dwMoney = pPlayer->m_Property.m_Money;
    //GetPlayer(0)->wResult = 0;
    //SendOnePlayerInfoToAll(GetPlayer(0));
    // 庄家身上的钱数据的刷新
//     SABuy msg;
//     msg.nRet = 1;
//     msg.nMoney = pPlayer->m_Property.m_Money;
//     msg.bShowMessage = false;
//     g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABuy));

    // 刷新所有人的数据
    for(int n = 0; n < GAMING_MAXPLAER; n++)
    {
        CGamingHouse::GAMINGPLAYER *pOPlayer = GetPlayer(n);
        if(pOPlayer)
        {
            if(pOPlayer->szName[0] != '\0')
            {
                pOPlayer->wResult = 0;
                pOPlayer->nGrade = 0;
                SendAllPlayerInfoToOne(pOPlayer->dnid);
            }
        }
    }

}

void CGamingHouse::Budget(GAMINGPLAYER *pOtherGPlayer)
{ 
    if(pOtherGPlayer == NULL)
        return;

    // 判断庄家是否在赌场
    if(!CheckGPlayer(GetPlayer(0)->dwGID))
    {
		ZeroMemory(GetPlayer(0)->szName, CONST_USERNAME);
        SendOnePlayerInfoToAll(GetPlayer(0), GetPlayer(0)->dnid);
        ZeroMemory(GetPlayer(0), sizeof(GAMINGPLAYER));
        GamingOver();
        return;
    }

    if(pOtherGPlayer->wPosID == 0)
    {
        // 是庄家直接更新投掷结果
        SendOnePlayerInfoToAll(pOtherGPlayer);
        return;
    }

    if(GetPlayer(0)->wResult <= pOtherGPlayer->wResult)
    {
        // 预算庄家收入
        _Budget[0] += pOtherGPlayer->dwWager;
        
        // 本次输赢的记录
        pOtherGPlayer->nGrade -= pOtherGPlayer->dwWager;
        SendOnePlayerInfoToAll(pOtherGPlayer);

        pOtherGPlayer->dwWager = 0;
    }
    else
    {
        if(pOtherGPlayer->wResult >= 37)
        {
            // 预算庄家收入
            _Budget[0] += pOtherGPlayer->dwWager;
            
            // 本次输赢的记录
            pOtherGPlayer->nGrade -= pOtherGPlayer->dwWager;
            SendOnePlayerInfoToAll(pOtherGPlayer);

            pOtherGPlayer->dwWager = 0; 
        }
        else
        {
            // 预算庄家赔出
            DWORD dwNum = 0;
            _winSet.Add(pOtherGPlayer->wPosID);
            _Budget[1] = GetMultiple(pOtherGPlayer->wResult) * pOtherGPlayer->dwWager;
            
            // 本次输赢的记录
            pOtherGPlayer->nGrade = _Budget[1];
            SendOnePlayerInfoToAll(pOtherGPlayer);

            CPlayer *pPlayer = (CPlayer *)GetPlayerByGID(GetPlayer(0)->dwGID)->DynamicCast(IID_PLAYER);
            if(pPlayer)
            {
                if(_Budget[1] > pPlayer->m_Property.m_Money)
                {
                    dwNum = _Budget[1] - pPlayer->m_Property.m_Money;
                    pPlayer->m_Property.m_Money = 0;

                    // 记录庄家本次输赢的记录
                    GetPlayer(0)->nGrade += pPlayer->m_Property.m_Money;
                }
                else
                {
                    pPlayer->m_Property.m_Money -= _Budget[1];

                    // 记录庄家本次输赢的记录
                    GetPlayer(0)->nGrade += _Budget[1];
                }

                GetPlayer(0)->dwMoney = pPlayer->m_Property.m_Money;

                CPlayer *pOtherPlayer = (CPlayer *)GetPlayerByGID(pOtherGPlayer->dwGID)->DynamicCast(IID_PLAYER);
                if(pOtherPlayer)
                {
                    pOtherPlayer->m_Property.m_Money += ((_Budget[1] - dwNum) + pOtherGPlayer->dwWager);
                    pOtherGPlayer->dwWager = dwNum;
                    pOtherGPlayer->dwMoney = pOtherPlayer->m_Property.m_Money;

                    // 玩家身上的钱数据的刷新
//                     SABuy msg;
//                     msg.nRet = 1;
//                     msg.nMoney = pOtherPlayer->m_Property.m_Money;
//                     msg.bShowMessage = false;
//                     g_StoreMessage(pOtherPlayer->m_ClientIndex, &msg, sizeof(SABuy));

                }
                
            }
        }
    }

    // 最后清掉数据
    //pOtherGPlayer->wResult = 0;
    _loseSet.Del(pOtherGPlayer->wPosID);
}

void CGamingHouse::SendInfoToAll(SGamingHouseBaseMsg *pMsg, WORD wSize,  DNID Except)
{
    if(pMsg == NULL)
        return;

    for(int n = 0; n < GAMING_MAXPLAER; n++)
    {
        if(GetPlayer(n)->dnid != 0)
        {
            if(GetPlayer(n)->dnid != Except)
                g_StoreMessage(GetPlayer(n)->dnid, pMsg, wSize);
        }
    }
}

void CGamingHouse::SendOnePlayerInfoToAll(GAMINGPLAYER *pGPlayerInfo, DNID Except)
{
    if(pGPlayerInfo == NULL)
        return;

    SAGamingPlayerInfoMsg gpinfoMsg;
    ZeroMemory(gpinfoMsg.GPlayer, sizeof(_GAMINGPLAYER)*GAMING_MAXPLAER);

    gpinfoMsg.wNumber = 1;
    memcpy(gpinfoMsg.GPlayer, pGPlayerInfo, sizeof(_GAMINGPLAYER));
    SendInfoToAll(&gpinfoMsg, gpinfoMsg.MySize(), Except);
}

void CGamingHouse::SendAllPlayerInfoToOne(DNID dnid)
{
    SAGamingPlayerInfoMsg gpinfoMsg;
    ZeroMemory(gpinfoMsg.GPlayer, sizeof(_GAMINGPLAYER)*GAMING_MAXPLAER);
    
    if(_wCurPlayerNumber > GAMING_MAXPLAER)
        _wCurPlayerNumber = GAMING_MAXPLAER;

    gpinfoMsg.wNumber = _wCurPlayerNumber;
    for(int m = 0; m < GAMING_MAXPLAER; m++)
    {
        if(_gamingPlayerList[m].szName[0] != '\0')
        {
            for(int n = 0; n < _wCurPlayerNumber; n++)
            {
                if(gpinfoMsg.GPlayer[n].szName[0] == '\0')
                {
                    memcpy(&gpinfoMsg.GPlayer[n], &_gamingPlayerList[m], sizeof(_GAMINGPLAYER));
                    break;
                }
            }
        }
    }

    g_StoreMessage(dnid, &gpinfoMsg, gpinfoMsg.MySize());
}

void CGamingHouse::SendShowWagerToAll()
{   
    SAShowWagerInfoMsg msg;    
    
    // 下注消息只是发给闲家
    for(int n = 1; n < GAMING_MAXPLAER; n++)
    {
        //if(!CheckGPlayer(GetPlayer(n)->dwGID))
            //continue;

        SendShowWagerToOne(GetPlayer(n));
        //++_wCount;
        //msg.wPosID =  GetPlayer(n)->wPosID; 
        //g_StoreMessage(GetPlayer(n)->dnid, &msg, sizeof(SAShowWagerInfoMsg));
    }  

    _dwCountTime = GetTickCount();
}

void CGamingHouse::SendShowWagerToOne(GAMINGPLAYER *pGPlayerInfo)
{
    if(pGPlayerInfo == NULL)
        return;

    if(!CheckGPlayer(pGPlayerInfo->dwGID))
    {
        if(pGPlayerInfo->szName[0] != '\0')
        {
			ZeroMemory(pGPlayerInfo->szName, CONST_USERNAME);
            SendOnePlayerInfoToAll(pGPlayerInfo);
            ZeroMemory(pGPlayerInfo, sizeof(GAMINGPLAYER));
        }

        return;
    }

    SAShowWagerInfoMsg msg;
    msg.wPosID = pGPlayerInfo->wPosID;
    g_StoreMessage(pGPlayerInfo->dnid, &msg, sizeof(SAShowWagerInfoMsg));

    ++_wCount;

}

void CGamingHouse::SendShowGamingCast(void)
{
    SAShowGamingcastMsg msg;

    msg.wPosID = _wCurPlayerPos;
    SendInfoToAll(&msg, sizeof(SAShowGamingcastMsg));

    // 开始倒计时
    _dwCountTime = GetTickCount();
}

BOOL CGamingHouse::CheckGPlayer(DWORD dwGID)
{
    CPlayer *pPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
    if(pPlayer == NULL)
        return FALSE;   

    if(pPlayer->m_OnlineState != CPlayer::OST_NORMAL)
        return FALSE;

    if(pPlayer->GetSportsRoomID() != GetRoomID())
        return FALSE;

    return TRUE;
}

void CGamingHouse::PayToMaster(GAMINGPLAYER *pGPlayer)
{
	if (!pGPlayer)
	{
		rfalse(4, 1, "GamingHouse.cpp - PayToMaster() - !pGPlayer");
		return;
	}

    CPlayer *pPlayer = (CPlayer *)GetPlayerByGID(GetMasterID())->DynamicCast(IID_PLAYER);
    if(pPlayer == NULL)
        return;
    
    pPlayer->m_Property.m_Money += pGPlayer->dwWager;

    // 玩家身上的钱数据的刷新
//     SABuy msg;
//     msg.nRet = 1;
//     msg.nMoney = pPlayer->m_Property.m_Money;
//     msg.bShowMessage = false;
//     g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABuy));
}

void CGamingHouse::GamingCast(GAMINGPLAYER *pGPlayer)
{
    if(pGPlayer == NULL)
        return;

    if(CheckGPlayer(pGPlayer->dwGID))
    {
        WORD wCastNumber = (rand()%GAMING_RATE)+1;
        pGPlayer->wResult = wCastNumber;

        //// 向赌场所有人员更新我的数据
        //SendOnePlayerInfoToAll(pGPlayer);

        // 与庄家的比较结果
        Budget(pGPlayer);
    }
    else
    {
        // 压注的钱给庄家
        PayToMaster(pGPlayer);
    }
    // 设置下一个掷骰子的玩家
    bool bBreak = false;
    while(!bBreak)
    {
        ++_wCurPlayerPos;
        if(_wCurPlayerPos >= GAMING_MAXPLAER) 
        {
            // 新一轮更新数据
            GameResult();
            GameZero();
            _GamingState = GMT_START; 
            bBreak = true;
        }
        else
        {
            CGamingHouse::GAMINGPLAYER *pGPlayer = GetPlayer(_wCurPlayerPos);
            if(pGPlayer != NULL)
            {
                if(CheckGPlayer(pGPlayer->dwGID))
                {
                    if(pGPlayer->dwWager != 0)
                    {
                        SendShowGamingCast();
                        bBreak = true;
                    }
                }
                else
                {
                    if(pGPlayer->dwWager > 0)
                        PayToMaster(pGPlayer);
                }
            }
        }
    }

}

BOOL CGamingHouse::CheckMoney(DWORD dwGID, DWORD dwMoney)
{
    CPlayer *pPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
    if(pPlayer == NULL)
        return FALSE; 

    if(pPlayer->m_Property.m_Money == dwMoney)
        return TRUE;

    return FALSE;
}

BOOL CGamingHouse::GamingLoop(void)
{
    if(IsEmptyRoom())
        return FALSE;

    if(!CheckGPlayer(GetMasterID()))
    {
		ZeroMemory(GetPlayer(0)->szName, CONST_USERNAME);
        SendOnePlayerInfoToAll(GetPlayer(0), GetPlayer(0)->dnid);
        ZeroMemory(GetPlayer(0), sizeof(GAMINGPLAYER));
        GamingOver(); 
        return FALSE;
    }

    if(_bNewLoop)
    {
        if(CheckMoney(GetMasterID(), 0))
        {
            CPlayer *pPlayer = (CPlayer *)GetPlayerByGID(GetMasterID())->DynamicCast(IID_PLAYER);
            if(pPlayer)
            {
                DelGamingPlayer(0, pPlayer);
                GamingOver(); 
                return FALSE;
            }
        }

        // 新局开始的时候设定及时间的倒计时
        if(_dwCountTime > 0)
        {
            if((_dwCountTime + 1000*GAMING_WAITTIME) < GetTickCount())
            {
                if(_loseSet.Size() > 0)
                {
                    _bNewLoop = FALSE;         
                }
                else
                {
                    _GamingState = GMT_START;
                    _wCount = 0;

                }
                _dwCountTime = 0;
            }
            else
            {
                if(_wCount == _loseSet.Size())
                {
                    _bNewLoop = FALSE;
                    _dwCountTime = 0;
                }
            }
        }
        else
        {
            if(_GamingState != GMT_CREATE)
            {
                SendShowWagerToAll();
            }
        }

    }
    else
    {
        // 已经开始的局及等待玩家掷骰子的时间倒计

        if(_loseSet.Size() == 0)
        {
            _bNewLoop = TRUE;
            _dwCountTime = 0;
            _wCount = 0;
            _GamingState = GMT_START;
            return FALSE;
        }

        if(_GamingState == GMT_CREATE ||
            _GamingState == GMT_START)
        {
            _GamingState = GMT_WAIT;
            SendShowGamingCast();  
        }

        if(_dwCountTime)
        {
            if((_dwCountTime + 1000*GAMING_WAITTIME) < GetTickCount())
                GamingCast(GetPlayer(_wCurPlayerPos));
        }
    }

    return TRUE;
}

void CGamingHouse::GamingOver(void)
{  
    BackWager();
    GameZero();
    _GamingState = GMT_EMPTY;

    _theGHouseManager->PushEmptyList(GetRoomID());
}

CGamingHouse::GAMINGPLAYER *CGamingHouse::GetPlayer(WORD wID)  
{ 
    if(wID >= GAMING_MAXPLAER)
        return NULL;

    return &_gamingPlayerList[wID];
}

int CGamingHouse::GetSpacePos(const char* name)
{
    if(name == NULL)
        return -1;

    // 这里只找闲家的位置 庄家默认为0
    for(int n = 1; n < GAMING_MAXPLAER; n++)
    {
		if (dwt::strcmp(name, GetPlayer(n)->szName, CONST_USERNAME) == 0)
        {
            SendAllPlayerInfoToOne(GetPlayer(n)->dnid);
            return -1;
        }
        else
        {
            if(GetPlayer(n)->szName[0] == '\0')  
                return n;
        }
    }

    return -1;
}

BOOL CGamingHouse::AddGamingPlayer(CPlayer *pPlayer)
{
//     if(pPlayer == NULL)
//         return FALSE;
// 
//     if(_wCurPlayerNumber >= GAMING_MAXPLAER)
//     {
//         TalkToDnid(pPlayer->m_ClientIndex, "人数已经满了！");
//         return FALSE;
//     }
// 
//     GAMINGPLAYER gplayer;
//     ZeroMemory(&gplayer, sizeof(GAMINGPLAYER));
// 
//     int wPos = 0;
//     if(pPlayer->GetGID() != GetMasterID())
//         wPos = GetSpacePos(pPlayer->GetName());
// 
//     if(wPos == -1)
//         return FALSE;
// 
//     dwt::strcpy(gplayer.szName, pPlayer->GetName(), CONST_USERNAME);
//     gplayer.wPosID = wPos;
//     gplayer.dwMoney = pPlayer->m_Property.m_Money;
//     gplayer.dnid = pPlayer->m_ClientIndex;
//     gplayer.dwGID = pPlayer->GetGID();
//     gplayer.m_bySex = pPlayer->m_Property.m_Sex;
//     gplayer.m_bySchool = pPlayer->m_Property.m_School;
//     gplayer.m_byBRON = pPlayer->m_Property.m_byBRON;
// 
//     _gamingPlayerList[wPos] = gplayer;
//     ++_wCurPlayerNumber;
// 
//     pPlayer->SetSportsRoomID(GetRoomID());
//     SendOnePlayerInfoToAll(&gplayer, pPlayer->m_ClientIndex);
//     SendAllPlayerInfoToOne(pPlayer->m_ClientIndex);
// 
//     if(_bNewLoop)
//     {
//         if(wPos != 0)
//         {
//             SendShowWagerToOne(&gplayer);
//         }
//     }

    return TRUE;

}

BOOL CGamingHouse::DelGamingPlayer(WORD wListID, CPlayer *pPlayer)
{
    if(pPlayer == NULL)
        return FALSE;

    CGamingHouse::GAMINGPLAYER *pGPlayer = GetPlayer(wListID);
    if(pGPlayer == NULL)
        return FALSE;

	if (dwt::strcmp(pGPlayer->szName, pPlayer->GetName(), CONST_USERNAME) == 0)
    {       
        if(wListID == 0) 
        {
            GamingOver(); 
        }
        else
        {
            if( (pGPlayer->dwWager > 0) && (!_winSet.Locate(wListID)) )
                PayToMaster(pGPlayer);
        }

        --_wCurPlayerNumber;
		ZeroMemory(pGPlayer->szName, CONST_USERNAME);
        
        if(wListID == 0)
            SendOnePlayerInfoToAll(pGPlayer);
        else
            SendOnePlayerInfoToAll(pGPlayer, pGPlayer->dnid);
        
        ZeroMemory(pGPlayer, sizeof(GAMINGPLAYER));

        if(_bNewLoop)
        {
            if(wListID != 0)
                --_wCount;
        }
        else
        {
            if(_wCurPlayerPos == wListID)
                GamingCast(pGPlayer);
        }

        pPlayer->SetSportsRoomID(0);

    }

    return TRUE;
}

void CGamingHouse::RecvWagerMessage(SQShowWagerInfoMsg *pMsg, CPlayer *pPlayer)
{
    if(!_bNewLoop)
        return;

    if(pMsg == NULL || pPlayer == NULL)
        return;

    if(pMsg->dwWager == 0 || pPlayer->m_Property.m_Money == 0)
    {
        --_wCount;
        return;
    }

    CGamingHouse::GAMINGPLAYER *pGPlayer = GetPlayer(pMsg->wPosID);
    if(NULL == pGPlayer)
        return;
        
	if (dwt::strcmp(pGPlayer->szName, pPlayer->GetName(), CONST_USERNAME) == 0)
    {
        if(pMsg->dwWager > pPlayer->m_Property.m_Money)
            pMsg->dwWager = pPlayer->m_Property.m_Money;

        pGPlayer->dwWager = pMsg->dwWager;
        pPlayer->m_Property.m_Money -= (pGPlayer->dwWager) > pPlayer->m_Property.m_Money ? 
            pPlayer->m_Property.m_Money : pGPlayer->dwWager;

        // 扣除玩家身上的钱
        pGPlayer->dwMoney = pPlayer->m_Property.m_Money;

//         SABuy msg;
//         msg.nRet = 1;
//         msg.nMoney = pPlayer->m_Property.m_Money;
//         msg.bShowMessage = false;
//         g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABuy));

        // 向赌场所有人员更新我的数据
        SendOnePlayerInfoToAll(pGPlayer);

        // 记录交了赌资的人
        _loseSet.Add(pGPlayer->wPosID);
    }
}

void CGamingHouse::RecvGamingcastMessage(SQShowGamingcastMsg *pMsg, CPlayer *pPlayer)
{
    if(pMsg == NULL || pPlayer == NULL)
        return;

    if(_bNewLoop)
        return;

    CGamingHouse::GAMINGPLAYER *pGPlayer = GetPlayer(pMsg->wPosID);
    if(NULL == pGPlayer)
        return;

    if(pGPlayer->wPosID != _wCurPlayerPos)
        return;

	if (dwt::strcmp(pGPlayer->szName, pPlayer->GetName(), CONST_USERNAME) == 0)
    {
        GamingCast(pGPlayer);
    }
}

void CGamingHouse::RecvGamingPlayerRequestMessage(
    SQGamingPlayerRequestMsg *pMsg, class CPlayer *pPlayer)
{
    if(pMsg == NULL || pPlayer == NULL)
        return;  

    switch(pMsg->wRequest)
    {
    case SQGamingPlayerRequestMsg::GPR_LEAVE:
        DelGamingPlayer(pMsg->wPosID, pPlayer);
        break;

    default:
        return;
    }
}
