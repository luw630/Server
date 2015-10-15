#pragma once
#include "NetModule.h"
#include "../pub/ConstValue.h"

// 账号数据库操作规则
// 登陆时会检测账号 [EPRO_CHECK_ACCOUNT] ，由于仅当账号开始记费时才认为客户端连接有效，所以检测成功的同时账号开始记费（实际上是记录登陆时间，等到退出的时候改数据库扣点）
// 记费过程中，随时可以发消息 [EPRO_REFRESH_CARD_POINT] 到账号服务器进行数据刷新，也就是根据游戏时间进行扣点，同时游戏时间清零，如果该账号还未启动记费，则自动开始记费
// 记费过程中，账号服务器随时可以询问游戏服务器 [ ] 某个账号是否仍然存在，如果询问失败（游戏服务器故障或该账号不存在于游戏服务器），记费停止
// 玩家退出时，游戏服务器通知账号服务器 [EPRO_ACCOUNT_LOGOUT] 账号退出， 记费停止

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 账号服务器相关消息类
//=============================================================================================
DECLARE_MSG_MAP(SAccountBaseMsg, SMessage, SMessage::EPRO_ACCOUNT_MESSAGE)
//{{AFX
EPRO_REGISTER_LOGINSERVER,
EPRO_CHECK_PASSWORD,        // 检查帐号密码
EPRO_CHECK_ACCOUNT,         // 检测账号并做登入
EPRO_ACCOUNT_LOGOUT,        // 账号退出
EPRO_REFRESH_CARD_POINT,    // 点数刷新
EPRO_PLAYER_ADD_ACCOUNT,    // ...
EPRO_FORCE_LOGOUT,
//GM功能
EPRO_CHANGE_PASSWORD,       // 改玩家密码
ERPO_BLOCK_ACCOUNT,
EPRO_GETONLINETIME,
EPRO_GMCHECK_ACCOUNT,
EPRO_GMLOGOUT,
EPRO_EXCHANGECARDPOINT,     // 玩家交易点数
EPRO_ACCOUNT_HANGUP,        // 玩家开始挂机
EPRO_REFRESH_MONEY_PIONT,	// 元宝积分刷新
EPRO_GET_ACCOUNT_MDC_ID,    // 获取玩家在北京移数通电讯有限公司中申请的ID号
EPRO_QUERY_ENCOURAGEMENT, //抽奖查询
EPRO_CHECK_PLAYEREXIST,
EPRO_UPDATE_SERVER_STATE,   // 更新服务器状态
EPRO_UPDATE_TIMELIMIT,      // 时间更新数据,用于防沉迷系统!
//}}AFX
END_MSG_MAP_WITH_ROTOC()

DECLARE_MSG( _SUpdateServerState, SAccountBaseMsg, SAccountBaseMsg::EPRO_UPDATE_SERVER_STATE )
struct SUpdateServerState :
    public _SUpdateServerState
{
    DWORD ServerID;
    DWORD PlayerNumber;
    DWORD ServerState;
	//10-06-08
	WORD  TotalMac;       // MAC连接数
	//扩展参数
	char streamData[ MAX_ONLINEEXTEND ];
};

DECLARE_MSG(SGMAccountLogoutBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_GMLOGOUT)
struct SQGMAccountLogoutMsg :
    public SGMAccountLogoutBaseMsg
{
    // 账号，密码
	char streamData[ MAX_ACCAPASS ];
};

struct SAGMAccountLogoutMsg :
    public SGMAccountLogoutBaseMsg
{
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

    BYTE byResult;
};



//=============================================================================================
// 用于账号检测的消息，通过账号名&账号密码来判断是否为合法用户，同时返回相关数据（剩余点数）
DECLARE_MSG(SGMCheckAccountBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_GMCHECK_ACCOUNT)
struct SQGMCheckAccountMsg :
    public SGMCheckAccountBaseMsg
{
    int  iServerID;
    char szIP[17];

    // 账号，密码
	char streamData[ MAX_ACCAPASS ];
};

struct SAGMCheckAccountMsg :
    public SGMCheckAccountBaseMsg
{
    enum
    {
        RET_SUCCESS,                // 成功
        RET_INVALID_ACCOUNT,        // 无效的账号
        RET_INVALID_PASSWORD,       // 无效的密码
        RET_NOCARDPOINT,            // 点卡值不足
        RET_LOGGEDIN,               // 已经处于登录状态
        RET_BADINFO,                // 异常信息
        RET_BLOCKED,
    };

    BYTE byResult;
    DWORD dwCardPoint;              // 剩余的点数
	//DWORD dwMoneyPoint;				// 元宝积分

	// 这里增加ADDHERE
	//DWORD dwXYDPoint;
    QWORD qwPuid;                   // 帐号的唯一ID！之所以替换 dwMoneyPoint 和 dwXYDPoint 而不是 dwGiftCoupon 和 dwXYDPoint 是为了解决数据对齐的问题！
	DWORD dwGiftCoupon;

    WORD  wGMLevel;

    // 账号，密码，2级密码
	char streamData[ MAX_ACCAPASS + CONST_USERPASS ];
};
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseBAMsg,SAccountBaseMsg,SAccountBaseMsg::ERPO_BLOCK_ACCOUNT)
struct SQBAMsg : 
    public SBaseBAMsg
{
    DWORD dwBlockTime;

    // 账号
	char streamData[ MAX_ACCOUNT ];
};

struct SABAMsg :
    public SBaseBAMsg
{
    DWORD dwBlockTime;
    WORD  wResult;
    enum 
    {
        RT_SUCCESS,
        RT_NOACCOUNT,
        RT_ERROR
    };

    // 账号
	char streamData[ MAX_ACCOUNT ];
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseGOTMsg,SAccountBaseMsg,SAccountBaseMsg::EPRO_GETONLINETIME)
struct SQGOTMsg :
    public SBaseGOTMsg
{
    // 账号
	char streamData[ MAX_ACCOUNT ];
};
struct SAGOTMsg :
    public SBaseGOTMsg
{
    int iOnlineTime;
    WORD  wResult;
    enum 
    {
        RT_SUCCESS,
        RT_NOACCOUNT,
        RT_ERROR
    };

    // 账号
	char streamData[ MAX_ACCOUNT ];
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SChangePasswordMsg,SAccountBaseMsg,SAccountBaseMsg::EPRO_CHANGE_PASSWORD)
struct SQChangePasswordMsg :
    public SChangePasswordMsg
{
    // 账号，密码，新密码
	char streamData[ MAX_ACCOUNT + MAX_PASSWORD*2 ];
};

struct SAChangePasswordMsg :
    public SChangePasswordMsg
{
    WORD wResult;
    enum
    {
        RT_SUCCESS,
        RT_WRONGPASS,
        RT_ERROR    
    };
};

DECLARE_MSG(SCheckPasswordMsg,SAccountBaseMsg,SAccountBaseMsg::EPRO_CHECK_PASSWORD)
struct SQCheckPasswordMsg :
    public SCheckPasswordMsg
{
    // 账号
	char streamData[ MAX_ACCOUNT ];
};

struct SACheckPasswordMsg :
    public SCheckPasswordMsg
{
    WORD wResult;
};


DECLARE_MSG(SForceLogoutMsg,SAccountBaseMsg,SAccountBaseMsg::EPRO_FORCE_LOGOUT)
struct SQForceLogoutMsg :
    public SForceLogoutMsg
{
    // 账号，密码
	char streamData[ MAX_ACCAPASS ];
};

struct SAForceLogoutMsg :
    public SForceLogoutMsg
{
    WORD wResult;
};


//=============================================================================================
// 在账号服务器上注册登录服务器
DECLARE_MSG(SRegisterLogsrvBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_REGISTER_LOGINSERVER)
struct SQRegisterLogsrvMsg :
    public SRegisterLogsrvBaseMsg
{
    DWORD IP;
    DWORD ID;
};

struct SARegisterLogsrvMsg :
    public SRegisterLogsrvBaseMsg
{
    enum
    {
        RET_SUCCESS,
        RET_BADINFO,
    };

    BYTE byResult;
};
//---------------------------------------------------------------------------------------------

// 用于账号检测的消息，通过账号名&账号密码来判断是否为合法用户，同时返回相关数据（剩余点数）
DECLARE_MSG(SCheckAccountBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_CHECK_ACCOUNT)
struct SQCheckAccountMsg : public SCheckAccountBaseMsg
{
    int  iServerID; 
    char szIP[17];

    // 固定参数     ： 账号，密码，macAddress
    // 扩展参数写法 ： 在固定参数后附加键值对（键必须为字符串，完整的存储过程参数比如 @PlayerAccount， 值为对应数据类型）
	char streamData[MAX_ACCAPASS];
};

struct SACheckAccountMsg : public SCheckAccountBaseMsg
{
    enum
    {
        RET_SUCCESS,                // 成功
        RET_INVALID_ACCOUNT,        // 无效的账号
        RET_INVALID_PASSWORD,       // 无效的密码
        RET_NOCARDPOINT,            // 点卡值不足
        RET_LOGGEDIN,               // 已经处于登录状态
        RET_BADINFO,                // 异常信息
        RET_BLOCKED,                // 被封帐号
        RET_LIMITED,                // 被限制的账号
        RET_SUCCESS_WITH_ITEM,      // 成功并且有附加道具存在
		RET_LOGIN_OTHER_SERVER      // 已在其它服务器登录
    };

	SACheckAccountMsg()
	{
		dwCardPoint = 0;              // 剩余的点数
        qwPuid = 0;
		dwGiftCoupon = 0;
	}

    BYTE	byResult;
    DWORD	dwCardPoint;              // 剩余的点数
    QWORD	qwPuid;                   // 帐号的唯一ID
	DWORD	dwGiftCoupon;
    WORD	wGMLevel;

    // 账号，密码，2级密码，附加文字描述
	char streamData[ MAX_ACCAPASS + CONST_USERPASS + MAX_USERDESC ];
};


//=============================================================================================
// 用于检测角色对象是否存在的消息
DECLARE_MSG(SCheckPlayerExistBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_CHECK_PLAYEREXIST)
struct SQCheckPlayerExistBaseMsg :
    public SCheckPlayerExistBaseMsg
{
	SQCheckPlayerExistBaseMsg()
	{
		iDestServerID = 0;
	}

	int  iDestServerID;

    // 账号，密码
	char streamData[ MAX_ACCOUNT ];
};

struct SACheckPlayerExistBaseMsg :
    public SCheckPlayerExistBaseMsg
{
	SACheckPlayerExistBaseMsg()
	{
		iSrcServerID = 0;
		iDestServerID = 0;
		wExist = 1;
	}

    int  iSrcServerID;
	int  iDestServerID;
	WORD wExist;	// 0未找到， 1对象已存在

    // 账号
	char streamData[ MAX_ACCOUNT ];
};

//=============================================================================================
// 用于刷新目标账户的剩余点数
DECLARE_MSG(SRefreshCardBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_REFRESH_CARD_POINT)
struct SQRefreshCardMsg :
    public SRefreshCardBaseMsg
{
    enum
    {
        QUEST_REFRESH,      // 请求点数刷新！
        QUEST_DEL,          // 请求扣点！
        QUEST_EXCHANGE,     // 请求准备交易！
        QUEST_ACCHANGE,     // 请求确认交易！
        QUEST_CHKITEM,      // 请求准备附加道具！
        QUEST_ACCITEM,      // 请求确认附加道具！
        // 交易过程描述：先通过请求交易确认点数，返回操作码！（账号服务器log请求交易记录和操作码！）
        // 然后通过确认交易，判断操作码，将点数传入指定账号！（区域服务器log确认交易记录和操作码！）
    };

    BYTE byRefreshType;     // 刷新方式
    int iCardPoint;         // 改变的点数
    DWORD dwActionCode;     // 操作码（回传）

    // 账号
	char streamData[ MAX_ACCOUNT ];
};

struct SARefreshCardMsg :
    public SRefreshCardBaseMsg
{
    enum
    {
        QUEST_REFRESH,      // 请求点数刷新！
        QUEST_DEL,          // 请求扣点！
        QUEST_EXCHANGE,     // 请求交易！
        QUEST_ACCHANGE,     // 确认交易！
        QUEST_CHKITEM,      // 请求准备附加道具！
        QUEST_ACCITEM,      // 请求确认附加道具！
        // 交易过程描述：先通过请求交易确认点数，返回操作码！（账号服务器log请求交易记录和操作码！）
        // 然后通过确认交易，判断操作码，将点数传入指定账号！（区域服务器log确认交易记录和操作码！）
    };

    enum 
    {
        RET_SUCCESS,            // 操作成功
        RET_INVALID_ACCOUNT,    // 无效的账号
        RET_POINT_NOTENOUGH,    // 没有足够的点卡
        RET_BADINFO,            // 操作失败，不带信息的返回
        RET_ITEM_NOTEXIST,      // 不存在附加道具
    };

    BYTE byResult;          // 操作结果
    BYTE byRefreshType;     // 刷新方式
    DWORD dwCardPoint;      // 剩余点数

	// 这里增加ADDHERE

    DWORD dwActionCode;     // 操作码（回传）

    // 账号
	char streamData[ MAX_ACCOUNT ];
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 玩家之间
// EPRO_EXCHANGECARDPOINT
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 通知账号服务器，某个账号退出游戏，需要停止记费
DECLARE_MSG(SAccountLogoutBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_ACCOUNT_LOGOUT)
struct SQAccountLogoutMsg :
    public SAccountLogoutBaseMsg
{
    // 账号，密码
	char streamData[ MAX_ACCAPASS ];
};

struct SAAccountLogoutMsg :
    public SAccountLogoutBaseMsg
{
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

    BYTE byResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 通知账号服务器，某个账号退出游戏，需要停止记费
DECLARE_MSG(SAccountHangupBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_ACCOUNT_HANGUP)
struct SQAccountHangupMsg :
    public SAccountHangupBaseMsg
{
    BYTE byPoint;
    // 账号，密码
	char streamData[ MAX_ACCAPASS ];
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 创建一个账号
DECLARE_MSG(SAddAccountBaseMsg,SAccountBaseMsg,SAccountBaseMsg::EPRO_PLAYER_ADD_ACCOUNT)
struct SQAddAccountMsg : 
    public SAddAccountBaseMsg
{
    struct SNewPlayerInfo
    {
        char szName     [32];
        int  iSex ;
        int  iAge;
        char szAddr1 [64];
        char szCity  [32];
        char szProvince [32];
        char szCountry [32];
    };

    SNewPlayerInfo PlayerData;

    // 账号，密码
	char streamData[ MAX_ACCAPASS ];
};

struct SAAddAccountMsg :
    public SAddAccountBaseMsg
{
    enum ADD_ACCOUNT_RET
    {
        ADD_ACCOUNT_SUCESS,
        ADD_ACCOUNT_ERROR_ACCOUNTEXISTS,
        ADD_ACCOUNT_ERROR_NAMEEXISTS,
        ADD_ACCOUNT_BAD_INFO
    };

    BYTE byResult;
};
//---------------------------------------------------------------------------------------------

//
enum ADD_CARDPOINT_RET
{
    ADD_CARDPOINT_SUCCESS,
    ADD_CARDPOINT_ERROR_NOACCOUNT,
    ADD_CARDPOINT_ERROR_INGAME,//the player is in the game ,cannot add card point
    ADD_CARDPOINT_ERROR_BAD_INFO,
    ADD_CARDPOINT_UNHANDLED
};


enum PRODUCT_REGIST_RET // Player Regist product return value
{
    PROREG_SUCCESS,
    PROREG_ERROR_NOPRODUCT,//the product
    PROREG_ERROR_TIMEAHEAD,//the product will be registed in the furture
    PROREG_BAD_INFO
};
//

//////////////////////////////////////////////
// 元宝积分刷新
//#include "MoneyPointDef.h"
DECLARE_MSG(SLRefreshMoneyPointMsg,SAccountBaseMsg,SAccountBaseMsg::EPRO_REFRESH_MONEY_PIONT)
struct SQLRefreshMoneyPointMsg : public SLRefreshMoneyPointMsg
{
	//SMoneyPointNotify mpn;
	enum { RT_QUEST, RT_REPLACE };
	DWORD dwRefType;
	DWORD dwMoneyPoint;

    // 账号
	char streamData[ MAX_ACCOUNT ];
};

struct SALRefreshMoneyPointMsg : public SLRefreshMoneyPointMsg
{
	enum { R_INVALID_ACCOUNT, R_NOTENOUGH_POINT, R_OK, R_COMPLETE}; 
	DWORD	dwRet;
	DWORD dwMoneyPoint;		

    // 账号
	char streamData[ MAX_ACCOUNT ];
};

DECLARE_MSG(SQueryEncouragementMsg,SAccountBaseMsg,SAccountBaseMsg::EPRO_QUERY_ENCOURAGEMENT)

struct SQQueryEncouragement :public SQueryEncouragementMsg
{
    char m_szName[CONST_USERNAME];
    char m_szID[20];
};

struct SAQueryEncouragement:public SQueryEncouragementMsg
{
	char m_szName[CONST_USERNAME];
	BYTE m_iRet;
	enum
	{
        RET_INVALIDCARD = 2,
	    RET_ISUSEED =3,
	    RET_WEEKOK = 101,		//包周OK
	    RET_MONTH = 102 //包月OK
	};
};

//////////////////////////////////////////////

DECLARE_MSG( SUpdateTimeLimit, SAccountBaseMsg, SAccountBaseMsg::EPRO_UPDATE_TIMELIMIT )

struct SQUpdateTimeLimit : public SUpdateTimeLimit
{
    // str key
    char streamBuffer[256];
};
