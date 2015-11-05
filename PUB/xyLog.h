/**
** Author:	邓超
** QQ:		23427470
** Mail:	aron_d@yeah.net
** Time:	
*  版本	 :	
*  描述  :  断言日志工具
**/
#pragma once
#include <assert.h>
#include <crtdbg.h> 
#include <process.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>

#ifdef XYASSERT
#define _xyassert(a,msg) assert(a)
#else
#define _xyassert(a,msg) if (!(a)) PrintError(#a,__FILE__,__LINE__,msg) 
#endif

#define xyassert(a) _xyassert(a,0)
#define xyassertmsg(a,msg) _xyassert(a,msg)
namespace xysj{
class FreeLock{
public:
	FreeLock( ){ nLock = 0; }

	void    Enter( )
	{	
		long id = (long)::GetCurrentThreadId( );
		long atom = InterlockedExchange(&nLock,id);
		if (atom && atom!=id) {
			// 别的地方 lock 住了, 当其它线程解锁后,nLock的值会变回0,所以当InterlockedExchange交换出0时,表示其它线程已经解锁.
			while (InterlockedExchange(&nLock,id)) {
				// 等待别的地方 Unlock
				Sleep(1);
			}
		}

	}

	void    Leave( )
	{
		InterlockedExchange(&nLock,0);
	}
private:
	long     nLock;
};
//自动锁
template< typename CS >
class Lock
{
public:
	Lock(CS &cs)
	{
		m_pCs = &cs;
		m_pCs->Enter();
	}
	virtual ~Lock()
	{
		m_pCs->Leave(); m_pCs = NULL;
	}
private:
	CS *m_pCs;
};

}
void InitLog( const char* plogfile );
void PrintError(const char* a, const char* pFile, int line, const char* msg);