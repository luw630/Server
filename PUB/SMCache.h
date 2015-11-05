#pragma once

#include <unknwn.h>

// 基本功能接口
interface SharedMemoryCache
{
    typedef BOOL ( CALLBACK *TCB )( LPVOID key, LPVOID element, LPVOID argment );

    // 根据设定值初始化并获取指定缓存管理器，新开的缓存器会启动一个守护进程用于保持共享内存对象
    // uniqueName = 唯一名称，用于判断指定共享内存是否存在！
    // elementSize = 缓存单元, count = 预定的缓存单元数量！
    // keySize = 每个关键字的空间大小（用于表索引，默认情况下，为最大30字节的字符串（不包含\0））
    static SharedMemoryCache& Singleton( LPCSTR uniqueName, WORD capacity, DWORD elementSize, BYTE keySize = 30 );

    // 销毁这个共享空间，守护进程结束退出，Singleton本身还仍然有效
    // 如果不主动调用该函数，则就算当前进程正常退出关闭，但缓存会仍然持续有效！
    virtual void Release() = 0;

    // 返回缓存空间大小（以单元计），Size返回有效单元数量，Capacity返回总单元容量
    virtual DWORD Size() = 0;
    virtual DWORD Capacity() = 0;

    // 用以判断该管理器是新创建还是已经有效存在的
    virtual BOOL isNewly() = 0;

    // 将所有单元清除，并重置缓存
    virtual void Clear() = 0;

    // 删除一个已经存在的单元
    virtual BOOL Erase( LPCVOID key ) = 0;

    // 获取指定单元（一般情况下，key为字符串指针，特殊情况下，如keySize<=4，则key为32bit整数值！）
    virtual LPVOID operator [] ( LPCVOID key ) = 0;

    // 遍历所有有效对象
    virtual void Traverse( TCB fn, LPVOID argment ) = 0;
};

// 包装后的应用模版类
template < typename _type, DWORD count >
class ObjectCache
{
public:
    DWORD Size() { return cache.Size(); }
    DWORD Capacity() { return cache.Capacity(); }
    BOOL isNewly() { return cache.isNewly(); }
    void Clear() { cache.Clear(); }
    BOOL Erase( LPCSTR key ) { return cache.Erase( key ); }
    _type& operator [] ( LPCSTR key ) { return *reinterpret_cast< _type* >( cache[ key ] ); }
    template < typename _functor > void Traverse( _functor &functor ) 
    { 
        cache.Traverse( TCB_Functor< void >::GetFunctor( &functor ), ( LPVOID )&functor ); 
    }

public:
    ObjectCache( LPCSTR uniqueName ) : 
      cache( SharedMemoryCache::Singleton( uniqueName, count, sizeof( _type ) ) ) {}

    void TheEnd() { cache.Release(); }

private:
    template < typename _functor >
    struct TCB_Functor 
    {
        template < typename _functor > static SharedMemoryCache::TCB 
            GetFunctor( _functor functor ) { return TCB_Functor< _functor >::TCB_Call; }

        static BOOL CALLBACK TCB_Call( LPVOID key, LPVOID element, LPVOID argment )
        {
            return ( *reinterpret_cast< _functor >( argment ) )
                ( reinterpret_cast< LPCSTR >( key ), reinterpret_cast< _type& >( *( LPBYTE )element ) );
        };
    };

    SharedMemoryCache &cache;
};
