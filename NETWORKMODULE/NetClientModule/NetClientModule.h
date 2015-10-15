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
	bool	m_bIsKuaFu;		// �Ƿ��ڿ����

    int     Initialize(void);
    int     Destroy(void);
    int     Execution(void);	// �������������׽��ֲ��������ݵĽ����뷢�ͣ���Ӧ�����ĵ���

    // ��Ӧ����
    int     OnDispatch(struct SMessage *Msg); // ����������������Ϣ�ɷ��������ģ����ȥ

    ///////////////////////////
    int     SendToServer(char * pSendMsg , DWORD nCount);
    int     ConnectToServer(char *szIP, char *szPORT);
    void    DisconnectFromServer(void);

    int     RebindToServer(char *name); // �����ض����µķ�����

    BOOL    IsConnected() { if (m_pMainLink == NULL) return false; return m_pMainLink->IsConnected(); }
    BOOL    IsWaitingConnect() { if (m_pMainLink == NULL) return false; return m_pMainLink->IsWaitingConnect(); }
    DWORD   GetSrvTime() { if ( m_pMainLink == NULL ) return 0; return m_pMainLink->GetSrvTime(); }

	void	OnConnectComplete();
	void	OnRedirectorConnect();
    void    UpdateInOutSize(DWORD ios[2]);

private:
    CNetClientEx *m_pMainLink;         // ��ǰ����
    CNetClientEx *m_pTempLink;         // ��������
};


extern CNetClientModule *g_pNetClientModule;