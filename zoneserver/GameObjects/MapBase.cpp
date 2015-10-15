#include "stdafx.h"
#include "MapBase.h"
//#include "MEApp.h"
MapBase::MapBase(void)
:m_pdwMapData(0),
m_bOpen(false),
m_dwMapSize( 0 ),
m_dwGridSize(0),
m_dwWidth(0),
m_dwHeight(0)
{

}

MapBase::MapBase( const MapBase& kMap )
:m_pdwMapData(0),
m_bOpen(false),
m_dwMapSize( 0 ),
m_dwGridSize(0),
m_dwWidth(0),
m_dwHeight(0)
{
	m_dwGridSize = kMap.m_dwGridSize;
	m_dwWidth = kMap.m_dwWidth;
	m_dwHeight =kMap.m_dwHeight;
	m_dwMapSize = m_dwWidth * m_dwHeight;

	//m_pdwMapData = (DWORD*)malloc( sizeof(DWORD) * m_dwMapSize );
	m_pdwMapData = new DWORD(sizeof(DWORD) * m_dwMapSize);
	if ( !m_pdwMapData ) 
	{
		m_dwGridSize = 0;
		m_dwWidth = 0;
		m_dwHeight = 0;
		m_dwMapSize = 0;
	} else {
		memcpy( m_pdwMapData, kMap.m_pdwMapData, sizeof(DWORD)*m_dwMapSize);
	}
}

MapBase::~MapBase(void)
{
	Close();
}

bool MapBase::Open( const char* pchMapPath )
{
	if ( !pchMapPath )
		return false;

	//Reset
	if ( m_bOpen )
		Close();

	FILE* fp;
	fp = fopen( pchMapPath, "rb" );
	if ( !fp)
		return false;

	if( !ReadHeader(fp) )
	{
		fclose(fp );
		return false;
	}

	if ( 0==m_dwMapSize )
		m_dwMapSize = 1;

	//m_pdwMapData = (DWORD*)malloc( sizeof(DWORD) * m_dwMapSize );
	m_pdwMapData = new DWORD[sizeof(DWORD) * m_dwMapSize ];
	if ( !m_pdwMapData ) {
		fclose(fp);
		return false;

	}

	if ( !ReadBody(fp) )
	{
		SAFE_DELETE( m_pdwMapData );
		fclose(fp );
		return false;
	}

	m_bOpen = true;
	m_sMapPath = pchMapPath;
	fclose(fp);
	
	if (false)
	{
		int		itmpWidth = GetWidth();
		int		itmpHeight = GetHeight();
		// test code 
		for (int x = 0; x < itmpWidth; x++)
		{
			for (int y = 0; y < itmpHeight; y++)
			{
				if (m_pdwMapData[itmpWidth*y + x ] != ET_MOVE)
				{
					char szTmp[64] = {0};
					//sprintf_s(szTmp, sizeof(szTmp), "stat[%d,%d]=%d", x, y, m_pdwMapData[itmpWidth*y + x ]);
					//printf(szTmp, "不能移动");
				}
			}
		}
	}



	return true;
}

// bool MapBase::Save( const char* pchSavePath/*=0*/ )
// {
// 	if ( !m_bOpen)
// 		return false;
// 
// 	//文件名
// 	const char* pSaveFN = 0;
// 	if ( pchSavePath )
// 		pSaveFN = pchSavePath;
// 	else
// 		pSaveFN = m_sMapPath.c_str();
// 
// 	if ( !pSaveFN )
// 		return false;
// 
// 	FILE* fp;
// 	fp = fopen( pSaveFN, "wb+" );
// 	if ( !fp)
// 		return false;
// 
// 	//wrire map data header
// 	if ( !WriteHeader(fp) )
// 	{
// 		fclose(fp);
// 		return false;
// 	}
// 
// 	//write map data body
// 	if ( !WriteBody(fp) )
// 	{
// 		fclose(fp);
// 		return false;
// 	}
// 
// 	fclose(fp);
// 	return true;
// }

bool MapBase::SetStatus( const DWORD x, const DWORD y, E_TILE_STATUS eStatus )
{
	if ( m_bOpen && x<GetWidth() && y<GetHeight() )
	{
		m_pdwMapData[GetWidth()*y + x ] = eStatus;
		return true;
	}

	return false;
}

E_TILE_STATUS MapBase::GetStatus( const  DWORD x, const DWORD y ) const
{
	if ( m_bOpen && x<GetWidth() && y<GetHeight() )
	{
		return (E_TILE_STATUS)m_pdwMapData[GetWidth()*y +x ];
	}

	return ET_INVALID;
}

void MapBase::Close()
{
	if ( m_bOpen )
	{
		SAFE_DELETE( m_pdwMapData );
		m_sMapPath = "";
		// 清空数据
		m_dwGridSize = 0;
		m_dwWidth = 0;
		m_dwHeight = 0;
		m_dwMapSize = 0;
		m_bOpen = false;
	}
}

// bool MapBase::Generate(DWORD dwGridSize, const DWORD dwWidth, const DWORD dwHeight,const char* pchFileName )
// {
// 	if( m_bOpen )
// 		Close();
// 
// 	//产生地图数据数组
// 	const DWORD dwTileCount = dwWidth* dwHeight;
// 	m_pdwMapData =  (DWORD*)malloc( sizeof(DWORD)*dwTileCount );
// 	if ( !m_pdwMapData)
// 		return false;
// 
// 	//设置
// 	m_dwGridSize = dwGridSize;
// 	m_dwWidth = dwWidth;
// 	m_dwHeight = dwHeight;
// 	m_dwMapSize = dwTileCount;
// 
// 	//初始化
// 	//memset( m_pdwMapData, 0,  sizeof(DWORD)*m_dwMapSize );
// 	for ( unsigned int i=0; i< m_dwMapSize; ++i )
// 		m_pdwMapData[i] = ((MEApp*)NiApplication::ms_pkApplication)->GetCubeInitStatus();
// 
// 
// 	if ( pchFileName )
// 		m_sMapPath = pchFileName;
// 	else
// 		m_sMapPath = "";
// 
// 	m_bOpen = true;
// 
// 	return true;
// }