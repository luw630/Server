#pragma once

extern "C" {
#include "lualib.h"
}

#include "liteserializer/lite.h"
#include "lite_lualibrary.hpp"

struct SSDecoder
{
    static struct LS
    {
        LS() { ls = lua_open(); luaL_openlibs( ls ); }
        ~LS() { lua_close( ls ); }
        operator lua_State* () { return ls; }
        lua_State *ls;
    };

    static lua_State* ls() { static LS ls; return ls; }

    SSDecoder( void *data, int idx = __COUNTER__ ) {
        _idx = -1;

        WORD size = *( LPWORD )data;
        if ( ( size & 0x8000 ) == 0 )
            return; // 错误的数据流

        int rc = luaEx_unserialize( ls(), ( LPBYTE )data + 2, size & 0x7fff );
        if ( rc < 0 )
            return;

        char str[256];
        sprintf( str, "_%d", idx );
        lua_setfield( ls(), LUA_GLOBALSINDEX, str );

        //lua_rawseti( ls(), LUA_GLOBALSINDEX, idx );
        _idx = idx;
    }

    lite::lua_variant operator [] ( LPCSTR key ) const {
        if ( _idx == -1 )
            return lite::lua_variant();

        int top = lua_gettop( ls() );

        char str[256];
        sprintf( str, "_tmp = _%d%s", _idx, key );
        if ( luaL_loadstring( ls(), str ) == 0 ) {
            if ( lua_pcall( ls(), 0, 0, 0 ) == 0 ) {
                lua_getfield( ls(), LUA_GLOBALSINDEX, "_tmp" );
                lite::lua_variant rc( ls(), -1 );
                lua_settop( ls(), top );
                return rc;
            }
        }

        if ( LPCSTR err = lua_tostring( ls(), -1 ) )
        {
            sprintf( str, "%s\r\n", err );
            OutputDebugString( str );
        }

        lua_settop( ls(), top );
        return lite::lua_variant();
    }

	/// 判断key是不是一个table
	bool CheckTable( LPCSTR key ) const {
		if ( _idx == -1 || key == NULL )
			return false;

		int top = lua_gettop( ls() );

		char str[256];
		sprintf( str, "_tmp = _%d%s", _idx, key );
		if ( luaL_loadstring( ls(), str ) == 0 ) {
			if ( lua_pcall( ls(), 0, 0, 0 ) == 0 ) {
				lua_getfield( ls(), LUA_GLOBALSINDEX, "_tmp" );
				bool ret = (lua_type( ls(), -1 ) == LUA_TTABLE);
				lua_settop( ls(), top );
				return ret;
			}
		}

		if ( LPCSTR err = lua_tostring( ls(), -1 ) )
		{
			sprintf( str, "%s\r\n", err );
			OutputDebugString( str );
		}

		lua_settop( ls(), top );
		return false;
	}

    int _idx;
};
