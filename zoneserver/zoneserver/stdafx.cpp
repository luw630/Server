// stdafx.cpp : 只包括标准包含文件的源文件
// 登陆服务器.pch 将成为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"
#include "pub/traceinfo.h"

// TODO: 在 STDAFX.H 中
//引用任何所需的附加头，而不是在此文件中引用

#include <eh.h>
#include <dbghelp.h>
#include <crtdbg.h>
#include <Shellapi.h>

#define MAXBUFFER 10240

extern void AddInfo(LPCSTR Info);
extern LPCSTR GetStringTime();

dwt::stringkey<char [256]> szIniFilename = "server.ini";
dwt::stringkey<char [256]> szDirname = "";

inline unsigned __int64 GetCpuCycle();
unsigned __int64 GetCpuSpeed();
unsigned __int64 CycleSeg();
unsigned __int64 CycleEnd();

unsigned __int32 seg_time;

class CDebug
{
public:
    CDebug()
    {
        // Get the current bits
        int tmp = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

        // Clear the upper 16 bits and OR in the desired freqency
        tmp = (tmp & 0x0000FFFF) | _CRTDBG_LEAK_CHECK_DF;

        // Set the new bits
        _CrtSetDbgFlag(tmp);

        seg_time = timeGetTime();

        // init cpu speed
        GetCpuSpeed();

        TraceInfo( "serverst.log", "[%s] zone[%s] start!", GetStringTime(), __argv[1] );
    }

    ~CDebug()
    {
        TraceInfo( "serverst.log", "[%s] zone[%s] close!", GetStringTime(), __argv[1] );
    }

    char buffer[MAXBUFFER+32];
    char FormatBuffer[MAXBUFFER+32];
} __dbend;

int &GetTraceType()
{
    static int nTraceType = 0;
    return nTraceType;
}

int &GetUseEndl()
{
    static int nUseEndl = 0;
    return nUseEndl;
}

bool &GetSkipMargin()
{
    static bool bSkipMargin = true;
    return bSkipMargin;
}

void trace(char lTraceType, char bUseEndl)
{
    //DWORD size = (DWORD)dwt::strlen(__dbend.buffer, MAXBUFFER);

    //// if (bUseEndl)
    //{
    //    __dbend.buffer[size++] = '\r';
    //    __dbend.buffer[size++] = '\n';
    //    __dbend.buffer[size++] = '\0';
    //}

    switch (lTraceType)
    {
    case 0:
#ifdef _DEBUG
        if (MessageBox(0, __dbend.buffer, "detected error, use break?", MB_YESNO) == IDYES)
            __asm int 3;
#else
        MessageBox(0, __dbend.buffer, "error info", 0);
#endif
        break;

    case 1:
        {
            static std::string filename = szDirname + "debug.txt";
            TraceInfoDirectly( filename.c_str(), __dbend.buffer );
#ifndef _XYDRELEASE_
			DWORD size = (DWORD)dwt::strlen(__dbend.buffer, MAXBUFFER);
			__dbend.buffer[size++] = '\r';
			__dbend.buffer[size++] = '\n';
			__dbend.buffer[size++] = '\0';
			AddInfo(__dbend.buffer);
#endif
        }
        //{
        //    static HANDLE hFile = INVALID_HANDLE_VALUE;
        //    if (hFile == INVALID_HANDLE_VALUE)
        //    {
        //        hFile = ::CreateFile(szDirname + "debug.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
        //        if (hFile == INVALID_HANDLE_VALUE)
        //            return;
        //        SetFilePointer(hFile, 0, 0, FILE_END);
        //    }
        //    // 不需要将\0写进去！
        //    WriteFile(hFile, __dbend.buffer, size-1, &size, 0);
        //}
        break;

    case 2:
        {
            DWORD size = (DWORD)dwt::strlen(__dbend.buffer, MAXBUFFER);
            __dbend.buffer[size++] = '\r';
            __dbend.buffer[size++] = '\n';
            __dbend.buffer[size++] = '\0';
            AddInfo(__dbend.buffer);
        }
        break;

    case 3:
        {
            static std::string filename = szDirname + "otherinfo.txt";
            TraceInfoDirectly( filename.c_str(), __dbend.buffer );
        }
        //{
        //    HANDLE hFile = ::CreateFile(szDirname + "otherinfo.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, NULL, NULL);
        //    if (hFile == INVALID_HANDLE_VALUE)
        //        return;
        //    SetFilePointer(hFile, 0, 0, FILE_END);
        //    WriteFile(hFile, __dbend.buffer, size-1, &size, 0);
        //    CloseHandle(hFile);
        //}
		break;

    case 4:
        {
            static std::string filename = szDirname + "actionlog.txt";

            LPCSTR _GetStringTime();
            TraceInfo( filename.c_str(), "%s %s", _GetStringTime(), __dbend.buffer );
        }
        //{
        //    static HANDLE hFile = INVALID_HANDLE_VALUE;
        //    if (hFile == INVALID_HANDLE_VALUE)
        //    {
        //        hFile = ::CreateFile(szDirname + "actionlog.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
        //        if (hFile == INVALID_HANDLE_VALUE)
        //            return;
        //        SetFilePointer(hFile, 0, 0, FILE_END);
        //    }
        //    if (GetUseEndl() != 2)
        //    {
        //        LPCSTR GetStringTime();
        //        LPCSTR time = GetStringTime();
        //        DWORD tl = dwt::strlen(time, 64);
        //        WriteFile(hFile, time, tl, &tl, 0);
        //        WriteFile(hFile, " -> ", 4, &tl, 0);
        //    }
        //    // 不需要将\0写进去！
        //    WriteFile(hFile, __dbend.buffer, size-1, &size, 0);
        //}
        break;
	case 5:
		{
			static std::string filename = szDirname + "GemExchangeInfo.txt";
			TraceInfo(filename.c_str(),"%s",__dbend.buffer);
		}
		break;
	case 6:
		{
			static std::string filename = szDirname + "GroundItemLogInfo.txt";
			LPCSTR _GetStringTime();
			//TraceInfo( filename.c_str(), "%s %s", _GetStringTime(), __dbend.buffer );
			TraceInfo(filename.c_str(),"%s",__dbend.buffer);
		}
		break;
    }
}

LPCSTR FormatString(LPCSTR szFormat, ...)
{
    va_list header;
    va_start(header, szFormat);
    _vsnprintf(__dbend.FormatBuffer, MAXBUFFER, szFormat, header);
    va_end(header);

    return __dbend.FormatBuffer;
}

BOOL rfalse(LPCSTR szFormat, ...)
{
    va_list header;
    va_start(header, szFormat);
    _vsnprintf(__dbend.buffer, MAXBUFFER, szFormat, header);
    va_end(header);

    trace(GetTraceType(), GetUseEndl());

    return FALSE;
}

BOOL rfalse(char lTraceType, char bUseEndl, LPCSTR szFormat, ...)
{
    GetTraceType() = lTraceType;
    GetUseEndl() = bUseEndl;

    va_list header;
    va_start(header, szFormat);
    _vsnprintf(__dbend.buffer, MAXBUFFER, szFormat, header);
    va_end(header);

    trace(GetTraceType(), GetUseEndl());

    return FALSE;
}

static unsigned __int64 CpuSpeed = 0;
static unsigned __int64 CycleCount[100] = { 0, };
static unsigned __int32 CallStackPos = 0;

__declspec (naked) unsigned __int64 GetCpuCycle() 
{ 
    _asm 
    {
        rdtsc
        ret
    }
}

__declspec (naked) unsigned __int64 GetStackValue() 
{
    _asm 
    {
        // locate dest temp data
        lea    ecx, CycleCount
        mov    ebx, CallStackPos
        imul   ebx, ebx, 8
        add    ecx, ebx

        // get the 2 cycle interval
        mov    eax, [ecx] 
        mov    edx, [ecx+4] 

        // if (CallStackPos != 0) CallStackPos --
        lea    ebx, CallStackPos
        test   [ebx], 0ffh
        jz    _reteaxandedx

        dec    [ebx]

        // clear temp data
_reteaxandedx:
        mov    [ecx], 0 
        mov    [ecx+4], 0 

        ret
    }
}

unsigned __int64 GetCpuSpeed()
{
    if (CpuSpeed == 0)
    {
        // 获取当前进程属性
        DWORD   PriorityClass = GetPriorityClass(GetCurrentProcess());
        int     Priority = GetThreadPriority(GetCurrentThread());

        // 设置当前进程为实时控制
        SetPriorityClass(GetCurrentProcess, REALTIME_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread, THREAD_PRIORITY_TIME_CRITICAL);

        // 等待系统调整
        Sleep(10);

        // 取CPU周期差
        CpuSpeed = GetCpuCycle();
        Sleep(1000);
        CpuSpeed = GetCpuCycle()-CpuSpeed;

        // 恢复当前进程的属性
        SetThreadPriority(GetCurrentThread(), Priority);
        SetPriorityClass(GetCurrentProcess(), PriorityClass);
    }

    return CpuSpeed;
}

unsigned __int64 CycleSeg()
{
    if (CallStackPos >= 100) { CallStackPos = 99; CycleCount[99] = 0; }
    if (CycleCount[CallStackPos] != 0) CallStackPos ++;

    // locate dest temp data
    _asm
    {
        lea    ecx, CycleCount
        mov    ebx, CallStackPos
        imul   ebx, ebx, 8
        add    ecx, ebx

        // get cpu cycle
        rdtsc

        // store cycle into temp data
        mov    [ecx], eax 
        mov    [ecx+4], edx 
    }
}

unsigned __int64 CycleEnd()
{
    if (CallStackPos >= 100) { CallStackPos = 99; }

    // get cpu cycle
    _asm 
    {
        rdtsc

        // locate dest temp data
        lea    ecx, CycleCount
        mov    ebx, CallStackPos
        imul   ebx, ebx, 8
        add    ecx, ebx

        // get the 2 cycle interval
        sub    eax, [ecx] 
        sbb    edx, [ecx+4] 

        // if (CallStackPos != 0) CallStackPos --
        lea    ebx, CallStackPos
        test   [ebx], 0ffh
        jz    _reteaxandedx

        dec    [ebx]

        // clear temp data
_reteaxandedx:
        mov    [ecx], 0 
        mov    [ecx+4], 0 
    }
}

void TraceMemory(LPVOID pDumpEntry, int iOffset, unsigned int uiSize, LPCSTR szFilename)
{
    HANDLE hFile = ::CreateFile(szDirname + ((szFilename == NULL)?"DumpMemory.Bin":szFilename), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

    DWORD dwSize = 0;
    SetFilePointer(hFile, 0, 0, FILE_END);
    WriteFile(hFile, "{[(Head)", sizeof("{[(Head)")-1, &dwSize, 0);

    LPBYTE pEntry = ((LPBYTE)pDumpEntry) + iOffset;
    if (!IsBadReadPtr(pEntry, uiSize))
    {
        WriteFile(hFile, pEntry, uiSize, &dwSize, 0);
    }
    else
    {
        WriteFile(hFile, &pEntry, 4, &dwSize, 0);
        WriteFile(hFile, "IsBadReadPtr", sizeof("IsBadReadPtr")-1, &dwSize, 0);
    }

    WriteFile(hFile, "(Tail)]}", sizeof("(Tail)]}")-1, &dwSize, 0);

	CloseHandle(hFile);
}

void DumpStack(LPCSTR info)
{
	char szFilename[256] = { 0 };

	SYSTEMTIME st;
	GetLocalTime(&st);
	_snprintf(szFilename, 250, "DUMPSTACK[%d-%d-%d][%d;%d;%d].LOG", 
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	extern BOOL TraceInfo(LPCSTR, LPCSTR, ...);

    __try
    {
    __eip:

        DWORD _EIP = 0;
        DWORD _ESP = 0;
        DWORD _EBP = 0;

        _asm
        {
            lea ecx, _EIP
            mov eax, __eip
            mov [ecx], eax

            lea ecx, _ESP
            mov [ecx], esp

            lea ecx, _EBP
            mov [ecx], ebp
        }

        STACKFRAME stackFrame;

	    ::ZeroMemory(&stackFrame, sizeof(stackFrame));

	    stackFrame.AddrPC.Offset = _EIP;
	    stackFrame.AddrPC.Mode = AddrModeFlat;

	    stackFrame.AddrStack.Offset = _ESP;
	    stackFrame.AddrStack.Mode = AddrModeFlat;

	    stackFrame.AddrFrame.Offset = _EBP;
	    stackFrame.AddrFrame.Mode = AddrModeFlat;

	    while (TRUE)
	    {
		    if (StackWalk(IMAGE_FILE_MACHINE_I386, ::GetCurrentProcess(), ::GetCurrentThread(),
			    &stackFrame, NULL, NULL, NULL, NULL, NULL) == FALSE)
			    break;

            if (stackFrame.AddrPC.Offset == _EIP)
            {
                if (info != NULL)
                    TraceInfo(szFilename, "%s", info);
                else
                    TraceInfo(szFilename, "DumpStack");
            }
            else
            {
                TraceInfo(szFilename, "0x%08x", stackFrame.AddrPC.Offset);
            }

            _asm nop
	    }
    }
    __finally
    {
        TraceInfo(szFilename, "End of DumpStack");
    }
}
