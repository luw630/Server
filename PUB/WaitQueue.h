/*************************************************
Copyright (C), 2003-2005, U-Soft Co., Ltd.

File name:      WaitObject.h
Author:         Joe Xu
Version:        1.0.0.0
Date:           2005-6-20

Description:
一个用于对象等待操作的模版，当对象设定的时间到达后，可以通过TraverseAll操作来使所有时间
到达的对象进行操作回调！这多用在进行AI响应回调的部分！

Attation:
虽然可以使用SetTimer等来代替，但是对于游戏，高效是必需的。。。

如果使用旧的调度方式，遍历，则每一个时间单位都需要遍历所有对象，以判断目标对象是否达到等
待的时间界限，从而判断这个对象是否会出现掉用！但是对于游戏来说，里边的对象在同一时刻会出
现大量AI调用的机会非常少，甚至基本没有，而绝大部分时间都是在等待时间到达，所以在这种情况
下，遍历所带来的资源浪费非常之大！

假设一个服务器中有10万个对象，每个对象平均4秒才会出现一次随机行走的AI，所以平均1秒会有2.5
万个对象出现AI响应，但是如果使用遍历来处理，则在每一帧都需要进行AI判断，而一般1秒有20帧，
所以，每4秒会进行[4 * 20 * 10万]800万次判断，并且在做这些判断前，每一个对象一般还附带虚函
数调用，读写内存等额外的代价，所以带来的浪费是惊人的！

这里的这个实现在release下对于引用指针的管理可以达到每秒120万次调用，对于一般指针的管理可
以达到每秒300万次调用，基本上达到普通遍历map管理器一样的效果，同时也可以起到等待时间到达
后才会出现调用，节省了极大的CPU资源！

PS：
现在在多线程下尝试，发现如果使用临界区保证不出错的状况下，1秒钟约可以达到100万次[裸指针]


Template class list:
TAutoAllocator  用于进行快速分配的模版，实现了分配、构造、析构、销毁这几个操作
TWaitQueue      用于对象等待操作的模版

History:
*************************************************/

#pragma once

#include "mmsystem.h"
#include "assert.h"
#include "new.h"

namespace UGE
{
    // 空锁
    class DummyLock
    {
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

    // 栅栏
    template< typename _lock >
    class Barrier
    {
    public:
        Barrier( _lock &other ) : lock( other ) { lock.Lock(); }
        ~Barrier() { lock.Unlock(); }

    private:
        _lock &lock;
    };

	// 一个固定空间分配器，专用于固定大小的空间分配
	// _TObject  ：指定分配单元空间的固定大小
	// _growStep ：指定当空余单元不足时，数据块的成长大小
    // _lock     ：用于多线程的时候进行临界区操作
	// 关于void **的使用说明：在这里是将他做为了链表指针来使用
	// 所以分配的时候都有考虑额外的sizeof(void *)空间的！
	template < typename _TObject, unsigned long _growStep, typename _lock >
    class BlockAllocator : public _lock
	{
	public:
		_TObject* Construct()
		{
			// 显示指定在目标空间上进行构造
			return new ( Alloc() ) _TObject;
		}

		template < typename _TParam >
			_TObject* Construct( _TParam &param )
		{
			return new ( Alloc() ) _TObject( param );
		}

		void Destory( _TObject *pObj )
		{
			// 显示指定对象的析构
            ( pObj )->~_TObject();
			Free( pObj );
		}

	private:
		void *Alloc()
		{
            // 将当前函数设置栅栏，临界区
            Barrier< _lock >( *this );

			volatile void **ppTemp = NULL;
			if ( m_ppJunk == NULL )
			{
				// 如果没有空余的分配空间，就使用系统的new分配一个指定长度的空间
				// 空间的第一个位置是void*，用于将所有已分配的空间串起来
				// m_ppHeader就是专门用于记录已分配空间的链表首节点
				ppTemp = ( volatile void ** ) new char [ sizeof( void * ) + 
                    ( sizeof( void * ) + sizeof( _TObject ) ) * _growStep ];
				*ppTemp = m_ppHeader;
				m_ppHeader = ppTemp;

				// 跳过第一个位置（已分配空间链表节点）
				ppTemp ++;

				// 将分配空间中的每一个块串起来放到未使用单元块的链表中
				// 同样的每个单元块的第一个位置也是void*，用于将所有未使用单元块串起来
				for ( int i=0; i<_growStep; i++ )
				{
					*( void ** )&( ( char * )ppTemp )[ i * ( sizeof( void * ) + sizeof( _TObject ) ) ] = m_ppJunk;
					m_ppJunk = ( volatile void ** )&( ( char * )ppTemp )[ i * ( sizeof( void * ) + sizeof( _TObject ) ) ];
				}
			}

			ppTemp = m_ppJunk;
			_TObject *pRet = (_TObject *)(ppTemp + 1);
			m_ppJunk = (volatile void **)*m_ppJunk;
			*ppTemp = NULL;

			number ++;

			return pRet;
		}

		void Free( void *pPtr )
		{
            // 将当前函数设置栅栏，临界区
            Barrier< _lock > barrier( *this );

			// 回收单元块，但不释放，等待下次使用
			volatile void **ppTemp = ( volatile void ** )pPtr;
			*( ppTemp - 1 ) =  m_ppJunk;
			m_ppJunk = ( ppTemp - 1 );

			number --;
		}

	public:
		BlockAllocator()
		{
			m_ppHeader = NULL;
			m_ppJunk = NULL;
		}

		~BlockAllocator()
		{
            Barrier< _lock > barrier( *this );

			m_ppJunk = NULL;

			// 析构时释放所有已经分配的空间
			// 但是前提是必须保证所有单元块未在使用！
			while ( m_ppHeader != NULL )
			{
				volatile void *pTemp = m_ppHeader;
				m_ppHeader = ( volatile void ** )*m_ppHeader;
				delete pTemp;
			}
		}

	private:
		volatile void **m_ppHeader;     // 用于保存分配空间起始地址的链表
		volatile void **m_ppJunk;       // 用于保存未使用单元块的链表

	public:
		volatile DWORD number;			// 用于保存未使用单元块的链表
	};

	// 对象等待操作的模版
	// _TObject     是用于等待的对象，可以是任何东西
	// _maxLength   因为这是一个按单位时间进行操作序的管理器，这个参数是用来指定最长会需要等待的单位时间
    // _lock        临界锁对象类型，用于在多线程状态下工作，不过默认状况下使用不支持多线程的空锁
	template < typename _TObject, unsigned long _maxLength, typename _lock = DummyLock >
	class WaitQueue
	{
	public:
		struct WaitObject;

		// 用于描述事件的句柄
		class EventHandle
		{
		public:
			EventHandle() : tick( 0 ), handle( NULL ) {}

		private:
			void Clear() { *reinterpret_cast< __int64 * >( this ) = 0; }
			void Assign( DWORD tick, WaitObject *handle ) { this->tick = tick; this->handle = handle; }
			bool isValid() { return handle != NULL; }

		private:
			DWORD tick;			// 只是用于记录位置，不直接操作针指向的空间
			WaitObject *handle; // 事件的句柄地址，用于比较和查找，但不用于直接操作

			friend class WaitQueue;
		};

	private:
		// 等待节点，要求可以将自己从双向链表中删除（以实现某个对象需要取消自己以前所做的等待）
		struct WaitNode
		{
			WaitNode() : m_pPrev( NULL ), m_pNext( NULL ) {}

		    WaitNode *&GetPrev() { return m_pPrev; }

		    WaitNode *&GetNext() { return m_pNext; }

		    void InsertBack( WaitNode *pNode )
		    {
			    if ( pNode == NULL )
				    return;

			    pNode->GetPrev() = this;
			    pNode->GetNext() = m_pNext;

			    if ( m_pNext != NULL )
				    m_pNext->GetPrev() = pNode;

			    m_pNext = pNode;
		    }

		    void CutLink()
		    {
			    if ( m_pPrev != NULL )
				    m_pPrev->m_pNext = m_pNext;

			    if ( m_pNext != NULL )
				    m_pNext->m_pPrev = m_pPrev;

			    m_pNext = m_pPrev = NULL;
		    }

		private:
			WaitNode *m_pNext;
			WaitNode *m_pPrev;
		};

		// 具体的等待对象节点，用于绑定需要响应的对象！
		class WaitObject :
			public WaitNode
		{
		    WaitObject( _TObject &o ) : m_Object( o ) {}

			~WaitObject() { CutLink(); }

		private:
			// 等待激活的对象
			_TObject m_Object;
            void *m_uData;

		private:
			EventHandle *reference;

			friend class WaitQueue;
			friend class BlockAllocator< WaitObject, _maxLength / 10, _lock >;
		};

		// 等待链表的首部，因为按时间进行每个单位的指定，需要准备一个大数组
		// 如果这个数组的元素很大，那么将占用更多的空间，这个是一个折中的方案！
		struct WaitHeader
		{
			bool isEmpty() { return m_pNext == NULL; }

			// 这一步有一个需要注意的地方！
			// 因为要实现节点的自我移出需要双向节点，但是为了减小空间，这里将自己
			// 作为了双向链表的首部！因为InsertBack不会操作到自己的m_pPrev，所以不会出现错误
			void PushFront(WaitObject *pObject)
			{
				( reinterpret_cast< WaitNode * >( this ) )->InsertBack( pObject );
			}

			WaitObject *GetFront() { return m_pNext; }

			WaitHeader() { m_pNext = NULL; }

		private:
			WaitObject *m_pNext;
		};

	public:
		// 遍历所有已经超时的对象
		// 参照了for_each操作，使用函数对象来进行回调！
		template < typename _TOperation >
			void Traverse( DWORD timeMargin, _TOperation op )
		{
            if ( timeMargin == 0 )
                return;

            m_Allocator.Lock();

            __try
            {
                m_InTraverse = true;
            }
            __finally
            {
                m_Allocator.Unlock();
            }

			assert( timeMargin < _maxLength );
			assert( m_CheckCursor < _maxLength );

            if ( timeMargin >= _maxLength )
                timeMargin = _maxLength - 1;

            WaitHeader *it = &m_Indexs[ m_CheckCursor ];
            WaitHeader *end = &m_Indexs[ m_CheckCursor + timeMargin ];

            // 上边的 end 取地址可能越界！在这里将其纠正
            if ( ( m_CheckCursor + timeMargin ) >= _maxLength )
                end = &m_Indexs[ ( m_CheckCursor + timeMargin ) - _maxLength ];

            // 循环的处理所有时间已经超时的对象
            while ( it != end )
            {
				//if ( it == &m_Indexs[0] )
				//{
				//	char temp[123];
				//	sprintf( temp, "traverse [%d] [%d] [%d]\r\n", m_Allocator.number, !it->isEmpty(), end - it );
				//	OutputDebugString( temp );
				//}

				// 如果已经有数据存在就设定ret和退出循环
				while ( true )
				{
					_TObject tempObj;
					void *tempData = NULL;

                    m_Allocator.Lock();

                    __try
                    {
						if ( it->isEmpty() )
							break;

					    WaitObject *p = it->GetFront();

					    // 将这一步放到这里是因为在回调过程中可能会出现修改链表
					    // 如果链表在m_Indexs[m_CheckCursor] = p->LinkPtr();这之前被修改的话
					    // 调用链就可能会断掉。。。
					    tempObj = p->m_Object;
					    tempData = p->m_uData;

						if ( p->reference != NULL )
						{
							assert( p->reference->handle == p );

							// 同时在临界区中，将引用的句柄清除
							// ￥￥￥￥￥￥￥￥￥￥￥￥￥￥￥￥
							// 这里涉及到一个指针对象的有效性问题：
							// reference是外部保持有效性的一个空间，如果外部将该空间释放……
							// 分析：
							// 其实这里也是 p->m_Object、p->m_uData 同样会遇到的状况
							// 所以需要用户自己细心的处理！
							// 最简单的解决方法其实是：m_uData、reference都是m_Object中的数据
							// 这样就只需要保证m_Object有效就可以了
							p->reference->Clear();
						}

					    m_Allocator.Destory( p );
                    }
                    __finally
                    {
                        m_Allocator.Unlock();
                    }

                    // 触发事件处理，这里就不加锁了，容错部分也交给事件处理器去做
					op( tempObj, tempData );
				}

                it ++;
				m_CheckCursor ++;

                if ( it != &m_Indexs[_maxLength] )
                    continue;

                // 当 it == &m_Indexs[_maxLength] 时，已经走了一圈了，要重头处理
                it = m_Indexs;
                m_CheckCursor = 0;
            }

            m_Allocator.Lock();

            __try
            {
                m_InTraverse = false;
            }
            __finally
            {
                m_Allocator.Unlock();
            }
		}

		// 向等待队列中压入一个对象
		BOOL PushObject( unsigned long waitTick, _TObject &obj, void *uData = NULL, EventHandle &reference = *reinterpret_cast< EventHandle * >( NULL ) )
		{
            m_Allocator.Lock();

            __try
            {
                assert( m_CheckCursor < _maxLength );
                assert( waitTick != 0 );

			    // 如果超出最大所能容纳的等待时间，压入失败
			    if ( ( waitTick >= _maxLength ) || ( waitTick == 0 ) )
				    return FALSE;

			    unsigned long indexTick = m_CheckCursor + waitTick;

			    if ( indexTick >= _maxLength )
				    indexTick -= _maxLength;

				// 创建等待事件对象，压入等待队列
			    WaitObject *temp = m_Allocator.Construct( obj );
                temp->m_uData = uData;
				temp->reference = &reference;
			    m_Indexs[ indexTick ].PushFront( temp );

				// 在压入之后，判断reference的状态，如果还存在旧的未使用的事件，则将其释放
				if ( &reference != NULL )
				{
					if ( reference.isValid() )
					{
						assert( reference.handle != NULL );
						assert( reference.handle->reference == &reference );

						m_Allocator.Destory( reference.handle );
					}

					reference.Assign( indexTick, temp );
				}

			    return TRUE;
            }
            __finally
            {
                m_Allocator.Unlock();
            }

            return FALSE;
        }

		BOOL RemoveRefHandle( EventHandle &reference )
		{
            m_Allocator.Lock();

            __try
            {
			    if ( !reference.isValid() )
				    return FALSE;

				assert( reference.handle != NULL );
				assert( reference.handle->reference == &reference );

			    m_Allocator.Destory( reference.handle );
			    reference.Clear();
			    return TRUE;
/*
			    if ( !reference.isValid() )
				    return FALSE;

			    WaitNode *pObject = m_Indexs[ reference.tick ].GetFront();
			    while ( pObject != NULL )
			    {
				    if ( pObject == reference.handle )
				    {
					    m_Allocator.Destory( ( WaitObject * )pObject );
					    reference.Clear();
					    return TRUE;
				    }

				    pObject = pObject->GetNext();
			    }
*/
            }
            __finally
            {
                m_Allocator.Unlock();
            }

            // 以后在完全保证没有找不到对象的状况下，可以进一步的将handle.handle直接拿来使用，可以进一步提高对象查找效率
            //assert( !"not find waiting object!!!" );
            return FALSE;
		}

		// 由于函数中使用的是NonSelfLock，所以，调用这个函数的应该不是调用Traverse的那个线程
		BOOL Reset()
		{
			if ( !m_Allocator.NonSelfLock() )
				return FALSE;

			__try
			{
				// 如果已经处于遍历状态中，则不能进行Reset
				if ( m_InTraverse )
					return FALSE;

				m_CheckCursor = 0;

				for ( int i=0; i<_maxLength; i++ )
				{
					while ( !m_Indexs[i].isEmpty() )
					{
						m_Allocator.Destory( m_Indexs[i].GetFront() );
					}
				}
			}
			__finally
			{
				m_Allocator.Unlock();
			}

			return TRUE;
		}

		DWORD Size() 
		{
			return m_Allocator.number;
		}

	public:
		WaitQueue()
		{
            m_InTraverse = FALSE;
			m_CheckCursor = 0;
		}

		~WaitQueue()
		{
			Reset();
		}

	private:
		// 用于读写等待队列时候的的临界锁
        // 多线程说明：
        // 一般说来遍历的入口只有一个，
        // 而队列&链表的操作则可能由多个事务处理线程发起，
        // 所以，这里需要为队列&链表操作添加临界锁，然而，
        // 队列&链表的操作有：Reset[不常用]、RemoveRefHandle[较频繁]、PushObject[频繁]、Traverse[频繁]

		BlockAllocator< WaitObject, _maxLength / 10, _lock > m_Allocator;

		BOOL m_InTraverse;

		// 检查时间用的游标
		DWORD m_CheckCursor;

		// 等待队列
		WaitHeader m_Indexs[_maxLength];
	};
}
