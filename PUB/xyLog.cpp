/**
** Author:	�˳�
** QQ:		23427470
** Mail:	aron_d@yeah.net
** Time:	
*  �汾	 :	
*  ����  :  ������־����
**/
#pragma once
#include "stdafx.h"
#include "xyLog.h"
static std::string logFileName = "xysj_log.txt";

void InitLog( const char* plogfile )
{
	if( plogfile!= NULL ) 
		logFileName = plogfile;
}
void PrintError(const char* a, const char* pFile, int line, const char* msg)
{
	static xysj::FreeLock cs;
	
	// ���̰߳�ȫ
	xysj::Lock<xysj::FreeLock> lock(cs);

	FILE *hf = NULL;
	hf = fopen(logFileName.c_str(), "a");
	if(!hf) return;

	if( NULL == msg ) msg="";

	char tmp[4096];
	tmp[0]='\0';

	sprintf(tmp, "%s [%s:��%d��] %s",a,pFile,line,msg);
	::OutputDebugString(tmp);

	fprintf(hf, tmp);
	time_t current_time;
	time(&current_time);
	fprintf(hf," --%s", ctime(&current_time));

	fclose(hf);
}