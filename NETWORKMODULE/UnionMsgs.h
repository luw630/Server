#pragma once
#include "NetModule.h"

DECLARE_MSG_MAP(SUnionBaseMsg, SMessage, SMessage::EPRO_UNION_MESSAGE)
//{{AFX
EPRO_UNION_CREATE,            // 创建结义
EPRO_UNION_INVITATORY,        // 邀请结义
EPRO_UNION_ONLINE,            // 好友在线消息
EPRO_UNION_UPDATE,            // 更新结义（）
EPRO_GET_UNIONDATA,           // 取得结义数据
EPRO_SAVE_UNIONDATA,          // 保存结义数据
EPRO_UNION_GET_EXP,           // 请求获取经验
EPRO_UNION_KICK,              // 退出结拜
//}}AFX
END_MSG_MAP()
//=============================================================================================

// 结义数据
DECLARE_MSG( SSaveUnionMsgBase, SUnionBaseMsg, SUnionBaseMsg::EPRO_SAVE_UNIONDATA )
struct SQSaveUnionDataMsg : public SSaveUnionMsgBase
{
    DWORD saveTime;
	DWORD unionCount;
	DWORD dwSendSize;
	char streamData[0xe000];
};

DECLARE_MSG( SGetUnionMsgBase, SUnionBaseMsg, SUnionBaseMsg::EPRO_GET_UNIONDATA )
struct SQGetUnionDataMsg : public SGetUnionMsgBase
{
    DNID	dwZoneID;		//区域服务器ID（用于设置回传目标！）
};

struct SAGetUnionDataMsg : public SGetUnionMsgBase
{
    DNID dwZoneID;		//区域服务器ID（用于设置回传目标！）
	DWORD unionCount;
	DWORD dwSendSize;
    BOOL  readEnd;
	char streamData[0xe000];
};

//=============================================================================================
DECLARE_MSG(SCreateUnionMsg, SUnionBaseMsg, SUnionBaseMsg::EPRO_UNION_CREATE )
struct SACreateUnionMsg : public SCreateUnionMsg
{
	DWORD destID;       // 与目标玩家的ID
};

struct SQCreateUnionMsg : public SCreateUnionMsg
{
	DWORD destID;	    // 与某某玩家结义
    char title[9];      // 标题
    char suffx[3];      // 后缀
};

//=============================================================================================
DECLARE_MSG( SInvitatoryUnionMsg, SUnionBaseMsg, SUnionBaseMsg::EPRO_UNION_INVITATORY )
struct SAInvitatoryUnionMsg : public SInvitatoryUnionMsg // 通知某玩家，某玩家要加为他为好友
{
	DWORD playerID;       // 目标玩家的ID
	char playerName[CONST_USERNAME];
};

// 玩家返回是否愿意加为好友
struct SQInvitatoryUnionMsg : public SInvitatoryUnionMsg
{
	DWORD playerID;       // 目标玩家的ID
	bool value;
};

//=============================================================================================
DECLARE_MSG( SOnlineUnionMsg, SUnionBaseMsg, SUnionBaseMsg::EPRO_UNION_ONLINE )
struct SUnionOnlineMsg : public SOnlineUnionMsg
{
	enum FRIEND_STATE
	{
		UNION_STATE_ONLINE,	    // 上线
		UNION_STATE_OUTLINE,	// 下线
		UNION_STATE_HANGUP,	    // 挂机
		UNION_STATE_KILLOTHER,  // 打败别人
		UNION_STATE_BEKILLED,	// 被别人打败
		UNION_STATE_LEVELUP,	// 升级
		UNION_STATE_PASSPULSE,	// 冲穴
	}eState;

	char	cName[CONST_USERNAME];		 	// 好友
};

//=============================================================================================
struct UnionInfo
{
	DWORD rolrid;            // 唯一标识ID
	DWORD exp;               // 当前经验
	char szName[CONST_USERNAME];         // 角色名
	BYTE school;             // 门派
	BYTE camp;               // 阵营 1: 正 2：中 3：邪
	BYTE job;                // 职位
	char szFaceionName[CONST_USERNAME];  // 帮派名
	bool online;             // 是否在线
};
DECLARE_MSG( SUpdateUnionMsg, SUnionBaseMsg, SUnionBaseMsg::EPRO_UNION_UPDATE )
struct SAUpdateUnionMsg : public SUpdateUnionMsg
{
	enum {
		UPDATE_MEMBER,         // 更新成员
		UPDATE_EXP,            // 更新经验
	};

	BYTE type;
	char szName[CONST_USERNAME];
	BYTE change;
	UnionInfo unionInfo[8];
};

struct SAUpdateExpMsg : public SUpdateUnionMsg
{
	BYTE type;
	BYTE memberIdx;          // 指定成员
	DWORD exp;               // 经验
};

//=============================================================================================

DECLARE_MSG( SKickUnionMsg, SUnionBaseMsg, SUnionBaseMsg::EPRO_UNION_KICK )
struct SAKickUnionMsg : public SKickUnionMsg
{
    enum
    {
        KICK_ME,          // T出自己
        KICK_OTHER,       // T出其他兄弟
    };

    DWORD  dwPlayerID;    // 被T玩家ID
	char   szName[CONST_USERNAME];  // 被T玩家角色名
    DWORD  dwBossID;      // 老大的ID
    BYTE   byType;
};

struct SQKickUnionMsg : public SKickUnionMsg
{
    DWORD  dwPlayerID;    // 目标玩家ID
};






