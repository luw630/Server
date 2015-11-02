#pragma once

#include "netloginservermodule.h"
#include "netorbservermodule.h"
#include "GameObjects\GameWorld.h"
#include "CenterModule.h"

class CDAppMain
{
public:
    BOOL	InitServer();
    BOOL	LoopServer();
    BOOL	ExitServer();

    BOOL	InitWindow(HINSTANCE hInstance);
    LRESULT	MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    bool    OnDispatch( DNID dnidClient, LINKSTATUS enumStatus, LPVOID data, LPVOID attachment );  // ����ͻ���������Ϣ

public:
    // ��������������
    // ���˺��޹أ����ض������֤
    int     QuestCheckRebind(DNID dnidClient, struct SQRebindMsg *pMsg);

    DWORD   SendToChatServer(SMessage * pMsg,DWORD dwSize);

	BOOL SendToCenterServer( void* pMsg, WORD iSize );

    void    DisplayServerInfo(char i);

	

    // ���ĵĽ�Ǯ�ܶ�
    void    AddSpendMoney( __int64 iSpendMoney )
    {
        m_i64TotalSpendMoney += iSpendMoney;
    };
    __int64   GetSpendMoney(void)
    {
        return  m_i64TotalSpendMoney;
    }
public:
    CDAppMain(void);
    ~CDAppMain(void);

public:
    enum SERVICE_STATUS { SS_NULL, SS_LOGIN, SS_INIT, SS_SUCCESS, SS_BREAK=0xff };

private:
    __int64     m_i64TotalSpendMoney;      // ���ĵĽ�Ǯ�ܶ�

public:
    HINSTANCE	m_hInst;
    HWND		m_hMainWnd;
    HMENU		m_hMenu;
    char		m_InitPath[MAX_PATH];
    BOOL		m_bQuit;

    DWORD       m_dwIP;
    DWORD       m_dwPort;
	DWORD		m_dwIndex;		//���������
	DWORD       m_dwNew;		//�·�
	DWORD		m_dwActivity;	//�
	DWORD		m_dwRush;		//��ע //wk 20150613 ������½���Ķ˿�
	int m_ServerStartTime; //����ʱ��


    char        m_szName[64];  // �õ�Ԫ������
    static WORD m_wServerID;   // �õ�Ԫ��ID ֮���Ը�Ϊstatic����Ϊ�������˳����ʹ�����ֵ��

    SERVICE_STATUS  m_eLoginStatus;

    CNetProviderModuleAdv       m_Server;       // ����ֻ�ܽ�һ���������������ڲ����������ⲿ�ͻ��˵����Ӷ�ʹ��ͬһ��������������
    CNetLoginServerModule       m_LoginServer;  // �͵�½������������Ĺ�����
    //CNetORBServerModule         m_ORBServer;    // �Ϳ糡��������������Ĺ�����

    CGameWorld                  *m_pGameWorld;    // ��Ϸ���磨����ļ������򳡾���
    //CGameWorld                  &m_GameWorld;    // ��Ϸ���磨����ļ������򳡾���

	CenterModule				m_CenterClient;		//�������ķ���Socket

    DWORD m_dwNormalPlayerNumber;
    DWORD m_dwHangupPlayerNumber;
    DWORD m_dwLinkBreakPlayerNumber;

    // ���ڶ�������������
    DWORD m_ServerVersion;
	char szLoginIP[32];
	char szLoginPORT[32];
	char szOrbIP[32];
	char szOrbPORT[32];
	//TODO:Tony Modify [2012-3-6]Comment:[�������ķ���IP��Port]
	char m_szCenterIP[32];
	char m_szCenterPORT[32];
	///////////////////////////////////////////////////////////////////
	
};

void    g_AddSpendMoney( __int64 iSpendMoney );
BOOL CreateApp(void);
void ClearApp(void);
CDAppMain *&GetApp(void);
