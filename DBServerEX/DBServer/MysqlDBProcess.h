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
            MYSQL_STMT      **stmt;         // Ԥ����ִ�о��
	        MYSQL_RES       *res;           // ���β���ִ�к󱣴������Ľ��
            MYSQL_BIND      *binds;         //
	        const void      *sql;           // ���β���ִ�к󱣴������Ľ��
	        my_ulonglong    nrow;           // ���β�����Ӱ������

        public:
	        my_ulonglong    num_rows;       // �����������������
	        unsigned int    num_fields;     // �������ݵ��ֶ��� 
	        unsigned int    cursor;         // ��ǰ�������α�[��ʵֻ��һ����¼����]
	        unsigned long   *lens;          // ��ǰ��ÿ���ֶ����ݵĳ��ȣ� 
	        MYSQL_ROW       rows;           // ��ǰ�е�����

            friend class Query;
        };

        // ͳһ�ýӿڵķ���ֵ��Ĭ������·��� 0 Ϊִ�гɹ���
        int ExecuteStmt( const void *sql, size_t size, QueryResult *qr, Arguments args = Arguments( 0, 0 ) );
        int ExecuteSql( const void *sql, size_t size, QueryResult *qr );

    public:
        class ProcedureHelper : public QueryResult
        {
            //��������
            struct SqlParam
            {
	            enum DIRECTION { V_IN, V_OUT, V_INOUT };

	            DIRECTION           direction;  // �������ݷ���
	            enum_field_types    type;	    // ��������
	            size_t              lens;	    // ��Ч�Ĳ������ȣ�������ַ�����Blob���ͣ�

                lite::Variant       bindValue;	// ����ֵ���������봫����
            };

            // ���ڶ���Ĵ洢����Ŀǰ���֧��16������
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
    	
	    //��ȡ���Ӷ���
	    const MYSQL* GetMySql();
	    //��ʼ�����ݿ�����
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
	    MYSQL       *m_pData; // msyql ���Ӿ�� 
	    MYSQL_STMT  *m_pstmt; // smt���
    public:
	    ErrMsg m_errmsg;

    private:
		DWORD prevCheckTime; //��һ�μ������ʱ��
		//--�����ӵ��ִ���������
		std::string m_Password;
		std::string m_Host;
		std::string m_User;		
		std::string m_DBName; // ��ǰ��Ĭ�����ݿ�
		int			m_Port;		//�˿ں�

	    void Clear();
	    // �������ݿ� ��Init�����ѵ���
	    bool Connect( LPCSTR host, LPCSTR user, LPCSTR passwd, LPCSTR db, 
		    UINT port = 3306, DWORD client_flag = 0 );
    };
};
