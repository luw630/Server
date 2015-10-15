#pragma once

#include "NetModule.h"
#include "chattypedef.h"

// 聊天的表情符号
// 聊天中出现特殊字符作为表情符号
//=============================================================================================
DECLARE_MSG_MAP(SChatBaseMsg, SMessage, SMessage::EPRO_CHAT_MESSAGE)
//{{AFX
EPRO_CHAT_SYSMSG,	        // 系统消息
EPRO_CHAT_SYSCALL,			// 系统公告
EPRO_CHAT_PUBLIC,			// 附近频道
EPRO_CHAT_TEAM,			    // 队伍频道
EPRO_CHAT_GANG,			    // 帮派
EPRO_CHAT_WHISPER,			// 私聊频道
EPRO_CHAT_CHATROOM,	    	// 聊天室
EPRO_CHAT_TEMP_SYSTEM,      // 临时显示消息
EPRO_CHAT_CHANNEL,          // 聊天频道
EPRO_CHAT_GLOBAL,           // 世界频道
EPRO_CHAT_FACTION,          // 帮派频道
EPRO_CHAT_SCRIPTBULLETIN,   // 脚本公告
EPRO_CHAT_TALKMASK,			// 服务器通知客户端 角色被禁言和禁言时间
EPRO_CHAT_SCHOOL,           // 门派频道
EPRO_CHAT_KILLED,			// 死亡通知(用于有帮的玩家死亡通知全帮)
EPRO_CHAT_REGION,           // 区域频道
EPRO_CHAT_TIPS,				// 提示信息
EPRO_CHAT_RUMOR,			// 传闻频道
EPRO_CHAT_BROADCAST,
EPRO_CHAT_BUSINESS,			// 商会频道
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------


//=============================================================================================

DECLARE_MSG(SChatBaseTalkMaskMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_TALKMASK)

struct SChatTalkMaskMsg : public SChatBaseTalkMaskMsg
{
	DWORD dwTime;		//时间单位是 分钟
};

//---------------------------------------------------------------------------------------------


//=============================================================================================
DECLARE_MSG(SChatBaseSysMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_SYSMSG)
struct SChatSysMsg : public SChatBaseSysMsg
{
    BYTE btMsgID;							        // 系统消息ID
    char cSystem[MAX_CHAT_LEN];                     // 系统消息信息
    WORD GetMySize()
    {
        _GetSize(SChatSysMsg, cSystem);
    }
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SChatBaseToAllMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_SYSCALL)
struct SChatToAllMsg : public SChatBaseToAllMsg
{
    WORD wSoundId;							        // 声音编号
    WORD wChatLen;							        // 聊天信息长度
    char cChat[MAX_CHAT_LEN];	                    // 聊天信息(包括图片编号)
	WORD wShowNum;	//add by ly 2014/5/4 系统公告显示的次数
    WORD GetMySize()
    {
        _GetSize(SChatToAllMsg, cChat);
    }
	SChatToAllMsg() :wShowNum(1)
	{

	}
};
//---------------------------------------------------------------------------------------------

DECLARE_MSG(SChatBasePublic, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_PUBLIC)
struct SQChatPublic : public SChatBasePublic
{
    // 不包括空结束
    WORD wLength;
    WORD wSoundId;
    char cCharData[MAX_CHAT_LEN];

    WORD GetMySize()
    {
        _GetSize(SQChatPublic, cCharData);
    }
};

struct SAChatPublic : public SChatBasePublic
{
    DWORD  dwGID;
    WORD   wLength;
    WORD   wSoundId;
    char   cCharData[MAX_CHAT_LEN];

    WORD GetMySize()
    {
        _GetSize(SAChatPublic, cCharData);
    }
};

DECLARE_MSG( SChatBaseTeam, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_TEAM )
struct SQChatTeam : public SChatBaseTeam
{
	char cTalkerName[CONST_USERNAME];
	char cChatData[ MAX_CHAT_LEN ];
	WORD GetMySize()
	{
		_GetSize( SQChatTeam, cChatData );
	}
};

struct SAChatTeam: public SChatBaseTeam
{
	char cTalkerName[CONST_USERNAME];
	char cChatData[ MAX_CHAT_LEN ];
	WORD GetMySize()
	{
		_GetSize( SAChatTeam, cChatData );
	}

};

//---------------------------------------------------------------------------------------------
//=============================================================================================
// 帮派聊天

DECLARE_MSG(SChatBaseFactionMsg,SChatBaseMsg,SChatBaseMsg::EPRO_CHAT_FACTION)
struct SQChatFactionMsg : public SChatBaseFactionMsg
{
	char cTalkerName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SQChatFactionMsg,cChatData);
    }
};
struct SAChatFactionMsg : public SChatBaseFactionMsg
{
    WORD wResult; // 0 还未加入帮会 , 1 成功
	char cTalkerName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SAChatFactionMsg,cChatData);
    }
};

// ==================================================================================
// 世界聊天
DECLARE_MSG(SChatBaseGlobalMsg,SChatBaseMsg,SChatBaseMsg::EPRO_CHAT_GLOBAL)
struct SQChatGlobalMsg : public SChatBaseGlobalMsg
{
    BYTE  byType;
	DWORD color;
	char  cTalkerName[CONST_USERNAME];
	char  cHead[MAX_HEAD_LEN];
    char  cChatData[MAX_CHAT_LEN];
	
    WORD GetMySize()
    {
        _GetSize(SQChatGlobalMsg,cChatData);
    };
};

struct SAChatGlobalMsg : public SChatBaseGlobalMsg
{
    typedef enum
    {
        ECT_NORMAL,
        ECT_SYSTEM,
        ECT_GAMEMASTER,
        ECT_RUMOUR,
        ECT_TRADE,
        ECT_RUMOUR_FAIL,
		ECT_SMALL_HORN,			// 小喇叭
		ECT_AFFICHE,			// 系统公告;显示在屏幕正中间的字
		ECT_IMPORTANT_AFFICHE,  // 优先级高的系统公告
		ECT_SPECIAL,			// 特殊消息（如奇遇），显示在最下边公告消息显示的位置
		ECT_TREASURE_SCROLL,	// 百宝阁的滚动消息
		ECT_TREASURE_EXPAIN,	// 百宝阁左下角的信息

        ECT_CTIYOWNER,			// 城主
        ECT_CTIYOWNERTRADE,     // 商贸不带商品
        ECT_CTIYOWNERTRADE_ITEM, 

		ECT_WULINCHIEF,
    };

    BYTE  byType;
	DWORD color;
	
	char  cTalkerName[CONST_USERNAME];
	//WK 2015 .1.13 添加,前台头像 begin
	char  cHead[MAX_HEAD_LEN];
	//WK 2015 .1.13 添加,前台头像 end

    char  cChatData[MAX_CHAT_LEN];
    
	WORD GetMySize()
    {
        _GetSize(SAChatGlobalMsg,cChatData);
    };
};

struct SAAfficheMsg : public SChatBaseGlobalMsg
{

    BYTE byType;
	BYTE color[3];
	BYTE scrollNum;
    char cChatData[200];
};

DECLARE_MSG(SScriptChatGlobalMsg,SChatBaseMsg,SChatBaseMsg::EPRO_CHAT_SCRIPTBULLETIN)
// 对应的脚本通告
#define _GetSSize(classname, buffer)    WORD len; \
    len = (WORD)dwt::strlen(buffer, 500); \
    buffer[len] = 0; \
    return sizeof(classname) - (500 - len - 1) 

struct SAScriptChatGlobalMsg : public SScriptChatGlobalMsg
{
    BYTE byType;
    char cChatData[500];
    WORD GetMySize()
    {
        _GetSSize(SAScriptChatGlobalMsg,cChatData);
    };
};
//=============================================================================================
DECLARE_MSG(SChatBaseGang, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_GANG)
//Client To Server
struct SQChatGang : public SChatBaseGang
{
    WORD wLength;
    char cCharData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SQChatGang, cCharData);
    }
};

//Server To Client
struct SAChatGang : public SChatBaseGang
{
    DWORD dwGID;
    WORD wLength;
    char cCharData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SAChatGang, cCharData);
    }

};
// ====================================================
// 私聊
DECLARE_MSG(SChatBaseWisperMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_WHISPER)
struct SQChatWisperMsg : public SChatBaseWisperMsg
{
	char szRecvName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SQChatWisperMsg, cChatData);
    }
};

struct SAChatWisperMsg : public SChatBaseWisperMsg
{
    BYTE byType;			// 1 for recveiver // 2 for sender
	char szName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
	WORD wHeadIcon;   //头像ID
	WORD wLevel;         //等级
    WORD GetMySize()
    {
        _GetSize(SAChatWisperMsg, cChatData);
    }
};

// 门派聊天
//=============================================================================================
DECLARE_MSG( SChatBaseSchool, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_SCHOOL )
struct SQChatSchool : public SChatBaseSchool
{
	char cTalkerName[CONST_USERNAME];
    char cChatData[ MAX_CHAT_LEN ];
    WORD GetMySize()
    {
        _GetSize( SQChatSchool, cChatData );
    }
};

struct SAChatSchool: public SChatBaseSchool
{
	char cTalkerName[CONST_USERNAME];
    char cChatData[ MAX_CHAT_LEN ];
    WORD GetMySize()
    {
        _GetSize( SAChatSchool, cChatData );
    }

};
// ==================================================================================
// 传闻聊天
DECLARE_MSG(SChatRumorMsg,SChatBaseMsg,SChatBaseMsg::EPRO_CHAT_RUMOR)
struct SQChatRumorMsg : public SChatRumorMsg
{
	char  cTalkerName[CONST_USERNAME];
	char  cChatData[MAX_CHAT_LEN];

	WORD GetMySize()
	{
		_GetSize(SQChatRumorMsg,cChatData);
	};
};

struct SAChatRumorMsg : public SChatRumorMsg
{
	char  cTalkerName[CONST_USERNAME];
	char  cChatData[MAX_CHAT_LEN];

	WORD GetMySize()
	{
		_GetSize(SAChatRumorMsg, cChatData);
	};
};

// 场景聊天
//=============================================================================================
DECLARE_MSG( SChatBaseRegion, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_REGION )
struct SQAChatRegion : public SChatBaseRegion
{
	char cTalkerName[CONST_USERNAME];
    char cChatData[ MAX_CHAT_LEN ];
    WORD GetMySize()
    {
        _GetSize( SQAChatRegion, cChatData );
    }
};

//---------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////
enum CHANNEL_RULES
{
    CR_CLOSE    =     0x0000000f,//所有人不可进 
    CR_OPEN     =     0x000000f0,//所有人可进
    CR_FRIEND   =     0x00000f00,//好友可进 
    CR_TEAM     =     0x0000f000,
    CR_TONG     =     0x000f0000,
};

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 聊天频道
//=============================================================================================
DECLARE_MSG_MAP(SChannelBaseMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_CHANNEL)
//{{AFX
EPRO_CHANNEL_JOIN,
EPRO_CHANNEL_LEAVE,
EPRO_CHANNEL_TALK,
EPRO_CHANNEL_KICK,
EPRO_CHANNEL_CREATE,
EPRO_CHANNEL_DISMISS,
EPRO_CHANNEL_CHANGERULE,
EPRO_CHANNEL_GETMEMBERINFO,
EPRO_CHANNEL_GETCHANNELINFO,
EPRO_CHANNEL_WHISPER,
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================
struct SForChannelBroadCast
{
    SChannelBaseMsg * pChannelBaseMsg;
    int iSize;
};
struct SForGlobalTalkBroadCast
{
    SChatBaseGlobalMsg * pChatBaseGlobalMsg;
    int iSize;
};

DECLARE_MSG(SChangeRuleBaseChannelMsg,SChannelBaseMsg,SChannelBaseMsg::EPRO_CHANNEL_CHANGERULE)
struct SQChangeRuleChannelMsg : public SChangeRuleBaseChannelMsg
{
    DWORD dwStaticID;
    int iChannelRules;
};

struct SAChangeRuleChannelMsg : public SChangeRuleBaseChannelMsg
{
    DWORD dwChannelID;
    int iChannelRules;
    WORD wResult;
};


//=============================================================================================
DECLARE_MSG(SCreateBaseChannelMsg, SChannelBaseMsg, SChannelBaseMsg::EPRO_CHANNEL_CREATE)
struct SQCreateChannelMsg : public SCreateBaseChannelMsg
{

    DWORD dwCreatorStaticID;//the creator static id
    int   iChannelRules; 
	char  szChannelName[CONST_USERNAME];
};

struct SACreateChannelMsg : public SCreateBaseChannelMsg
{
	char  szCreatorName[CONST_USERNAME];
    DWORD dwChannelID;
    WORD wResult;
	char szChannelName[CONST_USERNAME];
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SDisMissBaseChannelMsg, SChannelBaseMsg, SChannelBaseMsg::EPRO_CHANNEL_DISMISS)
struct SQDisMissChannelMsg : public SDisMissBaseChannelMsg
{
    DWORD dwStaticID;//the creator static id 
    DWORD dwChannelID;
};

struct SADisMissChannelMsg :public SDisMissBaseChannelMsg
{
    DWORD dwChannelID;
    WORD wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SJoinBaseChannelMsg, SChannelBaseMsg, SChannelBaseMsg::EPRO_CHANNEL_JOIN)
struct SQJoinChannelMsg : public SJoinBaseChannelMsg
{
    WORD  wResult;
    DWORD dwStaticID;
    DWORD  dwChannelID;

};

struct SAJoinChannelMsg : public SJoinBaseChannelMsg
{
    WORD  wResult;
	char  szPlayerName[CONST_USERNAME];
    DWORD dwStaticID;
    DWORD dwChannelID;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SLeaveBaseChannelMsg, SChannelBaseMsg, SChannelBaseMsg::EPRO_CHANNEL_LEAVE)
struct SQLeaveChannelMsg : public SLeaveBaseChannelMsg
{
    WORD  wResult;
    DWORD dwStaticID;
    DWORD  dwChannelID;
};

struct SALeaveChannelMsg : public SLeaveBaseChannelMsg
{
    WORD  wResult;
	char  szPlayerName[CONST_USERNAME];
    DWORD dwStaticID;
    DWORD dwChannelID;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SKickBaseChannelMsg, SChannelBaseMsg, SChannelBaseMsg::EPRO_CHANNEL_KICK)
struct SQKickChannelMsg : public SKickBaseChannelMsg
{
    WORD  wResult;
    DWORD dwCreatorStaticID;
    DWORD dwKickedStaticID;
    DWORD dwChannelID;
};

struct SAKickChannelMsg : public SKickBaseChannelMsg
{
    WORD  wResult;
	char  szPlayerName[CONST_USERNAME];
    DWORD dwCreatorStaticID;

    DWORD dwKickedStaticID;
    DWORD dwChannelID;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SChatBaseChannelMsg, SChannelBaseMsg, SChannelBaseMsg::EPRO_CHANNEL_TALK)
struct SQChatChannelMsg : public SChatBaseChannelMsg
{
    DWORD dwGlobalID;
    DWORD dwChannelID;
    WORD wLength;
    DWORD dwReceiverSID; // 聊天室里XX对XX说的形式
    char cCharData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SQChatChannelMsg, cCharData);
    }
};

struct SAChatChannleMsg : public SChatBaseChannelMsg
{
    DWORD dwGlobalID;
    DWORD  dwChannelID;
    WORD  wLength;
	char  szPlayerName[CONST_USERNAME];
    char  cCharData[MAX_CHAT_LEN];
    WORD  GetMySize()
    {
        _GetSize(SAChatChannleMsg, cCharData);
    }
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SGetBaseChannelMsg,SChannelBaseMsg, SChannelBaseMsg::EPRO_CHANNEL_GETMEMBERINFO)
struct SQGetChannelMsg : public SGetBaseChannelMsg
{
    DWORD dwStaticID ; //想取得聊天频道信息的玩家的StaticID
    DWORD dwChannelID; //想取得的聊天室的ID
};

struct SAGetChannelMsg : public SGetBaseChannelMsg
{
    DWORD dwStaticID;  //想取得聊天频道信息的玩家的StaticID
    DWORD dwChannelID; //想取得的聊天室的ID
    WORD wPlayerNumber;//聊天室里面玩家的个数
    SChannelPlayerInfo ChannelPlayer[MAX_CHANNEL_PLAYER_NUMBER];
    WORD GetMySize()
    {
        return sizeof(SAGetChannelMsg)-(MAX_CHANNEL_PLAYER_NUMBER-wPlayerNumber)*sizeof(SChannelPlayerInfo);
    };
};
//=============================================================================================
DECLARE_MSG(SGetBaseChannelInfoMsg,SChannelBaseMsg,SChannelBaseMsg::EPRO_CHANNEL_GETCHANNELINFO)
struct SQGetChannelInfoMsg : SGetBaseChannelInfoMsg
{
};
//these messages are send one bye one 
struct SAGetChannelInfoMsg : SGetBaseChannelInfoMsg
{
    DWORD dwChannelID;
	char  szChannelName[CONST_USERNAME];
	char  szCreatorName[CONST_USERNAME];
    int   iRules;
};


//=============================================================================================
DECLARE_MSG(SChatBaseChannelWhisper,SChannelBaseMsg,SChannelBaseMsg::EPRO_CHANNEL_WHISPER)
struct SQChatChannelWhisper : SChatBaseChannelWhisper
{
    DWORD dwTalkerStaticID; // 发出信息的玩家的StaticID
    DWORD dwReceiverStaticID;
    DWORD dwChannelID;
    char  szChatMsg[MAX_CHAT_LEN];
};
//---------------------------------------------------------------------------------------------
struct SAChatChannelWhisper : SChatBaseChannelWhisper
{
    DWORD dwTalkerStaticID;
	char szTalkerName[CONST_USERNAME];
	char szReceiverName[CONST_USERNAME];
    DWORD dwReceiverStaticID;
    DWORD dwChannelID;
    char szChatMsg[MAX_CHAT_LEN];
};

DECLARE_MSG(SChatBaseKilledMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_KILLED)//新加的
struct SAChatKilledMsg : public SChatBaseKilledMsg
{
	char szName[CONST_USERNAME];
	char killerName[CONST_USERNAME];
	char killerTong[CONST_USERNAME];
	WORD regionID;
	WORD X;
	WORD Y;
};

// 侠义道三统一的提示消息
DECLARE_MSG(SChatTipsMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_TIPS)
struct SAChatTipsMsg : public SChatTipsMsg
{
	enum
	{
		TIPS_SHOWTYPE_SYSTEM = 1, //系统
		TIPS_SHOWTYPE_WINDOWS = 2, //系统提示，显示一个对话框
	};
	bool bHasPrefix;		// 是否有前缀
	char cPrefix[11];		// 前缀
	WORD wShowType;			// 显示类型，指出在什么地方显示消息
	char cChatData[500];	// 显示内容
	WORD GetMySize()
	{
		_GetSSize(SAChatTipsMsg, cChatData);
	};
};

DECLARE_MSG(SChatBroadcastMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_BROADCAST)
struct SAChatBroadcastMsg : public SChatBroadcastMsg
{
	char BroadcasInfo[256];
	SAChatBroadcastMsg()
	{
		memset(BroadcasInfo,0,256);
	}
};

DECLARE_MSG(SChatBusiNessMsg,SChatBaseMsg,SChatBaseMsg::EPRO_CHAT_BUSINESS)
struct SAChatBusiNessMsg : public SChatBusiNessMsg //商会频道聊天服务器发送消息
{
	DWORD		 m_GID;				    // 玩家GID
	char	     m_Playername[40];		// 游戏名字(呢称)
	BYTE		 m_moneyType;			// 金钱类型 银两交易
	DWORD		 m_AllPrice;			// 总价
	SPackageItem m_SPackageItem;        // 格子信息
	WORD         m_Pos;					// 位置
	SAChatBusiNessMsg():m_GID(0),m_moneyType(0),m_AllPrice(0),m_Pos(0)
	{
		memset(m_Playername,0,40);
	}
};