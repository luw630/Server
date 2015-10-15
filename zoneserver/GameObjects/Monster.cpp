#include "StdAfx.h"
#include <map>
#include "monster.h"
#include "gameworld.h"
#include "area.h"
#include "item.h"
#include "SingleItem.h"
#include "region.h"
#include "Player.h"
#include "Trigger.h"
#include "ScriptManager.h"
extern CScriptManager g_Script;

#include "TeamManager.h"
#include "networkmodule/movemsgs.h"
#include "networkmodule/regionmsgs.h"
#include "networkmodule/upgrademsgs.h"
#include "NetWorkModule/TeamMsgs.h"
#include "区域服务器/Environment.h"
#include "DSpcialItemData.h"
#include "BuffBaseData.h"
#include "Random.h"
#include "CMonsterService.h"
#include "CItemDropService.h"
#include "CItemDefine.h"
#include "CItemService.h"
#include "CMonsterDefine.h"
#include "Pet.h"
#include "DynamicRegion.h"
#include "DynamicDailyArea.h"
#include "CSkillService.h"

static long s_lMonsterSize = 0;                   // 怪物基本数据列表大小

static SMonsterSkill m_SMSData[MAX_MSKILLCOUNT];  // 怪物技能数据

DWORD CMonster::monsterDeadMoney = 0;

int MutateRank[10] = {18182, 36364, 54545, 72727, 90909, 109091, 127273, 145455, 163636, 181818};

extern LPIObject GetPlayerBySID(DWORD dwStaticID);
extern LPIObject GetPlayerByGID(DWORD);
extern LPIObject GetObjectByGID(DWORD GID);
extern float GetDirection(float x1, float y1, float x2, float y2);

// 鉴定的四个等级属性数组
int  g_LevelAttributesArray[ 4 ][ 3 ];
int LoadLevelAttributeArray( void )
{
    memset( g_LevelAttributesArray, -1, sizeof( g_LevelAttributesArray ) );
    dwt::ifstream file( "data\\LevelAttributesList.txt" );
    int AttrIndex = 0;
    if ( file.is_open() )
    {
        int i = 0, j = 0;
        for ( i = 0; i < 4; ++i )
        {
            for ( j = 0; j < 3; ++j )
            {
                if ( file.eof() )
                    break;

                file >> AttrIndex;
                if ( AttrIndex < 0 || AttrIndex >= 57 )
                {
                    file.close();
                    return rfalse( 2, 1, "读取鉴定等级属性列表文件错误！" );
                }
                g_LevelAttributesArray[ i ][ j ] = AttrIndex;
            }
        }

        if ( i != 4 || j != 3 )
        {
            file.close();
            return rfalse( 2, 1, "读取鉴定等级属性列表文件错误！" );
        }
    }
    else
        return rfalse( 2, 1, "无法打开鉴定等级属性列表文件！" );
    
    file.close();
    return 1;
}
//-------------------------------------------------------------------------------------------------------------------------------------
//侠义世界
//--------------------------------------------------------------------------------------------------------------------------------------

//载入随机属性个数几率表
int CMonster::LoadEquipDropAttNum()
{
	dwt::ifstream stream("Data\\item\\dropEquipTable\\属性个数几率表.txt");
	if (!stream.is_open())
		return rfalse(0, 0, "无法打开[Data\\item\\dropEquipTable\\属性个数几率表.txt]文件！");

	char linemax[2048],tmp[32];
	int maxid = 0;
	while ( !stream.eof() ) {
		stream.getline( linemax, sizeof( linemax ) );
		if ( linemax[0] == 0 || ( *(LPWORD)linemax ) == '//'  )
			continue;

		std::strstream linebuf( linemax, (std::streamsize)strlen( linemax ) );

		linebuf >> tmp;
		int i = 0,tmpMem[512],MaxNum = 0;
		memset(&tmpMem,0,sizeof(tmpMem));
		while ( !linebuf.eof())
		{
			linebuf >> tmpMem[i++];
		}
		g_wAttNumOdds = new WORD[i];

		i = 0;
		while ( tmpMem[i] != 0)
		{
			g_wAttNumOdds[i] = tmpMem[i++];
		}	
	}
	stream.close();
	return 1;
}

//载入随机属性类型几率表
int CMonster::LoadEquipDropAttType()
{
	dwt::ifstream stream("Data\\item\\dropEquipTable\\属性出现几率表.txt");
	if (!stream.is_open())
		return rfalse(0, 0, "无法打开[Data\\item\\dropEquipTable\\属性出现几率表.txt]文件！");

	char linemax[2048];
	int maxid = 0,i = 0;

	g_sAttTypeOdds = new SEquipAttTypeOddsData[Max_Att_Num];

	memset(g_sAttTypeOdds, 0, sizeof(SEquipAttTypeOddsData) * Max_Att_Num);

	while ( !stream.eof() ) {
		stream.getline( linemax, sizeof( linemax ) );
		if ( linemax[0] == 0 || ( *(LPWORD)linemax ) == '//'  )
			continue;

		std::strstream linebuf( linemax, (std::streamsize)strlen( linemax ) );

		linebuf >> g_sAttTypeOdds[i].name;
		linebuf >>  g_sAttTypeOdds[i++].OddsValue;
	}
	stream.close();
	return 1;
}

//载入装备每种属性取每个值的几率
int CMonster::LoadEquipDropAttVale()
{
	dwt::ifstream stream("Data\\item\\dropEquipTable\\随机属性单独属性值.txt");
	if (!stream.is_open())
		return rfalse(0, 0, "无法打开[Data\\item\\dropEquipTable\\随机属性单独属性值.txt]文件！");

	g_sAttValueOdds = new SEquipAttValueOddsData[Max_Att_Num];
	memset(g_sAttValueOdds, 0, sizeof(SEquipAttValueOddsData) * Max_Att_Num);

	char linemax[2048];
	int maxid = 0,i = 0;
	while ( !stream.eof() ) {
		stream.getline( linemax, sizeof( linemax ) );
		if ( linemax[0] == 0 || ( *(LPWORD)linemax ) == '//'  )
			continue;

		std::strstream linebuf( linemax, (std::streamsize)strlen( linemax ) );

		int id = 0;
		linebuf >> g_sAttValueOdds[i].name;
		while(!linebuf.eof())
		{
			linebuf >>  g_sAttValueOdds[i].OddsValue[id++];
		}
		i++;
	}
	stream.close();
	return 1;
}

//载入装备属性条数规则数据
int CMonster::LoadEquipDropAttRule()
{
	dwt::ifstream stream("Data\\item\\dropEquipTable\\装备属性条数规则表.txt");
	if (!stream.is_open())
		return rfalse(0, 0, "无法打开[Data\\item\\dropEquipTable\\装备属性条数规则表.txt]文件！");

	g_sAttNumRule = new SEquipAttNumRuleData[Max_Att_Num_Rule];
	memset(g_sAttNumRule, 0, sizeof(SEquipAttNumRuleData) * Max_Att_Num_Rule);

	char linemax[2048];
	int tmp = 0,i = 0;
	while ( !stream.eof() ) {
		stream.getline( linemax, sizeof( linemax ) );
		if ( linemax[0] == 0 || ( *(LPWORD)linemax ) == '//'  )
			continue;

		std::strstream linebuf( linemax, (std::streamsize)strlen( linemax ) );

		linebuf >> tmp >> g_sAttNumRule[i].levle >> g_sAttNumRule[i++].number;	
	}
	stream.close();
	return 1;
}

//载入侠义世界全局道具掉落表
int CMonster::LoadItemDropTable()
{
	//memset(s_pDropTable, 0, sizeof(SDropTable) * 256);

	static WORD itemId = 0;
	static char str[128];
	g_vItemDropIdTable.clear();
	dwt::ifstream stream("Data\\item\\xysjDropItemTable\\掉落配置表.def");
	if (!stream.is_open())
	{
		return rfalse(0, 0, "无法打开[Data\\item\\xysjDropItemTable\\掉落配置表.def]道具掉落数据列表文件！");
	}
	
	while (!stream.eof())
	{	
		ITEMDROPTABLE curDropTable;
		SItemDropTableData curDataStruct;

		curDropTable.clear();
		std::string fname( "Data\\item\\xysjDropItemTable\\" );
		stream >> str;
		fname += str;
		dwt::ifstream tfile( fname.c_str() );
		if (!tfile.is_open())
		{
			rfalse(0, 0, "无法打开[%s]道具掉落数据列表文件！", fname.c_str());
		}
		else
		{
			int i = 0;
			while (!tfile.eof())
			{
				char tmp[1024];
				SItemOddsTabledata curTable;
				tfile.getline(tmp,sizeof(tmp));
				std::strstream lineStream( tmp, ( int )strlen( tmp ), ios_base::in );

				lineStream >> curTable.itemId >> curTable.wOdds;
				curDropTable.push_back(curTable);	

				//查看数据正确性写的
				itemId = (WORD)curDropTable.size();
				itemId = curDropTable[i++].itemId;
			}
			
			tfile.close();
		}
		char tmp[64] = "";
		_snprintf(tmp,5,str);
		curDataStruct.itemId	  = atoi(tmp);
		curDataStruct.itemIdTable = curDropTable;
		g_vItemDropIdTable.push_back(curDataStruct);
	}

	stream.close();

	return 1;
}

//载入侠义世界怪物掉落任务物品表
int CMonster::LoadTaskDropTable()
{

	static WORD itemId = 0;
	static char str[128];
	g_vTaskDropIdTable.clear();
	dwt::ifstream stream("Data\\item\\xysjTaskDropTable\\掉落配置表.def");
	if (!stream.is_open())
	{
		return rfalse(0, 0, "无法打开[Data\\item\\xysjTaskDropTable\\掉落配置表.def]道具掉落数据列表文件！");
	}

	while (!stream.eof())
	{	
		ITEMDROPTABLE curDropTable;
		SItemDropTableData curDataStruct;

		curDropTable.clear();
		std::string fname( "Data\\item\\xysjTaskDropTable\\" );
		stream >> str;
		fname += str;
		dwt::ifstream tfile( fname.c_str() );
		if (!tfile.is_open())
		{
			rfalse(0, 0, "无法打开[%s]道具掉落数据列表文件！", fname.c_str());
		}
		else
		{
			int i = 0;
			while (!tfile.eof())
			{
				char tmp[1024];
				SItemOddsTabledata curTable;
				tfile.getline(tmp,sizeof(tmp));
				std::strstream lineStream( tmp, ( int )strlen( tmp ), ios_base::in );

				lineStream >> curTable.itemId >> curTable.wOdds;
				curDropTable.push_back(curTable);	

				//查看数据正确性写的
				itemId = (WORD)curDropTable.size();
				itemId = curDropTable[i++].itemId;
			}

			tfile.close();
		}
		char tmp[64] = "";
		_snprintf(tmp,5,str);
		curDataStruct.itemId	  = atoi(tmp);
		curDataStruct.itemIdTable = curDropTable;
		g_vTaskDropIdTable.push_back(curDataStruct);
	}

	stream.close();

	return 1;
}

int CMonster::LoadEquipDropAttributeTable()
{
	//m_wAttNumOdds
	//m_wAttTypeOdds
	//m_wAttValueOdds

	//载入随机属性个数几率表
	LoadEquipDropAttNum();

	//载入随机属性类型几率表
	LoadEquipDropAttType();

	//载入装备每种属性取每个值的几率
	LoadEquipDropAttVale();

	//载入装备属性条数规则数据
	LoadEquipDropAttRule();

	//载入侠义世界全局道具掉落表
	LoadItemDropTable();

	//载入侠义世界掉落任务物品表
	LoadTaskDropTable();
	return 1;
}
//----------------------------------------------------------------------------------------------------------	
int CMonster::LoadMonsterData()
{
//     // 借用地方调用哈
//     void LoadDropItemData();
//     LoadDropItemData();
// 
// 	//加载掉落装备属性相关配置表
// 	LoadEquipDropAttributeTable();
// 
// 	//
// 	static char str[128];
// 	dwt::ifstream stream("Data\\怪物配置表.txt");
// 	if (!stream.is_open())
// 		return rfalse(0, 0, "无法打开[Data\\怪物配置表.txt]怪物数据列表文件！");
// 
//     // 新的怪物表
//     // 编号 名称 图像档名 等级 是否是BOSS 
//     // 生命值 外功基础 内功基础 外防基础 内防基础 
//     // 特殊攻击力（无视防御） 武功倍数 武功使用几率 身法 攻击间隔[攻击速度] 命中时间 攻击范围 
//     // 变身外功色怪几率 变身内功色怪几率 变身外防色怪比率 变身内防色怪几率 变身“金钱色怪”几率 
//     // 携带经验值 携带金钱 携带侠义值 携带物品1列表 物品1掉落比率 物品1掉落最大个数 携带物品列表n 物品掉落比率n 物品n掉落最大个数...
// 
//     std::list< SMonsterData > lst;
//     char linemax[2048];
//     int maxid = 0;
//     while ( !stream.eof() ) {
//         stream.getline( linemax, sizeof( linemax ) );
//         if ( linemax[0] == 0 || ( *(LPWORD)linemax ) == '//' || ( *(LPWORD)linemax ) == '--' )
//             continue;
// 
//         std::strstream linebuf( linemax, (std::streamsize)strlen( linemax ) );
//         SMonsterData d;
// 		memset(&d, 0, sizeof(SMonsterData));
//         linebuf >> d.id >> str >> d.image >> d.iHeadImage >> d.level >> d.boss;
//         linebuf >> d.hp >> d.outAtkBase >> d.innerAtkBase >> d.outDefBase >> d.innerDefBase;
// 		linebuf >> d.outAtkChance >> d.innerAtkChance >> d.avgAtkChance;
//         linebuf >> d.specAtk >> d.bWeaponOdds>> d.specPro>> d.bArmourOdds>>d.skillScale >> d.skillRate >> d.agility >> d.hitSpeed >> d.hitTick >> d.hitArea;
//         linebuf >> d.ChangeRate;
//         linebuf >> d.exp >> d.money >> d.xydvalue;
// 		for (int i = 0;i < 5;++i)
// 		{
// 			linebuf >> d.skillInfo[i][0] >> d.skillInfo[i][1];
// 		}
// 		for (int i = 0;i < 8;++i)
// 		{
// 			linebuf >> d.moveBuffInfo[i][0] >> d.moveBuffInfo[i][1];
// 		}
// 		linebuf >> d.scriptId >> d.dropMaxNum >> d.dropTaskTable;
// 
//         strncpy( d.name, str, sizeof( d.name ) );
//         d.name[ sizeof( d.name ) - 1 ] = 0;
//         int c = 0;
//         while ( !linebuf.eof() && c < ( sizeof(d.dropTable) / sizeof(d.dropTable[0]) ) ) {
//             linebuf >> d.dropTable[c][0];
//             linebuf >> d.dropTable[c][1];
//             if ( d.dropTable[c][1] == 0 )
//                 break;
//             c++;
//         }
// 		linebuf >> d.extraScriptId;
// 
//         if ( d.id > maxid )
//             maxid = d.id;
//         lst.push_back( d );
//     }
// 
//     if ( maxid > (int)lst.size() * 2 )
//         return rfalse(0, 0, "怪物列表数据有误！ID[%d] > MAXIDSIZE[%d]", maxid, lst.size() * 2);
// 
//     s_lMonsterSize = maxid+1;
// 	s_pMonsterDatas = new SMonsterData[s_lMonsterSize];
//     memset(s_pMonsterDatas, 0, sizeof(SMonsterData) * s_lMonsterSize);
//     for ( std::list< SMonsterData >::iterator it = lst.begin(); it != lst.end(); it++ )
//         s_pMonsterDatas[it->id] = *it;
// 
// 	stream.close();

	return 1;
}

//----------------------------------------------------------------------------------------------------------	
// SMonsterData *CMonster::GetMonsterData(int index)
// {
// 	if (index >= s_lMonsterSize) 
//         return NULL;
// 
// 	return &s_pMonsterDatas[index];
// }

//----------------------------------------------------------------------------------------------------------	
void CMonster::ClearMonsterData()
{
//	if (s_pMonsterDatas != NULL) 
//		delete s_pMonsterDatas;

    s_lMonsterSize = 0;
//    s_pMonsterDatas = NULL;

	if(g_wAttNumOdds != NULL)
		delete []g_wAttNumOdds;
	g_wAttNumOdds = NULL;

	if(g_sAttTypeOdds != NULL)
		delete []g_sAttTypeOdds;
	g_sAttTypeOdds = NULL;

	if(g_sAttValueOdds != NULL)
		delete []g_sAttValueOdds;
	g_sAttValueOdds = NULL;

	if(g_sAttNumRule != NULL)
		delete []g_sAttNumRule;
	g_sAttNumRule = NULL;
}

//----------------------------------------------------------------------------------------------------------	
void CMonster::LoadMonsterSkillData()   
{
    int         i, iDataCount;
    dwt::ifstream    FileStream( FILENAME_MSKILL );
    char        szBuf[256];

    if( !FileStream.is_open() )
    {
        rfalse( "无法打开[%s]内功数据列表文件！", FILENAME_MSKILL );
        return;
    }

    while (!FileStream.eof())
    {
        FileStream >> szBuf;
        if (strcmp(szBuf, "·") == 0)
			break;
	}
    i = 0;
    iDataCount = MAX_MSKILLCOUNT;
    char tmpStr[1024];
    while( !FileStream.eof() )
    {
        FileStream.getline( tmpStr, sizeof( tmpStr ) );
        if ( tmpStr[0] == 0 )
            continue;

        std::strstream FileStream( tmpStr, (std::streamsize)strlen( tmpStr ) );

        DWORD dwTemp = 0;
        char nameTemp[16] = "";

        m_SMSData[i].type = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // 技能类型	

        FileStream >> nameTemp;
        if ( nameTemp[0] == 0 )
            continue;
        strncpy( m_SMSData[i].szName, nameTemp, 15 );               // 名称[未用]
            m_SMSData[i].szName[15] = '\0';

        m_SMSData[i].effectId        = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // 对应动画
        m_SMSData[i].rating          = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // 以1/10000为小数单位,计算出现概率               
        m_SMSData[i].vitalsodds      = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // 层数，这个数值会被玩家心法抵抗(影响命中概率)   
        m_SMSData[i].effectTime      = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // 持续时间                       
        m_SMSData[i].waiting         = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // 从发动到命中的等待时间
                                                                                              
        m_SMSData[i].wRedDefRate     = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // 降低防御力比率p                       
        m_SMSData[i].wRedAgiRate     = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // 降低灵敏度比率p                           
        m_SMSData[i].wToxicRedHPRate = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // 中毒损生命比率p             
        m_SMSData[i].wDemToHPRate    = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // 吸收伤害转生命比率   
        m_SMSData[i].dwDamageArea    = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // 群伤的攻击范围
        m_SMSData[i].dwDamageTotal   = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // 总伤害

        m_SMSData[i].dwSkillTimes = -1;  // 初始状态
        i ++;
        if( i>=iDataCount ) break;
    } 
    FileStream.close();
}

//----------------------------------------------------------------------------------------------------------	
SMonsterSkill *CMonster::GetMonsterSkillData(int SkillType)
{
    if( SkillType < 1 || SkillType > MAX_MSKILLCOUNT )   return  NULL;

    return  &m_SMSData[ SkillType - 1 ]; 
}

int CMonster::OnCreate(_W64 long pParameter)
{
	SParameter *pParam = (SParameter*)pParameter;

	if (!pParam)
		return 0;

    memset(&m_Property, 0, sizeof(SMonsterProperty));
	memset(m_Name, 0, CONST_USERNAME);

	m_Property.m_BirthPosX = pParam->wX;
	m_Property.m_BirthPosY = pParam->wY;

	// 获取基本数据
	const SMonsterBaseData *monster = CMonsterService::GetInstance().GetMonsterBaseData(pParam->wListID);
	if (!monster)
		return 0;

	SMonsterBaseData *temp = &m_Property;
	memcpy(temp, monster, sizeof(SMonsterBaseData));
	if (pParam->PartolRadius == 0)
	{
		
	}
	else
	{
		m_Property.m_PartolRadius = pParam->PartolRadius;
	}
	if (pParam->dwRefreshTime)
		m_Property.m_ReBirthInterval = pParam->dwRefreshTime;

	m_Property.controlId = 0;
	m_wCuruseskill = 0;
	//AI怪物这两个变量有效
	memcpy(&m_AIParameter, pParam, sizeof(SParameter));
	memset(&m_AIProperty, 0, sizeof(SMonsterAIProperty));

	// 侠义道3移动
	m_curX		= ((DWORD)m_Property.m_BirthPosX) << TILE_BITW;
	m_curY		= ((DWORD)m_Property.m_BirthPosY) << TILE_BITH;
	m_curZ		= 0.0f;
	m_Direction = m_Property.m_Direction;
	m_curTileX	= m_Property.m_BirthPosX;
	m_curTileY	= m_Property.m_BirthPosY;
	m_BaseSpeed	= monster->m_WalkSpeed;
	m_OriSpeed	= monster->m_WalkSpeed;
	m_Speed = (float)monster->m_WalkSpeed/300;
	if (m_Speed < 0.1f)
	{
		m_Speed += 0.1f;
	}
	

	m_MaxHp		= m_Property.m_MaxHP;
	m_MaxMp		= m_Property.m_MaxMP;
	m_Level		= m_Property.m_level;
	m_CurHp		= m_MaxHp;
	m_CurMp		= m_MaxMp;
	
	m_GongJi	= m_Property.m_GongJi;
	m_FangYu	= m_Property.m_FangYu;
	m_BaoJi		= m_Property.m_BaoJi;
	m_ShanBi	= m_Property.m_ShanBi;
	m_AtkSpeed	= m_Property.m_AtkSpeed;
	m_StaticpreUpdateTime = 0;

	//月夜新增属性初始化
	m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT] = m_Property.m_uncrit;
	m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT] = m_Property.m_wreck;
	m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT] = m_Property.m_unwreck;
	m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT] = m_Property.m_puncture;
	m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT] = m_Property.m_unpuncture;
	m_Hit = m_Property.m_Hit;
// 	m_MaxHp	= m_Property.m_MaxHP;
// 	m_CurHp = m_MaxHp;
// 	m_MaxMp = m_Property.m_MaxMP;
// 	m_CurMp = m_MaxMp;
// 	m_Level	= m_Property.m_level;
// 	

// 
// 	m_PhyEscapeFix	= 0;
// 	m_PhyHitFix		= 0;
// 	m_PhyCriHitFix	= 0;
// 	m_PhyDamageFix	= 0;
// 	m_PhyDefenceFix	= 0;
// 	m_FpEscapeFix	= 0;
// 	m_FpHitFix		= 0;
// 	m_FpCriHitFix	= 0;
// 	m_FpDamageFix	= 0;
// 	m_FpDefenceFix	= 0;

	// 刚开始怪物的AI为站立，所以初始化站立的时间
	m_ActionFrame		= 5/*CRandom::RandRange(100, 150)*/;
	m_SearchTargetTime	= timeGetTime();
	m_BackProtection	= false;

    m_bStopToStand = FALSE;

    m_dwEndAttackTime = 0;

    m_bDeadEvent = 0;
    m_dwDeadDelayScript = 0;
    m_dwKillerPlayerGID = 0;
    m_byMutateState = 0;			
    m_wDeadTimes = 0;
    targetDamage = 0;
    targetSegTime = 0;
	m_nlastdropTime=0;

    bySelectType = 0;        
    byPriorityExceptType = 0;
    dwPriorityTarget = 0xffffffff;  

    m_dwTelesportTime = 0;
	
	///用于怪物AI时间控制
	m_nAttackTime=0;
	m_nRandomSkillIndex=0;
	m_nMoveStartTime = 0;
	///记录上一次发送的目标点信息
	m_vdesPos = new D3DXVECTOR2;

	///被地图碰撞阻挡计时
	m_nCollisionTime = 0;
	// 初始化技能ID
	int i=0;
	while(i<MAX_MONTER_SKILLCOUNT)
	{
		m_pSkills[i].wTypeID		= monster->m_SkillID[i];
		m_pSkills[i].byLevel		= 1;
		m_pSkills[i].dwProficiency	= 0;
		i++;
	}

	m_gmakeparm1 = 1;
	m_gmakeparm2 = 1;
	m_gmakeparm3 = 1;
	m_gmakeparm4 = 1;
	m_CopySceneGID = 0;

	m_dCreatePlayerID = 0; //怪物的创建者
	m_dCreatePlayerID = pParam->wCreatePlayerID;
	m_dRankIndex = 0;
	m_dRankIndex = pParam->dRankIndex;
	m_ShowState = 0;
	strcpy(m_Name, m_Property.m_Name);
	m_Level = m_Property.m_level;
	m_dModID = m_Property.m_LookID;
	//初始化怪物脚本事件
	InitScriptEvent();


	if (IsBoss()) {	//我是BOSS
		UpdateMyData(pParam->reginID,0,0,"无",1);
	}

	

	return CFightObject::OnCreate(pParameter);
}



void CMonster::Thinking()
{
	///超过1分钟调用一次脚本
// 	if(GetTickCount()-m_nBirthTime>60000)
// 	{
// 	///暂时调用下初始化的脚本测试
// 		g_Script.SetCondition(this, NULL, NULL);
// 		g_Script.CallFunc("AttackMonsterScript");
// 		g_Script.CleanCondition();
// 		m_nBirthTime=GetTickCount();
// 	}

	if (m_BackProtection)
		return;

	const SMonsterBaseData *pData = CMonsterService::GetInstance().GetMonsterBaseData(m_Property.m_ID);
	if (!pData)
		return;

	switch (GetCurActionID())
	{
	case EA_STAND:
		if (0xffffffff == m_SearchTargetTime)
		{
			m_SearchTargetTime = timeGetTime();
//			rfalse(2, 1, "完成了上次搜索，开始新的搜索！【T:%d】", m_SearchTargetTime);
		}

		// 如果不移动，则保持站立
		if (MMT_NOMOVE == pData->m_MoveType)
			break;

		// 如果是被动还击，并且当前有目标在攻击，那么转入攻击流程
		if (MAT_PASSIVE == pData->m_AttType && 0 != m_dwLastEnemyID)
			break;

		if (MAT_ACTIVELY == pData->m_AttType && 0 == m_dwLastEnemyID && 0xffffffff != m_SearchTargetTime && 
								timeGetTime()-m_SearchTargetTime>5000 && m_ParentRegion && m_ParentArea)
		{
			m_SearchTargetTime = 0xffffffff;

			// 搜寻一个目标
			check_list<LPIObject> *playerList = m_ParentRegion->GetPlayerListByAreaIndex(m_ParentArea->m_X, m_ParentArea->m_Y);
			
			if (playerList)
			{
				for (check_list<LPIObject>::iterator it = playerList->begin(); it != playerList->end(); ++it)
				{
					LPIObject target = *it;
					CPlayer *player = (CPlayer *)target->DynamicCast(IID_PLAYER);
					if (player && 0 != player->m_CurHp)
					{
						m_dwLastEnemyID = player->GetGID();	
//						rfalse(2, 1, "找到一个没死的目标【%d】，准备发起攻击！", m_dwLastEnemyID);
						break;
					}
				}				
			}
		}

		if (--m_ActionFrame <= 0)
		{
			if ((m_fightState & FS_DIZZY) || (m_fightState & FS_DINGSHENG))
			{
				m_ActionFrame = 1;
				return;
			}

			// 站立的时间结束，开始随机移动
			SQSynPathMsg msg;
			
			MakeRadomPath(m_Property.m_ActiveRadius, m_Property.m_BirthPosX << TILE_BITH, m_Property.m_BirthPosY << TILE_BITH, msg);
			
			// 如果设置路径失败，则继续站立
			if (!SetMovePath(&msg))
				m_ActionFrame = CRandom::RandRange(m_Property.m_StayTimeMin, m_Property.m_StayTimeMax);
			
			return;
		}
		break;
	
	case EA_RUN:
		// 如果在移动过程中，被人攻击，那么转入攻击流程
		if (SQSynPathMsg::normal == m_IsMove2Attack && 0 != m_dwLastEnemyID)
		{
			SetCurActionID(EA_STAND);
			SetBackupActionID(EA_STAND);
		}
		break;

	case EA_DEAD:
		MY_ASSERT(0 == m_CurHp);

		if (--m_ActionFrame <= 0)
			SetRenascence(0, 0, m_MaxHp, 2);
		break;

	default:
		break;
	}

	// 判断是否有人在攻击
	if (GetCurActionID() == EA_STAND && m_dwLastEnemyID != 0 && MAT_RUBBER != pData->m_AttType)
	{
		///在队伍攻击模式下随即切换目标
// 		LPIObject tempobject=m_ParentRegion->SearchObjectListInAreas(m_dwLastEnemyID, m_ParentArea->m_X, m_ParentArea->m_Y);
// 		if (!tempobject)return;
// 		CPlayer *pDestattk = (CPlayer *)tempobject->DynamicCast(IID_PLAYER);
// 		if (pDestattk&&pDestattk->m_dwTeamID>0&&pDestattk->m_CurHp>0)
// 		{
// 			///如果被攻击开始计算攻击开始时间
// 			if (m_nAttackTime==0)m_nAttackTime=timeGetTime();
// 
// 			///超过5秒随即切换一个目标
// 			if (m_nAttackTime>0&&timeGetTime()-m_nAttackTime>5000)
// 			{
// 				GetRandomEnemyID();
// 				m_nAttackTime=timeGetTime();
// 				m_nRandomSkillIndex=rand()%5;
// 			}
// 		}

		LPIObject object=m_ParentRegion->SearchObjectListInAreas(m_dwLastEnemyID, m_ParentArea->m_X, m_ParentArea->m_Y);
		if (!object)return;
		
		float DefenderWorldPosX = 0.0f;
		float DefenderWorldPosY = 0.0f;

		CPlayer *pDest = (CPlayer *)object->DynamicCast(IID_PLAYER);
		CFightPet	*pfpet =  (CFightPet *)object->DynamicCast(IID_FIGHT_PET);
		if  (pDest && pDest->m_CurHp > 0)
		{
			DefenderWorldPosX = pDest->m_curX;
			DefenderWorldPosY = pDest->m_curY;
		}

		//2014/06/10 ly 注释掉，不希望怪物攻击宠物
		//else if(pfpet && pfpet->m_CurHp > 0)
		//{
		//	DefenderWorldPosX = pfpet->m_curX;
		//	DefenderWorldPosY = pfpet->m_curY;
		//}
		else
		{
			m_dwLastEnemyID = 0;
		}

		if(m_dwLastEnemyID == 0)return;

		SQuestSkill_C2S_MsgBody msg;
		msg.mDefenderGID				= m_dwLastEnemyID;
		msg.mAttackerGID				= GetGID();
		msg.mDefenderWorldPosX			= DefenderWorldPosX;
		msg.mDefenderWorldPosY			= DefenderWorldPosY;
		msg.dwSkillIndex				= m_nRandomSkillIndex;

		ProcessQuestSkill(&msg);
		
	}

}

SASynMonsterMsg *CMonster::GetStateMsg()
{
    static SASynMonsterMsg msg;

	msg.dwGlobalID		= GetGID();
	msg.bChangeType		= m_byMutateState;
	msg.bMonsterType = m_Property.m_LevelType;
	//msg.mMoveSpeed = (float)m_Property.m_FightSpeed/1000;
	msg.mMoveSpeed = (float)m_Property.m_WalkSpeed ;
	msg.direction		= m_Direction;
	msg.ilevel = m_Level;
	msg.mImageID = m_dModID;
	msg.monsterID = m_Property.m_ID;
	msg.fScale			= m_Property.m_BodySize/100.0f;
	msg.dfightState = m_fightState;
	dwt::strcpy(msg.sName, m_Name, strlen(m_Name)+1);
	FillSynPos(&msg.ssp);

// 	char s[255]; memset(s, 0, 255);
// 	rfalse(2,1,"SASynMonsterMsg | GID:%d ", msg.dwGlobalID);
// 	for (int i = 0; i < msg.ssp.m_wayPoints; i++)
// 	{
// 		sprintf(s, " -- Idx=%d,X=%4.2f,Y=%4.2f", i , msg.ssp.m_path[i].m_X, msg.ssp.m_path[i].m_Y);
// 		rfalse(2,1,s);
// 	}

    return &msg;
}

BOOL CMonster::GetDropItem(CPlayer *Killer)
{
	if (!m_ParentRegion || !Killer)
		return FALSE;

	if (m_dCreatePlayerID)
	{
		CPlayer *player = (CPlayer*)GetPlayerByGID(m_dCreatePlayerID)->DynamicCast(IID_PLAYER);
		if (player)
		{
			Killer = player;
			m_dCreatePlayerID = 0;
		}
	}

	CItem::SParameter args;
	std::vector<SRawItemBuffer> item;

	const SMonsterDropTable *MDTable = CItemDropService::GetInstance().GetMonsterDropTable(m_Property.m_ID);
	if (!MDTable)
		return FALSE;

	WORD MoneyMountRate = CRandom::RandRange(70, 130);
	WORD tempMoney = MDTable->m_MoneyMount * MoneyMountRate / 100;

	SMonsterDropTable *mtab = const_cast<SMonsterDropTable*>(MDTable);

	if (Killer->limitedState == CPlayer::LIMITED_HALF){  //属于防沉迷时间，掉落金钱概率减半
		mtab->m_MoneyRate /= 2;
		Killer->ShowLimitTips();
	}
	else if (Killer->limitedState == CPlayer::LIMITED_ZERO) {	//属于疲劳时间，收益为0
		mtab->m_MoneyRate = 0;
		Killer->ShowLimitTips();
	}

	MDTable->m_MoneyRate > CRandom::RandRange(1, 10000) ? (args.Money = tempMoney, args.MoneyType = MDTable->m_MoneyType) 
														: (args.Money = 0, args.MoneyType = 0);

	if (args.Money)
		MY_ASSERT(args.MoneyType);

	for (int i = 0; i < 10; i++)
	{
		DWORD itemID		= MDTable->m_DropTable[i][0];
		DWORD itemRate		= MDTable->m_DropTable[i][1];
		DWORD itemNum		= MDTable->m_DropTable[i][2];
		DWORD itemUpdate	= MDTable->m_DropTable[i][3];
		
		if (!itemID)
			continue;

		if (0 == itemNum || itemRate > 10000 || itemUpdate > 10)
			return FALSE;

		if (Killer->limitedState == CPlayer::LIMITED_HALF){  //属于防沉迷时间，掉落物品概率减半
			itemRate /= 2;
			Killer->ShowLimitTips();
		}
		else if (Killer->limitedState == CPlayer::LIMITED_ZERO) {	//属于疲劳时间，收益为0
			itemRate = 0;
			Killer->ShowLimitTips();
		}

		// 首先断定概率，如果失败，则不需要计算下面的内容
		DWORD tempRate = CRandom::RandRange(1, 10000);
		if (tempRate > itemRate)
			continue;

		// 掉落物品
		if (itemID >= 200000001)				// 全局掉落表
		{
			const std::vector<DWORD> &temp = CItemDropService::GetInstance().GetGlobalDropTable(itemID);
			if (temp.empty())
				return FALSE;

			WORD rateNum = CRandom::RandRange(1, temp.size()) - 1;

			itemID = temp[rateNum];
		}		
		
		const SItemBaseData *pItemData = CItemService::GetInstance().GetItemBaseData(itemID);
		if (!pItemData)
			return FALSE;

		// 要产生的个数不能大于物品的最大叠加数
		if (itemNum > pItemData->m_Overlay)
			return FALSE;

		SRawItemBuffer temp;
		BOOL rt = CItemUser::GenerateNewItem(temp, SAddItemInfo(itemID, itemNum));
		if (FALSE == rt)
			return FALSE;

		// 如果是任务道具，直接加到人的身上
		if (ITEM_IS_TASK(pItemData->m_Type))
		{
			if (pItemData->m_TaskID && TS_ACCEPTED == Killer->GetTaskStatus(pItemData->m_TaskID))
			{
				if (Killer->GetItemNum(pItemData->m_ID, XYD_FT_WHATEVER) < pItemData->m_TaskNeed)
				{
					std::list<SAddItemInfo> itemList;
					itemList.push_back(SAddItemInfo(pItemData->m_ID, 1));		// 任务道具只能掉落1个

					if (Killer->CanAddItems(itemList))
						Killer->StartAddItems(itemList);
				}
			}
		}
		else
			item.push_back(temp);
	}
	
	if (item.empty() && 0 == args.Money)	// 没东西可掉
		return FALSE;

	CSingleItem::SParameter Singleargs;

	if (args.Money)
	{
		SRawItemBuffer itemTemp;
		int moneyid = Killer->_L_GetLuaValue("OnGetMoneyItemID"); //判断是否为金钱道具
		if (!moneyid)return FALSE;
		
		BOOL rt = CItemUser::GenerateNewItem(itemTemp, SAddItemInfo(moneyid, args.Money)); //这里金钱作为一个特殊的道具处理
		if (FALSE == rt)
			return FALSE;

		Singleargs.PackageModel = IPM_BUDAI;
		Singleargs.dwLife			= MakeLifeTime(5);
		Singleargs.xTile			= m_curTileX;
		Singleargs.yTile			= m_curTileY;
		Singleargs.ItemsNum = itemTemp.overlap;
		Singleargs.ItemsID = itemTemp.wIndex;
		Singleargs.ProtectedGID=Killer->GetGID();
		Singleargs.ProtectTeamID	= Killer->m_dwTeamID;
		Singleargs.MoneyType = args.MoneyType;
		
		if (!CSingleItem::GenerateNewGroundItem(m_ParentRegion, 3, Singleargs, itemTemp, LogInfo(m_Property.m_ID, "怪物掉落")))
			return FALSE; 
	}

	std::vector<SRawItemBuffer>::iterator iter = item.begin();
	while(iter != item.end())
	{
		SRawItemBuffer itemTemp = *iter;
	

		Singleargs.PackageModel = IPM_TANMUBOX;
		Singleargs.dwLife			= MakeLifeTime(5);
		Singleargs.xTile			= m_curTileX;
		Singleargs.yTile			= m_curTileY;
		Singleargs.ItemsNum = itemTemp.overlap;
		Singleargs.ItemsID = itemTemp.wIndex;
		Singleargs.ProtectedGID=Killer->GetGID();
		Singleargs.ProtectTeamID	= Killer->m_dwTeamID;
		if (!CSingleItem::GenerateNewGroundItem(m_ParentRegion, 3, Singleargs, itemTemp, LogInfo(m_Property.m_ID, "怪物掉落")))
			return FALSE; 

		iter++;
	}

	//if (!CItem::GenerateNewGroundItem(m_ParentRegion, 3, args, item, LogInfo(m_Property.m_ID, "怪物掉落")))
		//return FALSE; 
	return TRUE;
}

int CMonster::Attack()
{
    return WatchArea();
}

int CMonster::WatchArea()
{
    if (m_ParentRegion == NULL)
        return 0;

    int iRet = 0;

    /*

    WORD Area_X;
    WORD Area_Y;

    WORD NowX;
    WORD NowY;

    WORD DistanceNow;
    WORD DistanceBest;

    CArea * pArea;
    CPlayer* pTarget = NULL;          // 选择的目标
    CPlayer* pPriorityTarget = NULL;  // 用于记录优先的目标

    Area_X = m_ParentArea->m_X;
    Area_Y = m_ParentArea->m_Y;

    int ix;
    int iy;
    DistanceBest = 20;
    //Search around 9 areas for life 
    for (int iLooper = 0;iLooper < 9 ;iLooper ++)
    {
        switch(iLooper) 
        {
        case 0:
            ix = 0;
            iy = 0;
            break;
        case 1:
            ix = 0;
            iy = -1;
            break;
        case 2:
            ix = 1;
            iy = -1;
            break;
        case 3:
            ix = 1;
            iy = 0;
            break;
        case 4:
            ix = 1;
            iy = 1;
            break;
        case 5:
            ix = 0;
            iy = 1;
            break;
        case 6:
            ix = -1;
            iy = 1 ;
            break;
        case 7:
            ix = -1;
            iy = 0;
            break;
        case 8:
            ix = -1;
            iy = -1;
            break;
        default:
            break;
        }

        pArea = ( CArea* )m_ParentRegion->GetArea( Area_X + ix ,Area_Y + iy )->DynamicCast( IID_AREA );
        if ( pArea == NULL)
            continue;

        check_list< LPIObject >::iterator it = pArea->m_PlayerList.begin();
        while ( it != pArea->m_PlayerList.end() )
        {
            pTarget = ( CPlayer* )( *it )->DynamicCast( IID_PLAYER );
            
            // 怪物不会主动攻击隐身GM
            if ( ( pTarget->m_Property.m_GMLevel != 0 ) && ( ( LPBYTE )&pTarget->m_Property.m_dwConsumePoint )[ 4 ] == 0 )
            {
                it ++;
                continue;
            }

            BOOL isPriority = ( bySelectType == 1 );  // 是否为优先
            BOOL isExcept   = ( bySelectType == 2 );  // 是否为排除
            BOOL type       = FALSE;
            switch ( byPriorityExceptType ) // 优先或排除选择目标类型， 1,角色  2,门派  3,性别  4,帮派  5,队伍
            {
            case 1:  // 某个角色优先或排除
                type = ( pTarget->m_Property.m_dwStaticID == dwPriorityTarget );
                break;
            case 2: //  某个门派优先或排除
                type = ( pTarget->m_Property.m_School == ( BYTE )dwPriorityTarget );
                break;
            case 3: //  某个性别优先或排除
                type = ( pTarget->m_Property.m_Sex == ( BYTE )dwPriorityTarget );
                break;
            case 4: //  某个帮派优先或排除
                type = ( pTarget->m_Property.factionId == ( WORD )dwPriorityTarget );
                break;
            case 5: //  某个队伍优先或排除
                type = ( pTarget->m_dwTeamID == dwPriorityTarget );
                break;
            }

            if ( type && isExcept )  // 如果被排除，直接进行下一次循环
            {
                it++;
                continue;
            }

			pTarget->GetCurPos( NowX, NowY );

			DistanceNow = max( abs( NowX - m_wCurX ), abs( NowY - m_wCurY ) );
			if ( ( DistanceNow < DistanceBest ) && ( DistanceNow <= m_Property.m_wMoveArea ) && ( pTarget->m_CurHp != 0 ) )
			{
                if ( type && isPriority )  // 此对象为优先对象,记录下来
                    pPriorityTarget = pTarget;

                m_pAttackTarget = pTarget;
				targetDamage = 0;
				targetSegTime = timeGetTime() + ( rand() % 10000 );

				m_dwEndAttackTime = timeGetTime() + 40000;

				iRet = 1;
				DistanceBest = DistanceNow;
			}
            it++;
        }

        if ( pPriorityTarget != NULL )         // 如果有优先的对象，改变之前选择的可能为普通对象的指针
            m_pAttackTarget = pPriorityTarget; // 到这里的时间很短，就不用重新记录m_dwEndAttackTime时间了
    }
*/
    return iRet;
}


//----------------------------------------------------------------------------------------------------------	
BOOL CMonster::CheckTarget(void)
{
	/*
    if (m_ParentRegion == NULL)
        return false;

    if (m_dwLastEnemyID != 0)
    {
        m_pAttackTarget = m_ParentRegion->SearchObjectListInAreas(m_dwLastEnemyID, m_ParentArea->m_X, m_ParentArea->m_Y);
        m_dwLastEnemyID = 0;
        targetDamage = 0;
        targetSegTime = timeGetTime() + ( rand() % 10000 );
        m_fAttackWaitFrames = (float)(m_Property.hitSpeed >> 1);
        return true;
    }//*/

    return false;
}

bool CMonster::StopTracing()
{
	// 检测停止追踪的条件
	float m_BX	= ((DWORD)m_Property.m_BirthPosX) << TILE_BITW;
	float m_BY	= ((DWORD)m_Property.m_BirthPosY) << TILE_BITH;

	float distance = sqrt(pow((m_curX-m_BX),2)+pow((m_curY-m_BY),2));
	if ((WORD)distance > m_Property.m_TailLength)  //停止追击的同时移除追击
	{
		SetStopTracing(EA_RUN);
		return true;
	}


	
		CFightObject *pfight =(CFightObject*) GetObjectByGID(m_dwLastEnemyID)->DynamicCast(IID_FIGHTOBJECT);
		if (pfight)
		{
			const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(m_wCuruseskill);
			if (pData)
			{
				D3DXVECTOR2 curPos((m_curX - pfight->m_curX), (m_curY - pfight->m_curY));
				float fdistance = D3DXVec2Length(&curPos);
				if (fdistance > (pData->m_SkillMaxDistance + 100)) //距离远
				{
					D3DXVECTOR2 vPos((pfight->m_curX - m_vdesPos->x), (pfight->m_curY - m_vdesPos->y));
					fdistance = D3DXVec2Length(&vPos);
					if (fdistance > 100) //追击目标已经移动过了
					{
						SendMove2TargetForAttackMsg(pData->m_SkillMaxDistance, m_wCuruseskill, pfight);
						//rfalse(2, 1, "追击目标已经移动过了");
						return false;
					}
				}
				else  //可能已经和目标想接近
				{
					SetCurActionID(EA_STAND);
					SetBackupActionID(EA_STAND);
					Stand(EA_STAND);
					m_isRunEnd = false;
					OnRunEnd();
					//rfalse(2, 1, "可能已经接近目标");
					return true;
				}

			}
			return false;
		}
	
		
	SQSynPathMsg msg;
	
	msg.m_GID		= GetGID();
	msg.m_move2What = SQSynPathMsg::attack;
	msg.m_Path.m_wayPoints		= 2;
	msg.m_Path.m_moveType		= EA_RUN;
	msg.m_Path.m_path[0].m_X	= m_curX;
	msg.m_Path.m_path[0].m_Y	= m_curY;
	msg.m_Path.m_path[1].m_X	= m_BX;
	msg.m_Path.m_path[1].m_Y	= m_BY;

	// 如果设置路径失败，则瞬移过去
	if (!SetMovePath(&msg))
	{
		// 咋个瞬移喃？

	}
	else
	{
		m_IsMove2Attack  = SQSynPathMsg::normal;
		m_BackProtection = true;
	}

	m_dwLastEnemyID = 0;
	//m_BackProtection = true;
	//m_Speed = m_Property.m_WalkSpeed / 1000;
	return true;
}

//----------------------------------------------------------------------------------------------------------	
BOOL CMonster::AttackTarget(void)
{
    // 锁定攻击
	/*
    if (GetCurActionID() == EA_STAND)
    {
        CFightObject *pDest = (CFightObject *)m_pAttackTarget->DynamicCast(IID_FIGHTOBJECT);
        if (pDest == NULL)
        {
            // 如果对像不存在
            m_pAttackTarget = NULL;
        }
        else if (pDest->m_CurHp == 0)
        {
            // 如果对像死亡
            m_pAttackTarget = NULL;
        }
        else if (pDest->m_ParentRegion != m_ParentRegion)
        {
            // 如果对像不在同一个场景
            m_pAttackTarget = NULL;
        }
        else
        {
            WORD xTarget;
            WORD yTarget;
            // Wonly 2003-10-31 为修正玩家被攻击移动后，怪物攻击玩家要走到的点，实际上玩家过不去
            pDest->GetCurPos(xTarget, yTarget);
            //pDest->GetMarkOffPos(xTarget, yTarget);
            // 如果对象相距太远(1.2屏)
            if ( m_Property.hitArea != 2 )
            {
                if (abs((int)xTarget - (int)m_wCurX) > (15))
                    m_pAttackTarget = NULL;
                if (abs((int)yTarget - (int)m_wCurY) > 21)
                    m_pAttackTarget = NULL;
            }
        }
        
        if (m_pAttackTarget)
        {
            WORD xTarget;
            WORD yTarget;
            // Wonly 2003-10-31 为修正玩家被攻击移动后，怪物攻击玩家要走到的点，实际上玩家过不去
            pDest->GetCurPos(xTarget, yTarget);
            //pDest->GetMarkOffPos(xTarget, yTarget);
            // 如果对像不在攻击范围以内，就向对象移动

            // 如果是无法移动的类型，那么就不能像目标移动了,这一步移动到了MoveToTarget里边
            int ret = 0;
			ret = MoveToTarget( m_Property.hitArea );

            if ( ret == -1 && m_Property.hitArea != 2 )
            {
                // 如果找不到移动的路径就取消当前的攻击目标
                m_pAttackTarget = NULL;
            }
            else if (!m_fAttackWaitFrames && ret == 0)
            {
                SQAFightObjectMsg iMsg;
                iMsg.sa.dwGlobalID = GetGID();
                iMsg.sa.wPosX = m_wCurX;
                iMsg.sa.wPosY = m_wCurY;
                iMsg.sa.byWaitFrames = 0;
                //iMsg.sa.byAttackType = 0;
                //iMsg.sa.bySkillIndex = 0;
                iMsg.sa.byFightID = 0;
                iMsg.dwDestGlobalID = m_pAttackTarget->GetGID();
                SetAttackMsg(&iMsg);
            }
        }
    }
	//*/
    return  FALSE;
}

void CMonster::SetRenascence(DWORD dwLast, DWORD dwCur, DWORD dwHP, WORD wTime)
{
	if (!m_ParentRegion)
		return;

	SetCurActionID(EA_STAND);
	SetBackupActionID(EA_STAND);

	m_dwLastEnemyID = dwLast;

	m_CurHp = dwHP;

	m_curX		= ((DWORD)m_Property.m_BirthPosX) << TILE_BITW;
	m_curY		= ((DWORD)m_Property.m_BirthPosY) << TILE_BITH;
	m_curZ		= 0.0f;
	m_Direction = 0.0f;
	m_curTileX	= m_Property.m_BirthPosX;
	m_curTileY	= m_Property.m_BirthPosY;

	m_BackProtection	= false;
	m_SearchTargetTime	= timeGetTime();

	m_ActionFrame = 5/*CRandom::RandRange(100, 150)*/;
    
	if (m_ParentRegion->Move2Area(self.lock(), m_curTileX, m_curTileY, true))
		Stand();
	//我是BOSS
	if (m_Property.m_LevelType > 1) {
		UpdateMyData(m_ParentRegion->m_wRegionID,0,0,"木有人击杀",1);
	}
}

//----------------------------------------------------------------------------------------------------------	
void SetRandAttributes( DWORD level, SEquipment &equip )
{
    extern int randomatt_levelLimit, randomatt_below, randomatt_above;
    extern DWORD tickRandom();

    DWORD rand_ratting = randomatt_below;
    if ( level > (DWORD)randomatt_levelLimit )
        rand_ratting = randomatt_above;

    // 这里的rand_ratting是万分之一的比例精度
    if ( RandomAbilityTable::RATable.validate && ( ( tickRandom() % 10000 ) < rand_ratting ) )
    {
        int rand_times = 1;
	    int rand_ratting = rand() % 10000;
        if ( rand_ratting < 3 )         rand_times = 7;
        else if ( rand_ratting < 10 )   rand_times = 6;
        else if ( rand_ratting < 20 )   rand_times = 5;
        else if ( rand_ratting < 40 )   rand_times = 4;
        else if ( rand_ratting < 100 )  rand_times = 3;
        else if ( rand_ratting < 250 )  rand_times = 2;

        // 循环指定次数进行抽属性
        for ( int slot_index = 0; rand_times != 0; rand_times -- )
        {
            // 随机一个数，然后在循环中递减，来尝试命中
            rand_ratting = tickRandom() % 100000000;
            for ( int i = 1; ( i < 32 ) && ( rand_ratting > 0 ); i ++ )
            {
                if ( RandomAbilityTable::RATable.units[i].randomType == RandomAbilityTable::TYPE_NULL )
                    break;

                if ( (DWORD)rand_ratting > RandomAbilityTable::RATable.units[i].hitRating )
                {
                    rand_ratting -= RandomAbilityTable::RATable.units[i].hitRating;
                    continue;
                }

                //// 命中，按顺序获取attributes
                //SEquipment::ExtraAttribute &attex = equip.attributes[ slot_index ++ ];
                //attex.type = RandomAbilityTable::RATable.units[i].randomType;

                //// 尝试去抽取后面几种更高级属性
                //int j = rand() % 7;
                //if ( ( RandomAbilityTable::RATable.units[i].array[ 1 + j ].rating == 0 ) || 
                //    ( ( tickRandom() % 100000000 ) > RandomAbilityTable::RATable.units[i].array[ 1 + j ].rating ) )
                //    j = 0; // 如果失败则设定为默认值！

                //attex.value = j;
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------
#define RATE(_x, _r)    \
    if(_x < 200) { _r = (_x % 100) * 100;  } \
    else if(_x < 2000) { _r = (_x % 1000) * 10;  } \
    else if(_x < 20000) { _r = (_x % 10000);  }  \
	else { _r = 0;  }  
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// 直接在指定的数组上随机抽
// 配置表关联的属性类型ID
extern WORD mSpecialAttIdx[ 57 ];

/*
extern void GetAddAttribute( SEquipment::Attribute::Unit *unit, WORD id, int maxNum, WORD coff[ 57 ] = NULL, BYTE level = 0, BYTE itemType = 0 )
{
    #define CALCVALUE() ( ( ( m - n ) <= 0 ) ? n : ( rand() % ( ( m - n ) + 1 ) ) + n )

	if ( maxNum > 6 )
		return;

    SGemAtt* genAtt = CItem::GetSpecialItemAttribute( id );
    if ( genAtt == NULL )
        return;

    BYTE selectIdx[ 60 ];
    BYTE maxSelectIdx[ 60 ];        // 记录最多数量的一次
    bool levelIndex[ 4 ] = { 0 };   // 鉴定出来的四级属性的级别索引
    
    int  maxOddssIdx = 0;
    int  selectNum   = 0;
    int  maxSelNum   = 0;
    int  dropNum     = 0;           // 因为同时出现不应该同时出现的属性而丢弃的属性数量

    BYTE byTimes   = 1;             // 决定循环随机多少次
    BYTE byAttrNum = 1;             // 应该出几条属性

    int firstAttr = 0;
    if ( itemType == 104 ) 
        firstAttr = 14;  // 武器加外功
    else if ( itemType == 102 || itemType == 105 || itemType == 106 || itemType == 107 || itemType == 108 ) 
        firstAttr = 16;  // 防具加防御
    else if ( itemType == 101 || itemType == 103 || itemType == 112 || itemType == 113 || itemType == 110 || itemType == 111 ) 
        firstAttr = 17;  // 首饰加身法
    g_LevelAttributesArray[ 0 ][ 0 ] = firstAttr;

    if ( level == 0 && coff == NULL )      // 为了兼容掉落表
        byTimes = 1;
    else if ( level == 3 )                 // 对应关系为：1,2级选1次  3级最多选2次 4, 5级最多选3次 6级最多选4次
        byTimes = 2;                       // 如果出现选择次数个属性就停止选择
    else if ( level == 4 )
        byTimes = 3;
    else if ( level == 5 || level == 6 )
        byTimes = level - 2;
    
    byAttrNum = byTimes;
    
    while ( byTimes )
    {
        // 先行抽取所有命中概率的随机属性
        for ( int i = 0; i < 57; ++i )
        {
            // 有偏向加成就 加成 = 比例 * 加成因子( float )，否则不加成
            DWORD oddss = genAtt->att[ i ][ 0 ] * ( ( coff == NULL || coff[ i ] == 0 ) ? 1 : coff[ i ]  );  
            oddss = ( coff == NULL || coff[ i ] == 0 ) ? oddss : ( oddss / 100 );
            if ( oddss > genAtt->att[ maxOddssIdx ][ 0 ] )  // 记录最大的ODDSS索引值
                maxOddssIdx = i;

            if( IN_ODDSS( oddss ) )
                selectIdx[ selectNum++ ] = i;
        }

        if ( selectNum > maxSelNum )
        {
            maxSelNum = selectNum;
            memcpy( maxSelectIdx, selectIdx, sizeof( maxSelectIdx ) );  // 记录最大的一次
        }

        byTimes--;

        // 如果随机一次没有出现2个属性，就继续随机，最多循环2次
        if ( ( level == 3 || level == 4 ) && selectNum >= 2 )  
            break;

        // 如果随机一次没有出现level - 1个属性，就继续随机，最多循环level - 1次
        if ( ( level == 5 || level == 6 ) && selectNum >= level - 2 )  
            break;

        if ( byTimes > 0 )  // 如果还要随机，先清空重新随机
        {
            memset( selectIdx, 0, sizeof( selectIdx ) );
            selectNum = 0;
        }
    }

    BYTE* pIdxArray = ( selectNum >= maxSelNum ) ? selectIdx : maxSelectIdx;
    selectNum = max( selectNum, maxSelNum );
    
    // 如果已经抽出来的属性比计划的属性多，则需要做再随机
    BOOL randSet = maxNum < selectNum;  
    maxNum = min( maxNum, selectNum );
	for ( int i = 0; i < maxNum; i++ ) 
	{
        int idx = 0;
        if ( randSet == FALSE )  // 不需要再随机
		    idx = pIdxArray[ i ];
        else
        {
            // 先抽出来随机命中的属性数据，并为下一次抽取准备数据
            int selPos = rand() % selectNum;
		    idx  = pIdxArray[ selPos ];
            pIdxArray[ selPos ] = pIdxArray[ --selectNum ];
        }

        // 判断是否存在两条不能同时出现的属性
        bool isFound = false;
        for ( int j = 0; j < i; ++j )
        {
            if ( ( mSpecialAttIdx[ idx ] == 28 && unit[ j ].type == 29 ) || 
                 ( mSpecialAttIdx[ idx ] == 29 && unit[ j ].type == 28 ) ||  // 外增和内增
                 ( mSpecialAttIdx[ idx ] == 18 && unit[ j ].type == 19 ) || 
                 ( mSpecialAttIdx[ idx ] == 19 && unit[ j ].type == 18 ) )   // 内抵和外抵
            {
                dropNum++;
                isFound = true; 
                break;
            }
        }

        if ( isFound ) // 如果发现有不能同时出现的直接丢弃( 这里不随机是因为谁先写入本来就是随机的，如果这里随机下面不好处理)
            continue;
        
		unit[ i ].type = mSpecialAttIdx[ idx ];
		int n = genAtt->att[ idx ][ 1 ];
		int m = genAtt->att[ idx ][ 2 ];

        if ( unit[ i ].type == SGemData::GEMDATA_ACTIONSPEED || unit[ i ].type == SGemData::GEMDATA_MOVESPEED )
            unit[ i ].value = 500 + CALCVALUE();
        else
            unit[ i ].value = CALCVALUE();

        // 只有鉴定并且属性个数少于最小个数才进入
        if ( coff != NULL && level != 0 && ( ( maxNum - dropNum ) < byAttrNum ) ) 
        {                                                       
            for ( int i = 0; i < 4; ++i )
            {
                for ( int j = 0; j < 3; ++j )
                {
                    if ( g_LevelAttributesArray[ i ][ j ] == idx ) // 查找四级属性表里面有没有这条属性
                    {
                        levelIndex[ i ] = true;                    // 表示这级属性已经被鉴定出来了
                        goto __stopSearch;
                    }
                }
            }
        }

__stopSearch:
        continue;
    }

    // 如果鉴定出来的属性个数少于最少个数
    if ( coff != NULL && level != 0 && ( ( maxNum - dropNum ) < byAttrNum ) )  
    {
        for ( int i = 0; i < ( byAttrNum - ( maxNum - dropNum ) ); ++i )  // 加上差的几条属性
        {
            for ( int j = 0; j < 4; ++j )
            {
                if ( levelIndex[ j ] == true ) 
                    continue;

                int index = g_LevelAttributesArray[ j ][ rand() % 3 ];
                if ( index < 0 || index >= 57 )
                    continue;

                int n = genAtt->att[ index ][ 1 ];
                int m = genAtt->att[ index ][ 2 ];

                unit[ maxNum + i ].type  = mSpecialAttIdx[ index ];
                if ( unit[ maxNum + i ].type == SGemData::GEMDATA_ACTIONSPEED || unit[ maxNum + i ].type == SGemData::GEMDATA_MOVESPEED )
                    unit[ maxNum + i ].value = 500 + CALCVALUE();
                else
                    unit[ maxNum + i ].value = CALCVALUE();

                levelIndex[ j ] = true;
                break;
            }
        }
    }
    else if ( coff == NULL && maxNum == 0 && level == 0 )  // 掉落直接选最大的
    {
        int n = genAtt->att[ maxOddssIdx ][ 1 ];
        int m = genAtt->att[ maxOddssIdx ][ 2 ];
        unit[ 0 ].type  = mSpecialAttIdx[ maxOddssIdx ];
        if ( unit[ 0 ].type == SGemData::GEMDATA_ACTIONSPEED || unit[ 0 ].type == SGemData::GEMDATA_MOVESPEED )
            unit[ 0 ].value = 500 + CALCVALUE();
        else
            unit[ 0 ].value = CALCVALUE();
    }
}//*/

/**【掉宝率衰减规则】
	超过怪物等级50级后，爆率衰减100%

	例外情况：
	怪物类型是2，3，4的怪物，爆率不衰减
*/
BOOL CMonster::SendDropItemOnDead(BOOL bDoubleDrop, CPlayer *pActPlayer)
{
	/*
    if (pActPlayer == NULL)
        return FALSE;

    if (m_ParentRegion == NULL)
        return FALSE;

    g_Script.SetCondition( this, pActPlayer, NULL );
	BOOL SelectDropResult( CPlayer *pActPlayer, DWORD did, DWORD gid, DWORD tid, POINT pos, CRegion *region, CMonster *monster );
    POINT pos = { m_wCurX, m_wCurY };
    if ( m_Property.boss == 2)
        SelectDropResult( pActPlayer, m_Property.id, 0xffffffff, 0, pos, m_ParentRegion, this );
    else
    	SelectDropResult( pActPlayer, m_Property.id, pActPlayer->GetGID(), pActPlayer->m_dwTeamID, pos, m_ParentRegion, this );
    g_Script.CleanCondition();
	// */
    return  FALSE;
}
BOOL CMonster::SendDropItemOnDeadXYSJ( BOOL bDoubleDrop, class CPlayer *pActPlayer )
{
	/* For XYD3
	//怪物身上掉落物品的最大个数
	int Maxdrop = m_Property.dropMaxNum;
	//怪物身上任务掉落物品表编号
	int TaskdropTable = m_Property.dropTaskTable;

	extern bool NewItemDropGroundXYSJ(CPlayer *pActPlayer,DWORD did, DWORD count,DWORD protectedGID, 
						DWORD protectedTeamID,POINT pos,CRegion *region,DWORD &dropCount, 
						CMonster *monster,BOOL bind,int dropItemTableId);

	DWORD dropCount = 0;
	
	POINT pos = { m_wCurX, m_wCurY };

	if (pActPlayer == NULL)
		return FALSE;

	if (m_ParentRegion == NULL)
		return FALSE;

	g_Script.SetCondition( this, pActPlayer, NULL );

	if ( m_Property.boss == 2)
		NewItemDropGroundXYSJ(pActPlayer,m_Property.id,Maxdrop,0xffffffff,0,pos,m_ParentRegion,
								dropCount,this,false,TaskdropTable);	
	else
		NewItemDropGroundXYSJ(pActPlayer,m_Property.id,Maxdrop,pActPlayer->GetGID(),pActPlayer->m_dwTeamID,
								pos,m_ParentRegion,dropCount,this,false,TaskdropTable);	

	g_Script.CleanCondition();
	//*/
	return  FALSE;
}

BOOL CMonster::CheckAction(int eCA)        // 检测动作执行的条件
{
    return  TRUE;
}
	
void CMonster::OnDead(CFightObject *PKiller)
{
	_OnDead(PKiller);
	if (CDynamicRegion *pDynRegion = (CDynamicRegion *)m_ParentRegion->DynamicCast(IID_DYNAMICREGION))
	{
		pDynRegion->m_Monster.erase(GetGID());
		if (pDynRegion->m_Monster.empty())
		{
			pDynRegion->m_CanMoveOut		= true;
			pDynRegion->m_MoveOutTimeSeg	= timeGetTime();
			pDynRegion->m_MoveOutTimeleft= timeGetTime();
			pDynRegion->m_MoveOutOT			= 10000;
		}
		CPlayer *pPlayer = (CPlayer *)PKiller->DynamicCast(IID_PLAYER);
		if (pPlayer)
		{
			if (pPlayer->m_ParentRegion->GetGID() == pDynRegion->GetGID())//说明在一个副本中的
			{
				if (g_Script.m_pPlayer)
				{
					CScriptManager::TempCondition temp;
					g_Script.PushCondition(temp);
					LuaFunctor(g_Script,"OnTFPlayerValue")[pDynRegion->m_DynamicIndex]();
					g_Script.PopCondition(temp);
				}
				else
				{
					g_Script.SetCondition(0,pPlayer,0);
					LuaFunctor(g_Script,"OnTFPlayerValue")[pDynRegion->m_DynamicIndex]();
					g_Script.CleanCondition();
				}
			}
		}
	}
	
	if (PKiller)
	{
		g_Script.SetFightCondition(PKiller);
		g_Script.SetCondition(this,0,0);

		CDynamicDailyArea *pDailyScene = (CDynamicDailyArea *)PKiller->m_ParentRegion->DynamicCast(IID_DYNAMICDAILYAREA);
		if (pDailyScene != NULL)
		{
			if (g_Script.PrepareFunction("OnDailyObjectDead"))
			{
				g_Script.PushParameter(GetGID());
				g_Script.PushParameter(GetSceneRegionID());
				g_Script.PushParameter(m_ParentRegion->m_wRegionID);//地图regionid
				if (PKiller)
				{
					g_Script.PushParameter(PKiller->GetGID());	//杀死玩家的gid
				}
				g_Script.PushParameter(this->GetGID());	//当前怪物的gid
				g_Script.Execute();
			}
		}
		else
		{
			if (g_Script.PrepareFunction("OnObjectDead"))
			{
				g_Script.PushParameter(GetGID());
				g_Script.PushParameter(GetSceneRegionID());
				g_Script.PushParameter(m_ParentRegion->m_wRegionID);//地图regionid
				if (PKiller)
				{
					g_Script.PushParameter(PKiller->GetGID());	//gid
				}
				else
				{
					g_Script.PushParameter(0);//竞技场排名
				}
				g_Script.PushParameter(m_dRankIndex);//竞技场排名
				g_Script.Execute();
			}
		}

		if (g_Script.PrepareFunction("OnDropItem"))
		{
			g_Script.PushParameter(m_Property.m_ID);
			if (PKiller)
			{
				g_Script.PushParameter(PKiller->GetGID());
			}
			g_Script.Execute();
		}
		g_Script.CleanFightCondition();
		g_Script.CleanCondition();
	}



	
	//现在，动态场景的怪物的刷新的时间也不会配置为-1了，所以这个调用通常不会进入
	if (m_Property.m_ReBirthInterval == -1 && m_ParentRegion)
		m_ParentRegion->DelMonster(GetGID());
	
	if (IsBoss()) //我是BOSS
	{
		char Killname[CONST_USERNAME];
		CPlayer *player = (CPlayer*)PKiller->DynamicCast(IID_PLAYER);
		if (!player)
		{
			CFightPet *pet =  (CFightPet*)PKiller->DynamicCast(IID_FIGHT_PET);
			{
				if(!pet){
					dwt::strcpy(Killname, "未知玩家击杀", CONST_USERNAME);
				}
				else{
					player = pet->m_owner;
					if(player){
						dwt::strcpy(Killname, player->getname(), CONST_USERNAME);
					}
				}
			}
		}else{
			dwt::strcpy(Killname, player->getname(), CONST_USERNAME);
		}
		SYSTEMTIME stime;
		GetLocalTime(&stime);
		WORD whour = m_Property.m_ReBirthInterval / 1000/60/60 + stime.wHour;
		WORD wMinute =  m_Property.m_ReBirthInterval / 1000/60+ stime.wMinute;
		UpdateMyData(m_ParentRegion->m_wRegionID,whour,wMinute,Killname,0);
	}

	
}

void CMonster::KillMonsterTaskWithSingle(CPlayer *pCurrPlayer)
{
	if (!pCurrPlayer) return;
	CPlayer::TASKKILL::iterator it = pCurrPlayer->m_KillTask.find(this->m_Property.m_ID);
	//CPlayer::TASKKILL::iterator it = pCurrPlayer->m_KillTask.find(this->m_Property.m_KillTaskID);
	if (it != pCurrPlayer->m_KillTask.end())
	{
		for (std::list<DWORD>::iterator ti = (it->second).begin(); ti != (it->second).end(); ++ti)
		{
			DWORD flagIndex = *ti;

			if (CRoleTask *task = pCurrPlayer->m_TaskManager.GetRoleTask((flagIndex & 0xffff0000) >> 16))
			{
				if (STaskFlag *flag = task->GetFlag(flagIndex))
				{
					if (flag->m_Complete)			// 如果已经完成了，不做任何处理
						continue;

					BYTE oldComp = flag->m_Complete;

					if (++flag->m_TaskDetail.Kill.CurKillNum == flag->m_TaskDetail.Kill.KillNum)
						flag->m_Complete = 1;

					// 发送旗标更新消息
					pCurrPlayer->SendUpdateTaskFlagMsg(*flag, oldComp != flag->m_Complete);
				}
				else
				{
					rfalse("检查任务杀怪信息，竟然找不到任务旗标！");
				}
			}
			else
			{
				rfalse("检查任务杀怪信息，竟然找不到任务！");
			}
		}
	}
}

void CMonster::KillMonsterTask(CPlayer *pKiller)
{
	// 任务关联处理
	if (!pKiller) return;

	if(0 == pKiller->m_dwTeamID)
	{
		KillMonsterTaskWithSingle(pKiller);
	}
	else
	{
		for (size_t i=0; i<MAX_TEAM_MEMBER; ++i)
		{
			if(NULL == m_PlayerInDis[i]) continue;
			KillMonsterTaskWithSingle(m_PlayerInDis[i]);
		}
	}
}

void CMonster::HandlePlayerInDis(CPlayer *pKiller)
{
	if (!pKiller) return;

	m_dwPlayerInDisCount = 0;
	for (size_t i=0; i<MAX_TEAM_MEMBER; ++i)	m_PlayerInDis[i] = NULL; //每次都清零数组

	if (!pKiller->m_dwTeamID) return; 

	extern std::map< DWORD, Team > teamManagerMap;
	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pKiller->m_dwTeamID);

	if (iter == teamManagerMap.end())
	{
		rfalse("有队伍ID，却找不到所在队伍！");
		return;
	}

	Team &team = iter->second;
	MY_ASSERT(pKiller->m_ParentRegion);
	WORD mapID	= pKiller->m_Property.m_CurRegionID;

	///主要用于判断是否在动态场景中
	DWORD Regiongid=pKiller->GetDynamicRegionID();
	// 遍历队伍成员,找出所有在有效范围的成员
	for (size_t i=0; i<team.byMemberNum; ++i)
	{
		if (team.stTeamPlayer[i].wRegionID != mapID)
			continue;

		extern LPIObject GetPlayerByGID(DWORD);
		CPlayer *temp = (CPlayer *)GetPlayerByGID(team.stTeamPlayer[i].dwGlobal)->DynamicCast(IID_PLAYER);
		if (!temp || 0 == temp->m_CurHp)
			continue;

		///如果Regiongid不是0 ，判断是否在同一动态场景中
		if (Regiongid!=0&&Regiongid!=temp->GetDynamicRegionID())continue;
		// 测算距离
		DWORD distance = sqrt(pow((pKiller->m_curX-temp->m_curX),2)+pow((pKiller->m_curY-temp->m_curY),2));
		if (distance > 1500)
			continue;

		if(i>=MAX_TEAM_MEMBER)
		{
			rfalse("玩家的队伍Index超过了最大值！");
			continue;
		}

		m_dwPlayerInDisCount ++;
		m_PlayerInDis[i] = temp; //保存起来
	}
}

DWORD GetExp(DWORD dwMonsterExp, DWORD dwPlayerLevel, DWORD dwMonsterLevel)
{
	DWORD dwAddExp=0 , dwLevel=0 ;
	if(dwPlayerLevel <= dwMonsterLevel)
	{
		dwLevel = dwMonsterLevel - dwPlayerLevel;
		dwLevel = (dwLevel>20) ? 20 : dwLevel;

		dwAddExp = dwMonsterExp * (1 - dwLevel * 0.045);
	}
	else
	{
		dwLevel = dwPlayerLevel - dwMonsterLevel;
		dwLevel = (dwLevel > 40) ? 40 : dwLevel;

		dwAddExp = (40 == dwLevel) ? (dwMonsterExp * 0.1) : (dwMonsterExp * (1 - dwLevel * 0.02));
	}

	dwAddExp = (dwAddExp<1) ? 1 : dwAddExp;

	return dwAddExp;
}
//玩家杀死正负2级的怪物，奖获得2点真气值
void CMonster::HandleGetSP(CPlayer *pKiller)
{
	if (!pKiller) return;
	
	if (m_Property.m_Sp > 0)
	{
		pKiller->AddPlayerSp(m_Property.m_Sp);
		pKiller->syneffects(GetGID(), SASynEffectsMsg::EFFECT_ZHENQI, (float)m_curTileX, (float)m_curTileY);
	}
	
}
void CMonster::HandleExpWithTeam(CPlayer *pKiller)
{
 	if (!pKiller) return;

	//组队判断
	if(!pKiller->m_dwTeamID)
	{	//没有组队
		DWORD dwExp = GetExp(m_Property.m_Exp, pKiller->m_Property.m_Level, m_Level);
		if (m_ParentRegion){
			lite::Variant ret;
			LuaFunctor(g_Script, "reginaddexp")[m_ParentRegion->m_wRegionID](&ret);
			if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)return;
			if (0 == ( __int32 )ret)return ;
			dwExp *= ( __int32 )ret;
		}

		if (g_Script.GetMonsterVipExp(pKiller,"VipAddMonsterExp",dwExp,dwExp) == 0)
		{
			rfalse(2, 1, "GetMonsterVipExp Fail");
		}

		if (pKiller->limitedState == CPlayer::LIMITED_HALF){  //属于防沉迷时间，经验直接减半
			dwExp /= 2;
			pKiller->ShowLimitTips();
		}
		else if (pKiller->limitedState == CPlayer::LIMITED_ZERO) {	//属于疲劳时间，收益为0
			dwExp = 0;
			pKiller->ShowLimitTips();
		}
		
		pKiller->SendAddPlayerExp(dwExp, SAExpChangeMsg::MONSTERDEAD, "Killed Monster");
	}
	else
	{
		for (size_t i=0; i<MAX_TEAM_MEMBER; ++i)
		{
			if(NULL == m_PlayerInDis[i]) continue;

			CPlayer* lpPlayer = m_PlayerInDis[i];
			DWORD dwExp = GetExp(m_Property.m_Exp, lpPlayer->m_Property.m_Level, m_Level);
			//队伍个人所得经验＝（单独经验×（1＋（队伍人数－1）×3.5%））÷队伍人数
			dwExp =  dwExp * (1 + (m_dwPlayerInDisCount-1) * 0.05) / m_dwPlayerInDisCount;
			dwExp = (dwExp<1) ? 1 : dwExp;
			//好友关系经验加成
			DWORD addexp = 0 ;
			for (size_t j=0; j<MAX_TEAM_MEMBER; ++j)
			{
				if(NULL == m_PlayerInDis[j]) continue;
				CPlayer* pdest = m_PlayerInDis[j];
				if(pdest->GetGID()!=lpPlayer->GetGID())
				{
//					DWORD temp = pdest->GetAppendExp(lpPlayer->GetName(),dwExp);
//					addexp = addexp<temp?temp:addexp;
				}
			}
			dwExp  = dwExp +addexp ;

			if (pKiller->limitedState == CPlayer::LIMITED_HALF){  //属于防沉迷时间，经验直接减半
				dwExp /= 2;
				pKiller->ShowLimitTips();
			}
			else if (pKiller->limitedState == CPlayer::LIMITED_ZERO) {	//属于疲劳时间，收益为0
				dwExp = 0;
				pKiller->ShowLimitTips();
			}
			dwExp += dwExp * (lpPlayer->m_ExtraExpPercent * 0.01);
			lpPlayer->SendAddPlayerExp(dwExp, SAExpChangeMsg::MONSTERDEAD, "Killed Monster");
		}
	}

	// 增加真气
	//pKiller->ModifyCurrentSP(m_Property.m_Sp, 0, false);
}

void CMonster::_OnDead(CFightObject *PKiller)
{
	// 只有玩家可以攻击怪物（如果是玩家加了一个流血Buff后下线，那么PKiller可能为空）
	CPlayer *pPlayer = (CPlayer *)PKiller->DynamicCast(IID_PLAYER);

	CFightObject::OnDead(PKiller);

	ChangeFightSpeed(false);

	m_ActionFrame = (m_Property.m_ReBirthInterval/1000) * 5;
	if(!pPlayer)return;

	// 九字杀阵触发判定 --ahj--
	//SetNineWordsCondition(pPlayer);

	// 如果是boss，写入到人物结构，保存
	if (IsBoss())
	{
		++pPlayer->m_Property.m_KillBossNum;
	}

	if (!pPlayer)
	{
		CFightPet *pet=(CFightPet*)PKiller->DynamicCast(IID_FIGHT_PET);
		if (pet)
		{
			pPlayer=pet->m_owner;
		}
	}

	HandlePlayerInDis(pPlayer);	// 计算队伍中的玩家是否在有效范围内
	HandleExpWithTeam(pPlayer);	// 杀怪经验处理
	HandleGetSP(pPlayer);		// 杀怪真气奖励
	KillMonsterTask(pPlayer);	// 管理任务处理
	GetDropItem(pPlayer);		// 怪物掉落处理
	
	HandleExpWithObj(PKiller);//	侠客杀怪经验处理
	

	MY_ASSERT(0 == m_CurHp);

	if (pPlayer)
	{ 
		//pPlayer->RemoveFollow(this);
	}

	return;
}

CMonster::SHitData::SHitData()
{
    memset(this, 0, sizeof(SHitData));
    dwFristHitTime = dwLastHitTime = timeGetTime();
}

void CMonster::SetHit(DWORD GID, DWORD TeamID, DWORD Damage)
{
    SHitData *pHitData = &m_HitMap[GID];
    pHitData->dwDamage += Damage;
    pHitData->dwHitTimes ++;
    pHitData->dwTeamID = TeamID;
    pHitData->dwGID = GID;

    DWORD margin = timeGetTime() - pHitData->dwLastHitTime;
    pHitData->dwLastHitTime = timeGetTime();

    if (pHitData->dwTimeMargin < margin)
        pHitData->dwTimeMargin = margin;
}

void CMonster::UpdateHitMap()
{
    std::map<DWORD, SHitData>::iterator it = m_HitMap.begin();
    while (it != m_HitMap.end())
    {
        SHitData *pHitData = &it->second;

        if (timeGetTime() - pHitData->dwLastHitTime > 20000)
        {
            m_HitMap.erase(it);
            break;
        }

        it ++;
    }
}

bool CMonster::IsBoss(void)
{
	const SMonsterBaseData *pData = CMonsterService::GetInstance().GetMonsterBaseData(m_Property.m_ID);
	MY_ASSERT(pData);
	return pData->m_LevelType>MT_ELITE&&pData->m_LevelType<MT_MAX;
}

void RandomPos( CRegion *parentRegion, WORD &wDesPosX, WORD &wDesPosY, WORD wCenterX, WORD wCenterY, WORD wBRArea, DWORD dwSearchTimes )
{
    if ( parentRegion == NULL )
        return;

    RECT rcBRArea;          // 文件设置范围
    rcBRArea.left = (wCenterX > wBRArea) ? (wCenterX - wBRArea) : 0;
    rcBRArea.right = wCenterX + wBRArea;
    rcBRArea.top = (wCenterY > wBRArea) ? (wCenterY - wBRArea) : 0;
    rcBRArea.bottom = wCenterY + wBRArea;

    RECT rcRegion;          // 地图范围
    rcRegion.left = 0;
    rcRegion.right = parentRegion->m_RegionW;
    rcRegion.top = 0;
    rcRegion.bottom = parentRegion->m_RegionH;

    RECT rcAvaiRegion;
    if ( ( !::IntersectRect(&rcAvaiRegion, &rcBRArea, &rcRegion) ) || ( rcAvaiRegion.left == rcAvaiRegion.right && rcAvaiRegion.top == rcAvaiRegion.bottom ) )
    {
        rfalse(2, 1, "地图编号[%d]选取范围错误[fileLRTB]:%d, %d, %d, %d; [mapLRTB]:%d, %d, %d, %d;", 
            parentRegion->m_wRegionID ,rcBRArea.left, rcBRArea.right, rcBRArea.top, rcBRArea.bottom, rcRegion.left, rcRegion.right, rcRegion.top, rcRegion.bottom);
        return;
    }

    WORD wBRAreaX = (WORD)(( rcAvaiRegion.right - rcAvaiRegion.left ) >> 1);
    WORD wBRAreaY = (WORD)(( rcAvaiRegion.bottom - rcAvaiRegion.top ) >> 1);
    wCenterX = (WORD)(rcAvaiRegion.left + wBRAreaX);
    wCenterY = (WORD)(rcAvaiRegion.top + wBRAreaY);
    

    DWORD dwTempSearchTimes = 0;
    do 
    {
        ++dwTempSearchTimes;
        wDesPosX = 222/*GetRandomPos(wCenterX, wBRAreaX)*/;
        wDesPosY = 222/*GetRandomPos(wCenterY, wBRAreaY)*/;
    }
    // 存在阻挡且选择次数少于500
    while ( dwTempSearchTimes < dwSearchTimes &&  !parentRegion->isTileWalkAble(wDesPosX, wDesPosY) );

    if ( !parentRegion->isTileWalkAble(wDesPosX, wDesPosY) )     // 特殊处理，因为中心点是我们选的， 肯定不是阻挡
    {
        wDesPosX = wCenterX;
        wDesPosY = wCenterY;
    }
}

void CMonster::OnDamage(INT32 *nDamage, CFightObject *pFighter)
{
	// 当前的HP已经是0了
	if (0 == m_CurHp)
		return;

	//CMonster *monter=(CMonster*)pFighter->DynamicCast(IID_MONSTER);
	//if (monter)
	//	return;
	
	static int WearPos[EQUIP_P_MAX];
	
	CPlayer *attacker = (CPlayer *)pFighter->DynamicCast(IID_PLAYER);
	if (attacker)
	{
		// 减少攻击者武器耐久
		if (0 != attacker->m_Property.m_Equip[EQUIP_P_WEAPON].wIndex && 0 != attacker->m_Property.m_Equip[EQUIP_P_WEAPON].attribute.currWear)
		{
			const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(attacker->m_Property.m_Equip[EQUIP_P_WEAPON].wIndex);
			// 判断是否是永不磨损的武器
			if (0 != itemData->m_MaxWear)
				attacker->UpdateEquipWear(EQUIP_P_WEAPON, 1);
		}

		// 攻击者为玩家，被攻击者为怪物，且为已经杀了9个非精英怪的时候，触发九字杀阵
// 		if (9 == attacker->m_NineWordsKill && !IsBoss())
// 		{
// 			ModifyCurrentHP(-m_CurHp, 0, pFighter);
// 			return;
// 		}
	}

	// 查看伤害吸收
	/*if (m_SorbDamageValue)
		*nDamage -= m_SorbDamageValue;

	if (*nDamage < 0)
		*nDamage = 0;

	if (*nDamage && m_SorbDamagePercent)
		*nDamage -= (*nDamage * 100 / m_SorbDamagePercent);*/

	ModifyCurrentHP(-(*nDamage), 0, pFighter);

	if (0 != *nDamage)
		TriggerHpScriptEvent(*nDamage,pFighter);
		
	if (0 != m_CurHp)
	{
		//m_dwLastEnemyID = pFighter->GetGID();
	}

	if (pFighter)
	{
		m_Direction = GetDirection(m_curX, m_curY, pFighter->m_curX, pFighter->m_curY);
	}

	return;
}

void CMonster::SendPropertiesUpdate()
{
	CFightObject::SendPropertiesUpdate();

	memset(m_FightPropertyStatus, 0, sizeof(m_FightPropertyStatus));
}

void CMonster::OnRebound(int rebound, CFightObject* pFighter)
{
}

void CMonster::SwitchFightState(bool IsIn)
{
	return;
// 	if (IsIn)
// 		ChangeFightSpeed(true);
// 	else
// 	{
// 		RestoreFullHPDirectly();
// 		RestoreFullMPDirectly();
// 		ChangeFightSpeed(false);
// 	}
}

//----------------------------------------------------------------------------------------------------------	
bool CMonster::EndPrevActionByWoundState()
{
	/*
	if (!m_pAttackTarget)
	{
		CheckTarget();
	}

	// 受伤可能会造成触发AI脚本
	if(m_CurHp <= (m_dwMaxHP / 2)  &&  m_Property.m_wAIScriptID > 0)     
	{
		CTrigger trigger;
		trigger.m_dwType = CTrigger::TT_MONSTER_HALF_HP;
		trigger.SMonsterHalfHP.pMonster = this;
		trigger.SMonsterHalfHP.dwScriptID = m_Property.m_wAIScriptID;
		OnTriggerActivated(&trigger);
	} //*/

	return true;
}
//----------------------------------------------------------------------------------------------------------
bool CMonster::ProcessEndAttack()
{
	if (m_dwEndAttackTime != 0)
	{
		if ( (int)(m_dwEndAttackTime - timeGetTime()) <= 0 )
		{
			// 追击时间结束，不再追击目标
			m_dwEndAttackTime = 0;
			m_dwLastEnemyID = 0;
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------------------	
bool CMonster::EndPrevActionByStandState_DoAIByID2()
{
	/*
	if(m_bStopToStand )
	{
		if(!m_pAttackTarget)
			m_wIdleTime = 10 + rand() % 25;
		else
			m_wIdleTime = 1;
		m_bStopToStand = FALSE;
	}

	m_wIdleTime -- ;
	if(m_wIdleTime<=0)
	{
		Attack();

		if (!m_pAttackTarget)
		{
			SQSynWayTrackMsg WayMsg;
			int iXEnd = 0;
			int iYEnd = 0;
			int iRet = -1;
			MakeRadomPath(m_Property.m_wMoveArea,m_Property.m_wSrcX,m_Property.m_wSrcY,iXEnd,iYEnd);
			iRet = FindPath(iXEnd,iYEnd,WayMsg);
			{
				if(iRet>= 0)
				{
					SetMoveWay(&WayMsg);
					//add by yuntao.liu
					m_bIsMove2AttackObject = SQSynWayTrackMsg::normal;
				}
			}
		}
		else
		{
			SQAFightObjectMsg iMsg;
			iMsg.sa.dwGlobalID = GetGID();
			iMsg.sa.wPosX = m_wCurX;
			iMsg.sa.wPosY = m_wCurY;
			iMsg.sa.byWaitFrames = 0;
			iMsg.sa.byFightID = 0;
			iMsg.dwDestGlobalID = m_pAttackTarget->GetGID();
			SetAttackMsg(&iMsg);
		}

		m_bStopToStand = TRUE;
	}
	//*/

	return true;
}

//----------------------------------------------------------------------------------------------------------	
bool CMonster::EndPrevActionByStandState_DoAIByID1()
{
	/*
	if (m_bStopToStand)
	{
		m_bStopToStand = FALSE;
		m_wIdleTime = 10 + rand() % 25;
	}

	m_wIdleTime -- ;
	if(m_wIdleTime<=0)
	{
		SQSynWayTrackMsg WayMsg;
		int iXEnd = 0;
		int iYEnd = 0;
		int iRet = -1;
		MakeRadomPath(m_Property.m_wMoveArea,m_Property.m_BirthPosX << TILE_BITW,m_Property.m_BirthPosY << TILE_BITH, iXEnd,iYEnd);
		iRet = FindPath(iXEnd,iYEnd,WayMsg);
		{
			if(iRet>= 0)
			{
				SetMoveWay(&WayMsg);
				m_bIsMove2AttackObject = SQSynWayTrackMsg::normal;
			}
		}
		m_bStopToStand = TRUE;
	}//*/

	return true;
}

//----------------------------------------------------------------------------------------------------------	
bool CMonster::EndPrevActionByStandState_DoAIByID3Or4()
{
// 	if ( m_bStopToStand )
// 	{
// 		if(!m_pAttackTarget)
// 			m_wIdleTime = 5;
// 		else
// 			m_wIdleTime = 1;
// 		m_bStopToStand = FALSE;
// 	}
// 
// 	m_wIdleTime -- ;
// 	if(m_wIdleTime<=0)
// 	{
// 		Attack();
// 
// 		if ( m_pAttackTarget )
// 		{
// 			SQAFightObjectMsg iMsg;
// 			iMsg.sa.dwGlobalID = GetGID();
// 			iMsg.sa.wPosX = m_wCurX;
// 			iMsg.sa.wPosY = m_wCurY;
// 			iMsg.sa.byWaitFrames = 0;
// 			iMsg.sa.byFightID = 0;
// 			iMsg.dwDestGlobalID = m_pAttackTarget->GetGID();
// 			SetAttackMsg(&iMsg);
// 		}
// 
// 		m_bStopToStand = TRUE;
// 	}

	return true;
}

//----------------------------------------------------------------------------------------------------------	
bool CMonster::DoRenascenceScript()
{
	/*
	if(m_Property.m_wRefreshScriptID > 0)   
	{
		CTrigger trigger;
		trigger.m_dwType = CTrigger::TT_MONSTER_NEWLIFE;
		trigger.SMonsterNewLife.pMonster = this;
		trigger.SMonsterNewLife.dwScriptID = m_Property.m_wAIScriptID;
		OnTriggerActivated(&trigger);
	}
	else
	{
		SetRenascence(0, 0, m_dwMaxHP, 2);
	}//*/

	return true;
}
//----------------------------------------------------------------------------------------------------------	
bool CMonster::DeDeadEvent()
{
	SADelObjectMsg msg;
	msg.dwGlobalID = GetGID();
	if (m_ParentArea) 
		m_ParentArea->SendAdj(&msg, sizeof(msg), -1);

	if (m_dwDeadDelayScript)
	{
		extern LPIObject GetPlayerByGID(DWORD);
		if (CPlayer *pPlayer = (CPlayer *)GetPlayerByGID(m_dwKillerPlayerGID)->DynamicCast(IID_PLAYER))
		{
			CTrigger trigger;
			trigger.m_dwType = CTrigger::TT_MONSTERDEAD;
			trigger.SMonsterDead.dwScriptID = m_dwDeadDelayScript;
			trigger.SMonsterDead.pPlayer = pPlayer;
			trigger.SMonsterDead.pMonster = this;

			pPlayer->OnTriggerActivated(&trigger);
		}
	}

	m_bDeadEvent = FALSE;
	m_dwDeadDelayScript = 0;
	m_dwKillerPlayerGID = 0;
/*	m_fActionFrame = 20.0f;*/

	return true;
}

//----------------------------------------------------------------------------------------------------------	
bool CMonster::ProcessDeadWaitTime()
{
	/*
	if ( m_Property.m_dwBRTeamNumber == 0 || m_Property.m_wRefreshStyle == 1 )
	{
		m_DeadWaitTime --;
	}
	else if (m_Property.m_dwBRTeamNumber != 0
		&& m_Property.m_wRefreshStyle == 2 && m_ParentRegion != NULL)
	{
		if (m_Property.m_dwRefreshTime 
			&&  ( (m_ParentRegion->m_dwTickTime % m_Property.m_dwRefreshTime) >= 0 ) 
			&& ( (m_ParentRegion->m_dwTickTime % m_Property.m_dwRefreshTime) <= 19 ))
		{
			m_DeadWaitTime = 0;
		}
	}//*/

	return true;
}

//----------------------------------------------------------------------------------------------------------	
bool CMonster::EndPrevActionByDeadState()
{
// 	// 有重生脚本的话
// 	if (m_DeadWaitTime == 0)
// 	{
// 		DoRenascenceScript();    
// 	}
// 	else if (m_bDeadEvent)
// 	{
// 		DeDeadEvent();
// 	}
// 	else 
// 	{
// 		ProcessDeadWaitTime();
// 	}
// 
	return true;
}

bool CMonster::EndPrevActionByStandState()
{
	/*
	ProcessEndAttack();

	// 如果有攻击目标就向它攻击
	if (m_pAttackTarget != NULL)
	{
		AttackTarget();
	}
	// 否则就根据AI类型进行动作
	else 
	{
		switch (m_Property.m_wAIType)
		{
		case 2:
			{
				// 发呆
				EndPrevActionByStandState_DoAIByID2();
			}
			break;

		case 1:
			{
				// 随机走动
				EndPrevActionByStandState_DoAIByID1();
			}
			break;

		case 3:
		case 4:
			{
				// 主动攻击
				EndPrevActionByStandState_DoAIByID3Or4();
			}
			break;
		}
	}
	//*/

	return true;
}

//----------------------------------------------------------------------------------------------------------	
BOOL CMonster::EndPrevAction()
{
	/*
 	Parent::EndPrevAction();
 
 	switch (GetCurActionID())
 	{
 	case EA_STAND:
 		{
			switch (m_Property.m_wAIType)
			{
			case 2:
				{
					// 发呆
					EndPrevActionByStandState_DoAIByID2();
				}
				break;

			case 1:
				{
					// 随机走动
					EndPrevActionByStandState_DoAIByID1();
				}
				break;

			case 3:
			case 4:
				{
					// 主动攻击
					EndPrevActionByStandState_DoAIByID3Or4();
				}
				break;
			case 5:	//挂机流程(怪不攻击玩家,普通技能)
				{
					return true;
				}
				break;
			case 6://挂机处理(主动攻击,护体)
				{
					m_dwLastEnemyID = 0;
					EndPrevActionByStandState_DoAIByID3Or4();
				}
				break;
			}

 			if(m_dwLastEnemyID != 0)
 			{
				extern LPIObject GetPlayerByGID(DWORD GID);
				LPIObject object = GetPlayerByGID(m_dwLastEnemyID);

				CPlayer* player = (CPlayer*)(object->DynamicCast(IID_PLAYER));
				if(!player)
				{	
					m_dwLastEnemyID = 0;
				}
				else if(player->m_CurHp == 0)
 				{
					m_dwLastEnemyID = 0;
				}
				else
				{
					SQuestSkill_C2S_MsgBody msg;

					msg.mDefenderGID = m_dwLastEnemyID;
					msg.dwSkillIndex = 0;
					msg.mAttackWorldPosX = 0;
					msg.mAttackWorldPosY = 0;
					msg.mDefenderWorldPosX = 0;
					msg.mDefenderWorldPosY = 0;
// 					msg.mQuestTileX = -1;
// 					msg.mQuestTileY = -1;
					ProcessQuestSkill(&msg);
				}

				//m_dwLastEnemyID = 0;
 			}
 		}
 		break;
	case EA_WOUND:
		{
			EndPrevActionByWoundState();
		}

		break;

	case EA_DEAD:
		{
			EndPrevActionByDeadState();
		}

		break;
 	default:
 		{}
 		break;
 	}
	//*/

	return TRUE;
}

BOOL CMonster::DoCurAction()
{
	return Parent::DoCurAction();
}

bool CMonster::CreatePath(SQSynPathMsg& WayMsg, CFightObject *pLife)
{
	if (NULL == m_ParentRegion || NULL == pLife)
		return false;

	// 目前简单的处理方案
	WayMsg.m_GID = GetGID();
	WayMsg.m_Path.m_moveType = EA_RUN;
	WayMsg.m_Path.m_wayPoints = 2;
	WayMsg.m_Path.m_path[0].m_X = m_curX;
	WayMsg.m_Path.m_path[0].m_Y = m_curY;
	WayMsg.m_Path.m_path[1].m_X = pLife->m_curX;
	WayMsg.m_Path.m_path[1].m_Y = pLife->m_curY;

	return true;
}

int CMonster::CreatePath(SQSynWayTrackMsg& WayMsg,CFightObject* pLife)
{
// 	WayMsg.dwGlobalID = 0;
// 	ZeroMemory(&WayMsg.ssw, sizeof(WayMsg.ssw));
// 
// 	WayMsg.dwGlobalID = GetGID();
// 
// 	if (m_ParentRegion == NULL) 
// 		return 0;
// 
// 	if (!m_ParentRegion->isTileWalkAble(m_wCurX, m_wCurY))
// 	{
// 		return 0;
// 	}
// 
// 	int direction = GetDirection(m_wCurX,m_wCurY,pLife->m_wCurX,pLife->m_wCurY);
// 
// 	INT32 maxOffset = 1; //max(abs(pLife->m_wCurX - m_wCurX),abs(pLife->m_wCurY - m_wCurY));
// 
// 	for (int i = 0;i < 8;i ++)
// 	{
// 		direction &= 7;
// 
// 		INT32 len = 1;
// 		bool isFind = false;
// 
// 		while(
// 			m_ParentRegion->isTileWalkAble(
// 				m_wCurX + g_dir[ direction * 2 ] * len, 
// 				m_wCurY + g_dir[ direction * 2 + 1 ] * len
// 			)
// 			&&
// 			m_ParentRegion->isTileStopAble(
// 				m_wCurX + g_dir[ direction * 2 ] * len, 
// 				m_wCurY + g_dir[ direction * 2 + 1 ] * len,
// 				GetGID())	
// 			)
// 		{
// 			len ++;
// 			isFind = true;
// 			if(len >= maxOffset)
// 			{
// 				break;
// 			}
// 		}
// 
// 		if (isFind)
// 		{
// 			WayMsg.ssw.byAction = EA_WALK;
// 			WayMsg.ssw.wSegX = m_wCurX;
// 			WayMsg.ssw.wSegY = m_wCurY;
// 			WayMsg.ssw.Track[ 0 ].len = len;
// 			WayMsg.ssw.Track[ 0 ].dir = direction;
// 
// 			break;
// 		}
// 		direction ++;
// 	}
// 
// 	return WayMsg.ssw.Track[0].len;

	return 0;
}

bool CMonster::SendMove2TargetForAttackMsg(INT32 skillDistance, INT32 skillIndex, CFightObject *pLife)
{
	if (!m_ParentRegion || !pLife)
		return false;
	 
	// [2012-5-11 18-09 gw: + 定身状态下是不能移动去攻击的]
	if (m_fightState & FS_DIZZY || m_fightState & FS_JITUI)
	{ 
		//rfalse(2, 1, "定身状态下是不能移动去攻击的");
		return false;
	}

	if (skillIndex < 0) //来自玩家的通知追击的怪物，玩家已经改变了方向，如果不在追击状态直接返回
	{
		if (GetCurActionID() != EA_RUN )
		{
			return false;
		}
	}

	CPlayer *pPlayer = (CPlayer *)pLife->DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		SASynPathMonsterMsg  spathmonster;  //服务器要求客户端生成1条路径
		spathmonster.dwGlobalID = GetGID();
		spathmonster.m_path[0].m_X = m_curX;
		spathmonster.m_path[0].m_Y = m_curY;
		spathmonster.m_path[1].m_X = pPlayer->m_curX;
		spathmonster.m_path[1].m_Y = pPlayer->m_curY;
		g_StoreMessage(pPlayer->m_ClientIndex, &spathmonster, sizeof(SASynPathMonsterMsg));
	}
	if (skillIndex >= 0)
	{
		int nCuruseskill = GetSkillIDBySkillIndex(skillIndex);
		if (nCuruseskill >= 0)
		{
			m_wCuruseskill = nCuruseskill;
		}
	}
	WORD maxdistance = 200;
	WORD mindistance = 50;

	const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(m_wCuruseskill);
	if (pData)
	{
		maxdistance = pData->m_SkillMaxDistance;
		mindistance = pData->m_SkillMinDistance;
		skillDistance = maxdistance;
	}

	SQSynPathMsg WayMsg;

	// 首先计算距离
	D3DXVECTOR2 ver((m_curX - pLife->m_curX), (m_curY - pLife->m_curY));
	float distance = D3DXVec2Length(&ver);  //目标点与自己的距离
	//float distance = sqrt(pow((m_curX - pLife->m_curX), 2) + pow((m_curY - pLife->m_curY), 2));
	if (distance< skillDistance)
	{
		return true;
	}

/*	rfalse(2, 1, "Move2TargetForAttack %4.2f,%4.2f", m_curX, m_curY);*/

	float b = m_Property.m_WalkSpeed /2;  //200 毫秒每帧

	// 按照一定的步进单位逐步逼近（测试100）
	D3DXVECTOR2 curPos(m_curX, m_curY);
	D3DXVECTOR2 desPos(pLife->m_curX, pLife->m_curY);
	if (CreateRadomPoint(pLife,desPos,mindistance,maxdistance)== 0)
	{
		rfalse(2, 1, "CreateRadomPoint Faile");
	}
	
	//D3DXVECTOR2 desPos(pLife->m_curX, pLife->m_curY);
	D3DXVECTOR2 desVector = desPos - curPos;
	D3DXVec2Normalize(&desVector, &desVector);

	float fRadian = atan2(desVector.y, desVector.x);

	D3DXVECTOR2 vMovePos;
	vMovePos.x = m_curX + b * cos(fRadian);
	vMovePos.y = m_curY + b * sin(fRadian);
	float newlen = D3DXVec2Length(&(vMovePos - curPos)); //每次移动的距离



	WORD wnode = 1;
	while (1)
	{
		//distance = sqrt(pow((pLife->m_curX - DestPosX), 2) + pow((pLife->m_curY - DestPosY), 2));
		WayMsg.m_Path.m_path[wnode].m_X = vMovePos.x;
		WayMsg.m_Path.m_path[wnode].m_Y = vMovePos.y;

		//distance = sqrt(pow((m_BX - DestPosX), 2) + pow((m_BY - DestPosY), 2));
		distance = D3DXVec2Length(&(desPos - vMovePos));
		newlen = D3DXVec2Length(&(vMovePos - curPos));
		if (newlen >= distance ) //移动到超过目标点时
		{
			WayMsg.m_Path.m_path[wnode].m_X = desPos.x;   //最后1个点就是目标点了
			WayMsg.m_Path.m_path[wnode].m_Y = desPos.y;
			//rfalse(2, 1, "CreateRadomPoint id = %d , x = %f,y = %f ", GetGID(),desPos.x, desPos.y);
			break;
		}
// 		if (skillDistance >= distance)
// 		{
// 			//WayMsg.m_Path.m_path[wnode].m_X = desPos.x;
// 			//WayMsg.m_Path.m_path[wnode].m_Y = desPos.y;
// 			rfalse(2,1,"skillDistance = %d,distance = %f",skillDistance,distance);
// 			break;
// 		}

		wnode++;
		curPos = vMovePos;

		vMovePos.x = vMovePos.x + b * cos(fRadian);
		vMovePos.y = vMovePos.y + b * sin(fRadian);

		if (wnode+1 >= MAX_TRACK_LENGTH)
		{
			ver.x = pLife->m_curX - m_curX;
			ver.y = pLife->m_curY - m_curY;
			distance = D3DXVec2Length(&ver);
			rfalse("wnode to long Lenth = %f",distance);
			break;
		}
	}

	m_vdesPos->x = pLife->m_curX;
	m_vdesPos->y = pLife->m_curY;

// 	if (wnode > 2) //让怪物的坐标离开玩家2个点
// 	{
// 		wnode -= 2;
// 		m_vdesPos.x = WayMsg.m_Path.m_path[wnode].m_X;
// 		m_vdesPos.y = WayMsg.m_Path.m_path[wnode].m_Y;
// 	}



	//rfalse("SendMove2TargetForAttackMsg x = %f,y = %f,destX = %f,destY = %f", m_curX,m_curY,DestPosX,DestPosY);

	WayMsg.m_GID				= GetGID();
	WayMsg.m_move2What			= SQSynWayTrackMsg::move_2_attack;
	WayMsg.m_Path.m_moveType	= EA_RUN;
	WayMsg.m_Path.m_wayPoints	= wnode+1;
	WayMsg.m_Path.m_path[0].m_X = m_curX;
	WayMsg.m_Path.m_path[0].m_Y = m_curY;
// 	WayMsg.m_Path.m_path[1].m_X = pLife->m_curX;
// 	WayMsg.m_Path.m_path[1].m_Y = pLife->m_curY;
	
	//rfalse("MonsterGID =%d, MoveTime = %d", GetGID(),wnode * 200);
	// 如果路径成功生成，则移动过去攻击
	m_IsMove2Attack = SQSynPathMsg::attack;
	if(!SetMovePath(&WayMsg))
	{
		// 如果路径生成（设置）失败，那么表示出了问题，这时可以选择多种方案
		// 如：回到原点、原地不动等，此处暂时不处理，但是函数依然要返回true。
	}
	m_nMoveStartTime = GetTickCount64();
	ver.x = m_curX - pLife->m_curX;
	ver.y = m_curY - pLife->m_curY;
	distance = D3DXVec2Length(&ver);
	//rfalse(2, 1, "Move2TargetForAttack %4.2f,%4.2f ---%4.2f,%4.2f ", m_curX, m_curY, WayMsg.m_Path.m_path[wnode].m_X, WayMsg.m_Path.m_path[wnode].m_Y);
//	rfalse(2, 1, "Move2Target Target %4.2f,%4.2f", WayMsg.m_Path.m_path[wnode].m_X, WayMsg.m_Path.m_path[wnode].m_Y);
	return true;
}

INT32 CMonster::GetCurrentSkillLevel(DWORD dwSkillIndex)
{
	return 1;
}

INT32 CMonster::GetSkillIDBySkillIndex(INT32 index)
{
	//SSkill* pSkill = &m_pSkills[index];
	return (index < 0 || index >= MAX_MONTER_SKILLCOUNT)?-1:( (m_pSkills[index].byLevel == 0)?-1:m_pSkills[index].wTypeID);
}

void CMonster::OnChangeState(EActionState newActionID)
{
	if (newActionID == EA_STAND)
	{

	}
}

void CMonster::OnRunEnd()
{
	if (m_IsMove2Attack == SQSynWayTrackMsg::move_2_attack)
 	{
		// rfalse(2, 1, "怪物攻击移动完毕，当前坐标%f, %f", m_curX, m_curY);

 		m_backUpQusetSkillMsg = m_AtkContext;
 		__ProcessQuestSkill(&m_backUpQusetSkillMsg, true);
 	}
	else
	{
		if (m_BackProtection)
		{
			m_BackProtection = false;
			m_dwLastEnemyID	 = 0;
			m_Attacker.clear();
			
			// [2012-6-11 18-40 gw: +todo..] 如果当前攻击列表为空
			if (m_Attacker.empty())
				SwitchFightState(m_IsInFight = false);
		}

		// 进入站立状态
		//m_ActionFrame = 5/*CRandom::RandRange(100, 150)*/;
		m_ActionFrame = CRandom::RandRange(m_Property.m_StayTimeMin, m_Property.m_StayTimeMax);

	}
	//rfalse(2,1,"MonsterMoveEnd GID = %d,MoveTime = %d,MoveNode = %d", GetGID(), (GetTickCount64() - m_nMoveStartTime), m_NodeCount);
}

bool CMonster::CheckAddBuff(const SBuffBaseData *pBuff)
{
	if (m_BackProtection)	// 如果处于返回保护，无法附加Buff
	{
		rfalse(2, 1, "怪物处于保护状态，无法附加buff！！");
		return false;
	}

	return true;
}

void CMonster::ChangeFightSpeed(bool inFight)
{
	if (0 == m_Property.m_WalkSpeed)
		return;
	
	if (inFight && m_BaseSpeed == m_Property.m_FightSpeed) return;
	if (!inFight && m_BaseSpeed == m_Property.m_WalkSpeed) return;

	m_BaseSpeed = inFight ? m_Property.m_FightSpeed : m_Property.m_WalkSpeed;

	UpdateAllProperties();

	if (m_ParentArea)
	{
		SAMonsterWRChange msg;
		msg.bStatus		= inFight ? SAMonsterWRChange::SMWR_RUN : SAMonsterWRChange::SMWR_WALK;
		msg.dwGlobal	= GetGID();

		m_ParentArea->SendAdj(&msg, sizeof(msg), -1);
	}
}

void CMonster::UpdateAllProperties()
{
	WORD oldSpeed = m_OriSpeed;

	WORD wSpeed			= m_BaseSpeed;
	m_SorbDamageValue	= 0;
	m_SorbDamagePercent	= 0;

	m_MaxHp += GetBuffModifyValue(BAP_MHP, m_MaxHp);
	m_MaxMp += GetBuffModifyValue(BAP_MMP, m_MaxMp);
	m_MaxTp += GetBuffModifyValue(BAP_MTP, m_MaxTp);
	m_GongJi += GetBuffModifyValue(BAP_GONGJI, m_GongJi);
	m_FangYu += GetBuffModifyValue(BAP_FANGYU, m_FangYu);
	m_BaoJi += GetBuffModifyValue(BAP_BAOJI, m_BaoJi);
	m_ShanBi += GetBuffModifyValue(BAP_SHANBI, m_ShanBi);
	m_Hit += GetBuffModifyValue(BAP_HIT, m_Hit);

	m_SorbDamageValue	+= GetBuffModifyValue(BAP_SORBDAM,		0);
	m_SorbDamagePercent += GetBuffModifyValue(BAP_SORBDAM_PER,	0);
	wSpeed += GetBuffModifyValue(BAP_SPEED, m_BaseSpeed);
	m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_UNCRIT, m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_WRECK, m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_UNWRECK, m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_PUNCTURE, m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_UNPUNCTURE, m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT]);

	if (oldSpeed != wSpeed)
	{
		m_OriSpeed = wSpeed;
		//m_Speed = m_OriSpeed / 1000.0f;
		m_FightPropertyStatus[XA_SPEED] = true;
	}

	// 标记属性改变
	for (size_t i = 0; i < XA_MAX_EXP; i++)
		m_FightPropertyStatus[i] = true;
}

void CMonster::InitScriptEvent()
{
	///保存脚本中设置
	CScriptManager::TempCondition tCondition;
	g_Script.PushCondition(tCondition);

	///调用此脚本
	g_Script.SetCondition(this,0,0);
	//LuaFunctor(g_Script, FormatString("OnMonsterBirth%d",m_Property.m_ID))[m_Property.m_ID]();
	LuaFunctor(g_Script, "OnMonsterBirth")[m_Property.m_ID][m_dRankIndex]();
	g_Script.CleanCondition();

	//恢复脚本设置
	g_Script.PopCondition(tCondition);
	
	m_nBirthTime=GetTickCount64();
}

void CMonster::TriggerHpScriptEvent(int nDamage, CFightObject *pFighter)
{
	
	if (g_Script.PrepareFunction("OnObjectDamage"))
	{
		g_Script.PushParameter(GetGID());
		g_Script.PushParameter(nDamage);
		if (pFighter)
		{
			g_Script.PushParameter(pFighter->GetGID());
		}
		g_Script.Execute();
	}

}
#define NUM 100
void CMonster::HandleExpWithObj( CFightObject *pKiller )
{
	if (!pKiller) return;
	CFightPet *pFightPet=  (CFightPet *)pKiller->DynamicCast(IID_FIGHT_PET);
	if (!pFightPet)
	{
		CPlayer *player =  (CPlayer *)pKiller->DynamicCast(IID_PLAYER);
		if(player&&player->m_pFightPet)
		{
			pFightPet = player->m_pFightPet;
		}
	}
	if(!pFightPet)return;
	DWORD dwExp = GetExp(m_Property.m_Exp, pFightPet->m_Level, m_Level);
	//dwExp*=NUM;//测试100倍经验*/
	dwExp/=2;
	pFightPet->SendAddFightPetExp(dwExp, SAExpChangeMsg::MONSTERDEAD, "Killed Monster");

	//组队判断
// 	if(!pKiller->m_dwTeamID)
// 	{	//没有组队
// 		DWORD dwExp = GetExp(m_Property.m_Exp, pKiller->m_Property.m_Level, m_Level);
// 		pKiller->SendAddPlayerExp(dwExp, SAExpChangeMsg::MONSTERDEAD, "Killed Monster");
// 		if (pKiller->m_pFightPet)
// 		{	
// 
// 		}
// 	}
// 	else
// 	{
// 		for (size_t i=0; i<MAX_TEAM_MEMBER; ++i)
// 		{
// 			if(NULL == m_PlayerInDis[i]) continue;
// 
// 			CPlayer* lpPlayer = m_PlayerInDis[i];
// 			DWORD dwExp = GetExp(m_Property.m_Exp, lpPlayer->m_Property.m_Level, m_Level);
// 			//队伍个人所得经验＝（单独经验×（1＋（队伍人数－1）×3.5%））÷队伍人数
// 			dwExp =  dwExp * (1 + (m_dwPlayerInDisCount-1) * 0.035) / m_dwPlayerInDisCount;
// 			dwExp = (dwExp<1) ? 1 : dwExp;
// 			lpPlayer->SendAddPlayerExp(dwExp, SAExpChangeMsg::MONSTERDEAD, "Killed Monster");
// 		}
// 	}
}

BOOL CMonster::GetDropItem( DWORD itemID,INT32 itemNum )
{
	if (!m_ParentRegion )
		return FALSE;
	
	if (!g_Script.m_pAttacker)return FALSE;
	CPlayer *player=(CPlayer*)g_Script.m_pAttacker->DynamicCast(IID_PLAYER);
	if(!player)return FALSE;
	
	
	WORD Droproute = CRandom::RandRange(1, 100);
	if (player->limitedState == CPlayer::LIMITED_HALF){  //属于防沉迷时间，掉落物品概率减半
		player->ShowLimitTips();
		if (Droproute < 50) {
			return FALSE;
		}
	}
	else if (player->limitedState == CPlayer::LIMITED_ZERO) {	//属于疲劳时间，收益为0
		player->ShowLimitTips();
		return FALSE;
	}

	CItem::SParameter args;
	std::vector<SRawItemBuffer> item;


	SRawItemBuffer temp;
	BOOL rt = CItemUser::GenerateNewItem(temp, SAddItemInfo(itemID, itemNum));
	if (FALSE == rt)
		return FALSE;

	item.push_back(temp);

 	if (item.empty() && 0 == args.Money)	// 没东西可掉
		return FALSE;
// 	args.PackageModel	= IPM_BUDAI;
// 	args.dwLife			= MakeLifeTime(5);
// 	args.xTile			= m_curTileX;
// 	args.yTile			= m_curTileY;
// 	args.ItemsInPack	= item.size();
// 	args.ProtectedGID	= player->GetGID();
// 	args.ProtectTeamID	= player->m_dwTeamID;
// 
// 	if (!CItem::GenerateNewGroundItem(m_ParentRegion, 3, args, item, LogInfo(m_Property.m_ID, "特殊脚本怪物掉落")))
// 		return FALSE; 

	CSingleItem::SParameter Singleargs;
	if (args.Money)
	{
		SRawItemBuffer itemTemp;
		int moneyid = player->_L_GetLuaValue("OnGetMoneyItemID"); //判断是否为金钱道具
		if (!moneyid)return FALSE;

		BOOL rt = CItemUser::GenerateNewItem(itemTemp, SAddItemInfo(moneyid, args.Money)); //这里金钱作为一个特殊的道具处理
		if (FALSE == rt)
			return FALSE;

		Singleargs.PackageModel	= IPM_BUDAI;
		Singleargs.dwLife			= MakeLifeTime(5);
		Singleargs.xTile			= m_curTileX;
		Singleargs.yTile			= m_curTileY;
		Singleargs.ItemsNum = itemTemp.overlap;
		Singleargs.ItemsID = itemTemp.wIndex;
		Singleargs.ProtectedGID=player->GetGID();
		Singleargs.ProtectTeamID	= player->m_dwTeamID;

		if (!CSingleItem::GenerateNewGroundItem(m_ParentRegion, 3, Singleargs, itemTemp, LogInfo(m_Property.m_ID, "特殊脚本怪物掉落")))
			return FALSE; 
	}

	std::vector<SRawItemBuffer>::iterator iter = item.begin();
	while(iter != item.end())
	{
		SRawItemBuffer itemTemp = *iter;

		Singleargs.PackageModel	= IPM_BUDAI;
		Singleargs.dwLife			= MakeLifeTime(5);
		Singleargs.xTile			= m_curTileX;
		Singleargs.yTile			= m_curTileY;
		Singleargs.ItemsNum = itemTemp.overlap;
		Singleargs.ItemsID = itemTemp.wIndex;
		Singleargs.ProtectedGID=player->GetGID();
		Singleargs.ProtectTeamID	= player->m_dwTeamID;
		if (!CSingleItem::GenerateNewGroundItem(m_ParentRegion, 3, Singleargs, itemTemp, LogInfo(m_Property.m_ID, "特殊脚本怪物掉落")))
			return FALSE; 

		iter++;
	}

	return TRUE;
}

DWORD CMonster::GetRandomEnemyID()
{
	// 搜寻一个目标
	check_list<LPIObject> *playerList = m_ParentRegion->GetPlayerListByAreaIndex(m_ParentArea->m_X, m_ParentArea->m_Y);

	if (playerList)
	{
		size_t lenth=playerList->size();
		if(lenth==0)return 0;
		int ntemp=rand()%lenth;
		check_list<LPIObject>::iterator it = playerList->begin();
		while(ntemp--)it++;
		LPIObject target = *it;
		CPlayer *player = (CPlayer *)target->DynamicCast(IID_PLAYER);
		if (player && 0 != player->m_CurHp)
		{
			m_dwLastEnemyID = player->GetGID();	
			return m_dwLastEnemyID;
		}
			
	}
	return 0;
}

bool CMonster::SetStopTracing(EActionState action)//回到出生点
{
	float m_BX	= ((DWORD)m_Property.m_BirthPosX) << TILE_BITW;
	float m_BY	= ((DWORD)m_Property.m_BirthPosY) << TILE_BITH;
	m_BackProtection = true;
	SQSynPathMsg msg;
	//float distance = sqrt(pow((m_curX - m_BX), 2) + pow((m_curY - m_BY), 2));
	D3DXVECTOR2 ver((m_curX - m_BX), (m_curY - m_BY));
	float distance = D3DXVec2Length(&ver);
	float b = m_Property.m_WalkSpeed  ;  //200 毫秒每帧

	// 按照一定的步进单位逐步逼近（测试100）
	D3DXVECTOR2 curPos(m_curX, m_curY);
	D3DXVECTOR2 desPos(m_BX, m_BY);
	D3DXVECTOR2 desVector = desPos - curPos;
	D3DXVec2Normalize(&desVector, &desVector);

	float fRadian = atan2(desVector.y, desVector.x);

	D3DXVECTOR2 vMovePos;
	vMovePos.x = m_curX + b * cos(fRadian);
	vMovePos.y = m_curY + b * sin(fRadian);
	float newlen = D3DXVec2Length(&(vMovePos - curPos));

	WORD wnode = 1;
	while (1)
	{

		msg.m_Path.m_path[wnode].m_X = vMovePos.x;
		msg.m_Path.m_path[wnode].m_Y = vMovePos.y;

		//distance = sqrt(pow((m_BX - DestPosX), 2) + pow((m_BY - DestPosY), 2));
		distance = D3DXVec2Length(&(desPos - curPos));
		newlen = D3DXVec2Length(&(vMovePos - curPos));
		if (newlen >= distance)
		{
			msg.m_Path.m_path[wnode].m_X = desPos.x;
			msg.m_Path.m_path[wnode].m_Y = desPos.y;
			break;
		}


		wnode++;
		curPos = vMovePos;

		vMovePos.x = vMovePos.x + b * cos(fRadian);
		vMovePos.y = vMovePos.y + b * sin(fRadian);


		if (wnode+1 >= MAX_TRACK_LENGTH)
		{
// 			ver.x = m_curX - m_BX;
// 			ver.y = m_curY - m_BY;
			//distance = D3DXVec2Length(&ver);
			//distance = sqrt(pow((m_curX - m_BX), 2) + pow((m_curY - m_BY), 2));
			rfalse("wnode to long distanceLeft = %f",distance);
			//m_BackProtection = false;
			wnode = MAX_TRACK_LENGTH - 1;
			msg.m_Path.m_path[wnode].m_X = m_BX;
			msg.m_Path.m_path[wnode].m_Y = m_BY;
			break;
		}
	}

	

	msg.m_GID		= GetGID();
	msg.m_move2What	= SQSynPathMsg::normal;
	msg.m_Path.m_wayPoints		= wnode + 1;
	msg.m_Path.m_moveType		= action;
	msg.m_Path.m_path[0].m_X	= m_curX;
	msg.m_Path.m_path[0].m_Y	= m_curY;
	m_IsMove2Attack = SQSynPathMsg::normal;

	//rfalse(2,1,"BackToBirth %f,%f",m_BX,m_BY);
	// 如果设置路径失败，则瞬移过去
	if (!SetMovePath(&msg))
	{
		// 咋个瞬移喃？
		MoveToBirth();
	}
	else
	{
		m_IsMove2Attack  = SQSynPathMsg::normal;
		m_BackProtection = true;
	}

	m_nMoveStartTime = GetTickCount64();
	ver.x = m_curX - m_BX;
	ver.y = m_curY - m_BY;
	distance = D3DXVec2Length(&ver);
	//rfalse("MonsterMoveStart GID = %d,MoveTime = %d,MoveNode = %d,Lenth = % f", GetGID(), (wnode * 200), wnode+1, distance);
	//if (m_pFightFollow)
	//{
		//m_pFightFollow->RemoveFollow(this);
	//}

	m_dwLastEnemyID = 0;

	return true;
}

bool CMonster::MoveToBirth()
{
	//rfalse("MoveToBirth");
	MY_ASSERT(m_ParentRegion);
	return m_ParentRegion->Move2Area(this->self.lock(),m_Property.m_BirthPosX,m_Property.m_BirthPosY,true);
}

// 私有函数：九字杀阵条件判断
void CMonster::SetNineWordsCondition(CPlayer* player)
{
	if(player == NULL) 
		return;

	if (!IsBoss())
	{
		++player->m_NineWordsKill;
		//TalkToDnid(player->m_ClientIndex, FormatString("【九字杀阵】第%d个怪物", player->m_NineWordsKill));
		SANineWordMsg msg;
		msg.num = player->m_NineWordsKill;

		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SANineWordMsg));
	}

	if (10 == player->m_NineWordsKill)
	{
		TalkToDnid(player->m_ClientIndex, "临、兵、斗、者、皆、阵、列、前、行！杀！");
		
		//DWORD gid = this->GetGID();
		
		//MY_ASSERT(gid);
		SANineWordMsg msg;
		msg.num = player->m_NineWordsKill;
		//msg.gid = gid;

		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SANineWordMsg));
		
		player->m_NineWordsKill = 0;
	}
	
	return;
}
 
void CMonster::OnRun()
{
	if(m_AIParameter.wAIID != 0)
	{	//AI脚本控制的怪物
		SendPropertiesUpdate();
		ProcessScript();
		Parent::OnRun();
	}
	else
	{	//普通怪物
		SendPropertiesUpdate();
		//Thinking();
		if ( m_StaticpreUpdateTime == 0 )
		{
			g_Script.SetCondition( this, NULL, NULL);
			LuaFunctor(g_Script, FormatString("StaticScene_Monster_Init"))[m_Property.m_StaticAI][GetGID()]();
			g_Script.CleanCondition();
			m_StaticpreUpdateTime = ::timeGetTime();
		}
		else
		{   
			g_Script.SetCondition( this, NULL, NULL);
			LuaFunctor(g_Script, FormatString("StaticScene_Monster_Update"))[m_Property.m_StaticAI][GetGID()]();
			g_Script.CleanCondition();
		}
		Parent::OnRun();
	}
}

void CMonster::OnClose()
{
	g_Script.SetCondition(this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("StaticScene_Monster_End"))[m_Property.m_StaticAI][GetGID()]();
	g_Script.CleanCondition();

// 	if(m_AIParameter.wAIID != 0)
// 	{
// 		g_Script.SetCondition( this, NULL, NULL );
// 		LuaFunctor(g_Script, FormatString( "Monster_End_%d", m_AIParameter.wAIID ))[m_ParentRegion->GetGID()][GetGID()][m_AIParameter.wListID]();
// 		g_Script.CleanCondition();
// 	}
// 	else
// 	{
// 		g_Script.SetCondition( this, NULL, NULL );
// 		LuaFunctor(g_Script, FormatString( "StaticScene_Monster_End", m_Property.m_StaticAI))[GetGID()]();
// 		g_Script.CleanCondition();
// 	}
	return;
}

void CMonster::OnDisppear()
{
	m_buffMgr.CheckDelBuffSpot(0x00000004);
	SAPlayerDeadMsg DeadMsg;
	DeadMsg.dwGID = GetGID();
	DeadMsg.killGID =  0;
	if (m_ParentArea)
		m_ParentArea->SendAdj(&DeadMsg, sizeof(DeadMsg), -1);

	SetCurActionID(EA_DEAD);
	SetBackupActionID(EA_DEAD);
}

int CMonster::OnCreateEnd()
{
	return 1;
}

void CMonster::UpdateMyData( WORD reginID,WORD wHour,WORD wMinute,char *killmeName,WORD mystate )
{
	BossDeadData bossdata;
	bossdata.m_ID = m_Property.m_ID;
	bossdata.m_level = m_Property.m_level;
	bossdata.reginid= reginID;				//场景ID
	bossdata.wX= m_Property.m_BirthPosX;					//场景X	
	bossdata.wY= m_Property.m_BirthPosY	;					//场景Y
	bossdata.wHour =  wHour;			//几点
	bossdata.wMinute=  wMinute;		
	bossdata.wState = mystate;
	bossdata.m_UpdateTime = timeGetTime();
	dwt::strcpy(bossdata.m_KillMyName, killmeName, CONST_USERNAME);
	GetGW()->UpdateBossData(bossdata);
}

void CMonster::OnRecvPath(SQSynPathMonsterMsg *pMsg)
{
	SQSynPathMsg WayMsg;
	WayMsg.m_GID = GetGID();
	WayMsg.m_move2What = SQSynWayTrackMsg::move_2_attack;
	WayMsg.m_Path.m_moveType = EA_RUN;
	WayMsg.m_Path.m_wayPoints = pMsg->m_Path.m_wayPoints;
	for (WORD i = 0; i < pMsg->m_Path.m_wayPoints; i++)
	{
		WayMsg.m_Path.m_path[i] = pMsg->m_Path.m_path[i];
	}
	if (WayMsg.m_Path.m_wayPoints > 1)
	{
		m_curX = WayMsg.m_Path.m_path[0].m_X;
		m_curY= WayMsg.m_Path.m_path[0].m_Y;
		SetMovePath(&WayMsg);
	}
}

int CMonster::CreateRadomPoint(CFightObject* pLife, D3DXVECTOR2 &vtargetPos, WORD mindistance, WORD maxdistance)
{
	if (!pLife)
	{
		return 0;
	}
	D3DXVECTOR2 curPos(m_curX, m_curY);
	D3DXVECTOR2 desPos(pLife->m_curX, pLife->m_curY);
	D3DXVECTOR2 desVector = curPos - desPos;
	D3DXVec2Normalize(&desVector, &desVector);
	
	float fRadian = atan2(desVector.y, desVector.x); //当前方向
	int nRadian = CRandom::RandRange(fRadian - D3DX_PI / 2, fRadian + D3DX_PI/2);  //当前方向随机左右90，分开
	int randdistance = CRandom::RandRange(mindistance, maxdistance);

	vtargetPos.x = pLife->m_curX + randdistance * cosf(nRadian);
	vtargetPos.y = pLife->m_curY + randdistance * sinf(nRadian);
	//rfalse(2, 1, "CreateRadomPoint TargetPosX = %4.2f, TargetPosY = %4.2f",vtargetPos.x,vtargetPos.y);
	return 1;
}

float CMonster::GetWalkSpeed()
{
	return m_Property.m_WalkSpeed;
}
