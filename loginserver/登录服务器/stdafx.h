// stdafx.h : 标准系统包含文件的包含文件，
// 或是常用但不常更改的项目特定的包含文件
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
// Windows 头文件：
#include <windows.h>
#include <windowsx.h>

// C 运行时头文件
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "resource.h"

// TODO: 在此处引用程序要求的附加头
#include <commctrl.h>
#include <mmsystem.h>
#include <shlwapi.h>
//#include <comdef.h>
#include <winioctl.h>
#include <Aclapi.h>
#include <ntsecapi.h>

#include <map>
#include <list>

#include "engine.h"
#include "globals.h"

#define USING_PAKSTREAM
#include "pub\dwt.h"
//#include "pub\__STException.h"
#include "pub/ComplierConfigure.h"

BOOL rfalse(LPCSTR format, ...);
BOOL rfalse(char lTraceType, char bUseEndl, LPCSTR szFormat, ...);

inline unsigned __int64 GetCpuCycle();
unsigned __int64 CycleSeg();
unsigned __int64 CycleEnd();
unsigned __int64 GetCpuSpeed();
