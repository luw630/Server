#pragma once
#include "stdafx.h"
#include <time.h>
#include "DMainApp.h"
#include "SocketAndThreadObj.h"
#include "NETWORKMODULE\PlayerBaseInf.h"


int InitGMServer();

void LoopGMServer();

void ReleaseGMServer();

void OnDispatchMessage(LPSOCKET_OBJ lpSocketObj, BYTE *lpData, int DataSize);

void OnDispatchPlayerCfgMsg(LPSOCKET_OBJ lpSocketObj, BaseMessage *pMsg);

void OnDispatchSyscallMsg(LPSOCKET_OBJ lpSocketObj, BaseMessage *pMsg);

void OnDispatchLoginMsg(LPSOCKET_OBJ lpSocketObj, BaseMessage *pMsg);

void OnHandleFindPlayerMsg(LPSOCKET_OBJ lpSocketObj, SQPlayerBaseInf *pMsg);

void OnHandleSavePlayerInf(LPSOCKET_OBJ lpSocketObj, SQSavePlayerInf *pMsg);

void OnHandleTransfer(LPSOCKET_OBJ lpSocketObj, SQTransfer *pMsg);

void OnHandleAddItem(LPSOCKET_OBJ lpSocketObj, SQAddItem *pMsg);

void HandleLoginMsg(void* pSocketObj, SQLoginGMMsg * pMsg);

void HandleRecrodSyscallMsg(void* pSocketObj, SQRecordSyscallMsg *pMsg);

void HandleAddSyscall(void* pSocketObj, SQAddSyscallMsg *pMsg);

void HandleUptSyscall(void* pSocketObj, SQUptSyscallMsg *pMsg);

void HandleDelSyscall(void* pSocketObj, SQDelSyscallMsg *pMsg);

void HandleGetContentByID(void* pSocketObj, SQGetContentByIDMsg * pMsg);