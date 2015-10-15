#include "StdAfx.h"
#include <map>
#include "monster.h"
#include "gameworld.h"
#include "area.h"
#include "item.h"
#include "region.h"
#include "Player.h"
#include "Trigger.h"
#include "ScriptManager.h"
extern CScriptManager g_Script;

#include "networkmodule/movemsgs.h"
#include "networkmodule/regionmsgs.h"
#include "networkmodule/upgrademsgs.h"

#include "区域服务器/Environment.h"
#include "DSpcialItemData.h"


//侠义世界  装备掉落随机属性相关  ab_yue

WORD	*g_wAttNumOdds;									//装备随机属性的个数（百分率，除 100）		
SEquipAttTypeOddsData		*g_sAttTypeOdds;			//装备随机每种属性出现几率（百分率，除 100）
SEquipAttValueOddsData		*g_sAttValueOdds;			//装备每种属性取每个值的几率(万分率，除 10000)
SEquipAttNumRuleData		*g_sAttNumRule;				//装备属性条数规则数据

ITEMDROPIDTABLE g_vItemDropIdTable;		//掉落物品id表,所有掉落表数据结构
ITEMDROPIDTABLE g_vTaskDropIdTable;		//掉落物品id表,所有掉落表数据结构

//end


struct SDropTable
{
	BYTE byNumber;
	BYTE curIndex;
	WORD wIndex[256];
};

static BYTE s_byDropTableMax = 0;
static SDropTable s_pDropTable[256];

LPCSTR FormatString(LPCSTR szFormat, ...);

void LoadDropItemData()
{
	memset(s_pDropTable, 0, sizeof(SDropTable) * 256);

	static char str[128];
	dwt::ifstream stream("Data\\item\\droptable\\droptable.def");
	if (!stream.is_open())
	{
		rfalse(0, 0, "无法打开[Data\\item\\droptable\\droptable.def]道具掉落数据列表文件！");
		return; 
	}

	int i = 0;
	while (!stream.eof() && (i < 256))
	{
		stream >> str;
		std::string fname( "Data\\item\\droptable\\" );
		fname += str;
		dwt::ifstream tfile( fname.c_str() );
		if (!tfile.is_open())
		{
			rfalse(0, 0, "无法打开[%s]道具掉落数据列表文件！", fname.c_str());
		}
		else
		{
			int i2 = 0;
			while (!tfile.eof() && (i2 < 256))
			{
				tfile >> str;

				s_pDropTable[i].wIndex[i2] = atoi(str);

				i2++;
			}

			tfile.close();

			s_pDropTable[i].byNumber = i2;
		}

		i++;
	}

	s_byDropTableMax = i;

	stream.close();
}

struct GlobalDropTable
{
	DWORD tableId;

	struct DropUnit
	{
		void init()
		{
			percentType = 1; // 如果只写了倒落物品名，而没有写暴率，视为暴率为0
			percent = 0;
			dropType = 0;
			group = 0;
			bind = 0;
			dropId = 0;
			killCountCur = 0;
			count = 0;
			attenuation = 0;
		}

		BYTE  dropType;     // 掉落类型
		BYTE  percentType;  // 百分比类型，是概率还是累计计数
		BYTE  group;        // 掉落分组，为0时不判断
		BYTE  bind;         // 掉落后是否为绑定道具。。。
		BYTE  attenuation;  // 衰减类型 1 ：在衰减之外，0：普通
		DWORD dropId;       // 掉落值
		WORD  percent;      // 百分比值，由于是用rand函数，所以最大值就只能到RAND_MAX（32767）...
		WORD  killCountCur; // 当前击杀次数
		DWORD count;        // 掉落个数，不会大于单组最大值！
	};

	std::list< DropUnit > dropList;
};

// size = 32byte
struct ItemDropRcd
{
	ItemDropRcd() { memset( this, 0, sizeof( *this ) ); }

	// 前8个字节正好组成一个QWORD的查询关键字！
	struct Key64 {
		union {
			struct {
				WORD mapId;     // 地图编号
				WORD regionId;  // 场景编号（如果为动态场景，则场景编号为0）
				WORD monsterId; // 产出怪物编号
				WORD itemId;    // 产出道具编号
			};
			QWORD key;
		};
	};

	DWORD isScript; // 是否是脚本，这个比较特殊。。。
	DWORD white, blue, green, purple, orange; // 5种颜色的产出数量比！
};

extern std::map< std::string, DWORD > itemNameMap;          // 道具名-ID对照表
static std::map< DWORD, GlobalDropTable > globalDropTable;  // 全局掉落表
static std::vector< SDropTable > static_DropArray;          // 特殊掉落表
static std::map< std::string, DWORD > static_DropTableIdx;  // 读取特殊掉落表时的临时表数据
static std::map< QWORD, ItemDropRcd > static_DropRcd;       // 掉落表的日志数据！

void SetRandAttributes( DWORD level, SEquipment &equip );
//void GetAddAttribute( SEquipment::Attribute::Unit *unit, WORD id, int maxNum, WORD coff[ 57 ] = NULL, BYTE level = 0, BYTE itemType = 0 );

const char *tolwr( LPCSTR name )
{
	static char tempstr[1024];
	LPSTR iter = tempstr;
	for ( ; *name != 0; iter ++, name ++ )
	{
		if ( *name < 0 && name[1] != 0 )
			*( LPWORD )iter++ = *( LPWORD )name++;
		else
			*iter = tolower( *name );
	}
	*iter = 0;
	return tempstr;
}

DWORD LoadDropSubTable( LPCSTR name )
{
	std::string str( tolwr( name ) );
	std::map< std::string, DWORD >::iterator it = static_DropTableIdx.find( str );
	if ( it != static_DropTableIdx.end() )
		return it->second;

	std::string tempName = "data\\item\\subtableex\\" + str + ".txt";
	dwt::ifstream stream( tempName.c_str() );
	if ( !stream.is_open() )
		return MessageBox( 0, tempName.c_str(), "指定文件打不开！", 0 ), 0xffffffff;

	SDropTable dt;
	memset( &dt, 0, sizeof( dt ) );
	std::string temp;
	char szTemp[1024];
	while ( !stream.eof() && dt.byNumber < 255 )
	{
		stream >> szTemp;
		temp = szTemp;
		if ( temp.length() == 0 )
			break;

		if ( temp[0] == '#' )
			dt.wIndex[dt.byNumber++] = atoi( temp.c_str() + 1 );
		else if ( temp[0] == '$' )
		{
			std::map< std::string, DWORD >::iterator it = itemNameMap.find( temp.c_str() + 1 );
			if ( it != itemNameMap.end() )
				dt.wIndex[dt.byNumber++] = (WORD)it->second;
			else
			{
				rfalse( 2, 1, "道具掉落子表[%s]中的[%s]不是有效的道具名！", tempName.c_str(), temp.c_str() + 1 );
				dt.wIndex[dt.byNumber++] = 0xffff;
			}
		}
	}

	if ( dt.byNumber == 0 )
		return 0xffffffff;

	if ( dt.byNumber >= 254 )
		rfalse( 2, 1, "道具掉落子表[%s]中的道具太多了。。。！", tempName.c_str() );

	// 初始化随机！
	dt.curIndex = rand() % dt.byNumber;

	static_DropArray.push_back( dt );
	return (DWORD)static_DropArray.size() - 1;
}
 
BOOL LoadGlobalDrop( std::string strDropFileName = "" )
{
	dwt::ifstream stream( ( strDropFileName == "" ) ? ( "data\\item\\全局掉落表.txt" ) : strDropFileName.c_str() );
	if ( !stream.is_open() )
		return MessageBox( 0, "全局道具掉落表未打开！", 0, 0 ), false;

	globalDropTable.clear();
	static_DropArray.clear();
	static_DropTableIdx.clear();

	// zeb 2009.11.19,
	//  char lineBuffer[10240];
	//  while ( !stream.eof() )
	//  {
	//      stream.getline( lineBuffer, 10240 );
	//      std::strstream line( lineBuffer, strlen( lineBuffer ), ios_base::in );
	//      int tableId = 0;
	//      line >> tableId;
	//      if ( tableId == 0 )
	//          continue;

	//      GlobalDropTable &dropTable = globalDropTable[tableId];
	//      dropTable.tableId = tableId;
	//      GlobalDropTable::DropUnit dropUnit;
	//      std::string temp;
	//      //memset( &dropUnit, 0, sizeof( dropUnit ) );
	//dropUnit.init();
	//      while ( !line.eof() )
	//      {
	//	temp.clear();
	//          line >> temp;
	//          if ( temp.length() == 0 )
	//              break;

	//          // 是否是起始数据
	//          DWORD dropType = 0;
	//          switch ( temp[0] )
	//          {
	//          case '?':
	//              dropType++;
	//          case '%':
	//              dropType++;
	//          case '$':
	//              dropType++;
	//          case '#':
	//              dropType++;

	//              // 如果已经填充数据了，则需要将这个数据添加到表里面！
	//              if ( dropUnit.dropType )
	//              {
	//                  if ( dropUnit.dropId != 0xffffffff )
	//                      dropTable.dropList.push_back( dropUnit );
	//                  //memset( &dropUnit, 0, sizeof( dropUnit ) );
	//			dropUnit.init();
	//              }

	//              // 准备新数据
	//              dropUnit.dropType = dropType;
	//              if ( dropType == 2 ) // 特殊处理，道具名字！
	//              {
	//                  std::map< std::string, DWORD >::iterator it = itemNameMap.find( temp.c_str() + 1 );
	//                  if ( it != itemNameMap.end() )
	//                      dropUnit.dropId = it->second;
	//                  else
	//                  {
	//                      rfalse( 2, 1, "道具掉落表中的[%s]不是有效的道具名！", temp.c_str() + 1 );
	//                      dropUnit.dropId = 0xffffffff;
	//                  }
	//              }
	//              else if ( dropType == 4 )
	//              {
	//                  dropUnit.dropId = LoadDropSubTable( temp.c_str() + 1 );
	//                  if ( dropUnit.dropId == 0xffffffff ) 
	//                      return MessageBox( 0, lineBuffer, temp.c_str(), 0 ), false;
	//              }
	//              else if ( !isdigit( temp[1] ) )
	//                  return MessageBox( 0, lineBuffer, "全局道具掉落表有错[1]！", 0 ), false;
	//              else
	//                  dropUnit.dropId = atoi( temp.c_str() + 1 );
	//              break;

	//          case '&':
	//              if ( !isdigit( temp[1] ) )
	//                  return MessageBox( 0, lineBuffer, "全局道具掉落表有错[4]！", 0 ), false;

	//              dropUnit.group = atoi( temp.c_str() + 1 );
	//              if ( dropUnit.group == 0 || dropUnit.group > 10 )
	//                  return MessageBox( 0, lineBuffer, "分组不能大于10或等于0！", 0 ), false;
	//              break;

	//          case '*':
	//              if ( !isdigit( temp[1] ) )
	//                  return MessageBox( 0, lineBuffer, "全局道具掉落表有错[5]！", 0 ), false;

	//              dropUnit.count = atoi( temp.c_str() + 1 );
	//              break;

	//          case '@':
	//              dropUnit.bind = 1; // 这个道具掉出来后需要设为绑定状态。。。
	//              break;
	//
	//          case '~':
	//              dropUnit.attenuation = 1;
	//              break;

	//          default:
	//              if ( dropUnit.dropType && dropUnit.percentType == 1 || isdigit( temp[1] ) )
	//              {
	//                  size_t pos = temp.find_first_of( '/' );
	//                  if ( pos == temp.npos )
	//                      return MessageBox( 0, lineBuffer, "全局道具掉落表有错[2]！", 0 ), false;

	//                  int first = atoi( temp.substr( 0, pos ).c_str() );
	//                  int second = atoi( temp.c_str() + pos + 1 );
	//                  if ( first <= second )
	//                  {
	//                      dropUnit.percentType = 1;
	//                      dropUnit.percent = first * RAND_MAX / second;
	//                  }
	//                  else
	//                  {
	//                      dropUnit.percentType = 2;
	//                      dropUnit.percent = first;
	//                  }
	//              }
	//              else
	//                  return MessageBox( 0, lineBuffer, "全局道具掉落表有错[3]！", 0 ), false;
	//          }
	//      }

	//      if ( dropUnit.dropType && dropUnit.dropId != 0xffffffff )
	//          dropTable.dropList.push_back( dropUnit );
	//  }
	// zeb 2009.11.19
	return true;
}

// 构造静态掉落范围表! 16*16大小！
static POINT dropMap[1024];
static BOOL MakeDropMap();
BOOL static_init_MakeDropMap = MakeDropMap();
static BOOL MakeDropMap()
{
	dropMap[0].x = dropMap[0].y = 0;
	int index = -1, dir = -1, levelMax = 0, x = 0, y = 0;
	for ( int i = 0; i < 1000; i ++ )
	{
		index ++;
		if ( index >= 1000 )
			break;

		if ( index >= levelMax * 4 )
		{
			levelMax ++;
			y -= 1;
			index = 0;
			dropMap[i+1].x = x;
			dropMap[i+1].y = y;
		}
		else if ( levelMax )
		{
			dir = 1 + ( index / levelMax ) * 2;

			extern POINT vdir[8];
			dropMap[i+1].x = ( x += vdir[ dir ].x );
			dropMap[i+1].y = ( y += vdir[ dir ].y );
		}
	}
	return levelMax;
}

/**
天生附加			资质附加			掉落附加			颜色
普通装备    没有(0)				没有(0)				没有(0)				白色(0)

极品装备    没有					没有					有(1)				蓝色(1)

极品装备    有(2)				没有					没有					绿色(2)
极品装备    有(2)				没有					有(1)				绿色(3)

资质装备    没有					有(4)				没有					紫色(4)

天赋装备    有(2)				有(4)				没有					橙色(6)
天赋装备    有(2)				未鉴定(3)			没有					橙色(5)
*/

DWORD GetEquipmentColor( SEquipment &e )
{
// 	BOOL devAddon = false;                              // 是否存在资质附加
// 	BOOL dropAddon = false;                             // 是否存在掉落附加
// 	BOOL createAddon = e.attribute.naturals.type != 0;  // 是否存在天生附加
// 	DWORD colorValue = ( createAddon ? 2 : 0 );
// 	SEquipment::Attribute::Unit *it = e.attribute.qualitys;
// 	for ( int i = 0; i < 6; i ++ )
// 	{
// 		if ( it[i].type == 0 )
// 			continue;
// 
// 		// 如果isDevelop无值，表示qualitys是掉落附加属性，可能为白板
// 		// 如果isDevelop有值，表示qualitys是鉴定资质属性，如果为白板，表示未鉴定！
// 		*( e.isDevelop ? &devAddon : &dropAddon ) = true;
// 		break;
// 	}
// 
// 	if ( dropAddon )
// 		colorValue += 1;
// 	else if ( e.isDevelop )
// 		colorValue += devAddon ? 4 : 3;

	return 0;
}

BOOL DropToGround( SRawItemBuffer &itemDrop, DWORD protectedGID, DWORD protectedTeamID, POINT pos, CRegion *region, DWORD &dropCount )
{
	/*
	CItem::SParameter param;
	ZeroMemory( &param, sizeof( param ) );
	param.dwLife = MakeLifeTime( 5 );
	param.pItem = &itemDrop;
	param.dwProtectedGID = protectedGID;
	param.ProtectTeamID = protectedTeamID;

	do {
		if ( dropCount >= 1000 )
		{
			param.xTile = (WORD)pos.x;
			param.yTile = (WORD)pos.y;
			break;
		}

		param.xTile = WORD(pos.x + dropMap[dropCount].x);
		param.yTile = WORD(pos.y + dropMap[dropCount].y);
		dropCount ++;
		// 需要确认目标点可以放道具！
	} while ( !region->isTileWalkAble( param.xTile, param.yTile ) );

	return region->Move2Area( region->GenerateObject( IID_ITEM, 0, (LPARAM)&param ), param.xTile, param.yTile, true );
	//*/

	return FALSE;
}

//
BOOL DropNewItemToGround( SRawItemBuffer &itemRcd, DWORD itemId, DWORD count, DWORD protectedGID, 
						 DWORD protectedTeamID, POINT pos, CRegion *region, DWORD &dropCount, CMonster *monster, BOOL bind )
{
	/*
	if( !monster )
		return FALSE;

	SGroundItem item;
	memset( &item, 0, sizeof( item ) );
	CItem::SParameter param;
	ZeroMemory( &param, sizeof( param ) );
	param.dwLife = MakeLifeTime( 5 );
	param.dwProtectedGID = protectedGID;
	param.ProtectTeamID = protectedTeamID;

	do {
		if ( dropCount >= 1000 )
		{
			param.xTile = (WORD)pos.x;
			param.yTile = (WORD)pos.y;
			break;
		}

		param.xTile = WORD(pos.x + dropMap[dropCount].x);
		param.yTile = WORD(pos.y + dropMap[dropCount].y);
		dropCount ++;
		// 需要确认目标点可以放道具！
	} while ( !region->isTileWalkAble( param.xTile, param.yTile ) );

	// 随机装备孔的设定，1/20的几率出现孔设置，孔设置中1/3的几率1~2个孔
	const SItemData *itemData = CItem::GetItemData( itemId );

	// 新加特殊物品属性处理
	DWORD slotNumber = 0;
	SpecialItem* sItem = CItem::GetSpecialItem( itemId );
	if ( sItem && ITEM_IS_EQUIPMENT( itemData->byType ) )
	{
		// 0. 加洞
		if( IN_ODDSS( sItem->mHoleAdd[0] ) )
		{
			DWORD v = ( sItem->mHoleAdd[2] - sItem->mHoleAdd[1] );
			if ( v == 0 ) v = 1;
			slotNumber = ( rand() % v ) + sItem->mHoleAdd[1];
		}
	}

	// 创建道具到场景地表！
	param.pItem = &item;
	if ( !GenerateNewItem( item, GenItemParams( (WORD)itemId, count, (WORD)slotNumber ), LogInfo( 2, "怪物死亡掉落" ) ) )
		return FALSE;

	// 设置绑定信息
	item.flags = bind;

	if ( sItem && ITEM_IS_EQUIPMENT( itemData->byType ) )
	{
		// SetRandAttributes( 100, *reinterpret_cast< SEquipment* >( param.pItem ) );
		SEquipment *equip = (SEquipment*)( ( SItemBase* )&item );

		// 1. 附加等级，最大15
		equip->level = 1;
		if( IN_ODDSS( sItem->mLevelAdd[0]) )
		{
			DWORD v = ( sItem->mLevelAdd[2] - sItem->mLevelAdd[1] );
			if ( v == 0 ) v = 1;
			equip->level = ( rand() % v ) + sItem->mLevelAdd[1];
		}
		if ( equip->level > 9 ) 
			equip->level = 9;

		// 2. 强化附加关联标识
		for( int i = 0; i < 3; i ++ )
			GetAddAttribute( &equip->attribute.activates[i], (WORD)sItem->mIntensifyAdd[i], 1 );

		// 3. 天生附加
		if ( IN_ODDSS( sItem->mInnetenessAddOddsAndFlag[0] ) )
			GetAddAttribute( &equip->attribute.naturals, (WORD)sItem->mInnetenessAddOddsAndFlag[1], 1 );

		// 4. 资质鉴定
		if( IN_ODDSS( sItem->mAptitudeCheckupOdds[0] ) )
			equip->isDevelop = 1;
		else
		{	
			// 4.1掉落附加
			equip->isDevelop = 0;
			if( IN_ODDSS( sItem->mDropAddsOddsAndFlag[0]) )
				GetAddAttribute( equip->attribute.qualitys, (WORD)sItem->mDropAddsOddsAndFlag[1], 6 );
		}
	}

	// 给外部返回新创建的道具数据！
	itemRcd = item;

	return region->Move2Area( 
		region->GenerateObject( IID_ITEM, 0, (LPARAM)&param ), 
		param.xTile, param.yTile, true );
	//*/

	return FALSE;
}

BOOL CheckPercent( GlobalDropTable::DropUnit &dropUnit )
{
	if ( dropUnit.percentType == 0 )
		return dropUnit.dropId != 0xffffffff;

	if ( dropUnit.percentType == 1 )
		return rand() < dropUnit.percent;

	if ( dropUnit.percentType == 2 )
		if ( ++ dropUnit.killCountCur >= dropUnit.percent )
			return ( dropUnit.killCountCur = 0 ), true;

	return false;
}

BOOL SelectDropResult( CPlayer *pActPlayer, DWORD did, DWORD gid, DWORD tid, POINT pos, CRegion *region, CMonster *monster )
{
	/*
	DWORD dropCount = 0; // 掉落个数，用来将所有道具分散排列！
	std::map< DWORD, GlobalDropTable >::iterator it = globalDropTable.find( did );
	SRawItemBuffer itemRcd; 
	if ( it != globalDropTable.end() && it->first == it->second.tableId )
	{
		ItemDropRcd::Key64 key;
		key.mapId = region->m_wMapID;
		key.regionId = region->m_wRegionID;
		key.monsterId = monster->m_Property.id;

		BOOL isLimited = ( monster->m_Property.boss != 2 &&  
			monster->m_Property.boss != 3 && 
			monster->m_Property.boss != 4 && 
			pActPlayer->m_Property.m_Level - monster->m_Property.level >= 50 );

		int group[10] = { 0,0,0,0,0, 0,0,0,0,0 };
		for ( std::list< GlobalDropTable::DropUnit >::iterator it2 = it->second.dropList.begin(); 
			it2 != it->second.dropList.end(); it2 ++ )
		{
			assert( it2->group >= 0 && it2->group <= 10 );
			if ( it2->group && group[ it2->group - 1 ] )
				continue; // 当前分组已经掉落过东西了！

			if ( CheckPercent( *it2 ) )
			{
				if( isLimited && it2->attenuation == 0 )
					continue;

				if ( it2->group ) 
					group[ it2->group - 1 ] = 1;

				switch ( it2->dropType )
				{
				case 1: // 道具ID
				case 2: // 道具名
					DropNewItemToGround( itemRcd, it2->dropId, it2->count, gid, tid, pos, region, dropCount, monster, it2->bind );
					break;

				case 3: // 脚本ID
					LuaFunctor( g_Script, "OnMonsterDead" )[ it2->dropId ]();
					itemRcd.wIndex = (WORD)it2->dropId;
					break;

				case 4: // 子表
					if ( it2->dropId < static_DropArray.size() )
					{
						SDropTable &dt = static_DropArray[it2->dropId];
						if ( dt.curIndex >= dt.byNumber ) 
							dt.curIndex = 0;

						DropNewItemToGround( itemRcd, dt.wIndex[dt.curIndex++], 1, gid, tid, pos, region, dropCount, monster, it2->bind );
					}
					break;
				}

				// 日志统计，所有装备
				const SItemData *itemData = CItem::GetItemData( itemRcd.wIndex );
				if( !itemData )
					return FALSE;

				DWORD colorValue = 0;
				if ( ITEM_IS_EQUIPMENT( itemData->byType ) )
					colorValue = GetEquipmentColor( ( SEquipment& )itemRcd );

				key.itemId = itemRcd.wIndex;
				ItemDropRcd *ptr = &static_DropRcd[ key.key ];
				if ( it2->dropType == 3 ) ptr->isScript = 1;
				assert( colorValue <= 5 ); // 因为初创建时，资质必定是未鉴定状态！
				if ( colorValue >= 3 ) colorValue --; // 因为有2个绿色。。。
				( ( LPDWORD )&ptr->white )[colorValue]++;

				// iflag 是否是需要公告的道具
				DWORD GetCheckItemFlag( WORD itemId );
				void TalkToAll( LPCSTR info, SRawItemBuffer *item, LPCSTR name = NULL );
				DWORD iflag = GetCheckItemFlag( itemRcd.wIndex );
				if ( iflag & 0x80 )
					TalkToAll( FormatString( "“%s”被“%s”击败后留下了宝物。", monster->m_Property.name, 
					pActPlayer->GetName() ? pActPlayer->GetName() : "" ), &itemRcd );
			}
		}
	}//*/
	return false;
}

// 返回等级为level的装备的阶数
WORD GetEquipRank(WORD level)
{
	if (level > 0 && level <= 10)
		return 1;
	else if (level > 10 && level <= 30)
		return 2;
	else if (level > 30 && level <= 50)
		return 3;
	else if (level > 50 && level <= 70)
		return 4;
	else if (level > 70 && level <= 90)
		return 5;
	else if (level > 90 && level <= 110)
		return 6;
	else if (level > 110 && level <= 120)
		return 7;
	else if (level > 120 && level <= 140)
		return 8;
	else
		return 0;
}

//侠义世界生成装备随机属性的个数
WORD GenerateEquipAttNum(WORD levle)
{
	//生成唯一的随机数
	WORD odds = abs(rand()) % 10000 + 1; 

	WORD maxAttribute = 0;

	//当前等级武器最大生成属性条数
	WORD maxAttRule = 0;

	//计算当前等级武器属性最大条数
	for (int i = 0;i < Max_Att_Num_Rule;++i)
	{
		if(g_sAttNumRule[i].levle >= levle)
		{
			maxAttRule = g_sAttNumRule[i].number;
			break;
		}
	}

	for (int m = 0;m < Max_Att_Num;m++)
	{
		int ret= 0;

		int j = 0;
		for (;j <= m;j++)
		{
			ret += g_wAttNumOdds[j];
		}
		if ( odds > ret )
			continue;		

		//超过最大条数，设置为应该的最大条数
		if( j > maxAttRule)
			j = maxAttRule;

		maxAttribute = j;
		break;
	}
	return maxAttribute;

}
//侠义世界生成装备随机属性属性类型
bool g_isSelected[11];
void GenerateEquipAttType(int *attType,int *curattType)
{
	//按照个数生成具体的属性、每个属性的具体数值
	//临时的新老结构映射表，来对应属性类型
// 	int mapDataType[11] = 
// 	{
// 		SGemData::GEMDATA_ADDHPMAXP,				//增加生命上限
// 		SGemData::GEMDATA_ADDMPMAXP,				//增加内力上限
// 
// 		SGemData::GEMDATA_OUTATT_BASE,				//外功基础
// 		SGemData::GEMDATA_OUTATT_MASTERY,			// 外攻精通
// 		SGemData::GEMDATA_INNATT_BASE,				//内功基础
// 		SGemData::GEMDATA_INNATT_MASTERY,			// 内攻精通
// 
// 		SGemData::GEMDATA_OUTDEF_BASE,				//外防基础
// 		SGemData::GEMDATA_OUTDEF_MASTERY,			// 外防精通
// 		SGemData::GEMDATA_INNDEF_BASE,				//内防基础
// 		SGemData::GEMDATA_INNDEF_MASTERY,			// 内防精通
// 
// 		SGemData::GEMDATA_AMUCK,                      // 杀气
// 	};
// 	//生成属性类型
// 	WORD odds = abs(rand()) % 10000 + 1; 
// 	for (int m = 0;m < Max_Att_Num;m++)
// 	{
// 		WORD ret= 0;
// 
// 		for (int j = 0;j <= m;j++)
// 		{
// 			ret += g_sAttTypeOdds[j].OddsValue;
// 		}
// 		//已被选出，或则未命中
// 		if ( g_isSelected[m] || odds > ret )
// 		{
// 			//如果这是最后一个属性，而且这个属性不满足选出条件
// 			if(m == Max_Att_Num - 1)
// 			{
// 				//向下选出最后一个未被选出的属性
// 				int ret = 0;
// 				for (int flg = Max_Att_Num - 1;flg >= 0;flg--)
// 				{
// 					if (!g_isSelected[flg])
// 					{
// 						ret = flg;
// 						break;
// 					}
// 				}
// 
// 				if(ret < 0 || ret > Max_Att_Num - 1)
// 					ret = 0;
// 				*curattType = ret;
// 				*attType= mapDataType[ret];
// 				g_isSelected[ret] = true;
// 				break;
// 			}
// 			else continue;		
// 		}
// 		*curattType = m/*mapDataType[m]*/;
// 		*attType = mapDataType[m];
// 		g_isSelected[m] = true;
// 		break;
// 	}
}
//侠义世界装备属性值生成
void GenerateEquipAttValue(int *attValue,int attType)
{
	UINT32 oddss = ((abs(rand()) % 10000) * 100) % 1000000 + 1; 
	for (int m = 0;m < Max_Att_Value_Num;m++)
	{
		UINT32 ret= 0;

		for (int j = 0;j <= m;j++)
		{
			ret += g_sAttValueOdds[attType].OddsValue[j];
		}
		if ( oddss > ret)
			continue;		

		*attValue = m + 1;
		break;
	}
}
//侠义世界掉落道具生成
bool GenerateNewItemXYSJ(CPlayer *pActPlayer,CRegion *region,CMonster *monster, DWORD protectedGID,DWORD protectedTeamID,
						 DWORD &dropCount, POINT pos,DWORD itemId,SRawItemBuffer itemRcd, BOOL bind)
{
	/*
	//往下执行生成道具
	ItemDropRcd::Key64 key;
	key.mapId = region->m_wMapID;
	key.regionId = region->m_wRegionID;
	key.monsterId = monster->m_Property.id;

	BOOL isLimited = ( monster->m_Property.boss != 2 &&  
		monster->m_Property.boss != 3 && 
		monster->m_Property.boss != 4 && 
		pActPlayer->m_Property.m_Level - monster->m_Property.level >= 50 );


	//掉落到场景中的道具的属性
	SGroundItem item;
	memset( &item, 0, sizeof( item ) );
	CItem::SParameter param;
	ZeroMemory( &param, sizeof( param ) );
	param.dwLife = MakeLifeTime( 5 );
	param.dwProtectedGID = protectedGID;
	param.ProtectTeamID = protectedTeamID;

	do 
	{
		if ( dropCount >= 1000 )
		{
			param.xTile = (WORD)pos.x;
			param.yTile = (WORD)pos.y;
			break;
		}

		param.xTile = WORD(pos.x + dropMap[dropCount].x);
		param.yTile = WORD(pos.y + dropMap[dropCount].y);
		dropCount ++;
		// 需要确认目标点可以放道具！
	} while ( !region->isTileWalkAble( param.xTile, param.yTile ) );

	const SItemData *itemData = CItem::GetItemData( itemId );

	DWORD slotNumber = 0;

	int ncount = 1;

	// 创建道具到场景地表！
	param.pItem = &item;
	if ( !GenerateNewItem( item, GenItemParams( (WORD)itemId, ncount, (WORD)slotNumber ), LogInfo( 2, "怪物死亡掉落" ) ) )
		return FALSE;

	// 设置绑定信息
	item.flags = bind;

	//测试用的装备属性随机逻辑
	if (  ITEM_IS_EQUIPMENT( itemData->byType ) )
	{
		//生成装备属性（包括属性个数，属性类型，每个属性的值）
		SEquipment *equip = (SEquipment*)( ( SItemBase* )&item );

		//1.属性个数
		WORD maxAttribute = GenerateEquipAttNum(itemData->byLevelEM);


		// 1. 附加等级，最大15
		//equip->level = rand() % 10;
		//if ( equip->level > 9 ) 
		//	equip->level = 9;
// 
// 		int curattType = 0,attValue = 0,attType;
// 		memset(&g_isSelected,0,sizeof(g_isSelected));
// 
// 		//依次生成每个属性	
// 		for ( int k = 0; k < maxAttribute; k++ ) 
// 		{
// 			//属性类型
// 			GenerateEquipAttType(&attType,&curattType);
// 		    //属性具体值
// 			GenerateEquipAttValue(&attValue,curattType);
// 
// 			//暂时用老的结构来显示
// 			if (k < 6)
// 			{
// 				equip->attribute.inherits[ k ].type = attType;
// 				equip->attribute.inherits[ k ].value = attValue;
// 			}
// 			else
// 			{
// 				equip->attribute.qualitys[ k ].type = attType;
// 				equip->attribute.qualitys[ k ].value = attValue;
// 			}
// 
// 			//equip->attribute.newAttTpye.equipAttType[k].value = attValue;
// 			//equip->attribute.newAttTpye.equipAttType[k].type  = attType;
// 
// 			continue;
// 		}
	}
	// 给外部返回新创建的道具数据！
	itemRcd = item;

	region->Move2Area( region->GenerateObject( IID_ITEM, 0, (LPARAM)&param ), param.xTile, param.yTile, true );
	//*/

	return true;
}

//从一个掉落表里随机取一个道具ID,根据后面跟的几率
int GetDropItemIDFromTable(int ItemID)
{
	ItemID = ItemID - 100000;
	bool isTrue = false;
	WORD curItemId = 0;
	WORD curOdds = (rand() % 100);
	WORD tableSize = (WORD)g_vItemDropIdTable.size();
	WORD tableId = 0;
	WORD curOddsSum = 0;
	for (int i = 0;i < tableSize;++i)
	{
		if(g_vItemDropIdTable[i].itemId == ItemID)
		{
			isTrue = true;
			tableId = i;
			break;
		}
	}
	if (!isTrue)
		return rfalse(0, 0, "道具掉落数据不匹配！");

	tableSize = (WORD)g_vItemDropIdTable[tableId].itemIdTable.size();
	//初始化最后得到的Item 为最后一个
	curItemId = tableSize - 1;
	for (WORD i = 0;i < tableSize;++i)
	{	
		curOddsSum += g_vItemDropIdTable[tableId].itemIdTable[i].wOdds;
		if (curOddsSum > curOdds)
		{
			//如果几率满足，重置最后ITEM 
			curItemId = i;
			break;
		}
	}
	ItemID = g_vItemDropIdTable[tableId].itemIdTable[curItemId].itemId;

	return ItemID;
}

//侠义世界判断是否是任务需要的道具
BOOL GenerateTaskDropItem(WORD itemId,CPlayer *pActPlayer)
{
	g_Script.SetCondition( NULL, pActPlayer, NULL );
	g_Script.PrepareFunction( "GetMyUnfinishedTaskItems" );
	g_Script.PushParameter( lite::Variant( itemId) );

	lite::Variant scriptResult;
	BOOL execResult = g_Script.Execute( &scriptResult );
	g_Script.CleanCondition();

	if ( execResult )
	{
		execResult = FALSE;
		try { 
			execResult = ( int )scriptResult; 
			return execResult;
		}
		catch ( lite::InvalidConvert & ) {return false;}
	}
	return false;
}

//返回命中任务表中的道具ID
int IsTaskItemTable(size_t VectorIndex,CPlayer* pActPlayer)
{
	//比对得到需要掉落的道具列表
	ITEMDROPTABLE curTaskDropTable;
	size_t len = (size_t)g_vTaskDropIdTable.size();

	if(VectorIndex >= len)
		return false;

	size_t curlen = (size_t)g_vTaskDropIdTable[VectorIndex].itemIdTable.size();
	for (size_t i = 0;i < curlen;++i){
		if(GenerateTaskDropItem(g_vTaskDropIdTable[VectorIndex].itemIdTable[i].itemId,pActPlayer))
			curTaskDropTable.push_back(g_vTaskDropIdTable[VectorIndex].itemIdTable[i]);
	}		

	//随机确定一个需要掉落的道具
	len = (size_t)curTaskDropTable.size();
	if(len == 0) 
		return -1;

	WORD curDropItem = (WORD)(rand()%len);
	int retDropItemId = -1;	
	//根据几率看是否命中可以掉落
	if(IN_ODDS(curTaskDropTable[curDropItem].wOdds))
		retDropItemId = curTaskDropTable[curDropItem].itemId;

	return retDropItemId;
	////////////end
}

//侠义世界道具掉落
bool NewItemDropGroundXYSJ(CPlayer *pActPlayer,DWORD did, DWORD count, DWORD protectedGID, DWORD protectedTeamID, 
						   POINT pos, CRegion *region, DWORD &dropCount, CMonster *monster, BOOL bind,int dropItemTableId)
{
	//pActPlayer				玩家
	//did							怪物ID
	//count						怪物身上道具最大个数
	//protectedGID			玩家GID
	//protectedTeamID	玩家队伍ID
	//pos							TILE坐标
	//region					所属场景
	//monster					怪物
	//bind						是否绑定

	//得到怪物掉落的任务掉落道具列表
	/*
	size_t VectorIndex = 0;
	size_t len = (size_t)g_vTaskDropIdTable.size();
	for (size_t i = 0;i < len;++i)
	{
		WORD id =  g_vTaskDropIdTable[i].itemId;
		if(g_vTaskDropIdTable[i].itemId == dropItemTableId){
			VectorIndex = i;
			break;
		}
	}

	int itemId = 0;

	//是否命中，可以生成该道具
	bool CanGanerate = false;

	//monster->m_Property.dropTable;

	if (pActPlayer == NULL || count <= 0)
		return false;

	if( !monster )
		return false;
	
	//怪物最多携带的物品
	assert(sizeof(monster->m_Property.dropTable[0]) > 0);
	int maxItemNum = ( sizeof(monster->m_Property.dropTable) / sizeof(monster->m_Property.dropTable[0]) );


	SRawItemBuffer itemRcd; 
	ZeroMemory(&itemRcd,sizeof(itemRcd));

	srand(timeGetTime());
	//从最大掉落数中随机取个数掉，至少有一个
	int itemNum = abs(rand())%count + 1;

	//要产生的最大物品数目(如果配置表里写出几率相加为0，永远不掉东西)
	for (int i = 0; i< itemNum;i++)
	{
		//优先查看任务道具是否命中
		int TaskItemID = IsTaskItemTable(VectorIndex,pActPlayer);

		//没有命中任务道具，继续尝试命中普通道具
		if(TaskItemID == -1)
		{
			//生成唯一的随机数（暂时用的十万分比）rand()函数的范围是-90～32767之间
			int itemodds = ((abs(rand()) % 10000) * 10) % 100000 + 1; 
			for (int m = 0;m < maxItemNum;m++)
			{
				int ret = 0;

				for (int j = 0;j <= m;j++)
				{
					ret += monster->m_Property.dropTable[j][1];
					itemId = monster->m_Property.dropTable[j][0];
				}
				if ( itemodds < ret )	
				{
					CanGanerate = true;
					break;
				}
			}
			
			//如果没任何道具命中
			if (!CanGanerate)
				continue;

			CanGanerate = false;
		}
		else
			itemId = TaskItemID;

		
	//	  道具生成规则:
	//	  负数代表触发脚本;
	//	  1-99999区间数字代表掉落物品编号;
	//	  100000以上数字代表触发掉落列表编号.
		
		if (itemId < 0)
		{
			//执行脚本

			CTrigger trigger;
			trigger.m_dwType = CTrigger::TT_MONSTERDEAD;
			trigger.SMonsterDead.dwScriptID = abs(itemId);
			trigger.SMonsterDead.pPlayer = pActPlayer;
			trigger.SMonsterDead.pMonster = monster;

			pActPlayer->OnTriggerActivated(&trigger);
		}
		else
		{
			if (itemId >= 100000)
				itemId = GetDropItemIDFromTable(itemId);

			//往下生成道具
			GenerateNewItemXYSJ(pActPlayer,region,monster, protectedGID, protectedTeamID,
				dropCount, pos,itemId,itemRcd,bind);	
		}
	}//*/

	return true;
}

void SaveItemDropRcdToFile( BOOL saveName )
{
//     CreateDirectory( "Logs", NULL );
//     CreateDirectory( "Logs\\ItemDropRcd", NULL );
//     SYSTEMTIME s;
//     GetLocalTime(&s);
//     LPCSTR filename = FormatString( "Logs\\ItemDropRcd\\%d-%d-%d %d;%d;%d.log", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond );
//     HANDLE hFile = CreateFile( filename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, NULL, NULL );
// 	if ( hFile == INVALID_HANDLE_VALUE )
// 		return;
// 
//     DWORD cbWrite = 0;
//     #define TITLE "产出地图\t产出场景\t产出怪物\t产出道具\t类型\t是否脚本\t白色个数\t蓝色个数\t绿色个数\t紫色个数\t橙色个数\r\n"
//     ::WriteFile( hFile, TITLE, sizeof( TITLE ) - 1, &cbWrite, NULL );
//     for ( std::map< QWORD, ItemDropRcd >::iterator it = static_DropRcd.begin();
//         it != static_DropRcd.end(); it ++ )
//     {
//         extern BYTE hugeBuffer[ 0xffff ];
//         ItemDropRcd *ptr = &it->second;
// 		const ItemDropRcd::Key64 *key = ( const ItemDropRcd::Key64* )&it->first;
//         size_t size = 0;
//         if ( saveName )
//         {
//             const SItemData *itemData = CItem::GetItemData( key->itemId );
//             const SMonsterData *monsterData = CMonster::GetMonsterData( key->monsterId - 1 );
//             if ( itemData == NULL || monsterData == NULL )
//                 goto _doid;
// 
//             size = sprintf( ( LPSTR )hugeBuffer, "%d\t%d\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r\n", key->mapId, key->regionId, 
//                 monsterData->name, itemData->szName, itemData->byType, ptr->isScript, ptr->white, ptr->blue, ptr->green, ptr->purple, ptr->orange );
//         }
//         else
//         {
//             _doid:
//             const SItemData *itemData = CItem::GetItemData( key->itemId );
//             if ( itemData == NULL )
//                 continue;
// 
//             size = sprintf( ( LPSTR )hugeBuffer, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r\n", key->mapId, key->regionId, 
//                 key->monsterId, key->itemId, itemData->byType, ptr->isScript, ptr->white, ptr->blue, ptr->green, ptr->purple, ptr->orange );
//         }
//         ::WriteFile( hFile, hugeBuffer, (WORD)size, &cbWrite, NULL );
//     }
//     CloseHandle( hFile );
}

void CleanItemDropRcd() { static_DropRcd.clear(); }

