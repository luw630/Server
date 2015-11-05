// stdafx.h : 标准系统包含文件的包含文件，
// 或是常用但不常更改的项目特定的包含文件
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料

#define _WIN32_WINNT 0x0400

// Windows 头文件：
#include <windows.h>

// TODO: 在此处引用程序要求的附加头
#define DIRECTNET_API __declspec(dllexport)
#include "../DirectNet.h"

#include <map>
#include <list>

#include <assert.h>
#include <process.h>

#include <fstream>

#define KEY_EXIT 0xDEADBABE

#ifdef _DEBUG
#include <iomanip>
static std::ofstream _dout("DirectNetServer.log", std::ios_base::trunc);
#define _endl std::endl
#define DEC(i) std::dec << i
#define HEX(i) std::hex << std::setw(2*sizeof(i)) << std::setfill('0') << ((unsigned int)i)
#define DEBUG_ONLY(xxx) (xxx)
#else
#define DEBUG_ONLY(xxx) ( (void)0 )
#endif // _DEBUG

#pragma comment (lib, "ws2_32.lib")
