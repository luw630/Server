#pragma once

// 侠义道3属性同步机制，按需同步，节约网络流量
// 属性枚举
enum XYD3_ATTRI
{
	////////// 战斗属性 ////////////	
	XA_GONGJI,			// 攻击
	XA_FANGYU,			// 防御
	XA_BAOJI,			// 暴击
	XA_SHANBI,			// 闪避

	XA_MAX_HP,			// 最大生命
	XA_MAX_MP,			// 最大内力
	XA_MAX_TP,			// 最大体力

	XA_CUR_HP,			// 当前生命
	XA_CUR_MP,			// 当前内力
	XA_CUR_TP,			// 当前体力

	XA_LEVEL,			// 等级
	XA_SPEED,			// 移动速度
	XA_ATKSPEED,		// 攻击速度

	XA_HIT, //命中
	XA_UNCRIT, //抗暴
	XA_WRECK,//破击
	XA_UNWRECK,	//抗破
	XA_PUNCTURE,	//穿刺
	XA_UNPUNCTURE,	//抗穿

	XA_REDUCEDAMAGE,	// 伤害减免
	XA_ABSDAMAGE,		// 绝对伤害
	XA_NONFANGYU,		// 无视防御
	XA_MULTIBAOJI,		// 暴击倍数

// 	XA_ICE_DAMAGE,		// 冰伤害
// 	XA_ICE_DEFENCE,		// 冰抗性
// 	XA_FIRE_DAMAGE,		// 火伤害
// 	XA_FIRE_DEFENCE,	// 火抗性
// 	XA_XUAN_DAMAGE,		// 玄伤害
// 	XA_XUAN_DEFENCE,	// 玄抗性
// 	XA_POISON_DAMAGE,	// 毒伤害
// 	XA_POISON_DEFENCE,	// 毒抗性

	////////// 玩家专有 ////////////
	XA_MAX_EXP,			// 最大经验
	XA_CUR_EXP,			// 当前经验
	XA_MAX_JP,			// 最大精力
	XA_CUR_JP,			// 当前精力
	XA_MAX_SP,			// 最大真气
	XA_CUR_SP,			// 当前真气

// 	XA_BIND_MONEY,		// 绑定货币
// 	XA_UNBIND_MONEY,	// 非绑定货币
	XA_UNBIND_MONEY,
	XA_BIND_MONEY,

	XA_ZENGBAO,			// 赠宝
	XA_YUANBAO,			// 元宝

	XA_TF_JINGGONG,		// 进攻
	XA_TF_FANGYU,		// 防御
	XA_TF_QINGSHEN,		// 轻身
	XA_TF_JIANSHEN,		// 健身
	XA_REMAINPOINT,		// 剩余点数

	XA_FIGHTPOWER,	//玩家战斗力同步

	XA_MAX,
};

// 属性数据类型，解析用
enum XYD3_ATTRI_TYPE
{
	XAT_WORD,		// 16无符号
	XAT_DWORD,		// 32无符号	
	XAT_QWORD,		// 64无符号

	XAT_MAX,		
};

inline WORD GetAttriType(XYD3_ATTRI attri, WORD &size)
{
	WORD type = XAT_MAX;
	size = 0;

	switch (attri)
	{
	case XA_GONGJI:
	case XA_FANGYU:
	case XA_BAOJI:
	case XA_SHANBI:
	case XA_LEVEL:
	case XA_ATKSPEED:
	case XA_TF_JINGGONG:
	case XA_TF_FANGYU:
	case XA_TF_QINGSHEN:
	case XA_TF_JIANSHEN:
	case XA_REMAINPOINT:

	case XA_REDUCEDAMAGE:
	case XA_ABSDAMAGE:
	case XA_NONFANGYU:
	case XA_MULTIBAOJI:
	case XA_UNCRIT:
	case XA_WRECK:
	case XA_UNWRECK:
	case XA_PUNCTURE:
	case XA_UNPUNCTURE:
	case XA_HIT:
		type = XAT_WORD;
		size = sizeof(WORD);
		break;

	case XA_MAX_HP:
	case XA_MAX_MP:
	case XA_MAX_TP:
	case XA_MAX_JP:
	case XA_MAX_SP:
	case XA_CUR_HP:
	case XA_CUR_MP:
	case XA_CUR_TP:
	case XA_CUR_JP:
	case XA_CUR_SP:
	case XA_BIND_MONEY:
	case XA_UNBIND_MONEY:
	case XA_ZENGBAO:
	case XA_YUANBAO:
	case XA_FIGHTPOWER:
		type = XAT_DWORD;
		size = sizeof(DWORD);
		break;

	case XA_MAX_EXP:
	case XA_CUR_EXP:
		type = XAT_QWORD;
		size = sizeof(QWORD);
		break;

	default:
		break;
	}

	return type;
}