#include "stdafx.h"
#include "plugingame_happy10_5.h"
#include <list>

#define PLUGINGAME_HAPPY10_5_MAX_WAIT_CHOICE_TICK	(TICKS_PER_SECOND * 10)

extern LPIObject GetPlayerByGID(DWORD);
extern CGameWorld *&GetGW();
#define _GetGameTick()	(GetGW()->GetGameTick())

CHappy10_5::CHappy10_5(DWORD dwGameId ): CPlugInGame(dwGameId)
{
	m_pPoker = new CPoker(false);
}

CHappy10_5::~CHappy10_5()
{
	delete m_pPoker;
	m_pPoker = NULL;
}

int CHappy10_5::GetMoneyType()
{
    if ( !m_pTable || !m_pTable->GetGameIntro() || m_pTable->GetAreaId() - 1 >= PLUGINGAME_MAX_GAME_AREA_CNT )
        return -1;

	SRegAreaParm *pParm = &m_pTable->GetGameIntro()->gameCode.areaParms[m_pTable->GetAreaId() - 1];
    if ( !pParm )
        return -1;
	return pParm->GetMoneyType();
}

DWORD CHappy10_5::GetBaseMoneyCnt()
{
    if ( !m_pTable || !m_pTable->GetGameIntro() || m_pTable->GetAreaId() - 1 >= PLUGINGAME_MAX_GAME_AREA_CNT )
        return 0;

	SRegAreaParm *pParm = &m_pTable->GetGameIntro()->gameCode.areaParms[m_pTable->GetAreaId() - 1];
    if ( !pParm )
        return 0;
	return pParm->GetBaseMoney();
}

DWORD CHappy10_5::GetLotteryMoneyCnt()
{
    if ( !m_pTable || !m_pTable->GetGameIntro() || m_pTable->GetAreaId() - 1 >= PLUGINGAME_MAX_GAME_AREA_CNT )
        return 0;

	SRegAreaParm *pParm = &m_pTable->GetGameIntro()->gameCode.areaParms[m_pTable->GetAreaId() - 1];
    if ( !pParm )
        return 0;
	return pParm->GetLotteryMoney();
}

DWORD CHappy10_5::GetBaseTaxMoney()
{
    if ( !m_pTable || !m_pTable->GetGameIntro() || m_pTable->GetAreaId() - 1 >= PLUGINGAME_MAX_GAME_AREA_CNT )
        return 0;

    SRegAreaParm *pParm = &m_pTable->GetGameIntro()->gameCode.areaParms[m_pTable->GetAreaId() - 1];
    if ( !pParm )
        return 0;
    return pParm->GetBaseTaxMoney();
}

DWORD CHappy10_5::GetTaxRate()
{
    if ( !m_pTable || !m_pTable->GetGameIntro() || m_pTable->GetAreaId() - 1 >= PLUGINGAME_MAX_GAME_AREA_CNT )
        return 0;

    SRegAreaParm *pParm = &m_pTable->GetGameIntro()->gameCode.areaParms[m_pTable->GetAreaId() - 1];
    if ( !pParm )
        return 0;
    return (pParm->GetTaxRate() <=100 ? pParm->GetTaxRate() : 5 );
}

void CHappy10_5::Update(int nTick)
{ 
	//static int si = 0;
	//if (si == 0)
	//	return;

	if (GetGameStatus() != gtsPlaying)
		return;

	_SHappy10_5_User_Data *pUserData = NULL;

	switch (m_nGameProgress)
	{
	case gpWaitPlayerChoice:
		{
			_SHappy10_5_User_Data *pUserData = &m_vtUserData[m_nCurtPlayerIndex];
            if ( !pUserData )
                break;

            int testTime = 0;
			if (_GetGameTick() - pUserData->dwWaitBeginTime > PLUGINGAME_HAPPY10_5_MAX_WAIT_CHOICE_TICK)	//等待玩家选择超时， 系统自动选择pass
			{
				cs_HappyTenPointFive_CommitChoice choice;
				MakeLogicHeader(choice.posInfo);
				choice.byChoice = 0;
				this->ParserMsg(m_nCurtPlayerIndex + 1, &choice, TRUE);

				/*				sc_HappyTenPointFive_WaitChoiceTimeOut msg;
				MakeLogicHeader(msg.posInfo);
				pUserData->pOutterPlayer->SendMessage(&msg, sizeof msg); */              
			}				
		}
		break;
	default:
		break;
	}

}

int CHappy10_5::ParserMsg(WORD wSeatId, void *pMsg, int nParam /* = 0 */)
{
    if ( pMsg == NULL )
        return 0;

	SPluginGame_HappyTenPointFive_BaseMsg *pHappyTenPointFiveMsg = (SPluginGame_HappyTenPointFive_BaseMsg*)pMsg;
	switch(pHappyTenPointFiveMsg->_protocol)
	{
	case SPluginGame_HappyTenPointFive_BaseMsg::MSG_CS_COMMIT_CHOICE:
		{
			cs_HappyTenPointFive_CommitChoice *pChoice = (cs_HappyTenPointFive_CommitChoice*)pMsg;			
			assert(wSeatId != 0 && wSeatId <= m_vtUserData.size());

			if (m_nCurtPlayerIndex != (wSeatId - 1))	//没有轮到
				return -1;

			_SHappy10_5_User_Data *pUserData = &m_vtUserData[wSeatId - 1];

            if ( !pUserData )
                break;

			if (pChoice->byChoice == 1) //要牌
			{
				if (pUserData->byCardCnt >= 7)	//最多只能要7张牌
				{
					CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData->pOutterPlayer, "底牌数量达到最大了。");
					pChoice->byChoice = 0;
				}				
			}

			pUserData->bChosePass = !pChoice->byChoice;

			sc_HappyTenPointFive_PlayerChoice playerChoice;
			MakeLogicHeader(playerChoice.posInfo);
			playerChoice.bySeatId = (BYTE)wSeatId;

			if (pUserData->bChosePass )
			{
				playerChoice.byChice = 0;			
			}
			else
			{
				if (pUserData->wHalfPointCnt >= 21)
				{
					CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData->pOutterPlayer, "你的点数不能再要了！");
					pUserData->bChosePass = 1;
				}
				else if ( pUserData->pOutterPlayer && (pUserData->pOutterPlayer->GetMoney( GetMoneyType() ) < GetBaseMoneyCnt()) )
				{
					CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData->pOutterPlayer, "你的底金不足！");
					pUserData->bChosePass = 1;
				}
				else
				{
					pUserData->bChosePass = 0;
					pUserData->dwMoneyOnTable += GetBaseMoneyCnt();

                    if ( pUserData->pOutterPlayer )
					    pUserData->pOutterPlayer->DelMoney(GetMoneyType(), GetBaseMoneyCnt() );

					m_dwMoneyOnTable += GetBaseMoneyCnt();
				}				
			}			

			//通知客户端			
			playerChoice.byChice = pUserData->bChosePass ? 0 : 1;
			bool bSend = true;
			for (int i = 0; i < (int)m_vtUserData.size(); i++)
			{
				if (/*(i != (wSeatId - 1)) && */!m_vtUserData[i].bQuitGame && m_vtUserData[i].pOutterPlayer)
                    g_StoreMessage( m_vtUserData[i].pOutterPlayer->m_ClientIndex, &playerChoice, sizeof playerChoice); 
			}

			if (!pUserData->bChosePass)	//发牌
			{
				sc_HappyTenPointFive_OneCard aCard;
				MakeLogicHeader(aCard.posInfo);
				aCard.byCardId = m_pPoker->GetOne();
				WORD wHalfPoint = CalcHalfPoint(aCard.byCardId);
				pUserData->wHalfPointCnt += wHalfPoint;			

				pUserData->arrCardHistory[pUserData->byCardCnt++] = aCard.byCardId;

                if ( pUserData->pOutterPlayer )
                    g_StoreMessage( pUserData->pOutterPlayer->m_ClientIndex, &aCard, sizeof aCard); 
			}	


			//1 检测是否结束
			int n = 100;
			while (n > 0)
			{
				if (CheckFinish())
				{
					GameOver();
					break;
				}
				else
				{
					SetNextPlayer();
					if (OnPlayer_s_Turn(nParam) == 0 )// no error
						break;
				}
				n --;
			}

			assert(n > 0);
		}
		break;
	}

	return 0;
}

BOOL CHappy10_5::CheckFinish()
{
	if (m_nCurtTurnPlayerCnt > 7)
		return TRUE;


	////////////if (m_nPassCnt >= m_pTable->m_vtSeats.size())
	//////////	//return TRUE;

	////////////if (m_nTurnCnt > 7)
	//////////	//return TRUE;

	return FALSE;
}

int CHappy10_5::CalcHalfPoint(WORD wCardId)
{
	wCardId = (wCardId - 1) % 13;
	wCardId += 1;

	if (wCardId >= pcvOne && wCardId <= pcvTen)
		return wCardId * 2;
	else if (wCardId >= pcvJack && wCardId <= pcvKing)
		return 1;

	return 0;
}

void CHappy10_5::InitData(_RegPluginGame *info)
{
	m_vtUserData.resize(info->wMaxPlayerCnt);
	for (int i = 0; i < (int)m_vtUserData.size(); i++)
	{
		_SHappy10_5_User_Data *pData = &m_vtUserData[i];

        if ( pData )
        {
		    pData->Reset();
		    pData->bQuitGame = TRUE;		//!!!
		    pData->bChosePass = TRUE;
		    pData->bIsStandingBy = TRUE;
		    pData->dwLotteryMoney = 0;
		    pData->dwMoneyOnTable = 0;
		    pData->dwMoneyGet = pData->dwMoneyLost = 0;
		    pData->pGamePlayer = NULL;
		    pData->pOutterPlayer = NULL;
		    pData->wHalfPointCnt = 0;
        }
	}

	////GetMoneyType() = info->areaParms[m_pTable->pRoom->pArea->wAreaId - 1].arrParm10[0];	
	////GetBaseMoneyCnt() = info->areaParms[m_pTable->pRoom->pArea->wAreaId - 1].arrParm10[1];		//每轮底金
	////GetLotteryMoneyCnt() = info->areaParms[m_pTable->pRoom->pArea->wAreaId - 1].arrParm10[2];	//彩金
}

void CHappy10_5::RefreshRegData(SRegDynamicData *pNewData)
{
	//m_nMoneyType = pNewData->Parms[0];
	//GetBaseMoneyCnt() = pNewData->Parms[1];
	//GetLotteryMoneyCnt() = pNewData->Parms[2];
}

int CHappy10_5::CheckCanStart()
{
	for (int i = 0; i < (int)m_pTable->m_vtSeats.size(); i++)
	{
		CPlayer *pOutterPlayer = ( CPlayer* )GetPlayerByGID( m_pTable->m_vtSeats[i] )->DynamicCast( IID_PLAYER );
		//SGamePlayer *pGamePlayer = CPlugInGameManager::GetInstance()->GetGamePlayer(m_pTable->m_vtSeats[i]);
		////assert(pGamePlayer != NULL);
		////assert(pOutterPlayer != NULL);
		/////////
		if (pOutterPlayer == NULL)
			continue;

		//1. 玩家是否有冲突的交易？

		//2. 玩家金钱是否足够？
		if (pOutterPlayer->GetMoney( GetMoneyType() ) < GetBaseMoneyCnt())
		{			
			return ecNotEnoughtMoney;
		}
		/////////	
	}

	return ecNone;
}

BOOL CHappy10_5::CheckPlayerCanPlay(CPlayer *pPlayer)
{
	//1. 玩家是否有冲突的交易？

	//2. 玩家金钱是否足够？
	//if (pPlayer->GetMoney( GetMoneyType() ) < GetBaseMoneyCnt())
	//	return ecNotEnoughtMoney;

	return ecNone;
}

void CHappy10_5::GetStartParm(IN OUT DWORD *pParms, OUT WORD &wParmCnt)
{
	assert(pParms != NULL);

	wParmCnt = 2;
	pParms[0] = (DWORD)GetMoneyType();

	int nPlayerCnt = 0;
	for (int i = 0; i < (int)m_vtUserData.size(); i++)
	{
		_SHappy10_5_User_Data *pUserData = &m_vtUserData[i];
		if (pUserData && !pUserData->bQuitGame /*&& !pUserData->bIsStandingBy*/)
		{
			nPlayerCnt ++;
		}
	}

	pParms[1] = nPlayerCnt * GetLotteryMoneyCnt();
}

void CHappy10_5::OnGameStart()
{
    if ( m_pPoker )
	    m_pPoker->Reset();
	/////////////////////////////////m_nTurnCnt = 1;
	m_nGameProgress = gpBeginTurn;
	/////////////////////////////////m_nPassCnt = 0;
	m_nCurtTurnPlayerCnt = 0;
	///////////////////////////////m_nStartPlayerIndexofCurtTurn = 0;
	m_nCurtPlayerIndex = -1;

	m_dwMoneyOnTable = m_dwAllLotterMoney = 0;

	//扣首轮金币, 扣彩金。  + 发牌 。 todo: 是否还需要检查玩家金钱是否足够？

	/////////////////////////////////////////////////m_nTurnCnt ++;	

	sc_HappyTenPointFive_OneCard aCard;
	MakeLogicHeader(aCard.posInfo);

	for (int i = 0; i < (int)m_pTable->m_vtSeats.size(); i++)
	{
		_SHappy10_5_User_Data *pUserData = &m_vtUserData[i];

		if ( pUserData == NULL ||  pUserData->bQuitGame || pUserData->pOutterPlayer == NULL )	//空位
			continue;

		pUserData->Reset();

		CPlayer *pOutterPlayer = pUserData->pOutterPlayer;

		//扣彩金+首轮底金
		pOutterPlayer->DelMoney(GetMoneyType(), GetBaseMoneyCnt() + GetLotteryMoneyCnt() );
		pUserData->dwMoneyOnTable = GetBaseMoneyCnt();
		pUserData->dwLotteryMoney = GetLotteryMoneyCnt();

		m_dwMoneyOnTable += GetBaseMoneyCnt();
		m_dwAllLotterMoney += GetLotteryMoneyCnt();

		///首轮默认 “要牌”
		aCard.byCardId = m_pPoker->GetOne();
		WORD wHalfPoint = CalcHalfPoint(aCard.byCardId);
		pUserData->wHalfPointCnt += wHalfPoint;
		pUserData->arrCardHistory[pUserData->byCardCnt++] = aCard.byCardId;

		//通知客户端
		sc_HappyTenPointFive_PlayerChoice playerChoice;
		MakeLogicHeader(playerChoice.posInfo);
		playerChoice.byChice = 1; //要牌。
		playerChoice.bySeatId = i + 1;
		for (int j = 0; j < (int)m_pTable->m_vtSeats.size(); j++)
		{
			if (!m_vtUserData[j].bQuitGame)
                g_StoreMessage( m_vtUserData[j].pOutterPlayer->m_ClientIndex, &playerChoice, sizeof playerChoice); 
		}

		//系统发牌
		g_StoreMessage(pOutterPlayer->m_ClientIndex, &aCard, sizeof aCard);
	}


	m_nGameProgress = gpWaitPlayerChoice;

	SetNextPlayer(TRUE);
	OnPlayer_s_Turn();

	//还是算了， 太快了， 客户端看不清楚。
	//////////////////先特殊处理第一个玩家。		 
	////////////////SetNextPlayer(TRUE);	
	////////////////if (0 == OnPlayer_s_Turn())	//第一个玩家可以要牌
	////////////////{
	////////////////	return;
	////////////////}
	////////////////else 
	////////////////{
	////////////////	//kevin  @ 2008.10.31
	////////////////	//如果所有玩家都没钱了， 则游戏一开始就会结束。

	////////////////	//new 或者要轮到第一个可以要牌的玩家
	////////////////	int n = 100;
	////////////////	while (n > 0)
	////////////////	{
	////////////////		if (CheckFinish())
	////////////////		{
	////////////////			GameOver();
	////////////////			break;
	////////////////		}
	////////////////		else
	////////////////		{
	////////////////			SetNextPlayer();
	////////////////			if (OnPlayer_s_Turn() == 0 )// no error
	////////////////			{
	////////////////				break;
	////////////////			}
	////////////////		}

	////////////////		n --;
	////////////////	}

	////////////////	assert(n > 0);
	////////////////}	
}

void CHappy10_5::SetNextPlayer(BOOL bFirstTime /* = False */)
{

	if (bFirstTime)
	{
		m_nCurtPlayerIndex = 0;
		m_nCurtTurnPlayerCnt = 1;

		m_nCurtTurnPlayerCnt = 1;
	}
	else
	{
		//下一个玩家的条件是，当前玩家已经pass了（standby的玩家被强制设置成pass了。）。
		_SHappy10_5_User_Data *pUserData = &m_vtUserData[m_nCurtPlayerIndex];
         if ( pUserData == NULL)
             return;

		if (pUserData->bQuitGame || pUserData->bChosePass || pUserData->bIsStandingBy)
		{
			m_nCurtPlayerIndex ++;
			m_nCurtTurnPlayerCnt ++;
		}		
	}

	//m_nCurtTurnPlayerCnt ++;

	//////////////一轮是否结束
	////////////if (m_nCurtTurnPlayerCnt >= m_pTable->m_vtSeats.size()) //一轮结束
	////////////{
	////////////	m_nTurnCnt++;
	////////////	m_nPassCnt = 0;
	////////////	m_nCurtTurnPlayerCnt = 0;

	////////////	// 变换首轮发牌者。
	////////////	m_nStartPlayerIndexofCurtTurn ++;
	////////////	m_nStartPlayerIndexofCurtTurn %= m_pTable->m_vtSeats.size();
	////////////	m_nCurtPlayerIndex = m_nStartPlayerIndexofCurtTurn;

	////////////	//m_nCurtPlayerIndex = 0;
	////////////}
	////////////else//下一个玩家
	////////////{
	////////////	m_nCurtPlayerIndex ++;
	////////////	m_nCurtPlayerIndex %= m_pTable->m_vtSeats.size(); //or m_vtusreData.size();
	////////////}
}

int CHappy10_5::OnPlayer_s_Turn(BOOL bServerAuto /* = false */)
{
	//1. 是否pass过
	//2. 是否点数过大

	////////////////////////m_nCurtTurnPlayerCnt++;

	_SHappy10_5_User_Data *pUserData = NULL;
	pUserData = &m_vtUserData[m_nCurtPlayerIndex];	

    if ( pUserData == NULL || pUserData->pOutterPlayer == NULL )
        return -1;

	if (pUserData->bChosePass || pUserData->bQuitGame || pUserData->bIsStandingBy/* standby的玩家被强制设置成pass了。*/) //!!!!
	{
		/////////////////////m_nPassCnt ++;
		return -1;
	}

	pUserData->dwWaitBeginTime = _GetGameTick();

	if (pUserData->pOutterPlayer->GetMoney( GetMoneyType() ) < GetBaseMoneyCnt())
	{
		pUserData->bChosePass = true;	//new

		if (!bServerAuto /*&& !pUserData->wNotifyTimesOnFrezze*/)
		{
			// 底金不足      
            CPlugInGameManager::GetInstance()->SendPlugInGameErr( pUserData->pOutterPlayer->m_ClientIndex, -10 );
			CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData->pOutterPlayer, "底金不足,不能继续要牌了！");
		}


		/////////////////////////////m_nPassCnt ++;
		return -2;
	}

	if (pUserData->wHalfPointCnt >= 10 * 2 + 1)	//10.5 * 2
	{
		pUserData->bChosePass = true;
		if (!bServerAuto /*&& !pUserData->wNotifyTimesOnFrezze*/)
		{			
			CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData->pOutterPlayer, "你的点数已经不能再要了哦！");
		}

		/////////////////////////////m_nPassCnt ++;
		/////////////////////////////pUserData->wNotifyTimesOnFrezze ++;

		return -3;
	}

	//ok 提示玩家是否要牌
	sc_HappyTenPointFive_PlayerSTurn msg;
	MakeLogicHeader(msg.posInfo);
	for (int i = 0; i < (int)m_vtUserData.size(); i++)
	{
		msg.bySeatId = m_nCurtPlayerIndex + 1;
		pUserData = &m_vtUserData[i];
		if (!pUserData->bQuitGame)
            g_StoreMessage( pUserData->pOutterPlayer->m_ClientIndex, &msg, sizeof msg); 
	}

	return 0;
}

void CHappy10_5::SendCardDetails(WORD wSeatId)
{
	//standingby者不会进入

	_SHappy10_5_User_Data *pThisPlayer = &m_vtUserData[wSeatId - 1];
    if ( !pThisPlayer )
        return;

	_SHappy10_5_User_Data *pOtherPlayer = NULL;

	if (pThisPlayer->byCardCnt > 7)
	{
		return;
	}

	sc_HappyTenPointFive_PlayerSCardsDetails myCards;
	//sc_HappyTenPointFive_PlayerSCardsDetails OtherCards;
	for (int i = 0; i < pThisPlayer->byCardCnt; i++)
	{
		myCards.arrCards[i] = pThisPlayer->arrCardHistory[i];
	}
	myCards.byCardCnt = pThisPlayer->byCardCnt;
	myCards.bySeatId = (BYTE)wSeatId;

	for (int i = 0; i < (int)m_vtUserData.size(); i++)
	{
		if (wSeatId == (i+1))
			continue;		

		pOtherPlayer = &m_vtUserData[i];

		if (pOtherPlayer == NULL || pOtherPlayer->bQuitGame || pOtherPlayer->pOutterPlayer == NULL )
			continue;
        g_StoreMessage( pOtherPlayer->pOutterPlayer->m_ClientIndex, &myCards, myCards.GetSize()); 
	}
}

void CHappy10_5::OnGameOver(WORD *pArrWinners, int nWinnerCnt)
{
	//1. 找出胜利者。
// 	WORD wMaxPoint = 0;
// 	int nTmp = 0;
// 	WORD arrWinner[PLUGINGAME_MAX_ONE_GAME_PLAYER_CNT] = {0};		
// 	DWORD dwAllMoney = 0;
// 	DWORD dwAllLotteryMoney = 0;
// 
// 	for (int i = 0; i < (int)m_vtUserData.size(); i++)
// 	{	
// 		_SHappy10_5_User_Data *pUserData1 = &m_vtUserData[i];
//         if ( !pUserData1 )
//             continue;
// 
// 		pUserData1->bLastWin = FALSE;
// 
// 		if (pUserData1->bIsStandingBy)	//中途加入的玩家不影响结局
// 			continue;
// 
// 		dwAllMoney += pUserData1->dwMoneyOnTable;
// 		dwAllLotteryMoney += pUserData1->dwLotteryMoney;
// 
// 		if (pUserData1->bQuitGame)	//空位（或中途退出游戏）的玩家判为失败。
// 		{
// 			pUserData1->wHalfPointCnt = 999;
// 		}
// 
// 		//发送玩家牌面详细信息
// 		SendCardDetails(i + 1);
// 
// 		if (pUserData1->wHalfPointCnt > 21)//肯定的失败者	
// 		{
// 			pUserData1->dwMoneyLost += pUserData1->dwMoneyOnTable;
// 		}
// 		else	//可能的胜者
// 		{
// 			if (pUserData1->wHalfPointCnt > wMaxPoint)
// 				wMaxPoint = pUserData1->wHalfPointCnt;
// 		}
// 	}
// 
// 	if (wMaxPoint == 0 ) //没有胜利者
// 	{
// 		int i = 123;
// 	}
// 	else
// 	{
// 		list<_SHappy10_5_User_Data*> lstWinner;       
// 
// 		lstWinner.clear();
// 		for (int i = 0; i < (int)m_vtUserData.size(); i++)	//找出胜利者 etc.
// 		{
// 			_SHappy10_5_User_Data *pUserData2 = &m_vtUserData[i];
//             if ( !pUserData2 )
//                 continue;
// 
// 			if (wMaxPoint == pUserData2->wHalfPointCnt)
// 			{
// 				pUserData2->bLastWin = TRUE;
// 				lstWinner.push_back(pUserData2);
// 				arrWinner[nTmp++] = i + 1;
// 			}
// 		}
// 
// 		//按低金总数 收税
// 		DWORD dwBounds = dwAllMoney >= GetBaseTaxMoney() ? ( (dwAllMoney - (dwAllMoney * GetTaxRate()/100)) / lstWinner.size()) : (dwAllMoney / lstWinner.size());
//         DWORD dwPersonalTax = dwAllMoney / lstWinner.size() - dwBounds;
// 
// 		DWORD dwLotteryMoney = 0;
// 		if (wMaxPoint == 21)	//分彩金， 否则退还彩金
// 		{
// 			dwLotteryMoney = dwAllLotteryMoney / lstWinner.size();
// 
// 			//记录未中彩的损失彩金
// 			for (int i = 0; i < (int)m_vtUserData.size(); i++)
// 			{
// 				_SHappy10_5_User_Data *pUserData3 = &m_vtUserData[i];
//                 if ( !pUserData3 )
//                     continue;
// 
// 				if (!pUserData3->bLastWin && !pUserData3->bQuitGame)
// 				{ 
// 					pUserData3->dwMoneyLost += pUserData3->dwLotteryMoney;
// 				}
// 			}
// 		}
// 
// 		//分钱了。
// 		for (list<_SHappy10_5_User_Data*>::iterator it = lstWinner.begin(); it != lstWinner.end(); ++it)
// 		{
// 			_SHappy10_5_User_Data *pUserData4 = *it;
//             if ( !pUserData4 )
//                 continue;
// 			pUserData4->dwMoneyGet += (dwBounds + dwLotteryMoney);
// 			pUserData4->pOutterPlayer->AddMoney(GetMoneyType(), dwBounds + dwLotteryMoney);
// 			CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData4->pOutterPlayer, "恭喜你获得本轮胜利!");
// 
// 			char strMoney[10] = {0};
// 			switch(GetMoneyType())
// 			{
// 			case MoneyType_Yuanbao:				
// 				strcpy(strMoney, "元宝");
// 				break;
// 			case MoneyType_Gift:
// 				strcpy(strMoney, "赠宝");
// 				break;
// 			case MoneyType_Gold:
// 				strcpy(strMoney, "金币");
// 				break;
// 			}
// 
// 			CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData4->pOutterPlayer, "你分得了%d%s", dwBounds, strMoney);
//             if ( (dwAllMoney * GetTaxRate()/100) > 0 )
//                 CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData4->pOutterPlayer, "你缴纳税收%d%s", dwPersonalTax, strMoney);
// 
// 			if (dwLotteryMoney > 0)
// 				CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData4->pOutterPlayer, "得到彩金%d%s", dwLotteryMoney, strMoney);
// 		}
// 	}  
// 
// 	//退还彩金
// 	if (wMaxPoint != 21)
// 	{
// 		for (int i = 0; i < (int)m_vtUserData.size(); i++)
// 		{
// 			_SHappy10_5_User_Data *pUserData5 = &m_vtUserData[i];
//             if ( !pUserData5 )
//                 continue;
// 
// 			if (!pUserData5->bQuitGame)
// 			{					
// 				CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData5->pOutterPlayer, "无人中彩，退还彩金！");
// 				if (!pUserData5->bIsStandingBy)
// 				{
// 					pUserData5->pOutterPlayer->AddMoney(GetMoneyType(), pUserData5->dwLotteryMoney);	
// 				}
// 			}
// 		}
// 	}
// 
// 
// 	//通知。。。 ， 修改游戏状态。
// 	m_nGameProgress = gpWaitting4Start;	
// 	CPlugInGame::OnGameOver(arrWinner, nTmp);
// 
// 	//4. 检查底金， 不足则踢人。
// 	for (int i = 0; i < (int)m_vtUserData.size(); i++)
// 	{
// 		_SHappy10_5_User_Data *pUserData6 = &m_vtUserData[i];
// 
//         if ( !pUserData6 )
//             continue;
// 
// 		if (pUserData6->bQuitGame)
// 			continue;
// 
// 		if (pUserData6->pOutterPlayer->GetMoney( GetMoneyType() ) < (GetBaseMoneyCnt() + GetLotteryMoneyCnt()))
// 		{				
// 			// 底金不足
//             CPlugInGameManager::GetInstance()->SendPlugInGameErr( pUserData6->pOutterPlayer->m_ClientIndex, -10 );
// 			//DWORD dwPlayer
// 
// 			//注意此时游戏已经结束， 需要把游戏状态修改成 gtsWaitting, 否则会多次gameover, 导致发送奖金和重复退还彩金			
//             int result = CPlugInGameManager::GetInstance()->PlayerLeaveTable((WORD)m_dwGameSerID, pUserData6->pGamePlayer, pUserData6->pOutterPlayer, TRUE);
//             if ( result < 0 )
//                 CPlugInGameManager::GetInstance()->SendPlugInGameErr( pUserData6->pOutterPlayer->m_ClientIndex, result );
// 
// 			//!!! 此时puserdata6已经无效了。
// 			//g_StoreMessage(pUserData6->);
// 		}
// 	}
}

void CHappy10_5::OnPlayerEnterGame(SGamePlayer *pPlayer, WORD wSeatId)
{
	//分2中情况处理
	//1. 游戏已经开始
	//2. 游戏没有开始
	
    if ( pPlayer == NULL )
        return;

	_SHappy10_5_User_Data *pUserData = &m_vtUserData[wSeatId - 1];
    if ( !pUserData )
        return;

	pUserData->Init();
	pUserData->bQuitGame = FALSE;	//重要标示！
	pUserData->pGamePlayer = pPlayer;
	pUserData->pOutterPlayer = ( CPlayer* )GetPlayerByGID( pPlayer->dwID )->DynamicCast( IID_PLAYER );

    if ( !pUserData->pOutterPlayer )
        return;

	if (GetGameStatus() == gtsPlaying)	//设置成standby.
	{
		pUserData->bIsStandingBy = TRUE;	//!!!
		pUserData->bChosePass = TRUE;	//当作已经pass了， 不过判断是否参与的必要条件还是 bIsStandingBy.	

		//发送现场信息
		sc_HappyTenPointFive_GameStatus status;
		MakeLogicHeader(status.posInfo);
		status.dwAllBaseMoney = m_dwMoneyOnTable;
		status.dwAllLotteryMoney = m_dwAllLotterMoney;
		status.byCurtPlayer = (BYTE)(m_nCurtPlayerIndex + 1);	//当前玩家的计时问题， 认为从头开始， 这个不重要。

		int nIndex = 0;
		for (int i = 0; i < (int)m_vtUserData.size(); i++)
		{
			_SHappy10_5_User_Data *pUserData2 = &m_vtUserData[i];
			if (!pUserData2 || pUserData2->bQuitGame || pUserData2->bIsStandingBy || wSeatId == (i + 1))	//不发送自己和其他等待者的信息
				continue;

			status.arrPlayerInfo[nIndex].byCardCnt = pUserData2->byCardCnt;
			status.arrPlayerInfo[nIndex].bySeatId = i + 1;	
			nIndex ++;
		}	

		status.byPlayersCnt = nIndex;

        g_StoreMessage( pUserData->pOutterPlayer->m_ClientIndex, &status, status.GetSize()); 
	}
	else	
	{
		pUserData->bIsStandingBy = FALSE;
	}
}

void CHappy10_5::OnPlayerLeaveGame(WORD wSeatId)
{
	assert(wSeatId <= m_vtUserData.size());
	_SHappy10_5_User_Data *pUserData = &m_vtUserData[wSeatId - 1];
    if ( !pUserData )
        return;

	assert(!pUserData->bQuitGame);		

	pUserData->Reset();				//离开时候， 游戏数据清零
	pUserData->dwMoneyGet = pUserData->dwMoneyLost = 0;

	pUserData->bQuitGame = TRUE;	//重要标示。

	pUserData->bIsStandingBy = TRUE;	//非必要
	pUserData->pOutterPlayer = NULL;
	pUserData->pGamePlayer = NULL;

	if (GetGameStatus() == gtsPlaying)
	{
		int nPlayerLeft = (int)m_vtUserData.size();		//正在玩耍的玩家（除去离开的， standing by 的。）
		_SHappy10_5_User_Data *pLastUser = NULL;
		for (int i = 0; i < (int)m_vtUserData.size(); i++)
		{
			pUserData = &m_vtUserData[i];
			if (pUserData->bQuitGame || pUserData->bIsStandingBy)
			{
				nPlayerLeft --;
			}
			else
			{
				pLastUser = pUserData;
			}
		}

		if (nPlayerLeft == 1 )	//game over//如果当前玩家离开后只剩一个人则直接导致游戏结束了。
		{
			//制造一个gameover的条件
			/////////////////////////////////////////m_nTurnCnt = 7 + 1;	//直接结束	
			m_nCurtTurnPlayerCnt = 7; //新规则

			GameOver();
			return;
		}
		else
		{
			if (m_nCurtPlayerIndex == (wSeatId - 1))	//当前玩家	， 相当于pass.
			{
				cs_HappyTenPointFive_CommitChoice choice;
				MakeLogicHeader(choice.posInfo);			
				choice.byChoice = 0;
				this->ParserMsg(m_nCurtPlayerIndex + 1, &choice, 1);	
			}
			else
			{
				//剩下的人继续游戏
			}
		}

	}
	else //游戏尚未开始
	{
		SPlayerGameInfo aGame;
		aGame.wGameId = (WORD)m_dwGameSerID;
		aGame.wAreaId = m_pTable->pRoom->pArea->wAreaId;
		aGame.wRoomId = m_pTable->pRoom->wRoomId;
		aGame.wTableId = m_pTable->wTableId;
	}	
}
