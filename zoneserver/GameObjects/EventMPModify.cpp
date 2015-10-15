#include "Stdafx.h"
#include "FightObject.h"
#include "EventMPModify.h"

/*
void CEventMPModify::OnActive(EventMgr *mgr)
{
}

void CEventMPModify::SetParameter(CFightObject* pFightObject, DWORD dwDelay, int nModify, int nModifyCount, BOOL bInterrupt)
{
	m_pFightObject = pFightObject;
	m_dwDelay = dwDelay;
	m_nModify = nModify;
	m_nModifyCount = nModifyCount;
	m_bInterrupt = bInterrupt;
}

void CEventMPModify::OnInterrupt()
{
	if (m_bInterrupt)
		EventMgr::singleton().ResetEvent(this);
}


BOOL CBuffMPModify::BuffBegin( BuffContainer* owner, DWORD margin, DWORD times, int nModify )
{
	return true;
}

void CBuffMPModify::OnTimer( CFightObject *owner, int step )
{
}//*/