#pragma once

#include "PubRoom.h"
#include "common.h"
#include "Networkmodule\SportsMsgs.h"
/*==========================================================================
<规则>:
①、	三6 >   三5 >   三4 >   三3 >   三2 >   三1                
②、	二6一5 >  二6一4 > 二6一3 >  二6一2 >  二6一1    
③、	二5一6 >  二5一4 > 二5一3 >  二5一2 >  二5一1    
④、	二4一6 >  二4一5 > 二4一3 >  二4一2 >  二4一1    
⑤、	二3一6 >  二3一5 > 二3一4 >  二3一2 >  二3一1    
⑥、	二2一6 >  二2一5 > 二2一4 >  二2一3 >  二2一1
⑦、	二1一6 >  二1一5 > 二1一4 >  二1一3 >  二1一2    
⑧、	所有单点
倍数：三6为18，三5为10，三4为8，三3为6，三2为4，三1为2，其他为1
<实现思想>:
从①到⑧，依次为1-->37, 从而在1--37之间随机,随机数对应上面的关系.
//=========================================================================*/


#define _theGHouseManager   (CGHouseManager::Instance())
class CGHouseManager : public CRoomContainer
{
    typedef std::list<DWORD> GHLIST;

protected:
    CGHouseManager(void);

public:
    ~CGHouseManager(void);
    static CGHouseManager *Instance(void);
    
    void PushEmptyList(DWORD dwRoomID);
    CRoom *GetEmptyRoom(void);

    void DispatchGamingHouseMsg(SGamingHouseBaseMsg *pMsg, class CPlayer *pPlayer);

private:
    static CGHouseManager *_Instance;
    GHLIST _ghouseList;
};


// gaming house 
static int GAMING_WAITTIME   = 30;
static int GAMING_RATE       = 119;
const int GAMING_MAXPLAER   = 10;
const int GAMING_BASEPLAYERNUM = 1;

class CGamingHouse : public CRoom
{
    struct GAMINGPLAYER : 
        public _GAMINGPLAYER 
    {
        DNID dnid;
        DWORD dwGID;
    };

    enum GMSTATE { GMT_CREATE, GMT_START, GMT_WAIT, GMT_EMPTY };
protected:
    void SendInfoToAll(SGamingHouseBaseMsg *pMsg, WORD wSize, DNID Except = 0);
    void SendOnePlayerInfoToAll(GAMINGPLAYER *pGPlayerInfo, DNID Except = 0);
    void SendAllPlayerInfoToOne(DNID dnid);
    void SendShowWagerToAll(void);
    void SendShowWagerToOne(GAMINGPLAYER *pGPlayerInfo);
    void SendShowGamingCast(void);

    int  GetSpacePos(const char* name);
    BOOL CheckGPlayer(DWORD dwGID);
    void GamingCast(GAMINGPLAYER *pGPlayer);
    WORD GetMultiple(WORD wPutNumber);
    void GameZero(void);
    void GameResult(void);
    void Budget(GAMINGPLAYER *pOtherGPlayer);
    GAMINGPLAYER *GetPlayer(WORD wID);
    void BackWager(void);
    BOOL CheckMoney(DWORD dwGID, DWORD dwMoney);
    void PayToMaster(GAMINGPLAYER *pGPlayer);

public:
    CGamingHouse(DWORD dwRoomID);
    ~CGamingHouse(void);

    virtual void Release(void);
    virtual void Update(CRoomContainer *pSubFrame);

    BOOL AddGamingPlayer(class CPlayer *pPlayer);
    BOOL DelGamingPlayer(WORD wListID, class CPlayer *pPlayer);
    BOOL GamingLoop(void);
    void GamingOver(void);

    void RecvWagerMessage(SQShowWagerInfoMsg *pMsg, class CPlayer *pPlayer);
    void RecvGamingcastMessage(SQShowGamingcastMsg *pMsg, class CPlayer *pPlayer);
    void RecvGamingPlayerRequestMessage(SQGamingPlayerRequestMsg *pMsg, class CPlayer *pPlayer);

    WORD CountPlayer(void) const { return _wCurPlayerNumber; }
    BOOL IsEmptyRoom(void) const { return (_GamingState == GMT_EMPTY); }
    void InitNew(void);

private:
    GAMINGPLAYER _gamingPlayerList[GAMING_MAXPLAER];
    CTSet<WORD> _winSet;
    CTSet<WORD> _loseSet;
    WORD _wCurPlayerNumber;
    WORD _wCurPlayerPos;
    WORD _wCount;
    WORD _GamingState;
    BOOL _bNewLoop;
    DWORD _dwCountTime;
    DWORD _Budget[2];   
};
