#pragma once

#include "networkmodule\netconsumermodule\netconsumermodule.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class DBCenterModule
{
public:
    // �������������׽��ֲ��������ݵĽ����뷢�ͣ���Ӧ�����ĵ���
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

