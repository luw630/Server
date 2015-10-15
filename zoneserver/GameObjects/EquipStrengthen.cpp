#include "StdAfx.h"
#include "./EquipStrengthen/EquipStrengthen.h"

#include "Random.h"
#include "Player.h"
#include "Networkmodule\ItemMsgs.h"
#include ".\CItemDefine.h"

CEquipStrengthenServer::CEquipStrengthenServer()
{
}

CEquipStrengthenServer::~CEquipStrengthenServer()
{
}

// ������϶���Ʒ�Ļ�����Ϣ�Ƿ������������������ͣ��Ƿ�ɲ���
bool CEquipStrengthenServer::checkCanDragStrengthenEquip(BYTE &MsgFlag, BYTE byRefineType, const struct SEquipment *pEquip, const struct SItemBaseData *srcData)
{
	bool bRetFailed = false;
	if (!pEquip)
	{
		return bRetFailed;
	}

	BOOL Flag = FALSE;
	int index = 0;
	switch (byRefineType)
	{
	case SQAddEquip::ADE_REFINE_GRADE:
		// �ж�װ�����׵Ľ���
		if (pEquip->attribute.grade < 0 || pEquip->attribute.grade >= SEquipment::MAX_GRADE_NUM )
		{
			MsgFlag = SAAddEquip::ADE_REFINE_GRADE_ERROR; bRetFailed = true;			
		}
		if ( !srcData->m_CanGrade )
		{
			MsgFlag = SAAddEquip::ADE_No_Grade; bRetFailed = true;
		}
		break;
	case SQAddEquip::ADE_REFINE_STAR:
		// �ж�װ�������Ƿ��Ѿ�ȫ������
		if(!srcData->m_CanStar)
		{
			MsgFlag = SAAddEquip::ADE_REFINE_STAR_Equip; bRetFailed = true; break;
		}		
		if (CEquipStrengthenServer::GetEquipStarNum(pEquip) == SEquipment::MAX_STAR_NUM)
		{
			MsgFlag = SAAddEquip::ADE_REFINE_STAR_FULL; bRetFailed = true; break;		
		}
		break;
	case SQAddEquip::ADE_ReMove_STAR:
		if (!srcData->m_CanUnStar)
		{
			MsgFlag = SAAddEquip::ADE_REFINE_ReSTAR_Equip; bRetFailed = true; break;			
		}
		if (CEquipStrengthenServer::GetEquipStarNum(pEquip) == 0)
		{
			MsgFlag = SAAddEquip::ADE_REFINE_ReSTAR_Empty; bRetFailed = true; break;			
		}
		break;
	case SQAddEquip::ADE_UPDATE_QUALITY:
		// �ж���Ʒ���Ƿ��Ѿ��ǽ�ɫװ��
		if (IC_GOLD == srcData->m_Color)
		{
			MsgFlag = SAAddEquip::ADE_UPDATE_QUALITY_1; bRetFailed = true; break;
		}
		if (!srcData->m_CanQuality)
		{
			MsgFlag = SAAddEquip::ADE_UPDATE_QUALITY_2; bRetFailed = true; break;
		}
		break;
	case SQAddEquip::ADE_ADD_HOLE://����
		if (!srcData->m_CanMakeHole)
		{
			MsgFlag = SAAddEquip::ADE_EQUIP_NoMakeHole; bRetFailed = true; break;
		}
		index = 0;
		Flag = FALSE;
		for (; index < SEquipment::MAX_SLOTS; ++index)
		{
			if (pEquip->slots[index].isInvalid())
			{
				Flag = TRUE;
				break;
			}
		}
		if ( !Flag)
		{
			MsgFlag = SAAddEquip::ADE_EQUIP_MakeHoleFUll; bRetFailed = true; break;
		}
		break;
	case SQAddEquip::ADE_GEM_INSERT:
		index = 0;
		Flag = FALSE;
		for (; index < SEquipment::MAX_SLOTS; ++index)
		{
			if (pEquip->slots[index].isEmpty())
			{
				Flag = TRUE;
				break;
			}
		}
		if ( !Flag )
		{
			MsgFlag = SAAddEquip::ADE_EQUIP_MakeGemNoEmpty; bRetFailed = true; break;			
		}
		break;
	case SQAddEquip::ADE_GEM_REMOVE: //ժ��
		index = SEquipment::MAX_SLOTS - 1;
		Flag = FALSE;
		for (;	index >= 0 ; --index)
		{
			if (pEquip->slots[index].isJewel())
			{
				Flag = TRUE;
				break;
			}
		}
		if (!Flag)
		{
			MsgFlag = SAAddEquip::ADE_EQUIP_NoFindGem; bRetFailed = true; break;
		}
		break;
	case SQAddEquip::ADE_UPDATE_LEVEL:
		// �ж������Ƿ��Ѿ������ȼ�
		if (!srcData->m_CanUpdate)
		{
			MsgFlag = SAAddEquip::ADE_UPDATE_NoCanLEVEL; bRetFailed = true; break;			
		}
		break;
	case SQAddEquip::ADE_EQUIP_RESET:
		if (!srcData->m_CanRefine)// �ж�װ��ϴ��
		{
			MsgFlag = SAAddEquip::ADE_NORESET; bRetFailed = true; break;	
		}
		break;
	case SQAddEquip::ADE_EQUIP_REFINE://װ������
		if (!srcData->m_CanRefresh)
		{
			MsgFlag = SAAddEquip::ADE_NORFRESH; bRetFailed = true; break;			
		}
		break;
	case SQAddEquip::ADE_EQUIP_DECOMPOSITION:
		if (!srcData->m_CanEquipDescomposition)
		{
			MsgFlag = SAAddEquip::ADE_EQUIP_NOMakeDECOMPOSITION; bRetFailed = true; break;
		}
		break;
	case SQAddEquip::ADE_EQUIP_SMELTING:
		if (!srcData->m_CanEquipSmelting)
		{
			MsgFlag = SAAddEquip::ADE_EQUIP_NOMakeSmelting; bRetFailed = true; break;
		}
		break;
	case SQAddEquip::ADE_EQUIP_SPIRITATTACHBODY: // �Ƿ�Ӧ�����һ�������鸽		
		break;
	default:
		MsgFlag = SAAddEquip::ADE_NONE; bRetFailed = true; break;			
		break;
	}		

	return bRetFailed;
}

// ��������װ��
BOOL CEquipStrengthenServer::handleElemDragStrengthenEquip(CPlayer *pPlayer, const struct SEquipment* pEquip, BYTE byRefineType)
{
	if (!pPlayer || !pEquip)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvDragStrengthenEquip() - !pEquip");
		return FALSE;
	}

	const SItemBaseData *srcData = CItemService::GetInstance().GetItemBaseData(pEquip->wIndex);
	if (!srcData)
		return FALSE;
	

	SAAddEquip pSaddEquipMsg;
	if (checkCanDragStrengthenEquip(pSaddEquipMsg.Flag, byRefineType, pEquip, srcData))
	{
		g_StoreMessage(pPlayer->m_ClientIndex,&pSaddEquipMsg,sizeof(SAAddEquip));
		return FALSE;
	}


	
	// �ᵽ����ȥ����
	//WORD wPos = byItemPosX * PackageWidth + byItemPosY;
	//pPlayer->m_vecStrengthenEquip.push_back(wPos);	
	//pPlayer->LockItemCell(wPos, true);
	
	return TRUE;
}


BOOL CEquipStrengthenServer::RecvDragStrengthenEquip(CPlayer *pPlayer, struct SQAddEquip* pMsg)
{
	
	return TRUE;
}

// ����װ���ֽ�
BOOL CEquipStrengthenServer::handleElemEquipDecomposition(CPlayer *pPlayer, const struct SEquipment *pEquip, BYTE byRefineType, WORD wEquipPos )
{	
	if (!pEquip)
	{
		rfalse(4, 1, "[ItemUser] - RecvBeginDecomposition");
		return FALSE;
	}	
	if (pPlayer->m_refineType != SQAddEquip::ADE_EQUIP_DECOMPOSITION || byRefineType != SQBeginIdentifyMsg::QBIM_FENJIE)
	{
		return FALSE;
	}
	
	SABeginIdentifyMsg msg;
	msg.type = SQBeginIdentifyMsg::QBIM_FENJIE;
	const SItemBaseData *srcData = CItemService::GetInstance().GetItemBaseData(pEquip->wIndex);
	if (!srcData)
		return FALSE;

	byte byStarNum = CEquipStrengthenServer::GetInstance().GetEquipStarNum(pEquip);
	const SEquipDecomposition *pEDecompose = CItemService::GetInstance().GetSEquipDecompositionInfo(srcData, byStarNum);
	if (!pEDecompose)
	{
		rfalse(2, 1, "Non Find SEquipDecomposition itemid=%d starnum=%d", pEquip->wIndex, byStarNum);
		return FALSE;
	}

	if (0 == pPlayer->m_vecStrengthenEquip.size())
	{
		msg.result = SABeginIdentifyMsg::BIM_NOEQUIP;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginIdentifyMsg));
		return FALSE;
	}

	std::vector<SPackageItem*> vec;
	if (SQBeginIdentifyMsg::QBIM_FENJIE == byRefineType) 
	{
		if (srcData->m_Color == IC_WHITE)
		{
			return FALSE;
		}
		if (!srcData->m_CanEquipDescomposition)
		{
			return FALSE;
		}
		if (pEDecompose->lNeedMoney > pPlayer->GetPlayerAllMoney())
		{
			msg.result = SABeginIdentifyMsg::BIM_NOMONEY;
			g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginIdentifyMsg));
			pPlayer->OnthSysNotice(2);
			return FALSE;
		}
	}

	CItemUser::TItemPosType ePosType = CItemUser::GetItemPosType(wEquipPos);
	// �������Ƿ��㹻�ռ䣨���ֻ����һ�ֲ������ݵĻ��ǲ�����ֿռ䲻�㣬��Ϊ��ɾ��һ����Ȼ�����һ�������Ҫ�������ֲ��ϣ�����Ҫ�ж��ˡ���
	bool bCanAdd = true;
	std::list<SAddItemInfo> listAddItem;
	{
		if (0 != pEDecompose->byGenNormalItemNum )
		{
			listAddItem.push_back(SAddItemInfo(pEDecompose->dwGenNormalItemID, pEDecompose->byGenNormalItemNum));
		}
		if (0 != pEDecompose->byGenSpecialItemNum)
		{
			listAddItem.push_back(SAddItemInfo(pEDecompose->dwGenSpecialItemID, pEDecompose->byGenSpecialItemNum));
		}				
		int	 iGenListSize = listAddItem.size();
		if (iGenListSize == 0)
		{
			bCanAdd = false;
		}
		else if (iGenListSize == 1) //����Ҫ��飬װ��������ӵ�
		{		
			bCanAdd = false;
			if (ePosType == CItemUser::EPackage_PosType)
			{
				bCanAdd = true;
			}
			else if (pPlayer->CanAddItems(listAddItem))
			{
				bCanAdd = true;
			}									
		}
		else if (iGenListSize > 1)
		{
			SPackageItem tmpPackItem = *((SPackageItem *)(SItemBase*)pEquip);
			if (!pPlayer->CanAddItems(listAddItem))
			{				
				if (ePosType == CItemUser::EPackage_PosType)
				{
					std::list<SAddItemInfo> tmpListFirst; tmpListFirst.push_back(listAddItem.front());			
					std::list<SAddItemInfo> tmpListSecond; tmpListSecond.push_back(listAddItem.back());
					bCanAdd = pPlayer->CanAddItems(tmpListFirst) || pPlayer->CanAddItems(tmpListSecond);					
				}
				else 
				{
					bCanAdd = false;
				}
				
			}
			else 
			{
				bCanAdd = true;
			}
		}
	}	
	if (!bCanAdd)
	{
		msg.result = SABeginIdentifyMsg::BIM_PACKAGE_FULL;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginIdentifyMsg));
		return FALSE;
	}

	bool bHaveBindMetrial = false;
	if (!pPlayer->ReducedCommonMoney(pEDecompose->lNeedMoney))
	{
		msg.result = SABeginIdentifyMsg::BIM_NOMONEY;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginIdentifyMsg));
		return FALSE;
	}
	if (bCanAdd)
	{
		switch (ePosType)
		{
		case CItemUser::EPackage_PosType:
			{
				const SPackageItem *ptmpPackItem = ((SPackageItem *)(SItemBase*)pEquip);		
				if (NULL != ptmpPackItem)
				{
					pPlayer->DelItem(*ptmpPackItem, "��װ���ֽ���ʾ���ɹ��ֽ⣬ɾ��ԭװ����", true);
				}				
			}
			break;
		case CItemUser::EEquipColumn_PosType:
			{		
				pPlayer->LockItemCell(wEquipPos, false);
				pPlayer->ToPackage(wEquipPos-pPlayer->sciEquipColumnBaseLine, TRUE);
			}
			break;
		default:
			return FALSE;		
		}
		pPlayer->StartAddItems(listAddItem);
		pPlayer->OnthSysNotice(16);	
	}

	return bCanAdd;
}


// ����װ������
BOOL CEquipStrengthenServer::handleElemEquipSmelting(CPlayer *pPlayer, const struct SEquipment *pEquip, BYTE byRefineType, WORD wEquipPos)
{
	/*
	if (!pEquip)
	{
		rfalse(4, 1, "[ItemUser] - RecvBeginSmelting");
		return FALSE;
	}	
	if (pPlayer->m_refineType != SQAddEquip::ADE_EQUIP_SMELTING || byRefineType != SQBeginIdentifyMsg::QBIM_SMELTING)
	{
		return FALSE;
	}	
	SABeginIdentifyMsg msg;
	msg.type = SQBeginIdentifyMsg::QBIM_SMELTING;
	const SItemBaseData *srcData = CItemService::GetInstance().GetItemBaseData(pEquip->wIndex);
	if (!srcData)
		return FALSE;

	const SEquipSmelting *pSESmelt = CItemService::GetInstance().GetSEquipSmeltingInfo(srcData->m_Color);	
	if (!pSESmelt)
	{
		rfalse(2, 1, "Non Find SEquipSmelting itemid=%d", pEquip->wIndex);
		return FALSE;
	}

	if (0 == pPlayer->m_vecStrengthenEquip.size())
	{
		msg.result = SABeginIdentifyMsg::BIM_NOEQUIP;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginIdentifyMsg));
		return FALSE;
	}

	byte byStarNum = CEquipStrengthenServer::GetInstance().GetEquipStarNum(pEquip);
	if (pSESmelt->byStarNum > byStarNum)
	{
		msg.result = SABeginIdentifyMsg::BIM_STAR_LESS;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginIdentifyMsg));
		return FALSE;
	}

	std::vector<SPackageItem*> vec;
	if (SQBeginIdentifyMsg::QBIM_SMELTING == byRefineType) 
	{
		if (srcData->m_Color == IC_WHITE)
		{
			return FALSE;
		}
		if (!srcData->m_CanEquipSmelting)
		{
			return FALSE;
		}
		if (pSESmelt->lNeedMoney > pPlayer->GetPlayerAllMoney())
		{
			msg.result = SABeginIdentifyMsg::BIM_NOMONEY;
			g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginIdentifyMsg));
			pPlayer->OnthSysNotice(2);
			return FALSE;
		}
	}

	CItemUser::TItemPosType ePosType = CItemUser::GetItemPosType(wEquipPos);
	// �������Ƿ��㹻�ռ䣨���ֻ����һ�ֲ������ݵĻ��ǲ�����ֿռ䲻�㣬��Ϊ��ɾ��һ����Ȼ�����һ�������Ҫ�������ֲ��ϣ�����Ҫ�ж��ˡ���
	bool bCanAdd = true;
	std::list<SAddItemInfo> listAddItem;
	{
		if (0 != pSESmelt->byGenNormalItemNum )
		{
			listAddItem.push_back(SAddItemInfo(pSESmelt->dwGenNormalItemID, pSESmelt->byGenNormalItemNum));
		}		
		int	 iGenListSize = listAddItem.size();
		if (iGenListSize == 0)
		{
			bCanAdd = false;
		}
		else if (iGenListSize == 1) //����Ҫ��飬װ��������ӵ�
		{		
			bCanAdd = false;
			if (ePosType == CItemUser::EPackage_PosType)
			{
				bCanAdd = true;
			}
			else if (pPlayer->CanAddItems())
			{
				bCanAdd = true;			
			}			
		}		
	}	
	if (!bCanAdd)
	{
		msg.result = SABeginIdentifyMsg::BIM_PACKAGE_FULL;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginIdentifyMsg));
		return FALSE;
	}

	bool bHaveBindMetrial = false;
	if (!pPlayer->ReducedCommonMoney(pSESmelt->lNeedMoney))
	{
		msg.result = SABeginIdentifyMsg::BIM_NOMONEY;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginIdentifyMsg));
		return FALSE;
	}
	// װ������	
	if (bCanAdd)
	{
		switch (ePosType)
		{
		case CItemUser::EPackage_PosType:
			{
				const SPackageItem *ptmpPackItem = ((SPackageItem *)(SItemBase*)pEquip);		
				if (NULL != ptmpPackItem)
				{
					pPlayer->DelItem(*ptmpPackItem, "��װ���ֽ���ʾ���ɹ��ֽ⣬ɾ��ԭװ����", true);
				}				
			}
			break;
		case CItemUser::EEquipColumn_PosType:
			{
				pPlayer->LockItemCell(wEquipPos, false);
				pPlayer->ToPackage(wEquipPos-pPlayer->sciEquipColumnBaseLine, TRUE);
			}
			break;
		default:
			return FALSE;		
		}
		pPlayer->StartAddItems(listAddItem);			
		pPlayer->OnthSysNotice(16);
	}
	return bCanAdd;//*/
return 0;
}


// ����װ������
BOOL CEquipStrengthenServer::handleElemEquipSpiritAttachBody(CPlayer *pPlayer, struct SEquipment *const pEquip, BYTE byRefineType, WORD wEquipPos, WORD wScrollPos) 
{
	if (!pPlayer || !pEquip)
	{
		return FALSE;
	}
	SABeginIdentifyMsg msg;
	SPackageItem *srcItem = pPlayer->FindItemByPos(wScrollPos, XYD_FT_ONLYLOCK);
	if (!srcItem) 
	{
		return FALSE;
	}	
	WORD wSuitEquipID = CItemService::GetInstance().GetSuitEquipIDbyScrollID(srcItem->wIndex);
	if (0 == wSuitEquipID)
	{
		return FALSE;
	}
	const SEquipSpiritAttachBodyAttr *pSESABA = CItemService::GetInstance().GetSEquipSpiritAttachBodyAttri(wSuitEquipID);
	if (NULL == pSESABA)
	{
		return FALSE;
	}			
	
	// ���ȼ��Ƿ��㹻
	const SItemBaseData *pcEquipItem = CItemService::GetInstance().GetItemBaseData(pEquip->wIndex);
	if (NULL == pcEquipItem || pcEquipItem->m_Level < wSuitEquipID)
	{
		msg.result = SABeginIdentifyMsg::BIM_LEVEL_LESS;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginIdentifyMsg));
		pPlayer->OnthSysNotice(2);
		return FALSE;
	}

	// ���Ǯ�Ƿ��㹻
	if (pSESABA->lNeedMoney > pPlayer->GetPlayerAllMoney())
	{
		msg.result = SABeginIdentifyMsg::BIM_NOMONEY;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginIdentifyMsg));
		pPlayer->OnthSysNotice(2);
		return FALSE;
	}

	// ��Ǯ�Ƿ��㹻
	if (!pPlayer->ReducedCommonMoney(pSESABA->lNeedMoney))
	{
		msg.result = SABeginIdentifyMsg::BIM_NOMONEY;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginIdentifyMsg));
		return FALSE;
	}	

	// �˴�Ӧ��ֱ�ӿ۶�Ӧ������
	//pPlayer->LockItemCell(byItemPosX, byItemPosY, false);
//	pPlayer->DelItem(*srcItem, "ɾ���鸽����", true);
// 	std::vector<SPackageItem*> vec;
// 	// �������Ƿ��㹻
// 	long nFlg = pPlayer->OnGetNeedMetrial(pPlayer, pSESABA->byNeedItemNum,vec, pSESABA->dwNeedItemID, XYD_FT_WHATEVER);
// 	if (nFlg != 1)
// 	{
// 		msg.result = SABeginIdentifyMsg::BIM_DefineMetrial_Less;
// 		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginIdentifyMsg));
// 		pPlayer->OnthSysNotice(1);
// 		return FALSE;
// 	}

	// ��Ǯ�Ͳ���
	bool bHaveBindMetrial = false;
	if (!pPlayer->ReducedCommonMoney(pSESABA->lNeedMoney))
	{
		msg.result = SABeginIdentifyMsg::BIM_NOMONEY;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginIdentifyMsg));
		return FALSE;
	}	
	pPlayer->LockItemCell(srcItem->wCellPos, false);
	WORD TempNum = pSESABA->byNeedItemNum;
	if (srcItem->overlap > TempNum)
	{
		// [2012-8-21 19-27 gw: +����Ʒ��ʹ��changeoverlapͳһ����]
		pPlayer->ChangeOverlap(srcItem, TempNum, false, true);
		// [2012-8-21 19-27 gw: -ʹ������ķ�ʽû�ж�m_ItemsState�Ĵ���]
		//item->overlap -= TempNum;
		//pPlayer->SendItemSynMsg(srcItem, XYD_FT_ONLYUNLOCK);		
	}
	else if (srcItem->overlap <= TempNum)
	{	
		TempNum -= srcItem->overlap;
		pPlayer->DelItem(*srcItem, "ɾ���鸽����", false);
	}			
	//pPlayer->OnProcessNeedMetrial(pPlayer, vec, pSESABA->byNeedItemNum,"��װ��ˢ����ʾ��ԭ�������ģ�",bHaveBindMetrial);

//	pEquip->attribute.wSuitEquipID = wSuitEquipID;
	pPlayer->SendUpdateEquipInfo(pEquip);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// @msg: ���е���Ϣ��Ŷ��
// @newMetrial: ��Ҫ�²��������
// @bMerge: �Ƿ�ϲ�����-����ҵ��Ժ���ϺͲ������ID��������ͬ�Ļ��ͺϲ�����,���û���ҵ������շ��������� ��-�¼�һ�������ں���
//////////////////////////////////////////////////////////////////////////
BOOL CEquipStrengthenServer::insertMetrialItem2NeedMetrial(struct SANeedMETRIAL *msg, struct MetrialItem *newMetrial, BOOL bMerge/* = TRUE*/)
{
	if (!msg || !newMetrial)
	{
		return FALSE;
	}
	int iLen = msg->byMetrialNum > SANeedMETRIAL::msc_iMetrialMaxLen ? SANeedMETRIAL::msc_iMetrialMaxLen : msg->byMetrialNum;
	int iFindIndex = -1;
	if (bMerge)
	{
		for (int i = 0; i < iLen; ++i)
		{
			if (msg->aMetrialItem[i].dwID == newMetrial->dwID && msg->aMetrialItem[i].byMetrialType == newMetrial->byMetrialType)
			{
				iFindIndex = i;
				break;
			}
		}
	}
	if (iFindIndex >= 0)
	{
		msg->aMetrialItem[iFindIndex].byNum += newMetrial->byNum;
		// todo.. �ɹ���
	}
	else 
	{
		if (msg->byMetrialNum >= SANeedMETRIAL::msc_iMetrialMaxLen)
		{
			rfalse(2, 1, "insertMetrialItem2NeedMetrial is error.");
			return FALSE;
		}
		msg->aMetrialItem[msg->byMetrialNum++] = *newMetrial;
	}
	return TRUE;	
}
// �����������
BOOL CEquipStrengthenServer::handleElemNeedMETRIALInfo(CPlayer *pPlayer, const struct SEquipment *pEquip, BYTE byRefineType, struct SANeedMETRIAL &msg )
{
	if (!pEquip || !pPlayer)
		return FALSE;	
	BYTE	byTmpTypeNum = msg.byMetrialNum;	

	const SItemBaseData *srcData = CItemService::GetInstance().GetItemBaseData(pEquip->wIndex);
	if (!srcData)
		return FALSE;
	switch (byRefineType)
	{
	case SQAddEquip::ADE_UPDATE_QUALITY:
		{
			const SItemUpdateQuality* pQualityData = CItemService::GetInstance().GetUpdateQuality(srcData->m_Color);
			if (!pQualityData)
				return FALSE;		
			if (pQualityData->bStoneNum > 0)
			{
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pQualityData->NeedID
					, 10000
					, pQualityData->bStoneNum
					, SANeedMETRIAL::EMETR_NEED_COMMON));					
			}					
			msg.MoneyType = pQualityData->m_Type;
			msg.bType = byRefineType;
			msg.NeedMoney = pQualityData->dwNeedMoney;		
			pPlayer->m_MetrialItemID = pQualityData->NeedID;
		}
		break;
	case SQAddEquip::ADE_UPDATE_LEVEL:
		{
			const SEquipLevelUp * pEquipLevel = CItemService::GetInstance().GetLevelUpData(srcData->m_Level);
			if (!pEquipLevel)
			{
				return FALSE;
			}		
			if (pEquipLevel->StoneNum > 0)
			{
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pEquipLevel->NeedID
					, 10000
					, pEquipLevel->StoneNum
					, SANeedMETRIAL::EMETR_NEED_COMMON));					
			}		
			msg.MoneyType = pEquipLevel->m_Type;
			msg.bType = byRefineType;
			msg.NeedMoney = pEquipLevel->NeedMoney;		
			pPlayer->m_MetrialItemID = pEquipLevel->NeedID;
		}
		break;
	case SQAddEquip::ADE_REFINE_GRADE: // ����
		{
			const SItemUpdateGradeInfo * pUpdateGrade = CItemService::GetInstance().GetUpgradeInfo(pEquip->attribute.grade);
			if (!pUpdateGrade)
			{
				return FALSE;
			}
			if (pUpdateGrade->byNeedItemNum > 0)
			{
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pUpdateGrade->dwNeedItemID
					, 10000
					, pUpdateGrade->byNeedItemNum
					, SANeedMETRIAL::EMETR_NEED_COMMON));				
			}		
			// ���˷�����
			if (pUpdateGrade->byLuckyCharmNum > 0)
			{
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pUpdateGrade->dwLuckyCharmID
					, pUpdateGrade->wLuckyCharmAddSuccessRate
					, pUpdateGrade->byLuckyCharmNum
					, SANeedMETRIAL::EMETR_NEED_LUCKYCHARM));					
			}		
			msg.bType = byRefineType;		
			msg.MoneyType = pUpdateGrade->byMoneyType;		
			msg.NeedMoney = pUpdateGrade->needMoney;
			msg.wSuccessRate = pUpdateGrade->baseSuccessRate;

			pPlayer->m_MetrialItemID = pUpdateGrade->dwNeedItemID;
		}
		break;
	case SQAddEquip::ADE_EQUIP_RESET:
		{
			const SItemUpgradeAttribute *pUpgradeAttri = CItemService::GetInstance().GetUpgradeAttribute(srcData);
			if (!pUpgradeAttri)
			{
				return FALSE;
			}		
			if (pUpgradeAttri->equipRefineRequire.ResetNeedNum > 0)
			{
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pUpgradeAttri->equipRefineRequire.ResetItemID
					, 10000
					, pUpgradeAttri->equipRefineRequire.ResetNeedNum
					, SANeedMETRIAL::EMETR_NEED_COMMON));	
			}		
			// ����ʯ����
			if (pUpgradeAttri->equipRefineRequire.ExternLockeItemNum > 0)
			{
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pUpgradeAttri->equipRefineRequire.ExternLockedItemID
					, 10000
					, pUpgradeAttri->equipRefineRequire.ExternLockeItemNum
					, SANeedMETRIAL::EMETR_NEED_PROTECTION));				
			}					

			msg.MoneyType	  = pUpgradeAttri->equipRefineRequire.ResetMoneyType;
			msg.bType = byRefineType;
			msg.NeedMoney = pUpgradeAttri->equipRefineRequire.ResetNeedMoney;
			pPlayer->m_MetrialItemID = pUpgradeAttri->equipRefineRequire.ResetItemID;
		}
		break;
	case SQAddEquip::ADE_EQUIP_REFINE:
		{// װ���ľ���
			const SItemUpgradeAttribute *pUpgradeAttri = CItemService::GetInstance().GetUpgradeAttribute(srcData);
			if (!pUpgradeAttri)
			{
				return FALSE;
			}
			if (pUpgradeAttri->equipRefineRequire.RefreshNeedNum > 0)
			{
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pUpgradeAttri->equipRefineRequire.RefreshItemID
					, 10000
					, pUpgradeAttri->equipRefineRequire.RefreshNeedNum
					, SANeedMETRIAL::EMETR_NEED_COMMON));				
			}
			
			msg.MoneyType     = pUpgradeAttri->equipRefineRequire.RefreshMoneyType;
			msg.bType = byRefineType;
			msg.NeedMoney = pUpgradeAttri->equipRefineRequire.RefreshNeedMoney;		
			pPlayer->m_MetrialItemID = pUpgradeAttri->equipRefineRequire.RefreshItemID;
		}
		break;
	case SQAddEquip::ADE_REFINE_STAR:
		{
			int index = 0;	// ���ǵľ��Ը���
			// �ж����ǵĸ���
			for (; index < SEquipment::MAX_STAR_NUM; ++index)//
			{
//				if (SEquipment::SLOT_IS_EMPTY == pEquip->attribute.starNum[index])
				{
					break;
				}
			}
			const SEquipStar* pStar = CItemService::GetInstance().GetEquipStarData(index);
			if (!pStar)
			{
				return FALSE;
			}
			if (pStar->StarNum > 0)
			{
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pStar->StarNeedID
					, 10000
					, pStar->StarNum
					, SANeedMETRIAL::EMETR_NEED_COMMON));				
			}		
			if (pStar->byLuckyCharmNum > 0)
			{
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pStar->dwLuckyCharmID
					, pStar->wLuckyCharmAddSuccessRate
					, pStar->byLuckyCharmNum
					, SANeedMETRIAL::EMETR_NEED_LUCKYCHARM));				
			}
			if (pStar->byProtectionCharacterNum > 0)
			{
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pStar->dwProtectionCharacterID, 10000, pStar->byProtectionCharacterNum, SANeedMETRIAL::EMETR_NEED_PROTECTION));				
			}
			msg.MoneyType     = pStar->StarMoneyType;
			msg.bType = byRefineType;
			msg.NeedMoney = pStar->money;
			msg.wSuccessRate = 0;

			pPlayer->m_MetrialItemID = pStar->StarNeedID;
		}
		break;
	case SQAddEquip::ADE_EQUIP_SMELTING:
		{
			const SEquipSmelting *pSES = CItemService::GetInstance().GetSEquipSmeltingInfo(srcData->m_Color);
			if (!pSES)
			{
				rfalse(2, 1, "Non Find SEquipSmelting itemid=%d", pEquip->wIndex);
				return FALSE;
			}			
			if (pSES->byGenNormalItemNum > 0)
			{
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pSES->dwGenNormalItemID
					, 10000
					, pSES->byGenNormalItemNum
					, SANeedMETRIAL::EMETR_PRODUCE_COMMON), FALSE);				
			}			
			msg.bType = byRefineType; 
			msg.MoneyType = pSES->byMoneyType;
			msg.NeedMoney = pSES->lNeedMoney;

			pPlayer->m_MetrialItemID = 0;
		}
		break;
	case SQAddEquip::ADE_EQUIP_DECOMPOSITION:
		{
			byte byStarNum = 0;
			for (int i = 0; i < SEquipment::MAX_STAR_NUM; ++i)
			{
//				if (pEquip->attribute.starNum[i] == SEquipment::SLOT_IS_VALID)
				{
					++byStarNum;
				}
			}		
			const SEquipDecomposition *pSED = CItemService::GetInstance().GetSEquipDecompositionInfo(srcData, byStarNum);
			if (!pSED)
			{
				rfalse(2, 1, "Non Find SEquipDecomposition itemid=%d, starnum=%d", pEquip->wIndex, byStarNum);
				return FALSE;
			}				
			if (pSED->byGenNormalItemNum > 0)
			{
				// [2012-9-1 15-49 gw: +�˴��Ĺ���-������ͨ���ϵ�ID��������ͨ���ϵ�������������ͨ���ϵĳɹ���] 		
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pSED->dwGenNormalItemID
					, pSED->wGenNormalItemRate
					, pSED->byGenNormalItemNum
					, SANeedMETRIAL::EMETR_PRODUCE_COMMON));				
			}
			if (pSED->byGenSpecialItemNum > 0)
			{
				// [2012-9-1 15-50 gw: +�˴��Ĺ���-����������ϵ�ID������������ϵ�����������������ϵĳɹ���]	
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pSED->dwGenSpecialItemID
					, pSED->wGenSpecialItemRate
					, pSED->byGenSpecialItemNum
					, SANeedMETRIAL::EMETR_PRODUCE_SPECIAL));				
			}		

			msg.bType = byRefineType;
			msg.MoneyType = pSED->byMoneyType;
			msg.NeedMoney = pSED->lNeedMoney;

			pPlayer->m_MetrialItemID = 0;
		}
		break;
	case SQAddEquip::ADE_EQUIP_SPIRITATTACHBODY:
		{ // �鸽��η�����Ϣ
			if (pPlayer->m_vecStrengthenEquipAssist.size() == 0)
			{ //  ��û���Ͼ���
				return TRUE;
			}
			WORD wItemPos = pPlayer->m_vecStrengthenEquipAssist.at(0);
			SPackageItem *pIB = pPlayer->FindItemByPos(wItemPos, XYD_FT_WHATEVER);
			WORD wSuitEquipID = CItemService::GetInstance().GetSuitEquipIDbyScrollID(pIB->wIndex);
			const SEquipSpiritAttachBodyAttr *pSABA = CItemService::GetInstance().GetSEquipSpiritAttachBodyAttri(wSuitEquipID);
			if (!pSABA)
			{
				rfalse(2, 1, "Non Find SEquipSpiritAttachBodyAttr itemid=%d, wSuitEquipID=%d", pIB->wIndex, wSuitEquipID);
				return FALSE;
			}				
			if (pSABA->byNeedItemNum > 0)
			{
				// [2012-9-1 15-49 gw: +�˴��Ĺ���-������ͨ���ϵ�ID��������ͨ���ϵ�������������ͨ���ϵĳɹ���] 		
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pSABA->wSuitEquipID
					, 10000
					, 1
					, SANeedMETRIAL::EMETR_PRODUCE_COMMON));				
			}				
			msg.bType = byRefineType;
			msg.MoneyType = pSABA->byMoneyType;
			msg.NeedMoney = pSABA->lNeedMoney;

			pPlayer->m_MetrialItemID = pSABA->dwNeedItemID;
		}
		break;
	case SQAddEquip::ADE_ReMove_STAR:
		{
			int index = SEquipment::MAX_STAR_NUM - 1;	// ���ǵľ��Ը���
			// �ж����ǵĸ���
			for (; index >= 0  ; --index)//
			{
//				if (SEquipment::SLOT_IS_INVALID == pEquip->attribute.starNum[index] || 
	//				SEquipment::SLOT_IS_VALID == pEquip->attribute.starNum[index])
				{
					break;
				}
			}
			if (index < 0)
			{
				return FALSE;
			}
			const SEquipStar* pStar = CItemService::GetInstance().GetEquipStarData(index);
			if (!pStar)
			{
				return FALSE;
			}		

			insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pStar->ReMoveStarNeedID
				, 10000
				, pStar->StarNum
				, SANeedMETRIAL::EMETR_NEED_COMMON));	
			msg.MoneyType     = pStar->ReMoveStarMoneyType;
			msg.bType = byRefineType;
			msg.NeedMoney = pStar->ReMovemoney;		
			pPlayer->m_MetrialItemID = pStar->ReMoveStarNeedID;

		}
		break;
	case SQAddEquip::ADE_ADD_HOLE:
		{
			int index = 0;
			for (; index < SEquipment::MAX_SLOTS; ++index)
			{
				if ( pEquip->slots[index].isInvalid() )
				{
					break;
				}
			}
			const SMakeHoleInfo * pMakeHole = CItemService::GetInstance().GetMakeHoleInfo(index);
			if (!pMakeHole)
			{
				return 0;
			}		
			if (pMakeHole->HoleNum > 0)
			{
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pMakeHole->HoleNeedID
					, pMakeHole->wMakeSuccessRate
					, pMakeHole->HoleNum
					, SANeedMETRIAL::EMETR_NEED_COMMON));					
			}
			if (pMakeHole->byLuckyCharmNum > 0)
			{
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pMakeHole->dwLuckyCharmID
					, pMakeHole->wLuckyCharmAddSuccessRate
					, pMakeHole->byLuckyCharmNum
					, SANeedMETRIAL::EMETR_NEED_LUCKYCHARM));				
			}
			if (pMakeHole->byProtectionCharacterNum > 0)
			{
				insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pMakeHole->dwProtectionCharacterID, 10000, pMakeHole->byProtectionCharacterNum, SANeedMETRIAL::EMETR_NEED_PROTECTION));				
			}
			msg.MoneyType     = pMakeHole->HoleMoneyType;
			msg.bType = byRefineType;
			msg.NeedMoney = pMakeHole->HoleMoney;
			msg.wSuccessRate = pMakeHole->wMakeSuccessRate;
			pPlayer->m_MetrialItemID = pMakeHole->HoleNeedID;
		}
		break;
	case SQAddEquip::ADE_GEM_INSERT: // ��ʯ��Ƕ
		{
			int index = 0;
			bool Falg = FALSE;
			for (; index < SEquipment::MAX_SLOTS; ++index)
			{
				if (pEquip->slots[index].isEmpty())
				{
					Falg = TRUE;
					break;
				}
			}
			if (!Falg)
			{
				return 0;
			}
			const SInsertGem * pInsertGem = CItemService::GetInstance().GetInsertGemInfo(index);
			if (!pInsertGem)
			{

				return FALSE;
			}
			msg.MoneyType = pInsertGem->GemMoneyType;
			msg.bType = byRefineType;
			msg.NeedMoney = pInsertGem->GemMoney;
		}
		break;
	case SQAddEquip::ADE_GEM_REMOVE:
		{
			int index = SEquipment::MAX_SLOTS - 1;
			for (; index >= 0; --index)
			{
				if (pEquip->slots[index].isJewel())
				{
					const STakeOffGem * pTakeOffGem = CItemService::GetInstance().GetSTakeOffGemInfo(index);
					if (pTakeOffGem)
					{					
						if (pTakeOffGem->TakeOffGemNum > 0)
						{
							insertMetrialItem2NeedMetrial(&msg, &MetrialItem(pTakeOffGem->TakeOffGemItemID
								, 10000
								, pTakeOffGem->TakeOffGemNum
								, SANeedMETRIAL::EMETR_NEED_COMMON));								
						}					
						msg.MoneyType = pTakeOffGem->TakeOffGemMoneyType;
						msg.bType = byRefineType;
						msg.NeedMoney = pTakeOffGem->TakeOffGemMoney;					
						pPlayer->m_MetrialItemID = pTakeOffGem->TakeOffGemItemID;
					}
					break;
				}
			}

		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

// ��������
BOOL CEquipStrengthenServer::handleElemEquipGrade(CPlayer *pPlayer, struct SEquipment *pEquip, BYTE byRefineType, byte byStrengthenExternChoose)
{
	SABeginRefineGrade msg;
	if (!pEquip || !pPlayer)
	{
		msg.result = SABeginRefineGrade::BRG_LACK_METRIAL;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginRefineGrade));
		return FALSE;
	}
	if (pPlayer->m_refineType != SQAddEquip::ADE_REFINE_GRADE)
	{
		return FALSE;
	}

	// ��ȡ�������ñ�
	const SItemUpdateGradeInfo *pData = CItemService::GetInstance().GetUpgradeInfo(pEquip->attribute.grade);
	if (!pData)
		return FALSE;

	// �ж��Ƿ�����
	if (pEquip->attribute.grade >= SEquipment::MAX_GRADE_NUM)
	{
		msg.result = SABeginRefineGrade::BRG_GRADE_FULL;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginRefineGrade));
		return false;
	}

	// ���Ǯ�Ƿ��㹻
	if (pData->needMoney > pPlayer->GetPlayerAllMoney())
	{
		msg.result = SABeginRefineGrade::BRG_NOMONEY;		
		pPlayer->OnthSysNotice(2);
		g_StoreMessage(pPlayer->m_ClientIndex,&msg,sizeof(SABeginRefineGrade));
		return FALSE;
	}

	// ��ȡ�����Ƿ��㹻
	std::vector<SPackageItem*> vec;
	long nFlg = pPlayer->OnGetNeedMetrial(pPlayer,pData->byNeedItemNum,vec,pPlayer->m_MetrialItemID);
	if (nFlg != 1)
	{
		msg.result = SABeginRefineGrade::BRG_LACK_METRIAL;//���ϲ���
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginRefineGrade));
		pPlayer->OnthSysNotice(1);
		return FALSE;
	}


	if (!pPlayer->ReducedCommonMoney(pData->needMoney))
	{
		rfalse(2, 1, "������Ǯ�������� havemoney=%d, needmoney=%d", pPlayer->GetPlayerAllMoney(),  pData->needMoney);
		msg.result = SABeginRefineGrade::BRG_NOMONEY;		
		pPlayer->OnthSysNotice(2);
		g_StoreMessage(pPlayer->m_ClientIndex,&msg,sizeof(SABeginRefineGrade));
		return FALSE;
	}

	bool bHaveBindMetrial = false;
	pPlayer->OnProcessNeedMetrial(pPlayer,vec,pData->byNeedItemNum,"��װ��������ʾ��ԭ�������ģ�",bHaveBindMetrial);

	// [��ȡ���˷����в���ѡ����ʹ�õĻ��۳�һ�����ɹ�������]	
	bool bHaveLuckyCharmItem = false;	
	if (byStrengthenExternChoose & EStrengthenChoose_LuckyCharm)
	{
		std::vector<SPackageItem*> vecLuckyCharm;
		nFlg = pPlayer->OnGetNeedMetrial(pPlayer, pData->byLuckyCharmNum, vecLuckyCharm, pData->dwLuckyCharmID);	
		if (nFlg == 1)
		{
			pPlayer->OnProcessNeedMetrial(pPlayer, vecLuckyCharm, pData->byLuckyCharmNum, "�۳���һ�����˷���", bHaveBindMetrial);
			bHaveLuckyCharmItem = true;
		}
	}
	// ������ʣ����ݸ����ж����׽�� �ɹ�orʧ��or����
	return pPlayer->GetUpgradeResult(pEquip, msg.result, bHaveLuckyCharmItem);
}

// �������ǵ�Ԫ
BOOL CEquipStrengthenServer::handleElemRefineStar(CPlayer *pPlayer, struct SEquipment *pEquip, BYTE byRefineType, byte byStrengthenExternChoose)
{
	SABeginRefineStar msg;
	if (!pPlayer)
		return FALSE;
	if (!pEquip && !pPlayer->m_strengthenMetrial)
	{
		msg.result = SABeginRefineStar::BRS_LACK_METRIAL;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginRefineStar));
		pPlayer->OnthSysNotice(1);
		return FALSE;
	}

	const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(pEquip->wIndex);
	if (!pData)
		return FALSE;

	if (!pData->m_CanStar)
	{
		return FALSE;
	}
	// �ж��Ƿ��пյ�����λ��
	int index = -1;
	for (size_t i = 0; i < SEquipment::MAX_STAR_NUM; ++i)
	{
//		if (SEquipment::SLOT_IS_EMPTY == pEquip->attribute.starNum[i])
		{
			index = i;
			break;
		}
	}

	if (-1 == index)
	{
		rfalse(2, 1, "��װ��ǿ����û�п��õ�λ���������ǡ�");
		return FALSE;
	}

	const SEquipStar *pEquipStar = CItemService::GetInstance().GetEquipStarData(index);
	if (!pEquipStar)
		return FALSE;

	if (CEquipStrengthenServer::GetInstance().GetEquipStarNum(pEquip) == SEquipment::MAX_STAR_NUM)	
	{
		msg.result = SABeginRefineStar::BRS_REFINE_STAR_FULL;
		g_StoreMessage(pPlayer->m_ClientIndex,&msg,sizeof(SABeginRefineStar));
		return FALSE;
	}

	if (pEquipStar->money > pPlayer->GetPlayerAllMoney())
	{
		msg.result = SABeginRefineStar::BRS_NOMONEY;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginRefineStar));
		pPlayer->OnthSysNotice(2);
		return FALSE;
	}
	std::vector<SPackageItem*> vec;
	long nFlg = pPlayer->OnGetNeedMetrial(pPlayer,pEquipStar->StarNum,vec,pPlayer->m_MetrialItemID);
	if (nFlg != 1)
	{
		msg.result = SABeginRefineStar::BRS_LACK_METRIAL;//���ϲ���
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginRefineStar));
		pPlayer->OnthSysNotice(1);
		return FALSE;
	}
	if (!pPlayer->ReducedCommonMoney(pEquipStar->money))
	{
		return FALSE;
	}
	bool bHaveBindMetrial = false;
	pPlayer->OnProcessNeedMetrial(pPlayer,vec,pEquipStar->StarNum,"��װ��ǿ����ʾ��ԭ�������ģ�",bHaveBindMetrial);
	if (bHaveBindMetrial)
	{
		pEquip->flags |= SItemBase::F_SYSBINDED;
	}

	//////////////////////////////////////////////////////////////////////////
	bool bHaveLuckyCharmItem = false;
	// [��ȡ���˷����еĻ��۳�һ�����ɹ�������]	
	if (byStrengthenExternChoose & EStrengthenChoose_LuckyCharm)
	{
		std::vector<SPackageItem*> vecLuckyCharm;
		nFlg = pPlayer->OnGetNeedMetrial(pPlayer, pEquipStar->byLuckyCharmNum, vecLuckyCharm, pEquipStar->dwLuckyCharmID);	
		if (nFlg == 1)
		{
			pPlayer->OnProcessNeedMetrial(pPlayer, vecLuckyCharm, pEquipStar->byLuckyCharmNum, "�۳���һ�����˷���", bHaveBindMetrial);
			bHaveLuckyCharmItem = true;
		}
	}
	// [��ȡ������,����б������Ļ����۳�һ����������]
	bool bHaveProtectionCharacterItem = false;
	if (byStrengthenExternChoose & EStrengthenChoose_ProtectionCharacter)
	{
		std::vector<SPackageItem*> vecProtectionCharacter;
		nFlg = pPlayer->OnGetNeedMetrial(pPlayer, pEquipStar->byProtectionCharacterNum, vecProtectionCharacter, pEquipStar->dwProtectionCharacterID);
		bHaveProtectionCharacterItem = (nFlg == 1);
		if (nFlg == 1)
		{
			pPlayer->OnProcessNeedMetrial(pPlayer, vecProtectionCharacter, pEquipStar->byProtectionCharacterNum, "�۳���һ����������", bHaveBindMetrial);
			bHaveProtectionCharacterItem = true;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	// �������
	// [2012-8-17 12-41 gw: -] DWORD totalRate = pEquipStar->Rate;
	DWORD	totalRate = 0;
	DWORD rate = CRandom::RandRange(1, 10000);
	//rfalse("ǿ���ɹ��ʣ�%d�� ��ǰ��ȡ�ĸ���ֵ��%d", totalRate, rate);

	if (rate <= totalRate)
	{
		pPlayer->OnthSysNotice(12);
		msg.result = SABeginRefineStar::BRS_SUCCESS;
//		pEquip->attribute.starNum[index] = SEquipment::SLOT_IS_VALID;
		// ��װ��������ǳɹ��������
		pPlayer->AddRefineAttribute(pEquip);
		//equip->attribute.starNum[index] = SEquipment::SLOT_IS_VALID;
		pPlayer->SendUpdateEquipInfo(pEquip);
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginRefineStar));		
		// ����������ֵ��ȡ��װ��
		if (SEquipment::MAX_STAR_NUM - 1 == index)
		{
			pPlayer->RemoveRefineMetrial();
			pPlayer->OnthSysNotice(14);
		}
		else
		{
			// ������ʾ��Ϣ����
			// ��ʾ��һ�ȼ�
			pPlayer->SendNextLevelEquip(SAAfterUpgradeEquipMsg::UPDATE_STAR);
			pPlayer->SendNeedMETRIALInfo(pPlayer,SQAddEquip::ADE_REFINE_STAR);
		}

	}
	else
	{
		pPlayer->OnthSysNotice(13);
		msg.result = SABeginRefineStar::BRS_FAIL;

		if (bHaveProtectionCharacterItem)
		{
			TalkToDnid(pPlayer->m_ClientIndex, "������ʹ���˱���������Ȼǿ��û�гɹ��������ή�ǡ�");
		}
		// [2012-8-17 12-50 gw: ǿ��ʧ��֮�󣬲���Ҫ���ÿտף����ո����ж��Ƿ񽵼�]equip->attribute.starNum[index] = SEquipment::SLOT_IS_INVALID;
		else if (0)
		{
			int iBackPos = index - 1;
			if (iBackPos>= 0 && iBackPos < SEquipment::MAX_STAR_NUM)
			{
//				pEquip->attribute.starNum[iBackPos] = SEquipment::SLOT_IS_EMPTY;
				pPlayer->AddRefineAttribute(pEquip);
			}
			index = index - 1;
		}		
		pPlayer->SendUpdateEquipInfo(pEquip);
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginRefineStar));
		// [2012-8-21 19-50 gw: -ʧ��֮��ʹ9����Ҳ����Ҫ��ղ���]
		// 		if (SEquipment::MAX_STAR_NUM - 1 == index)
		// 			RemoveRefineMetrial();
		// 		else
		{
			// ������ʾ��Ϣ����

			// ��ʾ��һ�ȼ�
			pPlayer->SendNextLevelEquip(SAAfterUpgradeEquipMsg::UPDATE_STAR);
			pPlayer->SendNeedMETRIALInfo(pPlayer,SQAddEquip::ADE_REFINE_STAR);
		}
	}

	return TRUE;
}







// [2012-8-29 18-07 gw: +���װ�������Ƿ�����]
BOOL CEquipStrengthenServer::IsFullEquipJingLian(const struct SEquipment *equip)
{
	bool bIsFull = true;
	if (!equip)
	{
		return bIsFull;
	}

	int iLen = sizeof(equip->attribute.bornAttri) / sizeof(equip->attribute.bornAttri[0]);
	for (int i = 0; i < iLen; ++i)
	{
		if (equip->attribute.bornAttri[i].type == 0xff)
		{
			continue;
		}
//		WORD wTmpRefine = (equip->attribute.refineExtraAttri[i].type==0xff)?0:equip->attribute.refineExtraAttri[i].value;
		//if (equip->attribute.bornAttri[i].value + wTmpRefine < equip->attribute.MaxExtraAttri[i].value)
		{
			bIsFull = false;
			break;
		}
	}

	return bIsFull;	
}

 // [��ȡװ��������] 
BYTE CEquipStrengthenServer::GetEquipStarNum(const struct SEquipment *equip)
{
	BYTE	byStarNum = 0;
	if (!equip)
	{
		return byStarNum;
	}
//	int iLen = sizeof(equip->attribute.starNum) / sizeof(equip->attribute.starNum[0]);
	//for (int i = 0; i < iLen; ++i)
	{
//		if (equip->attribute.starNum[i] == SEquipment::SLOT_IS_VALID)
		{
			++byStarNum;
		}
	}
	return byStarNum;
}