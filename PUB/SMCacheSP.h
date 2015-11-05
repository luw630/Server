#pragma once

#include <unknwn.h>

// 基本功能接口，这个是EX版，主要是增加了多线程/进程的支持
interface SharedMemoryCacheEx
{
    typedef BOOL ( CALLBACK *TCB )( LPVOID key, LPVOID element, LPVOID argment );

    // 根据设定值初始化并获取指定缓存管理器，新开的缓存器会启动一个守护进程用于保持共享内存对象
    // uniqueName = 唯一名称，用于判断指定共享内存是否存在！
    // elementSize = 缓存单元, count = 预定的缓存单元数量！
    // keySize = 每个关键字的空间大小（用于表索引，默认情况下，为最大30字节的字符串（不包含\0））
    static SharedMemoryCacheEx& Singleton( LPCSTR uniqueName, WORD capacity, DWORD elementSize, BYTE keySize = 30 );

    // 同是初始化操作,不过会产生新的管理器对象,必须通过调用Release才会正常销毁
    static SharedMemoryCacheEx& Generate( LPCSTR uniqueName, WORD capacity, DWORD elementSize, BYTE keySize = 30 );

    // 销毁这个共享空间，守护进程结束退出，Singleton本身还仍然有效
    // 如果不主动调用该函数，则就算当前进程正常退出关闭，但缓存会仍然持续有效！
    virtual void Release() = 0;

    // 返回缓存空间大小（以单元计），Size返回有效单元数量，Capacity返回总单元容量
    virtual DWORD Size() = 0;
    virtual DWORD Capacity() = 0;

    // 用以判断该管理器是新创建还是已经有效存在的
    virtual BOOL isNewly() = 0;

    // 将所有单元清除，并重置缓存
    // 锁状态，加锁[管理器/对象]，解锁[管理器/对象]
    virtual void Clear() = 0;

    // 删除一个已经存在的单元
    // 锁状态，加锁[管理器/对象]，解锁[管理器/对象]
    virtual BOOL Erase( LPCVOID key ) = 0;
    virtual BOOL Erase( DWORD block ) = 0;

    // 获取指定单元（一般情况下，key为字符串指针，特殊情况下，如keySize<=4，则key为32bit整数值！）
    // 锁状态，加锁[管理器/对象]，解锁[管理器]
    virtual LPVOID operator [] ( LPCVOID key ) = 0;

    // 通过关键字获取指定单元的偏移量
    // 锁状态，加锁[管理器]
    virtual DWORD Locate( LPCSTR key ) = 0;

    // 还有另外一种方式得到指定单元的使用权，即不通过查询映射表直接检测制定对象的有效性（但在内部仍然会执行管理器的全局锁定）
    // 在通过后该单元将被加了锁，所以在使用完毕后需要释放资源！
    // 锁状态，加锁[对象]
    virtual LPVOID Useseg( DWORD offset, LPCSTR key ) = 0;
    virtual LPVOID Useseg( DWORD offset ) = 0;

    // 因为在获取指定单元的同时，该单元是被加了锁的，所以在使用完毕后需要释放资源！
    // 锁状态，解锁[对象]
    virtual void Useend( LPVOID block ) = 0;

    // 遍历所有有效对象
    // 锁状态，加锁[管理器/对象]，解锁[管理器/对象]
    virtual void Traverse( TCB fn, LPVOID argment ) = 0;
};

// 包装后的应用模版类
template < typename _type, DWORD count >
class ObjectCacheEx
{
public:
    DWORD Size() { return cache.Size(); }
    DWORD Capacity() { return cache.Capacity(); }
    BOOL isNewly() { return cache.isNewly(); }
    void Clear() { cache.Clear(); }
    BOOL Erase( LPCSTR key ) { return cache.Erase( key ); }
    BOOL Erase( DWORD offset ) { return cache.Erase( offset ); }
    _type& operator [] ( LPCSTR key ) { return *reinterpret_cast< _type* >( cache[ key ] ); }
    DWORD Locate( LPCSTR key ) { return cache.Locate( key ); }
    _type* Useseg( DWORD offset ) { return reinterpret_cast< _type* >( cache.Useseg( offset ) ); }
    void Useend( _type *block ) { cache.Useend( block ); }
    template < typename _functor > void Traverse( _functor &functor ) 
    { 
        cache.Traverse( TCB_Functor< void >::GetFunctor( &functor ), ( LPVOID )&functor ); 
    }

public:
    ObjectCacheEx( LPCSTR uniqueName, BYTE keySize = 30 ) : 
      cache( SharedMemoryCacheEx::Generate( uniqueName, count, sizeof( _type ), keySize ) ) {}

    void TheEnd() { cache.Release(); }

private:
    template < typename _functor >
    struct TCB_Functor 
    {
        template < typename _functor > static SharedMemoryCacheEx::TCB 
            GetFunctor( _functor functor ) { return TCB_Functor< _functor >::TCB_Call; }

        static BOOL CALLBACK TCB_Call( LPVOID key, LPVOID element, LPVOID argment )
        {
            return ( *reinterpret_cast< _functor >( argment ) )
                ( reinterpret_cast< LPCSTR >( key ), reinterpret_cast< _type& >( *( LPBYTE )element ) );
        };
    };

    SharedMemoryCacheEx &cache;
};
