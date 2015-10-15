//////////////////////////////////////////////////////////////////////////
// Author	:	JYL
// Date		:	2010-03-08  13:27
// Purpose	:	描述网络消息流，每一个动态长度的网络消息流的结构如下：
//				----------------------------------------------------
//				| MajorPro | SecondaryPro | Msg Size | Msg Body....|
//				----------------------------------------------------
//				@ MajorPro		: 消息的主协议
//				@ SecondaryPro	: 消息的次协议
//				@ Msg Size		: 消息的大小（此参数可选，可以作为错误检测）
//				@ Msg Body		: 消息体，包含了消息的所有数据（动态的）
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
		UINT32		m_capacity;					// 流的容量
		UINT32		m_size;						// 当前流大小
		UINT32		m_pos;						// 流指针（只用于读）
		UINT8		*m_stream;					// 消息流
		
		CStream();
		CStream(UINT32 cap);
		~CStream();

		// 从buf中将size字节写入流
		void write(void *buf, UINT32 size);		

		// 从buf中将amount个大小为size的对象写入流
		void write(void *buf, UINT32 amount, UINT32 size);

		// 从流中读取size字节写入buf
		void read(void *buf, UINT32 size);

		// 从流中读取amount个size大小的对象写入buf
		void read(void *buf, UINT32 amount, UINT32 size);

		void ReAlloc(UINT32 hint);				// 扩充缓存
		UINT32 AlignSize(UINT32 size);			// 圆整大小

	private:
		// 禁止拷贝构造和拷贝赋值
		CStream(const CStream&);
		const CStream& operator=(const CStream&);

		// 禁止在堆上创建流对象
		void *operator new(size_t);
		void operator delete(void *);
		void *operator new[](size_t);
		void operator delete[](void *);
		
		friend class CInArchive;
		friend class COutArchive;
	};
}
#endif // CSTREAM_H