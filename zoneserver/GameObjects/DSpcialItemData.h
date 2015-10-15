#ifndef     _SpecialItemData_H_
#define     _SpcialItemData_H_

#include "DBaseIntList.h"

#define     FILENAME_SPECIALITEM        "Data\\SpecialItem.txt"

class   CSpecialItemData
{
    CBaseIntList        m_IntData;
    int                 *m_pData;
    int                 m_iCount;

    int                 m_iCreatedItem;
    int                 m_iUseedItem;
public:
    CSpecialItemData(void);
    ~CSpecialItemData(void);

    BOOL        LoadData(void);

    void        OnCreatedItem( int itemID, int count );
    void        OnUseedItem( int itemID, int count );

    int         GetCreatedItem(void)        {   return  m_iCreatedItem;                         }
    int         GetUseedItem(void)          {   return  m_iUseedItem;                           }
};

extern  CSpecialItemData    *g_pSpecialItemData;

#endif  _SpecialItemData_H_