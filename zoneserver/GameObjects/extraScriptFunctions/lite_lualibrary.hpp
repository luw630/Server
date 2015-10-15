#pragma once

#include < stack >

extern "C" {
#include "lauxlib.h"
}

namespace lite
{
    class lua_variant : public lite::Variant
    {
    public:
        lua_variant() {}

        // 这个构造函数属于特殊处理，不负责数据的中转，所以使用这种方式创建出来的对象需要尽快使用！
        lua_variant( lua_State *L, int index ) 
        {
            switch ( lua_type( L, index ) )
            {
            case LUA_TNIL:
                dataType = lite::Variant::VT_NULL;
                break;

            case LUA_TBOOLEAN:
                dataType = lite::Variant::VT_INTEGER;
                dataSize = 4;
                _integer = ( int )lua_toboolean( L, index );
                break;

            case LUA_TNUMBER:
                dataType = lite::Variant::VT_INTEGER;
                dataSize = 4;
                _integer = ( int )lua_tonumber( L, index );
                break;

            case LUA_TSTRING:
                {
                    LPCSTR str = lua_tostring( L, index );
                    if ( str )
                    {
                        dataType = VT_STRING;
                        dataSize = static_cast< WORD >( strlen( str ) + 1 );
                        _string = str;
                    }
                }
                break;
            case LUA_TLIGHTUSERDATA:
            case LUA_TUSERDATA:
                {
                    if ( luaEx_isint63( L, index ) )
                    {
                        unsigned __int64 v = luaEx_toint63( L, index );
                        lite::Variant::DeepCopy( lite::Variant( v ) );
                    }
                    else
                    {
                        int size = 0;
                        LPCVOID data = luaEx_touserdata( L, index, &size );
                        if ( data && size && ( size < 0xffff ) )
                        {
                            dataType = VT_POINTER;
                            dataSize = size;
                            _pointer = data;
                        }
                    }
                }
                break;
            }
        }

    public:
        BOOL get( lua_State *L, int index )
        {
            // 当前variant已经获取过有效数据了
            if ( dataType != lite::Variant::VT_EMPTY )
                return FALSE;

			int luatype =  lua_type( L, index );
            switch ( lua_type( L, index ) )
            {
            case LUA_TNIL:
                dataType = lite::Variant::VT_NULL;
                break;

            case LUA_TBOOLEAN:
                dataType = lite::Variant::VT_INTEGER;
                dataSize = 4;
                _integer = ( int )lua_toboolean( L, index );
                break;

            case LUA_TNUMBER:
                {
                    double tempNumber = lua_tonumber( L, index );
                    if ( ( tempNumber >= INT_MAX ) || ( tempNumber <= INT_MIN ) ||
                        ( tempNumber != 0 && tempNumber > -1 && tempNumber < 1 ) )
                        AllocateAndAssign( lite::Variant::VT_DOUBLE, &tempNumber, 8 );
                    else
                    {
                        dataType = lite::Variant::VT_INTEGER;
                        dataSize = 4;
                        _integer = ( int )tempNumber;
                    }
                }
                break;

            case LUA_TSTRING:
                {
                    LPCSTR str = lua_tostring( L, index );
                    if ( str )
                        AllocateAndAssign( lite::Variant::VT_STRING, str, strlen( str ) + 1 );
                }
                break;

            case LUA_TLIGHTUSERDATA:
            case LUA_TUSERDATA:
                {
                    if ( luaEx_isint63( L, index ) )
                    {
                        unsigned __int64 v = luaEx_toint63( L, index );
                        lite::Variant::DeepCopy( lite::Variant( v ) );
                    }
                    else
                    {
                        int size = 0;
                        LPCVOID data = luaEx_touserdata( L, index, &size );
                        if ( data && size && ( size < 0xffff ) )
                            AllocateAndAssign( lite::Variant::VT_POINTER, data, size );
                    }
                }
                break;
			case LUA_TTABLE:
				{
					rfalse(2,1,"LUA_TTABLE 未处理");
				}
				break;
            default:
                return FALSE;
            }

            return TRUE;
        }

        BOOL push( lua_State *L ) const
        {
            switch ( dataType )
            {
            case lite::Variant::VT_EMPTY:
            case lite::Variant::VT_NULL:
                lua_pushnil( L );
                break;

            case lite::Variant::VT_DWORD:
            case lite::Variant::VT_WORD:
            case lite::Variant::VT_U32_24:
                lua_pushnumber( L, operator unsigned long () );
                break;

            case lite::Variant::VT_INTEGER:
            case lite::Variant::VT_SHORTINTEGER:
            case lite::Variant::VT_I32_24:
                lua_pushnumber( L, operator __int32 () );
                break;

            case lite::Variant::VT_DOUBLE:
                lua_pushnumber( L, operator double () );
                break;

            case lite::Variant::VT_FLOAT:
                lua_pushnumber( L, _float );
                break;

            case lite::Variant::VT_STRING:
                lua_pushstring( L, _string );
                break;

            case lite::Variant::VT_LARGINTEGER:
            case lite::Variant::VT_I64_56:
            case lite::Variant::VT_I64_24:
                luaEx_pushint63( L, operator __int64 () );
                break;

            case lite::Variant::VT_QWORD:
            case lite::Variant::VT_U64_56:
            case lite::Variant::VT_U64_24:
                luaEx_pushint63( L, operator unsigned __int64 () );
                break;

            case lite::Variant::VT_POINTER:
                {
                    if ( dataSize > 0xffff )
                        return FALSE;

                    void *p = lua_newuserdata( L, dataSize );
                    memcpy( p, _pointer, dataSize );
                }
                break;

            default:
                return FALSE;
            }

            return TRUE;
        }
    };

    class table_iterator
    {
    public:
        table_iterator() : lua_stack( NULL ), table_index( 0 ) 

    #ifdef _DEBUG
            , stack_top( 0 )
    #endif

        {}

        table_iterator( const table_iterator &other ) : 
            lua_stack( other.lua_stack ), table_index( other.table_index ) 

    #ifdef _DEBUG
            , stack_top( other.stack_top )
    #endif

        { 
            const_cast< lua_State*& >( other.lua_stack ) = NULL; 
        }

        table_iterator( lua_State *stack, int index )
        {
            lua_stack = stack;
            table_index = index;

    #ifdef _DEBUG
            stack_top = lua_gettop( lua_stack );
    #endif

            // 判断指定对象是否是一个有效的table
            if ( !lua_istable( lua_stack, table_index ) )
            {
                lua_stack = NULL;
                return;
            }

            lua_pushnil( lua_stack );
            lua_pushnil( lua_stack );

            if ( table_index < 0 )
                table_index -= 1;
        }

        ~table_iterator()
        {
            if ( lua_stack != NULL )
            {
                lua_pop( lua_stack, 2 );

    #ifdef _DEBUG
                assert( stack_top == lua_gettop( lua_stack ) );
    #endif
                lua_stack = NULL;
            }
        }

        bool isValid() { return lua_stack != NULL; }

        bool operator ++ ( int )
        {
            lua_pop( lua_stack, 1 );

            if ( lua_next( lua_stack, table_index ) == 0 )
            {
    #ifdef _DEBUG
                assert( stack_top == lua_gettop( lua_stack ) );
    #endif

                lua_stack = NULL;
                return false;
            }

            return true;
        }

        const static int index_of_key   = -2;
        const static int index_of_value = -1;

    private:
        lua_State   *lua_stack;
        int         table_index;

    #ifdef _DEBUG
        int         stack_top;
    #endif
    };

    class table_iterator_wrapper
    {
    public:
        table_iterator_wrapper( lua_State *stack, LPCSTR table_name ) :
            stack_for_remove( stack ),
            stack_top( lua_gettop( stack ) ),
            inner_iterator( stack, ( lua_getglobal( stack, table_name ), -1 ) )
        {
        }

        ~table_iterator_wrapper()
        {
            inner_iterator.~table_iterator();

            lua_pop( stack_for_remove, 1 );

            if ( stack_top != lua_gettop( stack_for_remove ) )
                rfalse( 2,1, "在table遍历后，出现了错误的lua栈！！！" );
        }

        bool isValid() { return inner_iterator.isValid(); }
        bool operator ++ ( int ) { return inner_iterator ++; }

        const static int index_of_key   = -2;
        const static int index_of_value = -1;

    private:
        lua_State       *stack_for_remove;
        const int       stack_top;
        table_iterator  inner_iterator;
    };

};
