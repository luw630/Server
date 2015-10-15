#include "stdafx.h"
#include "GoatMutateData.h"


CGoatMutateData    *g_pGoatMutateData = NULL;

CGoatMutateData::CGoatMutateData()
{
    m_pData = NULL;
    m_iCount= 0;
}
CGoatMutateData::~CGoatMutateData()
{
}
///////////////////////////////////////////////
BOOL        CGoatMutateData::LoadData(void)
{
    BOOL    bRet = false;
    
    bRet = m_IntData.LoadDataFromFile( FILENAME_GOATMUTATEDATA );
    m_pData = m_IntData.GetData();
    m_iCount = m_IntData.GetCount();

    return  bRet;
}

int         CGoatMutateData::GetGoatMutateOdds( int iGoatID ) // 目前只支持1-11套和5大套
{
    if( iGoatID>=11 || (iGoatID+13)>=m_iCount || iGoatID<0 )
    {
        if ( iGoatID>=19 &&  iGoatID<=23)
            return m_pData[23];
        return  0;
    }

    return  m_pData[iGoatID+12];
}

int         CGoatMutateData::GetGoatMutateTime( int iGoatID )
{
    if( iGoatID>=11 || iGoatID>=m_iCount || iGoatID<0 )
    {
        if ( iGoatID>=19 &&  iGoatID<=23)
            return m_pData[11];
        return  0;
    }

    return  m_pData[iGoatID];
}
