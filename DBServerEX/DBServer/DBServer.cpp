// DBServer.cpp : ����Ӧ�ó������ڵ㡣

#include "stdafx.h"
#include "DBServer.h"

#include "DMainApp.h"
#include "NetworkModule\DataMsgs.h"
#include "pub/minihelp.h"

#define MAX_LOADSTRING 100
class CCommand;
extern void AddInfo(LPCSTR Info);

// ȫ�ֱ�����
HINSTANCE hInst;								// ��ǰʵ��
HWND hInfoWin, hMainWnd;                        // ��ʾ��Ϣ�Ĵ���
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������
HWND  hInputWin;                                // ���봰��Handle
LRESULT CALLBACK ReloadEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// �˴���ģ���а����ĺ�����ǰ��������
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
	//���ڱ�Ŀ¼������һ��Exception.dmpʹ��windbg����ֱ����vs �򿪣����õ��Ծ��ܶ�λ����
	SetUnhandledExceptionFilter(GPTUnhandledExceptionFilter);

	// ��������Ҫ��Ϊ�˽������˳���µ��޷���ȷ�����ڴ��ͷŵ�����
	std::map<CCommand, int> command_map;
	InitCommandMap( command_map );
	pcmap = &command_map;

 	// TODO: �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_DBSERVER, szWindowClass, MAX_LOADSTRING);
    strcpy( szTitle, "DBServerEx" );
    strcpy( szWindowClass, "DBServerEx" );
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ����
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_DBSERVER);

	// ����Ϣѭ����
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
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_DBSERVER);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;//(LPCTSTR)IDC_DBSERVER;
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

	case WM_CLOSE:
        GetApp().ExitServer();
		// DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
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
			SendMessage(hInfoWin, EM_REPLACESEL, 0, (LPARAM)"\r\nCMD����");
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

	// ����������ʹ��[]��������ֹӳ���Ķ�̬����
	it = pcmap->find(Param1);
	if (it == pcmap->end())
	{
		AddInfo("��Ч������\r\n");
		return;
	}

	switch(it->second)
	{
	case 1: // help
		AddInfo("�����б�:\r\n"
			"help	��ǰ����\r\n"
			"quit	�رոó�ʽ\r\n"
			"list	��ʾ��ǰ�������б�\r\n"
			"save   �������з������ϵ�����\r\n"
			"print  ��ʾ����ʱ��\r\n"
			"item_find ͨ��ΨһID���ҵ��߰���Ϣ\r\n"
			"item_check ������е��ߵİ���Ϣ\r\n"
			"writerole д���н�ɫ��Ϣ\r\n"
			"�÷�������֧�ֶ�̬���ӹ���");
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
			AddInfo("�������� \r\n");
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
