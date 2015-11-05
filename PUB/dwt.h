#pragma once

// 这里使用到了KEY
// 但是在管理的时候，直接使用HASH表还是MAP二叉树呢
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <map>
#include <list>
#include <queue>
#include <fstream>
#include <strstream>

#define RES_WAIT
#define RES_FAIL
#define RES_SUCCESS

#define MAXSTRINGLEN(s) ( sizeof(s)/sizeof(s[0]) - 1 )

#pragma pack(push, 8)

namespace dwt {

//****************************************************************************
// 系统标准API::IsBadStringPtr只是判断了指定字符串在指定长度内的读有效性…… 
// 这和我们的初衷不一致——至少需要保证该字符串是完整的[null terminate]！！！
// 所以自己来写了一个……
//**************************************************************************** 
inline BOOL IsBadStringPtr( LPCSTR string, DWORD size ) throw()
{
    try
    {
        while ( *string != 0 )
        {
            size--;
            string++;

            // 如果实际长度[+'\0']超过默认长度，则检测失败
            if ( size == 0  )
                return true;
        }

        return false;
    }
    catch( ... )
    {
        // 如果检测过程中出现异常，则失败
    }

    return true;
}

template < typename _Ty >
struct AutoDelete
{
	AutoDelete( _Ty ptr ) : ptr( ptr ) {}
	~AutoDelete() { delete ptr; }
	_Ty ptr;
};


// 字符串拷贝函数
// 以count和source的最小长度作为dest的拷贝长度，并且会自动在dest末尾添加\0
// 例：
// LPCSTR sInfo[10];
// strcpy(sInfo, "123456789123456789", 10);
// sInfo => "123456789";
inline char * __stdcall strcpy(char * dest, const char * source, unsigned __int32 count) 
 throw() 
{
    if (dest == NULL) return 0;
    if (source == NULL) { dest[0] = 0; return dest; }

    // attention:
    // dest space size must equal to source !!!
    // and above size must equal to count !!!
    _asm
    {
        mov     edi, dest           // register all parameter
        mov     esi, source         // ..
        mov     ecx, count          // ..
        dec     ecx                 // f ecx equal to count, cause the last char(out of valid memory space) modified 

    _loop:
        cmp     byte ptr [esi], 0   // if (*source == '\0') 
        jz      _break              //     break;
        movsb                       // *dest++ = *source++
        loop    _loop               // if (--count == 0) break;

    _break:
        mov     byte ptr [edi], 0   // *dest = '\0';    set last char endl
        mov     eax, dest           // return dest;     this is raw dest
    }
}

// 字符串长度检测函数
// 返回source字符串的长度，长度的大小决不会超过count
// 例：
// strlen(NULL, 10) == 0
// strlen("1234567", 10) == 7
// strlen("123456789123", 10) == 9
inline unsigned __int32 __stdcall strlen(const char * source, unsigned __int32 count) throw()
{
    if (source == NULL) return 0;

    _asm
    {
        mov     esi, source         // register all parameter
        mov     ecx, count          // ..
        xor     eax, eax            // eax = 0;
        dec     ecx                 // if ecx equal to count, cause the last char(out of valid memory space) modified 

    _loop:
        cmp     byte ptr [esi], 0   // if (*source == '\0') 
        jz      _break              //     break;
        inc     esi                 // source++;
        inc     eax                 // eax++;
        loop    _loop               // if (--count == 0) break;

    _break:
    }
}

inline char * _strncat(char* str1, const char* suffix, unsigned int n)
{
	return strncat( str1, suffix, __min( n, MAXSTRINGLEN(str1) - dwt::strlen(str1,MAXSTRINGLEN(str1))) );
}

// 字符串比较函数
inline int __stdcall strcmp(const char * source, const char * dest, unsigned __int32 count) throw()
{
    if (dest == NULL) return -1;
    if (source == NULL) return 1;

    _asm
    {
        mov     edi, dest           // register all parameter
        mov     esi, source         // ..
        mov     ecx, count          // ..

    _loop:
        cmp     byte ptr [esi], 0   // if (*source == '\0') 
        je      _test_end           //     goto _test_end;

        cmp     byte ptr [edi], 0   // if (*source != '\0') if (*dest == '\0') 
        je      _ret_above          //     return 1;

        cmpsb                       // COMPARE (*dest++  *source++);
        jg      _ret_above          // if (*dest > *source) return 1;
        jl      _ret_below          // if (*dest > *source) return -1;
        loop    _loop               // if (--count == 0) break loop and return 0; directly run to _ret_equal

    _ret_equal:
        xor     eax, eax            // return 0;
        jmp     _end

    _ret_above:
        mov     eax, 1              // return 1;
        jmp     _end

    _ret_below:
        mov     eax, -1             // return -1;
        jmp     _end

    _test_end:
        cmp     byte ptr [edi], 0   //  if (*source == '\0') if (*dest == '\0') 
        je      _ret_equal          //      return 0;
        jmp     _ret_below          //      else return -1;    

    _end:
    }
}

//以下这个是专门用来处理以数据流来作为关键字的对象
// #define stringkey StringKey_CRC32
template <class _Key>
    class stringkey
    {
    public:
        stringkey()
        {
            _key[0] = 0;
        }

        stringkey(LPCSTR Key)
        {
            strcpy(_key, Key, sizeof(_Key));
        }

        bool operator < (const stringkey<_Key> &key) const
        {
            return strcmp(_key, key._key, sizeof(_Key)) < 0;
        }

        stringkey<_Key> operator + (LPCSTR Key)
        {
            stringkey<_Key> ret;

            if (Key == NULL)
                ret = _key;
            else
            {
                _snprintf(ret, sizeof(_Key) - 1, "%s%s", _key, Key);
                ret[sizeof(_Key) - 1] = 0;
            }

            return ret;
        }

        bool operator == (LPCSTR Key) const
        {
            if ((Key == NULL) && (_key[0] == 0) )
                return true;

            return strcmp(_key, Key, sizeof(_Key)) == 0;
        }

        bool operator == (const stringkey<_Key> &key) const
        {
            return strcmp(_key, key._key, sizeof(_Key)) == 0;
        }

        bool operator != (LPCSTR Key) const
        {
            if ((Key == NULL) && (_key[0] == 0) )
                return false;

            return strcmp(_key, Key, sizeof(_Key)) != 0;
        }

        bool operator != (const stringkey<_Key> &key) const
        {
            return strcmp(_key, key._key, sizeof(_Key)) != 0;
        }

        operator LPCSTR () const
        {
            return _key;
        }

        operator LPSTR ()
        {
            return _key;
        }

        __int8 _key[sizeof(_Key)];
    };

class cStringMatchDict
{
public:
__declspec(noinline) 
    void SetKeyWord(LPCSTR lpKeyword, const DWORD dwSize)
    {
        sLetter *lpDictIndex = &m_DictRoot;
        LPCSTR lpKeyend = lpKeyword + dwSize;

        while (lpKeyword < lpKeyend)
        {
            BYTE index = ((BYTE)*lpKeyword) >> 4;
            if (lpDictIndex->lpFollow[index] == NULL)
                lpDictIndex->lpFollow[index] = new sLetter;
            lpDictIndex = lpDictIndex->lpFollow[index];

            index = ((BYTE)*lpKeyword) & 15;
            if (lpDictIndex->lpFollow[index] == NULL)
                lpDictIndex->lpFollow[index] = new sLetter;
            lpDictIndex = lpDictIndex->lpFollow[index];

            lpKeyword++;
        }

        lpDictIndex->bLeafNode = true;
    }

__declspec(noinline) 
    BOOL Contain(LPCSTR lpKeyword, const DWORD dwSize)
    {
        sLetter *lpDictIndex = &m_DictRoot;
        LPCSTR lpKeyend = lpKeyword + dwSize;

        while (lpKeyword < lpKeyend)
        {
            lpDictIndex = lpDictIndex->lpFollow[((BYTE)*lpKeyword) >> 4];
            if (lpDictIndex == NULL)
                break;

            lpDictIndex = lpDictIndex->lpFollow[((BYTE)*lpKeyword) & 15];
            if (lpDictIndex == NULL)
                break;

            if (lpDictIndex->bLeafNode)
                return TRUE;

            lpKeyword++;
        }

        return FALSE;
    }

private:
    struct sLetter
    {
        sLetter()
        {
            bLeafNode = false;
            memset(lpFollow, 0, sizeof(lpFollow));
        }

        ~sLetter()
        {
            Clear();
        }

        void Clear()
        {
            for (int i=0; i<16; i++)
            {
                if (lpFollow[i] != NULL)
                {
                    delete lpFollow[i];
                    lpFollow[i] = NULL;
                }
            }
        }

        bool bLeafNode;
        sLetter *lpFollow[16];
    };

    sLetter m_DictRoot;
};

#define REFRESHMARGIN   10000  // 每10秒钟刷新一次

enum ERRES
{
    ERR_RELOAD_FAIL,
    ERR_RELOAD_SUCCESS,
    ERR_RELOADING,
    ERR_CANNOT_RELOAD,
};

template <class _Data, class _Key>
    class resource
    {
    public:
        void reload(bool immediately = false) 
        { 
            if (immediately || (_dflag == ERR_RELOAD_FAIL) || ( (int)(timeGetTime() - _dtime) > 0 ))
            {
                _dflag = _data.Reload(); 
                _dtime = timeGetTime() + REFRESHMARGIN; 
            }
        }

    public:
        resource() : _dtime(0), _dflag(ERR_RELOAD_FAIL) {}
        resource(const _Data &Data, const _Key &Key) : _data(Data), _key(Key), _dtime(0), _dflag(ERR_RELOAD_FAIL) {}

        _Data _data;        // 数据保存部分
        _Key  _key;         // 数据索引关键字
        DWORD _dtime;       // 等待刷新的时间(还有多久数据过期)
        ERRES _dflag;       // 目前（上一次）的刷新状态 （NULL，写请求，读请求，删除请求）
    };


// 资源管理器，现在直接使用map（二叉树映射表）
// 以后转成比较快的HASH表 -____-bb
template <class _Data, class _Key>
    class resmanager
    {
    public: 
        void RefreshSingle(_Key &Key)
        {
            std::map< _Key, resource<_Data, _Key> >::iterator it = _depository.find(Key);
            if (it != _depository.end())
                it->second._dflag = it->second._data.Reload();
        }

        void RefreshAll(BOOL bExpriation)
        {
            ::SetEvent(bExpriation ? _hRefreshEvent[0] : _hRefreshEvent[1]);
        }

		BOOL AddRes(const _Data &Data, const _Key &Key) 
		{
			_depository[Key] = resource<_Data, _Key>(Data, Key);
			return TRUE;
		}
        
        BOOL DelRes(const _Key &Key) 
        {
            std::map< _Key, resource<_Data, _Key> >::iterator it = _depository.find(Key);
            if (it == _depository.end()) 
                return FALSE;
            _depository.erase(it);
            return TRUE;
        }

        resource<_Data, _Key> *GetRes(const _Key &Key) 
        {
            std::map< _Key, resource<_Data, _Key> >::iterator it = _depository.find(Key);
            if (it != _depository.end())
                return &it->second;
            return NULL;
        }

        _Data * GetData(const _Key &Key) 
        {
            resource<_Data, _Key> *pRes = GetRes(Key);
            if (pRes == NULL) return NULL;
            return &pRes->_data;
        }

    public:
        resmanager() : _dwRefreshMargin(REFRESHMARGIN)
        {
            _hRefreshEvent[0] = ::CreateEvent(0, 0, 0, "refresh all expiration resource immediately");
            _hRefreshEvent[1] = ::CreateEvent(0, 0, 0, "refresh all resource immediately");
            _hRefreshEvent[2] = ::CreateEvent(0, 0, 0, "refresh single resource immediately");

            // 启动刷新线程
            _hRefresh = ::CreateThread(0, 0, REFRESH_THREAD<_Data, _Key>, this, 0, 0);
        }

        ~resmanager()
        {
            // 关闭刷新线程
            ::TerminateThread(_hRefresh, -1);

            ::CloseHandle(_hRefreshEvent[2]);
            ::CloseHandle(_hRefreshEvent[1]);
            ::CloseHandle(_hRefreshEvent[0]);
        }
        
    public:
        std::map< _Key, resource<_Data, _Key> > _depository;

    public:
        HANDLE _hRefresh;
        HANDLE _hRefreshEvent[3];
        DWORD  _dwRefreshMargin;

//        friend DWORD WINAPI REFRESH_THREAD<_Data, _Key> (LPVOID lpThreadParameter);
    };

template <class _Data, class _Key>
    DWORD WINAPI REFRESH_THREAD (LPVOID lpThreadParameter)
    {
        // 对于刷新线程来说，所处理的数据和实时刷新没有必然联系，所以暂不考虑加线程锁
        resmanager<_Data, _Key> *pResm = (resmanager<_Data, _Key> *)lpThreadParameter;
        while (true)
        {
            switch (::WaitForMultipleObjects(3, pResm->_hRefreshEvent, FALSE, pResm->_dwRefreshMargin))
            {
            case WAIT_TIMEOUT:
                // 超时后，定时刷新所有过期数据
            case WAIT_OBJECT_0:
                // 根据请求，立即刷新所有过期数据
                {
					std::map< _Key, resource<_Data, _Key> >::iterator it = pResm->_depository.begin();
					while (it != pResm->_depository.end())
					{
                        it->second.reload();
						it++;
					}
				}
                break;

            case WAIT_OBJECT_0+1:
                // 根据请求，立即刷新所有数据
                {
					std::map< _Key, resource<_Data, _Key> >::iterator it = pResm->_depository.begin();
					while (it != pResm->_depository.end())
					{
						it->second.reload(true);
						it++;
					}
                }
                break;

            case WAIT_OBJECT_0+2:
                // 根据请求，立即刷新一个目标数据
                break;
            }
        }
        return 0;
    }

#define InitTimeOutChecker(Checker, TimeLimit, pCallback, pThis) (Checker).__init(TimeLimit, pCallback, pThis)

template <class _Object>
    class tTimeOutChecker
    {
        class tObject
        {
        public:
            unsigned __int32 i32Time;
            unsigned __int32 i32Serial;
            _Object Object;
        };

    public:
        tTimeOutChecker()
        {
            m_pOnTimeOut = NULL;
            m_pThis = NULL;
            i32TimeLimit = 0;
            i32CurTime = 0;
            i32Serial = 0;
        }

        void __init(unsigned __int32 TimeLimit, ...)
        {
            va_list cp;

            va_start (cp, TimeLimit);
            m_pOnTimeOut = va_arg (cp, void*);
            m_pThis = va_arg (cp, void*);
            va_end (cp);

            i32TimeLimit = TimeLimit;
            i32CurTime = timeGetTime();
            i32Serial = 0;
        }

        __int32 Check()
        {
            i32CurTime = timeGetTime();

            __int32 ret = 0;
            // initialize __thiscall function parameter
            void *pOnTimeOut = m_pOnTimeOut, *pThis = m_pThis; 

            while (CheckQueue.size() != 0)
            {
                if ( (int)(i32CurTime - CheckQueue.front().i32Time) > 0 )
                {
                    _Object *pObj = &CheckQueue.front().Object;
#ifdef USE_TRY
                    try
#endif
                    {
                        ret++;
                        __asm 
                        {
                            push    pObj
                            mov     edx, pOnTimeOut
                            mov     ecx, pThis
                            call    edx
                        }
                    }
#ifdef USE_TRY
                    catch(...)
                    {   
                        BOOL rfalse(char, char, LPCSTR, ...);
                        rfalse(1, 1, "catched N.E. "__FILE__"(%d)", __LINE__);
                        //ERROR HANDLING
                        //throw;
                    }
#endif
                }
                else
                {
                    break;
                }

                CheckQueue.pop_front();
            }

            return ret;
        }

        __int64 Push(_Object obj)
        {
            tObject tobj;
            tobj.i32Serial = i32Serial++;
            tobj.i32Time = i32CurTime + i32TimeLimit;
            tobj.Object = obj;
            CheckQueue.push_back(tobj);
            return ((__int64)tobj.i32Time << 32) | tobj.i32Serial;
        }

        bool Remove(__int64 TimeSerial)
        {
            unsigned __int32 Time   = (unsigned  __int32)(TimeSerial>>32);
            unsigned __int32 Serial = (unsigned  __int32)TimeSerial;

            std::list<tObject>::iterator it = CheckQueue.begin();

            while (it != CheckQueue.end())
            {
                tObject &tobj = *it;

                if (tobj.i32Time == Time)
                {
                    if (tobj.i32Serial == Serial)
                    {
                        CheckQueue.erase(it);
                        return true;
                    }
                }

                if (tobj.i32Time > Time)
                {
                    // 因为是顺序排放的，所以此处以后就没有符合条件的对象了
                    break;
                }

                it++;
            }

            return false;
        }

    private:
        unsigned __int32 i32CurTime;            // 当前时间
        unsigned __int32 i32Serial;             // 当前序列号
        unsigned __int32 i32TimeLimit;          // 过期时限

        std::list<tObject> CheckQueue;          // 检测队列

        void *m_pOnTimeOut, *m_pThis;           // __thiscall function 超时回调
    };

template <typename _Ty>
    class mtQueue : 
        public std::queue<_Ty>
    {
    public:
        void push(const _Ty &_Val)
        {
            Lock();
            std::queue<_Ty>::push(_Val);
            Unlock();
        }

        void pop()
        {
            Lock();
            std::queue<_Ty>::pop();
            Unlock();
        }

        _Ty &front()
        {
            __try
            {
                Lock();
                return std::queue<_Ty>::front();
            }
            __finally
            {
                Unlock();
            }
        }

        _Ty &back()
        {
            __try
            {
                Lock();
                return std::queue<_Ty>::back();
            }
            __finally
            {
                Unlock();
            }
        }

        void Lock()
        {
            EnterCriticalSection(&m_QueueLock);
        }

        void Unlock()
        {
            LeaveCriticalSection(&m_QueueLock);
        }

        void PushExitEvent(HANDLE event)
        {
            __try
            {
                Lock();
                m_hExitEvent.push(event);
            }
            __finally
            {
                Unlock();
            }
        }

        BOOL isEnd()
        {
            return m_bEnd;
        }

        size_t Size() const
	    {
		    size_t n; 
		    lock();
		    n =  std::queue<_Ty>::size();
		    Unlock();
		    return n;
	    }

    public:
        mtQueue() :
        std::queue<_Ty>()
	    {
            InitializeCriticalSection(&m_QueueLock);
            m_bEnd = false;
	    }

        ~mtQueue()
	    {
            m_bEnd = true;

            while (!m_hExitEvent.empty())
            {
                WaitForSingleObject(m_hExitEvent.front(), INFINITE);
                m_hExitEvent.pop();
            }

            DeleteCriticalSection(&m_QueueLock);
	    }

    private:
        CRITICAL_SECTION m_QueueLock;
        volatile BOOL m_bEnd;
        std::queue<HANDLE> m_hExitEvent;
    };

template < typename _Ty >
    class mtUniqueQueue : 
        public std::queue<_Ty>
    {
    public:
        void push(const _Ty &_Val)
        {
            Lock();
            if ( m_UniqueID[ _Val.GetKey() ] == NULL )
            {
                std::queue<_Ty>::push(_Val);
                m_UniqueID[ _Val.GetKey() ] = &std::queue<_Ty>::back();
            }
            else
            {
                *m_UniqueID[ _Val.GetKey() ] = _Val;
            }

            Unlock();
        }

        void pop()
        {
            Lock();
            if ( !std::queue<_Ty>::empty() )
            {
                m_UniqueID[ std::queue<_Ty>::front().GetKey() ] = NULL;
                std::queue<_Ty>::pop();
            }
            Unlock();
        }

        _Ty &front()
        {
            __try
            {
                Lock();
                return std::queue<_Ty>::front();
            }
            __finally
            {
                Unlock();
            }
        }

        _Ty &back()
        {
            __try
            {
                Lock();
                return std::queue<_Ty>::back();
            }
            __finally
            {
                Unlock();
            }
        }

        void Lock()
        {
            EnterCriticalSection(&m_QueueLock);
        }

        void Unlock()
        {
            LeaveCriticalSection(&m_QueueLock);
        }

        void PushExitEvent(HANDLE event)
        {
            __try
            {
                Lock();
                m_hExitEvent.push(event);
            }
            __finally
            {
                Unlock();
            }
        }

        BOOL isEnd()
        {
            return m_bEnd;
        }

        size_t Size() const
	    {
		    size_t n; 
		    lock();
		    n =  std::queue<_Ty>::size();
		    Unlock();
		    return n;
	    }

    public:
        mtUniqueQueue() :
        std::queue<_Ty>()
	    {
            InitializeCriticalSection(&m_QueueLock);
            m_bEnd = false;
	    }

        ~mtUniqueQueue()
	    {
            m_bEnd = true;

            while (!m_hExitEvent.empty())
            {
                WaitForSingleObject(m_hExitEvent.front(), INFINITE);
                m_hExitEvent.pop();
            }

            DeleteCriticalSection(&m_QueueLock);
	    }

    private:
        CRITICAL_SECTION m_QueueLock;
        volatile BOOL m_bEnd;
        std::queue< HANDLE > m_hExitEvent;
        std::map< typename _Ty::_Tkey, _Ty* > m_UniqueID;
    };
};

#ifdef USING_PAKSTREAM
#include "Win32.h"
#include "PakFile.h"

namespace dwt
{
    class ifstream // :
        // virtual public std::istream
    {
    public:
        ifstream()
        {
            isFileStream = true;
        }

        ifstream(const char *szFilename)
        {
            isFileStream = true;
            open(szFilename);
        }

    public:
        void open(const char *szFilename)
        {
            m_FileStream.open(szFilename);

            if (m_FileStream.is_open())
                return;

            PakFile pf;
            if (!pf.Open((char *)szFilename))
                return;

            DWORD size = pf.Size();

            char *buf = new char [size];
            if (pf.Read(buf, size) != size)
            {
                delete buf;
                return;
            }

            m_BufStream.write(buf, size);
            delete buf;
            isFileStream = false;
        }

        void close()
        {
            if (isFileStream)
            {
                m_FileStream.close();
                return m_FileStream.clear();
            }

            return m_BufStream.clear();
        }

        bool is_open()
        {
            if (isFileStream)
            {
                return m_FileStream.is_open();
            }

            return m_BufStream.pcount() != 0;
        }

        bool eof()
        {
            if (isFileStream)
            {
                return m_FileStream.eof();
            }

            return m_BufStream.eof();
        }

        ifstream &getline(char *szBuffer, int cbSize)
        {
            if (isFileStream)
            {
                m_FileStream.getline(szBuffer, cbSize);
            }
            else
            {
                m_BufStream.getline(szBuffer, cbSize);
				cbSize = ( int )::strlen( szBuffer );
				// eat endline
				while ( cbSize && ( szBuffer[cbSize-1] == '\r' || szBuffer[cbSize-1] == '\n' ) )
					szBuffer[--cbSize] = 0;
            }

            return *this;
        }

#define EasyOp(_T) ifstream &operator >> (_T _Val) { \
            if (isFileStream) m_FileStream >> _Val; \
            else m_BufStream >> _Val; \
            return *this; \
        }

        EasyOp(char *)
        EasyOp(char &)
        EasyOp(short &)
        EasyOp(long &)
        EasyOp(int &)
        EasyOp(unsigned char &)
        EasyOp(unsigned short &)
        EasyOp(unsigned long &)
        EasyOp(unsigned int &)
		EasyOp(unsigned __int64 &)

    private:
        std::ifstream m_FileStream;
        std::strstream m_BufStream;
        bool isFileStream;
    };
};

#endif

#pragma pack(pop)
