#pragma once
#include <list>
#include ".\mysqldbprocess.h"
#include "NetWorkModule\DataMsgs.h"
#include "NetWorkModule\MailMsg.h"
#include "NetWorkModule\FactionBBSMsgs.h"
#include "NetWorkModule\logtypedef.h"
#include "NetWorkModule\BuildingTypedef.h"
#include "NetWorkModule\unionmsgs.h"
#include "NetWorkModule\MailMsg_SanGuo.h"



class CNetDBModuleEx :
    public mysql::Query
{	

public:
	int GetPlayerCharList( LPCSTR pAccount, DWORD dwServerID, SCharListData * pCharListData );
	int CreatePlayerData( LPCSTR pAccount,WORD wIndex, DWORD dwServerID, SFixData &PlayerData );
	int GetPlayerData(LPCSTR pAccount, DWORD dwServerID, BYTE byIndex, SFixData &PlayerData);
	int SavePlayerData(LPCSTR pAccount, DWORD dwServerID, SFixData &bdata, SWareHouses *wdata);
	int DeletePlayerData( DWORD dwRoleID, LPCSTR pAccount, DWORD dwServerID );
	int UpdateAccountLock( LPCSTR pAccount, DWORD dwServerID ,BOOL block );
	int RpcProcedure( DNID srcDnid, LPVOID rpcBuffer, DNID passDnid );
	int SavePlayerConfig( LPCSTR pAccount, DWORD dwServerID, LPCSTR PlayerConfig );
	int GetAllPlayerRanklist(DWORD level,long &num,NewRankList * pNewRanklist);
	struct WriteInfo
	{
		char accountname[46];
		char rolename[CONST_USERNAME];
		SCharListData rolebuf;
		SFixPlayerDataBuf databuf;
	};
	typedef QueryBind< 5 > WriteQuery;
	typedef std::map<unsigned long, WriteInfo> WriteInfoVec;
	WriteInfoVec m_WriteInfoVec;
	int LoadAllPlayerData( void );

public:
    int SaveFactions( struct SQSaveFactionMsg *pMsg );
	int SaveFactionsData(struct SQSaveFactionDataMsg *pMsg);


   // int SaveFaction( SaveFactionData &stFaction, DWORD serverId, DWORD tick );
	int SaveFaction(SaveFactionData_New &stFaction, DWORD serverId, DWORD tick);
	int SaveFaction(SaveFactionData_Lua &stluaFaction, DWORD serverId, DWORD tick);

	int DeleteFaction(const DNID dwSID,struct SQDeleteFaction *pMsg);

	int SaveScriptData(struct SQScriptData *pMsg);
	int GetScriptData(const DNID dwSID,struct SQGetScriptData *pMsg);


    int GetFactions( const DNID dwSID, const __int32 nServerID );
	int GetFactions_New(const DNID dwSID, const __int32 nServerID);
	int GetFactions_Data(const DNID dwSID, const __int32 nServerID);


public:
    int SaveBuildings( struct SQBuildingSaveMsg *pMsg );
    int GetBuildings( const DNID dwSID, struct SQBuildingGetMsg *pMsg );

public:
   int GetUnionData( const DNID dwSID, SQGetUnionDataMsg * pMsg );
   int SaveUnionDatas( SQSaveUnionDataMsg *pMsg );
   int SaveUnionData( SPlayerUnionBase *p, DWORD tick );

private :
	int CreateName( LPCSTR pAccount,WORD wIndex,DWORD dwServerID, LPCSTR pRoleName );
	//int GetWareHouseData( LPCSTR pAccount, LPCSTR name, DWORD dwServerID, SFixProperty &data );
	//int SaveWareHouseData( LPCSTR pAccount,LPCSTR name, DWORD dwServerID, bool blocked, SWareHouses &data );
	//int CheckAccountLock( LPCSTR pAccount, DWORD dwServerID );
	//int InsertWareHouse( LPCSTR pAccount );

	//三国 2015.1.12 wk 邮件系统 存储过程调用##begin######
public:
	int SendMail_sys(S2D_SendMailSYS_MSG*pBaseMsg);
	int SendMail(struct S2D_SendMail_MSG*pBaseMsg);
	int GetMailInfo_DB(const DNID clientDnid, SQ_GetMailInfo_MSG* pMsg);
	int GetMailAwards_DB(const DNID clientDnid,struct SQ_GetAwards_MSG* pMsg);
	int SetMailState_DB(struct SQ_SetMailState_MSG* pMsg);
	
	//三国 2015.1.12 wk 邮件系统 存储过程调用##end######
	int WritePointLog(SQPointLogMsg* pMsg);
	int test(string src);
};
