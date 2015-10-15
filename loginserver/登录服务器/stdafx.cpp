// stdafx.cpp : 只包括标准包含文件的源文件
// 登陆服务器.pch 将成为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO: 在 STDAFX.H 中
//引用任何所需的附加头，而不是在此文件中引用
#include <crtdbg.h>
#include <Shellapi.h>

#define MAXBUFFER 256

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

        // 装载异常捕获器
        // __STException::install(true);

        //_CrtSetBreakAlloc(4796);
        rfalse(1, 1, "start!!!!!");

        //dwDebugFileSize = 0;
        //HANDLE hFile = ::CreateFile("debug.txt", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, NULL, NULL);
        //if (hFile != INVALID_HANDLE_VALUE)
        //{
        //    SetFilePointer(hFile, 0, 0, FILE_END);
        //    dwDebugFileSize = ::GetFileSize(hFile, &dwDebugFileSize);
        //    CloseHandle(hFile);
        //}
    }
    ~CDebug()
    {
        //HANDLE hFile = ::CreateFile("debug.txt", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, NULL, NULL);
        //if (hFile != INVALID_HANDLE_VALUE)
        //{
        //    DWORD temp;
        //    temp = ::GetFileSize(hFile, &temp);
        //    CloseHandle(hFile);
        //    if (dwDebugFileSize != temp)
        //    {
        //        if (MessageBox(0, "debug.txt文件的长度改变了！需要打开察看吗", "@.@", MB_YESNO) == IDYES)
        //        {
        //            ShellExecute(::GetActiveWindow(), "open", "debug.txt", NULL, NULL, SW_SHOWNORMAL);
        //        }
        //    }
        //}

        rfalse(1, 1, "close!!!!!");

        // _CrtDumpMemoryLeaks();
    }

    char buffer[1024];
    //DWORD dwDebugFileSize;
} __dbend;

int &GetTraceType()
{
    static int nTraceType = 0;
    return nTraceType;
}

bool &GetSkipMargin()
{
    static bool bSkipMargin = true;
    return bSkipMargin;
}

int &GetUseEndl()
{
	static int nUseEndl = 0;
	return nUseEndl;
}

void trace(char lTraceType, char bUseEndl)
{
    DWORD size = (DWORD)dwt::strlen(__dbend.buffer, MAXBUFFER);

    //if (bUseEndl)
	{
        __dbend.buffer[size++] = '\r';
        __dbend.buffer[size++] = '\n';
	}
    __dbend.buffer[size] = '\0';

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
			MessageBox(0, __dbend.buffer, "error info", 0);
		#endif
		}
		break;
	case 1:
		{
			HANDLE hFile = ::CreateFile("debug.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, NULL, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				//MessageBox(0, "无法打开debug.txt文件！", "=.=", 0);
				return;
			}

            SetFilePointer(hFile, 0, 0, FILE_END);
			WriteFile(hFile, __dbend.buffer, size, &size, 0);
			CloseHandle(hFile);
		}
		break;
	case 2:
		{
			extern void AddInfo(LPCSTR Info);
			AddInfo(__dbend.buffer);
		}
		break;
	case 3:
		{
			HANDLE hFile = ::CreateFile("otherinfo.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, NULL, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
				return;

            SetFilePointer(hFile, 0, 0, FILE_END);
			WriteFile(hFile, __dbend.buffer, size, &size, 0);
			CloseHandle(hFile);
		}
		break;
	case 4:
		{
			HANDLE hFile = ::CreateFile("loginfail.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, NULL, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
				return;

            SetFilePointer(hFile, 0, 0, FILE_END);
			WriteFile(hFile, __dbend.buffer, size, &size, 0);
			CloseHandle(hFile);
		}
		break;
	}
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

void TraceMemory(LPVOID pDumpEntry, int iOffset, unsigned int uiSize, LPCSTR szFilename)
{
#ifdef USE_TRY
    try
#endif
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
#ifdef USE_TRY
    catch (...)
    {
    }
#endif
}

/*
// 登陆过程描述
{
    __property 底层自带连接无响应超时断开功能;
    __property 底层自带连接可设定超时断开功能;
    __property 使用带Context的ContextNetProvider;

    // 连接建立

    wait_event（新建连接的认证消息）
    {
        if （该连接断开）
        {
            不做任何操作;
            return;
        }
    }

    if （服务器处于登陆拒绝状态 || 认证信息中的数据有误）
    {
        通知原因，同时断开连接;
        return;
    }

    向账号服务器发送消息（账号登陆消息［账号、密码、用于回传的连接编号］）;
    if (发送失败)
    {
        通知原因，同时断开连接;
        return;
    }

    wait_event（等待账号服务器的认证结果）
    {
        if （响应时间超时）
        {
            通知原因，同时断开连接;
            return;
        }

        if （该连接断开）
        {
            不做任何操作;
            return;
        }
    }

    if （认证结果表明登陆失败 && 不是该账号已登陆）
    {
        if （如果是严重错误）
        {
            不做任何操作，直接断开连接;
            return;
        }
        else
        {
            // 这里可以无视回传的连接编号是否有效
            通知原因，同时断开连接;
            return;
        }
    }

    if （认证结果中回传的连接编号如果无效）
    {
        // 那么本次操作已经不能继续下去了
        if （认证结果为账号登陆成功）
        {
            通知账号服务器账号登出（账号、密码）;
            // 可以暂时无视返回值
        }

        return;
    }

    // 到这里就可以认为该连接认证成功！

    设置该连接为认证成功的状态

    if （如果是该账号已登陆）
    {
        // 考虑到不能确认的因数，现在暂时认为合法，绑定到已有的该账号关联的对象
        在本地服务器维护的对象映射表中重新绑定账号（账号、密码、回传的连接编号）;
        if (绑定失败)
        {
            通知原因，同时断开连接;
        }

        // 绑定结束后，当前连接的操作完成。。。
        return;
    }
    else
    {
        给该连接附加Context（账号、密码）
        if （附加失败）
        {
            通知原因，同时断开连接;

            通知账号服务器账号登出（账号、密码）;

            // 可以暂时无视返回值
            return;
        }

        //**********************************************************
        // 附加成功后，该连接的登出由连接断开时操作Context数据来实现
        // 此后如无特殊操作，断开规则相同
        //**********************************************************
    }

    // 进入游戏流程，获取角色列表数据

    向游戏数据库服务器发送消息（获取角色列表数据［账号、密码、用于回传的连接编号］）;
    if (发送失败)
    {
        通知原因，同时断开连接;
        return;
    }

    wait_event（等待游戏数据库服务器回传的角色列表数据）
    {
        if （响应时间超时）
        {
            通知原因，同时断开连接;
            return;
        }
    }

    if （返回的结果为操作失败）
    {
        通知原因，同时断开连接;
        return;
    }

    if （返回的中回传的连接编号如果无效）
    {
        // 那么本次操作已经不能继续下去了
        return;
    }

    通知客户端连接进入角色选择界面

    while （直到玩家进入游戏）
    {
        设置该连接的等待时间（最大操作等待时间）

        wait_event（等待客户端连接提交操作）
        {
            // 这里没有任何操作
            // 因为设置了该连接的等待时间
            // 所以时间超过后即自动断开连接
            // 同时上层的消息处理自动更具Context处理登出的操作
        }

        if （提交数据检测失败）
        {
            通知原因，同时断开连接;
            return;
        }

        switch （操作类型）
        {
        case 创建新角色：

            向游戏数据库服务器发送消息（创建角色消息）;
            if （发送失败）
            {
                通知原因;
                continue;
            }

            ...接下的操作基本类似

            break;

        case 删除已有角色：

            向游戏数据库服务器发送消息（删除角色消息）;
            if （发送失败）
            {
                通知原因;
                continue;
            }

            ...接下的操作基本类似

            break;

        case 选择角色进入游戏：

            向游戏数据库服务器发送消息（获取角色消息）;
            if （发送失败）
            {
                通知原因;
                continue;
            }

            // 取消等待时间
            设置该连接的等待时间（0）

            wait_event（等待游戏数据库服务器返回所选角色的数据）
            {
            }

            if (返回的连接编号已经失效)
            {
                // 连接已断开，整个操作结束
                return;
            }

            if (返回的结果为操作失败)
            {
                通知原因;
                continue;
            }

            将返回的数据设为角色对象;

            将该角色放入指定的场景;

            if (放入失败)
            {
                通知原因;
                continue;
            }

            // 为避免转移连接时的连接断开造成的Context自动登出
            清除该连接对应的Context;

            加入服务器维护的对象映射表;

            break;
        }
    }
}
*/

/*
extern "C" int __cdecl _callnewh(size_t size);

void * __cdecl operator new(size_t size) _THROW1(_STD bad_alloc)
{
    // try to allocate size bytes
    void *p;
    while ((p = malloc(size)) == 0)
	    if (_callnewh(size) == 0)
            break;

    if (p == 0)
    {
        _STD _Nomemory();
    }

    return (p);
}
*/

/*
extern "C" 
{
    void * __cdecl _heap_alloc (size_t size);
    int __cdecl _callnewh(size_t size);

    void * __cdecl _nh_malloc(size_t size, int nhFlag)
    {
        void * pvReturn;

        //  validate size
        if (size > _HEAP_MAXREQ)
            return NULL;

        for (;;) {

            //  allocate memory block
            if (size <= _HEAP_MAXREQ)
                pvReturn = _heap_alloc(size);
            else
                pvReturn = NULL;

            //  if successful allocation, return pointer to memory
            //  if new handling turned off altogether, return NULL

            if (pvReturn || nhFlag == 0)
                return pvReturn;

            //  call installed new handler
            if (!_callnewh(size))
                return NULL;

            //  new handler was successful -- try to allocate again
        }
    }
}
*/
