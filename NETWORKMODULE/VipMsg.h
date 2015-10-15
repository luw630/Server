#pragma once
#include "NetModule.h"

enum VipFactorType //Vip������������
{
	VFT_HP,
	VFT_GONGJI,
	VFT_FANGYU,
	VFT_MAX,
};

//=============================================================================================
DECLARE_MSG_MAP(SVIPMsg, SMessage, SMessage::EPRO_VIP_MESSAGE)
//{{AFX
//2014/5/17 add by ly
EPRO_VIP_INITINFO,		//��ʼ��VIP��Ϣ
EPRO_VIP_GETGIFTINFO,		//��ȡ�����Ϣ
EPRO_VIP_GETGIFT,	//��ȡVIP���
//}}AFX
END_MSG_MAP()

//��ʼ��VIP��Ϣ
DECLARE_MSG(SInitVipInfMsg, SVIPMsg, SVIPMsg::EPRO_VIP_INITINFO)
struct SQInitVipInfMsg : public SInitVipInfMsg
{
};

struct SAInitVipInfMsg : public SInitVipInfMsg
{
	BYTE m_VipGetGiftState[10];	//�Ƿ������ȡ���״̬��Ϊ1ʱ��ʾֻ��������������ȡ��Ϊ2�Ǳ�ʾֻ�и���������Թ���Ϊ3ʱ��ʾ���������������ȡ����
};

//��ȡ�����Ϣ
DECLARE_MSG(SGetVipGiftInfMsg, SVIPMsg, SVIPMsg::EPRO_VIP_GETGIFTINFO)
struct SQGetVipGiftInfMsg : public SGetVipGiftInfMsg
{
	BYTE m_VipLevel;	//VIP�ȼ�
	BYTE m_GetType;	//��ȡ�����������Ϣ 0Ϊ�����ȡ��� 1Ϊ���ѹ������
};

struct SAGetVipGiftInfMsg : public SGetVipGiftInfMsg
{
	WORD m_ItemSize;	//����е��ߵ�����
	DWORD *m_pItemID;	//����е���
	BYTE *m_pItemIDNum;	//�����ÿ�����ߵ�����
};

//��ȡVIP���
DECLARE_MSG(SGetVipGiftMsg, SVIPMsg, SVIPMsg::EPRO_VIP_GETGIFT)
struct SQGetVipGiftMsg : public SGetVipGiftMsg
{
	BYTE m_VipLevel;	//VIP�ȼ�
	BYTE m_GetType;	//��ȡ��ʽ 0Ϊ�����ȡ��� 1Ϊ���ѹ������
};

struct SAGetVipGiftMsg : public SGetVipGiftMsg
{
	BYTE m_Result;	//��� 0Ϊʧ��  1Ϊ�ɹ�
};
