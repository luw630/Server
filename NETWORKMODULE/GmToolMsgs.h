#pragma once

#include "NetModule.h"
#include "networkmodule/chatmsgs.h"
#include "mailmsg_sanguo.h"
const int MAX_GM_recharge = 30; //取充值信息最大条数
const int MAX_GM_online = 24; //取每日在线数据最大条数
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// GM工具相关消息类
//=============================================================================================
DECLARE_MSG_MAP(SGMTOOLMsg, SMessage, SMessage::EPRO_GMTOOL_MSG)

	GMq_login,
	GMa_login,
	GMq_getplayerinfo,
	GMa_getplayerinfo,
	GMq_broadcast,
	GMa_broadcast,
	GM_registerNum,
	GM_stayAll,//固定日期留存
	GM_stayOne,//单日留存
	GM_online,
	GM_recharge,
	GM_leave,//流失
	GM_pointuse,//消费日志
	GM_sendmail,//发邮件
	GM_operation,//功能数据

END_MSG_MAP()
//---------------------------------------------------------------------------------------------
//gm登陆
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
	BYTE m_Result; //1成功 2密码错误 3用户名错误
	SAgm_login()
	{
		SGMTOOLMsg::_protocol = GMa_login;
	}
};
//取玩家信息
struct SQgm_getplayerinfo :public SGMTOOLMsg
{
	char m_GMUserInfo[32];
	BYTE itype;//1 名字 2账号 3sid
	SQgm_getplayerinfo()
	{
		SGMTOOLMsg::_protocol = GMq_getplayerinfo;
	}
};

struct SAgm_getplayerinfo : public SGMTOOLMsg
{
	BYTE m_Result;//1 成功, 2失败
	SFixData  pData;
	SAgm_getplayerinfo()
	{
		SGMTOOLMsg::_protocol = GMa_getplayerinfo;
	}
};

//公告
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
	BYTE m_Result;//1 成功, 2失败
	SAgm_broadcast()
	{
		SGMTOOLMsg::_protocol = GMa_broadcast;
	}
};
//取时间段内注册人数
struct SQgm_registerNum :public SGMTOOLMsg
{
	uint32_t beginTime;//起始时间
	uint32_t endTime;//结束时间
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
//取某日留存,固定返回 1,2,3,4,5,6,7,14,21,30日的留存
struct SQgm_stayAll :public SGMTOOLMsg
{
	uint32_t beginTime;//当日0点时间戳
	uint32_t endTime;//当日24时间戳
	SQgm_stayAll()
	{
		SGMTOOLMsg::_protocol = GM_stayAll;
	}
};
struct SAgm_stayAll : public SGMTOOLMsg
{
	uint32_t result[11];//第1个为当日注册数,后面为留存
	SAgm_stayAll()
	{
		SGMTOOLMsg::_protocol = GM_stayAll;
	}
};
//取某日的单日留存
struct SQgm_stayOne :public SGMTOOLMsg
{
	uint32_t beginTime;//当日0点时间戳
	uint32_t endTime;//当日24时间戳
	int32_t day;//第几日留存
	SQgm_stayOne()
	{
		SGMTOOLMsg::_protocol = GM_stayOne;
	}
};
struct SAgm_stayOne : public SGMTOOLMsg
{
	int32_t result[2];//第1个为当日注册数,后面为留存
	SAgm_stayOne()
	{
		SGMTOOLMsg::_protocol = GM_stayOne;
	}
};
//取某日的在线人数
struct SQgm_online :public SGMTOOLMsg
{
	uint32_t beginTime;//当日0点时间戳
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
//取时间段内充值
struct SQgm_recharge :public SGMTOOLMsg
{
	uint32_t beginTime;//起始日期
	uint32_t endTime;//结束时间
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

//取流失数据 beginTime 起始日;leaveday 几日的流失;itype=1按等级,2按关卡,3按vip等级,4按引导;num和iype对应,如等级=5
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
//取消费数据 -取消费数据-optype=1按等级,按vip等级;itype=货币类型,detailType功能类型,otherinfo道具id等,lv_mini 等级段
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

//发邮件,optype=1发全服有奖,2发单人邮件
struct SQgm_mail :public SGMTOOLMsg
{
	int32_t optype;
	char name[CONST_USERNAME];//玩家名字,optype=1时无用
	uint32_t _expiretime;//过期时间(分钟)
	MailInfo _MailInfo;
	SQgm_mail()
	{
		SGMTOOLMsg::_protocol = GM_sendmail;
		memset(name, 0, CONST_USERNAME);
	}
};

//取功能统计数据
struct SQgm_operation :public SGMTOOLMsg
{
	int32_t itype;//1 秘境寻宝;2 将神传说;3 过关斩将;4 练武场
	int32_t begintime;
	int32_t endtime;

	SQgm_operation()
	{
		SGMTOOLMsg::_protocol = GM_operation;
	}
};

//返回功能统计数据
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

