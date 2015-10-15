#pragma once

#include "QuestCardPointModify.h"

struct SExchangeFullInfo
{
    // 用于交易对象的查询
    DWORD m_SrcGid;                                             // 交易发起人ID
    DWORD m_DstGid;                                             // 交易接受人ID

    // 为了保证安全性，交易过程中如果出现了连接断开，也不允许继续交易
    DNID m_SrcDnid;                                             // 交易发起人的连接编号
    DNID m_DstDnid;                                             // 交易接受人的连接编号

    // 具体的交易内容！
    SExchangeBox m_SrcEB;                                       // 交易发起人信息
    SExchangeBox m_DstEB;                                       // 交易接受人信息

    // 用于确认交易的校验信息
    SExchangeVerifyInfo m_VerifyInfo;                           // 用于确认操作的校验编号！
};

BOOL PushExchangeInfo(SExchangeVerifyInfo &vi, SExchangeFullInfo &fi);
BOOL PopExchangeInfo(SExchangeVerifyInfo &vi, SExchangeFullInfo &fi);