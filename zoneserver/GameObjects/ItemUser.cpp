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

// 打孔
const DWORD dwEbonyID		= 4010003;			// 打孔道具 玄铁石ID
const DWORD dwRedCopperID	= 4010004;			// 打孔道具 赤铜石ID
const DWORD dwSeaSparID		= 4010005;			// 打孔道具 海晶石ID

const DWORD dwDiamondID		= 4010006;			// 摘除道具1 金刚凿ID
const DWORD dwJadeID		= 4010007;			// 摘除道具2 玉凿ID

// 基础金钱
const DWORD BaseMoney		= 1000;

// 摘除宝石失败概率
const DWORD TakeOffRate		= 30;

// 三种打孔情况下失败的概率
enum FailureRate
{
	FAILURERATE_25 = 25,    // 1-3个孔 
	FAILURERATE_60 = 60,    // 4-5个孔
	FAILURERATE_90 = 90     // 6个孔
};

// 道具升级
enum
{
	HUISHENDAN_ID = 8040001,		// 回神丹
	LIANZHISHI_ID = 8040022,		// 炼制石ID
	XUANMENGTIANJING_ID = 8040023,	// 玄梦天精
};

// ============装备升阶的所用常量==============
// 升阶装备
const DWORD dwJinGangID		= 4000001;	// 金刚石 0 - 2 
const DWORD dwHaoYueID		= 4000002;	// 皓月石 3 - 5 
const DWORD dwYunTieID		= 4000003;	// 陨铁石 6 - 8
const DWORD dwJinJingID		= 4000004;	// 金精石 9

// 保护道具
const DWORD dwYuSuiID		= 4010001;	// 玉髓

// 概率道具
const DWORD dwMiJiID		= 4010002;	// 欧冶子秘籍

// ====== 装备升品质所用道具 =========
const DWORD dwTianQingID	= 4003001;	//天青石

const DWORD dwJianJingID	= 4001001;	// 尖晶石
const DWORD dwHeiXuanID		= 4002001;	// 黑玄石

// =================装备鉴定跟属性重置===========
const DWORD dwXingChenID	= 4020001;	// 属性重置
const DWORD dwXilianID		= 4020002;	// 装备鉴定
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

	// 启动道具组冷却时间
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

	// 启动道具单独冷却
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

	// 检查跑地图任务信息
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

					// 获取坐标值
					WORD x = flag->m_TaskDetail.Region.DestX;
					WORD y = flag->m_TaskDetail.Region.DestY;

					if (x >= m_curTileX-5 && x <= m_curTileX+5 && y >= m_curTileY-5 && y <= m_curTileY+5)
					{
						flag->m_Complete = 1;

						// 发送旗标更新消息
						pPlayer->SendUpdateTaskFlagMsg(*flag, true);
					}
				}
				else
				{
					rfalse("检查跑地图任务信息，竟然找不到任务旗标！");
				}
			}
			else
			{
				rfalse("检查跑地图任务信息，竟然找不到任务！");
			}
		}
	}
}

void CItemUser::UpdateUseItemTaskFlag(CPlayer *pPlayer, DWORD index)
{
	if (!pPlayer || 0 == index)
		return;

	// 检查物品使用任务信息
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

					// 发送旗标更新消息
					pPlayer->SendUpdateTaskFlagMsg(*flag, oldComp != flag->m_Complete);
				}
				else
				{
					rfalse("检查使用道具任务信息，竟然找不到任务旗标！");
				}
			}
			else
			{
				rfalse("检查使用道具任务信息，竟然找不到任务！");
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

	// 检查获取东西任务信息
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
						// 发送旗标更新消息
						pPlayer->SendUpdateTaskFlagMsg(*flag, oldComp != flag->m_Complete);
					}
				}
				else
				{
					rfalse("检查取物品任务信息，竟然找不到任务旗标！");
				}
			}
			else
			{
				rfalse("检查取物品任务信息，竟然找不到任务！");
			}
		}
	}
}

void CItemUser::UpdateGiveItemTaskFlag(CPlayer *pPlayer, DWORD index)
{
	if (0 == pPlayer || 0 == index)
		return;

	// 检查给物品任务信息
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
						// 发送旗标更新消息
						pPlayer->SendUpdateTaskFlagMsg(*flag, oldComp != flag->m_Complete);
					}
				}
				else
				{
					rfalse("检查给物品任务信息，竟然找不到任务旗标！");
				}
			}
			else
			{
				rfalse("检查给物品任务信息，竟然找不到任务！");
			}
		}
	}
}

void CItemUser::UpdateItemTaskFlag(CPlayer *pPlayer, DWORD index)
{
	if (!pPlayer || 0 == index)
		return;

	// 检查物品任务信息
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

					// 如果曾经完成，并且新的道具数量不影响结果，直接跳过
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
						// 发送旗标更新消息
						pPlayer->SendUpdateTaskFlagMsg(*flag, oldComp != flag->m_Complete);
					}
				}
				else
				{
					rfalse("检查任务物品信息，竟然找不到任务旗标！");
				}
			}
			else
			{
				rfalse("检查任务物品信息，竟然找不到任务！");
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

	if (autoUnLock)		// 需要解锁，就先解锁
		LockItemCell(item.wCellPos, false, true); // [2012-9-10 12-17 gw: *这里主动同步给客户端解锁 false->true]

	if (!FillItemPos(linePos, SItemWithFlag::IWF_ITEMIN, false))
	{
		return FALSE;
		MY_ASSERT(0);
	}
		
	// 删除索引
	if (m_PackageItems[linePos].m_Item.overlap < itemData->m_Overlay)
	{
		m_ItemsState[item.wIndex] -= (itemData->m_Overlay - item.overlap);
		//rfalse(2, 1, "删除后，为%d的道具目前可以再叠加%d个", item.wIndex, m_ItemsState[item.wIndex]);
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
		rfalse(2,1,"_UseItem: 传进来的指针为空!!!");
		return FALSE;
	}

	if ( pItem->wIndex==0 || pItem->overlap == 0)
	{
		rfalse(4,1,FormatString("发现一个错误的道具ID: %d,叠加数量%d 账号%s",pItem->wIndex,pItem->overlap,pPlayer->GetAccount()));
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

	// 独立脚本扩展
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
		DelItem(*pItem, "道具使用删除！");

	// 加入冷却（组和单独）
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

	//rfalse(2, 1, "为%d的道具目前可以再叠加%d个", pItem->wIndex, m_ItemsState[pItem->wIndex]);

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

	// 保存当前位置上的装备数据
	SEquipment tempEquip = player->m_Property.m_Equip[i];

	SItemBase *item = pItem;
	
	if (!item)
	{
		rfalse(4, 1, "[ItemUser] - _EquipItem()");
		return FALSE;
	}

	// 更新绑定状态
	item->flags |= GetSItemBaseBindState(EITEM_BIND_EQUIP, itemWillEquip->m_BindType);

	player->m_Property.m_Equip[i] = *(SEquipment *)item;

	// 将装备信息发送给客户端
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

		// 将消息发送给有效区域的所有玩家
		CArea *area = (CArea *)m_ParentArea->DynamicCast(IID_AREA);
		if (area)
			area->SendAdj(&msg, sizeof(msg), -1);
	}
	
	// 重新计算装备加成属性
	player->InitEquipmentData();
	UpdateAllProperties();
	
	// 当前装备之前是无效数据时，属于道具移动，删除装备栏中的
	if (tempEquip.wIndex == 0)
		return DelItem(*pItem);

	// 如果是道具交换，则需要更新这个道具的内容
	memset((SItemBase*)pItem, 0, sizeof(SRawItemBuffer));
	memcpy((SItemBase*)pItem, (SItemBase*)&tempEquip, sizeof(SRawItemBuffer));
	SendItemSynMsg(pItem);

	// 更新状态（其实对于装备，不用更新，因为装备是不可叠加的！）

	ValidationPos(pItem->wCellPos);

	//更新修为值 
	player->UpdatePlayerXwzMsg();

	return TRUE;
}

BOOL CItemUser::_UseBookUpSkill(SPackageItem *pItem)		//使用技能书激活技能
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
	AddSkill.wTypeID = SkillID;//在配置技能表中技能ID和所对应的技能书的ID相同
	AddSkill.byLevel = 1;//成功激活技能后，等级为1
	BOOL bRet = player->ActivaSkill(AddSkill.wTypeID, "学习技能！");
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

    // 对其他准备动作的打断操作

	// 将装备孔信息发送给客户端
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

		// 将消息发给有效区域的所有玩家
		if (CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA))
			pArea->SendAdj(&EquipItemMsg, sizeof(SAEquipItemMsg), -1);
	}

// 	WORD playeratk[4];
// 	for (int i = 0 ;i<4;i++)
// 	{
// 		playeratk[i] =*reinterpret_cast<WORD *>(m_AttriRefence[i]);
// 	}
    // 重新计算装备加成属性
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
// 				nattri > 0 ?  TalkToDnid(player->m_ClientIndex,FormatString("攻击力增加了%d",nattri)):TalkToDnid(player->m_ClientIndex,FormatString("攻击力减少了%d",abs(nattri)));
// 		}
// 	}

	return TRUE;
}

// PickOneItem只是内部工具函数，所以很多验证在外部做了，在此就不做了
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

	if (g_PackageMoneyIndex == index)	// 拾取货币
	{
		if (0 == pItem->m_Money)
			return FALSE;

		pPlayer->AddPlayerMoney(pItem->m_MoneyType, pItem->m_Money);
		pItem->m_Money = 0;
	}
	else								// 拾取某个位置上的道具
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
			TalkToDnid(pPlayer->m_ClientIndex, "背包已满！无法拾取！");
			return FALSE;
		}
			
		PickOneItem(pItem->m_GroundItem[index]);

		--pItem->m_ItemsInPack;

		// 修正索引
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

	// 检查道具保护
	if (!pPlayer->m_pCheckItem->m_dwTeamID && pPlayer->m_pCheckItem->m_dwGID!=GetGID())
	{
		//TalkToDnid(pPlayer->m_ClientIndex,"又求不是你的，再拣手打断！",0);
		return FALSE;
	}

	// 检查道具保护
	if (pPlayer->m_pCheckItem->m_dwTeamID)
	{
		if (pPlayer->m_pCheckItem->m_dwTeamID != pPlayer->m_dwTeamID)		// 没权利
			return FALSE;

		// 钱已经被分了，所以此处必须相等！
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

	// 道具包裹（分全部拾取/单个拾取）
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
			TalkToDnid(pPlayer->m_ClientIndex, "背包已满！无法拾取！");
			return FALSE;
		}

		// 到此为止，拾取一定会是成功的，依次拾取
		for (int i=0; i<pPlayer->m_pCheckItem->m_ItemsInPack; ++i)
			PickOneItem(pPlayer->m_pCheckItem->m_GroundItem[i]);

		// 加入玩家的货币数
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

	//+ 任务道具不能丢弃,需要通知客户端
	if (ITEM_IS_TASK(pData->m_Type))
	{
		return FALSE;
	}

	SPackageItem temp = *pIB;

	if (DelItem(*pIB, "从背包丢弃！"))
	{
		// 如果删除成功，判断是否掉落
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

// 客户要求获取装备信息
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

	// 格子未激活
	if (0 == (m_PackageItems[linePos].m_State & SItemWithFlag::IWF_ACTIVED))
		return FALSE;

	// 格子无道具
	if (!canBeEmpty && (0 == (m_PackageItems[linePos].m_State & SItemWithFlag::IWF_ITEMIN)))
		return FALSE;

	// 格子被锁定
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

	// 性别验证
	if (pData->m_Sex != 0 && (pPlayer->m_Property.m_Sex ? 1 : 2) != pData->m_Sex)
		return FALSE;

	// 门派验证
	if (pData->m_School != 0 && pPlayer->m_Property.m_School != pData->m_School)
		return FALSE;

	// 等级需求验证
	if (pData->m_Level != 0 && pPlayer->m_Property.m_Level < pData->m_Level)
		return FALSE;

	// 检查冷却
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

	// 耐久度为0，则不能装备
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
	if (ITEM_IS_BOOK(pData->m_Type))	//使用技能书升级技能
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
	// 在背包内移动
	CPlayer *player = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!player)
		return FALSE;

	if (!IsItemPosValid(wCellPos1) || !IsItemPosValid(wCellPos2, false, true))
	{
		return FALSE;
	}

	// 任务栏内的道具无法移入移出
	WORD srcLinePos = wCellPos1;
	WORD desLinePos = wCellPos2;

	ValidationPos(srcLinePos);
	ValidationPos(desLinePos);

	// 检测源物品存在
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

	// 背包内移动有几种状况
	// 1. 目标位置为空，只有在没有道具的情况下才会返回0
	SPackageItem *desItem = FindItemByPos(desLinePos, XYD_FT_ONLYUNLOCK);
	if (0 == desItem)
	{
		if (!FillItemPos(srcLinePos, SItemWithFlag::IWF_ITEMIN, false))return FALSE;
		
		if(!FillItemPos(desLinePos, SItemWithFlag::IWF_ITEMIN, true))return FALSE;
		

		memcpy(&m_PackageItems[desLinePos].m_Item, &m_PackageItems[srcLinePos].m_Item, sizeof(SPackageItem));
		memset(&m_PackageItems[srcLinePos].m_Item, 0, sizeof(SPackageItem));

		// 发送源道具的删除消息
		SADelPackageItemMsg msg;
		msg.stItem = m_PackageItems[desLinePos].m_Item;
		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(msg));

		// 修改源道具的坐标
		m_PackageItems[desLinePos].m_Item.wCellPos = wCellPos2;
		SendItemSynMsg(&m_PackageItems[desLinePos].m_Item);

		ValidationPos(srcLinePos);
		ValidationPos(desLinePos);
		return TRUE;
	}

	const SItemBaseData *desData = CItemService::GetInstance().GetItemBaseData(desItem->wIndex);
	if (!desData)
		return FALSE;

	// 2. 交换的双方为不同的道具或者为相同的道具，但不可叠加，则直接交换
	// 注意第二个判断，处理了2种情况：相同类型不可叠加和相同类型可叠加，但是目标道具已是最大量
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
		// 交换的双方为相同道具，执行重叠流程
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
	
	// 这里为了和玩家默认产生装备相对应
	if (0 == player->m_Property.m_Equip[byPos].uniID)
	{
		extern BOOL GenerateNewUniqueId(SItemBase &item);

		// 说明是数据库服务器默认产生的，那么在这里产生一个UniqueID
		if (!GenerateNewUniqueId(player->m_Property.m_Equip[byPos]))
			return FALSE;
	}
	bool ret = false;
	if (onlydel)
		 ret =  DelEquipment(byPos);
	else
	     ret = AddExistingItem(player->m_Property.m_Equip[byPos], emptyPos, true) && DelEquipment(byPos);

	//更新修为值 
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

// Func: 锁定或者解锁一批格子
// @ vecCell 格子数据列表
// @ lock 是否锁定
// @ sync 是否同步给客户端
// return 是否有锁定成功的
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
	
	// [2012-9-13 19-05 gw: + 对于装备栏的处理]
	//update by ly 2014/4/10 现在的玩家最多只能有60个格子
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

// 收到装备打造的消息
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
	// 数据异常~
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

// 收到拖入材料消息
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

		// 可以添加一些判断检测

		if (pPlayer->m_forgeMetrial)
		{
			rfalse(2, 1, "已经拖入了材料，解除当前材料~~");
			LockItemCell(pPlayer->m_forgeMetrial->wCellPos, false);
		}	
		pPlayer->m_forgeMetrial = srcItem;

		pPlayer->m_vecStrengthenMetrial.push_back(pMsg->waPos[i]);		
		LockItemCell(byPosX, byPosY, true);
	}	

	return TRUE;
}

// 收到取消打造消息
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

// 宝石镶嵌
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
		if (pEquip->slots[i].isEmpty())//找一个空格子
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

	// 执行嵌入
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
		DelItem(*player->m_forgeMetrial, "镶嵌消耗！", true);
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

// 宝石摘除
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
	if (pMsg->SelectType == 0 || pMsg->SelectType == 2)//保护石
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
		msg.result = SATakeOffJewel::ATJ_Metrial_ERROR;//材料不够
		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SATakeOffJewel));
		OnthSysNotice(1);
		return FALSE;
	}
	msg.pos	   = index;	
	msg.result = SATakeOffJewel::ATJ_SUCCESS;
	player->ChangeMoney(pStakeoff->TakeOffGemMoneyType,-pStakeoff->TakeOffGemMoney);
	bool bHaveBindMetrial = false;
	OnProcessNeedMetrial(player,vec,pStakeoff->TakeOffGemNum,"【装备摘除提示】原材料消耗！", bHaveBindMetrial);
	if (pMsg->SelectType == 2)
	{
		OnProcessNeedMetrial(player,vec1,1,"保护道具提示】原材料消耗！",bHaveBindMetrial);
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
	if (Var > pStakeoff->TakeOffGemHoleRale) //成功
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
	// 给玩家添加宝石
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
	//// 扣除货币和材料
	//if (player->m_forgeMetrial->overlap > 1)
	//{
	//	player->m_forgeMetrial->overlap--;
	//	player->SendItemSynMsg(player->m_forgeMetrial, XYD_FT_ONLYLOCK);
	//}
	//else
	//{
	//	DelItem(*player->m_forgeMetrial, "摘除消耗", true);
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

	// 判断装备孔洞是否已经被打开以及得到孔洞索引
	for (size_t i = 0; i < SEquipment::MAX_SLOTS; ++i)
	{
		if (pEquip->slots[i].isInvalid())	
		{
			index = i; break;
		}
	}

	if (-1 == index)
	{
		//rfalse(2, 1, "装备的洞已经全部打开，无法再开孔~");
		msg.result = SAMakeHole::AMH_ALL_OPENED;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMakeHole));
		return FALSE;
	}
	msg.index = index;
	// 获取所需道具ID和金钱
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
// 	if (pPlayer->m_HoleType == 1)//使用了成功符
// 	{
// 		pPro = CProtectInfo::GetInstance().FindProtectInfoStruct(pPlayer->m_HoleType);
// 		if (!pPro)
// 		{
// 			msg.result = SAMakeHole::AMH_PrctectMetrial;	// 缺少材料
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
	OnProcessNeedMetrial(pPlayer,vec,pMakeHole->HoleNum,"【装备打孔提示】原材料消耗！",bHaveBindMetrial);

	//////////////////////////////////////////////////////////////////////////
	bool bHaveLuckyCharmItem = false;
	// [扣取幸运符，有的话扣除一个，成功率增加]	
	if (byStrengthenExternChoose & EStrengthenChoose_LuckyCharm)
	{
		std::vector<SPackageItem*> vecLuckyCharm;
		nFlg = pPlayer->OnGetNeedMetrial(pPlayer, pMakeHole->byLuckyCharmNum, vecLuckyCharm, pMakeHole->dwLuckyCharmID);	
		if (nFlg == 1)
		{
			pPlayer->OnProcessNeedMetrial(pPlayer, vecLuckyCharm, pMakeHole->byLuckyCharmNum, "扣除了一个幸运符！", bHaveBindMetrial);
			bHaveLuckyCharmItem = true;
		}
	}
	// [扣取保护符,如果有保护符的话，扣除一个，不降星]
	bool bHaveProtectionCharacterItem = false;
	if (byStrengthenExternChoose & EStrengthenChoose_ProtectionCharacter)
	{
		std::vector<SPackageItem*> vecProtectionCharacter;
		nFlg = pPlayer->OnGetNeedMetrial(pPlayer, pMakeHole->byProtectionCharacterNum, vecProtectionCharacter, pMakeHole->dwProtectionCharacterID);
		bHaveProtectionCharacterItem = (nFlg == 1);
		if (nFlg == 1)
		{
			pPlayer->OnProcessNeedMetrial(pPlayer, vecProtectionCharacter, pMakeHole->byProtectionCharacterNum, "扣除了一个保护符！", bHaveBindMetrial);
			bHaveProtectionCharacterItem = true;
		}
	}

	// 计算概率，成功或者失败
	long ktemp = CRandom::RandRange(1, 10000);
	if (bHaveLuckyCharmItem)
	{		
		wmakeSuccessRate += pMakeHole->wLuckyCharmAddSuccessRate;		
	}
	// 条件成熟，可以开始打孔了
	if (ktemp <= wmakeSuccessRate)
	{
		//rfalse(2, 1, "打第%d个洞，概率为%d，打孔成功~", msg.index, makeRate);
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
// 			rfalse(2, 1, "降孔了:id=%d", pEquip->wIndex);
// 			pEquip->slots[index-1].type = 0;
// 			pEquip->slots[index-1].value = 0;
// 			SendUpdateEquipInfo(pEquip);
// 		}
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAMakeHole));
		return FALSE;
	}
	
	return TRUE;
}

// 装备打孔
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
	if(pp->SetForgeData(pStart))  // 是装备和酒的话
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
	//+ 任务道具无法拆分
	if (ITEM_IS_TASK(itemData->m_Type))
	{
		return FALSE;
	}

	if (0 == pMsg->count || pMsg->count >= source->overlap)
		return FALSE;

	// 被锁定/绑定的道具不能拆分
	if ((source->flags & SItemBase::F_SYSBINDED) || (source->flags & SItemBase::F_PLAYERBINDED))
		return FALSE;

	// 验证背包，拆分后只会占用一个格子，所以只需要一个格子
	WORD emptyPos = FindBlankPos(XYD_PT_BASE);
	if (0xffff == emptyPos)
		return FALSE;

	// 先创建新的
	SPackageItem new_item = *source;
	new_item.overlap = pMsg->count;

	extern BOOL GenerateNewUniqueId(SItemBase &item);
	if (!GenerateNewUniqueId(new_item))
		return FALSE;

	AddExistingItem(new_item, emptyPos, true);

	// 首先修改原有的
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
	//如果在道具读条使用状态中，不允许物品的移动
	//if(pPlayer->GetReadyEvent() && pPlayer->GetReadyEvent()->IsEvent("UseItemEvent"))
	//{
	//	pPlayer->BreakOperation();	// 中断操作
	//}
	pPlayer->m_BatchReadyEvent.BreakOperationReadyEvent(Event::EEventInterrupt_MoveItem);
	
	switch (pMsg->byMoveTo)
	{
	case IMD_INPACKAGE:		// 背包内
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

	case IMD_PACK2EQUIP:	// 拖动换装
		return ToEquip(pMsg->wCellPos1);
		break;

	case IMD_EQUIP2PACK:	// 拖动卸装
		return ToPackage(pMsg->wCellPos1); // pMsg->CellY1
		break;

	case IMD_FP_PACK2EQUIP:	// 侠客拖动换装
		return _fpToEquip(pMsg->wCellPos1,pMsg->byindex);
		break;

	case IMD_FP_EQUIP2PACK:	// 侠客拖动卸装
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

	// 判断能否添加
	if (!CanAddItems(itemList))
	{
		TalkToDnid(pPlayer->m_ClientIndex, "背包已满！无法拾取！");
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
	LPIObject npc = m_ParentRegion->SearchObjectListByRegion(m_CommNpcGlobalID); //手游版本
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

// 获取ID为index的道具在背包中的可叠加数
WORD CItemUser::GetRemainOverlap(DWORD index)
{
	if (index <= 0)
	{
		return 0;
	}

	MY_ASSERT(index);

	// 背包中没有ID为index的道具
	ItemState::iterator itemState = m_ItemsState.find(index);
	if (itemState == m_ItemsState.end())
		return 0;

	return itemState->second;
}

// 获取所需的空格子数目
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

	// 首先查看能叠加的数目
	WORD canOverap = GetRemainOverlap(Info.m_Index);
	
	// 如果能完全叠加
	if (canOverap >= Info.m_Count)
		return 0;

	WORD remainCount = Info.m_Count - canOverap;

	// 如果无法完全叠加，则计算所需格子数
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
// [2012-8-23 18-06 gw: +重新设置随机属性的条目，对于已经锁定的条目和数据 不处理]
BOOL CItemUser::ResetRandomExtraAttriEntry(SEquipment* equip, const vector<int> &vecLockedIndex)
{
	return TRUE;
}

// 添加装备的随机属性个数
BOOL CItemUser::AddRandomAttri(SEquipment* equip)
{
// 	if (!equip)
// 	{
// 		rfalse(4, 1, "Itemuser.cpp - AddRandomAttri() - !equip");
// 		return FALSE;
// 	}
// 
// 	// 得到装备的随机属性列表
// 	const SEquipExtraAttriTable *ExtraAttribute = CItemService::GetInstance().GetExtraAttriTable(equip->wIndex);
// 
// 	if (!ExtraAttribute)
// 	{
// 		rfalse(2, 1, "找不到 %d 装备的随机属性信息", equip->wIndex);
// 		return FALSE;
// 	}
// 
// 	// 计算ID，装备等级 * 1000 + 装备品质 * 100 + 装备阶数
// 	const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(equip->wIndex);
// 	if (!pData)
// 	{
// 		rfalse(4, 1, "[ItemUser.cpp] - AddRandomAttri()");
// 		return FALSE;
// 	}
// 	MY_ASSERT(pData);
// 
// 	// 读取最大随机附加属性表
// 	const SMaxExtraAttri *pMaxExtraAttri = CItemService::GetInstance().GetMaxExtraAttri(pData, equip->attribute.grade);
// 	if (!pMaxExtraAttri && pData->m_Level >= 10)
// 	{
// 		rfalse(2, 1, "找不到 %d 装备的【最大】随机属性%d,  %d   %d  ", index,pData->m_Level,pData->m_Color,equip->attribute.grade);
// 		return FALSE;
// 	}
// 
// 	// 获得装备的随机属性个数
// 	int ExAttriNum = CRandom::RandRange(ExtraAttribute->MinNum, ExtraAttribute->MaxNum);
// 	
// 	if (ExAttriNum < 1 && ExAttriNum > SEquipment::MAX_BORN_ATTRI)
// 	{
// 		rfalse(2, 1, "存储属性个数的数组越界！");
// 		return FALSE;
// 	}
// 	ExAttriNum = CRandom::RandRange(1,100);
// 	ExAttriNum = CItemService::GetInstance().GetRandomNum(pData->m_Color,ExAttriNum);
// 	if (ExAttriNum == 0)
// 	{
// 		rfalse(2, 1,"GetRandomNum");
// 		return FALSE;
// 	}
// 	std::vector<int> IndexVec(ExAttriNum, SEquipDataEx::EEA_MAX);	// 用来存储最终随机属性类型
// 	
// 	//int RateBase		 = 10000;									// 概率基数
// 	int TempRate		 = 0;
// 	int TypeIndex		 = 0;										// 用于存储最终附加到装备上的属性索引
// 	int AttriRate[SEquipDataEx::EEA_MAX];							// 随机属性的概率数组
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
// 			int index = CRandom::RandRange(0, SEquipDataEx::EEA_MAX);//随机索引而已
// 			if (find(IndexVec.begin(), IndexVec.end() ,index) == IndexVec.end() )//没有找到
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
// 		// 排除0属性值
// 		if (0 == ExtraAttribute->ExtraData[AttriType][0] || 0 == ExtraAttribute->ExtraData[AttriType][1])
// 		{
// 			rfalse(2, 1, "出现了一个空的随机属性值!");
// 			continue;
// 		}
// 
// 		// 添加初始随机附加属性值
// 		equip->attribute.bornAttri[k].type  = AttriType;
// 		equip->attribute.bornAttri[k].value = CRandom::RandRange(ExtraAttribute->ExtraData[AttriType][0], 
// 																 ExtraAttribute->ExtraData[AttriType][1]);
// 		if (pData->m_Level >= 10)
// 		{
// 			// 添加最大随机附加属性值，其值是初始附加属性最大值+提升的附加属性值
// 			equip->attribute.MaxExtraAttri[k].type	= AttriType;
// 			equip->attribute.MaxExtraAttri[k].value	= ExtraAttribute->ExtraData[AttriType][1] + pMaxExtraAttri->ExtraData[AttriType];
// 		}
// 	}

	return TRUE;
}

// func:获得物品当前应该设置的绑定状态,目前的绑定都定位玩家绑定
// param: eCurBindType-当前判断的绑定类型 ITEM_BIND_TYPE
// param: byItemDataBindType-物品表中原始绑定类型 
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
		rfalse(4,1,FormatString("发错一个错误的道具%d",params.m_Index));
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

	// 回神丹的单独处理
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

		// 成功升级次数跟已经升级次数都是0，一共占2个byte
		pBuf += 2 * sizeof(BYTE);

		// 得到当前的真气值
		memcpy(pBuf, &SpValue, sizeof(WORD));
	}

	// 装备类
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

		equip->attribute.grade	  = itemData->m_StepNum;//品质0
		equip->attribute.bCurrentStar = 0 ; //当前星级0
		equip->attribute.bIntensifyLevel = 1;	//强化等级1

		for (size_t i=0; i<SEquipment::MAX_SLOTS; i++)
			equip->slots[i].type = 0, equip->slots[i].value = 0;

		// 装备的随机附加属性添加
		memset(equip->attribute.bornAttri, 0, MAX_ATTRI_NUM * sizeof(SEquipment::ExtraAttri));  //强化属性
		memset(equip->attribute.starAttri,0,MAX_ATTRI_NUM*sizeof(SEquipment::ExtraAttri));	//升星属性
		memset(equip->attribute.MaxJDingAttri, 0, SEquipment::MAX_SLOTS * sizeof(SEquipment::ExtraAttri)); //鉴定属性
	
		equip->attribute.bJDingAttriNum = 0;   //初始化鉴定属性条数为0
		OnUpdataGrade(equip);
		// 升星初始化
//		ZeroMemory(equip->attribute.starNum, SEquipment::MAX_STAR_NUM * sizeof(BYTE));
		//AddRefineAttribute(equip);
	}

	return TRUE;
}

WORD CItemUser::TryPossibleMerge(DWORD index, WORD count, WORD maxOverlap)
{
	WORD willCreate = count;

	// 如果不可叠加，那么返回所有
	if (1 == maxOverlap)
		return willCreate;

	// 先执行可能的合并
	ItemState::iterator specItem = m_ItemsState.find(index);
	if (specItem == m_ItemsState.end() || 0 == specItem->second)
		return willCreate;

	// 遍历道具表
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

			if (willCreate >= remainOverlap)		// 无法一次重叠完
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

	//rfalse(2, 1, "执行完合并后，ID为%d的道具目前可以再叠加%d个", specItem->first, m_ItemsState[specItem->first]);

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

		// 在产生物品时脚本扩展
		if (genNew)
		{
			lite::Variant ret;
			g_Script.SetRawItemBuffer(&item);
			LuaFunctor(g_Script, FormatString("OnGenerateNewItem%d", pData->m_ScriptID))[pData->m_ID](&ret);
			g_Script.CleanRawItemBuffer();
		}
		// 都放在普通背包里
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
		rfalse(4,1,"添加物品是物品列表为空");
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

		// 先执行可能的合并
		WORD willCreate = TryPossibleMerge(it->m_Index, it->m_Count, pData->m_Overlay);

		SRawItemBuffer itemBuffer;
		InsertItem(it->m_Index, willCreate, pData, itemBuffer, true);
	}

	return TRUE;
}

// 能否在背包中加入一些道具
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

	WORD baseNeedBlank	= 0;		// 基本背包需要的个数
	WORD taskNeedBlank	= 0;		// 任务背包需要的个数
	
	WORD baseBlankCell	= GetBlankCell(XYD_PT_BASE) + reuseBaseBlank;
	WORD taskBlankCell	= GetBlankCell(XYD_PT_BASE) + reuseTaskBlank;	


	CPlayer *player = (CPlayer*)DynamicCast(IID_PLAYER);
	const SItemBaseData *pData = 0;
	for (std::map<DWORD, WORD>::iterator it = addList.begin(); it != addList.end(); ++it)
	{
		pData = CItemService::GetInstance().GetItemBaseData(it->first);
		if (!pData)
		{
			TalkToDnid(player->m_ClientIndex, "没有找到该道具的信息！");
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
				TalkToDnid(player->m_ClientIndex, "任务所需格子数不足！");
				return FALSE;
			}
		}
		else
		{
			baseNeedBlank += needBlank;
			if (baseNeedBlank > baseBlankCell)
			{
				TalkToDnid(player->m_ClientIndex, "背包格子不足！");
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

	if (pos >= BaseStart && pos < BaseEnd)		// 对于基本的道具格子
	{
		// 如果未激活，那么格子不应该被锁定和有道具
		bool Condition1 = ((0 == (pItemCell->m_State & SItemWithFlag::IWF_ACTIVED)) && (0 == (pItemCell->m_State & SItemWithFlag::IWF_ITEMIN)) &&
			(0 == (pItemCell->m_State & SItemWithFlag::IWF_LOCKED)) && (0 == memcmp(&pItemCell->m_Item, &m_EmptyItem4Debug, sizeof(SPackageItem))));

		// 如果激活，有道具，可以没有锁定
		bool Condition2 = (pItemCell->m_State & SItemWithFlag::IWF_ACTIVED) && (pItemCell->m_State & SItemWithFlag::IWF_ITEMIN) &&
			(0 == (pItemCell->m_State & SItemWithFlag::IWF_LOCKED)) && (0 != memcmp(&pItemCell->m_Item, &m_EmptyItem4Debug, sizeof(SPackageItem))) &&
			(0 != pItemCell->m_Item.wIndex) && ((pItemCell->m_Item.wCellPos) == pos);
		
		// 如果激活，没有道具，则不能被锁定
		bool Condition3 = (pItemCell->m_State & SItemWithFlag::IWF_ACTIVED) && (0 == (pItemCell->m_State & SItemWithFlag::IWF_ITEMIN)) &&
			(0 == (pItemCell->m_State & SItemWithFlag::IWF_LOCKED)) && (0 == memcmp(&pItemCell->m_Item, &m_EmptyItem4Debug, sizeof(SPackageItem)));

		// 如果激活，有道具，可以被锁定
		bool Condition4 = (pItemCell->m_State & SItemWithFlag::IWF_ACTIVED) && (pItemCell->m_State & SItemWithFlag::IWF_ITEMIN) &&
			(pItemCell->m_State & SItemWithFlag::IWF_LOCKED) && (0 != memcmp(&pItemCell->m_Item, &m_EmptyItem4Debug, sizeof(SPackageItem))) &&
			(0 != pItemCell->m_Item.wIndex) && ((pItemCell->m_Item.wCellPos) == pos);
		
		if (!Condition1 && !Condition2 && !Condition3 && !Condition4)
		{
			return FALSE;
		}
		MY_ASSERT(Condition1 || Condition2 || Condition3 || Condition4);
	}
	else										// 对于任务道具，永恒激活，永恒不锁定
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
	
	// 此格子未激活
	if (0 == (pItemCell->m_State & SItemWithFlag::IWF_ACTIVED))
		return 0;

	// 此格子无道具
	if (0 == (pItemCell->m_State & SItemWithFlag::IWF_ITEMIN))
		return 0;

	// 锁定/非锁定逻辑
	if ((XYD_FT_ONLYLOCK == LookType && (0 == (pItemCell->m_State & SItemWithFlag::IWF_LOCKED))) ||
		((XYD_FT_ONLYUNLOCK == LookType && (pItemCell->m_State & SItemWithFlag::IWF_LOCKED))))
		return 0;

	return &pItemCell->m_Item;
}


//add by ly 2014/3/25
//检索相应的道具ID是否在背包中,如果在返回对应的信息
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
	// 保证是0x1,0x2,0x4,0x8,0x10.....
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

	// 这里特殊处理当一个格子被标记为锁定/解锁的时候
	// 对道具状态的处理
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
			//rfalse(2, 1, "%s后，ID为%d的道具目前可以再叠加%d个", mark ? "锁定" : "解锁", pItemCell->m_Item.wIndex, m_ItemsState[pItemCell->m_Item.wIndex]);
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
		// 确定哪些是已经激活的
		for (DWORD i = 0; i < PackageAllCells; i++)
		{
			if (i >= dwNeedActivedLen)
				continue;

			m_PackageItems[i].m_State |= SItemWithFlag::IWF_ACTIVED;

			ValidationPos(i);
		}

		// 道具恢复
		for (DWORD i = 0; i < PackageAllCells; i++)
		{
			ValidationPos(i);

			if (pPlayer->m_Property.m_BaseGoods[i].wIndex != 0)		// 说明有道具
				AddExistingItem(pPlayer->m_Property.m_BaseGoods[i], i, false);

			ValidationPos(i);
		}
	}
	else
	{
		SAAddPackageItemMsg msg;

		// 发送所有的道具信息
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

	// 添加道具
	SCellPos &actPos = (SCellPos &)m_PackageItems[pos].m_Item;
	actPos.wCellPos = pos;
	memcpy(&(SItemBase&)m_PackageItems[pos].m_Item, &item, sizeof(SRawItemBuffer));
	UpdateItemRelatedTaskFlag(pPlayer, m_PackageItems[pos].m_Item.wIndex);

	if (m_PackageItems[pos].m_Item.overlap < itemData->m_Overlay)		// 有容量才更新
	{
		m_ItemsState[item.wIndex] += (itemData->m_Overlay - item.overlap);
		//rfalse(2, 1, "新建后，ID为%d的道具目前可以再叠加%d个", item.wIndex, m_ItemsState[item.wIndex]);
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

	// 验证金钱是否足够
	if (!player->CheckPlayerMoney(itemData->m_BuyMoneyType, itemData->m_BuyPrice * count, true))
		return FALSE;

	std::list<SAddItemInfo> itemList;
	itemList.push_back(SAddItemInfo(index, count));

	// 判断能否添加
	if (!CanAddItems(itemList))
	{
		TalkToDnid(player->m_ClientIndex, "亲，背包已满哦~装不下了哦~");
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
	LuaFunctor(g_Script, "ActivePackageOpt")[ActivePackageCellIndex][(int)(player->m_Property.m_OnlineTime + TimeNow - player->m_dwLoginTime)][1][msg->wWillActiveNum]();	//最后一个参数为1，表示金币购买激活格子
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

		if (pMsg->bBuy)				// 如果是买
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
		else					// 如果是卖
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

// 检测指定类型，数量的道具是否存在，可通过指定checkOnly == false，同时删除
BOOL CItemUser::CheckGoods(DWORD index, DWORD number, BOOL checkOnly)
{
	if (0 == index || 0 == number)
		return FALSE;

	if (GetItemNum(index, XYD_FT_ONLYUNLOCK) < number)
		return FALSE;

	if (checkOnly)
		return TRUE;			// 只查询

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
			DelItem(*pItem, "CheckGoods收取！");
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
	if ( type >= SAUseIntervalEffect::EUIT_MAX )	//新的CD暂时依靠老的传送结构
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
	// 暂时注释掉（zeb-2010-2-22 道具类型整理）
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

    //    _DelItem( *( *it ), "系统回收" );

    //    if ( player )
    //        TalkToDnid( player->m_ClientIndex, FormatString( "您[%s]已被系统回收!", itemData->szName ) );
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
		TalkToDnid(pPlayer->m_ClientIndex,"你正在上马状态，无法使用道具");
		return FALSE;
	}

	if (pPlayer->m_InUseItemevens)
	{
		TalkToDnid(pPlayer->m_ClientIndex,"你当前状态无法使用道具");
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
							GenerateChangeItem( pp, GenItemParams( pData->wChangedItemID2, 1, 5, 1 ),  tempCellPosition ,LogInfo( 100, "可变物品概率1的创建！" ));
						}
						else
						{
							GenerateChangeItem( pp, GenItemParams( pData->wChangedItemID1, 1, 5, 1 ),  tempCellPosition ,LogInfo( 100, "可变物品概率2的创建！" ));
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

// 处理单修
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
		TalkToDnid(pPlayer->m_ClientIndex, "只有装备才可以修理！");
		return FALSE;
	}

	if (!itemData->m_MaxWear)
	{
		TalkToDnid(pPlayer->m_ClientIndex, "永不磨损的装备不能够修理！");
		return FALSE;
	}

	BYTE pos = pMsg->pos;
	SEquipment *pEquip	= GetEquipByPos(pMsg->type, pos);

	if (!pEquip)
		return FALSE;

	if (!pEquip->attribute.maxWear)
	{
		TalkToDnid(pPlayer->m_ClientIndex, "当前最大耐久度为0，不能修理！");
		return FALSE;
	}

	WORD needMoney = (static_cast<float>(itemData->m_BuyPrice) * 1.75 / static_cast<float>(itemData->m_MaxWear)) * (pEquip->attribute.maxWear - pEquip->attribute.currWear);
	if (!pPlayer->CheckPlayerMoney(XYD_UM_ONLYUNBIND, needMoney, false))
	{
		TalkToDnid(pPlayer->m_ClientIndex, "修复装备所需的银币金钱不足！");
		return FALSE;
	}

	//if (pEquip->attribute.maxWear - pEquip->attribute.currWear < 100)
	//	return FALSE;

	pEquip->attribute.maxWear = static_cast<float>(pEquip->attribute.maxWear) * 0.95;

	if (!pEquip->attribute.maxWear)
	{
		TalkToDnid(pPlayer->m_ClientIndex, "修理以后，装备的当前最大耐久度已经为0，无法使用！");
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

	// 只有耐久度为0时，才影响到玩家的属性，所以如果变为了0，则检查所有的装备
	if (!pPlayer->m_Property.m_Equip[pos].attribute.currWear)
	{
		pPlayer->InitEquipmentData();
		pPlayer->UpdateAllProperties();
	}

	return TRUE;
}

// 处理全修
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

		// 未修复前的最大耐久度是否为0
		if (!pPlayer->m_Property.m_Equip[i].attribute.maxWear)
		{
			TalkToDnid(pPlayer->m_ClientIndex, "最大耐久度为0哦，不能修复！");
			continue;
		}

		// 判断金钱
		WORD needMoney = (static_cast<float>(itemData->m_BuyPrice) * 1.75 / static_cast<float>(itemData->m_MaxWear)) * (pPlayer->m_Property.m_Equip[i].attribute.maxWear - pPlayer->m_Property.m_Equip[i].attribute.currWear);
		
		if (!pPlayer->CheckPlayerMoney(XYD_UM_ONLYUNBIND, needMoney, false))
		{
			TalkToDnid(pPlayer->m_ClientIndex, "只完成了部分装备的修复，由于银币不足，修复终止！");
			return FALSE;
		}
	
		// 修复一次要减少最大耐久度
		WORD WearValue = static_cast<float>(pPlayer->m_Property.m_Equip[i].attribute.maxWear) * 0.05;
		pPlayer->UpdateEquipWear(i, WearValue, true);

		if (!pPlayer->m_Property.m_Equip[i].attribute.maxWear)
		{
			TalkToDnid(pPlayer->m_ClientIndex, "最大耐久度已经为0，不能继续修复，或者使用！");
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

// 装备修复的内部使用函数
SEquipment* CItemUser::GetEquipByPos(BYTE type, BYTE pos)
{
	SEquipment *pEquip = 0;
	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);

	switch(type)
	{
		// 背包里的装备
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
				rfalse("装备位置错误鸟~~~");
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

// =================装备升级、精炼（AHJ）==================
// 拖入装备与拖入材料基本上按照装备强化的步骤来
// 所有的限制也是按照装备强化的来要求
// =================================================
// 获取一个位置 分布情况,可能是包裹，可能是装备栏
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
			rfalse(2, 1, "获取物品位置出错了。 pos = %d", wItemPos);
			MY_ASSERT(0);
		}
	}
	else 
	{		
		rfalse(2, 1, "获取物品位置出错了。 pos = %d", wItemPos);		
		MY_ASSERT(0);
	}
	return ePosType;
}

// 获取包裹栏的位置，注意是否需要添加装备栏基准线
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

// 检测装备栏是否被锁定了
BOOL CItemUser::IsLockedEquipColumnCell(WORD wEquipPos)
{
	if (wEquipPos >= EQUIP_P_MAX)
	{
		return FALSE;
	}
	return (m_EquipedColumnFlag[wEquipPos].m_State & SItemWithFlag::IWF_LOCKED) != 0;
}

// 锁定或解锁定装备栏的格子
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
// 通过位置获取装备(可能是从包裹中获取，也可能是从装备栏里面获取)
SEquipment * CItemUser::GetEquipmentbyItem(WORD wEquipPos, WORD LookType/* = XYD_FT_WHATEVER*/)
{
	SEquipment *pEquip = NULL;

	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return pEquip;

	TItemPosType ePosType = GetItemPosType(wEquipPos);
	if (ePosType == EPackage_PosType)
	{
		// [2012-9-13 19-34 gw: -不太好控制是否锁定]
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

// 通过通用位置检测并且更新装备栏的属性
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
	// 重新计算装备加成属性
	pPlayer->InitEquipmentData();
	pPlayer->UpdateAllProperties();
}

// 拖入装备
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
	
	// 数据异常检测	
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
	int iSuccessNum = 0; // 成功处理拖入物品的个数
	for (int i = 0; i < pMsg->byPosNum; ++i)
	{
		const SEquipment *pEquip = GetEquipmentbyItem(pMsg->waPos[i], XYD_FT_ONLYUNLOCK);
		if (CEquipStrengthenServer::handleElemDragStrengthenEquip(pPlayer, pEquip, pMsg->bType))
		{			
			pPlayer->m_vecStrengthenEquip.push_back(pMsg->waPos[i]);	

			bool Flag1 = false;
			// 显示下一等级
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
			case SQAddEquip::ADE_EQUIP_RESET:	// 装备洗练(重置附加属性条数)	
				type = SAAfterUpgradeEquipMsg::EQUIP_RESET;
				break;
			case SQAddEquip::ADE_EQUIP_REFINE: // 装备精炼(提升附加属性数值)	
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
		// 材料我们统一发送		
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
// 拖入材料
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
	// 数据异常~
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
	// 检测之前拖的装备是否存在
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
			// 检测所拖的材料数据是否存在
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

			// 检测材料是否正确 todo..
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
// 装备升阶
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

// 装备精炼--升星
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

// 升品质
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

// 升级,装备强化
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

// 装备分解
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
// 装备熔炼
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

// 装备升阶（提升附加属性的上限）
BOOL CItemUser::RecvBeginShengJie(struct SQBeginIdentifyMsg* pMsg)
{


	
	return TRUE;
}

// 装备灵附
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
		// 获取卷轴		
		//通过属性来临时标记
		WORD wMetrialPos =  pPlayer->m_vecStrengthenEquipAssist.at(i);		
		if (CEquipStrengthenServer::handleElemEquipSpiritAttachBody(pPlayer, pEquip, pMsg->type, wEquipPos, wMetrialPos))
		{
			CheckAndUpdateEquipColumnAttribyPos(pPlayer, wEquipPos);
			++bySuccessNum;
		}		
	}
	if (bySuccessNum > 0)
	{		
		// 成功后，解锁卷轴，清空卷轴列表
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

// 结束强化
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
	case SQEndRefineMsg::ERM_EQUIP_OFF:// 取消装备
	case SQEndRefineMsg::ERM_CLOSE:// 关闭面板
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
// 	case SQEndRefineMsg::ERM_EQUIP_OFF:// 取消装备
// 		{

// 			if (player->m_forgeType== SQAddMetrial::ADM_RAW_METRIAL && player->m_forgeMetrial)
// 			{
// 				LockItemCell(player->m_forgeMetrial->byCellX,player->m_forgeMetrial->byCellY,false);
// 				player->m_forgeMetrial = 0;
// 				player->m_forgeType	= 0;	
// 			}
// 		}
// 		break;
	case SQEndRefineMsg::ERM_METRAIL_OFF:// 取消原始材料
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
	// 读取升阶配置表
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

		// 失败后的处理，降阶
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
		// 此处添加升阶后的附加属性值
		AddRefineAttribute(pE);
		// 背包显示信息更新
		SendUpdateEquipInfo(pE);
		// 显示下一等级
		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SABeginRefineGrade));
		if (pE->attribute.grade == SEquipment::MAX_GRADE_NUM )
		{
			RemoveRefineMetrial();
			OnthSysNotice(8);
		}
		else
		{
			SendNextLevelEquip(SAAfterUpgradeEquipMsg::UPDATE_GRADE);
			// 根据计算结果扣减材料			
			SendNeedMETRIALInfo(player,SQAddEquip::ADE_REFINE_GRADE);
		}
		// 当最高阶时，清除所有材料
		
	}

	return bRet;
}

// 打造界面中用于显示下一等级装备的信息
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
// 				// 添加最大附加随机属性
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
// 				// 添加最大附加随机属性
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

// [锻造功能里发送极品预览]
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
		case SAAfterUpgradeEquipMsg::UPDATE_STAR: // 强化
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
		
		case SAAfterUpgradeEquipMsg::EQUIP_RESET: // 洗练			
		case SAAfterUpgradeEquipMsg::EQUIP_REFINE: // 精炼
		case SAAfterUpgradeEquipMsg::UPDATE_GRADE: // 升阶
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
				// 添加最大附加随机属性
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
	
	
	// 添加基础属性
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

	// 读取“升阶精炼增加属性”配置表
	const SItemUpgradeAttribute* pUattri = CItemService::GetInstance().GetUpgradeAttribute(pData);
	if (!pUattri)
		return FALSE;

	const SMaxExtraAttri *pMaxExtraAttri = CItemService::GetInstance().GetMaxExtraAttri(pData, pEquip->attribute.grade);
	if (!pMaxExtraAttri)
		return FALSE;

	// 得到装备的随机属性列表
	const SItemUpgradeAttribute *pUpgradeAttri = CItemService::GetInstance().GetUpgradeAttribute(pData);
	if (!pUpgradeAttri)
	{
		return FALSE;
	}
	MY_ASSERT(pEquip);

	// 添加附加属性
	switch(player->m_refineType)
	{
		case SQAddEquip::ADE_REFINE_GRADE:
			break;
		
		case SQAddEquip::ADE_REFINE_STAR:
			{
				int num	= 0;		// 用来标记附加的基础属性的个数

				int star_num = 0;	// 星星的绝对个数
				int starAttriValue = 0;

				// 判断星星的个数
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
				// [2012-8-21 19-13 gw: -下标调整] starAttriValue = pUattri->wStarAttri[star_num];
				starAttriValue = pUattri->wStarAttri[iStarIndex];
				for (size_t j = 0; j < SEquipDataEx::EEA_MAX; ++j)
				{
					if (pItemFactor->m_ValueFactor[j])
					{
						// [2012-8-17 18-17 gw: +增加基础属性]
						pEquip->attribute.starAttri[num].type	= j;
						pEquip->attribute.starAttri[num].value	= starAttriValue;
						++num;
						
						//// 最大附加属性值
						//pEquip->attribute.MaxExtraAttri[num].type	= j;
						//pEquip->attribute.MaxExtraAttri[num++].value = ExtraAttribute->ExtraData[j][1] + pMaxExtraAttri->ExtraData[j];
					}

					if (num >= SEquipment::MAX_GRADE_ATTRI)
					{
						break;
					}
				}

				// [2012-8-21 18-52 gw: +清空无效数据]
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
		case SQAddEquip::ADE_EQUIP_REFINE: //精炼
			{
				std::vector<int> typevec;				
				int	iTmpAddValue = 0; // 添加的属性值，需要从配置里面读取
				int iTmpHandleAttriNum = 1; // 需要在几个属性上添加上面的属性值				

				// 取出已经有的附加属性值
				std::vector<int> vecTmpBornAttriIndex;
				const int iArrayBornAttriLen = sizeof(pEquip->attribute.bornAttri) / sizeof(pEquip->attribute.bornAttri[0]);
				for (size_t i = 0; i < iArrayBornAttriLen; ++i)
				{
//					WORD wTmpRefineValue = (pEquip->attribute.refineExtraAttri[i].type==0xff)?0:pEquip->attribute.refineExtraAttri[i].value;
// 					if (pEquip->attribute.bornAttri[i].type != 0xff
// 						&& pEquip->attribute.bornAttri[i].value + wTmpRefineValue < pEquip->attribute.MaxExtraAttri[i].value)
					{ //如果达到最大值了，不再会有精炼
						vecTmpBornAttriIndex.push_back(i);
					}
				}
				if (vecTmpBornAttriIndex.size() == 0)
				{
					break;
				}

				for (size_t i = 0; i < iTmpHandleAttriNum; ++i)
				{
					//这里有问题 iBornIndex = vecTmpBornAttriIndex.at(iBornIndex);
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

					// 越界判断
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

// Func: 添加最大的随机附加属性值
// @ pEquip- 需要添加附加属性的装备
// @ bFillRefineAttri-是否填充附加属性到最大附加属性,慎用：是-会设置精炼属性值，使附加属性之和达到最大附加属性值
BOOL CItemUser::AddMaxExtraAttri(SEquipment* pEquip, BOOL bFillRefineAttri/* = FALSE*/)
{

	return FALSE;
}
// 私有函数：最高级别的时候移除所有强化材料
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
		
	TalkToDnid(pPlayer->m_ClientIndex, "已经达到最大等级了，不能继续强化装备！");
	return TRUE;
}

// 私有函数：装备是否是新手装备
BOOL CItemUser::Equip_Is_Tyler(DWORD wIndex)
{
	if ((wIndex >= 9000001 && wIndex <= 9000005) || (wIndex >= 9010001 && wIndex <= 9010024))
		return FALSE;
	else
		return TRUE;
}

// 背包显示信息更新
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

// 升级成功后添加新装备
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

	// 产生新装备属性
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
	{// 如果包裹中的装备
		DelItem(*pPackagItem, "【装备升级提示】成功升级，删除原装备！", true);
	}
	else
	{// 如果自己身上的装备 todo..

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
	// 添加最大附加随机属性
	AddMaxExtraAttri(equip);
	WORD pos = FindBlankPos(XYD_PT_BASE);
	AddExistingItem(item, pos, true);

	

	// 产生新装备后锁定
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
			//设置装备道具的侠客索引
			player->_fpSetEquipFightPet(index);
			IsEquip = _EquipFightPetItem(pIB,index);
			///清除需要装备道具的侠客索引
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
		///清除需要装备道具的侠客索引
		player->_fpInitEquipFightPetIndex();
		return FALSE;
	}
		
	MY_ASSERT(ITEM_IS_EQUIPMENT(itemWillEquip->m_Type) && 1 == itemWillEquip->m_Overlay && 1 == pItem->overlap);

	///如果不是侠客装备
// 	if (itemWillEquip->m_School != IO_XIAKE)
// 	{
// 		///清除需要装备道具的侠客索引
// 		player->_fpInitEquipFightPetIndex();
// 		return FALSE;
// 	}
	
	if (!CanEquipItFightPet(player,pItem,itemWillEquip,index))
	{
		///清除需要装备道具的侠客索引
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
		///清除需要装备道具的侠客索引
		player->_fpInitEquipFightPetIndex();
		return FALSE;
	}

	// 保存当前位置上的装备数据
	SEquipment tempEquip = player->m_Property.m_FightPets[index].m_Equip[i];

	SItemBase *item = pItem;
	player->m_Property.m_FightPets[index].m_Equip[i] = *(SEquipment *)item;

	// 将装备信息发送给客户端
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

		// 将消息发送给有效区域的所有玩家
		CArea *area = (CArea *)m_ParentArea->DynamicCast(IID_AREA);
		if (area)
			area->SendAdj(&msg, sizeof(msg), -1);
	}

	// 重新计算侠客装备加成属性
	player->_fpInitEquipmentData();
	player->_fpUpdateAllProperties();

	// 当前装备之前是无效数据时，属于道具移动，删除装备栏中的
	if (tempEquip.wIndex == 0)
		return DelItem(*pItem);

	// 如果是道具交换，则需要更新这个道具的内容
	memset((SItemBase*)pItem, 0, sizeof(SRawItemBuffer));
	memcpy((SItemBase*)pItem, (SItemBase*)&tempEquip, sizeof(SRawItemBuffer));
	SendItemSynMsg(pItem);

	// 更新状态（其实对于装备，不用更新，因为装备是不可叠加的！）
	ValidationPos(pItem->wCellPos);

	///清除需要装备道具的侠客索引
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

	// 性别验证
	if (pData->m_Sex != 0 && temp != pData->m_Sex)
		return FALSE;

	temp =  pPlayer->m_Property.m_FightPets[index].m_fpLevel;

	// 等级需求验证
	if (pData->m_Level != 0 && temp< pData->m_Level)
		return FALSE;
	
	// 侠客门派验证
	const SFightPetBaseData *pBaseData =CFightPetService::GetInstance().GetFightPetBaseData(pPlayer->m_Property.m_FightPets[index].m_fpID);
	if(!pBaseData)return FALSE;
	if (pData->m_School != pBaseData->m_Type&&pData->m_School != IO_XIAKE)
	{
		TalkToDnid(pPlayer->m_ClientIndex,"侠客的门派不同无法装备");
		return FALSE;
	}

	// 检查冷却
	if (CheckItemCDTime(pData))
		return FALSE;

	// 耐久度为0，则不能装备
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
	
	//设置操作的侠客
	if (!player->_fpSetEquipFightPet(index))return FALSE;

	//寻找当前位置的装备
	if (byPos >= EQUIP_P_MAX || (player->m_Property.m_FightPets[index].m_Equip[byPos].wIndex == 0))
		return FALSE;

	WORD emptyPos = FindBlankPos(XYD_PT_BASE);
	if (0xffff == emptyPos)
		return FALSE;

	// 这里为了和玩家默认产生装备相对应
	if (0 == player->m_Property.m_FightPets[index].m_Equip[byPos].uniID)
	{
		extern BOOL GenerateNewUniqueId(SItemBase &item);

		// 说明是数据库服务器默认产生的，那么在这里产生一个UniqueID
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

	// 对其他准备动作的打断操作

	// 将装备孔信息发送给客户端
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

		// 将消息发给有效区域的所有玩家
		if (CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA))
			pArea->SendAdj(&EquipItemMsg, sizeof(SAEquipItemMsg), -1);
	}

	// 重新计算装备加成属性
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
	//+ 任务物品不能装备
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

// 取消洗点
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
		TalkToDnid(player->m_ClientIndex,"随身商店的道具列表不存在!");
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
		TalkToDnid(player->m_ClientIndex,"随身商店的道具不存在!");
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
		rfalse(2, 1, "你在没有申请的收，就发接了！");
		
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

// 拖入升级道具
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

	// 数据异常~
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

	// 判断最大升级次数
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
		rfalse(2, 1, "【回神丹】已经拖入的一个道具，解锁当前道具！");
		LockItemCell(player->m_UpdateItem->wCellPos, false);

		if (player->m_LJS_Metrial)
		{
			rfalse(2, 1, "概率材料也要清空！");
			LockItemCell(player->m_LJS_Metrial->wCellPos, false);
			player->m_LJS_Metrial = 0;
		}

		if (player->m_XMTJ_Metrial)
		{
			rfalse(2, 1, "概率材料也要清空！");
			LockItemCell(player->m_XMTJ_Metrial->wCellPos, false);
			player->m_XMTJ_Metrial = 0;
		}
	}

	player->m_UpdateItem = srcItem;
	player->m_UpdateItemType = pMsg->bType;	

	LockItemCell(pMsg->wCellPos, true);

	return TRUE;
}

// 拖入升级道具需要的保护材料
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

	// 需要先拖入装备
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

	// 判断类型是否正确
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
			rfalse(2, 1, "[回神丹]已经拖入了材料[炼制石]，解除当前材料~~");
			LockItemCell(pPlayer->m_LJS_Metrial->wCellPos, false);
		}

		pPlayer->m_LJS_Metrial = srcItem;
	}

	if (SQDragUpdateMetrialMsg::DUM_XUANJING == pMsg->bType)
	{
		if (pPlayer->m_XMTJ_Metrial)
		{
			rfalse(2, 1, "[回神丹]已经拖入了材料[玄梦天精]，解除当前材料~~");
			LockItemCell(pPlayer->m_XMTJ_Metrial->wCellPos, false);
		}
		
		pPlayer->m_XMTJ_Metrial = srcItem;
	}

	LockItemCell(pMsg->wCellPos, true);

	return TRUE;
}

// 开始升级道具
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

	/*	此处解析存放在SRawItemBuffer中关于道具升级的信息
		struct SRawItemBuffer : public SItemBase
		{
		BYTE buffer[120];		// 公用空间
		};
	*/
	// ==============调用脚本进行相关数据测试===============
	g_Script.SetCondition(0, pPlayer, 0);
	LuaFunctor(g_Script, "HuiShenDanLimitInfo")[pPlayer->m_UpdateItem->wIndex]();
	g_Script.CleanCondition();
	// ====================================
	return TRUE;
}

// 结束升级道具
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

	// 得到升级的次数次数
	memcpy(pCurrentUpdateTimes, pBuf, sizeof(BYTE));
	pBuf += sizeof(BYTE);

	// 得到已经升级成功的次数
	memcpy(pSuccessUpdateTimes, pBuf, sizeof(BYTE));
	pBuf += sizeof(BYTE);

	// 得到当前的真气值
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
	
	// 成功
	if (RandRate <= rate)
	{
		// 成功以后，pSpValue释放，不再使用
		delete pSPValue;

		const SHuiYuanDanInfo* pInfo = CItemService::GetInstance().GetHuiYuanDanInfo(++(*pSuccessUpdateTimes));
		if (!pInfo)
		{
			return FALSE;
		}
		MY_ASSERT(pInfo);

		// 成功添加高等级的回神丹
		SRawItemBuffer item;
		GenerateNewItem(item, SAddItemInfo(pPlayer->m_UpdateItem->wIndex + 1, 1));

		// 复制升级属性信息
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

		// 删除以前的道具
		DelItem(*pPlayer->m_UpdateItem, "[回神丹升级成功，删除原道具]", true);
		pPlayer->m_UpdateItem = 0;

		// 添加新道具
		WORD pos = FindBlankPos(XYD_PT_BASE);
		AddExistingItem(item, pos, true);

		// 产生新装备后锁定
		pPlayer->m_UpdateItem = FindItemByPos(pos, XYD_FT_ONLYUNLOCK);
		LockItemCell(pos, true);

		msg.wCellPos = pos;
		msg.bType = SABeginUpdateItem::BUI_SUCCESS;
	}
	else
	{
		memcpy(pPlayer->m_UpdateItem->buffer, pCurrentUpdateTimes, sizeof(BYTE));
		// 失败，先发送道具更新
		pPlayer->SendItemSynMsg(pPlayer->m_UpdateItem, XYD_FT_ONLYLOCK);

		// 然后发送失败消息
		msg.wCellPos = pPlayer->m_UpdateItem->wCellPos;		
		msg.bType = SABeginUpdateItem::BUI_FAILED;
	}

	// 扣减材料
	if (pPlayer->m_LJS_Metrial)
	{
		if( pPlayer->m_LJS_Metrial->overlap > 1)
		{
			pPlayer->m_LJS_Metrial->overlap--;
			pPlayer->SendItemSynMsg(pPlayer->m_LJS_Metrial, XYD_FT_ONLYLOCK);
		}
		else
		{
			DelItem(*pPlayer->m_LJS_Metrial, "道具升级消耗！", true);
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
			DelItem(*pPlayer->m_XMTJ_Metrial, "道具升级消耗！", true);
			pPlayer->m_XMTJ_Metrial = 0;
		}
	}

	msg.bSuccessTimes = *pSuccessUpdateTimes;
	msg.bTotalUpdateTimes = *pCurrentUpdateTimes;

	g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SABeginUpdateItem));

	// 释放指针
	delete pSuccessUpdateTimes;
	delete pCurrentUpdateTimes;
	
	pSuccessUpdateTimes = 0;
	pCurrentUpdateTimes = 0;

	return TRUE;
}

// 客户端请求获得所有套装属性是否激活
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
	else if (Flag == 2)//要特殊处理情况了
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
	
	// 添加道具
	SCellPos &actPos = (SCellPos &)m_PackageItems[pos].m_Item;
	actPos.wCellPos = pos;

	memcpy(&(SItemBase&)m_PackageItems[pos].m_Item, &item, sizeof(SRawItemBuffer));

	if (m_PackageItems[pos].m_Item.overlap < itemData->m_Overlay)		// 有容量才更新
	{
		//m_ItemsState[item.wIndex] += (itemData->m_Overlay - item.overlap);
		//rfalse(2, 1, "新建后，ID为%d的道具目前可以再叠加%d个", item.wIndex, m_ItemsState[item.wIndex]);
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
	if (Money < 10) // 小于十两不够扣
	{
		
		return FALSE;
	}
	Money *= 0.1f; 
	Money = ceil(Money);
	if (pMsg->NoticeRoll)
	{
		Money += 100.0f;
	}
	
	if (pPlayer->m_Property.m_BindMoney < Money)//不够扣钱
	{
		return FALSE;
	}
	
	WORD linePos = pMsg->wCellPos;

	if (linePos > PackageAllCells)
		return FALSE;

	ValidationPos(linePos);

	// 格子未激活
	if (0 == (m_PackageItems[linePos].m_State & SItemWithFlag::IWF_ACTIVED))
		return FALSE;

	// 格子无道具
	//if (!canBeEmpty && (0 == (m_PackageItems[linePos].m_State & SItemWithFlag::IWF_ITEMIN)))
	//	return FALSE;

	//// 格子被锁定
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

	// 检查道具保护
	if (!pItem->m_dwTeamID && pItem->m_dwGID!=pPlayer->GetGID())
	{
		//TalkToDnid(pPlayer->m_ClientIndex,"又求不是你的，再拣手打断！",0);
		return FALSE;
	}

	// 检查道具保护
	if (pItem->m_dwTeamID)
	{
		if (pItem->m_dwTeamID != pPlayer->m_dwTeamID)		// 没权利
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

	int itemismoney = pPlayer->_L_GetLuaValue("OnItemIsMoney",pData->m_ID,0); //判断是否为金钱道具
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
		TalkToDnid(pPlayer->m_ClientIndex, "背包已满！无法拾取！");
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

long CItemUser::OnGetNeedMetrial(CPlayer* pPlayer,DWORD NeedNum,std::vector<SPackageItem*> & vec,long ItemID, WORD LookType/* = XYD_FT_ONLYUNLOCK*/)//从背包中得到合适的道具集合
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
		if (  (c1 != 0 ) && (c2 != 0) && (c3 == 0))//此格子是激活(并且)格子中有道具(并且)没有被锁定
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
		return 1;//说明数量是满足条件的
	}
	else
	{
		return 2;
	}
	return 2;
}
// [Func:扣除所需要的材料]
// [pPlayer-扣除谁身上的材料]
// [vec-从该道具列表扣除]
// [NeedNum-需要扣除的数量]
// [str-提示信息]
// [bHaveBindMetrial-获取是否有绑定的材料]
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
			// [此处依次扣除材料，没有先扣绑定]
			if (!bHaveBindMetrial)
			{
				bHaveBindMetrial = ((item->flags & (SItemBase::F_SYSBINDED | SItemBase::F_PLAYERBINDED))!=0);
			}			
			if (item->overlap > TempNum)
			{
				// [2012-8-21 19-27 gw: +扣物品，使用changeoverlap统一处理]
				ChangeOverlap(item, TempNum, false, true);
				// [2012-8-21 19-27 gw: -使用下面的方式没有对m_ItemsState的处理]
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

	if(btype == 0) //装备栏
	{
		if (bIndex >= 0 && bIndex < EQUIP_P_MAX)
		{
			memcpy(&equip,&pPlayer->m_Property.m_Equip[bIndex],sizeof(SEquipment));
			return true;
		}
	}
	else if (btype == 1) //背包
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

			// 将装备信息发送给客户端
			SAEquipInfoMsg msg;
			int n = sizeof(msg.stEquip);
			msg.stEquip = pPlayer->m_Property.m_Equip[bIndex];
			msg.byPos = bIndex;
			g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAEquipInfoMsg));

			// 重新计算装备加成属性
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

			// 将装备信息发送给客户端
			SAEquipInfoMsg msg;
			msg.stEquip = pPlayer->m_Property.m_Equip[pMsg->bIndex];
			msg.byPos = pMsg->bIndex;
			g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAEquipInfoMsg));

			msg.stEquip = pPlayer->m_Property.m_Equip[pMsg->bIndex2];
			msg.byPos = pMsg->bIndex2;
			g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAEquipInfoMsg));


			// 重新计算装备加成属性
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
	// 先执行可能的合并
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

void CItemUser::OnReturnQiDaoMsg(struct SQQiDaoInfo *pMsg)	//接收请求祈祷消息
{
	if (!pMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;

	g_Script.SetPlayer(pPlayer);
	//执行lua获得祈祷的相关数据
	SAQiDaoInfo QiDaoMsg;
	//添加接收操作的处理代码
	time_t dwCurTime;
	time(&dwCurTime);
	tm * pCurT = localtime(&dwCurTime);
	tm CurTime, PreTime;
	memcpy(&CurTime, pCurT, sizeof(tm));
	tm *pPreT = localtime(&pPlayer->m_Property.m_dwPreReqTime);
	memcpy(&PreTime, pPreT, sizeof(tm));
	if (CurTime.tm_year > PreTime.tm_year || CurTime.tm_yday > PreTime.tm_yday)		//24点重置功能
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

void CItemUser::OnAddQiDaoAndReturn(struct SQAddQiDaoInfo *pMsg)	//接收客户端领取祈祷消息做相应的处理并返回结果
{
	if (!pMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;

	SAAddQiDaoInfo AddQiDaoMsg;
	//执行lua获得祈祷的相关数据
	g_Script.SetPlayer(pPlayer);
	//添加接收操作的处理代码
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

	//更新数据
	if (AddQiDaoMsg.cOptResult == 2)
	{
		//更新祈祷相关类型的数据
		if (pMsg->bQiDaoType == 0)		//增加银币
		{
			pPlayer->ChangeMoney(2, lua_tonumber(g_Script.ls, -1));		//更新银币
			pPlayer->m_Property.m_bySilerCoinUsedNum++;
		}
		else if (pMsg->bQiDaoType == 1)		//增加真气
		{
			pPlayer->m_Property.m_CurSp += lua_tonumber(g_Script.ls, -1);
			pPlayer->m_PlayerPropertyStatus[XA_CUR_SP - XA_MAX_EXP] = true;
			//pPlayer->m_PlayerAttriRefence[XA_CUR_SP - XA_MAX_EXP] = &pPlayer->m_Property.m_CurSp;
			pPlayer->m_Property.m_byAnimaUsedNum++;
		}
		else    //增加经验
		{
			DWORD exp = static_cast<DWORD>(lua_tonumber(g_Script.ls, -1));
			SendAddPlayerExp(exp, SAExpChangeMsg::TASKFINISHED, "ScriptGive");
			pPlayer->m_PlayerPropertyStatus[XA_CUR_EXP - XA_MAX_EXP] = true;
			//pPlayer->SendAddPlayerExp(lua_tonumber(g_Script.ls, -1), SAExpChangeMsg::GM, "GM指令添加");
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
void CItemUser::OnReturnYuanBaoMsg(struct SQGiftPacketMsg *pMsg)		//返回元宝礼包中的装备信息
{
	if (!pMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;
	pPlayer->m_Property.m_YuanBaoGiftPos = pMsg->dwRqItemPos;		//保存玩家所点击的礼包的位置

	SPackageItem* pPackageItem = pPlayer->FindItemByPos(pMsg->dwRqItemPos, XYD_FT_ONLYUNLOCK);
	if (pPackageItem == NULL)	//没有改礼包
		return;
	//执行lua获得元宝礼包的相关数据
	//添加接收操作的处理代码
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


void CItemUser::OnBueYuanBaoGift(struct SQBuyGiftPacketMsg *pMsg)	//购买元宝礼包中的装备
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
	if (AnsGiftMsg.bResult == 2)	//购买成功,且背包格子足够
		pPlayer->m_Property.m_IsBuyed = 0;
	if (AnsGiftMsg.bResult == 1)	//购买成功，但格子不足
		pPlayer->m_Property.m_IsBuyed = 1;
	lua_pop(g_Script.ls, 1);
	g_Script.CleanPlayer();
	g_StoreMessage(pPlayer->m_ClientIndex, &AnsGiftMsg, sizeof(SABuyGiftPacketMsg));
}

//add 2014.3.5 ly
void CItemUser::OnOpenSetSkillPlan(struct SQOpenSetSkillBaseMsg *pMsg)		//请求打开技能设置面板配置技能
{
	if (!pMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;

	//填写应答消息
	SAOpenSetSkillBaseMsg AnsData;
	memcpy(AnsData.m_UpdateCurUsedSkill, pPlayer->m_Property.m_CurUsedSkill, 10 * sizeof(DWORD));
	AnsData.m_UpdatePlayerPattern = /*pPlayer->m_Property.m_PlayerPattern*/0;
	//相应客户端的请求
	g_StoreMessage(pPlayer->m_ClientIndex, &AnsData, sizeof(SAOpenSetSkillBaseMsg));
}

void CItemUser::OnUdtPlayerSkillAndPatern(struct SQUpdateSkillBaseMsg *pMsg)		//更新玩家当前所使用的技能和当前的形态
{
	if (!pMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return;
	////根据技能的属性，查看是否更改玩家的基本属性
	//1)把之前技能所加的属性从玩家身上减去

	//2)把新技能的属性加到玩家身上
	memcpy(pPlayer->m_Property.m_CurUsedSkill, pMsg->m_UpdateCurUsedSkill, 10 * sizeof(DWORD));		//更新当前所使用的技能
	////pPlayer->m_Property.m_PlayerPattern = pMsg->m_UpdatePlayerPattern;		//更新玩家当前形态
	//通知客户端替换结果
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
void CItemUser::OnReturnCellCountDown(const SQCellCountDown *pMsg)	//请求背包格子倒计时时间
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

