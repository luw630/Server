#include "StdAfx.h"
#include "ArenaMap.h"


CArenaMap::~CArenaMap()
{
}

BOOL CArenaMap::ProcessDyArenaMsg(DNID dnidClient, struct SArenaMsg *pSArenaMsg, CPlayer *pPlayer)
{
	if (!pSArenaMsg || !pPlayer)
	{
		return FALSE;
	}
	switch (pSArenaMsg->_protocol)
	{
		case SArenaMsg::EPRO_DYARENA_INIT:
		{
			SQSArenaInit* psarenamsg = static_cast<SQSArenaInit*>(pSArenaMsg);
			if (psarenamsg)
			{
				return OnRecvArenaInitMsg(dnidClient, psarenamsg, pPlayer);
			}
		}
		break;
		case SArenaMsg::EPRO_DYARENA_REWARD:
		{
			SQSArenaRewardUI* psarenamsg = static_cast<SQSArenaRewardUI*>(pSArenaMsg);
			if (psarenamsg)
			{
				return OnRecvArenaRewardUIMsg(dnidClient, psarenamsg, pPlayer);
			}
		}
		break;
		case SArenaMsg::EPRO_DYARENA_QUESTREWARD:
		{
			SQArenaQuestReward* psarenamsg = static_cast<SQArenaQuestReward*>(pSArenaMsg);
			if (psarenamsg)
			{
				return OnRecvArenaQuestReward(dnidClient, psarenamsg, pPlayer);
			}
		}
		break;
		case SArenaMsg::EPRO_DYARENA_START:
		{
			SQArenaStart* psarenamsg = static_cast<SQArenaStart*>(pSArenaMsg);
			if (psarenamsg)
			{
				return OnRecvArenaStartMsg(dnidClient, psarenamsg, pPlayer);
			}
		}
		break;
	default:
		break;
	}
	return TRUE;
}

BOOL CArenaMap::OnRecvArenaInitMsg(DNID dnidClient, struct SQSArenaInit *pSArenaMsg, CPlayer *pPlayer)
{
	if (!pSArenaMsg || !pPlayer)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnRecvArenaInitMsg"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
	return TRUE;
}

BOOL CArenaMap::OnRecvArenaRewardUIMsg(DNID dnidClient, struct SQSArenaRewardUI *pSArenaMsg, CPlayer *pPlayer)
{
	if (!pSArenaMsg || !pPlayer)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnRecvArenaRewardUIMsg"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
	return TRUE;
}

bool CArenaMap::SendArenaMsg(CPlayer *pPlayer, SArenaMsg *pSArenaMsg, WORD wmsglenth)
{
	if (!pPlayer || !pSArenaMsg)
	{
		return false;
	}
	return (bool)g_StoreMessage(pPlayer->m_ClientIndex, pSArenaMsg, wmsglenth);
}

BOOL CArenaMap::OnRecvArenaStartMsg(DNID dnidClient, struct SQArenaStart *pSArenaMsg, CPlayer *pPlayer)
{
	if (!pSArenaMsg || !pPlayer)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnPutPlayerInArena"))
	{
		g_Script.PushParameter(pSArenaMsg->bIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
	return TRUE;
}

BOOL CArenaMap::OnRecvArenaQuestReward(DNID dnidClient, struct SQArenaQuestReward *pSArenaMsg, CPlayer *pPlayer)
{
	if (!pSArenaMsg || !pPlayer)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnQuestArenaReward"))
	{
		g_Script.PushParameter(pSArenaMsg->bRewardtype);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
	return TRUE;
}
