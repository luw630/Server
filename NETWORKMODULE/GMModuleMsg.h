//////////////////////////////////////////
//FileName		    :   GMModuleMsg.h   // 
//FileAuthor		:	luou            //
//FileCreateDate	:	15:09 2004-2-5  //
//FileDescription	:	GM模块处理的消息//
//////////////////////////////////////////
#pragma once

#include "NetModule.h"
#include "chattypedef.h"
#include "itemtypedef.h"
#include "playertypedef.h"
#include "../pub/ConstValue.h"





///////////////////////////////////////////////////////////////////////////////////////////////
// GM相关消息类
///////////////////////////////////////////////////////////////////////////////////////////////

//=============================================================================================
DECLARE_MSG_MAP(SGMMMsg, SMessage, SMessage::EPRO_GMM_MESSAGE)
//{{AFX
EPRO_GM_TALKMASK,		//禁某人言
EPRO_GM_TALKUNMASK,
EPRO_GM_MOVETOHIM,
EPRO_GM_MOVEBYME,
EPRO_GM_MOVETOBORN,
EPRO_GM_SENDTALK,		//发送滚屏消息
EPRO_GM_ACCOUNTMASK,
EPRO_GM_ACCOUNTUNMASK,
EPRO_GM_CHECKPLAYER,	//查某人的背包，装备

EPRO_GM_CHANGENAME,
EPRO_GM_CHANGEMORE,
EPRO_CHECK_SB_INFO,		//查某人的资料（包括等级，地图位置，在线时间。。。）
EPRO_CHECK_ONLINE,		//检查在线人数
EPRO_GM_CHANGEPASS,		//改玩家密码
EPRO_GM_BLOCKACCOUNT,
EPRO_GM_GETONLINETIME,
EPRO_GM_LEVELDEF,		//GM等级不够

EPRO_GM_CHECKPLAYER_DB,
EPRO_GM_CHECKWAREHOUSE, // GM 查询玩家仓库的消息
EPRO_GM_LOGIN,			// GM 登入登录服务器的消息
EPRO_GM_LOGOUT,			// GM 登出登录服务器的消息
EPRO_GM_CHECKONLINE,	// GM 检查玩家是否在线
EPRO_UPDATE_SB_INFO,	// GM 更新玩家资料

EPRO_GM_PROBLEM,		//玩家提出问题
EPRO_GM_GETREQUEST,		//gm工具端提出取等待队列里的请求

EPRO_GM_SAYGLOBAL,		//GM发千里传音
EPRO_GM_CHECKCHAT,
EPRO_GM_WISPER,			//gm和玩家之间的密聊消息
EPRO_GM_CLOSE,			//关放监狱
EPRO_GM_CHECKSCORE,		//查询10大排行榜
EPRO_GM_CUT,			//gm踢玩家下线
EPRO_GM_PUBLICINFO,		//发公告消息
EPRO_GM_CHANGEPUBINFO,	//更改公告信息
EPRO_GM_CHECKONLINEGM,  //玩家查询在线gm
EPRO_GM_GETPING,		//取玩家ping值
EPRO_GM_REFPLAYERNUM,	//刷新玩家数量~~
EPRO_GM_TALKMASKSRV,	//禁言操作 因为多区域服务器操作 重新做了一个服务器间使用的禁言消息
EPRO_GM_GETMASKNAME,	//获取禁言名单~~
EPRO_GM_GETNAMELIST,	//获取角色名字列表
EPRO_GM_MOVETOSAFEPOINT,//传送玩家的角色到指定场景的安全点（坐标和场景在工具端指定）
EPRO_GM_VISIBLE,		//是否隐藏在线GM
EPRO_GM_CMD = 42,       // GM工具通知客户端执行操作
EPRO_GM_CHECKGMIDINF,   //验证GM身份信息

//}}AFX
END_MSG_MAP_WITH_ROTOC_GMLEVEL()


//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseGMCMD,SGMMMsg,SGMMMsg::EPRO_GM_CMD)
struct SQEXECGMCMD : public SBaseGMCMD
{
    char streamData[4096];
};

struct SAEXECGMCMD : public SBaseGMCMD
{
    char streamData[4096];
};

//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseGMTalkMsg,SGMMMsg,SGMMMsg::EPRO_GM_SENDTALK)
struct SQGMTalkMsg : public SBaseGMTalkMsg
{
    char szTalkMsg[MAX_CHAT_LEN];
};
struct SAGMTalkMsg : public SBaseGMTalkMsg
{
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseMovetoBornMsg,SGMMMsg,SGMMMsg::EPRO_GM_MOVETOBORN)
struct SQMovetoBornMsg : public SBaseMovetoBornMsg
{
    char szName[CONST_USERNAME];
};
struct SAMovetoBornMsg : public SBaseMovetoBornMsg
{
    char szName[CONST_USERNAME];
    BYTE byResult; 
    // 1 ,成功
    // 0 ,不在线，失败
};

//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseCheckPLOnlineMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHECKONLINE)
struct SQCheckPLOnlineMsg : public SBaseCheckPLOnlineMsg
{
    char szName[CONST_USERNAME];
    BYTE bType; // Check type , 0 Name ,1 Account

    // 账号
	char streamData[ MAX_ACCOUNT ];
};

struct SACheckPLOnlineMsg : public SBaseCheckPLOnlineMsg
{
    BYTE bOnline; // 1 , online 0 ,offline
};



//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseCheckWareHouseMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHECKWAREHOUSE)
struct SQCheckWareHouseMsg : public SBaseCheckWareHouseMsg
{
    // 账号
	char streamData[ MAX_ACCOUNT ];
};
struct SACheckWareHouseMsg : public SBaseCheckWareHouseMsg
{  
    WORD wResult; // 
    enum 
    {
        ERC_FAIL,
        ERC_SUCCESS
    };
    SFixStorage1 WarehouseI;
//    SFixStorage2 WarehouseII;
//    SFixStorage3 WarehouseIII;

    // 账号
	char streamData[ MAX_ACCOUNT ];
};


//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseCheckPlayerDatabaseMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHECKPLAYER_DB)
struct SQCheckPlayerDatabaseMsg : public SBaseCheckPlayerDatabaseMsg
{
    char szName[CONST_USERNAME];
};
struct SACheckPlayerDatabaseMsg : public SBaseCheckPlayerDatabaseMsg
{  
    WORD wResult;
    DWORD dwStaticID;
    SFixBaseData PlayerData;
    SFixPackage  PlayerPackage;
    char szIP[17];

    // 账号
	char streamData[ MAX_ACCOUNT ];
};


//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseGMLoginMsg,SGMMMsg,SGMMMsg::EPRO_GM_LOGIN)
struct SQGMLoginMsg : public SBaseGMLoginMsg
{
    // 账号，密码
	char streamData[ MAX_ACCAPASS ];
};
struct SAGMLoginMsg : public SBaseGMLoginMsg
{
    WORD wResult;
    WORD wGMLevel;
	enum ERetCode
	{
		ERC_LOGIN_SUCCESS,		        // 登陆成功
		ERC_INVALID_VERSION,	        // 非法的版本号
		ERC_INVALID_ACCOUNT,	        // 无效的账号
		ERC_INVALID_PASSWORD,	        // 错误的密码
        ERC_LOGIN_ALREADY_LOGIN,        // 此账号已登陆
        ERC_GETLIST_TIMEOUT,            // 获取角色列表超时
        ERC_GETLIST_FAIL,               // 获取角色列表失败
        ERC_CHECKACC_TIMEOUT,           // 账号检测超时
        ERC_SEND_GETCL_TO_DATASRV_FAIL, // 向数据库服务器发送获取列表消息失败
        ERC_SEND_CACC_TO_ACCSRV_FAIL,   // 向账号服务器发送账号检测消息失败
        ERC_ALREADYLOGIN_AND_LINKVALID, // 此账号已登陆，同时相应的连接还未失效
        ERC_ALREADYLOGIN_BUT_INREBIND,  // 此账号已登陆，同时相应的连接已失效，但是处于重定向连接中[BUG]
        ERC_NOTENOUGH_CARDPOINT,        // 此账号点数不足
        ERC_SERVER_UPDATE,              // 服务器更新中，暂时不能登陆
        ERC_LOGIN_ERROR,                // 登陆失败
        ERC_CREATE_TEST_CHARACTER,      // 试玩账号，直接通知进入角色创建画面
        ERC_BLOCKED,                    // 账号被停权
        ERC_CHECK_CHAT,                 // 查看玩家聊天消息
		ERC_IP_ERROR,					// ip验证失败
	};

    // 账号
	char streamData[ MAX_ACCOUNT ];
};

//=============================================================================================
DECLARE_MSG(SGMLogoutMsg, SGMMMsg, SGMMMsg::EPRO_GM_LOGOUT)
// 服务器接受玩家连接（简单版），数据中有用户版本号，账号，密码
struct SQGMLogoutMsg : public SGMLogoutMsg  // 无返回数据
{
    // 账号
	char streamData[ MAX_ACCOUNT ];
};
//---------------------------------------------------------------------------------------------
struct SAGMLogoutMsg : public SGMLogoutMsg  // 无返回数据
{
    BYTE byResult;
    enum
    {
        RET_SUCCESS,
        RET_INVALID_ACCOUNT,        // 无效的账号
        RET_INVALID_PASSWORD,       // 无效的密码
        RET_BADINFO,
        LOGOUT_ERROR_NOTLOGIN,      // Player Logout without login ,some thing wrong happend 
        LOGOUT_ERROR_NORIGHT,
        LOGOUT_ERROR_CATCHERROR,    // 
        LOGOUT_ERROR_NOSESSIONID,
        LOGOUT_ERROR_UNHANDLED
    };
};

//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseGMLevelDef,SGMMMsg,SGMMMsg::EPRO_GM_LEVELDEF)
struct SAGBLevelDef : public  SBaseGMLevelDef
{

};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseGetOnlineTimeMsg,SGMMMsg,SGMMMsg::EPRO_GM_GETONLINETIME)
struct SAGetOnlineTimeMsg : public SBaseGetOnlineTimeMsg 
{
    int iOnlineTime;
    WORD wResult;
    enum 
    {
        RT_SUCCESS,
        RT_NOACCOUNT,
        RT_ERROR
    };

    // 账号
	char streamData[ MAX_ACCOUNT ];
};
struct SQGetOnlineTimeMsg : public SBaseGetOnlineTimeMsg
{
    // 账号
	char streamData[ MAX_ACCOUNT ];
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseBlockAccountMsg,SGMMMsg,SGMMMsg::EPRO_GM_BLOCKACCOUNT)
struct SABlockAccountMsg : public SBaseBlockAccountMsg
{
    DWORD dwBlockDuration;
    WORD wResult;
    enum
    {
        RT_SUCCESS,
        RT_NOACCOUNT,
        RT_ERROR
    };

    // 账号
	char streamData[ MAX_ACCOUNT ];
};
struct SQBlockAccountMsg : public SBaseBlockAccountMsg
{
    DWORD dwBlockDuration;

    // 账号
	char streamData[ MAX_ACCOUNT ];
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseMoveTohimMsg,SGMMMsg,SGMMMsg::EPRO_GM_MOVETOHIM)
struct SAMoveTohimMsg : public SBaseMoveTohimMsg
{
    WORD wResult;
};

struct SQMoveTohimMsg : public SBaseMoveTohimMsg
{
    char szName[CONST_USERNAME];
};

DECLARE_MSG(SBaseCheckOnlineMsg,SGMMMsg,SGMMMsg::EPRO_CHECK_ONLINE)
struct SACheckOnlineMsg : public SBaseCheckOnlineMsg
{
    DWORD dwOnlineCount;
    DWORD dwMaxCount;

};
struct SQCheckOnlineMsg : public SBaseCheckOnlineMsg
{
};

DECLARE_MSG(SBaseTalkMaskGMMsg, SGMMMsg, SGMMMsg::EPRO_GM_TALKMASK)
struct SATalkMaskGMMsg : public SBaseTalkMaskGMMsg
{
    char szName[CONST_USERNAME];
    DWORD dwTime;
    WORD wResult; // 0 失败，该玩家不在线
    // 1 成功
    // 2 失败，该玩家已经被禁言
};

struct SQTalkMaskGMMsg : public SBaseTalkMaskGMMsg
{
    char szName[CONST_USERNAME]; // 被禁言的玩家的名字
    DWORD dwTime;      // 被禁言的时间
};

DECLARE_MSG(SBaseTalkUnMaskGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_TALKUNMASK)
struct SATalkUnMaskGMMsg : public SBaseTalkUnMaskGMMsg
{
    char szName[CONST_USERNAME];
    WORD wResult; 
    // 1 成功
    // 2 失败
};

struct SQTalkUnMaskGMMsg : public SBaseTalkUnMaskGMMsg
{  
    char szName[CONST_USERNAME];
};

DECLARE_MSG(SBaseMovebyMeGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_MOVEBYME)
struct SAMovebyMeGMMsg : public SBaseMovebyMeGMMsg
{
    WORD wResult;  
};
struct SQMovebyMeGMMsg : public SBaseMovebyMeGMMsg
{
    char szName[CONST_USERNAME];
};

DECLARE_MSG(SBaseCheckSBInfo,SGMMMsg,SGMMMsg::EPRO_CHECK_SB_INFO)
struct SQCheckSBInfoGMMsg : public SBaseCheckSBInfo
{
    char szName[CONST_USERNAME];
};
struct SACheckSBInfoGMMsg : public SBaseCheckSBInfo
{
    WORD wResult;
    DWORD dwStaticID;
    SFixBaseData PlayerData;
    SFixPackage  PlayerPackage;
    SPlayerTasks PlayerTasks;
    char cszIP[17];
};

DECLARE_MSG(SBaseUpdateSBInfo,SGMMMsg,SGMMMsg::EPRO_UPDATE_SB_INFO)
struct SQUpdateSBInfoMsg : public SBaseUpdateSBInfo
{
    DWORD dwStaticID;
    SFixBaseData PlayerData;
};
struct SAUpdateSBInfoMsg : public SBaseUpdateSBInfo
{
    WORD wResult; // 0 失败 ， 1 成功
	char szName[CONST_USERNAME];
};

//-----------------------------------------------------------------------
//Add by LoveLonely 20040302
//-----------------------------------------------------------------------




DECLARE_MSG(SBaseMoveToSafePointGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_MOVETOSAFEPOINT)

struct SQMoveToSafePointGMMsg :public SBaseMoveToSafePointGMMsg
{
	char szName[CONST_USERNAME];
	int iRegion;
	int iX;
	int iY;
};

struct SAMoveToSafePointGMMsg :public SBaseMoveToSafePointGMMsg
{
	char szName[CONST_USERNAME];

	WORD wRet;
	enum
	{
		RET_SUCCESS, //成功
		RET_FAIL,	 //失败	
	};
};

DECLARE_MSG(SBaseGetMaskNameGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_GETMASKNAME)

struct SQGetMaskNameGMMsg :public SBaseGetMaskNameGMMsg
{
	WORD wPara;
	enum
	{
		PA_GAOL,
		PA_TALKMASK,
	};
};

struct SAGetMaskNameGMMsg :public SBaseGetMaskNameGMMsg
{
	char szName[20];

	WORD wPara;
	enum
	{
		PA_BEGIN,
		PA_SEND,
		PA_END,
	};

	WORD wResult;
	enum
	{
		RT_GAOL,
		RT_TALKMASK,
	};

};

DECLARE_MSG(SBaseTalkMaskSrvGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_TALKMASKSRV)

struct SQTalkMaskSrvGMMsg :public SBaseTalkMaskSrvGMMsg
{
	DWORD	dwTime;
	char	szName[CONST_USERNAME];

    // 账号
	char streamData[ MAX_ACCOUNT ];
};//登陆服务器发给区域服务器

DECLARE_MSG(SBaseRefPlayerNumGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_REFPLAYERNUM)

struct SARefPlayerNumGMMsg :public SBaseRefPlayerNumGMMsg
{
	int iPlayerNum;
	int iPlayerNotOnline;
	int iPlayerAuto;
};//只是服务器发给工具的。so 只有 A 消息

DECLARE_MSG(SBaseGetPingGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_GETPING)

struct SQGetPingGMMsg :public SBaseGetPingGMMsg
{
	int  iPing;
    DWORD ip32;
    QWORD mac64;
	WORD wPara;
	enum
	{
		PA_GET,
		PA_RETURN,
	};
};

struct SAGetPingGMMsg :public SBaseGetPingGMMsg
{
	int  iPing;
    DWORD ip32;
    QWORD mac64;
};

DECLARE_MSG(SBaseCheckOnlineGMGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHECKONLINEGM)

struct SQCheckOnlineGMGMMsg :public SBaseCheckOnlineGMGMMsg
{
	char szName[CONST_USERNAME];

	char szGM[10][CONST_USERNAME];

	WORD wPara;
	enum
	{
		PA_GET,
		PA_RETURN,
	};
};

DECLARE_MSG(SBaseChangePubInfoGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHANGEPUBINFO)

struct SQChangePubInfoGMMsg :public SBaseChangePubInfoGMMsg
{
	char szInfo[10][MAXPUBLICINFO];

	WORD wPara;
	enum
	{
		PA_GET,
		PA_CHANGE,
	};
};

struct SAChangePubInfoGMMsg :public SBaseChangePubInfoGMMsg
{
	char szInfo[10][MAXPUBLICINFO];

	WORD wRet;
	enum
	{
		RT_RETURN,
		RT_SUCCESS,
		RT_ERROR,
	};
};

DECLARE_MSG(SBasePublicInfoGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_PUBLICINFO)
struct SQPublicInfoGMMsg :public SBasePublicInfoGMMsg
{
	int GetMySize(){return sizeof(SQPublicInfoGMMsg)-(128-byLength);};

	BYTE	byLength;

	long	lClr;					//颜色
	int		nTime;					//间隔时间

	WORD	wType;
	enum
	{
		TP_CHANNEL,					//聊天频道显示
		TP_ROLL,					//滚动显示
	};

	char	szSay[128];				//内
};

struct SAPublicInfoGMMsg :public SBasePublicInfoGMMsg
{
	int GetMySize(){return sizeof(SQPublicInfoGMMsg)-(128-byLength);};

	BYTE	byLength;

	WORD	wType;

	enum
	{
		TP_CHANNEL,					//聊天频道显示
		TP_ROLL,					//滚动显示
	};

	WORD wResult;
	enum
	{
		RT_SUCCESS,
		RT_ERROR,
	};

	char	szSay[128];				//内容
};

DECLARE_MSG(SBaseCutGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CUT)
struct SQCutGMMsg  :public SBaseCutGMMsg
{
	char szGM[CONST_USERNAME];
	char szName[CONST_USERNAME];
};

struct SACutGMMsg  :public SBaseCutGMMsg
{
	char szName[CONST_USERNAME];

	WORD wResult;
	enum
	{
		RT_SUCCESS,
		RT_ERROR,
	};
};
//
//DECLARE_MSG(SBaseCheckScoreGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHECKSCORE)
//struct SQCheckScoreGMMsg  :public SBaseCheckScoreGMMsg
//{
//	char szGM[CONST_USERNAME];
//};
//
//struct SACheckScoreGMMsg  :public SBaseCheckScoreGMMsg
//{
//	SScoreTable s_ScoreTab;
//};



DECLARE_MSG(SBaseCloseGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CLOSE)
struct SQCloseGMMsg		: public SBaseCloseGMMsg
{
	char szName[CONST_USERNAME];
	char szGM[CONST_USERNAME];
	WORD wPara;
	enum
	{
		PARA_SETFREE,
		PARA_CLOSE,
	};
};

struct SACloseGMMsg		: public SBaseCloseGMMsg
{
	char szName[CONST_USERNAME];
	char szGM[CONST_USERNAME];
	WORD wResult;
	enum
	{
		RT_CLOSESUC,
		RT_SETFREESUC,
		RT_ERROR,
	};

};


DECLARE_MSG(SBaseSayGlobalGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_SAYGLOBAL)
struct SQSayGlobalGMMsg  : public SBaseSayGlobalGMMsg
{
	char _szSay[250];
	char szName[CONST_USERNAME];
};

struct SASayGlobalGMMsg	 : public SBaseSayGlobalGMMsg
{
	char _szSay[250];
	char szName[CONST_USERNAME];
	
	WORD wResult;
	enum
	{
		RT_SUC,
		RT_ERROR,	
	};

};

DECLARE_MSG(SBaseWisperGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_WISPER)
struct SQWisperGMMsg	:public SBaseWisperGMMsg
{
	SQWisperGMMsg()
	{
		dwConsumePoint = 0;
	}
	char szGM[CONST_USERNAME];
	char szPlayer[CONST_USERNAME];
	char szSay[MAX_CHAT_LEN];
	DWORD dwConsumePoint; //消费积份

    // 账号
	char streamData[ MAX_ACCOUNT ];
};

struct SAWisperGMMsg	:public SBaseWisperGMMsg
{
   SAWisperGMMsg()
	{
	dwConsumePoint = 0;
	}
	char szGM[CONST_USERNAME];
	char szPlayer[CONST_USERNAME];
	char szSay[MAX_CHAT_LEN];
	DWORD dwConsumePoint; //消费积份
	WORD wResult;
	enum
	{
		RT_ERROR,	
	};

    // 账号
	char streamData[ MAX_ACCOUNT ];
};


DECLARE_MSG(SBaseChangePassGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHANGEPASS)
struct SQChangePassGMMsg : public SBaseChangePassGMMsg
{
    // 账号，密码，新密码
	char streamData[ MAX_ACCOUNT + MAX_PASSWORD*2 ];
};
struct SAChangePassGMMsg : public SBaseChangePassGMMsg
{
    WORD wResult;
    enum
    {
        RT_SUCCESS,
        RT_WRONGPASS,
        RT_ERROR    
    };

    // 账号，密码，新密码
	char streamData[ MAX_ACCOUNT + MAX_PASSWORD*2 ];
};

DECLARE_MSG(SBasePlayerProblemGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_PROBLEM)
struct SQPlayerProblemGMMsg :public SBasePlayerProblemGMMsg
{
	char szName[CONST_USERNAME];
	char szRequest[MAX_CHAT_LEN];
	

	bool bIsRequest;		//true为首次请求，false为gm接受处理后的对话
	char szGMID[CONST_USERNAME];		//gmID 请求的时候为空
};
struct SAPlayerProblemGMMsg	:public SBasePlayerProblemGMMsg
{
	char szName[CONST_USERNAME];
	char szGMID[CONST_USERNAME];
	char szTalkText[MAX_CHAT_LEN];
	

	WORD wResult;
	enum
	{
		RT_TAKEON,			//接受处理
		RT_PASSUP,			//拒绝处理
		RT_NORMAL,			//处理ing
		RT_END,				//结束处理
		RT_NOGM,			//无gm在线
		RT_DROP,			//gm掉线
	};

};

DECLARE_MSG(SBaseGetRequestGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_GETREQUEST)
struct SQGetRequestGMMsg :public SBaseGetRequestGMMsg
{
	char szGMID[CONST_USERNAME];		//提出请求的gmID
};

struct SAGetRequestGMMsg :public SBaseGetRequestGMMsg
{
	char szName[CONST_USERNAME];	
	char szRequest[MAX_CHAT_LEN];

	WORD wResult;
	enum
	{
		RT_SUCCESS,			//成功
		RT_EMPTY,			//等待队列为空
	};
};
//-----------------------------------------------------------------------
//=============================================================================================
DECLARE_MSG(SBaseCheckChatGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHECKCHAT)
struct SQGMCheckChatGMMsg : public SBaseCheckChatGMMsg
{
    BYTE byOpr ; //0 停止接收，1开始接收
};
struct SAGMCheckChatGMMsg : public SBaseCheckChatGMMsg
{
    WORD wChatType;       // 说话的类型
    WORD wChatPara;       // 说话的参数（如系统消息分类）
    WORD wChatLen;        // 聊天信息长度
    WORD wGMLevel;        // GM等级
    char szNameTalker[CONST_USERNAME];// 说话的人
    char szNameRecver[CONST_USERNAME];// 接受的人（可能为空）
    char szMessage[MAX_CHAT_LEN];   // 说话的内容
};



DECLARE_MSG( SBaseVisibleGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_VISIBLE )
struct SQVisibleMsg : public SBaseVisibleGMMsg
{
	BYTE byOerate;	// 是否隐藏在线GM
	char szName[CONST_USERNAME];	// GM名字
};

struct SAVisibleMsg : public SBaseVisibleGMMsg
{
	BYTE byRet;		// 返回值
	char szName[CONST_USERNAME];	// GM名字
};

DECLARE_MSG( SBaseCheckGMInfGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHECKGMIDINF )
struct SACheckGMInfMsg : public SBaseCheckGMInfGMMsg            // 向KFD要GMID信息
{
    DWORD dwRandNumber;  // 随机数
};

struct SQCheckGMInfMsg : public SBaseCheckGMInfGMMsg            // 处理KFD返回信息
{
    char szName[CONST_USERNAME];	// GM名字
    char streamData[1024];	// 输出用户身份字符串
};
