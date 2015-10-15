#pragma once

#include "NetModule.h"
#include "pub\ConstValue.h"


struct SMatchMember
{
    BYTE			byTeamID;		//ֻ��1��2
	char			m_szName[CONST_USERNAME];
	WORD			m_wLevel;
};


////////////////////////////////////////////////////////////////////////////////////////
// �˶������������Ϣ
DECLARE_MSG_MAP(SSportsBaseMsg, SMessage, SMessage::EPRO_SPORT_MESSAGE)
//{{AFX
EPRO_ASKCREATEROOM,         // ���󿪼�
EPRO_ANSWERROOM,            // Ӧ��
EPRO_SENDTERM,              // ����
EPRO_JOINSIDE,              // ����
EPRO_SENDTEAMINFO,          // ��Ա��Ϣ
EPRO_KICKOUT,				// �ߵ���Ա
EPRO_MEMBERPOS,				// ��Աλ��
EPRO_LEADERMONEY,			// �ӳ�Я����Ǯ
EPRO_FACTIONINFO,			// ������Ϣ
EPRO_GAMINGHOUSE,           // �ĳ���Ϣ
//}}AFX
END_MSG_MAP()
//------------------------------------------------------------------------
enum TYPE { FREE_SIDE, FREE_FACTION};

typedef struct _tagTERM {
    PVOID lpAddr;
    DWORD dwSize;
}TERM, *LPTERM;

struct SIDE_TERM
{
    enum PLACE_TYPE {
        PT_PINGDI,
        PT_SHUANGXIAN,
        PT_SHIZI,
        PT_KOUZI,
        PT_QUEKOU,
        PT_JIUGONG,
        PT_DINGZI,
        PT_TIAOYAO
    };

    WORD    wPlace;
    WORD    wUseItem;
    WORD    wTime;
    DWORD   dwMoney;
    DWORD   dwExper;
	bool	bHaveGoods; //�Ƿ��в�����Ʒ
};

DECLARE_MSG(SAskCreateRoomMsg, SSportsBaseMsg, SSportsBaseMsg::EPRO_ASKCREATEROOM)
struct SQAskCreateRoomMsg : public SAskCreateRoomMsg
{
    BYTE  byType;
    DWORD dwSrcGID;
    DWORD dwDesGID;
};

struct SAAskCreateRoomMsg : public SQAskCreateRoomMsg
{
	char szName[CONST_USERNAME]; // �д跢��������
};

DECLARE_MSG(SAnswerRoomMsg, SSportsBaseMsg, SSportsBaseMsg::EPRO_ANSWERROOM)
struct SQAnswerRoomMsg : public SAnswerRoomMsg
{
    enum STATE {
        ST_OK,			
        ST_CANCEL,		
        ST_TERM_OK,     
        ST_TERM_CANCEL, 
        ST_ENTER_OK,    
        ST_MEMBER_EXIT, 

    };

    BYTE byState;
    DWORD dwMasterGID;
    DWORD dwOtherGID;
};

struct SAAnswerRoomMsg : public SQAnswerRoomMsg
{
};

DECLARE_MSG(SSendTermMsg, SSportsBaseMsg, SSportsBaseMsg::EPRO_SENDTERM)
struct SQSendTermMsg : public SSendTermMsg
{
    SIDE_TERM stTerm;
    DWORD dwMasterGID;
    DWORD dwOtherGID;
};

struct SASendTermMsg : public SSendTermMsg
{
    SIDE_TERM stTerm;
    DWORD dwMasterGID;
};

DECLARE_MSG(SJoinSideMsg, SSportsBaseMsg, SSportsBaseMsg::EPRO_JOINSIDE)
struct SQJoinSideMsg : public SJoinSideMsg
{
    BYTE  byType;
    DWORD dwSrcGID;
    DWORD dwDesGID;   
};

#define MAX_SMNUMER 16
DECLARE_MSG(SSendTeamMsg, SSportsBaseMsg, SSportsBaseMsg::EPRO_SENDTEAMINFO)
struct SASendTeamMsg : public SSendTeamMsg
{
    enum M_STATE {
        M_ONE,
        M_ALL,
        M_DELONE
    };

	WORD wState;  
    SMatchMember stTeam[MAX_SMNUMER];
};

DECLARE_MSG(SKickoutMsg, SSportsBaseMsg, SSportsBaseMsg::EPRO_KICKOUT)
struct SQKickoutMsg : public SKickoutMsg	//�ӳ��߳���Ա��������SASendTeamMsg��Ϣ��M_STATE::M_DELONE)
{
	char szName[CONST_USERNAME];
};

struct SMPOS {
    WORD wTeamID;
    WORD wPosX;
    WORD wPosY;
};

#define POS_MAXPLAYERS 200
DECLARE_MSG(SSMemberPosMsg, SSportsBaseMsg, SSportsBaseMsg::EPRO_MEMBERPOS)
struct SSAMemberPosMsg : public SSMemberPosMsg	
{
    WORD wNumber;
    SMPOS stMPos[POS_MAXPLAYERS];	

    WORD MySize()
    {
        return sizeof(SSAMemberPosMsg) - sizeof(SMPOS)* (POS_MAXPLAYERS - wNumber);
    }
};

DECLARE_MSG(SLeaderMoneyMsg, SSportsBaseMsg, SSportsBaseMsg::EPRO_LEADERMONEY)
struct SALeaderMoneyMsg : public SLeaderMoneyMsg	
{
    DWORD dwMasterMoney;
    DWORD dwSubMasterMoney;
};

DECLARE_MSG(SFactionInfoMsg, SSportsBaseMsg, SSportsBaseMsg::EPRO_FACTIONINFO)
struct SAFactionInfoMsg : public SFactionInfoMsg	
{
	char szAskFactionName[CONST_USERNAME];
    DWORD dwMasterID;
};

//////////////////////////////////////////////////////////////////////////
// gaminghouse
struct _GPLAYERBASEINFO {
    BYTE    m_bySex:1;              // ��ҽ�ɫ�Ա�
    BYTE    m_byBRON:3;             // �������ʾͼƬ����==1��ʾ��ʦ
    BYTE    m_bySchool:4;           // �����������
};

struct _GAMINGPLAYER : 
    public _GPLAYERBASEINFO 
{
	char szName[CONST_USERNAME];
    WORD wResult; 
    WORD wPosID;
    DWORD dwWager;
    DWORD dwMoney;
    LONG nGrade;
};

DECLARE_MSG_MAP(SGamingHouseBaseMsg, SSportsBaseMsg, SSportsBaseMsg::EPRO_GAMINGHOUSE)
//{{AFX
EPRO_SHOWWAGERINFO,                 // ��ע
EPRO_GAMINGCAST,                    // ������
EPRO_GAMINGPLAYERINFO,              // �ĳ���Ա��Ϣ
//}}AFX
END_MSG_MAP()

DECLARE_MSG(SShowWagerInfoMsg, SGamingHouseBaseMsg, SGamingHouseBaseMsg::EPRO_SHOWWAGERINFO)
struct SQShowWagerInfoMsg : public SShowWagerInfoMsg
{
    WORD wPosID;
    DWORD dwWager;  
};

struct SAShowWagerInfoMsg : public SShowWagerInfoMsg
{
    WORD wPosID;  
};

DECLARE_MSG(SShowGamingcastMsg, SGamingHouseBaseMsg, SGamingHouseBaseMsg::EPRO_GAMINGCAST)
struct SQShowGamingcastMsg : public SShowGamingcastMsg
{
    WORD wPosID;
};

struct SAShowGamingcastMsg : public SShowGamingcastMsg
{
    WORD wPosID;
   // WORD wNumber;
};

DECLARE_MSG(SGamingPlayerInfoMsg, SGamingHouseBaseMsg, SGamingHouseBaseMsg::EPRO_GAMINGPLAYERINFO)
struct SQGamingPlayerRequestMsg : public SGamingPlayerInfoMsg
{
    enum GPR_TYPE 
    {
        GPR_LEAVE,
    };

    WORD wPosID;
    WORD wRequest;
};

struct SAGamingPlayerInfoMsg : public SGamingPlayerInfoMsg
{
    WORD wNumber;
    _GAMINGPLAYER GPlayer[10];

    WORD MySize()
    {
        return sizeof(SAGamingPlayerInfoMsg) - sizeof(_GAMINGPLAYER)*(10 - wNumber);
    }
};
