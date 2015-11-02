#include "stdafx.h"
#include "SocketAndThreadObj.h"
#include <stdlib.h>
#include <WinSock2.h>
#pragma comment(lib,"WS2_32")


extern void AddInfo(LPCSTR Info);

LPDATABUF_OBJ g_HeadDataBuf = NULL;
LPDATABUF_OBJ g_TailDataBuf = NULL;
CRITICAL_SECTION g_DataCs;	//数据对象互斥（用于数据接收和数据处理互斥）

LPTHREAD_OBJ LPThreadList = NULL;
CRITICAL_SECTION g_cs;
LONG g_nCurrentConnections = 0; // 当前连接数量
static UINT WINAPI ThreadProc(LPVOID lParma);



LPDATABUF_OBJ CreateDataBufObj(LPSOCKET_OBJ lpSocketObj, BYTE *lpData, int DataSize)	//分配一个数据对象
{
	LPDATABUF_OBJ lpDataBuf = new DATABUF_OBJ();
	if (lpDataBuf != NULL)
	{
		lpDataBuf->m_lpSocketObj = lpSocketObj;
		lpDataBuf->m_DataSize = DataSize;
		lpDataBuf->m_lpData = new BYTE[DataSize + 1];
		memcpy(lpDataBuf->m_lpData, lpData, DataSize);
		lpDataBuf->m_lpData[DataSize] = 0;
		lpDataBuf->m_lpNext = NULL;
		EnterCriticalSection(&g_DataCs);
		if (g_HeadDataBuf == NULL)
		{
			g_HeadDataBuf = lpDataBuf;
			g_TailDataBuf = lpDataBuf;
		}
		else
		{
			g_TailDataBuf->m_lpNext = lpDataBuf;
		}
		LeaveCriticalSection(&g_DataCs);
	}
	return lpDataBuf;
}

void ReleaseHeadDataBufObj()	//释放表头数据对象
{
	LPDATABUF_OBJ lpDataBuf = g_HeadDataBuf;
	if (lpDataBuf != NULL)
	{
		EnterCriticalSection(&g_DataCs);
		g_HeadDataBuf = g_HeadDataBuf->m_lpNext;
		if (g_HeadDataBuf == NULL)
			g_TailDataBuf = NULL;
		if (lpDataBuf->m_lpData != NULL)
		{
			delete[] lpDataBuf->m_lpData;
			lpDataBuf->m_lpData = NULL;
		}
		LeaveCriticalSection(&g_DataCs);

		delete lpDataBuf;
		lpDataBuf = NULL;
	}
}

LPSOCKET_OBJ CreateSocketObj(SOCKET s)		//分配套接字对象
{
	LPSOCKET_OBJ LPSockObj = (LPSOCKET_OBJ)malloc(sizeof(SOCKET_OBJ));
	if(LPSockObj != NULL)
	{
		LPSockObj->m_socket = s;
		LPSockObj->m_event = WSACreateEvent();
		LPSockObj->m_lpnext = NULL;
	}
	return LPSockObj;
}

void FreeSocketObj(LPSOCKET_OBJ lpSockObj)	//释放套接字对象
{
	CloseHandle(lpSockObj->m_event);
	if(lpSockObj->m_socket != INVALID_SOCKET)
		closesocket(lpSockObj->m_socket);
	free(lpSockObj);
}

LPTHREAD_OBJ CreateThreadObj()				//分配一个新的线程对象
{
	LPTHREAD_OBJ lpThreadObj = (LPTHREAD_OBJ)malloc(sizeof(THREAD_OBJ));
	if(lpThreadObj != NULL)
	{
		::InitializeCriticalSection(&lpThreadObj->m_cs);
		lpThreadObj->m_eventlist[0] = WSACreateEvent();
		lpThreadObj->m_cursocketcount = 0;
		lpThreadObj->m_lpsockheader = NULL;
		lpThreadObj->m_lpsocktail = NULL;
		::EnterCriticalSection(&g_cs);
		lpThreadObj->m_lpnext = LPThreadList;
		LPThreadList = lpThreadObj;
		::LeaveCriticalSection(&g_cs);
	}
	return lpThreadObj;
}

void FreeThreadObj(LPTHREAD_OBJ lpThreadObj)//释放线程对象
{
	::EnterCriticalSection(&g_cs);
	LPTHREAD_OBJ lp = LPThreadList, lppre = NULL;
	while(lp != NULL)
	{
		if(lp == lpThreadObj)
			break;
		lppre = lp;
		lp = lp->m_lpnext;
	}
	if(lp == LPThreadList)
		LPThreadList = lp->m_lpnext;
	else if(lp->m_lpnext != NULL)
		lppre->m_lpnext = lp->m_lpnext;
	else
		lppre->m_lpnext = NULL;
	::LeaveCriticalSection(&g_cs);
	::CloseHandle(lpThreadObj->m_eventlist[0]);
	::DeleteCriticalSection(&lpThreadObj->m_cs);
	free(lpThreadObj);
}

void ReBuildEvents(LPTHREAD_OBJ lpThreadObj)//重建线程事件数组
{
	EnterCriticalSection(&lpThreadObj->m_cs);
	LPSOCKET_OBJ pSockObj = lpThreadObj->m_lpsockheader;
	int Index = 1;
	while (pSockObj != NULL)
	{
		lpThreadObj->m_eventlist[Index++] = pSockObj->m_event;
		pSockObj = pSockObj->m_lpnext;
	}
	LeaveCriticalSection(&lpThreadObj->m_cs);
}

bool SockObjInsertThreadObj(LPTHREAD_OBJ pThreadObj, LPSOCKET_OBJ pSockObj)		//将一个套接字插入一个线程对象中
{
	bool bRet = false;
	EnterCriticalSection(&pThreadObj->m_cs);
	if (pThreadObj->m_cursocketcount < WSA_MAXIMUM_WAIT_EVENTS - 1)
	{
		if (pThreadObj->m_lpsockheader == NULL)
		{
			pThreadObj->m_lpsockheader = pThreadObj->m_lpsocktail = pSockObj;
		}
		else
		{
			pThreadObj->m_lpsocktail->m_lpnext = pSockObj;
			pThreadObj->m_lpsocktail = pSockObj;
		}
		pThreadObj->m_cursocketcount++;
		pThreadObj->m_eventlist[pThreadObj->m_cursocketcount] = pSockObj->m_event;
		bRet = true;
	}
	LeaveCriticalSection(&pThreadObj->m_cs);
	if (bRet)
	{
		InterlockedIncrement(&g_nCurrentConnections);
	}
	return bRet;
}

void AssignToFreeThread(LPSOCKET_OBJ pSockObj)		//将一个套接字对象安排给空闲的线程处理
{
	pSockObj->m_lpnext = NULL;
	LPTHREAD_OBJ pThreadObj = LPThreadList;
	bool bRet = false;
	while(pThreadObj != NULL)
	{
		bRet = SockObjInsertThreadObj(pThreadObj, pSockObj);
		if (bRet)
			break;
		pThreadObj = pThreadObj->m_lpnext;
	}
	// 没有空闲线程，为这个套接字创建新的线程 
	if (!bRet)
	{
		pThreadObj = CreateThreadObj();
		bRet = SockObjInsertThreadObj(pThreadObj, pSockObj);
		//创建线程
		::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, pThreadObj, 0, NULL);
	}
	WSASetEvent(pThreadObj->m_eventlist[0]);
}

void RemoveSockObjFromThreadObj(LPTHREAD_OBJ pThreadObj, LPSOCKET_OBJ pSockObj)		//从给定线程的套接字对象列表中移除一个套接字对象
{
	LPSOCKET_OBJ pCurObj = pThreadObj->m_lpsockheader, pPreObj = NULL;
	while(pCurObj != NULL)
	{
		if (pCurObj == pSockObj)
			break;
		pPreObj = pCurObj;
		pCurObj = pCurObj->m_lpnext;
	}
	EnterCriticalSection(&pThreadObj->m_cs);
	if (pPreObj == NULL)	//查找的套接字在列表头
	{
		if(pThreadObj->m_lpsockheader == pThreadObj->m_lpsocktail)
			pThreadObj->m_lpsockheader = pThreadObj->m_lpsocktail = NULL;
		else
			pThreadObj->m_lpsockheader = pCurObj->m_lpnext;
	}
	else
	{
		if (pCurObj == pThreadObj->m_lpsocktail)	//查找的套接字在列表尾
			pThreadObj->m_lpsocktail = pPreObj;
		pPreObj->m_lpnext = pCurObj->m_lpnext;
	}
	pThreadObj->m_cursocketcount--;
	WSASetEvent(pThreadObj->m_eventlist[0]);
	LeaveCriticalSection(&pThreadObj->m_cs);
	InterlockedDecrement(&g_nCurrentConnections);
	char sAct[64];
	sprintf(sAct, "IP地址(%s)断开了与区域服的GM服务连接\n", inet_ntoa(pSockObj->m_clientaddr.sin_addr));
	AddInfo(sAct);
}

static LPSOCKET_OBJ FindSockObj(LPTHREAD_OBJ pThreadObj, int EventIndex)		//通过事件ID(从1开始)在当前线程中查找对应的套接字
{
	LPSOCKET_OBJ pSockObj = pThreadObj->m_lpsockheader;
	while(--EventIndex)
	{
		if(pSockObj == NULL)
			break;
		pSockObj = pSockObj->m_lpnext;
	}
	return pSockObj;
}

static void HandleIO(LPTHREAD_OBJ pThreadObj, LPSOCKET_OBJ pSockObj)
{
	char RecvData[1025];
	memset(RecvData, 0, 1025);
	WSANETWORKEVENTS NetEvent;
	WSAEnumNetworkEvents(pSockObj->m_socket, pSockObj->m_event, &NetEvent);
	if (NetEvent.lNetworkEvents & FD_READ)
	{
		if (NetEvent.iErrorCode[FD_READ_BIT] == 0)
		{
			//接受数据
			int nRet = recv(pSockObj->m_socket, RecvData, 1025, 0);
			if (nRet != 0)
			{
				//RecvData[nRet] = '\n';
				//printf("从主机(%s)接收到的数据为：%s\n",inet_ntoa(pSockObj->m_clientaddr.sin_addr), RecvData);
				//memset(RecvData, 97, 1000);

				////回显数据
				//bool bflag = true;
				//int nRet = send(pSockObj->m_socket, RecvData, strlen(RecvData), 0);
				//if (nRet == SOCKET_ERROR)
				//{
				//	bflag = false;
				//}
				//printf("回显数据到主机(%s),成功为1、失败为0:%d\n", inet_ntoa(pSockObj->m_clientaddr.sin_addr), bflag);
				CreateDataBufObj(pSockObj, (BYTE*)RecvData, nRet);	//分配一个数据对象
				return;
			}
		}
		else	//出错（可能是：套接字断线，重启等）
		{

		}
	}
	else if (NetEvent.lNetworkEvents & FD_WRITE)	
	{
		if(NetEvent.iErrorCode[FD_WRITE_BIT] == 0)	//发送数据
		{
			return;
		}
		else	//出错（可能是：套接字断线，重启等）
		{

		}
	}
	else if (NetEvent.lNetworkEvents & FD_CLOSE)
	{
	}
	// 套接字关闭，或者有错误发生，程序都会转到这里来执行
	RemoveSockObjFromThreadObj(pThreadObj, pSockObj);
	ReBuildEvents(pThreadObj);
	FreeSocketObj(pSockObj);
}

static UINT WINAPI ThreadProc(LPVOID lParma)
{
	// 取得本线程对象的指针
	LPTHREAD_OBJ pThreadObj = (LPTHREAD_OBJ)lParma;
	while(true)
	{
		unsigned int Index = WSAWaitForMultipleEvents(pThreadObj->m_cursocketcount+1, pThreadObj->m_eventlist, false, WSA_INFINITE, false);
		Index = Index - WSA_WAIT_EVENT_0;
		for (unsigned int i = Index; i <= pThreadObj->m_cursocketcount; i++)
		{
			int iRet = WSAWaitForMultipleEvents(1, &pThreadObj->m_eventlist[i], TRUE, 1000, FALSE);
			if (iRet == WAIT_FAILED || iRet == WAIT_TIMEOUT)
			{
				continue;
			}
			else
			{
				if (i == 0)		//重建事件数组
				{
					if (pThreadObj->m_cursocketcount == 0)
					{
						FreeThreadObj(pThreadObj);
						return 0;	//线程结束
					}
					ReBuildEvents(pThreadObj);
					ResetEvent(pThreadObj->m_eventlist[i]);
				}
				else	//处理网路事件
				{
					LPSOCKET_OBJ pSockObj = FindSockObj(pThreadObj, i);
					if (pSockObj != NULL)
					{
						HandleIO(pThreadObj, pSockObj);
					}
					else
						;//printf("没有找到有效套接字对象\n ");
				}
			}
		}
	}
	return 0;
}

void HandleDataBuf(LPHandleMessageFunc DispatchMessage)
{
	LPDATABUF_OBJ lpDataBuf = g_HeadDataBuf;
	while (lpDataBuf != NULL)
	{
		(*DispatchMessage)(lpDataBuf->m_lpSocketObj, lpDataBuf->m_lpData, lpDataBuf->m_DataSize);
		lpDataBuf = lpDataBuf->m_lpNext;
		ReleaseHeadDataBufObj();
	}
}
