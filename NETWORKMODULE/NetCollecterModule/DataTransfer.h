#ifndef     _DATATRANSFER_H_
#define     _DATATRANSFER_H_

#include "CollecterServer.h"
#include "CollectMessage.h"


// �������ݵĴ��䣬����������������ݷ��͡���Ϣ����
class   CDataTransfer
{
    CCollecterServer    *m_pNetServer;  // ����ֻ��һ�����ܣ�����������
public:
    CDataTransfer(void);
    ~CDataTransfer(void);

    // ���ݴ������ز�����׼����
    bool    InitializeNet(char *szPort);
    void    UpdateNetMsg(void);
    void    OnDispatchMsg(DNID dnidClient, LINKSTATUS enumStatus, SMessage *pMsg);
    void    DestroyNet(void);
    BOOL    SendMessage(DNID dnidClient, const void *pvBuf, WORD wSize); 

    DWORD   GetClientNum();

    // �����ݲɼ��п��ܻ��õ���һЩ�麯��
    virtual void    OnRecvRequestData( int iDataType, DNID dnid );

    // 
};


#endif      //_DATATRANSFER_H_
/**********************
���ݴ���ֻ������������ݱ�ţ�������һ���ɱ��С�����ݽṹ������


**************************/