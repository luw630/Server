/*
	File: Poker class.
	Author: kevin
	Date: 2008.9
*/

#include "stdafx.h"
#include "poker.h"
#include <list>

using namespace std;

int CPoker::m_arrRndNums[_MAX_RAND_NUMS_];
int CPoker::m_nRndIndex = 0;

CPoker::CPoker(bool bNeedJoker)
{
	srand( GetTickCount() );

    for (int i = 0; i < _MAX_RAND_NUMS_; i++)
    {
        m_arrRndNums[i] = rand();
    }

	m_bNeedJoker = bNeedJoker;

	if (m_bNeedJoker)
		m_nTotalCardCnt = 54;
	else
		m_nTotalCardCnt = 52;

	Init();
}

CPoker::~CPoker()
{
	//
}

int CPoker::Init()
{
	m_nNextCard = 0;
	return 0;
}

int CPoker::Reset()
{
	m_nNextCard = 0;

	struct _S1
	{
		int nValue;
		_S1 *pNext;
	};

	int nCardCnt = m_nTotalCardCnt;
    if ( nCardCnt - 1 < 0 || nCardCnt - 1 >= 54 )
        return 0;

	_S1 arrAllCards[54];	//array list.

	/////////////////////////////////////////////////// 
	arrAllCards[0].nValue = 1;
	arrAllCards[0].pNext = &arrAllCards[1];
	for (int i = 1; i < nCardCnt -1; i++)
	{
		arrAllCards[i].nValue = i + 1;
		arrAllCards[i].pNext = &arrAllCards[i+1];     
	}
	arrAllCards[nCardCnt-1].nValue = nCardCnt;
	arrAllCards[nCardCnt-1].pNext = NULL;
	/////////////////////////////////////////////////

	_S1 *pHead = &arrAllCards[0];
	_S1 *pPre = NULL;
	for (int i = 0; i < nCardCnt; i++)
	{
		int nCardValue = 0;
		int nTmpIndex = 0;
		_S1 *pCard = NULL;		

        m_nRndIndex = m_nRndIndex % _MAX_RAND_NUMS_; 
		int nRndCardIndex = m_arrRndNums[m_nRndIndex++] % (nCardCnt - i);

		if (nRndCardIndex == 0)
		{
			pCard = pHead;
			pHead = pHead->pNext;
			pPre = NULL;
		}
		else
		{
			pCard = pHead;
			for (int j = 0; j < nRndCardIndex; j++)
			{		
				if (j == nRndCardIndex - 1)
				{
					pPre = pCard;
				}

				pCard = pCard->pNext;
			}
		}

		m_arrCards[i] = pCard->nValue;

		//////////¸üÐÂÁ´±í
		if (pPre != NULL)
		{
			pPre->pNext = pCard->pNext;
		}
	}

	return 0;
}

int CPoker::GetOne()
{
	if (m_nNextCard > m_nTotalCardCnt)	
	{
		return POKER_NO_CARD;
	}

	return m_arrCards[m_nNextCard++];
}

int CPoker::GetPokerCntLeft()
{
	if (m_nNextCard > m_nTotalCardCnt - 1)	
	{
		return 0;
	}
	else
	{
		return m_nTotalCardCnt - m_nNextCard;
	}
}