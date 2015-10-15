#pragma once
#include "../pub/ConstValue.h"
struct SExchangeVerifyInfo
{
    DWORD m_dwTime;                                 // 交易启动时间用的是time_t，实际上大小为32bit，因为这里直接使用了DWORD，所以不可移植！
    int m_zoneid;                                   // 记录操作位于哪个区域服务器
    int m_loginid;                                  // 初始化操作位于那个登陆服务器
    std::string ExchangeSrcAccount;                 // 交易发起人的账号
    std::string ExchangeDstAccount;                 // 交易接受人的账号

    bool operator < (const SExchangeVerifyInfo &other) const
    {
        if (m_dwTime < other.m_dwTime)
            return true;

        if (ExchangeSrcAccount < other.ExchangeSrcAccount)
            return true;

        if (ExchangeDstAccount < other.ExchangeDstAccount)
            return true;

        return false;
    }
};

BOOL QuestCardPointModify(SExchangeVerifyInfo &vi,                      // 交易校验信息
                          std::string &SrcAccount,                      // 点数交易的源账号
                          std::string &DstAccount,                      // 点数交易的目标账号
                          DWORD dwPoint, DWORD SrcGID, DWORD DstGID);   // 具体的交易点数

BOOL QuestConfirmPointModify(SExchangeVerifyInfo &vi,               // 交易校验信息
                          std::string &SrcAccount,                  // 点数交易的源账号
                          std::string &DstAccount,                  // 点数交易的目标账号
                          DWORD dwPoint);                           // 具体的交易点数

BOOL QuestCancelPointModify(SExchangeVerifyInfo &vi, 
                          std::string &SrcAccount, 
                          std::string &DstAccount, 
                          DWORD dwPoint);

/////////////////////////////////////////////////////////////////////////////
// 元宝积分相关消息操作也放在这儿
BOOL NotifyMoneyPointToLogin(const char *szAccount, DWORD dwOperate, DWORD dwMoneyPoint); 