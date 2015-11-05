#pragma once

#include <process.h>
#include <cassert>
#include <queue>
#include <stack>
#include <map>

namespace UGE
{
    namespace Details
    {
        // 栅栏
        template< typename _lock, bool _trace = false > class Barrier;

        template< typename _lock > class Barrier < _lock, true >
        {
        public:
			Barrier() : lock( NULL ) { segTime = timeGetTime(); }

            Barrier( _lock &other ) : lock( &other ) 
            { 
                segTime = timeGetTime(); 

                lock->Lock(); 

                if ( timeGetTime() - segTime > 1 )
                {
                    char temp[256];
                    sprintf( temp, "limit wait lock = %d\r\n", timeGetTime() - segTime );
                    OutputDebugString( temp );
                }
            }

            ~Barrier() 
            { 
                if ( lock ) lock->Unlock(); 

                if ( timeGetTime() - segTime > 2 )
                {
                    char temp[256];
                    sprintf( temp, "limit use lock = %d\r\n", timeGetTime() - segTime );
                    OutputDebugString( temp );
                }
            }

		public:
			void Assign( _lock *other ) 
			{ 
				if ( lock ) 
					lock->Unlock(); 
				
				lock = other; 
				if ( lock ) 
					lock->Lock(); 
			}

        private:
            _lock *lock;

            DWORD segTime;
        };

        // 栅栏
        template< typename _lock > class Barrier < _lock, false >
        {
        public:
			Barrier() : lock( NULL ) {}
            Barrier( _lock &other ) : lock( &other ) { lock->Lock(); }

            ~Barrier() { if ( lock ) lock->Unlock(); }

			void Assign( _lock *other ) { if ( lock ) lock->Unlock(); lock = other; if ( lock ) lock->Lock(); }

        private:
            _lock *lock;
        };
    }

    // 空锁
    class DummyLock
    {
    public:
        typedef Details::Barrier< DummyLock > Barrier4Read;
        typedef Details::Barrier< DummyLock > Barrier4ReadWrite;

    public:
        void Lock() {}
        void Unlock() {}
        BOOL NonSelfLock() { return true; }

    public:
        DummyLock() {}
        ~DummyLock() {}
    };

    // 临界锁
    class CriticalLock
    {
    public:
        typedef Details::Barrier< CriticalLock > Barrier4Read;
        typedef Details::Barrier< CriticalLock > Barrier4ReadWrite;

    public:
        void Lock() { EnterCriticalSection( &cs ); }
        void Unlock() { LeaveCriticalSection( &cs ); }

        // 不可重入的加锁
        BOOL NonSelfLock() 
        { 
            if ( cs.OwningThread == GetCurrentThread() ) 
                return FALSE;
            
            EnterCriticalSection( &cs ); 
            return TRUE;
        }

    public:
        CriticalLock() { InitializeCriticalSection( &cs ); }
        ~CriticalLock() { DeleteCriticalSection( &cs ); }

    private:
        CRITICAL_SECTION cs;
    };

    // 读写锁
    // 注意：在同一个线程内！获取了读者锁之后，不能获取读写锁！！！否则死锁！
    // 反之：在获取了读写锁后，可以在内部继续使用读者锁，而不会造成死锁！！！
    class ReadWriteLock :
        private CriticalLock
    {
    public:
        // 只读栅栏
        class Barrier4Read
        {
        public:
            Barrier4Read( ReadWriteLock &lock ) : number( lock.number )
            {
                CriticalLock::Barrier4ReadWrite barrier( lock );
                InterlockedIncrement( &number );
            }

            ~Barrier4Read() { InterlockedDecrement( &number ); }

        private:
            volatile long &number;
        };

        // 读写栅栏
        class Barrier4ReadWrite :
            public CriticalLock::Barrier4ReadWrite
        {
        public:
            Barrier4ReadWrite( ReadWriteLock &lock ) : CriticalLock::Barrier4ReadWrite( lock )
            {
                // 开始自旋，等待读锁释放资源
                while ( lock.number != 0 )
                    Sleep( 0 );
            }
        };

    public:
        ReadWriteLock() : number( 0 ) {}

    private:
        volatile long number; // 读者数量
    };

    // 线程辅助类，创建线程，同时在退出时等待所有线程完全退出
    class ThreadWaiter :
        protected CriticalLock
    {
    private:
        template < typename procedure > static UINT __stdcall ThreadEntry( void *arg )
        {
            procedure *proc = reinterpret_cast< procedure * >( arg );
            int result = (*proc)();
            delete proc;
            return result;
        }

    public:
        template < typename procedure > HANDLE BeginThread( procedure &proc )
        {
            procedure *arg = new procedure( proc );
            UINT threadID = 0;

            ResetEvent( quitEvent );

            HANDLE handle = reinterpret_cast< HANDLE >( _beginthreadex( NULL, 0, ThreadEntry< procedure >, arg, 0, &threadID ) );

            Barrier4ReadWrite barrier( *this );
            threads.push( handle );
            return handle;
        }

        HANDLE LatestThread()
        {
            Barrier4ReadWrite barrier( *this );

            if ( threads.empty() )
                return INVALID_HANDLE_VALUE;

            return threads.top();
        }

        BOOL QuitFlag( DWORD interval = 0 )
        {
			return WaitForSingleObject( quitEvent, interval ) != WAIT_TIMEOUT;
        }

        template < typename _BeforQuit > void Quit( _BeforQuit beforQuit )
        {
            SetEvent( quitEvent );
            beforQuit();
            while ( PopThread() ) Sleep( 0 );
        }

        void Quit()
        {
            SetEvent( quitEvent );
            while ( PopThread() ) Sleep( 0 );
        }

        size_t Size()
        {
            return threads.size();
        }

        size_t PopThreadDirectly()
        {
            Barrier4ReadWrite barrier( *this );
            if ( threads.empty() )
                return 0;

            threads.pop();

            return threads.size();
        }

    private:
        size_t PopThread()
        {
			size_t result = 0;
			HANDLE thread = INVALID_HANDLE_VALUE;

			{
				Barrier4ReadWrite barrier( *this );

				if ( threads.empty() )
					return 0;

				thread = threads.top();
				threads.pop();
				result = threads.size();
			}

            // 不排除外部会将这个线程冻结，这里尝试将其激活！
            ResumeThread( thread );

		    while ( true )
		    {
			    if ( WaitForSingleObject( thread, 100 ) != WAIT_TIMEOUT )
				    break;

			    DWORD ret = 0;
			    if ( !GetExitCodeThread( thread, &ret ) )
				    break;

			    if ( STILL_ACTIVE != ret )
				    break;
		    }

            CloseHandle( thread );

            return result;
        }

    public:
        ThreadWaiter()
        {
			quitEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        }

        ~ThreadWaiter()
        { 
            Quit();

			CloseHandle( quitEvent );
        }

    protected:
        HANDLE quitEvent;
        std::stack< HANDLE > threads;
    };

    // 多线程安全的队列
    template < typename _Ty >
    class mtQueue : 
        private std::queue< _Ty >,
        private CriticalLock
    {
    public:
        typedef std::queue< _Ty > baseQueue;

        using baseQueue::size;
        using baseQueue::empty;

    public:
        ~mtQueue() { Quit(); }

    public:
        void mtsafe_push( const _Ty &_Val )
        {
            Barrier4ReadWrite barrier( *this );

            baseQueue::push( _Val );
        }

        BOOL mtsafe_get_and_pop( _Ty &_Ret )
        {
            Barrier4ReadWrite barrier( *this );

            if ( baseQueue::empty() )
                return FALSE;

            _Ret = baseQueue::front();

            baseQueue::pop();

            return TRUE;
        }
    };

    // 多线程安全，且数据唯一的队列
    template < typename _Ty >
    class mtUniqueQueue : 
        private std::queue< _Ty >,
        private CriticalLock
    {
    public:
        typedef std::queue< _Ty > baseQueue;

        using baseQueue::size;
        using baseQueue::empty;

    public:
        ~mtUniqueQueue() { Quit(); }

    public:
        void mtsafe_push( const _Ty &_Val )
        {
            Barrier4ReadWrite barrier( *this );

            if ( unique[ _Val.GetKey() ] == NULL )
            {
                baseQueue::push( _Val );
                unique[ _Val.GetKey() ] = &baseQueue::back();
            }
            else
            {
                assert( !baseQueue::empty() );
                *unique[ _Val.GetKey() ] = _Val;
            }
        }

        BOOL mtsafe_get_and_pop( _Ty &_Ret )
        {
            Barrier4ReadWrite barrier( *this );

            if ( baseQueue::empty() )
                return FALSE;

            _Ret = baseQueue::front();

            unique[ baseQueue::front().GetKey() ] = NULL;
            baseQueue::pop();

            return TRUE;
        }

    private:
        std::map< typename _Ty::_Tkey, _Ty* > unique;
    };

    // 多线程安全的队列
    template < typename _Ty >
    class mtwQueue : 
        private std::queue< _Ty >,
        public ThreadWaiter
    {
    public:
        typedef std::queue< _Ty > baseQueue;

        using baseQueue::size;
        using baseQueue::empty;
        using ThreadWaiter::QuitFlag;

    public:
        void set_wait_state( BOOL dowait )
        {
            ( dowait ? ResetEvent : SetEvent )( quitEvent );
        }

        void mtsafe_push( const _Ty &_Val )
        {
            Barrier4ReadWrite barrier( *this );

            baseQueue::push( _Val );

			// 如果存在处于休眠期的线程，就尝试将其唤醒
			if ( sleepCount > 0 ) 
				SetEvent( wakeupEvent );
        }

        int mtsafe_get_and_pop( _Ty &result, DWORD interval = INFINITE )
		{
			// 首次进入临界区!
			{
				Barrier4ReadWrite barrier( *this );

				// 在临界区中尝试获取数据，如果成功，则直接返回！
				if ( get_and_pop( result ) )
					return TRUE;

				// 如果失败，则增加count，开始进入休眠期！
				sleepCount++;
			}

			// 进入休眠期，等待唤醒（需要脱离临界区，避免长时间持有资源！）
			HANDLE events[2] = { quitEvent, wakeupEvent };
			DWORD waitResult = WaitForMultipleObjects( 2, events, FALSE, interval );

			// 被唤醒，再次进入临界区
			{
				Barrier4ReadWrite barrier( *this );

				// 首先处理count，减少处于等待状态的线程计数！
				sleepCount--;

				// 对quitEvent进行特殊处理（退出时不再处理任何可能存在的资源！）
				if ( waitResult == WAIT_OBJECT_0 ) 
					return -1;

				// 不管成功也好，超时也好，反正都进入临界区了，再来取一次数据！
				return get_and_pop( result );
			}
        }

        template < typename _Procedure >
        int mtsafe_process_and_pop( _Procedure procedure, DWORD interval = INFINITE )
		{
			// 首次进入临界区!
			{
				Barrier4ReadWrite barrier( *this );

				// 在临界区中尝试获取数据，如果成功，则直接返回！
				if ( process_and_pop( procedure ) )
					return TRUE;

				// 如果失败，则增加count，开始进入休眠期！
				sleepCount++;
			}

			// 进入休眠期，等待唤醒（需要脱离临界区，避免长时间持有资源！）
			HANDLE events[2] = { quitEvent, wakeupEvent };
			DWORD waitResult = WaitForMultipleObjects( 2, events, FALSE, interval );

			// 被唤醒，再次进入临界区
			{
				Barrier4ReadWrite barrier( *this );

				// 首先处理count，减少处于等待状态的线程计数！
				sleepCount--;

				// 对quitEvent进行特殊处理（退出时不再处理任何可能存在的资源！）
				if ( waitResult == WAIT_OBJECT_0 ) 
					return -1;

				// 不管成功也好，超时也好，反正都进入临界区了，再来取一次数据！
				return process_and_pop( procedure );
			}
        }

	public:
		mtwQueue() : sleepCount( 0 ) { wakeupEvent = CreateEvent( NULL, FALSE, FALSE, NULL ); }
		~mtwQueue() { Quit(); CloseHandle( wakeupEvent ); }

    private:
		inline BOOL get_and_pop( _Ty &result )
		{
			if ( baseQueue::empty() )
				return FALSE;

			result = baseQueue::front();
			baseQueue::pop();
			return TRUE;
		}

        template < typename _Procedure >
		inline BOOL process_and_pop( _Procedure procedure )
		{
			if ( baseQueue::empty() )
				return FALSE;

            while ( !baseQueue::empty() )
            {
			    int result = procedure( baseQueue::front() );

                if ( result >= 0 )
                    baseQueue::pop();

                if ( result <= 0 )
                    break;
            }

			return TRUE;
		}

	private:
		HANDLE  wakeupEvent;
		volatile LONG sleepCount;
    };

    // 多线程安全，且数据唯一的队列
    template < typename _Ty >
    class mtwUniqueQueue : 
        private std::queue< _Ty >,
        public ThreadWaiter
    {
    public:
        typedef std::queue< _Ty > baseQueue;

        using baseQueue::size;
        using baseQueue::empty;
        using ThreadWaiter::QuitFlag;

    public:
        void set_wait_state( BOOL dowait )
        {
            ( dowait ? ResetEvent : SetEvent )( quitEvent );
        }

        void mtsafe_push( const _Ty &_Val )
        {
            Barrier4ReadWrite barrier( *this );

            if ( unique[ _Val.GetKey() ] == NULL )
            {
                baseQueue::push( _Val );
                unique[ _Val.GetKey() ] = &baseQueue::back();
            }
            else
            {
                assert( !baseQueue::empty() );
                *unique[ _Val.GetKey() ] = _Val;
            }
 
			// 如果存在处于休眠期的线程，就尝试将其唤醒
			if ( sleepCount > 0 ) 
				SetEvent( wakeupEvent );
		}

        int mtsafe_get_and_pop( _Ty &result, DWORD interval = INFINITE )
		{
			// 首次进入临界区!
			{
				Barrier4ReadWrite barrier( *this );

				// 在临界区中尝试获取数据，如果成功，则直接返回！
				if ( get_and_pop( result ) )
					return TRUE;

				// 如果失败，则增加count，开始进入休眠期！
				sleepCount++;
			}

			// 进入休眠期，等待唤醒（需要脱离临界区，避免长时间持有资源！）
			HANDLE events[2] = { quitEvent, wakeupEvent };
			DWORD waitResult = WaitForMultipleObjects( 2, events, FALSE, interval );

			// 被唤醒，再次进入临界区
			{
				Barrier4ReadWrite barrier( *this );

				// 首先处理count，减少处于等待状态的线程计数！
				sleepCount--;

				// 对quitEvent进行特殊处理（退出时不再处理任何可能存在的资源！）
				if ( waitResult == WAIT_OBJECT_0 ) 
					return -1;

				// 不管成功也好，超时也好，反正都进入临界区了，再来取一次数据！
				return get_and_pop( result );
			}
        }

        template < typename _Procedure >
        int mtsafe_process_and_pop( _Procedure procedure, DWORD interval = INFINITE )
		{
			// 首次进入临界区!
			{
				Barrier4ReadWrite barrier( *this );

				// 在临界区中尝试获取数据，如果成功，则直接返回！
				if ( process_and_pop( procedure ) )
					return TRUE;

				// 如果失败，则增加count，开始进入休眠期！
				sleepCount++;
			}

			// 进入休眠期，等待唤醒（需要脱离临界区，避免长时间持有资源！）
			HANDLE events[2] = { quitEvent, wakeupEvent };
			DWORD waitResult = WaitForMultipleObjects( 2, events, FALSE, interval );

			// 被唤醒，再次进入临界区
			{
				Barrier4ReadWrite barrier( *this );

				// 首先处理count，减少处于等待状态的线程计数！
				sleepCount--;

				// 对quitEvent进行特殊处理（退出时不再处理任何可能存在的资源！）
				if ( waitResult == WAIT_OBJECT_0 ) 
					return -1;

				// 不管成功也好，超时也好，反正都进入临界区了，再来取一次数据！
				return process_and_pop( procedure );
			}
        }

	public:
		mtwUniqueQueue() : sleepCount( 0 ) { wakeupEvent = CreateEvent( NULL, FALSE, FALSE, NULL ); }
		~mtwUniqueQueue() { Quit(); CloseHandle( wakeupEvent ); }

    private:
 		inline BOOL get_and_pop( _Ty &result )
		{
			if ( baseQueue::empty() )
				return FALSE;

			result = baseQueue::front();
            unique[ baseQueue::front().GetKey() ] = NULL;
			baseQueue::pop();
			return TRUE;
		}

        template < typename _Procedure >
		inline BOOL process_and_pop( _Procedure procedure )
		{
			if ( baseQueue::empty() )
				return FALSE;

            while ( !baseQueue::empty() )
            {
			    int result = procedure( baseQueue::front() );

                if ( result >= 0 )
                {
                    unique[ baseQueue::front().GetKey() ] = NULL;
                    baseQueue::pop();
                }

                if ( result <= 0 )
                    break;
            }

			return TRUE;
		}

	private:
		HANDLE  wakeupEvent;
		volatile LONG sleepCount;
        std::map< typename _Ty::_Tkey, _Ty* > unique;
    };
}