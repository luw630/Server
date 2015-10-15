#include "Stdafx.h"
#include "EventBase.h"
#include "EventMgr.h"

bool EventBase::Interrupt()
{
	return EventManager::GetInstance().Interrupt(this);
}