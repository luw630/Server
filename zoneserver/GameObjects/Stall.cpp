#include "StdAfx.h"
#include "stall.h"
#include "Item.h"
#include "player.h"
#include "Area.h"
#include "region.h"

#include "GlobalFunctions.h"

#include "Networkmodule\RegionMsgs.h"
#include "Networkmodule\ItemMsgs.h"
#include "Networkmodule\ScriptMsgs.h"
#include "CItemDefine.h"
#include "CItemService.h"

extern LPIObject GetPlayerByGID(DWORD);
extern float FindRgionInRateMap(DWORD dwRegionID);
extern void AddMoneyIfINRMap(DWORD dwRegionID, DWORD dwValue);

CStall::CStall(void)
{
	memset(m_StallCells,	0, sizeof(m_StallCells));
	memset(m_StallItems,	0, sizeof(m_StallItems));
	memset(szStallName,		0, sizeof(szStallName));
	memset(szSaleInfo,		0, sizeof(szSaleInfo));

	m_ForSaleNumber = 0;
	ItemUpdateTime	= 0;
	m_LogIndex		= 0;
	dwOtherGID		= 0;
}

CStall::~CStall(void)
{
}

void CStall::RecvChangeStallName(struct SQChangeStallNameMsg *pMsg)
{
	CPlayer *pPlayer = (CPlayer *)(this);
	if (!pPlayer)
		return;

	if (!pPlayer->IsInStall())
		return;
	
	pMsg->szStallName[MAX_STALLNAME-1] = '\0';

	dwt::strcpy(szStallName, pMsg->szStallName, MAX_STALLNAME);

	// 同步摊位信息给周围的玩家
	SASendSaleNameMsg NameMsg;
	NameMsg.dwGlobalID = pPlayer->GetGID();
	dwt::strcpy(NameMsg.szStallName, szStallName, MAX_STALLNAME);
	
	if (pPlayer->m_ParentArea)
		pPlayer->m_ParentArea->SendAdj(&NameMsg, sizeof(SASendSaleNameMsg), -1);

	return;
}

void CStall::RecvChangeStallInfo(struct SQChangeStallInfoMsg *pMsg)
{
	CPlayer *pPlayer = (CPlayer *)(this);
	if (!pPlayer)
		return;

	if (!pPlayer->IsInStall())
		return;

	pMsg->szStallInfo[MAX_SALEINFO-1] = '\0';

	dwt::strcpy(szSaleInfo, pMsg->szStallInfo, MAX_SALEINFO);

	// 回馈玩家
	SAChangeStallInfoMsg msg;
	msg.dwGlobalID = pPlayer->GetGID();
	dwt::strcpy(msg.szStallInfo, szSaleInfo, MAX_SALEINFO);
	g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));

	// 同步给所有观察者
	for (PLAYERMAP::iterator it = m_CurPlayerMap.begin(); it != m_CurPlayerMap.end(); ++it)
		g_StoreMessage(it->second, &msg, sizeof(msg));
}

void CStall::RecvChangePrice(struct SQChangeItemPriceMsg *pMsg)
{
	CPlayer *pPlayer = (CPlayer *)this;
	if (!pPlayer)
		return;
	
	WORD pos = pMsg->bPos;

	if (pos >= MAX_SALEITEMNUM)
		return;

	if (CheckPos(pos, 0))
	{
		MY_ASSERT(0 == m_StallItems[pos].bPrice && 0 == m_StallItems[pos].item.wIndex);
		return;
	}

	MY_ASSERT(0 != m_StallItems[pos].bPrice && 0 != m_StallItems[pos].item.wIndex);

	SaleItem *pItemInfo = &m_StallItems[pos];

	// 验证道具是否还在背包
	SPackageItem *pSrc = pPlayer->FindItemByPos(pItemInfo->item.wCellPos, XYD_FT_ONLYLOCK);
	MY_ASSERT(pSrc && (0 == memcmp(pSrc, &pItemInfo->item, sizeof(SPackageItem))));

	// 如果总价超过32位所能表示的最大值，则失败
	QWORD totalMoney = pMsg->newPrice * pSrc->overlap;
	if (totalMoney > 0xffffffff)
		return;

	pItemInfo->bPrice = pMsg->newPrice;

	SAUpdateSaleItemMsg msgInfo;
	msgInfo.stSaleItem = *pItemInfo;
	g_StoreMessage(pPlayer->m_ClientIndex, &msgInfo, sizeof(SAUpdateSaleItemMsg));

	if (!pPlayer->IsInStall())
		MY_ASSERT(m_CurPlayerMap.empty());

	for (PLAYERMAP::iterator it = m_CurPlayerMap.begin(); it != m_CurPlayerMap.end(); ++it)
		g_StoreMessage(it->second, &msgInfo, sizeof(SAUpdateSaleItemMsg));
}

void CStall::RecvCloseSaleItemMsg(SQCloseSaleItemMsg *pMsg)
{
	if (SQCloseSaleItemMsg::CS_SELF == pMsg->bOperation || SQCloseSaleItemMsg::CS_SELF_NOBACK == pMsg->bOperation)
	{
		CPlayer *pPlayer = (CPlayer *)this;
		if (!pPlayer)
			return;

		DWORD debug4Count = 0;

		if (SQCloseSaleItemMsg::CS_SELF == pMsg->bOperation)		// 首先退回所有的道具
		{
			for (size_t i=0; i<MAX_SALEITEMNUM; ++i)
			{
				SaleItem *pSale = &m_StallItems[i];
				if (0 == pSale->item.wIndex || 0 == pSale->bPrice)
					continue;
	
				MY_ASSERT(CheckPos(i, 1));
	
				SPackageItem *pItem = pPlayer->FindItemByPos(m_StallItems[i].item.wCellPos, XYD_FT_ONLYLOCK);
				MY_ASSERT(pItem && (0 == memcmp(pItem, &m_StallItems[i].item, sizeof(SPackageItem))));
				
				pPlayer->LockItemCell(m_StallItems[i].item.wCellPos, false);

				debug4Count++;
			}

			MY_ASSERT(debug4Count == m_ForSaleNumber);

			// 归零
			m_ForSaleNumber = 0;
			memset(m_StallCells, 0, sizeof(m_StallCells));
			memset(m_StallItems, 0, sizeof(m_StallItems));
		}

		if (pPlayer->IsInStall())
		{
			SACloseSaleItemMsg CloseMsg;
			CloseMsg.dwGID		= pPlayer->GetGID();	
			CloseMsg.bOperation	= SACloseSaleItemMsg::CSI_CLOSESEE;

			// 通知查看者关闭面板
			for (PLAYERMAP::iterator it = m_CurPlayerMap.begin(); it != m_CurPlayerMap.end(); ++it)
				g_StoreMessage(it->second, &CloseMsg, sizeof(SACloseSaleItemMsg));

			// 通知周围玩家收摊喽~
			CloseMsg.bOperation = SACloseSaleItemMsg::CSI_CLOSESTALL;
			CArea *pArea = (CArea*)pPlayer->m_ParentArea->DynamicCast(IID_AREA);
			if (pArea)
				pArea->SendAdj(&CloseMsg, sizeof(SACloseSaleItemMsg), -1);

			// 站立
			pPlayer->SetZazen(false);

			pPlayer->m_Property.m_ShowState &= ~PSE_STALL_SALE;
		}

		ItemUpdateTime		= 0;
		szStallName[0]		= 0;
		szSaleInfo[0]		= 0;
		dwOtherGID			= 0;
		m_LogIndex			= 0;
		m_Logs.clear();
		m_LogSendIndex.clear();
		m_CurPlayerMap.clear();
	}
	else
	{
		if (0 == dwOtherGID)
			return;

		CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwOtherGID)->DynamicCast(IID_PLAYER);
		if (!pDestPlayer)
			return;

		pDestPlayer->DelCurPlayerFromList(((CPlayer *)this)->GetGID());

		dwOtherGID = 0;
	}
}

void CStall::RecvSendSaleInfo(SQSendSaleInfoMsg *pMsg)
{
	CPlayer *pPlayer = (CPlayer *)(this);
	if (!pPlayer)
		return;

	// 执行验证
	if (pPlayer->m_Property.m_Level < 10)
		return;

	// 如果当前不是打坐，也不是站立，则状态不对
	if (EA_ZAZEN != pPlayer->GetCurActionID() && EA_STAND != pPlayer->GetCurActionID())
		return;

	// 此处还要验证所在的场景等等

	if (pPlayer->IsInStall())
	{
		MY_ASSERT(EA_ZAZEN == pPlayer->GetCurActionID());
		return;
	}

	// 验证已上架的货物是否依然存在
	DWORD count4Debug = 0;

	for (size_t i=0; i<MAX_SALEITEMNUM; ++i)
	{
		SaleItem *pItem = &m_StallItems[i];
		if (0 == pItem->bPrice)
			continue;

		MY_ASSERT(m_StallCells[i]);

		SPackageItem *pRawItem = pPlayer->FindItemByPos(pItem->item.wCellPos, XYD_FT_ONLYLOCK);
		MY_ASSERT(pRawItem && (0 == memcmp(&pItem->item, pRawItem, sizeof(SPackageItem))));

		count4Debug++;
	}

	MY_ASSERT(m_ForSaleNumber == count4Debug);

	// 保存摊位名和摊位说明
	pMsg->szStallName[MAX_STALLNAME-1] = '\0';
	pMsg->szStallInfo[MAX_SALEINFO -1] = '\0';

	dwt::strcpy(szStallName, pMsg->szStallName, MAX_STALLNAME);
	dwt::strcpy(szSaleInfo,  pMsg->szStallInfo, MAX_SALEINFO );

	if (EA_STAND == pPlayer->GetCurActionID())
	{
		pPlayer->SetZazen(true);					// 发送打坐消息
	}

	pPlayer->m_Property.m_ShowState |= PSE_STALL_SALE;

	// 同步摊位信息给周围的玩家（包括自己）
	SASendSaleNameMsg NameMsg;
	NameMsg.dwGlobalID = pPlayer->GetGID();
	dwt::strcpy(NameMsg.szStallName, szStallName, MAX_STALLNAME);
	pPlayer->m_ParentArea->SendAdj(&NameMsg, sizeof(SASendSaleNameMsg), -1);

	MY_ASSERT(m_CurPlayerMap.empty());

	return;
}

void CStall::DispatchSaleMessage(SSaleItemBaseMsg *pMsg)
{
    switch(pMsg->_protocol) {
        case SSaleItemBaseMsg::RPRO_MOVETO_ITEM:
            {
                RecvMoveSaleItemMsg((SQMovetoSaleBoxMsg *)pMsg);
            }break;
        case SSaleItemBaseMsg::EPRO_SEESALE_ITEM:
            {
                RecvSeeSaleItemMsg((SQSeeSaleItemMsg *)pMsg);
            }break;
        case SSaleItemBaseMsg::EPRO_BUYSALE_ITEM:
            {
                RecvBuySaleItemMsg((SQBuySaleItemMsg *)pMsg);
            }break;
        case SSaleItemBaseMsg::EPRO_CLOSESALE_ITEM:
            {
                RecvCloseSaleItemMsg((SQCloseSaleItemMsg *)pMsg);
            }break;
		case SSaleItemBaseMsg::EPRO_CHANGE_PRICE:
			{
				RecvChangePrice((SQChangeItemPriceMsg *)pMsg);
			}break;
        case SSaleItemBaseMsg::EPRO_SEND_SALEINFO:
            {
                RecvSendSaleInfo((SQSendSaleInfoMsg *)pMsg);
            }break;
		case SSaleItemBaseMsg::EPRO_SEND_MESSAGE:
			{
				RecvSendMessage((SQSendMessageMsg *)pMsg);
			}break;
		case SSaleItemBaseMsg::EPRO_CHANGE_NAME:
			{
				RecvChangeStallName((SQChangeStallNameMsg *)pMsg);
			}break;
		case SSaleItemBaseMsg::EPRO_CHANGE_INFO:
			{
				RecvChangeStallInfo((SQChangeStallInfoMsg *)pMsg);
			}break;
        default:
            return;
    }
}
 
void CStall::RecvSeeSaleItemMsg(SQSeeSaleItemMsg *pMsg)
{
	CPlayer *pPlayer = (CPlayer *)(this);
	if (!pPlayer)
		return;

	if (pPlayer->GetGID() != pMsg->stBothSides.dwSelfGlobalID)
		return;

	if (pPlayer->GetGID() == pMsg->stBothSides.dwOtherGlobalID)
		return;

	// 自己摆摊无法看别人的
	if (pPlayer->IsInStall())
		return;

	CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(pMsg->stBothSides.dwOtherGlobalID)->DynamicCast(IID_PLAYER);
	if (!pDestPlayer || 0 == pDestPlayer->m_CurHp || !pDestPlayer->IsInStall())
		return;

	MY_ASSERT(pDestPlayer->GetSaleItemNum() <= MAX_SALEITEMNUM);

	pDestPlayer->AddCurPlayertoList(pPlayer->GetGID(), pPlayer->m_ClientIndex);
	dwOtherGID = pDestPlayer->GetGID();

	SASeeSaleItemMsg msg;
	msg.stItemBaseInfo.dwOtherGlobalID	= pDestPlayer->GetGID();
	msg.stItemBaseInfo.dwSelfGlobalID	= pPlayer->GetGID();
	msg.stItemBaseInfo.ItemNumber		= 0;
	dwt::strcpy(msg.stItemBaseInfo.szStallInfo, pDestPlayer->szSaleInfo, MAX_SALEINFO);
	memset(&msg.stItemBaseInfo.stItemBaseInfo, 0, sizeof(msg.stItemBaseInfo.stItemBaseInfo));

	SaleItem *pSale = pDestPlayer->GetItemList();

	DWORD number = 0;

	for (size_t i=0; i<MAX_SALEITEMNUM; ++i)
	{
		if (0 == pSale[i].bPrice)
			continue;

		MY_ASSERT(0 != pSale[i].item.wIndex);

		MY_ASSERT(pDestPlayer->CheckPos(i, 1));
		MY_ASSERT(pSale[i].bPos == i);

		SPackageItem *pItem = pDestPlayer->FindItemByPos(pSale[i].item.wCellPos, XYD_FT_ONLYLOCK);
		MY_ASSERT(pItem && (0 == memcmp(pItem, &pSale[i].item, sizeof(SPackageItem))));

		msg.stItemBaseInfo.stItemBaseInfo[number++] = pSale[i];
	}

	MY_ASSERT(number == pDestPlayer->GetSaleItemNum());

	msg.stItemBaseInfo.ItemNumber = number;

	g_StoreMessage(pPlayer->m_ClientIndex, &msg, msg.MySize());

	// 发送留言
	if (pDestPlayer->m_Logs.empty())
		return;

	SASendAllMessageMsg logMsg;

	std::map<DWORD, DWORD>::iterator iter = pDestPlayer->m_LogSendIndex.find(pPlayer->GetGID());
	if (iter == pDestPlayer->m_LogSendIndex.end())
	{
		// 如果不存在，那么就发送所有数据，并记录在案
		logMsg.bNumber = pDestPlayer->m_Logs.size();
		int i = 0;
		for (std::list<StallLogInfo>::iterator infoIter = pDestPlayer->m_Logs.begin(); infoIter != pDestPlayer->m_Logs.end(); ++infoIter)
		{
			logMsg.logs[i++] = *infoIter;
		}
	}
	else
	{
		DWORD lastIndex   = iter->second;
		DWORD recordFirst = (*pDestPlayer->m_Logs.begin()).index;

		if (lastIndex < recordFirst)
		{
			logMsg.bNumber = pDestPlayer->m_Logs.size();
			int i = 0;
			for (std::list<StallLogInfo>::iterator infoIter = pDestPlayer->m_Logs.begin(); infoIter != pDestPlayer->m_Logs.end(); ++infoIter)
			{
				logMsg.logs[i++] = *infoIter;
			}
		}
		else
		{
			int i = 0;
			for (std::list<StallLogInfo>::iterator infoIter = pDestPlayer->m_Logs.begin(); infoIter != pDestPlayer->m_Logs.end(); ++infoIter)
			{
				if (infoIter->index <= lastIndex)
					continue;

				logMsg.logs[i++] = *infoIter;
			}

			logMsg.bNumber = i;
		}
	}

	pDestPlayer->m_LogSendIndex[pPlayer->GetGID()] = logMsg.logs[logMsg.bNumber-1].index;

	g_StoreMessage(pPlayer->m_ClientIndex, &logMsg, sizeof(logMsg));
}

void CStall::RecvBuySaleItemMsg(SQBuySaleItemMsg *pMsg)
{
	CPlayer *pPlayer = (CPlayer *)this;
	if (!pPlayer)
		return;

	if (0 == dwOtherGID || dwOtherGID != pMsg->stBuyItem.stBothSides.dwOtherGlobalID)
		return;

	if (pPlayer->GetGID() != pMsg->stBuyItem.stBothSides.dwSelfGlobalID)
		return;

	if (pPlayer->GetGID() == pMsg->stBuyItem.stBothSides.dwOtherGlobalID)
		return;

	CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwOtherGID)->DynamicCast(IID_PLAYER);
	if (!pDestPlayer || 0 == pDestPlayer->m_CurHp || !pDestPlayer->IsInStall())
		return;

	WORD pos = pMsg->stBuyItem.bPos;

	if (pos >= MAX_SALEITEMNUM)
		return;

	if (0 == pDestPlayer->m_ForSaleNumber)
		return;

	// 此位置没有货物
	if (pDestPlayer->CheckPos(pos, 0))
	{
		MY_ASSERT(0 == pDestPlayer->GetItemList()[pos].item.wIndex && 0 == pDestPlayer->GetItemList()[pos].bPrice);
		return;
	}

	MY_ASSERT(0 != pDestPlayer->GetItemList()[pos].item.wIndex && 0 != pDestPlayer->GetItemList()[pos].bPrice);

	// 获取道具
	SaleItem *pItemInfo = &pDestPlayer->GetItemList()[pos];
	
	// 验证道具是否还在背包
	SPackageItem *pSrc = pDestPlayer->FindItemByPos(pItemInfo->item.wCellPos, XYD_FT_ONLYLOCK);
	MY_ASSERT(pSrc && (0 == memcmp(pSrc, &pItemInfo->item, sizeof(SPackageItem))));

	// 验证数量是否正确
	if (0 == pMsg->stBuyItem.wNumber || pMsg->stBuyItem.wNumber > pSrc->overlap)
		return;

	const SItemBaseData *pItemData = CItemService::GetInstance().GetItemBaseData(pItemInfo->item.wIndex);
	if (!pItemData)
		return;

	// 只能使用非绑定
	DWORD needMoney = pMsg->stBuyItem.wNumber * pItemInfo->bPrice;
	if (pPlayer->m_Property.m_Money < needMoney)
	{
		TalkToDnid(pPlayer->m_ClientIndex, "您的金钱不足，无法买入！！");
		return;
	}

	// 检验背包
	std::list<SAddItemInfo> itemList;
	itemList.push_back(SAddItemInfo(pSrc->wIndex, pMsg->stBuyItem.wNumber));

	if (!pPlayer->CanAddItems(itemList))
	{
		TalkToDnid(pPlayer->m_ClientIndex, "您的背包不足，无法买入！！");
		return;
	}

	SPackageItem tempCreate = *pSrc;
	WORD willCreate = pPlayer->TryPossibleMerge(tempCreate.wIndex, pMsg->stBuyItem.wNumber, pItemData->m_Overlay);
	pPlayer->InsertItem(tempCreate.wIndex, willCreate, pItemData, tempCreate, false);
	
	if (pMsg->stBuyItem.wNumber == pSrc->overlap)			// 全购
	{
		pDestPlayer->CStall::m_StallCells[pos] = 0;
		memset(&pDestPlayer->GetItemList()[pos], 0, sizeof(SaleItem));

		pDestPlayer->m_ForSaleNumber--;

		SADelSaleItemMsg delMsg;
		delMsg.bPos = pos;
		g_StoreMessage(pDestPlayer->m_ClientIndex, &delMsg, sizeof(SADelSaleItemMsg));

		pDestPlayer->DelItem(*pSrc, "摆摊", true);			// 自动解锁

		for (PLAYERMAP::iterator it = pDestPlayer->m_CurPlayerMap.begin(); it != pDestPlayer->m_CurPlayerMap.end(); ++it)
			g_StoreMessage(it->second, &delMsg, sizeof(SADelSaleItemMsg));
	}
	else
	{
		pSrc->overlap -= pMsg->stBuyItem.wNumber;
		pDestPlayer->SendItemSynMsg(pSrc, XYD_FT_ONLYLOCK);

		// 由于道具处于锁定状态，不能更新道具信息！

		// 同步给周围玩家数量的减少
		pDestPlayer->GetItemList()[pos].item.overlap -= pMsg->stBuyItem.wNumber;

		SAUpdateSaleItemMsg msgInfo;
		msgInfo.stSaleItem = pDestPlayer->GetItemList()[pos];
		g_StoreMessage(pDestPlayer->m_ClientIndex, &msgInfo, sizeof(SAUpdateSaleItemMsg));

		for (PLAYERMAP::iterator it = pDestPlayer->m_CurPlayerMap.begin(); it != pDestPlayer->m_CurPlayerMap.end(); ++it)
			g_StoreMessage(it->second, &msgInfo, sizeof(SAUpdateSaleItemMsg));
	}
	
	// 修正金钱
	pPlayer->m_Property.m_Money -= needMoney;

	DWORD remain = 0xffffffff - pDestPlayer->m_Property.m_Money;
	pDestPlayer->m_Property.m_Money += remain > needMoney ? needMoney :remain;

	pPlayer->m_PlayerPropertyStatus[XA_UNBIND_MONEY-XA_MAX_EXP] = true;
	pDestPlayer->m_PlayerPropertyStatus[XA_UNBIND_MONEY-XA_MAX_EXP] = remain;

	// 发送购买日志
	SQSendMessageMsg buyMsg;
	buyMsg.dwGlobal			= pDestPlayer->GetGID();

	_snprintf(buyMsg.szMsg, 80, "玩家{%s}花费%d购买了%d个%s", pPlayer->GetName(), needMoney, pMsg->stBuyItem.wNumber, pItemData->m_Name);

	pDestPlayer->RecvSendMessage(&buyMsg);

	SAMoneyInMsg moneyMsg;
	moneyMsg.dwMoneyIn = needMoney;
	g_StoreMessage(pDestPlayer->m_ClientIndex, &moneyMsg, sizeof(moneyMsg));
}

WORD CStall::FindPos()
{
	for (size_t i=0; i<MAX_SALEITEMNUM; i++)
	{
		if (0 == m_StallCells[i])
			return i;
	}

	return 0xffff;
}

void CStall::RecvMoveSaleItemMsg(SQMovetoSaleBoxMsg *pMsg)
{
	CPlayer *pPlayer = (CPlayer *)this;
	if (!pPlayer)
		return;

	// 其余的一些验证，比如当前安全时间，骑乘，地点等
	if (pPlayer->GetCurActionID() != (pPlayer->IsInStall() ? EA_ZAZEN : EA_STAND))
	{
		rfalse(2, 1, "当前的状态不是站立或打坐，无法上架/下架道具！");
		return;
	}
	
	switch(pMsg->byMoveto)
	{
	case SQMovetoSaleBoxMsg::MP_SALEBOX:		// 上架
		{
			if (pPlayer->m_Property.m_Level < 10)
				return;

			SPackageItem *pSrc = pPlayer->FindItemByPos(pMsg->wCellPos, XYD_FT_ONLYUNLOCK);
			if (!pSrc || (pSrc->wIndex != pMsg->wIndex))
				return;
			
			const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(pSrc->wIndex);
			if (!pData || ITEM_IS_TASK(pData->m_Type))
				return;

			if (!pData->m_CanTran)
				return;

			if (MAX_SALEITEMNUM == m_ForSaleNumber)
				return;

			// 如果总价超过32位所能表示的最大值，则失败
			QWORD totalMoney = pMsg->dMoney * pSrc->overlap;
			if (0 == totalMoney || totalMoney > 0xffffffff)
				return;

			// 找到一个空位子
			WORD pos = FindPos();
			if (0xffff == pos)
			{
				MY_ASSERT(m_ForSaleNumber == MAX_SALEITEMNUM);
				return;
			}

			MY_ASSERT(0 == m_StallItems[pos].item.wIndex && 0 == m_StallItems[pos].bPrice);

			if (!FillStallPos(pos, 0, 1))
			{
				MY_ASSERT(0);
				return;
			}

			pPlayer->LockItemCell(pMsg->wCellPos, true);

			// 加入摊位
			m_StallItems[pos].bPos	= pos;
			m_StallItems[pos].bPrice	= pMsg->dMoney;
			m_StallItems[pos].item	= *pSrc;

			SAAddSaleItemMsg addItem;
			addItem.bPos	= pos;
			addItem.dMoney	= pMsg->dMoney;
			addItem.item	= m_StallItems[pos].item;
			g_StoreMessage(pPlayer->m_ClientIndex, &addItem, sizeof(SAAddSaleItemMsg));

			m_ForSaleNumber++;

			if (!pPlayer->IsInStall())
				MY_ASSERT(m_CurPlayerMap.empty());

			// 发送给周围的观察者
			for (PLAYERMAP::iterator it = m_CurPlayerMap.begin(); it != m_CurPlayerMap.end(); ++it)
				g_StoreMessage(it->second, &addItem, sizeof(SAAddSaleItemMsg));
		}
		break;

	case SQMovetoSaleBoxMsg::MP_PACKAGE:		// 下架
		{
			WORD pos = pMsg->wCellPos;   // 摊位的坐标byX
			
			if (pos >= MAX_SALEITEMNUM || 0 == m_ForSaleNumber)
				return;

			if (0 == m_StallCells[pos])
			{
				MY_ASSERT(0 == m_StallItems[pos].item.wIndex && 0 == m_StallItems[pos].bPrice);
				return;
			}

			MY_ASSERT(0 != m_StallItems[pos].item.wIndex && 0 != m_StallItems[pos].bPrice);
			
			if (pMsg->wIndex != m_StallItems[pos].item.wIndex)
				return;

			// 验证道具是否还在背包
			SPackageItem *pSrc = pPlayer->FindItemByPos(m_StallItems[pos].item.wCellPos, XYD_FT_ONLYLOCK);
			MY_ASSERT(pSrc && (0 == memcmp(pSrc, &m_StallItems[pos].item, sizeof(SPackageItem))));

			pPlayer->LockItemCell(m_StallItems[pos].item.wCellPos, false);
			
			m_StallCells[pos] = 0;
			memset(&m_StallItems[pos], 0, sizeof(SaleItem));

			m_ForSaleNumber--;

			SADelSaleItemMsg delMsg;
			delMsg.bPos = pos;
			g_StoreMessage(pPlayer->m_ClientIndex, &delMsg, sizeof(SADelSaleItemMsg));

			if (!pPlayer->IsInStall())
				MY_ASSERT(m_CurPlayerMap.empty());

			for (PLAYERMAP::iterator it = m_CurPlayerMap.begin(); it != m_CurPlayerMap.end(); ++it)
				g_StoreMessage(it->second, &delMsg, sizeof(SADelSaleItemMsg));
		}
		break;

    default:
        return;
    }
}

void CStall::StallLoseStatus()
{
	SQCloseSaleItemMsg closeStall;
	closeStall.bOperation = SQCloseSaleItemMsg::CS_SELF;
	RecvCloseSaleItemMsg(&closeStall);

	if (dwOtherGID)
	{
		closeStall.bOperation = SQCloseSaleItemMsg::CS_BUYER;
		RecvCloseSaleItemMsg(&closeStall);
	}
}

void CStall::SendLogInfoToAll(SASendMessageMsg &logInfoMsg)
{
	for (PLAYERMAP::iterator it = m_CurPlayerMap.begin(); it != m_CurPlayerMap.end(); ++it)
	{
		g_StoreMessage(it->second, &logInfoMsg, sizeof(logInfoMsg));

		m_LogSendIndex[it->first] = logInfoMsg.log.index;
	}
}

void CStall::RecvSendMessage(struct SQSendMessageMsg *pMsg)
{
	CPlayer *pPlayer = (CPlayer *)this;
	if (!pPlayer)
		return;

	// 自己留言
	if (pMsg->dwGlobal == pPlayer->GetGID())
	{
		if (!pPlayer->IsInStall())
		{
			TalkToDnid(pPlayer->m_ClientIndex, "你不在摆摊中，无法留言！");
			return;
		}

		// 如果满了，就删除第一个
		if (MAX_LOGLINES == m_Logs.size())
			m_Logs.erase(m_Logs.begin());

		pMsg->szMsg[MAX_LOGSIZE-1] = '\0';

		StallLogInfo tempLog;
		tempLog.index = ++m_LogIndex;
		dwt::strcpy(tempLog.Info, pMsg->szMsg, MAX_LOGSIZE);

		m_Logs.push_back(tempLog);

		SASendMessageMsg messageBack;
		messageBack.log.index = tempLog.index;
		strcpy(messageBack.log.Info, tempLog.Info);

		// 发给自己
		g_StoreMessage(pPlayer->m_ClientIndex, &messageBack, sizeof(SASendMessageMsg));

		pPlayer->SendLogInfoToAll(messageBack);
	}
	else
	{
		if (0 == dwOtherGID)
			return;

		CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwOtherGID)->DynamicCast(IID_PLAYER);
		if (!pDestPlayer || 0 == pDestPlayer->m_CurHp || !pDestPlayer->IsInStall())
			return;

		PLAYERMAP::iterator iter = pDestPlayer->m_CurPlayerMap.find(pPlayer->GetGID());
		if (iter == pDestPlayer->m_CurPlayerMap.end())
			return;

		// 如果满了，就删除第一个
		if (MAX_LOGLINES == pDestPlayer->m_Logs.size())
			pDestPlayer->m_Logs.erase(pDestPlayer->m_Logs.begin());

		pMsg->szMsg[MAX_LOGSIZE-1] = '\0';

		StallLogInfo tempLog;
		tempLog.index = ++pDestPlayer->m_LogIndex;
		dwt::strcpy(tempLog.Info, pMsg->szMsg, MAX_LOGSIZE);

		pDestPlayer->m_Logs.push_back(tempLog);

		SASendMessageMsg messageBack;
		messageBack.log.index = tempLog.index;
		strcpy(messageBack.log.Info, tempLog.Info);

		// 发给摊主
		g_StoreMessage(pDestPlayer->m_ClientIndex, &messageBack, sizeof(SASendMessageMsg));

		pDestPlayer->SendLogInfoToAll(messageBack);
	}
}

BOOL CStall::FillStallPos(WORD pos, BYTE chk_value, BYTE fil_value)
{
	if (pos >= MAX_SALEITEMNUM)
		return FALSE;

	if (m_StallCells[pos] != chk_value)
		return FALSE;

	m_StallCells[pos] = fil_value;

	return TRUE;
}

void CStall::AddCurPlayertoList(DWORD dwGID, DNID qwDNID)
{
	m_CurPlayerMap[dwGID] = qwDNID;
}

bool CStall::DelCurPlayerFromList(DWORD dwGID)
{
	PLAYERMAP::iterator it = m_CurPlayerMap.find(dwGID);
	if (it != m_CurPlayerMap.end())
	{
		m_CurPlayerMap.erase(it);
		return true;
	}
	return false;
}