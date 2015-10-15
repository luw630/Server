#pragma once

#include "networkmodule\netconsumermodule\netconsumermodule.h"
#define DEFAULT_ACCOUNT_PREFIX "default"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CNetAccountCheckModule
{
public:
    // �������������׽��ֲ��������ݵĽ����뷢�ͣ���Ӧ�����ĵ���
    int Execution(void);
    void Destroy();

    bool Connect( LPCSTR ip, LPCSTR port );
    bool Entry( LPCVOID data, size_t size );

public:
    void RecvCheckAccountMsg( struct SACheckAccountMsg *pMsg );

    void RecvChangePasswordMsg( struct SAChangePasswordMsg *pMsg );
    void RecvBlockAccountMsg( struct SABAMsg *pMsg );
    void RecvGetOnlineTimeMsg( struct SAGOTMsg *pMsg );

    // void RecvGMLoginMsg( struct SAGMCheckAccountMsg *pMsg );

    void RecvRefreshAccountMsg( struct SARefreshCardMsg *pMsg );
	void RecvRefreshMoneyPointMsg( struct SALRefreshMoneyPointMsg *pMsg );
    void RecvQueryEncouragementMsg( struct SAQueryEncouragement *pMsg );
	void RecvCheckPlayerExistMsg( struct SQCheckPlayerExistBaseMsg *pMsg );

    void ProcessInterop( void *data );
	BOOL Reconnect();

public:
    CNetAccountCheckModule();
	CNetAccountCheckModule( LPCSTR prefix ) : accountPrefix( prefix ){  dwPrev = timeGetTime(); };
    ~CNetAccountCheckModule();

public:
    CNetConsumerModule client;
	std::string accountPrefix;  // ��ǰ�ʺ�ģ���Ӧ�ķ�����ǰ׺���������ֵΪempty����˵����Ĭ�ϵ��ʺŷ�����

private:
    std::string accountServerIP;
    std::string accountServerPort;
	DWORD dwPrev;

public:
    // �Ŷ�ϵͳ��������ط���ʹ�����оͷ���������
    // ����һ���ṹ��ԼΪ500 ���ڼٶ��Ŷ����Ϊ 2000 * 500��1M ��ôռ�� 1M ���ݿռ�
    struct SQueueData
    {
        std::string account;
        std::string password;
        std::string userpass;
        std::string idkey;
        DWORD online;
        DWORD offline;
        DWORD limitedState;
        DNID dnidClient;
        QWORD puid;
    };

    // ��½����
    std::list< SQueueData > loginQueueList;

    // �����Ŷ����
    void UpdateQueuePlayer();
	void UpdateLoginPlayer(); //����������������д�������
	void PopLoginPlayer(std::string strcount);
// 	//��ҵ�¼ʱ�ڵ�¼���������棬�����������������ɺ���ɾ�����Լ�������������ĵ�¼ѹ��
// 	std::vector<std::string> playerloginlist;
};

