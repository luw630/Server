#ifndef     _DATATRANSFER_H_
#define     _DATATRANSFER_H_

#include "CollecterServer.h"
#include "CollectMessage.h"


// 负责数据的传输，包括网络监听、数据发送、消息处理
class   CDataTransfer
{
    CCollecterServer    *m_pNetServer;  // 网络只是一个功能，而不是特性
public:
    CDataTransfer(void);
    ~CDataTransfer(void);

    // 数据传输的相关操作（准备）
    bool    InitializeNet(char *szPort);
    void    UpdateNetMsg(void);
    void    OnDispatchMsg(DNID dnidClient, LINKSTATUS enumStatus, SMessage *pMsg);
    void    DestroyNet(void);
    BOOL    SendMessage(DNID dnidClient, const void *pvBuf, WORD wSize); 

    DWORD   GetClientNum();

    // 在数据采集中可能会用到的一些虚函数
    virtual void    OnRecvRequestData( int iDataType, DNID dnid );

    // 
};


#endif      //_DATATRANSFER_H_
/**********************
数据传输只关心请求的数据编号，均按照一个可变大小的数据结构来传递


**************************/