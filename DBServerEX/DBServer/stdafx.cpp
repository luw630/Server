// stdafx.cpp : 只包括标准包含文件的源文件
// DBServer.pch 将成为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO: 在 STDAFX.H 中
//引用任何所需的附加头，而不是在此文件中引用

#include <crtdbg.h>
#include <Shellapi.h>

inline unsigned __int64 GetCpuCycle();
unsigned __int64 GetCpuSpeed();
unsigned __int64 CycleSeg();
unsigned __int64 CycleEnd();
#define MAXBUFFER 256

DWORD dwNetRunLimit = 1000;

LPCSTR GetStringTime()
{
    static char buffer[1024];

    SYSTEMTIME s;
    GetLocalTime(&s);

    sprintf(buffer, "%d-%d-%d %d:%d:%d", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);

    return buffer;
}

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

        // init cpu speed
        GetCpuSpeed();

        // 装载异常捕获器
        // __STException::install(true);

        //_CrtSetBreakAlloc(130);
        dwDebugFileSize = 0;
        HANDLE hFile = ::CreateFile("debug.txt", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, NULL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            SetFilePointer(hFile, 0, 0, FILE_END);
            DWORD size = sizeof("start!!!!!\r\n");
            WriteFile(hFile, "start!!!!!\r\n", size, &size, 0);
            dwDebugFileSize = ::GetFileSize(hFile, &dwDebugFileSize);
            CloseHandle(hFile);
        }
    }
    ~CDebug()
    {

        HANDLE hFile = ::CreateFile("debug.txt", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, NULL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD temp;
            temp = ::GetFileSize(hFile, &temp);
            CloseHandle(hFile);
            if (dwDebugFileSize != temp)
            {
                if (MessageBox(0, "debug.txt文件的长度改变了！需要打开察看吗", "@.@", MB_YESNO) == IDYES)
                {
                    ShellExecute(::GetActiveWindow(), "open", "debug.txt", NULL, NULL, SW_SHOWNORMAL);
                }
            }
        }
        _CrtDumpMemoryLeaks();
    }

    char buffer[1024];
    DWORD dwDebugFileSize;
} __dbend;

int &GetTraceType()
{
    static int nTraceType = 0;
    return nTraceType;
}
void TraceMemory(LPVOID pDumpEntry, int iOffset, unsigned int uiSize, LPCSTR szFilename)
{
    try
    {
        HANDLE hFile = ::CreateFile((szFilename == NULL)?"DumpMemory.Bin":szFilename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, NULL, NULL);
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
            WriteFile(hFile, "IsBadReadPtr", sizeof("IsBadReadPtr")-1, &dwSize, 0);
        }

        WriteFile(hFile, "(Tail)]}", sizeof("(Tail)]}")-1, &dwSize, 0);

		CloseHandle(hFile);
    }
    catch (...)
    {
    }
}

int &GetUseEndl()
{
    static int nUseEndl = 0;
    return nUseEndl;
}

void trace(char lTraceType, char bUseEndl)
{
    DWORD size = (DWORD)dwt::strlen(__dbend.buffer, MAXBUFFER);

    switch (lTraceType)
    {
    case 0:
        {
#ifdef _DEBUG
            if (MessageBox(0, __dbend.buffer, "detected error, use break?", MB_YESNO) == IDYES)
            {
                __asm int 3;
            }
#else
            //MessageBox(0, __dbend.buffer, "error info", 0);
#endif
        }
        break;
    case 1:
        {
            static HANDLE hFile = INVALID_HANDLE_VALUE;

            if (hFile == INVALID_HANDLE_VALUE)
            {
                hFile = ::CreateFile("debug.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);

                if (hFile == INVALID_HANDLE_VALUE)
                    return;

                SetFilePointer(hFile, 0, 0, FILE_END);
            }

            // 不需要将\0写进去！
            WriteFile(hFile, __dbend.buffer, size, &size, 0);
            WriteFile(hFile, "\r\n", 2, &size, 0);
        }
        break;
    case 2:
        {
            extern void AddInfo(LPCSTR Info);
            AddInfo(__dbend.buffer);
            if (bUseEndl)
                AddInfo("\r\n");
        }
        break;
    }
}

bool &GetSkipMargin()
{
    static bool bSkipMargin = true;
    return bSkipMargin;
}

BOOL rfalse(LPCSTR szFormat, ...)
{
    va_list header;
    va_start(header, szFormat);
    vsprintf(__dbend.buffer, szFormat, header);
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
    vsprintf(__dbend.buffer, szFormat, header);
    va_end(header);

    trace(GetTraceType(), GetUseEndl());

    return FALSE;
}

static unsigned __int64 CpuSpeed = 0;
static unsigned __int64 CycleCount[100] = { 0, };
static unsigned __int32 CallStackPos = 0;

inline unsigned __int64 GetCpuCycle() { _asm rdtsc }

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
        sub    edx, [ecx+4] 

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
