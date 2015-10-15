#include "stdafx.h"
#include "DSpcialItemData.h"

CSpecialItemData    *g_pSpecialItemData = NULL;

CSpecialItemData::CSpecialItemData()
{
    m_pData = NULL;
    m_iCreatedItem  = 0;
    m_iUseedItem    = 0;
}
CSpecialItemData::~CSpecialItemData()
{
}
///////////////////////////////////////////////
BOOL        CSpecialItemData::LoadData(void)
{
    BOOL    bRet = false;
    
    m_IntData.ClearData();

    bRet = m_IntData.LoadDataFromFile( FILENAME_SPECIALITEM );
    m_pData = m_IntData.GetData();
    m_iCount = m_IntData.GetCount();

    return  bRet;
}

void        CSpecialItemData::OnCreatedItem( int itemID, int count )
{
    for( int i=0; i<m_iCount; ++i )
    {
        if( itemID==m_pData[i] )
        {
            m_iCreatedItem += count;
            break;
        }
    }
}

void        CSpecialItemData::OnUseedItem( int itemID, int count )
{
    for( int i=0; i<m_iCount; ++i )
    {
        if( itemID==m_pData[i] )
        {
            m_iUseedItem += count;
            break;
        }
    }
}
