#include "stdafx.h"
#include "smcachesp.h"
#include "threadex.h"
#include <windows.h>
#include <string>
#include <cassert>

#define exception std::exception

static DWORD lh_strhash( LPCVOID mem, size_t size ) 
{ 
    if ( mem == NULL ) 
        return 0; 

    DWORD ret = 0; 
    LPWORD s = ( LPWORD )mem; 
    size_t l = ( size + 1 ) / 2; 
    for ( size_t i = 0; i < l; i ++ )
        ret ^= ( s[i] << ( i&0x0f ) ); 

    return ret; 
}

struct details_SharedMemoryCacheEx :
    public SharedMemoryCacheEx
{
    static const int HASHKEYCAPACITY = 0x40000; // 26万个桶，占地1M

    // 链表节点，因为该共享空间的基地址是浮动的，所以需要设定segment来重定位目标地址
    struct ChainNode
    {
        DWORD offset;

        // 这里使用到了 GetThreadTimes 获取到线程创建的时间！
        // 该时间是FILETIME格式，以100纳秒为单位，从1601年1月1日开始计时
        // 所以1秒就为 1,000,000,000 / 100 = 10,000,000，使用 64bit 可以表示到 1601 + 58494 年
        // 而我们用来做判断的数据并不需要这么精确，能够达到毫秒级就可以了，所以可以再除以1w
        DWORD idHolder; // 当前是哪个线程正在持有资源32bit值，高14bit保存线程ID，低18bit保存该线程的唯一性时间戳！

        void Lock() 
        {
            FILETIME ct, ot;    // 获取当前线程的创建时间戳，不能失败，也不大可能失败
            while ( !GetThreadTimes( GetCurrentThread(), &ct, &ot, &ot, &ot ) ) { Sleep( 1 ); }

            // 将时间单位扩大到以毫秒为单位，这样就可以用19bit来保存524秒内的数据变化
            DWORD tid = GetCurrentThreadId();
            if ( tid & 0xffffc000 )
                throw exception( "当前线程由于某种特殊情况，导致线程ID大于8192*2，无法适应当前的共享内存管理规则！" );

            DWORD idTemp = tid | ( static_cast< DWORD >( reinterpret_cast< unsigned __int64 & >( ct ) / 10000 ) << 14 );

_retry:
            // 先尝试直接设置数据
            LONG result = InterlockedCompareExchange( ( LPLONG )&idHolder, idTemp, 0 );
            if ( result == 0 )
                return;     // 居然成功了！直接返回。。。这应该是最普遍的情况，性能也是最高的！

            // 先休眠一下，至少将CPU资源释放出来！
            Sleep( 1 ); 

            // 不成功，说明有其他线程正持有该资源！但由于有可能目标线程已经死亡导致资源未释放，则需要检测状态
            if ( HANDLE threadHandle = OpenThread( THREAD_QUERY_INFORMATION, FALSE, result & 0x3fff ) )
            {
                // 如果打开成功，则说明该线程还有效，但因为线程ID会被重用，所以需要获取目标线程的时间戳，进行匹配
                while ( !GetThreadTimes( threadHandle, &ct, &ot, &ot, &ot ) ) { Sleep( 1 ); }

                CloseHandle( threadHandle );

                if ( ( result & 0xffffe000 ) == 
                    ( static_cast< DWORD >( reinterpret_cast< unsigned __int64 & >( ct ) / 10000 ) << 14 ) )
                {
                    // 时间戳也匹配上了，说明目标线程是有效存活的，需要等待结束！
                    // 但在这个过程中可能目标会死亡，所以整个的判断需要重复重复再重复。。。
                    goto _retry;
                }
            }

            // 如果检测失败，则说明目标线程已经不存在了，或者已经不是当初启动的那个线程，则强制抢占资源！
            if ( InterlockedCompareExchange( ( LPLONG )&idHolder, idTemp, result ) != result )
                goto _retry;    // 抢占也会失败，可能被其他的线程抢先了吧，只能继续等待，重复重复再重复。。。
        }

        void Unlock() 
        {
            FILETIME ct, ot;    // 获取当前线程的创建时间戳，不能失败，也不大可能失败
            while ( !GetThreadTimes( GetCurrentThread(), &ct, &ot, &ot, &ot ) ) { Sleep( 1 ); }
            DWORD idTemp = GetCurrentThreadId() | 
                ( static_cast< DWORD >( reinterpret_cast< unsigned __int64 & >( ct ) / 10000 ) << 14 );

            if ( InterlockedCompareExchange( ( LPLONG )&idHolder, 0, idTemp ) != idTemp )
                throw exception( "释放锁资源时，发现持有者不是自己！" );
        }

        LPVOID data() { return ( LPBYTE )this + sizeof( ChainNode ); }

        ChainNode* next( LPVOID segment )
        {
            if ( offset == 0 )
                return NULL;

			return ( ChainNode* )( ( LPBYTE )segment + offset );
        }

        void Insert( LPVOID segment, ChainNode *node )
        {
            if ( ( node->idHolder & 0x3fff ) != GetCurrentThreadId() )
                throw exception( "试图插入一个非当前线程控制的对象！" );

			node->offset = offset;
			offset = ( DWORD )( ( __w64 DWORD )node - ( __w64 DWORD )segment );
        }

        ChainNode* Pop( LPVOID segment )
        {
            if ( offset == 0 )
                return NULL;

            ChainNode *result = next( segment );
            offset = result->offset;
            return result;
        }

        BOOL Remove( LPVOID segment, ChainNode *node, ChainNode *holder )
        {
            if ( ( node->idHolder & 0x3fff ) != GetCurrentThreadId() )
                throw exception( "试图销毁一个非当前线程控制的对象！" );

            ChainNode *iter = next( segment );
            if ( iter == node )
                return ( offset = node->offset ), holder->Insert( segment, node ), TRUE;

            while ( iter != NULL )
            {
                if ( iter->next( segment ) == node )
                    return ( iter->offset = node->offset ), holder->Insert( segment, node ), TRUE;

                iter = iter->next( segment );
            }

            return FALSE;
        }
    };

    typedef UGE::Details::Barrier< ChainNode > NodeBarrier;

    struct Header_32byte
    {
        DWORD   elementSize;        // 元素大小
        
        WORD    capacity;           // 设定的最大容器空间
        BYTE    keySize;            // 关键字的有效大小
        BYTE    keyCapacity;        // 关键字的空间大小

        WORD    countUnuse;         // 处于闲置的元素数量

        ChainNode elementsUnuse;    // 处于闲置的对象链表，自旋锁也放在了里边，用于在多线程/进程同时对容器进行操作时保证互斥

        DWORD   dummy[2];

        BOOL    quitFlag;           // 退出标志,提供该标志用于守护进程将缓存释放
    };

    inline Header_32byte* GetHeader() { return reinterpret_cast< Header_32byte* >( memoryAddress ); }

    void initialize( LPCSTR name, WORD capacity, DWORD elementSize, BYTE keySize )
    {
        if ( ( capacity < 1 ) || ( capacity > 0xffff ) || 
            ( keySize < 1 ) || ( keySize > 127 ) || ( elementSize < 1 )  ) 
            throw exception( "error in argment ( details_SharedMemoryCacheEx )" );

        // 16字节对齐
        BYTE keyCapacity = keySize;
        if ( keyCapacity & 0xf ) keyCapacity = ( keyCapacity + 0xf ) & ~0xf;
        if ( elementSize & 0xf ) elementSize = ( elementSize + 0xf ) & ~0xf;

        // 容器结构：32byteHader + hash空间 + Blocks[ 拉链指针 + 自旋锁标记 + 关键字空间 + 对象数据空间 ]
        assert( sizeof( Header_32byte ) == 32 );
        DWORD fullSize = sizeof( Header_32byte ) + sizeof( DWORD ) * HASHKEYCAPACITY + 
            ( sizeof( ChainNode ) + keyCapacity + elementSize ) * capacity;

        // 假设现在能够容忍的最大限制为1GBbyte
        assert( fullSize <= 1024 * 1024 * 1024 );

        // 先尝试打开指定名称的共享内存对象
        std::string uniqueName = std::string( "Global\\" ) + name;
        if ( uniqueName.length() >= 256 )
            throw exception( "uniqueName too long !" );

        memoryMapFile = OpenFileMapping( FILE_MAP_ALL_ACCESS, TRUE, uniqueName.c_str() );

        // 如果没有这个对象，则新创建并备份句柄到守护进程
        if ( _isNewly = ( memoryMapFile == NULL ) )
        {
            memoryMapFile = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, 
                PAGE_READWRITE, 0, fullSize, uniqueName.c_str() );

            if ( memoryMapFile == NULL )
                throw exception( "createfiemapping failure !" );
        }

        memoryAddress = MapViewOfFile( memoryMapFile, FILE_MAP_ALL_ACCESS, 0, 0, fullSize );
        if ( memoryAddress == NULL )
            throw exception( "memoryAddress is null !" );

        hashEntry = ( LPVOID* )( ( LPBYTE )memoryAddress + sizeof( Header_32byte ) );
        dataEntry = ( LPBYTE )hashEntry + sizeof( DWORD ) * HASHKEYCAPACITY;
        Header_32byte &header = *GetHeader();

        if ( _isNewly )
        {
            memset( &header, 0, sizeof( Header_32byte ) );
            header.elementSize = elementSize;
            header.capacity = capacity;
            header.keySize = keySize;
            header.keyCapacity = keyCapacity;

            Clear();

            STARTUPINFO si;
            PROCESS_INFORMATION pi;
            ZeroMemory( &si, sizeof(si) );
            ZeroMemory( &pi, sizeof(pi) );
            si.cb = sizeof(si);

            #define SMCHNAME "SMCacheHolder "
            char tempStr[512];
            memcpy( tempStr, SMCHNAME, sizeof( SMCHNAME ) );
            memcpy( tempStr + sizeof( SMCHNAME ) - 1, uniqueName.c_str(), uniqueName.length() + 1 );
            if ( !CreateProcess( NULL, tempStr, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) ) 
                throw exception( "create smcholder failure !" );

            HANDLE _handle;
            if ( !DuplicateHandle( GetCurrentProcess(), memoryMapFile, 
                pi.hProcess, &_handle, DUPLICATE_SAME_ACCESS, TRUE, DUPLICATE_SAME_ACCESS ) )
                throw exception( "duplicate memorymaphandle failure !" );

            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );
        }
        else
        {
            if ( ( header.elementSize != elementSize ) || ( header.capacity != capacity ) ||
                ( header.keySize != keySize ) || ( header.keyCapacity != keyCapacity ) )
                throw exception( "共享空间冲突了！" );
        }
    }

    details_SharedMemoryCacheEx( LPCSTR name, WORD capacity, DWORD elementSize, BYTE keySize, BOOL allocated ) :
        allocated( allocated )
    {
        memoryMapFile = INVALID_HANDLE_VALUE;
        memoryAddress = dataEntry = hashEntry = NULL;
        _isNewly = FALSE;

        try { initialize( name, capacity, elementSize, keySize ); } 

        catch ( exception & ) { Release(); throw; }
    }

    virtual void Release() 
    {
        if ( memoryAddress )
            ( GetHeader()->quitFlag = TRUE ), UnmapViewOfFile( memoryAddress );

        if ( ( memoryMapFile != NULL ) && ( memoryMapFile != INVALID_HANDLE_VALUE ) )
            CloseHandle( memoryMapFile );

        memoryMapFile = INVALID_HANDLE_VALUE;
        memoryAddress = dataEntry = hashEntry = NULL;

        // 如果是分配出来的对象，则需要释放内存
        if ( allocated )
            delete this;
    }

    virtual DWORD Size() { return GetHeader()->capacity - GetHeader()->countUnuse; }

    virtual DWORD Capacity() { return GetHeader()->capacity; }

    virtual BOOL isNewly() { return _isNewly; }

    virtual void Clear() 
    { 
        // 这里用于保持整个管理器的数据不被重入修改
        NodeBarrier barrier( GetHeader()->elementsUnuse );

        Header_32byte &header = *GetHeader();
        header.countUnuse = header.capacity;
        header.elementsUnuse.offset = 0;
        
        memset( hashEntry, 0, sizeof( DWORD ) * HASHKEYCAPACITY );

        LPVOID pointer = ( LPBYTE )memoryAddress + sizeof( Header_32byte ) + sizeof( DWORD ) * HASHKEYCAPACITY;
        for ( int i = 0; i < header.capacity; i ++ )
        {
            LPVOID blockSegment = ( LPBYTE )pointer + ( sizeof( ChainNode ) + header.keyCapacity + header.elementSize ) * i;
            // 这里用于保证其他用户并没有持有这个数据对象，并且在持有资源的情况下做Insert，否则会导致数据异常
            NodeBarrier barrier( *reinterpret_cast< ChainNode* >( blockSegment ) );
            header.elementsUnuse.Insert( memoryAddress, ( ChainNode* )blockSegment );
        }
    }

	virtual BOOL Erase( DWORD block )
    {
        const Header_32byte &header = *GetHeader();

        DWORD segment = sizeof( Header_32byte ) + sizeof( DWORD ) * HASHKEYCAPACITY;
        DWORD offset = segment + ( sizeof( ChainNode ) + header.keyCapacity + header.elementSize ) * ( header.capacity - 1 );

        if ( block < segment || block > offset )
            return FALSE;   // 该地址不在管理器管理范围内

        if ( ( ( block - segment ) % ( sizeof( ChainNode ) + header.keyCapacity + header.elementSize ) ) != 0 )
            return FALSE;
            // throw exception( "bad block!" );    // 该地址不是有效的数据块首！

        // 这里用于保持整个管理器的数据不被重入修改
        NodeBarrier barrier1( GetHeader()->elementsUnuse );

        ChainNode *node = ( ChainNode* )( ( LPBYTE )memoryAddress + block );
		if ( ( ( LPBYTE )node->data() )[0] == 0 )
            return FALSE;   // 已经被清除key数据的数据块

        // 通过，获取使用权
		NodeBarrier barrier2( *node );

		if ( ( ( LPBYTE )node->data() )[0] == 0 )
            return FALSE;   // 已经被清除key数据的数据块

		size_t len = strlen( ( LPCSTR )node->data() );
		DWORD hash = lh_strhash( ( LPCSTR )node->data(), len ) & ( HASHKEYCAPACITY - 1 );
		
		if ( ( ( ChainNode& )hashEntry[ hash ] ).Remove( memoryAddress, node, &GetHeader()->elementsUnuse ) )
			GetHeader()->countUnuse ++;
		
		// 更新关键字
		memset( ( LPBYTE )node->data(), 0, GetHeader()->keyCapacity );

        return TRUE;
    }

    virtual BOOL Erase( LPCVOID key ) 
    { 
        size_t len = strlen( ( LPCSTR )key );
        if ( len > GetHeader()->keySize )
            throw exception( "key string too long !" );

        // 在末尾加上\0，便于使用memcmp来比较
        size_t minSize = min( len, GetHeader()->keySize ) + 1;
        DWORD hash = lh_strhash( key, len ) & ( HASHKEYCAPACITY - 1 );

        // 这里用于保持整个管理器的数据不被重入修改
        NodeBarrier barrier( GetHeader()->elementsUnuse );

        for ( ChainNode *iter = ( ( ChainNode& )hashEntry[ hash ] ).next( memoryAddress ); 
			iter != NULL; iter = iter->next( memoryAddress ) )
        {
            if ( memcmp( ( LPBYTE )iter->data(), key, minSize ) == 0 )
            {
                NodeBarrier barrier( *iter );
                if ( ( ( ChainNode& )hashEntry[ hash ] ).Remove( memoryAddress, iter, &GetHeader()->elementsUnuse ) )
                    GetHeader()->countUnuse ++;

                // 更新关键字
                memset( ( LPBYTE )iter->data(), 0, GetHeader()->keyCapacity );

                return TRUE;
            }
        }

        return FALSE;
    }

    // 获取指定单元（一般情况下，key为字符串指针，特殊情况下，如keySize<=4，则key为32bit整数值！）
    virtual LPVOID operator [] ( LPCVOID key ) 
    {
        size_t len = strlen( ( LPCSTR )key );
        if ( len > GetHeader()->keySize )
            throw exception( "key string too long !" );

        // 在末尾加上\0，便于使用memcmp来比较
        size_t minSize = min( len, GetHeader()->keySize ) + 1;
        DWORD hash = lh_strhash( key, len ) & ( HASHKEYCAPACITY - 1 );

        // 这里用于保持整个管理器的数据不被重入修改
        NodeBarrier barrier( GetHeader()->elementsUnuse );

        for ( ChainNode *iter = ( ( ChainNode& )hashEntry[ hash ] ).next( memoryAddress ); 
			iter != NULL; iter = iter->next( memoryAddress ) )
        {
            if ( memcmp( ( LPBYTE )iter->data(), key, minSize ) == 0 )
            {
                iter->Lock(); // 在返回数据前，先让当前线程保持该资源的所有权！
                return ( LPBYTE )iter->data() + GetHeader()->keyCapacity;
            }
        }

        // 添加拉链，并将其从闲置列表中移出
        ChainNode *node = GetHeader()->elementsUnuse.Pop( memoryAddress );
        assert( node );
        if ( node == NULL ) 
            return 0;

        GetHeader()->countUnuse --;

        // 在返回数据前，先让当前线程保持该资源的所有权！
        node->Lock(); 
        ( ( ChainNode& )hashEntry[ hash ] ).Insert( memoryAddress, node );

        // 更新关键字
        memcpy( ( LPBYTE )node->data(), key, minSize );

        return ( LPBYTE )node->data() + GetHeader()->keyCapacity;
    }

    // 遍历所有有效对象
    virtual void Traverse( TCB fn, LPVOID argment ) 
    {
        // 这里用于保持整个管理器的数据不被重入修改
        NodeBarrier barrier( GetHeader()->elementsUnuse );

        // 这种做法效率并不高，可考虑使用双向链表来提高效率
        // 但因为在这里遍历应该是很少用到的功能，所以暂不考虑优化问题
        for ( int i = 0; i < HASHKEYCAPACITY; i++ )
        {
            if ( ( ( ChainNode& )hashEntry[ i ] ).next( memoryAddress ) == NULL )
                continue;

            for ( ChainNode *it = ( ( ChainNode& )hashEntry[ i ] ).next( memoryAddress ); 
                it != NULL; it = it->next( memoryAddress ) )
            {
                NodeBarrier barrier( *it );

                fn( ( LPBYTE )it + sizeof( ChainNode ), 
                    ( LPBYTE )it + sizeof( ChainNode ) + GetHeader()->keyCapacity, argment );
            }
        }
    }

    virtual DWORD Locate( LPCSTR key )
    {
        size_t len = strlen( ( LPCSTR )key );
        if ( len > GetHeader()->keySize )
            throw exception( "key string too long !" );

        // 在末尾加上\0，便于使用memcmp来比较
        size_t minSize = min( len, GetHeader()->keySize ) + 1;
        DWORD hash = lh_strhash( key, len ) & ( HASHKEYCAPACITY - 1 );

        // 这里用于保持整个管理器的数据不被重入修改
        NodeBarrier barrier( GetHeader()->elementsUnuse );

        for ( ChainNode *iter = ( ( ChainNode& )hashEntry[ hash ] ).next( memoryAddress ); 
			iter != NULL; iter = iter->next( memoryAddress ) )
        {
            if ( memcmp( ( LPBYTE )iter->data(), key, minSize ) == 0 )
                // 这里只是为了获取目标数据的地址，所以不需要加锁！
                return static_cast< DWORD >( ( LPBYTE )iter - ( LPBYTE )memoryAddress ); 
        }

        return -1;
    }

    virtual LPVOID Useseg( DWORD block )
    {
        const Header_32byte &header = *GetHeader();

        DWORD segment = sizeof( Header_32byte ) + sizeof( DWORD ) * HASHKEYCAPACITY;
        DWORD offset = segment + ( sizeof( ChainNode ) + header.keyCapacity + header.elementSize ) * ( header.capacity - 1 );

        if ( block < segment || block > offset )
            return FALSE;   // 该地址不在管理器管理范围内

        if ( ( ( block - segment ) % ( sizeof( ChainNode ) + header.keyCapacity + header.elementSize ) ) != 0 )
            return FALSE;   // 该地址不在管理器管理范围内
            // throw exception( "bad block!" );    // 该地址不是有效的数据块首！

        // 这里用于保持整个管理器的数据不被重入修改
        // 有这个必要吗？本来这块内存就必定是有效存在的，唯一可能出现的可能就是该数据块被释放或被其他线程抢占
        // 不管是释放也好，抢占也好，资源地址的有效性应该是保持不变的，通过对比关键字，应该就可以直接得到结果！
        // NodeBarrier barrier( GetHeader()->elementsUnuse );

        ChainNode *node = ( ChainNode* )( ( LPBYTE )memoryAddress + block );

		if ( ( ( LPBYTE )node->data() )[0] == 0 )
            return FALSE;   // 已经被清除key数据的数据块

        // 通过，获取使用权
        node->Lock();

        // 再次对比结果，看数据是否依然有效！
		if ( ( ( LPBYTE )node->data() )[0] == 0 )
            return node->Unlock(), FALSE;

        return reinterpret_cast< LPBYTE >( node->data() ) + GetHeader()->keyCapacity;
    }

    virtual LPVOID Useseg( DWORD block, LPCSTR key )
    {
        const Header_32byte &header = *GetHeader();

        size_t len = strlen( ( LPCSTR )key );
        if ( len > header.keySize )
            throw exception( "key string too long !" );

        // 在末尾加上\0，便于使用memcmp来比较
        size_t minSize = min( len, GetHeader()->keySize ) + 1;

        DWORD segment = sizeof( Header_32byte ) + sizeof( DWORD ) * HASHKEYCAPACITY;
        DWORD offset = segment + ( sizeof( ChainNode ) + header.keyCapacity + header.elementSize ) * ( header.capacity - 1 );

        if ( block < segment || block > offset )
            return FALSE;   // 该地址不在管理器管理范围内

        if ( ( ( block - segment ) % ( sizeof( ChainNode ) + header.keyCapacity + header.elementSize ) ) != 0 )
            return FALSE;
            // throw exception( "bad block!" );    // 该地址不是有效的数据块首！

        // 这里用于保持整个管理器的数据不被重入修改
        // 有这个必要吗？本来这块内存就必定是有效存在的，唯一可能出现的可能就是该数据块被释放或被其他线程抢占
        // 不管是释放也好，抢占也好，资源地址的有效性应该是保持不变的，通过对比关键字，应该就可以直接得到结果！
        // NodeBarrier barrier( GetHeader()->elementsUnuse );

        ChainNode *node = ( ChainNode* )( ( LPBYTE )memoryAddress + block );

        if ( memcmp( ( LPBYTE )node->data(), key, minSize ) != 0 )
            return FALSE;   // 不同，当然就不能通过拉

        // 通过，获取使用权
        node->Lock();

        // 再次对比结果，看数据是否依然有效！
        if ( memcmp( ( LPBYTE )node->data(), key, minSize ) != 0 )
            return node->Unlock(), FALSE;   // 不同，说明上一次比较后，已经有其他线程对他进行了数据修改（释放或者抢占）

        return reinterpret_cast< LPBYTE >( node->data() ) + GetHeader()->keyCapacity;
    }

    // 因为在获取指定单元的同时，该单元是被加了锁的，所以在使用完毕后需要释放资源！
    virtual void Useend( LPVOID block )
    {
        const Header_32byte &header = *GetHeader();

        ChainNode *node = reinterpret_cast< ChainNode* >( reinterpret_cast< LPBYTE >( block ) - 
            sizeof( ChainNode ) - header.keyCapacity );

        LPBYTE segment = ( LPBYTE )memoryAddress + sizeof( Header_32byte ) + sizeof( DWORD ) * HASHKEYCAPACITY;
        LPBYTE offset = segment + ( sizeof( ChainNode ) + header.keyCapacity + header.elementSize ) * ( header.capacity - 1 );

        if ( ( LPBYTE )node < segment || ( LPBYTE )node > offset )
            return;

        if ( ( ( ( LPBYTE )node - segment ) % ( sizeof( ChainNode ) + header.keyCapacity + header.elementSize ) ) != 0 )
            throw exception( "bad block!" );    // 该地址不是有效的数据块首！

        node->Unlock();
    }

    HANDLE memoryMapFile;
    LPVOID memoryAddress;
    LPVOID* hashEntry;
    LPVOID dataEntry;
    BOOL _isNewly;

    const BOOL allocated;
};

SharedMemoryCacheEx& SharedMemoryCacheEx::Singleton( LPCSTR uniqueName, WORD capacity, DWORD elementSize, BYTE keySize )
{
    static details_SharedMemoryCacheEx singleton( uniqueName, capacity, elementSize, keySize, FALSE );
    return singleton;
}

SharedMemoryCacheEx& SharedMemoryCacheEx::Generate( LPCSTR uniqueName, WORD capacity, DWORD elementSize, BYTE keySize )
{
    return * new details_SharedMemoryCacheEx( uniqueName, capacity, elementSize, keySize, TRUE );
}

/*
        // 这里使用到了 GetThreadTimes 获取到线程创建的时间！
        // 该时间是FILETIME格式，以100纳秒为单位，从1601年1月1日开始计时
        // 所以1秒就为 1,000,000,000 / 100 = 10,000,000，使用 64bit 可以表示到 1601 + 58494 年
        // 而我们用来做判断的数据并不需要这么精确，能够达到毫秒级就可以了，所以可以再除以1w
        union {
            struct {
                DWORD idHolder, timestamp; // 当前是哪个线程正在持有资源64bit值，高32bit保存线程ID，低32bit保存该线程的唯一性时间戳！
            };
            LONGLONG llvalue;
        };

        void Lock() 
        {
            FILETIME ct, ot;    // 获取当前线程的创建时间戳，不能失败，也不大可能失败
            while ( !GetThreadTimes( GetCurrentThread(), &ct, &ot, &ot, &ot ) ) { Sleep( 1 ); }

            // 将时间单位扩大到以毫秒为单位，这样就可以用32bit来保存近48天的数据变化
            LONGLONG lltemp;
            reinterpret_cast< LPDWORD >( &lltemp )[0] = GetCurrentThreadId();
            reinterpret_cast< LPDWORD >( &lltemp )[1] = 
                static_cast< DWORD >( reinterpret_cast< unsigned __int64 & >( ct ) / 10000 );

_fullretry:
            // 先尝试直接设置数据
            LONGLONG result = InterlockedCompareExchange64( &llvalue, lltemp, 0 );
            if ( result == 0 )
                return;     // 居然成功了！直接返回。。。这应该是最普遍的情况，性能也是最高的！

_retry:
            // 先休眠一下，至少将CPU资源释放出来！
            Sleep( 0 ); 

            // 不成功，说明有其他线程正持有该资源！但由于有可能目标线程已经死亡导致资源未释放，则需要检测状态
            if ( HANDLE threadHandle = OpenThread( 
                THREAD_QUERY_INFORMATION, FALSE, reinterpret_cast< LPDWORD >( &result  )[0] ) )
            {
                // 如果打开成功，则说明该线程还有效，但因为线程ID会被重用，所以需要获取目标线程的时间戳，进行匹配
                while ( !GetThreadTimes( threadHandle, &ct, &ot, &ot, &ot ) ) { Sleep( 1 ); }

                CloseHandle( threadHandle );

                if ( reinterpret_cast< LPDWORD >( &result )[1] == 
                    static_cast< DWORD >( reinterpret_cast< unsigned __int64 & >( ct ) / 10000 ) )
                {
                    // 时间戳也匹配上了，说明目标线程是有效存活的，需要等待结束！
                    if ( InterlockedCompareExchange64( &llvalue, lltemp, 0 ) == 0 )
                        return;     // 成功等待结束，返回;

                    // 但在这个过程中可能目标会死亡，所以整个的判断需要重复重复再重复。。。
                    goto _retry;
                }
            }

            // 如果检测失败，则说明目标线程已经不存在了，或者已经不是当初启动的那个线程，则强制抢占资源！
            if ( InterlockedCompareExchange64( &llvalue, lltemp, result ) != result )
                goto _fullretry;    // 抢占也会失败，可能被其他的线程抢先了吧，只能继续等待，重复重复再重复。。。
        }

        void Unlock() 
        {
            FILETIME ct, ot;    // 获取当前线程的创建时间戳，不能失败，也不大可能失败
            while ( !GetThreadTimes( GetCurrentThread(), &ct, &ot, &ot, &ot ) ) { Sleep( 1 ); }

            // 将时间单位扩大到以毫秒为单位，这样就可以用32bit来保存近48天的数据变化
            LONGLONG lltemp;
            reinterpret_cast< LPDWORD >( &lltemp )[0] = GetCurrentThreadId();
            reinterpret_cast< LPDWORD >( &lltemp )[1] = 
                static_cast< DWORD >( reinterpret_cast< unsigned __int64 & >( ct ) / 10000 );

            if ( InterlockedCompareExchange64( &llvalue, 0, lltemp ) != lltemp )
                throw exception( "释放锁资源时，发现持有者不是自己！" );
        }
*/
