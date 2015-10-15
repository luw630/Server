#include "StdAfx.h"
#include "warehouse.h"
#include "Player.h"
#include "Item.h"
#include "CItemDefine.h"
#include "CItemService.h"
#include "Networkmodule\ItemMsgs.h"
#include "Networkmodule\ScriptMsgs.h"

bool CheckPos(LPBYTE Entry, BYTE wmax, BYTE hmax, BYTE sx, BYTE sy, BYTE w, BYTE h, BYTE chk_value)
{
    if (Entry == NULL || wmax > 6 || hmax > 16)
        return false;

	if (Entry[sy*wmax + sx] != chk_value)
		return false;

    return true;
}

CWareHouse::CWareHouse(void)
{
	memset((CWareHouse*)this, 0, sizeof(CWareHouse));

	m_WareHouseType = WHT_MAX;

	m_WHArrangeIntervalTime = timeGetTime();
}

CWareHouse::~CWareHouse(void)
{
}

void CWareHouse::OnDispatchMsg(SWareHouseBaseMsg *pMsg)
{
    switch (pMsg->_protocol)
    {
    // 请求激活一个仓库（返回仓库中的物品）
    case SWareHouseBaseMsg::RPRO_OPEN_ITEM_WAREHOUSE: 
        OnRecvOpenItemWareHouseMsg((SQOpenItemWareHouseMsg *)pMsg);
        break;

    // 请求关闭一个仓库（通知服务器仓库使用完毕）
    case SWareHouseBaseMsg::RPRO_CLOSE_WAREHOUSE: 
        OnRecvCloseWareHouseMsg((SQCloseWareHouseMsg *)pMsg);
        break;

    // 将一个道具（或者金钱）从身上移动到仓库
    case SWareHouseBaseMsg::RPRO_ITEM_MOVEIN: 
        OnRecvMoveItemInWareHouseMsg((SQMoveItemInWareHouseMsg *)pMsg);
        break;

    // 将一个道具（或者金钱）从仓库移动到身上
    case SWareHouseBaseMsg::RPRO_ITEM_MOVEOUT: 
        OnRecvMoveItemOutWareHouseMsg((SQMoveItemOutWareHouseMsg *)pMsg);
        break;

    // 将一个道具在仓库中移动
    case SWareHouseBaseMsg::RPRO_ITEM_MOVESELF: 
        OnRecvMoveItemSelfWareHouseMsg((SQMoveSelfItemWareHouseMsg *)pMsg);
        break;

	case SWareHouseBaseMsg::RPRO_ITEM_LOCKIT:
		OnRecvLockWareHouseMsg((SQLockWareHouseMsg*) pMsg);
		break;

	case SWareHouseBaseMsg::RPRO_ACTIVE_WAREHOUSE:
		OnRecvActiveWareHouseMsg((SQActiveWareHouseMsg *)pMsg);

	case SWareHouseBaseMsg::RPRO_ARRANGE_WAREHOUSE:
		ArrangeWareHouse();
		break;
    }
}

void CWareHouse::OnRecvActiveWareHouseMsg(struct SQActiveWareHouseMsg *pMsg)
{
	CPlayer *pPlayer = (CPlayer *)this;

	if (pPlayer->GetGID() != pMsg->dwGlobal)
		return;

	BYTE curActive = pPlayer->m_Property.m_ActiveTimes;
	
	SAActiveWareHouseMsg msg;

	if (3 == curActive)
	{
		msg.bResult = SAActiveWareHouseMsg::AWH_ALL_ACTIVED;
		msg.bCurActiveTimes = curActive;
	}
	else
	{
		DWORD needMoney = 100;
		if (curActive){
			needMoney = (curActive * 2 + 1) * 100;
		}
		 
		if (pPlayer->CheckPlayerMoney(XYD_UM_ONLYBIND,needMoney,false))
		{
			msg.bResult = SAActiveWareHouseMsg::AWH_SUCCESS;
			pPlayer->m_Property.m_ActiveTimes++;
			msg.bCurActiveTimes = pPlayer->m_Property.m_ActiveTimes;
		}
		else
		{
			msg.bResult = SAActiveWareHouseMsg::AWH_LACK_MONEY;
			msg.bCurActiveTimes = curActive;
		}

// 		if (pPlayer->m_Property.m_Money < 100)
// 		{
// 			msg.bResult = SAActiveWareHouseMsg::AWH_LACK_MONEY;
// 			msg.bCurActiveTimes = curActive;
// 		}
// 		else
// 		{
// 			msg.bResult = SAActiveWareHouseMsg::AWH_SUCCESS;
// 			pPlayer->m_Property.m_ActiveTimes++;
// 			msg.bCurActiveTimes = pPlayer->m_Property.m_ActiveTimes;
// 
// 			pPlayer->m_Property.m_Money -= needMoney;
// 
// 			pPlayer->m_PlayerPropertyStatus[XA_UNBIND_MONEY-XA_MAX_EXP] = true;
// 		}
	}

	g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
}

void CWareHouse::OnRecvLockWareHouseMsg(SQLockWareHouseMsg *pMsg)
{
    CPlayer *pPlayer = (CPlayer*)this;

	SALockWareHouseMsg msg;
	if (0 == dwt::strcmp( pMsg->szUserpass, pPlayer->m_Property.m_UPassword, CONST_USERPASS))
	{
		pPlayer->m_Property.m_bWarehouseLocked = pMsg->bLock;
		msg.bLock	 = pMsg->bLock;
		msg.bSuccess = true;
	}
	else
	{
		msg.bSuccess = false;
	}

	g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SALockWareHouseMsg));
}

void CWareHouse::WRLostStatus()
{
	TheEnd();
}

void CWareHouse::OnRecvOpenItemWareHouseMsg(SQOpenItemWareHouseMsg *pMsg)
{
	CPlayer *pPlayer = (CPlayer *)this;

	if (m_dwOperationSerial != 0)
	{
		rfalse(2, 1, "仓库已经被打开，无法二次打开！");
		pPlayer->SendStatusMsg(SANotifyStatus::NS_WH_ALREADY_OPEN);
		return;
	}

	SAOpenItemWareHouseMsg msg;
	msg.byGoodsNumber		= 0;
	msg.dwOperationSerial	= 0;
	msg.nMoneyStoraged		= 0;
	msg.m_ActiveTimes		= 0;
	msg.byRetCode			= SAOpenItemWareHouseMsg::ERC_SERVER_DISCARD;

	// 看看玩家是不是设置了2级密码
	if ('\0' != pPlayer->m_Property.m_UPassword[0] && pPlayer->m_Property.m_bWarehouseLocked)
	{
		if (pMsg->byDirectly)
			msg.byRetCode			= SAOpenItemWareHouseMsg::ERC_NEEDPASSWORD;
		else if (((0 != dwt::strcmp(pMsg->szUserpass, pPlayer->m_Property.m_UPassword, CONST_USERPASS))))
			msg.byRetCode			= SAOpenItemWareHouseMsg::ERC_PASSWORD_ERROR;

		g_StoreMessage(pPlayer->m_ClientIndex, &msg, msg.MySize());
		return;
	}

	WORD wEndPos = GetValidPosEnd(pPlayer->m_Property.m_ActiveTimes);
	
	for (int i=0; i<MAX_WAREHOUSE_ITEMNUMBER; i++)
	{
		SPackageItem *pItem = &pPlayer->m_Property.m_pStorageGoods[i];

		if (0 == pItem->wIndex)
			continue;

		// 验证坐标是否正确
		if (pItem->wCellPos >= wEndPos)
			goto _check_item_fail;

		const SItemBaseData *pItemData = CItemService::GetInstance().GetItemBaseData(pItem->wIndex);
		if (!pItemData)
			goto _check_item_fail;

		msg.GoodsArray[msg.byGoodsNumber++] = *pItem;

		continue;

_check_item_fail:

		// 该道具无效的情况（直接销毁）
		memset(pItem, 0, sizeof(SPackageItem));
	}

	m_WareHouseType = WHT_ITEM;

	// 生成序列号
	msg.dwOperationSerial	= m_dwOperationSerial = timeGetTime();
    msg.byRetCode			= SAOpenItemWareHouseMsg::ERC_OK;
    msg.nMoneyStoraged		= pPlayer->m_Property.m_whMoney;
	msg.m_ActiveTimes		= pPlayer->m_Property.m_ActiveTimes;
	msg.nBindMoney = pPlayer->m_Property.m_whBindMoney;
    // 向客户端发消息
    g_StoreMessage(pPlayer->m_ClientIndex, &msg, msg.MySize());

	return;
}

void CWareHouse::OnRecvCloseWareHouseMsg(SQCloseWareHouseMsg *pMsg)
{
	SACloseWareHouseMsg msg;
    msg.byRetCode = SACloseWareHouseMsg::ERC_SERVER_DISCARD;

    CPlayer *pPlayer = (CPlayer*)this;

    // 当前还没有打开这个仓库
	if (0 == m_dwOperationSerial || pMsg->dwOperationSerial != m_dwOperationSerial)
        return;

	// 将备份的数据转到仓库
    TheEnd();
    
	// 向客户端发消息
    msg.byRetCode = SACloseWareHouseMsg::ERC_OK;
    g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SACloseWareHouseMsg));
}

void CWareHouse::OnRecvMoveItemInWareHouseMsg(SQMoveItemInWareHouseMsg *pMsg)
{
	CPlayer *pPlayer = static_cast<CPlayer *>(this);
	if (!pPlayer)
		return;

	if (0 == m_dwOperationSerial || pMsg->dwOperationSerial != m_dwOperationSerial)
		return;

	if (WHT_ITEM != m_WareHouseType)
		return;

	SAMoveItemInWareHouseMsg msg;
	msg.byRetCode			= SAMoveItemInWareHouseMsg::ERC_SERVER_DISCARD;
	msg.nMoneyOfStorage		= pPlayer->m_Property.m_whMoney;
	msg.nBindMoneyOfStorage = pPlayer->m_Property.m_whBindMoney;
	memset(&msg.Item, 0, sizeof(SPackageItem));

	bool bIsStoreMoney = ((pMsg->Store.wCellPos>>0xff)&0xff)==255;	// pMsg->Store.byCellY
    // 首先如果要存的是钱
	if (bIsStoreMoney)
	{
		if (0 == pMsg->wIndex)
			return;
		WORD wMonType = (pMsg->Store.wCellPos & 0xff); // pMsg->Store.byCellX
		if (1 == wMonType)			// 绑定货币
		{
			if (!pPlayer->CheckPlayerMoney(XYD_UM_ONLYBIND, pMsg->wIndex, true))
				return;

			DWORD remain = 99999999 - pPlayer->m_Property.m_whBindMoney;
			
			if (pMsg->wIndex > remain)
			{
				TalkToDnid(pPlayer->m_ClientIndex, "仓库中的绑定货栏空间不足，无法再存入！");
				return;
			}

			pPlayer->m_Property.m_whBindMoney += pMsg->wIndex;

			pPlayer->CheckPlayerMoney(XYD_UM_ONLYBIND, pMsg->wIndex, false);
		}
		else if (2 == wMonType)		// 非绑定货币
		{
			if (!pPlayer->CheckPlayerMoney(XYD_UM_ONLYUNBIND, pMsg->wIndex, true))
				return;

			DWORD remain = 99999999 - pPlayer->m_Property.m_whMoney;

			if (pMsg->wIndex > remain)
			{
				TalkToDnid(pPlayer->m_ClientIndex, "仓库中的非绑定货栏空间不足，无法再存入！");
				return;
			}

			pPlayer->m_Property.m_whMoney += pMsg->wIndex;

			pPlayer->CheckPlayerMoney(XYD_UM_ONLYUNBIND, pMsg->wIndex, false);
		}
// 		// 存礼券
// 		else if (3 == wMonType)
// 		{
// 			if (!pPlayer->CheckPlayerMoney(XYD_UM_ZENGBAO, pMsg->wIndex, true))
// 				return;
// 
// 			DWORD remain = 99999999 - pPlayer->m_Property.m_whZengBao;
// 
// 			if (pMsg->wIndex > remain)
// 			{
// 				TalkToDnid(pPlayer->m_ClientIndex, "仓库中的赠宝货栏空间不足，无法再存入！");
// 				return;
// 			}
// 
// 			pPlayer->m_Property.m_whZengBao += pMsg->wIndex;
// 			pPlayer->CheckPlayerMoney(XYD_UM_ZENGBAO, pMsg->wIndex, false);
// 		}
// 		// 存元宝
// 		else if (4 == wMonType)
// 		{
// 			if (!pPlayer->CheckPlayerMoney(XYD_UM_YUANBAO, pMsg->wIndex, true))
// 				return;
// 
// 			DWORD remain = 99999999 - pPlayer->m_Property.m_whYuanBao;
// 
// 			if (pMsg->wIndex > remain)
// 			{
// 				TalkToDnid(pPlayer->m_ClientIndex, "仓库中的元宝货栏空间不足，无法再存入！");
// 				return;
// 			}
// 
// 			pPlayer->m_Property.m_whYuanBao += pMsg->wIndex;
// 			pPlayer->CheckPlayerMoney(XYD_UM_YUANBAO, pMsg->wIndex, false);
// 		}

		msg.byRetCode			= SAMoveItemInWareHouseMsg::ERC_SET_MONEY;
		msg.nMoneyOfStorage		= pPlayer->m_Property.m_whMoney;
		msg.nBindMoneyOfStorage = pPlayer->m_Property.m_whBindMoney;

		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMoveItemInWareHouseMsg));
		return;
	}
	
	// 检测道具的有效性
	WORD linePos = pMsg->Pocket.wCellPos;
	
	SPackageItem *pItem = pPlayer->FindItemByPos(linePos, XYD_FT_ONLYUNLOCK);
	if (!pItem)
		return;

	MY_ASSERT(pItem->wIndex);

	const SItemBaseData *pSrcItemData = CItemService::GetInstance().GetItemBaseData(pItem->wIndex);
	if (!pSrcItemData || !pSrcItemData->m_CanStroage)
		return;

	//+ 任务道具无法存入仓库,是否需要提示
	if (ITEM_IS_TASK(pSrcItemData->m_Type))
	{
		return ;
	}
	
	// 玩家点击右键自动存储
	if (1 == pMsg->wIndex)			
	{
		WORD DestPos = GetValidPosEnd(pPlayer->m_Property.m_ActiveTimes);

		if (1 == pSrcItemData->m_Overlay)
		{
			for (int i=0; i< DestPos; i++)
			{
				SPackageItem *dest = &pPlayer->m_Property.m_pStorageGoods[i];
				
				if (0 == dest->wIndex)
				{
					*dest = *pItem;
					pPlayer->DelItem(*pItem, "移动到仓库");	

					dest->wCellPos = i;					

					msg.byRetCode = SAMoveItemInWareHouseMsg::ERC_OK;
					msg.Item = *dest;
					g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMoveItemInWareHouseMsg));
					break;
				}
			}
		}
		else
		{
			WORD stillLeft = pItem->overlap;

			for (int i=0; i< DestPos; i++)			// 先尝试合并
			{
				SPackageItem *dest = &pPlayer->m_Property.m_pStorageGoods[i];

				if (0 == dest->wIndex)
					continue;

				if (dest->wIndex != pItem->wIndex || dest->overlap == pSrcItemData->m_Overlay)
					continue;

				WORD remain = pSrcItemData->m_Overlay - dest->overlap;

				if (remain >= stillLeft)
				{
					dest->overlap += stillLeft;
					pPlayer->DelItem(*pItem, "移动到仓库");
					stillLeft = 0;
				}
				else
				{
					dest->overlap = pSrcItemData->m_Overlay;
					pPlayer->ChangeOverlap(pItem, remain, false);
					stillLeft -= remain;
				}

				msg.byRetCode = SAMoveItemInWareHouseMsg::ERC_OK;
				msg.Item = *dest;
				g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMoveItemInWareHouseMsg));

				if (0 == stillLeft)
					break;
			}

			if (stillLeft)				// 可能需要新建，只需要一个空格子
			{
				for (int i=0; i< DestPos; i++)
				{
					SPackageItem *dest = &pPlayer->m_Property.m_pStorageGoods[i];

					if (0 == dest->wIndex)
					{
						*dest = *pItem;
						pPlayer->DelItem(*pItem, "移动到仓库");

						dest->wCellPos = i;

						msg.byRetCode = SAMoveItemInWareHouseMsg::ERC_OK;
						msg.Item = *dest;
						g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMoveItemInWareHouseMsg));

						break;
					}
				}
			}
		}

		return;
	}

	// 往下就是拖动道具了，执行检测，检测目的坐标的有效性
	if (pMsg->Store.wCellPos >= GetValidPosEnd(pPlayer->m_Property.m_ActiveTimes))
		return;

	WORD destPos = pMsg->Store.wCellPos;

	SPackageItem *dest = &pPlayer->m_Property.m_pStorageGoods[destPos];
	
	// 如果目的物品不同于源物品或者目的物品上限已满（包含不可叠加的物品），那么返回失败
	if (0 != dest->wIndex && ((dest->wIndex != pItem->wIndex) || (dest->overlap == pSrcItemData->m_Overlay)))
	{
		TalkToDnid(pPlayer->m_ClientIndex, "目的物品不同于源物品或者目的物品上限已满！");
		msg.byRetCode = SAMoveItemInWareHouseMsg::ERC_CANNOT_PUT;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMoveItemInWareHouseMsg));
		return;
	}

	if (0 == dest->wIndex)
	{
		*dest = *pItem;
		pPlayer->DelItem(*pItem, "移动到仓库");	

		dest->wCellPos = pMsg->Store.wCellPos;		
	}
	else
	{
		WORD storageRemain = pSrcItemData->m_Overlay - dest->overlap;
		if (storageRemain >= pItem->overlap)
		{
			dest->overlap += pItem->overlap;
			pPlayer->DelItem(*pItem, "移动到仓库");
		}
		else
		{
			dest->overlap = pSrcItemData->m_Overlay;	// 最大
			pPlayer->ChangeOverlap(pItem, storageRemain, false);
		}
	}
	
	msg.byRetCode = SAMoveItemInWareHouseMsg::ERC_OK;
	msg.Item = *dest;
	g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMoveItemInWareHouseMsg));

	return;
}

void CWareHouse::OnRecvMoveItemOutWareHouseMsg(SQMoveItemOutWareHouseMsg *pMsg)
{
	CPlayer *pPlayer = static_cast<CPlayer *>(this);
	if (!pPlayer)
		return;

	if (0 == m_dwOperationSerial || pMsg->dwOperationSerial != m_dwOperationSerial)
		return;

	if (WHT_ITEM != m_WareHouseType)
		return;

	SAMoveItemOutWareHouseMsg msg;
	msg.byRetCode			= SAMoveItemOutWareHouseMsg::ERC_SERVER_DISCARD;
	msg.nMoneyOfStorage		= pPlayer->m_Property.m_whMoney;
	msg.nBindMoneyOfStorage = pPlayer->m_Property.m_whBindMoney;
	memset(&msg.Item, 0, sizeof(SPackageItem));

	bool bIsStoreMoney = ((pMsg->Store.wCellPos>>0xff)&0xff)==255;	//255 == pMsg->Store.byCellY
   	// 如果是要取钱
	if (bIsStoreMoney)
    {
		if (0 == pMsg->wIndex)
			return;
		WORD wMonType = (pMsg->Store.wCellPos & 0xff); // pMsg->Store.byCellX
		if (1 == wMonType)			// 绑定货币
		{
			if (pMsg->wIndex > pPlayer->m_Property.m_whBindMoney)
				return;

			DWORD left = pPlayer->AddPlayerMoney(XYD_UM_ONLYBIND, pMsg->wIndex);
			pPlayer->m_Property.m_whBindMoney -= (pMsg->wIndex-left);
		}
		else if (2 == wMonType)
		{
			if (pMsg->wIndex > pPlayer->m_Property.m_whMoney)
				return;

			DWORD left = pPlayer->AddPlayerMoney(XYD_UM_ONLYUNBIND, pMsg->wIndex);
			pPlayer->m_Property.m_whMoney -= (pMsg->wIndex-left);
		}

		msg.byRetCode			= SAMoveItemOutWareHouseMsg::ERC_SET_MONEY;
		msg.nMoneyOfStorage		= pPlayer->m_Property.m_whMoney;
		msg.nBindMoneyOfStorage = pPlayer->m_Property.m_whBindMoney;

		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMoveItemOutWareHouseMsg));
		return;
    }

	// 检测原坐标的有效性
	if (pMsg->Store.wCellPos >= GetValidPosEnd(pPlayer->m_Property.m_ActiveTimes))
		return;

	WORD srcPos = pMsg->Store.wCellPos;

	SPackageItem *srcStore = &pPlayer->m_Property.m_pStorageGoods[srcPos];
	if (0 == srcStore->wIndex)
		return;

	const SItemBaseData *pSrcItemData = CItemService::GetInstance().GetItemBaseData(srcStore->wIndex);
	if (!pSrcItemData)
		return;
	//+ 任务道具不能放入仓库
	if (ITEM_IS_TASK(pSrcItemData->m_Type))
	{
		return ;
	}

	// 玩家点击右键自动加入背包
	if (1 == pMsg->wIndex)			
	{
		if (1 == pSrcItemData->m_Overlay)
		{
			WORD pos = pPlayer->FindBlankPos(XYD_PT_BASE);
			if (0xffff == pos)
				return;

			pPlayer->AddExistingItem(*srcStore, pos, true);

			msg.Item = *srcStore;
			msg.Item.wIndex  = 0;
			msg.byRetCode	 = SAMoveItemOutWareHouseMsg::ERC_OK;

			memset(srcStore, 0, sizeof(*srcStore));
		}
		else
		{
			srcStore->overlap = pPlayer->TryPossibleMerge(srcStore->wIndex, srcStore->overlap, pSrcItemData->m_Overlay);

			msg.byRetCode = SAMoveItemOutWareHouseMsg::ERC_OK;
			msg.Item = *srcStore;

			if (srcStore->overlap)
			{
				WORD pos = pPlayer->FindBlankPos(XYD_PT_BASE);
				if (0xffff != pos)
				{
					pPlayer->AddExistingItem(*srcStore, pos, true);

					msg.Item.wIndex  = 0;
					memset(srcStore, 0, sizeof(*srcStore));
				}
			}
			else
			{
				msg.Item.wIndex = 0;
				memset(srcStore, 0, sizeof(*srcStore));
			}
		}

		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMoveItemOutWareHouseMsg));

		return;
	}

	// 检测道具的有效性
	WORD destPos = pMsg->Pocket.wCellPos;

	if (!pPlayer->IsItemPosValid(pMsg->Pocket.wCellPos, false, true))
		return;

	SPackageItem *dest = pPlayer->FindItemByPos(pMsg->Pocket.wCellPos, XYD_FT_ONLYUNLOCK);

	if (dest && ((0 != dest->wIndex && (dest->wIndex != srcStore->wIndex)) || (dest->overlap == pSrcItemData->m_Overlay)))
	{
		TalkToDnid(pPlayer->m_ClientIndex, "目标点道具类型不同或者已到叠加上限！");
		msg.byRetCode = SAMoveItemOutWareHouseMsg::ERC_CANNOT_PUT;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMoveItemOutWareHouseMsg));
		return;
	}

	msg.byRetCode = SAMoveItemOutWareHouseMsg::ERC_OK;

	if (!dest)
	{
		pPlayer->AddExistingItem(*srcStore, destPos, true);

		msg.Item		 = *srcStore;
		msg.Item.wIndex  = 0;
		memset(srcStore, 0, sizeof(*srcStore));
	}
	else
	{
		WORD pocketRemain = pSrcItemData->m_Overlay - dest->overlap;
		if (pocketRemain >= srcStore->overlap)
		{
			pPlayer->ChangeOverlap(dest, srcStore->overlap, true);

			msg.Item		 = *srcStore;
			msg.Item.wIndex  = 0;
			memset(srcStore, 0, sizeof(*srcStore));
		}
		else
		{
			pPlayer->ChangeOverlap(dest, pocketRemain, true);

			srcStore->overlap	-= pocketRemain;
			msg.Item			= *srcStore;
		}
	}

	g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMoveItemOutWareHouseMsg));

	return;
}

void CWareHouse::OnRecvMoveItemSelfWareHouseMsg(SQMoveSelfItemWareHouseMsg *pMsg)
{
	CPlayer *pPlayer = (CPlayer *)(this);
	if (!pPlayer)
		return;

	SAMoveSelfItemWareHouseMsg msg;
	msg.byRetCode = SAMoveSelfItemWareHouseMsg::ERC_SERVER_DISCARD;
	memset(&msg.srcItem,  0, sizeof(SPackageItem));
	memset(&msg.destItem, 0, sizeof(SPackageItem));

	if (0 == m_dwOperationSerial || pMsg->dwOperationSerial != m_dwOperationSerial)
		return;

	if (WHT_ITEM != m_WareHouseType)
		return;

	// 检测索引的有效性
	// 检测原坐标的有效性
	WORD wEndPos = GetValidPosEnd(pPlayer->m_Property.m_ActiveTimes);

	if (pMsg->StoreSrc.wCellPos >= wEndPos || pMsg->StoreDest.wCellPos >= wEndPos)
		return;

	WORD srcPos = pMsg->StoreSrc.wCellPos;
	WORD desPos = pMsg->StoreDest.wCellPos;

	SPackageItem *src = &pPlayer->m_Property.m_pStorageGoods[srcPos];
	if (0 == src->wIndex)
		return;

	SPackageItem *des = &pPlayer->m_Property.m_pStorageGoods[desPos];
	
	bool willResetPos = false;

	// 目标位置为空
	if (0 == des->wIndex)
	{
		src->wCellPos = pMsg->StoreDest.wCellPos;		

		pPlayer->m_Property.m_pStorageGoods[desPos] = pPlayer->m_Property.m_pStorageGoods[srcPos];

		memset(&pPlayer->m_Property.m_pStorageGoods[srcPos], 0, sizeof(SPackageItem));

		willResetPos = true;
	}
	else
	{
		const SItemBaseData *pSrcData = CItemService::GetInstance().GetItemBaseData(src->wIndex);
		const SItemBaseData *pDesData = CItemService::GetInstance().GetItemBaseData(des->wIndex);
		
		if (!pSrcData || !pDesData)
			return;

		if ((des->wIndex != src->wIndex) || (des->overlap == pDesData->m_Overlay))
		{
			// 直接交换
			des->wCellPos = pMsg->StoreSrc.wCellPos;
			src->wCellPos = pMsg->StoreDest.wCellPos;

			SPackageItem temp	 = pPlayer->m_Property.m_pStorageGoods[srcPos];
			pPlayer->m_Property.m_pStorageGoods[srcPos] = pPlayer->m_Property.m_pStorageGoods[desPos];
			pPlayer->m_Property.m_pStorageGoods[desPos] = temp;
		}
		else
		{
			// 叠加
			WORD stillReamin = pDesData->m_Overlay - des->overlap;

			if (stillReamin >= src->overlap)
			{
				des->overlap += src->overlap;

				memset(&pPlayer->m_Property.m_pStorageGoods[srcPos], 0, sizeof(SPackageItem));

				willResetPos = true;
			}
			else
			{
				des->overlap += stillReamin;
				src->overlap -= stillReamin;
			}
		}
	}
	
	msg.byRetCode = SAMoveSelfItemWareHouseMsg::ERC_OK;
	msg.srcItem   = pPlayer->m_Property.m_pStorageGoods[srcPos];
	msg.destItem  = pPlayer->m_Property.m_pStorageGoods[desPos];

	if (willResetPos)
	{
		msg.srcItem.wCellPos  = pMsg->StoreSrc.wCellPos;
	}
	
	g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
}

void CWareHouse::TheEnd(BOOL backupOnly)
{
    // 当前还没有打开这个仓库
    if (0 == m_dwOperationSerial)
        return;

	m_dwOperationSerial = 0;
	m_WareHouseType		= WHT_MAX;
}

void CWareHouse::ArrangeWareHouse()
{
	CPlayer *pPlayer = (CPlayer *)(this);
	if (!pPlayer)
		return;

	int index = 0;

	SPackageItem *pPackage[MAX_WAREHOUSE_ITEMNUMBER] = {0};
	SPackageItem final[MAX_WAREHOUSE_ITEMNUMBER];

	// 验证时间间隔
	if (timeGetTime() - m_WHArrangeIntervalTime < 10000)
		return;

	m_WHArrangeIntervalTime = timeGetTime();

	WORD DestPos = GetValidPosEnd(pPlayer->m_Property.m_ActiveTimes);

	WORD totalItems = 0;
	for (size_t i = 0; i < DestPos; i++)
	{
		if (0 == pPlayer->m_Property.m_pStorageGoods[i].wIndex)
			continue;

		final[index] = pPlayer->m_Property.m_pStorageGoods[i];
		pPackage[index] = &final[index];
		index++;
		totalItems++;
	}

	if (0 == totalItems)
		return;

	// 先开始合并道具
	for (int i=0; i<index; i++)
	{
		if (!pPackage[i])
			continue;

		const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(pPackage[i]->wIndex);
		MY_ASSERT(itemData);

		// 如果无法叠加或者已是最大数，则跳过
		if (itemData->m_Overlay == pPackage[i]->overlap)
			continue;

		WORD StillCon = itemData->m_Overlay - pPackage[i]->overlap;

		for (int j=i+1; j<index; j++)
		{
			if (!pPackage[j])
				continue;

			// 判断是否可以叠加至pPackage[i]
			if ((pPackage[i]->wIndex != pPackage[j]->wIndex))
				continue;

			if (StillCon >= pPackage[j]->overlap)
			{
				pPackage[i]->overlap += pPackage[j]->overlap;
				pPackage[j] = 0;
			}
			else
			{
				pPackage[i]->overlap = itemData->m_Overlay;
				pPackage[j]->overlap -= StillCon;
			}

			StillCon = itemData->m_Overlay - pPackage[i]->overlap;

			if (0 == StillCon)
				break;
		}
	}

	DWORD finalCount = 0;
	for (int i=0; i<index; i++)
	{
		if (!pPackage[i])
			continue;

		final[finalCount++] = *pPackage[i];
	}

	SAMoveItemOutWareHouseMsg msg;
	msg.byRetCode			= SAMoveItemInWareHouseMsg::ERC_OK;
	msg.nMoneyOfStorage		= pPlayer->m_Property.m_whMoney;
	msg.nBindMoneyOfStorage	= pPlayer->m_Property.m_whBindMoney;

	for (size_t i = 0; i < DestPos; i++)
	{
		if (0 == pPlayer->m_Property.m_pStorageGoods[i].wIndex)
			continue;

		msg.Item = pPlayer->m_Property.m_pStorageGoods[i];
		msg.Item.wIndex = 0;

		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}

	memset(pPlayer->m_Property.m_pStorageGoods, 0, sizeof(pPlayer->m_Property.m_pStorageGoods));

	std::map<WORD, std::list<WORD> > Level1Info;

	for (size_t i=0; i<finalCount; i++)
	{
		MY_ASSERT(final[i].wIndex);

		const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(final[i].wIndex);
		if (!itemData)
			continue;

		Level1Info[itemData->m_Type].push_back(i);
	}

	WORD insertIndex = 0;

	// 开始分析每一个类型
	for (std::map<WORD, std::list<WORD> >::iterator it = Level1Info.begin(); it != Level1Info.end(); ++it)
	{
		std::map<DWORD, std::list<WORD> > Level2Info;

		for (std::list<WORD>::iterator ti = it->second.begin(); ti != it->second.end(); ++ti)
		{
			Level2Info[final[*ti].wIndex].push_back(*ti);
		}

		// 开始分析第二层
		for (std::map<DWORD, std::list<WORD> >::iterator it3 = Level2Info.begin(); it3 != Level2Info.end(); ++it3)
		{
			// 开始执行插入
			for (std::list<WORD>::iterator ti3 = it3->second.begin(); ti3 != it3->second.end(); ++ti3)
			{
				SPackageItem *item = &final[*ti3];
				MY_ASSERT(item->wIndex);

				pPlayer->m_Property.m_pStorageGoods[insertIndex] = *item;
				pPlayer->m_Property.m_pStorageGoods[insertIndex].wCellPos = insertIndex;		

				msg.Item = pPlayer->m_Property.m_pStorageGoods[insertIndex];

				g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMoveItemInWareHouseMsg));

				insertIndex++;
			}
		}
	}	

    return;
}

//+临时添加一个获取仓库有效范围的函数，避免到处都用到纯数字，可替换 
// 目前初始36格，然后每激活一次打开12格格子，最多激活3次
WORD CWareHouse::GetValidPosEnd(BYTE byActiveTimes) const
{
	WORD wCurEnd = 36 + byActiveTimes*2*6;
	if (wCurEnd > MAX_WAREHOUSE_ITEMNUMBER)
	{
		MY_ASSERT(0);
		wCurEnd = MAX_WAREHOUSE_ITEMNUMBER;
	}
	return wCurEnd;
}