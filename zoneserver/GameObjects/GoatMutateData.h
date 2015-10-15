#ifndef     _GoatMutateData_H_
#define     _GoatMutateData_H_

#include "DBaseIntList.h"


#define     FILENAME_GOATMUTATEDATA        "Data\\GoatMutateData.txt"

class   CGoatMutateData
{
    CBaseIntList        m_IntData;
    int                 *m_pData;
    int                 m_iCount;

public:
    CGoatMutateData(void);
    ~CGoatMutateData(void);

    BOOL        LoadData(void);

    int         GetGoatMutateOdds( int iGoatID );
    int         GetGoatMutateTime( int iGoatID );
};

extern  CGoatMutateData    *g_pGoatMutateData;
#endif  _GoatMutateData_H_