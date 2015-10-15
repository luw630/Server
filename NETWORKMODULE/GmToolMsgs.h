#pragma once

#include "NetModule.h"
#include "networkmodule/chatmsgs.h"
#include "mailmsg_sanguo.h"
const int MAX_GM_recharge = 30; //ȡ��ֵ��Ϣ�������
const int MAX_GM_online = 24; //ȡÿ�����������������
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// GM���������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SGMTOOLMsg, SMessage, SMessage::EPRO_GMTOOL_MSG)

	GMq_login,
	GMa_login,
	GMq_getplayerinfo,
	GMa_getplayerinfo,
	GMq_broadcast,
	GMa_broadcast,
	GM_registerNum,
	GM_stayAll,//�̶���������
	GM_stayOne,//��������
	GM_online,
	GM_recharge,
	GM_leave,//��ʧ
	GM_pointuse,//������־
	GM_sendmail,//���ʼ�
	GM_operation,//��������

END_MSG_MAP()
//---------------------------------------------------------------------------------------------
//gm��½
struct SQgm_login:public SGMTOOLMsg
{
	char m_GMUserName[32];
	char m_GMUserPwd[16];
	SQgm_login()
	{ 
		SGMTOOLMsg::_protocol = GMq_login;
	}
};
struct SAgm_login : public SGMTOOLMsg
{
	BYTE m_Result; //1�ɹ� 2������� 3�û�������
	SAgm_login()
	{
		SGMTOOLMsg::_protocol = GMa_login;
	}
};
//ȡ�����Ϣ
struct SQgm_getplayerinfo :public SGMTOOLMsg
{
	char m_GMUserInfo[32];
	BYTE itype;//1 ���� 2�˺� 3sid
	SQgm_getplayerinfo()
	{
		SGMTOOLMsg::_protocol = GMq_getplayerinfo;
	}
};

struct SAgm_getplayerinfo : public SGMTOOLMsg
{
	BYTE m_Result;//1 �ɹ�, 2ʧ��
	SFixData  pData;
	SAgm_getplayerinfo()
	{
		SGMTOOLMsg::_protocol = GMa_getplayerinfo;
	}
};

//����
struct SQgm_broadcast :public SGMTOOLMsg
{
	char  cChatData[MAX_CHAT_LEN];
	SQgm_broadcast()
	{
		SGMTOOLMsg::_protocol = GMq_broadcast;
	}
};
struct SAgm_broadcast : public SGMTOOLMsg
{
	BYTE m_Result;//1 �ɹ�, 2ʧ��
	SAgm_broadcast()
	{
		SGMTOOLMsg::_protocol = GMa_broadcast;
	}
};
//ȡʱ�����ע������
struct SQgm_registerNum :public SGMTOOLMsg
{
	uint32_t beginTime;//��ʼʱ��
	uint32_t endTime;//����ʱ��
	SQgm_registerNum()
	{
		SGMTOOLMsg::_protocol = GM_registerNum;
	}
};
struct SAgm_registerNum : public SGMTOOLMsg
{
	uint32_t registerNum;
	SAgm_registerNum()
	{
		SGMTOOLMsg::_protocol = GM_registerNum;
	}
};
//ȡĳ������,�̶����� 1,2,3,4,5,6,7,14,21,30�յ�����
struct SQgm_stayAll :public SGMTOOLMsg
{
	uint32_t beginTime;//����0��ʱ���
	uint32_t endTime;//����24ʱ���
	SQgm_stayAll()
	{
		SGMTOOLMsg::_protocol = GM_stayAll;
	}
};
struct SAgm_stayAll : public SGMTOOLMsg
{
	uint32_t result[11];//��1��Ϊ����ע����,����Ϊ����
	SAgm_stayAll()
	{
		SGMTOOLMsg::_protocol = GM_stayAll;
	}
};
//ȡĳ�յĵ�������
struct SQgm_stayOne :public SGMTOOLMsg
{
	uint32_t beginTime;//����0��ʱ���
	uint32_t endTime;//����24ʱ���
	int32_t day;//�ڼ�������
	SQgm_stayOne()
	{
		SGMTOOLMsg::_protocol = GM_stayOne;
	}
};
struct SAgm_stayOne : public SGMTOOLMsg
{
	int32_t result[2];//��1��Ϊ����ע����,����Ϊ����
	SAgm_stayOne()
	{
		SGMTOOLMsg::_protocol = GM_stayOne;
	}
};
//ȡĳ�յ���������
struct SQgm_online :public SGMTOOLMsg
{
	uint32_t beginTime;//����0��ʱ���
	SQgm_online()
	{
		SGMTOOLMsg::_protocol = GM_online;
	}
};
struct SAgm_online : public SGMTOOLMsg
{
	int32_t m_time[MAX_GM_online];//time
	int32_t m_num[MAX_GM_online];//num
	SAgm_online()
	{
		SGMTOOLMsg::_protocol = GM_online;
		memset(&m_time, 0, sizeof(int32_t)*MAX_GM_online);
		memset(&m_num, 0, sizeof(int32_t) *MAX_GM_online);
	}
};
//ȡʱ����ڳ�ֵ
struct SQgm_recharge :public SGMTOOLMsg
{
	uint32_t beginTime;//��ʼ����
	uint32_t endTime;//����ʱ��
	SQgm_recharge()
	{
		SGMTOOLMsg::_protocol = GM_recharge;
	}
};
struct SAgm_recharge : public SGMTOOLMsg
{

	int32_t result[MAX_GM_recharge];
	SAgm_recharge()
	{
		SGMTOOLMsg::_protocol = GM_recharge;
	}
};

//ȡ��ʧ���� beginTime ��ʼ��;leaveday ���յ���ʧ;itype=1���ȼ�,2���ؿ�,3��vip�ȼ�,4������;num��iype��Ӧ,��ȼ�=5
struct SQgm_leave:public SGMTOOLMsg 
{
	int32_t beginTime;
	int32_t leaveday;
	int32_t itype;
	int32_t num;
	SQgm_leave()
	{
		SGMTOOLMsg::_protocol = GM_leave;
	}
};
struct SAgm_leave:public SGMTOOLMsg
{
	int32_t itype;
	int32_t num;
	int32_t res;
	SAgm_leave()
	{
		SGMTOOLMsg::_protocol = GM_leave;
	}
};
//ȡ�������� -ȡ��������-optype=1���ȼ�,��vip�ȼ�;itype=��������,detailType��������,otherinfo����id��,lv_mini �ȼ���
struct SQgm_pointuse:public SGMTOOLMsg
{
	int32_t optype;
	int32_t begintime;
	int32_t itype;
	int32_t detailType;
	int32_t otherinfo;
	int32_t lv_mini;
	int32_t lv_max;
	SQgm_pointuse()
	{
		SGMTOOLMsg::_protocol = GM_pointuse;
	}
};
struct SAgm_pointuse:public SGMTOOLMsg
{
	int32_t res;
	SAgm_pointuse()
	{
		SGMTOOLMsg::_protocol = GM_pointuse;
	}
};

//���ʼ�,optype=1��ȫ���н�,2�������ʼ�
struct SQgm_mail :public SGMTOOLMsg
{
	int32_t optype;
	char name[CONST_USERNAME];//�������,optype=1ʱ����
	uint32_t _expiretime;//����ʱ��(����)
	MailInfo _MailInfo;
	SQgm_mail()
	{
		SGMTOOLMsg::_protocol = GM_sendmail;
		memset(name, 0, CONST_USERNAME);
	}
};

//ȡ����ͳ������
struct SQgm_operation :public SGMTOOLMsg
{
	int32_t itype;//1 �ؾ�Ѱ��;2 ����˵;3 ����ն��;4 ���䳡
	int32_t begintime;
	int32_t endtime;

	SQgm_operation()
	{
		SGMTOOLMsg::_protocol = GM_operation;
	}
};

//���ع���ͳ������
struct SAgm_operation :public SGMTOOLMsg
{
	int32_t itype;
	int32_t res1;
	int32_t res2;
	int32_t res3;
	int32_t res4;
	int32_t res5;
	int32_t res6;

	SAgm_operation()
	{
		SGMTOOLMsg::_protocol = GM_operation;
	}
};

