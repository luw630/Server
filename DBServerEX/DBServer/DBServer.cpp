// DBServer.cpp : 定义应用程序的入口点。

#include "stdafx.h"
#include "DBServer.h"

#include "DMainApp.h"
#include "NetworkModule\DataMsgs.h"
#include "pub/minihelp.h"

#define MAX_LOADSTRING 100
class CCommand;
extern void AddInfo(LPCSTR Info);

// 全局变量：
HINSTANCE hInst;								// 当前实例
HWND hInfoWin, hMainWnd;                        // 显示信息的窗口
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名
HWND  hInputWin;                                // 输入窗口Handle
LRESULT CALLBACK ReloadEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// 此代码模块中包含的函数的前向声明：
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

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
		return dwt::strcmp(szCommand, dest.szCommand, 32) < 0;
	}
};

int InitCommandMap( std::map<CCommand, int> &command_map )
{
	command_map["help"] = 1;
	command_map["quit"] = 2;
	command_map["list"] = 3;
    command_map["save"] = 4;
	command_map["print"] = 5;
	command_map["item_find" ] = 6;
	command_map["item_check" ] = 7;
	command_map["resetbackup" ] = 8;
	command_map["backup" ] = 9;
	command_map["writerole"] = 10;
	command_map["test"] = 11;
	return 0;
};

std::map<CCommand, int> *pcmap = NULL;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	//将在本目录下生成一个Exception.dmp使用windbg或者直接用vs 打开，启用调试就能定位问题
	SetUnhandledExceptionFilter(GPTUnhandledExceptionFilter);

	// 这样做主要是为了解决析构顺序导致的无法正确分析内存释放的问题
	std::map<CCommand, int> command_map;
	InitCommandMap( command_map );
	pcmap = &command_map;

 	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_DBSERVER, szWindowClass, MAX_LOADSTRING);
    strcpy( szTitle, "DBServerEx" );
    strcpy( szWindowClass, "DBServerEx" );
	MyRegisterClass(hInstance);

	// 执行应用程序初始化：
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_DBSERVER);

	// 主消息循环：
    if ( !CreateApp() )
    {
        return FALSE;
    }

    if ( !GetApp().InitServer() )
    {
        return FALSE;
    }

    while ( true /*GetApp().m_bQuit != 1*/ )
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
            GetApp().LoopServer();
        }
    }

    ClearApp();

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
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_DBSERVER);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;//(LPCTSTR)IDC_DBSERVER;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

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
WNDPROC OldProc;
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // 将实例句柄存储在全局变量中

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED,
		CW_USEDEFAULT, 0, 400, 200, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return FALSE;

    hMainWnd = hWnd;

	RECT WinRect;
	GetClientRect(hWnd,&WinRect);

#define __input_h 17
#define __input_x 46

	hInfoWin = CreateWindowEx(WS_EX_CLIENTEDGE,"edit","",WS_CHILDWINDOW|WS_VISIBLE|WS_VSCROLL|ES_READONLY|ES_MULTILINE,
		0,0,WinRect.right-WinRect.left,WinRect.bottom-__input_h,hWnd,0,hInstance,NULL);
	if(!hInfoWin)
	{
		return FALSE;
	}

	hInputWin = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", WS_CHILDWINDOW | WS_VISIBLE,
		0,WinRect.bottom-__input_h, WinRect.right-WinRect.left,__input_h, hWnd, (HMENU)IDC_INPUT, hInstance, NULL);
	if(!hInputWin)
	{
		return FALSE;
	}
	HFONT hFont;
	hFont =  CreateFont(12,6,0,0,400,0,0,0,0,0,0,0,1,"新宋体");
	SetWindowFont(hInputWin, hFont, 1);
	SetWindowFont(hInfoWin,hFont, 1);

	OldProc = (WNDPROC)(_W64 LONG)SetWindowLong(hInputWin, GWL_WNDPROC, (LONG)(_W64 LONG)ReloadEditWndProc);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	// DeleteObject(hFont);
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// 分析菜单选择：
		switch (wmId)
		{
        case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_CLOSE:
        GetApp().ExitServer();
		// DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
        break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

// “关于”框的消息处理程序。
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void AddInfo(LPCSTR Info)
{
    int nLines = (int)SendMessage(hInfoWin, EM_GETLINECOUNT, 0, 0);
    if (nLines >= 256)
    {
        int nIndex = (int)SendMessage(hInfoWin, EM_LINEINDEX, 10, 0);
        if (nIndex != -1)
        {
            SendMessage(hInfoWin, EM_SETSEL, 0, nIndex);
            SendMessage(hInfoWin, EM_REPLACESEL, 0, (LPARAM)"");
        }
    }
    int end = (int)SendMessage(hInfoWin, EM_GETLIMITTEXT, 0, 0);
    SendMessage(hInfoWin, EM_SETSEL, end, end);
    SendMessage(hInfoWin, EM_REPLACESEL, 0, (LPARAM)Info);
}

char InputBuffer[256];
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

			end = (int)SendMessage(hInfoWin, EM_GETLIMITTEXT, 0, 0);
			SendMessage(hInfoWin, EM_SETSEL, end, end);
			SendMessage(hInfoWin, EM_REPLACESEL, 0, (LPARAM)"\r\nCMD：〉");
			//			SendMessage(hInfo, EM_REPLACESEL, 0, (LPARAM)InputBuffer);

			AddInfo(InputBuffer);

			void ExplainCommand(LPCSTR szCommand);
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

static QWORD HexStringToNumber64( LPCSTR str )
{
    #define IsHexNumber( c ) ( ( c >= '0'&& c <= '9' ) || ( c >= 'A'&& c <= 'F' ) || ( c >= 'a'&& c <= 'f' ) )
    #define GetHexNumber( c ) ( ( c >= '0'&& c <= '9' ) ? ( c - '0' ) : ( 10 + ( ( c >= 'A'&& c <= 'F' ) ? ( c - 'A' ) : ( c - 'a' ) ) ) )

    if ( str == NULL )
        return 0;

    QWORD result = 0;

    if ( str[0] == '0' && str[1] == 'x' )
        str += 2;

    for ( int i = 0; ( i < 16 ) && ( str[i] != 0 ); i ++ )
    {
        if ( !IsHexNumber( str[i] ) )
            break;

        result = ( result << 4 ) | GetHexNumber( str[i] );
    }

    #undef GetHexNumber
    #undef IsHexNumber

    return result;
}

#define MAX_PARAM_SIZE 256
#define isParamSplit(x) ((x == '\0') || (x == '\t') || (x == '\r') || (x == '\n') || (x == ' '))
LPCSTR GetStringParameter(LPCSTR string, int maxsize, int index)
{
	static char szParameter[MAX_PARAM_SIZE] = "";
	int param_count = 0;
	int size_count = 0;
	int param_seg = 0;
	int param_end = 0;
	int in_param = 0;
	if (index == 0) return string;

	while ((size_count < maxsize) && (string[size_count] != 0))
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

extern void _TypeItemInfo( QWORD id );
extern void _CheckItemInfo();

void ExplainCommand(LPCSTR szCommand)
{
	LPCSTR Param1 = GetStringParameter(szCommand, 256, 1);
	std::map<CCommand, int>::iterator it;

	if (Param1 == NULL) return;

	// 这样做（不使用[]）可以阻止映射表的动态膨胀
	it = pcmap->find(Param1);
	if (it == pcmap->end())
	{
		AddInfo("无效的命令\r\n");
		return;
	}

	switch(it->second)
	{
	case 1: // help
		AddInfo("命令列表:\r\n"
			"help	当前帮助\r\n"
			"quit	关闭该程式\r\n"
			"list	显示当前服务器列表\r\n"
			"save   保存所有服务器上的数据\r\n"
			"print  显示保存时间\r\n"
			"item_find 通过唯一ID查找道具绑定信息\r\n"
			"item_check 检查所有道具的绑定信息\r\n"
			"writerole 写所有角色信息\r\n"
			"该服务器不支持动态连接功能");
		break;

    case 2: // quit
		GetApp().ExitServer();
		break;

    case 3: // list
		GetApp().DisplayInfo();
		break;

	case 4: // save
		GetApp().SaveAll();
		break;

	case 5: // test_param
		GetApp().PrintTimeLog();
		break;

    case 6: // item_find
        if ( LPCSTR arg2 = GetStringParameter( szCommand, 256, 2 ) )
            _TypeItemInfo( HexStringToNumber64( arg2 ) );
        break;

    case 7: // item_check
        _CheckItemInfo();
        break;

    case 8:
        GetApp().LoadBackupSetting();
        break;

    case 9:
        GetApp().Backup();
		break;

	case 10:
		GetApp().WriteRole();
		AddInfo( "WriteRole Ok ! \r\n" );
		break;
	case 11:
	{
		LPCSTR aa = GetStringParameter(szCommand, 256, 2);
		if (aa==NULL)
		{
			AddInfo("参数出错 \r\n");
			return ;
		}
		string str = (char *)GetStringParameter(szCommand, 256, 2);
		GetApp().test(str);
		AddInfo("test Ok ! \r\n");
	}
		
		break;
	default:
		break;
	}
}
