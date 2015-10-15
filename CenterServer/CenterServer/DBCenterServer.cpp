// AccountServer.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "windowsx.h"
#include "DBCenterServer.h"
#include "DMainApp.h"
#include "pub/dwt.h"
#include <fstream>

#include <stdio.h>

#include "engine.h"


using namespace std;
#define MAX_LOADSTRING 100

// 全局变量：
extern void AddInfo(LPCSTR Info);

class CCommand;
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING] = "log server";					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING] = "log server";			// 主窗口类名
HWND  hInfoWin;                                 // 信息窗口Handle
HWND  hInputWin;                                // 输入窗口Handle
HWND  hMainWin;
LRESULT CALLBACK ReloadEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//std::hash_map<CCommand, int> command_map; so slow !!!

// 此代码模块中包含的函数的前向声明：
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_DBCENTERSERVER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化：
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_DBCENTERSERVER);

	// 主消息循环：
    if ( !CreateApp() )
        return false;

    if ( !GetApp().InitServer() )
    {
        MessageBox( NULL, "初始化失败", NULL, 0 );
        return false;
    }

    while ( !GetApp().m_bQuit )
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

    if ( !GetApp().ExitServer() )
        MessageBox( NULL, "退出过程中出现异常", NULL, 0 );

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
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_DBCENTERSERVER);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL; //(LPCTSTR)IDC_ACCOUNTSERVER;
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

   hMainWin = hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED,
      CW_USEDEFAULT, 0, 400, 300, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
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

    return;
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
        return dwt::strcmp(szCommand, dest.szCommand, 32) < 0;
    }
};

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
            SendMessage(hInfoWin, EM_REPLACESEL, 0, (LPARAM)"CMD：〉");
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
                AddInfo( "parameter buffer overflow!\r\n" );
            }
        }

        size_count ++;
    }

    return NULL;
}

std::map<CCommand, int> command_map;

int InitCommandMap()
{
    command_map["help"] = 1;
    command_map["quit"] = 2;
    command_map["list"] = 3;
	command_map["reload_con"] = 4;
	command_map["reload_wl"] = 5;
    return 0;
};

int l = InitCommandMap();
void reload();
void ExplainCommand(LPCSTR szCommand)
{
    LPCSTR Param1 = GetStringParameter(szCommand, 256, 1);
    std::map<CCommand, int>::iterator it;

    if (Param1 == NULL) return;

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
        AddInfo("命令列表:\r\n"
            "help	当前帮助\r\n"
            "quit	关闭该程式\r\n"
            "list	显示当前服务器列表\r\n"
			"debug  显示登录耗时开关\r\n"
            "该服务器不支持动态连接功能");
        break;
    case 2: // quit
        GetApp().m_bQuit = true;
        break;
    case 3: // list
        if (GetStringParameter(szCommand, 256, 2) == NULL)
        {
            // 没有使用附加参数
            //GetApp().DisplayServerInfo();
        }
        break;
	case 4:
		reload();
		break;
	case 5:
		GetApp().LoadWhiteList();
		break;
    case 99: // test_param
        break;
    default:
        break;
    }


}
 void reload()
{
	CDAppMain& appMain = GetApp();
	ZeroMemory(appMain.m_AssetbundleVersionInfo, MAX_ASSET_SIZE);
	File fii;
	fii.Open("Announcement.xml");
	appMain.m_iAssetBundleInfoLength = fii.Size();
	fii.Read(appMain.m_AssetbundleVersionInfo, fii.Size());
	fii.Close();
}
