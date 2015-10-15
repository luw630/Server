#include "StdAfx.h"
#include "SceneMap.h"

CSceneMap::~CSceneMap(void)
{
}

BOOL CSceneMap::ProcessSceneMsg( DNID dnidClient, struct SSceneMsg *pSceneMsg, CPlayer *pPlayer )
{
	if (!pSceneMsg || !pPlayer)
	{
		return FALSE;
	}
	switch (pSceneMsg->_protocol) 
	{
	case SSceneMsg::EPRO_SCENE_INIT:
		{
			SQSceneInit* psceneinitmsg =static_cast<SQSceneInit*>(pSceneMsg);
			if (psceneinitmsg)
			{
				return OnRecvGetSceneMsg(dnidClient,psceneinitmsg,pPlayer);
			}
		}
		break;
	case SSceneMsg::EPRO_SCENE_SELECT:
		{
			SQSceneSelect* psceneselectmsg = static_cast<SQSceneSelect*>(pSceneMsg);
			if (psceneselectmsg)
			{
				return OnRecvSelectSceneMsg(dnidClient,psceneselectmsg,pPlayer);
			}
		}
		break;
	case SSceneMsg::EPRO_SCENE_ENTER:
		{
			SQSceneEnter* psceneentermsg = static_cast<SQSceneEnter*>(pSceneMsg);
			if (psceneentermsg)
			{
				return OnRecvEnterSceneMsg(dnidClient,psceneentermsg,pPlayer);
			}
		}
		break;
	case SSceneMsg::EPRO_SCENE_PICKCARDS:
		{
			SQPickCards *pwinrewardmsg =  static_cast<SQPickCards*>(pSceneMsg);
			if (pwinrewardmsg)
			{
				return OnRecvGetRewardMsg(dnidClient,pwinrewardmsg,pPlayer);
			}
		}
		break;
	case SSceneMsg::EPRO_SCENE_SDINFO:
		{
			SQStopSD *pstopmsg =  static_cast<SQStopSD*>(pSceneMsg);
			if (pstopmsg)
			{
				return OnRecvStopSDMsg(dnidClient,pstopmsg,pPlayer);
			}
		}
		break;
	case SSceneMsg::EPRO_SCENE_SDREWARD:
		{
			SQSDReward *pstopmsg =  static_cast<SQSDReward*>(pSceneMsg);
			if (pstopmsg)
			{
				return OnRecvSDRewardMsg(dnidClient,pstopmsg,pPlayer);
			}
		}
		break;
	case SSceneMsg::EPRO_SCENE_LEAVE:
		{
			SQSceneLeave *pleavemsg =  static_cast<SQSceneLeave*>(pSceneMsg);
			if (pleavemsg)
			{
				return OnRecvSQSceneLeave(dnidClient,pleavemsg,pPlayer);
			}
		}
		break;
	case SSceneMsg::EPRO_SCENE_QUESTREWARD:
	{
		SQSceneQuestReward *prewardmsg = static_cast<SQSceneQuestReward*>(pSceneMsg);
		if (prewardmsg)
		{
			return OnRecvSceneQuestReward(dnidClient, prewardmsg, pPlayer);
		}
	}
	break;
	case SSceneMsg::EPRO_SCENE_DEBRISINFO:
	{
		SQDebrisinfo *pDebrisinfo = static_cast<SQDebrisinfo*>(pSceneMsg);
		if (pDebrisinfo)
		{
			return OnRecvQuestDebrisinfo(dnidClient, pDebrisinfo, pPlayer);
		}
	}
		break;
	case SSceneMsg::EPRO_SCENE_DEBRISAWARD:
	{
		SQDebrisAward *pDebrisaward = static_cast<SQDebrisAward*>(pSceneMsg);
		if (pDebrisaward)
		{
			return OnRecvQuestDebrisAward(dnidClient, pDebrisaward, pPlayer);
		}
	}
		break;
	case SSceneMsg::EPRO_SCENE_GROUPMONSTERDIE:
	{
		SQGroupMonsterDie *pGroupMonsterDie = static_cast<SQGroupMonsterDie*>(pSceneMsg);
		if (pGroupMonsterDie)
		{
			return OnRecvSendGroupMonsterDie(dnidClient, pGroupMonsterDie, pPlayer);
		}
	}
		break;
	default:
		break;
	}
	return FALSE;
}

BOOL CSceneMap::OnRecvGetSceneMsg( DNID dnidClient, struct SQSceneInit *pSceneMsg, CPlayer *pPlayer )
{
	if (!pSceneMsg || !pPlayer)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("onGetSceneMsg"))
	{
		g_Script.PushParameter(pSceneMsg->bSceneMapIndex);
		g_Script.Execute();
	}
	
	g_Script.CleanPlayer();
	return TRUE;
}

BOOL CSceneMap::OnRecvSelectSceneMsg( DNID dnidClient, struct SQSceneSelect *pSceneMsg, CPlayer *pPlayer )
{
	if (!pSceneMsg || !pPlayer)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	
	if (g_Script.PrepareFunction("onSelectSceneMap"))
	{
		g_Script.PushParameter(pSceneMsg->wSceneMap);
		g_Script.PushParameter(pSceneMsg->wsceneIndex);
		g_Script.Execute();
	}

	g_Script.CleanPlayer();
	return TRUE;
}

BOOL CSceneMap::OnRecvEnterSceneMsg( DNID dnidClient, struct SQSceneEnter *pSceneMsg, CPlayer *pPlayer )
{
	if (!pSceneMsg || !pPlayer)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	
	if (g_Script.PrepareFunction("onRecvEnterSceneMap"))
	{
		g_Script.PushParameter(pSceneMsg->wSceneMap);
		g_Script.PushParameter(pSceneMsg->wsceneIndex);
		g_Script.PushParameter(pSceneMsg->bentertype);
		g_Script.PushParameter(pSceneMsg->bcount);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
	return TRUE;
}

bool CSceneMap::SendSceneMsg( CPlayer *pPlayer,SASceneInit* pSceneMsg,WORD wmsglenth )
{
	if (!pPlayer || !pSceneMsg || wmsglenth == 0)
	{
		return false;
	}
	return (bool)g_StoreMessage(pPlayer->m_ClientIndex, pSceneMsg, wmsglenth);
}

bool CSceneMap::SendSelectSceneMsg( CPlayer *pPlayer,SASceneSelect* pSceneMsg,WORD wmsglenth )
{
	if (!pPlayer || !pSceneMsg || wmsglenth == 0)
	{
		return false;
	}
	return (bool)g_StoreMessage(pPlayer->m_ClientIndex, pSceneMsg, wmsglenth);
}

bool CSceneMap::SendEnterSceneMsg( CPlayer *pPlayer,SASceneEnter* pSceneMsg )
{
	if (!pPlayer || !pSceneMsg)
	{
		return false;
	}
	return (bool)g_StoreMessage(pPlayer->m_ClientIndex, pSceneMsg, sizeof(SASceneEnter));
}

bool CSceneMap::SendRewardMsg( CPlayer *pPlayer,SAWinrewardInfo* pSceneMsg,WORD wmsglenth )
{
	if (!pPlayer || !pSceneMsg)
	{
		return false;
	}
	return (bool)g_StoreMessage(pPlayer->m_ClientIndex, pSceneMsg, wmsglenth);
}

BOOL CSceneMap::OnRecvGetRewardMsg( DNID dnidClient, struct SQPickCards *pSceneMsg, CPlayer *pPlayer )
{
	if (!pSceneMsg || !pPlayer)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("onRecvGetRewardMsg"))
	{
		g_Script.PushParameter(pSceneMsg->bextracttype);
		g_Script.PushParameter(pSceneMsg->bextractindex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
	return TRUE;
}

bool CSceneMap::SendWinRewardMsg( CPlayer *pPlayer,SAPickCards* pSceneMsg,WORD wmsglenth )
{
	if (!pPlayer || !pSceneMsg)
	{
		return false;
	}
	return (bool)g_StoreMessage(pPlayer->m_ClientIndex, pSceneMsg, wmsglenth);
}

bool CSceneMap::SendPassFailMsg( CPlayer *pPlayer,SAPassFail* pSceneMsg,WORD wmsglenth )
{
	if (!pPlayer || !pSceneMsg)
	{
		return false;
	}
	return (bool)g_StoreMessage(pPlayer->m_ClientIndex, pSceneMsg, wmsglenth);
}

BOOL CSceneMap::OnRecvStopSDMsg( DNID dnidClient, struct SQStopSD *pSceneMsg, CPlayer *pPlayer )
{
	if (!pPlayer || !pSceneMsg)
	{
		return false;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnRecvStopSDMsg"))
	{
		g_Script.PushParameter(pSceneMsg->bStopType);
		g_Script.Execute();
	}
	
	g_Script.CleanPlayer();
	return TRUE;
}

BOOL CSceneMap::OnRecvSDRewardMsg( DNID dnidClient, struct SQSDReward *pSceneMsg, CPlayer *pPlayer )
{
	if (!pPlayer || !pSceneMsg)
	{
		return false;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnRecvSDRewardMsg"))
	{
		g_Script.PushParameter(pSceneMsg->bisdouble);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
	return TRUE;
}

bool CSceneMap::SendSDMsg( CPlayer *pPlayer,SAStopSD* pSceneMsg,WORD wmsglenth )
{
	if (!pPlayer || !pSceneMsg)
	{
		return false;
	}
	return (bool)g_StoreMessage(pPlayer->m_ClientIndex, pSceneMsg, wmsglenth);
}

bool CSceneMap::SendSDReward( CPlayer *pPlayer,SASDReward* pSceneMsg,WORD wmsglenth )
{
	if (!pPlayer || !pSceneMsg)
	{
		return false;
	}
	return (bool)g_StoreMessage(pPlayer->m_ClientIndex, pSceneMsg, wmsglenth);
}

BOOL CSceneMap::OnRecvSQSceneLeave( DNID dnidClient, struct SQSceneLeave *pSceneMsg, CPlayer *pPlayer )
{
	if (!pPlayer || !pSceneMsg)
	{
		return false;
	}
	g_Script.SetPlayer(pPlayer);
	
	if (g_Script.PrepareFunction("OnRecvSQSceneLeave"))
	{
		g_Script.PushParameter(pPlayer->GetGID());
		g_Script.Execute();
	}

	g_Script.CleanPlayer();
	return TRUE;
}

bool CSceneMap::SendSceneListMsg( CPlayer *pPlayer,SASceneList* pSceneMsg,WORD wmsglenth )
{
	if (!pPlayer || !pSceneMsg)
	{
		return false;
	}
	return (bool)g_StoreMessage(pPlayer->m_ClientIndex, pSceneMsg, wmsglenth);
}

bool CSceneMap::SendSceneFinishMsg(CPlayer *pPlayer, SASceneFinish* pSceneMsg, WORD wmsglenth)
{
	if (!pPlayer || !pSceneMsg)
	{
		return false;
	}
	return (bool)g_StoreMessage(pPlayer->m_ClientIndex, pSceneMsg, wmsglenth);
}

BOOL CSceneMap::OnRecvSceneQuestReward(DNID dnidClient, struct SQSceneQuestReward *pSceneMsg, CPlayer *pPlayer)
{
	if (!pSceneMsg || !pPlayer)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnSceneQuestReward"))
	{
		g_Script.PushParameter(pPlayer->GetGID());
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
	return TRUE;
}

BOOL CSceneMap::OnRecvQuestDebrisinfo(DNID dnidClient, struct SQDebrisinfo *pSceneMsg, CPlayer *pPlayer)
{
	if (!pSceneMsg || !pPlayer)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnRecvQuestDebrisinfo"))
	{
		g_Script.Execute();
	}
	
	
	g_Script.CleanPlayer();
	return TRUE;
}

BOOL CSceneMap::OnRecvQuestDebrisAward(DNID dnidClient, struct SQDebrisAward *pSceneMsg, CPlayer *pPlayer)
{
	if (!pSceneMsg || !pPlayer)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnRecvQuestDebrisAward"))
	{
		g_Script.Execute();
	}
	
	g_Script.CleanPlayer();
	return TRUE;
}

bool CSceneMap::SendDebrisinfo(CPlayer *pPlayer, SADebrisinfo* pSceneMsg, WORD wmsglenth)
{
	if (!pPlayer || !pSceneMsg)
	{
		return false;
	}
	return (bool)g_StoreMessage(pPlayer->m_ClientIndex, pSceneMsg, wmsglenth);
}

bool CSceneMap::SendDebrisAward(CPlayer *pPlayer, SADebrisAward* pSceneMsg, WORD wmsglenth)
{
	if (!pPlayer || !pSceneMsg)
	{
		return false;
	}
	return (bool)g_StoreMessage(pPlayer->m_ClientIndex, pSceneMsg, wmsglenth);
}

bool CSceneMap::SendBatchesMonster(CPlayer *pPlayer, SABatchesMonster* pSceneMsg, WORD wmsglenth)
{
	if (!pPlayer || !pSceneMsg)
	{
		return false;
	}
	return (bool)g_StoreMessage(pPlayer->m_ClientIndex, pSceneMsg, wmsglenth);
}

bool CSceneMap::SendEPSceneMsg(CPlayer *pPlayer, SSceneMsg* pSceneMsg, WORD wmsglenth)
{
	if (!pPlayer || !pSceneMsg)
	{
		return false;
	}
	return (bool)g_StoreMessage(pPlayer->m_ClientIndex, pSceneMsg, wmsglenth);
}

BOOL CSceneMap::OnRecvSendGroupMonsterDie(DNID dnidClient, struct SQGroupMonsterDie *pSceneMsg, CPlayer *pPlayer)
{
	if (!pSceneMsg || !pPlayer)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnRecvSendGroupMonsterDie"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
	return TRUE;
}
