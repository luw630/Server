/*	
	File: plugin mini game baisc define
	Desc: .
	Created by: kevin
	Date: 09.09.18
*/

#ifndef _PLUGIN_GAME_H_
#define _PLUGIN_GAME_H_

#include "NETWORKMODULE/plugingamemsg.h"
#include "player.h"
#include <vector>
#include "GameWorld.h"
#include <string>

using namespace std;

#define PLUGINGAME_NULL_0	        (0)
#define PLUGINGAME_MAX_AREA_CNT		(1024)
#define PLUGINGAME_MAX_ROOM_CNT		(1024)
#define PLUGINGAME_MAX_TABLE_CNT	(1024)
#define PLUGINGAME_MAX_SEAT_CNT		(32)

#define SINGLE_TICK_TIME_MS			(1000/TICKS_PER_SECOND)
#define TICKS_PER_SECOND			(GAME_FPS/5)
#define NULL_PLAYER_ID              (0)       
#define MAX_NAMELEN                 (CONST_USERNAME)
#define GAME_FPS                    (5)

class CPlugInGameManager;
class CPlugInGame;
struct SGameTable;  // 房间桌子，现在一个房间就一个桌子
struct SGameRoom;   // 房间
struct SGameArea;   // 新手--至尊
struct SGameFloor;  // 楼层， 1个楼层对应一种游戏
struct SGameIntro;

enum EGamePlace
{
	gpOuterWorld,
	gpHall,
	gpArea,
	gpRoom,
	gpTable
};

enum EPluginGameTable_status
{
	gtsUnknown = 0,
	gtsWaitting = 1,
	gtsPlaying = 2
};

enum EPluginGame_PlayerStatus
{
	ppsNone = 0,	//不在游戏大楼

	//暂时做相同处理/////////////
	ppsHall = 1,		//在大厅选游戏
	ppsSelecting = 1,	//选择房间， 座位
	///////////////////////////////

	ppsGameLine	,	//游戏界限

	ppsStandingBy,	//游戏正在进行中加入游戏， 等待下一轮开始。！！！！！！

	ppsReadying,	//刚刚游戏开始或刚刚结束
	ppsWaitting4Start, //准备好， 等待开始
	ppsPlaying		//正在玩耍游戏
};

typedef struct SPlayerPosInf
{
	WORD wGameId;	//id从1开始
	WORD wAreaId;
	WORD wRoomId;
	WORD wTableId;
	WORD wSeatId;

	DWORD dwLastSitDownTime;	//60秒内不开始就起立

	SPlayerPosInf()
	{
		Reset();
	};

	void Reset()
	{
		wGameId = wAreaId = wRoomId = wTableId = wSeatId = PLUGINGAME_NULL_0;
		dwLastSitDownTime = 0;
	}

}SPlayerGameInfo;

struct SGamePlayer
{
	DWORD dwID;
	char strName[MAX_NAMELEN];
	EPluginGame_PlayerStatus enGameStatus;
	SPlayerGameInfo posInfo;

	//其他关注的
	//WORD wPossibleGameId;
	//WORD wPossibleAreaId;
	//WORD wPossibleRoomId;

	SPlayerGameInfo tmpPos;	//关注的

	//
	DWORD dwLastSitDwonTime;

	SGamePlayer()
	{
		//wPossibleAreaId = wPossibleGameId = wPossibleRoomId = dwID = PLUGINGAME_NULL_0;
		posInfo.Reset();
		tmpPos.Reset();

		enGameStatus = ppsNone;
		dwLastSitDwonTime = 0;
	}

	inline EPluginGame_PlayerStatus GetStatus(){ return enGameStatus; };
	inline void SetStatus(EPluginGame_PlayerStatus status){ enGameStatus = status; };
}; 

struct SRegAreaParm
{
	//tips: 注册一个游戏有n个参数, 其中前几个参数固定,  主要描述，金钱奖惩等区别， 不描述游戏逻辑规则问题！
	bool bIsOpen;
	DWORD arrParm10[10];

	int GetMoneyType() { return (int)arrParm10[0];};

	DWORD GetBaseMoney() { return arrParm10[1]; };
	DWORD GetLotteryMoney() { return arrParm10[2]; };
	DWORD GetEnterMoneyCnt() { return arrParm10[1] + arrParm10[2]; };
    DWORD GetBaseTaxMoney() { return arrParm10[3]; };
    DWORD GetTaxRate() { return (arrParm10[4] <= 100 ? arrParm10[4] : 5); };
};

struct _RegPluginGame
{    
public:
	char strGameName[PLUGINGAME_MAX_GAME_NAME_LEN];
	WORD wGameId;
	WORD wAreaCnt;
	WORD wRoomCnt;
	WORD wTableCnt;
	WORD wMaxPlayerCnt;	//每张桌子
	WORD wMinPlayerCnt;

	//BOOL bAllowWatchOnly;	//允许绝对旁观者 not yet.

	BOOL bAllowStandBy;		//允许随时可以加入的游戏， 这代表了2种游戏（1： 固定人数， 2： 人数不固定）

	//注册的附加参数, 主要描述不同分区的收费和规则差别。 所有游戏 前2个参数固定表示收费信息，3-4 留用。 不定参数在第5个以后
	SRegAreaParm areaParms[PLUGINGAME_MAX_GAME_AREA_CNT];	//areaParms: 主要对应上边的分区， 一般大厅都是不同的区才会有细小的差别， 体现在金钱奖惩上。	


	_RegPluginGame() { memset(strGameName, 0, PLUGINGAME_MAX_GAME_NAME_LEN); };
};

struct SRegDynamicData
{
	WORD Parms[PLUGINGAME_MAX_GAME_AREA_CNT];
};

struct SGameTable		//桌子
{	
	WORD wTableId;
	SGameRoom *pRoom;
	CPlugInGame *pOneGame;
	BOOL __bAsyncTryStartGame;
public:
	void SetAsyncStartCheckTag(BOOL bTag = TRUE ){ __bAsyncTryStartGame = bTag; };
	WORD GetGameId();
	WORD GetRoomId();
	WORD GetAreaId();	
	SGameIntro* GetGameIntro();
public:
	vector<DWORD> m_vtSeats;
	WORD wCurtPlayerCnt;
	SGameTable(_RegPluginGame *info, SGameRoom *pRoom, WORD wTableId);
	~SGameTable();
	BOOL GetSeatPlayer(WORD wSeatId, DWORD & dwOutPlayerID);
	BOOL PlayerLeaveSeat(WORD wSeatId);
	void Update(int nTick);
	//void RefreshRegData(SRegDynamicData *pData);
};

struct SGameRoom		//房间
{	
	WORD wRoomId;
	SGameArea *pArea;	
public:
	vector<SGameTable*> m_vtTables;
	DWORD wCurtPlayerCnt;
	DWORD wPlayerCapcity;
	map<DWORD, DWORD> m_mapAllPlayers;
public:
	SGameRoom(_RegPluginGame *info, SGameArea *pArea, WORD wRoomId)
	{		
		wCurtPlayerCnt = 0;
		wPlayerCapcity = info->wTableCnt * info->wMaxPlayerCnt;
		this->pArea = pArea;
		this->wRoomId = wRoomId;

		m_vtTables.resize(info->wTableCnt);	
		for (int i = 0; i < (int) info->wTableCnt; i++)
		{
			m_vtTables[i] = new SGameTable(info, this, i+1);
		}
	}

	~SGameRoom()
	{
		for (DWORD i = 0; i < m_vtTables.size(); i++)
		{
			delete m_vtTables[i];
			m_vtTables[i] = NULL;
		}
	}

	SGameTable* GetTable(WORD wTableId)
	{
		if (wTableId == 0 || wTableId > m_vtTables.size())
			return NULL;
		else
			return m_vtTables[wTableId - 1];
	}

	void Update(int nTick)
	{
		if (m_vtTables.size() == 0)
			return;
		for (int i = 0; i < (int) (int)m_vtTables.size(); i++)
		{
			m_vtTables[i]->Update(nTick);
		}
	}

	//void RefreshRegData(SRegDynamicData *pData)
	//{
	//	for (int i = 0; i < (int)m_vtTables.size(); i++)
	//	{
	//		m_vtTables[i].RefreshRegData(pData);
	//	}
	//}
};

struct SGameArea		//分区
{
	SGameFloor *pFloor;
	vector<SGameRoom*> m_vtRooms;
public:
	WORD wAreaId;
	DWORD wCurtPlayerCnt;
	DWORD wPlayerCapcity;
	//map<DWORD, DWORD> m_mapAllPlayers;	 
public:
	SGameArea(_RegPluginGame *info, SGameFloor *pFloor, WORD wAreaId)
	{
		wCurtPlayerCnt = 0;
		wPlayerCapcity = info->wRoomCnt * info->wTableCnt * info->wMaxPlayerCnt;
		this->pFloor = pFloor;
		this->wAreaId = wAreaId;

		m_vtRooms.resize(info->wRoomCnt);
		for (int i = 0; i < (int) info->wRoomCnt; i++)
		{
			m_vtRooms[i] = new SGameRoom(info, this, i+1);
		}
	}

	~SGameArea()
	{
		for (DWORD i = 0; i < m_vtRooms.size(); i++)
		{
			delete m_vtRooms[i];
			m_vtRooms[i] = NULL;
		}
	}

	SGameRoom *GetRoom(WORD wRoomId)
	{
		if (wRoomId == 0 || wRoomId > m_vtRooms.size())		
			return NULL;
		else
			return m_vtRooms[wRoomId - 1];		
	}

	void Update(int nTick)
	{
		if (m_vtRooms.size() == 0)
			return;
		for (int i = 0; i < (int) m_vtRooms.size(); i++)
		{
			m_vtRooms[i]->Update(nTick);
		}
	}

	//void RefreshRegData(SRegDynamicData *pData)
	//{
	//	for (int i = 0; i < (int) m_vtRooms.size(); i++)
	//	{
	//		m_vtRooms[i].RefreshRegData(pData);
	//	}	
	//}
};

struct SGameFloor		//楼层， 1个楼层对应一种游戏
{
	SGameIntro *pGameIntro;	
	vector<SGameArea*> m_vtAreas;
public:	
	SGameFloor()
	{

	};
	SGameFloor(_RegPluginGame *info)
	{
		m_vtAreas.resize(info->wAreaCnt);
		for (int i = 0; i < (int) info->wAreaCnt; i++)
		{
			m_vtAreas[i] = new SGameArea(info, this, i+1);
		}
	}

	~SGameFloor()
	{
		for (DWORD i = 0; i < m_vtAreas.size(); i++)
		{
			delete m_vtAreas[i];
			m_vtAreas[i] = 0;
		}
	}

	SGameArea* GetArea(WORD wId)
	{
		if (wId == 0 || wId > m_vtAreas.size())
			return NULL;
		else
			return m_vtAreas[wId - 1];
	}

	void Update(int nTick)
	{
		if (m_vtAreas.size() == 0)
			return;
		for (int i = 0; i < (int) m_vtAreas.size(); i++)
		{
			m_vtAreas[i]->Update(nTick);
		}
	}
};

struct SGameIntro
{
public:
	_RegPluginGame gameCode;
	SGameFloor *pGameData;
	
	SGameIntro()
	{
		pGameData = NULL;
	}

	~SGameIntro()
	{
		if (NULL != pGameData)	
			delete pGameData;
		pGameData = NULL;
	}
};

enum EnPluinGameErrCode
{
	ecNone = 0,
	ecNotEnoughtMoney = 1,
	ecP2PTrading = 2,
	ecP2CTrading = 3,
	ecAccessingStorage = 4
};


class CPlugInGame
{
protected:
	DWORD m_dwGameSerID;			//游戏编号

	int m_nGameStatus;
	int m_nMinPlayerCnt;
	int m_nMaxPlayerCnt;

	SGameTable *m_pTable;
public:
	CPlugInGame(DWORD dwGameId);
	virtual ~CPlugInGame();
public:
	void SetGameStatus(int nStatus){ m_nGameStatus = nStatus; };
	int GetGameStatus(){ return m_nGameStatus; };

	virtual void InitData(_RegPluginGame *info) = 0;
	virtual void RefreshRegData(SRegDynamicData *pNewData) = 0;
	virtual int CheckPlayerCanPlay(CPlayer *pPlayer) = 0;	//
	virtual int CheckCanStart() = 0;
	virtual BOOL CheckFinish() = 0;	// 

	virtual void GetStartParm(IN OUT DWORD *pParms, OUT WORD &wParmCnt) = 0;
	void StartGame();
	virtual void OnGameStart() = 0;
	void GameOver();
	virtual void OnGameOver(WORD *pArrWinners, int nWinnerCnt);

	void PlayerEnterGame(SGamePlayer *pPlayer, WORD wSeatId){ OnPlayerEnterGame(pPlayer, wSeatId); };
	virtual void OnPlayerEnterGame(SGamePlayer *pPlayer, WORD wSeatId) = 0;
	void PlayerLeaveGame(WORD wSeatId){ OnPlayerLeaveGame(wSeatId); };
	virtual void OnPlayerLeaveGame(WORD wSeatId) = 0;

	//virtual int KickPlayer(int nSeatId);

	virtual void Update(int nTick) = 0;
	virtual int ParserMsg(WORD wSeatId, void *pMsg, int nParam = 0) = 0;
public:
	void MakeLogicHeader(__SPosInfo& pData);
public:
	friend class CPlugInGameManager;
	friend struct SGameTable;
};



class CPlugInGameManager
{
	map<DWORD, SGamePlayer> m_mapAllPlayers;
	map<DWORD, SGameIntro*> m_mapAllGames;				
	list<DNID> m_lstBadGuys;                    // 不忠实玩家

	CPlugInGameManager();
	~CPlugInGameManager();
private:
	void __QuitHall(SGameIntro *pGame, SGamePlayer *pPlayer, SGameArea *pArea, SGameRoom *pRoom, SGameTable *pTable);
	int QuitGame(DWORD dwPlayer);
public:	//todo . private 

	BOOL CheckPlayerExists(DWORD dwID);
	SGameIntro* GetGameIntro(DWORD dwGameId);
	SGamePlayer* GetGamePlayer(DWORD dwPlayerId);
	int QuietlyQuit(SGamePlayer* pPlayer, int nQuit2Where);	
	int BroadCastMsg(int nToWhere, void *pMsg, int nSize, WORD wGameId = 0, WORD wAreaId = 0, WORD wRoomId = 0, WORD wTableId = 0);
	int ParserMsg(DNID dnidClient, SPluginGameBaseMsg *pMsg);

	void SyncAreaPlayerCnt(WORD wGameId, WORD wAreaId);
	void SyncRoomPlayerCnt(WORD wGameId, WORD wAreaId, WORD wRoomId);
	void SyncAreaRoomsPlayerCnt(WORD wGameId, WORD wAreaId);

	int PlayerLeaveTable(WORD wGameId, SGamePlayer *pInnerPlayer, CPlayer *pOutterPlayer, BOOL bNotify = TRUE);
public:
	int TryStartGame(SGameTable *pTable);
public:
	static CPlugInGameManager* GetInstance();
	static void FreeInstance();

	CPlugInGame *CreatePluginGameInst(WORD wGameId);
public:
	int RegPlugInGame(_RegPluginGame regInfo);
	void Update(int nTick);
public:
	EPluginGame_PlayerStatus GetPlayerGamesStatus(DWORD dwPlayerId);
	BOOL SetPlayerGameStatus(DWORD dwPlayerId, EPluginGame_PlayerStatus status);
	int ForceQuit(DWORD dwPlayerId);                                                    // 强制退出游戏
public:
	int RecMsg(DNID dnidClient, SPluginGameBaseMsg *pMsg);
	void SendSysMsgTip(CPlayer *pOutterPlayer, char *lpszMsg, ...);

private:
	int LoadGameAreaConfig(string strFile);
	//int LoadGameAreaConfigEx(string strFile, OUT SRegAreaParm& areaParms[PLUGINGAME_MAX_GAME_AREA_CNT]);
public:
	int RegPlugInGameFromFile(string strFile);
    void SendPlugInGameErr( DNID dnidClient, int nGameErrType );

    BOOL m_bOpen;
};


#endif
