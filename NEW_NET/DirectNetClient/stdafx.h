// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���ǳ��õ��������ĵ���Ŀ�ض��İ����ļ�
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����

#define _WIN32_WINNT    0x0400
#define _WIN32_WINDOWS  0x0410
#define WINVER          0x0400

// Windows ͷ�ļ���
#include <windows.h>

#include <stdio.h>
// TODO: �ڴ˴����ó���Ҫ��ĸ���ͷ

#define DIRECTNET_API __declspec(dllexport)
#include "../DirectNet.h"

#include <assert.h>
#include <process.h>

#include <fstream>

#ifdef _DEBUG
#include <iomanip>
static std::ofstream _dout("DirectNetClient.log", std::ios_base::trunc);
#define _endl std::endl
#define DEC(i) std::dec << i
#define HEX(i) std::hex << std::setw(2*sizeof(i)) << std::setfill('0') << ((unsigned int)i)
#define DEBUG_ONLY(xxx) (xxx)
#else
#define DEBUG_ONLY(xxx) ( (void)0 )
#endif // _DEBUG

#pragma comment (lib, "ws2_32.lib")
