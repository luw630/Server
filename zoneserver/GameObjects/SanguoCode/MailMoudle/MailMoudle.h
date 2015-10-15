// ----- MailMoudle.h -----
//
//   --  Author: wk
//   --  Date:   15/01/18
//   --  Desc:   邮件系统
// --------------------------------------------------------------------
// 由于玩家离线 邮件发往数据库,取邮件为数据库取出,系统已兼容系统发个人,系统发全服,帮会邮件,玩家邮件,拍卖邮件等
//---------------------------------------------------------------------  

# pragma once

#include<memory>
#include "Networkmodule\SanguoPlayerMsg.h"
#include"../PUB/Singleton.h"
#include "NetWorkModule\MailMsg_SanGuo.h"



class MailMoudle : public CSingleton < MailMoudle >
{
public:
	MailMoudle();
	~MailMoudle();
	void DispatchMailMsg(DNID dnidClient, SMailMsg *pMsg, int  sid);
	bool Release();
	void SendMail_SYS_SanGuo(uint32_t _expiretime, MailInfo*_MailInfo);

	void SendMail_SanGuo(int recv_pSid, uint32_t _expiretime, MailInfo*_MailInfo);//以玩家id 发邮件
	void SendMail_SanGuoByName(char name[CONST_USERNAME], uint32_t _expiretime, MailInfo*_MailInfo);//以玩家名字发邮件

	void GetMailAwards(DNID dnidClient,SQ_GetAwards_MSG * pmsg, int sid);
	void DbBack_GetMailAwards(SA_DB_GetAwards_MSG* Msg);
	void SetMailState(SQ_SetMailState_MSG * pmsg,int sid);
	bool MBToUTF8(vector<char>& pu8, const char* pmb, int32_t mLen);
private:
	void GetMailInfoFromDB(int sid, DNID dnidClient);
	void DbBack_GetMailInfo(SA_DBGETMailInfoList* Msg);
};