// ----- GmToolMoudle.h -----
//
//   --  Author: wk
//   --  Date:   15/03/13
//   --  Desc:   GmToolMoudleϵͳ
// --------------------------------------------------------------------

//---------------------------------------------------------------------  

# pragma once

#include<memory>
#include "Networkmodule\SanguoPlayerMsg.h"
#include"../PUB/Singleton.h"
#include "NetWorkModule\GmToolMsgs.h"



class GmToolMoudle : public CSingleton < GmToolMoudle >
{
public:
	GmToolMoudle();
	~GmToolMoudle();
	void DispatchGmToolMsg(DNID dnidClient, SGMTOOLMsg *pMsg);
	bool Release();

private:
	void gm_login( DNID dnidClient, SQgm_login *pMsg);
	void gm_getplayerinfo(DNID dnidClient, SQgm_getplayerinfo *pMsg);
	void gm_broadcast(DNID dnidClient, SQgm_broadcast *pMsg);
	void gm_registerNum(DNID dnidClient, SQgm_registerNum *pMsg);
	void gm_stayAll(DNID dnidClient, SQgm_stayAll *pMsg);
	void gm_stayOne(DNID dnidClient, SQgm_stayOne *pMsg);
	void gm_online(DNID dnidClient, SQgm_online *pMsg);
	void gm_recharge(DNID dnidClient, SQgm_recharge *pMsg);
	void gm_leavedata(DNID dnidClient, SQgm_leave *pMsg);
	void gm_pointuse(DNID dnidClient, SQgm_pointuse *pMsg);
	void gm_mail(DNID dnidClient, SQgm_mail *pMsg);
	void gm_operation(DNID dnidClient, SQgm_operation *pMsg);
};