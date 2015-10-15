// AccountServer.cpp : ����Ӧ�ó������ڵ㡣
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

// ȫ�ֱ�����
extern void AddInfo(LPCSTR Info);

class CCommand;
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING] = "log server";					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING] = "log server";			// ����������
HWND  hInfoWin;                                 // ��Ϣ����Handle
HWND  hInputWin;                                // ���봰��Handle
HWND  hMainWin;
LRESULT CALLBACK ReloadEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//std::hash_map<CCommand, int> command_map; so slow !!!

// �˴���ģ���а����ĺ�����ǰ��������
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_DBCENTERSERVER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ����
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_DBCENTERSERVER);

	// ����Ϣѭ����
    if ( !CreateApp() )
        return false;

    if ( !GetApp().InitServer() )
    {
        MessageBox( NULL, "��ʼ��ʧ��", NULL, 0 );
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
        MessageBox( NULL, "�˳������г����쳣", NULL, 0 );

    ClearApp();
    
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
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_DBCENTERSERVER);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL; //(LPCTSTR)IDC_ACCOUNTSERVER;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

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
WNDPROC OldProc;
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

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
	hFont =  CreateFont(12,6,0,0,400,0,0,0,0,0,0,0,1,"������");
    SetWindowFont(hInputWin, hFont, 1);
    SetWindowFont(hInfoWin,hFont, 1);

   OldProc = (WNDPROC)(_W64 LONG)SetWindowLong(hInputWin, GWL_WNDPROC, (LONG)(_W64 LONG)ReloadEditWndProc);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
 // DeleteObject(hFont);
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

		// �����˵�ѡ��
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
		// TODO: �ڴ���������ͼ����...
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

// �����ڡ������Ϣ�������
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
            SendMessage(hInfoWin, EM_REPLACESEL, 0, (LPARAM)"CMD����");
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
        AddInfo("�����б�:\r\n"
            "help	��ǰ����\r\n"
            "quit	�رոó�ʽ\r\n"
            "list	��ʾ��ǰ�������б�\r\n"
			"debug  ��ʾ��¼��ʱ����\r\n"
            "�÷�������֧�ֶ�̬���ӹ���");
        break;
    case 2: // quit
        GetApp().m_bQuit = true;
        break;
    case 3: // list
        if (GetStringParameter(szCommand, 256, 2) == NULL)
        {
            // û��ʹ�ø��Ӳ���
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
