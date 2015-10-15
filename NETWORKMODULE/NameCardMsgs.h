#pragma once

#include "NetModule.h"
#include "playertypedef.h"

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ��Ƭ���
//=============================================================================================
DECLARE_MSG_MAP(SNameCardBaseMsg, SMessage, SMessage::EPRO_NAMECARD_BASE)
//{{AFX
EPRO_NAMECARD_QUESTINFO,    // �����ȡһ����ҵ���Ƭ����
EPRO_NAMECARD_UPDATEINFO,   // ����ˢ����Ƭ���ݣ������Լ��ĺͱ��˵ģ�
EPRO_NAMECARD_SETMASK,      // ����Ϳ����Ƭ�����ݣ�ֻ�����Լ��ģ�
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================
// �����ȡһ����ҵ���Ƭ����
DECLARE_MSG(SNameCardQuestInfoMsg, SNameCardBaseMsg, SNameCardBaseMsg::EPRO_NAMECARD_QUESTINFO)
struct SQNameCardQuestInfoMsg : 
    public SNameCardQuestInfoMsg
{
    DWORD dwDestGID;    // Ŀ���GID
    DWORD dwSelfGID;    // �Լ���GID
};

struct SANameCardQuestInfoMsg : 
    public SNameCardQuestInfoMsg
{
    enum ERetCode
    {
        ERC_OK,
        ERC_SERVER_DISCARD,
    };

    BYTE byRetCode;

    DWORD dwDestGID;    // Ŀ���GID

    // SPlayerComment Info;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ����ˢ����Ƭ���ݣ������Լ��ĺͱ��˵ģ�
DECLARE_MSG(SNameCardUpdateInfoMsg, SNameCardBaseMsg, SNameCardBaseMsg::EPRO_NAMECARD_UPDATEINFO)
struct SQNameCardUpdateInfoMsg : 
    public SNameCardUpdateInfoMsg
{
    DWORD dwDestGID;    // Ŀ���GID

    DWORD dwSelfGID;    // �Լ���GID

    BYTE byInfoNumber;  // �޸ĵ�Ŀ�ı�� 1�����ҽ��� 2��·������ 3���������� 4����������

    char szSelfIntroduction[200]; // �޸�����
};

struct SANameCardUpdateInfoMsg : 
    public SNameCardUpdateInfoMsg
{
    enum ERetCode
    {
        ERC_OK,
        ERC_SERVER_DISCARD,
    };

    BYTE byRetCode;

    DWORD dwSrcGID;                 // ԴGID ���޸ĵ��ˣ�

    DWORD dwDestGID;                // Ŀ���GID �����޸ĵ��ˣ�

    BYTE byInfoNumber;              // �޸ĵ�Ŀ�ı�� 1�����ҽ��� 2��·������ 3���������� 4����������

    BYTE byIndex;                   // ������Ϊ ��·������/�������ۣ���ʱ��ָ����λ��

    SYSTEMTIME UpdateTime;          // ��ǰ���޸�ʱ��

    char szSelfIntroduction[200];   // �޸ĵ�����
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ����Ϳ����Ƭ�����ݣ�ֻ�����Լ��ģ�
DECLARE_MSG(SNameCardSetMaskMsg, SNameCardBaseMsg, SNameCardBaseMsg::EPRO_NAMECARD_SETMASK)
struct SQNameCardSetMaskMsg : 
    public SNameCardSetMaskMsg
{
    DWORD dwSelfGID;    // �Լ���GID

    BYTE byInfoNumber;  // �޸ĵ�Ŀ�ı�� 1�����ҽ��� 2��·������ 3���������� 4����������

    BYTE byMaskNumber;  // �ڼ�����
};

struct SANameCardSetMaskMsg : 
    public SNameCardSetMaskMsg
{
    enum ERetCode
    {
        ERC_OK,
        ERC_SERVER_DISCARD,
    };

    BYTE byRetCode;

    BYTE byMaskNumber;  // �ڼ�����

    BYTE byInfoNumber;  // �޸ĵ�Ŀ�ı�� 1�����ҽ��� 2��·������ 3���������� 4����������

    BYTE byLeftMask;    // ��ʣ�µ�Mask����
};
//---------------------------------------------------------------------------------------------
