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
    // ���ݹ�С������ѹ������
    if ( src_size < 32 )
    {
        dst_size = src_size;
        dst_data = ( void* )src_data;
        return FALSE;
    }

	dst_size = LzwEncoding( src_data, src_size, ( char* )dst_data + 8, dst_size - 8 );
	if ( dst_size <= 0 || dst_size >= ( src_size - 16 ) )
    {
        // ѹ��ʧ�ܣ�����ѹ����������������󣩣�ֱ�ӷ���ԭʼ����
        dst_data = ( void* )src_data;
        dst_size = src_size;
        return FALSE;
	}

    // ѹ���ɹ������ѹ����ǣ�
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

    // ����Ҫ��ѹ�������
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
	rfalse("��ȡ��ɫ�б�");
    assert(pAccount && dwServerID && pChartListData);
    assert(( pChartListData[0].m_dwStaticID | pChartListData[1].m_dwStaticID | pChartListData[2].m_dwStaticID | pChartListData[3].m_dwStaticID | pChartListData[4].m_dwStaticID ) == 0);

    char sql[1024];

    // ����������ܷ��� -3��-2��-1��0��1����ֻ�� -3 ʱ���Ǳ���������������
    int result = UpdateAccountLock(pAccount, dwServerID, TRUE);
    if (result == -1) return SAGetCharacterListMsg::RET_ERROR_INFO;
    if (result == -3) return SAGetCharacterListMsg::RET_LOCKEDOTHER;
	
	//����˺��Ƿ񱻶��ᣬ�������ڵ��˺ŷ�����û�е�ͨ��������ʱ�ô���������������ݿ�����������˺ŵ���Ч��
	_snprintf(sql, sizeof sql, CHECK_ACCOUNT_FROZEN, pAccount);
	sql[sizeof(sql) - 1] = 0;
	
	QueryResult qr;
	// ���ִ��ʧ�ܣ���ô����-1
	if (ExecuteStmt(sql, strlen(sql), &qr) != 0)
		return -1;
	// ���û��Ӱ�������������û�ж�Ӧ���˺����ݣ�Ӧ�÷���-2
	if (qr.AffectedRows() != 0)
	{
		// ��ȡ��ɫ�������ݣ����ڼ���˺��Ƿ񱻶���
		unsigned long sid = 0;
		QueryBind<1 > bind;
		bind[0].BindULong(&sid);
		if (qr.FetchResult(bind) != 0)
			return -1;
		if (sid > 0)
			return SAGetCharacterListMsg::RET_ERROR_INFO;
	}
	//if (result == -2) //2014/11/21 Ŀǰ�������汾���ܴ����˺ŵ�����WareHouse�˺������ֶ��������������޸�������ֱ�ӷ���
	//	return SAGetCharacterListMsg::RET_ERROR_INFO;


	if (result == -2)
	{
		// �������½���ɫ�����Ա�Ȼû�����ݣ�ֱ�ӷ��ؽ��
		_snprintf(sql, sizeof sql, INSERT_WAREHOUSE, pAccount, dwServerID, pAccount);
		sql[sizeof(sql) - 1] = 0;
		if (ExecuteSql(sql, strlen(sql), &qr) != 0 || qr.AffectedRows() != 1)
			return SAGetCharacterMsg::ERC_FAIL;
		SFixData PlayerData;
		memset(&PlayerData, 0, sizeof(PlayerData));
		PlayerData.m_version = SFixData::GetVersion();
		if (CreatePlayerData(pAccount, 1, dwServerID, PlayerData) != 1) // ����1��ʾ�ߴ�����ɫ�������ݳɹ�
			return SAGetCharacterMsg::ERC_FAIL;

		// �޸ĵ�¼ʱ��
		_snprintf(sql, sizeof sql, UPDATE_PROPERTY_LOGINTIME, PlayerData.m_dwStaticID, pAccount, dwServerID);
		qr.Reset();
		if (ExecuteSql(sql, strlen(sql), &qr) != 0 || qr.AffectedRows() != 1)
			m_errmsg.ShowMsg(EDEBUG, "���½�ɫ����ʱ��ʧ��%s,%d ", mysql_stmt_error(m_pstmt), PlayerData.m_dwStaticID);

		return SAGetCharacterMsg::ERC_SUCCESS;
	}

    // ��ʱ���ݿ��л�û������ʺţ����չ�����Ҫ�û��ڳɹ�������ݵ�ͬʱ����������������û�У�����Ҫ�½�һ��
    //QueryResult qr;
    //if ( result == -2 ) 
    //{
    //    // �������½���ɫ�����Ա�Ȼû�����ݣ�ֱ�ӷ��ؽ��
    //    _snprintf( sql, sizeof sql, INSERT_WAREHOUSE, pAccount, dwServerID, pAccount );
    //    sql[ sizeof( sql ) - 1 ] = 0;
    //    if ( ExecuteSql( sql, strlen( sql ), &qr ) != 0 || qr.AffectedRows() != 1 )
    //        return SAGetCharacterListMsg::RET_ERROR_INFO;
	//
    //    return SAGetCharacterListMsg::RET_SUCCESS;
    //}
	//
    //// �Ѿ��ɹ���ɷ�������������ʽ��ʼ��ȡ��ɫ�б�
    //_snprintf(sql, sizeof sql, SELECT_PROPERTY_ROLELIST, pAccount, pAccount, dwServerID);
    //sql[sizeof(sql) - 1 ] = 0;
    //if ( ExecuteStmt( sql, strlen( sql ), &qr ) != 0 || qr.AffectedRows() < 0)
    //    return SAGetCharacterListMsg::RET_ERROR_INFO;
	//
    //assert(qr.AffectedRows() <= 5);
	//
    //// ���һ����ɫ��û�У���ô��ߵ�ȡ���ݾͲ���Ҫ�ˣ�������Ҫ�ں���ͷ���Դ
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
    //        // ���⴦�����ݴ���Ϊ�գ����������Ŵ��ڵ���5�����Ѿ���ȡ�����ݣ� ���ٶ���
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

// ���ش����ɹ���Ľ�ɫ�����ݿ��ϵ�ΨһID��
int CNetDBModuleEx::CreateName(LPCSTR pAccount, WORD wIndex, DWORD dwServerID, LPCSTR pRoleName)
{
    assert((pAccount != NULL) && (pRoleName != NULL));

    if (wIndex <= 0 || wIndex > 5)
        return 0;
	char *AnsiRoleName = g_Utf8ToAnsi(pRoleName);
    // ����˺��Ƿ���ڣ���������ھ���Ҫ�½�һ��
    // �����߼��޸��ˣ���Ϊһ��ʼ��ȡ��ɫ�б�ʱ�����û���˺�����Զ�����һ��

    // ��� serverId ��ͬʱ�������½�ɫ
    char sql[1024];
	string roleName(pRoleName);
	
	//20150619 wk ��ȡǰ31����Ĭ������
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

    // ��ɫ������ɣ���ȡ��ɫ��ID
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

// ���ﷵ��0��ĩ֪����
int CNetDBModuleEx::CreatePlayerData(LPCSTR pAccount,WORD wIndex, DWORD dwServerID, SFixData &PlayerData)
{
    if (wIndex < 1 || wIndex > 5)
        return 0;

    // ������ɫ���֣�����StaticID
	int staticid = CreateName(pAccount, wIndex, dwServerID, pAccount);
    if (staticid == 0)
        return SACreatePlayerMsg::ERC_NAMETAKEN;

    PlayerData.m_dwStaticID = staticid;
	PlayerData.m_bNewPlayer = true;
	PlayerData.m_byStoreFlag = SFixData::ST_LOGIN;
	PlayerData.m_EnduranceData.m_dwEndurance = 90;
	PlayerData.m_EnduranceData.m_dwRemainingSeconds = 0;
	PlayerData.m_dwLevel = 1;
	
	///-----------�½��佫���ݳ�ʼ��
	SHeroData tempHeroData;
	memset(&tempHeroData, 0, sizeof(SHeroData));
	//װ����ʼ��
	SHeroEquipment tempEquip;
	//���ܳ�ʼ��
	memset(&tempEquip, 0, sizeof(SHeroEquipment));
	SHeroSkillInfo tempSkill;
	memset(&tempSkill, 0, sizeof(SHeroSkillInfo));
	for (int i = 0; i < EQUIP_MAX; i++)
		memcpy(&tempHeroData.m_Equipments[i], &tempEquip, sizeof(SHeroEquipment));
	for (int i = 0; i < MAX_SKILLNUM; i++)
		memcpy(&tempHeroData.m_SkillInfoList[i], &tempSkill, sizeof(SHeroSkillInfo));
	//�½��佫���Գ�ʼ��
	for (int i = 0; i < MAX_HERO_NUM; i++)
		memcpy(&PlayerData.m_HeroList[i], &tempHeroData, sizeof(SHeroData));

	///------------�½��ı�����ʼ��
	memset(&PlayerData.m_ItemList, 0, sizeof(SSanguoItem) * MAX_ITEM_NUM);

	///------------�����ݳ�ʼ��
	PlayerData.m_BlessData.m_dwMoneyFreeBlessCount = 5;
	PlayerData.m_BlessData.m_dwDiamondFreeBlessCount = 1;
	PlayerData.m_BlessData.m_dwLastMoneyFreeBlessDate = time(nullptr) - 600000;
	PlayerData.m_BlessData.m_dwLastDiamondFreeBlessDate = time(nullptr)  - 17280000 * 10;
	PlayerData.m_BlessData.m_dwIsFirstDiamondBless = 1;
	PlayerData.m_BlessData.m_dwIsFirstMoneyBless = 1;
	// ������ɫʱ����Ӧ�ö����ֿ�
    return SavePlayerData(pAccount, dwServerID, PlayerData, NULL); 
	
}


/*
	��ʼ�������������Ϸ����,
	�˴�����ת���࣬C++���Զ�
	����ָ��ƫ��
*/
// static void InitXiaYiShiJieData(SPlayerXiaYiShiJie* data)
// {
// 	memset(data,0,sizeof(SPlayerXiaYiShiJie));
// 	for (int i = 0;i < SPlayerXiaYiShiJie::ATTACK_DEFENSE_MAX;++i)
// 	{
// 		// ���Դ��롣Ŀǰ����ʱ��ʼ��Ϊ0
// 		//data->m_AttackAndDefense[ i ].m_wLevel = 0;
// 		//data->m_AttackAndDefense[ i ].m_wMastery = 0;
// 	}
// }

/*
	�汾3���汾4��ת������
		�汾4����������İ汾
*/
static bool ConverVersion3ToVersion4(SFixPlayerDataBuf* fpdb)
{
//	InitXiaYiShiJieData(fpdb);
	fpdb->m_version = SFixPlayerDataBuf::GetVersion();
	return true;
}

// ��ȡ��ҵĽ�ɫ������Ϣ
int CNetDBModuleEx::GetPlayerData(LPCSTR pAccount, DWORD dwServerID, BYTE byIndex, SFixData &PlayerData)
{
	rfalse("��ȡ��ɫ����");
    if (!pAccount || byIndex <= 0 || byIndex > 5)
        return SAGetCharacterMsg::ERC_FAIL;
	char sql[1024];
	QueryResult qr;
	
	// ����������ܷ��� -3��-2��-1��0��1����ֻ�� -3 ʱ���Ǳ���������������
	int result = UpdateAccountLock(pAccount, dwServerID, TRUE);
	if (result == -1) return SAGetCharacterMsg::ERC_FAIL;
	if (result == -3) return SAGetCharacterMsg::ERC_LOCKEDOTHER;
	// ��ʱ���ݿ��л�û������ʺţ����չ�����Ҫ�û��ڳɹ�������ݵ�ͬʱ����������������û�У�����Ҫ�½�һ��
	//if (result == -2)
	//{
	//	// �������½���ɫ�����Ա�Ȼû�����ݣ�ֱ�ӷ��ؽ��
	//	_snprintf(sql, sizeof sql, INSERT_WAREHOUSE, pAccount, dwServerID, pAccount);
	//	sql[sizeof(sql) - 1] = 0;
	//	if (ExecuteSql(sql, strlen(sql), &qr) != 0 || qr.AffectedRows() != 1)
	//		return SAGetCharacterMsg::ERC_FAIL;
	//	PlayerData.m_version = SFixData::GetVersion();
	//	if (CreatePlayerData(pAccount, byIndex, dwServerID, PlayerData) != 1) // ����1��ʾ�ߴ�����ɫ�������ݳɹ�
	//		return SAGetCharacterMsg::ERC_FAIL;
	//
	//	// �޸ĵ�¼ʱ��
	//	_snprintf(sql, sizeof sql, UPDATE_PROPERTY_LOGINTIME, PlayerData.m_dwStaticID, pAccount, dwServerID);
	//	qr.Reset();
	//	if (ExecuteSql(sql, strlen(sql), &qr) != 0 || qr.AffectedRows() != 1)
	//		m_errmsg.ShowMsg(EDEBUG, "���½�ɫ����ʱ��ʧ��%s,%d ", mysql_stmt_error(m_pstmt), PlayerData.m_dwStaticID);
	//
	//	return SAGetCharacterMsg::ERC_SUCCESS;
	//}
	/*
	//2014/11/21 Ŀǰ�������汾���ܴ����˺ŵ�����, �������û��PROPERTY������ֱ�Ӵ���
	_snprintf(sql, sizeof sql, CHECK_PROPERTY_EXIST, pAccount);
	sql[sizeof(sql) - 1] = 0;
	if (ExecuteStmt(sql, strlen(sql), &qr) != 0 || qr.AffectedRows() > 1)
		return SAGetCharacterMsg::ERC_FAIL;
	if (qr.AffectedRows() == 0) //û��propertyֱ�Ӵ���
	{
		PlayerData.m_version = SFixData::GetVersion();
		if (CreatePlayerData(pAccount, byIndex, dwServerID, PlayerData) != 1) // ����1��ʾ�ߴ�����ɫ�������ݳɹ�
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
        TraceInfo_C( "modify.txt", "������ݱ��޸ģ�[acc = %s][name = %s][verify = %d]", pAccount, name, verify );
        if ( verify != 0 )
            return SAGetCharacterMsg::ERC_FAIL;
    }

    void *unpakData = tmpBuffer;
    size_t unpakSize = sizeof( tmpBuffer );

    TryDecoding( &databuf, bind[2].Length(), unpakData, unpakSize );

    // ������������ݼ�����ݵ���Ч��!
	SFixData *fpdb = (SFixData*)unpakData;
//    if ( unpakSize != ( fpdb->m_version >> 16 ) )
  //  {
    //    OutputDebugString( "��ѹ������ݴ�С��ƥ�䣡\r\n" );
      //  return SAGetCharacterMsg::ERC_FAIL;
    //}

    // ����Ƿ��ȡ������δ����ѹ�����ԭʼ���ݣ��������������ڰ汾����ʱ�����쳣��
    if ( unpakData != tmpBuffer )
    {
        memcpy( tmpBuffer, unpakData, unpakSize );
        unpakData = tmpBuffer;
    }

    // ����Ϊ�汾�����߼��������3����������ݰ汾������
	
	/*
    // �汾1�������汾2������
    if ( fpdb->m_version == typedef_v1::SFixPlayerDataBuf::GetVersion() )
    {
        typedef_v1::SFixPlayerDataBuf *ptr = ( typedef_v1::SFixPlayerDataBuf* )tmpBuffer;
        typedef_v2::SFixPlayerDataBuf *dest = ( typedef_v2::SFixPlayerDataBuf* )tmpBufEx;

        // ���汾�������ɰ汾�������°汾
        // ͨ���������ݱȶԣ�Ĭ�ϸö��߼���Ӧ�ò������룡
        static const int checkSize = offsetof( typedef_v1::SFixProperty, typedef_v1::SFixProperty::m_dwStaticID );
        static const int checkTail = sizeof( typedef_v1::SFixProperty ) - checkSize;
        assert( checkSize == offsetof( typedef_v2::SFixProperty, typedef_v2::SPlayerDataExpand::masterName ) );
        assert( checkTail == sizeof( typedef_v2::SFixProperty ) - offsetof( typedef_v2::SFixProperty, typedef_v2::SFixProperty::m_dwStaticID ) );

        // Ĭ����������
        memset( tmpBufEx, 0, sizeof( tmpBufEx ) );

        // ֱ�ӿ���ǰһ������
        memcpy( tmpBufEx, tmpBuffer, checkSize ); 

        // �����3���ؼ����ݿ���������
        dest->m_dwStaticID = ptr->m_dwStaticID;
        dest->m_byStoreFlag = ptr->m_byStoreFlag;
        memcpy( dest->m_szUPassword, ptr->m_szUPassword, sizeof( dest->m_szUPassword ) );

        // ����汾�ţ�
        dest->m_version = dest->GetVersion();

        // ������tmpBuffer��
        memcpy( tmpBuffer, tmpBufEx, sizeof( tmpBuffer ) );
    }

    // �汾2�������汾3������
    if ( fpdb->m_version == typedef_v2::SFixPlayerDataBuf::GetVersion() )
    {
        typedef_v2::SFixPlayerDataBuf *ptr = ( typedef_v2::SFixPlayerDataBuf* )tmpBuffer;
        typedef_v3::SFixPlayerDataBuf *dest = ( typedef_v3::SFixPlayerDataBuf* )tmpBufEx;

        // ���汾�������ɰ汾�������°汾
        // ͨ���������ݱȶԣ�Ĭ�ϸö��߼���Ӧ�ò������룡
        static const int checkHead = offsetof( typedef_v2::SFixProperty, typedef_v2::SPlayerGM::m_wGMLevel );
        static const int checkTail = sizeof( typedef_v2::SFixProperty ) - checkHead;
        static const int checkBody = offsetof( typedef_v3::SFixProperty, typedef_v3::SPlayerGM::m_wGMLevel );
        assert( checkHead == offsetof( typedef_v3::SFixProperty, typedef_v3::SPlayerTasks::PlayerTask ) + 
            sizeof( typedef_v3::SPlayerTask[typedef_v2::MAX_TASK_NUMBER] ) );
        assert( checkTail == sizeof( typedef_v3::SFixProperty ) - checkBody );

        // Ĭ����������
        memset( tmpBufEx, 0, sizeof( tmpBufEx ) );

        // ֱ�ӿ���ǰһ������
        memcpy( tmpBufEx, tmpBuffer, checkHead ); 

        // ��������ݿ���������
        memcpy( tmpBufEx + checkBody, tmpBuffer + checkHead, checkTail ); 

        // ����汾�ţ�
        dest->m_version = dest->GetVersion();

        // ������tmpBuffer��
        memcpy( tmpBuffer, tmpBufEx, sizeof( tmpBuffer ) );
    }

	//�汾3����������İ汾4
	if (fpdb->m_version == typedef_v3::SFixPlayerDataBuf::GetVersion())
	{
		ConverVersion3ToVersion4(fpdb);
	}
	//*/

    // ���ϵ�ǰ�汾�ŵ�����£���ֵ��ɹ�����
    if (fpdb->m_version == SFixData::GetVersion())
    {
        PlayerData = *fpdb;
    }
    else
    {
		extern void AddInfo(LPCSTR Info);
		AddInfo( "����һ����Ч�İ汾�����ݣ�\r\n" );
		//MessageBox( 0,"����һ����Ч�İ汾�����ݣ�\r\n",pAccount,0);
        return SAGetCharacterMsg::ERC_FAIL;
    }

    if (strncmp(name, fpdb->m_Name, sizeof(name)) != 0)
        TraceInfo_C( "modify.txt", "��ҽ�ɫ�����޸ģ�[%s] -> [%s]", fpdb->m_Name, name);

    assert( sid != 0 );
	if (sid != fpdb->m_dwStaticID)
	{
		AddInfo( "��ҶԷ�������ȺΨһ��̬��ID��ƥ�䣡\r\n" );
		//MessageBox( 0,"����һ����Ч�İ汾�����ݣ�\r\n",pAccount,0);
		return SAGetCharacterMsg::ERC_FAIL;
	}
    //assert( sid == fpdb->m_dwStaticID );
    assert( bind[1].Length() <= 10 );

    strncpy( PlayerData.m_Name, name, sizeof( PlayerData.m_Name ) );
    PlayerData.m_Name[CONST_USERNAME-1] = 0;
    PlayerData.m_dwStaticID = sid;

    // �޸ĵ�¼ʱ��
    _snprintf( sql, sizeof sql, UPDATE_PROPERTY_LOGINTIME, PlayerData.m_dwStaticID, pAccount, dwServerID );	
    qr.Reset();
    if ( ExecuteSql( sql, strlen( sql ), &qr ) != 0 || qr.AffectedRows() != 1 )
        m_errmsg.ShowMsg( EDEBUG, "���½�ɫ����ʱ��ʧ��%s,%d ",mysql_stmt_error(m_pstmt), PlayerData.m_dwStaticID );

    return SAGetCharacterMsg::ERC_SUCCESS;
}
// ��ȡ���ݿ��ɫѹ��������ݲ�����
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
	TraceInfo_C("roleinfo.txt","��ɫ��Ϣ��¼");
	TraceInfo( "roleinfo.txt", "�˺���	��ɫ��	��ɫindex	�������ʱ��ʱ��	�������ʱ��IP	����ʱ��	�Ա�	�������ʾͼƬ����	����	��ǰ��ʾ��ͷ��	��ģ��	ͷ��ģ��	�ȼ�	ת������	��������ID	��ҵ���ʾ��Ϣ	�󶨻���	�ǰ󶨻���	��ȯ	��õ���ȯ����	Ԫ��	Ԫ�������ܳ�����	Ԫ�������ܽ�����	Ԫ��������˰����	��ҳ�ֵ��������	��ǰ����	��ǰ����	��ǰ����	��ǰ����	��ǰ����	����	����	����	����	��ҵ�ǰ����	���ʣ�����	����	PKֵ	����ֵ	���ͼ���ֵ	��ǰ��ͼID	��ǰ��������x	��ǰ��������y	��ǰ��������z	����	��չ�����������	��չ�����������	�Ƿ������˲ֿ�	���͸��Ӽ������	��ҵ�һ�ε�½��Ϸ��������ۼ�ʱ��	�Ƿ�ֹͣ��ʱ	�Ѿ���ȡ�������������	��ǰ�ķ�װģʽ����ʱװ������ͨ");
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

// �����ɫ�����ݳɹ����ط�0
int CNetDBModuleEx::SavePlayerData(LPCSTR pAccount, DWORD dwServerID, SFixData &bdata, SWareHouses *wdata)
{
    assert( pAccount != NULL );

    // �����ʱ������Ҫ���¿������ݣ������е��鷳�������ܷ��Ż�
    SCharListData snapshot;
    memset(&snapshot, 0, sizeof snapshot);
    memcpy(snapshot.m_szName, bdata.m_Name, sizeof bdata.m_Name);

    // ����ѹ��
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

	DWORD tollgate = bdata.m_dwLatestBattleFileID;//�ؿ�
	bind[5].BindULong(&tollgate);
	DWORD viplv = 0;//vip�ȼ�
	bind[6].BindULong(&viplv);
	DWORD guideID = bdata.m_NewbieGuideData.curGuide;//�������� ǧλΪ��������,����Ϊ��һ��
	bind[7].BindULong(&guideID);


    char sql[1024];
    _snprintf(sql, sizeof sql, UPDATE_PROPERTY_ONLYBUF, bdata.m_dwStaticID, pAccount, dwServerID);
    sql[sizeof( sql ) - 1] = 0;

    QueryResult qr;
    if (ExecuteStmt(sql, strlen(sql), &qr, bind) != 0 || qr.AffectedRows() != 1)
    {
        // ���⴦����δ���µ��κ�����ʱ��˵��������ݿ��ܴ��ڲ���Ӧ��serverid
        if (qr.AffectedRows() == 0)
            m_errmsg.ShowMsg(EDEBUG, "Save Player failure��[%s]", sql);
        return 0;
    }

    
    return 1;
}




// ɾ����ҽ�ɫ����
int CNetDBModuleEx::DeletePlayerData( DWORD dwRoleID, LPCSTR account, DWORD dwServerID )
{
    char sql[512];
    _snprintf(sql, sizeof sql, "call DeleteRoleData( %d, '%s', %d )", dwRoleID, account, dwServerID);
    sql[sizeof(sql) - 1] = 0;

    QueryResult qr;
    return (ExecuteSql( sql, strlen( sql ), &qr ) == 0 ) && ( qr.AffectedRows() == 1);
}

// �����˺ŷ���������blockΪFalseʱΪ�������
// ����ֵ�У�
// [0]	[�����ɹ�] 
// [1]	[�������ɹ�����Ϊ����״̬�Ѿ�����ΪĿ��״̬] 
// [-1]	[ִ�й����г��ִ���]
// [-2]	[�����ڴ��˺�] 
// [-3]	[���ڴ��˺ţ����Ѿ�����������������]
int CNetDBModuleEx::UpdateAccountLock( LPCSTR account, DWORD serverId, BOOL lockState )
{
    char sql[512];
    _snprintf( sql, sizeof sql, CHECK_ACCOUNT_EXIST, account );
    sql[ sizeof( sql ) - 1 ] = 0;

    QueryResult qr;
    // ���ִ��ʧ�ܣ���ô����-1
    if ( ExecuteStmt( sql, strlen( sql ), &qr ) != 0 )
        return -1;

    // ���û��Ӱ�������������û�ж�Ӧ���˺����ݣ�Ӧ�÷���-2
    if ( qr.AffectedRows() == 0 )
        return -2;

    // ��ȡ��ɫ�������ݣ����ڼ���Ƿ���Ҫ���¡�����
    unsigned long sid = 0;
    QueryBind< 1 > bind;
    bind[0].BindULong( &sid );
    if ( qr.FetchResult( bind ) != 0 )
        return -1;

    // ���Ŀ�����Ѿ�Ϊ�����½�������� 1
    if ( serverId == ( lockState ? sid : 0 ) )
        return 1;

    // ���ִ�и���
    if ( lockState ) _snprintf( sql, sizeof sql, UPDATE_ACCOUNT_LOCK, serverId, account );
    else _snprintf( sql, sizeof sql, UPDATE_ACCOUNT_UNLOCK, account, serverId );
    sql[ sizeof( sql ) - 1 ] = 0;
    qr.Reset();
    if ( ExecuteSql( sql, strlen( sql ), &qr ) != 0 )
        return -1;

    return ( qr.AffectedRows() == 1 ) ? 0 : -3;
}

/*
//Get�˺ŵĲֿ�����
int CNetDBModuleEx::GetWareHouseData( LPCSTR pAccount, LPCSTR name, DWORD dwServerID, SFixProperty &data )
{
    assert( pAccount != NULL );
    assert( name != NULL );

    char sql[1024];
    _snprintf( sql, sizeof sql, SELECT_WAREHOUSE, pAccount, name, dwServerID );
    sql[ sizeof( sql ) - 1 ] = 0;
    QueryResult qr;
    // ��Ҫ����û�����ݵ�������˺�������δ���������ô������쳣�����ܼ���ִ�У�
    if ( ExecuteStmt( sql, strlen( sql ), &qr ) != 0 || qr.AffectedRows() != 1 )
        return 0;

    my_bool isnull = false;
    unsigned long len = 0;
    SWareHouses warehouses;
    long lockState = 0;

    QueryBind< 2 > bind;
    bind[0].BindLong( &lockState ); // ��������ͱ����� MYSQL_TYPE_TINY�� ���ڶ�ȡ��ʱ��Ӧ���ǿ���֧�ֵ�
    bind[1].BindBlob( &warehouses, sizeof( warehouses ) );
    if ( qr.FetchResult( bind ) != 0 )
        return 0;

    // һ��ʼ��ʼ���Ĳֿ����û�����ݣ�
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

//����ֿ�����
int CNetDBModuleEx::SaveWareHouseData( LPCSTR pAccount, LPCSTR name, DWORD dwServerID, bool blocked, SWareHouses &data )
{
    char sql[1024];
    _snprintf( sql, sizeof sql, UPDATE_WAREHOUSE, blocked, pAccount, name, dwServerID );
    sql[ sizeof( sql ) - 1 ] = 0;

    // ����ѹ��
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
//     TraceInfo_C( "faction.txt", "�����������Ϣ��%d����%d����", pMsg->nFactions, pMsg->nTime );
// 
// 	// ���Ƚ�����ȫ����ȡ����
//     SaveFactionData sfd;
// 	LPBYTE streamIter = ( LPBYTE )pMsg->streamFaction;
// 	const DWORD dwFactionMax = pMsg->nFactions;
// 	for( DWORD n = 0; n < dwFactionMax; n++ )
// 	{
// 		// ��ȡ����ͷ����
// 		sfd.stFaction = *( SaveFactionData::SaveFactionInfo* )streamIter;
// 		streamIter += sizeof( SaveFactionData::SaveFactionInfo );
// 
// 		// ��Ա����
// 		size_t memberSize = sizeof( SaveFactionData::SaveMemberInfo ) * sfd.stFaction.byMemberNum;
// 		memcpy( sfd.stMember, streamIter, memberSize );
// 		streamIter += memberSize;
// 
//         TraceInfo_C( "faction.txt", "������ɡ�%s����%d����", sfd.stFaction.szFactionName, sfd.stFaction.byMemberNum );
// 
// 		if ( SaveFaction( sfd, pMsg->nServerID, pMsg->nTime ) == 0 )
// 			m_errmsg.ShowMsg( EDEBUG, "�������ݱ���ʧ��%s", sfd.stFaction.szFactionName );
// 	}
// 
//     // ���ڲ�ֱ��ɾ������������! ��Ϊ��ȡʱ��ʱ�����󱣴�����
// 	//ɾ��������α���ʧ��֮�������
//     //char sql[1024];
//     //_snprintf( sql, sizeof sql, DEL_FACTIONDATA, pMsg->nServerID, pMsg->nTime );
//     //sql[ sizeof( sql ) - 1 ] = 0;
//     //ExecuteSqlCmd( sql );	//�����жϷ���ֵ��
//     return 1;
// }

int CNetDBModuleEx::SaveFactions(struct SQSaveFactionMsg *pMsg)
{
	if (pMsg == NULL || pMsg->nFactions >= MAX_FACTION_NUMBER)
		return 0;

	//TraceInfo_C("faction.txt", "�����������Ϣ��%d����%d����", pMsg->nFactions, pMsg->nTime);

	rfalse(2, 1, "SaveFactions  %d", pMsg->nFactions);

// ���Ƚ�����ȫ����ȡ����
	SaveFactionData_New ssfdnew;
	LPBYTE streamIter = (LPBYTE)pMsg->streamFaction;
	const DWORD dwFactionMax = pMsg->nFactions;
	for (DWORD n = 0; n < dwFactionMax; n++)
	{
		// ��ȡ����ͷ����
		
		memset(&ssfdnew, 0, sizeof(SaveFactionData_New));
		memcpy(&ssfdnew, streamIter, sizeof(SaveFactionData_New));


		// ��Ա����
		streamIter += sizeof(SaveFactionData_New);

		TraceInfo_C("faction.txt", "������ɡ�%s����%d����", ssfdnew.sfactioninfo.szFactionName, ssfdnew.sfactioninfo.CurMemberNum);

		if (SaveFaction(ssfdnew, pMsg->nServerID, pMsg->nTime) == 0)
			m_errmsg.ShowMsg(EDEBUG, "�������ݱ���ʧ��%s", ssfdnew.sfactioninfo.szFactionName);
	}
	return 1;
}

int CNetDBModuleEx::SaveFactionsData(struct SQSaveFactionDataMsg *pMsg)
{
	if (pMsg == NULL || pMsg->nFactions >= MAX_FACTION_NUMBER)
		return 0;

	//TraceInfo_C("faction.txt", "�����������Ϣ��%d����%d����", pMsg->nFactions, pMsg->nTime);

	rfalse(2, 1, "SaveFactions  %d", pMsg->nFactions);

	// ���Ƚ�����ȫ����ȡ����
	SaveFactionData_Lua ssfdnew;
	LPBYTE streamIter = (LPBYTE)pMsg->streamFaction;
	const DWORD dwFactionMax = pMsg->nFactions;
	for (DWORD n = 0; n < dwFactionMax; n++)
	{
		// ��ȡ����ͷ����

		memset(&ssfdnew, 0, sizeof(SaveFactionData_Lua));
		memcpy(&ssfdnew, streamIter, sizeof(SaveFactionData_Lua));


		// ��Ա����
		streamIter += sizeof(SaveFactionData_Lua);

		TraceInfo_C("faction.txt", "������ɡ�%s����", ssfdnew.szFactionName);

		if (SaveFaction(ssfdnew, pMsg->nServerID, pMsg->nTime) == 0)
			m_errmsg.ShowMsg(EDEBUG, "�������ݱ���ʧ��%s", ssfdnew.szFactionName);
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
//     // ������ڲ��ᵼ���ر����صĺ�������������ݴ�
//     if ( stFaction.stFaction.byMemberNum > stFaction.stFaction.byFactionLevel * 50 )
//         LogError( "SaveFaction", "membernum error " );
// 
//     // ����ѹ��
//     size_t dataSize = sizeof( SaveFactionData ) - 
//         ( MAX_MEMBER_NUMBER - stFaction.stFaction.byMemberNum ) * sizeof( stFaction.stMember[0] );
// 
//     LPVOID pakBuf = tmpBuffer;
//     size_t pakSize = sizeof( tmpBuffer );
//     TryEncoding( &stFaction, dataSize, pakBuf, pakSize );
// 
//     DWORD factionId = stFaction.stFaction.factionId;
// 
//     // �ȳ��Ը��£�ʧ��ʱֱ�����
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

	// ����ѹ��
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
	if (qr.AffectedRows() == 0) //û�и��µ����ݿ�
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

	// ����ѹ��
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
	if (qr.AffectedRows() == 0) //û�и��µ����ݿ�
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

    TraceInfo_C( "faction.txt", "�����ȡ������Ϣ��%d����", nServerID );

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

        // ���ݽ�ѹ����ֱ��д��Ŀ��slot���!
        void *unpakData = &stFaction[ idx ];
        size_t unpakSize = sizeof( SaveFactionData );
        BOOL ck = TryDecoding( &sfd, bind[0].Length(), unpakData, unpakSize );

        SaveFactionData *temp = ( SaveFactionData* )unpakData;
        size_t dataSize = sizeof( SaveFactionData ) - 
            ( MAX_MEMBER_NUMBER - stFaction[ idx ].stFaction.byMemberNum ) * sizeof( stFaction[ idx ].stMember[0] );

        // ���ݴ�С�Բ��ϣ�
        if ( unpakSize != dataSize )
        {
            TraceInfo_C( "faction.txt", "��ȡ���ɡ�%d������ѹʧ�ܣ�", idx );
            continue;
        }

        // ������ǽ�ѹ���ݣ�����Ҫ�����ݿ�����Ŀ��slot���
        if ( !ck )
            memcpy( &stFaction[ idx ], &sfd, dataSize );

        TraceInfo_C( "faction.txt", "��ȡ���ɳɹ���%s����%d����", 
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

        // ����Ƿ��ͻ�Խ�磡
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

		// д�����ͷ����
		*( SaveFactionData::SaveFactionInfo* )streamIter = iter->stFaction;
		streamIter += sizeof( SaveFactionData::SaveFactionInfo );
		
		// ��Ա����
		memcpy( streamIter, iter->stMember, sizeof( SaveFactionData::SaveMemberInfo ) * iter->stFaction.byMemberNum );
		streamIter += sizeof( SaveFactionData::SaveMemberInfo ) * iter->stFaction.byMemberNum;

		nFactions ++;
	}

    // ��������Ϣ����֪ͨ��½��������ȡ������ɣ����Ծ���û�ж����κΰ��ɣ�Ҳ��Ҫ���͸���Ϣ
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

    // ��������⴦��==4 ʱ����û�����ݣ�
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
                // ʧ�ܵ�����£������½����룡
                qr.Reset();
                if ( ExecuteStmt( INSERT_BUILDING, sizeof( INSERT_BUILDING ), &qr, bind ) != 0 || qr.AffectedRows() != 1 )
                    continue; // ... ������ʧ�ܵĻ��������������أ�
            }
		}
    }

    // �Ͱ���һ����������߼�Ҳ�������ˣ�
	////�����β���û�и��µ�����ȫ��ɾ��
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
        return 0; // ִ�д���ʱ�������κ���Ϣ

	WORD wIndex = 0;	
	DWORD dwTotalSize = 0;
	size_t size = 0;	//���δ����ݿ��ж�ȡ�Ļ������Ĵ�С	

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
                // �����Ѿ����ˣ��Ȱ�ǰ�沿�����ݷ�������
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

	//���ǵ����ݶ�������,ǰ��һ����wActualSize ��̫�̶����������ܴ�С - ��̬BUF��С
	//dwTotalSize += wIndex * ( sizeof msg.Buffers[0] - sizeof msg.Buffers[0].buffer );
	return TRUE;
}


int CNetDBModuleEx::GetUnionData( const DNID dwSID, SQGetUnionDataMsg * pMsg )
{
//     TraceInfo_C( "uniondata.txt", "��ʼ�����������!" );
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
//         // д�����ͷ����
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
//     TraceInfo_C( "uniondata.txt", "��ʼ����������� tick = %d count = %d size = %d��", pMsg->saveTime, pMsg->unionCount, pMsg->dwSendSize );
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
	// �޸���ҵ�������
	_snprintf( sql, sizeof sql, UPDATE_PLAYER_CONFIG, PlayerConfig, pAccount );	
	if ( ExecuteSql( sql, strlen( sql ), &qr ) != 0 || qr.AffectedRows() != 1 ){
		 m_errmsg.ShowMsg(EDEBUG, "�����˺�Ϊ%s����ҵ�������ʧ��", pAccount);
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
		return 3;//sqlִ��ʧ��

	if (qr.AffectedRows() < 0)
	{
		return 4;//Ӱ������С��0
	}

	if (qr.AffectedRows() != 0)
	{
		NewRankList tmpbuf;
		Query::QueryBind< 1 > bind;
		bind[0].BindBlob( &tmpbuf, sizeof( tmpbuf ) );
		num = 0;
		while ( qr.FetchResult( bind ) == 0 )
		{
			// ���⴦�����ݴ���Ϊ�գ����������Ŵ��ڵ���5�����Ѿ���ȡ�����ݣ� ���ٶ���
			if ( bind[0].IsNull())
				continue;
			pNewRanklist[num++] = tmpbuf;
			if (num >= ALL_RANK_NUM)
			{
				return 1;
			}
		}
		return 1;//�ɹ�
	}
	

	return 2;//û���ҵ����ʵ�����;
}

//���� 2015.1.12 wk �ʼ�ϵͳ �洢���̵���##begin######
//ϵͳ����ȫ��,�����id,���ݿ�sysmail����
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
			m_errmsg.ShowMsg(EDEBUG, "SendMail1 failure��[%s]", sql);
		return 0;
	}
	return 1;
}

//ϵͳ�������,�����id,���ݿ�emailmsg����
int CNetDBModuleEx::SendMail(S2D_SendMail_MSG*pBaseMsg)
//int CNetDBModuleEx::SendMail(DWORD senderid, LPCSTR pRoleName, DWORD emailtype, MailInfo &MailInfotData, DWORD serverid, DWORD expiretime)
{
	
	QueryBind<1> bind;
	bind[0].BindBlob(&pBaseMsg->m_MailInfo, sizeof(MailInfo));

	char sql[1024];
	if (pBaseMsg->dw_RevId>0)//��id��
	{
		_snprintf(sql, sizeof sql, INSERT_SEND_MAIL, pBaseMsg->dw_RevId, pBaseMsg->dw_emailtype, pBaseMsg->dw_serverid, pBaseMsg->dw_expiretime);
	}
	else //�����ַ�
	{
		_snprintf(sql, sizeof sql, CALL_SEND_MAILBYNAME, 0, pBaseMsg->name, pBaseMsg->dw_emailtype, pBaseMsg->dw_serverid, pBaseMsg->dw_expiretime, "@out_ret");
	}
	

	sql[sizeof(sql) - 1] = 0;


	QueryResult qr;
	if (ExecuteStmt(sql, strlen(sql), &qr, bind) != 0 || qr.AffectedRows() != 1)
	{
		if (qr.AffectedRows() == 0)
			m_errmsg.ShowMsg(EDEBUG, "SendMail failure��[%s]", sql);
		return 0;
	}
	return 1;
}

int CNetDBModuleEx::GetMailInfo_DB(const DNID clientDnid, SQ_GetMailInfo_MSG* pMsg)
{
	int sid = pMsg->m_dwSid;
	//��ȡϵͳȫ���ʼ�
	char sql_1[1024];
	_snprintf(sql_1, sizeof sql_1, CALL_GETMAILFROM_MAILSYS, sid, 0, "@out_ret");
	sql_1[sizeof(sql_1) - 1] = 0;
	QueryResult qr_1;
	if ((ExecuteStmt(sql_1, strlen(sql_1), &qr_1) != 0) || qr_1.AffectedRows() < 0)
		return 0;

	//ȡ�Լ���
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
		return 1;//�ɹ�
	}
	return 2;
}
//ȡ����ʱͬʱ�����ʼ�״̬Ϊ��ȡ
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
		
		//������ȡ״̬
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

	TraceInfo_C("faction.txt", "�����ȡ������Ϣ��%d����", nServerID);

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

		// ���ݽ�ѹ����ֱ��д��Ŀ��slot���!
		void *unpakData = &stFaction[idx];
		size_t unpakSize = sizeof(SaveFactionData_New);
		BOOL ck = TryDecoding(&sfd, bind[0].Length(), unpakData, unpakSize);

	/*	SaveFactionData_New *temp = (SaveFactionData_New*)unpakData;*/
		size_t dataSize = sizeof(SaveFactionData_New);
		
		// ���ݴ�С�Բ��ϣ�
		if (unpakSize != dataSize)
		{
			TraceInfo_C("faction.txt", "��ȡ���ɡ�%d������ѹʧ�ܣ�", idx);
			continue;
		}

		// ������ǽ�ѹ���ݣ�����Ҫ�����ݿ�����Ŀ��slot���
		if (!ck)
			memcpy(&stFaction[idx], &sfd, dataSize);

// 		TraceInfo_C("faction.txt", "��ȡ���ɳɹ���%s����%d����",
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
		if (idx > MAX_SENDFACTIONS && nFactions  >= MAX_SENDFACTIONS) //�Ѿ����������
		{
			factionMsg.nFactions = MAX_SENDFACTIONS;
			SendMessage(dwSID, &factionMsg, sizeof(SAGetFactionMsg));
			memset(streamIter, 0, sizeof(SaveFactionData_New) * MAX_SENDFACTIONS);     //������0

			rfalse(2, 1, "GetFactions_New  %d", factionMsg.nFactions);
			streamIter = (LPBYTE)factionMsg.streamFaction;
			factionMsg.nFactions = 0;
			nFactions = 0;
		}
		
	}

	// ��������Ϣ����֪ͨ��½��������ȡ������ɣ����Ծ���û�ж����κΰ��ɣ�Ҳ��Ҫ���͸���Ϣ
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

	TraceInfo_C("faction.txt", "DeleteFaction��%d����", Factionid);

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
		// ���ݽ�ѹ����ֱ��д��Ŀ��slot���!
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

	TraceInfo_C("faction.txt", "�����ȡ������Ϣ��%d����", nServerID);

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

		// ���ݽ�ѹ����ֱ��д��Ŀ��slot���!
		void *unpakData = &stFaction[idx];
		size_t unpakSize = sizeof(SaveFactionData_Lua);
		BOOL ck = TryDecoding(&sfd, bind[0].Length(), unpakData, unpakSize);

		/*	SaveFactionData_New *temp = (SaveFactionData_New*)unpakData;*/
		size_t dataSize = sizeof(SaveFactionData_Lua);

		// ���ݴ�С�Բ��ϣ�
		if (unpakSize != dataSize)
		{
			TraceInfo_C("faction.txt", "��ȡ���ɡ�%d������ѹʧ�ܣ�", idx);
			continue;
		}

		// ������ǽ�ѹ���ݣ�����Ҫ�����ݿ�����Ŀ��slot���
		if (!ck)
			memcpy(&stFaction[idx], &sfd, dataSize);

		// 		TraceInfo_C("faction.txt", "��ȡ���ɳɹ���%s����%d����",
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
		if (idx > MAX_SENDFACTIONDATA && nFactions >= MAX_SENDFACTIONDATA) //�Ѿ����������
		{
			factionMsg.nFactions = MAX_SENDFACTIONDATA;
			SendMessage(dwSID, &factionMsg, sizeof(SAGetFactionDataMsg));
			memset(streamIter, 0, sizeof(SaveFactionData_Lua) * MAX_SENDFACTIONDATA);     //������0

			rfalse(2, 1, "GetFactions_New  %d", factionMsg.nFactions);
			streamIter = (LPBYTE)factionMsg.streamFaction;
			factionMsg.nFactions = 0;
			nFactions = 0;
		}

	}

	// ��������Ϣ����֪ͨ��½��������ȡ������ɣ����Ծ���û�ж����κΰ��ɣ�Ҳ��Ҫ���͸���Ϣ
	factionMsg.isLast = TRUE;
	factionMsg.nFactions = nFactions;
	SendMessage(dwSID, &factionMsg, sizeof(SAGetFactionDataMsg));
	rfalse(2, 1, "GetFactions_Data  %d", nFactions);
	return 1;
}

//���� 2015.1.12 wk �ʼ�ϵͳ �洢���̵���##begin######