#ifndef     _COLLECTDATADEF_H_
#define     _COLLECTDATADEF_H_

#include <windows.h>

/* 数据传输的一些规则

1、数据消息只关心数据的编号和大小，具体是什么类型，有多少个，则由采集端和收集端自己处理。
struct  DataMsg
{
    WORD    wDataID;
    DWORD   dwDataSize;
    char    Data[1];
};

2、采集端只保存当前数据，不保存历史数据，如果需要了解一段时间的数据变化，则由收集端自己保留

*/


#define     S_VER           // 专门用于只显示少量数据的版本


#define     SIZEOF_TRANSFERMSG( size )                                                      \
            (sizeof(SACollectTransferMsg)+size-1)

// 创建一个包含数据结构的CollectDataMsg
#define     NEW_TRANSFERMSG_DATA( p, size )                                                 \
            (p) = (SACollectTransferMsg*)new char[sizeof(SACollectTransferMsg)+(size)-1];   \
            (p)->SMessage::_protocol = SMessage::EPRO_COLLECT_MESSAGE;                      \
            (p)->_protocol           = SCollectBaseMsg::EPRO_COLLECT_TRANSFER;              \
            (p)->Data.dwDataSize = (size);                            

// 创建一个int的CollectDataMsg
#define     NEW_TRANSFERMSG_INT(p)                                                          \
            (p) = (SACollectTransferMsg*)new char[sizeof(SACollectTransferMsg)+4-1];        \
            (p)->SMessage::_protocol = SMessage::EPRO_COLLECT_MESSAGE;                      \
            (p)->_protocol           = SCollectBaseMsg::EPRO_COLLECT_TRANSFER;              \
            (p)->Data.dwDataSize = (4);                               

// 删除CollectDataMsg
#define     DELETE_TRANSFERMSG(p)                                                           \
            delete  (p);

// 根据数据编号，回复一个数据
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


// 传输消息中的数据结构
struct  CollectDataMsg
{

    short   wDataID;
    __int32 dwDataSize;
    char    Data[1];
};

// 采集的数据编号
enum    E_DATA_ID
{
    //DATAID_NONE,
    DATAID_TOTLEMONEY,

    DATAID_ALLLOGINDATA,    // 所有SCollect_LoginServerData中的数据
    DATAID_TOTLEMONEY_OT,   // 按时间段采集的数据

    DATAID_MAX,             // 保证连续性，用于现实对象的定义
};

// 构造函数初始化dwSize
//#define     CONSTRUCT_SIZE(stct)    \
//    stct()                          \
//    {                               \
//        dwSize = sizeof( stct );    \
//    }                               

// 采集的数据对象
struct  SCollect_LoginServerData
{
//    DWORD   dwSize;                 // 1.1以上的版本才有，为适应不同版本的数据结构

    __int64 i64TotalMoney;          // 当前存在的金钱总额,只有读取,没有存入的时候就为负数
    __int64 i64TotalSpendMoney;     // 消耗的金钱总额

    DWORD   dwTotalPlayers;         // 总的玩家数
    DWORD   dwTotalHPlayers;        // 挂机人数
    DWORD   dwMaxPlayers;           // 最大玩家人数

    DWORD   dwAutoPlayer;           // 角色在线挂机

    DWORD   dwAvePing;              // 平均ping值
    DWORD   dwInFlowPS;             // 每秒流量
    DWORD   dwOutFlowPS;            // 每秒流量
    
    // 2004-11-15
    DWORD   dwCreatedSpecialItem;   // 产生的特殊物品
    DWORD   dwUseSpecialItem;       // 消耗的特殊物品
//    CONSTRUCT_SIZE( SCollect_LoginServerData )
};

#define     BASEHISTORYDATA_VAR 1                           // 历史数据版本号基数
#ifdef  S_VER
#define     HISTORYDATA_VAR     BASEHISTORYDATA_VAR+100     // 历史数据版本号
#else
#define     HISTORYDATA_VAR     BASEHISTORYDATA_VAR         // 历史数据版本号
#endif

// 某一个时间段在某个服务器上采集的数据
struct  SLoginDataOnTimeX
{
    __int64 i64TotalMoney;            // 当前存在的金钱总额,只有读取,没有存入的时候就为负数
    __int64 i64TotalSpendMoney;      // 消耗的金钱总额

    DWORD   dwTotalPlayers;         // 总的玩家数
    DWORD   dwAvePing;              // 平均ping值
    
    //2005-03-23
    DWORD   dwTotalHPlayers;        // 挂机人数
    DWORD   dwMaxPlayers;           // 最大玩家人数
};

#endif      //_COLLECTDATADEF_H_
