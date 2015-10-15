#include "stdafx.h"
#include "TeamManager.h"
#include "Player.h"
#include "PlayerManager.h"
#include "Area.h"
#include "Region.h"
#include "pub/smcacheex.h"
#include "randomsys.h"
#include <time.h>
#include "NetWorkModule/OrbMsgs.h"
#include "NetWorkModule/TeamMsgs.h"
#include "networkmodule\scriptmsgs.h"
#include "networkmodule\UpgradeMsgs.h"
#include "GameWorld.h"
#include "networkmodule/FightMsgs.h"
#include "CMakeTeamQuestService.h"
#include "CItemDefine.h"
#include "GameObjects/DynamicRegion.h"
void RefreshTeam(Team &team);
void UpdatePlayer(Team &teamInfo, int nNum, CPlayer *pPlayer);
void OnRecvJoinMsg(DNID dnidClient, SQTeamJoinMsgG *pMsg, CPlayer *pPlayer, bool Auto = false);
void DelMember(const char* szName, CPlayer* pPlayer, bool IsKick = false);
BOOL AddItemToTeamPack(CPlayer *pPlayer, CItem *pItem, WORD index);
BOOL AddItemToTeamPack(CPlayer *pPlayer, CSingleItem *pItem);
void KickAllTeamPlayer(Team team);
void KickToTeamPlayer(Team team,const char* szname);
static void OnRecvTeamTalkMsg(DNID dnidClient, SQTeamTalkMsgG *pMsg, CPlayer *pPlayer);
static void OnRecvTeamTalkMsg(DNID dnidClient, SQChatTeam *pMsg, CPlayer *pPlayer);
static void OnRecvTeamKickMsg(DNID dnidClient, SQTeamKickMsgG *pMsg, CPlayer *pPlayer);
static void OnRecvTeamSetHeadmen(DNID dnidClient, SQCSTeamBaseHeadmenMsg* pMsg, CPlayer *pPlayer);
static void AddMember(CPlayer *pPlayer, const DWORD dwTeamId, bool IsLeader = false);
static void DelTeam(const DWORD dwHandle);
static void SendAllMember( Team *stManager, LPVOID pMsg, WORD wSize );
static BOOL CanInviteOther(CPlayer *pLeader, CPlayer *pMember);
static BOOL CanJoinOther(CPlayer *pOther, CPlayer *pNewMember);
static DWORD CreateTeam();
static void OnRecvGetSubmitTeamRequest(DNID dnidClient, SQGetTeamQuestMsg *pMsg, CPlayer *pPlayer);
static void OnRecvSubmitTeamRequest(DNID dnidClient, SQSubmitMakeTeamMsg *pMsg, CPlayer *pPlayer);
static void OnRecvGetCurrentTeamList(DNID dnidClient, SQFindTeamInfoMsg *pMsg, CPlayer *pPlayer);
static void OnRecvChangeGiveMode(DNID dnidClient, SQAChangeGiveMode *pMsg, CPlayer *pPlayer);
static void OnRecvDispatchItem(DNID dnidClient, SQDispatchToMemberMsg *pMsg, CPlayer *pPlayer);
extern LPIObject GetPlayerByGID(DWORD);

extern CGameWorld *&GetGW();

std::map<DWORD, Team> teamManagerMap;

TeamManager::~TeamManager()
{
	teamManagerMap.clear();
}

void TeamManager::Run()
{
	for (std::map<DWORD, Team>::iterator it = teamManagerMap.begin(); it != teamManagerMap.end(); ++it)
	{
		Team &team = it->second;

		// ���¶�����Ϣ
		for (size_t i = 0; i < team.byMemberNum; i++)
		{
			//MY_ASSERT(team.stTeamPlayer[i].PlayerRef);
			UpdatePlayer(team, i, team.stTeamPlayer[i].PlayerRef);
		}

		RefreshTeam(team);
	}
}

int TeamManager::OnDispatch(DNID dnidClient, SQChatTeam *pMsg, CPlayer *pPlayer)
{
	if (!pMsg || 0 == dnidClient || !pPlayer)
		return 0;

	if (!pPlayer->m_ParentRegion || !pPlayer->m_ParentArea)
		return 0;
	OnRecvTeamTalkMsg(dnidClient, (SQChatTeam *)pMsg, pPlayer);
	return 0;
}

int TeamManager::OnDispatch(DNID dnidClient, SMessage *pMsg, CPlayer *pPlayer)
{
	if (!pMsg || 0 == dnidClient || !pPlayer)
		return 0;

	if (!pPlayer->m_ParentRegion || !pPlayer->m_ParentArea)
		return 0;

	if (pMsg->_protocol != SMessage::EPRO_TEAM_MESSAGE)
		return 0;

	STeamBaseMsgG *pTb = (STeamBaseMsgG *)pMsg;

    switch (pTb->_protocol)
    {
	case STeamBaseMsgG::EPRO_TEAM_JOIN:
		OnRecvJoinMsg(dnidClient, (SQTeamJoinMsgG*)pMsg, pPlayer);
		break;

	case STeamBaseMsgG::EPRO_TEAM_TALK:
		OnRecvTeamTalkMsg(dnidClient, (SQTeamTalkMsgG *)pMsg, pPlayer);
		break;

    case STeamBaseMsgG::EPRO_TEAM_KICK:
		OnRecvTeamKickMsg(dnidClient, (SQTeamKickMsgG*)pMsg, pPlayer);
		break;

	case STeamBaseMsgG::EPRO_TEAM_HEADMEN:
		OnRecvTeamSetHeadmen(dnidClient,(SQCSTeamBaseHeadmenMsg*)pMsg, pPlayer);
		break;

	case STeamBaseMsgG::EPRO_TEAM_CHANGEGIVEMODE:
		OnRecvChangeGiveMode(dnidClient, (SQAChangeGiveMode *)pMsg, pPlayer);
		break;

	case STeamBaseMsgG::EPRO_TEAM_DISPATCHTOMEMBER:
		OnRecvDispatchItem(dnidClient, (SQDispatchToMemberMsg *)pMsg, pPlayer);
		break;

	case STeamBaseMsgG::EPRO_TEAM_SUBMIT_REQUEST:
		OnRecvSubmitTeamRequest(dnidClient, (SQSubmitMakeTeamMsg *)pMsg, pPlayer);
		break;

	case STeamBaseMsgG::EPRO_TEAM_GETTEAMQUEST:
		OnRecvGetSubmitTeamRequest(dnidClient, (SQGetTeamQuestMsg *)pMsg, pPlayer);

	case STeamBaseMsgG::EPRO_TEAM_GETFINDTEAMINFO:
		OnRecvGetCurrentTeamList(dnidClient, (SQFindTeamInfoMsg *)pMsg, pPlayer);
		break;

	case STeamBaseMsgG::EPRO_TEAM_INVITEWITHNAME:
		{
			SQInviteWithNameMsg *pDestMsg = (SQInviteWithNameMsg *)pMsg;
			if (!pDestMsg)
			{
				return 0;
			}
			pDestMsg->szName[CONST_USERNAME-1] = 0;
	
			CPlayer *destPlayer = (CPlayer *)GetPlayerByName(pDestMsg->szName)->DynamicCast(IID_PLAYER);
			if (!destPlayer || !destPlayer->m_ParentRegion)
				break;
	
			OnRecvTeamMessage(dnidClient, destPlayer->GetGID(), true, pPlayer, destPlayer);
		}
		break;

	case STeamBaseMsgG::EPRO_TEAM_IWANTTOJION:
		{
			SQWantJoinTeamMsg *pDestMsg = (SQWantJoinTeamMsg *)pMsg;
			if (!pDestMsg)
			{
				return 0;
			}
			CPlayer::SyncFindTeamList::iterator init = pPlayer->m_LastSynFindTeamIDList.find(pDestMsg->dwTeamID);
			if (init == pPlayer->m_LastSynFindTeamIDList.end())
				break;

			// ��һ����ȫ���б���
			if (!CMakeTeamQuestService::GetInstance().IsEmptyTeamExist(pDestMsg->dwTeamID))
				break;

			std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pDestMsg->dwTeamID);
			MY_ASSERT(iter != teamManagerMap.end());

			Team &team = iter->second;
			
			// �ҵ��ӳ�
			CPlayer *Leader = (CPlayer *)GetPlayerByGID(team.LeaderGID)->DynamicCast(IID_PLAYER);
			if (!Leader || !Leader->m_ParentRegion)
				break;

			OnRecvTeamMessage(dnidClient, team.LeaderGID, false, pPlayer, Leader);
		}
		break;

	case STeamBaseMsgG::EPRO_TEAM_CANCELTEAMQUEST:
		{
			if (pPlayer->m_SendRequestID)
			{
				CMakeTeamQuestService::GetInstance().RemoveQuest(pPlayer->m_SendRequestID);
				pPlayer->m_SendRequestID = 0;
			}
		}
		break;
    }
	
	return 1;
}

static void OnRecvGetCurrentTeamList(DNID dnidClient, SQFindTeamInfoMsg *pMsg, CPlayer *pPlayer)
{
	if (!pPlayer)
	{
		rfalse(4,1,"void OnRecvGetCurrentTeamList");
		return;
	}
	if (0 == pPlayer->m_CurHp)
		return;

	SANotifyOldFindMsg oldList;
	oldList.wNumber = 0;

	// ���ȿ�����û�б�ɾ���ģ����¹��Ķ���/�ѽ�ɢ���Ķ��飩
	for (CPlayer::SyncFindTeamList::iterator it = pPlayer->m_LastSynFindTeamIDList.begin(); it != pPlayer->m_LastSynFindTeamIDList.end();)
	{
		MY_ASSERT(it->first);

		if (!CMakeTeamQuestService::GetInstance().IsEmptyTeamExist(it->first) || it->second != CMakeTeamQuestService::GetInstance().GetEmptyTeamUpdateTime(it->first))
		{
			if (oldList.wNumber != SANotifyOldFindMsg::NOFM_OLDMAX)
				oldList.dwOldList[oldList.wNumber++] = it->first;

			it = pPlayer->m_LastSynFindTeamIDList.erase(it);
		}
		else
			++it;
	}

	if (oldList.wNumber)
		g_StoreMessage(pPlayer->m_ClientIndex, &oldList, oldList.GetMySize());

	SANotifyNewFindMsg newList;
	newList.wNumber = 0;

	// �ٿ�����û���µ�
	CMakeTeamQuestService::HasPosTeamList &tempList = CMakeTeamQuestService::GetInstance().GetEmptyTeamList();

	for (CMakeTeamQuestService::HasPosTeamList::iterator it = tempList.begin(); it != tempList.end(); ++it)
	{
		CPlayer::SyncFindTeamList::iterator hasOne = pPlayer->m_LastSynFindTeamIDList.find(it->first);
		if (hasOne != pPlayer->m_LastSynFindTeamIDList.end())
			continue;

		// �Լ������ÿ��Լ��Ķ�����
		if (pPlayer->m_dwTeamID == it->first)
			continue;

		std::map<DWORD, Team>::iterator iter = teamManagerMap.find(it->first);
		MY_ASSERT(iter != teamManagerMap.end());

		Team &team = iter->second;

		MY_ASSERT(team.byMemberNum >= 2 && team.byMemberNum < 5);

		newList.dwNewList[newList.wNumber].teamID = it->first;

		for (size_t i = 0; i < 5; ++i)
		{
			newList.dwNewList[newList.wNumber].teamMember[i].bySex	  = team.stTeamPlayer[i].bySex;
			newList.dwNewList[newList.wNumber].teamMember[i].bySchool = team.stTeamPlayer[i].bySchool;
			newList.dwNewList[newList.wNumber].teamMember[i].byLevel  = team.stTeamPlayer[i].byLevel;
			newList.dwNewList[newList.wNumber].teamMember[i].isLeader = team.stTeamPlayer[i].isLeader;
			newList.dwNewList[newList.wNumber].teamMember[i].status   = team.stTeamPlayer[i].status;

			dwt::strcpy(newList.dwNewList[newList.wNumber].teamMember[i].szName, team.stTeamPlayer[i].szName, CONST_USERNAME);
			newList.dwNewList[newList.wNumber].teamMember[i].szName[CONST_USERNAME-1] = 0;
		}

		// SaveIt����
		pPlayer->m_LastSynFindTeamIDList[it->first] = it->second;

		++newList.wNumber;
	}

	if (newList.wNumber)
		g_StoreMessage(pPlayer->m_ClientIndex, &newList, newList.GetMySize());

	return;
}

static void OnRecvGetSubmitTeamRequest(DNID dnidClient, SQGetTeamQuestMsg *pMsg, CPlayer *pPlayer)
{
	if (!pPlayer)
	{
		rfalse(4,1,"void OnRecvGetSubmitTeamRequestΪNULL");
		return;
	}
	if (0 == pPlayer->m_CurHp)			// �����̱���״̬
		return;

	SANotifyOldQuestMsg oldList;
	oldList.wNumber = 0;

	// ���ȿ�����û�б�ɾ����
	for (CPlayer::SyncTeamQuestList::iterator it = pPlayer->m_LastSynTeamRequestIDList.begin(); it != pPlayer->m_LastSynTeamRequestIDList.end();)
	{
		MY_ASSERT(*it);

		if (!CMakeTeamQuestService::GetInstance().IsQuestExist(*it))
		{
			// ˵����������Ѿ���ȡ�����Ѿ��������ӡ�������
			if (oldList.wNumber != SANotifyOldQuestMsg::NOQM_OLDMAX)
				oldList.dwOldList[oldList.wNumber++] = *it;

			it = pPlayer->m_LastSynTeamRequestIDList.erase(it);
		}
		else
			++it;
	}

	if (oldList.wNumber)
		g_StoreMessage(pPlayer->m_ClientIndex, &oldList, oldList.GetMySize());

	SANotifyNewQuestMsg newList;
	newList.wNumber = 0;

	// �ٿ�����û���µ�
	CMakeTeamQuestService::MTQuestList &tempList = CMakeTeamQuestService::GetInstance().GetQuestList();
	CMakeTeamQuestService::MTQuestList::iterator newBegin = tempList.end();
	
	MY_ASSERT(tempList.size() >= pPlayer->m_LastSynTeamRequestIDList.size());

	if (pPlayer->m_LastSynTeamRequestIDList.empty())
		newBegin = tempList.begin();
	else
	{
		newBegin = tempList.find(*(--pPlayer->m_LastSynTeamRequestIDList.end()));
		MY_ASSERT(newBegin != tempList.end());
		++newBegin;
	}

	for (; newBegin != tempList.end(); ++newBegin)
	{
		// �Լ�������Ͳ����Լ���
		if (newBegin->second.dwIndex == pPlayer->m_SendRequestID)
			continue;

		if (newList.wNumber != SANotifyNewQuestMsg::NNQM_NEWMAX)
			newList.dwNewList[newList.wNumber++] = newBegin->second;

		pPlayer->m_LastSynTeamRequestIDList.insert(newBegin->second.dwIndex);
	}

	if (newList.wNumber)
		g_StoreMessage(pPlayer->m_ClientIndex, &newList, newList.GetMySize());

	return;
}

static void OnRecvSubmitTeamRequest(DNID dnidClient, SQSubmitMakeTeamMsg *pMsg, CPlayer *pPlayer)
{
	if (!pPlayer || !pMsg)
	{
		rfalse(4,1,"void OnRecvSubmitTeamRequestΪNULL");
		return;
	}
	if (0 == pPlayer->m_CurHp)			// �����̱���״̬
		return;

	if (pPlayer->m_dwTeamID)			// �Ѿ��ڶ�����
		return;

	if (pPlayer->m_SendRequestID)		// �Ѿ��ύ������
	{
		MY_ASSERT(CMakeTeamQuestService::GetInstance().IsQuestExist(pPlayer->m_SendRequestID));
		return;
	}

	TRWithIndex teamQuest;
	teamQuest.dwGID		= pPlayer->GetGID();
	teamQuest.bySex		= pPlayer->m_Property.m_Sex;
	teamQuest.bySchool	= pPlayer->m_Property.m_School;
	teamQuest.byLevel	= pPlayer->m_Property.m_Level;

	dwt::strcpy(teamQuest.szName, pPlayer->m_Property.m_Name, CONST_USERNAME);
	teamQuest.szName[CONST_USERNAME-1] = 0;

	dwt::strcpy(teamQuest.szInfo, pMsg->szInfo, TeamRequest::PTR_MAXINFO);
	teamQuest.szInfo[TeamRequest::PTR_MAXINFO-1] = 0;

	CMakeTeamQuestService::GetInstance().InsertQuest(teamQuest);

	// SaveIt��
	pPlayer->m_SendRequestID = teamQuest.dwIndex;

	SASubmitMakeTeamMsg backMsg;
	backMsg.bReulst = SASubmitMakeTeamMsg::SMT_SUCCESS;

	g_StoreMessage(pPlayer->m_ClientIndex, &backMsg, sizeof(SASubmitMakeTeamMsg));
}

static void OnRecvDispatchItem(DNID dnidClient, SQDispatchToMemberMsg *pMsg, CPlayer *pPlayer)
{
	if (!pPlayer || !pMsg)
	{
		rfalse(4,1,"void OnRecvDispatchItem");
		return;
	}
	if (0 == pPlayer->m_CurHp)
		return;

	if (0 == pPlayer->m_dwTeamID)
		return;

	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pPlayer->m_dwTeamID);
	if (iter == teamManagerMap.end())
		return;

	Team &team = iter->second;

	if (!pPlayer->IsTeamLeader())
		return;

	if (0 == team.LeaderPack[pMsg->ItemIndex].wIndex || pMsg->MemIndex >= team.byMemberNum)
		return;

	CPlayer *destPlayer = team.stTeamPlayer[pMsg->MemIndex].PlayerRef;
	if (!destPlayer)
	{
		return;
	}
	MY_ASSERT(destPlayer);

	SADispatchToMemberMsg disResult;
	disResult.Result	= SADispatchToMemberMsg::SDTM_PACKFULL;
	disResult.MemIndex	= pMsg->MemIndex;

	std::list<SAddItemInfo> itemList;
	itemList.push_back(SAddItemInfo(team.LeaderPack[pMsg->ItemIndex].wIndex, team.LeaderPack[pMsg->ItemIndex].overlap));
	
	if (destPlayer->CanAddItems(itemList))		// �����㹻
	{
		const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(team.LeaderPack[pMsg->ItemIndex].wIndex);
		MY_ASSERT(pData);

		WORD willCreate = destPlayer->TryPossibleMerge(team.LeaderPack[pMsg->ItemIndex].wIndex, team.LeaderPack[pMsg->ItemIndex].overlap, pData->m_Overlay);
		destPlayer->InsertItem(team.LeaderPack[pMsg->ItemIndex].wIndex, willCreate, pData, team.LeaderPack[pMsg->ItemIndex], false);

		memset(&team.LeaderPack[pMsg->ItemIndex], 0, sizeof(SRawItemBuffer));
		team.PackCount--;

		SADelTempItemMsg delMsg;
		delMsg.ItemIndex = pMsg->ItemIndex;
		SendAllMember(&team, &delMsg, sizeof(SADelTempItemMsg));

		disResult.Result = SADispatchToMemberMsg::SDTM_SUCCESS;
	}

	g_StoreMessage(pPlayer->m_ClientIndex, &disResult, sizeof(disResult));

	return;
}

static void OnRecvChangeGiveMode(DNID dnidClient, SQAChangeGiveMode *pMsg, CPlayer *pPlayer)
{
	if (!pMsg || !pPlayer)
	{
		rfalse(4,1,"void OnRecvChangeGiveMode");
		return;
	}
	if (0 == pPlayer->m_CurHp)
		return;

	if (0 == pPlayer->m_dwTeamID)
		return;

	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pPlayer->m_dwTeamID);
	if (iter == teamManagerMap.end())
		return;

	Team &team = iter->second;

	if (!pPlayer->IsTeamLeader())
		return;

	if (SQAChangeGiveMode::SCGM_MODE == pMsg->bOperation)
	{
		if (pMsg->bMode < Team::TGM_TEAMLEADER || pMsg->bMode > Team::TGM_FREE)
			return;

		team.byGiveMode = pMsg->bMode;
	}
	else if (SQAChangeGiveMode::SCGM_COLOER == pMsg->bOperation)
	{
		if (pMsg->bMode < Team::TGC_BLUE || pMsg->bMode > Team::TGC_GOLD)
			return;

		team.byGiveColor = pMsg->bMode;
	}

	// ���ظ���Ա
	SendAllMember(&team, pMsg, sizeof(SQAChangeGiveMode));

	return;
}

static void OnRecvTeamSetHeadmen(DNID dnidClient, SQCSTeamBaseHeadmenMsg* pMsg, CPlayer *pPlayer)
{
	if (!pPlayer || !pMsg)
	{
		rfalse(4,1,"OnRecvTeamSetHeadmen");
		return;
	}
	if (0 == pPlayer->m_CurHp)
		return;
	
	if (0 == pPlayer->m_dwTeamID)
		return;

	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pPlayer->m_dwTeamID);
	if (iter == teamManagerMap.end())
		return;

	Team &team = iter->second;

	if (pMsg->pos >= team.byMemberNum)
		return;

	if (!pPlayer->IsTeamLeader())
		return;

	// �Լ����������Լ�
	if (pPlayer->GetGID() == team.stTeamPlayer[pMsg->pos].dwGlobal)
		return;

	int curTeam = team.FindNum(pPlayer->m_Property.m_dwStaticID);
	if (-1 == curTeam || (curTeam >= team.byMemberNum))
		return;

	MY_ASSERT(team.stTeamPlayer[pMsg->pos].PlayerRef);

	// ��֤ͨ�������Կ�ʼ������
	team.stTeamPlayer[curTeam].isLeader = false;
	pPlayer->m_bIsTeamLeader = false;
	team.stTeamPlayer[pMsg->pos].isLeader = true;
	team.stTeamPlayer[pMsg->pos].PlayerRef->m_bIsTeamLeader = true;
	team.LeaderGID = team.stTeamPlayer[pMsg->pos].dwGlobal;

	SACSTeamBaseHeadmenMsg backMsg;
	backMsg.pos = pMsg->pos;
	SendAllMember(&team, &backMsg, sizeof(SACSTeamBaseHeadmenMsg));

	CMakeTeamQuestService::GetInstance().UpdateEmptyTeam(pPlayer->m_dwTeamID);
}

BOOL TeamManager::OnRecvTeamMessage(DNID dnidClient, DWORD dwDestGID, bool IsJoinMe, CPlayer *pSource, CPlayer *pDst)
{
	CPlayer *pDest = 0;
    if (!pSource)
    {
		rfalse(4,1,"TeamManager::OnRecvTeamMessageΪNULL");
		return FALSE;
    }
	if (!pDst)					// ����Զ�̾��ڽ�����
		pDest = (CPlayer *)pSource->m_ParentRegion->SearchObjectListInAreas(dwDestGID, pSource->m_ParentArea->m_X, pSource->m_ParentArea->m_Y)->DynamicCast(IID_PLAYER);
    else
		pDest = pDst;

	// Ѱ�ҶԷ�ʧ��
	if (!pDest || 0 == pDest->m_CurHp)
		return FALSE;

	if (pSource->GetGID() == pDest->GetGID())
		return FALSE;

	// �ӳ�����
	CPlayer *pLeader = 0;

	// �ж��Ƿ��Ѿ������Ŀ�귢��������
	CPlayer::TeamQuestList::iterator hasSendOut = pSource->m_TeamSentOut.find(pDest->GetGID());
	if (hasSendOut != pSource->m_TeamSentOut.end())
		return FALSE;

	// �ж���ӹ���
	if (IsJoinMe)				// �������
	{
		if (!CanInviteOther(pSource, pDest))
			return FALSE;

		// ���
		pSource->m_TeamSentOut[pDest->GetGID()].first  = pDest->m_TeamGetIn[pSource->GetGID()].first = timeGetTime();
		pSource->m_TeamSentOut[pDest->GetGID()].second = pDest->m_TeamGetIn[pSource->GetGID()].second = IsJoinMe ? SATeamJoinMsgG::TJM_JOINME : SATeamJoinMsgG::TJM_JOINOTHER;
	}
	else						// �������
	{
		if (0 == pDest->m_dwTeamID)
			return FALSE;

		std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pDest->m_dwTeamID);
		if (iter == teamManagerMap.end())
			return FALSE;

		if (pDest->m_bIsTeamLeader)
			pLeader = pDest;
		else
		{
			Team &team = iter->second;
			DWORD leaderGID = team.LeaderGID;
			pLeader = (CPlayer *)GetPlayerByGID(leaderGID)->DynamicCast(IID_PLAYER);
		}

		if (!pLeader)
			return FALSE;

		MY_ASSERT(pLeader->m_dwTeamID == pDest->m_dwTeamID);

		if (!CanJoinOther(pLeader, pSource))
			return FALSE;

		// ���
		pSource->m_TeamSentOut[pLeader->GetGID()].first  = pLeader->m_TeamGetIn[pSource->GetGID()].first = timeGetTime();
		pSource->m_TeamSentOut[pLeader->GetGID()].second = pLeader->m_TeamGetIn[pSource->GetGID()].second = IsJoinMe ? SATeamJoinMsgG::TJM_JOINME : SATeamJoinMsgG::TJM_JOINOTHER;
	}

	// ȷ������Ϣ���Է����������
	SATeamJoinMsgG stJoinMsg;
	stJoinMsg.type	= IsJoinMe ? SATeamJoinMsgG::TJM_JOINME : SATeamJoinMsgG::TJM_JOINOTHER;
	stJoinMsg.dwGID	= pSource->GetGID();
	dwt::strcpy(stJoinMsg.cName, pSource->GetName(), CONST_USERNAME);

	if (IsJoinMe)
		g_StoreMessage(pDest->m_ClientIndex, &stJoinMsg, sizeof(SATeamJoinMsgG));
	else
		g_StoreMessage(pLeader->m_ClientIndex, &stJoinMsg, sizeof(SATeamJoinMsgG));

	return TRUE;
}

static void OnRecvTeamTalkMsg(DNID dnidClient, SQTeamTalkMsgG *pMsg, CPlayer *pPlayer)
{
	if (!pPlayer || !pMsg)
	{
		rfalse(4,1,"OnRecvTeamTalkMsgΪNULL");
		return;
	}
	if (0 == pPlayer->m_dwTeamID)
		return;

	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pPlayer->m_dwTeamID);
    if (iter == teamManagerMap.end())
        return;

	if ((timeGetTime() - pPlayer->m_dwTeamLimitTime) > 1000)
	{
		pPlayer->m_dwTeamLimitTime = timeGetTime();

		pMsg->szTalkMsg[MAX_CHAT_LEN-1] = 0;
		pMsg->szTalkerName[CONST_USERNAME-1] = 0;

		Team &teamInfo = iter->second;

		dwt::strcpy(pMsg->szTalkerName, pPlayer->GetName(), sizeof(pMsg->szTalkerName));
		size_t size = dwt::strlen(pMsg->szTalkMsg, MAX_CHAT_LEN) + 1;
		
		// ֱ��ͨ��ORBת��
		SendAllMember(&teamInfo, pMsg, (WORD)(sizeof(SATeamTalkMsgG) - (sizeof(pMsg->szTalkMsg) - size)));
	}
}

static void OnRecvTeamTalkMsg(DNID dnidClient, SQChatTeam *pMsg, CPlayer *pPlayer)
{
	if (!pPlayer || !pMsg)
	{
		rfalse(4,1,"OnRecvTeamTalkMsgΪNULL");
		return;
	}
	if (0 == pPlayer->m_dwTeamID)
		return;

	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pPlayer->m_dwTeamID);
	if (iter == teamManagerMap.end())
		return;

	if ((timeGetTime() - pPlayer->m_dwTeamLimitTime) > 1000)
	{
		pPlayer->m_dwTeamLimitTime = timeGetTime();

		pMsg->cChatData[MAX_CHAT_LEN-1] = 0;
		pMsg->cTalkerName[CONST_USERNAME-1] = 0;

		Team &teamInfo = iter->second;

		dwt::strcpy(pMsg->cTalkerName, pPlayer->GetName(), sizeof(pMsg->cTalkerName));
		size_t size = dwt::strlen(pMsg->cChatData, MAX_CHAT_LEN) + 1;

		// ֱ��ͨ��ORBת��
		SendAllMember(&teamInfo, pMsg, (WORD)(sizeof(SAChatTeam) - (sizeof(pMsg->cChatData) - size)));
	}
}

static void OnRecvTeamKickMsg(DNID dnidClient, SQTeamKickMsgG *pMsg, CPlayer *pPlayer)
{
	if (!pMsg || !pPlayer)
	{
		rfalse(4,1,"OnRecvTeamKickMsgΪ��");
		return;
	}
	if (0 == pPlayer->m_dwTeamID || 0 == pPlayer->m_CurHp)
		return;

	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pPlayer->m_dwTeamID);
    if (iter == teamManagerMap.end())
        return;

	if (SQTeamKickMsgG::TKM_DISMISSED == pMsg->operation && pPlayer->IsTeamLeader())
	{
		//�������˳�
		KickAllTeamPlayer(iter->second);
		DelTeam(pPlayer->m_dwTeamID);
		return;
	}

	BOOL isSelf = (dwt::strcmp(pPlayer->GetName(), pMsg->szName, sizeof(pMsg->szName)) == 0);

	if (SQTeamKickMsgG::TKM_QUIT == pMsg->operation && isSelf)		// �Լ��˳�
	{
		//if (iter->second.byMemberNum == 2) //�����еĶ�Ա��
		//{
		//	//�������˳�
		//	KickAllTeamPlayer(iter->second);
		//}
		//else
		{
			KickToTeamPlayer(iter->second,pMsg->szName);
			//pMsg->szName ���ߵ��˵�����
		}
		DelMember(pMsg->szName, pPlayer, false);
	}
	else if (SQTeamKickMsgG::TKM_KICKOFF == pMsg->operation && pPlayer->IsTeamLeader() && !isSelf)
	{
		//if (iter->second.byMemberNum == 2) //�����еĶ�Ա��
		//{
		//	//�������˳�
		//	KickAllTeamPlayer(iter->second);
		//}
		//else
		{
			//pMsg->szName ���ߵ��˵�����
			KickToTeamPlayer(iter->second,pMsg->szName);
		}
		DelMember(pMsg->szName, pPlayer, true);
	}
	
	return;
}

//Buffͬ����Ϣ
static void OnSynBuffState( CPlayer *pPlayer, const DWORD dwTeamId)
{
	if(pPlayer == NULL || dwTeamId == 0)
		return;

	//����BUFF��Ϣ������

//	extern std::map< DWORD, Team > teamManagerMap;
//	std::map< DWORD, Team >::iterator iter = teamManagerMap.find( dwTeamId );
//	if ( iter != teamManagerMap.end() )
//	{
// 		Team team = iter->second;
// 
// 		SASetBuffInfo msg;
// 		BuffContainer::BUFFMAP buffmap = pPlayer->buffContainer.GetBuffMap();
// 		if (buffmap.size() == 0)
// 			return;
// 
// 		msg.bIsTeamMsg = true;
// 		msg.dwTargetGID = pPlayer->GetGID();
// 
// 		BuffContainer::BUFFMAP::iterator it = buffmap.begin();
// 		for (int i = 0;it != buffmap.end();it++,i++)
// 		{
// 			DWORD oldtime = it->second->GetBuffSegment();
// 			int maxTime   = it->second->GetBuffDuration() * 1000;
// 			DWORD curTime = timeGetTime();
// 			BOOL isForever = it->second->IsForeverBuff();
// 			BYTE ret = 0;
// 			if (maxTime != 0)
// 			{
// 				ret = (BYTE)((float)((float)(curTime - oldtime) / (float)maxTime) * 100.0);
// 			}
// 
// 			msg.stbuffinfo[i].dwBuffID = it->second->GetBuffId();
// 			msg.stbuffinfo[i].bProcessTime = ret;
// 			msg.stbuffinfo[i].ticks		=	maxTime;
// 			//����BUFF���⴦��
// 			if(isForever)
// 				msg.stbuffinfo[i].ticks = 0xffffffff;
// 		}
// 		for( int n = 0; n < 8; n++ )
// 		{
// 			if( team.stTeamPlayer[n].szName[0] == 0 )
// 				continue;
// 			g_StoreMessage( team.stTeamPlayer[n].dnidPlayer, &msg, sizeof(SASetBuffInfo) );
// 		}
//	}
	return;
}

// Ӧ�𷽵Ĵ�
void OnRecvJoinMsg(DNID dnidClient, SQTeamJoinMsgG *pMsg, CPlayer *pPlayer, bool Auto)
{
	if (!pPlayer || !pMsg)
	{
		rfalse(4,1," Ӧ�𷽵Ĵ� OnRecvJoinMsgΪNULL");
		return;
	}
	if (false == Auto && 0 == pPlayer->m_CurHp)
		return;

	CPlayer *pMember = 0;
	CPlayer *pLeader = 0;

	CPlayer *pQuest = (CPlayer*)GetPlayerByGID(pMsg->dwGID)->DynamicCast(IID_PLAYER);
	if (!pQuest)
		return;

	CPlayer::TeamQuestList::iterator QuestIt = pQuest->m_TeamSentOut.find(pPlayer->GetGID());
	CPlayer::TeamQuestList::iterator ReplyIt = pPlayer->m_TeamGetIn.find(pQuest->GetGID());

	if ((QuestIt == pQuest->m_TeamSentOut.end()) || (ReplyIt == pPlayer->m_TeamGetIn.end()))
		MY_ASSERT((QuestIt == pQuest->m_TeamSentOut.end()) && (ReplyIt == pPlayer->m_TeamGetIn.end()));
	if ((QuestIt != pQuest->m_TeamSentOut.end()) || (ReplyIt != pPlayer->m_TeamGetIn.end()))
		MY_ASSERT((QuestIt != pQuest->m_TeamSentOut.end()) && (ReplyIt != pPlayer->m_TeamGetIn.end()));

	if (QuestIt == pQuest->m_TeamSentOut.end() || ReplyIt == pPlayer->m_TeamGetIn.end())
		return;

	// �Ƴ�����
	pQuest->m_TeamSentOut.erase(QuestIt);
	pPlayer->m_TeamGetIn.erase(ReplyIt);

	if (SQTeamJoinMsgG::TJM_JOINME == pMsg->type)	// ������������ߵĶ���
	{
		pMember = pPlayer;
		pLeader = pQuest;
	}
	else
	{
		pMember = pQuest;
		pLeader = pPlayer;
	}

	if (SQTeamJoinMsgG::QTJ_REJECT == pMsg->bResult)
	{
		char buffer[128] = {0};
		_snprintf(buffer, 128, "��%s�� �ܾ���������룡", pPlayer->GetName());
		buffer[127] = 0;
		if (pMsg->type == SQTeamJoinMsgG::TJM_JOINME)
			TalkToDnid(pLeader->m_ClientIndex, buffer);
		else
			TalkToDnid(pMember->m_ClientIndex, buffer);
		return;
	}
	
	if (SQTeamJoinMsgG::TJM_JOINME == pMsg->type && !CanInviteOther(pLeader, pMember))
		return;
	
	if (SQTeamJoinMsgG::TJM_JOINOTHER == pMsg->type && !CanJoinOther(pLeader, pMember))
		return;

	// ����������Ƿ�����˶���
	if (0 == pLeader->m_dwTeamID && 0 == pMember->m_dwTeamID)
	{
		DWORD teamId = CreateTeam();

		if (teamId != -1)
        {
			pLeader->m_bIsTeamLeader = true;
			AddMember(pLeader, teamId, true);
			AddMember(pMember, teamId);
        }
        return;
	}
	else if (0 != pLeader->m_dwTeamID)
	{
		AddMember(pMember, pLeader->m_dwTeamID);
	}
	else if (0 != pMember->m_dwTeamID)
	{
		AddMember(pMember, pLeader->m_dwTeamID);
	}
}

// ����һ����ҵ�������ȥ
static void AddMember(CPlayer *pPlayer, const DWORD dwTeamId, bool IsLeader)
{
	if (0 == dwTeamId || !pPlayer)
		return;

	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(dwTeamId);
	if (iter == teamManagerMap.end())
		return;

	Team::TeamPlayerData player;
	ZeroMemory(&player, sizeof(player));

	// �������������Ϣ
	dwt::strcpy(player.szName, pPlayer->GetName(), CONST_USERNAME);
	player.bySex			= pPlayer->m_Property.m_Sex;
	player.bySchool			= pPlayer->m_Property.m_School;
	player.wMaxHp			= pPlayer->m_MaxHp;
	player.wCurHp			= pPlayer->m_CurHp;
	player.wMaxMp			= pPlayer->m_MaxMp;
	player.wCurMp			= pPlayer->m_CurMp;
	player.dwGlobal			= pPlayer->GetGID();
    player.byLevel			= pPlayer->m_Property.m_Level;
	player.isLeader			= IsLeader;
	player.bFaceID			= pPlayer->m_Property.m_FaceID;
	player.bHairID			= pPlayer->m_Property.m_HairID;
	player.wRegionID		= pPlayer->m_Property.m_CurRegionID;
	player.status			= 1;
	player.PlayerRef		= pPlayer;
	player.dnidPlayer		= pPlayer->m_ClientIndex;
	player.staticId			= pPlayer->m_Property.m_dwStaticID;
	
	for (int i = 0; i < sizeof(ShowEquipPos) / sizeof(int); i++)
		player.m_3DEquipID[i] = pPlayer->m_Property.m_Equip[ShowEquipPos[i]].wIndex;

	Team &team  = iter->second;
	DWORD index = team.byMemberNum++ ;
	team.stTeamPlayer[index] = player;
	team.bySend |= (1 << index);

	// �����������Ķ��鴴������Ҫ����ڵ�ǰ���ֱ��ˢ��
	if (index >= 2)
    {
		SATeamRefreshMsgG msg;
		msg.memberMark = (1 << (index + 1)) - 1;

        Team::TeamPlayer *iter = msg.stMember;
		for (int i = 0; i<MAX_TEAM_MEMBER; ++i)
        {
			if (msg.memberMark & (1 << i))
            {
                *iter = team.stTeamPlayer[i];
                iter++;
            }
        }

		g_StoreMessage(pPlayer->m_ClientIndex, &msg, msg.GetSize());
    }

	// һ��OK
	pPlayer->m_dwTeamID			= dwTeamId;
	pPlayer->m_bIsTeamLeader	= IsLeader;
	if (IsLeader)
		team.LeaderGID = player.dwGlobal;

	// �����䷽ʽͬ��һ��
	SQAChangeGiveMode giveMsg;
	giveMsg.bOperation	= SQAChangeGiveMode::SCGM_MODE;
	giveMsg.bMode		= team.byGiveMode;
	g_StoreMessage(pPlayer->m_ClientIndex, &giveMsg, sizeof(giveMsg));

	giveMsg.bOperation	= SQAChangeGiveMode::SCGM_COLOER;
	giveMsg.bMode		= team.byGiveColor;
	g_StoreMessage(pPlayer->m_ClientIndex, &giveMsg, sizeof(giveMsg));
	
	// ����ʱ����ͬ��һ��
	if (team.PackCount)
	{
		SAAddTempItemMsg tempMsg;
		tempMsg.count = 0;

		for (size_t i = 0; i < MAX_TEMPPACK_SIZE; i++)
		{
			if (team.LeaderPack[i].wIndex)
			{
				tempMsg.tempItems[tempMsg.count].item = team.LeaderPack[i];
				tempMsg.tempItems[tempMsg.count++].pos	= i;
			}
		}

		g_StoreMessage(pPlayer->m_ClientIndex, &tempMsg, tempMsg.MySize());	
	}

	if (pPlayer->m_SendRequestID)
	{
		CMakeTeamQuestService::GetInstance().RemoveQuest(pPlayer->m_SendRequestID);
		pPlayer->m_SendRequestID = 0;
	}

	if (5 == team.byMemberNum)									// ��Ա
		CMakeTeamQuestService::GetInstance().RemoveUnJoinedTeam(dwTeamId);
	else if (1 != team.byMemberNum && 2 != team.byMemberNum)	// ���£�1,2��������½�����CreateTeam���Ѿ�������ˣ�
		CMakeTeamQuestService::GetInstance().UpdateEmptyTeam(dwTeamId);

	//ͬ��BUFF����Ա
	//OnSynBuffState(pPlayer,dwTeamId);
}

BOOL AddItemToTeamPack(CPlayer *pPlayer, CItem *pItem, WORD index)
{
	if (!pPlayer || 0 == pPlayer->m_dwTeamID || !pItem)
		return FALSE;

	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pPlayer->m_dwTeamID);
	if (iter == teamManagerMap.end())
		return FALSE;

	Team &team = iter->second;

	//MY_ASSERT(team.byMemberNum>=2 && team.byMemberNum<=5);
	//MY_ASSERT(index < pItem->m_ItemsInPack);

	if (MAX_TEMPPACK_SIZE == team.PackCount)	// ��ʱ��������
		return FALSE;

	// �ҵ�һ�����е�λ��
	size_t i;
	for (i = 0; i < MAX_TEMPPACK_SIZE; i++)
	{
		if (0 == team.LeaderPack[i].wIndex)
			break;
	}

	MY_ASSERT(i <= MAX_TEMPPACK_SIZE);

	team.LeaderPack[i] = pItem->m_GroundItem[index];
	team.PackCount++;

	--pItem->m_ItemsInPack;

	// ��������
	//MY_ASSERT(0xff != pItem->m_ExPosition[index] && pItem->m_ExPosition[index] < g_PackageMaxItem);

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

	if (--pItem->m_Remain)
		pItem->UpdateItemPicked(index);
	else
		pItem->RemoveMe();

	// ͬ�������ж�Ա
	SAAddTempItemMsg synMsg;
	synMsg.count = 1;
	synMsg.tempItems[0].item = team.LeaderPack[i];
	synMsg.tempItems[0].pos	 = i;
	SendAllMember(&team, &synMsg, synMsg.MySize());

	return TRUE;
}

BOOL AddItemToTeamPack( CPlayer *pPlayer, CSingleItem *pItem )
{
	if (!pPlayer || 0 == pPlayer->m_dwTeamID || !pItem)
		return FALSE;

	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pPlayer->m_dwTeamID);
	if (iter == teamManagerMap.end())
		return FALSE;

	Team &team = iter->second;

	//MY_ASSERT(team.byMemberNum>=2 && team.byMemberNum<=5);
	//MY_ASSERT(index < pItem->m_ItemsInPack);

	if (MAX_TEMPPACK_SIZE == team.PackCount)	// ��ʱ��������
		return FALSE;

	// �ҵ�һ�����е�λ��
	size_t i;
	for (i = 0; i < MAX_TEMPPACK_SIZE; i++)
	{
		if (0 == team.LeaderPack[i].wIndex)
			break;
	}

	MY_ASSERT(i <= MAX_TEMPPACK_SIZE);

	team.LeaderPack[i] = pItem->m_GroundItem;
	team.PackCount++;

	pItem->m_IndexMask = -1;

	memset(&pItem->m_GroundItem, 0, sizeof(SRawItemBuffer));
	pItem->m_PickStatus = PIPT_LEADER + 1;
	pItem->m_ExPosition= 0xff;

	pItem->RemoveMe();

	// ͬ�������ж�Ա
	SAAddTempItemMsg synMsg;
	synMsg.count = 1;
	synMsg.tempItems[0].item = team.LeaderPack[i];
	synMsg.tempItems[0].pos	 = i;
	SendAllMember(&team, &synMsg, synMsg.MySize());

	return TRUE;
}

// ��һ����ҴӶ�����ɾȥ
void DelMember(const char *szName, CPlayer *pPlayer, bool IsKick)
{
	const DWORD dwTeamId = pPlayer->m_dwTeamID;
	if (!pPlayer || !szName || 0 == szName[0] || 0 == dwTeamId)
		return;
	
	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(dwTeamId);
	if (iter == teamManagerMap.end())
		return;

	Team &team = iter->second;
	
	if (team.byMemberNum < 2 || team.byMemberNum > 5)
	{
		rfalse(4,1,"team.byMemberNum < 2 || team.byMemberNum > 5");
		return;
	}
	MY_ASSERT(team.byMemberNum>=2 && team.byMemberNum<=5);

	int nNum = team.FindNum(szName);
	if (-1 == nNum)
		return;

	// ���ֻ��2���ˣ���ֱ�ӽ�ɢ
	if (team.byMemberNum <= 2)
	{
		DelTeam(dwTeamId); 
		return;
	}

	CPlayer *willDelPlayer = team.stTeamPlayer[nNum].PlayerRef;
	if (!willDelPlayer)
	{
		return;
	}
	// �ӳ���ӣ���Ҫ�������ζӳ�
	if (!IsKick && team.stTeamPlayer[nNum].isLeader)
	{
		MY_ASSERT(team.stTeamPlayer[nNum].dwGlobal == team.LeaderGID);

		// ���õ�һ����Ϊ�ӳ�
		int newLeaderPos = (nNum == 0) ? (nNum+1) : 0;
		CPlayer *nextLeader = team.stTeamPlayer[newLeaderPos].PlayerRef;
		if (nextLeader)
		{
			team.stTeamPlayer[newLeaderPos].isLeader = true;
			team.LeaderGID = team.stTeamPlayer[newLeaderPos].dwGlobal;
			nextLeader->m_bIsTeamLeader = true;
		}
	}

	for (int i=team.byMemberNum-1; i>= nNum; i--)
		team.bySend |= (1 << i);

	--team.byMemberNum;

	DWORD tempGID = team.stTeamPlayer[nNum].dwGlobal;

	if ((nNum != team.byMemberNum) && (nNum < 5) && (team.byMemberNum > nNum))
		memmove(&team.stTeamPlayer[nNum], &team.stTeamPlayer[nNum + 1], (sizeof(Team::TeamPlayerData) * (team.byMemberNum - nNum)));

	memset(&team.stTeamPlayer[team.byMemberNum], 0, sizeof(Team::TeamPlayerData));
	team.stTeamPlayer[team.byMemberNum].dwGlobal = tempGID;

	// ֪ͨ��Ҽ�����
	SATeamKickMsgG kickMsg;
	dwt::strcpy(kickMsg.szName, szName, CONST_USERNAME);
	kickMsg.operation = IsKick ? SATeamKickMsgG::TKM_KICKOFF : SATeamKickMsgG::TKM_QUIT;
	g_StoreMessage(willDelPlayer->m_ClientIndex, &kickMsg, sizeof(kickMsg));
	SendAllMember(&team, &kickMsg, sizeof(SATeamKickMsgG));
	willDelPlayer->m_dwTeamID = 0;
	willDelPlayer->m_bIsTeamLeader = false;
	
	if (4 == team.byMemberNum)		// ˵���Ǵ���Ա�����п�λ��
		CMakeTeamQuestService::GetInstance().InsertEmptyTeam(dwTeamId);
	else
		CMakeTeamQuestService::GetInstance().UpdateEmptyTeam(dwTeamId);
}

static void DelTeam(const DWORD dwTeamId)
{
	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(dwTeamId);
	if (iter == teamManagerMap.end())
		return;

	Team &stTeamInfo = iter->second;

	// ֪ͨ��Ա
	SATeamKickMsgG kickMsg;
	kickMsg.operation = SATeamKickMsgG::TKM_DISMISSED;

	for (size_t i = 0; i < stTeamInfo.byMemberNum; i++)
	{
		CPlayer *temp = stTeamInfo.stTeamPlayer[i].PlayerRef;
		if (temp)
		{
			dwt::strcpy(kickMsg.szName, temp->GetName(), CONST_USERNAME);
			g_StoreMessage(temp->m_ClientIndex, &kickMsg, sizeof(kickMsg));

			// �ؼ�����ʱ��Ч��
			temp->m_dwTeamID = 0;
			temp->m_bIsTeamLeader = false;
		}
	}

	teamManagerMap.erase(iter);

	CMakeTeamQuestService::GetInstance().RemoveUnJoinedTeam(dwTeamId);
}

// ����һ�����鲢����Id
static DWORD CreateTeam()
{
	static DWORD teamId = 0;
    teamId++;

    std::map<DWORD, Team>::iterator iter = teamManagerMap.find(teamId);
    if (iter != teamManagerMap.end())
        return -1;

	Team team;
	ZeroMemory(&team, sizeof(team));
	team.byGiveMode  = Team::TGM_FREE;
	team.byGiveColor = Team::TGC_BLUE;

	teamManagerMap[teamId] = team;

	CMakeTeamQuestService::GetInstance().InsertEmptyTeam(teamId);

    return teamId;
}

// �㲥��Ϣ����Ա
static void SendAllMember(Team *stManager, LPVOID pMsg, WORD wSize)
{
	if (!stManager)
	{
		rfalse(4,1,"void SendAllMemberΪNULL");
		return;
	}
	for (int n = 0; n < MAX_TEAM_MEMBER; ++n)
	{
		if (0 == stManager->stTeamPlayer[n].status)
			continue;

		g_StoreMessage(stManager->stTeamPlayer[n].dnidPlayer, pMsg, wSize);
	}
}

static BOOL CanJoinOther(CPlayer *pOther, CPlayer *pNewMember)
{
	if (!pOther || !pNewMember)
		return FALSE;

	// �Լ������ڶ�����
	if (0 != pNewMember->m_dwTeamID)
		return FALSE;

	// �ӳ�����û�ж���
	if (0 == pOther->m_dwTeamID)
		return FALSE;

	// �鿴�����Ƿ�����
	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pOther->m_dwTeamID);
	if (iter == teamManagerMap.end())
		return FALSE;

	if (!pOther->IsTeamLeader())
		return FALSE;

	Team &team = iter->second;

	if (5 == team.byMemberNum)
		return FALSE;

	if (-1 == team.FindNum(pNewMember->m_Property.m_dwStaticID))
	{
		return TRUE;
	}

	//MY_ASSERT(-1 == team.FindNum(pNewMember->m_Property.m_dwStaticID));

	return FALSE;
}

// ������
static BOOL CanInviteOther(CPlayer *pLeader, CPlayer *pMember)
{
	if (!pLeader || !pMember)
		return FALSE;
	
	if (pLeader->m_dwTeamID != 0)
	{
		std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pLeader->m_dwTeamID);
		if (iter == teamManagerMap.end())
			return FALSE;
	
		if (!pLeader->IsTeamLeader())			// ֻ�жӳ�����������˼���
			return FALSE;

		Team &team = iter->second;
		int hasPos = team.FindNum(pMember->m_Property.m_dwStaticID);
		if (-1 != hasPos)
			return FALSE;

		if (5 == team.byMemberNum)
			return FALSE;
	}
	
	// ����Ŀ������Ƿ��Ѿ����
	if (0 != pMember->m_dwTeamID)
		return FALSE;

	return TRUE;
}

void RefreshTeam(Team &team)
{
	SATeamRefreshMsgG msg;
	msg.memberMark = team.bySend;

	if (0 == msg.memberMark)
		return;

	Team::TeamPlayer *pMember = msg.stMember;
	for (int i=0; i<8; ++i)
	{
		if (msg.memberMark & (1 << i))
		{
			*pMember = team.stTeamPlayer[i];
			pMember++;
		}
	}
	
	SendAllMember(&team, &msg, msg.GetSize());
	
	team.bySend = 0;	
}

// �������״̬
void UpdatePlayer(Team &teamInfo, int nNum, CPlayer *pPlayer)
{
	if (!pPlayer)
	{
		rfalse(4,1,"UpdatePlayer ΪNull");
		return;
	}
	Team::TeamPlayerData member;
	memcpy(&member, &teamInfo.stTeamPlayer[nNum], sizeof(Team::TeamPlayerData));

	member.wMaxHp		= pPlayer->m_MaxHp;
	member.wCurHp		= pPlayer->m_CurHp;
	member.wMaxMp		= pPlayer->m_MaxMp;
	member.wCurMp		= pPlayer->m_CurMp;
	member.byLevel		= pPlayer->m_Property.m_Level;
	member.wRegionID	= pPlayer->m_Property.m_CurRegionID;
	member.bFaceID		= pPlayer->m_Property.m_FaceID;
	member.bHairID		= pPlayer->m_Property.m_HairID;

	for (int i = 0; i < sizeof(ShowEquipPos) / sizeof(int); i++)
		member.m_3DEquipID[i] = pPlayer->m_Property.m_Equip[ShowEquipPos[i]].wIndex;

	LPVOID checkSegment  = (LPBYTE)&teamInfo.stTeamPlayer[nNum] + sizeof(member.szName);
	LPVOID verifySegment = (LPBYTE)&member + sizeof(member.szName);
	
	BOOL isSame = (memcmp(checkSegment, verifySegment, sizeof(member) - sizeof(member.szName)) == 0);

	if ((!isSame) && nNum>=0 && nNum<5)
	{
		memcpy((LPBYTE)&teamInfo.stTeamPlayer[nNum]+sizeof(member.szName), (LPBYTE)&member + sizeof(member.szName), sizeof(member)-sizeof(member.szName));

		teamInfo.bySend |= (1 << nNum);

		if (MAX_TEAM_MEMBER != teamInfo.byMemberNum)
			CMakeTeamQuestService::GetInstance().UpdateEmptyTeam(pPlayer->m_dwTeamID);
	}

	// ���Ͷ��ѵ��������
	SATeamPosMsg posMsg;
	posMsg.m_Num = 0;

	for (size_t i=0; i<teamInfo.byMemberNum; i++)
	{
		if (pPlayer->GetGID() == teamInfo.stTeamPlayer[i].dwGlobal)
			continue;

		if (pPlayer->m_Property.m_CurRegionID != teamInfo.stTeamPlayer[i].wRegionID)
			continue;

		CPlayer *pMember = teamInfo.stTeamPlayer[i].PlayerRef;
		if (pMember)
		{
			posMsg.m_Pos[posMsg.m_Num].dwGID	= teamInfo.stTeamPlayer[i].dwGlobal;
			posMsg.m_Pos[posMsg.m_Num].fX		= pMember->m_curX;	
			posMsg.m_Pos[posMsg.m_Num].fY		= pMember->m_curY;
			++posMsg.m_Num;
		}
	}

	if (posMsg.m_Num)
		g_StoreMessage(pPlayer->m_ClientIndex, &posMsg, sizeof(SATeamPosMsg) - ((5-posMsg.m_Num)*sizeof(TeamMemberPos)));
}

// �ڵ���/�˳���ʱ�򣬴��������Ӧ��
void TeamManager::TeamLoseStatus(CPlayer *pPlayer, BYTE bResult)
{
	if (!pPlayer)
		return;

	// ������Ӱ��
	if (bResult == SATeamCancelQuestMsg::TCQ_DEAD)
		return;

	// �Ƿ��ڵȴ���Ӧ
	if (pPlayer->m_TeamSentOut.size())
	{
		for (CPlayer::TeamQuestList::iterator it = pPlayer->m_TeamSentOut.begin(); it != pPlayer->m_TeamSentOut.end(); ++it)
		{
			CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(it->first)->DynamicCast(IID_PLAYER);
			if (!pDestPlayer)
				continue;

			//MY_ASSERT(pDestPlayer->m_CurHp);
			pDestPlayer->m_TeamGetIn.erase(pPlayer->GetGID());		// �ڶԷ����б����Ƴ���
		}

		pPlayer->m_TeamSentOut.clear();
	}

	// �Ƿ���Ҫ��Ӧ
	if (pPlayer->m_TeamGetIn.size())
	{
		CPlayer::TeamQuestList tempCopy(pPlayer->m_TeamGetIn);

		for (CPlayer::TeamQuestList::iterator it = tempCopy.begin(); it != tempCopy.end(); ++it)
		{
			CPlayer *pQuest = (CPlayer *)GetPlayerByGID(it->first)->DynamicCast(IID_PLAYER);
			if (pQuest)
			{
				// ֱ�Ӿܾ�
				SQTeamJoinMsgG rejectMsg;
				rejectMsg.bResult	= SQTeamJoinMsgG::QTJ_REJECT;
				rejectMsg.type		= it->second.second;
				rejectMsg.dwGID		= it->first;

				OnRecvJoinMsg(pPlayer->m_ClientIndex, &rejectMsg, pPlayer, true);
			}
		}

		//MY_ASSERT(pPlayer->m_TeamGetIn.empty());
	}

	if (pPlayer->m_SendRequestID)
	{
		CMakeTeamQuestService::GetInstance().RemoveQuest(pPlayer->m_SendRequestID);
		pPlayer->m_SendRequestID = 0;
	}

	pPlayer->m_LastSynTeamRequestIDList.clear();
	pPlayer->m_LastSynFindTeamIDList.clear();

	// ������˳������뿪����
	if (SATeamCancelQuestMsg::TCQ_LOGOUT == bResult)
		DelMember(pPlayer->GetName(), pPlayer, false);

	return;
}

void KickAllTeamPlayer(Team stTeamInfo)
{
	//Team team = iter->second;
	for (int i = 0; i < stTeamInfo.byMemberNum; ++i)
	{
		CPlayer *pPlayer = stTeamInfo.stTeamPlayer[i].PlayerRef;
		if (pPlayer)
		{
			if (pPlayer->m_ParentRegion )
			{
				if (pPlayer->m_ParentRegion->DynamicCast(IID_DYNAMICREGION) && pPlayer->m_DynamicRegionState == 1)
				{
					LuaFunctor(g_Script,"PartKickerToReturn")[pPlayer->m_ParentRegion->GetGID()][pPlayer->GetGID()]();
				}
			}
		}
	}
}

void KickToTeamPlayer(Team stTeamInfo,const char* szName)
{
	int nNum = stTeamInfo.FindNum(szName);
	if (-1 == nNum)
		return;

	CPlayer *pPlayer = stTeamInfo.stTeamPlayer[nNum].PlayerRef;
	if (pPlayer)
	{
		if (pPlayer->m_ParentRegion )
		{
			if (pPlayer->m_ParentRegion->DynamicCast(IID_DYNAMICREGION) && pPlayer->m_DynamicRegionState == 1)
			{
				LuaFunctor(g_Script,"PartKickerToReturn")[pPlayer->m_ParentRegion->GetGID()][pPlayer->GetGID()]();
			}
		}
	}
}
