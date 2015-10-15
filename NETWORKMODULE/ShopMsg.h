#pragma once
#include "NetModule.h"
//=============================================================================================
DECLARE_MSG_MAP(SShopMsg, SMessage, SMessage::EPRO_SHOP_MESSAGE)
//{{AFX
//2014/5/7 add by ly
EPRO_REQUEST_SHOP,	//��������̳��б���Ϣ
EPRO_QUEST_SPECIFYSHOP,	//��������ض��̳�����Ʒ��Ϣ
EPRO_BUY_GOODS,		//�����������Ʒ
EPRO_NOTITY_SPECIALGOODSUPDATE,	//֪ͨ���ĳ���̳��е���Ʒ�Ѿ�ˢ��
EPRO_SHOP_COUNTDOWN,	//�����̳ǵ���ʱʱ��
EPRO_SHOP_FINDGOODSBYITERMID,	//�����ȡ��Ʒ��Ϣ����ͨ�����ߵ�ID

//add by ly 2014/6/25
EPRO_PLAYER_PAY,	//��ҳ�ֵ
//}}AFX
END_MSG_MAP()


//2014/5/7 add by ly
//��Ʒ���ݽṹ
typedef struct goodsdata
{
	DWORD m_GoodsIndex;	//��Ʒ����(ͨ���̳����͡���Ʒ������1������Ʒ������2�������)
	BYTE m_ShopType;	//�̳�����
	BYTE m_GoodsType;	//��Ʒ����
	DWORD m_GoodsID;	//��ƷID
	WORD m_OriginalJinBiCost;	//���ԭ��
	WORD m_CurrentJinBiCost;	//����ּ�
	WORD m_OriginalYinBiCost;	//����ԭ��
	WORD m_CurrentYinBiCost;	//�����ּ�
	WORD m_Count;	//ÿ��ˢ��������
}GoodsData, *LPGoodsData;

//�̳�����
typedef struct shopdata
{
	BYTE m_ShopType;	//�̳�����
	BYTE m_RefreshFlag;	//�̳�ˢ�±�־��0Ϊ����ˢ���̳ǣ�1Ϊ����ˢ���̳ǣ�
	BYTE m_RefreshType;	//�̳�ˢ�����ͣ�Ϊ0ʱ�������ǣ�Ϊ1ʱ����ʾÿ������ʱ�䣻Ϊ2ʱ����ʾ�������ʱ��, �ű���������¼���ʱҲ�����ã�
	BYTE m_UpdateFlag;	//�̳Ǹ��±�ʶ��Ĭ��Ϊ1����Ʒ�Ѿ����£�
	DWORD m_RefreshAllTime;	//�̳�ˢ��������ʱ�䣨��m_RefreshFlag = 1ʱ��ʹ�ã�

}ShopData, *LPShopData;

//��������̳��б���Ϣ
DECLARE_MSG(SShopListMsg, SShopMsg, SShopMsg::EPRO_REQUEST_SHOP)
struct SQShopListMsg : public SShopListMsg
{
};

struct SAShopListMsg : public SShopListMsg
{
	WORD m_ShopNum;	//�̳�����
	LPShopData m_lpShopData;	//�̳���Ϣ
};

//��������ض��̳�����Ʒ���б���Ϣ
DECLARE_MSG(SShopGoodsListMsg, SShopMsg, SShopMsg::EPRO_QUEST_SPECIFYSHOP)
struct SQShopGoodsListMsg : public SShopGoodsListMsg
{
	BYTE m_ShopType;	//�̳�����
};

struct SAShopGoodsListMsg : public SShopGoodsListMsg
{
	WORD m_GoodsNum;	//��Ʒ����
	LPGoodsData m_lpGoodsData;	//��Ʒ��Ϣ
};

//�����������Ʒ
DECLARE_MSG(SBuyGoodsMsg, SShopMsg, SShopMsg::EPRO_BUY_GOODS)
struct SQBuyGoodsMsg : public SBuyGoodsMsg
{
	BYTE m_BuyType;		//����ʽ 1Ϊ��ҹ���2Ϊ���ҹ���
	DWORD m_GoodsIndex;	//��Ʒ����(ͨ���̳����͡���Ʒ������1������Ʒ������2�������)
	WORD m_BuyNum;	//��������
};

struct SABuyGoodsMsg : public SBuyGoodsMsg
{
	BYTE m_BuyResult;	//��������0��ʾʧ�ܣ�1��ʾ�ɹ�
};

//֪ͨ����ؼ���Ʒ�Ѿ�ˢ��
DECLARE_MSG(SNotifySpecialGoodsUpdateMsg, SShopMsg, SShopMsg::EPRO_NOTITY_SPECIALGOODSUPDATE)
struct SANotifySpecialGoodsUpdateMsg : public SNotifySpecialGoodsUpdateMsg
{
	BYTE m_ShopType;	//�̳�����
	BYTE m_UpdateFlag;	//���±�ʶ��Ϊ1��ʾ���̳��е���Ʒ�Ѿ�����
	DWORD m_UpdateRefreshAllTime;	//�̳�ˢ�µ�������ʱ�����ֵ��Ϊ0ʱ�������̳�ˢ�µ�������ʱ�䣩
};

//��������̳�ˢ�µ���ʱ
DECLARE_MSG(SGetShopCountDownMsg, SShopMsg, SShopMsg::EPRO_SHOP_COUNTDOWN)
struct SQGetShopCountDownMsg : public SGetShopCountDownMsg
{
	BYTE m_ShopType;	//�̳�����
};

struct SAGetShopCountDownMsg : public SGetShopCountDownMsg
{
	BYTE m_ShopType;	//�̳�����
	DWORD m_CountDownTime;	//����ʱʱ��
};


//�����ȡ��Ʒ��Ϣ����ͨ�����ߵ�ID
DECLARE_MSG(SFindGoodsMsg, SShopMsg, SShopMsg::EPRO_SHOP_FINDGOODSBYITERMID)
struct SQFindGoodsMsg : public SFindGoodsMsg
{
	DWORD m_ItemID;		//����ID
};

struct SAFindGoodsMsg : public SFindGoodsMsg
{
	GoodsData m_GoodsData;	//��Ʒ��Ϣ
};

//add by ly 2014/6/25
//��ҳ�ֵ
DECLARE_MSG(SPlayerPayMsg, SShopMsg, SShopMsg::EPRO_PLAYER_PAY)
struct SQPlayerPayMsg : public SPlayerPayMsg
{
	DWORD m_PayCount;		//��ҳ�ֵ����
};

struct SAPlayerPayMsg : public SPlayerPayMsg
{
	BYTE m_Result;	//��ֵ���
};