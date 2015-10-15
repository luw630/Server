#pragma once
#include <new_net/netserverex.h>

typedef unsigned __int64 QWORD;

// pEntry 必须是 int __thiscall Entry(unsigned __int64 qwDnid, LINKSTATUS enumStatus, void *pMsg); 类型的函数
// 或者 pEntry 是 int __callback Entry(unsigned __int64 qwDnid, LINKSTATUS enumStatus, void *pMsg); 类型的函数
#define InitializeServer(Server, szPortName, pEntry, pThis) (Server).__init(szPortName, pEntry, pThis) 

//模块说明：
//基于 NetServer的一个模块，由CNetModule继承而来，负责处理server端的数据
//Initialize 作为初始化函数
class CNetServerModule : 
    public CNetServerEx
{
public:
    // 流程控制函数
    bool    __init(char *, ...);
    int     Execution(void);	            // 这里做基本的套接字操作，数据的接收与发送，相应函数的调用
    int     Destroy(void);

    BOOL Create( const char *szPortName );  // 重载基类create，里边有特殊实现！！！

    // 响应函数
    void    *m_pOnDispatch, *m_pThis;       // __thiscall function 消息解析回调

    // 构造析构
    CNetServerModule(void);
    ~CNetServerModule(void);
}; 

