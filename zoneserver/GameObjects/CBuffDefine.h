#pragma once

#include "BuffSys.h"

enum BUFF_DELETE_TYPE
{
	BDT_NULL		= 0x00000000,		// ��ͨ�ĵ���ɾ��/������/����ɾ��
	BDT_ATTACKED	= 0x00000001,		// ��������ʧ
	BDT_DAMAGED		= 0x00000002,		// �ܵ��˺���ʧ
	BDT_DEAD		= 0x00000004,		// ������ʧ
};

// �����Ƿ񱣴�
enum BUFF_SAVE_TYPE
{
	BST_NO_SAVE		= 1,				// ������
	BST_SAVE_NO_TIME,					// ���棬����¼����ʱ��
	BST_SAVE_WITH_TIME,					// ���棬����¼����ʱ��	
};

// Buff����
enum BUFF_EFFECT_TYPE
{
	BET_BUFF,							// ����
	BET_DEBUFF,							// ����
};

// Buff�Ļ�������
class SBuffBaseData
{
public:
	SBuffBaseData() 
	{ 
		memset(this, 0, sizeof(*this)); 
	}

public:
	DWORD			m_ID;				// ���
	DWORD			m_DeleteType;		// ɾ������
	DWORD			m_SaveType;			// ��������
	DWORD			m_ScriptID;			// �ű�ID
	DWORD			m_Margin;			// ����ʱ����
	WORD			m_WillActionTimes;	// Ҫ����Ĵ���
	WORD			m_GroupID;			// Buff����
	WORD			m_Weight;			// BuffȨ��
	WORD			m_InturptRate;		// ��ϼ���
	WORD			m_PropertyID;		// ����ID
	WORD			m_ActionType;		// �޸�ģʽ
	BYTE			m_Type;				// ����/����/����Buff
	bool			m_CanBeReplaced;	// �ܷ��滻
	BuffActionEx	m_Action[BATS_MAX];	// Buff����
	WORD		m_ReginType;//����Buff����0 ����Ϊԭ�� 1 Ŀ��Ϊԭ��
	WORD		m_ReginShape;//������״ 0 Բ�� 1����
	WORD		m_ReginHeight;//���򳤶ȣ����ΪԲ�ξ�����Ϊ����뾶��
	WORD		m_ReginWidth;//�����ȣ����ΪԲ�ν���ʹ�����������
};