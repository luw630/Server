/****************************************************************
//  FileName:   XYDDebug.cpp
//  Author:		Wonly
//  Create:		2003.5.26
****************************************************************/

#include "StdAfx.h"
#include "XYDDebug.h"

int     g_iComTestTime;
int     g_iTime1,g_iTime2;

static  __int64     _gCurTime[8];
__int64    StartGetTime(int id)
{
    _gCurTime[id] = GetCycleCount();
    return  _gCurTime[id];
}

int     EndGetTime(int id)
{
    static  __int64 i64CurTime;
    i64CurTime = GetCycleCount();
    return  (int)(i64CurTime - _gCurTime[id]);
}


OutPutDebug::OutPutDebug(void)
{
    m_pFt = NULL;
    m_Frame = 0;
}

OutPutDebug::~OutPutDebug(void)
{
    if( m_pFt )   fclose( m_pFt );
}

BOOL    OutPutDebug::Open(char *filename)
{
    if( (m_pFt=fopen(filename,"w"))==NULL )
        return  FALSE;
    else
        return  TRUE;
}

void    OutPutDebug::Close()
{
    if( m_pFt )   fclose( m_pFt );
    m_pFt = NULL;
}

void    OutPutDebug::PutString(char *str, ...)
{
    static  int     iFrame=m_Frame;
    static  DWORD   dwLastTick = GetTickCount();
    DWORD   dwCurTick = GetTickCount();
    iFrame = (dwCurTick-dwLastTick)/40;
    dwLastTick = dwCurTick;

    char    szShow[128];
    va_list header;
    va_start(header, str);
    vsprintf(szShow, str, header);
    va_end(header);

    //iFrame = m_Frame - iFrame;
    fprintf( m_pFt, "%d - %s\n", m_Frame, szShow );//iFrame
    iFrame = m_Frame;
}

void    OutPutDebug::Update()
{
    m_Frame ++;
}
