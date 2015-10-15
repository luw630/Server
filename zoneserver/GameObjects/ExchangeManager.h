#pragma once

#include "QuestCardPointModify.h"

struct SExchangeFullInfo
{
    // ���ڽ��׶���Ĳ�ѯ
    DWORD m_SrcGid;                                             // ���׷�����ID
    DWORD m_DstGid;                                             // ���׽�����ID

    // Ϊ�˱�֤��ȫ�ԣ����׹�����������������ӶϿ���Ҳ�������������
    DNID m_SrcDnid;                                             // ���׷����˵����ӱ��
    DNID m_DstDnid;                                             // ���׽����˵����ӱ��

    // ����Ľ������ݣ�
    SExchangeBox m_SrcEB;                                       // ���׷�������Ϣ
    SExchangeBox m_DstEB;                                       // ���׽�������Ϣ

    // ����ȷ�Ͻ��׵�У����Ϣ
    SExchangeVerifyInfo m_VerifyInfo;                           // ����ȷ�ϲ�����У���ţ�
};

BOOL PushExchangeInfo(SExchangeVerifyInfo &vi, SExchangeFullInfo &fi);
BOOL PopExchangeInfo(SExchangeVerifyInfo &vi, SExchangeFullInfo &fi);