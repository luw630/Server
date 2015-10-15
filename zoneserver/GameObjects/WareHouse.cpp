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
    // ���󼤻�һ���ֿ⣨���زֿ��е���Ʒ��
    case SWareHouseBaseMsg::RPRO_OPEN_ITEM_WAREHOUSE: 
        OnRecvOpenItemWareHouseMsg((SQOpenItemWareHouseMsg *)pMsg);
        break;

    // ����ر�һ���ֿ⣨֪ͨ�������ֿ�ʹ����ϣ�
    case SWareHouseBaseMsg::RPRO_CLOSE_WAREHOUSE: 
        OnRecvCloseWareHouseMsg((SQCloseWareHouseMsg *)pMsg);
        break;

    // ��һ�����ߣ����߽�Ǯ���������ƶ����ֿ�
    case SWareHouseBaseMsg::RPRO_ITEM_MOVEIN: 
        OnRecvMoveItemInWareHouseMsg((SQMoveItemInWareHouseMsg *)pMsg);
        break;

    // ��һ�����ߣ����߽�Ǯ���Ӳֿ��ƶ�������
    case SWareHouseBaseMsg::RPRO_ITEM_MOVEOUT: 
        OnRecvMoveItemOutWareHouseMsg((SQMoveItemOutWareHouseMsg *)pMsg);
        break;

    // ��һ�������ڲֿ����ƶ�
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
		rfalse(2, 1, "�ֿ��Ѿ����򿪣��޷����δ򿪣�");
		pPlayer->SendStatusMsg(SANotifyStatus::NS_WH_ALREADY_OPEN);
		return;
	}

	SAOpenItemWareHouseMsg msg;
	msg.byGoodsNumber		= 0;
	msg.dwOperationSerial	= 0;
	msg.nMoneyStoraged		= 0;
	msg.m_ActiveTimes		= 0;
	msg.byRetCode			= SAOpenItemWareHouseMsg::ERC_SERVER_DISCARD;

	// ��������ǲ���������2������
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

		// ��֤�����Ƿ���ȷ
		if (pItem->wCellPos >= wEndPos)
			goto _check_item_fail;

		const SItemBaseData *pItemData = CItemService::GetInstance().GetItemBaseData(pItem->wIndex);
		if (!pItemData)
			goto _check_item_fail;

		msg.GoodsArray[msg.byGoodsNumber++] = *pItem;

		continue;

_check_item_fail:

		// �õ�����Ч�������ֱ�����٣�
		memset(pItem, 0, sizeof(SPackageItem));
	}

	m_WareHouseType = WHT_ITEM;

	// �������к�
	msg.dwOperationSerial	= m_dwOperationSerial = timeGetTime();
    msg.byRetCode			= SAOpenItemWareHouseMsg::ERC_OK;
    msg.nMoneyStoraged		= pPlayer->m_Property.m_whMoney;
	msg.m_ActiveTimes		= pPlayer->m_Property.m_ActiveTimes;
	msg.nBindMoney = pPlayer->m_Property.m_whBindMoney;
    // ��ͻ��˷���Ϣ
    g_StoreMessage(pPlayer->m_ClientIndex, &msg, msg.MySize());

	return;
}

void CWareHouse::OnRecvCloseWareHouseMsg(SQCloseWareHouseMsg *pMsg)
{
	SACloseWareHouseMsg msg;
    msg.byRetCode = SACloseWareHouseMsg::ERC_SERVER_DISCARD;

    CPlayer *pPlayer = (CPlayer*)this;

    // ��ǰ��û�д�����ֿ�
	if (0 == m_dwOperationSerial || pMsg->dwOperationSerial != m_dwOperationSerial)
        return;

	// �����ݵ�����ת���ֿ�
    TheEnd();
    
	// ��ͻ��˷���Ϣ
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
    // �������Ҫ�����Ǯ
	if (bIsStoreMoney)
	{
		if (0 == pMsg->wIndex)
			return;
		WORD wMonType = (pMsg->Store.wCellPos & 0xff); // pMsg->Store.byCellX
		if (1 == wMonType)			// �󶨻���
		{
			if (!pPlayer->CheckPlayerMoney(XYD_UM_ONLYBIND, pMsg->wIndex, true))
				return;

			DWORD remain = 99999999 - pPlayer->m_Property.m_whBindMoney;
			
			if (pMsg->wIndex > remain)
			{
				TalkToDnid(pPlayer->m_ClientIndex, "�ֿ��еİ󶨻����ռ䲻�㣬�޷��ٴ��룡");
				return;
			}

			pPlayer->m_Property.m_whBindMoney += pMsg->wIndex;

			pPlayer->CheckPlayerMoney(XYD_UM_ONLYBIND, pMsg->wIndex, false);
		}
		else if (2 == wMonType)		// �ǰ󶨻���
		{
			if (!pPlayer->CheckPlayerMoney(XYD_UM_ONLYUNBIND, pMsg->wIndex, true))
				return;

			DWORD remain = 99999999 - pPlayer->m_Property.m_whMoney;

			if (pMsg->wIndex > remain)
			{
				TalkToDnid(pPlayer->m_ClientIndex, "�ֿ��еķǰ󶨻����ռ䲻�㣬�޷��ٴ��룡");
				return;
			}

			pPlayer->m_Property.m_whMoney += pMsg->wIndex;

			pPlayer->CheckPlayerMoney(XYD_UM_ONLYUNBIND, pMsg->wIndex, false);
		}
// 		// ����ȯ
// 		else if (3 == wMonType)
// 		{
// 			if (!pPlayer->CheckPlayerMoney(XYD_UM_ZENGBAO, pMsg->wIndex, true))
// 				return;
// 
// 			DWORD remain = 99999999 - pPlayer->m_Property.m_whZengBao;
// 
// 			if (pMsg->wIndex > remain)
// 			{
// 				TalkToDnid(pPlayer->m_ClientIndex, "�ֿ��е����������ռ䲻�㣬�޷��ٴ��룡");
// 				return;
// 			}
// 
// 			pPlayer->m_Property.m_whZengBao += pMsg->wIndex;
// 			pPlayer->CheckPlayerMoney(XYD_UM_ZENGBAO, pMsg->wIndex, false);
// 		}
// 		// ��Ԫ��
// 		else if (4 == wMonType)
// 		{
// 			if (!pPlayer->CheckPlayerMoney(XYD_UM_YUANBAO, pMsg->wIndex, true))
// 				return;
// 
// 			DWORD remain = 99999999 - pPlayer->m_Property.m_whYuanBao;
// 
// 			if (pMsg->wIndex > remain)
// 			{
// 				TalkToDnid(pPlayer->m_ClientIndex, "�ֿ��е�Ԫ�������ռ䲻�㣬�޷��ٴ��룡");
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
	
	// �����ߵ���Ч��
	WORD linePos = pMsg->Pocket.wCellPos;
	
	SPackageItem *pItem = pPlayer->FindItemByPos(linePos, XYD_FT_ONLYUNLOCK);
	if (!pItem)
		return;

	MY_ASSERT(pItem->wIndex);

	const SItemBaseData *pSrcItemData = CItemService::GetInstance().GetItemBaseData(pItem->wIndex);
	if (!pSrcItemData || !pSrcItemData->m_CanStroage)
		return;

	//+ ��������޷�����ֿ�,�Ƿ���Ҫ��ʾ
	if (ITEM_IS_TASK(pSrcItemData->m_Type))
	{
		return ;
	}
	
	// ��ҵ���Ҽ��Զ��洢
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
					pPlayer->DelItem(*pItem, "�ƶ����ֿ�");	

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

			for (int i=0; i< DestPos; i++)			// �ȳ��Ժϲ�
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
					pPlayer->DelItem(*pItem, "�ƶ����ֿ�");
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

			if (stillLeft)				// ������Ҫ�½���ֻ��Ҫһ���ո���
			{
				for (int i=0; i< DestPos; i++)
				{
					SPackageItem *dest = &pPlayer->m_Property.m_pStorageGoods[i];

					if (0 == dest->wIndex)
					{
						*dest = *pItem;
						pPlayer->DelItem(*pItem, "�ƶ����ֿ�");

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

	// ���¾����϶������ˣ�ִ�м�⣬���Ŀ���������Ч��
	if (pMsg->Store.wCellPos >= GetValidPosEnd(pPlayer->m_Property.m_ActiveTimes))
		return;

	WORD destPos = pMsg->Store.wCellPos;

	SPackageItem *dest = &pPlayer->m_Property.m_pStorageGoods[destPos];
	
	// ���Ŀ����Ʒ��ͬ��Դ��Ʒ����Ŀ����Ʒ�����������������ɵ��ӵ���Ʒ������ô����ʧ��
	if (0 != dest->wIndex && ((dest->wIndex != pItem->wIndex) || (dest->overlap == pSrcItemData->m_Overlay)))
	{
		TalkToDnid(pPlayer->m_ClientIndex, "Ŀ����Ʒ��ͬ��Դ��Ʒ����Ŀ����Ʒ����������");
		msg.byRetCode = SAMoveItemInWareHouseMsg::ERC_CANNOT_PUT;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMoveItemInWareHouseMsg));
		return;
	}

	if (0 == dest->wIndex)
	{
		*dest = *pItem;
		pPlayer->DelItem(*pItem, "�ƶ����ֿ�");	

		dest->wCellPos = pMsg->Store.wCellPos;		
	}
	else
	{
		WORD storageRemain = pSrcItemData->m_Overlay - dest->overlap;
		if (storageRemain >= pItem->overlap)
		{
			dest->overlap += pItem->overlap;
			pPlayer->DelItem(*pItem, "�ƶ����ֿ�");
		}
		else
		{
			dest->overlap = pSrcItemData->m_Overlay;	// ���
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
   	// �����ҪȡǮ
	if (bIsStoreMoney)
    {
		if (0 == pMsg->wIndex)
			return;
		WORD wMonType = (pMsg->Store.wCellPos & 0xff); // pMsg->Store.byCellX
		if (1 == wMonType)			// �󶨻���
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

	// ���ԭ�������Ч��
	if (pMsg->Store.wCellPos >= GetValidPosEnd(pPlayer->m_Property.m_ActiveTimes))
		return;

	WORD srcPos = pMsg->Store.wCellPos;

	SPackageItem *srcStore = &pPlayer->m_Property.m_pStorageGoods[srcPos];
	if (0 == srcStore->wIndex)
		return;

	const SItemBaseData *pSrcItemData = CItemService::GetInstance().GetItemBaseData(srcStore->wIndex);
	if (!pSrcItemData)
		return;
	//+ ������߲��ܷ���ֿ�
	if (ITEM_IS_TASK(pSrcItemData->m_Type))
	{
		return ;
	}

	// ��ҵ���Ҽ��Զ����뱳��
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

	// �����ߵ���Ч��
	WORD destPos = pMsg->Pocket.wCellPos;

	if (!pPlayer->IsItemPosValid(pMsg->Pocket.wCellPos, false, true))
		return;

	SPackageItem *dest = pPlayer->FindItemByPos(pMsg->Pocket.wCellPos, XYD_FT_ONLYUNLOCK);

	if (dest && ((0 != dest->wIndex && (dest->wIndex != srcStore->wIndex)) || (dest->overlap == pSrcItemData->m_Overlay)))
	{
		TalkToDnid(pPlayer->m_ClientIndex, "Ŀ���������Ͳ�ͬ�����ѵ��������ޣ�");
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

	// �����������Ч��
	// ���ԭ�������Ч��
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

	// Ŀ��λ��Ϊ��
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
			// ֱ�ӽ���
			des->wCellPos = pMsg->StoreSrc.wCellPos;
			src->wCellPos = pMsg->StoreDest.wCellPos;

			SPackageItem temp	 = pPlayer->m_Property.m_pStorageGoods[srcPos];
			pPlayer->m_Property.m_pStorageGoods[srcPos] = pPlayer->m_Property.m_pStorageGoods[desPos];
			pPlayer->m_Property.m_pStorageGoods[desPos] = temp;
		}
		else
		{
			// ����
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
    // ��ǰ��û�д�����ֿ�
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

	// ��֤ʱ����
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

	// �ȿ�ʼ�ϲ�����
	for (int i=0; i<index; i++)
	{
		if (!pPackage[i])
			continue;

		const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(pPackage[i]->wIndex);
		MY_ASSERT(itemData);

		// ����޷����ӻ��������������������
		if (itemData->m_Overlay == pPackage[i]->overlap)
			continue;

		WORD StillCon = itemData->m_Overlay - pPackage[i]->overlap;

		for (int j=i+1; j<index; j++)
		{
			if (!pPackage[j])
				continue;

			// �ж��Ƿ���Ե�����pPackage[i]
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

	// ��ʼ����ÿһ������
	for (std::map<WORD, std::list<WORD> >::iterator it = Level1Info.begin(); it != Level1Info.end(); ++it)
	{
		std::map<DWORD, std::list<WORD> > Level2Info;

		for (std::list<WORD>::iterator ti = it->second.begin(); ti != it->second.end(); ++ti)
		{
			Level2Info[final[*ti].wIndex].push_back(*ti);
		}

		// ��ʼ�����ڶ���
		for (std::map<DWORD, std::list<WORD> >::iterator it3 = Level2Info.begin(); it3 != Level2Info.end(); ++it3)
		{
			// ��ʼִ�в���
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

//+��ʱ���һ����ȡ�ֿ���Ч��Χ�ĺ��������⵽�����õ������֣����滻 
// Ŀǰ��ʼ36��Ȼ��ÿ����һ�δ�12����ӣ���༤��3��
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