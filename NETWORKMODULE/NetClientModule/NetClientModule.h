#pragma once
//#include <Win32.h>
#include <new_net/NetClientEx.h>
#include "NetworkModule\RefreshMsgs.h"

class CNetClientModule
{
private:
	SQRebindMsg  msgRebindTmp;

public:
	CNetClientModule();
	bool	m_bIsKuaFu;		// 是否在跨服中

    int     Initialize(void);
    int     Destroy(void);
    int     Execution(void);	// 这里做基本的套接字操作，数据的接收与发送，相应函数的调用

    // 响应函数
    int     OnDispatch(struct SMessage *Msg); // 这个函数将具体的消息派发到具体的模块中去

    ///////////////////////////
    int     SendToServer(char * pSendMsg , DWORD nCount);
    int     ConnectToServer(char *szIP, char *szPORT);
    void    DisconnectFromServer(void);

    int     RebindToServer(char *name); // 连接重定向到新的服务器

    BOOL    IsConnected() { if (m_pMainLink == NULL) return false; return m_pMainLink->IsConnected(); }
    BOOL    IsWaitingConnect() { if (m_pMainLink == NULL) return false; return m_pMainLink->IsWaitingConnect(); }
    DWORD   GetSrvTime() { if ( m_pMainLink == NULL ) return 0; return m_pMainLink->GetSrvTime(); }

	void	OnConnectComplete();
	void	OnRedirectorConnect();
    void    UpdateInOutSize(DWORD ios[2]);

private:
    CNetClientEx *m_pMainLink;         // 当前连接
    CNetClientEx *m_pTempLink;         // 备用连接
};


extern CNetClientModule *g_pNetClientModule;