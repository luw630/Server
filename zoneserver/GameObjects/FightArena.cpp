#include "StdAfx.h"
#include "fightarena.h"
#include "player.h"
#include "GlobalFunctions.h"

#include "NetworkModule/ChatMsgs.h"

CFightArena::CFightArena(void)
{
}

CFightArena::~CFightArena(void)
{
}

int CFightArena::OnCreate(_W64 long pParameter)
{
    return CRegion::OnCreate(pParameter);
}

void CFightArena::OnClose()
{
    CRegion::OnClose();
}

int CFightArena::AddObject(LPIObject pChild)
{
    CPlayer *pPlayer = (CPlayer *)pChild->DynamicCast(IID_PLAYER);
    if (pPlayer != NULL)
    {
        // 进入斗技场，在该玩家身上打标记
        pPlayer->m_dwPKNumber = 0;

        SAChatGlobalMsg msg;
        msg.byType = SAChatGlobalMsg::ECT_GAMEMASTER;
        dwt::strcpy(msg.cTalkerName, "[系统]", sizeof(msg.cTalkerName));
        dwt::strcpy(msg.cChatData, 
            FormatString("[%s]进来了，小心啊……", pPlayer->GetName()), 
            sizeof(msg.cChatData));

        Broadcast(&msg, sizeof(msg), pPlayer->m_ClientIndex);
    }

    return CRegion::AddObject(pChild);
}

int CFightArena::DelObject(LPIObject pChild)
{
    CPlayer *pPlayer = (CPlayer *)pChild->DynamicCast(IID_PLAYER);
    if (pPlayer != NULL)
    {
        // 离开斗技场
        SAChatGlobalMsg msg;
        msg.byType = SAChatGlobalMsg::ECT_GAMEMASTER;
        dwt::strcpy(msg.cTalkerName, "[系统]", sizeof(msg.cTalkerName));
        dwt::strcpy(msg.cChatData, 
            FormatString("[%s]已经离开了,请继续……", pPlayer->GetName()), 
            sizeof(msg.cChatData));

        //Broadcast(&msg, sizeof(msg), pPlayer->m_ClientIndex);

        if (pPlayer->m_dwPKNumber == 0)
        {
            //TalkToDnid(pPlayer->m_ClientIndex, 
            //    FormatString("阁下是进来参观的啊？！就让你见识一下什么是真正的高手！"));
        }
        else
        {
            TalkToDnid(pPlayer->m_ClientIndex, 
                FormatString("阁下在本次切磋过程中击败了[%d]个对手！真不愧为高手！", pPlayer->m_dwPKNumber));

            static char info [][10] = {"残忍", "愉快", "高兴", "悲伤", "下流", "勤快", 
                "麻木", "淅沥哗啦", "婆婆妈妈", "傻乎乎", "罗罗嗦嗦", "干净利落"};

            if (pPlayer->m_dwPKNumber < 33)
            {
            }
            else if (pPlayer->m_dwPKNumber < 100)
            {
                TalkToAll(FormatString("竞技场中，高手突现！！！"));
                TalkToAll(FormatString("[%s]在本次竞技中[%s]的击败了[%d]个对手！", 
                    pPlayer->GetName(), info[rand()%(sizeof(info)/10)], pPlayer->m_dwPKNumber));
            }
            else
            {
                TalkToAll(FormatString("竞技场中，血腥惨案，横尸遍野，生灵涂炭！！！"));
                TalkToAll(FormatString("[%s]在竞技场中[%s]的干掉了[%d]个对手！", 
                    pPlayer->GetName(), info[rand()%(sizeof(info)/10)], pPlayer->m_dwPKNumber));
            }
        }

        pPlayer->m_dwPKNumber = 0;
    }

    return CRegion::DelObject(pChild);
}

void CFightArena::OnRun(void)
{
    CRegion::OnRun();
}

void CFightArena::OnKill(CFightObject *PKiller, CFightObject *PDead)
{
    CPlayer *pKiller = (CPlayer *)PKiller->DynamicCast(IID_PLAYER);
    CPlayer *pDead = (CPlayer *)PDead->DynamicCast(IID_PLAYER);

    if (pKiller == NULL || pDead == NULL)
        return;

    pKiller->m_dwPKNumber ++;

    SAChatGlobalMsg msg;
    msg.byType = SAChatGlobalMsg::ECT_GAMEMASTER;
    dwt::strcpy(msg.cTalkerName, "[系统]", sizeof(msg.cTalkerName));
    dwt::strcpy(msg.cChatData, 
        FormatString("[%s]击败了[%s]，连胜累计[%d]次！", pKiller->GetName(), pDead->GetName(), pKiller->m_dwPKNumber), 
        sizeof(msg.cChatData));

    Broadcast(&msg, sizeof(msg), 0);
}
