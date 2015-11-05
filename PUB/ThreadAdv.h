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

            // ���ų��ⲿ�Ὣ����̶߳��ᣬ���ﳢ�Խ��伤�
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

    // ���̰߳�ȫ�Ķ���
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

    // ���̰߳�ȫ��������Ψһ�Ķ���
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

    // ���̰߳�ȫ�Ķ���
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

			// ������ڴ��������ڵ��̣߳��ͳ��Խ��份��
			if ( sleepCount > 0 ) 
				SetEvent( wakeupEvent );
        }

        int mtsafe_get_and_pop( _Ty &result, DWORD interval = INFINITE )
		{
			// �״ν����ٽ���!
			{
				Barrier4ReadWrite barrier( *this );

				// ���ٽ����г��Ի�ȡ���ݣ�����ɹ�����ֱ�ӷ��أ�
				if ( get_and_pop( result ) )
					return TRUE;

				// ���ʧ�ܣ�������count����ʼ���������ڣ�
				sleepCount++;
			}

			// ���������ڣ��ȴ����ѣ���Ҫ�����ٽ��������ⳤʱ�������Դ����
			HANDLE events[2] = { quitEvent, wakeupEvent };
			DWORD waitResult = WaitForMultipleObjects( 2, events, FALSE, interval );

			// �����ѣ��ٴν����ٽ���
			{
				Barrier4ReadWrite barrier( *this );

				// ���ȴ���count�����ٴ��ڵȴ�״̬���̼߳�����
				sleepCount--;

				// ��quitEvent�������⴦���˳�ʱ���ٴ����κο��ܴ��ڵ���Դ����
				if ( waitResult == WAIT_OBJECT_0 ) 
					return -1;

				// ���ܳɹ�Ҳ�ã���ʱҲ�ã������������ٽ����ˣ�����ȡһ�����ݣ�
				return get_and_pop( result );
			}
        }

        template < typename _Procedure >
        int mtsafe_process_and_pop( _Procedure procedure, DWORD interval = INFINITE )
		{
			// �״ν����ٽ���!
			{
				Barrier4ReadWrite barrier( *this );

				// ���ٽ����г��Ի�ȡ���ݣ�����ɹ�����ֱ�ӷ��أ�
				if ( process_and_pop( procedure ) )
					return TRUE;

				// ���ʧ�ܣ�������count����ʼ���������ڣ�
				sleepCount++;
			}

			// ���������ڣ��ȴ����ѣ���Ҫ�����ٽ��������ⳤʱ�������Դ����
			HANDLE events[2] = { quitEvent, wakeupEvent };
			DWORD waitResult = WaitForMultipleObjects( 2, events, FALSE, interval );

			// �����ѣ��ٴν����ٽ���
			{
				Barrier4ReadWrite barrier( *this );

				// ���ȴ���count�����ٴ��ڵȴ�״̬���̼߳�����
				sleepCount--;

				// ��quitEvent�������⴦���˳�ʱ���ٴ����κο��ܴ��ڵ���Դ����
				if ( waitResult == WAIT_OBJECT_0 ) 
					return -1;

				// ���ܳɹ�Ҳ�ã���ʱҲ�ã������������ٽ����ˣ�����ȡһ�����ݣ�
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

    // ���̰߳�ȫ��������Ψһ�Ķ���
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
 
			// ������ڴ��������ڵ��̣߳��ͳ��Խ��份��
			if ( sleepCount > 0 ) 
				SetEvent( wakeupEvent );
		}

        int mtsafe_get_and_pop( _Ty &result, DWORD interval = INFINITE )
		{
			// �״ν����ٽ���!
			{
				Barrier4ReadWrite barrier( *this );

				// ���ٽ����г��Ի�ȡ���ݣ�����ɹ�����ֱ�ӷ��أ�
				if ( get_and_pop( result ) )
					return TRUE;

				// ���ʧ�ܣ�������count����ʼ���������ڣ�
				sleepCount++;
			}

			// ���������ڣ��ȴ����ѣ���Ҫ�����ٽ��������ⳤʱ�������Դ����
			HANDLE events[2] = { quitEvent, wakeupEvent };
			DWORD waitResult = WaitForMultipleObjects( 2, events, FALSE, interval );

			// �����ѣ��ٴν����ٽ���
			{
				Barrier4ReadWrite barrier( *this );

				// ���ȴ���count�����ٴ��ڵȴ�״̬���̼߳�����
				sleepCount--;

				// ��quitEvent�������⴦���˳�ʱ���ٴ����κο��ܴ��ڵ���Դ����
				if ( waitResult == WAIT_OBJECT_0 ) 
					return -1;

				// ���ܳɹ�Ҳ�ã���ʱҲ�ã������������ٽ����ˣ�����ȡһ�����ݣ�
				return get_and_pop( result );
			}
        }

        template < typename _Procedure >
        int mtsafe_process_and_pop( _Procedure procedure, DWORD interval = INFINITE )
		{
			// �״ν����ٽ���!
			{
				Barrier4ReadWrite barrier( *this );

				// ���ٽ����г��Ի�ȡ���ݣ�����ɹ�����ֱ�ӷ��أ�
				if ( process_and_pop( procedure ) )
					return TRUE;

				// ���ʧ�ܣ�������count����ʼ���������ڣ�
				sleepCount++;
			}

			// ���������ڣ��ȴ����ѣ���Ҫ�����ٽ��������ⳤʱ�������Դ����
			HANDLE events[2] = { quitEvent, wakeupEvent };
			DWORD waitResult = WaitForMultipleObjects( 2, events, FALSE, interval );

			// �����ѣ��ٴν����ٽ���
			{
				Barrier4ReadWrite barrier( *this );

				// ���ȴ���count�����ٴ��ڵȴ�״̬���̼߳�����
				sleepCount--;

				// ��quitEvent�������⴦���˳�ʱ���ٴ����κο��ܴ��ڵ���Դ����
				if ( waitResult == WAIT_OBJECT_0 ) 
					return -1;

				// ���ܳɹ�Ҳ�ã���ʱҲ�ã������������ٽ����ˣ�����ȡһ�����ݣ�
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