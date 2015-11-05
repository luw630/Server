#include "stdafx.h"
#include <stdio.h>
#include <fstream>
#include "traceinfo.h"
#include "dwt.h"
#include <string>
#include "./ConstValue.h"

const int MAXFILENAME = 64;     //文件名大小
const int MAXBUFFSIZE = 10240;   //单条信息大小

static LPCSTR GetStringTime()
{
    static char buffer[1024];

    SYSTEMTIME s;
    GetLocalTime(&s);

    sprintf(buffer, "%d-%02d-%02d %02d:%02d:%02d", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);

    return buffer;
}

struct STraceFile
{
	std::ofstream *m_pOutFile;

	STraceFile()
	{
		m_pOutFile=NULL;
	};
};

class CTraceInfo
{
public:
	BOOL OutTraceInfo(LPCSTR m_szFileName, LPCSTR szInfo);
	BOOL OutTraceData(LPCSTR szFileName, LPVOID pData, DWORD dwSize);

public:
	CTraceInfo();
	~CTraceInfo();

private:
	std::map<std::string,STraceFile> m_mapOutFile;
	std::map<std::string,STraceFile> m_mapOutFileBinary;

};

CTraceInfo& Singletion()
{
    static CTraceInfo uniqueThis;
    return uniqueThis;
}

#define g_TraceInfo Singletion()

//---------------------------------------------------------------------
//函数：TraceInfo
//功能：带有时间前缀的跟踪信息存储到指定的文件
//参数：szFileName:指定文件名
//	   szFormat:格式化的待存储信息
//返回值：TRUE为成功 FALSE为失败
//---------------------------------------------------------------------
BOOL TraceData(LPCSTR szFileName, LPVOID pData, DWORD dwSize)
{
    if ( dwt::IsBadStringPtr( szFileName, MAXFILENAME ) )
		return FALSE;

	return g_TraceInfo.OutTraceData(szFileName,pData,dwSize);
};

BOOL TraceInfoDirectly( LPCSTR szFileName, LPCSTR szInfo )
{
	if ( dwt::IsBadStringPtr( szFileName, MAXFILENAME ) )
		return FALSE;

	return g_TraceInfo.OutTraceInfo( szFileName, szInfo );
};

__declspec( thread ) char szBuff[MAXBUFFSIZE] = { 0 };

BOOL TraceInfo(LPCSTR szFileName, LPCSTR szFormat, ...)
{
	va_list arg;
	if ( dwt::IsBadStringPtr( szFileName, MAXFILENAME ) )
		return FALSE;

	va_start(arg,szFormat);
	_vsnprintf(szBuff, MAXBUFFSIZE - 1, szFormat,arg);
    szBuff[ MAXBUFFSIZE - 1 ] = 0;
	va_end(arg);

	return g_TraceInfo.OutTraceInfo(szFileName,szBuff);
};

BOOL TraceInfo_C( LPCSTR szFileName, LPCSTR szFormat, ... )
{
	va_list arg;
	if ( dwt::IsBadStringPtr( szFileName, MAXFILENAME ) )
		return FALSE;

	va_start( arg, szFormat );
	_vsnprintf( szBuff, MAXBUFFSIZE - 1, szFormat, arg );
    szBuff[ MAXBUFFSIZE - 1 ] = 0;
	va_end(arg);

	std::ofstream soutfile( szFileName, std::ios_base::app );
	if ( !soutfile.is_open() )
		return FALSE;

    soutfile << "[" << GetStringTime() << "] " << szBuff << std::endl;
    return TRUE;
}

BOOL TraceMsgInfo_C( LPCSTR szFileName, LPCSTR szFormat, ... ) //密聊记录专用信息保存
{
	va_list arg;
	char tmpName[CONST_USERNAME];
	if ( dwt::IsBadStringPtr( szFileName, MAXFILENAME ) )
		return FALSE;
	
	va_start( arg, szFormat );
	_vsnprintf( szBuff, MAXBUFFSIZE - 1, szFormat, arg );
	szBuff[ MAXBUFFSIZE - 1 ] = 0;
	va_end(arg);

	std::ofstream soutfile( szFileName, std::ios_base::app );
	if ( !soutfile.is_open() )
		return FALSE;

	dwt::strcpy(tmpName, szBuff, CONST_USERNAME);
	soutfile << tmpName << " " << GetStringTime() << &szBuff[10] << std::endl;
	return TRUE;
}


BOOL TraceInfoDirectly_C( LPCSTR szFileName, LPCSTR szInfo )
{
	std::ofstream soutfile( szFileName, std::ios_base::app );
	if ( !soutfile.is_open() )
		return FALSE;

    soutfile << "[" << GetStringTime() << "] " << szInfo << std::endl;
    return TRUE;
}


CTraceInfo::CTraceInfo()
{
	m_mapOutFile.clear();
	m_mapOutFileBinary.clear();
};

CTraceInfo::~CTraceInfo()
{
	std::map<std::string,STraceFile>::iterator it;

	it=m_mapOutFile.begin();

	while(it != m_mapOutFile.end())
	{
		it->second.m_pOutFile->close();

		delete it->second.m_pOutFile;

		it++;
	}

	m_mapOutFile.clear();

	it=m_mapOutFileBinary.begin();

	while(it != m_mapOutFileBinary.end())
	{
		it->second.m_pOutFile->close();

		delete it->second.m_pOutFile;

		it++;
	}

	m_mapOutFileBinary.clear();
};

BOOL CTraceInfo::OutTraceInfo(LPCSTR m_szFileName, LPCSTR szInfo)
{
	if (szInfo == NULL)
		return FALSE;

	std::map<std::string,STraceFile>::iterator it;

	it=m_mapOutFile.find(m_szFileName);

	if (it != m_mapOutFile.end())
	{
		if (!(it->second.m_pOutFile)->is_open())
		{
			m_mapOutFile.erase(it);

			return FALSE;
		}

		*(it->second.m_pOutFile)<<szInfo<<std::endl;
	}
	else
	{
		STraceFile soutfile;

		soutfile.m_pOutFile=new (std::ofstream);

		soutfile.m_pOutFile->open(m_szFileName,std::ios_base::app);

		if (!soutfile.m_pOutFile->is_open())
		{
			return FALSE;
		}


		*(soutfile.m_pOutFile)<<szInfo<<std::endl;

		m_mapOutFile.insert(std::map<std::string,STraceFile>::value_type(m_szFileName,soutfile));
	}
	
	return TRUE;
};

BOOL CTraceInfo::OutTraceData(LPCSTR szFileName,LPVOID pData,DWORD dwSize)
{
	std::map<std::string,STraceFile>::iterator it;

	it=m_mapOutFileBinary.find(szFileName);

	if (it != m_mapOutFileBinary.end())
	{
		if (!(it->second.m_pOutFile)->is_open())
		{
			m_mapOutFile.erase(it);

			return FALSE;
		}

		char *p=(char*)pData;

		(it->second.m_pOutFile)->write(p,dwSize);
        (it->second.m_pOutFile)->flush();
	}
	else
	{
		STraceFile soutfile;

		soutfile.m_pOutFile=new (std::ofstream);

		soutfile.m_pOutFile->open(szFileName,std::ios_base::app|std::ios_base::binary);

		if (!soutfile.m_pOutFile->is_open())
		{
			return FALSE;
		}

		char *p=(char*)pData;

		soutfile.m_pOutFile->write(p,dwSize);
        soutfile.m_pOutFile->flush();

		m_mapOutFileBinary.insert(std::map<std::string,STraceFile>::value_type(szFileName,soutfile));
	}

	return TRUE;
}
