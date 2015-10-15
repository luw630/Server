#pragma once

#include "NetModule.h"
#include "itemtypedef.h"
#include "chattypedef.h"
#include "../pub/ConstValue.h"

#define MAX_TEAM_MEMBER		(5)
#define MAX_TEMPPACK_SIZE	(20)

DECLARE_MSG_MAP(STeamBaseMsgG, SMessage, SMessage::EPRO_TEAM_MESSAGE)
//{{AFX
EPRO_TEAM_CHANGE_SKILL = 20,	// ���ｫ��ʼ��ֵ��Ϊ20��Ϊ�˺;ɵ���Ϣ���뿪,��֤���㲻����,Ҳ�������...
EPRO_TEAM_REFRESH,				// ˢ�¶�������
EPRO_TEAM_JOIN,					// �������
EPRO_TEAM_KICK,					// ����/�뿪/��ɢ
EPRO_TEAM_TALK,					// ��������
EPRO_TEAM_HEADMEN,				// ���öӳ�
EPRO_TEAM_POSITION,				// ���Ͷ�Ա������
EPRO_CANCEL_REPLY,				// ��һЩ����µ�ȡ���������
EPRO_TEAM_SUBMIT_REQUEST,		// �ύ�������
EPRO_TEAM_GETTEAMQUEST,			// ��ȡ���������Ϣ
EPRO_TEAM_OLDQUESTLIST,			// ֪ͨ�ͻ�����Щ��������ѹ�ʱ
EPRO_TEAM_NEWQUESTLIST,			// ֪ͨ�ͻ�����Щ��������Ѹ���
EPRO_TEAM_GETFINDTEAMINFO,		// ��ȡ������Ϣ
EPRO_TEAM_OLDFINDLIST,			// ֪ͨ�ͻ�����Щ�����ѹ�ʱ
EPRO_TEAM_NEWFINDLIST,			// ֪ͨ�ͻ�����Щ����������
EPRO_TEAM_INVITEWITHNAME,		// ���������飨ͨ�����֣�
EPRO_TEAM_IWANTTOJION,			// �����б��У������
EPRO_TEAM_CANCELTEAMQUEST,		// ȡ���Լ����������
EPRO_TEAM_CHANGEGIVEMODE,		// �ı����ģʽ/��ɫ
EPRO_TEAM_ADDTEMPITEM,			// ���ӵ��ߵ��ӳ��������
EPRO_TEAM_DELTEMPITEM,			// �Ӷӳ��������ɾ��һ������
EPRO_TEAM_DISPATCHTOMEMBER,		// ����ĳ�����߸���Ա
//}}AFX
END_MSG_MAP()

class CPlayer;

struct Team						// ������Ϣ
{
	enum	// ���䷽ʽ
	{
		TGM_TEAMLEADER = 1,		// �ӳ�����
		TGM_SCHOOL,				// ���ɷ���
		TGM_FREE,				// ���ɷ���
	};

	enum	// ����Ʒ�����͵���һ�£�
	{
		TGC_BLUE = 3,			// ��ɫ
		TGC_ZISE,				// ��ɫ
		TGC_ORANAGE,			// ��ɫ
		TGC_GOLD,				// ��ɫ
	};

	struct TeamPlayer					// ��Ա��Ϣ�����ڿͻ�����ʾ���֣�
	{
		char	szName[CONST_USERNAME];	// ����
		WORD	bySex		: 1;		// �Ա�
		WORD	bySchool	: 3;		// ����
		WORD	byLevel		: 9;		// �ȼ�
		WORD	isLeader	: 1;		// �Ƿ�ӳ�
		WORD	status		: 2;		// ״̬
		
		QWORD	wMaxHp		: 24;		// ���Ѫ��
		QWORD	wCurHp		: 24;		// ��ǰѪ��
		QWORD	wMaxMp		: 24;		// ���MP
		QWORD	wCurMp		: 24;		// ��ǰMP

		DWORD	dwGlobal;				// ID
		BYTE	bFaceID;				// ��ģ��
		BYTE	bHairID;				// ͷ��ģ��
		WORD	wRegionID;				// ��ͼID
		DWORD	m_3DEquipID[11];			// 3Dװ���Ҽ���Ʒ��
	};

	struct TeamPlayerData : public TeamPlayer
	{
		CPlayer *PlayerRef;				// ��������
		DWORD	staticId;				// ��̬ID
		DNID	dnidPlayer;				// ����ID
	};

	BYTE			byGiveMode;			// ���䷽ʽ
	BYTE			byGiveColor;		// ����Ʒ��
	BYTE			bySend;				// �Ƿ���Ҫ����
	BYTE			byMemberNum;		// ��Ա����
	DWORD			LeaderGID;			// �ӳ���GID
	DWORD           ParentReginID;      // ֻ��¼��������ID
	DWORD			PartregionID;		// ��������ű�ID
	TeamPlayerData	stTeamPlayer[MAX_TEAM_MEMBER];

	// �ӳ��������
	struct LeaderPackItem
	{
		SRawItemBuffer	item;
		BYTE			pos;
	};

	SRawItemBuffer	LeaderPack[MAX_TEMPPACK_SIZE];
	WORD			PackCount;

	// �������ַ�������λ��
	int FindNum(const char *name)
	{
		for (int n = 0; n < MAX_TEAM_MEMBER; n++)
		{
			if (dwt::strcmp(stTeamPlayer[n].szName, name, CONST_USERNAME) == 0)
				return n;
		}

		return -1;
	}

	// �������ַ�������λ��
	int FindNum(DWORD sid)
	{
		for (int n = 0; n < MAX_TEAM_MEMBER; n++)
		{	
			if (stTeamPlayer[n].staticId == sid)
				return n;
		}

		return -1;
	}
};

struct FindTeamInfo
{
	struct SmallTeamPlayer
	{
		char	szName[CONST_USERNAME];		// ����
		WORD	bySex		: 1;			// �Ա�
		WORD	bySchool	: 3;			// ����
		WORD	byLevel		: 9;			// �ȼ�
		WORD	isLeader	: 1;			// �Ƿ�ӳ�
		WORD	status		: 2;			// ״̬
	};
	
	DWORD			teamID;
	SmallTeamPlayer	teamMember[MAX_TEAM_MEMBER];
};

struct TeamRequest
{
	enum
	{
		PTR_MAXINFO = 41,
	};

	DWORD	dwGID;						// ID
	WORD	bySex		: 1;			// �Ա�
	WORD	bySchool	: 3;			// ����
	WORD	byLevel		: 9;			// �ȼ�
	WORD	reserve		: 3;			// ����
	char	szName[CONST_USERNAME];		// �����
	char	szInfo[PTR_MAXINFO];		// ��ע
};

struct TRWithIndex : public TeamRequest
{
	DWORD	dwIndex;
};

struct TeamMemberPos
{
	DWORD	dwGID;
	float	fX;
	float	fY;
};

DECLARE_MSG(SAddTempItemMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_ADDTEMPITEM)
struct SAAddTempItemMsg : public SAddTempItemMsg
{
	BYTE					count;
	Team::LeaderPackItem	tempItems[MAX_TEMPPACK_SIZE];
	
	WORD MySize()
	{
		return sizeof(SAAddTempItemMsg) - (MAX_TEMPPACK_SIZE-count) * sizeof(Team::LeaderPackItem);
	}
};

DECLARE_MSG(SDelTempItemMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_DELTEMPITEM)
struct SADelTempItemMsg : public SDelTempItemMsg
{
	BYTE ItemIndex;		// ɾ���ĵ�������
};

DECLARE_MSG(SDispatchToMemberMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_DISPATCHTOMEMBER)
struct SQDispatchToMemberMsg : public SDispatchToMemberMsg
{
	BYTE ItemIndex;		// ��������
	BYTE MemIndex;		// ��Ա����
};

struct SADispatchToMemberMsg : public SDispatchToMemberMsg
{
	enum
	{
		SDTM_SUCCESS,	// �ɹ�
		SDTM_PACKFULL,	// ��Ա��������
	};
	BYTE Result;		// ���
	BYTE MemIndex;		// ��Ա����
};

DECLARE_MSG(SChangeGiveMode, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_CHANGEGIVEMODE)
struct SQAChangeGiveMode : public SChangeGiveMode
{
	enum
	{
		SCGM_MODE,		// ���䷽ʽ
		SCGM_COLOER,	// ����Ʒ��
	};
	BYTE	bOperation;
	BYTE	bMode;
};

DECLARE_MSG(SCancelTeamQuest, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_CANCELTEAMQUEST)
struct SQCancelTeamQuest : public SCancelTeamQuest
{
};

DECLARE_MSG(SWantJoinTeamMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_IWANTTOJION)
struct SQWantJoinTeamMsg : public SWantJoinTeamMsg
{
	DWORD dwTeamID;
};

DECLARE_MSG(SInviteWithNameMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_INVITEWITHNAME)
struct SQInviteWithNameMsg : public SInviteWithNameMsg
{
	char szName[CONST_USERNAME];
};

DECLARE_MSG(SFindTeamInfoMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_GETFINDTEAMINFO)
struct SQFindTeamInfoMsg : public SFindTeamInfoMsg
{
};

DECLARE_MSG(SNotifyOldFindMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_OLDFINDLIST)
struct SANotifyOldFindMsg : public SNotifyOldFindMsg
{
	enum { NOFM_OLDMAX = 200, };

	WORD	wNumber;
	DWORD	dwOldList[NOFM_OLDMAX];

	DWORD GetMySize()
	{
		return (sizeof(SANotifyOldFindMsg) - ((NOFM_OLDMAX-wNumber)*sizeof(DWORD)));
	}
};

DECLARE_MSG(SNotifyNewFindMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_NEWFINDLIST)
struct SANotifyNewFindMsg : public SNotifyNewFindMsg
{
	enum { NOFM_NEWMAX = 200, };

	WORD			wNumber;
	FindTeamInfo	dwNewList[NOFM_NEWMAX];

	DWORD GetMySize()
	{
		return (sizeof(SANotifyNewFindMsg) - ((NOFM_NEWMAX-wNumber)*sizeof(FindTeamInfo)));
	}
};

DECLARE_MSG(SGetTeamQuestMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_GETTEAMQUEST)
struct SQGetTeamQuestMsg : public SGetTeamQuestMsg
{
};

DECLARE_MSG(SNotifyOldQuestMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_OLDQUESTLIST)
struct SANotifyOldQuestMsg : public SNotifyOldQuestMsg
{
	enum { NOQM_OLDMAX = 200, };

	WORD	wNumber;
	DWORD	dwOldList[NOQM_OLDMAX];

	DWORD GetMySize()
	{
		return (sizeof(SANotifyOldQuestMsg) - ((NOQM_OLDMAX-wNumber)*sizeof(DWORD)));
	}
};

DECLARE_MSG(SNotifyNewQuestMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_NEWQUESTLIST)
struct SANotifyNewQuestMsg : public SNotifyNewQuestMsg
{
	enum { NNQM_NEWMAX = 200, };

	WORD		wNumber;
	TRWithIndex	dwNewList[NNQM_NEWMAX];

	DWORD GetMySize()
	{
		return (sizeof(SANotifyNewQuestMsg) - ((NNQM_NEWMAX-wNumber)*sizeof(TRWithIndex)));
	}
};

DECLARE_MSG(SSubmitMakeTeamMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_SUBMIT_REQUEST)
struct SQSubmitMakeTeamMsg : public SSubmitMakeTeamMsg
{
	char szInfo[TeamRequest::PTR_MAXINFO];
};

struct SASubmitMakeTeamMsg : public SSubmitMakeTeamMsg
{
	enum
	{
		SMT_SUCCESS = 1,		// �ɹ�
		SMT_INTEAM,				// ���ڶ�����
		SMT_ALREADYSUBMIT,		// �Ѿ��ύ����
	};

	BYTE bReulst;
};

DECLARE_MSG(STeamCancelQuestMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_CANCEL_REPLY)
struct SATeamCancelQuestMsg : public STeamCancelQuestMsg
{
	enum
	{
		TCQ_DEAD = 1,		// ����
		TCQ_DISCONNECT,		// ����
		TCQ_LOGOUT,			// �˳�
		TCQ_OVERTIME,		// ��ʱ
	};
	DWORD dwSrcID;			// ���������
	BYTE  bResult;			// ȡ����ԭ��
};

DECLARE_MSG(STeamBasePosMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_POSITION)
struct SATeamPosMsg : public STeamBasePosMsg
{
	BYTE			m_Num;
	TeamMemberPos	m_Pos[5];
};

DECLARE_MSG(STeamBaseJoinMsgG, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_JOIN)
struct SQTeamJoinMsgG : public STeamBaseJoinMsgG
{
	enum
	{
		QTJ_AGREE,
		QTJ_REJECT,
	};
	enum
	{
		TJM_JOINME = 1,
		TJM_JOINOTHER,
	};
	BYTE	bResult;					// ���
	BYTE	type;						// ����
	DWORD	dwGID;						// ������GID
};

struct SATeamJoinMsgG : public STeamBaseJoinMsgG
{
	enum
	{
		TJM_JOINME = 1,
		TJM_JOINOTHER,
	};
	DWORD	dwGID;						// ������GID
	BYTE	type;						// ��������
	char    cName[CONST_USERNAME];					// ����������
};

DECLARE_MSG(STeamBaseRefreshMsgG, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_REFRESH)

struct SATeamRefreshMsgG : public STeamBaseRefreshMsgG
{
    BYTE                memberMark;         // bitarray[8]����ʾ�ĸ���Ա��������Ҫ�޸�
	Team::TeamPlayer	stMember[8];		// ���еĶ�Ա��Ϣ

    WORD GetSize()
    {
        if (0 == memberMark)
            return 0;

		int i = 0;
        for (int n = 0; n < 8; ++n)
            if (memberMark & (1 << n))
                ++i;

        return (WORD)(sizeof(SATeamRefreshMsgG) - (8 - i) * sizeof(Team::TeamPlayer));
    }
};

DECLARE_MSG(STeamBaseKickMsgG, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_KICK)
struct SQTeamKickMsgG : public STeamBaseKickMsgG
{
	enum
	{
		TKM_DISMISSED,		// �����ɢ
		TKM_KICKOFF,		// ����
		TKM_QUIT,			// �Լ��˳�
	};

	BYTE operation;
    char szName[CONST_USERNAME];
};

struct SATeamKickMsgG : public STeamBaseKickMsgG
{
	enum
	{
		TKM_DISMISSED,		// �����ɢ
		TKM_KICKOFF,		// ����
		TKM_QUIT,			// �Լ��˳�
	};

	BYTE	operation;
	char	szName[CONST_USERNAME];
};

DECLARE_MSG(STeamBaseTalkMsgG, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_TALK)
struct SQTeamTalkMsgG : public STeamBaseTalkMsgG
{
    char  szTalkerName[CONST_USERNAME];
    char  szTalkMsg[MAX_CHAT_LEN];
};

struct SATeamTalkMsgG : public STeamBaseTalkMsgG
{
	char  szTalkerName[CONST_USERNAME];
    char  szTalkMsg[MAX_CHAT_LEN];
};

DECLARE_MSG(STeamBaseChangeSkillMsgG, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_CHANGE_SKILL)
struct SQTeamChangeSkillMsgG : public STeamBaseChangeSkillMsgG
{
    BYTE bySkill;
	BYTE byLevel;
};

struct SATeamChangeSkillMsgG : public STeamBaseChangeSkillMsgG
{
    BYTE bySkill;
	BYTE byLevel;
};

// ���öӳ�
DECLARE_MSG(STeamBaseHeadmenMsg, STeamBaseMsgG, STeamBaseMsgG::EPRO_TEAM_HEADMEN)
struct SQCSTeamBaseHeadmenMsg : public STeamBaseHeadmenMsg
{
	BYTE pos;				// Ҫ���öӳ���λ��
};

struct SACSTeamBaseHeadmenMsg : public STeamBaseHeadmenMsg
{
	enum
	{
		CSTB_SUCCESS,		// �ɹ�
		CSTB_NO_ACCESS,		// �㲻�Ƕӳ���û��Ȩ��
	};

	BYTE pos;				// ���õ�λ��
};