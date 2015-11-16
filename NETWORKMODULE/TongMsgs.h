#pragma once

#include "networkmodule/netmodule.h"
#include "..\���������\GameObjects\SanguoCode\Common\ConfigManager.h"
#include "SanguoPlayerMsg.h"


#define MAX_FACTION_NUMBER 50 //globalConfig.FactionPlayerMaxNum 
//#define MAX_MEMBER_NUMBER 200	// ���ܰ��ɼ����������ô����		2015-5-28 �����
#define MAX_MEMBER_NUMBER 50//globalConfig.FactionPlayerMaxNum 	// ���ܰ��ɼ����������ô����
#define MAX_MEMO_NUMBER 50//globalConfig.FactionMemoMaxNum //�������Գ�������
#define MAX_NOTICE_NUMBER 100
#define MAX_JoinListCount 20
#define MAX_SENDFACTIONS 10   //���͵��������
#define FACTIONDATA_LENTH		25 * 1024 *2    //�������ݴ�С
#define MAX_SENDFACTIONDATA 1   //���͵��������
#define CONSTMAILTITLE  256
#define CONSTMAILCONTENT  512
#define MAX_EQUIPTLISTNUM  20	//�鿴װ��������е��������
#define MAX_QUESTEQUIPTBUFFER  1024   //����鿴װ����Buff����С
//CGlobalConfig globalConfig; globalConfig = CGlobalConfig globalConfig;
struct SimFactionData
{
	

	struct RequestJionPlayer
	{

		DWORD headIconID;
		char name[CONST_USERNAME];
		int level;
		RequestJionPlayer()
		{
			headIconID = 0;
			memset(name, 0, sizeof(name));
			int level = 0;
		}

	};

	struct SimFactionInfo
	{
		char szFactionName[CONST_FACTNAME];			// ��������
		char szCreatorName[CONST_USERNAME];			// ��ʼ������
		char szFactionMemo[100];			// ���������ݶ�150���ַ�
		//char factionIcon[MAX_PlayerIcon];			// ����ͼ��
		int factionIcon;
		BYTE  byMemberNum;				// ��Ա����
		BYTE  byFactionLevel;			// ��ǰ���ɵȼ�
		WORD  factionId;                // ����Id����IDΪ���ɵ�Ψһ�Ա�־�������ɳ���Ҳ������������Ϊ����ID����
		DWORD dwFactionProsperity;	    // ���ɷ��ٶȣ���ǰ�ǰ��ɻ�Ծ��
		DWORD iMoney;					// �����ʽ�(��ǧΪ��λ)


		DWORD dwWood;					// ľ��
		DWORD dwStone;					// ʯͷ
		DWORD dwMine;					// ��ʯ
		DWORD dwPaper;					// ֽ��
		DWORD dwJade;					// ��ʯ		


        struct {
            QWORD parentId  : 24;       // ���ɴ��ŵĸ�����ID
            QWORD uniqueId  : 40;       // ���ɴ��ŵ�ΨһID
        } buildingStub;                 // ͨ�����������ҵ����ɴ��ţ�
		BYTE byNpcNum;					// NPC����

        BYTE byOpenRApplyJoin;          // ����Զ����� 1���� 0�ر�

		//��Ҽ��������
		//std::map < std::string, RequestJionPlayer > RJlist;
		RequestJionPlayer RjoinList[MAX_JoinListCount];
		int Rhlistcount ;
		SimFactionInfo()
		{
			Rhlistcount = 0;
			memset(RjoinList, 0, sizeof(RjoinList));
		}
		//BYTE  mapTemplateId;            // ���ɳ����ĵ�ͼģ����
		//WORD  factionRegionId;          // ���ɳ���Id����Id������Ψһ�ģ���ֻ��16λ����Ҫ��Ϊ�ڵ�½��������ע���ã�
		//WORD  parentRegionId;           // ���ɳ����ĸ�����Id
		//WORD  xEntry, yEntry;           // ���ɳ����ڸ������ϵ���ڵ�����
	};

	struct SimMemberRight
	{
		//----------------------------------------
		DWORD Level:9;					// ��Ա�ڰ����еĵȼ�
		DWORD Title:4;					// ְ�� ���������������󻤷����һ�����̳������������ʦ��[��ҽ]����ʦ��[��ҽ]������
		DWORD School:3;					// ����������ɼ��Ա� �������С��䵱�С�����Ů�������С�ħ���С��䵱Ů������Ů��ħ��Ů����
		DWORD Camp:2;					// ��ҵ���Ӫ ����а��������
		DWORD IsOnline:1;				// �Ƿ�����

		DWORD Appoint:1;				// ����Ȩ
		DWORD Accept:1;					// ����Ȩ
		DWORD Authorize:1;				// ��Ȩ
		DWORD Dismiss:1;				// ���Ȩ
		DWORD Editioner:1;				// ������Ȩ��
		DWORD CastOut:1;				// ����Ȩ

		//DWORD EspecialTitle:1;		// �����ڳ�Ա�����ر�ƺŵ�Ȩ�� ( *ֻ�а�����ʹ�� )
		//DWORD Resignation:1;			// ��ְȨ (*��ְ����Ҫ��Ȩ)
		//DWORD QuitFaction:1;			// �˰�
		//DWORD IsMsg:1;				// �Ƿ�������
		//-----------------------------------------����28λ����ʣ�µ�13λ�����Ժ���չ
	};


	

	struct SimMemberInfo : public SimMemberRight
	{
		DWORD dwJoinTime;				// (���˼������/�ı�ְλ)��ʱ��
		DWORD iDonation;				// ���˾��׽�Ǯ(��ǧΪ��λ)
		DWORD dwAccomplishment;			// ���˳ɾ�
		DWORD dwWeeklySalary;			// ��н
		DWORD  PlayerIconID;			//��Աͷ��
		char  szName[CONST_USERNAME];				// ��Ա������
		char  szCustomTitle[CONST_USERNAME];		// �����ڲ��ɰ���ָ��������ƺ�
		
	};

	SimFactionInfo stFaction;
	SimMemberInfo  stMember[MAX_MEMBER_NUMBER];
};

// ����汾���ڷ����������ڴ�ʹ�õ�,��������ȫ��
struct SFactionData
{
	struct SFaction : public SimFactionData::SimFactionInfo
	{
		DWORD   dwTotalMoney;			// ÿ�������ĵ��ʽ�
		BYTE	byDelete;
	};

	struct SMember : public SimFactionData::SimMemberInfo
	{
		DNID  dnidClient;
		DWORD wServerID;				// ���������������IDֵ
		DWORD LastLineTime;	
	};

	SFaction stFaction;
	SMember stMember[MAX_MEMBER_NUMBER];
};

// ����汾����DB�͵�½�����õ�
struct SaveFactionData
{
	struct SaveFactionInfo : public SimFactionData::SimFactionInfo
	{
	};

	struct SaveMemberInfo : public SimFactionData::SimMemberInfo
	{
	};

	SaveFactionInfo stFaction;
	SaveMemberInfo  stMember[MAX_MEMBER_NUMBER];
};

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ��������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(STongBaseMsg,SMessage,SMessage::EPRO_TONG_MESSAGE)
//{{AFX
EPRO_TONG_SYSMSG,
EPRO_TONG_REQUARE_FACTION_NAME,     // ֪ͨ�ͻ�������������֣����Դ������ɣ�
EPRO_TONG_QUEST_JOIN,               // ����������Ҽ����Լ����ɣ�
EPRO_TONG_MEMBER_UPDATE,            // ���ɳ�Ա�ĵ������£�
EPRO_TONG_DELETE,				    // ɾ������
EPRO_TONG_INITFACTION_INFO,			// ���Ͱ�����Ļ������ݸ��ͷ���
EPRO_TONG_SEND_SERVER,				// ���ɹ㲥��Ϣ�õ�
EPRO_INPUT_MEMO,					// Ҫ�������������
EPRO_TONG_UPDATA_FACTION_HEAD,		// �ͷ���������°�������
EPRO_TONG_MEMBER_MANAGER,			// ���ɳ�Ա����
EPRO_FACTIONBBS_MESSAGE,			// ���ɵ����Թ���
EPRO_TONG_ENTERREGION_NOTIFY,		// ��ҽ�����ɳ���ʱ����ʾ��Ϣ��
EPRO_TONG_UPDATA_RECRUIT,           // ������ļ״̬
EPRO_TONG_REMOTE_APPLY_JOIN,        // Զ���������
EPRO_TONG_RECV_RAPPLY_JOIN,         // ����Զ���������

EPRO_TONG_CREATE_FACTION,			//��������
EPRO_TONG_FACTION_INFO,               // �������ݣ�
EPRO_TONG_FACTION_LIST,               // ���о����б�
EPRO_TONG_QUEST_JOINFACTION,           // ����������
EPRO_TONG_QUEST_LEAVEFACTION,           // �����뿪����
//EPRO_TONG_SHOW_JOINLIST,           // ���ų��鿴��ǰ���������ŵ���Ա�б�
EPRO_TONG_MANAGER_MEMBER,          // ���ų��Ծ��ŵĹ���
EPRO_TONG_FACTION_DELETE,				    // ɾ������
EPRO_TONG_SYNFACTION,				    // ͬ��������Ϣ
EPRO_TONG_SCENEDATA,				    // ���Ÿ�����Ϣ
EPRO_TONG_SCENEMAPDATA,				    // ���Ÿ����ؿ���Ϣ
EPRO_TONG_SCENECHALLENGE,				    // ���Źؿ���ս
EPRO_TONG_SCENEFINISH,				    // ���Ÿ�����ս���
EPRO_TONG_SCENEHURTRANK,				    // ���Ÿ����˺�����
EPRO_TONG_INITFACTIONSALARY,				    // ����ٺ»��ʼ��
EPRO_TONG_GETFACTIONSALARY,				    // ��ȡ����ٺ»
EPRO_TONG_NOTICE,									//���Ź���
EPRO_TONG_MODIFYNOTICE,									//�����޸Ĺ���
EPRO_TONG_OPERATELOG,									//���Ų�����־
EPRO_TONG_SENDEMAILTOALL,							//����Ⱥ���ʼ�
EPRO_TONG_SHOWEQUIPT,							//���Ų鿴�������뵽��װ���б�
EPRO_TONG_QUESTEQUIPT,							//��������װ��
EPRO_TONG_SHOWQUESTSTATUS,							//�鿴����״̬
EPRO_TONG_CANCELEDQUEST,							//ȡ������װ��

//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------

/************************************/
/* �������ݲ��ֵ���Ϣ����				*/
/************************************/
//=============================================================================================
// ����������Ҽ����Լ����ɣ�
DECLARE_MSG(SQuestJoinMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_QUEST_JOIN)
// ������֪ͨĿ����Ҵ�ȷ������������
struct SAQuestJoinMsg :
    public SQuestJoinMsg
{
    DWORD dwSrcGID;     // �������뷢���˵�GID
    DWORD dwCheckID;    // ����У���ID
};

// �ͻ����ύȷ�Ͻ����������
struct SQQuestJoinMsg :
    public SQuestJoinMsg
{
    DWORD dwSrcGID;     // �������뷢���˵�GID���ش���
    DWORD dwCheckID;    // ����У���ID���ش���
    BOOL bAccept;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================

//---------------------------------------------------------------------------------------------

//=============================================================================================
// ɾ��һ������
DECLARE_MSG(SDeleteFactionrMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_DELETE)
struct SQDeleteFactionrMsg:
    public SDeleteFactionrMsg
{
	// ����Ƿ�����½�ã���½��ת�ظ���������������Ա����,���ɾ����
	char szFactionName[CONST_USERNAME];// ��Ҫ�����İ�����
};

struct SADeleteFactionrMsg:
    public SDeleteFactionrMsg
{
	// ����Ƿ����ͻ����õ�
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// ���ɳ�Աһ��ĸ��£�
DECLARE_MSG(SInitFactionInfoMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_INITFACTION_INFO)
// ������֪ͨ�ͻ��˸��°�������
struct SAInitFactionInfoMsg:
	public SInitFactionInfoMsg
{
	BYTE byMemberNum;				// ��Ա����
	char szFactionName[CONST_USERNAME];			// ��������
	char szCreatorName[CONST_USERNAME];			// ��ʼ������
	SimFactionData::SimMemberInfo stMemberInfo[MAX_MEMBER_NUMBER];
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// ������°�������
DECLARE_MSG(SUpdataFactionHeadMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_UPDATA_FACTION_HEAD)
// �ͻ���������°�������
struct SQUpdataFactionHeadMsg:
	public SUpdataFactionHeadMsg
{
};

struct SAUpdataFactionHeadMsg:
	public SUpdataFactionHeadMsg
{
	char szFactionMemo[50];			// ���������ݶ�50���ַ�
	BYTE  byFactionLevel;			// ��ǰ���ɵȼ�
	DWORD dwFactionProsperity;		// ���ɷ��ٶȣ���ǰ�ǰ��ɻ�Ծ��
	DWORD dwTotalMoney;				// �����ĵ��ʽ�

	__int64 iMoney;					// �����ʽ�
	DWORD dwWood;					// ľ��
	DWORD dwStone;					// ʯͷ
	DWORD dwMine;					// ��ʯ
	DWORD dwPaper;					// ֽ��
	DWORD dwJade;					// ��ʯ	

    BYTE  byOpenRApplyJoin;         // ����Զ����� 1���� 0�ر�
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// ���ɳ�Ա�������£�
DECLARE_MSG(SUpdateMemberMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_MEMBER_UPDATE)
// ������֪ͨ�ͻ��˸���һ����Ա
struct SAUpdateMemberMsg:
    public SUpdateMemberMsg
{
    enum 
	{
		E_UPDATE_MEMBER,		// ��ӻ��߸���
        E_DELETE_MEMBER,		// ɾ��
        E_DELETE_MEMBER_LEAVE	// �뿪, �ǰ�������
    };

	BYTE byType;            // ���β���������
	SimFactionData::SimMemberInfo stInfo;
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// ����������Ҽ����Լ����ɣ�
DECLARE_MSG(SSendServerMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_SEND_SERVER)
struct SQSendServerMsg :
    public SSendServerMsg
{
	enum { TALK, DELFACTION, UPDATEMEMBER, CITYOWENTALK, MSGCOPY }; //CITYOWENTALKҪ�������Ҵ��˻�������
	BYTE byMsgType;
	WORD wRegion;			// 0=ȫ������
	char szFactionName[CONST_USERNAME];
	char szBuffer[1024*5];
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// �޸İ�������
DECLARE_MSG(SInputMemoMsg, STongBaseMsg, STongBaseMsg::EPRO_INPUT_MEMO)
struct SQInputMemoMsg :
    public SInputMemoMsg
{
	char szFactionMemo[50];
};

struct SAInputMemoMsg :
    public SInputMemoMsg
{
	char szFactionMemo[50];
};
//---------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------
DECLARE_MSG(SMemberManager, STongBaseMsg, STongBaseMsg::EPRO_TONG_MEMBER_MANAGER)
struct SQMemberManager 
	: public SMemberManager
{
	enum { AUTHORIZE, EMPOWER, SHELVE, TITLENAME ,ABDICATE, BANISH, QUIT, KCAUTHORIZE, KCQUIT };
	WORD tpye;
	char streamData[1024];
};

struct SAMemberManager
	: public SMemberManager
{

};
//---------------------------------------------------------------------------------------------

DECLARE_MSG(_SEntryRegionNotify, STongBaseMsg, STongBaseMsg::EPRO_TONG_ENTERREGION_NOTIFY)
struct SEntryRegionNotify
	: public _SEntryRegionNotify
{
	WORD type; // ���ڿ��� streamData ���뷽ʽ�ı�־����Ҫ���ں�����չ��������ʱû�ã���0����
	char streamData[256];
};

//---------------------------------------------------------------------------------------------
DECLARE_MSG(SRemoteApplyJoin, STongBaseMsg, STongBaseMsg::EPRO_TONG_REMOTE_APPLY_JOIN)
struct SQRemoteApplyJoin : public SRemoteApplyJoin
{
   DWORD dwApplyGID;             // �����˵�ID
   //DWORD dwTongID;               // Ҫ�����5����ID
   char szTongName[CONST_USERNAME];
};

struct SARemoteApplyJoin: public SRemoteApplyJoin
{
    enum // ����֪ͨ
    { 
        SA_AJ_MSGBOX,        // ���������
        SA_AJ_ADOPT,         // ͨ������
       
        // ʧ����Ϣ
        SA_AJ_ER_BUSY,       // ����æ
        SA_AJ_ER_REFUSE,     // �ܾ�����
        SA_AJ_ER_CLOSE,      // �ر�����
        SA_AJ_ER_OUTLINE,    // ����������
        SA_AJ_ER_HAVE,       // �������Ѿ��а�
        SA_AJ_ER_NOTONG,     // ���ɲ�����
        SA_AJ_ER_AJNULL,     // �����˲�����
        
        SA_AJ_ER_UNKONWN     // δ֪������Ϣ
    };
    WORD type;
    DWORD dwApplyGID;             // �����˵�ID
};

//---------------------------------------------------------------------------------------------

DECLARE_MSG(SRecvOperApplyJoin, STongBaseMsg, STongBaseMsg::EPRO_TONG_RECV_RAPPLY_JOIN)
struct SQRecvOperApplyJoin: public SRecvOperApplyJoin
{
    enum // ���ڲ���
    { 
        SA_RRAJ_ADOPT,         // ͨ������
        SA_RRAJ_REFUSE,        // �ܾ����� 
    };
    WORD type;
    DWORD dwSrcGID; 
    DWORD dwApplyGID;         // �����˵�ID
};

//---------------------------------------------------------------------------------------------
DECLARE_MSG(SUpdateRecruit, STongBaseMsg, STongBaseMsg::EPRO_TONG_UPDATA_RECRUIT)
struct SQSUpdateRecruit : public SUpdateRecruit
{
    BYTE tpye;                // 0�ر� 1����
    DWORD dwApplyGID;         // �����˵�ID
};

struct SASUpdateRecruit: public SUpdateRecruit
{
    BYTE tpye;
};



//--2015/07/31  �¼��޸ĵľ���

struct FactionMember   //�������緢�͵ĳ�Ա����
{
	char strMemberName[CONST_USERNAME];
	WORD wLevel;						//�ȼ�
	WORD wFightPower;			//ս����
	WORD wPlayerIcon;//���ͷ��
	WORD  wWeekActiveValue;//7�ջ�Ծ�ȹ���
	BYTE  bisonline;				//�Ƿ�����
	BYTE  Title;	//�Լ��ڰ����е�ְλ
};

struct FactionInfoList  //���Ծ����б����г����ŵļ���Ϣ
{
	char szFactionName[CONST_USERNAME];			// ��������
	char szCreatorName[CONST_USERNAME];			// ��ʼ������ 
	WORD  byFactionLevel;			// ��ǰ���ɵȼ�
	WORD FactionIcon;				//����ͼ��
	BYTE  CurMemberNum;				// ��Ա����
	DWORD  FactionID;//����ID
};

// ֪ͨ�ͻ�������������֣����Դ������ɣ�
DECLARE_MSG(SCreateFaction, STongBaseMsg, STongBaseMsg::EPRO_TONG_CREATE_FACTION)
// �ͻ����ύ����İ������ֵ�������
struct SQCreateFaction :public SCreateFaction
{
	char szFactionName[CONST_USERNAME];
	WORD	wFactionIcon;		//����ͼ��
};
// ���������ؽ��
struct SACreateFaction :public SCreateFaction
{
	BYTE		bresult;//  1 �ɹ� 2 Ԫ������ 3 �����ظ� 4 ���ֳ��Ȳ��� 5 ���ֲ�����Ҫ��
};


//���������������
DECLARE_MSG(SFactioninfo, STongBaseMsg, STongBaseMsg::EPRO_TONG_FACTION_INFO)
struct SQFactioninfo : public SFactioninfo
{
};

struct SAFactioninfo : public SFactioninfo
{
	char szFactionName[CONST_USERNAME];			// ��������
	char szCreatorName[CONST_USERNAME];			// ��ʼ������ 

	WORD FactionIcon;
	WORD  byFactionLevel;			// ��ǰ���ɵȼ�
	BYTE  CurMemberNum;				// ��Ա����
	BYTE	  QuestJoinNum;			//���������Ա����
	DWORD  FactionID;//����ID
	DWORD  dActiveValue;		//���Ż�Ծ��
	DWORD   dfactionexp;  //���ŵľ���ֵ
	BYTE  Title;	//�Լ��ڰ����е�ְλ
	FactionMember  m_factionmember[MAX_FACTION_NUMBER];
};
//������ʹ�õ����ݽṹ
typedef struct ITEMDATA   //���߽ṹ��
{
	BYTE  bitemtype;				//��������
	DWORD  ditemIndex;    //����ID
	DWORD  ditemnum;     //��������
}itemdata;

typedef struct FACTIONSCENE   //���Źؿ�
{
	BYTE   bscenestatus;		//����״̬  0 �����ر�δͨ�� 1 �����ر���ͨ��  2 ��������δͨ�� 3 ����������ͨ�� 
	WORD   wTurnOnNum;  //�����ؿ�����
	DWORD dsceneID;     //����ID
}factionscene;

typedef struct FACTIONSCENEMAP   //���Źؿ�����
{
	BYTE  bscenemapIndex;//�ؿ�����
	BYTE   bscenestatus;		//����״̬  0 �����ر�δͨ�� 1 �����ر���ͨ��  2 ��������δͨ�� 3 ����������ͨ�� 
	double   dmonsterHP;		//��ǰ�ؿ��еĹ�����Ѫ��
	double   dmonstercurrentHP;	//�ؿ����ﵱǰѪ�� 
}factionscenemap;

typedef struct FACTIONSCENERANK  //�����˺�����
{
	char playername[CONST_USERNAME];  //�������
	double  dHurt;		//����˺�
	DWORD  drewardnum; //�ɵõ��ľ���������
}factionscenerank;

typedef struct FACTIONSALARYDATA   //����ٺ»����
{
	//DWORD ditemlist[10];  //����ID + ����  ��ÿ�ĸ��ֽڣ�
	itemdata  sitemdata[MAX_SENDFACTIONS];
	DWORD  dActivityValue;//���ӵĻ�Ծ��
	DWORD  dExp; //���ӵľ���
	WORD  wcostgold;	//��Ҫ���ѵ�Ԫ������
}factionsalarydata;

typedef struct OPERATELOG    //������־
{
	BYTE   btype;			//����
	char  strName[CONST_USERNAME];  //���������
	DWORD  dParamID;			//�ھ��Ÿ����б�ʾ���Ÿ���ID
}operatelog;


typedef struct REQUESTELIST     //����װ��
{
	WORD  wRank;
	WORD wIconIndex; //ͷ��
	WORD  wLevle;// �ȼ�
	char   strname[CONST_USERNAME]; //�������
}requestlist;

//��������б�
DECLARE_MSG(SFactionList, STongBaseMsg, STongBaseMsg::EPRO_TONG_FACTION_LIST)
struct SQFactionList : public SFactionList
{
	BYTE		btype;			//�������� 0  ȫ�����  1 ID���� 2 ���ֲ���
	DWORD  FactionID;//����ID
	char szFactionName[CONST_USERNAME]; //��������
};
struct SAFactionList : public SFactionList
{
	FactionInfoList m_factionlist[MAX_FACTION_NUMBER];

};

//�������ĳ������
DECLARE_MSG(SJoinFaction, STongBaseMsg, STongBaseMsg::EPRO_TONG_QUEST_JOINFACTION)
struct SQJoinFaction : public SJoinFaction
{
	DWORD dFactionID;			//����ID
};
struct SAJoinFaction : public SJoinFaction
{
	BYTE bresult;	//1 �ɹ�  2 �Ѿ��ﵽ�����������  3  ���ų�Ա����
};

//�����뿪����
DECLARE_MSG(SLeaveFaction, STongBaseMsg, STongBaseMsg::EPRO_TONG_QUEST_LEAVEFACTION)
struct SQLeaveFaction : public SLeaveFaction
{
};
struct SALeaveFaction : public SLeaveFaction
{
	BYTE bresult;	//1 �ɹ�����    
};

//���ų��������
DECLARE_MSG(SManagerFaction, STongBaseMsg, STongBaseMsg::EPRO_TONG_MANAGER_MEMBER)
struct SQManagerFaction : public SManagerFaction
{
	BYTE  bManagerType;		//   0  �鿴��������������б�     1   ͬ���ܾ���Ҽ���  2  ����ҳ����� 
	BYTE		bAgree;					//   ͬ���ܾ���Ҽ���
	char szPlayerName[CONST_USERNAME];			// ��Ҫ������������� 
};

struct SAManagerFaction : public SManagerFaction
{
	BYTE bresult;	//1 �ɹ�����    0  Ȩ�޲���
	BYTE  bMembernum;	//ʵ������
	BYTE		Buffer[MAX_JoinListCount * sizeof(FactionMember)];//��ǰ���������б� ������������;Ͳ��ᷢ������
};

//��ɢ����disband
DECLARE_MSG(SDisbandFaction, STongBaseMsg, STongBaseMsg::EPRO_TONG_FACTION_DELETE)
struct SQDisbandFaction : public SDisbandFaction
{
};

struct SADisbandFaction : public SDisbandFaction
{
	BYTE bresult;	//1 �ɹ�����    0  Ȩ�޲���
};



//ͬ������
DECLARE_MSG(SSynFaction, STongBaseMsg, STongBaseMsg::EPRO_TONG_SYNFACTION)
struct SQSynFaction : public SSynFaction
{
};
struct SASynFaction : public SSynFaction
{
	DWORD  dFactionID;    //��������ID
	DWORD  dJoinID;        //��ǰ�����ID
	DWORD  dFactionMoney;		//�Լ��ľ�����
	BYTE			bJoinState;	 //�����״̬  0 ��״̬   1 �Ѿ�ͬ�����  2 �ܾ�
};



//���Ÿ�����Ϣ
DECLARE_MSG(SFactionSceneData, STongBaseMsg, STongBaseMsg::EPRO_TONG_SCENEDATA)
struct SQFactionSceneData : public SFactionSceneData
{
	BYTE   btype;	//   0  ����ͬ����������   1   ��������
	DWORD   dsceneID;        //����ĳ������
};
struct SAFactionSceneData : public SFactionSceneData
{
	WORD   wturnonnum;			//��ǰ�򿪵�����
	DWORD    wactivevalue;			//���ŵ�ǰ��Ծ��
	factionscene m_factionscene[MAX_SENDFACTIONS];  //�����б�
};

//ͬ�����Ÿ����ؿ���Ϣ
DECLARE_MSG(SFactionSceneMapData, STongBaseMsg, STongBaseMsg::EPRO_TONG_SCENEMAPDATA)
struct SQFactionSceneMapData : public SFactionSceneMapData
{
	DWORD   dsceneID;        //����ID
};
struct SAFactionSceneMapData : public SFactionSceneMapData
{
	BYTE   bChallengeCount; //��ս����
	DWORD   dsceneID;        //����ID
	factionscenemap  m_factionscenemap[MAX_SENDFACTIONS];   //�ؿ���Ϣ�б�
};


//��ս�ؿ�
DECLARE_MSG(SChallengeSceneMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_SCENECHALLENGE)
struct SQChallengeSceneMsg : public SChallengeSceneMsg
{
	DWORD dsceneID; //����ID
	BYTE		bscenemapIndex;	//��ս�Ĺؿ��и����е�����
};
struct SAChallengeSceneMsg : public SChallengeSceneMsg
{
	BYTE  bresult;  //  ���
	MonsterData  monsterdata;	//��ǰ�Ĺ�������
};

//��ս�ؿ����
DECLARE_MSG(SSceneFinishMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_SCENEFINISH)
struct SQSceneFinishMsg : public SSceneFinishMsg
{
	double   dHurtValue;		//�Թ�����ɵ��˺�
};

DECLARE_MSG(SSceneHurtRank, STongBaseMsg, STongBaseMsg::EPRO_TONG_SCENEHURTRANK)
struct SQSceneHurtRank : public SSceneHurtRank
{
	DWORD dsceneID; //����ID
};
struct SASceneHurtRank : public SSceneHurtRank
{
	factionscenerank m_factionrank[MAX_FACTION_NUMBER];
	double  dselfhurt;   //�����˺�
	WORD  wRankNum;//���а�����
	WORD  wPlayerRank;//������������
	DWORD dRewardNum;// �ɵõ��ľ���������
	BYTE    bNextRange;//������һ�׶εĽ����������
};

//��ʼ����ȡٺ»����
DECLARE_MSG(SInitFactionSalary, STongBaseMsg, STongBaseMsg::EPRO_TONG_INITFACTIONSALARY)
struct SQInitFactionSalary : public SInitFactionSalary
{
};
struct SAInitFactionSalary : public SInitFactionSalary
{
	BYTE   bIsGot;  // �Ƿ��Ѿ���ȡ   
	factionsalarydata  m_fsalarydata[2];
};

//�����ȡ
DECLARE_MSG(SGetFactionSalary, STongBaseMsg, STongBaseMsg::EPRO_TONG_GETFACTIONSALARY)
struct SQGetFactionSalary : public SGetFactionSalary
{
	BYTE  bType; //��ȡ�������е�����
};
struct SAGetFactionSalary : public SGetFactionSalary
{
	BYTE  bresult;		//  1 �ɹ�  2 Ԫ������  3 �Ѿ���ȡ  4  �޾��Ż������ó����
};

//���Ź���
DECLARE_MSG(SFactionNotice, STongBaseMsg, STongBaseMsg::EPRO_TONG_NOTICE)
struct SQFactionNotice : public SFactionNotice
{ 
};
struct SAFactionNotice : public SFactionNotice
{
	char strnotice[MAX_NOTICE_NUMBER];
};


//�����޸Ĺ���
DECLARE_MSG(SFModifyNotice, STongBaseMsg, STongBaseMsg::EPRO_TONG_MODIFYNOTICE)
struct SQFModifyNotice : public SFModifyNotice
{
	char modifynotice[MAX_NOTICE_NUMBER];
};
struct SAFModifyNotice : public SFModifyNotice
{
	BYTE  bresult;		// 0�޸�ʧ�� 1 �ɹ� 
};

//���Ų�����־
DECLARE_MSG(SFactionOperateLog, STongBaseMsg, STongBaseMsg::EPRO_TONG_OPERATELOG)
struct SQFactionOperateLog : public SFactionOperateLog
{
};
struct SAFactionOperateLog : public SFactionOperateLog
{
	BYTE			blognum;
	operatelog   factionlog[MAX_NOTICE_NUMBER];
};


//����Ⱥ���ʼ�
DECLARE_MSG(SFcEmailToAll, STongBaseMsg, STongBaseMsg::EPRO_TONG_SENDEMAILTOALL)
struct SQFcEmailToAll : public SFcEmailToAll
{
	char title[CONSTMAILTITLE];	//�ʼ�����,Ҫ��
	char content[CONSTMAILCONTENT];//�ʼ�����,Ҫ��
};
struct SAFcEmailToAll : public SFcEmailToAll
{
	BYTE			bresult; //0  ����ʧ��  1�ɹ�
};

//���Ų鿴�������뵽��װ���б�
DECLARE_MSG(SShowEquipt, STongBaseMsg, STongBaseMsg::EPRO_TONG_SHOWEQUIPT)
struct SQShowEquipt : public SShowEquipt
{
};
struct SAShowEquipt : public SShowEquipt
{
	BYTE  blevelnum;   //װ���ȼ�����
	DWORD  dSelectedEquipt;    //�����ѡ��װ��
	BYTE  Buffer[MAX_QUESTEQUIPTBUFFER];
};

//�������뵽��װ��
DECLARE_MSG(SRequestEquipt, STongBaseMsg, STongBaseMsg::EPRO_TONG_QUESTEQUIPT)
struct SQRequestEquipt : public SRequestEquipt
{
	BYTE  blevle;    //�ȼ�
	BYTE   bIndex;  //����
};
struct SARequestEquipt : public SRequestEquipt
{
	BYTE  bresult;   //������  1�ɹ� 2 ����ȼ�������������  3 ���������������� 4 ���û�м������ 5 ��������ȴʱ����  6 �Ѿ������װ�� 
	DWORD dParam;//��չ���� 

};

//��������װ��״̬
DECLARE_MSG(SRequestStatus, STongBaseMsg, STongBaseMsg::EPRO_TONG_SHOWQUESTSTATUS)
struct SQRequestStatus : public SRequestStatus
{
	BYTE    bCanceled;	//�Ƿ�ȡ������        1 ȡ������      0 �鿴��ǰ����״̬
};
struct SARequestStatus : public SRequestStatus
{
	WORD   wEquiptNum;	//��ǰ����
	requestlist mquestlist[MAX_EQUIPTLISTNUM];
};

//ȡ����ǰ������
DECLARE_MSG(SCanceledQuest, STongBaseMsg, STongBaseMsg::EPRO_TONG_CANCELEDQUEST)
struct SQCanceledQuest : public SCanceledQuest
{
};
struct SACanceledQuest : public SCanceledQuest
{
	BYTE  bresult;   //1 �ɹ�  2 ȡ�������װ��û���ҵ� 3 û�м������
};


struct SaveFactionData_New
{
	FactionMember sjoinlist[MAX_JoinListCount];  //���뱾���ŵ�����б�
	SAFactioninfo sfactioninfo;								//��������
};


struct SaveFactionData_Lua
{
	char szFactionName[CONST_USERNAME];			// ��������
	DWORD  FactionID;//����ID
	DWORD   dfactionlenth;
	BYTE   bFactionData[FACTIONDATA_LENTH];  //��������
};





//----------------------------------------------------------��������----------------------------------------�������--------------------------------------------------------


struct SAFactionListHeadData
{
	char szFactionName[CONST_USERNAME];			// ��������
	char szCreatorName[CONST_USERNAME];			// ��ʼ������ 
	//char FactionIcon[MAX_PlayerIcon];//����ͼ��
	int FactionIcon;
	int  byFactionLevel;			// ��ǰ���ɵȼ�
	int  CurMemberNum;				// ��Ա����
	int FactionID;//����ID

	int Title;	//�Լ��ڰ����е�ְλ
	int FactionExp;					//���ž���
	int FactionMoney;				//���Ž�Ǯ
	char FactionMemo[100];				//��������
	SAFactionListHeadData()
	{
		memset(szFactionName, 0, sizeof(szFactionName));
		memset(szCreatorName, 0, sizeof(szCreatorName));
		//memset(FactionIcon, 0, sizeof(FactionIcon));
		FactionIcon = 0;
		byFactionLevel=0;			// ��ǰ���ɵȼ�
		CurMemberNum=0;				// ��Ա����
		FactionID=0;//����ID
		FactionExp = 0;
		FactionMoney = 0;
		Title = 0;
		memset(FactionMemo, 0, sizeof(FactionMemo));
	}

};

enum Enum_TongMsg
{
	Tong_CreateFaction = 80,		//�������
	Tong_DletePalyer,		//����뿪���
	Tong_JoinFation,		//������������
	Tong_JoinFationToList,		//�����������ᵽ����� ��������õ�ͬһ��
	Tong_DisposeFaction,	//��ɢ���
	Tong_RequestFactionList,	//�������б�
	Tong_RequestMySelfFactionMsg,	//�����Լ��İ����Ϣ
	Tong_ChangeFactionMemo,	//��������־
	Tong_RequestGetApplicant,//��ȡ�����˶���
	Tong_RequestMembersData, //��ȡ��Ա����
	Tong_RequestAffirmJioned, //ȷ�ϼ������
	Tong_MemberLeave //����뿪���
};

//����ö��
enum Enum_TongMsg_ErrorCode
{
	Tong_OK,
	Tong_ReName ,    //�������Ѵ���
	Tong_MaxCount ,  //�ﵽ���Ŵ�������
	Tong_VaildName , //��Ч������
	Tong_IntoIng, 	//���о���
	Tong_MemBerToMax,	//������������
	Tong_JoinED,		//����ѱ�����������¼
	Tong_RequestJionListToMax, //�����б�ﵽ����
	Tong_RequestJionCurDayToMax, //���������ﵽ����
	Tong_FacationDsiposeed, //����Ѿ���ɢ
	Tong_FacationJoinNoOk //������뱻�ܾ�
};

//������������
struct SQCreatetchTong 
{
	 byte _messageprotocol;
	 byte _baseProtocol;

	 char factionName[CONST_USERNAME];
	 //char factionIcon[MAX_PlayerIcon];
	 int factionIcon;
	 SQCreatetchTong()
	 {
		 memset(factionName, 0, sizeof(factionName));
	 }
};

struct SACreatetchTong
{
	byte _messageprotocol;
	byte _baseProtocol;
	byte _result;

	SACreatetchTong()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_baseProtocol = Enum_TongMsg::Tong_CreateFaction;
		_result = 0;
	}
};


//��������б�
struct SQRequestFactionList
{
	byte _messageprotocol;
	byte _protocol;
	bool allOrsingle;
	int factionID;
	char factionName[32];
	SQRequestFactionList()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestFactionList;
		memset(factionName, 0, sizeof(factionName));
		factionID = 0;
	}

};

struct SARequestFactionList
{
	byte _messageprotocol;
	byte _protocol;
	byte Fcount;
	SAFactionListHeadData factionlist[MAX_FACTION_NUMBER];
	SARequestFactionList()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestFactionList;
		Fcount = 0;
	}
};


//����������
struct SQRequestJoinFaction
{
	byte _messageprotocol;
	byte _protocol;
	bool agree;
	char RequsetName[CONST_USERNAME];

	SQRequestJoinFaction()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_JoinFation;
		memset(RequsetName, 0, sizeof(RequsetName));
	}

};

struct SARequestJoinFaction
{
	byte _messageprotocol;
	byte _protocol;
	byte result;
	SARequestJoinFaction()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_JoinFation;
		result = Enum_TongMsg_ErrorCode::Tong_OK;
	}
};

//������ŵ������
struct SQRequestJoinFactionToList
{
	byte _messageprotocol;
	byte _protocol;
	char factionName[CONST_USERNAME];

	SQRequestJoinFactionToList()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_JoinFationToList;
		memset(factionName, 0, sizeof(factionName));
	}

};

struct SARequestJoinFactionToList
{
	byte _messageprotocol;
	byte _protocol;
	byte result;
	SARequestJoinFactionToList()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_JoinFationToList;
		result = Enum_TongMsg_ErrorCode::Tong_OK;
	}
};





//��ɢ����
struct SQDisposeFaction
{
	byte _messageprotocol;
	byte _protocol;
	char factionName[CONST_USERNAME];
	SQDisposeFaction()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_DisposeFaction;
		memset(factionName, 0, sizeof(factionName));
	}

};

struct SADisposeFaction
{
	byte _messageprotocol;
	byte _protocol;
	byte result;
	SADisposeFaction()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_DisposeFaction;
		result = Enum_TongMsg_ErrorCode::Tong_OK;
	}

};



//�������
struct SQDletePalyer
{
	byte _messageprotocol;
	byte _protocol;
	char pname[CONST_USERNAME];
	SQDletePalyer()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_DletePalyer;
		memset(pname, 0, sizeof(pname));
	}

};

struct SADletePalyer
{
	byte _messageprotocol;
	byte _protocol;
	byte result;
	SADletePalyer()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_DletePalyer;
		result = Enum_TongMsg_ErrorCode::Tong_OK;
	}

};

//���İ�������
struct SQChangeFactionMemo
{
	byte _messageprotocol;
	byte _protocol;
	char memomsg[100];
	SQChangeFactionMemo()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_ChangeFactionMemo;
		memset(memomsg, 0, sizeof(memomsg));
	}
};

struct SAChangeFactionMemo
{
	byte _messageprotocol;
	byte _protocol;
	byte result;
	SAChangeFactionMemo()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_ChangeFactionMemo;
		result = Enum_TongMsg_ErrorCode::Tong_OK;
	}
};



//�������������
struct SQMySelfFactionMsg
{
	byte _messageprotocol;
	byte _protocol;
	byte factionName[CONST_USERNAME];
	byte pname[CONST_USERNAME];
	
	SQMySelfFactionMsg()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestMySelfFactionMsg;
		memset(factionName, 0, sizeof(factionName));
		memset(pname, 0, sizeof(pname));
	};
};

struct SAMySelfFactionMsg
{
	byte _messageprotocol;
	byte _protocol;
	byte result;
	SAFactionListHeadData headdata;
	SAMySelfFactionMsg()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestMySelfFactionMsg;
		result = Enum_TongMsg_ErrorCode::Tong_OK;

	};
};

// ��ȡ�����˶���
struct SQArmyGroupApplicantList
{
	byte _messageprotocol;
	byte _protocol;
	SQArmyGroupApplicantList()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestGetApplicant;
	}
};

struct SAArmyGroupApplicantList
{
	byte _messageprotocol;
	byte _protocol;
	byte _count;
	SimFactionData::RequestJionPlayer RjoinList[20];
	
	SAArmyGroupApplicantList()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestGetApplicant;
		_count = 0;
		memset(RjoinList, 0, sizeof(RjoinList));
	};
};

//��ȡ��Ա����
struct SQArmyGroupGetMembers
{
	byte _messageprotocol;
	byte _protocol;
	SQArmyGroupGetMembers()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestMembersData;
	};
};

struct ArmyGroupMembersData
{
	char name[CONST_USERNAME];
	int IconID;
	int level;
	int Position;
	int LineTime;
	int LastLineTime;

	ArmyGroupMembersData()
	{
		IconID = 0;
		memset(name, 0, sizeof(name));
		level = 0;
		Position = 0;
		LineTime = 0;
		LastLineTime = 0;
	};
};

struct SAArmyGroupGetMembers
{
	byte _messageprotocol;
	byte _protocol;
	byte _count;
	ArmyGroupMembersData memberlist[MAX_FACTION_NUMBER];
	SAArmyGroupGetMembers()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestMembersData;
		_count = 0;
		//memset(members, 0, sizeof(members));
	};
};




//ȷ�Ͻ�����Ϣ
struct SQAffirmJoinFacttion
{
	byte _messageprotocol;
	byte _protocol;

	SQAffirmJoinFacttion()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestAffirmJioned;
		
	};
};

struct SAAffirmJoinFacttion
{
	byte _messageprotocol;
	byte _protocol;
	byte factionName[CONST_USERNAME];
	bool result;
	SAAffirmJoinFacttion()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestAffirmJioned;
		result = true;
		memset(factionName, 0, sizeof(factionName));
	};
};
