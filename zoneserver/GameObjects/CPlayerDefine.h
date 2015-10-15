#ifndef CPLAYERDEFINE_H
#define CPLAYERDEFINE_H

class SBaseAttribute
{
public:
// 	WORD		m_FreePoint;			// 自由加点
// 	DWORD		m_BaseMp;				// 内力
// 	DWORD		m_BaseSp;				// 真气
// 	WORD		m_HpRecoverSpeed;		// 生命恢复速度
// 	WORD		m_MpRecoverSpeed;		// 内力恢复速度
// 	WORD		m_TpRecoverSpeed;		// 体力恢复速度
// 	WORD		m_HpRecoverInterval;	// 生命恢复间隔
// 	WORD		m_MpRecoverInterval;	// 内力恢复间隔

	DWORD		m_BaseHp;				// 生命
	DWORD		m_BaseJp;				// 精力
	WORD		m_GongJi;				// 攻击
	WORD		m_FangYu;				// 防御
	WORD  m_Hit;			//命中
	WORD		m_ShanBi;				// 闪避
	WORD		m_BaoJi;				// 暴击
	WORD m_uncrit; //抗暴
    WORD m_wreck;//破击
	WORD m_unwreck;	//抗破
	WORD m_puncture;	//穿刺
	WORD m_unpuncture;	//抗穿
	DWORD		m_BaseTp;				// 体力
	WORD  m_TpRecover; //体力恢复
	WORD		m_TpRecoverInterval;	// 体力恢复间隔
	QWORD		m_Exp;					// 所需经验

};

struct XWZ_Attribute			// 修为值计算
{
	WORD		m_Atk;			// 攻击
	WORD		m_Defence;		// 防御
	WORD		m_CtrAtk;		// 暴击
	WORD		m_Escape;		// 躲避
	WORD		m_Hp;			// 生命
	WORD		m_Mp;			// 内力
	WORD		m_Tp;			// 体力
	WORD		m_SPEED;		// 移动速度
	WORD		m_ATKSPEED;		// 攻击速度
	WORD		m_ABSDAMAGE;	// 绝对伤害
	WORD		m_REDUCEDAMAGE;	// 伤害减免
	WORD		m_NONFANGYU;	// 无视防御
	WORD		KylinArmLevel;	// 麒麟臂等级
};

#endif // CPLAYERDEFINE_H