// StopWatch.cpp: implementation of the CStopWatch class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


/*
//�������ݲ�����ʱ���࣬��Ҫ�Ǳ��ڼ�������ִ������
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