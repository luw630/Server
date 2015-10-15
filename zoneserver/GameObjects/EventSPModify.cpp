#include "Stdafx.h"
#include "FightObject.h"
#include "EventSPModify.h"

/*
void CEventSPModify::OnActive(EventMgr *mgr)
{
}

void CEventSPModify::SetParameter(CFightObject* pFightObject, DWORD dwDelay, int nModify, int nModifyCount, BOOL bInterrupt)
{
	m_pFightObject = pFightObject;
	m_dwDelay = dwDelay;
	m_nModify = nModify;
	m_nModifyCount = nModifyCount;
	m_bInterrupt = bInterrupt;
}

void CEventSPModify::OnInterrupt()
{
	if (m_bInterrupt)
		EventMgr::singleton().ResetEvent(this);
}


BOOL CBuffSPModify::BuffBegin( BuffContainer* owner, DWORD margin, DWORD times, int nModify )
{
	return true;
}

void CBuffSPModify::OnTimer( CFightObject *owner, int step )
{
}//*/