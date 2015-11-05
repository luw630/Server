#include "stdafx.h"
#include "smcache.h"
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

struct details_SharedMemoryCache :
    public SharedMemoryCache
{
    static const int HASHKEYCAPACITY = 0x40000; // 26万个桶，占地1M

    // 链表节点，因为该共享空间的基地址是浮动的，所以需要设定segment来重定位目标地址
    struct ChainNode
    {
        DWORD offset;

        LPVOID data() { return ( LPBYTE )this + sizeof( ChainNode ); }

        ChainNode* next( LPVOID segment )
        {
            if ( offset == 0 )
                return NULL;

			return ( ChainNode* )( ( LPBYTE )segment + offset );
        }

        void Insert( LPVOID segment, ChainNode *node )
        {
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

        BOOL Remove( LPVOID segment, ChainNode *node )
        {
            ChainNode *iter = next( segment );
            if ( iter == node )
                return ( offset = node->offset ), TRUE;

            while ( iter != NULL )
            {
                if ( iter->next( segment ) == node )
                    return ( iter->offset = node->offset ), TRUE;

                iter = iter->next( segment );
            }

            return FALSE;
        }
    };

    struct Header_32byte
    {
        DWORD   elementSize;    // 元素大小
        
        WORD    capacity;       // 设定的最大容器空间
        BYTE    keySize;        // 关键字的有效大小
        BYTE    keyCapacity;    // 关键字的空间大小

        WORD    countUnuse;     // 处于闲置的元素数量

        ChainNode elementsInuse;    // 正被使用的对象链表
        ChainNode elementsUnuse;    // 处于闲置的对象链表

        DWORD   dummy[2];

        BOOL    quitFlag;       // 退出标志,提供该标志用于守护进程将缓存释放
    };

    inline Header_32byte* GetHeader() { return reinterpret_cast< Header_32byte* >( memoryAddress ); }

    void initialize( LPCSTR name, WORD capacity, DWORD elementSize, BYTE keySize )
    {
        if ( ( capacity < 1 ) || ( capacity > 0xffff ) || 
            ( keySize < 1 ) || ( keySize > 127 ) || ( elementSize < 1 )  ) 
            throw exception( "error in argment ( details_SharedMemoryCache )" );

        // 16字节对齐
        BYTE keyCapacity = keySize;
        if ( keyCapacity & 0xf ) keyCapacity = ( keyCapacity + 0xf ) & ~0xf;
        if ( elementSize & 0xf ) elementSize = ( elementSize + 0xf ) & ~0xf;

        // 容器结构：32byteHader + hash空间 + Blocks[ 拉链指针 + 关键字空间 + 对象数据空间 ]
        assert( sizeof( Header_32byte ) == 32 );
        DWORD fullSize = sizeof( Header_32byte ) + sizeof( DWORD ) * HASHKEYCAPACITY + 
            ( 4 + keyCapacity + elementSize ) * capacity;

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
            header.countUnuse = capacity;
            
            memset( hashEntry, 0, sizeof( DWORD ) * HASHKEYCAPACITY );

            LPVOID pointer = ( LPBYTE )memoryAddress + sizeof( Header_32byte ) + sizeof( DWORD ) * HASHKEYCAPACITY;
            for ( int i = 0; i < capacity; i ++ )
            {
                LPVOID blockSegment = ( LPBYTE )pointer + ( 4 + keyCapacity + elementSize ) * i;
                header.elementsUnuse.Insert( memoryAddress, ( ChainNode* )blockSegment );
            }

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

    details_SharedMemoryCache( LPCSTR name, WORD capacity, DWORD elementSize, BYTE keySize )
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
    }

    virtual DWORD Size() { return GetHeader()->capacity - GetHeader()->countUnuse; }

    virtual DWORD Capacity() { return GetHeader()->capacity; }

    virtual BOOL isNewly() { return _isNewly; }

    virtual void Clear() 
    { 
        Header_32byte &header = *GetHeader();
        header.countUnuse = header.capacity;
        header.elementsUnuse.offset = header.elementsInuse.offset = 0;
        
        memset( hashEntry, 0, sizeof( DWORD ) * HASHKEYCAPACITY );

        LPVOID pointer = ( LPBYTE )memoryAddress + sizeof( Header_32byte ) + sizeof( DWORD ) * HASHKEYCAPACITY;
        for ( int i = 0; i < header.capacity; i ++ )
        {
            LPVOID blockSegment = ( LPBYTE )pointer + ( 4 + header.keyCapacity + header.elementSize ) * i;
            header.elementsUnuse.Insert( memoryAddress, ( ChainNode* )blockSegment );
        }
    }

    virtual BOOL Erase( LPCVOID key ) 
    { 
        size_t len = strlen( ( LPCSTR )key );
        if ( len > GetHeader()->keySize )
            throw exception( "key string too long !" );

        // 在末尾加上\0，便于使用memcmp来比较
        size_t minSize = min( len, GetHeader()->keySize ) + 1;
        DWORD hash = lh_strhash( key, len ) & ( HASHKEYCAPACITY - 1 );

        for ( ChainNode *iter = ( ( ChainNode& )hashEntry[ hash ] ).next( memoryAddress ); 
			iter != NULL; iter = iter->next( memoryAddress ) )
        {
            if ( memcmp( ( LPBYTE )iter->data(), key, minSize ) == 0 )
            {
                ( ( ChainNode& )hashEntry[ hash ] ).Remove( memoryAddress, iter );
                GetHeader()->elementsUnuse.Insert( memoryAddress, iter );
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

        for ( ChainNode *iter = ( ( ChainNode& )hashEntry[ hash ] ).next( memoryAddress ); 
			iter != NULL; iter = iter->next( memoryAddress ) )
        {
            if ( memcmp( ( LPBYTE )iter->data(), key, minSize ) == 0 )
                return ( LPBYTE )iter->data() + GetHeader()->keyCapacity;
        }

        // 添加拉链，并将其从闲置列表中移出
        ChainNode *node = GetHeader()->elementsUnuse.Pop( memoryAddress );
        assert( node );
        if ( node == NULL ) return 0;

        GetHeader()->countUnuse --;
        ( ( ChainNode& )hashEntry[ hash ] ).Insert( memoryAddress, node );

        // 更新关键字
        memcpy( ( LPBYTE )node->data(), key, minSize );

        return ( LPBYTE )node->data() + GetHeader()->keyCapacity;
    }

    // 遍历所有有效对象
    virtual void Traverse( TCB fn, LPVOID argment ) 
    {
        // 这种做法效率并不高，可考虑使用双向链表来提高效率
        // 但因为在这里遍历应该是很少用到的功能，所以暂不考虑优化问题
        for ( int i = 0; i < HASHKEYCAPACITY; i++ )
        {
            if ( ( ( ChainNode& )hashEntry[ i ] ).next( memoryAddress ) == NULL )
                continue;

            for ( ChainNode *it = ( ( ChainNode& )hashEntry[ i ] ).next( memoryAddress ); 
				it != NULL; it = it->next( memoryAddress ) )
                fn( ( LPBYTE )it + 4, ( LPBYTE )it + 4 + GetHeader()->keyCapacity, argment );
        }

        //for ( ChainNode *it = GetHeader()->elementsInuse.next; it != NULL; it = it->next )
        //    fn( ( LPBYTE )it + 4, ( LPBYTE )it + 4 + GetHeader()->keyCapacity, argment );
    }

    HANDLE memoryMapFile;
    LPVOID memoryAddress;
    LPVOID* hashEntry;
    LPVOID dataEntry;
    BOOL _isNewly;
};

SharedMemoryCache& SharedMemoryCache::Singleton( LPCSTR uniqueName, WORD capacity, DWORD elementSize, BYTE keySize )
{
    static details_SharedMemoryCache singleton( uniqueName, capacity, elementSize, keySize );
    return singleton;
}
