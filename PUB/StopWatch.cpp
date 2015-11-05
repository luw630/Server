// StopWatch.cpp: implementation of the CStopWatch class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


/*
//用于数据操作计时的类，主要是便于计算数据执行性能
static LARGE_INTEGER _tstart, _tend;
static LARGE_INTEGER freq;
void  tStart(void)
{
static int first = 1;

if(first) {
QueryPerformanceFrequency(&freq);
first = 0;
}
QueryPerformanceCounter(&_tstart);
}

double tEnd(void)
{ 
QueryPerformanceCounter(&_tend);
return ((double)_tend.QuadPart -
(double)_tstart.QuadPart)/((double)freq.QuadPart);
}
*/