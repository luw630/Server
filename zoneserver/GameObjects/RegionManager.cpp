#include "StdAfx.h"
#include "regionmanager.h"

#include "region.h"
#include "fightarena.h"
#include "prison.h"
#include "biguanarea.h"
#include "eventregion.h"
#include "sportsarea.h"
#include "factionregion.h"
#include "dynamicregion.h"
#include "DynamicScene.h"
#include "DynamicArena.h"
#include "prisonex.h"
#include "SceneMap.h"
#include "networkmodule/refreshmsgs.h"
#include "DynamicDailyArea.h"

// #include "orbframework/orbframework.h"
// tForceReadMostlyOrbCenter<KILLINFO> orb(0,0,12345);
extern BOOL TraceInfo(LPCSTR szFileName, LPCSTR szFormat, ...);
extern LPCSTR _GetStringTime();

static CRegionManager *s_pUniqueThis = NULL; 
static LPIObject s_pFindRegion; 

CRegionManager::CRegionManager() :
    CObjectManager()
{
    s_pUniqueThis = this;
}

IObject* CRegionManager::CreateObject(const __int32 IID)
{
	if (IID == IID_REGION)
		return new CRegion;

    else if (IID == IID_FIGHTARENA)
		return new CFightArena;

    else if (IID == IID_PRISON)
		return new CPrison;

    else if (IID == IID_BIGUANAREA)
		return new CBiGuanArea;

    else if (IID == IID_EVENTREGION)
		return new CEventRegion;

    else if (IID == IID_SPORTSAREA)
	    return new CSportsArea;

    else if (IID == IID_FACTIONREGION)
	    return new CFactionRegion;
    else if ( IID == IID_PRISONEX )
        return new CPrisonEx;
	else if ( IID == IID_DYNAMICREGION )
		return new CDynamicRegion;
	else if (IID == IID_DYNAMICSCENE)
		return new CDynamicScene;
	else if (IID == IID_DYNAMICARENA)
		return new CDynamicArena;
	else if (IID == IID_DYNAMICDAILYAREA)
		return new CDynamicDailyArea;

    return NULL;
}

BOOL CALLBACK DoSaveBuildings( LPIObject &region, DWORD ID )
{
    if ( CRegion *ptr = ( CRegion* )region->DynamicCast( IID_REGION ) )
    {
        if ( ptr->m_IID == IID_FACTIONREGION || ptr->m_IID == IID_REGION )
            ptr->SaveBuildings();

        // 除帮派场景和普通场景以外的都不能保存建筑物数据！
        //if ( ptr->DynamicCast( IID_DYNAMICREGION ))
        //  return TRUE; // 动态场景不保存建筑物！
    }

    return TRUE;
}

bool CRegionManager::Run()
{
//     static DWORD prevTick = timeGetTime();
//     if ( abs( ( int )timeGetTime() - ( int )prevTick ) > 60*60*1000 ) // 一个小时保存一次（所有场景的所有建筑！）
//     {
//         prevTick = timeGetTime();
// 		if (s_pUniqueThis)
// 		{
// 			s_pUniqueThis->TraversalObjects( ( TRAVERSALFUNC )DoSaveBuildings, 0 );
// 		}
//     }

    return CObjectManager::Run();
}

bool CRegionManager::Destroy()
{
    return CObjectManager::Destroy();
}

void CRegionManager::SaveBuildings()
{
    // 退出前保存
	if (s_pUniqueThis)
	{
		s_pUniqueThis->TraversalObjects( ( TRAVERSALFUNC )DoSaveBuildings, 0 );
	}
}

BOOL CALLBACK FindRegionUseID(LPIObject &Region, DWORD ID)
{
	if (CRegion *pRegion = (CRegion*)Region->DynamicCast(IID_REGION))
    {
        if (Region->DynamicCast(IID_DYNAMICREGION) != NULL)
        {
            // 动态场景不纳入登陆服务器管理中！
            return TRUE;
        }

	    if (pRegion->m_wRegionID == ID)
        {
            s_pFindRegion = Region;
            return FALSE;
        }
    }

    return TRUE;
}

LPIObject FindRegionByID(DWORD ID)
{
    if (s_pUniqueThis == NULL)
        return LPIObject();

    LPIObject ret;
    s_pFindRegion.reset();
    s_pUniqueThis->TraversalObjects((TRAVERSALFUNC)FindRegionUseID, (LPARAM)ID);
    if (s_pFindRegion != NULL)
    {
        ret = s_pFindRegion;
        s_pFindRegion.reset();
    }

    return ret;
}

LPIObject FindRegionByGID(DWORD GID)
{
    if (s_pUniqueThis == NULL)
        return LPIObject();

    return s_pUniqueThis->LocateObject(GID);
}

BOOL CALLBACK SetRegionInfo(LPIObject &Region, SARefreshRegionServerMsg *pMsg)
{
    if (pMsg == NULL) return FALSE;
    if (pMsg->byRegionNumber >= MAX_REGION_NUMBER) return FALSE;

    if (CRegion *p = (CRegion*)Region->DynamicCast(IID_REGION))
    {
        if ( Region->DynamicCast(IID_DYNAMICREGION) || Region->DynamicCast(IID_FACTIONREGION) )
            return TRUE; // 动态场景和帮派场景不纳入登陆服务器管理中！

        SRegion *pRinfo = &pMsg->aRegions[pMsg->byRegionNumber];
        pRinfo->ID = p->m_wRegionID;
        pRinfo->MapID = p->m_wMapID;
		pRinfo->NeedLevel = (WORD)p->m_dwNeedLevel;
        pMsg->byRegionNumber++;
    }

    return TRUE;
}

BOOL FillRegionInfoMsg(SARefreshRegionServerMsg &msg, bool bFullRefresh)
{
    msg.byRegionNumber = 0;

    if (s_pUniqueThis == NULL)
        return FALSE;

    static int prev_number = 0;

    if ((prev_number != s_pUniqueThis->size()) || bFullRefresh)
    {
        s_pUniqueThis->TraversalObjects((TRAVERSALFUNC)SetRegionInfo, (LPARAM)&msg);
        prev_number = msg.byRegionNumber;
    }

    return msg.byRegionNumber;
}

BOOL CALLBACK TraceRegionInfo(LPIObject &Region, LPARAM i)
{
    if (CRegion *p = (CRegion*)Region->DynamicCast(IID_REGION))
    {
        rfalse((char)i, 1, "RID = %03d, PlayerNumber = %d", p->m_wRegionID, p->m_PlayerList.size());
    }

    return TRUE;
}

BOOL TraceAllRegionInfo(char i)
{
	if (s_pUniqueThis)
	{
		s_pUniqueThis->TraversalObjects((TRAVERSALFUNC)TraceRegionInfo, i);
	}
    return TRUE;
}

DWORD CreateDynamicRegion(const __int32 DR_IID, WORD RegionIndex, int RegionID, int RelifeRegionID, int RelifeX, int RelifeY, DWORD copySceneGID,WORD wPkType)
{
	LPIObject ret = s_pUniqueThis->GenerateObject(DR_IID, 0, RegionID);
    if (!ret)
        return 0;

    CDynamicRegion *pRegion = (CDynamicRegion *)ret->DynamicCast(IID_DYNAMICREGION);
    if (!pRegion)
        return false;

	pRegion->m_DynamicIndex = RegionIndex;

	if (RelifeRegionID)
		pRegion->m_wReLiveRegionID = RelifeRegionID;

	if (RelifeX)
		pRegion->m_ptReLivePoint.x = RelifeX;

	if(RelifeY)
		pRegion->m_ptReLivePoint.y = RelifeY;

	pRegion->m_dwPKAvailable = wPkType;

	pRegion->SetCopySceneGID(copySceneGID);
    
    return ret->GetGID();
}

// 检查动态场景
BOOL CALLBACK CallBackCheckDynamicRegion( LPIObject &region, LPARAM parm )
{
	CDynamicRegion *pDynamicRegion = ( CDynamicRegion* )region->DynamicCast( IID_DYNAMICREGION );

	if ( pDynamicRegion && pDynamicRegion->m_PlayerList.size() == 0 )
	{
		rfalse( 2, 1, "检测到未被删除的动态场景! 场景编号:[%d]", pDynamicRegion->m_wRegionID );

		// 返回FALSE后就不会继续后面的检查所以不能返回
		//return FALSE;
	}
	else
		parm ++;

	return TRUE;
}

void CheckDynamicRegion()
{
	if ( s_pUniqueThis == NULL )
		return ;

	int count = 0;

	rfalse( 2, 1, "###################################" );
	
	s_pUniqueThis->TraversalObjects( ( TRAVERSALFUNC )CallBackCheckDynamicRegion, ( LPARAM )&count );

	rfalse( 2, 1, "被创建的动态场景个数:[%d] 有效动态场景个数为:[%d]", 
		CDynamicRegion::s_DynamicRegionNumber, count );

	rfalse( 2, 1, "###################################" );

}

DWORD CreateDynamicScene(const __int32 DR_IID, WORD RegionIndex, int RegionID, int RelifeRegionID, int RelifeX, int RelifeY, DWORD copySceneGID,WORD pktype)
{
	LPIObject ret = s_pUniqueThis->GenerateObject(DR_IID, 0, RegionID);
	if (!ret)
		return 0;

	CDynamicScene *pScene = (CDynamicScene *)ret->DynamicCast(IID_DYNAMICSCENE);
	if (!pScene)
		return false;

	pScene->m_DynamicIndex = RegionIndex;
	pScene->m_dwPKAvailable = pktype;

	pScene->SetCopySceneGID(copySceneGID);

	return ret->GetGID();
}


DWORD CreateDynamicArena(const __int32 DR_IID, WORD RegionIndex, int RegionID, int RelifeRegionID, int RelifeX, int RelifeY, DWORD copySceneGID,WORD pktype)
{
	LPIObject ret = s_pUniqueThis->GenerateObject(DR_IID, 0, RegionID);
	if (!ret)
		return 0;

	CDynamicArena *pScene = (CDynamicArena *)ret->DynamicCast(IID_DYNAMICARENA);
	if (!pScene)
		return false;

	pScene->m_DynamicIndex = RegionIndex;

	pScene->SetCopySceneGID(copySceneGID);

	pScene->m_dwPKAvailable = pktype;

	return ret->GetGID();
}


DWORD CreateDynamicDailyArea(const __int32 DR_IID, WORD RegionIndex, int RegionID, int RelifeRegionID, int RelifeX, int RelifeY, DWORD copySceneGID)
{
	LPIObject ret = s_pUniqueThis->GenerateObject(DR_IID, 0, RegionID);
	if (!ret)
		return 0;

	CDynamicDailyArea *pScene = (CDynamicDailyArea *)ret->DynamicCast(IID_DYNAMICDAILYAREA);
	if (!pScene)
		return false;

	pScene->m_DynamicIndex = RegionIndex;

	pScene->SetCopySceneGID(copySceneGID);

	return ret->GetGID();
}

LPIObject CALLBACK CallBackCheckGetObject( LPIObject &region, LPARAM parm )
{
	CDynamicRegion *pdyregion =  (CDynamicRegion *)region->DynamicCast(IID_DYNAMICREGION);
	if (pdyregion)
	{
		return pdyregion->SearchObjectListByRegion(*(DWORD*)parm);
	}

	CRegion *pregion = (CRegion *)region->DynamicCast(IID_REGION);
	if (pregion)
	{
		return pregion->SearchObjectListByRegion(*(DWORD*)parm);
	}
	return LPIObject();
}

LPIObject GetObjectByGID(DWORD GID)
{
	if ( s_pUniqueThis == NULL )
		return LPIObject();
	 return s_pUniqueThis->FindObjects((FINDFUNC)CallBackCheckGetObject,( LPARAM )&GID);
}