#include "StdAfx.h"
#include "Transformers.h"
#include "Player.h"
#include "ScriptManager.h"
#include "DynamicRegion.h"

CTransformers::CTransformers()
{
}


CTransformers::~CTransformers()
{
}

void CTransformers::OnTransformersInit(SQTransformersInit *pTransformersMsg)
{
	if (!pTransformersMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		g_Script.SetPlayer(pPlayer);
		if (g_Script.PrepareFunction("OnTransformersInit"))
		{
			g_Script.PushParameter(pTransformersMsg->bTransformerIndex);
			g_Script.Execute();
		}
		g_Script.CleanPlayer();
	}
}

void CTransformers::OnTransformersInfo(SQTransformersInfo *pTransformersMsg)
{
	if (!pTransformersMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		g_Script.SetPlayer(pPlayer);
		if (g_Script.PrepareFunction("OnTransformersInfo"))
		{
			g_Script.PushParameter(pTransformersMsg->bTransformerIndex);
			g_Script.Execute();
		}
		g_Script.CleanPlayer();
	}
}

void CTransformers::OnTransformersSwitch(SQTransformersSwitch *pTransformersMsg)
{
	if (!pTransformersMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	//if (pPlayer)
	//{
	//	g_Script.SetPlayer(pPlayer);
	//	if (g_Script.PrepareFunction("OnTransformersSwitch"))
	//	{
	//		g_Script.PushParameter(pTransformersMsg->bTransformerIndex);
	//		g_Script.Execute();
	//	}
	//	g_Script.CleanPlayer();
	//}
	BOOL bRet = FALSE;
	BYTE State = GetTransformersLevel(0);
	DWORD TranIntervalTime = pPlayer->_L_GetLuaValue("ReturnPlayerTranLimit");
	DWORD DieTranIntervalTime = pPlayer->_L_GetLuaValue("ReturnPlayerDieTranLimit");
	time_t CurTime = time(NULL);
	DWORD TransteredTime = CurTime - pPlayer->m_RecordPreTransferTime;
	if (pTransformersMsg->bTransformerIndex == 1)
	{
		if (pPlayer->m_Property.m_NewPetData[pPlayer->m_Property.m_TransPetIndex].m_CurPetDurable <= 0)
		{
			TalkToDnid(pPlayer->m_ClientIndex, "变身宠物的魂力为0");
		}
		else if ((State != pTransformersMsg->bTransformerIndex || pPlayer->m_Property.m_FightPetActived != pPlayer->m_Property.m_TransPetIndex)
			&& pPlayer->m_Property.m_TransPetIndex != 0xff
			&& CurTime - pPlayer->m_RecordPreDeadCTime >= DieTranIntervalTime
			&& TransteredTime >= TranIntervalTime)
		{
			if (pPlayer->m_ParentRegion->DynamicCast(IID_DYNAMICREGION) != NULL)
				bRet = pPlayer->PlayerTransferPet(pPlayer->m_Property.m_TransPetIndex);
			else
				bRet = TRUE;
			pPlayer->m_Property.m_FightPetActived = pPlayer->m_Property.m_TransPetIndex;
			SetTransformersLevel(0, 1);
			pPlayer->SendMyState();
			//g_StoreMessage(pPlayer->m_ClientIndex, pPlayer->GetStateMsg(), sizeof(SASynPlayerMsg));
		}
		else if (pPlayer->m_SysAutoTransToPlayer)
			bRet = TRUE;
	}
	else
	{
		if ((State != pTransformersMsg->bTransformerIndex && pPlayer->m_Property.m_FightPetActived != 0xff
			&& TransteredTime >= TranIntervalTime && CurTime - pPlayer->m_RecordPreDeadCTime >= DieTranIntervalTime))
		{
			if (pPlayer->m_ParentRegion->DynamicCast(IID_DYNAMICREGION) != NULL)
				bRet = pPlayer->TransferPlayer();
			else
				bRet = TRUE;
			pPlayer->m_Property.m_FightPetActived = 0xff;
			SetTransformersLevel(0, 0);
			pPlayer->SendMyState();
		}
		else if (pPlayer->m_SysAutoTransToPlayer)
			bRet = TRUE;
	}
	SATransformersSwitch Res;
	if (bRet)
	{
		if (!pPlayer->m_SysAutoTransToPlayer)
			pPlayer->m_RecordPreTransferTime = time(NULL);
		else
			pPlayer->m_SysAutoTransToPlayer = FALSE;
		Res.bresult = 1;
	}
	else
		Res.bresult = 0;
	g_StoreMessage(pPlayer->m_ClientIndex, &Res, sizeof(SATransformersSwitch));
}

BOOL CTransformers::ProcessTransformersMsg(STransformersMsgs *pTransformersMsg)
{
	if (pTransformersMsg)
	{
		switch (pTransformersMsg->_protocol)
		{
			case STransformersMsgs::EPRO_TRANSFORMERS_INFO:
			{
				OnTransformersInfo((SQTransformersInfo*)pTransformersMsg);
			}
			break;
			case STransformersMsgs::EPRO_TRANSFORMERS_INIT:
			{
				OnTransformersInit((SQTransformersInit*)pTransformersMsg);
			}
			break;
			case STransformersMsgs::EPRO_TRANSFORMERS_SWITCH:
			{
				OnTransformersSwitch((SQTransformersSwitch*)pTransformersMsg);
			}
			break;
			case STransformersMsgs::EPRO_TRANSFORMERS_SKILLINFO:
			{
				OnTransformersSkillInfo((SQTransformersSkillInfo*)pTransformersMsg);
			}
				break;
		default:
			break;
		}
	}
	return TRUE;
}

BYTE CTransformers::GetTransformersLevel(BYTE bTransformerIndex)
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		if (bTransformerIndex == 1) //变身1状态
		{
			DWORD dTransformersstate = pPlayer->m_Property.m_ShowState;
			if (dTransformersstate > 0)
			{
				dTransformersstate >>= 0x8;
				BYTE level = dTransformersstate & 0xff;
				return level;
			}
		}
		else if (bTransformerIndex == 2) //变身2状态
		{
			DWORD dTransformersstate = pPlayer->m_Property.m_ShowState;
			if (dTransformersstate > 0)
			{
				dTransformersstate >>= 0x10;
				BYTE level = dTransformersstate & 0xff;
				return level;
			}
		}
		else if (bTransformerIndex == 0) //当前变身状态
		{
			DWORD dTransformersstate = pPlayer->m_Property.m_ShowState;
			if (dTransformersstate > 0)
			{
				BYTE level = dTransformersstate & 0xff;
				return level;
			}
		}
	}
	return 0;
}

BOOL CTransformers::SetTransformersLevel(BYTE bTransformerIndex, BYTE blevel)
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		if (bTransformerIndex == 1)
		{
			BYTE level2 = GetTransformersLevel(bTransformerIndex + 1);
			BYTE Transforstate = GetTransformersLevel(0);
			DWORD TransformersLevel = level2 << 0x10;
			TransformersLevel |= blevel << 0x8;
			TransformersLevel |= Transforstate;
			pPlayer->m_Property.m_ShowState = TransformersLevel;
			return TRUE;
		}
		else if (bTransformerIndex == 2)
		{
			BYTE level1 = GetTransformersLevel(bTransformerIndex - 1);
			BYTE Transforstate = GetTransformersLevel(0);
			DWORD TransformersLevel = level1 << 0x8;
			TransformersLevel |= blevel << 0x10;
			TransformersLevel |= Transforstate;
			pPlayer->m_Property.m_ShowState = TransformersLevel;
			return TRUE;
		}
		else if (bTransformerIndex == 0)
		{
			BYTE level1 = GetTransformersLevel(bTransformerIndex + 1);
			BYTE level2 = GetTransformersLevel(bTransformerIndex + 2);
			DWORD TransformersLevel = level1 << 0x8;
			TransformersLevel |= level2 << 0x10;
			TransformersLevel |= blevel;
			pPlayer->m_Property.m_ShowState = TransformersLevel;
			return TRUE;
		}
	}
	return FALSE;
}

void CTransformers::OnSendTransformersInfoMsg(SATransformersInfo* pTransformersMsg, WORD wmsglenth)
{
	if (!pTransformersMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;
	g_StoreMessage(pPlayer->m_ClientIndex, (LPVOID)pTransformersMsg, wmsglenth);
}

void CTransformers::SendTransformersSwitch(SATransformersSwitch *pTransformersMsg, WORD wmsglenth)
{
	if (!pTransformersMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;
	g_StoreMessage(pPlayer->m_ClientIndex, (LPVOID)pTransformersMsg, wmsglenth);
}

void CTransformers::OnTransformersSkillInfo(SQTransformersSkillInfo *pTransformersMsg)
{
	if (!pTransformersMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		g_Script.SetPlayer(pPlayer);
		if (g_Script.PrepareFunction("OnTransformersSkillInfo"))
		{
			g_Script.PushParameter(pTransformersMsg->bTransformerIndex);
			g_Script.Execute();
		}
		g_Script.CleanPlayer();
	}
}

void CTransformers::SendTransformersSkillInfo(SATransformersSkillInfo *pTransformersMsg, WORD wmsglenth)
{
	if (!pTransformersMsg)
	{
		return;
	}
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer) return;
	g_StoreMessage(pPlayer->m_ClientIndex, (LPVOID)pTransformersMsg, wmsglenth);
}
