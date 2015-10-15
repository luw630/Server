
#ifndef     _DBDATACOLLECT_H_
#define     _DBDATACOLLECT_H_

#include "NetWorkModule/NetCollecterModule/DataTransfer.h"
#include <map>
using namespace std;

#include "pub\dwt.h"
#include "..\pub\ConstValue.h"


class   CLoginDataCollect : public CDataTransfer
{
public:
    // 采集数据纪录
    SCollect_LoginServerData   m_Data;

    struct  SSaveMoneyState
    {
        bool    bAdd;
        int     iMoney;
    };
	std::map< dwt::stringkey<char[CONST_USERNAME]>, SSaveMoneyState > m_SwitchMap;
public:
    CLoginDataCollect(void);
    ~CLoginDataCollect(void);

    void    OnRecvRequestData( int iDataType, DNID dnid );

	void    AddTotalMoney(bool bAdd, dwt::stringkey<char[CONST_USERNAME]> name, int iMoney);
    //void    SetSpendMoney( DWORD dwMoney );
    //void    SetAvrgPing( DWORD dwAvrgPing );
};

#endif      //_DBDATACOLLECT_H_