#include "stdafx.h"
#include "LoginDataCollect.h"
#include "networkmodule/netmodule.h"


CLoginDataCollect::CLoginDataCollect(void)
{
    memset( &m_Data, 0, sizeof(m_Data) );

    //for test
    //m_Data.dwCreatedSpecialItem = 987;
    //m_Data.dwUseSpecialItem     = 644;
}

CLoginDataCollect::~CLoginDataCollect(void)
{
}
//////////////////////////////////////////////////////
void    CLoginDataCollect::OnRecvRequestData( int iDataType, DNID dnid )
{
    switch( iDataType )
    {
    case DATAID_TOTLEMONEY:
        REPLY_REQUESTDATA( dnid, iDataType, &m_Data.i64TotalMoney, int, 1 );
        break;

    case DATAID_ALLLOGINDATA:
        REPLY_REQUESTDATA( dnid, iDataType, &m_Data, SCollect_LoginServerData, 1 );
        break;

    case DATAID_TOTLEMONEY_OT:
        {
        SLoginDataOnTimeX   OnTimeXData;
        OnTimeXData.dwAvePing           = m_Data.dwAvePing;
        OnTimeXData.dwTotalPlayers      = m_Data.dwTotalPlayers;
        OnTimeXData.i64TotalSpendMoney  = m_Data.i64TotalSpendMoney;
        OnTimeXData.i64TotalMoney       = m_Data.i64TotalMoney;

        OnTimeXData.dwTotalHPlayers     = m_Data.dwTotalHPlayers;
        OnTimeXData.dwMaxPlayers        = m_Data.dwMaxPlayers;

        REPLY_REQUESTDATA( dnid, iDataType, &OnTimeXData, SLoginDataOnTimeX, 1 );
        }
        break;
    }
}

/////////////////////////////////////////////////////////////////////// ��������
void    CLoginDataCollect::AddTotalMoney(bool bAdd, dwt::stringkey<char[CONST_USERNAME]> name, int iMoney)
{
    SSaveMoneyState     SaveState;
	std::map< dwt::stringkey<char[CONST_USERNAME]>, SSaveMoneyState>::iterator it;

    it = m_SwitchMap.find( name );
    if( it!=m_SwitchMap.end() ) // �Ѵ��ڵ�
    {
        SaveState = it->second;
        if( SaveState.bAdd!= bAdd )
        {
            if( bAdd )
                m_Data.i64TotalMoney += (__int64)iMoney;    // ��һ�λ�Ǯ
            else
                m_Data.i64TotalMoney -= (__int64)iMoney;    // �Ƴ���������Ǯ

            //SaveState.bAdd  = bAdd;
            //SaveState.iMoney= iMoney;
            //m_SwitchMap[name] = SaveState;
            it->second.bAdd  = bAdd;
            it->second.iMoney= iMoney;
        }
        else    if( !SaveState.bAdd )                       // û�л�Ǯ������Ǯ�������
        {
        }
        else    // ��n�λ�Ǯ
        {
            m_Data.i64TotalMoney += (__int64)(iMoney-SaveState.iMoney);// ��n�λ�Ǯ
            
            it->second.bAdd  = bAdd;
            it->second.iMoney= iMoney;
        }
    }
    else                        // �¼����
    {
        if( !bAdd )
        {
            m_Data.i64TotalMoney -= (__int64)iMoney;

            SaveState.bAdd  = bAdd;
            SaveState.iMoney= iMoney;
			m_SwitchMap.insert(std::map<dwt::stringkey<char[CONST_USERNAME]>, SSaveMoneyState >::value_type(name, SaveState));
        }
    }
}

//void    CLoginDataCollect::SetSpendMoney( DWORD dwMoney )
//{
//    m_Data.dwTotalSpendMoney = dwMoney;
//}
//
//void    CLoginDataCollect::SetAvrgPing( DWORD dwAvrgPing )
//{
//    m_Data.dwAvePing = dwAvrgPing;
//}

