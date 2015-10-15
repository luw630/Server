#include "StdAfx.h"
#include "CStream.h"

CStream::CStream() : m_capacity(0), m_size(0), m_pos(0), m_stream(0)
{
};

CStream::CStream(UINT32 cap)
{
	m_capacity	= AlignSize(cap);
	m_size		= 0;
	m_pos		= 0;
	m_stream	= new UINT8[m_capacity];
}

CStream::~CStream()
{
	if (m_stream)
		delete[] m_stream;
}

UINT32 CStream::AlignSize(UINT32 size)
{
	return size <= INCREASE_SIZE ? INCREASE_SIZE : ((size/INCREASE_SIZE+1) * INCREASE_SIZE);
}

// hint是一个提示值，用于在某些情况下减少重新分配的次数
void CStream::ReAlloc(UINT32 hint)
{
	UINT32 inc = AlignSize(hint);
	UINT8 *temp = new UINT8[m_capacity+inc];	// 扩展buffer
	if (m_stream)
	{
		memcpy(temp, m_stream, m_size);			// 进行深拷贝
		delete[] m_stream;
	}
	m_stream = temp;							// 更新消息流
	m_capacity += inc;
	return;
}

void CStream::write(void *buf, UINT32 size)
{
	if (!buf || !size)
		return;
	
	if (m_size + size > m_capacity)			// 如果需要，扩展流缓冲区		
		ReAlloc(size);
	
	memcpy(m_stream+m_size, buf, size);		// 写入
	m_size += size;							
	return;
}

void CStream::write(void *buf, UINT32 amount, UINT32 size)
{
	UINT32 total = amount*size;

	if (!buf || !total)
		return;

	if (m_size + total > m_capacity)
		ReAlloc(total);

	memcpy(m_stream+m_size, buf, total);
	m_size += total;
	return;
}

void CStream::read(void *buf, UINT32 size)
{
	if (!buf || !size)
		return;

	// 对于消息流来说，C/S两端的数据应该完全匹配，如果出现访问越界
	// 就表示C/S两端的序列化/反序列化不一致
	assert(m_pos + size <= m_size);

	memcpy(buf, m_stream+m_pos, size);
	return;
}

void CStream::read(void *buf, UINT32 amount, UINT32 size)
{
	UINT32 total = amount*size;

	if (!buf || !total)
		return;

	assert(m_pos+total <= m_size);

	memcpy(buf, m_stream+m_pos, total);
	return;
}