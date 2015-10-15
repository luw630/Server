#pragma once

#include "../pub/ConstValue.h"
// 通宝交换
#define MPE_MAX_ITEMS	5
struct SMoneyPointNotify
{
	enum OPTYPE
	{
		OPT_EXCHANGEITEM,			// 道具交换
		OPT_EXCHANGETELE,			// 心法交换
        OPT_EXBOTH,                 // 道具\心法交换
	};

	// char szAccount[ACCOUNTSIZE];				// 账号
	DWORD dwOprate;					// 此次的操作
	DWORD dwMoneyPoint;				// 操作的元宝积分数量	

	union 
	{	
		struct 
		{
			DWORD ItemID[MPE_MAX_ITEMS];
		}ITEMS;

		struct 
		{
			DWORD dwTeleID;
			DWORD dwTeleNum;
		}TELE;

		struct 
		{
            DWORD dwItemID;
			DWORD dwTeleID;
			DWORD dwTeleNum;
		}EXBOTH;
	}ATT;
};

// 侠义币换点卡
enum CARDTYPE { CT_COM180 = 0, CT_COM600, CT_SZ50, CT_SZ100 };

typedef struct _MCCLIENTINFO
{
	char szName[CONST_USERNAME];
	DWORD dwType;
	DWORD dwExMoney;
}MCCLIENTINFO, *LPMCCLIENTINFO;

const DWORD g_EMCNeedMoney = 30000;
const DWORD g_EMCMaxMoney = 1000000;


// 只能在此区域服务器编号上的交换
const WORD MCINFOSERVERID = 115;

