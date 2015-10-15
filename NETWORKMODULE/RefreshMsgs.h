#pragma once

#include "NetModule.h"
#include "playertypedef.h"
#include "../pub/ConstValue.h"
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 服务器之间的数据刷新消息类
//=============================================================================================
DECLARE_MSG_MAP(SRefreshBaseMsg, SMessage, SMessage::EPRO_REFRESH_MESSAGE)
//{{AFX
EPRO_REFRESH_REGION_SERVER,     // 刷新区域服务器
EPRO_REFRESH_PLAYER,            // 刷新玩家数据
EPRO_REFRESH_RankList,			// 排行榜消息
EPRO_REFRESH_PLAYERLOGINCOMPLTE,			//玩家在区域服务器登陆完成
EPRO_REFRESH_PLAYERLOGINEXCEPTION,			//刷新异常的玩家信息
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 服务器数据刷新消息
DECLARE_MSG(SRefreshRegionServerMsg, SRefreshBaseMsg, SRefreshBaseMsg::EPRO_REFRESH_REGION_SERVER)
struct SQRefreshRegionServerMsg : 
    public SRefreshRegionServerMsg
{
    DNID dnidClient;// 本次刷新的连接校验码
    DWORD key;      // 本次刷新的服务器编号
    DWORD gid;      // 本次刷新的全局编号

    // 没有什么实质内容，请求场景刷新的时候同时刷新排行榜数据
    //SScoreTable Scores;

    bool bFullQuest;
};

struct SARefreshRegionServerMsg : 
    public SRefreshRegionServerMsg 
{
    DNID dnidClient;// 本次刷新的连接校验码
    DWORD key;      // 本次刷新的服务器编号
    DWORD gid;      // 本次刷新的全局编号

    // 具体的场景刷新数据
    DWORD   dwIP;                   // 用于接受外部联接的端口
    DWORD   dwPort;                 // 用于接受外部联接的端口
    WORD    wTotalPlayers;          // 说明该场景当前存在多少个玩家对象
    WORD    wPlayers[4];            // 4个状态的玩家数目！

    // 采集数据
    __int64 i64TotalSpendMoney;     // 消耗的金钱总额
    DWORD   dwAvePing;              // 平均ping值
    DWORD   dwInFlowPS;             // 每秒流量
    DWORD   dwOutFlowPS;            // 每秒流量
    DWORD   dwCreatedSpecItem;      // 产生的特殊物品数量
    DWORD   dwUseSpecItem;          // 消耗的特殊物品数量

    BYTE    byRegionNumber;         // 实际有效的场景个数
    SRegion aRegions[MAX_REGION_NUMBER + 10];          // 最大100个场景的有效数据

    int GetMySize()
    {
        if (byRegionNumber >= MAX_REGION_NUMBER) return 0;
        return sizeof(SARefreshRegionServerMsg) - sizeof(SRegion)*(MAX_REGION_NUMBER + 10 - byRegionNumber);
    }
};

// 这个新定义的消息，主要是用来作区域服务器通知登陆服务器，自己增加或删除了一些什么样的固定场景！
struct SARefreshRegionServerMsgEx : 
    public SRefreshRegionServerMsg 
{
    DNID dnidClient;        // 利用这个数据来作为效验码，为0xff12ff341ff56ff78的时候，就是新消息了
    char dataStream[1];     // 序列化数据流
};

//---------------------------------------------------------------------------------------------



//=============================================================================================
// 刷新玩家数据的消息，目前好像没有用到吧
DECLARE_MSG(SRefreshPlayerMsg, SRefreshBaseMsg, SRefreshBaseMsg::EPRO_REFRESH_PLAYER)

struct SARefreshPlayerMsg : 
    public SRefreshPlayerMsg 
{
    enum { ONLY_BACKUP, BACKUP_AND_SAVE, SAVE_AND_LOGOUT, BACKUP_AND_FLAG, ONLY_LOGOUT, SAVE_AND_HANGUP};
    BYTE storeflag;
    //DWORD gid;
	SFixData fixData;
    char streamData[MAX_ACCOUNT];
};
//---------------------------------------------------------------------------------------------

DECLARE_MSG(SRefreshRanklist,SRefreshBaseMsg,SRefreshBaseMsg::EPRO_REFRESH_RankList)
struct SQRefreshRanklist : public SRefreshRanklist
{
	//DWORD serverID;
	char  name[CONST_USERNAME];
	DWORD level;
};
struct SARefreshRanklist : public SRefreshRanklist
{
	
};

// 服务器玩家登录刷新
DECLARE_MSG(SPlayerLoginComplte, SRefreshBaseMsg, SRefreshBaseMsg::EPRO_REFRESH_PLAYERLOGINCOMPLTE)
struct SQPlayerLoginComplte :public SPlayerLoginComplte
{
	char  account[MAX_ACCOUNT]; //登录成功的玩家帐户
};

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////  ////////////////////////////////////////////  ////////
////// /////    ///////    ///////////  /////////////  /////////////////////////////  /////////
/////// /   /////////  ///  /////////  //////////////////////  /     //////////////  //////////
///////  ///////////        ////////     //////////  ////////  ////  /////////      ///////////
//////  ///////////  //////////////  ///  ////////  ////////  ////  ////////  ///  ////////////
/////  ////////////  ///  ////////  ///  ////////  ////////  ////  ////////  ///  /////////////
////  //////////////    /////////      /////////  ////////  ////  /////////      //////////////
///////////////////////////////////////////////////////////////////////////////////////////////

// 玩家的场景转移 与 玩家的场景服务器转移

// 这一部分需要客户端的配合
// 客户端需要有两个连接，一个用于保持上一个连接和作为激活连接，一个用于建立新的的重定向连接

// 玩家在场景的转移时，可能会发生目标场景不在当前场景服务器上的情况
// 这种时候就会出现场景服务器转移

// 首先，玩家所在的当前服务器（重定向始发服务器）会通知登陆服务器将该玩家将转移场景，并且发送重
//   定向相关数据（目标场景的ID），同时该玩家在始发服务器上处于失效状态（不能做任何操作，但是所
//   有数据都效）

// 接着，登陆服务器将该玩家的状态标志设为连接重定向状态，同时通知始发服务器场景转移相关的数据
//   （目标IP&PORT。。。）（被置为连接重定向状态的玩家对象会被放入实时监控队列，一定时间内没有
//   收到其他任何一个服务器传来的重定向认证，就将该玩家作断线处理）
// 然后，始发服务器通知客户端转移场景并且需要新建连接到目标服务器
// 当客户端连接到目标服务器后，目标服务器根据重定向请求向登陆服务器请求认证该玩家的连接重定向
// 登陆服务器收到重定向认证后，在本地查找目标玩家，如果没找到，说明本次重定向已经超时，反之，进
//   行重定向认证
// 如果重定向认证通过，那么客户端就重连成功了，将该玩家的重定向状态去掉，通知目标服务器认证成功
//   （启用新连接），通知始发服务器（断开连接）
// 如果重定向认证失败，那么通知始发服务器重新启用该目标连接（也可能会存在该目标在始发服务器上已
//   经消失的情况），并且通知目标服务器认证失败（断开连接）

// 以初次登陆为例：
// 客户端连接到登陆服务器
// 登陆服务器获取该玩家的数据
// [根据该玩家的数据开始场景服务器转移]
// 因为场景列表在本机，所以直接获得目标场景的相关数据
// -〉获取失败：通知客户端[没有目标场景的异常处理]
// 获取成功后，将该玩家设为连接重定向状态，向客户端发送重定向通知
// 客户端将当前连接设为失效状态，根据重定向数据向目标服务器建立连接
// -〉连接失败：通知始发服务器（登陆服务器），无法建立连接
// 连接成功后，向目标服务器发送重定向请求
// 目标服务器根据重定向请求，到登陆服务器进行认证
// -〉认证失败，目标服务器断开连接，始发服务器（登陆服务器）恢复原连接的有效性
// 认证成功，登陆服务器上的玩家重定向状态取消
// 
// 新的一个版本中，所有的操作如果出现失败的话，是没有出错处理的
// 必须等到超时才会有具体的反映
//=============================================================================================
DECLARE_MSG_MAP(_SRebindMsg, SMessage, SMessage::EPRO_REBIND_MESSAGE)
//{{AFX
EPRO_REBIND_REGION_SERVER,      // 场景重定向消息
EPRO_CHECK_REBIND,              // 重定向认证消息

EPRO_REBIND_LOGIN_SERVER,		// 服务器之间无缝连接（不退出客户端直接连接）
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 重定向操作消息，在服务器和客户端之间传递
DECLARE_MSG(SRebindMsg, _SRebindMsg, _SRebindMsg::EPRO_REBIND_REGION_SERVER)
// 本消息是登陆服务器发给客户端的，通知重连到新的场景服务器
struct SRebindRegionServerMsg : 
   public SRebindMsg        // 所有的重定向消息都是由服务器通知客户端的，客户端不能主动的请求连接重定向
{
    DWORD   ip;       // 重定向目标的地址
    DWORD   port;     // 重定向目标的端口
    DWORD   gid;      // 校验用全局关键字（这个可以不传，理由如上）

    // char    key[ACCOUNTSIZE];  // 校验关键字（这个可以不传，直接使用客户端保存下来的数据）

    char streamData[MAX_ACCOUNT];
};

// 本消息是客户端发给目标服务器的，通知目标服务器去登陆服务器校验
struct SQRebindMsg : public SRebindMsg        
{
	DWORD	gid;      // 校验用全局关键字（传递）
    char	streamData[MAX_ACCOUNT];
};

DECLARE_MSG( SRebindLoginBaseMsg, _SRebindMsg, _SRebindMsg::EPRO_REBIND_LOGIN_SERVER )
struct SARebindLoginServerMsg :
	public SRebindLoginBaseMsg
{
	enum { REBIND_NORMAL = 1, REBIND_UNUSUAL, };
	DWORD m_dwGID;
	DWORD m_GMLevel;
	DWORD m_dwType;
	char streamData[ 4096 ];
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// 重定向验证消息，在服务器和服务器之间传递
DECLARE_MSG(SCheckRebindMsg, _SRebindMsg, _SRebindMsg::EPRO_CHECK_REBIND)
// 本消息是区域服务器发给登陆服务器的，通知需要验证一个新的客户端
struct SQCheckRebindMsg : public SCheckRebindMsg
{
    DNID    dnidClient;     // 基本连接编号
    DWORD   gid;            // 校验用全局关键字
    char	streamData[MAX_ACCOUNT];
};

// 本消息是登陆服务器发给区域服务器的，通知认证结果，成功后会在本机（区域服务器上创建一个相对应的对象玩家）
struct SACheckRebindMsg : public SCheckRebindMsg
{
	DWORD   dwResult;       // 认证结果
	DNID    dnidClient;     // 基本连接编号
	DWORD   gid;            // 校验用全局关键字

    // 这个操作可能会引起再次重定向
	DWORD   ip;       // 重定向目标的地址
	DWORD   port;     // 重定向目标的端口
	WORD    wMapID;
	WORD    wRegionID;
	WORD    wCurX;
	WORD    wCurY;

    char streamData[MAX_ACCOUNT];
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 控制消息，这一部分需要保证操作的可靠性，所以使用了消息响应超时处理
DECLARE_MSG_MAP(SServerCtrlMsg, SMessage, SMessage::EPRO_SERVER_CONTROL)
//{{AFX
EPRO_PLAYER_DATA,               // 设置一个玩家的数据
EPRO_PUT_PLAYER_IN_REGION,      // 将一个玩家放入场景
EPRO_RESOLUTION_REGION,         // 根据场景转移的需要，在登陆服务器上获取相关转移数据
EPRO_GAMESERVER_REQUEST_RPCOP,	// 请求数据库进行RPC操作
EPRO_SCRIPT_TRANSFER,           // 脚本消息转发
//}}AFX
END_MSG_MAP_WITH_ROTOC()
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 本消息是区域服务器主动请求登陆服务器的，通知需要获取一个玩家的数据（一般是在场景服务器转移完成的时候，发送场景转移之前）
// 本来应该是Q&A结构的，结果换成了Get&Set…… 还好，差不多吧……
DECLARE_MSG(SPlayerDataMsg, SServerCtrlMsg, SServerCtrlMsg::EPRO_PLAYER_DATA)
struct SGetPlayerDataMsg : 
   public SPlayerDataMsg
{
    DNID    dnidClient;     // 基本连接编号

    //char    key[ACCOUNTSIZE];        // 校验关键字
    DWORD   gid;            // 校验用全局关键字

    char streamData[MAX_ACCOUNT];
};

struct SSetPlayerDataMsg : public SPlayerDataMsg
{
    DNID			dnidClient;     // 基本连接编号
    DWORD			gid;            // 校验用全局关键字
    SFixData	data;
    SPlayerTempData tdata;

    // 为了新功能而添加了这个参数，注意：
    // 因为这个数据是放在了最末尾的，而之前处理SSetPlayerDataMsg时是定长处理
    // 所以添加了这个数据后的消息应该可以兼容旧的版本
    char streamData[MAX_ACCOUNT];
};

//=============================================================================================
// 场景转移控制消息
DECLARE_MSG(SChangeRegionMsg, SServerCtrlMsg, SServerCtrlMsg::EPRO_PUT_PLAYER_IN_REGION)
struct SAChangeRegionMsg : 
   public SChangeRegionMsg
{
    DNID    dnidClient;     // 基本连接编号

    WORD    wRegionID;      // 这个暂时没有用，直接用玩家身上的数据来转移场景
    WORD    x, y;           // 同上
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 场景解析控制消息，区域服务器通过它来获得目标场景的位置
DECLARE_MSG(SResolutionRegionMsg, SServerCtrlMsg, SServerCtrlMsg::EPRO_RESOLUTION_REGION)
struct SQResolutionDestRegionMsg : 
    public SResolutionRegionMsg
{
    DNID dnidClient;     // 基本连接编号
    WORD wRegionID;
    WORD wX;
    WORD wY;
    DWORD dwGID;
    WORD wLevel;

    char streamData[MAX_ACCOUNT];
};

struct SAResolutionDestRegionMsg : 
    public SResolutionRegionMsg
{
    DWORD dwip;
    DWORD dwport;

    DNID dnidClient;     // 基本连接编号

    WORD wRegionID;
    WORD wMapID;
    WORD wX;
    WORD wY;

    DWORD dwGID;
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// 请求数据库进行RPC操作
DECLARE_MSG( SGameServerRPCOPMsg, SServerCtrlMsg, SServerCtrlMsg::EPRO_GAMESERVER_REQUEST_RPCOP )
struct SQGameServerRPCOPMsg : 
    public SGameServerRPCOPMsg
{
	enum { CHARACTER_DATABASE, ACCOUNT_DATABASE,DBCENTER_DATABASE,  };

    BYTE dstDatabase; // [角色数据库]/[账号数据库]
    char streamData[ 4096 ];
	LPVOID GetExtendBuffer() { return &dnidClient; };
};

struct SAGameServerRPCOPMsg : 
    public SGameServerRPCOPMsg
{
	enum { CHARACTER_DATABASE, ACCOUNT_DATABASE };

    BYTE srcDatabase; // [角色数据库]/[账号数据库]
    char streamData[ 4096 ];
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG( SGameScriptTransfreMsg, SServerCtrlMsg, SServerCtrlMsg::EPRO_SCRIPT_TRANSFER )
struct SAGameScriptTransfreMsg : 
    public SGameScriptTransfreMsg
{
    DWORD dbcId;                 // 要发向的服务器
    DWORD destSercerId;          // 目标服务期ID
    char streamData[ 4096 ];
};

//---------------------------------------------------------------------------------------------

// 接收来自DB->LOGIN->REGION的排行榜信息
DECLARE_MSG(SGetRankListFromDB, SMessage, SMessage::EPRO_GET_RANKLIST_FROM_DB)
struct SAGetRankListFromDB : public SGetRankListFromDB
{
 	long m_Num;							// 排行榜条数，最大为3000
 	NewRankList m_NewRankList[ALL_RANK_NUM];
};