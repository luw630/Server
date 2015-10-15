#include "Stdafx.h"
#include "EventManager.h"

// 精简版事件管理器，逻辑算是非常简单的啦
EventMgr& EventMgr::singleton() {
	static EventMgr mgr;
	return mgr;
}

EventMgr::EventMgr() : chkPoint( 0 ), segTick( timeGetTime() ) {}
EventMgr::~EventMgr() {}

DWORD EventMgr::ServerTime() { return timeGetTime() - segTick; }

void EventMgr::Run( DWORD duration ) {
// 	DWORD cur = ServerTime(); 
// 	DWORD curPoint = cur / ( 1000/EVENTFPS );
// 	for ( ; chkPoint <= curPoint; chkPoint ++ ) { 
// 		EventList &lst = ticks[ chkPoint % MAXUNITS ];
// 		for ( EventList::iterator it = lst.begin(); it != lst.end(); ) {
// 			EventBase *e = *it;
// 			if ( e->pos > chkPoint ) 
// 				it++;
// 			else {
// 				it = lst.erase( it );
//                 if ( e->mgr == this ) {
// 				    e->mgr = NULL;					// 从事件队列中移除
// 				    e->OnActive( this );			// 触发事件
// 				    e->SelfDestructor();			// 尝试进行自销毁
//                 }
// 			}
// 			if ( duration != INFINITE ) {       // 时间判断逻辑下的超时检测
// 				if ( ( DWORD )abs( ( int )( ServerTime() - cur ) ) > duration )
// 					return;
// 			}
// 		}
// 	}
}

void EventMgr::SetEvent( EventBase *e, DWORD remain ) {
	if (!e)
	{
		rfalse(4,1,"EventMgr::SetEvent( EventBase *e )");
		return;
	}
	e->mgr = this;
	DWORD pos = ( ServerTime() + remain ) / ( 1000 / EVENTFPS );
	if ( chkPoint == pos )
		e->pos = pos + 1;
	else
		e->pos = pos;
	ticks[ e->pos % MAXUNITS ].push_back( e );
}

void EventMgr::ResetEvent( EventBase *e ) {
	if (!e)
	{
		rfalse(4,1,"EventMgr::ResetEvent( EventBase *e )");
		return;
	}
	if ( e->mgr == this ) {
		EventList &lst = ticks[ e->pos % MAXUNITS ];
		for ( EventList::iterator it = lst.begin(); it != lst.end(); it++ ) {
			if ( *it == e ) {
				lst.erase( it );
				e->mgr = NULL;
				e->OnCancel( this );
				e->SelfDestructor();           // 尝试进行自销毁
                break;
			}
		}
	}
}