#pragma once

#include "NetModule.h"
#include "movetypedef.h"

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// �ƶ���Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SMoveBaseMsg, SMessage, SMessage::EPRO_MOVE_MESSAGE)
EPRO_SYN_WAYTRACK,          // ͬ���ƶ�·����Ϣ
EPRO_SYN_POSITION,          // У������������Ϣ
EPRO_SET_ZAZEN,             // ����/�����Ϣ
EPRO_TEST_POSITION,         // ������
EPRO_SYN_WAYJUMP,           // ��Ծ��Ϣ
EPRO_TITLECHANGED,          // �ƺŸı�
EPRO_NAMECHANGED,           // ���ָı�
// add by yuntao.liu
EPRO_UP_SPEED,				// �����ٶ�
EPRO_NOTIFY_MOVE,	
EPRO_LOCK_PLAYER,

EPRO_SYN_PATH,				// �����3ͬ���ƶ�·����Ϣ
EPRO_SYN_POS,				// �����3ͬ��λ����Ϣ
EPRO_SYN_Z,					// �����3ͬ���߶���Ϣ
EPRO_SYN_JUMP,				// �����3ͬ����Ծ��Ϣ

EPRO_TASK_MOVE,		// �����3ͬ����������Ϣ
EPRO_SYN_UNITY3DJUMP, // UNITY3D���������Ϣͬ��

EPRO_SYN_PATHMONSTER,				// �����3ͬ���ƶ�����·����Ϣ
END_MSG_MAP()
//---------------------------------------------------------------------------------------------

DECLARE_MSG(STitleChangedMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_TITLECHANGED)
struct  SQTitleChangedMsg    :   public STitleChangedMsg
{
	char cChangedTitle[CONST_USERNAME];
};
struct  SATitleChangedMsg    :   public STitleChangedMsg
{
	char cChangedTitle[CONST_USERNAME];
    DWORD dwPlayerChangedID;
};

DECLARE_MSG(SSynJumpMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_JUMP)
struct SAQSynJumpMsg : public SSynJumpMsg
{
	DWORD	dwGlobalID;
	float	x;
	float	y;
};

DECLARE_MSG(SSynUnity3DJumpMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_UNITY3DJUMP)
struct SAQUnity3DJumpMsg: public SSynUnity3DJumpMsg
{
	DWORD	dwGlobalID;
};

//=============================================================================================
// ��Ծ��Ϣ
DECLARE_MSG(SSynWayJumpMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_WAYJUMP)
struct  SQASynWayJumpMsg    :   public SSynWayJumpMsg
{
    DWORD   dwGlobalID;         // ��������ȫ��Ψһ��ʶ��
    WORD    wCurX, wCurY;       // �ý�ɫ�ĵ�ǰ����(�Է��ͷ�������Ϊ׼)
    SNWayNode   NextPos;        // Ҫ������Ŀ���
};
//---------------------------------------------------------------------------------------------

// �����3�ߣ��ܣ���Ϣ
DECLARE_MSG(SSynPathMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_PATH)
struct SQSynPathMsg : public SSynPathMsg
{
	enum { normal = 0, attack };	// �ƶ�Ŀ��
	
	DWORD			m_GID;			// ���ID
	BYTE			m_move2What;	// �ƶ�Ŀ��
	SPath			m_Path;			// �ƶ�·��������Ϊ���һ����Ա
};

struct SASynPathMsg : public SSynPathMsg
{
	DWORD			m_GID;			// ���ID
	SPath			m_Path;			// �ƶ�·��������Ϊ���һ����Ա
};

// �����3ͬ��λ����Ϣ
DECLARE_MSG(SSynPosMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_POS)
struct SQSynPosMsg : public SSynPosMsg
{
	DWORD			m_GID;			// ���ID
	float			m_X;			// ��ǰ�����꼰����
	float			m_Y;
	float			m_Z;
	float			m_Dir;			
	BYTE			m_Action;		// ����
};

typedef SQSynPosMsg	SASynPosMsg;

// �����3ͬ���߶���Ϣ��ֻ��Ҫ�ͻ��˷����������˽���ͬ����
DECLARE_MSG(SSynZMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_Z)
struct SQSynZMsg : public SSynZMsg
{
	DWORD			m_GID;			// ���ID
	float			m_Z;			// �߶�
};

//=============================================================================================
// �ߣ��ܣ�����Ϣ
DECLARE_MSG(SSynWayTrackMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_WAYTRACK)
// �ͻ�������������ƶ�
struct SQSynWayTrackMsg : public SSynWayTrackMsg 
{
    DWORD   dwGlobalID;         // ��������ȫ��Ψһ��ʶ��
    SSynWay ssw;                // ͬ���ƶ�·��

	enum
	{
		normal = 0,
		move_2_attack
	};
	INT32 move2What;
};

// ������Ҫ��ͻ����ƶ�
struct SASynWayTrackMsg : public SSynWayTrackMsg
{
    DWORD   dwGlobalID;         // ��������ȫ��Ψһ��ʶ��
    SSynWay ssw;                // ͬ���ƶ�·��
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// ����/�����Ϣ
DECLARE_MSG(SSetZazenMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SET_ZAZEN)
//  ����Ϣֻ��server����client��Ҫ��ͻ��˸��ݷ���������ͬ����ǰλ��״̬
struct SQSetZazenMsg: public SSetZazenMsg
{
    DWORD dwGlobalID;           // ��������ȫ��Ψһ��ʶ��
    BYTE  byAction;             // ��Ϊ��ʽ
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// ͬ��λ����Ϣ
DECLARE_MSG(SSynPositionMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_POSITION)
//  ����Ϣֻ��server����client��Ҫ��ͻ��˸��ݷ���������ͬ����ǰλ��״̬
struct SASynPositionMsg: public SSynPositionMsg
{
    DWORD dwGlobalID;           // ��������ȫ��Ψһ��ʶ��
    WORD  wCurX, wCurY;         // ��ǰ����
    BYTE  byAction;             // ��Ϊ��ʽ
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// Test Msg
DECLARE_MSG(STestPos, SMoveBaseMsg, SMoveBaseMsg::EPRO_TEST_POSITION)
struct SATP : public STestPos
{
    WORD wCurX, wCurY;
    WORD iMsgCount;
    BYTE byCurAction;
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SUpSpeed_S2C, SMoveBaseMsg, SMoveBaseMsg::EPRO_UP_SPEED)
struct SUpSpeed_S2C_MsgBody : public SUpSpeed_S2C
{
	INT32 objectGID;
	float newSpeed;
};

//----------------------------------------------------------------------------------------------------------	
DECLARE_MSG(SNotifyMove_S2C, SMoveBaseMsg, SMoveBaseMsg::EPRO_NOTIFY_MOVE)
struct SNotifyMove_S2C_MsgBody : public SNotifyMove_S2C
{
	INT32 moveTileX;
	INT32 moveTileY;
};

DECLARE_MSG(SLockPlayer, SMoveBaseMsg, SMoveBaseMsg::EPRO_LOCK_PLAYER)
struct SALockPlayer : public SLockPlayer
{
	BYTE byLock;
};

// �����3������������Ϣ
DECLARE_MSG(Staskmove,SMoveBaseMsg, SMoveBaseMsg::EPRO_TASK_MOVE)
struct SQtaskmove : public Staskmove
{
	WORD taskID;		//�����ƶ�����NPC����ID
	WORD wCellPos;		//ʹ�õĴ��ͷ��ڱ�����λ��
};

//������Ҫ��ͻ�������һ��Ѱ·��·����
DECLARE_MSG(SSynPathMonsterMsg, SMoveBaseMsg, SMoveBaseMsg::EPRO_SYN_PATHMONSTER)
struct SASynPathMonsterMsg : public SSynPathMonsterMsg
{
	SWayPoint	m_path[2];		// ����ԭ�غ��ƶ�����Ŀ���
	DWORD  dwGlobalID;           // ��������ȫ��Ψһ��ʶ��
};

//�ͻ��˻ظ�·����Ϣ
struct SQSynPathMonsterMsg : public SSynPathMonsterMsg
{
	DWORD dwGlobalID;           // ��������ȫ��Ψһ��ʶ��
	SPath			m_Path;			// �ƶ�·��
};

