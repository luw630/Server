/*************************************************
Copyright (C), 2003-2005, U-Soft Co., Ltd.

File name:      WaitObject.h
Author:         Joe Xu
Version:        1.0.0.0
Date:           2005-6-20

Description:
һ�����ڶ���ȴ�������ģ�棬�������趨��ʱ�䵽��󣬿���ͨ��TraverseAll������ʹ����ʱ��
����Ķ�����в����ص���������ڽ���AI��Ӧ�ص��Ĳ��֣�

Attation:
��Ȼ����ʹ��SetTimer�������棬���Ƕ�����Ϸ����Ч�Ǳ���ġ�����

���ʹ�þɵĵ��ȷ�ʽ����������ÿһ��ʱ�䵥λ����Ҫ�������ж������ж�Ŀ������Ƿ�ﵽ��
����ʱ����ޣ��Ӷ��ж���������Ƿ����ֵ��ã����Ƕ�����Ϸ��˵����ߵĶ�����ͬһʱ�̻��
�ִ���AI���õĻ���ǳ��٣���������û�У������󲿷�ʱ�䶼���ڵȴ�ʱ�䵽��������������
�£���������������Դ�˷ѷǳ�֮��

����һ������������10�������ÿ������ƽ��4��Ż����һ��������ߵ�AI������ƽ��1�����2.5
����������AI��Ӧ���������ʹ�ñ�������������ÿһ֡����Ҫ����AI�жϣ���һ��1����20֡��
���ԣ�ÿ4������[4 * 20 * 10��]800����жϣ�����������Щ�ж�ǰ��ÿһ������һ�㻹�����麯
�����ã���д�ڴ�ȶ���Ĵ��ۣ����Դ������˷��Ǿ��˵ģ�

��������ʵ����release�¶�������ָ��Ĺ�����Դﵽÿ��120��ε��ã�����һ��ָ��Ĺ����
�Դﵽÿ��300��ε��ã������ϴﵽ��ͨ����map������һ����Ч����ͬʱҲ�����𵽵ȴ�ʱ�䵽��
��Ż���ֵ��ã���ʡ�˼����CPU��Դ��

PS��
�����ڶ��߳��³��ԣ��������ʹ���ٽ�����֤�������״���£�1����Լ���Դﵽ100���[��ָ��]


Template class list:
TAutoAllocator  ���ڽ��п��ٷ����ģ�棬ʵ���˷��䡢���졢�����������⼸������
TWaitQueue      ���ڶ���ȴ�������ģ��

History:
*************************************************/

#pragma once

#include "mmsystem.h"
#include "assert.h"
#include "new.h"

namespace UGE
{
    // ����
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

    // �ٽ���
    class CriticalLock
    {
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

    // դ��
    template< typename _lock >
    class Barrier
    {
    public:
        Barrier( _lock &other ) : lock( other ) { lock.Lock(); }
        ~Barrier() { lock.Unlock(); }

    private:
        _lock &lock;
    };

	// һ���̶��ռ��������ר���ڹ̶���С�Ŀռ����
	// _TObject  ��ָ�����䵥Ԫ�ռ�Ĺ̶���С
	// _growStep ��ָ�������൥Ԫ����ʱ�����ݿ�ĳɳ���С
    // _lock     �����ڶ��̵߳�ʱ������ٽ�������
	// ����void **��ʹ��˵�����������ǽ�����Ϊ������ָ����ʹ��
	// ���Է����ʱ���п��Ƕ����sizeof(void *)�ռ�ģ�
	template < typename _TObject, unsigned long _growStep, typename _lock >
    class BlockAllocator : public _lock
	{
	public:
		_TObject* Construct()
		{
			// ��ʾָ����Ŀ��ռ��Ͻ��й���
			return new ( Alloc() ) _TObject;
		}

		template < typename _TParam >
			_TObject* Construct( _TParam &param )
		{
			return new ( Alloc() ) _TObject( param );
		}

		void Destory( _TObject *pObj )
		{
			// ��ʾָ�����������
            ( pObj )->~_TObject();
			Free( pObj );
		}

	private:
		void *Alloc()
		{
            // ����ǰ��������դ�����ٽ���
            Barrier< _lock >( *this );

			volatile void **ppTemp = NULL;
			if ( m_ppJunk == NULL )
			{
				// ���û�п���ķ���ռ䣬��ʹ��ϵͳ��new����һ��ָ�����ȵĿռ�
				// �ռ�ĵ�һ��λ����void*�����ڽ������ѷ���Ŀռ䴮����
				// m_ppHeader����ר�����ڼ�¼�ѷ���ռ�������׽ڵ�
				ppTemp = ( volatile void ** ) new char [ sizeof( void * ) + 
                    ( sizeof( void * ) + sizeof( _TObject ) ) * _growStep ];
				*ppTemp = m_ppHeader;
				m_ppHeader = ppTemp;

				// ������һ��λ�ã��ѷ���ռ�����ڵ㣩
				ppTemp ++;

				// ������ռ��е�ÿһ���鴮�����ŵ�δʹ�õ�Ԫ���������
				// ͬ����ÿ����Ԫ��ĵ�һ��λ��Ҳ��void*�����ڽ�����δʹ�õ�Ԫ�鴮����
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
            // ����ǰ��������դ�����ٽ���
            Barrier< _lock > barrier( *this );

			// ���յ�Ԫ�飬�����ͷţ��ȴ��´�ʹ��
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

			// ����ʱ�ͷ������Ѿ�����Ŀռ�
			// ����ǰ���Ǳ��뱣֤���е�Ԫ��δ��ʹ�ã�
			while ( m_ppHeader != NULL )
			{
				volatile void *pTemp = m_ppHeader;
				m_ppHeader = ( volatile void ** )*m_ppHeader;
				delete pTemp;
			}
		}

	private:
		volatile void **m_ppHeader;     // ���ڱ������ռ���ʼ��ַ������
		volatile void **m_ppJunk;       // ���ڱ���δʹ�õ�Ԫ�������

	public:
		volatile DWORD number;			// ���ڱ���δʹ�õ�Ԫ�������
	};

	// ����ȴ�������ģ��
	// _TObject     �����ڵȴ��Ķ��󣬿������κζ���
	// _maxLength   ��Ϊ����һ������λʱ����в�����Ĺ��������������������ָ�������Ҫ�ȴ��ĵ�λʱ��
    // _lock        �ٽ����������ͣ������ڶ��߳�״̬�¹���������Ĭ��״����ʹ�ò�֧�ֶ��̵߳Ŀ���
	template < typename _TObject, unsigned long _maxLength, typename _lock = DummyLock >
	class WaitQueue
	{
	public:
		struct WaitObject;

		// ���������¼��ľ��
		class EventHandle
		{
		public:
			EventHandle() : tick( 0 ), handle( NULL ) {}

		private:
			void Clear() { *reinterpret_cast< __int64 * >( this ) = 0; }
			void Assign( DWORD tick, WaitObject *handle ) { this->tick = tick; this->handle = handle; }
			bool isValid() { return handle != NULL; }

		private:
			DWORD tick;			// ֻ�����ڼ�¼λ�ã���ֱ�Ӳ�����ָ��Ŀռ�
			WaitObject *handle; // �¼��ľ����ַ�����ڱȽϺͲ��ң���������ֱ�Ӳ���

			friend class WaitQueue;
		};

	private:
		// �ȴ��ڵ㣬Ҫ����Խ��Լ���˫��������ɾ������ʵ��ĳ��������Ҫȡ���Լ���ǰ�����ĵȴ���
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

		// ����ĵȴ�����ڵ㣬���ڰ���Ҫ��Ӧ�Ķ���
		class WaitObject :
			public WaitNode
		{
		    WaitObject( _TObject &o ) : m_Object( o ) {}

			~WaitObject() { CutLink(); }

		private:
			// �ȴ�����Ķ���
			_TObject m_Object;
            void *m_uData;

		private:
			EventHandle *reference;

			friend class WaitQueue;
			friend class BlockAllocator< WaitObject, _maxLength / 10, _lock >;
		};

		// �ȴ�������ײ�����Ϊ��ʱ�����ÿ����λ��ָ������Ҫ׼��һ��������
		// �����������Ԫ�غܴ���ô��ռ�ø���Ŀռ䣬�����һ�����еķ�����
		struct WaitHeader
		{
			bool isEmpty() { return m_pNext == NULL; }

			// ��һ����һ����Ҫע��ĵط���
			// ��ΪҪʵ�ֽڵ�������Ƴ���Ҫ˫��ڵ㣬����Ϊ�˼�С�ռ䣬���ｫ�Լ�
			// ��Ϊ��˫��������ײ�����ΪInsertBack����������Լ���m_pPrev�����Բ�����ִ���
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
		// ���������Ѿ���ʱ�Ķ���
		// ������for_each������ʹ�ú������������лص���
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

            // �ϱߵ� end ȡ��ַ����Խ�磡�����ｫ�����
            if ( ( m_CheckCursor + timeMargin ) >= _maxLength )
                end = &m_Indexs[ ( m_CheckCursor + timeMargin ) - _maxLength ];

            // ѭ���Ĵ�������ʱ���Ѿ���ʱ�Ķ���
            while ( it != end )
            {
				//if ( it == &m_Indexs[0] )
				//{
				//	char temp[123];
				//	sprintf( temp, "traverse [%d] [%d] [%d]\r\n", m_Allocator.number, !it->isEmpty(), end - it );
				//	OutputDebugString( temp );
				//}

				// ����Ѿ������ݴ��ھ��趨ret���˳�ѭ��
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

					    // ����һ���ŵ���������Ϊ�ڻص������п��ܻ�����޸�����
					    // ���������m_Indexs[m_CheckCursor] = p->LinkPtr();��֮ǰ���޸ĵĻ�
					    // �������Ϳ��ܻ�ϵ�������
					    tempObj = p->m_Object;
					    tempData = p->m_uData;

						if ( p->reference != NULL )
						{
							assert( p->reference->handle == p );

							// ͬʱ���ٽ����У������õľ�����
							// ��������������������������������
							// �����漰��һ��ָ��������Ч�����⣺
							// reference���ⲿ������Ч�Ե�һ���ռ䣬����ⲿ���ÿռ��ͷš���
							// ������
							// ��ʵ����Ҳ�� p->m_Object��p->m_uData ͬ����������״��
							// ������Ҫ�û��Լ�ϸ�ĵĴ���
							// ��򵥵Ľ��������ʵ�ǣ�m_uData��reference����m_Object�е�����
							// ������ֻ��Ҫ��֤m_Object��Ч�Ϳ�����
							p->reference->Clear();
						}

					    m_Allocator.Destory( p );
                    }
                    __finally
                    {
                        m_Allocator.Unlock();
                    }

                    // �����¼���������Ͳ������ˣ��ݴ���Ҳ�����¼�������ȥ��
					op( tempObj, tempData );
				}

                it ++;
				m_CheckCursor ++;

                if ( it != &m_Indexs[_maxLength] )
                    continue;

                // �� it == &m_Indexs[_maxLength] ʱ���Ѿ�����һȦ�ˣ�Ҫ��ͷ����
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

		// ��ȴ�������ѹ��һ������
		BOOL PushObject( unsigned long waitTick, _TObject &obj, void *uData = NULL, EventHandle &reference = *reinterpret_cast< EventHandle * >( NULL ) )
		{
            m_Allocator.Lock();

            __try
            {
                assert( m_CheckCursor < _maxLength );
                assert( waitTick != 0 );

			    // �����������������ɵĵȴ�ʱ�䣬ѹ��ʧ��
			    if ( ( waitTick >= _maxLength ) || ( waitTick == 0 ) )
				    return FALSE;

			    unsigned long indexTick = m_CheckCursor + waitTick;

			    if ( indexTick >= _maxLength )
				    indexTick -= _maxLength;

				// �����ȴ��¼�����ѹ��ȴ�����
			    WaitObject *temp = m_Allocator.Construct( obj );
                temp->m_uData = uData;
				temp->reference = &reference;
			    m_Indexs[ indexTick ].PushFront( temp );

				// ��ѹ��֮���ж�reference��״̬����������ھɵ�δʹ�õ��¼��������ͷ�
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

            // �Ժ�����ȫ��֤û���Ҳ��������״���£����Խ�һ���Ľ�handle.handleֱ������ʹ�ã����Խ�һ����߶������Ч��
            //assert( !"not find waiting object!!!" );
            return FALSE;
		}

		// ���ں�����ʹ�õ���NonSelfLock�����ԣ��������������Ӧ�ò��ǵ���Traverse���Ǹ��߳�
		BOOL Reset()
		{
			if ( !m_Allocator.NonSelfLock() )
				return FALSE;

			__try
			{
				// ����Ѿ����ڱ���״̬�У����ܽ���Reset
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
		// ���ڶ�д�ȴ�����ʱ��ĵ��ٽ���
        // ���߳�˵����
        // һ��˵�����������ֻ��һ����
        // ������&����Ĳ���������ɶ���������̷߳���
        // ���ԣ�������ҪΪ����&�����������ٽ�����Ȼ����
        // ����&����Ĳ����У�Reset[������]��RemoveRefHandle[��Ƶ��]��PushObject[Ƶ��]��Traverse[Ƶ��]

		BlockAllocator< WaitObject, _maxLength / 10, _lock > m_Allocator;

		BOOL m_InTraverse;

		// ���ʱ���õ��α�
		DWORD m_CheckCursor;

		// �ȴ�����
		WaitHeader m_Indexs[_maxLength];
	};
}
