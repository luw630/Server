#pragma once
#include "NetModule.h"
#include "logtypedef.h"
#include "playertypedef.h"
#include "../pub/ConstValue.h"
#include "tongmsgs.h"

// 游戏数据库操作规则
// 当登陆开始时，会有第一个游戏数据库操作：获取角色列表 [EPRO_GET_CHARACTER_LIST]
// 当玩家选择某角色后就需要去数据库服务器获取角色资料 [EPRO_GET_CHARACTER] 然后就开始了游戏的正常进行
// 在角色选择的过程中，可能会创建新的角色 [EPRO_CREATE_CHARACTER] ，也可能会删除某个已存在的角色 [EPRO_DELETE_CHARACTER]
// 游戏过程中，可能会随时要求数据库服务器保存某个角色的资料 [EPRO_SAVE_CHARACTER]

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 数据库服务器相关消息类
//=============================================================================================
DECLARE_MSG_MAP(_SDataBaseMsg, SMessage, SMessage::EPRO_DATABASE_MESSAGE)
//{{AFX
EPRO_GET_CHARACTER_LIST,    // 获取某个账号相关联的角色列表
EPRO_GET_CHARACTER,         // 获取某个角色的资料
EPRO_SAVE_CHARACTER,        // 保存某个角色的资料
EPRO_CREATE_CHARACTER,      // 新创建一个角色，同时改动该账号相关联的角色列表
EPRO_DELETE_CHARACTER,      // 删除某个已存在角色，同时改动该账号相关联的角色列表
EPRO_SAVE_TASK,             // 保存某个角色玩家的任务
EPRO_GET_TASK,
EPRO_SAVE_GM_LOG,           // 保存某个GM的操作
EPRO_GET_GM_LOG,            // 获取某个GM的操作
EPRO_GM_CHECKPLAYER,        // gm查看玩家资料
EPRO_GM_CHECKWAREHOUSE,     // 查看玩家的仓库
EPRO_GM_UPDATEPLAYER,       // 更新玩家资料
ERPO_SAVE_WAREHOUSE,
ERPO_GET_WAREHOUSE,
EPRO_GM_GETNAMELISTSRV,		//获取玩家角色列表]
EPRO_GET_ACCOUNTBYNAME,		//获取离线玩家的帐号
EPRO_SAVE_DBCENTERDATA,		//保存DBCenter过来的玩家保存数据
EPRO_SAVE_FACTION,			// 保存帮派数据
EPRO_GET_FACTION,			// 读取帮派数据
EPRO_SAVE_BUILDING,			//保存建筑物数据
EPRO_GET_BUILDING,			//读取建筑物数据
EPRO_DOUBLE_SAVE_PLAYER,     // 同步刷新2个玩家的数据；主要是为了解决道具交易；交换的问题
EPRO_SAVEPLAYER_CONFIG,			//保存玩家电脑配置
EPRO_DBGET_RankList,		//向数据库获取详细消息
EPRO_DB_Mail_SANGUO,		//数据库邮件
EPRO_WRITE_POINT_LOG,		//玩家消费日志
EPRO_DELETE_FACTION,			// 删除帮派数据
EPRO_SAVE_SCRIPTDATA,		//脚本数据保存
EPRO_GET_SCRIPTDATA,		//脚本数据保存

//}}AFX
END_MSG_MAP_WITH_ROTOC()

//=============================================================================================

// 保存帮派数据
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SSaveFactionMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_SAVE_FACTION)
struct SQSaveFactionMsg : public SSaveFactionMsg
{
	__int32  nServerID;
	DWORD nFactions;
	DWORD dwSendSize;
	BOOL  bSaveMode;		// T会做清除过期数据操作,F则相反
	int   nTime;			// 更新用的验证时间
	char streamFaction[sizeof(SaveFactionData_New)*MAX_SENDFACTIONS];
};

struct SQSaveFactionDataMsg : public SSaveFactionMsg
{
	__int32  nServerID;
	DWORD nFactions;
	DWORD dwSendSize;
	BOOL  bSaveMode;		// T会做清除过期数据操作,F则相反
	int   nTime;			// 更新用的验证时间
	char streamFaction[sizeof(SaveFactionData_Lua)*MAX_SENDFACTIONDATA];
};



// 获取帮派数据
DECLARE_MSG(SGetFactionMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_GET_FACTION)
struct SQGetFactionMsg : public SGetFactionMsg
{
	__int32  nServerId;
};

struct SAGetFactionMsg : public SGetFactionMsg
{
    BOOL  isLast;   // 如果为真，该消息表示数据库已经成功获取了所有帮派！
	DWORD nFactions;
	DWORD dwSendSize;
	char streamFaction[sizeof(SaveFactionData_New) * MAX_SENDFACTIONS];
};
struct SAGetFactionDataMsg : public SGetFactionMsg
{
	BOOL  isLast;   // 如果为真，该消息表示数据库已经成功获取了所有帮派！
	DWORD nFactions;
	DWORD dwSendSize;
	char streamFaction[sizeof(SaveFactionData_Lua) * MAX_SENDFACTIONDATA];
};

// 删除帮派数据
DECLARE_MSG(SDeleteFaction, _SDataBaseMsg, _SDataBaseMsg::EPRO_DELETE_FACTION)
struct SQDeleteFaction : public SDeleteFaction
{
	DWORD  nServerId;
	DWORD  dFactionID;
};

struct SADeleteFaction : public SDeleteFaction
{
	DWORD  nServerId;
	DWORD  dFactionID;
	BYTE bresult;
};



//-------------------------------------------------------------------------

//=========================建筑物数据保存===============================================
//
const WORD  MAX_BUILDING_BUF_SIZE	=	0xFF;
DECLARE_MSG(SDBBuildingSaveMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_SAVE_BUILDING )
struct SQDBBuildingSaveMsg : public SDBBuildingSaveMsg
{
	DWORD	dwServerId;	
	char	buffer[MAX_BUILDING_BUF_SIZE];
};


DECLARE_MSG(SDBBuildingGetMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_GET_BUILDING)
struct SQDBBuildingGetMsg : public SDBBuildingGetMsg
{
	DWORD	dwServerId;
	WORD	parentID;
};

struct SADBBuildingGetMsg : public SDBBuildingGetMsg
{
	QWORD	uniquid;	
	WORD	wparentID;
	WORD	itemIndex;	
	char streamFaction[MAX_BUILDING_BUF_SIZE];
	//inline void reset() { memset (this, 0, sizeof *this);}
};
//==========================建筑物数据保存==============================================


DECLARE_MSG(SBaseGetNameListSrvMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_GM_GETNAMELISTSRV)
struct SQGetNameListSrvMsg : public SBaseGetNameListSrvMsg
{
    // 账号
	char streamData[ MAX_ACCOUNT ];
};

struct SAGetNameListSrvMsg : public SBaseGetNameListSrvMsg
{
	SCharListData CharListData[5];
};

DECLARE_MSG(SBaseUpdatePlayerMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_GM_UPDATEPLAYER)
struct SQUpdatePlayerMsg : public SBaseUpdatePlayerMsg
{
    DWORD dwStaticID;
    SFixBaseData PlayerBaseData;
};
struct SAUpdatePlayerMsg : public SBaseUpdatePlayerMsg
{
    BYTE byResult;
	char szName[CONST_USERNAME];
};

//---------------------------------------------------------------------------------------------
//=============================================================================================
//GM检查玩家仓库资料
DECLARE_MSG(SBaseCheckWarehouseMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_GM_CHECKWAREHOUSE)
struct SQCheckWarehouseMsg : public SBaseCheckWarehouseMsg
{
    // 账号
	char streamData[ MAX_ACCOUNT ];
};
struct SACheckWarehouseMsg : public SBaseCheckWarehouseMsg
{
    WORD wResult;
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
//=============================================================================================
//GM检查玩家资料

DECLARE_MSG(SBaseCheckPlayerDBMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_GM_CHECKPLAYER)
struct SQCheckPlayerDBMsg : public SBaseCheckPlayerDBMsg
{
    char szName[CONST_USERNAME];
};

struct SACheckPlayerDBMsg : public SBaseCheckPlayerDBMsg
{
    WORD wResult;
    DWORD dwStaticID;
    SFixBaseData PlayerData;
    SFixPackage  PlayerPackage;

    // 账号
	char streamData[ MAX_ACCOUNT ];
};
//=============================================================================================
// 取得GM操作的消息

DECLARE_MSG(SGetGMLogMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_SAVE_GM_LOG)
struct SQGetGMLogMsg : 
    public SGetGMLogMsg
{
    SGMLog GmLog;
};

struct SAGetGMLogMsg  :
    public SGetGMLogMsg
{
    SGMLog GmLog;
};


//=============================================================================================
// 保存GM操作的消息

DECLARE_MSG(SSaveGMLogMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_SAVE_GM_LOG)
struct SQSaveGMLogMsg : 
    public SSaveGMLogMsg
{
    SGMLog GmLog;
};

struct SASaveGMLogMsg :
    public SSaveGMLogMsg
{
    SGMLog GmLog;
};

// 获取角色列表的消息
DECLARE_MSG(SGetCharacterListMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_GET_CHARACTER_LIST)
struct  SQGetCharacterListMsg : public SGetCharacterListMsg
{
    DWORD	dwServerId;					// 发起请求的服务器ID
	char	streamData[MAX_ACCOUNT];	// 账号
};

struct SAGetCharacterListMsg : public SGetCharacterListMsg
{
    enum
    {
        RET_SUCCESS,                // 操作成功
        RET_LOCKEDSELF,             // 已经被当前服务器ID锁定
        RET_LOCKEDOTHER,            // 已经被其他服务器ID锁定
        RET_ERROR_INFO,             // 操作失败
    };

    BYTE byResult;                  // 以上返回值
    SCharListData CharListData[5];  // 如果某个位的角色不存在，那么它相映位上的数据就为空
	char streamData[ MAX_ACCOUNT ]; // 账号
};
//---------------------------------------------------------------------------------------------

// 删除一个已经存在的角色，因为不会涉及到账号数据，所以不需要传递 szAccount szPassword
DECLARE_MSG(SDeleteCharacterMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_DELETE_CHARACTER)
struct SQDeleteCharacterMsg : public SDeleteCharacterMsg
{
    DWORD dwServerId;               // 发起请求的服务器ID
    DWORD dwStaticID;				// 要删除角色的ID

    // 账号
	char streamData[ MAX_ACCOUNT ];
};

struct SADeleteCharacterMsg : public SDeleteCharacterMsg
{
    BYTE byResult;
    DWORD dwStaticID;
    char szName[CONST_USERNAME];
};
//---------------------------------------------------------------------------------------------

// 创建一个新的角色，因为不会涉及到账号数据，所以不需要返回 szAccount szPassword
DECLARE_MSG(SCreatePlayerMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_CREATE_CHARACTER)
struct SQCreatePlayerMsg : public SCreatePlayerMsg
{
    DWORD	dwServerId;					// 发起请求的服务器ID
    BYTE	m_byIndex;
	SCreateFixProperty PlayerData;

	char	streamData[MAX_ACCOUNT];	// 账号
};

struct SACreatePlayerMsg : public SCreatePlayerMsg
{
    enum ERetErrCode
    {
        ERC_SUCCESS = 1,
        ERC_INDEXTAKEN,
        ERC_NAMETAKEN,
        ERC_UNHANDLE,
    };

    BYTE	byResult;
    BYTE	m_byIndex;
    SCreateFixProperty PlayerData;
};
//---------------------------------------------------------------------------------------------

// 获取一个角色的数据
DECLARE_MSG(SGetCharacterMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_GET_CHARACTER)
struct SQGetCharacterMsg : public SGetCharacterMsg
{
    DWORD	dwServerId;					// 发起请求的服务器ID
    BYTE	byIndex;					// 角色索引
	char	streamData[MAX_ACCOUNT];	// 账号
};

struct SAGetCharacterMsg : public SGetCharacterMsg
{
    enum ERetErrCode
    {
        ERC_SUCCESS,
		ERC_LOCKEDSELF,             // 已经被当前服务器ID锁定
		ERC_LOCKEDOTHER,            // 已经被其他服务器ID锁定
        ERC_FAIL,
    };

    BYTE			byResult;
    SFixData	PlayerData;					// 角色数据
	char			streamData[MAX_ACCOUNT];	// 账号
};
//---------------------------------------------------------------------------------------------
//获取一个角色的仓库数据
DECLARE_MSG(SGetWarehouseMsg, _SDataBaseMsg, _SDataBaseMsg::ERPO_GET_WAREHOUSE)
struct SQGetWarehouseMsg : public SGetWarehouseMsg
{
    DWORD dwServerId;               // 发起请求的服务器ID

    // 账号
	char streamData[ MAX_ACCOUNT ];
};

struct SAGetWarehouseMsg : public SGetWarehouseMsg
{
    SFixStorage1 PlayerWarehouseI;
//    SFixStorage2 PlayerWarehouseII;
//    SFixStorage3 PlayerWarehouseIII;

    // 账号
	char streamData[ MAX_ACCOUNT ];
};


//=============================================================================================
// 保存一个角色的资料
DECLARE_MSG(SSaveCharacterMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_SAVE_CHARACTER)
struct SQSaveCharacterMsg : public SSaveCharacterMsg
{
    DWORD dwServerId;               // 发起请求的服务器ID

    SFixData PlayerData;

    // 账号
	char streamData[ MAX_ACCOUNT ];
};

struct SASaveCharacterMsg : public SSaveCharacterMsg
{
    enum ERetErrCode
    {
        ERC_SUCCESS,
        ERC_FAIL,
    };

    BYTE byResult;

    // 账号
	char streamData[ MAX_ACCOUNT ];
};
//---------------------------------------------------------------------------------------------
// 保存角色的仓库资料
DECLARE_MSG(SSaveWarehouseMsg, _SDataBaseMsg, _SDataBaseMsg::ERPO_SAVE_WAREHOUSE)
struct SQSaveWarehouseMsg : public SSaveWarehouseMsg
{
    DWORD dwServerId;               // 发起请求的服务器ID

    SFixStorage1 PlayerWarehouseI;
//    SFixStorage2 PlayerWarehouseII;
//    SFixStorage3 PlayerWarehouseIII;

    // 账号
	char streamData[ MAX_ACCOUNT ];
};

//---------------------------------------------------------------------------------------------
// 通过名字来取离线玩家的帐号
DECLARE_MSG(SGetAccountByName, _SDataBaseMsg, _SDataBaseMsg::EPRO_GET_ACCOUNTBYNAME)
struct SQGetAccountByName : public SGetAccountByName
{
	char szQuestName[CONST_USERNAME];	
	char szName[CONST_USERNAME];

    // 账号
	char streamData[ MAX_ACCOUNT ];
};

struct SAGetAccountByName : public SGetAccountByName
{
	enum ERetErrCode
	{
		ERC_SUCCESS,
		ERC_FAIL,
	};

    SAGetAccountByName()
    {
	    memset(szQuestName,0,sizeof(szQuestName));
	    memset(szName,0,sizeof(szName));
    }
  
	BYTE byResult;
	char szQuestName[CONST_USERNAME];
	char szName[CONST_USERNAME];

    // szResultAccount，szAccount
	char streamData[ MAX_ACCOUNT * 2 ];
};

struct DBHeadData
{
 char account[MAX_ACCOUNT];
 DWORD Paramter[4]; 
};

DECLARE_MSG(SSaveDBCenterData, _SDataBaseMsg, _SDataBaseMsg::EPRO_SAVE_DBCENTERDATA)
struct SQSaveDBCenterData: public SSaveDBCenterData
{
enum ERetErrCode
	{
		ERC_SUCCESS,
		ERC_FAIL,
	};
	DBHeadData	msghead;
	SFixBaseData basedata;
};

struct SASaveDBCenterData: public SSaveDBCenterData
{
enum ERetErrCode
	{
		ERC_SUCCESS,
		ERC_FAIL,
	};
	BYTE byResult;
};

// 同步刷新2个玩家的数据；主要是为了解决道具交易；交换的问题
DECLARE_MSG( _SDoubleSavePlayerMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_DOUBLE_SAVE_PLAYER )

struct SDoubleSavePlayerMsg : 
    public _SDoubleSavePlayerMsg
{
	DWORD serverId;
    DWORD gid[2];
    char account[2][MAX_ACCOUNT];
    SFixData data[2]; // 这里可能只有一个数据有效,也可能是2个...
};

//相DB发送玩家电脑配置
DECLARE_MSG(SSavePlayerConfig, _SDataBaseMsg, _SDataBaseMsg::EPRO_SAVEPLAYER_CONFIG)
struct SQLSavePlayerConfig : public SSavePlayerConfig
{
	DWORD serverId;
	char	streamData[MAX_ACCOUNT];	// 账号
	char playerconfig[1024];	//玩家电脑配置
};

// 排行榜相关信息
DECLARE_MSG(SDBGETRankList,_SDataBaseMsg,_SDataBaseMsg::EPRO_DBGET_RankList)
struct SQDBGETRankList : public SDBGETRankList
{
	DNID	serverID;
	DWORD	level;
};
struct SADBGETRankList : public SDBGETRankList
{
	DNID serverID;
	long m_Num;							// 排行榜条数，最大为3000
	NewRankList m_NewRankList[ALL_RANK_NUM];
};


// 写玩家消费日志  20150408 wk
DECLARE_MSG(SPointLogMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_WRITE_POINT_LOG)


struct SQPointLogMsg : public SPointLogMsg
{	
	DWORD  uid;
	char account[MAX_ACCOUNT];//名字
	DWORD itype;//虚拟点数类型
	INT diff_value;//增加或减少点数
	DWORD finale_value;//操作后点数
	BYTE detailType;//细节类型,如 itype=2元宝时,detailType=1为积分商城,2为竞技场商城
	DWORD pInfo;//玩家信息,低3位为等级,高2位为vip等级,如22111,111级22级vip
	DWORD otherInfo;//目前道具类型时表示道具id

};


DECLARE_MSG(SScriptData, _SDataBaseMsg, _SDataBaseMsg::EPRO_SAVE_SCRIPTDATA)
struct SQScriptData : public SScriptData
{
	WORD   wLenth;
	BYTE    *pBuffer;
};

struct SAScriptData : public SScriptData
{
	WORD   wLenth;
	BYTE    *pBuffer;
};


DECLARE_MSG(SGetScriptData, _SDataBaseMsg, _SDataBaseMsg::EPRO_GET_SCRIPTDATA)
struct SQGetScriptData : public SGetScriptData
{
};

struct SAGetScriptData : public SGetScriptData
{
	WORD   wLenth;
	BYTE    *pBuffer;
};