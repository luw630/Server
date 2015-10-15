// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���ǳ��õ��������ĵ���Ŀ�ض��İ����ļ�
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����

// TODO: �ڴ˴����ó���Ҫ��ĸ���ͷ
#define USING_PAKSTREAM
#include "pub/dwt.h"
#include "networkmodule/netprovidermodule/netprovidermodule.h"
#include <windows.h>
#include <WinNT.h>
#include <WinDef.h>
#include "LiteSerializer/lite.h"

#include "Shlwapi.h"
#pragma comment(lib, "Shlwapi.lib")

#include <math.h>

#include <fstream>
#include <list>
#include <vector>
#include <string>
using namespace std;

#include <string.h>

#include <Engine.h>
#include <IniFile.h>

BOOL rfalse(LPCSTR format, ...);
BOOL rfalse(char, char, LPCSTR, ...);

unsigned __int64 CycleSeg();
unsigned __int64 CycleEnd();
unsigned __int64 GetCpuSpeed();

extern double Fpc[100];
extern DWORD frames;

#ifdef _DEBUG
#include <crtdbg.h>
#undef _AFX_NO_DEBUG_CRT
//#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif _DEBUG
typedef const LPBYTE LPCBYTE;

#define DOTHROW
extern LPSTR g_AnsiToUtf8(const char* AnsiStr);
extern LPSTR g_Utf8ToAnsi(const char* Utf8Str);
extern int g_StoreMessage(DNID dnidClient, LPVOID pMsg, DWORD wSize);
extern void BroadcastMsg(struct SMessage *pMsg, DWORD wSize);
extern LPCSTR GetStringTime();
extern LPCSTR FormatString(LPCSTR szFormat, ...);
extern int SendToLoginServer(SMessage *data, long size);
extern void SendMessageToPlayer(LPCSTR szName, SMessage *pMsg, DWORD wSize);
extern void TalkToPlayer( LPCSTR, LPCSTR );
extern void TalkToDnid(DNID dnidClient, LPCSTR info,bool addPrefix = false,bool bPickUp = false, WORD wShowType=1);
extern void TalkToAll(LPCSTR info);
extern void g_CutClient(DNID dnidClient);
extern DWORD g_GetClientCount();
extern BOOL g_GetAddrByDnid(DNID dnidClient, sockaddr_in *addr, size_t size);
extern BOOL IsValidName(LPCSTR szName);
extern WORD GetServerID();
extern void AddInfo(LPCSTR Info);

extern DWORD g_StartUpZoneServerTime;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			{ if(p) { delete (p);	(p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete [](p);	(p)=NULL; } }
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(p)			{ if(p) { free(p); (p)=NULL; } }
#endif

#define IN_ODDS(odds)                   (int(odds) >= ((rand()%100)+1))
#define IN_ODDSS(odds)                   (int(odds) >= ((rand()%10000)+1))
#define IN_ODDSSS(odds)                   (int(odds) >= ((rand()%1000)+1))
#define IN_VARODDS(odds, var)              (int(odds) >= ((rand()%var)+1))

//#define MY_ASSERT(exp) if ((exp) == 0)return;
// Ϊ�����3����Ķ���
#ifdef XYDT_RELEASE
	#define MY_ASSERT(exp) ((void)(0))
#else
	extern "C" void __cdecl _assert(const char *, const char *, unsigned);
	#define MY_ASSERT(exp) (void)((exp) || (_assert(#exp, __FILE__, __LINE__), 0))
#endif

//�����3��˶���
// #define XYD_DC
// #ifdef XYD_DC
// #define XYDDC 1
// #else
// #define XYDDC 0
// #endif

// Ϊ�����3����ĵ��Է��ű��
extern bool g_InDebugMode;

#pragma warning(disable : 4100)
#pragma warning(disable : 4189)
#pragma warning(disable : 4996)
#pragma warning(disable : 4244)