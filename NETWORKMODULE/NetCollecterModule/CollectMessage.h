//#pragma once

#include "../NetModule.h"
#include "CollectDataDef.h"

#ifndef _COLLECTMESSAGE_H_
#define _COLLECTMESSAGE_H_

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ���ݲɼ�
//=============================================================================================
DECLARE_MSG_MAP(SCollectBaseMsg, SMessage, SMessage::EPRO_COLLECT_MESSAGE)
//{{AFX
EPRO_COLLECT_LOGIN,		// ��¼
//EPRO_COLLECT_REQUEST,   // ��������
EPRO_COLLECT_TRANSFER,  // ���ݴ���
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ��¼
DECLARE_MSG(SCollectLoginMsg, SCollectBaseMsg, SCollectBaseMsg::EPRO_COLLECT_LOGIN)
struct SQCollectLoginMsg : public SCollectLoginMsg
{
    char    szName[32];
    char    szPassword[32];
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// ��������
DECLARE_MSG(SCollectTransferMsg, SCollectBaseMsg, SCollectBaseMsg::EPRO_COLLECT_TRANSFER)
struct SQCollectTransferMsg : public SCollectTransferMsg
{
    WORD    wDataID;
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// ���ݴ���
struct SACollectTransferMsg : public SCollectTransferMsg
{
    CollectDataMsg  Data;
};
//---------------------------------------------------------------------------------------------


#endif  //_COLLECTMESSAGE_H_
