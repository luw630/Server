#include "StdAfx.h"
#include "fightListener.h"

AttribListener::AttribListener(CPlayer *pObserver)
{
	m_pObserver = pObserver;
}

void AttribListener::SetSubject(CFightObject* pSubject)
{
	CFightListener::SetSubject(pSubject);

	sendAttribute();
}

void AttribListener::OnExchange(CFightObject* pObj)
{
	sendAttribute();
}

void AttribListener::OnClose(CFightObject* pObj)
{
	if (m_pObserver)
	{
		m_pObserver->OnCloseListener(this);
	}
}

// ·¢ËÍÏûÏ¢
void AttribListener::sendAttribute()
{
	if (GetSubject() && m_pObserver)
	{
		SASelectTarget msg;
		msg.dwTargetGID = GetSubject()->GetGID();
		msg.dwMaxHp		= GetSubject()->m_MaxHp;
		msg.dwCurHp		= GetSubject()->m_CurHp;
		msg.dwMaxMp		= GetSubject()->m_MaxMp;
		msg.dwCurMp		= GetSubject()->m_CurMp;
		msg.wLevel		= GetSubject()->m_Level;

		g_StoreMessage(m_pObserver->m_ClientIndex, &msg, sizeof(msg));
	}
//	rfalse(2, 1, "SASelectTarget Send, TargetID: %d", msg.dwTargetGID);
}