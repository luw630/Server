// stdafx.h : 标准系统包含文件的包含文件，
// 或是常用但不常更改的项目特定的包含文件
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#define _WIN32_WINNT 0x400

// Windows 头文件：
#include <windows.h>
#include <windowsx.h>

// C 运行时头文件
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// TODO: 在此处引用程序要求的附加头
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
