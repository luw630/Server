#pragma once
#include "../pub/ConstValue.h"
struct SExchangeVerifyInfo
{
    DWORD m_dwTime;                                 // ��������ʱ���õ���time_t��ʵ���ϴ�СΪ32bit����Ϊ����ֱ��ʹ����DWORD�����Բ�����ֲ��
    int m_zoneid;                                   // ��¼����λ���ĸ����������
    int m_loginid;                                  // ��ʼ������λ���Ǹ���½������
    std::string ExchangeSrcAccount;                 // ���׷����˵��˺�
    std::string ExchangeDstAccount;                 // ���׽����˵��˺�

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

BOOL QuestCardPointModify(SExchangeVerifyInfo &vi,                      // ����У����Ϣ
                          std::string &SrcAccount,                      // �������׵�Դ�˺�
                          std::string &DstAccount,                      // �������׵�Ŀ���˺�
                          DWORD dwPoint, DWORD SrcGID, DWORD DstGID);   // ����Ľ��׵���

BOOL QuestConfirmPointModify(SExchangeVerifyInfo &vi,               // ����У����Ϣ
                          std::string &SrcAccount,                  // �������׵�Դ�˺�
                          std::string &DstAccount,                  // �������׵�Ŀ���˺�
                          DWORD dwPoint);                           // ����Ľ��׵���

BOOL QuestCancelPointModify(SExchangeVerifyInfo &vi, 
                          std::string &SrcAccount, 
                          std::string &DstAccount, 
                          DWORD dwPoint);

/////////////////////////////////////////////////////////////////////////////
// Ԫ�����������Ϣ����Ҳ�������
BOOL NotifyMoneyPointToLogin(const char *szAccount, DWORD dwOperate, DWORD dwMoneyPoint); 