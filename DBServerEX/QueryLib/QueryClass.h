////////////////////////////////////////////////////////////
//	FileName		:	QueryClass.h                      //
//	FileAuthor		:	Luou                              //          
//	FileCreateDate	:	2003-9-26                         //
//	FileDescription	:	访问数据的底层，用ADO实现         //
//  Reviewed on 26th 2003                                 //
////////////////////////////////////////////////////////////
//One connection to SQL server for one Query class only 
#pragma once
#define UN_USE_ADONET

#include <atlstr.h>
#include <comdef.h>
#include <assert.h>
#import "c:\program files\common files\system\ado\msado15.dll" no_namespace  rename("EOF","adoEOF")

#define MAX_SQLSTRING_LEN 1024
#define MAX_LOGMSG_LEN 256

class AnsiStringC
{
    CString szString;

public:
    AnsiStringC();
    ~AnsiStringC();
	void Add(LPTSTR szAddString)
	{
		szString.Format(szAddString);
	}
    void AddString(CString AddString)
	{
			szString +=  szString;
	}
    CString GetString()
	{
		return szString;
	}
    void Clear()
	{
			szString.Empty();
	}
};

class Query
{
    CString m_szUserName;
    CString m_szPassWord;
    CString m_szDBIP;
    BSTR m_bstrSQL;

public:
    //Smart pointer for one connection
    _ConnectionPtr m_pConnection;
    //Smart pointer for the record set has been retrieved
    _RecordsetPtr m_pRecordset;
    //Smart pointer for sql commands 
    _CommandPtr m_pCommand;
    //Records has been affected in one database transaction
    _variant_t m_RecordsAffected;
    //Fields retrieved in a "select" transaction
#define Fields m_pRecordset->GetFields()

    _bstr_t ConnectionString;
    DWORD prevCheckTime;
    short Eof;
    //record counts for one transaction
    int iRecordCount;

    //Initialize function, with parameters to make the connection string
    BOOL Init(LPCTSTR UserName,LPCTSTR PassWord,LPCTSTR DBIP,LPCTSTR DBName);
	BOOL Init(LPCTSTR szConnectStr);
    BOOL UnInit();

    Query();
    ~Query();

    BOOL KeepAlive( DWORD timeMargin = 5000 );

    //The string to store sql commands
    AnsiStringC *m_SQL;
    //The string to store sql error messages
    AnsiStringC *m_ErrorString;

    void LogError(LPCTSTR FuncName,LPCTSTR Error);

    //Execute a SQL command with "select"
    BOOL Open();
	BOOL OpenNotCount();
    //Execute a SQL command without "select"
    BOOL ExecSQL();
    //Close the recordset after one execute and prepare for the next transaction
    void Close();
    void ReleaseRecord();
    //Check if the recordset is empty after Query::Open();
    BOOL IsEmpty();
    //Move to next record after Query::Open();
    BOOL Next();
    //Begin transaction ,this function will lock all the operations on the table
    //Even select
    BOOL BeginTransAction();
    //End transaction
    BOOL EndTransAction();
    //Roll transaction
    BOOL RollbackTransAction();


	// 注意，调用这个函数时要保证数据据集已处于编辑状态
	BOOL AddBlob( LPCSTR FieldName,  const LPVOID  pBuf ,DWORD dwBuflen );
	// 从数据字段中读取一个BLOB数据，由调用者传入一个分配好的BUF
	DWORD GetBolb(LPCSTR FieldName, LPVOID pBuf,const DWORD dwbuflen);
	void PrepareProcedure(LPCTSTR szProcName)
	{
		try
		{
			m_pCommand->ActiveConnection = m_pConnection;
			m_pCommand->CommandText = szProcName;
			m_pCommand->CommandType = adCmdStoredProc;		
			m_pCommand->Parameters->Refresh();//取出存储过程的参数列表	
			m_pReturnValues = Query::m_pCommand->Parameters->GetItem("@RETURN_VALUE");	

		}
		catch(_com_error &e)
		{
			LogError("Query::PrepareProcedure",e.Description()+szProcName);
		}
	}

	//执行不用返回数据集的SQL语句
	BOOL ExecCommandSQL( LPCSTR szSQL )
	{
		_bstr_t  bsql( szSQL  );
		try
		{
			m_pConnection->Execute( bsql,NULL,adCmdText);
		}
		catch(_com_error& e)
		{
			LogError("Query::ExecCommandSQL",e.Description() + szSQL);
			return FALSE;
		}
		return TRUE;
	}
	BOOL ReturnOK()
	{
		 int p = m_pReturnValues->Value.intVal;
		return m_pReturnValues->Value.intVal == 0;
	}
	int GetProcudureRetValue()
	{
		return m_pReturnValues->Value.intVal;
	}

private:
_ParameterPtr m_pReturnValues;//用于存放存储过程的返回值
};
