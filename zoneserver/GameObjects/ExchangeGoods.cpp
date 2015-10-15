#include "StdAfx.h"
#include "exchangegoods.h"
#include "player.h"
#include "item.h"
#include "count.h"
#include "GlobalFunctions.h"
#include "region.h"
#include "dynamicregion.h"
#include "Networkmodule\ScriptMsgs.h"
#include "Networkmodule\ItemMsgs.h"
#include "CItemDefine.h"
#include "CItemService.h"
#include "GameWorld.h"
#include <time.h>
#include "Networkmodule/SectionMsgs.h"

extern "C" DWORD _GetCrc32(LPVOID buf, DWORD size);
extern LPIObject GetPlayerByDnid(DNID dnidClient);
extern LPIObject GetPlayerByGID(DWORD);
extern CGameWorld *&GetGW();

CExchangeGoods::CExchangeGoods(void)
{
	memset((CExchangeGoods*)this, 0, sizeof(CExchangeGoods));
}

CExchangeGoods::~CExchangeGoods(void)
{
}

void CExchangeGoods::OnDispatchMsg(SExchangeBaseMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "ExchangeGoods.cpp - OnDispatchMsg() - !pMsg");
		return;
	}

    switch (pMsg->_protocol)
    {
    case SExchangeBaseMsg::RPRO_QUEST_EXCHANGE:
        RecvQuestExchangeMsg((SQQuestExchangeMsg *)pMsg);
        break;

    case SExchangeBaseMsg::RPRO_MOVE_EXCHANGE_ITEM:
        RecvPutItemToExchangeBoxMsg((SQPutItemToExchangeBoxMsg *)pMsg);
        break;

    case SExchangeBaseMsg::RPRO_EXCHANGE_OPERATION:
        RecvExchangeOperationMsg((SQExchangeOperationMsg *)pMsg);
        break;

    case SExchangeBaseMsg::RPRO_QUEST_REMOTE_EXCHANGE:
        //RecvRemoteExchangeOperationMsg((SQAQuestRemoteExchangeMsg *)pMsg);
        break;
    }
}

void CExchangeGoods::ExchgLoseStatus()
{
	CPlayer *pPlayer = static_cast<CPlayer *>(this);
	if (!pPlayer)
		return;

	// ���ȿ����Ƿ��ڵȴ����׻�Ӧ
	if (pPlayer->m_ChxSentOut.size())
	{
		for (CPlayer::CHXQuestList::iterator it = pPlayer->m_ChxSentOut.begin(); it != pPlayer->m_ChxSentOut.end(); ++it)
		{
			CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(it->first)->DynamicCast(IID_PLAYER);
			if (!pDestPlayer)
				continue;

			MY_ASSERT(pDestPlayer->m_CurHp);
			pDestPlayer->m_ChxGetIn.erase(pPlayer->GetGID());
		}

		pPlayer->m_ChxSentOut.clear();
	}

	// �Ƿ��ڵȴ���Ӧ��������
	if (pPlayer->m_ChxGetIn.size())
	{
		CPlayer::CHXQuestList tempCopy(pPlayer->m_ChxGetIn);

		for (CPlayer::CHXQuestList::iterator it = tempCopy.begin(); it != tempCopy.end(); ++it)
		{
			CPlayer *pQuest = (CPlayer *)GetPlayerByGID(it->first)->DynamicCast(IID_PLAYER);
			if (pQuest)
			{
				MY_ASSERT(pQuest->m_CurHp);

				// ��ʱ���ܾ�
				SQQuestExchangeMsg rejectMsg;
				rejectMsg.bAccept    = false;
				rejectMsg.dnidClient = 0;
				rejectMsg.dwDestGID  = pPlayer->GetGID();
				rejectMsg.dwSrcGID   = it->first;

				RecvQuestExchangeMsg(&rejectMsg, true);
			}
			else
			{
				MY_ASSERT(0);
				pPlayer->m_ChxGetIn.erase(it->first);
			}
		}

		MY_ASSERT(pPlayer->m_ChxGetIn.empty());
	}

	// �Ƿ��ڽ�����
	if (pPlayer->InExchange())
	{
		CExchangeGoods::TheEnd();
	}
}

void CExchangeGoods::RecvQuestExchangeMsg(struct SQQuestExchangeMsg *pMsg, bool Auto)
{
	if (!pMsg)
	{
		rfalse(4, 1, "ExchangeGoods.cpp - RecvQuestExchangeMsg() - !pMsg");
		return;
	}

	// ������Ŀ�괫��ѡ���Ƿ���ܽ���
	CPlayer *pPlayer = static_cast<CPlayer *>(this);
	if (!pPlayer)
		return;

	if ((pPlayer->GetGID() == pMsg->dwSrcGID) || (pPlayer->GetGID() != pMsg->dwDestGID))
		return;

	CPlayer *pQuest = (CPlayer *)GetPlayerByGID(pMsg->dwSrcGID)->DynamicCast(IID_PLAYER);
	if (!pQuest)
        return;

	CPlayer::CHXQuestList::iterator QuestIt = pQuest->m_ChxSentOut.find(pPlayer->GetGID());
	CPlayer::CHXQuestList::iterator	ReplyIt = pPlayer->m_ChxGetIn.find(pQuest->GetGID());

	if ((QuestIt == pQuest->m_ChxSentOut.end()) || (ReplyIt == pPlayer->m_ChxGetIn.end()))
		MY_ASSERT((QuestIt == pQuest->m_ChxSentOut.end()) && (ReplyIt == pPlayer->m_ChxGetIn.end()));
	if ((QuestIt != pQuest->m_ChxSentOut.end()) || (ReplyIt != pPlayer->m_ChxGetIn.end()))
		MY_ASSERT((QuestIt != pQuest->m_ChxSentOut.end()) && (ReplyIt != pPlayer->m_ChxGetIn.end()));

	if (QuestIt == pQuest->m_ChxSentOut.end() || ReplyIt == pPlayer->m_ChxGetIn.end())
		return;

	MY_ASSERT(pQuest->m_CurHp);

	// �Ƴ�����
	pQuest->m_ChxSentOut.erase(QuestIt);
	pPlayer->m_ChxGetIn.erase(ReplyIt);

	// ������彻�ײ��֣����еĽ��׶��ɽ��׷�����������
	if (pMsg->bAccept)
    {
		if (0 == pPlayer->m_CurHp || 0 == pQuest->m_CurHp)
			return;
		
		if (pQuest->InExchange() || pPlayer->InExchange())
			return;

		// ���ƾ���
		DWORD distance = sqrt(pow((pQuest->m_curX-pPlayer->m_curX),2)+pow((pQuest->m_curY-pPlayer->m_curY), 2));
		if (distance > 500)
			return;			// �����Զ
	
		// �Է�ͬ�⽻��
		pQuest->StartExchange(pPlayer);
    }
	else
	{
		if (false == Auto && 0 == pPlayer->m_CurHp)
			return;

		// ���ױ��ܾ�
		TalkToDnid(pQuest->m_ClientIndex, "���Ľ������󱻾ܾ���");
	}
}

void CExchangeGoods::RecvPutItemToExchangeBoxMsg(struct SQPutItemToExchangeBoxMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "ExchangeGoods.cpp - RecvPutItemToExchangeBoxMsg() - !pMsg");
		return;
	}

	// һ�����׷�������һ����Ʒ
	CPlayer *pPlayer = static_cast<CPlayer *>(this);
	if (!pPlayer)
		return;

    CPlayer *pSrcPlayer = 0;
    CPlayer *pDestPlayer = 0;

	bool isSrc = false;
	
	if (!pPlayer->InExchange())
		return;

    // �ҵ�����������
    if (pPlayer->GetGID() == pMsg->dwSrcGID)
    {
        isSrc = true;
        pSrcPlayer = pPlayer;
        pDestPlayer = (CPlayer *)GetPlayerByGID(pMsg->dwDestGID)->DynamicCast(IID_PLAYER);
		if (!pDestPlayer || !pDestPlayer->InExchange())
			return;
    }
    else if (pPlayer->GetGID() == pMsg->dwDestGID)
    {
        pSrcPlayer = (CPlayer *)GetPlayerByGID(pMsg->dwSrcGID)->DynamicCast(IID_PLAYER);
        pDestPlayer = pPlayer;
		if (!pSrcPlayer || !pSrcPlayer->InExchange())
			return;
    }

    // �ж��Ƿ���ȷ���Ľ���˫��
    if ((pSrcPlayer->m_dwSrcPlayerGID != pSrcPlayer->GetGID())   ||
        (pSrcPlayer->m_dwDestPlayerGID != pDestPlayer->GetGID()) ||
        (pSrcPlayer->m_dwOtherPlayerGID != pDestPlayer->GetGID()))
        return;

	MY_ASSERT(pSrcPlayer->m_CurHp && pDestPlayer->m_CurHp);

    // ������彻�ײ��֣����еĽ��׶��ɽ��׷�����������
    pSrcPlayer->PutItemToExchangeBox(pDestPlayer, pMsg->Item, pMsg->wCellX, pMsg->wCellY, isSrc);
}

void CExchangeGoods::RecvRemoteExchangeOperationMsg( struct SQAQuestRemoteExchangeMsg *pMsg )
{
	if (!pMsg)
	{
		rfalse(4, 1, "ExchangeGoods.cpp - RecvRemoteExchangeOperationMsg() - !pMsg");
		return;
	}

    // Զ�̽���˼·��Ϊÿһ����ɫ���һ����ó��Ϣ��map<DWORD, DWORD>( itemIdx, time )
    // ֻҪ�Ƿ�����������Ϣ�ģ��ͻ��������������һ����Ϣ��ͬʱ����Ѿ����ڵ���Ϣ��
    // �����еĲ�����ȷ�ϡ�ȡ����
    CPlayer *src = static_cast<CPlayer *>(this);
    if (src == NULL)
        return;

    // ����߼��ǿͻ��˷���Զ�̽�������
    if ( pMsg->dnidClient == 0 )
    {
        CPlayer *dst = ( CPlayer* )GetPlayerByName( pMsg->name )->DynamicCast( IID_PLAYER );

        if ( dst == NULL || dst->m_ClientIndex == 0 || dst == src )
        { 
            TalkToDnid( src->m_ClientIndex, "Զ�̽���Ŀ����Ҳ����ߣ�" );
            return;
        }

        //������Ϸ����
        if ( src->IsBetting() || dst->IsBetting() )
        {
            TalkToDnid( src->m_ClientIndex, src->IsBetting() ? 
                "��Ϸ�������㲻�ܽ��н��ס���" : "�Է�������Ϸ�����У�û�취�͸��½��ס���" );
            return;
        }

        if ( src->m_ParentRegion == NULL || dst->m_ParentRegion == NULL ||
            src->m_ParentRegion->DynamicCast( IID_DYNAMICREGION ) || dst->m_ParentRegion->DynamicCast( IID_DYNAMICREGION ) )
        {
            TalkToDnid( src->m_ClientIndex, "����ɫλ�ڶ�̬����ʱ������ʹ��Զ�̽���" );
            return;
        }

        if ( src->InExchange() || dst->InExchange() )
        {
            TalkToDnid( src->m_ClientIndex, src->InExchange() ? 
                "��ǰ״̬�����㲻�ܽ��н��ס���" : "�Է����ڽ����У�û�취�͸��½��ס���" );
            return;
        }

//         if ( src->GetSaleState() != 0 || dst->GetSaleState() )
//         {
//             TalkToDnid( src->m_ClientIndex, ( src->GetSaleState() != 0 ) ? 
//                 "�������ڰ�̯�У����ܽ��н��ס���" : "�Է����ڰ�̯�У�û�취�͸��½��ס���" );
//             return;
//         }

        std::map< DWORD, time_t >::iterator it = dst->excInfoMap.find( pMsg->item );
        if ( it == dst->excInfoMap.end() || abs( ( int )( time( NULL ) - it->second ) ) > 5*60 )
        { 
            TalkToDnid( src->m_ClientIndex, "���������Ϣ�Ѿ����ڣ�" );
            return;
        }

        SQAQuestRemoteExchangeMsg msg;
        msg.dnidClient = src->m_ClientIndex;
        msg.dwSrcGID = src->GetGID();
        msg.dwDestGID = dst->GetGID();
        msg.item = pMsg->item;
        dwt::strcpy( msg.name, src->GetName(), sizeof( msg.name ) );
        if ( g_StoreMessage( dst->m_ClientIndex, &msg, sizeof( msg ) ) )
            TalkToDnid( src->m_ClientIndex, "�Ѿ���Ŀ��ת������Ľ�������" );
        else
            TalkToDnid( src->m_ClientIndex, "��Ŀ��ת�ｻ������Ĺ����г��ִ���" );
    }
    else
    {
        CPlayer *dst = ( CPlayer* )GetPlayerByDnid( pMsg->dnidClient )->DynamicCast( IID_PLAYER );
        if ( dst == NULL || dst->m_ClientIndex == 0 || dst == src )
        { 
            TalkToDnid( src->m_ClientIndex, "Զ�̽���Ŀ����Ҳ����ߣ�" );
            return;
        }

        if ( pMsg->item == 0 || pMsg->dwSrcGID != src->GetGID() || pMsg->dwDestGID != dst->GetGID() )
            TalkToDnid( src->m_ClientIndex, FormatString( "%s�ܾ�����Ľ�������", dst->GetName() ) );
        else
        {
            if ( src->m_ParentRegion == NULL || dst->m_ParentRegion == NULL ||
                src->m_ParentRegion->DynamicCast( IID_DYNAMICREGION ) || dst->m_ParentRegion->DynamicCast( IID_DYNAMICREGION ) )
            {
                TalkToDnid( src->m_ClientIndex, "����ɫλ�ڶ�̬����ʱ������ʹ��Զ�̽���" );
                return;
            }

            std::map< DWORD, time_t >::iterator it = src->excInfoMap.find( pMsg->item );
            if ( it == src->excInfoMap.end() || abs( ( int )( time( NULL ) - it->second ) ) > 5*60 )
                TalkToDnid( src->m_ClientIndex, "����Ӧ��Զ�̽��������Ѿ����ڣ�" );
            else
                StartExchange( dst ); // ������������
        }
    }
}

void CExchangeGoods::RecvExchangeOperationMsg(struct SQExchangeOperationMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "ExchangeGoods.cpp - RecvExchangeOperationMsg() - !pMsg");
		return;
	}

	// �����еĲ�����������ȷ�ϡ�ȡ����
	CPlayer *pPlayer = static_cast<CPlayer *>(this);
	if (!pPlayer)
        return;

	CPlayer *pSrcPlayer  = 0;
    CPlayer *pDestPlayer = 0;

	bool isSrc = false;

	// ��������Ѿ��������߱����û��ʼ
	if (!pPlayer->InExchange())
		return;

	// �ҵ�����������
    if (pPlayer->GetGID() == pMsg->dwSrcGID)
    {
		isSrc = true;
        pSrcPlayer = pPlayer;
        pDestPlayer = (CPlayer *)GetPlayerByGID(pMsg->dwDestGID)->DynamicCast(IID_PLAYER);
		if (!pDestPlayer || !pDestPlayer->InExchange())
			return;
    }
    else if (pPlayer->GetGID() == pMsg->dwDestGID)
    {
        pSrcPlayer = (CPlayer *)GetPlayerByGID(pMsg->dwSrcGID)->DynamicCast(IID_PLAYER);
        pDestPlayer = pPlayer;
		if (!pSrcPlayer || !pSrcPlayer->InExchange())
			return;
    }

    // �ж��Ƿ���ȷ���Ľ���˫��
    if ((pSrcPlayer->m_dwSrcPlayerGID   != pSrcPlayer->GetGID())  ||
        (pSrcPlayer->m_dwDestPlayerGID  != pDestPlayer->GetGID()) ||
        (pSrcPlayer->m_dwOtherPlayerGID != pDestPlayer->GetGID()))
        return;

	MY_ASSERT(pSrcPlayer->m_CurHp && pDestPlayer->m_CurHp);

	// ������彻�ײ��֣����еĽ��׶��ɽ��׷�����������
	if (pMsg->operation == SQExchangeOperationMsg::LOCK || pMsg->operation == SQExchangeOperationMsg::UNLOCK)
	{
		// ��һ����������/������
		pSrcPlayer->LockExchange(pDestPlayer, isSrc, pMsg->operation == SQExchangeOperationMsg::LOCK ? true : false);
	}
    else if (pMsg->operation == SQExchangeOperationMsg::COMMIT)
	{
		// ��һ�����㽻��ȷ��
        pSrcPlayer->CommitExchange(pDestPlayer, isSrc);
	}
	else if (pMsg->operation == SQExchangeOperationMsg::CANCEL)
	{
		// ��һ������ȡ������
		pSrcPlayer->ExchangeCancel(pDestPlayer, isSrc);
	}
}

void CExchangeGoods::LockExchange(CPlayer *pDestPlayer, bool isSrc, bool isLock)
{
	CPlayer *pSrcPlayer = static_cast<CPlayer *>(this);
	if (!pSrcPlayer)
		return;

	CPlayer *pPlayer = isSrc ? pSrcPlayer : pDestPlayer;
	SExchangeBox *pExchangeBox = isSrc ? &m_MyBox : &m_YouBox;

	// ����Ѿ�����˽��ף����޷���������/������
	if (m_MyBox.m_bCommit)
	{
		pPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_ALREADY_IN_COMMIT);
		return;
	}

	if (m_YouBox.m_bCommit)
	{
		pPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_ALREADY_IN_COMMIT);
		return;
	}

	// ֪ͨ����˫������һ�����Ѿ�����/������
	SAExchangeOperationMsg opmsg;
	opmsg.dwSrcGID	= pSrcPlayer->GetGID();
	opmsg.dwDestGID = pDestPlayer->GetGID();
	opmsg.isSrc		= isSrc;

	if (isLock)
	{
		// ����Ѿ������ˣ�ֱ�ӷ���
		if (pExchangeBox->m_bLocked)
			return;

		pExchangeBox->m_bLocked = true;
		opmsg.operation = SAExchangeOperationMsg::LOCK;
	}
	else
	{
		// ���û��������ֱ�ӷ���
		if (!pExchangeBox->m_bLocked)
			return;

		pExchangeBox->m_bLocked = false;
		opmsg.operation = SAExchangeOperationMsg::UNLOCK;
	}
	
	g_StoreMessage(pSrcPlayer->m_ClientIndex, &opmsg, sizeof(SAExchangeOperationMsg));
	g_StoreMessage(pDestPlayer->m_ClientIndex, &opmsg, sizeof(SAExchangeOperationMsg));

	return;
}

void CExchangeGoods::QuestExchangeWith(DWORD dwGID)
{
	// ֻ����ҿ��Խ��н���
	CPlayer *pPlayer = static_cast<CPlayer *>(this);
	if (!pPlayer)
        return;

	if (0 == dwGID || pPlayer->GetGID() == dwGID)
		return;

	if (0 == pPlayer->m_CurHp)
		return;

	if (pPlayer->InExchange())
    {
		// �Լ������ڽ�����
		pPlayer->SendStatusMsg(SANotifyStatus::NS_EXG_SELFINEXGING);
        return;
    }

	CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
	if (!pDestPlayer || 0 == pDestPlayer->m_CurHp)
    {
		// ���׶��󲻴���/�̱�����
		pPlayer->SendStatusMsg(SANotifyStatus::NS_EXG_CANTFIND_DEST);
        return;
    }

	if (pDestPlayer->InExchange())
    {
		// ���׶��������ڽ�����
		pPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_DESTEXGING);
        return;
	}

	// �Ƿ��Ѿ�������������
	CPlayer::CHXQuestList::iterator hasSendOut = pPlayer->m_ChxSentOut.find(pDestPlayer->GetGID());
	if (hasSendOut != pPlayer->m_ChxSentOut.end())
		return;

	// ���ƾ���
	DWORD distance = sqrt(pow((pPlayer->m_curX-pDestPlayer->m_curX),2)+pow((pPlayer->m_curY-pDestPlayer->m_curY), 2));
	if (distance > 500)
		return;

	SAQuestExchangeMsg msg;
    msg.dnidClient	= pPlayer->m_ClientIndex;
    msg.dwSrcGID	= pPlayer->GetGID();
    msg.dwDestGID	= pDestPlayer->GetGID();
    msg.bAccept		= false;	// Ϊ�٣�Ҳ���ǽ��������״̬
    g_StoreMessage(pDestPlayer->m_ClientIndex, &msg, sizeof(SAQuestExchangeMsg));

	pPlayer->m_ChxSentOut[pDestPlayer->GetGID()] = pDestPlayer->m_ChxGetIn[pPlayer->GetGID()] = timeGetTime();
	
    // ֪ͨ������ҵȴ��Է���ȷ��
	pPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_WAITDEST_RESPOND);
}

void CExchangeGoods::StartExchange(CPlayer *pDestPlayer)
{
	CPlayer *pPlayer = static_cast<CPlayer *>(this);
	if (!pPlayer || 0 == pPlayer->m_CurHp)
		return;

	MY_ASSERT(!pPlayer->InExchange() && !pDestPlayer->InExchange());

	// ͨ�����ּ�⣬��ʼ������״̬���ر�Ҫע�����ǣ���Ҫ�趨����˫����ID
    m_dwOtherPlayerGID	= pDestPlayer->GetGID();
    m_dwSrcPlayerGID	= pPlayer->GetGID();
    m_dwDestPlayerGID	= pDestPlayer->GetGID();
    memset(&m_MyBox, 0, sizeof(SExchangeBox));
    memset(&m_YouBox, 0, sizeof(SExchangeBox));

    pDestPlayer->m_dwOtherPlayerGID = pPlayer->GetGID();
    pDestPlayer->m_dwSrcPlayerGID	= pPlayer->GetGID();
    pDestPlayer->m_dwDestPlayerGID	= pDestPlayer->GetGID();
    memset(&pDestPlayer->m_MyBox, 0, sizeof(SExchangeBox));
    memset(&pDestPlayer->m_YouBox, 0, sizeof(SExchangeBox));

	// ͬʱ֪ͨ˫���򿪽������
    SAQuestExchangeMsg msg;
    msg.dnidClient	= pPlayer->m_ClientIndex;
    msg.dwSrcGID	= pPlayer->GetGID();
    msg.dwDestGID	= pDestPlayer->GetGID();
    msg.bAccept		= true;		// Ϊ�棬Ҳ���Ǵ򿪽������
    g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAQuestExchangeMsg));
    g_StoreMessage(pDestPlayer->m_ClientIndex, &msg, sizeof(SAQuestExchangeMsg));
}

void CExchangeGoods::PutItemToExchangeBox(CPlayer *pDestPlayer, SPackageItem &item, WORD wDestX, WORD wDestY, bool isSrc)
{
	if (!pDestPlayer)
	{
		rfalse(4, 1, "ExchangeGoods.cpp - PutItemToExchangeBox() - !pDestPlayer");
		return;
	}

	CPlayer *pSrcPlayer = static_cast<CPlayer *>(this);
    if (!pSrcPlayer)
        return;

    CPlayer *pPlayer = isSrc ? pSrcPlayer : pDestPlayer;
    SExchangeBox *pExchangeBox = isSrc ? &m_MyBox : &m_YouBox;

	if (!pExchangeBox)
	{
		rfalse(4, 1, "ExchangeGoods.cpp - PutItemToExchangeBox() - !pExchangeBox");
		return;
	}

	// ����������������ύ�����޷�������ӵ���
	if (pExchangeBox->m_bLocked || pExchangeBox->m_bCommit)
	{
		pPlayer->SendStatusMsg(pExchangeBox->m_bLocked ? SANotifyStatus::NS_EXT_ALREADY_IN_LOCK : SANotifyStatus::NS_EXT_ALREADY_IN_COMMIT);
        return;
	}

	SAPutItemToExchangeBoxMsg msg;
	msg.dwSrcGID	= pSrcPlayer->GetGID();
    msg.dwDestGID	= pDestPlayer->GetGID();
    msg.isSrc		= isSrc;
    msg.wCellX		= wDestX;
    msg.wCellY		= wDestY;
    msg.Item		= item;

    bool bSuccess = false;

    // �Ƿ��ǽ��׽�Ǯ
	if (0xffff == item.wIndex)
    {
		if (0 == item.details.uniqueTimestamp || item.details.uniqueTimestamp > 99999999)
		{
			pPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_MONEY_INVALID);
			return;
		}

		if (item.details.uniqueTimestamp > pPlayer->m_Property.m_Money)
            return;

        // ͨ����⣬���½���״̬
        pExchangeBox->m_dwMoney = item.details.uniqueTimestamp;
		pPlayer->m_IsMoneyLocked = true;	// �˴�������Ǯ
		bSuccess = true;
    }
    else
    {
		// �ж�Ŀ����ߵĴ�����
		SPackageItem *pItem = pPlayer->FindItemByPos(item.wCellPos, XYD_FT_ONLYUNLOCK);
		
		if (!pItem || pItem->wIndex != msg.Item.wIndex)
            return;

		const SItemBaseData *pItemData = CItemService::GetInstance().GetItemBaseData(item.wIndex);
		if (!pItemData || ITEM_IS_TASK(pItemData->m_Type))	// ������߲��ɽ���
			return;

		// �ж������Ƿ���Ա�����
		if (!pItemData->m_CanTran)
		{
			TalkToDnid(pPlayer->m_ClientIndex, "�õ����޷������ף�");
			pPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_ITEM_CANTEXG);
			return;
		}
		if (item.IsBindState())
		{
			TalkToDnid(pPlayer->m_ClientIndex, "�õ����Ѱ󶨣����ܱ����ס�");
			pPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_ITEM_BIND);
			return ;
		}

		// �������Ƿ��㹻
		if (MAX_EXCHANGE_ITEM_NUMBER == pExchangeBox->m_SellNumber)
		{
			TalkToDnid(pPlayer->m_ClientIndex, "������������");
			pPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_EXTBOX_FULL);
			return;
		}
	
		SPackageItem *const pTempItem = &pExchangeBox->m_GoodsArray[pExchangeBox->m_SellNumber];
		*pTempItem = *pItem;
		msg.wCellX		= pExchangeBox->m_SellNumber / 5;
		msg.wCellY		= pExchangeBox->m_SellNumber % 5;
		msg.Item		= *pItem;

		pExchangeBox->m_SellNumber++;

		bSuccess = true;

		pPlayer->LockItemCell(item.wCellPos, true);
    }

    if (bSuccess)
    {
        g_StoreMessage(pSrcPlayer->m_ClientIndex, &msg, sizeof(SAPutItemToExchangeBoxMsg));
        g_StoreMessage(pDestPlayer->m_ClientIndex, &msg, sizeof(SAPutItemToExchangeBoxMsg));
    }
}

void CExchangeGoods::ExchangeCancel(CPlayer *pDestPlayer, bool isSrc)
{
	if (!pDestPlayer)
		return;

	CPlayer *pSrcPlayer = static_cast<CPlayer *>(this);
	if (!pSrcPlayer)
		return;

	// ����ȡ������Ҫ�ָ��������ĵ��ߺͽ�Ǯ
	for (int i=0; i<m_MyBox.m_SellNumber; i++)
	{
		SPackageItem &item  = m_MyBox.m_GoodsArray[i];
		SPackageItem *pItem = pSrcPlayer->FindItemByPos(item.wCellPos, XYD_FT_ONLYLOCK);
		
		if (!pItem)
		{
			rfalse(4, 1, "ExchangeGoods.cpp - ExchangeCancel() - !pItem");
			return;
		}

		MY_ASSERT(pItem);

		pSrcPlayer->LockItemCell(pItem->wCellPos, false);
	}

	for (int i=0; i<m_YouBox.m_SellNumber; i++)
	{
		SPackageItem &item  = m_YouBox.m_GoodsArray[i];
		SPackageItem *pItem = pDestPlayer->FindItemByPos(item.wCellPos, XYD_FT_ONLYLOCK);
		if (!pItem)
		{
			rfalse(4, 1, "ExchangeGoods.cpp - ExchangeCancel() - !pItem");
			return;
		}
		MY_ASSERT(pItem);

		pDestPlayer->LockItemCell(pItem->wCellPos, false);
	}

	// �����ǰ�Ľ���״̬�������˽�Ǯ��������
	memset((CExchangeGoods *)pSrcPlayer, 0, sizeof(CExchangeGoods));
	memset((CExchangeGoods *)pDestPlayer, 0, sizeof(CExchangeGoods));

	// ֪ͨ�ͻ��˽��ױ�ȡ��
	SAExchangeOperationMsg opmsg;
	opmsg.dwSrcGID	= pSrcPlayer->GetGID();
	opmsg.dwDestGID = pDestPlayer->GetGID();
	opmsg.isSrc		= isSrc;
	opmsg.operation = SAExchangeOperationMsg::CANCEL;

	g_StoreMessage(pSrcPlayer->m_ClientIndex, &opmsg, sizeof(SAExchangeOperationMsg));
	g_StoreMessage(pDestPlayer->m_ClientIndex, &opmsg, sizeof(SAExchangeOperationMsg));

	return;
}

void CExchangeGoods::CommitExchange(CPlayer *pDestPlayer, bool isSrc, DWORD dwVerifyID4CardPoint)
{
	if (!pDestPlayer)
		return;

	CPlayer *pSrcPlayer = static_cast<CPlayer *>(this);
	if (!pSrcPlayer)
		return;

	CPlayer *pPlayer = isSrc ? pSrcPlayer : pDestPlayer;
    SExchangeBox *pExchangeBox = isSrc ? &m_MyBox : &m_YouBox;

	if (!pPlayer || !pExchangeBox)
	{
		rfalse(4, 1, "ExchangeGoods.cpp - CommitExchange() - !pPlayer || !pExchangeBox");
		return;
	}

	// ��û��ȷ��
	if (!(m_MyBox.m_bLocked && m_YouBox.m_bLocked))
	{
		TalkToDnid(pPlayer->m_ClientIndex, "һ����˫��û���������޷��ύ����");
		return;
	}

	if (pExchangeBox->m_bCommit)
		return;

    pExchangeBox->m_bCommit = true;

	if (!(m_MyBox.m_bCommit && m_YouBox.m_bCommit))
    {
		TalkToDnid(pPlayer->m_ClientIndex, "�Է���û���ύ����ȴ�");

		// ֪ͨ����˫������һ�����Ѿ�ȷ����
		SAExchangeOperationMsg opmsg;
        opmsg.dwSrcGID	= pSrcPlayer->GetGID();
        opmsg.dwDestGID = pDestPlayer->GetGID();
        opmsg.isSrc		= isSrc;
        opmsg.operation = SAExchangeOperationMsg::COMMIT;

		g_StoreMessage(pSrcPlayer->m_ClientIndex, &opmsg, sizeof(SAExchangeOperationMsg));
		g_StoreMessage(pDestPlayer->m_ClientIndex, &opmsg, sizeof(SAExchangeOperationMsg));
        return;
    }

	// ˫������ȷ�ϣ����Կ�ʼ�����ˣ�����Ǯ�ĺϷ���
	MY_ASSERT(pSrcPlayer->m_Property.m_Money >= m_MyBox.m_dwMoney);
	MY_ASSERT(pDestPlayer->m_Property.m_Money >= m_YouBox.m_dwMoney);

	if ((pSrcPlayer->m_Property.m_Money < m_MyBox.m_dwMoney) || (pDestPlayer->m_Property.m_Money < m_YouBox.m_dwMoney))
		return;
	
	bool succeed = false;
	// ����Ҫȷ���������ٽ�����һ�����ߣ�����һ��Ǯ
	if (0 == m_MyBox.m_dwMoney && 0 == m_YouBox.m_dwMoney && !m_MyBox.m_SellNumber && !m_YouBox.m_SellNumber)
		goto goto_finish;
	else
	{
		// ���齻�׷����ܷ����ɽ�Ǯ
		DWORD canGet = 0xffffffff - pSrcPlayer->m_Property.m_Money + m_MyBox.m_dwMoney;
		if (canGet < m_YouBox.m_dwMoney)
		{
			TalkToDnid(pSrcPlayer->m_ClientIndex, "��Ǯ��̫���ˣ�");
			TalkToDnid(pDestPlayer->m_ClientIndex, "�Է�Ǯ��̫���ˣ�");

			// ���׷����޷����ɽ�Ǯ
			pSrcPlayer->ExchangeCancel(pDestPlayer, isSrc);
			return;
		}

		// ��⽻��Ŀ�ķ��ܷ����ɽ�Ǯ
		canGet = 0xffffffff - pDestPlayer->m_Property.m_Money + m_YouBox.m_dwMoney;
		if (canGet < m_MyBox.m_dwMoney)
		{
			TalkToDnid(pSrcPlayer->m_ClientIndex, "�Է�Ǯ��̫���ˣ�");
			TalkToDnid(pDestPlayer->m_ClientIndex, "��Ǯ��̫���ˣ�");

			// ����Ŀ�ķ��޷����ɽ�Ǯ
			pSrcPlayer->ExchangeCancel(pDestPlayer, isSrc);
			return;
		}

		std::list<SAddItemInfo> itemList;

		// ��齻��Ŀ����ҵı���״��
		for (int i=0; i<m_YouBox.m_SellNumber; i++)
			itemList.push_back(SAddItemInfo(m_YouBox.m_GoodsArray[i].wIndex, m_YouBox.m_GoodsArray[i].overlap));

		if (itemList.size() && !pSrcPlayer->CanAddItems(itemList, m_MyBox.m_SellNumber))
		{
			// ���׷��𷽵ı����޷����㽻��Ŀ�ĵ���
			pSrcPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_SELF_PACKAGE_LACK);
			pDestPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_DEST_PACKAGE_LACK);

			pSrcPlayer->ExchangeCancel(pDestPlayer, isSrc);
			return;
		}
		
		itemList.clear();
		
		for (int i=0; i<m_MyBox.m_SellNumber; i++)
			itemList.push_back(SAddItemInfo(m_MyBox.m_GoodsArray[i].wIndex, m_MyBox.m_GoodsArray[i].overlap));
		
		if (itemList.size() && !pDestPlayer->CanAddItems(itemList, m_YouBox.m_SellNumber))
		{
			// ����Ŀ�ĵı����޷����㽻��Ŀ�ĵ���
			pSrcPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_DEST_PACKAGE_LACK);
			pDestPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_SELF_PACKAGE_LACK);

			pSrcPlayer->ExchangeCancel(pDestPlayer, isSrc);
			return;
		}
	
	
		succeed = true; //�������ɹ���λ���㹻�����Կ�ʼ���ף�����ɾ������
		for (int i=0; i<m_MyBox.m_SellNumber; i++)
		{
			SPackageItem *pItem = pSrcPlayer->FindItemByPos(m_MyBox.m_GoodsArray[i].wCellPos, XYD_FT_ONLYLOCK);
			if (!pItem)
			{
				rfalse(4, 1, "ExchangeGoods.cpp - CommitExchange() - !pItem - 2");
				return;
			}
			MY_ASSERT(pItem);

			pSrcPlayer->DelItem(*pItem, "����ɾ��", true);
		}
	
		for (int i=0; i<m_YouBox.m_SellNumber; i++)
		{
			SPackageItem *pItem = pDestPlayer->FindItemByPos(m_YouBox.m_GoodsArray[i].wCellPos, XYD_FT_ONLYLOCK);
			if (!pItem)
			{
				rfalse(4, 1, "ExchangeGoods.cpp - CommitExchange() - !pItem");
				return;
			}
			MY_ASSERT(pItem);

			pDestPlayer->DelItem(*pItem, "����ɾ��", true);
		}
		
		// �����׷�����ӵ���
		for (int i=0; i<m_YouBox.m_SellNumber; i++)
		{
			const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(m_YouBox.m_GoodsArray[i].wIndex);
			if (!pData)
			{
				rfalse(4, 1, "ExchangeGoods.cpp - CommitExchange() - !pData");
				return;
			}
			MY_ASSERT(pData);
		
			WORD willCreate = pSrcPlayer->TryPossibleMerge(m_YouBox.m_GoodsArray[i].wIndex, m_YouBox.m_GoodsArray[i].overlap, pData->m_Overlay);
			pSrcPlayer->InsertItem(m_YouBox.m_GoodsArray[i].wIndex, willCreate, pData, m_YouBox.m_GoodsArray[i], false);
		}

		// ������Ŀ�ķ���ӵ���
		for (int i=0; i<m_MyBox.m_SellNumber; i++)
		{
			const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(m_MyBox.m_GoodsArray[i].wIndex);
			if (!pData)
			{
				rfalse(4, 1, "ExchangeGoods.cpp - CommitExchange() - !pData - 2");
			}
			MY_ASSERT(pData);

			WORD willCreate = pDestPlayer->TryPossibleMerge(m_MyBox.m_GoodsArray[i].wIndex, m_MyBox.m_GoodsArray[i].overlap, pData->m_Overlay);
			pDestPlayer->InsertItem(m_MyBox.m_GoodsArray[i].wIndex, willCreate, pData, m_MyBox.m_GoodsArray[i], false);
		}

		if (m_MyBox.m_dwMoney || m_YouBox.m_dwMoney)
		{
			pSrcPlayer->m_Property.m_Money  -= m_MyBox.m_dwMoney;
			pDestPlayer->m_Property.m_Money -= m_YouBox.m_dwMoney;

			pSrcPlayer->m_Property.m_Money	+= m_YouBox.m_dwMoney;
			pDestPlayer->m_Property.m_Money += m_MyBox.m_dwMoney;
	
			pSrcPlayer->m_PlayerPropertyStatus[XA_UNBIND_MONEY-XA_MAX_EXP] = true;
			pDestPlayer->m_PlayerPropertyStatus[XA_UNBIND_MONEY-XA_MAX_EXP] = true;
		}
	}
	
goto_finish:
	// ������ɣ������ǰ�Ľ���״̬
	memset((CExchangeGoods*)pSrcPlayer, 0, sizeof(CExchangeGoods));
	memset((CExchangeGoods*)pDestPlayer, 0, sizeof(CExchangeGoods));

	// ֪ͨ�ͻ��˽��׳ɹ���ɣ����Թرս��״�����
	SAExchangeOperationMsg opmsg;
	opmsg.dwSrcGID	= pSrcPlayer->GetGID();
	opmsg.dwDestGID = pDestPlayer->GetGID();
	opmsg.isSrc		= isSrc;
	opmsg.operation = SAExchangeOperationMsg::SUCCEED;
	g_StoreMessage(pSrcPlayer->m_ClientIndex, &opmsg, sizeof(SAExchangeOperationMsg));
	g_StoreMessage(pDestPlayer->m_ClientIndex, &opmsg, sizeof(SAExchangeOperationMsg));


	//���ͽ��׳ɹ��󱣴�����
	if (!succeed) return;

	// ������ҵ���
	SPackageItem *pItems = pSrcPlayer->m_Property.m_BaseGoods;
	memset(pItems, 0, PackageAllCells * sizeof(SPackageItem));
	for (DWORD i = 0; i < PackageAllCells; i++, pItems++)
	{
		SPackageItem *curItem = pSrcPlayer->FindItemByPos(i, XYD_FT_WHATEVER);
		if (curItem)
			*pItems = *curItem;
	}


	SPackageItem *pdestItems = pDestPlayer->m_Property.m_BaseGoods;
	memset(pdestItems, 0, PackageAllCells * sizeof(SPackageItem));
	for (DWORD i = 0; i < PackageAllCells; i++, pdestItems++)
	{
		SPackageItem *curItem = pDestPlayer->FindItemByPos(i, XYD_FT_WHATEVER);
		if (curItem)
			*pdestItems = *curItem;
	}

	SDoubleSavePlayerMsg sdsavemsg;
	dwt::strcpy(sdsavemsg.account[0],pSrcPlayer->GetAccount(),MAX_ACCOUNT);
    dwt::strcpy(sdsavemsg.account[1],pDestPlayer->GetAccount(),MAX_ACCOUNT);
	sdsavemsg.gid[0] = pSrcPlayer->GetGID();
	sdsavemsg.gid[1] = pDestPlayer->GetGID();
	
	SFixProperty *pSreData = dynamic_cast<SFixProperty *>(&pSrcPlayer->m_Property);
	SFixProperty *pDestData =dynamic_cast<SFixProperty *>(&pDestPlayer->m_Property);
	memcpy(&sdsavemsg.data[0],pSreData,sizeof(SFixProperty));
	memcpy(&sdsavemsg.data[1],pDestData,sizeof(SFixProperty));
	int num = SectionMessageManager::getInstance().evaluateDevidedAmount(sizeof(sdsavemsg));
	int id = 0;
	BOOL ret;
	for (int i = 0;i < num;i++)
	{
		SSectionMsg sMsg;
		id = SectionMessageManager::getInstance().devideMessage(i,num,&sMsg,&sdsavemsg,sizeof(sdsavemsg),id);
		ret = SendToLoginServer(&sMsg,sizeof(SSectionMsg));
	}
	//SectionMessageManager::getInstance().
	//SendToLoginServer(&sdsavemsg,sizeof(SDoubleSavePlayerMsg));
//	g_StoreMessage(pSrcPlayer->m_ClientIndex,&sdsavemsg,sizeof(SDoubleSavePlayerMsg));
	

	return;
}

void CExchangeGoods::TheEnd()
{
	if (0 == m_dwOtherPlayerGID)
        return;

	CPlayer *pPlayer = static_cast<CPlayer *>(this);
	if (!pPlayer)
		return;

    CPlayer *pSrcPlayer  = 0;
    CPlayer *pDestPlayer = 0;
    bool isSrc = false;

	// ����Է��ǽ���Ŀ��
	if (m_dwOtherPlayerGID == m_dwDestPlayerGID)
    {
        isSrc = true;
        pSrcPlayer = pPlayer;
        pDestPlayer = (CPlayer *)GetPlayerByGID(m_dwDestPlayerGID)->DynamicCast(IID_PLAYER);
		if (!pDestPlayer || !pDestPlayer->InExchange())
			return;

		if (!pDestPlayer->m_CurHp)
		{
			rfalse(4, 1, "ExchangeGoods.cpp - TheEnd() - !pDestPlayer->m_CurHp");
			return;
		}

		MY_ASSERT(pDestPlayer->m_CurHp);
    }
    else if (m_dwOtherPlayerGID == m_dwSrcPlayerGID)
    {
        pDestPlayer = pPlayer;
        pSrcPlayer = (CPlayer *)GetPlayerByGID(m_dwSrcPlayerGID)->DynamicCast(IID_PLAYER);
		if (!pSrcPlayer || !pSrcPlayer->InExchange())
			return;

		if (!pDestPlayer->m_CurHp)
		{
			rfalse(4, 1, "ExchangeGoods.cpp - TheEnd() - !pSrcPlayer->m_CurHp");
			return;
		}

		MY_ASSERT(pSrcPlayer->m_CurHp);
    }
 
	pSrcPlayer->ExchangeCancel(pDestPlayer, isSrc);
}