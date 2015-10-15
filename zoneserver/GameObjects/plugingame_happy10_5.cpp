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
			if (_GetGameTick() - pUserData->dwWaitBeginTime > PLUGINGAME_HAPPY10_5_MAX_WAIT_CHOICE_TICK)	//�ȴ����ѡ��ʱ�� ϵͳ�Զ�ѡ��pass
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

			if (m_nCurtPlayerIndex != (wSeatId - 1))	//û���ֵ�
				return -1;

			_SHappy10_5_User_Data *pUserData = &m_vtUserData[wSeatId - 1];

            if ( !pUserData )
                break;

			if (pChoice->byChoice == 1) //Ҫ��
			{
				if (pUserData->byCardCnt >= 7)	//���ֻ��Ҫ7����
				{
					CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData->pOutterPlayer, "���������ﵽ����ˡ�");
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
					CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData->pOutterPlayer, "��ĵ���������Ҫ�ˣ�");
					pUserData->bChosePass = 1;
				}
				else if ( pUserData->pOutterPlayer && (pUserData->pOutterPlayer->GetMoney( GetMoneyType() ) < GetBaseMoneyCnt()) )
				{
					CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData->pOutterPlayer, "��ĵ׽��㣡");
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

			//֪ͨ�ͻ���			
			playerChoice.byChice = pUserData->bChosePass ? 0 : 1;
			bool bSend = true;
			for (int i = 0; i < (int)m_vtUserData.size(); i++)
			{
				if (/*(i != (wSeatId - 1)) && */!m_vtUserData[i].bQuitGame && m_vtUserData[i].pOutterPlayer)
                    g_StoreMessage( m_vtUserData[i].pOutterPlayer->m_ClientIndex, &playerChoice, sizeof playerChoice); 
			}

			if (!pUserData->bChosePass)	//����
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


			//1 ����Ƿ����
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
	////GetBaseMoneyCnt() = info->areaParms[m_pTable->pRoom->pArea->wAreaId - 1].arrParm10[1];		//ÿ�ֵ׽�
	////GetLotteryMoneyCnt() = info->areaParms[m_pTable->pRoom->pArea->wAreaId - 1].arrParm10[2];	//�ʽ�
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

		//1. ����Ƿ��г�ͻ�Ľ��ף�

		//2. ��ҽ�Ǯ�Ƿ��㹻��
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
	//1. ����Ƿ��г�ͻ�Ľ��ף�

	//2. ��ҽ�Ǯ�Ƿ��㹻��
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

	//�����ֽ��, �۲ʽ�  + ���� �� todo: �Ƿ���Ҫ�����ҽ�Ǯ�Ƿ��㹻��

	/////////////////////////////////////////////////m_nTurnCnt ++;	

	sc_HappyTenPointFive_OneCard aCard;
	MakeLogicHeader(aCard.posInfo);

	for (int i = 0; i < (int)m_pTable->m_vtSeats.size(); i++)
	{
		_SHappy10_5_User_Data *pUserData = &m_vtUserData[i];

		if ( pUserData == NULL ||  pUserData->bQuitGame || pUserData->pOutterPlayer == NULL )	//��λ
			continue;

		pUserData->Reset();

		CPlayer *pOutterPlayer = pUserData->pOutterPlayer;

		//�۲ʽ�+���ֵ׽�
		pOutterPlayer->DelMoney(GetMoneyType(), GetBaseMoneyCnt() + GetLotteryMoneyCnt() );
		pUserData->dwMoneyOnTable = GetBaseMoneyCnt();
		pUserData->dwLotteryMoney = GetLotteryMoneyCnt();

		m_dwMoneyOnTable += GetBaseMoneyCnt();
		m_dwAllLotterMoney += GetLotteryMoneyCnt();

		///����Ĭ�� ��Ҫ�ơ�
		aCard.byCardId = m_pPoker->GetOne();
		WORD wHalfPoint = CalcHalfPoint(aCard.byCardId);
		pUserData->wHalfPointCnt += wHalfPoint;
		pUserData->arrCardHistory[pUserData->byCardCnt++] = aCard.byCardId;

		//֪ͨ�ͻ���
		sc_HappyTenPointFive_PlayerChoice playerChoice;
		MakeLogicHeader(playerChoice.posInfo);
		playerChoice.byChice = 1; //Ҫ�ơ�
		playerChoice.bySeatId = i + 1;
		for (int j = 0; j < (int)m_pTable->m_vtSeats.size(); j++)
		{
			if (!m_vtUserData[j].bQuitGame)
                g_StoreMessage( m_vtUserData[j].pOutterPlayer->m_ClientIndex, &playerChoice, sizeof playerChoice); 
		}

		//ϵͳ����
		g_StoreMessage(pOutterPlayer->m_ClientIndex, &aCard, sizeof aCard);
	}


	m_nGameProgress = gpWaitPlayerChoice;

	SetNextPlayer(TRUE);
	OnPlayer_s_Turn();

	//�������ˣ� ̫���ˣ� �ͻ��˿��������
	//////////////////�����⴦���һ����ҡ�		 
	////////////////SetNextPlayer(TRUE);	
	////////////////if (0 == OnPlayer_s_Turn())	//��һ����ҿ���Ҫ��
	////////////////{
	////////////////	return;
	////////////////}
	////////////////else 
	////////////////{
	////////////////	//kevin  @ 2008.10.31
	////////////////	//���������Ҷ�ûǮ�ˣ� ����Ϸһ��ʼ�ͻ������

	////////////////	//new ����Ҫ�ֵ���һ������Ҫ�Ƶ����
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
		//��һ����ҵ������ǣ���ǰ����Ѿ�pass�ˣ�standby����ұ�ǿ�����ó�pass�ˡ�����
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

	//////////////һ���Ƿ����
	////////////if (m_nCurtTurnPlayerCnt >= m_pTable->m_vtSeats.size()) //һ�ֽ���
	////////////{
	////////////	m_nTurnCnt++;
	////////////	m_nPassCnt = 0;
	////////////	m_nCurtTurnPlayerCnt = 0;

	////////////	// �任���ַ����ߡ�
	////////////	m_nStartPlayerIndexofCurtTurn ++;
	////////////	m_nStartPlayerIndexofCurtTurn %= m_pTable->m_vtSeats.size();
	////////////	m_nCurtPlayerIndex = m_nStartPlayerIndexofCurtTurn;

	////////////	//m_nCurtPlayerIndex = 0;
	////////////}
	////////////else//��һ�����
	////////////{
	////////////	m_nCurtPlayerIndex ++;
	////////////	m_nCurtPlayerIndex %= m_pTable->m_vtSeats.size(); //or m_vtusreData.size();
	////////////}
}

int CHappy10_5::OnPlayer_s_Turn(BOOL bServerAuto /* = false */)
{
	//1. �Ƿ�pass��
	//2. �Ƿ��������

	////////////////////////m_nCurtTurnPlayerCnt++;

	_SHappy10_5_User_Data *pUserData = NULL;
	pUserData = &m_vtUserData[m_nCurtPlayerIndex];	

    if ( pUserData == NULL || pUserData->pOutterPlayer == NULL )
        return -1;

	if (pUserData->bChosePass || pUserData->bQuitGame || pUserData->bIsStandingBy/* standby����ұ�ǿ�����ó�pass�ˡ�*/) //!!!!
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
			// �׽���      
            CPlugInGameManager::GetInstance()->SendPlugInGameErr( pUserData->pOutterPlayer->m_ClientIndex, -10 );
			CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData->pOutterPlayer, "�׽���,���ܼ���Ҫ���ˣ�");
		}


		/////////////////////////////m_nPassCnt ++;
		return -2;
	}

	if (pUserData->wHalfPointCnt >= 10 * 2 + 1)	//10.5 * 2
	{
		pUserData->bChosePass = true;
		if (!bServerAuto /*&& !pUserData->wNotifyTimesOnFrezze*/)
		{			
			CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData->pOutterPlayer, "��ĵ����Ѿ�������Ҫ��Ŷ��");
		}

		/////////////////////////////m_nPassCnt ++;
		/////////////////////////////pUserData->wNotifyTimesOnFrezze ++;

		return -3;
	}

	//ok ��ʾ����Ƿ�Ҫ��
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
	//standingby�߲������

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
	//1. �ҳ�ʤ���ߡ�
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
// 		if (pUserData1->bIsStandingBy)	//��;�������Ҳ�Ӱ����
// 			continue;
// 
// 		dwAllMoney += pUserData1->dwMoneyOnTable;
// 		dwAllLotteryMoney += pUserData1->dwLotteryMoney;
// 
// 		if (pUserData1->bQuitGame)	//��λ������;�˳���Ϸ���������Ϊʧ�ܡ�
// 		{
// 			pUserData1->wHalfPointCnt = 999;
// 		}
// 
// 		//�������������ϸ��Ϣ
// 		SendCardDetails(i + 1);
// 
// 		if (pUserData1->wHalfPointCnt > 21)//�϶���ʧ����	
// 		{
// 			pUserData1->dwMoneyLost += pUserData1->dwMoneyOnTable;
// 		}
// 		else	//���ܵ�ʤ��
// 		{
// 			if (pUserData1->wHalfPointCnt > wMaxPoint)
// 				wMaxPoint = pUserData1->wHalfPointCnt;
// 		}
// 	}
// 
// 	if (wMaxPoint == 0 ) //û��ʤ����
// 	{
// 		int i = 123;
// 	}
// 	else
// 	{
// 		list<_SHappy10_5_User_Data*> lstWinner;       
// 
// 		lstWinner.clear();
// 		for (int i = 0; i < (int)m_vtUserData.size(); i++)	//�ҳ�ʤ���� etc.
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
// 		//���ͽ����� ��˰
// 		DWORD dwBounds = dwAllMoney >= GetBaseTaxMoney() ? ( (dwAllMoney - (dwAllMoney * GetTaxRate()/100)) / lstWinner.size()) : (dwAllMoney / lstWinner.size());
//         DWORD dwPersonalTax = dwAllMoney / lstWinner.size() - dwBounds;
// 
// 		DWORD dwLotteryMoney = 0;
// 		if (wMaxPoint == 21)	//�ֲʽ� �����˻��ʽ�
// 		{
// 			dwLotteryMoney = dwAllLotteryMoney / lstWinner.size();
// 
// 			//��¼δ�вʵ���ʧ�ʽ�
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
// 		//��Ǯ�ˡ�
// 		for (list<_SHappy10_5_User_Data*>::iterator it = lstWinner.begin(); it != lstWinner.end(); ++it)
// 		{
// 			_SHappy10_5_User_Data *pUserData4 = *it;
//             if ( !pUserData4 )
//                 continue;
// 			pUserData4->dwMoneyGet += (dwBounds + dwLotteryMoney);
// 			pUserData4->pOutterPlayer->AddMoney(GetMoneyType(), dwBounds + dwLotteryMoney);
// 			CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData4->pOutterPlayer, "��ϲ���ñ���ʤ��!");
// 
// 			char strMoney[10] = {0};
// 			switch(GetMoneyType())
// 			{
// 			case MoneyType_Yuanbao:				
// 				strcpy(strMoney, "Ԫ��");
// 				break;
// 			case MoneyType_Gift:
// 				strcpy(strMoney, "����");
// 				break;
// 			case MoneyType_Gold:
// 				strcpy(strMoney, "���");
// 				break;
// 			}
// 
// 			CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData4->pOutterPlayer, "��ֵ���%d%s", dwBounds, strMoney);
//             if ( (dwAllMoney * GetTaxRate()/100) > 0 )
//                 CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData4->pOutterPlayer, "�����˰��%d%s", dwPersonalTax, strMoney);
// 
// 			if (dwLotteryMoney > 0)
// 				CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData4->pOutterPlayer, "�õ��ʽ�%d%s", dwLotteryMoney, strMoney);
// 		}
// 	}  
// 
// 	//�˻��ʽ�
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
// 				CPlugInGameManager::GetInstance()->SendSysMsgTip(pUserData5->pOutterPlayer, "�����вʣ��˻��ʽ�");
// 				if (!pUserData5->bIsStandingBy)
// 				{
// 					pUserData5->pOutterPlayer->AddMoney(GetMoneyType(), pUserData5->dwLotteryMoney);	
// 				}
// 			}
// 		}
// 	}
// 
// 
// 	//֪ͨ������ �� �޸���Ϸ״̬��
// 	m_nGameProgress = gpWaitting4Start;	
// 	CPlugInGame::OnGameOver(arrWinner, nTmp);
// 
// 	//4. ���׽� ���������ˡ�
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
// 			// �׽���
//             CPlugInGameManager::GetInstance()->SendPlugInGameErr( pUserData6->pOutterPlayer->m_ClientIndex, -10 );
// 			//DWORD dwPlayer
// 
// 			//ע���ʱ��Ϸ�Ѿ������� ��Ҫ����Ϸ״̬�޸ĳ� gtsWaitting, �������gameover, ���·��ͽ�����ظ��˻��ʽ�			
//             int result = CPlugInGameManager::GetInstance()->PlayerLeaveTable((WORD)m_dwGameSerID, pUserData6->pGamePlayer, pUserData6->pOutterPlayer, TRUE);
//             if ( result < 0 )
//                 CPlugInGameManager::GetInstance()->SendPlugInGameErr( pUserData6->pOutterPlayer->m_ClientIndex, result );
// 
// 			//!!! ��ʱpuserdata6�Ѿ���Ч�ˡ�
// 			//g_StoreMessage(pUserData6->);
// 		}
// 	}
}

void CHappy10_5::OnPlayerEnterGame(SGamePlayer *pPlayer, WORD wSeatId)
{
	//��2���������
	//1. ��Ϸ�Ѿ���ʼ
	//2. ��Ϸû�п�ʼ
	
    if ( pPlayer == NULL )
        return;

	_SHappy10_5_User_Data *pUserData = &m_vtUserData[wSeatId - 1];
    if ( !pUserData )
        return;

	pUserData->Init();
	pUserData->bQuitGame = FALSE;	//��Ҫ��ʾ��
	pUserData->pGamePlayer = pPlayer;
	pUserData->pOutterPlayer = ( CPlayer* )GetPlayerByGID( pPlayer->dwID )->DynamicCast( IID_PLAYER );

    if ( !pUserData->pOutterPlayer )
        return;

	if (GetGameStatus() == gtsPlaying)	//���ó�standby.
	{
		pUserData->bIsStandingBy = TRUE;	//!!!
		pUserData->bChosePass = TRUE;	//�����Ѿ�pass�ˣ� �����ж��Ƿ����ı�Ҫ�������� bIsStandingBy.	

		//�����ֳ���Ϣ
		sc_HappyTenPointFive_GameStatus status;
		MakeLogicHeader(status.posInfo);
		status.dwAllBaseMoney = m_dwMoneyOnTable;
		status.dwAllLotteryMoney = m_dwAllLotterMoney;
		status.byCurtPlayer = (BYTE)(m_nCurtPlayerIndex + 1);	//��ǰ��ҵļ�ʱ���⣬ ��Ϊ��ͷ��ʼ�� �������Ҫ��

		int nIndex = 0;
		for (int i = 0; i < (int)m_vtUserData.size(); i++)
		{
			_SHappy10_5_User_Data *pUserData2 = &m_vtUserData[i];
			if (!pUserData2 || pUserData2->bQuitGame || pUserData2->bIsStandingBy || wSeatId == (i + 1))	//�������Լ��������ȴ��ߵ���Ϣ
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

	pUserData->Reset();				//�뿪ʱ�� ��Ϸ��������
	pUserData->dwMoneyGet = pUserData->dwMoneyLost = 0;

	pUserData->bQuitGame = TRUE;	//��Ҫ��ʾ��

	pUserData->bIsStandingBy = TRUE;	//�Ǳ�Ҫ
	pUserData->pOutterPlayer = NULL;
	pUserData->pGamePlayer = NULL;

	if (GetGameStatus() == gtsPlaying)
	{
		int nPlayerLeft = (int)m_vtUserData.size();		//������ˣ����ң���ȥ�뿪�ģ� standing by �ġ���
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

		if (nPlayerLeft == 1 )	//game over//�����ǰ����뿪��ֻʣһ������ֱ�ӵ�����Ϸ�����ˡ�
		{
			//����һ��gameover������
			/////////////////////////////////////////m_nTurnCnt = 7 + 1;	//ֱ�ӽ���	
			m_nCurtTurnPlayerCnt = 7; //�¹���

			GameOver();
			return;
		}
		else
		{
			if (m_nCurtPlayerIndex == (wSeatId - 1))	//��ǰ���	�� �൱��pass.
			{
				cs_HappyTenPointFive_CommitChoice choice;
				MakeLogicHeader(choice.posInfo);			
				choice.byChoice = 0;
				this->ParserMsg(m_nCurtPlayerIndex + 1, &choice, 1);	
			}
			else
			{
				//ʣ�µ��˼�����Ϸ
			}
		}

	}
	else //��Ϸ��δ��ʼ
	{
		SPlayerGameInfo aGame;
		aGame.wGameId = (WORD)m_dwGameSerID;
		aGame.wAreaId = m_pTable->pRoom->pArea->wAreaId;
		aGame.wRoomId = m_pTable->pRoom->wRoomId;
		aGame.wTableId = m_pTable->wTableId;
	}	
}
