// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���ǳ��õ��������ĵ���Ŀ�ض��İ����ļ�
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ���
#include <windows.h>
#include <windowsx.h>

// C ����ʱͷ�ļ�
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "resource.h"

// TODO: �ڴ˴����ó���Ҫ��ĸ���ͷ
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
