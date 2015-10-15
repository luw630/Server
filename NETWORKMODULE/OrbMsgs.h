#pragma once

#include "NetModule.h"
#include "orbtypedef.h"
#include "chattypedef.h"
#include "../pub/ConstValue.h"

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ORB-SERVER��������������������Ϣ�࣬���п�����Ϣ��������ʱ���
//=============================================================================================
DECLARE_MSG_MAP(SORBMsg, SMessage, SMessage::EPRO_ORB_MESSAGE)
//{{AFX
EPRO_CTRL_SIMPLAYER,    // ������ɫ���������Ϣ
EPRO_CTRL_TEAMGROUP,    // ��ӿ��������Ϣ
EPRO_CTRL_CHANNEL,      // ����Ƶ�������Ϣ
EPRO_CTRL_FACTION,        // �����������Ϣ
EPRO_CTRL_CHAT,         // ���������������Ϣ
EPRO_CTRL_CHECK,        // �������Ƿ����ߵ���Ϣ
EPRO_CTRL_FRIEND,       // ������ص���Ϣ
EPRO_CTRL_DATATRANS,    // ���ݴ���Э�飡
//}}AFX
END_MSG_MAP_WITH_ROTOC()
//---------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////
// EPRO_ADD_SIMPLAYER,     // ������ɫ�����
// ���������Ϣ��ȥ���ˣ���Ϊ���Ĳ�����EPRO_REFRESH_SIMPLAYER������ͬ
//  ����EPRO_REFRESH_SIMPLAYER��ȫ�������EPRO_ADD_SIMPLAYER
// EPRO_REFRESH_SIMPLAYER => EPRO_SYN_SIMPLAYER
///////////////////////////////////////////////////////////////////////////////////////////////
// �������������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SCtrlSimPlayer, SORBMsg, SORBMsg::EPRO_CTRL_SIMPLAYER)
//{{AFX
EPRO_DEL_SIMPLAYER,     // ������ɫ��ɾ��
EPRO_SYN_SIMPLAYER,     // ������ɫ��ˢ�������

EPRO_QUERY_SIMPLAYER,   // ��ѯ��ɫ��Ϣ added by yg
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------
DECLARE_MSG_MAP(SCtrlCheckMsg, SORBMsg, SORBMsg::EPRO_CTRL_CHECK)
//{{AFX
EPRO_CHECK_NAME,        // �����ֲ�������Ƿ�����
//}}AFX
END_MSG_MAP()

DECLARE_MSG_MAP(SCtrlFactionMsg,SORBMsg,SORBMsg::EPRO_CTRL_FACTION)
//{{AFX
EPRO_TRANSFER_FACTION,
//}}AFX
END_MSG_MAP()

DECLARE_MSG(STransferFactionMsg,SCtrlFactionMsg,SCtrlFactionMsg::EPRO_TRANSFER_FACTION)
struct SQTransferFactionMsg :
    public STransferFactionMsg
{
	char szFactionName[CONST_USERNAME];
    WORD wSize; // size of byMessage 
    DNID    dnidClientExp;  
    BYTE byMessage[2048*10];

    WORD GetMySize()
    {
        return (WORD)(sizeof(SQTransferFactionMsg)-(2048*10-wSize));
    };
    void SetMySize(WORD wSetSize)
    {
        wSize = wSetSize;
    };
};

struct SATransferFactionMsg :
    public STransferFactionMsg
{
	char szFactionName[CONST_USERNAME];
    WORD wSize;
    DNID    dnidClientExp;  
    BYTE byMessage[2048*10];
    WORD GetMySize()
    {
        return (WORD)(sizeof(SATransferFactionMsg)-(2048*10-wSize));
    };
    void SetMySize(WORD wSetSize)
    {
        wSize = wSetSize;
    };
};

DECLARE_MSG(SCheckNameMsg, SCtrlCheckMsg,SCtrlCheckMsg::EPRO_CHECK_NAME)
struct SQCheckNameMsg :
    public SCheckNameMsg
{
    char szMyName[CONST_USERNAME];
	char szFriendName[CONST_USERNAME];
    BYTE byType; // 0 ���� 
};

struct SACheckNameMsg :
     public SCheckNameMsg
{
    char szName[CONST_USERNAME];
    BYTE byType; 
    BYTE byResult; // 0 ���� 1 //������
};
//=============================================================================================
// ��������ɾ��������Ϣ
DECLARE_MSG(SDelSimPlayerMsg, SCtrlSimPlayer, SCtrlSimPlayer::EPRO_DEL_SIMPLAYER)
struct SQDelSimPlayerMsg :
    public SDelSimPlayerMsg
{
    // �����ؼ���
    DWORD   dwStaticID;

    // ��λ�ؼ���
    WORD    wServerID;          // ��������������ID
    DNID    dnidClient;         // ����������ϸ���ҵ�DNID

    // У��ؼ���
    DWORD   dwClientGID;        // ����ұ��ε�¼��ȫ�ֹؼ���
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ������������ˢ����Ϣ
DECLARE_MSG(SSynSimPlayerMsg, SCtrlSimPlayer, SCtrlSimPlayer::EPRO_SYN_SIMPLAYER)
struct SQSynSimPlayerMsg :
    public SSynSimPlayerMsg
{
    // �����ؼ���
    DWORD   dwStaticID;

    // ��λ�ؼ���
    WORD    wServerID;          // ��������������ID
    DNID    dnidClient;         // ����������ϸ���ҵ�DNID

    // У��ؼ���
    DWORD   dwClientGID;        // ����ұ��ε�¼��ȫ�ֹؼ���

    // ��������
    char	szName[CONST_USERNAME];         // ����ҵ�����[�Լ���ĩβ�ض�]
    WORD    wCurRegionID;       // ���ڳ�����ID
    DWORD   dwGMLevel;          // GM�ȼ�

	BYTE	bOnlineState;		// 0:����  1:����  2:�һ� 
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// ������ͨ��ORB��ѯ�����Ϣ
DECLARE_MSG(SQuerySimPlayerMsg, SCtrlSimPlayer, SCtrlSimPlayer::EPRO_QUERY_SIMPLAYER)
struct SQQuerySimPlayerMsg :
    public SQuerySimPlayerMsg
{
	char szName[CONST_USERNAME];			// Player Name to query
    DNID dnidClient;			// ����������ϸ���ҵ�DNID������������ORB����ʱ�踳ֵ���ͻ���������ʱ����
};
struct SAQuerySimPlayerMsg :
    public SQuerySimPlayerMsg
{
	enum OperateType
	{
		REQUEST,
		ANSWER,
	} stOperateType;

    WORD wServerID;          // ��������������ID
    DNID dnidClient;         // ����������ϸ���ҵ�DNID
	char szName[CONST_USERNAME];
	bool bIsMan;
	char szTitle[16];
	char szTongName[CONST_USERNAME];
	BYTE byPKValue;
	BYTE byXValue;
};
//---------------------------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ���������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SCtrlTeamGroup, SORBMsg, SORBMsg::EPRO_CTRL_TEAMGROUP)
//{{AFX


EPRO_CREATE_TEAM,           // ����Ĵ���
EPRO_DELETE_TEAM,           // �����ɾ��

// �����ˢ��
EPRO_SOMEONE_JOIN_TEAM,     // ĳ�˼������
EPRO_SOMEONE_LEAVE_TEAM,    // ĳ���뿪����
EPRO_SOMEONE_KICKED_TEAM,   // ĳ���߳�����

// ����Ĳ�ѯ&����
EPRO_TEAM_OPERATION,        // �����ز�������
EPRO_TEAM_PASSBYMESSAGE,

//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------
//=============================================================================================
DECLARE_MSG(STeamBasePassbyMsg,SCtrlTeamGroup,SCtrlTeamGroup::EPRO_TEAM_PASSBYMESSAGE)
struct SQTeamPassbyMsg :
    public STeamBasePassbyMsg,
    public SQTeamOperation
{
    DNID  dnidSrc;
    DNID  dnidDest;
    DWORD dwSrcGID;
    DWORD dwDestGID;
    DWORD dwSrcStaticID;
    DWORD dwDestStaticID;
};

struct SATeamPassbyMsg :
    public STeamBasePassbyMsg,
    public SATeamOperation
{
    WORD  wOprType;   // �������� 1.���� 2.������� 3.������� 4.����ʧ�� 0.ʧ��
    DNID  dnidSrc;
    DNID  dnidDest;
    DWORD dwSrcGID;
    DWORD dwDestGID;
    DWORD dwSrcStaticID;
    DWORD dwDestStaticID;
    DWORD dwSrcTeamID;
    DWORD dwDestTeamID;
};



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// �����ѯ��ز�����Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(STeamOperationMsg, SCtrlTeamGroup, SCtrlTeamGroup::EPRO_TEAM_OPERATION)
//{{AFX
EPRO_TEAM_GET_MEMBER,       // ��ȡ��ӳ�Ա������Ϣ
EPRO_TEAM_TALK,             // �������
EPRO_TEAM_SHARE_EXP,        // ��ӷ־���
EPRO_TEAM_CHANGE_SKILL,     // ����л����μ�
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ����һ������
DECLARE_MSG(STeamCreateMsg,SCtrlTeamGroup,SCtrlTeamGroup::EPRO_CREATE_TEAM)
struct SQTeamCreateMsg :
    public STeamCreateMsg,
    public SQTeamOperation
{
    // Ԥ����ID��Ϊ��һλ��Ա�ı�ʶ
    DWORD dwFlag; // ��Ӵ������,��������������ܵ�һЩѡ��
};

struct SATeamCreateMsg : 
    public STeamCreateMsg,
    public SATeamOperation
{
    DWORD   dwTeamID;
    WORD    wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ɾ��һ������
DECLARE_MSG(STeamDeleteMsg,SCtrlTeamGroup,SCtrlTeamGroup::EPRO_DELETE_TEAM)
struct SQTeamDeleteMsg :
    public STeamDeleteMsg,
    public SQTeamOperation
{

};

struct SATeamDeleteMsg : 
    public STeamDeleteMsg,
    public SATeamOperation
{
    WORD wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ĳ�˼������
DECLARE_MSG(STeamSomeoneJoinMsg,SCtrlTeamGroup,SCtrlTeamGroup::EPRO_SOMEONE_JOIN_TEAM)
struct SQTeamSomeoneJoinMsg:
    public STeamSomeoneJoinMsg,
    public SQTeamOperation
{
    //dwReserved ��ΪҪ����Ķ�����
};

struct SATeamSomeoneJoinMsg:
    public STeamSomeoneJoinMsg,
    public SATeamOperation
{
    WORD wResult;
    WORD wTeamSkill;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ĳ���뿪����
DECLARE_MSG(STeamSomeoneLeaveMsg,SCtrlTeamGroup,SCtrlTeamGroup::EPRO_SOMEONE_LEAVE_TEAM)
struct SQTeamSomeoneLeaveMsg:
    public STeamSomeoneLeaveMsg,
    public SQTeamOperation
{
   DWORD dwLeaverSID;
};

struct SATeamSomeoneLeaveMsg:
    public STeamSomeoneLeaveMsg,
    public SATeamOperation

{
    //The ID of the player whom the message should not be sendto 
    DNID  dnidLeaver;
    WORD  wLeaverServerID; 
    DWORD dwLeaverSID;
    WORD wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ĳ�˱��߳�����
DECLARE_MSG(STeamSomeoneKickMsg,SCtrlTeamGroup,SCtrlTeamGroup::EPRO_SOMEONE_KICKED_TEAM)
struct SQTeamSomeoneKickMsg :
    public STeamSomeoneKickMsg,
    public SQTeamOperation
{
    DWORD dwTeamID;
    DWORD dwKickedSID;
};

struct SATeamSomeoneKickMsg :
    public STeamSomeoneKickMsg,
    public SATeamOperation
{
    WORD wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ��ȡ�����Ա��������
DECLARE_MSG(SGetTeamMemberMsg, STeamOperationMsg, STeamOperationMsg::EPRO_TEAM_GET_MEMBER)
struct SQGetTeamMemberMsg :
    public SGetTeamMemberMsg,
    public SQTeamOperation
{
    // ûʲô�����Ա������SQTeamOperation�����
    DWORD dwTeamID;
};

struct SAGetTeamMemberMsg :
    public SGetTeamMemberMsg,
    public SATeamOperation
{
    WORD wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ������죬���ܻ�ʹ������ϵͳ�Ͳ���һ��ɣ�������ʱ������
DECLARE_MSG(STeamTalkBaseMsg, STeamOperationMsg, STeamOperationMsg::EPRO_TEAM_TALK)
struct  SQTeamTalkMsg : STeamTalkBaseMsg,
    public SQTeamOperation
{
    DWORD dwTeamID;
    DWORD dwSrcGlobalID;
    char  szTalkerName[CONST_USERNAME];
    char  szTalkMsg[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        return (WORD)(sizeof(SQTeamOperation) - (MAX_CHAT_LEN-strlen(szTalkMsg)-1));
    }
};

struct  SATeamTalkMsg : STeamTalkBaseMsg,
    public SATeamOperation
{
    DWORD dwTeamID;
    char  szTalkerName[CONST_USERNAME];
    char  szTalkMsg[MAX_CHAT_LEN];

    WORD GetMySize()
    {
        return (WORD)(sizeof(SATeamTalkMsg) - (MAX_CHAT_LEN-strlen(szTalkMsg)-1));
    }
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ��ӷ־��飬������Ϊ��ͬ�����Ľ�ɫ���ܹ����飬���������Ϣ���ܻ��ò��ϣ�������ʱ������
DECLARE_MSG(STeamShareExpMsg, STeamOperationMsg, STeamOperationMsg::EPRO_TEAM_SHARE_EXP)
struct SQTeamShareExpMsg : 
    public STeamShareExpMsg,
    public SQTeamOperation
{
    DWORD dwTeamID;
    //dwReserved ��Ϊ�������õ��ľ���ֵ
};

struct SATeamShareExpMsg :
    public STeamShareExpMsg,
    public SATeamOperation
{
    DWORD dwExp;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// �л����μ�
DECLARE_MSG(STeamChangeSkillMsg, STeamOperationMsg, STeamOperationMsg::EPRO_TEAM_CHANGE_SKILL)
struct SQTeamChangeSkillMsg :
    public STeamChangeSkillMsg,
    public SQTeamOperation
{
    BYTE bySkill; // ��������ڻش��ĵ�ǰ�л��ļ���
    BYTE byLevel;
};

struct SATeamChangeSkillMsg :
    public STeamChangeSkillMsg,
    public SATeamOperation
{
    BYTE bySkill; // ��������ڻش��ĵ�ǰ�л��ļ���
    BYTE byLevel;
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// ����Ƶ����Ϣ
DECLARE_MSG_MAP(SORBChannelBaseMsg, SORBMsg, SORBMsg::EPRO_CTRL_CHANNEL)
//{{AFX
EPRO_CHANNEL_JOIN,
EPRO_CHANNEL_LEAVE,
EPRO_CHANNEL_TALK,
EPRO_CHANNEL_KICK,
EPRO_CHANNEL_CREATE,
EPRO_CHANNEL_DISMISS,
EPRO_CHANNEL_CHANGERULE,
EPRO_CHANNEL_GETMEMBERINFO,   // ȡ������������ҵ���Ϣ
EPRO_CHANNEL_GETCHANNELINFO,  // ȡ�������ҵĻ�����Ϣ
EPRO_CHANNEL_WHISPER,
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------
//=============================================================================================
DECLARE_MSG(SORBChangeRuleBaseChannelMsg,SORBChannelBaseMsg,SORBChannelBaseMsg::EPRO_CHANNEL_CHANGERULE)
struct SQORBChangeRuleChannelMsg : public SORBChangeRuleBaseChannelMsg
{
    DWORD dwStaticID;
    int iChannelRules;
};
struct SAORBChangeRuleChannelMsg : public SORBChangeRuleBaseChannelMsg
{
    DWORD dwChannelID;
    int iChannelRules;

    WORD wResult;
};


//=============================================================================================
DECLARE_MSG(SORBCreateBaseChannelMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_CREATE)
struct SQORBCreateChannelMsg : public SORBCreateBaseChannelMsg
{
    DWORD dwCreatorStaticID;//the creator static id
    int   iChannelRules; 
	char  szChannelName[CONST_USERNAME];
};

struct SAORBCreateChannelMsg : public SORBCreateBaseChannelMsg
{
    char  szCreatorName[CONST_USERNAME];
	char  szChannelName[CONST_USERNAME];
    DWORD dwChannelID;

    WORD wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SORBDisMissBaseChannelMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_DISMISS)
struct SQORBDisMissChannelMsg : public SORBDisMissBaseChannelMsg
{
    DWORD dwStaticID;//the creator static id 
    DWORD  dwChannelID;
};

struct SAORBDisMissChannelMsg :public SORBDisMissBaseChannelMsg
{
    DWORD dwChannelID;

    WORD wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SORBJoinBaseChannelMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_JOIN)
struct SQORBJoinChannelMsg : public SORBJoinBaseChannelMsg
{
    WORD  wResult;
    DWORD dwStaticID;
    DWORD  dwChannelID;
};
//---------------------------------------------------------------------------------------------
struct SAORBJoinChannelMsg : public SORBJoinBaseChannelMsg
{
    WORD  wResult;
    char  szPlayerName[CONST_USERNAME];

    DWORD dwStaticID;
    DWORD  dwChannelID;
};


//=============================================================================================
DECLARE_MSG(SORBLeaveBaseChannelMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_LEAVE)
struct SQORBLeaveChannelMsg : public SORBLeaveBaseChannelMsg
{
    WORD  wResult;
    DWORD dwStaticID;
    DWORD  dwChannelID;
};
//---------------------------------------------------------------------------------------------
struct SAORBLeaveChannelMsg : public SORBLeaveBaseChannelMsg
{
    WORD  wResult;
    char  szPlayerName[CONST_USERNAME];

    DNID  dnidTo;

    DWORD dwStaticID;
    DWORD dwChannelID;
};


//=============================================================================================
DECLARE_MSG(SORBKickBaseChannelMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_KICK)
struct SQORBKickChannelMsg : public SORBKickBaseChannelMsg
{
    WORD  wResult;
    DWORD dwCreatorStaticID;
    DWORD dwKickedStaticID;
    DWORD  dwChannelID;
};
//---------------------------------------------------------------------------------------------
struct SAORBKickChannelMsg : public SORBKickBaseChannelMsg
{
    WORD  wResult;
    char  szPlayerName[CONST_USERNAME];

    DWORD dwCreatorStaticID;
    DWORD dwKickedStaticID;
    DWORD  dwChannelID;
};


//=============================================================================================
DECLARE_MSG(SORBChatBaseChannelMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_TALK)
//��������������������
struct SQORBChatChannelMsg : public SORBChatBaseChannelMsg
{
    DWORD dwStaticID;
    
    DWORD dwGlobalID;
    DWORD dwChannelID;
    WORD wLength;
    DWORD dwRecieverSID;
    char cCharData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SQORBChatChannelMsg, cCharData);
    }
};
//---------------------------------------------------------------------------------------------

//��������������������
struct SAORBChatChannleMsg : public SORBChatBaseChannelMsg
{
    DWORD dwStaticID;
    DWORD dwGlobalID;
    DWORD dwChannelID;
    WORD  wLength;
    char  szPlayerName[CONST_USERNAME];
    char  szRecieverName[CONST_USERNAME];

    DNID dnidTo;       // ������������Ͻ�����ҵ�Dnid

    char  cCharData[MAX_CHAT_LEN];
    WORD  GetMySize()
    {
        _GetSize(SAORBChatChannleMsg, cCharData);
    }
};
//=============================================================================================
DECLARE_MSG(SORBGetBaseChannelMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_GETMEMBERINFO)
struct SQORBGetChannelMsg : public SORBGetBaseChannelMsg
{
    DWORD dwStaticID ; //��ȡ������Ƶ����Ϣ����ҵ�StaticID
    DWORD dwChannelID; //��ȡ�õ������ҵ�ID
};

struct SAORBGetChannelMsg : public SORBGetBaseChannelMsg
{
    DWORD dwStaticID;  //��ȡ������Ƶ����Ϣ����ҵ�StaticID
    DWORD dwChannelID; //��ȡ�õ������ҵ�ID
    WORD wPlayerNumber;//������������ҵĸ���
    DNID dnidTo;
    SChannelPlayerInfo ChannelPlayer[MAX_CHANNEL_PLAYER_NUMBER];
    WORD GetMySize()
    {
        return sizeof(SAORBGetChannelMsg)-(MAX_CHANNEL_PLAYER_NUMBER-wPlayerNumber)*sizeof(SChannelPlayerInfo);
    };
};
//=============================================================================================
DECLARE_MSG(SORBGetBaseChannelInfoMsg, SORBChannelBaseMsg, SORBChannelBaseMsg::EPRO_CHANNEL_GETCHANNELINFO)
struct SQORBGetChannelInfoMsg : SORBGetBaseChannelInfoMsg
{
    DWORD dwStaticID;
};
//---------------------------------------------------------------------------------------------
struct SAORBGetChannelInfoMsg : SORBGetBaseChannelInfoMsg
{
    DWORD dwChannelID;
	char  szChannelName[CONST_USERNAME];
    char  szCreatorName[CONST_USERNAME];
    int   iRules;
};


//=============================================================================================
DECLARE_MSG(SORBBaseChannelWhisperMsg, SORBChannelBaseMsg ,SORBChannelBaseMsg::EPRO_CHANNEL_WHISPER)
struct SQORBChannelWhisperMsg : SORBBaseChannelWhisperMsg
{
    DWORD dwTalkerStaticID; // ������Ϣ����ҵ�StaticID
    DWORD dwReceiverStaticID;
    DWORD dwChannelID;
    char  szChatMsg[MAX_CHAT_LEN];
};
//---------------------------------------------------------------------------------------------
struct SAORBChannelWhisperMsg : SORBBaseChannelWhisperMsg
{
    DNID dnidTo;
    DWORD dwTalkerStaticID;
    DWORD dwChannelID;
    char szTalkerName[CONST_USERNAME];
    char szReceiverName[CONST_USERNAME];
    DWORD dwReceiverStaticID;
    char szChatMsg[MAX_CHAT_LEN];
};
//=============================================================================================
DECLARE_MSG_MAP(SORBFriendBaseMsg,SORBMsg,SORBMsg::EPRO_CTRL_FRIEND)
//{{AFX
EPRO_FRIEND_ONLINE, // ��������֪ͨ��Ϣ
//}}AFX
END_MSG_MAP()

DECLARE_MSG(SORBBaseFriendOnlineMsg,SORBFriendBaseMsg,SORBFriendBaseMsg::EPRO_FRIEND_ONLINE)
struct SQORBFriendOnlineMsg : SORBBaseFriendOnlineMsg
{
    char cName[CONST_USERNAME]; // ������ҵ�����
    char cFriendName[CONST_USERNAME]; // ���ѵ�����
    DWORD dwStaticID;

	bool bOnline;
};
struct SAORBFriendOnlineMsg : SORBBaseFriendOnlineMsg
{
    char cName[CONST_USERNAME]; 
    char cFriendName[CONST_USERNAME];
    DWORD dwStaticID;

	bool bOnline;
};

//=============================================================================================
DECLARE_MSG_MAP(SORBChatBaseMsg, SORBMsg, SORBMsg::EPRO_CTRL_CHAT)
//{{AFX
EPRO_CHAT_WISPER,   //������Ϣ
EPRO_CHAT_GLOBAL,
EPRO_CHAT_FACTION,
//}}AFX
END_MSG_MAP()

DECLARE_MSG(SORBBaseChatWisperMsg, SORBChatBaseMsg, SORBChatBaseMsg::EPRO_CHAT_WISPER)
struct SQORBChatWisperMsg : SORBBaseChatWisperMsg
{
    char cTalkerName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
    char cRecvName[CONST_USERNAME];
};
//---------------------------------------------------------------------------------------------
struct SAORBChatWisperMsg : SORBBaseChatWisperMsg
{
    char  cName[CONST_USERNAME];
    char  cNameII[CONST_USERNAME];
    char  cChatData[MAX_CHAT_LEN];
    BYTE  byType; // 1 for recveiver 2 for sender
    DNID  dnidRecv;
    WORD  wRecvServerID;
    DWORD dwGMLevel;
    DWORD dwGMLevelII;
};

DECLARE_MSG(SORBBaseChatGlobalMsg, SORBChatBaseMsg, SORBChatBaseMsg::EPRO_CHAT_GLOBAL)
struct SQORBChatGlobalMsg : public SORBBaseChatGlobalMsg
{
    BYTE byType;
    char cTalkerName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SQORBChatGlobalMsg,cChatData);
    }
};

struct SAORBChatGlobalMsg : public SORBBaseChatGlobalMsg
{
    BYTE byType;
    char cTalkerName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SAORBChatGlobalMsg,cChatData);
    }
};
DECLARE_MSG(SORBBaseChatFactionMsg, SORBChatBaseMsg, SORBChatBaseMsg::EPRO_CHAT_FACTION)
struct SQORBChatFactionMsg : public SORBBaseChatFactionMsg
{
    char cTalkerName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
	char cFactionName[CONST_USERNAME];
    WORD GetMySize()
    {
        _GetSize(SQORBChatFactionMsg,cChatData);
    }
};
struct SAORBChatFactionMsg : public SORBBaseChatFactionMsg
{
    char cTalkerName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
	char cFactionName[CONST_USERNAME];
    WORD GetMySize()
    {
        _GetSize(SAORBChatFactionMsg,cChatData);
    };
};


//////////////////////////////////////////////////////////////////////
// orb ����ת����ز���
//////////////////////////////////////////////////////////////////////
DECLARE_MSG(SORBDataTransMsg, SORBMsg, SORBMsg::EPRO_CTRL_DATATRANS)

struct SQAORBDataTransMsg : public SORBDataTransMsg
{
public:
	enum TRANS_TYPE
	{
		TRANSDATATOPLAYER,
		PLAYERINFO_QUERY,
		TRANSDATATO_DESTPLAYERSERVER,
		TRANSDATATO_ALLSERVER,
	};

private:
    WORD m_wTransType;
    WORD m_wTransSize;
	char m_szTransName[CONST_USERNAME];
    char m_pTransData[1];

public:
    WORD GetTransType() { return m_wTransType; }
    WORD GetTransSize() { return m_wTransSize; }
    LPCSTR GetTransName() { return m_szTransName; }
    LPCVOID GetTransData() { return m_pTransData; }

    WORD GetMySize() { return sizeof( SQAORBDataTransMsg ) + m_wTransSize; }

    static SQAORBDataTransMsg* MakeDataTransMsg( WORD wTransType, LPCSTR szTransName, 
        LPCVOID pTransData, WORD wTransSize, void *bufHolder, size_t bufSize )
    {
        if ( wTransSize > 0xC000 || bufHolder == NULL || bufSize < sizeof( SQAORBDataTransMsg ) + wTransSize )
            return NULL;

#ifdef _DEBUG
#   undef new
#endif
        SQAORBDataTransMsg *pMsg = new ( bufHolder ) SQAORBDataTransMsg;
        pMsg->dnidClient = -1;
        pMsg->m_wTransType = wTransType;
        pMsg->m_wTransSize = wTransSize;
		dwt::strcpy(pMsg->m_szTransName, szTransName, CONST_USERNAME);
        memcpy( pMsg->m_pTransData, pTransData, wTransSize );

        return pMsg;
    }

/*
    static WORD MakeDataTransMsg(SQAORBDataTransMsg *&pMsg, LPCSTR szTransName, LPCVOID pTransData, WORD wTransSize, WORD wTransType)
    {
        pMsg = NULL;

        // ̫��İ��ˣ�
        if (wTransSize > 0xC000)
            return 0;

        SQAORBDataTransMsg temp;
        temp.dnidClient = -1;
        temp.m_wTransType = wTransType;
        temp.m_wTransSize = wTransSize;
        dwt::strcpy(temp.m_szTransName, szTransName, CONST_USERNAME);

        pMsg = (SQAORBDataTransMsg *)new char[sizeof(SQAORBDataTransMsg) + wTransSize];

        memcpy(pMsg, &temp, sizeof(SQAORBDataTransMsg));
        memcpy(pMsg->m_pTransData, pTransData, wTransSize);

        return sizeof(SQAORBDataTransMsg) + wTransSize;
    }
*/
};
