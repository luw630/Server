#include "minidump.h"
//#include <iostream>
using namespace std;


std::ostream& operator<<(std::ostream& os, const EXCEPTION_RECORD& red)
{
	return os << "   Thread ID:" << GetCurrentThreadId()
		<< "   ExceptionCode: " << red.ExceptionCode << "/n"
		<< "   ExceptionFlags: " << red.ExceptionFlags << "/n"
		<< "   ExceptionAddress: " << red.ExceptionAddress << "/n"
		<< "   NumberParameters: " << red.NumberParameters;
}


LONG WINAPI GPTUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	FILE *fp;
	fp = fopen("DumpInfo.txt", "a+");
	char buffer[256];
	sprintf(buffer, "%s", "   ZoneServer Dead,CreatDump!!\n");

	static char __time[1024];
	SYSTEMTIME s;
	GetLocalTime(&s);
	sprintf(__time, "[%d-%d-%d %d:%d:%d]", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);

	fwrite(__time, strlen(__time), 1, fp);
	fwrite(buffer, strlen(buffer), 1, fp);
	fclose(fp);

	//StackWalker sw;
	//sw.ShowCallstack();//actionlog.txt

	CreateMiniDump(pExceptionInfo, "Exception.dmp");
	//std::cerr << "未知错误：" << (*pExceptionInfo->ExceptionRecord) << std::endl;
	//exit(pExceptionInfo->ExceptionRecord->ExceptionCode);

	return EXCEPTION_EXECUTE_HANDLER;    // 程序停止运行
}

// 此函数一旦成功调用，之后对 SetUnhandledExceptionFilter 的调用将无效  
void DisableSetUnhandledExceptionFilter()  
{  
    void* addr = (void*)GetProcAddress(LoadLibrary("kernel32.dll"), "SetUnhandledExceptionFilter");  
  
    if (addr)  
    {  
        unsigned char code[16];  
        int size = 0;    
        code[size++] = 0x33;  
        code[size++] = 0xC0;  
        code[size++] = 0xC2;  
        code[size++] = 0x04;  
        code[size++] = 0x00;  
  
        DWORD dwOldFlag, dwTempFlag;  
        VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);  
        WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);  
        VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);  
    }  
}  
  
void InitMinDump()  
{  
    //注册异常处理函数  
	SetUnhandledExceptionFilter(GPTUnhandledExceptionFilter);
  
    //使SetUnhandledExceptionFilter  
    DisableSetUnhandledExceptionFilter();  
}