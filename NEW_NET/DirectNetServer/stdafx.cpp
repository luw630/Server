// stdafx.cpp : 只包括标准包含文件的源文件
// DirectNetServer.pch 将成为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

#ifndef _DEBUG
// TODO: 在 STDAFX.H 中
//引用任何所需的附加头，而不是在此文件中引用

#include <cstdlib>
#include <new>

_C_LIB_DECL
int __cdecl _callnewh(size_t size) _THROW1(_STD bad_alloc);
_END_C_LIB_DECL

void *__cdecl operator new(size_t size) _THROW1(_STD bad_alloc)
{
    // try to allocate size bytes
    void *p;
    while ((p = malloc(size)) == 0)
	    if (_callnewh(size) == 0)
            break;

    if (p == 0)
    {
       /* _STD _Nomemory();*/
		return NULL;
    }

    return (p);
}

#endif
