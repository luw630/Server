#pragma once

#include "common.h"
#include "Networkmodule\SportsMsgs.h"
#include "PubRoom.h"
#include "pub\ConstValue.h"

class CRoomManager;
// 分类房间 (双方自由PK和帮派群PK)
#define TEAM_NUMBERS 8
#define SPORT_TIMES 10

#define AREA_NUMBER 8
#define AREA_OFFER 1

#define _theRoomManager (CRoomManager::Instance())

class CSportRoom : public CRoom
{
	typedef dwt::stringkey<char[CONST_USERNAME]> ustring;
    typedef struct P_STATE 
    {   
        DWORD dwGID;
        DWORD dwState;
    }PSTATE;

    typedef struct P_INFO
    {
        WORD  wTeamID;
        WORD  wLevel;
        DWORD dwGID;
    }PINFO;

    typedef std::map<ustring, PINFO> PILIST; 
public:
    enum TTYPE { T_TALK, T_OK, T_ENTER };
    enum SSTYPE { SST_TERM, SST_TEAMNAME, SST_ZALLSTATE, SST_ENTERAREA, SST_CLOSETERM };
    SIDE_TERM term;
    __int64 nCreateTime;
    BYTE    byRoomType;        

protected:
    void    SendTermMsg(DWORD dwIndex);
    void    SendTermToOther(CTSet<DWORD> &set, BYTE bySSType);
    void    PutPlayerToSportsArea(DWORD dwPlayerGID);
    bool    CheckTermMoney(DWORD dwGID, DWORD dwMoney, bool bSubtract = false, bool bAdd = false, BYTE AddN = 2);
    bool    CheckTermExper(DWORD dwGID, DWORD dwExper, bool bSubOrAdd = false);
    void    PutSportsItems(class CRegion *pRegion);
    void    Bulletin(BYTE byWin);
    void    OperatePInfo(DWORD dwGID, WORD byTeam, WORD wOperate);  
    void    SendPInfo(DWORD dwGID);
    void    SendPInfoToAll(DWORD dwGID, WORD wTeam, WORD wState);
    void    SendPInfoMsg(CTSet<DWORD> &set, SASendTeamMsg &msg);
    void    SendLeaderMoneyInfo(DWORD dwGID = 0);
    ustring  GetNeedName(DWORD dwGID);

    // 帮派相关
    bool    PutFaction(void);
    void    PutFactionMember(DWORD dwGID);
    void    SendFPtoMember(void);

public:
    CSportRoom(void) {};
    CSportRoom(CRoomManager *pPubFrame, DWORD dwMasterID);
    virtual ~CSportRoom(void);

    virtual void Update(CRoomContainer *pPubFrame) {};
    virtual void Release(void) {};
    virtual bool Add(DWORD wID, BYTE bySide);
    virtual bool Del(DWORD wID, BYTE bySide);

    void    SetSideSate(BYTE byWhich, DWORD dwGID, DWORD dwState);
    DWORD   GetSideSate(BYTE byWhich);
    void    SetPState(DWORD dwGID, BYTE byTeamID, DWORD dwMasterID);
    void    SetWinner(BYTE byTeamID);

    void    OnOkTerm(PSTATE *pState, BYTE byTeamID);
    void    OnOkEnter(void);
    void    OnCancel(void);
    void    SendAskMsg(void);
    void    SendAnswerMsg(BYTE byState, DWORD dwGID);
    void    SendTermToOther(void);
    void    RecvAnswerMsg(struct SQAnswerRoomMsg *pMsg);
    void    RecvJoinSideMsg(struct SQJoinSideMsg *pMsg);
    void    RecvTermInfo(SIDE_TERM *pTerm, DWORD dwSendGID);
    void    RecvDelMember(class CPlayer *pPlayer, const char *szName);

private:

    PILIST _MemberInfo;

    CTSet<DWORD> _listOne;
    CTSet<DWORD> _listTwo;
    PSTATE _roomMaster;
    PSTATE _roomSubMaster;
    CRoomManager *_pManager;
    DWORD dwRegionGID;


};

#define CHECK_COUNT 50
class CRoomManager
{
    enum ASKSTATE { AS_CREATE, AS_JOIN };
protected:
    CRoomManager(void);
    void RecvJoinSideMsg(struct SQJoinSideMsg *pMsg);
    bool CheckAskState(DWORD dwScrID, DWORD dwDesID, WORD wAskState = AS_CREATE);
    void CheckCount(void);

    // 帮派相关 
    bool    CheckFaction(DWORD dwSrcID, bool bCheckMoney = false);
    
public:
    ~CRoomManager(void);

    void Attach(CSportRoom* pSubRoom);
    void Detach(CSportRoom* pSubRoom);
    void Detach(DWORD  masterID);

    CSportRoom *GetSubPtr(DWORD dwSubID);
    void DispatchSportsMsg(struct SSportsBaseMsg *pMsg, class CPlayer *pPlayer);

    static CRoomManager *Instance(void);

private:
    std::map<DWORD, CSportRoom> _list;
    static CRoomManager *_instance;
    WORD _Count;
};