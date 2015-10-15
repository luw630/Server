#pragma once

// 技能的门派
// 1 五岳
// 2 蜀山
// 3 少林
// 4 慈航
// 5 日月
// 6 其他（绝学）
// 7 怪物
enum SkillSchool
{
	SS_WUYUE = 1,			// 五岳
	SS_SHUSHAN,				// 蜀山
	SS_SHAOLIN,				// 少林
	SS_CIHANG,				// 慈航
	SS_RIYUE,				// 日月
	SS_XIAKE,				// 侠客
	SS_OTHER,				// 其他
	SS_MONSTER,				// 怪物
	SS_MAX,
};

// 攻击模式
enum SkillAtkScopeType
{
	SAST_SINGLE,			// 单攻（存在单一目标）				子弹可用
	SAST_EMPTY_CIRCLE,		// 圆形群攻（自身原点）				子弹不可用
	SAST_EMPTY_SECTOR,		// 扇形群攻（自身原点）矩形			子弹不可用
	SAST_EMPTY_POINTS,		// 直线群攻（自身原点）				子弹不可用
	SAST_POINT_CIRCLE,		// 圆形群攻（目标点）				子弹可用
	SAST_SELF,				// 对自己释放						子弹不可用
	SAST_POINT_SINGLE,		// 对目标点释放（瞬移，丢道具等）	子弹可用
	//SAST_EMPTY_RECT,		// 直线矩形群攻（自身原点）				
};

// 技能属性
// 用在技能升级消耗计算公式中
enum SkillPropertyType
{
	SPT_COMMAN,				// 普通攻击
	SPT_NEARBY,				// 近身技能
	SPT_LONGDISTANCE,		// 远程技能
	SPT_RANGE,				// 范围技能
	SPT_AUXILIARY_ACTIVE,	// 辅助主动技能
	SPT_AUXILIARY_PASSIVE,  // 辅助抵抗技能
	SPT_PASSIVE,			// 被动技能
	SPT_CHARGE,				// 冲锋技能，需要验证路径
	SPT_REPEL,				// 击退目标
	SPT_MOVE,				// 位移技能，自身移动同时目标后退
	SPT_SPECIAL,			// 特殊技能(暂时没用了 2011.08.26 by dj)
};

//技能子属性
enum SkillSubProperty
{
	SSP_CHARGE_AYB = 1,	//类似“WOW 暗影步”
	SSP_CHARGE_SX ,		//类似“WOW 闪现”
	SSP_CHARGE_CF ,		//类似“WOW 冲锋”
	SSP_CHARGE_TY ,		//类似“WOW 跳跃”
	SSP_CHARGE_LR ,		//类似“WOW 拉人”
};

// 用在伤害计算公式中
enum SkillDamageCalcType
{
	SDCT_PHY_NEAR,			// 外功近程
	SDCT_PHY_FAR,			// 外功远程
	SDCT_FP_NEAR,			// 内功近程
	SDCT_FP_FAR,			// 内功远程
};

// 技能类型
enum SKillType
{
	SKT_ACTIVE,				// 主动
	SKT_PASSITIVE = 2,			// 被动
};

// 一次攻击的几个阶段
enum SKillStep
{
	SKS_PREPARE,			// 准备
	SKS_SING,				// 吟唱
	SKS_BEFORE,				// 准备攻击前事件
	SKS_ATTACK,				// 攻击
	SKS_AFTER,				// 攻击完毕后事件
	SKS_OVER,				// 攻击完成
};

// 心法增加的附加属性
enum TelergyType
{
	TET_ATTACK,			// 增加攻击力
	TET_DEFENCE,		// 增加防御
	TET_HP,				// 增加生命
	TET_MP,				// 增加内力
	TET_TP,				// 增加体力
	TET_SHANBI,			// 增加闪避
	TET_BAOJI,			// 增加暴击
	// 特殊BUFFER
	TET_SUCKHP_PER,		// 增加吸血百分比
	TET_SUCKMP_PER,		// 增加吸内力百分比
	TET_SUCKTP_PER,		// 增加扣体力百分比
	TET_SUCKATTK,		// 增加扣攻击力
	TET_SUCKDEF,		// 增加扣防御力

	TET_MAX,
};

const WORD MaxSkillLevel = 100;		// 最高技能等级
const WORD MaxXinFaLevel = 100;		// 最高心法等级

// 技能打击点配置
class SSkillHitSpot
{
public:
	SSkillHitSpot() { memset(this, 0, sizeof(SSkillHitSpot)); }
	
public:
	WORD	m_ID;			// 技能ID
	WORD	m_HitCount;		// 打击点个数
	WORD	m_HitPots[12];	// 打击点时刻表
	WORD	m_CalcDmg[12];	// 打击点是否产生伤害
};

// 心法的基本配置
class SXinFaData
{
public:
	SXinFaData() { memset(this, 0, sizeof(SXinFaData)); }

public:
	char	m_Name[31];				// 名称
	WORD	m_ID;					// 心法ID
	WORD	m_Level;				// 心法等级
	DWORD	m_NeedSp;				// 所需真气
	DWORD	m_NeedMoney;			// 所需金钱
	WORD    m_AddAttack;            // 增加攻击力
	WORD    m_AddDefence;           // 增加防御力
	WORD    m_AddLife;              // 增加生命值
	WORD    m_AddInterforce;        // 增加内力值
	WORD    m_AddPhysiforce;        // 增加体力值
	WORD    m_AddDodgeforce;        // 增加躲闪值
	WORD    m_AddCriceforce;        // 增加暴击值
	WORD    m_AddPctBloodSucker;    // 增加吸血百分比
	WORD    m_AddPctCutInterforce;  // 增加扣内力百分比
	WORD    m_AddPctCutPhysiforce;  // 增加扣体力百分比
	WORD    m_AddPctCutAttakNum;	// 增加扣功力数量
	WORD    m_AddPctCutDefecNum;	// 增加扣防御力数量
};

// 技能的基本属性
class SSkillBaseData
{
public:
	SSkillBaseData() { memset(this, 0, sizeof(SSkillBaseData)); }

public:
	char	m_Name[31];				// 名称
	WORD	m_ID;					// 编号
	WORD	m_Level;				// 技能等级
	DWORD	m_UpNeedSp;				// 升级所需真气
	DWORD	m_UpNeedMoney;			// 升级所需金钱
	BYTE	m_School;				// 门派
	BYTE	m_SkillType;			// 技能类型
	BYTE	m_SkillPropertyType;	// 技能属性类型	
	BYTE	m_SkillSubProperty;		// 技能子属性	
	WORD	m_SkillAttack;			// 技能攻击力
	WORD	m_AtkType;				// 攻击方式
	WORD	m_CDGroup;				// 冷却组ID
	DWORD	m_CDTime;				// 自身冷却时间
	WORD	m_SkillMaxDistance;		// 技能最大距离
	WORD	m_SkillMinDistance;		// 技能最小距离
	WORD	m_SkillAtkRadius;		// 技能攻击半径
	WORD	m_SkillAtkWidth;		// 技能攻击宽度（扇形直线有用）
	WORD	m_AtkAmount;			// 攻击人数
	WORD	m_ConsumeMP;			// MP消耗
	WORD	m_ConsumeHP;			// HP消耗
	WORD	m_HitBackDis;			// 击退距离
	WORD	m_HateValue;			// 附加仇恨值
	bool	m_IsNormalSkill;		// 是否为普通攻击
	WORD	m_DamageRate;			// 伤害比率
	bool	m_CanUseInDizzy;		// 眩晕时可否使用
	WORD	m_BulletID;				// 子弹ID
	WORD	m_SingTime;				// 吟唱时间

	//add 2014.3.4
	DWORD   m_IconID;				//技能图标
	char    m_Description[256];		//技能描述
	char    m_Detail[256];			//技能效果
	WORD    m_SkillTypePos;			//技能存放位置
	char    m_FindSkillDes[256];			//技能出处
	DWORD   m_SkillBookID;			//技能书ID
	DWORD   m_ConsumeItemID;		//消耗道具的ID
	BYTE	m_OnceNeedItemCount;	//一次需要道具的数量
	BYTE	m_NeedPlayerLevel;	//激活技能需要玩家的等级
};
