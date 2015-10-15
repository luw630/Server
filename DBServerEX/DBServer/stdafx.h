// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���ǳ��õ��������ĵ���Ŀ�ض��İ����ļ�
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����
#define _WIN32_WINNT 0x400

// Windows ͷ�ļ���
#include <windows.h>
#include <windowsx.h>

// C ����ʱͷ�ļ�
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// TODO: �ڴ˴����ó���Ҫ��ĸ���ͷ
#include <commctrl.h>
#include <mmsystem.h>
#include <shlwapi.h>
#include "pub\stlini.h"
#include "pub/traceinfo.h"

#include "resource.h"

#include "pub\dwt.h"
#include "pub\runmark.h"
#include "pub\thread.h"
#include "networkmodule\playertypedef.h"
//
//namespace typedef_v2 {
//    #include "networkmodule\typede_v1\typedef.h"
//};
//
//namespace typedef_v1 {
//    #include "networkmodule\typede_v1\typedef.h"
//};

BOOL rfalse(LPCSTR format, ...);
BOOL rfalse(char lTraceType, char bUseEndl, LPCSTR szFormat, ...);
void trace(char lTraceType, char bUseEndl);

unsigned __int64 CycleSeg();
unsigned __int64 CycleEnd();
unsigned __int64 GetCpuSpeed();

typedef unsigned __int64 DNID;
