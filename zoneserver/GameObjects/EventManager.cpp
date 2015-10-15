#include "Stdafx.h"
#include "EventManager.h"

// ������¼����������߼����Ƿǳ��򵥵���
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
// 				    e->mgr = NULL;					// ���¼��������Ƴ�
// 				    e->OnActive( this );			// �����¼�
// 				    e->SelfDestructor();			// ���Խ���������
//                 }
// 			}
// 			if ( duration != INFINITE ) {       // ʱ���ж��߼��µĳ�ʱ���
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
				e->SelfDestructor();           // ���Խ���������
                break;
			}
		}
	}
}