#include "Stdafx.h"
#include "CInArchive.h"

CInArchive& CInArchive::operator<<(UINT8 uint8)
{
	stream.write(&uint8, sizeof(uint8));
	return *this;
}

CInArchive& CInArchive::operator<<(UINT16	uint16)
{
	stream.write(&uint16, sizeof(uint16));
	return *this;
}

CInArchive& CInArchive::operator<<(UINT32	uint32)
{
	stream.write(&uint32, sizeof(uint32));
	return *this;
}

CInArchive& CInArchive::operator<<(CStream::UINT64	uint64)
{
	stream.write(&uint64, sizeof(uint64));
	return *this;
}

CInArchive& CInArchive::operator<<(CStream::INT8 int8)
{
	stream.write(&int8, sizeof(int8));
	return *this;
}

CInArchive& CInArchive::operator<<(CStream::INT16 int16)
{
	stream.write(&int16, sizeof(int16));
	return *this;
}

CInArchive& CInArchive::operator<<(CStream::INT32 int32)
{
	stream.write(&int32, sizeof(int32));
	return *this;
}

CInArchive& CInArchive::operator<<(CStream::INT64 int64)
{
	stream.write(&int64, sizeof(int64));
	return *this;
}

CInArchive& CInArchive::operator<<(CStream::CHAR character)
{
	stream.write(&character, sizeof(character));
	return *this;
}

CInArchive& CInArchive::operator<<(CStream::BOOL torf)
{
	stream.write(&torf, sizeof(torf));
	return *this;
}

CInArchive& CInArchive::operator<<(FLOAT floatnum)
{
	stream.write(&floatnum, sizeof(floatnum));
	return *this;
}

CInArchive& CInArchive::operator<<(DOUBLE	doublenum)
{
	stream.write(&doublenum, sizeof(doublenum));
	return *this;
}