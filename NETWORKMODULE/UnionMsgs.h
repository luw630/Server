#pragma once
#include "NetModule.h"

DECLARE_MSG_MAP(SUnionBaseMsg, SMessage, SMessage::EPRO_UNION_MESSAGE)
//{{AFX
EPRO_UNION_CREATE,            // ��������
EPRO_UNION_INVITATORY,        // �������
EPRO_UNION_ONLINE,            // ����������Ϣ
EPRO_UNION_UPDATE,            // ���½��壨��
EPRO_GET_UNIONDATA,           // ȡ�ý�������
EPRO_SAVE_UNIONDATA,          // �����������
EPRO_UNION_GET_EXP,           // �����ȡ����
EPRO_UNION_KICK,              // �˳����
//}}AFX
END_MSG_MAP()
//=============================================================================================

// ��������
DECLARE_MSG( SSaveUnionMsgBase, SUnionBaseMsg, SUnionBaseMsg::EPRO_SAVE_UNIONDATA )
struct SQSaveUnionDataMsg : public SSaveUnionMsgBase
{
    DWORD saveTime;
	DWORD unionCount;
	DWORD dwSendSize;
	char streamData[0xe000];
};

DECLARE_MSG( SGetUnionMsgBase, SUnionBaseMsg, SUnionBaseMsg::EPRO_GET_UNIONDATA )
struct SQGetUnionDataMsg : public SGetUnionMsgBase
{
    DNID	dwZoneID;		//���������ID���������ûش�Ŀ�꣡��
};

struct SAGetUnionDataMsg : public SGetUnionMsgBase
{
    DNID dwZoneID;		//���������ID���������ûش�Ŀ�꣡��
	DWORD unionCount;
	DWORD dwSendSize;
    BOOL  readEnd;
	char streamData[0xe000];
};

//=============================================================================================
DECLARE_MSG(SCreateUnionMsg, SUnionBaseMsg, SUnionBaseMsg::EPRO_UNION_CREATE )
struct SACreateUnionMsg : public SCreateUnionMsg
{
	DWORD destID;       // ��Ŀ����ҵ�ID
};

struct SQCreateUnionMsg : public SCreateUnionMsg
{
	DWORD destID;	    // ��ĳĳ��ҽ���
    char title[9];      // ����
    char suffx[3];      // ��׺
};

//=============================================================================================
DECLARE_MSG( SInvitatoryUnionMsg, SUnionBaseMsg, SUnionBaseMsg::EPRO_UNION_INVITATORY )
struct SAInvitatoryUnionMsg : public SInvitatoryUnionMsg // ֪ͨĳ��ң�ĳ���Ҫ��Ϊ��Ϊ����
{
	DWORD playerID;       // Ŀ����ҵ�ID
	char playerName[CONST_USERNAME];
};

// ��ҷ����Ƿ�Ը���Ϊ����
struct SQInvitatoryUnionMsg : public SInvitatoryUnionMsg
{
	DWORD playerID;       // Ŀ����ҵ�ID
	bool value;
};

//=============================================================================================
DECLARE_MSG( SOnlineUnionMsg, SUnionBaseMsg, SUnionBaseMsg::EPRO_UNION_ONLINE )
struct SUnionOnlineMsg : public SOnlineUnionMsg
{
	enum FRIEND_STATE
	{
		UNION_STATE_ONLINE,	    // ����
		UNION_STATE_OUTLINE,	// ����
		UNION_STATE_HANGUP,	    // �һ�
		UNION_STATE_KILLOTHER,  // ��ܱ���
		UNION_STATE_BEKILLED,	// �����˴��
		UNION_STATE_LEVELUP,	// ����
		UNION_STATE_PASSPULSE,	// ��Ѩ
	}eState;

	char	cName[CONST_USERNAME];		 	// ����
};

//=============================================================================================
struct UnionInfo
{
	DWORD rolrid;            // Ψһ��ʶID
	DWORD exp;               // ��ǰ����
	char szName[CONST_USERNAME];         // ��ɫ��
	BYTE school;             // ����
	BYTE camp;               // ��Ӫ 1: �� 2���� 3��а
	BYTE job;                // ְλ
	char szFaceionName[CONST_USERNAME];  // ������
	bool online;             // �Ƿ�����
};
DECLARE_MSG( SUpdateUnionMsg, SUnionBaseMsg, SUnionBaseMsg::EPRO_UNION_UPDATE )
struct SAUpdateUnionMsg : public SUpdateUnionMsg
{
	enum {
		UPDATE_MEMBER,         // ���³�Ա
		UPDATE_EXP,            // ���¾���
	};

	BYTE type;
	char szName[CONST_USERNAME];
	BYTE change;
	UnionInfo unionInfo[8];
};

struct SAUpdateExpMsg : public SUpdateUnionMsg
{
	BYTE type;
	BYTE memberIdx;          // ָ����Ա
	DWORD exp;               // ����
};

//=============================================================================================

DECLARE_MSG( SKickUnionMsg, SUnionBaseMsg, SUnionBaseMsg::EPRO_UNION_KICK )
struct SAKickUnionMsg : public SKickUnionMsg
{
    enum
    {
        KICK_ME,          // T���Լ�
        KICK_OTHER,       // T�������ֵ�
    };

    DWORD  dwPlayerID;    // ��T���ID
	char   szName[CONST_USERNAME];  // ��T��ҽ�ɫ��
    DWORD  dwBossID;      // �ϴ��ID
    BYTE   byType;
};

struct SQKickUnionMsg : public SKickUnionMsg
{
    DWORD  dwPlayerID;    // Ŀ�����ID
};






