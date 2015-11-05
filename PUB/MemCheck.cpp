#include "stdafx.h"

#include <stdio.h>
#include <Mmsystem.h>
#include <dbghelp.h>
#include <Psapi.h>

#pragma comment (lib, "Winmm.lib")
#pragma comment (lib, "dbghelp.lib")
#pragma comment (lib, "Psapi.lib")
//20150423 wk 增加3倍
//#define DUMP_WORKINGSETSIZE     (800000000)
//#define CLOSE_WORKINGSETSIZE    (1000000000)

#define DUMP_WORKINGSETSIZE     (3200000000) 
#define CLOSE_WORKINGSETSIZE    (4000000000)

#define LOGFILE_NAME "MemoryLog.txt"

class CriticalLock
{
public:
    CriticalLock()
    {
        InitializeCriticalSection(&_Lock);
    }

    ~CriticalLock()
    {
        Lock();
        Unlock();
        DeleteCriticalSection(&_Lock);
    }

public:
    void Lock()
    {
        EnterCriticalSection(&_Lock);
    }

    void Unlock()
    {
        LeaveCriticalSection(&_Lock);
    }

private:
    CRITICAL_SECTION _Lock;
};

class   CLogFile
{
public:
    CLogFile(void)                      { pf = NULL; }
    CLogFile(const char* szFileName)    { pf = NULL;  Open(szFileName); };
    ~CLogFile(void)                     { Close(); };

    bool    Open(const char* szFileName)
    {
        if( pf )    return  true;

        pf = fopen( szFileName, "a+" );

        if( pf )    return  true;

        return  false;
    }

    void    Close(void)
    {
        if( pf )    fclose( pf );

        pf = NULL;
    }

    void    WriteLog(char* szStr,... )
    {
        if( pf )
        {
            static  char szShow[256];
            va_list header;
            va_start(header, szStr);
            vsprintf(szShow, szStr, header);
            va_end(header);

            fprintf( pf, szShow );
        }
    }

public:
    FILE *pf;
};

#ifndef _DEBUG

void MemoryCheck(DWORD dwSize);
void DumpStack(DWORD dwSize);

_C_LIB_DECL
int __cdecl _callnewh(size_t size) _THROW1(_STD bad_alloc);
_END_C_LIB_DECL

void *__cdecl operator new[](size_t count) _THROW1(std::bad_alloc)
{
    // try to allocate count bytes for an array
	return (operator new(count));
}
//std::bad_alloc		_STD bad_alloc
void *__cdecl operator new(size_t size) _THROW1(_STD bad_alloc)
{
    // try to allocate size bytes
    void *p;
    while ((p = malloc(size)) == 0)
	    if (_callnewh(size) == 0)
            break;

    if (p == 0)
    {
		//_STD _Nomemory();
// #if _MSC_VER>1500
//        throw bad_alloc();
// #else
// 		 _STD _Nomemory();
// #endif
		return NULL;
    }

    static CriticalLock lock;
    lock.Lock();
    MemoryCheck( (DWORD)size );
    lock.Unlock();

    return (p);
}

void MemoryCheck(DWORD dwSize)
{
    static  DWORD   dwLastTime = timeGetTime();
            DWORD   dwCurTime = timeGetTime();
    static  bool    bDumped = false;

    if( (int)(dwCurTime-dwLastTime) >= 1000 || bDumped )
    {
        dwLastTime = dwCurTime;

        PROCESS_MEMORY_COUNTERS MemInfo;
        if (!GetProcessMemoryInfo(GetCurrentProcess(), &MemInfo, sizeof(MemInfo)))
        {
            CLogFile LogFile( LOGFILE_NAME );
            LogFile.WriteLog( "GetProcessMemoryInfo fail!\r\n" );
            LogFile.Close();
            return;
        }

        if( MemInfo.WorkingSetSize>=DUMP_WORKINGSETSIZE )
        {
            // 转储
            DumpStack( dwSize );
            bDumped = true;
        }

        if( MemInfo.WorkingSetSize>=CLOSE_WORKINGSETSIZE )
        {
            CLogFile LogFile( LOGFILE_NAME );
            LogFile.WriteLog( "*** Process exit!\r\n" );
            LogFile.Close();
            extern LPCSTR GetMemCheckString();
            extern void AppQuit();
            if ( MessageBox( 0,"dump mem end to exit!!!","内存已经被耗尽，是否要退出游戏？",MB_YESNO ) == IDYES )
                AppQuit(); /*TerminateProcess( GetCurrentProcess(), 0 ); */
            else 
                MessageBox( 0,GetMemCheckString(),"尝试打印lua数据",0);
        }
    }
}

void DumpStack( DWORD dwSize )
{
__DumpStack_eip:
    DWORD _EIP = 0;
    DWORD _ESP = 0;
    DWORD _EBP = 0;
    _asm
    {
        lea ecx, _EIP
        mov eax, __DumpStack_eip
        mov [ecx], eax

        lea ecx, _ESP
        mov [ecx], esp

        lea ecx, _EBP
        mov [ecx], ebp
    }

    static STACKFRAME stackFrame;
	::ZeroMemory(&stackFrame, sizeof(stackFrame));
    stackFrame.AddrPC.Offset = _EIP;
	stackFrame.AddrStack.Offset = _ESP;
	stackFrame.AddrFrame.Offset = _EBP;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrStack.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Mode = AddrModeFlat;

    CLogFile LogFile( LOGFILE_NAME );

    static  int t;
    LogFile.WriteLog( "%05d -----------------\r\n", dwSize );
    t ++;

    int skip = 3;
	while (TRUE)
	{
		if (StackWalk(IMAGE_FILE_MACHINE_I386, ::GetCurrentProcess(), ::GetCurrentThread(),
			&stackFrame, NULL, NULL, NULL, NULL, NULL) == FALSE)
			break;

        if( stackFrame.AddrFrame.Offset==0 )    
            break;

        if (skip > 0)
        {
            skip--;
            continue;
        }

        LogFile.WriteLog( "[0x%08x]\r\n", stackFrame.AddrPC.Offset);
    }
}

std::string DumpStack()
{
__DumpStack_eip:
	DWORD _EIP = 0;
	DWORD _ESP = 0;
	DWORD _EBP = 0;
	_asm
	{
		lea ecx, _EIP
			mov eax, __DumpStack_eip
			mov [ecx], eax

			lea ecx, _ESP
			mov [ecx], esp

			lea ecx, _EBP
			mov [ecx], ebp
	}

	static STACKFRAME stackFrame;
	std::string result;
	::ZeroMemory(&stackFrame, sizeof(stackFrame));
	stackFrame.AddrPC.Offset = _EIP;
	stackFrame.AddrStack.Offset = _ESP;
	stackFrame.AddrFrame.Offset = _EBP;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrStack.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Mode = AddrModeFlat;

	int skip = 3;
	while (TRUE)
	{
		if (StackWalk(IMAGE_FILE_MACHINE_I386, ::GetCurrentProcess(), ::GetCurrentThread(),
			&stackFrame, NULL, NULL, NULL, NULL, NULL) == FALSE)
			break;

		if( stackFrame.AddrFrame.Offset==0 )    
			break;

		if (skip > 0)
		{
			skip--;
			continue;
		}

		char str[256];
		sprintf( str, "[0x%08x]\r\n", stackFrame.AddrPC.Offset );
		result += str;
	}
	return result;
}

#endif
