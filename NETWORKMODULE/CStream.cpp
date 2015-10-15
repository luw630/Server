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

// hint��һ����ʾֵ��������ĳЩ����¼������·���Ĵ���
void CStream::ReAlloc(UINT32 hint)
{
	UINT32 inc = AlignSize(hint);
	UINT8 *temp = new UINT8[m_capacity+inc];	// ��չbuffer
	if (m_stream)
	{
		memcpy(temp, m_stream, m_size);			// �������
		delete[] m_stream;
	}
	m_stream = temp;							// ������Ϣ��
	m_capacity += inc;
	return;
}

void CStream::write(void *buf, UINT32 size)
{
	if (!buf || !size)
		return;
	
	if (m_size + size > m_capacity)			// �����Ҫ����չ��������		
		ReAlloc(size);
	
	memcpy(m_stream+m_size, buf, size);		// д��
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

	// ������Ϣ����˵��C/S���˵�����Ӧ����ȫƥ�䣬������ַ���Խ��
	// �ͱ�ʾC/S���˵����л�/�����л���һ��
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