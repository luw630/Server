#pragma once

// ���Ʒ��
enum EMountQuality
{
	EMQ_YIBAN = 1,		// һ��
	EMQ_YOULIANG,		// ����
	EMQ_SHANGCHENG,		// �ϳ�
	EMQ_XIYOU,			// ϡ��
	EMQ_SHISHI,			// ʷʫ
	EMQ_JUESHI,			// ����
};

// ���Ӱ������
enum EMountAttribute
{
	EMA_MHP,			// ��������
	EMA_MMP,			// ��������
	EMA_MTP,			// ��������
	EMA_GONGJI,			// ����
	EMA_FANGYU,			// ����
	EMA_BAOJI,			// ����
	EMA_SHANBI,			// ����

	EMA_MAX,			
};

const int MAX_MOUNT_LEVEL = 9;		// ������ȼ�

// ���
class SMountBaseData
{
public:
	SMountBaseData()
	{
		memset(this, 0, sizeof(SMountBaseData));
	}

public:
	char	m_Name[13];				// ����
	WORD	m_ID;					// ID
	BYTE	m_Level;				// �ȼ�
	BYTE	m_Quality;				// Ʒ��
	BYTE	m_RideNum;				// ���������
	BYTE	m_CanUpdate;			// �Ƿ������
	DWORD	m_NeedExp;				// �������辭��
	WORD	m_SpeedUp;				// ���ٶ�ϵ��
	WORD	m_Attri[EMA_MAX][2];	// ���Լӳ�
	WORD	m_ModelID;				// ģ��ID
};