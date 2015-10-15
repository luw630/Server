#pragma once

#include <list>
#pragma warning ( push )
#pragma warning ( disable : 4800 )
#include <boost/pool/pool_alloc.hpp>
#pragma warning ( pop )

// 事件管理器
// 原理：游戏主逻辑中的事件处理最小间隔单位为帧
// 那么，我们可以确定在某个时刻，某帧的事件均在同一个时刻处理
// 而对于最大值的60*60*EVENTFPS就是一个小时内的事件容器空间
// 如果是超出了一个小时，那这些特殊的事件其实是不会很频繁的，那么用数组保存，逻辑就简单了
// 同时，如果需要做精确时间触发控制，那么首先要保证，服务器的主循环帧间隔要足够小，然后将EVENTFPS相应扩大
class EventMgr {
public:
	static const int EVENTFPS = 10;
	static const int MAXUNITS = 60*60*EVENTFPS;
	static EventMgr &singleton();

	struct EventBase {
		friend class EventMgr;
	protected:
		EventMgr *mgr;	    // 只要这个数据存在，就表示事件处于激活等待状态中，如果这个时候被销毁，则应该触发OnBreak
		DWORD pos;		    // 时间线上的绝对坐标 可以用来计算出他所在的容器和坐标
	public:
		EventBase() : mgr(0), pos(0) {}
		virtual ~EventBase() { /*Reset(); 这个地方的写法实际上是有问题的，析构的时候虚表已经被篡改了，无法由ResetEvent正确调用继承类的OnCancel方法*/ }
		void Reset() { if ( mgr != NULL ) mgr->ResetEvent( this );}
		virtual void SelfDestructor() {};           // 自销毁方法
		virtual void OnCancel( EventMgr *mgr ) {};  // 事件被中断
		virtual void OnActive( EventMgr *mgr ) = 0; // 事件被正常激活
	};

	void Run( DWORD duration );
	void SetEvent( EventBase *e, DWORD remain );
	void ResetEvent( EventBase *e );
	DWORD ServerTime();
	EventMgr();
	~EventMgr();

private:
	// 为了提高性能（频率非常高的插入和删除，但对象又是指针），需要使用自定义的分配器！
    typedef std::list< EventBase*, boost::fast_pool_allocator< EventBase* > > EventList;
	EventList ticks[MAXUNITS]; // 按每秒10帧的频率计算 
	DWORD chkPoint; // 使用ServerTime获取时间来计算的帧数
	DWORD segTick;
};

//事件包装,因为EventBase::ResetEvent是虚函数,不能在EventBase的析构中调用
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