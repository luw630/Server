// ----- Recharge.h -----
//
//   --  Author: wk
//   --  Date:   15/05/22
//   --  Desc:   萌将三国 充值模块,另外开线程发http请求到企鹅取余额,涉及到加锁等多线程操作
// --------------------------------------------------------------------
//  
//---

#pragma once

#include"../PUB/Singleton.h"
#include "networkmodule/SanguoPlayerMsg.h"
#include "json/json.h"

#define MAX_BUF_HTTPBACK      1024  //http请求后返回数据 最大长度
//充值类型
enum RechargeType
{
	yuanbao = 1,  //充钱
	month = 2, //月卡
};

//需要取的充值信息结构体
struct rechargeNeedGet 
{
	DWORD playerSid;
	DWORD balance; //余额
	WORD platfrom;//平台 1 微信平台 2 QQ平台 
	char url[MAX_ADDR_HTTP];
	rechargeNeedGet()
	{
		playerSid = 0;
		memset(url, 0, MAX_ADDR_HTTP);
	}
};
//取到充值信息后返回结构体
struct rechargeGetBack
{
	DWORD playerSid; 
	DWORD balance; //余额
	BYTE first_save;
};


class CRecharge : public CSingleton < CRecharge >
{
public:
	CRecharge();
	~CRecharge();
	DWORD LoopRechargeGet(LPVOID threadNum); //充值线程运行
	int ExitRechargeTHD(); //退出充值线程
	int pushRechargeGet(rechargeNeedGet * _rechargeNeedGet); //充值消息放入队列
	int RechargeGetBackDispose(); //主线程处理获得的取余额结果
	int RecvRechargeMsg(CPlayer* player,SSGPlayerMsg* pMsg);//收到充值请求
	int sendmsgRecharge(DWORD playerSid, int ret);
	int testhttp();

	//20150929 wk yijieSDK充值服务器
	int  LoopRecharge_yijie(LPVOID threadNum);


private:
	int rechargeHttpGet(char * urlIn, int platform); //发送http请求
	int  rechargeAdd(rechargeGetBack * _rechargeGetBack);
	int  rechargeAdd_yijie(rechargeGetBack * _rechargeGetBack);
	int rechargeBack_json_get(int &ret, int &balance, int &first_save);
	//size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);

	int getRecharge_yijie(int psid, int feeType, int fee);

	CRITICAL_SECTION m_rechargeNeedGetLock;             // 取充值队列互斥锁
	CRITICAL_SECTION m_rechargeGetBackLock;             // 取充值返回队列互斥锁
	std::queue <rechargeNeedGet *> m_rechargeNeedGet; //取充值队列--http线程使用
	std::queue <rechargeGetBack*> m_rechargeGetBack; //取充值返回队列--gameserver使用
	bool rechargeMark = false; //取充值开关
	std::map <string,int> rechargeMap  ;//充值过的订单
	Json::Reader Json_reader;
	Json::Value Json_Value;
};


DWORD WINAPI   g_LoopRechargeGet(LPVOID threadNum); //充值线程运行

