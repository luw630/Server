#pragma once

#include "player.h"

#include "networkmodule/tongmsgs.h"
#include "networkmodule/refreshmsgs.h"
#include "networkmodule/datamsgs.h"

#include <list>
using namespace std;

class CFactionManager : public CSingleton<CFactionManager>
{
public:
	CFactionManager(void);
	~CFactionManager(void);

	BOOL CreateFaction(LPCSTR szFactionName, CPlayer* pPlayer, DWORD factionId, SQCreatetchTong *pMsg);
	BOOL DeleteFaction(LPCSTR szFactionName, CPlayer	*pPlayer);
//	void MemberLogin( CPlayer* pPlayer, BOOL isLogin = TRUE );
//	void MemberLogout( CPlayer* pPlayer );
	BOOL AddMember(LPCSTR szFactionName, CPlayer *pPlayer, SQRequestJoinFaction *Msg);
	BOOL SendFactionHeadInfo( CPlayer* pPlayer );
	void EditFactionMemo(LPCSTR szFaction, LPCSTR szMemo, CPlayer * pPlayer);
	SimFactionData::SimFactionInfo* GetFacResBuffer( LPCSTR szFactionName );
	BOOL OperFacRes( LPCSTR szFacName, const DWORD dwType, const int nNum );
	BOOL UpdataMemberInfo( CPlayer *pPlayer );
    BOOL CheckMemberAtLogin( CPlayer *pPlayer );
	static BOOL DelMember(CPlayer *pPlayer, LPCSTR szTongName, LPCSTR szName, BOOL isLeave);
	static BOOL UpdateMember( SFactionData *pFaction, CPlayer* pPlayer,BOOL isLogin, BOOL sendDisable = false );
	static void SendFactionAllServer( LPCSTR szFaction, const BYTE byMsgTyte, LPVOID pMsg, WORD wSize, WORD wRegion = 0, DNID exceptDnid = 0);
	static void ProcessSSMsg( SQSendServerMsg* pMsg );
	static void SendEditFactionMemoMsg( CPlayer *pPlayer );
    static void RecvMemberManager( CPlayer *pPlayer, SQMemberManager* pMsg );
    static BOOL InitFactionRegion();
    static void GetFactionIds( std::list< WORD > &ids );

    void FactionDepletion( BOOL onlyCheck );
   
    void Run();

	void LoadFactionDataList(SAGetFactionDataMsg *pMsg);

    void SaveFaction( BOOL bSaveMode );
	void DeleteFactionRecall(SADeleteFaction *pMsg); //删除后数据库的返回

	void GetScriptData(SAGetScriptData *pMsg);
	void GetScriptData(SAScriptData *pMsg);

    BOOL RecvRemoteApplyJoinMsg( SQRemoteApplyJoin *pMsg );
    BOOL RecvOperApplyJoinMsg( SQRecvOperApplyJoin *pMsg );
    BOOL SendRApplyJoinRes( DNID dClientIndex, WORD wResType, DWORD dwApplyGID = 0 );
//    BOOL CalcAndSaveTopFiveFaction();
    BOOL RecvUpdateRecruitMsg( SQSUpdateRecruit *pMsg );
    DWORD GetFactionFightInf( WORD wFactionId, WORD wInfType );
    BOOL LoadTopFiveFactionInf();

	DWORD GetFactionAIID();

	BOOL RequestFactionListToList(CPlayer *pname, LPCSTR pfname);
	BOOL RequestFactionList(DNID dClientIndex, SQRequestFactionList * pMsg);

	BOOL RequsetFactionInfo(DNID dClientIndex, CPlayer * pMsg);

	BOOL RequsetJoinFactionList(DNID dClientIndex, CPlayer * pMsg);
	BOOL RequsetFactionMembersList(DNID dClientIndex, CPlayer * pMsg);
	BOOL Off_Line_AddMember(LPCSTR szFactionName, CPlayer *pPlayer, SQRequestJoinFaction *Msg);
	BOOL RequestAffirmJoinFacttion(DNID dClientIndex, CPlayer * pMsg);
	BOOL MemberLeave(CPlayer *p,LPCSTR szName);
	//跟新玩家的军团名字
//	BOOL UpdateMemberMsg(CPlayer *cplayer, LPCSTR newName);
	//跟新玩家军团等级
	BOOL UpdateMemberlevel(CPlayer *cplayer);

	bool m_bRefused;
	std::string m_sErrInfo;

	//---20150731 新加
	BOOL DispatchFactionMessage(DNID dnidClient, struct STongBaseMsg *pMsg, CPlayer *pPlayer);
	BOOL OnCreateFaction(DNID dnidClient,CPlayer* pPlayer, SQCreateFaction *pMsg);
	BOOL OnQuestFactionInfo(DNID dnidClient, CPlayer* pPlayer, SQFactioninfo *pMsg);
	BOOL OnQuestFactionList(DNID dnidClient, CPlayer* pPlayer, SQFactionList *pMsg);
	BOOL OnQuestJoinFaction(DNID dnidClient, CPlayer* pPlayer, SQJoinFaction *pMsg);
	BOOL OnQuestLeaveFaction(DNID dnidClient, CPlayer* pPlayer, SQLeaveFaction *pMsg);
	BOOL OnManagerFaction(DNID dnidClient, CPlayer* pPlayer, SQManagerFaction *pMsg);
	BOOL OnDeleteFaction(DNID dnidClient, CPlayer* pPlayer, SQDisbandFaction *pMsg);
	BOOL OnSynFaction(DNID dnidClient, CPlayer* pPlayer, SQSynFaction *pMsg);
};
