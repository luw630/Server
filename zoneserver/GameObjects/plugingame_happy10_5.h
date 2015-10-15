#ifndef _PLUGINGAME_HAPPY10_5_H_
#define _PLUGINGAME_HAPPY10_5_H_

#include "plugingame.h"
#include "poker.h"

struct _SHappy10_5_User_Data
{
	CPlayer *pOutterPlayer;
	SGamePlayer *pGamePlayer;

	BOOL bQuitGame;	//�Ѿ��˳���Ϸ/û����ҵı�ʾ,��Ҫ��������
	BOOL bIsStandingBy;	//����Ϸ�����м�����Ϸ����ҡ� �����Թ�

	BOOL bChosePass; //�Ƿ�pass����
	DWORD dwWaitBeginTime;			//��ʼ�ȴ�ʱ�䡣

	WORD wHalfPointCnt;	//�������
	BYTE byCardCnt;
	BYTE arrCardHistory[_PLUGINGAME_HAPPY10_5_MAX_TURN_CNT_];	

	DWORD dwMoneyOnTable;	//��ǰ���ʺ�
	DWORD dwLotteryMoney;	//���ֲʽ�
	
	BOOL bLastWin;
	DWORD dwMoneyGet;
	DWORD dwMoneyLost;

	_SHappy10_5_User_Data()
	{
		Init();
	};

	//ÿ�ֿ�ʼ��reset, ���ǽṹ��reset.
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

	//int m_nMoneyType;							//Ǯ������
	//DWORD m_dwBaseMoney;						//ÿ����Ҫ�ĵ׽�
	//DWORD m_dwLotteryMoney;					//�ʽ�

	vector<_SHappy10_5_User_Data> m_vtUserData;

	DWORD m_dwBeginTime;
	
	//int m_nStartPlayerIndexofCurtTurn;		//ÿ�ֿ�ʼ���
	int m_nCurtPlayerIndex;						//��ǰ���

	//int m_nTurnCnt;							// ��ǰ�ִ�
	int m_nCurtTurnPlayerCnt;					// ��ǰ�ִ��Ѿ�������������

	//int m_nPassCnt;							// ѡ��pass���������

	DWORD m_dwMoneyOnTable;						//��ǰ���ж���
	DWORD m_dwAllLotterMoney;					//�ʽ𵥶�����

	int m_nGameProgress;
private:
	int GetMoneyType();							//Ǯ������
	DWORD GetBaseMoneyCnt();					//ÿ����Ҫ�ĵ׽�
	DWORD GetLotteryMoneyCnt();					//�ʽ�
    DWORD GetBaseTaxMoney();
    DWORD GetTaxRate();
private:
	void SetNextPlayer(BOOL bFirstTime/*�¹������*/ = FALSE);	//�����Ϸ�Ƿ�Ľ����ˣ�, 
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