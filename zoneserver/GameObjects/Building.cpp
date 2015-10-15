#include "StdAfx.h"
#include "building.h"
#include "player.h"
#include "region.h"
#include "area.h"
#include "ScriptManager.h"

#include "networkmodule\regionmsgs.h"

extern LPCSTR FormatString(LPCSTR szFormat, ...);
extern BOOL GenerateNewUniqueId( SItemBase &item );

static const DWORD MAXBUILDING = 20000;
static std::list< SBuildingConfigData > s_buildingDatas;
static std::pair< DWORD, const SBuildingConfigData* > s_buildingTable[ MAXBUILDING ];

// ��İ��ɵ�����
extern BOOL GetFactionInfo( LPCSTR szFactionName, SFactionData &stFactionInfo );
//extern BOOL GetFactionHeaderInfo( LPCSTR szFactionName, SFactionData::SFaction &factionHeader );

extern void TalkToDnid(DNID dnidClient, LPCSTR info,bool addPrefix,WORD wShowType);
extern BYTE commBuffer[ 0xfff ];

extern LPIObject FindRegionByID( DWORD ID );

QWORD HexStringToNumber( LPCSTR str )
{
    #define IsHexNumber( c ) ( ( c >= '0'&& c <= '9' ) || ( c >= 'A'&& c <= 'F' ) || ( c >= 'a'&& c <= 'f' ) )
    #define GetHexNumber( c ) ( ( c >= '0'&& c <= '9' ) ? ( c - '0' ) : ( 10 + ( ( c >= 'A'&& c <= 'F' ) ? ( c - 'A' ) : ( c - 'a' ) ) ) )

    if ( str == NULL )
        return 0;

    QWORD result = 0;

    if ( str[0] == '0' && str[1] == 'x' )
        str += 2;

    for ( int i = 0; i < 16; i ++ )
    {
        if ( !IsHexNumber( str[i] ) )
            break;

        result = ( result << 4 ) | GetHexNumber( str[i] );
    }

    #undef GetHexNumber
    #undef IsHexNumber

    return result;
}

int CBuilding::LoadBuildingData()
{
    memset( s_buildingTable, 0, sizeof( s_buildingTable ) );

	char lineBuffer[1024];
	dwt::ifstream stream( "Data\\building\\building.def" );
	if ( !stream.is_open() )
		return rfalse( 0, 1, "�޷���[Data\\building\\building.def]�����������б��ļ���" );

    while ( !stream.eof() )
	{
        stream.getline( lineBuffer, sizeof( lineBuffer ) );
        if ( *lineBuffer == 0 || *( LPWORD )lineBuffer == '//' || *( LPWORD )lineBuffer == '--' )
            continue;

        std::strstream lineStream( lineBuffer, ( int )strlen( lineBuffer ), ios_base::in );

        DWORD tempDword;
        SBuildingConfigData tempData;
        memset( &tempData, 0, sizeof( tempData ) );

        lineStream >> tempData.index;
        if ( tempData.index == 0 || tempData.index >= MAXBUILDING )
            continue;

        lineStream >> lineBuffer; // building name is't dummy

        lineStream >> tempData.image >> lineBuffer >> tempData.hpMax;
        tempData.qwtileMask = HexStringToNumber( lineBuffer );

		lineStream >> lineBuffer;
		tempData.qwtileLimit = HexStringToNumber( lineBuffer );

        tempData.centerx		= ( BYTE )( ( lineStream >> tempDword ), tempDword );
        tempData.centery		= ( BYTE )( ( lineStream >> tempDword ), tempDword );
        /*tempData.buildType	= */( BYTE )( ( lineStream >> tempDword ), tempDword );
        /*tempData.buildSpeed	= */( BYTE )( ( lineStream >> tempDword ), tempDword );
        /*tempData.material		= */( BYTE )( ( lineStream >> tempDword ), tempDword );
        /*tempData.maintenance	= */( BYTE )( ( lineStream >> tempDword ), tempDword );
		tempData.line.seg.x		= ( short )( ( lineStream >> tempDword ), tempDword );
		tempData.line.seg.y		= ( short )( ( lineStream >> tempDword ), tempDword );
		tempData.line.end.x		= ( short )( ( lineStream >> tempDword ), tempDword );
		tempData.line.end.y		= ( short )( ( lineStream >> tempDword ), tempDword );
		tempData.ctrlScript	= ( WORD )( ( lineStream >> tempDword ), tempDword );
		tempData.initScript = ( WORD )( ( lineStream >> tempDword ), tempDword );

        s_buildingDatas.push_back( tempData );
        s_buildingTable[ tempData.index ].first = 0;
        s_buildingTable[ tempData.index ].second = &s_buildingDatas.back();
	}

	return 1;
}

static void ClearBuildingData()
{
}

void CBuilding::OnRecvBuildingMsgs( struct SBuildingBaseMsg *pMsg, class CPlayer *sender )
{
    assert( ( pMsg != NULL ) && ( sender != NULL ) );

    switch ( ( ( SBuildingBaseMsg* )pMsg )->_protocol )
    {
    case SBuildingBaseMsg::EPRO_BUILDSTRUCTURE:
        {
            SQBuildStructureMsg *tempMsg = ( SQBuildStructureMsg* )pMsg;
            if ( ( sender->buildVerify == 0 ) || ( sender->buildVerify != tempMsg->index ) )
                return;

            if ( sender->m_ParentRegion == NULL )
                return;

            // ���, ��������ظ�����...
            sender->buildVerify = 0;

            CreateParameter param;
            ZeroMemory( &param, sizeof( param ) );
            param.index = tempMsg->index;
            // param.inverse = tempMsg->inverse; ��ʱ���η��򡣡���
            param.x = tempMsg->x;
            param.y = tempMsg->y;
            if ( sender->m_Property.m_szTongName[0] )
            {
                param.ownerType = 2;
                dwt::strcpy( param.ownerName, sender->m_Property.m_szTongName, sizeof( param.ownerName ) );
            }
            else
            {
                param.ownerType = 1;
                dwt::strcpy( param.ownerName, sender->GetName(), sizeof( param.ownerName ) );
            }

            sender->m_ParentRegion->CreateBuilding( &param, sender, TRUE );
        }
        break;
    }
}

const SBuildingConfigData* CBuilding::GetBuildingData( WORD index )
{
    if ( index >= MAXBUILDING )
        return NULL;

    return s_buildingTable[ index ].second;
}

CBuilding::CBuilding(void)
{
    checkId = 10;
	controlId = 0;
}

CBuilding::~CBuilding(void)
{
}

int CBuilding::OnCreate( _W64 long pParameter )
{
    //if ( sizeof( CreateParameter ) != size )
    //    return false;

	param = *( CreateParameter * )pParameter;

    if ( param.loadData == NULL )
    {
        const SBuildingConfigData *buidlingData = GetBuildingData( param.index );
        if ( buidlingData == NULL )
            return false;

        memset( &property, 0, sizeof( property ) );

        property.index = param.index;
        property.inverse = param.inverse;
        //property.ownerType = param.ownerType;
        //memcpy( property.owner, param.ownerName, sizeof( property.owner ) );

//         property.curX = m_wCurX = param.x;
//         property.curY = m_wCurY = param.y;

        // ��ʱ������������Ϊͣ��״̬
        property.usingState = 0;
        property.ownerType = param.ownerType;
        property.owner = param.ownerName;

	    if ( !GenerateNewUniqueId( reinterpret_cast< SItemBase& >( property ) ) )
	    {
		    rfalse( 2, 1, "��������ΨһID����!" );
		    return false;
	    }
    }
    else
    {
        lite::Serialreader slr( ( LPVOID )param.loadData );
        lite::Variant lvt = slr();
        if ( ( lvt.dataType != lite::Variant::VT_POINTER ) || ( lvt.dataSize != sizeof( SaveBlock_fixed ) ) )
            return false;

        const SaveBlock_fixed &block = *( const SaveBlock_fixed* )lvt._pointer;

        const SBuildingConfigData *buidlingData = GetBuildingData( block.index );
        if ( buidlingData == NULL )
            return false;

        memset( &property, 0, sizeof( property ) );

        property.index = block.index;
        reinterpret_cast< QWORD& >( property ) = reinterpret_cast< const QWORD& >( block );
        if ( block.ownerName[10] != 0 )
            return false;
        property.owner = block.ownerName;
        
		//property.inverse = ?
        //property.ownerType = block->ownerType;
        //memcpy( property.owner, block->ownerName, sizeof( property.owner ) );

        if ( slr.curSize() < slr.maxSize() )
        {
            lvt = slr();

            if ( ( lvt.dataType != lite::Variant::VT_RPC_OP ) || ( lvt._rpcop != 0x13579 ) )
                return false;

            // �Ƿ��ж������չ���ݣ�
            while ( slr.curSize() < slr.maxSize() )
            {
                // ѹ�����е���չ���ݣ�
                LPCSTR key = slr();

                // ���Ᵽ��һ���յ����ݣ�
                if ( slr.curSize() >= slr.maxSize() )
                    return false;

                // ע�⣬ʹ����׿�����������֮������ʧЧ������
                svarMap[ key ].DeepCopy( slr() );
            }
        }
    }

    return true;
}

void CBuilding::OnClose()
{
}

void CBuilding::OnRun()
{
}

// ����ֻ���Ƶ�ǰ
BOOL CBuilding::MoveBuilding( WORD regionId, WORD x, WORD y, BOOL bCheckBlk )
{
// 	if ( m_ParentRegion == NULL )
// 		return FALSE;
// 
//     const SBuildingConfigData *buidlingData = GetBuildingData( property.index );
//     if ( buidlingData == NULL )
//         return false;
// 
// 	CRegion *region = m_ParentRegion;
// 	//if ( m_ParentRegion->m_wMapID != regionId )
// 	//{
// 	//	region = ( CRegion * )FindRegionByID( regionId )->DynamicCast( IID_REGION );
// 	//	if ( region == NULL )
// 	//		return 0;
// 	//}
// 
// 	RelationBlock( FALSE );
// 
// 	WORD prevX = m_wCurX, prevY = m_wCurY;
// 	m_wCurX = x;
// 	m_wCurY = y;
// 
// 	// add by yuntao.liu
// 	mCurShowX = float(m_wCurX << TILE_BITW);
// 	mCurShowY = float(m_wCurY << TILE_BITH);
// 
// 
// 	if ( bCheckBlk && !CheckAreaBlock( NULL, region, FALSE ) )
//     {
//         rfalse( 2, 1, "CheckAreaBlock" );
// 		goto err_buildmove;
//     }
// 
// 	{
// 		SADelObjectMsg msg;
// 		msg.dwGlobalID = GetGID();
// 		if ( m_ParentArea ) 
// 			m_ParentArea->SendAdj( &msg, sizeof( msg ), -1 );
// 	}
// 
// 	// ȡ������������赲
//     if ( !region->Move2Area( self.lock(), x, y, true ) )
// 		goto err_buildmove;
// 
// 	if ( !RelationBlock( TRUE ) )
// 		goto err_buildmove;
// 
// 	if ( buidlingData->ctrlScript )
// 	{
//         g_Script.SetCondition( NULL, NULL, NULL, this );
// 		LuaFunctor( g_Script, FormatString( "Building_Remove_%d", buidlingData->ctrlScript ) ) [ prevX ] [ prevY ] ();
//         g_Script.CleanCondition();
// 	}
// 
// 	return TRUE;
// 
// err_buildmove:
// 	m_wCurX = prevX;
// 	m_wCurY = prevY;
// 
// 	// add by yuntao.liu
// 	mCurShowX = float(m_wCurX << TILE_BITW);
// 	mCurShowY = float(m_wCurY << TILE_BITH);
// 
// 	RelationBlock( TRUE );
	return FALSE;
}


int CBuilding::AfterCreate( CPlayer *pPlayer, BOOL isGenerate )
{
    if ( m_ParentRegion == NULL )
        return false;

    const SBuildingConfigData *buidlingData = GetBuildingData( property.index );
    if ( buidlingData == NULL )
        return false;

    // ���δ����������д����ű���
    if ( isGenerate && buidlingData->ctrlScript )
    {
		lite::Variant ret;
		// ��Ϊ���ɴ�����˵����ʹ���������ð��ɲ�����
		g_Script.SetCondition( NULL, pPlayer, NULL, this );
		LuaFunctor( g_Script, FormatString( "Building_CreateScript_%d", buidlingData->ctrlScript ) ) ( &ret );
        g_Script.CleanCondition();

		try
		{
			if ( ret.dataType != lite::Variant::VT_EMPTY && ret.dataType != lite::Variant::VT_NULL )
				if ( ( int )ret == -1 ) 
					return -1;
		}
		catch ( lite::Xcpt & )
		{
		}
    }

    // ����г�ʼ���ű�������ã���ʱֻ�������ð��ɳ�����ڣ�
    if ( buidlingData->initScript )
    {
		lite::Variant ret;
        g_Script.SetCondition( NULL, pPlayer, NULL, this );
		LuaFunctor( g_Script, FormatString( "Building_InitScript_%d", buidlingData->initScript ) ) ( &ret );
        g_Script.CleanCondition();

		try
		{
			if ( ret.dataType != lite::Variant::VT_EMPTY && ret.dataType != lite::Variant::VT_NULL )
				if ( ( int )ret == -1 ) 
					return -1;
		}
		catch ( lite::Xcpt & )
		{
		}
    }

    // �����赲��Ϣ
	RelationBlock( TRUE );
	//if ( buidlingData->qwtileMask || buidlingData->dwtileLimit )
	//{
	//	POINT blkpos = { m_wCurX  - ( property.inverse ? ( 8 - buidlingData->centerx  ) : buidlingData->centerx ), 
	//		m_wCurY - buidlingData->centery };

	//	for ( int row = 0; row < 8; row ++ )
	//	{
	//		for ( int col = 0; col < 8; col ++ )
	//		{		
	//			int tx = blkpos.x + col;
	//			int ty = blkpos.y + row;

	//			if ( ( tx >= m_ParentRegion->m_RegionW ) || 
	//				( ty >= m_ParentRegion->m_RegionH ) ) 
	//				return false;

	//			if ( ( ( LPBYTE )&buidlingData->dwtileLimit )[ row ] & ( 1 << ( property.inverse ? 7 - col : col ) ) )
	//			{
	//				// ��������
	//				DWORD tiles = m_ParentRegion->m_pdwTiles[ tx + ( ty * m_ParentRegion->m_RegionW ) ];
	//				tiles |= TILETYPE_LIMIT;
	//				m_ParentRegion->m_pdwTiles[ tx + ( ty * m_ParentRegion->m_RegionW ) ] = tiles;
	//			}
	//			if ( ( ( LPBYTE )&buidlingData->dwtileMask )[ row ] & ( 1 << ( property.inverse ? 7 - col : col ) ) )
	//			{
	//				// �赲
	//				DWORD tiles = m_ParentRegion->m_pdwTiles[ tx + ( ty * m_ParentRegion->m_RegionW ) ] ;
	//				tiles &= 0xfffffffe;
	//				m_ParentRegion->m_pdwTiles[ tx + ( ty * m_ParentRegion->m_RegionW ) ] = tiles;
	//			}
	//		}
	//	}
	//}

	return true;
}

BOOL CBuilding::RelationBlock( BOOL relation )
{
// 	const SBuildingConfigData *buidlingData = GetBuildingData( property.index );
// 	if ( buidlingData == NULL )
// 		return false;
// 
// 	if ( buidlingData->qwtileMask || buidlingData->dwtileLimit )
// 	{
// 		if ( m_ParentRegion == NULL )
// 			return false;
// 
// 		POINT blkpos = { m_wCurX  - ( property.inverse ? ( 8 - buidlingData->centerx  ) : buidlingData->centerx ), 
// 			m_wCurY - buidlingData->centery };
// 
// 		if ( ( ( blkpos.x + 7 ) >= m_ParentRegion->m_RegionW ) || ( ( blkpos.y + 7 ) >= m_ParentRegion->m_RegionH ) ||
// 			( blkpos.x < 0 ) || ( blkpos.y < 0 ) ) 
// 			return false;
// 
// 		for ( int row = 0; row < 8; row ++ )
// 		{
// 			int ty = blkpos.y + row;
// 			for ( int col = 0; col < 8; col ++ )
// 			{		
// 				int tx = blkpos.x + col;
// 				if ( ( ( LPBYTE )&buidlingData->dwtileLimit )[ row ] & ( 1 << ( property.inverse ? 7 - col : col ) ) )
// 				{
// 					// ��������
// 					DWORD tiles = m_ParentRegion->m_pdwTiles[ tx + ( ty * m_ParentRegion->m_RegionW ) ];
// 
// 					if ( !relation )
// 						tiles &= 0xfffffffb;
// 					else
// 						tiles |= TILETYPE_LIMIT;
// 
// 					m_ParentRegion->m_pdwTiles[ tx + ( ty * m_ParentRegion->m_RegionW ) ] = tiles;
// 				}
// 				if ( ( ( LPBYTE )&buidlingData->dwtileMask )[ row ] & ( 1 << ( property.inverse ? 7 - col : col ) ) )
// 				{
// 					// �赲
// 					DWORD tiles = m_ParentRegion->m_pdwTiles[ tx + ( ty * m_ParentRegion->m_RegionW ) ];
// 
// 					if ( !relation )
// 						tiles |= 0x1;
// 					else
// 						tiles &= 0xfffffffe;
// 
// 					m_ParentRegion->m_pdwTiles[ tx + ( ty * m_ParentRegion->m_RegionW ) ] = tiles;
// 				}
// 			}
// 		}
// 	}

	return TRUE;
}

int CBuilding::DeforeRemove()
{
	const SBuildingConfigData *buidlingData = GetBuildingData( property.index );
    if ( buidlingData == NULL )
        return false;

	// ����赲ʧ��
	if ( !RelationBlock( FALSE ) )
		return false;

	//if ( buidlingData->qwtileMask || buidlingData->dwtileLimit )
	//{
	//	if ( m_ParentRegion == NULL )
	//		return false;

	//	POINT blkpos = { m_wCurX  - ( property.inverse ? ( 8 - buidlingData->centerx  ) : buidlingData->centerx ), 
	//		m_wCurY - buidlingData->centery };

	//	for ( int row = 0; row < 8; row ++ )
	//	{
	//		for ( int col = 0; col < 8; col ++ )
	//		{		
	//			int tx = blkpos.x + col;
	//			int ty = blkpos.y + row;

	//			if ( ( tx >= m_ParentRegion->m_RegionW ) || 
	//				( ty >= m_ParentRegion->m_RegionH ) ) 
	//				return false;

	//			if ( ( ( LPBYTE )&buidlingData->dwtileLimit )[ row ] & ( 1 << ( property.inverse ? 7 - col : col ) ) )
	//			{
	//				// ��������
	//				DWORD tiles = m_ParentRegion->m_pdwTiles[ tx + ( ty * m_ParentRegion->m_RegionW ) ];
	//				tiles &= 0xfffffffb;
	//				m_ParentRegion->m_pdwTiles[ tx + ( ty * m_ParentRegion->m_RegionW ) ] = tiles;
	//			}
	//			if ( ( ( LPBYTE )&buidlingData->dwtileMask )[ row ] & ( 1 << ( property.inverse ? 7 - col : col ) ) )
	//			{
	//				// �赲
	//				DWORD tiles = m_ParentRegion->m_pdwTiles[ tx + ( ty * m_ParentRegion->m_RegionW ) ] ;
	//				tiles |= 0x1;
	//				m_ParentRegion->m_pdwTiles[ tx + ( ty * m_ParentRegion->m_RegionW ) ] = tiles;
	//			}
	//		}
	//	}
	//}

	if ( buidlingData->ctrlScript )
	{
        g_Script.SetCondition( NULL, NULL, NULL, this );
		LuaFunctor( g_Script, FormatString( "Building_Remove_%d", buidlingData->ctrlScript ) ) ();
        g_Script.CleanCondition();
	}
    return 0;
}

BOOL CBuilding::GetInteger( LPCSTR key, int &value )
{
    std::map< std::string, lite::Variant >::iterator it = svarMap.find( key );
    if ( it == svarMap.end() )
        return FALSE;

    try
    {
        value = it->second;
        return TRUE;
    }
    catch ( lite::InvalidConvert & )
    {
        return FALSE;
    }
}

LPCSTR CBuilding::GetString( LPCSTR key, size_t *size )
{
    std::map< std::string, lite::Variant >::iterator it = svarMap.find( key );
    if ( it == svarMap.end() )
        return NULL;

    if ( it->second.dataType != lite::Variant::VT_STRING )
        return FALSE;
    
    if ( size != NULL )
        *size = it->second.dataSize;

    return it->second._string;
}

BOOL CBuilding::GetStateMsg( void *data, WORD &size )
{
//     if ( size < sizeof( SASynBuildingMsg ) )
//         return size = 0, FALSE;
// 
//     const SBuildingConfigData *buidlingData = GetBuildingData( property.index );
//     if ( buidlingData == NULL )
//         return false;
// 
// #undef new 
//     //property.ownerType = param.ownerType;
//     SASynBuildingMsg &msg = * new ( data ) SASynBuildingMsg;
// 
//     msg.gid = GetGID();
// 
// 	WLINE line = buidlingData->line;
//     lite::Serializer slm( msg.append, size - sizeof( SASynBuildingMsg ) );
// 
//     // ����ʱͬ����Ҫ�����ݹ�ȥ������û�з��ͽ��������ñ�ţ�ֻ����ʾ�����ݣ��������������չ���޸�
//     // ״̬ + ΨһID���赲�����꣬��ۣ����ƣ�
//     slm ( reinterpret_cast< QWORD& >( property ) )
//         ( buidlingData->qwtileMask )
//         ( ( DWORD )MAKELONG( m_wCurX, m_wCurY ) )
//         ( buidlingData->image )
// 		( ( DWORD )MAKELONG( checkId, buidlingData->index ) )
// 		( MAKEWORD( buidlingData->centerx, buidlingData->centery ) )
// 		( &line, sizeof( buidlingData->line ) )
//         ( property.viewTitle.c_str() )
//         ( property.owner.c_str() );
// 
//     // ������Ҫ���͵����ݴ�С��
//     size = ( WORD )( size - slm.EndEdition() );

    return TRUE;
}

size_t CBuilding::OnSave( LPVOID pBuf, size_t bufsize )
{
//     if ( m_ParentRegion == NULL )
//         return 0;
// 
//     SaveBlock_fixed block;
//     ZeroMemory( &block, sizeof( block ) );
//     reinterpret_cast< QWORD& >( block ) = reinterpret_cast< QWORD& >( property );
//     block.index = property.index;
//     block.parentId = m_ParentRegion->m_wRegionID;
//     block.x = m_wCurX;
//     block.y = m_wCurY;
//     dwt::strcpy( block.ownerName, property.owner.c_str(), sizeof( block.ownerName ) );
// 
//     // HP���ű�����������������ȫΪ��̬���ݣ����� svarMap ���ں�߱���ʱѹ������
// 	lite::Serializer slm( pBuf, bufsize );
//     slm ( &block, sizeof( block ) ) [ 0x13579 ]; // �����չ������ʼ���
// 
//     // ѹ�����е���չ���ݣ�
//     for ( std::map< std::string, lite::Variant >::iterator it = svarMap.begin(); it != svarMap.end(); it ++ )
//         slm( it->first.c_str() )( it->second );
// 
//     return bufsize - slm.EndEdition();

	return 0;
}

BOOL CBuilding::CheckAreaBlock( CPlayer *pPlayer, CRegion *region, BOOL checkLmt, BOOL checkMask )
{
//     if ( region == NULL )
//         return false;
// 
//     const SBuildingConfigData *buidlingData = GetBuildingData( property.index );
//     if ( buidlingData == NULL )
//         return false;
// 
//     POINT blkpos = { m_wCurX  - ( property.inverse ? ( 8 - buidlingData->centerx  ) : buidlingData->centerx ), 
//         m_wCurY - buidlingData->centery };
// 
//     for ( int row = 0; row < 8; row ++ )
//     {
//         for ( int col = 0; col < 8; col ++ )
//         {
//             int tx = blkpos.x + col;
//             int ty = blkpos.y + row;
// 
//             if ( tx >= region->m_RegionW || ty >= region->m_RegionH )
//                 return false;
// 
//             if ( checkLmt )
//             {
//                 if ( ( ( LPBYTE )&buidlingData->dwtileLimit )[ row ] & ( 1 << ( property.inverse ? 7 - col : col ) ) )
//                 {
//                     if ( !region->isTileWalkAble( tx, ty ) ||
//                         ( region->m_pdwTiles[ tx + ty * region->m_RegionW ] & TILETYPE_LIMIT ) )
//                     {
//                         if ( pPlayer ) TalkToDnid( pPlayer->m_ClientIndex, "[ϵͳ]:���ý���ʧ��!" );
//                         //return rfalse( 2, 1, "[checkLmt]���ý���ʱ isTileWalkAble û��ͨ��" );
//                         return rfalse( 2, 1, "���ý���ʱ_A isTileWalkAble û��ͨ��[%I64u][%s][%d][%d]", property.uniqueId(), property.owner, property.curX, property.curY );
//                     }
//                 }
//             }
// 
//             if ( ( ( LPBYTE )&buidlingData->dwtileMask )[ row ] & ( 1 << ( property.inverse ? 7 - col : col ) ) )
//             {
//                 // ֻ������Ҵ�����ʱ���
//                 if ( checkMask && ( !region->isTileStopAble( tx, ty, 0 ) ) )
//                 {
//                     if ( pPlayer ) 
//                         TalkToDnid( pPlayer->m_ClientIndex, "[ϵͳ]:���ý���ʧ��!" );
//                     return rfalse( 2, 1, "[checkMask]���ý���ʱ isTileStopAble û��ͨ��" );
//                 }
// 
//                 if ( !region->isTileWalkAble( tx, ty ) ||
//                     ( checkMask && ( region->m_pdwTiles[ tx + ty * region->m_RegionW ] & TILETYPE_LIMIT ) ) )
//                 {
//                     if ( pPlayer ) 
//                         TalkToDnid( pPlayer->m_ClientIndex, "[ϵͳ]:���ý���ʧ��!" );
//                     return rfalse( 2, 1, "���ý���ʱ isTileWalkAble û��ͨ��[%I64u][%s][%d][%d]", property.uniqueId(), property.owner, property.curX, property.curY );
//                 }
//             }
//         }
//     }
    return true;
}

BOOL CBuilding::MoveBuilding( WORD curregionId, QWORD uniqueId, WORD regionId, WORD x, WORD y, BOOL checkBlk )
{
	if ( uniqueId == 0 || curregionId == 0 )
		return FALSE;

	CRegion *region = ( CRegion * )FindRegionByID( curregionId )->DynamicCast( IID_REGION );
	if ( region == NULL )
		return FALSE;

	std::map< QWORD, LPIObject >::iterator it = region->m_BuildingMap.find( uniqueId );
	if ( it == region->m_BuildingMap.end() )
		return FALSE;

	CBuilding *building = ( CBuilding* )it->second->DynamicCast( IID_BUILDING );
	if ( building == NULL )
		return FALSE;

	if ( !building->MoveBuilding( regionId, x, y, checkBlk ) )
		return FALSE;

	return TRUE;
}

BOOL CBuilding::MoveBuilding( LPCSTR name, WORD regionId, WORD x, WORD y,  BOOL checkBlk )
{
	if ( name == NULL )
		return FALSE;

	SFactionData::SFaction faction;
// 	if ( !GetFactionHeaderInfo( name, faction ) )
// 		return FALSE;

	if ( faction.buildingStub.uniqueId == 0 || faction.buildingStub.parentId == 0 )
		return FALSE;

	return MoveBuilding( faction.buildingStub.parentId, faction.buildingStub.uniqueId, regionId, x, y, checkBlk );
}
