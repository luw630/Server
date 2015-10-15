#include "StdAfx.h"
#include "prison.h"
#include "player.h"

#include "globalfunctions.h"

#include "networkmodule/chatmsgs.h"

CPrison::CPrison(void)
{
}

CPrison::~CPrison(void)
{
}

int CPrison::AddObject(LPIObject pChild)
{
    CPlayer *pPlayer = (CPlayer *)pChild->DynamicCast(IID_PLAYER);
    if (pPlayer != NULL)
    {
        if (pPlayer->m_Property.m_GMLevel == 0)
        {
            SAChatGlobalMsg msg;
            msg.byType = SAChatGlobalMsg::ECT_GAMEMASTER;
            dwt::strcpy(msg.cTalkerName, "[系统]", sizeof(msg.cTalkerName));
            dwt::strcpy(msg.cChatData, 
                FormatString("[%s]进监狱来参观了！大家小心照顾哈！", pPlayer->GetName()), 
                sizeof(msg.cChatData));

            Broadcast(&msg, sizeof(msg), pPlayer->m_ClientIndex);

            TalkToDnid(pPlayer->m_ClientIndex, "怎么进来啦？！监狱可不是用来参观的啊！");
        }
    }

    return CRegion::AddObject(pChild);
}

int CPrison::DelObject(LPIObject pChild)
{
//     CPlayer *pPlayer = (CPlayer *)pChild->DynamicCast(IID_PLAYER);
//     if (pPlayer != NULL)
//     {
//         if ( pPlayer->m_Property.m_GMLevel == 0 && pPlayer->m_Property.m_byPKValue <= 4 && pPlayer->m_ClientIndex )
//         {
//             SAChatGlobalMsg msg;
//             msg.byType = SAChatGlobalMsg::ECT_GAMEMASTER;
//             dwt::strcpy(msg.cTalkerName, "[系统]", sizeof(msg.cTalkerName));
//             dwt::strcpy(msg.cChatData, 
//                 FormatString("[%s]刑满释放，大家继续努力啊！", pPlayer->GetName()), 
//                 sizeof(msg.cChatData));
// 
//             Broadcast(&msg, sizeof(msg), pPlayer->m_ClientIndex);
// 
//             TalkToDnid(pPlayer->m_ClientIndex, "恭喜恭喜，刑期已满，苦海无涯，回头是岸……");
//         }
//     }
// 
	return CRegion::DelObject(pChild);
}

void CPrison::OnRun(void)
{
    CRegion::OnRun();
}
