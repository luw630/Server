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
    static const int HASHKEYCAPACITY = 0x40000; // 26���Ͱ��ռ��1M

    // ����ڵ㣬��Ϊ�ù���ռ�Ļ���ַ�Ǹ����ģ�������Ҫ�趨segment���ض�λĿ���ַ
    struct ChainNode
    {
        DWORD offset;

        // ����ʹ�õ��� GetThreadTimes ��ȡ���̴߳�����ʱ�䣡
        // ��ʱ����FILETIME��ʽ����100����Ϊ��λ����1601��1��1�տ�ʼ��ʱ
        // ����1���Ϊ 1,000,000,000 / 100 = 10,000,000��ʹ�� 64bit ���Ա�ʾ�� 1601 + 58494 ��
        // �������������жϵ����ݲ�����Ҫ��ô��ȷ���ܹ��ﵽ���뼶�Ϳ����ˣ����Կ����ٳ���1w
        DWORD idHolder; // ��ǰ���ĸ��߳����ڳ�����Դ32bitֵ����14bit�����߳�ID����18bit������̵߳�Ψһ��ʱ�����

        void Lock() 
        {
            FILETIME ct, ot;    // ��ȡ��ǰ�̵߳Ĵ���ʱ���������ʧ�ܣ�Ҳ�������ʧ��
            while ( !GetThreadTimes( GetCurrentThread(), &ct, &ot, &ot, &ot ) ) { Sleep( 1 ); }

            // ��ʱ�䵥λ�����Ժ���Ϊ��λ�������Ϳ�����19bit������524���ڵ����ݱ仯
            DWORD tid = GetCurrentThreadId();
            if ( tid & 0xffffc000 )
                throw exception( "��ǰ�߳�����ĳ����������������߳�ID����8192*2���޷���Ӧ��ǰ�Ĺ����ڴ�������" );

            DWORD idTemp = tid | ( static_cast< DWORD >( reinterpret_cast< unsigned __int64 & >( ct ) / 10000 ) << 14 );

_retry:
            // �ȳ���ֱ����������
            LONG result = InterlockedCompareExchange( ( LPLONG )&idHolder, idTemp, 0 );
            if ( result == 0 )
                return;     // ��Ȼ�ɹ��ˣ�ֱ�ӷ��ء�������Ӧ�������ձ�����������Ҳ����ߵģ�

            // ������һ�£����ٽ�CPU��Դ�ͷų�����
            Sleep( 1 ); 

            // ���ɹ���˵���������߳������и���Դ���������п���Ŀ���߳��Ѿ�����������Դδ�ͷţ�����Ҫ���״̬
            if ( HANDLE threadHandle = OpenThread( THREAD_QUERY_INFORMATION, FALSE, result & 0x3fff ) )
            {
                // ����򿪳ɹ�����˵�����̻߳���Ч������Ϊ�߳�ID�ᱻ���ã�������Ҫ��ȡĿ���̵߳�ʱ���������ƥ��
                while ( !GetThreadTimes( threadHandle, &ct, &ot, &ot, &ot ) ) { Sleep( 1 ); }

                CloseHandle( threadHandle );

                if ( ( result & 0xffffe000 ) == 
                    ( static_cast< DWORD >( reinterpret_cast< unsigned __int64 & >( ct ) / 10000 ) << 14 ) )
                {
                    // ʱ���Ҳƥ�����ˣ�˵��Ŀ���߳�����Ч���ģ���Ҫ�ȴ�������
                    // ������������п���Ŀ��������������������ж���Ҫ�ظ��ظ����ظ�������
                    goto _retry;
                }
            }

            // ������ʧ�ܣ���˵��Ŀ���߳��Ѿ��������ˣ������Ѿ����ǵ����������Ǹ��̣߳���ǿ����ռ��Դ��
            if ( InterlockedCompareExchange( ( LPLONG )&idHolder, idTemp, result ) != result )
                goto _retry;    // ��ռҲ��ʧ�ܣ����ܱ��������߳������˰ɣ�ֻ�ܼ����ȴ����ظ��ظ����ظ�������
        }

        void Unlock() 
        {
            FILETIME ct, ot;    // ��ȡ��ǰ�̵߳Ĵ���ʱ���������ʧ�ܣ�Ҳ�������ʧ��
            while ( !GetThreadTimes( GetCurrentThread(), &ct, &ot, &ot, &ot ) ) { Sleep( 1 ); }
            DWORD idTemp = GetCurrentThreadId() | 
                ( static_cast< DWORD >( reinterpret_cast< unsigned __int64 & >( ct ) / 10000 ) << 14 );

            if ( InterlockedCompareExchange( ( LPLONG )&idHolder, 0, idTemp ) != idTemp )
                throw exception( "�ͷ�����Դʱ�����ֳ����߲����Լ���" );
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
                throw exception( "��ͼ����һ���ǵ�ǰ�߳̿��ƵĶ���" );

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
                throw exception( "��ͼ����һ���ǵ�ǰ�߳̿��ƵĶ���" );

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
        DWORD   elementSize;        // Ԫ�ش�С
        
        WORD    capacity;           // �趨����������ռ�
        BYTE    keySize;            // �ؼ��ֵ���Ч��С
        BYTE    keyCapacity;        // �ؼ��ֵĿռ��С

        WORD    countUnuse;         // �������õ�Ԫ������

        ChainNode elementsUnuse;    // �������õĶ�������������Ҳ��������ߣ������ڶ��߳�/����ͬʱ���������в���ʱ��֤����

        DWORD   dummy[2];

        BOOL    quitFlag;           // �˳���־,�ṩ�ñ�־�����ػ����̽������ͷ�
    };

    inline Header_32byte* GetHeader() { return reinterpret_cast< Header_32byte* >( memoryAddress ); }

    void initialize( LPCSTR name, WORD capacity, DWORD elementSize, BYTE keySize )
    {
        if ( ( capacity < 1 ) || ( capacity > 0xffff ) || 
            ( keySize < 1 ) || ( keySize > 127 ) || ( elementSize < 1 )  ) 
            throw exception( "error in argment ( details_SharedMemoryCacheEx )" );

        // 16�ֽڶ���
        BYTE keyCapacity = keySize;
        if ( keyCapacity & 0xf ) keyCapacity = ( keyCapacity + 0xf ) & ~0xf;
        if ( elementSize & 0xf ) elementSize = ( elementSize + 0xf ) & ~0xf;

        // �����ṹ��32byteHader + hash�ռ� + Blocks[ ����ָ�� + ��������� + �ؼ��ֿռ� + �������ݿռ� ]
        assert( sizeof( Header_32byte ) == 32 );
        DWORD fullSize = sizeof( Header_32byte ) + sizeof( DWORD ) * HASHKEYCAPACITY + 
            ( sizeof( ChainNode ) + keyCapacity + elementSize ) * capacity;

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
                throw exception( "����ռ��ͻ�ˣ�" );
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

        // ����Ƿ�������Ķ�������Ҫ�ͷ��ڴ�
        if ( allocated )
            delete this;
    }

    virtual DWORD Size() { return GetHeader()->capacity - GetHeader()->countUnuse; }

    virtual DWORD Capacity() { return GetHeader()->capacity; }

    virtual BOOL isNewly() { return _isNewly; }

    virtual void Clear() 
    { 
        // �������ڱ������������������ݲ��������޸�
        NodeBarrier barrier( GetHeader()->elementsUnuse );

        Header_32byte &header = *GetHeader();
        header.countUnuse = header.capacity;
        header.elementsUnuse.offset = 0;
        
        memset( hashEntry, 0, sizeof( DWORD ) * HASHKEYCAPACITY );

        LPVOID pointer = ( LPBYTE )memoryAddress + sizeof( Header_32byte ) + sizeof( DWORD ) * HASHKEYCAPACITY;
        for ( int i = 0; i < header.capacity; i ++ )
        {
            LPVOID blockSegment = ( LPBYTE )pointer + ( sizeof( ChainNode ) + header.keyCapacity + header.elementSize ) * i;
            // �������ڱ�֤�����û���û�г���������ݶ��󣬲����ڳ�����Դ���������Insert������ᵼ�������쳣
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
            return FALSE;   // �õ�ַ���ڹ���������Χ��

        if ( ( ( block - segment ) % ( sizeof( ChainNode ) + header.keyCapacity + header.elementSize ) ) != 0 )
            return FALSE;
            // throw exception( "bad block!" );    // �õ�ַ������Ч�����ݿ��ף�

        // �������ڱ������������������ݲ��������޸�
        NodeBarrier barrier1( GetHeader()->elementsUnuse );

        ChainNode *node = ( ChainNode* )( ( LPBYTE )memoryAddress + block );
		if ( ( ( LPBYTE )node->data() )[0] == 0 )
            return FALSE;   // �Ѿ������key���ݵ����ݿ�

        // ͨ������ȡʹ��Ȩ
		NodeBarrier barrier2( *node );

		if ( ( ( LPBYTE )node->data() )[0] == 0 )
            return FALSE;   // �Ѿ������key���ݵ����ݿ�

		size_t len = strlen( ( LPCSTR )node->data() );
		DWORD hash = lh_strhash( ( LPCSTR )node->data(), len ) & ( HASHKEYCAPACITY - 1 );
		
		if ( ( ( ChainNode& )hashEntry[ hash ] ).Remove( memoryAddress, node, &GetHeader()->elementsUnuse ) )
			GetHeader()->countUnuse ++;
		
		// ���¹ؼ���
		memset( ( LPBYTE )node->data(), 0, GetHeader()->keyCapacity );

        return TRUE;
    }

    virtual BOOL Erase( LPCVOID key ) 
    { 
        size_t len = strlen( ( LPCSTR )key );
        if ( len > GetHeader()->keySize )
            throw exception( "key string too long !" );

        // ��ĩβ����\0������ʹ��memcmp���Ƚ�
        size_t minSize = min( len, GetHeader()->keySize ) + 1;
        DWORD hash = lh_strhash( key, len ) & ( HASHKEYCAPACITY - 1 );

        // �������ڱ������������������ݲ��������޸�
        NodeBarrier barrier( GetHeader()->elementsUnuse );

        for ( ChainNode *iter = ( ( ChainNode& )hashEntry[ hash ] ).next( memoryAddress ); 
			iter != NULL; iter = iter->next( memoryAddress ) )
        {
            if ( memcmp( ( LPBYTE )iter->data(), key, minSize ) == 0 )
            {
                NodeBarrier barrier( *iter );
                if ( ( ( ChainNode& )hashEntry[ hash ] ).Remove( memoryAddress, iter, &GetHeader()->elementsUnuse ) )
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

        // �������ڱ������������������ݲ��������޸�
        NodeBarrier barrier( GetHeader()->elementsUnuse );

        for ( ChainNode *iter = ( ( ChainNode& )hashEntry[ hash ] ).next( memoryAddress ); 
			iter != NULL; iter = iter->next( memoryAddress ) )
        {
            if ( memcmp( ( LPBYTE )iter->data(), key, minSize ) == 0 )
            {
                iter->Lock(); // �ڷ�������ǰ�����õ�ǰ�̱߳��ָ���Դ������Ȩ��
                return ( LPBYTE )iter->data() + GetHeader()->keyCapacity;
            }
        }

        // ���������������������б����Ƴ�
        ChainNode *node = GetHeader()->elementsUnuse.Pop( memoryAddress );
        assert( node );
        if ( node == NULL ) 
            return 0;

        GetHeader()->countUnuse --;

        // �ڷ�������ǰ�����õ�ǰ�̱߳��ָ���Դ������Ȩ��
        node->Lock(); 
        ( ( ChainNode& )hashEntry[ hash ] ).Insert( memoryAddress, node );

        // ���¹ؼ���
        memcpy( ( LPBYTE )node->data(), key, minSize );

        return ( LPBYTE )node->data() + GetHeader()->keyCapacity;
    }

    // ����������Ч����
    virtual void Traverse( TCB fn, LPVOID argment ) 
    {
        // �������ڱ������������������ݲ��������޸�
        NodeBarrier barrier( GetHeader()->elementsUnuse );

        // ��������Ч�ʲ����ߣ��ɿ���ʹ��˫�����������Ч��
        // ����Ϊ���������Ӧ���Ǻ����õ��Ĺ��ܣ������ݲ������Ż�����
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

        // ��ĩβ����\0������ʹ��memcmp���Ƚ�
        size_t minSize = min( len, GetHeader()->keySize ) + 1;
        DWORD hash = lh_strhash( key, len ) & ( HASHKEYCAPACITY - 1 );

        // �������ڱ������������������ݲ��������޸�
        NodeBarrier barrier( GetHeader()->elementsUnuse );

        for ( ChainNode *iter = ( ( ChainNode& )hashEntry[ hash ] ).next( memoryAddress ); 
			iter != NULL; iter = iter->next( memoryAddress ) )
        {
            if ( memcmp( ( LPBYTE )iter->data(), key, minSize ) == 0 )
                // ����ֻ��Ϊ�˻�ȡĿ�����ݵĵ�ַ�����Բ���Ҫ������
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
            return FALSE;   // �õ�ַ���ڹ���������Χ��

        if ( ( ( block - segment ) % ( sizeof( ChainNode ) + header.keyCapacity + header.elementSize ) ) != 0 )
            return FALSE;   // �õ�ַ���ڹ���������Χ��
            // throw exception( "bad block!" );    // �õ�ַ������Ч�����ݿ��ף�

        // �������ڱ������������������ݲ��������޸�
        // �������Ҫ�𣿱�������ڴ�ͱض�����Ч���ڵģ�Ψһ���ܳ��ֵĿ��ܾ��Ǹ����ݿ鱻�ͷŻ������߳���ռ
        // �������ͷ�Ҳ�ã���ռҲ�ã���Դ��ַ����Ч��Ӧ���Ǳ��ֲ���ģ�ͨ���Աȹؼ��֣�Ӧ�þͿ���ֱ�ӵõ������
        // NodeBarrier barrier( GetHeader()->elementsUnuse );

        ChainNode *node = ( ChainNode* )( ( LPBYTE )memoryAddress + block );

		if ( ( ( LPBYTE )node->data() )[0] == 0 )
            return FALSE;   // �Ѿ������key���ݵ����ݿ�

        // ͨ������ȡʹ��Ȩ
        node->Lock();

        // �ٴζԱȽ�����������Ƿ���Ȼ��Ч��
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

        // ��ĩβ����\0������ʹ��memcmp���Ƚ�
        size_t minSize = min( len, GetHeader()->keySize ) + 1;

        DWORD segment = sizeof( Header_32byte ) + sizeof( DWORD ) * HASHKEYCAPACITY;
        DWORD offset = segment + ( sizeof( ChainNode ) + header.keyCapacity + header.elementSize ) * ( header.capacity - 1 );

        if ( block < segment || block > offset )
            return FALSE;   // �õ�ַ���ڹ���������Χ��

        if ( ( ( block - segment ) % ( sizeof( ChainNode ) + header.keyCapacity + header.elementSize ) ) != 0 )
            return FALSE;
            // throw exception( "bad block!" );    // �õ�ַ������Ч�����ݿ��ף�

        // �������ڱ������������������ݲ��������޸�
        // �������Ҫ�𣿱�������ڴ�ͱض�����Ч���ڵģ�Ψһ���ܳ��ֵĿ��ܾ��Ǹ����ݿ鱻�ͷŻ������߳���ռ
        // �������ͷ�Ҳ�ã���ռҲ�ã���Դ��ַ����Ч��Ӧ���Ǳ��ֲ���ģ�ͨ���Աȹؼ��֣�Ӧ�þͿ���ֱ�ӵõ������
        // NodeBarrier barrier( GetHeader()->elementsUnuse );

        ChainNode *node = ( ChainNode* )( ( LPBYTE )memoryAddress + block );

        if ( memcmp( ( LPBYTE )node->data(), key, minSize ) != 0 )
            return FALSE;   // ��ͬ����Ȼ�Ͳ���ͨ����

        // ͨ������ȡʹ��Ȩ
        node->Lock();

        // �ٴζԱȽ�����������Ƿ���Ȼ��Ч��
        if ( memcmp( ( LPBYTE )node->data(), key, minSize ) != 0 )
            return node->Unlock(), FALSE;   // ��ͬ��˵����һ�αȽϺ��Ѿ��������̶߳��������������޸ģ��ͷŻ�����ռ��

        return reinterpret_cast< LPBYTE >( node->data() ) + GetHeader()->keyCapacity;
    }

    // ��Ϊ�ڻ�ȡָ����Ԫ��ͬʱ���õ�Ԫ�Ǳ��������ģ�������ʹ����Ϻ���Ҫ�ͷ���Դ��
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
            throw exception( "bad block!" );    // �õ�ַ������Ч�����ݿ��ף�

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
        // ����ʹ�õ��� GetThreadTimes ��ȡ���̴߳�����ʱ�䣡
        // ��ʱ����FILETIME��ʽ����100����Ϊ��λ����1601��1��1�տ�ʼ��ʱ
        // ����1���Ϊ 1,000,000,000 / 100 = 10,000,000��ʹ�� 64bit ���Ա�ʾ�� 1601 + 58494 ��
        // �������������жϵ����ݲ�����Ҫ��ô��ȷ���ܹ��ﵽ���뼶�Ϳ����ˣ����Կ����ٳ���1w
        union {
            struct {
                DWORD idHolder, timestamp; // ��ǰ���ĸ��߳����ڳ�����Դ64bitֵ����32bit�����߳�ID����32bit������̵߳�Ψһ��ʱ�����
            };
            LONGLONG llvalue;
        };

        void Lock() 
        {
            FILETIME ct, ot;    // ��ȡ��ǰ�̵߳Ĵ���ʱ���������ʧ�ܣ�Ҳ�������ʧ��
            while ( !GetThreadTimes( GetCurrentThread(), &ct, &ot, &ot, &ot ) ) { Sleep( 1 ); }

            // ��ʱ�䵥λ�����Ժ���Ϊ��λ�������Ϳ�����32bit�������48������ݱ仯
            LONGLONG lltemp;
            reinterpret_cast< LPDWORD >( &lltemp )[0] = GetCurrentThreadId();
            reinterpret_cast< LPDWORD >( &lltemp )[1] = 
                static_cast< DWORD >( reinterpret_cast< unsigned __int64 & >( ct ) / 10000 );

_fullretry:
            // �ȳ���ֱ����������
            LONGLONG result = InterlockedCompareExchange64( &llvalue, lltemp, 0 );
            if ( result == 0 )
                return;     // ��Ȼ�ɹ��ˣ�ֱ�ӷ��ء�������Ӧ�������ձ�����������Ҳ����ߵģ�

_retry:
            // ������һ�£����ٽ�CPU��Դ�ͷų�����
            Sleep( 0 ); 

            // ���ɹ���˵���������߳������и���Դ���������п���Ŀ���߳��Ѿ�����������Դδ�ͷţ�����Ҫ���״̬
            if ( HANDLE threadHandle = OpenThread( 
                THREAD_QUERY_INFORMATION, FALSE, reinterpret_cast< LPDWORD >( &result  )[0] ) )
            {
                // ����򿪳ɹ�����˵�����̻߳���Ч������Ϊ�߳�ID�ᱻ���ã�������Ҫ��ȡĿ���̵߳�ʱ���������ƥ��
                while ( !GetThreadTimes( threadHandle, &ct, &ot, &ot, &ot ) ) { Sleep( 1 ); }

                CloseHandle( threadHandle );

                if ( reinterpret_cast< LPDWORD >( &result )[1] == 
                    static_cast< DWORD >( reinterpret_cast< unsigned __int64 & >( ct ) / 10000 ) )
                {
                    // ʱ���Ҳƥ�����ˣ�˵��Ŀ���߳�����Ч���ģ���Ҫ�ȴ�������
                    if ( InterlockedCompareExchange64( &llvalue, lltemp, 0 ) == 0 )
                        return;     // �ɹ��ȴ�����������;

                    // ������������п���Ŀ��������������������ж���Ҫ�ظ��ظ����ظ�������
                    goto _retry;
                }
            }

            // ������ʧ�ܣ���˵��Ŀ���߳��Ѿ��������ˣ������Ѿ����ǵ����������Ǹ��̣߳���ǿ����ռ��Դ��
            if ( InterlockedCompareExchange64( &llvalue, lltemp, result ) != result )
                goto _fullretry;    // ��ռҲ��ʧ�ܣ����ܱ��������߳������˰ɣ�ֻ�ܼ����ȴ����ظ��ظ����ظ�������
        }

        void Unlock() 
        {
            FILETIME ct, ot;    // ��ȡ��ǰ�̵߳Ĵ���ʱ���������ʧ�ܣ�Ҳ�������ʧ��
            while ( !GetThreadTimes( GetCurrentThread(), &ct, &ot, &ot, &ot ) ) { Sleep( 1 ); }

            // ��ʱ�䵥λ�����Ժ���Ϊ��λ�������Ϳ�����32bit�������48������ݱ仯
            LONGLONG lltemp;
            reinterpret_cast< LPDWORD >( &lltemp )[0] = GetCurrentThreadId();
            reinterpret_cast< LPDWORD >( &lltemp )[1] = 
                static_cast< DWORD >( reinterpret_cast< unsigned __int64 & >( ct ) / 10000 );

            if ( InterlockedCompareExchange64( &llvalue, 0, lltemp ) != lltemp )
                throw exception( "�ͷ�����Դʱ�����ֳ����߲����Լ���" );
        }
*/
