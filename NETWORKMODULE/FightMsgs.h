#pragma once

#include "NetModule.h"
#include "attacktypedef.h"
#include "../pub/ConstValue.h"
#include "PlayerTypedef.h"
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 战斗相关消息类
//=============================================================================================
DECLARE_MSG_MAP(SFightBaseMsg, SMessage, SMessage::EPRO_FIGHT_MESSAGE)
//{{AFX
EPRO_FIGHT_OBJECT,		// 攻击场景上的对象
EPRO_WOUND_OBJECT,      // 场景上的对象被攻击
EPRO_FIGHT_POSITION,	// 攻击场景上的某个地方
EPRO_SET_CURTELERGY,    // 设置当前使用的心法
EPRO_SET_CURSKILL,		// 设置当前使用的武功
EPRO_SET_EXTRASTATE,    // 设置附加状态(点穴、眩晕)
EPRO_RET_VENATIONSTATE, // 返回经脉状态
EPRO_CURE_VENATION,     // 请求治疗经脉
EPRO_CUREOK_VENATION,   // 确认经脉治疗
EPRO_DELETE_TELERGY,    // 删除装备的心法
EPRO_KILLED_COUNT,      // 显示杀阵计数
EPRO_PASSVENA_EFFECT,   // 暴穴效果广播

EPRO_ONGOAT_MONSTERCOUNT,// 替身打怪计数
EPRO_REFRESH_ONUP,       // 离线后上线数据更新
EPRO_WOUND_OBJECTEX,     // 场景上的对象被攻击
EPRO_UPDATE_XINMOVALUE,  // 更新心魔值
EPRO_UPDATE_CUREQUIPDURANCE,// 更新耐久

EPRO_UPDATE_DECDURTIME,  // 更新不掉耐久时间 

EPRO_SWITCH_TO_SKILL_ATTACK_STATE,	// 通知客户端切换到远程技能攻击状态下
EPRO_SWITCH_TO_PREPARE_SKILL_ATTACK_STATE,
EPRO_CANCEL_SKILL_ATTACK_PROCESS_BAR,
EPRO_NOTIFY_START_COLD_TIMER,
EPRO_MOUNTSKILL_COLDTIMER,
EPRO_MOVE_TO_TARGET_FOR_ATTACK,
EPRO_SHOW_ENEMY_INFO,
EPRO_QUEST_SKILL,
EPRO_SHOW_SKILL_HINT_INFO,
EPRO_OPEN_SKILL_PROCESS_BAR,

EPRO_QUEST_SKILL_FP,

EPRO_LEARN_SKILL,

EPRO_UPDATE_SKILL_BOX,

EPRO_FORCE_QUEST_ENEMY_INFO,

EPRO_SELECT_TARGET,
EPRO_TELERGY_STATE,		//add by xj 设置心法状态
EPRO_SETBUFFICON_CHANGE,		//buff改变
EPRO_SETCOLLECT,				//开始采集
EPRO_CANCELCOLLECT,				//取消采集
EPRO_UPDATEBUFF_INFO,			//即时更新目标所有buff

//add by xj
EPRO_PRACTICE_REQUEST,			// 请求闭关修炼
EPRO_PRACTICE_ANSWER,			// 请求结果
EPRO_PRACTICE_ITEM_UPDATE,		// 道具跟新(因为挂机面板的道具和包裹的道具实际上已经分开，所以不能用同一消息)

EPRO_PRACTICE_UPDATEBOX,		// 保存格子信息

EPRO_PRACTICE_RESULT,			// 修炼的结果
EPRO_PRACTICE_STAR_TYPE,		// 广播修炼的类型
EPRO_NOTIFY_FIGHTSTATE,			// 进入/离开战斗状态时发送
EPRO_WIND_MOVE_CHECK,			// 击退客户端验证消息
EPRO_WIND_MOVE,					// 瞬移消息
EPRO_SKILL_CANCEL,				// 技能打断消息
EPRO_MONSTER_WRCHANGE,			// 怪物的走/跑切换

EPRO_UPDATE_EQUIPWEAR_IN_BAG,	// 更新背包里装备的耐久
EPRO_JIU_ZI_SHA,				// 九字杀阵数量更新


EPRO_PKVALUE_UPDATE,									// 玩家杀孽值更新
EPRO_PLAYERNAMECOLOR_CHANGE,				// 玩家姓名颜色改变

EPRO_MONSTER_BOSSUPDATE,				// BOSS怪物更新

EPRO_PlayerTemplateSkill,				// 玩家模板技能
EPRO_QUEST_MULTIPLESKILL,			//请求多个目标的技能
// EPRO_ACTIVITY_UPDATE,				// 活动信息更新
// EPRO_NEWS_UPDATE,				// 活动新闻信息更新
//}}AFX

//add by ly 2014/3/17
EPRO_XINYANG_SHENGXING,				//信仰升星请求消息

END_MSG_MAP()
//---------------------------------------------------------------------------------------------

DECLARE_MSG(SMonsterWRChange, SFightBaseMsg, SFightBaseMsg::EPRO_MONSTER_WRCHANGE)
struct SAMonsterWRChange : public SMonsterWRChange
{
	enum
	{
		SMWR_RUN,
		SMWR_WALK,
	};

	BYTE	bStatus;		// 当前状态
	DWORD	dwGlobal;		// 怪物ID
};

DECLARE_MSG(SSkillCancel, SFightBaseMsg, SFightBaseMsg::EPRO_SKILL_CANCEL)
struct SASkillCancel : public SSkillCancel
{
	enum
	{
		SSC_PREPARE,		// 准备
		SSC_SING,			// 吟唱
		SSC_BEFORE,			// 准备攻击前事件
		SSC_ATTACK,			// 攻击
		SSC_AFTER,			// 攻击完毕后事件
	};

	DWORD	dwGlobal;		// 目标
	BYTE	bPhase;			// 在哪个阶段被打断
	BYTE	bSkillIndex;	// 技能索引
};

DECLARE_MSG(SWindMoveCheckMsg, SFightBaseMsg, SFightBaseMsg::EPRO_WIND_MOVE_CHECK)
struct SAWindMoveCheckMsg : public SWindMoveCheckMsg
{
	DWORD dtagertgid;//后退目标GID
	float	fSouceX;   //起点坐标
	float	fSouceY;
	float fdir;     //后退方向
	float fDestX;   //后退目标点
	float fDestY;
};

struct SQWinMoveCheckMsg : public SWindMoveCheckMsg
{
	DWORD dtagertgid;//后退目标GID
	float	fDestX;
	float	fDestY;
};

DECLARE_MSG(SWindMoveMsg, SFightBaseMsg, SFightBaseMsg::EPRO_WIND_MOVE)
struct SAWindMoveMsg : public SWindMoveMsg
{
	enum
	{
		SAM_PULL,		// 拉人
		SAM_RUSH,		// 冲锋
	};

	BYTE	bType;
	DWORD	dwSelf;
	DWORD	dwTarget;
	float	fDestX;
	float	fDestY;
	WORD	bSkillIndex;	// 技能索引
};


// 通知玩家是否进入了战斗状态或离开了战斗状态
DECLARE_MSG(SFightStateMsg, SFightBaseMsg, SFightBaseMsg::EPRO_NOTIFY_FIGHTSTATE)
struct SAFightStateMsg : public SFightStateMsg
{
	DWORD	dwGlobalID;		// 目标
	bool	IsInFight;		// 是否处于战斗状态中（true->是，false->否）
};

//=============================================================================================
// 客户端请求攻击场景上的某个对象，以及服务器同意攻击的回应
DECLARE_MSG(SFightObjectMsg, SFightBaseMsg, SFightBaseMsg::EPRO_FIGHT_OBJECT)
struct SQAFightObjectMsg : public SFightObjectMsg
{
	DWORD dwDestGlobalID;	// 攻击目标的ID
	SAttack	sa;				// 攻击的参数
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 服务器要求客户端相应一个被攻击的动作
DECLARE_MSG(SWoundObjectMsg, SFightBaseMsg, SFightBaseMsg::EPRO_WOUND_OBJECT)
struct SAWoundObjectMsg : public SWoundObjectMsg
{
	DWORD   dwGlobalID;	    // 被攻击方的ID
	SWound	sw;				// 被攻击方的响应数据
};

DECLARE_MSG(SWoundObjectMsgEx, SFightBaseMsg, SFightBaseMsg::EPRO_WOUND_OBJECTEX)
struct SAWoundObjectMsgEx : public SWoundObjectMsgEx
{
	enum
	{
		MAX_ENEMY_NUMBER = 50,
	};

    BYTE	mWoundObjectNumber;		// 受伤的目标数量...限制50个
    WORD	mSkillIndex;			// 招式编号（如果是玩家，则该值为招式编号，否则为武器效果）
	DWORD	mAttackerGID;			// 攻击方的ID, 用于区分SAWoundObjectMsg
	DWORD	mMaxHP;					// 最大血量
	DWORD	mCurHP;					// 当前血量
// 	float	mAttackerWorldPosX;		// 攻击者的的世界坐标（对于某些技能来说是攻击点）
// 	float	mAttackerWorldPosY;

	// 足够大的同步空间，最多可以保存50个受伤目标数据
	char streamData[sizeof(SWoundTiny) * MAX_ENEMY_NUMBER + 8 + 4];
};

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 范围攻击相关消息类
//=============================================================================================
DECLARE_MSG_MAP(SFightPositionMsg, SFightBaseMsg, SFightBaseMsg::EPRO_FIGHT_POSITION)
//{{AFX
EPRO_ATTACK,		// 攻击
EPRO_WOUND,			// 被攻击
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 客户端请求坐标攻击 以及 服务器要求客户端响应坐标攻击
DECLARE_MSG(SPosAttackMsg, SFightPositionMsg, SFightPositionMsg::EPRO_ATTACK)
struct SQAPosAttackMsg : public SPosAttackMsg
{
	SAttack	sa;				// 攻击的参数
	WORD wPosX;				// 攻击目标的位置
	WORD wPosY;				// ...
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
#if 0 // 此消息废弃--yuntao.liu
// 服务器要求客户端响应某人受伤
DECLARE_MSG(SPosWoundMsg, SFightPositionMsg, SFightPositionMsg::EPRO_WOUND)
struct SAPosWoundMsg : public SPosWoundMsg
{
	SWound	sw;				// 被攻击方的响应数据
};
#endif
//---------------------------------------------------------------------------------------------

// 客户端选择当前心法
DECLARE_MSG(SSetCurTelergyMsg, SFightBaseMsg, SFightBaseMsg::EPRO_SET_CURTELERGY)
struct SQSetCurTelergyMsg : public SSetCurTelergyMsg
{
	BYTE    byCurTelergy;   // 要选择使用的心法
};

//////////////////////////////////////////////////////////////////////////
//add by xj
DECLARE_MSG(SSetTelergyStateMsg, SFightBaseMsg, SFightBaseMsg::EPRO_TELERGY_STATE)
struct SQSetTelergyStateMsg : public SSetTelergyStateMsg
{
	BYTE    byTelergy;   // 设置心法状态
	//STelergy::STATETEL	byState;
};



//---------------------------------------------------------------------------------------------

//=============================================================================================
// 客户端选择当前武功
DECLARE_MSG(SSetCurSelSkillMsg, SFightBaseMsg, SFightBaseMsg::EPRO_SET_CURSKILL)
struct SQSetCurSelSkillMsg : public SSetCurSelSkillMsg
{
	WORD    curSelSkill;   // 要选择使用的武功
};

enum FIGHT_STATE
{
	FS_NONE			= 0,				// 无	
	FS_DIZZY		= 0x00000001,		// 眩晕
	FS_LIMIT_SKILL	= 0x00000002,		// 封招
	FS_HYPNOTISM	= 0x00000004,		// 催眠
	FS_DINGSHENG	= 0x00000008,		// 定身
	FS_WUDI = 0x00000010,		// 无敌
	FS_ADDBUFF = 0x00000020,		// 无法附加任何BUFF
	FS_JITUI = 0x00000040,		// 被击退
};

// 设置战斗附加状态
DECLARE_MSG(SSetExtraStateyMsg, SFightBaseMsg, SFightBaseMsg::EPRO_SET_EXTRASTATE)
struct SASetExtraStateyMsg : public SSetExtraStateyMsg
{
	DWORD dwGlobalID;	        // 目标的ID
	DWORD dwFightExtraState;    // 所设置附加的状态
	DWORD	dwpushID;			//释放者
	float  fdestX;			//目标X
	float  fdestY;			//目标Y
	DWORD  dskillid;	//技能Index
};

//---------------------------------------------------------------------------------------------

//=============================================================================================
// 返回经脉状态
DECLARE_MSG(SVenationStateMsg, SFightBaseMsg, SFightBaseMsg::EPRO_RET_VENATIONSTATE)
struct SAVenationStateMsg : public SVenationStateMsg
{
    DWORD   dwGlobalID;	    // 需要更新经脉状态的ID
	BYTE    byWhoVenation;  // 四种经脉
    BYTE    byState;        // 状态，0=完好
    BYTE    byIsUpdateMsg;  // 游戏进行中的更新消息，用于判断界面是否显示提示
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 请求治疗经脉
DECLARE_MSG(SCureVenationMsg, SFightBaseMsg, SFightBaseMsg::EPRO_CURE_VENATION)
struct SQCureVenationMsg : public SCureVenationMsg
{
	DWORD   dwGlobalID;	    // 被治疗方的ID
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 确认经脉治疗
DECLARE_MSG(SCureVenationOKMsg, SFightBaseMsg, SFightBaseMsg::EPRO_CUREOK_VENATION)
struct SACureVenationOKMsg : public SCureVenationOKMsg
{
	DWORD   dwGlobalID;	    // 治疗方的ID
    DWORD   dwDestGID;	    // 被治疗方的ID
	BYTE    byDir;          // 治疗动作的方向
};
//---------------------------------------------------------------------------------------------

// 删除装备的心法
DECLARE_MSG(SDeleteTelergyMsg, SFightBaseMsg, SFightBaseMsg::EPRO_DELETE_TELERGY)
struct SQDeleteTelergyMsg : public SDeleteTelergyMsg
{
	BYTE    byTelergyNum;          // 心法所处的位置
	char	szUserpass[CONST_USERPASS];
};

struct SADeleteTelergyMsg : public SDeleteTelergyMsg
{
    BYTE    byResult; // 1 成功 0失败，二级密码不正确
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 显示杀阵计数
DECLARE_MSG(SKilledCountMsg, SFightBaseMsg, SFightBaseMsg::EPRO_KILLED_COUNT)
struct SAKilledCountMsg : public SKilledCountMsg
{
	BYTE    byKilledCount;          // 杀死的数量
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 暴穴效果广播
DECLARE_MSG(SPassvenaEffMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PASSVENA_EFFECT)
struct SAPassvenaEffMsg : public SPassvenaEffMsg
{
	DWORD   dwGlobalID;	            // 暴穴者ID
	BYTE    byPassvenaType;         // 杀死的数量
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 替身打怪计数
DECLARE_MSG(SOnGoatMonsterCountMsg, SFightBaseMsg, SFightBaseMsg::EPRO_ONGOAT_MONSTERCOUNT)
struct SAOnGoatMonsterCountMsg : public SOnGoatMonsterCountMsg
{
	DWORD   dwGlobalID;	            // 接收玩家的ID
	WORD    wOnGoatMonsterCount;    // 当前数量
    WORD    wMutateTime;            // 变身时间（=0无效）
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 离线后上线数据更新
DECLARE_MSG(SRefreshOnUpMsg, SFightBaseMsg, SFightBaseMsg::EPRO_REFRESH_ONUP)
struct SARefreshOnUpMsg : public SRefreshOnUpMsg
{
	DWORD   dwGlobalID;	            // 接收玩家的ID
    WORD    m_wMonsterCountOnGoat;  // 套装替身杀死怪物的计数
    BYTE    m_byKilledCount;        // 杀阵计数器
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 更新心魔值
DECLARE_MSG(SUpdateXMValueMsg, SFightBaseMsg, SFightBaseMsg::EPRO_UPDATE_XINMOVALUE)
struct SAUpdateXMValueMsg : public SUpdateXMValueMsg
{
    WORD m_wXinMoValue;         // 心魔值
};
//-----

// 更新耐久
DECLARE_MSG(SUpdateCurEqDuranceMsg, SFightBaseMsg, SFightBaseMsg::EPRO_UPDATE_CUREQUIPDURANCE)
struct SAUpdateCurEqDuranceMsg : public SUpdateCurEqDuranceMsg
{
    DWORD   wEqIndex;
    BYTE    byPos;
    WORD    wCurEqDuranceCur;
	WORD	wMaxEqDuranceCur;
};

//=============================================================================================
// 更新不掉耐久时间
DECLARE_MSG(SUpdateDecDurTimeMsg, SFightBaseMsg, SFightBaseMsg::EPRO_UPDATE_DECDURTIME)
struct SAUpdateDecDurTimeMsg : public SUpdateDecDurTimeMsg
{
    DWORD    dwDecDurTimeEnd;      // 持久到期时间
};

// 技能武功攻击请求
DECLARE_MSG(SQuestSkill_C2S, SFightBaseMsg, SFightBaseMsg::EPRO_QUEST_SKILL)
struct SQuestSkill_C2S_MsgBody : public SQuestSkill_C2S
{
	DWORD	mAttackerGID;			// 攻击者的ID
	DWORD	mDefenderGID;			// 被攻击者的ID
	float	mDefenderWorldPosX;		// 被攻击者的世界坐标（对于某些技能来说是攻击点）
	float	mDefenderWorldPosY;
	BYTE	dwSkillIndex;			// 技能索引
};

struct QuestSkill
{
	DWORD	mDefenderGID;			// 被攻击者的ID
	float	mDefenderWorldPosX;		// 被攻击者的世界坐标（对于某些技能来说是攻击点）
	float	mDefenderWorldPosY;
	WORD  bDefenderState;//目标状态  0 无 1 击退状态
};

// 技能武功攻击请求
DECLARE_MSG(SQuestSkillMultiple_C2S, SFightBaseMsg, SFightBaseMsg::EPRO_QUEST_MULTIPLESKILL)
struct SQuestSkillMultiple_C2S_MsgBody : public SQuestSkillMultiple_C2S
{
	BYTE		dwSkillIndex;			// 技能索引
	BYTE		bTargetNum;		//数量
	DWORD	mAttackerGID;			// 攻击者的ID
	float	mAttackerWorldPosX;		// 攻击者的的世界坐标（对于某些技能来说是攻击点）
	float	mAttackerWorldPosY;
	BYTE		buffer[1024];
};

//#pragma message ("SUpdateSkillBox_C2S_MsgBody 数据结构过于庞大，不适合网络传递")
DECLARE_MSG(SUpdateSkillBox_C2S, SFightBaseMsg, SFightBaseMsg::EPRO_UPDATE_SKILL_BOX)
struct SUpdateSkillBox_C2S_MsgBody : public SUpdateSkillBox_C2S
{
	INT32 playerGID;
//	SHORTCUT stBox[ 6 ][ 9 ]; 
};

DECLARE_MSG(SForceQuestEnemyInfo_C2S, SFightBaseMsg, SFightBaseMsg::EPRO_FORCE_QUEST_ENEMY_INFO)
struct SForceQuestEnemyInfo_C2S_MsgBody : public SForceQuestEnemyInfo_C2S
{
};

//----------------------------------------------------------------------------------------------------------	
 DECLARE_MSG(SOpenSkillProcessBar_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_OPEN_SKILL_PROCESS_BAR)
 struct SOpenSkillProcessBar_S2C_MsgBody : public SOpenSkillProcessBar_S2C
 {
 		INT32 mAttackGID;  //攻击者的gid
 		INT32 mCastTime;
 }; 

DECLARE_MSG(SSwitch2SkillAttackState_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_SWITCH_TO_SKILL_ATTACK_STATE)
struct SSwitch2SkillAttackState_S2C_MsgBody : public SSwitch2SkillAttackState_S2C
{
	BYTE   btargetnum;
	INT32	mAttackGID;		// 攻击者的GID
	INT32	m_CurSkillID;
	float mAttackerWorldPosX;//攻击者的坐标
	float mAttackerWorldPosY;
	BYTE		buffer[1024];  //数据
	
// 	INT32	mDefenderGID;	// 被攻击的GID
// 	float	mDefenderWorldPosX;		// 被攻击者的世界坐标（对于某些技能来说是攻击点）
// 	float	mDefenderWorldPosY;
};


DECLARE_MSG(SSwitch2PrepareSkillAttackState_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_SWITCH_TO_PREPARE_SKILL_ATTACK_STATE)
struct SSwitch2PrepareSkillAttackState_S2C_MsgBody : public SSwitch2PrepareSkillAttackState_S2C
{
	INT32 mAttackGID;		// 攻击者的GID
	INT32 mDefenderGID;		// 被攻击的GID
	INT32 mCastTime;		// 吟唱时间
	INT32 m_CurSkillID;		// 当前技能ID
	float fsingDir;//释放吟唱技能时的方向
};


DECLARE_MSG(SCancelSkillAttackProcessBar_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_CANCEL_SKILL_ATTACK_PROCESS_BAR)
struct SCancelSkillAttackProcessBar_S2C_MsgBody : public SCancelSkillAttackProcessBar_S2C
{
	INT32 mAttackGID;  //攻击者的gid
};

//----------------------------------------------------------------------------------------------------------	
DECLARE_MSG(SMove2TargetForAttack_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_MOVE_TO_TARGET_FOR_ATTACK)
struct SMove2TargetForAttack_S2C_MsgBody : public SMove2TargetForAttack_S2C
{
	INT32 mAttackGID;		// 攻击者的ID
	INT32 mDefenderGID;		// 被攻击者的ID
	INT32 distance;			// 攻击距离
};

//----------------------------------------------------------------------------------------------------------	
DECLARE_MSG(SShowEnemyInfo_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_SHOW_ENEMY_INFO)
struct SShowEnemyInfo_S2C_MsgBody : public SShowEnemyInfo_S2C
{
	INT32 mPlayerGID;  
	INT32 mEnemyGID;
};

DECLARE_MSG(SShowSkillHintInfo_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_SHOW_SKILL_HINT_INFO)
struct SShowSkillHintInfo_S2C_MsgBody : public SShowSkillHintInfo_S2C
{
	enum
	{
		invalid_skill_index = 0,
		invalid_weapon,
		invalid_common_CD_time,
		invalid_gropu_CD_time,
		invalid_skill_CD_time,
		invalid_consume_hp,
		invalid_consume_mp,
		invalid_consume_sp,
		invalid_state,
		invalid_object,
		object_is_dead,
		invalid_skill_distance,
		send_too_quick,
		move_failed,
		info_max,
	};
	INT32	what;
	INT32	skillID;
};

DECLARE_MSG(SNotifyStartColdTimer_S2C, SFightBaseMsg, SFightBaseMsg::EPRO_NOTIFY_START_COLD_TIMER)
struct SNotifyStartColdTimer_S2C_MsgBody : public SNotifyStartColdTimer_S2C
{
	DWORD	CDType;
	DWORD	coldeTime;
};

//坐骑技能冷却时间
DECLARE_MSG(SMountSkillColdTimer, SFightBaseMsg, SFightBaseMsg::EPRO_MOUNTSKILL_COLDTIMER)
struct SAMountSkillColdTimer : public SMountSkillColdTimer
{
	BYTE  SkillIndex;  
	BYTE  MountIndex;
	INT32 ColdeTime;
};

enum
{
	SKILL = 1,
	FLY,
	PROTECTED,
	TELERGY,
	SPECIAL,
	MOUNT_SCRIPT_SKILL,		// 坐骑脚本技能
};

DECLARE_MSG(SLearnSkillMsg_C2S, SFightBaseMsg, SFightBaseMsg::EPRO_LEARN_SKILL)
struct SQLearnSkillMsg : public SLearnSkillMsg_C2S
{
	BYTE  byType;
	INT32 dwSkillIndex;
};

DECLARE_MSG(SSkillFPMsg_C2S, SFightBaseMsg, SFightBaseMsg::EPRO_QUEST_SKILL_FP)
struct SQSkillFPMsg : public SSkillFPMsg_C2S
{
	BYTE  byType;			// FLY or PROTECTED
	INT32 dwSkillIndex;
};

// 玩家选中一个目标
DECLARE_MSG(SSelectTarget, SFightBaseMsg, SFightBaseMsg::EPRO_SELECT_TARGET)
struct SQSelectTarget : public SSelectTarget
{
	DWORD dwTargetGID;		// 目标的全局ID
};

struct SASelectTarget : public SSelectTarget
{
	DWORD dwTargetGID;		// 目标的全局ID
	DWORD dwMaxHp;			// 最大血
	DWORD dwCurHp;			// 当前血
	DWORD dwMaxMp;			// 最大蓝
	DWORD dwCurMp;			// 当前蓝
	WORD  wLevel;			// 等级
};

//add by xj 
DECLARE_MSG(SSSetBuffIcon, SFightBaseMsg, SFightBaseMsg::EPRO_SETBUFFICON_CHANGE)
struct SASetBuffIcon : public SSSetBuffIcon
{
	DWORD	dwTargetGID;		// 目标的全局ID
	DWORD	dwBuffID;			//buffID
	WORD	buffType;			// BUFF类型
	DWORD	ticks;				// BUFF持续时间（如果有值，但客户端没有记录该BUFF，则为新增，否则为更新；如果无值，同时客户端有改BUFF，则为销毁）
	BYTE 	iProcessTime;			//buff 开始后，已经流逝的时间(百分比)
};

DECLARE_MSG(SSSetBuffInfo, SFightBaseMsg, SFightBaseMsg::EPRO_UPDATEBUFF_INFO)
struct SQSetBuffInfo : public SSSetBuffInfo
{
	DWORD	dwTargetGID;	//请求的到这个目标ID的所有buff信息			
};
struct BUFFINFO
{	
	BUFFINFO() 
		: dwBuffID( 0), bProcessTime( 0), ticks(0)
	{}

	DWORD	dwBuffID;			//buffID
	BYTE	bProcessTime;		//当前倒计时时间百分比
	DWORD	ticks;				// BUFF持续时间
};
struct SASetBuffInfo : public SSSetBuffInfo
{
	enum
	{
		MAX_BUFF_NUM = 32,
	};
	DWORD	dwTargetGID;	 //请求的到这个目标ID的所有buff信息
	BOOL	bIsTeamMsg;		//是否是组队同步的消息
	BUFFINFO stbuffinfo[MAX_BUFF_NUM]; //目标buff列表，默认最大个数32..
};

DECLARE_MSG(SSetCollect, SFightBaseMsg, SFightBaseMsg::EPRO_SETCOLLECT)
struct SASetCollectionMsg : public SSetCollect
{
	DWORD	dwGID;
	DWORD	dwNpcId;
	DWORD	dwTimeRemain;
	WORD	x;
	WORD	y;
};

DECLARE_MSG(SCancelCollect, SFightBaseMsg, SFightBaseMsg::EPRO_CANCELCOLLECT)
struct SACancelCollectMsg : public SCancelCollect
{
	DWORD	dwGID;
};
////////////////////////////////////////////////////////////////////////// add by xj
//请求闭关修炼技能
DECLARE_MSG(SCPracRequesttMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PRACTICE_REQUEST)
struct SAPracRequesttMsg : public SCPracRequesttMsg
{
	DWORD	dwGID;			//gid
	bool	bPractice;		//是否取消修炼
	BYTE    byProLevel;		//护体挂机修炼的等级地图(1,2,3)速度不一样
	SAPracRequesttMsg()
	{
		byProLevel	=	1;
	}
//	stPractice stPracValue[6];		//格子结构
};
//请求结果
DECLARE_MSG(SCPracAnswertMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PRACTICE_ANSWER)
struct SAPracAnswertMsg : public SCPracAnswertMsg
{
	DWORD	dwGID;		
	bool	bSuccess;	//是否请求成功
};
//跟新挂机面板道具
DECLARE_MSG(SCPracItemUpdateMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PRACTICE_ITEM_UPDATE)
struct SAPracItemUpdateMsg : public SCPracItemUpdateMsg
{
	DWORD	dwGID;		
	WORD	wWhichNum;	//那一个格子
	WORD	wNum;		//剩余个数
	SAPracItemUpdateMsg()
	{
		wWhichNum = 0;
		wNum		=	0;
	}
};
// 和stBox一起更新，这个消息没有用了
 DECLARE_MSG(SUpdatePracticeBox_C2S, SFightBaseMsg, SFightBaseMsg::EPRO_PRACTICE_UPDATEBOX)
 struct SUpdatePracticeBox_C2S_MsgBody : public SUpdatePracticeBox_C2S
 {
 	INT32 playerGID;
 //	stPractice stPracBox[ 6 ]; 
 };
//得到修炼结束的结果
DECLARE_MSG(SCPracResultMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PRACTICE_RESULT)
struct SAPracResultMsg : public SCPracResultMsg
{
	WORD	wType;				//武功类型
	WORD	wSkillID;			//武功ID
	DWORD	wResultValue;		//增长的熟练度
};
//广播状态
DECLARE_MSG(SCPracStarIDMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PRACTICE_STAR_TYPE)
struct SAPracStarIDMsg : public SCPracStarIDMsg
{
	DWORD	dwPlayID;			//id
	WORD	wSkillType;			//类型
	bool	bIsStar;			//true 开始修炼  false 取消修炼
};

// 更新背包里装备的耐久
DECLARE_MSG(SUpdateEquipWearInBagMsg, SFightBaseMsg, SFightBaseMsg::EPRO_UPDATE_EQUIPWEAR_IN_BAG)
struct SAUpdateEquipWearInBagMsg : public SUpdateEquipWearInBagMsg
{
	DWORD   wEqIndex;
	BYTE    byPos;
	WORD    wCurEqDuranceCur;
	WORD	wMaxEqDuranceCur;
};

// 九字杀阵通知消息
DECLARE_MSG(SSNineWordMsg, SFightBaseMsg, SFightBaseMsg::EPRO_JIU_ZI_SHA)
struct SANineWordMsg : public SSNineWordMsg
{
	int		num;	// 九字杀阵，1 - 9，亮九个灯。当值为10的时候，客户端九字杀阵灯。当0的时候，清除九字杀阵的显示。
	DWORD	gid;	// 第10个怪物的时候，发送攻击者的GID
};

// 玩家杀孽值更新
DECLARE_MSG(SPkvalueChangeMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PKVALUE_UPDATE)
struct SAPkvalueChangeMsg : public SPkvalueChangeMsg
{
	WORD nPkvalue;	//杀孽值	
};

// 玩家姓名颜色改变
DECLARE_MSG(SPNameColorChangeMsg, SFightBaseMsg, SFightBaseMsg::EPRO_PLAYERNAMECOLOR_CHANGE)
struct SAPNameColorChangeMsg : public SPNameColorChangeMsg
{
	enum
	{
		COLOR_NORMAL,			//正常颜色,
		COLOR_GREEN,            //绿色
		COLOR_BLUE,             //蓝色
		COLOR_VIOLET,           //紫色
		COLOR_ORANGE,           //橙色
		COLOR_YELLOW,			//黄色
		COLOR_LIGHTRED,		//浅红色
		COLOR_RED,					//红色
	};
	BYTE bNameColor;	//颜色
	DWORD m_gid;			//玩家gid
};
#define MAX_BOSSUPDATE_SIZE 10240
// BOSS怪物更新
DECLARE_MSG(SBossDeadUpdatemsg, SFightBaseMsg, SFightBaseMsg::EPRO_MONSTER_BOSSUPDATE)
struct SQBossDeadUpdatemsg : public SBossDeadUpdatemsg
{
	enum
	{
		UPDATE_BOSSMSG,					//Boss更新
		UPDATE_ACTIVITYMSG,			//日常活动
		UPDATE_NEWSMSG,			//日常活动
	};
	BYTE		bType;
	DWORD nBossState;	//状态值	
};

// BOSS怪物更新
struct SABossDeadUpdatemsg : public SBossDeadUpdatemsg
{
	enum
	{
		UPDATE_BOSSMSG,					//Boss更新
		UPDATE_ACTIVITYMSG,			//日常活动
		UPDATE_NEWSMSG,			//日常活动
	};
	BYTE		bType;
	DWORD nBossState;	//状态值
	BYTE Buff[MAX_BOSSUPDATE_SIZE]; //Boss数据
};

struct TemplateSkillInfo
{
	BYTE SkillIndex;
	long SkillID;
};
DECLARE_MSG(SPlayerTemplateSkillmsg, SFightBaseMsg, SFightBaseMsg::EPRO_PlayerTemplateSkill)
struct SAPlayerTemplateSkillmsg : public SPlayerTemplateSkillmsg
{
	BYTE Flag; //0,加载技能 1去掉这些技能
	BYTE m_num; //技能个数
	TemplateSkillInfo temp[10];
	SAPlayerTemplateSkillmsg():Flag(0),m_num(0)
	{
		memset(&temp[0],0,sizeof(TemplateSkillInfo) * 10);
	}
}; 

//add by ly 2014/3/17	信仰升星相关请求和应答消息
DECLARE_MSG(SXinYangRiseStarmsg, SFightBaseMsg, SFightBaseMsg::EPRO_XINYANG_SHENGXING)
struct SQXinYangRiseStarmsg : public SXinYangRiseStarmsg
{
	BYTE m_XinXiuID; //起始星宿ID
	BYTE MoneyType;	//消耗钱的类型
	BYTE m_num; //升星次数 最大为12次
};

struct SAXinYangRiseStarmsg : public SXinYangRiseStarmsg
{
	BYTE m_SuccessNum;	//升星成功次数 m_SuccessNum = -1时，表示钱不够。0表示一次都没有升星成功
	DWORD m_ConsumeMoney;	//消耗钱的数量
	DWORD m_ConsumeSP;		//消耗真气的数量
};


