/**
** Author:	�˳�
** QQ:		23427470
** Mail:	aron_d@yeah.net
** Time:	
*  �汾	 :	
*  ����  :  ������־����
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
			// ��ĵط� lock ס��, �������߳̽�����,nLock��ֵ����0,���Ե�InterlockedExchange������0ʱ,��ʾ�����߳��Ѿ�����.
			while (InterlockedExchange(&nLock,id)) {
				// �ȴ���ĵط� Unlock
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
//�Զ���
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