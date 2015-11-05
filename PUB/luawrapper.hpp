#pragma once

#include "lite_lualibrary.hpp"

class LuaFunctor
{
public:
    LuaFunctor( lua_State *ls, LPCSTR funcName ) :
        funcName( funcName ), callStack( ls ), stackTop( lua_gettop( callStack ) )
    {
        lua_getglobal( callStack, funcName );
        if ( initFailre = lua_isnil( callStack, -1 ) ) 
            lua_settop( callStack, stackTop );
    }

    LuaFunctor& operator [] ( lite::Variant const &lvt )
    {
        if ( initFailre )
            return *this;

        if ( initFailre = !reinterpret_cast< lite::lua_variant const & >( lvt ).push( callStack ) )
            lua_settop( callStack, stackTop );

        return *this;
    }

    // @nResult ����ʱ����ϣ����ȡ�ķ���ֵ����
    //          ����ʱ��¼��ʵ�ʵõ��Ĳ�������
    // @rArray  ����ֵ����, ����С�ڵ��ڴ���ʱnResultָ��������
    //          ����ʱ, ���ʵ�ʷ����������ڵ���nResultָ������, rArray��ȫ������, ��֮, ֻ������Ч����!
    BOOL operator () ( int nResult = 0, lite::Variant *rArray = NULL )
    {
        if ( initFailre )
            return FALSE;

        int paramSize = lua_gettop( callStack ) - stackTop - 1;
        if ( paramSize < 0 )
            return FALSE;

        if ( lua_pcall( callStack, paramSize, nResult, 0 ) != 0 )
        {
            char err[1024];
            sprintf( err, "%s\r\n", lua_tostring( callStack, -1 ) );
            OutputDebugString( err );

            lua_settop( callStack, stackTop );
            return FALSE;
        }

        int resultMax = lua_gettop( callStack ) - stackTop;
        if ( resultMax < 0 )
            throw exception( "�����lua���ý����ջ�����쳣��" );

        for ( int i = 0; i < min( resultMax, nResult ); i ++ )
            reinterpret_cast< lite::lua_variant& >( rArray[i] ).get( callStack, stackTop + i + 1 );

        lua_settop( callStack, stackTop );

        return TRUE;
    }

private:
    BOOL            initFailre;
    lua_State       *callStack;
    const LPCSTR    funcName;
    const int       stackTop;
};
