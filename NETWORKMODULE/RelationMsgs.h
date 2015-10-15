#include "NetModule.h"
//#include "PlayerTypedef.h"
#include "HeroTypeDef.h"
#pragma once
DECLARE_MSG_MAP(SRelationBaseMsg, SMessage, SMessage::EPRO_RELATION_MESSAGE)
//{{AFX
EPRO_CHARACTER_FRIENDLIST_INFO_ADD, // ��Һ����б�
EPRO_CHARACTER_FRIENDLIST_INFO_DEL, // ��Һ����б�
EPRO_FRIENDONLINE,                  // ��������֪ͨ
EPRO_REFRESH_ONLINE_FRIENDS,        // ���ߺ����б�ˢ��

//����������Ϣ��ʱ�������
EPRO_USEITEMTO_FRIENDS,				// �Ժ���ʹ�õ���
EPRO_REGUEST_ADDFRIEND,				// ����������
EPRO_REGUEST_ROSERECOD,				// ����ȡ���ͻ���¼

EPRO_FRIEND_FAIL_MSG, //����ʧ����Ϣ
//}}AFX
END_MSG_MAP()



enum RelationType {
    RT_FRIEND,			//����
    RT_BLACKLIST,		//������
	RT_ENEMIES,		//����
	RT_MATE,				//���
	RT_TEMP,				//��ʱ����
};

enum DEGREEITEM
{
	ITEM_DEGREE_ROSE	= 8003000,    //�����ڼ���
	ITEM_RED_ROSE,					   //��õ��
	ITEM_BLUE_ROSE,						//��õ��
	ITEM_WHITE_ROSE,				    //��õ��
	ITEM_YELLOW_ROSE,			    //��õ��
	ITEM_PURPLE_ROSE,				   //��õ��
};

struct FriendData
{
	char  bFriendName[CONST_USERNAME];
	WORD wIconIndex;
	WORD wLevel;
	DWORD wFightPower;
	DWORD dwSid; //���Sid
	BYTE    byType;     // 0 ���� ,1 �Ƽ� 2 ��� 
	BYTE   bonlinestate;	// 0 ���� 1����
	BYTE bySendFlag; //�����������
	BYTE byReceiveFlag;//��ȡ�������
};
//=============================================================================================
DECLARE_MSG(SAddFriendMsg, SRelationBaseMsg, SRelationBaseMsg::EPRO_CHARACTER_FRIENDLIST_INFO_ADD) //���ӵ��б���
// ���Ӻ���(����Ϊ���й�ϵ)
struct SAAddFriendMsg : public SAddFriendMsg
{
	FriendData m_friendData;
//     char    szName[CONST_USERNAME];
// 	char		sHeadIcon[CONST_USERNAME];
//     BYTE    byType;     // 0 ���� ,1 �Ƽ� 2 ��� 
// 	WORD  wLevle;		//�ȼ�
};
struct SQAddFriendMsg : public SAddFriendMsg
{
	DWORD dwFriendSid;
    char    szName[CONST_USERNAME];
	BYTE    byType; //0 ���� ,1 �Ƽ� 2 ��� 5 ����ȫ����������Ϸ�� ��7���ͺ��������� 8 ��ȡ������������
};


DECLARE_MSG(SFriendFailMsg, SRelationBaseMsg, SRelationBaseMsg::EPRO_FRIEND_FAIL_MSG) //���ӵ��б���
struct SAFriendFailMsg : public SFriendFailMsg
{
	BYTE    byType; // 1 �Լ����Ѵﵽ���� 2 �Է����Ѵﵽ���� 3 �������
	SAFriendFailMsg()
	{
		byType = 0;
	}
};


DECLARE_MSG(SAgreeAddFriendMsg, SRelationBaseMsg, SRelationBaseMsg::EPRO_REGUEST_ADDFRIEND)
struct SQAgreeAddFriendMsg: public SAgreeAddFriendMsg
{
	char    szName[CONST_USERNAME];
};
struct SAAgreeAddFriendMsg: public SAgreeAddFriendMsg
{
	char    szName[CONST_USERNAME];
    BYTE    byResult; // 0 ��ͬ�� ,1 ͬ��
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
DECLARE_MSG(SDelFriendMsg, SRelationBaseMsg, SRelationBaseMsg::EPRO_CHARACTER_FRIENDLIST_INFO_DEL)
// ɾ������
struct SQDelFriendMsg : public SDelFriendMsg
{
    char    szName[CONST_USERNAME];
	BYTE    byType; //0 ���� ,1 ����,2����
};
struct SADelFriendMsg : public SDelFriendMsg
{
    BYTE    byResult;
    char    szName[CONST_USERNAME];
	BYTE    byType; //0 ���� ,1 ����,2����
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SFriendOnlineBaseMsg,SRelationBaseMsg, SRelationBaseMsg::EPRO_FRIENDONLINE)
struct SFriendOnlineMsg : SFriendOnlineBaseMsg	// ԭ�������Ǹ��º�������״̬���ָ�Ϊ�������º���ɱ�ˡ���ɱ����������Ѩ��״̬
{
// ������Һ���״̬
	enum FRIEND_STATE
	{
		FRIEND_STATE_ONLINE,	// ����
		FRIEND_STATE_OUTLINE,	// ����
		FRIEND_STATE_HANGUP,	// �һ�
		FRIEND_STATE_KILLOTHER, // ��ܱ���
		FRIEND_STATE_BEKILLED,	// �����˴��
		FRIEND_STATE_LEVELUP,	// ����
		FRIEND_STATE_PASSPULSE,	// ��Ѩ
		FRIEND_STATE_LEVELUPWITHITEM,	// ���������͵��� 
		FRIEND_STATE_TIMEOUTWITHEXP,	// ����ʱ��þ���
		FRIEND_STATE_REFRESH,	// ˢ�£����ܶȣ���
	}eState;

    char	cName[CONST_USERNAME];		 	// ����
	BYTE    byType;			 	// 0 ���� ,1 ����

	char	szOtherName[CONST_USERNAME]; 		// ���ߴ�ܣ����߱���ܣ��������
	WORD	byFriendLevel;	 		// ���ѵȼ�
	//char	szPulsePassed[20];			// ���ѳ忪��Ѩ������

	//DWORD	dwgiveexp;			// ���͵ľ���
	//DWORD   dwgiveItemID;		// ���͵ĵ���ID
	//SYSTEMTIME giveTime;			// ���;����ʱ��
	//WORD		wFriendCount;		//���ߺ��Ѹ���
   // WORD    wDearValue;			// ���ܶ�
};
struct SAFriendOnlineMsg : public SFriendOnlineBaseMsg	
{
	// ������Һ���״̬
	enum FRIEND_STATE
	{
		FRIEND_STATE_ONLINE,	// ����
		FRIEND_STATE_OUTLINE,	// ����
		FRIEND_STATE_HANGUP,	// �һ�
		FRIEND_STATE_KILLOTHER, // ��ܱ���
		FRIEND_STATE_BEKILLED,	// �����˴��
		FRIEND_STATE_LEVELUP,	// ����
		FRIEND_STATE_PASSPULSE,	// ��Ѩ
		FRIEND_STATE_LEVELUPWITHITEM,	// ���������͵���
		FRIEND_STATE_TIMEOUTWITHEXP,	// ����ʱ��þ���
		FRIEND_STATE_REFRESH,	// ˢ�£����ܶȣ���
	}eState;

		 char	cName[CONST_USERNAME];		 	// ����
		 //DWORD	dwgiveexp;	// ���ӵľ���
		 //DWORD   dwgiveItemID;	// ���͵ĵ���ID
};


const int MAX_PEOPLE_NUMBER  = 100 ;	///�������
const int MAX_TEMP_NUMBER			= 50;		//��ʱ����������
DECLARE_MSG(SRefreshFriendsBaseMsg,SRelationBaseMsg,SRelationBaseMsg::EPRO_REFRESH_ONLINE_FRIENDS)
struct SRefreshFriendsMsg : SRefreshFriendsBaseMsg
{
	WORD wFriendCount; //������
	WORD wSendCount; //��ǰ���ʹ���
	WORD wReceiveCount; //��ǰ��ȡ����
	WORD wSendLimit; //���ʹ�������
	WORD wReceiveLimit; //��ȡ��������
	WORD wStaminaNum; //ÿ����������
	FriendData fdata[MAX_PEOPLE_NUMBER];
// 	
};

//---------------------------------------------------------------------------------------------
DECLARE_MSG(SUseFriendItemMsg,SRelationBaseMsg, SRelationBaseMsg::EPRO_USEITEMTO_FRIENDS)
struct SQUseFriendItemMsg : SUseFriendItemMsg	// �Ժ��ѻ������ʹ�õ���
{
	WORD wCellPos; // ����
	WORD wItemNum;
	char	cName[CONST_USERNAME];		 	// ���ѵ�����
	BYTE    byType;			 	// 0 ���� ,1 ����
};
struct SAUseFriendItemMsg : SUseFriendItemMsg	// �Ժ��ѻ������ʹ�õ���
{
	WORD wItemNum;		//��������
	DWORD wIndex ;			//����ID
	char	cName[CONST_USERNAME];		 	// ����������
	BYTE    byType;			 	// 0 ���� ,1 ����
	int	nDegree ;				//���ӻ���ٵĺ��Ѷ�
};


DECLARE_MSG(SGetRoseRecodMsg,SRelationBaseMsg, SRelationBaseMsg::EPRO_REGUEST_ROSERECOD)
struct SQGetRoseRecodMsg : SGetRoseRecodMsg	// �Ժ��ѻ������ʹ�õ���
{
	UINT64		m_PlayerRoseUpdata;//��ҵ�ǰ�ͻ�״ֵ̬�����ֵ�����ж��Ƿ���¿ͻ���
};
