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
        // ���붷�������ڸ�������ϴ���
        pPlayer->m_dwPKNumber = 0;

        SAChatGlobalMsg msg;
        msg.byType = SAChatGlobalMsg::ECT_GAMEMASTER;
        dwt::strcpy(msg.cTalkerName, "[ϵͳ]", sizeof(msg.cTalkerName));
        dwt::strcpy(msg.cChatData, 
            FormatString("[%s]�����ˣ�С�İ�����", pPlayer->GetName()), 
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
        // �뿪������
        SAChatGlobalMsg msg;
        msg.byType = SAChatGlobalMsg::ECT_GAMEMASTER;
        dwt::strcpy(msg.cTalkerName, "[ϵͳ]", sizeof(msg.cTalkerName));
        dwt::strcpy(msg.cChatData, 
            FormatString("[%s]�Ѿ��뿪��,���������", pPlayer->GetName()), 
            sizeof(msg.cChatData));

        //Broadcast(&msg, sizeof(msg), pPlayer->m_ClientIndex);

        if (pPlayer->m_dwPKNumber == 0)
        {
            //TalkToDnid(pPlayer->m_ClientIndex, 
            //    FormatString("�����ǽ����ι۵İ������������ʶһ��ʲô�������ĸ��֣�"));
        }
        else
        {
            TalkToDnid(pPlayer->m_ClientIndex, 
                FormatString("�����ڱ����д�����л�����[%d]�����֣��治��Ϊ���֣�", pPlayer->m_dwPKNumber));

            static char info [][10] = {"����", "���", "����", "����", "����", "�ڿ�", 
                "��ľ", "��������", "��������", "ɵ����", "��������", "�ɾ�����"};

            if (pPlayer->m_dwPKNumber < 33)
            {
            }
            else if (pPlayer->m_dwPKNumber < 100)
            {
                TalkToAll(FormatString("�������У�����ͻ�֣�����"));
                TalkToAll(FormatString("[%s]�ڱ��ξ�����[%s]�Ļ�����[%d]�����֣�", 
                    pPlayer->GetName(), info[rand()%(sizeof(info)/10)], pPlayer->m_dwPKNumber));
            }
            else
            {
                TalkToAll(FormatString("�������У�Ѫ�ȲҰ�����ʬ��Ұ������Ϳ̿������"));
                TalkToAll(FormatString("[%s]�ھ�������[%s]�ĸɵ���[%d]�����֣�", 
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
    dwt::strcpy(msg.cTalkerName, "[ϵͳ]", sizeof(msg.cTalkerName));
    dwt::strcpy(msg.cChatData, 
        FormatString("[%s]������[%s]����ʤ�ۼ�[%d]�Σ�", pKiller->GetName(), pDead->GetName(), pKiller->m_dwPKNumber), 
        sizeof(msg.cChatData));

    Broadcast(&msg, sizeof(msg), 0);
}
