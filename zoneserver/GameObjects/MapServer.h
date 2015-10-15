#pragma once
#include "mapbase.h"



class MapServer :
	public MapBase
{
public:
	MapServer(void);
	~MapServer(void);

	//virtual bool WriteHeader( FILE* fp);
	virtual bool ReadHeader( FILE* fp);
	//virtual bool WriteBody( FILE* fp);
	virtual bool ReadBody( FILE* fp);

private:
	SMapHeader	m_kHeader;
};
