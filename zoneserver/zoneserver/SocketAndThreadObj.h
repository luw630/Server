#pragma once
#include <WinSock2.h>
#pragma comment(lib,"WS2_32")
#define SERVERPORT 10000
//�׽��ֶ���
typedef struct _SOCKET_OBJ 
{
	SOCKET m_socket;
	HANDLE m_event;
	SOCKADDR_IN m_clientaddr;
	_SOCKET_OBJ *m_lpnext;
}SOCKET_OBJ, *LPSOCKET_OBJ;

//�̳߳ض���
typedef struct _THREAD_OBJ
{
	HANDLE m_eventlist[WSA_MAXIMUM_WAIT_EVENTS];
	unsigned m_cursocketcount;
	LPSOCKET_OBJ m_lpsockheader;
	LPSOCKET_OBJ m_lpsocktail;
	CRITICAL_SECTION m_cs;
	_THREAD_OBJ *m_lpnext;
}THREAD_OBJ, *LPTHREAD_OBJ;

//���ݶ���
typedef struct _DATABUF_OBJ
{
	LPSOCKET_OBJ m_lpSocketObj;
	int m_DataSize;		//���յ����ݴ�С
	BYTE *m_lpData;		//��������
	_DATABUF_OBJ *m_lpNext;
}DATABUF_OBJ, *LPDATABUF_OBJ;

extern LPDATABUF_OBJ g_HeadDataBuf;
extern LPDATABUF_OBJ g_TailDataBuf;
extern CRITICAL_SECTION g_DataCs;	//���ݶ��󻥳⣨�������ݽ��պ����ݴ����⣩

extern LPTHREAD_OBJ LPThreadList;
extern CRITICAL_SECTION g_cs;
extern LONG g_nCurrentConnections; // ��ǰ��������

extern LPDATABUF_OBJ CreateDataBufObj(LPSOCKET_OBJ lpSocketObj, BYTE *lpData, int DataSize);	//����һ�����ݶ���

extern void ReleaseHeadDataBufObj();	//�ͷű�ͷ���ݶ���


extern LPSOCKET_OBJ CreateSocketObj(SOCKET s);		//�����׽��ֶ���

extern void FreeSocketObj(LPSOCKET_OBJ lpSockObj);		//�ͷ��׽��ֶ���

extern LPTHREAD_OBJ CreateThreadObj();		//����һ���µ��̶߳���

extern void FreeThreadObj(LPTHREAD_OBJ lpThreadObj);	//�ͷ��̶߳���

extern void ReBuildEvents(LPTHREAD_OBJ lpThreadObj);	//�ؽ��߳��¼�����

extern bool SockObjInsertThreadObj(LPTHREAD_OBJ pThreadObj, LPSOCKET_OBJ pSockObj);		//��һ���׽��ֲ���һ���̶߳�����

extern void AssignToFreeThread(LPSOCKET_OBJ pSockObj);		//��һ���׽��ֶ����Ÿ����е��̴߳���

extern void RemoveSockObjFromThreadObj(LPTHREAD_OBJ pThreadObj, LPSOCKET_OBJ pSockObj);		//�Ӹ����̵߳��׽��ֶ����б����Ƴ�һ���׽��ֶ���

typedef void(*LPHandleMessageFunc)(LPSOCKET_OBJ lpSocketObj, BYTE *lpData, int DataSize);

extern void HandleDataBuf(LPHandleMessageFunc DispatchMessage);



