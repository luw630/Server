//#include <WinSock2.h>
//#include <stdio.h>
//using namespace std;
#pragma comment(lib, "WS2_32")


#include "stdafx.h"
#include "Recharge.h"

#include "gameobjects\player.h"
#include "gameobjects\PlayerManager.h"
#include "gameobjects\SanguoCode\BaseDataManager.h"
#include "curl/curl.h"
#include "gameobjects\GlobalFunctions.h"




int recharge_timeout_sec = 3; //取充值超时时间/秒
char wr_buf[MAX_BUF_HTTPBACK + 1];//http请求后返回数据
int  wr_index; //http请求相关
char headers_buf[128] = { 0 }; //http请求cookie相关
#if MSDKTEST
char * qq_httpUrl = "http://msdktest.qq.com/mpay/get_balance_m?%s";//沙箱
#else
char * qq_httpUrl = "http://msdk.qq.com/mpay/get_balance_m?%s";//现网
#endif
//测试
//char* tempurl = "http://msdktest.qq.com/mpay/get_balance_m?openkey=OezXcEiiBSKSxW0eoylIePinS9UQEhiQu_QjCM8LWZcGZetQ_DzOL8zNTDjZBNp6Rfdrqj_3kfSuWRW6kKBJ04zmIaYSpyG9Twf57h-T9SOXxVCDBnbAIj49ePvONPA-TrJEKUFMLEJL02m_i0pRFw&pf =wechat_wx-73213123-android-73213123 - wx - wx7c582db0278ac464 - okoxhs - 7qMc3ZdUKG1NvGichHFXc & ts = 1433056540 & zoneid = 1 & appid = 1104522991 & openid = okoxhs - 7qMc3ZdUKG1NvGichHFXc & pfkey = b9338538af708013f0ce329512f2aeef&sig = q0Eyii88d / NV35JcmrbeWRJum8I =";


char* tempurl = "http://msdk.qq.com/mpay/get_balance_m?openkey=C6B9EC2F94CC250ABC6AAAECE1B87359&pay_token=C0D6B2299DB35141E251CA14934473CE&pf=desktop_m_qq-73213123-android-73213123-qq-1104737118-541DEC552F9CB6C6852ED6261328AB2F&ts=1436892699&zoneid=1011001&appkey=KZ12jlGOE1FzKRzs&appid=1104737118&openid=541DEC552F9CB6C6852ED6261328AB2F&pfkey=690bf8387c3af79960cf9e54c5965dde&sig=1ku2nG6FAzt+DnkmGD116kQvZqU=";


CRecharge::CRecharge()
{
}
CRecharge::~CRecharge()
{
}

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	int segsize = size * nmemb;
	if (wr_index + segsize > MAX_BUF_HTTPBACK) {
		*(int *)userp = 1;
		return 0;
	}
	memcpy((void *)&wr_buf[wr_index], buffer, (size_t)segsize);
	wr_index += segsize;
	wr_buf[wr_index] = 0;
	return segsize;
}

//发送http请求
int CRecharge::rechargeHttpGet(char *urlIn, int platform)
{
	CURL *curl;
	CURLcode ret;
	int  wr_error;

	wr_error = 0;
	wr_index = 0;
	curl = curl_easy_init();
	if (!curl) {
		//rfalse("couldn't init curl ");
		return 0;
	}

	struct curl_slist *headers = NULL;
	
	sprintf_s(headers_buf, sizeof(headers_buf), "%s", "Expect:");
	headers = curl_slist_append(headers, headers_buf);
	if (platform == 1) //微信
		headers = curl_slist_append(headers, "Cookie:session_id=hy_gameid; session_type=wc_actoken; org_loc=/mpay/get_balance_m");
	else//QQ
		headers = curl_slist_append(headers, "Cookie:session_id=openid; session_type=kp_actoken; org_loc=/mpay/get_balance_m");

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	static char errorBuffer[CURL_ERROR_SIZE];
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
	curl_easy_setopt(curl, CURLOPT_URL, urlIn); // ok
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &wr_error); // ok
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); // ok

	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (int)(recharge_timeout_sec * 1000));
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, (int)(recharge_timeout_sec * 1000));

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

	ret = curl_easy_perform(curl);
	//rfalse("ret = %d (write_error = %d) ", ret, wr_error);
	//if (ret == 0) rfalse("%s ", wr_buf);

	/*FILE * fp; fopen_s(&fp, "C:\\123.htm", "wb");
	fwrite(wr_buf, strlen(wr_buf), 1, fp);
	fclose(fp);*/
	curl_easy_cleanup(curl);
}

//请求反馈json解析
int CRecharge::rechargeBack_json_get(int &ret, int &balance, int &first_save)
{
	if (Json_reader.parse(wr_buf, Json_Value))  // reader将Json字符串解析到Json_Value，Json_Value将包含Json里所有子元素  
	{
		if (Json_Value["ret"].isNull())
		{
			ret = -1;
		}
		else
		{
			ret = Json_Value["ret"].asInt();    // 结果 0成功过
			balance = Json_Value["balance"].asInt();    // 余额
			first_save = Json_Value["first_save"].asInt();    // 是否首次 1：满足，0：不满足
		}
	}
	return ret;
}
//充值消息进入
int CRecharge::pushRechargeGet(rechargeNeedGet * _rechargeNeedGet)
{
	__try
	{
		EnterCriticalSection(&m_rechargeNeedGetLock);
		m_rechargeNeedGet.push(_rechargeNeedGet);
	}
	__finally
	{
		LeaveCriticalSection(&m_rechargeNeedGetLock);
	}
	return 1;
}
//发送回馈消息到前台
int CRecharge::sendmsgRecharge(DWORD playerSid, int ret)
{
	
	CPlayer* pPlayer;
	pPlayer = (CPlayer *)GetPlayerBySID(playerSid)->DynamicCast(IID_PLAYER);
	if (pPlayer == nullptr)
	{
		return 0;
	}
	int32_t nowRecharge = pPlayer->GetSanguoBaseData()->GetMaxRecharge();
	SARecharge rMsg;
	rMsg.nowPoint = nowRecharge;
	rMsg.ret = ret;
	g_StoreMessage(pPlayer->m_ClientIndex, &rMsg, sizeof(SARecharge));
	return 1;
}
//退出充值线程
int CRecharge::ExitRechargeTHD()
{
	rechargeMark = false;
	
	return 1;
}

int CRecharge::testhttp()
{
	rechargeNeedGet*  _rechargeNeedGet = new rechargeNeedGet;
	_rechargeNeedGet->playerSid = 1111;
	char addr[MAX_ADDR_HTTP]="2openkey=C6B9EC2F94CC250ABC6AAAECE1B87359&pay_token=C0D6B2299DB35141E251CA14934473CE&pf=desktop_m_qq-73213123-android-73213123-qq-1104737118-541DEC552F9CB6C6852ED6261328AB2F&ts=1436892947&zoneid=1011001&appkey=KZ12jlGOE1FzKRzs&appid=1104737118&openid=541DEC552F9CB6C6852ED6261328AB2F&pfkey=690bf8387c3af79960cf9e54c5965dde&sig=lsDfZy9xgp94kvD+QChglxyhYU4=";
	memcpy(_rechargeNeedGet->url, addr, strlen(addr));
	m_rechargeNeedGet.push(_rechargeNeedGet);
	return 1;
}
//充值线程运行
DWORD CRecharge::LoopRechargeGet(LPVOID threadNum)
{
	
	rechargeMark = true;
	InitializeCriticalSection(&m_rechargeGetBackLock);
	InitializeCriticalSection(&m_rechargeNeedGetLock);
	while (rechargeMark)
	{
		Sleep(1);
		if (m_rechargeNeedGet.size() > 0)
		{
			char httpurl[1024] ;
			int platform = m_rechargeNeedGet.front()->platfrom; 
			sprintf_s(httpurl, sizeof httpurl, qq_httpUrl, &m_rechargeNeedGet.front()->url);
			httpurl[sizeof(httpurl)-1] = 0;
			rfalse(httpurl);
			rechargeHttpGet(httpurl, platform);

			//rechargeHttpGet(tempurl, 21);
		
			int ret=-1000, balance, first_save;
			rechargeBack_json_get(ret, balance, first_save);

			//rfalse(m_rechargeNeedGet.front()->url);
			rfalse("查询余额结果 ret==%d",ret);
			rfalse(wr_buf);
			
			if (ret==0)//成功
			{
				if (balance != m_rechargeNeedGet.front()->balance) //TODO ：如果检查结果与客户端余额不同，则再次请求 
				{
					return 0;
				}
				rechargeGetBack* _rechargeGetBack = new rechargeGetBack;
				_rechargeGetBack->balance = balance;
				_rechargeGetBack->first_save = first_save;
				_rechargeGetBack->playerSid = m_rechargeNeedGet.front()->playerSid;

				__try
				{
					EnterCriticalSection(&m_rechargeGetBackLock);
					m_rechargeGetBack.push(_rechargeGetBack);
				}
				__finally
				{
					LeaveCriticalSection(&m_rechargeGetBackLock);
				}
			}
			else//失败
			{
				rfalse("查询余额结果失败 playerid==%d", m_rechargeNeedGet.front()->playerSid);
				sendmsgRecharge(m_rechargeNeedGet.front()->playerSid, ret);
			}

			__try
			{
				EnterCriticalSection(&m_rechargeNeedGetLock);
				delete  m_rechargeNeedGet.front();
				m_rechargeNeedGet.pop();
			}
			__finally
			{
				LeaveCriticalSection(&m_rechargeNeedGetLock);
			}
			

		}
	}
	DeleteCriticalSection(&m_rechargeGetBackLock);
	DeleteCriticalSection(&m_rechargeNeedGetLock);
	return 1;
}



//充值加钱处理
int  CRecharge::rechargeAdd(rechargeGetBack * _rechargeGetBack)
{
	CPlayer* pPlayer;
	pPlayer = (CPlayer *)GetPlayerBySID(_rechargeGetBack->playerSid)->DynamicCast(IID_PLAYER);
	if (pPlayer == nullptr)
		return 0;

	CBaseDataManager* baseDataMgr = pPlayer->GetSanguoBaseData();
	int32_t preRecharge = baseDataMgr->GetMaxRecharge();
	int32_t curRecharge = _rechargeGetBack->balance - preRecharge;


	lite::Variant ret;//从lua获取充值标识
	LuaFunctor(g_Script, "GetRechargeFlag")[baseDataMgr->GetSID()](&ret);
	g_Script.SetPlayer(pPlayer);
	if (preRecharge >= _rechargeGetBack->balance) //充值有误,记录余额比腾讯记录余额大
	{
		sendmsgRecharge(_rechargeGetBack->playerSid, 2);
		g_Script.CleanPlayer();
		rfalse("recharge is wrong！！！！！！！！！,\n sid is : %d ", _rechargeGetBack->playerSid);
		rfalse("MaxRecharge is %d", preRecharge);
		rfalse("current balance is %d", _rechargeGetBack->balance);
		return 0;
	}

	int rechargeFlag = (int)ret;
	if (rechargeFlag != 2)//玩家在线处理,不在线等下次上线单独处理
	{
		baseDataMgr->AddGoods_SG(GoodsType::diamond, 0, curRecharge, GoodsWay::recharge);
		baseDataMgr->AddGoods_SG(GoodsType::vipExp, 0, curRecharge, GoodsWay::recharge);
		baseDataMgr->AddMaxRechage(curRecharge);
		sendmsgRecharge(_rechargeGetBack->playerSid, 1);
		//触发脚本玩家充值成功的回调函数
		LuaFunctor(g_Script, "OnChagreSuccess")[baseDataMgr->GetSID()][curRecharge]();
		rfalse("sid:%d充值成功,现在最大值为==%d", baseDataMgr->GetSID(),pPlayer->GetSanguoBaseData()->GetMaxRecharge());
		
	}
	else //充值月卡 充值标记为2
	{
		lite::Variant ret2;//从lua获取到的返回值
		bool bResult = false;
		BOOL execResult = FALSE;
		if (g_Script.PrepareFunction("BuyMonthlyMembership"))
		{
			g_Script.PushParameter(baseDataMgr->GetSID());
			g_Script.PushParameter(curRecharge);
			execResult = g_Script.Execute(&ret2);
		}

		if (execResult == TRUE && ret2.dataType != LUA_TNIL)  //月卡购买是否成功
		{
			bResult = true;
			baseDataMgr->AddMaxRechage(curRecharge);
			rfalse("sid:%d购买月卡成功,现在最大值为==%d", baseDataMgr->GetSID(), pPlayer->GetSanguoBaseData()->GetMaxRecharge());
		}
		sendmsgRecharge(_rechargeGetBack->playerSid, bResult);
	}
	g_Script.CleanPlayer();
	return 1;
}

//主线程处理获得的取余额结果
int CRecharge::RechargeGetBackDispose()
{

	while (m_rechargeGetBack.size()>0)
	{
		if (GetGroupID()/10000>110) //101-104为qq平台,111为360平台
		{
			rechargeAdd_yijie(m_rechargeGetBack.front());
		}
		else
		{
			rechargeAdd(m_rechargeGetBack.front());
		}
		
		
		__try
		{
			EnterCriticalSection(&m_rechargeGetBackLock);
			delete  m_rechargeGetBack.front();
			m_rechargeGetBack.pop();
		}
		__finally
		{
			LeaveCriticalSection(&m_rechargeGetBackLock);
		}
	}
	return 1;
}

int CRecharge::RecvRechargeMsg(CPlayer* pPlayer,SSGPlayerMsg* pMsg)
{
	SRecharge*  recMsg = static_cast<SRecharge*>(pMsg);
	switch (recMsg->_protocol)
	{
	case SRecharge::SANGUO_RECHARGE://余额查询http请求
	{
		SQRecharge* _SQRECHARGE = static_cast<SQRecharge*>(pMsg);
		rechargeNeedGet*  _rechargeNeedGet = new rechargeNeedGet;
		_rechargeNeedGet->playerSid = pPlayer->GetSID();
		_rechargeNeedGet->platfrom = _SQRECHARGE->wPlatfrom;
		_rechargeNeedGet->balance = _SQRECHARGE->dwNowPoint;
		memcpy(_rechargeNeedGet->url, _SQRECHARGE->addr, strlen(_SQRECHARGE->addr));
		pushRechargeGet(_rechargeNeedGet);
	}
		break;
	case SRecharge::SANGUO_RECHARGE_REQUEST: //充值请求
	{
		SARechargeRequst buyMsg;
		buyMsg.bResult = false;
		lite::Variant ret0;//从lua获取充值标识
		LuaFunctor(g_Script, "GetRechargeFlag")[pPlayer->GetSID()](&ret0);
		if (ret0.dataType != LUA_TNIL && (int)ret0 == 0) //充值标识为0
		{
			SQRechargeRequst* msgRequset = static_cast<SQRechargeRequst*>(pMsg);
			buyMsg.byType = msgRequset->byType;
			lite::Variant ret;//从lua获取到的返回值
			BOOL execResult = FALSE;
			if (g_Script.PrepareFunction("SetRechargeFlag"))
			{
				g_Script.PushParameter(pPlayer->GetSID());
				g_Script.PushParameter(msgRequset->byType);
				execResult = g_Script.Execute(&ret);
			}

			if (execResult == TRUE && ret.dataType != LUA_TNIL)
			{
				int request = (int)ret;
				if (request != 0)
				{
					buyMsg.bResult = true;
					buyMsg.dwDiamond = msgRequset->dwDiamond; 
				}
			}
		}
		g_StoreMessage(pPlayer->m_ClientIndex, &buyMsg, sizeof(buyMsg));
	}
		break;
	case SRecharge::SANGUO_RECHARGE_RESET: //充值标识重置
	{
		if (g_Script.PrepareFunction("SetRechargeFlag"))
		{
			g_Script.PushParameter(pPlayer->GetSID());
			g_Script.PushParameter(0);
			g_Script.Execute();
		}
	}
		break;
	default:
		break;
	}
	return 1;
}


#define BUFFER_SIZE 1024
#define HOST "127.0.0.1"
#define PORT 8999
#define HEADER "\
HTTP/1.1 200 OK\r\n\
Content-Type: text/html; charset=gb2312\r\n\
Server: ZYhttp_v1.0.1\r\n\
Content-Length: %d\r\n\r\n%s\
"
#define HTML "\
<html>\
    <head>\
        <title>擦擦擦</title>\
    <head>\
    <body>\
        <h1>1</h1>\
    </body>\
</html>\
"

//易接sdk充值,http服务器
int  CRecharge::LoopRecharge_yijie(LPVOID threadNum)
{
	rechargeMark = true;
	int rechargePort = GetGroupID() % 10000 + 10000;
	InitializeCriticalSection(&m_rechargeGetBackLock);
	// define and init an server sockaddr
	sockaddr_in addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.S_un.S_addr = INADDR_ANY;
	addrServer.sin_port = htons(rechargePort);
	// init socket dll
	WSADATA wsaData;
	WORD socketVersion = MAKEWORD(2, 0);
	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		rfalse("Init socket dll error!");
		exit(1);
	}
	// create socket
	SOCKET socketServer = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == socketServer)
	{
		rfalse("Create socket error!");
		exit(1);
	}
	// bind server socket host
	//int a = ::bind(socketServer, (LPSOCKADDR)&addrServer, sizeof(addrServer));
	if (SOCKET_ERROR == ::bind(socketServer, (LPSOCKADDR)&addrServer, sizeof(addrServer)))
	{
		rfalse("Bind server host failed!");
		exit(1);
	}
	// listen
	if (SOCKET_ERROR == listen(socketServer, 10))
	{
		rfalse("Listen failed!");
		exit(1);
	}
	while (rechargeMark)
	{
		rfalse("Listening ... \n");
		sockaddr_in addrClient;
		int nClientAddrLen = sizeof(addrClient);
		SOCKET socketClient = accept(socketServer, (sockaddr*)&addrClient, &nClientAddrLen);
		if (SOCKET_ERROR == socketClient)
		{
			rfalse("Accept failed!");
			break;
		}
		char buffer[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		if (recv(socketClient, buffer, BUFFER_SIZE, 0) < 0)
		{
			rfalse("Recvive data failed!");
			break;
		}
		rfalse("Recv data : \n%s", buffer);
		// response 

		

		if (GetGroupID() / 10000 < 800) // 易接
		{
			//char* gethuffer_test = "http://service.cp.host.domain/path/?app=1234567890ABCDEF&cbi=1000100008-1&ct=1376578903&fee=100&pt=1376577801&ssid=123456&st=1&tcd=137657AVDEDFS&uid=1234&ver=1& sign = xxxxxxxxxxx";
			string rechargeBuff = buffer;

			size_t iPos1 = rechargeBuff.find("cbi");
			size_t iPos2 = rechargeBuff.find("&", iPos1);
			string cib = rechargeBuff.substr(iPos1 + 4, iPos2 - iPos1 - 4);

			iPos1 = cib.find("-");
			string psidStr = cib.substr(0, iPos1);//玩家sid
			string feeTypeStr = cib.substr(iPos1 + 1, cib.length());//充值

			//iPos1 = rechargeBuff.find("ct");
			//iPos2 = rechargeBuff.find("&", iPos1);
			//string ct = rechargeBuff.substr(iPos1 + 3, iPos2 - iPos1 - 3);//支付完成时间

			iPos1 = rechargeBuff.find("fee");
			iPos2 = rechargeBuff.find("&", iPos1);
			string feeStr = rechargeBuff.substr(iPos1 + 4, iPos2 - iPos1 - 4);//金额（分）

			iPos1 = rechargeBuff.find("ssid");
			iPos2 = rechargeBuff.find("&", iPos1);
			string ssid = rechargeBuff.substr(iPos1 + 5, iPos2 - iPos1 - 5);//订单在渠道平台上的流水号

			iPos1 = rechargeBuff.find("st", iPos2);
			iPos2 = rechargeBuff.find("&", iPos1);
			string stStr = rechargeBuff.substr(iPos1 + 3, iPos2 - iPos1 - 3);//是否成功标志，1 标示成功，其余都表示失败

			int  psid = atoi(psidStr.c_str());
			int  feeType = atoi(feeTypeStr.c_str());
			int  fee = atoi(feeStr.c_str());
			int  st = atoi(stStr.c_str());

			if (rechargeMap.find(ssid) == rechargeMap.end() && st == 1)
			{
				rfalse("already have rechargeMap");
				rechargeMap[ssid] = 1;
				getRecharge_yijie(psid, feeType, fee / 10);
			}

			char sendBuffer[BUFFER_SIZE];
			memset(sendBuffer, 0, BUFFER_SIZE);
			//sprintf_s(sendBuffer, HEADER, strlen(HTML), HTML);
			sprintf_s(sendBuffer, HEADER, strlen("SUCCESS"), "SUCCESS");
			if (send(socketClient, sendBuffer, strlen(sendBuffer), 0) < 0)
			{
				rfalse("Send data failed!");
				break;
			}
			rfalse("Send data : \n%s", sendBuffer);
		}
		else //九州,ios,日韩
		{
			/*
			applicationCode   程序标识id
			referenceId       唯一订单号
			paymentId         透传参数
			amount            这次总额
			totalAmount       充值总金额
			paymentStatusCode 充值状态
					00 Success     Payment completed and paid.
					01 Incomplete  Payment has not complete or in middle of processing.
					02 Expired	   Payment has been failed as expired.
					99 Failure     Payment for the given transaction failed.
			paymentStatusDate 充值日期  2015-10-26 15:46:02
			version           版本      ex: 1.0
			signature       sha1(applicationCode+referenceId+paymentId+amount+totalAmount+paymentStatusCode+paymentStatusDate+version+appkey) 
			applicationCode= 200101001   applicationKey=cb45cf1bc59924759b8eeb77f26aca0a
			*/
			char* gethuffer_test = "http://service.cp.host.domain/path/?applicationCode=1234567890ABCDEF&referenceId=1000100008-1&paymentId         =1000100008-1&amount=100&totalAmount=1376577801&paymentStatusCode=123456&paymentStatusDate =1&version=137657AVDEDFS&signature= xxxxxxxxxxx";
			string rechargeBuff = buffer;

			size_t iPos1 = rechargeBuff.find("paymentId");
			size_t iPos2 = rechargeBuff.find("&", iPos1);
			string paymentId = rechargeBuff.substr(iPos1 + 10, iPos2 - iPos1 - 10);

			iPos1 = paymentId.find("-");
			string psidStr = paymentId.substr(0, iPos1);//玩家sid
			string feeTypeStr = paymentId.substr(iPos1 + 1, paymentId.length());//充值

			//iPos1 = rechargeBuff.find("ct");
			//iPos2 = rechargeBuff.find("&", iPos1);
			//string ct = rechargeBuff.substr(iPos1 + 3, iPos2 - iPos1 - 3);//支付完成时间

			iPos1 = rechargeBuff.find("amount");
			iPos2 = rechargeBuff.find("&", iPos1);
			string feeStr = rechargeBuff.substr(iPos1 + 7, iPos2 - iPos1 - 7);//金额（分）

			iPos1 = rechargeBuff.find("referenceId");
			iPos2 = rechargeBuff.find("&", iPos1);
			string ssid = rechargeBuff.substr(iPos1 + 12, iPos2 - iPos1 - 12);//订单在渠道平台上的流水号

			iPos1 = rechargeBuff.find("paymentStatusCode", iPos2);
			iPos2 = rechargeBuff.find("&", iPos1);
			string stStr = rechargeBuff.substr(iPos1 + 18, iPos2 - iPos1 - 18);//是否成功标志，0 标示成功，其余都表示失败

			int  psid = atoi(psidStr.c_str());
			int  feeType = atoi(feeTypeStr.c_str());
			int  fee = atoi(feeStr.c_str());
			int  st = atoi(stStr.c_str());

			if (rechargeMap.find(ssid) == rechargeMap.end() && st == 0)
			{
				rfalse("already have rechargeMap");
				rechargeMap[ssid] = 1;
				getRecharge_yijie(psid, feeType, fee );//传过来即为元宝数
			}

			char sendBuffer[BUFFER_SIZE];
			memset(sendBuffer, 0, BUFFER_SIZE);
			//sprintf_s(sendBuffer, HEADER, strlen(HTML), HTML);
			sprintf_s(sendBuffer, HEADER, strlen("1"), "1");
			if (send(socketClient, sendBuffer, strlen(sendBuffer), 0) < 0)
			{
				rfalse("Send data failed!");
				break;
			}
			rfalse("Send data : \n%s", sendBuffer);
		}


		closesocket(socketClient);
	}

	DeleteCriticalSection(&m_rechargeGetBackLock);
	closesocket(socketServer);
	WSACleanup();
	return 0;
}
int CRecharge::getRecharge_yijie(int psid, int feeType, int fee)
{
	rechargeGetBack* _rechargeGetBack = new rechargeGetBack;
	_rechargeGetBack->balance = fee;
	_rechargeGetBack->first_save = feeType;//充值类型,1充值,2月卡
	_rechargeGetBack->playerSid = psid;

	__try
	{
		EnterCriticalSection(&m_rechargeGetBackLock);
		m_rechargeGetBack.push(_rechargeGetBack);
	}
	__finally
	{
		LeaveCriticalSection(&m_rechargeGetBackLock);
	}
	return 1;
}


DWORD WINAPI   g_LoopRechargeGet(LPVOID threadNum) //充值线程运行
{
	if (GetGroupID() / 10000 > 110) //101-104为qq平台,111为360平台
	{
		CRecharge::getSingleton()->LoopRecharge_yijie(threadNum);
	}
	else
	{
		CRecharge::getSingleton()->LoopRechargeGet(threadNum);
	}
	return 1;
}



int  CRecharge::rechargeAdd_yijie(rechargeGetBack * _rechargeGetBack)
{
	CPlayer* pPlayer;
	pPlayer = (CPlayer *)GetPlayerBySID(_rechargeGetBack->playerSid)->DynamicCast(IID_PLAYER);
	if (pPlayer == nullptr)
		return 0;

	CBaseDataManager* baseDataMgr = pPlayer->GetSanguoBaseData();
	//int32_t preRecharge = baseDataMgr->GetMaxRecharge();
	int32_t curRecharge = _rechargeGetBack->balance;
	g_Script.SetPlayer(pPlayer);


	switch (_rechargeGetBack->first_save)
	{
	case RechargeType::yuanbao://元宝
	{
		baseDataMgr->AddGoods_SG(GoodsType::diamond, 0, curRecharge, GoodsWay::recharge);
		baseDataMgr->AddGoods_SG(GoodsType::vipExp, 0, curRecharge, GoodsWay::recharge);
		baseDataMgr->AddMaxRechage(curRecharge);
		sendmsgRecharge(_rechargeGetBack->playerSid, 1);
		
		//触发脚本玩家充值成功的回调函数
		LuaFunctor(g_Script, "OnChagreSuccess")[_rechargeGetBack->playerSid][curRecharge]();
		rfalse("sid:%d充值成功%d,现在最大值为==%d", _rechargeGetBack->playerSid, curRecharge, baseDataMgr->GetMaxRecharge());
	}
	break;
	case RechargeType::month://月卡
	{
		lite::Variant ret2;//从lua获取到的返回值

		bool bResult = false;
		BOOL execResult = FALSE;
		if (g_Script.PrepareFunction("BuyMonthlyMembership"))
		{
			g_Script.PushParameter(_rechargeGetBack->playerSid);
			g_Script.PushParameter(curRecharge);
			execResult = g_Script.Execute(&ret2);
		}

		if (execResult == TRUE && ret2.dataType != LUA_TNIL)  //月卡购买是否成功
		{
			bResult = true;
			baseDataMgr->AddMaxRechage(curRecharge);
			rfalse("sid:%d购买月卡成功,现在最大值为==%d", _rechargeGetBack->playerSid, pPlayer->GetSanguoBaseData()->GetMaxRecharge());
		}
		sendmsgRecharge(_rechargeGetBack->playerSid, bResult);
	}
	break;
	default:
		break;
	}
	
	g_Script.CleanPlayer();
	return 1;
}