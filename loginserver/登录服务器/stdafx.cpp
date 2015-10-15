// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// ��½������.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

#include "stdafx.h"

// TODO: �� STDAFX.H ��
//�����κ�����ĸ���ͷ���������ڴ��ļ�������
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

        // װ���쳣������
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
        //        if (MessageBox(0, "debug.txt�ļ��ĳ��ȸı��ˣ���Ҫ�򿪲쿴��", "@.@", MB_YESNO) == IDYES)
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
				//MessageBox(0, "�޷���debug.txt�ļ���", "=.=", 0);
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
        // ��ȡ��ǰ��������
        DWORD   PriorityClass = GetPriorityClass(GetCurrentProcess());
        int     Priority = GetThreadPriority(GetCurrentThread());

        // ���õ�ǰ����Ϊʵʱ����
        SetPriorityClass(GetCurrentProcess, REALTIME_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread, THREAD_PRIORITY_TIME_CRITICAL);

        // �ȴ�ϵͳ����
        Sleep(10);

        // ȡCPU���ڲ�
        CpuSpeed = GetCpuCycle();
        Sleep(1000);
        CpuSpeed = GetCpuCycle()-CpuSpeed;

        // �ָ���ǰ���̵�����
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
// ��½��������
{
    __property �ײ��Դ���������Ӧ��ʱ�Ͽ�����;
    __property �ײ��Դ����ӿ��趨��ʱ�Ͽ�����;
    __property ʹ�ô�Context��ContextNetProvider;

    // ���ӽ���

    wait_event���½����ӵ���֤��Ϣ��
    {
        if �������ӶϿ���
        {
            �����κβ���;
            return;
        }
    }

    if �����������ڵ�½�ܾ�״̬ || ��֤��Ϣ�е���������
    {
        ֪ͨԭ��ͬʱ�Ͽ�����;
        return;
    }

    ���˺ŷ�����������Ϣ���˺ŵ�½��Ϣ���˺š����롢���ڻش������ӱ�ţݣ�;
    if (����ʧ��)
    {
        ֪ͨԭ��ͬʱ�Ͽ�����;
        return;
    }

    wait_event���ȴ��˺ŷ���������֤�����
    {
        if ����Ӧʱ�䳬ʱ��
        {
            ֪ͨԭ��ͬʱ�Ͽ�����;
            return;
        }

        if �������ӶϿ���
        {
            �����κβ���;
            return;
        }
    }

    if ����֤���������½ʧ�� && ���Ǹ��˺��ѵ�½��
    {
        if ����������ش���
        {
            �����κβ�����ֱ�ӶϿ�����;
            return;
        }
        else
        {
            // ����������ӻش������ӱ���Ƿ���Ч
            ֪ͨԭ��ͬʱ�Ͽ�����;
            return;
        }
    }

    if ����֤����лش������ӱ�������Ч��
    {
        // ��ô���β����Ѿ����ܼ�����ȥ��
        if ����֤���Ϊ�˺ŵ�½�ɹ���
        {
            ֪ͨ�˺ŷ������˺ŵǳ����˺š����룩;
            // ������ʱ���ӷ���ֵ
        }

        return;
    }

    // ������Ϳ�����Ϊ��������֤�ɹ���

    ���ø�����Ϊ��֤�ɹ���״̬

    if ������Ǹ��˺��ѵ�½��
    {
        // ���ǵ�����ȷ�ϵ�������������ʱ��Ϊ�Ϸ����󶨵����еĸ��˺Ź����Ķ���
        �ڱ��ط�����ά���Ķ���ӳ��������°��˺ţ��˺š����롢�ش������ӱ�ţ�;
        if (��ʧ��)
        {
            ֪ͨԭ��ͬʱ�Ͽ�����;
        }

        // �󶨽����󣬵�ǰ���ӵĲ�����ɡ�����
        return;
    }
    else
    {
        �������Ӹ���Context���˺š����룩
        if ������ʧ�ܣ�
        {
            ֪ͨԭ��ͬʱ�Ͽ�����;

            ֪ͨ�˺ŷ������˺ŵǳ����˺š����룩;

            // ������ʱ���ӷ���ֵ
            return;
        }

        //**********************************************************
        // ���ӳɹ��󣬸����ӵĵǳ������ӶϿ�ʱ����Context������ʵ��
        // �˺���������������Ͽ�������ͬ
        //**********************************************************
    }

    // ������Ϸ���̣���ȡ��ɫ�б�����

    ����Ϸ���ݿ������������Ϣ����ȡ��ɫ�б����ݣ��˺š����롢���ڻش������ӱ�ţݣ�;
    if (����ʧ��)
    {
        ֪ͨԭ��ͬʱ�Ͽ�����;
        return;
    }

    wait_event���ȴ���Ϸ���ݿ�������ش��Ľ�ɫ�б����ݣ�
    {
        if ����Ӧʱ�䳬ʱ��
        {
            ֪ͨԭ��ͬʱ�Ͽ�����;
            return;
        }
    }

    if �����صĽ��Ϊ����ʧ�ܣ�
    {
        ֪ͨԭ��ͬʱ�Ͽ�����;
        return;
    }

    if �����ص��лش������ӱ�������Ч��
    {
        // ��ô���β����Ѿ����ܼ�����ȥ��
        return;
    }

    ֪ͨ�ͻ������ӽ����ɫѡ�����

    while ��ֱ����ҽ�����Ϸ��
    {
        ���ø����ӵĵȴ�ʱ�䣨�������ȴ�ʱ�䣩

        wait_event���ȴ��ͻ��������ύ������
        {
            // ����û���κβ���
            // ��Ϊ�����˸����ӵĵȴ�ʱ��
            // ����ʱ�䳬�����Զ��Ͽ�����
            // ͬʱ�ϲ����Ϣ�����Զ�����Context����ǳ��Ĳ���
        }

        if ���ύ���ݼ��ʧ�ܣ�
        {
            ֪ͨԭ��ͬʱ�Ͽ�����;
            return;
        }

        switch ���������ͣ�
        {
        case �����½�ɫ��

            ����Ϸ���ݿ������������Ϣ��������ɫ��Ϣ��;
            if ������ʧ�ܣ�
            {
                ֪ͨԭ��;
                continue;
            }

            ...���µĲ�����������

            break;

        case ɾ�����н�ɫ��

            ����Ϸ���ݿ������������Ϣ��ɾ����ɫ��Ϣ��;
            if ������ʧ�ܣ�
            {
                ֪ͨԭ��;
                continue;
            }

            ...���µĲ�����������

            break;

        case ѡ���ɫ������Ϸ��

            ����Ϸ���ݿ������������Ϣ����ȡ��ɫ��Ϣ��;
            if ������ʧ�ܣ�
            {
                ֪ͨԭ��;
                continue;
            }

            // ȡ���ȴ�ʱ��
            ���ø����ӵĵȴ�ʱ�䣨0��

            wait_event���ȴ���Ϸ���ݿ������������ѡ��ɫ�����ݣ�
            {
            }

            if (���ص����ӱ���Ѿ�ʧЧ)
            {
                // �����ѶϿ���������������
                return;
            }

            if (���صĽ��Ϊ����ʧ��)
            {
                ֪ͨԭ��;
                continue;
            }

            �����ص�������Ϊ��ɫ����;

            ���ý�ɫ����ָ���ĳ���;

            if (����ʧ��)
            {
                ֪ͨԭ��;
                continue;
            }

            // Ϊ����ת������ʱ�����ӶϿ���ɵ�Context�Զ��ǳ�
            ��������Ӷ�Ӧ��Context;

            ���������ά���Ķ���ӳ���;

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
