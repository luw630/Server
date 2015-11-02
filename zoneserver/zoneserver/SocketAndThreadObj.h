#pragma once
#include <WinSock2.h>
#pragma comment(lib,"WS2_32")
#define SERVERPORT 10000
//套接字对象
typedef struct _SOCKET_OBJ 
{
	SOCKET m_socket;
	HANDLE m_event;
	SOCKADDR_IN m_clientaddr;
	_SOCKET_OBJ *m_lpnext;
}SOCKET_OBJ, *LPSOCKET_OBJ;

//线程池对象
typedef struct _THREAD_OBJ
{
	HANDLE m_eventlist[WSA_MAXIMUM_WAIT_EVENTS];
	unsigned m_cursocketcount;
	LPSOCKET_OBJ m_lpsockheader;
	LPSOCKET_OBJ m_lpsocktail;
	CRITICAL_SECTION m_cs;
	_THREAD_OBJ *m_lpnext;
}THREAD_OBJ, *LPTHREAD_OBJ;

//数据对象
typedef struct _DATABUF_OBJ
{
	LPSOCKET_OBJ m_lpSocketObj;
	int m_DataSize;		//接收的数据大小
	BYTE *m_lpData;		//数据内容
	_DATABUF_OBJ *m_lpNext;
}DATABUF_OBJ, *LPDATABUF_OBJ;

extern LPDATABUF_OBJ g_HeadDataBuf;
extern LPDATABUF_OBJ g_TailDataBuf;
extern CRITICAL_SECTION g_DataCs;	//数据对象互斥（用于数据接收和数据处理互斥）

extern LPTHREAD_OBJ LPThreadList;
extern CRITICAL_SECTION g_cs;
extern LONG g_nCurrentConnections; // 当前连接数量

extern LPDATABUF_OBJ CreateDataBufObj(LPSOCKET_OBJ lpSocketObj, BYTE *lpData, int DataSize);	//分配一个数据对象

extern void ReleaseHeadDataBufObj();	//释放表头数据对象


extern LPSOCKET_OBJ CreateSocketObj(SOCKET s);		//分配套接字对象

extern void FreeSocketObj(LPSOCKET_OBJ lpSockObj);		//释放套接字对象

extern LPTHREAD_OBJ CreateThreadObj();		//分配一个新的线程对象

extern void FreeThreadObj(LPTHREAD_OBJ lpThreadObj);	//释放线程对象

extern void ReBuildEvents(LPTHREAD_OBJ lpThreadObj);	//重建线程事件数组

extern bool SockObjInsertThreadObj(LPTHREAD_OBJ pThreadObj, LPSOCKET_OBJ pSockObj);		//将一个套接字插入一个线程对象中

extern void AssignToFreeThread(LPSOCKET_OBJ pSockObj);		//将一个套接字对象安排给空闲的线程处理

extern void RemoveSockObjFromThreadObj(LPTHREAD_OBJ pThreadObj, LPSOCKET_OBJ pSockObj);		//从给定线程的套接字对象列表中移除一个套接字对象

typedef void(*LPHandleMessageFunc)(LPSOCKET_OBJ lpSocketObj, BYTE *lpData, int DataSize);

extern void HandleDataBuf(LPHandleMessageFunc DispatchMessage);



