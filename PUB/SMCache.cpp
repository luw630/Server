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
    static const int HASHKEYCAPACITY = 0x40000; // 26���Ͱ��ռ��1M

    // ����ڵ㣬��Ϊ�ù���ռ�Ļ���ַ�Ǹ����ģ�������Ҫ�趨segment���ض�λĿ���ַ
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
        DWORD   elementSize;    // Ԫ�ش�С
        
        WORD    capacity;       // �趨����������ռ�
        BYTE    keySize;        // �ؼ��ֵ���Ч��С
        BYTE    keyCapacity;    // �ؼ��ֵĿռ��С

        WORD    countUnuse;     // �������õ�Ԫ������

        ChainNode elementsInuse;    // ����ʹ�õĶ�������
        ChainNode elementsUnuse;    // �������õĶ�������

        DWORD   dummy[2];

        BOOL    quitFlag;       // �˳���־,�ṩ�ñ�־�����ػ����̽������ͷ�
    };

    inline Header_32byte* GetHeader() { return reinterpret_cast< Header_32byte* >( memoryAddress ); }

    void initialize( LPCSTR name, WORD capacity, DWORD elementSize, BYTE keySize )
    {
        if ( ( capacity < 1 ) || ( capacity > 0xffff ) || 
            ( keySize < 1 ) || ( keySize > 127 ) || ( elementSize < 1 )  ) 
            throw exception( "error in argment ( details_SharedMemoryCache )" );

        // 16�ֽڶ���
        BYTE keyCapacity = keySize;
        if ( keyCapacity & 0xf ) keyCapacity = ( keyCapacity + 0xf ) & ~0xf;
        if ( elementSize & 0xf ) elementSize = ( elementSize + 0xf ) & ~0xf;

        // �����ṹ��32byteHader + hash�ռ� + Blocks[ ����ָ�� + �ؼ��ֿռ� + �������ݿռ� ]
        assert( sizeof( Header_32byte ) == 32 );
        DWORD fullSize = sizeof( Header_32byte ) + sizeof( DWORD ) * HASHKEYCAPACITY + 
            ( 4 + keyCapacity + elementSize ) * capacity;

        // ���������ܹ����̵��������Ϊ1GBbyte
        assert( fullSize <= 1024 * 1024 * 1024 );

        // �ȳ��Դ�ָ�����ƵĹ����ڴ����
        std::string uniqueName = std::string( "Global\\" ) + name;
        if ( uniqueName.length() >= 256 )
            throw exception( "uniqueName too long !" );

        memoryMapFile = OpenFileMapping( FILE_MAP_ALL_ACCESS, TRUE, uniqueName.c_str() );

        // ���û������������´��������ݾ�����ػ�����
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
                throw exception( "����ռ��ͻ�ˣ�" );
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

        // ��ĩβ����\0������ʹ��memcmp���Ƚ�
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

                // ���¹ؼ���
                memset( ( LPBYTE )iter->data(), 0, GetHeader()->keyCapacity );

                return TRUE;
            }
        }

        return FALSE;
    }

    // ��ȡָ����Ԫ��һ������£�keyΪ�ַ���ָ�룬��������£���keySize<=4����keyΪ32bit����ֵ����
    virtual LPVOID operator [] ( LPCVOID key ) 
    {
        size_t len = strlen( ( LPCSTR )key );
        if ( len > GetHeader()->keySize )
            throw exception( "key string too long !" );

        // ��ĩβ����\0������ʹ��memcmp���Ƚ�
        size_t minSize = min( len, GetHeader()->keySize ) + 1;
        DWORD hash = lh_strhash( key, len ) & ( HASHKEYCAPACITY - 1 );

        for ( ChainNode *iter = ( ( ChainNode& )hashEntry[ hash ] ).next( memoryAddress ); 
			iter != NULL; iter = iter->next( memoryAddress ) )
        {
            if ( memcmp( ( LPBYTE )iter->data(), key, minSize ) == 0 )
                return ( LPBYTE )iter->data() + GetHeader()->keyCapacity;
        }

        // ���������������������б����Ƴ�
        ChainNode *node = GetHeader()->elementsUnuse.Pop( memoryAddress );
        assert( node );
        if ( node == NULL ) return 0;

        GetHeader()->countUnuse --;
        ( ( ChainNode& )hashEntry[ hash ] ).Insert( memoryAddress, node );

        // ���¹ؼ���
        memcpy( ( LPBYTE )node->data(), key, minSize );

        return ( LPBYTE )node->data() + GetHeader()->keyCapacity;
    }

    // ����������Ч����
    virtual void Traverse( TCB fn, LPVOID argment ) 
    {
        // ��������Ч�ʲ����ߣ��ɿ���ʹ��˫�����������Ч��
        // ����Ϊ���������Ӧ���Ǻ����õ��Ĺ��ܣ������ݲ������Ż�����
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
