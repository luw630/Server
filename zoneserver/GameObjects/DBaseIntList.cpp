#include "stdafx.h"
#include "DBaseIntList.h"

CBaseIntList::CBaseIntList()
{
    m_iCount    = 0;
    m_pData     = NULL;;
}
CBaseIntList::~CBaseIntList()
{
    ClearData();
}
///////////////////////////////////////////////
BOOL            CBaseIntList::LoadDataFromFile(char *fileName)
{
    dwt::ifstream   FileStream;

    FileStream.open( fileName );

    if( !FileStream.is_open() )
        return rfalse( "无法打开[%s]数据文件", fileName );// 
    
    FileStream >> m_iCount;
    if( m_iCount )
    {
        m_pData = new int[m_iCount];

        for( int i=0; i<m_iCount; i++ )
        {
            if( FileStream.eof() )      break;
            FileStream >> m_pData[i];
        }
    }

    FileStream.close();

    return true;
}

BOOL            CBaseIntList::SaveDataToFile(char *fileName)
{
    return  false;
}

void            CBaseIntList::ClearData(void)
{
    if( m_pData )
    {
        delete[]  m_pData;
        m_iCount    = 0;
        m_pData     = NULL;;
    }
}

int             CBaseIntList::SetData( int *pData, int iCount )
{
    // ???
    return  0;
}
