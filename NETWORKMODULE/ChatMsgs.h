#pragma once

#include "NetModule.h"
#include "chattypedef.h"

// ����ı������
// �����г��������ַ���Ϊ�������
//=============================================================================================
DECLARE_MSG_MAP(SChatBaseMsg, SMessage, SMessage::EPRO_CHAT_MESSAGE)
//{{AFX
EPRO_CHAT_SYSMSG,	        // ϵͳ��Ϣ
EPRO_CHAT_SYSCALL,			// ϵͳ����
EPRO_CHAT_PUBLIC,			// ����Ƶ��
EPRO_CHAT_TEAM,			    // ����Ƶ��
EPRO_CHAT_GANG,			    // ����
EPRO_CHAT_WHISPER,			// ˽��Ƶ��
EPRO_CHAT_CHATROOM,	    	// ������
EPRO_CHAT_TEMP_SYSTEM,      // ��ʱ��ʾ��Ϣ
EPRO_CHAT_CHANNEL,          // ����Ƶ��
EPRO_CHAT_GLOBAL,           // ����Ƶ��
EPRO_CHAT_FACTION,          // ����Ƶ��
EPRO_CHAT_SCRIPTBULLETIN,   // �ű�����
EPRO_CHAT_TALKMASK,			// ������֪ͨ�ͻ��� ��ɫ�����Ժͽ���ʱ��
EPRO_CHAT_SCHOOL,           // ����Ƶ��
EPRO_CHAT_KILLED,			// ����֪ͨ(�����а���������֪ͨȫ��)
EPRO_CHAT_REGION,           // ����Ƶ��
EPRO_CHAT_TIPS,				// ��ʾ��Ϣ
EPRO_CHAT_RUMOR,			// ����Ƶ��
EPRO_CHAT_BROADCAST,
EPRO_CHAT_BUSINESS,			// �̻�Ƶ��
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------


//=============================================================================================

DECLARE_MSG(SChatBaseTalkMaskMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_TALKMASK)

struct SChatTalkMaskMsg : public SChatBaseTalkMaskMsg
{
	DWORD dwTime;		//ʱ�䵥λ�� ����
};

//---------------------------------------------------------------------------------------------


//=============================================================================================
DECLARE_MSG(SChatBaseSysMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_SYSMSG)
struct SChatSysMsg : public SChatBaseSysMsg
{
    BYTE btMsgID;							        // ϵͳ��ϢID
    char cSystem[MAX_CHAT_LEN];                     // ϵͳ��Ϣ��Ϣ
    WORD GetMySize()
    {
        _GetSize(SChatSysMsg, cSystem);
    }
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SChatBaseToAllMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_SYSCALL)
struct SChatToAllMsg : public SChatBaseToAllMsg
{
    WORD wSoundId;							        // �������
    WORD wChatLen;							        // ������Ϣ����
    char cChat[MAX_CHAT_LEN];	                    // ������Ϣ(����ͼƬ���)
	WORD wShowNum;	//add by ly 2014/5/4 ϵͳ������ʾ�Ĵ���
    WORD GetMySize()
    {
        _GetSize(SChatToAllMsg, cChat);
    }
	SChatToAllMsg() :wShowNum(1)
	{

	}
};
//---------------------------------------------------------------------------------------------

DECLARE_MSG(SChatBasePublic, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_PUBLIC)
struct SQChatPublic : public SChatBasePublic
{
    // �������ս���
    WORD wLength;
    WORD wSoundId;
    char cCharData[MAX_CHAT_LEN];

    WORD GetMySize()
    {
        _GetSize(SQChatPublic, cCharData);
    }
};

struct SAChatPublic : public SChatBasePublic
{
    DWORD  dwGID;
    WORD   wLength;
    WORD   wSoundId;
    char   cCharData[MAX_CHAT_LEN];

    WORD GetMySize()
    {
        _GetSize(SAChatPublic, cCharData);
    }
};

DECLARE_MSG( SChatBaseTeam, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_TEAM )
struct SQChatTeam : public SChatBaseTeam
{
	char cTalkerName[CONST_USERNAME];
	char cChatData[ MAX_CHAT_LEN ];
	WORD GetMySize()
	{
		_GetSize( SQChatTeam, cChatData );
	}
};

struct SAChatTeam: public SChatBaseTeam
{
	char cTalkerName[CONST_USERNAME];
	char cChatData[ MAX_CHAT_LEN ];
	WORD GetMySize()
	{
		_GetSize( SAChatTeam, cChatData );
	}

};

//---------------------------------------------------------------------------------------------
//=============================================================================================
// ��������

DECLARE_MSG(SChatBaseFactionMsg,SChatBaseMsg,SChatBaseMsg::EPRO_CHAT_FACTION)
struct SQChatFactionMsg : public SChatBaseFactionMsg
{
	char cTalkerName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SQChatFactionMsg,cChatData);
    }
};
struct SAChatFactionMsg : public SChatBaseFactionMsg
{
    WORD wResult; // 0 ��δ������ , 1 �ɹ�
	char cTalkerName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SAChatFactionMsg,cChatData);
    }
};

// ==================================================================================
// ��������
DECLARE_MSG(SChatBaseGlobalMsg,SChatBaseMsg,SChatBaseMsg::EPRO_CHAT_GLOBAL)
struct SQChatGlobalMsg : public SChatBaseGlobalMsg
{
    BYTE  byType;
	DWORD color;
	char  cTalkerName[CONST_USERNAME];
	char  cHead[MAX_HEAD_LEN];
    char  cChatData[MAX_CHAT_LEN];
	
    WORD GetMySize()
    {
        _GetSize(SQChatGlobalMsg,cChatData);
    };
};

struct SAChatGlobalMsg : public SChatBaseGlobalMsg
{
    typedef enum
    {
        ECT_NORMAL,
        ECT_SYSTEM,
        ECT_GAMEMASTER,
        ECT_RUMOUR,
        ECT_TRADE,
        ECT_RUMOUR_FAIL,
		ECT_SMALL_HORN,			// С����
		ECT_AFFICHE,			// ϵͳ����;��ʾ����Ļ���м����
		ECT_IMPORTANT_AFFICHE,  // ���ȼ��ߵ�ϵͳ����
		ECT_SPECIAL,			// ������Ϣ��������������ʾ�����±߹�����Ϣ��ʾ��λ��
		ECT_TREASURE_SCROLL,	// �ٱ���Ĺ�����Ϣ
		ECT_TREASURE_EXPAIN,	// �ٱ������½ǵ���Ϣ

        ECT_CTIYOWNER,			// ����
        ECT_CTIYOWNERTRADE,     // ��ó������Ʒ
        ECT_CTIYOWNERTRADE_ITEM, 

		ECT_WULINCHIEF,
    };

    BYTE  byType;
	DWORD color;
	
	char  cTalkerName[CONST_USERNAME];
	//WK 2015 .1.13 ���,ǰ̨ͷ�� begin
	char  cHead[MAX_HEAD_LEN];
	//WK 2015 .1.13 ���,ǰ̨ͷ�� end

    char  cChatData[MAX_CHAT_LEN];
    
	WORD GetMySize()
    {
        _GetSize(SAChatGlobalMsg,cChatData);
    };
};

struct SAAfficheMsg : public SChatBaseGlobalMsg
{

    BYTE byType;
	BYTE color[3];
	BYTE scrollNum;
    char cChatData[200];
};

DECLARE_MSG(SScriptChatGlobalMsg,SChatBaseMsg,SChatBaseMsg::EPRO_CHAT_SCRIPTBULLETIN)
// ��Ӧ�Ľű�ͨ��
#define _GetSSize(classname, buffer)    WORD len; \
    len = (WORD)dwt::strlen(buffer, 500); \
    buffer[len] = 0; \
    return sizeof(classname) - (500 - len - 1) 

struct SAScriptChatGlobalMsg : public SScriptChatGlobalMsg
{
    BYTE byType;
    char cChatData[500];
    WORD GetMySize()
    {
        _GetSSize(SAScriptChatGlobalMsg,cChatData);
    };
};
//=============================================================================================
DECLARE_MSG(SChatBaseGang, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_GANG)
//Client To Server
struct SQChatGang : public SChatBaseGang
{
    WORD wLength;
    char cCharData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SQChatGang, cCharData);
    }
};

//Server To Client
struct SAChatGang : public SChatBaseGang
{
    DWORD dwGID;
    WORD wLength;
    char cCharData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SAChatGang, cCharData);
    }

};
// ====================================================
// ˽��
DECLARE_MSG(SChatBaseWisperMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_WHISPER)
struct SQChatWisperMsg : public SChatBaseWisperMsg
{
	char szRecvName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SQChatWisperMsg, cChatData);
    }
};

struct SAChatWisperMsg : public SChatBaseWisperMsg
{
    BYTE byType;			// 1 for recveiver // 2 for sender
	char szName[CONST_USERNAME];
    char cChatData[MAX_CHAT_LEN];
	WORD wHeadIcon;   //ͷ��ID
	WORD wLevel;         //�ȼ�
    WORD GetMySize()
    {
        _GetSize(SAChatWisperMsg, cChatData);
    }
};

// ��������
//=============================================================================================
DECLARE_MSG( SChatBaseSchool, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_SCHOOL )
struct SQChatSchool : public SChatBaseSchool
{
	char cTalkerName[CONST_USERNAME];
    char cChatData[ MAX_CHAT_LEN ];
    WORD GetMySize()
    {
        _GetSize( SQChatSchool, cChatData );
    }
};

struct SAChatSchool: public SChatBaseSchool
{
	char cTalkerName[CONST_USERNAME];
    char cChatData[ MAX_CHAT_LEN ];
    WORD GetMySize()
    {
        _GetSize( SAChatSchool, cChatData );
    }

};
// ==================================================================================
// ��������
DECLARE_MSG(SChatRumorMsg,SChatBaseMsg,SChatBaseMsg::EPRO_CHAT_RUMOR)
struct SQChatRumorMsg : public SChatRumorMsg
{
	char  cTalkerName[CONST_USERNAME];
	char  cChatData[MAX_CHAT_LEN];

	WORD GetMySize()
	{
		_GetSize(SQChatRumorMsg,cChatData);
	};
};

struct SAChatRumorMsg : public SChatRumorMsg
{
	char  cTalkerName[CONST_USERNAME];
	char  cChatData[MAX_CHAT_LEN];

	WORD GetMySize()
	{
		_GetSize(SAChatRumorMsg, cChatData);
	};
};

// ��������
//=============================================================================================
DECLARE_MSG( SChatBaseRegion, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_REGION )
struct SQAChatRegion : public SChatBaseRegion
{
	char cTalkerName[CONST_USERNAME];
    char cChatData[ MAX_CHAT_LEN ];
    WORD GetMySize()
    {
        _GetSize( SQAChatRegion, cChatData );
    }
};

//---------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////
enum CHANNEL_RULES
{
    CR_CLOSE    =     0x0000000f,//�����˲��ɽ� 
    CR_OPEN     =     0x000000f0,//�����˿ɽ�
    CR_FRIEND   =     0x00000f00,//���ѿɽ� 
    CR_TEAM     =     0x0000f000,
    CR_TONG     =     0x000f0000,
};

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ����Ƶ��
//=============================================================================================
DECLARE_MSG_MAP(SChannelBaseMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_CHANNEL)
//{{AFX
EPRO_CHANNEL_JOIN,
EPRO_CHANNEL_LEAVE,
EPRO_CHANNEL_TALK,
EPRO_CHANNEL_KICK,
EPRO_CHANNEL_CREATE,
EPRO_CHANNEL_DISMISS,
EPRO_CHANNEL_CHANGERULE,
EPRO_CHANNEL_GETMEMBERINFO,
EPRO_CHANNEL_GETCHANNELINFO,
EPRO_CHANNEL_WHISPER,
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================
struct SForChannelBroadCast
{
    SChannelBaseMsg * pChannelBaseMsg;
    int iSize;
};
struct SForGlobalTalkBroadCast
{
    SChatBaseGlobalMsg * pChatBaseGlobalMsg;
    int iSize;
};

DECLARE_MSG(SChangeRuleBaseChannelMsg,SChannelBaseMsg,SChannelBaseMsg::EPRO_CHANNEL_CHANGERULE)
struct SQChangeRuleChannelMsg : public SChangeRuleBaseChannelMsg
{
    DWORD dwStaticID;
    int iChannelRules;
};

struct SAChangeRuleChannelMsg : public SChangeRuleBaseChannelMsg
{
    DWORD dwChannelID;
    int iChannelRules;
    WORD wResult;
};


//=============================================================================================
DECLARE_MSG(SCreateBaseChannelMsg, SChannelBaseMsg, SChannelBaseMsg::EPRO_CHANNEL_CREATE)
struct SQCreateChannelMsg : public SCreateBaseChannelMsg
{

    DWORD dwCreatorStaticID;//the creator static id
    int   iChannelRules; 
	char  szChannelName[CONST_USERNAME];
};

struct SACreateChannelMsg : public SCreateBaseChannelMsg
{
	char  szCreatorName[CONST_USERNAME];
    DWORD dwChannelID;
    WORD wResult;
	char szChannelName[CONST_USERNAME];
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SDisMissBaseChannelMsg, SChannelBaseMsg, SChannelBaseMsg::EPRO_CHANNEL_DISMISS)
struct SQDisMissChannelMsg : public SDisMissBaseChannelMsg
{
    DWORD dwStaticID;//the creator static id 
    DWORD dwChannelID;
};

struct SADisMissChannelMsg :public SDisMissBaseChannelMsg
{
    DWORD dwChannelID;
    WORD wResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SJoinBaseChannelMsg, SChannelBaseMsg, SChannelBaseMsg::EPRO_CHANNEL_JOIN)
struct SQJoinChannelMsg : public SJoinBaseChannelMsg
{
    WORD  wResult;
    DWORD dwStaticID;
    DWORD  dwChannelID;

};

struct SAJoinChannelMsg : public SJoinBaseChannelMsg
{
    WORD  wResult;
	char  szPlayerName[CONST_USERNAME];
    DWORD dwStaticID;
    DWORD dwChannelID;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SLeaveBaseChannelMsg, SChannelBaseMsg, SChannelBaseMsg::EPRO_CHANNEL_LEAVE)
struct SQLeaveChannelMsg : public SLeaveBaseChannelMsg
{
    WORD  wResult;
    DWORD dwStaticID;
    DWORD  dwChannelID;
};

struct SALeaveChannelMsg : public SLeaveBaseChannelMsg
{
    WORD  wResult;
	char  szPlayerName[CONST_USERNAME];
    DWORD dwStaticID;
    DWORD dwChannelID;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SKickBaseChannelMsg, SChannelBaseMsg, SChannelBaseMsg::EPRO_CHANNEL_KICK)
struct SQKickChannelMsg : public SKickBaseChannelMsg
{
    WORD  wResult;
    DWORD dwCreatorStaticID;
    DWORD dwKickedStaticID;
    DWORD dwChannelID;
};

struct SAKickChannelMsg : public SKickBaseChannelMsg
{
    WORD  wResult;
	char  szPlayerName[CONST_USERNAME];
    DWORD dwCreatorStaticID;

    DWORD dwKickedStaticID;
    DWORD dwChannelID;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SChatBaseChannelMsg, SChannelBaseMsg, SChannelBaseMsg::EPRO_CHANNEL_TALK)
struct SQChatChannelMsg : public SChatBaseChannelMsg
{
    DWORD dwGlobalID;
    DWORD dwChannelID;
    WORD wLength;
    DWORD dwReceiverSID; // ��������XX��XX˵����ʽ
    char cCharData[MAX_CHAT_LEN];
    WORD GetMySize()
    {
        _GetSize(SQChatChannelMsg, cCharData);
    }
};

struct SAChatChannleMsg : public SChatBaseChannelMsg
{
    DWORD dwGlobalID;
    DWORD  dwChannelID;
    WORD  wLength;
	char  szPlayerName[CONST_USERNAME];
    char  cCharData[MAX_CHAT_LEN];
    WORD  GetMySize()
    {
        _GetSize(SAChatChannleMsg, cCharData);
    }
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
DECLARE_MSG(SGetBaseChannelMsg,SChannelBaseMsg, SChannelBaseMsg::EPRO_CHANNEL_GETMEMBERINFO)
struct SQGetChannelMsg : public SGetBaseChannelMsg
{
    DWORD dwStaticID ; //��ȡ������Ƶ����Ϣ����ҵ�StaticID
    DWORD dwChannelID; //��ȡ�õ������ҵ�ID
};

struct SAGetChannelMsg : public SGetBaseChannelMsg
{
    DWORD dwStaticID;  //��ȡ������Ƶ����Ϣ����ҵ�StaticID
    DWORD dwChannelID; //��ȡ�õ������ҵ�ID
    WORD wPlayerNumber;//������������ҵĸ���
    SChannelPlayerInfo ChannelPlayer[MAX_CHANNEL_PLAYER_NUMBER];
    WORD GetMySize()
    {
        return sizeof(SAGetChannelMsg)-(MAX_CHANNEL_PLAYER_NUMBER-wPlayerNumber)*sizeof(SChannelPlayerInfo);
    };
};
//=============================================================================================
DECLARE_MSG(SGetBaseChannelInfoMsg,SChannelBaseMsg,SChannelBaseMsg::EPRO_CHANNEL_GETCHANNELINFO)
struct SQGetChannelInfoMsg : SGetBaseChannelInfoMsg
{
};
//these messages are send one bye one 
struct SAGetChannelInfoMsg : SGetBaseChannelInfoMsg
{
    DWORD dwChannelID;
	char  szChannelName[CONST_USERNAME];
	char  szCreatorName[CONST_USERNAME];
    int   iRules;
};


//=============================================================================================
DECLARE_MSG(SChatBaseChannelWhisper,SChannelBaseMsg,SChannelBaseMsg::EPRO_CHANNEL_WHISPER)
struct SQChatChannelWhisper : SChatBaseChannelWhisper
{
    DWORD dwTalkerStaticID; // ������Ϣ����ҵ�StaticID
    DWORD dwReceiverStaticID;
    DWORD dwChannelID;
    char  szChatMsg[MAX_CHAT_LEN];
};
//---------------------------------------------------------------------------------------------
struct SAChatChannelWhisper : SChatBaseChannelWhisper
{
    DWORD dwTalkerStaticID;
	char szTalkerName[CONST_USERNAME];
	char szReceiverName[CONST_USERNAME];
    DWORD dwReceiverStaticID;
    DWORD dwChannelID;
    char szChatMsg[MAX_CHAT_LEN];
};

DECLARE_MSG(SChatBaseKilledMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_KILLED)//�¼ӵ�
struct SAChatKilledMsg : public SChatBaseKilledMsg
{
	char szName[CONST_USERNAME];
	char killerName[CONST_USERNAME];
	char killerTong[CONST_USERNAME];
	WORD regionID;
	WORD X;
	WORD Y;
};

// �������ͳһ����ʾ��Ϣ
DECLARE_MSG(SChatTipsMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_TIPS)
struct SAChatTipsMsg : public SChatTipsMsg
{
	enum
	{
		TIPS_SHOWTYPE_SYSTEM = 1, //ϵͳ
		TIPS_SHOWTYPE_WINDOWS = 2, //ϵͳ��ʾ����ʾһ���Ի���
	};
	bool bHasPrefix;		// �Ƿ���ǰ׺
	char cPrefix[11];		// ǰ׺
	WORD wShowType;			// ��ʾ���ͣ�ָ����ʲô�ط���ʾ��Ϣ
	char cChatData[500];	// ��ʾ����
	WORD GetMySize()
	{
		_GetSSize(SAChatTipsMsg, cChatData);
	};
};

DECLARE_MSG(SChatBroadcastMsg, SChatBaseMsg, SChatBaseMsg::EPRO_CHAT_BROADCAST)
struct SAChatBroadcastMsg : public SChatBroadcastMsg
{
	char BroadcasInfo[256];
	SAChatBroadcastMsg()
	{
		memset(BroadcasInfo,0,256);
	}
};

DECLARE_MSG(SChatBusiNessMsg,SChatBaseMsg,SChatBaseMsg::EPRO_CHAT_BUSINESS)
struct SAChatBusiNessMsg : public SChatBusiNessMsg //�̻�Ƶ�����������������Ϣ
{
	DWORD		 m_GID;				    // ���GID
	char	     m_Playername[40];		// ��Ϸ����(�س�)
	BYTE		 m_moneyType;			// ��Ǯ���� ��������
	DWORD		 m_AllPrice;			// �ܼ�
	SPackageItem m_SPackageItem;        // ������Ϣ
	WORD         m_Pos;					// λ��
	SAChatBusiNessMsg():m_GID(0),m_moneyType(0),m_AllPrice(0),m_Pos(0)
	{
		memset(m_Playername,0,40);
	}
};