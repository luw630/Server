#pragma once

#include "networkmodule\netconsumermodule\netconsumermodule.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class DBCenterModule
{
public:
    // 这里做基本的套接字操作，数据的接收与发送，相应函数的调用
    int Execution(void);
    void Destroy();

    bool Connect( LPCSTR ip, LPCSTR port );
    bool Entry( LPCVOID data, size_t size );

public:
    void ProcessInterop( void *data );

public:
    DBCenterModule();
    ~DBCenterModule();

public:
    CNetConsumerModule client;

    std::string currentIP;
    std::string currentPort;
};

