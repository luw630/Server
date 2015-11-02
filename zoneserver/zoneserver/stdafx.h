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

#include <commctrl.h>
#include <mmsystem.h>
#include <shlwapi.h>

#include "resource.h"

// TODO: 在此处引用程序要求的附加头
#include "NETWORKMODULE\NetProviderModule\NetProviderModuleAdv.h"

#include <Win32.h>
#include <Inifile.h>

#define USING_PAKSTREAM
#include <pub\dwt.h>
#include "LITESERIALIZER/lite.h"

#include <fstream>
#include <list>
#include <string>
using namespace std;

BOOL rfalse(LPCSTR format, ...);
BOOL rfalse(char lTraceType, char bUseEndl, LPCSTR szFormat, ...);

unsigned __int64 CycleSeg();
unsigned __int64 CycleEnd();
unsigned __int64 GetCpuSpeed();

#pragma comment (lib, "Winmm.Lib")
#pragma comment (lib, "WS2_32.Lib")
//#ifndef _DEBUG
#pragma comment (lib, "EngineLib.Lib")
//#else
//#pragma comment (lib, "EngineLibDbg.Lib")
//#endif

#ifdef _DEBUG
#include <crtdbg.h>
#undef _AFX_NO_DEBUG_CRT
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif _DEBUG
typedef const LPBYTE LPCBYTE;

#define DOTHROW
extern LPSTR g_AnsiToUtf8(const char* AnsiStr);
extern LPSTR g_Utf8ToAnsi(const char* Utf8Str);
extern int g_StoreMessage(DNID dnidClient, LPVOID pMsg, DWORD wSize);
extern void BroadcastMsg(struct SMessage *pMsg, DWORD wSize);
extern void TalkToDnid(DNID dnidClient, LPCSTR info,bool addPrefix = true,bool bPickUp = false,WORD wShowType=1);
extern void TalkToAll(LPCSTR info);
extern void g_CutClient(DNID dnidClient);

extern DWORD g_StartUpZoneServerTime;

#pragma warning(disable : 4996)