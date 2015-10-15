#include "StdAfx.h"
#include "TreasureShop.h"
#include "Player.h"
#include "globalfunctions.h"
#include "Region.h"
#include "Item.h"
#include "Networkmodule\ScriptMsgs.h"
#include "Networkmodule/CardPointModifyMsgs.h"

char nameList1[][128] = { "Ԫ��ר��", "����ר��", "����ר��" };
char nameList2[][128] = { "��Ʒ����", "�ۿ���Ʒ", "ҩˮ��Ʒ", "��ʯ�ϳ�", "�Ż����", "�ӻ��̵�" };	

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
		return rfalse( 0, 0, "�޷���[%s]��Ʒ�����б��ļ���", mItemFileName.c_str() );

    while(!stream.eof())
    {
        stream >> str;
        if(strcmp(str,"��") == 0)
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

	rfalse( 2, 1, "��ȡ��ȡ�ٱ����ݳɹ�" );
	return TRUE;
}

// ������Ʒ���ݵ��ͻ���
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
// ��ͻ��˸��°ٱ���Ʒ
void TreasureShop::OnUpdateItems( CPlayer *player, SQUpdateItemTreasureShopMsg *pMsg )
{
	if( !pMsg || !player )
		return;

	// �����ж���Ʒ�Ƿ���ȷ
	BYTE type[2];
	type[0] = ( pMsg->chargeItemType & 0xf0 ) >> 4;
	type[1] = pMsg->chargeItemType & 0xf;

	if( type[0] > 3 || type[0] < 1 )
		return;

	// 0 ���⴦����ʾ���� 1 || 2 || 3 
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
				type[ 1 ] = 0xf; // ��Ǹ�ҳ�޵���
		}
	}

	// �����ҳ�޵��� ���� ѡ��ĸ��ѷ�ʽ���е���
	if( ( type[ 1 ] == 0xf ) && ( _type != 0xf ) )
		type[ 1 ] = _type + 1;

	std::map< std::string, std::vector< GoodItem > >::iterator it2 = it1->second.find( nameList2[ type[ 1 ] - 1 ] );
	if( it2 == it1->second.end() )
		return;
 
	 // ����������� 500
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

// ������Ʒ
BOOL TreasureShop::BuyGoods( CPlayer *player, SQTreasureShopMsg *msg )
{
// 	if( !msg )
// 		return FALSE;
// 
// 	// �����ж���Ʒ�Ƿ���ȷ
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
  ������ index: ��Ʒ���
         count: ��������
		 type: ֧�����ͣ�Ԫ���������������
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
//     // ȷ���õ��ߵ�����ص�������
//     int maxOlap = ( !ITEM_IS_OVERLAP( itemData->byType ) ) ? 1 : itemData->wOverlay;
// 
//     if ( count > (DWORD)maxOlap )
//         count = 1;
// 
//     EQUIVALENT buyType = EEQ_MAXTYPE;
//     DWORD price = 0;
// 
// 	// Ԫ��
//     if ( type == 1 )
//     {
// 		if ( goodItem.newRice && ( player->m_Property.m_dwXYDPoint >= goodItem.newRice * count ) )
// 		{
// 			buyType = EEQ_XYDPOINT, price = goodItem.newRice;
// 			// ���²Ʋ�
// 			SATreasureShopMsg __msg;
// 			__msg.type_ = SQTreasureShopMsg::TYPE_MONEY;
// 			__msg.riches[0] = player->m_Property.m_dwXYDPoint - goodItem.newRice * count;  // Ԫ��
// 			__msg.riches[1] = player->m_Property.m_Money;  // ����
// 			__msg.riches[2] = player->m_Property.m_dwGiftCoupon;  // ����
// 
// 			g_StoreMessage( player->m_ClientIndex, &__msg, sizeof(SATreasureShopMsg) );
// 		}
//     }
// 	// ����
// 	else if ( type == 3 )
//     {
// 		if ( goodItem.newRice && ( player->m_Property.m_dwGiftCoupon >= goodItem.newRice * count ) )
// 		{
// 			buyType = EEQ_GIFTCOUPON_NEWXYD, price = goodItem.newRice;
// 			// ���²Ʋ�
// 			SATreasureShopMsg __msg;
// 			__msg.type_ = SQTreasureShopMsg::TYPE_MONEY;
// 			__msg.riches[0] = player->m_Property.m_dwXYDPoint;  // Ԫ��
// 			__msg.riches[1] = player->m_Property.m_Money;  // ����
// 			__msg.riches[2] = player->m_Property.m_dwGiftCoupon - goodItem.newRice * count;  // ����
// 
// 			g_StoreMessage( player->m_ClientIndex, &__msg, sizeof(SATreasureShopMsg) );
// 		}
//     }
// 	// ��Ǯ
// 	else if ( type == 2 )
//     {
// 	    // ����۸�С�������۸���Ϊ�Ǵ����
//         if ( goodItem.newRice < itemData->dwSell )
// 		    return FALSE;
// 
// 		if ( goodItem.newRice && ( player->m_Property.m_Money >= goodItem.newRice * count ) )
// 		{
// 			buyType = EEQ_MONEY, price = goodItem.newRice;
// 			// ���²Ʋ�
// 			SATreasureShopMsg __msg;
// 			__msg.type_ = SQTreasureShopMsg::TYPE_MONEY;
// 			__msg.riches[0] = player->m_Property.m_dwXYDPoint;  // Ԫ��
// 			__msg.riches[1] = player->m_Property.m_Money - goodItem.newRice * count;  // ����
// 			__msg.riches[2] = player->m_Property.m_dwGiftCoupon;  // ����
// 
// 			g_StoreMessage( player->m_ClientIndex, &__msg, sizeof(SATreasureShopMsg) );
// 		}
//     }
// 
//     SABuy msg;
//     if ( buyType == EEQ_MAXTYPE )    // Ǯ����
//     {
// // 	    msg.nRet = 0, msg.nMoney = player->m_Property.m_Money;
// // 	    g_StoreMessage( player->m_ClientIndex, &msg, sizeof( SABuy ) );
//         return FALSE;
//     }
// 	// ���������ң���ֱ�ӹ�����Ʒ
//     else if ( buyType == EEQ_MONEY )
//     {
//         if ( !GenerateNewItem( player, GenItemParams( index, count ), LogInfo( 6, "Ԫ���̳�") ) )
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
//                 ( 7, mapid )( 8, x )( 9, y )( 10, player->GetAccount() )( 11, "Ԫ���̳ǹ���" )END_SNOTIFY;
//         }
// // 
// //         msg.nRet = 1, msg.nMoney = ( player->m_Property.m_Money -= dwBuyMoney );
// 	    g_StoreMessage( player->m_ClientIndex, &msg, sizeof( SABuy ) );
// 
//         // ͳ���̵������Ľ�Ǯ
//         if ( ( __int64 )player->shopBuyMoney + dwBuyMoney < 0xffff0000 )
//             player->shopBuyMoney += dwBuyMoney;
// 
//         // ʹ������ҹ��򣬾Ͳ���Ҫ��¼��
//         return TRUE;
//     }
//     else if ( player->isWaitingLockedResult( FALSE, TRUE ) )
//     {
// //         msg.nRet = -1, msg.nMoney = player->m_Property.m_Money;
// // 	    g_StoreMessage( player->m_ClientIndex, &msg, sizeof( SABuy ) );
//         return FALSE;
//     }
//  
//     // ��Ԫ����������
//     // ����Ƿ��пռ����ɸõ���
// //     SCellPos pos;
// //     if ( !player->CheckAddItem( index, pos ) )
// //     {
// // 		player->SendErrorMsg( SABackMsg::B_FULLBAG );
// // 		return FALSE;
// //     }
// 
// 	// ������Ӷ����ɹ��������
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
// 	TraceInfo(szDirname + "EQVInfo.log", "[%s]����acc=%s, type=%d, item=%d, count=%d��",
// 	//TraceInfo( szDirname + "EQVInfo.log", 
//     //    "[%s]�����ף��˺ţ�%s, ʹ�û������ࣺ%d, �������ݣ�������Ϊ%d����Ʒ%d����",
//         GetStringTime(), player->GetAccount(), buyType, index, count );
// 
//     // ���ϱ�ǣ���������в������ٴι���
// 	player->m_bWaitBuyResult = true;
//     player->ATM_temporaryVerifier = timeGetTime();

	return TRUE;
}
