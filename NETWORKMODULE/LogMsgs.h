#pragma once

#include "NetModule.h"
#include "logtypedef.h"
#include "itemtypedef.h"
#include "playertypedef.h"
#include "../pub/ConstValue.h"
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 系统消息类
//=============================================================================================
DECLARE_MSG_MAP(SSysBaseMsg, SMessage, SMessage::EPRO_SYSTEM_MESSAGE)
//{{AFX
EPRO_LOGIN,	                // 登陆 检查帐号和密码
EPRO_CREATE_CHARACTER,      // 创建角色
EPRO_DEL_CHARACTER,         // 删除角色
EPRO_SELECT_CHARACTER,      // 选择角色
EPRO_LOGOUT,                // 退出
EPRO_CHARACTER_LIST_INFO,   // 设置客户端角色列表
EPRO_CHARACTER_DATA_INFO,   // 设置客户端主角资料
EPRO_LOGIN_NOCHECKACCOUNT,	// 不到帐号服务器检查（如从特殊服务器返回原服务器）
EPRO_ENGINE_FLAGE,          // 用于统计使用引擎 有调色版本设置为1,没有设置为0
EPRO_CLIENT_READY,          // 用于通知服务器客户端界面准备完毕
EPRO_SAVEPLAYER_CONFIG_INFO,          // 保存玩家电脑配置
EPRO_BLESSOpenClose_INFO,	// 祈福系统开关
EPRO_EVERYDAYSIGNIN_AWARD,	//每日签到奖励消息
EPRO_GET_SIGNINAWARD,	//领取每日签到奖励
EPRO_DATA_GETSID,			// 获取指定玩家SID
//}}AFX
END_MSG_MAP()

// 用户登陆，服务器接受玩家连接（简单版），数据中有用户版本号，账号，密码
DECLARE_MSG(SLoginMsg, SSysBaseMsg, SSysBaseMsg::EPRO_LOGIN)
struct SQLoginMsg : public SLoginMsg 
{
	WORD wVersion;			        // 版本信息
    char streamData[MAX_ACCAPASS];	// 账号&密码
};

// 接受登陆请求后的回应，成功或者失败信息在dwRetCode之中
struct SALoginMsg : public SLoginMsg
{
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
		ERC_LIMITED,                    // 登陆时间太短
		ERC_MAXCLIENTS,                 // 已经达到连接上限
		ERC_LOCKEDON_SERVERID,          // 此账号数据已经被其他游戏服务器锁定，拒绝重复登陆
		ERC_QUEUEWAIT,                  // 排队等待
		ERC_REUPDATE,                   // 重新更新

		ERC_SPLIT = 32,					// （分割标示）
		ERC_LOCKEDON_MOBILPHONE = 99,	// 该账号处于手机锁定状态，请拨打以下电话进行解锁。4008112096 或 4006567023
	};

	WORD	dwRetCode;					// 操作反馈信息
    WORD	wGMLevel;
    DWORD	dwEchoIP;					// 回显IP
	char	szDescription[MAX_USERDESC];

	SALoginMsg()
	{
		memset(szDescription, 0, sizeof(szDescription));
	}
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG( SNoCheckAccountLoginMsg, SSysBaseMsg, SSysBaseMsg::EPRO_LOGIN_NOCHECKACCOUNT )
struct SQLoginNoCheckAccountMsg : public SNoCheckAccountLoginMsg
{
	char streamData[1024];		// 保存 帐号 检测密码 服务器ID
};

DECLARE_MSG(SChListMsg, SSysBaseMsg, SSysBaseMsg::EPRO_CHARACTER_LIST_INFO)
struct SAChListMsg : public SChListMsg
{
    SCharListData CharListData[5];
};

//=============================================================================================
DECLARE_MSG(SLogoutMsg, SSysBaseMsg, SSysBaseMsg::EPRO_LOGOUT)
// 服务器接受玩家连接（简单版），数据中有用户版本号，账号，密码
struct SQLogoutMsg : public SLogoutMsg  // 无返回数据
{
    enum
    {
        EST_LOGOUT_NORMAL,  // 普通退出
        EST_LOGOUT_HANGUP,  // 离线挂机
    };

	DWORD dwGlobalID;		// 自己的全局ID编号
    BYTE byLogoutState;     // 退出的状态
};
//---------------------------------------------------------------------------------------------

// 创建角色
DECLARE_MSG(SCrePlayerMsg, SSysBaseMsg, SSysBaseMsg::EPRO_CREATE_CHARACTER)
struct SQCrePlayerMsg : public SCrePlayerMsg
{
    BYTE byIndex;
    SC2LCreateFixProperty PlayerData;
};

struct SACrePlayerMsg : public SCrePlayerMsg
{
    enum ERetErrCode
    {
        ERC_SUCCESS = 1,
        ERC_INDEXTAKEN,
        ERC_NAMETAKEN,
        ERC_UNHANDLE,
        ERC_EXCEPUTION_NAME,
    };

    BYTE byResult;
    BYTE byIndex;
    SC2LCreateFixProperty PlayerData;
};
//---------------------------------------------------------------------------------------------

// 删除角色
DECLARE_MSG(SDelPlayerMsg, SSysBaseMsg, SSysBaseMsg::EPRO_DEL_CHARACTER)
struct SQDelPlayerMsg : SDelPlayerMsg
{
    DWORD dwStaticID;
    char  szUserpass[CONST_USERPASS];
};

struct SADelPlayerMsg : SDelPlayerMsg
{
    enum ERetErrCode
    {
        ERC_SUCCESS = 1,
        ERC_UNHANDLE,
    };
    BYTE byResult;
    DWORD dwStaticID;
};

// 选择角色
DECLARE_MSG(SSelPlayerMsg, SSysBaseMsg, SSysBaseMsg::EPRO_SELECT_CHARACTER)
struct SQSelPlayerMsg : public SSelPlayerMsg
{
    BYTE byIndex;
};

// 选则角色的回应（简单版），成功或者失败信息在dwRetCode之中，角色具体数据还没有
struct SASelPlayerMsg : public SSelPlayerMsg 
{
	WORD dwRetCode;					// 操作反馈信息
	enum ERetCode
	{
		ERC_SELPLAYER_SUCCESS,	    // 登陆成功
        ERC_GETCHDATA_TIMEOUT,      // 向数据库发送获取角色资料消息超时
        ERC_SEND_GETCHDATA_FAIL,    // 向数据库发送获取角色资料消息失败
        ERC_GETCHDATA_FAIL,         // 向数据库获取角色资料失败
        ERC_PUTTOREGION_FAIL,       // 将角色放入具体场景失败
        ERC_PLAYER_FULL,            // 该服务器组已经达到人数上限
	};
};

DECLARE_MSG(SChDataMsg, SSysBaseMsg, SSysBaseMsg::EPRO_CHARACTER_DATA_INFO)
// 这个是服务器获得角色列表后通知客户端的消息
struct SAChDataMsg : public SChDataMsg
{
	DWORD	dwPlayerGlobalID;			// 本次运行全局唯一标识符
	DWORD	dwRegionGlobalID;			// 本次运行全局唯一标识符
    DWORD   dwPlayerStaticID;           // 玩家的StaticID
    //WORD	wCurMapID;                  // 当前场景图片编号
	SSanGuoPlayerFixData fixData; //玩家属性
	DWORD dwLastArenaChallengeTimeSpan;//竞技场时间间隔
	DWORD dwArenaChallengeCount; //竞技场挑战次数
	DWORD BuyChallengeCount; //竞技场挑战次数
	DWORD ReSetCountDownCount;//竞技场重置次数
	DWORD dwServerStartTime; //服务器开服时间
	DWORD dwBestArenaRank; //竞技场最佳排名
    //SXYD3FixData		PlayerBaseData;				// 玩家基本数据
	//SPlayerMounts		mountsData;					// 坐骑数据	
	//BYTE				taskInfo[MAX_WHOLE_TASK/8];	// 任务数据
	//SPlayerJingMai		venInfo;					// 经脉数据
	//SPlayerRelation		m_playerrelation;		//玩家好友数据
	//SPlayerPets			petsData;					// [2012-7-24 12-25 gw: +非战斗宠物数据]
};

DECLARE_MSG( SEngineFlage, SSysBaseMsg, SSysBaseMsg::EPRO_ENGINE_FLAGE )
struct SQEngineFlage : public SEngineFlage
{
	BYTE byEngineFlage;		
};

DECLARE_MSG( SClientReady, SSysBaseMsg, SSysBaseMsg::EPRO_CLIENT_READY )
struct SQClientReady : public SClientReady
{
			
};
DECLARE_MSG( SSavePlayerConfigInfo, SSysBaseMsg, SSysBaseMsg::EPRO_SAVEPLAYER_CONFIG_INFO )
struct SQSavePlayerConfigInfo : public SSavePlayerConfigInfo
{
	char	streamData[MAX_ACCOUNT];	// 账号
	char playerconfig[1024];	//玩家电脑配置
};
DECLARE_MSG( SBLESSOpenClose, SSysBaseMsg, SSysBaseMsg::EPRO_BLESSOpenClose_INFO )
struct SABLESSOpenClose : SBLESSOpenClose
{
	BYTE Flag;  // 1 是开 2是关
};


//每日签到奖励消息
DECLARE_MSG(SEveryDaySignInAwardMsg, SSysBaseMsg, SSysBaseMsg::EPRO_EVERYDAYSIGNIN_AWARD)
struct SAEveryDaySignInAwardMsg : public SEveryDaySignInAwardMsg
{
	BYTE m_EveryDaySignInIndex;  // 每日签到索引
	BYTE m_AwardState;	//该奖励是否已经领取的状态值；1可以领取，0不可以领取
};

//领取每日签到奖励
DECLARE_MSG(SGetSignInAwardMsg, SSysBaseMsg, SSysBaseMsg::EPRO_GET_SIGNINAWARD)
struct SQGetSignInAwardMsg : SGetSignInAwardMsg
{
	BYTE m_EveryDaySignInIndex;  // 每日签到索引
};
struct SAGetSignInAwardMsg : SGetSignInAwardMsg
{
	BYTE m_OptResult;  //领取结果。0失败，1成功
};

DECLARE_MSG(SGetsid, SSysBaseMsg, SSysBaseMsg::EPRO_DATA_GETSID)
struct SQGetsid : public SGetsid
{
	char  strname[CONST_USERNAME];  //请求查看的玩家名
};
struct SAGetsid : public SGetsid
{
	char  strname[CONST_USERNAME];//请求查看的玩家名
	DWORD  dsid;					//对应的SID
};