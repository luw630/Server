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

    bool    OnDispatch( DNID dnidClient, LINKSTATUS enumStatus, LPVOID data, LPVOID attachment );  // 处理客户端来的消息

public:
    // 这个是区域服务器
    // 和账号无关，有重定向的认证
    int     QuestCheckRebind(DNID dnidClient, struct SQRebindMsg *pMsg);

    DWORD   SendToChatServer(SMessage * pMsg,DWORD dwSize);

	BOOL SendToCenterServer( void* pMsg, WORD iSize );

    void    DisplayServerInfo(char i);

	

    // 消耗的金钱总额
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
    __int64     m_i64TotalSpendMoney;      // 消耗的金钱总额

public:
    HINSTANCE	m_hInst;
    HWND		m_hMainWnd;
    HMENU		m_hMenu;
    char		m_InitPath[MAX_PATH];
    BOOL		m_bQuit;

    DWORD       m_dwIP;
    DWORD       m_dwPort;
	DWORD		m_dwIndex;		//区域服索引
	DWORD       m_dwNew;		//新服
	DWORD		m_dwActivity;	//活动
	DWORD		m_dwRush;		//抢注 //wk 20150613 用作登陆服的端口
	int m_ServerStartTime; //开服时间


    char        m_szName[64];  // 该单元的名称
    static WORD m_wServerID;   // 该单元的ID 之所以改为static是因为服务器退出后会使用这个值！

    SERVICE_STATUS  m_eLoginStatus;

    CNetProviderModuleAdv       m_Server;       // 由于只能建一个服务器，所以内部服务器和外部客户端的连接都使用同一个管理器来操作
    CNetLoginServerModule       m_LoginServer;  // 和登陆服务器相关联的管理器
    //CNetORBServerModule         m_ORBServer;    // 和跨场景服务器相关联的管理器

    CGameWorld                  *m_pGameWorld;    // 游戏世界（管理的几个区域场景）
    //CGameWorld                  &m_GameWorld;    // 游戏世界（管理的几个区域场景）

	CenterModule				m_CenterClient;		//连接中心服的Socket

    DWORD m_dwNormalPlayerNumber;
    DWORD m_dwHangupPlayerNumber;
    DWORD m_dwLinkBreakPlayerNumber;

    // 用于断线重连的数据
    DWORD m_ServerVersion;
	char szLoginIP[32];
	char szLoginPORT[32];
	char szOrbIP[32];
	char szOrbPORT[32];
	//TODO:Tony Modify [2012-3-6]Comment:[加入中心服的IP和Port]
	char m_szCenterIP[32];
	char m_szCenterPORT[32];
	///////////////////////////////////////////////////////////////////
	
};

void    g_AddSpendMoney( __int64 iSpendMoney );
BOOL CreateApp(void);
void ClearApp(void);
CDAppMain *&GetApp(void);
