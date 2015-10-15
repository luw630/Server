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
#include "���������/dmainapp.h"
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

_DeclareXcpt( InvalidArray,         0x00000008, "��������л�����" )
	_DeclareXcpt( CreateTableFailure,   0x00000009, "����lua��ʧ�ܣ�" )
	_DeclareXcpt( PushVariantFailure,   0x0000000a, "�����л�����ѹ��luaջʧ�ܣ�" )
	_DeclareXcpt( StackBreak,           0x0000000b, "lua�ڷ����л������б��ƻ��ˣ�" )

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

	// �����в����������ջ
	lua_createtable( g_Script.ls, 0, 0 );
	if ( !lua_istable( g_Script.ls, -1 ) )
		throw lite::CreateTableFailure();

	int count = 1;
	while ( slr.curSize() < slr.maxSize() )
	{
		lite::Variant lvt = slr();
		if ( lite::Variant::VT_RPC_OP == lvt.dataType )
		{
			// ˵�����ص���һ������
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

	// �������ã�
	assert( top + 1 == lua_gettop( g_Script.ls ) );
	if ( top + 1 != lua_gettop( g_Script.ls ) )
		throw lite::StackBreak();

	prepared_paramnum = 1;
}

void CScriptManager::PushVariantArray( lite::Serialreader &slr )
{
	// ע�⣬Ŀǰ���ⲿ����ֻ��һ���ط������������Ѿ�����lite::Xcpt���쳣����
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
	// ע�⣬Ŀǰ���ⲿ����ֻ��һ���ط������������Ѿ�����lite::Xcpt���쳣����
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

		// 2ά���
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

		// 1ά���
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
	// �ڱ���ǰ����Ҫ�����ȴӽű������һ�Σ�����
	lite::Variant lvt;
	LuaFunctor( g_Script, "SaveDBTable" )[ it->first.c_str() ]( &lvt );
	if ( lvt.dataType <= lite::Variant::VT_NULL )
		return; // �������ʧ�ܣ�ȡ�����棡����

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
		return; // ���ʧ��ֱ�ӷ���

	DWORD size = 0;
	BOOL rt = WriteFile( hFile, teb->saveData, saveSize, &size, NULL );
	CloseHandle( hFile );
}

void CScriptManager::SaveTaskEx( LPCSTR key )
{
	CreateDirectory( "dbTables", NULL );
	if ( key != NULL )
	{
		// �Ȳ����Ƿ��Ѿ����棬���������ˣ���ֱ�Ӷ�ȡ�������ȥ�Ҷ�Ӧ�������ļ�
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
	// �Ȳ����Ƿ��Ѿ����棬���������ˣ���ֱ�Ӷ�ȡ�������ȥ�Ҷ�Ӧ�������ļ�
	std::map< std::string, STaskExBuffer >::iterator it = taskExMgr.find( key );
	if ( it == taskExMgr.end() )
	{
		STaskExBuffer *teb = &taskExMgr[ key ];
		char filename[256];
		sprintf( filename, "dbTables/%s.dbt", key );
		HANDLE hFile = CreateFile( filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL );
		if ( hFile == INVALID_HANDLE_VALUE )
			return teb; // ���û���ļ����򷵻�һ��������

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
		throw exception( "�����lua���ý����ջ�����쳣��" );

	if ( ( result ) && ( resultSize == 1 ) )
		reinterpret_cast< lite::lua_variant& >( *result ).get( callStack, -1 );

	lua_settop( callStack, stackTop );

	return TRUE;
}

// ��̬����һ������, ����Ҫʹ�õ�����صĲ���, ���Խ������ƶ����������
int CScriptManager::L_CreateRegion(lua_State *L)
{
	DWORD dwGID = 0;

	// �������Ĳ�����һ��table�Ļ���˵��Ҫʹ���µĳ��������߼���
	if (lua_istable(L, 1))
	{
#define CHECK_AND_SET( _key, _member ) _IF_GET_FIELD_NUMBER( 1, _key ) \
	param._member = static_cast< WORD >( __number ); else goto __checkfail;

		CFactionRegion::CreateParameter param;
		// �����ʶ��������ֵĻ�����ʾ���������Ҫָ������ģ���ô�����Լ���ǰ����Ļ����ͷ��ظ�0������
		DWORD zoneId = 0;
		DWORD needLevel = 0;
		_IF_GET_FIELD_NUMBER( 1, "zoneId" ) 
		{
			extern BYTE GetZoneID();
			zoneId = static_cast< DWORD >( __number );
			if ( zoneId != GetZoneID() ) 
				return 0;
		}

		// �����ʶ��������ֵĻ���˵���ǹ̶�������ֱ�Ӷ�ȡ������
		_IF_GET_FIELD_NUMBER( 1, "staticId" ) 
		{
			DWORD staticId = static_cast< DWORD >( __number );


			if ( FindRegionByID( staticId ) )
				return lua_pushnumber( L, 0 ), 1; // Ŀ�곡���Ѿ����ڣ�ʧ�ܷ��ص�����Ҫ����ʾ��

			DWORD type = (DWORD)_GET_FIELD_NUMBER_DIRECTLY( 1, "type" );
			if ( type > 5 )
				return rfalse( 2, 1, "�����������ʹ���[%d]", type ), 0;

			static DWORD REGIONTYPE[5] = { IID_REGION, IID_FIGHTARENA, IID_PRISON, IID_PRISONEX, IID_BIGUANAREA };

			LPIObject region = GetApp()->m_pGameWorld->CRegionManager::GenerateObject( REGIONTYPE[type], 0, staticId );
			if ( !region )
				return rfalse( 2, 1, "�޷�װ��Ŀ�곡�� [%d:%d]", staticId, type ), 0;

			// ���½��������������, ��ȡ��ǰ���������п��ܴ��ڵĽ�����!
// 			if ( type == 0 ) {
// 				SQBuildingGetMsg msg;
// 				msg.wParentID = (WORD)staticId;
// 				SendToLoginServer( &msg, sizeof( msg ) );
// 			}

			CRegion *ptr = ( CRegion* )region->DynamicCast( IID_REGION );
			if ( ptr == NULL )
				return rfalse( 2, 1, "�����Ŀ�곡��ָ�� [%d]", staticId ), 0;

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
	CHECK_AND_SET( "regionId"  , factionRegionId )  // ��ǰ������ΨһID���
		CHECK_AND_SET( "templateId", mapTemplateId   )  // ��ǰ������ģ�峡�����
		CHECK_AND_SET( "parentId"  , parentRegionId  )  // ��ǰ�����ĳ���ͨ���Ŀ�곡��������
		CHECK_AND_SET( "xEntry"    , xEntry          )  
		CHECK_AND_SET( "yEntry"    , yEntry          )  
		_IF_GET_FIELD_STRING( 1, "owner" ) param.name = __string; // �ó�����ӵ���ߵģ����ɣ����ƣ�����
else { __checkfail: return rfalse( 2, 1, "CreateRegion : ����Ĳ���" ), 0; }  

if ( !GetApp()->m_pGameWorld->CRegionManager::GenerateObject( IID_FACTIONREGION, 0, ( LPARAM )&param ) )
	return rfalse( 2, 1, "�޷�װ��Ŀ�곡�� [%d][%s]", param.mapTemplateId, param.name.c_str() ), 0;
}
 
// ����������ɹ�,�ͽ���Щ���ɳ����ı��֪ͨ����½��������
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

		WORD	RegionIndex		= static_cast<WORD>	( lua_tonumber( L, 1 ) );//20000 lua���
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

	DWORD giftid = static_cast<DWORD>(lua_tonumber(L, 1)); //���ID
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
						/* 'key' (����-2) �� 'value' (����-1) */  
						const char* key = "unknown";  
						const char* value;  
						if(lua_type(L, -2) == LUA_TSTRING)  
						{  
							key = lua_tostring(L, -2);  
							value = lua_tostring(L, -1);  
						}  
						else if(lua_type(L, -2) == LUA_TNUMBER)  
						{  
							// ��Ϊlua_tostring�����ջ�ϵ�Ԫ�أ�  
							// ���Բ���ֱ����key�Ͻ���lua_tostring  
							// ��ˣ�����һ��key��ѹ��ջ��������lua_tostring  
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
						/* �Ƴ� 'value' ������ 'key' ����һ�ε��� */  
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
		AddInfo("��Ҫ��ӵ�����е���Ʒһ����û��");
		return 0;
	}
	// ��������һ���������

	const SItemBaseData *itemdata = CItemService::GetInstance().GetItemBaseData(giftid);
	if(!itemdata)
	{
		AddInfo(FormatString("û���ҵ��������%d,���ɵ���ʧ��",giftid));
		return 0;
	}

	// 	WORD pos = g_Script.m_pPlayer->FindBlankPos(XYD_PT_BASE);
	// 	if (0xffff == pos)
	// 	{
	// 		TalkToDnid(g_Script.m_pPlayer->m_ClientIndex,"�����������޷���ȡ");
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

// ��Ϊ�ṹ����������Ԫ�������޷������ֿռ�������
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

	// �򿪣��£��������а����
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
					// �ж��ӱ��ÿһ��Ԫ�ض����ַ�����
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
			// ֻ��������жϵĴ���
		}

		return 0;
	}

	static BOOL traverseWndTable( lua_State *L, lite::Serializer &slm, int wndTableOffset)
	{
#define PUSH_LUATABLE( _index ) {               \
	lite::table_iterator temp( L, _index );     \
	if ( !temp.isValid() ) return FALSE;            \
	table_stack.push( temp );           }

		// ��Ϊ�漰��������Ƕ�ף�����ʹ��һ��stack������
		std::stack< lite::table_iterator > table_stack;
		PUSH_LUATABLE( 1 + wndTableOffset )

			while ( !table_stack.empty() )
			{
				lite::table_iterator &table = table_stack.top();

				if ( !( table ++ ) )
				{
					// ����������ջ��
					slm[ WND_CUSTOM_OP::OP_END ];
					table_stack.pop();
					continue;
				}

				//// ���ÿһ��Ԫ�����Ͷ�Ӧ�����ַ�����
				//if ( lua_type( L, table.index_of_key ) != LUA_TSTRING )
				//    return FALSE;

				// �жϱ��ÿһ��Ԫ��ֵ����
				if ( lua_type( L, table.index_of_value ) == LUA_TTABLE )
				{
					// ���ؼ�������ʼ���Ϳؼ�������ջ��
					slm[ WND_CUSTOM_OP::OP_OCX ]( lua_tostring( L, table.index_of_key ) );

					// ���ֵ������table����ö��������һ���ؼ�
					PUSH_LUATABLE( table.index_of_value )
						continue;
				}

				// �����ͺ�ֵ��ջ��
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
		//         // ���Խ��
		//         if ( ( DWORD )type >= 4 )
		//             return 0;
		// 
		//         // ���ض�Ӧ���͵�ֵ
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
		// ���������Ҫ���ڣ�������������½����������۳�ָ�������ģ�Ԫ����ͨ�����������ȿ��ܴ��ڵĻ������ͣ���
		// ��һ������Ϊ ��������
		// �ڶ�������Ϊ �۳����������ض�Ϊ�������Ҳ���۳�����1ǧ����ң���10��Ԫ����
		// ����������Ϊ д��LOG�ģ����ڲ�ѯ�ģ���ֵ������ֵ
		// ���ĸ�����Ϊ д��LOG�ģ����ڲ�ѯ�ģ���ϸ˵��������
		// ���������Ϊ �۳���ɺ�������Ӧ����Ļص�����      ʹ��ATM_temporaryScriptVariant��ʱ����[ջ��]
		// �����Ĳ���Ϊ ָ���ص������ڵ���ʱ��ǰ���Ĭ�ϲ���    ʹ��ATM_temporaryScriptVariant��ʱ����[ջ��]
		// #####################################################################################
		// ע�⣺
		// ���ص�����������ʱ�����еĵ�һ�������ض��ǿ۵�������ؽ�����ɹ���Ϊ1����
		if ( g_Script.m_pPlayer == NULL )
			return 0;

		// ��ͨ��������������жϣ���߿��ܴ��ڳ�ʱ������
		if ( g_Script.m_pPlayer->isWaitingLockedResult( FALSE, TRUE ) )
			return 0;

		// �˲�����ͨ������ATM_temporaryScriptVariant.empty()���п���
		// �����һ������δ���ǰ��������ATM_Decrement�򲻻ᱻ�ٴδ���������
		if ( !g_Script.m_pPlayer->ATM_temporaryScriptVariant.empty() )
			return 0;

		int     type    = ( int )lua_tonumber( L, 1 );
		int     number  = ( int )lua_tonumber( L, 2 );
		int     optype  = ( int )lua_tonumber( L, 3 );
		LPCSTR  opinfo  = lua_tostring( L, 4 );
		//�ǵ��Ǵ�ȡ����ǰ׺���˺ŵ��˺ŷ�������Ӵ.
		LPCSTR  account = g_Script.m_pPlayer->GetNotPrefixAccount();
		DWORD   staticId = g_Script.m_pPlayer->GetSID();

		// ���ܿ۳� 0��Ϊ������ֵ��Ϊ�������������Ӷ��ǿ۳�����
		if ( ( number <= 0 ) || ( number > 100000 ) )
			return 0;

		// �ݴ������ָ�����
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
				// �����ǰ���ô��ڻص�����ʱ����ʼ����ʱ����ѹջ��
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
				// ���������ȫ�����ԣ���Ϊ��û�лص��Ļ��ҿ۳��ص�����
			}

			// ��������ڻص����Ǿ��ò��ţ�Ҳ���ܣ����ز������������������Ҫ�ص��Ĳ������ͻ����
			if ( !param_stack.empty() )
			{
				// [OP_BEGIN_ERRHANDLE] ( 0 ) ( "sp_DecreasePoint" ) [OP_END_ERRHANDLE]

				// ע������쳣ʱ���ص����ݣ�����
				// �������쳣���ص����ݼ����Լ������ݿ⴦�����Ϊ����ʧ�ܣ�
				slm [OP_BEGIN_ERRHANDLE] 
				( (int)SMessage::EPRO_ACCOUNT_MESSAGE )
					( (int)SAccountBaseMsg::EPRO_REFRESH_CARD_POINT )
					( staticId )
					( 0 )
					[OP_END_ERRHANDLE];
			}

			// ׼���洢���̣����㣩
			slm [OP_PREPARE_STOREDPROC] ( "AccountManager.sp_DecreasePoint" ) 

				// �趨���ò���
				[OP_BEGIN_PARAMS]
			(1) ( account )     // �۵��˺�
				(2) ( type )        // �۵�����
				(3) ( number )      // �۵����� ֵ����Ϊ��
				(4) ( optype )      // �������� ����LOG
				(5) ( opinfo )      // ����˵�� ����LOG
				[OP_END_PARAMS]

			// ���ô洢����
			[OP_CALL_STOREDPROC];

			// ��������ڻص����Ǿ��ò��ţ�Ҳ���ܣ����ز������������������Ҫ�ص��Ĳ������ͻ����
			if ( !param_stack.empty() )
			{
				// �����Ҫ����ֵ���ں�������������Ҫ����һ������Ч���������ݣ�
				// �Ա�����һ�ʲ���Ŀ�ĺͱ��β���Ŀ�Ĳ�ͬ��ͬʱ�ֳ�������������µ��쳣
				g_Script.m_pPlayer->ATM_temporaryVerifier = timeGetTime();

				// ��ʼ���������ݺ�
				slm [OP_INIT_RETBOX]    (1024)

					// ���ݺеĵ�һ��ֵ
					[OP_BOX_VARIANT]    ( (int)SMessage::EPRO_ACCOUNT_MESSAGE )
					[OP_BOX_VARIANT]    ( (int)SAccountBaseMsg::EPRO_REFRESH_CARD_POINT )
					[OP_BOX_VARIANT]    ( staticId )

					// ���ؿ۵�����
					[OP_BOX_VARIANT]    ( g_Script.m_pPlayer->ATM_temporaryVerifier )
					[OP_BOX_VARIANT]    ( type )

					// ���ش洢���̵ķ���ֵ
					[OP_BOX_PARAM]      ( 0 )   // �۵��Ƿ�ɹ�
					[OP_BOX_PARAM]      ( 6 )   // �۵��ʣ����ٵ�

					// �������ݺ�
					[OP_RETURN_BOX];
			}
			else
			{
				// staticId �����������ڻص���һ���������������Ҫ�ص���������Ϊ-1�Ա���LOG�з���
				staticId = -1;
			}

			// ����
			slm [OP_RPC_END];

			// ȫ��׼���ú��ٷ�����Ϣ����½�������������ɵ�½��������ת���˺ŷ�����
			msg.dstDatabase = SQGameServerRPCOPMsg::ACCOUNT_DATABASE;
			//�����ǰ��˺ŵ�ǰ׺������Ϣ���淢����¼������,�Ա��ڷֱ�ַ��������˺��ϴ���
			dwt::strcpy( (char*)msg.GetExtendBuffer(), g_Script.m_pPlayer->GetAccountPrefix(), 16 );
			if ( !SendToLoginServer( &msg, ( long )( sizeof( msg ) - slm.EndEdition() ) ) )
				return 0;

			extern dwt::stringkey<char [256]> szDirname;
			TraceInfo( szDirname + "decpoint.log", "[%s]�����۵������[%s][%08X][%d][%d][%d][%s]", 
				GetStringTime(), account, staticId, type, number, optype, opinfo );
		}
		catch ( lite::Xcpt & )
		{
			// ֻ��������жϵĴ���
		}

		return 0;
	}

	static int L_CancelWaitingState( lua_State *L )
	{
		if ( g_Script.m_pPlayer == NULL )
			return 0;

		extern dwt::stringkey<char [256]> szDirname;
		TraceInfo( szDirname + "decpoint.log", "[%s]�۵�����Ѿ���ȡ����[%08X][%d]%s", 
			GetStringTime(), g_Script.m_pPlayer->GetSID(), g_Script.m_pPlayer->m_bWaitBuyResult,
			( ( L == NULL ) ? "[�Զ�����]" : "" ) );

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
		//             // ɾ��ָ������
		//             if ( !g_Script.m_pPlayer->CheckItem( item ) )
		//                 return 0;
		// 
		//             result = g_Script.m_pPlayer->DelItem( item, GetNpcScriptInfo( "�ű�ɾ��ModifyItem" ), option[0] == 'X' );
		//             break;
		// 
		//         case 'A': // ��ָ��������ӵ�������ϣ�����ΨһID��
		//         case 'R': // ��ָ�����߻ָ���������ϣ�����ΨһID��
		//             {
		// // 		        if ( g_Script.m_pPlayer == NULL )
		// // 			        return 0;
		// // 
		// //                 if ( option[0] == 'A' )
		// //                 {
		// //                     // ���Ƹù���ֻ�����GenerateItemDetailsʹ�ã�
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
		// //                     // �����µ�ΨһID��
		// //                     if ( !GenerateNewUniqueId( item ) )
		// //                         return 0;
		// //                 }
		// // 
		// //                 result = g_Script.m_pPlayer->AddExistingItem( item, pos, true, GetNpcScriptInfo( "�ű����ModifyItem" ) );
		//             }
		//             break;
		// 
		//         case 'G':
		//             // ��ָ���������ɵ�������
		//             {
		// //                 // ���Ƹù���ֻ�����GenerateItemDetailsʹ�ã�
		// //                 if ( ( item.byCellX != 255 ) || ( item.byCellY != 255 ) )
		// //                     return 0;
		// // 
		// //                 // ��ȡָ������������
		// //                 if ( !lua_istable( L, 3 ) )
		// //                     return 0;
		// // 
		// //                 #define GET_FIELD_NUMBER_DIRECTLY( _key ) _GET_FIELD_NUMBER_DIRECTLY( 3, _key )
		// // 
		// //                 DWORD regionId = static_cast< DWORD >( GET_FIELD_NUMBER_DIRECTLY( "regionId" ) );
		// //                 if ( regionId == 0 )
		// //                     return rfalse( 2, 1, "(MI)û��ָ��Ŀ�곡��" );
		// // 
		// //                 CRegion *destRegion = GetRegionById( regionId );
		// //                 if ( destRegion == NULL )
		// //                     return rfalse( 2, 1, "(MI)�Ҳ���ָ������" );
		// // 
		// //                 CItem::SParameter param;
		// //                 memset( &param, 0, sizeof( param ) );
		// // 
		// //                 param.xTile = static_cast< WORD >( GET_FIELD_NUMBER_DIRECTLY( "x" ) );
		// //                 param.yTile = static_cast< WORD >( GET_FIELD_NUMBER_DIRECTLY( "y" ) );
		// //                 if ( ( param.xTile == 0 ) || ( param.yTile == 0 ) )
		// //                     return rfalse( 2, 1, "(MI)δ�趨��������" );
		// // 
		// //                 param.dwLife = static_cast< DWORD >( GET_FIELD_NUMBER_DIRECTLY( "lifeTime" ) );
		// //                 if ( param.dwLife == 0 ) param.dwLife = 10 * 60 * 1000;         // ����Ĭ��ֵ��
		// //                 else if ( param.dwLife != 0xffffffff ) param.dwLife *= 1000;    // �����ߴ��ʱ����ߵ��뼶�趨
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
		// //                 // �����µ�ΨһID��
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
		// �Ƿ���ȫ����������֪ͨ��
		bool fullSynchro = ( lua_toboolean( L, 2 ) == 1 );

		// �µ�������Ա���һ���̶���ʽ��������

		lite::table_iterator mainTable( L, 1 );
		if ( !mainTable.isValid() ) 
			return rfalse( 2, 1, "����ı������L_SetRandomAbilityTable��" ), 0;

		RandomAbilityTable tempTable;
		memset( &tempTable, 0, sizeof( tempTable ) );

		while ( mainTable ++ )
		{
			// �����ÿһ�����Ӧ�÷��� key[number] = value[subtable]
			if ( ( lua_type( L, mainTable.index_of_key ) != LUA_TNUMBER ) ||
				( lua_type( L, mainTable.index_of_value ) != LUA_TTABLE ) )
				return rfalse( 2, 1, "����һ����[����/�ӱ�]�Ĵ��������L_SetRandomAbilityTable��" ), 0;

			DWORD index = ( DWORD )lua_tonumber( L, mainTable.index_of_key );
			if ( index >= 32 )
				return rfalse( 2, 1, "�ӱ����������31����L_SetRandomAbilityTable��" ), 0;

			RandomAbilityTable::Unit &unit = tempTable.units[ index ];

			lite::table_iterator subTable( L, mainTable.index_of_value );
			if ( !subTable.isValid() )
				return rfalse( 2, 1, "��Ч���ӱ�L_SetRandomAbilityTable��" ), 0;

			while ( subTable ++ )
			{
				// ע�⣺����û����lua_isnumber/isstring...֮��ĺ���������Ϊ�����key��numberʱ������������������ͨ������
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
									return rfalse( 2, 1, "���������ѡ��[%s]��L_SetRandomAbilityTable��", randomType.c_str() ), 0;

								unit.randomType = ( RandomAbilityTable::RANDOM_TYPE )index;
					}
					else if ( lua_type( L, subTable.index_of_value ) == LUA_TNUMBER )
					{
						if ( keyValue == "rate" ) 
						{
							double rating = lua_tonumber( L, subTable.index_of_value );
							if ( ( rating > 1 ) || ( rating < 0 ) )
								return rfalse( 2, 1, "���������Ҫ��0~1֮�䣡��L_SetRandomAbilityTable��[%d]", index ), 0;

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
						return rfalse( 2, 1, "�仯��������8�ף�L_SetRandomAbilityTable��" ), 0;

					lua_rawgeti( L, -1, 1 );
					lua_rawgeti( L, -2, 2 );
					unit.array[ slotIndex ].rating = ( DWORD )( lua_tonumber( L, -2 ) * 100000000 );
					unit.array[ slotIndex ].value  = ( float )( lua_tonumber( L, -1 ) );
					lua_pop( L, 2 );

					if ( ( unit.array[ slotIndex ].rating == 0 ) || ( unit.array[ slotIndex ].value == 0 ) || 
						( unit.array[ slotIndex ].rating > 100000000 ) )
					{
						return rfalse( 2, 1, 
							"��Ч���ӱ��趨��L_SetRandomAbilityTable��"
							"[%d] = { [%d] = { %.7f, %.3f } }", index, slotIndex, 
							( double )unit.array[ slotIndex ].rating / 100000000, 
							unit.array[ slotIndex ].value ), 0;
					}
				}
				else
					return rfalse( 2, 1, "����һ����Ч�ı��L_SetRandomAbilityTable����" ), 0;
			}

			// newly ���ڲ��������ˣ���ȡ��ʽ�Ѿ������޸ģ�
			// ���ӱ�������Ϻ���Ҫ����value���µ���һ��array�����ݵ�����˳�����������Ļ����ܿ������ҵģ���
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
			rfalse( 2, 1, "������Ա����óɹ���L_SetRandomAbilityTable��" );
		else
		{
			AnswerUpdateItemDataMsg msg;
			msg.index = QueryUpdateItemDataMsg::RANDMONTABLE;

			try
			{
				lite::Serializer slm( msg.streamBuffer, sizeof( msg.streamBuffer ) );
				slm( lite::Variant( &RandomAbilityTable::RATable, sizeof( RandomAbilityTable::RATable ) ) );
				BroadcastMsg( &msg, ( WORD )( sizeof( msg ) - slm.EndEdition() ) );
				rfalse( 2, 1, "������Ա����óɹ���L_SetRandomAbilityTable�������ҹ㲥���µ���������" );
			}
			catch ( lite::Xcpt & )
			{
			}
		}

		return 0;
	}

	// ͨ���ýű�����������һ�����ñ��д��ڵ�����Ϊ�������ñ��е����ݣ�
	static int L_GetItemSetting( lua_State *L )
	{
		// zeb 2009.11.19
		//         DWORD index = ( DWORD )lua_tonumber( L, 1 );
		//         const SItemData *itemData = CItem::GetItemData( index );
		//         if ( index == 0 || itemData == NULL )
		//             return 0;
		// 
		//         // ���ڱ��ֶ�ջƽ�⣡
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
		// 			/// װ����Ҫ���������
		// 			if( ITEM_IS_EQUIPMENT( itemData->byType ) )
		// 			{
		// 				SETTABLE_BY_STRING( "School", itemData->byGroupEM );
		// 			}
		// 		}
		// 
		// 
		//         // scope of object
		//         {
		//             // ��ȫ��һ����ȫ��һ��ʹ��std::string��Ϊ�˽����ݽضϣ�����overrun
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


	// ͨ���ýű�����������һ�����ñ��д��ڵ�����Ϊ���ݵ������ͷֽ�������������ݣ�
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
		//             // ֻ���� misc�ײ����ݣ����ڼ�ӹ�����ߵĸ��±�
		//             // lua_pushnumber( L, reinterpret_cast< double& >( static_cast< SItemBase& >( item ) ) );
		//             char buf[256];
		//             sprintf( buf, "%I64X", ( QWORD )( ( reinterpret_cast< QWORD& >( static_cast< SItemBase& >( item ) ) >> 24 ) & 0x000000ffffffffff ) );
		//             lua_pushstring( L, buf );
		//             return 1;
		//         }
		// 
		//         // ���ڱ��ֶ�ջƽ�⣡
		//         int stackPos = lua_gettop( L );
		// 
		//         // ���ݵ��������ж���Ҫ����һ������luaTable
		//         // ##############################################
		//         // ע�⣡�������͵��߽ṹ������ȫ��Ӧ������
		//         // ##############################################
		// 
		//         // ��ʼ����4 [index,type,flags,misc] ���߱�š��������͡���־λ����������
		//         // ����flagsΪ�ɽ��ܱ仯�����ݣ�index,typeΪ�����ڲ�ѯ�����ݣ�miscΪ8�ֽڵ�itemBase�ײ��������ݣ�
		//         int nElements = 5;//4��Ϊ5�����һ����ߵȼ����� jym
		// 
		//         if ( ITEM_IS_OVERLAP( itemData->byType ) )
		//             // ���Ϊ���ص��࣬����2������Ϊ[number,maximum] ��ǰ�ص�����������ص�����
		//             // ����numberΪ�ɽ��ܱ仯�����ݣ�maximum�����ڲ�ѯ
		//             nElements += 2;
		//         else if ( ITEM_IS_EQUIPMENT( itemData->byType ) )
		//             // ���Ϊװ���࣬����7������Ϊ[prefix,level,experience,durance,slots,attributes,scripts] 
		//             // ǰ׺���ȼ������飬�;öȣ���Ƕ���[5]���������[5]���ű�����[4]�������е����ݾ��ɽ��ܱ仯��
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
		//         // ע�⣬����ط���һ�����⴦��
		//         // ����lua��֧��64λ����������ֻ�ܲ�ȡ����;����ʵ��
		//         // �����������ô���������Ϊ�����ڼ��㣬��������ʵ�ֱ�ʶ��
		//         // ��ǰʵ�ֵķ�ʽ��ʹ�� lightuserdata ����¼63bit�����ݣ����λ������Ǹ��û�������63bit���ͣ�����
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
		//             // Ϊ slots��attributes��scripts �����ӱ�
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
		//             // ��������������
		//             // �书���⴦��
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
		//             // ����ǽű�����ߣ�����һ��script�����������нű����ݣ�
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
		//         // ��Ȼ��������ô�����ݣ���ֻ������һ������ѣ�
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


	// ͨ���ú��������ݴ�������ñ��޸ģ����£����ߵ�����
	// ����õ�����������ϵĵ��ߣ���ͬ��֪ͨ�ͻ��˸������ݣ�
	static int L_UpdateItemDetails( lua_State *L )
	{
		// zeb 2009.11.19
		//         int size = 0;
		//         void *data = luaEx_touserdata( L, 1, &size );
		//         bool doCheck = ( lua_toboolean( L, 3 ) == 0 );
		//         bool acceptIndexChange = ( lua_toboolean( L, 4 ) == 1 ); // �Ƿ����index�ĸı䣡
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
		//         // ����֧�� GET_FIELD_NUMBER
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
		//         // ��ȡ2������,misc��flags,���misc���޸ĵĻ����Ͳ��ܼ�������ִ���ˣ�
		//         lua_getfield( L, 2, "misc" );
		//         QWORD misc = luaEx_toint63( L, -1 );
		//         lua_pop( L, 1 );
		// 
		//         if ( /*reinterpret_cast< QWORD& >( GET_FIELD_NUMBER_DIRECTLY( "misc" ) ) != 
		//             reinterpret_cast< QWORD& >( static_cast< SItemBase& >( item ) ) ) */
		//             misc != item.uniqueId() )
		//             return 0;
		// 
		//         // �����־λ�趨Ϊ�����Խ��ܵ���index���޸ģ�
		//         if ( acceptIndexChange )
		//         {
		//             // �ڻ�ȡ���߱�ź�Ҫͳһ�жϣ��仯ǰ��ĵ���Ӧ�����������
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
		//                 // ���ж��¾ɵ��ߵ����ͣ������ͬ���Ͳ���Ҫ����ߵ��ж���
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
		//                 // ���ָ�����޸�index������û���ҵ�index�����ݣ���ʧ�ܷ��أ�
		//                 return 0;
		//             }
		//         }
		// 
		//         // ���±�־λ��
		//         IF_GET_FIELD_NUMBER( "flags" )  item.flags = ( DWORD )_result;
		// 
		//         if ( ITEM_IS_OVERLAP( itemData->byType ) )
		//         {
		// #define ITEM_CONVERT reinterpret_cast< SOverlap& >( static_cast< SItemBase& >( item ) )
		// 
		//             // ���Ϊ���ص��࣬��Ҫ����number
		//             IF_GET_FIELD_NUMBER( "number" ) 
		//             {
		//                 ITEM_CONVERT.number = ( DWORD )_result;
		// 
		//                 // �ص���������Ϊ0
		//                 if ( ITEM_CONVERT.number == 0 )
		//                     return 0;
		//             }
		// #undef ITEM_CONVERT
		//         }
		//         else if ( ITEM_IS_EQUIPMENT( itemData->byType ) )
		//         {
		// #define ITEM_CONVERT reinterpret_cast< SEquipment& >( static_cast< SItemBase& >( item ) )
		// 
		//             // ���Ϊװ������Ҫ���µĶ����Ͷ࿩
		//             // ǰ׺���ȼ������飬�;ö�
		//             //IF_GET_FIELD_NUMBER( "prefix" )         ITEM_CONVERT.prefix         = ( BYTE )_result;
		//             //IF_GET_FIELD_NUMBER( "level" )          ITEM_CONVERT.level          = ( BYTE )_result;
		//             //IF_GET_FIELD_NUMBER( "experience" )     ITEM_CONVERT.experience     = ( WORD )_result;
		//             //IF_GET_FIELD_NUMBER( "cur_durance" )    ITEM_CONVERT.durance.cur    = ( WORD )_result;
		//             //IF_GET_FIELD_NUMBER( "max_durance" )    ITEM_CONVERT.durance.max    = ( BYTE )_result;
		//             IF_GET_FIELD_NUMBER( "color" )          ITEM_CONVERT.reserve        = ( ITEM_CONVERT.reserve & 0xfc ) | ( ( BYTE )_result & 0x3 );
		// 
		//             // ��Ƕ���[5]���������[5]���ű�����[4]
		//             lua_getfield( L, 2, "slots" );
		//             if ( lua_istable( L, -1 ) ) for ( int i = 0; i < SEquipment::MAX_SLOTS; ++i )
		//             {
		//                 // ���ǰ��Ĳ����Ч�����������ݶ��������ˣ�
		//                 if ( ( i != 0 ) && ITEM_CONVERT.slots[i-1].isInvalid() )
		//                 {
		//                     *( LPBYTE )&ITEM_CONVERT.slots[i] = 0;
		//                     continue;
		//                 }
		// 
		//                 ITEM_CONVERT.slots[i].type = ( BYTE )( ( DWORD )GET_ARRAY_NUMBER_DIRECTLY( i + 1 ) / 100 );
		//                 ITEM_CONVERT.slots[i].value = ( BYTE )( ( DWORD )_result % 100 );
		// 
		//                 // ͬ�ϣ����ǰ��Ĳ��Ϊ�գ�����������Ҳ������Ϊ��Ƕ״̬��
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
		//             // �������ù���˵��:
		//             // 1000 ǧλ��Ϊ����ֵ
		//             // 10000 ��λ��Ϊ ��������
		//             // ����ֵ
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
		//             // ��������������
		//             // �书���⴦��
		//             IF_GET_FIELD_NUMBER( "addskill" )
		//             {
		//                 int temp = ( int )_result;
		//                 ITEM_CONVERT.attribute.unitSkill.skillType = abs( (int)( temp / 1000 ) );
		//                 ITEM_CONVERT.attribute.unitSkill.skillValue = temp % 1000;
		//             }
		// 
		//             // �ķ����⴦��
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
		//             if ( itemData->wMaxWear /*itemData->wWearPrime*/ != 0xffff )  // ���;ò��������;ö�
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
		//             //    // ���ǰ������������Ч�����������ݶ��������ˣ�
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
		//             // ����ǽű�����ߣ�����һ��script�����������нű����ݣ�
		//             lua_getfield( L, 2, "scripts" );
		//             if ( !lua_istable( L, -1 ) )    // ���û��script�ӱ������ʧ�ܣ�
		//                 return lua_pop( L, 1 ), 0;
		// 
		//             size_t size = 0;
		//             SScriptItem &scriptItem = reinterpret_cast< SScriptItem& >( static_cast< SItemBase& >( item ) );
		//             try 
		//             {
		//                 ZeroMemory( item.buffer, sizeof( item.buffer ) );
		//                 lite::Serializer slm( scriptItem.streamData, sizeof( scriptItem.streamData ) );
		// 
		//                 // ����8��������ȷ��ÿһ��Ԫ�ص���Ч�� 1 2 4 8 16 32 64 128
		//                 for ( int i = 0; i < 8; i ++ )
		//                 {
		//                     lua_rawgeti( L, -1, i + 1 );
		//                     if ( !lua_isnil( L, -1 ) )  // ��Ч�����ݲű����������
		//                     {
		//                         scriptItem.declare |= ( 1 << i );
		//                         slm( lite::lua_variant( L, -1 ) );
		//                     }
		//                     else
		//                         scriptItem.declare &= ( ~( 1 << i ) );
		//                     lua_pop( L, 1 );
		//                 }
		// 
		//                 // ƽ��������־!
		//                 slm.EndEdition();
		//                 size = slm.maxSize();
		//             }
		//             catch ( lite::Xcpt & )
		//             {
		//                 // ������ִ��󣬵�Ȼ�͸���ʧ���ˣ�
		//                 // ��Ҫ�ѳ����쳣ʱ���Ǹ��ű����ݳ�ջ! �Ա���ջƽ��
		//                 lua_pop( L, 1 );
		//             }
		// 
		//             // ��script��ջ
		//             lua_pop( L, 1 );
		// 
		//             // ��;�����쳣, �޷��ټ���!
		//             if ( size == 0 )
		//                 return 0;
		// 
		//             // ���û���κ�����, ���൱���������
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
		//         // ������ϵĵ��ߣ����ҳ��������ݱ仯���Ÿ��µ��ͻ��ˣ�
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
		//                 // ����
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
		//         // �ɹ��󷵻�1��
		//         lua_pushnumber( L, 1 );
		// zeb 2009.11.19
		//	return 1;

		return 0;
	}

	/*
	static void AttUpdate( SEquipment::Attribute::Unit &att, DWORD vt )
	{
	DWORD type = att.type = vt / 10000;
	// ע�⣺���ﲻ����ȡģ��������һ���޷���λ���ܴ渺�����ڶ������ݵ��ű���������ݶ���Ĭ������������
	//if ( type == SGemData::GEMDATA_ACTIONSPEED || type == SGemData::GEMDATA_MOVESPEED )
	//    att.value = 500 + ( vt % 500 );   
	//else
	att.value = vt % 1000;
	}//*/

	// ֱ�Ӹ���item���ݴ�����ָ�����͵ĵ��ߣ�
	// ��ֱ������չʾ����Ҫͨ��modifyitem��ӵ�������ϣ�
	static int L_GenerateItemDetails( lua_State *L )
	{
		// zeb 2009.11.19
		//         if ( !lua_istable( L, 1 ) )
		//             return 0;
		// 
		//         // ����֧�� GET_FIELD_NUMBER
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
		//         // ���ñ�־λ��
		//         IF_GET_FIELD_NUMBER( "flags" )  item.flags = ( DWORD )_result;
		// 
		//         if ( ITEM_IS_OVERLAP( itemData->byType ) )
		//         {
		//             item.size = sizeof( SOverlap );
		// 
		// #define ITEM_CONVERT reinterpret_cast< SOverlap& >( static_cast< SItemBase& >( item ) )
		// 
		//             // ���Ϊ���ص��࣬��Ҫ����number
		//             IF_GET_FIELD_NUMBER( "number" ) 
		//             {
		//                 ITEM_CONVERT.number = ( DWORD )_result;
		// 
		//                 // �ص���������Ϊ0
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
		//             // ���Ϊװ������Ҫ���µĶ����Ͷ࿩
		//             // ǰ׺���ȼ������飬�;ö�
		//             //IF_GET_FIELD_NUMBER( "prefix" )         ITEM_CONVERT.prefix         = ( BYTE )_result;
		//             //IF_GET_FIELD_NUMBER( "level" )          ITEM_CONVERT.level          = ( BYTE )_result;
		//             //IF_GET_FIELD_NUMBER( "experience" )     ITEM_CONVERT.experience     = ( WORD )_result;
		//             //IF_GET_FIELD_NUMBER( "cur_durance" )    ITEM_CONVERT.durance.cur    = ( WORD )_result;
		//             //IF_GET_FIELD_NUMBER( "max_durance" )    ITEM_CONVERT.durance.max    = ( BYTE )_result;
		//             IF_GET_FIELD_NUMBER( "color" )          ITEM_CONVERT.reserve        = ( ITEM_CONVERT.reserve & 0xfc ) | ( ( BYTE )_result & 0x3 );
		// 
		//             // ��Ƕ���[5]���������[5]���ű�����[4]
		//             lua_getfield( L, 1, "slots" );
		//             if ( lua_istable( L, -1 ) ) for ( int i = 0; i < SEquipment::MAX_SLOTS; ++i )
		//             {
		//                 // ���ǰ��Ĳ����Ч�����������ݶ��������ˣ�
		//                 if ( ( i != 0 ) && ITEM_CONVERT.slots[i-1].isInvalid() )
		//                 {
		//                     *( LPBYTE )&ITEM_CONVERT.slots[i] = 0;
		//                     continue;
		//                 }
		// 
		//                 ITEM_CONVERT.slots[i].type = ( BYTE )( ( DWORD )GET_ARRAY_NUMBER_DIRECTLY( i + 1 ) / 100 );
		//                 ITEM_CONVERT.slots[i].value = ( BYTE )( ( DWORD )_result % 100 );
		// 
		//                 // ͬ�ϣ����ǰ��Ĳ��Ϊ�գ�����������Ҳ������Ϊ��Ƕ״̬��
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
		//             // �������ù���˵��:
		//             // 1000 ǧλ��Ϊ����ֵ
		//             // 10000 ��λ��Ϊ ��������
		//             // ����ֵ
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
		//             // ��������������
		//             // �书���⴦��
		//             IF_GET_FIELD_NUMBER( "addskill" )
		//             {
		//                 int temp = ( int )_result;
		//                 ITEM_CONVERT.attribute.unitSkill.skillType = abs( ( int )( temp / 1000 ) );
		//                 ITEM_CONVERT.attribute.unitSkill.skillValue = temp % 1000;
		//             }
		// 
		//             // �ķ����⴦��
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
		// //             if ( itemData->wWearPrime != 0xffff )  // ���;õ�װ�����ܼ��;�
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
		//             //    // ���ǰ������������Ч�����������ݶ��������ˣ�
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
		//             // ����ǽű�����ߣ�����һ��script�����������нű����ݣ�
		//             lua_getfield( L, 1, "scripts" );
		//             if ( lua_istable( L, -1 ) )    // ���û��script�ӱ������ʧ�ܣ�
		// 			{
		// 
		// 				size_t size = 0;
		// 				SScriptItem &scriptItem = reinterpret_cast< SScriptItem& >( static_cast< SItemBase& >( item ) );
		// 				try 
		// 				{
		// 					ZeroMemory( item.buffer, sizeof( item.buffer ) );
		// 					lite::Serializer slm( scriptItem.streamData, sizeof( scriptItem.streamData ) );
		// 
		// 					// ����8��������ȷ��ÿһ��Ԫ�ص���Ч�� 1 2 4 8 16 32 64 128
		// 					for ( int i = 0; i < 8; i ++ )
		// 					{
		// 						lua_rawgeti( L, -1, i + 1 );
		// 						if ( !lua_isnil( L, -1 ) )  // ��Ч�����ݲű����������
		// 						{
		// 							scriptItem.declare |= ( 1 << i );
		// 							slm( lite::lua_variant( L, -1 ) );
		// 						}
		// 						lua_pop( L, 1 );
		// 					}
		// 
		// 					// ƽ��������־!
		// 					slm.EndEdition();
		// 					size = slm.maxSize();
		// 				}
		// 				catch ( lite::Xcpt & )
		// 				{
		// 				}
		// 
		// 				// ��;�����쳣, �޷��ټ���!
		// 				if ( size == 0 )
		// 					return 0;
		// 
		// 				// ���û���κ�����, ���൱���������
		// 				scriptItem.size = sizeof( SItemBase ) + 1;
		// 				if ( scriptItem.declare != 0 )
		// 					scriptItem.size += ( BYTE )size;
		// 			}
		// 			// ��script��ջ
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
		//         // �ɹ��󷵻�dataHandle��
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
			return rfalse(2, 1, "(COI)û��ָ��Ŀ�곡��");

		CRegion *destRegion = GetRegionById(regionID);
		if (!destRegion)
			return rfalse(2, 1, "(COI)�Ҳ���ָ������");

		// �ȳ��Ի�ȡ����ID�ţ�����У�������ظ����
		DWORD controlId = 0;
		IF_GET_FIELD_NUMBER("controlId")
		{
			if (0 != _result && destRegion->CheckObjectByControlId(controlId = static_cast<DWORD>(_result)))
				return rfalse(2, 1, "(COI)ָ�������е��ƶ���Ŷ����Ѿ�����[r=%d, cid=%d]", regionID, controlId);
		}

		DWORD objectType = static_cast<DWORD>(GET_FIELD_NUMBER_DIRECTLY("objectType"));
		if (1 == objectType)			// ����NPC
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
				return rfalse(2, 1, "(COI)CreateNpcʧ��!");

			lua_pushnumber(L, npc->GetGID());
		}
		else if (0 == objectType)		// ��������
		{
			CMonster::SParameter param;
			memset(&param, 0, sizeof(param));

			param.wListID = static_cast<WORD>(GET_FIELD_NUMBER_DIRECTLY("monsterId"));
			const SMonsterBaseData *pMonsterData = CMonsterService::GetInstance().GetMonsterBaseData(param.wListID);
			if (!pMonsterData)
				return rfalse(2, 1, "(COI)�ڻ����б����Ҳ�����ӦID�Ĺ���!");

			IF_GET_FIELD_NUMBER( "refreshTime"  ) param.dwRefreshTime    = static_cast<DWORD>( _result );
			IF_GET_FIELD_NUMBER( "x"			) param.wX														 = static_cast<WORD >( _result );
			IF_GET_FIELD_NUMBER( "y"			) param.wY													     = static_cast<WORD >( _result );
			IF_GET_FIELD_NUMBER( "CreatePlayerID"			) param.wCreatePlayerID               = static_cast<DWORD >( _result );
			IF_GET_FIELD_NUMBER("RankIndex") param.dRankIndex = static_cast<DWORD >(_result);

			param.reginID = destRegion->m_wRegionID;
			CMonster *monster = destRegion->CreateMonster(&param, controlId);
			if (!monster)
				return rfalse(2, 1, "(COI)CreateMonsterʧ��!");

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
			return rfalse(2, 1, "(ROI)�Ҳ���ָ������");

		BOOL result = destRegion->RemoveObjectByControlId(controlId);
		if (!result)
			return 0;

		lua_pushnumber(L, 1);
		return 1;
	}


	static int L_UpdateItemSetting( lua_State *L )
	{
		// zeb 2009.11.19
		//        // �ú���ֻ��Ψһ�Ĳ���,һ������������Ϣ�ı�!
		//        if ( !lua_istable( L, 1 ) )
		//            return 0;
		//
		//        // ����֧�� GET_FIELD_NUMBER
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
		//        // ��������ڵı�Ų�����Ч�ĵ��߱��!
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
		//            // �ȸ�������!
		//            lua_getfield( L, 1, "name" );
		//            if ( lua_isstring( L, -1 ) ) dwt::strcpy( newItemData.szName, lua_tostring( L, -1 ), sizeof( newItemData.szName ) );
		//            lua_pop( L, 1 );
		//
		//            // �������������Ŀ
		//            IF_GET_FIELD_NUMBER( "buy"      ) newItemData.dwBuy         = static_cast< DWORD >( _result );      // ����۸�
		//            IF_GET_FIELD_NUMBER( "sell"     ) newItemData.dwSell        = static_cast< DWORD >( _result );      // �����۸�
		//	        IF_GET_FIELD_NUMBER( "icon"     ) newItemData.wSellPrice    = static_cast< WORD >( _result );	    // ��Ʒ����Ԫ�������۸�
		//	        IF_GET_FIELD_NUMBER( "gift"     ) newItemData.wScorePrice   = static_cast< WORD >( _result );	    // ��Ʒ�۸�
		//            IF_GET_FIELD_NUMBER( "image"    ) newItemData.byGraph1      = static_cast< WORD >( _result );       // ͼƬ��ţ�����������Ļ���Ҳʹ����һ��IDȥ�Ҿ��������������
		//	        IF_GET_FIELD_NUMBER( "image2"   ) newItemData.wGraph2       = static_cast< WORD >( _result );       // ������ʾ�õ�ͼƬ���
		//	        IF_GET_FIELD_NUMBER( "sound"    ) newItemData.wSound1       = static_cast< WORD >( _result );       // ����Ʒ���䣨������Ʒ����װ��������Ч���
		//	        IF_GET_FIELD_NUMBER( "overlap"  ) newItemData.wSpecial      = static_cast< WORD >( _result );       // ����Ӧ�ã���Ŀǰ���������ص�������
		//            IF_GET_FIELD_NUMBER( "groupRQ"  ) newItemData.byGroupEM     = static_cast< BYTE >( _result );       // ��������
		//            IF_GET_FIELD_NUMBER( "levelRQ"  ) newItemData.byLevelEM     = static_cast< WORD >( _result );       // �ȼ�����
		//            IF_GET_FIELD_NUMBER( "enRQ"     ) newItemData.wENEM         = static_cast< WORD >( _result );       // ��������
		//            IF_GET_FIELD_NUMBER( "inRQ"     ) newItemData.wINEM         = static_cast< WORD >( _result );       // ��������
		//            IF_GET_FIELD_NUMBER( "stRQ"     ) newItemData.wSTEM         = static_cast< WORD >( _result );       // ��������
		//            IF_GET_FIELD_NUMBER( "agRQ"     ) newItemData.wAGEM         = static_cast< WORD >( _result );       // ������
		//            IF_GET_FIELD_NUMBER( "hpAD"     ) newItemData.wHPAD        = static_cast< WORD >( _result );       // �����������ֵ
		//            IF_GET_FIELD_NUMBER( "spAD"     ) newItemData.wSPAD        = static_cast< WORD >( _result );       // �����������ֵ
		//            IF_GET_FIELD_NUMBER( "mpAD"     ) newItemData.wMPAD        = static_cast< WORD >( _result );       // �����������ֵ
		//            IF_GET_FIELD_NUMBER( "hpR"      ) newItemData.wHPR         = static_cast< WORD >( _result );       // �ָ�������
		//            IF_GET_FIELD_NUMBER( "spR"      ) newItemData.wSPR         = static_cast< WORD >( _result );       // �ָ�������
		//            IF_GET_FIELD_NUMBER( "mpR"      ) newItemData.wMPR         = static_cast< WORD >( _result );       // �ָ�������
		//            IF_GET_FIELD_NUMBER( "type"     ) newItemData.byType        = static_cast< BYTE >( _result );       // �õ��ߵ�����
		//	        IF_GET_FIELD_NUMBER( "cells"    ) newItemData.byHoldSize    = static_cast< BYTE >( _result );       // �õ���ռ�õ���Ʒ������
		//            // ʹ��/װ���������
		//            IF_GET_FIELD_NUMBER( "damAD"    ) newItemData.wDAMAD        = static_cast< WORD >( _result );       // �ӹ�����
		//            IF_GET_FIELD_NUMBER( "powAD"    ) newItemData.wPOWAD        = static_cast< WORD >( _result );       // ���ڹ�������
		//            IF_GET_FIELD_NUMBER( "defAD"    ) newItemData.wDEFAD        = static_cast< WORD >( _result );       // �ӷ�����
		//            IF_GET_FIELD_NUMBER( "agiAD"    ) newItemData.wAGIAD        = static_cast< WORD >( _result );       // ��������
		//            IF_GET_FIELD_NUMBER( "amuckAD"  ) newItemData.wAmuckAD      = static_cast< WORD >( _result );       // ��ɱ��
		//            IF_GET_FIELD_NUMBER( "luckAD"   ) newItemData.wLUAD         = static_cast< WORD >( _result );       // ������
		//            IF_GET_FIELD_NUMBER( "hpRAD"    ) newItemData.wRHPAD        = static_cast< WORD >( _result );       // �������Զ��ָ��ٶ�
		//            IF_GET_FIELD_NUMBER( "spRAD"    ) newItemData.wRSPAD        = static_cast< WORD >( _result );       // �������Զ��ָ��ٶ�
		//            IF_GET_FIELD_NUMBER( "mpRAD"    ) newItemData.wRMPAD        = static_cast< WORD >( _result );       // �������Զ��ָ��ٶ�
		//            IF_GET_FIELD_NUMBER( "clear"    ) newItemData.wClear        = static_cast< WORD >( _result );       // �����쳣״̬
		//            IF_GET_FIELD_NUMBER( "pointAD"  ) newItemData.wPointAD      = static_cast< WORD >( _result );       // �����Ե�
		//            IF_GET_FIELD_NUMBER( "levelAD"  ) newItemData.wLevelAD      = static_cast< WORD >( _result );       // �ӵȼ�
		//            // IF_GET_FIELD_NUMBER( "material" ) BYTE material;          // ���ߵĲ������ͣ������趨�;öȵĿ۳���
		//            IF_GET_FIELD_NUMBER( "subLevel" ) newItemData.wSubLevel     = static_cast< WORD >( _result );       // װ�����ӵȼ����ͻ�����Ҫ�õġ���
		//            IF_GET_FIELD_NUMBER( "script"   ) newItemData.dwScripID     = static_cast< DWORD >( _result );      // �ű����
		//	        IF_GET_FIELD_NUMBER( "changing" ) newItemData.wEffect       = static_cast< WORD >( _result );       // Ч��ͼ��Ŀǰ������Ƥ��߱���
		//            IF_GET_FIELD_NUMBER( "mainLevel") newItemData.nMainLevel    = static_cast< BYTE >( _result );       // ����4�����ݺ���װ��أ�ֻ��Ҫ�ڷ������жϣ�
		//            IF_GET_FIELD_NUMBER( "addType"  ) newItemData.nAddType      = static_cast< BYTE >( _result );
		//	        IF_GET_FIELD_NUMBER( "addScale" ) newItemData.wAddScale     = static_cast< BYTE >( _result );       // �ӳɱ��� 
		//	        // IF_GET_FIELD_NUMBER( "gestID" ) BYTE wGestID;           // ϰ���书����μ����
		//	        // IF_GET_FIELD_NUMBER( "gestLevel" ) BYTE wGestLevel;        // �书����ȼ�����
		//            // IF_GET_FIELD_NUMBER( "durance" ) BYTE durance;           // ���ߵ�Ĭ���;öȣ����ڳ�ʼ��ʱ�趨�;öȵ�����
		//            IF_GET_FIELD_NUMBER( "stone" ) newItemData.stone            = static_cast< WORD >( _result );       // Ӱ��ĳɹ��ٷֱ�
		//            IF_GET_FIELD_NUMBER( "expand" ) newItemData.dwExpand        = static_cast< DWORD >( _result );       // ��������
		//            IF_GET_FIELD_NUMBER( "restrict" ) newItemData.restrict      = static_cast< WORD >( _result );       // �Ƿ���Ҫ�������������Ÿ��蹦��
		//            IF_GET_FIELD_NUMBER( "wearPrime" ) newItemData.wWearPrime   = static_cast< WORD >( _result );       // ����Ч���ĳ���ʱ�䣬������ʱ����0�Ļ���˵���������Ե�����Ч��
		//        }
		//
		//        // �Ƿ���ȫ����������֪ͨ��
		//        bool fullSynchro = ( lua_toboolean( L, 2 ) == 1 );
		//
		//        // ���²��ȶ�crc����,�ж��Ƿ����
		//        DWORD crc32 = ( CItem::GetRawData( index ) ? CItem::GetRawData( index )->first : -1 );
		//
		//        // �������ˣ���������ȫ���£��������ͬ������Ҳ���޸��ˣ���crc����δ�仯����Ȼ�ﲻ��ͬ������
		//        CItem::UpdateItemData( index, itemData, true );
		//        if ( fullSynchro )
		//        {
		//            std::pair< DWORD, const SItemData* > *rawData = CItem::GetRawData( index );
		//            if ( ( rawData != NULL ) && ( rawData->first != crc32 ) )
		//            {
		//                // ���µ����пͻ���
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
		//             // �����ҵ��ĵ��ǲ������ߵ����꣡��Ҫ����
		// _refind:
		//             index ++;
		//             if ( index >= 1000 ) // ����ļ�ⷶΧ������չ��ȥ��Ӱ������
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
		//             // Ѱ�ҿɷ��õĳ�������
		//             CItem::GenerateNewGroundItem( destRegion, 0, 
		//                 param, GenItemParams( itemId, number ), LogInfo( 4, "�ű�(DropItems)����" ) );
		//         }

		return 0;
	}

	static int L_SelectObject( lua_State *L )
	{
		DWORD controlId = static_cast< DWORD >( lua_tonumber( L, 1 ) );     // Ŀ�����Ŀ��Ʊ��
		DWORD regionId = static_cast< DWORD >( lua_tonumber( L, 2 ) );      // ������ţ����Ⱥ͵�ǰ��ʹ�õĳ�����ƥ��

		CRegion *destRegion = NULL;
		if ( regionId == 0 )
			destRegion = g_Script.m_pPlayer ? g_Script.m_pPlayer->m_ParentRegion : NULL;
		else
			destRegion = GetRegionById( regionId );
		if ( destRegion == NULL )
			return 0;

		QWORD uniqueId = luaEx_toint63( L, 3 );
		//double tempNumber = lua_tonumber( L, 3 );     // ����Ľ������ΨһID
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
		// �ú���ֻ��Ψһ�Ĳ���,һ������������Ϣ�ı�!
		if ( !lua_istable( L, 1 ) )
			return 0;

		LPCSTR spname = _GET_FIELD_STRING_DIRECTLY( 1, "sp" );
		if ( spname == NULL )
			return rfalse( 2, 1, "û���趨�洢��������" );

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

			// �Ƿ���ڻص��ű�
			if ( lua_istable( L, 2 ) )
			{
				LPCSTR callback_name = _GET_FIELD_STRING_DIRECTLY( 2, "callback" );
				if ( callback_name == NULL )
					return rfalse( 2, 1, "û���趨�ص���������" );

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
						// ϣ������һ��ָ���Ĵ洢���̲���
						if ( tempStr[0] == '?'  )
							slm [OP_BOX_PARAM] ( atoi( tempStr + 1 ) );

						// ϣ������һ�������ݣ�������ת����lua_table '@'��ʾʹ��2�������ݿ飬'#'��ʾʹ��2ά��� '|'��ʾʹ��1ά���
						else if ( tempStr[0] == '@' || tempStr[0] == '#' || tempStr[0] == '|' )
							slm [OP_TABLE_DATA] ( abs( atoi( tempStr + 1 ) ) ) ( tempStr[0] ); 

						// ��������£�ϣ�����ص�ǰ����ַ�������
						else
							slm [OP_BOX_VARIANT] ( tempStr ); 
					}
					else
						slm [OP_BOX_VARIANT] ( static_cast< lite::Variant& >( lite::lua_variant( L, -1 ) ) ); // ϣ������һ��lua���ݡ�����

					lua_pop( L, 1 );
				}

				slm [OP_RETURN_BOX];
			}

			slm [OP_RPC_END];

			// ȫ��׼���ú��ٷ�����Ϣ����½�������������ɵ�½��������ת���˺ŷ�����
			msg.dstDatabase = static_cast< BYTE >( _GET_FIELD_NUMBER_DIRECTLY( 1, "dbtype" ) );
			dwt::strcpy( (char*)msg.GetExtendBuffer(), g_Script.m_pPlayer ? g_Script.m_pPlayer->GetAccountPrefix() : "", 16 );
			if ( !SendToLoginServer( &msg, ( long )( sizeof( msg ) - slm.EndEdition() ) ) )
				return rfalse( 2, 1, "����DBRPC��Ϣʧ��" );
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

	// ���ϵͳ�����ӵĵ���Ч��
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

	// ���ý�ɫ����ʱ����״̬��
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
		//// û��SID
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

		// DBCENTER ��������ID
		DWORD dbcenterId = static_cast< DWORD >( lua_tonumber( L, 4 ) );

		if ( password == NULL || serverip == NULL || serverport == NULL )
			return 0;

		extern BYTE tempTransBuff[0xffff];
		extern CDAppMain *&GetApp();

		if ( GetApp()->szLoginIP == NULL )
			return 0;

		SQGameServerRPCOPMsg &msg = * new ( tempTransBuff ) SQGameServerRPCOPMsg;
		msg.dstDatabase = (BYTE)dbcenterId; // DBCenter

		// ��������,ȡ���������µ�m_Property
		g_Script.m_pPlayer->SendData( SARefreshPlayerMsg::ONLY_BACKUP );

		static SSpanPlayerDataBuf data;
		memset( &data, 0, sizeof( data ) );

		memcpy( ( SFixBaseData* )&data,  ( SFixBaseData* )&g_Script.m_pPlayer->m_Property, sizeof( SFixBaseData ) );
		//        memcpy( ( SPlayerVenapointEx* )&data,  ( SPlayerVenapointEx* )&g_Script.m_pPlayer->m_Property, sizeof( SPlayerVenapointEx ) );
		//        memcpy( ( SPlayerDataExpand* )&data,  ( SPlayerDataExpand* )&g_Script.m_pPlayer->m_Property, sizeof( SPlayerDataExpand ) );

		// ��������汾�����ڿ�����
		data.m_version = SSpanPlayerDataBuf::GetVersion();

		//SFixProperty *data = ( SFixProperty * )( &g_Script.m_pPlayer->m_Property );
		size_t dataSize = sizeof( data );

		try
		{
			lite::Serializer sl( msg.streamData, dataSize + sizeof( SQGameServerRPCOPMsg ) );

			// ע������쳣ʱ���ص����ݣ�����
			sl [OP_BEGIN_ERRHANDLE] ( 0 ) ( "p_SavePlayerDataDBCenter" ) [OP_END_ERRHANDLE]

			// ׼���洢����
			[OP_PREPARE_STOREDPROC] ( "p_SavePlayerDataDBCenter" )

				// �趨���ò���
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

			// ���ô洢����
			[OP_CALL_STOREDPROC]

			// ��ʼ���������ݺ�
			[OP_INIT_RETBOX] ( 1024 )
				[OP_BOX_VARIANT] ( ( int )SMessage::EPRO_SCRIPT_MESSAGE )
				[OP_BOX_VARIANT] ( 14 ) // �µ���Ϣ����
				[OP_BOX_VARIANT] ( serverip )
				[OP_BOX_VARIANT] ( serverport )
				[OP_BOX_VARIANT] ( g_Script.m_pPlayer->m_ClientIndex )
				[OP_BOX_PARAM] ( 1 )
				[OP_BOX_PARAM] ( 4 )
				[OP_BOX_VARIANT] ( g_Script.m_pPlayer->GetGID() )
				[OP_BOX_VARIANT] ( g_Script.m_pPlayer->m_Property.m_GMLevel )

				// �������ݺ�
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

		// �������Ĳ�����һ��table�Ļ�
		if ( !lua_istable( L, 1 ) )
			return 0;

#define CHECK_AND_SET_G( _key, _member ) _IF_GET_FIELD_NUMBER( 1, _key ) \
	g_Script.m_pPlayer->m_stFacRight._member = static_cast< WORD >( __number ); else goto __checkfail;

		CHECK_AND_SET_G( "Donation"  , iDonation )
			CHECK_AND_SET_G( "Accomplishment", dwAccomplishment )

			GetGW()->m_FactionManager.UpdataMemberInfo( g_Script.m_pPlayer );
		return 0;

__checkfail: 
		return rfalse( 2, 1, "SetMemberInfo : ����Ĳ���" ), 0;  
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


	// ͨ���ýű�����������һ�����ñ��д��ڵ�����Ϊ���ݵ������ͷֽ�������������ݣ�
	static int L_GetItemScripts( lua_State *L )
	{
		return 0;
	}

	// ͨ���ú��������ݴ�������ñ��޸ģ����£����ߵ�����
	// ����õ�����������ϵĵ��ߣ���ͬ��֪ͨ�ͻ��˸������ݣ�
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
		// double �����������������ȵ� һǧ���� 15λ���� ������Ҫ�������������������Χ��
		double number = lua_tonumber( L, 1 );
		luaEx_pushint63( L, ( unsigned __int64 )number );
		return 1;
	}

	static int L_CheckPoints( lua_State *L )
	{
		//         if ( g_Script.m_pPlayer == NULL )
		//             return 0;
		// 
		//         // ��ʼ60��
		//         int pointCount = 60;
		// 
		//         // ��һ�������������
		//         int taskPoint = g_Script.m_pPlayer->CheckTaskPoints();
		// 
		//         pointCount += taskPoint;
		// 
		//         // �ڶ����ȼ�
		//         SLevelData *pLevelData = g_pUpGradeData->GetLevelData( g_Script.m_pPlayer->m_Property.m_School );
		//         if ( pLevelData == NULL )
		//             return 0;
		// 
		//         // - 5 ����Ϊ���������߼���ĵ���������ڴ��󣬵�����������ͼӵ㶼�ǰ��ȼ�+1�����
		//         // ����ֱ�Ӿ������˵ȼ�1->2�Ĺ��̣�����Ҫ��1->2�ĵ����۳�������
		//         int levelPoint = pLevelData->Data[g_Script.m_pPlayer->m_byLevel].wPoint - 5;
		// 
		//         pointCount += levelPoint;
		// 
		//         // ������
		//         // С�����������
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

		// ȷ����������
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

		// �����Ҫ��ָ���ļ���ȡ�Ļ�
		if ( lua_type( L, 2 ) == LUA_TSTRING )
		{
			filename = lua_tostring( L, 2 );
			HANDLE hFile = CreateFile( filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL );
			if ( hFile == INVALID_HANDLE_VALUE )
				return 0; // ���û���ļ����򷵻�һ��������

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

		// �ȱ�� flag Ϊ�쳣�����������������˺��ٻָ�
		// ������������г��ִ��������ݱ��治�ˣ�������������߼����󣡣���
		teBuffer->flags |= STaskExBuffer::ERROR_IN_SERIAL;

		int top = lua_gettop( L );

		// �ȼ����������Ƿ���ڸö���
		lua_getglobal( L, "dbMgr" );
		if  ( !lua_istable( L, -1 ) )
			goto __error_exit;

		lua_pushvalue( L, 1 );
		lua_rawget( L, -2 ); // keyɾ�������������
		if ( lua_type( L, -1 ) > LUA_TNIL )
			goto __error_exit;

		lua_pop( L, 2 ); // ��ָ��������dbMgr��ջ
		assert( lua_gettop( L ) == top );

		lua_createtable( L, 0, 0 ); // Ȼ�󴴽���������ı�����Ϊ�ӱ�
		if ( !lua_istable( L, -1 ) )
			goto __error_exit;

		// ÿ�����ݱ��÷�Ϊ2�����֣���ʱ�ĺ���Ҫ�����
		if ( teBuffer->saveData )
		{
			//lua_createtable( L, 0, 0 ); // ����data�ӱ�
			//if ( !lua_istable( L, -1 ) )
			//    goto __error_exit;

			// ����ǿ����ݣ�������ǰ�ϵ�liteserail�߼����ݣ���ֱ�ӷ���һ���ձ�
			if ( *( LPDWORD )teBuffer->saveData == 0 || *( LPDWORD )teBuffer->saveData == 0x00040004 )
				lua_createtable( L, 0, 0 ); // ����data�ӱ�
			else {
				int ck = luaEx_unserialize( L, ( LPBYTE )teBuffer->saveData + 4, *( LPDWORD )teBuffer->saveData );
				if ( ck <= 0 || ck != *( LPDWORD )teBuffer->saveData )
					goto __error_exit;
			}
			lua_setfield( L, -2, "data" );
		}

		if ( teBuffer->tempData )
		{
			//lua_createtable( L, 0, 0 ); // ����temp�ӱ�
			//if ( !lua_istable( L, -1 ) )
			//    goto __error_exit;

			if ( *( LPDWORD )teBuffer->tempData == 0 || *( LPDWORD )teBuffer->tempData == 0x00040004 )
				lua_createtable( L, 0, 0 ); // ����data�ӱ�
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

		// ȷ����������
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

		// �����Ҫд�뵽�ļ��Ļ�
		if ( lua_type( L, 2 ) == LUA_TSTRING )
		{
			filename = lua_tostring( L, 2 );
			teBuffer = &tpBuffer;
		}

		// �Ƿ��ڸ��º����lua����
		BOOL doClean = ( lua_tonumber( L, 3 ) == 1 );

		if ( teBuffer == NULL )
			return 0;

		if ( teBuffer->flags & STaskExBuffer::ERROR_IN_SERIAL )
			luaL_error( L, "serial op get bad buffer!" );

		// �ȱ�� flag Ϊ�쳣�����������������˺��ٻָ�
		// ������������г��ִ��������ݱ��治�ˣ�������������߼����󣡣���
		teBuffer->flags |= STaskExBuffer::ERROR_IN_SERIAL;

		int top = lua_gettop( L );

		// ��ȡ������
		lua_getglobal( L, "dbMgr" );
		if  ( lua_istable( L, -1 ) )
		{
			// ��ȡָ�����ݱ�
			lua_pushvalue( L, 1 );
			lua_rawget( L, -2 ); // keyɾ�������������
			BOOL isTable = lua_istable( L, -1 );
			if ( isTable )
			{
				// �ֱ�ȷ���ɱ�������ݴ�С
				size_t size1 = ( teBuffer->flags & STaskExBuffer::SIZE1_FIXED ) ? 
					teBuffer->saveSize : sizeof( sdbBuffer );
				size_t size2 = ( teBuffer->flags & STaskExBuffer::SIZE2_FIXED ) ? 
					teBuffer->tempSize : sizeof( tdbBuffer );

				int prev1 = 0;
				int prev2 = 0; 
				int cur1  = 0;
				int cur2  = 0; 

				// ÿ�����ݱ��÷�Ϊ2�����֣���ʱ�ĺ���Ҫ�����
				// ��������ݣ���������ݱ��棡
				// ###########################################################################################
				// ע�⣬������Ҫ���ǵ����������Ҳ���������������д������ݴ�����ôԭʼ����Ҳ�����������
				// ���Ժ����滻Ϊ�ˣ������л�����ʱ�ռ䣬Ȼ���ٿ�����ʵ��������
				// ###########################################################################################
				// ����lua��jex��ԭ��֧��serialize
				lua_getfield( L, -1, "data" );
				if ( lua_istable( L, -1 ) )
				{
					if ( teBuffer->saveData != NULL ) {
						prev1 = *( LPDWORD )teBuffer->saveData;
						if ( HIWORD(prev1) ) // �ݴ���������ǰ�������л�����0x00040004���´���
							prev1 = 0;
					}
					cur1 = luaEx_serialize( L, -1, sdbBuffer + 4, (int)size1 - 4 );
					if ( cur1 < 0 )
						goto __error_exit;
					*( LPDWORD )sdbBuffer = cur1;
					cur1 += 4;
				}
				lua_pop( L, 1 );

				// ��������ݣ���������ݱ��棡
				lua_getfield( L, -1, "temp" );
				if ( lua_istable( L, -1 ) )
				{
					if ( teBuffer->tempData != NULL ) {
						prev2 = *( LPDWORD )teBuffer->tempData;
						if ( HIWORD(prev2) ) // �ݴ���������ǰ�������л�����0x00040004���´���
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
					// ���л�������ݴ�С���ܵ������ݿռ�仯
					if ( cur1 > teBuffer->saveSize )
					{
						delete teBuffer->saveData;
						prev1 = cur1 + ( cur1 >> 3 );
						if ( prev1 & 0xf ) // 16�ֽڶ���
							prev1 = ( prev1 & ~0xf ) + 0x10;
						teBuffer->saveSize = prev1;
						teBuffer->saveData = new char[ prev1 ];
					}

					// ���л�������ݸ�ֵ
					if ( teBuffer->saveData )
					{
						memcpy( teBuffer->saveData, sdbBuffer, cur1 );
						if ( cur1 < prev1 ) // ��������С��ԭ�����ݴ�С������£����ԭ�е�������
							memset( ( LPBYTE )teBuffer->saveData + cur1, 0, prev1 - cur1 );
					}

					// ���л�������ݴ�С���ܵ������ݿռ�仯
					if ( cur2 > teBuffer->tempSize )
					{
						delete teBuffer->tempData;
						prev2 = cur2 + ( cur2 >> 3 );
						if ( prev2 & 0xf ) // 16�ֽڶ���
							prev2 = ( prev2 & ~0xf ) + 0x10;
						teBuffer->tempSize = prev2;
						teBuffer->tempData = new char[ prev2 ];
					}

					// ���л�������ݸ�ֵ
					if ( teBuffer->tempData )
					{
						memcpy( teBuffer->tempData, tdbBuffer, cur2 );
						if ( cur2 < prev2 ) // ��������С��ԭ�����ݴ�С������£����ԭ�е�������
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

			// �Ƿ��ڳɹ������󣬽���������������������Ҫ���ڽ�ɫ�����ϣ���Ϊ��ɫ���ߺ��ǲ�Ӧ�ö������ݽ��з��ʵġ���
			if ( doClean && isTable )
			{
				lua_pushvalue( L, 1 );
				lua_pushnil( L );
				lua_rawset( L, -3 );
			}
		}
		else
		{
			// ������ݣ������������ݱ�����������±��������쳣������
			teBuffer->flags &= ~STaskExBuffer::ERROR_IN_SERIAL;
		}
		lua_pop( L, 1 );

		assert( lua_gettop( L ) == top );

__error_exit:
		if ( lua_gettop( L ) != top )
			lua_settop( L, top );

		// �Ƿ��д����־
		if ( teBuffer->flags & STaskExBuffer::ERROR_IN_SERIAL )
			return 0;

		lua_pushboolean( L, true );
		return 1;
	}

	// added by xhy,copy from L_SaveDBTable
	// TODO:Ŀǰֻ���������񶼷��͹�ȥ���Ժ���Ҫ�޸�ֻ���ͷ����ı������
	static int L_SyncTaskData(lua_State* L)
	{
		STaskExBuffer tpBuffer;
		STaskExBuffer *teBuffer = NULL;


		// ȷ����������
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

		// �Ƿ��ڸ��º����lua����
		BOOL doClean = ( lua_toboolean( L, 3 ) == 1 );

		if ( teBuffer == NULL )
			return 0;

		if ( teBuffer->flags & STaskExBuffer::ERROR_IN_SERIAL )
			luaL_error( L, "serial op get bad buffer!" );

		// �ȱ�� flag Ϊ�쳣�����������������˺��ٻָ�
		// ������������г��ִ��������ݱ��治�ˣ�������������߼����󣡣���
		teBuffer->flags |= STaskExBuffer::ERROR_IN_SERIAL;

		int top = lua_gettop( L );

		// ��ȡ������
		lua_getglobal( L, "dbMgr" );
		if  ( lua_istable( L, -1 ) )
		{
			// ��ȡָ�����ݱ�
			lua_pushnumber( L, player_sid );
			lua_rawget( L, -2 ); // keyɾ�������������
			BOOL isTable = lua_istable( L, -1 );
			if ( isTable )
			{
				// �ֱ�ȷ���ɱ�������ݴ�С
				size_t size1 = ( teBuffer->flags & STaskExBuffer::SIZE1_FIXED ) ? 
					teBuffer->saveSize : sizeof( sdbBuffer );

				int prev1 = 0;
				int cur1  = 0;

				// ÿ�����ݱ��÷�Ϊ2�����֣���ʱ�ĺ���Ҫ�����
				// ��������ݣ���������ݱ��棡
				// ###########################################################################################
				// ע�⣬������Ҫ���ǵ����������Ҳ���������������д������ݴ�����ôԭʼ����Ҳ�����������
				// ���Ժ����滻Ϊ�ˣ������л�����ʱ�ռ䣬Ȼ���ٿ�����ʵ��������
				// ###########################################################################################
				// ����lua��jex��ԭ��֧��serialize
				lua_getfield( L, -1, "data" );
				if ( lua_istable( L, -1 ) )
				{
					if ( teBuffer->saveData != NULL ) {
						prev1 = *( LPDWORD )teBuffer->saveData;
						if ( HIWORD(prev1) ) // �ݴ���������ǰ�������л�����0x00040004���´���
							prev1 = 0;
					}
					cur1 = luaEx_serialize( L, -1, sdbBuffer + 4, (int)size1 - 4 );
					if ( cur1 < 0 )
						goto __error_exit;
					*( LPDWORD )sdbBuffer = cur1;
					cur1 += 4;
				}
				lua_pop( L, 1 );


				// ���л�������ݴ�С���ܵ������ݿռ�仯
				if ( cur1 > teBuffer->saveSize )
				{
					delete teBuffer->saveData;
					prev1 = cur1 + ( cur1 >> 3 );
					if ( prev1 & 0xf ) // 16�ֽڶ���
						prev1 = ( prev1 & ~0xf ) + 0x10;
					teBuffer->saveSize = prev1;
					teBuffer->saveData = new char[ prev1 ];
				}

				// ���л�������ݸ�ֵ
				if ( teBuffer->saveData )
				{
					memcpy( teBuffer->saveData, sdbBuffer, cur1 );
					if ( cur1 < prev1 ) // ��������С��ԭ�����ݴ�С������£����ԭ�е�������
						memset( ( LPBYTE )teBuffer->saveData + cur1, 0, prev1 - cur1 );
				}

			}
			lua_pop( L, 1 );

			teBuffer->flags &= ~STaskExBuffer::ERROR_IN_SERIAL;

			// �Ƿ��ڳɹ������󣬽���������������������Ҫ���ڽ�ɫ�����ϣ���Ϊ��ɫ���ߺ��ǲ�Ӧ�ö������ݽ��з��ʵġ���
			if ( doClean && isTable )
			{
				lua_pushvalue( L, 1 );
				lua_pushnil( L );
				lua_rawset( L, -3 );
			}
		}
		else
		{
			// ������ݣ������������ݱ�����������±��������쳣������
			teBuffer->flags &= ~STaskExBuffer::ERROR_IN_SERIAL;
		}
		lua_pop( L, 1 );

		assert( lua_gettop( L ) == top );

		// �������ݵ��ͻ���(teBuffer->saveData,teBuffer->saveSize)
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

		// �Ƿ��д����־
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

	/// ����ɱ�������еĹ���
	/// ����: ����id �������� ���������Ƴ�����
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
						return 0;   // ������ظ���ӣ���ʧ�ܷ���
					i->second.erase( it );
					lua_pushboolean( L, true );
					return 1;       // �����ɾ�����򷵻سɹ�
				}
			}
		}

		if ( !isAdd ) 
			return 0; // �����ɾ������ʧ�ܷ���
		g_Script.m_pPlayer->taskKillMonsters[monsterName].push_back( dwTaskId );
		lua_pushboolean( L, true );
		return 1;       // �������ӣ��򷵻سɹ�
	}

	/// ���ݹ���Id��ȡ���������
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
		rfalse("װ���ȼ�Ϊ%d", level);

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
			GenerateEquipAttType(&attType,&curattType);		// ��������
			GenerateEquipAttValue(&attValue,curattType);	// ���Ծ���ֵ

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

		RegisterFunction( LoadDBTable               );  // ��ָ��������ļ���ȡ���ݣ��������ݱ��nil
		RegisterFunction( SaveDBTable               );  // ��ָ�����ݱ��浽ָ��������ļ������ز������
		RegisterFunction( SetRegionBatchrRefresh    );  // ����ĳ��������ˢ�ֹ��� �رգ��ָ����ر���������

		RegisterFunction( SyncTaskData              );  // ͬ���������ݵ��ͻ���,added by xhy

		RegisterFunction( MarkTaskKillMonster		);	// ����ɱ������Ĺ���
		RegisterFunction( GetMonsterNameById		);	// ���ݹ���id��ȡ��������
		RegisterFunction( ActiveNoticeBroadcast		);
#undef  RegisterFunction

		RegisterScriptFunctions1();
		RegisterScriptFunctions_EventRegion();
	}
}


