// ----- MailMoudle.h -----
//
//   --  Author: wk
//   --  Date:   15/01/18
//   --  Desc:   �ʼ�ϵͳ
// --------------------------------------------------------------------
// ����������� �ʼ��������ݿ�,ȡ�ʼ�Ϊ���ݿ�ȡ��,ϵͳ�Ѽ���ϵͳ������,ϵͳ��ȫ��,����ʼ�,����ʼ�,�����ʼ���
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

	void SendMail_SanGuo(int recv_pSid, uint32_t _expiretime, MailInfo*_MailInfo);//�����id ���ʼ�
	void SendMail_SanGuoByName(char name[CONST_USERNAME], uint32_t _expiretime, MailInfo*_MailInfo);//��������ַ��ʼ�

	void GetMailAwards(DNID dnidClient,SQ_GetAwards_MSG * pmsg, int sid);
	void DbBack_GetMailAwards(SA_DB_GetAwards_MSG* Msg);
	void SetMailState(SQ_SetMailState_MSG * pmsg,int sid);
	bool MBToUTF8(vector<char>& pu8, const char* pmb, int32_t mLen);
private:
	void GetMailInfoFromDB(int sid, DNID dnidClient);
	void DbBack_GetMailInfo(SA_DBGETMailInfoList* Msg);
};