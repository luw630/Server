#include "StdAfx.h"
#include "itemuser.h"
#include "Region.h"
#include "Area.h"
#include "Item.h"
#include "Player.h"
#include "monster.h"
#include "npc.h"
#include "ScriptManager.h"
#include "GlobalFunctions.h"
#include "Networkmodule\ScriptMsgs.h"
#include "Networkmodule\ItemMsgs.h"
#include "Networkmodule/RegionMsgs.h"
#include "Networkmodule/CardPointModifyMsgs.h"
#include "Networkmodule/refreshmsgs.h"
#include "networkmodule/TeamMsgs.h"
#include "liteserializer/lite.h"
#include "pub/rpcop.h"
#include "count.h"
#include "sportsarea.h"
#include "fightarena.h"
#include "roommanager.h"
#include "DSpcialItemData.h"
#include "PlayerManager.h"
#include "TeamManager.h"
#include "BuffEntityManager.h"
#include "CItemDefine.h"
#include "CItemService.h"
#include "Random.h"
#include "CDManager.h"
#include "NpcSaleMgr.h"
#include <time.h>
#include <vector>
#include <algorithm>
#include "CFightPetService.h"
#include "CFightPetDefine.h"
#include "StackWalker.h"
#include "./EquipStrengthen/EquipStrengthen.h"
#include "./ItemUseEvent.h"
#include "CSkillService.h"

extern void g_AddSpendMoney(__int64 iSpendMoney);

extern DWORD rcdMoneyRank;
extern BOOL  isLimitWG;
extern std::map<DWORD, Team> teamManagerMap;

// ���
const DWORD dwEbonyID		= 4010003;			// ��׵��� ����ʯID
const DWORD dwRedCopperID	= 4010004;			// ��׵��� ��ͭʯID
const DWORD dwSeaSparID		= 4010005;			// ��׵��� ����ʯID

const DWORD dwDiamondID		= 4010006;			// ժ������1 �����ID
const DWORD dwJadeID		= 4010007;			// ժ������2 ����ID

// ������Ǯ
const DWORD BaseMoney		= 1000;

// ժ����ʯʧ�ܸ���
const DWORD TakeOffRate		= 30;

// ���ִ�������ʧ�ܵĸ���
enum FailureRate
{
	FAILURERATE_25 = 25,    // 1-3���� 
	FAILURERATE_60 = 60,    // 4-5����
	FAILURERATE_90 = 90     // 6����
};

// ��������
enum
{
	HUISHENDAN_ID = 8040001,		// ����
	LIANZHISHI_ID = 8040022,		// ����ʯID
	XUANMENGTIANJING_ID = 8040023,	// �����쾫
};

// ============װ�����׵����ó���==============
// ����װ��
const DWORD dwJinGangID		= 4000001;	// ���ʯ 0 - 2 
const DWORD dwHaoYueID		= 4000002;	// ���ʯ 3 - 5 
const DWORD dwYunTieID		= 4000003;	// ����ʯ 6 - 8
const DWORD dwJinJingID		= 4000004;	// ��ʯ 9

// ��������
const DWORD dwYuSuiID		= 4010001;	// ����

// ���ʵ���
const DWORD dwMiJiID		= 4010002;	// ŷұ���ؼ�

// ====== װ����Ʒ�����õ��� =========
const DWORD dwTianQingID	= 4003001;	//����ʯ

const DWORD dwJianJingID	= 4001001;	// �⾧ʯ
const DWORD dwHeiXuanID		= 4002001;	// ����ʯ

// =================װ����������������===========
const DWORD dwXingChenID	= 4020001;	// ��������
const DWORD dwXilianID		= 4020002;	// װ������
// ==============================================
#define GetAbsLen(a, b) (abs(((long)a) - ((long)b)))

CItemUser::CItemUser(void)
{
	m_bWaitBuyResult	= false;

	ArrangeIntervalTime	= timeGetTime();
	m_CommNpcGlobalID	= 0;
	m_CurShopID			= 0;
	
	memset(m_PackageItems,		0, sizeof(m_PackageItems));	
	memset(m_EquipedColumnFlag, 0, sizeof(m_EquipedColumnFlag));
	memset(&m_EmptyItem4Debug,	0, sizeof(m_EmptyItem4Debug));
}

CItemUser::~CItemUser(void) {}

void CItemUser::StartItemCDTimer(const SItemBaseData *pItemData)
{
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player || !pItemData)
		return;

	// ������������ȴʱ��
	DWORD groupTime = 0;
	if (pItemData->m_LocSrvCDGroupID)
	{
		groupTime = CItemService::GetInstance().GetItemGroupCDTime(pItemData->m_LocSrvCDGroupID);
		if (!groupTime)
		{
			rfalse(4, 1, "ItemUser.cpp StartItemCDTimer()");
			return;
		}

		MY_ASSERT(groupTime);
	}

	if (groupTime && m_CDMgr.Active(pItemData->m_LocSrvCDGroupID, groupTime))
	{
		SNotifyStartColdTimer_S2C_MsgBody msg;
		msg.CDType		= pItemData->m_LocSrvCDGroupID;
		msg.coldeTime	= groupTime;
		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(msg));
	}

	// �������ߵ�����ȴ
	if (pItemData->m_LocSrvCoolingTime && m_CDMgr.Active(30000 + pItemData->m_ID, pItemData->m_LocSrvCoolingTime))
	{
		SNotifyStartColdTimer_S2C_MsgBody msg;
		msg.CDType		= 30000 + pItemData->m_ID;
		msg.coldeTime	= pItemData->m_LocSrvCoolingTime;
		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(msg));
	}

	return;
}

int CItemUser::CheckItemCDTime(const SItemBaseData *pItemData)
{
	if (!pItemData)
	{
		rfalse(4, 1, "[ItemUser.cpp] - CheckItemCDTime(const SItemBaseData *pItemData) - pItemData");
		return 0;
	}
	MY_ASSERT(pItemData);

	if (pItemData->m_LocSrvCDGroupID && m_CDMgr.Check(pItemData->m_LocSrvCDGroupID))
		return 1;
	else if (pItemData->m_LocSrvCoolingTime && m_CDMgr.Check(30000 + pItemData->m_ID))
		return 2;
	else
		return 0;
}

BOOL CItemUser::AutoUseItem(WORD wItemID, DWORD useType)
{
	return FALSE;
}

BOOL  CItemUser::AutoUseItem(SPackageItem *pItem, DWORD useType)
{
	return FALSE;
}

void CItemUser::SendItemSynMsg(SPackageItem *pItem, WORD LookType)
{
	if (!pItem)
		return;

	SPackageItem * pl = FindItemByPos(pItem->wCellPos, LookType);
	if (!pl)
	{
		return;
	}

	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;

	SAAddPackageItemMsg msg;
	memcpy(&msg.stItem, pItem, sizeof(SPackageItem));

	g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAAddPackageItemMsg));
	
	UpdateItemRelatedTaskFlag(pPlayer, msg.stItem.wIndex);

	return;
}

void CItemUser::UpdateMapTaskFlag()
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;

	if (!m_ParentRegion || !m_ParentArea)
	{
		StackWalker wp;
		wp.ShowCallstack();
		rfalse(4,1," itemuser.cpp - UpdateMapTaskFlag() - !m_ParentRegion || !m_ParentArea");
		return;
	}

	if (0 == pPlayer->m_CurHp)
		return;

	// ����ܵ�ͼ������Ϣ
	CPlayer::TASKMAP::iterator it = pPlayer->m_MapTask.find(m_ParentRegion->m_wRegionID);
	if (it != pPlayer->m_MapTask.end())
	{
		for (std::list<DWORD>::iterator ti = (it->second).begin(); ti != (it->second).end(); ++ti)
		{
			DWORD flagIndex = *ti;

			if (CRoleTask* task = pPlayer->m_TaskManager.GetRoleTask((flagIndex & 0xffff0000) >> 16))
			{
				if (!task)
				{
					rfalse(4, 1, "Itemuser.cpp - UpdateMapTaskFlag() - !task");
					return;
				}

				if (STaskFlag *flag = task->GetFlag(flagIndex))
				{
					if (!flag)
					{
						rfalse(4, 1, "Itemuser.cpp - UpdateMapTaskFlag() - !flag");
						return;
					}

					if (flag->m_Type != TT_REGION)
					{
						rfalse(4, 1, "Itemuser.cpp - UpdateMapTaskFlag() - flag->m_Type != TT_REGION");
						return;
					}

					MY_ASSERT(TT_REGION == flag->m_Type);

					if (1 == flag->m_Complete)
						continue;

					// ��ȡ����ֵ
					WORD x = flag->m_TaskDetail.Region.DestX;
					WORD y = flag->m_TaskDetail.Region.DestY;

					if (x >= m_curTileX-5 && x <= m_curTileX+5 && y >= m_curTileY-5 && y <= m_curTileY+5)
					{
						flag->m_Complete = 1;

						// ������������Ϣ
						pPlayer->SendUpdateTaskFlagMsg(*flag, true);
					}
				}
				else
				{
					rfalse("����ܵ�ͼ������Ϣ����Ȼ�Ҳ���������꣡");
				}
			}
			else
			{
				rfalse("����ܵ�ͼ������Ϣ����Ȼ�Ҳ�������");
			}
		}
	}
}

void CItemUser::UpdateUseItemTaskFlag(CPlayer *pPlayer, DWORD index)
{
	if (!pPlayer || 0 == index)
		return;

	// �����Ʒʹ��������Ϣ
	CPlayer::TASKUSEITEM::iterator it = pPlayer->m_UseItemTask.find(index);
	if (it != pPlayer->m_UseItemTask.end())
	{
		for (std::list<DWORD>::iterator ti = (it->second).begin(); ti != (it->second).end(); ++ti)
		{
			DWORD flagIndex = *ti;

			if (CRoleTask *task = pPlayer->m_TaskManager.GetRoleTask((flagIndex & 0xffff0000) >> 16))
			{
				if (!task)
				{
					rfalse(4, 1, "Itemuser.cpp - UpdateUseItemTaskFlag() - !task");
					return;
				}

				if (STaskFlag *flag = task->GetFlag(flagIndex))
				{
					if (!flag)
					{
						rfalse(4, 1, "Itemuser.cpp - UpdateUseItemTaskFlag() - !flag");
						return;
					}
					if (TT_USEITEM != flag->m_Type)
					{
						rfalse(4, 1, "Itemuser.cpp - UpdateUseItemTaskFlag() - TT_USEITEM != flag->m_Type");
						return;
					}
					MY_ASSERT(TT_USEITEM == flag->m_Type);
					
					if (1 == flag->m_Complete)
						continue;

					BYTE oldComp = flag->m_Complete;

					if (++flag->m_TaskDetail.UseItem.CurUseTimes == flag->m_TaskDetail.UseItem.UseTimes)
					{
						flag->m_Complete = 1;
					}

					// ������������Ϣ
					pPlayer->SendUpdateTaskFlagMsg(*flag, oldComp != flag->m_Complete);
				}
				else
				{
					rfalse("���ʹ�õ���������Ϣ����Ȼ�Ҳ���������꣡");
				}
			}
			else
			{
				rfalse("���ʹ�õ���������Ϣ����Ȼ�Ҳ�������");
			}
		}
	}
}

void CItemUser::UpdateItemRelatedTaskFlag(CPlayer *pPlayer, DWORD index)
{
	if (!pPlayer)
	{
		rfalse(4, 1, "[ItemUser.cpp] - UpdateItemRelatedTaskFlag(CPlayer *pPlayer, DWORD index) - !pPlayer");
		return;
	}
	UpdateItemTaskFlag(pPlayer, index);
	UpdateGiveItemTaskFlag(pPlayer, index);
	UpdateGetItemTaskFlag(pPlayer, index);
}

void CItemUser::UpdateGetItemTaskFlag(CPlayer *pPlayer, DWORD index)
{
	if (!pPlayer || 0 == index)
		return;

	// ����ȡ����������Ϣ
	CPlayer::TASKGETITEM::iterator it = pPlayer->m_GetItemTask.find(index);
	if (it != pPlayer->m_GetItemTask.end())
	{
		for (std::list<DWORD>::iterator ti = (it->second).begin(); ti != (it->second).end(); ++ti)
		{
			DWORD flagIndex = *ti;

			if (CRoleTask *task = pPlayer->m_TaskManager.GetRoleTask((flagIndex & 0xffff0000) >> 16))
			{
				if (!task)
				{
					rfalse(4, 1, "Itemuser.cpp - UpdateGetItemTaskFlag() - !task");
					return;
				}
				if (STaskFlag *flag = task->GetFlag(flagIndex))
				{
					if (!flag)
					{
						rfalse(4, 1, "Itemuser.cpp - UpdateGetItemTaskFlag() - !flag");
						return;
					}

					if (flag->m_Type != TT_GET)
					{
						rfalse(4, 1, "Itemusr.cpp - UpadateGetItemTaskFlag() - flag->m_Type != TT_GET");
						return;
					}
					MY_ASSERT(TT_GET == flag->m_Type);

					DWORD itemCount = pPlayer->GetItemNum(flag->m_TaskDetail.GiveGet.ItemID, XYD_FT_WHATEVER);

					if (1 == flag->m_Complete && (itemCount >= flag->m_TaskDetail.GiveGet.ItemNum))
						continue;

					STaskFlag temp = *flag;
					
					BYTE oldComp = flag->m_Complete;

					if (itemCount >= flag->m_TaskDetail.GiveGet.ItemNum)
					{
						flag->m_TaskDetail.GiveGet.CurItemNum = flag->m_TaskDetail.GiveGet.ItemNum;
						flag->m_Complete  = 1;
					}
					else
					{
						flag->m_TaskDetail.GiveGet.CurItemNum = itemCount;
						flag->m_Complete = 0;
					}

					if (0 != memcmp(&temp, flag, sizeof(STaskFlag)))
					{
						// ������������Ϣ
						pPlayer->SendUpdateTaskFlagMsg(*flag, oldComp != flag->m_Complete);
					}
				}
				else
				{
					rfalse("���ȡ��Ʒ������Ϣ����Ȼ�Ҳ���������꣡");
				}
			}
			else
			{
				rfalse("���ȡ��Ʒ������Ϣ����Ȼ�Ҳ�������");
			}
		}
	}
}

void CItemUser::UpdateGiveItemTaskFlag(CPlayer *pPlayer, DWORD index)
{
	if (0 == pPlayer || 0 == index)
		return;

	// ������Ʒ������Ϣ
	CPlayer::TASKGIVEITEM::iterator it = pPlayer->m_GiveItemTask.find(index);
	if (it != pPlayer->m_GiveItemTask.end())
	{
		for (std::list<DWORD>::iterator ti = (it->second).begin(); ti != (it->second).end(); ++ti)
		{
			DWORD flagIndex = *ti;

			if (CRoleTask *task = pPlayer->m_TaskManager.GetRoleTask((flagIndex & 0xffff0000) >> 16))
			{
				if (!task)
				{
					rfalse(4, 1, "Itemuser.cpp - UpdateGiveItemTaskFlag() - !task");
					return;
				}

				if (STaskFlag *flag = task->GetFlag(flagIndex))
				{
					if (!flag)
					{
						rfalse(4, 1, "Itemuser.cpp - UpdateGiveItemTaskFlag() - !flag");
						return;
					}

					if (flag->m_Type != TT_GIVE)
					{
						rfalse(4, 1, "Itemuser.cpp - UpdateGiveItemTaskFlag() - flag->m_Type != TT_GIVE");
						return;
					}
					MY_ASSERT(TT_GIVE == flag->m_Type);

					DWORD itemCount = pPlayer->GetItemNum(flag->m_TaskDetail.GiveGet.ItemID, XYD_FT_WHATEVER);

					if (1 == flag->m_Complete && (itemCount >= flag->m_TaskDetail.GiveGet.ItemNum))
						continue;

					STaskFlag temp = *flag;
				
					BYTE oldComp = flag->m_Complete;

					if (itemCount >= flag->m_TaskDetail.GiveGet.ItemNum)
					{
						flag->m_TaskDetail.GiveGet.CurItemNum = flag->m_TaskDetail.GiveGet.ItemNum;
						flag->m_Complete  = 1;
					}
					else
					{
						flag->m_TaskDetail.GiveGet.CurItemNum = itemCount & itemCount;
						flag->m_Complete = 0;
					}

					if (0 != memcmp(&temp, flag, sizeof(STaskFlag)))
					{
						// ������������Ϣ
						pPlayer->SendUpdateTaskFlagMsg(*flag, oldComp != flag->m_Complete);
					}
				}
				else
				{
					rfalse("������Ʒ������Ϣ����Ȼ�Ҳ���������꣡");
				}
			}
			else
			{
				rfalse("������Ʒ������Ϣ����Ȼ�Ҳ�������");
			}
		}
	}
}

void CItemUser::UpdateItemTaskFlag(CPlayer *pPlayer, DWORD index)
{
	if (!pPlayer || 0 == index)
		return;

	// �����Ʒ������Ϣ
	CPlayer::TASKITEM::iterator it = pPlayer->m_ItemTask.find(index);
	if (it != pPlayer->m_ItemTask.end())
	{
		for (std::list<DWORD>::iterator ti = (it->second).begin(); ti != (it->second).end(); ++ti)
		{
			DWORD flagIndex = *ti;

			if (CRoleTask *task = pPlayer->m_TaskManager.GetRoleTask((flagIndex & 0xffff0000) >> 16))
			{
				if (!task)
				{
					rfalse(4, 1, "Itemuser.cpp - UpdateItemTaskFlag() - !task");
					return;
				}

				if (STaskFlag *flag = task->GetFlag(flagIndex))
				{
					if (!flag)
					{
						rfalse(4, 1, "Itemuser.cpp - UpdateItemTaskFlag() - !flag");
						return;
					}

					if (TT_ITEM != flag->m_Type)
					{
						rfalse(4, 1, "Itemuser.cpp - UpdateItemTaskFlag() - TT_ITEM != flag->m_Type");
						return;
					}
					MY_ASSERT(TT_ITEM == flag->m_Type);

					DWORD itemCount = pPlayer->GetItemNum(flag->m_TaskDetail.Item.ItemID, XYD_FT_WHATEVER);

					// ���������ɣ������µĵ���������Ӱ������ֱ������
					if (1 == flag->m_Complete && itemCount >= flag->m_TaskDetail.Item.ItemNum)
						continue;

					STaskFlag temp = *flag;

					BYTE oldComp = flag->m_Complete;

					if (itemCount >= flag->m_TaskDetail.Item.ItemNum)
					{
						flag->m_TaskDetail.Item.CurItemNum = flag->m_TaskDetail.Item.ItemNum;
						flag->m_Complete = 1;
					}
					else
					{
						flag->m_TaskDetail.Item.CurItemNum = itemCount;
						flag->m_Complete = 0;
					}

					if (0 != memcmp(&temp, flag, sizeof(STaskFlag)))
					{
						// ������������Ϣ
						pPlayer->SendUpdateTaskFlagMsg(*flag, oldComp != flag->m_Complete);
					}
				}
				else
				{
					rfalse("���������Ʒ��Ϣ����Ȼ�Ҳ���������꣡");
				}
			}
			else
			{
				rfalse("���������Ʒ��Ϣ����Ȼ�Ҳ�������");
			}
		}
	}
}

BOOL CItemUser::DelItem(const SPackageItem &item, LPCSTR info, bool autoUnLock)
{
	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	WORD linePos = item.wCellPos;

	SPackageItem *temp = FindItemByPos(linePos, autoUnLock ? XYD_FT_ONLYLOCK : XYD_FT_ONLYUNLOCK);

	if (!temp)
	{
		rfalse(4, 1, "[ItemUser.cpp] - DelItem(const SPackageItem &item, LPCSTR info, bool autoUnLock) - temp");
		return FALSE;
	}

	if (&item != temp || memcmp(temp, &item, sizeof(SPackageItem)))
	{
		rfalse(4, 1, "itemuser.cpp - DelItem() - &item != temp || memcmp(temp, &item, sizeof(SPackageItem))");
		return FALSE;
	}

	MY_ASSERT(temp && (&item == temp) && (0 == memcmp(temp, &item, sizeof(SPackageItem))));
	if(!ValidationPos(linePos)){return false;}

	const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(item.wIndex);
	if (!itemData)
		return FALSE;

	if (autoUnLock)		// ��Ҫ���������Ƚ���
		LockItemCell(item.wCellPos, false, true); // [2012-9-10 12-17 gw: *��������ͬ�����ͻ��˽��� false->true]

	if (!FillItemPos(linePos, SItemWithFlag::IWF_ITEMIN, false))
	{
		return FALSE;
		MY_ASSERT(0);
	}
		
	// ɾ������
	if (m_PackageItems[linePos].m_Item.overlap < itemData->m_Overlay)
	{
		m_ItemsState[item.wIndex] -= (itemData->m_Overlay - item.overlap);
		//rfalse(2, 1, "ɾ����Ϊ%d�ĵ���Ŀǰ�����ٵ���%d��", item.wIndex, m_ItemsState[item.wIndex]);
	}

	SADelPackageItemMsg msg;
	msg.stItem = item;
	g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));

	memset(&m_PackageItems[linePos].m_Item, 0, sizeof(SPackageItem));
	
	UpdateItemRelatedTaskFlag(pPlayer, msg.stItem.wIndex);

	ValidationPos(linePos);

	return TRUE;
}

DWORD CItemUser::GetItemNum(DWORD wIndex, WORD GetType)
{
	DWORD nNum = 0;

	for (DWORD i = 0; i < PackageAllCells; i++)
	{
		SPackageItem *pItem = FindItemByPos(i, GetType);
		if (pItem && pItem->wIndex == wIndex)
			nNum += pItem->overlap;
	}

	return nNum;
}

extern string s_FactionName;
extern DWORD s_dwFactionTitle;

BOOL CItemUser::_UseItem(SPackageItem *pItem, DWORD useType)
{
	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	if (!pItem)
	{
		rfalse(2,1,"_UseItem: ��������ָ��Ϊ��!!!");
		return FALSE;
	}

	if ( pItem->wIndex==0 || pItem->overlap == 0)
	{
		rfalse(4,1,FormatString("����һ������ĵ���ID: %d,��������%d �˺�%s",pItem->wIndex,pItem->overlap,pPlayer->GetAccount()));
		return FALSE;
	}

	MY_ASSERT(pItem && pItem->wIndex && pItem->overlap);

	const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(pItem->wIndex);
	if (!pData)
		return FALSE;

	if (!CanUseIt(pPlayer, pItem, pData))
		return FALSE;

	if (pData->m_LocSrvPreTime){
		LockItemCell(pItem->wCellPos,false);
	}

	// �����ű���չ
	lite::Variant ret;
	g_Script.SetCondition(NULL, pPlayer, NULL);
	g_Script.SetInUseItemPackage(pItem);
	LuaFunctor(g_Script, FormatString("OnUseItem"))[pItem->wIndex](&ret);
	g_Script.CleanCondition();

	if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)
		return FALSE;

	if (0 == ( __int32 )ret)
		return FALSE;

	if (pItem->overlap > 1)
		ChangeOverlap(pItem, 1, false);
	else
		DelItem(*pItem, "����ʹ��ɾ����");

	// ������ȴ����͵�����
	StartItemCDTimer(pData);
    
    return TRUE;
}

void CItemUser::ChangeOverlap(SPackageItem *pItem, WORD delta, bool Add, bool Sync)
{
	if (!pItem || 0 == pItem->overlap || 0 == delta)
		return;

	const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(pItem->wIndex);
	if (!pData)
		return;

	if (Add)
	{
		//MY_ASSERT(pItem->overlap <= pData->m_Overlay);
		//MY_ASSERT((pData->m_Overlay-pItem->overlap) >= delta);
		pItem->overlap += delta;
		m_ItemsState[pItem->wIndex] -= delta;
		//MY_ASSERT(pItem->overlap <= pData->m_Overlay);
	}
	else
	{
		//MY_ASSERT(pItem->overlap > delta);
		pItem->overlap -= delta;
		m_ItemsState[pItem->wIndex] += delta;
		//MY_ASSERT(pItem->overlap);
	}

	//rfalse(2, 1, "Ϊ%d�ĵ���Ŀǰ�����ٵ���%d��", pItem->wIndex, m_ItemsState[pItem->wIndex]);

	if (Sync)
		SendItemSynMsg(pItem, XYD_FT_ONLYUNLOCK);
}

BOOL CItemUser::_EquipItem(SPackageItem *pItem)
{
	if (!pItem || 0 == pItem->wIndex || 0 == pItem->overlap)
		return FALSE;

	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	const SItemBaseData *itemWillEquip = CItemService::GetInstance().GetItemBaseData(pItem->wIndex);
	if (!itemWillEquip)
		return FALSE;

	if (ITEM_IS_TASK(itemWillEquip->m_Type))
	{
		return FALSE;
	}
	//MY_ASSERT(ITEM_IS_EQUIPMENT(itemWillEquip->m_Type) && 1 == itemWillEquip->m_Overlay && 1 == pItem->overlap);

	if (!CanEquipIt(player, pItem, itemWillEquip))
		return FALSE;

	EQUIP_POSITION i = EQUIP_P_WEAPON;
	switch (itemWillEquip->m_Type)
	{
	case EQUIP_T_WEAPON:
		i = EQUIP_P_WEAPON;
		break;
	case EQUIP_T_HAT:
		i = EQUIP_P_HAT;
		break;
	case EQUIP_T_WRIST:
		i = EQUIP_P_WRIST;
		break;
	case EQUIP_T_PAT:
		i = EQUIP_P_PAT;
		break;
	case EQUIP_T_WAIST:
		i = EQUIP_P_WAIST;
		break;
	case EQUIP_T_SHOSE:
		i = EQUIP_P_SHOSE;
		break;
	case EQUIP_T_CLOTH:
		i = EQUIP_P_CLOTH;
		break;
	case EQUIP_T_MANTLE:
		i = EQUIP_P_MANTLE;
		break;
	case EQUIP_T_PENDANTS:
		if (player->m_Property.m_Equip[EQUIP_P_PENDANTS1].wIndex == 0)
			i = EQUIP_P_PENDANTS1;
		else if (player->m_Property.m_Equip[EQUIP_P_PENDANTS2].wIndex == 0)
			i = EQUIP_P_PENDANTS2;
		else
			i = EQUIP_P_PENDANTS1;
		break;
	case EQUIP_T_PROTECTION:
		i = EQUIP_P_PROTECTION;
		break;
	case EQUIP_T_RING:
		if (player->m_Property.m_Equip[EQUIP_P_RING1].wIndex == 0)
			i = EQUIP_P_RING1;
		else if (player->m_Property.m_Equip[EQUIP_P_RING2].wIndex == 0)
			i = EQUIP_P_RING2;
		else
			i = EQUIP_P_RING1;
		break;
	case EQUIP_T_PRECIOUS:
		i = EQUIP_P_PRECIOUS;
		break;
	case EQUIP_T_SHIZHUANG_HAT:
		i = EQUIP_P_SHIZHUANG_HAT1;
		break;
	case EQUIP_T_SHIZHUANG_CLOTH:
		i = EQUIP_P_SHIZHUANG_CLOTH1;
		break;
	default:
		return FALSE;
	}
	
	if (IsLockedEquipColumnCell(i))
	{
		rfalse(4, 1, "EquipColumCell was Locked. pos = %d", i);
		return FALSE;
	}

	// ���浱ǰλ���ϵ�װ������
	SEquipment tempEquip = player->m_Property.m_Equip[i];

	SItemBase *item = pItem;
	
	if (!item)
	{
		rfalse(4, 1, "[ItemUser] - _EquipItem()");
		return FALSE;
	}

	// ���°�״̬
	item->flags |= GetSItemBaseBindState(EITEM_BIND_EQUIP, itemWillEquip->m_BindType);

	player->m_Property.m_Equip[i] = *(SEquipment *)item;

	// ��װ����Ϣ���͸��ͻ���
	SAEquipInfoMsg msg;
	int n = sizeof(msg.stEquip);
	msg.stEquip = player->m_Property.m_Equip[i];
	msg.byPos   = i;
	g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SAEquipInfoMsg));

	if ((IsPosNeedToSwitchMesh(i)) && m_ParentArea)
	{
		SAEquipItemMsg msg;
		msg.dwGlobalID = GetGID();
		msg.byEquipPos = i;
		msg.wItemIndex = item->wIndex;

		// ����Ϣ���͸���Ч������������
		CArea *area = (CArea *)m_ParentArea->DynamicCast(IID_AREA);
		if (area)
			area->SendAdj(&msg, sizeof(msg), -1);
	}
	
	// ���¼���װ���ӳ�����
	player->InitEquipmentData();
	UpdateAllProperties();
	
	// ��ǰװ��֮ǰ����Ч����ʱ�����ڵ����ƶ���ɾ��װ�����е�
	if (tempEquip.wIndex == 0)
		return DelItem(*pItem);

	// ����ǵ��߽���������Ҫ����������ߵ�����
	memset((SItemBase*)pItem, 0, sizeof(SRawItemBuffer));
	memcpy((SItemBase*)pItem, (SItemBase*)&tempEquip, sizeof(SRawItemBuffer));
	SendItemSynMsg(pItem);

	// ����״̬����ʵ����װ�������ø��£���Ϊװ���ǲ��ɵ��ӵģ���

	ValidationPos(pItem->wCellPos);

	//������Ϊֵ 
	player->UpdatePlayerXwzMsg();

	return TRUE;
}

BOOL CItemUser::_UseBookUpSkill(SPackageItem *pItem)		//ʹ�ü����鼤���
{
	if (!pItem || 0 == pItem->wIndex || 0 == pItem->overlap)
		return FALSE;

	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;
	const WORD SkillID = CSkillService::GetInstance().GetSkillIdByBookId(pItem->wIndex);
	const SSkillBaseData *pSSkillBaseData = CSkillService::GetInstance().GetSkillBaseData(SkillID);
	if (pSSkillBaseData == NULL)
		return FALSE;
	SSkill AddSkill;
	AddSkill.wTypeID = SkillID;//�����ü��ܱ��м���ID������Ӧ�ļ������ID��ͬ
	AddSkill.byLevel = 1;//�ɹ�����ܺ󣬵ȼ�Ϊ1
	BOOL bRet = player->ActivaSkill(AddSkill.wTypeID, "ѧϰ���ܣ�");
	if (!bRet)
		return FALSE;
	player->SendData(SARefreshPlayerMsg::ONLY_BACKUP);
	SAUseItemMsg AnsMsg;
	memcpy(&AnsMsg.stSkill, &AddSkill, sizeof(SSkill));
	g_StoreMessage(player->m_ClientIndex, &AnsMsg, sizeof(SAUseItemMsg));
	return TRUE;
}

BOOL CItemUser::DelEquipment(BYTE equipId)
{
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
    if (!player)
        return FALSE;

	if ((equipId >= EQUIP_P_MAX ) || (player->m_Property.m_Equip[equipId].wIndex == 0))
		return FALSE;

	ZeroMemory(&player->m_Property.m_Equip[equipId], sizeof(SEquipment));

    // ������׼�������Ĵ�ϲ���

	// ��װ������Ϣ���͸��ͻ���
	SAEquipInfoMsg msg;
	msg.stEquip = player->m_Property.m_Equip[equipId];
	msg.byPos   = equipId;
	g_StoreMessage(player->m_ClientIndex, &msg, sizeof(msg));

	if (IsPosNeedToSwitchMesh(equipId))
	{
		SAEquipItemMsg EquipItemMsg;

		EquipItemMsg.dwGlobalID = GetGID();
		EquipItemMsg.byEquipPos = equipId;
		EquipItemMsg.wItemIndex = 0;

		// ����Ϣ������Ч������������
		if (CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA))
			pArea->SendAdj(&EquipItemMsg, sizeof(SAEquipItemMsg), -1);
	}

// 	WORD playeratk[4];
// 	for (int i = 0 ;i<4;i++)
// 	{
// 		playeratk[i] =*reinterpret_cast<WORD *>(m_AttriRefence[i]);
// 	}
    // ���¼���װ���ӳ�����
    player->InitEquipmentData();
	UpdateAllProperties();
// 	for (int i = 0 ;i<4;i++)
// 	{
// 		WORD attrival =*reinterpret_cast<WORD *>(m_AttriRefence[i]);
// 		if (playeratk[i] != attrival)
// 		{
// 			SAequitattrichangeMsg attrichangeMsg; 
// 			int nattri = attrival - playeratk[i] ;
// 			attrichangeMsg.index = i;
// 			attrichangeMsg.index = nattri > 0 ? attrichangeMsg.index|0x80: attrichangeMsg.index;
// 			attrichangeMsg.changevalue = abs(nattri);
// 			g_StoreMessage(player->m_ClientIndex,&attrichangeMsg,sizeof(SAequitattrichangeMsg));
// 			if(i == 0)
// 				nattri > 0 ?  TalkToDnid(player->m_ClientIndex,FormatString("������������%d",nattri)):TalkToDnid(player->m_ClientIndex,FormatString("������������%d",abs(nattri)));
// 		}
// 	}

	return TRUE;
}

// PickOneItemֻ���ڲ����ߺ��������Ժܶ���֤���ⲿ���ˣ��ڴ˾Ͳ�����
BOOL CItemUser::PickOneItem(SRawItemBuffer &goundItem)
{
	const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(goundItem.wIndex);
	
	if (!pData)
	{
		rfalse(4, 1, "[ItemUser] - PickOneItem()");
		return FALSE;
	}

	MY_ASSERT(pData);

	WORD willCreate = TryPossibleMerge(goundItem.wIndex, goundItem.overlap, pData->m_Overlay);
	InsertItem(goundItem.wIndex, willCreate, pData, goundItem, false);

	return TRUE;
}

BOOL CItemUser::StartPickSingleOne(CPlayer *pPlayer, CItem *pItem, WORD index)
{
	if (!pPlayer || !pItem)
	{
		rfalse(4, 1, "ItemUser.cpp - StartPickSingleOne() - !pPlayer || !pItem");
		return FALSE;
	}

	if (index > g_PackageMaxItem)
		return FALSE;

	if (g_PackageMoneyIndex == index)	// ʰȡ����
	{
		if (0 == pItem->m_Money)
			return FALSE;

		pPlayer->AddPlayerMoney(pItem->m_MoneyType, pItem->m_Money);
		pItem->m_Money = 0;
	}
	else								// ʰȡĳ��λ���ϵĵ���
	{
		if (index >= pItem->m_ItemsInPack)
			return FALSE;

		if (0 == pItem->m_GroundItem[index].wIndex)
		{
			rfalse(4, 1, "Itemuser.cpp - StartPickSingleOne() - 0 == pItem->m_GroundItem[index].wIndex");
			return FALSE;
		}
		MY_ASSERT(pItem->m_GroundItem[index].wIndex);
		
		std::list<SAddItemInfo> itemList;
		itemList.push_back(SAddItemInfo(pItem->m_GroundItem[index].wIndex, pItem->m_GroundItem[index].overlap));

		if (!CanAddItems(itemList))
		{
			TalkToDnid(pPlayer->m_ClientIndex, "�����������޷�ʰȡ��");
			return FALSE;
		}
			
		PickOneItem(pItem->m_GroundItem[index]);

		--pItem->m_ItemsInPack;

		// ��������
		if (0xff == pItem->m_ExPosition[index] || pItem->m_ExPosition[index] >= g_PackageMaxItem)
		{
			rfalse(4, 1, "Itemuser.cpp - StartPickSingleOne() - 0xff == pItem->m_ExPosition[index] || pItem->m_ExPosition[index] >= g_PackageMaxItem");
			return FALSE;
		}
		MY_ASSERT(0xff != pItem->m_ExPosition[index] && pItem->m_ExPosition[index] < g_PackageMaxItem);

		pItem->m_IndexMask[pItem->m_ExPosition[index]] = -1;
		for (size_t j = pItem->m_ExPosition[index] + 1; j < g_PackageMaxItem; j++)
			pItem->m_IndexMask[j]--;

		if (index != pItem->m_ItemsInPack)
		{
			memmove(&pItem->m_GroundItem[index], &pItem->m_GroundItem[index+1], sizeof(SRawItemBuffer) * (pItem->m_ItemsInPack - index));
			memmove(&pItem->m_PickStatus[index], &pItem->m_PickStatus[index+1], sizeof(BYTE) * (pItem->m_ItemsInPack - index));
			memmove(&pItem->m_ExPosition[index], &pItem->m_ExPosition[index+1], sizeof(BYTE) * (pItem->m_ItemsInPack - index));
		}

		memset(&pItem->m_GroundItem[pItem->m_ItemsInPack], 0, sizeof(SRawItemBuffer));
		pItem->m_PickStatus[pItem->m_ItemsInPack] = PIPT_LEADER + 1;
		pItem->m_ExPosition[pItem->m_ItemsInPack] = 0xff;
	}

	if (--pItem->m_Remain)
		pItem->UpdateItemPicked(index);
	else
		pItem->RemoveMe();

	return TRUE;
}

BOOL CItemUser::RecvPickupItem(SQPickupItemMsg *msg)
{
	if (!msg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvPickupItem(SQPickupItemMsg *msg) - !msg");
		return FALSE;
	}

	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	if (!pPlayer->m_pCheckItem || pPlayer->m_pCheckItem->GetGID() != msg->dwGlobalID)
		return FALSE;

	// �����߱���
	if (!pPlayer->m_pCheckItem->m_dwTeamID && pPlayer->m_pCheckItem->m_dwGID!=GetGID())
	{
		//TalkToDnid(pPlayer->m_ClientIndex,"��������ģ��ټ��ִ�ϣ�",0);
		return FALSE;
	}

	// �����߱���
	if (pPlayer->m_pCheckItem->m_dwTeamID)
	{
		if (pPlayer->m_pCheckItem->m_dwTeamID != pPlayer->m_dwTeamID)		// ûȨ��
			return FALSE;

		// Ǯ�Ѿ������ˣ����Դ˴�������ȣ�
		if (pPlayer->m_pCheckItem->m_Remain != pPlayer->m_pCheckItem->m_ItemsInPack)
		{
			rfalse(4, 1, "[Itemuser] - RecvPickupItem() - pPlayer->m_pCheckItem->m_Remain != pPlayer->m_pCheckItem->m_ItemsInPack");
			return FALSE;
		}

		MY_ASSERT(pPlayer->m_pCheckItem->m_Remain == pPlayer->m_pCheckItem->m_ItemsInPack);

		if (0xffff != msg->wIndex && msg->wIndex >= pPlayer->m_pCheckItem->m_ItemsInPack)
			return FALSE;

		std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pPlayer->m_pCheckItem->m_dwTeamID);
		if (iter == teamManagerMap.end())
			return FALSE;

		int TrackIndex = (0xffff == msg->wIndex) ? 0 : msg->wIndex;
		int TotalItems = (0xffff == msg->wIndex) ? pPlayer->m_pCheckItem->m_ItemsInPack : 1;

		extern BOOL AddItemToTeamPack(CPlayer *pPlayer, CItem *pItem, WORD index);

		for (int i = 0; i < TotalItems; i++)
		{
			switch (pPlayer->m_pCheckItem->m_PickStatus[TrackIndex])
			{
			case PIPT_FREE:
				if (!StartPickSingleOne(pPlayer, pPlayer->m_pCheckItem, TrackIndex))
					TrackIndex++;
				break;

			case PIPT_LEADER:
				if (!AddItemToTeamPack(pPlayer, pPlayer->m_pCheckItem, TrackIndex))
					TrackIndex++;
				break;

			default:
				TrackIndex++;
			}
		}

		return TRUE;
	}

	// ���߰�������ȫ��ʰȡ/����ʰȡ��
	if (0xffff != msg->wIndex)
	{
		return StartPickSingleOne(pPlayer, pPlayer->m_pCheckItem, msg->wIndex);
	}
	else
	{
		std::list<SAddItemInfo> itemList;

		for (int i = 0; i < pPlayer->m_pCheckItem->m_ItemsInPack; i++)
		{
			if (!pPlayer->m_pCheckItem->m_GroundItem[i].wIndex)
			{
				rfalse(4, 1, "[Itemuser] - RecvPickupItem() - !pPlayer->m_pCheckItem->m_GroundItem[i].wIndex");
				return FALSE;
			}

			MY_ASSERT(pPlayer->m_pCheckItem->m_GroundItem[i].wIndex);

			const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(pPlayer->m_pCheckItem->m_GroundItem[i].wIndex);
			if (!pData)
				return FALSE;

			itemList.push_back(SAddItemInfo(pPlayer->m_pCheckItem->m_GroundItem[i].wIndex, pPlayer->m_pCheckItem->m_GroundItem[i].overlap));
		}

		if (itemList.size() && !CanAddItems(itemList))
		{
			TalkToDnid(pPlayer->m_ClientIndex, "�����������޷�ʰȡ��");
			return FALSE;
		}

		// ����Ϊֹ��ʰȡһ�����ǳɹ��ģ�����ʰȡ
		for (int i=0; i<pPlayer->m_pCheckItem->m_ItemsInPack; ++i)
			PickOneItem(pPlayer->m_pCheckItem->m_GroundItem[i]);

		// ������ҵĻ�����
		if (0 != pPlayer->m_pCheckItem->m_Money)
			pPlayer->AddPlayerMoney(pPlayer->m_pCheckItem->m_MoneyType, pPlayer->m_pCheckItem->m_Money);

		pPlayer->m_pCheckItem->m_Remain = 0;
		pPlayer->m_pCheckItem->RemoveMe();
		
		return TRUE;
	}
}

BOOL CItemUser::RecvDropItem(SQDropItemMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvDropItem(SQDropItemMsg *pMsg) - !pMsg");
		return FALSE;
	}

	if (!IsItemPosValid(pMsg->stItem.wCellPos))
		return FALSE;

	WORD linePos = pMsg->stItem.wCellPos;
	
	SPackageItem *pIB = FindItemByPos(linePos, XYD_FT_ONLYUNLOCK);		
	if (!pIB)
	{
		rfalse(4, 1, "[ItemUser] - RecvDropItem~ - !pIB");
		return FALSE;
	}
	MY_ASSERT(pIB);

	const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(pIB->wIndex);
	if (!pData)
		return FALSE;

	if (!pData->m_CanDrop)
		return FALSE;

	//+ ������߲��ܶ���,��Ҫ֪ͨ�ͻ���
	if (ITEM_IS_TASK(pData->m_Type))
	{
		return FALSE;
	}

	SPackageItem temp = *pIB;

	if (DelItem(*pIB, "�ӱ���������"))
	{
		// ���ɾ���ɹ����ж��Ƿ����
// 		CItem::SParameter param;
// 		ZeroMemory(&param, sizeof(param));
// 		
// 		param.dwLife		= MakeLifeTime(1);
// 		param.xTile			= m_curTileX;
// 		param.yTile			= m_curTileY;
// 		param.PackageModel	= IPM_ITEM;
// 		param.m_ItemsInPack	= 0;
// 		
// 		if (!CItem::GenerateNewGroundItem(m_ParentRegion, 3, param, temp, 0))
// 			return FALSE;
	}

	return TRUE;
}

// �ͻ�Ҫ���ȡװ����Ϣ
BOOL CItemUser::RecvRequireItemInfoMsg(SQRequireItemInfoMsg *pMsg)
{

	return 1;
}

BOOL CItemUser::RecvUnEquipItem(SQUnEquipItemMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvUnEquipItem(SQUnEquipItemMsg *pMsg) - !pMsg");
		return FALSE;
	}
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	if (pMsg->dwGlobalID != player->GetGID())
		return FALSE;

	return ToPackage(pMsg->byEquipPos);
}

BOOL CItemUser::IsItemPosValid(WORD wCellPos, BOOL canBeLock, BOOL canBeEmpty)
{
	WORD linePos = wCellPos;

	if (linePos > PackageAllCells)
		return FALSE;

	ValidationPos(linePos);

	// ����δ����
	if (0 == (m_PackageItems[linePos].m_State & SItemWithFlag::IWF_ACTIVED))
		return FALSE;

	// �����޵���
	if (!canBeEmpty && (0 == (m_PackageItems[linePos].m_State & SItemWithFlag::IWF_ITEMIN)))
		return FALSE;

	// ���ӱ�����
	if (!canBeLock && m_PackageItems[linePos].m_State & SItemWithFlag::IWF_LOCKED)
		return FALSE;

	return TRUE;
}

BOOL CItemUser::CanUseIt(CPlayer *pPlayer, SPackageItem *pIB, const SItemBaseData *pData)
{
	if (!pPlayer || !pIB || !pData)
	{
		rfalse(4, 1, "Itemuser.cpp - CanEquipIt() - !pPlayer || !pIB || !pData");
		return FALSE;
	}

	if (!pData->m_CanUse)
		return FALSE;

	// �Ա���֤
	if (pData->m_Sex != 0 && (pPlayer->m_Property.m_Sex ? 1 : 2) != pData->m_Sex)
		return FALSE;

	// ������֤
	if (pData->m_School != 0 && pPlayer->m_Property.m_School != pData->m_School)
		return FALSE;

	// �ȼ�������֤
	if (pData->m_Level != 0 && pPlayer->m_Property.m_Level < pData->m_Level)
		return FALSE;

	// �����ȴ
	if (CheckItemCDTime(pData))
		return FALSE;

	WORD wcanuse = 0;
	if (g_Script.GetItemUseLimit(pPlayer, "GetItemUseLimit", pData->m_ID, wcanuse))
	{
		return wcanuse > 0 ? TRUE : FALSE;
	}

	return TRUE;
}

BOOL CItemUser::CanEquipIt(CPlayer *pPlayer, SPackageItem *pIB, const SItemBaseData *pData)
{
	if (!pPlayer || !pIB || !pData)
	{
		rfalse(4, 1, "Itemuser.cpp - CanEquipIt() - !pPlayer || !pIB || !pData");
		return FALSE;
	}

// 	int n = sizeof(SItemBase);
// 	n= sizeof(SPackageItem);
// 	n = sizeof(SRawItemBuffer);
// 	n  = sizeof(SEquipment);

	BOOL result = ITEM_IS_EQUIPMENT(pData->m_Type);
	
	result &= CanUseIt(pPlayer, pIB, pData);

	// �;ö�Ϊ0������װ��
	if (0 == reinterpret_cast<SEquipment *>(static_cast<SRawItemBuffer*>(pIB))->attribute.currWear)
		result &= FALSE;

	return result;
}

BOOL CItemUser::RecvUseItem(SQUseItemMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvUseItem() - !pMsg");
		return FALSE;
	}

	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	if (!IsItemPosValid(pMsg->wCellPos))
		return FALSE;

	SPackageItem *pIB = FindItemByPos(pMsg->wCellPos, XYD_FT_ONLYUNLOCK);
	if (!pIB)
	{
		rfalse(4, 1, "[ItemUser] - RecvUseItem~");
		return FALSE;
	}

	MY_ASSERT(pIB);

	const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(pIB->wIndex);
	if (!pData)
		return FALSE;
	if (ITEM_IS_BOOK(pData->m_Type))	//ʹ�ü�������������
	{
		BOOL bRet = _UseBookUpSkill(pIB);
		if (bRet)
		{
			DWORD ItemID = pIB->wIndex;
			WORD Overlap = pIB->overlap;
			Overlap--;
			pPlayer->DelItem(*pIB);
			if (Overlap > 0)
				pPlayer->RecvAddItem(ItemID, Overlap);

		}
		return bRet;
	}

	if (ITEM_IS_EQUIPMENT(pData->m_Type))
        return _EquipItem(pIB);
	if (ITEM_IS_SKILLCHIP(pData->m_Type))
	{
		g_Script.SetPlayer(pPlayer);
		if (g_Script.PrepareFunction("UsePetSkillClipOpt"))
		{
			g_Script.PushParameter(pIB->wIndex);
			g_Script.Execute();
		}
		g_Script.CleanPlayer();
		return TRUE;
	}
	if (ITEM_IS_PETSKILLBOOK(pData->m_Type))
	{
		SAOpenPetStudySkillPlanMsg Msg;
		Msg.m_Result = 1;
		g_StoreMessage(pPlayer->m_ClientIndex, &Msg, sizeof(SAOpenPetStudySkillPlanMsg));
		return TRUE;
	}

	//if (ITEM_IS_FIGHTPET(pData->m_Type))
	//	/*return pPlayer->_fpItemTofp(pIB);*/
	//	return pPlayer->UseItemGetPet(pIB);

	if(pData->m_LocSrvPreTime)
	{
		if (CanUseIt(pPlayer,pIB,pData)){
			if(!SetPreTime(pIB,pData->m_LocSrvPreTime))
				return FALSE;
		}
	}
	else
	{
		if (!_UseItem(pIB))
			return FALSE;
	}

	UpdateUseItemTaskFlag(pPlayer, pData->m_ID);

    return TRUE;
}

BOOL CItemUser::InPackage(WORD wCellPos1, WORD wCellPos2)
{
	// �ڱ������ƶ�
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	if (!IsItemPosValid(wCellPos1) || !IsItemPosValid(wCellPos2, false, true))
	{
		return FALSE;
	}

	// �������ڵĵ����޷������Ƴ�
	WORD srcLinePos = wCellPos1;
	WORD desLinePos = wCellPos2;

	ValidationPos(srcLinePos);
	ValidationPos(desLinePos);

	// ���Դ��Ʒ����
	SPackageItem *srcItem = FindItemByPos(srcLinePos, XYD_FT_ONLYUNLOCK);
	if (!srcItem)
	{
		rfalse(4, 1, "[ItemUser] - InPackage~");
		return FALSE;
	}
	MY_ASSERT(srcItem);

	const SItemBaseData *srcData = CItemService::GetInstance().GetItemBaseData(srcItem->wIndex);
	if (!srcData)
		return FALSE;
	
	if (srcLinePos == desLinePos)
	{
		SendItemSynMsg(srcItem);
		return TRUE;
	}

	// �������ƶ��м���״��
	// 1. Ŀ��λ��Ϊ�գ�ֻ����û�е��ߵ�����²Ż᷵��0
	SPackageItem *desItem = FindItemByPos(desLinePos, XYD_FT_ONLYUNLOCK);
	if (0 == desItem)
	{
		if (!FillItemPos(srcLinePos, SItemWithFlag::IWF_ITEMIN, false))return FALSE;
		
		if(!FillItemPos(desLinePos, SItemWithFlag::IWF_ITEMIN, true))return FALSE;
		

		memcpy(&m_PackageItems[desLinePos].m_Item, &m_PackageItems[srcLinePos].m_Item, sizeof(SPackageItem));
		memset(&m_PackageItems[srcLinePos].m_Item, 0, sizeof(SPackageItem));

		// ����Դ���ߵ�ɾ����Ϣ
		SADelPackageItemMsg msg;
		msg.stItem = m_PackageItems[desLinePos].m_Item;
		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(msg));

		// �޸�Դ���ߵ�����
		m_PackageItems[desLinePos].m_Item.wCellPos = wCellPos2;
		SendItemSynMsg(&m_PackageItems[desLinePos].m_Item);

		ValidationPos(srcLinePos);
		ValidationPos(desLinePos);
		return TRUE;
	}

	const SItemBaseData *desData = CItemService::GetInstance().GetItemBaseData(desItem->wIndex);
	if (!desData)
		return FALSE;

	// 2. ������˫��Ϊ��ͬ�ĵ��߻���Ϊ��ͬ�ĵ��ߣ������ɵ��ӣ���ֱ�ӽ���
	// ע��ڶ����жϣ�������2���������ͬ���Ͳ��ɵ��Ӻ���ͬ���Ϳɵ��ӣ�����Ŀ��������������
	if ((srcItem->wIndex != desItem->wIndex) || ((srcItem->wIndex == desItem->wIndex) && desData->m_Overlay == desItem->overlap))
	{
		WORD wtmpPos = desItem->wCellPos;

		desItem->wCellPos = srcItem->wCellPos;		
		srcItem->wCellPos = wtmpPos;

		SPackageItem temp = *srcItem;
		memcpy(srcItem, desItem, sizeof(SPackageItem));
		memcpy(desItem, &temp, sizeof(SPackageItem));

		SendItemSynMsg(srcItem);
		SendItemSynMsg(desItem);
	}
	else
	{
		// ������˫��Ϊ��ͬ���ߣ�ִ���ص�����
		DWORD remain = desData->m_Overlay - desItem->overlap;

		if (remain >= srcItem->overlap)
		{
			ChangeOverlap(desItem, srcItem->overlap, true);
			DelItem(*srcItem);
		}
		else
		{	
			desItem->overlap = desData->m_Overlay;
			srcItem->overlap -= (WORD)remain;

			SendItemSynMsg(srcItem);
			SendItemSynMsg(desItem);
		}
	}

	ValidationPos(srcLinePos);
	ValidationPos(desLinePos);
	return TRUE;
}

BOOL CItemUser::ToPackage(BYTE byPos,BOOL onlydel )
{
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	if (byPos >= EQUIP_P_MAX || (player->m_Property.m_Equip[byPos].wIndex == 0))
		return FALSE;

	WORD emptyPos = FindBlankPos(XYD_PT_BASE);
	if (0xffff == emptyPos)
		return FALSE;
	if (IsLockedEquipColumnCell(byPos))
	{
		rfalse(2, 1, "UnEquip - EquipColumnCell was locked. pos = %d", byPos);
		return FALSE;
	}
	
	// ����Ϊ�˺����Ĭ�ϲ���װ�����Ӧ
	if (0 == player->m_Property.m_Equip[byPos].uniID)
	{
		extern BOOL GenerateNewUniqueId(SItemBase &item);

		// ˵�������ݿ������Ĭ�ϲ����ģ���ô���������һ��UniqueID
		if (!GenerateNewUniqueId(player->m_Property.m_Equip[byPos]))
			return FALSE;
	}
	bool ret = false;
	if (onlydel)
		 ret =  DelEquipment(byPos);
	else
	     ret = AddExistingItem(player->m_Property.m_Equip[byPos], emptyPos, true) && DelEquipment(byPos);

	//������Ϊֵ 
	player->UpdatePlayerXwzMsg();

	return ret ? TRUE : FALSE;
}

BOOL CItemUser::ToEquip(WORD wCellPos)
{
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	if (!IsItemPosValid(wCellPos))
		return FALSE;

	WORD linePos = wCellPos;
	SPackageItem *pIB = FindItemByPos(linePos, XYD_FT_ONLYUNLOCK);
	if (!pIB)
	{
		rfalse(4, 1, "[ItemUser] - ToEquip() ~");
		return FALSE;
	}
	MY_ASSERT(pIB);

	return _EquipItem(pIB);
}

// Func: �������߽���һ������
// @ vecCell ���������б�
// @ lock �Ƿ�����
// @ sync �Ƿ�ͬ�����ͻ���
// return �Ƿ��������ɹ���
BOOL CItemUser::LockBatchCells(std::vector<WORD> &vecCell, bool lock, bool sync)
{
	BOOL bRet = FALSE;
	for (int i = 0; i < vecCell.size(); ++i)
	{
		WORD wPos = vecCell.at(i);		
		bRet |= LockItemCell(wPos, lock, sync);
	}
	return bRet;
}

BOOL CItemUser::LockItemCell(WORD wCellPos, bool lock, bool sync)
{
	WORD linePos = wCellPos;
	
	// [2012-9-13 19-05 gw: + ����װ�����Ĵ���]
	//update by ly 2014/4/10 ���ڵ�������ֻ����60������
	/*if (linePos >= PackageAllCells) */
	if (linePos >= BaseEnd)
	{
		if (GetItemPosType(linePos) == EEquipColumn_PosType)
		{
			return LockEquipColumnCell(linePos, lock, sync);
		}		
		return FALSE;
	}
	SPackageItem *pItem = FindItemByPos(linePos, lock ? XYD_FT_ONLYUNLOCK : XYD_FT_ONLYLOCK);
	if (!pItem)
	{
		rfalse(4, 1, "[ItemUser] - LockItemCell()");
		return FALSE;
	}
	MY_ASSERT(pItem);

	if (!FillItemPos(wCellPos, SItemWithFlag::IWF_LOCKED, lock))
	{
		rfalse(4, 1, "[ItemUser] - LockItemCell()");
		return FALSE;

		MY_ASSERT(0);
	}
		
	if (sync)
	{
		CPlayer *pPlayer = (CPlayer *)this;
		if (!pPlayer)
			return FALSE;

		SALockItemCell lockMsg;
		lockMsg.bTyte	= SALockItemCell::SLC_PACKAGE;
		lockMsg.wPos = linePos;
		lockMsg.bLock	= lock;

		g_StoreMessage(pPlayer->m_ClientIndex, &lockMsg, sizeof(SALockItemCell));
	}

	return TRUE;
}

BOOL CItemUser::SynItemFlag(SPackageItem *item)
{
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	SAUpdateItemAttribute UpdateItemMsg;

	UpdateItemMsg.wCellPos		= item->wCellPos;	
	UpdateItemMsg.bFlag			= item->flags;
	UpdateItemMsg.wIndex		= item->wIndex;
	UpdateItemMsg.dwGlobalID	= player->GetGID();

	g_StoreMessage(player->m_ClientIndex, &UpdateItemMsg, sizeof(SAUpdateItemAttribute));

	return TRUE;
}

// �յ�װ���������Ϣ
BOOL CItemUser::RecvForgeItem(struct SQStartForge *pMsg)
{
	return FALSE;
	if (!pMsg)
	{
		rfalse(4, 1, "Itemus.cpp - BOOL CItemUser::RecvForgeItem(struct SQStartForge *pMsg) - !pMsg");
		return FALSE;
	}
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;
	SAStartForge msg;
	// �����쳣~
	if (SQStartForge::ASF_INLAY_JEWEL != pMsg->bType)
	{
		msg.Result = SAStartForge::SAS_INLAY_NoGEM;
		g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAStartForge));
		return FALSE;
	}
	if (player->m_refineType != SQAddEquip::ADE_GEM_INSERT)
	{	
		msg.Result = SAStartForge::SAS_INLAY_Error;
		g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAStartForge));
		return FALSE;
	}
	
	if (0 == player->m_vecStrengthenEquip.size())
	{
		return FALSE;
	}
	const SEquipment* pEquip = GetEquipmentbyItem(player->m_vecStrengthenEquip.at(0), XYD_FT_ONLYLOCK);
	if (!pEquip)
	{
		msg.Result = SAStartForge::SAS_INLAY_EQUIP;
		g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAStartForge));
		return FALSE;
	}
	if (!IsItemPosValid(pMsg->wCellPos))
	{
		msg.Result = SAStartForge::SAS_INLAY_XY;
		g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAStartForge));
		return FALSE;
	}

	SPackageItem *srcItem = FindItemByPos(pMsg->wCellPos, XYD_FT_ONLYUNLOCK);
	if (!srcItem)
	{
		msg.Result = SAStartForge::SAS_INLAY_XY_LOCK;
		g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAStartForge));
		return FALSE;
	}

	const SItemBaseData *srcData = CItemService::GetInstance().GetItemBaseData(srcItem->wIndex);
	if (!srcData)
	{
		msg.Result = SAStartForge::SAS_NoFindGEM;
		g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAStartForge));
		return FALSE;
	}

	if (!ITEM_IS_JEWELEX(srcData->m_Type))
	{
		msg.Result = SAStartForge::SAS_NoFindGEM;
		g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAStartForge));
		return FALSE;
	}
	//LockItemCell(player->m_forgeItem->byCellX, player->m_forgeItem->byCellY, false);
	//player->m_forgeMetrial = srcItem;
	player->LockBatchCells(player->m_vecStrengthenMetrial, false);
	player->m_vecStrengthenMetrial.clear();
	player->m_vecStrengthenMetrial.push_back(pMsg->wCellPos);

	player->m_forgeType	= pMsg->bType;	
	SendNextLevelEquip(SAAfterUpgradeEquipMsg::GEM_INSERT);
	LockItemCell(pMsg->wCellPos, true);

	return TRUE;
}

// �յ����������Ϣ
BOOL CItemUser::RecvDragMetrail(struct SQDragMetrial *pMsg)
{	
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvDragMetrial() - !pMsg");
		return FALSE;
	}

	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	if (pPlayer->m_vecStrengthenEquip.empty())
	{
		return FALSE;
	}
	pPlayer->LockBatchCells(pPlayer->m_vecStrengthenMetrial, false);
	pPlayer->m_vecStrengthenMetrial.clear();

	BYTE	byPosX = 0;
	BYTE	byPosY = 0;
	int		iMetrialCellNum = pMsg->byPosNum>SQDragMetrial::msc_wMaxLen?SQDragMetrial::msc_wMaxLen:pMsg->byPosNum;
	for (int i = 0; i < iMetrialCellNum; ++i)
	{
		if (!IsItemPosValid(pMsg->waPos[i]))
			return FALSE;

		SPackageItem *srcItem = FindItemByPos(pMsg->waPos[i], XYD_FT_ONLYUNLOCK);
		if (!srcItem)
			return FALSE;

		const SItemBaseData *srcData = CItemService::GetInstance().GetItemBaseData(srcItem->wIndex);
		if (!srcData)
			return FALSE;

		// �������һЩ�жϼ��

		if (pPlayer->m_forgeMetrial)
		{
			rfalse(2, 1, "�Ѿ������˲��ϣ������ǰ����~~");
			LockItemCell(pPlayer->m_forgeMetrial->wCellPos, false);
		}	
		pPlayer->m_forgeMetrial = srcItem;

		pPlayer->m_vecStrengthenMetrial.push_back(pMsg->waPos[i]);		
		LockItemCell(byPosX, byPosY, true);
	}	

	return TRUE;
}

// �յ�ȡ��������Ϣ
BOOL CItemUser::RecvEndForgeItem(struct SQEndForge *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvEndForgeItem() - !pMsg");
		return FALSE;
	}

	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;
 
	bool EquipOff	= false;
	bool MetrailOff = false;
 	
	switch (pMsg->bOps)
	{
	case SQEndForge::QEF_CLOSE:
	case SQEndForge::QEF_EQUIP_OFF:
		EquipOff	= true;
		MetrailOff	= true;
		break;
	case SQEndForge::QEF_METRAIL_OFF:
		MetrailOff	= true;
		break;
 	}
 
// 	if (EquipOff && player->m_forgeItem)
//  	{
// 		LockItemCell(player->m_forgeItem->byCellX, player->m_forgeItem->byCellY, false);
// 		player->m_forgeItem = 0;
// 		player->m_forgeType = SQStartForge::ASF_NONE;
//  	}
 
	if (MetrailOff && player->m_forgeMetrial)
 	{
		LockItemCell(player->m_forgeMetrial->wCellPos, false);
		player->m_forgeMetrial = 0;
 	}

	return TRUE;
}

// ��ʯ��Ƕ
BOOL CItemUser::RecvInLayJewel(struct SQInLayJewel *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvInLayJewel() - !pMsg");
		return FALSE;
	}
	SAInLayJewel msg;
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	if (SQAddMetrial::ADM_RAW_METRIAL != player->m_forgeType)
	{	
		return FALSE;
	}
	if (player->m_refineType != SQAddEquip::ADE_GEM_INSERT)
	{
		return FALSE;
	}
	
	if (!player->m_forgeMetrial)
	{
		OnthSysNotice(1);
		return FALSE;
	}
	if ( player->m_forgeMetrial->overlap <= 0 )
	{
		msg.result = SAInLayJewel::ALJ_LOCK_TOOL;
		g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAInLayJewel));
		OnthSysNotice(1);
		return FALSE;
	}	
	if (0 == player->m_vecStrengthenEquip.size())
	{
		return FALSE;
	}
	WORD wEquipPos = player->m_vecStrengthenEquip.at(0);
	SEquipment *pEquip = GetEquipmentbyItem(wEquipPos, XYD_FT_WHATEVER);
	if (!pEquip)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvInLayJewel() - !pEquip");
		return FALSE;
	}
	
	int index =	-1;
	size_t i = 0;
	for (; i < SEquipment::MAX_SLOTS; ++i)
	{
		if (pEquip->slots[i].isEmpty())//��һ���ո���
		{
			index = i; break;
		}
	}
	if (-1 == index)
		return FALSE;

	const SJewelAttribute* pJewel = CItemService::GetInstance().GetJewelAttribute(player->m_forgeMetrial->wIndex);
	
	if (!pJewel)
	{
		rfalse(4 ,1, "[ItemUser] - RecvInlayJewel()~");
		return FALSE;
	}
	
	const SInsertGem * pSInsertGem = CItemService::GetInstance().GetInsertGemInfo(index);
	if (!pSInsertGem)
	{
		return FALSE;
	}
	msg.index = index;
	msg.type  = 0;
	msg.value = 0;
	
	if (!player->ReducedCommonMoney(pSInsertGem->GemMoney))	
	{
		msg.result = SAInLayJewel::ALJ_LOCK_MONEY;
		g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAInLayJewel));
		OnthSysNotice(2);
		return FALSE;
	}	

	// ִ��Ƕ��
	pEquip->slots[i].type  = (player->m_forgeMetrial->wIndex - 4004001) / 100 + 1;
	pEquip->slots[i].value = player->m_forgeMetrial->wIndex - (pEquip->slots[i].type - 1) * 100 - 4004001 + 1;

	msg.type   = pEquip->slots[i].type;
	msg.value  = pEquip->slots[i].value;
	SendUpdateEquipInfo(pEquip);
	msg.result = SAInLayJewel::ALJ_SUCCESS;
	g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SAInLayJewel));
	OnthSysNotice(22);
	//LockItemCell(player->m_forgeMetrial->byCellX,player->m_forgeMetrial->byCellY,false);
	if (player->m_forgeMetrial->overlap > 1)
	{
		player->m_forgeMetrial->overlap--;
		player->SendItemSynMsg(player->m_forgeMetrial, XYD_FT_ONLYLOCK);
	}
	else
	{
		DelItem(*player->m_forgeMetrial, "��Ƕ���ģ�", true);
		player->m_forgeMetrial = 0;
		player->m_forgeType = 0;
	}
	bool Flag1 = SendNeedMETRIALInfo(player,SQAddEquip::ADE_GEM_INSERT);
	if (!Flag1)
	{			
		LockItemCell(wEquipPos,false);
		player->ClearMetrialRelation();
		player->m_refineType = 0;
		OnthSysNotice(23);
		return FALSE;
	}
	CheckAndUpdateEquipColumnAttribyPos(player, wEquipPos);
	//player->CheckPlayerMoney(XYD_UM_ONLYBIND, BaseMoney, false);

	return TRUE;
}

// ��ʯժ��
BOOL CItemUser::RecvTakeoffJewel(struct SQTakeOffJewel *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvTakeoffJewel() - !pMsg");
		return FALSE;
	}

	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	/*if (SQStartForge::ASF_TAKEOUT_JEWEL != player->m_forgeType || !player->m_forgeItem || !player->m_forgeMetrial)
		return FALSE;*/

	/*if (pMsg->pos > 5)
		return FALSE;*/
	SATakeOffJewel msg;

	if (player->m_refineType != SQAddEquip::ADE_GEM_REMOVE)
	{
		msg.result = SATakeOffJewel::ATJ_protocol_ERROR;
		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SATakeOffJewel));
		return FALSE;
	}
	bool nflasg = FALSE;
	if (pMsg->SelectType == 0 || pMsg->SelectType == 2)//����ʯ
	{
		nflasg = TRUE;
	}
	if (!nflasg)
	{
		msg.result = SATakeOffJewel::ATJ_NoFindHole;
		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SATakeOffJewel));
		return FALSE;
	}	
	if (0 == player->m_vecStrengthenEquip.size())
	{
		return FALSE;
	}
	SEquipment* pEquip = GetEquipmentbyItem(player->m_vecStrengthenEquip.at(0), XYD_FT_WHATEVER);

	if (!pEquip)
	{
		rfalse(4, 1, "[itemuser.cpp] - RecvTakeoffJewel!");
		return FALSE;
	}
	BOOL nindexFlag = FALSE;
	int index = SEquipment::MAX_SLOTS - 1;
	for (; index >= 0; --index)
	{
		if (pEquip->slots[index].isJewel())
		{
			nindexFlag = TRUE;
			break;
		}
	}
	if (!nindexFlag)
	{
		msg.result = SATakeOffJewel::ATJ_NoFindHole;
		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SATakeOffJewel));
		return FALSE;
	}
	const STakeOffGem * pStakeoff = CItemService::GetInstance().GetSTakeOffGemInfo(index);
	if (!pStakeoff)
	{
		return FALSE;
	}
	if (pStakeoff->TakeOffGemMoney > player->GetPlayerMoney(pStakeoff->TakeOffGemMoneyType))
	{
		msg.result = SATakeOffJewel::ATJ_LACK_MONEY;
		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SATakeOffJewel));
		OnthSysNotice(2);
		return FALSE;
	}
	std::vector<SPackageItem* > vec1;
	long nFlg = 0;
	ProtectInfoStruct* pPro = 0;
	if (pMsg->SelectType == 2)
	{
		pPro = CProtectInfo::GetInstance().FindProtectInfoStruct(pMsg->SelectType);
		if (!pPro)
		{
			msg.result = SATakeOffJewel::ATJ_Metrial_ERROR;
			g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SATakeOffJewel));
			OnthSysNotice(1);
			return FALSE;
		}
		nFlg = OnGetNeedMetrial(player,1,vec1,pPro->ItemID);
		if (nFlg == 2)
		{
			msg.result = SATakeOffJewel::ATJ_Metrial_ERROR;
			g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SATakeOffJewel));
			OnthSysNotice(1);
			return FALSE;
		}
	}
	std::vector<SPackageItem*> vec;
	nFlg = OnGetNeedMetrial(player,pStakeoff->TakeOffGemNum,vec,player->m_MetrialItemID);
	if (nFlg != 1)
	{
		msg.result = SATakeOffJewel::ATJ_Metrial_ERROR;//���ϲ���
		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SATakeOffJewel));
		OnthSysNotice(1);
		return FALSE;
	}
	msg.pos	   = index;	
	msg.result = SATakeOffJewel::ATJ_SUCCESS;
	player->ChangeMoney(pStakeoff->TakeOffGemMoneyType,-pStakeoff->TakeOffGemMoney);
	bool bHaveBindMetrial = false;
	OnProcessNeedMetrial(player,vec,pStakeoff->TakeOffGemNum,"��װ��ժ����ʾ��ԭ�������ģ�", bHaveBindMetrial);
	if (pMsg->SelectType == 2)
	{
		OnProcessNeedMetrial(player,vec1,1,"����������ʾ��ԭ�������ģ�",bHaveBindMetrial);
	}
	
	DWORD dwJewIndex = 4004001 + (pEquip->slots[index].type - 1) * 100 + (pEquip->slots[index].value - 1);
	SRawItemBuffer item;
	GenerateNewItem(item, SAddItemInfo(dwJewIndex, 1));
	item.flags |= SItemBase::F_SYSBINDED;

	WORD pos = FindBlankPos(XYD_PT_BASE);
	bool kGIVEfLAG = AddExistingItem(item, pos, true);
	if (!kGIVEfLAG)
	{
		msg.result = SATakeOffJewel::ATJ_LACK_SPACE;
		OnthSysNotice(24);
		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SATakeOffJewel));
		return FALSE;
	}
	long Var = CRandom::RandRange(1, 100);
	if (pMsg->SelectType == 2)
	{
		Var = 100;
	}
	if (Var > pStakeoff->TakeOffGemHoleRale) //�ɹ�
	{
		pEquip->slots[index].type  = 0;
		pEquip->slots[index].value = 1;
		SendUpdateEquipInfo(pEquip);
		msg.result = SATakeOffJewel::ATJ_SUCCESS;
		msg.pos = index;
		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SATakeOffJewel));
		OnthSysNotice(25);
	}
	else
	{
		pEquip->slots[index].type  = 0;
		pEquip->slots[index].value = 0;
		SendUpdateEquipInfo(pEquip);
		msg.result = SATakeOffJewel::ATJ_CLOSED;
		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SATakeOffJewel));
		OnthSysNotice(26);
	}
	// �������ӱ�ʯ
	BOOL nindexFlag1 = FALSE;
	int index1 = SEquipment::MAX_SLOTS - 1;
	for (; index1 >= 0; --index1)
	{
		if (pEquip->slots[index1].isJewel())
		{
			nindexFlag1 = TRUE;
			break;
		}
	}
	if (nindexFlag1)
	{
		SendNextLevelEquip(SAAfterUpgradeEquipMsg::GEM_REMOVE);
		SendNeedMETRIALInfo(player,SQAddEquip::ADE_GEM_REMOVE);
	}
	else
	{
		RemoveRefineMetrial();
		OnthSysNotice(27);
	}
	//// �۳����ҺͲ���
	//if (player->m_forgeMetrial->overlap > 1)
	//{
	//	player->m_forgeMetrial->overlap--;
	//	player->SendItemSynMsg(player->m_forgeMetrial, XYD_FT_ONLYLOCK);
	//}
	//else
	//{
	//	DelItem(*player->m_forgeMetrial, "ժ������", true);
	//	player->m_forgeMetrial = 0;
	//}

	//player->CheckPlayerMoney(XYD_UM_ONLYBIND, BaseMoney, false);

	return TRUE;
}

BOOL CItemUser::StartMakeHole()
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	int index =	-1;

	SAMakeHole msg;
	msg.index = SAMakeHole::AMH_FAILED;
	bool nflags = FALSE;
	if (pPlayer->m_HoleType == 0 || pPlayer->m_HoleType == 1)
	{
		nflags = TRUE;
	}
	if (!nflags )
	{
		return FALSE;
	}
	if (pPlayer->m_refineType != SQAddEquip::ADE_ADD_HOLE)
	{
		/*msg.result = SAMakeHole::AMH_FAILED;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMakeHole));*/
		return FALSE;
	}

	if (0 == pPlayer->m_vecStrengthenEquip.size())
	{
		return FALSE;
	}
	WORD wEquipPos = pPlayer->m_vecStrengthenEquip.at(0);
	SEquipment* pEquip = GetEquipmentbyItem(wEquipPos, XYD_FT_WHATEVER);
	if (!pEquip)
	{
		rfalse(4, 1, "[itemuser.cpp] - StartMakehole()");
		return FALSE;
	}

	// �ж�װ���׶��Ƿ��Ѿ������Լ��õ��׶�����
	for (size_t i = 0; i < SEquipment::MAX_SLOTS; ++i)
	{
		if (pEquip->slots[i].isInvalid())	
		{
			index = i; break;
		}
	}

	if (-1 == index)
	{
		//rfalse(2, 1, "װ���Ķ��Ѿ�ȫ���򿪣��޷��ٿ���~");
		msg.result = SAMakeHole::AMH_ALL_OPENED;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMakeHole));
		return FALSE;
	}
	msg.index = index;
	// ��ȡ�������ID�ͽ�Ǯ
	const SMakeHoleInfo * pMakeHole = CItemService::GetInstance().GetMakeHoleInfo(index);
	if (!pMakeHole)
	{
		return FALSE;
	}
	const DWORD needItemID = pMakeHole->HoleNeedID;
	const DWORD needMoney  = pMakeHole->HoleMoney;
	WORD   wmakeSuccessRate   = pMakeHole->wMakeSuccessRate;
	//
	if (pMakeHole->HoleMoney > pPlayer->GetPlayerMoney(pMakeHole->HoleMoneyType))
	{
		msg.result = SAMakeHole::AMH_LACK_MONEY;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMakeHole));
		OnthSysNotice(2);
		return FALSE;
	}
	std::vector<SPackageItem*> vec;
	long nFlg = OnGetNeedMetrial(pPlayer,pMakeHole->HoleNum,vec,pPlayer->m_MetrialItemID);
	if (nFlg == 2)
	{
		msg.result = SAMakeHole::AMH_LACK_TOOL;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMakeHole));
		OnthSysNotice(1);
		return FALSE;
	}
	std::vector<SPackageItem*> vec1;
	long nNeedMetrial = 0;
	ProtectInfoStruct* pPro = 0;
	BYTE byStrengthenExternChoose = pPlayer->m_HoleType;
// 	if (pPlayer->m_HoleType == 1)//ʹ���˳ɹ���
// 	{
// 		pPro = CProtectInfo::GetInstance().FindProtectInfoStruct(pPlayer->m_HoleType);
// 		if (!pPro)
// 		{
// 			msg.result = SAMakeHole::AMH_PrctectMetrial;	// ȱ�ٲ���
// 			g_StoreMessage(pPlayer->m_ClientIndex,&msg,sizeof(SAMakeHole));
// 			OnthSysNotice(1);
// 			return FALSE;
// 		}
// 		nNeedMetrial = OnGetNeedMetrial(pPlayer,1,vec1,pPro->ItemID);
// 		if (nNeedMetrial == 2)
// 		{
// 			msg.result = SAMakeHole::AMH_PrctectMetrial;	
// 			g_StoreMessage(pPlayer->m_ClientIndex,&msg,sizeof(SAMakeHole));
// 			OnthSysNotice(1);
// 			return FALSE;
// 		}
// 	}
	pPlayer->ChangeMoney(pMakeHole->HoleMoneyType,-pMakeHole->HoleMoney);
	bool bHaveBindMetrial = false;
	OnProcessNeedMetrial(pPlayer,vec,pMakeHole->HoleNum,"��װ�������ʾ��ԭ�������ģ�",bHaveBindMetrial);

	//////////////////////////////////////////////////////////////////////////
	bool bHaveLuckyCharmItem = false;
	// [��ȡ���˷����еĻ��۳�һ�����ɹ�������]	
	if (byStrengthenExternChoose & EStrengthenChoose_LuckyCharm)
	{
		std::vector<SPackageItem*> vecLuckyCharm;
		nFlg = pPlayer->OnGetNeedMetrial(pPlayer, pMakeHole->byLuckyCharmNum, vecLuckyCharm, pMakeHole->dwLuckyCharmID);	
		if (nFlg == 1)
		{
			pPlayer->OnProcessNeedMetrial(pPlayer, vecLuckyCharm, pMakeHole->byLuckyCharmNum, "�۳���һ�����˷���", bHaveBindMetrial);
			bHaveLuckyCharmItem = true;
		}
	}
	// [��ȡ������,����б������Ļ����۳�һ����������]
	bool bHaveProtectionCharacterItem = false;
	if (byStrengthenExternChoose & EStrengthenChoose_ProtectionCharacter)
	{
		std::vector<SPackageItem*> vecProtectionCharacter;
		nFlg = pPlayer->OnGetNeedMetrial(pPlayer, pMakeHole->byProtectionCharacterNum, vecProtectionCharacter, pMakeHole->dwProtectionCharacterID);
		bHaveProtectionCharacterItem = (nFlg == 1);
		if (nFlg == 1)
		{
			pPlayer->OnProcessNeedMetrial(pPlayer, vecProtectionCharacter, pMakeHole->byProtectionCharacterNum, "�۳���һ����������", bHaveBindMetrial);
			bHaveProtectionCharacterItem = true;
		}
	}

	// ������ʣ��ɹ�����ʧ��
	long ktemp = CRandom::RandRange(1, 10000);
	if (bHaveLuckyCharmItem)
	{		
		wmakeSuccessRate += pMakeHole->wLuckyCharmAddSuccessRate;		
	}
	// �������죬���Կ�ʼ�����
	if (ktemp <= wmakeSuccessRate)
	{
		//rfalse(2, 1, "���%d����������Ϊ%d����׳ɹ�~", msg.index, makeRate);
		OnthSysNotice(19);
		pEquip->slots[index].type  = 0;
		pEquip->slots[index].value = 1;
		SendUpdateEquipInfo(pEquip);
		msg.result = SAMakeHole::AMH_SUCCESS;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMakeHole));

		int index = 0;
		bool Flag = FALSE;
		for (; index < SEquipment::MAX_SLOTS; ++index)
		{
			if (pEquip->slots[index].isInvalid())
			{
				Flag = TRUE;
				break;
			}
		}
		if (!Flag)
		{			
			LockItemCell(wEquipPos, false);
			pPlayer->ClearMetrialRelation();
			pPlayer->m_refineType = 0;
			OnthSysNotice(21);
		}
		else
		{
			SendNextLevelEquip(SAAfterUpgradeEquipMsg::ADD_HOLE);
			SendNeedMETRIALInfo(pPlayer,SQAddEquip::ADE_ADD_HOLE);
		}
	}
	else
	{
		msg.result = SAMakeHole::AMH_FAILED;
		OnthSysNotice(20);
// 		if (index > 0 && !bHaveProtectionCharacterItem && CRandom.RandRange(0, 10000) < pMakeHole->wReduceHoleRate)
// 		{			
//			msg.result = SAMakeHole::AMH_FAILED_REDUCEHOLE;
// 			rfalse(2, 1, "������:id=%d", pEquip->wIndex);
// 			pEquip->slots[index-1].type = 0;
// 			pEquip->slots[index-1].value = 0;
// 			SendUpdateEquipInfo(pEquip);
// 		}
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMakeHole));
		return FALSE;
	}
	
	return TRUE;
}

// װ�����
BOOL CItemUser::RecvMakeHole(struct SQMakeHole *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvMakeHole() - !pMsg");
		return FALSE;
	}

	//return FALSE;
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;
	pPlayer->m_HoleType = pMsg->byStrengthenExternChoose;
	StartMakeHole();

// 	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
// 	if (!pPlayer)
// 		return FALSE;
// 
// 	if (0xffffffff != pPlayer->m_MakeHoleTime)
// 		return FALSE;
// 		
// 	pPlayer->m_MakeHoleTime = timeGetTime();
	
	return TRUE;
}

BOOL CItemUser::ToForge(BYTE byX, BYTE byY)
{
	/*
	CPlayer *pp = (CPlayer*)DynamicCast(IID_PLAYER);
	if (pp == NULL)  return FALSE;
	SPackageItem *pStart = FindItemByPos(byX,byY);
	if(pStart == NULL) return FALSE;
	if(pp->SetForgeData(pStart))  // ��װ���;ƵĻ�
	{
	// send message to client
	int nGoodsID = 0, nDrinkNum = 0, nSuccess = 0;
	DWORD nMoney = 0;
	pp->GetForgeData(nGoodsID, nDrinkNum, nSuccess, nMoney);
	SAForgeGoodsMsg msg;
	msg.byWhat = 1;
	msg.wEquipment = nGoodsID;
	msg.wDrinkNum = nDrinkNum;
	msg.wSuccess = nSuccess;
	msg.wMoney = nMoney;
	if(pp->m_nMoney > nMoney)
	{
	g_StoreMessage(pp->m_ClientIndex, &msg, sizeof(SAForgeGoodsMsg));
	// DelItem(pStart, false);//, false);

	if(pStart->wIndex == 49)
	DelItem(pStart);
	else
	{
	SADelPackageItemMsg msg;
	msg.stItem = *pStart;
	g_StoreMessage(pp->m_ClientIndex, &msg, sizeof(SADelPackageItemMsg));
	}
	return TRUE;
	}
	else
	{
	SRoleTask *pRT = FindTask(FORGE);
	if(pRT)
	DelTask(pRT);
	SABuy SMsg;
	SMsg.nRet = 0;
	SMsg.nMoney = pp->m_nMoney;
	g_StoreMessage(pp->m_ClientIndex, &SMsg, sizeof(SABuy));
	}
	}
	*/
	return FALSE;
}

BOOL CItemUser::ToFuse(BYTE byX, BYTE byY)
{
	/*
	CPlayer *pp = (CPlayer*)DynamicCast(IID_PLAYER);
	if (pp == NULL)  return FALSE;
	SPackageItem *pIB = FindItemByPos(byX, byY);
	if(pIB == NULL) return FALSE;
	if(pp->SetFuseData(pIB))
	{
	g_Script.SetCondition(NULL, pp, NULL);
	g_Script.CallFunc("R_FuseMoney");
	g_Script.CleanCondition(); 
	DWORD nMoney = (int)g_Script.GetRet(0);
	SAForgeGoodsMsg msg;
	msg.byWhat = 2;
	msg.wEquipment = pIB->wIndex;
	msg.wMoney = nMoney;
	if(pp->m_nMoney > nMoney)
	{
	g_StoreMessage(pp->m_ClientIndex, &msg, sizeof(SAForgeGoodsMsg));
	// DelItem(pIB, false);

	SADelPackageItemMsg msg;
	msg.stItem = *pIB;
	g_StoreMessage(pp->m_ClientIndex, &msg, sizeof(SADelPackageItemMsg));

	return TRUE;
	}
	else
	{
	SRoleTask *pRT = FindTask(FUSE);
	if(pRT)
	DelTask(pRT);
	SABuy SMsg;
	SMsg.nRet = 0;
	SMsg.nMoney = pp->m_nMoney;
	g_StoreMessage(pp->m_ClientIndex, &SMsg, sizeof(SABuy));
	}

	}
	*/

	return FALSE;
}

BOOL CItemUser::RecvSplitItem(QuerySplitItemMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvSplitItem() - !pMsg");
		return FALSE;
	}
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	if (!IsItemPosValid(pMsg->src.wCellPos))
		return FALSE;

	WORD linePos = pMsg->src.wCellPos;
	if (!ValidationPos(linePos))return FALSE;

	SPackageItem *source = FindItemByPos(linePos, XYD_FT_ONLYUNLOCK);
	if (!source)
	{
		rfalse(4, 1, "[itemuser.cpp] - RecvSplitItem()");
		return FALSE;
	}
	MY_ASSERT(source);

	const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(source->wIndex);
	if (!itemData)
		return FALSE;	
	//+ ��������޷����
	if (ITEM_IS_TASK(itemData->m_Type))
	{
		return FALSE;
	}

	if (0 == pMsg->count || pMsg->count >= source->overlap)
		return FALSE;

	// ������/�󶨵ĵ��߲��ܲ��
	if ((source->flags & SItemBase::F_SYSBINDED) || (source->flags & SItemBase::F_PLAYERBINDED))
		return FALSE;

	// ��֤��������ֺ�ֻ��ռ��һ�����ӣ�����ֻ��Ҫһ������
	WORD emptyPos = FindBlankPos(XYD_PT_BASE);
	if (0xffff == emptyPos)
		return FALSE;

	// �ȴ����µ�
	SPackageItem new_item = *source;
	new_item.overlap = pMsg->count;

	extern BOOL GenerateNewUniqueId(SItemBase &item);
	if (!GenerateNewUniqueId(new_item))
		return FALSE;

	AddExistingItem(new_item, emptyPos, true);

	// �����޸�ԭ�е�
	ChangeOverlap(source, pMsg->count, false);

	ValidationPos(emptyPos);
	ValidationPos(linePos);

	return FALSE;
}

BOOL CItemUser::RecvOpenBuyBackList()
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	SASaledItemList msg;
	msg.bCount = pPlayer->m_curSaledItemCount;
	memcpy(msg.iSaledList, pPlayer->m_buyBackItem, sizeof(msg.iSaledList));

	g_StoreMessage(pPlayer->m_ClientIndex, &msg, (sizeof(msg) - ((sizeof(SaledItemType) * (CONST_BUYBACK_COUNT-msg.bCount)))));

	return 0;
}

BOOL CItemUser::RecvMoveItem(SQMoveItemMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvMoveItem() - !pMsg");
		return FALSE;
	}

	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;
	//����ڵ��߶���ʹ��״̬�У���������Ʒ���ƶ�
	//if(pPlayer->GetReadyEvent() && pPlayer->GetReadyEvent()->IsEvent("UseItemEvent"))
	//{
	//	pPlayer->BreakOperation();	// �жϲ���
	//}
	pPlayer->m_BatchReadyEvent.BreakOperationReadyEvent(Event::EEventInterrupt_MoveItem);
	
	switch (pMsg->byMoveTo)
	{
	case IMD_INPACKAGE:		// ������
		{
			bool Flag = InPackage(pMsg->wCellPos1,pMsg->wCellPos2);
			if (!Flag)
			{
				SAMoveItemMsg msg;
				msg.wIndex = 0;
				g_StoreMessage(pPlayer->m_ClientIndex,&msg,sizeof(SAMoveItemMsg));
			}
			return Flag;
		}
		break;

	case IMD_PACK2EQUIP:	// �϶���װ
		return ToEquip(pMsg->wCellPos1);
		break;

	case IMD_EQUIP2PACK:	// �϶�жװ
		return ToPackage(pMsg->wCellPos1); // pMsg->CellY1
		break;

	case IMD_FP_PACK2EQUIP:	// �����϶���װ
		return _fpToEquip(pMsg->wCellPos1,pMsg->byindex);
		break;

	case IMD_FP_EQUIP2PACK:	// �����϶�жװ
		return _fpToPackage(pMsg->wCellPos1,pMsg->byindex); //pMsg->CellY1
		break;
	}
	return FALSE;
}

BOOL CItemUser::RecvAddItem(DWORD wIndex, WORD iNum, WORD level)
{
	if (0 == wIndex || 0 == iNum) 
		return FALSE;

	const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(wIndex);
	if (!pData)
		return FALSE;

	const CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	std::list<SAddItemInfo> itemList;
	itemList.push_back(SAddItemInfo(wIndex, iNum));

	// �ж��ܷ����
	if (!CanAddItems(itemList))
	{
		TalkToDnid(pPlayer->m_ClientIndex, "�����������޷�ʰȡ��");
		return FALSE;
	}

	return StartAddItems(itemList);
}

CNpc *CItemUser::GetCurrentNpc()
{
	if (!m_ParentRegion)
	{
		StackWalker wp;
		wp.ShowCallstack();
		rfalse(4,1, "itemuser.cpp - GetCurrentNpc() - !m_ParentRegion");
		return 0;
	}

	if (0 == m_CommNpcGlobalID)
	{
		rfalse(4, 1, "itemuser.cpp - GetCurrentNpc() - 0 == m_CommNpcGlobalID");
		return 0;
	}

	//LPIObject npc = m_ParentRegion->SearchObjectListInAreas(m_CommNpcGlobalID , m_ParentArea->m_X, m_ParentArea->m_Y);
	LPIObject npc = m_ParentRegion->SearchObjectListByRegion(m_CommNpcGlobalID); //���ΰ汾
	if (npc)
	{
		CNpc *pNpc = (CNpc*)npc->DynamicCast(IID_NPC);
		return pNpc;
	}

	return 0;
}

WORD CItemUser::GetBlankCell(BYTE type)
{
	DWORD startPos	= 0;
	DWORD endPos	= 0;

	switch (type)
	{
	case XYD_PT_BASE:
		startPos = BaseStart;
		endPos	 = BaseEnd;
		break;
	default:
		return 0;
	}

	WORD blankCount = 0;

	for (DWORD i=startPos; i<endPos; ++i)
	{
		ValidationPos(i);

		if ((m_PackageItems[i].m_State & SItemWithFlag::IWF_ACTIVED) && (0 == (m_PackageItems[i].m_State & SItemWithFlag::IWF_ITEMIN)))
			blankCount++;
	}

	return blankCount;
}

// ��ȡIDΪindex�ĵ����ڱ����еĿɵ�����
WORD CItemUser::GetRemainOverlap(DWORD index)
{
	if (index <= 0)
	{
		return 0;
	}

	MY_ASSERT(index);

	// ������û��IDΪindex�ĵ���
	ItemState::iterator itemState = m_ItemsState.find(index);
	if (itemState == m_ItemsState.end())
		return 0;

	return itemState->second;
}

// ��ȡ����Ŀո�����Ŀ
WORD CItemUser::GetNeedBlank(const SItemBaseData *pData, const SAddItemInfo &Info)
{
	if (!pData)
	{
		rfalse(4 ,1, "[ItemUser] - GetNeedBlank() - !pData");
		return 0;
	}

	if (!Info.m_Index || !Info.m_Count)
	{
		rfalse(4, 1, "[Itemuser] - GetNeedBlank() - info.m_Index [%d], info.m_count [%d]", Info.m_Index, Info.m_Count);
		return 0;
	}
	MY_ASSERT(pData && Info.m_Index && Info.m_Count);

	// ���Ȳ鿴�ܵ��ӵ���Ŀ
	WORD canOverap = GetRemainOverlap(Info.m_Index);
	
	// �������ȫ����
	if (canOverap >= Info.m_Count)
		return 0;

	WORD remainCount = Info.m_Count - canOverap;

	// ����޷���ȫ���ӣ���������������
	return (remainCount <= pData->m_Overlay) ? 1 : (remainCount/(DWORD)pData->m_Overlay + ((0 == remainCount%pData->m_Overlay) ? 0 : 1));
}

WORD CItemUser::FindBlankPos(BYTE type)
{
	if (type < XYD_PT_BASE || type >= XYD_PT_MAX)
	{
		rfalse(4, 1, "Itemuser.cpp - FindBlankPos() - type < XYD_PT_BASE || type >= XYD_PT_MAX");
		return 0xffff;
	}
	MY_ASSERT(type >= XYD_PT_BASE && type < XYD_PT_MAX);
	
	DWORD startPos	= 0;
	DWORD endPos	= 0;

	switch (type)
	{
	case XYD_PT_BASE:
		startPos = BaseStart;
		endPos	 = BaseEnd;
		break;
	}

	for (DWORD i=startPos; i<endPos; ++i)
	{
		ValidationPos(i);

		if ((m_PackageItems[i].m_State & SItemWithFlag::IWF_ACTIVED) && (0 == (m_PackageItems[i].m_State & SItemWithFlag::IWF_ITEMIN)))	
			return i;
	}

	return 0xffff;
}

//////////////////////////////////////////////////////////////////////////
// [2012-8-23 18-06 gw: +��������������Ե���Ŀ�������Ѿ���������Ŀ������ ������]
BOOL CItemUser::ResetRandomExtraAttriEntry(SEquipment* equip, const vector<int> &vecLockedIndex)
{
	return TRUE;
}

// ���װ����������Ը���
BOOL CItemUser::AddRandomAttri(SEquipment* equip)
{
// 	if (!equip)
// 	{
// 		rfalse(4, 1, "Itemuser.cpp - AddRandomAttri() - !equip");
// 		return FALSE;
// 	}
// 
// 	// �õ�װ������������б�
// 	const SEquipExtraAttriTable *ExtraAttribute = CItemService::GetInstance().GetExtraAttriTable(equip->wIndex);
// 
// 	if (!ExtraAttribute)
// 	{
// 		rfalse(2, 1, "�Ҳ��� %d װ�������������Ϣ", equip->wIndex);
// 		return FALSE;
// 	}
// 
// 	// ����ID��װ���ȼ� * 1000 + װ��Ʒ�� * 100 + װ������
// 	const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(equip->wIndex);
// 	if (!pData)
// 	{
// 		rfalse(4, 1, "[ItemUser.cpp] - AddRandomAttri()");
// 		return FALSE;
// 	}
// 	MY_ASSERT(pData);
// 
// 	// ��ȡ�������������Ա�
// 	const SMaxExtraAttri *pMaxExtraAttri = CItemService::GetInstance().GetMaxExtraAttri(pData, equip->attribute.grade);
// 	if (!pMaxExtraAttri && pData->m_Level >= 10)
// 	{
// 		rfalse(2, 1, "�Ҳ��� %d װ���ġ�����������%d,  %d   %d  ", index,pData->m_Level,pData->m_Color,equip->attribute.grade);
// 		return FALSE;
// 	}
// 
// 	// ���װ����������Ը���
// 	int ExAttriNum = CRandom::RandRange(ExtraAttribute->MinNum, ExtraAttribute->MaxNum);
// 	
// 	if (ExAttriNum < 1 && ExAttriNum > SEquipment::MAX_BORN_ATTRI)
// 	{
// 		rfalse(2, 1, "�洢���Ը���������Խ�磡");
// 		return FALSE;
// 	}
// 	ExAttriNum = CRandom::RandRange(1,100);
// 	ExAttriNum = CItemService::GetInstance().GetRandomNum(pData->m_Color,ExAttriNum);
// 	if (ExAttriNum == 0)
// 	{
// 		rfalse(2, 1,"GetRandomNum");
// 		return FALSE;
// 	}
// 	std::vector<int> IndexVec(ExAttriNum, SEquipDataEx::EEA_MAX);	// �����洢���������������
// 	
// 	//int RateBase		 = 10000;									// ���ʻ���
// 	int TempRate		 = 0;
// 	int TypeIndex		 = 0;										// ���ڴ洢���ո��ӵ�װ���ϵ���������
// 	int AttriRate[SEquipDataEx::EEA_MAX];							// ������Եĸ�������
// 
// 	/*for (int index = SEquipDataEx::EEA_MAXHP; index < SEquipDataEx::EEA_MAX; ++index)
// 	{
// 		TempRate += ExtraAttribute->ExtraData[index][2];
// 		AttriRate[index] = TempRate;
// 	}*/
// 
// 	//MY_ASSERT(10000 == AttriRate[SEquipDataEx::EEA_MAX]);
// 
// 	for (int num = 0; num < ExAttriNum; num++)
// 	{
// 		while (true) 
// 		{
// 			int index = CRandom::RandRange(0, SEquipDataEx::EEA_MAX);//�����������
// 			if (find(IndexVec.begin(), IndexVec.end() ,index) == IndexVec.end() )//û���ҵ�
// 			{
// 				IndexVec[TypeIndex++] = index;
// 				break;	
// 			}
// 			else
// 			{
// 				continue;
// 			}
// 		}
// 		//for (int j = SEquipDataEx::EEA_MAXHP; j < SEquipDataEx::EEA_MAX; ++j)
// 		//{
// 		//	if ((SEquipDataEx::EEA_MAXHP == j && rate < AttriRate[j]) ||
// 		//		(j > SEquipDataEx::EEA_MAXHP && rate < AttriRate[j] && rate > AttriRate[j - 1]))
// 		//	{
// 		//		/*if (find(IndexVec.begin(), IndexVec.end(), j) != IndexVec.end())
// 		//		{
// 		//			rate = CRandom::RandRange(1, 10000);
// 		//			j = SEquipDataEx::EEA_MAXHP;
// 		//			continue;
// 		//		}*/
// 
// 		//		/*if (ExtraAttribute->ExtraData[j][2] == 0)
// 		//			break;*/
// 
// 		//		IndexVec[TypeIndex++] = j;
// 		//		break;
// 		//	}
// 		//}
// 	}
// 	sort(IndexVec.begin(), IndexVec.end());
// 	//MY_ASSERT(ExAttriNum == TypeIndex);
// 
// 	for (size_t k = 0; k < IndexVec.size(); ++k)
// 	{
// 		int AttriType = IndexVec[k];
// 
// 		// �ų�0����ֵ
// 		if (0 == ExtraAttribute->ExtraData[AttriType][0] || 0 == ExtraAttribute->ExtraData[AttriType][1])
// 		{
// 			rfalse(2, 1, "������һ���յ��������ֵ!");
// 			continue;
// 		}
// 
// 		// ��ӳ�ʼ�����������ֵ
// 		equip->attribute.bornAttri[k].type  = AttriType;
// 		equip->attribute.bornAttri[k].value = CRandom::RandRange(ExtraAttribute->ExtraData[AttriType][0], 
// 																 ExtraAttribute->ExtraData[AttriType][1]);
// 		if (pData->m_Level >= 10)
// 		{
// 			// �����������������ֵ����ֵ�ǳ�ʼ�����������ֵ+�����ĸ�������ֵ
// 			equip->attribute.MaxExtraAttri[k].type	= AttriType;
// 			equip->attribute.MaxExtraAttri[k].value	= ExtraAttribute->ExtraData[AttriType][1] + pMaxExtraAttri->ExtraData[AttriType];
// 		}
// 	}

	return TRUE;
}

// func:�����Ʒ��ǰӦ�����õİ�״̬,Ŀǰ�İ󶨶���λ��Ұ�
// param: eCurBindType-��ǰ�жϵİ����� ITEM_BIND_TYPE
// param: byItemDataBindType-��Ʒ����ԭʼ������ 
BYTE  CItemUser::GetSItemBaseBindState( BYTE eCurBindType, BYTE byItemDataBindType)
{
	BYTE byBindState = (BYTE)((eCurBindType & byItemDataBindType) ? SItemBase::F_NONE : SItemBase::F_PLAYERBINDED);
	return byBindState;
}

BOOL CItemUser::GenerateNewItem(SRawItemBuffer &itemBuffer, const SAddItemInfo &params,BYTE index)
{
	if (params.m_Index == 0 || params.m_Count == 0)
	{
		rfalse(4,1,"CItemUser::GenerateNewItem %d(ID),  %d(Count)",params.m_Index,params.m_Count);
		StackWalker sw;
		sw.ShowCallstack();
		return FALSE;
	}

	MY_ASSERT(params.m_Index && params.m_Count);

	const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(params.m_Index);
	if (!itemData)
	{
		rfalse(4,1,FormatString("����һ������ĵ���%d",params.m_Index));
		return FALSE;
	}

	MY_ASSERT(itemData);

	if (params.m_Count > itemData->m_Overlay)
	{
		rfalse(4, 1, "[ItemUser.cpp] - GetnerageNewItem() - params.m_Count = %d", params.m_Count);
		//StackWalker sw;
		//sw.ShowCallstack();
		return FALSE;
	}
	MY_ASSERT(params.m_Count <= itemData->m_Overlay);

	memset(&itemBuffer, 0, sizeof(itemBuffer));

	extern BOOL GenerateNewUniqueId(SItemBase &item);
	if (!GenerateNewUniqueId(itemBuffer))
		return FALSE;

	SItemBase *item = &itemBuffer;
	item->wIndex	= params.m_Index;
	item->usetimes	= 1;
	item->flags		|= GetSItemBaseBindState(EITEM_BIND_GET, itemData->m_BindType);
	item->overlap	= params.m_Count;

	// ���񵤵ĵ�������
	if (HUISHENDAN_ID == itemBuffer.wIndex)
	{
		BYTE *pBuf = itemBuffer.buffer;
		memset(pBuf, 0, sizeof(pBuf));

		BYTE updateTimes = 0;
		
		const SHuiYuanDanInfo* pInfo = CItemService::GetInstance().GetHuiYuanDanInfo(updateTimes);
		if (!pInfo)
		{
			rfalse(4, 1, "[ItemUser.cpp] - GenerateNewItem - pInfo");
			return FALSE;
		}

		WORD SpValue = pInfo->wSPValue;

		// �ɹ������������Ѿ�������������0��һ��ռ2��byte
		pBuf += 2 * sizeof(BYTE);

		// �õ���ǰ������ֵ
		memcpy(pBuf, &SpValue, sizeof(WORD));
	}

	// װ����
	if (ITEM_IS_EQUIPMENT(itemData->m_Type))
	{
		if (1 != itemData->m_Overlay)
		{
			rfalse(4, 1, "[ItemUser.cpp] - GenerateNewItem - 1 != itemData->m_Overlay");
			return FALSE;
		}
		MY_ASSERT(1 == itemData->m_Overlay);
		
		SEquipment *equip = reinterpret_cast<SEquipment *>(item);
		equip->attribute.currWear = itemData->m_MaxWear;
		equip->attribute.maxWear  = itemData->m_MaxWear;
		equip->wIndex = itemData->m_ID;
		equip->flags = item->flags;
		equip->overlap = item->overlap;
		equip->usetimes = item->usetimes;

		equip->attribute.grade	  = itemData->m_StepNum;//Ʒ��0
		equip->attribute.bCurrentStar = 0 ; //��ǰ�Ǽ�0
		equip->attribute.bIntensifyLevel = 1;	//ǿ���ȼ�1

		for (size_t i=0; i<SEquipment::MAX_SLOTS; i++)
			equip->slots[i].type = 0, equip->slots[i].value = 0;

		// װ������������������
		memset(equip->attribute.bornAttri, 0, MAX_ATTRI_NUM * sizeof(SEquipment::ExtraAttri));  //ǿ������
		memset(equip->attribute.starAttri,0,MAX_ATTRI_NUM*sizeof(SEquipment::ExtraAttri));	//��������
		memset(equip->attribute.MaxJDingAttri, 0, SEquipment::MAX_SLOTS * sizeof(SEquipment::ExtraAttri)); //��������
	
		equip->attribute.bJDingAttriNum = 0;   //��ʼ��������������Ϊ0
		OnUpdataGrade(equip);
		// ���ǳ�ʼ��
//		ZeroMemory(equip->attribute.starNum, SEquipment::MAX_STAR_NUM * sizeof(BYTE));
		//AddRefineAttribute(equip);
	}

	return TRUE;
}

WORD CItemUser::TryPossibleMerge(DWORD index, WORD count, WORD maxOverlap)
{
	WORD willCreate = count;

	// ������ɵ��ӣ���ô��������
	if (1 == maxOverlap)
		return willCreate;

	// ��ִ�п��ܵĺϲ�
	ItemState::iterator specItem = m_ItemsState.find(index);
	if (specItem == m_ItemsState.end() || 0 == specItem->second)
		return willCreate;

	// �������߱�
	for (DWORD i = 0; i < PackageAllCells; i++)
	{
		ValidationPos(i);

		if (0 == specItem->second || 0 == willCreate)
			break;

		SPackageItem *pItem = FindItemByPos(i, XYD_FT_ONLYUNLOCK);
		if (pItem && index == pItem->wIndex && maxOverlap != pItem->overlap)
		{
			WORD remainOverlap = maxOverlap - pItem->overlap;
			if (maxOverlap <= pItem->overlap)
			{
				rfalse(4, 1, "[Itemuser.cpp] - TryPossibleMerge - MY_ASSERT(maxOverlap > pItem->overlap)");
				return FALSE;
			}

			if (specItem->second < remainOverlap)
			{
				rfalse(4, 1, "[Itemuser.cpp] - TryPossibleMerge - MY_ASSERT(specItem->second >= remainOverlap)");
				return FALSE;
			}

			MY_ASSERT(maxOverlap > pItem->overlap);
			MY_ASSERT(specItem->second >= remainOverlap);

			if (willCreate >= remainOverlap)		// �޷�һ���ص���
			{
				pItem->overlap += remainOverlap;
				SendItemSynMsg(pItem);
				
				if (pItem->overlap != maxOverlap)
				{
					rfalse(4, 1, "[ItemUser.cpp] - TryPossibleMerge - pItem->overlap != maxOverlap");
					return FALSE;
				}
				MY_ASSERT(pItem->overlap == maxOverlap);

				willCreate -= remainOverlap;
				specItem->second -= remainOverlap;
			}
			else
			{
				pItem->overlap += willCreate;
				SendItemSynMsg(pItem);

				if (pItem->overlap == maxOverlap)
				{
					rfalse(4, 1, "[ItemUser.cpp] - TryPossibleMerge - pItem->overlap == maxOverlap");
					return FALSE;
				}
				MY_ASSERT(pItem->overlap != maxOverlap);

				specItem->second -= willCreate;
				willCreate = 0;
			}
		}

		ValidationPos(i);
	}

	//rfalse(2, 1, "ִ����ϲ���IDΪ%d�ĵ���Ŀǰ�����ٵ���%d��", specItem->first, m_ItemsState[specItem->first]);

	return willCreate;
}

int CItemUser::InsertItem(DWORD index, WORD willCreate, const SItemBaseData *pData, SRawItemBuffer &item, bool genNew)
{
	if (!pData)
	{
		rfalse(4, 1, "Itemuser.cpp - InsertItem() - !pData");
		return -1;
	}
	int  npos = -1;
	while (true)
	{
		if (0 == willCreate)
			break;

		WORD count = (willCreate > pData->m_Overlay) ? pData->m_Overlay : willCreate;

		if (genNew)
			GenerateNewItem(item, SAddItemInfo(pData->m_ID, count));
		else
		{
			if (item.wIndex != index)
			{
				rfalse(4, 1, "[ItemUser.cpp] - InsertItem - item.wIndex != index");
				return -1;
			}
			MY_ASSERT(item.wIndex == index);
			item.overlap = count;
		}

		// �ڲ�����Ʒʱ�ű���չ
		if (genNew)
		{
			lite::Variant ret;
			g_Script.SetRawItemBuffer(&item);
			LuaFunctor(g_Script, FormatString("OnGenerateNewItem%d", pData->m_ScriptID))[pData->m_ID](&ret);
			g_Script.CleanRawItemBuffer();
		}
		// ��������ͨ������
		WORD pos = FindBlankPos(XYD_PT_BASE);
		AddExistingItem(item, pos ,true);

		willCreate -= count;
		npos = pos;
	}
	return npos;
}

BOOL CItemUser::StartAddItems(std::list<SAddItemInfo> &itemList)
{
	if (itemList.empty())
	{
		rfalse(4,1,"�����Ʒ����Ʒ�б�Ϊ��");
		return FALSE;
	}

	MY_ASSERT(!itemList.empty());

	for (std::list<SAddItemInfo>::iterator it = itemList.begin(); it != itemList.end(); ++it)
	{
		const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(it->m_Index);
		if (!pData)
		{
			rfalse(4, 1, "[ItemUser.cpp] - StarAddItems() - !pData");
			return FALSE;
		}
		MY_ASSERT(pData);

		// ��ִ�п��ܵĺϲ�
		WORD willCreate = TryPossibleMerge(it->m_Index, it->m_Count, pData->m_Overlay);

		SRawItemBuffer itemBuffer;
		InsertItem(it->m_Index, willCreate, pData, itemBuffer, true);
	}

	return TRUE;
}

// �ܷ��ڱ����м���һЩ����
BOOL CItemUser::CanAddItems(std::list<SAddItemInfo> &itemList, WORD reuseBaseBlank, WORD reuseTaskBlank)
{
	if (itemList.empty())
	{
		rfalse(4, 1, "[Itemuser.cpp] - CanAddItems() - itemlist.empty()");
		return FALSE;
	}
	MY_ASSERT(!itemList.empty());

	std::map<DWORD, WORD> addList;
	for (std::list<SAddItemInfo>::iterator it = itemList.begin(); it != itemList.end(); ++it)
	{
		if (!it->m_Index)
		{
			rfalse(4, 1, "[ItemUser.cpp] - CanAddItems() - !it->m_Index");
			return FALSE;
		}
		MY_ASSERT(it->m_Index);
		addList[it->m_Index] += it->m_Count;
	}

	WORD baseNeedBlank	= 0;		// ����������Ҫ�ĸ���
	WORD taskNeedBlank	= 0;		// ���񱳰���Ҫ�ĸ���
	
	WORD baseBlankCell	= GetBlankCell(XYD_PT_BASE) + reuseBaseBlank;
	WORD taskBlankCell	= GetBlankCell(XYD_PT_BASE) + reuseTaskBlank;	


	CPlayer *player = (CPlayer*)DynamicCast(IID_PLAYER);
	const SItemBaseData *pData = 0;
	for (std::map<DWORD, WORD>::iterator it = addList.begin(); it != addList.end(); ++it)
	{
		pData = CItemService::GetInstance().GetItemBaseData(it->first);
		if (!pData)
		{
			TalkToDnid(player->m_ClientIndex, "û���ҵ��õ��ߵ���Ϣ��");
			return FALSE;
		}

		WORD needBlank = GetNeedBlank(pData, SAddItemInfo(it->first, it->second));
		
		if (0 == needBlank)
			continue;

		if (ITEM_IS_TASK(pData->m_Type))
		{
			taskNeedBlank += needBlank;
			if (taskNeedBlank > taskBlankCell)
			{
				TalkToDnid(player->m_ClientIndex, "����������������㣡");
				return FALSE;
			}
		}
		else
		{
			baseNeedBlank += needBlank;
			if (baseNeedBlank > baseBlankCell)
			{
				TalkToDnid(player->m_ClientIndex, "�������Ӳ��㣡");
				return FALSE;
			}
		}
	}

	return TRUE;
}

bool CItemUser::ValidationPos(WORD pos)
{
	if (pos>= PackageAllCells)return false;
	
	//MY_ASSERT(pos < PackageAllCells);

	SItemWithFlag *pItemCell = &m_PackageItems[pos];

	if (pos >= BaseStart && pos < BaseEnd)		// ���ڻ����ĵ��߸���
	{
		// ���δ�����ô���Ӳ�Ӧ�ñ��������е���
		bool Condition1 = ((0 == (pItemCell->m_State & SItemWithFlag::IWF_ACTIVED)) && (0 == (pItemCell->m_State & SItemWithFlag::IWF_ITEMIN)) &&
			(0 == (pItemCell->m_State & SItemWithFlag::IWF_LOCKED)) && (0 == memcmp(&pItemCell->m_Item, &m_EmptyItem4Debug, sizeof(SPackageItem))));

		// �������е��ߣ�����û������
		bool Condition2 = (pItemCell->m_State & SItemWithFlag::IWF_ACTIVED) && (pItemCell->m_State & SItemWithFlag::IWF_ITEMIN) &&
			(0 == (pItemCell->m_State & SItemWithFlag::IWF_LOCKED)) && (0 != memcmp(&pItemCell->m_Item, &m_EmptyItem4Debug, sizeof(SPackageItem))) &&
			(0 != pItemCell->m_Item.wIndex) && ((pItemCell->m_Item.wCellPos) == pos);
		
		// ������û�е��ߣ����ܱ�����
		bool Condition3 = (pItemCell->m_State & SItemWithFlag::IWF_ACTIVED) && (0 == (pItemCell->m_State & SItemWithFlag::IWF_ITEMIN)) &&
			(0 == (pItemCell->m_State & SItemWithFlag::IWF_LOCKED)) && (0 == memcmp(&pItemCell->m_Item, &m_EmptyItem4Debug, sizeof(SPackageItem)));

		// �������е��ߣ����Ա�����
		bool Condition4 = (pItemCell->m_State & SItemWithFlag::IWF_ACTIVED) && (pItemCell->m_State & SItemWithFlag::IWF_ITEMIN) &&
			(pItemCell->m_State & SItemWithFlag::IWF_LOCKED) && (0 != memcmp(&pItemCell->m_Item, &m_EmptyItem4Debug, sizeof(SPackageItem))) &&
			(0 != pItemCell->m_Item.wIndex) && ((pItemCell->m_Item.wCellPos) == pos);
		
		if (!Condition1 && !Condition2 && !Condition3 && !Condition4)
		{
			return FALSE;
		}
		MY_ASSERT(Condition1 || Condition2 || Condition3 || Condition4);
	}
	else										// ����������ߣ����㼤����㲻����
	{
	/*	MY_ASSERT((pItemCell->m_State & SItemWithFlag::IWF_ACTIVED) && (0 == (pItemCell->m_State & SItemWithFlag::IWF_LOCKED)));

		bool Condition1 = ((pItemCell->m_State & SItemWithFlag::IWF_ITEMIN) && (0 != memcmp(&pItemCell->m_Item, &m_EmptyItem4Debug, sizeof(SPackageItem))) &&
			(0 != pItemCell->m_Item.wIndex) && ((pItemCell->m_Item.wCellPos) == pos));

		bool Condition2 = ((0 == (pItemCell->m_State & SItemWithFlag::IWF_ITEMIN)) && (0 == memcmp(&pItemCell->m_Item, &m_EmptyItem4Debug, sizeof(SPackageItem))));

		MY_ASSERT(Condition1 || Condition2);*/
		if ( (pItemCell->m_State & SItemWithFlag::IWF_ACTIVED) && (0 == (pItemCell->m_State & SItemWithFlag::IWF_LOCKED)))
		{
			bool Condition1 = ((pItemCell->m_State & SItemWithFlag::IWF_ITEMIN) && (0 != memcmp(&pItemCell->m_Item, &m_EmptyItem4Debug, sizeof(SPackageItem))) &&
				(0 != pItemCell->m_Item.wIndex) && ((pItemCell->m_Item.wCellPos) == pos));

			bool Condition2 = ((0 == (pItemCell->m_State & SItemWithFlag::IWF_ITEMIN)) && (0 == memcmp(&pItemCell->m_Item, &m_EmptyItem4Debug, sizeof(SPackageItem))));
			if (Condition1 == false && Condition2 == false)
			{
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}
		//MY_ASSERT((pItemCell->m_State & SItemWithFlag::IWF_ACTIVED) && (0 == (pItemCell->m_State & SItemWithFlag::IWF_LOCKED)));
		//MY_ASSERT(Condition1 || Condition2);
	}

	return true;
}

SPackageItem *CItemUser::FindItemByPos(WORD pos, WORD LookType)
{
	ValidationPos(pos);

	SItemWithFlag *pItemCell = &m_PackageItems[pos];
	if (!pItemCell)
	{
		rfalse(4, 1, "Itemuser.cpp - FindItemByPos() - !pItemCell");
		return 0;
	}
	
	// �˸���δ����
	if (0 == (pItemCell->m_State & SItemWithFlag::IWF_ACTIVED))
		return 0;

	// �˸����޵���
	if (0 == (pItemCell->m_State & SItemWithFlag::IWF_ITEMIN))
		return 0;

	// ����/�������߼�
	if ((XYD_FT_ONLYLOCK == LookType && (0 == (pItemCell->m_State & SItemWithFlag::IWF_LOCKED))) ||
		((XYD_FT_ONLYUNLOCK == LookType && (pItemCell->m_State & SItemWithFlag::IWF_LOCKED))))
		return 0;

	return &pItemCell->m_Item;
}


//add by ly 2014/3/25
//������Ӧ�ĵ���ID�Ƿ��ڱ�����,����ڷ��ض�Ӧ����Ϣ
SPackageItem *CItemUser::FindItemByItemID(DWORD ItemID, WORD LookType)
{
	SPackageItem *lpPackageItem = NULL;
	for (unsigned long i = 0; i < PackageAllCells; i++)
	{
		lpPackageItem = FindItemByPos(i, LookType);
		if (lpPackageItem == NULL)
			continue;
		if (lpPackageItem->wIndex == ItemID)
			return lpPackageItem;
	}
	return lpPackageItem;
}

BOOL CItemUser::FillItemPos(WORD pos, BYTE type, bool mark)
{
	// ��֤��0x1,0x2,0x4,0x8,0x10.....
/*	if (type != SItemWithFlag::IWF_ACTIVED || type != SItemWithFlag::IWF_LOCKED || type != SItemWithFlag::IWF_ITEMIN ) return FALSE;*/
	MY_ASSERT(type && (0 == (type & (type-1))));

	if (!ValidationPos(pos))return FALSE;


	SItemWithFlag *pItemCell = &m_PackageItems[pos];
	if (!pItemCell)
	{
		rfalse(4, 1, "Itemusr.cpp - FillItemPos() - !pItemCell");
		return FALSE;
	}

	if (mark && (pItemCell->m_State & type))
		return FALSE;

	if (!mark && (0 == (pItemCell->m_State & type)))
		return FALSE;

	if (mark)
		pItemCell->m_State |= type;
	else
		pItemCell->m_State &= (~type);

	// �������⴦��һ�����ӱ����Ϊ����/������ʱ��
	// �Ե���״̬�Ĵ���
	if (SItemWithFlag::IWF_LOCKED == type)
	{
		const SItemBaseData *pItemData = CItemService::GetInstance().GetItemBaseData(pItemCell->m_Item.wIndex);
		if (!pItemData)
		{
			rfalse(4, 1, "[Itemuser.cpp] - FillItemPos() - !pItemData");
			return FALSE;
		}

		if (pItemData->m_Overlay < pItemCell->m_Item.overlap)
		{
			rfalse(4, 1, "[Itemuser.cpp] - FillItemPos() - pItemCell->m_Item.overlap = %d", pItemCell->m_Item.overlap);
			return FALSE;
		}

		MY_ASSERT(pItemData && (pItemData->m_Overlay >= pItemCell->m_Item.overlap));

		if (WORD countNum = pItemData->m_Overlay - pItemCell->m_Item.overlap)
		{	
			m_ItemsState[pItemCell->m_Item.wIndex] += mark ? (-countNum) : countNum;
			//rfalse(2, 1, "%s��IDΪ%d�ĵ���Ŀǰ�����ٵ���%d��", mark ? "����" : "����", pItemCell->m_Item.wIndex, m_ItemsState[pItemCell->m_Item.wIndex]);
		}
	}
	
	return TRUE;
}

void CItemUser::SetPlayerInitItems(CPlayer *pPlayer, bool OnlyInit)
{
	if (!pPlayer)
	{
		rfalse(4, 1, "Itemuser.cpp - SetPlayerInitItems() - !pPlayer");
		return;
	}

	if (OnlyInit)
	{
		m_ItemsState.clear();

		memset(&pPlayer->m_PackageItems, 0, sizeof(pPlayer->m_PackageItems));

		
		DWORD dwNeedActivedLen = BaseActivedPackage + pPlayer->m_Property.m_wExtGoodsActivedNum;
		// ȷ����Щ���Ѿ������
		for (DWORD i = 0; i < PackageAllCells; i++)
		{
			if (i >= dwNeedActivedLen)
				continue;

			m_PackageItems[i].m_State |= SItemWithFlag::IWF_ACTIVED;

			ValidationPos(i);
		}

		// ���߻ָ�
		for (DWORD i = 0; i < PackageAllCells; i++)
		{
			ValidationPos(i);

			if (pPlayer->m_Property.m_BaseGoods[i].wIndex != 0)		// ˵���е���
				AddExistingItem(pPlayer->m_Property.m_BaseGoods[i], i, false);

			ValidationPos(i);
		}
	}
	else
	{
		SAAddPackageItemMsg msg;

		// �������еĵ�����Ϣ
		for (DWORD i = 0; i < PackageAllCells; i++)
		{
			if (FindItemByPos(i, XYD_FT_ONLYLOCK))
			{
				rfalse(4, 1, "[ItemUser.cpp] - SetPlayerInitItems() - FindItemByPos(i, XYD_FT_ONLYLOCK)");
			//	return;
			}
			//MY_ASSERT(0 == FindItemByPos(i, XYD_FT_ONLYLOCK));

			SPackageItem *pItem = FindItemByPos(i, XYD_FT_ONLYUNLOCK);
			if (pItem)
			{
				msg.stItem = *pItem;
				g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
			}
		}
	}
}

BOOL CItemUser::AddExistingItem(const SItemBase &item, WORD pos, BOOL synchro)
{
	CPlayer *pPlayer = static_cast<CPlayer *>(this);
	if (!pPlayer)
		return FALSE;

	if (!ValidationPos(pos))return FALSE;

	if (!item.wIndex)
	{
		rfalse(4, 1, "[Itemuser.cpp] - AddExistingItem() - item.wIndex = 0");
		return FALSE;
	}
	MY_ASSERT(item.wIndex);

	const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(item.wIndex);
	if (!itemData)
	{
		rfalse(4, 1, "Itemuser.cpp - AddExitingItem() - !itemData");
		return FALSE;
	}
	MY_ASSERT(itemData);

	if (item.overlap > itemData->m_Overlay)
	{
		return FALSE;
	}
	MY_ASSERT(item.overlap <= itemData->m_Overlay);

	if (!FillItemPos(pos, SItemWithFlag::IWF_ITEMIN, true))
	{
		return FALSE;
		MY_ASSERT(0);
	}

	// ��ӵ���
	SCellPos &actPos = (SCellPos &)m_PackageItems[pos].m_Item;
	actPos.wCellPos = pos;
	memcpy(&(SItemBase&)m_PackageItems[pos].m_Item, &item, sizeof(SRawItemBuffer));
	UpdateItemRelatedTaskFlag(pPlayer, m_PackageItems[pos].m_Item.wIndex);

	if (m_PackageItems[pos].m_Item.overlap < itemData->m_Overlay)		// �������Ÿ���
	{
		m_ItemsState[item.wIndex] += (itemData->m_Overlay - item.overlap);
		//rfalse(2, 1, "�½���IDΪ%d�ĵ���Ŀǰ�����ٵ���%d��", item.wIndex, m_ItemsState[item.wIndex]);
	}

	if (synchro)
	{
		SAAddPackageItemMsg msg;
		msg.stItem = m_PackageItems[pos].m_Item;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}

	ValidationPos(pos);

	return TRUE;
}

BOOL CItemUser::BuyItem(DWORD index, WORD count)
{
	if (index == 0 || count == 0)
	{
		rfalse(4,1,"CItemUser::BuyItem %d  %d",index,count);
		return false;
	}
	MY_ASSERT(index && count);

	CPlayer *player = (CPlayer*)DynamicCast(IID_PLAYER);
	const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(index);

	if (!player || !itemData)
        return FALSE;

	// ��֤��Ǯ�Ƿ��㹻
	if (!player->CheckPlayerMoney(itemData->m_BuyMoneyType, itemData->m_BuyPrice * count, true))
		return FALSE;

	std::list<SAddItemInfo> itemList;
	itemList.push_back(SAddItemInfo(index, count));

	// �ж��ܷ����
	if (!CanAddItems(itemList))
	{
		TalkToDnid(player->m_ClientIndex, "�ף���������Ŷ~װ������Ŷ~");
		return FALSE;
	}

	if (StartAddItems(itemList))
	{
		player->CheckPlayerMoney(itemData->m_BuyMoneyType, itemData->m_BuyPrice * count, false);
		return TRUE;
	}

	return FALSE;
}

BOOL CItemUser::RecvActiveExtPackage(struct SQActiveExtPackage *msg)
{
	if (!msg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvActiveExtPackage - !msg");
		return FALSE;
	}

	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	WORD wActivedNum = player->m_Property.m_wExtGoodsActivedNum + BaseActivedPackage;	
	//lite::Variant ret;
    g_Script.SetCondition( NULL, player, NULL, NULL );
	//LuaFunctor( g_Script, "ActiveExtendPackage")[BaseActivedPackage][player->m_Property.m_wExtGoodsActivedNum][msg->wWillActiveNum][BaseEnd] ( &ret );
	BYTE ActivePackageCellIndex = player->m_Property.m_wExtGoodsActivedNum + BaseActivedPackage + 1;
	INT64 TimeNow = 0;
	_time64(&TimeNow);
	LuaFunctor(g_Script, "ActivePackageOpt")[ActivePackageCellIndex][(int)(player->m_Property.m_OnlineTime + TimeNow - player->m_dwLoginTime)][1][msg->wWillActiveNum]();	//���һ������Ϊ1����ʾ��ҹ��򼤻����
    g_Script.CleanCondition();
	//if (ret.dataType != lite::Variant::VT_INTEGER)
	//{
	//	return FALSE;
	//}
	//int iRetValue = ret;
	//if (iRetValue == 0)
	//{
	//	return FALSE;
	//}

	return TRUE;
}

BOOL CItemUser::RecvBuyItem(SBuy *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemsuer.cpp - RecvBuyItem() - !pMsg");
		return FALSE;
	}

	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	if (pMsg->Flag != 0)
	{
		FollowShopStruct * pFollowShopStruct = CFollowShopService::GetInstance().FindFollowStruct(player->m_CurFollowIndex);
		if (pFollowShopStruct)
		{
			if (pMsg->bBuy)
			{
				WORD index = pMsg->nPage * 40 + pMsg->index;
				if (  index   >= pFollowShopStruct->m_Num)
					return FALSE;
				DWORD nIndex = pFollowShopStruct->m_Itemlist[index];
				if (0 == nIndex || 0 == pMsg->nCount)
					return FALSE;
				return BuyItem(nIndex, pMsg->nCount);
			}
			else
			{
				if (!IsItemPosValid(pMsg->index))
					return FALSE;

				SPackageItem *sellItem = FindItemByPos(pMsg->index, XYD_FT_ONLYUNLOCK);
				if (!sellItem)
				{
					rfalse(4, 1, "itemuser.cpp - RecvBuyItem() - !sellItem");
					return FALSE;
				}
				MY_ASSERT(sellItem);

				const SItemBaseData *sellData = CItemService::GetInstance().GetItemBaseData(sellItem->wIndex);
				if (!sellData)
					return FALSE;

				if (!sellData->m_CanSale)
					return FALSE;

				DWORD sellMoney = sellData->m_SellPrice * sellItem->overlap;

				if (!ITEM_IS_TASK(sellData->m_Type))
					player->AddToSaledItemList(*sellItem, sellMoney);

				if (player->DelItem(*sellItem))
					player->AddPlayerMoney(sellData->m_SellMoneyType, sellMoney);
			}
		}
	} 
	else
	{
		CNpc *npc = GetCurrentNpc();
		if (!npc)
			return FALSE;

		if (pMsg->bBuy)				// �������
		{
			NpcGoods *saleInfo = CNpcSaleService::GetInstance().GetSaleInfo(m_CurShopID);
			if (!saleInfo)
				return FALSE;

			WORD index = pMsg->nPage*40 + pMsg->index;
			if (index >= saleInfo->m_GoodCount)
				return FALSE;
			DWORD nIndex = saleInfo->m_Goods[2*index];
			if (0 == nIndex || 0 == pMsg->nCount)
				return FALSE;

			return BuyItem(nIndex, pMsg->nCount);
		}
		else					// �������
		{
			if (!IsItemPosValid(pMsg->index))
				return FALSE;

			SPackageItem *sellItem = FindItemByPos(pMsg->index, XYD_FT_ONLYUNLOCK);
			if (!sellItem)
			{
				rfalse(4, 1, "itemuser.cpp - RecvBuyItem() - !sellItem");
				return FALSE;
			}
			MY_ASSERT(sellItem);

			const SItemBaseData *sellData = CItemService::GetInstance().GetItemBaseData(sellItem->wIndex);
			if (!sellData)
				return FALSE;

			if (!sellData->m_CanSale)
				return FALSE;

			DWORD sellMoney = sellData->m_SellPrice * sellItem->overlap;

			if (!ITEM_IS_TASK(sellData->m_Type))
				player->AddToSaledItemList(*sellItem, sellMoney);

			if (player->DelItem(*sellItem))
				player->AddPlayerMoney(sellData->m_SellMoneyType, sellMoney);
		}
	}
	

	return TRUE;
}

// ���ָ�����ͣ������ĵ����Ƿ���ڣ���ͨ��ָ��checkOnly == false��ͬʱɾ��
BOOL CItemUser::CheckGoods(DWORD index, DWORD number, BOOL checkOnly)
{
	if (0 == index || 0 == number)
		return FALSE;

	if (GetItemNum(index, XYD_FT_ONLYUNLOCK) < number)
		return FALSE;

	if (checkOnly)
		return TRUE;			// ֻ��ѯ

	for (DWORD i = 0; i < PackageAllCells; i++)
	{
		SPackageItem *pItem = FindItemByPos(i, XYD_FT_ONLYUNLOCK);
		if (pItem && pItem->wIndex == index)
		{
			if (pItem->overlap > number)
			{
				ChangeOverlap(pItem, number, false);
				number = 0;
				break;
			}

			int delNumber = pItem->overlap;
			DelItem(*pItem, "CheckGoods��ȡ��");
			number -= delNumber;
		}
	}

	if (number)
	{
		rfalse(4, 1, "Itemuser.cpp - CheckItem() - number != 0");
		return FALSE;
	}
	MY_ASSERT(0 == number);

	return TRUE;
}

BOOL CItemUser::RecvQueryUpdateItemData( struct QueryUpdateItemDataMsg *pMsg )
{


	return TRUE;
}

void CItemUser::SendUseIntEffMsg( WORD type, DWORD time, WORD in_wKindOfCD )
{
	if ( type >= SAUseIntervalEffect::EUIT_MAX )	//�µ�CD��ʱ�����ϵĴ��ͽṹ
		return;

	SAUseIntervalEffect msg;
	msg.type = type;
	msg.time = time;
	msg.wLocSrvKindNumberForCD = in_wKindOfCD;

	CPlayer *pp = (CPlayer*)DynamicCast(IID_PLAYER);
	if ( pp != NULL )
		g_StoreMessage( pp->m_ClientIndex, &msg, sizeof(SAUseIntervalEffect) );
}

BOOL CItemUser::UpdateTimeRecycleItem()
{   
	return TRUE;
	// ��ʱע�͵���zeb-2010-2-22 ������������
    //DWORD tempTime = 0xffffffff;
    //__int64 now = time( NULL );

    //CPlayer *player = ( CPlayer* )DynamicCast( IID_PLAYER );
    //if ( player == NULL )
    //    return false;

    //static std::list< SPackageItem* > delItem;

    //for ( check_list<SPackageItem>::iterator it = m_ItemList.begin(); it != m_ItemList.end(); it++ )
    //{
    //    SPackageItem &packag = (*it);
    //    const SItemData* itemData = CItem::GetItemData( packag.wIndex );
    //    if ( itemData == NULL )
    //        continue;

    //    if ( !ISTIMERECYCLE( itemData->byType ) )
    //        continue;

    //    STimeRecycle *timeRecycle = ( STimeRecycle * )( ( SItemBase* )&packag );

    //    if ( timeRecycle->recycleTiem == 0xffffffff )
    //        continue;

    //    __int64 r = timeRecycle->recycleTiem;
    //    __int64 last = r - now;

    //    if ( last <= 0 )
    //        delItem.push_back( &packag );
    //    else if ( tempTime > r )
    //        tempTime = r;
    //}
    //
    //recycleTime = tempTime;
    //for ( list<SPackageItem*>::iterator it = delItem.begin(); it != delItem.end(); it++ ) 
    //{
    //    const SItemData* itemData = CItem::GetItemData( (*it)->wIndex );
    //    if ( itemData == NULL )
    //        continue;

    //    if ( itemData->wOverlay != 0 ) 
    //    {
    //        g_Script.SetCondition( NULL, player, NULL );
    //        LuaFunctor( g_Script, FormatString( "OnTimeRecycle%d", itemData->wOverlay ) )[ lite::Variant( (*it), sizeof( SPackageItem ) ) ]();
    //        g_Script.CleanCondition();
    //    }

    //    _DelItem( *( *it ), "ϵͳ����" );

    //    if ( player )
    //        TalkToDnid( player->m_ClientIndex, FormatString( "��[%s]�ѱ�ϵͳ����!", itemData->szName ) );
    //}

    //delItem.clear();

    //return TRUE;
}

BOOL CItemUser::SetPreTime(SPackageItem *pItem, DWORD in_dwLocSrvPreTime, DWORD in_dwLocSrvBreakOdds)
{
	if (!pItem || 0 == in_dwLocSrvPreTime)
		return FALSE;

	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	if (pPlayer->m_InRidingevens)
	{
		TalkToDnid(pPlayer->m_ClientIndex,"����������״̬���޷�ʹ�õ���");
		return FALSE;
	}

	if (pPlayer->m_InUseItemevens)
	{
		TalkToDnid(pPlayer->m_ClientIndex,"�㵱ǰ״̬�޷�ʹ�õ���");
		return FALSE;
	}

	ReadyUseItemEvent *pEvent = new ReadyUseItemEvent(pPlayer, pItem, in_dwLocSrvPreTime, Event::EEventInterrupt_Combinate_CastItem);
	if(pEvent)
	{
		pEvent->SetEventName("UseItemEvent");
		pPlayer->m_BatchReadyEvent.SetReadyEvent(pEvent, in_dwLocSrvPreTime);
		pPlayer->m_InUseItemevens = 1;
	}
	LockItemCell(pItem->wCellPos,true);

	return TRUE;
}


BOOL CItemUser::CheckItemChange()
{
	/* For XYD3
	CPlayer *pp = ( CPlayer* )DynamicCast( IID_PLAYER );
	if ( pp == NULL )
		return FALSE;

	int iTempNum = (int)m_ItemList.size();

	time_t t;
	time(&t);
	SCellPos tempCellPosition;

	bool doDelete = FALSE;
	check_list<SPackageItem>::iterator it = m_ItemList.begin();
	check_list<SPackageItem>::iterator tempIterator = it;

	for ( int i = 0; i < iTempNum; i++ )
	{
		SPackageItem *pIB = &( *it );

		const SItemData *pData = CItem::GetItemData(pIB->wIndex);
		if (1 == pData->wItemChangeFlag)
		{
			SChangeItem *pSCItem = reinterpret_cast<SChangeItem*>(static_cast<SItemBase*>(pIB));
			if ( NULL != pIB )
			{
				tempCellPosition.byCellX = pIB->byCellX;
				tempCellPosition.byCellY = pIB->byCellY;

				if (DWORD(t) - pSCItem->dwCreateTime >  pData->dwItemChangePeriod)
				{
					//DelItem(pData->wItemID, NULL);

					if ( (TRUE == XYWorldCheckDelChangeItem2( *it, NULL )) && (TRUE == XYWorldCheckGenerateChangeItem2( pp, GenItemParams( pData->wChangedItemID2, 1, 5, 1 ))))
					{
						//10.3.10 jym
						doDelete = TRUE;
						it++;
						_XYWorldDelItem2( *tempIterator, NULL );

						if (IN_ODDS(pData->dwItemChangeOdds1))
						{
							GenerateChangeItem( pp, GenItemParams( pData->wChangedItemID2, 1, 5, 1 ),  tempCellPosition ,LogInfo( 100, "�ɱ���Ʒ����1�Ĵ�����" ));
						}
						else
						{
							GenerateChangeItem( pp, GenItemParams( pData->wChangedItemID1, 1, 5, 1 ),  tempCellPosition ,LogInfo( 100, "�ɱ���Ʒ����2�Ĵ�����" ));
						}
					}
					else
					{

					}
				}
			}
		}
		//10.3.10 jym
		if(doDelete)
			doDelete = FALSE;
		else
			it++;
		tempIterator = it;

	}//*/
	return 0;
}

// ������
BOOL CItemUser::RecvFixOneItem(struct SQEquipFixOne *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvFixOneItem() - !pMsg");
		return FALSE;
	}

	CPlayer* pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(pMsg->index);
	if (!itemData)
		return FALSE;

	if (!ITEM_IS_EQUIPMENT(itemData->m_Type))
	{
		TalkToDnid(pPlayer->m_ClientIndex, "ֻ��װ���ſ�������");
		return FALSE;
	}

	if (!itemData->m_MaxWear)
	{
		TalkToDnid(pPlayer->m_ClientIndex, "����ĥ���װ�����ܹ�����");
		return FALSE;
	}

	BYTE pos = pMsg->pos;
	SEquipment *pEquip	= GetEquipByPos(pMsg->type, pos);

	if (!pEquip)
		return FALSE;

	if (!pEquip->attribute.maxWear)
	{
		TalkToDnid(pPlayer->m_ClientIndex, "��ǰ����;ö�Ϊ0����������");
		return FALSE;
	}

	WORD needMoney = (static_cast<float>(itemData->m_BuyPrice) * 1.75 / static_cast<float>(itemData->m_MaxWear)) * (pEquip->attribute.maxWear - pEquip->attribute.currWear);
	if (!pPlayer->CheckPlayerMoney(XYD_UM_ONLYUNBIND, needMoney, false))
	{
		TalkToDnid(pPlayer->m_ClientIndex, "�޸�װ����������ҽ�Ǯ���㣡");
		return FALSE;
	}

	//if (pEquip->attribute.maxWear - pEquip->attribute.currWear < 100)
	//	return FALSE;

	pEquip->attribute.maxWear = static_cast<float>(pEquip->attribute.maxWear) * 0.95;

	if (!pEquip->attribute.maxWear)
	{
		TalkToDnid(pPlayer->m_ClientIndex, "�����Ժ�װ���ĵ�ǰ����;ö��Ѿ�Ϊ0���޷�ʹ�ã�");
		return FALSE;
	}

	pEquip->attribute.currWear = pEquip->attribute.maxWear;
	
	if (SQEquipFixOne::EEP_BAG == pMsg->type)
	{
		SAUpdateEquipWearInBagMsg msg;
		msg.wEqIndex			= pEquip->wIndex;
		msg.byPos				= pos;
		msg.wCurEqDuranceCur	= pEquip->attribute.currWear;
		msg.wMaxEqDuranceCur	= pEquip->attribute.maxWear;

		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}

	if (SQEquipFixOne::EFP_WEAR == pMsg->type)
	{
		SAUpdateCurEqDuranceMsg msg;
		msg.wEqIndex			= pEquip->wIndex;
		msg.byPos				= pos;
		msg.wCurEqDuranceCur	= pEquip->attribute.currWear;
		msg.wMaxEqDuranceCur	= pEquip->attribute.maxWear;

		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}

	// ֻ���;ö�Ϊ0ʱ����Ӱ�쵽��ҵ����ԣ����������Ϊ��0���������е�װ��
	if (!pPlayer->m_Property.m_Equip[pos].attribute.currWear)
	{
		pPlayer->InitEquipmentData();
		pPlayer->UpdateAllProperties();
	}

	return TRUE;
}

// ����ȫ��
BOOL CItemUser::RecvFixAllItem(struct SQEquipFixAll *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvFixAllItem() - !pMsg");
		return FALSE;
	}

	CPlayer* pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	WORD TotalMoney = 0;
	
	for (int i = EQUIP_P_WEAPON; i < EQUIP_P_MAX; ++i)
	{
		DWORD index = pPlayer->m_Property.m_Equip[i].wIndex;
		const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(index);

		if (!itemData || !itemData->m_MaxWear /*|| ((pPlayer->m_Property.m_Equip[i].attribute.maxWear - pPlayer->m_Property.m_Equip[i].attribute.currWear <= 100))*/)
			continue;

		// δ�޸�ǰ������;ö��Ƿ�Ϊ0
		if (!pPlayer->m_Property.m_Equip[i].attribute.maxWear)
		{
			TalkToDnid(pPlayer->m_ClientIndex, "����;ö�Ϊ0Ŷ�������޸���");
			continue;
		}

		// �жϽ�Ǯ
		WORD needMoney = (static_cast<float>(itemData->m_BuyPrice) * 1.75 / static_cast<float>(itemData->m_MaxWear)) * (pPlayer->m_Property.m_Equip[i].attribute.maxWear - pPlayer->m_Property.m_Equip[i].attribute.currWear);
		
		if (!pPlayer->CheckPlayerMoney(XYD_UM_ONLYUNBIND, needMoney, false))
		{
			TalkToDnid(pPlayer->m_ClientIndex, "ֻ����˲���װ�����޸����������Ҳ��㣬�޸���ֹ��");
			return FALSE;
		}
	
		// �޸�һ��Ҫ��������;ö�
		WORD WearValue = static_cast<float>(pPlayer->m_Property.m_Equip[i].attribute.maxWear) * 0.05;
		pPlayer->UpdateEquipWear(i, WearValue, true);

		if (!pPlayer->m_Property.m_Equip[i].attribute.maxWear)
		{
			TalkToDnid(pPlayer->m_ClientIndex, "����;ö��Ѿ�Ϊ0�����ܼ����޸�������ʹ�ã�");
			continue;
		}

		pPlayer->m_Property.m_Equip[i].attribute.currWear = pPlayer->m_Property.m_Equip[i].attribute.maxWear;
		
		SAUpdateCurEqDuranceMsg msg;
		msg.wEqIndex			= pPlayer->m_Property.m_Equip[i].wIndex;
		msg.byPos				= i;
		msg.wCurEqDuranceCur	= pPlayer->m_Property.m_Equip[i].attribute.currWear;
		msg.wMaxEqDuranceCur	= pPlayer->m_Property.m_Equip[i].attribute.maxWear;

		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	}

	return TRUE;
}

// װ���޸����ڲ�ʹ�ú���
SEquipment* CItemUser::GetEquipByPos(BYTE type, BYTE pos)
{
	SEquipment *pEquip = 0;
	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);

	switch(type)
	{
		// �������װ��
	case SQEquipFixOne::EEP_BAG:
		{
			SPackageItem *pIB = FindItemByPos(pos, XYD_FT_ONLYUNLOCK);
			if (!pIB)
				return 0;
			else
			{
				pEquip = reinterpret_cast<SEquipment *>(static_cast<SRawItemBuffer*>(pIB));
				if (!pEquip)
				{
					rfalse(4, 1, "Itemuser.cpp - GetEquipByPos() - !pEquip");
					return 0;
				}

				return pEquip;
			}
		}

	case SQEquipFixOne::EFP_WEAR:
		{
			if (pos < EQUIP_P_WEAPON || pos >= EQUIP_P_MAX)
			{
				rfalse("װ��λ�ô�����~~~");
				return 0;
			}

			for (int i = EQUIP_P_WEAPON; i < EQUIP_P_MAX; ++i)
			{
				if (pos == i)
				{
					pEquip = &(pPlayer->m_Property.m_Equip[i]);
					if (!pEquip)
					{
						rfalse(4, 1, "Itemuser.cpp - GetEquipByPos() - !pEquip");
						return 0;
					}
					return pEquip;
				}
			}
		}

	default:
		return 0;
	}
}

// =================װ��������������AHJ��==================
// ����װ����������ϻ����ϰ���װ��ǿ���Ĳ�����
// ���е�����Ҳ�ǰ���װ��ǿ������Ҫ��
// =================================================
// ��ȡһ��λ�� �ֲ����,�����ǰ�����������װ����
CItemUser::TItemPosType CItemUser::GetItemPosType(WORD wItemPos)
{
	
	TItemPosType ePosType = EError_PosType;
	if (wItemPos < PackageAllCells)
	{
		ePosType = EPackage_PosType;
	}
	else if (wItemPos >= sciEquipColumnBaseLine)
	{
		WORD wTmpMyEquipPos = wItemPos - sciEquipColumnBaseLine;
		if (wTmpMyEquipPos >= 0 && wTmpMyEquipPos < EQUIP_P_MAX)
		{
			ePosType = EEquipColumn_PosType;
		}
		else 
		{
			rfalse(2, 1, "��ȡ��Ʒλ�ó����ˡ� pos = %d", wItemPos);
			MY_ASSERT(0);
		}
	}
	else 
	{		
		rfalse(2, 1, "��ȡ��Ʒλ�ó����ˡ� pos = %d", wItemPos);		
		MY_ASSERT(0);
	}
	return ePosType;
}

// ��ȡ��������λ�ã�ע���Ƿ���Ҫ���װ������׼��
WORD CItemUser::GetEquipColumnPos(WORD wEquipPos, bool bAddEquipColumnBaseLine)
{
	WORD  wTmpPos = 0;
	if (bAddEquipColumnBaseLine)
	{
		wTmpPos = wEquipPos >= sciEquipColumnBaseLine? wEquipPos : wEquipPos + sciEquipColumnBaseLine;
	}
	else 
	{
		wTmpPos = wEquipPos >= sciEquipColumnBaseLine? wEquipPos-sciEquipColumnBaseLine : wEquipPos;
	}
	return wTmpPos;
}

// ���װ�����Ƿ�������
BOOL CItemUser::IsLockedEquipColumnCell(WORD wEquipPos)
{
	if (wEquipPos >= EQUIP_P_MAX)
	{
		return FALSE;
	}
	return (m_EquipedColumnFlag[wEquipPos].m_State & SItemWithFlag::IWF_LOCKED) != 0;
}

// �����������װ�����ĸ���
BOOL CItemUser::LockEquipColumnCell(WORD wEquipPos, bool lock, bool sync)
{
	if (wEquipPos >= EQUIP_P_MAX)
	{
		return FALSE;
	}
	if (lock)
	{
		m_EquipedColumnFlag[wEquipPos].m_State |= SItemWithFlag::IWF_LOCKED;
	}
	else
	{
		m_EquipedColumnFlag[wEquipPos].m_State &= (~SItemWithFlag::IWF_LOCKED);
	}
	
	if (sync)
	{
		CPlayer *pPlayer = (CPlayer *)this;
		if (!pPlayer)
			return FALSE;

		SALockItemCell lockMsg;
		lockMsg.bTyte	= SALockItemCell::SLC_EQUIPCOLUMN;		
		lockMsg.bLock	= lock;
		lockMsg.wPos	= wEquipPos;
		g_StoreMessage(pPlayer->m_ClientIndex, &lockMsg, sizeof(SALockItemCell));
	}	
	return TRUE;
}
// ͨ��λ�û�ȡװ��(�����ǴӰ����л�ȡ��Ҳ�����Ǵ�װ���������ȡ)
SEquipment * CItemUser::GetEquipmentbyItem(WORD wEquipPos, WORD LookType/* = XYD_FT_WHATEVER*/)
{
	SEquipment *pEquip = NULL;

	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return pEquip;

	TItemPosType ePosType = GetItemPosType(wEquipPos);
	if (ePosType == EPackage_PosType)
	{
		// [2012-9-13 19-34 gw: -��̫�ÿ����Ƿ�����]
		//if (!pPlayer->IsItemPosValid(byItemPosX, byItemPosY)) 
		//	return pEquip;
		SPackageItem *srcItem = pPlayer->FindItemByPos(wEquipPos, LookType);
		if (!srcItem) 
			return pEquip;

		const SItemBaseData *srcData = CItemService::GetInstance().GetItemBaseData(srcItem->wIndex);
		if (!srcData) 
			return pEquip;
		if (srcData->m_Color == IC_WHITE)
		{
			return pEquip;
		}
		if (!ITEM_IS_EQUIPMENT(srcData->m_Type) || !pPlayer->Equip_Is_Tyler(srcItem->wIndex))
		{
			return pEquip;
		}

		pEquip = (SEquipment *)(SItemBase *)srcItem;
	}
	else if (ePosType == EEquipColumn_PosType)
	{
		WORD wTmpMyEquipPos = wEquipPos - sciEquipColumnBaseLine;
		if (wTmpMyEquipPos >= 0 && wTmpMyEquipPos < EQUIP_P_MAX)
		{
			if (pPlayer->m_Property.m_Equip[wTmpMyEquipPos].wIndex != 0)
			{
				pEquip = &pPlayer->m_Property.m_Equip[wTmpMyEquipPos];
			}
		}
	}
	return pEquip;
}

// ͨ��ͨ��λ�ü�Ⲣ�Ҹ���װ����������
void CItemUser::CheckAndUpdateEquipColumnAttribyPos(CPlayer *pPlayer, WORD wEquipPos)
{
	if (!pPlayer)
	{
		return ;
	}
	if (EEquipColumn_PosType != GetItemPosType(wEquipPos))
	{
		return ;
	}
	// ���¼���װ���ӳ�����
	pPlayer->InitEquipmentData();
	pPlayer->UpdateAllProperties();
}

// ����װ��
BOOL CItemUser::RecvDragStrengthenEquip(struct SQAddEquip* pMsg)
{
	BOOL bRet = FALSE;
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvDragStrengthenEquip() - !pMsg");
		return bRet;
	}

	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return bRet;

	if (pPlayer->m_vecStrengthenEquip.size() > 0)
	{
		pPlayer->ClearMetrialRelation();
		pPlayer->m_refineType = 0;		
	}	
	
	// �����쳣���	
	if (pMsg->byPosNum == 0 || pMsg->byPosNum + pMsg->byAssistPosNum > SQAddEquip::msc_wMaxLen)
	{
		return bRet;
	}
	switch (pMsg->bType)
	{
	case SQAddEquip::ADE_NONE:
		return bRet;
	case SQAddEquip::ADE_EQUIP_DECOMPOSITION:
		if (pMsg->byPosNum > SQAddEquip::msc_byBatchDecomposition_Max)
		{
			return bRet;
		}
		break;
	case SQAddEquip::ADE_EQUIP_SMELTING:
		if (pMsg->byPosNum > SQAddEquip::msc_byBatchSmelting_Max)
		{
			return bRet;
		}
		break;		
	}
	if (pPlayer->m_refineType != SQAddEquip::ADE_NONE && pPlayer->m_refineType != pMsg->bType)
	{
		return bRet;
	}

	SAAddEquip pSaddEquipMsg;
	int iSuccessNum = 0; // �ɹ�����������Ʒ�ĸ���
	for (int i = 0; i < pMsg->byPosNum; ++i)
	{
		const SEquipment *pEquip = GetEquipmentbyItem(pMsg->waPos[i], XYD_FT_ONLYUNLOCK);
		if (CEquipStrengthenServer::handleElemDragStrengthenEquip(pPlayer, pEquip, pMsg->bType))
		{			
			pPlayer->m_vecStrengthenEquip.push_back(pMsg->waPos[i]);	

			bool Flag1 = false;
			// ��ʾ��һ�ȼ�
			BOOL	bDisplayNextLevel = TRUE;
			BYTE type = -1;
			switch(pMsg->bType)
			{
			case SQAddEquip::ADE_REFINE_GRADE:
				type = SAAfterUpgradeEquipMsg::UPDATE_GRADE;
				break;

			case SQAddEquip::ADE_REFINE_STAR:
				type = SAAfterUpgradeEquipMsg::UPDATE_STAR;
				break;
			case SQAddEquip::ADE_ReMove_STAR:
				type = SAAfterUpgradeEquipMsg::UPDATE_ReMove;
				break;
			case SQAddEquip::ADE_UPDATE_LEVEL:
				type = SAAfterUpgradeEquipMsg::UPDATE_LEVEL;
				break;
			case SQAddEquip::ADE_UPDATE_QUALITY:
				type = SAAfterUpgradeEquipMsg::UPDATE_QUALITY;
				break;
			case SQAddEquip::ADE_EQUIP_RESET:	// װ��ϴ��(���ø�����������)	
				type = SAAfterUpgradeEquipMsg::EQUIP_RESET;
				break;
			case SQAddEquip::ADE_EQUIP_REFINE: // װ������(��������������ֵ)	
				type = SAAfterUpgradeEquipMsg::EQUIP_REFINE;
				break;
			case SQAddEquip::ADE_GEM_INSERT:
			case SQAddEquip::ADE_EQUIP_DECOMPOSITION:
			case SQAddEquip::ADE_EQUIP_SMELTING:
				bDisplayNextLevel = FALSE;
				break;
			case SQAddEquip::ADE_EQUIP_SPIRITATTACHBODY:
				bDisplayNextLevel = FALSE;
				if (pMsg->byAssistPosNum > i && pMsg->byPosNum + i < SQAddEquip::msc_wMaxLen)
				{
					int	iArrayIndex = pMsg->byPosNum + i;
					pPlayer->m_vecStrengthenEquipAssist.push_back(pMsg->waPos[iArrayIndex]);
					pPlayer->LockItemCell(pMsg->waPos[iArrayIndex], true);
				}
				break;
			case SQAddEquip::ADE_ADD_HOLE:
				type = SAAfterUpgradeEquipMsg::ADD_HOLE;
				break;		
			case SQAddEquip::ADE_GEM_REMOVE:
				type = SAAfterUpgradeEquipMsg::GEM_REMOVE;
				break;		
			default:
				return FALSE;
			}
			if (bDisplayNextLevel)
			{
				Flag1 = pPlayer->SendNextLevelEquip(type);
				if (!Flag1)
				{
					pSaddEquipMsg.Flag = SAAddEquip::ADE_GETNEXTEquip_Error;
					g_StoreMessage(pPlayer->m_ClientIndex,&pSaddEquipMsg,sizeof(SAAddEquip));
					pPlayer->ClearMetrialRelation();
					return FALSE;
				}
			}

			pPlayer->m_refineType		= pMsg->bType;
			pPlayer->LockItemCell(pMsg->waPos[i], true);
			++iSuccessNum;
		}
	}	
	
	if (iSuccessNum > 0)
	{		
		// ��������ͳһ����		
		if (!pPlayer->SendNeedMETRIALInfo(pPlayer, pMsg->bType))
		{
			pSaddEquipMsg.Flag = SAAddEquip::ADE_SendNEEDITEM_Info_Error;			
			pPlayer->ClearMetrialRelation();
			bRet = FALSE;
		}	
		else 
		{
			pSaddEquipMsg.Flag = SAAddEquip::ADE_Success;
			bRet = TRUE;			
		}
	}
	else 
	{
		pPlayer->ClearMetrialRelation();
	}
	g_StoreMessage(pPlayer->m_ClientIndex,&pSaddEquipMsg,sizeof(SAAddEquip));

	return bRet;
}
// �������
BOOL CItemUser::RecvDragStrengthenMetrial(struct SQAddMetrial *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemus.cpp - BOOL CItemUser::RecvForgeItem(struct SQStartForge *pMsg) - !pMsg");
		return FALSE;
	}
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;
	SAAddMetrial msg;
	// �����쳣~
	if (SQAddMetrial::ADM_RAW_METRIAL != pMsg->bType)
	{
		msg.Result = SAAddMetrial::ADD_INLAY_NoGEM;
		g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAAddMetrial));
		return FALSE;
	}

	if (0 == player->m_vecStrengthenEquip.size())
	{
		return FALSE;
	}	
	// ���֮ǰ�ϵ�װ���Ƿ����
	const SEquipment* pEquip = GetEquipmentbyItem(player->m_vecStrengthenEquip.at(0), XYD_FT_WHATEVER);
	if (!pEquip)
	{
		msg.Result = SAAddMetrial::ADD_INLAY_EQUIP;
		g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAAddMetrial));
		return FALSE;
	}
	const SItemBaseData *EquipData = CItemService::GetInstance().GetItemBaseData(pEquip->wIndex);
	if (!EquipData)
	{
		msg.Result = SAAddMetrial::ADD_INLAY_EQUIP;
		g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAAddMetrial));
		return FALSE;
	}


	LockBatchCells(player->m_vecStrengthenMetrial, false);	
	player->m_vecStrengthenMetrial.clear();

	WORD	wTmpPos = 0;
	int		iMetrialCellNum = pMsg->byPosNum>SQDragMetrial::msc_wMaxLen?SQDragMetrial::msc_wMaxLen:pMsg->byPosNum;
	for (int i = 0; i < iMetrialCellNum; ++i)
	{
		{
			wTmpPos = pMsg->waPos[i];
			// ������ϵĲ��������Ƿ����
			if (!IsItemPosValid(pMsg->waPos[i]))
			{
				msg.Result = SAAddMetrial::ADD_INLAY_XY;
				g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAAddMetrial));
				return FALSE;
			}
			SPackageItem *srcItem = FindItemByPos(pMsg->waPos[i], XYD_FT_ONLYUNLOCK);
			if (!srcItem)
			{
				msg.Result = SAAddMetrial::ADD_INLAY_XY_LOCK;
				g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAAddMetrial));
				return FALSE;
			}
			const SItemBaseData *srcData = CItemService::GetInstance().GetItemBaseData(srcItem->wIndex);
			if (!srcData)
			{
				msg.Result = SAAddMetrial::ADD_NoFindGEM;
				g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAAddMetrial));
				return FALSE;
			}

			// �������Ƿ���ȷ todo..
		}

		player->m_vecStrengthenMetrial.push_back(pMsg->waPos[i]);
	}

	player->m_forgeType	= pMsg->bType;	
	msg.Result = SAAddMetrial::ADD_SUCCESS;
	msg.wCellPos = wTmpPos;	
	g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SAAddMetrial));	
	player->LockBatchCells(player->m_vecStrengthenMetrial, true);
	
	return TRUE;
}
// װ������
BOOL CItemUser::RecvBeginRefineGrade(struct SQBeginRefineGrade *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "RecvBeginRefineGrade() - !pMsg");
		return FALSE;
	}
	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	byte bySuccessNum = 0;	
	for (int i = 0; i < pPlayer->m_vecStrengthenEquip.size(); ++i)
	{
		WORD wEquipPos = pPlayer->m_vecStrengthenEquip.at(i);
		SEquipment* pEquip = GetEquipmentbyItem(wEquipPos, XYD_FT_ONLYLOCK);		
		if (CEquipStrengthenServer::handleElemEquipGrade(pPlayer, pEquip, 0, pMsg->byStrengthenExternChoose))
		{
			CheckAndUpdateEquipColumnAttribyPos(pPlayer, wEquipPos);
			++bySuccessNum;
		}
	}
		
	if (bySuccessNum > 0)
	{
		//SABeginRefineGrade msg;
		//msg.result = SABeginRefineGrade::BRG_SUCCESS;
		//g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginRefineGrade));
		return TRUE;
	}
	return FALSE;
}

// װ������--����
BOOL CItemUser::RecvBeginRefineStar(struct SQBeginRefineStar *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvBeginRefineStar() - !pMsg");
		return FALSE;
	}

	CPlayer* pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnRecvBeginRefineStar"))
	{
		g_Script.PushParameter(pMsg->bType);
		g_Script.PushParameter(pMsg->bIndex);
		g_Script.PushParameter(pMsg->bCount);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
	return TRUE;
}

// ��Ʒ��
BOOL CItemUser::RecvBeginUpgradeQuality(struct SQStartQuality *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvBeginUpgradeQuality() - !pMsg");
		return FALSE;
	}

	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	g_Script.SetPlayer(pPlayer);
	
	if (g_Script.PrepareFunction("OnBeginUpgradeQuality"))
	{
		g_Script.PushParameter(pMsg->bType);
		g_Script.PushParameter(pMsg->bIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
	
	return TRUE;
}

// ����,װ��ǿ��
BOOL CItemUser::RecvBeginUpgradeLevel(struct SQBeginEquipLevelUpMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvBeginUpgradeLevel() - !pMsg");
		return FALSE;
	}
	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnBeginIntensify"))
	{
		g_Script.PushParameter(pMsg->bType);
		g_Script.PushParameter(pMsg->bIndex);
		g_Script.PushParameter(pMsg->bCount);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();

	return TRUE;
}

// װ���ֽ�
BOOL CItemUser::RecvBeginDecomposition(struct SQBeginIdentifyMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvBeginDecomposition() - !pMsg");
		return FALSE;
	}

	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	BYTE bySuccessNum = 0;	
	for (int i = 0; i < pPlayer->m_vecStrengthenEquip.size(); ++i)
	{
		WORD wEquipPos = pPlayer->m_vecStrengthenEquip.at(i);
		const SEquipment *pEquip = (const SEquipment*)GetEquipmentbyItem(wEquipPos, XYD_FT_ONLYLOCK);
		if (CEquipStrengthenServer::handleElemEquipDecomposition(pPlayer, pEquip, pMsg->type, wEquipPos))
		{
			//CheckAndUpdateEquipColumnAttribyPos(pPlayer, wEquipPos);
			++bySuccessNum;
		}		
	}
	if (bySuccessNum > 0)
	{
		SABeginIdentifyMsg msg;
		msg.type = SQBeginIdentifyMsg::QBIM_FENJIE;
		pPlayer->ClearMetrialRelation(); 
		pPlayer->m_refineType = 0;
		msg.result = SABeginIdentifyMsg::BIM_SUCESS;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));		
	}

	return TRUE;
}
// װ������
BOOL CItemUser::RecvBeginSmelting(struct SQBeginIdentifyMsg* pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvBeginDecomposition() - !pMsg");
		return FALSE;
	}

	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	BYTE bySuccessNum = 0;	
	for (int i = 0; i < pPlayer->m_vecStrengthenEquip.size(); ++i)
	{
		WORD wEquipPos = pPlayer->m_vecStrengthenEquip.at(i);
		const SEquipment *pEquip = (const SEquipment*)GetEquipmentbyItem(wEquipPos, XYD_FT_ONLYLOCK);
		if (CEquipStrengthenServer::handleElemEquipSmelting(pPlayer, pEquip, pMsg->type, wEquipPos))
		{
			//CheckAndUpdateEquipColumnAttribyPos(pPlayer, wEquipPos);
			++bySuccessNum;
		}		
	}
	if (bySuccessNum > 0)
	{
		SABeginIdentifyMsg msg;
		msg.type = SQBeginIdentifyMsg::QBIM_SMELTING;
		pPlayer->ClearMetrialRelation(); 
		pPlayer->m_refineType = 0;
		msg.result = SABeginIdentifyMsg::BIM_SUCESS;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));		
	}	
	
	return TRUE;
}

// װ�����ף������������Ե����ޣ�
BOOL CItemUser::RecvBeginShengJie(struct SQBeginIdentifyMsg* pMsg)
{


	
	return TRUE;
}

// װ���鸽
BOOL CItemUser::RecvBeginSpiritAttachBody(struct SQBeginIdentifyMsg* pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvBeginDecomposition() - !pMsg");
		return FALSE;
	}

	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	BYTE bySuccessNum = 0;	
	SABeginIdentifyMsg msg;

	for (int i = 0; i < pPlayer->m_vecStrengthenEquip.size(); ++i)
	{		
		WORD wEquipPos = pPlayer->m_vecStrengthenEquip.at(i);
		SEquipment *pEquip = (SEquipment*)GetEquipmentbyItem(wEquipPos, XYD_FT_ONLYLOCK);
		
		if (pPlayer->m_vecStrengthenEquipAssist.size() <= i)
		{
			continue;
		}
		// ��ȡ����		
		//ͨ����������ʱ���
		WORD wMetrialPos =  pPlayer->m_vecStrengthenEquipAssist.at(i);		
		if (CEquipStrengthenServer::handleElemEquipSpiritAttachBody(pPlayer, pEquip, pMsg->type, wEquipPos, wMetrialPos))
		{
			CheckAndUpdateEquipColumnAttribyPos(pPlayer, wEquipPos);
			++bySuccessNum;
		}		
	}
	if (bySuccessNum > 0)
	{		
		// �ɹ��󣬽������ᣬ��վ����б�
		pPlayer->LockBatchCells(pPlayer->m_vecStrengthenEquipAssist, false, true);
		pPlayer->m_vecStrengthenEquipAssist.clear();

		msg.type = SQBeginIdentifyMsg::QBIM_LINGFU;
		//pPlayer->ClearMetrialRelation(); 
		pPlayer->m_refineType = 0;
		msg.result = SABeginIdentifyMsg::BIM_SUCESS;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));		
	}	

	return TRUE;
}

// ����ǿ��
BOOL CItemUser::RecvEndRefine(struct SQEndRefineMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvEndRefine() - !pMsg");
		return FALSE;
	}

	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	SAEndRefineMsg msgRet;
	msgRet.bType = pMsg->bType;
	msgRet.bPosX = pMsg->bPosX;
	msgRet.bPosY = pMsg->bPosY;

	bool EquipOff	= false;
	bool MetrailOff = false;
	bool ProtectMetrialOff = false;
	bool AddRateMetrialOff = false;
	if (player->m_refineType >= SQAddEquip::ADE_Max || player->m_refineType <= SQAddEquip::ADE_NONE) 
	{
		msgRet.bResult = SAEndRefineMsg::ERM_FAIL;
		g_StoreMessage(player->m_ClientIndex, &msgRet, sizeof(msgRet));	
		return FALSE;
	}
	switch (pMsg->bType)
	{		
	case SQEndRefineMsg::ERM_EQUIP_OFF:// ȡ��װ��
	case SQEndRefineMsg::ERM_CLOSE:// �ر����
		{
			player->ClearMetrialRelation();							
			player->m_refineType = 0;
			if (player->m_forgeType== SQAddMetrial::ADM_RAW_METRIAL && player->m_forgeMetrial)
			{
				LockItemCell(player->m_forgeMetrial->wCellPos,false);
				player->m_forgeMetrial = 0;
				player->m_forgeType	= 0;	
			}
		}
		break;
// 	case SQEndRefineMsg::ERM_EQUIP_OFF:// ȡ��װ��
// 		{

// 			if (player->m_forgeType== SQAddMetrial::ADM_RAW_METRIAL && player->m_forgeMetrial)
// 			{
// 				LockItemCell(player->m_forgeMetrial->byCellX,player->m_forgeMetrial->byCellY,false);
// 				player->m_forgeMetrial = 0;
// 				player->m_forgeType	= 0;	
// 			}
// 		}
// 		break;
	case SQEndRefineMsg::ERM_METRAIL_OFF:// ȡ��ԭʼ����
		{
			if (player->m_forgeType== SQAddMetrial::ADM_RAW_METRIAL && player->m_forgeMetrial)
			{
				LockItemCell(player->m_forgeMetrial->wCellPos,false);
				player->m_forgeMetrial = 0;
				player->m_forgeType	= 0;	
			}
		}
		break;
	default:
		msgRet.bResult = SAEndRefineMsg::ERM_FAIL;
		g_StoreMessage(player->m_ClientIndex, &msgRet, sizeof(msgRet));	
		return FALSE;
		break;
	}

	SAFE_DELETE(player->m_pIdentifyEquipBackUpNeedDel);
	msgRet.bResult = SAEndRefineMsg::ERM_SUCCESS;
	g_StoreMessage(player->m_ClientIndex, &msgRet, sizeof(msgRet));	

	return TRUE;
}



BOOL CItemUser::GetUpgradeResult(SEquipment *pE, BYTE &result, bool bHaveLuckyCharmItem)
{
	BOOL bRet = FALSE;
	if (!pE)
	{
		return bRet;
	}
	MY_ASSERT(pE);

	CPlayer* player = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!player)
		return bRet;
	SABeginRefineGrade msg;
	// ��ȡ�������ñ�
	const SItemUpdateGradeInfo *pData = CItemService::GetInstance().GetUpgradeInfo(pE->attribute.grade);
	if (!pData)
		return bRet;

	BYTE beforeGrade = pE->attribute.grade;

	WORD totalSuccessRate	= pData->baseSuccessRate + (bHaveLuckyCharmItem?pData->wLuckyCharmAddSuccessRate:0);
	WORD totalFailRate		= pData->baseFailedReduceGradeRate;
	WORD totalDestroyRate	= pData->baseFailedDestroyRate;

	WORD wRate = CRandom::RandRange(1, 10000);

	if (totalSuccessRate > 10000)
	{
		totalSuccessRate = 10000;
		return bRet;
	}

	if (wRate <= totalSuccessRate)
	{
		bRet = TRUE;
		result = SABeginRefineGrade::BRG_SUCCESS;
		msg.result = result;
		if (pE->attribute.grade >= SEquipment::MAX_GRADE_NUM)
		{
			return bRet;
		}		

		++pE->attribute.grade;
		OnthSysNotice(6);		
	}
	else
	{
		bRet = FALSE;
		result = SABeginRefineGrade::BRG_FAIL;
		msg.result = result;			
		BYTE reduceGrade = 0;

		// ʧ�ܺ�Ĵ�������
		WORD rate = CRandom::RandRange(1, 100);
		if (rate < totalFailRate)
		{
			reduceGrade = 1;
		}
		if (reduceGrade > 0)
		{
			pE->attribute.grade = (pE->attribute.grade <= reduceGrade) ? 1 : (pE->attribute.grade - reduceGrade);
		}
		OnthSysNotice(7);
	}	

	if (player->m_vecStrengthenEquip.size() > 0)
	{
		// �˴�������׺�ĸ�������ֵ
		AddRefineAttribute(pE);
		// ������ʾ��Ϣ����
		SendUpdateEquipInfo(pE);
		// ��ʾ��һ�ȼ�
		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SABeginRefineGrade));
		if (pE->attribute.grade == SEquipment::MAX_GRADE_NUM )
		{
			RemoveRefineMetrial();
			OnthSysNotice(8);
		}
		else
		{
			SendNextLevelEquip(SAAfterUpgradeEquipMsg::UPDATE_GRADE);
			// ���ݼ������ۼ�����			
			SendNeedMETRIALInfo(player,SQAddEquip::ADE_REFINE_GRADE);
		}
		// ����߽�ʱ��������в���
		
	}

	return bRet;
}

// ���������������ʾ��һ�ȼ�װ������Ϣ
BOOL CItemUser::SendNextLevelEquip(BYTE type)
{	
	return SendRefineEquipTheBestPreview(type);
// 	CPlayer* player = (CPlayer*)DynamicCast(IID_PLAYER);
// 	if (!player)
// 		return FALSE;
// 
// 	if (0 == player->m_vecStrengthenEquip.size())
// 	{
// 		return FALSE;
// 	}	
// 	const SEquipment* pEquip = GetEquipmentbyItem(player->m_vecStrengthenEquip.at(0), XYD_FT_WHATEVER);
// 	if (!pEquip)
// 	{
// 		rfalse(4, 1, "Itemuser.cpp - SendNextLevelEquip() - !pEquip");
// 		return FALSE;
// 	}
// 	
// 	/*const SItemFactorData *pItemFactor = CItemService::GetInstance().GetItemFactorData(pEquip->wIndex);
// 	if (!pItemFactor)
// 		return FALSE;*/
// 
// 	SAAfterUpgradeEquipMsg msg;
// 
// 	msg.type = type;
// 
// 	switch (type)
// 	{
// 		case SAAfterUpgradeEquipMsg::UPDATE_GRADE:
// 			{
// 				memcpy(&msg.equip, pEquip, sizeof(SEquipment));
// 				++msg.equip.attribute.grade;
// 				AddRefineAttribute(&msg.equip);
// 			}
// 			break;
// 
// 		case SAAfterUpgradeEquipMsg::UPDATE_LEVEL:
// 			{
// 				memcpy(&msg.equip, pEquip, sizeof(SEquipment));
// 				const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(msg.equip.wIndex);
// 				if (75 == pData->m_Level)
// 					return FALSE;
// 				int rs = 0;
// 				lite::Variant ret;
// 				LuaFunctor(g_Script,"GetNextUpdateLevelItemID")[pEquip->wIndex](&ret);
// 				if (lite::Variant::VT_INTEGER == ret.dataType)
// 				{
// 					rs = ret;
// 					msg.equip.wIndex = rs;
// 					AddMaxExtraAttri(&msg.equip);
// 				}
// 				/*const SEquipLevelUp *pLevelData = CItemService::GetInstance().GetLevelUpData(pData->m_Level);
// 				if (!pLevelData || (pLevelData && (!pLevelData->StoneNum || !pLevelData->NeedMoney || !pLevelData->SuccessRate)))
// 					return FALSE;*/
// 				// �����󸽼��������
// 			}
// 			break;
// 
// 		case SAAfterUpgradeEquipMsg::UPDATE_QUALITY:
// 			{
// 				memcpy(&msg.equip, pEquip, sizeof(SEquipment));
// 				//msg.equip.wIndex = pEquip->wIndex + 1000;
// 
// 				const SItemBaseData* pData = CItemService::GetInstance().GetItemBaseData(msg.equip.wIndex);
// 				if (!pData)
// 					return FALSE;
// 				int rs = 0;
// 				lite::Variant ret;
// 				LuaFunctor(g_Script,"GetNextUpdateQualityItemID")[pEquip->wIndex](&ret);
// 				if (lite::Variant::VT_INTEGER == ret.dataType)
// 				{
// 					rs = ret;
// 					msg.equip.wIndex = rs;
// 					AddMaxExtraAttri(&msg.equip);
// 				}
// 				// �����󸽼��������
// 			}
// 			break;
// 
// 		case SAAfterUpgradeEquipMsg::UPDATE_STAR:
// 			{
// 				BOOL Flag = FALSE;
// 				memcpy(&msg.equip, pEquip, sizeof(SEquipment));
// 				for (size_t index = 0; index < SEquipment::MAX_STAR_NUM; ++index)
// 				{
// 					if (SEquipment::SLOT_IS_EMPTY == msg.equip.attribute.starNum[index])
// 					{
// 						AddRefineAttribute(&msg.equip);
// 						msg.equip.attribute.starNum[index] = SEquipment::SLOT_IS_VALID;
// 						//Flag = TRUE;
// 						break;
// 					}
// 				}
// 				/*if (Flag)
// 				{
// 					return FALSE;
// 				}*/
// 			}
// 			break;
// 		case SAAfterUpgradeEquipMsg::UPDATE_ReMove:
// 			{
// 				memcpy(&msg.equip, pEquip, sizeof(SEquipment));
// 				memset(&msg.equip.attribute.starAttri,0,sizeof(SEquipment::ExtraAttri) * SEquipment::MAX_GRADE_ATTRI);
// 				memset(&msg.equip.attribute.starNum,0,sizeof(BYTE) * SEquipment::MAX_STAR_NUM);
// 			}
// 			break;
// 		case SAAfterUpgradeEquipMsg::ADD_HOLE:
// 			{
// 				memcpy(&msg.equip, pEquip, sizeof(SEquipment));
// 				int index = 0;
// 				for (; index < SEquipment::MAX_SLOTS; ++index)
// 				{
// 					if (msg.equip.slots[index].isInvalid())
// 					{
// 						break;
// 					}
// 				}
// 				msg.equip.slots[index].type = 0;
// 				msg.equip.slots[index].value = 1;
// 			}
// 			break;
// 		case SAAfterUpgradeEquipMsg::GEM_INSERT:
// 			{
// 				memcpy(&msg.equip, pEquip, sizeof(SEquipment));
// 				int index = 0;
// 				bool Flag = false;
// 				if (!player->m_forgeMetrial)
// 				{
// 					return FALSE;
// 				}
// 				//pEquip->slots[i].type  = (player->m_forgeMetrial->wIndex - 4004001) / 100 + 1;
// 				//pEquip->slots[i].value = player->m_forgeMetrial->wIndex - (pEquip->slots[i].type - 1) * 100 - 4004001 + 1;
// 				for (; index < SEquipment::MAX_SLOTS; ++index)
// 				{
// 					if (msg.equip.slots[index].isEmpty())
// 					{
// 						Flag = true;
// 						msg.equip.slots[index].type   = (player->m_forgeMetrial->wIndex - 4004001) / 100 + 1;
// 						msg.equip.slots[index].value  = player->m_forgeMetrial->wIndex - (msg.equip.slots[index].type - 1) * 100 - 4004001 + 1;
// 						break;
// 					}
// 				}
// 			}
// 			break;
// 		case SAAfterUpgradeEquipMsg::GEM_REMOVE:
// 			{
// 				memcpy(&msg.equip, pEquip, sizeof(SEquipment));
// 				int index = SEquipment::MAX_SLOTS - 1;
// 				for (; index >= 0; --index)
// 				{
// 					if (msg.equip.slots[index].isJewel())
// 					{
// 						msg.equip.slots[index].type = 0;
// 						msg.equip.slots[index].value = 1;
// 						break;
// 					}
// 				}
// 				
// 			}
// 			break;
// 		default:
// 			break;
// 	}
// 
// 	//if (SAAfterUpgradeEquipMsg::UPDATE_QUALITY == type || SAAfterUpgradeEquipMsg::UPDATE_LEVEL == type)
// 	//{
// 	//	SRawItemBuffer item;
// 	//	GenerateNewItem(item, SAddItemInfo(msg.equip.wIndex, 1));
// 
// 	//	SEquipment *equip = (SEquipment*)((SItemBase*)&item);
// 	//	memcpy(&msg.equip, equip, sizeof(SEquipment));
// 	//}
// 
// 	g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SAAfterUpgradeEquipMsg));
// 
// 	return TRUE;
}

// [���칦���﷢�ͼ�ƷԤ��]
BOOL CItemUser::SendRefineEquipTheBestPreview(BYTE byType)
{
	CPlayer* player = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	if (0 == player->m_vecStrengthenEquip.size())
	{
		return FALSE;
	}		
	const SEquipment* pEquip = GetEquipmentbyItem(player->m_vecStrengthenEquip.at(0), XYD_FT_WHATEVER);
	if (!pEquip)
	{
		rfalse(4, 1, "Itemuser.cpp - SendNextLevelEquip() - !pEquip");
		return FALSE;
	}
	
	/*const SItemFactorData *pItemFactor = CItemService::GetInstance().GetItemFactorData(pEquip->wIndex);
	if (!pItemFactor)
		return FALSE;*/

	SAAfterUpgradeEquipMsg msg;

	msg.type = byType;

	switch (byType)
	{
		case SAAfterUpgradeEquipMsg::UPDATE_STAR: // ǿ��
			{
				BOOL Flag = FALSE;
				memcpy(&msg.equip, pEquip, sizeof(SEquipment));
				for (size_t index = 0; index < SEquipment::MAX_STAR_NUM; ++index)
				{
//					msg.equip.attribute.starNum[index] = SEquipment::SLOT_IS_VALID;					
					if (index >= SEquipment::MAX_STAR_NUM-2)
					{
						AddRefineAttribute(&msg.equip);				
					}
				}
				AddRefineAttribute(&msg.equip);				
			}
			break;
		
		case SAAfterUpgradeEquipMsg::EQUIP_RESET: // ϴ��			
		case SAAfterUpgradeEquipMsg::EQUIP_REFINE: // ����
		case SAAfterUpgradeEquipMsg::UPDATE_GRADE: // ����
			break;

		case SAAfterUpgradeEquipMsg::UPDATE_LEVEL:
			{
				memcpy(&msg.equip, pEquip, sizeof(SEquipment));
				const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(msg.equip.wIndex);
				if (75 == pData->m_Level)
					return FALSE;
				int rs = 0;
				lite::Variant ret;
				LuaFunctor(g_Script,"GetNextUpdateLevelItemID")[pEquip->wIndex](&ret);
				if (lite::Variant::VT_INTEGER == ret.dataType)
				{
					rs = ret;
					msg.equip.wIndex = rs;
					AddMaxExtraAttri(&msg.equip);
				}				
			}
			break;

		case SAAfterUpgradeEquipMsg::UPDATE_QUALITY:
			{
				memcpy(&msg.equip, pEquip, sizeof(SEquipment));
				const SItemBaseData* pData = CItemService::GetInstance().GetItemBaseData(msg.equip.wIndex);
				if (!pData)
					return FALSE;
				int rs = 0;
				lite::Variant ret;
				LuaFunctor(g_Script,"GetNextUpdateQualityItemID")[pEquip->wIndex](&ret);
				if (lite::Variant::VT_INTEGER == ret.dataType)
				{
					rs = ret;
					msg.equip.wIndex = rs;
					AddMaxExtraAttri(&msg.equip);
				}
				// �����󸽼��������
			}
			break;


		case SAAfterUpgradeEquipMsg::UPDATE_ReMove:
			{
				memcpy(&msg.equip, pEquip, sizeof(SEquipment));
				memset(&msg.equip.attribute.starAttri,0,sizeof(SEquipment::ExtraAttri) * SEquipment::MAX_GRADE_ATTRI);
//				memset(&msg.equip.attribute.starNum,0,sizeof(BYTE) * SEquipment::MAX_STAR_NUM);
			}
			break;
		case SAAfterUpgradeEquipMsg::ADD_HOLE:
			{
				memcpy(&msg.equip, pEquip, sizeof(SEquipment));
				int index = 0;
				for (; index < SEquipment::MAX_SLOTS; ++index)
				{
					//if (msg.equip.slots[index].isInvalid())
					//{
					//	break;
					//}
					msg.equip.slots[index].type = 0;
					msg.equip.slots[index].value = 1;
				}
				//msg.equip.slots[index].type = 0;
				//msg.equip.slots[index].value = 1;
			}
			break;
		case SAAfterUpgradeEquipMsg::GEM_INSERT:
			{
				memcpy(&msg.equip, pEquip, sizeof(SEquipment));
				int index = 0;
				bool Flag = false;
				if (!player->m_forgeMetrial)
				{
					return FALSE;
				}
				//pEquip->slots[i].type  = (player->m_forgeMetrial->wIndex - 4004001) / 100 + 1;
				//pEquip->slots[i].value = player->m_forgeMetrial->wIndex - (pEquip->slots[i].type - 1) * 100 - 4004001 + 1;
				for (; index < SEquipment::MAX_SLOTS; ++index)
				{
					if (msg.equip.slots[index].isEmpty())
					{
						Flag = true;
						msg.equip.slots[index].type   = (player->m_forgeMetrial->wIndex - 4004001) / 100 + 1;
						msg.equip.slots[index].value  = player->m_forgeMetrial->wIndex - (msg.equip.slots[index].type - 1) * 100 - 4004001 + 1;
						break;
					}
				}
			}
			break;
		case SAAfterUpgradeEquipMsg::GEM_REMOVE:
			{
				memcpy(&msg.equip, pEquip, sizeof(SEquipment));
				int index = SEquipment::MAX_SLOTS - 1;
				for (; index >= 0; --index)
				{
					if (msg.equip.slots[index].isJewel())
					{
						msg.equip.slots[index].type = 0;
						msg.equip.slots[index].value = 1;
						break;
					}
				}
				
			}
			break;
		default:
			break;
	}
	g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SAAfterUpgradeEquipMsg));

	return TRUE;
}

BOOL CItemUser::OnUpdataGrade(SEquipment * pEquip)
{
	if (!pEquip)
	{
		rfalse(4, 1, "Itemuser.cpp - AddRefineAttribute() - !pEquip");
		return FALSE;
	}
	
	const SItemBaseData* pData = CItemService::GetInstance().GetItemBaseData(pEquip->wIndex);
	if (!pData)
		return FALSE;

	const SItemFactorData *pItemFactor = CItemService::GetInstance().GetItemFactorData(pEquip->wIndex);
	if (!pItemFactor)
		return FALSE;
	
	
	// ��ӻ�������
	for (size_t k = 0; k < SEquipment::MAX_BORN_ATTRI; ++k)
	{
		WORD attri = 0;
		WORD attvalue = 0;
		if (CItemService::GetInstance().GetFactorData(pEquip->wIndex,attri,attvalue,k))
		{
			pEquip->attribute.bornAttri[k].type = attri;
			pEquip->attribute.bornAttri[k].value = attvalue;
		}
	}	
	return TRUE;
}
BOOL CItemUser::AddRefineAttribute(SEquipment* pEquip)
{
	if (!pEquip)
	{
		rfalse(4, 1, "Itemuser.cpp - AddRefineAttribute() - !pEquip");
		return FALSE;
	}
	MY_ASSERT(pEquip);

	CPlayer* player = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	const SItemBaseData* pData = CItemService::GetInstance().GetItemBaseData(pEquip->wIndex);
	if (!pData)
		return FALSE;

	const SItemFactorData *pItemFactor = CItemService::GetInstance().GetItemFactorData(pEquip->wIndex);
	if (!pItemFactor)
		return FALSE;

	// ��ȡ�����׾����������ԡ����ñ�
	const SItemUpgradeAttribute* pUattri = CItemService::GetInstance().GetUpgradeAttribute(pData);
	if (!pUattri)
		return FALSE;

	const SMaxExtraAttri *pMaxExtraAttri = CItemService::GetInstance().GetMaxExtraAttri(pData, pEquip->attribute.grade);
	if (!pMaxExtraAttri)
		return FALSE;

	// �õ�װ������������б�
	const SItemUpgradeAttribute *pUpgradeAttri = CItemService::GetInstance().GetUpgradeAttribute(pData);
	if (!pUpgradeAttri)
	{
		return FALSE;
	}
	MY_ASSERT(pEquip);

	// ��Ӹ�������
	switch(player->m_refineType)
	{
		case SQAddEquip::ADE_REFINE_GRADE:
			break;
		
		case SQAddEquip::ADE_REFINE_STAR:
			{
				int num	= 0;		// ������Ǹ��ӵĻ������Եĸ���

				int star_num = 0;	// ���ǵľ��Ը���
				int starAttriValue = 0;

				// �ж����ǵĸ���
				size_t i = 0;
				for (; i < SEquipment::MAX_STAR_NUM; ++i)
				{
//					if (SEquipment::SLOT_IS_VALID == pEquip->attribute.starNum[i])
					{
						++star_num;
					}
					//if (SEquipment::SLOT_IS_EMPTY == pEquip->attribute.starNum[i])
					{
						break;
					}
				}
				if (star_num >=  SEquipment::MAX_STAR_NUM)
				{
					return FALSE;
				}
				int iStarIndex = star_num-1;
				if (iStarIndex < 0 || iStarIndex > SEquipment::MAX_STAR_NUM)
				{
					return FALSE;
				}
				// [2012-8-21 19-13 gw: -�±����] starAttriValue = pUattri->wStarAttri[star_num];
				starAttriValue = pUattri->wStarAttri[iStarIndex];
				for (size_t j = 0; j < SEquipDataEx::EEA_MAX; ++j)
				{
					if (pItemFactor->m_ValueFactor[j])
					{
						// [2012-8-17 18-17 gw: +���ӻ�������]
						pEquip->attribute.starAttri[num].type	= j;
						pEquip->attribute.starAttri[num].value	= starAttriValue;
						++num;
						
						//// ��󸽼�����ֵ
						//pEquip->attribute.MaxExtraAttri[num].type	= j;
						//pEquip->attribute.MaxExtraAttri[num++].value = ExtraAttribute->ExtraData[j][1] + pMaxExtraAttri->ExtraData[j];
					}

					if (num >= SEquipment::MAX_GRADE_ATTRI)
					{
						break;
					}
				}

				// [2012-8-21 18-52 gw: +�����Ч����]
// 				int iStarAttriLen = sizeof(pEquip->attribute.starAttri) / sizeof(pEquip->attribute.starAttri[0]);
// 				for (i = num; i < iStarAttriLen; ++i)
// 				{
// 					pEquip->attribute.starAttri[i].type = 0;
// 					pEquip->attribute.starAttri[i].value = 0;
// 				}
			}
			break;
		case SQAddEquip::ADE_ReMove_STAR:
			{
				memset(&pEquip->attribute.starAttri,0,sizeof(SEquipment::ExtraAttri) * SEquipment::MAX_GRADE_ATTRI);
//				memset(&pEquip->attribute.starNum,0,sizeof(BYTE) * SEquipment::MAX_STAR_NUM);
			}
			break;
		case SQAddEquip::ADE_EQUIP_REFINE: //����
			{
				std::vector<int> typevec;				
				int	iTmpAddValue = 0; // ��ӵ�����ֵ����Ҫ�����������ȡ
				int iTmpHandleAttriNum = 1; // ��Ҫ�ڼ���������������������ֵ				

				// ȡ���Ѿ��еĸ�������ֵ
				std::vector<int> vecTmpBornAttriIndex;
				const int iArrayBornAttriLen = sizeof(pEquip->attribute.bornAttri) / sizeof(pEquip->attribute.bornAttri[0]);
				for (size_t i = 0; i < iArrayBornAttriLen; ++i)
				{
//					WORD wTmpRefineValue = (pEquip->attribute.refineExtraAttri[i].type==0xff)?0:pEquip->attribute.refineExtraAttri[i].value;
// 					if (pEquip->attribute.bornAttri[i].type != 0xff
// 						&& pEquip->attribute.bornAttri[i].value + wTmpRefineValue < pEquip->attribute.MaxExtraAttri[i].value)
					{ //����ﵽ���ֵ�ˣ����ٻ��о���
						vecTmpBornAttriIndex.push_back(i);
					}
				}
				if (vecTmpBornAttriIndex.size() == 0)
				{
					break;
				}

				for (size_t i = 0; i < iTmpHandleAttriNum; ++i)
				{
					//���������� iBornIndex = vecTmpBornAttriIndex.at(iBornIndex);
					int iBornIndex = CRandom::RandRange(0, vecTmpBornAttriIndex.size()-1);
					iBornIndex = vecTmpBornAttriIndex[iBornIndex];
					MY_ASSERT(iBornIndex >=0 && iBornIndex < iArrayBornAttriLen);
					int iTmpType = pEquip->attribute.bornAttri[iBornIndex].type;
					MY_ASSERT( iTmpType >= 0 && iTmpType < SEquipDataEx::EEA_MAX);
					iTmpAddValue = pUattri->equipRefineRequire.iaExtraData[iTmpType][SEquipExtraAttriRequire::EExtraData_JingLianAdd];

//					if (pEquip->attribute.refineExtraAttri[iBornIndex].type == 0xff)
				//	{
// 						pEquip->attribute.refineExtraAttri[iBornIndex].type = iTmpType;
// 						pEquip->attribute.refineExtraAttri[iBornIndex].value = iTmpAddValue;
// 					}
// 					else 
// 					{
// 						pEquip->attribute.refineExtraAttri[iBornIndex].value += iTmpAddValue;
// 					}

					// Խ���ж�
// 					if (pEquip->attribute.bornAttri[iBornIndex].value + pEquip->attribute.refineExtraAttri[iBornIndex].value 
// 								> pEquip->attribute.MaxExtraAttri[iBornIndex].value)
// 					{
// 						pEquip->attribute.refineExtraAttri[iBornIndex].value = pEquip->attribute.MaxExtraAttri[iBornIndex].value 
// 							- pEquip->attribute.bornAttri[iBornIndex].value;
// 					}
				}		
			}
			break;

		default:
			return FALSE;
	}
	return TRUE;
}

// Func: ������������������ֵ
// @ pEquip- ��Ҫ��Ӹ������Ե�װ��
// @ bFillRefineAttri-�Ƿ���丽�����Ե���󸽼�����,���ã���-�����þ�������ֵ��ʹ��������֮�ʹﵽ��󸽼�����ֵ
BOOL CItemUser::AddMaxExtraAttri(SEquipment* pEquip, BOOL bFillRefineAttri/* = FALSE*/)
{

	return FALSE;
}
// ˽�к�������߼����ʱ���Ƴ�����ǿ������
BOOL CItemUser::RemoveRefineMetrial()
{
	CPlayer* pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);	
	
	if (0 == pPlayer->m_vecStrengthenEquip.size())
	{
		return FALSE;
	}
	const SEquipment* pEquip = GetEquipmentbyItem(pPlayer->m_vecStrengthenEquip.at(0), XYD_FT_WHATEVER);
	if (!pPlayer || !pEquip)
	{
		return FALSE;
	}
	
	if (pPlayer->m_vecStrengthenEquip.size() > 0)
	{		
		for (int i = 0; i < pPlayer->m_vecStrengthenEquip.size(); ++i)
		{
			LockItemCell(pPlayer->m_vecStrengthenEquip.at(i), false);
		}				
		//player->m_strengthenEquip = 0;
		pPlayer->m_vecStrengthenEquip.clear();
		pPlayer->m_refineType = SQAddEquip::ADE_NONE;
	}
		
	TalkToDnid(pPlayer->m_ClientIndex, "�Ѿ��ﵽ���ȼ��ˣ����ܼ���ǿ��װ����");
	return TRUE;
}

// ˽�к�����װ���Ƿ�������װ��
BOOL CItemUser::Equip_Is_Tyler(DWORD wIndex)
{
	if ((wIndex >= 9000001 && wIndex <= 9000005) || (wIndex >= 9010001 && wIndex <= 9010024))
		return FALSE;
	else
		return TRUE;
}

// ������ʾ��Ϣ����
void CItemUser::SendUpdateEquipInfo(SEquipment* equip)
{
	CPlayer* player = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!player || !equip)
	{
		return;
	}
	MY_ASSERT(player && equip);

	SAEquipInfoInBagMsg _msg;

	memcpy(&_msg.equip, equip, sizeof(SEquipment));
	
	if (player->m_vecStrengthenEquip.size() > 0)
	{
		_msg.wPos = player->m_vecStrengthenEquip.at(0);		
	}
	g_StoreMessage(player->m_ClientIndex, &_msg, sizeof(SAEquipInfoInBagMsg));
}

// �����ɹ��������װ��
long CItemUser::AddStrengthenEquip(DWORD ItemId)
{
	CPlayer* pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return 99999;

	if (SQAddEquip::ADE_UPDATE_LEVEL != pPlayer->m_refineType && SQAddEquip::ADE_UPDATE_QUALITY != pPlayer->m_refineType)
	{
		return 99999;
	}
	//MY_ASSERT(SQAddEquip::ADE_UPDATE_LEVEL == pPlayer->m_refineType || SQAddEquip::ADE_UPDATE_QUALITY == pPlayer->m_refineType);
	
	if (0 == pPlayer->m_vecStrengthenEquip.size())
	{
		return 99999;
	}
	SEquipment *pEquip = GetEquipmentbyItem(pPlayer->m_vecStrengthenEquip.at(0), XYD_FT_WHATEVER);
	if (!pEquip)
		return 99999;
	SPackageItem *pPackagItem = FindItemByPos(pPlayer->m_vecStrengthenEquip.at(0), XYD_FT_WHATEVER);

	DWORD wIndex = 0;
	
	SRawItemBuffer item;
	GenerateNewItem(item, SAddItemInfo(ItemId, 1));

	// ������װ������
	SEquipment *equip = (SEquipment*)((SItemBase*)&item);
	if (!equip)
	{
		rfalse(4, 1, "Itemuser.cpp - AddStrenthenEquip() - !equip");
		return	99999;
	}
	//pEquip->wIndex = wIndex;
	pEquip->attribute.currWear = equip->attribute.currWear;
	pEquip->attribute.maxWear = equip->attribute.maxWear;
	memcpy(equip, pEquip, sizeof(SEquipment));
	
	if (NULL != pPackagItem)
	{// ��������е�װ��
		DelItem(*pPackagItem, "��װ��������ʾ���ɹ�������ɾ��ԭװ����", true);
	}
	else
	{// ����Լ����ϵ�װ�� todo..

	}
	
	//pPlayer->m_strengthenEquip = 0;
	if (pPlayer->m_vecStrengthenEquip.size() > 0)
	{		
		for (int i = 0; i < pPlayer->m_vecStrengthenEquip.size(); ++i)
		{
			LockItemCell(pPlayer->m_vecStrengthenEquip.at(i), false);
		}				
		//player->m_strengthenEquip = 0;
		pPlayer->m_vecStrengthenEquip.clear();		
	}

	equip->wIndex = ItemId;
	// �����󸽼��������
	AddMaxExtraAttri(equip);
	WORD pos = FindBlankPos(XYD_PT_BASE);
	AddExistingItem(item, pos, true);

	

	// ������װ��������
	SPackageItem *pItemElem = FindItemByPos(pos, XYD_FT_ONLYUNLOCK);
	if (NULL != pItemElem)
	{
		pPlayer->m_vecStrengthenEquip.push_back(pos);
	}	
	LockItemCell(pos, true);

	return pos;
}

BOOL CItemUser::RecvSpecialUseItem( struct SQSpecialUseItemMsg* pMsg )
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvSpecialUseItem() - !pMsg");
		return FALSE;
	}

	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	if (!IsItemPosValid(pMsg->wCellPos))
		return FALSE;

	SPackageItem *pIB = FindItemByPos(pMsg->wCellPos, XYD_FT_ONLYUNLOCK);
	if (!pIB)
	{
		return FALSE;
	}
	MY_ASSERT(pIB);

	const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(pIB->wIndex);
	if (!pData)
		return FALSE;

	BYTE index = pMsg->windex;
	BOOL IsEquip = FALSE;
	if(pMsg->btype == SQSpecialUseItemMsg::USEFOR_XIAKE)
	{
		if (index>=MAX_FIGHTPET_NUM)return FALSE;
		if (player->m_Property.m_FightPets[index].m_fpID == 0)return FALSE;
		if (ITEM_IS_EQUIPMENT(pData->m_Type))
		{
			//����װ�����ߵ���������
			player->_fpSetEquipFightPet(index);
			IsEquip = _EquipFightPetItem(pIB,index);
			///�����Ҫװ�����ߵ���������
			player->_fpInitEquipFightPetIndex();
		}
		else if (ITEM_IS_BOOK(pData->m_Type))
		{
			IsEquip = player->_fpRecvEquipSkillBookMsg(index,pIB);
		}
	}
		return IsEquip;
}

BOOL CItemUser::_EquipFightPetItem( SPackageItem *pItem,BYTE index )
{
	if (!pItem || 0 == pItem->wIndex || 0 == pItem->overlap)
		return FALSE;

	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	const SItemBaseData *itemWillEquip = CItemService::GetInstance().GetItemBaseData(pItem->wIndex);	
	//if (!itemWillEquip)
	//{
	//	return FALSE;
	//}
	//MY_ASSERT(itemWillEquip);

	if (!itemWillEquip)
	{
		///�����Ҫװ�����ߵ���������
		player->_fpInitEquipFightPetIndex();
		return FALSE;
	}
		
	MY_ASSERT(ITEM_IS_EQUIPMENT(itemWillEquip->m_Type) && 1 == itemWillEquip->m_Overlay && 1 == pItem->overlap);

	///�����������װ��
// 	if (itemWillEquip->m_School != IO_XIAKE)
// 	{
// 		///�����Ҫװ�����ߵ���������
// 		player->_fpInitEquipFightPetIndex();
// 		return FALSE;
// 	}
	
	if (!CanEquipItFightPet(player,pItem,itemWillEquip,index))
	{
		///�����Ҫװ�����ߵ���������
		player->_fpInitEquipFightPetIndex();
		return FALSE;
	}
		

	EQUIP_POSITION i = EQUIP_P_WEAPON;
	switch (itemWillEquip->m_Type)
	{
	case EQUIP_T_WEAPON:
		i = EQUIP_P_WEAPON;
		break;
	case EQUIP_T_HAT:
		i = EQUIP_P_HAT;
		break;
	case EQUIP_T_WRIST:
		i = EQUIP_P_WRIST;
		break;
	case EQUIP_T_PAT:
		i = EQUIP_P_PAT;
		break;
	case EQUIP_T_WAIST:
		i = EQUIP_P_WAIST;
		break;
	case EQUIP_T_SHOSE:
		i = EQUIP_P_SHOSE;
		break;
	case EQUIP_T_CLOTH:
		i = EQUIP_P_CLOTH;
		break;
	case EQUIP_T_MANTLE:
		i = EQUIP_P_MANTLE;
		break;
	case EQUIP_T_PENDANTS:
		if (player->m_Property.m_FightPets[index].m_Equip[EQUIP_P_PENDANTS1].wIndex == 0)
			i = EQUIP_P_PENDANTS1;
		else if (player->m_Property.m_FightPets[index].m_Equip[EQUIP_P_PENDANTS2].wIndex == 0)
			i = EQUIP_P_PENDANTS2;
		else
			i = EQUIP_P_PENDANTS1;
		break;
	case EQUIP_T_PROTECTION:
		i = EQUIP_P_PROTECTION;
		break;
	case EQUIP_T_RING:
		if (player->m_Property.m_FightPets[index].m_Equip[EQUIP_P_RING1].wIndex == 0)
			i = EQUIP_P_RING1;
		else if (player->m_Property.m_FightPets[index].m_Equip[EQUIP_P_RING2].wIndex == 0)
			i = EQUIP_P_RING2;
		else
			i = EQUIP_P_RING1;
		break;
	case EQUIP_T_PRECIOUS:
		i = EQUIP_P_PRECIOUS;
		break;
	case EQUIP_T_SHIZHUANG_HAT:
		i = EQUIP_P_SHIZHUANG_HAT1;
		break;
	case EQUIP_T_SHIZHUANG_CLOTH:
		i = EQUIP_P_SHIZHUANG_CLOTH1;
		break;
	default:
		///�����Ҫװ�����ߵ���������
		player->_fpInitEquipFightPetIndex();
		return FALSE;
	}

	// ���浱ǰλ���ϵ�װ������
	SEquipment tempEquip = player->m_Property.m_FightPets[index].m_Equip[i];

	SItemBase *item = pItem;
	player->m_Property.m_FightPets[index].m_Equip[i] = *(SEquipment *)item;

	// ��װ����Ϣ���͸��ͻ���
	SAFightPetEquipInfoMsg msg;
	msg.stEquip = player->m_Property.m_FightPets[index].m_Equip[i];
	msg.byPos   = i;
	msg.index = index;
	g_StoreMessage(player->m_ClientIndex, &msg, sizeof(msg));

	if ((IsPosNeedToSwitchMesh(i)) && m_ParentArea)
	{
		SAFightPetEquipItemMsg msg;
		msg.dwGlobalID = GetGID();
		msg.byEquipPos = i;
		msg.wItemIndex = item->wIndex;
		msg.index = index;

		// ����Ϣ���͸���Ч������������
		CArea *area = (CArea *)m_ParentArea->DynamicCast(IID_AREA);
		if (area)
			area->SendAdj(&msg, sizeof(msg), -1);
	}

	// ���¼�������װ���ӳ�����
	player->_fpInitEquipmentData();
	player->_fpUpdateAllProperties();

	// ��ǰװ��֮ǰ����Ч����ʱ�����ڵ����ƶ���ɾ��װ�����е�
	if (tempEquip.wIndex == 0)
		return DelItem(*pItem);

	// ����ǵ��߽���������Ҫ����������ߵ�����
	memset((SItemBase*)pItem, 0, sizeof(SRawItemBuffer));
	memcpy((SItemBase*)pItem, (SItemBase*)&tempEquip, sizeof(SRawItemBuffer));
	SendItemSynMsg(pItem);

	// ����״̬����ʵ����װ�������ø��£���Ϊװ���ǲ��ɵ��ӵģ���
	ValidationPos(pItem->wCellPos);

	///�����Ҫװ�����ߵ���������
	player->_fpInitEquipFightPetIndex();
	return TRUE;
}

BOOL CItemUser::CanEquipItFightPet( CPlayer *pPlayer, SPackageItem *pIB, const struct SItemBaseData *pData ,BYTE index)
{
	if (!pPlayer || !pIB || !pData)
	{
		rfalse(4, 1, "Itemuser.cpp - CanEquipItFightPet() - !pPlayer || !pIB || !pData");
		return FALSE;
	}

	BOOL result = ITEM_IS_EQUIPMENT(pData->m_Type);

	if (!pData->m_CanUse)return FALSE;

	if (0 == pPlayer->m_Property.m_FightPets[index].m_fpID)return FALSE;

	WORD temp =  pPlayer->m_Property.m_FightPets[index].m_bSex;

	// �Ա���֤
	if (pData->m_Sex != 0 && temp != pData->m_Sex)
		return FALSE;

	temp =  pPlayer->m_Property.m_FightPets[index].m_fpLevel;

	// �ȼ�������֤
	if (pData->m_Level != 0 && temp< pData->m_Level)
		return FALSE;
	
	// ����������֤
	const SFightPetBaseData *pBaseData =CFightPetService::GetInstance().GetFightPetBaseData(pPlayer->m_Property.m_FightPets[index].m_fpID);
	if(!pBaseData)return FALSE;
	if (pData->m_School != pBaseData->m_Type&&pData->m_School != IO_XIAKE)
	{
		TalkToDnid(pPlayer->m_ClientIndex,"���͵����ɲ�ͬ�޷�װ��");
		return FALSE;
	}

	// �����ȴ
	if (CheckItemCDTime(pData))
		return FALSE;

	// �;ö�Ϊ0������װ��
	if (0 == reinterpret_cast<SEquipment *>(static_cast<SRawItemBuffer*>(pIB))->attribute.currWear)
		result &= FALSE;

	return result;
}

BOOL CItemUser::_fpToPackage( BYTE byPos,BYTE index )
{
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	if (0 == player->m_Property.m_FightPets[index].m_fpID)return FALSE;
	
	//���ò���������
	if (!player->_fpSetEquipFightPet(index))return FALSE;

	//Ѱ�ҵ�ǰλ�õ�װ��
	if (byPos >= EQUIP_P_MAX || (player->m_Property.m_FightPets[index].m_Equip[byPos].wIndex == 0))
		return FALSE;

	WORD emptyPos = FindBlankPos(XYD_PT_BASE);
	if (0xffff == emptyPos)
		return FALSE;

	// ����Ϊ�˺����Ĭ�ϲ���װ�����Ӧ
	if (0 == player->m_Property.m_FightPets[index].m_Equip[byPos].uniID)
	{
		extern BOOL GenerateNewUniqueId(SItemBase &item);

		// ˵�������ݿ������Ĭ�ϲ����ģ���ô���������һ��UniqueID
		if (!GenerateNewUniqueId(player->m_Property.m_FightPets[index].m_Equip[byPos]))
			return FALSE;
	}

	bool ret = AddExistingItem(player->m_Property.m_FightPets[index].m_Equip[byPos], emptyPos, true) && _fpDelEquipment(byPos);

	return ret ? TRUE : FALSE;
}

BOOL CItemUser::_fpDelEquipment( BYTE equipId )
{
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;
	int index = player->_fpgetEquipFightPetIndex();
	if (index < 0 )return FALSE;
	
	if (0 == player->m_Property.m_FightPets[index].m_fpID)return FALSE;

	if ((equipId >= EQUIP_P_MAX ) || (player->m_Property.m_FightPets[index].m_Equip[equipId].wIndex == 0))
		return FALSE;

	ZeroMemory(&player->m_Property.m_FightPets[index].m_Equip[equipId], sizeof(SEquipment));

	// ������׼�������Ĵ�ϲ���

	// ��װ������Ϣ���͸��ͻ���
	SAFightPetEquipInfoMsg msg;
	msg.stEquip = player->m_Property.m_FightPets[index].m_Equip[equipId];
	msg.byPos   = equipId;
	msg.index = index;
	g_StoreMessage(player->m_ClientIndex, &msg, sizeof(msg));

	if (IsPosNeedToSwitchMesh(equipId))
	{
		SAFightPetEquipItemMsg EquipItemMsg;

		EquipItemMsg.dwGlobalID = GetGID();
		EquipItemMsg.byEquipPos = equipId;
		EquipItemMsg.wItemIndex = 0;
		EquipItemMsg.index = index;

		// ����Ϣ������Ч������������
		if (CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA))
			pArea->SendAdj(&EquipItemMsg, sizeof(SAEquipItemMsg), -1);
	}

	// ���¼���װ���ӳ�����
	player->_fpInitEquipmentData();
	player->_fpUpdateAllProperties();

	player->_fpInitEquipFightPetIndex();

	return TRUE;
}

BOOL CItemUser::RecvUnfpEquipItem( struct SQfpUnEquipItemMsg *pMsg )
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuesr.cpp - RecvUnfpEquipItem() - !pMsg");
		return FALSE;
	}

	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	if (pMsg->dwGlobalID != player->GetGID())
		return FALSE;

	return _fpToPackage(pMsg->byEquipPos,pMsg->index);
}

BOOL CItemUser::_fpToEquip( WORD wCellPos,BYTE index )
{
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	if (!IsItemPosValid(wCellPos))
		return FALSE;

	WORD linePos = wCellPos;
	//if (linePos >= TaskStart)
	//	return FALSE;

	SPackageItem *pIB = FindItemByPos(linePos, XYD_FT_ONLYUNLOCK);
	if (!pIB)
	{
		return FALSE;
	}
	MY_ASSERT(pIB);	
	const SItemBaseData *itemWillEquip = CItemService::GetInstance().GetItemBaseData(pIB->wIndex);	
	if (!itemWillEquip)
	{
		return FALSE;
	}
	//+ ������Ʒ����װ��
	if ( ITEM_IS_TASK(itemWillEquip->m_Type) )
	{
		return FALSE;
	}

	player->_fpSetEquipFightPet(index);
	return _EquipFightPetItem(pIB,index);
}

BOOL CItemUser::_UseFightPetItem( SPackageItem *pItem,BYTE index )
{
	if (!pItem || 0 == pItem->wIndex || 0 == pItem->overlap)
		return FALSE;

	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(pItem->wIndex);
	if (!pData)
		return FALSE;
}

// ȡ��ϴ��
BOOL CItemUser::RecvEndResetPoint(struct SQEndResetPoint* pMsg)
{
	
	return TRUE;
}

DWORD CItemUser::GetAllItemNum( WORD GetType )
{
	DWORD nNum = 0;
	DWORD wIndex = 0;
	for (DWORD i=0; i<PackageAllCells; i++)
	{
		SPackageItem *pItem = FindItemByPos(i, GetType);
		if (pItem && pItem->wIndex){
			nNum ++;
		}
	}

	return nNum;
}

DWORD CItemUser::GetRadomItem( WORD GetType /*= XYD_FT_ONLYUNLOCK*/ )
{
	if (GetAllItemNum(GetType) == 0)return 0;
	DWORD itemCells = CRandom::RandRange(0,PackageAllCells-1);
	SPackageItem *pItem = NULL;
	while(1)
	{
		pItem = FindItemByPos(itemCells, GetType);
		if (pItem)return pItem->wIndex;
		itemCells = CRandom::RandRange(0,PackageAllCells-1);
	}
	return 0;
}

int CItemUser::GetRadomItemPos( WORD GetType /*= XYD_FT_ONLYUNLOCK*/ )
{
	if (GetAllItemNum(GetType) == 0)return -1;
	DWORD itemCells = CRandom::RandRange(0,PackageAllCells-1);
	SPackageItem *pItem = NULL;
	while(1)
	{
		pItem = FindItemByPos(itemCells, GetType);
		if (pItem)return itemCells;
		itemCells = CRandom::RandRange(0,PackageAllCells-1);
	}
	return -1;
}


BOOL CItemUser::RecvQuestFollowItem(struct SQQuestFollowItemMsg *pMsg)
{
	if (!pMsg)
	{
		return false;
	}
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
	{
		return false;
	}
	SAQuestFollowItemMsg Msg;
	bool Flag = CFollowShopService::GetInstance().GetFollowShopList(pMsg->m_Index,&Msg.m_Follow.m_Itemlist[0],Msg.m_Follow.m_Num,
		Msg.m_Follow.m_name,Msg.m_Follow.m_NameNum);
	if (Flag)
	{
		player->m_CurFollowIndex = pMsg->m_Index;
		g_StoreMessage(player->m_ClientIndex,&Msg,sizeof(Msg));
	} 
	else
	{
		TalkToDnid(player->m_ClientIndex,"�����̵�ĵ����б�����!");
	}
	return true;
}
BOOL CItemUser::RecvBuyFollowItem(struct SQBUYFollowItemMsg *pMsg)
{
	if (!pMsg)
	{
		return false;
	}
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
	{
		return false;
	}
	bool Flag = CFollowShopService::GetInstance().IsExitFollowItemID(pMsg->m_Index,pMsg->m_ItemID);
	if (Flag)
	{
		BuyItem(pMsg->m_ItemID,pMsg->m_Num);
		return true;
	}
	else
	{
		TalkToDnid(player->m_ClientIndex,"�����̵�ĵ��߲�����!");
	}
	return false;
}

BOOL CItemUser::RecvBlessExcellenceData(struct SQBlessExcellenceDataMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvBlessExcellenceData() - !pMsg");
		return FALSE;
	}

	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
	{
		return false;
	}
	g_Script.SetCondition(0, player, 0);
	LuaFunctor(g_Script,"GetBlessInfo")();
	g_Script.CleanCondition();
	return true;
}
BOOL CItemUser::RecvBlessBlessReQuest(struct SQBlessItemReQuestMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemsuer.cpp - RecvBlessBelssReQuest() - !pMsg");
		return FALSE;
	}

	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
	{
		return false;
	}
	if (player->m_PlayerOpenClose == 2)
	{
		return FALSE;
	}
	if (m_Blessvec.size() != 0)
	{
		m_Blessvec.clear();
	}
	if (!pMsg)
	{
		return FALSE;
	}
	
	if (player->m_Property.m_BlessOnce == 1)
	{
		rfalse(2, 1, "����û��������գ��ͷ����ˣ�");
		
		OnAllNotice();
		player->LockItemCellTwo(player->m_Property.m_BlessSPackageitem.wCellPos,false);
		player->m_Property.m_BlessOnce = 0;
		SABlessQuestAddItems msg;
		msg.Item = player->m_Property.m_BlessSPackageitem;
		g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SABlessQuestAddItems));

		SABlessStopBless pMsg;
		pMsg.BlessStoptype = 3;
		g_StoreMessage(player->m_ClientIndex,&pMsg,sizeof(SABlessStopBless));
		return FALSE;
	}
	g_Script.SetCondition(0, player, 0);
	LuaFunctor(g_Script,"OnProcessGetBless")[pMsg->Flag][pMsg->QuickFlag]();
	g_Script.CleanCondition();
	return true;
}

// ������������
BOOL CItemUser::RecvDragUpdateItem(struct SQDragUpdateItemMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "itemuser.cpp - RecvDragUpdateItem() - !pMsg");
		return FALSE;
	}

	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	// �����쳣~
	if (SQDragUpdateItemMsg::DUI_NONE == pMsg->bType || (SQDragUpdateItemMsg::DUI_NONE != player->m_UpdateItemType && player->m_UpdateItemType != pMsg->bType))
		return FALSE;

	if (!IsItemPosValid(pMsg->wCellPos))
		return FALSE;

	SPackageItem *srcItem = FindItemByPos(pMsg->wCellPos, XYD_FT_ONLYUNLOCK);
	if (!srcItem)
		return FALSE;

	const SItemBaseData *srcData = CItemService::GetInstance().GetItemBaseData(srcItem->wIndex);
	if (!srcData)
		return FALSE;

	// �ж������������
	BYTE	CurrentUpdateTimes;
	BYTE*	pBuf = srcItem->buffer;
	if (!pBuf)
	{
		rfalse(4, 1, "Itemuser.cpp - !pBuf");
		return FALSE;
	}
	
	memcpy(&CurrentUpdateTimes, srcItem->buffer, sizeof(BYTE));
	// 
	//if (CurrentUpdateTimes >= 9)
	//	return FALSE;

	// ==================================
	if (player->m_UpdateItem)
	{
		rfalse(2, 1, "�����񵤡��Ѿ������һ�����ߣ�������ǰ���ߣ�");
		LockItemCell(player->m_UpdateItem->wCellPos, false);

		if (player->m_LJS_Metrial)
		{
			rfalse(2, 1, "���ʲ���ҲҪ��գ�");
			LockItemCell(player->m_LJS_Metrial->wCellPos, false);
			player->m_LJS_Metrial = 0;
		}

		if (player->m_XMTJ_Metrial)
		{
			rfalse(2, 1, "���ʲ���ҲҪ��գ�");
			LockItemCell(player->m_XMTJ_Metrial->wCellPos, false);
			player->m_XMTJ_Metrial = 0;
		}
	}

	player->m_UpdateItem = srcItem;
	player->m_UpdateItemType = pMsg->bType;	

	LockItemCell(pMsg->wCellPos, true);

	return TRUE;
}

// ��������������Ҫ�ı�������
BOOL CItemUser::RecvDragUpdateMetrial(struct SQDragUpdateMetrialMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "itemuser.cpp - RecvDragUpdateMetrial - !pMsg");
		return FALSE;
	}

	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	// ��Ҫ������װ��
	if (!pPlayer->m_UpdateItem)
		return FALSE;

	if (!IsItemPosValid(pMsg->wCellPos))
		return FALSE;

	SPackageItem *srcItem = FindItemByPos(pMsg->wCellPos, XYD_FT_ONLYUNLOCK);
	if (!srcItem)
		return FALSE;

	const SItemBaseData *srcData = CItemService::GetInstance().GetItemBaseData(srcItem->wIndex);
	if (!srcData)
		return FALSE;

	// �ж������Ƿ���ȷ
	const BYTE updateType = pPlayer->m_UpdateItemType;

	if (SQDragUpdateItemMsg::DUI_HUISHENDAN == updateType)
	{
		if (srcItem->wIndex != LIANZHISHI_ID && srcItem->wIndex != XUANMENGTIANJING_ID)
			return FALSE;
	}

	if (SQDragUpdateMetrialMsg::DUM_LIANZHI == pMsg->bType)
	{
		if (pPlayer->m_LJS_Metrial)
		{
			rfalse(2, 1, "[����]�Ѿ������˲���[����ʯ]�������ǰ����~~");
			LockItemCell(pPlayer->m_LJS_Metrial->wCellPos, false);
		}

		pPlayer->m_LJS_Metrial = srcItem;
	}

	if (SQDragUpdateMetrialMsg::DUM_XUANJING == pMsg->bType)
	{
		if (pPlayer->m_XMTJ_Metrial)
		{
			rfalse(2, 1, "[����]�Ѿ������˲���[�����쾫]�������ǰ����~~");
			LockItemCell(pPlayer->m_XMTJ_Metrial->wCellPos, false);
		}
		
		pPlayer->m_XMTJ_Metrial = srcItem;
	}

	LockItemCell(pMsg->wCellPos, true);

	return TRUE;
}

// ��ʼ��������
BOOL CItemUser::RecvBeginUpdateItem(struct SQBeginUpdateItem *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvBeginUpdateItem() - !pMsg");
		return FALSE;
	}

	CPlayer* pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	if (!pPlayer->m_UpdateItem)
		return FALSE;

	if (SQDragUpdateItemMsg::DUI_NONE == pPlayer->m_UpdateItemType || SQDragUpdateItemMsg::DUI_HUISHENDAN != pPlayer->m_UpdateItemType)
		return FALSE;

	/*	�˴����������SRawItemBuffer�й��ڵ�����������Ϣ
		struct SRawItemBuffer : public SItemBase
		{
		BYTE buffer[120];		// ���ÿռ�
		};
	*/
	// ==============���ýű�����������ݲ���===============
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "HuiShenDanLimitInfo")[pPlayer->m_UpdateItem->wIndex]();
	g_Script.CleanCondition();
	// ====================================
	return TRUE;
}

// ������������
BOOL CItemUser::RecvEndUpdateItem(struct SQEndUpdateItem *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvEndUpdateItem() - !pMsg");
		return FALSE;
	}

	CPlayer* pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	bool ItemOff	= false;
	bool LJSOff		= false;
	bool XMTJOff	= false;

	switch(pMsg->bType)
	{
	case SQEndUpdateItem::EUI_ITEM_OFF:
		ItemOff = true;
		LJSOff = true;
		XMTJOff = true;
		break;

	case SQEndUpdateItem::EUI_LJS_OFF:
		LJSOff = true;
		break;

	case SQEndUpdateItem::EUI_XMTJ_OFF:
		XMTJOff = true;
		break;

	default:
		return FALSE;
		MY_ASSERT(0);
	}

	if (ItemOff && pPlayer->m_UpdateItem)
	{
		LockItemCell(pPlayer->m_UpdateItem->wCellPos, false);
		pPlayer->m_UpdateItem = 0;
		pPlayer->m_UpdateItemType = SQDragUpdateItemMsg::DUI_NONE;
	}

	if (LJSOff && pPlayer->m_LJS_Metrial)
	{
		LockItemCell(pPlayer->m_LJS_Metrial->wCellPos, false);
		pPlayer->m_LJS_Metrial = 0;
	}

	if (XMTJOff && pPlayer->m_XMTJ_Metrial)
	{
		LockItemCell(pPlayer->m_XMTJ_Metrial->wCellPos, false);
		pPlayer->m_XMTJ_Metrial = 0;
	}
	
	return TRUE;
}

BOOL CItemUser::BeginUpdateItem()
{
	CPlayer* pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	if (!pPlayer->m_UpdateItem)
		return FALSE;

	if (SQDragUpdateItemMsg::DUI_NONE == pPlayer->m_UpdateItemType || SQDragUpdateItemMsg::DUI_HUISHENDAN != pPlayer->m_UpdateItemType)
		return FALSE;

	BYTE *pCurrentUpdateTimes = new BYTE;
	BYTE *pSuccessUpdateTimes = new BYTE;
	WORD *pSPValue = new WORD;

	BYTE* pBuf = pPlayer->m_UpdateItem->buffer;
	if (!pBuf)
	{
		rfalse(4, 1, "Itemuser.cpp - BeginUpdateItem() - !pBuf");
		return FALSE;
	}

	// �õ������Ĵ�������
	memcpy(pCurrentUpdateTimes, pBuf, sizeof(BYTE));
	pBuf += sizeof(BYTE);

	// �õ��Ѿ������ɹ��Ĵ���
	memcpy(pSuccessUpdateTimes, pBuf, sizeof(BYTE));
	pBuf += sizeof(BYTE);

	// �õ���ǰ������ֵ
	memcpy(pSPValue, pBuf, sizeof(WORD));
	
	SABeginUpdateItem msg;

	if (*pCurrentUpdateTimes >= pPlayer->m_MaxUpdateTimes)
	{
		msg.bSuccessTimes = *pSuccessUpdateTimes;
		msg.bTotalUpdateTimes = *pCurrentUpdateTimes;
		msg.wCellPos = pPlayer->m_UpdateItem->wCellPos;

		msg.bType = SABeginUpdateItem::BUI_MAX_TIMES;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginUpdateItem));
		
		return FALSE;
	}

	if (*pCurrentUpdateTimes < *pSuccessUpdateTimes)
	{
		return FALSE;
	}
	MY_ASSERT(*pCurrentUpdateTimes >= *pSuccessUpdateTimes);

	const SHuiYuanDanInfo* pInfo = CItemService::GetInstance().GetHuiYuanDanInfo(*pSuccessUpdateTimes);
	if (!pInfo)
	{
		return FALSE;
	}
	MY_ASSERT(pInfo);

	++(*pCurrentUpdateTimes);	

	WORD rate = pInfo->wSuccessRate;

	if (pPlayer->m_LJS_Metrial)
		rate += pInfo->wLianzhiRate;

	if (pPlayer->m_XMTJ_Metrial)
		rate += pInfo->wXMTJRate;

	WORD RandRate = CRandom::RandRange(1, 10000);
	
	// �ɹ�
	if (RandRate <= rate)
	{
		// �ɹ��Ժ�pSpValue�ͷţ�����ʹ��
		delete pSPValue;

		const SHuiYuanDanInfo* pInfo = CItemService::GetInstance().GetHuiYuanDanInfo(++(*pSuccessUpdateTimes));
		if (!pInfo)
		{
			return FALSE;
		}
		MY_ASSERT(pInfo);

		// �ɹ���Ӹߵȼ��Ļ���
		SRawItemBuffer item;
		GenerateNewItem(item, SAddItemInfo(pPlayer->m_UpdateItem->wIndex + 1, 1));

		// ��������������Ϣ
		BYTE *pItemBuf = item.buffer;
		if (!pItemBuf)
		{
			rfalse(4, 1, "Itemuser.cpp - !pItembuff");
			return FALSE;
		}

		memcpy(pItemBuf, pCurrentUpdateTimes, sizeof(BYTE));
		pItemBuf += sizeof(BYTE);
		memcpy(pItemBuf, pSuccessUpdateTimes, sizeof(BYTE));
		pItemBuf += sizeof(BYTE);
		memcpy(pItemBuf, &pInfo->wSPValue, sizeof(WORD));

		// ɾ����ǰ�ĵ���
		DelItem(*pPlayer->m_UpdateItem, "[���������ɹ���ɾ��ԭ����]", true);
		pPlayer->m_UpdateItem = 0;

		// ����µ���
		WORD pos = FindBlankPos(XYD_PT_BASE);
		AddExistingItem(item, pos, true);

		// ������װ��������
		pPlayer->m_UpdateItem = FindItemByPos(pos, XYD_FT_ONLYUNLOCK);
		LockItemCell(pos, true);

		msg.wCellPos = pos;
		msg.bType = SABeginUpdateItem::BUI_SUCCESS;
	}
	else
	{
		memcpy(pPlayer->m_UpdateItem->buffer, pCurrentUpdateTimes, sizeof(BYTE));
		// ʧ�ܣ��ȷ��͵��߸���
		pPlayer->SendItemSynMsg(pPlayer->m_UpdateItem, XYD_FT_ONLYLOCK);

		// Ȼ����ʧ����Ϣ
		msg.wCellPos = pPlayer->m_UpdateItem->wCellPos;		
		msg.bType = SABeginUpdateItem::BUI_FAILED;
	}

	// �ۼ�����
	if (pPlayer->m_LJS_Metrial)
	{
		if( pPlayer->m_LJS_Metrial->overlap > 1)
		{
			pPlayer->m_LJS_Metrial->overlap--;
			pPlayer->SendItemSynMsg(pPlayer->m_LJS_Metrial, XYD_FT_ONLYLOCK);
		}
		else
		{
			DelItem(*pPlayer->m_LJS_Metrial, "�����������ģ�", true);
			pPlayer->m_LJS_Metrial = 0;
		}
	}

	if (pPlayer->m_XMTJ_Metrial)
	{
		if( pPlayer->m_XMTJ_Metrial->overlap > 1)
		{
			pPlayer->m_XMTJ_Metrial->overlap--;
			pPlayer->SendItemSynMsg(pPlayer->m_XMTJ_Metrial, XYD_FT_ONLYLOCK);
		}
		else
		{
			DelItem(*pPlayer->m_XMTJ_Metrial, "�����������ģ�", true);
			pPlayer->m_XMTJ_Metrial = 0;
		}
	}

	msg.bSuccessTimes = *pSuccessUpdateTimes;
	msg.bTotalUpdateTimes = *pCurrentUpdateTimes;

	g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginUpdateItem));

	// �ͷ�ָ��
	delete pSuccessUpdateTimes;
	delete pCurrentUpdateTimes;
	
	pSuccessUpdateTimes = 0;
	pCurrentUpdateTimes = 0;

	return TRUE;
}

// �ͻ���������������װ�����Ƿ񼤻�
BOOL CItemUser::RecvGetSuitAttributes(struct SQSuitAttributeMsg *pMsg)
{	
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvGetSuitAttributes() - !pMsg");
		return FALSE;
	}

	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	SASuitAttributeMsg msg;
	msg.bSuitAttriState = pPlayer->m_ModifyTypeMark;

	g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SASuitAttributeMsg));

	return TRUE;
}

long CItemUser::IsHaveSpaceCell()
{
	long baseCell = GetBlankCell(XYD_PT_BASE);
	return baseCell;
}
long CItemUser::CreateNewItemInfo(long index,long Flag,SPackageItem &Item,long FLAGX)
{
	if (Flag == 1)
	{
		GenerateNewItem(Item,SAddItemInfo(index,1));
		Item.wCellPos = 0;
	}
	else if (Flag == 2)//Ҫ���⴦�������
	{
		const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(index);
		if (!pData)
		{
			return 0;
		}
		MY_ASSERT(pData);
	
		GenerateNewItem(Item,SAddItemInfo(index,1));

		WORD pos = FindBlankPos(XYD_PT_BASE);

		InSertBlessItem(Item,pos);
		SCellPos actPos;
		actPos.wCellPos = pos;

		Item.wCellPos = actPos.wCellPos;

		LockItemCellTwo(actPos.wCellPos,true);

		CPlayer *pPlayer = static_cast<CPlayer *>(this);
		if (!pPlayer)
			return 0;

		if (pPlayer->m_Property.m_BlessOnce == 0)
		{
			pPlayer->m_Property.m_BlessOnce = 1;
			pPlayer->m_Property.m_BlessSPackageitem = Item;
		}	
	}
	return 1;
}

void CItemUser::InSertBlessItem(const SItemBase &item, WORD pos)
{	
	ValidationPos(pos);

	if (!item.wIndex)
	{
		rfalse(4, 1, "Itemuser.cpp - InsertBlessItem() - item.windex = %d", item.wIndex);
		return;
	}
	MY_ASSERT(item.wIndex);

	const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(item.wIndex);
	if (!itemData)
	{
		return;
	}
	MY_ASSERT(itemData);

	if (item.overlap > itemData->m_Overlay)
	{
		return;
	}
	MY_ASSERT(item.overlap <= itemData->m_Overlay);

	if (!FillItemPos(pos, SItemWithFlag::IWF_ITEMIN, true))
	{
		return;
		MY_ASSERT(0);
	}
	
	// ��ӵ���
	SCellPos &actPos = (SCellPos &)m_PackageItems[pos].m_Item;
	actPos.wCellPos = pos;

	memcpy(&(SItemBase&)m_PackageItems[pos].m_Item, &item, sizeof(SRawItemBuffer));

	if (m_PackageItems[pos].m_Item.overlap < itemData->m_Overlay)		// �������Ÿ���
	{
		//m_ItemsState[item.wIndex] += (itemData->m_Overlay - item.overlap);
		//rfalse(2, 1, "�½���IDΪ%d�ĵ���Ŀǰ�����ٵ���%d��", item.wIndex, m_ItemsState[item.wIndex]);
	}

	ValidationPos(pos);
}

BOOL CItemUser::RecvBlessQuestAddItems(struct SQBlessQuestAddItems* pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvBlessQuestAddItems() - !pMsg");
		return FALSE;
	}

	CPlayer* pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	if (pPlayer->m_Property.m_BlessOnce == 1)
	{
		OnAllNotice();
		pPlayer->LockItemCellTwo(pPlayer->m_Property.m_BlessSPackageitem.wCellPos,false);
		pPlayer->m_Property.m_BlessOnce = 0;
		SABlessQuestAddItems msg;
		msg.Item = pPlayer->m_Property.m_BlessSPackageitem;
		g_StoreMessage(pPlayer->m_ClientIndex,&msg,sizeof(SABlessQuestAddItems));
	}
	
	return TRUE;
}

void CItemUser::OnAllNotice()
{
	CPlayer* pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;

	if (pPlayer->m_Property.m_HightiTEM == 1)
	{
		g_Script.SetCondition(0,pPlayer,0);
		LuaFunctor(g_Script,"LuaProcessNotice")();
		g_Script.CleanCondition();
	}
}

BOOL CItemUser::LockItemCellTwo(WORD wCellPos, bool lock, bool sync /* = true */ )
{
	WORD linePos = wCellPos;
	SPackageItem *pItem = FindItemByPos(linePos, lock ? XYD_FT_ONLYUNLOCK : XYD_FT_ONLYLOCK);
	if (!pItem)
	{
		return FALSE;
	}
	MY_ASSERT(pItem);

	if (!FillItemPos(wCellPos, SItemWithFlag::IWF_LOCKED, lock))
	{
		return FALSE;
		MY_ASSERT(0);
	}

	return TRUE;
}

BOOL CItemUser::RecvChannelCallSellInfoQuest(struct SQChannelCallSellInfo* pMsg)
{
	if (!pMsg)
	{
		rfalse("SQChannelCallSellInfo* pMsg Null");
		return FALSE;
	}
	CPlayer* pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
	{
		rfalse("SQChannelCallSellInfo* pMsg Null Player");
		return FALSE;
	}
	
	float Money = pMsg->AllPrice;
	if (Money < 10) // С��ʮ��������
	{
		
		return FALSE;
	}
	Money *= 0.1f; 
	Money = ceil(Money);
	if (pMsg->NoticeRoll)
	{
		Money += 100.0f;
	}
	
	if (pPlayer->m_Property.m_BindMoney < Money)//������Ǯ
	{
		return FALSE;
	}
	
	WORD linePos = pMsg->wCellPos;

	if (linePos > PackageAllCells)
		return FALSE;

	ValidationPos(linePos);

	// ����δ����
	if (0 == (m_PackageItems[linePos].m_State & SItemWithFlag::IWF_ACTIVED))
		return FALSE;

	// �����޵���
	//if (!canBeEmpty && (0 == (m_PackageItems[linePos].m_State & SItemWithFlag::IWF_ITEMIN)))
	//	return FALSE;

	//// ���ӱ�����
	//if (!canBeLock && m_PackageItems[linePos].m_State & SItemWithFlag::IWF_LOCKED)
	//	return FALSE;



	return TRUE;
}

BOOL CItemUser::OnPickupItem(CSingleItem *pItem )
{
	if (!pItem)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvPickupItem(SQPickupItemMsg *msg) - !msg");
		return FALSE;
	}

	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

// 	if (!pPlayer->m_pSingleItem )
// 		return FALSE;

	// �����߱���
	if (!pItem->m_dwTeamID && pItem->m_dwGID!=pPlayer->GetGID())
	{
		//TalkToDnid(pPlayer->m_ClientIndex,"��������ģ��ټ��ִ�ϣ�",0);
		return FALSE;
	}

	// �����߱���
	if (pItem->m_dwTeamID)
	{
		if (pItem->m_dwTeamID != pPlayer->m_dwTeamID)		// ûȨ��
			return FALSE;

		std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pItem->m_dwTeamID);
		if (iter == teamManagerMap.end())
			return FALSE;

		extern BOOL AddItemToTeamPack(CPlayer *pPlayer, CSingleItem *cpItem);

			switch (pItem->m_PickStatus)
			{
			case PIPT_FREE:
				return PickSingleItem(*pItem);
				break;
			case PIPT_LEADER:
				return AddItemToTeamPack(pPlayer, pItem);
				break;
			default:
				break;
			}
		return TRUE;
	}
	return	PickSingleItem(*pItem);
}

BOOL CItemUser::PickSingleItem( CSingleItem &SingleItem )
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)return FALSE;

	const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(SingleItem.m_GroundItem.wIndex);

	if (!pData)
	{
		rfalse(4, 1, "[ItemUser] - PickOneItem()");
		return FALSE;
	}

	int itemismoney = pPlayer->_L_GetLuaValue("OnItemIsMoney",pData->m_ID,0); //�ж��Ƿ�Ϊ��Ǯ����
	if (itemismoney)
	{
		pPlayer->AddPlayerMoney(XYD_UM_ONLYBIND, SingleItem.m_ItemsNum);
		/*pPlayer->AddPlayerMoney(XYD_UM_ONLYUNBIND, SingleItem.m_ItemsNum);*/
		SingleItem.RemoveMe();
		return TRUE;
	}

	std::list<SAddItemInfo> itemList;
	itemList.push_back(SAddItemInfo(SingleItem.m_GroundItem.wIndex, SingleItem.m_GroundItem.overlap));

	if (itemList.size() && !CanAddItems(itemList))
	{
		TalkToDnid(pPlayer->m_ClientIndex, "�����������޷�ʰȡ��");
		return FALSE;
	}

	WORD willCreate = TryPossibleMerge(SingleItem.m_GroundItem.wIndex, SingleItem.m_GroundItem.overlap, pData->m_Overlay);
	InsertItem(SingleItem.m_GroundItem.wIndex, willCreate, pData, SingleItem.m_GroundItem, false);

	SingleItem.RemoveMe();

	return TRUE;
}
BOOL CItemUser::SendNeedMETRIALInfo(CPlayer *pPlayer,BYTE  bType)
{
	if (!pPlayer)
	{
		return FALSE;
	}	
	SANeedMETRIAL msg;	
	BYTE	bySuccessNum = 0;
	
	for (int i = 0; i < pPlayer->m_vecStrengthenEquip.size(); ++i)
	{		
		SEquipment *pEquip = GetEquipmentbyItem(pPlayer->m_vecStrengthenEquip.at(i), XYD_FT_WHATEVER);
		if (CEquipStrengthenServer::handleElemNeedMETRIALInfo(pPlayer, pEquip, bType, msg))
		{
			++bySuccessNum;
		}
	}		

	if (bySuccessNum > 0)
	{
		g_StoreMessage(pPlayer->m_ClientIndex,&msg,sizeof(msg));
	}
	else 
	{
		return FALSE;
	}
	
	return TRUE;
	
}

long CItemUser::OnGetNeedMetrial(CPlayer* pPlayer,DWORD NeedNum,std::vector<SPackageItem*> & vec,long ItemID, WORD LookType/* = XYD_FT_ONLYUNLOCK*/)//�ӱ����еõ����ʵĵ��߼���
{
	if (!pPlayer)
	{
		return 2;
	}
	WORD PracticNum = 0;
	for (DWORD i = BaseStart; i < BaseEnd; i++)
	{
		long temp = m_PackageItems[i].m_State;
		BYTE c1 = m_PackageItems[i].m_State & SItemWithFlag::IWF_ACTIVED;
		BYTE c2 = m_PackageItems[i].m_State & SItemWithFlag::IWF_ITEMIN;
		BYTE c3 = m_PackageItems[i].m_State & SItemWithFlag::IWF_LOCKED;
		if (  (c1 != 0 ) && (c2 != 0) && (c3 == 0))//�˸����Ǽ���(����)�������е���(����)û�б�����
		{
			if (m_PackageItems[i].m_Item.wIndex == ItemID)
			{
				PracticNum += m_PackageItems[i].m_Item.overlap;
				vec.push_back(&m_PackageItems[i].m_Item);
				if (PracticNum >= NeedNum)
				{
					break;
				}
			}
		}	

	}
	if (PracticNum >= NeedNum)
	{
		return 1;//˵������������������
	}
	else
	{
		return 2;
	}
	return 2;
}
// [Func:�۳�����Ҫ�Ĳ���]
// [pPlayer-�۳�˭���ϵĲ���]
// [vec-�Ӹõ����б�۳�]
// [NeedNum-��Ҫ�۳�������]
// [str-��ʾ��Ϣ]
// [bHaveBindMetrial-��ȡ�Ƿ��а󶨵Ĳ���]
long CItemUser::OnProcessNeedMetrial(CPlayer* pPlayer,std::vector<SPackageItem*> vec,long NeedNum,std::string str,bool& bHaveBindMetrial)
{
	if (!pPlayer)
	{
		return 0;
	}
	bHaveBindMetrial = false;
	long size = vec.size();
	long TempNum = NeedNum;
	for (int i = 0; i < size; ++i)
	{
		SPackageItem * item = vec[i];
		if (item)
		{
			// [�˴����ο۳����ϣ�û���ȿ۰�]
			if (!bHaveBindMetrial)
			{
				bHaveBindMetrial = ((item->flags & (SItemBase::F_SYSBINDED | SItemBase::F_PLAYERBINDED))!=0);
			}			
			if (item->overlap > TempNum)
			{
				// [2012-8-21 19-27 gw: +����Ʒ��ʹ��changeoverlapͳһ����]
				ChangeOverlap(item, TempNum, false, true);
				// [2012-8-21 19-27 gw: -ʹ������ķ�ʽû�ж�m_ItemsState�Ĵ���]
				//item->overlap -= TempNum;
				//pPlayer->SendItemSynMsg(item, XYD_FT_ONLYUNLOCK);
			}
			else if (item->overlap <= TempNum)
			{	
				TempNum -= item->overlap;
				DelItem(*item, str.c_str(), false);
				if (TempNum == 0)
				{
					break;
				}
			}
		}
		
	}
	return 1;
}
void CItemUser::OnthSysNotice(WORD index)
{
	CPlayer * pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		g_Script.SetCondition(NULL,pPlayer,NULL);
		LuaFunctor(g_Script,"OnthSysNotice")[index]();
		g_Script.CleanCondition();
	}
}

bool CItemUser::GetEquipment( BYTE btype,BYTE bIndex,SEquipment &equip )
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return false;

	if(btype == 0) //װ����
	{
		if (bIndex >= 0 && bIndex < EQUIP_P_MAX)
		{
			memcpy(&equip,&pPlayer->m_Property.m_Equip[bIndex],sizeof(SEquipment));
			return true;
		}
	}
	else if (btype == 1) //����
	{
		SPackageItem *srcItem = pPlayer->FindItemByPos(bIndex, XYD_FT_WHATEVER);
		if (!srcItem)return false;
	
		const SItemBaseData *srcData = CItemService::GetInstance().GetItemBaseData(srcItem->wIndex);
		if (!srcData) 
		{
			return false;
		}

		if (!ITEM_IS_EQUIPMENT(srcData->m_Type))
		{
			return false;
		}
		memcpy(&equip,srcItem->buffer,sizeof(SEquipment));
		return true;
	}
	return false;
}

void CItemUser::OnGetIntensifyInfo( struct SQIntensifyInfo *pintensifyinfo )
{
	if (!pintensifyinfo)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return ;

	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnGetIntensifyInfo"))
	{
		g_Script.PushParameter(pintensifyinfo->bType);
		g_Script.PushParameter(pintensifyinfo->bIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CItemUser::SendIntensifyInfo(const struct SAIntensifyInfo *pIntensifyInfo)
{
	if (!pIntensifyInfo)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return ;

	g_StoreMessage(pPlayer->m_ClientIndex, (LPVOID)pIntensifyInfo, sizeof(SAIntensifyInfo));
}

bool CItemUser::SetEquipment( BYTE btype,BYTE bIndex,const SEquipment *sequip )
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return false;

	if (!sequip)return false;

	if (sequip->wIndex == 0)
	{
		rfalse(2, 1, "SetEquipment Index == 0");
		return false;
	}

	if (btype == 0)
	{
		if (bIndex >= 0 && bIndex < EQUIP_P_MAX)
		{
			//sequip.overlap = pPlayer->m_Property.m_Equip[bIndex].overlap;

			memcpy(&pPlayer->m_Property.m_Equip[bIndex],sequip,sizeof(SEquipment));

			// ��װ����Ϣ���͸��ͻ���
			SAEquipInfoMsg msg;
			int n = sizeof(msg.stEquip);
			msg.stEquip = pPlayer->m_Property.m_Equip[bIndex];
			msg.byPos = bIndex;
			g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAEquipInfoMsg));

			// ���¼���װ���ӳ�����
			pPlayer->InitEquipmentData();
			UpdateAllProperties();
			return true;
		}
	}
	else if (btype == 1)
	{
		SPackageItem *srcItem = pPlayer->FindItemByPos(bIndex, XYD_FT_WHATEVER);
		if (!srcItem)return false;

		if (DelItem(*srcItem))
		{
			WORD pos = FindBlankPos(XYD_PT_BASE);
			if (0xffff == pos)
			{
				return false;
			}

			SRawItemBuffer itemBuffer;
			memset(&itemBuffer, 0, sizeof(itemBuffer));
			extern BOOL GenerateNewUniqueId(SItemBase &item);
			if (!GenerateNewUniqueId(itemBuffer))
				return false;

			SItemBase *item = &itemBuffer;
			item->wIndex = sequip->wIndex; //
			item->usetimes	= 1;
			item->flags		= 0;
			item->overlap	= 1;

			memcpy(&itemBuffer, sequip, sizeof(SEquipment));
			AddExistingItem(itemBuffer,pos,TRUE);
			return true;
		}
	}
	return false;
}

BOOL CItemUser::SendIntensifyResult( struct SABeginEquipLevelUpMsg *pMsg )
{
	if (!pMsg)
	{
		return FALSE;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return FALSE;

	g_StoreMessage(pPlayer->m_ClientIndex,pMsg,sizeof(SABeginEquipLevelUpMsg));
	return TRUE;
}

BOOL CItemUser::OnQuestEquipSwitch( struct SQEquipSwitch* pMsg )
{
	if (!pMsg)
	{
		return FALSE;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return FALSE;
	SAEquipSwitch squipswitchmsg;
	squipswitchmsg.bresule = 1;
	if (IsLockedEquipColumnCell(pMsg->bIndex) || IsLockedEquipColumnCell(pMsg->bIndex2))
	{
		g_StoreMessage(pPlayer->m_ClientIndex,&squipswitchmsg,sizeof(SAEquipSwitch));
		return FALSE;
	}
	SEquipment equipt[2];
	memset(&equipt[0], 0, sizeof(SEquipment));
	memset(&equipt[1], 0, sizeof(SEquipment));
	if (GetEquipment(0,pMsg->bIndex,equipt[0]) && GetEquipment(0,pMsg->bIndex2,equipt[1]))
	{
		if (SetEquipment(0,pMsg->bIndex,&equipt[1]) && SetEquipment(0,pMsg->bIndex2,&equipt[0]) )
		{

			// ��װ����Ϣ���͸��ͻ���
			SAEquipInfoMsg msg;
			msg.stEquip = pPlayer->m_Property.m_Equip[pMsg->bIndex];
			msg.byPos = pMsg->bIndex;
			g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAEquipInfoMsg));

			msg.stEquip = pPlayer->m_Property.m_Equip[pMsg->bIndex2];
			msg.byPos = pMsg->bIndex2;
			g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAEquipInfoMsg));


			// ���¼���װ���ӳ�����
			pPlayer->InitEquipmentData();
			UpdateAllProperties();

			squipswitchmsg.bresule = 0;
			g_StoreMessage(pPlayer->m_ClientIndex,&squipswitchmsg,sizeof(SAEquipSwitch));
			return TRUE;
		}
	}
	return FALSE;
}

void CItemUser::OnGetQualityInfo(const struct SQEquipQualityInfo *pQualityInfo)
{
	if (!pQualityInfo)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;

	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnGetQualityInfo"))
	{
		g_Script.PushParameter(pQualityInfo->bType);
		g_Script.PushParameter(pQualityInfo->bIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CItemUser::SendQualityInfo(const struct SAEquipQualityInfo *pQualityInfo, WORD msgsize)
{
	if (!pQualityInfo)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;

	g_StoreMessage(pPlayer->m_ClientIndex, (LPVOID)pQualityInfo, msgsize);
}

BOOL CItemUser::DeleteItem(DWORD ItemID, WORD num)
{
	for (DWORD i = 0; i < PackageAllCells; i++)
	{
		SPackageItem *pItem = FindItemByPos(i, XYD_FT_WHATEVER);
		if (pItem && pItem->wIndex == ItemID)
		{
			if (num >= pItem->overlap)
			{
				num -= pItem->overlap;
				DelItem(*pItem);
			}  
			else
			{
				ChangeOverlap(pItem, num, false);
				num = 0;
			}

			if (num == 0)
			{
				break;
			}
		}
	}
	return (BOOL)(num == 0);
}

int CItemUser::AddItem(SAddItemInfo &siteminfo)
{
	const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(siteminfo.m_Index);
	if (!pData)
	{
		rfalse(4, 1, "[ItemUser.cpp] - StarAddItems() - !pData");
		return -1;
	}
	MY_ASSERT(pData);
	// ��ִ�п��ܵĺϲ�
	WORD willCreate = TryPossibleMerge(siteminfo.m_Index, siteminfo.m_Count, pData->m_Overlay);

	SRawItemBuffer itemBuffer;
	return InsertItem(siteminfo.m_Index, willCreate, pData, itemBuffer, true);
}

BOOL CItemUser::SendQualityResult(struct SAStartQuality *pMsg)
{
	if (!pMsg)
	{
		return FALSE;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return FALSE;

	g_StoreMessage(pPlayer->m_ClientIndex, pMsg, sizeof(SAStartQuality));
	return TRUE;
}

void CItemUser::OnRecvSaleItem(struct SQItemSale *pMsg)
{
	if (!pMsg)
	{
		return ;
	}
	SAItemSale saleresule;
	saleresule.bResult = 1;
	if (!IsItemPosValid(pMsg->bIndex))
	{
		return;
	}

	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return ;
	SPackageItem *pItem = FindItemByPos(pMsg->bIndex, XYD_FT_WHATEVER);
	if (pItem&&pItem->wIndex>0)
	{
		const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(pItem->wIndex);
		if (!pData)return ;

		DWORD sellMoney = pData->m_SellPrice * pItem->overlap;
			
		if (pPlayer->DelItem(*pItem))
		{
			pPlayer->AddPlayerMoney(pData->m_SellMoneyType, sellMoney);
			saleresule.bResult = 0;
		}
	}
	g_StoreMessage(pPlayer->m_ClientIndex, &saleresule, sizeof(SAItemSale));
}

void CItemUser::OnRecvCleanColdTime(struct SQCleanColdtime *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvBeginUpgradeLevel() - !pMsg");
		return ;
	}
	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return ;

	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("clearcoldtime"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CItemUser::SendCleanColdTimeResult(struct SACleanColdtime *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "Itemuser.cpp - RecvBeginUpgradeLevel() - !pMsg");
		return ;
	}
	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return ;
	g_StoreMessage(pPlayer->m_ClientIndex, pMsg, sizeof(SACleanColdtime));
}

void CItemUser::OnGetRiseStarInfo(const struct SQRiseStarInfo *pRiseStarInfo)
{
	if (!pRiseStarInfo)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;

	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnGetRefineStarInfo"))
	{
		g_Script.PushParameter(pRiseStarInfo->bType);
		g_Script.PushParameter(pRiseStarInfo->bIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CItemUser::SendRiseStarInfo(const struct SARiseStarInfo *pRiseStarInfo, WORD msgsize)
{
	if (!pRiseStarInfo)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;

	g_StoreMessage(pPlayer->m_ClientIndex, (LPVOID)pRiseStarInfo, msgsize);
}

void CItemUser::SendRefineStarResult(const struct SABeginRefineStar *pRefineStar, WORD msgsize)
{
	if (!pRefineStar)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;

	g_StoreMessage(pPlayer->m_ClientIndex, (LPVOID)pRefineStar, msgsize);
}

void CItemUser::SendMsgToPlayer(const LPVOID strarresule, WORD msgsize)
{
	if (!strarresule)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;
	g_StoreMessage(pPlayer->m_ClientIndex, strarresule, msgsize);
}

void CItemUser::OnGetEquiptKeYinInfo(const struct SQKeYinInfo *pKeYinInfo)
{
	if (!pKeYinInfo)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;

	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnGetEquiptKeYinInfo"))
	{
		g_Script.PushParameter(pKeYinInfo->bType);
		g_Script.PushParameter(pKeYinInfo->bIndex);
		g_Script.PushParameter(pKeYinInfo->bPropertyIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CItemUser::SendEquiptKeYinInfo(const struct SAKeYinInfo *pKeYinInfo, WORD msgsize)
{
	SendMsgToPlayer((LPVOID)pKeYinInfo, msgsize);
}

void CItemUser::OnBeginKeYin(const struct SQBeginKeYin *pBeginKeYinInfo)
{
	if (!pBeginKeYinInfo)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;

	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnBeginKeYin"))
	{
		g_Script.PushParameter(pBeginKeYinInfo->bType);
		g_Script.PushParameter(pBeginKeYinInfo->bIndex);
		g_Script.PushParameter(pBeginKeYinInfo->bPropertyIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CItemUser::OnBeginKeYinChange(const struct SQKeYinChange *pBeginKeYinChange)
{
	if (!pBeginKeYinChange)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;

	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnBeginKeYinChange"))
	{
		g_Script.PushParameter(pBeginKeYinChange->bType);
		g_Script.PushParameter(pBeginKeYinChange->bIndex);
		g_Script.PushParameter(pBeginKeYinChange->bChangeType);
		g_Script.PushParameter(pBeginKeYinChange->bPropertyIndex);
		g_Script.PushParameter(pBeginKeYinChange->bTargetIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CItemUser::OnBeginEquipJDing(const struct SQEquipJDing* pEquipJDing)
{
	if (!pEquipJDing)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;

	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnBeginJDing"))
	{
		g_Script.PushParameter(pEquipJDing->bType);
		g_Script.PushParameter(pEquipJDing->bIndex);
		g_Script.PushParameter(pEquipJDing->bPropertyIndex);
		g_Script.PushParameter(pEquipJDing->bIslock);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CItemUser::SendEquipJDingInfo(const struct SAEquipJDingInfo *pEquipJDingInfo, WORD msgsize)
{
	if (!pEquipJDingInfo)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;

	g_StoreMessage(pPlayer->m_ClientIndex, (LPVOID)pEquipJDingInfo, msgsize);
}

void CItemUser::SendEquipJDingResult(const struct SAEquipJDing *pEquipJDing, WORD msgsize)
{
	if (!pEquipJDing)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;

	g_StoreMessage(pPlayer->m_ClientIndex, (LPVOID)pEquipJDing, msgsize);
}

void CItemUser::OnGetEquipJDingInfo(const struct SQEquipJDingInfo* pEquipJDingInfo)
{
	if (!pEquipJDingInfo)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;

	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnGetEquipJDingInfo"))
	{
		g_Script.PushParameter(pEquipJDingInfo->bType);
		g_Script.PushParameter(pEquipJDingInfo->bIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CItemUser::SendEquipJDingColor(const struct SAEquipJDingColor *pEquipJDingColor, WORD msgsize)
{
	if (!pEquipJDingColor)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;

	g_StoreMessage(pPlayer->m_ClientIndex, (LPVOID)pEquipJDingColor, msgsize);
}

int getfield(const char* Str)
{
	int result;
	lua_pushstring(g_Script.ls, Str);
	lua_gettable(g_Script.ls, -2);  /* get background[key] */
	result = (int)lua_tonumber(g_Script.ls, -1);
	lua_pop(g_Script.ls, 1);  /* remove number */
	return result;
}

int getTableData(const char* Str, int Index)
{
	int result;
	lua_pushstring(g_Script.ls, Str);
	lua_gettable(g_Script.ls, -2);  /* get background[key] */
	lua_rawgeti(g_Script.ls, -1, Index);
	result = (int)lua_tonumber(g_Script.ls, -1);
	lua_pop(g_Script.ls, 2);  /* remove number */
	return result;
}

void CItemUser::OnReturnQiDaoMsg(struct SQQiDaoInfo *pMsg)	//������������Ϣ
{
	if (!pMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;

	g_Script.SetPlayer(pPlayer);
	//ִ��lua��������������
	SAQiDaoInfo QiDaoMsg;
	//��ӽ��ղ����Ĵ������
	time_t dwCurTime;
	time(&dwCurTime);
	tm * pCurT = localtime(&dwCurTime);
	tm CurTime, PreTime;
	memcpy(&CurTime, pCurT, sizeof(tm));
	tm *pPreT = localtime(&pPlayer->m_Property.m_dwPreReqTime);
	memcpy(&PreTime, pPreT, sizeof(tm));
	if (CurTime.tm_year > PreTime.tm_year || CurTime.tm_yday > PreTime.tm_yday)		//24�����ù���
	{
		pPlayer->m_Property.m_dwPreReqTime = 0;
		pPlayer->m_Property.m_bySilerCoinUsedNum = 0;
		pPlayer->m_Property.m_byAnimaUsedNum = 0;
		pPlayer->m_Property.m_byExpUsedNum = 0;
	}
	pPlayer->m_Property.m_dwPreReqTime = dwCurTime;

	lua_getglobal(g_Script.ls, "ReturnCurVipQiDaoInfo");
	lua_pushnumber(g_Script.ls, pPlayer->m_Property.m_bVipLevel + 1);
	if (lua_pcall(g_Script.ls, 1, 1, 0) != 0)
	{
		const char* ss = lua_tostring(g_Script.ls, -1);
		lua_pop(g_Script.ls, 1);
		return;
	}
	QiDaoMsg.wSilerCoinUsedNum = pPlayer->m_Property.m_bySilerCoinUsedNum;
	QiDaoMsg.wAnimaUsedNum = pPlayer->m_Property.m_byAnimaUsedNum;
	QiDaoMsg.wExpUsedNum = pPlayer->m_Property.m_byExpUsedNum;
	QiDaoMsg.wSilerCoinGetCount = getfield("SilerCoinGetCount");
	QiDaoMsg.wSilerCoinPrice = getTableData("SilerCoinPrice", pPlayer->m_Property.m_bySilerCoinUsedNum + 1);
	QiDaoMsg.wSilerCoinCanNum = getfield("SilerCoinCanNum");
	QiDaoMsg.wAnimaGetCount = getfield("AnimaGetCount");
	QiDaoMsg.wAnimaPrice = getTableData("AnimaPrice", pPlayer->m_Property.m_byAnimaUsedNum + 1);
	QiDaoMsg.wAnimaCanNum = getfield("AnimaCanNum");
	QiDaoMsg.wExpGetCount = getfield("ExpGetCount");
	QiDaoMsg.wExpPrice = getTableData("ExpPrice", pPlayer->m_Property.m_byExpUsedNum + 1);
	QiDaoMsg.wExpCanNum = getfield("ExpCanNum");
	QiDaoMsg.wNextVipCanNum = getfield("NextVipCanNum");
	lua_pop(g_Script.ls, 1);

	lua_getglobal(g_Script.ls, "ReturnGetCoin");
	lua_pushnumber(g_Script.ls, QiDaoMsg.wSilerCoinGetCount);
	if (lua_pcall(g_Script.ls, 1, 1, 0) != 0)
	{
		const char* ss = lua_tostring(g_Script.ls, -1);
		lua_pop(g_Script.ls, 1);
		return;
	}
	QiDaoMsg.wSilerCoinGetCount = static_cast<WORD>(lua_tonumber(g_Script.ls, -1));
	lua_pop(g_Script.ls, 1);

	lua_getglobal(g_Script.ls, "ReturnGetSp");
	lua_pushnumber(g_Script.ls, QiDaoMsg.wAnimaGetCount);
	if (lua_pcall(g_Script.ls, 1, 1, 0) != 0)
	{
		lua_pop(g_Script.ls, 1);
		return;
	}
	QiDaoMsg.wAnimaGetCount = static_cast<WORD>(lua_tonumber(g_Script.ls, -1));
	lua_pop(g_Script.ls, 1);

	lua_getglobal(g_Script.ls, "ReturnGetExp");
	lua_pushnumber(g_Script.ls, QiDaoMsg.wExpGetCount);
	if (lua_pcall(g_Script.ls, 1, 1, 0) != 0)
	{
		lua_pop(g_Script.ls, 1);
		return;
	}
	QiDaoMsg.wExpGetCount = static_cast<WORD>(lua_tonumber(g_Script.ls, -1));
	lua_pop(g_Script.ls, 1);

	//lua_getglobal(g_Script.ls, "ReturnCurVipQiDaoInfo");
	//lua_pushstring(g_Script.ls, pPlayer->GetAccount());
	//lua_pushnumber(g_Script.ls, pPlayer->m_Property.m_bVipLevel + 1);
	//if (lua_pcall(g_Script.ls, 2, 4, 0) != 0)
	//	const char* ss = lua_tostring(g_Script.ls, -1);
	//QiDaoMsg.wSilerCoinUsedNum = lua_tonumber(g_Script.ls, -4);
	//QiDaoMsg.wAnimaUsedNum = lua_tonumber(g_Script.ls, -3);
	//QiDaoMsg.wExpUsedNum = lua_tonumber(g_Script.ls, -2);
	//QiDaoMsg.wSilerCoinGetCount = getfield("SilerCoinGetCount");
	//QiDaoMsg.wSilerCoinPrice = getfield("SilerCoinPrice");
	//QiDaoMsg.wSilerCoinCanNum = getfield("SilerCoinCanNum");
	//QiDaoMsg.wAnimaGetCount = getfield("AnimaGetCount");
	//QiDaoMsg.wAnimaPrice = getfield("AnimaPrice");
	//QiDaoMsg.wAnimaCanNum = getfield("AnimaCanNum");
	//QiDaoMsg.wExpGetCount = getfield("ExpGetCount");
	//QiDaoMsg.wExpPrice = getfield("ExpPrice");
	//QiDaoMsg.wExpCanNum = getfield("ExpCanNum");
	//QiDaoMsg.wNextVipCanNum = getfield("NextVipCanNum");
	g_Script.CleanPlayer();
	g_StoreMessage(pPlayer->m_ClientIndex, &QiDaoMsg, sizeof(SAQiDaoInfo));
}

void CItemUser::OnAddQiDaoAndReturn(struct SQAddQiDaoInfo *pMsg)	//���տͻ�����ȡ����Ϣ����Ӧ�Ĵ������ؽ��
{
	if (!pMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;

	SAAddQiDaoInfo AddQiDaoMsg;
	//ִ��lua��������������
	g_Script.SetPlayer(pPlayer);
	//��ӽ��ղ����Ĵ������
	lua_getglobal(g_Script.ls, "AddOneQiDao");
	lua_pushnumber(g_Script.ls, pPlayer->m_Property.m_bVipLevel + 1);
	lua_pushnumber(g_Script.ls, pMsg->bQiDaoType);
	if (pMsg->bQiDaoType == 0)
		lua_pushnumber(g_Script.ls, pPlayer->m_Property.m_bySilerCoinUsedNum);
	else if (pMsg->bQiDaoType == 1)
		lua_pushnumber(g_Script.ls, pPlayer->m_Property.m_byAnimaUsedNum);
	else
		lua_pushnumber(g_Script.ls, pPlayer->m_Property.m_byExpUsedNum);
	if (lua_pcall(g_Script.ls, 3, 2, 0) != 0)
	{
		const char* ss = lua_tostring(g_Script.ls, -1);
		lua_pop(g_Script.ls, 1);
		return;
	}
	AddQiDaoMsg.cOptResult = lua_tonumber(g_Script.ls, -2);

	//��������
	if (AddQiDaoMsg.cOptResult == 2)
	{
		//������������͵�����
		if (pMsg->bQiDaoType == 0)		//��������
		{
			pPlayer->ChangeMoney(2, lua_tonumber(g_Script.ls, -1));		//��������
			pPlayer->m_Property.m_bySilerCoinUsedNum++;
		}
		else if (pMsg->bQiDaoType == 1)		//��������
		{
			pPlayer->m_Property.m_CurSp += lua_tonumber(g_Script.ls, -1);
			pPlayer->m_PlayerPropertyStatus[XA_CUR_SP - XA_MAX_EXP] = true;
			//pPlayer->m_PlayerAttriRefence[XA_CUR_SP - XA_MAX_EXP] = &pPlayer->m_Property.m_CurSp;
			pPlayer->m_Property.m_byAnimaUsedNum++;
		}
		else    //���Ӿ���
		{
			DWORD exp = static_cast<DWORD>(lua_tonumber(g_Script.ls, -1));
			SendAddPlayerExp(exp, SAExpChangeMsg::TASKFINISHED, "ScriptGive");
			pPlayer->m_PlayerPropertyStatus[XA_CUR_EXP - XA_MAX_EXP] = true;
			//pPlayer->SendAddPlayerExp(lua_tonumber(g_Script.ls, -1), SAExpChangeMsg::GM, "GMָ�����");
			pPlayer->m_Property.m_byExpUsedNum++;
		}
	}
	lua_pop(g_Script.ls, 2);
	g_Script.CleanPlayer();
	pPlayer->SendData(SARefreshPlayerMsg::ONLY_BACKUP);
	g_StoreMessage(pPlayer->m_ClientIndex, &AddQiDaoMsg, sizeof(SAAddQiDaoInfo));
}

//2014.2.21 add
void CItemUser::SendKeYinChangeData(const struct SAKeYinChange *pKeYinChange)
{
	if (!pKeYinChange)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;
	WORD msgsize = sizeof(SAKeYinChange);
	g_StoreMessage(pPlayer->m_ClientIndex, (LPVOID)pKeYinChange, msgsize);
}
//2014.2.21 add
void CItemUser::OnReturnYuanBaoMsg(struct SQGiftPacketMsg *pMsg)		//����Ԫ������е�װ����Ϣ
{
	if (!pMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;
	pPlayer->m_Property.m_YuanBaoGiftPos = pMsg->dwRqItemPos;		//�������������������λ��

	SPackageItem* pPackageItem = pPlayer->FindItemByPos(pMsg->dwRqItemPos, XYD_FT_ONLYUNLOCK);
	if (pPackageItem == NULL)	//û�и����
		return;
	//ִ��lua���Ԫ��������������
	//��ӽ��ղ����Ĵ������
	lua_getglobal(g_Script.ls, "GetYuanBaoGiftBaseInf");
	if (lua_pcall(g_Script.ls, 0, 2, 0) != 0)
	{
		lua_pop(g_Script.ls, 1);
		return;
	}
	SAGiftPacketMsg MyDefineGiftMsg(lua_tonumber(g_Script.ls, -1));
	MyDefineGiftMsg.Isbuyed = pPlayer->m_Property.m_IsBuyed;
	MyDefineGiftMsg.IsExtraCell = lua_tonumber(g_Script.ls, -2);
	lua_pop(g_Script.ls, 2);
	for (int i = 1; i <= MyDefineGiftMsg.TypeCount; i++)
	{
		lua_getglobal(g_Script.ls, "GetYuanBaoGiftInfo");
		lua_pushnumber(g_Script.ls, i);
		lua_pcall(g_Script.ls, 1, 2, 0);
		MyDefineGiftMsg.lpGiftPacketData[i - 1].ItemID = lua_tonumber(g_Script.ls, -1);
		MyDefineGiftMsg.lpGiftPacketData[i - 1].CurIDItemCount = lua_tonumber(g_Script.ls, -2);
		lua_pop(g_Script.ls, 2);
	}
	int DataSize = sizeof(SAGiftPacketMsg) - sizeof(void*) + MyDefineGiftMsg.TypeCount * sizeof(GiftPacketData);
	char *TmpBuf = new char[DataSize];
	memcpy(TmpBuf, &MyDefineGiftMsg, sizeof(SAGiftPacketMsg) - sizeof(void*));
	memcpy(TmpBuf + sizeof(SAGiftPacketMsg) - sizeof(void*), MyDefineGiftMsg.lpGiftPacketData, MyDefineGiftMsg.TypeCount * sizeof(GiftPacketData));
	g_StoreMessage(pPlayer->m_ClientIndex, TmpBuf, DataSize);
	delete[] TmpBuf;
}


void CItemUser::OnBueYuanBaoGift(struct SQBuyGiftPacketMsg *pMsg)	//����Ԫ������е�װ��
{
	if (!pMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;
	SABuyGiftPacketMsg AnsGiftMsg;

	g_Script.SetPlayer(pPlayer);
	lua_getglobal(g_Script.ls, "BuyYuanBaoGift");
	lua_pushnumber(g_Script.ls, pPlayer->m_Property.m_YuanBaoGiftPos);
	lua_pushnumber(g_Script.ls, pPlayer->m_Property.m_IsBuyed);
	if (lua_pcall(g_Script.ls, 2, 1, 0) != 0)
	{
		const char* ss = lua_tostring(g_Script.ls, -1);
		lua_pop(g_Script.ls, 1);
		return;
	}
	AnsGiftMsg.bResult = lua_tonumber(g_Script.ls, -1);
	if (AnsGiftMsg.bResult == 2)	//����ɹ�,�ұ��������㹻
		pPlayer->m_Property.m_IsBuyed = 0;
	if (AnsGiftMsg.bResult == 1)	//����ɹ��������Ӳ���
		pPlayer->m_Property.m_IsBuyed = 1;
	lua_pop(g_Script.ls, 1);
	g_Script.CleanPlayer();
	g_StoreMessage(pPlayer->m_ClientIndex, &AnsGiftMsg, sizeof(SABuyGiftPacketMsg));
}

//add 2014.3.5 ly
void CItemUser::OnOpenSetSkillPlan(struct SQOpenSetSkillBaseMsg *pMsg)		//����򿪼�������������ü���
{
	if (!pMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;

	//��дӦ����Ϣ
	SAOpenSetSkillBaseMsg AnsData;
	memcpy(AnsData.m_UpdateCurUsedSkill, pPlayer->m_Property.m_CurUsedSkill, 10 * sizeof(DWORD));
	AnsData.m_UpdatePlayerPattern = /*pPlayer->m_Property.m_PlayerPattern*/0;
	//��Ӧ�ͻ��˵�����
	g_StoreMessage(pPlayer->m_ClientIndex, &AnsData, sizeof(SAOpenSetSkillBaseMsg));
}

void CItemUser::OnUdtPlayerSkillAndPatern(struct SQUpdateSkillBaseMsg *pMsg)		//������ҵ�ǰ��ʹ�õļ��ܺ͵�ǰ����̬
{
	if (!pMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;
	////���ݼ��ܵ����ԣ��鿴�Ƿ������ҵĻ�������
	//1)��֮ǰ�������ӵ����Դ�������ϼ�ȥ

	//2)���¼��ܵ����Լӵ��������
	memcpy(pPlayer->m_Property.m_CurUsedSkill, pMsg->m_UpdateCurUsedSkill, 10 * sizeof(DWORD));		//���µ�ǰ��ʹ�õļ���
	////pPlayer->m_Property.m_PlayerPattern = pMsg->m_UpdatePlayerPattern;		//������ҵ�ǰ��̬
	//֪ͨ�ͻ����滻���
	SAUpdateSkillBaseMsg saRes;
	saRes.m_byRet = 1;
	g_StoreMessage(pPlayer->m_ClientIndex, &saRes, sizeof(SAUpdateSkillBaseMsg));
}

void CItemUser::OnGetKeYinChangeInfo(const struct SQKeYinChangeInfo *pKeYinChangeInfo)
{
	if (!pKeYinChangeInfo)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnGetKeYinChangeData"))
	{
		g_Script.PushParameter(pKeYinChangeInfo->bType);
		g_Script.PushParameter(pKeYinChangeInfo->bIndex);
		g_Script.PushParameter(pKeYinChangeInfo->bPropertyIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CItemUser::OnGetEquipJDingColor(const struct SQEquipJDingColor *pEquipJDingColor)
{
	if (!pEquipJDingColor)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnGetEquipJDingColor"))
	{
		g_Script.PushParameter(pEquipJDingColor->bType);
		g_Script.PushParameter(pEquipJDingColor->bIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CItemUser::SendSuitcondition(const struct SASuitcondition *pSuitcondition, WORD msgsize)
{
	if (!pSuitcondition)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;
	g_StoreMessage(pPlayer->m_ClientIndex, (LPVOID)pSuitcondition, msgsize);
}

void CItemUser::OnGetSuitcondition(const struct SQSuitcondition *pSuitcondition)
{
	if (!pSuitcondition)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnGetSuitcondition"))
	{
		g_Script.PushParameter(pSuitcondition->btype);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CItemUser::SendKeYinResult(const struct SABeginKeYin *pBeginKeYinInfo, WORD msgsize)
{
	if (!pBeginKeYinInfo)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;
	g_StoreMessage(pPlayer->m_ClientIndex, (LPVOID)pBeginKeYinInfo, msgsize);
}

//add by ly 2014/4/14
void CItemUser::OnReturnCellCountDown(const SQCellCountDown *pMsg)	//���󱳰����ӵ���ʱʱ��
{
	if (!pMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;
	SACellCountDown CellCountDown;
	lite::Variant ret;
	g_Script.SetCondition(NULL, pPlayer, NULL, NULL);
	INT64 TimeNow = 0;
	_time64(&TimeNow);
	DWORD  PlayerOnlineTime = pPlayer->m_Property.m_OnlineTime + (TimeNow - pPlayer->m_dwLoginTime);
	LuaFunctor(g_Script, FormatString("ReturnCountDownTime"))[BaseActivedPackage + pPlayer->m_Property.m_wExtGoodsActivedNum + 1][PlayerOnlineTime][0](&ret);
	g_Script.CleanCondition();
	int PackageCellCountDown = 0;
	if (ret.dataType == lite::Variant::VT_INTEGER)
		PackageCellCountDown = ret;
	CellCountDown.m_CountDown = PackageCellCountDown;
	g_StoreMessage(pPlayer->m_ClientIndex, &CellCountDown, sizeof(SACellCountDown));
}

void CItemUser::OnGetEquipPower(const struct SQEquipPower *pEquipPower)
{
	if (!pEquipPower)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;
	g_Script.SetPlayer(pPlayer);
	DWORD EquiptFightPower = 0;
	if (g_Script.GetEquiptFightPower("GetEquiptFightPower", GetGID(), pEquipPower->bType, pEquipPower->bIndex, EquiptFightPower))
	{
		SAEquipPower sepower;
		sepower.bIndex = pEquipPower->bIndex;
		sepower.dequippower = EquiptFightPower;
		g_StoreMessage(pPlayer->m_ClientIndex, &sepower, sizeof(SAEquipPower));
	}
	g_Script.CleanPlayer();
}

void CItemUser::OnBuyPlayerTp(const struct SQBuyPlayerTp* pPlayerTp)
{
	if (!pPlayerTp)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnPlayerBuyTp"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

