#include "StdAfx.h"
#include "ItemUseEvent.h"

ReadyUseItemEvent::ReadyUseItemEvent(CPlayer *pUser, SPackageItem *pItem, DWORD dwEventInterrupType, int time) 
: ReadyEvent(dwEventInterrupType)
{
	if (!pUser || !pItem)
	{
		rfalse(4, 1, "Itemuser.cpp - !pUser || !pItem");
		return;
	}
	m_pUser = pUser;
	m_pItem = pItem; 

	// 通知客户端
	SAPreUseItemMsg msg;
	msg.time		= time;
	msg.operateType = SAPreUseItemMsg::OT_USE_NORMAL_ITEM; //临时
	
	g_StoreMessage(pUser->m_ClientIndex, &msg, sizeof(SAPreUseItemMsg));
	pUser->Synchrostates(EA_USEITEM);
}

void ReadyUseItemEvent::OnActive()
{
	if (m_pUser && m_pItem) {
		m_pUser->_UseItem(m_pItem);

		m_pUser->Synchrostates(EA_STAND);
		m_pUser->m_InUseItemevens = 0;
	}
}

void ReadyUseItemEvent::OnCancel()
{
	// 通知客户端
	SAPreUseItemMsg msg;
	msg.time		= 0;
	msg.operateType = SAPreUseItemMsg::OT_USE_ITEM_CANCEL;//

	if (m_pUser)
	{
		g_StoreMessage(m_pUser->m_ClientIndex, &msg, sizeof(SAPreUseItemMsg));
		m_pUser->Synchrostates(EA_STAND);
		m_pUser->m_InUseItemevens = 0;
		if (m_pItem){
			m_pUser->LockItemCell(m_pItem->wCellPos,FALSE);
		}		
	}
}

void ReadyUseItemEvent::SelfDestructor()
{
	if (m_pUser) 
		m_pUser->m_BatchReadyEvent.OnCloseReadyEvent(this); 
	delete this; 
}

void ReadyUseItemEvent::SelfDestory()
{
	SelfDestructor();
} 
