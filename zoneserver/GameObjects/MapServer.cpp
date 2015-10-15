#include "stdafx.h"
#include "MapServer.h"

MapServer::MapServer(void)
{
}

MapServer::~MapServer(void)
{
}

// bool MapServer::WriteHeader( FILE* fp )
// {
// 	m_kHeader.mfWidth = (WORD)m_dwWidth;
// 	m_kHeader.mfHeight = (WORD)m_dwHeight;
// 	const DWORD dwByteCount = sizeof(SMapHeader);
// 	return 1==fwrite( &m_kHeader, dwByteCount, 1, fp );
// }

bool MapServer::ReadHeader( FILE* fp )
{
	const DWORD dwByteCount = sizeof(SMapHeader);
	if( 1 == fread( &m_kHeader, dwByteCount, 1, fp ))
	{
		if ( MY_MAP_VERSION != m_kHeader.mfVersion )
			return false;

		m_dwGridSize = m_kHeader.mfGridSize;
		m_dwWidth = m_kHeader.mfWidth;
		m_dwHeight = m_kHeader.mfHeight;
		m_dwMapSize = m_dwHeight * m_dwWidth;
		return true;
	}
	return false;
}

// bool MapServer::WriteBody( FILE* fp )
// {
// 	const DWORD dwByteCount = m_dwMapSize*sizeof(DWORD);
// 	return ( 1 == fwrite( GetBuffer(), dwByteCount, 1, fp ));
// }

bool MapServer::ReadBody( FILE* fp )
{
	const DWORD dwByteCount = m_dwMapSize*sizeof(DWORD);
	return ( 1 == fread( GetBuffer(), dwByteCount, 1, fp ));
	//return (1 == fread( m_pdwMapData, sizeof(DWORD), m_dwMapSize ,fp ));
}