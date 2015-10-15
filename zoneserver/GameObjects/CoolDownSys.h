#pragma once

#include "EventBase.h"

// ��ȴϵͳ�����ߣ����ܣ�
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
	virtual void OnCancel();		// �ж�CD
	virtual void OnActive();		// CD����

public:
	DWORD	m_Type;					// CD���
	DWORD	m_Margin;				// CD�ĳ���ʱ��
	bool	m_IsOver;				// ����CD�Ƿ����
};