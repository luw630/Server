#ifndef CMONSTERDEFINE_H
#define CMONSTERDEFINE_H
#include "pub\ConstValue.h"

// 怪物的势力
// 0	中立	攻击任何门派
// 1	蜀山	攻击除蜀山声望友好以外势力
// 2	日月	攻击除日月声望友好以外势力
// 3	少林	攻击除少林声望友好以外势力
// 4	五岳	攻击除五岳声望友好以外势力
// 5	慈航	攻击除慈航声望友好以外势力
enum MonsterOrbit
{
	MO_NONE = 0,			// 中立
	MO_SHUSHAN,				// 蜀山
	MO_RIYUE,				// 日月
	MO_SHAOLIN,				// 少林
	MO_WUYUE,				// 五岳
	MO_CIHANG,				// 慈航

	M0_MAX,					// Max
};

// 怪物的身体模型大小
const float MB_SMALL	= 0.5;
const float MB_NORMAL	= 1;
const float MB_LARGE	= 2;

enum MonsterLevelType
{
	MT_NORMAL = 0,			// 普通
	MT_ELITE,				// 精英
	MT_JUNIOR_BOSS,			// 初级BOSS
	MT_MIDDLE_BOSS,			// 中级BOSS
	MT_ADVANCE_BOSS,		// 高级BOSS

	MT_MAX,					// Max
};

enum MonsterAttType
{
	MAT_ACTIVELY,			// 主动攻击
	MAT_PASSIVE,			// 被动攻击
	MAT_RUBBER,				// 橡皮人，打不还手

	MAT_MAX,				// Max
};

enum MonsterMoveType
{
	MMT_NOMOVE = 0,			// 不移动
	MMT_AREA,				// 范围移动
	MMT_ROAD,				// 路点移动
};

// 追击停止条件
enum MonsterAfterCondition
{
	MAC_OVER_PARTOL = 0,	// 超出活动范围
	MAC_MAX,				

	// 大于MAX，就表示是追击的最大距离
	MAC_OVER_DISTANCE,		// 距离大于某个值
};

// 逃跑条件
enum MonsterEscapeCondition
{
	MES_NORUN,				// 从不逃跑
	MES_LOW_HP,				// 血量过低
};

#define MAX_MONTER_SKILLCOUNT 5 ///怪物技能最大数量
// 怪物的基本属性
class SMonsterBaseData
{
public:
	SMonsterBaseData()
	{
		memset(this, 0, sizeof(SMonsterBaseData));
	}

public:
	char	m_Name[CONST_USERNAME];				// 名称，name[10] == 0
	BYTE	m_Orbit;				// 势力
	BYTE	m_LevelType;			// 怪物类型（普通&精英&BOSS）
	BYTE	m_AttType;				// 攻击类型
	DWORD	m_WalkSpeed;			// 非战斗移动速度
	DWORD	m_FightSpeed;			// 战斗移动速度
	DWORD	m_MaxHP;				// 最大血量
	DWORD	m_ReBirthInterval;		// 重生间隔时间
	DWORD	m_PartolRadius;			// 监控半径
	DWORD	m_ActiveRadius; 		// 活动半径
	DWORD	m_TailLength;			// 追击距离
	DWORD	m_Exp;					// 携带经验
	DWORD	m_Sp;					// 携带真气
	WORD	m_ID;					// 怪物ID
	WORD	m_LookID;				// 外观&头像编号
	WORD	m_BodySize;				// 身体模型大小
	WORD	m_level;				// 等级
	WORD	m_AttInterval;			// 攻击间隔
	WORD	m_MaxMP;				// 最大内功
	WORD	m_BloodMute;			// 血怪变身几率
	WORD	m_AttMute;				// 攻怪变身几率
	WORD	m_DefenceMute;			// 防怪变身几率
	WORD	m_KillTaskID;			// 杀怪任务分组
	WORD	m_Direction;			// 怪物朝向
	WORD	m_EndTailCondition;		// 追击停止条件
	WORD	m_MoveType;				// 移动类型
	WORD	m_SearchTargetInterval;	// 搜索目标间隔
	WORD	m_StayTimeMin;			// 怪物停留时间下限
	WORD	m_StayTimeMax;			// 怪物停留时间上限

	WORD	m_GongJi;				// 攻击
	WORD	m_FangYu;				// 防御
	WORD	m_BaoJi;				// 暴击
	WORD	m_ShanBi;				// 闪避
	WORD	m_AtkSpeed;				// 攻击速度	

	WORD m_SkillID[MAX_MONTER_SKILLCOUNT];	///怪物技能ID
	WORD m_StaticAI;						///AI编号
	WORD m_GroupID;							///AI组别
	WORD m_uncrit;//抗暴
	WORD m_wreck;//破击
	WORD m_unwreck;//抗破
	WORD m_puncture;//穿刺
	WORD m_unpuncture;//抗穿
	WORD m_Hit;//命中

};

struct BossDeadData	//Boss类型怪物死亡数据结构
{
	WORD	m_ID;					// 怪物ID
	WORD	m_level;				// 等级
	WORD	reginid;					//场景ID
	WORD  wX;						//场景X	
	WORD  wY;				//场景Y
	WORD	wHour;					//几点
	WORD	wMinute;				//几分
	WORD	wState;					//当前状态
	char	m_KillMyName[CONST_USERNAME];				// 杀死怪物的玩家姓名
	DWORD m_UpdateTime;	//状态更新时间
};

struct ActivityNotice	//日常的活动数据结构
{
	
	char  ActivityName[10] ;		//活动名称(10)
	char	Activitydescribe[50] ;			//活动描述(50)
	char	ActivityNpcName[10] ;			//活动Npc名称(50)

	BYTE		  ActivityType ;				//活动类型
	DWORD	ActivityDate ;			//活动日期
	WORD	 ActivityStartTime ;		//活动开始时间
	WORD	ActivityEntryRegin ;		//活动入口场景id
	WORD	ActivityEntryCoordX ;	//活动入口坐标
	WORD	ActivityEntryCoordY;		//活动入口坐标

	DWORD	ActivityReward[2];			//活动奖励道具
	//DWORD m_UpdateTime;	//状态更新时间
};

#endif // CMONSTERDEFINE_H
