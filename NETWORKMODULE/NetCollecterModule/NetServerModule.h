#pragma once
#include <new_net/netserverex.h>

typedef unsigned __int64 QWORD;

// pEntry ������ int __thiscall Entry(unsigned __int64 qwDnid, LINKSTATUS enumStatus, void *pMsg); ���͵ĺ���
// ���� pEntry �� int __callback Entry(unsigned __int64 qwDnid, LINKSTATUS enumStatus, void *pMsg); ���͵ĺ���
#define InitializeServer(Server, szPortName, pEntry, pThis) (Server).__init(szPortName, pEntry, pThis) 

//ģ��˵����
//���� NetServer��һ��ģ�飬��CNetModule�̳ж�����������server�˵�����
//Initialize ��Ϊ��ʼ������
class CNetServerModule : 
    public CNetServerEx
{
public:
    // ���̿��ƺ���
    bool    __init(char *, ...);
    int     Execution(void);	            // �������������׽��ֲ��������ݵĽ����뷢�ͣ���Ӧ�����ĵ���
    int     Destroy(void);

    BOOL Create( const char *szPortName );  // ���ػ���create�����������ʵ�֣�����

    // ��Ӧ����
    void    *m_pOnDispatch, *m_pThis;       // __thiscall function ��Ϣ�����ص�

    // ��������
    CNetServerModule(void);
    ~CNetServerModule(void);
}; 

