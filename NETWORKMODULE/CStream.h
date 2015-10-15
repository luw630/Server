//////////////////////////////////////////////////////////////////////////
// Author	:	JYL
// Date		:	2010-03-08  13:27
// Purpose	:	����������Ϣ����ÿһ����̬���ȵ�������Ϣ���Ľṹ���£�
//				----------------------------------------------------
//				| MajorPro | SecondaryPro | Msg Size | Msg Body....|
//				----------------------------------------------------
//				@ MajorPro		: ��Ϣ����Э��
//				@ SecondaryPro	: ��Ϣ�Ĵ�Э��
//				@ Msg Size		: ��Ϣ�Ĵ�С���˲�����ѡ��������Ϊ�����⣩
//				@ Msg Body		: ��Ϣ�壬��������Ϣ���������ݣ���̬�ģ�
//////////////////////////////////////////////////////////////////////////

#ifndef CSTREAM_H
#define CSTREAM_H

#include <assert.h>
#include "typedef.h"

namespace DreamWorkXYD
{
	class CStream
	{
		enum {INCREASE_SIZE = 1024};
	private:
		UINT32		m_capacity;					// ��������
		UINT32		m_size;						// ��ǰ����С
		UINT32		m_pos;						// ��ָ�루ֻ���ڶ���
		UINT8		*m_stream;					// ��Ϣ��
		
		CStream();
		CStream(UINT32 cap);
		~CStream();

		// ��buf�н�size�ֽ�д����
		void write(void *buf, UINT32 size);		

		// ��buf�н�amount����СΪsize�Ķ���д����
		void write(void *buf, UINT32 amount, UINT32 size);

		// �����ж�ȡsize�ֽ�д��buf
		void read(void *buf, UINT32 size);

		// �����ж�ȡamount��size��С�Ķ���д��buf
		void read(void *buf, UINT32 amount, UINT32 size);

		void ReAlloc(UINT32 hint);				// ���仺��
		UINT32 AlignSize(UINT32 size);			// Բ����С

	private:
		// ��ֹ��������Ϳ�����ֵ
		CStream(const CStream&);
		const CStream& operator=(const CStream&);

		// ��ֹ�ڶ��ϴ���������
		void *operator new(size_t);
		void operator delete(void *);
		void *operator new[](size_t);
		void operator delete[](void *);
		
		friend class CInArchive;
		friend class COutArchive;
	};
}
#endif // CSTREAM_H