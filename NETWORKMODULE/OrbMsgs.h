#pragma once

#include "NetModule.h"
#include "orbtypedef.h"
#include "chattypedef.h"
#include "../pub/ConstValue.h"

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ORB-SERVER（跨区域服务器）相关消息类，带有控制信息，可做超时检测
//=============================================================================================
DECLARE_MSG_MAP(SORBMsg, SMessage, SMessage::EPRO_ORB_MESSAGE)
//{{AFX
EPRO_CTRL_SIMPLAYER,    // 基本角色控制相关消息
EPRO_CTRL_TEAMGROUP,    // 组队控制相关消息
EPRO_CTRL_CHANNEL,      // 聊天频道相关消息
EPRO_CTRL_FACTION,        // 帮会控制相关消息
EPRO_CTRL_CHAT,         // 跨服务器的聊天消息
EPRO_CTRL_CHECK,        // 检测玩家是否在线的消息
EPRO_CTRL_FRIEND,       // 好友相关的消息
EPRO_CTRL_DATATRANS,    // 数据传送协议！
//}}AFX
END_MSG_MAP_WITH_ROTOC()
//---------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////
// EPRO_ADD_SIMPLAYER,     // 基本角色的添加
// 上面这个消息被去掉了，因为它的操作和EPRO_REFRESH_SIMPLAYER基本相同
//  所以EPRO_REFRESH_SIMPLAYER完全可以替代EPRO_ADD_SIMPLAYER
// EPRO_REFRESH_SIMPLAYER => EPRO_SYN_SIMPLAYER
///////////////////////////////////////////////////////////////////////////////////////////////
// 基本对象相关消息类
//=============================================================================================
DECLARE_MSG_MAP(SCtrlSimPlayer, SORBMsg, SORBMsg::EPRO_CTRL_SIMPLAYER)
//{{AFX
EPRO_DEL_SIMPLAYER,     // 基本角色的删除
EPRO_SYN_SIMPLAYER,     // 基本角色的刷新与添加

EPRO_QUERY_SIMPLAYER,   // 查询角色信息 added by yg
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------
DECLARE_MSG_MAP(SCtrlCheckMsg, SORBMsg, SORBMsg::EPRO_CTRL_CHECK)
//{{AFX
EPRO_CHECK_NAME,        // 用名字查找玩家是否在线
//}}AFX
END_MSG_MAP()

DECLARE_MSG_MAP(SCtrlFactionMsg,SORBMsg,SORBMsg::EPRO_CTRL_FACTION)
//{{AFX
EPRO_TRANSFER_FACTION,
//}}AFX
END_MSG_MAP()

DECLARE_MSG(STransferFactionMsg,SCtrlFactionMsg,SCtrlFactionMsg::EPRO_TRANSFER_FACTION)
struct SQTransferFactionMsg :
    public STransferFactionMsg
{
	char szFactionName[CONST_USERNAME];
    WORD wSize; // size of byMessage 
    DNID    dnidClientExp;  
    BYTE byMessage[2048*10];

    WORD GetMySize()
    {
        return (WORD)(sizeof(SQTransferFactionMsg)-(2048*10-wSize));
    };
    void SetMySize(WORD wSetSize)
    {
        wSize = wSetSize;
    };
};

struct SATransferFactionMsg :
    public STransferFactionMsg
{
	char szFactionName[CONST_USERNAME];
    WORD wSize;
    DNID    dnidClientExp;  
    BYTE byMessage[2048*10];
    WORD GetMySize()
    {
        return (WORD)(sizeof(SATransferFactionMsg)-(2048*10-wSize));
    };
    void SetMySize(WORD wSetSize)
    {
        wSize = wSetSize;
    };
};

DECLARE_MSG(SCheckNameMsg, SCtrlCheckMsg,SCtrlCheckMsg::EPRO_CHECK_NAME)
struct SQCheckNameMsg :
    public SCheckNameMsg
{
    char szMyName[CONST_USERNAME];
	char szFriendName[CONST_USERNAME];
    BYTE byType; // 0 好友 
};

struct SACheckNameMsg :
     public SCheckNameMsg
{
    char szName[CONST_USERNAME];
    BYTE byType; 
    BYTE byResult; // 0 在线 1 //不在线
};
//=============================================================================================
// 基本对象删除控制消息
DECLARE_MSG(SDelSimPlayerMsg, SCtrlSimPlayer, SCtrlSimPlayer::EPRO_DEL_SIMPLAYER)
struct SQDelSimPlayerMsg :
    public SDelSimPlayerMsg
{
    // 索引关键字
    DWORD   dwStaticID;

    // 定位关键字
    WORD    wServerID;          // 该玩家区域服务器ID
    DNID    dnidClient;         // 区域服务器上该玩家的DNID

    // 校验关键字
    DWORD   dwClientGID;        // 该玩家本次登录的全局关键字
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 基本对象数据刷新消息
DECLARE_MSG(SSynSimPlayerMsg, SCtrlSimPlayer, SCtrlSimPlayer::EPRO_SYN_SIMPLAYER)
struct SQSynSimPlayerMsg :
    public SSynSimPlayerMsg
{
    // 索引关键字
    DWORD   dwStaticID;

    // 定位关键字
    WORD    wServerID;          // 该玩家区域服务器ID
    DNID    dnidClient;         // 区域服务器上该玩家的DNID

    // 校验关键字
    DWORD   dwClientGID;        // 该玩家本次登录的全局关键字

    // 基本数据
    char	szName[CONST_USERNAME];         // 该玩家的名字[自己做末尾截断]
    WORD    wCurRegionID;       // 所在场景的ID
    DWORD   dwGMLevel;          // GM等级

	BYTE	bOnlineState;		// 0:离线  1:在线  2:挂机 
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 跨区域通过ORB查询玩家信息
DECLARE_MSG(SQuerySimPlayerMsg, SCtrlSimPlayer, SCtrlSimPlayer::EPRO_QUERY_SIMPLAYER)
struct SQQuerySimPlayerMsg :
    public SQuerySimPlayerMsg
{
	char szName[CONST_USERNAME];			// Player Name to query
    DNID dnidClient;			// 区域服务器上该玩家的DNID，当从区域向ORB发送时需赋值，客户端向区域发时不需
};
struct SAQuerySimPlayerMsg :
    public SQuerySimPlayerMsg
{
	enum OperateType
	{
		REQUEST,
		ANSWER,
	} stOperateType;

    WORD wServerID;          // 该玩家区域服务器ID
    DNID dnidClient;         // 区域服务器上该玩家的DNID
	char szName[CONST_USERNAME];
	bool bIsMan;
	char szTitle[16];
	char szTongName[CONST_USERNAME];
	BYTE byPKValue;
	BYTE byXValue;
};
//---------------------------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 队伍相关消息类
//=============================================================================================
DECLARE_MSG_MAP(SCtrlTeamGroup, SORBMsg, SORBMsg::EPRO_CTRL_TEAMGROUP)
//{{AFX


EPRO_CREATE_TEAM,           // 队伍的创建
EPRO_DELETE_TEAM,           // 队伍的删除

// 队伍的刷新
EPRO_SOMEONE_JOIN_TEAM,     // 某人加入队伍
EPRO_SOMEONE_LEAVE_TEAM,    // 某人离开队伍
EPRO_SOMEONE_KICKED_TEAM,   // 某人踢出队伍

// 队伍的查询&操作
EPRO_TEAM_OPERATION,        // 组队相关操作基类
EPRO_TEAM_PASSBYMESSAGE,

//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------
//=============================================================================================
DECLARE_MSG(STeamBasePassbyMsg,SCtrlTeamGroup,SCtrlTeamGroup::EPRO_TEAM_PASSBYMESSAGE)
struct SQTeamPassbyMsg :
    public STeamBasePassbyMsg,
    public SQTeamOperation
{
    DNID  dnidSrc;
    DNID  dnidDest;
    DWORD dwSrcGID;
    DWORD dwDestGID;
    DWORD dwSrcStaticID;
    DWORD dwDestStaticID;
};

struct SATeamPassbyMsg :
    public STeamBasePassbyMsg,
    public SATeamOperation
{
    WORD  wOprType;   // 操作类型 1.创建 2.申请加入 3.邀请加入 4.加入失败 0.失败
    DNID  dnidSrc;
    DNID  dnidDest;
    DWORD dwSrcGID;
    DWORD dwDestGID;
    DWORD dwSrcStaticID;
    DWORD dwDestStaticID;
    DWORD dwSrcTeamID;
    DWORD dwDestTeamID;
};



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 队伍查询相关操作消息类
//=============================================================================================
DECLARE_MSG_MAP(STeamOperationMsg, SCtrlTeamGroup, SCtrlTeamGroup::EPRO_TEAM_OPERATION)
//{{AFX
EPRO_TEAM_GET_MEMBER,       // 获取组队成员基本信息
EPRO_TEAM_TALK,             // 组队聊天
EPRO_TEAM_SHARE_EXP,        // 组队分经验
EPRO_TEAM_CHANGE_SKILL,     // 组队切换队形技
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 创建一个队伍
DECLARE_MSG(STeamCreateMsg,SCtrlTeamGroup,SCtrlTeamGroup::EPRO_CREATE_TEAM)
struct SQTeamCreateMsg :
    public STeamCreateMsg,
    public SQTeamOperation
{
    // 预留的ID作为另一位成员的标识
    DWORD dwFlag; // 组队创建旗标,包括创建队伍可能的一些选项
};

struct SATeamCreateMsg : 
    public STeamCreateMsg,
    public SATeamOperation
{
    DWORD   dwTeamID;
    WORD    wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 删除一个队伍
DECLARE_MSG(STeamDeleteMsg,SCtrlTeamGroup,SCtrlTeamGroup::EPRO_DELETE_TEAM)
struct SQTeamDeleteMsg :
    public STeamDeleteMsg,
    public SQTeamOperation
{

};

struct SATeamDeleteMsg : 
    public STeamDeleteMsg,
    public SATeamOperation
{
    WORD wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 某人加入队伍
DECLARE_MSG(STeamSomeoneJoinMsg,SCtrlTeamGroup,SCtrlTeamGroup::EPRO_SOMEONE_JOIN_TEAM)
struct SQTeamSomeoneJoinMsg:
    public STeamSomeoneJoinMsg,
    public SQTeamOperation
{
    //dwReserved 作为要加入的队伍编号
};

struct SATeamSomeoneJoinMsg:
    public STeamSomeoneJoinMsg,
    public SATeamOperation
{
    WORD wResult;
    WORD wTeamSkill;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 某人离开队伍
DECLARE_MSG(STeamSomeoneLeaveMsg,SCtrlTeamGroup,SCtrlTeamGroup::EPRO_SOMEONE_LEAVE_TEAM)
struct SQTeamSomeoneLeaveMsg:
    public STeamSomeoneLeaveMsg,
    public SQTeamOperation
{
   DWORD dwLeaverSID;
};

struct SATeamSomeoneLeaveMsg:
    public STeamSomeoneLeaveMsg,
    public SATeamOperation

{
    //The ID of the player whom the message should not be sendto 
    DNID  dnidLeaver;
    WORD  wLeaverServerID; 
    DWORD dwLeaverSID;
    WORD wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 某人被踢出队伍
DECLARE_MSG(STeamSomeoneKickMsg,SCtrlTeamGroup,SCtrlTeamGroup::EPRO_SOMEONE_KICKED_TEAM)
struct SQTeamSomeoneKickMsg :
    public STeamSomeoneKickMsg,
    public SQTeamOperation
{
    DWORD dwTeamID;
    DWORD dwKickedSID;
};

struct SATeamSomeoneKickMsg :
    public STeamSomeoneKickMsg,
    public SATeamOperation
{
    WORD wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 获取队伍成员基本数据
DECLARE_MSG(SGetTeamMemberMsg, STeamOperationMsg, STeamOperationMsg::EPRO_TEAM_GET_MEMBER)
struct SQGetTeamMemberMsg :
    public SGetTeamMemberMsg,
    public SQTeamOperation
{
    // 没什么具体成员，都在SQTeamOperation里边了
    DWORD dwTeamID;
};

struct SAGetTeamMemberMsg :
    public SGetTeamMemberMsg,
    public SATeamOperation
{
    WORD wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 组队聊天，可能会和大的聊天系统和并在一起吧，现在暂时不管它
DECLARE_MSG(STeamTalkBaseMsg, STeamOperationMsg, STeamOperationMsg::EPRO_TEAM_TALK)
struct  SQTeamTalkMsg : STeamTalkBaseMsg,
    public SQTeamOperation
{
    DWORD dwTeamID;
    DWORD dwSrcGlobalID;
    char  szTalkerName[CONST_USERNAME];
    char  szTalkMsg[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        return (WORD)(sizeof(SQTeamOperation) - (MAX_CHAT_LEN-strlen(szTalkMsg)-1));
    }
};

struct  SATeamTalkMsg : STeamTalkBaseMsg,
    public SATeamOperation
{
    DWORD dwTeamID;
    char  szTalkerName[CONST_USERNAME];
    char  szTalkMsg[MAX_CHAT_LEN];

    WORD GetMySize()
    {
        return (WORD)(sizeof(SATeamTalkMsg) - (MAX_CHAT_LEN-strlen(szTalkMsg)-1));
    }
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 组队分经验，但是因为是同场景的角色才能共享经验，所以这个消息可能会用不上，现在暂时不管它
DECLARE_MSG(STeamShareExpMsg, STeamOperationMsg, STeamOperationMsg::EPRO_TEAM_SHARE_EXP)
struct SQTeamShareExpMsg : 
    public STeamShareExpMsg,
    public SQTeamOperation
{
    DWORD dwTeamID;
    //dwReserved 作为消灭怪物得到的经验值
};

struct SATeamShareExpMsg :
    public STeamShareExpMsg,
    public SATeamOperation
{
    DWORD dwExp;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 切换队形技
DECLARE_MSG(STeamChangeSkillMsg, STeamOperationMsg, STeamOperationMsg::EPRO_TEAM_CHANGE_SKILL)
struct SQTeamChangeSkillMsg :
    public STeamChangeSkillMsg,
    public SQTeamOperation
{
    BYTE bySkill; // 这个是用于回传的当前切换的技能
    BYTE byLevel;
};

struct SATeamChangeSkillMsg :
    public STeamChangeSkillMsg,
    public SATeamOperation
{
    BYTE bySkill; // 这个是用于回传的当前切换的技能
    BYTE byLevel;
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 聊天频道消息
DECLARE_MSG_MAP(SORBChannelBaseMsg, SORBMsg, SORBMsg::EPRO_CTRL_CHANNEL)
//{{AFX
EPRO_CHANNEL_JOIN,
EPRO_CHANNEL_LEAVE,
EPRO_CHANNEL_TALK,
EPRO_CHANNEL_KICK,
EPRO_CHANNEL_CREATE,
EPRO_CHANNEL_DISMISS,
EPRO_CHANNEL_CHANGERULE,
EPRO_CHANNEL_GETMEMBERINFO,   // 取得聊天室里玩家的信息
EPRO_CHANNEL_GETCHANNELINFO,  // 取得聊天室的基本信息
EPRO_CHANNEL_WHISPER,
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------
//=============================================================================================
DECLARE_MSG(SORBChangeRuleBaseChannelMsg,SORBChannelBaseMsg,SORBChannelBaseMsg::EPRO_CHANNEL_CHANGERULE)
struct SQORBChangeRuleChannelMsg : public SORBChangeRuleBaseChannelMsg
{
    DWORD dwStaticID;
    int iChannelRules;
};
struct SAORBChangeRuleChannelMsg : public SORBChangeRuleBaseChannelMsg
{
    DWORD dwChannelID;
    int iChannelRules;

    WORD wResult;
};


//=============================================================================================
DECLARE_MSG(SORBCreateBaseChannelMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_CREATE)
struct SQORBCreateChannelMsg : public SORBCreateBaseChannelMsg
{
    DWORD dwCreatorStaticID;//the creator static id
    int   iChannelRules; 
	char  szChannelName[CONST_USERNAME];
};

struct SAORBCreateChannelMsg : public SORBCreateBaseChannelMsg
{
    char  szCreatorName[CONST_USERNAME];
	char  szChannelName[CONST_USERNAME];
    DWORD dwChannelID;

    WORD wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SORBDisMissBaseChannelMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_DISMISS)
struct SQORBDisMissChannelMsg : public SORBDisMissBaseChannelMsg
{
    DWORD dwStaticID;//the creator static id 
    DWORD  dwChannelID;
};

struct SAORBDisMissChannelMsg :public SORBDisMissBaseChannelMsg
{
    DWORD dwChannelID;

    WORD wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SORBJoinBaseChannelMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_JOIN)
struct SQORBJoinChannelMsg : public SORBJoinBaseChannelMsg
{
    WORD  wResult;
    DWORD dwStaticID;
    DWORD  dwChannelID;
};
//---------------------------------------------------------------------------------------------
struct SAORBJoinChannelMsg : public SORBJoinBaseChannelMsg
{
    WORD  wResult;
    char  szPlayerName[CONST_USERNAME];

    DWORD dwStaticID;
    DWORD  dwChannelID;
};


//=============================================================================================
DECLARE_MSG(SORBLeaveBaseChannelMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_LEAVE)
struct SQORBLeaveChannelMsg : public SORBLeaveBaseChannelMsg
{
    WORD  wResult;
    DWORD dwStaticID;
    DWORD  dwChannelID;
};
//---------------------------------------------------------------------------------------------
struct SAORBLeaveChannelMsg : public SORBLeaveBaseChannelMsg
{
    WORD  wResult;
    char  szPlayerName[CONST_USERNAME];

    DNID  dnidTo;

    DWORD dwStaticID;
    DWORD dwChannelID;
};


//=============================================================================================
DECLARE_MSG(SORBKickBaseChannelMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_KICK)
struct SQORBKickChannelMsg : public SORBKickBaseChannelMsg
{
    WORD  wResult;
    DWORD dwCreatorStaticID;
    DWORD dwKickedStaticID;
    DWORD  dwChannelID;
};
//---------------------------------------------------------------------------------------------
struct SAORBKickChannelMsg : public SORBKickBaseChannelMsg
{
    WORD  wResult;
    char  szPlayerName[CONST_USERNAME];

    DWORD dwCreatorStaticID;
    DWORD dwKickedStaticID;
    DWORD  dwChannelID;
};


//=============================================================================================
DECLARE_MSG(SORBChatBaseChannelMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_TALK)
//区域服务器到聊天服务器
struct SQORBChatChannelMsg : public SORBChatBaseChannelMsg
{
    DWORD dwStaticID;
    
    DWORD dwGlobalID;
    DWORD dwChannelID;
    WORD wLength;
    DWORD dwRecieverSID;
    char cCharData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SQORBChatChannelMsg, cCharData);
    }
};
//---------------------------------------------------------------------------------------------

//聊天服务器到区域服务器
struct SAORBChatChannleMsg : public SORBChatBaseChannelMsg
{
    DWORD dwStaticID;
    DWORD dwGlobalID;
    DWORD dwChannelID;
    WORD  wLength;
    char  szPlayerName[CONST_USERNAME];
    char  szRecieverName[CONST_USERNAME];

    DNID dnidTo;       // 在区域服务器上接收玩家的Dnid

    char  cCharData[MAX_CHAT_LEN];
    WORD  GetMySize()
    {
        _GetSize(SAORBChatChannleMsg, cCharData);
    }
};
//=============================================================================================
DECLARE_MSG(SORBGetBaseChannelMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_GETMEMBERINFO)
struct SQORBGetChannelMsg : public SORBGetBaseChannelMsg
{
    DWORD dwStaticID ; //想取得聊天频道信息的玩家的StaticID
    DWORD dwChannelID; //想取得的聊天室的ID
};

struct SAORBGetChannelMsg : public SORBGetBaseChannelMsg
{
    DWORD dwStaticID;  //想取得聊天频道信息的玩家的StaticID
    DWORD dwChannelID; //想取得的聊天室的ID
    WORD wPlayerNumber;//聊天室里面玩家的个数
    DNID dnidTo;
    SChannelPlayerInfo ChannelPlayer[MAX_CHANNEL_PLAYER_NUMBER];
    WORD GetMySize()
    {
        return sizeof(SAORBGetChannelMsg)-(MAX_CHANNEL_PLAYER_NUMBER-wPlayerNumber)*sizeof(SChannelPlayerInfo);
    };
};
//=============================================================================================
DECLARE_MSG(SORBGetBaseChannelInfoMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_GETCHANNELINFO)
struct SQORBGetChannelInfoMsg : SORBGetBaseChannelInfoMsg
{
    DWORD dwStaticID;
};
//---------------------------------------------------------------------------------------------
struct SAORBGetChannelInfoMsg : SORBGetBaseChannelInfoMsg
{
    DWORD dwChannelID;
	char  szChannelName[CONST_USERNAME];
    char  szCreatorName[CONST_USERNAME];
    int   iRules;
};


//=============================================================================================
DECLARE_MSG(SORBBaseChannelWhisperMsg, SORBChannelBaseMsg ,SORBChannelBaseMsg::EPRO_CHANNEL_WHISPER)
struct SQORBChannelWhisperMsg : SORBBaseChannelWhisperMsg
{
    DWORD dwTalkerStaticID; // 发出信息的玩家的StaticID
    DWORD dwReceiverStaticID;
    DWORD dwChannelID;
    char  szChatMsg[MAX_CHAT_LEN];
};
//---------------------------------------------------------------------------------------------
struct SAORBChannelWhisperMsg : SORBBaseChannelWhisperMsg
{
    DNID dnidTo;
    DWORD dwTalkerStaticID;
    DWORD dwChannelID;
    char szTalkerName[CONST_USERNAME];
    char szReceiverName[CONST_USERNAME];
    DWORD dwReceiverStaticID;
    char szChatMsg[MAX_CHAT_LEN];
};
//=============================================================================================
DECLARE_MSG_MAP(SORBFriendBaseMsg,SORBMsg,SORBMsg::EPRO_CTRL_FRIEND)
//{{AFX
EPRO_FRIEND_ONLINE, // 好友上线通知消息
//}}AFX
END_MSG_MAP()

DECLARE_MSG(SORBBaseFriendOnlineMsg,SORBFriendBaseMsg,SORBFriendBaseMsg::EPRO_FRIEND_ONLINE)
struct SQORBFriendOnlineMsg : SORBBaseFriendOnlineMsg
{
    char cName[CONST_USERNAME]; // 上线玩家的名字
    char cFriendName[CONST_USERNAME]; // 好友的名字
    DWORD dwStaticID;

	bool bOnline;
};
struct SAORBFriendOnlineMsg : SORBBaseFriendOnlineMsg
{
    char cName[CONST_USERNAME]; 
    char cFriendName[CONST_USERNAME];
    DWORD dwStaticID;

	bool bOnline;
};

//=============================================================================================
DECLARE_MSG_MAP(SORBChatBaseMsg, SORBMsg, SORBMsg::EPRO_CTRL_CHAT)
//{{AFX
EPRO_CHAT_WISPER,   //密聊消息
EPRO_CHAT_GLOBAL,
EPRO_CHAT_FACTION,
//}}AFX
END_MSG_MAP()

DECLARE_MSG(SORBBaseChatWisperMsg, SORBChatBaseMsg, SORBChatBaseMsg::EPRO_CHAT_WISPER)
struct SQORBChatWisperMsg : SORBBaseChatWisperMsg
{
    char cTalkerName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
    char cRecvName[CONST_USERNAME];
};
//---------------------------------------------------------------------------------------------
struct SAORBChatWisperMsg : SORBBaseChatWisperMsg
{
    char  cName[CONST_USERNAME];
    char  cNameII[CONST_USERNAME];
    char  cChatData[MAX_CHAT_LEN];
    BYTE  byType; // 1 for recveiver 2 for sender
    DNID  dnidRecv;
    WORD  wRecvServerID;
    DWORD dwGMLevel;
    DWORD dwGMLevelII;
};

DECLARE_MSG(SORBBaseChatGlobalMsg, SORBChatBaseMsg, SORBChatBaseMsg::EPRO_CHAT_GLOBAL)
struct SQORBChatGlobalMsg : public SORBBaseChatGlobalMsg
{
    BYTE byType;
    char cTalkerName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SQORBChatGlobalMsg,cChatData);
    }
};

struct SAORBChatGlobalMsg : public SORBBaseChatGlobalMsg
{
    BYTE byType;
    char cTalkerName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SAORBChatGlobalMsg,cChatData);
    }
};
DECLARE_MSG(SORBBaseChatFactionMsg, SORBChatBaseMsg, SORBChatBaseMsg::EPRO_CHAT_FACTION)
struct SQORBChatFactionMsg : public SORBBaseChatFactionMsg
{
    char cTalkerName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
	char cFactionName[CONST_USERNAME];
    WORD GetMySize()
    {
        _GetSize(SQORBChatFactionMsg,cChatData);
    }
};
struct SAORBChatFactionMsg : public SORBBaseChatFactionMsg
{
    char cTalkerName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
	char cFactionName[CONST_USERNAME];
    WORD GetMySize()
    {
        _GetSize(SAORBChatFactionMsg,cChatData);
    };
};


//////////////////////////////////////////////////////////////////////
// orb 数据转发相关操作
//////////////////////////////////////////////////////////////////////
DECLARE_MSG(SORBDataTransMsg, SORBMsg, SORBMsg::EPRO_CTRL_DATATRANS)

struct SQAORBDataTransMsg : public SORBDataTransMsg
{
public:
	enum TRANS_TYPE
	{
		TRANSDATATOPLAYER,
		PLAYERINFO_QUERY,
		TRANSDATATO_DESTPLAYERSERVER,
		TRANSDATATO_ALLSERVER,
	};

private:
    WORD m_wTransType;
    WORD m_wTransSize;
	char m_szTransName[CONST_USERNAME];
    char m_pTransData[1];

public:
    WORD GetTransType() { return m_wTransType; }
    WORD GetTransSize() { return m_wTransSize; }
    LPCSTR GetTransName() { return m_szTransName; }
    LPCVOID GetTransData() { return m_pTransData; }

    WORD GetMySize() { return sizeof( SQAORBDataTransMsg ) + m_wTransSize; }

    static SQAORBDataTransMsg* MakeDataTransMsg( WORD wTransType, LPCSTR szTransName, 
        LPCVOID pTransData, WORD wTransSize, void *bufHolder, size_t bufSize )
    {
        if ( wTransSize > 0xC000 || bufHolder == NULL || bufSize < sizeof( SQAORBDataTransMsg ) + wTransSize )
            return NULL;

#ifdef _DEBUG
#   undef new
#endif
        SQAORBDataTransMsg *pMsg = new ( bufHolder ) SQAORBDataTransMsg;
        pMsg->dnidClient = -1;
        pMsg->m_wTransType = wTransType;
        pMsg->m_wTransSize = wTransSize;
		dwt::strcpy(pMsg->m_szTransName, szTransName, CONST_USERNAME);
        memcpy( pMsg->m_pTransData, pTransData, wTransSize );

        return pMsg;
    }

/*
    static WORD MakeDataTransMsg(SQAORBDataTransMsg *&pMsg, LPCSTR szTransName, LPCVOID pTransData, WORD wTransSize, WORD wTransType)
    {
        pMsg = NULL;

        // 太大的包了！
        if (wTransSize > 0xC000)
            return 0;

        SQAORBDataTransMsg temp;
        temp.dnidClient = -1;
        temp.m_wTransType = wTransType;
        temp.m_wTransSize = wTransSize;
        dwt::strcpy(temp.m_szTransName, szTransName, CONST_USERNAME);

        pMsg = (SQAORBDataTransMsg *)new char[sizeof(SQAORBDataTransMsg) + wTransSize];

        memcpy(pMsg, &temp, sizeof(SQAORBDataTransMsg));
        memcpy(pMsg->m_pTransData, pTransData, wTransSize);

        return sizeof(SQAORBDataTransMsg) + wTransSize;
    }
*/
};
