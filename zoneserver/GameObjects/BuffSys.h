#pragma once

#include "EventBase.h"
#include "networkmodule/playertypedef.h"
#include "BaseObject.h"
class CFightObject;

// Buff可以修改的属性/标记列表
enum BuffActionProperty
{
	// 可逆属性
	BAP_MHP = 1,			// 生命上限
	BAP_MMP,				// 内力上限
	BAP_MTP,				// 体力上限
	BAP_GONGJI,				// 攻击
	BAP_FANGYU,				// 防御
	BAP_BAOJI,				// 暴击
	BAP_SHANBI,				// 闪避
	BAP_SPEED,				// 移动速度	
	BAP_HIT,				//命中
	BAP_ATKSPEED,			// 攻击速度
	BAP_SORBDAM,			// 伤害吸收值
	BAP_SORBDAM_PER,		// 伤害吸收百分比
	BAP_BACK_DAMAGE,		// 伤害反弹值
	BAP_BACK_DAMAGE_PER,	// 伤害反弹百分比
	BAP_EXTRA_EXP_PER,		// 额外经验百分比
	BAP_UNCRIT, //抗暴
	BAP_WRECK,//破击
	BAP_UNWRECK,	//抗破
	BAP_PUNCTURE,	//穿刺
	BAP_UNPUNCTURE,	//抗穿
	BAP_ATTRI_MAX,

	// 不可逆属性（因为不缓冲，所以不用分配号码，附属于可逆属性之后）
	BAP_CHP,				// 当前生命
	BAP_CMP,				// 当前内力
	BAP_CTP,				// 当前体力

	//********战斗状态*********/
	BAP_DIZZY = 1000,		// 昏迷(不会被打醒)
	BAP_LIMIT_SKILL,		// 封招
	BAP_HYPNOTISM,			// 催眠(目前同昏迷)
	BAP_DINGSHENG ,			// 定身
	BAP_WUDI,				// 无敌
	BAP_ADDBUFF,				// 无法附加任何BUFF
	BAP_JITUI,				// 被击退
	BAP_STATE_MAX,

	//********生物抗性*********/
//	BAP_RESIST_CHP = 2001,	// 抗流血
//	BAP_RESIST_SPEED,		// 抗降速
//	BAP_RESIST_DIZZY,		// 抗眩晕
//	BAP_RESIST_LIMIT_SKILL,	// 抗封招
//	BAP_RESIST_HYPNOTISM,	// 抗催眠
//	BAP_RESIST_POISION,		// 抗中毒
	BAP_RESIST_SUCKHP = 2001,	// 抗吸血ok
	BAP_RESIST_SUCKMP,			// 抗吸内OK
	BAP_RESIST_SUCKTP,			// 抗吸体力OK
	BAP_RESIST_DEDUCTHP,		// 抗减血OK
	BAP_RESIST_DEDECTMP,		// 抗减内OK
	BAP_RESIST_DEDUCTTP,		// 抗减体力OK
	BAP_RESIST_JITUI,			// 抗击退ok
	BAP_RESIST_DINGSHEN,		// 抗定身ok
	BAP_RESIST_DEDUCTSPEED,		// 抗减速OK
	BAP_RESIST_MAX,

	//********特殊Buff*********/
	BAP_SP_SUCKHP = 3001,	// 吸生命
	BAP_SP_SUCKMP,			// 吸内力
	BAP_SP_SUCKTP,			// 吸体力
	BAP_SP_DEDUCTHP,		// 扣生命OK
	BAP_SP_DEDUCTMP,		// 扣内力OK
	BAP_SP_DEDUCTTP,		// 扣体力OK

	BAP_SP_MAX,				

	BAP_NULL = 0xffff,		// 什么都不修改
};

// Buff事件的触发点
enum BuffActionTriggerSpot
{
	BATS_ADD = 0,		// 附加到玩家身上时触发
	BATS_REMOVE,		// 从玩家身上移除时触发
	BATS_CANCEL,		// 被打断的时候触发
	BATS_INTERVAL,		// 间隔触发

	BATS_MAX,			// Max
};

// Buff改变值方式
enum BuffActionMode
{
	BAM_POT = 0,		// 改变值
	BAM_PRECENT,		// 改变百分比
	BAM_MAX,			// Max
};

// Buff事件元
class BuffActionEx
{
public:
	WORD	m_TriggetSpot;	// 触发时机
	int		m_Value;		// 值
};


// Buff基类
class BuffModify : public EventBase
{
	friend class SBuffBaseData;
	friend class BuffManager;

	typedef std::vector<BuffActionEx*> BuffActionVector;

public:
	BuffModify(CFightObject *owner, CFightObject *pusher) : m_ID(0), m_GroupID(0), m_Weight(0), m_InturptRate(0), m_Margin(0), m_WillActTimes(0), 
		m_ActedTimes(0), m_DeleteType(0), m_SaveType(0), m_curStep(0), m_CanDropBuff(false), m_Type(0), m_CanBeReplaced(false), m_ScriptID(0),
		m_PropertyID(0), m_ActionType(BAM_MAX)
	{
		m_owner		= owner;
		m_pusher	= pusher;
	}

	//获取和设置buff事件的数据，主要用于下线保存上线恢复
	void GetSaveBuffData(TSaveBuffData *pdat_t) const;
	void SetSaveBuffData(const TSaveBuffData *pdat_t);
private:
	virtual void OnCancel();				// Buff中断
	virtual void OnActive();				// Buff激活
	virtual void SelfDestory();				// 自我销毁

	void SendNotify(bool init);				// 发送通知
	void OnTimer(int step);					// 定时触发
	void OnTimerActived(int step);      //buff定时激活
	bool InitBuffData(const SBuffBaseData *pData);
	bool ModifyProperty(WORD ProID, WORD Mode, int val, int *ret, bool isEnd);
	bool CreateReginBuff();

	void CheckAddList(check_list<LPIObject> *objectList, std::list<CFightObject *> *destList);
	bool IsCanDamageByBuff(CFightObject* desObject);//Buff条件
public:
	CFightObject	*m_owner;				// Buff拥有者
	CFightObject	*m_pusher;				// Buff释放者

	DWORD			m_ID;					// 编号
	DWORD			m_DeleteType;			// 删除类型
	DWORD			m_SaveType;				// 保存类型
	DWORD			m_ScriptID;				// 脚本ID
	DWORD			m_Margin;				// 激活时间间隔
	int				m_curStep;				// 当前的阶段
	WORD			m_WillActTimes;			// 要激活的次数（0表示永久，1表示瞬发，2表示持续，2以上表示间隔）
	WORD			m_ActedTimes;			// Buff已经激活的次数
	WORD			m_GroupID;				// Buff分组
	WORD			m_Weight;				// Buff权重
	WORD			m_InturptRate;			// 打断几率
	BYTE			m_Type;					// 增益/损益
	bool			m_CanBeReplaced;		// 能否被替换
	bool			m_CanDropBuff;			// 可否删除

	WORD			m_PropertyID;			// 属性ID
	WORD			m_ActionType;			// 修改模式
	BuffActionEx	m_BuffAction[BATS_MAX];	// Buff动作
	WORD		m_ReginBuffType;//区域Buff类型0 自身为原点 1 目标为原点
	WORD		m_ReginBuffShape;//区域形状 0 圆形 1矩形
	WORD		m_ReginBuffHeight;//区域长度（如果为圆形就配置为区域半径）
	WORD		m_ReginBuffWidth;//区域宽度（如果为圆形将不使用这个参数）
	float			 m_fCenterPosX;
	float			m_fCenterPosY;
	
};