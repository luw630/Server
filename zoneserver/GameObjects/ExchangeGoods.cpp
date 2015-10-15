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

	// 首先看看是否在等待交易回应
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

	// 是否在等待响应交易请求
	if (pPlayer->m_ChxGetIn.size())
	{
		CPlayer::CHXQuestList tempCopy(pPlayer->m_ChxGetIn);

		for (CPlayer::CHXQuestList::iterator it = tempCopy.begin(); it != tempCopy.end(); ++it)
		{
			CPlayer *pQuest = (CPlayer *)GetPlayerByGID(it->first)->DynamicCast(IID_PLAYER);
			if (pQuest)
			{
				MY_ASSERT(pQuest->m_CurHp);

				// 超时，拒绝
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

	// 是否处于交易中
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

	// 被请求目标传回选择，是否接受交易
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

	// 移除引用
	pQuest->m_ChxSentOut.erase(QuestIt);
	pPlayer->m_ChxGetIn.erase(ReplyIt);

	// 进入具体交易部分，所有的交易都由交易发起人来处理
	if (pMsg->bAccept)
    {
		if (0 == pPlayer->m_CurHp || 0 == pQuest->m_CurHp)
			return;
		
		if (pQuest->InExchange() || pPlayer->InExchange())
			return;

		// 限制距离
		DWORD distance = sqrt(pow((pQuest->m_curX-pPlayer->m_curX),2)+pow((pQuest->m_curY-pPlayer->m_curY), 2));
		if (distance > 500)
			return;			// 距离过远
	
		// 对方同意交易
		pQuest->StartExchange(pPlayer);
    }
	else
	{
		if (false == Auto && 0 == pPlayer->m_CurHp)
			return;

		// 交易被拒绝
		TalkToDnid(pQuest->m_ClientIndex, "您的交易请求被拒绝！");
	}
}

void CExchangeGoods::RecvPutItemToExchangeBoxMsg(struct SQPutItemToExchangeBoxMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "ExchangeGoods.cpp - RecvPutItemToExchangeBoxMsg() - !pMsg");
		return;
	}

	// 一个交易方拖入了一个物品
	CPlayer *pPlayer = static_cast<CPlayer *>(this);
	if (!pPlayer)
		return;

    CPlayer *pSrcPlayer = 0;
    CPlayer *pDestPlayer = 0;

	bool isSrc = false;
	
	if (!pPlayer->InExchange())
		return;

    // 找到两个交易人
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

    // 判断是否是确定的交易双方
    if ((pSrcPlayer->m_dwSrcPlayerGID != pSrcPlayer->GetGID())   ||
        (pSrcPlayer->m_dwDestPlayerGID != pDestPlayer->GetGID()) ||
        (pSrcPlayer->m_dwOtherPlayerGID != pDestPlayer->GetGID()))
        return;

	MY_ASSERT(pSrcPlayer->m_CurHp && pDestPlayer->m_CurHp);

    // 进入具体交易部分，所有的交易都由交易发起人来处理
    pSrcPlayer->PutItemToExchangeBox(pDestPlayer, pMsg->Item, pMsg->wCellX, pMsg->wCellY, isSrc);
}

void CExchangeGoods::RecvRemoteExchangeOperationMsg( struct SQAQuestRemoteExchangeMsg *pMsg )
{
	if (!pMsg)
	{
		rfalse(4, 1, "ExchangeGoods.cpp - RecvRemoteExchangeOperationMsg() - !pMsg");
		return;
	}

    // 远程交易思路，为每一个角色添加一个商贸信息表，map<DWORD, DWORD>( itemIdx, time )
    // 只要是发布过交易信息的，就会在这个表里边添加一条信息（同时清除已经过期的信息）
    // 交易中的操作（确认、取消）
    CPlayer *src = static_cast<CPlayer *>(this);
    if (src == NULL)
        return;

    // 这个逻辑是客户端发起远程交易请求
    if ( pMsg->dnidClient == 0 )
    {
        CPlayer *dst = ( CPlayer* )GetPlayerByName( pMsg->name )->DynamicCast( IID_PLAYER );

        if ( dst == NULL || dst->m_ClientIndex == 0 || dst == src )
        { 
            TalkToDnid( src->m_ClientIndex, "远程交易目标玩家不在线！" );
            return;
        }

        //正在游戏大厅
        if ( src->IsBetting() || dst->IsBetting() )
        {
            TalkToDnid( src->m_ClientIndex, src->IsBetting() ? 
                "游戏大厅中你不能进行交易……" : "对方正在游戏大厅中，没办法和阁下交易……" );
            return;
        }

        if ( src->m_ParentRegion == NULL || dst->m_ParentRegion == NULL ||
            src->m_ParentRegion->DynamicCast( IID_DYNAMICREGION ) || dst->m_ParentRegion->DynamicCast( IID_DYNAMICREGION ) )
        {
            TalkToDnid( src->m_ClientIndex, "当角色位于动态场景时，不能使用远程交易" );
            return;
        }

        if ( src->InExchange() || dst->InExchange() )
        {
            TalkToDnid( src->m_ClientIndex, src->InExchange() ? 
                "当前状态限制你不能进行交易……" : "对方正在交易中，没办法和阁下交易……" );
            return;
        }

//         if ( src->GetSaleState() != 0 || dst->GetSaleState() )
//         {
//             TalkToDnid( src->m_ClientIndex, ( src->GetSaleState() != 0 ) ? 
//                 "阁下正在摆摊中，不能进行交易……" : "对方正在摆摊中，没办法和阁下交易……" );
//             return;
//         }

        std::map< DWORD, time_t >::iterator it = dst->excInfoMap.find( pMsg->item );
        if ( it == dst->excInfoMap.end() || abs( ( int )( time( NULL ) - it->second ) ) > 5*60 )
        { 
            TalkToDnid( src->m_ClientIndex, "你请求的信息已经过期！" );
            return;
        }

        SQAQuestRemoteExchangeMsg msg;
        msg.dnidClient = src->m_ClientIndex;
        msg.dwSrcGID = src->GetGID();
        msg.dwDestGID = dst->GetGID();
        msg.item = pMsg->item;
        dwt::strcpy( msg.name, src->GetName(), sizeof( msg.name ) );
        if ( g_StoreMessage( dst->m_ClientIndex, &msg, sizeof( msg ) ) )
            TalkToDnid( src->m_ClientIndex, "已经向目标转达了你的交易请求！" );
        else
            TalkToDnid( src->m_ClientIndex, "向目标转达交易请求的过程中出现错误！" );
    }
    else
    {
        CPlayer *dst = ( CPlayer* )GetPlayerByDnid( pMsg->dnidClient )->DynamicCast( IID_PLAYER );
        if ( dst == NULL || dst->m_ClientIndex == 0 || dst == src )
        { 
            TalkToDnid( src->m_ClientIndex, "远程交易目标玩家不在线！" );
            return;
        }

        if ( pMsg->item == 0 || pMsg->dwSrcGID != src->GetGID() || pMsg->dwDestGID != dst->GetGID() )
            TalkToDnid( src->m_ClientIndex, FormatString( "%s拒绝了你的交易请求！", dst->GetName() ) );
        else
        {
            if ( src->m_ParentRegion == NULL || dst->m_ParentRegion == NULL ||
                src->m_ParentRegion->DynamicCast( IID_DYNAMICREGION ) || dst->m_ParentRegion->DynamicCast( IID_DYNAMICREGION ) )
            {
                TalkToDnid( src->m_ClientIndex, "当角色位于动态场景时，不能使用远程交易" );
                return;
            }

            std::map< DWORD, time_t >::iterator it = src->excInfoMap.find( pMsg->item );
            if ( it == src->excInfoMap.end() || abs( ( int )( time( NULL ) - it->second ) ) > 5*60 )
                TalkToDnid( src->m_ClientIndex, "你响应的远程交易请求已经过期！" );
            else
                StartExchange( dst ); // 尝试启动交易
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

	// 交易中的操作（锁定、确认、取消）
	CPlayer *pPlayer = static_cast<CPlayer *>(this);
	if (!pPlayer)
        return;

	CPlayer *pSrcPlayer  = 0;
    CPlayer *pDestPlayer = 0;

	bool isSrc = false;

	// 如果交易已经结束或者本身就没开始
	if (!pPlayer->InExchange())
		return;

	// 找到两个交易人
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

    // 判断是否是确定的交易双方
    if ((pSrcPlayer->m_dwSrcPlayerGID   != pSrcPlayer->GetGID())  ||
        (pSrcPlayer->m_dwDestPlayerGID  != pDestPlayer->GetGID()) ||
        (pSrcPlayer->m_dwOtherPlayerGID != pDestPlayer->GetGID()))
        return;

	MY_ASSERT(pSrcPlayer->m_CurHp && pDestPlayer->m_CurHp);

	// 进入具体交易部分，所有的交易都由交易发起人来处理
	if (pMsg->operation == SQExchangeOperationMsg::LOCK || pMsg->operation == SQExchangeOperationMsg::UNLOCK)
	{
		// 有一方打算锁定/解锁定
		pSrcPlayer->LockExchange(pDestPlayer, isSrc, pMsg->operation == SQExchangeOperationMsg::LOCK ? true : false);
	}
    else if (pMsg->operation == SQExchangeOperationMsg::COMMIT)
	{
		// 有一方打算交易确认
        pSrcPlayer->CommitExchange(pDestPlayer, isSrc);
	}
	else if (pMsg->operation == SQExchangeOperationMsg::CANCEL)
	{
		// 有一方打算取消交易
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

	// 如果已经点击了交易，则无法进行锁定/解锁定
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

	// 通知交易双方，有一个人已经锁定/解锁了
	SAExchangeOperationMsg opmsg;
	opmsg.dwSrcGID	= pSrcPlayer->GetGID();
	opmsg.dwDestGID = pDestPlayer->GetGID();
	opmsg.isSrc		= isSrc;

	if (isLock)
	{
		// 如果已经锁定了，直接返回
		if (pExchangeBox->m_bLocked)
			return;

		pExchangeBox->m_bLocked = true;
		opmsg.operation = SAExchangeOperationMsg::LOCK;
	}
	else
	{
		// 如果没有锁定，直接返回
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
	// 只有玩家可以进行交易
	CPlayer *pPlayer = static_cast<CPlayer *>(this);
	if (!pPlayer)
        return;

	if (0 == dwGID || pPlayer->GetGID() == dwGID)
		return;

	if (0 == pPlayer->m_CurHp)
		return;

	if (pPlayer->InExchange())
    {
		// 自己正处于交易中
		pPlayer->SendStatusMsg(SANotifyStatus::NS_EXG_SELFINEXGING);
        return;
    }

	CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
	if (!pDestPlayer || 0 == pDestPlayer->m_CurHp)
    {
		// 交易对象不存在/翘辫子了
		pPlayer->SendStatusMsg(SANotifyStatus::NS_EXG_CANTFIND_DEST);
        return;
    }

	if (pDestPlayer->InExchange())
    {
		// 交易对象正处于交易中
		pPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_DESTEXGING);
        return;
	}

	// 是否已经发出过邀请了
	CPlayer::CHXQuestList::iterator hasSendOut = pPlayer->m_ChxSentOut.find(pDestPlayer->GetGID());
	if (hasSendOut != pPlayer->m_ChxSentOut.end())
		return;

	// 限制距离
	DWORD distance = sqrt(pow((pPlayer->m_curX-pDestPlayer->m_curX),2)+pow((pPlayer->m_curY-pDestPlayer->m_curY), 2));
	if (distance > 500)
		return;

	SAQuestExchangeMsg msg;
    msg.dnidClient	= pPlayer->m_ClientIndex;
    msg.dwSrcGID	= pPlayer->GetGID();
    msg.dwDestGID	= pDestPlayer->GetGID();
    msg.bAccept		= false;	// 为假，也就是交易请求的状态
    g_StoreMessage(pDestPlayer->m_ClientIndex, &msg, sizeof(SAQuestExchangeMsg));

	pPlayer->m_ChxSentOut[pDestPlayer->GetGID()] = pDestPlayer->m_ChxGetIn[pPlayer->GetGID()] = timeGetTime();
	
    // 通知发起玩家等待对方的确认
	pPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_WAITDEST_RESPOND);
}

void CExchangeGoods::StartExchange(CPlayer *pDestPlayer)
{
	CPlayer *pPlayer = static_cast<CPlayer *>(this);
	if (!pPlayer || 0 == pPlayer->m_CurHp)
		return;

	MY_ASSERT(!pPlayer->InExchange() && !pDestPlayer->InExchange());

	// 通过各种检测，初始化交易状态，特别要注明的是，需要设定交易双方的ID
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

	// 同时通知双方打开交易面板
    SAQuestExchangeMsg msg;
    msg.dnidClient	= pPlayer->m_ClientIndex;
    msg.dwSrcGID	= pPlayer->GetGID();
    msg.dwDestGID	= pDestPlayer->GetGID();
    msg.bAccept		= true;		// 为真，也就是打开交易面板
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

	// 如果已锁定或者已提交，则无法继续添加道具
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

    // 是否是交易金钱
	if (0xffff == item.wIndex)
    {
		if (0 == item.details.uniqueTimestamp || item.details.uniqueTimestamp > 99999999)
		{
			pPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_MONEY_INVALID);
			return;
		}

		if (item.details.uniqueTimestamp > pPlayer->m_Property.m_Money)
            return;

        // 通过检测，更新交易状态
        pExchangeBox->m_dwMoney = item.details.uniqueTimestamp;
		pPlayer->m_IsMoneyLocked = true;	// 此处锁定金钱
		bSuccess = true;
    }
    else
    {
		// 判定目标道具的存在性
		SPackageItem *pItem = pPlayer->FindItemByPos(item.wCellPos, XYD_FT_ONLYUNLOCK);
		
		if (!pItem || pItem->wIndex != msg.Item.wIndex)
            return;

		const SItemBaseData *pItemData = CItemService::GetInstance().GetItemBaseData(item.wIndex);
		if (!pItemData || ITEM_IS_TASK(pItemData->m_Type))	// 任务道具不可交易
			return;

		// 判定道具是否可以被交易
		if (!pItemData->m_CanTran)
		{
			TalkToDnid(pPlayer->m_ClientIndex, "该道具无法被交易！");
			pPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_ITEM_CANTEXG);
			return;
		}
		if (item.IsBindState())
		{
			TalkToDnid(pPlayer->m_ClientIndex, "该道具已绑定，不能被交易。");
			pPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_ITEM_BIND);
			return ;
		}

		// 交易栏是否足够
		if (MAX_EXCHANGE_ITEM_NUMBER == pExchangeBox->m_SellNumber)
		{
			TalkToDnid(pPlayer->m_ClientIndex, "交易栏已满！");
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

	// 交易取消，需要恢复被锁定的道具和金钱
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

	// 清除当前的交易状态（包括了金钱的锁定）
	memset((CExchangeGoods *)pSrcPlayer, 0, sizeof(CExchangeGoods));
	memset((CExchangeGoods *)pDestPlayer, 0, sizeof(CExchangeGoods));

	// 通知客户端交易被取消
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

	// 还没有确认
	if (!(m_MyBox.m_bLocked && m_YouBox.m_bLocked))
	{
		TalkToDnid(pPlayer->m_ClientIndex, "一方或双方没有锁定，无法提交交易");
		return;
	}

	if (pExchangeBox->m_bCommit)
		return;

    pExchangeBox->m_bCommit = true;

	if (!(m_MyBox.m_bCommit && m_YouBox.m_bCommit))
    {
		TalkToDnid(pPlayer->m_ClientIndex, "对方还没有提交，请等待");

		// 通知交易双方，有一个人已经确认了
		SAExchangeOperationMsg opmsg;
        opmsg.dwSrcGID	= pSrcPlayer->GetGID();
        opmsg.dwDestGID = pDestPlayer->GetGID();
        opmsg.isSrc		= isSrc;
        opmsg.operation = SAExchangeOperationMsg::COMMIT;

		g_StoreMessage(pSrcPlayer->m_ClientIndex, &opmsg, sizeof(SAExchangeOperationMsg));
		g_StoreMessage(pDestPlayer->m_ClientIndex, &opmsg, sizeof(SAExchangeOperationMsg));
        return;
    }

	// 双方都已确认，可以开始交易了，检测金钱的合法性
	MY_ASSERT(pSrcPlayer->m_Property.m_Money >= m_MyBox.m_dwMoney);
	MY_ASSERT(pDestPlayer->m_Property.m_Money >= m_YouBox.m_dwMoney);

	if ((pSrcPlayer->m_Property.m_Money < m_MyBox.m_dwMoney) || (pDestPlayer->m_Property.m_Money < m_YouBox.m_dwMoney))
		return;
	
	bool succeed = false;
	// 这里要确认他们至少交换过一个道具，或者一分钱
	if (0 == m_MyBox.m_dwMoney && 0 == m_YouBox.m_dwMoney && !m_MyBox.m_SellNumber && !m_YouBox.m_SellNumber)
		goto goto_finish;
	else
	{
		// 检验交易发起方能否容纳金钱
		DWORD canGet = 0xffffffff - pSrcPlayer->m_Property.m_Money + m_MyBox.m_dwMoney;
		if (canGet < m_YouBox.m_dwMoney)
		{
			TalkToDnid(pSrcPlayer->m_ClientIndex, "您钱包太满了！");
			TalkToDnid(pDestPlayer->m_ClientIndex, "对方钱包太满了！");

			// 交易发起方无法容纳金钱
			pSrcPlayer->ExchangeCancel(pDestPlayer, isSrc);
			return;
		}

		// 检测交易目的方能否容纳金钱
		canGet = 0xffffffff - pDestPlayer->m_Property.m_Money + m_YouBox.m_dwMoney;
		if (canGet < m_MyBox.m_dwMoney)
		{
			TalkToDnid(pSrcPlayer->m_ClientIndex, "对方钱包太满了！");
			TalkToDnid(pDestPlayer->m_ClientIndex, "你钱包太满了！");

			// 交易目的方无法容纳金钱
			pSrcPlayer->ExchangeCancel(pDestPlayer, isSrc);
			return;
		}

		std::list<SAddItemInfo> itemList;

		// 检查交易目的玩家的背包状况
		for (int i=0; i<m_YouBox.m_SellNumber; i++)
			itemList.push_back(SAddItemInfo(m_YouBox.m_GoodsArray[i].wIndex, m_YouBox.m_GoodsArray[i].overlap));

		if (itemList.size() && !pSrcPlayer->CanAddItems(itemList, m_MyBox.m_SellNumber))
		{
			// 交易发起方的背包无法满足交易目的道具
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
			// 交易目的的背包无法满足交易目的道具
			pSrcPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_DEST_PACKAGE_LACK);
			pDestPlayer->SendStatusMsg(SANotifyStatus::NS_EXT_SELF_PACKAGE_LACK);

			pSrcPlayer->ExchangeCancel(pDestPlayer, isSrc);
			return;
		}
	
	
		succeed = true; //背包检测成功，位置足够，可以开始交易，首先删除道具
		for (int i=0; i<m_MyBox.m_SellNumber; i++)
		{
			SPackageItem *pItem = pSrcPlayer->FindItemByPos(m_MyBox.m_GoodsArray[i].wCellPos, XYD_FT_ONLYLOCK);
			if (!pItem)
			{
				rfalse(4, 1, "ExchangeGoods.cpp - CommitExchange() - !pItem - 2");
				return;
			}
			MY_ASSERT(pItem);

			pSrcPlayer->DelItem(*pItem, "交易删除", true);
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

			pDestPlayer->DelItem(*pItem, "交易删除", true);
		}
		
		// 给交易发起方添加道具
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

		// 给交易目的放添加道具
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
	// 交易完成，清除当前的交易状态
	memset((CExchangeGoods*)pSrcPlayer, 0, sizeof(CExchangeGoods));
	memset((CExchangeGoods*)pDestPlayer, 0, sizeof(CExchangeGoods));

	// 通知客户端交易成功完成，可以关闭交易窗口了
	SAExchangeOperationMsg opmsg;
	opmsg.dwSrcGID	= pSrcPlayer->GetGID();
	opmsg.dwDestGID = pDestPlayer->GetGID();
	opmsg.isSrc		= isSrc;
	opmsg.operation = SAExchangeOperationMsg::SUCCEED;
	g_StoreMessage(pSrcPlayer->m_ClientIndex, &opmsg, sizeof(SAExchangeOperationMsg));
	g_StoreMessage(pDestPlayer->m_ClientIndex, &opmsg, sizeof(SAExchangeOperationMsg));


	//发送交易成功后保存数据
	if (!succeed) return;

	// 保存玩家道具
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

	// 如果对方是交易目标
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