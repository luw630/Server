//#pragma once

#include "../NetModule.h"
#include "CollectDataDef.h"

#ifndef _COLLECTMESSAGE_H_
#define _COLLECTMESSAGE_H_

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 数据采集
//=============================================================================================
DECLARE_MSG_MAP(SCollectBaseMsg, SMessage, SMessage::EPRO_COLLECT_MESSAGE)
//{{AFX
EPRO_COLLECT_LOGIN,		// 登录
//EPRO_COLLECT_REQUEST,   // 数据请求
EPRO_COLLECT_TRANSFER,  // 数据传输
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 登录
DECLARE_MSG(SCollectLoginMsg, SCollectBaseMsg, SCollectBaseMsg::EPRO_COLLECT_LOGIN)
struct SQCollectLoginMsg : public SCollectLoginMsg
{
    char    szName[32];
    char    szPassword[32];
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// 数据请求
DECLARE_MSG(SCollectTransferMsg, SCollectBaseMsg, SCollectBaseMsg::EPRO_COLLECT_TRANSFER)
struct SQCollectTransferMsg : public SCollectTransferMsg
{
    WORD    wDataID;
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// 数据传输
struct SACollectTransferMsg : public SCollectTransferMsg
{
    CollectDataMsg  Data;
};
//---------------------------------------------------------------------------------------------


#endif  //_COLLECTMESSAGE_H_
