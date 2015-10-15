#ifndef     _COLLECTERSERVER_H_
#define     _COLLECTERSERVER_H_


#include "NetServerModule.h"

// 为数据传输提供网络支持
class   CCollecterServer    :   public  CNetServerModule
{
public:
    CCollecterServer(void);
    ~CCollecterServer(void);
};

#endif      //_COLLECTERSERVER_H_