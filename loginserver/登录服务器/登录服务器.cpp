#include "stdafx.h"
#include "��¼������.h"
#include "DMainApp.h"
#include "IDllInterface.h"
#include "NETWORKMODULE\UnionMsgs.h"
#include "pub/minihelp.h"

#define MAX_LOADSTRING 100

// ȫ�ֱ�����
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

HWND hMainWnd;
HWND hInfo; 
HWND hInput;
HMODULE hndDLLInstance = NULL;
IDllInterface *dllinterface = NULL;

WNDPROC OldProc;
char InputBuffer[256] = {0};
LRESULT CALLBACK ReloadEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void ExplainCommand(LPCSTR szCommand);

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
	// _set_security_error_handler( report_failure );
	InitMinDump();
    MSG msg;
    HACCEL hAccelTable;

    // ��ʼ��ȫ���ַ���
    //LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    //LoadString(hInstance, IDC_MY, szWindowClass, MAX_LOADSTRING);
    strcpy( szTitle, "LoginServer" );
    strcpy( szWindowClass, "LoginServer" );
    MyRegisterClass(hInstance);

    // ִ��Ӧ�ó����ʼ����
    if (!InitInstance (hInstance, nCmdShow)) 
        return FALSE;

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_MY);

    CreateApp();
    if (!GetApp()->InitServer())
    {
        ClearApp();
        return rfalse("��ʼ��ʧ��");
    }

	////����װ�ض�̬DLL
	//ExplainCommand( "ReLoadDll\r\n" );

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
            GetApp()->LoopServer();
        }
    }

    GetApp()->ExitServer();
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
    wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_MY);
    wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName	= NULL;//(LPCTSTR)IDC_MY;
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����
    hMainWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME,
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

    int size = dwt::strlen(Info, 10000);

    if (max_size + size > 30000)
    {
        int nIndex = (int)SendMessage(hInfo, EM_LINEINDEX, 64, 0);
        if (nIndex < size)
        {
            nIndex = -1;
            max_size = 0;
        }
        else
        {
            max_size -= nIndex;
        }

        SendMessage(hInfo, EM_SETSEL, 0, nIndex);
        SendMessage(hInfo, EM_REPLACESEL, 0, (LPARAM)"");
    }

    max_size += size;

    SendMessage(hInfo, EM_SETSEL, 30000, 30000);
    SendMessage(hInfo, EM_REPLACESEL, 0, (LPARAM)Info);
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;

    switch (uMsg) 
    {
    case WM_COMMAND:
        wmId    = LOWORD(wParam); 
        wmEvent = HIWORD(wParam); 
        // �����˵�ѡ��
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

	case WM_COPYDATA:
        if ( PCOPYDATASTRUCT pInfo = ( PCOPYDATASTRUCT )lParam )
        {
            if ( pInfo->dwData != 1 && dllinterface != NULL )
                dllinterface->ExecuteCmd( ( HWND )wParam, pInfo->dwData, pInfo->lpData, pInfo->cbData );
            else if ( pInfo->dwData == 1 && pInfo->cbData < 250 )
            {
                memcpy( InputBuffer, pInfo->lpData, pInfo->cbData );
                ( ( LPBYTE )pInfo->lpData )[ pInfo->cbData ] = 0;
                ExplainCommand( InputBuffer );
            }
        }
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return TRUE;
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

std::map<CCommand, int> command_map;
//std::hash_map<CCommand, int> command_map; so slow !!!

int InitCommandMap()
{
    command_map["help"] = 1;
    command_map["quit"] = 2;
    command_map["list"] = 3;
    command_map["type"] = 4;
    command_map["c2d"] = 5;
    command_map["fps"] = 6;
    command_map["echo"] = 7;
    command_map["logoff"] = 8;
	command_map["conn2dbc"] = 10;
	command_map["update_allplayer"] = 11;
    command_map["test_param"] = 99;
    command_map["debug"] = 101;
	command_map["loadgmip"]=110;
	command_map["setmaxplayer"]=111;
	command_map["ChangeLoginIP"]=112;
	command_map["tickon"] = 113;
	command_map["tickoff"] = 114;
	command_map["ReLoadDll"] = 115;	
	command_map["saveconfig"] = 116;	

    command_map["reload_configuration"] = 326;
    return 0;
}

int o = InitCommandMap();

bool g_check = false;

DWORD cinsize = 0;
DWORD cinsized = 0;
DWORD sinsize = 0;
DWORD sinsized = 0;
DWORD alla = 0, alld = 0;
DWORD soutsize = 0, soutnumber = 0;

DWORD dwNetRunLimit = 1000;

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
            "list	��ǰ����\r\n"
            "type	��ǰ����\r\n"
            "�÷�������֧�ֶ�̬���ӹ���");
        break;
    case 2: // quit
        GetApp()->m_bQuit = true;
        break;
    case 3: // list
        if (GetStringParameter(szCommand, 256, 2) == NULL)
        {
            // û��ʹ�ø��Ӳ���
            GetApp()->DisplayServerInfo();
        }
        break;
    case 4: // type
/*
        {
            extern CNetPackageRecver *GetTest(WORD i);

            CNetPackageRecver *pnpr = NULL;

            LPCSTR Param = GetStringParameter(szCommand, 256, 2);
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
    case 5: // c2d (connect to database)
        Globals::Reconnect();
        break;
    case 6: // fps
        break;
    case 7:
        echo = !echo;
        break;
    case 8:
        GetApp()->m_PlayerManager.m_bCanLogin = !GetApp()->m_PlayerManager.m_bCanLogin;
        break;
    case 10:
        {
            /*
            LPCSTR _ip = GetStringParameter( szCommand, 256, 2 );
            if ( _ip == NULL )
                break;

            std::string ip = _ip;
            LPCSTR port = GetStringParameter( szCommand, 256, 3 );
            if ( port == NULL )
                break;

            if ( GetApp()->m_dbCenterClient.client.IsConnected() )
                GetApp()->m_dbCenterClient.client.Disconnect();

            if ( GetApp()->m_dbCenterClient.Connect( ip.c_str(), port ) )
            {
                rfalse( 2, 1, "���ӵ��������ݿ�[%s:%s]�ɹ�", ip.c_str(), port );
                rfalse( 1, 1, "���ӵ��������ݿ�[%s:%s]�ɹ�", ip.c_str(), port );
            }
            else
            {
                rfalse( 2, 1, "���������ݿ�[%s:%s]��������ʧ��", ip.c_str(), port );
                rfalse( 1, 1, "���������ݿ�[%s:%s]��������ʧ��", ip.c_str(), port );
            }
            */

            int param = 0;
            if ( LPCSTR szParam = GetStringParameter( szCommand, 256, 2 ) )
                param = atoi( szParam );
		    Globals::LoadDbcList( param );
        }
        break;
    case 11:
        GetApp()->m_PlayerManager.SaveAllPlayer();
        break;
    case 99: // test_param
        break;
    case 101:
        g_check = !g_check;
        break;
	case 110://loadgmip
		{
			if (GetApp()->m_GMManager.LoadGMIP())
			{
				AddInfo("��ȡgm��½ip�����б��ļ��ɹ�!!!\r\n");
			}
			else
			{
				AddInfo("��ȡgm��½ip�����б��ļ�ʧ��!!!\r\n");
			}
		}
		break;
    case 111:
        extern size_t max_player_limit;
        {
            LPCSTR pNumber = GetStringParameter(szCommand, 256, 2);
            if (pNumber != NULL)
                max_player_limit = atoi(pNumber);
        }
        break;
	case 115:
        {
            if ( hndDLLInstance != NULL ) 
            {
                FreeLibrary( hndDLLInstance );	
                dllinterface = NULL;
            }

            LPCSTR dllName = GetStringParameter( szCommand, 256, 2 );
            if ( hndDLLInstance = LoadLibrary( dllName ? dllName : "AttachDll.dll" ) )
            {
                if ( LOADSELF *pfn = ( LOADSELF* )GetProcAddress( hndDLLInstance, "LoadSelf" ) )
                {
                    dllinterface = &pfn();
                    if ( dllinterface->SetObject( GetApp() ) )
                    {
                        rfalse( 2, 1, "%sװ��ɹ�!", dllName ? dllName : "AttachDll.dll" );
                        break;
                    }
                }
            }

            rfalse( 2, 1, "%sװ��ʧ��!", dllName ? dllName : "AttachDll.dll" );
        }
		break;
		//"saveconfig"] = 116;	
	case  116:
		{
			dwt::stringkey<char[MAX_ACCOUNT]>  szaccount = GetStringParameter( szCommand, MAX_ACCOUNT, 2 ) ;
			dwt::stringkey<char[1024]> szconfig = GetStringParameter( szCommand, 1024, 3 ) ;  
			GetApp()->m_PlayerManager.SavePlayerConfig(szaccount,szconfig);
		}
		break;

    case 326:
        Globals::LoadConfigure();
	    GetApp()->m_AccountModuleManage.RegisterALLACC();
        Globals::Reconnect();
        break;
    //case 112:
    //    if (GetApp()->m_AccountServer.IsConnected())
    //    {
    //        GetApp()->m_AccountServer.Disconnect();
    //    }

    //    if (LPCSTR lpIP = GetStringParameter(szCommand, 256, 2))
    //    {
    //        dwt::strcpy(GetApp()->m_szAccountSrvIP, lpIP, 32);
    //    }

    //    if (!InitializeClient(GetApp()->m_AccountServer, GetApp()->m_szAccountSrvIP, 
    //        GetApp()->m_szAccountSrvPort, GetApp()->m_AccountServer.OnDispatch, &GetApp()->m_AccountServer))
    //    {
    //        rfalse(2, 1, "�ʺŷ��������������ϰ�������-____-bb");
    //    }
    //    else
    //    {
    //        rfalse(2, 1, "�ʺŷ��������Ͽ�-____-bb");
    //        SQRegisterLogsrvMsg msg;
    //        msg.IP = (200<<24)|(200<<16)|(200<<8)|107;
    //        extern int serverid;
    //        msg.ID = serverid;
    //        GetApp()->m_AccountServer.SendMessage(&msg, sizeof(SQRegisterLogsrvMsg));
    //    }
    //    break;
	//case 113:
	//	GetApp()->m_bTickStatus = TRUE;
	//	break;
	//case 114:
	//	GetApp()->m_bTickStatus = FALSE;
	//	break;
    default:
        break;
    }
}
