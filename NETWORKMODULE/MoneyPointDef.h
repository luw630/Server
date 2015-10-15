#pragma once

#include "../pub/ConstValue.h"
// ͨ������
#define MPE_MAX_ITEMS	5
struct SMoneyPointNotify
{
	enum OPTYPE
	{
		OPT_EXCHANGEITEM,			// ���߽���
		OPT_EXCHANGETELE,			// �ķ�����
        OPT_EXBOTH,                 // ����\�ķ�����
	};

	// char szAccount[ACCOUNTSIZE];				// �˺�
	DWORD dwOprate;					// �˴εĲ���
	DWORD dwMoneyPoint;				// ������Ԫ����������	

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

// ����һ��㿨
enum CARDTYPE { CT_COM180 = 0, CT_COM600, CT_SZ50, CT_SZ100 };

typedef struct _MCCLIENTINFO
{
	char szName[CONST_USERNAME];
	DWORD dwType;
	DWORD dwExMoney;
}MCCLIENTINFO, *LPMCCLIENTINFO;

const DWORD g_EMCNeedMoney = 30000;
const DWORD g_EMCMaxMoney = 1000000;


// ֻ���ڴ��������������ϵĽ���
const WORD MCINFOSERVERID = 115;

