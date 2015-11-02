// ��½������.cpp : ����Ӧ�ó������ڵ㡣
//

//#include "stdafx.h"
//#include "��½������.h"

// ��Ϸ��½������.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "GMServer.h"
#include <map>
#include <time.h>
#include "���������.h"
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
void Onsreset(); //�ڽű�ˢ������Ժ����
extern int SendToLoginServer(SMessage *, long size);
//extern const __int32 IID_REGION; 
// ��������������󲿷�����µĻ�����!
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

// ȫ�ֱ�����
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

HWND hMainWnd;
HWND hInfo;
HWND hInput;

WNDPROC OldProc;
char InputBuffer[256] = {0};
LRESULT CALLBACK ReloadEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void ExplainCommand(LPCSTR szCommand);

// �˴���ģ���а����ĺ�����ǰ��������
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
	//���ڱ�Ŀ¼������һ��Exception.dmpʹ��windbg����ֱ����vs �򿪣����õ��Ծ��ܶ�λ����
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

		// ��ʼ��ȫ���ַ���
		LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadString(hInstance, IDC_MY, szWindowClass, MAX_LOADSTRING);
		strcpy(szWindowClass, "xyd_zoneserver");
		MyRegisterClass(hInstance);

		// ִ��Ӧ�ó����ʼ����
		if (!InitInstance(hInstance, nCmdShow))
		{
			return FALSE;
		}

		hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_MY);

		CreateApp();
		if (!GetApp()->InitServer())
		{
			ClearApp();
			return rfalse(0, 0, "��ʼ��ʧ��");
		}

		g_StartUpZoneServerTime = time(NULL);

		//add by ly 2014/10/10 GM�������
		//int InitGMFlag = InitGMServer();
		CreateThread(0, 0, g_LoopRechargeGet, 0, 0, 0);//��ʼ��ֵ�߳�

		// ����Ϣѭ����
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

		CRecharge::getSingleton()->ExitRechargeTHD();//�˳���ֵ�߳�
		GetApp()->ExitServer();
		ClearApp();
		Sleep(1000);
	}
	
	return (int) msg.wParam;
}



//
//  ������MyRegisterClass()
//
//  Ŀ�ģ�ע�ᴰ���ࡣ
//
//  ע�ͣ�
//
//    ����ϣ��������ӵ� Windows 95 ��
//    ��RegisterClassEx������֮ǰ�˴����� Win32 ϵͳ����ʱ��
//    ����Ҫ�˺��������÷������ô˺���
//    ʮ����Ҫ������Ӧ�ó���Ϳ��Ի�ù�����
//   ����ʽ��ȷ�ġ�Сͼ�ꡣ
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
//   ������InitInstance(HANDLE, int)
//
//   Ŀ�ģ�����ʵ�����������������
//
//   ע�ͣ�
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{

	hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

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

	SetWindowFont(hInfo, CreateFont(12,6,0,0,400,0,0,0,0,0,0,0,1,"������"), 1);

	hInput = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", WS_CHILDWINDOW | WS_VISIBLE,
		__input_x, rect.bottom-__input_h, rect.right-__input_x, __input_h, hMainWnd, (HMENU)IDC_INPUT, hInstance, NULL);

	if (!hInput)
	{
		return FALSE;
	}

	SetWindowFont(hInput, CreateFont(12,6,0,0,400,0,0,0,0,0,0,0,1,"������"), 1);

	OldProc = (WNDPROC)(_W64 LONG)SetWindowLong(hInput, GWL_WNDPROC, (LONG)(_W64 LONG)ReloadEditWndProc);

	SetWindowFont(CreateWindowEx(WS_EX_CLIENTEDGE, "Static", "CMD����", WS_CHILDWINDOW | WS_VISIBLE | SS_CENTER,
		0, rect.bottom-__input_h, __input_x, __input_h, hMainWnd, (HMENU)IDC_INPUT, hInstance, NULL), CreateFont(12,6,0,0,400,0,0,0,0,0,0,0,1,"������"), 1);

	ShowWindow(hMainWnd, nCmdShow);
	UpdateWindow(hMainWnd);

	return TRUE;
}

//
//  ������WndProc(HWND, unsigned, WORD, LONG)
//
//  Ŀ�ģ����������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
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
			SendMessage(hInfo, EM_REPLACESEL, 0, (LPARAM)"CMD����");
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
	command_map["loadGMCheckURL"]			= 134;  // ���ļ�����GM��֤URL

	command_map["loadLevelAttributes"]		= 135;
	command_map["unlimit"]					= 136;  // �࿪

	// �����3����ϵͳ
	command_map["kickoneplayer"]		= 399;		// kickһ�����
	command_map["reloadskill"]			= 400;
	command_map["reloaditem"]			= 401;
	command_map["reloaddropitem"]		= 402;
	command_map["reloadmonster"]		= 403;
	command_map["reloadbuff"]			= 404;
	command_map["kickall"]				= 405;
	command_map["mtdr"]					= 406;
	command_map["setpk"]				= 407;
	command_map["talktoall"]			= 408;
	command_map["����"]					= 409;
	command_map["createmonster"]		= 410;
	command_map["addmoney"]				= 411;	// ��Ǯ
	command_map["addexp"]				= 412;	// �Ӿ���
	command_map["additem"]				= 413;	// ����Ʒ
	command_map["addpoint"]				= 414;	// �ӵ�
	command_map["opencol"]				= 415;	// ʹ�õ�ͼ��ײ
	command_map["closecol"]				= 416;	// �رյ�ͼ��ײ
	command_map["useitemtofriend"]		= 417;  // �Ժ���ʹ����Ʒ
	command_map["equippet"]				= 418;	// ����ʹ������װ��equippet
	command_map["unequippet"]			= 419;	// ����ʹ������װ��unequippet
	command_map["getitemskill"]			= 420;	// ���Դӽű��з��ؼ������Ӧ�ļ���ID
	command_map["fptoitem"]				= 421;	// ��������ת������
	command_map["calloutfp"]			= 422;	// �����ٻ�����
	command_map["killmonster"]			= 423;	// ����ɾ������
	command_map["settaskInfo"]			= 424;	// ��������״̬
	command_map["createmonsterregin"]	= 425;	// ��ͼˢ��
	command_map["relive"]				= 426;	// ����
	command_map["addsp"]				= 427;	// ������
	command_map["setlevel"]				= 428;	// ������ҵȼ�
	command_map["luacall"]				= 429;	// ����lua����
	command_map["setplayergmlevel"] = 430;	//�������GM�ȼ�
	command_map["syscall"] = 431;	//����ϵͳ����
	command_map["print"] = 432;	//��ӡ���

	////**********************************************////
	////****************�ȶ�����ϵͳ********************////
	////**********************************************////
	command_map["gm"] = 500;//ϵͳ����
	command_map["getmail"] = 501;//ȡ�ʼ���Ϣ arg1=���id
	command_map["sendmail"] = 502;//���ʼ� arg1=���id
	command_map["getg"] = 503;//ȡ���� arg1=�ʼ�id
	command_map["del"] = 504;//ɾ���ʼ� arg1=�ʼ�id
	command_map["smsys"] = 505;//���ʼ�,ϵͳ��ȫ�� 
	command_map["wk"] = 506;//����

	command_map["testjt"] = 507;//��Ṧ�ܲ���
	command_map["testaddjt"] = 508;//�����Ṧ�ܲ���
	command_map["testsave"] = 509;//���湫�ᵽ���ݿ�

	command_map["loadfaction"] = 510;//���湫�ᵽ���ݿ�
	return 1;
}

int o = InitCommandMap();

double fps_max = 0, cur = 0;
double fps4ls = 0, curls = 0;
double fps4os = 0, curos = 0;
double fps4s = 0, curs = 0;
double fps4g = 0, curg = 0;
//TODO:Tony Modify [2012-3-6]Comment:[�������ķ���]
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

	// תСд
	_strlwr(const_cast<char*>(Param1));

	// ����������ʹ��[]��������ֹӳ���Ķ�̬����
	it = command_map.find(Param1);
	if (it == command_map.end())
	{
		AddInfo("��Ч������\r\n");
		return;
	}

	//	switch((int)command_map[Param1])
	switch(it->second)
	{
	case 1: // help
		AddInfo(/*
			"========================================Help=======================================================\r\n"
			"\t ��������		����				����\r\n"
			"===================================================================================================\r\n"
			"\r\n"
			"\t help			-				�鿴���������Ϣ\r\n"
			"\t quit			-				�ر����������\r\n"
			"\t list			-				��ʾ�����������ǰ״̬\r\n"
			"\t fps			-				��ʾ��ǰ������FPS��Ϣ\r\n"
			"\r\n"
			"\t mtsb			��ɫ���� ����ID x���� y����	����ɫ�ƶ���ĳ���������ض�λ��\r\n"
			"\t playerinfo		��ɫ����			�鿴�����ƽ�ɫ����Ϣ\r\n"
			"\r\n"
			"\t sreset			-				�������롾�ű���\r\n"
			"\t reloaditem		-				���¼��ء����߱�\r\n"
			"\t reloaddropitem		-				���¼��ء���Ʒ�����б�\r\n"
			"\t reloadskill		-				���¼��ء��������ñ�\r\n"
			"\t reloadmonster		-				���¼��ء��������ñ�\r\n"
			"\t reloadbuff		-				���¼��ء�buff���ñ�\r\n"
			"\r\n"
			"\t kickoneplayer		��ɫ����			�߳��ض���һ�����\r\n"
			"\t kickall		-				�߳���ǰ�����������е����\r\n"
			"\t addsp			��ɫ���� ����			������������\r\n"
			"\t addexp			��ɫ���� ����			�������Ӿ���\r\n"
			"\t addmoney		��ɫ���� ����			�������ӽ�Ǯ\r\n"
			"\t additem		��ɫ���� ����ID ��������	�������ӵ���\r\n"
			"\t addpoint		��ɫ���� �������� ���ӵ���	�������ӿ��õ�\r\n"
			"\t							��������(1.����2.����3.�Ṧ4.����)"
			"\r\n"
			"\t killmonster		��ɫ��				ɱ����ǰ��ͼ�����й���\r\n"
			"\t createmonsterregin	��ͼID x���� y���� ����ID ����	�ض���ͼ�ض�����ˢ����\r\n"
			"\t relive			��ɫ�� ��������			����һ�����\r\n"
			"\t							��������(0.�س� 1.ԭ��free 2.ԭ�ظ�Ǯ)\r\n"
			"\t���÷�������֧�ֶ�̬���ӹ��ܡ�\r\n"
			"==============================================================\r\n"*/
			"�Ƚ���������\r\n"
			"\t sendmail		-			���ʼ������ arg1=���sid\r\n"
			"\t smsys		-				���ʼ�,ϵͳ��ȫ�� \r\n"
			"\t gm		-				ȫ������ \r\n"
			);
		break;

	case 2: // quit
		CSingleItem::SaveAllLog();
		GetApp()->m_bQuit = true;
		break;

	case 3: // list
		if (GetStringParameter(szCommand, 256, 1) == NULL)
		{
			// û��ʹ�ø��Ӳ���
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
		rfalse(2, 0, "��ӡ��ϣ�����\r\n");
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
				extern std::map<std::string, DWORD> GMTalkMask; // ����ӳ���
				extern LPIObject GetPlayerByName(LPCSTR);
				if (CPlayer *pPlayer = (CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER))
				{
					GMTalkMask[pPlayer->GetAccount()] = timeGetTime() + itime*60000;
					rfalse(2, 1, "�趨[%s]���ֽ�%s���˽���%d����", pPlayer->GetAccount(), (char *)szname, itime);
				}
				else
				{
					rfalse(2, 1, "�Ҳ������ֽ�%s����", (char *)szname);
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
				rfalse(2, 1, "��ǰ���������Ҳ���������");
			else
				rfalse(2, 1, "��[%s]�ƶ���[%d][%d/%d]", (char *)szname, regionid, x, y);
		}
		break;

	case 91: // playerinfo
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);

			// �������Ʋ������
			CPlayer* destPlayer = (CPlayer*)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if (!destPlayer)
			{
				rfalse(2, 1, "��ǰ��������û�д����");
				break;
			}

			// �ҵ��ˣ���ʾ����
			rfalse(2, 1, "==========================���[%s]����Ϣ==========================", (char*)szname);
			// 			rfalse(2, 1, "*��Ϸ״̬*");
			destPlayer->limitedState > 0 ? rfalse(2, 1,"����Ѿ������������ϵͳ��"): rfalse(2, 1,"�����δ���������ϵͳ��");
			if(destPlayer->limitedState ){
				rfalse(2, 1, "---����ʱ�䣺	[%d]����", destPlayer->offlineTime/60);
				rfalse(2, 1, "---����ʱ�䣺	[%d]����", destPlayer->m_OnlineTime/60);
			}
			rfalse(2, 1, "---����˺ţ�	%s", destPlayer->GetAccount());
			rfalse(2, 1, "*��������*");
			rfalse(2, 1, "---���ɣ�	%d", destPlayer->m_Property.m_School);
			rfalse(2, 1, "---��ǰ�ȼ���	%d", destPlayer->m_Level);
			rfalse(2, 1, "---�������:	%d", destPlayer->m_MaxHp);
			rfalse(2, 1, "---��ǰ������	%d", destPlayer->m_CurHp);
			rfalse(2, 1, "---���������	%d", destPlayer->m_MaxMp);
			rfalse(2, 1, "---��ǰ������	%d", destPlayer->m_CurMp);
			rfalse(2, 1, "---���������	%d", destPlayer->m_MaxTp);
			rfalse(2, 1, "---��ǰ������	%d", destPlayer->m_CurTp);
			rfalse(2, 1, "---��ǰ������	%d", destPlayer->m_Property.m_CurSp);

			rfalse(2, 1, "*ս������*");
			rfalse(2, 1, "---������	%d", destPlayer->m_GongJi);
			rfalse(2, 1, "---������	%d", destPlayer->m_FangYu);
			rfalse(2, 1, "---������	%d", destPlayer->m_BaoJi);
			rfalse(2, 1, "---���ܣ�	%d", destPlayer->m_ShanBi);
			rfalse(2, 1, "---�˺����⣺	%d", destPlayer->m_ReduceDamage);
			rfalse(2, 1, "---�����˺�:	%d", destPlayer->m_AbsDamage);
			rfalse(2, 1, "---���ӷ�����	%d", destPlayer->m_NonFangyu);
			rfalse(2, 1, "---����������	%d", destPlayer->m_MultiBaoJi);
			rfalse(2, 1, "---���˺���	%d", destPlayer->m_IceDamage);
			rfalse(2, 1, "---�����ԣ�	%d", destPlayer->m_IceDefence);
			rfalse(2, 1, "---���˺���	%d", destPlayer->m_FireDamage);
			rfalse(2, 1, "---���ԣ�	%d", destPlayer->m_FireDefence);
			rfalse(2, 1, "---���˺���	%d", destPlayer->m_XuanDamage);
			rfalse(2, 1, "---�����ԣ�	%d", destPlayer->m_XuanDefence);
			rfalse(2, 1, "---���˺���	%d", destPlayer->m_PoisonDamage);
			rfalse(2, 1, "---�����ԣ�	%d", destPlayer->m_PoisonDefence);

			rfalse(2, 1, "===================================================================================================\n");
		}
		break;
	case 92: // ison
	{
		dwt::stringkey<char[10]> indexstr = GetStringParameter(szCommand, 256, 1);
		int index = atoi(indexstr);
		// �������Ʋ������
		CPlayer* destPlayer = (CPlayer*)GetPlayerBySID(index)->DynamicCast(IID_PLAYER);
		if (!destPlayer)
		{
			rfalse(2, 1, "��ǰ��������û�д����");
			break;
		}

		// �ҵ��ˣ���ʾ����
		//rfalse(2, 1, "==========================���[%s]����Ϣ==========================", (char*)szname);
		// 			rfalse(2, 1, "*��Ϸ״̬*");
		destPlayer->limitedState > 0 ? rfalse(2, 1, "����Ѿ������������ϵͳ��") : rfalse(2, 1, "�����δ���������ϵͳ��");
		if (destPlayer->limitedState){
			rfalse(2, 1, "---����ʱ�䣺	[%d]����", destPlayer->offlineTime / 60);
			rfalse(2, 1, "---����ʱ�䣺	[%d]����", destPlayer->m_OnlineTime / 60);
		}
		rfalse(2, 1, "---����˺ţ�	%s", destPlayer->GetAccount());
		rfalse(2, 1, "*��������*");
		rfalse(2, 1, "---���ɣ�	%d", destPlayer->m_Property.m_School);
		rfalse(2, 1, "---��ǰ�ȼ���	%d", destPlayer->m_Level);
		rfalse(2, 1, "---�������:	%d", destPlayer->m_MaxHp);
		rfalse(2, 1, "---��ǰ������	%d", destPlayer->m_CurHp);
		rfalse(2, 1, "---���������	%d", destPlayer->m_MaxMp);
		rfalse(2, 1, "---��ǰ������	%d", destPlayer->m_CurMp);
		rfalse(2, 1, "---���������	%d", destPlayer->m_MaxTp);
		rfalse(2, 1, "---��ǰ������	%d", destPlayer->m_CurTp);
		rfalse(2, 1, "---��ǰ������	%d", destPlayer->m_Property.m_CurSp);

		rfalse(2, 1, "*ս������*");
		rfalse(2, 1, "---������	%d", destPlayer->m_GongJi);
		rfalse(2, 1, "---������	%d", destPlayer->m_FangYu);
		rfalse(2, 1, "---������	%d", destPlayer->m_BaoJi);
		rfalse(2, 1, "---���ܣ�	%d", destPlayer->m_ShanBi);
		rfalse(2, 1, "---�˺����⣺	%d", destPlayer->m_ReduceDamage);
		rfalse(2, 1, "---�����˺�:	%d", destPlayer->m_AbsDamage);
		rfalse(2, 1, "---���ӷ�����	%d", destPlayer->m_NonFangyu);
		rfalse(2, 1, "---����������	%d", destPlayer->m_MultiBaoJi);
		rfalse(2, 1, "---���˺���	%d", destPlayer->m_IceDamage);
		rfalse(2, 1, "---�����ԣ�	%d", destPlayer->m_IceDefence);
		rfalse(2, 1, "---���˺���	%d", destPlayer->m_FireDamage);
		rfalse(2, 1, "---���ԣ�	%d", destPlayer->m_FireDefence);
		rfalse(2, 1, "---���˺���	%d", destPlayer->m_XuanDamage);
		rfalse(2, 1, "---�����ԣ�	%d", destPlayer->m_XuanDefence);
		rfalse(2, 1, "---���˺���	%d", destPlayer->m_PoisonDamage);
		rfalse(2, 1, "---�����ԣ�	%d", destPlayer->m_PoisonDefence);

		rfalse(2, 1, "===================================================================================================\n");
	}
	break;
	case 93: // ison
	{
		dwt::stringkey<char[10]> indexstr = GetStringParameter(szCommand, 256, 1);
		int index = atoi(indexstr);
		// �������Ʋ������
		CPlayer* destPlayer = (CPlayer*)GetPlayerByDnid(index)->DynamicCast(IID_PLAYER);
		if (!destPlayer)
		{
			rfalse(2, 1, "��ǰ��������û�д����");
			break;
		}

		// �ҵ��ˣ���ʾ����
		//rfalse(2, 1, "==========================���[%s]����Ϣ==========================", (char*)szname);
		// 			rfalse(2, 1, "*��Ϸ״̬*");
		destPlayer->limitedState > 0 ? rfalse(2, 1, "����Ѿ������������ϵͳ��") : rfalse(2, 1, "�����δ���������ϵͳ��");
		if (destPlayer->limitedState){
			rfalse(2, 1, "---����ʱ�䣺	[%d]����", destPlayer->offlineTime / 60);
			rfalse(2, 1, "---����ʱ�䣺	[%d]����", destPlayer->m_OnlineTime / 60);
		}
		rfalse(2, 1, "---����˺ţ�	%s", destPlayer->GetAccount());
		rfalse(2, 1, "*��������*");
		rfalse(2, 1, "---���ɣ�	%d", destPlayer->m_Property.m_School);
		rfalse(2, 1, "---��ǰ�ȼ���	%d", destPlayer->m_Level);
		rfalse(2, 1, "---�������:	%d", destPlayer->m_MaxHp);
		rfalse(2, 1, "---��ǰ������	%d", destPlayer->m_CurHp);
		rfalse(2, 1, "---���������	%d", destPlayer->m_MaxMp);
		rfalse(2, 1, "---��ǰ������	%d", destPlayer->m_CurMp);
		rfalse(2, 1, "---���������	%d", destPlayer->m_MaxTp);
		rfalse(2, 1, "---��ǰ������	%d", destPlayer->m_CurTp);
		rfalse(2, 1, "---��ǰ������	%d", destPlayer->m_Property.m_CurSp);

		rfalse(2, 1, "*ս������*");
		rfalse(2, 1, "---������	%d", destPlayer->m_GongJi);
		rfalse(2, 1, "---������	%d", destPlayer->m_FangYu);
		rfalse(2, 1, "---������	%d", destPlayer->m_BaoJi);
		rfalse(2, 1, "---���ܣ�	%d", destPlayer->m_ShanBi);
		rfalse(2, 1, "---�˺����⣺	%d", destPlayer->m_ReduceDamage);
		rfalse(2, 1, "---�����˺�:	%d", destPlayer->m_AbsDamage);
		rfalse(2, 1, "---���ӷ�����	%d", destPlayer->m_NonFangyu);
		rfalse(2, 1, "---����������	%d", destPlayer->m_MultiBaoJi);
		rfalse(2, 1, "---���˺���	%d", destPlayer->m_IceDamage);
		rfalse(2, 1, "---�����ԣ�	%d", destPlayer->m_IceDefence);
		rfalse(2, 1, "---���˺���	%d", destPlayer->m_FireDamage);
		rfalse(2, 1, "---���ԣ�	%d", destPlayer->m_FireDefence);
		rfalse(2, 1, "---���˺���	%d", destPlayer->m_XuanDamage);
		rfalse(2, 1, "---�����ԣ�	%d", destPlayer->m_XuanDefence);
		rfalse(2, 1, "---���˺���	%d", destPlayer->m_PoisonDamage);
		rfalse(2, 1, "---�����ԣ�	%d", destPlayer->m_PoisonDefence);

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
			// ���ڵ�������Ŀ���
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
			//	rfalse(2, 1, "LoadNewItemData��ȡʧ��.");
			//else
			//	rfalse(2, 1, "LoadNewItemData�Ѿ��ɹ�����.");
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
					rfalse( 2, 1, "δ�ҵ�ָ����ɫ��" );
				}
			}
		}
		break;

	case 131:
		{
			int num = GetApp()->m_Server.GetLinkByFlags( tmp, 3000, 1 );
			LPCSTR str =  GetStringParameter( szCommand, 256, 1 );
			rfalse( 2, 1, "Ŀǰʹ����ҵ�������%d", num );
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

			rfalse(2, 1, "�Ѿ���������GMCheckURL!");
		}
		break;
	case 135:
		{
			// ��ȡ�����ȼ������б��ļ�
			extern int LoadLevelAttributeArray( void );
			LoadLevelAttributeArray();
			rfalse(2, 1, "LevelAttributeArray���سɹ�!");
		}
		break;
	case 136:
		{
			GetApp()->m_Server.SetMacLimit( 0 );
			rfalse(2, 1, "������Ϸ�࿪��");
		}
		break;

	case 399:	// kickoneplayer   kickһ�����
		{

		}
		break;

	case 400:
		{
			CSkillService::GetInstance().ReLoad();
			rfalse(2, 1, "�����б����¼�����ϣ�");
		}
		break;

	case 401:
		{
			CItemService::GetInstance().ReLoad();
			rfalse(2, 1, "���߱����¼�����ϣ�");
		}
		break;

	case 402:
		{
			CItemDropService::GetInstance().Reload();
			rfalse(2, 1, "��Ʒ��������¼�����ϣ�");
		}
		break;

	case 403:
		{
			CMonsterService::GetInstance().ReLoad();
			rfalse(2, 1, "��������¼�����ϣ�");
		}
		break;

	case 404:
		{
			CBuffService::GetInstance().ReLoad();
			rfalse(2, 1, "Buff�����¼�����ϣ�");
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
				rfalse(2, 1, "��ǰ���������Ҳ���������");
				break;
			}
			int regionid = atoi(szregionid);
			int x = atoi(szx);
			int y = atoi(szy);

			g_Script.CallFunc("PutPlayerToDynamicRegion",regionid,x,y,player->GetGID());
			//rfalse(2, 1, "��ǰ���������Ҳ���������");
		}
		break;
	case 407://PKģʽ����ʱ��������� setpk
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> pkrule = GetStringParameter(szCommand, 256, 2);
			CPlayer* player=(CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2, 1, "��ǰ���������Ҳ���������");
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
			sprintf(playernum,"��ǰ����������%d��",num);
			AddInfo(playernum);
			break;
		}
		break;
	case 410://createmonster(�������,id,����)
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> monterid = GetStringParameter(szCommand, 256, 2);
			dwt::stringkey<char[10]> monternum = GetStringParameter(szCommand, 256, 3);

			CPlayer* player=(CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2, 1, "��ǰ���������Ҳ���������");
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
	case 	411: //��Ǯ(����,����)showmethemoney
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> moneynum = GetStringParameter(szCommand, 256, 2);

			CPlayer* player=(CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2, 1, "��ǰ���������Ҳ���������");
				break;
			}
			int num=atoi(moneynum);
			player->AddPlayerMoney(1,num);
			player->AddPlayerMoney(2,num);
			player->AddPlayerMoney(3,num);
		}
		break;
	case 	412: //�Ӿ���(����,����)addexp
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> moneynum = GetStringParameter(szCommand, 256, 2);

			CPlayer* player=(CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2, 1, "��ǰ���������Ҳ���������");
				break;
			}
			int num=atoi(moneynum);
			player->SendAddPlayerExp(num, SAExpChangeMsg::GM, "GMGive");
		}
		break;
	case 	413: //����Ʒ(����,ID,����)additem
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> itemidstr = GetStringParameter(szCommand, 256, 2);
			dwt::stringkey<char[10]> itemnumstr = GetStringParameter(szCommand, 256, 3);

			CPlayer* player=(CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2, 1, "��ǰ���������Ҳ���������");
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
	case 	414: //�ӵ�(����,ID,����)addpoint
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> typestr = GetStringParameter(szCommand, 256, 2);
			dwt::stringkey<char[10]> valuestr = GetStringParameter(szCommand, 256, 3);

			CPlayer* player=(CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2, 1, "��ǰ���������Ҳ���������");
				break;
			}
			int pointtype=atoi(typestr) + 22;	// ���õ����Ǵ�ö������23��ʼ��ö������27��һ��������
			int pointvalue=atoi(valuestr);

			player->AddPlayerPoint(pointtype,pointvalue);
			//player->StartAddItems(itemList);
		}

		break;
	case 415://�򿪵�ͼ��ײ
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
	case 416://�رյ�ͼ��ײ
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

	case 417://�Ժ���ʹ�õ���
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
	case 418://����ʹ������װ��equippet
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
	case 419://����ʹ������װ��unequippet
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
	case 420://���Դӽű��з��ؼ������Ӧ�ļ���IDgetitemskill
		{
			dwt::stringkey<char[10]> itemidstr= GetStringParameter(szCommand, 256, 1);
			int itemid = atoi(itemidstr);

			// �����ű���չ
			lite::Variant ret;
			g_Script.SetCondition(NULL, NULL, NULL);
			LuaFunctor(g_Script, "GetItemTofpSkillID")[itemid](&ret);
			g_Script.CleanCondition();

			if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)
				break;
			int skillid = ( __int32 )ret;
		}
		break;
	case 421://��������ת������fptoitem
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
	case 422://�����ٻ�����calloutfp
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
	case 423://����ɾ������["killmonster"] = 423;
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
	case 424://��������״̬settaskInfo
		{
			dwt::stringkey<char[CONST_USERNAME]> myname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> indexstr= GetStringParameter(szCommand, 256, 2);
			dwt::stringkey<char[10]> statusstr= GetStringParameter(szCommand, 256, 3);
			CPlayer *player =(CPlayer*) GetPlayerByName(myname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2,1,FormatString("�Ҳ������ֽ�%s�����",myname));
				return;
			}
			WORD taskID = atoi(indexstr);
			WORD taskStatus = atoi(statusstr);
			player->m_TaskInfo.push_back(make_pair(taskID, taskStatus));
		}
		break;
	case 425://createmonsterregin ;//��ͼˢ�� mapid x y monsterid num
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
	case 426: //����
		{
			dwt::stringkey<char[CONST_USERNAME]> myname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> indexstr= GetStringParameter(szCommand, 256, 2);

			CPlayer *player =(CPlayer*) GetPlayerByName(myname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2,1,"�Ҳ����������!");
				return;
			}
			WORD liveID = atoi(indexstr);
			player->QuestToRelive(liveID);
		}
		break;
	case 427:// addsp ������ ���������������ֵ��
		{
			dwt::stringkey<char[CONST_USERNAME]> szname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> spnum = GetStringParameter(szCommand, 256, 2);

			CPlayer* player=(CPlayer *)GetPlayerByName(szname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2, 1, "��ǰ���������Ҳ���������");
				break;
			}
			int num=atoi(spnum);
			player->AddPlayerSp(num);
		}
		break;
	case 428:  //["setlevel"]				= 427;	// ������ҵȼ�
		{
			dwt::stringkey<char[CONST_USERNAME]> myname = GetStringParameter(szCommand, 256, 1);
			dwt::stringkey<char[10]> levelstr= GetStringParameter(szCommand, 256, 2);
			CPlayer *player =(CPlayer*) GetPlayerByName(myname)->DynamicCast(IID_PLAYER);
			if(!player)
			{
				rfalse(2,1,"�Ҳ����������!");
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
	case 430:	//�������GM�ȼ�
	{
					dwt::stringkey<char[CONST_USERNAME]> myname = GetStringParameter(szCommand, 256, 1);
					dwt::stringkey<char[10]> GMLevelStr = GetStringParameter(szCommand, 256, 2);
					CPlayer *player = (CPlayer*)GetPlayerByName(myname)->DynamicCast(IID_PLAYER);
					if (!player)
					{
						rfalse(2, 1, "�Ҳ����������!");
						return;
					}
					WORD GMLevel = atoi(GMLevelStr);
					player->m_Property.m_GMLevel = GMLevel;
	}
		break;
	case 431:	//����ϵͳ����
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
	case 432: //��ӡ��������鿴
	{
		dwt::stringkey<char[100]> mystr = GetStringParameter(szCommand, 256, 1);
		//int nstr = atoi(mystr);
		strcat(mystr, "\n");
		AddInfo(mystr);
	}
		break;


	////**********************************************////
	////****************�ȶ�����ϵͳ********************////
	////**********************************************////


	case 500: //ϵͳ����
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
		AddInfo("ϵͳ���淢�ͳɹ�\n");
	}
		break;
	case 501: //ģ����ҵ��ȡ�ʼ���Ϣ
	{
		dwt::stringkey<char[10]> indexstr = GetStringParameter(szCommand, 256, 1);
		int index = atoi(indexstr);
		AddInfo("ģ����ҵ��ȡ�ʼ���Ϣ");
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

		AddInfo("���ʼ���Ϣ");
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
		strcpy_s(_MailInfo->content, "��������~~~~ɧ��,�㷢��!,�콱��!");
		strcpy_s(_MailInfo->senderName, "ϵͳ GM��");
		strcpy_s(_MailInfo->title, "���Ǳ���_�������������!");
		MailMoudle::getSingleton()->SendMail_SanGuo(index, 3*24*60*60, _MailInfo);
		delete _MailInfo;
	}
		break;
	case 503://ȡ����
	{

		AddInfo("ȡ����");
		dwt::stringkey<char[10]> indexstr = GetStringParameter(szCommand, 256, 1);
		int index = atoi(indexstr);
		SQ_GetAwards_MSG *msg = new SQ_GetAwards_MSG;
		msg->m_dwMailId = index;
		MailMoudle::getSingleton()->GetMailAwards(5,msg,5);
		delete msg;
	}
		break;
	case 504://ɾ��
	{

		AddInfo("ɾ��");
		dwt::stringkey<char[10]> indexstr = GetStringParameter(szCommand, 256, 1);
		int index = atoi(indexstr);
		SQ_SetMailState_MSG *msg = new SQ_SetMailState_MSG;
		msg->m_dwMailId = index;
		MailMoudle::getSingleton()->SetMailState(msg,5);
		delete msg;
	}
		break;
	case 505://��ȫ���ʼ���Ϣ
	{
		dwt::stringkey<char[10]> indexstr = GetStringParameter(szCommand, 256, 1);
		int index = atoi(indexstr);

		AddInfo("��ȫ���ʼ���Ϣ");
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
		strcpy_s(_MailInfo->content, "      ɧ��,�ҿ����������,�β���һ��ά�������ƽ,������������!");
		strcpy_s(_MailInfo->senderName, "�����гͳս��˼�ܴ�");
		strcpy_s(_MailInfo->title, "���Խ𽫾����ʺ�");
		/*for (int i = 0; i < 10000;i++)
		{
		MailMoudle::getSingleton()->SendMail_SYS_SanGuo(10 * 24 * 60 * 60, _MailInfo);
		}*/
		MailMoudle::getSingleton()->SendMail_SYS_SanGuo(10 * 24 * 60 * 60, _MailInfo);
		delete _MailInfo;


		
	}
	break;
	case 506://����
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
		//��Ṧ�ܼ��
		CPlayer *pDestPlayer = (CPlayer *)GetPlayerByName("gmphy5name")->DynamicCast(IID_PLAYER);
		//GetGW()->m_FactionManager.CreateFaction("ss", pDestPlayer, 10001);
	}
	break;

	case 508:
	{
		//��Ṧ�ܼ��
		CPlayer *pDestPlayer = (CPlayer *)GetPlayerByName("gmphy20name")->DynamicCast(IID_PLAYER);
		//GetGW()->m_FactionManager.AddMember("ss", pDestPlayer);
	}
	break;

	case 509:
	{
		//��Ṧ�ܼ��
		//GetGW()->m_FactionManager.Run();
	}
	break;

	case 510: //��ȡ����
	{
		// ��DB Server�����ȡ��������
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
// 		//�㲥��Ϣ
// 	}
}
