#include "StdAfx.h"
#include <assert.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <ctime>
#include <cctype>
#include <algorithm>
#include "mysqldbprocess.h"
#include "SQLTextDefine.h"
#include "../../pub/stlini.h"
#include "../../pub/path.h"

using namespace std;
using namespace mysql;
void ErrMsg::ShowMsg( int nflag ,const char* szOutput, ... )
{		
	va_list  va;
	if(szOutput)	
	{
		va_start(va, szOutput);
		vsprintf(out, szOutput, va);
		va_end(va);
	}
	if ( nflag )
	{
		::MessageBox( 0, out, "", MB_ICONERROR );
	}
	else
	{
        strcat( out, "\r\n" );
		OutputDebugString( out );
	}
}
void ErrMsg::Save()
{
	ofstream out("m_errmsg.txt");	
	struct tm *newtime;	
	__int64 ltime;
	_time64( &ltime );   
	newtime = _localtime64( &ltime ); 
	out << newtime->tm_year+1900<< '-' 
		<<  newtime->tm_mon+1 << '-' 
		<< newtime->tm_mday<< " " 
		<< newtime->tm_hour
		<<':' << newtime->tm_min
		<< ':' <<newtime->tm_sec 
	//	<< " ## "<<  out << " ##" << endl;	
		<< " ## " << endl;
}


Query::Query(void)
{
	m_pData = NULL;
	m_pstmt = NULL;
}

Query::~Query(void)
{
	Clear();
}

const  MYSQL * Query::GetMySql( void )
{
	return m_pData;
}

bool Query::Connect( LPCSTR host, LPCSTR user, LPCSTR passwd, LPCSTR db, 
							 UINT port , DWORD client_flag )
{
    client_flag = CLIENT_MULTI_RESULTS/* | CLIENT_MULTI_STATEMENTS*/;
	if ( !mysql_real_connect( m_pData, host, user, passwd, db, port, NULL, client_flag ) )
	{
		m_errmsg.ShowMsg( EDEBUG, "数据库连接出现错误 %s", mysql_error(m_pData) );
		return false;
	}
	return true;
}

bool Query::Init( LPCTSTR UserName, LPCTSTR PassWord, LPCTSTR DBIP, LPCTSTR DBName, DWORD dwPort )
{
	// 初始化mysql
	prevCheckTime = timeGetTime();
	m_pData = mysql_init( NULL );
	assert( m_pData );
	if( m_pData == NULL )
	{
		m_errmsg.ShowMsg( EMSGBOX, "初始化mysql失败" );
		return false;
	}

	//mysql_options( m_pData, MYSQL_READ_DEFAULT_GROUP, "client" );
	mysql_options( m_pData, MYSQL_OPT_CONNECT_TIMEOUT, "120" );
	mysql_options( m_pData, MYSQL_SET_CHARSET_NAME, "utf8" ); //utf8
    //mysql_options( m_pData, MYSQL_OPT_RECONNECT, "1" );

	//连接数据库
	if ( !Connect( DBIP, UserName, PassWord, DBName, dwPort )) return false;

	//初始化STMT
	m_pstmt = mysql_stmt_init( m_pData );
	assert( m_pstmt );
	if (!m_pstmt)
	{
		m_errmsg.ShowMsg( EMSGBOX, "mysql_stmt_init(), out of memory\n" );		
		return false;
	}

	m_DBName = DBName;
	m_User = UserName;
	m_Password = PassWord;
	m_Host = DBIP;

	return  true;
}

void Query::Clear()
{
	if ( m_pstmt != NULL )
	{
		mysql_stmt_close(m_pstmt);
		m_pstmt = NULL;
	}

	if( m_pData != NULL )
	{
		mysql_close( m_pData );		
		m_pData = NULL;
	}
}

void Query::LogError( LPCTSTR FuncName, LPCTSTR Error)
{	
    ofstream out( "DebugQuery.txt", std::ios::app );	
    extern LPCSTR GetStringTime();
    out << GetStringTime() << " ## " <<  FuncName << " ## " << Error << endl;	
}

/*
int Query::ExecuteSQL( LPCSTR sql, MYSQL_RES **res )
{
	m_AffectedRows = 0;
	MYSQL_RES *temp = NULL;
	if ( !mysql_query( m_pData, sql ) ) 
	{
		if ( res != NULL )
			*res = mysql_store_result( m_pData );
		else
			temp = mysql_store_result( m_pData );

		m_AffectedRows = mysql_affected_rows( m_pData );

		if ( temp != NULL ) 
			mysql_free_result( temp );
	}
	else
	{
		m_errmsg.ShowMsg( EDEBUG, "执行SQL=%s失败= %s", sql, mysql_error(m_pData)); 
	}

	//由于返回无符号值，可通过比较返回值和“(my_ulonglong)-1”（或等效的“(my_ulonglong)~0”），检查“-1”。
	return m_AffectedRows;	
}*/

// 主要功能: 执行非返回结果查询
// 参数:sql 待执行的查询语句
// 返回值; 返回影响的行数 -1 为执行失败 

int  Query::ExecuteSqlCmd( LPCSTR sql )
{
	assert( sql != NULL );
	int ret = mysql_real_query( m_pData, sql, static_cast< unsigned long >( strlen( sql ) ) );
	if ( ret != 0 )
	{
		m_errmsg.ShowMsg( EDEBUG, " mysql_real_query 失败 Error: %d\nsql = %s\ninfo = %s", ret, sql, mysql_error(m_pData) ), 0;
		return -1;
	}

	return static_cast< int >( mysql_affected_rows( m_pData ) );
}

BOOL Query::KeepAlive( DWORD timeMargin, LPCSTR who )
{
	if ( (int)timeGetTime() - (int)prevCheckTime > (int)timeMargin )
	{
		prevCheckTime = timeGetTime() + timeMargin;

        // mysql_real_query( m_pData, "select 1", sizeof( "select 1" ) ) /**/
        if ( int err = mysql_ping( m_pData ) )
		{
            // LPCSTR info = mysql_error( m_pData );
			// LogError( "KeepAlive", "数据库重连接失败!尝试重连接！" );
			Clear();
			BOOL succ = Init( m_User.c_str(), m_Password.c_str(), m_Host.c_str(), m_DBName.c_str() );
            char temp[256];
            sprintf( temp, succ ? "Ado[%d] [%s]连接断开, 重连接成功!" : "Ado[%d] [%s]连接断开, 重连接失败！！！", err, who ? who : "" );
            LogError( "Query::KeepAlive", temp ); 
            return succ;
		}
        //else
        //{
        //    MYSQL_RES *res = mysql_store_result( m_pData );
        //    mysql_free_result( res );
        //}
	}

    return true;
}

void Query::UnInit()
{
	Clear();
}

int Query::ExecuteStmt( const void *sql, size_t size, QueryResult *qr, Arguments args )
{
    assert( ( qr == NULL ) || ( qr->stmt == NULL ) );

    int result = mysql_stmt_prepare( m_pstmt, ( const char * )sql, ( unsigned long )size );
	if ( result != 0 )
        return m_errmsg.ShowMsg( EDEBUG, "err at mysql_stmt_prepare : %s [%s]", mysql_stmt_error( m_pstmt ), sql ), result;

    if ( args.size != 0 )
    {
        assert( args.size == mysql_stmt_param_count( m_pstmt ) );
        result = mysql_stmt_bind_param( m_pstmt, args.array );
		if (result != 0)
		{
			rfalse("err at mysql_stmt_bind_param : %s [%s]", mysql_stmt_error(m_pstmt), sql);
			return m_errmsg.ShowMsg( EDEBUG, "err at mysql_stmt_bind_param : %s [%s]", mysql_stmt_error( m_pstmt ), sql ), result;
		}
            
    }

    result = mysql_stmt_execute( m_pstmt );
	if (result != 0)
	{
		rfalse("err at mysql_stmt_execute : %s [%s]", mysql_stmt_error(m_pstmt), sql);
		return m_errmsg.ShowMsg( EDEBUG, "err at mysql_stmt_execute : %s [%s]", mysql_stmt_error( m_pstmt ), sql ), result;
	}
        

    result = mysql_stmt_store_result( m_pstmt );
	if ( result != 0 )
        return m_errmsg.ShowMsg( EDEBUG, "err at mysql_stmt_store_result : %s [%s]", mysql_stmt_error( m_pstmt ), sql ), result;

    if ( qr != NULL )
    {
        qr->sql = sql;
        qr->stmt = &m_pstmt;
        qr->res = mysql_stmt_result_metadata( m_pstmt );
        qr->nrow = mysql_stmt_affected_rows( m_pstmt );

        if ( qr->res )
        {
            qr->num_rows = mysql_num_rows( qr->res );
            qr->num_fields = mysql_num_fields( qr->res );
        }
    }
    else
    {
        int ck = mysql_stmt_free_result( m_pstmt );
        assert( ck == 0 );
    }

    return 0;
}

int Query::ExecuteSql( const void *sql, size_t size, QueryResult *qr )
{
    assert( ( qr == NULL ) || ( qr->stmt == NULL ) );
	char sre[512];
	memcpy(sre,sql,size);
	sre[size] = '\0';
	/*std::string strlpt = sql;*/
    int result = mysql_real_query( m_pData, ( const char * )sre, ( unsigned long )size );
	if ( result != 0 ) // 通过显示 sql内容，来说明执行的有效性
    {
        if ( size < 1024 )
        {
            assert( reinterpret_cast< LPCSTR >( sql )[ size ] == 0 );
            if ( reinterpret_cast< LPCSTR >( sql )[ size ] == 0 )
                m_errmsg.ShowMsg( EDEBUG, "err at mysql_real_query : %s [%s]", mysql_error( m_pData ), sql );
        }
        else
        {
            char tempBuf[256];
            memcpy( tempBuf, sql, 200 );
            *( LPDWORD )( tempBuf + 200 ) = 0;
            m_errmsg.ShowMsg( EDEBUG, "err at mysql_real_query : %s [ size = %d ][ %s ]", mysql_error( m_pData ), size, tempBuf );
        }

        return result;
    }

	MYSQL_RES *temp = mysql_store_result( m_pData );
    // OutputDebugString( "store\r\n" );
    if ( qr != NULL )
    {
        assert( qr->res == NULL );
        //if ( qr->res )
        //{
        //    m_errmsg.ShowMsg( EDEBUG, "find a validate res!!! [ size = %d ][ %s ]", size, sql );
        //    mysql_free_result( qr->res );
        //}

        qr->sql = sql;
		qr->res = temp;
        qr->nrow = mysql_affected_rows( m_pData );

        if ( qr->res )
        {
            qr->num_rows = mysql_num_rows( qr->res );
            qr->num_fields = mysql_num_fields( qr->res );
        }

        temp = NULL;
    }

    if ( temp != NULL )
        mysql_free_result( temp )/*, OutputDebugString( "release\r\n" )*/;

    return 0;
}

//从一个指定的配置文件中读取数据库连接参数
int Query::Init( LPCSTR ConfigFileName)
{
	IniFiles  IniFile;
	if ( !IniFile.open( ConfigFileName ) )
	{
		MessageBox( 0, "Cannot open ini file", ConfigFileName, MB_ICONERROR );
		return FALSE ;
	}
	m_Host		= IniFile.read( "MYSQLDATABASE", "IP",        "127.0.0.1" );
	m_User		= IniFile.read( "MYSQLDATABASE", "ACCOUNT",   "root" );
	m_DBName	= IniFile.read( "MYSQLDATABASE", "DEFAULTDB", "roledbi" );
	m_Port		= IniFile.read_int ( "MYSQLDATABASE",   "PORT", 3306 );
	m_Password	= IniFile.read( "MYSQLDATABASE", "PASSWORD", "tracert");
	return Init(m_User.c_str(), m_Password.c_str(), m_Host.c_str(), m_DBName.c_str(), m_Port );
}

void Query::Backup( LPCSTR path, LPCSTR title )
{
	return; //2013.03.20 暂时屏蔽掉备份操作

    // 当前逻辑的执行线程是主工作线程，这个线程里如果弹对话框，
    // 会导致角色保存、读取逻辑的阻塞，但不会导致严重影响
    DWORD dfs[4];
    char str[1024];
    char cmd[1024];

    sprintf( str, "%s/%s", path, title );
    if ( !CreateDir( str ) )
        MessageBox( 0, "准备备份目录失败，请检测系统信息后继续！", str, 0 );

    if ( !GetDiskFreeSpace( path, &dfs[0], &dfs[1], &dfs[2], &dfs[3] ) )
        MessageBox( 0, "获取剩余磁盘空间失败，请检测系统信息后继续！", title, 0 );

    if ( dfs[2] < 200 * 1024 * 1024 / ( dfs[0] * dfs[1] ) )
        MessageBox( 0, "系统空间不足200MB，请检测系统信息后继续！", title, 0 );

    SYSTEMTIME st;
    GetLocalTime(&st);
    sprintf( str, "%s/%s/mysqldump%04d-%02d-%02d_%02d-%02d", 
        path, title, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute );

    sprintf( cmd, "%s.dat", str );

    // 安全描述符需要设置句柄可继承才能用于重定向！
    SECURITY_ATTRIBUTES sa = { sizeof( sa ), NULL, TRUE }; 
    HANDLE hRedirect = CreateFile( cmd, GENERIC_WRITE, 0, &sa, CREATE_ALWAYS, 0, 0 );

    sprintf( cmd, "mysqldump.exe --default-character-set=gbk -R -h%s -u%s -p%s %s",
        m_Host.c_str(), m_User.c_str(), m_Password.c_str(), m_DBName.c_str() );

    PROCESS_INFORMATION pi = { 0 }; 
    STARTUPINFO si = { sizeof( si ) }; 
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; 
    si.hStdError = si.hStdOutput = hRedirect;
    si.wShowWindow = SW_HIDE;
    if ( !CreateProcess( NULL, cmd, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi ) )
    { 
        MessageBox( 0, "备份操作执行失败，请检测系统信息后继续！", title, 0 );
        return;
    }

    WaitForSingleObject( pi.hProcess, INFINITE ); 
    CloseHandle( pi.hProcess ); 
    CloseHandle( pi.hThread ); 
    CloseHandle( hRedirect ); 

    // 尝试启动 rar 压缩备份文件
    sprintf( cmd, "rar.exe a -df -ep -inul \"%s.rar\" \"%s.dat\"", str, str );

    memset( &pi, 0, sizeof( pi ) ); 
    memset( &si, 0, sizeof( si ) ); 
    si.cb = sizeof( si );
    si.dwFlags = STARTF_USESHOWWINDOW; 
    si.wShowWindow = SW_HIDE;
    if ( CreateProcess( NULL, cmd, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi ) )
    {
        CloseHandle( pi.hProcess ); 
        CloseHandle( pi.hThread ); 
    }
}

// #########################Query End##############################

// #########################BIND Begin##############################
Query::TempBind::TempBind( MYSQL_BIND &a, my_bool &b, unsigned long &c ) : 
    bind( a ), null( b ), lens( c ) 
{
    if ( lens == 0 )
        assert( bind.buffer_type == 0 );
}

void Query::TempBind::BindBlob( void *data, size_t size ) 
{
    lens = ( unsigned long )size;
    bind.buffer_type = MYSQL_TYPE_BLOB;
    bind.buffer_length = ( unsigned long )size;
    bind.buffer = ( char* )data;
    bind.is_null = &null;
    bind.length = &lens;
} 

void Query::TempBind::BindString( const char *data, size_t size )
{
    lens = ( unsigned long )size;
    bind.buffer_type = MYSQL_TYPE_STRING;
    bind.buffer_length = ( unsigned long )size;
    bind.buffer = ( char* )data;
    bind.is_null = &null;
    bind.length = &lens;
}

void Query::TempBind::BindUTiny( unsigned char *data )
{
    bind.buffer_type = MYSQL_TYPE_TINY;
    bind.buffer = ( char* )data;
    bind.is_null = &null;
    bind.length = &lens;
    bind.is_unsigned = true;
}

void Query::TempBind::BindLong( long *data ) 
{
    bind.buffer_type = MYSQL_TYPE_LONG;
    bind.buffer = ( char* )data;
    bind.is_null = &null;
    bind.length = &lens;
} 

void Query::TempBind::BindULong( unsigned long *data ) 
{
    bind.buffer_type = MYSQL_TYPE_LONG;
    bind.buffer = ( char* )data;
    bind.is_null = &null;
    bind.length = &lens;
    bind.is_unsigned = true;
} 

void Query::TempBind::BindLongLong( __int64 *data ) 
{
    bind.buffer_type = MYSQL_TYPE_LONGLONG;
    bind.buffer = ( char* )data;
    bind.is_null = &null;
    bind.length = &lens;
} 

void Query::TempBind::BindULongLong( unsigned __int64 *data ) 
{
    bind.buffer_type = MYSQL_TYPE_LONGLONG;
    bind.buffer = ( char* )data;
    bind.is_null = &null;
    bind.length = &lens;
    bind.is_unsigned = true;
} 
// #########################BIND End#############################

// #########################QueryResult Begin####################
void Query::QueryResult::Release() 
{
	if ( res )
        mysql_free_result( res )/*, OutputDebugString( "release\r\n" )*/; 

    if ( stmt != NULL && *stmt != NULL ) 
    {
        int ck = mysql_stmt_free_result( *stmt ); 
        assert( ck == 0 );
    }
}

int Query::QueryResult::AffectedRows() 
{ 
    if ( /*( res == NULL && ( stmt == NULL || *stmt == NULL ) ) || */nrow >= 0x80000000 ) 
        return -1; 

    return ( int )nrow; 
}

int Query::QueryResult::FetchResult( MYSQL_BIND *bind )
{
    if ( stmt == NULL || *stmt == NULL )
        return -1;

    if ( binds == NULL )
    {
        int result = mysql_stmt_bind_result( *stmt, bind );
		if ( result != 0 )
            return result;
    }

	return mysql_stmt_fetch( *stmt );
}

int Query::QueryResult::FetchResult()
{
    if ( res == NULL )
        return -1;

    if ( num_rows == 0 || num_fields == 0 )
        return 1;

    rows = mysql_fetch_row( res );
    lens = mysql_fetch_lengths( res );
    return ( rows != NULL && lens != NULL ) ? 0 : -1;
}

// #########################QueryResult End##########################

// #########################Procedure Begin#################################
static enum_field_types CheckType( std::string &str )
{
    static const enum_field_types errtype = ( enum_field_types )-1;
    //"blob"
    //"varchar"
    //"int"
    //"bigint"
    switch ( str[0] )
    {
    case 'b':
        switch ( str[1] )
        {
        case 'l':   return ( str == "blob" ) ? MYSQL_TYPE_BLOB : errtype;
        case 'i':   return ( str == "bigint" ) ? MYSQL_TYPE_LONGLONG : errtype;
        }
        break;

    case 'i':
        return ( str == "int" ) ? MYSQL_TYPE_LONG : errtype;

    case 'c':
        return ( str == "char" ) ? MYSQL_TYPE_STRING : errtype;

    case 'v':
        return ( str == "varchar" ) ? MYSQL_TYPE_STRING : errtype;
    }

    return errtype;
}

int Query::ProcedureHelper::Prepare( LPCSTR procName )
{
    strProc = procName;
	char sql[1024];
	_snprintf( sql, sizeof( sql ), SELECT_PROCEDURE_PARAMLIST, query.GetDBName(), procName );
	sql[ sizeof( sql ) - 1 ] = 0;

    QueryResult qr;
    int result = query.ExecuteSql( sql, strlen( sql ), &qr );
    if ( result != 0 )
        return result;
    
    if ( qr.AffectedRows() != 1 )
        return -1;

    result = qr.FetchResult();
    if ( result != 0 )
        return result;

    assert( qr.num_rows == 1 );
    assert( qr.num_fields == 1 );

    // 这里限定了存储过程参数列表的最基本规则：
    //  不能在参数列表中使用双字节字符
    //  不能在参数列表中使用注释
    //  长度标识必须紧靠参数类型
    //  ‘，’必须紧靠参数申明的末尾
    //  参数和参数之间的‘，’后必须存在空格
    std::string s( qr.rows[0], qr.lens[0] );
    std::transform( s.begin(), s.end(), s.begin(), std::tolower );
    std::stringstream stream( s );

    // 0 : 检查参数传递方向
    // 1 : 检查参数名
    // 2 : 检查参数类型
    int checkState = 0;

    totalCount = 0;
	inCount = 0;
	outCount = 0;
	inoutCount = 0;

    std::string str;
    stream >> str;
    while ( !str.empty() )
    {
        switch ( checkState )
        {
        case 0:
            checkState = 1;
            if ( str == "in" )
            {
                parameters[ totalCount ].direction = SqlParam::V_IN;
                inCount ++;
            }
            else if ( str == "out" )
            {
                parameters[ totalCount ].direction = SqlParam::V_OUT;
                outCount ++;
            }
            else if ( str == "inout" )
            {
                parameters[ totalCount ].direction = SqlParam::V_INOUT;
                inoutCount ++;
            }
            else
            {
                // 由于未定义（直接就指定了参数名），默认为传入类型 
                parameters[ totalCount ].direction = SqlParam::V_IN;
                inCount ++;
                checkState = 2;
            }
            break;

        case 1:
            checkState = 2;
            break;

        case 2:
            {
                size_t last = str.length();
                if ( str.at( last - 1 ) == ',' )
                    last -= 1;

                // 是否还有长度信息？
                if ( str.at( last - 1 ) == ')' )
                {
                    last = str.find_first_of( '(' );
                    if ( last != std::string::npos )
                        parameters[ totalCount ].lens = atoi( str.c_str() + last + 1 );
                }

                parameters[ totalCount ].type = CheckType( str.substr( 0, last ) );
                assert( parameters[ totalCount ].type != -1 );

                totalCount ++;
                checkState = 0;
            }
            break;
        }

        str.clear();
        stream >> str;
    }

    assert( totalCount == inCount + outCount + inoutCount );

	return 0;
}

// 准备一个 64K 的 BUFFER 来作为 real_query 的缓存
__declspec( thread ) char PrepareBuffer[ 0xffff ];
__declspec( thread ) char StrBuffer[ 0xfff ];

LPCSTR ToString( lite::Variant &lvt, enum_field_types type )
{
    // 处理未付初值或空的变量
    if ( lvt.dataType <= 1 )
        return "null";

    switch ( type )
    {
    case MYSQL_TYPE_LONG:
        if ( lvt.dataType & 0x1 ) // 区分有符号和无符号数
            sprintf( StrBuffer, "%d", ( int )lvt );
        else
            sprintf( StrBuffer, "%u", ( unsigned int )lvt );
        break;

    case MYSQL_TYPE_LONGLONG:
        if ( lvt.dataType & 0x1 ) // 区分有符号和无符号数
            sprintf( StrBuffer, "%I64d", ( __int64 )lvt );
        else
            sprintf( StrBuffer, "%I64u", ( unsigned __int64 )lvt );
        break;

    case MYSQL_TYPE_STRING:
        sprintf( StrBuffer, "'%s'", ( LPCSTR )lvt );
        break;
    }

    return StrBuffer;
}

void ToVariant( lite::Variant &lvt, LPCSTR value, size_t size, enum_field_types type )
{
    // 处理未付初值或空的变量
    assert( lvt.dataType == lite::Variant::VT_EMPTY );

    if ( size == 0 )
    {
        lvt.dataType = lite::Variant::VT_NULL;
        return;
    }

    __int64 tv64 = 0;

    switch ( type )
    {
    case MYSQL_TYPE_LONG:
        tv64 = _atoi64( value ); // 需要确认字符串有 '\0'
        lvt.DeepCopy( lite::Variant( ( int )tv64 ) );
        break;

    case MYSQL_TYPE_LONGLONG:
        tv64 = _atoi64( value ); // 需要确认字符串有 '\0'
        lvt.DeepCopy( lite::Variant( tv64 ) );
        break;

    case MYSQL_TYPE_STRING:
        lvt.DeepCopy( lite::Variant( value ) );
        break;

    case MYSQL_TYPE_BLOB:
        lvt.DeepCopy( lite::Variant( value, size ) );
        break;
    }
}

int Query::ProcedureHelper::Execute()
{
    int len = 0;
    int result = 0;
    DWORD idxPrepare = 0;
    char tempStr[256] = "";
    std::string strSelect;
    std::string strCall = "call ";
    strCall += strProc;
    strCall.append( " ( ", 3 );

    for ( DWORD i = 0; i < totalCount; i ++ )
    {
        // 如果不是第一个参数，就需要加分隔符了。。。
        if ( i != 0 )
            strCall += ", ";

        // 准备所有调用参数！
        lite::Variant &lvt = parameters[i].bindValue;

        // 如果是传入参数，一般情况下不使用用户变量！
        if ( parameters[i].direction == SqlParam::V_IN )
        {
            if ( parameters[i].type != MYSQL_TYPE_BLOB )
                strCall += ToString( lvt, parameters[i].type );
            else
            {
                // 二进制数据特殊，需要单独准备参数
                assert( lvt.dataType == lite::Variant::VT_POINTER );

                // 为前一个语句加上结束符
                if ( idxPrepare != 0 )
                    PrepareBuffer[ idxPrepare++ ] = ',';
                else
                {
                    *( LPDWORD )PrepareBuffer = *( LPDWORD )"set";
                    idxPrepare += 3;
                }

                len = sprintf( tempStr, " @_%d = '", i );
                memcpy( PrepareBuffer + idxPrepare, tempStr, len );
                idxPrepare += len;
                len = mysql_real_escape_string( query.m_pData, 
                    PrepareBuffer + idxPrepare, ( LPCSTR )lvt._pointer, lvt.dataSize );
                idxPrepare += len;
                PrepareBuffer[ idxPrepare++ ] = '\'';

                // 插入变量调用
                len = sprintf( tempStr, "@_%d", i );
                strCall += tempStr;
            }
        }
        else 
        {
            // 接下来处理的就是 IN/INOUT 不管是哪一类，都需要准备用户变量
            // 为前一个语句加上结束符
            if ( idxPrepare != 0 )
                PrepareBuffer[ idxPrepare++ ] = ',';
            else
            {
                *( LPDWORD )PrepareBuffer = *( LPDWORD )"set";
                idxPrepare += 3;
            }

            // INOUT 是需要传值的
            if ( parameters[i].direction == SqlParam::V_INOUT )
            {
                // 插入变量调用
                if ( parameters[i].type != MYSQL_TYPE_BLOB )
                {
                    len = sprintf( PrepareBuffer + idxPrepare, " @_%d = %s", i, ToString( lvt, parameters[i].type ) );
                    idxPrepare += len;
                }
                else
                {
                    assert( lvt.dataType == lite::Variant::VT_POINTER );

                    len = sprintf( PrepareBuffer + idxPrepare, " @_%d = '", i );
                    idxPrepare += len;
                    len = mysql_real_escape_string( query.m_pData, 
                        PrepareBuffer + idxPrepare, ( LPCSTR )lvt._pointer, lvt.dataSize );
                    idxPrepare += len;
                    PrepareBuffer[ idxPrepare++ ] = '\'';
                }
            }
            else
            {
                // 单纯的 OUT 类型，需要初始化
                int len = sprintf( PrepareBuffer + idxPrepare, " @_%d = null", i );
                idxPrepare += len;
            }

            // 设置变量的调用
            len = sprintf( tempStr, "@_%d", i );
            strCall.append( tempStr, len );

            // 准备获取执行结果！
            len = sprintf( tempStr, strSelect.empty() ? "select @_%d" : ", @_%d", i );
            strSelect.append( tempStr, len );
        }
    }

    // 加上结束括号
    strCall.append( " )", 2 );

    // 如果存在预执行语句，需要先调用
    if ( idxPrepare != 0 )
    {
        //int result = mysql_real_query( query.m_pData, ( const char * )PrepareBuffer, ( unsigned long )idxPrepare );
        //if ( result != 0 )
        //    return result;

        //do {
        //    if ( MYSQL_RES *res = mysql_store_result( query.m_pData ) )
        //        mysql_free_result( res );
        //} while ( !mysql_next_result( query.m_pData ) );

        // 容错，处理越界问题！
        assert( idxPrepare < 0xffff - 20 );
        if ( idxPrepare < 0xffff - 20 )
        {
            PrepareBuffer[ idxPrepare ] = 0;
            result = query.ExecuteSql( PrepareBuffer, idxPrepare, NULL );
            if ( result != 0 )
                return result;
        }
    }

    // 执行存储过程！
    // 在没有返回值的情况下，可能会返回数据集，所以需要保存QueryResult
    result = query.ExecuteSql( strCall.c_str(), strCall.length(), strSelect.empty() ? this : NULL );
    if ( result != 0 )
        return result;

    // 获取执行结果
    if ( strSelect.empty() )
        return 0;

    result = query.ExecuteSql( strSelect.c_str(), strSelect.length(), this );
    if ( result != 0 )
        return result;
    
    if ( AffectedRows() != 1 )
        return -1;

    result = FetchResult();
    if ( result != 0 )
        return result;

    assert( num_rows == 1 );
    assert( num_fields >= 1 );

    for ( DWORD i = 0; i < outCount + inoutCount; i ++ )
        ToVariant( parameters[ inCount + i ].bindValue, rows[i], lens[i], parameters[ inCount + i ].type ); 

    return 0;
}

lite::Variant Query::ProcedureHelper::GetField( int idx )
{
    if ( rows == NULL || lens == NULL || num_rows == 0 )
        throw lite::NullPointer();

    if ( ( unsigned long )idx >= num_fields )
        throw lite::InvalidConvert();

    MYSQL_FIELD *field = mysql_fetch_field_direct( res, idx );
    if ( field == NULL )
        throw lite::NullPointer();

    if ( lens[ idx ] == 0 )
    {
        static lite::Variant null;
        null.dataType = lite::Variant::VT_NULL;
        return null;
    }

    switch ( field->type )
    {
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_DATETIME:
        return lite::Variant( rows[ idx ] );

    case MYSQL_TYPE_LONG:
        return lite::Variant( atoi( rows[ idx ] ) );

    case MYSQL_TYPE_LONGLONG:
        return lite::Variant( _atoi64( rows[ idx ] ) );

    case MYSQL_TYPE_BLOB:
        return lite::Variant( ( void* )rows[ idx ], lens[ idx ] );
    }

    throw lite::InvalidConvert();

    static lite::Variant s_null;
    return s_null;
}

// 现在已经排除了 使用 @_0来作为返回值的逻辑了
lite::Variant & Query::ProcedureHelper::GetParam( int idx )
{
    if ( idx <= 0 || ( unsigned int )idx > totalCount )
        throw lite::InvalidConvert();

    return parameters[ idx - 1 ].bindValue;
}

int Query::ProcedureHelper::BindParam( int idx, lite::Variant &param )
{
    // 这个地方的 idx 是从 1 开始的， 0被默认了为标准返回值！
    if ( idx <= 0 || ( unsigned int )idx > totalCount )
        throw lite::InvalidConvert();

    parameters[ idx - 1 ].bindValue = param;
    return 0;
}

Query::ProcedureHelper::ProcedureHelper( Query &qry ) : 
    query( qry ) 
{
    memset( &parameters, 0, sizeof( parameters ) );
    totalCount = inCount = outCount = inoutCount = 0;
}

Query::ProcedureHelper::~ProcedureHelper()
{
}
