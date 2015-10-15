#pragma once
#include "./Player.h"
#include "./ItemUser.h"
#include "./EventTimer.h"


class ReadyUseItemEvent : public Event::ReadyEvent	// 吟唱道具事件
{
public:
	ReadyUseItemEvent(CPlayer* pUser, SPackageItem *pItem, DWORD dwEventInterrupType, int time);
	virtual void OnActive();
	virtual void OnCancel();
	virtual void SelfDestructor();
	virtual void SelfDestory(); 
public:
	CPlayer			*m_pUser;
	SPackageItem	*m_pItem;	
	DWORD			m_wUseType;
};

