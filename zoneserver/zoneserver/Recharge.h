// ----- Recharge.h -----
//
//   --  Author: wk
//   --  Date:   15/05/22
//   --  Desc:   �Ƚ����� ��ֵģ��,���⿪�̷߳�http�������ȡ���,�漰�������ȶ��̲߳���
// --------------------------------------------------------------------
//  
//---

#pragma once

#include"../PUB/Singleton.h"
#include "networkmodule/SanguoPlayerMsg.h"
#include "json/json.h"

#define MAX_BUF_HTTPBACK      1024  //http����󷵻����� ��󳤶�
//��ֵ����
enum RechargeType
{
	yuanbao = 1,  //��Ǯ
	month = 2, //�¿�
};

//��Ҫȡ�ĳ�ֵ��Ϣ�ṹ��
struct rechargeNeedGet 
{
	DWORD playerSid;
	DWORD balance; //���
	WORD platfrom;//ƽ̨ 1 ΢��ƽ̨ 2 QQƽ̨ 
	char url[MAX_ADDR_HTTP];
	rechargeNeedGet()
	{
		playerSid = 0;
		memset(url, 0, MAX_ADDR_HTTP);
	}
};
//ȡ����ֵ��Ϣ�󷵻ؽṹ��
struct rechargeGetBack
{
	DWORD playerSid; 
	DWORD balance; //���
	BYTE first_save;
};


class CRecharge : public CSingleton < CRecharge >
{
public:
	CRecharge();
	~CRecharge();
	DWORD LoopRechargeGet(LPVOID threadNum); //��ֵ�߳�����
	int ExitRechargeTHD(); //�˳���ֵ�߳�
	int pushRechargeGet(rechargeNeedGet * _rechargeNeedGet); //��ֵ��Ϣ�������
	int RechargeGetBackDispose(); //���̴߳����õ�ȡ�����
	int RecvRechargeMsg(CPlayer* player,SSGPlayerMsg* pMsg);//�յ���ֵ����
	int sendmsgRecharge(DWORD playerSid, int ret);
	int testhttp();

	//20150929 wk yijieSDK��ֵ������
	int  LoopRecharge_yijie(LPVOID threadNum);


private:
	int rechargeHttpGet(char * urlIn, int platform); //����http����
	int  rechargeAdd(rechargeGetBack * _rechargeGetBack);
	int  rechargeAdd_yijie(rechargeGetBack * _rechargeGetBack);
	int rechargeBack_json_get(int &ret, int &balance, int &first_save);
	//size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);

	int getRecharge_yijie(int psid, int feeType, int fee);

	CRITICAL_SECTION m_rechargeNeedGetLock;             // ȡ��ֵ���л�����
	CRITICAL_SECTION m_rechargeGetBackLock;             // ȡ��ֵ���ض��л�����
	std::queue <rechargeNeedGet *> m_rechargeNeedGet; //ȡ��ֵ����--http�߳�ʹ��
	std::queue <rechargeGetBack*> m_rechargeGetBack; //ȡ��ֵ���ض���--gameserverʹ��
	bool rechargeMark = false; //ȡ��ֵ����
	std::map <string,int> rechargeMap  ;//��ֵ���Ķ���
	Json::Reader Json_reader;
	Json::Value Json_Value;
};


DWORD WINAPI   g_LoopRechargeGet(LPVOID threadNum); //��ֵ�߳�����

