#include "StdAfx.h"
#include "DynamicArena.h"


CDynamicArena::CDynamicArena() :m_CopySceneGID(0), m_DynamicIndex(0)
{
}


CDynamicArena::~CDynamicArena()
{
}

int CDynamicArena::OnCreate(_W64 long pParameter)
{
	return CRegion::OnCreate(pParameter);
}

void CDynamicArena::OnClose()
{
	return CRegion::OnClose();
}

int CDynamicArena::AddObject(LPIObject pChild)
{
	return CRegion::AddObject(pChild);
}

int CDynamicArena::DelObject(LPIObject pChild)
{
	return CRegion::DelObject(pChild);
}

void CDynamicArena::OnRun(void)
{
	return CRegion::OnRun();
}

int CDynamicArena::DelMonster(DWORD dwGID)
{
	return CRegion::DelMonster(dwGID);
}
