#include "StdAfx.h"
#include "QuestCardPointModify.h"

#include "NetworkModule/CardPointModifyMsgs.h"

int SendToLoginServer(SMessage *data, long size);

BOOL QuestCardPointModify(SExchangeVerifyInfo &vi, 
                          std::string &SrcAccount, 
                          std::string &DstAccount, 
                          DWORD dwPoint, DWORD SrcGID, DWORD DstGID)
{
    //SQPrepareExchangeMsg msg;
    //msg.vi = vi;
    //msg.SrcAccount = SrcAccount;
    //msg.DstAccount = DstAccount;
    //msg.TradePoint = dwPoint;
    //msg.SrcGID = SrcGID;
    //msg.DstGID = DstGID;

    //SendToLoginServer(&msg, sizeof(msg));

    return true;
}

BOOL QuestConfirmPointModify(SExchangeVerifyInfo &vi, 
                          std::string &SrcAccount, 
                          std::string &DstAccount, 
                          DWORD dwPoint)
{
    //SQConfirmExchangeMsg msg;
    //msg.vi = vi;
    //msg.SrcAccount = SrcAccount;
    //msg.DstAccount = DstAccount;
    //msg.TradePoint = dwPoint;
    //msg.isConfirm = TRUE;

    //SendToLoginServer(&msg, sizeof(msg));

    return true;
}

BOOL QuestCancelPointModify(SExchangeVerifyInfo &vi, 
                          std::string &SrcAccount, 
                          std::string &DstAccount, 
                          DWORD dwPoint)
{
    //SQConfirmExchangeMsg msg;
    //msg.vi = vi;
    //msg.SrcAccount = SrcAccount;
    //msg.DstAccount = DstAccount;
    //msg.TradePoint = dwPoint;
    //msg.isConfirm = FALSE;

    //SendToLoginServer(&msg, sizeof(msg));

    return true;
}

//=========================================================================================
BOOL NotifyMoneyPointToLogin(const char *szAccount, DWORD dwOperate, DWORD dwMoneyPoint)
{
	//if(szAccount == NULL)
	//	return FALSE;

	//if(szAccount[0] < ' ')
	//	return FALSE;

	//SQNotifyMoneyPointMsg msg;
	//msg.dwOpeate = dwOperate;
	//msg.dwMoneyPoint = dwMoneyPoint;
	//dwt::strcpy(msg.szAccount, szAccount, sizeof(msg.szAccount));
	//
	//SendToLoginServer(&msg, sizeof(msg));

	return true;
}