#include "StdAfx.h"
#include "SingleItem.h"
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
extern std::vector<std::string> g_strItemlog;
extern std::map<DWORD,WORD> g_allitemCount;
extern std::map<WORD,DWORD> g_allmoneyCount;

CSingleItem::CSingleItem(void) {}

CSingleItem::~CSingleItem(void) {}

int CSingleItem::OnCreate(_W64 long pParameter)
{
	SParameter *pParam = (SParameter *)pParameter;

	m_pParentArea = 0;

	memset(&m_GroundItem, 0, sizeof(m_GroundItem));
	m_PickStatus = PIPT_FREE ;


	// 验证并赋值，如果完成了验证，也就同时复制了数据
	//MY_ASSERT(pParam->ItemsInPack <= g_PackageMaxItem);

	//验证物品未通过
	if (pParam->ItemsID != pParam->pItem->wIndex)return 0;
	
	
	memcpy(&m_GroundItem, pParam->pItem, sizeof(SRawItemBuffer));
	
	
	// 验证通过
	m_xTile			= pParam->xTile;
	m_yTile			= pParam->yTile;
	m_ItemsNum	= pParam->ItemsNum;
	m_PackageModel	= pParam->PackageModel;
	m_dwGID			= pParam->ProtectedGID;
	m_dwTeamID		= pParam->ProtectTeamID;
	m_dwSegTime		= timeGetTime();
	m_dwLifeTime	= (-1 != pParam->dwLife) ? (m_dwSegTime + pParam->dwLife) : pParam->dwLife;
	m_IsDispatched	= pParam->ProtectTeamID ? false : true;
	m_ItemsID = pParam->ItemsID ;

	m_IndexMask = m_ExPosition =0;

    return 1;
}

int CSingleItem::AddObject(LPIObject pChild) { return 1; }

int CSingleItem::DelObject(LPIObject pChild) { return 1; }

void CSingleItem::OnClose() {}

void CSingleItem::OnRun(void)
{
	if (m_dwLifeTime == 0xffffffff)
        return;

	if ((int)(timeGetTime() - m_dwLifeTime) > 0)
        RemoveMe();
}

SAAddGroundItemMsg *CSingleItem::GetStateMsg()
{
	static SAAddGroundItemMsg msg;
	msg.ItemID = m_GroundItem.wIndex ? m_GroundItem.wIndex : 0 ;
	msg.dwGlobalID  = GetGID();
	msg.ItemNum = m_ItemsNum;
	msg.PackageMode = m_PackageModel;
	msg.wTileX		= m_xTile;
	msg.wTileY		= m_yTile;

	return &msg;
}



void CSingleItem::RemoveMe()
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
	{
		if (*it){
			(*it)->m_pSingleItem = 0;
		}
	}
		

	m_Checker.clear();

	if (m_ChanceItems.size())
	{
		CGroundItemWinner::GetInstance().GroundItemGone(this);
		m_ChanceItems.clear();
	}
    m_bValid = false;
}

void CSingleItem::UpdateItemPicked(WORD index)
{
	return ;
}

BOOL CSingleItem::GenerateNewGroundItem(CRegion *parentRegion, DWORD range, const CSingleItem::SParameter &args, SRawItemBuffer &item, const LogInfo &log)
{
	if (!parentRegion)
		return FALSE;

	CSingleItem::SParameter param = args;

	param.pItem = &item;

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
		sprintf(timelog,"%d-%d-%d %d:%d:%d\t%d",stime.wYear,stime.wMonth,stime.wDay,stime.wHour,stime.wMinute,stime.wSecond,log.rcdtype);
		itemlog.append(timelog);

		sprintf(timelog,"\t%d\t%d",param.ItemsID,param.ItemsNum);
		std::map<DWORD,WORD>::iterator itr = g_allitemCount.find(param.ItemsID);
		if (itr!=g_allitemCount.end() ){
			itr->second += param.ItemsNum;
		}
		else{ 
			g_allitemCount[param.ItemsID] = param.ItemsNum;
		}
		itemlog.append(timelog);
		g_strItemlog.push_back(itemlog);
		if (g_strItemlog.size()>5)
		{
			for(int i=0;i<g_strItemlog.size();i++){
				rfalse(6,1,g_strItemlog[i].c_str());
			}
			g_strItemlog.clear();
		}

		//rfalse(6,1,itemlog.c_str());
	}

	return parentRegion->Move2Area(parentRegion->GenerateObject(IID_SINGLEITEM, 0, (LPARAM)&param), param.xTile, param.yTile, true);
}

void CSingleItem::SaveAllLog()
{
	if (g_strItemlog.size())
	{
		for(int i=0;i<g_strItemlog.size();i++){
			rfalse(6,1,g_strItemlog[i].c_str());
		}
		g_strItemlog.clear();
	}
	std::map<DWORD,WORD>::iterator itr = g_allitemCount.begin();
	char allitem[100]={0};
	sprintf(allitem,"%s\t%s","掉落ID","掉落总数");
	rfalse(6,1,allitem);
	while(itr != g_allitemCount.end())
	{
		sprintf(allitem,"%d\t%d",itr->first,itr->second);
		rfalse(6,1,allitem);
		itr++;
	}
}

BOOL CSingleItem::StartDispatch( CPlayer *pChecker )
{
	MY_ASSERT(pChecker->m_dwTeamID);

	std::map<DWORD, Team>::iterator it = teamManagerMap.find(pChecker->m_dwTeamID);
	if (it == teamManagerMap.end())
		return FALSE;

	static CPlayer *pTeamMember[MAX_TEAM_MEMBER] = {0};

	std::vector<CPlayer *> fullMember;

	size_t i = 0;
	for (; i < it->second.byMemberNum; i++)
	{
		pTeamMember[i] = it->second.stTeamPlayer[i].PlayerRef;
		fullMember.push_back(pTeamMember[i]);
	}

		const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(m_ItemsID);
		if (!pData)
			return FALSE;

		if (pData->m_Color < it->second.byGiveColor)
		{
			rfalse(2, 1, "位置为%d的道具颜色小于分配品级，直接自由拾取~", i);
			m_PickStatus = PIPT_FREE;
		}
		else
		{
			switch (it->second.byGiveMode)
			{
			case Team::TGM_TEAMLEADER:		// 队长分配
				{
					m_PickStatus = PIPT_LEADER;
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
						CGroundItemWinner::GetInstance().AddChanceItem(otherSchool, this);
					}
					else if (otherSchool.empty())	// 没有闲散人员，那么门派内摇号
					{
						CGroundItemWinner::GetInstance().AddChanceItem(onlySchool, this);
					}
					else
					{
						CGroundItemWinner::GetInstance().AddChanceItem(onlySchool,this);
						CGroundItemWinner::GetInstance().AddDummyChance(otherSchool, this);
					}

					m_PickStatus = PIPT_TAKECHANCE;
				}
				break;

			case Team::TGM_FREE:			// 自由分配
				CGroundItemWinner::GetInstance().AddChanceItem(fullMember, this);
				m_PickStatus = PIPT_TAKECHANCE;
				break;
			}
		}
	//}

	m_IsDispatched = true;

	return TRUE;
}
