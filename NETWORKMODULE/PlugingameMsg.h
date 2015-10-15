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
#define PLUGINGAME_MAX_ONE_GAME_PLAYER_CNT	8		//���8�˲μӵ���Ϸ
#define PLUGINGAME_MAX_PLAYER_CNT		MAXWORD

#define PLUGINGAME_MAX_GAME_CNT			128
#define PLUGINGAME_MAX_GAME_AREA_CNT	128
#define PLUGINGAME_MAX_GAME_ROOM_CNT	512
#define PLUGINGAME_MAX_GAME_TABLE_CNT	512


//����10���
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
	MSG_PLUGINGAME_CS_ENTER_HALL,				//�������
	MSG_PLUGINGAME_SC_GAME_LIST,				//��Ϸ�б�

	MSG_PLUGINGAME_CS_SELECT_GAME,				//ѡ����Ϸ
	MSG_PLUGINGAME_SC_GAME_AREA_LIST,			//��Ϸ'����'�б� ���������� vip��...��

	MSG_PLUGINGAME_CS_SELECT_GAME_AREA,			//ѡ��������Ϸ����
	MSG_PLUGINGAME_SC_GAME_ROOM_LIST,			//������Ϸ�����б�

	MSG_PLUGINGAME_CS_SELECT_ROOM,				//ѡ�񷿼�
	MSG_PLUGINGAME_SC_TABLE_LIST,				//������Ϸ�����б�

	MSG_PLUGINGAME_CS_SELECT_TABLE,				//ѡ������(��λ)
	MSG_PLUGINGAME_CS_LEAVE_TABLE,				//�뿪��λ	
	MSG_PLUGINGAME_SC_SYNC_TABLE_STATUS,		//ͬ����λ��Ϣ

	MSG_PLUGINGAME_CS_READY,					//׼��
    	
	MSG_PLUGINGAME_SC_START,					//��Ϸ��ʼ
	MSG_PLUGINGAME_SC_FINISH,					//��Ϸ����(ʤ���ߣ� ���𡣡���)

	MSG_PLUGINGAME_SC_SYNC_AREA_PLAYER_NUM,		//ͬ�������������
	MSG_PLUGINGAME_SC_SYNC_ROOM_PLAYER_NUM,		//ͬ�������������
	//MSG_PLUGINGAME_FORCE_TO_QUIT,				//ǿ���˳���Ϸ
	//MSG_PLUGINGAME_PLAYER_QUIT_GAME,			//����Ѿ��˳�

	MSG_PLUGINGAME_SC_SYNC_PLAYER_INFO,			//ͬ�������Ϣ��
	MSG_PLUGINGAME_GAME_LOGIC,					//������Ϸ�߼�

	MSG_PLUGINGAME_CS_CHAT_PUBLIC,						//����
	MSG_PLUGINGAME_SC_CHAT_PUBLIC,						//����

	MSG_PLUGINGAME_CS_CHAT_PRIVATE,					//˽��
	MSG_PLUGINGAME_SC_CHAT_PRIVATE,					//˽��

	MSG_PLUGINGAME_SC_CHAT_SYS_MSG,					//ϵͳ��Ϣ

	MSG_PLUGINGAME_CS_EXIT_HALL,					//�뿪����

    MSG_PLUGINGAME_SC_ERR                           // ʧ��
END_MSG_MAP()




//MSG_PLUGINGAME_CS_ENTER_HALL,				//�������
DECLARE_MSG(_cs_PlugInGame_EnterHall, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_ENTER_HALL)
struct cs_PlugInGame_EnterHall : public _cs_PlugInGame_EnterHall
{

};

//MSG_PLUGINGAME_CS_EXIT_HALL						//�뿪����
DECLARE_MSG(_cs_PlugInGame_ExitHall, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_EXIT_HALL)
struct cs_PlugInGame_ExitHall : public _cs_PlugInGame_ExitHall
{

};

//MSG_PLUGINGAME_SC_GAME_LIST,				//��Ϸ�б�
DECLARE_MSG(_sc_PlugInGame_GameList, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_GAME_LIST)
struct sc_PlugInGame_GameList : public _sc_PlugInGame_GameList
{
	char strGameName[PLUGINGAME_MAX_GAME_NAME_LEN];
	DWORD dwGameId;
};

//MSG_PLUGINGAME_CS_SELECT_GAME,				//ѡ����Ϸ
DECLARE_MSG(_cs_PlugInGame_SelGame, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_SELECT_GAME)
struct cs_PlugInGame_SelGame : public _cs_PlugInGame_SelGame
{
	DWORD dwGameId;
};

//MSG_PLUGINGAME_SC_GAME_AREA_LIST,			//��Ϸ'����'�б� ���������� vip��...��
DECLARE_MSG(_cs_PlugInGame_AreaList, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_GAME_AREA_LIST)
struct cs_PlugInGame_AreaList : public _cs_PlugInGame_AreaList
{
	WORD wGameId;
	WORD wAreaId;	
	WORD wCurtPlayerCnt;
	WORD wMaxPlayerCnt;
	char strAreaName[PLUGINGAME_MAX_GAME_NAME_LEN];

	//�볡�׽�
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

//MSG_PLUGINGAME_CS_SELECT_GAME_AREA,			//ѡ��������Ϸ����
DECLARE_MSG(_cs_PlugInGame_SelArea, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_SELECT_GAME_AREA)
struct cs_PlugInGame_SelArea : public _cs_PlugInGame_SelArea
{
	WORD wGameId;
	WORD wAreaId;	
};

//MSG_PLUGINGAME_SC_GAME_ROOM_LIST,			//������Ϸ�����б�
DECLARE_MSG(_sc_PlugInGame_RoomList, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_GAME_ROOM_LIST)
struct sc_PlugInGame_RoomList : public _sc_PlugInGame_RoomList
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;

	WORD wCurtPlayerCnt;
	WORD wMaxPlayerCnt;

	//todo: ����
	BYTE byFirstTableStatus;	//����״̬		//0:�ȴ� 1���Ѿ���ʼ�ˡ�
};

//MSG_PLUGINGAME_CS_SELECT_ROOM,				//ѡ�񷿼�
DECLARE_MSG(_cs_PlugInGame_SelectRoom, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_SELECT_ROOM)
struct cs_PlugInGame_SelectRoom : public _cs_PlugInGame_SelectRoom
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;
};

//MSG_PLUGINGAME_SC_TABLE_LIST,				//������Ϸ�����б�
DECLARE_MSG(_sc_PlugInGame_TableList, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_TABLE_LIST)
struct sc_PlugInGame_TableList : public _sc_PlugInGame_TableList
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;

	WORD wTableCnt;
};

//MSG_PLUGINGAME_CS_SELECT_TABLE,				//ѡ������(��λ)
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

//MSG_PLUGINGAME_CS_LEAVE_TABLE,				//�뿪��λ	
DECLARE_MSG(_cs_PlugInGame_LeaveTable, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_LEAVE_TABLE)
struct cs_PlugInGame_LeaveTable : public _cs_PlugInGame_LeaveTable
{	
	WORD wGameId;
};

//MSG_PLUGINGAME_SC_SYNC_TABLE_STATUS,		//ͬ����λ��Ϣ
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
		__tsPlayerLeft = 2			//����뿪��λ
	};

	BYTE byStatus;
	BYTE byMode;		//Parm.
};


//MSG_PLUGINGAME_CS_READY,					//׼��
DECLARE_MSG(_cs_PlugInGame_PlayerReady, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_CS_READY)
struct cs_PlugInGame_PlayerReady : public _cs_PlugInGame_PlayerReady
{
	WORD wGameId;
};

//MSG_PLUGINGAME_SC_SYNC_AREA_PLAYER_NUM,			//ͬ�������������
DECLARE_MSG(_sc_PlugInGame_SyncAreaPlayerCnt, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_SYNC_AREA_PLAYER_NUM)
struct sc_PlugInGame_SyncAreaPlayerCnt : public _sc_PlugInGame_SyncAreaPlayerCnt
{
	WORD wGameId;
	WORD wAreaId;
	WORD wPlayerCnt;
};

//MSG_PLUGINGAME_SC_SYNC_ROOM_PLAYER_NUM,			//ͬ�������������
DECLARE_MSG(_sc_PlugInGame_SyncRoomPlayerCnt, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_SYNC_ROOM_PLAYER_NUM)
struct sc_PlugInGame_SyncRoomPlayerCnt : public _sc_PlugInGame_SyncRoomPlayerCnt
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;
	WORD wPlayerCnt;
};

//MSG_PLUGINGAME_SC_START,					//��Ϸ��ʼ
DECLARE_MSG(_sc_PlugInGame_GameStart, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_SC_START)
struct sc_PlugInGame_GameStart : public _sc_PlugInGame_GameStart
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;
	WORD wTableId;

	//��ͬ��Ϸ�Ĳ�ͬ������ �䳤�� 
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


//MSG_PLUGINGAME_SC_FINISH,					//��Ϸ����(ʤ���ߣ� ���𡣡���)
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

//MSG_PLUGINGAME_CS_CHAT_PUBLIC,						//����
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

//MSG_PLUGINGAME_SC_CHAT_PUBLIC,						//����
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



//MSG_PLUGINGAME_CS_CHAT_PRIVATE,					//˽����
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

//MSG_PLUGINGAME_SC_CHAT_PRIVATE,					//˽����
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

//MSG_PLUGINGAME_SC_CHAT_SYS_MSG,					//ϵͳ��Ϣ
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



//MSG_PLUGINGAME_SC_GAME_LOGIC				//������Ϸ�߼�sc
DECLARE_MSG(_PlugInGame_GameLogic, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_GAME_LOGIC)
struct PlugInGame_GameLogic : public _PlugInGame_GameLogic
{
	WORD wGameId;
	WORD wAreaId;
	WORD wRoomId;
	WORD wTableId;
};


//MSG_PLUGINGAME_SC_SYNC_PLAYER_INFO,			//ͬ�������Ϣ��
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
//////////////////////////////////////����10��롣
//////////////////////////////////////����10��롣
//////////////////////////////////////����10��롣
//////////////////////////////////////����10��롣
//////////////////////////////////////����10��롣

DECLARE_MSG_MAP(SPluginGame_HappyTenPointFive_BaseMsg, SPluginGameBaseMsg, SPluginGameBaseMsg::MSG_PLUGINGAME_GAME_LOGIC)
	MSG_SC_PLAYER_S_TRUN,				//��ѯ�Ƿ�Ҫ��
	MSG_CS_COMMIT_CHOICE,				//�ύѡ����
	MSG_SC_PLAYER_S_CHOICE,				//�㲥���ѡ����
	MSG_SC_ONE_CARD,					//ϵͳ����
	//MSG_SC_WAIT_CHOICE_TIME_OUT,		//�ȴ����ѡ��ʱ��
	MSG_SC_PLAYER_S_CRAD_DETAILS,		//��Ϸ����ʱ��ÿ���˵�������ϸ��Ϣ

	MSG_SC_GAME_STATUS,					//��Ϸ�ֳ����������;����ʱ��
END_MSG_MAP()


///	MSG_SC_PLAYER_S_TRUN,				//��ѯ�Ƿ�Ҫ��
DECLARE_PLUGINGAME_SUB_MSG(_sc_HappyTenPointFive_PlayerSTurn, PlugInGame_GameLogic, SPluginGame_HappyTenPointFive_BaseMsg, SPluginGame_HappyTenPointFive_BaseMsg::MSG_SC_PLAYER_S_TRUN)
struct sc_HappyTenPointFive_PlayerSTurn : public _sc_HappyTenPointFive_PlayerSTurn
{
	BYTE bySeatId;	//��λ���
};

//MSG_CS_COMMIT_CHOICE,				//�ύѡ����
DECLARE_PLUGINGAME_SUB_MSG(_cs_HappyTenPointFive_CommitChoice, PlugInGame_GameLogic, SPluginGame_HappyTenPointFive_BaseMsg, SPluginGame_HappyTenPointFive_BaseMsg::MSG_CS_COMMIT_CHOICE)
struct cs_HappyTenPointFive_CommitChoice : public _cs_HappyTenPointFive_CommitChoice
{	
	BYTE byChoice;	//0: pass, 1:yes.
};

//MSG_SC_PLAYER_S_CHOICE,				//�㲥���ѡ����
DECLARE_PLUGINGAME_SUB_MSG(_sc_HappyTenPointFive_PlayerChoice, PlugInGame_GameLogic, SPluginGame_HappyTenPointFive_BaseMsg, SPluginGame_HappyTenPointFive_BaseMsg::MSG_SC_PLAYER_S_CHOICE)
struct sc_HappyTenPointFive_PlayerChoice : public _sc_HappyTenPointFive_PlayerChoice
{	
	BYTE bySeatId;	//��λ���
	BYTE byChice;	//0: pass, 1:yes.
	BYTE byReason;	//���pass��ָ��ԭ��	��ʱ����
};

//MSG_SC_ONE_CARD,					//ϵͳ����
DECLARE_PLUGINGAME_SUB_MSG(_sc_HappyTenPointFive_OneCard, PlugInGame_GameLogic, SPluginGame_HappyTenPointFive_BaseMsg, SPluginGame_HappyTenPointFive_BaseMsg::MSG_SC_ONE_CARD)
struct sc_HappyTenPointFive_OneCard : public _sc_HappyTenPointFive_OneCard
{
	BYTE byCardId; //1-54;
};

////MSG_SC_WAIT_CHOICE_TIME_OUT,		//�ȴ����ѡ��ʱ��
//DECLARE_PLUGINGAME_SUB_MSG(_sc_HappyTenPointFive_WaitChoiceTimeOut, PlugInGame_GameLogic, SPluginGame_HappyTenPointFive_BaseMsg, SPluginGame_HappyTenPointFive_BaseMsg::MSG_SC_WAIT_CHOICE_TIME_OUT)
//struct sc_HappyTenPointFive_WaitChoiceTimeOut : public _sc_HappyTenPointFive_WaitChoiceTimeOut
//{
//	BYTE byParm; //useless, yet.
//};

//MSG_SC_PLAYER_S_CRAD_DETAILS,		//��Ϸ����ʱ��ÿ���˵�������ϸ��Ϣ
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

//MSG_SC_GAME_STATUS,					//��Ϸ�ֳ����������;����ʱ��
DECLARE_PLUGINGAME_SUB_MSG(_sc_HappyTenPointFive_GameStatus, PlugInGame_GameLogic, SPluginGame_HappyTenPointFive_BaseMsg, SPluginGame_HappyTenPointFive_BaseMsg::MSG_SC_GAME_STATUS)
struct sc_HappyTenPointFive_GameStatus : public _sc_HappyTenPointFive_GameStatus
{
	DWORD dwAllBaseMoney;		//Ŀǰ���л�������
	DWORD dwAllLotteryMoney;	//�������вʽ�
	BYTE byCurtPlayer;		//�Ǹ���λid

	BYTE byPlayersCnt;		//�������˳������
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