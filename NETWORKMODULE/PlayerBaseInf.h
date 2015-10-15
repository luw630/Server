//gm工具相关消息

#pragma  once

#define  CONST_NAME 32
struct BaseMessage
{
	enum
	{
		MSG_PLAYERCFG,
		MSG_SYSCALL,
		MSG_LOGIN=224,
	};
	BYTE m_FirstFlag;
	BaseMessage()
	{}
};

struct BasePlayerCfgMsg : public BaseMessage
{
	enum
	{
		MSG_FINDPLAYER,		//检索指定的玩家
		MSG_SAVEUPTPLAYER,	//保存更新后的玩家数据
		MSG_TRANSFORMPLAYER,	//传送玩家到指定地图
		MSG_ADDITEM,	//添加道具
	};

	BYTE m_SecondFlag;
	BasePlayerCfgMsg()
	{
		BaseMessage::m_FirstFlag = BaseMessage::MSG_PLAYERCFG;
	}
};

struct BaseSyscallMsg : public BaseMessage
{
	enum
	{
		MSG_GETRECORDSYSCALL,   //获取系统公告记录
		MSG_ADDSYSCALL,     //添加系统公告
		MSG_UPTSYSCALL,		//更新系统公告
		MSG_DELSYSCALL,		//删除系统公告
		MSG_GETCONTENTBYID,	//通过公告ID获取公告内容
	};

	BYTE m_SecondFlag;
	BaseSyscallMsg()
	{
		BaseMessage::m_FirstFlag = BaseMessage::MSG_SYSCALL;
	}
};

struct BaseLoginMsg : public BaseMessage
{
	enum
	{
		MSG_GMLOGIN,      //GM管理员登陆
	};

	BYTE m_SecondFlag;
	BaseLoginMsg()
	{
		BaseMessage::m_FirstFlag = BaseMessage::MSG_LOGIN;
	}
};

struct SQAPlayerBaseInf : public BasePlayerCfgMsg
{
	SQAPlayerBaseInf()
	{
		BasePlayerCfgMsg::m_SecondFlag = BasePlayerCfgMsg::MSG_FINDPLAYER;
	}
};

struct SQPlayerBaseInf : public SQAPlayerBaseInf
{
	char m_PlayerName[CONST_NAME];
	SQPlayerBaseInf()
	{
		memset(m_PlayerName, 0, CONST_NAME);
	}
};

struct SAPlayerBaseInf : public SQAPlayerBaseInf
{
	BYTE m_Result;	//检索结果0表示失败，后面数据无效，1表示成功
	BYTE m_School;
	BYTE m_Level;
	BYTE m_VIPLevel;
	DWORD m_PlayerGID;
	DWORD m_YinBi;
	DWORD m_JinBi;
	DWORD m_GloryDot;
	DWORD m_Sp;
	DWORD m_Hp;
	DWORD m_Tp;
	DWORD m_GongJi;
	DWORD m_FangYu;
	DWORD m_Hit;
	DWORD m_ShanBi;
	DWORD m_BaoJi;
	DWORD m_KangBao;
	DWORD m_PoJi;
	DWORD m_KangPo;
	DWORD m_ChuanCi;
	DWORD m_KangChuan;
	SAPlayerBaseInf()
	{
		m_Result = 1;
		m_School = 1;
		m_Level = 1;
		m_VIPLevel = 1;
		m_YinBi = 1;
		m_JinBi = 0;
		m_GloryDot = 0;
		m_Sp = 0;
		m_Hp = 0;
		m_Tp = 0;
		m_GongJi = 0;
		m_FangYu = 0;
		m_Hit = 0;
		m_ShanBi = 0;
		m_BaoJi = 0;
		m_KangBao = 0;
		m_PoJi = 0;
		m_KangPo = 0;
		m_ChuanCi = 0;
		m_KangChuan = 0;
	}
};

struct SQASavePlayerInf : public BasePlayerCfgMsg
{
	SQASavePlayerInf()
	{
		BasePlayerCfgMsg::m_SecondFlag = BasePlayerCfgMsg::MSG_SAVEUPTPLAYER;
	}
};

struct SQSavePlayerInf : public SQASavePlayerInf
{
	BYTE m_School;
	BYTE m_Level;
	BYTE m_VIPLevel;
	DWORD m_PlayerGID;
	DWORD m_YinBi;
	DWORD m_JinBi;
	DWORD m_GloryDot;
	DWORD m_Sp;
	DWORD m_Hp;
	DWORD m_Tp;
	DWORD m_GongJi;
	DWORD m_FangYu;
	DWORD m_Hit;
	DWORD m_ShanBi;
	DWORD m_BaoJi;
	DWORD m_KangBao;
	DWORD m_PoJi;
	DWORD m_KangPo;
	DWORD m_ChuanCi;
	DWORD m_KangChuan;
};

struct SASavePlayerInf : public SQASavePlayerInf
{
	BYTE m_Result;	//1表示保存成功，0表示失败
};

struct SQATransfer : public BasePlayerCfgMsg
{
	SQATransfer()
	{
		BasePlayerCfgMsg::m_SecondFlag = BasePlayerCfgMsg::MSG_TRANSFORMPLAYER;
	}
};

struct SQTransfer : public SQATransfer
{
	DWORD m_PlayerGID;
	DWORD m_MapID;	//地图ID
	float m_XPos;
	float m_YPos;
};

struct SATransfer : public SQATransfer
{
	BYTE m_Result;	//1表示成功，0表示失败
};

struct SQAAddItem : public BasePlayerCfgMsg
{
	SQAAddItem()
	{
		BasePlayerCfgMsg::m_SecondFlag = BasePlayerCfgMsg::MSG_ADDITEM;
	}
};

struct SQAddItem : public SQAAddItem
{
	BYTE m_ItemNum;
	DWORD m_PlayerGID;
	DWORD m_ItemID;
};

struct SAAddItem : public SQAAddItem
{
	BYTE m_Result;
};

/////系统公告相关消息


struct SQARecordSyscallMsg : public BaseSyscallMsg
{
	SQARecordSyscallMsg()
	{
		BaseSyscallMsg::m_SecondFlag = BaseSyscallMsg::MSG_GETRECORDSYSCALL;
	}
};
//请求系统公告记录
struct SQRecordSyscallMsg : public SQARecordSyscallMsg
{

};

struct SyscallInf
{
	char m_Theme[64];
	DWORD m_SyscallID;
	DWORD m_RecordTime;
	DWORD m_StartTime;
	DWORD m_EndTime;
};

struct SARecordSyscallMsg : public SQARecordSyscallMsg
{
	BYTE m_SyscallSize;
	SyscallInf m_lpSyscallInf[100];
	SARecordSyscallMsg(BYTE SyscallInfSize) :m_SyscallSize(SyscallInfSize)
	{
		memset(m_lpSyscallInf, 0, sizeof(SyscallInf)* SyscallInfSize);
	}
};


#define THEMEMAX 64
#define SYSCALLDATAMAX 400

struct SQAAddSyscallMsg : public BaseSyscallMsg
{
	SQAAddSyscallMsg()
	{
		BaseSyscallMsg::m_SecondFlag = BaseSyscallMsg::MSG_ADDSYSCALL;
	}
};
//添加系统公告
struct SQAddSyscallMsg : public SQAAddSyscallMsg
{
	char m_Theme[THEMEMAX];
	char m_Data[SYSCALLDATAMAX];
	BYTE m_TimesPerTimes;
	WORD m_Interval;
	DWORD m_RecordTime;
	DWORD m_StartTime;
	DWORD m_EndTime;
	SQAddSyscallMsg()
	{
		memset(m_Theme, 0, THEMEMAX);
		memset(m_Data, 0, SYSCALLDATAMAX);
	}
};

struct SAAddSyscallMsg : public SQAAddSyscallMsg
{
	BYTE m_Result;	//0表示失败，1表示成功
	DWORD m_SyscallID;	//公告的ID
};

//更新系统公告
struct SQAUptSyscallMsg : public BaseSyscallMsg
{
	SQAUptSyscallMsg()
	{
		BaseSyscallMsg::m_SecondFlag = BaseSyscallMsg::MSG_UPTSYSCALL;
	}
};
struct SQUptSyscallMsg : public SQAUptSyscallMsg
{
	char m_Theme[THEMEMAX];
	char m_Data[SYSCALLDATAMAX];
	BYTE m_TimesPerTimes;
	WORD m_Interval;
	DWORD m_SyscallID;
	DWORD m_RecordTime;
	DWORD m_StartTime;
	DWORD m_EndTime;
	SQUptSyscallMsg()
	{
		memset(m_Theme, 0, THEMEMAX);
		memset(m_Data, 0, SYSCALLDATAMAX);
	}
};

struct SAUptSyscallMsg : public SQAUptSyscallMsg
{
	BYTE m_Result;	//0表示失败，1表示成功
};

//删除系统公告
struct SQADelSyscallMsg : public BaseSyscallMsg
{
	SQADelSyscallMsg()
	{
		BaseSyscallMsg::m_SecondFlag = BaseSyscallMsg::MSG_DELSYSCALL;
	}
};
struct SQDelSyscallMsg : public SQADelSyscallMsg
{
	DWORD m_SyscallID;
};

struct SADelSyscallMsg : public SQADelSyscallMsg
{
	BYTE m_Result;	//0表示失败，1表示成功
};


//通过公告ID获取公告内容
struct SQAGetContentByIDMsg : public BaseSyscallMsg
{
	SQAGetContentByIDMsg()
	{
		BaseSyscallMsg::m_SecondFlag = BaseSyscallMsg::MSG_GETCONTENTBYID;
	}
};
struct SQGetContentByIDMsg : public SQAGetContentByIDMsg
{
	DWORD m_SyscallID;
};

struct SAGetContentByIDMsg : public SQAGetContentByIDMsg
{
	char m_Content[SYSCALLDATAMAX];
};

//GM管理员登陆验证
struct SQALoginGMMsg : public BaseLoginMsg
{
	SQALoginGMMsg()
	{
		BaseLoginMsg::m_SecondFlag = BaseLoginMsg::MSG_GMLOGIN;
	}
};
struct SQLoginGMMsg : public SQALoginGMMsg
{
	char m_GMUserName[32];
	char m_GMUserPwd[16];
	SQLoginGMMsg()
	{
		memset(m_GMUserName, 0, 32);
		memset(m_GMUserPwd, 0, 16);
	}
};

struct SALoginGMMsg : public SQALoginGMMsg
{
	BYTE m_Result;
};


struct SQPlayerSaveOtherDataMsg : public SQSavePlayerInf
{

};

struct  SAPlayerFindOtherDataMsg : public SAPlayerBaseInf
{

};

struct SQPlayerFindOtherDataMsg : public SQSavePlayerInf
{

};

struct SAPlayerSaveOtherDataMsg : public SAPlayerBaseInf
{

};
