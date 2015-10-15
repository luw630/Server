#pragma once
#include "NetModule.h"

enum VipFactorType //Vip附加属性类型
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
EPRO_VIP_INITINFO,		//初始化VIP信息
EPRO_VIP_GETGIFTINFO,		//获取礼包信息
EPRO_VIP_GETGIFT,	//获取VIP礼包
//}}AFX
END_MSG_MAP()

//初始化VIP信息
DECLARE_MSG(SInitVipInfMsg, SVIPMsg, SVIPMsg::EPRO_VIP_INITINFO)
struct SQInitVipInfMsg : public SInitVipInfMsg
{
};

struct SAInitVipInfMsg : public SInitVipInfMsg
{
	BYTE m_VipGetGiftState[10];	//是否可以领取礼包状态；为1时表示只有免费礼包可以领取、为2是表示只有付费礼包可以购买、为3时表示两种礼包都可以领取或购买
};

//获取礼包信息
DECLARE_MSG(SGetVipGiftInfMsg, SVIPMsg, SVIPMsg::EPRO_VIP_GETGIFTINFO)
struct SQGetVipGiftInfMsg : public SGetVipGiftInfMsg
{
	BYTE m_VipLevel;	//VIP等级
	BYTE m_GetType;	//获取那类礼包的信息 0为免费领取礼包 1为付费购买礼包
};

struct SAGetVipGiftInfMsg : public SGetVipGiftInfMsg
{
	WORD m_ItemSize;	//礼包中道具的数量
	DWORD *m_pItemID;	//礼包中道具
	BYTE *m_pItemIDNum;	//礼包中每个道具的数量
};

//获取VIP礼包
DECLARE_MSG(SGetVipGiftMsg, SVIPMsg, SVIPMsg::EPRO_VIP_GETGIFT)
struct SQGetVipGiftMsg : public SGetVipGiftMsg
{
	BYTE m_VipLevel;	//VIP等级
	BYTE m_GetType;	//获取方式 0为免费领取礼包 1为付费购买礼包
};

struct SAGetVipGiftMsg : public SGetVipGiftMsg
{
	BYTE m_Result;	//结果 0为失败  1为成功
};
