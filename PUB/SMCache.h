#pragma once

#include <unknwn.h>

// �������ܽӿ�
interface SharedMemoryCache
{
    typedef BOOL ( CALLBACK *TCB )( LPVOID key, LPVOID element, LPVOID argment );

    // �����趨ֵ��ʼ������ȡָ��������������¿��Ļ�����������һ���ػ��������ڱ��ֹ����ڴ����
    // uniqueName = Ψһ���ƣ������ж�ָ�������ڴ��Ƿ���ڣ�
    // elementSize = ���浥Ԫ, count = Ԥ���Ļ��浥Ԫ������
    // keySize = ÿ���ؼ��ֵĿռ��С�����ڱ�������Ĭ������£�Ϊ���30�ֽڵ��ַ�����������\0����
    static SharedMemoryCache& Singleton( LPCSTR uniqueName, WORD capacity, DWORD elementSize, BYTE keySize = 30 );

    // �����������ռ䣬�ػ����̽����˳���Singleton������Ȼ��Ч
    // ������������øú���������㵱ǰ���������˳��رգ����������Ȼ������Ч��
    virtual void Release() = 0;

    // ���ػ���ռ��С���Ե�Ԫ�ƣ���Size������Ч��Ԫ������Capacity�����ܵ�Ԫ����
    virtual DWORD Size() = 0;
    virtual DWORD Capacity() = 0;

    // �����жϸù��������´��������Ѿ���Ч���ڵ�
    virtual BOOL isNewly() = 0;

    // �����е�Ԫ����������û���
    virtual void Clear() = 0;

    // ɾ��һ���Ѿ����ڵĵ�Ԫ
    virtual BOOL Erase( LPCVOID key ) = 0;

    // ��ȡָ����Ԫ��һ������£�keyΪ�ַ���ָ�룬��������£���keySize<=4����keyΪ32bit����ֵ����
    virtual LPVOID operator [] ( LPCVOID key ) = 0;

    // ����������Ч����
    virtual void Traverse( TCB fn, LPVOID argment ) = 0;
};

// ��װ���Ӧ��ģ����
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
