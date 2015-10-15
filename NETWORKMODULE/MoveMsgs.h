#pragma once

#include "NetModule.h"
#include "movetypedef.h"

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 移动消息类
//=============================================================================================
DECLARE_MSG_MAP(SMoveBaseMsg, SMessage, SMessage::EPRO_MOVE_MESSAGE)
EPRO_SYN_WAYTRACK,          // 同步移动路径信息
EPRO_SYN_POSITION,          // 校正趋向数据信息
EPRO_SET_ZAZEN,             // 打坐/解除消息
EPRO_TEST_POSITION,         // 测试用
EPRO_SYN_WAYJUMP,           // 跳跃消息
EPRO_TITLECHANGED,          // 称号改变
EPRO_NAMECHANGED,           // 名字改变
// add by yuntao.liu
EPRO_UP_SPEED,				// 提升速度
EPRO_NOTIFY_MOVE,	
EPRO_LOCK_PLAYER,

EPRO_SYN_PATH,				// 侠义道3同步移动路径消息
EPRO_SYN_POS,				// 侠义道3同步位置消息
EPRO_SYN_Z,					// 侠义道3同步高度消息
EPRO_SYN_JUMP,				// 侠义道3同步跳跃消息

EPRO_TASK_MOVE,		// 侠义道3同步任务传送信息
EPRO_SYN_UNITY3DJUMP, // UNITY3D里面的跳消息同步

EPRO_SYN_PATHMONSTER,				// 侠义道3同步移动怪物路径消息
END_MSG_MAP()
//---------------------------------------------------------------------------------------------

DECLARE_MSG(STitleChangedMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_TITLECHANGED)
struct  SQTitleChangedMsg    :   public STitleChangedMsg
{
	char cChangedTitle[CONST_USERNAME];
};
struct  SATitleChangedMsg    :   public STitleChangedMsg
{
	char cChangedTitle[CONST_USERNAME];
    DWORD dwPlayerChangedID;
};

DECLARE_MSG(SSynJumpMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_JUMP)
struct SAQSynJumpMsg : public SSynJumpMsg
{
	DWORD	dwGlobalID;
	float	x;
	float	y;
};

DECLARE_MSG(SSynUnity3DJumpMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_UNITY3DJUMP)
struct SAQUnity3DJumpMsg: public SSynUnity3DJumpMsg
{
	DWORD	dwGlobalID;
};

//=============================================================================================
// 跳跃消息
DECLARE_MSG(SSynWayJumpMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_WAYJUMP)
struct  SQASynWayJumpMsg    :   public SSynWayJumpMsg
{
    DWORD   dwGlobalID;         // 本次运行全局唯一标识符
    WORD    wCurX, wCurY;       // 该角色的当前坐标(以发送方的数据为准)
    SNWayNode   NextPos;        // 要跳到的目标点
};
//---------------------------------------------------------------------------------------------

// 侠义道3走（跑）消息
DECLARE_MSG(SSynPathMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_PATH)
struct SQSynPathMsg : public SSynPathMsg
{
	enum { normal = 0, attack };	// 移动目的
	
	DWORD			m_GID;			// 玩家ID
	BYTE			m_move2What;	// 移动目的
	SPath			m_Path;			// 移动路径，必须为最后一个成员
};

struct SASynPathMsg : public SSynPathMsg
{
	DWORD			m_GID;			// 玩家ID
	SPath			m_Path;			// 移动路径，必须为最后一个成员
};

// 侠义道3同步位置消息
DECLARE_MSG(SSynPosMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_POS)
struct SQSynPosMsg : public SSynPosMsg
{
	DWORD			m_GID;			// 玩家ID
	float			m_X;			// 当前的坐标及方向
	float			m_Y;
	float			m_Z;
	float			m_Dir;			
	BYTE			m_Action;		// 动作
};

typedef SQSynPosMsg	SASynPosMsg;

// 侠义道3同步高度消息（只需要客户端发往服务器端进行同步）
DECLARE_MSG(SSynZMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_Z)
struct SQSynZMsg : public SSynZMsg
{
	DWORD			m_GID;			// 玩家ID
	float			m_Z;			// 高度
};

//=============================================================================================
// 走（跑）动消息
DECLARE_MSG(SSynWayTrackMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_WAYTRACK)
// 客户端请求服务器移动
struct SQSynWayTrackMsg : public SSynWayTrackMsg 
{
    DWORD   dwGlobalID;         // 本次运行全局唯一标识符
    SSynWay ssw;                // 同步移动路径

	enum
	{
		normal = 0,
		move_2_attack
	};
	INT32 move2What;
};

// 服务器要求客户端移动
struct SASynWayTrackMsg : public SSynWayTrackMsg
{
    DWORD   dwGlobalID;         // 本次运行全局唯一标识符
    SSynWay ssw;                // 同步移动路径
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// 打坐/解除消息
DECLARE_MSG(SSetZazenMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SET_ZAZEN)
//  该消息只从server发给client，要求客户端根据服务器数据同步当前位置状态
struct SQSetZazenMsg: public SSetZazenMsg
{
    DWORD dwGlobalID;           // 本次运行全局唯一标识符
    BYTE  byAction;             // 行为方式
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// 同步位置消息
DECLARE_MSG(SSynPositionMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_POSITION)
//  该消息只从server发给client，要求客户端根据服务器数据同步当前位置状态
struct SASynPositionMsg: public SSynPositionMsg
{
    DWORD dwGlobalID;           // 本次运行全局唯一标识符
    WORD  wCurX, wCurY;         // 当前坐标
    BYTE  byAction;             // 行为方式
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// Test Msg
DECLARE_MSG(STestPos, SMoveBaseMsg, SMoveBaseMsg::EPRO_TEST_POSITION)
struct SATP : public STestPos
{
    WORD wCurX, wCurY;
    WORD iMsgCount;
    BYTE byCurAction;
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SUpSpeed_S2C, SMoveBaseMsg, SMoveBaseMsg::EPRO_UP_SPEED)
struct SUpSpeed_S2C_MsgBody : public SUpSpeed_S2C
{
	INT32 objectGID;
	float newSpeed;
};

//----------------------------------------------------------------------------------------------------------	
DECLARE_MSG(SNotifyMove_S2C, SMoveBaseMsg, SMoveBaseMsg::EPRO_NOTIFY_MOVE)
struct SNotifyMove_S2C_MsgBody : public SNotifyMove_S2C
{
	INT32 moveTileX;
	INT32 moveTileY;
};

DECLARE_MSG(SLockPlayer, SMoveBaseMsg, SMoveBaseMsg::EPRO_LOCK_PLAYER)
struct SALockPlayer : public SLockPlayer
{
	BYTE byLock;
};

// 侠义道3请求任务传送信息
DECLARE_MSG(Staskmove,SMoveBaseMsg, SMoveBaseMsg::EPRO_TASK_MOVE)
struct SQtaskmove : public Staskmove
{
	WORD taskID;		//请求移动到的NPC任务ID
	WORD wCellPos;		//使用的传送符在背包的位置
};

//服务器要求客户端生成一条寻路的路径点
DECLARE_MSG(SSynPathMonsterMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_PATHMONSTER)
struct SASynPathMonsterMsg : public SSynPathMonsterMsg
{
	SWayPoint	m_path[2];		// 包含原地和移动到的目标点
	DWORD  dwGlobalID;           // 本次运行全局唯一标识符
};

//客户端回复路径信息
struct SQSynPathMonsterMsg : public SSynPathMonsterMsg
{
	DWORD dwGlobalID;           // 本次运行全局唯一标识符
	SPath			m_Path;			// 移动路径
};

