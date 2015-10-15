//////////////////////////////////////////////////////////////////////////
// Author	:	JYL
// Date		:	2010-03-08  14:01
// Purpose	:	���л��࣬���ڽ������������л�Ϊ�ֽ�����δ��ɣ�
//////////////////////////////////////////////////////////////////////////

#ifndef CINARCHIVE_H
#define CINARCHIVE_H

#include "CStream.h"

class CInArchive
{
private:
	CStream stream;

public:
	CInArchive(CStream::UINT32 size = 0) : stream(size){}

	~CInArchive(){}

	// �������͵����л�
	CInArchive& operator<<(UINT8	uint8);
	CInArchive& operator<<(UINT16	uint16);
	CInArchive& operator<<(UINT32	uint32);
	CInArchive& operator<<(UINT64	uint64);
	CInArchive& operator<<(INT8		int8);
	CInArchive& operator<<(INT16	int16);
	CInArchive& operator<<(INT32	int32);
	CInArchive& operator<<(INT64	int64);
	CInArchive& operator<<(CHAR		character);
	CInArchive& operator<<(BOOL		torf);
	CInArchive& operator<<(FLOAT	floatnum);
	CInArchive& operator<<(DOUBLE	doublenum);

	// �Զ������͵����л�
	// ע�⣺������Ҫ�����л�������˵���������ڲ�ʵ��ArchiveIn;
	// void ArchiveIn(CInArchive& ar);
	template<typename T>
	CInArchive& operator<<(T& own)
	{
		own.ArchiveIn(*this);
	}

	// �������͵����л�
	template<typename T, UINT32 size>
	CInArchive& operator<<(T (&arr)[size])
	{
		// �������л�����Ĵ�С
		*this<<size;

		// Ȼ�����л�����Ԫ��
		for (SIZE_T i=0; i<size; i++)
			*this<<arr[i];
		
		return *this;
	}

private:
	// ��ֹ��������Ϳ�����ֵ
	CInArchive(const CInArchive&);
	const CInArchive& operator=(const CInArchive&);

	// ��ֹ�ڶ��ϴ���������
	void *operator new(size_t);
	void operator delete(void *);
	void *operator new[](size_t);
	void operator delete[](void *);
};

#endif // CINARCHIVE_H