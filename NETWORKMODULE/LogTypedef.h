#pragma once
#include "../pub/ConstValue.h"

///////////////////////////////////////////////////////////////////////////////////////////////
// ���ݶ���ԭ��
// ��������Ķ��ǻ�ȫ���õ������ݽṹ
///////////////////////////////////////////////////////////////////////////////////////////////

// ��ҽ�ɫ�б���Ϣ
struct SCharListData
{
    BYTE    m_bySex:1;					// ��ҽ�ɫ�Ա�
    BYTE    m_byBRON:3;					// �������ʾͼƬ����1��ʾ��ʦ
    BYTE    m_bySchool:4;				// �����������
	BYTE	m_FaceID;					// ��ģ��
	BYTE	m_HairID;					// ͷ��ģ��
    char	m_szName[CONST_USERNAME];	// �������
    DWORD   m_dwStaticID;				// ���ݿ�ID
    BYTE    m_byIndex;					// ��ɫ����
    WORD    m_wLevel;					// ��ҵȼ�
	DWORD	m_w3DEquipID[13];			// 3Dװ���Ҽ���Ʒ��,�������е�2��ʱװ
	DWORD	m_wWeaponID;				// ������
	BYTE			bFashionMode;		//ʱװģʽ������ͨģʽ
	DWORD   dShowState;//��ҵ�ǰ����״̬

};

// �ͻ������¼������֮��Ĵ�����ɫ����Ϣ
// Ϊ�˽�ʡ����������ȥ���˶�������ݡ�
struct SC2LCreateFixProperty
{
	char    m_szName[CONST_USERNAME];	// ����
	BYTE    m_bySex:1;					// �Ա�
	BYTE    m_byBRON:3;					// �������ʾͼƬ����1��ʾ��ʦ
	BYTE    m_bySchool:4;				// ����
	BYTE	m_FaceID;					// ��ģ��
	BYTE	m_HairID;					// ͷ��ģ��
	DWORD	m_w3DEquipID[13];			// 3Dװ���Ҽ���Ʒ��
	DWORD   m_dwStaticID;				// �ͻ��˵���½������������
};

// ������ɫ��Ϣ
struct SCreateFixProperty
{
    char    m_szName[CONST_USERNAME];	// ����
    BYTE    m_byBRON:3;					// �������ʾͼƬ����1��ʾ��ʦ
    BYTE    m_bySex:1;					// �Ա�
    BYTE    m_bySchool:4;				// ����
	BYTE	m_FaceID;					// ��ģ��
	BYTE	m_HairID;					// ͷ��ģ��
	DWORD   m_dwStaticID;
	WORD    m_wCurRegionID;				// ��ǰ���ڵ�ͼ��ID
};

// GMLog��Ϣ
struct SGMLog
{
	char szAccount[MAX_ACCOUNT];
	WORD wOperType; // �������ͣ��������1 ��������ʱ��2...
	char szOperObject[MAX_ACCOUNT]; //�����Ķ���һ��Ϊ��ҵ����֣���ĳЩ�����������ʺţ�
	char szOperPara1[33];  //�����Ĳ���1������Ϊ���ֻ��߷���
	char szOperPara2[33];  //�����Ĳ���2������Ϊ���ֻ��߷���
	int  OperTime;         //������ʱ�䣬 TIME Ϊ�����ṹ
};

