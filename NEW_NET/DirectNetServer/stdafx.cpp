// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// DirectNetServer.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

#include "stdafx.h"

#ifndef _DEBUG
// TODO: �� STDAFX.H ��
//�����κ�����ĸ���ͷ���������ڴ��ļ�������

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
