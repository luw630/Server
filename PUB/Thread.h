#pragma once

#include <process.h>
#include <queue>
#include <cassert>
#include <stack>

namespace UGE
{
    namespace Details
    {
        //// 栅栏
        //template< typename _lock >
        //class Barrier
        //{
        //public:
        //    Barrier() : lock( NULL ) {}
        //    Barrier( _lock &other ) : lock( &other ) { lock->Lock(); }
        //    ~Barrier() { if ( lock ) lock->Unlock(); }

        //public:
        //    void Assign( _lock *other ) 
        //    { 
        //        if ( lock ) 
        //            lock->Unlock(); 

        //        lock = other; 
        //        if ( lock ) 
        //            lock->Lock(); 
        //    }

        //private:
        //    _lock *lock;
        //};

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

        template < typename procedure > static UINT __stdcall ThreadEntry_Ref( void *arg )
        {
            procedure *proc = reinterpret_cast< procedure * >( arg );
            int result = (*proc)();
            return result;
        }

    public:
        template < typename procedure > HANDLE BeginThread( procedure &proc )
        {
            procedure *arg = new procedure( proc );
            UINT threadID = 0;

            HANDLE handle = reinterpret_cast< HANDLE >( _beginthreadex( NULL, 0, ThreadEntry< procedure >, arg, 0, &threadID ) );

            Barrier4ReadWrite barrier( *this );
            threads.push( handle );
            return handle;
        }

        // 由于waiter本来就会一直持有线程，直到对方生命期结束，反过来说，在对方生命期结束前，waiter是一直有效的，所以，proc也可以做到这一点！
        template < typename procedure > HANDLE BeginThread_Ref( procedure &proc )
        {
            UINT threadID = 0;
            HANDLE handle = reinterpret_cast< HANDLE >( _beginthreadex( NULL, 0, ThreadEntry_Ref< procedure >, &proc, 0, &threadID ) );
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

        BOOL QuitFlag()
        {
            return quitFlag;
        }

        void Quit()
        {
            quitFlag = true;

	        while ( true )
	        {
				HANDLE thread = INVALID_HANDLE_VALUE;

				{
					// 现在出现了使用 barrier( *this ) 进行自锁的对象如: mtQueue, 
					// 结果在退出时, 主线程持有锁等待工作线程结束, 而工作线程等待锁资源好获取mtQueue数据...
					// 这样写最直接的目的就是消除死锁!
					Barrier4ReadWrite barrier( *this );

					if ( threads.empty() )
						break;

					thread = threads.top();
					threads.pop();
				}

                // 不排除外部会将这个线程冻结，这里尝试将其激活！
                ResumeThread( thread );

		        while ( true )
		        {
			        if ( WaitForSingleObject( thread, 100 ) == WAIT_OBJECT_0 )
				        break;

			        DWORD ret = 0;
			        if ( !GetExitCodeThread( thread, &ret ) )
				        break;

			        if ( STILL_ACTIVE != ret )
				        break;
		        }

                CloseHandle( thread );
	        }
        }

    public:
        ThreadWaiter() : quitFlag( false )
        {
        }

        ~ThreadWaiter()
        { 
            Quit();
        }

    private:
        volatile BOOL quitFlag;
        std::stack< HANDLE > threads;
    };

    // 多线程安全的队列
    template < typename _Ty >
    class mtQueue : 
        private std::queue< _Ty >,
        public ThreadWaiter
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
        public ThreadWaiter
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
}