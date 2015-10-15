#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////
// 数据定义原则
// 放在这里的都是会全局用到的数据结构
///////////////////////////////////////////////////////////////////////////////////////////////

enum    E_EXTRA_STATE       // 战斗附加的特殊状态
{
    EES_DOUBLEBLOOD=1,        // 双倍伤血 1 2 4 8...
};

enum E_WOUNDTYPEEX      // 受伤类型的扩展标记
{
    EWT_MISS = 10,      // 躲闪
    EWT_GUARD,          // 防御
    EWT_NOTATACK,       // 辅助攻击
};

enum E_EXTRASTATUS   // 受伤附加效果 （按位取）
{
    EES_NONE =0,    // 没有附加效果
//    EES_DIZZY=1,    // 眩晕
    EES_XXX  =2,
};

enum    E_ATTACK_CTRL   // 玩家攻击控制字
{
    EAC_NONE,
    EAC_NOPLAYER,       // 按着Ctrl键，避开范围内的玩家
    //...
};

enum    E_VENATION_LIST // 经脉种类
{
    VENALIST_DAM,       // 手太阳
    VENALIST_POW,       // 手少阳
    VENALIST_DEF,       // 足太阴
    VENALIST_AGI,       // 足少阴
    VENALIST_NONE,      // 无经脉
};


// 发起攻击的数据
struct SAttack
{
	DWORD	dwGlobalID;			// 攻击者的ID
	WORD	wPosX;  			// 当前战斗结束后的位置(S->C)或者攻击的目标点(C->S)
	WORD	wPosY;  			// ...
    BYTE    byDirection;        // (S->C)攻击方向 ($1有必要分开两方的数据)

	BYTE    byWaitFrames;		// 等待帧数，以服务器的当前帧数为准，还需要多少帧才走到[wPosX/wPosY]

	//BYTE	byAttackType;		// 攻击的方式
    BYTE	byFightID;	        // 攻击招式编号 0=普通攻击

    BYTE    byLevel;            // 当前所用招式等级
    DWORD   dwProficiency;      // 技能的熟练度
    DWORD   dwMaxProficiency;   // 技能的熟练度最大值
    BYTE    byCtrlByte;         // 控制字
};

// 被攻击的数据
struct SWound
{
	DWORD	dwFightGlobalID;	// 攻击者的ID
	WORD	wPosX;  			// 当前战斗结束后的位置
	WORD	wPosY;  			// ...

    BYTE    byAttackerSex;      // 攻击者性别
    WORD    wAttackWeapon;      // 攻击者的武器（唐门）或特殊怪物的武器，受伤效果较为特殊
    BYTE	byFightID;          // 被攻击招式ID

	BYTE	byWoundType;		// 受伤的类型
    BYTE    byDizzyTime;        // 眩晕时间
	BYTE	byExtraStatus;		// 附加的特殊状态（包括所有的良性恶性数据，一次只会有一个状态被附加）
//	BYTE	byIsCritical;		// 是否为会心一击（忽略防御）
	BYTE	byIsDead;			// 目标是否死亡
	DWORD	dwDamage;			// 造成的伤害
    DWORD	dwLastHP;			// 还剩下的HP
    DWORD	dwMaxHP;			// HP最大值
    WORD	wLastMP;			// 还剩下的HP
    WORD	wMaxMP;			    // HP最大值
    WORD	wLastSP;			// 还剩下的HP
    WORD	wMaxSP;			    // HP最大值
    BYTE    byWhichVena;        // 受伤经脉
    BYTE    byVenaState;        // 经脉受伤状态
};

// 瘦身版 SWound，36 -> 16 字节
struct SWoundTiny
{
	enum
	{
		Hit = 0,				// 命中
		Not_Hit,				// 未命中
		BAOJI_HIT,			//暴击
		UNCRIT_HIT,			//抗暴
		WRECK_HIT,			//破击
		UNWRECK_HIT,		//抗破
		PUNCTURE_HIT,	//穿刺
		UNPUNCTURE_HIT, //抗穿
		NONE,					// 边界标记，什么也不做
	};

	BYTE	mHitState;			// 攻击类型(命中，未命中，麒麟臂伤害，额外伤害)
	WORD	mDamage;			// 造成的普通伤害
    DWORD	mDefenserGID;		// 被攻击者ID
    float	mDefenserX;			// 被攻击者战斗结束后的位置
	float	mDefenserY;
// 	WORD	mDamageExtra;		// 造成的额外伤害
// 	WORD	mIgnoreDefDamage;	// 造成的破防伤害
// 	WORD	mRebound;			// 反弹伤害
// 	BYTE	isCritical;			// 是否重击
	DWORD	mMaxHp;				// 最大血
	DWORD	mCurHp;				// 当前血
};
