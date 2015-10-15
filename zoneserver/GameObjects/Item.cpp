#include "StdAfx.h"
#include "item.h"
#include "Area.h"
#include "region.h"
#include "Networkmodule\ItemMsgs.h"
#include "networkmodule\RegionMsgs.h"
#include "networkmodule\TeamMsgs.h"
#include "GroundItemWinner.h"
#include "TeamManager.h"
#include "CItemDefine.h"
#include "CItemService.h"
#include <vector>

extern std::map<DWORD, Team> teamManagerMap;
extern LPIObject GetPlayerByGID(DWORD);
std::vector<std::string> g_strItemlog;
std::map<DWORD,WORD> g_allitemCount;
std::map<WORD,DWORD> g_allmoneyCount;

CItem::CItem(void) {}

CItem::~CItem(void) {}

int CItem::OnCreate(_W64 long pParameter)
{
	SParameter *pParam = (SParameter *)pParameter;

	m_pParentArea = 0;

	memset(m_GroundItem, 0, sizeof(m_GroundItem));
	memset(m_PickStatus, 0, sizeof(m_PickStatus));

	// 验证并赋值，如果完成了验证，也就同时复制了数据
	MY_ASSERT(pParam->ItemsInPack <= g_PackageMaxItem);

	for (int i=0; i<pParam->ItemsInPack; i++)
	{
		if (!pParam->pItem[i])
			return 0;

		memcpy(&m_GroundItem[i], pParam->pItem[i], sizeof(SRawItemBuffer));
	}

	// 验证通过
	m_xTile			= pParam->xTile;
	m_yTile			= pParam->yTile;
	m_ItemsInPack	= pParam->ItemsInPack;
	m_PackageModel	= pParam->PackageModel;
	m_Remain		= pParam->ItemsInPack + (pParam->Money ? 1 : 0);
	m_dwGID			= pParam->ProtectedGID;
	m_dwTeamID		= pParam->ProtectTeamID;
	m_dwSegTime		= timeGetTime();
	m_dwLifeTime	= (-1 != pParam->dwLife) ? (m_dwSegTime + pParam->dwLife) : pParam->dwLife;
	m_Money			= pParam->Money;	
	m_MoneyType		= pParam->MoneyType;
	m_IsDispatched	= pParam->ProtectTeamID ? false : true;

	for (size_t i = 0; i < g_PackageMaxItem; i++)
		m_IndexMask[i] = m_ExPosition[i] = i;

    return 1;
}

int CItem::AddObject(LPIObject pChild) { return 1; }

int CItem::DelObject(LPIObject pChild) { return 1; }

void CItem::OnClose() {}

void CItem::OnRun(void)
{
	if (m_dwLifeTime == 0xffffffff)
        return;

	if ((int)(timeGetTime() - m_dwLifeTime) > 0)
        RemoveMe();
}

SAAddGroundItemMsg *CItem::GetStateMsg()
{
	static SAAddGroundItemMsg msg;
	msg.ItemID = m_GroundItem[0].wIndex ? m_GroundItem[0].wIndex : 0 ;
	msg.dwGlobalID  = GetGID();
	msg.PackageMode = m_PackageModel;
	msg.wTileX		= m_xTile;
	msg.wTileY		= m_yTile;

	return &msg;
}



void CItem::RemoveMe()
{
	LPIObject PTemp = self.lock();

	if (m_pParentArea)
	{
		SADelObjectMsg msg;
		msg.dwGlobalID = GetGID();
		m_pParentArea->SendAdj(&msg, sizeof(msg), -1);

		m_pParentArea->DelObject(PTemp);
	}

	for (std::set<CPlayer *>::iterator it = m_Checker.begin(); it != m_Checker.end(); ++it)
		(*it)->m_pCheckItem = 0;
	
	m_Checker.clear();

	if (m_ChanceItems.size())
	{
		CGroundItemWinner::GetInstance().GroundItemGone(this);
		m_ChanceItems.clear();
	}

    m_bValid = false;
}

void CItem::UpdateItemPicked(WORD index)
{
	SAUpdateGroundItemMsg updateMsg;
	updateMsg.dwGlobalID = GetGID();
	updateMsg.wIndex	 = index;

	// 通知查看的人
	for (std::set<CPlayer *>::iterator it = m_Checker.begin(); it != m_Checker.end(); ++it)
		g_StoreMessage((*it)->m_ClientIndex, &updateMsg, sizeof(updateMsg));
}

BOOL CItem::StartDispatch(CPlayer *pChecker)
{
	MY_ASSERT(pChecker->m_dwTeamID);

	std::map<DWORD, Team>::iterator it = teamManagerMap.find(pChecker->m_dwTeamID);
	if (it == teamManagerMap.end())
		return FALSE;

	static CPlayer *pTeamMember[MAX_TEAM_MEMBER] = {0};

	std::vector<CPlayer *> fullMember;

	for (size_t i = 0; i < it->second.byMemberNum; i++)
	{
		pTeamMember[i] = it->second.stTeamPlayer[i].PlayerRef;
		fullMember.push_back(pTeamMember[i]);
	}

	if (m_Money)				// 先把钱分了
	{
		m_Money = 0;

		if (0 == --m_Remain)	// 这个包裹除了钱，没别的了		
		{
			RemoveMe();
			return FALSE;
		}
	}

	// 挨个查看每个道具
	for (size_t i = 0; i < m_ItemsInPack; i++)
	{
		const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(m_GroundItem[i].wIndex);
		if (!pData)
			return FALSE;

		if (pData->m_Color < it->second.byGiveColor)
		{
			rfalse(2, 1, "位置为%d的道具颜色小于分配品级，直接自由拾取~", i);
			m_PickStatus[i] = PIPT_FREE;
		}
		else
		{
			switch (it->second.byGiveMode)
			{
			case Team::TGM_TEAMLEADER:		// 队长分配
				{
					m_PickStatus[i] = PIPT_LEADER;
				}
				break;

			case Team::TGM_SCHOOL:			// 门派分配
				{
					std::vector<CPlayer *> onlySchool;
					std::vector<CPlayer *> otherSchool;
					
					for (size_t j = 0; j < it->second.byMemberNum; j++)
					{
						std::vector<CPlayer *> &destList = (pData->m_School == pTeamMember[j]->m_Property.m_School) ? onlySchool : otherSchool;
						destList.push_back(pTeamMember[j]);
					}

					if (onlySchool.empty())			// 没有对应门派，那么所有人摇号（包含道具无门派需求的情况！）
					{
						CGroundItemWinner::GetInstance().AddChanceItem(otherSchool, this, i);
					}
					else if (otherSchool.empty())	// 没有闲散人员，那么门派内摇号
					{
						CGroundItemWinner::GetInstance().AddChanceItem(onlySchool, this, i);
					}
					else
					{
						CGroundItemWinner::GetInstance().AddChanceItem(onlySchool, this, i);
						CGroundItemWinner::GetInstance().AddDummyChance(otherSchool, this, i);
					}

					m_PickStatus[i] = PIPT_TAKECHANCE;
				}
				break;

			case Team::TGM_FREE:			// 自由分配
				CGroundItemWinner::GetInstance().AddChanceItem(fullMember, this, i);
				m_PickStatus[i] = PIPT_TAKECHANCE;
				break;
			}
		}
	}

	m_IsDispatched = true;

	return TRUE;
}

BOOL CItem::GenerateNewGroundItem(CRegion *parentRegion, DWORD range, const CItem::SParameter &args, std::vector<SRawItemBuffer> &item, const LogInfo &log)
{
	if (!parentRegion)
		return FALSE;
	
	if (item.size() != args.ItemsInPack)
		return FALSE;

	CItem::SParameter param = args;

	for (int i=0; i<param.ItemsInPack; i++)
		param.pItem[i] = &item[i];

	if (range != 0)
	{
		WORD x = (WORD)(param.xTile - range + (rand() % (range * 2 + 1)));
		WORD y = (WORD)(param.yTile - range + (rand() % (range * 2 + 1)));

		param.xTile = x;
		param.yTile = y;
	}

	if (log.rcdtype)
	{
		std::string itemlog;
		SYSTEMTIME stime;
		GetLocalTime(&stime);
		char timelog[200]={0};
		sprintf(timelog,"[%d-%d-%d,%d:%d:%d],怪物[%d]\r\n",stime.wYear,stime.wMonth,stime.wDay,stime.wHour,stime.wMinute,stime.wSecond,log.rcdtype);
		itemlog.append(timelog);

		if (param.ItemsInPack)
		{
			for (int i=0; i<param.ItemsInPack; i++){
				sprintf(timelog,"掉落物品ID[%d],物品数量[%d]\r\n",param.pItem[i]->wIndex,param.pItem[i]->overlap);
				std::map<DWORD,WORD>::iterator itr = g_allitemCount.find(param.pItem[i]->wIndex);
				if (itr!=g_allitemCount.end() ){
					itr->second += param.pItem[i]->overlap;
				}
				else{
					g_allitemCount[param.pItem[i]->wIndex] = param.pItem[i]->overlap;
				}
			}
			itemlog.append(timelog);


		}

		if (param.Money)
		{
			sprintf(timelog,"掉落货币类型[%d],货币数量[%d]\r\n",param.MoneyType,param.Money);
			itemlog.append(timelog);
			std::map<WORD,DWORD>::iterator itr = g_allmoneyCount.find(param.MoneyType);
			if (itr!=g_allmoneyCount.end() ){
				itr->second += param.Money;
			}
			else{
				g_allmoneyCount[param.MoneyType] = param.Money;
			}
		}

		g_strItemlog.push_back(itemlog);
		if (g_strItemlog.size()>50)
		{
			for(int i=0;i<g_strItemlog.size();i++){
				rfalse(6,1,g_strItemlog[i].c_str());
			}
			g_strItemlog.clear();
		}

		//rfalse(6,1,itemlog.c_str());
	}

	return parentRegion->Move2Area(parentRegion->GenerateObject(IID_ITEM, 0, (LPARAM)&param), param.xTile, param.yTile, true);
}

void CItem::SaveAllLog()
{
	if (g_strItemlog.size())
	{
		for(int i=0;i<g_strItemlog.size();i++){
			rfalse(6,1,g_strItemlog[i].c_str());
		}
		g_strItemlog.clear();
	}


}
