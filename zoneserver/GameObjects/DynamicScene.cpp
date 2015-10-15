#include "StdAfx.h"
#include "DynamicScene.h"


CDynamicScene::CDynamicScene(void):m_CopySceneGID(0),m_DynamicIndex(0)
{
}


CDynamicScene::~CDynamicScene(void)
{
	
}

int CDynamicScene::OnCreate( _W64 long pParameter )
{
	 return CRegion::OnCreate(pParameter);
}

void CDynamicScene::OnClose()
{
	    return CRegion::OnClose();
}

int CDynamicScene::AddObject( LPIObject pChild )
{
	return CRegion::AddObject(pChild);
}

int CDynamicScene::DelObject( LPIObject pChild )
{
	return CRegion::DelObject(pChild);
}

int CDynamicScene::DelMonster( DWORD dwGID )
{
	int ret = CRegion::DelMonster(dwGID);
	return ret;
}

void CDynamicScene::OnRun( void )
{
	CRegion::OnRun();
	return;
}
