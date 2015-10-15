/*
	File: Plugin mini games.
	kevin @ 09.09.16
*/
#ifndef _PLUGIN_GAME_MSG_H_
#define _PLUGIN_GAME_MSG_H_

#include "NetModule.h"
#include "../pub/ConstValue.h"
//#include "GameBaseTypedef.h"
#include <assert.h>

#define PLUGINGAME_GAME_INDEX_HAPPY10_5	1

#define PLUGINGAME_CHAT_MAX_LEN			255
#define PLUGINGAME_MAX_GAME_NAME_LEN	32
#define PLUGINGAME_MAX_ONE_GAME_PLAYER_CNT	8		//最多8人参加的游戏
#define PLUGINGAME_MAX_PLAYER_CNT		MAXWORD

#define PLUGINGAME_MAX_GAME_CNT			128
#define PLUGINGAME_MAX_GAME_AREA_CNT	128
#define PLUGINGAME_MAX_GAME_ROOM_CNT	512
#define PLUGINGAME_MAX_GAME_TABLE_CNT	512


//欢乐10点半
#define _PLUGINGAME_HAPPY10_5_MAX_PLAYER_CNT_			7
#define _PLUGINGAME_HAPPY10_5_MAX_TURN_CNT_				7

#pragma pack(1)
struct __SPosInfo
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;
	WORD wTableId;
};

#define DECLARE_PLUGINGAME_SUB_MSG(name, firstStruct, BaseMessage, MessageType) struct name : public BaseMessage { __SPosInfo posInfo; name() {BaseMessage::_protocol = MessageType; } };



///////////////////////////////////////////////////////////////////////////
DECLARE_MSG_MAP(SPluginGameBaseMsg, SMessage, SMessage::EPRO_PLUGINGAME_MESSAGE)
	MSG_PLUGINGAME_CS_ENTER_HALL,				//进入大厅
	MSG_PLUGINGAME_SC_GAME_LIST,				//游戏列表

	MSG_PLUGINGAME_CS_SELECT_GAME,				//选择游戏
	MSG_PLUGINGAME_SC_GAME_AREA_LIST,			//游戏'区域'列表 （新手区， vip区...）

	MSG_PLUGINGAME_CS_SELECT_GAME_AREA,			//选择意向游戏分区
	MSG_PLUGINGAME_SC_GAME_ROOM_LIST,			//分区游戏房间列表

	MSG_PLUGINGAME_CS_SELECT_ROOM,				//选择房间
	MSG_PLUGINGAME_SC_TABLE_LIST,				//房间游戏桌子列表

	MSG_PLUGINGAME_CS_SELECT_TABLE,				//选择桌子(座位)
	MSG_PLUGINGAME_CS_LEAVE_TABLE,				//离开座位	
	MSG_PLUGINGAME_SC_SYNC_TABLE_STATUS,		//同步座位信息

	MSG_PLUGINGAME_CS_READY,					//准备
    	
	MSG_PLUGINGAME_SC_START,					//游戏开始
	MSG_PLUGINGAME_SC_FINISH,					//游戏结束(胜利者， 奖金。。。)

	MSG_PLUGINGAME_SC_SYNC_AREA_PLAYER_NUM,		//同步区域玩家人数
	MSG_PLUGINGAME_SC_SYNC_ROOM_PLAYER_NUM,		//同步房间玩家人数
	//MSG_PLUGINGAME_FORCE_TO_QUIT,				//强行退出游戏
	//MSG_PLUGINGAME_PLAYER_QUIT_GAME,			//玩家已经退出

	MSG_PLUGINGAME_SC_SYNC_PLAYER_INFO,			//同步玩家信息，
	MSG_PLUGINGAME_GAME_LOGIC,					//具体游戏逻辑

	MSG_PLUGINGAME_CS_CHAT_PUBLIC,						//聊天
	MSG_PLUGINGAME_SC_CHAT_PUBLIC,						//聊天

	MSG_PLUGINGAME_CS_CHAT_PRIVATE,					//私聊
	MSG_PLUGINGAME_SC_CHAT_PRIVATE,					//私聊

	MSG_PLUGINGAME_SC_CHAT_SYS_MSG,					//系统信息

	MSG_PLUGINGAME_CS_EXIT_HALL,					//离开大厅

    MSG_PLUGINGAME_SC_ERR                           // 失败
END_MSG_MAP()




//MSG_PLUGINGAME_CS_ENTER_HALL,				//进入大厅
DECLARE_MSG(_cs_PlugInGame_EnterHall, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_ENTER_HALL)
struct cs_PlugInGame_EnterHall : public _cs_PlugInGame_EnterHall
{

};

//MSG_PLUGINGAME_CS_EXIT_HALL						//离开大厅
DECLARE_MSG(_cs_PlugInGame_ExitHall, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_EXIT_HALL)
struct cs_PlugInGame_ExitHall : public _cs_PlugInGame_ExitHall
{

};

//MSG_PLUGINGAME_SC_GAME_LIST,				//游戏列表
DECLARE_MSG(_sc_PlugInGame_GameList, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_GAME_LIST)
struct sc_PlugInGame_GameList : public _sc_PlugInGame_GameList
{
	char strGameName[PLUGINGAME_MAX_GAME_NAME_LEN];
	DWORD dwGameId;
};

//MSG_PLUGINGAME_CS_SELECT_GAME,				//选择游戏
DECLARE_MSG(_cs_PlugInGame_SelGame, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_SELECT_GAME)
struct cs_PlugInGame_SelGame : public _cs_PlugInGame_SelGame
{
	DWORD dwGameId;
};

//MSG_PLUGINGAME_SC_GAME_AREA_LIST,			//游戏'区域'列表 （新手区， vip区...）
DECLARE_MSG(_cs_PlugInGame_AreaList, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_GAME_AREA_LIST)
struct cs_PlugInGame_AreaList : public _cs_PlugInGame_AreaList
{
	WORD wGameId;
	WORD wAreaId;	
	WORD wCurtPlayerCnt;
	WORD wMaxPlayerCnt;
	char strAreaName[PLUGINGAME_MAX_GAME_NAME_LEN];

	//入场底金
	BYTE byMoneyType;
	DWORD dwMoneyCnt;
	DWORD dwLotteryMoney;
    DWORD dwBaseTaxMoney;
    WORD wTaxRate;

	cs_PlugInGame_AreaList()
	{
		strcpy(strAreaName, "no name");
		byMoneyType = 0/*MoneyType_Gold*/;
		dwMoneyCnt = 0;
        dwLotteryMoney = 0;
        dwBaseTaxMoney = 0;
        wTaxRate = 0;
	}
};

//MSG_PLUGINGAME_CS_SELECT_GAME_AREA,			//选择意向游戏分区
DECLARE_MSG(_cs_PlugInGame_SelArea, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_SELECT_GAME_AREA)
struct cs_PlugInGame_SelArea : public _cs_PlugInGame_SelArea
{
	WORD wGameId;
	WORD wAreaId;	
};

//MSG_PLUGINGAME_SC_GAME_ROOM_LIST,			//分区游戏房间列表
DECLARE_MSG(_sc_PlugInGame_RoomList, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_GAME_ROOM_LIST)
struct sc_PlugInGame_RoomList : public _sc_PlugInGame_RoomList
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;

	WORD wCurtPlayerCnt;
	WORD wMaxPlayerCnt;

	//todo: 暂用
	BYTE byFirstTableStatus;	//桌子状态		//0:等待 1：已经开始了。
};

//MSG_PLUGINGAME_CS_SELECT_ROOM,				//选择房间
DECLARE_MSG(_cs_PlugInGame_SelectRoom, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_SELECT_ROOM)
struct cs_PlugInGame_SelectRoom : public _cs_PlugInGame_SelectRoom
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;
};

//MSG_PLUGINGAME_SC_TABLE_LIST,				//房间游戏桌子列表
DECLARE_MSG(_sc_PlugInGame_TableList, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_TABLE_LIST)
struct sc_PlugInGame_TableList : public _sc_PlugInGame_TableList
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;

	WORD wTableCnt;
};

//MSG_PLUGINGAME_CS_SELECT_TABLE,				//选择桌子(座位)
DECLARE_MSG(_cs_PlugInGame_SelTable, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_SELECT_TABLE)
struct cs_PlugInGame_SelTable : public _cs_PlugInGame_SelTable
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;
	WORD wTableId;

	BYTE byAutoSel;
	BYTE bySeatId;
};

//MSG_PLUGINGAME_CS_LEAVE_TABLE,				//离开座位	
DECLARE_MSG(_cs_PlugInGame_LeaveTable, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_LEAVE_TABLE)
struct cs_PlugInGame_LeaveTable : public _cs_PlugInGame_LeaveTable
{	
	WORD wGameId;
};

//MSG_PLUGINGAME_SC_SYNC_TABLE_STATUS,		//同步座位信息
DECLARE_MSG(_sc_PlugInGame_SyncTable, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_SYNC_TABLE_STATUS)
struct sc_PlugInGame_SyncTable : public _sc_PlugInGame_SyncTable
{
	//where
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;
	WORD wTableId;
	BYTE bySeatId;

	//who
	DWORD dwPlayerId;
	char strPlayerName[CONST_USERNAME];

	//do what?
	enum EnDoWhat
	{
		__tsSitDownORReadying = 0,
		__tsWaitting4Start = 1,
		__tsPlayerLeft = 2			//玩家离开座位
	};

	BYTE byStatus;
	BYTE byMode;		//Parm.
};


//MSG_PLUGINGAME_CS_READY,					//准备
DECLARE_MSG(_cs_PlugInGame_PlayerReady, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_READY)
struct cs_PlugInGame_PlayerReady : public _cs_PlugInGame_PlayerReady
{
	WORD wGameId;
};

//MSG_PLUGINGAME_SC_SYNC_AREA_PLAYER_NUM,			//同步区域玩家人数
DECLARE_MSG(_sc_PlugInGame_SyncAreaPlayerCnt, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_SYNC_AREA_PLAYER_NUM)
struct sc_PlugInGame_SyncAreaPlayerCnt : public _sc_PlugInGame_SyncAreaPlayerCnt
{
	WORD wGameId;
	WORD wAreaId;
	WORD wPlayerCnt;
};

//MSG_PLUGINGAME_SC_SYNC_ROOM_PLAYER_NUM,			//同步房间玩家人数
DECLARE_MSG(_sc_PlugInGame_SyncRoomPlayerCnt, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_SYNC_ROOM_PLAYER_NUM)
struct sc_PlugInGame_SyncRoomPlayerCnt : public _sc_PlugInGame_SyncRoomPlayerCnt
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;
	WORD wPlayerCnt;
};

//MSG_PLUGINGAME_SC_START,					//游戏开始
DECLARE_MSG(_sc_PlugInGame_GameStart, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_START)
struct sc_PlugInGame_GameStart : public _sc_PlugInGame_GameStart
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;
	WORD wTableId;

	//不同游戏的不同参数， 变长包 
	enum
	{
		MAX_PARM_CNT = 10
	};
	BYTE byParmCnt;
	DWORD arrParms[MAX_PARM_CNT];

	WORD CalcSize(){ return ( WORD )( sizeof(sc_PlugInGame_GameStart) - sizeof (DWORD) * (MAX_PARM_CNT - byParmCnt) ); };

	sc_PlugInGame_GameStart()
	{
		byParmCnt = 0;
		memset(arrParms, 0, sizeof(arrParms) );
	}
};


//MSG_PLUGINGAME_SC_FINISH,					//游戏结束(胜利者， 奖金。。。)
DECLARE_MSG(_sc_PlugInGame_GameFinish, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_FINISH)
struct sc_PlugInGame_GameFinish : public _sc_PlugInGame_GameFinish
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;
	WORD wTableId;

	BYTE byWinnerCnt;
	BYTE arrWinnerSeatId[MSG_PLUGINGAME_SC_FINISH];

	WORD GetSize()
	{
		return ( WORD )( sizeof(sc_PlugInGame_GameFinish ) - (MSG_PLUGINGAME_SC_FINISH - byWinnerCnt) * sizeof BYTE );
	}
};

//MSG_PLUGINGAME_CS_CHAT_PUBLIC,						//聊天
DECLARE_MSG(_cs_PlugInGame_Chat, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_CHAT_PUBLIC)
struct cs_PlugInGame_Chat : public _cs_PlugInGame_Chat
{
	BYTE byLen;
	char strMsg[PLUGINGAME_CHAT_MAX_LEN];	

	WORD GetSize()
	{
		return sizeof(cs_PlugInGame_Chat) - (PLUGINGAME_CHAT_MAX_LEN - byLen) * sizeof (char);
	}
};

//MSG_PLUGINGAME_SC_CHAT_PUBLIC,						//聊天
DECLARE_MSG(_sc_PlugInGame_Chat, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_CHAT_PUBLIC)
struct sc_PlugInGame_Chat : public _sc_PlugInGame_Chat
{
	WORD wSeatId;
	BYTE byLen;
	char strMsg[PLUGINGAME_CHAT_MAX_LEN];	

	WORD GetSize()
	{
		return sizeof(sc_PlugInGame_Chat) - (PLUGINGAME_CHAT_MAX_LEN - byLen) * sizeof (char);
	}
};



//MSG_PLUGINGAME_CS_CHAT_PRIVATE,					//私聊天
DECLARE_MSG(_cs_PlugInGame_Chat_Private, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_CHAT_PRIVATE)
struct cs_PlugInGame_Chat_Private : public _cs_PlugInGame_Chat_Private
{
	BYTE byTargetSeatId;
	BYTE byLen;
	char strMsg[PLUGINGAME_CHAT_MAX_LEN];	

	WORD GetSize()
	{
		return sizeof(cs_PlugInGame_Chat_Private) - (PLUGINGAME_CHAT_MAX_LEN - byLen) * sizeof (char);
	}
};

//MSG_PLUGINGAME_SC_CHAT_PRIVATE,					//私聊天
DECLARE_MSG(_sc_PlugInGame_Chat_Private, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_CHAT_PRIVATE)
struct sc_PlugInGame_Chat_Private : public _sc_PlugInGame_Chat_Private
{
	BYTE bySrcSeatId;
	BYTE byLen;
	char strMsg[PLUGINGAME_CHAT_MAX_LEN];	

	WORD GetSize()
	{
		return sizeof(sc_PlugInGame_Chat_Private) - (PLUGINGAME_CHAT_MAX_LEN - byLen) * sizeof (char);
	}
};

//MSG_PLUGINGAME_SC_CHAT_SYS_MSG,					//系统信息
DECLARE_MSG(_sc_PlugInGame_Chat_SysMsg, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_CHAT_SYS_MSG)
struct sc_PlugInGame_Chat_SysMsg : public _sc_PlugInGame_Chat_SysMsg
{	
	BYTE byLen;
	char strMsg[PLUGINGAME_CHAT_MAX_LEN];	
	WORD GetSize()
	{
		return sizeof(sc_PlugInGame_Chat_SysMsg) - (PLUGINGAME_CHAT_MAX_LEN - byLen) * sizeof (char);
	}
	

	sc_PlugInGame_Chat_SysMsg()
	{
		memset(strMsg, 0, PLUGINGAME_CHAT_MAX_LEN);
	}
};



//MSG_PLUGINGAME_SC_GAME_LOGIC				//具体游戏逻辑sc
DECLARE_MSG(_PlugInGame_GameLogic, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_GAME_LOGIC)
struct PlugInGame_GameLogic : public _PlugInGame_GameLogic
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;
	WORD wTableId;
};


//MSG_PLUGINGAME_SC_SYNC_PLAYER_INFO,			//同步玩家信息，
DECLARE_MSG(_sc_PlugInGame_SyncPlayerInfo, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_SYNC_PLAYER_INFO)
struct sc_PlugInGame_SyncPlayerInfo : public _sc_PlugInGame_SyncPlayerInfo
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;
	WORD wTableId;
	WORD wSeatId;

	//
	BYTE bySex;		//
	BYTE bySchool;	//
};


//////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////欢乐10点半。
//////////////////////////////////////欢乐10点半。
//////////////////////////////////////欢乐10点半。
//////////////////////////////////////欢乐10点半。
//////////////////////////////////////欢乐10点半。

DECLARE_MSG_MAP(SPluginGame_HappyTenPointFive_BaseMsg, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_GAME_LOGIC)
	MSG_SC_PLAYER_S_TRUN,				//轮询是否要牌
	MSG_CS_COMMIT_CHOICE,				//提交选择结果
	MSG_SC_PLAYER_S_CHOICE,				//广播玩家选择结果
	MSG_SC_ONE_CARD,					//系统发牌
	//MSG_SC_WAIT_CHOICE_TIME_OUT,		//等待玩家选择超时。
	MSG_SC_PLAYER_S_CRAD_DETAILS,		//游戏结束时，每个人的牌面详细信息

	MSG_SC_GAME_STATUS,					//游戏现场（当玩家中途加入时候）
END_MSG_MAP()


///	MSG_SC_PLAYER_S_TRUN,				//轮询是否要牌
DECLARE_PLUGINGAME_SUB_MSG(_sc_HappyTenPointFive_PlayerSTurn, PlugInGame_GameLogic, SPluginGame_HappyTenPointFive_BaseMsg, SPluginGame_HappyTenPointFive_BaseMsg::MSG_SC_PLAYER_S_TRUN)
struct sc_HappyTenPointFive_PlayerSTurn : public _sc_HappyTenPointFive_PlayerSTurn
{
	BYTE bySeatId;	//座位编号
};

//MSG_CS_COMMIT_CHOICE,				//提交选择结果
DECLARE_PLUGINGAME_SUB_MSG(_cs_HappyTenPointFive_CommitChoice, PlugInGame_GameLogic, SPluginGame_HappyTenPointFive_BaseMsg, SPluginGame_HappyTenPointFive_BaseMsg::MSG_CS_COMMIT_CHOICE)
struct cs_HappyTenPointFive_CommitChoice : public _cs_HappyTenPointFive_CommitChoice
{	
	BYTE byChoice;	//0: pass, 1:yes.
};

//MSG_SC_PLAYER_S_CHOICE,				//广播玩家选择结果
DECLARE_PLUGINGAME_SUB_MSG(_sc_HappyTenPointFive_PlayerChoice, PlugInGame_GameLogic, SPluginGame_HappyTenPointFive_BaseMsg, SPluginGame_HappyTenPointFive_BaseMsg::MSG_SC_PLAYER_S_CHOICE)
struct sc_HappyTenPointFive_PlayerChoice : public _sc_HappyTenPointFive_PlayerChoice
{	
	BYTE bySeatId;	//座位编号
	BYTE byChice;	//0: pass, 1:yes.
	BYTE byReason;	//如果pass则指明原因	暂时不用
};

//MSG_SC_ONE_CARD,					//系统发牌
DECLARE_PLUGINGAME_SUB_MSG(_sc_HappyTenPointFive_OneCard, PlugInGame_GameLogic, SPluginGame_HappyTenPointFive_BaseMsg, SPluginGame_HappyTenPointFive_BaseMsg::MSG_SC_ONE_CARD)
struct sc_HappyTenPointFive_OneCard : public _sc_HappyTenPointFive_OneCard
{
	BYTE byCardId; //1-54;
};

////MSG_SC_WAIT_CHOICE_TIME_OUT,		//等待玩家选择超时。
//DECLARE_PLUGINGAME_SUB_MSG(_sc_HappyTenPointFive_WaitChoiceTimeOut, PlugInGame_GameLogic, SPluginGame_HappyTenPointFive_BaseMsg, SPluginGame_HappyTenPointFive_BaseMsg::MSG_SC_WAIT_CHOICE_TIME_OUT)
//struct sc_HappyTenPointFive_WaitChoiceTimeOut : public _sc_HappyTenPointFive_WaitChoiceTimeOut
//{
//	BYTE byParm; //useless, yet.
//};

//MSG_SC_PLAYER_S_CRAD_DETAILS,		//游戏结束时，每个人的牌面详细信息
DECLARE_PLUGINGAME_SUB_MSG(_sc_HappyTenPointFive_PlayerSCardsDetails, PlugInGame_GameLogic, SPluginGame_HappyTenPointFive_BaseMsg, SPluginGame_HappyTenPointFive_BaseMsg::MSG_SC_PLAYER_S_CRAD_DETAILS)
struct sc_HappyTenPointFive_PlayerSCardsDetails : public _sc_HappyTenPointFive_PlayerSCardsDetails
{
	BYTE bySeatId;
	BYTE byCardCnt;
	BYTE arrCards[7];

	WORD GetSize()
	{
		byCardCnt = byCardCnt > 7 ? 7 : byCardCnt;
		return sizeof(sc_HappyTenPointFive_PlayerSCardsDetails) - (7 - byCardCnt) * sizeof BYTE;
	}
};

//MSG_SC_GAME_STATUS,					//游戏现场（当玩家中途加入时候）
DECLARE_PLUGINGAME_SUB_MSG(_sc_HappyTenPointFive_GameStatus, PlugInGame_GameLogic, SPluginGame_HappyTenPointFive_BaseMsg, SPluginGame_HappyTenPointFive_BaseMsg::MSG_SC_GAME_STATUS)
struct sc_HappyTenPointFive_GameStatus : public _sc_HappyTenPointFive_GameStatus
{
	DWORD dwAllBaseMoney;		//目前所有基本赌资
	DWORD dwAllLotteryMoney;	//本局所有彩金
	BYTE byCurtPlayer;		//是个座位id

	BYTE byPlayersCnt;		//不包含退出的玩家
	struct _SPlayerInfo
	{
		BYTE bySeatId;
		BYTE byCardCnt;
	}arrPlayerInfo[_PLUGINGAME_HAPPY10_5_MAX_TURN_CNT_];

	WORD GetSize()
	{
		assert(byPlayersCnt <= _PLUGINGAME_HAPPY10_5_MAX_TURN_CNT_);
		return sizeof(sc_HappyTenPointFive_GameStatus) - sizeof(_SPlayerInfo) * (_PLUGINGAME_HAPPY10_5_MAX_TURN_CNT_ - byPlayersCnt);
	}
};

//MSG_PLUGINGAME_SC_ERR
DECLARE_MSG(_sc_PlugInGame_GameErr, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_ERR)
struct sc_PlugInGame_GameErr : public _sc_PlugInGame_GameErr
{
    int wGameErrType;
};

#pragma pack()
#endif