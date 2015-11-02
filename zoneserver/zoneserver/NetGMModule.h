#pragma once 
#include "networkmodule\GMModuleMsg.h"
#include <map>
#include <list>
#include <string>

#ifdef GMMODULEON

typedef enum 
{
    GM_LEVEL_1,
    GM_LEVEL_2,
    GM_LEVEL_3,
    GM_LEVEL_4,
    GM_LEVEL_5
} GMLEVEL;

typedef enum
{
    GM_ACT_TALKMASK,
    GM_ACT_MOVEPLAYER,
    GM_ACT_CHECKPLAYER,
    GM_ACT_CHANGEPLAYER
} GM_ACTION;

struct SGMOperation
{
    GMLEVEL GMLevel;
    GM_ACTION GMAction;
};

class CGMLog
{
    void Log(LPTSTR Name,GMLEVEL Level,SGMOperation &GMOperation,DWORD dwSize);//
    //参数：GM玩家的名字，GM玩家的等级，记录内容，内容大小 
    BOOL ReadLog(LPTSTR Name);
    std::list<SGMOperation> m_GMOperationList;
public:
    static unsigned int __stdcall ReadFileFunc(void *pParameter);
    void ClearOprList();
    void AddOprList(SGMOperation &GMOperaion);
    CGMLog();
    ~CGMLog();
};


struct SStructForMultThread
{
    char szFileName[20];
    char cReadBuf[1024];
    CGMLog * pGMLog;

};
static SStructForMultThread ThreadParameter;


class CGMLevelChecker
{
public:
    BOOL CheckLevel(BYTE Action, WORD Level); // TRUE 权限足够 FALSE 权限不足

public:
    CGMLevelChecker();
    ~CGMLevelChecker();
};

class CNetGMModule
{
    CGMLevelChecker * m_pLevelChecker;

    CGMLog * m_pGMLog;
    
    BOOL m_bSendChatCheck;
    WORD m_wChatCheckNumber; // 有GM在查看聊天记录


public:
    void Init();
    BOOL OnGMMessage(DNID dnidClient,SGMMMsg * pGMMsg,BOOL bDistribute);
    BOOL GetChatMessage(DNID dnidClient,struct SChatBaseMsg* pChatBaseMsg,CPlayer *pPlayer);
    BOOL GetORBChatMessage(struct SORBMsg *pMsg);
    CNetGMModule();
    ~CNetGMModule();
};

#endif