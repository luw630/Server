/*
	File: Poker class.
	Author: kevin
	Date: 2008.9	
*/

#ifndef __POKER_H__
#define __POKER_H__

#define POKER_NO_CARD (0)
#define POKER_MAX_CARD_ID (54)
#define POKER_MAX_CARD_ID_WITHOUT_JOKER (52)
#define _MAX_RAND_NUMS_ 20000

enum EnPoker_CardClass
{
	pccRad,
	pccBlck,
	pccFlower,
	pccRect,
	pccKings
};

enum EnPoker_BaseCardValue
{
	//A
	pcvAce = 1,
	pcvOne = pcvAce,
	
	//2..10
	pcvTwo = 2,
	pcvTen = 10,

	//J,Q,K,
	pcvJack = 11,
	pcvQueen = 12,
	pcvKing = 13,

	//other
	pcvJokerB = 53,
	pcvJokerS = 54
};

#define _BASETYPECARDCNT_ (13)

class CPoker
{
    static int m_arrRndNums[_MAX_RAND_NUMS_];
    static int m_nRndIndex;
	int m_arrCards[54];
	int m_nTotalCardCnt;

	int m_nNextCard;
	bool m_bNeedJoker;
public:
	CPoker(bool bNeedJoker);	//是否需要大小王 ？
	~CPoker();
public:
	int Init();
	int Reset();

	int GetOne();
	int GetPokerCntLeft();
};

#endif