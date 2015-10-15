#include "stdafx.h"
#include <time.h>
#include "netdbmoduleex.h"
#include "sqltextdefine.h"
#include "directnet/lzw.hpp"
#include "liteserializer/lite.h"

extern BOOL SendMessage( DNID dnidClient, LPCVOID lpMsg, size_t wSize );

__declspec( thread ) BYTE tmpBuffer[ 0x1000000 ];
__declspec( thread ) BYTE tmpBufEx[ 0x100000 ];
__declspec( thread ) char msgBuffer[ sizeof( SAGetFactionDataMsg ) ];
__declspec( thread ) char msgBuffer2[ sizeof( SAGetUnionDataMsg ) ];

extern "C" DWORD _GetCrc32( LPVOID buf, DWORD size );
extern void AddInfo(LPCSTR Info);

static time_t timeSegment = 0;

static BOOL timeAccumulatorInit()
{
    tm timeMaker; 
    ZeroMemory( &timeMaker, sizeof( tm ) ); 
    timeMaker.tm_year = SItemBase::YEAR_SEGMENT - 1900;
    timeSegment = mktime( &timeMaker );
    return TRUE;
}

static BOOL timeAccumulatorInited = timeAccumulatorInit();

size_t LzwEncoding( const void * src_data, size_t src_size, void * dst_data, size_t dst_size )
{
    lzw::lite_lzwEncoder lencoder;
	lencoder.SetEncoderData( dst_data, dst_size );
    return lencoder.lzw_encode( ( void* )src_data, src_size ) ? lencoder.size() : 0;
}

BOOL TryEncoding( const void *src_data, size_t src_size, void *&dst_data, size_t &dst_size )
{
    // 数据过小，不做压缩处理
    if ( src_size < 32 )
    {
        dst_size = src_size;
        dst_data = ( void* )src_data;
        return FALSE;
    }

	dst_size = LzwEncoding( src_data, src_size, ( char* )dst_data + 8, dst_size - 8 );
	if ( dst_size <= 0 || dst_size >= ( src_size - 16 ) )
    {
        // 压缩失败（包括压缩后数据量不变或变大），直接返回原始数据
        dst_data = ( void* )src_data;
        dst_size = src_size;
        return FALSE;
	}

    // 压缩成功，添加压缩标记！
    * reinterpret_cast< QWORD* >( dst_data ) = 0xefcdab9036587214;
    dst_size += 8;
    return TRUE;
}

size_t LzwDecoding( const void * src_data, size_t src_size, void * dst_data, size_t dst_size )
{
    lzw::lzwDecoder ldecoder;
    ldecoder.SetEncoderData( ( void* )src_data, src_size );
    return ldecoder.lzw_decode( dst_data, dst_size );
}

BOOL TryDecoding( const void * src_data, size_t src_size, void *&dst_data, size_t &dst_size )
{
    if ( *reinterpret_cast< const QWORD* >( src_data ) == 0xefcdab9036587214 )
    {
        dst_size = LzwDecoding( reinterpret_cast< const char* >( src_data ) + 8, src_size - 8, dst_data, dst_size );
        return TRUE;
    }

    // 不需要解压的情况下
    dst_data = ( void* )src_data;
    dst_size = src_size;
    return FALSE;
}

BOOL TryDecoding_NilBuffer( const void * src_data, size_t src_size, void *&dst_data, size_t &dst_size )
{
    assert( dst_data == NULL && dst_size == 0 );
    dst_data = tmpBuffer;
    dst_size = sizeof( tmpBuffer );
    return TryDecoding( src_data, src_size, dst_data, dst_size );
}

int CNetDBModuleEx::GetPlayerCharList(LPCSTR pAccount, DWORD dwServerID, SCharListData *pChartListData)
{
	rfalse("获取角色列表");
    assert(pAccount && dwServerID && pChartListData);
    assert(( pChartListData[0].m_dwStaticID | pChartListData[1].m_dwStaticID | pChartListData[2].m_dwStaticID | pChartListData[3].m_dwStaticID | pChartListData[4].m_dwStaticID ) == 0);

    char sql[1024];

    // 这个函数可能返回 -3、-2、-1、0、1，而只有 -3 时，是被其他服务器锁定
    int result = UpdateAccountLock(pAccount, dwServerID, TRUE);
    if (result == -1) return SAGetCharacterListMsg::RET_ERROR_INFO;
    if (result == -3) return SAGetCharacterListMsg::RET_LOCKEDOTHER;
	
	//检查账号是否被冻结，由于现在的账号服务器没有调通，所以暂时用错误的做法，在数据库管理这里检测账号的有效性
	_snprintf(sql, sizeof sql, CHECK_ACCOUNT_FROZEN, pAccount);
	sql[sizeof(sql) - 1] = 0;
	
	QueryResult qr;
	// 如果执行失败，那么返回-1
	if (ExecuteStmt(sql, strlen(sql), &qr) != 0)
		return -1;
	// 如果没有影响行数，则表明没有对应的账号数据，应该返回-2
	if (qr.AffectedRows() != 0)
	{
		// 获取角色锁定数据，用于检测账号是否被冻结
		unsigned long sid = 0;
		QueryBind<1 > bind;
		bind[0].BindULong(&sid);
		if (qr.FetchResult(bind) != 0)
			return -1;
		if (sid > 0)
			return SAGetCharacterListMsg::RET_ERROR_INFO;
	}
	//if (result == -2) //2014/11/21 目前满足外测版本不能创建账号的需求，WareHouse账号数据手动创建，所以若无该数据则直接返回
	//	return SAGetCharacterListMsg::RET_ERROR_INFO;


	if (result == -2)
	{
		// 由于是新建角色，所以必然没有数据，直接返回结果
		_snprintf(sql, sizeof sql, INSERT_WAREHOUSE, pAccount, dwServerID, pAccount);
		sql[sizeof(sql) - 1] = 0;
		if (ExecuteSql(sql, strlen(sql), &qr) != 0 || qr.AffectedRows() != 1)
			return SAGetCharacterMsg::ERC_FAIL;
		SFixData PlayerData;
		memset(&PlayerData, 0, sizeof(PlayerData));
		PlayerData.m_version = SFixData::GetVersion();
		if (CreatePlayerData(pAccount, 1, dwServerID, PlayerData) != 1) // 返回1表示策创建角色属性数据成功
			return SAGetCharacterMsg::ERC_FAIL;

		// 修改登录时间
		_snprintf(sql, sizeof sql, UPDATE_PROPERTY_LOGINTIME, PlayerData.m_dwStaticID, pAccount, dwServerID);
		qr.Reset();
		if (ExecuteSql(sql, strlen(sql), &qr) != 0 || qr.AffectedRows() != 1)
			m_errmsg.ShowMsg(EDEBUG, "更新角色上线时间失败%s,%d ", mysql_stmt_error(m_pstmt), PlayerData.m_dwStaticID);

		return SAGetCharacterMsg::ERC_SUCCESS;
	}

    // 此时数据库中还没有这个帐号，按照规则，需要用户在成功获得数据的同时，做出锁定，现在没有，就需要新建一个
    //QueryResult qr;
    //if ( result == -2 ) 
    //{
    //    // 由于是新建角色，所以必然没有数据，直接返回结果
    //    _snprintf( sql, sizeof sql, INSERT_WAREHOUSE, pAccount, dwServerID, pAccount );
    //    sql[ sizeof( sql ) - 1 ] = 0;
    //    if ( ExecuteSql( sql, strlen( sql ), &qr ) != 0 || qr.AffectedRows() != 1 )
    //        return SAGetCharacterListMsg::RET_ERROR_INFO;
	//
    //    return SAGetCharacterListMsg::RET_SUCCESS;
    //}
	//
    //// 已经成功完成服务器锁定，正式开始获取角色列表
    //_snprintf(sql, sizeof sql, SELECT_PROPERTY_ROLELIST, pAccount, pAccount, dwServerID);
    //sql[sizeof(sql) - 1 ] = 0;
    //if ( ExecuteStmt( sql, strlen( sql ), &qr ) != 0 || qr.AffectedRows() < 0)
    //    return SAGetCharacterListMsg::RET_ERROR_INFO;
	//
    //assert(qr.AffectedRows() <= 5);
	//
    //// 如果一个角色都没有，那么后边的取数据就不需要了，但是需要在后边释放资源
    //if ( qr.AffectedRows() != 0 )
    //{
    //    SCharListData tmpbuf;
    //    long reolindex = 0;
	//
    //    Query::QueryBind< 2 > bind;
    //    bind[0].BindBlob( &tmpbuf, sizeof( tmpbuf ) );
    //    bind[1].BindLong( &reolindex );
	//
    //    while ( qr.FetchResult( bind ) == 0 )
    //    {
    //        // 特殊处理，数据错误（为空）；或索引号大于等于5；或已经读取过数据； 不再读入
    //        if ( bind[0].IsNull() || reolindex > 5 || reolindex <= 0 || pChartListData[ reolindex - 1 ].m_dwStaticID != 0 )
    //            continue;
	//
    //        pChartListData[ reolindex - 1 ] = tmpbuf;
    //        pChartListData[ reolindex - 1 ].m_byIndex = (BYTE)reolindex;
    //    }
    //}

    return SAGetCharacterListMsg::RET_SUCCESS;
}

LPSTR g_Utf8ToAnsi(const char* Utf8Str)
{
	if (Utf8Str == NULL)
		return NULL;
	wchar_t* pBuf = NULL;
	int WriteByte = 0;
	WriteByte = ::MultiByteToWideChar(CP_UTF8, 0, Utf8Str, -1, NULL, 0);
	pBuf = new wchar_t[WriteByte + 1];
	memset(pBuf, 0, (WriteByte + 1)*sizeof(wchar_t));
	::MultiByteToWideChar(CP_UTF8, 0, Utf8Str, -1, (LPWSTR)pBuf, WriteByte + 1);

	char* AnsiStr = NULL;
	int AnsiStrByte = 0;
	AnsiStrByte = ::WideCharToMultiByte(CP_ACP, 0, pBuf, -1, NULL, 0, 0, 0);
	AnsiStr = new char[AnsiStrByte + 1];
	::WideCharToMultiByte(CP_ACP, 0, pBuf, WriteByte + 1, AnsiStr, AnsiStrByte + 1, 0, 0);
	if (pBuf != NULL)
		delete[] pBuf;
	return AnsiStr;
}

// 返回创建成功后的角色在数据库上的唯一ID号
int CNetDBModuleEx::CreateName(LPCSTR pAccount, WORD wIndex, DWORD dwServerID, LPCSTR pRoleName)
{
    assert((pAccount != NULL) && (pRoleName != NULL));

    if (wIndex <= 0 || wIndex > 5)
        return 0;
	char *AnsiRoleName = g_Utf8ToAnsi(pRoleName);
    // 检测账号是否存在，如果不存在就需要新建一个
    // 现在逻辑修改了，改为一开始获取角色列表时，如果没有账号则会自动创建一个

    // 检测 serverId 的同时，创建新角色
    char sql[1024];
	string roleName(pRoleName);
	
	//20150619 wk 截取前31个当默认名字
	//roleName.append("name");
	roleName = roleName.substr(0, CONST_USERNAME-1);
	
	_snprintf(sql, sizeof sql, INSERT_PROPERTY_ONLYNAME, pAccount, roleName.c_str(),
		wIndex, pAccount, dwServerID, pRoleName, pAccount, wIndex);
    sql[sizeof( sql ) - 1] = 0;

    QueryResult qr;

	if (ExecuteSql(sql, strlen(sql), &qr) != 0 || qr.AffectedRows() != 1)
	{
		if (AnsiRoleName != NULL)
			delete[] AnsiRoleName;
		return 0;
	}

    // 角色插入完成，读取角色的ID
	_snprintf(sql, sizeof sql, SELECT_PROPERTY_ROLEID, roleName.c_str());
	if (AnsiRoleName != NULL)
		delete[] AnsiRoleName;
    sql[sizeof( sql ) - 1] = 0;
    qr.Reset();

    if (ExecuteStmt(sql, strlen( sql ), &qr ) != 0 || qr.AffectedRows() != 1)
        return 0;

    __int64 roleId = 0;
    QueryBind<1> bind;
    bind[0].BindLongLong(&roleId);
    if (qr.FetchResult( bind ) != 0)
        return 0;

    return (int)roleId;
}

// 这里返回0是末知错误
int CNetDBModuleEx::CreatePlayerData(LPCSTR pAccount,WORD wIndex, DWORD dwServerID, SFixData &PlayerData)
{
    if (wIndex < 1 || wIndex > 5)
        return 0;

    // 创建角色名字，保存StaticID
	int staticid = CreateName(pAccount, wIndex, dwServerID, pAccount);
    if (staticid == 0)
        return SACreatePlayerMsg::ERC_NAMETAKEN;

    PlayerData.m_dwStaticID = staticid;
	PlayerData.m_bNewPlayer = true;
	PlayerData.m_byStoreFlag = SFixData::ST_LOGIN;
	PlayerData.m_EnduranceData.m_dwEndurance = 90;
	PlayerData.m_EnduranceData.m_dwRemainingSeconds = 0;
	PlayerData.m_dwLevel = 1;
	
	///-----------新建武将数据初始化
	SHeroData tempHeroData;
	memset(&tempHeroData, 0, sizeof(SHeroData));
	//装备初始化
	SHeroEquipment tempEquip;
	//技能初始化
	memset(&tempEquip, 0, sizeof(SHeroEquipment));
	SHeroSkillInfo tempSkill;
	memset(&tempSkill, 0, sizeof(SHeroSkillInfo));
	for (int i = 0; i < EQUIP_MAX; i++)
		memcpy(&tempHeroData.m_Equipments[i], &tempEquip, sizeof(SHeroEquipment));
	for (int i = 0; i < MAX_SKILLNUM; i++)
		memcpy(&tempHeroData.m_SkillInfoList[i], &tempSkill, sizeof(SHeroSkillInfo));
	//新建武将属性初始化
	for (int i = 0; i < MAX_HERO_NUM; i++)
		memcpy(&PlayerData.m_HeroList[i], &tempHeroData, sizeof(SHeroData));

	///------------新建的背包初始化
	memset(&PlayerData.m_ItemList, 0, sizeof(SSanguoItem) * MAX_ITEM_NUM);

	///------------祈福数据初始化
	PlayerData.m_BlessData.m_dwMoneyFreeBlessCount = 5;
	PlayerData.m_BlessData.m_dwDiamondFreeBlessCount = 1;
	PlayerData.m_BlessData.m_dwLastMoneyFreeBlessDate = time(nullptr) - 600000;
	PlayerData.m_BlessData.m_dwLastDiamondFreeBlessDate = time(nullptr)  - 17280000 * 10;
	PlayerData.m_BlessData.m_dwIsFirstDiamondBless = 1;
	PlayerData.m_BlessData.m_dwIsFirstMoneyBless = 1;
	// 创建角色时，不应该动到仓库
    return SavePlayerData(pAccount, dwServerID, PlayerData, NULL); 
	
}


/*
	初始化侠义世界的游戏数据,
	此处子类转父类，C++会自动
	调整指针偏移
*/
// static void InitXiaYiShiJieData(SPlayerXiaYiShiJie* data)
// {
// 	memset(data,0,sizeof(SPlayerXiaYiShiJie));
// 	for (int i = 0;i < SPlayerXiaYiShiJie::ATTACK_DEFENSE_MAX;++i)
// 	{
// 		// 测试代码。目前都暂时初始化为0
// 		//data->m_AttackAndDefense[ i ].m_wLevel = 0;
// 		//data->m_AttackAndDefense[ i ].m_wMastery = 0;
// 	}
// }

/*
	版本3到版本4的转换函数
		版本4的侠义世界的版本
*/
static bool ConverVersion3ToVersion4(SFixPlayerDataBuf* fpdb)
{
//	InitXiaYiShiJieData(fpdb);
	fpdb->m_version = SFixPlayerDataBuf::GetVersion();
	return true;
}

// 读取玩家的角色数据信息
int CNetDBModuleEx::GetPlayerData(LPCSTR pAccount, DWORD dwServerID, BYTE byIndex, SFixData &PlayerData)
{
	rfalse("获取角色数据");
    if (!pAccount || byIndex <= 0 || byIndex > 5)
        return SAGetCharacterMsg::ERC_FAIL;
	char sql[1024];
	QueryResult qr;
	
	// 这个函数可能返回 -3、-2、-1、0、1，而只有 -3 时，是被其他服务器锁定
	int result = UpdateAccountLock(pAccount, dwServerID, TRUE);
	if (result == -1) return SAGetCharacterMsg::ERC_FAIL;
	if (result == -3) return SAGetCharacterMsg::ERC_LOCKEDOTHER;
	// 此时数据库中还没有这个帐号，按照规则，需要用户在成功获得数据的同时，做出锁定，现在没有，就需要新建一个
	//if (result == -2)
	//{
	//	// 由于是新建角色，所以必然没有数据，直接返回结果
	//	_snprintf(sql, sizeof sql, INSERT_WAREHOUSE, pAccount, dwServerID, pAccount);
	//	sql[sizeof(sql) - 1] = 0;
	//	if (ExecuteSql(sql, strlen(sql), &qr) != 0 || qr.AffectedRows() != 1)
	//		return SAGetCharacterMsg::ERC_FAIL;
	//	PlayerData.m_version = SFixData::GetVersion();
	//	if (CreatePlayerData(pAccount, byIndex, dwServerID, PlayerData) != 1) // 返回1表示策创建角色属性数据成功
	//		return SAGetCharacterMsg::ERC_FAIL;
	//
	//	// 修改登录时间
	//	_snprintf(sql, sizeof sql, UPDATE_PROPERTY_LOGINTIME, PlayerData.m_dwStaticID, pAccount, dwServerID);
	//	qr.Reset();
	//	if (ExecuteSql(sql, strlen(sql), &qr) != 0 || qr.AffectedRows() != 1)
	//		m_errmsg.ShowMsg(EDEBUG, "更新角色上线时间失败%s,%d ", mysql_stmt_error(m_pstmt), PlayerData.m_dwStaticID);
	//
	//	return SAGetCharacterMsg::ERC_SUCCESS;
	//}
	/*
	//2014/11/21 目前满足外测版本不能创建账号的需求, 这里如果没有PROPERTY存在则直接创建
	_snprintf(sql, sizeof sql, CHECK_PROPERTY_EXIST, pAccount);
	sql[sizeof(sql) - 1] = 0;
	if (ExecuteStmt(sql, strlen(sql), &qr) != 0 || qr.AffectedRows() > 1)
		return SAGetCharacterMsg::ERC_FAIL;
	if (qr.AffectedRows() == 0) //没有property直接创建
	{
		PlayerData.m_version = SFixData::GetVersion();
		if (CreatePlayerData(pAccount, byIndex, dwServerID, PlayerData) != 1) // 返回1表示策创建角色属性数据成功
			return SAGetCharacterMsg::ERC_FAIL;
	}
	*/
    _snprintf( sql, sizeof sql, SELECT_ALL_PROPERTY_DATA, pAccount, byIndex, pAccount, dwServerID );
    sql[ sizeof( sql ) - 1 ] = 0;
    if ( ExecuteStmt( sql, strlen( sql ), &qr ) != 0 || qr.AffectedRows() != 1 )
        return SAGetCharacterMsg::ERC_FAIL;

	SFixData databuf;
    unsigned long sid = 0;
	char name[CONST_USERNAME];
    DWORD verify = 0;
	name[CONST_USERNAME -1] = 0;

    QueryBind< 4 > bind;
    bind[0].BindULong( &sid );
    bind[1].BindString( name, sizeof( name ) - 1 );
    bind[2].BindBlob( &databuf, sizeof( databuf ) );
    bind[3].BindULong( &verify );

    if ( qr.FetchResult( bind ) != 0 || bind[2].IsNull() )
        return SAGetCharacterMsg::ERC_FAIL;

    assert( bind[2].Length() <= sizeof( SFixPlayerDataBuf ) );

    DWORD crc = _GetCrc32( &databuf, ( DWORD )bind[2].Length() );
    if ( crc != verify ) {
        TraceInfo_C( "modify.txt", "玩家数据被修改！[acc = %s][name = %s][verify = %d]", pAccount, name, verify );
        if ( verify != 0 )
            return SAGetCharacterMsg::ERC_FAIL;
    }

    void *unpakData = tmpBuffer;
    size_t unpakSize = sizeof( tmpBuffer );

    TryDecoding( &databuf, bind[2].Length(), unpakData, unpakSize );

    // 利用自身的数据检测数据的有效性!
	SFixData *fpdb = (SFixData*)unpakData;
//    if ( unpakSize != ( fpdb->m_version >> 16 ) )
  //  {
    //    OutputDebugString( "解压后的数据大小不匹配！\r\n" );
      //  return SAGetCharacterMsg::ERC_FAIL;
    //}

    // 检测是否获取到的是未经解压处理的原始数据，并作处理，避免在版本升级时处理异常！
    if ( unpakData != tmpBuffer )
    {
        memcpy( tmpBuffer, unpakData, unpakSize );
        unpakData = tmpBuffer;
    }

    // 以下为版本升级逻辑（侠义道3暂无玩家数据版本升级）
	
	/*
    // 版本1升级到版本2！！！
    if ( fpdb->m_version == typedef_v1::SFixPlayerDataBuf::GetVersion() )
    {
        typedef_v1::SFixPlayerDataBuf *ptr = ( typedef_v1::SFixPlayerDataBuf* )tmpBuffer;
        typedef_v2::SFixPlayerDataBuf *dest = ( typedef_v2::SFixPlayerDataBuf* )tmpBufEx;

        // 检测版本，并将旧版本升级到新版本
        // 通过常量数据比对，默认该段逻辑不应该产生代码！
        static const int checkSize = offsetof( typedef_v1::SFixProperty, typedef_v1::SFixProperty::m_dwStaticID );
        static const int checkTail = sizeof( typedef_v1::SFixProperty ) - checkSize;
        assert( checkSize == offsetof( typedef_v2::SFixProperty, typedef_v2::SPlayerDataExpand::masterName ) );
        assert( checkTail == sizeof( typedef_v2::SFixProperty ) - offsetof( typedef_v2::SFixProperty, typedef_v2::SFixProperty::m_dwStaticID ) );

        // 默认数据清零
        memset( tmpBufEx, 0, sizeof( tmpBufEx ) );

        // 直接拷贝前一段数据
        memcpy( tmpBufEx, tmpBuffer, checkSize ); 

        // 后面的3个关键数据拷贝回来。
        dest->m_dwStaticID = ptr->m_dwStaticID;
        dest->m_byStoreFlag = ptr->m_byStoreFlag;
        memcpy( dest->m_szUPassword, ptr->m_szUPassword, sizeof( dest->m_szUPassword ) );

        // 重设版本号！
        dest->m_version = dest->GetVersion();

        // 拷贝回tmpBuffer！
        memcpy( tmpBuffer, tmpBufEx, sizeof( tmpBuffer ) );
    }

    // 版本2升级到版本3！！！
    if ( fpdb->m_version == typedef_v2::SFixPlayerDataBuf::GetVersion() )
    {
        typedef_v2::SFixPlayerDataBuf *ptr = ( typedef_v2::SFixPlayerDataBuf* )tmpBuffer;
        typedef_v3::SFixPlayerDataBuf *dest = ( typedef_v3::SFixPlayerDataBuf* )tmpBufEx;

        // 检测版本，并将旧版本升级到新版本
        // 通过常量数据比对，默认该段逻辑不应该产生代码！
        static const int checkHead = offsetof( typedef_v2::SFixProperty, typedef_v2::SPlayerGM::m_wGMLevel );
        static const int checkTail = sizeof( typedef_v2::SFixProperty ) - checkHead;
        static const int checkBody = offsetof( typedef_v3::SFixProperty, typedef_v3::SPlayerGM::m_wGMLevel );
        assert( checkHead == offsetof( typedef_v3::SFixProperty, typedef_v3::SPlayerTasks::PlayerTask ) + 
            sizeof( typedef_v3::SPlayerTask[typedef_v2::MAX_TASK_NUMBER] ) );
        assert( checkTail == sizeof( typedef_v3::SFixProperty ) - checkBody );

        // 默认数据清零
        memset( tmpBufEx, 0, sizeof( tmpBufEx ) );

        // 直接拷贝前一段数据
        memcpy( tmpBufEx, tmpBuffer, checkHead ); 

        // 后面的数据拷贝回来。
        memcpy( tmpBufEx + checkBody, tmpBuffer + checkHead, checkTail ); 

        // 重设版本号！
        dest->m_version = dest->GetVersion();

        // 拷贝回tmpBuffer！
        memcpy( tmpBuffer, tmpBufEx, sizeof( tmpBuffer ) );
    }

	//版本3到侠义世界的版本4
	if (fpdb->m_version == typedef_v3::SFixPlayerDataBuf::GetVersion())
	{
		ConverVersion3ToVersion4(fpdb);
	}
	//*/

    // 符合当前版本号的情况下，赋值后成功返回
    if (fpdb->m_version == SFixData::GetVersion())
    {
        PlayerData = *fpdb;
    }
    else
    {
		extern void AddInfo(LPCSTR Info);
		AddInfo( "发现一个无效的版本号数据！\r\n" );
		//MessageBox( 0,"发现一个无效的版本号数据！\r\n",pAccount,0);
        return SAGetCharacterMsg::ERC_FAIL;
    }

    if (strncmp(name, fpdb->m_Name, sizeof(name)) != 0)
        TraceInfo_C( "modify.txt", "玩家角色名被修改！[%s] -> [%s]", fpdb->m_Name, name);

    assert( sid != 0 );
	if (sid != fpdb->m_dwStaticID)
	{
		AddInfo( "玩家对服务器集群唯一静态的ID不匹配！\r\n" );
		//MessageBox( 0,"发现一个无效的版本号数据！\r\n",pAccount,0);
		return SAGetCharacterMsg::ERC_FAIL;
	}
    //assert( sid == fpdb->m_dwStaticID );
    assert( bind[1].Length() <= 10 );

    strncpy( PlayerData.m_Name, name, sizeof( PlayerData.m_Name ) );
    PlayerData.m_Name[CONST_USERNAME-1] = 0;
    PlayerData.m_dwStaticID = sid;

    // 修改登录时间
    _snprintf( sql, sizeof sql, UPDATE_PROPERTY_LOGINTIME, PlayerData.m_dwStaticID, pAccount, dwServerID );	
    qr.Reset();
    if ( ExecuteSql( sql, strlen( sql ), &qr ) != 0 || qr.AffectedRows() != 1 )
        m_errmsg.ShowMsg( EDEBUG, "更新角色上线时间失败%s,%d ",mysql_stmt_error(m_pstmt), PlayerData.m_dwStaticID );

    return SAGetCharacterMsg::ERC_SUCCESS;
}
// 读取数据库角色压缩相关数据并解析
int CNetDBModuleEx::LoadAllPlayerData( )
{
	char sql[1024];
	_snprintf( sql, sizeof sql, SELECT_ALL_PLAYER_DATA);
	sql[ sizeof( sql ) - 1 ] = 0;
	QueryResult qr;
	if ( ExecuteStmt( sql, strlen( sql ), &qr ) != 0 )
		return SAGetCharacterMsg::ERC_FAIL;

	my_ulonglong playercount = 0;
	playercount = qr.AffectedRows();

	m_WriteInfoVec.clear();

	for (int i=0; i<playercount; i++)
	{
		WriteQuery bind;
		char accountname[46];
		char rolename[CONST_USERNAME];
		long index;
		SCharListData rolebuf;
		SFixPlayerDataBuf databuf;
		accountname[45] = 0;
		rolename[10] = 0;
		index = 0;
		WriteInfo tempinfo;
		bind[0].BindString( accountname, sizeof( accountname ) - 1 );
		bind[1].BindString( rolename, sizeof( rolename ) - 1 );
		bind[2].BindBlob( &rolebuf, sizeof( rolebuf ) );
		bind[3].BindBlob( &databuf, sizeof( databuf ) );
		bind[4].BindLong( &index );

		if ( qr.FetchResult( bind ) != 0 || bind[2].IsNull() || bind[3].IsNull())
			return SAGetCharacterMsg::ERC_FAIL;
		assert( bind[2].Length() <= sizeof( SCharListData ) );
		assert( bind[3].Length() <= sizeof( SFixPlayerDataBuf ) );

		memcpy(tempinfo.accountname, accountname, sizeof(accountname));
		memcpy(tempinfo.rolename, rolename, sizeof(rolename));
		tempinfo.rolebuf = rolebuf;
		tempinfo.rolebuf.m_byIndex = (BYTE)index;

		void *unpakData = tmpBuffer;
		size_t unpakSize = sizeof( tmpBuffer );
		TryDecoding( &databuf, bind[2].Length(), unpakData, unpakSize );
		tempinfo.databuf = *(SFixPlayerDataBuf*) unpakData;

		m_WriteInfoVec.insert(WriteInfoVec::value_type(i, tempinfo));
		//m_WriteInfoVec.push_back(tempinfo);
	}
	TraceInfo_C("roleinfo.txt","角色信息记录");
	TraceInfo( "roleinfo.txt", "账号名	角色名	角色index	玩家离线时的时间	玩家离线时的IP	在线时间	性别	玩家所显示图片级别	门派	当前显示的头衔	脸模型	头发模型	等级	转生次数	所属帮派ID	玩家的显示信息	绑定货币	非绑定货币	礼券	获得的礼券总数	元宝	元宝交易总出帐数	元宝交易总进账数	元宝交易总税收数	玩家充值的总数额	当前生命	当前内力	当前真气	当前体力	当前精力	进攻	防御	轻身	健身	玩家当前经验	玩家剩余点数	名望	PK值	侠义值	侠客激励值	当前地图ID	当前世界坐标x	当前世界坐标y	当前世界坐标z	方向	扩展背包激活个数	扩展背包激活个数	是否锁定了仓库	侠客格子激活次数	玩家第一次登陆游戏后的在线累计时间	是否停止计时	已经领取的在线礼包个数	当前的服装模式，是时装还是普通");
	WriteInfoVec::iterator it, end;
	it = m_WriteInfoVec.begin();
	end = m_WriteInfoVec.end();
	while(it != end)
	{
		DOUBLE temp1 = it->second.databuf.m_wExtGoodsActivedNum;
		DOUBLE temp2 = it->second.databuf.m_wExtGoodsActivedNum;		
		DOUBLE temp3 = (it->second.databuf.m_bWarehouseLocked)?1.0f : 0;
		char buff[4096];
		DOUBLE temp4 = it->second.rolebuf.m_byIndex;
		DOUBLE temp5 = it->second.databuf.m_LeaveTime;
		DOUBLE temp6 = it->second.databuf.m_LeaveIP;
		DOUBLE temp7 = it->second.databuf.m_OnlineTime;
		DOUBLE temp8 = it->second.databuf.m_Sex;
		DOUBLE temp9 = it->second.databuf.m_BRON;
		DOUBLE temp10 = it->second.databuf.m_School;
		DOUBLE temp11 = it->second.databuf.m_CurTitle;
		DOUBLE temp12 = it->second.databuf.m_FaceID;
		DOUBLE temp13 = it->second.databuf.m_HairID;
		DOUBLE temp14 = it->second.databuf.m_Level;
		DOUBLE temp15 = it->second.databuf.m_TurnLife;
		DOUBLE temp16 = it->second.databuf.m_TongID;
		DOUBLE temp17 = it->second.databuf.m_ShowState;
		DOUBLE temp18 = it->second.databuf.m_BindMoney;
		DOUBLE temp19 = it->second.databuf.m_Money;
		DOUBLE temp20 = it->second.databuf.m_ZengBao;
		DOUBLE temp21 = it->second.databuf.m_ZBTotalReplenish;
		DOUBLE temp22 = it->second.databuf.m_YuanBao;
		DOUBLE temp23 = it->second.databuf.m_YBTranOut;
		DOUBLE temp24 = it->second.databuf.m_YBTranIn;
		DOUBLE temp25 = it->second.databuf.m_YBTranTax;
		DOUBLE temp26 = it->second.databuf.m_YBTotalReplenish;
		DOUBLE temp27 = it->second.databuf.m_CurHp;
		DOUBLE temp28 = it->second.databuf.m_CurMp;
		DOUBLE temp29 = it->second.databuf.m_CurSp;
		DOUBLE temp30 = it->second.databuf.m_CurTp;
		DOUBLE temp31 = it->second.databuf.m_CurJp;
		DOUBLE temp32 = it->second.databuf.m_JingGong;
		DOUBLE temp33 = it->second.databuf.m_FangYu;
		DOUBLE temp34 = it->second.databuf.m_QingShen;
		DOUBLE temp35 = it->second.databuf.m_JianShen;
		DOUBLE temp36 = it->second.databuf.m_Exp;
		DOUBLE temp37 = it->second.databuf.m_RemainPoint;
		DOUBLE temp38 = it->second.databuf.m_MingWang;
		DOUBLE temp39 = it->second.databuf.m_PKValue;
		DOUBLE temp40 = it->second.databuf.m_XYValue;
		DOUBLE temp41 = it->second.databuf.m_EngValue;
		DOUBLE temp42 = it->second.databuf.m_CurRegionID;
		DOUBLE temp43 = it->second.databuf.m_X;
		DOUBLE temp44 = it->second.databuf.m_Y;
		DOUBLE temp45 = it->second.databuf.m_Z;
		DOUBLE temp46 = it->second.databuf.m_dir;
		DOUBLE temp47 = it->second.databuf.m_FightPetActived;
		DOUBLE temp48 = it->second.databuf.m_dayOnlineTime;
		DOUBLE temp49 = it->second.databuf.m_bStopTime;
		DOUBLE temp50 = it->second.databuf.m_CountDownBeginTime;
		DOUBLE temp51 = it->second.databuf.m_FashionMode;
		sprintf( buff, "%s	%s	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f", 
			it->second.accountname
			, it->second.rolename
			, temp4
			, temp5
			, temp6
			, temp7
			, temp8
			, temp9
			, temp10
			, temp11
			, temp12
			, temp13
			, temp14
			, temp15
			, temp16
			, temp17
			, temp18
			, temp19
			, temp20
			, temp21
			, temp22
			, temp23
			, temp24
			, temp25
			, temp26
			, temp27
			, temp28
			, temp29
			, temp30
			, temp31
			, temp32
			, temp33
			, temp34
			, temp35
			, temp36
			, temp37
			, temp38
			, temp39
			, temp40
			, temp41
			, temp42
			, temp43
			, temp44
			, temp45
			, temp46
			, temp1
			, temp2
			, temp3
			, temp47
			, temp48
			, temp49
			, temp50
			, temp51);
		TraceInfo( "roleinfo.txt", buff );
		it ++;
	}
	//for (int i=0; i<m_WriteInfoVec.size(); i++)
// 	{
// 		DOUBLE temp1,temp2,temp3;
// 		if (m_WriteInfoVec[i].databuf.m_ExtGoods1Actived)
// 			temp1 = 1.0f;
// 		else
// 			temp1 = 0.0f;
// 		if (m_WriteInfoVec[i].databuf.m_ExtGoods2Actived)
// 			temp2 = 1.0f;
// 		else
// 			temp2 = 0.0f;
// 		if (m_WriteInfoVec[i].databuf.m_bWarehouseLocked)
// 			temp3 = 1.0f;
// 		else
// 			temp3 = 0.0f;
// 		char buff[4096];
// 		DOUBLE temp4 = m_WriteInfoVec[i].rolebuf.m_byIndex;
// 		DOUBLE temp5 = m_WriteInfoVec[i].databuf.m_LeaveTime;
// 		DOUBLE temp6 = m_WriteInfoVec[i].databuf.m_LeaveIP;
// 		DOUBLE temp7 = m_WriteInfoVec[i].databuf.m_OnlineTime;
// 		DOUBLE temp8 = m_WriteInfoVec[i].databuf.m_Sex;
// 		DOUBLE temp9 = m_WriteInfoVec[i].databuf.m_BRON;
// 		DOUBLE temp10 = m_WriteInfoVec[i].databuf.m_School;
// 		DOUBLE temp11 = m_WriteInfoVec[i].databuf.m_CurTitle;
// 		DOUBLE temp12 = m_WriteInfoVec[i].databuf.m_FaceID;
// 		DOUBLE temp13 = m_WriteInfoVec[i].databuf.m_HairID;
// 		DOUBLE temp14 = m_WriteInfoVec[i].databuf.m_Level;
// 		DOUBLE temp15 = m_WriteInfoVec[i].databuf.m_TurnLife;
// 		DOUBLE temp16 = m_WriteInfoVec[i].databuf.m_TongID;
// 		DOUBLE temp17 = m_WriteInfoVec[i].databuf.m_ShowState;
// 		DOUBLE temp18 = m_WriteInfoVec[i].databuf.m_BindMoney;
// 		DOUBLE temp19 = m_WriteInfoVec[i].databuf.m_Money;
// 		DOUBLE temp20 = m_WriteInfoVec[i].databuf.m_ZengBao;
// 		DOUBLE temp21 = m_WriteInfoVec[i].databuf.m_ZBTotalReplenish;
// 		DOUBLE temp22 = m_WriteInfoVec[i].databuf.m_YuanBao;
// 		DOUBLE temp23 = m_WriteInfoVec[i].databuf.m_YBTranOut;
// 		DOUBLE temp24 = m_WriteInfoVec[i].databuf.m_YBTranIn;
// 		DOUBLE temp25 = m_WriteInfoVec[i].databuf.m_YBTranTax;
// 		DOUBLE temp26 = m_WriteInfoVec[i].databuf.m_YBTotalReplenish;
// 		DOUBLE temp27 = m_WriteInfoVec[i].databuf.m_CurHp;
// 		DOUBLE temp28 = m_WriteInfoVec[i].databuf.m_CurMp;
// 		DOUBLE temp29 = m_WriteInfoVec[i].databuf.m_CurSp;
// 		DOUBLE temp30 = m_WriteInfoVec[i].databuf.m_CurTp;
// 		DOUBLE temp31 = m_WriteInfoVec[i].databuf.m_CurJp;
// 		DOUBLE temp32 = m_WriteInfoVec[i].databuf.m_JingGong;
// 		DOUBLE temp33 = m_WriteInfoVec[i].databuf.m_FangYu;
// 		DOUBLE temp34 = m_WriteInfoVec[i].databuf.m_QingShen;
// 		DOUBLE temp35 = m_WriteInfoVec[i].databuf.m_JianShen;
// 		DOUBLE temp36 = m_WriteInfoVec[i].databuf.m_Exp;
// 		DOUBLE temp37 = m_WriteInfoVec[i].databuf.m_RemainPoint;
// 		DOUBLE temp38 = m_WriteInfoVec[i].databuf.m_MingWang;
// 		DOUBLE temp39 = m_WriteInfoVec[i].databuf.m_PKValue;
// 		DOUBLE temp40 = m_WriteInfoVec[i].databuf.m_XYValue;
// 		DOUBLE temp41 = m_WriteInfoVec[i].databuf.m_EngValue;
// 		DOUBLE temp42 = m_WriteInfoVec[i].databuf.m_CurRegionID;
// 		DOUBLE temp43 = m_WriteInfoVec[i].databuf.m_X;
// 		DOUBLE temp44 = m_WriteInfoVec[i].databuf.m_Y;
// 		DOUBLE temp45 = m_WriteInfoVec[i].databuf.m_Z;
// 		DOUBLE temp46 = m_WriteInfoVec[i].databuf.m_dir;
// 		DOUBLE temp47 = m_WriteInfoVec[i].databuf.m_FightPetActived;
// 		DOUBLE temp48 = m_WriteInfoVec[i].databuf.m_firstLoginTime;
// 		DOUBLE temp49 = m_WriteInfoVec[i].databuf.m_bStopTime;
// 		DOUBLE temp50 = m_WriteInfoVec[i].databuf.m_dTimeGiftState;
// 		DOUBLE temp51 = m_WriteInfoVec[i].databuf.m_FashionMode;
// 		sprintf( buff, "%s	%s	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f	%f", 
// 			m_WriteInfoVec[i].accountname
// 			, m_WriteInfoVec[i].rolename
// 			, temp4
// 			, temp5
// 			, temp6
// 			, temp7
// 			, temp8
// 			, temp9
// 			, temp10
// 			, temp11
// 			, temp12
// 			, temp13
// 			, temp14
// 			, temp15
// 			, temp16
// 			, temp17
// 			, temp18
// 			, temp19
// 			, temp20
// 			, temp21
// 			, temp22
// 			, temp23
// 			, temp24
// 			, temp25
// 			, temp26
// 			, temp27
// 			, temp28
// 			, temp29
// 			, temp30
// 			, temp31
// 			, temp32
// 			, temp33
// 			, temp34
// 			, temp35
// 			, temp36
// 			, temp37
// 			, temp38
// 			, temp39
// 			, temp40
// 			, temp41
// 			, temp42
// 			, temp43
// 			, temp44
// 			, temp45
// 			, temp46
// 			, temp1
// 			, temp2
// 			, temp3
// 			, temp47
// 			, temp48
// 			, temp49
// 			, temp50
// 			, temp51);
// 		TraceInfo( "roleinfo.txt", buff );
// 	}
	return true;
}

// 保存角色的数据成功返回非0
int CNetDBModuleEx::SavePlayerData(LPCSTR pAccount, DWORD dwServerID, SFixData &bdata, SWareHouses *wdata)
{
    assert( pAccount != NULL );

    // 保存的时候还是需要更新快照数据，这里有点麻烦。考虑能否优化
    SCharListData snapshot;
    memset(&snapshot, 0, sizeof snapshot);
    memcpy(snapshot.m_szName, bdata.m_Name, sizeof bdata.m_Name);

    // 数据压缩
    LPVOID pakBuf = tmpBuffer;
    size_t pakSize = sizeof(tmpBuffer);
	TryEncoding(&bdata, sizeof(SFixData), pakBuf, pakSize);

    DWORD verify = _GetCrc32(pakBuf, (DWORD)pakSize);

    //QueryBind<3> bind;
	QueryBind<8> bind;
    bind[0].BindBlob(pakBuf, pakSize);
    bind[1].BindBlob(&snapshot, sizeof(snapshot));
    bind[2].BindULong(&verify);
	DWORD level = bdata.m_dwLevel;
	bind[3].BindULong(&level);

	NewRankList temp;
	
	memcpy(temp.name,bdata.m_Name,CONST_USERNAME);
	bind[4].BindBlob(&temp,sizeof(NewRankList));

	DWORD tollgate = bdata.m_dwLatestBattleFileID;//关卡
	bind[5].BindULong(&tollgate);
	DWORD viplv = 0;//vip等级
	bind[6].BindULong(&viplv);
	DWORD guideID = bdata.m_NewbieGuideData.curGuide;//新手引导 千位为引导类型,以下为哪一步
	bind[7].BindULong(&guideID);


    char sql[1024];
    _snprintf(sql, sizeof sql, UPDATE_PROPERTY_ONLYBUF, bdata.m_dwStaticID, pAccount, dwServerID);
    sql[sizeof( sql ) - 1] = 0;

    QueryResult qr;
    if (ExecuteStmt(sql, strlen(sql), &qr, bind) != 0 || qr.AffectedRows() != 1)
    {
        // 特殊处理，当未更新到任何数据时，说明玩家数据可能存在不对应的serverid
        if (qr.AffectedRows() == 0)
            m_errmsg.ShowMsg(EDEBUG, "Save Player failure：[%s]", sql);
        return 0;
    }

    
    return 1;
}




// 删除玩家角色数据
int CNetDBModuleEx::DeletePlayerData( DWORD dwRoleID, LPCSTR account, DWORD dwServerID )
{
    char sql[512];
    _snprintf(sql, sizeof sql, "call DeleteRoleData( %d, '%s', %d )", dwRoleID, account, dwServerID);
    sql[sizeof(sql) - 1] = 0;

    QueryResult qr;
    return (ExecuteSql( sql, strlen( sql ), &qr ) == 0 ) && ( qr.AffectedRows() == 1);
}

// 更新账号服务器锁定block为False时为解除锁定
// 返回值有：
// [0]	[操作成功] 
// [1]	[操作不成功，因为锁定状态已经被置为目标状态] 
// [-1]	[执行过程中出现错误]
// [-2]	[不存在此账号] 
// [-3]	[存在此账号，但已经被其他服务器锁定]
int CNetDBModuleEx::UpdateAccountLock( LPCSTR account, DWORD serverId, BOOL lockState )
{
    char sql[512];
    _snprintf( sql, sizeof sql, CHECK_ACCOUNT_EXIST, account );
    sql[ sizeof( sql ) - 1 ] = 0;

    QueryResult qr;
    // 如果执行失败，那么返回-1
    if ( ExecuteStmt( sql, strlen( sql ), &qr ) != 0 )
        return -1;

    // 如果没有影响行数，则表明没有对应的账号数据，应该返回-2
    if ( qr.AffectedRows() == 0 )
        return -2;

    // 获取角色锁定数据，用于检测是否需要更新。。。
    unsigned long sid = 0;
    QueryBind< 1 > bind;
    bind[0].BindULong( &sid );
    if ( qr.FetchResult( bind ) != 0 )
        return -1;

    // 如果目标结果已经为待更新结果，返回 1
    if ( serverId == ( lockState ? sid : 0 ) )
        return 1;

    // 最后执行更新
    if ( lockState ) _snprintf( sql, sizeof sql, UPDATE_ACCOUNT_LOCK, serverId, account );
    else _snprintf( sql, sizeof sql, UPDATE_ACCOUNT_UNLOCK, account, serverId );
    sql[ sizeof( sql ) - 1 ] = 0;
    qr.Reset();
    if ( ExecuteSql( sql, strlen( sql ), &qr ) != 0 )
        return -1;

    return ( qr.AffectedRows() == 1 ) ? 0 : -3;
}

/*
//Get账号的仓库数据
int CNetDBModuleEx::GetWareHouseData( LPCSTR pAccount, LPCSTR name, DWORD dwServerID, SFixProperty &data )
{
    assert( pAccount != NULL );
    assert( name != NULL );

    char sql[1024];
    _snprintf( sql, sizeof sql, SELECT_WAREHOUSE, pAccount, name, dwServerID );
    sql[ sizeof( sql ) - 1 ] = 0;
    QueryResult qr;
    // 需要考虑没有数据的情况（账号主数据未创建）！该处属于异常，不能继续执行！
    if ( ExecuteStmt( sql, strlen( sql ), &qr ) != 0 || qr.AffectedRows() != 1 )
        return 0;

    my_bool isnull = false;
    unsigned long len = 0;
    SWareHouses warehouses;
    long lockState = 0;

    QueryBind< 2 > bind;
    bind[0].BindLong( &lockState ); // 这里的类型本来是 MYSQL_TYPE_TINY， 但在读取的时候应该是可以支持的
    bind[1].BindBlob( &warehouses, sizeof( warehouses ) );
    if ( qr.FetchResult( bind ) != 0 )
        return 0;

    // 一开始初始化的仓库可能没有数据！
    if ( bind[1].IsNull() )
    {
        memset( data.SFixStorage1::m_pStorageGoods, 0, sizeof( SFixStorage1 ) );
        memset( data.SFixStorage2::m_pStorageGoods, 0, sizeof( SFixStorage1 ) );
        memset( data.SFixStorage3::m_pStorageGoods, 0, sizeof( SFixStorage1 ) );
    }
    else
    {
        assert( bind[1].Length() <= sizeof( warehouses ) );

        void *unpakData = tmpBuffer;
        size_t unpakSize = sizeof( tmpBuffer );
        TryDecoding( &warehouses, bind[1].Length(), unpakData, unpakSize );
        if ( unpakSize != sizeof( SWareHouses ) )
            return 0;

        SWareHouses *temp = ( SWareHouses* )unpakData;
        memcpy( data.SFixStorage1::m_pStorageGoods, &temp->m_WareHouse[0], sizeof( SFixStorage1 ) );
        memcpy( data.SFixStorage2::m_pStorageGoods, &temp->m_WareHouse[1], sizeof( SFixStorage1 ) );
        memcpy( data.SFixStorage3::m_pStorageGoods, &temp->m_WareHouse[2], sizeof( SFixStorage1 ) );
    }

    return 1;
}

//保存仓库数据
int CNetDBModuleEx::SaveWareHouseData( LPCSTR pAccount, LPCSTR name, DWORD dwServerID, bool blocked, SWareHouses &data )
{
    char sql[1024];
    _snprintf( sql, sizeof sql, UPDATE_WAREHOUSE, blocked, pAccount, name, dwServerID );
    sql[ sizeof( sql ) - 1 ] = 0;

    // 数据压缩
    LPVOID pakBuf = tmpBuffer;
    size_t pakSize = sizeof( tmpBuffer );
    TryEncoding( &data, sizeof( SWareHouses ), pakBuf, pakSize );

    QueryResult qr;
    QueryBind< 1 > args;
    args[0].BindBlob( pakBuf, pakSize );
    if ( ExecuteStmt( sql, strlen( sql ), &qr, args ) != 0 || qr.AffectedRows() != 1 )
        return 0;

    return 1;
}
*/

// int CNetDBModuleEx::SaveFactions( SQSaveFactionMsg *pMsg )
// {
// 	if ( pMsg == NULL || pMsg->nFactions >= MAX_FACTION_NUMBER )
// 		return 0;
// 
//     TraceInfo_C( "faction.txt", "处理保存帮派消息【%d】【%d】！", pMsg->nFactions, pMsg->nTime );
// 
// 	// 首先将数据全部提取出来
//     SaveFactionData sfd;
// 	LPBYTE streamIter = ( LPBYTE )pMsg->streamFaction;
// 	const DWORD dwFactionMax = pMsg->nFactions;
// 	for( DWORD n = 0; n < dwFactionMax; n++ )
// 	{
// 		// 读取帮派头数据
// 		sfd.stFaction = *( SaveFactionData::SaveFactionInfo* )streamIter;
// 		streamIter += sizeof( SaveFactionData::SaveFactionInfo );
// 
// 		// 成员数据
// 		size_t memberSize = sizeof( SaveFactionData::SaveMemberInfo ) * sfd.stFaction.byMemberNum;
// 		memcpy( sfd.stMember, streamIter, memberSize );
// 		streamIter += memberSize;
// 
//         TraceInfo_C( "faction.txt", "保存帮派【%s】【%d】！", sfd.stFaction.szFactionName, sfd.stFaction.byMemberNum );
// 
// 		if ( SaveFaction( sfd, pMsg->nServerID, pMsg->nTime ) == 0 )
// 			m_errmsg.ShowMsg( EDEBUG, "帮派数据保存失败%s", sfd.stFaction.szFactionName );
// 	}
// 
//     // 现在不直接删除批次数据了! 改为读取时临时检测最后保存批次
// 	//删除所有这次保存失败之后的数据
//     //char sql[1024];
//     //_snprintf( sql, sizeof sql, DEL_FACTIONDATA, pMsg->nServerID, pMsg->nTime );
//     //sql[ sizeof( sql ) - 1 ] = 0;
//     //ExecuteSqlCmd( sql );	//不好判断返回值。
//     return 1;
// }

int CNetDBModuleEx::SaveFactions(struct SQSaveFactionMsg *pMsg)
{
	if (pMsg == NULL || pMsg->nFactions >= MAX_FACTION_NUMBER)
		return 0;

	//TraceInfo_C("faction.txt", "处理保存帮派消息【%d】【%d】！", pMsg->nFactions, pMsg->nTime);

	rfalse(2, 1, "SaveFactions  %d", pMsg->nFactions);

// 首先将数据全部提取出来
	SaveFactionData_New ssfdnew;
	LPBYTE streamIter = (LPBYTE)pMsg->streamFaction;
	const DWORD dwFactionMax = pMsg->nFactions;
	for (DWORD n = 0; n < dwFactionMax; n++)
	{
		// 读取帮派头数据
		
		memset(&ssfdnew, 0, sizeof(SaveFactionData_New));
		memcpy(&ssfdnew, streamIter, sizeof(SaveFactionData_New));


		// 成员数据
		streamIter += sizeof(SaveFactionData_New);

		TraceInfo_C("faction.txt", "保存帮派【%s】【%d】！", ssfdnew.sfactioninfo.szFactionName, ssfdnew.sfactioninfo.CurMemberNum);

		if (SaveFaction(ssfdnew, pMsg->nServerID, pMsg->nTime) == 0)
			m_errmsg.ShowMsg(EDEBUG, "帮派数据保存失败%s", ssfdnew.sfactioninfo.szFactionName);
	}
	return 1;
}

int CNetDBModuleEx::SaveFactionsData(struct SQSaveFactionDataMsg *pMsg)
{
	if (pMsg == NULL || pMsg->nFactions >= MAX_FACTION_NUMBER)
		return 0;

	//TraceInfo_C("faction.txt", "处理保存帮派消息【%d】【%d】！", pMsg->nFactions, pMsg->nTime);

	rfalse(2, 1, "SaveFactions  %d", pMsg->nFactions);

	// 首先将数据全部提取出来
	SaveFactionData_Lua ssfdnew;
	LPBYTE streamIter = (LPBYTE)pMsg->streamFaction;
	const DWORD dwFactionMax = pMsg->nFactions;
	for (DWORD n = 0; n < dwFactionMax; n++)
	{
		// 读取帮派头数据

		memset(&ssfdnew, 0, sizeof(SaveFactionData_Lua));
		memcpy(&ssfdnew, streamIter, sizeof(SaveFactionData_Lua));


		// 成员数据
		streamIter += sizeof(SaveFactionData_Lua);

		TraceInfo_C("faction.txt", "保存帮派【%s】！", ssfdnew.szFactionName);

		if (SaveFaction(ssfdnew, pMsg->nServerID, pMsg->nTime) == 0)
			m_errmsg.ShowMsg(EDEBUG, "帮派数据保存失败%s", ssfdnew.szFactionName);
	}
	return 1;
}

// int CNetDBModuleEx::SaveFaction( SaveFactionData &stFaction, DWORD serverId, DWORD tick )
// {
//     QueryResult qr;
//     QueryBind< 5 > args;
// 
//     if ( stFaction.stFaction.szFactionName[0] == 0 || 
//         stFaction.stFaction.szCreatorName[0] == 0 ||
//         stFaction.stFaction.factionId < 10000 || 
//         stFaction.stFaction.factionId > 0xffff ||
//         stFaction.stFaction.byMemberNum == 0 || 
//         stFaction.stFaction.byMemberNum > MAX_MEMBER_NUMBER )
//         return 0;
// 
//     // 这个由于不会导致特别严重的后果，算是用作容错
//     if ( stFaction.stFaction.byMemberNum > stFaction.stFaction.byFactionLevel * 50 )
//         LogError( "SaveFaction", "membernum error " );
// 
//     // 数据压缩
//     size_t dataSize = sizeof( SaveFactionData ) - 
//         ( MAX_MEMBER_NUMBER - stFaction.stFaction.byMemberNum ) * sizeof( stFaction.stMember[0] );
// 
//     LPVOID pakBuf = tmpBuffer;
//     size_t pakSize = sizeof( tmpBuffer );
//     TryEncoding( &stFaction, dataSize, pakBuf, pakSize );
// 
//     DWORD factionId = stFaction.stFaction.factionId;
// 
//     // 先尝试更新，失败时直接添加
// 	args[0].BindULong( &tick );
// 	args[1].BindBlob( pakBuf, pakSize );
// 	args[2].BindLong( ( long* )&serverId );
// 	args[3].BindString( stFaction.stFaction.szFactionName, strlen( stFaction.stFaction.szFactionName ) );
// 	args[4].BindULong( &factionId );
// 
// 	if ( ExecuteStmt( UPDATE_FACTION, sizeof( UPDATE_FACTION ), &qr, args ) != 0 || qr.AffectedRows() != 1 )
//     {
//         qr.Reset();
//         if ( ExecuteStmt( INSERT_FACTION, sizeof( INSERT_FACTION ), &qr, args ) != 0 || qr.AffectedRows() != 1 )
//             return 0;
//     }
// 
//     return 1;
// }

int CNetDBModuleEx::SaveFaction(SaveFactionData_New &stFaction, DWORD serverId, DWORD tick)
{
	QueryResult qr;
	

	if ((strlen(stFaction.sfactioninfo.szFactionName) == 0) || (strlen(stFaction.sfactioninfo.szCreatorName) == 0))
	{
		return 0;
	}

	// 数据压缩
// 	size_t dataSize = sizeof(SaveFactionData_New) -
// 		(MAX_MEMBER_NUMBER - stFaction.sfactioninfo.CurMemberNum) * sizeof(stFaction.sfactioninfo.m_factionmember[0]);
	size_t dataSize = sizeof(SaveFactionData_New);

	LPVOID pakBuf = tmpBuffer;
	size_t pakSize = sizeof(tmpBuffer);
	if (!TryEncoding(&stFaction, dataSize, pakBuf, pakSize))
	{
		rfalse("TryEncoding Faile");
	}
	
	DWORD factionId = stFaction.sfactioninfo.FactionID;

// 	char sql[1024];
// 	_snprintf(sql, sizeof sql, UPDATE_FACTION, serverId, stFaction.sfactioninfo.szFactionName, factionId);
// 	sql[sizeof(sql) - 1] = 0;
	QueryBind< 3 > args;
	/*args[0].BindBlob(&stFaction, sizeof(SaveFactionData_New));*/
	args[0].BindBlob(pakBuf, pakSize);
	args[1].BindLong((long*)&serverId);
	args[2].BindULong(&factionId);

	if (ExecuteStmt(UPDATE_FACTION, sizeof(UPDATE_FACTION), &qr, args) != 0 )
	{
		qr.Reset();
		if (ExecuteStmt(INSERT_FACTION, sizeof(INSERT_FACTION), &qr, args) != 0)
			return 0;
	}
	if (qr.AffectedRows() == 0) //没有更新到数据库
	{
		qr.Reset();
		QueryBind< 5> args;
		args[0].BindULong(&tick);
		args[1].BindBlob(pakBuf, pakSize);
		//args[1].BindBlob(&stFaction, sizeof(SaveFactionData_New));
		args[2].BindLong((long*)&serverId);
		args[3].BindString(stFaction.sfactioninfo.szFactionName, CONST_USERNAME);
		args[4].BindULong(&factionId);
		if (ExecuteStmt(INSERT_FACTION, sizeof(INSERT_FACTION), &qr, args) != 0)
		{
			return 0;
		}
	}

// 	if (ExecuteStmt(UPDATE_FACTION, sizeof(UPDATE_FACTION), &qr, args) != 0 || qr.AffectedRows() != 1)
// 	{
// 		qr.Reset();
// 		if (ExecuteStmt(INSERT_FACTION, sizeof(INSERT_FACTION), &qr, args) != 0 || qr.AffectedRows() != 1)
// 			return 0;
// 	}

	return 1;
}

int CNetDBModuleEx::SaveFaction(SaveFactionData_Lua &stluaFaction, DWORD serverId, DWORD tick)
{
	QueryResult qr;

	if ((strlen(stluaFaction.szFactionName) == 0))
	{
		return 0;
	}

	// 数据压缩
	// 	size_t dataSize = sizeof(SaveFactionData_New) -
	// 		(MAX_MEMBER_NUMBER - stFaction.sfactioninfo.CurMemberNum) * sizeof(stFaction.sfactioninfo.m_factionmember[0]);
	size_t dataSize = sizeof(SaveFactionData_Lua);

	LPVOID pakBuf = tmpBuffer;
	size_t pakSize = sizeof(tmpBuffer);
	if (!TryEncoding(&stluaFaction, dataSize, pakBuf, pakSize))
	{
		rfalse("TryEncoding Faile");
	}

	DWORD factionId = stluaFaction.FactionID;

	// 	char sql[1024];
	// 	_snprintf(sql, sizeof sql, UPDATE_FACTION, serverId, stFaction.sfactioninfo.szFactionName, factionId);
	// 	sql[sizeof(sql) - 1] = 0;
	QueryBind< 3 > args;
	/*args[0].BindBlob(&stFaction, sizeof(SaveFactionData_New));*/
	args[0].BindBlob(pakBuf, pakSize);
	args[1].BindLong((long*)&serverId);
	args[2].BindULong(&factionId);

	if (ExecuteStmt(UPDATE_FACTION, sizeof(UPDATE_FACTION), &qr, args) != 0)
	{
		qr.Reset();
		if (ExecuteStmt(INSERT_FACTION, sizeof(INSERT_FACTION), &qr, args) != 0)
		{
			LogError("ExecuteStmt  INSERT_FACTION", GetLastError());
			return 0;
		}
			
	}
	if (qr.AffectedRows() == 0) //没有更新到数据库
	{
		qr.Reset();
		QueryBind< 5> args;
		args[0].BindULong(&tick);
		args[1].BindBlob(pakBuf, pakSize);
		//args[1].BindBlob(&stFaction, sizeof(SaveFactionData_New));
		args[2].BindLong((long*)&serverId);
		args[3].BindString(stluaFaction.szFactionName, CONST_USERNAME);
		args[4].BindULong(&factionId);
		if (ExecuteStmt(INSERT_FACTION, sizeof(INSERT_FACTION), &qr, args) != 0)
		{
			LogError("ExecuteStmt", GetLastError());
			return 0;
		}
	}
}

int CNetDBModuleEx::GetFactions( const DNID dwSID, const __int32 nServerID )
{
	if ( nServerID <= 0 )
		return 0;

    TraceInfo_C( "faction.txt", "处理读取帮派消息【%d】！", nServerID );

	char sql[1024];
	_snprintf( sql, sizeof sql, GET_FACTIONLIST, nServerID, nServerID );
    sql[ sizeof( sql ) - 1 ] = 0;
	QueryResult qr;
	if ( ( ExecuteStmt( sql, strlen( sql ), &qr ) != 0 ) || qr.AffectedRows() < 0 )
        return 0;

	SaveFactionData *stFaction = ( SaveFactionData* )tmpBuffer;
	SAGetFactionMsg &factionMsg = * new ( msgBuffer ) SAGetFactionMsg();
    factionMsg.isLast = FALSE;
    factionMsg.dwSendSize = 0;
    factionMsg.nFactions = 0;
		 
	SaveFactionData sfd;
	memset( &sfd, 0, sizeof( sfd ) );

    Query::QueryBind< 1 > bind;
    bind[0].BindBlob( &sfd, sizeof( SaveFactionData ) );

    long idx = 0;
    while ( qr.FetchResult( bind ) == 0 )
    {
        assert( !bind[0].IsNull() );
        assert( bind[0].Length() <= sizeof( SaveFactionData ) );

        // 数据解压缩，直接写到目标slot里边!
        void *unpakData = &stFaction[ idx ];
        size_t unpakSize = sizeof( SaveFactionData );
        BOOL ck = TryDecoding( &sfd, bind[0].Length(), unpakData, unpakSize );

        SaveFactionData *temp = ( SaveFactionData* )unpakData;
        size_t dataSize = sizeof( SaveFactionData ) - 
            ( MAX_MEMBER_NUMBER - stFaction[ idx ].stFaction.byMemberNum ) * sizeof( stFaction[ idx ].stMember[0] );

        // 数据大小对不上！
        if ( unpakSize != dataSize )
        {
            TraceInfo_C( "faction.txt", "读取帮派【%d】，解压失败！", idx );
            continue;
        }

        // 如果不是解压数据，则需要将数据拷贝到目标slot里边
        if ( !ck )
            memcpy( &stFaction[ idx ], &sfd, dataSize );

        TraceInfo_C( "faction.txt", "读取帮派成功【%s】【%d】！", 
            stFaction[ idx ].stFaction.szFactionName, stFaction[ idx ].stFaction.byMemberNum );

	    // memset( reinterpret_cast< char* >( &stFaction[ idx ] ) + dataSize, 0, sizeof( sfd ) - dataSize );
		idx++;
    }
	
    // copy from old netdbmoudule.cpp
	SaveFactionData *iter = stFaction;
	LPBYTE streamIter = ( LPBYTE )factionMsg.streamFaction;
	DWORD nFactions = 0;

	for ( int n = 0; n < idx; n ++, iter ++ )
	{
        if ( iter->stFaction.byMemberNum == 0 )
            continue;

		DWORD size = sizeof( SaveFactionData::SaveFactionInfo ) + iter->stFaction.byMemberNum * sizeof( SaveFactionData::SaveMemberInfo );

        // 检测是否发送会越界！
		if ( factionMsg.dwSendSize + size >= sizeof( factionMsg.streamFaction ) )
		{
			factionMsg.nFactions = nFactions;
			SendMessage( dwSID, &factionMsg, factionMsg.dwSendSize + 
				( sizeof( factionMsg ) - sizeof( factionMsg.streamFaction ) ) );

			streamIter = (LPBYTE)factionMsg.streamFaction;
			nFactions = 0;
			factionMsg.dwSendSize = 0;
		}

		factionMsg.dwSendSize += size;

		// 写入帮派头数据
		*( SaveFactionData::SaveFactionInfo* )streamIter = iter->stFaction;
		streamIter += sizeof( SaveFactionData::SaveFactionInfo );
		
		// 成员数据
		memcpy( streamIter, iter->stMember, sizeof( SaveFactionData::SaveMemberInfo ) * iter->stFaction.byMemberNum );
		streamIter += sizeof( SaveFactionData::SaveMemberInfo ) * iter->stFaction.byMemberNum;

		nFactions ++;
	}

    // 最后这个消息用于通知登陆服务器读取正常完成！所以就算没有读到任何帮派，也需要发送该消息
    factionMsg.isLast = TRUE;
	factionMsg.nFactions = nFactions;
	SendMessage( dwSID, &factionMsg, factionMsg.dwSendSize + 
		( sizeof( factionMsg ) - sizeof( factionMsg.streamFaction ) ) );

    return 1;
}

int CNetDBModuleEx::SaveBuildings( struct SQBuildingSaveMsg *pMsg )
{
	int nTime = static_cast< DWORD >( time( NULL ) - timeSegment );
    if ( nTime < 0 )
        return MessageBox( 0, "time check error !!!", "critical error", 0 );

    // 这个是特殊处理，==4 时属于没有数据！
    if ( ( ( LPWORD )pMsg->Buffers )[ 0 ] > 4 )
    {
        lite::Serialreader slr( pMsg->Buffers );
        while ( slr.curSize() < slr.maxSize() )
        {
            lite::Variant lvt = slr();
            if ( lvt.dataType != lite::Variant::VT_POINTER ) 
                continue;

            lite::Serialreader slr2( ( void* )lvt._pointer );
            lite::Variant lvt2 = slr2();
            if ( ( lvt2.dataType != lite::Variant::VT_POINTER ) || ( lvt2.dataSize != sizeof( SaveBlock_fixed ) ) )
                continue;

            const SaveBlock_fixed *block = ( const SaveBlock_fixed* )lvt2._pointer;
            DWORD parentId = block->parentId;
            QWORD uniqueId = block->UniqueId();

            mysql::Query::QueryBind< 5 > bind;
            bind[2].BindULong( &pMsg->dwServerID );
            bind[3].BindULong( &parentId );
			bind[4].BindULongLong( &uniqueId );
            bind[1].BindBlob( ( void* )lvt._pointer, lvt.dataSize );
            bind[0].BindULong( ( ULONG* )&nTime );

            QueryResult qr;
            if ( ExecuteStmt( UPDATE_BUILDING, sizeof( UPDATE_BUILDING ), &qr, bind ) != 0 || qr.AffectedRows() != 1 )
            {
                // 失败的情况下，尝试新建插入！
                qr.Reset();
                if ( ExecuteStmt( INSERT_BUILDING, sizeof( INSERT_BUILDING ), &qr, bind ) != 0 || qr.AffectedRows() != 1 )
                    continue; // ... 这样都失败的话，该怎样处理呢？
            }
		}
    }

    // 和帮派一样，这里的逻辑也不处理了！
	////将本次操作没有更新的数据全部删掉
	//Query::PrepareProcedure( "p_DelBuildingData" );
	//Query::m_pCommand->Parameters->GetItem( "@ParentID")->Value = pmsg->wParentID;
	//Query::m_pCommand->Parameters->GetItem( "@ServerID")->Value = pmsg->dwServerID; 
	//Query::m_pCommand->Parameters->GetItem( "@updateTime")->Value = ntime;
	//Query::m_pCommand->Execute( NULL, NULL, Query::m_pCommand->CommandType );	
	//assert( Query::ReturnOK() );	
	//Query::EndTransAction();
    return TRUE;
}

int CNetDBModuleEx::GetBuildings( const DNID clientDnid, struct SQBuildingGetMsg *pMsg )
{
	char sql[1024];
    _snprintf( sql, sizeof( sql ), GET_BUILDINGLIST, 
        pMsg->dwServerID, pMsg->wParentID, pMsg->dwServerID, pMsg->wParentID );
    sql[ sizeof( sql ) - 1 ] = 0;

    QueryResult qr;
    if ( ExecuteStmt( sql, strlen( sql ), &qr ) != 0 || qr.AffectedRows() < 0 )
        return 0; // 执行处措时不返回任何消息

	WORD wIndex = 0;	
	DWORD dwTotalSize = 0;
	size_t size = 0;	//本次从数据库中读取的缓冲区的大小	

    SABuildingGetMsg &msg = * new ( msgBuffer ) SABuildingGetMsg;
    msg.dwZoneID = pMsg->dwZoneID;
    msg.wParentID = pMsg->wParentID;
    msg.retMsg = SABuildingGetMsg::RET_SUCCESS;
	lite::Serializer slm( msg.Buffers, sizeof( msgBuffer ) - sizeof( msg ) );
	
    if ( qr.AffectedRows() > 0 )
    {
        QueryBind< 1 > bind;
        bind[0].BindBlob( tmpBuffer, sizeof( tmpBuffer ) );
        while ( qr.FetchResult( bind ) == 0 ) 
        {
            assert( !bind[0].IsNull() );

            try 
            { 
                slm( tmpBuffer, bind[0].Length() ); 
            }
            catch ( lite::FullBuffer & )
            {
                // 缓存已经满了，先把前面部分数据发送了来
                msg.WBufSize = ( WORD )( sizeof( msgBuffer ) - slm.EndEdition() );
                SendMessage( clientDnid, &msg, msg.WBufSize );
                slm.Rewind();
            }

		    dwTotalSize += ( DWORD )size;
		    size = 0;
		    wIndex++;
        }
    }

    msg.WBufSize = ( WORD )( sizeof( msgBuffer ) - slm.EndEdition() );
    SendMessage( clientDnid, &msg, msg.WBufSize );

	//Msg.wNumbers = wIndex;
	//Msg.WBufSize =  sizeof BYTE * SENDER_MAXBUFFER - slm.EndEdition();

	//考虑到数据对齐问题,前面一部份wActualSize 不太固定，所以用总大小 - 动态BUF大小
	//dwTotalSize += wIndex * ( sizeof msg.Buffers[0] - sizeof msg.Buffers[0].buffer );
	return TRUE;
}


int CNetDBModuleEx::GetUnionData( const DNID dwSID, SQGetUnionDataMsg * pMsg )
{
//     TraceInfo_C( "uniondata.txt", "开始处理结义数据!" );
// 
//     QueryResult qr;
//     QueryResult qr2;
//     char sql[1024] = { 0 };
// 
//     if ( ( ExecuteStmt( GET_UNIONLIST, strlen( GET_UNIONLIST ), &qr ) != 0 ) || qr.AffectedRows() < 0 )
//         return 0;
// 
//     char title[9];
//     char suffix[3];
//     DWORD roleid[ SPlayerUnionBase::NUMBER_MAX ];
//     DWORD exp[ SPlayerUnionBase::NUMBER_MAX ];
// 
//     SPlayerUnionData *unionData = ( SPlayerUnionData* )tmpBuffer;
// 	SAGetUnionDataMsg &msg = * new ( msgBuffer ) SAGetUnionDataMsg();
//     msg.dwZoneID = pMsg->dwZoneID;
//     msg.readEnd = FALSE;
// 
//     Query::QueryBind< 18 > bind;
// 
//     bind[0].BindString( title, sizeof( title ) - 1 );
//     bind[1].BindString( suffix, sizeof( suffix ) - 1 );
// 
//     for ( int i = 0; i < SPlayerUnionBase::NUMBER_MAX; i++ )
//     {
//         bind[ 2 + i ].BindULong( &roleid[ i ] );
//         bind[ 10 + i ].BindULong( &exp[ i ] );
//     }
// 
//     long idx = 0;
//     while ( qr.FetchResult( bind ) == 0 )
//     {
//         dwt::strcpy( unionData[ idx ].title, title, 9 );
//         dwt::strcpy( unionData[ idx ].suffx, suffix, 3 );
//         unionData[ idx ].title[8] = 0;
//         unionData[ idx ].suffx[2] = 0;
// 
//         for ( int i = 0; i < SPlayerUnionBase::NUMBER_MAX; i++ )
//         {
//             unionData[ idx ].roleid[ i ] = roleid[ i ];
//             unionData[ idx ].exp[ i ] = exp[ i ];
//             unionData[ idx ].szName[ i ][ 0 ] = 0;
//         }
// 
//         idx ++;
//     }
// 
//     for ( int i = 0; i < idx; i++ )
//     {
//         for ( int j = 0; j < SPlayerUnionBase::NUMBER_MAX; j++ )
//         if ( unionData[ i ].roleid[ j ] != 0 )
//         {
//             _snprintf( sql, sizeof sql, GET_PLAYERNAME, unionData[ i ].roleid[ j ] );
//             sql[ sizeof( sql ) - 1 ] = 0;
// 
//             qr2.Reset();
//             if ( ( ExecuteStmt( sql, strlen( sql ), &qr2 ) != 0 ) || qr2.AffectedRows() < 0 )
//                 continue;
// 
//             Query::QueryBind< 1 > args;
//             char name[CONST_USERNAME];
//             args[ 0 ].BindString( name, sizeof( name ) - 1 );
//             if ( qr2.FetchResult( args ) == 0 )
//             {
//                 dwt::strcpy( unionData[ i ].szName[ j ], name, 10 );
//                 unionData[ i ].szName[ j ][10] = 0;
//             }
//         }
//     }
// 
//     // copy from old netdbmoudule.cpp
// 	SPlayerUnionData *iter = unionData;
//     LPBYTE streamIter = ( LPBYTE )msg.streamData;
//     msg.dwSendSize = 0;
//     msg.unionCount = 0;
// 
// 	for ( int i = 0; i < idx; i ++ )
// 	{
//         if ( msg.dwSendSize + sizeof( SPlayerUnionData ) > sizeof( msg.streamData ) )
//         {
//             SendMessage( dwSID, &msg, msg.dwSendSize + ( sizeof( SAGetUnionDataMsg ) - sizeof( msg.streamData ) ) );
// 
// 			streamIter = (LPBYTE)msg.streamData;
// 			msg.unionCount = 0;
// 			msg.dwSendSize = 0;
//         }
// 
//         // 写入帮派头数据
// 		*( SPlayerUnionData* )streamIter = unionData[i];
// 		streamIter += sizeof( SPlayerUnionData );
//         msg.dwSendSize += sizeof( SPlayerUnionData );
// 
//         msg.unionCount ++;
//     }
// 
//     msg.readEnd = TRUE;
//     SendMessage( dwSID, &msg, msg.dwSendSize + ( sizeof( SAGetUnionDataMsg ) - sizeof( msg.streamData ) ) );
    return 1;
}

int CNetDBModuleEx::SaveUnionDatas( SQSaveUnionDataMsg *pMsg )
{
//     TraceInfo_C( "uniondata.txt", "开始保存结义数据 tick = %d count = %d size = %d！", pMsg->saveTime, pMsg->unionCount, pMsg->dwSendSize );
//     LPBYTE streamIter = ( LPBYTE )pMsg->streamData;
//     const DWORD count = pMsg->unionCount;
// 	for ( DWORD i = 0; i < count; i++ )
//     {
// 		SPlayerUnionBase *p = ( SPlayerUnionBase* )streamIter;
//         SaveUnionData( p, pMsg->saveTime );
// 		streamIter += sizeof( SPlayerUnionBase );
// 
//         TraceInfo_C( "uniondata.txt", "%d %s %s", pMsg->saveTime, p->title, p->suffx );
//     }

    return 1;

}
int CNetDBModuleEx::SaveUnionData( SPlayerUnionBase *p, DWORD tick )
{
//     QueryResult qr;
//     QueryBind< 19 > bind;
//     
//     for ( int i = 0; i < SPlayerUnionBase::NUMBER_MAX; i++ )
//     {
//         bind[ i ].BindULong( &p->roleid[ i ] );
//         bind[ 8 + i ].BindULong( &p->exp[ i ] );
//     }
// 
//     bind[ 16 ].BindULong( &tick );
//     bind[ 17 ].BindString( p->title, strlen( p->title ) );
//     bind[ 18 ].BindString( p->suffx, strlen( p->suffx ) );
// 
//     if ( ExecuteStmt( UPDATE_UNION, sizeof( UPDATE_UNION ), &qr, bind ) != 0 || qr.AffectedRows() != 1 )
//     {
//         qr.Reset();
// 
//         QueryBind< 19 > args;
// 
//         args[ 0 ].BindString( p->title, strlen( p->title ) );
//         args[ 1 ].BindString( p->suffx, strlen( p->suffx ) );
// 
//         for ( int i = 0; i < SPlayerUnionBase::NUMBER_MAX; i++ )
//         {
//             args[ 2 + i ].BindULong( &p->roleid[ i ] );
//             args[ 10 + i ].BindULong( &p->exp[ i ] );
//         }
// 
//         args[ 18 ].BindULong( &tick );
// 
//         if ( ExecuteStmt( INSERT_UNION, sizeof( INSERT_UNION ), &qr, args ) != 0 || qr.AffectedRows() != 1 )
//             return 0;
//     }

    return 1;
}

int CNetDBModuleEx::SavePlayerConfig( LPCSTR pAccount, DWORD dwServerID, LPCSTR PlayerConfig )
{
	 QueryResult qr;
	  char sql[1024];
	// 修改玩家电脑配置
	_snprintf( sql, sizeof sql, UPDATE_PLAYER_CONFIG, PlayerConfig, pAccount );	
	if ( ExecuteSql( sql, strlen( sql ), &qr ) != 0 || qr.AffectedRows() != 1 ){
		 m_errmsg.ShowMsg(EDEBUG, "保存账号为%s的玩家电脑配置失败", pAccount);
		return 0;
	}
	return 1;
}
int CNetDBModuleEx::GetAllPlayerRanklist(DWORD level,long &num,NewRankList * pNewRanklist)
{
	char sql[1024];
	_snprintf( sql, sizeof sql, SELECT_ALL_PLAYER_RANKLIST, level );
	sql[sizeof(sql) - 1 ] = 0;
	QueryResult qr;
	if ( ExecuteStmt( sql, strlen( sql ), &qr ) != 0 )
		return 3;//sql执行失败

	if (qr.AffectedRows() < 0)
	{
		return 4;//影响行数小于0
	}

	if (qr.AffectedRows() != 0)
	{
		NewRankList tmpbuf;
		Query::QueryBind< 1 > bind;
		bind[0].BindBlob( &tmpbuf, sizeof( tmpbuf ) );
		num = 0;
		while ( qr.FetchResult( bind ) == 0 )
		{
			// 特殊处理，数据错误（为空）；或索引号大于等于5；或已经读取过数据； 不再读入
			if ( bind[0].IsNull())
				continue;
			pNewRanklist[num++] = tmpbuf;
			if (num >= ALL_RANK_NUM)
			{
				return 1;
			}
		}
		return 1;//成功
	}
	

	return 2;//没有找到合适的数据;
}

//三国 2015.1.12 wk 邮件系统 存储过程调用##begin######
//系统发给全服,无玩家id,数据库sysmail接收
int CNetDBModuleEx::SendMail_sys(S2D_SendMailSYS_MSG*pBaseMsg)
//int CNetDBModuleEx::SendMail(DWORD senderid, LPCSTR pRoleName, DWORD emailtype, MailInfo &MailInfotData, DWORD serverid, DWORD expiretime)
{

	QueryBind<1> bind;
	bind[0].BindBlob(&pBaseMsg->m_MailInfo, sizeof(MailInfo));

	char sql[1024];
	_snprintf(sql, sizeof sql, CALL_SEND_MAIL, 0, "",  pBaseMsg->dw_emailtype, pBaseMsg->dw_serverid, pBaseMsg->dw_expiretime,"@out_ret");
	sql[sizeof(sql) - 1] = 0;

	QueryResult qr;
	if (ExecuteStmt(sql, strlen(sql), &qr, bind) != 0 || qr.AffectedRows() != 1)
	{
		if (qr.AffectedRows() == 0)
			m_errmsg.ShowMsg(EDEBUG, "SendMail1 failure：[%s]", sql);
		return 0;
	}
	return 1;
}

//系统发给玩家,有玩家id,数据库emailmsg接收
int CNetDBModuleEx::SendMail(S2D_SendMail_MSG*pBaseMsg)
//int CNetDBModuleEx::SendMail(DWORD senderid, LPCSTR pRoleName, DWORD emailtype, MailInfo &MailInfotData, DWORD serverid, DWORD expiretime)
{
	
	QueryBind<1> bind;
	bind[0].BindBlob(&pBaseMsg->m_MailInfo, sizeof(MailInfo));

	char sql[1024];
	if (pBaseMsg->dw_RevId>0)//用id发
	{
		_snprintf(sql, sizeof sql, INSERT_SEND_MAIL, pBaseMsg->dw_RevId, pBaseMsg->dw_emailtype, pBaseMsg->dw_serverid, pBaseMsg->dw_expiretime);
	}
	else //用名字发
	{
		_snprintf(sql, sizeof sql, CALL_SEND_MAILBYNAME, 0, pBaseMsg->name, pBaseMsg->dw_emailtype, pBaseMsg->dw_serverid, pBaseMsg->dw_expiretime, "@out_ret");
	}
	

	sql[sizeof(sql) - 1] = 0;


	QueryResult qr;
	if (ExecuteStmt(sql, strlen(sql), &qr, bind) != 0 || qr.AffectedRows() != 1)
	{
		if (qr.AffectedRows() == 0)
			m_errmsg.ShowMsg(EDEBUG, "SendMail failure：[%s]", sql);
		return 0;
	}
	return 1;
}

int CNetDBModuleEx::GetMailInfo_DB(const DNID clientDnid, SQ_GetMailInfo_MSG* pMsg)
{
	int sid = pMsg->m_dwSid;
	//收取系统全服邮件
	char sql_1[1024];
	_snprintf(sql_1, sizeof sql_1, CALL_GETMAILFROM_MAILSYS, sid, 0, "@out_ret");
	sql_1[sizeof(sql_1) - 1] = 0;
	QueryResult qr_1;
	if ((ExecuteStmt(sql_1, strlen(sql_1), &qr_1) != 0) || qr_1.AffectedRows() < 0)
		return 0;

	//取自己的
	char sql[1024];
	_snprintf(sql, sizeof sql, SELECT_MAILINFO_LIST, sid, MailReadState::getGoods);
	sql[sizeof(sql) - 1] = 0;
	QueryResult qr;
	if ((ExecuteStmt(sql, strlen(sql), &qr) != 0) || qr.AffectedRows() < 0)
		return 0;

	if (qr.AffectedRows() != 0)
	{
		SA_DBGETMailInfoList Msg;
		MailInfo tmpbuf;
		memset(&tmpbuf, 0, sizeof(tmpbuf));
		memset(&Msg.m_MailInfo, 0, sizeof(Msg.m_MailInfo));

		long tmpstate, tmpID;

		Query::QueryBind< 3 > bind;
		bind[0].BindBlob(&tmpbuf, sizeof(tmpbuf));
		bind[1].BindLong(&tmpID);
		bind[2].BindLong(&tmpstate);

		int num = 0;
		while (qr.FetchResult(bind) == 0)
		{
			if (bind[0].IsNull())
				continue;

			tmpbuf.readState = tmpstate;
			tmpbuf.id = tmpID;
			Msg.m_MailInfo[num++] = tmpbuf;
			if (num >= CONST_MAIL_MAX_NUM)
			{
				break;
			}
		}
		Msg.dnidClient = pMsg->_dnidClient;
		SendMessage(clientDnid, &Msg, sizeof(SA_DBGETMailInfoList));
		return 1;//成功
	}
	return 2;
}
//取奖励时同时设置邮件状态为已取
int CNetDBModuleEx::GetMailAwards_DB(const DNID clientDnid,struct SQ_GetAwards_MSG* pMsg)
{
	int id = pMsg->m_dwMailId;
	char sql[1024];
	_snprintf(sql, sizeof sql, SELECT_MAILINFO_ONE, id);
	sql[sizeof(sql) - 1] = 0;
	QueryResult qr;
	if ((ExecuteStmt(sql, strlen(sql), &qr) != 0) || qr.AffectedRows() < 0)
		return 0;

	if (qr.AffectedRows() != 0)
	{
		SA_DB_GetAwards_MSG Msg;
		MailInfo tmpbuf;
		memset(&tmpbuf, 0, sizeof(tmpbuf));

		long tmpstate;

		Query::QueryBind< 2 > bind;
		bind[0].BindBlob(&tmpbuf, sizeof(tmpbuf));
		bind[1].BindLong(&tmpstate);
		while (qr.FetchResult(bind) == 0)
		{
			if (bind[0].IsNull())
				continue;
			tmpbuf.readState = tmpstate;
			tmpbuf.id = id;
			Msg.m_MailInfo = tmpbuf;
			
		}
		Msg.m_dwSid = pMsg->m_dwSid;
		Msg.dnidClient = pMsg->dnidClient;
		SendMessage(clientDnid, &Msg, sizeof(SA_DB_GetAwards_MSG));
		
		//设置领取状态
		char _sql[1024];
		_snprintf(_sql, sizeof _sql, UPDATE_MAIL_ISREAD, MailReadState::getGoods, id);
		sql[sizeof(_sql) - 1] = 0;
		QueryResult _qr;
		if ((ExecuteStmt(_sql, strlen(_sql), &_qr) != 0) || _qr.AffectedRows() < 0)
			return 0;

		return 1;
	}

	return 1;
}

int CNetDBModuleEx::SetMailState_DB(struct SQ_SetMailState_MSG* pMsg)
{

	int id = pMsg->m_dwMailId;
	int setState = pMsg->m_dwState;
	char sql[1024];
	_snprintf(sql, sizeof sql, CALL_SETMAILSTATE, id, setState);
	sql[sizeof(sql) - 1] = 0;
	QueryResult qr;
	if ((ExecuteStmt(sql, strlen(sql), &qr) != 0) || qr.AffectedRows() < 0)
		return 0;
	return 1;
}
int CNetDBModuleEx::WritePointLog(SQPointLogMsg* pMsg)
{
	char sql[1024];
	_snprintf(sql, sizeof sql, CALL_POINT_LOG, pMsg->uid, pMsg->account, pMsg->itype, pMsg->diff_value, pMsg->finale_value, pMsg->detailType, pMsg->pInfo, pMsg->otherInfo);
	sql[sizeof(sql) - 1] = 0;
	QueryResult qr;
	if ((ExecuteStmt(sql, strlen(sql), &qr) != 0) || qr.AffectedRows() < 0)
		return 0;
	return 1;
}
int CNetDBModuleEx::test(string str)
{
	SQPointLogMsg msg;
	msg.diff_value = 111;
	msg.finale_value = 222;
	msg.uid = 111111;
	msg.itype = 1;
	_snprintf(msg.account, sizeof msg.account, "wwwwwwwwwwwwwwwwww");
	WritePointLog(&msg);
	//char s1ql[1024];
	//_snprintf(s1ql, sizeof s1ql, "call p_EmailGetItem(5,4,@a,@b,@c,@d,@e,@f)", str.c_str());
	//s1ql[sizeof(s1ql) - 1] = 0;
	//QueryResult q1r;
	//if ((ExecuteStmt(s1ql, strlen(s1ql), &q1r) != 0) || q1r.AffectedRows() < 0)
	//	return 0;


	//char sql[1024];
	////_snprintf(sql, sizeof sql, "SELECT @a, @b, @c, @d, @e, @f)", str.c_str());
	//_snprintf(sql, sizeof sql, "SELECT @f,@e", str.c_str());
	//sql[sizeof(sql) - 1] = 0;
	//QueryResult qr;
	//int a = ExecuteStmt(sql, strlen(sql), &qr);
	//if (a != 0) 
	//	return 0;
	//int b = qr.AffectedRows();
	//if (b < 0)
	//	return 0;
	//long _time = 0;
	//long _time2 = 0;
	//if (qr.AffectedRows() != 0)
	//{
	//	
	//	Query::QueryBind< 2 > bind;
	//	bind[0].BindLong(&_time);
	//	bind[1].BindLong(&_time2);

	//	while (qr.FetchResult(bind) == 0)
	//	{
	//		if (bind[0].IsNull())
	//			continue;
	//	}
	//}
	
	return 1;
}

int CNetDBModuleEx::GetFactions_New(const DNID dwSID, const __int32 nServerID)
{
	if (nServerID <= 0)
		return 0;

	TraceInfo_C("faction.txt", "处理读取帮派消息【%d】！", nServerID);

	char sql[1024];
	_snprintf(sql, sizeof sql, GET_FACTIONLIST, nServerID, nServerID);
	sql[sizeof(sql) - 1] = 0;
	QueryResult qr;
	if ((ExecuteStmt(sql, strlen(sql), &qr) != 0) || qr.AffectedRows() < 0)
		return 0;

	//SaveFactionData *stFaction = (SaveFactionData*)tmpBuffer;

	SaveFactionData_New  *stFaction = (SaveFactionData_New*)tmpBuffer;
	SAGetFactionMsg &factionMsg = *new (msgBuffer)SAGetFactionMsg();
	factionMsg.isLast = FALSE;
	factionMsg.dwSendSize = 0;
	factionMsg.nFactions = 0;

	SaveFactionData_New sfd;
	memset(&sfd, 0, sizeof(sfd));

	Query::QueryBind< 1 > bind;
	bind[0].BindBlob(&sfd, sizeof(SaveFactionData_New));

	long idx = 0;
	while (qr.FetchResult(bind) == 0)
	{
		assert(!bind[0].IsNull());
		assert(bind[0].Length() <= sizeof(SaveFactionData_New));

		// 数据解压缩，直接写到目标slot里边!
		void *unpakData = &stFaction[idx];
		size_t unpakSize = sizeof(SaveFactionData_New);
		BOOL ck = TryDecoding(&sfd, bind[0].Length(), unpakData, unpakSize);

	/*	SaveFactionData_New *temp = (SaveFactionData_New*)unpakData;*/
		size_t dataSize = sizeof(SaveFactionData_New);
		
		// 数据大小对不上！
		if (unpakSize != dataSize)
		{
			TraceInfo_C("faction.txt", "读取帮派【%d】，解压失败！", idx);
			continue;
		}

		// 如果不是解压数据，则需要将数据拷贝到目标slot里边
		if (!ck)
			memcpy(&stFaction[idx], &sfd, dataSize);

// 		TraceInfo_C("faction.txt", "读取帮派成功【%s】【%d】！",
// 			stFaction[idx].stFaction.szFactionName, stFaction[idx].stFaction.byMemberNum);

		// memset( reinterpret_cast< char* >( &stFaction[ idx ] ) + dataSize, 0, sizeof( sfd ) - dataSize );
		idx++;
	}

	// copy from old netdbmoudule.cpp
	SaveFactionData_New *iter = stFaction;
	LPBYTE streamIter = (LPBYTE)factionMsg.streamFaction;
	memset(streamIter, 0, sizeof(SaveFactionData_New) * MAX_SENDFACTIONS);

	DWORD nFactions = 0;

	SaveFactionData_New tempData ;
	for (int n = 0; n < idx; n++, iter++)
	{
		//memset(tempData, 0, sizeof(SaveFactionData_New));
		memcpy(&tempData, iter, sizeof(SaveFactionData_New));

		memset(tempData.sfactioninfo.m_factionmember, 0, sizeof(FactionMember)*MAX_FACTION_NUMBER);
		memset(tempData.sjoinlist, 0, sizeof(FactionMember)*MAX_JoinListCount);


		size_t nlenth = iter->sfactioninfo.CurMemberNum;
		for (int i = 0; i < nlenth; i++)
		{
			tempData.sfactioninfo.m_factionmember[i] = iter->sfactioninfo.m_factionmember[i];
		}

		nlenth = iter->sfactioninfo.QuestJoinNum;
		for (int i = 0; i < nlenth; i++)
		{
			tempData.sjoinlist[i] = iter->sjoinlist[i];
		}

		memcpy(streamIter, &tempData, sizeof(SaveFactionData_New));

		streamIter += sizeof(SaveFactionData_New);

		nFactions++;
		if (idx > MAX_SENDFACTIONS && nFactions  >= MAX_SENDFACTIONS) //已经是最大数量
		{
			factionMsg.nFactions = MAX_SENDFACTIONS;
			SendMessage(dwSID, &factionMsg, sizeof(SAGetFactionMsg));
			memset(streamIter, 0, sizeof(SaveFactionData_New) * MAX_SENDFACTIONS);     //重新清0

			rfalse(2, 1, "GetFactions_New  %d", factionMsg.nFactions);
			streamIter = (LPBYTE)factionMsg.streamFaction;
			factionMsg.nFactions = 0;
			nFactions = 0;
		}
		
	}

	// 最后这个消息用于通知登陆服务器读取正常完成！所以就算没有读到任何帮派，也需要发送该消息
	factionMsg.isLast = TRUE;
	factionMsg.nFactions = nFactions;
	SendMessage(dwSID, &factionMsg, sizeof(SAGetFactionMsg));
	rfalse(2, 1, "GetFactions_New  %d",nFactions);
	return 1;
}

int CNetDBModuleEx::DeleteFaction(const DNID dwSID,struct SQDeleteFaction *pMsg)
{
	SADeleteFaction sDeleteFaction;
	sDeleteFaction.bresult = TRUE;
	sDeleteFaction.nServerId = pMsg->nServerId;
	sDeleteFaction.dFactionID = pMsg->dFactionID;

	DWORD dserverid = pMsg->nServerId;
	DWORD  Factionid = pMsg->dFactionID;

	TraceInfo_C("faction.txt", "DeleteFaction【%d】！", Factionid);

	char sql[1024];
	_snprintf(sql, sizeof sql, DELETE_FACTION, dserverid, Factionid);
	sql[sizeof(sql) - 1] = 0;
	QueryResult qr;
	if ((ExecuteStmt(sql, strlen(sql), &qr) != 0) || qr.AffectedRows() < 0)
	{
		sDeleteFaction.bresult = FALSE;
	}
	
	SendMessage(dwSID, &sDeleteFaction, sizeof(SADeleteFaction));
	return 1;
}

int CNetDBModuleEx::SaveScriptData(struct SQScriptData *pMsg)
{
	BYTE *pBuffer = (BYTE*)&pMsg->wLenth + sizeof(WORD);

	LPVOID pakBuf = tmpBuffer;
	size_t pakSize = sizeof(tmpBuffer);
	if (!TryEncoding(pBuffer, pMsg->wLenth, pakBuf, pakSize))
	{
		rfalse("TryEncoding Faile");
	}

	QueryBind< 1 > args;
	/*args[0].BindBlob(&stFaction, sizeof(SaveFactionData_New));*/
	args[0].BindBlob(pakBuf, pakSize);

	QueryResult qr;
	if (ExecuteStmt(SAVE_SCRIPTDATA, sizeof(SAVE_SCRIPTDATA), &qr, args) != 0)
	{
		return 0;
	}
	return 1;
}

int CNetDBModuleEx::GetScriptData(const DNID dwSID,struct SQGetScriptData *pMsg)
{
	char sql[1024];
	_snprintf(sql, sizeof sql, GET_SCRIPTDATA);
	sql[sizeof(sql) - 1] = 0;
	QueryResult qr;
	if ((ExecuteStmt(sql, strlen(sql), &qr) != 0) || qr.AffectedRows() < 0)
		return 0;

	LPVOID pakBuf = tmpBuffer;
	size_t pakSize = sizeof(tmpBuffer);

	Query::QueryBind< 1 > bind;
	bind[0].BindBlob(pakBuf, pakSize);

	BYTE   *pBuffer = new BYTE[20480];

	long idx = 0;
	while (qr.FetchResult(bind) == 0)
	{
		// 数据解压缩，直接写到目标slot里边!
		void *unpakData = pBuffer;
		size_t unpakSize = 20480;
		BOOL ck = TryDecoding(pakBuf, bind[0].Length(), unpakData, unpakSize);
		if (!ck)
		{
			memcpy(unpakData, pakBuf, bind[0].Length());
		}

		WORD  datalenth = sizeof(SAGetScriptData) + bind[0].Length();
		SAGetScriptData   sqscriptdata;
		SAGetScriptData   *pscriptdata = (SAGetScriptData*)malloc(datalenth);
		int copylenth = (int)&sqscriptdata.wLenth - (int)&sqscriptdata;
		if (copylenth > 0)
		{
			memcpy(pscriptdata, &sqscriptdata, copylenth);
			pscriptdata->wLenth = bind[0].Length();
			pscriptdata->pBuffer = (BYTE*)&pscriptdata->wLenth + sizeof(WORD);
			memcpy(pscriptdata->pBuffer, unpakData, bind[0].Length());
			SendMessage(dwSID, pscriptdata, datalenth);
			return 1;
		}
	}
	return 0;
}

int CNetDBModuleEx::GetFactions_Data(const DNID dwSID, const __int32 nServerID)
{
	if (nServerID <= 0)
		return 0;

	TraceInfo_C("faction.txt", "处理读取帮派消息【%d】！", nServerID);

	char sql[1024];
	_snprintf(sql, sizeof sql, GET_FACTIONLIST, nServerID, nServerID);
	sql[sizeof(sql) - 1] = 0;
	QueryResult qr;
	if ((ExecuteStmt(sql, strlen(sql), &qr) != 0) || qr.AffectedRows() < 0)
		return 0;

	SaveFactionData_Lua  *stFaction = (SaveFactionData_Lua*)tmpBuffer;

	//SAGetFactionDataMsg factionMsg;

	SAGetFactionDataMsg &factionMsg = *new (msgBuffer)SAGetFactionDataMsg();
	factionMsg.isLast = FALSE;
	factionMsg.dwSendSize = 0;
	factionMsg.nFactions = 0;

	SaveFactionData_Lua sfd;
	memset(&sfd, 0, sizeof(sfd));

	Query::QueryBind< 1 > bind;
	bind[0].BindBlob(&sfd, sizeof(SaveFactionData_Lua));

	long idx = 0;
	while (qr.FetchResult(bind) == 0)
	{
		assert(!bind[0].IsNull());
		assert(bind[0].Length() <= sizeof(SaveFactionData_Lua));

		// 数据解压缩，直接写到目标slot里边!
		void *unpakData = &stFaction[idx];
		size_t unpakSize = sizeof(SaveFactionData_Lua);
		BOOL ck = TryDecoding(&sfd, bind[0].Length(), unpakData, unpakSize);

		/*	SaveFactionData_New *temp = (SaveFactionData_New*)unpakData;*/
		size_t dataSize = sizeof(SaveFactionData_Lua);

		// 数据大小对不上！
		if (unpakSize != dataSize)
		{
			TraceInfo_C("faction.txt", "读取帮派【%d】，解压失败！", idx);
			continue;
		}

		// 如果不是解压数据，则需要将数据拷贝到目标slot里边
		if (!ck)
			memcpy(&stFaction[idx], &sfd, dataSize);

		// 		TraceInfo_C("faction.txt", "读取帮派成功【%s】【%d】！",
		// 			stFaction[idx].stFaction.szFactionName, stFaction[idx].stFaction.byMemberNum);

		// memset( reinterpret_cast< char* >( &stFaction[ idx ] ) + dataSize, 0, sizeof( sfd ) - dataSize );
		idx++;
	}

	// copy from old netdbmoudule.cpp
	SaveFactionData_Lua *iter = stFaction;
	LPBYTE streamIter = (LPBYTE)factionMsg.streamFaction;
	memset(streamIter, 0, sizeof(SaveFactionData_Lua) * MAX_SENDFACTIONDATA);

	DWORD nFactions = 0;

	SaveFactionData_Lua tempData;
	for (int n = 0; n < idx; n++, iter++)
	{
		//memset(tempData, 0, sizeof(SaveFactionData_New));
		memcpy(&tempData, iter, sizeof(SaveFactionData_Lua));

		if (tempData.FactionID == 0)
		{
			rfalse(2, 1, ".FactionID == 0");
			break;
		}

		memcpy(streamIter, &tempData, sizeof(SaveFactionData_Lua));

		streamIter += sizeof(SaveFactionData_Lua);

		nFactions++;
		if (idx > MAX_SENDFACTIONDATA && nFactions >= MAX_SENDFACTIONDATA) //已经是最大数量
		{
			factionMsg.nFactions = MAX_SENDFACTIONDATA;
			SendMessage(dwSID, &factionMsg, sizeof(SAGetFactionDataMsg));
			memset(streamIter, 0, sizeof(SaveFactionData_Lua) * MAX_SENDFACTIONDATA);     //重新清0

			rfalse(2, 1, "GetFactions_New  %d", factionMsg.nFactions);
			streamIter = (LPBYTE)factionMsg.streamFaction;
			factionMsg.nFactions = 0;
			nFactions = 0;
		}

	}

	// 最后这个消息用于通知登陆服务器读取正常完成！所以就算没有读到任何帮派，也需要发送该消息
	factionMsg.isLast = TRUE;
	factionMsg.nFactions = nFactions;
	SendMessage(dwSID, &factionMsg, sizeof(SAGetFactionDataMsg));
	rfalse(2, 1, "GetFactions_Data  %d", nFactions);
	return 1;
}

//三国 2015.1.12 wk 邮件系统 存储过程调用##begin######