#include "StdAfx.h"
#include "ExtraScriptFunctions.h"
#include "lite_lualibrary.hpp"

#include "pub/wndcusop.h"
#include "pub/traceinfo.h"

#include "gameobjects/region.h"
#include "gameobjects/monster.h"
#include "gameobjects/player.h"
#include "gameobjects/area.h"
#include "gameobjects/npc.h"
#include "gameobjects/item.h"
#include "gameobjects/gameworld.h"
#include "gameobjects/globalfunctions.h"
#include "gameobjects/scriptmanager.h"
#include "gameobjects/factionregion.h"
#include "gameobjects/eventregion.h"
#include "gameobjects/fightarena.h"
#include "gameobjects/prison.h"
#include "gameobjects/prisonex.h"
#include "gameobjects/biguanarea.h"
#include "gameobjects/building.h"

#include "networkmodule/netprovidermodule/netprovidermoduleadv.h"
#include "区域服务器/dmainapp.h"
#include "gameobjects/upgradesys.h"

#include "networkmodule/itemmsgs.h"
#include "networkmodule/scriptmsgs.h"
#include "networkmodule/tongmsgs.h"
#include "networkmodule/regionmsgs.h"
#include "networkmodule/cardpointmodifymsgs.h"
#include "networkmodule/refreshmsgs.h"
#include "networkmodule/accountmsgs.h"
#include "pub/rpcop.h"

#include "gameobjects/CMonsterService.h"

#include <time.h>

extern LPIObject FindRegionByGID( DWORD GID );
extern LPIObject FindRegionByID( DWORD ID );
extern BOOL GenerateNewUniqueId( SItemBase &item );
extern void ArrangePackage( SPackageItem **storage, DWORD count , CPlayer * _this );
extern LPCSTR GetNpcScriptInfo( LPCSTR info );
extern BOOL rfalse(char lTraceType, char bUseEndl, LPCSTR szFormat, ...);

extern CScriptManager g_Script;
//extern void GetAddAttribute( SEquipment::Attribute::Unit *unit, WORD id, int maxNum, WORD coff[ 57 ] = NULL, BYTE level = 0, BYTE itemType = 0 );

int exreaScriptArray[20] = { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0 };

BYTE sdbBuffer[0x800000];
BYTE tdbBuffer[0x800000];

BOOL CScriptManager::PrepareFunction( LPCSTR funcname )
{
	assert( ( lua_gettop( ls ) == 0 ) && ( prepared_paramnum == -1 ) );
	//rfalse(2, 1, "LuaStack num = %d,  LuaCall %s", lua_gettop(ls), funcname);
	if ( lua_gettop( ls ) != 0)
	{
		int nlenth = lua_gettop(ls);
		rfalse(2, 1, FormatString("%s   get a invalid lua stack position!  topnum = %d  \r\n", funcname, nlenth));
		for (size_t i = 0; i < nlenth; i++)
		{
			if (lua_type(ls, i + 1) == LUA_TNUMBER || lua_type(ls, i + 1) == LUA_TBOOLEAN)
			{
				int num = lua_tonumber(ls, i+1);
				rfalse(2, 1, FormatString("%s   get a invalid lua stack position! paramindex = %d  num = %d  \r\n", funcname,i+1, num));
			}
			else if (lua_type(ls, i + 1) == LUA_TSTRING)
			{
				const char* str = lua_tostring(ls, i+1);
				rfalse(2, 1, FormatString("%s   get a invalid lua stack position! paramindex = %d  string = %s  \r\n", funcname, i + 1, str));
			}
			else
			{
				int ntype = lua_type(ls, i + 1);
				const char* str = lua_tostring(ls, i + 1);
				rfalse(2, 1, FormatString("%s   get a invalid lua stack position! paramindex = %d  type = %d ,name = %s  \r\n", funcname, i + 1, ntype,str));
			}
		}
		lua_settop(ls, 0);
		//return FALSE;
	}

	if ( ( lua_gettop( ls ) != 0 ) && ( prepared_paramnum == -1 ) )
	{
		OutputDebugString( "get a invalid lua stack position!\r\n" );
		rfalse(1,1,FormatString("%s   get a invalid lua stack position!  size = %d  \r\n",funcname,lua_gettop( ls )));
		return FALSE;
	}

	lua_settop( ls, 0 );
	prepared_paramnum = 0;
	lua_getglobal( ls, funcname );
	if ( lua_isnil( ls, -1 ) )
	{
		//rfalse(2,1,FormatString("PrepareFunction %s not find \r\n",funcname));
		lua_pop( ls, 1 );
		return FALSE;
	}
	return TRUE;
}

void CScriptManager::PrepareBreak()
{
	assert( ( lua_gettop( ls ) > 0 ) || ( prepared_paramnum != -1 ) );
	if ( ( lua_gettop( ls ) <= 0 ) || ( prepared_paramnum == -1 ) )
		return;

	lua_settop( ls, 0 );
	prepared_paramnum = -1;
}

#define _DeclareXcpt( XcptName, ErrCode, ErrInfo ) namespace lite { class XcptName : public lite::Xcpt { \
public : XcptName() : Xcpt( ErrCode ) {} LPCSTR GetErrInfo() { return ErrInfo; } }; }

_DeclareXcpt( InvalidArray,         0x00000008, "错误的序列化数组" )
	_DeclareXcpt( CreateTableFailure,   0x00000009, "创建lua表失败！" )
	_DeclareXcpt( PushVariantFailure,   0x0000000a, "将序列化变量压入lua栈失败！" )
	_DeclareXcpt( StackBreak,           0x0000000b, "lua在反序列化过程中被破坏了！" )

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

void CScriptManager::PrepareParameters( lite::Serialreader &slr )
{
	if ( prepared_paramnum < 0 )
		return;

	int top = lua_gettop( ls );

	// 将所有参数放入调用栈
	lua_createtable( g_Script.ls, 0, 0 );
	if ( !lua_istable( g_Script.ls, -1 ) )
		throw lite::CreateTableFailure();

	int count = 1;
	while ( slr.curSize() < slr.maxSize() )
	{
		lite::Variant lvt = slr();
		if ( lite::Variant::VT_RPC_OP == lvt.dataType )
		{
			// 说明返回的是一个数组
			if ( lvt._rpcop != 0x80 )
				throw lite::InvalidArray();

			DWORD number = slr();
			if ( number > 1000 )
				throw lite::InvalidArray();

			lua_createtable( ls, number, 0 );
			if ( !lua_istable( ls, -1 ) )
				throw lite::CreateTableFailure();

			for ( DWORD i = 0; i < number; i ++ )
			{
				if ( !reinterpret_cast< lite::lua_variant & >( slr() ).push( ls ) )
					throw lite::PushVariantFailure();

				lua_rawseti( ls, -2, i + 1 );
			}
		}
		else if ( !reinterpret_cast< lite::lua_variant const & >( lvt ).push( g_Script.ls ) )
			throw lite::PushVariantFailure();

		lua_rawseti( g_Script.ls, -2, count++ );
	}

	// 启动调用！
	assert( top + 1 == lua_gettop( g_Script.ls ) );
	if ( top + 1 != lua_gettop( g_Script.ls ) )
		throw lite::StackBreak();

	prepared_paramnum = 1;
}

void CScriptManager::PushVariantArray( lite::Serialreader &slr )
{
	// 注意，目前的外部调用只有一个地方，并且那里已经做了lite::Xcpt的异常处理！
	if ( prepared_paramnum < 0 )
		return;

	DWORD number = slr();

	if ( number > 100 )
		throw lite::InvalidArray();

	lua_createtable( ls, number, 0 );
	if ( !lua_istable( ls, -1 ) )
		throw lite::CreateTableFailure();

	for ( DWORD i = 0; i < number; i ++ )
	{
		if ( !reinterpret_cast< lite::lua_variant & >( slr() ).push( ls ) )
			throw lite::PushVariantFailure();

		lua_rawseti( ls, -2, i + 1 );
	}

	prepared_paramnum ++;
}

void CScriptManager::PushDWORDArray( std::list< DWORD > &lst )
{
	// 注意，目前的外部调用只有一个地方，并且那里已经做了lite::Xcpt的异常处理！
	if ( prepared_paramnum < 0 )
		return;

	DWORD number = (DWORD)lst.size();
	if ( number > 100 )
		throw lite::InvalidArray();

	lua_createtable( ls, number, 0 );
	if ( !lua_istable( ls, -1 ) )
		throw lite::CreateTableFailure();

	int i=1;
	for ( std::list< DWORD >::iterator it = lst.begin(); it != lst.end(); it++, i++ ) {
		lua_pushnumber( ls, *it );
		lua_rawseti( ls, -2, i );
	}

	prepared_paramnum ++;
}

void CScriptManager::PushVariantTable( lite::Serialreader &slr, int tableType )
{
	if ( prepared_paramnum < 0 )
		return;

	DWORD colNumber = slr();
	DWORD rowNumber = slr();

	if ( colNumber > 100 || rowNumber > 10000 )
		throw lite::InvalidArray();

	if ( colNumber == 0 || rowNumber == 0 )
		lua_pushnil( ls );
	else
	{
		lua_createtable( ls, ( tableType == '|' ) ? rowNumber * colNumber : rowNumber, 2 );
		if ( !lua_istable( ls, -1 ) )
			throw lite::CreateTableFailure();

		lua_pushstring( ls, "rows" ); 
		lua_pushnumber( ls, rowNumber ); 
		lua_settable( ls, -3 );

		lua_pushstring( ls, "cols" ); 
		lua_pushnumber( ls, colNumber ); 
		lua_settable( ls, -3 );

		lua_pushstring( ls, "type" ); 
		lua_pushnumber( ls, tableType ); 
		lua_settable( ls, -3 );

		// 2维表格
		if ( tableType == '#' )
		{
			for ( DWORD i = 0; i < rowNumber; i ++ )
			{
				lua_createtable( ls, colNumber, 0 );
				if ( !lua_istable( ls, -1 ) )
					throw lite::CreateTableFailure();

				for ( DWORD j = 0; j < colNumber; j ++ )
				{
					if ( !reinterpret_cast< lite::lua_variant & >( slr() ).push( ls ) )
						throw lite::PushVariantFailure();

					lua_rawseti( ls, -2, j + 1 );
				}

				lua_rawseti( ls, -2, i + 1 );
			}
		}

		// 1维表格
		else if ( tableType == '|' )
		{
			DWORD elements = rowNumber * colNumber;
			for ( DWORD i = 0; i < elements; i ++ )
			{
				if ( !reinterpret_cast< lite::lua_variant & >( slr() ).push( ls ) )
					throw lite::PushVariantFailure();

				lua_rawseti( ls, -2, i + 1 );
			}
		}
	}

	prepared_paramnum ++;
}

void CScriptManager::PushParameter( lite::Variant const &lvt )
{
	if ( prepared_paramnum < 0 )
		return;

	if ( !reinterpret_cast< lite::lua_variant const & >( lvt ).push( ls ) )
		return;

	prepared_paramnum ++;
}

BOOL CScriptManager::Execute( lite::Variant *result )
{
	if ( prepared_paramnum < 0 )
		return FALSE;

	assert( prepared_paramnum + 1 == lua_gettop( ls ) );
	if ( prepared_paramnum + 1 != lua_gettop( ls ) )
		return PrepareBreak(), FALSE;

	BOOL ret = TRUE;
	if ( lua_pcall( ls, prepared_paramnum, result ? 1 : 0, 0 ) != 0 )
	{
		ret = FALSE;

		char err[1024];
		sprintf( err, "%s\r\n", lua_tostring( ls, -1 ) );
// 		if (lua_isstring(ls,1))
// 		{
// 			const char* fname = lua_tostring(ls,1);
// 			char errfname[1024]={};
// 			sprintf(errfname,"%s  %s",fname,err);
// 			strcpy(err,errfname);
// 		}
		OutputDebugString( err );
		rfalse(1,1,err);
	}

	if ( ( result ) && ( lua_gettop( ls ) == 1 ) )
		reinterpret_cast< lite::lua_variant& >( *result ).get( ls, -1 );

	lua_settop( ls, 0 );
	prepared_paramnum = -1;

	return ret;
}

static void SaveTaskSingleFile( std::map< std::string, STaskExBuffer >::iterator &it )
{
	// 在保存前，需要尝试先从脚本里更新一次！！！
	lite::Variant lvt;
	LuaFunctor( g_Script, "SaveDBTable" )[ it->first.c_str() ]( &lvt );
	if ( lvt.dataType <= lite::Variant::VT_NULL )
		return; // 如果更新失败，取消保存！！！

	STaskExBuffer *teb = &it->second;
	if ( teb->saveData == NULL )
		return;

	DWORD saveSize = 4 + *( LPDWORD )teb->saveData;
	if ( saveSize > (DWORD)teb->saveSize )
		return;

	char filename[256];
	sprintf( filename, "dbTables/%s.dbt", it->first.c_str() );
	HANDLE hFile = CreateFile( filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		return; // 如果失败直接返回

	DWORD size = 0;
	BOOL rt = WriteFile( hFile, teb->saveData, saveSize, &size, NULL );
	CloseHandle( hFile );
}

void CScriptManager::SaveTaskEx( LPCSTR key )
{
	CreateDirectory( "dbTables", NULL );
	if ( key != NULL )
	{
		// 先查找是否已经缓存，如果缓存过了，则直接读取，否则就去找对应的数据文件
		std::map< std::string, STaskExBuffer >::iterator it = taskExMgr.find( key );
		if ( it != taskExMgr.end() )
			SaveTaskSingleFile( it );
	}
	else
	{
		for ( std::map< std::string, STaskExBuffer >::iterator it = taskExMgr.begin();
			it != taskExMgr.end(); it ++ )
			SaveTaskSingleFile( it );
	}
}

STaskExBuffer* CScriptManager::LoadTaskEx( LPCSTR key )
{
	// 先查找是否已经缓存，如果缓存过了，则直接读取，否则就去找对应的数据文件
	std::map< std::string, STaskExBuffer >::iterator it = taskExMgr.find( key );
	if ( it == taskExMgr.end() )
	{
		STaskExBuffer *teb = &taskExMgr[ key ];
		char filename[256];
		sprintf( filename, "dbTables/%s.dbt", key );
		HANDLE hFile = CreateFile( filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL );
		if ( hFile == INVALID_HANDLE_VALUE )
			return teb; // 如果没有文件，则返回一个空数据

		DWORD size = GetFileSize( hFile, NULL );
		teb->saveData = new BYTE[ teb->saveSize = size ];
		BOOL rt = ReadFile( hFile, teb->saveData, size, &size, NULL );
		CloseHandle( hFile );
		return teb;
	}

	return &it->second;
}

LuaFunctor::LuaFunctor( CScriptManager &script, LPCSTR funcName ) : 
funcName( funcName ), callStack( script.ls ), stackTop( lua_gettop( callStack ) )
{
	//rfalse(2, 1, "LuaStack num = %d,  LuaCall %s", lua_gettop(script.ls), funcName);
	lua_getglobal( callStack, funcName );
	if ( initFailre = lua_isnil( callStack, -1 ) ) 
		lua_settop( callStack, stackTop );
}

LuaFunctor& LuaFunctor::operator [] ( lite::Variant const &lvt ) 
{
	if ( initFailre )
		return *this;

	if ( initFailre = !reinterpret_cast< lite::lua_variant const & >( lvt ).push( callStack ) )
		lua_settop( callStack, stackTop );

	return *this;
}

BOOL LuaFunctor::operator () ( lite::Variant *result )
{
	if ( initFailre )
		return FALSE;
	int paramSize = lua_gettop( callStack ) - stackTop - 1;
	if ( paramSize < 0 )
		return FALSE;

	if ( lua_pcall( callStack, paramSize, result ? 1 : 0, 0 ) != 0 )
	{
		result = NULL;

		char err[1024];
		sprintf( err, "%s\r\n", lua_tostring( callStack, -1 ) );
		OutputDebugString( err );
		rfalse(1,1,err);
	}

	int resultSize = lua_gettop( callStack ) - stackTop;
	if ( resultSize < 0 )
		throw exception( "错误的lua调用结果（栈数据异常）" );

	if ( ( result ) && ( resultSize == 1 ) )
		reinterpret_cast< lite::lua_variant& >( *result ).get( callStack, -1 );

	lua_settop( callStack, stackTop );

	return TRUE;
}

// 动态创建一个场景, 由于要使用到表相关的操作, 所以将代码移动到这边来了
int CScriptManager::L_CreateRegion(lua_State *L)
{
	DWORD dwGID = 0;

	// 如果传入的参数是一个table的话，说明要使用新的场景创建逻辑了
	if (lua_istable(L, 1))
	{
#define CHECK_AND_SET( _key, _member ) _IF_GET_FIELD_NUMBER( 1, _key ) \
	param._member = static_cast< WORD >( __number ); else goto __checkfail;

		CFactionRegion::CreateParameter param;
		// 如果标识了这个数字的话，表示这个场景需要指定区域的，那么不是自己当前区域的话，就返回个0。。。
		DWORD zoneId = 0;
		DWORD needLevel = 0;
		_IF_GET_FIELD_NUMBER( 1, "zoneId" ) 
		{
			extern BYTE GetZoneID();
			zoneId = static_cast< DWORD >( __number );
			if ( zoneId != GetZoneID() ) 
				return 0;
		}

		// 如果标识了这个数字的话，说明是固定场景，直接读取就行了
		_IF_GET_FIELD_NUMBER( 1, "staticId" ) 
		{
			DWORD staticId = static_cast< DWORD >( __number );


			if ( FindRegionByID( staticId ) )
				return lua_pushnumber( L, 0 ), 1; // 目标场景已经存在！失败返回但不需要给提示！

			DWORD type = (DWORD)_GET_FIELD_NUMBER_DIRECTLY( 1, "type" );
			if ( type > 5 )
				return rfalse( 2, 1, "创建场景类型错误[%d]", type ), 0;

			static DWORD REGIONTYPE[5] = { IID_REGION, IID_FIGHTARENA, IID_PRISON, IID_PRISONEX, IID_BIGUANAREA };

			LPIObject region = GetApp()->m_pGameWorld->CRegionManager::GenerateObject( REGIONTYPE[type], 0, staticId );
			if ( !region )
				return rfalse( 2, 1, "无法装载目标场景 [%d:%d]", staticId, type ), 0;

			// 向登陆服务器发出请求, 获取当前场景上所有可能存在的建筑物!
// 			if ( type == 0 ) {
// 				SQBuildingGetMsg msg;
// 				msg.wParentID = (WORD)staticId;
// 				SendToLoginServer( &msg, sizeof( msg ) );
// 			}

			CRegion *ptr = ( CRegion* )region->DynamicCast( IID_REGION );
			if ( ptr == NULL )
				return rfalse( 2, 1, "错误的目标场景指针 [%d]", staticId ), 0;

			param.mapTemplateId = (WORD)staticId;
			param.factionRegionId = (WORD)staticId;
			param.parentRegionId = ptr->m_wReLiveRegionID;
			param.xEntry = (WORD)ptr->m_ptReLivePoint.x;
			param.yEntry = (WORD)ptr->m_ptReLivePoint.y;
			param.name = "static";
			needLevel = ptr->m_dwNeedLevel;
		}
else
{
	CHECK_AND_SET( "regionId"  , factionRegionId )  // 当前场景的唯一ID编号
		CHECK_AND_SET( "templateId", mapTemplateId   )  // 当前场景的模板场景编号
		CHECK_AND_SET( "parentId"  , parentRegionId  )  // 当前场景的出口通向的目标场景和坐标
		CHECK_AND_SET( "xEntry"    , xEntry          )  
		CHECK_AND_SET( "yEntry"    , yEntry          )  
		_IF_GET_FIELD_STRING( 1, "owner" ) param.name = __string; // 该场景的拥有者的（帮派）名称．．．
else { __checkfail: return rfalse( 2, 1, "CreateRegion : 错误的参数" ), 0; }  

if ( !GetApp()->m_pGameWorld->CRegionManager::GenerateObject( IID_FACTIONREGION, 0, ( LPARAM )&param ) )
	return rfalse( 2, 1, "无法装载目标场景 [%d][%s]", param.mapTemplateId, param.name.c_str() ), 0;
}
 
// 如果都创建成功,就将这些帮派场景的编号通知到登陆服务器！
char tempBuffer[ 256 ];
SARefreshRegionServerMsgEx &msg = * new ( tempBuffer ) SARefreshRegionServerMsgEx;
msg.dnidClient = 0xff12ff34ff56ff78;
lite::Serializer slm( msg.dataStream, sizeof( tempBuffer ) - sizeof( msg ) );
SRegion r;
dwt::strcpy( r.szName, param.name.c_str(), sizeof( r.szName ) );
r.ID = param.factionRegionId;
r.MapID = param.mapTemplateId;
r.NeedLevel = 0;
slm( &r, sizeof( r ) );
SendToLoginServer( &msg, ( int )( sizeof( tempBuffer ) - slm.EndEdition() ) );
dwGID = param.factionRegionId;
	}
	else
	{
		extern DWORD CreateDynamicRegion(const __int32 DR_IID, WORD RegionIndex, int RegionID, int, int, int, DWORD,WORD);

		WORD	RegionIndex		= static_cast<WORD>	( lua_tonumber( L, 1 ) );//20000 lua编号
		int		RegionID        = static_cast<int>  ( lua_tonumber( L, 2 ) );
		int		RelifeRegionID  = static_cast<int>  ( lua_tonumber( L, 3 ) );
		int		RelifeX         = static_cast<int>  ( lua_tonumber( L, 4 ) );
		int		RelifeY         = static_cast<int>  ( lua_tonumber( L, 5 ) );
		DWORD	copySceneGID	= static_cast<DWORD>( lua_tonumber( L, 6 ) );
		WORD  wpktype = static_cast<WORD>(lua_tonumber(L, 7));

		dwGID = CreateDynamicRegion(IID_DYNAMICREGION, RegionIndex, RegionID, RelifeRegionID, RelifeX, RelifeY, copySceneGID, wpktype);
	}

	if (0 == dwGID)
		return 0;

	lua_pushnumber(L, dwGID);
	return 1;
}

int CScriptManager::L_creategift( lua_State *L )
{
	//	if (!g_Script.m_pPlayer)return 0;

	DWORD giftid = static_cast<DWORD>(lua_tonumber(L, 1)); //礼包ID
	if(giftid == 0)return 0;
	WORD num	= 0;
	std::list<SAddItemInfo> itemList;
	SRawItemBuffer *itemBuffer = g_Script.m_pItemBuffer;
	if(!itemBuffer) return 0;
	if (lua_istable(L,2))
	{
		while(lua_gettop(L))
		{
			switch (lua_type( L, lua_gettop( L ) )) 
			{
			case LUA_TTABLE:
				{
					lua_pushnil(L);
					int num = -2;
					DWORD itemid = 0xffffffff;
					WORD itemnum = 0xffff;
					while (lua_next(L, num) != 0)
					{
						/* 'key' (索引-2) 和 'value' (索引-1) */  
						const char* key = "unknown";  
						const char* value;  
						if(lua_type(L, -2) == LUA_TSTRING)  
						{  
							key = lua_tostring(L, -2);  
							value = lua_tostring(L, -1);  
						}  
						else if(lua_type(L, -2) == LUA_TNUMBER)  
						{  
							// 因为lua_tostring会更改栈上的元素，  
							// 所以不能直接在key上进行lua_tostring  
							// 因此，复制一个key，压入栈顶，进行lua_tostring  
							lua_pushvalue(L, -2);  
							key = lua_tostring(L, -1);  
							lua_pop(L, 1);  
							value = lua_tostring(L, -1);  
						}  
						else  
						{  
							value = lua_tostring(L, -1);  
						}  
						if (atoi(key)%2 != 0){
							itemid = atol(value);
						}
						else{
							itemnum = atol(value);
						}

						if (itemid != 0xffffffff && itemnum != 0xffff)
						{
							itemList.push_back(SAddItemInfo(itemid, itemnum));
							itemid = 0xffffffff;
							itemnum = 0xffff;
						}
						/* 移除 'value' ；保留 'key' 做下一次迭代 */  
						lua_pop(L, 1);
					}
				}
			}
			lua_pop( L, 1 );
		}		
	}

	num = itemList.size();
	if (num == 0)
	{
		AddInfo("需要添加到礼包中的物品一个都没有");
		return 0;
	}
	// 给玩家添加一个礼包道具

	const SItemBaseData *itemdata = CItemService::GetInstance().GetItemBaseData(giftid);
	if(!itemdata)
	{
		AddInfo(FormatString("没有找到礼包道具%d,生成道具失败",giftid));
		return 0;
	}

	// 	WORD pos = g_Script.m_pPlayer->FindBlankPos(XYD_PT_BASE);
	// 	if (0xffff == pos)
	// 	{
	// 		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex,"背包已满，无法获取");
	// 		return 0;
	// 	}

	BYTE *Buffer = itemBuffer->buffer;
	memcpy(Buffer,&num,sizeof(WORD));
	Buffer += sizeof(WORD);

	std::list<SAddItemInfo>::iterator iter = itemList.begin();
	while(iter != itemList.end())
	{
		SAddItemInfo iteminfo = *iter;
		memcpy(Buffer,&iteminfo.m_Index,sizeof(DWORD));
		Buffer += sizeof(DWORD);

		memcpy(Buffer,&iteminfo.m_Count,sizeof(WORD));
		Buffer += sizeof(WORD);

		iter++;
	}
	//lua_pushlightuserdata(L,itemBuffer);
	//g_Script.m_pPlayer->AddExistingItem(*itemBuffer, pos, true);
	return 1;

}

// 因为结构才能申明友元，所以无法用名字空间来代替
struct ExtraLuaFunctions
{
	struct Details
	{
		static void CreateFolder( LPCSTR path )
		{
			char str[1024]	= { 0 };
			int  state = 2, j = 0, len = static_cast< int >( strlen( path ) );

			for ( int i = 0; ( i < len ) && ( j < sizeof( str ) ); i++ )
			{
				if ( ( path[i] == '\\' ) || ( path[i] == '/' )  )
				{
					if ( state != 1 )
						CreateDirectory( str, NULL );

					if ( state != 2 )
					{
						state = 2;
						str[ j++ ] = '\\';
					}

					continue;
				}

				str[ j++ ] = path[ i ];
				state      = ( ( path[i] == ':' ) && ( i == 1 ) );
			}

			CreateDirectory( str, NULL );
		}

		static DWORD HexStringToNumber( LPCSTR str )
		{
#define IsHexNumber( c ) ( ( c >= '0'&& c <= '9' ) || ( c >= 'A'&& c <= 'F' ) || ( c >= 'a'&& c <= 'f' ) )
#define GetHexNumber( c ) ( ( c >= '0'&& c <= '9' ) ? ( c - '0' ) : ( 10 + ( ( c >= 'A'&& c <= 'F' ) ? ( c - 'A' ) : ( c - 'a' ) ) ) )

			if ( str == NULL )
				return 0;

			DWORD result = 0;

			if ( str[0] == '0' && str[1] == 'x' )
				str += 2;

			for ( int i = 0; i < 8; i ++ )
			{
				if ( !IsHexNumber( str[i] ) )
					break;

				result = ( result << 4 ) | GetHexNumber( str[i] );
			}

#undef GetHexNumber
#undef IsHexNumber

			return result;
		}
	};

	static CRegion* GetRegionById( DWORD regionId )
	{
		if ( regionId == 0 )
			return NULL;

		BOOL isDynamic = regionId > 0xffff;

		if ( g_Script.m_pPlayer && g_Script.m_pPlayer->m_ParentRegion )
			if ( regionId == ( isDynamic ? g_Script.m_pPlayer->m_ParentRegion->GetGID() : g_Script.m_pPlayer->m_ParentRegion->m_wRegionID ) )
				return g_Script.m_pPlayer->m_ParentRegion;

		if ( g_Script.m_pNpc && g_Script.m_pNpc->m_ParentRegion )
			if ( regionId == ( isDynamic ? g_Script.m_pNpc->m_ParentRegion->GetGID() : g_Script.m_pNpc->m_ParentRegion->m_wRegionID ) )
				return g_Script.m_pNpc->m_ParentRegion;

		if ( g_Script.m_pMonster && g_Script.m_pMonster->m_ParentRegion )
			if ( regionId == ( isDynamic ? g_Script.m_pMonster->m_ParentRegion->GetGID() : g_Script.m_pMonster->m_ParentRegion->m_wRegionID ) )
				return g_Script.m_pMonster->m_ParentRegion;

		return ( CRegion* )( isDynamic ? FindRegionByGID : FindRegionByID )( regionId )->DynamicCast( IID_REGION );
	}

	static int L_CheckItemCount( lua_State *L )
	{
		// 		if ( g_Script.m_pPlayer == NULL )
		// 			return 0;
		// 
		// 		WORD index = static_cast< WORD >( lua_tonumber( L, 1 ) );
		// 		DWORD count = 0;
		// 
		// 		const SItemData *data = CItem::GetItemData( index );
		// 		if ( data == NULL )
		// 			return 0;
		// 
		// 		bool isOverlap = ITEM_IS_OVERLAP( data->byType );
		// 
		// 		for ( std::list< SPackageItem >::iterator it = g_Script.m_pPlayer->m_ItemList.begin(); 
		// 			it != g_Script.m_pPlayer->m_ItemList.end(); it++ )
		// 		{
		// 			if ( it->wIndex == index )
		// 			{
		// 				if ( isOverlap )
		// 				{
		// // 					SOverlap &overlap = reinterpret_cast< SOverlap& >( static_cast< SItemBase& >( *it ) );
		// // 					count += overlap.number; 
		// 				}
		// 				else
		// 				{
		// 					count ++;
		// 				}
		// 			}
		// 		}
		// 
		// 		lua_pushnumber( L, count );

		return 1;
	}

	// 打开（新）江湖排行榜面版
	static int L_OpenScoreList( lua_State *L )
	{
		if ( g_Script.m_pPlayer == NULL )
			return 0;

		SAUpdateScoreListMsg msg;

		lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );

		try
		{
			int number = 0;

			{
				lite::table_iterator titleTable( L, 1 );
				if ( !titleTable.isValid() )
					return 0;

				while ( titleTable ++ )
				{
					// 判断子表的每一个元素都是字符串！
					if ( !lua_isstring( L, titleTable.index_of_value ) )
						return 0;

					number ++;
				}
			}

			if ( number != 0 )
			{
				slm[ /*SQUpdateScoreListMsg::QUERY_TITLE*/ 0 ]( number );

				{
					lite::table_iterator titleTable( L, 1 );
					if ( !titleTable.isValid() )
						return 0;

					while ( titleTable ++ )
					{
						slm( lua_tostring( L, titleTable.index_of_value ) );
					}
				}

				g_StoreMessage( g_Script.m_pPlayer->m_ClientIndex, &msg, 
					( WORD )( sizeof( msg ) - slm.EndEdition() ) );
			}
		}
		catch ( lite::Xcpt & )
		{
			// 只捕获可以判断的错误
		}

		return 0;
	}

	static BOOL traverseWndTable( lua_State *L, lite::Serializer &slm, int wndTableOffset)
	{
#define PUSH_LUATABLE( _index ) {               \
	lite::table_iterator temp( L, _index );     \
	if ( !temp.isValid() ) return FALSE;            \
	table_stack.push( temp );           }

		// 因为涉及到多个表的嵌套，所以使用一个stack来处理！
		std::stack< lite::table_iterator > table_stack;
		PUSH_LUATABLE( 1 + wndTableOffset )

			while ( !table_stack.empty() )
			{
				lite::table_iterator &table = table_stack.top();

				if ( !( table ++ ) )
				{
					// 将结束符入栈！
					slm[ WND_CUSTOM_OP::OP_END ];
					table_stack.pop();
					continue;
				}

				//// 表的每一个元素类型都应该是字符串！
				//if ( lua_type( L, table.index_of_key ) != LUA_TSTRING )
				//    return FALSE;

				// 判断表的每一个元素值类型
				if ( lua_type( L, table.index_of_value ) == LUA_TTABLE )
				{
					// 将控件定义起始符和控件类型入栈！
					slm[ WND_CUSTOM_OP::OP_OCX ]( lua_tostring( L, table.index_of_key ) );

					// 如果值类型是table，则该对象可能是一个控件
					PUSH_LUATABLE( table.index_of_value )
						continue;
				}

				// 将类型和值入栈！
				slm ( ( lite::Variant& )lite::lua_variant( L, table.index_of_key ) )
					( ( lite::Variant& )lite::lua_variant( L, table.index_of_value ) );
			}

#undef  PUSH_LUATABLE

			return TRUE;
	}

	static int L_WndCustomize( lua_State *L )
	{
		return 0;
	}

	static int L_UpdateCustomWnd( lua_State *L )
	{
		return 0;
	}

	static int L_CustomWndClose( lua_State* L )
	{
		return 0;
	}

	static int L_GetRemainPoint( lua_State *L )
	{
		// 		if ( g_Script.m_pPlayer == NULL )
		// 			return 0;
		// 
		//         int type = ( int )lua_tonumber( L, 1 );
		// 
		//         // 检测越界
		//         if ( ( DWORD )type >= 4 )
		//             return 0;
		// 
		//         // 返回对应类型的值
		//         lua_pushnumber( L, g_Script.m_pPlayer->m_Property.checkPoints[ type ] );
		//         return 1;

		return 0;
	}

	static int L_ATM_QueryRemainPoints( lua_State *L )
	{
		BOOL QueryAccountPoints( CPlayer *player );
		QueryAccountPoints( g_Script.m_pPlayer );
		return 0;
	}

	static int L_ATM_DecreasePoint( lua_State *L )
	{
		// 这个函数主要用于：区域服务器向登陆服务器请求扣除指定数量的（元宝、通宝、赠宝、等可能存在的货币类型）！
		// 第一个参数为 货币类型
		// 第二个参数为 扣除的数量（必定为负数，且不会扣出超过1千人民币，即10万元宝）
		// 第三个参数为 写入LOG的，用于查询的，数值型类型值
		// 第四个参数为 写入LOG的，用于查询的，详细说明！！！
		// 第五个参数为 扣除完成后，用于响应结果的回调函数      使用ATM_temporaryScriptVariant临时保存[栈底]
		// 后续的参数为 指定回调函数在调用时的前面的默认参数    使用ATM_temporaryScriptVariant临时保存[栈顶]
		// #####################################################################################
		// 注意：
		// 当回调函数被调用时，其中的第一个参数必定是扣点操作返回结果（成功则为1）！
		if ( g_Script.m_pPlayer == NULL )
			return 0;

		// 先通过这个函数进行判断，里边可能存在超时解锁！
		if ( g_Script.m_pPlayer->isWaitingLockedResult( FALSE, TRUE ) )
			return 0;

		// 此操作会通过变量ATM_temporaryScriptVariant.empty()进行控制
		// 如果上一个操作未完成前，后续的ATM_Decrement则不会被再次触发！！！
		if ( !g_Script.m_pPlayer->ATM_temporaryScriptVariant.empty() )
			return 0;

		int     type    = ( int )lua_tonumber( L, 1 );
		int     number  = ( int )lua_tonumber( L, 2 );
		int     optype  = ( int )lua_tonumber( L, 3 );
		LPCSTR  opinfo  = lua_tostring( L, 4 );
		//记得是传取消了前缀的账号到账号服务器上哟.
		LPCSTR  account = g_Script.m_pPlayer->GetNotPrefixAccount();
		DWORD   staticId = g_Script.m_pPlayer->GetSID();

		// 不能扣除 0或为负数的值（为负数则变成了增加而非扣除！）
		if ( ( number <= 0 ) || ( number > 100000 ) )
			return 0;

		// 容错，避免空指针操作
		if ( opinfo == NULL )
			opinfo = "";

		assert( account != NULL );

		SQGameServerRPCOPMsg msg;

		lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );

		try
		{
			std::stack< lite::Variant > &param_stack = g_Script.m_pPlayer->ATM_temporaryScriptVariant;

			int t = lua_type( L, 5 );
			if ( lua_isstring( L, 5 ) )
			{
				// 如果当前调用存在回调函数时，则开始将临时数据压栈！
				int top = lua_gettop( L );

				do 
				{
					param_stack.push( lite::Variant() );
					if ( !reinterpret_cast< lite::lua_variant & >( param_stack.top() ).get( L, top -- ) )
						return 0;
				}
				while ( top >= 5 );
			}
			else
			{
				// 其余情况都全部忽略，认为是没有回调的货币扣除回调操作
			}

			// 如果不存在回调，那就用不着（也不能）返回操作结果！（否则会和需要回调的操作起冲突！）
			if ( !param_stack.empty() )
			{
				// [OP_BEGIN_ERRHANDLE] ( 0 ) ( "sp_DecreasePoint" ) [OP_END_ERRHANDLE]

				// 注册出现异常时返回的数据（集）
				// 这样的异常返回的数据集可以兼容数据库处理错误为处理失败！
				slm [OP_BEGIN_ERRHANDLE] 
				( (int)SMessage::EPRO_ACCOUNT_MESSAGE )
					( (int)SAccountBaseMsg::EPRO_REFRESH_CARD_POINT )
					( staticId )
					( 0 )
					[OP_END_ERRHANDLE];
			}

			// 准备存储过程（减点）
			slm [OP_PREPARE_STOREDPROC] ( "AccountManager.sp_DecreasePoint" ) 

				// 设定调用参数
				[OP_BEGIN_PARAMS]
			(1) ( account )     // 扣点账号
				(2) ( type )        // 扣点类型
				(3) ( number )      // 扣点数量 值本身为正
				(4) ( optype )      // 操作类型 用于LOG
				(5) ( opinfo )      // 操作说明 用于LOG
				[OP_END_PARAMS]

			// 调用存储过程
			[OP_CALL_STOREDPROC];

			// 如果不存在回调，那就用不着（也不能）返回操作结果！（否则会和需要回调的操作起冲突！）
			if ( !param_stack.empty() )
			{
				// 如果需要返回值用于后续操作，则需要增加一个用于效验结果的数据！
				// 以避免上一笔操作目的和本次操作目的不同，同时又出现了意外而导致的异常
				g_Script.m_pPlayer->ATM_temporaryVerifier = timeGetTime();

				// 初始化返回数据盒
				slm [OP_INIT_RETBOX]    (1024)

					// 数据盒的第一个值
					[OP_BOX_VARIANT]    ( (int)SMessage::EPRO_ACCOUNT_MESSAGE )
					[OP_BOX_VARIANT]    ( (int)SAccountBaseMsg::EPRO_REFRESH_CARD_POINT )
					[OP_BOX_VARIANT]    ( staticId )

					// 返回扣点类型
					[OP_BOX_VARIANT]    ( g_Script.m_pPlayer->ATM_temporaryVerifier )
					[OP_BOX_VARIANT]    ( type )

					// 返回存储过程的返回值
					[OP_BOX_PARAM]      ( 0 )   // 扣点是否成功
					[OP_BOX_PARAM]      ( 6 )   // 扣点后剩余多少点

					// 返回数据盒
					[OP_RETURN_BOX];
			}
			else
			{
				// staticId 本来就是用于回调的一个参数，如果不需要回调，则将其设为-1以便在LOG中分析
				staticId = -1;
			}

			// 结束
			slm [OP_RPC_END];

			// 全都准备好后，再发送消息到登陆服务器！并且由登陆服务器再转到账号服务器
			msg.dstDatabase = SQGameServerRPCOPMsg::ACCOUNT_DATABASE;
			//这里是把账号的前缀加在消息里面发到登录服务器,以便于分别分发到各个账号上处理
			dwt::strcpy( (char*)msg.GetExtendBuffer(), g_Script.m_pPlayer->GetAccountPrefix(), 16 );
			if ( !SendToLoginServer( &msg, ( long )( sizeof( msg ) - slm.EndEdition() ) ) )
				return 0;

			extern dwt::stringkey<char [256]> szDirname;
			TraceInfo( szDirname + "decpoint.log", "[%s]启动扣点操作！[%s][%08X][%d][%d][%d][%s]", 
				GetStringTime(), account, staticId, type, number, optype, opinfo );
		}
		catch ( lite::Xcpt & )
		{
			// 只捕获可以判断的错误
		}

		return 0;
	}

	static int L_CancelWaitingState( lua_State *L )
	{
		if ( g_Script.m_pPlayer == NULL )
			return 0;

		extern dwt::stringkey<char [256]> szDirname;
		TraceInfo( szDirname + "decpoint.log", "[%s]扣点操作已经被取消！[%08X][%d]%s", 
			GetStringTime(), g_Script.m_pPlayer->GetSID(), g_Script.m_pPlayer->m_bWaitBuyResult,
			( ( L == NULL ) ? "[自动解锁]" : "" ) );

		g_Script.m_pPlayer->m_bWaitBuyResult = false;
		g_Script.m_pPlayer->ATM_temporaryVerifier = 0;
		while ( !g_Script.m_pPlayer->ATM_temporaryScriptVariant.empty() ) 
			g_Script.m_pPlayer->ATM_temporaryScriptVariant.pop();

		return 0;
	}

	static int L_ModifyItem( lua_State *L )
	{
		//         int size = 0;
		//         void *data = luaEx_touserdata( L, 1, &size );
		//         LPCSTR option = lua_tostring( L, 2 );
		// 
		//         if ( size != sizeof( SPackageItem ) )
		//             return 0;
		// 
		//         SPackageItem &item = *( SPackageItem* )data;
		// 
		//         BOOL result = FALSE;
		//         switch ( option[0] )
		//         {
		//         case 'X':
		//         case 'D':
		// 		    if ( g_Script.m_pPlayer == NULL )
		// 			    return 0;
		// 
		//             // 删除指定道具
		//             if ( !g_Script.m_pPlayer->CheckItem( item ) )
		//                 return 0;
		// 
		//             result = g_Script.m_pPlayer->DelItem( item, GetNpcScriptInfo( "脚本删除ModifyItem" ), option[0] == 'X' );
		//             break;
		// 
		//         case 'A': // 将指定道具添加到玩家身上（创建唯一ID）
		//         case 'R': // 将指定道具恢复到玩家身上（保留唯一ID）
		//             {
		// // 		        if ( g_Script.m_pPlayer == NULL )
		// // 			        return 0;
		// // 
		// //                 if ( option[0] == 'A' )
		// //                 {
		// //                     // 限制该功能只能配合GenerateItemDetails使用！
		// //                     if ( ( item.byCellX != 255 ) || ( item.byCellY != 255 ) )
		// //                         return 0;
		// //                 }
		// // 
		// //                 SCellPos pos = { -1, -1 };
		// //                 if ( !g_Script.m_pPlayer->CheckAddItem( item.wIndex, pos ) )
		// //                 {
		// //                     g_Script.m_pPlayer->SendErrorMsg( SABackMsg::B_FULLBAG );
		// //                     return 0;
		// //                 }
		// // 
		// //                 if ( option[0] == 'A' )
		// //                 {
		// //                     // 生成新的唯一ID！
		// //                     if ( !GenerateNewUniqueId( item ) )
		// //                         return 0;
		// //                 }
		// // 
		// //                 result = g_Script.m_pPlayer->AddExistingItem( item, pos, true, GetNpcScriptInfo( "脚本添加ModifyItem" ) );
		//             }
		//             break;
		// 
		//         case 'G':
		//             // 将指定道具生成到场景上
		//             {
		// //                 // 限制该功能只能配合GenerateItemDetails使用！
		// //                 if ( ( item.byCellX != 255 ) || ( item.byCellY != 255 ) )
		// //                     return 0;
		// // 
		// //                 // 获取指定场景和坐标
		// //                 if ( !lua_istable( L, 3 ) )
		// //                     return 0;
		// // 
		// //                 #define GET_FIELD_NUMBER_DIRECTLY( _key ) _GET_FIELD_NUMBER_DIRECTLY( 3, _key )
		// // 
		// //                 DWORD regionId = static_cast< DWORD >( GET_FIELD_NUMBER_DIRECTLY( "regionId" ) );
		// //                 if ( regionId == 0 )
		// //                     return rfalse( 2, 1, "(MI)没有指定目标场景" );
		// // 
		// //                 CRegion *destRegion = GetRegionById( regionId );
		// //                 if ( destRegion == NULL )
		// //                     return rfalse( 2, 1, "(MI)找不到指定场景" );
		// // 
		// //                 CItem::SParameter param;
		// //                 memset( &param, 0, sizeof( param ) );
		// // 
		// //                 param.xTile = static_cast< WORD >( GET_FIELD_NUMBER_DIRECTLY( "x" ) );
		// //                 param.yTile = static_cast< WORD >( GET_FIELD_NUMBER_DIRECTLY( "y" ) );
		// //                 if ( ( param.xTile == 0 ) || ( param.yTile == 0 ) )
		// //                     return rfalse( 2, 1, "(MI)未设定道具坐标" );
		// // 
		// //                 param.dwLife = static_cast< DWORD >( GET_FIELD_NUMBER_DIRECTLY( "lifeTime" ) );
		// //                 if ( param.dwLife == 0 ) param.dwLife = 10 * 60 * 1000;         // 处理默认值！
		// //                 else if ( param.dwLife != 0xffffffff ) param.dwLife *= 1000;    // 将道具存活时间提高到秒级设定
		// // 
		// //                 WORD range = static_cast< WORD >( GET_FIELD_NUMBER_DIRECTLY( "range" ) );
		// // 
		// //                 #undef GET_FIELD_NUMBER_DIRECTLY
		// // 
		// //                 param.pItem = &item;
		// //                 if ( range != 0 )
		// //                 {
		// //                     WORD x = ( WORD )( param.xTile - range + ( rand() % ( range * 2 + 1 ) ) );
		// //                     WORD y = ( WORD )( param.yTile - range + ( rand() % ( range * 2 + 1 ) ) );
		// // 
		// //                     if ( destRegion->isTileWalkAble( x, y ) )
		// //                         param.xTile = x, param.yTile = y;
		// //                 }
		// // 
		// //                 // 生成新的唯一ID！
		// //                 if ( !GenerateNewUniqueId( item ) )
		// //                     return 0;
		// // 
		// //                 result = destRegion->Move2Area( 
		// //                     destRegion->GenerateObject( IID_ITEM, 0, (LPARAM)&param ), 
		// //                     param.xTile, param.yTile, true );
		//             }
		//             break;
		// 
		//         case 'J' :
		//             _asm nop
		//             break;
		//         }
		// 
		//         if ( !result )
		// 		    return 0;
		// 
		//         lua_pushnumber( L, 1 );
		//         return 1;
		return 0;
	}

	static int L_SetRandomAbilityTable( lua_State *L )
	{
		// 是否做全服务器更新通知！
		bool fullSynchro = ( lua_toboolean( L, 2 ) == 1 );

		// 新的随机属性表是一个固定格式的数组表格

		lite::table_iterator mainTable( L, 1 );
		if ( !mainTable.isValid() ) 
			return rfalse( 2, 1, "错误的表参数（L_SetRandomAbilityTable）" ), 0;

		RandomAbilityTable tempTable;
		memset( &tempTable, 0, sizeof( tempTable ) );

		while ( mainTable ++ )
		{
			// 主表的每一个表项都应该符合 key[number] = value[subtable]
			if ( ( lua_type( L, mainTable.index_of_key ) != LUA_TNUMBER ) ||
				( lua_type( L, mainTable.index_of_value ) != LUA_TTABLE ) )
				return rfalse( 2, 1, "存在一个非[索引/子表]的错误参数（L_SetRandomAbilityTable）" ), 0;

			DWORD index = ( DWORD )lua_tonumber( L, mainTable.index_of_key );
			if ( index >= 32 )
				return rfalse( 2, 1, "子表的索引超过31！（L_SetRandomAbilityTable）" ), 0;

			RandomAbilityTable::Unit &unit = tempTable.units[ index ];

			lite::table_iterator subTable( L, mainTable.index_of_value );
			if ( !subTable.isValid() )
				return rfalse( 2, 1, "无效的子表（L_SetRandomAbilityTable）" ), 0;

			while ( subTable ++ )
			{
				// 注意：这里没有用lua_isnumber/isstring...之类的函数，是因为如果当key是number时，这两个函数都可以通过……
				if ( lua_type( L, subTable.index_of_key ) == LUA_TSTRING )
				{
					std::string keyValue = lua_tostring( L, subTable.index_of_key );
					if ( ( keyValue == "name" ) && ( lua_type( L, subTable.index_of_value ) == LUA_TSTRING ) )
					{
						std::string randomType = lua_tostring( L, subTable.index_of_value );

						size_t index = 0;
						for ( const LPCSTR *it = &RandomAbility::RANDOM_TYPE_STRING[0]; 
							it != &RandomAbility::RANDOM_TYPE_STRING[ 
								sizeof( RandomAbility::RANDOM_TYPE_STRING ) / 
									sizeof( RandomAbility::RANDOM_TYPE_STRING[0] ) ]; it ++ )
								{
									if ( randomType == *it )
									{
										index = ( it - &RandomAbility::RANDOM_TYPE_STRING[0] );
										break;
									}
								}

								if ( ( index <= RandomAbilityTable::TYPE_NULL ) && ( index >= RandomAbilityTable::TYPE_MAX ) )
									return rfalse( 2, 1, "错误的属性选项[%s]（L_SetRandomAbilityTable）", randomType.c_str() ), 0;

								unit.randomType = ( RandomAbilityTable::RANDOM_TYPE )index;
					}
					else if ( lua_type( L, subTable.index_of_value ) == LUA_TNUMBER )
					{
						if ( keyValue == "rate" ) 
						{
							double rating = lua_tonumber( L, subTable.index_of_value );
							if ( ( rating > 1 ) || ( rating < 0 ) )
								return rfalse( 2, 1, "随机几率需要在0~1之间！（L_SetRandomAbilityTable）[%d]", index ), 0;

							unit.base_default.rating = unit.hitRating = ( DWORD )( rating * 100000000 );
						}
						else if ( keyValue == "default" ) 
						{
							unit.base_default.value = ( float )lua_tonumber( L, subTable.index_of_value );
						}
					}
				}
				else if ( ( lua_type( L, subTable.index_of_key ) == LUA_TNUMBER ) && 
					( lua_type( L, subTable.index_of_value ) == LUA_TTABLE ) )
				{
					DWORD slotIndex = ( DWORD )lua_tonumber( L, subTable.index_of_key );
					if ( slotIndex >= 8 )
						return rfalse( 2, 1, "变化段数超过8阶（L_SetRandomAbilityTable）" ), 0;

					lua_rawgeti( L, -1, 1 );
					lua_rawgeti( L, -2, 2 );
					unit.array[ slotIndex ].rating = ( DWORD )( lua_tonumber( L, -2 ) * 100000000 );
					unit.array[ slotIndex ].value  = ( float )( lua_tonumber( L, -1 ) );
					lua_pop( L, 2 );

					if ( ( unit.array[ slotIndex ].rating == 0 ) || ( unit.array[ slotIndex ].value == 0 ) || 
						( unit.array[ slotIndex ].rating > 100000000 ) )
					{
						return rfalse( 2, 1, 
							"无效的子表设定（L_SetRandomAbilityTable）"
							"[%d] = { [%d] = { %.7f, %.3f } }", index, slotIndex, 
							( double )unit.array[ slotIndex ].rating / 100000000, 
							unit.array[ slotIndex ].value ), 0;
					}
				}
				else
					return rfalse( 2, 1, "发现一个无效的表项（L_SetRandomAbilityTable）！" ), 0;
			}

			// newly 现在不用排序了，读取方式已经做了修改！
			// 当子表设置完毕后，需要根据value重新调整一下array中数据的排序顺序（如果不排序的话，很可能是乱的！）
			//for ( int i = 0; ( ( i < 7 ) && ( unit.array[ i ].value != 0 ) ); ++i )
			//{
			//    for ( int j = 0; ( ( j < 7 ) && ( unit.array[ j + 1 ].value != 0 ) ); ++j )
			//    {
			//        if ( unit.array[ j ].value > unit.array[ j + 1 ].value )
			//        {
			//            QWORD tempValue = reinterpret_cast< QWORD& >( unit.array[ j + 1 ] );
			//            reinterpret_cast< QWORD& >( unit.array[ j + 1 ] ) = 
			//                reinterpret_cast< QWORD& >( unit.array[ j ] );
			//            reinterpret_cast< QWORD& >( unit.array[ j ] ) = tempValue;
			//        }
			//    }
			//}
		}

		tempTable.validate = true;
		RandomAbilityTable::RATable = tempTable;

		if ( !fullSynchro )
			rfalse( 2, 1, "随机属性表设置成功（L_SetRandomAbilityTable）" );
		else
		{
			AnswerUpdateItemDataMsg msg;
			msg.index = QueryUpdateItemDataMsg::RANDMONTABLE;

			try
			{
				lite::Serializer slm( msg.streamBuffer, sizeof( msg.streamBuffer ) );
				slm( lite::Variant( &RandomAbilityTable::RATable, sizeof( RandomAbilityTable::RATable ) ) );
				BroadcastMsg( &msg, ( WORD )( sizeof( msg ) - slm.EndEdition() ) );
				rfalse( 2, 1, "随机属性表设置成功（L_SetRandomAbilityTable），并且广播了新的配置数据" );
			}
			catch ( lite::Xcpt & )
			{
			}
		}

		return 0;
	}

	// 通过该脚本函数，返回一个表，该表中存在的数据为道具配置表中的数据！
	static int L_GetItemSetting( lua_State *L )
	{
		// zeb 2009.11.19
		//         DWORD index = ( DWORD )lua_tonumber( L, 1 );
		//         const SItemData *itemData = CItem::GetItemData( index );
		//         if ( index == 0 || itemData == NULL )
		//             return 0;
		// 
		//         // 用于保持堆栈平衡！
		//         int stackPos = lua_gettop( L );
		// 
		//         lua_createtable( L, 0, 3 );
		//         if ( !lua_istable( L, -1 ) )
		//             goto _failure;
		// 
		// #define SETTABLE_BY_STRING( _key_of_string, _value_of_number ) \
		//         lua_pushstring( L, _key_of_string ); lua_pushnumber( L, _value_of_number ); lua_settable( L, -3 );
		// 
		// #define SETTABLE_OF_STRING( _key_of_string, _value_of_string ) \
		//         lua_pushstring( L, _key_of_string ); lua_pushstring( L, _value_of_string ); lua_settable( L, -3 );
		// 
		//         SETTABLE_BY_STRING( "type", itemData->byType );
		// 
		// 		//add by ab_yue
		// 		SETTABLE_BY_STRING( "iconID", itemData->wInventoryBigIconID );
		// //MODIFY BY AB_YUE
		// //         SETTABLE_BY_STRING( "mainlevel", itemData->nMainLevel );
		// //         SETTABLE_BY_STRING( "sublevel", itemData->wSubLevel );
		// //         SETTABLE_BY_STRING( "stone", itemData->stone );
		// //END
		// 
		//         if ( ITEM_IS_OVERLAP( itemData->byType ) )
		//         {
		// 			//MODIFY BY AB_YUE
		//             //SETTABLE_BY_STRING( "maximun", ( ISSPECIALOVERLAP( itemData->byType ) ?  itemData->wSpecial : 9 ) );
		//         }
		//         else
		//         {
		//             SETTABLE_BY_STRING( "maximun", 1 );
		// 			/// 装备需要更多的数据
		// 			if( ITEM_IS_EQUIPMENT( itemData->byType ) )
		// 			{
		// 				SETTABLE_BY_STRING( "School", itemData->byGroupEM );
		// 			}
		// 		}
		// 
		// 
		//         // scope of object
		//         {
		//             // 安全第一，安全第一！使用std::string是为了将数据截断，避免overrun
		//             std::string name( itemData->szName, dwt::strlen( itemData->szName, sizeof( itemData->szName ) ) );
		//             SETTABLE_OF_STRING( "name", name.c_str() );
		//         }
		// 
		// #undef SETTABLE_OF_STRING
		// #undef SETTABLE_BY_STRING
		// 
		//         assert( stackPos + 1 == lua_gettop( L ) );
		//         return 1;
		// 
		// _failure:
		//         assert( stackPos < lua_gettop( L ) );
		//         if ( stackPos < lua_gettop( L ) )
		//             lua_settop( L, stackPos );
		// zeb 2009.11.19
		return 0;
	}


	// 通过该脚本函数，返回一个表，该表中存在的数据为根据道具类型分解出来的所有数据！
	static int L_GetItemDetails( lua_State *L )
	{
		// zeb 2009.11.19
		//         int size = 0;
		//         void *data = luaEx_touserdata( L, 1, &size );
		//         bool doCheck = ( lua_toboolean( L, 2 ) == 0 );
		//         bool miscOnly = ( lua_toboolean( L, 3 ) == 1 );
		// 
		//         if ( size != sizeof( SPackageItem ) )
		//             return 0;
		// 
		//         SPackageItem &item = *( SPackageItem* )data;
		//         if ( doCheck && ( ( g_Script.m_pPlayer == NULL ) || !g_Script.m_pPlayer->CheckItem( item ) ) )
		//             return 0;
		// 
		//         const SItemData *itemData = CItem::GetItemData( item.wIndex );
		//         if ( itemData == NULL )
		//             return 0;
		// 
		//         if ( miscOnly )
		//         {
		//             // 只返回 misc首部数据，用于间接构造道具的更新表
		//             // lua_pushnumber( L, reinterpret_cast< double& >( static_cast< SItemBase& >( item ) ) );
		//             char buf[256];
		//             sprintf( buf, "%I64X", ( QWORD )( ( reinterpret_cast< QWORD& >( static_cast< SItemBase& >( item ) ) >> 24 ) & 0x000000ffffffffff ) );
		//             lua_pushstring( L, buf );
		//             return 1;
		//         }
		// 
		//         // 用于保持堆栈平衡！
		//         int stackPos = lua_gettop( L );
		// 
		//         // 根据道具类型判断需要创建一个多大的luaTable
		//         // ##############################################
		//         // 注意！该项数和道具结构并不完全对应！！！
		//         // ##############################################
		// 
		//         // 初始表项4 [index,type,flags,misc] 道具编号、道具类型、标志位、杂项数据
		//         // 其中flags为可接受变化的数据，index,type为仅用于查询的数据，misc为8字节的itemBase首部镜像数据！
		//         int nElements = 5;//4改为5，添加一项道具等级需求 jym
		// 
		//         if ( ITEM_IS_OVERLAP( itemData->byType ) )
		//             // 如果为可重叠类，增加2项数据为[number,maximum] 当前重叠数量、最大重叠数量
		//             // 其中number为可接受变化的数据，maximum仅用于查询
		//             nElements += 2;
		//         else if ( ITEM_IS_EQUIPMENT( itemData->byType ) )
		//             // 如果为装备类，增加7项数据为[prefix,level,experience,durance,slots,attributes,scripts] 
		//             // 前缀，等级，经验，耐久度，镶嵌插槽[5]，随机属性[5]，脚本变量[4]：（所有的数据均可接受变化）
		//             nElements += 7;
		// 
		//         lua_createtable( L, 0, nElements );
		//         if ( !lua_istable( L, -1 ) )
		//             goto _failure;
		// 
		// #define SETTABLE_BY_STRING( _key_of_string, _value_of_number ) \
		//         lua_pushstring( L, _key_of_string ); lua_pushnumber( L, _value_of_number ); lua_settable( L, -3 );
		// 
		//         SETTABLE_BY_STRING( "index", ( DWORD )item.wIndex );
		//         SETTABLE_BY_STRING( "type", itemData->byType );
		//         SETTABLE_BY_STRING( "flags", ( DWORD )item.flags );
		// 
		//         //SETTABLE_BY_STRING( "misc", reinterpret_cast< double& >( static_cast< SItemBase& >( item ) ) );
		//         // 注意，这个地方是一个特殊处理：
		//         // 由于lua不支持64位整数，所以只能采取其他途径来实现
		//         // 我们现在是用大整数不是为了用于计算，而是用于实现标识符
		//         // 当前实现的方式是使用 lightuserdata 来记录63bit的数据，最高位用来标记该用户数据是63bit类型！！！
		//         lua_pushstring( L, "misc" ); 
		//         luaEx_pushint63( L, item.uniqueId() );
		//         lua_settable( L, -3 );
		// 
		// 		SETTABLE_BY_STRING( "levelEM", itemData->byLevelEM );
		// 
		//         if ( ITEM_IS_OVERLAP( itemData->byType ) )
		//         {
		//             SETTABLE_BY_STRING( "number", reinterpret_cast< SOverlap& >( 
		//                 static_cast< SItemBase& >( item ) ).number  );
		// 
		//             //SETTABLE_BY_STRING( "maximun", ( ISSPECIALOVERLAP( itemData->byType ) ?  itemData->wSpecial : 9 ) );
		//         }
		//         else if ( ITEM_IS_EQUIPMENT( itemData->byType ) )
		//         {
		// #define ITEM_CONVERT reinterpret_cast< SEquipment& >( static_cast< SItemBase& >( item ) )
		// 
		//             //SETTABLE_BY_STRING( "prefix",       ITEM_CONVERT.prefix );
		//             ////SETTABLE_BY_STRING( "level",        ITEM_CONVERT.level );
		//             //SETTABLE_BY_STRING( "experience",   ITEM_CONVERT.experience );
		//             //SETTABLE_BY_STRING( "cur_durance",  ITEM_CONVERT.durance.cur );
		//             //SETTABLE_BY_STRING( "max_durance",  ITEM_CONVERT.durance.max );
		//             SETTABLE_BY_STRING( "color",        ( ITEM_CONVERT.reserve & 0x3 ) );
		// 
		//             SETTABLE_BY_STRING( "isDevelop",        ( ITEM_CONVERT.isDevelop & 0x1 ) );
		//             SETTABLE_BY_STRING( "isRefine",         ( ITEM_CONVERT.isRefine & 0x1 ) );
		//             SETTABLE_BY_STRING( "developCount",     ( ITEM_CONVERT.developCount & 0x3ff ) );
		//             SETTABLE_BY_STRING( "level",     ( ITEM_CONVERT.level & 0xf ) );
		//             // 为 slots、attributes、scripts 创建子表
		//             lua_pushstring( L, "slots" );
		//             lua_createtable( L, 5, 0 );
		//             if ( !lua_istable( L, -1 ) )
		//                 goto _failure;
		// 
		//             for ( int i = 0; i < SEquipment::MAX_SLOTS; ++i )
		//             {
		//                 lua_pushnumber( L, ITEM_CONVERT.slots[i].type * 100 + ITEM_CONVERT.slots[i].value );
		//                 lua_rawseti( L, -2, i + 1 );
		//             }
		// 
		//             lua_settable( L, -3 );
		// 
		//             lua_pushstring( L, "activates" );
		//             lua_createtable( L, 3, 0 );
		//             if ( !lua_istable( L, -1 ) )
		//                 goto _failure;
		// 
		//             for ( int i = 0; i < SEquipment::MAX_ACTIVATES; ++i )
		//             {
		//                 lua_pushnumber( L, ITEM_CONVERT.attribute.activates[i].type * 10000 +  ITEM_CONVERT.attribute.activates[i].value );
		//                 lua_rawseti( L, -2, i + 1 );
		//             }
		//             lua_settable( L, -3 );
		// 
		//             SETTABLE_BY_STRING( "naturals", ( ITEM_CONVERT.attribute.naturals.type * 10000 +  ITEM_CONVERT.attribute.naturals.value ) );
		// 
		//             lua_pushstring( L, "qualitys" );
		//             lua_createtable( L, 6, 0 );
		//             if ( !lua_istable( L, -1 ) )
		//                 goto _failure;
		//             for ( int i = 0; i < SEquipment::MAX_QUALITYS; ++i )
		//             {
		//                 lua_pushnumber( L, ITEM_CONVERT.attribute.qualitys[i].type * 10000 +  ITEM_CONVERT.attribute.qualitys[i].value );
		//                 lua_rawseti( L, -2, i + 1 );
		//             }
		//             lua_settable( L, -3 );
		// 
		//             lua_pushstring( L, "inherits" );
		//             lua_createtable( L, 6, 0 );
		//             if ( !lua_istable( L, -1 ) )
		//                 goto _failure;
		// 
		//             for ( int i = 0; i < SEquipment::MAX_INHERITS; ++i )
		//             {
		//                 lua_pushnumber( L, ITEM_CONVERT.attribute.inherits[i].type * 10000 +  ITEM_CONVERT.attribute.inherits[i].value );
		//                 lua_rawseti( L, -2, i + 1 );
		//             }
		//             lua_settable( L, -3 );
		// 
		//             // 这里是两个特殊
		//             // 武功特殊处理
		//             SETTABLE_BY_STRING( "addskill", ( ITEM_CONVERT.attribute.naturals.type * 1000 +  ITEM_CONVERT.attribute.naturals.value ) );
		// 
		//             SETTABLE_BY_STRING( "addteletgy", ( ITEM_CONVERT.attribute.naturals.type * 1000 +  ITEM_CONVERT.attribute.naturals.value ) );
		// 
		//             //extern DWORD g_dwEquipmentWearScale;
		//             SETTABLE_BY_STRING( "curwear", ITEM_CONVERT.attribute.currWear )
		//             //SETTABLE_BY_STRING( "maxwear", ITEM_CONVERT.attribute.currWear )
		// 
		//             //for ( int i = 0; i < SEquipment::MAX_ATTRIBUTES; ++i )
		//             //{
		//             //    lua_pushnumber( L, ITEM_CONVERT.attributes[i].type * 100 + 
		//             //        ITEM_CONVERT.attributes[i].value );
		//             //    lua_rawseti( L, -2, i + 1 );
		//             //}
		// 
		//             lua_pushstring( L, "scripts" );
		//             lua_createtable( L, 3, 0 );
		//             if ( !lua_istable( L, -1 ) )
		//                 goto _failure;
		// 
		//             for ( int i = 0; i < SEquipment::MAX_SCRIPTS; ++i )
		//             {
		//                 lua_pushnumber( L, ITEM_CONVERT.scriptDatas[i] );
		//                 lua_rawseti( L, -2, i + 1 );
		//             }
		// 
		//             lua_settable( L, -3 );
		// 
		// #undef ITEM_CONVERT
		//         }
		//         else if ( ITEM_IS_SCRIPT( itemData->byType ) )
		//         {
		//             // 如果是脚本类道具，独立一个script表来保存所有脚本数据！
		//             lua_pushstring( L, "scripts" );
		//             lua_createtable( L, 8, 0 );
		//             if ( !lua_istable( L, -1 ) )
		//                 goto _failure;
		// 
		//             try
		//             {
		//                 SScriptItem &scriptItem = reinterpret_cast< SScriptItem& >( static_cast< SItemBase& >( item ) );
		//                 if ( scriptItem.declare )
		//                 {
		//                     lite::Serialreader slr( scriptItem.streamData );
		//                     for ( int i = 0; i < 8; i ++ )
		//                     {
		//                         if ( scriptItem.declare & ( 1 << i ) )
		//                         {
		//                             reinterpret_cast< lite::lua_variant& >( slr() ).push( L );
		//                             lua_rawseti( L, -2, i + 1 );
		//                         }
		//                     }
		//                 }
		//             }
		//             catch ( lite::Xcpt& )
		//             {
		//                 goto _failure;
		//             }
		// 
		//             lua_settable( L, -3 );
		//         }
		// 
		// #undef SETTABLE_BY_STRING
		// 
		//         // 虽然创建了这么多数据，但只返回了一个表而已！
		//         assert( stackPos + 1 == lua_gettop( L ) );
		//         return 1;
		// 
		// _failure:
		//         assert( stackPos < lua_gettop( L ) );
		//         if ( stackPos < lua_gettop( L ) )
		//             lua_settop( L, stackPos );
		// zeb 2009.11.19
		return 0;
	}


	// 通过该函数，根据传入的配置表修改（更新）道具的数据
	// 如果该道具是玩家身上的道具，则同步通知客户端更新数据！
	static int L_UpdateItemDetails( lua_State *L )
	{
		// zeb 2009.11.19
		//         int size = 0;
		//         void *data = luaEx_touserdata( L, 1, &size );
		//         bool doCheck = ( lua_toboolean( L, 3 ) == 0 );
		//         bool acceptIndexChange = ( lua_toboolean( L, 4 ) == 1 ); // 是否接受index的改变！
		// 
		//         if ( size != sizeof( SPackageItem ) )
		//             return 0;
		// 
		//         if ( !lua_istable( L, 2 ) )
		//             return 0;
		// 
		//         SPackageItem &item = *( SPackageItem* )data;
		//         SPackageItem tempItem;
		//         if ( doCheck )
		//         {
		//             if ( ( g_Script.m_pPlayer == NULL ) || !g_Script.m_pPlayer->CheckItem( item ) )
		//                 return 0;
		// 
		//             tempItem = item;
		//         }
		// 
		//         const SItemData *itemData = CItem::GetItemData( item.wIndex );
		//         if ( itemData == NULL )
		//             return 0;
		// 
		//         // 用于支撑 GET_FIELD_NUMBER
		//         double _result = 0;
		//         bool _isnul = false;
		// 
		// #define IF_GET_FIELD_NUMBER( _key ) ( ( lua_getfield( L, 2, _key ),         \
		//             ( lua_isnil( L, -1 )                                            \
		//             ? ( _isnul = true, _result = 0 )                                \
		//             : ( _isnul = false, _result = lua_tonumber( L, -1 ) ) ),        \
		//             lua_pop( L, 1 ) ) );    if ( !_isnul )
		// 
		// #define GET_FIELD_NUMBER_DIRECTLY( _key ) ( ( lua_getfield( L, 2, _key ),   \
		//             ( lua_isnil( L, -1 )                                            \
		//             ? ( _isnul = true, _result = 0 )                                \
		//             : ( _isnul = false, _result = lua_tonumber( L, -1 ) ) ),        \
		//             lua_pop( L, 1 ) ), _result )
		// 
		// #define GET_ARRAY_NUMBER_DIRECTLY( _index ) ( ( lua_rawgeti( L, -1, ( _index ) ),   \
		//             ( _result = lua_tonumber( L, -1 ) ), lua_pop( L, 1 ) ), _result )
		// 
		//         // 获取2个数据,misc和flags,如果misc被修改的话，就不能继续往下执行了！
		//         lua_getfield( L, 2, "misc" );
		//         QWORD misc = luaEx_toint63( L, -1 );
		//         lua_pop( L, 1 );
		// 
		//         if ( /*reinterpret_cast< QWORD& >( GET_FIELD_NUMBER_DIRECTLY( "misc" ) ) != 
		//             reinterpret_cast< QWORD& >( static_cast< SItemBase& >( item ) ) ) */
		//             misc != item.uniqueId() )
		//             return 0;
		// 
		//         // 如果标志位设定为：可以接受道具index的修改！
		//         if ( acceptIndexChange )
		//         {
		//             // 在获取道具编号后要统一判断：变化前后的道具应该类型相符合
		//             IF_GET_FIELD_NUMBER( "index" )
		//             {
		//                 const SItemData *newData = CItem::GetItemData( ( WORD )_result ); 
		//                 if ( newData == NULL )
		//                     return 0;
		// 
		// #define COMPARE( _t ) ( _t( newData->byType ) && _t( itemData->byType ) )
		// #define COMPARE_IDX( _t ) ( _t( newData->wItemID ) && _t( itemData->wItemID ) )
		// #define MULTI_COMPARE  ( COMPARE( ITEM_IS_EQUIPMENT ) /*|| COMPARE( ISNORMAL ) */|| COMPARE( ITEM_IS_OVERLAP ) /*|| COMPARE( ISSPECIALOVERLAP )*//* || COMPARE_IDX( ITEM_IS_JEWEL ) */)
		// 
		//                 // 先判断新旧道具的类型，如果相同，就不需要作后边的判断了
		//                 if ( ( itemData->byType != newData->byType )/* && !MULTI_COMPARE*/ )
		//                     return 0;
		// 
		// #undef MULTI_COMPARE
		// #undef COMPARE
		// 
		//                 item.wIndex = ( WORD )_result;
		//             }
		//             else
		//             {
		//                 // 如果指定了修改index但是又没有找到index的数据，则失败返回！
		//                 return 0;
		//             }
		//         }
		// 
		//         // 更新标志位！
		//         IF_GET_FIELD_NUMBER( "flags" )  item.flags = ( DWORD )_result;
		// 
		//         if ( ITEM_IS_OVERLAP( itemData->byType ) )
		//         {
		// #define ITEM_CONVERT reinterpret_cast< SOverlap& >( static_cast< SItemBase& >( item ) )
		// 
		//             // 如果为可重叠类，需要更新number
		//             IF_GET_FIELD_NUMBER( "number" ) 
		//             {
		//                 ITEM_CONVERT.number = ( DWORD )_result;
		// 
		//                 // 重叠数量不能为0
		//                 if ( ITEM_CONVERT.number == 0 )
		//                     return 0;
		//             }
		// #undef ITEM_CONVERT
		//         }
		//         else if ( ITEM_IS_EQUIPMENT( itemData->byType ) )
		//         {
		// #define ITEM_CONVERT reinterpret_cast< SEquipment& >( static_cast< SItemBase& >( item ) )
		// 
		//             // 如果为装备类需要更新的东西就多咯
		//             // 前缀，等级，经验，耐久度
		//             //IF_GET_FIELD_NUMBER( "prefix" )         ITEM_CONVERT.prefix         = ( BYTE )_result;
		//             //IF_GET_FIELD_NUMBER( "level" )          ITEM_CONVERT.level          = ( BYTE )_result;
		//             //IF_GET_FIELD_NUMBER( "experience" )     ITEM_CONVERT.experience     = ( WORD )_result;
		//             //IF_GET_FIELD_NUMBER( "cur_durance" )    ITEM_CONVERT.durance.cur    = ( WORD )_result;
		//             //IF_GET_FIELD_NUMBER( "max_durance" )    ITEM_CONVERT.durance.max    = ( BYTE )_result;
		//             IF_GET_FIELD_NUMBER( "color" )          ITEM_CONVERT.reserve        = ( ITEM_CONVERT.reserve & 0xfc ) | ( ( BYTE )_result & 0x3 );
		// 
		//             // 镶嵌插槽[5]，随机属性[5]，脚本变量[4]
		//             lua_getfield( L, 2, "slots" );
		//             if ( lua_istable( L, -1 ) ) for ( int i = 0; i < SEquipment::MAX_SLOTS; ++i )
		//             {
		//                 // 如果前面的插槽无效，则后面的数据都不用填了！
		//                 if ( ( i != 0 ) && ITEM_CONVERT.slots[i-1].isInvalid() )
		//                 {
		//                     *( LPBYTE )&ITEM_CONVERT.slots[i] = 0;
		//                     continue;
		//                 }
		// 
		//                 ITEM_CONVERT.slots[i].type = ( BYTE )( ( DWORD )GET_ARRAY_NUMBER_DIRECTLY( i + 1 ) / 100 );
		//                 ITEM_CONVERT.slots[i].value = ( BYTE )( ( DWORD )_result % 100 );
		// 
		//                 // 同上，如果前面的插槽为空，则后面的数据也都不能为镶嵌状态！
		//                 if ( ( i != 0 ) && ITEM_CONVERT.slots[i-1].isEmpty() && ITEM_CONVERT.slots[i].isJewel() )
		//                     *( LPBYTE )&ITEM_CONVERT.slots[i] = 0;
		//             }
		//             lua_pop( L, 1 );
		// 
		//             IF_GET_FIELD_NUMBER( "isDevelop" )  ITEM_CONVERT.isDevelop = ( BYTE )( _result ) & 0x1;
		//             IF_GET_FIELD_NUMBER( "isRefine" )  ITEM_CONVERT.isRefine = ( BYTE )( _result ) & 0x1;
		//             IF_GET_FIELD_NUMBER( "developCount" )  ITEM_CONVERT.developCount = ( WORD )( _result ) & 0x3ff;
		//             IF_GET_FIELD_NUMBER( "level" )  ITEM_CONVERT.level = ( BYTE )( _result ) & 0xf;
		// 
		//             // 属性设置规则说明:
		//             // 1000 千位数为具体值
		//             // 10000 万位数为 集体类型
		//             // 激活值
		//             SEquipment::Attribute::Unit * att = NULL;
		//             lua_getfield( L, 2, "activates" );
		//             if ( lua_istable( L, -1 ) )
		//             {
		//                 att = ITEM_CONVERT.attribute.activates;
		//                 for ( int i = 0; i < SEquipment::MAX_ACTIVATES; i++ )
		//                     AttUpdate( att[i], ( DWORD )GET_ARRAY_NUMBER_DIRECTLY( i + 1 ) );
		//             }
		// 	    /*
		//             else
		//             {
		//                 SpecialItem* sItem = CItem::GetSpecialItem( itemData->wItemID );
		//                 if ( sItem )
		//                 {
		//                     for( int i = 0; i < SEquipment::MAX_ACTIVATES; i ++ )
		//                         GetAddAttribute( &(ITEM_CONVERT.attribute.activates[i]), sItem->mIntensifyAdd[i], 1 );
		//                 }       
		//             }
		// 	    */
		//             lua_pop( L, 1 );
		// 
		//             IF_GET_FIELD_NUMBER( "naturals" )
		//             {
		//                 att = &ITEM_CONVERT.attribute.naturals;
		//                 AttUpdate( att[0], ( DWORD )_result );
		//             }
		// 
		//             lua_getfield( L, 2, "qualitys" );
		//             if ( lua_istable( L, -1 ) )
		//             {
		//                 att = ITEM_CONVERT.attribute.qualitys;
		//                 for ( int i = 0; i < SEquipment::MAX_QUALITYS; i++ )
		//                     AttUpdate( att[i], ( DWORD )GET_ARRAY_NUMBER_DIRECTLY( i + 1 ) );
		//             }
		//             lua_pop( L, 1 );
		// 
		//             lua_getfield( L, 2, "inherits" );
		//             if ( lua_istable( L, -1 ) )
		//             {
		//                 att = ITEM_CONVERT.attribute.inherits;
		//                 for ( int i = 0; i < SEquipment::MAX_INHERITS; i++ )
		//                     AttUpdate( att[i], ( DWORD )GET_ARRAY_NUMBER_DIRECTLY( i + 1 ) );
		//             }
		//             lua_pop( L, 1 );
		// 
		//             // 这里是两个特殊
		//             // 武功特殊处理
		//             IF_GET_FIELD_NUMBER( "addskill" )
		//             {
		//                 int temp = ( int )_result;
		//                 ITEM_CONVERT.attribute.unitSkill.skillType = abs( (int)( temp / 1000 ) );
		//                 ITEM_CONVERT.attribute.unitSkill.skillValue = temp % 1000;
		//             }
		// 
		//             // 心法特殊处理
		//             IF_GET_FIELD_NUMBER( "addteletgy" )
		//             {
		//                 int temp = ( int )_result;
		//                 ITEM_CONVERT.attribute.unitSkill.teletgyType = abs( (int)( temp / 1000 ) );
		//                 ITEM_CONVERT.attribute.unitSkill.teletgyValue = temp % 1000;
		//             }
		// 
		//            /* extern DWORD g_dwEquipmentWearScale;
		//             if ( g_dwEquipmentWearScale == 0 )
		//                 g_dwEquipmentWearScale = 1000;*/
		// 
		//             if ( itemData->wMaxWear /*itemData->wWearPrime*/ != 0xffff )  // 有耐久才能设置耐久度
		//             {
		// 				//modify by ab_yue
		//                 //IF_GET_FIELD_NUMBER( "curwear" ) ITEM_CONVERT.attribute.curWear = ( WORD )_result;
		//                 //IF_GET_FIELD_NUMBER( "maxwear" ) ITEM_CONVERT.attribute.maxWear = ( WORD )_result;
		// 
		//                 //if ( ITEM_CONVERT.attribute.maxWear  != 0xffff && ITEM_CONVERT.attribute.maxWear > itemData->wWearPrime )
		//                 //    ITEM_CONVERT.attribute.maxWear = itemData->wWearPrime;
		//                 //if ( ITEM_CONVERT.attribute.curWear > ITEM_CONVERT.attribute.maxWear )
		//                 //    ITEM_CONVERT.attribute.curWear = ITEM_CONVERT.attribute.maxWear;
		//             }
		// 
		//             //lua_getfield( L, 2, "attributes" );
		//             //if ( lua_istable( L, -1 ) ) for ( int i = 0; i < SEquipment::MAX_ATTRIBUTES; ++i )
		//             //{
		//             //    // 如果前面的随机属性无效，则后面的数据都不用填了！
		//             //    if ( ( i != 0 ) && ITEM_CONVERT.attributes[i-1].isInvalid() )
		//             //    {
		//             //        *( LPBYTE )&ITEM_CONVERT.attributes[i] = 0;
		//             //        continue;
		//             //    }
		// 
		//             //    ITEM_CONVERT.attributes[i].type = ( BYTE )( ( DWORD )GET_ARRAY_NUMBER_DIRECTLY( i + 1 ) / 100 );
		//             //    ITEM_CONVERT.attributes[i].value = ( BYTE )( ( DWORD )_result % 100 );
		//             //}
		//             //lua_pop( L, 1 );
		// 
		//             lua_getfield( L, 2, "scripts" );
		//             if ( lua_istable( L, -1 ) ) for ( int i = 0; i < SEquipment::MAX_SCRIPTS; ++i )
		//                 ITEM_CONVERT.scriptDatas[i] = ( WORD )GET_ARRAY_NUMBER_DIRECTLY( i + 1 );
		//             lua_pop( L, 1 );
		// 
		// #undef ITEM_CONVERT
		//         }
		//         else if ( ITEM_IS_SCRIPT( itemData->byType ) )
		//         { 
		//             // 如果是脚本类道具，独立一个script表来保存所有脚本数据！
		//             lua_getfield( L, 2, "scripts" );
		//             if ( !lua_istable( L, -1 ) )    // 如果没有script子表，则更新失败！
		//                 return lua_pop( L, 1 ), 0;
		// 
		//             size_t size = 0;
		//             SScriptItem &scriptItem = reinterpret_cast< SScriptItem& >( static_cast< SItemBase& >( item ) );
		//             try 
		//             {
		//                 ZeroMemory( item.buffer, sizeof( item.buffer ) );
		//                 lite::Serializer slm( scriptItem.streamData, sizeof( scriptItem.streamData ) );
		// 
		//                 // 遍历8项数据来确定每一个元素的有效性 1 2 4 8 16 32 64 128
		//                 for ( int i = 0; i < 8; i ++ )
		//                 {
		//                     lua_rawgeti( L, -1, i + 1 );
		//                     if ( !lua_isnil( L, -1 ) )  // 有效的数据才保存进数据流
		//                     {
		//                         scriptItem.declare |= ( 1 << i );
		//                         slm( lite::lua_variant( L, -1 ) );
		//                     }
		//                     else
		//                         scriptItem.declare &= ( ~( 1 << i ) );
		//                     lua_pop( L, 1 );
		//                 }
		// 
		//                 // 平安结束标志!
		//                 slm.EndEdition();
		//                 size = slm.maxSize();
		//             }
		//             catch ( lite::Xcpt & )
		//             {
		//                 // 如果出现错误，当然就更新失败了！
		//                 // 需要把出现异常时的那个脚本数据出栈! 以保持栈平衡
		//                 lua_pop( L, 1 );
		//             }
		// 
		//             // 把script出栈
		//             lua_pop( L, 1 );
		// 
		//             // 中途出现异常, 无法再继续!
		//             if ( size == 0 )
		//                 return 0;
		// 
		//             // 如果没有任何数据, 则相当于数据清空
		//             scriptItem.size = sizeof( SItemBase ) + 1;
		//             if ( scriptItem.declare != 0 )
		//                 scriptItem.size += ( BYTE )size;
		//         }
		// 		else if ( /*ISTIMERECYCLE*/ITEM_IS_CHANGE( itemData->byType ) )
		//         {
		//             STimeRecycle &tr = reinterpret_cast< STimeRecycle& >( static_cast< SItemBase& >( item ) );
		//             DWORD tick = (DWORD)GET_FIELD_NUMBER_DIRECTLY( "recycletiem" );
		//             tr.recycleTiem = (DWORD)(tick  + time( NULL ));
		//             if ( g_Script.m_pPlayer )
		//                 g_Script.m_pPlayer->UpdateTimeRecycleItem();
		//         }
		// 
		// #undef IF_GET_FIELD_NUMBER
		// #undef GET_FIELD_NUMBER_DIRECTLY
		// #undef GET_ARRAY_NUMBER_DIRECTLY
		// 
		//         // 玩家身上的道具，并且出现了数据变化，才更新到客户端！
		//         if ( doCheck && ( memcmp( &tempItem, &item, sizeof( tempItem ) ) != 0 ) )
		//         {
		//             SPackageItem *realItem = g_Script.m_pPlayer->FindItemByPos( item.byCellX, item.byCellY );
		// 			if( realItem ) 
		// 			{
		// 				if ( memcmp( realItem, &tempItem, sizeof( tempItem ) ) != 0 )
		// 					return 0;
		// 
		//                 if ( g_Script.m_pPlayer->m_pItemXQD == NULL && (realItem->wIndex == 40138 || realItem->wIndex == 40139 || realItem->wIndex == 40381) && ( (SScriptItem *)((SItemBase *)realItem) )->declare )
		//                     g_Script.m_pPlayer->m_pItemXQD = &item;
		// 
		//                 if ( g_Script.m_pPlayer->m_pItemXTBL == NULL && (realItem->wIndex == 40141 || realItem->wIndex == 40142 || realItem->wIndex == 40143 ) && ( (SScriptItem *)((SItemBase *)realItem) )->declare )
		//                     g_Script.m_pPlayer->m_pItemXTBL = &item;
		// 
		//                 // 灵珠
		//                 const int nLZType = realItem->wIndex - 40317;
		//                 if ( (nLZType >=0 && nLZType < 6) && g_Script.m_pPlayer->m_pItemLZ[nLZType] == NULL && ( (SScriptItem *)((SItemBase *)realItem) )->declare )
		//                 {
		//                     g_Script.m_pPlayer->m_pItemLZ[nLZType] = &item;
		//                     g_Script.m_pPlayer->m_nLZEffectVal[nLZType] = itemData->dwItemChangePeriod/*itemData->dwExpand*/;
		//                 }
		//  
		// 			}
		//         }
		// 
		//         // 成功后返回1！
		//         lua_pushnumber( L, 1 );
		// zeb 2009.11.19
		//	return 1;

		return 0;
	}

	/*
	static void AttUpdate( SEquipment::Attribute::Unit &att, DWORD vt )
	{
	DWORD type = att.type = vt / 10000;
	// 注意：这里不能做取模炒作，第一，无符号位域不能存负数，第二，传递到脚本外面的数据都是默认做正数处理！
	//if ( type == SGemData::GEMDATA_ACTIONSPEED || type == SGemData::GEMDATA_MOVESPEED )
	//    att.value = 500 + ( vt % 500 );   
	//else
	att.value = vt % 1000;
	}//*/

	// 直接根据item数据创建出指定类型的道具！
	// 可直接用于展示，需要通过modifyitem添加到玩家身上！
	static int L_GenerateItemDetails( lua_State *L )
	{
		// zeb 2009.11.19
		//         if ( !lua_istable( L, 1 ) )
		//             return 0;
		// 
		//         // 用于支撑 GET_FIELD_NUMBER
		//         double _result = 0;
		//         bool _isnul = false;
		// 
		// #define IF_GET_FIELD_NUMBER( _key ) ( ( lua_getfield( L, 1, _key ),         \
		//             ( lua_isnil( L, -1 )                                            \
		//             ? ( _isnul = true, _result = 0 )                                \
		//             : ( _isnul = false, _result = lua_tonumber( L, -1 ) ) ),        \
		//             lua_pop( L, 1 ) ) );    if ( !_isnul )
		// 
		// #define GET_FIELD_NUMBER_DIRECTLY( _key ) ( ( lua_getfield( L, 1, _key ),   \
		//             ( lua_isnil( L, -1 )                                            \
		//             ? ( _isnul = true, _result = 0 )                                \
		//             : ( _isnul = false, _result = lua_tonumber( L, -1 ) ) ),        \
		//             lua_pop( L, 1 ) ), _result )
		// 
		// #define GET_ARRAY_NUMBER_DIRECTLY( _index ) ( ( lua_rawgeti( L, -1, ( _index ) ),   \
		//             ( _result = lua_tonumber( L, -1 ) ), lua_pop( L, 1 ) ), _result )
		// 
		//         WORD index = static_cast< WORD >( GET_FIELD_NUMBER_DIRECTLY( "index" ) );
		// 
		//         const SItemData *itemData = CItem::GetItemData( index );
		//         if ( itemData == NULL )
		//             return 0;
		// 
		//         SPackageItem itemBuffer;
		//         memset( &itemBuffer, 0, sizeof( itemBuffer ) );
		// 
		//         SPackageItem &item = itemBuffer;
		//         item.byCellX = item.byCellY = -1;
		//         item.wIndex = index;
		//         item.size = sizeof( SItemBase );
		// 
		//         // 设置标志位！
		//         IF_GET_FIELD_NUMBER( "flags" )  item.flags = ( DWORD )_result;
		// 
		//         if ( ITEM_IS_OVERLAP( itemData->byType ) )
		//         {
		//             item.size = sizeof( SOverlap );
		// 
		// #define ITEM_CONVERT reinterpret_cast< SOverlap& >( static_cast< SItemBase& >( item ) )
		// 
		//             // 如果为可重叠类，需要更新number
		//             IF_GET_FIELD_NUMBER( "number" ) 
		//             {
		//                 ITEM_CONVERT.number = ( DWORD )_result;
		// 
		//                 // 重叠数量不能为0
		//                 if ( ITEM_CONVERT.number == 0 )
		//                     return 0;
		//             }
		// #undef ITEM_CONVERT
		//         }
		//         else if ( ITEM_IS_EQUIPMENT( itemData->byType ) )
		//         {
		//             item.size = sizeof( SEquipment );
		// 
		// #define ITEM_CONVERT reinterpret_cast< SEquipment& >( static_cast< SItemBase& >( item ) )
		// 
		//             // 如果为装备类需要更新的东西就多咯
		//             // 前缀，等级，经验，耐久度
		//             //IF_GET_FIELD_NUMBER( "prefix" )         ITEM_CONVERT.prefix         = ( BYTE )_result;
		//             //IF_GET_FIELD_NUMBER( "level" )          ITEM_CONVERT.level          = ( BYTE )_result;
		//             //IF_GET_FIELD_NUMBER( "experience" )     ITEM_CONVERT.experience     = ( WORD )_result;
		//             //IF_GET_FIELD_NUMBER( "cur_durance" )    ITEM_CONVERT.durance.cur    = ( WORD )_result;
		//             //IF_GET_FIELD_NUMBER( "max_durance" )    ITEM_CONVERT.durance.max    = ( BYTE )_result;
		//             IF_GET_FIELD_NUMBER( "color" )          ITEM_CONVERT.reserve        = ( ITEM_CONVERT.reserve & 0xfc ) | ( ( BYTE )_result & 0x3 );
		// 
		//             // 镶嵌插槽[5]，随机属性[5]，脚本变量[4]
		//             lua_getfield( L, 1, "slots" );
		//             if ( lua_istable( L, -1 ) ) for ( int i = 0; i < SEquipment::MAX_SLOTS; ++i )
		//             {
		//                 // 如果前面的插槽无效，则后面的数据都不用填了！
		//                 if ( ( i != 0 ) && ITEM_CONVERT.slots[i-1].isInvalid() )
		//                 {
		//                     *( LPBYTE )&ITEM_CONVERT.slots[i] = 0;
		//                     continue;
		//                 }
		// 
		//                 ITEM_CONVERT.slots[i].type = ( BYTE )( ( DWORD )GET_ARRAY_NUMBER_DIRECTLY( i + 1 ) / 100 );
		//                 ITEM_CONVERT.slots[i].value = ( BYTE )( ( DWORD )_result % 100 );
		// 
		//                 // 同上，如果前面的插槽为空，则后面的数据也都不能为镶嵌状态！
		//                 if ( ( i != 0 ) && ITEM_CONVERT.slots[i-1].isEmpty() && ITEM_CONVERT.slots[i].isJewel() )
		//                     *( LPBYTE )&ITEM_CONVERT.slots[i] = 0;
		//             }
		//             lua_pop( L, 1 );
		// 
		// 
		//             IF_GET_FIELD_NUMBER( "isDevelop" )  ITEM_CONVERT.isDevelop = ( BYTE )( _result ) & 0x1;
		//             IF_GET_FIELD_NUMBER( "isRefine" )  ITEM_CONVERT.isRefine = ( BYTE )( _result ) & 0x1;
		//             IF_GET_FIELD_NUMBER( "developCount" )  ITEM_CONVERT.developCount = ( BYTE )( _result ) & 0x3ff;
		//             IF_GET_FIELD_NUMBER( "level" )  ITEM_CONVERT.level = ( BYTE )( _result ) & 0xf;
		// 
		//             // 属性设置规则说明:
		//             // 1000 千位数为具体值
		//             // 10000 万位数为 集体类型
		//             // 激活值
		//             SEquipment::Attribute::Unit * att = NULL;
		//             lua_getfield( L, 1, "activates" );
		//             if ( lua_istable( L, -1 ) )
		//             {
		//                 att = ITEM_CONVERT.attribute.activates;
		//                 for ( int i = 0; i < SEquipment::MAX_ACTIVATES; i++ )
		//                     AttUpdate( att[i], ( DWORD )GET_ARRAY_NUMBER_DIRECTLY( i + 1 ) );
		//             }
		//             else
		//             {
		//                 SpecialItem* sItem = CItem::GetSpecialItem( index );
		//                 if ( sItem )
		//                 {
		//                     for( int i = 0; i < SEquipment::MAX_ACTIVATES; i ++ )
		//                         GetAddAttribute( &(ITEM_CONVERT.attribute.activates[i]), (WORD)sItem->mIntensifyAdd[i], 1 );
		//                 }       
		//             }
		//             lua_pop( L, 1 );
		// 
		//             IF_GET_FIELD_NUMBER( "naturals" )
		//             {
		//                 att = &ITEM_CONVERT.attribute.naturals;
		//                 AttUpdate( att[0], ( DWORD )_result );
		//             }
		// 
		//             lua_getfield( L, 1, "qualitys" );
		//             if ( lua_istable( L, -1 ) )
		//             {
		//                 att = ITEM_CONVERT.attribute.qualitys;
		//                 for ( int i = 0; i < SEquipment::MAX_QUALITYS; i++ )
		//                     AttUpdate( att[i], ( DWORD )GET_ARRAY_NUMBER_DIRECTLY( i + 1 ) );
		//             }
		//             lua_pop( L, 1 );
		// 
		//             lua_getfield( L, 1, "inherits" );
		//             if ( lua_istable( L, -1 ) )
		//             {
		//                 att = ITEM_CONVERT.attribute.inherits;
		//                 for ( int i = 0; i < SEquipment::MAX_INHERITS; i++ )
		//                     AttUpdate( att[i], ( DWORD )GET_ARRAY_NUMBER_DIRECTLY( i + 1 ) );
		//             }
		//             lua_pop( L, 1 );
		// 
		//             // 这里是两个特殊
		//             // 武功特殊处理
		//             IF_GET_FIELD_NUMBER( "addskill" )
		//             {
		//                 int temp = ( int )_result;
		//                 ITEM_CONVERT.attribute.unitSkill.skillType = abs( ( int )( temp / 1000 ) );
		//                 ITEM_CONVERT.attribute.unitSkill.skillValue = temp % 1000;
		//             }
		// 
		//             // 心法特殊处理
		//             IF_GET_FIELD_NUMBER( "addteletgy" )
		//             {
		//                 int temp = ( int )_result;
		//                 ITEM_CONVERT.attribute.unitSkill.teletgyType = abs( ( int )( temp / 1000 ) );
		//                 ITEM_CONVERT.attribute.unitSkill.teletgyValue = temp % 1000;
		//             }
		// 
		//             /*extern DWORD g_dwEquipmentWearScale;
		//             if ( g_dwEquipmentWearScale == 0 )
		//                 g_dwEquipmentWearScale = 1000;*/
		// 
		// //modify by ab_yue
		// //             if ( itemData->wWearPrime != 0xffff )  // 有耐久的装备才能加耐久
		// //             {
		// //                 IF_GET_FIELD_NUMBER( "curwear" ) ITEM_CONVERT.attribute.curWear = ( WORD )_result;
		// //                 IF_GET_FIELD_NUMBER( "maxwear" ) ITEM_CONVERT.attribute.maxWear = ( WORD )_result;
		// // 
		// //                 if ( ITEM_CONVERT.attribute.maxWear != 0xffff && ITEM_CONVERT.attribute.maxWear > itemData->wWearPrime )
		// //                     ITEM_CONVERT.attribute.maxWear = itemData->wWearPrime;
		// //                 if ( ITEM_CONVERT.attribute.curWear > ITEM_CONVERT.attribute.maxWear )
		// //                     ITEM_CONVERT.attribute.curWear = ITEM_CONVERT.attribute.maxWear;
		// //             }
		// //end
		// 
		//             //lua_getfield( L, 1, "attributes" );
		//             //if ( lua_istable( L, -1 ) ) for ( int i = 0; i < SEquipment::MAX_ATTRIBUTES; ++i )
		//             //{
		//             //    // 如果前面的随机属性无效，则后面的数据都不用填了！
		//             //    if ( ( i != 0 ) && ITEM_CONVERT.attributes[i-1].isInvalid() )
		//             //    {
		//             //        *( LPBYTE )&ITEM_CONVERT.attributes[i] = 0;
		//             //        continue;
		//             //    }
		// 
		//             //    ITEM_CONVERT.attributes[i].type = ( BYTE )( ( DWORD )GET_ARRAY_NUMBER_DIRECTLY( i + 1 ) / 100 );
		//             //    ITEM_CONVERT.attributes[i].value = ( BYTE )( ( DWORD )_result % 100 );
		//             //}
		//             //lua_pop( L, 1 );
		// 
		//             lua_getfield( L, 1, "scripts" );
		//             if ( lua_istable( L, -1 ) ) for ( int i = 0; i < SEquipment::MAX_SCRIPTS; ++i )
		//                 ITEM_CONVERT.scriptDatas[i] = ( WORD )GET_ARRAY_NUMBER_DIRECTLY( i + 1 );
		//             lua_pop( L, 1 );
		// 
		// #undef ITEM_CONVERT
		//         }
		// 		else if ( ITEM_IS_SCRIPT( itemData->byType ) )
		//         {
		//             // 如果是脚本类道具，独立一个script表来保存所有脚本数据！
		//             lua_getfield( L, 1, "scripts" );
		//             if ( lua_istable( L, -1 ) )    // 如果没有script子表，则更新失败！
		// 			{
		// 
		// 				size_t size = 0;
		// 				SScriptItem &scriptItem = reinterpret_cast< SScriptItem& >( static_cast< SItemBase& >( item ) );
		// 				try 
		// 				{
		// 					ZeroMemory( item.buffer, sizeof( item.buffer ) );
		// 					lite::Serializer slm( scriptItem.streamData, sizeof( scriptItem.streamData ) );
		// 
		// 					// 遍历8项数据来确定每一个元素的有效性 1 2 4 8 16 32 64 128
		// 					for ( int i = 0; i < 8; i ++ )
		// 					{
		// 						lua_rawgeti( L, -1, i + 1 );
		// 						if ( !lua_isnil( L, -1 ) )  // 有效的数据才保存进数据流
		// 						{
		// 							scriptItem.declare |= ( 1 << i );
		// 							slm( lite::lua_variant( L, -1 ) );
		// 						}
		// 						lua_pop( L, 1 );
		// 					}
		// 
		// 					// 平安结束标志!
		// 					slm.EndEdition();
		// 					size = slm.maxSize();
		// 				}
		// 				catch ( lite::Xcpt & )
		// 				{
		// 				}
		// 
		// 				// 中途出现异常, 无法再继续!
		// 				if ( size == 0 )
		// 					return 0;
		// 
		// 				// 如果没有任何数据, 则相当于数据清空
		// 				scriptItem.size = sizeof( SItemBase ) + 1;
		// 				if ( scriptItem.declare != 0 )
		// 					scriptItem.size += ( BYTE )size;
		// 			}
		// 			// 把script出栈
		// 			lua_pop( L, 1 );
		//         }
		// //modify by ab_yue
		// // 		else if ( ITEM_IS_RECYCLE( itemData->byType ) )
		// //         {
		// //             STimeRecycle &tr = reinterpret_cast< STimeRecycle& >( static_cast< SItemBase& >( item ) );
		// //             DWORD tick = (DWORD)GET_FIELD_NUMBER_DIRECTLY( "recycletiem" );
		// //             tr.recycleTiem = tick  + time( NULL );
		// //             tr.size = sizeof( STimeRecycle );
		// //         }
		// //end
		// 
		// #undef IF_GET_FIELD_NUMBER
		// #undef GET_FIELD_NUMBER_DIRECTLY
		// #undef GET_ARRAY_NUMBER_DIRECTLY
		// 
		//         // 成功后返回dataHandle！
		//         void *p = lua_newuserdata( L, sizeof( SPackageItem ) );
		// //        memcpy( p, &item, sizeof( SPackageItem ) );
		// // zeb 2009.11.19
		//return 1;

		return 0;
	}

	static int L_CreateObjectIndirect(lua_State *L)
	{
		if (!lua_istable(L, 1))
			return 0;

		double _result = 0;
		bool   _isnul  = false;

#define IF_GET_FIELD_NUMBER(_key) ((lua_getfield(L, 1, _key),				\
	(lua_isnil(L, -1)                                      \
	? (_isnul = true, _result = 0)                         \
	: (_isnul = false, _result = lua_tonumber(L, -1))),    \
	lua_pop(L, 1)));    if (!_isnul)

#define GET_FIELD_NUMBER_DIRECTLY(_key) ((lua_getfield(L, 1, _key),			\
	(lua_isnil(L, -1)												\
	? (_isnul = true, _result = 0)									\
	: (_isnul = false, _result = lua_tonumber(L, -1))),			\
	lua_pop(L, 1)), _result)

#define GET_ARRAY_NUMBER_DIRECTLY(_index) ((lua_rawgeti(L, -1, (_index)),   \
	(_result = lua_tonumber( L, -1)), lua_pop(L, 1)), _result)


		DWORD regionID = static_cast<DWORD>(GET_FIELD_NUMBER_DIRECTLY("regionId"));
		if (0 == regionID)
			return rfalse(2, 1, "(COI)没有指定目标场景");

		CRegion *destRegion = GetRegionById(regionID);
		if (!destRegion)
			return rfalse(2, 1, "(COI)找不到指定场景");

		// 先尝试获取控制ID号，如果有，则进行重复检测
		DWORD controlId = 0;
		IF_GET_FIELD_NUMBER("controlId")
		{
			if (0 != _result && destRegion->CheckObjectByControlId(controlId = static_cast<DWORD>(_result)))
				return rfalse(2, 1, "(COI)指定场景中的制定编号对象已经存在[r=%d, cid=%d]", regionID, controlId);
		}

		DWORD objectType = static_cast<DWORD>(GET_FIELD_NUMBER_DIRECTLY("objectType"));
		if (1 == objectType)			// 创建NPC
		{
			CNpc::SParameter param;
			memset(&param, 0, sizeof(param));

			IF_GET_FIELD_NUMBER( "imageID"		) param.wImageID		= static_cast< DWORD >(_result);
			IF_GET_FIELD_NUMBER( "x"			) param.wX				= static_cast< WORD >(_result);
			IF_GET_FIELD_NUMBER( "y"			) param.wY				= static_cast< WORD >(_result);
			IF_GET_FIELD_NUMBER( "dir"			) param.wDir			= static_cast< WORD >(_result);
			IF_GET_FIELD_NUMBER( "chose"		) param.wChose			= static_cast< WORD >(_result);
			IF_GET_FIELD_NUMBER( "moveType"		) param.m_byMoveType	= static_cast< WORD >(_result);
			IF_GET_FIELD_NUMBER( "School"		) param.wSchool			= static_cast< WORD >(_result);
			IF_GET_FIELD_NUMBER( "clickScript"	) param.wClickScriptID	= static_cast< DWORD >(_result);
			IF_GET_FIELD_NUMBER( "LiveTime"	    ) param.wLiveTime		= static_cast< DWORD >(_result);

			CNpc *npc = destRegion->CreateNpc(&param, controlId);
			if (!npc)
				return rfalse(2, 1, "(COI)CreateNpc失败!");

			lua_pushnumber(L, npc->GetGID());
		}
		else if (0 == objectType)		// 创建怪物
		{
			CMonster::SParameter param;
			memset(&param, 0, sizeof(param));

			param.wListID = static_cast<WORD>(GET_FIELD_NUMBER_DIRECTLY("monsterId"));
			const SMonsterBaseData *pMonsterData = CMonsterService::GetInstance().GetMonsterBaseData(param.wListID);
			if (!pMonsterData)
				return rfalse(2, 1, "(COI)在基本列表中找不到对应ID的怪物!");

			IF_GET_FIELD_NUMBER( "refreshTime"  ) param.dwRefreshTime    = static_cast<DWORD>( _result );
			IF_GET_FIELD_NUMBER( "x"			) param.wX														 = static_cast<WORD >( _result );
			IF_GET_FIELD_NUMBER( "y"			) param.wY													     = static_cast<WORD >( _result );
			IF_GET_FIELD_NUMBER( "CreatePlayerID"			) param.wCreatePlayerID               = static_cast<DWORD >( _result );
			IF_GET_FIELD_NUMBER("RankIndex") param.dRankIndex = static_cast<DWORD >(_result);

			param.reginID = destRegion->m_wRegionID;
			CMonster *monster = destRegion->CreateMonster(&param, controlId);
			if (!monster)
				return rfalse(2, 1, "(COI)CreateMonster失败!");

			CDynamicRegion *pDynamicRegion = (CDynamicRegion *)destRegion->DynamicCast(IID_DYNAMICREGION);
			if (pDynamicRegion && 36000000 == param.dwRefreshTime)
				pDynamicRegion->m_Monster.insert(monster->GetGID());

			lua_pushnumber(L, monster->GetGID());
		}
		else
		{
			return 0;
		}

#undef IF_GET_FIELD_NUMBER
#undef GET_FIELD_NUMBER_DIRECTLY
#undef GET_ARRAY_NUMBER_DIRECTLY
		return 1;
	}

	static int L_RemoveObjectIndirect(lua_State *L)
	{
		DWORD regionId  = static_cast<DWORD>(lua_tonumber(L, 1));
		DWORD controlId = static_cast<DWORD>(lua_tonumber(L, 2));

		CRegion *destRegion = GetRegionById(regionId);
		if (!destRegion)
			return rfalse(2, 1, "(ROI)找不到指定场景");

		BOOL result = destRegion->RemoveObjectByControlId(controlId);
		if (!result)
			return 0;

		lua_pushnumber(L, 1);
		return 1;
	}


	static int L_UpdateItemSetting( lua_State *L )
	{
		// zeb 2009.11.19
		//        // 该函数只有唯一的参数,一个包含创建信息的表!
		//        if ( !lua_istable( L, 1 ) )
		//            return 0;
		//
		//        // 用于支撑 GET_FIELD_NUMBER
		//        double _result = 0;
		//        bool _isnul = false;
		//
		//#define IF_GET_FIELD_NUMBER( _key ) ( ( lua_getfield( L, 1, _key ),         \
		//            ( lua_isnil( L, -1 )                                            \
		//            ? ( _isnul = true, _result = 0 )                                \
		//            : ( _isnul = false, _result = lua_tonumber( L, -1 ) ) ),        \
		//            lua_pop( L, 1 ) ) );    if ( !_isnul )
		//
		//        int index = 0;
		//        IF_GET_FIELD_NUMBER( "index" )
		//            index = static_cast< int >( _result );
		//        else
		//            return 0;
		//
		//        if ( ( index <= 0 ) || ( index >= 65535 ) )
		//            return 0;
		//
		//        // 这个区间内的编号才是有效的道具编号!
		//        SItemData newItemData;
		//        SItemData *itemData = &newItemData;
		//        itemData->wItemID = 0;
		//
		//        IF_GET_FIELD_NUMBER( "restore"  ) 
		//            itemData = NULL;
		//        else
		//        {
		//            IF_GET_FIELD_NUMBER( "delete"   ) itemData->wItemID = 0xffff;
		//        }
		//
		//        if ( ( itemData != NULL ) && ( itemData->wItemID == 0 ) )
		//        {
		//            if ( const SItemData *oldItemData = CItem::GetItemData( index ) )
		//                newItemData = *oldItemData;
		//            else
		//            {
		//                memset( &newItemData, 0, sizeof( newItemData ) );
		//                newItemData.wItemID = index;
		//                newItemData.szName[0] = '-';
		//            }
		//
		//            // 先更新名字!
		//            lua_getfield( L, 1, "name" );
		//            if ( lua_isstring( L, -1 ) ) dwt::strcpy( newItemData.szName, lua_tostring( L, -1 ), sizeof( newItemData.szName ) );
		//            lua_pop( L, 1 );
		//
		//            // 更新所有相关项目
		//            IF_GET_FIELD_NUMBER( "buy"      ) newItemData.dwBuy         = static_cast< DWORD >( _result );      // 购入价格
		//            IF_GET_FIELD_NUMBER( "sell"     ) newItemData.dwSell        = static_cast< DWORD >( _result );      // 卖出价格
		//	        IF_GET_FIELD_NUMBER( "icon"     ) newItemData.wSellPrice    = static_cast< WORD >( _result );	    // 物品侠义元宝卖出价格
		//	        IF_GET_FIELD_NUMBER( "gift"     ) newItemData.wScorePrice   = static_cast< WORD >( _result );	    // 赠品价格
		//            IF_GET_FIELD_NUMBER( "image"    ) newItemData.byGraph1      = static_cast< WORD >( _result );       // 图片编号（如果是武器的话，也使用这一个ID去找具体的武器动画）
		//	        IF_GET_FIELD_NUMBER( "image2"   ) newItemData.wGraph2       = static_cast< WORD >( _result );       // 地面显示用的图片编号
		//	        IF_GET_FIELD_NUMBER( "sound"    ) newItemData.wSound1       = static_cast< WORD >( _result );       // 该物品掉落（放入物品栏，装备）的音效编号
		//	        IF_GET_FIELD_NUMBER( "overlap"  ) newItemData.wSpecial      = static_cast< WORD >( _result );       // 特殊应用！（目前用于最大的重叠数量）
		//            IF_GET_FIELD_NUMBER( "groupRQ"  ) newItemData.byGroupEM     = static_cast< BYTE >( _result );       // 门派需求
		//            IF_GET_FIELD_NUMBER( "levelRQ"  ) newItemData.byLevelEM     = static_cast< WORD >( _result );       // 等级需求
		//            IF_GET_FIELD_NUMBER( "enRQ"     ) newItemData.wENEM         = static_cast< WORD >( _result );       // 活力需求
		//            IF_GET_FIELD_NUMBER( "inRQ"     ) newItemData.wINEM         = static_cast< WORD >( _result );       // 悟性需求
		//            IF_GET_FIELD_NUMBER( "stRQ"     ) newItemData.wSTEM         = static_cast< WORD >( _result );       // 体魄需求
		//            IF_GET_FIELD_NUMBER( "agRQ"     ) newItemData.wAGEM         = static_cast< WORD >( _result );       // 身法需求
		//            IF_GET_FIELD_NUMBER( "hpAD"     ) newItemData.wHPAD        = static_cast< WORD >( _result );       // 增加生命最大值
		//            IF_GET_FIELD_NUMBER( "spAD"     ) newItemData.wSPAD        = static_cast< WORD >( _result );       // 增加体力最大值
		//            IF_GET_FIELD_NUMBER( "mpAD"     ) newItemData.wMPAD        = static_cast< WORD >( _result );       // 增加内力最大值
		//            IF_GET_FIELD_NUMBER( "hpR"      ) newItemData.wHPR         = static_cast< WORD >( _result );       // 恢复生命量
		//            IF_GET_FIELD_NUMBER( "spR"      ) newItemData.wSPR         = static_cast< WORD >( _result );       // 恢复体力量
		//            IF_GET_FIELD_NUMBER( "mpR"      ) newItemData.wMPR         = static_cast< WORD >( _result );       // 恢复内力量
		//            IF_GET_FIELD_NUMBER( "type"     ) newItemData.byType        = static_cast< BYTE >( _result );       // 该道具的类型
		//	        IF_GET_FIELD_NUMBER( "cells"    ) newItemData.byHoldSize    = static_cast< BYTE >( _result );       // 该道具占用的物品栏格子
		//            // 使用/装备相关数据
		//            IF_GET_FIELD_NUMBER( "damAD"    ) newItemData.wDAMAD        = static_cast< WORD >( _result );       // 加攻击力
		//            IF_GET_FIELD_NUMBER( "powAD"    ) newItemData.wPOWAD        = static_cast< WORD >( _result );       // 加内功攻击力
		//            IF_GET_FIELD_NUMBER( "defAD"    ) newItemData.wDEFAD        = static_cast< WORD >( _result );       // 加防御力
		//            IF_GET_FIELD_NUMBER( "agiAD"    ) newItemData.wAGIAD        = static_cast< WORD >( _result );       // 加灵敏度
		//            IF_GET_FIELD_NUMBER( "amuckAD"  ) newItemData.wAmuckAD      = static_cast< WORD >( _result );       // 加杀气
		//            IF_GET_FIELD_NUMBER( "luckAD"   ) newItemData.wLUAD         = static_cast< WORD >( _result );       // 加气运
		//            IF_GET_FIELD_NUMBER( "hpRAD"    ) newItemData.wRHPAD        = static_cast< WORD >( _result );       // 加生命自动恢复速度
		//            IF_GET_FIELD_NUMBER( "spRAD"    ) newItemData.wRSPAD        = static_cast< WORD >( _result );       // 加体力自动恢复速度
		//            IF_GET_FIELD_NUMBER( "mpRAD"    ) newItemData.wRMPAD        = static_cast< WORD >( _result );       // 加内力自动恢复速度
		//            IF_GET_FIELD_NUMBER( "clear"    ) newItemData.wClear        = static_cast< WORD >( _result );       // 消除异常状态
		//            IF_GET_FIELD_NUMBER( "pointAD"  ) newItemData.wPointAD      = static_cast< WORD >( _result );       // 加属性点
		//            IF_GET_FIELD_NUMBER( "levelAD"  ) newItemData.wLevelAD      = static_cast< WORD >( _result );       // 加等级
		//            // IF_GET_FIELD_NUMBER( "material" ) BYTE material;          // 道具的材质类型，用于设定耐久度的扣除量
		//            IF_GET_FIELD_NUMBER( "subLevel" ) newItemData.wSubLevel     = static_cast< WORD >( _result );       // 装备的子等级，客户端需要用的……
		//            IF_GET_FIELD_NUMBER( "script"   ) newItemData.dwScripID     = static_cast< DWORD >( _result );      // 脚本编号
		//	        IF_GET_FIELD_NUMBER( "changing" ) newItemData.wEffect       = static_cast< WORD >( _result );       // 效果图（目前用于人皮面具变身）
		//            IF_GET_FIELD_NUMBER( "mainLevel") newItemData.nMainLevel    = static_cast< BYTE >( _result );       // 以下4个数据和套装相关，只需要在服务器判断！
		//            IF_GET_FIELD_NUMBER( "addType"  ) newItemData.nAddType      = static_cast< BYTE >( _result );
		//	        IF_GET_FIELD_NUMBER( "addScale" ) newItemData.wAddScale     = static_cast< BYTE >( _result );       // 加成比例 
		//	        // IF_GET_FIELD_NUMBER( "gestID" ) BYTE wGestID;           // 习得武功或队形技编号
		//	        // IF_GET_FIELD_NUMBER( "gestLevel" ) BYTE wGestLevel;        // 武功自身等级需求
		//            // IF_GET_FIELD_NUMBER( "durance" ) BYTE durance;           // 道具的默认耐久度，用于初始化时设定耐久度的上限
		//            IF_GET_FIELD_NUMBER( "stone" ) newItemData.stone            = static_cast< WORD >( _result );       // 影响的成功百分比
		//            IF_GET_FIELD_NUMBER( "expand" ) newItemData.dwExpand        = static_cast< DWORD >( _result );       // 加能力点
		//            IF_GET_FIELD_NUMBER( "restrict" ) newItemData.restrict      = static_cast< WORD >( _result );       // 是否需要经过赠送手续才赋予功能
		//            IF_GET_FIELD_NUMBER( "wearPrime" ) newItemData.wWearPrime   = static_cast< WORD >( _result );       // 增加效果的持续时间，如果这个时间是0的话，说明是永久性的增加效果
		//        }
		//
		//        // 是否做全服务器更新通知！
		//        bool fullSynchro = ( lua_toboolean( L, 2 ) == 1 );
		//
		//        // 更新并比对crc数据,判断是否更新
		//        DWORD crc32 = ( CItem::GetRawData( index ) ? CItem::GetRawData( index )->first : -1 );
		//
		//        // 服务器端，属于数据全更新，但就算非同步数据也被修改了，但crc可能未变化，仍然达不到同步条件
		//        CItem::UpdateItemData( index, itemData, true );
		//        if ( fullSynchro )
		//        {
		//            std::pair< DWORD, const SItemData* > *rawData = CItem::GetRawData( index );
		//            if ( ( rawData != NULL ) && ( rawData->first != crc32 ) )
		//            {
		//                // 更新到所有客户端
		//                AnswerUpdateItemDataMsg msg;
		//                msg.index = index;
		//
		//                try
		//                {
		//                    lite::Serializer slm( msg.streamBuffer, sizeof( msg.streamBuffer ) );
		//
		//                    if ( rawData->second == NULL )
		//                        slm( lite::Variant() );
		//                    else
		//                        slm( lite::Variant( static_cast< const ClientVisible* >( rawData->second ), sizeof( ClientVisible ) ) );
		//
		//                    BroadcastMsg( &msg, static_cast< WORD>( sizeof( msg ) - slm.EndEdition() ) );
		//                }
		//                catch ( lite::Xcpt & )
		//                {
		//                }
		//            }
		//        }
		//
		//#undef IF_GET_FIELD_NUMBER
		//
		//        lua_pushnumber( L, 1 );
		// zeb 2009.11.19
		return 1;
	}

	static int L_CreateFolder( lua_State *L )
	{
		return Details::CreateFolder( lua_tostring( L, 1 ) ), 0;
	}

	static int L_DropItems( lua_State *L )
	{
		if ( !lua_istable( L, 1 ) )
			return 0;

		//         int life = -1;
		//         int loop_seg = 1;
		//         int loop_end = 100;
		//         int money_min = 1;
		//         int money_max = 100;
		//         int margin = 1;
		//         int x = 0, y = 0;
		//         int useimm = 0, remain = 0;
		// 
		//         CRegion *destRegion = NULL;
		//         _IF_GET_FIELD_NUMBER( 1, "regionId" ) 
		//         {
		//             destRegion = GetRegionById( static_cast< int >( __number ) );
		//             if ( destRegion == NULL ) 
		//                 return 0;
		// 
		//             _IF_GET_FIELD_NUMBER( 1, "x" ) x = static_cast< int >( __number ); else return 0;
		//             _IF_GET_FIELD_NUMBER( 1, "y" ) y = static_cast< int >( __number ); else return 0;
		//         }
		//         else if ( g_Script.m_pMonster != NULL )
		//         {
		//             destRegion = g_Script.m_pMonster->m_ParentRegion;
		//             x = g_Script.m_pMonster->m_wCurX;
		//             y = g_Script.m_pMonster->m_wCurY;
		//         }
		//         else return 0;
		// 
		//         if ( destRegion == NULL )
		//             return 0;
		// 
		//         _IF_GET_FIELD_NUMBER( 1, "loop_seg" ) loop_seg = static_cast< int >( __number );
		//         _IF_GET_FIELD_NUMBER( 1, "loop_end" ) loop_end = static_cast< int >( __number );
		//         _IF_GET_FIELD_NUMBER( 1, "life" ) life = static_cast< int >( __number );
		//         _IF_GET_FIELD_NUMBER( 1, "money_min" ) money_min = static_cast< int >( __number );
		//         _IF_GET_FIELD_NUMBER( 1, "money_max" ) money_max = static_cast< int >( __number );
		//         _IF_GET_FIELD_NUMBER( 1, "margin" ) margin = static_cast< int >( __number ) + 1;
		//         _IF_GET_FIELD_NUMBER( 1, "remain" ) remain = static_cast< int >( __number );
		// 
		//         if ( loop_seg <= 0 || loop_end <= 0 )
		//             return 0;
		// 
		//         int index = -1;
		//         int dir = -1;
		//         int levelMax = 0;
		//         for ( int i = loop_seg; i < loop_end; ++i )
		//         {
		//             WORD itemId = ( WORD )_GET_ARRAY_NUMBER_DIRECTLY( 1, i );
		//             if ( !__validate )
		//                 break;
		// 
		//             const SItemData *itemData = CItem::GetItemData( itemId );
		//             if ( itemData == NULL )
		//                 continue;
		// 
		//             CItem::SParameter param;
		//             ZeroMemory( &param, sizeof( param ) );
		//             param.dwLife = life;
		//             param.xTile = x, param.yTile = y;
		//             param.remain = remain;
		//             DWORD number = ( itemId == 0 ) ? ( money_min + rand() % ( money_max - money_min ) ) : 0;
		// 
		//             // 可能找到的点是不能行走的坐标！需要重找
		// _refind:
		//             index ++;
		//             if ( index >= 1000 ) // 过大的检测范围，再扩展下去会影响性能
		//                 break;
		// 
		//             if ( index >= levelMax * 4 )
		//             {
		//                 levelMax ++;
		//                 y -= margin;
		//                 index = 0;
		//                 param.xTile = x;
		//                 param.yTile = y;
		//             }
		//             else if ( levelMax )
		//             {
		//                 dir = 1 + ( index / levelMax ) * 2;
		// 
		//                 extern POINT vdir[8];
		//                 param.xTile = ( x += vdir[ dir ].x * margin );
		//                 param.yTile = ( y += vdir[ dir ].y * margin );
		//             }
		// 
		//             if ( !destRegion->isTileWalkAble( x, y ) )
		//                 goto _refind;
		// 
		//             // 寻找可放置的场景坐标
		//             CItem::GenerateNewGroundItem( destRegion, 0, 
		//                 param, GenItemParams( itemId, number ), LogInfo( 4, "脚本(DropItems)生成" ) );
		//         }

		return 0;
	}

	static int L_SelectObject( lua_State *L )
	{
		DWORD controlId = static_cast< DWORD >( lua_tonumber( L, 1 ) );     // 目标对象的控制编号
		DWORD regionId = static_cast< DWORD >( lua_tonumber( L, 2 ) );      // 场景编号，会先和当前可使用的场景作匹配

		CRegion *destRegion = NULL;
		if ( regionId == 0 )
			destRegion = g_Script.m_pPlayer ? g_Script.m_pPlayer->m_ParentRegion : NULL;
		else
			destRegion = GetRegionById( regionId );
		if ( destRegion == NULL )
			return 0;

		QWORD uniqueId = luaEx_toint63( L, 3 );
		//double tempNumber = lua_tonumber( L, 3 );     // 特殊的建筑物的唯一ID
		//QWORD &uniqueId = reinterpret_cast< QWORD& >( tempNumber );
		if ( uniqueId != 0 )
		{
			std::map< QWORD, LPIObject >::iterator it = destRegion->m_BuildingMap.find( uniqueId );
			if ( it == destRegion->m_BuildingMap.end() )
				return 0;

			g_Script.m_pBuilding = ( CBuilding* )it->second->DynamicCast( IID_BUILDING );
		}
		else
		{
			LPIObject obj = destRegion->GetObjectByControlId( controlId );
			if ( !obj )
				return 0;

			if ( CMonster *monster = ( CMonster* )obj->DynamicCast( IID_MONSTER ) )
				g_Script.m_pMonster = monster;
			else if ( CNpc *npc = ( CNpc* )obj->DynamicCast( IID_NPC ) )
				g_Script.m_pNpc = npc;
			else if ( CBuilding *building = ( CBuilding* )obj->DynamicCast( IID_BUILDING ) )
				g_Script.m_pBuilding = building;
			else
				return 0;
		}

		lua_pushboolean( L, 1 );
		return 1;
	}

	static int L_DBRPC( lua_State *L )
	{
		// 该函数只有唯一的参数,一个包含创建信息的表!
		if ( !lua_istable( L, 1 ) )
			return 0;

		LPCSTR spname = _GET_FIELD_STRING_DIRECTLY( 1, "sp" );
		if ( spname == NULL )
			return rfalse( 2, 1, "没有设定存储过程名！" );

		SQGameServerRPCOPMsg msg;
		lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );

		try
		{
			slm [OP_PREPARE_STOREDPROC] ( ( std::string( "script." ) + spname ).c_str() );

			_IF_GET_FIELD_NUMBER( 1, "args" )
			{
				slm [OP_BEGIN_PARAMS];

				for ( int i = 1; i < static_cast< int >( __number ) + 1; i ++ )
				{
					lua_rawgeti( L, 1, i );
					if ( !lua_isnil( L, -1 ) )
						slm ( i ) ( static_cast< lite::Variant& >( lite::lua_variant( L, -1 ) ) );
					lua_pop( L, 1 );
				}

				slm [OP_END_PARAMS];
			}

			slm [OP_CALL_STOREDPROC];

			// 是否存在回调脚本
			if ( lua_istable( L, 2 ) )
			{
				LPCSTR callback_name = _GET_FIELD_STRING_DIRECTLY( 2, "callback" );
				if ( callback_name == NULL )
					return rfalse( 2, 1, "没有设定回调函数名！" );

				slm [OP_INIT_RETBOX] ( 1024 )
					[OP_BOX_VARIANT] ( (int)SMessage::EPRO_SCRIPT_MESSAGE )
					[OP_BOX_VARIANT] ( (int)SScriptBaseMsg::EPRO_BACK_MSG )
					[OP_BOX_VARIANT] ( g_Script.m_pPlayer ? g_Script.m_pPlayer->GetSID() : 0 )
					[OP_BOX_VARIANT] ( callback_name );

				int args = static_cast< int >( _GET_FIELD_NUMBER_DIRECTLY( 2, "args" ) );
				slm [OP_BOX_VARIANT] ( args );

				for ( int i = 1; i < static_cast< int >( args ) + 1; i ++ )
				{
					lua_rawgeti( L, 2, i );
					int type = lua_type( L, -1 );
					LPCSTR tempStr = ( ( type != LUA_TSTRING ) ? NULL : lua_tostring( L, -1 ) );
					if ( tempStr != NULL ) 
					{
						// 希望返回一个指定的存储过程参数
						if ( tempStr[0] == '?'  )
							slm [OP_BOX_PARAM] ( atoi( tempStr + 1 ) );

						// 希望返回一个表数据，并将其转换回lua_table '@'表示使用2进制数据块，'#'表示使用2维表格， '|'表示使用1维表格
						else if ( tempStr[0] == '@' || tempStr[0] == '#' || tempStr[0] == '|' )
							slm [OP_TABLE_DATA] ( abs( atoi( tempStr + 1 ) ) ) ( tempStr[0] ); 

						// 其他情况下，希望返回当前这个字符串数据
						else
							slm [OP_BOX_VARIANT] ( tempStr ); 
					}
					else
						slm [OP_BOX_VARIANT] ( static_cast< lite::Variant& >( lite::lua_variant( L, -1 ) ) ); // 希望返回一个lua数据。。。

					lua_pop( L, 1 );
				}

				slm [OP_RETURN_BOX];
			}

			slm [OP_RPC_END];

			// 全都准备好后，再发送消息到登陆服务器！并且由登陆服务器再转到账号服务器
			msg.dstDatabase = static_cast< BYTE >( _GET_FIELD_NUMBER_DIRECTLY( 1, "dbtype" ) );
			dwt::strcpy( (char*)msg.GetExtendBuffer(), g_Script.m_pPlayer ? g_Script.m_pPlayer->GetAccountPrefix() : "", 16 );
			if ( !SendToLoginServer( &msg, ( long )( sizeof( msg ) - slm.EndEdition() ) ) )
				return rfalse( 2, 1, "发送DBRPC消息失败" );
		}
		catch ( lite::Xcpt & )
		{
			return 0;
		}

		return 0;
	}

	static int L_GetObjectUniqueId( lua_State *L )
	{
		DWORD npcId			= 0;
		DWORD monsterId		= 0;
		DWORD playerId		= 0;

		if (g_Script.m_pNpc    ) npcId     = g_Script.m_pNpc->m_Property.controlId;
		if (g_Script.m_pMonster) monsterId = g_Script.m_pMonster->m_Property.controlId;
		if (g_Script.m_pPlayer ) playerId  = g_Script.m_pPlayer->m_Property.m_dwStaticID;

		lua_pushnumber(L, npcId    );
		lua_pushnumber(L, monsterId);
		lua_pushnumber(L, playerId );
		return 3;
	}

	// 仇恨系统所增加的道具效果
	static int L_SetHateMonster( lua_State *L )
	{
		if ( g_Script.m_pPlayer == NULL ) 
			return 0;

		//        WORD imageId = static_cast< WORD >( lua_tonumber( L, 1 ) );
		//        WORD damage = static_cast< WORD >( lua_tonumber( L, 2 ) );

		//       g_Script.m_pPlayer->m_Property.hateSetting.targetImageId = imageId;
		//     g_Script.m_pPlayer->m_Property.hateSetting.additionDamage = damage;
		return 0;
	}

	static int L_UpdateMonsterData( lua_State *L )
	{

#undef IF_GET_FIELD_NUMBER

		return 0;
	}

	static int L_TalkBubble( lua_State *L )
	{
		DWORD type = static_cast< DWORD >( lua_tonumber( L, 1 ) );
		LPCSTR info = lua_tostring( L, 2 );
		bool broadcast = ( lua_toboolean( L, 3 ) == 0 );

		if ( broadcast )
		{
			if ( type == 0 && g_Script.m_pPlayer   ) g_Script.m_pPlayer->TalkBubble( info );
			else if ( type == 1 && g_Script.m_pNpc ) g_Script.m_pNpc->TalkBubble( info );
			else if ( g_Script.m_pMonster          ) g_Script.m_pMonster->TalkBubble( info );
		}
		else if ( g_Script.m_pPlayer )
		{
			SAChatPublic msg;
			msg.dwGID = 0;

			if      ( type == 0                    ) msg.dwGID = g_Script.m_pPlayer->GetGID();
			else if ( type == 1 && g_Script.m_pNpc ) msg.dwGID = g_Script.m_pNpc->GetGID();
			else if ( g_Script.m_pMonster          ) msg.dwGID = g_Script.m_pMonster->GetGID();

			if ( msg.dwGID != 0 )
			{
				msg.wSoundId = 0;
				dwt::strcpy( msg.cCharData, info, sizeof( msg.cCharData ) );
				g_StoreMessage( g_Script.m_pPlayer->m_ClientIndex, &msg, msg.GetMySize() );
			}
		}

		return 0;
	}

	static int L_GameRecords( lua_State *L )
	{
		return 0;
	}

	// 设置角色的临时属性状态！
	static int L_SetTempStatus( lua_State *L )
	{
		//         if ( !lua_istable( L, 1 ) && !lua_isnone( L, 1 ) )
		//             return 0;
		// 
		//         CFightObject *target = g_Script.m_pPlayer;
		// 
		//         _IF_GET_FIELD_NUMBER( 1, "target" )
		//         {
		//             DWORD controlId = static_cast< DWORD >( __number );
		//             if ( controlId == 0 )
		//                 target = g_Script.m_pMonster;
		//             else 
		//             {
		//                 _IF_GET_FIELD_NUMBER( 1, "regionId" )
		//                 {
		//                     DWORD regionId = static_cast< DWORD >( __number );
		//                     CRegion *destRegion = GetRegionById( regionId );
		//                     if ( destRegion == NULL )
		//                         return 0;
		// 
		//                     LPIObject obj = destRegion->GetObjectByControlId( controlId );
		//                     target = ( CFightObject* )obj->DynamicCast( IID_MONSTER );
		//                 }
		//             }
		//         }
		// 
		//         if ( target == NULL )
		//             return 0;
		// 
		//         int value;
		//         DWORD tick, cd;
		//         DWORD prevState = target->m_dwTempState;
		// 
		//         if ( lua_getfield( L, 1, "damage" ), lua_istable( L, -1 ) )
		//         {
		//             value = ( int )_GET_ARRAY_NUMBER_DIRECTLY( -1, 1 );
		//             tick  = ( DWORD )_GET_ARRAY_NUMBER_DIRECTLY( -1, 2 );
		//             cd    = ( DWORD )_GET_ARRAY_NUMBER_DIRECTLY( -1, 3 );
		//             target->m_tvAddDam.Set( value, (WORD)tick, (WORD)cd, target->m_dwTempState ); 
		//         }
		//         lua_pop( L, 1 );
		// 
		//         if ( lua_getfield( L, 1, "power" ), lua_istable( L, -1 ) )
		//         {
		//             value = ( int )_GET_ARRAY_NUMBER_DIRECTLY( -1, 1 );
		//             tick  = ( DWORD )_GET_ARRAY_NUMBER_DIRECTLY( -1, 2 );
		//             cd    = ( DWORD )_GET_ARRAY_NUMBER_DIRECTLY( -1, 3 );
		//             target->m_tvAddPow.Set( value, (WORD)tick, (WORD)cd, target->m_dwTempState ); 
		//         }
		//         lua_pop( L, 1 );
		// 
		//         if ( lua_getfield( L, 1, "defence" ), lua_istable( L, -1 ) )
		//         {
		//             value = ( int )_GET_ARRAY_NUMBER_DIRECTLY( -1, 1 );
		//             tick  = ( DWORD )_GET_ARRAY_NUMBER_DIRECTLY( -1, 2 );
		//             cd    = ( DWORD )_GET_ARRAY_NUMBER_DIRECTLY( -1, 3 );
		//             target->m_tvAddDef.Set( value, (WORD)tick, (WORD)cd, target->m_dwTempState ); 
		//         }
		//         lua_pop( L, 1 );
		// 
		//         if ( lua_getfield( L, 1, "agility" ), lua_istable( L, -1 ) )
		//         {
		//             value = ( int )_GET_ARRAY_NUMBER_DIRECTLY( -1, 1 );
		//             tick  = ( DWORD )_GET_ARRAY_NUMBER_DIRECTLY( -1, 2 );
		//             cd    = ( DWORD )_GET_ARRAY_NUMBER_DIRECTLY( -1, 3 );
		//             target->m_tvAddAgi.Set( value, (WORD)tick, (WORD)cd, target->m_dwTempState ); 
		//         }
		//         lua_pop( L, 1 );
		// 
		//         if ( lua_getfield( L, 1, "HPMax" ), lua_istable( L, -1 ) )
		//         {
		//             value = ( int )_GET_ARRAY_NUMBER_DIRECTLY( -1, 1 );
		//             tick  = ( DWORD )_GET_ARRAY_NUMBER_DIRECTLY( -1, 2 );
		//             cd    = ( DWORD )_GET_ARRAY_NUMBER_DIRECTLY( -1, 3 );
		//             target->m_tvAddHPMax.Set( value, (WORD)tick, (WORD)cd, target->m_dwTempState ); 
		//         }
		//         lua_pop( L, 1 );
		// 
		//         if ( prevState != target->m_dwTempState )
		//         {
		//             target->SendFightExtraState( target->m_dwTempState );
		//             target->SendPlayerAbility();
		//         }
		return 0;
	}

	static int L_GetPlayerSIDNAME( lua_State *L )
	{
		if ( g_Script.m_pPlayer == NULL ) 
			return 0;

		std::string account = g_Script.m_pPlayer->GetAccount();
		int len = ( int )account.find_first_of( ":" );

		if ( len < 0 )
			lua_pushstring( L, "" );
		else
			lua_pushstring( L, account.substr( 0, len ).c_str() );

		return 1;
	}

	static int L_MoveToServer( lua_State *L )
	{
		if ( g_Script.m_pPlayer == NULL ) 
			return 0;

		//SARebindLoginServerMsg msg;

		//LPCSTR serverip = lua_tostring( L, 1 );
		//LPCSTR serverport = lua_tostring( L, 2 );
		//LPCSTR password = lua_tostring( L, 3 );

		//if ( serverip == NULL || password == NULL )
		//	return 0;

		//if ( g_Script.m_pPlayer == NULL )
		//	return 0;

		//
		//std::string temp = g_Script.m_pPlayer->GetAccount();
		//int len = temp.find_first_of( ":" );

		//LPCSTR account = NULL;
		//// 没有SID
		//if ( len < 0 )
		//	return 0;

		//try
		//{
		//	lite::Serializer slz( msg.streamData, sizeof( msg.streamData ) );
		//	slz( serverip );
		//	slz( serverport );
		//	slz( temp.substr( ( len + 1 ), temp.size() - len ).c_str() );
		//	slz( password );
		//	slz( "" );

		//	g_StoreMessage( g_Script.m_pPlayer->m_ClientIndex, &msg, sizeof( msg ) - slz.EndEdition() );
		//}
		//catch ( lite::Xcpt & )
		//{
		//}
		return 0;
	}

	static int L_SavePlayerDataToDBCenter( lua_State *L )
	{
		extern std::string sidname;

		if ( g_Script.m_pPlayer == NULL )
			return 0;

		LPCSTR serverip = lua_tostring( L, 1 );
		LPCSTR serverport = lua_tostring( L, 2 );
		LPCSTR password = lua_tostring( L, 3 );

		// DBCENTER 服务器的ID
		DWORD dbcenterId = static_cast< DWORD >( lua_tonumber( L, 4 ) );

		if ( password == NULL || serverip == NULL || serverport == NULL )
			return 0;

		extern BYTE tempTransBuff[0xffff];
		extern CDAppMain *&GetApp();

		if ( GetApp()->szLoginIP == NULL )
			return 0;

		SQGameServerRPCOPMsg &msg = * new ( tempTransBuff ) SQGameServerRPCOPMsg;
		msg.dstDatabase = (BYTE)dbcenterId; // DBCenter

		// 保存数据,取得所有最新的m_Property
		g_Script.m_pPlayer->SendData( SARefreshPlayerMsg::ONLY_BACKUP );

		static SSpanPlayerDataBuf data;
		memset( &data, 0, sizeof( data ) );

		memcpy( ( SFixBaseData* )&data,  ( SFixBaseData* )&g_Script.m_pPlayer->m_Property, sizeof( SFixBaseData ) );
		//        memcpy( ( SPlayerVenapointEx* )&data,  ( SPlayerVenapointEx* )&g_Script.m_pPlayer->m_Property, sizeof( SPlayerVenapointEx ) );
		//        memcpy( ( SPlayerDataExpand* )&data,  ( SPlayerDataExpand* )&g_Script.m_pPlayer->m_Property, sizeof( SPlayerDataExpand ) );

		// 这里这个版本号用于跨服检查
		data.m_version = SSpanPlayerDataBuf::GetVersion();

		//SFixProperty *data = ( SFixProperty * )( &g_Script.m_pPlayer->m_Property );
		size_t dataSize = sizeof( data );

		try
		{
			lite::Serializer sl( msg.streamData, dataSize + sizeof( SQGameServerRPCOPMsg ) );

			// 注册出现异常时返回的数据（集）
			sl [OP_BEGIN_ERRHANDLE] ( 0 ) ( "p_SavePlayerDataDBCenter" ) [OP_END_ERRHANDLE]

			// 准备存储过程
			[OP_PREPARE_STOREDPROC] ( "p_SavePlayerDataDBCenter" )

				// 设定调用参数
				[OP_BEGIN_PARAMS]
			(1) ( g_Script.m_pPlayer->GetAccount() )
				(2) ( g_Script.m_pPlayer->GetName()  )
				(3) ( sidname.c_str() )
				(4) ( password )
				(5) ( &data, dataSize )
				(6) ( GetApp()->szLoginIP )
				(7) ( g_Script.m_pPlayer->m_Property.m_UPassword )
				(8) ( g_Script.m_pPlayer->m_Property.m_GMLevel )
				[OP_END_PARAMS]

			// 调用存储过程
			[OP_CALL_STOREDPROC]

			// 初始化返回数据盒
			[OP_INIT_RETBOX] ( 1024 )
				[OP_BOX_VARIANT] ( ( int )SMessage::EPRO_SCRIPT_MESSAGE )
				[OP_BOX_VARIANT] ( 14 ) // 新的消息类型
				[OP_BOX_VARIANT] ( serverip )
				[OP_BOX_VARIANT] ( serverport )
				[OP_BOX_VARIANT] ( g_Script.m_pPlayer->m_ClientIndex )
				[OP_BOX_PARAM] ( 1 )
				[OP_BOX_PARAM] ( 4 )
				[OP_BOX_VARIANT] ( g_Script.m_pPlayer->GetGID() )
				[OP_BOX_VARIANT] ( g_Script.m_pPlayer->m_Property.m_GMLevel )

				// 返回数据盒
				[OP_RETURN_BOX]

			[OP_RPC_END]; 

			sl.EndEdition();
			SendToLoginServer( &msg, sizeof( SQGameServerRPCOPMsg ) - sizeof( msg.streamData ) + sl.curSize() ); 
		}
		catch ( lite::Xcpt & )
		{
		}

		return 0;
	}

	static int L_GetServerName( lua_State *L )
	{
		extern std::string sidname;
		lua_pushstring( L, sidname.c_str() );
		return 1;
	}

	static int L_OpenVena( lua_State *L )
	{
		if ( g_Script.m_pPlayer == NULL )
			return 0;

		int byVenaNum = static_cast<int>(lua_tonumber(L, 1));
		int byPointNum = static_cast<int>(lua_tonumber(L, 2));

		BOOL result = g_Script.m_pPlayer->OpenVena( NULL, byVenaNum, byPointNum );
		lua_pushnumber( L, result );
		return 1;
	}

	static int L_InputFactionMemo( lua_State *L )
	{
		if ( g_Script.m_pPlayer == NULL )
			return 0;

		if ( g_Script.m_pPlayer->m_Property.m_szTongName[0] == 0 )
			return 0;

		CFactionManager::SendEditFactionMemoMsg( g_Script.m_pPlayer );
		return 0;
	}

	static int L_SetMemberInfo( lua_State *L )
	{
		if ( g_Script.m_pPlayer == NULL )
			return 0;

		if ( g_Script.m_pPlayer->m_Property.m_szTongName[0] == 0 )
			return 0;

		// 如果传入的参数是一个table的话
		if ( !lua_istable( L, 1 ) )
			return 0;

#define CHECK_AND_SET_G( _key, _member ) _IF_GET_FIELD_NUMBER( 1, _key ) \
	g_Script.m_pPlayer->m_stFacRight._member = static_cast< WORD >( __number ); else goto __checkfail;

		CHECK_AND_SET_G( "Donation"  , iDonation )
			CHECK_AND_SET_G( "Accomplishment", dwAccomplishment )

			GetGW()->m_FactionManager.UpdataMemberInfo( g_Script.m_pPlayer );
		return 0;

__checkfail: 
		return rfalse( 2, 1, "SetMemberInfo : 错误的参数" ), 0;  
	}

	// 
	static int L_BindEquip( lua_State *L )
	{
		return 0;
	}

	static int L_GetRegionPlayerCount( lua_State *L )
	{
		DWORD regionId = static_cast< DWORD >( lua_tonumber( L, 1 ) );
		if ( regionId == 0 )
			return 0;

		CRegion *region = reinterpret_cast< CRegion* >( GetRegionById( regionId ) );
		if ( region == NULL )
			return 0;

		lua_pushnumber( L, region->m_PlayerList.size() );
		return 1;
	}

	static int L_ClearPackage( lua_State *L )
	{
		if ( g_Script.m_pPlayer == NULL )
			return 0;

		g_Script.m_pPlayer->ArrangePackage( 0 );
		return 0;
	}


	// 通过该脚本函数，返回一个表，该表中存在的数据为根据道具类型分解出来的所有数据！
	static int L_GetItemScripts( lua_State *L )
	{
		return 0;
	}

	// 通过该函数，根据传入的配置表修改（更新）道具的数据
	// 如果该道具是玩家身上的道具，则同步通知客户端更新数据！
	static int L_UpdateItemScripts( lua_State *L )
	{
		return 0;
	}

	static int L_SetEvent( lua_State *L )
	{
		QWORD PushLuaEvent( lua_State *L );
		QWORD kid = PushLuaEvent( L );
		LPCSTR errstr = NULL;
		switch ( kid )
		{
		case -1: errstr = "SetEvent : removing lua event in event self-execution"; break;
		case -2: errstr = "SetEvent : get error ex parameters"; break;
		case -3: errstr = "SetEvent : tick margin greater than one day in seconds"; break;
		case -4: errstr = "SetEvent : args #1 or #3 is invalid type"; break;
		}

		if ( errstr )
			luaL_error( L, errstr );

		return luaEx_pushint63( L, kid ), 1;
	}

	static int L_ClrEvent( lua_State *L )
	{
		int t1 = lua_type( L, 1 );
		QWORD idk = 0;
		DWORD uid = 0;
		if ( t1 == LUA_TNUMBER )
			uid = (DWORD)lua_tonumber( L, 1 );
		else if ( luaEx_isint63( L, 1 ) )
			idk = luaEx_toint63( L, 1 );

		int ClrEvent( DWORD uid, QWORD *pidk );
		int ck = ClrEvent( uid, ( t1 == LUA_TNUMBER ) ? NULL : &idk );
		if ( ck == -1 )
			luaL_error( L, "ClrEvent : removing lua event in event self-execution" );

		if ( ck <= 0 )
			return 0;

		return lua_pushnumber( L, 1 ), 1;
	}

	static int L_Int63ToStr( lua_State *L )
	{
		if ( !luaEx_isint63( L, 1 ) )
			return 0;

		unsigned __int64 int63 = luaEx_toint63( L, 1 );
		char buf[256];
		sprintf( buf, "%I64X", int63 );
		lua_pushstring( L, buf );
		return 1;
	}

	static int L_StrToInt63( lua_State *L )
	{
		if ( !lua_isstring( L, 1 ) )
			return 0;

		LPCSTR str = lua_tostring( L, 1 );
		luaEx_pushint63( L, Details::HexStringToNumber( str ) );
		return 1;
	}

	static int L_Int63ToNumber( lua_State *L )
	{
		if ( !luaEx_isint63( L, 1 ) )
			return 0;

		unsigned __int64 int63 = luaEx_toint63( L, 1 );
		lua_pushnumber( L, (lua_Number)( __int64 )int63 );
		return 1;
	}

	static int L_NumberToInt63( lua_State *L )
	{
		if ( !lua_isnumber( L, 1 ) )
			return 0;

		// 1000 0000 0000 0000
		// double 的最连续大整数精度到 一千万亿 15位整数 所以需要限制整数数据在这个范围内
		double number = lua_tonumber( L, 1 );
		luaEx_pushint63( L, ( unsigned __int64 )number );
		return 1;
	}

	static int L_CheckPoints( lua_State *L )
	{
		//         if ( g_Script.m_pPlayer == NULL )
		//             return 0;
		// 
		//         // 初始60点
		//         int pointCount = 60;
		// 
		//         // 第一步计算任务点数
		//         int taskPoint = g_Script.m_pPlayer->CheckTaskPoints();
		// 
		//         pointCount += taskPoint;
		// 
		//         // 第二步等级
		//         SLevelData *pLevelData = g_pUpGradeData->GetLevelData( g_Script.m_pPlayer->m_Property.m_School );
		//         if ( pLevelData == NULL )
		//             return 0;
		// 
		//         // - 5 是因为正常升级逻辑里的点数运算存在错误，导致升级经验和加点都是按等级+1来算的
		//         // 所以直接就跳过了等级1->2的过程，所需要将1->2的点数扣除。。。
		//         int levelPoint = pLevelData->Data[g_Script.m_pPlayer->m_byLevel].wPoint - 5;
		// 
		//         pointCount += levelPoint;
		// 
		//         // 第三步
		//         // 小周天属性添加
		//         int puslPoint = g_Script.m_pPlayer->CheckPlusPoints();
		// 
		//         pointCount += puslPoint;
		// 
		//         int oldPoint = g_Script.m_pPlayer->m_Property.m_wIN + 
		//             g_Script.m_pPlayer->m_Property.m_wST + 
		//             g_Script.m_pPlayer->m_Property.m_wEN + 
		//             //g_Script.m_pPlayer->m_Property.m_wAG + 
		//             g_Script.m_pPlayer->m_byPoint;
		// 
		//         lua_pushnumber( L, pointCount );
		//         lua_pushnumber( L, oldPoint );
		// 
		//         return 2;
		return 0;
	}

	static int L_LoadDBTable(lua_State *L)
	{
		STaskExBuffer tpBuffer;
		STaskExBuffer *teBuffer = NULL;
		LPCSTR filename = NULL;

		// 确定操作对象
		if (lua_type(L, 1) == LUA_TNUMBER)
		{
			int player_sid = (int)lua_tonumber(L, 1);
			if (CPlayer *player = (CPlayer*)GetPlayerBySID(player_sid)->DynamicCast(IID_PLAYER))
				teBuffer = &player->taskEx;
		}
		else if (LPCSTR user_key = lua_tostring(L, 1))
		{
			teBuffer = g_Script.LoadTaskEx(user_key);
		}

		// 如果需要从指定文件读取的话
		if ( lua_type( L, 2 ) == LUA_TSTRING )
		{
			filename = lua_tostring( L, 2 );
			HANDLE hFile = CreateFile( filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL );
			if ( hFile == INVALID_HANDLE_VALUE )
				return 0; // 如果没有文件，则返回一个空数据

			DWORD size = GetFileSize( hFile, NULL );
			tpBuffer.saveData = new BYTE[ tpBuffer.saveSize = size ];
			BOOL rt = ReadFile( hFile, tpBuffer.saveData, size, &size, NULL );
			CloseHandle( hFile );

			try
			{
				lite::Serialreader sl( tpBuffer.saveData );
				DWORD s1 = sl.maxSize();
				if ( s1 > size )
					return 0;

				if ( size > s1 )
				{
					lite::Serialreader sl2( tpBuffer.tempData );
					DWORD s2 = sl2.maxSize();
					if ( s1 + s2 != size )
						return 0;
					tpBuffer.tempData = ( LPBYTE )tpBuffer.saveData + s1;
				}
			}
			catch ( lite::Xcpt& )
			{
				return 0;
			}

			teBuffer = &tpBuffer;
		}

		if ( teBuffer == NULL )
			return 0;

		if ( teBuffer->flags & STaskExBuffer::ERROR_IN_SERIAL )
			luaL_error( L, "deserial op get bad buffer!" );

		// 先标记 flag 为异常，整个保存过程完成了后再恢复
		// 避免操作过程中出现错误导致数据保存不了，而产生更大的逻辑错误！！！
		teBuffer->flags |= STaskExBuffer::ERROR_IN_SERIAL;

		int top = lua_gettop( L );

		// 先检测管理器中是否存在该对象！
		lua_getglobal( L, "dbMgr" );
		if  ( !lua_istable( L, -1 ) )
			goto __error_exit;

		lua_pushvalue( L, 1 );
		lua_rawget( L, -2 ); // key删除，被域结果替代
		if ( lua_type( L, -1 ) > LUA_TNIL )
			goto __error_exit;

		lua_pop( L, 2 ); // 将指定域结果和dbMgr退栈
		assert( lua_gettop( L ) == top );

		lua_createtable( L, 0, 0 ); // 然后创建表，最后将最后的表设置为子表
		if ( !lua_istable( L, -1 ) )
			goto __error_exit;

		// 每个数据表都该分为2个部分，临时的和需要保存的
		if ( teBuffer->saveData )
		{
			//lua_createtable( L, 0, 0 ); // 创建data子表
			//if ( !lua_istable( L, -1 ) )
			//    goto __error_exit;

			// 如果是空数据，或者以前老的liteserail逻辑数据，则直接返回一个空表！
			if ( *( LPDWORD )teBuffer->saveData == 0 || *( LPDWORD )teBuffer->saveData == 0x00040004 )
				lua_createtable( L, 0, 0 ); // 创建data子表
			else {
				int ck = luaEx_unserialize( L, ( LPBYTE )teBuffer->saveData + 4, *( LPDWORD )teBuffer->saveData );
				if ( ck <= 0 || ck != *( LPDWORD )teBuffer->saveData )
					goto __error_exit;
			}
			lua_setfield( L, -2, "data" );
		}

		if ( teBuffer->tempData )
		{
			//lua_createtable( L, 0, 0 ); // 创建temp子表
			//if ( !lua_istable( L, -1 ) )
			//    goto __error_exit;

			if ( *( LPDWORD )teBuffer->tempData == 0 || *( LPDWORD )teBuffer->tempData == 0x00040004 )
				lua_createtable( L, 0, 0 ); // 创建data子表
			else {
				int ck = luaEx_unserialize( L, ( LPBYTE )teBuffer->tempData + 4, *( LPDWORD )teBuffer->tempData );
				if ( ck <= 0 || ck != *( LPDWORD )teBuffer->tempData )
					goto __error_exit;
			}

			lua_setfield( L, -2, "temp" );
		}

		teBuffer->flags &= ~STaskExBuffer::ERROR_IN_SERIAL;
		assert( lua_gettop( L ) == top + 1 );
		return 1;

__error_exit:
		if ( lua_gettop( L ) != top )
			lua_settop( L, top );

		return 0;
	}

	static int L_SaveDBTable( lua_State *L )
	{
		STaskExBuffer tpBuffer;
		STaskExBuffer *teBuffer = NULL;
		LPCSTR filename = NULL;

		// 确定操作对象！
		if ( lua_type( L, 1 ) == LUA_TNUMBER )
		{
			int player_sid = ( int )lua_tonumber( L, 1 );
			if ( CPlayer *player = ( CPlayer* )GetPlayerBySID( player_sid )->DynamicCast( IID_PLAYER ) )
				teBuffer = &player->taskEx;
		}
		else if ( LPCSTR user_key = lua_tostring( L, 1 ) )
		{
			teBuffer = g_Script.LoadTaskEx( user_key );
		}

		// 如果需要写入到文件的话
		if ( lua_type( L, 2 ) == LUA_TSTRING )
		{
			filename = lua_tostring( L, 2 );
			teBuffer = &tpBuffer;
		}

		// 是否在更新后，清除lua数据
		BOOL doClean = ( lua_tonumber( L, 3 ) == 1 );

		if ( teBuffer == NULL )
			return 0;

		if ( teBuffer->flags & STaskExBuffer::ERROR_IN_SERIAL )
			luaL_error( L, "serial op get bad buffer!" );

		// 先标记 flag 为异常，整个保存过程完成了后再恢复
		// 避免操作过程中出现错误导致数据保存不了，而产生更大的逻辑错误！！！
		teBuffer->flags |= STaskExBuffer::ERROR_IN_SERIAL;

		int top = lua_gettop( L );

		// 获取管理器
		lua_getglobal( L, "dbMgr" );
		if  ( lua_istable( L, -1 ) )
		{
			// 获取指定数据表
			lua_pushvalue( L, 1 );
			lua_rawget( L, -2 ); // key删除，被域结果替代
			BOOL isTable = lua_istable( L, -1 );
			if ( isTable )
			{
				// 分别确定可保存的数据大小
				size_t size1 = ( teBuffer->flags & STaskExBuffer::SIZE1_FIXED ) ? 
					teBuffer->saveSize : sizeof( sdbBuffer );
				size_t size2 = ( teBuffer->flags & STaskExBuffer::SIZE2_FIXED ) ? 
					teBuffer->tempSize : sizeof( tdbBuffer );

				int prev1 = 0;
				int prev2 = 0; 
				int cur1  = 0;
				int cur2  = 0; 

				// 每个数据表都该分为2个部分，临时的和需要保存的
				// 如果有数据，则进行数据保存！
				// ###########################################################################################
				// 注意，这里需要考虑到特殊情况，也就是如果保存过程中存在数据错误，那么原始数据也都会出错！！！
				// 所以后面替换为了，先序列化到临时空间，然后再拷贝到实际数据区
				// ###########################################################################################
				// 现在lua的jex库原生支持serialize
				lua_getfield( L, -1, "data" );
				if ( lua_istable( L, -1 ) )
				{
					if ( teBuffer->saveData != NULL ) {
						prev1 = *( LPDWORD )teBuffer->saveData;
						if ( HIWORD(prev1) ) // 容错处理，避免以前的老序列化数据0x00040004导致错误
							prev1 = 0;
					}
					cur1 = luaEx_serialize( L, -1, sdbBuffer + 4, (int)size1 - 4 );
					if ( cur1 < 0 )
						goto __error_exit;
					*( LPDWORD )sdbBuffer = cur1;
					cur1 += 4;
				}
				lua_pop( L, 1 );

				// 如果有数据，则进行数据保存！
				lua_getfield( L, -1, "temp" );
				if ( lua_istable( L, -1 ) )
				{
					if ( teBuffer->tempData != NULL ) {
						prev2 = *( LPDWORD )teBuffer->tempData;
						if ( HIWORD(prev2) ) // 容错处理，避免以前的老序列化数据0x00040004导致错误
							prev2 = 0;
					}
					cur2 = luaEx_serialize( L, -1, tdbBuffer + 4, (int)size2 - 4 );
					if ( cur2 < 0 )
						goto __error_exit;
					*( LPDWORD )tdbBuffer = cur2;
					cur2 += 4;
				}
				lua_pop( L, 1 );

				if ( filename == NULL )
				{
					// 序列化后的数据大小可能导致数据空间变化
					if ( cur1 > teBuffer->saveSize )
					{
						delete teBuffer->saveData;
						prev1 = cur1 + ( cur1 >> 3 );
						if ( prev1 & 0xf ) // 16字节对齐
							prev1 = ( prev1 & ~0xf ) + 0x10;
						teBuffer->saveSize = prev1;
						teBuffer->saveData = new char[ prev1 ];
					}

					// 序列化后的数据赋值
					if ( teBuffer->saveData )
					{
						memcpy( teBuffer->saveData, sdbBuffer, cur1 );
						if ( cur1 < prev1 ) // 在新数据小于原有数据大小的情况下，清空原有的脏数据
							memset( ( LPBYTE )teBuffer->saveData + cur1, 0, prev1 - cur1 );
					}

					// 序列化后的数据大小可能导致数据空间变化
					if ( cur2 > teBuffer->tempSize )
					{
						delete teBuffer->tempData;
						prev2 = cur2 + ( cur2 >> 3 );
						if ( prev2 & 0xf ) // 16字节对齐
							prev2 = ( prev2 & ~0xf ) + 0x10;
						teBuffer->tempSize = prev2;
						teBuffer->tempData = new char[ prev2 ];
					}

					// 序列化后的数据赋值
					if ( teBuffer->tempData )
					{
						memcpy( teBuffer->tempData, tdbBuffer, cur2 );
						if ( cur2 < prev2 ) // 在新数据小于原有数据大小的情况下，清空原有的脏数据
							memset( ( LPBYTE )teBuffer->tempData + cur2, 0, prev2 - cur2 );
					}
				}
				else
				{
					HANDLE hFile = CreateFile( filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL );
					if ( hFile == INVALID_HANDLE_VALUE )
						goto __error_exit;

					DWORD s1 = 0, s2 = 0;
					BOOL rt = WriteFile( hFile, sdbBuffer, cur1, &s1, NULL ) &&
						WriteFile( hFile, tdbBuffer, cur2, &s2, NULL );
					CloseHandle( hFile );

					if ( !rt ) goto __error_exit;
				}
			}
			lua_pop( L, 1 );

			teBuffer->flags &= ~STaskExBuffer::ERROR_IN_SERIAL;

			// 是否在成功操作后，进行数据清除，这个功能主要用在角色数据上，因为角色下线后，是不应该对其数据进行访问的。。
			if ( doClean && isTable )
			{
				lua_pushvalue( L, 1 );
				lua_pushnil( L );
				lua_rawset( L, -3 );
			}
		}
		else
		{
			// 清除数据，避免由于数据表管理器错误导致保存数据异常。。。
			teBuffer->flags &= ~STaskExBuffer::ERROR_IN_SERIAL;
		}
		lua_pop( L, 1 );

		assert( lua_gettop( L ) == top );

__error_exit:
		if ( lua_gettop( L ) != top )
			lua_settop( L, top );

		// 是否有错误标志
		if ( teBuffer->flags & STaskExBuffer::ERROR_IN_SERIAL )
			return 0;

		lua_pushboolean( L, true );
		return 1;
	}

	// added by xhy,copy from L_SaveDBTable
	// TODO:目前只把所有任务都发送过去，以后需要修改只发送发生改变的数据
	static int L_SyncTaskData(lua_State* L)
	{
		STaskExBuffer tpBuffer;
		STaskExBuffer *teBuffer = NULL;


		// 确定操作对象！
		int player_sid = 0;
		DNID client_dnid = 0;
		if ( lua_type( L, 1 ) == LUA_TNONE )
		{
			if ( g_Script.m_pPlayer == NULL )
				return 0;

			player_sid = g_Script.m_pPlayer->GetSID();
			client_dnid = g_Script.m_pPlayer->m_ClientIndex;
			teBuffer = &( g_Script.m_pPlayer->taskEx );
		}
		//     else if ( lua_type( L, 1 ) == LUA_TNUMBER )
		//     {
		//         player_sid = ( int )lua_tonumber( L, 1 );
		//         if ( CPlayer *player = ( CPlayer* )GetPlayerBySID( player_sid )->DynamicCast( IID_PLAYER ) )
		//{
		//	client_dnid = player->m_ClientIndex;
		//             teBuffer = &player->taskEx;
		//}
		//     }
		else if ( lua_type( L, 1 ) == LUA_TNUMBER )
		{
			player_sid = ( int )lua_tonumber( L, 1 );
			if ( CPlayer *player = ( CPlayer* )GetPlayerBySID( player_sid )->DynamicCast( IID_PLAYER ) )
			{
				client_dnid = player->m_ClientIndex;
				teBuffer = &player->taskEx;
			}
		}
		else if ( LPCSTR user_key = lua_tostring( L, 1 ) )
		{
			teBuffer = g_Script.LoadTaskEx( user_key );
		}

		if ( player_sid == 0 )
			return 0;

		// 是否在更新后，清除lua数据
		BOOL doClean = ( lua_toboolean( L, 3 ) == 1 );

		if ( teBuffer == NULL )
			return 0;

		if ( teBuffer->flags & STaskExBuffer::ERROR_IN_SERIAL )
			luaL_error( L, "serial op get bad buffer!" );

		// 先标记 flag 为异常，整个保存过程完成了后再恢复
		// 避免操作过程中出现错误导致数据保存不了，而产生更大的逻辑错误！！！
		teBuffer->flags |= STaskExBuffer::ERROR_IN_SERIAL;

		int top = lua_gettop( L );

		// 获取管理器
		lua_getglobal( L, "dbMgr" );
		if  ( lua_istable( L, -1 ) )
		{
			// 获取指定数据表
			lua_pushnumber( L, player_sid );
			lua_rawget( L, -2 ); // key删除，被域结果替代
			BOOL isTable = lua_istable( L, -1 );
			if ( isTable )
			{
				// 分别确定可保存的数据大小
				size_t size1 = ( teBuffer->flags & STaskExBuffer::SIZE1_FIXED ) ? 
					teBuffer->saveSize : sizeof( sdbBuffer );

				int prev1 = 0;
				int cur1  = 0;

				// 每个数据表都该分为2个部分，临时的和需要保存的
				// 如果有数据，则进行数据保存！
				// ###########################################################################################
				// 注意，这里需要考虑到特殊情况，也就是如果保存过程中存在数据错误，那么原始数据也都会出错！！！
				// 所以后面替换为了，先序列化到临时空间，然后再拷贝到实际数据区
				// ###########################################################################################
				// 现在lua的jex库原生支持serialize
				lua_getfield( L, -1, "data" );
				if ( lua_istable( L, -1 ) )
				{
					if ( teBuffer->saveData != NULL ) {
						prev1 = *( LPDWORD )teBuffer->saveData;
						if ( HIWORD(prev1) ) // 容错处理，避免以前的老序列化数据0x00040004导致错误
							prev1 = 0;
					}
					cur1 = luaEx_serialize( L, -1, sdbBuffer + 4, (int)size1 - 4 );
					if ( cur1 < 0 )
						goto __error_exit;
					*( LPDWORD )sdbBuffer = cur1;
					cur1 += 4;
				}
				lua_pop( L, 1 );


				// 序列化后的数据大小可能导致数据空间变化
				if ( cur1 > teBuffer->saveSize )
				{
					delete teBuffer->saveData;
					prev1 = cur1 + ( cur1 >> 3 );
					if ( prev1 & 0xf ) // 16字节对齐
						prev1 = ( prev1 & ~0xf ) + 0x10;
					teBuffer->saveSize = prev1;
					teBuffer->saveData = new char[ prev1 ];
				}

				// 序列化后的数据赋值
				if ( teBuffer->saveData )
				{
					memcpy( teBuffer->saveData, sdbBuffer, cur1 );
					if ( cur1 < prev1 ) // 在新数据小于原有数据大小的情况下，清空原有的脏数据
						memset( ( LPBYTE )teBuffer->saveData + cur1, 0, prev1 - cur1 );
				}

			}
			lua_pop( L, 1 );

			teBuffer->flags &= ~STaskExBuffer::ERROR_IN_SERIAL;

			// 是否在成功操作后，进行数据清除，这个功能主要用在角色数据上，因为角色下线后，是不应该对其数据进行访问的。。
			if ( doClean && isTable )
			{
				lua_pushvalue( L, 1 );
				lua_pushnil( L );
				lua_rawset( L, -3 );
			}
		}
		else
		{
			// 清除数据，避免由于数据表管理器错误导致保存数据异常。。。
			teBuffer->flags &= ~STaskExBuffer::ERROR_IN_SERIAL;
		}
		lua_pop( L, 1 );

		assert( lua_gettop( L ) == top );

		// 发送数据到客户端(teBuffer->saveData,teBuffer->saveSize)
		{
			SQASyncTaskData msg;
			memset(msg.streamData,0,sizeof(msg.streamData));
			//lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
			//slm(teBuffer->saveData,teBuffer->saveSize);
			if( sizeof(msg.streamData) > teBuffer->saveSize )
			{
				memcpy(msg.streamData,teBuffer->saveData,teBuffer->saveSize);
				g_StoreMessage(client_dnid, &msg, sizeof(msg));
			}
		}

__error_exit:
		if ( lua_gettop( L ) != top )
			lua_settop( L, top );

		// 是否有错误标志
		if ( teBuffer->flags & STaskExBuffer::ERROR_IN_SERIAL )
			return 0;

		lua_pushboolean( L, true );
		return 1;
	}

	static int L_SetRegionBatchrRefresh( lua_State *L )
	{
		DWORD dwRegionId = (DWORD)lua_tonumber( L , 1 );
		if ( dwRegionId == 0 )
			return 0;

		CRegion *destRegion = GetRegionById(dwRegionId);
		if ( destRegion == NULL )
			return 0;

		DWORD dwBRTeamNumber = (DWORD)lua_tonumber( L , 2 );
		if ( dwBRTeamNumber == 0 )
			return 0;

		WORD wRefreshStyle = (WORD)lua_tonumber( L , 3 );       

		destRegion->SetRegionBRType( dwBRTeamNumber, wRefreshStyle );

		return 0;
	}

	/// 关联杀怪任务中的怪物
	/// 参数: 任务id 怪物名字 关联还是移除关联
	static int L_MarkTaskKillMonster( lua_State *L )
	{
		if ( g_Script.m_pPlayer == NULL ||
			lua_type( L, 1 ) != LUA_TNUMBER || 
			lua_type( L, 2 ) != LUA_TSTRING ||
			lua_type( L, 3 ) != LUA_TBOOLEAN )
			return 0;

		DWORD dwTaskId = (DWORD)lua_tonumber( L , 1 );
		const char* monsterName = lua_tostring( L, 2 );
		BYTE isAdd = (BYTE)lua_toboolean( L, 3 );
		if ( dwTaskId == 0 || monsterName == NULL )
			return 0;

		std::map< string, std::list<DWORD> >::iterator i = g_Script.m_pPlayer->taskKillMonsters.find( monsterName );
		if ( i != g_Script.m_pPlayer->taskKillMonsters.end() ) {
			for ( std::list<DWORD>::iterator it = i->second.begin(); it != i->second.end(); it++ ) {
				if ( *it == dwTaskId ) {
					if ( isAdd ) 
						return 0;   // 如果是重复添加，则失败返回
					i->second.erase( it );
					lua_pushboolean( L, true );
					return 1;       // 如果是删除，则返回成功
				}
			}
		}

		if ( !isAdd ) 
			return 0; // 如果是删除，则失败返回
		g_Script.m_pPlayer->taskKillMonsters[monsterName].push_back( dwTaskId );
		lua_pushboolean( L, true );
		return 1;       // 如果是添加，则返回成功
	}

	/// 根据怪物Id获取怪物的名字
	static int L_GetMonsterNameById( lua_State *L )
	{
		// 		DWORD monsterId = ( DWORD )lua_tonumber( L, 1 );
		// 		if ( monsterId < 1 )
		// 			return 0;
		//         const SMonsterData *monsterData = CMonster::GetMonsterData( monsterId-1 );
		// 		if ( monsterData == NULL )
		// 			return 0;
		// 		//if ( strlen(monsterData->m_szName) == 0 || strlen(monsterData->m_szName) > 10 )
		// 		//	lua_pushstring( L, "" );
		// 		//else
		// 		{
		// 			lua_pushstring( L, monsterData->name );
		// 		}

		return 0;
	}

	static int L_QualifyEquipAttNum(lua_State *L)
	{
		extern WORD GenerateEquipAttNum(WORD levle);
		extern WORD GetEquipRank(WORD level);

		WORD level = (WORD)lua_tonumber(L, 1);
		rfalse("装备等级为%d", level);

		WORD maxAttribute = GenerateEquipAttNum(level);
		lua_pushnumber(L, maxAttribute);
		return 1;
	}

	static int L_GenerateEquipAAttType(lua_State *L)
	{
		extern WORD GenerateEquipAttNum(WORD levle);
		extern void GenerateEquipAttType(int *attType,int *curattType);
		extern void GenerateEquipAttValue(int *attValue,int attType);

		WORD maxAttribute = (WORD)lua_tonumber(L, 1);

		extern bool g_isSelected[11];

		int stackPos = lua_gettop( L );
		lua_newtable(L);

		int curattType = 0,attValue = 0,attType;
		memset(&g_isSelected,0,sizeof(g_isSelected));

		int count = 1;
		for ( int k = 0; k < maxAttribute; k++ ) 
		{
			GenerateEquipAttType(&attType,&curattType);		// 属性类型
			GenerateEquipAttValue(&attValue,curattType);	// 属性具体值

			lua_pushnumber(L, count++);
			lua_pushnumber(L, attType);
			lua_settable( L, -3 );

			lua_pushnumber(L, count++);
			lua_pushnumber(L, attValue);
			lua_settable( L, -3 );
		}
		return 1;
	}

	static int L_JewelUpdateShow(lua_State *L)
	{
		return 0;
	}
	static int L_ActiveNoticeBroadcast(lua_State *L)
	{
		LPCSTR msg = static_cast<const char *>(lua_tostring(L, 1));
		if ( msg == NULL )
			msg = "(null)";
		SAChatBroadcastMsg aMsg;
		memcpy(aMsg.BroadcasInfo,msg,256);
		extern void BroadcastSysNoticeMsg(SMessage *aMsg);
		BroadcastSysNoticeMsg(&aMsg);
		return 1;
	}
};

namespace ExtraScriptFunctions 
{ 
	void RegisterScriptFunctions1();
	void RegisterScriptFunctions_EventRegion();

	void RegisterScriptFunctions()
	{

#define RegisterFunction( _funcname ) g_Script.RegisterFunction( #_funcname, ExtraLuaFunctions::L_##_funcname );
		RegisterFunction( JewelUpdateShow			)
			RegisterFunction( GenerateEquipAAttType		);
		RegisterFunction( QualifyEquipAttNum		);
		RegisterFunction( OpenScoreList             );
		RegisterFunction( WndCustomize              );
		RegisterFunction( UpdateCustomWnd           );
		RegisterFunction( CustomWndClose );

		RegisterFunction( ATM_DecreasePoint         );
		RegisterFunction( ATM_QueryRemainPoints     );
		RegisterFunction( GetRemainPoint            );
		RegisterFunction( CancelWaitingState        );

		RegisterFunction( SetRandomAbilityTable     );

		RegisterFunction( GetItemSetting            );
		RegisterFunction( UpdateItemSetting         );
		RegisterFunction( GetItemDetails            );
		RegisterFunction( UpdateItemDetails         );
		RegisterFunction( GenerateItemDetails       );

		RegisterFunction( ModifyItem                );
		RegisterFunction( CreateObjectIndirect      );
		RegisterFunction( RemoveObjectIndirect      );
		RegisterFunction( CreateFolder              );
		RegisterFunction( DBRPC                     );

		RegisterFunction( GetObjectUniqueId         );

		RegisterFunction( CheckItemCount            );
		RegisterFunction( DropItems                 );
		RegisterFunction( SetHateMonster            );
		RegisterFunction( UpdateMonsterData         );
		RegisterFunction( TalkBubble                );
		RegisterFunction( SelectObject              );
		RegisterFunction( SavePlayerDataToDBCenter  );
		RegisterFunction( GameRecords               );
		RegisterFunction( SetTempStatus             );
		RegisterFunction( GetPlayerSIDNAME          );
		RegisterFunction( GetServerName             );
		RegisterFunction( OpenVena                  );
		RegisterFunction( InputFactionMemo          );
		RegisterFunction( SetMemberInfo				);
		RegisterFunction( BindEquip               );
		RegisterFunction( GetRegionPlayerCount		);
		RegisterFunction( ClearPackage		        );

		RegisterFunction( GetItemScripts            );
		RegisterFunction( UpdateItemScripts         );

		RegisterFunction( SetEvent                  );
		RegisterFunction( ClrEvent                  );

		RegisterFunction( Int63ToStr                );
		RegisterFunction( StrToInt63                );
		RegisterFunction( Int63ToNumber             );
		RegisterFunction( NumberToInt63             );
		RegisterFunction( CheckPoints               );

		RegisterFunction( LoadDBTable               );  // 从指定缓存或文件读取数据，返回数据表或nil
		RegisterFunction( SaveDBTable               );  // 将指定数据表保存到指定缓存或文件，返回操作结果
		RegisterFunction( SetRegionBatchrRefresh    );  // 设置某区域批量刷怪功能 关闭，恢复，关闭批量重生

		RegisterFunction( SyncTaskData              );  // 同步任务数据到客户端,added by xhy

		RegisterFunction( MarkTaskKillMonster		);	// 关联杀怪任务的怪物
		RegisterFunction( GetMonsterNameById		);	// 根据怪物id获取怪物名字
		RegisterFunction( ActiveNoticeBroadcast		);
#undef  RegisterFunction

		RegisterScriptFunctions1();
		RegisterScriptFunctions_EventRegion();
	}
}


