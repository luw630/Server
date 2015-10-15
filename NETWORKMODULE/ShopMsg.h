#pragma once
#include "NetModule.h"
//=============================================================================================
DECLARE_MSG_MAP(SShopMsg, SMessage, SMessage::EPRO_SHOP_MESSAGE)
//{{AFX
//2014/5/7 add by ly
EPRO_REQUEST_SHOP,	//玩家请求商城列表信息
EPRO_QUEST_SPECIFYSHOP,	//玩家请求特定商城中商品信息
EPRO_BUY_GOODS,		//玩家请求购买物品
EPRO_NOTITY_SPECIALGOODSUPDATE,	//通知玩家某个商城中的商品已经刷新
EPRO_SHOP_COUNTDOWN,	//请求商城倒计时时间
EPRO_SHOP_FINDGOODSBYITERMID,	//请求获取商品信息索引通过道具的ID

//add by ly 2014/6/25
EPRO_PLAYER_PAY,	//玩家充值
//}}AFX
END_MSG_MAP()


//2014/5/7 add by ly
//商品数据结构
typedef struct goodsdata
{
	DWORD m_GoodsIndex;	//商品索引(通过商城类型、商品主索引1、和商品主索引2计算而来)
	BYTE m_ShopType;	//商城类型
	BYTE m_GoodsType;	//商品类型
	DWORD m_GoodsID;	//商品ID
	WORD m_OriginalJinBiCost;	//金币原价
	WORD m_CurrentJinBiCost;	//金币现价
	WORD m_OriginalYinBiCost;	//银币原价
	WORD m_CurrentYinBiCost;	//银币现价
	WORD m_Count;	//每次刷出的数量
}GoodsData, *LPGoodsData;

//商城数据
typedef struct shopdata
{
	BYTE m_ShopType;	//商城类型
	BYTE m_RefreshFlag;	//商城刷新标志（0为不可刷新商城，1为可以刷新商城）
	BYTE m_RefreshType;	//商城刷新类型（为0时，不考虑；为1时，表示每天重置时间；为2时，表示间隔重置时间, 脚本重配后重新加载时也会重置）
	BYTE m_UpdateFlag;	//商城更新标识（默认为1，商品已经更新）
	DWORD m_RefreshAllTime;	//商城刷新所需总时间（当m_RefreshFlag = 1时才使用）

}ShopData, *LPShopData;

//玩家请求商城列表信息
DECLARE_MSG(SShopListMsg, SShopMsg, SShopMsg::EPRO_REQUEST_SHOP)
struct SQShopListMsg : public SShopListMsg
{
};

struct SAShopListMsg : public SShopListMsg
{
	WORD m_ShopNum;	//商城数量
	LPShopData m_lpShopData;	//商城信息
};

//玩家请求特定商城中商品的列表信息
DECLARE_MSG(SShopGoodsListMsg, SShopMsg, SShopMsg::EPRO_QUEST_SPECIFYSHOP)
struct SQShopGoodsListMsg : public SShopGoodsListMsg
{
	BYTE m_ShopType;	//商城类型
};

struct SAShopGoodsListMsg : public SShopGoodsListMsg
{
	WORD m_GoodsNum;	//商品数量
	LPGoodsData m_lpGoodsData;	//商品信息
};

//玩家请求购买物品
DECLARE_MSG(SBuyGoodsMsg, SShopMsg, SShopMsg::EPRO_BUY_GOODS)
struct SQBuyGoodsMsg : public SBuyGoodsMsg
{
	BYTE m_BuyType;		//购买方式 1为金币购买；2为银币购买
	DWORD m_GoodsIndex;	//商品索引(通过商城类型、商品主索引1、和商品主索引2计算而来)
	WORD m_BuyNum;	//购买数量
};

struct SABuyGoodsMsg : public SBuyGoodsMsg
{
	BYTE m_BuyResult;	//购买结果，0表示失败，1表示成功
};

//通知玩家特价商品已经刷新
DECLARE_MSG(SNotifySpecialGoodsUpdateMsg, SShopMsg, SShopMsg::EPRO_NOTITY_SPECIALGOODSUPDATE)
struct SANotifySpecialGoodsUpdateMsg : public SNotifySpecialGoodsUpdateMsg
{
	BYTE m_ShopType;	//商城类型
	BYTE m_UpdateFlag;	//更新标识，为1表示该商城中的商品已经更新
	DWORD m_UpdateRefreshAllTime;	//商城刷新的所需总时间更新值（为0时不更新商城刷新的所需总时间）
};

//玩家请求商城刷新倒计时
DECLARE_MSG(SGetShopCountDownMsg, SShopMsg, SShopMsg::EPRO_SHOP_COUNTDOWN)
struct SQGetShopCountDownMsg : public SGetShopCountDownMsg
{
	BYTE m_ShopType;	//商城类型
};

struct SAGetShopCountDownMsg : public SGetShopCountDownMsg
{
	BYTE m_ShopType;	//商城类型
	DWORD m_CountDownTime;	//倒计时时间
};


//请求获取商品信息索引通过道具的ID
DECLARE_MSG(SFindGoodsMsg, SShopMsg, SShopMsg::EPRO_SHOP_FINDGOODSBYITERMID)
struct SQFindGoodsMsg : public SFindGoodsMsg
{
	DWORD m_ItemID;		//道具ID
};

struct SAFindGoodsMsg : public SFindGoodsMsg
{
	GoodsData m_GoodsData;	//商品信息
};

//add by ly 2014/6/25
//玩家充值
DECLARE_MSG(SPlayerPayMsg, SShopMsg, SShopMsg::EPRO_PLAYER_PAY)
struct SQPlayerPayMsg : public SPlayerPayMsg
{
	DWORD m_PayCount;		//玩家充值数量
};

struct SAPlayerPayMsg : public SPlayerPayMsg
{
	BYTE m_Result;	//充值结果
};