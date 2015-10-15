#pragma once

#include "NetModule.h"
#include "itemtypedef.h"
#include "chattypedef.h"
#include "../pub/ConstValue.h"

#define MAX_TEAM_MEMBER		(5)
#define MAX_TEMPPACK_SIZE	(20)

DECLARE_MSG_MAP(STeamBaseMsgG, SMessage, SMessage::EPRO_TEAM_MESSAGE)
//{{AFX
EPRO_TEAM_CHANGE_SKILL = 20,	// 这里将起始数值设为20是为了和旧的消息分离开,保证就算不兼容,也不会出错...
EPRO_TEAM_REFRESH,				// 刷新队伍数据
EPRO_TEAM_JOIN,					// 加入队伍
EPRO_TEAM_KICK,					// 踢人/离开/解散
EPRO_TEAM_TALK,					// 队伍聊天
EPRO_TEAM_HEADMEN,				// 设置队长
EPRO_TEAM_POSITION,				// 传送队员的坐标
EPRO_CANCEL_REPLY,				// 在一些情况下的取消组队请求
EPRO_TEAM_SUBMIT_REQUEST,		// 提交组队申请
EPRO_TEAM_GETTEAMQUEST,			// 获取请求组队信息
EPRO_TEAM_OLDQUESTLIST,			// 通知客户端哪些组队请求已过时
EPRO_TEAM_NEWQUESTLIST,			// 通知客户端哪些组队请求已更新
EPRO_TEAM_GETFINDTEAMINFO,		// 获取队伍信息
EPRO_TEAM_OLDFINDLIST,			// 通知客户端哪些队伍已过时
EPRO_TEAM_NEWFINDLIST,			// 通知客户端哪些队伍可以入队
EPRO_TEAM_INVITEWITHNAME,		// 邀请加入队伍（通过名字）
EPRO_TEAM_IWANTTOJION,			// 队伍列表中，想加入
EPRO_TEAM_CANCELTEAMQUEST,		// 取消自己的组队申请
EPRO_TEAM_CHANGEGIVEMODE,		// 改变分配模式/颜色
EPRO_TEAM_ADDTEMPITEM,			// 增加道具到队长分配包中
EPRO_TEAM_DELTEMPITEM,			// 从队长分配包中删除一个道具
EPRO_TEAM_DISPATCHTOMEMBER,		// 分配某个道具给队员
//}}AFX
END_MSG_MAP()

class CPlayer;

struct Team						// 队伍信息
{
	enum	// 分配方式
	{
		TGM_TEAMLEADER = 1,		// 队长分配
		TGM_SCHOOL,				// 门派分配
		TGM_FREE,				// 自由分配
	};

	enum	// 分配品级（和道具一致）
	{
		TGC_BLUE = 3,			// 蓝色
		TGC_ZISE,				// 紫色
		TGC_ORANAGE,			// 橘色
		TGC_GOLD,				// 金色
	};

	struct TeamPlayer					// 队员信息（用于客户端显示部分）
	{
		char	szName[CONST_USERNAME];	// 名字
		WORD	bySex		: 1;		// 性别
		WORD	bySchool	: 3;		// 门派
		WORD	byLevel		: 9;		// 等级
		WORD	isLeader	: 1;		// 是否队长
		WORD	status		: 2;		// 状态
		
		QWORD	wMaxHp		: 24;		// 最大血量
		QWORD	wCurHp		: 24;		// 当前血量
		QWORD	wMaxMp		: 24;		// 最大MP
		QWORD	wCurMp		: 24;		// 当前MP

		DWORD	dwGlobal;				// ID
		BYTE	bFaceID;				// 脸模型
		BYTE	bHairID;				// 头发模型
		WORD	wRegionID;				// 地图ID
		DWORD	m_3DEquipID[11];			// 3D装备挂件物品号
	};

	struct TeamPlayerData : public TeamPlayer
	{
		CPlayer *PlayerRef;				// 快速引用
		DWORD	staticId;				// 静态ID
		DNID	dnidPlayer;				// 网络ID
	};

	BYTE			byGiveMode;			// 分配方式
	BYTE			byGiveColor;		// 分配品级
	BYTE			bySend;				// 是否需要更新
	BYTE			byMemberNum;		// 队员数量
	DWORD			LeaderGID;			// 队长的GID
	DWORD           ParentReginID;      // 只记录副本区域ID
	DWORD			PartregionID;		// 副本区域脚本ID
	TeamPlayerData	stTeamPlayer[MAX_TEAM_MEMBER];

	// 队长分配包裹
	struct LeaderPackItem
	{
		SRawItemBuffer	item;
		BYTE			pos;
	};

	SRawItemBuffer	LeaderPack[MAX_TEMPPACK_SIZE];
	WORD			PackCount;

	// 根据名字返回所在位置
	int FindNum(const char *name)
	{
		for (int n = 0; n < MAX_TEAM_MEMBER; n++)
		{
			if (dwt::strcmp(stTeamPlayer[n].szName, name, CONST_USERNAME) == 0)
				return n;
		}

		return -1;
	}

	// 根据名字返回所在位置
	int FindNum(DWORD sid)
	{
		for (int n = 0; n < MAX_TEAM_MEMBER; n++)
		{	
			if (stTeamPlayer[n].staticId == sid)
				return n;
		}

		return -1;
	}
};

struct FindTeamInfo
{
	struct SmallTeamPlayer
	{
		char	szName[CONST_USERNAME];		// 名字
		WORD	bySex		: 1;			// 性别
		WORD	bySchool	: 3;			// 门派
		WORD	byLevel		: 9;			// 等级
		WORD	isLeader	: 1;			// 是否队长
		WORD	status		: 2;			// 状态
	};
	
	DWORD			teamID;
	SmallTeamPlayer	teamMember[MAX_TEAM_MEMBER];
};

struct TeamRequest
{
	enum
	{
		PTR_MAXINFO = 41,
	};

	DWORD	dwGID;						// ID
	WORD	bySex		: 1;			// 性别
	WORD	bySchool	: 3;			// 门派
	WORD	byLevel		: 9;			// 等级
	WORD	reserve		: 3;			// 保留
	char	szName[CONST_USERNAME];		// 玩家名
	char	szInfo[PTR_MAXINFO];		// 备注
};

struct TRWithIndex : public TeamRequest
{
	DWORD	dwIndex;
};

struct TeamMemberPos
{
	DWORD	dwGID;
	float	fX;
	float	fY;
};

DECLARE_MSG(SAddTempItemMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_ADDTEMPITEM)
struct SAAddTempItemMsg : public SAddTempItemMsg
{
	BYTE					count;
	Team::LeaderPackItem	tempItems[MAX_TEMPPACK_SIZE];
	
	WORD MySize()
	{
		return sizeof(SAAddTempItemMsg) - (MAX_TEMPPACK_SIZE-count) * sizeof(Team::LeaderPackItem);
	}
};

DECLARE_MSG(SDelTempItemMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_DELTEMPITEM)
struct SADelTempItemMsg : public SDelTempItemMsg
{
	BYTE ItemIndex;		// 删除的道具索引
};

DECLARE_MSG(SDispatchToMemberMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_DISPATCHTOMEMBER)
struct SQDispatchToMemberMsg : public SDispatchToMemberMsg
{
	BYTE ItemIndex;		// 道具索引
	BYTE MemIndex;		// 队员索引
};

struct SADispatchToMemberMsg : public SDispatchToMemberMsg
{
	enum
	{
		SDTM_SUCCESS,	// 成功
		SDTM_PACKFULL,	// 成员背包满了
	};
	BYTE Result;		// 结局
	BYTE MemIndex;		// 队员索引
};

DECLARE_MSG(SChangeGiveMode, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_CHANGEGIVEMODE)
struct SQAChangeGiveMode : public SChangeGiveMode
{
	enum
	{
		SCGM_MODE,		// 分配方式
		SCGM_COLOER,	// 分配品级
	};
	BYTE	bOperation;
	BYTE	bMode;
};

DECLARE_MSG(SCancelTeamQuest, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_CANCELTEAMQUEST)
struct SQCancelTeamQuest : public SCancelTeamQuest
{
};

DECLARE_MSG(SWantJoinTeamMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_IWANTTOJION)
struct SQWantJoinTeamMsg : public SWantJoinTeamMsg
{
	DWORD dwTeamID;
};

DECLARE_MSG(SInviteWithNameMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_INVITEWITHNAME)
struct SQInviteWithNameMsg : public SInviteWithNameMsg
{
	char szName[CONST_USERNAME];
};

DECLARE_MSG(SFindTeamInfoMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_GETFINDTEAMINFO)
struct SQFindTeamInfoMsg : public SFindTeamInfoMsg
{
};

DECLARE_MSG(SNotifyOldFindMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_OLDFINDLIST)
struct SANotifyOldFindMsg : public SNotifyOldFindMsg
{
	enum { NOFM_OLDMAX = 200, };

	WORD	wNumber;
	DWORD	dwOldList[NOFM_OLDMAX];

	DWORD GetMySize()
	{
		return (sizeof(SANotifyOldFindMsg) - ((NOFM_OLDMAX-wNumber)*sizeof(DWORD)));
	}
};

DECLARE_MSG(SNotifyNewFindMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_NEWFINDLIST)
struct SANotifyNewFindMsg : public SNotifyNewFindMsg
{
	enum { NOFM_NEWMAX = 200, };

	WORD			wNumber;
	FindTeamInfo	dwNewList[NOFM_NEWMAX];

	DWORD GetMySize()
	{
		return (sizeof(SANotifyNewFindMsg) - ((NOFM_NEWMAX-wNumber)*sizeof(FindTeamInfo)));
	}
};

DECLARE_MSG(SGetTeamQuestMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_GETTEAMQUEST)
struct SQGetTeamQuestMsg : public SGetTeamQuestMsg
{
};

DECLARE_MSG(SNotifyOldQuestMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_OLDQUESTLIST)
struct SANotifyOldQuestMsg : public SNotifyOldQuestMsg
{
	enum { NOQM_OLDMAX = 200, };

	WORD	wNumber;
	DWORD	dwOldList[NOQM_OLDMAX];

	DWORD GetMySize()
	{
		return (sizeof(SANotifyOldQuestMsg) - ((NOQM_OLDMAX-wNumber)*sizeof(DWORD)));
	}
};

DECLARE_MSG(SNotifyNewQuestMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_NEWQUESTLIST)
struct SANotifyNewQuestMsg : public SNotifyNewQuestMsg
{
	enum { NNQM_NEWMAX = 200, };

	WORD		wNumber;
	TRWithIndex	dwNewList[NNQM_NEWMAX];

	DWORD GetMySize()
	{
		return (sizeof(SANotifyNewQuestMsg) - ((NNQM_NEWMAX-wNumber)*sizeof(TRWithIndex)));
	}
};

DECLARE_MSG(SSubmitMakeTeamMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_SUBMIT_REQUEST)
struct SQSubmitMakeTeamMsg : public SSubmitMakeTeamMsg
{
	char szInfo[TeamRequest::PTR_MAXINFO];
};

struct SASubmitMakeTeamMsg : public SSubmitMakeTeamMsg
{
	enum
	{
		SMT_SUCCESS = 1,		// 成功
		SMT_INTEAM,				// 已在队伍里
		SMT_ALREADYSUBMIT,		// 已经提交过了
	};

	BYTE bReulst;
};

DECLARE_MSG(STeamCancelQuestMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_CANCEL_REPLY)
struct SATeamCancelQuestMsg : public STeamCancelQuestMsg
{
	enum
	{
		TCQ_DEAD = 1,		// 死亡
		TCQ_DISCONNECT,		// 掉线
		TCQ_LOGOUT,			// 退出
		TCQ_OVERTIME,		// 超时
	};
	DWORD dwSrcID;			// 组队请求人
	BYTE  bResult;			// 取消的原因
};

DECLARE_MSG(STeamBasePosMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_POSITION)
struct SATeamPosMsg : public STeamBasePosMsg
{
	BYTE			m_Num;
	TeamMemberPos	m_Pos[5];
};

DECLARE_MSG(STeamBaseJoinMsgG, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_JOIN)
struct SQTeamJoinMsgG : public STeamBaseJoinMsgG
{
	enum
	{
		QTJ_AGREE,
		QTJ_REJECT,
	};
	enum
	{
		TJM_JOINME = 1,
		TJM_JOINOTHER,
	};
	BYTE	bResult;					// 结果
	BYTE	type;						// 类型
	DWORD	dwGID;						// 邀请者GID
};

struct SATeamJoinMsgG : public STeamBaseJoinMsgG
{
	enum
	{
		TJM_JOINME = 1,
		TJM_JOINOTHER,
	};
	DWORD	dwGID;						// 邀请者GID
	BYTE	type;						// 邀请类型
	char    cName[CONST_USERNAME];					// 邀请者名字
};

DECLARE_MSG(STeamBaseRefreshMsgG, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_REFRESH)

struct SATeamRefreshMsgG : public STeamBaseRefreshMsgG
{
    BYTE                memberMark;         // bitarray[8]，标示哪个成员的数据需要修改
	Team::TeamPlayer	stMember[8];		// 所有的队员信息

    WORD GetSize()
    {
        if (0 == memberMark)
            return 0;

		int i = 0;
        for (int n = 0; n < 8; ++n)
            if (memberMark & (1 << n))
                ++i;

        return (WORD)(sizeof(SATeamRefreshMsgG) - (8 - i) * sizeof(Team::TeamPlayer));
    }
};

DECLARE_MSG(STeamBaseKickMsgG, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_KICK)
struct SQTeamKickMsgG : public STeamBaseKickMsgG
{
	enum
	{
		TKM_DISMISSED,		// 队伍解散
		TKM_KICKOFF,		// 被踢
		TKM_QUIT,			// 自己退出
	};

	BYTE operation;
    char szName[CONST_USERNAME];
};

struct SATeamKickMsgG : public STeamBaseKickMsgG
{
	enum
	{
		TKM_DISMISSED,		// 队伍解散
		TKM_KICKOFF,		// 被踢
		TKM_QUIT,			// 自己退出
	};

	BYTE	operation;
	char	szName[CONST_USERNAME];
};

DECLARE_MSG(STeamBaseTalkMsgG, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_TALK)
struct SQTeamTalkMsgG : public STeamBaseTalkMsgG
{
    char  szTalkerName[CONST_USERNAME];
    char  szTalkMsg[MAX_CHAT_LEN];
};

struct SATeamTalkMsgG : public STeamBaseTalkMsgG
{
	char  szTalkerName[CONST_USERNAME];
    char  szTalkMsg[MAX_CHAT_LEN];
};

DECLARE_MSG(STeamBaseChangeSkillMsgG, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_CHANGE_SKILL)
struct SQTeamChangeSkillMsgG : public STeamBaseChangeSkillMsgG
{
    BYTE bySkill;
	BYTE byLevel;
};

struct SATeamChangeSkillMsgG : public STeamBaseChangeSkillMsgG
{
    BYTE bySkill;
	BYTE byLevel;
};

// 设置队长
DECLARE_MSG(STeamBaseHeadmenMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_HEADMEN)
struct SQCSTeamBaseHeadmenMsg : public STeamBaseHeadmenMsg
{
	BYTE pos;				// 要设置队长的位置
};

struct SACSTeamBaseHeadmenMsg : public STeamBaseHeadmenMsg
{
	enum
	{
		CSTB_SUCCESS,		// 成功
		CSTB_NO_ACCESS,		// 你不是队长，没有权限
	};

	BYTE pos;				// 设置的位置
};