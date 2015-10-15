#pragma once

#include <list>
#pragma warning ( push )
#pragma warning ( disable : 4800 )
#include <boost/pool/pool_alloc.hpp>
#pragma warning ( pop )

// �¼�������
// ԭ����Ϸ���߼��е��¼�������С�����λΪ֡
// ��ô�����ǿ���ȷ����ĳ��ʱ�̣�ĳ֡���¼�����ͬһ��ʱ�̴���
// ���������ֵ��60*60*EVENTFPS����һ��Сʱ�ڵ��¼������ռ�
// ����ǳ�����һ��Сʱ������Щ������¼���ʵ�ǲ����Ƶ���ģ���ô�����鱣�棬�߼��ͼ���
// ͬʱ�������Ҫ����ȷʱ�䴥�����ƣ���ô����Ҫ��֤������������ѭ��֡���Ҫ�㹻С��Ȼ��EVENTFPS��Ӧ����
class EventMgr {
public:
	static const int EVENTFPS = 10;
	static const int MAXUNITS = 60*60*EVENTFPS;
	static EventMgr &singleton();

	struct EventBase {
		friend class EventMgr;
	protected:
		EventMgr *mgr;	    // ֻҪ������ݴ��ڣ��ͱ�ʾ�¼����ڼ���ȴ�״̬�У�������ʱ�����٣���Ӧ�ô���OnBreak
		DWORD pos;		    // ʱ�����ϵľ������� ������������������ڵ�����������
	public:
		EventBase() : mgr(0), pos(0) {}
		virtual ~EventBase() { /*Reset(); ����ط���д��ʵ������������ģ�������ʱ������Ѿ����۸��ˣ��޷���ResetEvent��ȷ���ü̳����OnCancel����*/ }
		void Reset() { if ( mgr != NULL ) mgr->ResetEvent( this );}
		virtual void SelfDestructor() {};           // �����ٷ���
		virtual void OnCancel( EventMgr *mgr ) {};  // �¼����ж�
		virtual void OnActive( EventMgr *mgr ) = 0; // �¼�����������
	};

	void Run( DWORD duration );
	void SetEvent( EventBase *e, DWORD remain );
	void ResetEvent( EventBase *e );
	DWORD ServerTime();
	EventMgr();
	~EventMgr();

private:
	// Ϊ��������ܣ�Ƶ�ʷǳ��ߵĲ����ɾ��������������ָ�룩����Ҫʹ���Զ���ķ�������
    typedef std::list< EventBase*, boost::fast_pool_allocator< EventBase* > > EventList;
	EventList ticks[MAXUNITS]; // ��ÿ��10֡��Ƶ�ʼ��� 
	DWORD chkPoint; // ʹ��ServerTime��ȡʱ���������֡��
	DWORD segTick;
};

//�¼���װ,��ΪEventBase::ResetEvent���麯��,������EventBase�������е���
template<typename _Ty>
class AutoEvent
{
public:
	template < typename _P >
	AutoEvent( _P p)  _THROW0()
		: _My(p),_Time(0)
	{	// construct from object pointer
	}
	AutoEvent( ) _THROW0()
		:_Time(0)
	{	// construct from object pointer
	}
	AutoEvent( DWORD time ) _THROW0()
		:_Time(time)
	{	// construct from object pointer
	}
	~AutoEvent()
	{	// destroy the object
		_My.Reset( );
	}
	
	void Setup( ){ EventMgr::singleton().SetEvent(&_My,time); }
	void Setup( DWORD time ){ EventMgr::singleton().SetEvent(&_My,time); _My.OnSetup( ); _Time = time; }
	void Destroy( ){ _My.Reset(); }

	void SetTime( DWORD time ) { _Time = time; }

	_Ty* operator->() _THROW0()
	{	// return designated value
		return (&_My);
	}

	_Ty* Get() { return &_My; }
private:
	_Ty _My;	// the wrapped object
	DWORD _Time;
};