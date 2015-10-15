#include "StdAfx.h"
#include "trigger.h"

CTrigger::CTrigger(void)
{
	memset(this, 0, sizeof(*this));
}

CTrigger::~CTrigger(void)
{
	if (TT_MAPTRAP_EVENT == m_dwType)
		SMapEvent.GetVariant().~Variant();
}