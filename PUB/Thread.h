#pragma once

#include <process.h>
#include <queue>
#include <cassert>
#include <stack>

namespace UGE
{
    namespace Details
    {
        //// դ��
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

        // դ��
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

        // դ��
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

    // ����
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

    // �ٽ���
    class CriticalLock
    {
    public:
        typedef Details::Barrier< CriticalLock > Barrier4Read;
        typedef Details::Barrier< CriticalLock > Barrier4ReadWrite;

    public:
        void Lock() { EnterCriticalSection( &cs ); }
        void Unlock() { LeaveCriticalSection( &cs ); }

        // ��������ļ���
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

    // ��д��
    // ע�⣺��ͬһ���߳��ڣ���ȡ�˶�����֮�󣬲��ܻ�ȡ��д������������������
    // ��֮���ڻ�ȡ�˶�д���󣬿������ڲ�����ʹ�ö��������������������������
    class ReadWriteLock :
        private CriticalLock
    {
    public:
        // ֻ��դ��
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

        // ��дդ��
        class Barrier4ReadWrite :
            public CriticalLock::Barrier4ReadWrite
        {
        public:
            Barrier4ReadWrite( ReadWriteLock &lock ) : CriticalLock::Barrier4ReadWrite( lock )
            {
                // ��ʼ�������ȴ������ͷ���Դ
                while ( lock.number != 0 )
                    Sleep( 0 );
            }
        };

    public:
        ReadWriteLock() : number( 0 ) {}

    private:
        volatile long number; // ��������
    };

    // �̸߳����࣬�����̣߳�ͬʱ���˳�ʱ�ȴ������߳���ȫ�˳�
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

        // ����waiter�����ͻ�һֱ�����̣߳�ֱ���Է������ڽ�����������˵���ڶԷ������ڽ���ǰ��waiter��һֱ��Ч�ģ����ԣ�procҲ����������һ�㣡
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
					// ���ڳ�����ʹ�� barrier( *this ) ���������Ķ�����: mtQueue, 
					// ������˳�ʱ, ���̳߳������ȴ������߳̽���, �������̵߳ȴ�����Դ�û�ȡmtQueue����...
					// ����д��ֱ�ӵ�Ŀ�ľ�����������!
					Barrier4ReadWrite barrier( *this );

					if ( threads.empty() )
						break;

					thread = threads.top();
					threads.pop();
				}

                // ���ų��ⲿ�Ὣ����̶߳��ᣬ���ﳢ�Խ��伤�
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

    // ���̰߳�ȫ�Ķ���
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

    // ���̰߳�ȫ��������Ψһ�Ķ���
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