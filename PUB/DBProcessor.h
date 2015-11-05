#pragma once

#include "boost/bind.hpp"
#include "pub/thread.h"
//#include "/QueryClass.h"

template < typename element_type, typename procedure_type, template < typename > class queue_type >
bool StartDBThread( queue_type< element_type > &queue, procedure_type procedure, 
    LPCSTR sDBAccount, LPCSTR sDBPassword, LPCSTR sDBIP, LPCSTR sDBDefault, LPCSTR who )
{
    typedef mtDBProcessor< element_type, procedure_type, queue_type > MTDBPROCESSOR;

    MTDBPROCESSOR *obj = new MTDBPROCESSOR( queue, procedure, who );

    if ( !obj->Init( sDBAccount, sDBPassword, sDBIP, sDBDefault ) )
    {
        delete obj;
        return false;
    }

    queue.BeginThread( boost::bind( &MTDBPROCESSOR::Routine, obj ) );

    return true;
}

template < typename element_type, typename procedure_type, template < typename > class queue_type >
class mtDBProcessor : public Query
{
    typedef queue_type< element_type > MTQueue;

public:
    int Routine()
    {
        bool isempty = false;

        while ( !mtQueue.QuitFlag() || ( !isempty && noLeak ) )
        {
            element_type element;

            if ( !mtQueue.mtsafe_get_and_pop( element ) )
            {
                isempty = true;
                Sleep( 1 );
                continue;
            }

            if ( !KeepAlive( 0, who.c_str() ) )
                continue;

            isempty = false;

            // 执行定制的元素处理过程
            procedure( *static_cast< Query* >( this ), element );
        }

        UnInit();

        delete this;

        return 0;
    }

    size_t size()
    {
        return mtQueue.size();
    }

public:
    mtDBProcessor( MTQueue &p, procedure_type procedure, LPCSTR who ) : 
      Query(), mtQueue(p), procedure(procedure), who( who ), noLeak(true) {}

private:
    MTQueue         &mtQueue;   // 请求的操作队列
    procedure_type  procedure;  // 元素处理过程
    bool            noLeak;     // 是否在所有元素处理完毕后才退出
    std::string     who;
};
