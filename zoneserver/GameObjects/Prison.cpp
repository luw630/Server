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
            dwt::strcpy(msg.cTalkerName, "[ϵͳ]", sizeof(msg.cTalkerName));
            dwt::strcpy(msg.cChatData, 
                FormatString("[%s]���������ι��ˣ����С���չ˹���", pPlayer->GetName()), 
                sizeof(msg.cChatData));

            Broadcast(&msg, sizeof(msg), pPlayer->m_ClientIndex);

            TalkToDnid(pPlayer->m_ClientIndex, "��ô���������������ɲ��������ι۵İ���");
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
//             dwt::strcpy(msg.cTalkerName, "[ϵͳ]", sizeof(msg.cTalkerName));
//             dwt::strcpy(msg.cChatData, 
//                 FormatString("[%s]�����ͷţ���Ҽ���Ŭ������", pPlayer->GetName()), 
//                 sizeof(msg.cChatData));
// 
//             Broadcast(&msg, sizeof(msg), pPlayer->m_ClientIndex);
// 
//             TalkToDnid(pPlayer->m_ClientIndex, "��ϲ��ϲ�������������ຣ���ģ���ͷ�ǰ�����");
//         }
//     }
// 
	return CRegion::DelObject(pChild);
}

void CPrison::OnRun(void)
{
    CRegion::OnRun();
}
