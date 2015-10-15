//////////////////////////////////////////////////////////////////////////
// Author	:	JYL
// Date		:	2010-03-08  14:01
// Purpose	:	序列化类，用于将各种类型序列化为字节流（未完成）
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

	// 基本类型的序列化
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

	// 自定义类型的序列化
	// 注意：对于想要被序列化的类来说，必须再内部实现ArchiveIn;
	// void ArchiveIn(CInArchive& ar);
	template<typename T>
	CInArchive& operator<<(T& own)
	{
		own.ArchiveIn(*this);
	}

	// 数组类型的序列化
	template<typename T, UINT32 size>
	CInArchive& operator<<(T (&arr)[size])
	{
		// 首先序列化数组的大小
		*this<<size;

		// 然后序列化所有元素
		for (SIZE_T i=0; i<size; i++)
			*this<<arr[i];
		
		return *this;
	}

private:
	// 禁止拷贝构造和拷贝赋值
	CInArchive(const CInArchive&);
	const CInArchive& operator=(const CInArchive&);

	// 禁止在堆上创建流对象
	void *operator new(size_t);
	void operator delete(void *);
	void *operator new[](size_t);
	void operator delete[](void *);
};

#endif // CINARCHIVE_H