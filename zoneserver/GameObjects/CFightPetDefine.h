#pragma once

#include "Networkmodule\ItemTypedef.h"

enum FightPetAttri
{
	FPA_MAXHP,			// ��������
	FPA_MAXMP,			// ��������
	FPA_GONGJI,			// ����
	FPA_FANGYU,			// ����
	FPA_BAOJI,			// ����
	FPA_SHANBI,			// ����

	FPA_MAX,
};

// ��������
enum FightPetType
{
	FPT_WAIGONG,		// �⹦��
	FPT_NEIGONG,		// �ڹ���
	FPT_WAIGONGFANG,	// �⹦������
	FPT_NEIGONGFANG,	// �ڹ�������
	FPT_ALL,			// �ۺ���
};

// ���͵Ļ�������
class SFightPetBaseData
{
public:
	SFightPetBaseData() { memset(this, 0, sizeof(SFightPetBaseData)); };

public:
	char	m_Name[SFightPetBase::FP_NAME];
	WORD	m_ID;					// ID
	bool	m_CanChangeName;		// �ܷ����
	BYTE	m_Type;					// ��������
	WORD	m_Attri[FPA_MAX][3];	// ����ֵ
	BYTE		m_RareMin;					// ϡ�ж���С
	BYTE		m_RareMax;					// ϡ�ж����
	DWORD	m_nexp;			// ��������
	WORD	m_nmodelID;			// ģ��ID
	DWORD	m_niconID;			// ����ICON
	WORD		m_nLimitLevel;		//���Я���ȼ�
	BYTE			m_bSex;				//�Ա�
};


// ���͵ĳɳ�����ȡֵ
class SFightPetRise
{
public:
	SFightPetRise() { memset(this, 0, sizeof(SFightPetRise)); };

public:
	BYTE		m_nRare;// ϡ�ж�
	WORD  m_nHpRiseMin;// �����ɳ���Сֵ
	WORD  m_nHpRiseMax;// �����ɳ����ֵ
	WORD  m_nMpRiseMin;// �����ɳ���Сֵ
	WORD  m_nMpRiseMax;// �����ɳ����ֵ
	WORD  m_nGongJiRiseMin;// �����ɳ���Сֵ
	WORD  m_nGongJiRiseMax;// �����ɳ����ֵ
	WORD  m_nFangYuRiseMin;// �����ɳ���Сֵ
	WORD  m_nFangYuRiseMax;// �����ɳ����ֵ

	WORD  m_nShanBiRiseMin;// ���ܳɳ���Сֵ
	WORD  m_nShanBiRiseMax;// ���ܳɳ����ֵ
	WORD  m_nBaojiRiseMin;// �����ɳ���Сֵ
	WORD  m_nBaojiRiseMax;// �����ɳ����ֵ
	WORD  m_nWuXinRiseMin;// ���Գɳ���Сֵ
	WORD  m_nWuXinRiseMax;// ���Գɳ����ֵ

};

