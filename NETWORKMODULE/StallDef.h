#pragma once
#include "itemtypedef.h"

//20150427 wk 优化数据,全改为1
//#define MAX_SALEBOXWIDTH	6
//#define MAX_SALEBOXHEIGHT	4
//#define MAX_SALEITEMNUM		(MAX_SALEBOXWIDTH * MAX_SALEBOXHEIGHT)
//#define MAX_STALLNAME		19
//#define MAX_SALEINFO		51
//#define MAX_LOGLINES		40		// 最大40条留言
//#define MAX_LOGSIZE			81		// 每条最大40个字节的符号

#define MAX_SALEBOXWIDTH	1
#define MAX_SALEBOXHEIGHT	1
#define MAX_SALEITEMNUM		(MAX_SALEBOXWIDTH * MAX_SALEBOXHEIGHT)
#define MAX_STALLNAME		1
#define MAX_SALEINFO		1
#define MAX_LOGLINES		1		// 最大40条留言
#define MAX_LOGSIZE			1		// 每条最大40个字节的符号

// 每一条记录对应一个编号
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
	BYTE			bPos;	// 代售物品在摆摊栏的位子（0~23）
	DWORD			bPrice;	// 当前的售价
	SPackageItem	item;	// 物品
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
	BYTE			bPos;			// 代售物品在摆摊栏的位子（0~23）
	WORD			wNumber;		// 需要购买的个数
	SALE_BOTHSIDES	stBothSides;
};
