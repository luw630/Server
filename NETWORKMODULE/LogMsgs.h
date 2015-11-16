#pragma once

#include "NetModule.h"
#include "logtypedef.h"
#include "itemtypedef.h"
#include "playertypedef.h"
#include "../pub/ConstValue.h"
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ϵͳ��Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SSysBaseMsg, SMessage, SMessage::EPRO_SYSTEM_MESSAGE)
//{{AFX
EPRO_LOGIN,	                // ��½ ����ʺź�����
EPRO_CREATE_CHARACTER,      // ������ɫ
EPRO_DEL_CHARACTER,         // ɾ����ɫ
EPRO_SELECT_CHARACTER,      // ѡ���ɫ
EPRO_LOGOUT,                // �˳�
EPRO_CHARACTER_LIST_INFO,   // ���ÿͻ��˽�ɫ�б�
EPRO_CHARACTER_DATA_INFO,   // ���ÿͻ�����������
EPRO_LOGIN_NOCHECKACCOUNT,	// �����ʺŷ�������飨����������������ԭ��������
EPRO_ENGINE_FLAGE,          // ����ͳ��ʹ������ �е�ɫ�汾����Ϊ1,û������Ϊ0
EPRO_CLIENT_READY,          // ����֪ͨ�������ͻ��˽���׼�����
EPRO_SAVEPLAYER_CONFIG_INFO,          // ������ҵ�������
EPRO_BLESSOpenClose_INFO,	// ��ϵͳ����
EPRO_EVERYDAYSIGNIN_AWARD,	//ÿ��ǩ��������Ϣ
EPRO_GET_SIGNINAWARD,	//��ȡÿ��ǩ������
EPRO_DATA_GETSID,			// ��ȡָ�����SID
//}}AFX
END_MSG_MAP()

// �û���½������������������ӣ��򵥰棩�����������û��汾�ţ��˺ţ�����
DECLARE_MSG(SLoginMsg, SSysBaseMsg, SSysBaseMsg::EPRO_LOGIN)
struct SQLoginMsg : public SLoginMsg 
{
	WORD wVersion;			        // �汾��Ϣ
    char streamData[MAX_ACCAPASS];	// �˺�&����
};

// ���ܵ�½�����Ļ�Ӧ���ɹ�����ʧ����Ϣ��dwRetCode֮��
struct SALoginMsg : public SLoginMsg
{
	enum ERetCode
	{
		ERC_LOGIN_SUCCESS,		        // ��½�ɹ�
		ERC_INVALID_VERSION,	        // �Ƿ��İ汾��
		ERC_INVALID_ACCOUNT,	        // ��Ч���˺�
		ERC_INVALID_PASSWORD,	        // ���������
		ERC_LOGIN_ALREADY_LOGIN,        // ���˺��ѵ�½
		ERC_GETLIST_TIMEOUT,            // ��ȡ��ɫ�б�ʱ
		ERC_GETLIST_FAIL,               // ��ȡ��ɫ�б�ʧ��
		ERC_CHECKACC_TIMEOUT,           // �˺ż�ⳬʱ
		ERC_SEND_GETCL_TO_DATASRV_FAIL, // �����ݿ���������ͻ�ȡ�б���Ϣʧ��
		ERC_SEND_CACC_TO_ACCSRV_FAIL,   // ���˺ŷ����������˺ż����Ϣʧ��
		ERC_ALREADYLOGIN_AND_LINKVALID, // ���˺��ѵ�½��ͬʱ��Ӧ�����ӻ�δʧЧ
		ERC_ALREADYLOGIN_BUT_INREBIND,  // ���˺��ѵ�½��ͬʱ��Ӧ��������ʧЧ�����Ǵ����ض���������[BUG]
		ERC_NOTENOUGH_CARDPOINT,        // ���˺ŵ�������
		ERC_SERVER_UPDATE,              // �����������У���ʱ���ܵ�½
		ERC_LOGIN_ERROR,                // ��½ʧ��
		ERC_CREATE_TEST_CHARACTER,      // �����˺ţ�ֱ��֪ͨ�����ɫ��������
		ERC_BLOCKED,                    // �˺ű�ͣȨ
		ERC_LIMITED,                    // ��½ʱ��̫��
		ERC_MAXCLIENTS,                 // �Ѿ��ﵽ��������
		ERC_LOCKEDON_SERVERID,          // ���˺������Ѿ���������Ϸ�������������ܾ��ظ���½
		ERC_QUEUEWAIT,                  // �Ŷӵȴ�
		ERC_REUPDATE,                   // ���¸���

		ERC_SPLIT = 32,					// ���ָ��ʾ��
		ERC_LOCKEDON_MOBILPHONE = 99,	// ���˺Ŵ����ֻ�����״̬���벦�����µ绰���н�����4008112096 �� 4006567023
	};

	WORD	dwRetCode;					// ����������Ϣ
    WORD	wGMLevel;
    DWORD	dwEchoIP;					// ����IP
	char	szDescription[MAX_USERDESC];

	SALoginMsg()
	{
		memset(szDescription, 0, sizeof(szDescription));
	}
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG( SNoCheckAccountLoginMsg, SSysBaseMsg, SSysBaseMsg::EPRO_LOGIN_NOCHECKACCOUNT )
struct SQLoginNoCheckAccountMsg : public SNoCheckAccountLoginMsg
{
	char streamData[1024];		// ���� �ʺ� ������� ������ID
};

DECLARE_MSG(SChListMsg, SSysBaseMsg, SSysBaseMsg::EPRO_CHARACTER_LIST_INFO)
struct SAChListMsg : public SChListMsg
{
    SCharListData CharListData[5];
};

//=============================================================================================
DECLARE_MSG(SLogoutMsg, SSysBaseMsg, SSysBaseMsg::EPRO_LOGOUT)
// ����������������ӣ��򵥰棩�����������û��汾�ţ��˺ţ�����
struct SQLogoutMsg : public SLogoutMsg  // �޷�������
{
    enum
    {
        EST_LOGOUT_NORMAL,  // ��ͨ�˳�
        EST_LOGOUT_HANGUP,  // ���߹һ�
    };

	DWORD dwGlobalID;		// �Լ���ȫ��ID���
    BYTE byLogoutState;     // �˳���״̬
};
//---------------------------------------------------------------------------------------------

// ������ɫ
DECLARE_MSG(SCrePlayerMsg, SSysBaseMsg, SSysBaseMsg::EPRO_CREATE_CHARACTER)
struct SQCrePlayerMsg : public SCrePlayerMsg
{
    BYTE byIndex;
    SC2LCreateFixProperty PlayerData;
};

struct SACrePlayerMsg : public SCrePlayerMsg
{
    enum ERetErrCode
    {
        ERC_SUCCESS = 1,
        ERC_INDEXTAKEN,
        ERC_NAMETAKEN,
        ERC_UNHANDLE,
        ERC_EXCEPUTION_NAME,
    };

    BYTE byResult;
    BYTE byIndex;
    SC2LCreateFixProperty PlayerData;
};
//---------------------------------------------------------------------------------------------

// ɾ����ɫ
DECLARE_MSG(SDelPlayerMsg, SSysBaseMsg, SSysBaseMsg::EPRO_DEL_CHARACTER)
struct SQDelPlayerMsg : SDelPlayerMsg
{
    DWORD dwStaticID;
    char  szUserpass[CONST_USERPASS];
};

struct SADelPlayerMsg : SDelPlayerMsg
{
    enum ERetErrCode
    {
        ERC_SUCCESS = 1,
        ERC_UNHANDLE,
    };
    BYTE byResult;
    DWORD dwStaticID;
};

// ѡ���ɫ
DECLARE_MSG(SSelPlayerMsg, SSysBaseMsg, SSysBaseMsg::EPRO_SELECT_CHARACTER)
struct SQSelPlayerMsg : public SSelPlayerMsg
{
    BYTE byIndex;
};

// ѡ���ɫ�Ļ�Ӧ���򵥰棩���ɹ�����ʧ����Ϣ��dwRetCode֮�У���ɫ�������ݻ�û��
struct SASelPlayerMsg : public SSelPlayerMsg 
{
	WORD dwRetCode;					// ����������Ϣ
	enum ERetCode
	{
		ERC_SELPLAYER_SUCCESS,	    // ��½�ɹ�
        ERC_GETCHDATA_TIMEOUT,      // �����ݿⷢ�ͻ�ȡ��ɫ������Ϣ��ʱ
        ERC_SEND_GETCHDATA_FAIL,    // �����ݿⷢ�ͻ�ȡ��ɫ������Ϣʧ��
        ERC_GETCHDATA_FAIL,         // �����ݿ��ȡ��ɫ����ʧ��
        ERC_PUTTOREGION_FAIL,       // ����ɫ������峡��ʧ��
        ERC_PLAYER_FULL,            // �÷��������Ѿ��ﵽ��������
	};
};

DECLARE_MSG(SChDataMsg, SSysBaseMsg, SSysBaseMsg::EPRO_CHARACTER_DATA_INFO)
// ����Ƿ�������ý�ɫ�б��֪ͨ�ͻ��˵���Ϣ
struct SAChDataMsg : public SChDataMsg
{
	DWORD	dwPlayerGlobalID;			// ��������ȫ��Ψһ��ʶ��
	DWORD	dwRegionGlobalID;			// ��������ȫ��Ψһ��ʶ��
    DWORD   dwPlayerStaticID;           // ��ҵ�StaticID
    //WORD	wCurMapID;                  // ��ǰ����ͼƬ���
	SSanGuoPlayerFixData fixData; //�������
	DWORD dwLastArenaChallengeTimeSpan;//������ʱ����
	DWORD dwArenaChallengeCount; //��������ս����
	DWORD BuyChallengeCount; //��������ս����
	DWORD ReSetCountDownCount;//���������ô���
	DWORD dwServerStartTime; //����������ʱ��
	DWORD dwBestArenaRank; //�������������
    //SXYD3FixData		PlayerBaseData;				// ��һ�������
	//SPlayerMounts		mountsData;					// ��������	
	//BYTE				taskInfo[MAX_WHOLE_TASK/8];	// ��������
	//SPlayerJingMai		venInfo;					// ��������
	//SPlayerRelation		m_playerrelation;		//��Һ�������
	//SPlayerPets			petsData;					// [2012-7-24 12-25 gw: +��ս����������]
};

DECLARE_MSG( SEngineFlage, SSysBaseMsg, SSysBaseMsg::EPRO_ENGINE_FLAGE )
struct SQEngineFlage : public SEngineFlage
{
	BYTE byEngineFlage;		
};

DECLARE_MSG( SClientReady, SSysBaseMsg, SSysBaseMsg::EPRO_CLIENT_READY )
struct SQClientReady : public SClientReady
{
			
};
DECLARE_MSG( SSavePlayerConfigInfo, SSysBaseMsg, SSysBaseMsg::EPRO_SAVEPLAYER_CONFIG_INFO )
struct SQSavePlayerConfigInfo : public SSavePlayerConfigInfo
{
	char	streamData[MAX_ACCOUNT];	// �˺�
	char playerconfig[1024];	//��ҵ�������
};
DECLARE_MSG( SBLESSOpenClose, SSysBaseMsg, SSysBaseMsg::EPRO_BLESSOpenClose_INFO )
struct SABLESSOpenClose : SBLESSOpenClose
{
	BYTE Flag;  // 1 �ǿ� 2�ǹ�
};


//ÿ��ǩ��������Ϣ
DECLARE_MSG(SEveryDaySignInAwardMsg, SSysBaseMsg, SSysBaseMsg::EPRO_EVERYDAYSIGNIN_AWARD)
struct SAEveryDaySignInAwardMsg : public SEveryDaySignInAwardMsg
{
	BYTE m_EveryDaySignInIndex;  // ÿ��ǩ������
	BYTE m_AwardState;	//�ý����Ƿ��Ѿ���ȡ��״ֵ̬��1������ȡ��0��������ȡ
};

//��ȡÿ��ǩ������
DECLARE_MSG(SGetSignInAwardMsg, SSysBaseMsg, SSysBaseMsg::EPRO_GET_SIGNINAWARD)
struct SQGetSignInAwardMsg : SGetSignInAwardMsg
{
	BYTE m_EveryDaySignInIndex;  // ÿ��ǩ������
};
struct SAGetSignInAwardMsg : SGetSignInAwardMsg
{
	BYTE m_OptResult;  //��ȡ�����0ʧ�ܣ�1�ɹ�
};

DECLARE_MSG(SGetsid, SSysBaseMsg, SSysBaseMsg::EPRO_DATA_GETSID)
struct SQGetsid : public SGetsid
{
	char  strname[CONST_USERNAME];  //����鿴�������
};
struct SAGetsid : public SGetsid
{
	char  strname[CONST_USERNAME];//����鿴�������
	DWORD  dsid;					//��Ӧ��SID
};