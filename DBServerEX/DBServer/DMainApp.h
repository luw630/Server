#pragma once

#include "networkmodule/netprovidermodule/netprovidermodule.h"
//#include "区域服务器/区域服务器/CenterModule.h"
#include "netdbmoduleex.h"
#include "../../区域服务器/区域服务器/CenterModule.h"

//测试数据保存的时间
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
	void SaveAll( BOOL bDelete = FALSE );		    // 保存所有玩家的数据
	void PrintTimeLog();    // 答应
    void DisplayInfo();
    void Backup();
	void WriteRole();
    int  LoadBackupSetting();

	void test(string str);
public:
	CDAppMain(void);
	~CDAppMain(void);

public:
	HANDLE quitEvent;   // 退出事件

	CNetProviderModule  m_NetProviderModule;

	// 处理网络消息的线程（非角色操作的消息部分，角色操作的部分已经通过dpl直接进入内部消息队列了！）
	UGE::mtQueue< std::pair< DNID, void* > > msgQueue;

	// 网络过来的RPC消息队列< 分配地址， 序列化数据地址 >
	UGE::mtQueue< std::pair< DNID, void* > > rpcQueue;

    // 专用于RPC操作的对象
	// CNetDBModuleEx m_RPCDB;
    BOOL atBackup;
    std::map< DWORD, DWORD > bakTicks; // 用于检测备份时间点的容器

	CenterModule				m_CenterClient;		//连接中心服的Socket
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
BOOL CreateApp(void);
void ClearApp(void);
CDAppMain &GetApp(void);