#pragma once

#include "networkmodule/netprovidermodule/netprovidermodule.h"
//#include "���������/���������/CenterModule.h"
#include "netdbmoduleex.h"
#include "../../���������/���������/CenterModule.h"

//�������ݱ����ʱ��
#define  _DEBUG_SAVE_TIME 1

class CDAppMain 
{
public:
	BOOL		InitServer();
	BOOL		LoopServer();
	BOOL		ExitServer();
	BOOL		InitWindow(HINSTANCE hInstance);

public:
	bool Entry( unsigned __int64 qwDnid, LINKSTATUS enumStatus, void *pMsg, size_t size );
	void SaveAll( BOOL bDelete = FALSE );		    // ����������ҵ�����
	void PrintTimeLog();    // ��Ӧ
    void DisplayInfo();
    void Backup();
	void WriteRole();
    int  LoadBackupSetting();

	void test(string str);
public:
	CDAppMain(void);
	~CDAppMain(void);

public:
	HANDLE quitEvent;   // �˳��¼�

	CNetProviderModule  m_NetProviderModule;

	// ����������Ϣ���̣߳��ǽ�ɫ��������Ϣ���֣���ɫ�����Ĳ����Ѿ�ͨ��dplֱ�ӽ����ڲ���Ϣ�����ˣ���
	UGE::mtQueue< std::pair< DNID, void* > > msgQueue;

	// ���������RPC��Ϣ����< �����ַ�� ���л����ݵ�ַ >
	UGE::mtQueue< std::pair< DNID, void* > > rpcQueue;

    // ר����RPC�����Ķ���
	// CNetDBModuleEx m_RPCDB;
    BOOL atBackup;
    std::map< DWORD, DWORD > bakTicks; // ���ڼ�ⱸ��ʱ��������

	CenterModule				m_CenterClient;		//�������ķ���Socket
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
BOOL CreateApp(void);
void ClearApp(void);
CDAppMain &GetApp(void);