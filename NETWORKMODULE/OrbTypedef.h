#pragma once
#include "pub\ConstValue.h"
///////////////////////////////////////////////////////////////////////////////////////////////
// ���ݶ���ԭ��
// ��������Ķ��ǻ�ȫ���õ������ݽṹ
///////////////////////////////////////////////////////////////////////////////////////////////

enum OPR_RESULT
{
    OPR_FAIL,
    OPR_SUCCESS
};

// ��������ʽ
struct SQTeamOperation
{
    bool useBroadcast;              // �Ƿ����ȫ�������㲥������ش�
    DWORD dwSrcStaticID;            // ���������Դ����StaticID(�ش�)
    DWORD dwReserved;               // Ԥ����StaticID
};

// �������ؽ��
struct SATeamOperation
{
    struct SOpPlayer
    {
        // �����ؼ���
        DWORD   dwStaticID;

        // ��λ�ؼ���
        WORD    wServerID;          // ��������������ID
        DNID    dnidClient;         // �������ӱ��

        // У��ؼ���
        DWORD   dwClientGID;        // ����ұ��ε�¼��ȫ�ֹؼ���

        // ��������
		char	szName[CONST_USERNAME];         // ����ҵ�����[�Լ���ĩβ�ض�]
        bool    isLeader;           // ���ڳƺţ��ӳ����Ա
        WORD    wCurRegionID;       // ���ڳ�����ID
    };

    DWORD dwSrcStaticID;            // ���������Դ����StaticID(�ش�)
    DWORD dwSrcGlobalID;            // ���������Դ����GlobalID(�ش�)

    DWORD dwTeamID;                 // ��ǰ�����ID

    bool useBroadcast;              // �Ƿ����ȫ�������㲥������ش�
    BYTE byMemberNumber;            // ���صĳ�Ա����
    SOpPlayer aMembers[8];          // ���صĳ�Ա����

    SOpPlayer* GetMember(int n)
    {
        if (byMemberNumber > 8) return NULL;
        if (n > byMemberNumber-1) return NULL;
        return &aMembers[n];
    }
};

