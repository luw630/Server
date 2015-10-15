#include "stdafx.h"
#include <assert.h>
#include <time.h>
#include <windows.h>
#include "Random.h"

// 以下函数内部都有static变量，不符合可重入函数的条件，但多线程可以安全的使用
// 因为static变量本身的值不代表什么精确的意义，就是随机值而已
INT32 CRandom::Random()
{
	static UINT32 prevSeed = (UINT32)::timeGetTime();
	UINT32 seed = (UINT32)(time(NULL)) ^ (UINT32)(::timeGetTime()) ^ prevSeed;

	return (((prevSeed = seed*214013L+2531011L) >> 16) & 0x7FFF);
}

INT32 CRandom::SimpleRandom()
{
	static UINT32 prevSeed = (UINT32)::timeGetTime();
	UINT32 seed = (UINT32)(time(NULL)) ^ (UINT32)(::timeGetTime()) ^ prevSeed;

	return (((prevSeed = seed+2531011L) >> 3) & 0x7FFF);
}

INT32 CRandom::ComplexRandom()
{
	static UINT32 prevSeed = (UINT32)::timeGetTime();
	UINT32 seed = (UINT32)(time(NULL)) ^ (UINT32)(::timeGetTime()) ^ prevSeed;

	UINT32	next = seed;
	INT32	result = 0;
	next *= 1103515245; next += 12345; result = (UINT32)(next/65536)%2048;
	next *= 1103515245; next += 12345; result <<= 10; result ^= (UINT32)(next/65536)%1024;
	next *= 1103515245; next += 12345; result <<= 10; result ^= (UINT32)(next/65536)%1024;
	prevSeed = next;

	return result;
}

INT32 CRandom::RandRange(INT32 lower, INT32 upper)
{
	if (lower == upper)
		return lower;
	else
	{
		INT32 bigger = lower > upper ? lower : upper;
		INT32 smaller = lower > upper ? upper : lower;

		bigger += 0-smaller;
		return (CRandom::Random()%(bigger+1)) + smaller;
	}
}