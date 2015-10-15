#ifndef _PLUGINGAME_HAPPY10_5_H_
#define _PLUGINGAME_HAPPY10_5_H_

#include "plugingame.h"
#include "poker.h"

struct _SHappy10_5_User_Data
{
	CPlayer *pOutterPlayer;
	SGamePlayer *pGamePlayer;

	BOOL bQuitGame;	//已经退出游戏/没有玩家的标示,充要条件！！
	BOOL bIsStandingBy;	//在游戏过程中加入游戏的玩家。 本局旁观

	BOOL bChosePass; //是否pass过？
	DWORD dwWaitBeginTime;			//开始等待时间。

	WORD wHalfPointCnt;	//半个点数
	BYTE byCardCnt;
	BYTE arrCardHistory[_PLUGINGAME_HAPPY10_5_MAX_TURN_CNT_];	

	DWORD dwMoneyOnTable;	//当前赌资和
	DWORD dwLotteryMoney;	//本轮彩金
	
	BOOL bLastWin;
	DWORD dwMoneyGet;
	DWORD dwMoneyLost;

	_SHappy10_5_User_Data()
	{
		Init();
	};

	//每局开始的reset, 不是结构体reset.
	void Reset()
	{		
		wHalfPointCnt = 0;
		bChosePass = FALSE;
		dwMoneyOnTable = 0;
		dwLotteryMoney = 0;
		bIsStandingBy = FALSE;

		byCardCnt = 0;
		for (int i = 0; i < _PLUGINGAME_HAPPY10_5_MAX_TURN_CNT_; i++)
		{
			arrCardHistory[i] = 0;
		}
	}

	void Init()
	{
		Reset();

		pOutterPlayer = NULL;
		pGamePlayer = NULL;
		bQuitGame = TRUE;

		bLastWin = FALSE;
		dwMoneyGet = dwMoneyLost = 0;
	}
};

class CHappy10_5: public CPlugInGame
{
	CPoker *m_pPoker;

	//int m_nMoneyType;							//钱币类型
	//DWORD m_dwBaseMoney;						//每轮需要的底金
	//DWORD m_dwLotteryMoney;					//彩金

	vector<_SHappy10_5_User_Data> m_vtUserData;

	DWORD m_dwBeginTime;
	
	//int m_nStartPlayerIndexofCurtTurn;		//每轮开始玩家
	int m_nCurtPlayerIndex;						//当前玩家

	//int m_nTurnCnt;							// 当前轮次
	int m_nCurtTurnPlayerCnt;					// 当前轮次已经处理的玩家数量

	//int m_nPassCnt;							// 选择pass的玩家数量

	DWORD m_dwMoneyOnTable;						//当前所有赌资
	DWORD m_dwAllLotterMoney;					//彩金单独计算

	int m_nGameProgress;
private:
	int GetMoneyType();							//钱币类型
	DWORD GetBaseMoneyCnt();					//每轮需要的底金
	DWORD GetLotteryMoneyCnt();					//彩金
    DWORD GetBaseTaxMoney();
    DWORD GetTaxRate();
private:
	void SetNextPlayer(BOOL bFirstTime/*新规则产物*/ = FALSE);	//检测游戏是否改结束了？, 
	int OnPlayer_s_Turn(BOOL bServerAuto = false);
	int CalcHalfPoint(WORD wCardId);

	void SendCardDetails(WORD wSeatId);
public:	
	enum EnGameProgress
	{
		gpWaitting4Start,
		gpBeginTurn,
		gpWaitPlayerChoice,
		gpTurnEnd,		
	};

	enum
	{
		enMaxPlayerCnt = 7
	};
public:
	CHappy10_5(DWORD m_dwGameId);
	virtual ~CHappy10_5();
public:
	virtual void InitData(_RegPluginGame *info);
	virtual void RefreshRegData(SRegDynamicData *pNewData);
	virtual int CheckCanStart();
	virtual BOOL CheckPlayerCanPlay(CPlayer *pPlayer);
	virtual BOOL CheckFinish();

	virtual void GetStartParm(IN OUT DWORD *pParms, OUT WORD &wParmCnt);
	virtual void OnGameStart();
	virtual void OnGameOver(WORD *pArrWinners, int nWinnerCnt);

	virtual void OnPlayerEnterGame(SGamePlayer *pPlayer, WORD wSeatId);
	virtual void OnPlayerLeaveGame(WORD wSeatId);
public:
	virtual void Update(int nTick);
	virtual int ParserMsg(WORD wSeatId, void *pMsg, int nParam = 0);
};	


#endif