#pragma once
#include "networkmodule\netconsumermodule\netconsumermodule.h"
#include "networkmodule\refreshmsgs.h"

class CNetLoginServerModule :
    public CNetConsumerModule
{
public:
    int Execution(void);	            // 这里做基本的套接字操作，数据的接收与发送，相应函数的调用

    BOOL SendMsgToLoginSrv(struct SMessage *pMsg, int iSize);

    bool OnDispatch( void *data, size_t size );

public:
    void RecvRefreshMsg(SRefreshBaseMsg *pMsg);
    void RecvCtrlMsg(SServerCtrlMsg *pMsg  );
    void RecvCheckRebind(SACheckRebindMsg *pMsg);
    void RecvLoginMessage(struct SALoginMsg *pMsg);
    void RecvPointModifyMsg(struct SPointModifyMsg *pMsg);
	void ProcessInterop( void *data, DWORD dbtype );

public:
    CNetLoginServerModule();
    ~CNetLoginServerModule();
};
