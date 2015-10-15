#pragma once
#include "NetModule.h"
#include "logtypedef.h"
#include "playertypedef.h"
#include "../pub/ConstValue.h"
#include "tongmsgs.h"

// ��Ϸ���ݿ��������
// ����½��ʼʱ�����е�һ����Ϸ���ݿ��������ȡ��ɫ�б� [EPRO_GET_CHARACTER_LIST]
// �����ѡ��ĳ��ɫ�����Ҫȥ���ݿ��������ȡ��ɫ���� [EPRO_GET_CHARACTER] Ȼ��Ϳ�ʼ����Ϸ����������
// �ڽ�ɫѡ��Ĺ����У����ܻᴴ���µĽ�ɫ [EPRO_CREATE_CHARACTER] ��Ҳ���ܻ�ɾ��ĳ���Ѵ��ڵĽ�ɫ [EPRO_DELETE_CHARACTER]
// ��Ϸ�����У����ܻ���ʱҪ�����ݿ����������ĳ����ɫ������ [EPRO_SAVE_CHARACTER]

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ���ݿ�����������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(_SDataBaseMsg, SMessage, SMessage::EPRO_DATABASE_MESSAGE)
//{{AFX
EPRO_GET_CHARACTER_LIST,    // ��ȡĳ���˺�������Ľ�ɫ�б�
EPRO_GET_CHARACTER,         // ��ȡĳ����ɫ������
EPRO_SAVE_CHARACTER,        // ����ĳ����ɫ������
EPRO_CREATE_CHARACTER,      // �´���һ����ɫ��ͬʱ�Ķ����˺�������Ľ�ɫ�б�
EPRO_DELETE_CHARACTER,      // ɾ��ĳ���Ѵ��ڽ�ɫ��ͬʱ�Ķ����˺�������Ľ�ɫ�б�
EPRO_SAVE_TASK,             // ����ĳ����ɫ��ҵ�����
EPRO_GET_TASK,
EPRO_SAVE_GM_LOG,           // ����ĳ��GM�Ĳ���
EPRO_GET_GM_LOG,            // ��ȡĳ��GM�Ĳ���
EPRO_GM_CHECKPLAYER,        // gm�鿴�������
EPRO_GM_CHECKWAREHOUSE,     // �鿴��ҵĲֿ�
EPRO_GM_UPDATEPLAYER,       // �����������
ERPO_SAVE_WAREHOUSE,
ERPO_GET_WAREHOUSE,
EPRO_GM_GETNAMELISTSRV,		//��ȡ��ҽ�ɫ�б�]
EPRO_GET_ACCOUNTBYNAME,		//��ȡ������ҵ��ʺ�
EPRO_SAVE_DBCENTERDATA,		//����DBCenter��������ұ�������
EPRO_SAVE_FACTION,			// �����������
EPRO_GET_FACTION,			// ��ȡ��������
EPRO_SAVE_BUILDING,			//���潨��������
EPRO_GET_BUILDING,			//��ȡ����������
EPRO_DOUBLE_SAVE_PLAYER,     // ͬ��ˢ��2����ҵ����ݣ���Ҫ��Ϊ�˽�����߽��ף�����������
EPRO_SAVEPLAYER_CONFIG,			//������ҵ�������
EPRO_DBGET_RankList,		//�����ݿ��ȡ��ϸ��Ϣ
EPRO_DB_Mail_SANGUO,		//���ݿ��ʼ�
EPRO_WRITE_POINT_LOG,		//���������־
EPRO_DELETE_FACTION,			// ɾ����������
EPRO_SAVE_SCRIPTDATA,		//�ű����ݱ���
EPRO_GET_SCRIPTDATA,		//�ű����ݱ���

//}}AFX
END_MSG_MAP_WITH_ROTOC()

//=============================================================================================

// �����������
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SSaveFactionMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_SAVE_FACTION)
struct SQSaveFactionMsg : public SSaveFactionMsg
{
	__int32  nServerID;
	DWORD nFactions;
	DWORD dwSendSize;
	BOOL  bSaveMode;		// T��������������ݲ���,F���෴
	int   nTime;			// �����õ���֤ʱ��
	char streamFaction[sizeof(SaveFactionData_New)*MAX_SENDFACTIONS];
};

struct SQSaveFactionDataMsg : public SSaveFactionMsg
{
	__int32  nServerID;
	DWORD nFactions;
	DWORD dwSendSize;
	BOOL  bSaveMode;		// T��������������ݲ���,F���෴
	int   nTime;			// �����õ���֤ʱ��
	char streamFaction[sizeof(SaveFactionData_Lua)*MAX_SENDFACTIONDATA];
};



// ��ȡ��������
DECLARE_MSG(SGetFactionMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_GET_FACTION)
struct SQGetFactionMsg : public SGetFactionMsg
{
	__int32  nServerId;
};

struct SAGetFactionMsg : public SGetFactionMsg
{
    BOOL  isLast;   // ���Ϊ�棬����Ϣ��ʾ���ݿ��Ѿ��ɹ���ȡ�����а��ɣ�
	DWORD nFactions;
	DWORD dwSendSize;
	char streamFaction[sizeof(SaveFactionData_New) * MAX_SENDFACTIONS];
};
struct SAGetFactionDataMsg : public SGetFactionMsg
{
	BOOL  isLast;   // ���Ϊ�棬����Ϣ��ʾ���ݿ��Ѿ��ɹ���ȡ�����а��ɣ�
	DWORD nFactions;
	DWORD dwSendSize;
	char streamFaction[sizeof(SaveFactionData_Lua) * MAX_SENDFACTIONDATA];
};

// ɾ����������
DECLARE_MSG(SDeleteFaction, _SDataBaseMsg, _SDataBaseMsg::EPRO_DELETE_FACTION)
struct SQDeleteFaction : public SDeleteFaction
{
	DWORD  nServerId;
	DWORD  dFactionID;
};

struct SADeleteFaction : public SDeleteFaction
{
	DWORD  nServerId;
	DWORD  dFactionID;
	BYTE bresult;
};



//-------------------------------------------------------------------------

//=========================���������ݱ���===============================================
//
const WORD  MAX_BUILDING_BUF_SIZE	=	0xFF;
DECLARE_MSG(SDBBuildingSaveMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_SAVE_BUILDING )
struct SQDBBuildingSaveMsg : public SDBBuildingSaveMsg
{
	DWORD	dwServerId;	
	char	buffer[MAX_BUILDING_BUF_SIZE];
};


DECLARE_MSG(SDBBuildingGetMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_GET_BUILDING)
struct SQDBBuildingGetMsg : public SDBBuildingGetMsg
{
	DWORD	dwServerId;
	WORD	parentID;
};

struct SADBBuildingGetMsg : public SDBBuildingGetMsg
{
	QWORD	uniquid;	
	WORD	wparentID;
	WORD	itemIndex;	
	char streamFaction[MAX_BUILDING_BUF_SIZE];
	//inline void reset() { memset (this, 0, sizeof *this);}
};
//==========================���������ݱ���==============================================


DECLARE_MSG(SBaseGetNameListSrvMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_GM_GETNAMELISTSRV)
struct SQGetNameListSrvMsg : public SBaseGetNameListSrvMsg
{
    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

struct SAGetNameListSrvMsg : public SBaseGetNameListSrvMsg
{
	SCharListData CharListData[5];
};

DECLARE_MSG(SBaseUpdatePlayerMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_GM_UPDATEPLAYER)
struct SQUpdatePlayerMsg : public SBaseUpdatePlayerMsg
{
    DWORD dwStaticID;
    SFixBaseData PlayerBaseData;
};
struct SAUpdatePlayerMsg : public SBaseUpdatePlayerMsg
{
    BYTE byResult;
	char szName[CONST_USERNAME];
};

//---------------------------------------------------------------------------------------------
//=============================================================================================
//GM�����Ҳֿ�����
DECLARE_MSG(SBaseCheckWarehouseMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_GM_CHECKWAREHOUSE)
struct SQCheckWarehouseMsg : public SBaseCheckWarehouseMsg
{
    // �˺�
	char streamData[ MAX_ACCOUNT ];
};
struct SACheckWarehouseMsg : public SBaseCheckWarehouseMsg
{
    WORD wResult;
    enum 
    {
        ERC_FAIL,
        ERC_SUCCESS
    };

    SFixStorage1 WarehouseI;
//    SFixStorage2 WarehouseII;
//    SFixStorage3 WarehouseIII;

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

//---------------------------------------------------------------------------------------------
//=============================================================================================
//GM����������

DECLARE_MSG(SBaseCheckPlayerDBMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_GM_CHECKPLAYER)
struct SQCheckPlayerDBMsg : public SBaseCheckPlayerDBMsg
{
    char szName[CONST_USERNAME];
};

struct SACheckPlayerDBMsg : public SBaseCheckPlayerDBMsg
{
    WORD wResult;
    DWORD dwStaticID;
    SFixBaseData PlayerData;
    SFixPackage  PlayerPackage;

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};
//=============================================================================================
// ȡ��GM��������Ϣ

DECLARE_MSG(SGetGMLogMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_SAVE_GM_LOG)
struct SQGetGMLogMsg : 
    public SGetGMLogMsg
{
    SGMLog GmLog;
};

struct SAGetGMLogMsg  :
    public SGetGMLogMsg
{
    SGMLog GmLog;
};


//=============================================================================================
// ����GM��������Ϣ

DECLARE_MSG(SSaveGMLogMsg,_SDataBaseMsg,_SDataBaseMsg::EPRO_SAVE_GM_LOG)
struct SQSaveGMLogMsg : 
    public SSaveGMLogMsg
{
    SGMLog GmLog;
};

struct SASaveGMLogMsg :
    public SSaveGMLogMsg
{
    SGMLog GmLog;
};

// ��ȡ��ɫ�б����Ϣ
DECLARE_MSG(SGetCharacterListMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_GET_CHARACTER_LIST)
struct  SQGetCharacterListMsg : public SGetCharacterListMsg
{
    DWORD	dwServerId;					// ��������ķ�����ID
	char	streamData[MAX_ACCOUNT];	// �˺�
};

struct SAGetCharacterListMsg : public SGetCharacterListMsg
{
    enum
    {
        RET_SUCCESS,                // �����ɹ�
        RET_LOCKEDSELF,             // �Ѿ�����ǰ������ID����
        RET_LOCKEDOTHER,            // �Ѿ�������������ID����
        RET_ERROR_INFO,             // ����ʧ��
    };

    BYTE byResult;                  // ���Ϸ���ֵ
    SCharListData CharListData[5];  // ���ĳ��λ�Ľ�ɫ�����ڣ���ô����ӳλ�ϵ����ݾ�Ϊ��
	char streamData[ MAX_ACCOUNT ]; // �˺�
};
//---------------------------------------------------------------------------------------------

// ɾ��һ���Ѿ����ڵĽ�ɫ����Ϊ�����漰���˺����ݣ����Բ���Ҫ���� szAccount szPassword
DECLARE_MSG(SDeleteCharacterMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_DELETE_CHARACTER)
struct SQDeleteCharacterMsg : public SDeleteCharacterMsg
{
    DWORD dwServerId;               // ��������ķ�����ID
    DWORD dwStaticID;				// Ҫɾ����ɫ��ID

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

struct SADeleteCharacterMsg : public SDeleteCharacterMsg
{
    BYTE byResult;
    DWORD dwStaticID;
    char szName[CONST_USERNAME];
};
//---------------------------------------------------------------------------------------------

// ����һ���µĽ�ɫ����Ϊ�����漰���˺����ݣ����Բ���Ҫ���� szAccount szPassword
DECLARE_MSG(SCreatePlayerMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_CREATE_CHARACTER)
struct SQCreatePlayerMsg : public SCreatePlayerMsg
{
    DWORD	dwServerId;					// ��������ķ�����ID
    BYTE	m_byIndex;
	SCreateFixProperty PlayerData;

	char	streamData[MAX_ACCOUNT];	// �˺�
};

struct SACreatePlayerMsg : public SCreatePlayerMsg
{
    enum ERetErrCode
    {
        ERC_SUCCESS = 1,
        ERC_INDEXTAKEN,
        ERC_NAMETAKEN,
        ERC_UNHANDLE,
    };

    BYTE	byResult;
    BYTE	m_byIndex;
    SCreateFixProperty PlayerData;
};
//---------------------------------------------------------------------------------------------

// ��ȡһ����ɫ������
DECLARE_MSG(SGetCharacterMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_GET_CHARACTER)
struct SQGetCharacterMsg : public SGetCharacterMsg
{
    DWORD	dwServerId;					// ��������ķ�����ID
    BYTE	byIndex;					// ��ɫ����
	char	streamData[MAX_ACCOUNT];	// �˺�
};

struct SAGetCharacterMsg : public SGetCharacterMsg
{
    enum ERetErrCode
    {
        ERC_SUCCESS,
		ERC_LOCKEDSELF,             // �Ѿ�����ǰ������ID����
		ERC_LOCKEDOTHER,            // �Ѿ�������������ID����
        ERC_FAIL,
    };

    BYTE			byResult;
    SFixData	PlayerData;					// ��ɫ����
	char			streamData[MAX_ACCOUNT];	// �˺�
};
//---------------------------------------------------------------------------------------------
//��ȡһ����ɫ�Ĳֿ�����
DECLARE_MSG(SGetWarehouseMsg, _SDataBaseMsg, _SDataBaseMsg::ERPO_GET_WAREHOUSE)
struct SQGetWarehouseMsg : public SGetWarehouseMsg
{
    DWORD dwServerId;               // ��������ķ�����ID

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

struct SAGetWarehouseMsg : public SGetWarehouseMsg
{
    SFixStorage1 PlayerWarehouseI;
//    SFixStorage2 PlayerWarehouseII;
//    SFixStorage3 PlayerWarehouseIII;

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};


//=============================================================================================
// ����һ����ɫ������
DECLARE_MSG(SSaveCharacterMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_SAVE_CHARACTER)
struct SQSaveCharacterMsg : public SSaveCharacterMsg
{
    DWORD dwServerId;               // ��������ķ�����ID

    SFixData PlayerData;

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

struct SASaveCharacterMsg : public SSaveCharacterMsg
{
    enum ERetErrCode
    {
        ERC_SUCCESS,
        ERC_FAIL,
    };

    BYTE byResult;

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};
//---------------------------------------------------------------------------------------------
// �����ɫ�Ĳֿ�����
DECLARE_MSG(SSaveWarehouseMsg, _SDataBaseMsg, _SDataBaseMsg::ERPO_SAVE_WAREHOUSE)
struct SQSaveWarehouseMsg : public SSaveWarehouseMsg
{
    DWORD dwServerId;               // ��������ķ�����ID

    SFixStorage1 PlayerWarehouseI;
//    SFixStorage2 PlayerWarehouseII;
//    SFixStorage3 PlayerWarehouseIII;

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

//---------------------------------------------------------------------------------------------
// ͨ��������ȡ������ҵ��ʺ�
DECLARE_MSG(SGetAccountByName, _SDataBaseMsg, _SDataBaseMsg::EPRO_GET_ACCOUNTBYNAME)
struct SQGetAccountByName : public SGetAccountByName
{
	char szQuestName[CONST_USERNAME];	
	char szName[CONST_USERNAME];

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

struct SAGetAccountByName : public SGetAccountByName
{
	enum ERetErrCode
	{
		ERC_SUCCESS,
		ERC_FAIL,
	};

    SAGetAccountByName()
    {
	    memset(szQuestName,0,sizeof(szQuestName));
	    memset(szName,0,sizeof(szName));
    }
  
	BYTE byResult;
	char szQuestName[CONST_USERNAME];
	char szName[CONST_USERNAME];

    // szResultAccount��szAccount
	char streamData[ MAX_ACCOUNT * 2 ];
};

struct DBHeadData
{
 char account[MAX_ACCOUNT];
 DWORD Paramter[4]; 
};

DECLARE_MSG(SSaveDBCenterData, _SDataBaseMsg, _SDataBaseMsg::EPRO_SAVE_DBCENTERDATA)
struct SQSaveDBCenterData: public SSaveDBCenterData
{
enum ERetErrCode
	{
		ERC_SUCCESS,
		ERC_FAIL,
	};
	DBHeadData	msghead;
	SFixBaseData basedata;
};

struct SASaveDBCenterData: public SSaveDBCenterData
{
enum ERetErrCode
	{
		ERC_SUCCESS,
		ERC_FAIL,
	};
	BYTE byResult;
};

// ͬ��ˢ��2����ҵ����ݣ���Ҫ��Ϊ�˽�����߽��ף�����������
DECLARE_MSG( _SDoubleSavePlayerMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_DOUBLE_SAVE_PLAYER )

struct SDoubleSavePlayerMsg : 
    public _SDoubleSavePlayerMsg
{
	DWORD serverId;
    DWORD gid[2];
    char account[2][MAX_ACCOUNT];
    SFixData data[2]; // �������ֻ��һ��������Ч,Ҳ������2��...
};

//��DB������ҵ�������
DECLARE_MSG(SSavePlayerConfig, _SDataBaseMsg, _SDataBaseMsg::EPRO_SAVEPLAYER_CONFIG)
struct SQLSavePlayerConfig : public SSavePlayerConfig
{
	DWORD serverId;
	char	streamData[MAX_ACCOUNT];	// �˺�
	char playerconfig[1024];	//��ҵ�������
};

// ���а������Ϣ
DECLARE_MSG(SDBGETRankList,_SDataBaseMsg,_SDataBaseMsg::EPRO_DBGET_RankList)
struct SQDBGETRankList : public SDBGETRankList
{
	DNID	serverID;
	DWORD	level;
};
struct SADBGETRankList : public SDBGETRankList
{
	DNID serverID;
	long m_Num;							// ���а����������Ϊ3000
	NewRankList m_NewRankList[ALL_RANK_NUM];
};


// д���������־  20150408 wk
DECLARE_MSG(SPointLogMsg, _SDataBaseMsg, _SDataBaseMsg::EPRO_WRITE_POINT_LOG)


struct SQPointLogMsg : public SPointLogMsg
{	
	DWORD  uid;
	char account[MAX_ACCOUNT];//����
	DWORD itype;//�����������
	INT diff_value;//���ӻ���ٵ���
	DWORD finale_value;//���������
	BYTE detailType;//ϸ������,�� itype=2Ԫ��ʱ,detailType=1Ϊ�����̳�,2Ϊ�������̳�
	DWORD pInfo;//�����Ϣ,��3λΪ�ȼ�,��2λΪvip�ȼ�,��22111,111��22��vip
	DWORD otherInfo;//Ŀǰ��������ʱ��ʾ����id

};


DECLARE_MSG(SScriptData, _SDataBaseMsg, _SDataBaseMsg::EPRO_SAVE_SCRIPTDATA)
struct SQScriptData : public SScriptData
{
	WORD   wLenth;
	BYTE    *pBuffer;
};

struct SAScriptData : public SScriptData
{
	WORD   wLenth;
	BYTE    *pBuffer;
};


DECLARE_MSG(SGetScriptData, _SDataBaseMsg, _SDataBaseMsg::EPRO_GET_SCRIPTDATA)
struct SQGetScriptData : public SGetScriptData
{
};

struct SAGetScriptData : public SGetScriptData
{
	WORD   wLenth;
	BYTE    *pBuffer;
};