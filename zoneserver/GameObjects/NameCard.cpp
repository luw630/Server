#include "StdAfx.h"
/*
#include <time.h>
#include "namecard.h"
#include "Player.h"

#include "Networkmodule\NameCardMsgs.h"

extern LPIObject GetPlayerByGID(DWORD GID);

#define DISABLE_NAMECARD

CNameCard::CNameCard(void)
{
    // 因为没有vptr， so...
    memset((CNameCard*)this, 0, sizeof(CNameCard));
}

CNameCard::~CNameCard(void)
{
}

void CNameCard::OnDispatchMsg(SNameCardBaseMsg *pMsg)
{
#ifdef DISABLE_NAMECARD
    return;
#endif

    if (pMsg == NULL)
        return;

    // 先进行时间更新
    UpdateTime();

    switch (pMsg->_protocol)
    {
    case SNameCardBaseMsg::EPRO_NAMECARD_SETMASK:
        OnRecvNameCardMaskInfoMsg((SQNameCardSetMaskMsg*)pMsg);
        break;
    // 请求获取一个玩家的名片数据
    case SNameCardBaseMsg::EPRO_NAMECARD_QUESTINFO: 
        OnRecvNameCardQuestInfoMsg((SQNameCardQuestInfoMsg *)pMsg);
        break;

    // 请求刷新名片数据（包括自己的和别人的）
    case SNameCardBaseMsg::EPRO_NAMECARD_UPDATEINFO: 
        OnRecvNameCardUpdateInfoMsg((SQNameCardUpdateInfoMsg *)pMsg);
        break;
    }
}

void CNameCard::OnRecvNameCardQuestInfoMsg(SQNameCardQuestInfoMsg *pMsg)
{
#ifdef DISABLE_NAMECARD
    return;
#endif

    SANameCardQuestInfoMsg msg;

    CPlayer *pPlayer = (CPlayer*)this;

    if (pPlayer->m_ClientIndex == 0xffff)
        return;

    if (pMsg->dwSelfGID != pPlayer->GetGID())
        return;

    msg.byRetCode = SANameCardQuestInfoMsg::ERC_SERVER_DISCARD;
    msg.dwDestGID = pMsg->dwDestGID;

    if (pMsg->dwDestGID == pPlayer->GetGID())
    {
        // 说明是自己的名片
        msg.Info = pPlayer->CNameCard::m_Comments;
        msg.byRetCode = SANameCardQuestInfoMsg::ERC_OK;
    }
    else if (CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(pMsg->dwDestGID)->DynamicCast(IID_PLAYER))
    {
        // 说明是别人的名片
        msg.Info = pDestPlayer->CNameCard::m_Comments;
        msg.byRetCode = SANameCardQuestInfoMsg::ERC_OK;
    }

    g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SANameCardQuestInfoMsg));
}

void CNameCard::OnRecvNameCardUpdateInfoMsg(SQNameCardUpdateInfoMsg *pMsg)
{
#ifdef DISABLE_NAMECARD
    return;
#endif

    SANameCardUpdateInfoMsg msg;

    CPlayer *pPlayer = (CPlayer*)this;

    if (pPlayer->m_ClientIndex == 0xffff)
        return;

    if (pMsg->dwSelfGID != pPlayer->GetGID())
        return;

    msg.byRetCode = SANameCardUpdateInfoMsg::ERC_SERVER_DISCARD;
    msg.dwSrcGID = pMsg->dwSelfGID;
    msg.dwDestGID = pMsg->dwDestGID;
    msg.byInfoNumber = pMsg->byInfoNumber;
    msg.byIndex = 0;

    if (pMsg->dwDestGID == pPlayer->GetGID())
    {
        // 说明是自己的名片
        if (pMsg->byInfoNumber == 1)
        {
            // 只能支持自己的数据更新
            memcpy(m_Comments.szSelfIntroduction, pMsg->szSelfIntroduction, sizeof(m_Comments.szSelfIntroduction));
            memcpy(msg.szSelfIntroduction, pMsg->szSelfIntroduction, sizeof(msg.szSelfIntroduction));
            *(time_t*)&msg.UpdateTime = time(0);
            msg.byRetCode = SANameCardUpdateInfoMsg::ERC_OK;
        }
    }
    else if (CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(pMsg->dwDestGID)->DynamicCast(IID_PLAYER))
    {
        // 说明是别人的名片
        if (pMsg->byInfoNumber == 2)
        {
            // 需要检测是否可以为别人的路人评价数据更新
            for (int i=0; i<10; i++)
            {
                if (pDestPlayer->CNameCard::m_Comments.PlayerCommentSetTime[i] != 0)
                // 路人列表中已经有自己的名字了，无法再做评价
                if (dwt::strcmp(pDestPlayer->CNameCard::m_Comments.PlayerNameComment[i], pPlayer->m_Property.m_Name, sizeof(pDestPlayer->CNameCard::m_Comments.PlayerNameComment[i])) == 0)
                    return;
            }

            for (int i=0; i<10; i++)
            {
                if (m_Comments.PlayerCommentSetTime[i] == 0)
                {
                    memcpy(pDestPlayer->CNameCard::m_Comments.PlayerNameComment[i], pPlayer->m_Property.m_Name, sizeof(pDestPlayer->CNameCard::m_Comments.PlayerNameComment[i]));
                    memcpy(pDestPlayer->CNameCard::m_Comments.ppPlayerComment[i], pMsg->szSelfIntroduction, sizeof(pDestPlayer->CNameCard::m_Comments.ppPlayerComment[i]));

                    memcpy(msg.szSelfIntroduction, pDestPlayer->CNameCard::m_Comments.PlayerNameComment[i], sizeof(pDestPlayer->CNameCard::m_Comments.PlayerNameComment[i]));
                    memcpy(&msg.szSelfIntroduction[sizeof(pDestPlayer->CNameCard::m_Comments.PlayerNameComment[i])], pDestPlayer->CNameCard::m_Comments.ppPlayerComment[i], sizeof(pDestPlayer->CNameCard::m_Comments.ppPlayerComment[i]));

                    pDestPlayer->CNameCard::m_Comments.PlayerCommentSetTime[i] = time(0);
                    msg.byRetCode = SANameCardUpdateInfoMsg::ERC_OK;
                    msg.byIndex = i;
                    break;
                }
            }
        }
    }
    else
    {
        return;
    }

    g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SQNameCardUpdateInfoMsg));
}

void CNameCard::OnRecvNameCardMaskInfoMsg(SQNameCardSetMaskMsg  * pMsg)
{
#ifdef DISABLE_NAMECARD
    return;
#endif

    SANameCardSetMaskMsg msg;

    int iNumber = pMsg->byInfoNumber;
    CPlayer * pDestPlayer = NULL;
    pDestPlayer = (CPlayer *)GetPlayerByGID(pMsg->dwSelfGID)->DynamicCast(IID_PLAYER);
    if(pDestPlayer == NULL)
    {
        return ;
    }

    switch(pMsg->byInfoNumber)
    {
    case 1:
        break;
    case 2:
        {

            if(m_Comments.PlayerCommentSetTime[iNumber] != 0 && pDestPlayer->m_Comments.nMarkNumber > 0)
            {
                memcpy(pDestPlayer->CNameCard::m_Comments.PlayerNameComment[iNumber],"\0",1);
                memcpy(pDestPlayer->CNameCard::m_Comments.ppPlayerComment[iNumber],"\0",1);
                pDestPlayer->CNameCard::m_Comments.PlayerCommentSetTime[iNumber] = 0;

                msg.byRetCode = SANameCardSetMaskMsg::ERC_OK;
                msg.byInfoNumber = pMsg->byInfoNumber;
                msg.byMaskNumber = pMsg->byMaskNumber;
                msg.byLeftMask   = pDestPlayer->m_Comments.nMarkNumber-- ;
            }
            else
            {
                msg.byRetCode = SANameCardSetMaskMsg::ERC_SERVER_DISCARD;
                g_StoreMessage(pDestPlayer->m_ClientIndex,&msg,sizeof(SANameCardSetMaskMsg));
            }

        }
        break;
    case 3:
        break;
    case 4:
        break;
    }
}

void CNameCard::UpdateTime()
{
#ifdef DISABLE_NAMECARD
    return;
#endif

    // 这个操作中可能会发相映的数据
    time_t curTime = time(0); // 获得当前时间

    double diff_in_sec = difftime(curTime, *(time_t*)&m_Comments.LastMaskTime);

    // 马赛克的更新时间是在最后一次打Mask的后面
    if (diff_in_sec >= 24*60*60)
        m_Comments.nMarkNumber = 0;

    for (int i=0; i<10; i++)
    {
        if (*(time_t*)&m_Comments.PlayerCommentSetTime[i] != 0)
        {
            diff_in_sec = difftime(curTime, *(time_t*)&m_Comments.PlayerCommentSetTime[i]);

            // 现在的设定是路人评价时间在12个小时后失效，so...
            if (diff_in_sec >= 12*60*60)
            {
                *(time_t*)&m_Comments.PlayerCommentSetTime[i] = 0;
                m_Comments.PlayerNameComment[i][0] = 0;
                m_Comments.ppPlayerComment[i][0] = 0;
            }
        }
        else
        {
            m_Comments.PlayerNameComment[i][0] = 0;
            m_Comments.ppPlayerComment[i][0] = 0;
        }
    }
}
*/
