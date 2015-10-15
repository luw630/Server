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

#include "���������/Environment.h"
#include "DSpcialItemData.h"


//��������  װ����������������  ab_yue

WORD	*g_wAttNumOdds;									//װ��������Եĸ������ٷ��ʣ��� 100��		
SEquipAttTypeOddsData		*g_sAttTypeOdds;			//װ�����ÿ�����Գ��ּ��ʣ��ٷ��ʣ��� 100��
SEquipAttValueOddsData		*g_sAttValueOdds;			//װ��ÿ������ȡÿ��ֵ�ļ���(����ʣ��� 10000)
SEquipAttNumRuleData		*g_sAttNumRule;				//װ������������������

ITEMDROPIDTABLE g_vItemDropIdTable;		//������Ʒid��,���е�������ݽṹ
ITEMDROPIDTABLE g_vTaskDropIdTable;		//������Ʒid��,���е�������ݽṹ

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
		rfalse(0, 0, "�޷���[Data\\item\\droptable\\droptable.def]���ߵ��������б��ļ���");
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
			rfalse(0, 0, "�޷���[%s]���ߵ��������б��ļ���", fname.c_str());
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
			percentType = 1; // ���ֻд�˵�����Ʒ������û��д���ʣ���Ϊ����Ϊ0
			percent = 0;
			dropType = 0;
			group = 0;
			bind = 0;
			dropId = 0;
			killCountCur = 0;
			count = 0;
			attenuation = 0;
		}

		BYTE  dropType;     // ��������
		BYTE  percentType;  // �ٷֱ����ͣ��Ǹ��ʻ����ۼƼ���
		BYTE  group;        // ������飬Ϊ0ʱ���ж�
		BYTE  bind;         // ������Ƿ�Ϊ�󶨵��ߡ�����
		BYTE  attenuation;  // ˥������ 1 ����˥��֮�⣬0����ͨ
		DWORD dropId;       // ����ֵ
		WORD  percent;      // �ٷֱ�ֵ����������rand�������������ֵ��ֻ�ܵ�RAND_MAX��32767��...
		WORD  killCountCur; // ��ǰ��ɱ����
		DWORD count;        // ���������������ڵ������ֵ��
	};

	std::list< DropUnit > dropList;
};

// size = 32byte
struct ItemDropRcd
{
	ItemDropRcd() { memset( this, 0, sizeof( *this ) ); }

	// ǰ8���ֽ��������һ��QWORD�Ĳ�ѯ�ؼ��֣�
	struct Key64 {
		union {
			struct {
				WORD mapId;     // ��ͼ���
				WORD regionId;  // ������ţ����Ϊ��̬�������򳡾����Ϊ0��
				WORD monsterId; // ����������
				WORD itemId;    // �������߱��
			};
			QWORD key;
		};
	};

	DWORD isScript; // �Ƿ��ǽű�������Ƚ����⡣����
	DWORD white, blue, green, purple, orange; // 5����ɫ�Ĳ��������ȣ�
};

extern std::map< std::string, DWORD > itemNameMap;          // ������-ID���ձ�
static std::map< DWORD, GlobalDropTable > globalDropTable;  // ȫ�ֵ����
static std::vector< SDropTable > static_DropArray;          // ��������
static std::map< std::string, DWORD > static_DropTableIdx;  // ��ȡ��������ʱ����ʱ������
static std::map< QWORD, ItemDropRcd > static_DropRcd;       // ��������־���ݣ�

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
		return MessageBox( 0, tempName.c_str(), "ָ���ļ��򲻿���", 0 ), 0xffffffff;

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
				rfalse( 2, 1, "���ߵ����ӱ�[%s]�е�[%s]������Ч�ĵ�������", tempName.c_str(), temp.c_str() + 1 );
				dt.wIndex[dt.byNumber++] = 0xffff;
			}
		}
	}

	if ( dt.byNumber == 0 )
		return 0xffffffff;

	if ( dt.byNumber >= 254 )
		rfalse( 2, 1, "���ߵ����ӱ�[%s]�еĵ���̫���ˡ�������", tempName.c_str() );

	// ��ʼ�������
	dt.curIndex = rand() % dt.byNumber;

	static_DropArray.push_back( dt );
	return (DWORD)static_DropArray.size() - 1;
}
 
BOOL LoadGlobalDrop( std::string strDropFileName = "" )
{
	dwt::ifstream stream( ( strDropFileName == "" ) ? ( "data\\item\\ȫ�ֵ����.txt" ) : strDropFileName.c_str() );
	if ( !stream.is_open() )
		return MessageBox( 0, "ȫ�ֵ��ߵ����δ�򿪣�", 0, 0 ), false;

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

	//          // �Ƿ�����ʼ����
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

	//              // ����Ѿ���������ˣ�����Ҫ�����������ӵ������棡
	//              if ( dropUnit.dropType )
	//              {
	//                  if ( dropUnit.dropId != 0xffffffff )
	//                      dropTable.dropList.push_back( dropUnit );
	//                  //memset( &dropUnit, 0, sizeof( dropUnit ) );
	//			dropUnit.init();
	//              }

	//              // ׼��������
	//              dropUnit.dropType = dropType;
	//              if ( dropType == 2 ) // ���⴦���������֣�
	//              {
	//                  std::map< std::string, DWORD >::iterator it = itemNameMap.find( temp.c_str() + 1 );
	//                  if ( it != itemNameMap.end() )
	//                      dropUnit.dropId = it->second;
	//                  else
	//                  {
	//                      rfalse( 2, 1, "���ߵ�����е�[%s]������Ч�ĵ�������", temp.c_str() + 1 );
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
	//                  return MessageBox( 0, lineBuffer, "ȫ�ֵ��ߵ�����д�[1]��", 0 ), false;
	//              else
	//                  dropUnit.dropId = atoi( temp.c_str() + 1 );
	//              break;

	//          case '&':
	//              if ( !isdigit( temp[1] ) )
	//                  return MessageBox( 0, lineBuffer, "ȫ�ֵ��ߵ�����д�[4]��", 0 ), false;

	//              dropUnit.group = atoi( temp.c_str() + 1 );
	//              if ( dropUnit.group == 0 || dropUnit.group > 10 )
	//                  return MessageBox( 0, lineBuffer, "���鲻�ܴ���10�����0��", 0 ), false;
	//              break;

	//          case '*':
	//              if ( !isdigit( temp[1] ) )
	//                  return MessageBox( 0, lineBuffer, "ȫ�ֵ��ߵ�����д�[5]��", 0 ), false;

	//              dropUnit.count = atoi( temp.c_str() + 1 );
	//              break;

	//          case '@':
	//              dropUnit.bind = 1; // ������ߵ���������Ҫ��Ϊ��״̬������
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
	//                      return MessageBox( 0, lineBuffer, "ȫ�ֵ��ߵ�����д�[2]��", 0 ), false;

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
	//                  return MessageBox( 0, lineBuffer, "ȫ�ֵ��ߵ�����д�[3]��", 0 ), false;
	//          }
	//      }

	//      if ( dropUnit.dropType && dropUnit.dropId != 0xffffffff )
	//          dropTable.dropList.push_back( dropUnit );
	//  }
	// zeb 2009.11.19
	return true;
}

// ���쾲̬���䷶Χ��! 16*16��С��
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
��������			���ʸ���			���丽��			��ɫ
��ͨװ��    û��(0)				û��(0)				û��(0)				��ɫ(0)

��Ʒװ��    û��					û��					��(1)				��ɫ(1)

��Ʒװ��    ��(2)				û��					û��					��ɫ(2)
��Ʒװ��    ��(2)				û��					��(1)				��ɫ(3)

����װ��    û��					��(4)				û��					��ɫ(4)

�츳װ��    ��(2)				��(4)				û��					��ɫ(6)
�츳װ��    ��(2)				δ����(3)			û��					��ɫ(5)
*/

DWORD GetEquipmentColor( SEquipment &e )
{
// 	BOOL devAddon = false;                              // �Ƿ�������ʸ���
// 	BOOL dropAddon = false;                             // �Ƿ���ڵ��丽��
// 	BOOL createAddon = e.attribute.naturals.type != 0;  // �Ƿ������������
// 	DWORD colorValue = ( createAddon ? 2 : 0 );
// 	SEquipment::Attribute::Unit *it = e.attribute.qualitys;
// 	for ( int i = 0; i < 6; i ++ )
// 	{
// 		if ( it[i].type == 0 )
// 			continue;
// 
// 		// ���isDevelop��ֵ����ʾqualitys�ǵ��丽�����ԣ�����Ϊ�װ�
// 		// ���isDevelop��ֵ����ʾqualitys�Ǽ����������ԣ����Ϊ�װ壬��ʾδ������
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
		// ��Ҫȷ��Ŀ�����Էŵ��ߣ�
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
		// ��Ҫȷ��Ŀ�����Էŵ��ߣ�
	} while ( !region->isTileWalkAble( param.xTile, param.yTile ) );

	// ���װ���׵��趨��1/20�ļ��ʳ��ֿ����ã���������1/3�ļ���1~2����
	const SItemData *itemData = CItem::GetItemData( itemId );

	// �¼�������Ʒ���Դ���
	DWORD slotNumber = 0;
	SpecialItem* sItem = CItem::GetSpecialItem( itemId );
	if ( sItem && ITEM_IS_EQUIPMENT( itemData->byType ) )
	{
		// 0. �Ӷ�
		if( IN_ODDSS( sItem->mHoleAdd[0] ) )
		{
			DWORD v = ( sItem->mHoleAdd[2] - sItem->mHoleAdd[1] );
			if ( v == 0 ) v = 1;
			slotNumber = ( rand() % v ) + sItem->mHoleAdd[1];
		}
	}

	// �������ߵ������ر�
	param.pItem = &item;
	if ( !GenerateNewItem( item, GenItemParams( (WORD)itemId, count, (WORD)slotNumber ), LogInfo( 2, "������������" ) ) )
		return FALSE;

	// ���ð���Ϣ
	item.flags = bind;

	if ( sItem && ITEM_IS_EQUIPMENT( itemData->byType ) )
	{
		// SetRandAttributes( 100, *reinterpret_cast< SEquipment* >( param.pItem ) );
		SEquipment *equip = (SEquipment*)( ( SItemBase* )&item );

		// 1. ���ӵȼ������15
		equip->level = 1;
		if( IN_ODDSS( sItem->mLevelAdd[0]) )
		{
			DWORD v = ( sItem->mLevelAdd[2] - sItem->mLevelAdd[1] );
			if ( v == 0 ) v = 1;
			equip->level = ( rand() % v ) + sItem->mLevelAdd[1];
		}
		if ( equip->level > 9 ) 
			equip->level = 9;

		// 2. ǿ�����ӹ�����ʶ
		for( int i = 0; i < 3; i ++ )
			GetAddAttribute( &equip->attribute.activates[i], (WORD)sItem->mIntensifyAdd[i], 1 );

		// 3. ��������
		if ( IN_ODDSS( sItem->mInnetenessAddOddsAndFlag[0] ) )
			GetAddAttribute( &equip->attribute.naturals, (WORD)sItem->mInnetenessAddOddsAndFlag[1], 1 );

		// 4. ���ʼ���
		if( IN_ODDSS( sItem->mAptitudeCheckupOdds[0] ) )
			equip->isDevelop = 1;
		else
		{	
			// 4.1���丽��
			equip->isDevelop = 0;
			if( IN_ODDSS( sItem->mDropAddsOddsAndFlag[0]) )
				GetAddAttribute( equip->attribute.qualitys, (WORD)sItem->mDropAddsOddsAndFlag[1], 6 );
		}
	}

	// ���ⲿ�����´����ĵ������ݣ�
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
	DWORD dropCount = 0; // ������������������е��߷�ɢ���У�
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
				continue; // ��ǰ�����Ѿ�����������ˣ�

			if ( CheckPercent( *it2 ) )
			{
				if( isLimited && it2->attenuation == 0 )
					continue;

				if ( it2->group ) 
					group[ it2->group - 1 ] = 1;

				switch ( it2->dropType )
				{
				case 1: // ����ID
				case 2: // ������
					DropNewItemToGround( itemRcd, it2->dropId, it2->count, gid, tid, pos, region, dropCount, monster, it2->bind );
					break;

				case 3: // �ű�ID
					LuaFunctor( g_Script, "OnMonsterDead" )[ it2->dropId ]();
					itemRcd.wIndex = (WORD)it2->dropId;
					break;

				case 4: // �ӱ�
					if ( it2->dropId < static_DropArray.size() )
					{
						SDropTable &dt = static_DropArray[it2->dropId];
						if ( dt.curIndex >= dt.byNumber ) 
							dt.curIndex = 0;

						DropNewItemToGround( itemRcd, dt.wIndex[dt.curIndex++], 1, gid, tid, pos, region, dropCount, monster, it2->bind );
					}
					break;
				}

				// ��־ͳ�ƣ�����װ��
				const SItemData *itemData = CItem::GetItemData( itemRcd.wIndex );
				if( !itemData )
					return FALSE;

				DWORD colorValue = 0;
				if ( ITEM_IS_EQUIPMENT( itemData->byType ) )
					colorValue = GetEquipmentColor( ( SEquipment& )itemRcd );

				key.itemId = itemRcd.wIndex;
				ItemDropRcd *ptr = &static_DropRcd[ key.key ];
				if ( it2->dropType == 3 ) ptr->isScript = 1;
				assert( colorValue <= 5 ); // ��Ϊ������ʱ�����ʱض���δ����״̬��
				if ( colorValue >= 3 ) colorValue --; // ��Ϊ��2����ɫ������
				( ( LPDWORD )&ptr->white )[colorValue]++;

				// iflag �Ƿ�����Ҫ����ĵ���
				DWORD GetCheckItemFlag( WORD itemId );
				void TalkToAll( LPCSTR info, SRawItemBuffer *item, LPCSTR name = NULL );
				DWORD iflag = GetCheckItemFlag( itemRcd.wIndex );
				if ( iflag & 0x80 )
					TalkToAll( FormatString( "��%s������%s�����ܺ������˱��", monster->m_Property.name, 
					pActPlayer->GetName() ? pActPlayer->GetName() : "" ), &itemRcd );
			}
		}
	}//*/
	return false;
}

// ���صȼ�Ϊlevel��װ���Ľ���
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

//������������װ��������Եĸ���
WORD GenerateEquipAttNum(WORD levle)
{
	//����Ψһ�������
	WORD odds = abs(rand()) % 10000 + 1; 

	WORD maxAttribute = 0;

	//��ǰ�ȼ��������������������
	WORD maxAttRule = 0;

	//���㵱ǰ�ȼ����������������
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

		//�����������������ΪӦ�õ��������
		if( j > maxAttRule)
			j = maxAttRule;

		maxAttribute = j;
		break;
	}
	return maxAttribute;

}
//������������װ�����������������
bool g_isSelected[11];
void GenerateEquipAttType(int *attType,int *curattType)
{
	//���ո������ɾ�������ԡ�ÿ�����Եľ�����ֵ
	//��ʱ�����Ͻṹӳ�������Ӧ��������
// 	int mapDataType[11] = 
// 	{
// 		SGemData::GEMDATA_ADDHPMAXP,				//������������
// 		SGemData::GEMDATA_ADDMPMAXP,				//������������
// 
// 		SGemData::GEMDATA_OUTATT_BASE,				//�⹦����
// 		SGemData::GEMDATA_OUTATT_MASTERY,			// �⹥��ͨ
// 		SGemData::GEMDATA_INNATT_BASE,				//�ڹ�����
// 		SGemData::GEMDATA_INNATT_MASTERY,			// �ڹ���ͨ
// 
// 		SGemData::GEMDATA_OUTDEF_BASE,				//�������
// 		SGemData::GEMDATA_OUTDEF_MASTERY,			// �����ͨ
// 		SGemData::GEMDATA_INNDEF_BASE,				//�ڷ�����
// 		SGemData::GEMDATA_INNDEF_MASTERY,			// �ڷ���ͨ
// 
// 		SGemData::GEMDATA_AMUCK,                      // ɱ��
// 	};
// 	//������������
// 	WORD odds = abs(rand()) % 10000 + 1; 
// 	for (int m = 0;m < Max_Att_Num;m++)
// 	{
// 		WORD ret= 0;
// 
// 		for (int j = 0;j <= m;j++)
// 		{
// 			ret += g_sAttTypeOdds[j].OddsValue;
// 		}
// 		//�ѱ�ѡ��������δ����
// 		if ( g_isSelected[m] || odds > ret )
// 		{
// 			//����������һ�����ԣ�����������Բ�����ѡ������
// 			if(m == Max_Att_Num - 1)
// 			{
// 				//����ѡ�����һ��δ��ѡ��������
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
//��������װ������ֵ����
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
//������������������
bool GenerateNewItemXYSJ(CPlayer *pActPlayer,CRegion *region,CMonster *monster, DWORD protectedGID,DWORD protectedTeamID,
						 DWORD &dropCount, POINT pos,DWORD itemId,SRawItemBuffer itemRcd, BOOL bind)
{
	/*
	//����ִ�����ɵ���
	ItemDropRcd::Key64 key;
	key.mapId = region->m_wMapID;
	key.regionId = region->m_wRegionID;
	key.monsterId = monster->m_Property.id;

	BOOL isLimited = ( monster->m_Property.boss != 2 &&  
		monster->m_Property.boss != 3 && 
		monster->m_Property.boss != 4 && 
		pActPlayer->m_Property.m_Level - monster->m_Property.level >= 50 );


	//���䵽�����еĵ��ߵ�����
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
		// ��Ҫȷ��Ŀ�����Էŵ��ߣ�
	} while ( !region->isTileWalkAble( param.xTile, param.yTile ) );

	const SItemData *itemData = CItem::GetItemData( itemId );

	DWORD slotNumber = 0;

	int ncount = 1;

	// �������ߵ������ر�
	param.pItem = &item;
	if ( !GenerateNewItem( item, GenItemParams( (WORD)itemId, ncount, (WORD)slotNumber ), LogInfo( 2, "������������" ) ) )
		return FALSE;

	// ���ð���Ϣ
	item.flags = bind;

	//�����õ�װ����������߼�
	if (  ITEM_IS_EQUIPMENT( itemData->byType ) )
	{
		//����װ�����ԣ��������Ը������������ͣ�ÿ�����Ե�ֵ��
		SEquipment *equip = (SEquipment*)( ( SItemBase* )&item );

		//1.���Ը���
		WORD maxAttribute = GenerateEquipAttNum(itemData->byLevelEM);


		// 1. ���ӵȼ������15
		//equip->level = rand() % 10;
		//if ( equip->level > 9 ) 
		//	equip->level = 9;
// 
// 		int curattType = 0,attValue = 0,attType;
// 		memset(&g_isSelected,0,sizeof(g_isSelected));
// 
// 		//��������ÿ������	
// 		for ( int k = 0; k < maxAttribute; k++ ) 
// 		{
// 			//��������
// 			GenerateEquipAttType(&attType,&curattType);
// 		    //���Ծ���ֵ
// 			GenerateEquipAttValue(&attValue,curattType);
// 
// 			//��ʱ���ϵĽṹ����ʾ
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
	// ���ⲿ�����´����ĵ������ݣ�
	itemRcd = item;

	region->Move2Area( region->GenerateObject( IID_ITEM, 0, (LPARAM)&param ), param.xTile, param.yTile, true );
	//*/

	return true;
}

//��һ������������ȡһ������ID,���ݺ�����ļ���
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
		return rfalse(0, 0, "���ߵ������ݲ�ƥ�䣡");

	tableSize = (WORD)g_vItemDropIdTable[tableId].itemIdTable.size();
	//��ʼ�����õ���Item Ϊ���һ��
	curItemId = tableSize - 1;
	for (WORD i = 0;i < tableSize;++i)
	{	
		curOddsSum += g_vItemDropIdTable[tableId].itemIdTable[i].wOdds;
		if (curOddsSum > curOdds)
		{
			//����������㣬�������ITEM 
			curItemId = i;
			break;
		}
	}
	ItemID = g_vItemDropIdTable[tableId].itemIdTable[curItemId].itemId;

	return ItemID;
}

//���������ж��Ƿ���������Ҫ�ĵ���
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

//��������������еĵ���ID
int IsTaskItemTable(size_t VectorIndex,CPlayer* pActPlayer)
{
	//�ȶԵõ���Ҫ����ĵ����б�
	ITEMDROPTABLE curTaskDropTable;
	size_t len = (size_t)g_vTaskDropIdTable.size();

	if(VectorIndex >= len)
		return false;

	size_t curlen = (size_t)g_vTaskDropIdTable[VectorIndex].itemIdTable.size();
	for (size_t i = 0;i < curlen;++i){
		if(GenerateTaskDropItem(g_vTaskDropIdTable[VectorIndex].itemIdTable[i].itemId,pActPlayer))
			curTaskDropTable.push_back(g_vTaskDropIdTable[VectorIndex].itemIdTable[i]);
	}		

	//���ȷ��һ����Ҫ����ĵ���
	len = (size_t)curTaskDropTable.size();
	if(len == 0) 
		return -1;

	WORD curDropItem = (WORD)(rand()%len);
	int retDropItemId = -1;	
	//���ݼ��ʿ��Ƿ����п��Ե���
	if(IN_ODDS(curTaskDropTable[curDropItem].wOdds))
		retDropItemId = curTaskDropTable[curDropItem].itemId;

	return retDropItemId;
	////////////end
}

//����������ߵ���
bool NewItemDropGroundXYSJ(CPlayer *pActPlayer,DWORD did, DWORD count, DWORD protectedGID, DWORD protectedTeamID, 
						   POINT pos, CRegion *region, DWORD &dropCount, CMonster *monster, BOOL bind,int dropItemTableId)
{
	//pActPlayer				���
	//did							����ID
	//count						�������ϵ���������
	//protectedGID			���GID
	//protectedTeamID	��Ҷ���ID
	//pos							TILE����
	//region					��������
	//monster					����
	//bind						�Ƿ��

	//�õ��������������������б�
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

	//�Ƿ����У��������ɸõ���
	bool CanGanerate = false;

	//monster->m_Property.dropTable;

	if (pActPlayer == NULL || count <= 0)
		return false;

	if( !monster )
		return false;
	
	//�������Я������Ʒ
	assert(sizeof(monster->m_Property.dropTable[0]) > 0);
	int maxItemNum = ( sizeof(monster->m_Property.dropTable) / sizeof(monster->m_Property.dropTable[0]) );


	SRawItemBuffer itemRcd; 
	ZeroMemory(&itemRcd,sizeof(itemRcd));

	srand(timeGetTime());
	//���������������ȡ��������������һ��
	int itemNum = abs(rand())%count + 1;

	//Ҫ�����������Ʒ��Ŀ(������ñ���д���������Ϊ0����Զ��������)
	for (int i = 0; i< itemNum;i++)
	{
		//���Ȳ鿴��������Ƿ�����
		int TaskItemID = IsTaskItemTable(VectorIndex,pActPlayer);

		//û������������ߣ���������������ͨ����
		if(TaskItemID == -1)
		{
			//����Ψһ�����������ʱ�õ�ʮ��ֱȣ�rand()�����ķ�Χ��-90��32767֮��
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
			
			//���û�κε�������
			if (!CanGanerate)
				continue;

			CanGanerate = false;
		}
		else
			itemId = TaskItemID;

		
	//	  �������ɹ���:
	//	  �����������ű�;
	//	  1-99999�������ִ��������Ʒ���;
	//	  100000�������ִ����������б���.
		
		if (itemId < 0)
		{
			//ִ�нű�

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

			//�������ɵ���
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
//     #define TITLE "������ͼ\t��������\t��������\t��������\t����\t�Ƿ�ű�\t��ɫ����\t��ɫ����\t��ɫ����\t��ɫ����\t��ɫ����\r\n"
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

