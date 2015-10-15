#pragma once

#include "NetModule.h"
#include "playertypedef.h"
#include "../pub/ConstValue.h"
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ������֮�������ˢ����Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SRefreshBaseMsg, SMessage, SMessage::EPRO_REFRESH_MESSAGE)
//{{AFX
EPRO_REFRESH_REGION_SERVER,     // ˢ�����������
EPRO_REFRESH_PLAYER,            // ˢ���������
EPRO_REFRESH_RankList,			// ���а���Ϣ
EPRO_REFRESH_PLAYERLOGINCOMPLTE,			//����������������½���
EPRO_REFRESH_PLAYERLOGINEXCEPTION,			//ˢ���쳣�������Ϣ
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ����������ˢ����Ϣ
DECLARE_MSG(SRefreshRegionServerMsg, SRefreshBaseMsg, SRefreshBaseMsg::EPRO_REFRESH_REGION_SERVER)
struct SQRefreshRegionServerMsg : 
    public SRefreshRegionServerMsg
{
    DNID dnidClient;// ����ˢ�µ�����У����
    DWORD key;      // ����ˢ�µķ��������
    DWORD gid;      // ����ˢ�µ�ȫ�ֱ��

    // û��ʲôʵ�����ݣ����󳡾�ˢ�µ�ʱ��ͬʱˢ�����а�����
    //SScoreTable Scores;

    bool bFullQuest;
};

struct SARefreshRegionServerMsg : 
    public SRefreshRegionServerMsg 
{
    DNID dnidClient;// ����ˢ�µ�����У����
    DWORD key;      // ����ˢ�µķ��������
    DWORD gid;      // ����ˢ�µ�ȫ�ֱ��

    // ����ĳ���ˢ������
    DWORD   dwIP;                   // ���ڽ����ⲿ���ӵĶ˿�
    DWORD   dwPort;                 // ���ڽ����ⲿ���ӵĶ˿�
    WORD    wTotalPlayers;          // ˵���ó�����ǰ���ڶ��ٸ���Ҷ���
    WORD    wPlayers[4];            // 4��״̬�������Ŀ��

    // �ɼ�����
    __int64 i64TotalSpendMoney;     // ���ĵĽ�Ǯ�ܶ�
    DWORD   dwAvePing;              // ƽ��pingֵ
    DWORD   dwInFlowPS;             // ÿ������
    DWORD   dwOutFlowPS;            // ÿ������
    DWORD   dwCreatedSpecItem;      // ������������Ʒ����
    DWORD   dwUseSpecItem;          // ���ĵ�������Ʒ����

    BYTE    byRegionNumber;         // ʵ����Ч�ĳ�������
    SRegion aRegions[MAX_REGION_NUMBER + 10];          // ���100����������Ч����

    int GetMySize()
    {
        if (byRegionNumber >= MAX_REGION_NUMBER) return 0;
        return sizeof(SARefreshRegionServerMsg) - sizeof(SRegion)*(MAX_REGION_NUMBER + 10 - byRegionNumber);
    }
};

// ����¶������Ϣ����Ҫ�����������������֪ͨ��½���������Լ����ӻ�ɾ����һЩʲô���Ĺ̶�������
struct SARefreshRegionServerMsgEx : 
    public SRefreshRegionServerMsg 
{
    DNID dnidClient;        // ���������������ΪЧ���룬Ϊ0xff12ff341ff56ff78��ʱ�򣬾�������Ϣ��
    char dataStream[1];     // ���л�������
};

//---------------------------------------------------------------------------------------------



//=============================================================================================
// ˢ��������ݵ���Ϣ��Ŀǰ����û���õ���
DECLARE_MSG(SRefreshPlayerMsg, SRefreshBaseMsg, SRefreshBaseMsg::EPRO_REFRESH_PLAYER)

struct SARefreshPlayerMsg : 
    public SRefreshPlayerMsg 
{
    enum { ONLY_BACKUP, BACKUP_AND_SAVE, SAVE_AND_LOGOUT, BACKUP_AND_FLAG, ONLY_LOGOUT, SAVE_AND_HANGUP};
    BYTE storeflag;
    //DWORD gid;
	SFixData fixData;
    char streamData[MAX_ACCOUNT];
};
//---------------------------------------------------------------------------------------------

DECLARE_MSG(SRefreshRanklist,SRefreshBaseMsg,SRefreshBaseMsg::EPRO_REFRESH_RankList)
struct SQRefreshRanklist : public SRefreshRanklist
{
	//DWORD serverID;
	char  name[CONST_USERNAME];
	DWORD level;
};
struct SARefreshRanklist : public SRefreshRanklist
{
	
};

// ��������ҵ�¼ˢ��
DECLARE_MSG(SPlayerLoginComplte, SRefreshBaseMsg, SRefreshBaseMsg::EPRO_REFRESH_PLAYERLOGINCOMPLTE)
struct SQPlayerLoginComplte :public SPlayerLoginComplte
{
	char  account[MAX_ACCOUNT]; //��¼�ɹ�������ʻ�
};

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////  ////////////////////////////////////////////  ////////
////// /////    ///////    ///////////  /////////////  /////////////////////////////  /////////
/////// /   /////////  ///  /////////  //////////////////////  /     //////////////  //////////
///////  ///////////        ////////     //////////  ////////  ////  /////////      ///////////
//////  ///////////  //////////////  ///  ////////  ////////  ////  ////////  ///  ////////////
/////  ////////////  ///  ////////  ///  ////////  ////////  ////  ////////  ///  /////////////
////  //////////////    /////////      /////////  ////////  ////  /////////      //////////////
///////////////////////////////////////////////////////////////////////////////////////////////

// ��ҵĳ���ת�� �� ��ҵĳ���������ת��

// ��һ������Ҫ�ͻ��˵����
// �ͻ�����Ҫ���������ӣ�һ�����ڱ�����һ�����Ӻ���Ϊ�������ӣ�һ�����ڽ����µĵ��ض�������

// ����ڳ�����ת��ʱ�����ܻᷢ��Ŀ�곡�����ڵ�ǰ�����������ϵ����
// ����ʱ��ͻ���ֳ���������ת��

// ���ȣ�������ڵĵ�ǰ���������ض���ʼ������������֪ͨ��½������������ҽ�ת�Ƴ��������ҷ�����
//   ����������ݣ�Ŀ�곡����ID����ͬʱ�������ʼ���������ϴ���ʧЧ״̬���������κβ�����������
//   �����ݶ�Ч��

// ���ţ���½������������ҵ�״̬��־��Ϊ�����ض���״̬��ͬʱ֪ͨʼ������������ת����ص�����
//   ��Ŀ��IP&PORT��������������Ϊ�����ض���״̬����Ҷ���ᱻ����ʵʱ��ض��У�һ��ʱ����û��
//   �յ������κ�һ���������������ض�����֤���ͽ�����������ߴ���
// Ȼ��ʼ��������֪ͨ�ͻ���ת�Ƴ���������Ҫ�½����ӵ�Ŀ�������
// ���ͻ������ӵ�Ŀ���������Ŀ������������ض����������½������������֤����ҵ������ض���
// ��½�������յ��ض�����֤���ڱ��ز���Ŀ����ң����û�ҵ���˵�������ض����Ѿ���ʱ����֮����
//   ���ض�����֤
// ����ض�����֤ͨ������ô�ͻ��˾������ɹ��ˣ�������ҵ��ض���״̬ȥ����֪ͨĿ���������֤�ɹ�
//   �����������ӣ���֪ͨʼ�����������Ͽ����ӣ�
// ����ض�����֤ʧ�ܣ���ô֪ͨʼ���������������ø�Ŀ�����ӣ�Ҳ���ܻ���ڸ�Ŀ����ʼ������������
//   ����ʧ�������������֪ͨĿ���������֤ʧ�ܣ��Ͽ����ӣ�

// �Գ��ε�½Ϊ����
// �ͻ������ӵ���½������
// ��½��������ȡ����ҵ�����
// [���ݸ���ҵ����ݿ�ʼ����������ת��]
// ��Ϊ�����б��ڱ���������ֱ�ӻ��Ŀ�곡�����������
// -����ȡʧ�ܣ�֪ͨ�ͻ���[û��Ŀ�곡�����쳣����]
// ��ȡ�ɹ��󣬽��������Ϊ�����ض���״̬����ͻ��˷����ض���֪ͨ
// �ͻ��˽���ǰ������ΪʧЧ״̬�������ض���������Ŀ���������������
// -������ʧ�ܣ�֪ͨʼ������������½�����������޷���������
// ���ӳɹ�����Ŀ������������ض�������
// Ŀ������������ض������󣬵���½������������֤
// -����֤ʧ�ܣ�Ŀ��������Ͽ����ӣ�ʼ������������½���������ָ�ԭ���ӵ���Ч��
// ��֤�ɹ�����½�������ϵ�����ض���״̬ȡ��
// 
// �µ�һ���汾�У����еĲ����������ʧ�ܵĻ�����û�г������
// ����ȵ���ʱ�Ż��о���ķ�ӳ
//=============================================================================================
DECLARE_MSG_MAP(_SRebindMsg, SMessage, SMessage::EPRO_REBIND_MESSAGE)
//{{AFX
EPRO_REBIND_REGION_SERVER,      // �����ض�����Ϣ
EPRO_CHECK_REBIND,              // �ض�����֤��Ϣ

EPRO_REBIND_LOGIN_SERVER,		// ������֮���޷����ӣ����˳��ͻ���ֱ�����ӣ�
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================
// �ض��������Ϣ���ڷ������Ϳͻ���֮�䴫��
DECLARE_MSG(SRebindMsg, _SRebindMsg, _SRebindMsg::EPRO_REBIND_REGION_SERVER)
// ����Ϣ�ǵ�½�����������ͻ��˵ģ�֪ͨ�������µĳ���������
struct SRebindRegionServerMsg : 
   public SRebindMsg        // ���е��ض�����Ϣ�����ɷ�����֪ͨ�ͻ��˵ģ��ͻ��˲������������������ض���
{
    DWORD   ip;       // �ض���Ŀ��ĵ�ַ
    DWORD   port;     // �ض���Ŀ��Ķ˿�
    DWORD   gid;      // У����ȫ�ֹؼ��֣�������Բ������������ϣ�

    // char    key[ACCOUNTSIZE];  // У��ؼ��֣�������Բ�����ֱ��ʹ�ÿͻ��˱������������ݣ�

    char streamData[MAX_ACCOUNT];
};

// ����Ϣ�ǿͻ��˷���Ŀ��������ģ�֪ͨĿ�������ȥ��½������У��
struct SQRebindMsg : public SRebindMsg        
{
	DWORD	gid;      // У����ȫ�ֹؼ��֣����ݣ�
    char	streamData[MAX_ACCOUNT];
};

DECLARE_MSG( SRebindLoginBaseMsg, _SRebindMsg, _SRebindMsg::EPRO_REBIND_LOGIN_SERVER )
struct SARebindLoginServerMsg :
	public SRebindLoginBaseMsg
{
	enum { REBIND_NORMAL = 1, REBIND_UNUSUAL, };
	DWORD m_dwGID;
	DWORD m_GMLevel;
	DWORD m_dwType;
	char streamData[ 4096 ];
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// �ض�����֤��Ϣ���ڷ������ͷ�����֮�䴫��
DECLARE_MSG(SCheckRebindMsg, _SRebindMsg, _SRebindMsg::EPRO_CHECK_REBIND)
// ����Ϣ�����������������½�������ģ�֪ͨ��Ҫ��֤һ���µĿͻ���
struct SQCheckRebindMsg : public SCheckRebindMsg
{
    DNID    dnidClient;     // �������ӱ��
    DWORD   gid;            // У����ȫ�ֹؼ���
    char	streamData[MAX_ACCOUNT];
};

// ����Ϣ�ǵ�½��������������������ģ�֪ͨ��֤������ɹ�����ڱ���������������ϴ���һ�����Ӧ�Ķ�����ң�
struct SACheckRebindMsg : public SCheckRebindMsg
{
	DWORD   dwResult;       // ��֤���
	DNID    dnidClient;     // �������ӱ��
	DWORD   gid;            // У����ȫ�ֹؼ���

    // ����������ܻ������ٴ��ض���
	DWORD   ip;       // �ض���Ŀ��ĵ�ַ
	DWORD   port;     // �ض���Ŀ��Ķ˿�
	WORD    wMapID;
	WORD    wRegionID;
	WORD    wCurX;
	WORD    wCurY;

    char streamData[MAX_ACCOUNT];
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ������Ϣ����һ������Ҫ��֤�����Ŀɿ��ԣ�����ʹ������Ϣ��Ӧ��ʱ����
DECLARE_MSG_MAP(SServerCtrlMsg, SMessage, SMessage::EPRO_SERVER_CONTROL)
//{{AFX
EPRO_PLAYER_DATA,               // ����һ����ҵ�����
EPRO_PUT_PLAYER_IN_REGION,      // ��һ����ҷ��볡��
EPRO_RESOLUTION_REGION,         // ���ݳ���ת�Ƶ���Ҫ���ڵ�½�������ϻ�ȡ���ת������
EPRO_GAMESERVER_REQUEST_RPCOP,	// �������ݿ����RPC����
EPRO_SCRIPT_TRANSFER,           // �ű���Ϣת��
//}}AFX
END_MSG_MAP_WITH_ROTOC()
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ����Ϣ��������������������½�������ģ�֪ͨ��Ҫ��ȡһ����ҵ����ݣ�һ�����ڳ���������ת����ɵ�ʱ�򣬷��ͳ���ת��֮ǰ��
// ����Ӧ����Q&A�ṹ�ģ����������Get&Set���� ���ã����ɡ���
DECLARE_MSG(SPlayerDataMsg, SServerCtrlMsg, SServerCtrlMsg::EPRO_PLAYER_DATA)
struct SGetPlayerDataMsg : 
   public SPlayerDataMsg
{
    DNID    dnidClient;     // �������ӱ��

    //char    key[ACCOUNTSIZE];        // У��ؼ���
    DWORD   gid;            // У����ȫ�ֹؼ���

    char streamData[MAX_ACCOUNT];
};

struct SSetPlayerDataMsg : public SPlayerDataMsg
{
    DNID			dnidClient;     // �������ӱ��
    DWORD			gid;            // У����ȫ�ֹؼ���
    SFixData	data;
    SPlayerTempData tdata;

    // Ϊ���¹��ܶ���������������ע�⣺
    // ��Ϊ��������Ƿ�������ĩβ�ģ���֮ǰ����SSetPlayerDataMsgʱ�Ƕ�������
    // ���������������ݺ����ϢӦ�ÿ��Լ��ݾɵİ汾
    char streamData[MAX_ACCOUNT];
};

//=============================================================================================
// ����ת�ƿ�����Ϣ
DECLARE_MSG(SChangeRegionMsg, SServerCtrlMsg, SServerCtrlMsg::EPRO_PUT_PLAYER_IN_REGION)
struct SAChangeRegionMsg : 
   public SChangeRegionMsg
{
    DNID    dnidClient;     // �������ӱ��

    WORD    wRegionID;      // �����ʱû���ã�ֱ����������ϵ�������ת�Ƴ���
    WORD    x, y;           // ͬ��
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ��������������Ϣ�����������ͨ���������Ŀ�곡����λ��
DECLARE_MSG(SResolutionRegionMsg, SServerCtrlMsg, SServerCtrlMsg::EPRO_RESOLUTION_REGION)
struct SQResolutionDestRegionMsg : 
    public SResolutionRegionMsg
{
    DNID dnidClient;     // �������ӱ��
    WORD wRegionID;
    WORD wX;
    WORD wY;
    DWORD dwGID;
    WORD wLevel;

    char streamData[MAX_ACCOUNT];
};

struct SAResolutionDestRegionMsg : 
    public SResolutionRegionMsg
{
    DWORD dwip;
    DWORD dwport;

    DNID dnidClient;     // �������ӱ��

    WORD wRegionID;
    WORD wMapID;
    WORD wX;
    WORD wY;

    DWORD dwGID;
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// �������ݿ����RPC����
DECLARE_MSG( SGameServerRPCOPMsg, SServerCtrlMsg, SServerCtrlMsg::EPRO_GAMESERVER_REQUEST_RPCOP )
struct SQGameServerRPCOPMsg : 
    public SGameServerRPCOPMsg
{
	enum { CHARACTER_DATABASE, ACCOUNT_DATABASE,DBCENTER_DATABASE,  };

    BYTE dstDatabase; // [��ɫ���ݿ�]/[�˺����ݿ�]
    char streamData[ 4096 ];
	LPVOID GetExtendBuffer() { return &dnidClient; };
};

struct SAGameServerRPCOPMsg : 
    public SGameServerRPCOPMsg
{
	enum { CHARACTER_DATABASE, ACCOUNT_DATABASE };

    BYTE srcDatabase; // [��ɫ���ݿ�]/[�˺����ݿ�]
    char streamData[ 4096 ];
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG( SGameScriptTransfreMsg, SServerCtrlMsg, SServerCtrlMsg::EPRO_SCRIPT_TRANSFER )
struct SAGameScriptTransfreMsg : 
    public SGameScriptTransfreMsg
{
    DWORD dbcId;                 // Ҫ����ķ�����
    DWORD destSercerId;          // Ŀ�������ID
    char streamData[ 4096 ];
};

//---------------------------------------------------------------------------------------------

// ��������DB->LOGIN->REGION�����а���Ϣ
DECLARE_MSG(SGetRankListFromDB, SMessage, SMessage::EPRO_GET_RANKLIST_FROM_DB)
struct SAGetRankListFromDB : public SGetRankListFromDB
{
 	long m_Num;							// ���а����������Ϊ3000
 	NewRankList m_NewRankList[ALL_RANK_NUM];
};