#ifndef     _COLLECTDATADEF_H_
#define     _COLLECTDATADEF_H_

#include <windows.h>

/* ���ݴ����һЩ����

1��������Ϣֻ�������ݵı�źʹ�С��������ʲô���ͣ��ж��ٸ������ɲɼ��˺��ռ����Լ�����
struct  DataMsg
{
    WORD    wDataID;
    DWORD   dwDataSize;
    char    Data[1];
};

2���ɼ���ֻ���浱ǰ���ݣ���������ʷ���ݣ������Ҫ�˽�һ��ʱ������ݱ仯�������ռ����Լ�����

*/


#define     S_VER           // ר������ֻ��ʾ�������ݵİ汾


#define     SIZEOF_TRANSFERMSG( size )                                                      \
            (sizeof(SACollectTransferMsg)+size-1)

// ����һ���������ݽṹ��CollectDataMsg
#define     NEW_TRANSFERMSG_DATA( p, size )                                                 \
            (p) = (SACollectTransferMsg*)new char[sizeof(SACollectTransferMsg)+(size)-1];   \
            (p)->SMessage::_protocol = SMessage::EPRO_COLLECT_MESSAGE;                      \
            (p)->_protocol           = SCollectBaseMsg::EPRO_COLLECT_TRANSFER;              \
            (p)->Data.dwDataSize = (size);                            

// ����һ��int��CollectDataMsg
#define     NEW_TRANSFERMSG_INT(p)                                                          \
            (p) = (SACollectTransferMsg*)new char[sizeof(SACollectTransferMsg)+4-1];        \
            (p)->SMessage::_protocol = SMessage::EPRO_COLLECT_MESSAGE;                      \
            (p)->_protocol           = SCollectBaseMsg::EPRO_COLLECT_TRANSFER;              \
            (p)->Data.dwDataSize = (4);                               

// ɾ��CollectDataMsg
#define     DELETE_TRANSFERMSG(p)                                                           \
            delete  (p);

// �������ݱ�ţ��ظ�һ������
#define     REPLY_REQUESTDATA( _dnid, _dataid, _pdata, _datastruct, _count )                    \
            {                                                                                   \
            SACollectTransferMsg    *_pMsg = NULL;                                              \
            NEW_TRANSFERMSG_DATA( _pMsg, sizeof(_datastruct)*(_count) );                        \
            if( _pMsg )                                                                         \
            {                                                                                   \
                _pMsg->Data.wDataID      = _dataid;                                             \
                _datastruct   *_pData = (_datastruct*)_pMsg->Data.Data;                         \
                memcpy( _pData, _pdata, sizeof(_datastruct)*(_count) );                         \
                SendMessage( _dnid, _pMsg, SIZEOF_TRANSFERMSG( sizeof(_datastruct)*(_count) ) );\
                DELETE_TRANSFERMSG( _pMsg );                                                    \
            }                                                                                   \
            }


// ������Ϣ�е����ݽṹ
struct  CollectDataMsg
{

    short   wDataID;
    __int32 dwDataSize;
    char    Data[1];
};

// �ɼ������ݱ��
enum    E_DATA_ID
{
    //DATAID_NONE,
    DATAID_TOTLEMONEY,

    DATAID_ALLLOGINDATA,    // ����SCollect_LoginServerData�е�����
    DATAID_TOTLEMONEY_OT,   // ��ʱ��βɼ�������

    DATAID_MAX,             // ��֤�����ԣ�������ʵ����Ķ���
};

// ���캯����ʼ��dwSize
//#define     CONSTRUCT_SIZE(stct)    \
//    stct()                          \
//    {                               \
//        dwSize = sizeof( stct );    \
//    }                               

// �ɼ������ݶ���
struct  SCollect_LoginServerData
{
//    DWORD   dwSize;                 // 1.1���ϵİ汾���У�Ϊ��Ӧ��ͬ�汾�����ݽṹ

    __int64 i64TotalMoney;          // ��ǰ���ڵĽ�Ǯ�ܶ�,ֻ�ж�ȡ,û�д����ʱ���Ϊ����
    __int64 i64TotalSpendMoney;     // ���ĵĽ�Ǯ�ܶ�

    DWORD   dwTotalPlayers;         // �ܵ������
    DWORD   dwTotalHPlayers;        // �һ�����
    DWORD   dwMaxPlayers;           // ����������

    DWORD   dwAutoPlayer;           // ��ɫ���߹һ�

    DWORD   dwAvePing;              // ƽ��pingֵ
    DWORD   dwInFlowPS;             // ÿ������
    DWORD   dwOutFlowPS;            // ÿ������
    
    // 2004-11-15
    DWORD   dwCreatedSpecialItem;   // ������������Ʒ
    DWORD   dwUseSpecialItem;       // ���ĵ�������Ʒ
//    CONSTRUCT_SIZE( SCollect_LoginServerData )
};

#define     BASEHISTORYDATA_VAR 1                           // ��ʷ���ݰ汾�Ż���
#ifdef  S_VER
#define     HISTORYDATA_VAR     BASEHISTORYDATA_VAR+100     // ��ʷ���ݰ汾��
#else
#define     HISTORYDATA_VAR     BASEHISTORYDATA_VAR         // ��ʷ���ݰ汾��
#endif

// ĳһ��ʱ�����ĳ���������ϲɼ�������
struct  SLoginDataOnTimeX
{
    __int64 i64TotalMoney;            // ��ǰ���ڵĽ�Ǯ�ܶ�,ֻ�ж�ȡ,û�д����ʱ���Ϊ����
    __int64 i64TotalSpendMoney;      // ���ĵĽ�Ǯ�ܶ�

    DWORD   dwTotalPlayers;         // �ܵ������
    DWORD   dwAvePing;              // ƽ��pingֵ
    
    //2005-03-23
    DWORD   dwTotalHPlayers;        // �һ�����
    DWORD   dwMaxPlayers;           // ����������
};

#endif      //_COLLECTDATADEF_H_
