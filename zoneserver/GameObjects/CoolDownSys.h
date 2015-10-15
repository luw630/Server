#pragma once

#include "EventBase.h"

// 冷却系统（道具，技能）
//enum CoolDownType
//{
//	CDT_ITEM,
//	CDT_SKILL,
//};

enum CDType
{
	CD_NONE = -1,
	CD_MoneyBless = 5000,
	CD_DiaomdBless,
	CD_MAX ,
};

class CoolDownBase : public EventBase
{
public:
	CoolDownBase(DWORD type, DWORD margin) : m_Type(type), m_Margin(margin)
	{
		m_IsOver = true;
	}

private:
	virtual void OnCancel();		// 中断CD
	virtual void OnActive();		// CD结束

public:
	DWORD	m_Type;					// CD编号
	DWORD	m_Margin;				// CD的持续时间
	bool	m_IsOver;				// 本次CD是否结束
};