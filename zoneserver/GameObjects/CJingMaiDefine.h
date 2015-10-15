#pragma once
#include "pub\ConstValue.h"

// ����Ӱ�������
enum JingMaiProperty
{
	JMP_MHP,		// ��������
	JMP_MMP,		// ��������
	JMP_MTP,		// ��������
	JMP_GONGJI,		// ����
	JMP_FANGYU,		// ����
	JMP_BAOJI,		// ����
	JMP_SHANBI,		// ����

	JMP_MAX,
};

class SJingMaiBaseData
{
public:
	SJingMaiBaseData() { memset(this, 0, sizeof(SJingMaiBaseData)); }

public:
	char	m_Name[CONST_USERNAME];
	WORD	m_MaiID;				// ����ID
	WORD	m_XueID;				// ѨλID
	WORD	m_Factor[JMP_MAX];		// ��������
	DWORD	m_NeedItem;				// �������
	WORD	m_Rate;					// �ɹ���
	DWORD	m_NeedSp;				// ��������
	DWORD   m_NeedFrontXueID;        //����ǰ��Ѩλ
};

/////add by ly 2014/3/17	��������ϵͳ�;���ϵͳΪ���ƹ��ܣ�����Ϊ���վ���ϵͳ������ʵ�֣���ϵͳû�о���ϵͳ������ܣ�

enum XinYangProperty
{
	XYP_HP,		//����
	XYP_JP,		//����
	XYP_TP,		//����
	XYP_HIT,	//����
	XYP_GONGJI,	//����
	XYP_FANGYU,	//����
	XYP_BAOJI,	//����
	XYP_POJI,	//�ƻ�
	XYP_CHUANCI,	//����
	XYP_KANGBAO,	//����
	XYP_KANGPO,	//����
	XYP_KANGCHUAN,	//����
	XYP_SHANBI,	//����
	XYP_MAX,
};

class SXinYangBaseData
{
public:
	SXinYangBaseData() { memset(this, 0, sizeof(SXinYangBaseData)); }

public:
	char m_Name[128];
	BYTE m_ZhuShenID;		//����ID
	BYTE m_XingxiuID;		//����ID, ����
	WORD m_Factor[XYP_MAX];	//��������
	BYTE m_SuccessRate;		//���ǳɹ���
	WORD m_ConsumeYinLiang;	//��������
	WORD m_ConsumeJinBi;	//���Ľ��
	DWORD m_ConsumeItem;	//���ĵ���
	char m_XingXiuDescript[256];	//��������
	BYTE m_PlayerVipLevel;		//��ҵȼ�����
	WORD m_CanHaveSkillID;	//�����Ӧ�������ܵ�ID
	WORD m_ConsumeSp;	//��������
};