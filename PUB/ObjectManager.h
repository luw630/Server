#pragma once

#include <map>
#include "thread.h"

namespace UGE {

template < typename _key, typename _obj, typename _lock = UGE::DummyLock >
class CObjectManager :
    private _lock
{
public:
    typedef std::map< _key, _obj > _map;
    typedef typename _map::value_type _element;

public:
    // 添加和移除对象
    BOOL InsertObject( _obj &obj, const _key &key, bool overwrite = false )
    {
        _lock::Barrier4ReadWrite barrier( *this );

        if ( !overwrite && ( objectMap.find( key ) != objectMap.end() ) )
            return FALSE;

        objectMap[ key ] = obj;

        return TRUE;
    }

    BOOL RemoveObject( const _key &key )
    {
        _lock::Barrier4ReadWrite barrier( *this );

        return ( objectMap.erase( key ) != 0 );
    }

    // 查找一个对象，之所以使用 _obj &obj 是因为：
    // 在多线程环境下，不光是容器需要加锁，而且很可能需要在容器的临界区内将对象也同时加锁或增加引用！
    BOOL LocateObject( const _key &key, _obj &obj )
    {
        _lock::Barrier4Read barrier( *this );

        std::map< _key, _obj >::iterator it = objectMap.find( key );
        if ( it == objectMap.end() )
            return FALSE;

        obj = it->second;

        return TRUE;
    }

    BOOL LocateTest( const _key &key )
    {
        _lock::Barrier4Read barrier( *this );

        return objectMap.find( key ) != objectMap.end();
    }

public:
    template < typename functor >
    void for_each( functor func )
    {
        _lock::Barrier4Read barrier( *this );

        std::for_each( objectMap.begin(), objectMap.end(), func );
    }

    template < typename functor >
    void find_if( functor func )
    {
        _lock::Barrier4Read barrier( *this );

        std::find_if( objectMap.begin(), objectMap.end(), func );
    }

public:
    // 清空该容器的所有IObject子对象
    void Clear() 
    { 
        _lock::Barrier4ReadWrite barrier( *this ); 
        
        objectMap.clear(); 
    } 

    // 确认当前容器中有多少个对象
    size_t Size() 
    { 
        return objectMap.size(); 
    }

protected:
    std::map< _key, _obj > objectMap;      // 容纳对象的容器
};

};