#pragma once

// Windows ͷ�ļ�:
#include <windows.h>

#include <cstdlib>
#include <tchar.h>
#include <vector>
#include <string>
#include <map>
#include <deque>
using namespace std;

//#include <NiApplication.h>
//#include <NiAnimation.h>

// #include <NiEntity.h>
// 
//#include <NiLicense.h>
// 
// #include <NiMain.h>
// #include <NiMath.h>
// #include <NiMemoryDefines.h>
// #include <NiMemObject.h>
// #include <NiMesh.h>
// #include <NiMeshLib.h>

// #include <NiParticle.h>
// #include <NiPick.h>
// 
// #include <NiSample.h>
// #include <NiSystem.h>

//#include <NiTerrainComponentLib.h>
//#include <NiTerrainComponent.h>

// #include <NiUIManager.h>
// #include <NiUICheckBox.h>
// #include <NiUISlider.h>

//#include <NiWaterComponent.h>

#include <string>
using std::string;

// #pragma comment(lib, "NiBinaryShaderLib.lib")
// #pragma comment(lib, "NiD3D10BinaryShaderLib.lib")
// #pragma comment(lib, "NSBShaderLib.lib")
// #pragma comment(lib, "NSFParserLib.lib")

// NiEmbedGamebryoLicenseCode;

#define NI_SAFE_DELETE(p)	do{	\
	if( NULL != p ){	\
	NiDelete p;	\
	p = NULL;	\
	}	\
} while (0)

#define NI_SAFE_DELETE_ARRAY(p)		do{	\
	if( NULL != p ){	\
	NiDelete[] p;	\
	p = NULL;	\
	}	\
} while (0)

#define F_EQ(fa, fb)	(fabs( (fa)-(fb) ) < 1e-8)
#define F_EQ_ZERO(fa)	F_EQ( fa, 0.0f )


#define TILE_STATUS_COUNT	5

//Tile ״̬
enum E_TILE_STATUS
{
	ET_MOVE		= 0x01,				// �ر���ƶ�
	ET_JUMP		= 0x02,				// �ر����Ծ
	ET_LIMIT	= 0x04,				// ��������
	ET_STOP		= 0x10,				// �ر�������

	ET_INVALID	= 0xFFFF,
};

#define INVALID_SIZE ((DWORD)-1)
//#define SAFE_DELETE(p) if(p){delete p;p=0;}
//#define malloc NiMalloc

// #define SAFE_DELETE(p)	do{	\
// 	if( NULL != p ){	\
// 	NiFree(p);	\
// 	p = NULL;	\
// 	}	\
// 	}while(0)

////////////////////////////////////////////////////

//��ͼ�ļ��汾
#define MY_MAP_VERSION 4

//const DWORD TILETYPE_MOVE   = 0x01;     // �ر���ƶ�
//const DWORD TILETYPE_JUMP   = 0x02;     // �ر����Ծ
//const DWORD TILETYPE_LIMIT	= 0x04;     // ��������
//const DWORD TILETYPE_STOP   = 0x10;     // �ر�������

//����������ĵ�ͼ�ļ���ʽ :
//		header + DWORD[mfWidth*mfHeight]
struct SMapHeader
{
	WORD    mfType;         // MAP���λ
	WORD    mfVersion;      // �汾
	DWORD   mfSize;         // �ļ���С
	DWORD   mfBlock0Seg;    // ����ƫ�Ƶ�ַ
	DWORD   mfBlock1Seg;    // ����ƫ�Ƶ�ַ
	DWORD   mfBlock2Seg;    // ����ƫ�Ƶ�ַ
	DWORD   mfBlock3Seg;    // ����ƫ�Ƶ�ַ
	DWORD   mfBlock4Seg;    // ����ƫ�Ƶ�ַ
	WORD    mfMYID;         // ����ͼ�ı��
	WORD	mfGridSize;		// [2011-3-22 17:20 gw]  ���Ӵ�С
	WORD    mfWidth;        // ����ͼ�Ŀ�
	WORD    mfHeight;       // ����ͼ�ĸߣ�Tile��
	BYTE    bfReserved[6];

	SMapHeader()
	{
		memset( this, 0, sizeof(SMapHeader) );
		mfVersion = MY_MAP_VERSION;	
	}
};

//////////////////////////////////////////////////////////////////////////

class MapBase
	/*:public NiMemObject*/
{
public:
	MapBase(void);
	MapBase(const MapBase& kMap);
	virtual ~MapBase(void);

	///
	void Close();

	///�������е�ͼ
	bool Open(const char* pchMapPath );

	///�����ͼ��ָ���ļ�
	///<param name=pchSavePaht> ���pchSavePathΪ�գ���дԭ�ļ�(m_sMapPath)		
	//bool Save(const char* pchSavePath=0);

	///����ָ������ĵ�ͼ����
	//bool Generate(DWORD dwGridSize, const DWORD dwWidth, const DWORD dwHeight, const char* pchFileName=0);

	const char* GetFileName() const { return m_bOpen ? m_sMapPath.c_str() : 0; };
	bool		IsOpen()	  const {return m_bOpen;}

	DWORD GetGridSize()		  const { return m_bOpen ? m_dwGridSize : INVALID_SIZE; }
	DWORD GetWidth()		  const { return m_bOpen ? m_dwWidth : INVALID_SIZE; };
	DWORD GetHeight()		  const { return m_bOpen ? m_dwHeight : INVALID_SIZE; };
	DWORD GetSize()			  const {return m_bOpen ? m_dwMapSize: INVALID_SIZE;}


	bool			SetStatus( const DWORD x, const DWORD y, E_TILE_STATUS eStatus);
	E_TILE_STATUS	GetStatus( const DWORD x, const DWORD y) const;

	//���Ҫ�������ŵ�ͼ������������Ӧ��ȥ��
	////const SMapHeader& GetMapHeader(){ return m_kMapHeader; };
	DWORD*			GetBuffer() const { return m_pdwMapData; };

protected:	
	//virtual bool WriteHeader( FILE* fp)=0;
	virtual bool ReadHeader( FILE* fp)=0;
	//virtual bool WriteBody( FILE* fp)=0;
	virtual bool ReadBody( FILE* fp)=0;
protected:
	string	m_sMapPath;
	bool	m_bOpen;

	DWORD	m_dwGridSize; // [2011-3-22 17:03 gw]+���Ӵ�С 
	DWORD	m_dwWidth;
	DWORD	m_dwHeight;
	DWORD	m_dwMapSize;
	DWORD*	m_pdwMapData;
};
