#include "NetModule.h"
//#include "PlayerTypedef.h"
#include "HeroTypeDef.h"
#pragma once
DECLARE_MSG_MAP(SRelationBaseMsg, SMessage, SMessage::EPRO_RELATION_MESSAGE)
//{{AFX
EPRO_CHARACTER_FRIENDLIST_INFO_ADD, // 玩家好友列表
EPRO_CHARACTER_FRIENDLIST_INFO_DEL, // 玩家好友列表
EPRO_FRIENDONLINE,                  // 好友上线通知
EPRO_REFRESH_ONLINE_FRIENDS,        // 在线好友列表刷新

//这里以下消息暂时不用理会
EPRO_USEITEMTO_FRIENDS,				// 对好友使用道具
EPRO_REGUEST_ADDFRIEND,				// 请求加入好友
EPRO_REGUEST_ROSERECOD,				// 请求取得送花记录

EPRO_FRIEND_FAIL_MSG, //好友失败消息
//}}AFX
END_MSG_MAP()



enum RelationType {
    RT_FRIEND,			//好友
    RT_BLACKLIST,		//黑名单
	RT_ENEMIES,		//仇人
	RT_MATE,				//结拜
	RT_TEMP,				//临时好友
};

enum DEGREEITEM
{
	ITEM_DEGREE_ROSE	= 8003000,    //无用于计算
	ITEM_RED_ROSE,					   //红玫瑰
	ITEM_BLUE_ROSE,						//蓝玫瑰
	ITEM_WHITE_ROSE,				    //白玫瑰
	ITEM_YELLOW_ROSE,			    //黄玫瑰
	ITEM_PURPLE_ROSE,				   //紫玫瑰
};

struct FriendData
{
	char  bFriendName[CONST_USERNAME];
	WORD wIconIndex;
	WORD wLevel;
	DWORD wFightPower;
	DWORD dwSid; //玩家Sid
	BYTE    byType;     // 0 好友 ,1 推荐 2 添加 
	BYTE   bonlinestate;	// 0 离线 1在线
	BYTE bySendFlag; //赠送体力标记
	BYTE byReceiveFlag;//领取体力标记
};
//=============================================================================================
DECLARE_MSG(SAddFriendMsg, SRelationBaseMsg, SRelationBaseMsg::EPRO_CHARACTER_FRIENDLIST_INFO_ADD) //增加到列表中
// 增加好友(扩充为所有关系)
struct SAAddFriendMsg : public SAddFriendMsg
{
	FriendData m_friendData;
//     char    szName[CONST_USERNAME];
// 	char		sHeadIcon[CONST_USERNAME];
//     BYTE    byType;     // 0 好友 ,1 推荐 2 添加 
// 	WORD  wLevle;		//等级
};
struct SQAddFriendMsg : public SAddFriendMsg
{
	DWORD dwFriendSid;
    char    szName[CONST_USERNAME];
	BYTE    byType; //0 好友 ,1 推荐 2 添加 5 请求全部，进入游戏后 ，7赠送好友体力， 8 领取好友赠送体力
};


DECLARE_MSG(SFriendFailMsg, SRelationBaseMsg, SRelationBaseMsg::EPRO_FRIEND_FAIL_MSG) //增加到列表中
struct SAFriendFailMsg : public SFriendFailMsg
{
	BYTE    byType; // 1 自己好友达到上限 2 对方好友达到上限 3 请求过期
	SAFriendFailMsg()
	{
		byType = 0;
	}
};


DECLARE_MSG(SAgreeAddFriendMsg, SRelationBaseMsg, SRelationBaseMsg::EPRO_REGUEST_ADDFRIEND)
struct SQAgreeAddFriendMsg: public SAgreeAddFriendMsg
{
	char    szName[CONST_USERNAME];
};
struct SAAgreeAddFriendMsg: public SAgreeAddFriendMsg
{
	char    szName[CONST_USERNAME];
    BYTE    byResult; // 0 不同意 ,1 同意
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
DECLARE_MSG(SDelFriendMsg, SRelationBaseMsg, SRelationBaseMsg::EPRO_CHARACTER_FRIENDLIST_INFO_DEL)
// 删除好友
struct SQDelFriendMsg : public SDelFriendMsg
{
    char    szName[CONST_USERNAME];
	BYTE    byType; //0 好友 ,1 黑名,2仇人
};
struct SADelFriendMsg : public SDelFriendMsg
{
    BYTE    byResult;
    char    szName[CONST_USERNAME];
	BYTE    byType; //0 好友 ,1 黑名,2仇人
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SFriendOnlineBaseMsg,SRelationBaseMsg, SRelationBaseMsg::EPRO_FRIENDONLINE)
struct SFriendOnlineMsg : SFriendOnlineBaseMsg	// 原来作用是更新好友在线状态，现改为包括更新好友杀人、被杀、升级、冲穴等状态
{
// 更新玩家好友状态
	enum FRIEND_STATE
	{
		FRIEND_STATE_ONLINE,	// 上线
		FRIEND_STATE_OUTLINE,	// 下线
		FRIEND_STATE_HANGUP,	// 挂机
		FRIEND_STATE_KILLOTHER, // 打败别人
		FRIEND_STATE_BEKILLED,	// 被别人打败
		FRIEND_STATE_LEVELUP,	// 升级
		FRIEND_STATE_PASSPULSE,	// 冲穴
		FRIEND_STATE_LEVELUPWITHITEM,	// 好友升级送道具 
		FRIEND_STATE_TIMEOUTWITHEXP,	// 在线时间得经验
		FRIEND_STATE_REFRESH,	// 刷新（亲密度）！
	}eState;

    char	cName[CONST_USERNAME];		 	// 好友
	BYTE    byType;			 	// 0 好友 ,1 黑名

	char	szOtherName[CONST_USERNAME]; 		// 或者打败（或者被打败）玩家名字
	WORD	byFriendLevel;	 		// 好友等级
	//char	szPulsePassed[20];			// 好友冲开的穴道索引

	//DWORD	dwgiveexp;			// 赠送的经验
	//DWORD   dwgiveItemID;		// 赠送的道具ID
	//SYSTEMTIME giveTime;			// 赠送经验的时间
	//WORD		wFriendCount;		//在线好友个数
   // WORD    wDearValue;			// 亲密度
};
struct SAFriendOnlineMsg : public SFriendOnlineBaseMsg	
{
	// 更新玩家好友状态
	enum FRIEND_STATE
	{
		FRIEND_STATE_ONLINE,	// 上线
		FRIEND_STATE_OUTLINE,	// 下线
		FRIEND_STATE_HANGUP,	// 挂机
		FRIEND_STATE_KILLOTHER, // 打败别人
		FRIEND_STATE_BEKILLED,	// 被别人打败
		FRIEND_STATE_LEVELUP,	// 升级
		FRIEND_STATE_PASSPULSE,	// 冲穴
		FRIEND_STATE_LEVELUPWITHITEM,	// 好友升级送道具
		FRIEND_STATE_TIMEOUTWITHEXP,	// 在线时间得经验
		FRIEND_STATE_REFRESH,	// 刷新（亲密度）！
	}eState;

		 char	cName[CONST_USERNAME];		 	// 好友
		 //DWORD	dwgiveexp;	// 增加的经验
		 //DWORD   dwgiveItemID;	// 赠送的道具ID
};


const int MAX_PEOPLE_NUMBER  = 100 ;	///最大数量
const int MAX_TEMP_NUMBER			= 50;		//临时好友数量。
DECLARE_MSG(SRefreshFriendsBaseMsg,SRelationBaseMsg,SRelationBaseMsg::EPRO_REFRESH_ONLINE_FRIENDS)
struct SRefreshFriendsMsg : SRefreshFriendsBaseMsg
{
	WORD wFriendCount; //好友数
	WORD wSendCount; //当前赠送次数
	WORD wReceiveCount; //当前领取次数
	WORD wSendLimit; //赠送次数上限
	WORD wReceiveLimit; //领取次数上限
	WORD wStaminaNum; //每次赠送体力
	FriendData fdata[MAX_PEOPLE_NUMBER];
// 	
};

//---------------------------------------------------------------------------------------------
DECLARE_MSG(SUseFriendItemMsg,SRelationBaseMsg, SRelationBaseMsg::EPRO_USEITEMTO_FRIENDS)
struct SQUseFriendItemMsg : SUseFriendItemMsg	// 对好友或者玩家使用道具
{
	WORD wCellPos; // 坐标
	WORD wItemNum;
	char	cName[CONST_USERNAME];		 	// 好友的名称
	BYTE    byType;			 	// 0 好友 ,1 黑名
};
struct SAUseFriendItemMsg : SUseFriendItemMsg	// 对好友或者玩家使用道具
{
	WORD wItemNum;		//道具数量
	DWORD wIndex ;			//道具ID
	char	cName[CONST_USERNAME];		 	// 赠送人名称
	BYTE    byType;			 	// 0 好友 ,1 黑名
	int	nDegree ;				//增加或减少的好友度
};


DECLARE_MSG(SGetRoseRecodMsg,SRelationBaseMsg, SRelationBaseMsg::EPRO_REGUEST_ROSERECOD)
struct SQGetRoseRecodMsg : SGetRoseRecodMsg	// 对好友或者玩家使用道具
{
	UINT64		m_PlayerRoseUpdata;//玩家当前送花状态值，这个值用于判断是否更新客户端
};
