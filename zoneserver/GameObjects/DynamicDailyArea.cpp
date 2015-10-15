#include "stdafx.h"
#include "DynamicDailyArea.h"


CDynamicDailyArea::CDynamicDailyArea():m_CopySceneGID(0), m_DynamicIndex(0)
{
}


CDynamicDailyArea::~CDynamicDailyArea()
{
}

int CDynamicDailyArea::OnCreate(_W64 long pParameter)
{
	return CRegion::OnCreate(pParameter);
}

void CDynamicDailyArea::OnClose()
{
	return CRegion::OnClose();
}

int CDynamicDailyArea::AddObject(LPIObject pChild)
{
	return CRegion::AddObject(pChild);
}

int CDynamicDailyArea::DelObject(LPIObject pChild)
{
	return CRegion::DelObject(pChild);
}

void CDynamicDailyArea::OnRun(void)
{
	return CRegion::OnRun();
}

int CDynamicDailyArea::DelMonster(DWORD dwGID)
{
	return CRegion::DelMonster(dwGID);
}
