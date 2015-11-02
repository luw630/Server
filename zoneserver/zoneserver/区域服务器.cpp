// 登陆服务器.cpp : 定义应用程序的入口点。
//

//#include "stdafx.h"
//#include "登陆服务器.h"

// 游戏登陆服务器.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "GMServer.h"
#include <map>
#include <time.h>
#include "区域服务器.h"
#include "DMainApp.h"
#include "gameobjects\ScriptManager.h"
#include "gameobjects\count.h"
#include "gameobjects\player.h"

#include "gameobjects\item.h"
#include "gameobjects\building.h"

#include "Environment.h"
#include "GameObjects\TreasureShop.h"
//#include "pub\__STException.h"

#include "gameobjects\DSpecialPlayerRcd.h"
#include "../pub/ConstValue.h"
#include "networkmodule/GMMsgs.h"
#include "pub/thread.h"
#include "pub/traceinfo.h"
#include "GameObjects/GameWorld.h"
#include "GameObjects/CSkillService.h"
#include "GameObjects/CItemDropService.h"
#include "GameObjects/CMonsterService.h"
#include "GameObjects/CBuffService.h"
#include "GameObjects/Region.h"
#include "GameObjects/Area.h"
#include "GameObjects/SanguoCode/MailMoudle/MailMoudle.h"
#include "GameObjects/SanguoCode/Common/ConfigManager.h"
#include "pub/minihelp.h"
#include "Recharge.h"
extern  CSpecialPlayerRcd    *g_pSpecialPlayerRcd;
extern BOOL LoadNewItemData(LPCSTR filename);
extern size_t GetPlayerNumber();
extern	LPIObject FindRegionByID(DWORD ID);
extern void TalkToAllObj(LPCSTR info);
extern LPCSTR FormatString(LPCSTR szFormat, ...);
void Onsreset(); //在脚本刷新完成以后调用
extern int SendToLoginServer(SMessage *, long size);
//extern const __int32 IID_REGION; 
// 这个数据用来做大部分情况下的缓冲区!
BYTE hugeBuffer[ 0xffff ];
BYTE commBuffer[ 0xfff ];
BYTE tinyBuffer[ 0xff ];

/*
#define START_TIME (0xffffffff - 100 * 1000)

static __int64 i64CounterSegment = 0;
static __int64 i64Frequency = 0;

DWORD WINAPI timeGetTime(void)
{
if (i64Frequency == 0)
{
QueryPerformanceFrequency((LARGE_INTEGER *)&i64Frequency);
QueryPerformanceCounter((LARGE_INTEGER *)&i64CounterSegment);
}

__int64 i64Counter = 0;
QueryPerformanceCounter((LARGE_INTEGER *)&i64Counter);

return START_TIME + (DWORD)((i64Counter - i64CounterSegment) * 1000 / i64Frequency);
}
*/

#define MAX_LOADSTRING 100

// 全局变量：
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

HWND hMainWnd;
HWND hInfo;
HWND hInput;

WNDPROC OldProc;
char InputBuffer[256] = {0};
LRESULT CALLBACK ReloadEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void ExplainCommand(LPCSTR szCommand);

// 此代码模块中包含的函数的前向声明：
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

extern dwt::stringkey<char [256]> szIniFilename;
extern dwt::stringkey<char [256]> szDirname;

int  _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	//将在本目录下生成一个Exception.dmp使用windbg或者直接用vs 打开，启用调试就能定位问题
	//SetUnhandledExceptionFilter(GPTUnhandledExceptionFilter);
	InitMinDump();
	

		MSG msg;
		HACCEL hAccelTable;
		{

		szIniFilename = lpCmdLine;
		if (szIniFilename == "")
			szIniFilename = "server.ini";

		//SYSTEMTIME s;
		//GetLocalTime(&s);
		//sprintf(szDirname, "[%d-%d-%d]", s.wYear, s.wMonth, s.wDay, (LPCSTR)szIniFilename);
		//CreateDirectory( (LPCSTR)szDirname, NULL );
		//strcat( szDirname, "\\" );

		// 初始化全局字符串
		LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadString(hInstance, IDC_MY, szWindowClass, MAX_LOADSTRING);
		strcpy(szWindowClass, "xyd_zoneserver");
		MyRegisterClass(hInstance);

		// 执行应用程序初始化：
		if (!InitInstance(hInstance, nCmdShow))
		{
			return FALSE;
		}

		hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_MY);

		CreateApp();
		if (!GetApp()->InitServer())
		{
			ClearApp();
			return rfalse(0, 0, "初始化失败");
		}

		g_StartUpZoneServerTime = time(NULL);

		//add by ly 2014/10/10 GM工具相关
		//int InitGMFlag = InitGMServer();
		CreateThread(0, 0, g_LoopRechargeGet, 0, 0, 0);//开始充值线程

		// 主消息循环：
		while (!GetApp()->m_bQuit)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
			{
				if (GetMessage(&msg, NULL, 0, 0) == 0)
					break;

				if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			else
			{
				/*if (InitGMFlag != 0)
				{
				LoopGMServer();
				}*/
				if (!GetApp()->LoopServer())
				{
					ClearApp();
					return 0;
				}
			}
		}
		/*if (InitGMFlag != 0)
		{
		ReleaseGMServer();
		}*/

		CRecharge::getSingleton()->ExitRechargeTHD();//退出充值线程
		GetApp()->ExitServer();
		ClearApp();
		Sleep(1000);
	}
	
	return (int) msg.wParam;
}



//
//  函数：MyRegisterClass()
//
//  目的：注册窗口类。
//
//  注释：
//
//    仅当希望在已添加到 Windows 95 的
//    “RegisterClassEx”函数之前此代码与 Win32 系统兼容时，
//    才需要此函数及其用法。调用此函数
//    十分重要，这样应用程序就可以获得关联的
//   “格式正确的”小图标。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (const char *)IDI_MY);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;//(LPCTSTR)IDC_MY;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (const char *)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   函数：InitInstance(HANDLE, int)
//
//   目的：保存实例句柄并创建主窗口
//
//   注释：
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{

	hInst = hInstance; // 将实例句柄存储在全局变量中

	hMainWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		0, 0, 640, 480, NULL, NULL, hInstance, NULL);

	if (!hMainWnd)
	{
		return FALSE;
	}

	RECT rect;
	GetClientRect(hMainWnd, &rect);

#define __input_h 17
#define __input_x 46

	hInfo = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", WS_CHILDWINDOW | WS_VISIBLE | WS_VSCROLL | ES_READONLY | ES_MULTILINE,
		0, 0, rect.right, rect.bottom-__input_h, hMainWnd, (HMENU)IDC_INFO, hInstance, NULL);

	if (!hInfo)
	{
		return FALSE;
	}

	SetWindowFont(hInfo, CreateFont(12,6,0,0,400,0,0,0,0,0,0,0,1,"新宋体"), 1);

	hInput = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", WS_CHILDWINDOW | WS_VISIBLE,
		__input_x, rect.bottom-__input_h, rect.right-__input_x, __input_h, hMainWnd, (HMENU)IDC_INPUT, hInstance, NULL);

	if (!hInput)
	{
		return FALSE;
	}

	SetWindowFont(hInput, CreateFont(12,6,0,0,400,0,0,0,0,0,0,0,1,"新宋体"), 1);

	OldProc = (WNDPROC)(_W64 LONG)SetWindowLong(hInput, GWL_WNDPROC, (LONG)(_W64 LONG)ReloadEditWndProc);

	SetWindowFont(CreateWindowEx(WS_EX_CLIENTEDGE, "Static", "CMD：〉", WS_CHILDWINDOW | WS_VISIBLE | SS_CENTER,
		0, rect.bottom-__input_h, __input_x, __input_h, hMainWnd, (HMENU)IDC_INPUT, hInstance, NULL), CreateFont(12,6,0,0,400,0,0,0,0,0,0,0,1,"新宋体"), 1);

	ShowWindow(hMainWnd, nCmdShow);
	UpdateWindow(hMainWnd);

	return TRUE;
}

//
//  函数：WndProc(HWND, unsigned, WORD, LONG)
//
//  目的：处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//

static bool echo = true;
void AddInfo(LPCSTR Info)
{
	if (!echo)
		return;

	static int max_size = 0;

	int size = dwt::strlen(Info, 10000) + 1;

	int max = (int)SendMessage(hInfo, EM_GETLIMITTEXT, 0, 0);

	if (max_size + size > max)
	{
		int curlc = (int)SendMessage(hInfo, EM_GETLINECOUNT, 0, 0);
		int repcc = (int)SendMessage(hInfo, EM_LINEINDEX, curlc / 10, 0);

		if (repcc < size)
		{
			repcc = -1;
			max_size = 0;
		}
		else
		{
			max_size -= repcc;
		}

		SendMessage(hInfo, EM_SETSEL, 0, repcc);
		SendMessage(hInfo, EM_REPLACESEL, 0, (LPARAM)"");
	}

	max_size += size;

	SendMessage(hInfo, EM_SETSEL, max, max);
	SendMessage(hInfo, EM_REPLACESEL, 0, (LPARAM)Info);

	extern dwt::stringkey<char [256]> szDirname;
	if ( szDirname[0] ) {
		static std::string filename = szDirname + "run.log";
		static time_t prevTick = 0;
		if ( prevTick != time( NULL ) ) {
			prevTick = time( NULL );
			char tickstr[64];
			SYSTEMTIME s;
			GetLocalTime(&s);
			int len = sprintf(tickstr, "*=> %d-%02d-%02d %02d:%02d:%02d\r\n", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);
			TraceData( filename.c_str(), ( LPVOID )tickstr, len );
		}
		TraceData( filename.c_str(), ( LPVOID )Info, size - 1 );
	}
}

LRESULT CALLBACK ReloadEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CHAR)
	{
		if (wParam == 13)
		{
			OldProc(hWnd, message, wParam, lParam);

			PostMessage(hWnd, EM_SETSEL, 0, -1);
			PostMessage(hWnd, WM_CLEAR, 0, 0);
			SendMessage(hWnd, WM_GETTEXT, 250, (LPARAM)InputBuffer);

			InputBuffer[255] = 0;
			size_t end = strlen(InputBuffer);
			if (InputBuffer[end-1] == '\r')
			{
				InputBuffer[end] = '\n';
				InputBuffer[end+1] = 0;
			}
			else if (InputBuffer[end-2] == '\r')
			{
			}
			else
			{
				InputBuffer[end] = '\r';
				InputBuffer[end+1] = '\n';
				InputBuffer[end+2] = 0;
			}

			end = (int)SendMessage(hInfo, EM_GETLIMITTEXT, 0, 0);
			SendMessage(hInfo, EM_SETSEL, end, end);
			SendMessage(hInfo, EM_REPLACESEL, 0, (LPARAM)"CMD：〉");
			//			SendMessage(hInfo, EM_REPLACESEL, 0, (LPARAM)InputBuffer);

			AddInfo(InputBuffer);

			ExplainCommand(InputBuffer);

		}
	}
	else if (message == WM_KEYUP)
	{
		if (wParam == 0x72)
		{
			::SetWindowText(hWnd, InputBuffer);
		}
	}

	return OldProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return GetApp()->MsgProc(hWnd, message, wParam, lParam);
}

#define MAX_PARAM_SIZE 256
#define isParamSplit(x) ((x == '\0') || (x == '\t') || (x == '\r') || (x == '\n') || (x == ' ') || (x == '#'))
LPCSTR GetStringParameter(LPCSTR string, int maxsize, int index)
{
	static char szParameter[MAX_PARAM_SIZE] = "";
	int param_count = 0;
	int size_count = 0;
	int param_seg = 0;
	int param_end = 0;
	int in_param = 0;
	//    if (index == 0) return string;

	index++;

	while (size_count < maxsize)
	{
		if (!isParamSplit(string[size_count]))
		{
			if (in_param == false)
			{
				// begin of parameter
				in_param = 1;
				param_count = 0;
			}
		}
		else if (in_param)
		{
			// end of parameter
			szParameter[param_count] = 0;
			if (--index == 0)
			{
				return szParameter;
			}

			in_param = 0;
			param_count = 0;
		}

		if (string[size_count] == 0)
			break;

		if (in_param)
		{
			szParameter[param_count++] = string[size_count];
			if (param_count >= MAX_PARAM_SIZE)
			{
				rfalse("parameter buffer overflow!");
			}
		}

		size_count ++;
	}

	return NULL;
}

bool  IsParameterString(LPCSTR stringparma, int maxsize)
{
	bool bisstring = false;
	for (size_t i = 0; i < maxsize; i++)
	{
		if (stringparma[i] == 0)
		{
			break;
		}
		if (stringparma[i] <'0' || stringparma[i] > '9')
		{
			bisstring = true;
		}
	}
	return bisstring;
}

class CCommand
{
public:
	char szCommand[32];
	CCommand(LPCSTR command)
	{
		dwt::strcpy(szCommand, command, 32);
	}

	CCommand(const CCommand &dest)
	{
		memcpy(szCommand, dest.szCommand, 32);
	}

	bool operator < (const CCommand &dest) const
	{
		return strcmp(szCommand, dest.szCommand) < 0;
	}

	bool operator > (const CCommand &dest) const
	{
		return strcmp(szCommand, dest.szCommand) > 0;
	}

	bool operator == (const CCommand &dest) const
	{
		return strcmp(szCommand, dest.szCommand) == 0;
	}

	operator size_t () const
	{
		return 1;
	}
};

std::map<CCommand, int> command_map;
//std::hash_map<CCommand, int> command_map; so slow !!!

int InitCommandMap()
{
	command_map["help"]				= 1;
	command_map["quit"]				= 2;
	command_map["list"]				= 3;
	//command_map["type"]				= 4;
	//command_map["d2o"]				= 5;
	command_map["fps"]				= 6;
	command_map["echo"]				= 7;
	command_map["gmtalkmask"]		= 8;
	command_map["bc"]				= 99;
	command_map["sreset"]			= 88;
	command_map["sview"]			= 89;
	command_map["mtsb"]				= 90;
	command_map["playerinfo"]		= 91;
	command_map["ison"] = 92;
	command_map["ison2"] = 93;

	command_map["pk"]				= 100;
	command_map["debug"]			= 101;
	command_map["viewmoney"]		= 98;
	command_map["resetenv"]			= 102;
	command_map["setplayercount"]	= 110;

	command_map["switchsm"]			= 111;
	command_map["setnetlimit"]		= 112;
	command_map["autopackage"]		= 113;

	// reload game data
	command_map["reloaditemrand"]	= 116;

	// Add Record Player
	command_map["rcd"]				= 117;
	command_map["dercd"]			= 118;
	command_map["backup"]			= 119;
	command_map["lua_exec"]			= 120;

	command_map["loadbuild"]				= 121;
	command_map["checkdynamicregion"]		= 122;
	command_map["resetShop"]				= 123;

	command_map["showpalyerutitle"]			= 124;
	command_map["showunioninfo"]			= 125;
	command_map["kickhangupall"]			= 126;

	command_map["checkmac"]					= 130;
	command_map["checkflag"]				= 131;

	command_map["reloadspecialitem"]		= 132;
	command_map["reloadSpecItemAttr"]		= 133;  // Reload special item attribute 
	command_map["loadGMCheckURL"]			= 134;  // 从文件加载GM验证URL

	command_map["loadLevelAttributes"]		= 135;
	command_map["unlimit"]					= 136;  // 多开

	// 侠义道3命令系统
	command_map["kickoneplayer"]		= 399;		// kick一个玩家
	command_map["reloadskill"]			= 400;
	command_map["reloaditem"]			= 401;
	command_map["reloaddropitem"]		= 402;
	command_map["reloadmonster"]		= 403;
	command_map["reloadbuff"]			= 404;
	command_map["kickall"]				= 405;
	command_map["mtdr"]					= 406;
	command_map["setpk"]				= 407;
	command_map["talktoall"]			= 408;
	command_map["人数"]					= 409;
	command_map["createmonster"]		= 410;
	command_map["addmoney"]				= 411;	// 加钱
	command_map["addexp"]				= 412;	// 加经验
	command_map["additem"]				= 413;	// 加物品
	command_map["addpoint"]				= 414;	// 加点
	command_map["opencol"]				= 415;	// 使用地图碰撞
	command_map["closecol"]				= 416;	// 关闭地图碰撞
	command_map["useitemtofriend"]		= 417;  // 对好友使用物品
	command_map["equippet"]				= 418;	// 测试使用侠客装备equippet
	command_map["unequippet"]			= 419;	// 测试使用侠客装备unequippet
	command_map["getitemskill"]			= 420;	// 测试从脚本中返回技能书对应的技能ID
	command_map["fptoitem"]				= 421;	// 测试侠客转换道具
	command_map["calloutfp"]			= 422;	// 测试召唤侠客
	command_map["killmonster"]			= 423;	// 测试删除怪物
	command_map["settaskInfo"]			= 424;	// 设置任务状态
	command_map["createmonsterregin"]	= 425;	// 地图刷怪
	command_map["relive"]				= 426;	// 复活
	command_map["addsp"]				= 427;	// 加真气
	command_map["setlevel"]				= 428;	// 增加玩家等级
	command_map["luacall"]				= 429;	// 调用lua函数
	command_map["setplayergmlevel"] = 430;	//设置玩家GM等级
	command_map["syscall"] = 431;	//发送系统公告
	command_map["print"] = 432;	//打印输出

	////**********************************************////
	////****************萌斗三国系统********************////
	////**********************************************////
	command_map["gm"] = 500;//系统公告
	command_map["getmail"] = 501;//取邮件信息 arg1=玩家id
	command_map["sendmail"] = 502;//发邮件 arg1=玩家id
	command_map["getg"] = 503;//取奖励 arg1=邮件id
	command_map["del"] = 504;//删除邮件 arg1=邮件id
	command_map["smsys"] = 505;//发邮件,系统发全服 
	command_map["wk"] = 506;//测试

	command_map["testjt"] = 507;//帮会功能测试
	command_map["testaddjt"] = 508;//加入帮会功能测试
	command_map["testsave"] = 509;//保存公会到数据库

	command_map["loadfaction"] = 510;//保存公会到数据库
	return 1;
}

int o = InitCommandMap();

double fps_max = 0, cur = 0;
double fps4ls = 0, curls = 0;
double fps4os = 0, curos = 0;
double fps4s = 0, curs = 0;
double fps4g = 0, curg = 0;
//TODO:Tony Modify [2012-3-6]Comment:[加入中心服的]
double fps4cs = 0 , curcs = 0;
///////////////////////////////////////////////////////////////////

bool g_check = false;
bool g_newtype = false;

DWORD cinsize = 0;
DWORD cinsized = 0;
DWORD sinsize = 0;
DWORD sinsized = 0;
DWORD alla = 0, alld = 0;
DWORD soutsize = 0, soutnumber = 0;

DWORD qa = 0, qp = 0, qn = 0, qm = 0, qi = 0, qb = 0, qsi = 0;

DWORD dwNetRunLimit = 1000;

BOOL g_bUsePackageMessage = FALSE;

static DNID tmp[3000];


void ExplainCommand(LPCSTR szCommand)
{
	LPCSTR Param1 = GetStringParameter(szCommand, 256, 0);
	std::map<CCommand, int>::iterator it;

	if (Param1 == NULL) return;

	// 转小写
	_strlwr(const_cast<char*>(Param1));

	// 这样做（不使用[]）可以阻止映射表的动态膨胀
	it = command_map.find(Param1);
	if (it == command_map.end())
	{
		AddInfo("无效的命令\r\n");
		return;
	}

	//	switch((int)command_map[Param1])
	switch(it->second)
	{
	case 1: // help
		AddInfo(/*
			"========================================Help=======================================================\r\n"
			"\t 命令名称		参数				作用\r\n"
			"===================================================================================================\r\n"
			"\r\n"
			"\t help			-				查看命令帮助信息\r\n"
			"\t quit			-				关闭区域服务器\r\n"
			"\t list			-				显示区域服务器当前状态\r\n"
			"\t fps			-				显示当前服务器FPS信息\r\n"
			"\r\n"
			"\t mtsb			角色名称 场景ID x坐标 y坐标	将角色移动到某个场景的特定位置\r\n"
			"\t playerinfo		角色名称			查看该名称角色的信息\r\n"
			"\r\n"
			"\t sreset			-				重新载入【脚本】\r\n"
			"\t reloaditem		-				重新加载【道具表】\r\n"
			"\t reloaddropitem		-				重新加载【物品掉落列表】\r\n"
			"\t reloadskill		-				重新加载【技能配置表】\r\n"
			"\t reloadmonster		-				重新加载【怪物配置表】\r\n"
			"\t reloadbuff		-				重新加载【buff配置表】\r\n"
			"\r\n"
			"\t kickoneplayer		角色名称			踢出特定的一个玩家\r\n"
			"\t kickall		-				踢出当前服务器中所有的玩家\r\n"
			"\t addsp			角色名称 数量			向玩家添加真气\r\n"
			"\t addexp			角色名称 数量			向玩家添加经验\r\n"
			"\t addmoney		角色名称 数量			向玩家添加金钱\r\n"
			"\t additem		角色名称 道具ID 道具数量	向玩家添加道具\r\n"
			"\t addpoint		角色名称 点数类型 增加点数	向玩家添加可用点\r\n"
			"\t							点数类型(1.攻击2.防御3.轻功4.健身)"
			"\r\n"
			"\t killmonster		角色名				杀死当前地图的所有怪物\r\n"
			"\t createmonsterregin	地图ID x坐标 y坐标 怪物ID 数量	特定地图特定坐标刷怪物\r\n"
			"\t relive			角色名 复活类型			复活一个玩家\r\n"
			"\t							复活类型(0.回城 1.原地free 2.原地给钱)\r\n"
			"\t【该服务器不支持动态连接功能】\r\n"
			"==============================================================\r\n"*/
			"萌将三国命令\r\n"
			"\t sendmail		-			发邮件给玩家 arg1=玩家sid\r\n"
			"\t smsys		-				发邮件,系统发全服 \r\n"
			"\t gm		-				全服公告 \r\n"
			);
		break;

	case 2: // quit
		CSingleItem::SaveAllLog();
		GetApp()->m_bQuit = true;
		break;

	case 3: // list
		if (GetStringParameter(szCommand, 256, 1) == NULL)
		{
			// 没有使用附加参数
			GetApp()->DisplayServerInfo(2);

			rfalse(2, 1, "ci = %d cid = %d si = %d sid = %d \r\n"
				"alla = %d, alld = %d soutsize = %d, soutnumber = %d\r\n"
				"qa = %d, qp = %d, qn = %d, qm = %d, qi = %d, qsi = %d\r\n"
				"SPL = %d, LPL = %d",
				cinsize, cinsized, sinsize, sinsized, 
				alla, alld, soutsize, soutnumber,
				qa, qp, qn, qm, qi, qsi,
				GetApp()->m_Server.GetPacketNumber(),
				GetApp()->m_LoginServer.GetPacketNumber());
		}
		break;

	case 4: // type
		/*
		{
		extern CNetPackageRecver *GetTest(WORD i);

		CNetPackageRecver *pnpr = NULL;

		LPCSTR Param = GetStringParameter(szCommand, 256, 1);
		if (Param == 0)
		{
		for (int i=0; i<MAX_CLIENT_NUMBER; i++)
		{
		pnpr = GetTest(i);
		rfalse(1, (int)(i%3 == 0), "i=%d imax=%d omax=%d itotal=%d ototal=%d", i, pnpr->in_max, pnpr->out_max, pnpr->in_total, pnpr->out_total);
		}
		rfalse(2, 0, "打印完毕！！！\r\n");
		}
		else
		{
		int i = atoi(Param);
		if (i < MAX_CLIENT_NUMBER)
		{
		pnpr = GetTest(i);
		rfalse(2, 1, "imax=%d omax=%d itotal=%d ototal=%d iBps=%d oBps=%d", 
		pnpr->in_max, pnpr->out_max, pnpr->in_total, pnpr->out_total, 
		pnpr->in_total*1000/(timeGetTime()-pnpr->segtick), pnpr->out_total*1000/(timeGetTime()-pnpr->segtick));
		}
		}
		}
		*/
		break;
		/*
		case 5: // disconnect from orbserver
		GetApp()->m_ORBServer.Disconnect();
		break;
		*/
	case 6: // fps
		rfalse(2, 1, "fps_max = %f, fps4ls = %f, fps4os = %f, fps4s = %f, fps4g = %f", 
			static_cast<float>(fps_max),
			static_cast<float>(fps4ls), 
			static_cast<float>(fps4os), 
			static_cast<float>(fps4s), 
			static_cast<float>(fps4g));

		fps_max =
			fps4ls = 
			fps4os = 
			fps4s = 
			fps4g = 0;

		break;

	case 7: // echo
		echo = !echo;
		//tracenew = !tracenew;
		break;

	case 8: // GMTalkMask
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[CONST_USERNAME]> sztime = GetStringParameter(szCommand, 256, 2);
			int itime = atoi(sztime);
			if (itime >= 1)
			{
				extern std::map<std::string, DWORD> GMTalkMask; // 禁言映射表
				extern LPIObject GetPlayerByName(LPCSTR);
				if (CPlayer *pPlayer = (CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER))
				{
					GMTalkMask[pPlayer->GetAccount()] = timeGetTime() + itime*60000;
					rfalse(2, 1, "设定[%s]名字叫%s的人禁言%d分钟", pPlayer->GetAccount(), (char *)szname, itime);
				}
				else
				{
					rfalse(2, 1, "找不到名字叫%s的人", (char *)szname);
				}
			}
		}
		break;

	case 88: // sreset
		{
			int param = 0;
			if ( LPCSTR szParam = GetStringParameter( szCommand, 256, 1 ) )
				param = atoi( szParam );
			g_Script.Restore( param );
			Onsreset();
		}
		break;

	case 89: // sview
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			extern void PrintScriptVariables(LPCSTR name);
			PrintScriptVariables(szname);
		}
		break;

	case 90: // mtsb
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> szregionid = GetStringParameter(szCommand, 256, 2);
			dwt::stringkey<char[10]> szx = GetStringParameter(szCommand, 256, 3);
			dwt::stringkey<char[10]> szy = GetStringParameter(szCommand, 256, 4);

			int regionid = atoi(szregionid);
			int x = atoi(szx);
			int y = atoi(szy);
			extern BOOL MoveSomebody(LPCSTR name, WORD regionid, WORD x, WORD y);
			if (!MoveSomebody(szname, (WORD)regionid, (WORD)x, (WORD)y))
				rfalse(2, 1, "当前服务器上找不到这个玩家");
			else
				rfalse(2, 1, "将[%s]移动到[%d][%d/%d]", (char *)szname, regionid, x, y);
		}
		break;

	case 91: // playerinfo
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);

			// 根据名称查找玩家
			CPlayer* destPlayer = (CPlayer*)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if (!destPlayer)
			{
				rfalse(2, 1, "当前服务器中没有此玩家");
				break;
			}

			// 找到了，显示名称
			rfalse(2, 1, "==========================玩家[%s]的信息==========================", (char*)szname);
			// 			rfalse(2, 1, "*游戏状态*");
			destPlayer->limitedState > 0 ? rfalse(2, 1,"玩家已经被纳入防沉迷系统中"): rfalse(2, 1,"玩家尚未纳入防沉迷系统中");
			if(destPlayer->limitedState ){
				rfalse(2, 1, "---离线时间：	[%d]分钟", destPlayer->offlineTime/60);
				rfalse(2, 1, "---在线时间：	[%d]分钟", destPlayer->m_OnlineTime/60);
			}
			rfalse(2, 1, "---玩家账号：	%s", destPlayer->GetAccount());
			rfalse(2, 1, "*基本属性*");
			rfalse(2, 1, "---门派：	%d", destPlayer->m_Property.m_School);
			rfalse(2, 1, "---当前等级：	%d", destPlayer->m_Level);
			rfalse(2, 1, "---最大生命:	%d", destPlayer->m_MaxHp);
			rfalse(2, 1, "---当前生命：	%d", destPlayer->m_CurHp);
			rfalse(2, 1, "---最大内力：	%d", destPlayer->m_MaxMp);
			rfalse(2, 1, "---当前内力：	%d", destPlayer->m_CurMp);
			rfalse(2, 1, "---最大体力：	%d", destPlayer->m_MaxTp);
			rfalse(2, 1, "---当前体力：	%d", destPlayer->m_CurTp);
			rfalse(2, 1, "---当前真气：	%d", destPlayer->m_Property.m_CurSp);

			rfalse(2, 1, "*战斗属性*");
			rfalse(2, 1, "---攻击：	%d", destPlayer->m_GongJi);
			rfalse(2, 1, "---防御：	%d", destPlayer->m_FangYu);
			rfalse(2, 1, "---暴击：	%d", destPlayer->m_BaoJi);
			rfalse(2, 1, "---闪避：	%d", destPlayer->m_ShanBi);
			rfalse(2, 1, "---伤害减免：	%d", destPlayer->m_ReduceDamage);
			rfalse(2, 1, "---绝对伤害:	%d", destPlayer->m_AbsDamage);
			rfalse(2, 1, "---无视防御：	%d", destPlayer->m_NonFangyu);
			rfalse(2, 1, "---暴击倍数：	%d", destPlayer->m_MultiBaoJi);
			rfalse(2, 1, "---冰伤害：	%d", destPlayer->m_IceDamage);
			rfalse(2, 1, "---冰抗性：	%d", destPlayer->m_IceDefence);
			rfalse(2, 1, "---火伤害：	%d", destPlayer->m_FireDamage);
			rfalse(2, 1, "---火抗性：	%d", destPlayer->m_FireDefence);
			rfalse(2, 1, "---玄伤害：	%d", destPlayer->m_XuanDamage);
			rfalse(2, 1, "---玄抗性：	%d", destPlayer->m_XuanDefence);
			rfalse(2, 1, "---毒伤害：	%d", destPlayer->m_PoisonDamage);
			rfalse(2, 1, "---毒抗性：	%d", destPlayer->m_PoisonDefence);

			rfalse(2, 1, "===================================================================================================\n");
		}
		break;
	case 92: // ison
	{
		dwt::stringkey<char[10]> indexstr = GetStringParameter(szCommand, 256, 1);
		int index = atoi(indexstr);
		// 根据名称查找玩家
		CPlayer* destPlayer = (CPlayer*)GetPlayerBySID(index)->DynamicCast(IID_PLAYER);
		if (!destPlayer)
		{
			rfalse(2, 1, "当前服务器中没有此玩家");
			break;
		}

		// 找到了，显示名称
		//rfalse(2, 1, "==========================玩家[%s]的信息==========================", (char*)szname);
		// 			rfalse(2, 1, "*游戏状态*");
		destPlayer->limitedState > 0 ? rfalse(2, 1, "玩家已经被纳入防沉迷系统中") : rfalse(2, 1, "玩家尚未纳入防沉迷系统中");
		if (destPlayer->limitedState){
			rfalse(2, 1, "---离线时间：	[%d]分钟", destPlayer->offlineTime / 60);
			rfalse(2, 1, "---在线时间：	[%d]分钟", destPlayer->m_OnlineTime / 60);
		}
		rfalse(2, 1, "---玩家账号：	%s", destPlayer->GetAccount());
		rfalse(2, 1, "*基本属性*");
		rfalse(2, 1, "---门派：	%d", destPlayer->m_Property.m_School);
		rfalse(2, 1, "---当前等级：	%d", destPlayer->m_Level);
		rfalse(2, 1, "---最大生命:	%d", destPlayer->m_MaxHp);
		rfalse(2, 1, "---当前生命：	%d", destPlayer->m_CurHp);
		rfalse(2, 1, "---最大内力：	%d", destPlayer->m_MaxMp);
		rfalse(2, 1, "---当前内力：	%d", destPlayer->m_CurMp);
		rfalse(2, 1, "---最大体力：	%d", destPlayer->m_MaxTp);
		rfalse(2, 1, "---当前体力：	%d", destPlayer->m_CurTp);
		rfalse(2, 1, "---当前真气：	%d", destPlayer->m_Property.m_CurSp);

		rfalse(2, 1, "*战斗属性*");
		rfalse(2, 1, "---攻击：	%d", destPlayer->m_GongJi);
		rfalse(2, 1, "---防御：	%d", destPlayer->m_FangYu);
		rfalse(2, 1, "---暴击：	%d", destPlayer->m_BaoJi);
		rfalse(2, 1, "---闪避：	%d", destPlayer->m_ShanBi);
		rfalse(2, 1, "---伤害减免：	%d", destPlayer->m_ReduceDamage);
		rfalse(2, 1, "---绝对伤害:	%d", destPlayer->m_AbsDamage);
		rfalse(2, 1, "---无视防御：	%d", destPlayer->m_NonFangyu);
		rfalse(2, 1, "---暴击倍数：	%d", destPlayer->m_MultiBaoJi);
		rfalse(2, 1, "---冰伤害：	%d", destPlayer->m_IceDamage);
		rfalse(2, 1, "---冰抗性：	%d", destPlayer->m_IceDefence);
		rfalse(2, 1, "---火伤害：	%d", destPlayer->m_FireDamage);
		rfalse(2, 1, "---火抗性：	%d", destPlayer->m_FireDefence);
		rfalse(2, 1, "---玄伤害：	%d", destPlayer->m_XuanDamage);
		rfalse(2, 1, "---玄抗性：	%d", destPlayer->m_XuanDefence);
		rfalse(2, 1, "---毒伤害：	%d", destPlayer->m_PoisonDamage);
		rfalse(2, 1, "---毒抗性：	%d", destPlayer->m_PoisonDefence);

		rfalse(2, 1, "===================================================================================================\n");
	}
	break;
	case 93: // ison
	{
		dwt::stringkey<char[10]> indexstr = GetStringParameter(szCommand, 256, 1);
		int index = atoi(indexstr);
		// 根据名称查找玩家
		CPlayer* destPlayer = (CPlayer*)GetPlayerByDnid(index)->DynamicCast(IID_PLAYER);
		if (!destPlayer)
		{
			rfalse(2, 1, "当前服务器中没有此玩家");
			break;
		}

		// 找到了，显示名称
		//rfalse(2, 1, "==========================玩家[%s]的信息==========================", (char*)szname);
		// 			rfalse(2, 1, "*游戏状态*");
		destPlayer->limitedState > 0 ? rfalse(2, 1, "玩家已经被纳入防沉迷系统中") : rfalse(2, 1, "玩家尚未纳入防沉迷系统中");
		if (destPlayer->limitedState){
			rfalse(2, 1, "---离线时间：	[%d]分钟", destPlayer->offlineTime / 60);
			rfalse(2, 1, "---在线时间：	[%d]分钟", destPlayer->m_OnlineTime / 60);
		}
		rfalse(2, 1, "---玩家账号：	%s", destPlayer->GetAccount());
		rfalse(2, 1, "*基本属性*");
		rfalse(2, 1, "---门派：	%d", destPlayer->m_Property.m_School);
		rfalse(2, 1, "---当前等级：	%d", destPlayer->m_Level);
		rfalse(2, 1, "---最大生命:	%d", destPlayer->m_MaxHp);
		rfalse(2, 1, "---当前生命：	%d", destPlayer->m_CurHp);
		rfalse(2, 1, "---最大内力：	%d", destPlayer->m_MaxMp);
		rfalse(2, 1, "---当前内力：	%d", destPlayer->m_CurMp);
		rfalse(2, 1, "---最大体力：	%d", destPlayer->m_MaxTp);
		rfalse(2, 1, "---当前体力：	%d", destPlayer->m_CurTp);
		rfalse(2, 1, "---当前真气：	%d", destPlayer->m_Property.m_CurSp);

		rfalse(2, 1, "*战斗属性*");
		rfalse(2, 1, "---攻击：	%d", destPlayer->m_GongJi);
		rfalse(2, 1, "---防御：	%d", destPlayer->m_FangYu);
		rfalse(2, 1, "---暴击：	%d", destPlayer->m_BaoJi);
		rfalse(2, 1, "---闪避：	%d", destPlayer->m_ShanBi);
		rfalse(2, 1, "---伤害减免：	%d", destPlayer->m_ReduceDamage);
		rfalse(2, 1, "---绝对伤害:	%d", destPlayer->m_AbsDamage);
		rfalse(2, 1, "---无视防御：	%d", destPlayer->m_NonFangyu);
		rfalse(2, 1, "---暴击倍数：	%d", destPlayer->m_MultiBaoJi);
		rfalse(2, 1, "---冰伤害：	%d", destPlayer->m_IceDamage);
		rfalse(2, 1, "---冰抗性：	%d", destPlayer->m_IceDefence);
		rfalse(2, 1, "---火伤害：	%d", destPlayer->m_FireDamage);
		rfalse(2, 1, "---火抗性：	%d", destPlayer->m_FireDefence);
		rfalse(2, 1, "---玄伤害：	%d", destPlayer->m_XuanDamage);
		rfalse(2, 1, "---玄抗性：	%d", destPlayer->m_XuanDefence);
		rfalse(2, 1, "---毒伤害：	%d", destPlayer->m_PoisonDamage);
		rfalse(2, 1, "---毒抗性：	%d", destPlayer->m_PoisonDefence);

		rfalse(2, 1, "===================================================================================================\n");
	}
	break;
	case 99: // test_param
		{
			void BroadCastAnnounce(LPCSTR info, int iSize);
			LPCSTR info = GetStringParameter(szCommand, 256, 1);
			BroadCastAnnounce(info, dwt::strlen(info, 60));
		}
		break;

	case 100:
		break;

	case 98:
		g_Count.PrintMoneyDataCount(2);
		break;

	case 101:
		{
			// 用于调试问题的开关
			g_check = !g_check;
		}
		break;

	case 102:
		{
			LoadEnvironment();
		}
		break;

	case 110: //setplayercount
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> flag = GetStringParameter(szCommand, 256, 2);
			dwt::stringkey<char[10]> ctime = GetStringParameter(szCommand, 256, 3);
			int nFlag = atoi(flag);
			int nCountTime = atoi(ctime);
			SetPlayerCountFlag(szname, nFlag);
			g_Count.SetUpdateTime(CCount::CCT_PLAYERDATA, nCountTime); 
		}

	case 111:
		g_newtype = !g_newtype;
		break;

	case 112:
		if (LPCSTR nt = GetStringParameter(szCommand, 256, 1))
		{
			dwNetRunLimit = atoi(nt);
			rfalse(2, 1, "Set auto_package Flag = %d", dwNetRunLimit);
		}
		break;

	case 113:
		g_bUsePackageMessage = !g_bUsePackageMessage;
		rfalse(2, 1, "%s auto pack broadcast message", g_bUsePackageMessage ? "Enable" : "Disable");
		break;

	case 114:
		{
			//LPCSTR filename = GetStringParameter(szCommand, 256, 1);
			//         if ( filename == NULL || filename[0] == 0 )
			//             filename = "data\\item\\item.def";

			//if(!LoadNewItemData(filename))
			//	rfalse(2, 1, "LoadNewItemData读取失败.");
			//else
			//	rfalse(2, 1, "LoadNewItemData已经成功加载.");
		}	
		break;

	case 115:
	case 116:
		break;

	case 117:
		{
			void SaveItemDropRcdToFile( BOOL saveName );
			SaveItemDropRcdToFile( true );

			DWORD clear = 0;
			void CleanItemDropRcd();
			if ( LPCSTR nt = GetStringParameter( szCommand, 256, 1 ) )
				if ( clear = atoi( nt ) )
					CleanItemDropRcd();

			/*if( g_pSpecialPlayerRcd )
			{
			//dwt::stringkey<char[20]> szname = ;
			LPCSTR  pName = GetStringParameter(szCommand, 256, 1);
			g_pSpecialPlayerRcd->AddRcdPlayer( (char*)pName );
			rfalse( 2, 1, "Add %s OK!", pName );
			}*/
		}
		break;
	case 118:
		int InitRcdMap();
		InitRcdMap();

		/*if( g_pSpecialPlayerRcd )
		{
		LPCSTR  pName = GetStringParameter(szCommand, 256, 1);
		if( g_pSpecialPlayerRcd->DelRcdPlayer( (char*)pName ) )
		rfalse( 2, 1, "Delete %s OK!", pName );
		else
		rfalse( 2, 1, "Con not delete %s !", pName );
		}*/
		break;
	case 119:
		extern void BackupPlayerIntoCache();
		BackupPlayerIntoCache();
		break;
	case 120:
		g_Script.DoBuffer( szCommand + sizeof( "lua_exec" ), strlen( szCommand ) - sizeof( "lua_exec" ) );
		break;
	case 121:
		CBuilding::LoadBuildingData();
		break;
	case 122:
		{
			void CheckDynamicRegion();
			CheckDynamicRegion();
		}
		break;
	case 123:
		{
			TreasureShop::LoadTreasureShopData("data\\item\\GoodsItem.def");
		}
		break;

	case 124:
		{
			LPCSTR str =  GetStringParameter( szCommand, 256, 1 );
			if ( str == NULL )
				break;

			__int64 sid = _atoi64( str );
			GetGW()->m_UnionManager.ShowPlayerTitle( ( DWORD )sid );
		}
		break;
	case 125:
		{
			LPCSTR str =  GetStringParameter( szCommand, 256, 1 );
			if ( str == NULL )
				break;
			GetGW()->m_UnionManager.ShowUnionInfo( str );
		}
		break;
	case 126:
		{
			GetGW()->CPlayerManager::KickHangupPlayerAll();
		}
		break;

	case 130:
		{
			LPCSTR str = GetStringParameter( szCommand, 256, 1 );
			if ( str == NULL )
				break;

			if ( str[0] == '0' && str[1] == 'x' )
			{
				extern QWORD HexStringToNumber( LPCSTR str );
				DNID mac = HexStringToNumber( str );
				int num = GetApp()->m_Server.GetLinkByMac( tmp, 3000, mac );
				for ( int i = 0; i < num; i ++ )
				{
					CPlayer *player = ( CPlayer* )GetPlayerByDnid( tmp[i] )->DynamicCast( IID_PLAYER );
					rfalse( 2, 1, "0x%I64x - %s", tmp[i], player ? player->GetName() : "(NULL)" );
				}
			}
			else
			{
				if ( CPlayer *player = ( CPlayer* )GetPlayerByName( str )->DynamicCast( IID_PLAYER ) )
				{
					DNID mac = GetApp()->m_Server.GetLinkMac( player->m_ClientIndex );
					rfalse( 2, 1, "%s - 0x%I64x", str, mac );
				}
				else
				{
					rfalse( 2, 1, "未找到指定角色！" );
				}
			}
		}
		break;

	case 131:
		{
			int num = GetApp()->m_Server.GetLinkByFlags( tmp, 3000, 1 );
			LPCSTR str =  GetStringParameter( szCommand, 256, 1 );
			rfalse( 2, 1, "目前使用外挂的人数有%d", num );
			if ( str != NULL )
			{
				for ( int i = 0; i < num; i ++ )
				{
					CPlayer *player = ( CPlayer* )GetPlayerByDnid( tmp[i] )->DynamicCast( IID_PLAYER );
					rfalse( 2, 1, "0x%I64x - %s", tmp[i], player ? player->GetName() : "(NULL)" );
				}
			}
		}
		break;

		// Reload Special Item List
	case 132:
		{
		}
		break;

		// Reload Special Item Attribute List
	case 133:
		{
		}
		break;
	case 134:
		{
			extern UGE::mtQueue< SGMCheck > gmCheckQueue;

			SGMCheck msg;
			msg.wGameType = -1;
			msg.msg.wGMCmd = SQGameMngMsg::GM_LOADGMCHECKURL;
			gmCheckQueue.mtsafe_push( msg );

			rfalse(2, 1, "已经启动加载GMCheckURL!");
		}
		break;
	case 135:
		{
			// 读取鉴定等级属性列表文件
			extern int LoadLevelAttributeArray( void );
			LoadLevelAttributeArray();
			rfalse(2, 1, "LevelAttributeArray加载成功!");
		}
		break;
	case 136:
		{
			GetApp()->m_Server.SetMacLimit( 0 );
			rfalse(2, 1, "开启游戏多开！");
		}
		break;

	case 399:	// kickoneplayer   kick一个玩家
		{

		}
		break;

	case 400:
		{
			CSkillService::GetInstance().ReLoad();
			rfalse(2, 1, "技能列表重新加载完毕！");
		}
		break;

	case 401:
		{
			CItemService::GetInstance().ReLoad();
			rfalse(2, 1, "道具表重新加载完毕！");
		}
		break;

	case 402:
		{
			CItemDropService::GetInstance().Reload();
			rfalse(2, 1, "物品掉落表重新加载完毕！");
		}
		break;

	case 403:
		{
			CMonsterService::GetInstance().ReLoad();
			rfalse(2, 1, "怪物表重新加载完毕！");
		}
		break;

	case 404:
		{
			CBuffService::GetInstance().ReLoad();
			rfalse(2, 1, "Buff表重新加载完毕！");
		}
		break;
	case 405:
		{
			GetGW()->CPlayerManager::KickPlayerAll();
		}
		break;
	case 406: // mtdr
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> szregionid = GetStringParameter(szCommand, 256, 2);
			dwt::stringkey<char[10]> szx = GetStringParameter(szCommand, 256, 3);
			dwt::stringkey<char[10]> szy = GetStringParameter(szCommand, 256, 4);

			CPlayer* player=(CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2, 1, "当前服务器上找不到这个玩家");
				break;
			}
			int regionid = atoi(szregionid);
			int x = atoi(szx);
			int y = atoi(szy);

			g_Script.CallFunc("PutPlayerToDynamicRegion",regionid,x,y,player->GetGID());
			//rfalse(2, 1, "当前服务器上找不到这个玩家");
		}
		break;
	case 407://PK模式，暂时用命令调试 setpk
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> pkrule = GetStringParameter(szCommand, 256, 2);
			CPlayer* player=(CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2, 1, "当前服务器上找不到这个玩家");
				break;
			}
			int pkrulemode = atoi(pkrule);
			if (pkrulemode>=0&&pkrulemode<5)
			{
				player->m_nPkRule=pkrulemode;
			}

		}
		break;
	case 408://talktoall
		{
			dwt::stringkey<char[MAX_ACCAPASS]> str = GetStringParameter(szCommand, MAX_ACCAPASS, 1);
			//dwt::stringkey<char[10]> chattype = GetStringParameter(szCommand, 256, 2);
			//BYTE btype=(BYTE)atoi(chattype);
			TalkToAllObj(str);
			break;
		}
		break;
	case 409:
		{
			size_t num=GetPlayerNumber();
			char playernum[100];
			sprintf(playernum,"当前服务器人数%d人",num);
			AddInfo(playernum);
			break;
		}
		break;
	case 410://createmonster(玩家姓名,id,数量)
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> monterid = GetStringParameter(szCommand, 256, 2);
			dwt::stringkey<char[10]> monternum = GetStringParameter(szCommand, 256, 3);

			CPlayer* player=(CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2, 1, "当前服务器上找不到这个玩家");
				break;
			}
			int reginid=player->GetPlayerReginID();
			CRegion *regin=(CRegion*)FindRegionByID(reginid)->DynamicCast(IID_REGION);
			if (regin)
			{
				CMonster::SParameter param;
				param.dwRefreshTime=-1;
				param.wListID=atoi(monterid);
				param.wX=player->m_curTileX;
				param.wY=player->m_curTileY;
				int montern=atoi(monternum);
				while(montern--)
				{
					//param.wX--;
					//param.wY--;
					regin->CreateMonster(&param);
				}
			}
		}
		break;
	case 	411: //加钱(姓名,数量)showmethemoney
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> moneynum = GetStringParameter(szCommand, 256, 2);

			CPlayer* player=(CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2, 1, "当前服务器上找不到这个玩家");
				break;
			}
			int num=atoi(moneynum);
			player->AddPlayerMoney(1,num);
			player->AddPlayerMoney(2,num);
			player->AddPlayerMoney(3,num);
		}
		break;
	case 	412: //加经验(姓名,数量)addexp
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> moneynum = GetStringParameter(szCommand, 256, 2);

			CPlayer* player=(CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2, 1, "当前服务器上找不到这个玩家");
				break;
			}
			int num=atoi(moneynum);
			player->SendAddPlayerExp(num, SAExpChangeMsg::GM, "GMGive");
		}
		break;
	case 	413: //加物品(姓名,ID,数量)additem
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> itemidstr = GetStringParameter(szCommand, 256, 2);
			dwt::stringkey<char[10]> itemnumstr = GetStringParameter(szCommand, 256, 3);

			CPlayer* player=(CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2, 1, "当前服务器上找不到这个玩家");
				break;
			}
			int itemid=atoi(itemidstr);
			int itemnum=atoi(itemnumstr);

			std::list<SAddItemInfo> itemList;
			itemList.push_back(SAddItemInfo(itemid, itemnum));
			if (player->CanAddItems(itemList)){
				player->StartAddItems(itemList);
			}
		}
		break;
	case 	414: //加点(类型,ID,数量)addpoint
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> typestr = GetStringParameter(szCommand, 256, 2);
			dwt::stringkey<char[10]> valuestr = GetStringParameter(szCommand, 256, 3);

			CPlayer* player=(CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2, 1, "当前服务器上找不到这个玩家");
				break;
			}
			int pointtype=atoi(typestr) + 22;	// 可用点数是从枚举类型23开始到枚举类型27，一共是四种
			int pointvalue=atoi(valuestr);

			player->AddPlayerPoint(pointtype,pointvalue);
			//player->StartAddItems(itemList);
		}

		break;
	case 415://打开地图碰撞
		{
			dwt::stringkey<char[CONST_USERNAME]> reginidstr = GetStringParameter(szCommand, 256, 1);
			int reginid=atoi(reginidstr);
			CRegion *region=(CRegion*)FindRegionByID(reginid)->DynamicCast(IID_REGION);
			if (region)
			{
				region->SetUseMapCollision(true);
			}
		}

		break;
	case 416://关闭地图碰撞
		{
			dwt::stringkey<char[CONST_USERNAME]> reginidstr = GetStringParameter(szCommand, 256, 1);
			int reginid=atoi(reginidstr);
			CRegion *region=(CRegion*)FindRegionByID(reginid)->DynamicCast(IID_REGION);
			if (region)
			{
				region->SetUseMapCollision(false);
			}
		}

		break;

	case 417://对好友使用道具
		{
			dwt::stringkey<char[CONST_USERNAME]> myname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[CONST_USERNAME]> destname = GetStringParameter(szCommand, 256, 2);
			dwt::stringkey<char[10]> itemCellPos = GetStringParameter(szCommand, 256, 3);
			dwt::stringkey<char[10]> itemnumstr= GetStringParameter(szCommand, 256, 4);
			int nitemCellPos=atoi(itemCellPos);				
			int nitemnum=atoi(itemnumstr);
			SQUseFriendItemMsg uFriendItemMsg;
			dwt::strcpy(uFriendItemMsg.cName,destname,CONST_USERNAME);
			uFriendItemMsg.wCellPos = (WORD)nitemCellPos;				
			uFriendItemMsg.wItemNum = nitemnum;
			CPlayer *player =(CPlayer*) GetPlayerByName(myname)->DynamicCast(IID_PLAYER);

		}
		break; 
	case 418://测试使用侠客装备equippet
		{
			dwt::stringkey<char[CONST_USERNAME]> myname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> typestr= GetStringParameter(szCommand, 256, 2);
			dwt::stringkey<char[10]> itemCellPos= GetStringParameter(szCommand, 256, 3);
			dwt::stringkey<char[10]> indexstr= GetStringParameter(szCommand, 256, 4);				
			int nitemCellPos = atoi(itemCellPos);
			int index =atoi(indexstr);
			int type = atoi(typestr);
			SQSpecialUseItemMsg suitemmsg;
			suitemmsg.btype =type;
			suitemmsg.wCellPos = (WORD)nitemCellPos;				
			suitemmsg.windex = index;
			CPlayer *player =(CPlayer*) GetPlayerByName(myname)->DynamicCast(IID_PLAYER);
			if (player)
			{
				player->RecvSpecialUseItem(&suitemmsg);
			}
		}
		break;
	case 419://测试使用侠客装备unequippet
		{
			dwt::stringkey<char[CONST_USERNAME]> myname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> posstr= GetStringParameter(szCommand, 256, 2);
			dwt::stringkey<char[10]> indexstr= GetStringParameter(szCommand, 256, 3);
			int index =atoi(indexstr);
			int pos = atoi(posstr);
			CPlayer *player =(CPlayer*) GetPlayerByName(myname)->DynamicCast(IID_PLAYER);
			if (player)
			{
				player->_fpToPackage(pos,index);
			}
		}
		break;
	case 420://测试从脚本中返回技能书对应的技能IDgetitemskill
		{
			dwt::stringkey<char[10]> itemidstr= GetStringParameter(szCommand, 256, 1);
			int itemid = atoi(itemidstr);

			// 独立脚本扩展
			lite::Variant ret;
			g_Script.SetCondition(NULL, NULL, NULL);
			LuaFunctor(g_Script, "GetItemTofpSkillID")[itemid](&ret);
			g_Script.CleanCondition();

			if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)
				break;
			int skillid = ( __int32 )ret;
		}
		break;
	case 421://测试侠客转换道具fptoitem
		{
			dwt::stringkey<char[CONST_USERNAME]> myname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> indexstr= GetStringParameter(szCommand, 256, 2);
			int index =atoi(indexstr);
			CPlayer *player =(CPlayer*) GetPlayerByName(myname)->DynamicCast(IID_PLAYER);
			if (player)
			{
				player->_fpMakeItem(index);
			}
		}
		break;
	case 422://测试召唤侠客calloutfp
		{
			dwt::stringkey<char[CONST_USERNAME]> myname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> indexstr= GetStringParameter(szCommand, 256, 2);
			int index =atoi(indexstr);
			CPlayer *player =(CPlayer*) GetPlayerByName(myname)->DynamicCast(IID_PLAYER);
			if (player)
			{
				player->CallOutFightPet(index);
			}
		}
		break;
	case 423://测试删除怪物["killmonster"] = 423;
		{
			dwt::stringkey<char[CONST_USERNAME]> myname = GetStringParameter(szCommand, 256, 1);
			CPlayer *player =(CPlayer*) GetPlayerByName(myname)->DynamicCast(IID_PLAYER);
			if (player&&player->m_ParentRegion)
			{
				player->m_ParentRegion->KillMonster(player);
				return;
			}

			dwt::stringkey<char[10]> indexstr= GetStringParameter(szCommand, 256, 1);
			int index =atoi(indexstr);
			CRegion *regin =(CRegion*)FindRegionByID(index)->DynamicCast(IID_REGION);
			if(regin)
			{
				regin->KillMonster(0);		
			}
		}
		break;
	case 424://设置任务状态settaskInfo
		{
			dwt::stringkey<char[CONST_USERNAME]> myname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> indexstr= GetStringParameter(szCommand, 256, 2);
			dwt::stringkey<char[10]> statusstr= GetStringParameter(szCommand, 256, 3);
			CPlayer *player =(CPlayer*) GetPlayerByName(myname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2,1,FormatString("找不到名字叫%s的玩家",myname));
				return;
			}
			WORD taskID = atoi(indexstr);
			WORD taskStatus = atoi(statusstr);
			player->m_TaskInfo.push_back(make_pair(taskID, taskStatus));
		}
		break;
	case 425://createmonsterregin ;//地图刷怪 mapid x y monsterid num
		{
			dwt::stringkey<char[10]> reginstr = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> reginx = GetStringParameter(szCommand, 256, 2);
			dwt::stringkey<char[10]> reginy = GetStringParameter(szCommand, 256, 3);
			dwt::stringkey<char[10]> monterid = GetStringParameter(szCommand, 256, 4);
			dwt::stringkey<char[10]> monternum = GetStringParameter(szCommand, 256, 5);

			//int reginid=atoi(reginstr);
			CPlayer *pPlayer = (CPlayer*)GetPlayerByName(reginstr)->DynamicCast(IID_PLAYER);
			if (pPlayer&&pPlayer->m_ParentRegion)
			{
				CMonster::SParameter param;
				param.dwRefreshTime = -1;
				param.wListID = atoi(monterid);
				param.wX = atoi(reginx);
				param.wY = atoi(reginy);
				int montern = atoi(monternum);
				while (montern--){
					pPlayer->m_ParentRegion->CreateMonster(&param);
				}
			}
// 			CRegion *regin=(CRegion*)FindRegionByID(reginid)->DynamicCast(IID_REGION);
// 			if (regin)
// 			{
// 				CMonster::SParameter param;
// 				param.dwRefreshTime=-1;
// 				param.wListID=atoi(monterid);
// 				param.wX=atoi(reginx);
// 				param.wY=atoi(reginy);
// 				int montern=atoi(monternum);
// 				while(montern--){
// 					regin->CreateMonster(&param);
// 				}
// 			}
		}
		break;
	case 426: //复活
		{
			dwt::stringkey<char[CONST_USERNAME]> myname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> indexstr= GetStringParameter(szCommand, 256, 2);

			CPlayer *player =(CPlayer*) GetPlayerByName(myname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2,1,"找不到玩家姓名!");
				return;
			}
			WORD liveID = atoi(indexstr);
			player->QuestToRelive(liveID);
		}
		break;
	case 427:// addsp 加真气 （玩家姓名、真气值）
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> spnum = GetStringParameter(szCommand, 256, 2);

			CPlayer* player=(CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2, 1, "当前服务器上找不到这个玩家");
				break;
			}
			int num=atoi(spnum);
			player->AddPlayerSp(num);
		}
		break;
	case 428:  //["setlevel"]				= 427;	// 增加玩家等级
		{
			dwt::stringkey<char[CONST_USERNAME]> myname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> levelstr= GetStringParameter(szCommand, 256, 2);
			CPlayer *player =(CPlayer*) GetPlayerByName(myname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2,1,"找不到玩家姓名!");
				return;
			}
			WORD level = atoi(levelstr);
			player->OnSetPlayerLevel(level);
		}
		break;
	case 429:
		{
			dwt::stringkey<char[100]> sfuncname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[100]> sparam = GetStringParameter(szCommand, 256, 2);
			if (sfuncname)
			{
				g_Script.PrepareFunction(sfuncname);
				int i = 3;
				while(1)
				{
					if (sparam && dwt::strlen(sparam,100) > 0)
					{
						if (!IsParameterString(sparam, strlen(sparam)))
						{
							int dparam = atoi(sparam);
							g_Script.PushParameter(lite::Variant(dparam));
						}
						else
						{
							g_Script.PushParameter(lite::Variant(sparam));
						}		
						sparam = GetStringParameter(szCommand, 256, i);
						i++;
					}
					else
					{
						break;
					}
				}
				g_Script.Execute();
			}
		}
		break;
	case 430:	//设置玩家GM等级
	{
					dwt::stringkey<char[CONST_USERNAME]> myname = GetStringParameter(szCommand, 256, 1);
					dwt::stringkey<char[10]> GMLevelStr = GetStringParameter(szCommand, 256, 2);
					CPlayer *player = (CPlayer*)GetPlayerByName(myname)->DynamicCast(IID_PLAYER);
					if (!player)
					{
						rfalse(2, 1, "找不到玩家姓名!");
						return;
					}
					WORD GMLevel = atoi(GMLevelStr);
					player->m_Property.m_GMLevel = GMLevel;
	}
		break;
	case 431:	//发送系统公告
	{
					dwt::stringkey<char[CONST_USERNAME]> sztimes = GetStringParameter(szCommand, 256, 1);
					dwt::stringkey<char[MAX_CHAT_LEN]> szcontent = GetStringParameter(szCommand, 256, 2);
					int itimes = atoi(sztimes);
					SChatToAllMsg ChatToAllMsg;
					ChatToAllMsg.wSoundId = 0;
					ChatToAllMsg.wChatLen = strlen(szcontent);
					ChatToAllMsg.wShowNum = itimes;
					strcpy(ChatToAllMsg.cChat, szcontent);
					BroadcastMsg(&ChatToAllMsg, sizeof(SChatToAllMsg));
	}
		break;
	case 432: //打印方便输出查看
	{
		dwt::stringkey<char[100]> mystr = GetStringParameter(szCommand, 256, 1);
		//int nstr = atoi(mystr);
		strcat(mystr, "\n");
		AddInfo(mystr);
	}
		break;


	////**********************************************////
	////****************萌斗三国系统********************////
	////**********************************************////


	case 500: //系统公告
	{
		
		dwt::stringkey<char[MAX_CHAT_LEN]> str = GetStringParameter(szCommand, 256, 1);
		
		CDChatManager *_CDChatManager = new CDChatManager;
		SAChatGlobalMsg *_msg = new SAChatGlobalMsg;
		_msg->_protocol = SChatBaseMsg::EPRO_CHAT_GLOBAL;
		_msg->byType = SAChatGlobalMsg::ECT_SYSTEM;
		dwt::strcpy(_msg->cChatData, str, MAX_CHAT_LEN);
		CPlayer *_cplayer=new CPlayer;

		_CDChatManager->ProcessChatMsg(0, (SChatBaseMsg *)_msg, _cplayer);
		delete _msg;
		delete _CDChatManager;
		delete _cplayer;
		AddInfo("系统公告发送成功\n");
	}
		break;
	case 501: //模拟玩家点击取邮件信息
	{
		dwt::stringkey<char[10]> indexstr = GetStringParameter(szCommand, 256, 1);
		int index = atoi(indexstr);
		AddInfo("模拟玩家点击取邮件信息");
		SMailMsg *_msg=new SMailMsg;
		_msg->_protocol = SMailMsg::GetMailInfo;
		MailMoudle::getSingleton()->DispatchMailMsg(5555, _msg, index);
		delete _msg;
	}
		break;
	case 502: 
	{
		dwt::stringkey<char[10]> indexstr = GetStringParameter(szCommand, 256, 1);
		int index = atoi(indexstr);

		AddInfo("发邮件信息");
		MailInfo*_MailInfo = new MailInfo;
		memset(_MailInfo, 0, sizeof(MailInfo));
		_MailInfo->icon = 1;
		_MailInfo->szItemInfo[0].itype = 3;
		_MailInfo->szItemInfo[0].id = 13012;
		_MailInfo->szItemInfo[0].num = 2;
		_MailInfo->szItemInfo[1].itype = 1;
		_MailInfo->szItemInfo[1].id = 14020;
		_MailInfo->szItemInfo[1].num = 10;
		_MailInfo->szItemInfo[2].itype = 2;
		_MailInfo->szItemInfo[2].id = 11000;
		_MailInfo->szItemInfo[2].num = 2;
		strcpy_s(_MailInfo->content, "这是内容~~~~骚年,你发了!,领奖吧!");
		strcpy_s(_MailInfo->senderName, "系统 GM哥");
		strcpy_s(_MailInfo->title, "这是标题_单人土豪大礼包!");
		MailMoudle::getSingleton()->SendMail_SanGuo(index, 3*24*60*60, _MailInfo);
		delete _MailInfo;
	}
		break;
	case 503://取奖励
	{

		AddInfo("取奖励");
		dwt::stringkey<char[10]> indexstr = GetStringParameter(szCommand, 256, 1);
		int index = atoi(indexstr);
		SQ_GetAwards_MSG *msg = new SQ_GetAwards_MSG;
		msg->m_dwMailId = index;
		MailMoudle::getSingleton()->GetMailAwards(5,msg,5);
		delete msg;
	}
		break;
	case 504://删除
	{

		AddInfo("删除");
		dwt::stringkey<char[10]> indexstr = GetStringParameter(szCommand, 256, 1);
		int index = atoi(indexstr);
		SQ_SetMailState_MSG *msg = new SQ_SetMailState_MSG;
		msg->m_dwMailId = index;
		MailMoudle::getSingleton()->SetMailState(msg,5);
		delete msg;
	}
		break;
	case 505://发全服邮件信息
	{
		dwt::stringkey<char[10]> indexstr = GetStringParameter(szCommand, 256, 1);
		int index = atoi(indexstr);

		AddInfo("发全服邮件信息");
		MailInfo*_MailInfo = new MailInfo;
		memset(_MailInfo, 0, sizeof(MailInfo));
		_MailInfo->icon = 1;
		_MailInfo->szItemInfo[0].itype = 2;
		_MailInfo->szItemInfo[0].id = 0;
		_MailInfo->szItemInfo[0].num = 20000;
		_MailInfo->szItemInfo[1].itype = 7;
		_MailInfo->szItemInfo[1].id = 0;
		_MailInfo->szItemInfo[1].num = 100;
		_MailInfo->szItemInfo[2].itype = 6;
		_MailInfo->szItemInfo[2].id = 0;
		_MailInfo->szItemInfo[2].num = 200;
		strcpy_s(_MailInfo->content, "      骚年,我看你骨骼惊奇,何不来一起维护世界和平,共享人世繁华!");
		strcpy_s(_MailInfo->senderName, "宇宙和谐统战部思密达");
		strcpy_s(_MailInfo->title, "来自金将军的问候");
		/*for (int i = 0; i < 10000;i++)
		{
		MailMoudle::getSingleton()->SendMail_SYS_SanGuo(10 * 24 * 60 * 60, _MailInfo);
		}*/
		MailMoudle::getSingleton()->SendMail_SYS_SanGuo(10 * 24 * 60 * 60, _MailInfo);
		delete _MailInfo;


		
	}
	break;
	case 506://测试
	{
		struct MyStruct
		{
			int i;
		};
		MyStruct * _MyStruct = new MyStruct;
		_MyStruct = NULL;
		
		_MyStruct->i = 1;
		
	}
	break;

	case 507:
	{
		//帮会功能检测
		CPlayer *pDestPlayer = (CPlayer *)GetPlayerByName("gmphy5name")->DynamicCast(IID_PLAYER);
		//GetGW()->m_FactionManager.CreateFaction("ss", pDestPlayer, 10001);
	}
	break;

	case 508:
	{
		//帮会功能检测
		CPlayer *pDestPlayer = (CPlayer *)GetPlayerByName("gmphy20name")->DynamicCast(IID_PLAYER);
		//GetGW()->m_FactionManager.AddMember("ss", pDestPlayer);
	}
	break;

	case 509:
	{
		//帮会功能检测
		//GetGW()->m_FactionManager.Run();
	}
	break;

	case 510: //读取工会
	{
		// 向DB Server请求读取帮派数据
		SQGetFactionMsg msg;
		extern DWORD GetGroupID();
		msg.nServerId = GetGroupID();
		SendToLoginServer(&msg, sizeof(msg));
	}
	break;
	default:
		break;
	}
}


void AppQuit()
{
	GetApp()->m_bQuit = true;
}

void Onsreset()
{
	GetGW()->LoadActivityNotice();
// 	bool Flag = GetGW()->LoadBlessOpenClose();
// 	if (Flag)
// 	{
// 		GetGW()->SendBlessOpenClose();
// 		//广播消息
// 	}
}
