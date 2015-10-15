#include "StdAfx.h"
#include "extraitemdata.h"


CExtraItemData::~CExtraItemData(void)
{
    ZeroData();
}

void CExtraItemData::UpdateItemData()
{  
    int n = 0;
    for(; n < MAX_EXITEMDATA; ) {
        switch(m_stExtraData[n].byType) 
        {
            case DT_DAM:
                m_stItemData.wDAMAD += m_stExtraData[n].byNum;
                break;
            case DT_POW:
                m_stItemData.wPOWAD += m_stExtraData[n].byNum;
                break;
            case DT_DEF:
                m_stItemData.wDEFAD += m_stExtraData[n].byNum;
                break;
            case DT_AGI:
                m_stItemData.wAGIAD += m_stExtraData[n].byNum;
                break;
            case DT_LUA:
                m_stItemData.wLUAD += m_stExtraData[n].byNum;
                break;
            case DT_AMU:
                m_stItemData.wAmuckAD += m_stExtraData[n].byNum;
                break;
            case DT_IN:
                m_stItemData.wIN += m_stExtraData[n].byNum;
                break;
            case DT_MHP:
                m_stItemData.wHPAD += m_stExtraData[n].byNum;
                break;
            case DT_MMP:
                m_stItemData.wMPAD += m_stExtraData[n].byNum;
                break;
            case DT_MSP:
                m_stItemData.wSPAD += m_stExtraData[n].byNum;
                break;
            case DT_RHP:
                m_stItemData.wRHPAD += m_stExtraData[n].byNum;
                break;
            case DT_RMP:
                m_stItemData.wRMPAD += m_stExtraData[n].byNum;
                break;
            case DT_RSP:
                m_stItemData.wRSPAD += m_stExtraData[n].byNum;
                break;

            default:
                return;

        }
        n++;
    }
}

void CExtraItemData::ZeroData() 
{
    ZeroMemory(&m_stExtraData, sizeof(EXTRADATA)*MAX_EXITEMDATA); 
    ZeroMemory(&m_stItemData, sizeof(EXTRA_ITEMDATA)); 
    m_wCount = 0;
}

bool CExtraItemData::AddExtraData(EXTRADATA *pData)
{
    if(pData == NULL)
        return false;

    if(m_wCount > MAX_EXITEMDATA)
        return false;

    int n = 0;
    for(; n < MAX_EXITEMDATA; n++) 
    {
        if (m_stExtraData[n].byType == pData->byType)
        {
            if (m_stExtraData[n].byNum < pData->byNum)
                m_stExtraData[n].byNum = pData->byNum;

            break;
        }
        else if(m_stExtraData[n].byNum != 0)
            continue;

        memcpy(&m_stExtraData[n], pData, sizeof(EXTRADATA));
        m_wCount++;
        break;
    }
    
    return true;
}
