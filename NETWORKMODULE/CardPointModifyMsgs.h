#pragma once

#include "NetModule.h"
// #include "区域服务器//GameObjects/QuestCardPointModify.h"
#include "../pub/ConstValue.h"

// 在账号服务器统计服务器点数交易总信息的方案
// 为了防止交易中出现了漏洞，需要能够判断服务器是否凭空增加点数，是否出现了复制！
// 现在处理交易部分的方法为，划分出来，单独处理，所以可以统计总的交易量
// 。。。。。还是没有说到关键问题哈。。。比较麻烦。。。


// 把对于点卡相关的元宝积分的消息也统一在这儿
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 点数交易部分的消息类
//=============================================================================================
DECLARE_MSG_MAP(SPointModifyMsg, SMessage, SMessage::EPRO_POINTMODIFY_MESSAGE)
//{{AFX
EPRO_PREPARE_EXCHANGE,      // 到账号服务器上准备交易，查询点数是否足够，划分一部分用于交易的点数到临时空间
EPRO_CONFIRM_EXCHANGE,      // 到账号服务器上确认交易，将临时空间的点数转账到实际目标身上
EPRO_REFRESH_CARDPOINT,     // 通知区域服务器刷新一个服务器上某个玩家的点数信息
EPRO_MONEYPOINT_BASE,		// 刷新元宝积分和其他的元宝操作的基类
EPRO_EXCHANGEMC_BASE,		// 侠义币换点卡的交易消息
EPRO_EQUIVALENT_BASE,		// 侠义点数交易基类（用于免费版本）
EPRO_ENCOURAGEEMENT,        // 骏网抽奖
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



////=============================================================================================
//// 到账号服务器上准备交易，查询点数是否足够，划分一部分用于交易的点数到临时空间
//DECLARE_MSG(SPrepareExchangeMsg, SPointModifyMsg, SPointModifyMsg::EPRO_PREPARE_EXCHANGE)
//struct SQPrepareExchangeMsg : public SPrepareExchangeMsg
//{
//    SExchangeVerifyInfo vi;
//    dwt::stringkey<char [ACCOUNTSIZE]> SrcAccount;
//    dwt::stringkey<char [ACCOUNTSIZE]> DstAccount;
//    DWORD TradePoint;
//    DWORD SrcGID;
//    DWORD DstGID;
//};
//
//struct SAPrepareExchangeMsg : public SPrepareExchangeMsg
//{
//    enum
//    {
//        RET_SUCCESS,
//        RET_FAILED,
//        RET_NOTENOUGH_POINT,
//    };
//
//    BYTE result;
//    SExchangeVerifyInfo vi;
//};
////---------------------------------------------------------------------------------------------



////=============================================================================================
//// 到账号服务器上准备交易，查询点数是否足够，划分一部分用于交易的点数到临时空间
//DECLARE_MSG(SConfirmExchangeMsg, SPointModifyMsg, SPointModifyMsg::EPRO_CONFIRM_EXCHANGE)
//struct SQConfirmExchangeMsg : public SConfirmExchangeMsg
//{
//    SExchangeVerifyInfo vi;
//    dwt::stringkey<char [ACCOUNTSIZE]> SrcAccount;
//    dwt::stringkey<char [ACCOUNTSIZE]> DstAccount;
//    DWORD TradePoint;
//    DWORD SrcGID;
//    DWORD DstGID;
//
//    BOOL isConfirm; //OrCancel;
//};
////---------------------------------------------------------------------------------------------



//=============================================================================================
// 通知区域服务器刷新一个服务器上某个玩家的点数信息
DECLARE_MSG(SRefreshCardPointMsg, SPointModifyMsg, SPointModifyMsg::EPRO_REFRESH_CARDPOINT)
struct SARefreshCardPointMsg : 
    SRefreshCardPointMsg
{
    DWORD dwGID;
    DWORD dwCardPoint;

	// 这里增加ADDHERE
};
//---------------------------------------------------------------------------------------------

///////////////////////////////////////////////////////////
// 元宝积分的消息
//=============================================================================================
DECLARE_MSG_MAP(SMoneyPointModifyMsg, SPointModifyMsg, SPointModifyMsg::EPRO_MONEYPOINT_BASE)
//{{AFX
EPRO_REFRESH_MONEYPOINT,	// 刷新元宝积分
EPRO_NOTIFY_MONEYPOINT,		// 通报区域执行情况
//}}AFX
END_MSG_MAP()

// =============================================================
// 刷新元宝数据
//////////////////////////////////////////////////////////////////////
// 元宝积分 因为跟点卡的性质一样 所以也放在这儿
#include "MoneyPointDef.h"
DECLARE_MSG(SRefreshMoneyPointMsg, SMoneyPointModifyMsg, SMoneyPointModifyMsg::EPRO_REFRESH_MONEYPOINT)
struct SQRefreshMoneyPointMsg : public SRefreshMoneyPointMsg
{
	SMoneyPointNotify mpn;
	DWORD dwMoneyPoint;

	// char szAccount[ACCOUNTSIZE];
    char streamData[MAX_ACCOUNT];
};

struct SARefreshMoneyPointMsg : public SRefreshMoneyPointMsg
{
	enum { R_INVALID_ACCOUNT, R_NOTENOUGH_POINT, R_OK, R_TIMEIN, R_TIMEOUT};
	
	DWORD dwGID;
	DWORD	dwRet;
    DWORD dwMoneyPoint;

	SMoneyPointNotify mpn;
};

DECLARE_MSG(SNotifyMoneyPointMsg, SMoneyPointModifyMsg, SMoneyPointModifyMsg::EPRO_NOTIFY_MONEYPOINT)
struct SQNotifyMoneyPointMsg : public SNotifyMoneyPointMsg
{
	enum { RET_OK, RET_FAIL };
	// char szAccount[ACCOUNTSIZE];
	DWORD dwOpeate;
	DWORD dwMoneyPoint;

    char streamData[MAX_ACCOUNT];
};
//---------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////
// 侠义币换点卡消息
//=============================================================================================
DECLARE_MSG_MAP(SExchangeMCMsg, SPointModifyMsg, SPointModifyMsg::EPRO_EXCHANGEMC_BASE)
//{{AFX
EPRO_REQUEST_MC,		// 请求侠义币换点卡/
EPRO_REQUEST_CM,		// 点卡换侠义币
EPRO_REQUEST_MCINFO,	// 请求察看交换信息
EPRO_REQUEST_VALIDATE,  // 请求验证点卡号密码
//}}AFX
END_MSG_MAP()
// =============================================================
DECLARE_MSG(SRequestMCMsg, SExchangeMCMsg, SExchangeMCMsg::EPRO_REQUEST_MC)
struct SQRequestMCMsg : public SRequestMCMsg
{
	enum RMCType { RT_MC, RT_DELMEINFO };
	WORD wType;

	MCCLIENTINFO info;
};

DECLARE_MSG(SRequestCMMsg, SExchangeMCMsg, SExchangeMCMsg::EPRO_REQUEST_CM)
struct SQRequestCMMsg : public SRequestCMMsg
{
	MCCLIENTINFO info;
	char szCardNumber[30];
	char szCardPassword[30];
};

DECLARE_MSG(SRequestMCInfoMsg, SExchangeMCMsg, SExchangeMCMsg::EPRO_REQUEST_MCINFO)
struct SQRequestMCInfoMsg : public SRequestMCInfoMsg
{
	enum RType { RT_COM180, RT_COM600, RT_SZ50, RT_SZ100, RT_ME };
	WORD wType;
};

struct SARequestMCInfoMsg : public SRequestMCInfoMsg
{
	enum INFOType { IT_ALL, IT_ME };
	WORD	wType;
	WORD	wSize;
	WORD	wPage;			// 传输的数据可能会有很多页的状况 这个值代表所处的页
	MCCLIENTINFO	info[100];

	WORD  MySize()
	{
		return sizeof(SARequestMCInfoMsg) - sizeof(MCCLIENTINFO) * (100 - wSize);
	}
};

DECLARE_MSG(SRequestValidateMsg, SExchangeMCMsg, SExchangeMCMsg::EPRO_REQUEST_VALIDATE)
struct SQRequestValidateMsg : SRequestValidateMsg
{
	enum VTYPE { VT_CHECK, VT_COMPLETE };

	char szCardNumber[30];
	char szCardPassword[30];
	//char szAccount[ACCOUNTSIZE];			
	WORD  wOperateType;
	DWORD wCardType;       

    char streamData[MAX_ACCOUNT];
};

struct SARequestValidateMsg : SRequestValidateMsg
{
	enum STATE { ST_OK, ST_FAIL, ST_HADUSE, ST_INVALIDCARDORPWD, ST_ERRCARDTYPE, ST_COMPLETEERR };
	WORD wState;

	char szCardNumber[30];
	char szCardPassword[30];
	//char szAccount[ACCOUNTSIZE];
	DWORD wCardType;

    char streamData[MAX_ACCOUNT];
};

#include "networkmodule\eqvtypedef.h"

///////////////////////////////////////////////////////////
// 侠义点数的消息
//=============================================================================================
DECLARE_MSG_MAP(SEquivalentModifyMsg, SPointModifyMsg, SPointModifyMsg::EPRO_EQUIVALENT_BASE)
//{{AFX
EPRO_GET_EQUIVALENT,	// 获取对应点数
EPRO_INC_EQUIVALENT,	// 增加对应点数
EPRO_DEC_EQUIVALENT,	// 扣掉对应点数
//}}AFX
END_MSG_MAP()

DECLARE_MSG(SGetEQVPointMsg, SEquivalentModifyMsg, SEquivalentModifyMsg::EPRO_GET_EQUIVALENT)
struct SQGetEQVPointMsg : public SGetEQVPointMsg
{
	//EQUIVALENT type;
	//char szAccount[ACCOUNTSIZE];
    char streamData[MAX_ACCOUNT];
};

struct SAGetEQVPointMsg : public SGetEQVPointMsg
{
	SAGetEQVPointMsg() : dwEQVPoint(0)
	{
	}

//	EQUIVALENT type;
	DWORD dwEQVPoint;
	//char szAccount[ACCOUNTSIZE];
    char streamData[MAX_ACCOUNT];
};

DECLARE_MSG(SIncEQVPointMsg, SEquivalentModifyMsg, SEquivalentModifyMsg::EPRO_INC_EQUIVALENT)
struct SQIncEQVPointMsg : public SIncEQVPointMsg
{
//	EQUIVALENT type;
	//char szAccount[ACCOUNTSIZE];

	DWORD dwEQVPoint;
    char streamData[MAX_ACCOUNT];
};

struct SAIncEQVPointMsg : public SIncEQVPointMsg
{
//	EQUIVALENT type;
	DWORD dwEQVPoint;
	//char szAccount[ACCOUNTSIZE];
    char streamData[MAX_ACCOUNT];
};

DECLARE_MSG(SDecEQVPointMsg, SEquivalentModifyMsg, SEquivalentModifyMsg::EPRO_DEC_EQUIVALENT)
struct SQDecEQVPointMsg : public SDecEQVPointMsg
{
//	EQUIVALENT type;
	//char szAccount[ACCOUNTSIZE];

	int dwEQVPoint;//消费金额

//	CONSUMABLETYPE consumable;
	int nIndex;
    int nNumber;
	int nLevel; //玩家等级	

    char streamData[MAX_ACCOUNT];
};

struct SADecEQVPointMsg : public SDecEQVPointMsg
{
//	EQUIVALENT type;
	//char szAccount[ACCOUNTSIZE];

	int dwEQVPoint;//剩余点数
	DWORD dwDecPoint;//本次消费扣掉的点数

//	CONSUMABLETYPE consumable;
	int nIndex;
    int nNumber;

    char streamData[MAX_ACCOUNT];
};

DECLARE_MSG(SEncouragement, SPointModifyMsg, SPointModifyMsg::EPRO_ENCOURAGEEMENT)
struct SQEncouragement : public SEncouragement
{
    char m_szName[CONST_USERNAME];
	char m_szCardNumber[30];	
};
struct SAEncouragement: public SEncouragement
{
  char m_szName[CONST_USERNAME];
  BYTE m_iRet;
  enum
  {  
	  RET_INVALIDCARD = 2,
      RET_ISUSEED = 3,
	  RET_WEEKOK = 101,		//包周OK
	  RET_MONTH = 102, //包月OK
	  RET_ERROR = 200//玩家角色不存在
  };

};
