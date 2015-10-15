#include "StdAfx.h"
#include "TreasureShop.h"
#include "Player.h"
#include "globalfunctions.h"
#include "Region.h"
#include "Item.h"
#include "Networkmodule\ScriptMsgs.h"
#include "Networkmodule/CardPointModifyMsgs.h"

char nameList1[][128] = { "元宝专区", "银两专区", "赠宝专区" };
char nameList2[][128] = { "新品上市", "折扣商品", "药水物品", "宝石合成", "优惠礼包", "杂货商店" };	

static std::map< std::string, std::map< std::string, std::vector< GoodItem > > > m_wares;
extern BYTE hugeBuffer[ 0xffff ];

TreasureShop::TreasureShop()
{
}

TreasureShop::~TreasureShop()
{
}

BOOL TreasureShop::LoadTreasureShopData( std::string mItemFileName )
{
	m_wares.clear();

	// Init
	std::vector< GoodItem > item1;
	std::map< std::string, std::vector< GoodItem > > item2;
	for( int i = 0; i < 6; i ++ )
		item2[ nameList2[i] ] = item1;
	for( int i = 0; i < 4; i ++ )
		m_wares[ nameList1[i] ] = item2;

    static char str[128];
	dwt::ifstream stream( mItemFileName.c_str() );
    if ( !stream.is_open() )
		return rfalse( 0, 0, "无法打开[%s]物品数据列表文件！", mItemFileName.c_str() );

    while(!stream.eof())
    {
        stream >> str;
        if(strcmp(str,"·") == 0)
            break;
    }

    int s_lItemSize = 0;
    if (!stream.eof())
    {
        stream >> str;
        if( strcmp(str,"items") == 0)
            stream >> s_lItemSize;
    }

    if(s_lItemSize == 0) return FALSE;

    long nTimes = 0;
    while( nTimes < s_lItemSize )
    {
		GoodItem psi;

        int dwTemp = 0;
		WORD itemID = 0;

		psi.type1   =       ( ( stream >> dwTemp ), dwTemp ); 
		psi.type2   =       ( ( stream >> dwTemp ), dwTemp ); 
		psi.itemID  =       ( ( stream >> dwTemp ), dwTemp ); 
		psi.newRice =       ( ( stream >> dwTemp ), dwTemp ); 
		psi.oldRice =       ( ( stream >> dwTemp ), dwTemp ); 
		psi.num     =       ( ( stream >> dwTemp ), dwTemp ); 
 
		if( psi.type1 > 3 || psi.type1 < 1 )
			psi.type1 = 3;

		if( psi.type2 > 6 || psi.type2 < 1 )
			psi.type2 = 6;

		std::map< std::string, std::map< std::string, std::vector< GoodItem > > >::iterator it1 = m_wares.find( nameList1[psi.type1-1] );
		if( it1 == m_wares.end() )
			return FALSE;

		std::map< std::string, std::vector< GoodItem > >::iterator                          it2 = it1->second.find( nameList2[psi.type2-1] );
		if( it2 == it1->second.end() )
			return FALSE;

		it2->second.push_back( psi );

		nTimes++;
    }
    stream.close();

	rfalse( 2, 1, "读取读取百宝数据成功" );
	return TRUE;
}

// 更新商品内容到客户端
void TreasureShop::UpdateAllGoods()
{
}

int GetIdx( const std::string &str )
{
	int return_value = -1;
	for( int i = 0; i < 6; i ++ )
	{
		if( strcmp( str.c_str(), nameList2[ i ] ) == 0 )
		{
			return_value = i;
			break;
		}
	}

	return return_value;
}
// 向客户端更新百宝物品
void TreasureShop::OnUpdateItems( CPlayer *player, SQUpdateItemTreasureShopMsg *pMsg )
{
	if( !pMsg || !player )
		return;

	// 首先判断物品是否正确
	BYTE type[2];
	type[0] = ( pMsg->chargeItemType & 0xf0 ) >> 4;
	type[1] = pMsg->chargeItemType & 0xf;

	if( type[0] > 3 || type[0] < 1 )
		return;

	// 0 特殊处理，表示按下 1 || 2 || 3 
	if( type[1] > 6 || type[1] < 1 )
		return;

	std::map< std::string, std::map< std::string, std::vector< GoodItem > > >::iterator it1 = m_wares.find( nameList1[ type[ 0 ] - 1] );
	if( it1 == m_wares.end() )
		return;

#undef new
	SAUpdateItemTreasureShopMsg &msg = * new ( hugeBuffer ) SAUpdateItemTreasureShopMsg;
	msg.shows = 0;
	BYTE _type = 0xf;
	std::map< std::string, std::vector< GoodItem > >::iterator it = it1->second.begin();
	for( ; it != it1->second.end(); it ++ )
	{
		int i = GetIdx( it->first );
		if( i != -1 )
		{
			if( it->second.size() > 0 )
			{
				msg.shows |= ( 1 << ( 7 - i ) );
				if( _type > i )
					_type = i;
			}
			else if( type[ 1 ] == ( i + 1 ) )
				type[ 1 ] = 0xf; // 标记该页无道具
		}
	}

	// 如果该页无道具 而且 选择的付费方式内有道具
	if( ( type[ 1 ] == 0xf ) && ( _type != 0xf ) )
		type[ 1 ] = _type + 1;

	std::map< std::string, std::vector< GoodItem > >::iterator it2 = it1->second.find( nameList2[ type[ 1 ] - 1 ] );
	if( it2 == it1->second.end() )
		return;
 
	 // 做了最大限制 500
	if(/* it2->second.size() == 0 || */it2->second.size() > 500 )
		return;

	msg.type_ = pMsg->type_;
	msg.chargeItemType = pMsg->chargeItemType;
	msg.chargeItemType &= 0xf0;
	msg.chargeItemType |= type[ 1 ] & 0xf;
	msg.itemNum = (WORD)it2->second.size();
	GoodItem *gitem = ( GoodItem* )msg.buffer;
	for( unsigned int i = 0; i < it2->second.size(); i ++ )
		gitem[ i ] = it2->second[i];
  
	g_StoreMessage( player->m_ClientIndex, &msg, ( sizeof(SAUpdateItemTreasureShopMsg) - 1 + sizeof( GoodItem ) * msg.itemNum ) );

}

// 购买商品
BOOL TreasureShop::BuyGoods( CPlayer *player, SQTreasureShopMsg *msg )
{
// 	if( !msg )
// 		return FALSE;
// 
// 	// 首先判断物品是否正确
// 	BYTE type[2];
// 	type[0] = msg->type[0];
// 	type[1] = msg->type[1];
// 
// 	if( type[0] > 3 || type[0] < 1 )
// 		return FALSE;
// 
// 	if( type[1] > 6 || type[1] < 1 )
// 		return FALSE;
// 
// 	std::map< std::string, std::map< std::string, std::vector< GoodItem > > >::iterator it1 = m_wares.find( nameList1[type[0]-1] );
// 	if( it1 == m_wares.end() )
// 		return FALSE;
// 
// 	std::map< std::string, std::vector< GoodItem > >::iterator it2 = it1->second.find( nameList2[type[1]-1] );
// 	if( it2 == it1->second.end() )
// 		return FALSE;  
// 
// 	//CPlayer *player = static_cast<CPlayer*>(this);
// 	//if ( player == NULL )
// 	//	return FALSE;
// 
// 	int i = 0;
// 	for(; i < (int)it2->second.size(); i ++ )
// 	{
// 		if( it2->second[i].itemID == msg->itemID )
// 		{
// //			player->BuyItem( msg->itemID, msg->num );
// 			BuyItem( player, it2->second[i], msg->num );
//             break;
// 		}
// 	}

	//if( i == it2->second.size() )
	//{
	//}

	//SQTreasureShopMsg _msg;
	//_msg.itemID = 0;
	//_msg.num = 0;
	//_msg.type[0] = _msg.type[1] = 0;
	//_msg.type_ = SQTreasureShopMsg::TYPE_BUY;
	//g_StoreMessage( player->m_ClientIndex, &_msg, sizeof(SQTreasureShopMsg) );

	return TRUE;
}
/**
  参数： index: 物品编号
         count: 购买数量
		 type: 支付类型：元宝；赠宝；狭义币
*/
BOOL TreasureShop::BuyItem( CPlayer *player, GoodItem &goodItem, DWORD count )
{
// 	WORD index = goodItem.itemID;
// 	BYTE type = goodItem.type1;
// 
// 	if ( goodItem.newRice == 0 )
// 		return FALSE;
// 
//     //CPlayer *player = static_cast<CPlayer*>(this);
//     //const SItemData *itemData = CItem::GetItemData( index );
//     //if ( player == NULL || itemData == NULL )
//     //    return FALSE;
// 
//     const SItemData *itemData = CItem::GetItemData( index );
//     if ( itemData == NULL )
//         return FALSE;
// 
//     // 确定该道具的最大重叠数量！
//     int maxOlap = ( !ITEM_IS_OVERLAP( itemData->byType ) ) ? 1 : itemData->wOverlay;
// 
//     if ( count > (DWORD)maxOlap )
//         count = 1;
// 
//     EQUIVALENT buyType = EEQ_MAXTYPE;
//     DWORD price = 0;
// 
// 	// 元宝
//     if ( type == 1 )
//     {
// 		if ( goodItem.newRice && ( player->m_Property.m_dwXYDPoint >= goodItem.newRice * count ) )
// 		{
// 			buyType = EEQ_XYDPOINT, price = goodItem.newRice;
// 			// 更新财产
// 			SATreasureShopMsg __msg;
// 			__msg.type_ = SQTreasureShopMsg::TYPE_MONEY;
// 			__msg.riches[0] = player->m_Property.m_dwXYDPoint - goodItem.newRice * count;  // 元宝
// 			__msg.riches[1] = player->m_Property.m_Money;  // 银两
// 			__msg.riches[2] = player->m_Property.m_dwGiftCoupon;  // 赠宝
// 
// 			g_StoreMessage( player->m_ClientIndex, &__msg, sizeof(SATreasureShopMsg) );
// 		}
//     }
// 	// 赠宝
// 	else if ( type == 3 )
//     {
// 		if ( goodItem.newRice && ( player->m_Property.m_dwGiftCoupon >= goodItem.newRice * count ) )
// 		{
// 			buyType = EEQ_GIFTCOUPON_NEWXYD, price = goodItem.newRice;
// 			// 更新财产
// 			SATreasureShopMsg __msg;
// 			__msg.type_ = SQTreasureShopMsg::TYPE_MONEY;
// 			__msg.riches[0] = player->m_Property.m_dwXYDPoint;  // 元宝
// 			__msg.riches[1] = player->m_Property.m_Money;  // 银两
// 			__msg.riches[2] = player->m_Property.m_dwGiftCoupon - goodItem.newRice * count;  // 赠宝
// 
// 			g_StoreMessage( player->m_ClientIndex, &__msg, sizeof(SATreasureShopMsg) );
// 		}
//     }
// 	// 金钱
// 	else if ( type == 2 )
//     {
// 	    // 购买价格小于卖出价格，视为是错误的
//         if ( goodItem.newRice < itemData->dwSell )
// 		    return FALSE;
// 
// 		if ( goodItem.newRice && ( player->m_Property.m_Money >= goodItem.newRice * count ) )
// 		{
// 			buyType = EEQ_MONEY, price = goodItem.newRice;
// 			// 更新财产
// 			SATreasureShopMsg __msg;
// 			__msg.type_ = SQTreasureShopMsg::TYPE_MONEY;
// 			__msg.riches[0] = player->m_Property.m_dwXYDPoint;  // 元宝
// 			__msg.riches[1] = player->m_Property.m_Money - goodItem.newRice * count;  // 银两
// 			__msg.riches[2] = player->m_Property.m_dwGiftCoupon;  // 赠宝
// 
// 			g_StoreMessage( player->m_ClientIndex, &__msg, sizeof(SATreasureShopMsg) );
// 		}
//     }
// 
//     SABuy msg;
//     if ( buyType == EEQ_MAXTYPE )    // 钱不够
//     {
// // 	    msg.nRet = 0, msg.nMoney = player->m_Property.m_Money;
// // 	    g_StoreMessage( player->m_ClientIndex, &msg, sizeof( SABuy ) );
//         return FALSE;
//     }
// 	// 如果是侠义币，就直接购买物品
//     else if ( buyType == EEQ_MONEY )
//     {
//         if ( !GenerateNewItem( player, GenItemParams( index, count ), LogInfo( 6, "元宝商城") ) )
//         {
// 			//SendErrorMsg( SABackMsg::EXM_FAIL );
// 			return FALSE;
//         }
// 
//         DWORD dwBuyMoney = /*itemData->dwBuy*/goodItem.newRice * count;
// 
//         extern DWORD rcdMoneyRank;
//         if ( player->m_ParentRegion && ( dwBuyMoney >= rcdMoneyRank ) )
//         {
//             int mapid = player->m_ParentRegion->m_wRegionID;
//             int x = player->m_wCurX;
//             int y = player->m_wCurY;
// 
//             BEGIN_SNOTIFY( 100 )( 0, ( int )player->GetSID() )( 1, 3 )( 2, ( int )goodItem.itemID )( 3, ( int )count )
//                 ( 4, ( int )player->m_Property.m_Money )( 5, -( int )dwBuyMoney )
//                 ( 7, mapid )( 8, x )( 9, y )( 10, player->GetAccount() )( 11, "元宝商城购买" )END_SNOTIFY;
//         }
// // 
// //         msg.nRet = 1, msg.nMoney = ( player->m_Property.m_Money -= dwBuyMoney );
// 	    g_StoreMessage( player->m_ClientIndex, &msg, sizeof( SABuy ) );
// 
//         // 统计商店卖出的金钱
//         if ( ( __int64 )player->shopBuyMoney + dwBuyMoney < 0xffff0000 )
//             player->shopBuyMoney += dwBuyMoney;
// 
//         // 使用侠义币购买，就不需要记录了
//         return TRUE;
//     }
//     else if ( player->isWaitingLockedResult( FALSE, TRUE ) )
//     {
// //         msg.nRet = -1, msg.nMoney = player->m_Property.m_Money;
// // 	    g_StoreMessage( player->m_ClientIndex, &msg, sizeof( SABuy ) );
//         return FALSE;
//     }
//  
//     // 用元宝或赠宝买
//     // 检测是否有空间容纳该道具
// //     SCellPos pos;
// //     if ( !player->CheckAddItem( index, pos ) )
// //     {
// // 		player->SendErrorMsg( SABackMsg::B_FULLBAG );
// // 		return FALSE;
// //     }
// 
// 	// 尝试添加东西成功的情况下
// 	SQDecEQVPointMsg eqvmsg;
// 	eqvmsg.type = buyType;
// 	eqvmsg.dwEQVPoint = price;
// 	eqvmsg.consumable = ECT_BUYITEM;
// 	eqvmsg.nIndex = index;
//     eqvmsg.nNumber = count;
// 	eqvmsg.nLevel = player->m_byLevel;
// 
//     try
//     {
//         lite::Serializer slm( eqvmsg.streamData, sizeof( eqvmsg.streamData ) );
// 		slm( player->GetAccount() );
//         SendToLoginServer( &eqvmsg, ( long )( sizeof(eqvmsg) - slm.EndEdition() ) );
//     }
//     catch ( lite::Xcpt & )
//     {
//         return FALSE;
//     }
// 
// 	extern BOOL TraceInfo(LPCSTR szFileName, LPCSTR szFormat, ...);
// 	extern LPCSTR GetStringTime();
//     extern dwt::stringkey<char [256]> szDirname;
// 
// 	TraceInfo(szDirname + "EQVInfo.log", "[%s]请求：acc=%s, type=%d, item=%d, count=%d。",
// 	//TraceInfo( szDirname + "EQVInfo.log", 
//     //    "[%s]请求交易：账号：%s, 使用货币种类：%d, 消费内容：购买编号为%d的物品%d个。",
//         GetStringTime(), player->GetAccount(), buyType, index, count );
// 
//     // 打上标记，这个过程中不允许再次购买
// 	player->m_bWaitBuyResult = true;
//     player->ATM_temporaryVerifier = timeGetTime();

	return TRUE;
}
