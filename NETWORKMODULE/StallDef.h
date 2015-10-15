#pragma once
#include "itemtypedef.h"

//20150427 wk �Ż�����,ȫ��Ϊ1
//#define MAX_SALEBOXWIDTH	6
//#define MAX_SALEBOXHEIGHT	4
//#define MAX_SALEITEMNUM		(MAX_SALEBOXWIDTH * MAX_SALEBOXHEIGHT)
//#define MAX_STALLNAME		19
//#define MAX_SALEINFO		51
//#define MAX_LOGLINES		40		// ���40������
//#define MAX_LOGSIZE			81		// ÿ�����40���ֽڵķ���

#define MAX_SALEBOXWIDTH	1
#define MAX_SALEBOXHEIGHT	1
#define MAX_SALEITEMNUM		(MAX_SALEBOXWIDTH * MAX_SALEBOXHEIGHT)
#define MAX_STALLNAME		1
#define MAX_SALEINFO		1
#define MAX_LOGLINES		1		// ���40������
#define MAX_LOGSIZE			1		// ÿ�����40���ֽڵķ���

// ÿһ����¼��Ӧһ�����
struct StallLogInfo
{
	DWORD	index;
	char	Info[MAX_LOGSIZE];
};

struct StallItemPos
{
	WORD	wCellPos;
	//BYTE	bX;
	//BYTE	bY;
};

struct SaleItem
{
	BYTE			bPos;	// ������Ʒ�ڰ�̯����λ�ӣ�0~23��
	DWORD			bPrice;	// ��ǰ���ۼ�
	SPackageItem	item;	// ��Ʒ
};

struct SALE_BOTHSIDES
{
	DWORD dwOtherGlobalID;
	DWORD dwSelfGlobalID;	
};

struct SaleSeeItemInfo 
{	
	WORD		ItemNumber;
	DWORD		dwOtherGlobalID;
	DWORD		dwSelfGlobalID;
	char		szStallInfo[MAX_SALEINFO];
	SaleItem	stItemBaseInfo[MAX_SALEITEMNUM];
};

struct SALE_BUYITEM 
{
	BYTE			bPos;			// ������Ʒ�ڰ�̯����λ�ӣ�0~23��
	WORD			wNumber;		// ��Ҫ����ĸ���
	SALE_BOTHSIDES	stBothSides;
};
