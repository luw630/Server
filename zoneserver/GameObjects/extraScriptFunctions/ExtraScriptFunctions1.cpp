#include "StdAfx.h"
#include <WinDef.h>
#include "ExtraScriptFunctions.h"
#include "lite_lualibrary.hpp"

#include "pub/wndcusop.h"
#include "pub/traceinfo.h"

#include "gameobjects/region.h"
#include "gameobjects/monster.h"
#include "gameobjects/player.h"
#include "gameobjects/area.h"
#include "gameobjects/building.h"
#include "gameobjects/npc.h"
#include "gameobjects/item.h"
#include "gameobjects/gameworld.h"
#include "gameobjects/globalfunctions.h"
#include "gameobjects/scriptmanager.h"
#include "gameobjects/building.h"
#include "gameobjects/factionmanager.h"
#include "gameobjects/dynamicregion.h"
#include "gameobjects/trigger.h"
#include "gameobjects/item.h"
#include "gameobjects/prisonex.h"
#include "gameobjects/prison.h"
#include "GameObjects/plugingame.h"
#include "GameObjects/Player.h"


#include "networkmodule/itemmsgs.h"
#include "networkmodule/scriptmsgs.h"
#include "networkmodule/tongmsgs.h"
#include "networkmodule/regionmsgs.h"
#include "networkmodule/cardpointmodifymsgs.h"
#include "networkmodule/refreshmsgs.h"
#include "networkmodule/accountmsgs.h"
#include "networkmodule/MoveMsgs.h"
#include "Networkmodule/UnionMsgs.h"
#include "Networkmodule/refreshmsgs.h"

#include "pub/rpcop.h"
#include "���������/Environment.h"
#include "NetWorkModule/PlayerTypedef.h"

extern LPIObject FindRegionByGID( DWORD GID );
extern LPIObject FindRegionByID( DWORD ID );
extern BOOL GenerateNewUniqueId( SItemBase &item );
// extern BOOL GetFactionHeaderInfo( LPCSTR szFactionName, SFactionData::SFaction &factionHeader );
// extern BOOL GetFactionHeaderInfo( WORD factionId, SFactionData::SFaction &factionHeader );
extern BOOL UpdateFaction( SFactionData::SFaction &faction );
extern CScriptManager g_Script;
extern BYTE commBuffer[ 0xfff ];

extern DWORD g_masterTask;
extern DWORD g_prenticeTask;
extern DWORD g_mastercount;

extern DWORD TopFiveSID[ 5 ][ 5 ]; // ��1: ����ǰ��, ��2: ����ǰ��, ��3: �Թ�ǰ��, ��4: ��ͼǰ��, ��5: ��ǰ��
extern DWORD TopFiveVals[ 5 ][ 5 ];  
extern std::string  TopFiveName[ 5 ][ 5 ];

extern BOOL PutPlayerIntoDestRegion(CPlayer *pPlayer, WORD wNewRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID = 0);
//extern void GetAddAttribute( SEquipment::Attribute::Unit *unit, WORD id, int maxNum, WORD coff[ 57 ] = NULL, BYTE level = 0, BYTE itemType = 0 );

extern std::map<std::string, DWORD> GMTalkMask; // ��GM���Ե����ӳ���
extern std::map<std::string, DWORD> KingTalkMask; // ���������Ե����ӳ���
extern std::map<std::string, DWORD> MasterTalkMask;  // �������˽��Ե����ӳ���
extern std::map< std::string, SFactionData > factionManagerMap;  // ����DATA������ӳ���

// ��İ��ɵ�����
extern BOOL GetFactionInfo( LPCSTR szFactionName, SFactionData& stFactionInfo );

static BOOL __validate = FALSE;
static double __number = 0;
static LPCSTR __string = NULL;

#define _IF_GET_FIELD_NUMBER( _table, _key )        if ( lua_getfield( L, _table, _key ), \
    ( lua_isnumber( L, -1 ) ? ( __number = lua_tonumber( L, -1 ), __validate = TRUE ) : \
    ( __number = 0, __validate = FALSE ) ), lua_pop( L, 1 ), __validate )

#define _IF_GET_FIELD_STRING( _table, _key )        if ( lua_getfield( L, _table, _key ), \
    ( lua_isstring( L, -1 ) ? ( __string = lua_tostring( L, -1 ), __validate = TRUE ) : \
    ( __string = NULL, __validate = FALSE ) ), lua_pop( L, 1 ), __validate )

#define _GET_FIELD_NUMBER_DIRECTLY( _table, _key )  ( lua_getfield( L, _table, _key ), \
    ( lua_isnumber( L, -1 ) ? ( __number = lua_tonumber( L, -1 ), __validate = TRUE ) : \
    ( __number = 0, __validate = FALSE ) ), lua_pop( L, 1 ), __number )

#define _GET_FIELD_STRING_DIRECTLY( _table, _key )  ( lua_getfield( L, _table, _key ), \
    ( lua_isstring( L, -1 ) ? ( __string = lua_tostring( L, -1 ), __validate = TRUE ) : \
    ( __string = NULL, __validate = FALSE ) ), lua_pop( L, 1 ), __string )

#define _GET_ARRAY_NUMBER_DIRECTLY( _table, _index ) ( ( lua_rawgeti( L, _table, _index ), \
    ( __validate = lua_isnumber( L, -1 ) ), ( __number = lua_tonumber( L, -1 ) ), lua_pop( L, 1 ) ), __number )


void RecvTransferMsg( SAGameScriptTransfreMsg *pMsg )
{
    if ( pMsg == NULL )
        return ;

    DWORD size1 = *(DWORD*)(&pMsg->streamData[0] );
    size1 += 4;
    if ( size1 > 4096 )
        return ;

    lite::Variant var( pMsg->streamData, size1 );
    LuaFunctor( g_Script, "OnTransfreData" )[ var ]();
}

int CBuilding::lua_PushTable( struct lua_State *L )
{
    // ȷ���ǻ�ȡ�������ݻ��ǽű���չ����
    if ( lua_isboolean( L, 1 ) && lua_toboolean( L, 1 ) )
    {
        lua_createtable( L, 0, ( int )svarMap.size() + 10 );
        if ( !lua_istable( L, -1 ) )
            return 0;

        lua_pushstring( L, "_inverse" ); 
        lua_pushnumber( L, property.inverse ); 
        lua_settable( L, -3 );

        lua_pushstring( L, "_uniqueId" ); 
        //QWORD uniqueId = property.uniqueId();
        //lua_pushnumber( L, reinterpret_cast< double& >( uniqueId ) ); 
        //lua_settable( L, -3 );
        luaEx_pushint63( L, property.uniqueId() );
        lua_settable( L, -3 );

        lua_pushstring( L, "_index" ); 
        lua_pushnumber( L, property.index ); 
        lua_settable( L, -3 );

        lua_pushstring( L, "_owner" ); 
        lua_pushstring( L, property.owner.c_str() ); 
        lua_settable( L, -3 );

        // return 1;
    }
    else
    {
        if ( svarMap.empty() )
            return 0;

        lua_createtable( L, 0, ( int )svarMap.size() );
        if ( !lua_istable( L, -1 ) )
            return 0;
    }

    for ( std::map< std::string, lite::Variant >::iterator it = svarMap.begin(); it != svarMap.end(); it ++ )
    {
        lua_pushstring( L, it->first.c_str() ); 
        reinterpret_cast< lite::lua_variant& >( it->second ).push( L );
        lua_settable( L, -3 );
    }

    return 1;
}

int CBuilding::lua_TabUpdate( struct lua_State *L, int idx )
{
    // ע��, ������һ�����, Ҳ����ζ�ű��β�������ȫ������������!
    if ( lua_isboolean( L, 2 ) && lua_toboolean( L, 2 ) )
        svarMap.clear();

    lite::table_iterator it( L, idx );
	if ( !it.isValid() )
		return 0;

    DWORD prevState = property._state;

	while ( it ++ )
	{
		// ͨ�� map ֻ����ؼ���Ϊ�ַ������͵�����!
        if ( lua_isstring( L, it.index_of_key ) && !lua_isnil( L, it.index_of_value ) )
        {
            LPCSTR key = lua_tostring( L, it.index_of_key );
            if ( *key == '_' )
            {
                // �����״̬��ʾ���»������ݣ����ǽű���չ���ݣ�
                if ( strcmp( key, "_usingState" ) == 0 )
                    property.usingState = ( DWORD )lua_tonumber( L, it.index_of_value );
				else if ( strcmp( key, "_showTitle" ) == 0   )
					property.showTitle = ( DWORD )lua_tonumber( L, it.index_of_value );
            }
            else
            {
                reinterpret_cast< lite::lua_variant& >( svarMap[ key ] ).get( L, it.index_of_value );
            }
        }
	}

#undef new
    if ( prevState != property._state )
    {
        size_t size = sizeof( commBuffer );
        SASynBuildingMsg &msg = * new ( commBuffer ) SASynBuildingMsg;
        msg.gid = GetGID();

		try
		{
			lite::Serializer slm( msg.append, size - sizeof( SASynBuildingMsg ) );
			slm [ 1 ]( ( DWORD )property._state );
			size = ( WORD )( size - slm.EndEdition() );
		}
		catch ( lite::Xcpt & )
		{
		}

        checkId ++;

        Synchro( commBuffer, size );
    }

    return 0;
}

int CBuilding::lua_VarUpdate( struct lua_State *L, int idx )
{
	// ͨ�� map ֻ����ؼ���Ϊ�ַ������͵�����!
    if ( !lua_isstring( L, idx ) )
        return 0;

    // ���û�����ݣ����ʾɾ���ñ�����
    if ( lua_isnil( L, idx + 1 ) )
    {
        svarMap.erase( lua_tostring( L, idx ) );
        return 0;
    }

    LPCSTR key = lua_tostring( L, idx );
    lite::Variant &variant = svarMap[ key ];

    if ( variant.dataType != VT_EMPTY )
    {
        variant.~Variant(); // ����, ��Ҫ��Ϊ���ͷ�lua��������ʱ��������!
        memset( &variant, 0, sizeof( lite::Variant ) );
    }

    reinterpret_cast< lite::lua_variant& >( variant ).get( L, idx + 1 );

    return 0;
}

// ��Ϊ�ṹ����������Ԫ�������޷������ֿռ�������
struct ExtraLuaFunctions
{
	struct Details
    {
	    static void CreateFolder( LPCSTR path );
        static DWORD HexStringToNumber( LPCSTR str );
    };

    static CRegion* GetRegionById( DWORD regionId );

	static int L_GetBuildingData( lua_State *L )
    {
        if ( g_Script.m_pBuilding == NULL )
            return 0;

        return g_Script.m_pBuilding->lua_PushTable( L );
    }

	static int L_SetBuildingData( lua_State *L )
    {
        if ( g_Script.m_pBuilding == NULL )
            return 0;

        return g_Script.m_pBuilding->lua_TabUpdate( L, 1 );
    }

	static int L_UpdateBuildingData( lua_State *L )
    {
        if ( g_Script.m_pBuilding == NULL )
            return 0;

        return g_Script.m_pBuilding->lua_VarUpdate( L, 1 );
    }

	static int L_GetBuildingCurPos( lua_State *L )
    {
        if ( g_Script.m_pBuilding == NULL )
            return 0;

        WORD wX = 0;
        WORD wY = 0;
        DWORD wRegionID = 0;
        g_Script.m_pBuilding->GetCurPos( wX, wY );
        if ( CRegion *region = g_Script.m_pBuilding->m_ParentRegion )
        {
            if ( region->DynamicCast( IID_DYNAMICREGION ) )
                wRegionID = region->GetGID();
            else
                wRegionID = region->m_wRegionID;
        }

        lua_pushnumber(L, wX);
        lua_pushnumber(L, wY);
        lua_pushnumber(L, wRegionID);
    
        return 3;
    }

	static int L_StartBuild( lua_State *L )
	{
		int index = static_cast<int>( lua_tonumber( L, 1 ) );
		if ( index == 0 )
			return 0;

		const SBuildingConfigData *buidlingData = CBuilding::GetBuildingData( index );
		if ( buidlingData == NULL )
			return 0;

		WORD size = sizeof( commBuffer );

		SASynBuildingMsg &msg = * new ( commBuffer ) SASynBuildingMsg;
		msg.gid = 0; // �����ͻ��˿�ʼbuilding
		
		try
		{
			lite::Serializer slm( msg.append, size - sizeof( SASynBuildingMsg ) );
			slm( ( int )buidlingData->index );
			slm( ( DWORD )buidlingData->image );
			slm( ( int )buidlingData->centerx )( ( int )buidlingData->centery );
			slm( buidlingData->qwtileMask );

            g_Script.m_pPlayer->buildVerify = buidlingData->index;

			g_StoreMessage( g_Script.m_pPlayer->m_ClientIndex, &msg, ( WORD )( size - slm.EndEdition() ) );
		}
		catch ( lite::Xcpt & )
		{
		};

		lua_pushnumber( L, TRUE	);
		return 1;
	}

	static int L_SetBuildTrap( lua_State *L )
	{
// 		if ( g_Script.m_pBuilding == NULL )
// 			return 0;
// 
// 		if ( g_Script.m_pBuilding->m_ParentRegion == NULL )
// 			return 0;
// 
// 		WORD order = static_cast<WORD>( lua_tonumber( L, 1 ) );
// 		int x = static_cast<int>( lua_tonumber( L, 2 ) );		// ����Ϊ��������(8*8������)
// 		int y = static_cast<int>( lua_tonumber( L, 3 ) );		// ����Ϊ��������(8*8������)
// 
// 		if ( abs( x ) >= 8 || abs( y ) >= 8 )
// 			return 0;
// 
// 		x = g_Script.m_pBuilding->m_wCurX + x;
// 		y = g_Script.m_pBuilding->m_wCurY + y;
// 
// 		CTrigger Trap;
// 		Trap.m_dwType = CTrigger::TT_MAPTRAP_BUILDING; 
// 		Trap.SMapEvent_Building.buildingId = g_Script.m_pBuilding->GetUniqueId();
//         Trap.SMapEvent_Building.wCurX = x;
//         Trap.SMapEvent_Building.wCurY = y;
// 
// 		if ( !g_Script.m_pBuilding->m_ParentRegion->SetTrap( order, x, y, &Trap ) )
// 			return 0;
// 
// 		return lua_pushnumber( L, 1 ), 1;
		return 0;
	}

	static int L_SetFactionInfo( lua_State *L )
	{
        LPCSTR factionName = lua_tostring( L, 1 );
		if ( factionName == NULL )
			return 0;

		SFactionData::SFaction faction;
// 		if ( !GetFactionHeaderInfo( factionName, faction ) )
// 			return 0;

		//if ( g_Script.m_pPlayer == NULL )
		//	return 0;

		//SFactionData::SFaction faction;
		//if ( !GetFactionHeaderInfo( g_Script.m_pPlayer->m_Property.m_szTongName, faction ) )
		//	return 0;

		if ( lua_istable( L, 2 ) )
		{
			_IF_GET_FIELD_NUMBER( 2, "parentId" )
				faction.buildingStub.parentId = static_cast< DWORD >( __number );

			//_IF_GET_FIELD_NUMBER( 2, "uniqueId" )
			//	faction.buildingStub.uniqueId = reinterpret_cast< QWORD& >( __number );

            {
                unsigned __int64 uid = 0;
                lua_getfield( L, 2, "uniqueId" );
                __validate = luaEx_isint63( L, -1 );
                if ( __validate )
                    uid = luaEx_toint63( L, -1 );
                lua_pop( L, 1 );
                if ( __validate )
                    faction.buildingStub.uniqueId = uid;
            }

            _IF_GET_FIELD_NUMBER( 2, "level" )
                faction.byFactionLevel = static_cast< BYTE >( __number );

            _IF_GET_FIELD_NUMBER( 2, "prosperity" )
                faction.dwFactionProsperity = static_cast< DWORD >( __number );
			
			return lua_pushnumber( L, UpdateFaction( faction ) ), 1;
		}
		return 0;
	}

	static int L_GetFactionInfo( lua_State *L )
	{
		SFactionData::SFaction faction;
        faction.factionId = 0;

        if ( lua_type( L, 1 ) == LUA_TSTRING )
        {
		    LPCSTR factionname = lua_tostring( L, 1 );
// 		    if ( factionname )
// 		        GetFactionHeaderInfo( factionname, faction );
        }
        else if ( lua_type( L, 1 ) == LUA_TNUMBER )
        {
		    DWORD factionId = ( DWORD )lua_tonumber( L, 1 );
// 		    if ( factionId != 0 && factionId < 0xffff )
// 		        GetFactionHeaderInfo( ( WORD )factionId, faction );
        }
        else if ( lua_type( L, 1 ) == LUA_TNONE )
        {
// 	        if ( g_Script.m_pPlayer && g_Script.m_pPlayer->m_Property.m_szTongName[0] )
// 		        GetFactionHeaderInfo( g_Script.m_pPlayer->m_Property.m_szTongName, faction );
        }

        if ( faction.factionId == 0 )
            return 0;

		lua_createtable( L, 10, 0 );
        if ( !lua_istable( L, -1 ) )
            return 0;

        lua_pushstring( L, "parentId" ); 
        lua_pushnumber( L, ( DWORD )faction.buildingStub.parentId ); 
        lua_settable( L, -3 );

		lua_pushstring( L, "uniqueId" ); 
        //QWORD uniqueId = faction.buildingStub.uniqueId;
        //lua_pushnumber( L, reinterpret_cast< double& >( uniqueId ) ); 
        //lua_settable( L, -3 );
        luaEx_pushint63( L, faction.buildingStub.uniqueId );
        lua_settable( L, -3 );

		lua_pushstring( L, "factionId" ); 
        lua_pushnumber( L, static_cast< WORD >( faction.factionId ) ); 
        lua_settable( L, -3 );

		lua_pushstring( L, "factionName" ); 
        lua_pushstring( L, faction.szFactionName ); 
        lua_settable( L, -3 );

		lua_pushstring( L, "membernum" ); 
        lua_pushnumber( L, static_cast< BYTE >( faction.byMemberNum ) ); 
        lua_settable( L, -3 );

		lua_pushstring( L, "npcnum" ); 
		lua_pushnumber( L, static_cast< BYTE >( faction.byNpcNum ) ); 
        lua_settable( L, -3 );

		lua_pushstring( L, "openRApplyJoin" ); 
		lua_pushnumber( L, static_cast< BYTE >( faction.byOpenRApplyJoin ) ); 
        lua_settable( L, -3 );

	    lua_pushstring( L, "creatorname" );
	    lua_pushstring( L, faction.szCreatorName );
	    lua_settable( L, -3 );

	    lua_pushstring( L, "memo" );
	    lua_pushstring( L, faction.szFactionMemo );
	    lua_settable( L, -3 );

	    lua_pushstring( L, "level" );
	    lua_pushnumber( L, faction.byFactionLevel );
	    lua_settable( L, -3 );

	    lua_pushstring( L, "prosperity" );
	    lua_pushnumber( L, faction.dwFactionProsperity );
	    lua_settable( L, -3 );

	    lua_pushstring( L, "money" );
	    lua_pushnumber( L, faction.iMoney );
	    lua_settable( L, -3 );

	    lua_pushstring( L, "wood" );
	    lua_pushnumber( L, faction.dwWood );
	    lua_settable( L, -3 );

	    lua_pushstring( L, "stone" );
	    lua_pushnumber( L, faction.dwStone );
	    lua_settable( L, -3 );

	    lua_pushstring( L, "mine" );
	    lua_pushnumber( L, faction.dwMine );
	    lua_settable( L, -3 );

	    lua_pushstring( L, "paper" );
	    lua_pushnumber( L, faction.dwPaper );
	    lua_settable( L, -3 );

	    lua_pushstring( L, "jade" );
	    lua_pushnumber( L, faction.dwJade );
	    lua_settable( L, -3 );

		return 1;
	}

	static int L_DeleteFaction( lua_State *L )
	{
		LPCSTR factionname = lua_tostring( L, 1 );
		if ( factionname == NULL )
			return 0;

		/*if ( GetGW()->m_FactionManager.DeleteFaction( factionname ) )
			return lua_pushnumber( L, 1 ), 1;*/

		return 0;
	}

    static int L_GetFactionIdList( lua_State *L )
    {
        std::list< WORD > factionIds;
        CFactionManager::GetFactionIds( factionIds );

        lua_createtable( L, ( int )factionIds.size(), 0 );
        if ( !lua_istable( L, -1 ) )
            return 0;

        int index = 1;
        for ( std::list< WORD >::iterator it = factionIds.begin();
            it != factionIds.end(); it ++ )
        {
            lua_pushnumber( L, *it );
            lua_rawseti( L, -2, index ++ );
        }

		return 1;
    }

	static int L_GetGroupID( lua_State *L )
	{
		extern DWORD GetGroupID();
		return lua_pushnumber( L, GetGroupID() ), 1;
	}

    static int L_RemoveBuiding( lua_State *L )
    {
        if ( g_Script.m_pBuilding == NULL )
			return 0;

        if ( g_Script.m_pBuilding->m_ParentRegion == NULL )
			return 0;

        QWORD uid = luaEx_toint63( L, 1 );
        //double temp = lua_tonumber( L, 1 );
        //QWORD uid = reinterpret_cast< QWORD& >( temp );

        if ( g_Script.m_pBuilding->m_ParentRegion->RemoveBuilding( uid ) )
            return 0;

        return lua_pushnumber( L, 1 ), 1;

    }

        // ��������
    static int L_IsPassVenationEx( lua_State *L )
    {
        if ( g_Script.m_pPlayer == NULL )
			return 0;

        BYTE temp = (BYTE)lua_tonumber( L, 1 );
        BYTE temp2 = (BYTE)lua_tonumber( L, 2 );
        bool ret = g_Script.m_pPlayer->IsPassVenationEx( temp, temp2 );
        if ( ret )
            return lua_pushnumber( L, 1 ), 1;
        return 0;
    }

    // ȡ�õ�ǰѨ������
    static int L_GetVenapointExNum( lua_State *L )
    {
        if ( g_Script.m_pPlayer == NULL )
			return 0;

        // ��������
        BYTE venapointType = ( BYTE )lua_tonumber( L, 1 );

        BYTE ret = g_pVenapointData->GetVenapointCountEx( venapointType );
        if ( ret == 0 )
            return 0;

        return lua_pushnumber( L, ret ), 1;
    }

    static int L_IsPassVenation( lua_State *L )
    {
        if ( g_Script.m_pPlayer == NULL )
			return 0;

        BYTE venapointType = ( BYTE )lua_tonumber( L, 1 );
        if ( g_Script.m_pPlayer->IsPassVenation( venapointType ) )
            return lua_pushnumber( L, 1 ), 1;
        else
            return 0;
    }

    // ���ô�����
    static int L_SetVenapointEx( lua_State *L )
    {
		return 0;
    }

    // ���ó�Ѩ��
    static int L_SetVenapointCount( lua_State *L )
    {
//         if ( g_Script.m_pPlayer == NULL )
//             return 0;
// 
//         int value = ( int )lua_tonumber( L, 1 );
// 
//         if ( ( value < 0 && g_Script.m_pPlayer->m_Property.m_byVenapointCount > 0 ) || ( value > 0 && g_Script.m_pPlayer->m_Property.m_byVenapointCount < 255 ) )
//         {
//             g_Script.m_pPlayer->m_Property.m_byVenapointCount += value;
//             return lua_pushnumber( L, 1 ), 1;
//         }

        return 0;
    }

    static int L_SetDDialAward( lua_State *L )
    {
//         if ( g_Script.m_pPlayer == NULL )
//             return 0;
// 
//         // �ú���ֻ��Ψһ�Ĳ���,һ������������Ϣ�ı�!
//         BYTE type =  ( BYTE )lua_tonumber( L, 1 );
//         if ( type == 1 )
//         {
//             int size = 0;
//             void *data = luaEx_touserdata( L, 2, &size );
// 
//             if ( size != sizeof( SPackageItem ) )
//                 return 0;
// 
//             SPackageItem &item = *( SPackageItem* )data;
//             if ( item.wIndex == 0 )
//                 return 0;
// 
//             const SItemData *itemdata = CItem::GetItemData( item.wIndex );
//             if ( itemdata == NULL )
//                 return 0;
// 
//             g_Script.m_pPlayer->m_Property.unitItem.type = type;
//             g_Script.m_pPlayer->m_Property.unitItem.itemBuffer = ( SRawItemBuffer )item;
//             return lua_pushnumber( L, 1 ), 1;
//         }
//         else if ( type == 2 )
//         {
//             int value = ( int )lua_tonumber( L, 2 );
//             g_Script.m_pPlayer->m_Property.unitOther.type = type;
//             g_Script.m_pPlayer->m_Property.unitOther.value = value;
//             return lua_pushnumber( L, 1 ), 1;
//         }

        return 0;
    }


    // ��ӵ��ߵ�ת��
    static int L_ShowDDial( lua_State *L )
    {
//         if ( g_Script.m_pPlayer == NULL )
//             return 0;
// 
//         BYTE showType =  ( BYTE )lua_tonumber( L, 2 );  // ��2��������������/Կ������ ��0��ͨ 1�ƽ� 2���� 3������� 4Ԫ����
// 
//         // �����һ�εĽ�Ʒ��δ��ȡ,���װ�Ϊ���µģ�����ǰ�ģ�
//         if ( g_Script.m_pPlayer->m_Property.base.type )
//         {
//             size_t size = sizeof( commBuffer );
//             SADialMsg &msg = * new ( commBuffer ) SADialMsg;
//             msg.type = SADialMsg::DIAL_SHOWAWARD;
// 	    msg.index = showType;
// 
//             try
//             {
//                 lite::Serializer slm( msg.buffer, size - sizeof( SADialMsg ) );
//                 if ( g_Script.m_pPlayer->m_Property.base.type == 1 )
//                     slm( &g_Script.m_pPlayer->m_Property.unitItem, sizeof( g_Script.m_pPlayer->m_Property.unitItem ) );
//                 else if ( g_Script.m_pPlayer->m_Property.base.type == 2 )
//                     slm( g_Script.m_pPlayer->m_Property.unitOther.value );
// 
//                 size = ( WORD )size - slm.EndEdition();
//             }
//             catch ( lite::Xcpt & )
//             {
//             }
// 
//             g_Script.m_pPlayer->SendMsg( &msg, size );
//             return 0;
//         }
// 
//         static SPlayerDialUnit unit[16];
//         memset( &unit, 0, sizeof( unit ) );
// 
//         // �ú���ֻ��Ψһ�Ĳ���,һ������������Ϣ�ı�!(������2������2����ʾ��ͨ���ӣ����ǻƽ���..���Ǳ��....)
//         int size = 0;
//         void *data = luaEx_touserdata( L, -1, &size );
// 
//         int count = 0;
//         size_t len = lua_objlen( L, 1 );
//         for ( DWORD i = 0; i < len; i ++ )
//         {
//             lua_rawgeti( L, 1, i + 1 );
//             if ( lua_istable( L, -1 ) ) 
//             {
//                 BYTE type = static_cast< BYTE >( _GET_FIELD_NUMBER_DIRECTLY( -1, "type" ) );
// 
//                 // װ����
//                 if ( type == 1 )
//                 {
//                     int size = 0;
//                     lua_getfield( L, -1, "unit" );
//                     void *data = luaEx_touserdata( L, -1, &size );
//                     lua_pop( L, 1 );
//                     
//                     if ( size != sizeof( SPackageItem ) )
//                         continue;
// 
//                     SPackageItem &item = *( SPackageItem* )data;
//                     if ( item.wIndex == 0 )
//                         continue;
// 
//                     unit[i].unitItem.type = 1;
//                     unit[i].unitItem.itemBuffer = ( SRawItemBuffer )item;
//                     count ++;
//                 }
//                 // ��������
//                 else if ( type == 2 )
//                 {
//                     int value = static_cast< int >( _GET_FIELD_NUMBER_DIRECTLY( -1, "unit" ) );    
//                     unit[i].unitOther.type = 2;
//                     unit[i].unitOther.value = value;
//                     count ++;
//                 }
//             }
//             lua_pop( L, 1 );
//         }
// 
//         size = sizeof( commBuffer );
//         SADialMsg &msg = *new ( commBuffer )SADialMsg;
//         msg.type = SADialMsg::DIAL_SHOW;
//         msg.index = showType;
// 
//         try
//         {
//             lite::Serializer slm( msg.buffer, size - sizeof( SADialMsg ) );
//             slm( count );
//             for ( int i = 0; i < count; i++ )
//                 slm( &unit[i], sizeof( unit[i] ) );
// 
//             size = ( int )( ( WORD )size - slm.EndEdition() );
//         }
//         catch ( lite::Xcpt & )
//         {
//         }
// 
//         g_Script.m_pPlayer->SendMsg( &msg, size );
// 
//         return lua_pushnumber( L, 1 ), 1;

		return 0;
    }

    static int L_GetSkill( lua_State *L )
    {
		return 0;
    }
	
	/// ֪ͨ�ͻ����Զ��ƶ���ĳ���ط�
	static int L_PlayerMoveTo( lua_State *L )
    {
        if ( g_Script.m_pPlayer == NULL )
            return 0;

		WORD x = static_cast< WORD >( lua_tonumber( L, 1 ) );
		WORD y = static_cast< WORD >( lua_tonumber( L, 2 ) );

		SASynWayTrackMsg msg;
		msg.dwGlobalID = 0;
		msg.ssw.byAction = 255; // ���⴦��\�Զ��ƶ�
		msg.ssw.wSegX = x;
		msg.ssw.wSegY = y;

		g_Script.m_pPlayer->SendMsg( &msg, sizeof( msg ) );

		return 0;
	}

	/// ����ϵͳ����
	// ���� (��������,��ɫrgb,����[����100��])
	// 1��ϵͳ�������� 2���ٱ������ 3���ٱ���˵��
	static int L_SetAffiche( lua_State *L )
	{
		WORD num = static_cast< WORD >( lua_tonumber( L, 1 ) );
		WORD r = static_cast< WORD >( lua_tonumber( L, 2 ) );
		WORD g = static_cast< WORD >( lua_tonumber( L, 3 ) );
		WORD b = static_cast< WORD >( lua_tonumber( L, 4 ) );
		LPCSTR str = lua_tostring( L, 5 );

		BYTE type = static_cast< BYTE >( lua_tonumber( L, 6 ) );

		SAAfficheMsg msg;
        msg.scrollNum = ( BYTE )num;
		msg.color[0] = ( BYTE )r;
		msg.color[1] = ( BYTE )g;
		msg.color[2] = ( BYTE )b;
        dwt::strcpy( msg.cChatData, str, sizeof( msg.cChatData ) );

		switch( type )
		{
		case 1:
			msg.byType = SAChatGlobalMsg::ECT_AFFICHE;
			break;

        case 2:
			msg.byType = SAChatGlobalMsg::ECT_TREASURE_SCROLL;
			break;

        case 3:
			msg.byType = SAChatGlobalMsg::ECT_TREASURE_EXPAIN;
			msg.color[0] = 0;
			msg.color[1] = 0;
			msg.color[2] = 0;
			msg.scrollNum = 0;
			break;
		}

		BroadcastMsg( &msg, sizeof( msg ) );

		return 0;
	}

    // ���ȡ�������������sid
    static int L_GetRandomPlayerSID( lua_State *L )
    {
//         if ( g_Script.m_pPlayer == NULL )
//             return 0;
//         // ȡ������
//         DWORD value = ( DWORD )lua_tonumber( L, 1 );    // ����
//         std::map<DWORD, DWORD>::iterator iter = GetGW()->m_PlayerSIDMap.begin();
//         std::vector< DWORD > list;
//         for ( iter; iter != GetGW()->m_PlayerSIDMap.end(); iter++ )
//         {
//             CPlayer *player = ( CPlayer * )GetPlayerByGID( iter->second )->DynamicCast( IID_PLAYER );
//             if ( player == NULL )
//                 continue;
// 
//             if ( g_Script.m_pPlayer->GetSID() != player->GetSID() && 
//                 ( player->m_Property.m_Level > value ) && ( player->m_Property.prenticeName[0] == 0 ) )
//                 list.push_back( player->GetSID() );
//         }
// 
//         if ( list.size() > 0 )
//         {
//             int rnd = ( int )( rand() % list.size() );
//             lua_pushnumber( L, list.at( rnd ) );
//             return 1;
//         }

        return 0;
    }

    static int L_SetMasterPrenticeName( lua_State *L )
    {
//         if ( g_Script.m_pPlayer == NULL )
//             return 0;
// 
//         BYTE type  = ( BYTE )lua_tonumber( L, 1 );       // ʦ�������
//         LPCSTR name = ( LPCSTR )lua_tostring( L, 2 );
//         if ( name == 0 )
//             return 0;
// 
//         if ( type == 0 ) // ʦ��
//             dwt::strcpy( g_Script.m_pPlayer->m_Property.masterName, name, CONST_USERNAME );
//         else
//             dwt::strcpy( g_Script.m_pPlayer->m_Property.prenticeName, name, CONST_USERNAME );
// 
//         g_Script.m_pPlayer->SendMyState();
        return 0;
    }

	/** ��ͻ��˵�����������Ի���
		������	1. destID: ��ĳĳ��ҽ����ID
	*/
	static int L_OnOpenUnionDialog( lua_State *L )
	{
		if ( g_Script.m_pPlayer == NULL )
			return 0;

		DWORD destID = ( DWORD )lua_tonumber( L, 1 );

		SACreateUnionMsg msg;
		msg.destID = destID;
		g_StoreMessage( g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof( SACreateUnionMsg ) );
		lua_pushnumber( L, 1 );	
		return 1;
	}

	/** ��ӽ�ɫ������
	*/
	static int L_AddPlayerToUnion( lua_State *L )
	{
		DWORD destID = ( DWORD )lua_tonumber( L, 1 );
		const char *unionName = static_cast<const char*>( lua_tostring( L, 2 ) );
		if( !unionName )
			return 0;

		GetGW()->m_UnionManager.AddPlayerToUnion( destID, std::string( unionName ) );

		lua_pushnumber( L, 1 );	
		return 1;
	}

	/** ��ɫ�˳�����
	*/
	static int L_DeletePlayerFormUnion( lua_State *L )
	{
		DWORD destID = ( DWORD )lua_tonumber( L, 1 );
		GetGW()->m_UnionManager.DeletePlayerToUnion( destID );
		lua_pushnumber( L, 1 );	
		return 1;
	}

	/** ��ɫ�Ƿ��Ѿ�����
	*/
	static int L_IsHaveInUnion( lua_State *L )
	{
		DWORD destID = ( DWORD )lua_tonumber( L, 1 );
		int value = GetGW()->m_UnionManager.IsUnion( destID );

		lua_pushnumber( L, value );
		return 1;
	}

	/** ��ȡ�ֵܻ��Ա����
	*/
	static int L_GetUnionMenbetNum( lua_State *L )
	{
		DWORD destID = ( DWORD )lua_tonumber( L, 1 );
		int value = GetGW()->m_UnionManager.GetUnionMemberNumByPlayerID( destID );
		lua_pushnumber( L, value );
		return 1;
	}
	
	/** ����ҷ������������Ϣ
	*/
	static int L_OnInvitatoryUnion( lua_State *L )
	{
		if ( g_Script.m_pPlayer == NULL )
			return 0;

		DWORD destID = ( DWORD )lua_tonumber( L, 1 );

		SAInvitatoryUnionMsg msg;
		msg.playerID = g_Script.m_pPlayer->GetSID();
		memcpy(msg.playerName, g_Script.m_pPlayer->GetName(), CONST_USERNAME);

		CPlayer* pPlayer = ( CPlayer* )GetPlayerBySID( destID )->DynamicCast( IID_PLAYER );
		if( !pPlayer )
			return 0;

		pPlayer->SendMsg( &msg, sizeof( msg ) );
		lua_pushnumber( L, 1 );
		return 1;
	}
	static int L_SendTipsMsg( lua_State *L )
	{
		DWORD dwSID = ( DWORD )lua_tonumber( L, 3 );
		CPlayer *pPlayer = g_Script.m_pPlayer;
		if( dwSID!=0 ) pPlayer = (CPlayer *)GetPlayerBySID(dwSID)->DynamicCast(IID_PLAYER);
		if( pPlayer==0 )return 0;

		
		BYTE type = ( BYTE )lua_tonumber( L, 1 );
		SQALuaTipsMsg errMsg;
		errMsg.flags = (BYTE)type;
		size_t len=0;
		if( lua_type(L,2) == LUA_TSTRING )
		{
			errMsg.flags |= 0x80;
			const char* msg = lua_tostring(L,2);
			len = strlen(msg);
			memcpy(errMsg.streamData,msg,len);
			errMsg.streamData[len] = '\0';
		}
		else if( lua_type(L,2) == LUA_TNUMBER )
		{
			len = 4;
			errMsg.flags &= 0x7f;
			DWORD msgid = ( DWORD )lua_tonumber( L, 2 );
			itoa(msgid,errMsg.streamData,10);	
		}
		else
		{
			return 0;
		}

		pPlayer->SendMsg( &errMsg, offsetof(SQALuaTipsMsg,streamData) +  len+1 );
		lua_pushnumber( L, 1 );
		return 1;
	}
    static int L_SendLuaMsg( lua_State *L )
    {
        // ��3����������ȷ������Ϣ��Ҫ�����������͵����л�
        DWORD type = ( DWORD )lua_tonumber( L, 3 );

        SQALuaCustomMsg msg;
        ZeroMemory( msg.streamData, sizeof( msg.streamData ) );
        msg.flags = ( BYTE )( type & 8 );
        type = type & ~8;
        size_t size = 0;
        int top = lua_gettop( L );
        try
        {
            if ( msg.flags == 8 )
            {
                int rc = luaEx_serialize( L, 2, msg.streamData + 2, sizeof( msg.streamData ) - 2 );
                if ( rc <= 0 || rc > sizeof( msg.streamData ) - 2 )
                    return 0; // ����ʧ��

                *( LPWORD )msg.streamData = ( rc | 0x8000 );
                size = sizeof( msg.streamData ) - ( rc + 2 );
            }
            else
            {
                lite::Serializer slm(  msg.streamData, sizeof(  msg.streamData ) );

                lua_getfield( L, 2, "data" );
                size_t len = lua_objlen( L, -1 );
                if ( len <= 0 || !lua_istable( L, -1 ) )
                    return lua_pop( L, 1 ), 0;

                for ( DWORD i = 0; i < len; i ++ )
                {
                    lua_rawgeti( L, -1, i + 1 );
                    if ( !lua_isnil( L, -1 ) )  // ��Ч�����ݲű����������
                        slm( lite::lua_variant( L, -1 ) );
                    lua_pop( L, 1 );
                }

                lua_pop( L, 1 );

                // ƽ��������־!
                size = slm.EndEdition();
            }
        }
        catch ( lite::Xcpt & )
        {
            lua_settop( L, top );
            return 0;
        }

        if ( lua_gettop( L ) != top )
        {
            rfalse( 1, 1, "lua_stack error" );
            lua_settop( L, top );
            return 0;
        }

        // ��;�����쳣, �޷��ټ���!
        if ( size == 0 )
            return 0;

        DWORD sid = ( DWORD )lua_tonumber( L, 1 );
        if ( type == 1 )
        {
            if ( g_Script.m_pPlayer )
            {
                g_Script.m_pPlayer->SendMsg( &msg, sizeof( msg ) - size );
                lua_pushnumber( L, 1 );
                return 1;
            }
        }
        else if ( type == 2 )
        {
            if ( CPlayer* player = ( CPlayer* )GetPlayerBySID( sid )->DynamicCast( IID_PLAYER ) )
            {
                player->SendMsg( &msg, sizeof( msg ) - size );
                lua_pushnumber( L, 1 );
                return 1;
            }
        }
        else if ( type == 0 )
        {
            if ( CRegion *region = GetRegionById( sid ) )
            {
                region->Broadcast( &msg, ( WORD )( sizeof( msg ) - size ), 0 );
                lua_pushnumber( L, 1 );
                return 1;
            }
        }
        else if ( type == 3 )
        {
            BroadcastMsg( &msg, ( WORD )( sizeof( msg ) - size ) );
            lua_pushnumber( L, 1 );
            return 1;
        }

        return 0;
    }

    /*
    static int L_Rollcall( lua_State* L )
    {
        BYTE callType    = ( BYTE )lua_tonumber( L, 1 );  // ��������
        BYTE areaType    = ( BYTE )lua_tonumber( L, 2 );  // ��������
        WORD wRegionID   = ( WORD )lua_tonumber( L, 3 );  // ��������ID
        DWORD SID        = 0;
        DWORD GID        = 0;
        WORD wCount      = 0;
        CPlayer* pPlayer = NULL;

#define GET_PLAYERSID( _ITER, _COUNT, _IDX ) if ( _COUNT != _IDX ) _COUNT ++; else {  GID = (*_ITER)->GetGID(); pPlayer = ( CPlayer* )GetPlayerByGID( GID )->DynamicCast( IID_PLAYER ); \
    if( pPlayer == NULL ) continue;  lua_pushnumber( L, pPlayer->GetSID() ); return 1; }

        // ��ҵ���----------------------------------------------------------------------------------------
        if ( callType == 1 )  
        {
            if ( g_Script.m_pPlayer && g_Script.m_pPlayer->m_ParentRegion && g_Script.m_pPlayer->m_ParentArea )
            {
                // �������
                if ( areaType == 1 )
                {   
                    DWORD size = g_Script.m_pPlayer->m_ParentRegion->m_PlayerList.size();
                    DWORD idx  = 0;
                    if ( size > 0 )
                        idx = rand() % size;

                    check_list<LPIObject>::iterator iter = g_Script.m_pPlayer->m_ParentRegion->m_PlayerList.begin();
                    for ( iter; iter != g_Script.m_pPlayer->m_ParentRegion->m_PlayerList.end(); ++iter )
                    {
                        GET_PLAYERSID( iter, wCount, idx )
                    } 
                }

                // �������
                if ( areaType == 2 )
                {
                    DWORD size = g_Script.m_pPlayer->m_ParentArea->m_PlayerList.size();
                    DWORD idx  = 0;
                    if ( size > 0 )
                        idx = rand() % size;

                    check_list<LPIObject>::iterator iter = g_Script.m_pPlayer->m_ParentArea->m_PlayerList.begin();
                    for ( iter; iter != g_Script.m_pPlayer->m_ParentArea->m_PlayerList.end(); ++iter )
                    {
                        GET_PLAYERSID( iter, wCount, idx )
                    } 
                } 
	    }
            return 0;
        }

        // �������----------------------------------------------------------------------------------------
        if ( callType == 2 )
        {
            if ( g_Script.m_pMonster && g_Script.m_pMonster->m_ParentRegion && g_Script.m_pMonster->m_ParentArea )
	    {
                // �������
                if ( areaType == 1 )
                {
                    DWORD size = g_Script.m_pMonster->m_ParentRegion->m_PlayerList.size();
                    DWORD idx  = 0;
                    if ( size > 0 )
                        idx = rand() % size;

                    check_list<LPIObject>::iterator iter = g_Script.m_pMonster->m_ParentRegion->m_PlayerList.begin();
                    for ( iter; iter != g_Script.m_pMonster->m_ParentRegion->m_PlayerList.end(); ++iter )
                    {
                        GET_PLAYERSID( iter, wCount, idx )
                    } 
                }

                // �������
                if ( areaType == 2 )
                {
                    DWORD size = g_Script.m_pMonster->m_ParentArea->m_PlayerList.size();
                    DWORD idx  = 0;
                    if ( size > 0 )
                        idx = rand() % size;

                    check_list<LPIObject>::iterator iter = g_Script.m_pMonster->m_ParentArea->m_PlayerList.begin(); 
                    for ( iter; iter != g_Script.m_pMonster->m_ParentArea->m_PlayerList.end(); ++iter )
                    {
                        GET_PLAYERSID( iter, wCount, idx )
                    } 
                }  
            }
            return 0;
        }

        // ָ����������------------------------------------------------------------------------------------
        if ( callType == 3 )
        {
            if ( CRegion *destRegion = GetRegionById( wRegionID ) )
	    {
                DWORD size = destRegion->m_PlayerList.size();
                DWORD idx  = 0;
                if ( size > 0 )
                    idx = rand() % size;

                check_list<LPIObject>::iterator iter;
                for ( iter = destRegion->m_PlayerList.begin(); iter != destRegion->m_PlayerList.end(); ++iter )
                {
                    GET_PLAYERSID( iter, wCount, idx )
                }
	    } 
            return 0;
        }

        // ȫ������-----------------------------------------------------------------------------------------
        if ( callType == 4 )
        {
            DWORD size = GetGW()->m_PlayerSIDMap.size();
            DWORD idx  = 0;
            if ( size > 0 )
                idx = rand() % size;

            std::map<DWORD, DWORD>::iterator iter;
            for ( iter = GetGW()->m_PlayerSIDMap.begin(); iter != GetGW()->m_PlayerSIDMap.end(); iter++ )
            {
                if ( wCount != idx )
                    wCount++;
                else
                {
                    lua_pushnumber( L, iter->first );
                    return 1;
                }   
            } 
        }
        return 0;
    }


    // ��ȡ��Χ����б�
    static int L_GetAroundPlayerList( lua_State* L )
    {
        if ( g_Script.m_pPlayer == NULL )
            return 0;

        if ( g_Script.m_pPlayer->m_ParentRegion == NULL )
            return 0;

        if ( g_Script.m_pPlayer->m_ParentArea == NULL )
            return 0;

        DWORD     SID          = 0;
        DWORD     GID          = 0;
        WORD      wCount       = 0;
        CPlayer*  pPlayer      = NULL;
        BYTE      parAreaType  = ( BYTE )lua_tonumber( L, 1 );  // ָ������������
        BYTE      byDist       = ( BYTE )lua_tonumber( L, 2 );  // ָ����ȡ����
        WORD      byCount      = ( WORD )lua_tonumber( L, 3 );  // ָ��ѡȡ�������

        if ( byDist == 0 )
            return 0; 

        byCount = ( byCount == 0 ) ? 8 : byCount;          // ��û��ָ������ʱ����Ϊһ�����������

        check_list<LPIObject>::iterator iter;
        check_list<LPIObject>::iterator iterEnd;

        if ( parAreaType == 1 )  
        {
            iter = g_Script.m_pPlayer->m_ParentRegion->m_PlayerList.begin();
            iterEnd = g_Script.m_pPlayer->m_ParentRegion->m_PlayerList.end();
        }
        else if ( parAreaType == 2 )
        {
            iter = g_Script.m_pPlayer->m_ParentArea->m_PlayerList.begin();
            iterEnd = g_Script.m_pPlayer->m_ParentArea->m_PlayerList.end();
        }
        else
            return 0;

        int stackPos = lua_gettop( L );

        lua_newtable( L );  // ����LUA��
        for ( iter; iter != iterEnd; iter++ )
        {
            GID = (*iter)->GetGID();
            pPlayer = ( CPlayer* )GetPlayerByGID( GID )->DynamicCast( IID_PLAYER );
            if ( pPlayer == NULL )
                continue;

            if ( abs( pPlayer->m_wCurX - g_Script.m_pPlayer->m_wCurX ) <= byDist && abs( pPlayer->m_wCurY - g_Script.m_pPlayer->m_wCurY ) <= byDist )
            {
                wCount++;
                lua_pushnumber( L, pPlayer->GetSID() );
                lua_rawseti( L, -2, wCount ); 
            }

            if ( wCount == byCount )
                break;
        }

        assert( stackPos + 1 == lua_gettop( L ) );
        if ( stackPos + 1 != lua_gettop( L ) )
	{
	    lua_settop(L, stackPos);
            return 0;
	}

        return 1;
    }
    */

    // ��ȡ��Χ�����б�
    // ����1��ѡȡ�������� 0����ң�1�����������ָ����controlId�Ĺ����������2�������ң�3����Ĺ��4��������ң�5�����Ĺ���
    // ����2��ѡȡ������   -1����ǰ��ң�-2����ǰ���-3����ǰNPC��Ϊ���ֲ��Ҵ���0��ָ�����SID��Ϊһ���ӱ�r,x,y�������峡�����꣨r�����Ƕ�̬����ID��̬������
    // ����3��ѡȡ���뷶Χ -1����ǰ9����ͬ�����ڣ�-2����ǰ������-3��������������ֻ�����ң���Ϊ�����Ҵ���0������ľ��뷶Χ�����ܴ���24��
    // ����4�����ѡȡ�������ɲ��Ĭ��Ϊȫѡ������
    // ����ֵ��ʧ�ܻ�û��ѡ���κζ��� nil �ɹ� ����ı�
    static int L_GetObjectList( lua_State* L )
    {
//         DWORD type = ( DWORD )lua_tonumber( L, 1 );
//         if ( type >= 6 )
//             return 0;
// 
//         // ��һ����ѡ�������
//         int t2 = lua_type( L, 2 );
//         CRegion *dr = NULL;
//         CArea *da = NULL;
//         POINT ptc = { -1, -1 };
//         if ( t2 == LUA_TTABLE )
//         {
//             lua_rawgeti( L, 2, 1 ); lua_rawgeti( L, 2, 2 ); lua_rawgeti( L, 2, 3 );
//             DWORD r = ( DWORD )lua_tonumber( L, -3 );
//             ptc.x   = ( DWORD )lua_tonumber( L, -2 );
//             ptc.y   = ( DWORD )lua_tonumber( L, -1 );
//             lua_pop( L, 3 );
// 
//             if ( dr = GetRegionById( r ) )
//                 da = ( CArea* )dr->GetArea( ( WORD )GetCurArea( ptc.x, _AreaW ), 
//                     ( WORD )GetCurArea( ptc.y, _AreaH ) )->DynamicCast( IID_AREA );
//         }
//         else if ( t2 == LUA_TNUMBER )
//         {
//             int dt2 = ( int )lua_tonumber( L, 2 );
//             if ( dt2 == 0 || dt2 < -3 )
//                 return 0;
// 
//             CActiveObject *po = NULL;
//             switch ( dt2 )
//             {
//             case -1: po = g_Script.m_pPlayer;  break;
//             case -2: po = g_Script.m_pMonster; break;
//             case -3: po = g_Script.m_pNpc;     break;
//             default:
//                 po = ( CPlayer* )GetPlayerBySID( dt2 )->DynamicCast( IID_PLAYER );
//             }
// 
//             if ( po == NULL )
//                 return 0;
// 
//             ptc.x = po->m_wCurX;
//             ptc.y = po->m_wCurY;
//             dr = po->m_ParentRegion;  
//             da = po->m_ParentArea;
//         }
// 
//         DWORD nmax = ( DWORD )lua_tonumber( L, 4 );
//         int dlen = ( int )lua_tonumber( L, 3 );
// 
//         if ( nmax == 0 || nmax > 9999 ) 
//             nmax = 9999;
// 
//         // �ڶ�����ѡ�����
//         std::list< DWORD > idList;
//         if ( dlen == -3 ) // ȫ��ѡ�����
//         {
//             DWORD size = ( DWORD )GetGW()->m_PlayerSIDMap.size();
//             if ( size == 0 )
//                 return 0;
// 
//             // ���������������ȫѡ
//             DWORD beg = 0;
//             if ( size <= nmax )
//                 nmax = size;
//             else // ���� ���������ѡ��һ����ʼ�㣬��Ҫ��֤���µĳ����㹻 nmax
//                 beg = ( rand() % ( size - nmax ) + 1 );
// 
//             std::map< DWORD, DWORD >::iterator iter = GetGW()->m_PlayerSIDMap.begin();
//             for ( DWORD i = 0; i < beg; i ++ )
//                 iter ++;
// 
//             for ( DWORD i = 0; i < nmax; i ++, iter ++ )
//             {
//                 // 2 ֻѡ���� 4 ֻѡ����
//                 if ( ( type & ~1 ) )
//                 {
//                     if ( CPlayer *p = ( CPlayer* )GetPlayerBySID( iter->first )->DynamicCast( IID_PLAYER ) )
//                     {
//                         if ( ( ( type & ~1 ) == 2 && p->m_CurHp == 0 ) ||
//                             ( ( type & ~1 ) == 4 && p->m_CurHp != 0 ) )
//                             continue;
//                     }
//                 }
// 
//                 idList.push_back( iter->first );
//             }
// 
//             // ��֧��ѡ��
//         }
//         else
//         {
//             if ( dlen < -3 || dr == NULL || da == NULL || da->m_ParentRegion != dr )
//                 return 0;
// 
//             if ( dlen == -2 ) // ��ǰ����ѡ��
//             {
//                 if ( ( type & 1 ) == 0 ) // ѡ���
//                 {
//                     DWORD size = ( DWORD )dr->m_PlayerList.size();
//                     if ( size == 0 )
//                         return 0;
// 
//                     // ���������������ȫѡ
//                     DWORD beg = 0;
//                     if ( size <= nmax )
//                         nmax = size;
//                     else // ���� ���������ѡ��һ����ʼ�㣬��Ҫ��֤���µĳ����㹻 nmax
//                         beg = ( rand() % ( size - nmax ) + 1 );
// 
//                     check_list< LPIObject >::iterator iter = dr->m_PlayerList.begin();
//                     for ( DWORD i = 0; i < beg; i ++ )
//                         iter ++;
// 
//                     for ( DWORD i = 0; i < nmax; i ++, iter ++ )
//                     {
//                         if ( CPlayer *p = ( CPlayer* )( *iter )->DynamicCast( IID_PLAYER ) )
//                         {
//                             // 2 ֻѡ���� 4 ֻѡ����
//                             if ( ( ( type & ~1 ) == 2 && p->m_CurHp == 0 ) ||
//                                 ( ( type & ~1 ) == 4 && p->m_CurHp != 0 ) )
//                                 continue;
// 
//                             idList.push_back( p->GetSID() );
//                         }
//                     }
//                 }
//                 // ��֧��ѡ��
//             }
//             else if ( dlen > 30 ) // ��ǰ����������ѡ��
//             {
//                 if ( ( type & 1 ) == 0 ) // ѡ���
//                 {
//                     for ( check_list< LPIObject >::iterator iter = dr->m_PlayerList.begin(); 
//                         iter != dr->m_PlayerList.end(); iter ++ )
//                     {
//                         if ( CPlayer *p = ( CPlayer* )( *iter )->DynamicCast( IID_PLAYER ) )
//                         {
//                             if ( abs( ( int )p->m_wCurX - ptc.x ) <= dlen && abs( ( int )p->m_wCurY - ptc.y ) <= dlen )
//                             {
//                                 // 2 ֻѡ���� 4 ֻѡ����
//                                 if ( ( ( type & ~1 ) == 2 && p->m_CurHp == 0 ) ||
//                                     ( ( type & ~1 ) == 4 && p->m_CurHp != 0 ) )
//                                     continue;
// 
//                                 idList.push_back( p->GetSID() );
//                                 if ( idList.size() >= nmax )
//                                     goto __break;
//                             }
//                         }
//                     }
//                 }
//                 // ��֧��ѡ��
//             }
//             else if ( dlen <= 30 ) // ����ǰ9������������˶�-1�Ĵ���
//             {
//                 if ( dlen == -1 )
//                     dlen = 9999;
// 
//                 WORD x = da->m_X, y = da->m_Y;
//                 POINT ivdir[9] = { {0, 0}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1} };
//                 for ( int i=0; i<9; i++ )
//                 {
//                     if ( CArea *area = ( CArea* )dr->GetArea( 
//                         ( WORD )( x + ivdir[i].x ), ( WORD )( y + ivdir[i].y ) )->DynamicCast( IID_AREA ) )
//                     {
//                         if ( ( type & 1 ) == 0 ) // ѡ���
//                         {
//                             for ( check_list< LPIObject >::iterator iter = area->m_PlayerList.begin(); 
//                                 iter != area->m_PlayerList.end(); iter ++ )
//                             {
//                                 if ( CPlayer *p = ( CPlayer* )( *iter )->DynamicCast( IID_PLAYER ) )
//                                 {
//                                     if ( abs( ( int )p->m_wCurX - ptc.x ) <= dlen && abs( ( int )p->m_wCurY - ptc.y ) <= dlen )
//                                     {
//                                         // 2 ֻѡ���� 4 ֻѡ����
//                                         if ( ( ( type & ~1 ) == 2 && p->m_CurHp == 0 ) ||
//                                             ( ( type & ~1 ) == 4 && p->m_CurHp != 0 ) )
//                                             continue;
// 
//                                         idList.push_back( p->GetSID() );
//                                         if ( idList.size() >= nmax )
//                                             goto __break;
//                                     }
//                                 }
//                             }
//                         }
//                         else // ѡ�֣�
//                         {
//                             for ( check_list< LPIObject >::iterator iter = area->m_MonsterList.begin(); 
//                                 iter != area->m_MonsterList.end(); iter ++ )
//                             {
//                                 if ( CMonster *p = ( CMonster* )( *iter )->DynamicCast( IID_MONSTER ) )
//                                 {
//                                     if ( p->m_Property.controlId && 
//                                         abs( ( int )p->m_wCurX - ptc.x ) <= dlen && abs( ( int )p->m_wCurY - ptc.y ) <= dlen )
//                                     {
//                                         // 2 ֻѡ���� 4 ֻѡ����
//                                         if ( ( ( type & ~1 ) == 2 && p->m_CurHp == 0 ) ||
//                                             ( ( type & ~1 ) == 4 && p->m_CurHp != 0 ) )
//                                             continue;
// 
//                                         idList.push_back( p->m_Property.controlId );
//                                         if ( idList.size() >= nmax )
//                                             goto __break;
//                                     }
//                                 }
//                             }
//                         }
//                     }
//                 }
//             }
// 
//             __break:
//             ;
//         }
// 
//         if ( idList.size() == 0 )
//             return 0;
// 
//         lua_createtable( L, 0, 0 );
//         int i = 1;
//         for ( std::list< DWORD >::iterator it = idList.begin(); it != idList.end(); it ++ )
//         {
//             lua_pushnumber( L, *it );
//             lua_rawseti( L, -2, i++ );
//         }
// 
//         return 1;

		return 0;
    }

    // ��ȡ����ʹ�ô���
    static int L_GetSkillUseTimes( lua_State* L )
    {
        if ( g_Script.m_pMonster == NULL )
            return 0;

        WORD wSkilltype = ( WORD )lua_tonumber( L, 1 );  // ����ID

        if ( !g_Script.m_pMonster->skillList.empty() )
        {
            for ( std::list< SMonsterSkill >::iterator it = g_Script.m_pMonster->skillList.begin(); 
                it != g_Script.m_pMonster->skillList.end(); it++ )
            {
                if ( it ->type == wSkilltype )
                {
                    lua_pushnumber(L, it->dwSkillTimes );
                    return 1;
                }               
            }
        }
        lua_pushnumber(L, -1);
        return 1;
    }

    static int L_PutPrison( lua_State* L )
    {
        if ( g_Script.m_pPlayer == NULL )
            return 0;

        DWORD regionId = ( DWORD )lua_tonumber( L, 1 );   // ����
        WORD freeTime = ( WORD )lua_tonumber( L, 2 );   // ����
        WORD x = ( WORD )lua_tonumber( L, 3 );   // ����
        WORD y = ( WORD )lua_tonumber( L, 4 );   // ����
        LPIObject pObj = FindRegionByID( regionId );
        BOOL ret = FALSE;
        if ( pObj->DynamicCast( IID_PRISON ) != NULL )
            ret = PutPlayerIntoDestRegion( g_Script.m_pPlayer, ( WORD )regionId,  x, y );
        else if ( pObj->DynamicCast( IID_PRISONEX ) != NULL  )
        {
            CPrisonEx *prison = ( CPrisonEx * )pObj->DynamicCast( IID_PRISONEX );
            prison->CPrisonEx::AddPrisoner( g_Script.m_pPlayer->GetSID(), freeTime );
            ret = PutPlayerIntoDestRegion( g_Script.m_pPlayer, ( WORD )regionId,  x, y );
        }

        if ( ret )
        {
            lua_pushnumber( L, 1 );
            return 1;
        }
        return 0;
    }

    static int L_GetSpecialBuffAdd( lua_State* L )
    {
        return 0;
    }

    static int L_GetPlayer( lua_State* L )
    {
        LPCSTR name = ( LPCSTR  )lua_tostring( L, 1 );
        BOOL isSelect = ( BOOL  )lua_tonumber( L, 2 );

        if ( name == NULL )
            return 0;

        CPlayer *player = (CPlayer *)(GetPlayerByName( name )->DynamicCast( IID_PLAYER ) );
        if ( player == NULL )
            return 0;

        if ( isSelect )
            g_Script.m_pPlayer = player;

        lua_pushnumber( L, player->GetSID() );
        return 1;
    }

    // �޸Ľ��Խӿ�
    // ����һ������ʱ��
    // ��������ִ�н����ߵ����: 1,����  2,������, ��������չ��
    static int L_PlayerDontTalk( lua_State* L )
    {
        if ( g_Script.m_pPlayer == NULL )
            return 0;

        BYTE byIdentity = ( BYTE )lua_tonumber( L, 2 );
        std::map<std::string, DWORD>* pTalkMask = NULL;
        switch ( byIdentity )
        {
        case 1:
            pTalkMask = &KingTalkMask;
            break;
        case 2:
            pTalkMask = &MasterTalkMask;
            break;
        }

        if ( pTalkMask == NULL )
            return 0;

        WORD dontTalkTime = ( WORD )lua_tonumber( L, 1 );  // ������
        if ( dontTalkTime )
        {
            if ( ( *pTalkMask ).find( g_Script.m_pPlayer->GetAccount() ) != ( *pTalkMask ).end() )
                return 0;

            ( *pTalkMask )[ g_Script.m_pPlayer->GetAccount() ] = timeGetTime() + dontTalkTime * 6000;
            lua_pushnumber( L, 1 );
            return 1;
        }
        else
        {
            if ( ( *pTalkMask ).find( g_Script.m_pPlayer->GetAccount() ) == ( *pTalkMask ).end() )  // ����֮ǰ�и�BUG, Ӧ����==
                return 0;

            ( *pTalkMask ).erase( g_Script.m_pPlayer->GetAccount() );
            lua_pushnumber( L, 1 );
            return 1;
        }

        return 0;
    }

    static int L_SetTempEffect( lua_State* L )
    {
        return 0;
    }

    /*
    // ȡ������ǰnλSID Table
    // ����ѡ��һ�����ģ�֮��ѡ���ѡ��֮����������е������
    // ѡ���ı��ǴӴ�С���е�
    static int L_GetSchoolTopN( lua_State* L )
    {
        DWORD dwLastTopSID = 0;                       // ǰһ��TOP���SID
        BYTE bySchool = ( BYTE )lua_tonumber( L, 1 );  // 1,����  2,�䵱  3,��ü  4,����  5,ħ��
        int iTopCount = ( int )lua_tonumber( L, 2 );  // ȡ����Ҹ���
        
        std::list< DWORD > sidList;                   // ������SID��list
        std::map< DWORD, DWORD > tempSIDMap;          // ��ʱ������SID

        std::map< DWORD, DWORD >::iterator iter = GetGW()->m_PlayerSIDMap.begin();

        // ���������ȱ��浽��ʱMAP��
        for ( ; iter != GetGW()->m_PlayerSIDMap.end(); iter++ )
            tempSIDMap[ iter->first ] = iter->second;

        iTopCount = ( iTopCount > tempSIDMap.size() ) ? tempSIDMap.size() : iTopCount;

        for ( int i = 0; i < iTopCount; ++i )
        {
            iter = tempSIDMap.begin();
            dwLastTopSID = iter->first;
            iter++;

            for ( ; iter != tempSIDMap.end(); ++iter )
            {
                if ( CPlayer *pCur = ( CPlayer* )GetPlayerBySID( iter->first )->DynamicCast( IID_PLAYER ) )
                {
                    CPlayer *pLast = ( CPlayer* )GetPlayerBySID( dwLastTopSID )->DynamicCast( IID_PLAYER );
                    
                    if ( pLast && pCur->m_Property.m_School == bySchool - 1 )
                    {
                        if ( pCur->m_Property.bySkillState > pLast->m_Property.bySkillState )               // ���ȱȽϼ��ܾ��� 
                            dwLastTopSID = iter->first;
                        else if ( pCur->m_Property.bySkillState == pLast->m_Property.bySkillState )
                        {
                            if ( pCur->m_Property.m_Level > pLast->m_Property.m_Level )                     // ��αȽϵȼ�  
                                dwLastTopSID = iter->first;
                            else if( pCur->m_Property.m_Level == pLast->m_Property.m_Level )
                            {
                                if ( abs( pCur->m_Property.m_sXValue ) > abs( pLast->m_Property.m_sXValue ) )   // ���Ƚ�����ֵ 
                                    dwLastTopSID = iter->first;
                            }                            
                        }   
                    }
                }
            }

            sidList.push_back( dwLastTopSID );
            tempSIDMap.erase( dwLastTopSID );   // ����һ��, ����һ��
        }

        if ( sidList.size() == 0 )
            return 0;

        lua_createtable( L, 0, 0 );
        int idx = 1;
        for ( std::list< DWORD >::iterator it = sidList.begin(); it != sidList.end(); it ++ )
        {
            lua_pushnumber( L, *it );
            lua_rawseti( L, -2, idx++ );
        }

        return 1;
    }*/
    
    // ���а��ɳ�Ա��ID
    static int L_GetFactionMemberInfo( lua_State *L )
    {

        LPCSTR factionname = lua_tostring( L, 1 );
		if ( factionname == NULL )
			return 0;

        static SFactionData data;
        memset( &data, 0, sizeof( data ) );
        int top = lua_gettop( L );

        if ( !GetFactionInfo( factionname, data ) )
            return 0;

        lua_createtable( L, data.stFaction.byMemberNum, 0 );
        if ( !lua_istable( L, -1 ) )
            return lua_settop( L, top ), 0;

        for ( int i = 0; i < data.stFaction.byMemberNum; i++ )
        {
            CPlayer *player = ( CPlayer * )( GetPlayerByName( data.stMember[i].szName )->DynamicCast( IID_PLAYER ) );
            if ( player == NULL )
                continue;

            lua_pushnumber( L, player->GetSID() ); 
            lua_rawseti( L, -2, i + 1 );
        }

        return 1;
    }

	static int L_SetState( lua_State* L )
	{
// 		if ( g_Script.m_pPlayer == NULL )
// 			return 0;
// 
// 		LPCSTR key = lua_tostring( L, 1 );
// 		if ( key == NULL )
// 			return 0;
// 
// 		if( strcmp( key, "3DEquip" ) == 0 )
// 		{
// 			LPCTSTR sValue = lua_tostring( L, 2 );
// 			if( sValue == NULL )
// 				return 0;
// 
// 			int bValue = -1;
// 			if( strcmp( sValue, "true") == 0 ) bValue = 1;
// 			else if( strcmp( sValue, "false") == 0 ) bValue = 0;
// 
// 			if( ( bValue != -1 ) &&
// 				( g_Script.m_pPlayer->m_dwExtraState & SHOWEXTRASTATE_D3D_EQUIP ) != bValue )
// 			{
// 				if( bValue ) g_Script.m_pPlayer->m_dwExtraState |= SHOWEXTRASTATE_D3D_EQUIP;
// 				else g_Script.m_pPlayer->m_dwExtraState &= ~SHOWEXTRASTATE_D3D_EQUIP;
// 
// 				g_Script.m_pPlayer->SendExtraState();
// 			}
// 		}

		return 0;
	}

	static int L_GetEquips( lua_State* L )
	{
		if ( g_Script.m_pPlayer == NULL )
			return 0;

		const char* sValue = lua_tostring( L, 1 );
		if( sValue == NULL )
			return 0;

		if( strcmp( sValue, "id" ) == 0 )
		{
			int top = lua_gettop( L );
			lua_createtable( L, 16, 0 );
			if ( !lua_istable( L, -1 ) )
				return lua_settop( L, top ), 0;

			for( int i = 0; i < 16; i ++ )
			{
				lua_pushnumber( L, g_Script.m_pPlayer->m_Property.m_Equip[i].wIndex );
				lua_rawseti( L, -2, i + 1 );
			}

			return 1;
		}

		return 0;
	}

    // ����ĳЩ��������ʹ��
    // param1������ID
    // param2: �����Ƶ���Ʒ�б�
    static int L_LimitUseItem( lua_State* L )
    {
        DWORD dwRegionID = ( DWORD )lua_tonumber( L, 1 );
        if ( !lua_istable( L, 2 ) )
            return 0;

        CRegion *destRegion = GetRegionById( dwRegionID );
        if ( destRegion == NULL )
            return 0;

        destRegion->m_LimitItemList.clear();

        size_t len = lua_objlen( L, 2 );

        for ( DWORD i = 0; i < len; ++i )
        {
            lua_rawgeti( L, 2, i + 1 );
            destRegion->m_LimitItemList.insert( ( DWORD )lua_tonumber( L, -1 ) );
            lua_pop( L, 1 );
        }

        lua_pushnumber( L, 1 );
        return 1;
    }

    // ����������Խ��
    // param1: ��ƷID
    // param2: ����ѡ��specialItem.def�ļ��к���5����ĳһ�У������ȼ���1��2��3��4��5��6
    // param3: ����ѡ��specialItemAttribute.def�ļ��е�ĳ���� ������ 1��2��3������
    // param4: ��������ƫ��ӳ�����
    // return: һ�������ӳɽ�����Ա�
    static int L_CalcJianDingAttribute( lua_State* L )
    {
//         DWORD dwItemID    = ( DWORD )lua_tonumber( L, 1 );
//         BYTE level        = ( BYTE  )lua_tonumber( L, 2 );
//         WORD addGene      = ( WORD  )lua_tonumber( L, 4 );
// 
//         if ( level < 1 || level > 6 )  // Խ��
//             return 0;
// 
//         if ( !lua_istable( L, 3 ) )
//             return 0;
// 
//         SpecialItem* pItem = CItem::GetSpecialItem( dwItemID );
//         if ( pItem == NULL )
//             return 0;
// 
//         const SItemData* pData = CItem::GetItemData( dwItemID );
//         if ( pData == NULL )
//             return 0;
// 
//         DWORD dwAttrID        = pItem->mJianDingLevel[ level - 1 ];
//         WORD  coff[ 57 ]      = { 0 };
// 
//         int    colAttri[ 6 ];
//         size_t len = lua_objlen( L, 3 );
//         len = len > 6 ? 6 : len;
// 
//         for ( DWORD i = 0; i < len; ++i )
//         {
//             lua_rawgeti( L, 3, i + 1 );
//             colAttri[ i ] = ( int )lua_tonumber( L, -1 );
//             lua_pop( L, 1 );
//         }
// 
//         for ( DWORD i = 0; i < 57; ++i )
//         {
//             for ( DWORD j = 0; j < len; ++j )
//             {
//                 if ( i + 1 == colAttri[ j ] )
//                 {
//                     coff[ i ] = addGene;
//                     break;
//                 }
//             }
//         }
// 
//         SEquipment::Attribute::Unit unit[ 6 ];  // �����type��ʾ���Ա�����ĵڼ��У�������1��ʼ
//         memset( unit, 0, sizeof( unit ) );
//         GetAddAttribute( unit, (WORD)dwAttrID, 6, coff, level, pData->byType );
// 
//         int retCount = 0;
//         for ( BYTE i = 0; i < 6; ++i )
//         {
//             if ( unit[ i ].value == 0 || unit[ i ].type == 0 ) // ������1��ʼ��
//                 continue;
// 
//             lua_pushnumber( L, unit[ i ].type * 10000 + unit[ i ].value );
//             retCount++;
//         }
// 
//         return retCount;

		return 0;
    }

    static int L_LoadDropItemList( lua_State* L )
    {
        extern BOOL LoadGlobalDrop( std::string strDropFileName = "" );

        LPCSTR fileName = ( LPCSTR )lua_tostring( L, 1 );
        if ( LoadGlobalDrop( fileName == NULL ? "" : fileName ) )
        {
            lua_pushnumber( L, 1 );
            return 1;
        }

        return 0;
    }
	   
	// ��ȡĳ������ҡ����NPC����
	// ����1������ID
	// ����2����ѯ�Ķ������� 0:��� 1:���� 2:NPC
	// ����3����ѯ�����������ҵ�����״̬ 1������ 2���һ� 3������ Ĭ�������Ϊ��ѯ�����������ò������Բ�����
    static int L_GetRegionCount( lua_State* L)
	{
		DWORD regionid = ( DWORD )lua_tonumber( L, 1);
		if (regionid == NULL)
			return 0;

		CRegion *destRegion = GetRegionById(regionid);
		if (destRegion == NULL)
			return 0;

		BYTE objType = ( BYTE )lua_tonumber( L, 2); 

		BYTE state = (BYTE)lua_tonumber(L, 3);

		switch (objType)
		{
		case 0:
			lua_pushnumber( L, destRegion->GetPlayerCount(state));
			break;
		case 1:
			lua_pushnumber( L, destRegion->GetMonsterCount());
			break;
		case 2:
			lua_pushnumber( L, destRegion->GetNpcCount());
			break;
		default:
			return 0;
		}
		return 1;
	}

    // ���ã�������Ϸ����Ŀ���
    // ������1��Ϸ�� 2����� 3״̬
    //       1ȡ-1��ʾ����������Ϸ�� 2ȡ-1��ʾ�������з���
    static int L_SetPlugingameState( lua_State* L)
    {
        if ( CPlugInGameManager::GetInstance() == NULL )
            return 0;

        int wGameId = ( int )lua_tonumber( L, 1 );
        int wAreaId = ( int )lua_tonumber( L, 2 );
        BYTE bIsOpen = ( BYTE )lua_toboolean( L, 3 );

        if ( wGameId > 0 )
        {
            SGameIntro *pGameIntro = CPlugInGameManager::GetInstance()->GetGameIntro(wGameId);

            if ( NULL == pGameIntro )
                return 0;
            
            if ( wAreaId == 0 || wAreaId > pGameIntro->gameCode.wAreaCnt )
                return 0;
            
            if ( wAreaId > 0 )
            {              
                SRegAreaParm *pAreaParm = &pGameIntro->gameCode.areaParms[wAreaId - 1];
                if ( pAreaParm == NULL )
                    return 0;
                pAreaParm->bIsOpen = bIsOpen != 0;

                rfalse( 2, 1, "%d����Ϸ��%d�ŷ��䣬����״̬Ϊ%d",wGameId, wAreaId, bIsOpen );
            }
            // �ر����з���
            else if ( wAreaId == -1 )
            {
                for ( int i= 0; i< pGameIntro->gameCode.wAreaCnt; i++ )
                {
                    SRegAreaParm *pAreaParm = &pGameIntro->gameCode.areaParms[i];
                    if ( pAreaParm == NULL )
                        continue;
                    pAreaParm->bIsOpen = bIsOpen != 0;
                }
            }        
        }
        // �ر�������Ϸ
        else if ( wGameId == -1 )
        {
            CPlugInGameManager::GetInstance()->m_bOpen = bIsOpen;
            if ( !bIsOpen )
            {
                rfalse(2, 1, "�ر�������Ϸ������˳��󽫲����ٴν���");
                sc_PlugInGame_GameErr errMsg;
                errMsg.wGameErrType = -201;
                CPlugInGameManager::GetInstance()->BroadCastMsg( gpHall, &errMsg, sizeof( sc_PlugInGame_GameErr ) );
            }
            else
            {
                rfalse( 2, 1, "����������Ϸ����ҿ��Խ��뿪������Ϸ�ͷ���" );
                sc_PlugInGame_GameErr errMsg;
                errMsg.wGameErrType = -202;
                CPlugInGameManager::GetInstance()->BroadCastMsg( gpHall, &errMsg, sizeof( sc_PlugInGame_GameErr ) );
            }
        }

        return 0;
    }

	static int L_SetEnvironment ( lua_State* L)
	{
		BYTE index = ( BYTE ) lua_tonumber( L, 1 ); //���õ����� 1����־���ֵ���� 2�������Ǯ������ 3��������Ʒ���ʱ���
												  // 4���౶ʱ��
		if ( index == NULL )
			return 0;

		float scale = ( float )lua_tonumber( L, 2 ); // ���õ�ֵ
		BYTE  byTimeCount = 0;
		if ( scale == NULL )
			return 0;

		switch ( index )
		{
		case 1:
			SetExpScale( scale );
			break;
		case 2:
			SetDropMoneyScale( scale );
			break;
		case 3:
			SetDropItemScale( scale );
			break;
		case 4:
			SetMultiTimeCount( ( BYTE )scale );
			break;
		}
		return 0;
	}

    static int L_SetRegionMultExp( lua_State *L )
    {
        DWORD regionid = ( DWORD )lua_tonumber( L, 1);
        if (regionid == NULL)
            return 0;

        CRegion *destRegion = GetRegionById(regionid);
        if (destRegion == NULL)
            return 0;

        destRegion->m_wRegionMultExp = ( float )lua_tonumber( L, 2 );
        if ( destRegion->m_wRegionMultExp < 0 || destRegion->m_wRegionMultExp > 10 )
        {
            destRegion->m_wRegionMultExp = 1;
            rfalse( 2, 1, "������%d���ű����õľ��鱶�ʡ�%f���д�����ʧ�ܣ�", regionid, destRegion->m_wRegionMultExp );
            return 0;
        }

        rfalse( 2, 1, "������%d���ű����õľ��鱶�ʡ�%f�����óɹ���", regionid, destRegion->m_wRegionMultExp );
        return 0;
    }

    // ˵��������ҽ��ֵ���ܵ�����ߺ��ۼ����Լ����
    // ����1 ����ҽ��ֵ��ֵ����  1��ǰ��ɫҽ��ֵ 2��ǰ��ɫҽ�µ����ۻ�ֵ
    // ����2 �����õ�ǰҽ��ֵʱ�仯�ĵ��������������յ������������ۻ�ֵΪ���յ��� 
    static int L_SetMedicalEthicInf( lua_State *L )
    {
//         if ( g_Script.m_pPlayer == NULL )
//             return 0;
// 
//         g_Script.m_pPlayer->UpdatePlayerDataValsPerDay( g_Script.m_pPlayer->m_Property.dwLastAddMETime, &g_Script.m_pPlayer->m_Property.wMaxPerDayME, ( LPDWORD )0 );
// 
//         int nSetType = (int)lua_tonumber( L, 1 );
//         int wSetPoint = (int)lua_tonumber( L, 2 );
// 
//         if ( nSetType == 1 )
//         {
//             if ( wSetPoint > 0 )
//                 g_Script.m_pPlayer->AddMedicalEthics(wSetPoint);
//             else if( wSetPoint < 0 )
//             {
//                 g_Script.m_pPlayer->m_Property.wMedicalEthics = ( g_Script.m_pPlayer->m_Property.wMedicalEthics > (abs(wSetPoint)) )
//                     ? g_Script.m_pPlayer->m_Property.wMedicalEthics + wSetPoint : 0;
//                 g_Script.m_pPlayer->m_Property.wMaxPerDayME = ( g_Script.m_pPlayer->m_Property.wMaxPerDayME > (abs(wSetPoint)) )
//                     ? g_Script.m_pPlayer->m_Property.wMaxPerDayME + wSetPoint : 0;
//             }
//         }
//         else if ( nSetType == 2 && wSetPoint >= 0 )
//             g_Script.m_pPlayer->m_Property.wMaxPerDayME = wSetPoint;
// 
//         g_Script.m_pPlayer->SendExtraState();

        return 0;
    }

    // ˵�������ҽ��ֵ
    // ����1 �õ�ҽ��ֵ��ֵ���� 1��ǰ��ɫҽ��ֵ 2��ǰ��ɫҽ�µ����ۻ�ֵ(ʵ���ۼ�) 3ҽ��ֵ��������ۼ�ֵ(���ȼ�����)
    static int L_GetMedicalEthicInf( lua_State *L )
    {
//         if ( g_Script.m_pPlayer == NULL )
//             return 0;
// 
//         g_Script.m_pPlayer->UpdatePlayerDataValsPerDay( g_Script.m_pPlayer->m_Property.dwLastAddMETime, &g_Script.m_pPlayer->m_Property.wMaxPerDayME, ( LPDWORD )0 );
// 
//         int nGetType = (int)lua_tonumber( L, 1 );
//         if ( nGetType == 1 )
//             lua_pushnumber( L, g_Script.m_pPlayer->m_Property.wMedicalEthics );
//         else if ( nGetType == 2 )
//             lua_pushnumber( L, g_Script.m_pPlayer->m_Property.wMaxPerDayME );
//         else if ( nGetType == 3 )
//             lua_pushnumber( L, g_Script.m_pPlayer->CalculateMaxPerDayME(g_Script.m_pPlayer->m_Property.m_Level) );
//         else
//             return 0;
// 
//         return 1;
		return 0;
    }

    static int L_GetTodayAchieveExpCount( lua_State *L )
    {
//         if ( g_Script.m_pPlayer == NULL )
//             return 0;
// 
//         CPlayer::SFluxProperty &p = g_Script.m_pPlayer->m_Property;
//         g_Script.m_pPlayer->UpdatePlayerDataValsPerDay( p.dwLastSaveExpTime, &p.dwTodayAchieveMonsterExpCount, &p.dwTodayAchieveOtherExpCount ); // ����ÿ�վ���ֵ����
//         int nGetType = (int)lua_tonumber( L, 1 );
//         switch ( nGetType )
//         {
//         case 1: // ��ȡ��־���
//             lua_pushnumber( L, p.dwTodayAchieveMonsterExpCount ); 
//             break; 
// 
//         case 2: // ��ȡ��������
//             lua_pushnumber( L, p.dwTodayAchieveOtherExpCount ); 
//             break; 
// 
//         default: 
//             return 0; 
//         };
//     
//         return 1;
		return 0;
    }

	//��ȡ�ﹱֵ
	static int L_GetTodayContribution( lua_State* L)
	{
// 		if (g_Script.m_pPlayer == NULL)
// 			return 0;
// 		
// 		CPlayer::SFluxProperty &p = g_Script.m_pPlayer->m_Property;
// 		g_Script.m_pPlayer->UpdatePlayerDataValsPerDay(p.dwLastSaveContributionTime, &p.wTodayContributionVal, (LPDWORD)0);
// 
// 		int contributionType = (int)lua_tonumber(L, 1);
// 
// 		switch (contributionType)
// 		{
// 		case 1:
// 			lua_pushnumber(L, p.wContribution);
// 			break;
// 		case 2:
// 			lua_pushnumber(L, p.wTodayContributionVal);
// 			break;
// 		default:
// 			return 0;
// 		};
// 		
// 		return 1;
		return 0;
	}

	//���ðﹱֵ
	//����1��1����ǰ��ɫ�İﹱֵ 2����ǰ��ɫ���հﹱֵ
	//����2�����õ�ֵ���仯ֵ��
	static int L_SetContribution( lua_State* L)
	{
// 		if (g_Script.m_pPlayer == NULL)
// 			return 0;
// 
// 		CPlayer::SFluxProperty &p = g_Script.m_pPlayer->m_Property;
// 
// 		g_Script.m_pPlayer->UpdatePlayerDataValsPerDay(p.dwLastSaveContributionTime, &p.wTodayContributionVal, (LPDWORD)0);
// 
// 		int contributionType = (int)lua_tonumber(L, 1);
// 		WORD val = (WORD)lua_tonumber(L, 2);
// 
// 		switch (contributionType)
// 		{
// 		case 1:
// 			p.wContribution = val;
// 			break;
// 		case 2:
// 			{
// 				p.wTodayContributionVal += val;
// 				p.wContribution += val;
// 			}
// 			break;
// 		default:
// 			return 0;
// 		};
// 
// 		if (p.wTodayContributionVal > 10000)
// 		{
// 			p.wTodayContributionVal = 10000;
// 			TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "���հﹱֵ����Ѵ����ޣ������ٻ�ø���ﹱֵ����");
// 		}
// 		if (p.wContribution > 10000)
// 		{
// 			p.wContribution = 10000;
// 			TalkToDnid(g_Script.m_pPlayer->m_ClientIndex, "�ﹱֵ��������Ѵ����ޣ������ٻ�ø���ﹱֵ����");
// 		}
		return 0;
	}
	
    // ������а��ɵķ��ٶ�ֵ
    static int L_ClearAllFactionProsperity( lua_State* L )
    {
        std::map< std::string, SFactionData >::iterator iter;
        for ( iter = factionManagerMap.begin(); iter != factionManagerMap.end(); ++iter )
            iter->second.stFaction.dwFactionProsperity = 0;
        
        lua_pushnumber( L, 1 );
        return 1;
    }

    // ���㲢����ǰ5λ���ٶȵİ���
    static int L_CalcAndSaveProspTopFiveFaction( lua_State* L )
    {
        if ( GetGW() == NULL )
            return 0;

        extern WORD TopFactionID[ 5 ];
        int type = ( int )lua_tonumber( L, 1 );

        // 1���㲢���淵�� 2ֻ����ǰ5      
        if ( type == 1 )
//            GetGW()->m_FactionManager.CalcAndSaveTopFiveFaction();

        if ( type == 1 ||  type == 2 )
        {
            int count = 0;
            for ( int i = 0; i < 5; ++i )
            {
                if ( TopFactionID[ i ] <= 0 )
                    break;
                lua_pushnumber( L, TopFactionID[ i ] );
                ++count;
            }
            return count;
        }

        return 0;
    }

    static int L_GetWulinChiefPos( lua_State *L ) //��ȡ�����ĳ���������
	{
// 		if( g_Script.m_pPlayer == NULL )
// 			return 0;
// 
// 		extern string s_WulinChiefName;
// 		extern BYTE s_WulinChiefSchool;
// 		CPlayer *chief = (CPlayer *)(GetPlayerByName(s_WulinChiefName.c_str())->DynamicCast( IID_PLAYER ) );
// 		if ( chief == NULL ) //û�ҵ�����
// 		{
// 			lua_pushnumber( L, -1);
// 			return 1;
// 		}
// 
// 		if(s_WulinChiefSchool > 5)
// 		{
// 			lua_pushnumber( L, -2);
// 			return 1;
// 		}
// 
// 		WORD sceneID = chief->m_ParentRegion->m_wRegionID;
// 		WORD posx = chief->m_wCurX; 
// 		WORD posy = chief->m_wCurY;
// 
// 		lua_pushnumber( L, sceneID);
// 		lua_pushnumber( L, posx);
// 		lua_pushnumber( L, posy);
// 		return 3;

		return 0;
	}

	static int L_GetCityOwenerPos( lua_State *L ) //��ȡ�����ĳ���������
	{
// 		if( g_Script.m_pPlayer == NULL )
// 			return 0;
// 
//         LPCSTR cityowenname = lua_tostring( L, 1 );
// 
// 		extern string s_FactionName;
// 		extern DWORD s_dwFactionTitle;
// 
// 		CPlayer *cityowener = (CPlayer *)(GetPlayerByName(cityowenname)->DynamicCast( IID_PLAYER ) );
// 		if ( cityowener == NULL ) 
// 		{
// 			lua_pushnumber( L, -1);//û�ҵ�����
// 			return 1;
// 		}
// 		
// 		if ( !cityowener->IsInFactionFirst() || cityowener->m_stFacRight.Title != 8 )
// 		{
// 			lua_pushnumber( L, -2); //���ǳ���
// 			return 1;
// 		}
// 
// 		WORD sceneID = cityowener->m_ParentRegion->m_wRegionID;
// 		WORD posx = cityowener->m_wCurX; 
// 		WORD posy = cityowener->m_wCurY;
// 
// 		lua_pushnumber( L, sceneID);
// 		lua_pushnumber( L, posx);
// 		lua_pushnumber( L, posy);
// 		return 3;

		return 0;
	}	

    // ���»�������Ҳ������е��������
    static int L_UpdataPlayerPaiHangJoinData( lua_State* L )
    {
//         if ( g_Script.m_pPlayer == NULL )
//             return 0;
// 
//         CPlayer::SFluxProperty &ppty = g_Script.m_pPlayer->m_Property;
//         BYTE type     = ( BYTE )lua_tonumber( L, 1 );   // ��������
//         WORD value    = ( WORD )lua_tonumber( L, 2 );   // ����ֵ
//         BYTE opType   = ( BYTE )lua_tonumber( L, 3 );   // �������� ��1:set, 2:add��
//         
//         #define  OPERATEDATAS( __optype, __opTag ) \
//                     if ( ( __optype == 2 ) && ( ( UINT64 )( __opTag ) + value ) > 0xffffffff ) \
//                         { __opTag = 0xffffffff; break; } \
//                     if ( ( __optype == 3 ) && ( ( INT64 )( __opTag ) - value ) < 0 ) \
//                         { __opTag = 0; break; } \
//                     if ( __optype == 1 ) \
//                         { __opTag = value; } \
//                     else if ( __optype == 2 ) \
//                         { __opTag += value; } \
//                     else if ( __optype == 3 ) \
//                         { __opTag -= value; }
//         
//         switch ( type )
//         {
//         case 1:
//             g_Script.m_pPlayer->UpdatePlayerDataValsPerDay( ppty.dwLastSaveCharmValTime, &ppty.dwTodayCharmValue, ( LPDWORD )0 );
//             OPERATEDATAS( opType, ppty.dwTodayCharmValue )  // ���½�������ֵ
//             break;
//         case 2:
//             OPERATEDATAS( opType, ppty.dwTotalCharmValue )  // ����������ֵ
//             break;
//         case 3:
//             OPERATEDATAS( opType, ppty.dwHuntScore )   // �������Ի���
//             break;
//         case 4:
//             OPERATEDATAS( opType, ppty.dwMazePassScore )  // �����Թ�ͨ�ش���
//             break;
//         case 5:
//             OPERATEDATAS( opType, ppty.dwBaoTuScore )  // �����ڲر�ͼ��÷���
//             break;
//         case 6:
//             OPERATEDATAS( opType, ppty.dwZhenFaPassScore )  // ���´����󷨷���
//             break;
//         default:
//             return 0;
//         }
//         
        return 0;
    }

    // ��ȡ��Ҳ������е��������
    static int L_GetPlayerPaiHangJoinData( lua_State* L )
    {
//         if ( g_Script.m_pPlayer == NULL )
//             return 0;
// 
//         CPlayer::SFluxProperty &ppty = g_Script.m_pPlayer->m_Property;
//         BYTE type = ( BYTE )lua_tonumber( L, 1 );   // ��ȡ����
// 
//         switch ( type )
//         {
//         case 1:
//             g_Script.m_pPlayer->UpdatePlayerDataValsPerDay( ppty.dwLastSaveCharmValTime, &ppty.dwTodayCharmValue, ( LPDWORD )0 );
//             lua_pushnumber( L, ppty.dwTodayCharmValue );  // ��ȡ��������ֵ
//             break;
//         case 2:
//             lua_pushnumber( L, ppty.dwTotalCharmValue );  // ��ȡ������ֵ
//             break;  
//         case 3:
//             lua_pushnumber( L, ppty.dwHuntScore );   // ��ȡ���Ի���
//             break;
//         case 4:
//             lua_pushnumber( L, ppty.dwMazePassScore );  // ��ȡ�Թ�ͨ�شη���
//             break;
//         case 5:
//             lua_pushnumber( L, ppty.dwBaoTuScore );  // ��ȡ�ڲر�ͼ��÷���
//             break;
//         case 6:
//             lua_pushnumber( L, ppty.dwZhenFaPassScore );  // ��ȡ�����󷨷���
//             break;
//         default:
//             return 0;
//         }
// 
//         return 1;
		return 0;
    }
	

	// ���ý�ֹ����˳�����
	// ���� trueΪ��ֹ falseΪ����ֹ
	static int L_RefuseLeaveUnion( lua_State* L)
	{
		if ( GetGW() == NULL )
			return 0;

		bool bIsRefused = (lua_toboolean(L , 1) == 1);
		LPCSTR errorInfo = (LPCSTR) lua_tostring(L, 2);
	
		
		GetGW()->m_UnionManager.m_bIsRefused = bIsRefused;
		
		if (errorInfo)
			GetGW()->m_UnionManager.m_errorInfo = errorInfo;
		return 0;
	}

	// �ű���ȡ������Ϣ
	// ����һ�ű����а���8���ӱ�ÿ���ӱ���3�1������������� 2���������SID 3������ڽ����е���λ
	static int L_GetUnionInfo( lua_State* L)
	{
// 		if (g_Script.m_pPlayer == NULL)
// 			return  0;
// 
// 		if (GetGW() == NULL)
// 			return 0;
// 
// 		int top = lua_gettop(L);
// 		if (GetGW()->m_UnionManager.IsUnion(g_Script.m_pPlayer->m_Property.m_dwStaticID))
// 		{
// 			SPlayerUnionData* data = GetGW()->m_UnionManager.GetUnionDataByPlayerID(g_Script.m_pPlayer->m_Property.m_dwStaticID);
// 
// 			if (data == NULL)
// 				return 0;
// 	
// 			lua_createtable(L, 0, 0);
// 			if (!lua_istable(L, -1))
// 				goto __error_exit;
// 			for (int i = 0; i < SPlayerUnionData::NUMBER_MAX; i++)
// 			{
// 				lua_createtable(L, 0, 0);
// 				if (!lua_istable(L, -1))
// 					goto __error_exit;
// 
// 				lua_pushstring(L, data->szName[i]);
// 				lua_rawseti(L, -2, 1);
// 				lua_pushnumber(L, data->roleid[i]);
// 				lua_rawseti(L, -2, 2);
// 				lua_pushnumber(L, i + 1);
// 				lua_rawseti(L, -2, 3);
// 
// 
// 				lua_rawseti(L, -2, i + 1);
// 			}
// 			assert(lua_gettop(L) == top + 1);
// 			if ( lua_gettop(L) != top + 1 )
// 				goto __error_exit;
// 			return 1;
// 		}
// 		else
// 			return 0;
// __error_exit:
// 		if (lua_gettop(L) != top)
// 			lua_settop(L, top);
		return 0;
	}	

	// �����Ҿ�BUFF�Ľű��ӿ�
	// ����1���ҾƵ�����
	static int L_AddWhineBUFF(lua_State* L)
	{
		if (g_Script.m_pPlayer == NULL)
			return 0;

		int WhineType = (int)lua_tonumber(L, 1);

		if (WhineType < 40300 || WhineType > 40308)
			return 0;

		
		g_Script.m_pPlayer->AddWhineBuff(WhineType);

		return 0;
	}

	// ��ȡ��ǰ��������Ҿ�BUFF����
	static int L_GetWhineBUFFType( lua_State* L)
	{
		if (g_Script.m_pPlayer == NULL)
			return 0;

		if (g_Script.m_pPlayer->m_wWhineBUFFType < 40300 || g_Script.m_pPlayer->m_wWhineBUFFType > 40308)
			return 0;
			
		lua_pushnumber(L, g_Script.m_pPlayer->m_wWhineBUFFType);
		return 1;	
	}

    // ���ָ�������漴����
    static int L_GetAppointRegionRandomPos( lua_State* L )
    {
        DWORD regionid = ( DWORD )lua_tonumber( L, 1);
        if ( regionid == NULL )
            regionid = 1;

        CRegion *destRegion = GetRegionById(regionid);
        if ( destRegion == NULL )
            destRegion = GetRegionById( 1 );

        if ( destRegion == NULL )
            return 0;

        WORD _wCenterX = ( WORD )lua_tonumber( L, 2);
        WORD _wCenterY = ( WORD )lua_tonumber( L, 3);
        if ( _wCenterX == NULL || _wCenterY == NULL )
        {
            _wCenterX = WORD(destRegion->m_ptReLivePoint.x);
            _wCenterY = WORD(destRegion->m_ptReLivePoint.y);    
        }
    
        WORD _wDesPosX = 0;
        WORD _wDesPosY = 0;

        extern void RandomPos( CRegion *parentRegion, WORD &wDesPosX, WORD &wDesPosY, WORD wCenterX, WORD wCenterY, WORD wBRArea, DWORD dwSearchTimes );
        RandomPos( destRegion, _wDesPosX, _wDesPosY, _wCenterX, _wCenterY, max( destRegion->m_RegionW , destRegion->m_RegionH), 500 );

        lua_pushnumber( L, _wDesPosX );
        lua_pushnumber( L, _wDesPosY );

        return 2;
    }
/*
    // ����5ǿ����
    // param1: ������ȡ�������ͣ�1������  2������  3���Թ�  4����ͼ  5���󷨣�
    // param2: �������ͣ�1�������� 2����ȡ���� ��
//     static int L_CalcAndSavePlayerTopFive( lua_State* L )
//     {
//         extern std::string TopFiveFilePath[ 5 ];
//         int retCount = 0; // ���ظ���
//         int calcType = static_cast< int >( lua_tonumber( L, 1 ) );
//         int operType = static_cast< int >( lua_tonumber( L, 2 ) );
// 
//         if ( calcType < 1 || calcType > 5 )
//             return 0;
// 
//         DWORD ( *pIDArray )[ 5 ] = &TopFiveSID[ calcType - 1 ];
//         DWORD ( *pValArray )[ 5 ] = &TopFiveVals[ calcType - 1 ];
//         std::string ( *pNameArray )[ 5 ] = &TopFiveName[ calcType - 1 ];
// 
//         if ( operType == 1 ) // �������㱣�沢����
//         {
//             ofstream file;
//             file.open( TopFiveFilePath[ calcType - 1 ].c_str(), ios::out );
//             if ( !file.is_open() )
//                 return 0;
// 
//             // ��ʱ������ָ��
//             std::list< const CPlayer* > tempPlayerList;  
//             std::list< const CPlayer* >::iterator it, MaxIt;
// 
//             // ���������ȱ��浽��ʱList��
//             extern int CALLBACK GetAllPlayerObject( LPIObject& pObject, std::list< const CPlayer* >* pPlayerList );
//             extern int TraversalPlayers( TRAVERSALFUNC TraversalFunc, LPARAM param );
//             TraversalPlayers( ( TRAVERSALFUNC )GetAllPlayerObject, ( LPARAM )&tempPlayerList );
// 
//             int num = min( tempPlayerList.size(), 5 );
//             if ( num == 0 ) return 0;
// 
//             memset( pIDArray, 0, sizeof( DWORD ) * 5 ); // �����һ������
//             memset( pValArray, 0, sizeof( DWORD ) * 5 );
//             for ( int i = 0; i < 5; ++i )
//                 pNameArray[ 0 ][ i ] = "";
// 
//             DWORD dwCurData = 0;
//             DWORD dwMaxData = 0;
//             for ( int i = 0; i < num; ++i )
//             {
//                 MaxIt = tempPlayerList.begin();
//                 if ( ( *MaxIt ) == NULL )
//                     continue;
// 
//                 for ( it = tempPlayerList.begin(); it != tempPlayerList.end(); ++it )
//                 {
//                     if ( ( *it ) == NULL )
//                         continue;
// 
//                     switch ( calcType )
//                     {
//                     case 1:
//                         dwCurData = ( *it )->m_Property.dwTotalCharmValue;
//                         dwMaxData = ( *MaxIt )->m_Property.dwTotalCharmValue;
//                         break;
//                     case 2:
//                         dwCurData = ( *it )->m_Property.dwHuntScore;
//                         dwMaxData = ( *MaxIt )->m_Property.dwHuntScore;
//                         break;
//                     case 3:
//                         dwCurData = ( *it )->m_Property.dwMazePassScore;
//                         dwMaxData = ( *MaxIt )->m_Property.dwMazePassScore;
//                         break;
//                     case 4:
//                         dwCurData = ( *it )->m_Property.dwBaoTuScore;
//                         dwMaxData = ( *MaxIt )->m_Property.dwBaoTuScore;
//                         break;
//                     case 5:
//                         dwCurData = ( *it )->m_Property.dwZhenFaPassScore;
//                         dwMaxData = ( *MaxIt )->m_Property.dwZhenFaPassScore;
//                         break;
//                     }
// 
//                     if ( dwCurData > dwMaxData )  // �Ƚϻ�������
//                     {
//                         MaxIt     = it;
//                         dwMaxData = dwCurData;    // Ϊ�ľ������һ��ѭ������������ֵ
//                     }
//                     else if ( dwCurData == dwMaxData )
//                     {
//                         if ( ( *it )->m_Property.m_Level > ( *MaxIt )->m_Property.m_Level )  // �Ƚ���ҵȼ�
//                             MaxIt = it;
//                         else if ( ( *it )->m_Property.m_Level == ( *it )->m_Property.m_Level )
//                         {
//                             if ( ( *it )->GetSID() > ( *MaxIt )->GetSID() ) // �Ƚ����SID
//                                 MaxIt = it;
//                         }
//                     }
//                 }
// 
//                 if ( calcType == 1 ) // ���Ӽ��˲���ʾ�Ա�
//                     dwMaxData = ( ( *MaxIt )->m_Property.m_Sex << 31 ) | ( dwMaxData & 0x7fffffff );  // ���λ��ʾ�Ա�
//                 
//                 pIDArray[ 0 ][ i ]   = ( *MaxIt )->GetSID();   // ����SID������
//                 pValArray[ 0 ][ i ]  = dwMaxData;              // �������������
//                 pNameArray[ 0 ][ i ] = ( *MaxIt )->m_Property.m_Name;  // �������ֵ�����
// 
//                 // ����SID,���֣�ֵ�����ļ�
//                 file << pIDArray[ 0 ][ i ] << "\t" << pNameArray[ 0 ][ i ].c_str() << "\t" << dwMaxData;
//                 if ( i != num - 1 ) file << std::endl;
//                      
//                 lua_pushnumber( L, pIDArray[ 0 ][ i ] );
//                 ++retCount;
// 
//                 tempPlayerList.erase( MaxIt ); 
//             }
// 
//             file.close();
//         }
//         else if ( operType == 2 )  // ֻ��ȡ
//         {
//             for ( int i = 0; i < 5; ++i )
//             {
//                 if ( pIDArray[ 0 ][ i ] == 0 )
//                     break;
//                 lua_pushnumber( L, pIDArray[ 0 ][ i ] );
//                 ++retCount;
//             }
//         }
// 
//         return retCount;
//     }
    */
	static int L_RefuseAddLeaveFaction( lua_State* L)
	{
		if ( GetGW() == NULL )
			return 0;

		bool bIsRefused = (lua_toboolean(L , 1) == 1);
		LPCSTR errorInfo = (LPCSTR) lua_tostring(L, 2);

		GetGW()->m_FactionManager.m_bRefused = bIsRefused;

		if (errorInfo)
			GetGW()->m_FactionManager.m_sErrInfo = errorInfo;
		return 0;
	}
    static int L_DeltelergyByID( lua_State* L)
    {
//         if ( g_Script.m_pPlayer == NULL )
//             return 0;
// 
//         DWORD dwID = static_cast<DWORD>(lua_tonumber(L, 1));
// 
//         for( int n = 0; n < MAX_EQUIPTELERGY; ++n ) 
//         {
//             if( g_Script.m_pPlayer->m_Property.m_Telergy[n].byTelergyLevel == 0 )
//                 continue;
// 
//             if( g_Script.m_pPlayer->m_Property.m_Telergy[n].wTelergyID == dwID )
//             {         
//                 WORD id = g_Script.m_pPlayer->m_Property.m_Telergy[n].wTelergyID;
//                 BYTE level = g_Script.m_pPlayer->m_Property.m_Telergy[n].byTelergyLevel;
//                 DWORD value = g_Script.m_pPlayer->m_Property.m_Telergy[n].dwTelergyVal;
//                 g_Script.m_pPlayer->m_Property.m_Telergy[n].wTelergyID       = 0;
// 
//                 //g_Script.m_pPlayer->m_Property.m_Telergy[n].byTelergyLevel   = 0;
// 				g_Script.m_pPlayer->SetTelergyLevel(n,0);
// 
//                 g_Script.m_pPlayer->m_Property.m_Telergy[n].dwTelergyVal     = 0;
//                 g_Script.m_pPlayer->SendTelergyChange();
//                 g_Script.m_pPlayer->CalculateTelergyEffects();
//                 g_Script.m_pPlayer->SendPlayerAbility();          
// 
//                 extern BOOL TraceInfo_C( LPCSTR szFileName, LPCSTR szFormat, ... );
//                 TraceInfo_C( "LOGS\\script_deltelergy.log", "�˺�[%s]����[%s] ID level value[%d, %d, %d]", g_Script.m_pPlayer->GetAccount(), g_Script.m_pPlayer->GetName(), id, level, value );
// 
//                 return 0;
//             }
//         }

        return 0;
    }
    
	/// �ж�����Ƿ���ĳ��״̬
	/// ����: ״̬���
	static int L_HasState( lua_State* L)
	{
		if ( g_Script.m_pPlayer == NULL )
			return 0;
		DWORD type = static_cast<DWORD>(lua_tonumber(L, 1));
		lua_pushboolean( L, ( g_Script.m_pPlayer->m_dwExtraState & ( 1 << type ) ) != 0 );
		return 1;
	}
	
	/// ǿ��
	/// ����: ����, �⹦, �ڹ�, ��
	static int L_AddDoctorBuff( lua_State* L)
	{
		if ( g_Script.m_pPlayer == NULL )
			return 0;
		BYTE defBuff = static_cast<BYTE>(lua_tonumber(L, 1));
		BYTE damBuff = static_cast<BYTE>(lua_tonumber(L, 2));
		BYTE powBuff = static_cast<BYTE>(lua_tonumber(L, 3));
		BYTE agiBuff = static_cast<BYTE>(lua_tonumber(L, 4));
		if ( defBuff > 100 || damBuff > 100 || powBuff > 100 || agiBuff > 100 )
			return 0;
		g_Script.m_pPlayer->m_byDoctorDefBUFF = defBuff;
		g_Script.m_pPlayer->m_byDoctorDamBUFF = damBuff;
		g_Script.m_pPlayer->m_byDoctorPowBUFF = powBuff;
		g_Script.m_pPlayer->m_byDoctorAgiBUFF = agiBuff;
		g_Script.m_pPlayer->DoAddDoctorBuff( 0, true );
		return 0;
	}

	//�޸��ֵܽ�������Ϊ���д��Ŀǰֻ�������ֵ��������
	//������1��ԭ��������ǰ׺+��׺�� 2�������֣���Ϊ�ֻ��ǰ׺���Ծ�ֻ����4��ǰ׺��
	//static int L_ModifyUnionName( lua_State* L)
	//{  
	//	//if ( g_Script.m_pPlayer == NULL )
	//	//	return 0;

	//	LPCSTR oldName = ( LPCSTR )(lua_tostring(L, 1));
	//	LPCSTR newName = ( LPCSTR )(lua_tostring(L, 2));

	//	if(oldName[0] == 0 || newName[0] == 0)
	//		return 0;

	//	SPlayerUnionData *uniuData;
	//	uniuData = GetGW()->m_UnionManager.GetUnionDataByUnionName(oldName);

	//	if (uniuData == NULL)
	//		return 0;

	//	string sNewName = newName;
	//	for (int i = 0; i < 8; i++)
	//	{
	//		if (uniuData->roleid[i] != 0)//��Ȼ������ģ�����Ϊ�˱�֤ÿ���ֵܶ����ĵ���ȫ������
	//		{
	//			GetGW()->m_UnionManager.unionMappedMap[uniuData->roleid[i]] = sNewName;
	//		}
	//	}

	//	SPlayerUnionData newData;
	//	memcpy(&newData, uniuData, sizeof(SPlayerUnionData));
	//	dwt::strcpy(newData.title, newName, 9);
	//	newData.title[8] = 0;
	//	newData.suffx[0] = 0;
	//	GetGW()->m_UnionManager.m_UnionDataMap[sNewName] = newData;
	//	GetGW()->m_UnionManager.DeleteUnionDataByUnionName(oldName);//ɾ��ԭ����

	//	return 0;
	//}

	//�޸��ֵ�����ɫ����ã�
	//����	���ֵܽ������� ����ɫ��0, ԭɫ 1����ɫ 2������ɫ 3����ɫ��
	static int L_SetUnionNameColor( lua_State* L)
	{
// 		LPCSTR UnionName = ( LPCSTR )(lua_tostring(L, 1));
// 		BYTE color = static_cast<BYTE>(lua_tonumber(L, 2));
// 
// 		if(UnionName == NULL || color > 3 )
// 			return 0;
// 
// 		CPlayer *player = NULL;
// 		SPlayerUnionData *uniuData = NULL;
// 		uniuData = GetGW()->m_UnionManager.GetUnionDataByUnionName(UnionName);
// 
// 		if (uniuData == NULL)
// 			return 0;
// 
// 		for (int i = 0; i < 8; i++)
// 		{
// 			player = (CPlayer *)GetPlayerBySID(uniuData->roleid[i])->DynamicCast( IID_PLAYER );
// 			if (player)//��Ȼ������ģ�����Ϊ�˱�֤ÿ���ֵܶ����ĵ���ȫ������
// 			{	//��������ɵ�״̬
// 				player->m_dwExtraState &= ~(SHOWEXTRASTATE_UNIONRED|SHOWEXTRASTATE_UNIONGOLD|SHOWEXTRASTATE_UNIONPURPLE);
// 				switch (color)
// 				{
// 					case 1:
// 						player->m_dwExtraState |= SHOWEXTRASTATE_UNIONRED;
// 						break;
// 					case 2:
// 						player->m_dwExtraState |= SHOWEXTRASTATE_UNIONGOLD;
// 						break;
// 					case 3:
// 						player->m_dwExtraState |= SHOWEXTRASTATE_UNIONPURPLE;
// 						break;
// 					default:
// 						break;
// 				}
// 				player->SendExtraState();
// 			}
// 		}

		return 0;
	}

	//��������8���ֵܵȼ�֮��
	//���� ���ֵ���
	static int L_CalcUnionTotalLevel( lua_State* L)
	{
// 		LPCSTR UnionName = ( LPCSTR )(lua_tostring(L, 1));
// 
// 		if (UnionName == NULL)
// 			return 0;
// 
// 		SPlayerUnionData *uniuData = NULL;
// 		uniuData = GetGW()->m_UnionManager.GetUnionDataByUnionName(UnionName);
// 		if (uniuData == NULL)
// 			return 0;
// 
// 		int totalLevel = 0;
// 		CPlayer *player = NULL;
// 		for (int i = 0; i < 8; i++)
// 		{
// 			player = (CPlayer *)GetPlayerBySID(uniuData->roleid[i])->DynamicCast( IID_PLAYER );
// 			if (player)//ֻͳ�����ߵ�
// 				totalLevel += player->m_Property.m_Level;
// 		}
// 
// 		if (totalLevel == 0)//0Ӧ�ñ�ʾû������壬��֮�������κ���
// 			return 0;
// 
// 		lua_pushnumber(L, totalLevel);
// 		return 1;
		return 0;
	}

	//����������ֵܵȼ��Ƚ�(�ֱ��ҵ����������������ֵ�����ߵȼ��Ƚϣ�����ٱȵڶ����Դ�����)
	//���� �ٽ���1���� �ڽ���2����   ���Ƚϲ���1�Ƿ���ڲ���2 ��
	//����ֵ ���ڷ�����ֵ1 ���ڷ�����ֵ0 С�ڷ���-1
	static int L_UnionMembersLevelCompare( lua_State* L)
	{
// 		LPCSTR unionName1 = ( LPCSTR )(lua_tostring(L, 1));
// 		LPCSTR unionName2 = ( LPCSTR )(lua_tostring(L, 2));
// 
// 		if (unionName1 == NULL || unionName2 == NULL)
// 			return 0;
// 
// 		SPlayerUnionData* unionData1 = GetGW()->m_UnionManager.GetUnionDataByUnionName(unionName1);
// 		if (unionData1 == NULL)
// 			return 0;
// 
// 		SPlayerUnionData* unionData2 = GetGW()->m_UnionManager.GetUnionDataByUnionName(unionName2);
// 		if (unionData2 == NULL)
// 			return 0;
// 
// 		std::vector<DWORD>  levels1;
// 		std::vector<DWORD>  levels2;
// 		CPlayer *player = NULL;
// 		for (int i = 0; i < 8; i++)
// 		{
// 			player = (CPlayer *)GetPlayerBySID(unionData1->roleid[i])->DynamicCast( IID_PLAYER );
// 			if (player)
// 				levels1.push_back(player->m_Property.m_Level);
// 
// 			player = (CPlayer *)GetPlayerBySID(unionData2->roleid[i])->DynamicCast( IID_PLAYER );
// 			if (player)
// 				levels2.push_back(player->m_Property.m_Level);
// 		}
// 
// 		sort(levels1.begin(), levels1.end(), greater<DWORD>());
// 		sort(levels2.begin(), levels2.end(), greater<DWORD>());
// 
// 		for(vector<DWORD>::iterator iter1=levels1.begin(),iter2=levels2.begin();(iter1 != levels1.end()) && (iter2 != levels2.end());)
// 		{
// 			if (*(iter1) > *(iter2))
// 			{
// 				lua_pushnumber(L, 1);
// 				return 1;
// 			}
// 			else if (*(iter1) < *(iter2))
// 			{
// 				lua_pushnumber(L, -1);
// 				return 1;
// 			}
// 
// 			++iter1,++iter2;
// 		}
// 
// 		if( levels1.size() > levels2.size() )
// 		{
// 			lua_pushnumber(L, 1);
// 			return 1;
// 		}
// 		else if( levels1.size() < levels2.size() )
// 		{
// 			lua_pushnumber(L, -1);
// 			return 1;
// 		}
// 
// 		lua_pushnumber(L, 0);
// 		return 1;
		return 0;
	}

	//�޸��ֵ�����ɫ����ã�
	//����	��SID ���SID �� ��ɫ
	static int L_SetUnionNameColorOnePlayer( lua_State* L)
	{
// 		DWORD sid = static_cast<DWORD>(lua_tonumber(L, 1));
// 		BYTE color = static_cast<BYTE>(lua_tonumber(L, 2));
// 		if( color > 3 )
// 			return 0;	
// 
// 		CPlayer *player = (CPlayer *)GetPlayerBySID(sid)->DynamicCast( IID_PLAYER );
// 		if (player)
// 		{	//������ɵ�״̬
// 			player->m_dwExtraState &= ~(SHOWEXTRASTATE_UNIONRED|SHOWEXTRASTATE_UNIONGOLD|SHOWEXTRASTATE_UNIONPURPLE);
// 			switch (color)
// 			{
// 			case 1:
// 				player->m_dwExtraState |= SHOWEXTRASTATE_UNIONRED;
// 				break;
// 			case 2:
// 				player->m_dwExtraState |= SHOWEXTRASTATE_UNIONGOLD;
// 				break;
// 			case 3:
// 				player->m_dwExtraState |= SHOWEXTRASTATE_UNIONPURPLE;
// 				break;
// 			default:
// 				break;
// 			}
// 			player->SendExtraState();
// 		}

		return 0;
	}

    static int L_SendTransferMsgTable( lua_State* L )
    {
        SAGameScriptTransfreMsg msg;
        int top = lua_gettop( L );

        msg.dbcId = static_cast< DWORD >( lua_tonumber( L, 1 ) );
        msg.destSercerId = static_cast< DWORD >( lua_tonumber( L, 2 ) );
        
        int size = sizeof( msg.streamData );
        int cur1 = luaEx_serialize( L, 3, msg.streamData + 4, size - 4 );
        int nowSize = size - ( cur1 + 4 );

        if ( cur1 < 0 || cur1 >  size - 4 )
            goto __error_SendTrn_exit;

        *( LPDWORD )msg.streamData = cur1;

        SendToLoginServer( &msg, sizeof( msg ) - nowSize );
        lua_pushnumber(L, 1);
        return 1;

__error_SendTrn_exit:
        if ( lua_gettop( L ) != top )
            lua_settop( L, top );

        return 0;
    }

    //if ( ck <= 0 || ck != *( LPDWORD )teBuffer->saveData )
    static int L_GetTransferMsgTable( lua_State* L )
    {
        int size = 0;
        int top = lua_gettop( L );

        void *data = luaEx_touserdata( L, 1, &size );

        if ( data == NULL || size <= 0 || size > 4096 )
            return 0;

        lua_createtable( L, 0, 0 ); // ����data�ӱ�
        int ck = luaEx_unserialize( L, ( LPBYTE )data + 4, *( LPDWORD )data );

        if ( ck <= 0 || ck != *( LPDWORD )data )
            goto __error_GetTrn_exit;

        return 1;
__error_GetTrn_exit:
        if ( lua_gettop( L ) != top )
            lua_settop( L, top );

        return 0;
    }

    // �õ��־������Ϣ����:1(����).0����ʱ�� 2.ʣ��ʱ��
    static int L_GetDecDurInf( lua_State* L )
    {
//         if ( g_Script.m_pPlayer == NULL )
//             return 0;
// 
//         int nInfType = static_cast<int>(lua_tonumber(L, 1)); 
//         DWORD dwNowTime = (DWORD)time( NULL );
// 
//         if ( nInfType == 0 )    //������ʱ��
//         {
//             if ( g_Script.m_pPlayer->m_Property.dwDecDurTimeEnd != 0 && dwNowTime < g_Script.m_pPlayer->m_Property.dwDecDurTimeEnd )
//                 lua_pushnumber(L, g_Script.m_pPlayer->m_Property.dwDecDurTimeEnd);
//             else 
//                 lua_pushnumber(L, 0);
// 
//             return 1;
//         }
//         else if ( nInfType == 1 )   //��ʣ��ʱ��
//         {
//             if ( g_Script.m_pPlayer->m_Property.dwDecDurTimeEnd != 0 && dwNowTime < g_Script.m_pPlayer->m_Property.dwDecDurTimeEnd )
//                 lua_pushnumber(L, g_Script.m_pPlayer->m_Property.dwDecDurTimeEnd - dwNowTime );
//             else
//                 lua_pushnumber(L, 0);
// 
//             return 1;
//         }

        return 0;
    }

    // ����...  ����:1.����ʱ��
    static int L_SetDecDurInf( lua_State* L )
    {
//         if ( g_Script.m_pPlayer == NULL )
//             return 0;
// 
//         int nInfType    = static_cast<int>(lua_tonumber(L, 1));        
//         DWORD dwNowTime = (DWORD)time( NULL );
//         bool isChange   = false;
// 
//         if ( nInfType == 0 )        //�����¿�ʼ
//         {
//             DWORD dwDurtime = static_cast<DWORD>(lua_tonumber(L, 2));
//             if ( dwDurtime != 0 )
//             {
//                 g_Script.m_pPlayer->m_Property.dwDecDurTimeEnd = dwNowTime + dwDurtime;
//                 isChange = true;
//             }
//         }
//         else if ( nInfType == 1 )   // �ۼ�
//         {
//             DWORD dwDurtime = static_cast<DWORD>(lua_tonumber(L, 2));
//             if ( dwDurtime != 0 )
//             {
//                 DWORD dwLastTime = ( ( g_Script.m_pPlayer->m_Property.dwDecDurTimeEnd != 0 && dwNowTime < g_Script.m_pPlayer->m_Property.dwDecDurTimeEnd) ? (g_Script.m_pPlayer->m_Property.dwDecDurTimeEnd - dwNowTime) : 0 );
//                 g_Script.m_pPlayer->m_Property.dwDecDurTimeEnd = dwNowTime + dwLastTime + dwDurtime;
//                 isChange = true;
//             }
//         }
//         else if ( nInfType == 2)
//         {
//             g_Script.m_pPlayer->m_Property.dwDecDurTimeEnd = 0;
//             isChange = true;
//         }
// 
//         if ( isChange )
//             g_Script.m_pPlayer->SendUpdateDecDurTimeMsg();
        return 0;
    }

	static int L_AddPlayerCurHp( lua_State* L )
	{
		return 0;
	}

	static int L_AddPlayerCurMp( lua_State* L )
	{
		return 0;
	}

	static int L_AddPlayerCurSp( lua_State* L )
	{
		return 0;
	}
};


namespace ExtraScriptFunctions 
{ 
    void RegisterScriptFunctions1()
    {

    #define RegisterFunction( _funcname ) g_Script.RegisterFunction( #_funcname, ExtraLuaFunctions::L_##_funcname );

	    RegisterFunction( GetBuildingData			);
	    RegisterFunction( SetBuildingData			);
	    RegisterFunction( UpdateBuildingData		);
	    RegisterFunction( GetBuildingCurPos			);
		RegisterFunction( StartBuild				);
		RegisterFunction( SetBuildTrap				);
		RegisterFunction( GetFactionInfo			);
		RegisterFunction( SetFactionInfo			);
		RegisterFunction( DeleteFaction				);
		RegisterFunction( GetFactionIdList	        );
		RegisterFunction( GetGroupID				);
        RegisterFunction( RemoveBuiding             );

        RegisterFunction( SetVenapointEx            );
        RegisterFunction( IsPassVenationEx          );
        RegisterFunction( GetVenapointExNum         );
        RegisterFunction( IsPassVenation            );
        RegisterFunction( SetVenapointCount         );
        RegisterFunction( ShowDDial                 );
        RegisterFunction( SetDDialAward             );
		RegisterFunction( GetSkill                  );
        RegisterFunction( PlayerMoveTo              );
		RegisterFunction( SetAffiche                );
        RegisterFunction( GetRandomPlayerSID        );
        RegisterFunction( SetMasterPrenticeName     );

		RegisterFunction( OnInvitatoryUnion         );
		RegisterFunction( OnOpenUnionDialog		    );
		RegisterFunction( AddPlayerToUnion          );
		RegisterFunction( DeletePlayerFormUnion     );
		RegisterFunction( IsHaveInUnion             );
		RegisterFunction( GetUnionMenbetNum         );
        RegisterFunction( SendLuaMsg                );
		RegisterFunction( SendTipsMsg               );

        RegisterFunction( GetObjectList             );
        RegisterFunction( SetTempEffect             );

        RegisterFunction( GetSkillUseTimes          );
        RegisterFunction( PlayerDontTalk            );
        RegisterFunction( PutPrison                 );
        RegisterFunction( GetSpecialBuffAdd         );
        RegisterFunction( GetFactionMemberInfo      );
        RegisterFunction( SetState                  );
        RegisterFunction( GetEquips                 );
        RegisterFunction( GetPlayer                 );
        RegisterFunction( LimitUseItem              );
        RegisterFunction( CalcJianDingAttribute     );
        RegisterFunction( LoadDropItemList          );
        RegisterFunction( GetRegionCount            );
        RegisterFunction( SetEnvironment            );
        RegisterFunction( SetRegionMultExp          );
        RegisterFunction( SetPlugingameState        );
        RegisterFunction( SetMedicalEthicInf        );
        RegisterFunction( GetMedicalEthicInf        );
        RegisterFunction( GetTodayAchieveExpCount   );
	    RegisterFunction( GetWulinChiefPos	        );
	    RegisterFunction( GetCityOwenerPos	        );
        RegisterFunction( UpdataPlayerPaiHangJoinData );
	    RegisterFunction( GetPlayerPaiHangJoinData  );
        RegisterFunction( ClearAllFactionProsperity );
	    RegisterFunction( CalcAndSaveProspTopFiveFaction );
		RegisterFunction( RefuseLeaveUnion			);
		RegisterFunction( GetUnionInfo				);
		RegisterFunction( AddWhineBUFF				);
		RegisterFunction( GetWhineBUFFType			);
        RegisterFunction( GetAppointRegionRandomPos );
		RegisterFunction( RefuseAddLeaveFaction		);
		RegisterFunction( GetTodayContribution      );
		RegisterFunction( SetContribution			);
        RegisterFunction( DeltelergyByID			);
		RegisterFunction( HasState					);	/// �ж�����Ƿ���ĳ��״̬
		RegisterFunction( AddDoctorBuff				);	/// ǿ��
//		RegisterFunction( ModifyUnionName			);
		RegisterFunction( SetUnionNameColor			);
		RegisterFunction( CalcUnionTotalLevel		);
		RegisterFunction( UnionMembersLevelCompare	);
		RegisterFunction( SetUnionNameColorOnePlayer);
        RegisterFunction( SendTransferMsgTable      );
        RegisterFunction( GetTransferMsgTable       );
        RegisterFunction( GetDecDurInf              );
        RegisterFunction( SetDecDurInf              );

		RegisterFunction( AddPlayerCurHp			);	//������ҵĵ�ǰѪ��
		RegisterFunction( AddPlayerCurMp			);	//������ҵĵ�ǰ����
		RegisterFunction( AddPlayerCurSp			);	//������ҵĵ�ǰ����
	
		//RegisterFunction( SetAffiche			);	//ȫ��ͨ��
    #undef  RegisterFunction

    }
}
