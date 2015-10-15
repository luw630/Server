#pragma once

#include "BuffSys.h"

enum BUFF_DELETE_TYPE
{
	BDT_NULL		= 0x00000000,		// 普通的到期删除/被顶替/下线删除
	BDT_ATTACKED	= 0x00000001,		// 被攻击消失
	BDT_DAMAGED		= 0x00000002,		// 受到伤害消失
	BDT_DEAD		= 0x00000004,		// 死亡消失
};

// 下线是否保存
enum BUFF_SAVE_TYPE
{
	BST_NO_SAVE		= 1,				// 不保存
	BST_SAVE_NO_TIME,					// 保存，不记录离线时间
	BST_SAVE_WITH_TIME,					// 保存，但记录离线时间	
};

// Buff类型
enum BUFF_EFFECT_TYPE
{
	BET_BUFF,							// 增益
	BET_DEBUFF,							// 损益
};

// Buff的基本属性
class SBuffBaseData
{
public:
	SBuffBaseData() 
	{ 
		memset(this, 0, sizeof(*this)); 
	}

public:
	DWORD			m_ID;				// 编号
	DWORD			m_DeleteType;		// 删除类型
	DWORD			m_SaveType;			// 保存类型
	DWORD			m_ScriptID;			// 脚本ID
	DWORD			m_Margin;			// 激活时间间隔
	WORD			m_WillActionTimes;	// 要激活的次数
	WORD			m_GroupID;			// Buff分组
	WORD			m_Weight;			// Buff权重
	WORD			m_InturptRate;		// 打断几率
	WORD			m_PropertyID;		// 属性ID
	WORD			m_ActionType;		// 修改模式
	BYTE			m_Type;				// 增益/损益/区域Buff
	bool			m_CanBeReplaced;	// 能否被替换
	BuffActionEx	m_Action[BATS_MAX];	// Buff动作
	WORD		m_ReginType;//区域Buff类型0 自身为原点 1 目标为原点
	WORD		m_ReginShape;//区域形状 0 圆形 1矩形
	WORD		m_ReginHeight;//区域长度（如果为圆形就配置为区域半径）
	WORD		m_ReginWidth;//区域宽度（如果为圆形将不使用这个参数）
};