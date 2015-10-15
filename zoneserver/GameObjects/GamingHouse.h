#pragma once

#include "PubRoom.h"
#include "common.h"
#include "Networkmodule\SportsMsgs.h"
/*==========================================================================
<����>:
�١�	��6 >   ��5 >   ��4 >   ��3 >   ��2 >   ��1                
�ڡ�	��6һ5 >  ��6һ4 > ��6һ3 >  ��6һ2 >  ��6һ1    
�ۡ�	��5һ6 >  ��5һ4 > ��5һ3 >  ��5һ2 >  ��5һ1    
�ܡ�	��4һ6 >  ��4һ5 > ��4һ3 >  ��4һ2 >  ��4һ1    
�ݡ�	��3һ6 >  ��3һ5 > ��3һ4 >  ��3һ2 >  ��3һ1    
�ޡ�	��2һ6 >  ��2һ5 > ��2һ4 >  ��2һ3 >  ��2һ1
�ߡ�	��1һ6 >  ��1һ5 > ��1һ4 >  ��1һ3 >  ��1һ2    
�ࡢ	���е���
��������6Ϊ18����5Ϊ10����4Ϊ8����3Ϊ6����2Ϊ4����1Ϊ2������Ϊ1
<ʵ��˼��>:
�Ӣٵ��࣬����Ϊ1-->37, �Ӷ���1--37֮�����,�������Ӧ����Ĺ�ϵ.
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
