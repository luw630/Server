#pragma once

#include "networkmodule\netconsumermodule\netconsumermodule.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class DBCenterBaseModule
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
    DBCenterBaseModule( DWORD dbcId );
    ~DBCenterBaseModule();

public:
    CNetConsumerModule client;

    std::string currentIP;      // 用于重连接的数据
    std::string currentPort;    // 同上

    DWORD dbcId;                // 用于标明当前中央数据库的编号，默认的起始编号为100
};

