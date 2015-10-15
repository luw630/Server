////////////////////////////////////////////////////////////
//	FileName		:	QueryClass.cpp                    //
//	FileAuthor		:	Luou                              //          
//	FileCreateDate	:	2003-9-26                         //
//	FileDescription	:	访问数据的底层，用ADO实现         //
//  Reviewed on 26th 2003                                 //
////////////////////////////////////////////////////////////
#include "QueryClass.h"
#include <windows.h>

#undef Fields

#define _SafeDelete(p)				if ( p ) { delete p; p = NULL; }
AnsiStringC::AnsiStringC()
{
	szString.Empty();
}

AnsiStringC::~AnsiStringC()
{
	szString.Empty();
}

Query::Query()
{
	m_SQL = NULL;
	m_ErrorString = NULL; 
	m_pConnection = NULL;
	m_pRecordset = NULL;
	m_pCommand = NULL;
	::CoInitialize(NULL); 
	prevCheckTime = timeGetTime();
}

BOOL Query::UnInit()
{
	_SafeDelete ( m_SQL );
	_SafeDelete ( m_ErrorString )
	if( m_pConnection )
		m_pConnection->Close();

	m_pConnection = NULL;
	m_pRecordset = NULL;
	m_pCommand = NULL;

	return TRUE;
}

BOOL Query::KeepAlive( DWORD timeMargin )
{
	if ( (int)timeGetTime() - (int)prevCheckTime > (int)timeMargin )
	{
		prevCheckTime = timeGetTime() + timeMargin;

		try
		{
			//m_pConnection->Execute( "set datefirst 1", NULL, 0 ); 
			m_pConnection->Execute( " select 1", NULL, 0 ); 
		}
		catch ( _com_error)
		{
			try
			{
				if ( m_pConnection->State != adStateClosed )
					m_pConnection->Close();

				m_pConnection->Open( ConnectionString, "", "", adModeUnknown );
				m_pConnection->put_ConnectionTimeout(0);
				m_pConnection->put_CommandTimeout(0);
				m_pCommand->PutCommandTimeout(0);
				m_pCommand->ActiveConnection = m_pConnection;
				if (m_pConnection->State == adStateOpen )
				LogError( "Query::KeepAlive","Ado 连接断开, 重连接成功!"); else
				LogError( "Query::KeepAlive","Ado 连接断开, 重连接失败！！！");

			}
			catch(_com_error& e)
			{
				LogError( "Query::KeepAlive",e.ErrorMessage()+e.Description()+" Ado 重连接异常 " + m_SQL->GetString() + "\r\n" );
				return FALSE;
			}
		}
	}

	return TRUE;
}

BOOL Query::Init(LPCTSTR UserName,LPCTSTR PassWord,LPCTSTR DBIP,LPCTSTR DBName)
{
	m_szUserName = UserName;
	m_szPassWord = PassWord;
	m_szDBIP     = DBIP;
	CString temp;
	temp.Format("Provider=SQLOLEDB;Data Source=%s,1433;Network Library=DBMSSOCN;Initial Catalog=%s;uid=%s;pwd=%s;",m_szDBIP,DBName,m_szUserName,m_szPassWord);
	return Init(temp.GetBuffer());
}

Query::~Query()
{

	::CoUninitialize(); 
}

//Close the recordset
void Query::ReleaseRecord()
{
	if(m_pRecordset->State)
		m_pRecordset->Close();
	m_pRecordset = NULL;
}

//Close the recordset
void Query::Close()
{
	try
	{
		if(m_pRecordset->State)
		{
			m_pRecordset->Close();
			Eof = 0;
			m_SQL->Clear();
		}
	}
	catch(_com_error e)
	{
		LogError("Query::Close()",e.Description()+m_SQL->GetString());
	}
}

void Query::LogError(LPCTSTR FuncName,LPCTSTR Error)
{
	static char szLogMessage[4096];
	static char szLogTime[128];
	static SYSTEMTIME s;

	GetLocalTime(&s);

	sprintf(szLogTime, "%d-%d-%d %d:%d:%d::", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);

	strcpy(szLogMessage,szLogTime);
	strcat(szLogMessage,FuncName);
	strcat(szLogMessage,Error);
	HANDLE hFile = ::CreateFile("DebugQuery.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		// MessageBox(0, "无法打开DebugQuery.txt文件！", "=.=", 0);
		return;
	}

	DWORD size = (DWORD)strlen(szLogMessage);

	if (size < 4092)
	{
		szLogMessage[size++] = '\r';
		szLogMessage[size++] = '\n';
		szLogMessage[size] = '\0';
	}

	SetFilePointer(hFile, 0, 0, FILE_END);
	WriteFile(hFile, szLogMessage, size, &size, 0);
	CloseHandle(hFile);
}

//Exec the SQL commands
BOOL  Query::ExecSQL()
{
	iRecordCount = 0;
	try
	{
		m_bstrSQL = m_SQL->GetString().AllocSysString(); 
		if(m_pRecordset->State)
			m_pRecordset->Close();
		m_pConnection->Execute(m_bstrSQL,&m_RecordsAffected,adCmdText);
		::SysFreeString(m_bstrSQL);
		iRecordCount = m_RecordsAffected.lVal;

	}
	catch(_com_error& e)
	{
		LogError("Query::ExecSQL()",e.Description()+m_SQL->GetString());
		return FALSE;
	}
	return TRUE;
}



BOOL Query::Open()
{
	try
	{
		iRecordCount = 0;
		m_bstrSQL = m_SQL->GetString().AllocSysString(); 
		if(m_pRecordset->State)
		{
			//m_pRecordset->Fields->Release();
			//if(Fields)
			//{
			//	if(Fields->Count>0)
			//		Fields->Release();
			//}
			m_pRecordset->Close();
		}
		m_pRecordset->Open(m_bstrSQL, m_pConnection.GetInterfacePtr(),adOpenDynamic,adLockOptimistic,adCmdText); 

		::SysFreeString(m_bstrSQL);
		while(!m_pRecordset->adoEOF)
		{
			iRecordCount++;
			m_pRecordset->MoveNext();
		}

		if(iRecordCount!=0)
		{ 
			m_pRecordset->MoveFirst();
			//Fields = m_pRecordset->GetFields();
			Eof = m_pRecordset->adoEOF;
		}
	}
	catch(_com_error e)
	{
		LogError("Query::Open",e.Description()+m_SQL->GetString());
		return FALSE;
	}
	return TRUE;
}
BOOL Query::OpenNotCount()
{
	try
	{
		iRecordCount = 0;
		m_bstrSQL = m_SQL->GetString().AllocSysString(); 
		if(m_pRecordset->State)
		{
			//m_pRecordset->Fields->Release();
			//if(Fields)
			//{
			//	if(Fields->Count>0)
			//		Fields->Release();
			//}
			m_pRecordset->Close();
		}
		m_pRecordset->Open(m_bstrSQL, m_pConnection.GetInterfacePtr(),adOpenDynamic,adLockOptimistic,adCmdText); 

		::SysFreeString(m_bstrSQL);
		m_pRecordset->MoveFirst();
		//Fields = m_pRecordset->GetFields();
		Eof = m_pRecordset->adoEOF;
	}
	catch(_com_error e)
	{
		LogError("Query::Open",e.Description()+m_SQL->GetString());
		return FALSE;
	}
	return TRUE;
}
BOOL Query::Next()
{
	HRESULT hr;
	try 
	{
		hr = m_pRecordset->MoveNext();
		//Fields = m_pRecordset->GetFields();
		Eof = m_pRecordset->adoEOF;
	}
	catch(_com_error e)
	{
		LogError("Query::Next()",e.Description()+m_SQL->GetString());
		return FALSE;
	}
	return TRUE;

}

BOOL Query::IsEmpty()
{
	return 0 == iRecordCount ? TRUE:FALSE;
}

BOOL Query::BeginTransAction()
{

	try
	{
		if(m_pConnection->State == adStateOpen)
			m_pConnection->BeginTrans();   
	}
	catch(_com_error &e)
	{
		LogError("Query::BeginTransAction",e.Description()+m_SQL->GetString());
		return FALSE;
	}
	return TRUE;

}

BOOL Query::EndTransAction()
{
	try
	{
		if(m_pConnection->State == adStateOpen)
			m_pConnection->CommitTrans();
	}
	catch(_com_error e)
	{
		LogError("Query::EndTransAction",e.Description()+m_SQL->GetString());
		return FALSE;
	}
	return TRUE;
}

BOOL Query::RollbackTransAction()
{
	try
	{
		if(m_pConnection->State == adStateOpen)
			m_pConnection->RollbackTrans();
	}
	catch(_com_error e)
	{
		LogError("Query::RollbackTransAction",e.Description()+m_SQL->GetString());
		return FALSE;
	}
	return TRUE;
}


BOOL Query::Init(LPCTSTR szConnectStr)
{
	ConnectionString = szConnectStr;
	m_SQL = new AnsiStringC();
	m_ErrorString = new AnsiStringC();

	m_pCommand.CreateInstance(_uuidof(Command));
	m_pConnection.CreateInstance(_uuidof(Connection)); //初始化Connection指针
	m_pRecordset.CreateInstance(_uuidof(Recordset));  //初始化Recordset指针

	try
	{
		//Make the connection string and connet
		m_pConnection->Open( ConnectionString, "", "", adModeUnknown ); 
		m_pConnection->put_ConnectionTimeout(0);
		m_pConnection->put_CommandTimeout(0);
		m_pCommand->PutCommandTimeout(0);
        m_pCommand->ActiveConnection = m_pConnection;
		return ( m_pConnection->State == adStateOpen );
	}
	catch(_com_error& e)
	{
		m_pRecordset = NULL;
		m_pConnection = NULL;
			

		//bactq  modify at 2007-04-23
		//MessageBox(NULL,e.ErrorMessage()+e.Description() ,"DBError",0);
		//PostQuitMessage(0);

		LogError("Query::Init",e.ErrorMessage()+e.Description()+" 初始化数据库连接失败，");
		return FALSE;
	}
	return TRUE;
}


BOOL Query::AddBlob( LPCSTR FieldName,const LPVOID pBuf ,DWORD dwBuflen)
{

	if( pBuf &&  !IsBadReadPtr( pBuf, dwBuflen) &&( m_pRecordset->State  != adStateClosed ))
	{
		try
		{
		VARIANT varBLOB;
		SAFEARRAY tempSA;
		tempSA.cbElements = 1;
		tempSA.cDims = 1;
		tempSA.cLocks = 0;
		tempSA.fFeatures = FADF_HAVEVARTYPE;
		tempSA.pvData = pBuf;
		tempSA.rgsabound[0].cElements = dwBuflen;
		tempSA.rgsabound[0].lLbound = 0;
		
		varBLOB.vt = VT_ARRAY | VT_UI1; 
		varBLOB.parray = &tempSA;
		m_pRecordset->GetFields()->GetItem( FieldName)->Value = varBLOB;
	   // SUCCEEDED( m_pRecordset->GetFields()->GetItem( FieldName)->Value = varBLOB );
		return TRUE;
		//return SUCCEEDED( m_pRecordset->Update());
		}
		catch( _com_error &e)
		{
			LogError("Query::AddBlob",e.Description()+m_SQL->GetString());
		}
	}
	 

	return FALSE;
}

DWORD Query::GetBolb(LPCSTR FieldName, LPVOID pBuf,const DWORD dwbuflen)
{
	if( IsEmpty() )
		return FALSE;

	try
	{
		DWORD dwDataSize = m_pRecordset->GetFields()->GetItem( FieldName )->ActualSize;//取到数据块大小
		if (( dwDataSize > 0 ) && ( dwDataSize <= dwbuflen ) )
		{
			_variant_t		varBLOB;
			varBLOB = m_pRecordset->GetFields()->GetItem( FieldName )->Value;
			if(varBLOB.vt == (VT_ARRAY | VT_UI1))
			{
				assert( pBuf != NULL);
				memset(pBuf,0,dwbuflen);
				memcpy( pBuf,varBLOB.parray->pvData, dwDataSize );				///复制数据到缓冲区					
				return dwDataSize;

			}
		}
	}
	catch ( _com_error &e) 
	{
		LogError("Query::GetBolb",e.Description()+m_SQL->GetString());
	}

	return FALSE;
}

 