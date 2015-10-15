#pragma once

#include "networkmodule\netconsumermodule\netconsumermodule.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class DBCenterBaseModule
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
    DBCenterBaseModule( DWORD dbcId );
    ~DBCenterBaseModule();

public:
    CNetConsumerModule client;

    std::string currentIP;      // ���������ӵ�����
    std::string currentPort;    // ͬ��

    DWORD dbcId;                // ���ڱ�����ǰ�������ݿ�ı�ţ�Ĭ�ϵ���ʼ���Ϊ100
};

