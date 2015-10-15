#include "StdAfx.h"
#include "networkmodule\itemtypedef.h"
#include "ItemData.h"

extern "C" DWORD _GetCrc32( LPVOID buf, DWORD size );

static long s_lItemSize = 0;							// 道具基本数据列表大小
//static SItemData *s_pItemDatas = NULL;					// 道具基本数据列表
static long s_lSpecialItemSize = 0;							// 特殊道具基本数据列表大小
//static SpecialItem *s_pSpecialItem = NULL;					// 道具基本数据列表
//static std::map< DWORD, SpecialItem > s_mapSpecialItem;     // 特殊道具
//static std::map< DWORD, SGemAtt > s_mapSpecialItemAddtribute; // 特殊道具的特殊属性
//WORD mSpecialAttIdx[57] = {1,3,2,4,15,16,17,12,13,14,21,27,51,50,5,6,7,8,9,10,11,18,19,20,0,0,48,24,25,26,28,29,30,31,39,40,41,42,52,0,0,0,0,32,33,34,35,0,0,0,0,0,0,0,47,49,0}; // 特殊属性编号
// 这里有些项未填导致爆出物品没有属性
WORD mSpecialAttIdx[57] = {1,3,2,4,15,16,17,12,13,14,21,27,51,50,5,6,7,8,9,10,11,18,19,20,22,23,48,24,25,26,28,29,30,31,39,40,41,42,52,43,44,45,36,32,33,34,35,0,36, 37, 38, 36, 0,46,47,49,29 }; // 特殊属性编号
// static std::pair< DWORD, const SItemData* > s_ppItemDatas[MAX_ITEMSCOPE];			        // 道具基本数据列表
// static std::map< int, SItemData > extraItemDatas;           // 道具基本数据列表

std::map< std::string, DWORD > itemNameMap;

// std::pair< DWORD, const SItemData* >* GetItemList()
// {
//     return s_ppItemDatas;
// }

// SItemData* GetItemData()
// {
// 	return s_pItemDatas;
// }

long GetItemCount(){ return s_lItemSize; }

static void ClearItemData()
{
//	if ( s_pItemDatas ) 
//        delete [] s_pItemDatas;

    s_lItemSize = 0;
//    s_pItemDatas = NULL;

 //   extraItemDatas.clear();
    itemNameMap.clear();

//	memset( s_ppItemDatas, 0, sizeof( s_ppItemDatas ) );
}

int CItemData::LoadSpecialItemAttribute( LPCSTR filename, BOOL refresh )
{
// 	static char str[128];
// 	dwt::ifstream stream( filename );
// 	if( !stream.is_open() )
// 		return rfalse( (refresh == FALSE ? 0 : 2), (refresh == FALSE ? 0 : 1), "无法打开[%s]特殊物品数据列表文件！", filename );
// 
//     s_mapSpecialItemAddtribute.clear();  // 换个位置，防止用命令行Load时，文件打开失败时列表被清除。。。重启服务器这里不影响
// 
// 	while( !stream.eof() )
// 	{
// 		stream >> str;
// 		if( strcmp( str, "·" ) == 0 )
// 			break;
// 	}
// 
// 	int num = 0;
// 	if( !stream.eof() )
// 	{
// 		stream >> str;
// 		if( strcmp( str, "items" ) == 0 )
// 			stream >> num;
// 	}
// 	if( num == 0 )
// 		return 0;
// 
// 	long nTimes = 0;
// 	while( nTimes < s_lSpecialItemSize )
// 	{
// 		SGemAtt psi;
// 		DWORD id;
// 
// 		int dwTemp = 0;
// 		stream >> str;
// 		id = ( ( stream >> dwTemp ), dwTemp );
// 
// 		for( int a = 0; a < 57; a ++ )
// 			for( int b = 0; b < 3; b ++ )
// 				psi.att[a][b] =  ( ( stream >> dwTemp ), dwTemp );
// 
// 		s_mapSpecialItemAddtribute[ id ] = psi;
// 
// 		nTimes ++;
// 	};
// 
// 	stream.close();
	return 1;
}

// SGemAtt* CItemData::GetSpecialItemAttribute( DWORD id )
// {
// 	std::map< DWORD, SGemAtt >::iterator it = s_mapSpecialItemAddtribute.find( id );
// 	if( it != s_mapSpecialItemAddtribute.end() )
// 		return &it->second;
// 
// 	return NULL;
// }

// SpecialItem* CItemData::GetSpecialItem( DWORD id )
// {
// 	std::map< DWORD, SpecialItem >::iterator it = s_mapSpecialItem.find( id );
// 	if( it != s_mapSpecialItem.end() )
// 		return &it->second;
// 
// 	return NULL;
// }

int CItemData::LoadSpecialItemData( LPCSTR filename, BOOL refresh )
{
// 	static char str[ 128 ];
// 	dwt::ifstream stream( filename );
// 	if( !stream.is_open() )
// 		return rfalse( (refresh == FALSE ? 0 : 2), (refresh == FALSE ? 0 : 1), "无法打开[%s]特殊物品数据列表文件！", filename );
// 
//     s_mapSpecialItem.clear();  // 换个位置，防止用命令行Load时，文件打开失败时列表被清除。。。重启服务器这里不影响
// 
// 	while( !stream.eof() )
// 	{
// 		stream >> str;
//         if ( dwt::strcmp( str, "·", 3 ) == 0 )
//             break;
// 	}
// 
// 	s_lSpecialItemSize = 0;
// 	if( !stream.eof() )
// 	{
// 		stream >> str;
// 		if( strcmp( str, "items" ) == 0 )
// 			stream >> s_lSpecialItemSize;
// 	}
// 	if( s_lSpecialItemSize == 0 )
// 		return 0;
// 
//     char lineBuffer[ 4096 ];
// 	long nTimes = 0;
// 
// #define STREAMLOAD( stream, buff ) ( ( stream >> buff ), buff )
// 
// 	while( nTimes < s_lSpecialItemSize )
// 	{
// 		SpecialItem psi;
// 
//         stream.getline( lineBuffer, sizeof( lineBuffer ) );
//         std::strstream line( lineBuffer, (std::streamsize)strlen( lineBuffer ), ios_base::in );
// 
// 		int dwTemp = 0;
// 		line >> str;
//         if ( str[ 0 ] == 0 )
//             continue;
// 
//         psi.mID                          =  STREAMLOAD( line, dwTemp );
//         psi.mLevelAdd[0]                 =  STREAMLOAD( line, dwTemp );
//         psi.mLevelAdd[1]                 =  STREAMLOAD( line, dwTemp );
//         psi.mLevelAdd[2]                 =  STREAMLOAD( line, dwTemp );
//         //psi.mLevelAdd[3]               =  STREAMLOAD( line, dwTemp );
//         psi.mIntensifyAdd[0]             =  STREAMLOAD( line, dwTemp );
//         psi.mIntensifyAdd[1]             =  STREAMLOAD( line, dwTemp );
//         psi.mIntensifyAdd[2]             =  STREAMLOAD( line, dwTemp );
//         psi.mInnetenessAddOddsAndFlag[0] =  STREAMLOAD( line, dwTemp );
//         psi.mInnetenessAddOddsAndFlag[1] =  STREAMLOAD( line, dwTemp );
//         psi.mAptitudeCheckupOdds[0]      =  STREAMLOAD( line, dwTemp );
//         //psi.mAptitudeCheckupOdds[1]    =  STREAMLOAD( line, dwTemp );
//         psi.mDropAddsOddsAndFlag[0]      =  STREAMLOAD( line, dwTemp );
//         psi.mDropAddsOddsAndFlag[1]      =  STREAMLOAD( line, dwTemp );
//         //psi.mDropAddsOddsAndFlag[2]    =  STREAMLOAD( line, dwTemp );
//         psi.mHoleAdd[0]                  =  STREAMLOAD( line, dwTemp );
//         psi.mHoleAdd[1]                  =  STREAMLOAD( line, dwTemp );
//         psi.mHoleAdd[2]                  =  STREAMLOAD( line, dwTemp );
//         //psi.mHoleAdd[3]                =  STREAMLOAD( line, dwTemp );
//         psi.mJianDingLevel[ 0 ]          =  STREAMLOAD( line, dwTemp );
//         psi.mJianDingLevel[ 1 ]          =  STREAMLOAD( line, dwTemp );
//         psi.mJianDingLevel[ 2 ]          =  STREAMLOAD( line, dwTemp );
//         psi.mJianDingLevel[ 3 ]          =  STREAMLOAD( line, dwTemp );
//         psi.mJianDingLevel[ 4 ]          =  STREAMLOAD( line, dwTemp );
//         psi.mJianDingLevel[ 5 ]          =  STREAMLOAD( line, dwTemp );
// 
// 		s_mapSpecialItem[ psi.mID ] = psi;
// 
// 		nTimes ++;
// 	};
// 
// 	stream.close();
	return 1;
}

int CItemData::LoadItemData( LPCSTR filename, bool crcInit, BOOL refresh )
{
	return 0;

// 	ClearItemData();
// 
//     static char str[128];
// 
//     dwt::ifstream stream( filename );
//     if ( !stream.is_open() )
//         return rfalse( (refresh == FALSE ? 0 : 2), (refresh == FALSE ? 0 : 1), "无法打开[%s]物品数据列表文件！", filename );
// 
//     while(!stream.eof())
//     {
//         stream >> str;
//         if(strcmp(str,"·") == 0)
//             break;
//     }
// 
//     s_lItemSize = 0;
//     if (!stream.eof())
//     {
//         stream >> str;
//         if( strcmp(str,"items") == 0)
//             stream >> s_lItemSize;
//     }
// 
//     if(s_lItemSize == 0) return 0;
// 
//     long nTimes = 0;
//     s_pItemDatas = new SItemData[ s_lItemSize ];
//     memset( s_pItemDatas, 0, sizeof( SItemData ) * s_lItemSize );
// 
//     char tmpStr[2048];
//     while( nTimes < s_lItemSize && !stream.eof() )
//     {
//         SItemData * psi = &s_pItemDatas[nTimes];
// 
//         stream.getline( tmpStr, sizeof( tmpStr ) );
//         if ( tmpStr[0] == 0 )
//             continue;
// 
//         std::strstream stream( tmpStr, (std::streamsize)strlen( tmpStr ) );
// 
// 		stream >> str;
//         if ( str[0] == 0 )
//             continue;
// 
//         int dwTemp = 0;
// 
// 		psi->wItemID = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->byType = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->byHoldSize = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wInventoryBigIconID = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wInventorySmallIconID = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wFacadeID = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wDropIconID = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wDropWavID = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wUseWavID = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wDropEffectID = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wUseEffectID = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wMaxWear = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wWearReduceCount = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wWearRepairMoney = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wPackNum  = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wOverlay = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->dwBuy   = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->dwSell   = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->byGroupEM = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->byLevelEM = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wOutAttaBaseMax = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wOutAttaBaseMin = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wOutAttaMasteryMax = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wOutAttaMasteryMin = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wInnerAttaBaseMax = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wInnerAttaBaseMin = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wInnerAttaMasteryMax = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wInnerAttaMasteryMin = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wOutDefBaseMax = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wOutDefBaseMin = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wOutDefMasteryMax = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wOutDefMasteryMin = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wInnerDefBaseMax = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wInnerDefBaseMin = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wInnerDefMasterMax = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wInnerDefMasteryMin = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wAgileAdd = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wSpeedAdd = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wAmuckAdd = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wAutoResHPAdd = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wAutoResHPIntervalDec = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wAutoResMPAdd = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wAutoResMPIntervalDec = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wAutoResSPAdd = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wAutoResSPIntervalDec = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wAttackPointAdd = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wDefensePointAdd = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->byAddOneLevel = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wMaxHPAdd = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wMaxMPAdd = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wMaxSPAdd = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->wTradeSearch = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0; 
// 		psi->byShortcutPick = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->byInterrupt = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wValidTime = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wActiveInterval = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wResHPAdd = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wResMPAdd = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wResSPAdd = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->byWeaponType = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wSkillID = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->byTelergyID = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->byWalkOnWaterID = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->byShieldID = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->dwScripID = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->byBindType = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->byShiness = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wShineIconID = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->byUsePrompt = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wBuffTable[0][0] = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wBuffTable[0][1] = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wBuffTable[1][0] = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->wBuffTable[1][1] = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;
// 		psi->dwLocSrvCoolingTime = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;		//冷却时间
// 		psi->wLocSrvKindNumberForCD = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;		//冷却类别
// 		psi->bSex = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;						//性别
// 		psi->dwLocSrvPreTime = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;			//物品使用预处理时间
// 		psi->dwLocSrvBreakOdds = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;			//物品使用预处理被打断几率
// 		psi->wItemChangeFlag = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;			// 物品改变标志
// 		psi->wChangedItemID1 = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;			// 物品改变为1
// 		psi->dwItemChangeOdds1 = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;			// 物品改变为1几率
// 		psi->wChangedItemID2 = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;			// 物品改变为2
// 		psi->dwItemChangeOdds2 = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;			// 物品改变为2几率
// 		psi->dwItemChangePeriod = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;			// 物品发生改变的周期
// 		stream >> psi->wItemDes;														// 物品说明
// 
// 		if ( psi->wItemID >= MAX_ITEMSCOPE || ( psi->wItemID == 0 && *( LPDWORD )str != *( LPDWORD )"金钱" ) )
// 			return false;
// 
//         // 目前暂时通过这个数值设定特殊重叠类道具的重叠上限，最大9999
//         if ( ITEM_IS_OVERLAP( psi->byType ) )
//         {
//             if ( psi->wOverlay > 9999 )
//                 return false;
//             if ( psi->wOverlay == 0 )
//                 psi->wOverlay = 9999;
//         }
// 
// 		dwt::strcpy( psi->szName, str, sizeof( psi->szName ) );
//         s_ppItemDatas[psi->wItemID] = std::pair< DWORD, const SItemData* >( 
//             ( crcInit 
//             ? _GetCrc32( ( LPVOID )static_cast< const ClientVisible* >( psi ), sizeof( ClientVisible ) ) 
//             : 0 ), psi );
// 
//         if ( itemNameMap.find( psi->szName ) == itemNameMap.end() )
//             itemNameMap[ psi->szName ] = psi->wItemID;
//         else
//             rfalse( 2, 1, "发现重复的道具名[%s]！", psi->szName );
// 
// 		nTimes++;
// 
//     }
//     stream.close();
//     return 1;
}

// BOOL CItemData::UpdateItemData( int index, const SItemData *itemData, BOOL fullUpdate )
// {
    // 数组范围限定，避免overrun
//     if ( (DWORD)index >= MAX_ITEMSCOPE )
//         return FALSE;
// 
//     if ( itemData == NULL )
//     {
//         // 恢复默认设置
//         for ( int i = ( ( itemData = NULL ), 0 ); i < s_lItemSize; i ++ )
//         {
//             if ( s_pItemDatas[i].wItemID != index )
//                 continue;
// 
//             itemData = &s_pItemDatas[i];
//             break;
//         }
//     }
//     else if ( itemData->wItemID != index )
//     {
//         // itemData->wItemID == -1 时为删除该项配置！
//         if ( itemData->wItemID != 0xffff )
//             return FALSE;
// 
//         itemData = NULL;
//     }
//     else if ( itemData != NULL )
//     {
//         SItemData item = *itemData;
// 
//         // 如果未标记 fullUpdate 的话，则表示只是部分更新（用于客户端同步）
//         if ( !fullUpdate )
//         {
//             const ClientInvisible *buffer = ( ClientInvisible* )s_ppItemDatas[index].second;
//             if ( buffer != NULL )
//                 memcpy( &item, buffer, sizeof( ClientInvisible ) );
//         }
// 
//         // 新增加或修改的道具项！
//         extraItemDatas[ index ] = item;
//         itemData = &extraItemDatas[ index ];
//     }
// 
//     s_ppItemDatas[index] = std::pair< DWORD, const SItemData* >( 
//         ( ( itemData != NULL ) 
//         ? _GetCrc32( ( LPVOID )static_cast< const ClientVisible* >( itemData ), sizeof( ClientVisible ) ) 
//         : -1 ), itemData );

//     return TRUE;
// }

// std::pair< DWORD, const SItemData* >* CItemData::GetRawData( int index )
// {
// 	if ( ( DWORD )index >= MAX_ITEMSCOPE )
//         return NULL;
// 
//     return &s_ppItemDatas[index];
// }

// const SItemData* CItemData::GetItemData( int index )
// {
// 	if ( index >= MAX_ITEMSCOPE ) 
//         return NULL;
// 
//     return s_ppItemDatas[index].second;
// }

namespace Item
{

    static struct RAII
    {
        ~RAII()
        {
            ClearItemData();
        }

    } resource_acquisition_is_initialization;
}

