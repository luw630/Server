#pragma once
#define __LCC__
#include <iostream>
#include <strstream>
#include "mysql.h"
#pragma comment( lib, "libmysql.lib")

#include "liteserializer/lite.h"

class ErrMsg 
{
private :
	char  out[5124];
public :
	void ShowMsg( int nflag ,const char* szOutput, ... );
	void Save();
};

namespace mysql
{
    class Query
    {
    public:
        class TempBind
        {
        public:
            TempBind( MYSQL_BIND &a, my_bool &b, unsigned long &c );

            BOOL IsNull() { return null; }
            size_t Length() { return lens; }

            void BindBlob( void *data, size_t size ); 
            void BindString( const char *data, size_t size );
            void BindLong( long *data ); 
            void BindULong( unsigned long *data );
            void BindLongLong( __int64 *data ); 
            void BindULongLong( unsigned __int64 *data );
            void BindUTiny( unsigned char *data );

        private:
	        MYSQL_BIND      &bind;
	        my_bool         &null;
	        unsigned long   &lens;
        };

        struct Arguments
        {
            Arguments( size_t a, MYSQL_BIND *b ) : size( a ), array( b ) {}
            size_t      size;
            MYSQL_BIND *array;
        };

        template < int nbind > class QueryBind
        {
        public:
            QueryBind() { memset( this, 0, sizeof *this ); }
            TempBind operator [] ( int i ) { return TempBind( bind[ i ], null[ i ], lens[ i ] ); }
            operator MYSQL_BIND* () { return bind; }
            operator Arguments () { return Arguments( nbind, bind ); }

        private:
	        MYSQL_BIND      bind[ nbind ];
	        unsigned long   lens[ nbind ];
	        my_bool         null[ nbind ];
        };

        struct QueryResult
        {
            ~QueryResult() { Release(); }
            QueryResult()  { memset( this, 0, sizeof *this ); }
            void Reset() { Release(); memset( this, 0, sizeof *this ); }
            void Release();
            int AffectedRows();
            int FetchResult( MYSQL_BIND *bind );
            int FetchResult();

        protected:
            MYSQL_STMT      **stmt;         // 预处理执行句柄
	        MYSQL_RES       *res;           // 本次操作执行后保存下来的结果
            MYSQL_BIND      *binds;         //
	        const void      *sql;           // 本次操作执行后保存下来的结果
	        my_ulonglong    nrow;           // 本次操作的影响行数

        public:
	        my_ulonglong    num_rows;       // 结果集的数据总行数
	        unsigned int    num_fields;     // 单个数据的字段数 
	        unsigned int    cursor;         // 当前行数据游标[其实只是一个记录数据]
	        unsigned long   *lens;          // 当前行每个字段数据的长度！ 
	        MYSQL_ROW       rows;           // 当前行的数据

            friend class Query;
        };

        // 统一该接口的返回值，默认情况下返回 0 为执行成功！
        int ExecuteStmt( const void *sql, size_t size, QueryResult *qr, Arguments args = Arguments( 0, 0 ) );
        int ExecuteSql( const void *sql, size_t size, QueryResult *qr );

    public:
        class ProcedureHelper : public QueryResult
        {
            //参数定义
            struct SqlParam
            {
	            enum DIRECTION { V_IN, V_OUT, V_INOUT };

	            DIRECTION           direction;  // 参数传递方向
	            enum_field_types    type;	    // 参数类型
	            size_t              lens;	    // 有效的参数长度（针对于字符串或Blob类型）

                lite::Variant       bindValue;	// 参数值（包括传入传出）
            };

            // 现在定义的存储过程目前最多支持16个参数
	        enum { MAX_PARAM_COUNT = 16 };

	        Query &query;

            std::string strProc;
	        std::string strArgs;

	        DWORD totalCount;
	        DWORD inCount;
	        DWORD outCount;
	        DWORD inoutCount;

	        SqlParam parameters[ MAX_PARAM_COUNT ];	

        public:
	        int Prepare( LPCSTR procName );
	        int Execute();

            int BindParam( int idx, lite::Variant &lvt );
            lite::Variant& GetParam( int idx );
            lite::Variant GetField( int idx );

	        ProcedureHelper( Query &qry );
	        ~ProcedureHelper();
        };

    public:
	    enum { MAXBINDS = 10 };

	    enum { EDEBUG, EMSGBOX, EMTHER, };
	    Query(void);
	    ~Query(void);
    	
	    //获取连接对象
	    const MYSQL* GetMySql();
	    //初始化数据库连接
	    bool Init( LPCTSTR UserName, LPCTSTR PassWord, LPCTSTR DBIP, LPCTSTR DBName, DWORD dwPort=3306 );
	    void LogError(LPCTSTR FuncName,LPCTSTR Error);
	    int  ExecuteSQL( LPCSTR sql, MYSQL_RES **res = NULL );
	    int  ExecuteSqlCmd( LPCSTR sql); 
	    BOOL KeepAlive( DWORD timeMargin = 5000, LPCSTR who = NULL );
	    void UnInit();
		int Init( LPCSTR ConfigFileName);
        LPCSTR GetLastError()  { return mysql_error( m_pData ); }
        DWORD GetLastErrorId() { return mysql_errno( m_pData ); }

	    LPCSTR GetDBName() { return m_DBName.c_str(); };
        void Backup( LPCSTR path, LPCSTR title );

    protected :
	    MYSQL       *m_pData; // msyql 连接句柄 
	    MYSQL_STMT  *m_pstmt; // smt句柄
    public:
	    ErrMsg m_errmsg;

    private:
		DWORD prevCheckTime; //上一次检测连接时间
		//--将连接的字串保存下来
		std::string m_Password;
		std::string m_Host;
		std::string m_User;		
		std::string m_DBName; // 当前的默认数据库
		int			m_Port;		//端口号

	    void Clear();
	    // 连接数据库 在Init里面已调用
	    bool Connect( LPCSTR host, LPCSTR user, LPCSTR passwd, LPCSTR db, 
		    UINT port = 3306, DWORD client_flag = 0 );
    };
};
