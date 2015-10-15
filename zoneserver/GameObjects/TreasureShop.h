#pragma once
#include "Networkmodule\ItemTypedef.h"
#include "Networkmodule\ItemMsgs.h"

struct GoodItem
{
	void operator = ( GoodItem item )
	{
		itemID = item.itemID;
		type1 = item.type1;
		type2 = item.type2;
		newRice = item.newRice;
		oldRice = item.oldRice;
		num = item.num;
	}

	WORD itemID;
	BYTE type1;
	BYTE type2;
	DWORD newRice;
	DWORD oldRice;
	WORD num; 
};

// ∞Ÿ±¶…Ã≥«
class TreasureShop
{
public:
	TreasureShop();
	~TreasureShop();

	static BOOL LoadTreasureShopData( std::string mItemFileName );
	static void UpdateAllGoods();

	void OnUpdateItems( CPlayer *player, SQUpdateItemTreasureShopMsg *pMsg );
	BOOL BuyGoods( class CPlayer *player, SQTreasureShopMsg *msg );
	BOOL BuyItem( CPlayer *player, GoodItem &goodItem, DWORD count );
};
