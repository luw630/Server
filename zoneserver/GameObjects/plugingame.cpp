#include "stdafx.h"
#include "plugingame.h"
#include <string>
#include <vector>
//#include "LifeManager.h"PlayerManager.h
#include <algorithm>
#include "networkmodule/PlayerTypedef.h"
#include "GameWorld.h"
#include "ItemUser.h"
#include "plugingame_happy10_5.h"

static vector<CPlayer*> gArrTmpPlayer;		//todo: Single-Thread
static CPlugInGameManager* m_pThis = NULL;

class _CRegGame
{
public:
	_CRegGame()
	{		
		CPlugInGameManager::GetInstance()->RegPlugInGameFromFile("data\\游戏大厅\\基础配置.txt");		
	}

	~_CRegGame()
	{
		CPlugInGameManager::FreeInstance();
	}

};
_CRegGame a;

extern LPIObject GetPlayerByGID(DWORD);
extern LPIObject GetPlayerByDnid(DNID dnidClient);
extern CGameWorld *&GetGW();
#define _GetGameTick()	(GetGW()->GetGameTick())

SGameTable::SGameTable(_RegPluginGame *info, SGameRoom *pRoom, WORD wTableId)
{
	__bAsyncTryStartGame = FALSE;
	wCurtPlayerCnt = 0;
	this->pRoom = pRoom;
	this->wTableId = wTableId;

	m_vtSeats.resize(info->wMaxPlayerCnt);
	for (int i = 0; i < (int) info->wMaxPlayerCnt; i++)
	{
		m_vtSeats[i] = 0;
	}

	pOneGame = CPlugInGameManager::GetInstance()->CreatePluginGameInst(info->wGameId);	
	pOneGame->m_pTable = this;
	pOneGame->SetGameStatus(gtsUnknown);
	pOneGame->InitData(info);
}

SGameTable::~SGameTable()
{
	if (NULL != pOneGame)
		delete pOneGame;
	pOneGame = NULL;
}

BOOL SGameTable::GetSeatPlayer(WORD wSeatId, DWORD & dwOutPlayerID)
{
	if (wSeatId == PLUGINGAME_NULL_0 || wSeatId > m_vtSeats.size())
	{
		dwOutPlayerID = NULL_PLAYER_ID;
		return FALSE;
	}
	else
	{
		dwOutPlayerID = m_vtSeats[wSeatId - 1];
		return TRUE;
	}
}

BOOL SGameTable::PlayerLeaveSeat(WORD wSeatId)
{
	if (wSeatId == 0 || wSeatId > m_vtSeats.size())
	{
		return FALSE;
	}

    if (NULL == pOneGame)
        return FALSE;

	pOneGame->PlayerLeaveGame(wSeatId);
	m_vtSeats[wSeatId - 1] = NULL_PLAYER_ID;

	if (wCurtPlayerCnt > 0)
	{
		wCurtPlayerCnt --;
	}
	else
	{
		//fk...
	}

	return TRUE;
}

WORD SGameTable::GetGameId()
{ 
    if ( pRoom && pRoom->pArea && pRoom->pArea->pFloor && pRoom->pArea->pFloor->pGameIntro )
	    return pRoom->pArea->pFloor->pGameIntro->gameCode.wGameId; 
    return 0;
};

WORD SGameTable::GetRoomId()
{
    if ( pRoom )
	    return pRoom->wRoomId; 
    return 0;
};

WORD SGameTable::GetAreaId()
{
    if ( pRoom && pRoom->pArea )
	    return pRoom->pArea->wAreaId; 
    return 0;
};	
SGameIntro* SGameTable::GetGameIntro()
{
    if ( pRoom && pRoom->pArea && pRoom->pArea->pFloor )
	    return pRoom->pArea->pFloor->pGameIntro;
    return NULL;
};

void SGameTable::Update(int nTick)
{
	if (NULL == pOneGame)
		return;

	switch (pOneGame->GetGameStatus())
	{
	case gtsPlaying:
		{
			pOneGame->Update(nTick);
		}
		break;
	default:
		{		
			if (wCurtPlayerCnt == 0)
				return;

			SGamePlayer *pPlayer = NULL;
			for (int i = 0; i < (int) m_vtSeats.size(); i++)
			{
				pPlayer = CPlugInGameManager::GetInstance()->GetGamePlayer(m_vtSeats[i]);
				if (NULL == pPlayer)
					continue;

				if (pPlayer->enGameStatus == ppsWaitting4Start)
				{
					//nothing.
				}
				else
				{
					if (pPlayer->enGameStatus != ppsReadying)
					{
						return;
					}

					assert(pPlayer->enGameStatus == ppsReadying);	//todo: 如果玩家被‘顶掉、 则不满足条件， 注意！！！, 所以在另外 ‘重新上线'的地方加了处理
					if ((nTick - pPlayer->dwLastSitDwonTime) > TICKS_PER_SECOND * 30)
					{
						CPlayer *pOutterPlayer = (CPlayer *)GetPlayerByGID(pPlayer->dwID)->DynamicCast(IID_PLAYER);
                        assert (pOutterPlayer != NULL);	// 是可能的???可能 玩家游戏中意外退出
                        if ( pOutterPlayer == NULL )
                        {
                            CPlugInGameManager::GetInstance()->QuietlyQuit( pPlayer, gpOuterWorld );
                            return;
                        }
						//{

						//}
						//else
						{
							// 你离开了游戏座位， 因为你长时间没有开始游戏                          
							int result = CPlugInGameManager::GetInstance()->PlayerLeaveTable(pPlayer->posInfo.wGameId, NULL, pOutterPlayer);
                            if ( result < 0 )
                                CPlugInGameManager::GetInstance()->SendPlugInGameErr( pOutterPlayer->m_ClientIndex, result );
                            else
                                CPlugInGameManager::GetInstance()->SendPlugInGameErr( pOutterPlayer->m_ClientIndex, -11 );
						}
					}
				}
			}

			if (__bAsyncTryStartGame)
			{
				__bAsyncTryStartGame = FALSE;
				CPlugInGameManager::GetInstance()->TryStartGame(this);
			}
		}
		break;
	}

}

CPlugInGameManager::CPlugInGameManager()
{
	gArrTmpPlayer.resize(PLUGINGAME_MAX_ONE_GAME_PLAYER_CNT);

    m_bOpen = TRUE;
}

CPlugInGameManager::~CPlugInGameManager()
{
	if (m_mapAllGames.size() > 0)
	{
		for (map<DWORD, SGameIntro*>::iterator it = m_mapAllGames.begin(); it != m_mapAllGames.end(); ++it)
		{
			delete it->second;
            it->second = NULL;
		}
	}
}

CPlugInGameManager* CPlugInGameManager::GetInstance()
{
	if (NULL == m_pThis)	
		m_pThis = new CPlugInGameManager();

	return m_pThis;
}

void CPlugInGameManager::FreeInstance()
{
	if (NULL != m_pThis)
	{
		delete m_pThis;
		m_pThis = NULL;
	}
}

SGameIntro* CPlugInGameManager::GetGameIntro(DWORD dwGameId)
{
	map<DWORD, SGameIntro*>::iterator it = m_mapAllGames.find(dwGameId);

	if (m_mapAllGames.end() == it)
	{
		return NULL;
	}

	return it->second;
}

EPluginGame_PlayerStatus CPlugInGameManager::GetPlayerGamesStatus(DWORD dwPlayerId)
{
	map<DWORD, SGamePlayer>::iterator it;
	it = m_mapAllPlayers.find(dwPlayerId);
	if (m_mapAllPlayers.end() == it)
	{
		return ppsNone;
	}

	SGamePlayer *pPlayer = &(it->second);
	return pPlayer->enGameStatus;
}

BOOL CPlugInGameManager::SetPlayerGameStatus(DWORD dwPlayerId, EPluginGame_PlayerStatus status)
{
	map<DWORD, SGamePlayer>::iterator it;
	it = m_mapAllPlayers.find(dwPlayerId);
	if (m_mapAllPlayers.end() == it)
	{
		return FALSE;
	}

	SGamePlayer *pPlayer = &(it->second);
	pPlayer->enGameStatus = status;

	return TRUE;
}

SGamePlayer* CPlugInGameManager::GetGamePlayer(DWORD dwPlayerId)
{
	map<DWORD, SGamePlayer>::iterator it;
	it = m_mapAllPlayers.find(dwPlayerId);
	if (m_mapAllPlayers.end() == it)
	{
		return NULL;
	}

	return &(it->second);
}

CPlugInGame* CPlugInGameManager::CreatePluginGameInst(WORD wGameId)
{
	switch(wGameId)
	{
	case PLUGINGAME_GAME_INDEX_HAPPY10_5:
		return new CHappy10_5(wGameId);
		break;
	default:
		return NULL;
	}
}

int CPlugInGameManager::RegPlugInGame(_RegPluginGame regInfo)
{
	//#define return_err(n, strDesc) {nErr = n; strMsg = strDesc; goto _err;}
#define return_err(n, strDesc) {return n;}

	int nErr = 0;
	string strMsg = "";

	if (regInfo.wAreaCnt > PLUGINGAME_MAX_AREA_CNT || regInfo.wRoomCnt > PLUGINGAME_MAX_ROOM_CNT
		|| regInfo.wTableCnt > PLUGINGAME_MAX_TABLE_CNT || regInfo.wMaxPlayerCnt > PLUGINGAME_MAX_SEAT_CNT
		|| regInfo.wMinPlayerCnt < 2)
	{		
		//OutPutInfo("赌场注册游戏%d参数不符合要求！", (int)regInfo.wGameId);
		return_err(-2, "参数不符合要求。")
	}

	map<DWORD, SGameIntro*>::iterator it = m_mapAllGames.find(regInfo.wGameId);	
	if (m_mapAllGames.end() != it)
	{
		//OutPutInfo("赌场重复注册游戏%d失败", (int)regInfo.wGameId);
		return_err(-3, "重复注册失败。");
	}	

	SGameIntro *pIntro = new SGameIntro();
    if ( pIntro == NULL )
        return -1;

	pIntro->gameCode = regInfo;	
	pIntro->pGameData = new SGameFloor(&regInfo);
	pIntro->pGameData->pGameIntro = pIntro;

	m_mapAllGames.insert(pair<DWORD, SGameIntro*>(regInfo.wGameId, pIntro));

	return nErr;
}

void CPlugInGameManager::Update(int nTick)
{
	static int nLast = 0;
	if (nTick - nLast < TICKS_PER_SECOND * 1)
		return;
	nLast = nTick;

	SGameIntro *pInst = NULL;
	if (m_mapAllGames.size() == 0)
		return;

	for (map<DWORD, SGameIntro*>::iterator it = m_mapAllGames.begin(); it != m_mapAllGames.end(); ++it)
	{
		pInst = it->second;
		if (pInst && pInst->pGameData &&  pInst->pGameData->m_vtAreas.size() > 0)
		{
			for (int i = 0; i < (int) pInst->pGameData->m_vtAreas.size(); i++)
			{
                if ( pInst->pGameData->m_vtAreas[i] )
				    pInst->pGameData->m_vtAreas[i]->Update(nTick);
			}
		}
	}
}

BOOL CPlugInGameManager::CheckPlayerExists(DWORD dwID)
{
	map<DWORD, SGamePlayer>::iterator itPlayer;
	itPlayer = m_mapAllPlayers.find(dwID);
	return (itPlayer != m_mapAllPlayers.end());
}


int CPlugInGameManager::QuietlyQuit(SGamePlayer* pPlayer, int nQuit2Where)
{
	if (NULL == pPlayer)
		return -1;

	PlayerLeaveTable(pPlayer->posInfo.wGameId, pPlayer, NULL, FALSE);

	switch(nQuit2Where)
	{
	case gpRoom:
	case gpArea:
	case gpHall:
		{			

		}
		break;
	case gpOuterWorld:
	default:
		{
			map<DWORD, SGamePlayer>::iterator itPlayer = m_mapAllPlayers.find(pPlayer->dwID);
			if (CPlugInGameManager::GetInstance()->m_mapAllPlayers.end() != itPlayer)
			{
				m_mapAllPlayers.erase(itPlayer);
			}
		}
		break;
	}	

	return 0;
}


//需要调用者保证参数正确！！！
int CPlugInGameManager::BroadCastMsg(int nToWhere, void *pMsg, int nSize, WORD wGameId /* = 0 */, WORD wAreaId /* = 0 */, WORD wRoomId /* = 0 */, WORD wTableId /* = 0 */)
{
	//todo: 效率问题
    if ( pMsg == NULL )
        return -1;

	if (wGameId == 0 && nToWhere != gpHall)
		return -1;

    CPlayer *pTargetPlayer = NULL;

	switch(nToWhere)
	{
	case gpHall:        // 可以考虑换成GM喇叭
		{
            for (map<DWORD , SGamePlayer>::iterator it = m_mapAllPlayers.begin(); it != m_mapAllPlayers.end(); ++it)
            {
                SGamePlayer *pPlayer =&(it->second);
                if ( !pPlayer )
                    continue;

                pTargetPlayer = ( CPlayer* )GetPlayerByGID( pPlayer->dwID )->DynamicCast( IID_PLAYER );
                if (NULL != pTargetPlayer)
                    g_StoreMessage( pTargetPlayer->m_ClientIndex, pMsg, nSize );             
            }
		}
		break;
	case gpArea:
		{
			if (wAreaId == PLUGINGAME_NULL_0)
			{
				return -1;
			}

			for (map<DWORD , SGamePlayer>::iterator it = m_mapAllPlayers.begin(); it != m_mapAllPlayers.end(); ++it)
			{
				SGamePlayer *pPlayer =&(it->second);
                if ( !pPlayer )
                    continue;

				if ((pPlayer->posInfo.wGameId == wGameId &&  pPlayer->posInfo.wAreaId == wAreaId)
					||(pPlayer->tmpPos.wGameId == wGameId && pPlayer->tmpPos.wAreaId == wAreaId))
				{
					pTargetPlayer = ( CPlayer* )GetPlayerByGID( pPlayer->dwID )->DynamicCast( IID_PLAYER );
					if (NULL != pTargetPlayer)
                        g_StoreMessage( pTargetPlayer->m_ClientIndex, pMsg, nSize );
				}

			}
		}
		break;
	case gpRoom:
		{
            if ( !GetGameIntro(wGameId) || !(GetGameIntro(wGameId)->pGameData) || !(GetGameIntro(wGameId)->pGameData->GetArea(wAreaId)) )
                break;

			SGameRoom *pRoom = GetGameIntro(wGameId)->pGameData->GetArea(wAreaId)->GetRoom(wRoomId);
            if ( !pRoom )
                break;

			for (int i = 0; i < (int) pRoom->m_vtTables.size(); i++)
			{
				SGameTable *pTable = pRoom->m_vtTables[i];
                if ( !pTable )
                    continue;

				for (int i = 0; i < (int) pTable->m_vtSeats.size(); i++)
				{
					CPlayer *pPlayer = ( CPlayer* )GetPlayerByGID( pTable->m_vtSeats[i] )->DynamicCast( IID_PLAYER );
					if (NULL != pPlayer)
                        g_StoreMessage( pPlayer->m_ClientIndex, pMsg, nSize); 
				}
			}

			//for (map<DWORD , SGamePlayer>::iterator it = m_mapAllPlayers.begin(); it != m_mapAllPlayers.end(); ++it)
			//{
			//	SGamePlayer *pPlayer =&(it->second);
			//	if (pPlayer->posInfo.wGameId == dwGameId &&  pPlayer->posInfo.wAreaId == dwAreaId && pPlayer->posInfo.wRoomId == dwRoomId)
			//	{
			//		pTargetPlayer = g_pLifeManager->GetPlayer(pPlayer->dwID);
			//		if (NULL != pTargetPlayer)
			//		{
			//			pTargetPlayer->SendMessage(pMsg, nSize);
			//		}
			//	}

			//}
		}
		break;
	case gpTable:
	default:
		{
            if ( !GetGameIntro(wGameId) || !(GetGameIntro(wGameId)->pGameData) || !(GetGameIntro(wGameId)->pGameData->GetArea(wAreaId)) 
                || !(GetGameIntro(wGameId)->pGameData->GetArea(wAreaId)->GetRoom(wRoomId)) )
                break;

			SGameTable *pTable = GetGameIntro(wGameId)->pGameData->GetArea(wAreaId)->GetRoom(wRoomId)->GetTable(wTableId);
            if ( !pTable )
                break;

			for (int i = 0; i < (int) pTable->m_vtSeats.size(); i++)
			{
				CPlayer *pPlayer = ( CPlayer* )GetPlayerByGID( pTable->m_vtSeats[i] )->DynamicCast( IID_PLAYER );
				if (NULL != pPlayer)
                     g_StoreMessage( pPlayer->m_ClientIndex, pMsg, nSize); 
			}

			//for (map<DWORD , SGamePlayer>::iterator it = m_mapAllPlayers.begin(); it != m_mapAllPlayers.end(); ++it)
			//{
			//	SGamePlayer *pPlayer =&(it->second);
			//	if (pPlayer->posInfo.wGameId == dwGameId 
			//		&&  pPlayer->posInfo.wAreaId == dwAreaId 
			//		&& pPlayer->posInfo.wRoomId == dwRoomId 
			//		&& pPlayer->posInfo.wTableId == dwTableId)
			//	{
			//		pTargetPlayer = g_pLifeManager->GetPlayer(pPlayer->dwID);
			//		if (NULL != pTargetPlayer)
			//		{
			//			pTargetPlayer->SendMessage(pMsg, nSize);
			//		}
			//	}

			//}
		}
		break;
	}

	return 0;
}

int CPlugInGameManager::ForceQuit(DWORD dwPlayerId) //todo
{
	SGamePlayer *pPlayer = GetGamePlayer(dwPlayerId);
    if ( pPlayer == NULL )
        return 0;
	QuietlyQuit(pPlayer, gpOuterWorld);

	return 0;
}

int CPlugInGameManager::RecMsg(DNID dnidClient, SPluginGameBaseMsg *pMsg)
{
    if ( pMsg == NULL )
        return 0;

    if ( m_bOpen == FALSE && pMsg->_protocol != SPluginGameBaseMsg::MSG_PLUGINGAME_CS_LEAVE_TABLE 
        && pMsg->_protocol != SPluginGameBaseMsg::MSG_PLUGINGAME_CS_EXIT_HALL )
    {
        SendPlugInGameErr( dnidClient, -200 );
        return 0;
    }

	int nRst = ParserMsg(dnidClient, pMsg) ;
	if (0 != nRst)	//max record cnt
	{
		CPlayer *pPlayer = ( CPlayer* )GetPlayerByDnid(dnidClient)->DynamicCast(IID_PLAYER);;

		if (NULL != pPlayer)
		{
			if ( m_lstBadGuys.size() > 1000000)
			{
				m_lstBadGuys.erase(m_lstBadGuys.begin(), m_lstBadGuys.begin() ++);
			}

			m_lstBadGuys.push_back(pPlayer->GetGID());
		}
        
        // 发送失败信息 用于提示
        if ( nRst < 0 )
            SendPlugInGameErr( pPlayer->m_ClientIndex, nRst );
	}

	return nRst;
}

int CPlugInGameManager::ParserMsg(DNID dnidClient, SPluginGameBaseMsg *pMsg)
{
	if (NULL == pMsg)
		return -1;

	CPlayer *pOutterPlayer = ( CPlayer* )GetPlayerByDnid(dnidClient)->DynamicCast(IID_PLAYER);
	if (NULL == pOutterPlayer)
		return -2;

    SGamePlayer *pPlayer = GetGamePlayer(pOutterPlayer->GetGID());

	switch(pMsg->_protocol)
	{
	case SPluginGameBaseMsg::MSG_PLUGINGAME_CS_ENTER_HALL:  // 进入大厅
		{
			if (NULL == pPlayer)  // 刚进游戏
			{
				SGamePlayer aGamePlayer;
				aGamePlayer.dwID = pOutterPlayer->GetGID();	
				strncpy(aGamePlayer.strName, pOutterPlayer->GetName(), MAX_NAMELEN);
				aGamePlayer.enGameStatus = ppsHall;
				m_mapAllPlayers.insert(pair<DWORD, SGamePlayer>(aGamePlayer.dwID, aGamePlayer));
			}
			else
			{
				if (pPlayer->enGameStatus >= ppsGameLine)
				{
					PlayerLeaveTable(pPlayer->posInfo.wGameId, pPlayer, pOutterPlayer, FALSE);
				}
			}

			//2.发送可玩游戏列表
			if (m_mapAllGames.size() == 0)
			{
				//pOutterPlayer->SendSysMessageBox("暂未开放.");
				return -13;
			}

			sc_PlugInGame_GameList aGame;
			SGameIntro *pInst = NULL;
			for (map<DWORD, SGameIntro*>::iterator it = m_mapAllGames.begin(); it != m_mapAllGames.end(); ++it)
			{
                if ( it->second == NULL )
                    continue;

				pInst = it->second;
				aGame.dwGameId = pInst->gameCode.wGameId;
				strncpy(aGame.strGameName, pInst->gameCode.strGameName, PLUGINGAME_MAX_GAME_NAME_LEN);

				g_StoreMessage(dnidClient, &aGame, sizeof aGame);
			}
		}  
		break;
	case SPluginGameBaseMsg::MSG_PLUGINGAME_CS_EXIT_HALL:   // 离开大厅
		{
			if (NULL != pPlayer)
				QuietlyQuit(pPlayer, gpOuterWorld);
		}
		break;
	case SPluginGameBaseMsg::MSG_PLUGINGAME_CS_SELECT_GAME: // 选择游戏
		{
			//1.检验规则
            // 你还没有进入大厅
			if (NULL == pPlayer)        
				return -2;

			//if (pPlayer->enGameStatus != ppsHall)
			//{	
			//	pOutterPlayer->SendSysMessageBox("请先进入游戏大厅.");
			//	return -1;
			//}

			//2. 选择游戏

			cs_PlugInGame_SelGame *pData = (cs_PlugInGame_SelGame*)pMsg;		
			SGameIntro *pInst = NULL;
			pInst = GetGameIntro(pData->dwGameId);
            // 选择的游戏不存在
			if (NULL == pInst)         
				return -3;

			pPlayer->tmpPos.wGameId = (WORD)pData->dwGameId;

			//pPlayer->posInfo.wGameId = pData->dwGameId;
			//pPlayer->enGameStatus = ppsSelecting;

			//3. 发送分区信息
			cs_PlugInGame_AreaList aArea;
			SGameArea *pArea = NULL;
			for (int i = 0; i < (int) pInst->pGameData->m_vtAreas.size(); ++i)			
			{
				pArea = pInst->pGameData->m_vtAreas[i];
				aArea.wGameId = (WORD)pData->dwGameId;
				aArea.wAreaId = (WORD)(i+1);
				aArea.wCurtPlayerCnt = (WORD)pArea->wCurtPlayerCnt;
				aArea.wMaxPlayerCnt = (WORD)pArea->wPlayerCapcity;
				aArea.byMoneyType = (BYTE)pInst->gameCode.areaParms[i].arrParm10[0];
				aArea.dwMoneyCnt = pInst->gameCode.areaParms[i].arrParm10[1];
				aArea.dwLotteryMoney = pInst->gameCode.areaParms[i].arrParm10[2];
                aArea.dwBaseTaxMoney = pInst->gameCode.areaParms[i].arrParm10[3];
                aArea.wTaxRate = (WORD)pInst->gameCode.areaParms[i].arrParm10[4];

				g_StoreMessage(dnidClient, &aArea, sizeof aArea);
			}

		}
		break;
	case SPluginGameBaseMsg::MSG_PLUGINGAME_CS_SELECT_GAME_AREA:    // 选择意向游戏分区
		{
			//1.检验规则
            // 你还没有进入大厅
			if (NULL == pPlayer)            
				return -2;

			//if (pPlayer->enGameStatus != ppsSelecting)
			//{	
			//	pOutterPlayer->SendSysMessageBox("还没有进入大厅.");
			//	return -1;
			//}

			//SGameIntro *pGame = GetGameIntro(pPlayer->posInfo.wGameId);
			//if (NULL == pGame)
			//{
			//	pOutterPlayer->SendSysMessageBox("还没有选择游戏呢.");
			//	return -3	;
			//}

			cs_PlugInGame_SelArea *pData = (cs_PlugInGame_SelArea*)pMsg;
			SGameIntro *pGameIntro = GetGameIntro(pData->wGameId);
            // 错误的游戏编号
			if (NULL == pGameIntro)
				return -3;			

			SGameArea *pArea = pGameIntro->pGameData->GetArea(pData->wAreaId);
            // 错误的游戏分区
			if (NULL == pArea)
				return -4;

			SRegAreaParm *pParm = &pGameIntro->gameCode.areaParms[pData->wAreaId - 1];
            // 暂未开放
			if (!pParm->bIsOpen)
				return -13;

			//2. 进入区域	
			//////////if (pPlayer->enGameStatus > ppsGameLine)
			//////////{
			//////////	assert(pPlayer->wPossibleAreaId == PLUGINGAME_NULL_0);
			//////////}

			//////////if (!(pPlayer->wPossibleAreaId == pData->wAreaId && pPlayer->wPossibleGameId == pData->wGameId))
			//////////{
			//////////	QuitPossibleArea(pPlayer);
			//////////	pPlayer->wPossibleGameId = pData->wGameId;
			//////////	pPlayer->wPossibleAreaId = pData->wAreaId;
			//////////	pArea->m_mapAllPlayers.insert(pair<DWORD, DWORD>(pPlayer->dwID, 0));
			//////////}

			pPlayer->tmpPos.wAreaId = pData->wAreaId;
			

			//发送房间列表
			sc_PlugInGame_RoomList aRoom;
			SGameRoom *pRoom = NULL;
			for (int i = 0; i < (int) pArea->m_vtRooms.size(); i++)
			{
				pRoom = pArea->m_vtRooms[i];
				aRoom.wGameId = pData->wGameId;
				aRoom.wAreaId = pData->wAreaId;
				aRoom.wRoomId = (WORD)(i+1);
				aRoom.wCurtPlayerCnt = (WORD)pRoom->wCurtPlayerCnt;
				aRoom.wMaxPlayerCnt = (WORD)pRoom->wPlayerCapcity;

				SGameTable *pTable = pArea->GetRoom(i+1)->GetTable(1); //first table;

				aRoom.byFirstTableStatus = pTable->pOneGame->GetGameStatus() == gtsPlaying ? 1 : 0;
				g_StoreMessage(dnidClient, &aRoom, sizeof aRoom);
			}

		}
		break;

	case SPluginGameBaseMsg::MSG_PLUGINGAME_CS_SELECT_ROOM:
		{
			//1.检验规则
            // 死亡
            if ( pOutterPlayer->GetCurActionID() == EA_DEAD )
                return -41;

			cs_PlugInGame_SelectRoom *pData = (cs_PlugInGame_SelectRoom*)pMsg;
            // 你还没有进入大厅
			if (NULL == pPlayer)        
				return -1;

			SGameIntro *pGame = GetGameIntro(pData->wGameId);
            // 错误的游戏编号
			if (NULL == pGame)
				return -3;

			SGameArea *pArea = pGame->pGameData->GetArea(pData->wAreaId);
            // 错误的游戏分区
			if (NULL == pArea)
				return -4;

			SGameRoom *pRoom = pArea->GetRoom(pData->wRoomId);
            // 错误的游戏房间
			if (NULL == pRoom)
				return -5;

			//2.选择房间
			pPlayer->tmpPos.wRoomId = 0; //todo: 目前暂时不用， 等房间开多张桌子后再说。


			//3. 发送桌子基本信息
			sc_PlugInGame_TableList aTable;			
			aTable.wGameId = pData->wGameId;
			aTable.wAreaId = pData->wAreaId;
			aTable.wRoomId = pData->wRoomId;
			for (int i = 0; i < (int) pRoom->m_vtTables.size(); i++)
			{
				aTable.wTableCnt = (WORD)pRoom->m_vtTables.size();
				g_StoreMessage(dnidClient, &aTable, sizeof aTable);
			}

			//4. 具体游戏同步游戏状态 //todo:..........暂时不需要， 现在一个房间只有一个桌子
		}
		break;
	case SPluginGameBaseMsg::MSG_PLUGINGAME_CS_SELECT_TABLE:
		{
			//1.检验规则
// 			BOOL bAllowStandBy = FALSE;
// 			BOOL bIsStandBy = FALSE;
// 
//             // 死亡
//             if ( pOutterPlayer->GetCurActionID() == EA_DEAD )
//                 return -41;
// 
// 			cs_PlugInGame_SelTable *pData = (cs_PlugInGame_SelTable*)pMsg;
//             // 你还没有进入大厅
// 			if (NULL == pPlayer)        
// 				return -2;
// 
//             // 请先离开游戏
// 			if (pPlayer->enGameStatus >= ppsGameLine)
// 				return -21;
// 
// 			if (pData->wGameId == pPlayer->posInfo.wGameId
// 				&& pData->wAreaId == pPlayer->posInfo.wAreaId
// 				&& pData->wRoomId == pPlayer->posInfo.wRoomId
// 				&& pData->wTableId == pPlayer->posInfo.wTableId)	//重复选择可以理解为在当前房间换位置？ 目前不允许
// 			{
// 				// 不能变更座位
// 				return -22;
// 			}
// 			//else
// 			//{
// 			//	PlayerLeaveTable(pPlayer->posInfo.wGameId/*TODO: 多个game支持问题*/, pOutterPlayer);
// 			//	return -70;
// 			//}
// 
// 			SGameIntro *pGame = GetGameIntro(pData->wGameId);
//             // 错误的游戏编号
// 			if (NULL == pGame)
// 				return -3;
// 
// 			SGameArea *pArea = pGame->pGameData->GetArea(pData->wAreaId);
//             // 错误的游戏分区
// 			if (NULL == pArea)
// 				return -4;
// 
// 			SGameRoom *pRoom = pArea->GetRoom(pData->wRoomId);
//             // 错误的游戏房间
// 			if (NULL == pRoom)
// 				return -5;
// 
// 			SGameTable *pTable = pRoom->GetTable(pData->wTableId);	
//             // 错误的游戏桌子
// 			if (NULL == pTable)
// 				return -6;
// 
//             // 你正在交易， 不能开始游戏
// 			if (pOutterPlayer->InExchange())
// 				return -12;
// 
// 			if (pTable->pOneGame->GetGameStatus() == gtsPlaying && !pGame->gameCode.bAllowStandBy)
// 			{
// 				if (pGame->gameCode.bAllowStandBy)	//人数不固定
// 				{
// 					//ok, 可以尝试加入
// 				}
// 				else	//人数固定的游戏，开始后就不能再加入了（其实人肯定也满了），  Todo: 暂不考虑完全旁观者
// 				{		
// 					// 游戏已经开始了
// 					return -7;
// 				}				
// 			}
// 
// 			//检查底金是否足够（不要求的则配置为：0）
// 			int nMoneyType = pGame->gameCode.areaParms[pData->wAreaId - 1].GetMoneyType();		
// 			if (nMoneyType >= MoneyType_Gold && nMoneyType <= MoneyType_Gift)
// 			{
// 				DWORD dwMoneyCnt = pGame->gameCode.areaParms[pData->wAreaId - 1].GetEnterMoneyCnt();
//                 // 你的底金不足， 改日再来吧
// 				if (pOutterPlayer->GetMoney( nMoneyType ) < dwMoneyCnt)
// 					return -10;
// 			}
// 
// 			WORD wSeatId = 0;
// 			if (pData->byAutoSel)
// 			{				
// 				for (int i = 0; i < (int) pTable->m_vtSeats.size(); i++)
// 				{
// 					if (pTable->m_vtSeats[i] == NULL_PLAYER_ID)
// 					{
// 						wSeatId = i + 1;
// 						break;
// 					}
// 				}
// 			}
// 			else	//目前都是auto
// 			{
// 				DWORD dwNowPlayer = 0;
//                 // 没有空位了
// 				if (!pTable->GetSeatPlayer(pData->bySeatId, dwNowPlayer))
// 					return -8;
// 
// 				if (dwNowPlayer == NULL_PLAYER_ID)
// 				{
// 					wSeatId = pData->bySeatId;
// 				}
// 			}
// 
// 			if (wSeatId == 0)
// 			{
// 				return 0; //不用广播
// 			}
// 			else //进入桌子， 分2中情况， 1： 正在游戏状态（只有bAllowStandBy才能加入）， 2：等待状态
// 			{
// 				//标记位置
// 				pPlayer->posInfo.wGameId = pData->wGameId;
// 				pPlayer->posInfo.wAreaId = pData->wAreaId;
// 				pPlayer->posInfo.wRoomId = pData->wRoomId;
// 				pPlayer->posInfo.wTableId = pData->wTableId;
// 				pPlayer->posInfo.wSeatId = wSeatId;	
// 
// 				//标记状态
// 				if (pTable->pOneGame->GetGameStatus() == gtsPlaying)
// 				{					
// 					pPlayer->SetStatus(ppsStandingBy);					
// 				}
// 				else
// 				{
// 					pPlayer->SetStatus(ppsReadying);
// 				}
// 
// 				pPlayer->dwLastSitDwonTime = GetGW()->GetGameTick();
// 				pTable->m_vtSeats[wSeatId - 1] = pPlayer->dwID;					
// 
// 				//设置外界人数
// 				pTable->wCurtPlayerCnt ++;
// 				pTable->pRoom->wCurtPlayerCnt++;
// 				pTable->pRoom->pArea->wCurtPlayerCnt++;
// 
// 				//QuitPossibleArea(pPlayer); //todo: 优化一下。
// 				//pArea->m_mapAllPlayers.insert(pair<DWORD, DWORD>(pPlayer->dwID, 0));
// 				pRoom->m_mapAllPlayers.insert(pair<DWORD, DWORD>(pPlayer->dwID, 0));
// 
// 				///////////////////////////////////////////////////////////
// 
// 				///发送消息。
// 
// 				sc_PlugInGame_SyncTable aTable;
// 				aTable.wGameId = pPlayer->posInfo.wGameId;
// 				aTable.wAreaId = pPlayer->posInfo.wAreaId;
// 				aTable.wRoomId = pPlayer->posInfo.wRoomId;
// 				aTable.wTableId = pPlayer->posInfo.wTableId;
// 				strncpy(aTable.strPlayerName, pPlayer->strName, MAX_NAMELEN);
// 				aTable.bySeatId = (BYTE)wSeatId;
// 				aTable.dwPlayerId = pPlayer->dwID;
// 				aTable.byStatus = sc_PlugInGame_SyncTable::__tsSitDownORReadying;
// 
// 				//todo: 向当前房间内的所有人发包？？
// 				//暂时为向当前区域的人发包
// 				BroadCastMsg(gpRoom, &aTable, sizeof aTable, pPlayer->posInfo.wGameId, pPlayer->posInfo.wAreaId, pPlayer->posInfo.wRoomId);
// 
// 				//发送其他座位信息给自己
// 				SGamePlayer *pOtherPlayer = NULL;
// 				for (int i = 0; i < (int) pTable->m_vtSeats.size(); i++)
// 				{
// 					DWORD wId = pTable->m_vtSeats[i];
// 
// 					if (wId != NULL_PLAYER_ID)
// 					{
// 						if (wId == pPlayer->dwID)
// 						{
// 							continue;
// 						}
// 
// 						pOtherPlayer = GetGamePlayer(wId);
// 						if (NULL != pOtherPlayer)
// 						{							
// 							aTable.bySeatId = i+1;
// 							strncpy(aTable.strPlayerName, pOtherPlayer->strName, MAX_NAMELEN);
// 							aTable.dwPlayerId = wId;
// 							switch(pOtherPlayer->enGameStatus)
// 							{
// 							case ppsWaitting4Start:
// 								aTable.byStatus = sc_PlugInGame_SyncTable::__tsWaitting4Start;
// 								break;
// 							case ppsReadying:
// 							default:
// 								aTable.byStatus = sc_PlugInGame_SyncTable::__tsSitDownORReadying;
// 								break;
// 							}
// 							g_StoreMessage(dnidClient, &aTable, sizeof aTable);
// 						}
// 					}
// 
// 				}
// 
// 				//当前区域房间人数变化
// 				SyncAreaRoomsPlayerCnt(pData->wGameId, pData->wAreaId);
// 
// 				//同步头像信息
// 				sc_PlugInGame_SyncPlayerInfo info;
// 				info.wGameId = pData->wGameId;
// 				info.wAreaId = pData->wAreaId;
// 				info.wRoomId = pData->wRoomId;
// 				info.wTableId = pData->wTableId;
// 
// 
// 				//发送头像信息....(有用啊？)
// 				CPlayer *pOldPlayer = NULL;
// 				for (int i = 0; i < (int) pTable->m_vtSeats.size(); i++)
// 				{
// 					if (wSeatId == (i + 1))
// 						continue;
// 
// 					pOldPlayer = ( CPlayer* )GetPlayerByGID( pTable->m_vtSeats[i] )->DynamicCast( IID_PLAYER );
// 					if (NULL != pOldPlayer)
// 					{				
// 						info.wSeatId = wSeatId;
// 						info.bySchool = pOutterPlayer->m_Property.m_School;
// 						info.bySex = pOutterPlayer->m_Property.m_Sex;	
//                         g_StoreMessage( pOldPlayer->m_ClientIndex, &info, sizeof info);
// 
// 						info.wSeatId = i + 1;
// 						info.bySchool = pOldPlayer->m_Property.m_School;
// 						info.bySex = pOldPlayer->m_Property.m_Sex;
//                         g_StoreMessage( pOutterPlayer->m_ClientIndex, &info, sizeof info);
// 					}				
// 				}
// 
// 				pTable->pOneGame->PlayerEnterGame(pPlayer, pPlayer->posInfo.wSeatId);
// 			}
		}
		break;
	case SPluginGameBaseMsg::MSG_PLUGINGAME_CS_LEAVE_TABLE:
		{			
            int result = PlayerLeaveTable(0/*todo: not yet!*/, NULL, pOutterPlayer);
            if ( result < 0 )
                SendPlugInGameErr( pOutterPlayer->m_ClientIndex, result );

		}
		break;
	case SPluginGameBaseMsg::MSG_PLUGINGAME_CS_READY:
		{
			//1.检验规则
            // 你还没有进入大厅
			if (NULL == pPlayer)
				return -2;

			SGameIntro *pGame = GetGameIntro(pPlayer->posInfo.wGameId);
            // 还没有选择游戏
			if (NULL == pGame)
				return -3;

			SGameArea *pArea = pGame->pGameData->GetArea(pPlayer->posInfo.wAreaId);
            // 错误的游戏分区
			if (NULL == pArea)
				return -4;			

			SGameRoom *pRoom = pArea->GetRoom(pPlayer->posInfo.wRoomId);
            // 错误的游戏房间
			if (NULL == pRoom)
				return -5;

			SGameTable *pTable = pRoom->GetTable(pPlayer->posInfo.wTableId);
            // 错误的游戏桌
			if (NULL == pTable)
				return -6;			

			DWORD dwPlayerId = 0;
            // 错误的游戏桌子座位
			if (!pTable->GetSeatPlayer(pPlayer->posInfo.wSeatId, dwPlayerId))
				return -7;

            // 你不在座位
			if (dwPlayerId != pPlayer->dwID)
				return -8;

            // 正在游戏中。 bad client
			if (pTable->pOneGame->GetGameStatus() == gtsPlaying)
				return -9;

            // 你正在交易， 不能开始游戏
			if (pOutterPlayer->InExchange())
				return -12;

			//int nRst = pTable->pOneGame->CheckPlayerCanPlay(pOutterPlayer);

			//if (nRst == ecNone)
			//{
			//	//ok .go on.
			//}
			//else
			//{
			//	switch(nRst)
			//	{
			//	case ecNotEnoughtMoney:
			//	default:
			//		{
			//			pOutterPlayer->SendSysMessageBox("你的底金不够！");
			//			return -9;
			//		}
			//		break;
			//	}

			//	return - 20;
			//}


			//2. 设置状态
			pPlayer->enGameStatus = ppsWaitting4Start;

			//3. 发送消息
			sc_PlugInGame_SyncTable aTable;
			aTable.wGameId = pPlayer->posInfo.wGameId;
			aTable.wAreaId = pPlayer->posInfo.wAreaId;
			aTable.wRoomId = pPlayer->posInfo.wRoomId;
			aTable.wTableId = pPlayer->posInfo.wTableId;
			strncpy(aTable.strPlayerName, pPlayer->strName, MAX_NAMELEN);
			aTable.bySeatId = (BYTE)pPlayer->posInfo.wSeatId;
			aTable.dwPlayerId = pPlayer->dwID;
			aTable.byStatus = sc_PlugInGame_SyncTable::__tsWaitting4Start;

			// 向当前房间内的所有人发包？？
			//向区域发包
			BroadCastMsg(gpArea, &aTable, sizeof aTable, pPlayer->posInfo.wGameId, pPlayer->posInfo.wAreaId);

			//4. 游戏开始？ (房间内所有的人都ready ok) ,Todo: 注意这里开始游戏的逻辑改动必须同步修改TryStartGame(), 暂时的。以后重构之。
			WORD wReadyOkCnt = 0;
			for (int i = 0; i < (int) pTable->m_vtSeats.size(); i++)	
			{				
				SGamePlayer *_pGP = GetGamePlayer(pTable->m_vtSeats[i]);
				if (_pGP == NULL)
				{
					continue;
				}
				else
				{
					if (_pGP->enGameStatus == ppsWaitting4Start)
					{
						wReadyOkCnt ++;
					}
					else
					{
						wReadyOkCnt = 0;
						break;
					}
				}
			}

			if (wReadyOkCnt >= pGame->gameCode.wMinPlayerCnt)	//游戏开始
			{	
				int nRst = pTable->pOneGame->CheckCanStart();
				//assert(nRst == ecNone);
                // 无法开始游戏
				if (nRst != ecNone)
				{
					return -100;		//fk.
				}

				sc_PlugInGame_GameStart startInfo;
				startInfo.wGameId = pPlayer->posInfo.wGameId;
				startInfo.wAreaId = pPlayer->posInfo.wAreaId;
				startInfo.wRoomId = pPlayer->posInfo.wRoomId;
				startInfo.wTableId = pPlayer->posInfo.wTableId;

				DWORD dwParms[sc_PlugInGame_GameStart::MAX_PARM_CNT];
				WORD wParmCnt = 0;
				startInfo.byParmCnt = 0;
				pTable->pOneGame->GetStartParm(dwParms, wParmCnt);
				startInfo.byParmCnt = (BYTE)wParmCnt;
				for (WORD i = 0; i < wParmCnt; i++)
				{
					startInfo.arrParms[i] = dwParms[i];
				}

				//todo: room or area?	
				CPlugInGameManager::GetInstance()->BroadCastMsg(gpArea, &startInfo, startInfo.CalcSize(), pPlayer->posInfo.wGameId, pPlayer->posInfo.wAreaId);

				pTable->pOneGame->StartGame();	
			}	
		}
		break;
	case SPluginGameBaseMsg::MSG_PLUGINGAME_CS_CHAT_PUBLIC:
		{
			//1.检验规则
            // 你还没有进入大厅
			if (NULL == pPlayer)
				return -2;

			SGameIntro *pGame = GetGameIntro(pPlayer->posInfo.wGameId);
            // 还没有选择游戏呢
			if (NULL == pGame)
				return -3;

			SGameArea *pArea = pGame->pGameData->GetArea(pPlayer->posInfo.wAreaId);
            // 错误的游戏分区
			if (NULL == pArea)
				return -4;	

			SGameRoom *pRoom = pArea->GetRoom(pPlayer->posInfo.wRoomId);
            // 错误的游戏房间
			if (NULL == pRoom)
				return -5;

			SGameTable *pTable = pRoom->GetTable(pPlayer->posInfo.wTableId);
            // 错误的游戏桌子
			if (NULL == pTable)
				return -6;

			DWORD dwPlayerId = 0;
            // 错误的游戏桌子座位
			if (!pTable->GetSeatPlayer(pPlayer->posInfo.wSeatId, dwPlayerId))
				return -7;

            // 你不在座位上
			if (dwPlayerId != pPlayer->dwID)
				return -8;

			cs_PlugInGame_Chat *pChat = (cs_PlugInGame_Chat*)pMsg;
			if (pChat->byLen > PLUGINGAME_CHAT_MAX_LEN)
				return - 9;

			sc_PlugInGame_Chat aMsg;
			aMsg.byLen = pChat->byLen;
			aMsg.wSeatId = pPlayer->posInfo.wSeatId;
			strncpy(aMsg.strMsg, pChat->strMsg, aMsg.byLen);

			BroadCastMsg(gpTable, &aMsg, aMsg.GetSize(), pPlayer->posInfo.wGameId, pPlayer->posInfo.wAreaId, pPlayer->posInfo.wRoomId, pPlayer->posInfo.wTableId);
		}
		break;
	case SPluginGameBaseMsg::MSG_PLUGINGAME_CS_CHAT_PRIVATE:
		{
			//1.检验规则
            // 你还没有进入大厅
			if (NULL == pPlayer)
				return -2;

			SGameIntro *pGame = GetGameIntro(pPlayer->posInfo.wGameId);
            // 还没有选择游戏呢
			if (NULL == pGame)
				return -3;

			SGameArea *pArea = pGame->pGameData->GetArea(pPlayer->posInfo.wAreaId);
            // 错误的游戏分区
			if (NULL == pArea)
				return -4;		

			SGameRoom *pRoom = pArea->GetRoom(pPlayer->posInfo.wRoomId);
            // 错误的游戏房间
			if (NULL == pRoom)
				return -5;

			SGameTable *pTable = pRoom->GetTable(pPlayer->posInfo.wTableId);
            // 错误的游戏桌子
			if (NULL == pTable)
				return -6;

			DWORD dwPlayerId = 0;
            // 错误的游戏桌子座位
			if (!pTable->GetSeatPlayer(pPlayer->posInfo.wSeatId, dwPlayerId))
				return -7;

            // 你不在座位上
			if (dwPlayerId != pPlayer->dwID)
				return -8;

			cs_PlugInGame_Chat_Private *pChat = (cs_PlugInGame_Chat_Private*)pMsg;
			if (pChat->byLen > PLUGINGAME_CHAT_MAX_LEN)
				return -30; 

			if (!pTable->GetSeatPlayer(pChat->byTargetSeatId, dwPlayerId))			
				return -31;

			CPlayer *pTargetPlayer = ( CPlayer* )GetPlayerByGID( dwPlayerId )->DynamicCast( IID_PLAYER );
			if (NULL == pTargetPlayer)
				return -32;

			sc_PlugInGame_Chat_Private aMsg;
			aMsg.bySrcSeatId = (BYTE)pPlayer->posInfo.wSeatId;
			aMsg.byLen = pChat->byLen;
			strncpy(aMsg.strMsg, pChat->strMsg, aMsg.byLen);

            g_StoreMessage( pTargetPlayer->m_ClientIndex, &aMsg, aMsg.GetSize()); 
		}
		break;
	case SPluginGameBaseMsg::MSG_PLUGINGAME_GAME_LOGIC:
		{
			//1.检验规则
            // 你还没有进入大厅
			if (NULL == pPlayer)
				return -2;

			SGameIntro *pGame = GetGameIntro(pPlayer->posInfo.wGameId);
            // 还没有选择游戏呢
			if (NULL == pGame)
				return -3;

			SGameArea *pArea = pGame->pGameData->GetArea(pPlayer->posInfo.wAreaId);
            // 错误的游戏分区
			if (NULL == pArea)
				return -4;		

			SGameRoom *pRoom = pArea->GetRoom(pPlayer->posInfo.wRoomId);
            // 错误的游戏房间
			if (NULL == pRoom)
				return -5;

			SGameTable *pTable = pRoom->GetTable(pPlayer->posInfo.wTableId);
            // 错误的游戏桌子
			if (NULL == pTable)
				return -6;

			DWORD dwPlayerId = 0;
            // 错误的游戏桌子座位
			if (!pTable->GetSeatPlayer(pPlayer->posInfo.wSeatId, dwPlayerId))
				return -7;

            // 你不在座位上
			if (dwPlayerId != pPlayer->dwID)
				return -8;

			pTable->pOneGame->ParserMsg(pPlayer->posInfo.wSeatId, pMsg);
		}
		break;
	default:
		break;
	}

	return 0;
}

int CPlugInGameManager::PlayerLeaveTable(WORD wGameId, SGamePlayer *pPlayer, CPlayer *pOutterPlayer, BOOL bNotify /* = TRUE */)
{
	//1.检验规则
	if (NULL == pPlayer)
	{
		pPlayer = GetGamePlayer(pOutterPlayer->GetGID());
	}

	if (NULL == pPlayer)
	{
		//if (bNotify)
			//pOutterPlayer->SendSysMessageBox("你还没有进入大厅.");
		return -2;
	}

	SGameIntro *pGame = GetGameIntro(pPlayer->posInfo.wGameId);
	if (NULL == pGame)
	{
		//if (bNotify)
			//pOutterPlayer->SendSysMessageBox("还没有选择游戏呢.");
		return -3;
	}

	SGameArea *pArea = pGame->pGameData->GetArea(pPlayer->posInfo.wAreaId);
	if (NULL == pArea)
	{
		//if (bNotify)
			//pOutterPlayer->SendSysMessageBox("错误的游戏分区！");
		return -4;
	}			

	SGameRoom *pRoom = pArea->GetRoom(pPlayer->posInfo.wRoomId);
	if (NULL == pRoom)
	{
		//if (bNotify)
			//pOutterPlayer->SendSysMessageBox("错误的游戏房间！");
		return -5;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
	}

	SGameTable *pTable = pRoom->GetTable(pPlayer->posInfo.wTableId);
	if (NULL == pTable)
	{
		//if (bNotify)
			//pOutterPlayer->SendSysMessageBox("错误的游戏桌子！");
		return -6;
	}

	DWORD dwPlayerId = 0;
	if (!pTable->GetSeatPlayer(pPlayer->posInfo.wSeatId, dwPlayerId))
	{
		//if (bNotify)
			//pOutterPlayer->SendSysMessageBox("错误的游戏桌子座位！");
		return -7;
	}

	if (dwPlayerId != pPlayer->dwID)
	{
		//if (bNotify)
			//pOutterPlayer->SendSysMessageBox("你不在座位上！");
		return -8;
	}

	//2. 离开座位
	SPlayerPosInf oldPos = pPlayer->posInfo;

	pTable->PlayerLeaveSeat(pPlayer->posInfo.wSeatId);
	pTable->pRoom->wCurtPlayerCnt --;
	pTable->pRoom->pArea->wCurtPlayerCnt --;	

	pPlayer->posInfo.Reset();
	pPlayer->enGameStatus = ppsSelecting;

	if (NULL == pOutterPlayer)
	{
		pOutterPlayer = ( CPlayer* )GetPlayerByGID( pPlayer->dwID )->DynamicCast( IID_PLAYER );
	}

	//3. 发送信息
	sc_PlugInGame_SyncTable aTable;
	aTable.wGameId = oldPos.wGameId;
	aTable.wAreaId = oldPos.wAreaId;
	aTable.wRoomId = oldPos.wRoomId;
	aTable.wTableId = oldPos.wTableId;
	if (NULL != pOutterPlayer)
	{
		strncpy(aTable.strPlayerName, pOutterPlayer->GetName(), MAX_NAMELEN);
	}

	aTable.bySeatId = (BYTE)oldPos.wSeatId;
	aTable.dwPlayerId = pPlayer->dwID;
	aTable.byStatus = sc_PlugInGame_SyncTable::__tsPlayerLeft;	

	//todo: 向当前房间内的所有人发包？？
	//现为向当前区域的人发包

	if (NULL != pOutterPlayer && bNotify)	
	{
        g_StoreMessage( pOutterPlayer->m_ClientIndex, &aTable, sizeof aTable); 
	}

	BroadCastMsg(gpArea, &aTable, sizeof aTable, oldPos.wGameId, oldPos.wAreaId);

	//当前区域房间人数变化
	SyncAreaRoomsPlayerCnt(oldPos.wGameId, oldPos.wAreaId);


	//人数不固定的游戏， 如果在准备阶段有人离开， 需要检查是否可以开始游戏。
	if(pTable->GetGameIntro()->gameCode.bAllowStandBy && pTable->pOneGame->GetGameStatus() != gtsPlaying)
	{
		pTable->__bAsyncTryStartGame = TRUE;
	}

	return 0;
}

int CPlugInGameManager::TryStartGame(SGameTable *pTable)
{
	WORD wReadyOkCnt = 0;
	for (int i = 0; i < (int) pTable->m_vtSeats.size(); i++)	
	{				
		SGamePlayer *_pGP = GetGamePlayer(pTable->m_vtSeats[i]);
		if (_pGP == NULL)
		{
			continue;
		}
		else
		{
			if (_pGP->enGameStatus == ppsWaitting4Start)
			{
				wReadyOkCnt ++;
			}
			else
			{
				wReadyOkCnt = 0;
				break;
			}
		}
	}

	if (wReadyOkCnt >= pTable->GetGameIntro()->gameCode.wMinPlayerCnt )	//游戏开始 !!!!!!!
	{	
		int nRst = pTable->pOneGame->CheckCanStart();

		if (nRst != ecNone)
		{
			//pOutterPlayer->SendSysMessageBox("无法开始游戏！");
			return -100;		//fk.
		}

		sc_PlugInGame_GameStart startInfo;
		startInfo.wGameId = pTable->GetGameId();
		startInfo.wAreaId = pTable->GetAreaId();
		startInfo.wRoomId = pTable->GetRoomId();
		startInfo.wTableId = pTable->wTableId;

		DWORD dwParms[sc_PlugInGame_GameStart::MAX_PARM_CNT];
		WORD wParmCnt = 0;
		startInfo.byParmCnt = 0;
		pTable->pOneGame->GetStartParm(dwParms, wParmCnt);
		startInfo.byParmCnt = (BYTE)wParmCnt;
		for (WORD i = 0; i < wParmCnt; i++)
		{
			startInfo.arrParms[i] = dwParms[i];
		}

		//todo: room or area?	
		CPlugInGameManager::GetInstance()->BroadCastMsg(gpArea, &startInfo, startInfo.CalcSize(), pTable->GetGameId(), pTable->GetAreaId());

		pTable->pOneGame->StartGame();	

		return 0;
	}
	else
	{
		return -23;
	}
}

void CPlugInGameManager::SyncAreaPlayerCnt(WORD wGameId, WORD wAreaId)
{
	SGameIntro *pInst = GetGameIntro(wGameId);
    if ( !pInst || !(pInst->pGameData) || wAreaId - 1 >= (WORD)pInst->pGameData->m_vtAreas.size() || !(pInst->pGameData->m_vtAreas[wAreaId - 1]) )
        return;

	sc_PlugInGame_SyncAreaPlayerCnt playercnt;
	playercnt.wGameId = wGameId;
	playercnt.wAreaId = wAreaId;
	playercnt.wPlayerCnt = (WORD)pInst->pGameData->m_vtAreas[wAreaId - 1]->wCurtPlayerCnt;
	BroadCastMsg(gpArea, &playercnt, sizeof playercnt, wGameId, wAreaId);
}

void CPlugInGameManager::SyncRoomPlayerCnt(WORD wGameId, WORD wAreaId, WORD wRoomId)
{
	SGameIntro *pInst = GetGameIntro(wGameId);
    if ( !pInst || !(pInst->pGameData) || wAreaId - 1 >= (WORD)pInst->pGameData->m_vtAreas.size() || 
        !(pInst->pGameData->m_vtAreas[wAreaId - 1]) || !(pInst->pGameData->m_vtAreas[wAreaId - 1]->m_vtRooms[wRoomId -1]) )
        return;

	sc_PlugInGame_SyncRoomPlayerCnt playercnt;
	playercnt.wGameId = wGameId;
	playercnt.wAreaId = wAreaId;
	playercnt.wRoomId = wRoomId;
	playercnt.wPlayerCnt = (WORD)pInst->pGameData->m_vtAreas[wAreaId - 1]->m_vtRooms[wRoomId -1]->wCurtPlayerCnt;
	BroadCastMsg(gpArea, &playercnt, sizeof playercnt, wGameId, wAreaId, wRoomId);
}

void CPlugInGameManager::SyncAreaRoomsPlayerCnt(WORD wGameId, WORD wAreaId)
{
	SGameIntro *pInst = GetGameIntro(wGameId);	
    if ( !pInst || !(pInst->pGameData) || wAreaId - 1 >= (WORD)pInst->pGameData->m_vtAreas.size() || !(pInst->pGameData->m_vtAreas[wAreaId - 1]) )
        return;

	sc_PlugInGame_SyncRoomPlayerCnt roominfo;
	roominfo.wGameId = wGameId;
	roominfo.wAreaId = wAreaId;

	SGameArea *pArea = pInst->pGameData->m_vtAreas[wAreaId - 1];
	if ( pArea && (pArea->m_vtRooms.size() > 0) )
	{
		for (int i = 0; i < (int) pArea->m_vtRooms.size(); i++)
		{
			SGameRoom *pRoom = pArea->m_vtRooms[i];
            if ( !pRoom )
                continue;

			roominfo.wRoomId = i+1;
			roominfo.wPlayerCnt = (WORD)pRoom->wCurtPlayerCnt;

			BroadCastMsg(gpArea, &roominfo, sizeof roominfo, wGameId, wAreaId);
		}
	}
}

void CPlugInGameManager::SendSysMsgTip(CPlayer *pOutterPlayer, char *lpszMsg, ...)
{
	char        szOut[512];
	va_list     va;
	va_start( va, lpszMsg );
	_vsnprintf( szOut, 1024, lpszMsg, va );
	va_end( va );

	sc_PlugInGame_Chat_SysMsg aMsg;
	aMsg.byLen = (BYTE)strlen(szOut);
	aMsg.byLen = aMsg.byLen >= PLUGINGAME_CHAT_MAX_LEN ? PLUGINGAME_CHAT_MAX_LEN - 1 : aMsg.byLen;
	aMsg.byLen += 1; //for \0;
	strncpy(aMsg.strMsg, szOut, PLUGINGAME_CHAT_MAX_LEN - 1);

    if ( pOutterPlayer != NULL )
        g_StoreMessage( pOutterPlayer->m_ClientIndex, &aMsg, aMsg.GetSize()); 
}

int CPlugInGameManager::RegPlugInGameFromFile(string strFile)
{
	int nErr = 0;
	std::ifstream aFile;
	aFile.open(strFile.c_str());
	if (!aFile.is_open())
	{
		rfalse( "读取赌场游戏配置文件[%s]失败(文件不存在)", strFile.c_str() );
		return -1;
	}

	char strBuff[4096] = {0};

	WORD wRcdCnt = 0;
	for (int i = 0; i < 100; i++)
	{
		aFile >> strBuff;
		if (strcmp(strBuff, "·") == 0)
		{
			aFile >> wRcdCnt;
			break;
		}
	}

	if (wRcdCnt == 0)
	{
        rfalse( "读取赌场游戏配置文件[%s]失败(记录条数为0)", strFile.c_str() );
		return - 2;
	}

	WORD wTmpRcdCnt = wRcdCnt;

	WORD wGameId = 0;
	string strGameName = "";
	WORD wAreaCnt = 0;
	WORD wRoomCnt = 0;
	WORD wTableCnt = 0;

    char tmpStr[2048];
	while ( !aFile.eof() )
	{
		if (wRcdCnt == 0)
			break;
        
        aFile.getline( tmpStr, sizeof( tmpStr ) );
        if ( tmpStr[0] == 0 )
            continue;

		std::strstream stream( tmpStr, (std::streamsize)strlen( tmpStr ) );

		stream >> wGameId;
        if ( wGameId == 0 ) continue;
		stream >> strGameName;
		stream >> wAreaCnt;
		stream >> wRoomCnt;
		stream >> wTableCnt;

		if (wGameId == 0 
			|| wAreaCnt == 0 || wAreaCnt > PLUGINGAME_MAX_AREA_CNT
			|| wRoomCnt == 0 || wRoomCnt > PLUGINGAME_MAX_ROOM_CNT
			|| wTableCnt == 0 || wTableCnt > 2)
		{

			rfalse( 2, 1, "赌场配置参数错误。 序号: %d", (wTmpRcdCnt - wRcdCnt + 1));
			return -3;
		}

		switch(wGameId)
		{
		case PLUGINGAME_GAME_INDEX_HAPPY10_5: /*1*/	//欢乐10点半
			{
// 				_RegPluginGame regInfo;
// 				regInfo.bAllowStandBy = TRUE;
// 				regInfo.wAreaCnt = wAreaCnt;
// 				regInfo.wRoomCnt = wRoomCnt;
// 				regInfo.wTableCnt = 1;
// 				regInfo.wMinPlayerCnt = 2;	
// 				regInfo.wMaxPlayerCnt = 7; 
// 
// 				regInfo.wGameId = PLUGINGAME_GAME_INDEX_HAPPY10_5; 
// 
// 				for (int i = 0; i < (int) regInfo.wAreaCnt; i++)	//初始化
// 				{
// 					regInfo.areaParms[i].bIsOpen = false;
// 					regInfo.areaParms[i].arrParm10[0] = MoneyType_Gold;		//钱币类型
// 					regInfo.areaParms[i].arrParm10[1] = 100;			//数量
// 					regInfo.areaParms[i].arrParm10[2] = 500;			//彩金
//                     regInfo.areaParms[i].arrParm10[3] = 500;            //起征 高于该数收税
//                     regInfo.areaParms[i].arrParm10[4] = 5;              //税率 %
// 				}
// 
// 				strcpy(regInfo.strGameName, strGameName.c_str());
// 				int _nTmpRst = CPlugInGameManager::GetInstance()->RegPlugInGame(regInfo);
// 
// 				if (_nTmpRst != 0)
// 				{
// 					rfalse( 2, 1, "注册赌场游戏欢乐10点半(编号%d)失败, 错误代码%d", (int)regInfo.wGameId, (int)_nTmpRst);
// 					return -4;
// 				}
			}
			break;
		}

		wRcdCnt --;
	}
    aFile.close();

	//最后读取动态配置信息。
	int _nTmpRst = CPlugInGameManager::GetInstance()->LoadGameAreaConfig("data\\游戏大厅\\区域配置.txt");
	if ( _nTmpRst != 0)
	{
		rfalse( 2, 1, "读取区域配置配置错误, 代码%d", _nTmpRst);
		return -5;
	}

	return 0;
}



int CPlugInGameManager::LoadGameAreaConfig(string strFile)
{
	int nErr = 0;
// 	std::ifstream aFile;
// 	aFile.open(strFile.c_str());
// 	if (!aFile.is_open())
// 	{
// 		rfalse("读取赌场配置文件[%s]失败(文件不存在).", strFile.c_str());
// 		return -1;
// 	}
// 
// 	char strBuff[4096] = {0};
// 
// 	WORD wRcdCnt = 0;
// 	for (int i = 0; i < 100; i++)
// 	{
// 		aFile >> strBuff;
// 		if (strcmp(strBuff, "·") == 0)
// 		{
// 			aFile >> wRcdCnt;
// 			break;
// 		}
// 	}
// 
// 	if (wRcdCnt == 0)
// 	{
// 		rfalse("读取赌场配置文件[%s]失败(记录条数为0).", strFile.c_str());
// 		return - 2;
// 	}
// 
// 	WORD wGameId = 0;
// 	WORD wAreaId = 0;
// 	BOOL bIsOpen = FALSE;
// 	WORD wMoneyType = MoneyType_Gold;
// 	DWORD wBaseMoneyCnt = 0;
// 	DWORD wLotteryMoneyCnt = 0;
//     DWORD dwBaseTaxMoney = 0;
//     WORD  wTaxRate = 0;
// 
//     char tmpStr[2048];
// 	while( !aFile.eof() )
// 	{
// 		if (wRcdCnt == 0)
// 			break;
//         aFile.getline( tmpStr, sizeof( tmpStr ) );
//         if ( tmpStr[0] == 0 )
//             continue;
// 
//         std::strstream stream( tmpStr, (std::streamsize)strlen( tmpStr ) );
// 
//         stream >> wGameId;
//         if ( wGameId == 0 ) continue;         
//         stream >> wAreaId;
//         stream >> bIsOpen;
//         stream >> wMoneyType;
//         stream >> wBaseMoneyCnt;
//         stream >> wLotteryMoneyCnt;
//         stream >> dwBaseTaxMoney;
//         stream >> wTaxRate;
// 
// 		SGameIntro *pGameIntro = GetGameIntro(wGameId);
// 		if (NULL == pGameIntro)
// 		{
// 			nErr = -1;
// 			goto err;
// 		}
// 
// 		if (wAreaId == 0 || wAreaId > pGameIntro->gameCode.wAreaCnt)
// 		{
// 			nErr = -2;
// 			goto err;
// 		}
// 		SRegAreaParm *pAreaParm = &pGameIntro->gameCode.areaParms[wAreaId - 1];
//         if ( pAreaParm == NULL )
//         {
//             nErr = -2;
//             goto err;
//         }
// 		pAreaParm->bIsOpen = bIsOpen == TRUE;
// 
// 		if (wMoneyType > MoneyType_Max)
// 		{
// 			nErr = -3;
// 			goto err;
// 		}
// 		pAreaParm->arrParm10[0] = wMoneyType;
// 		pAreaParm->arrParm10[1] = wBaseMoneyCnt;
// 		pAreaParm->arrParm10[2] = wLotteryMoneyCnt;
//         pAreaParm->arrParm10[3] = dwBaseTaxMoney;	
//         pAreaParm->arrParm10[4] = wTaxRate;	
// 
// 		wRcdCnt --;
// 	}
//     
//     aFile.close();
// 	return 0;
// err:
// 	rfalse( 2, 1, "读取赌场配置文件%s 失败， 代码%d", (char*)strFile.c_str(), nErr );
//     aFile.close();
	return nErr;
}

BOOL CPlayer::IsBetting()
{
	int nRst = FALSE;
	SGamePlayer *pPlayer = CPlugInGameManager::GetInstance()->GetGamePlayer(GetGID());
	if (pPlayer && pPlayer->enGameStatus >= ppsGameLine)
	{
		nRst = TRUE;
	}

	return nRst;
}


CPlugInGame::CPlugInGame(DWORD dwGameId)
{
	m_dwGameSerID = dwGameId;

	m_pTable = NULL;
}

CPlugInGame::~CPlugInGame()
{

}

void CPlugInGame::StartGame()
{	
	SetGameStatus(gtsPlaying);
	OnGameStart();
}

void CPlugInGame::GameOver()
{
	SetGameStatus(gtsWaitting);
	OnGameOver(NULL, 0);

	for (int i = 0; i < (int)m_pTable->m_vtSeats.size(); i++)
	{
		SGamePlayer *pPlayer = CPlugInGameManager::GetInstance()->GetGamePlayer(m_pTable->m_vtSeats[i]);
		if (NULL != pPlayer)
		{
			pPlayer->enGameStatus = ppsReadying;
			pPlayer->dwLastSitDwonTime = GetGW()->GetGameTick();
		}
	}	
}

void CPlugInGame::OnGameOver(WORD *pArrWinners, int nWinnerCnt)
{
	sc_PlugInGame_GameFinish finish;
	finish.wGameId = (WORD)m_dwGameSerID;
	finish.wAreaId = m_pTable->pRoom->pArea->wAreaId;
	finish.wRoomId = m_pTable->pRoom->wRoomId;
	finish.wTableId = m_pTable->wTableId;
	finish.byWinnerCnt = (BYTE)nWinnerCnt;
	if (nWinnerCnt > 0)
	{
		for (int i = 0; i < (int) nWinnerCnt; i++)
		{
			finish.arrWinnerSeatId[i] = (BYTE)pArrWinners[i];
		}
	}

	CPlugInGameManager::GetInstance()->BroadCastMsg(gpArea, &finish, finish.GetSize(), (WORD)m_dwGameSerID, finish.wAreaId);
}

void CPlugInGame::MakeLogicHeader(__SPosInfo& pData)
{
    if ( m_pTable && m_pTable->pOneGame && m_pTable->pRoom && m_pTable->pRoom->pArea )
    {
        pData.wGameId = (WORD)m_pTable->pOneGame->m_dwGameSerID;
        pData.wAreaId = m_pTable->pRoom->pArea->wAreaId;
        pData.wRoomId = m_pTable->pRoom->wRoomId;
        pData.wTableId = m_pTable->wTableId;
    }
}

void CPlugInGameManager::SendPlugInGameErr( DNID dnidClient, int nGameErrType )
{
    sc_PlugInGame_GameErr errMsg;
    errMsg.wGameErrType = nGameErrType;
    g_StoreMessage( dnidClient, &errMsg, sizeof(errMsg) );
}