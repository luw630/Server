#include "StdAfx.h"
#include "region.h"
#include "area.h"
#include "player.h"
#include "npc.h"
#include "monster.h"
#include "item.h"
#include "Pet.h"
#include "Trigger.h"
#include "building.h"
#include "pathfind\pathserver.h"
#include "NetworkModule\RegionMsgs.h"
#include "DynamicRegion.h"
#include "DynamicScene.h"

#include "GlobalFunctions.h"
#include "pub\ConstValue.h"
#include "ScriptManager.h"

extern LPCSTR GetStringParameter( LPCSTR string, int maxsize, int index );
extern int SendToLoginServer( SMessage *data, long size );
extern BOOL TraceInfo(LPCSTR szFileName, LPCSTR szFormat, ...);
extern LPCSTR _GetStringTime();
extern CScriptManager g_Script;

std::set< DWORD > GJLimitRegionMap;

ASHORT Judge(ASHORT xstart, ASHORT ystart, ASHORT xEnd, ASHORT yEnd)
{
    ASHORT dx=abs(xstart-xEnd);
    ASHORT dy=abs(ystart-yEnd);
    if (dx>dy) 
        return dx*10+dy*6;
    else
        return dx*6+dy*10;
}

char MoveCheck(ASHORT xstart, ASHORT ystart, CRegion *pRegion)
{	
    //m_Tiles
//     LPDWORD pdwTiles = pRegion->m_pdwTiles;
//     WORD rw = pRegion->m_RegionW;
// 
//     if (pdwTiles == 0 ) return false;
//     if (xstart >= pRegion->m_RegionW) return false;
//     if (ystart >= pRegion->m_RegionH) return false;
// 
//     // 检查地图本身数据的时候同时也检查地图标记数据
//     if ((pdwTiles[xstart + ystart*rw]&0x1) == 0) 
//         return false;

    return true;
}

IObject *CRegion::CreateObject(__int32 IID)
{
    if (IID == IID_AREA)
        return new CArea;

    if (IID == IID_NPC)
        return new CNpc;

    if (IID == IID_MONSTER)
        return new CMonster;

    if (IID == IID_ITEM)
        return new CItem;

    if (IID == IID_BUILDING)
        return new CBuilding;

	if (IID == IID_FIGHT_PET)
		return new CFightPet;

	if (IID == IID_SINGLEITEM)
		return new CSingleItem;

    return NULL;
}

int CRegion::OnCreate(_W64 long pParameter)
{
    prevBuildingNumber = -1;

    AreaTable = NULL;
    m_pdwTiles = NULL;
	m_pdwTileMarks = NULL;
    m_MapProperty = 
    m_ptReLivePoint.x = 
    m_ptReLivePoint.y = 
    m_nAreaW = 
    m_nAreaH = 
    m_wMapID = 
    m_RegionW = 
    m_RegionH = 
    m_wReLiveRegionID = 
    m_BgMusic = 0;
    m_bLoopPlay = 0;
    m_Weather = 0;
	m_nSafeZoneInfo	= 0;

    m_byMaxTrap = 0;
    m_pTrap = NULL;

    m_dwAIRefCount = 0;

    m_ExperMul = 1;


    m_IsHangup = 0;
    m_IsChildProtect = 0;
    m_IsDamageVenation = 0;
    m_IsBlock = 0;

    m_dwTickTime = 0;

#ifdef USEMAPCOLLISION
	m_pmapsever = NULL;
#endif

    memset( &skillEffect, 0, sizeof( skillEffect ) );
    // m_pAstar = NULL;

    static char pathname[256];
    static char filename[512];

    // 寻找具体的场景目标
    sprintf(pathname, "Map\\%04d*", pParameter);
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(pathname, &ffd);
    while (hFind != INVALID_HANDLE_VALUE)
    {
        if (ffd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
            break;

        if (!FindNextFile(hFind, &ffd))
            break;
    }

    if (hFind != INVALID_HANDLE_VALUE) 
    {
        dwt::strcpy(pathname, ffd.cFileName, 64);
    }
    else
    {
        sprintf(pathname, "%04d", pParameter);
        // return rfalse(2, 1, "无法找到[%s]场景目录！", pathname);
    }

    FindClose(hFind);

    // 装载场景信息
    sprintf(filename, "Map\\%s\\region.ini", pathname);

    IniFile inf;
    if (!inf.Load(filename))
        return rfalse(2, 1, "无法打开[%s]场景配置文件！", filename);

	int value = 0;
	int value2 = 0;
    float value3 = 0.0f;
	inf.GetInteger("MAIN", "MapID", 0, &value);
	m_wMapID = (WORD)value;

	inf.GetInteger("MAIN", "Property", 0, &value);
	m_MapProperty = (WORD)value;

	inf.GetInteger("MAIN", "RelifeMapID", 0, &value);
	m_wReLiveRegionID = (WORD)value;

	inf.GetInteger2("MAIN", "RelifePos", &value, &value2);
	m_ptReLivePoint.x = value;
	m_ptReLivePoint.y = value2;

	inf.GetInteger("MAIN", "Weather", 0, &value);
	m_Weather = (WORD)value;

	inf.GetInteger("MAIN", "Pktype", 0, &value);
	m_dwPKAvailable = (WORD)value;

    inf.GetInteger("MAIN", "PKValue", 10, &value);
    m_dwPKValueLimit = (DWORD)value;

	inf.GetInteger("MAIN", "Level", 0, &value);
	m_dwNeedLevel = (WORD)value;

	inf.GetInteger("MAIN", "SpecialAccess", 1, &value);
	m_dwSpecialAccess = (DWORD)value;

	inf.GetInteger("MAIN", "SkillAccess", 0, &value);
	m_wSkillAccess = (WORD)value;

    inf.GetInteger("MAIN", "BatchrRefresh", 0, &value);
    m_IsBatchrRefresh = (WORD)value;

    inf.GetFloat("MAIN", "RegionMultExp", 1.0f, &value3);
    m_wRegionMultExp = value3;

	inf.GetInteger("MAIN", "AllowedMount", 1, &m_bAllowedMount);

	m_wRegionMultExp = value3;
	//add by xj
	inf.GetInteger("MAIN", "practice", 0, &value);
	m_wPracticeSkillType = (WORD)value;

    // add by ab_yue
	char curStr[256];
	memset(curStr,0,sizeof(curStr));
	inf.GetString("MAIN", "LimitItemList", "", curStr, 256);
	SetParamToList(LimitItem,curStr);

	memset(curStr,0,sizeof(curStr));
	inf.GetString("MAIN", "unDealItemList", "", curStr, 256);
	SetParamToList(UnDealItem,curStr);

	memset(curStr,0,sizeof(curStr));
	inf.GetString("MAIN", "LimitSkillList", "", curStr, 256);
	SetParamToList(LimitSkill,curStr);

	memset(curStr,0,sizeof(curStr));
	inf.GetString("MAIN", "MonsterBuffList", "", curStr, 256);
	SetParamToList(TrigerBuffMonster,curStr);

	memset(curStr,0,sizeof(curStr));
	inf.GetString("MAIN", "PlyaerBuffList", "", curStr, 256);
	SetParamToList(TrigerBuffPlayer,curStr);

	inf.GetInteger("MAIN", "LimitMaxHP", 0, &value);
	m_wLimitMaxHP = (WORD)value;

	inf.GetInteger("MAIN", "LimitMaxMP", 0, &value);
	m_wLimitMaxMP = (WORD)value;

	inf.GetInteger("MAIN", "LimitMaxSP", 0, &value);
	m_wLimitMaxSP = (WORD)value;

	inf.GetInteger("MAIN", "LimitChangeEquip", 0, &m_bIsLimitChangeEquip);

	//end
	
	///安全区域属性
	inf.GetInteger("MAIN", "SafeZoneInfo", 0, &value);
	m_nSafeZoneInfo=(BYTE)value;

    inf.GetInteger( "ENVIRONMENT", "DAMAGEVENATION", 1, &m_IsDamageVenation);
    inf.GetInteger( "ENVIRONMENT", "HANGUP", 1, &m_IsHangup );
    inf.GetInteger( "ENVIRONMENT", "CHILDPROTECT", 0, &m_IsChildProtect );
    inf.GetInteger( "ENVIRONMENT", "BLOCK", 0, &m_IsBlock );
    
    for ( int i = 0; i < SRegionSkillEffect::SkillEffectMAX; i++ )
    {
        LPCSTR title = FormatString( "SKILLEFFECT%d" , i );
        inf.GetInteger( title , "ID", 0, &value);
        skillEffect[i].id = value;
        inf.GetInteger( title , "ODDS", 0, &value);
        skillEffect[i].odds = value;
    }

	if (m_wMapID == 0)
        return rfalse(2, 1, "无效的场景配置文件");

	m_wRegionID = (WORD)pParameter;

    if (LoadTileMark(m_wMapID) == 0)
        return rfalse(2, 1, "错误的场景标记数据");

    sprintf(filename, "Map\\%s\\npc.inf", pathname);
    if (LoadNpc(filename) == 0)
        rfalse(2, 1, "%s好像有问题", pathname);

    sprintf(filename, "Map\\%s\\monster.inf", pathname);
    if (LoadMonster(filename) == 0)
        rfalse(2, 1, "%s好像有问题", pathname);

    sprintf(filename, "Map\\%s\\trigger.inf", pathname);
    if (LoadTrigger(filename) == 0)
        rfalse(2, 1, "%s好像有问题", pathname);

    sprintf(filename, "Map\\%s\\timer.inf", pathname);
    LoadTimer(filename);

#ifdef USEMAPCOLLISION
	m_bUseCollision = true;
	if(LoadRegincollision(m_wMapID)==0)
	{
		rfalse(2, 1, FormatString("场景%d碰撞文件读取失败",m_wMapID));
		m_bUseCollision = false;
	}
#endif
    //m_pAstar = new AstarPathFind;
    //m_pAstar->Create(m_RegionW, m_RegionH, MoveCheck, Judge, this);

	m_dwPKX = PXT_NULL;

    sprintf(filename, "Map\\%s\\monsterBR.inf", pathname);
    if ( m_IsBatchrRefresh )
       LoadMonsterBR(filename);    // 不一定有这个配置所以读取失败了也没关系。

    return 1;
}

void CRegion::OnClose()
{
	for (int i=0; i<m_nAreaW * m_nAreaH; i++)
	{
		CArea *p = (CArea*)(AreaTable[i]->DynamicCast(IID_AREA));

		p->m_ItemList.clear();
		p->m_MonsterList.clear();
		p->m_NpcList.clear();
		p->m_PlayerList.clear();
		p->m_BuildingList.clear();
		p->m_singleItemList.clear();
	}

	Destroy();

    if (AreaTable)
	{
        delete [] AreaTable;
		AreaTable = 0;
	}

    if (m_pTrap)
	{
        delete [] m_pTrap;
		m_pTrap = 0;
	}

    m_pdwTiles		= 0;
    m_pdwTileMarks	= 0;
    m_byMaxTrap		= 0;

#ifdef USEMAPCOLLISION
	SAFE_DELETE(m_pmapsever);
#endif
    return;
}

void CRegion::OnRun(void)
{
	// 如果没有玩家在这个场景，那么就不需要处理这里的AI了
    if (m_PlayerList.size() == 0 && m_dwAIRefCount == 0)
        return;

	g_Script.CallFunc("reginrun",m_wRegionID);
    Run(); // 执行每一个Area的AI部分

    m_dwTickTime++;
    return;
}

int CRegion::AddObject(LPIObject pChild)
{
	if (CPlayer *pPlayer = (CPlayer *)pChild->DynamicCast(IID_PLAYER))
	{
		// 九字杀阵初始化
// 		if (pPlayer->m_NineWordsKill)
// 		{
// 			pPlayer->m_NineWordsKill = 0;
// 
// 			SANineWordMsg _msg;
// 			_msg.num = pPlayer->m_NineWordsKill;
// 
// 			g_StoreMessage(pPlayer->m_ClientIndex, &_msg, sizeof(SANineWordMsg));
// 		}
		// ==============

		pPlayer->OnRegionChanged(this, true);

		// 在这里需要先将玩家放到当前场景中，因为后面的m_ParentRegion->DelObject(pChild)函数会调用CS_OnCheckTerminate
		// CS_OnCheckTerminate中会判断动态场景中是否还有玩家，如果没有就会删除，从而可能导致崩溃.
		m_PlayerList.push_back(pChild);

		// 这个是玩家在场景与场景之间的转换
		if (pPlayer->m_ParentRegion)
			pPlayer->m_ParentRegion->DelObject(pChild);

		pPlayer->m_ParentRegion = this;
		
		// 同步场景信息
		SASetRegionMsg msg;
		msg.wMapID			= m_wMapID;
		msg.wCurRegionID	= m_wRegionID;
		msg.x				= pPlayer->m_curX;
		msg.y				= pPlayer->m_curY;
		msg.z				= pPlayer->m_curZ;
		msg.wPkType = m_dwPKAvailable;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SASetRegionMsg));

		if (Move2Area(pChild, pPlayer->m_curTileX, pPlayer->m_curTileY, true) == 0)
			return rfalse(2, 1, "AddObject: Move2Area Fail");

		pPlayer->OnRegionChanged(this, false);

        return 1;
    }
    else if (CMonster *pMonster = (CMonster*)pChild->DynamicCast(IID_MONSTER))
    {
        // 怪物只能放入场景，不能转换场景
        if (pMonster->m_ParentRegion)
            return 0;

        pMonster->m_ParentRegion = this;

        if (Move2Area(pChild, pMonster->m_curTileX, pMonster->m_curTileY, true) == 0)
            return rfalse(2, 1, "Move2Area fail");

		pMonster->OnCreateEnd();
        return 1;
    }
	else if (CNpc *pNpc = (CNpc *)pChild->DynamicCast(IID_NPC))
	{

	}
	else if (CFightPet *pPet = (CFightPet *)pChild->DynamicCast(IID_FIGHT_PET))
	{
		if (pPet->m_ParentRegion)
			pPet->m_ParentRegion->DelObject(pChild);

		pPet->m_ParentRegion = this;

		float fX	= pPet->m_owner->m_curX;
		float fY	= pPet->m_owner->m_curY;

		pPet->m_curX	 = fX;
		pPet->m_curY	 = fY;
		pPet->m_curTileX = ((DWORD)fX) >> TILE_BITW;
		pPet->m_curTileY = ((DWORD)fY) >> TILE_BITH;

		if (Move2Area(pChild, ((DWORD)fX) >> TILE_BITW, ((DWORD)fY) >> TILE_BITH, true) == 0)
			return rfalse(2, 1, "Move2Area fail");
			
		return 1;
	}

    return 0;
}

int CRegion::DelObject(LPIObject pChild)
{
    SADelObjectMsg msg;

    if (CPlayer *pPlayer = (CPlayer*)pChild->DynamicCast(IID_PLAYER))
    {
        if (pPlayer->m_ParentRegion != this)
            return 0;

        if (pPlayer->m_ParentArea)
        {
            msg.dwGlobalID = pPlayer->GetGID();
            pPlayer->m_ParentArea->SendAdj(&msg, sizeof(msg), pPlayer->m_ClientIndex);
            pPlayer->m_ParentArea->DelObject(pChild);
        }

        m_PlayerList.remove(pChild);

        pPlayer->m_ParentRegion = 0;
    }
	else if (CFightPet *pPet = (CFightPet *)pChild->DynamicCast(IID_FIGHT_PET))
	{
		if (pPet->m_ParentRegion != this)
			return 0;

		if (pPet->m_ParentArea)
		{
			msg.dwGlobalID = pPet->GetGID();
			pPet->m_ParentArea->SendAdj(&msg, sizeof(msg), -1);
			pPet->m_ParentArea->DelObject(pChild);
		}

		pPet->m_ParentRegion = 0;
	}

    return 0;
}

int CRegion::LoadTileMark(WORD ID)
{
	char name[32];
 	char buffer[128];
 	sprintf(name, "Map\\%d.ini", ID);

	FILE *fp = fopen(name, "r");
	if (!fp)
		return 0;

	fgets(buffer, 128, fp);
	sscanf(buffer, "%d %d", &m_RegionW, &m_RegionH);

	fclose(fp);

	if (0 == m_RegionW || 0 == m_RegionH)
		return 0;

	// 将整个场景分割，分配到每一个Area中去
    static CArea::SInitParam temp;

    memset(&temp, 0, sizeof(temp));

	// 这里的计算和确认对象在哪个区域的计算不同，这里只是计算最大的划分区域数
    m_nAreaW = (m_RegionW/_AreaW)+((m_RegionW%_AreaW)?1:0);
    m_nAreaH = (m_RegionH/_AreaH)+((m_RegionH%_AreaH)?1:0);

    AreaTable = new LPIObject[m_nAreaW*m_nAreaH];

	temp.pParentRegion = this;
	for (temp.y=0; temp.y<m_nAreaH; temp.y++)
    {
        for (temp.x=0; temp.x<m_nAreaW; temp.x++)
        {
            // 割出一个个_AreaW*_AreaH(Tile)的小区域
            AreaTable[temp.y*m_nAreaW + temp.x] = GenerateObject(IID_AREA, 0, (LPARAM)&temp);
        }
    }

    // 因为不想让CArea参与游戏AI，所以将当前的场景成员清空
    CObjectManager::Clear();

    return 1;
}

int CRegion::LoadNpc(LPCSTR filename)
{
    return 1;
}

int CRegion::LoadMonster(LPCSTR filename)
{
    static char str[128];
    dwt::ifstream stream(filename);
    if (!stream.is_open())
        return rfalse(2, 1, "无法打开[%s]Monster列表文件！", filename);

    while (!stream.eof())
    {
        stream >> str;
        if (dwt::strcmp(str, "·", 3) == 0)
            break;
    }

    DWORD nTimes = 0;
    if (!stream.eof())
    {
        stream >> str;
        if (dwt::strcmp(str, "MonsterNumber", 14) == 0)
            stream >> nTimes;
    }

    LPIObject pObject;
    CMonster::SParameter param;
    memset(&param, 0, sizeof(param));
	long k = 0;
    while (nTimes--)
    {
		/*if (m_wRegionID != 2 )
		{
			continue;
		}
		
		if (k == 1)
		{
			continue;
		}
		k++;*/
        stream >> param.wListID >> param.wX >> param.wY >> param.dwRefreshTime;

		param.reginID = m_wRegionID;

		param.dwRefreshTime = 0;

        pObject = GenerateObject(IID_MONSTER, 0, (LPARAM)&param);
        CMonster *pMonster = (CMonster*)pObject->DynamicCast(IID_MONSTER);

        if (pMonster == NULL)
            return rfalse(2, 1, "Monster读取失败");

        if (AddObject(pObject) == 0)
            return rfalse(2, 1, "无法将Monster放入场景");
    }

    stream.close();

    return 1;
}

int CRegion::LoadTrigger(LPCSTR filename)
{
    static char str[128];
    dwt::ifstream stream(filename);
    if (!stream.is_open())
        return rfalse(2, 1, "无法打开[%s]陷阱列表文件！", filename);

    while (!stream.eof())
    {
        stream >> str;
        if (dwt::strcmp(str, "·", 3) == 0)
            break;
    }

    DWORD nTimes = 0;
    if (!stream.eof())
    {
        stream >> str;
		if (dwt::strcmp(str, "TrapNumber", CONST_USERNAME) == 0)
            stream >> nTimes;
    }

    m_byMaxTrap = (BYTE)nTimes;
    m_pTrap = new CTrigger[m_byMaxTrap];
    while (nTimes--)
    {
        int index = 0;
        stream >> index;
        index--;
        if ((DWORD)index >= m_byMaxTrap)
            return rfalse(0, 0, "触发器数据有错");

        CTrigger *pT = &m_pTrap[index];
        DWORD temp;
        stream >> temp;
        pT->m_dwType = temp;
        switch (temp)
        {
        case CTrigger::TT_CHANGE_REGION:
            stream >> pT->SChangeRegion.wNewRegionID >> pT->SChangeRegion.wStartX >> pT->SChangeRegion.wStartY;
			pT->SChangeRegion.dwNewRegionGID = 0;
            break;

        case CTrigger::TT_MAPTRAP_EVENT:
            stream >> pT->SMapEvent.dwScriptID;
            break;

        default:
			return rfalse(0, 0, "目前不支持的触发器类型,文件:%s, 类型: %d", filename, temp);
        }
    }

    stream.close();

    return 1;
}

int CRegion::LoadTimer(LPCSTR filename)
{
    return 0;
}

template<typename T>
BOOL CheckStream( std::strstream &stream, T &dwData, LPCSTR filename, DWORD dwTimes )
{
    DWORD dwTemp = -1;
    stream >> dwTemp;

    if ( dwTemp == -1 )
    {
        rfalse(2, 1, "[%s]文件数据不完整！,[%d]批次刷怪失败", filename, dwTimes);
        return FALSE;
    }
    else
    {
        dwData = ( T )dwTemp;
        return TRUE;
    }
}

int CRegion::LoadMonsterBR(LPCSTR filename)
{
	/*
    static char str[128];
    dwt::ifstream stream(filename);
    if (!stream.is_open())
        return rfalse(2, 1, "无法打开[%s]monsterBR列表文件！", filename);

    while (!stream.eof())
    {
        stream >> str;
        if (dwt::strcmp(str, "·", 3) == 0)
            break;
    }

    DWORD dwTimes = 0;
    if (!stream.eof())
    {
        stream >> str;
        if (dwt::strcmp(str, "MonsterTeams", 13) == 0)
            stream >> dwTimes;
    }
    if ( dwTimes == 0 )
        return 0;

    char tmpStr[2048];
    DWORD dwMaxTimes = dwTimes;
    LPIObject pObject;
    CMonster::SParameter param;

    memset(&param, 0, sizeof(param));
    param.extraData = NULL;    

    while ( dwTimes  && !stream.eof() )
    {
        stream.getline( tmpStr, sizeof( tmpStr ) );
        if ( tmpStr[0] == 0 )
            continue;

		std::strstream stream( tmpStr, (std::streamsize)strlen( tmpStr ) );   

        stream >> param.dwBRTeamNumber;
        if ( param.dwBRTeamNumber == 0 )
            continue;

         --dwTimes;

        stream >> str;

        if ( !CheckStream( stream, param.wListID,          filename, dwMaxTimes - dwTimes ) )
            continue;
        if ( !CheckStream( stream, param.wCenterX,         filename, dwMaxTimes - dwTimes ) )
            continue;
        if ( !CheckStream( stream, param.wCenterY,         filename, dwMaxTimes - dwTimes ) )
            continue;
        if ( !CheckStream( stream, param.wBRArea,          filename, dwMaxTimes - dwTimes ) )
            continue;
        if ( !CheckStream( stream, param.dwRefreshTime,    filename, dwMaxTimes - dwTimes ) )
            continue;
        if ( !CheckStream( stream, param.dwBRNumber,       filename, dwMaxTimes - dwTimes ) )
            continue;
        if ( !CheckStream( stream, param.wRefreshStyle,    filename, dwMaxTimes - dwTimes ) )
            continue;
        if ( !CheckStream( stream, param.wAIType,          filename, dwMaxTimes - dwTimes ) )
            continue;
        if ( !CheckStream( stream, param.wMoveArea,        filename, dwMaxTimes - dwTimes ) )
            continue;
        if ( !CheckStream( stream, param.wRefreshScriptID, filename, dwMaxTimes - dwTimes ) )
            continue;
        if ( !CheckStream( stream, param.wAIScriptID,      filename, dwMaxTimes - dwTimes ) )
            continue;
        if ( !CheckStream( stream, param.wDeadScriptID,    filename, dwMaxTimes - dwTimes ) )
            continue;

        dwt::strcpy(param.szName, str, CONST_USERNAME); 

        if ( param.dwRefreshTime == 0 )
            param.dwRefreshTime = 1;

        while ( (param.dwBRNumber < 1000) && (param.dwBRNumber--) )
        {
            // 第一次确定刷新坐标
            extern void RandomPos( CRegion *parentRegion, WORD &wDesPosX, WORD &wDesPosY, WORD wCenterX, WORD wCenterY, WORD wBRArea, DWORD dwSearchTimes );
            RandomPos( this, param.wX, param.wY, param.wCenterX, param.wCenterY, param.wBRArea, 500);

            pObject = GenerateObject(IID_MONSTER, 0, (LPARAM)&param);
            CMonster *pMonster = (CMonster*)pObject->DynamicCast(IID_MONSTER);
            if (pMonster == NULL)
            {
                rfalse(2, 1, "Monster 读取失败");
                break;
            }

            if (AddObject(pObject) == 0)
            {
                rfalse(2, 1, "无法将monster放入场景");
                break;
            }
        }
    }

    stream.close();
	//*/

    return 1;
}

LPIObject &CRegion::GetArea(WORD x, WORD y)
{
    static LPIObject nil;

    if (x >= m_nAreaW) return nil;
    if (y >= m_nAreaH) return nil;
	if (x < 0) return nil;
	if (y < 0) return nil;

    return AreaTable[y*m_nAreaW + x];
}

BOOL CRegion::Move2Area(LPIObject pObj, WORD x, WORD y, bool bUseTile)
{
	if (!pObj) 
		return false;

	WORD oriX = x;
	WORD oriY = y;

	if (bUseTile)
    {
        x = GetCurArea(x, _AreaW);
		y = GetCurArea(y, _AreaH);
	}

	if (CArea *pArea = (CArea*)GetArea(x, y)->DynamicCast(IID_AREA))
		return pArea->AddObject(pObj);

    // 意外处理
    if (CArea *pArea = (CArea*)GetArea(0, 0)->DynamicCast(IID_AREA))
        return pArea->AddObject(pObj);

    return FALSE;
}

check_list<LPIObject>* CRegion::GetMonsterListByAreaIndex(INT32 x,INT32 y)
{
	CArea *pArea = (CArea*)GetArea(x, y)->DynamicCast(IID_AREA);
	if (!pArea)
		return NULL;

	return &pArea->m_MonsterList;
}

check_list<LPIObject>* CRegion::GetPlayerListByAreaIndex(INT32 x,INT32 y)
{
	CArea *pArea = (CArea*)GetArea(x, y)->DynamicCast(IID_AREA);
	if (!pArea)
	{
		return NULL;
	}

	return &pArea->m_PlayerList;
}

LPIObject CRegion::SearchNpcByScriptID(DWORD dwID)
{
	LPIObject PObject;

	for (int i=0; i<m_nAreaW * m_nAreaH; i++)
	{
		CArea *pArea = (CArea*)(AreaTable[i]->DynamicCast(IID_AREA));

		if (pArea)
		{
			check_list<LPIObject>::iterator iter;
			for (iter = pArea->m_NpcList.begin(); iter != pArea->m_NpcList.end(); ++iter) 
			{
				CNpc *pNpc = (CNpc*)(*iter)->DynamicCast(IID_NPC);
				if (pNpc->GetGID() == dwID) //兼容GID
				{
					return *iter;
				}
				if (pNpc && pNpc->m_Property.m_wClickScriptID == dwID) //兼容ScriptID
				{
					return *iter;
				}
			}
		}
	}

	return PObject;
}

LPIObject SearchObjectListByArea(CArea *pArea, DWORD dwGID)
{
	if (pArea)
	{
		switch (dwGID >> 28)
		{
		case 0: // player
			return g_SearchInObjectList(pArea->m_PlayerList, dwGID);

		case 1: // npc
			return g_SearchInObjectList(pArea->m_NpcList, dwGID);

		case 2: // monster
			return g_SearchInObjectList(pArea->m_MonsterList, dwGID);

		case 3: // item
			return g_SearchInObjectList(pArea->m_ItemList, dwGID);

		case 4: // pet
			return g_SearchInObjectList(pArea->m_FightPetList, dwGID);

		case 5: // singleitem
			return g_SearchInObjectList(pArea->m_singleItemList, dwGID);
		}
	}

	return LPIObject();
}

LPIObject CRegion::SearchObjectListByRegion(DWORD dwGID)
{
	LPIObject PObject;

	for (int i=0; i<m_nAreaW * m_nAreaH; i++)
	{
		CArea *pArea = (CArea*)(AreaTable[i]->DynamicCast(IID_AREA));
		PObject = SearchObjectListByArea(pArea, dwGID);
		if(PObject!=NULL) break;
	}

	return PObject;
}

LPIObject CRegion::SearchObjectListInArea(DWORD dwGID, WORD x, WORD y)
{
	CArea *pArea = 0;
	pArea = (CArea*)GetArea(x, y)->DynamicCast(IID_AREA);
	return SearchObjectListByArea(pArea, dwGID);
}

LPIObject CRegion::SearchObjectListInAreas(DWORD dwGID, WORD xCenter, WORD yCenter)
{
    LPIObject PObject;

    for (int y = yCenter-1; y <= yCenter+1; ++y)
		for (int x = xCenter-1; x <= xCenter+1; ++x)
		{
			PObject = SearchObjectListInArea(dwGID, x, y) ;
			if ( !PObject )
				continue;
			else
				goto _end;
		}

_end:
    return PObject;
}

LPIObject CRegion::SearchObjectByPos(WORD xCenter, WORD yCenter)
{
//     WORD x = GetCurArea(xCenter, _AreaW);
//     WORD y = GetCurArea(yCenter, _AreaH);
// 
//     if (CArea *pArea = (CArea*)GetArea(x, y)->DynamicCast(IID_AREA))
//     {
//         check_list<LPIObject>::iterator iter;
//         for (iter = pArea->m_MonsterList.begin(); iter != pArea->m_MonsterList.end(); iter++ )
//             if (CActiveObject *pAct = (CMonster *)(*iter)->DynamicCast(IID_ACTIVEOBJECT))
//                 if (pAct->m_wCurX == xCenter)
//                     if (pAct->m_wCurY == yCenter)
//                         return *iter;
// 
//         for (iter = pArea->m_PlayerList.begin(); iter != pArea->m_PlayerList.end(); iter++ )
//             if (CActiveObject *pAct = (CMonster *)(*iter)->DynamicCast(IID_ACTIVEOBJECT))
//                 if (pAct->m_wCurX == xCenter)
//                     if (pAct->m_wCurY == yCenter)
//                         return *iter;
//     }
	
    return LPIObject();
}

DWORD CRegion::GetTileMark(WORD CurX, WORD CurY)
{
	return 0;

//     if (m_pdwTiles == 0) return 0;
//     if (CurX >= m_RegionW) return 0;
//     if (CurY >= m_RegionH) return 0;
// 
//     return m_pdwTiles[CurX + CurY*m_RegionW];
}

void CRegion::MarkTileStop(WORD CurX, WORD CurY, DWORD &PrevMark, DWORD GID)
{
//     WORD PX = HIWORD(PrevMark);
//     WORD PY = LOWORD(PrevMark);
// 
//     if ((PX == CurX)&&(PY == CurY)) return;
// 
//     if (m_pdwTileMarks == 0) return;
//     if (CurX >= m_RegionW) return;
//     if (CurY >= m_RegionH) return;
// 
//     if (PX >= m_RegionW) PX = 0;
//     if (PY >= m_RegionH) PY = 0;
// 
//     // 这样做的目的是，保证以前位置上的角色的MARK点，不会被意外的覆盖
//     if (m_pdwTileMarks[PX + PY*m_RegionW] == GID)
//         m_pdwTileMarks[PX + PY*m_RegionW] = 0;
// 
//     // 这样做的目的是，保证以前位置上的角色的MARK点，不会被意外的覆盖
//     if (m_pdwTileMarks[CurX + CurY*m_RegionW] == 0)
//         m_pdwTileMarks[CurX + CurY*m_RegionW] = GID;
// 
//     PrevMark = MAKELONG(CurY, CurX);
}

bool CRegion::isTileWalkAble(WORD CurX, WORD CurY)
{
	return false;

//     if (m_pdwTiles == 0) return false;
//     if (CurX >= m_RegionW) return false;
//     if (CurY >= m_RegionH) return false;
// 
//     return m_pdwTiles[CurX + CurY*m_RegionW] & (0x1);
}

bool CRegion::isTileStopAble(WORD CurX, WORD CurY, DWORD GID)
{
	return false;

//     if (m_pdwTiles == 0) return false;
//     if (CurX >= m_RegionW) return false;
//     if (CurY >= m_RegionH) return false;
// 
//     if (m_pdwTileMarks[CurX + CurY*m_RegionW] == GID) return true;
// 
//     if (m_pdwTiles[CurX + CurY*m_RegionW] & (0x1))
//         return m_pdwTileMarks[CurX + CurY*m_RegionW] == 0;
// 
//     return false;
}

const CTrigger *CRegion::GetTrapTrigger( WORD index )
{
    if ( index > 255 )
    {
        std::map< int, CTrigger >::iterator it = trapMap.find( index );
        if ( it == trapMap.end() )
            return NULL;

        return &it->second;
    }

    BYTE byIndex = ( 0xff & index ) - 1;
    if ( byIndex >= m_byMaxTrap )
        return NULL;

    return &m_pTrap[ byIndex ];
}

CFightPet *CRegion::CreateFightPet(PVOID pParameter)
{
	CFightPet::SParameter *pParam = static_cast<CFightPet::SParameter *>(pParameter);
	if (!pParam)
		return 0;

	LPIObject pObject = GenerateObject(IID_FIGHT_PET, 0, (LPARAM)pParam);

	CFightPet *pFightPet = (CFightPet *)pObject->DynamicCast(IID_FIGHT_PET);
	if (!pFightPet)
		return 0;

	if (0 == Move2Area(pObject, pFightPet->m_curTileX, pFightPet->m_curTileY, true))
		return 0;

	return pFightPet;
}

// 对象生成
CMonster *CRegion::CreateMonster(PVOID pParameter, DWORD controlId)
{
	CMonster::SParameter *pParam = static_cast<CMonster::SParameter *>(pParameter);
    if (!pParam)
		return 0;

	LPIObject pObject = GenerateObject(IID_MONSTER, 0, (LPARAM)pParam);
   
    CMonster *pMonster = (CMonster*)pObject->DynamicCast(IID_MONSTER);
	if (!pMonster)
		return 0;

	if (Move2Area(pObject, pMonster->m_curTileX, pMonster->m_curTileY, true) == 0) 
	{
		rfalse(2, 1, "无法将monster放入场景!");
        return 0;
    }

	pMonster->m_Property.controlId = controlId;
	if (pMonster->m_Property.controlId)
        controlIdMap[controlId] = pMonster->GetGID();
    
	return pMonster;
}

int CRegion::DelMonster(DWORD dwGID)
{
	LPIObject pObject = LocateObject(dwGID);
	if(!pObject)
	{
		rfalse(2, 1, "Monster 没找到，可能已经删除了");
		return 0;
	}

	CMonster *pMonster = (CMonster*)pObject->DynamicCast(IID_MONSTER);
    if (!pMonster) 
	{
        rfalse(2, 1, "Monster 没找到，可能已经删除了");
        return 0;
    }

	if (pMonster->m_ParentRegion != this)
        return 0;

	if (pMonster->m_Property.controlId)
        controlIdMap.erase(pMonster->m_Property.controlId);

    SADelObjectMsg msg;

    if (pMonster->m_ParentArea)   
	{
        msg.dwGlobalID = dwGID;
        pMonster->m_ParentArea->SendAdj(&msg, sizeof(SADelObjectMsg), -1);
		pMonster->m_ParentArea->DelObject(pObject);
    }

    pMonster->isValid() = false;
    return 1;
}

CNpc *CRegion::CreateNpc(PVOID pParameter, DWORD controlId)
{
	CNpc::SParameter *pParam = static_cast<CNpc::SParameter *>(pParameter);
	if (!pParam) 
		return 0;

	LPIObject pObject = GenerateObject(IID_NPC, 0, (LPARAM)pParam);
   
	CNpc *pNpc = (CNpc*)pObject->DynamicCast(IID_NPC);
    if (!pNpc) 
	{
        rfalse(2, 1, "Npc 生成失败");
        return 0;
    }

	if (0 == Move2Area(pObject, pNpc->m_curTileX, pNpc->m_curTileY, true)) 
	{
        rfalse(2, 1, "无法将npc放入场景");
        return 0;
    }

	if (pNpc->m_Property.controlId = controlId)
        controlIdMap[controlId] = pNpc->GetGID();

	//rfalse(2, 1, "建立NPC：%d",  pNpc->m_Property.m_wClickScriptID);
	return pNpc;
}

int CRegion::DelFightPet(DWORD dwGID)
{
	LPIObject pObject = LocateObject(dwGID);
	CFightPet *pPet = (CFightPet *)pObject->DynamicCast(IID_FIGHT_PET);

	if (!pPet || pPet->m_ParentRegion != this)
		return 0;

	SADelObjectMsg msg;

	if (pPet->m_ParentArea)
	{
		msg.dwGlobalID = dwGID;
		pPet->m_ParentArea->SendAdj(&msg, sizeof(SADelObjectMsg), -1);
		pPet->m_ParentArea->DelObject(pObject);
	}

	pPet->isValid() = false;

	return 1;
}

int CRegion::DelNpc(DWORD dwGID)
{
	LPIObject pObject = LocateObject(dwGID);
	if (!pObject)
		return 0;

	CNpc *pNpc = (CNpc*)pObject->DynamicCast(IID_NPC);
	if (!pNpc)
		return 0;

	if (pNpc->m_ParentRegion != this)
        return 0;

	if (pNpc->m_Property.controlId)
		controlIdMap.erase(pNpc->m_Property.controlId);

	SADelObjectMsg msg;

	if (pNpc->m_ParentArea)   
	{
        msg.dwGlobalID = dwGID;
        pNpc->m_ParentArea->SendAdj(&msg, sizeof(SADelObjectMsg), -1);
        pNpc->m_ParentArea->DelObject(pObject);
    }

    pNpc->isValid() = false;

    return 1;
}

int CRegion::Broadcast(SMessage *pMsg, WORD wSize, DNID dnidExcept)
{
    if (pMsg == NULL || wSize == 0)
        return 0;

    check_list<LPIObject>::iterator iter = m_PlayerList.begin();
    while (iter != m_PlayerList.end())
    {
        CPlayer *pPlayer = (CPlayer *)(*iter)->DynamicCast(IID_PLAYER);
        if (pPlayer)
        {
            if (pPlayer->m_ClientIndex != dnidExcept)
            {
                g_StoreMessage(pPlayer->m_ClientIndex, pMsg, wSize);
            }
        }

        iter++;
    }

    return 1;
}

bool IsSpaceText( std::string str)
{
	if( str.size() == 0 )
		return true;
	for( int i = 0; i < (int)str.size()-1; i ++ )
	{
		if( str[i] != ' ' )
			return false;
	}
	return true;
}
void CRegion::GetAreaNpcList( std::vector<CRegion::NpcList>& npcList )
{
// 	std::map<unsigned __int32, LPIObject>::iterator iter= objectMap.begin(); 
//     while (iter != objectMap.end())
//     {
//         CNpc *pNpc = (CNpc *)iter->second->DynamicCast(IID_NPC);
//         if (pNpc)
//         {
// 			if( pNpc->m_Property.m_tType == 1 )
// 			{
// 				iter++;
// 				continue;
// 			}
// 			
// 			/* For XYD3
// 			CRegion::NpcList tempList;
// 			tempList.type = (pNpc->m_Property.m_tType > 4 ||  pNpc->m_Property.m_tType < 0 ) ? 0 : pNpc->m_Property.m_tType;
// 			tempList.x = pNpc->m_Property.m_wSrcX;
// 			tempList.y = pNpc->m_Property.m_wSrcY;
// 			tempList.npcid = pNpc->m_Property.controlId-100000;
// 			tempList.gid = pNpc->GetGID();
// 			
// 
// 			memset( tempList.name, 0, CONST_USERNAME );
// 			if( !IsSpaceText( std::string( pNpc->m_Property.m_Name ) ) )
// 				memcpy( tempList.name, pNpc->m_Property.m_Name, CONST_USERNAME );
// 			else if( !IsSpaceText( std::string( pNpc->m_Property.m_szTitle ) ) )
// 				memcpy( tempList.name, pNpc->m_Property.m_szTitle, CONST_USERNAME );
// 			else
// 			{
// 				iter++;
// 				continue;
// 			}
// 
// 			npcList.push_back(tempList);
// 			//*/
//         }
// 
//         iter++;
//     }
}

void UpdateRegionAIRefCount(class CRegion *pRegion, int iValue)
{
    if (pRegion != NULL)
    {
        pRegion->m_dwAIRefCount += iValue;
    }
}

void CRegion::DestroyTrap() 
{
    if(m_pTrap) 
        delete[] m_pTrap; 
    
    m_pTrap = NULL; 
}

CTrigger * CRegion::MakeNewTrap(BYTE byMax)
{
    if (byMax <= m_byMaxTrap)
        return NULL;

    if (m_TrapList.size() > 0)
        return m_pTrap;

    CTrigger *pTemp = new CTrigger[m_byMaxTrap];
    memcpy(pTemp, m_pTrap, sizeof(CTrigger)*m_byMaxTrap);

    m_TrapList.resize((byMax - m_byMaxTrap) + 1, 0);
    m_TrapList[0] = m_byMaxTrap;

    DestroyTrap();

    m_byMaxTrap = byMax;
    m_pTrap = new CTrigger[m_byMaxTrap];

    memset(m_pTrap, 0, sizeof(CTrigger*) * m_byMaxTrap);

    memcpy(m_pTrap, pTemp, sizeof(CTrigger)*(m_TrapList[0]));

    delete[] pTemp;
    pTemp = NULL;

    return m_pTrap;
}

bool CRegion::SetTrap( WORD order, WORD x, WORD y, CTrigger *pTrap )
{
//     if ( order > 255 )
//     {
//         std::map< int, CTrigger >::iterator it = trapMap.find( order );
// 
//         // 先删除，可以导致Trigger释放，避免泄露
//         if ( it != trapMap.end() )
//         {
//             WORD PrevX = x, PrevY = y;
//             CTrigger &trap = it->second;
//             if ( trap.m_dwType == CTrigger::TT_MAPTRAP_EVENT || trap.m_dwType == CTrigger::TT_MAPTRAP_BUILDING )
//             {
//                 PrevX = trap.SMapEvent.wCurX, PrevY = trap.SMapEvent.wCurY;
//                 if ( ( PrevX >= m_RegionW ) || ( PrevY >= m_RegionH ) )
//                     return false;
//             }
// 
//             DWORD dwTile = GetTileMark( PrevX, PrevY );
//             if ( ( ( dwTile >> 8 ) & 0xffff ) == order )
//             {
//                 dwTile &= 0xFF0000FF;
//                 m_pdwTiles[ PrevX + PrevY*m_RegionW ] = dwTile;     
//                 trapMap.erase( order );
//             }
//         }
// 
//         if ( pTrap->m_dwType == 0 )
//             return true;
// 
//         DWORD dwTile = GetTileMark( x, y );
//         dwTile &= 0xFF0000FF;
//         dwTile |= ( ( DWORD )order ) << 8;
//         m_pdwTiles[ x + y*m_RegionW ] = dwTile;
// 
//         trapMap[ order ] = *pTrap;
//         return true;
//     }
// 
//     BYTE byOrder = 0xff & order;
// 
//     if (pTrap == NULL)
//         return false;
// 
//     if ( ( x >= m_RegionW ) || ( y >= m_RegionH ) )
//         return false;
// 
//     if (m_TrapList.size() == 0)
//         return false;
// 
//     if (((m_byMaxTrap - m_TrapList[0]) <= 0) ||
//         (byOrder == 0) ||
//         (m_byMaxTrap == 0))
//         return false;
// 
//     if(byOrder > (m_byMaxTrap - m_TrapList[0]))
//         return false;
//         
//     if(pTrap->m_dwType == 0)
//     {
//         // 这里还要tile属性修改
//         if ((m_TrapList[byOrder] == 0) || (m_TrapList[byOrder] > m_byMaxTrap))
//             return false;
// 
//         WORD PrevX = x, PrevY = y;
//         CTrigger &trap = m_pTrap[m_TrapList[byOrder] - 1];
//         if ( trap.m_dwType == CTrigger::TT_MAPTRAP_EVENT || trap.m_dwType == CTrigger::TT_MAPTRAP_BUILDING )
//         {
//             PrevX = trap.SMapEvent.wCurX, PrevY = trap.SMapEvent.wCurY;
//             if ( ( PrevX >= m_RegionW ) || ( PrevY >= m_RegionH ) )
//                 return false;
//         }
// 
//         DWORD dwTile = GetTileMark(PrevX, PrevY);
//         // 不能清除自己标记外的陷阱
//         if ( ( ( dwTile >> 8 ) & 0xffff ) != m_TrapList[byOrder] )
//             return false;
// 
//         dwTile &= 0xFF0000FF;
//         m_pdwTiles[PrevX + PrevY*m_RegionW] = dwTile;     
//         memset(&m_pTrap[m_TrapList[byOrder] - 1], 0, sizeof(CTrigger));
//         m_TrapList[byOrder] = 0;
//     }
//     else
//     {
//         BYTE id = (BYTE)(GetTileMark(x, y) >> 8) & 0xffff;
//         if ( id != 0 ) 
//             return false;
// 
//         m_TrapList[byOrder] = m_TrapList[0] + byOrder;
//         m_pTrap[m_TrapList[byOrder] - 1] = *pTrap;
//         // 这里还要tile属性修改
//         DWORD dwTile = GetTileMark(x, y);
//         dwTile &= 0xFFFF00FF;
//         dwTile |= m_TrapList[byOrder] << 8;
//         m_pdwTiles[x + y*m_RegionW] = dwTile;
//     }

    return true;
}

DWORD CRegion::CheckObjectByControlId(DWORD controlId)
{
    std::map< DWORD, DWORD >::iterator it = controlIdMap.find(controlId);
    if (it == controlIdMap.end())
        return 0;

    return it->second;
}

LPIObject CRegion::GetObjectByControlId( DWORD controlId )
{
    std::map< DWORD, DWORD >::iterator it = controlIdMap.find( controlId );
    if ( it == controlIdMap.end() )
        return LPIObject();

    return LocateObject( it->second );
}

BOOL CRegion::RemoveObjectByControlId(DWORD controlId)
{
	DWORD gid = 0;

	std::map<DWORD, DWORD>::iterator it = controlIdMap.find(controlId);
	if (it == controlIdMap.end())
		return FALSE;

	gid = it->second;

    if (0x20000000 & gid)
        return DelMonster(gid);

    if (0x10000000 & gid)
        return DelNpc(gid);

    if (0x40000000 & gid) 
        return DelBuilding(gid);

    return 0;
}

int CRegion::GetMonsterCount(bool CountDead)
{
	int Count = 0;
	std::map<unsigned __int32, LPIObject>::iterator iter = objectMap.begin();
	for (iter; iter != objectMap.end(); ++iter)
	{
		CMonster *monster = (CMonster*)iter->second->DynamicCast(IID_MONSTER);
		if (monster && monster->m_ParentRegion == this)
		{
			if (0 == monster->m_CurHp && !CountDead)
				continue;

			Count++;
		}
	}

	return Count;//怪物计数每次比场景配置文件的数量多，忽略了脚本创建的王级怪物。。不应该。。
}

int CRegion::GetNpcCount()
{
	int Count = 0;
	std::map<unsigned __int32, LPIObject>::iterator iter = objectMap.begin();
	for ( iter; iter != objectMap.end(); iter++)
	{
		CNpc *pNpc = (CNpc*)iter->second->DynamicCast(IID_NPC);
		if (pNpc && pNpc->m_ParentRegion == this)
			Count++;
	}
	return Count;
}

int CRegion::GetPlayerCount(int state)
{
	int Count = 0;
	check_list<LPIObject>::iterator iter = m_PlayerList.begin();

	for (iter; iter != m_PlayerList.end(); iter++)
	{
		CPlayer *p = ( CPlayer* )( *iter )->DynamicCast( IID_PLAYER );
		switch (state)
		{
		case 1:
			if (p && p->m_OnlineState == CPlayer::OST_NORMAL)
				Count++;
			break;
		case 2:
			if (p && p->m_OnlineState == CPlayer::OST_HANGUP)
				Count++;
			break;
		case 3:
			if (p && p->m_OnlineState == CPlayer::OST_LINKBREAK)
				Count++;
			break;
		default:
			if (p && p->m_OnlineState == CPlayer::OST_NORMAL)
				Count++;
			break;
		}
	}
	return Count;
}

/*
CBuilding* CRegion::CreateBuilding( PVOID pParameter, CPlayer *pPlayer, BOOL isGenerate, DWORD controlId )
{
    if ( pParameter == NULL ) return 0;

    if ( ( controlId != 0 ) && ( controlIdMap.find( controlId ) != controlIdMap.end() ) )
        return rfalse( 2, 1, "ctrlId 重复 Building 生成失败"), NULL;

    LPIObject pObject = GenerateObject( IID_BUILDING, 0, ( LPARAM )pParameter );
    CBuilding *pBuilding = ( CBuilding* )pObject->DynamicCast( IID_BUILDING );
    if ( pBuilding == NULL )
    {
        TraceInfo( "建筑失败.txt", "%s %s", _GetStringTime(), "Building 生成失败" );
        return rfalse(2, 1, "Building 生成失败"), NULL;
    }

    // 检测前的调用
    // 检测前的一个脚本触发
    const SBuildingConfigData *buidlingData = pBuilding->GetBuildingData( pBuilding->GetIndex() );
    if ( buidlingData && buidlingData->ctrlScript )
    {
        lite::Variant ret;
        // 作为帮派大门来说，就使用它来设置帮派参数！
        g_Script.SetCondition( NULL, pPlayer, NULL, pBuilding );
        LuaFunctor( g_Script, FormatString( "Building_CheckFront_%d", buidlingData->ctrlScript ) ) ( &ret );
        g_Script.CleanCondition();

 		try
		{
			if ( ret.dataType != lite::Variant::VT_EMPTY && ret.dataType != lite::Variant::VT_NULL )
				if ( ( __int32 )ret == -1 ) 
					return RemoveBuilding( pBuilding, FALSE ), NULL;
		}
		catch ( lite::Xcpt & )
		{
		}
    }

	if ( !pBuilding->CheckAreaBlock( pPlayer, this, isGenerate, isGenerate ) )
    {
        //TraceInfo( "建筑失败.txt", "%s %s", _GetStringTime(), "存在阻挡，设置失败" );
        TraceInfo( "建筑失败.txt", "%s 存在阻挡，设置失败 RegId:%d ID:%d UID:%d X:%d Y:%d", _GetStringTime(), 
           m_wRegionID, pBuilding->GetIndex(), pBuilding->GetUniqueId(), pBuilding->GetCurX(),pBuilding->GetCurY()  );
		return NULL;
    }

    if ( Move2Area( pObject, pBuilding->m_wCurX, pBuilding->m_wCurY, true ) == 0 )
    {
        //TraceInfo( "建筑失败.txt", "%s %s", _GetStringTime(), "无法将Building放入场景" );
        TraceInfo( "建筑失败.txt", "%s 无法将Building放入场景 RegId:%d ID:%d UID:%d X:%d Y:%d", _GetStringTime(),
            m_wRegionID, pBuilding->GetIndex(), pBuilding->GetUniqueId(), pBuilding->GetCurX(),pBuilding->GetCurY() );
        return rfalse(2, 1, "无法将Building放入场景"), NULL;
    }

	pBuilding->m_ParentRegion = this;

	int ret = pBuilding->AfterCreate( pPlayer, isGenerate ) ;
    if ( ( !ret ) || ( ret == -1 ) )
	{
		// 只有特定的情况才会删除掉建筑
		if ( ret == -1 )
			RemoveBuilding( pBuilding, FALSE );

        //TraceInfo( "建筑失败.txt", "%s %s", _GetStringTime(), "AfterCreate 生成失败" );
        TraceInfo( "建筑失败.txt", "%s AfterCreate 生成失败 RegId:%d ID:%d UID:%I64d X:%d Y:%d", _GetStringTime(), 
            m_wRegionID, pBuilding->GetIndex(), pBuilding->GetUniqueId(), pBuilding->GetCurX(),pBuilding->GetCurY() );
        return rfalse( 2, 1, "AfterCreate 生成失败"), NULL;
	}

    // 唯一ID是在创建建筑物的同时生成的，具有时间上的唯一性！
    QWORD uniqueId = pBuilding->GetUniqueId();

    if ( m_BuildingMap.find( uniqueId ) != m_BuildingMap.end() )
	{
		RemoveBuilding( pBuilding );
        TraceInfo( "建筑失败.txt", "%s 严重的错误! 建筑物的唯一编号重复!  UID:%d", _GetStringTime() , uniqueId );
        return rfalse( 2, 1, "严重的错误! 建筑物的唯一编号重复!"), NULL;
	}

    m_BuildingMap[ uniqueId ] = pObject;

    if ( pBuilding->controlId = controlId )
        controlIdMap[ controlId ] = pObject->GetGID();

    return pBuilding;
}

int CRegion::DelBuilding( DWORD dwGID )
{
    LPIObject pObject = LocateObject( dwGID );
    CBuilding *pBuilding = ( CBuilding* )pObject->DynamicCast( IID_BUILDING );

    if ( pBuilding == NULL ) {
        rfalse( 2, 1, "Building 没找到，可能已经删除了" );
        return 0;
    }

    return RemoveBuilding( pBuilding->GetUniqueId() );
}

extern BYTE hugeBuffer[ 0xffff ];
extern BYTE commBuffer[ 0xfff ];

BOOL CRegion::SaveBuildings()
{
    // 如果 prevBuildingNumber 小于 0， 则说明该场景根本没有收到数据库的初始化确认消息！！！ 
    // 如果强制保存，则可能将数据库中一些本来正常的数据销毁掉！ 
    if ( prevBuildingNumber < 0 )
        return TraceInfo( "建筑失败.txt", "%s %s RegId:%d", _GetStringTime(), "本次未做保存prevBuildingNumber=-1", m_wRegionID ), FALSE;

	//当前场景没有动态建筑物、并且上次保存后也没有建筑物时才不做保存
	if ( ( prevBuildingNumber == 0 ) && ( m_BuildingMap.size() == 0 ) )
        return TRUE;

	prevBuildingNumber = (int)m_BuildingMap.size();

#undef new 
	SQBuildingSaveMsg &msg = * new( hugeBuffer ) SQBuildingSaveMsg;
	msg.dwServerID = 0;
	msg.wParentID = m_wRegionID;
	msg.wNumbers = 0;

	lite::Serializer slm( msg.Buffers, sizeof( hugeBuffer ) - sizeof( msg ) );

	for ( std::map< QWORD, LPIObject >::iterator it = m_BuildingMap.begin(); it != m_BuildingMap.end(); ++it )
	{
        CBuilding *pBuilding = ( CBuilding* )it->second->DynamicCast( IID_BUILDING );
		if ( pBuilding == NULL )
        {
            //return rfalse( 2, 1, "Building IS Invalid" ), NULL;
            // 如果中间某个意外出错，导致后面正常建筑没保存 
            rfalse( 2, 1, "Building IS Invalid" );
            TraceInfo( "建筑失败.txt", "%s %s RegId:%d", _GetStringTime(), "Building IS Invalid", m_wRegionID ); 
            continue;
        }
		
		int retsize = sizeof( commBuffer );
		if ( retsize = (int)pBuilding->OnSave( commBuffer, retsize ) )
		{
			msg.wNumbers ++;
			slm( commBuffer, retsize );
		}
	}

    msg.wBufSize = WORD(sizeof( hugeBuffer ) - slm.EndEdition());
	return SendToLoginServer( &msg, msg.wBufSize );
}

BOOL CRegion::LoadBuildings( struct SABuildingGetMsg *pMsg )
{
    if ( pMsg->retMsg == SABuildingGetMsg::RET_SUCCESS )
        prevBuildingNumber = 0;

    try
    {
        // 这个是特殊处理，属于没有数据！
        if ( ( ( LPWORD )pMsg->Buffers )[ 0 ] == 4 )
        {
            // TraceInfo( "建筑失败.txt", "%s %s", _GetStringTime(), "没有数据" ); 
            return true;
        }

        lite::Serialreader slr( pMsg->Buffers );

        while ( slr.curSize() < slr.maxSize() )
        {
            // 按保存的顺序进行反序列化
            lite::Variant lvt = slr();
            if ( lvt.dataType != lite::Variant::VT_POINTER )
            {
                TraceInfo( "建筑失败.txt", "%s %s", _GetStringTime(), "解码时结果不是lite::Variant::VT_POINTER" ); 
                continue;
            }

            CBuilding::CreateParameter param;
            param.loadData = lvt._pointer;
            CreateBuilding( &param, NULL );
            prevBuildingNumber ++;
        }
    }
    catch ( lite::Xcpt & )
    {
        TraceInfo( "建筑失败.txt", "%s %s", _GetStringTime(), "反序列化异常!" ); 
    }

    return true;
}

BOOL CRegion::RemoveBuilding( CBuilding *pBuilding, BOOL isDfRemove )
{
	if ( pBuilding == NULL )
		return FALSE;

    if ( pBuilding->m_ParentRegion != this )
        return 0;

	m_BuildingMap.erase( pBuilding->GetUniqueId() );

	if ( isDfRemove )
		pBuilding->DeforeRemove();

    if ( pBuilding->controlId )
        controlIdMap.erase( pBuilding->controlId );

    if ( pBuilding->m_ParentArea )
    {
        SADelObjectMsg msg;
        msg.dwGlobalID = pBuilding->GetGID();
        pBuilding->m_ParentArea->SendAdj( &msg, sizeof( msg ), -1 );
        pBuilding->m_ParentArea->DelObject( pBuilding );
    }

	pBuilding->isValid() = false;

	return TRUE;
}

BOOL CRegion::RemoveBuilding( QWORD uniqueId )
{
    std::map< QWORD, LPIObject >::iterator it = m_BuildingMap.find( uniqueId );
    if ( it == m_BuildingMap.end() )
        return FALSE;

    LPIObject pObject = it->second;
    CBuilding *pBuilding = ( CBuilding* )pObject->DynamicCast( IID_BUILDING );
    if ( pBuilding == NULL ) {
        rfalse( 2, 1, "CBuilding( UID ) 没找到，可能已经删除了" );
        return 0;
    }

	return RemoveBuilding( pBuilding );
}
*/

CBuilding* CRegion::CreateBuilding( PVOID pParameter, CPlayer *pPlayer, BOOL isGenerate, DWORD controlId )
{
    return NULL;
}

int CRegion::DelBuilding( DWORD dwGID )
{
    return false;
}

BOOL CRegion::SaveBuildings()
{
    return false;
}

BOOL CRegion::LoadBuildings( struct SABuildingGetMsg *pMsg )
{
    return false;
}

BOOL CRegion::RemoveBuilding( CBuilding *pBuilding, BOOL isDfRemove )
{
    return false;
}

BOOL CRegion::RemoveBuilding( QWORD uniqueId )
{
	return false;
}

void  CRegion::SetRegionBRType( DWORD dwBRTeamNumber, WORD wRefreshStyle )
{
	/*
    if ( dwBRTeamNumber == 0 )
        return;

    std::map<unsigned __int32, LPIObject>::iterator iter = objectMap.begin();
    for ( iter; iter != objectMap.end(); iter++)
    {
        CMonster *monster = (CMonster*)iter->second->DynamicCast( IID_MONSTER );
        if (monster && monster->m_ParentRegion == this && monster->m_Property.m_dwBRTeamNumber == dwBRTeamNumber )
        {
			monster->m_Property.m_wRefreshStyle = wRefreshStyle;
        }
    }//*/
}
void CRegion::ClearVectorByType(WORD type)
{
	switch (type)
	{
	case LimitItem:
		m_vLimitItemID.clear();
		break;
	case UnDealItem:
		m_vUnDealItemID.clear();
		break;
	case LimitSkill:
		m_vLimitSkillID.clear();
		break;
	case TrigerBuffMonster:
		m_vTrigerBuffMonster.clear();
		break;
	case TrigerBuffPlayer:
		m_vTrigerBuffPlayer.clear();
		break;
	default:
		break;
	}
}
void CRegion::SetParamToList( WORD type, char* str)
{
	ClearVectorByType(type);	
	std::string tmpStr = str;
	WORD lastPos = 0;
	WORD len =  (WORD)tmpStr.size();
	for (WORD i = 0;i <= len;i++)
	{
		if (' ' == str[i] ||
			'\0' == str[i]||
			',' == str[i]||
			'，' == str[i])
		{
			tmpStr[i - lastPos] = '\0';
			const char *ss = tmpStr.c_str();
			if(tmpStr[0] != '\0')
				PushItemInVevtor(type,ss);

			lastPos = i + 1;
		}
		else
		{
			tmpStr[i - lastPos] = str[i];
		}
	} 
}

void CRegion::PushItemInVevtor( WORD type,const char* str )
{
	int id = atoi(str);
	switch (type)
	{
	case LimitItem:
		m_vLimitItemID.push_back((WORD)id);	
		break;
	case UnDealItem:
		m_vUnDealItemID.push_back((WORD)id);	
		break;
	case LimitSkill:
		m_vLimitSkillID.push_back((WORD)id);	
		break;
	case TrigerBuffMonster:
		m_vTrigerBuffMonster.push_back((BYTE)id);
		break;
	case TrigerBuffPlayer:
		m_vTrigerBuffPlayer.push_back((BYTE)id);
		break;
	default:
		break;
	}
}

bool CRegion::CanUseItem( WORD id )
{
	WORD len = (WORD)m_vLimitItemID.size();
	for (WORD i = 0;i < len;++i)
	{
		if (m_vLimitItemID[i] == id)
		{
			return false;
		}
	}
	return true;
}

bool CRegion::CanDealItem( WORD id )
{
	WORD len = (WORD)m_vUnDealItemID.size();
	for (WORD i = 0;i < len;++i)
	{
		if (m_vUnDealItemID[i] == id)
		{
			return false;
		}
	}
	return true;
}

bool CRegion::CanUseSkill( WORD id )
{
	WORD len = (WORD)m_vLimitSkillID.size();
	for (WORD i = 0;i < len;++i)
	{
		if (m_vLimitSkillID[i] == id)
		{
			return false;
		}
	}
	return true;
}

LPIObject CRegion::GetObjectInRegin( DWORD dwGID )
{
	//check_list<LPIObject>::iterator iter=m_PlayerList.begin();
	return g_SearchInObjectList(m_PlayerList,dwGID);
}

int CRegion::LoadRegincollision( WORD ID )
{
#ifdef USEMAPCOLLISION
	char buffer[128];
	sprintf(buffer, "Map\\collision_%d.map", ID);

	if (!m_pmapsever)
	{
		m_pmapsever=new MapServer;
		return m_pmapsever->Open(buffer);
	}
#endif
	return 0;
}

bool CRegion::IsCanMove( float x,float y )
{
#ifdef USEMAPCOLLISION
	if (m_pmapsever&&m_bUseCollision)
	{
		const DWORD dwGridSize = m_pmapsever->GetGridSize();	
		if (dwGridSize != INVALID_SIZE)
		{
			DWORD dx=x/dwGridSize;
			DWORD dy=y/dwGridSize;
			return (m_pmapsever->GetStatus(dx,dy)&ET_MOVE);
		}
	}
#endif
	return true;
}

void CRegion::KillMonster(CPlayer *player)
{
	std::map<unsigned __int32, LPIObject>::iterator iter = objectMap.begin();
	for (iter; iter != objectMap.end(); ++iter)
	{
		CMonster *monster = (CMonster*)iter->second->DynamicCast(IID_MONSTER);
		if (monster && monster->m_ParentRegion == this)
		{
			if (monster->m_CurHp>0)
			{
				monster->ModifyCurrentHP(-(monster->m_CurHp), 0, player);
			}
		}
	}
}

int CRegion::GetLiveMonster( WORD monsterID )
{
	int Count = 0;
	std::map<unsigned __int32, LPIObject>::iterator iter = objectMap.begin();
	for (iter; iter != objectMap.end(); ++iter)
	{
		CMonster *monster = (CMonster*)iter->second->DynamicCast(IID_MONSTER);
		if (monster && monster->m_ParentRegion == this)
		{
			if (0 != monster->m_CurHp)
				Count++;
		}
	}
	return Count;//怪物计数每次比场景配置文件的数量多，忽略了脚本创建的王级怪物。。不应该。。
}
